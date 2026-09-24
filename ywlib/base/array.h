#pragma once
#include <core/core.h>
#include <core/property.h>

namespace yw {

template<typename T> class array_view {
public:
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
template<contiguous_iterator It, sized_sentinel_for<It> Se> array_view(It, Se) -> array_view<iter_value_t<It>>;
template<contiguous_range Rg> array_view(Rg&&) -> array_view<iter_value_t<Rg>>;

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
  const_property<size_t, array> size;
  const_property<size_t, array> capacity;
  const_property<T*, array> data;

  constexpr ~array() noexcept {
    std::destroy_n(data(), size());
    yw::deallocate(data());
  }

  constexpr array() noexcept : size(0), capacity(0), data(nullptr) {}

  constexpr array(const array& o) noexcept
    : size(o.size()), capacity(internal::_array_preferred_capacity(o.size())), data(yw::allocate<T>(capacity())) {
    std::uninitialized_copy_n(o.data(), o.size(), data());
  }

  constexpr array& operator=(const array& o) noexcept {
    if (this != &o) {
      array copy(o);
      swap(copy);
    }
    return *this;
  }

  constexpr array(array&& o) noexcept : size(o.size()), capacity(o.capacity()), data(o.data()) {
    o.size = 0, o.capacity = 0, o.data = nullptr;
  }

  constexpr array& operator=(array&& o) noexcept {
    if (this != &o) {
      std::destroy_n(data(), size());
      yw::deallocate(data());
      size = o.size();
      capacity = o.capacity();
      data = o.data();
      o.size = 0, o.capacity = 0, o.data = nullptr;
    }
    return *this;
  }

  constexpr array(size_t n) noexcept
    : size(n), capacity(internal::_array_preferred_capacity(n)), data(yw::allocate<T>(capacity())) {
    std::uninitialized_default_construct_n(data(), size());
  }

  constexpr array(size_t n, const T& v) noexcept
    : size(n), capacity(internal::_array_preferred_capacity(n)), data(yw::allocate<T>(capacity())) {
    std::uninitialized_fill_n(data(), n, v);
  }

  template<input_iterator I, sized_sentinel_for<I> S> //
  constexpr array(I i, S s) noexcept
    : size(std::ranges::distance(i, s)), capacity(internal::_array_preferred_capacity(size())),
      data(yw::allocate<T>(capacity())) {
    std::uninitialized_copy_n(i, size(), data());
  }

  template<sized_range R> requires std::ranges::input_range<R> //
  constexpr array(R&& r) noexcept : array(std::ranges::begin(r), std::ranges::end(r)) {}

  template<sized_range R> requires std::ranges::input_range<R> //
  constexpr array& operator=(R&& r) noexcept {
    array copy(static_cast<R&&>(r));
    swap(copy);
    return *this;
  }

  constexpr bool empty() const noexcept { return size() == 0; }
  constexpr T* begin() noexcept { return data(); }
  constexpr const T* begin() const noexcept { return data(); }
  constexpr T* end() noexcept { return data() + size(); }
  constexpr const T* end() const noexcept { return data() + size(); }
  constexpr T& operator[](size_t i) noexcept { return data()[i]; }
  constexpr const T& operator[](size_t i) const noexcept { return data()[i]; }
  constexpr T& front() noexcept { return data()[0]; }
  constexpr const T& front() const noexcept { return data()[0]; }
  constexpr T& back() noexcept { return data()[size() - 1]; }
  constexpr const T& back() const noexcept { return data()[size() - 1]; }

  constexpr void clear() noexcept {
    std::destroy_n(data(), size());
    size = 0;
  }

  constexpr void reserve(size_t n) noexcept {
    if (n <= capacity()) return;
    const auto new_capacity = internal::_array_preferred_capacity(n);
    T* new_data = yw::allocate<T>(new_capacity);
    std::uninitialized_move_n(data(), size(), new_data);
    std::destroy_n(data(), size());
    yw::deallocate(data());
    data = new_data;
    capacity = new_capacity;
  }

  constexpr void resize(size_t n) noexcept {
    const auto old_size = size();
    if (n < old_size) {
      std::destroy_n(data() + n, old_size - n);
      size = n;
      return;
    }
    reserve(n);
    std::uninitialized_default_construct_n(data() + old_size, n - old_size);
    size = n;
  }

  constexpr void pop_back() noexcept {
    if (size() > 0) {
      std::destroy_at(data() + size() - 1);
      size = size() - 1;
    }
  }

  constexpr array& push_back(const T& v) noexcept {
    reserve(size() + 1);
    std::construct_at(data() + size(), v);
    size = size() + 1;
    return *this;
  }
  constexpr array& push_back(T&& v) noexcept {
    reserve(size() + 1);
    std::construct_at(data() + size(), move(v));
    size = size() + 1;
    return *this;
  }

  template<typename... As> requires constructible<T, As...>
  constexpr T& emplace_back(As&&... as) noexcept(nt_constructible<T, As...>) {
    reserve(size() + 1);
    std::construct_at(data() + size(), static_cast<As&&>(as)...);
    size = size() + 1;
    return data()[size() - 1];
  }

  constexpr array& append(array_view<T> av) {
    if (av.empty()) return *this;
    const bool overlaps = data() && av.data() < data() + size() && data() < av.data() + av.size();
    const auto new_size = size() + av.size();
    const bool need_realloc = overlaps || new_size > capacity();
    if (need_realloc) {
      const auto new_capacity = internal::_array_preferred_capacity(new_size);
      const auto new_data = yw::allocate<T>(new_capacity);
      std::uninitialized_copy_n(data(), size(), new_data);
      std::uninitialized_copy_n(av.data(), av.size(), new_data + size());
      std::destroy_n(data(), size());
      yw::deallocate(data());
      data = new_data;
      size = new_size;
      capacity = new_capacity;
    } else {
      std::uninitialized_copy_n(av.data(), av.size(), data() + size());
      size = new_size;
    }
    return *this;
  }

  constexpr operator array_view<T>() noexcept { return array_view<T>(data(), size()); }
  constexpr operator array_view<const T>() const noexcept { return array_view<const T>(data(), size()); }
  constexpr array_view<T> view() noexcept { return array_view<T>(data(), size()); }
  constexpr array_view<const T> view() const noexcept { return array_view<const T>(data(), size()); }

  constexpr void swap(array& other) noexcept {
    std::ranges::swap(size.ref(), other.size.ref());
    std::ranges::swap(capacity.ref(), other.capacity.ref());
    std::ranges::swap(data.ref(), other.data.ref());
  }
};

template<typename T> array(size_t, const T&) -> array<T, npos>;
template<input_iterator I, sized_sentinel_for<I> S> array(I, S) -> array<iter_value_t<I>, npos>;
template<sized_range R> requires std::ranges::input_range<R> array(R&&) -> array<iter_value_t<R>, npos>;
} // namespace yw
