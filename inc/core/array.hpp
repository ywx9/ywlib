/// \file core/array.hpp

#pragma once

#ifndef YWLIB
#include <array>
#include <vector>
#else
import std;
#endif

#include "core.hpp"
#include "range.hpp"

export namespace yw {


/// class to represent an array; fixed-size if `N != npos`, otherwise dynamic-size
template<typename T, nat N = npos> class array {
public:

  T _data[N]{};

  /// number of elements
  static constexpr nat count = N;

  /// type of elements
  using value_type = T;

  /// assignment to copy the elements from a bounded array
  constexpr array& operator=(const T (&a)[N]) noexcept {
    for (nat i = 0; i < N; ++i) _data[i] = a[i];
    return *this;
  }

  /// conversion operator to lvalue reference to the array
  constexpr operator add_lvref<T[N]>() noexcept { return _data; }

  /// conversion operator to const lvalue reference to the array
  constexpr operator add_lvref<const T[N]>() const noexcept { return _data; }

  /// conversion operator to `std::basic_string_view<T>`
  constexpr operator std::basic_string_view<T>() const noexcept
    requires character<T> { return std::basic_string_view<T>(_data, N); }

  /// index operator to access the element
  constexpr T& operator[](nat i) noexcept { return _data[i]; }

  /// index operator to access the element
  constexpr const T& operator[](nat i) const noexcept { return _data[i]; }

  /// returns the number of elements
  constexpr nat size() const noexcept { return N; }

  /// checks if the array is empty
  constexpr bool empty() const noexcept { return N == 0; }

  /// returns the pointer to the first element
  constexpr T* data() noexcept { return _data; }

  /// returns the pointer to the first element
  constexpr const T* data() const noexcept { return _data; }

  /// returns the iterator to the first element
  constexpr T* begin() noexcept { return _data; }

  /// returns the iterator to the first element
  constexpr const T* begin() const noexcept { return _data; }

  /// returns the iterator to the end of the array
  constexpr T* end() noexcept { return _data + N; }

  /// returns the iterator to the end of the array
  constexpr const T* end() const noexcept { return _data + N; }

  /// returns the reference to the first element
  constexpr T& front() noexcept { return _data[0]; }

  /// returns the reference to the first element
  constexpr const T& front() const noexcept { return _data[0]; }

  /// returns the reference to the last element
  constexpr T& back() noexcept { return _data[N - 1]; }

  /// returns the reference to the last element
  constexpr const T& back() const noexcept { return _data[N - 1]; }

  /// `get` function
  template<nat I> requires (I < N)
  constexpr T& get() & noexcept { return _data[I]; }

  /// `get` function
  template<nat I> requires (I < N)
  constexpr const T& get() const & noexcept { return _data[I]; }

  /// `get` function
  template<nat I> requires (I < N)
  constexpr T&& get() && noexcept { return mv(_data[I]); }

  /// `get` function
  template<nat I> requires (I < N)
  constexpr const T&& get() const && noexcept { return mv(_data[I]); }

}; ///////////////////////////////////////////////////////////////////////////// class array<T, N>


/// specialization of `array` for empty array
template<typename T> class array<T, 0> {
public:

  /// number of elements
  static constexpr nat count = 0;

  /// type of elements
  using value_type = T;

  /// conversion operator to `std::basic_string_view<T>`
  constexpr operator std::basic_string_view<T>() const noexcept
    requires character<T> { return std::basic_string_view<T>(); }

  /// returns the number of elements
  constexpr nat size() const noexcept { return 0; }

  /// checks if the array is empty
  constexpr bool empty() const noexcept { return true; }

  /// returns the pointer to the first element
  constexpr T* data() noexcept { return nullptr; }

  /// returns the pointer to the first element
  constexpr const T* data() const noexcept { return nullptr; }

  /// returns the iterator to the first element
  constexpr T* begin() noexcept { return nullptr; }

  /// returns the iterator to the first element
  constexpr const T* begin() const noexcept { return nullptr; }

  /// returns the iterator to the end of the array
  constexpr T* end() noexcept { return nullptr; }

  /// returns the iterator to the end of the array
  constexpr const T* end() const noexcept { return nullptr; }

}; ///////////////////////////////////////////////////////////////////////////// class array<T, 0>


/// specialization of `array` for dynamic-size array
template<typename T> class array<T, npos> : std::vector<T> {
public:

  /// default constructor
  constexpr array() noexcept = default;

  /// constructor with the rvalue reference to `std::vector<T>`
  constexpr array(std::vector<T>&& v)
    noexcept(nt_constructible<std::vector<T>, std::vector<T>)
    : std::vector<T>(mv(v)) {}

  /// constructor with the initializer list
  constexpr array(std::initializer_list<T> il)
    : std::vector<T>(mv(il)) {}

  /// constructor with the number of elements
  constexpr array(nat n)
    : std::vector<T>(n) {}

  /// constructor with the number of elements and the value
  constexpr array(nat n, const T& v)
    : std::vector<T>(n, v) {}

  /// constructor with the iterator range
  template<fwd_iterator I>
  constexpr array(I first, I last)
    : std::vector<T>(first, last) {}

  /// constructor with the iterator range
  template<fwd_iterator I, sentinel_for<I> S> requires (!same_as<I, S>)
  constexpr array(I first, S last)
    : std::vector<T>(std::common_iterator<It, Se>(first),
                     std::common_iterator<It, Se>(last)) {}

  /// constructor with the iterator range
  template<range Rg> constexpr array(Rg&& r)
    : std::vector<T>(begin(fwd<Rg>(r)), end(fwd<Rg>(r))) {}

  /// conversion operator to `std::basic_string_view<T>`
  constexpr operator std::basic_string_view<T>() const noexcept
    requires character<T> { return std::basic_string_view<T>(data(), size()); }

}; ///////////////////////////////////////////////////////////////////////////// class array<T, npos>


// deduction guides

template<typename T, convertible_to<T>... Ts>
array(T, Ts...) -> array<T, 1 + sizeof...(Ts)>;

template<typename T, nat N>
array(const T (&)[N]) -> array<T, N>;

template<typename T>
array(nat, const T&) -> array<T, npos>;

template<iterator I, sentinel_for<I> S>
array(I, S) -> array<iter_value<I>>;

template<range Rg>
array(Rg&&) -> array<iter_value<Rg>, npos>;


} ////////////////////////////////////////////////////////////////////////////// namespace yw


namespace std {

// tuple_size

template<typename T, size_t N>
struct tuple_size<yw::array<T, N>> : std::integral_constant<size_t, N> {};

// tuple_element

template<size_t I, typename T, size_t N>
struct tuple_element<I, yw::array<T, N>> : type_identity<T> {};

} ////////////////////////////////////////////////////////////////////////////// namespace std
