/// \file vector/vector2.hpp

#pragma once

#ifndef YWLIB
#include <cmath>
#else
import std;
#endif

#include "../core.hpp"

namespace yw {


/// struct to represent a 2D vector
struct vector2 {

  /// number of elements
  static constexpr nat count = 2;

  /// element type
  using value_type = fat4;

  /// first element
  fat4 x{};

  /// second element
  fat4 y{};

  /// default constructor
  constexpr vector2() noexcept = default;

  /// constructor with a value to fill
  explicit constexpr vector2(const fat4 Fill) noexcept : x(Fill), y(Fill) {}

  /// constructor with a value to fill
  explicit constexpr vector2(numeric auto&& Fill) noexcept
    requires (!same_as<remove_cvref<decltype(Fill)>, fat4>)
    : vector2(fat4(Fill)) {}

  /// constructor with x and y
  constexpr vector2(numeric auto&& X, numeric auto&& Y) noexcept
    : x(fat4(X)), y(fat4(Y)) {}

  /// number of elements
  constexpr nat size() const noexcept { return count; }

  /// obtains the pointer to the first element
  constexpr fat4* data() noexcept { return &x; }

  /// obtains the pointer to the first element
  constexpr const fat4* data() const noexcept { return &x; }

  /// accesses the first element
  constexpr fat4& operator[](nat Index) {
    if (is_cev) {
      if (Index == 0) return x;
      else if (Index == 1) return y;
      else cannot_be_constant_evaluated();
    } else return data()[Index];
  }

  /// accesses the first element
  constexpr fat4 operator[](nat Index) const {
    if (is_cev) {
      if (Index == 0) return x;
      else if (Index == 1) return y;
      else cannot_be_constant_evaluated();
    } else return data()[Index];
  }

  /// obtains the iterator to the first element
  constexpr fat4* begin() noexcept { return data(); }

  /// obtains the iterator to the first element
  constexpr const fat4* begin() const noexcept { return data(); }

  /// obtains the iterator to the end
  constexpr fat4* end() noexcept { return data() + count; }

  /// obtains the iterator to the end
  constexpr const fat4* end() const noexcept { return data() + count; }

  /// equality comparison
  friend constexpr bool operator==(const vector2& a, const vector2& b)
    noexcept { return a.x == b.x && a.y == b.y; }

  /// three-way comparison
  friend constexpr auto operator<=>(
    const vector2& a, const vector2& b) noexcept {
    if (const auto c = a.x <=> b.x; c != 0) return c;
    else return a.y <=> b.y;
  }

  /// unary plus
  friend constexpr vector2 operator+(const vector2& a)
    noexcept { return a; }

  /// unary minus
  friend constexpr vector2 operator-(const vector2& a)
    noexcept { return {-a.x, -a.y}; }

  /// addition
  friend constexpr vector2 operator+(const vector2& a, const vector2& b)
    noexcept { return {a.x + b.x, a.y + b.y}; }

  /// subtraction
  friend constexpr vector2 operator-(const vector2& a, const vector2& b)
    noexcept { return {a.x - b.x, a.y - b.y}; }

  /// multiplication
  friend constexpr vector2 operator*(const vector2& a, const vector2& b)
    noexcept { return {a.x * b.x, a.y * b.y}; }

  /// multiplication
  friend constexpr vector2 operator*(const vector2& a, const fat4& b)
    noexcept { return {a.x * b, a.y * b}; }

  /// multiplication
  friend constexpr vector2 operator*(const fat4& a, const vector2& b)
    noexcept { return b * a; }

  /// division
  friend constexpr vector2 operator/(const vector2& a, const vector2& b)
    noexcept { return {a.x / b.x, a.y / b.y}; }

  /// division
  friend constexpr vector2 operator/(const vector2& a, const fat4& b)
    noexcept { return a * (1.f / b); }

  /// addition assignment
  constexpr vector2& operator+=(const vector2& a)
    noexcept { return x += a.x, y += a.y, *this; }

  /// subtraction assignment
  constexpr vector2& operator-=(const vector2& a)
    noexcept { return x -= a.x, y -= a.y, *this; }

  /// multiplication assignment
  constexpr vector2& operator*=(const vector2& a)
    noexcept { return x *= a.x, y *= a.y, *this; }

  /// multiplication assignment
  constexpr vector2& operator*=(const fat4& a)
    noexcept { return x *= a, y *= a, *this; }

  /// division assignment
  constexpr vector2& operator/=(const vector2& a)
    noexcept { return x /= a.x, y /= a.y, *this; }

  /// division assignment
  constexpr vector2& operator/=(const fat4& a)
    noexcept { return operator*=(1.f / a); }

  /// returns the absolute value
  friend constexpr vector2 abs(const vector2& v)
    noexcept { return {std::abs(v.x), std::abs(v.y)}; }

  /// returns the dot product
  friend constexpr fat4 dot(const vector2& a, const vector2& b)
    noexcept { return a.x * b.x + a.y * b.y; }

  /// returns the dot product of the first three elements
  friend constexpr fat4 dot3(const vector2& a, const vector2& b)
    noexcept { return dot(a, b); }

  /// returns the cross product
  friend constexpr fat4 cross(const vector2& a, const vector2& b)
    noexcept { return a.x * b.y - a.y * b.x; }

  /// returns the length
  friend fat4 length(const vector2& v)
    noexcept { return std::sqrt(dot(v, v)); }

  /// returns the length of the first three elements
  friend fat4 length3(const vector2& v)
    noexcept { return length(v); }

  /// returns the normalized vector
  friend vector2 normalize(const vector2& v)
    noexcept { return v / length(v); }

  /// returns the normalized vector of the first three elements
  friend vector2 normalize3(const vector2& v)
    noexcept { return normalize(v); }
};


} ////////////////////////////////////////////////////////////////////////////// namespace yw

namespace std {

// tuple size

template<> struct tuple_size<yw::vector2> : integral_constant<size_t, 2> {};

// tuple element

template<size_t I> struct tuple_element<I, yw::vector2> : type_identity<float> {};

// formatter

template<typename Ct> struct formatter<yw::vector2, Ct> : formatter<basic_string<Ct>, Ct> {
  static constexpr Ct text[]{'(', '{', '}', ',', '{', '}', ')', 0};
  auto format(const yw::vector2& v, auto& ctx) const {
    return formatter<basic_string<Ct>, Ct>::format(std::format(text, v.x, v.y), ctx);
  }
};

} ////////////////////////////////////////////////////////////////////////////// std
