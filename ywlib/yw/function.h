#pragma once
#include "yw/error.h"

namespace yw {

constexpr size_t function_sbo_size = 24;

template<typename R, typename... As> requires(!specialization_of<remove_cvref<R>, std::expected>) ||
                                             same_as<typename R::error_type, error> class function {
  static constexpr size_t _sbo_size = function_sbo_size;
  alignas(std::max_align_t) std::byte _storage[_sbo_size]{};

  R (*fp)(As...) = nullptr;
  R (*_invoke)(const void* src, As&&... args) = nullptr;
  void (*_destroy)(void* src) = nullptr;
  void (*_move)(void* dst, void* src) = nullptr;
  void (*_copy)(void* dst, const void* src) = nullptr;

  bool _has_obj() const noexcept { return _destroy != nullptr; }

  void _reset() noexcept {
    if (_has_obj()) _destroy(_storage);
    fp = nullptr;
    _invoke = nullptr;
    _destroy = nullptr;
    _move = nullptr;
    _copy = nullptr;
  }

public:
  using result_type = decltype([] {
    using U = remove_cvref<R>;
    if constexpr (specialization_of<U, std::expected>) return U();
    else if constexpr (specialization_of<U, std::optional>) return std::expected<typename U::value_type, error>();
    else if constexpr (specialization_of<U, yw::optional>) return std::expected<typename U::value_type, error>();
    else return std::expected<R, error>();
  }());

  constexpr function() noexcept = default;

  // assigns a function pointer
  template<class F> //
  requires is_pointer<remove_ref<F>> && is_function<remove_pointer<remove_ref<F>>> && invocable_r<F, R, As...>
  constexpr function(F f) noexcept : fp(f) {}

  // assigns a function object (SBO only, copyable)
  template<class F> //
  requires invocable_r<const remove_cvref<F>&, R, As...> &&
           (!(is_pointer<remove_ref<F>> && is_function<remove_pointer<remove_ref<F>>>)) &&
           (sizeof(remove_cvref<F>) <= _sbo_size) && (alignof(remove_cvref<F>) <= alignof(std::max_align_t)) &&
           std::is_nothrow_move_constructible_v<remove_cvref<F>> && std::is_copy_constructible_v<remove_cvref<F>> &&
           different_from<remove_cvref<F>, function> function(F&& f) noexcept {
    using G = remove_cvref<F>;
    new (_storage) G(static_cast<F&&>(f));
    _invoke = [](const void* src, As&&... args) -> R {
      return (*static_cast<const G*>(src))(static_cast<As&&>(args)...);
    };
    _destroy = [](void* src) { static_cast<G*>(src)->~G(); };
    _move = [](void* dst, void* src) {
      new (dst) G(std::move(*static_cast<G*>(src)));
      static_cast<G*>(src)->~G();
    };
    _copy = [](void* dst, const void* src) { new (dst) G(*static_cast<const G*>(src)); };
  }

  function(const function& other) {
    fp = other.fp;
    _invoke = other._invoke;
    _destroy = other._destroy;
    _move = other._move;
    _copy = other._copy;
    if (other._has_obj()) other._copy(_storage, other._storage);
  }

  function(function&& other) noexcept {
    fp = other.fp;
    _invoke = other._invoke;
    _destroy = other._destroy;
    _move = other._move;
    _copy = other._copy;
    if (other._has_obj()) other._move(_storage, other._storage);
    other.fp = nullptr;
    other._invoke = nullptr;
    other._destroy = nullptr;
    other._move = nullptr;
    other._copy = nullptr;
  }

  function& operator=(const function& other) {
    if (this == &other) return *this;
    _reset();
    fp = other.fp;
    _invoke = other._invoke;
    _destroy = other._destroy;
    _move = other._move;
    _copy = other._copy;
    if (other._has_obj()) other._copy(_storage, other._storage);
    return *this;
  }

  function& operator=(function&& other) noexcept {
    if (this == &other) return *this;
    _reset();
    fp = other.fp;
    _invoke = other._invoke;
    _destroy = other._destroy;
    _move = other._move;
    _copy = other._copy;
    if (other._has_obj()) other._move(_storage, other._storage);
    other.fp = nullptr;
    other._invoke = nullptr;
    other._destroy = nullptr;
    other._move = nullptr;
    other._copy = nullptr;
    return *this;
  }

  ~function() { _reset(); }

  result_type operator()(As... args) const {
    if (!fp && !_invoke) return std::unexpected(error(errors::not_initialized, "empty function"));
    using U = remove_cvref<R>;
    if constexpr (specialization_of<U, std::expected>) {
      if (fp) return fp(static_cast<As&&>(args)...);
      else return _invoke(_storage, static_cast<As&&>(args)...);
    } else if constexpr (specialization_of<U, std::optional> || specialization_of<U, yw::optional>) {
      if (fp) {
        if (auto res = fp(static_cast<As&&>(args)...)) return std::move(*res);
        else return std::unexpected(error(errors::operation_failed, "function returned empty optional"));
      } else if (auto res = _invoke(_storage, static_cast<As&&>(args)...)) return std::move(*res);
      else return std::unexpected(error(errors::operation_failed, "function returned empty optional"));
    } else if constexpr (is_void<R>) {
      if (fp) fp(static_cast<As&&>(args)...);
      else _invoke(_storage, static_cast<As&&>(args)...);
      return {};
    } else {
      if (fp) return fp(static_cast<As&&>(args)...);
      return _invoke(_storage, static_cast<As&&>(args)...);
    }
  }

  explicit operator bool() const noexcept { return fp || _invoke; }

  void reset() noexcept { _reset(); }

  class slot {
    function* _dest = nullptr;
    slot& operator=(const slot&) & = delete;

  public:
    explicit slot() noexcept = default;
    slot(function& Dest) noexcept : _dest(&Dest) {}

    void operator=(function func) && {
      if (_dest) *_dest = std::move(func);
    }

    void operator=(const slot& func) && {
      if (_dest && func._dest) *_dest = *func._dest;
    }

    void reset() && {
      if (_dest) _dest->reset();
    }
  };
};
} // namespace yw
