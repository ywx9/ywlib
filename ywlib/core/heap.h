#pragma once
#include <core/core.h>
#include <core/property.h>
#include <core/tuple.h>

namespace yw {

/// At runtime this owns raw storage only.
/// In constant evaluation it must allocate with `new T`,
/// so constructing another T in the same storage ends the lifetime of
/// the default-constructed object without running its destructor.
/// Use that path only when skipping that destructor is semantically harmless.

///--------------------------------------------------------------------------///
/// MARK: uninitialized_heap

template<is_object T> class uninitialized_heap {
  T* _ptr = nullptr;

  constexpr void _reset() const noexcept {
    if consteval {
      delete _ptr;
    } else {
      if constexpr (alignof(T) > __STDCPP_DEFAULT_NEW_ALIGNMENT__) {
        ::operator delete(_ptr, ::std::align_val_t(alignof(T)));
      } else ::operator delete(_ptr);
    }
  }

  static constexpr T* _allocate() noexcept {
    if consteval {
      return new T;
    } else {
      if constexpr (alignof(T) > __STDCPP_DEFAULT_NEW_ALIGNMENT__) {
        return static_cast<T*>(::operator new(sizeof(T), ::std::align_val_t(alignof(T)), std::nothrow));
      } else return static_cast<T*>(::operator new(sizeof(T), std::nothrow));
    }
  }

public:
  using value_type = T;

  explicit constexpr operator bool() const noexcept { return _ptr != nullptr; }
  constexpr value_type* get() const noexcept { return _ptr; }
  constexpr value_type* operator->() const noexcept ywlib_pre(_ptr != nullptr) { return _ptr; }
  constexpr value_type& operator*() const noexcept ywlib_pre(_ptr != nullptr) { return *_ptr; }

  constexpr ~uninitialized_heap() noexcept { _reset(); }
  constexpr uninitialized_heap() = default;
  uninitialized_heap(const uninitialized_heap&) = delete;
  uninitialized_heap& operator=(const uninitialized_heap&) = delete;
  constexpr uninitialized_heap(uninitialized_heap&& o) noexcept : _ptr(exchange(o._ptr, nullptr)) {}
  constexpr uninitialized_heap& operator=(uninitialized_heap&& o) noexcept {
    if (this == &o) return *this;
    _reset();
    _ptr = exchange(o._ptr, nullptr);
    return *this;
  }
  constexpr uninitialized_heap(is_none auto) noexcept ywlib_post(this->_ptr != nullptr) : _ptr(_allocate()) {}
  constexpr void reset() noexcept { _reset(), _ptr = nullptr; }
};

///--------------------------------------------------------------------------///
/// MARK: uninitialized_heap<T[]>

template<is_object T> class uninitialized_heap<T[]> {
  T* _ptr = nullptr;

  constexpr void _reset() const noexcept {
    if consteval {
      delete[] _ptr;
    } else {
      if constexpr (alignof(T) > __STDCPP_DEFAULT_NEW_ALIGNMENT__) {
        ::operator delete(_ptr, ::std::align_val_t(alignof(T)));
      } else ::operator delete(_ptr);
    }
  }

  static constexpr T* _allocate(size_t n) noexcept {
    if consteval {
      return new T[n];
    } else {
      if constexpr (alignof(T) > __STDCPP_DEFAULT_NEW_ALIGNMENT__) {
        return static_cast<T*>(::operator new(sizeof(T) * n, ::std::align_val_t(alignof(T)), std::nothrow));
      } else return static_cast<T*>(::operator new(sizeof(T) * n, std::nothrow));
    }
  }

public:
  using value_type = T;

  explicit constexpr operator bool() const noexcept { return _ptr != nullptr; }
  constexpr value_type* get() const noexcept { return _ptr; }
  constexpr value_type& operator[](size_t i) const noexcept ywlib_pre(_ptr != nullptr) { return _ptr[i]; }

  constexpr ~uninitialized_heap() noexcept { _reset(); }
  constexpr uninitialized_heap() = default;
  uninitialized_heap(const uninitialized_heap&) = delete;
  uninitialized_heap& operator=(const uninitialized_heap&) = delete;
  constexpr uninitialized_heap(uninitialized_heap&& o) noexcept : _ptr(exchange(o._ptr, nullptr)) {}
  constexpr uninitialized_heap& operator=(uninitialized_heap&& o) noexcept {
    if (this == &o) return *this;
    _reset();
    _ptr = exchange(o._ptr, nullptr);
    return *this;
  }
  constexpr uninitialized_heap(size_t n) noexcept ywlib_post(this->_ptr != nullptr) : _ptr(_allocate(n)) {}
  constexpr void reset() noexcept { _reset(), _ptr = nullptr; }
};

///--------------------------------------------------------------------------///
/// MARK: heap

template<is_object T> requires(!is_unbounded_array<T>) class heap {
  uninitialized_heap<T> _heap;

  constexpr void _reset() noexcept {
    if (_heap) _heap.get()->~T(), _heap.reset();
  }

public:
  using value_type = T;

  explicit constexpr operator bool() const noexcept { return _heap; }
  constexpr value_type* get() const noexcept { return _heap.get(); }
  constexpr value_type* operator->() const noexcept { return _heap.get(); }
  constexpr value_type& operator*() const noexcept { return *_heap; }

  constexpr ~heap() noexcept { _reset(); }
  heap(const heap&) = delete;
  heap& operator=(const heap&) = delete;
  constexpr heap(heap&&) noexcept = default;
  constexpr heap& operator=(heap&& o) noexcept {
    if (this == &o) return *this;
    _reset();
    _heap = exchange(o._heap, {});
    return *this;
  }

  template<typename... As> requires constructible<T, As...>
  constexpr explicit heap(As&&... as) noexcept(nt_constructible<T, As...>) : _heap(none()) {
    new (_heap.get()) T(static_cast<As&&>(as)...);
  }
};
} // namespace yw
