#pragma once
#include <core/core.h>

namespace yw {

template<typename Com> class comptr {
  comptr(const comptr&) = delete;
  comptr& operator=(const comptr&) = delete;
  Com* _ptr{nullptr};

public:
  explicit operator bool() const noexcept { return _ptr != nullptr; }
  explicit operator Com*&() & noexcept { return _ptr; }
  explicit operator Com*() const& noexcept { return _ptr; }

  Com* operator->() const noexcept { return _ptr; }

  bool operator==(Com* Other) const noexcept { return _ptr == Other; }

  ~comptr() {
    if (_ptr) _ptr->Release();
    _ptr = nullptr;
  }

  comptr() noexcept = default;

  comptr(comptr&& Other) noexcept : _ptr(std::exchange(Other._ptr, nullptr)) {}

  comptr& operator=(comptr&& Other) {
    if (this == &Other) return *this;
    if (_ptr) _ptr->Release();
    _ptr = std::exchange(Other._ptr, nullptr);
    return *this;
  }

  Com*& get() & noexcept { return _ptr; }
  Com* get() const& noexcept { return _ptr; }

  void release() {
    if (_ptr) _ptr->Release();
    _ptr = nullptr;
  }

  void reset(Com* New) noexcept {
    if (_ptr) _ptr->Release();
    _ptr = New;
  }
};
} // namespace yw
