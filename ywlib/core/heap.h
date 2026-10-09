#pragma once
#include <core/core.h>
#include <core/property.h>
#include <core/tuple.h>

namespace yw {

///--------------------------------------------------------------------------///
/// MARK: uninitialized_heap

template<is_object T> class uninitialized_heap {
public:
  const_property<T*, uninitialized_heap> get = nullptr;

protected:
  static constexpr T* _allocate() noexcept {
    try {
      return std::allocator<T>().allocate(1);
    } catch (...) { return nullptr; }
  }

public:
  using value_type = T;

  explicit constexpr operator bool() const noexcept { return get() != nullptr; }
  constexpr value_type* operator->() const noexcept ywlib_pre(get() != nullptr) { return get(); }
  constexpr value_type& operator*() const noexcept ywlib_pre(get() != nullptr) { return *(get()); }

  constexpr ~uninitialized_heap() noexcept { deallocate(); }

  constexpr uninitialized_heap() = default;
  uninitialized_heap(const uninitialized_heap&) = delete;
  uninitialized_heap& operator=(const uninitialized_heap&) = delete;

  constexpr uninitialized_heap(uninitialized_heap&& o) noexcept : get(exchange(o.get.ref(), nullptr)) {}

  constexpr uninitialized_heap& operator=(uninitialized_heap&& o) noexcept {
    if (this == &o) return *this;
    deallocate();
    get = exchange(o.get.ref(), nullptr);
    return *this;
  }

  constexpr uninitialized_heap(is_none auto) noexcept ywlib_post(this->get() != nullptr) : get(_allocate()) {}

  constexpr void allocate() noexcept ywlib_post(get() != nullptr) {
    if (get() == nullptr) get = _allocate();
  }

  constexpr void deallocate() noexcept {
    if (get() != nullptr) std::allocator<T>().deallocate(exchange(get.ref(), nullptr), 1);
  }
};

///--------------------------------------------------------------------------///
/// MARK: uninitialized_heap<T[]>

template<is_object T> class uninitialized_heap<T[]> {
public:
  const_property<T*, uninitialized_heap> get = nullptr;
  const_property<size_t, uninitialized_heap> size = 0;

protected:
  static constexpr T* _allocate(size_t n) noexcept {
    try {
      return std::allocator<T>().allocate(n);
    } catch (...) { return nullptr; }
  }

public:
  using value_type = T;

  explicit constexpr operator bool() const noexcept { return get() != nullptr; }
  constexpr value_type& operator[](size_t i) const noexcept ywlib_pre(i < size.cref()) { return get()[i]; }

  constexpr ~uninitialized_heap() noexcept { deallocate(); }

  constexpr uninitialized_heap() = default;
  uninitialized_heap(const uninitialized_heap&) = delete;
  uninitialized_heap& operator=(const uninitialized_heap&) = delete;

  constexpr uninitialized_heap(uninitialized_heap&& o) noexcept
    : get(exchange(o.get.ref(), nullptr)), size(exchange(o.size.ref(), 0)) {}

  constexpr uninitialized_heap& operator=(uninitialized_heap&& o) noexcept {
    if (this == &o) return *this;
    deallocate();
    get = exchange(o.get.ref(), nullptr);
    size = exchange(o.size.ref(), 0);
    return *this;
  }

  constexpr uninitialized_heap(size_t n) noexcept ywlib_post(this->get.cref() != nullptr) : get(_allocate(n)), size(n) {}

  constexpr void allocate(size_t n) noexcept ywlib_post(get() != nullptr) {
    if (get() != nullptr) {
      if (size == n) return;
      std::allocator<T>().deallocate(get(), size);
    }
    get = _allocate(n), size = n;
  }

  constexpr void deallocate() noexcept {
    if (get() != nullptr) std::allocator<T>().deallocate(exchange(get.ref(), nullptr), exchange(size.ref(), 0));
  }
};

///--------------------------------------------------------------------------///
/// MARK: heap

template<is_object T> requires(!is_unbounded_array<T>) class heap {
  uninitialized_heap<T> _heap;

public:
  using value_type = T;

  explicit constexpr operator bool() const noexcept { return bool(_heap); }
  constexpr value_type* get() const noexcept { return _heap.get(); }
  constexpr value_type* operator->() const noexcept ywlib_pre(bool(_heap)) { return _heap.operator->(); }
  constexpr value_type& operator*() const noexcept ywlib_pre(bool(_heap)) { return _heap.operator*(); }

  constexpr ~heap() noexcept {
    if (_heap) std::destroy_at(_heap.get());
  }

  constexpr heap() = default;
  heap(const heap&) = delete;
  heap& operator=(const heap&) = delete;
  constexpr heap(heap&&) noexcept = default;

  constexpr heap& operator=(heap&& o) noexcept {
    if (this == &o) return *this;
    if (_heap) std::destroy_at(_heap.get());
    _heap = move(o._heap);
    return *this;
  }

  template<typename... As> requires constructible<T, As...> && (sizeof...(As) > 0)
  constexpr explicit heap(As&&... as) noexcept(nt_constructible<T, As...>) {
    _heap.allocate();
    std::construct_at(_heap.get(), static_cast<As&&>(as)...);
  }

  template<typename... As> requires constructible<T, As...>
  constexpr void construct(As&&... as) noexcept(nt_constructible<T, As...>) {
    if (!_heap) _heap.allocate();
    else std::destroy_at(_heap.get());
    std::construct_at(_heap.get(), static_cast<As&&>(as)...);
  }

  constexpr void destruct() noexcept {
    if (_heap) std::destroy_at(_heap.get()), _heap.deallocate();
  }
};
} // namespace yw
