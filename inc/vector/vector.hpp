/// \file vector/vector.hpp

#pragma once

#include <immintrin.h>

#include "vector3.hpp"

export namespace yw {


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

  /// constructor with __m128
  vector(const __m128& v) noexcept { _mm_storeu_ps(&x, v); }

  /// conversion to `vector2`
  constexpr operator vector2() const noexcept { return {x, y}; }

  /// conversion to `vector3`
  constexpr operator vector3() const noexcept { return {x, y, z}; }

  /// conversion to __m128
  operator __m128() const noexcept { return _mm_loadu_ps(&x); }

  /// performs `get` operation
  template<nat I> requires (I < count)
  constexpr fat4& get() &
  noexcept { return select<I>(x, y, z, w); }

  /// performs `get` operation
  template<nat I> requires (I < count)
  constexpr const fat4& get() const &
  noexcept { return select<I>(x, y, z, w); }

  /// performs `get` operation
  template<nat I> requires (I < count)
  constexpr fat4&& get() &&
  noexcept { return select<I>(mv(x), mv(y), mv(z), mv(w)); }

  /// performs `get` operation
  template<nat I> requires (I < count)
  constexpr const fat4&& get() const &&
  noexcept { return select<I>(mv(x), mv(y), mv(z), mv(w)); }

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
  friend constexpr vector operator-(const vector& a)
  noexcept { return {-a.x, -a.y, -a.z, -a.w}; }

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
