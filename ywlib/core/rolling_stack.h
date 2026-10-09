#pragma once
#include <core/heap.h>

namespace yw {

template<typename T> class rolling_stack {
  uninitialized_heap<T[]> _heap;
  size_t _first = 0;

  constexpr size_t _index(size_t i) const noexcept {
    const auto tail = capacity() - _first;
    return i < tail ? _first + i : i - tail;
  }

  constexpr void _destroy(size_t i) noexcept {
    T* p = _heap.get() + _index(i);
    std::destroy_at(p);
    if consteval {
      if constexpr (constructible<T>) std::construct_at(p);
      else ::abort();
    }
  }

public:
  const_property<size_t, rolling_stack> size;
  const_property<size_t, rolling_stack> capacity;

  constexpr rolling_stack() = default;
  explicit constexpr rolling_stack(size_t n) noexcept
    : _heap(n ? uninitialized_heap<T[]>(n) : uninitialized_heap<T[]>()), capacity(n) {}
  constexpr ~rolling_stack() noexcept { clear(); }

  constexpr rolling_stack(const rolling_stack& o) noexcept requires constructible<T, const T&>
    : rolling_stack(o.capacity()) {
    for (size_t i = 0; i < o.size(); ++i) std::construct_at(_heap.get() + i, o[i]);
    size = o.size();
  }

  constexpr rolling_stack& operator=(const rolling_stack& o) noexcept requires constructible<T, const T&> {
    if (this == &o) return *this;
    rolling_stack copy(o);
    swap(copy);
    return *this;
  }

  constexpr rolling_stack(rolling_stack&& o) noexcept
    : _heap(move(o._heap)), _first(exchange(o._first, 0)),
      size(exchange(o.size.ref(), 0)), capacity(exchange(o.capacity.ref(), 0)) {}

  constexpr rolling_stack& operator=(rolling_stack&& o) noexcept {
    if (this == &o) return *this;
    clear();
    _heap = move(o._heap);
    _first = exchange(o._first, 0);
    size = exchange(o.size.ref(), 0);
    capacity = exchange(o.capacity.ref(), 0);
    return *this;
  }

  constexpr bool empty() const noexcept { return size() == 0; }
  constexpr T& operator[](size_t i) noexcept ywlib_pre(i < this->size()) { return _heap[_index(i)]; }
  constexpr const T& operator[](size_t i) const noexcept ywlib_pre(i < this->size()) { return _heap[_index(i)]; }
  constexpr T& front() noexcept ywlib_pre(this->size() > 0) { return (*this)[0]; }
  constexpr const T& front() const noexcept ywlib_pre(this->size() > 0) { return (*this)[0]; }
  constexpr T& back() noexcept ywlib_pre(this->size() > 0) { return (*this)[size() - 1]; }
  constexpr const T& back() const noexcept ywlib_pre(this->size() > 0) { return (*this)[size() - 1]; }

  constexpr void clear() noexcept {
    for (size_t i = 0; i < size(); ++i) _destroy(i);
    size = 0;
    _first = 0;
  }

  // Shrinking keeps the newest elements; changing capacity linearizes storage.
  constexpr void set_capacity(size_t n) noexcept {
    if (n == capacity()) return;
    if (n == 0) {
      clear();
      _heap.reset();
      capacity = 0;
      return;
    }
    uninitialized_heap<T[]> new_heap(n);
    const auto kept = yw::min(size(), n);
    const auto skipped = size() - kept;
    for (size_t i = 0; i < kept; ++i)
      std::construct_at(new_heap.get() + i, std::move_if_noexcept((*this)[skipped + i]));
    clear();
    _heap = move(new_heap);
    capacity = n;
    size = kept;
  }

  constexpr void pop() noexcept {
    if (empty()) return;
    _destroy(size() - 1);
    size = size() - 1;
    if (empty()) _first = 0;
  }

  template<typename... As> requires constructible<T, As...>
  constexpr T& emplace(As&&... as) noexcept ywlib_pre(this->capacity._ > 0) {
    if (size() == capacity()) {
      // Construct first so arguments may refer to the element being evicted.
      T value(static_cast<As&&>(as)...);
      _destroy(0);
      std::construct_at(_heap.get() + _first, std::move_if_noexcept(value));
      _first = _first + 1 == capacity() ? 0 : _first + 1;
    } else {
      std::construct_at(_heap.get() + _index(size()), static_cast<As&&>(as)...);
      size = size() + 1;
    }
    return back();
  }

  constexpr rolling_stack& push(const T& v) noexcept { emplace(v); return *this; }
  constexpr rolling_stack& push(T&& v) noexcept { emplace(move(v)); return *this; }

  constexpr void swap(rolling_stack& o) noexcept {
    std::ranges::swap(_heap, o._heap);
    std::ranges::swap(_first, o._first);
    std::ranges::swap(size.ref(), o.size.ref());
    std::ranges::swap(capacity.ref(), o.capacity.ref());
  }
};
} // namespace yw
