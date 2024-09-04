/// \file vector/vector3.hpp

#pragma once

#include "vector2.hpp"

namespace yw {


/// struct to represent a 3D vector
struct vector3 {

  /// number of elements
  static constexpr nat count = 3;

  /// element type
  using value_type = fat4;

  /// first element
  fat4 x{};

  /// second element
  fat4 y{};

  /// third element
  fat4 z{};

  /// default constructor
  constexpr vector3() noexcept = default;

  /// constructor with a value to fill
  explicit constexpr vector3(const fat4 Fill) noexcept
    : x(Fill), y(Fill), z{Fill} {}

  /// constructor with a value to fill
  explicit constexpr vector3(numeric auto&& Fill) noexcept
    requires (!same_as<remove_cvref<decltype(Fill)>, fat4>)
    : vector3(fat4(Fill)) {}

  /// constructor with x and y
  constexpr vector3(numeric auto&& X, numeric auto&& Y) noexcept
    : x(fat4(X)), y(fat4(Y)) {}

  /// constructor with x, y, and z
  constexpr vector3(
    numeric auto&& X, numeric auto&& Y, numeric auto&& Z) noexcept
    : x(fat4(X)), y(fat4(Y)), z(fat4(Z)) {}

  /// conversion to `vector2`
  constexpr operator vector2() const noexcept { return {x, y}; }

  /// conversion to __m128
  operator __m128() const noexcept { return _mm_loadu_ps(&x); }

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
      else if (Index == 2) return z;
      else cannot_be_constant_evaluated();
    } else return data()[Index];
  }

  /// accesses the first element
  constexpr fat4 operator[](nat Index) const {
    if (is_cev) {
      if (Index == 0) return x;
      else if (Index == 1) return y;
      else if (Index == 2) return z;
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
  friend constexpr bool operator==(const vector3& a, const vector3& b)
    noexcept { return a.x == b.x && a.y == b.y && a.z == b.z; }

  /// three-way comparison
  friend constexpr auto operator<=>(
    const vector3& a, const vector3& b) noexcept {
    if (auto c = a.x <=> b.x; c != 0) return c;
    else if (c = a.y <=> b.y; c != 0) return c;
    else return a.z <=> b.z;
  }

  /// unary plus
  friend constexpr vector3 operator+(const vector3& a)
    noexcept { return a; }

  /// unary minus
  friend constexpr vector3 operator-(const vector3& a)
    noexcept { return {-a.x, -a.y, -a.z}; }

  /// addition
  friend constexpr vector3 operator+(const vector3& a, const vector3& b)
    noexcept { return {a.x + b.x, a.y + b.y, a.z + b.z}; }

  /// subtraction
  friend constexpr vector3 operator-(const vector3& a, const vector3& b)
    noexcept { return {a.x - b.x, a.y - b.y, a.z - b.z}; }

  /// multiplication
  friend constexpr vector3 operator*(const vector3& a, const vector3& b)
    noexcept { return {a.x * b.x, a.y * b.y, a.z * b.z}; }

  /// multiplication
  friend constexpr vector3 operator*(const vector3& a, const fat4& b)
    noexcept { return {a.x * b, a.y * b, a.z * b}; }

  /// multiplication
  friend constexpr vector3 operator*(const fat4& a, const vector3& b)
    noexcept { return b * a; }

  /// division
  friend constexpr vector3 operator/(const vector3& a, const vector3& b)
    noexcept { return {a.x / b.x, a.y / b.y, a.z / b.z}; }

  /// division
  friend constexpr vector3 operator/(const vector3& a, const fat4& b)
    noexcept { return a * (1.f / b); }

  /// addition assignment
  constexpr vector3& operator+=(const vector3& a)
    noexcept { return x += a.x, y += a.y, z += a.z, *this; }

  /// subtraction assignment
  constexpr vector3& operator-=(const vector3& a)
    noexcept { return x -= a.x, y -= a.y, z -= a.z, *this; }

  /// multiplication assignment
  constexpr vector3& operator*=(const vector3& a)
    noexcept { return x *= a.x, y *= a.y, z *= a.z, *this; }

  /// multiplication assignment
  constexpr vector3& operator*=(const fat4& a)
    noexcept { return x *= a, y *= a, z *= a, *this; }

  /// division assignment
  constexpr vector3& operator/=(const vector3& a)
    noexcept { return x /= a.x, y /= a.y, z /= a.z, *this; }

  /// division assignment
  constexpr vector3& operator/=(const fat4& a)
    noexcept { return operator*=(1.f / a); }

  /// returns the absolute value
  friend constexpr vector3 abs(const vector3& v)
    noexcept { return {std::abs(v.x), std::abs(v.y), std::abs(v.z)}; }

  /// returns the dot product
  friend constexpr fat4 dot(const vector3& a, const vector3& b) noexcept {
    if (!is_cev) {
      return _mm_cvtss_f32(_mm_dp_ps(a, b, 0x7f));
    } return a.x * b.x + a.y * b.y;
  }

  /// returns the dot product of the first three elements
  friend constexpr fat4 dot3(const vector3& a, const vector3& b)
    noexcept { return dot(a, b); }

  /// returns the cross product
  friend constexpr vector3 cross(const vector3& a, const vector3& b) noexcept {
    return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x};
  }

  /// returns the length
  friend fat4 length(const vector3& v)
    noexcept { return std::sqrt(dot(v, v)); }

  /// returns the length of the first three elements
  friend fat4 length3(const vector3& v)
    noexcept { return length(v); }

  /// returns the normalized vector
  friend vector3 normalize(const vector3& v)
    noexcept { return v / length(v); }

  /// returns the normalized vector of the first three elements
  friend vector3 normalize3(const vector3& v)
    noexcept { return normalize(v); }

};


} ////////////////////////////////////////////////////////////////////////////// namespace yw

namespace std {

// tuple size

template<> struct tuple_size<yw::vector3> : integral_constant<size_t, 3> {};

// tuple element

template<size_t I> struct tuple_element<I, yw::vector3> : type_identity<float> {};

// formatter

template<typename Ct> struct formatter<yw::vector3, Ct> : formatter<basic_string<Ct>, Ct> {
  static constexpr Ct text[]{'(', '{', '}', ',', '{', '}', ',', '{', '}', ')', 0};
  auto format(const yw::vector3& v, auto& ctx) const {
    return formatter<basic_string<Ct>, Ct>::format(std::format(text, v.x, v.y, v.z), ctx);
  }
};

} ////////////////////////////////////////////////////////////////////////////// std
