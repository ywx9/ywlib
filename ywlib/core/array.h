#pragma once
#include <core/core.h>
#include <core/heap.h>
#include <core/property.h>

namespace yw {

///--------------------------------------------------------------------------///
/// MARK: array_view

template<typename T> struct array_view {
  const_property<T*, array_view> data = nullptr;
  const_property<size_t, array_view> size = 0;

  constexpr array_view() noexcept = default;
  constexpr array_view(const array_view&) noexcept = default;
  constexpr array_view& operator=(const array_view&) noexcept = default;
  constexpr array_view(array_view&&) noexcept = default;
  constexpr array_view& operator=(array_view&&) noexcept = default;

  constexpr array_view(T* data, size_t size) noexcept : data(data), size(size) {}

  template<contiguous_iterator<T> It, sized_sentinel_for<It> Se> //
  constexpr array_view(It i, Se s) noexcept : data(std::to_address(i)), size(s - i) {}

  template<contiguous_range<T> Rg> //
  constexpr array_view(Rg&& r) noexcept : array_view(yw::data(r), yw::size(r)) {}

  constexpr bool empty() const noexcept { return size() == 0; }
  constexpr T* begin() noexcept { return data(); }
  constexpr const T* begin() const noexcept { return data(); }
  constexpr T* end() noexcept { return data() + size(); }
  constexpr const T* end() const noexcept { return data() + size(); }
  constexpr T& operator[](size_t i) noexcept ywlib_pre(i < size()) { return data()[i]; }
  constexpr const T& operator[](size_t i) const noexcept ywlib_pre(i < size()) { return data()[i]; }
  constexpr T& front() noexcept ywlib_pre(!empty()) { return data()[0]; }
  constexpr const T& front() const noexcept ywlib_pre(!empty()) { return data()[0]; }
  constexpr T& back() noexcept ywlib_pre(!empty()) { return data()[size() - 1]; }
  constexpr const T& back() const noexcept ywlib_pre(!empty()) { return data()[size() - 1]; }

  constexpr void remove_prefix(size_t n) noexcept ywlib_pre(n <= size()) { data += n, size -= n; }
  constexpr void remove_suffix(size_t n) noexcept ywlib_pre(n <= size()) { size -= n; }

  constexpr void swap(array_view& o) noexcept {
    std::ranges::swap(data.ref(), o.data.ref());
    std::ranges::swap(size.ref(), o.size.ref());
  }

  constexpr array_view subview(size_t pos, size_t n = npos) const noexcept ywlib_pre(pos <= size()) {
    return array_view(data() + pos, yw::min(n, size() - pos));
  }
};

template<typename T> array_view(T*, size_t) -> array_view<T>;
template<contiguous_iterator It, sized_sentinel_for<It> Se> array_view(It, Se)
  -> array_view<remove_ref<iter_reference_t<It>>>;
template<contiguous_range Rg> array_view(Rg&&) -> array_view<remove_ref<iter_reference_t<Rg>>>;

///--------------------------------------------------------------------------///
/// MARK: array<T, 0>

template<typename T, size_t N = npos> struct array;

template<typename T> struct array<T, 0> {
  constexpr size_t size() const noexcept { return 0; }
  constexpr bool empty() const noexcept { return true; }
  constexpr T* data() noexcept { return nullptr; }
  constexpr const T* data() const noexcept { return nullptr; }
  constexpr T* begin() noexcept { return nullptr; }
  constexpr const T* begin() const noexcept { return nullptr; }
  constexpr T* end() noexcept { return nullptr; }
  constexpr const T* end() const noexcept { return nullptr; }

  constexpr operator array_view<T>() noexcept { return array_view<T>(nullptr, 0); }
  constexpr operator array_view<const T>() const noexcept { return array_view<const T>(nullptr, 0); }
  constexpr array_view<T> view() noexcept { return array_view<T>(nullptr, 0); }
  constexpr array_view<const T> view() const noexcept { return array_view<const T>(nullptr, 0); }
};

///--------------------------------------------------------------------------///
/// MARK: array<T, N>

template<typename T, size_t N> requires(N > 0 && N < npos) struct array<T, N> {
  property<T[N], array> data;

  constexpr size_t size() const noexcept { return N; }
  constexpr bool empty() const noexcept { return false; }
  constexpr T* begin() noexcept { return data(); }
  constexpr const T* begin() const noexcept { return data(); }
  constexpr T* end() noexcept { return data() + N; }
  constexpr const T* end() const noexcept { return data() + N; }
  constexpr T& operator[](size_t i) noexcept { return data()[i]; }
  constexpr const T& operator[](size_t i) const noexcept { return data()[i]; }
  constexpr T& front() noexcept { return data()[0]; }
  constexpr const T& front() const noexcept { return data()[0]; }
  constexpr T& back() noexcept { return data()[N - 1]; }
  constexpr const T& back() const noexcept { return data()[N - 1]; }

  constexpr void swap(array& o) noexcept { std::ranges::swap_ranges(data(), data() + N, o.data()); }

  constexpr operator array_view<T>() noexcept { return array_view<T>(data(), N); }
  constexpr operator array_view<const T>() const noexcept { return array_view<const T>(data(), N); }
  constexpr array_view<T> view() noexcept { return array_view<T>(data(), N); }
  constexpr array_view<const T> view() const noexcept { return array_view<const T>(data(), N); }

  constexpr array_view<T> subview(size_t pos, size_t n = npos) noexcept ywlib_pre(pos <= N) {
    return array_view<T>(data() + pos, yw::min(n, N - pos));
  }
  constexpr array_view<const T> subview(size_t pos, size_t n = npos) const noexcept ywlib_pre(pos <= N) {
    return array_view<const T>(data() + pos, yw::min(n, N - pos));
  }
};

///--------------------------------------------------------------------------///
/// MARK: array<T, npos>

namespace internal {
inline constexpr size_t _array_preferred_capacity(size_t Size) noexcept {
  return yw::max(Size + 1, 2 * std::bit_ceil(Size), size_t(256));
}
} // namespace internal

template<typename T> struct array<T, npos> {
public:
  const_property<size_t, array> size;
  const_property<uninitialized_heap<T[]>, array> heap;

protected:
  constexpr array(is_none auto, size_t n)
    : heap(internal::_array_preferred_capacity(n)) {}

public:
  constexpr ~array() noexcept { std::destroy_n(heap.ref().get(), size()); }

  constexpr array() = default;
  constexpr array(array&& o) noexcept
    : size(exchange(o.size.ref(), 0)), heap(move(o.heap.ref())) {}

  constexpr array& operator=(array&& o) noexcept {
    if (this == &o) return *this;
    std::destroy_n(heap.ref().get(), size());
    size = exchange(o.size.ref(), {});
    heap = move(o.heap.ref());
    return *this;
  }

  constexpr array(const array& o) noexcept(nt_constructible<T, const T&>) : array(none(), o.size()) {
    std::uninitialized_copy_n(o.data(), o.size(), heap.ref().get());
    size = o.size();
  }

  constexpr array& operator=(const array& o) noexcept(nt_constructible<T, const T&>) {
    if (this == &o) return *this;
    std::destroy_n(heap.ref().get(), size());
    const auto n = o.size();
    size = 0;
    if (capacity() < n) {
      heap.ref().allocate(internal::_array_preferred_capacity(n));
    }
    std::uninitialized_copy_n(o.data(), n, heap.ref().get());
    size = n;
    return *this;
  }

  constexpr array(size_t n) noexcept(nt_constructible<T>) : array(none(), n) {
    std::uninitialized_default_construct_n(heap.ref().get(), n);
    size = n;
  }

  constexpr array(size_t n, const T& v) noexcept(nt_constructible<T, const T&>) : array(none(), n) {
    std::uninitialized_fill_n(heap.ref().get(), n, v);
    size = n;
  }

  template<input_iterator I, sized_sentinel_for<I> S> requires constructible<T, iter_value_t<I>>
  constexpr array(I i, S s) noexcept(nt_constructible<T, iter_value_t<I>>) ywlib_pre(s - i >= 0)
    : array(none(), s - i) {
    const auto n = size_t(s - i);
    std::ranges::uninitialized_copy(i, s, data(), data() + n);
    size = n;
  }

  template<sized_range R> requires std::ranges::input_range<R> && constructible<T, iter_value_t<R>>
  constexpr array(R&& r) noexcept(nt_constructible<T, iter_value_t<R>>)
    : array(std::ranges::begin(r), std::ranges::end(r)) {}

  template<sized_range R> requires std::ranges::input_range<R> && constructible<T, iter_value_t<R>>
  constexpr array& operator=(R&& r) noexcept(nt_constructible<T, iter_value_t<R>>) {
    return *this = array<T>(static_cast<R&&>(r));
  }

  constexpr bool empty() const noexcept { return size() == 0; }
  constexpr size_t capacity() const noexcept { return heap.cref().size(); }
  constexpr T* data() noexcept { return heap.ref().get(); }
  constexpr const T* data() const noexcept { return heap.cref().get(); }
  constexpr T* begin() noexcept { return data(); }
  constexpr const T* begin() const noexcept { return data(); }
  constexpr T* end() noexcept { return data() + size(); }
  constexpr const T* end() const noexcept { return data() + size(); }
  constexpr T& operator[](size_t i) noexcept ywlib_pre(i < size.cref()) { return heap.ref()[i]; }
  constexpr const T& operator[](size_t i) const noexcept ywlib_pre(i < size()) { return heap.cref()[i]; }
  constexpr T& front() noexcept ywlib_pre(size() != 0) { return data()[0]; }
  constexpr const T& front() const noexcept ywlib_pre(size() != 0) { return data()[0]; }
  constexpr T& back() noexcept ywlib_pre(size() != 0) { return data()[size() - 1]; }
  constexpr const T& back() const noexcept ywlib_pre(size() != 0) { return data()[size() - 1]; }

  constexpr void clear() noexcept {
    for (size_t i = 0; i < size(); ++i) heap.ref()[i].~T();
    size = 0;
  }

  constexpr void reserve(size_t n) noexcept {
    if (n <= capacity()) return;
    const auto new_capacity = internal::_array_preferred_capacity(n);
    auto new_heap = uninitialized_heap<T[]>(new_capacity);
    std::uninitialized_move_n(heap.ref().get(), size(), new_heap.get());
    std::destroy_n(heap.ref().get(), size());
    heap = move(new_heap);
  }

  constexpr void resize(size_t n) noexcept requires constructible<T> {
    reserve(n);
    if (n > size()) std::uninitialized_value_construct_n(heap.ref().get() + size(), n - size());
    else if (n < size()) std::destroy_n(heap.ref().get() + n, size() - n);
    size = n;
  }

  constexpr void pop_back() noexcept {
    if (size() == 0) return;
    const auto new_size = size() - 1;
    std::destroy_at(heap.ref().get() + new_size);
    size = new_size;
  }

  constexpr void push_back(const T& v) noexcept {
    reserve(size() + 1);
    new (&heap.ref()[size()]) T(v);
    size = size() + 1;
  }
  constexpr void push_back(T&& v) noexcept {
    reserve(size() + 1);
    new (&heap.ref()[size()]) T(move(v));
    size = size() + 1;
  }

  template<typename... As> requires constructible<T, As...>
  constexpr void emplace_back(As&&... as) noexcept(nt_constructible<T, As...>) {
    reserve(size() + 1);
    new (&heap.ref()[size()]) T(static_cast<As&&>(as)...);
    size = size() + 1;
  }

  constexpr void erase(T* iter) {
    if (iter < data() || iter >= data() + size()) return;
    erase(iter, iter + 1);
  }

  constexpr void erase(size_t i) { erase(data() + i); }

  constexpr void erase(T* first, T* last) {
    if (first < data()) first = data();
    if (last > data() + size()) last = data() + size();
    if (first >= last) return;
    const auto count = size_t(last - first);
    for (T* p = first; p != last; ++p) p->~T();
    for (T *p = last, *q = end(); p != q; ++p, ++first) {
      try {
        new (first) T(move(*p));
      } catch (...) {
        std::destroy(p, q);
        size = size_t(first - data());
        throw;
      }
      p->~T();
    }
    size = size() - count;
  }

  constexpr void erase(size_t first, size_t last) { erase(data() + first, data() + last); }

  constexpr operator array_view<T>() noexcept { return array_view<T>(data(), size()); }
  constexpr operator array_view<const T>() const noexcept { return array_view<const T>(data(), size()); }
  constexpr array_view<T> view() noexcept { return array_view<T>(data(), size()); }
  constexpr array_view<const T> view() const noexcept { return array_view<const T>(data(), size()); }

  constexpr void swap(array& other) noexcept {
    std::ranges::swap(size.ref(), other.size.ref());
    std::ranges::swap(heap.ref(), other.heap.ref());
  }
};

template<typename T> array(size_t, const T&) -> array<T, npos>;
template<input_iterator I, sized_sentinel_for<I> S> array(I, S) -> array<iter_value_t<I>, npos>;
template<sized_range R> requires std::ranges::input_range<R> array(R&&) -> array<iter_value_t<R>, npos>;
} // namespace yw
