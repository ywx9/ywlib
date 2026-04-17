#pragma once
#include "ywx/core.h"

namespace yw {

class dll {
public:
  struct slot {
    HMODULE _hmodule = nullptr;
  };

  template<typename Rt, typename... Ts> class function {
    using slotid = slotset<slot>::slotid;
    using fn_type = Rt (*)(Ts...);

  private:
    slotid _id{};
    fn_type _fn = nullptr;

    explicit function(slotid id, fn_type fn) noexcept : _id(id), _fn(fn) {}
    friend class dll;

  public:
    function() noexcept = default;

    explicit operator bool() const noexcept { return _fn && dll::_alive(_id); }

    std::expected<Rt, error_trace> operator()(Ts... args) const {
      if (!_fn) return unexpected_error(errors::not_initialized, "function is not initialized");
      if (!dll::_alive(_id)) return unexpected_error(errors::operation_failed, "dll is already closed");
      if constexpr (is_void<Rt>) {
        _fn(static_cast<Ts&&>(args)...);
        return {};
      } else return _fn(static_cast<Ts&&>(args)...);
    }
  };

private:
  inline static slotset<slot> dlls = {};
  slotset<slot>::slotid _id{};

  static slot* _slot_of(slotset<slot>::slotid id) noexcept {
    if (!id) return nullptr;
    return dlls.get(id);
  }

  static bool _alive(slotset<slot>::slotid id) noexcept {
    const auto sp = _slot_of(id);
    return sp && sp->_hmodule;
  }

public:
  dll() noexcept = default;

  dll(const dll&) = delete;
  dll& operator=(const dll&) = delete;

  dll(dll&& other) noexcept : _id(std::exchange(other._id, {})) {}
  dll& operator=(dll&& other) noexcept {
    if (this == &other) return *this;
    close();
    _id = std::exchange(other._id, {});
    return *this;
  }

  ~dll() noexcept { close(); }

  static std::expected<dll, error_trace> open(const std::filesystem::path& Path) {
    if (Path.empty()) return unexpected_error(errors::invalid_argument, "Path is empty");
    const auto hm = ::LoadLibraryW(Path.c_str());
    if (!hm) return unexpected_win32_error("LoadLibraryW failed");

    std::unique_ptr<slot> sp{};
    try {
      sp = std::make_unique<slot>();
    } catch (...) {
      ::FreeLibrary(hm);
      return unexpected_error(errors::operation_failed, "Failed to allocate dll slot");
    }
    sp->_hmodule = hm;

    dll res{};
    res._id = dlls.add(std::move(sp));
    if (!res._id) {
      ::FreeLibrary(hm);
      return unexpected_error(errors::operation_failed, "Failed to register dll slot");
    }
    return res;
  }

  void close() noexcept {
    if (!_id) return;
    if (const auto sp = _slot_of(_id); sp && sp->_hmodule) {
      ::FreeLibrary(sp->_hmodule);
      sp->_hmodule = nullptr;
    }
    dlls.erase(_id);
    _id = {};
  }

  template<typename Rt, typename... Ts>
  std::expected<function<Rt, Ts...>, error_trace> get_function(null_terminated<char> name) {
    if (!_id) return unexpected_error(errors::not_initialized, "dll is not initialized");
    if (name.empty()) return unexpected_error(errors::invalid_argument, "function name is empty");
    const auto sp = _slot_of(_id);
    if (!sp || !sp->_hmodule) return unexpected_error(errors::operation_failed, "dll handle is not valid");

    ::SetLastError(ERROR_SUCCESS);
    const auto p = ::GetProcAddress(sp->_hmodule, name.data());
    if (!p) return unexpected_win32_error("GetProcAddress failed");
    return function<Rt, Ts...>(_id, reinterpret_cast<Rt (*)(Ts...)>(p));
  }
};
} // namespace yw
