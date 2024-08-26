/// \file vector/vector.hpp

#pragma once

#include "vector3.hpp"
#include "xvector.hpp"

namespace yw {


/// struct to represent a 4D vector
struct vector {

  /// number of elements
  static constexpr nat count = 4;

  /// element type
  using value_type = fat4;

  /// first element
  fat4 x{};

  /// second element
  fat4 y{};

  /// third element
  fat4 z{};

  /// fourth element
  fat4 w{};

  /// default constructor
  constexpr vector() noexcept = default;

  /// constructor with a value to fill
  explicit constexpr vector(const fat4 Fill) noexcept
    : x(Fill), y(Fill), z{Fill}, w{Fill} {}

  /// constructor with a value to fill
  explicit constexpr vector(numeric auto&& Fill) noexcept
    requires (!same_as<remove_cvref<decltype(Fill)>, fat4>)
    : vector(fat4(Fill)) {}

  /// constructor with x and y
  constexpr vector(numeric auto&& X, numeric auto&& Y) noexcept
    : x(fat4(X)), y(fat4(Y)) {}

  /// constructor with x, y, and z
  constexpr vector(
    numeric auto&& X, numeric auto&& Y, numeric auto&& Z) noexcept
    : x(fat4(X)), y(fat4(Y)), z(fat4(Z)) {}

  /// constructor with x, y, z, and w
  constexpr vector(
    numeric auto&& X, numeric auto&& Y, numeric auto&& Z, numeric auto&& W)
    noexcept : x(fat4(X)), y(fat4(Y)), z(fat4(Z)), w(fat4(W)) {}

  /// constructor with `vector2`
  constexpr vector(const vector2& v) noexcept : x(v.x), y(v.y) {}

  /// constructor with `vector2` and z
  constexpr vector(const vector2& v, numeric auto&& Z) noexcept
    : x(v.x), y(v.y), z(fat4(Z)) {}

  /// constructor with `vector2`, z, and w
  constexpr vector(
    const vector2& v, numeric auto&& Z, numeric auto&& W) noexcept
    : x(v.x), y(v.y), z(fat4(Z)), w(fat4(W)) {}

  /// constructor with two `vector2`
  constexpr vector(const vector2& v1, const vector2& v2) noexcept
    : x(v1.x), y(v1.y), z(v2.x), w(v2.y) {}

  /// constructor with `vector3`
  constexpr vector(const vector3& v) noexcept
    : x(v.x), y(v.y), z(v.z) {}

  /// constructor with `vector3` and w
  constexpr vector(const vector3& v, numeric auto&& W) noexcept
    : x(v.x), y(v.y), z(v.z), w(fat4(W)) {}

  /// constructor with `xvector`
  vector(const xvector& v) noexcept { xvstore(&x, v); }

  /// conversion to `vector2`
  constexpr operator vector2() const noexcept { return {x, y}; }

  /// conversion to `vector3`
  constexpr operator vector3() const noexcept { return {x, y, z}; }

  /// conversion to `xvector`
  operator xvector() const noexcept { return xvload(&x); }

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
      else if (Index == 3) return w;
      else cannot_be_constant_evaluated();
    } else return data()[Index];
  }

  /// accesses the first element
  constexpr fat4 operator[](nat Index) const {
    if (is_cev) {
      if (Index == 0) return x;
      else if (Index == 1) return y;
      else if (Index == 2) return z;
      else if (Index == 3) return w;
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
  friend constexpr bool operator==(const vector& a, const vector& b)
    noexcept { return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w; }

  /// three-way comparison
  friend constexpr auto operator<=>(
    const vector& a, const vector& b) noexcept {
    if (auto c = a.x <=> b.x; c != 0) return c;
    else if (c = a.y <=> b.y; c != 0) return c;
    else if (c = a.z <=> b.z; c != 0) return c;
    else return a.w <=> b.w;
  }

  /// unary plus
  friend constexpr vector operator+(const vector& a)
    noexcept { return a; }

  /// unary minus
  friend constexpr vector operator-(const vector& a) noexcept {
    if (is_cev) return {xvneg(a)};
    else return {-a.x, -a.y, -a.z, -a.w}; }

  /// addition
  friend constexpr vector operator+(const vector& a, const vector& b)
    noexcept { return {a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w}; }

  /// subtraction
  friend constexpr vector operator-(const vector& a, const vector& b)
    noexcept { return {a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w}; }

  /// multiplication
  friend constexpr vector operator*(const vector& a, const vector& b)
    noexcept { return {a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w}; }

  /// multiplication
  friend constexpr vector operator*(const vector& a, const fat4& b)
    noexcept { return {a.x * b, a.y * b, a.z * b, a.w * b}; }

  /// multiplication
  friend constexpr vector operator*(const fat4& a, const vector& b)
    noexcept { return b * a; }

  /// division
  friend constexpr vector operator/(const vector& a, const vector& b)
    noexcept { return {a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w}; }

  /// division
  friend constexpr vector operator/(const vector& a, const fat4& b)
    noexcept { return a * (1.f / b); }

  /// addition assignment
  constexpr vector& operator+=(const vector& a)
    noexcept { return x += a.x, y += a.y, z += a.z, w += a.w, *this; }

  /// subtraction assignment
  constexpr vector& operator-=(const vector& a)
    noexcept { return x -= a.x, y -= a.y, z -= a.z, w -= a.w, *this; }

  /// multiplication assignment
  constexpr vector& operator*=(const vector& a)
    noexcept { return x *= a.x, y *= a.y, z *= a.z, w *= a.w, *this; }

  /// multiplication assignment
  constexpr vector& operator*=(const fat4& a)
    noexcept { return x *= a, y *= a, z *= a, w *= a, *this; }

  /// division assignment
  constexpr vector& operator/=(const vector& a)
    noexcept { return x /= a.x, y /= a.y, z /= a.z, w /= a.w, *this; }

  /// division assignment
  constexpr vector& operator/=(const fat4& a)
    noexcept { return operator*=(1.f / a); }

  /// dot product
  friend constexpr fat4 dot(const vector& a, const vector& b)
    noexcept { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; }

  /// cross product
  friend constexpr vector cross(const vector& a, const vector& b) noexcept {
    return {a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x};
  }

  /// squared length
  constexpr fat4 length2() const
    noexcept { return x * x + y * y + z * z + w * w; }

  /// length
  fat4 length() const noexcept { return sqrt(length2()); }

  /// normalizes the vector
  vector normalize() const noexcept {
    return *this / length();
  }

};


} ////////////////////////////////////////////////////////////////////////////// namespace yw

namespace std {

// tuple size

template<> struct tuple_size<yw::vector> : integral_constant<size_t, 4> {};

// tuple element

template<size_t I> struct tuple_element<I, yw::vector> : type_identity<float> {};

// formatter

template<typename Ct> struct formatter<yw::vector, Ct> : formatter<basic_string<Ct>, Ct> {
  static constexpr Ct text[]{'(', '{', '}', ',', '{', '}', ',', '{', '}', ',', '{', '}', ')', 0};
  auto format(const yw::vector& v, auto& ctx) const {
    return formatter<basic_string<Ct>, Ct>::format(std::format(text, v.x, v.y, v.z, v.w), ctx);
  }
};

} ////////////////////////////////////////////////////////////////////////////// std
