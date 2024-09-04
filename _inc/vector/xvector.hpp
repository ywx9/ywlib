/// \file vector/xvector.hpp

#pragma once

#include <immintrin.h>

#include "../core.hpp"

namespace yw::_ {

template<int X, int Y, int Z, int W>
__m128 _xvpermute(const __m128& a) noexcept {
  constexpr auto f = [](auto a, auto b) { return a == b || a < 0 || 4 <= a; };
  constexpr auto x = f(X, 0) ? 0 : X, y = f(Y, 1) ? 1 : Y, z = f(Z, 2) ? 2 : Z, w = f(W, 3) ? 3 : W;
  if constexpr (f(X, 0) && f(Y, 1) && f(Z, 2) && f(W, 3)) return a;
  else if constexpr (f(X, 0) && f(Y, 0) && f(Z, 0) && f(W, 0)) return _mm_broadcastss_ps(a);
  else if constexpr (f(X, 0) && f(Y, 0) && f(Z, 2) && f(W, 2)) return _mm_moveldup_ps(a);
  else if constexpr (f(X, 1) && f(Y, 1) && f(Z, 3) && f(W, 3)) return _mm_movehdup_ps(a);
  else return _mm_permute_ps(a, x | y << 2 | z << 4 | w << 6);
}

template<int X, int Y, int Z, int W>
__m128 _xvpermute(const __m128& a, const __m128& b) noexcept {
  constexpr bool bx = X < 0, by = Y < 0, bz = Z < 0, bw = W < 0;
  if constexpr (X < 4 && Y < 4 && Z < 4 && W < 4) {
    return _xvpermute<X, Y, Z, W>(a);
  } else if constexpr ((bx || X >= 4) && (by || Y >= 4) && (bz || Z >= 4) && (bw || W >= 4)) {
    return _xvpermute<X - 4, Y - 4, Z - 4, W - 4>(b);
  } else if constexpr ((bx || !(X & 3 ^ 0)) && (by || !(Y & 3 ^ 1)) && (bz || !(Z & 3 ^ 2)) && (bw || !(W & 3 ^ 3))) {
    return _mm_blend_ps(a, b, (X >= 4) | (Y >= 4) << 1 | (Z >= 4) << 2 | (W >= 4) << 3);
  } else if constexpr (X < 4 && Y < 4 && (bz || Z >= 4) && (bw || W >= 4)) {
    constexpr auto x = bx ? 0 : X, y = by ? 1 : Y, z = bz ? 2 : Z - 4, w = bw ? 3 : W - 4;
    return _mm_shuffle_ps(a, b, x | y << 2 | z << 4 | w << 6);
  } else if constexpr ((bx || X >= 4) && (by || Y >= 4) && Z < 4 && W < 4) {
    constexpr auto x = bx ? 0 : X - 4, y = by ? 1 : Y - 4, z = f(Z) ? 2 : Z, w = f(W) ? 3 : W;
    return _mm_shuffle_ps(b, a, x | y << 2 | z << 4 | w << 6);
  } else if constexpr ((bx || X == 0) + (by || Y == 1) + (f(Z) || Z == 2) + (f(W) || W == 3) == 3) {
    constexpr auto x = bx ? 0 : X, y = by ? 1 : Y, z = f(Z) ? 2 : Z, w = f(W) ? 3 : W;
    constexpr nat i = inspects<ge(x, 4), ge(y, 4), ge(z, 4), ge(w, 4)>;
    return _mm_insert_ps(a, b, int((select_value<i, x, y, z, w> - 4) << 6 | i << 4));
  } else if constexpr ((bx || X == 4) + (by || Y == 5) + (f(Z) || Z == 6) + (f(W) || W == 7) == 3) {
    constexpr auto x = bx ? 4 : X, y = by ? 5 : Y, z = f(Z) ? 6 : Z, w = f(W) ? 7 : W;
    constexpr nat i = inspects<lt(x, 4), lt(y, 4), lt(z, 4), lt(w, 4)>;
    return _mm_insert_ps(b, a, int(select_value<i, x, y, z, w>) << 6 | i << 4);
  } else if constexpr ((X < 4 || X == 4) && (Y < 4 || Y == 5) && (Z < 4 || Z == 6) && (W < 4 || W == 7)) {
    return _mm_blend_ps(_xvpermute<X, Y, Z, W>(a), b, X >= 4 | (Y >= 4) << 1 | (Z >= 4) << 2 | (W >= 4) << 3);
  } else if constexpr ((bx || X >= 4 || X == 0) && (by || Y >= 4 || Y == 1) && (bz || Z >= 4 || Z == 2) && (bw || W >= 4 || W == 3)) {
    return _mm_blend_ps(a, _xvpermute<X - 4, Y - 4, Z - 4, W - 4>(b), X >= 4 | (Y >= 4) << 1 | (Z >= 4) << 2 | (W >= 4) << 3);
  } else if constexpr ((0 <= X && X < 4) + (0 <= Y && Y < 4) + (0 <= Z && Z < 4) + (0 <= W && W < 4) == 1) {
    constexpr nat i = inspects<(0 <= X && X < 4), (0 <= Y && Y < 4), (0 <= Z && Z < 4), (0 <= W && W < 4)>;
    return _mm_insert_ps(_xvpermute<X - 4, Y - 4, Z - 4, W - 4>(b), a, i << 4 | select_value<i, X, Y, Z, W> << 6);
  } else if constexpr ((4 <= X) + (4 <= Y) + (4 <= Z) + (4 <= W) == 1) {
    constexpr nat i = inspects<(4 <= X), (4 <= Y), (4 <= Z), (4 <= W)>;
    return _mm_insert_ps(_xvpermute<X, Y, Z, W>(a), b, i << 4 | (select_value<i, X, Y, Z, W> - 4) << 6);
  } else return _mm_blend_ps(_xvpermute<X, Y, Z, W>(a), _xvpermute<X - 4, Y - 4, Z - 4, W - 4>(b),
                             X >= 4 | (Y >= 4) << 1 | (Z >= 4) << 2 | (W >= 4) << 3);
}

} ////////////////////////////////////////////////////////////////////////////// namespace yw::_

export namespace yw {


/// intrinsic vector type
using xvector = __m128;

/// constant vector type
template<value X, value Y = X, value Z = Y, value W = Z>
struct xvconstant {
  operator const xvector&() const noexcept {
    static const xvector v = _mm_set_ps(fat4(W), fat4(Z), fat4(Y), fat4(X));
    return v;
  }
};

/// specialization for zero vector
template<> struct xvconstant<0, 0, 0, 0> {
  operator xvector() const noexcept { return _mm_setzero_ps(); }
};

/// constant vector; `{-0, -0, -0, -0}`
inline constexpr xvconstant<-0.> xvnegzero;

/// loads a vector from the memory
inline xvector xvload(const fat4* Ptr) noexcept { return _mm_loadu_ps(Ptr); }

/// fills a vector with a value
inline xvector xvfill(numeric auto&& Value)
  noexcept { return _mm_set1_ps(fat4(Value)); }

/// sets a vector with four values
inline xvector xvset(numeric auto&& X, numeric auto&& Y,
                     numeric auto&& Z, numeric auto&& W)
  noexcept { return _mm_set_ps(fat4(W), fat4(Z), fat4(Y), fat4(X)); }

/// stores a vector to the memory
inline void xvstore(fat4* Ptr, const xvector& v)
  noexcept { _mm_storeu_ps(Ptr, v); }

/// inserts a value to a vector
template<nat To> requires (To < 4)
inline xvector xvinsert(const xvector& v, numeric auto&& Value) noexcept {
  return _mm_castsi128_ps(_mm_insert_epi32(
    _mm_castps_si128(v), bitcast<int4>(fat4(Value)), To));
}

/// inserts an element of another vector to a vector
template<nat To, nat From, nat Zero = 0b0000>
requires ((To | From) < 4 && Zero < 16)
inline xvector xvinsert(const xvector& a, const xvector& b) noexcept {
  return _mm_insert_ps(a, b, To << 4 | From << 6 | Zero);
}

/// extracts a value from a vector
template<nat I> requires (I < 4)
inline fat4 xvextract(const xvector& v)
  noexcept { return _mm_extract_ps(v, I); }

/// blends two vectors
template<nat Mask> requires (Mask < 16)
inline xvector xvblend(const xvector& a, const xvector& b)
  noexcept { return _mm_blend_ps(a, b, Mask); }

/// blends two vectors
template<bool X, bool Y, bool Z, bool W>
inline xvector xvblend(const xvector& a, const xvector& b)
  noexcept { return xvblend<X | Y * 2 | Z * 4 | W * 8>(a, b); }

/// permutes a vector
template<int X, int Y, int Z, int W>
requires (X < 4 && Y < 4 && Z < 4 && W < 4)
inline xvector xvpermute(const xvector& v)
  noexcept { return _::_xvpermute<X, Y, Z, W>(v); }

/// permutes two vectors
template<int X, int Y, int Z, int W>
requires (X < 8 && Y < 8 && Z < 8 && W < 8)
inline xvector xvpermute(const xvector& a, const xvector& b)
  noexcept { return _::_xvpermute<X, Y, Z, W>(a, b); }

/// performs equality comparison
inline bool operator==(const xvector& a, const xvector& b) noexcept {
  return _mm_test_all_ones(_mm_castps_si128(_mm_cmpeq_ps(a, b)));
}

/// performs three-way comparison
inline po_t operator<=>(const xvector& a, const xvector& b) noexcept {
  return _mm_movemask_ps(_mm_permute_ps(_mm_cmpgt_ps(a, b), 0b00011011)) <=>
         _mm_movemask_ps(_mm_permute_ps(_mm_cmplt_ps(a, b), 0b00011011));
}

/// performs unary plus
inline xvector operator+(const xvector& v) noexcept { return v; }

/// performs unary minus
inline xvector operator-(const xvector& v)
  noexcept { return _mm_xor_ps(v, xvnegzero); }

/// performs addition
inline xvector operator+(const xvector& a, const xvector& b)
  noexcept { return _mm_add_ps(a, b); }

/// performs subtraction
inline xvector operator-(const xvector& a, const xvector& b)
  noexcept { return _mm_sub_ps(a, b); }

/// performs multiplication
inline xvector operator*(const xvector& a, const xvector& b)
  noexcept { return _mm_mul_ps(a, b); }

/// performs multiplication
inline xvector operator*(const xvector& a, numeric auto&& b)
  noexcept { return _mm_mul_ps(a, xvfill(b)); }

/// performs multiplication
inline xvector operator*(numeric auto&& a, const xvector& b)
  noexcept { return _mm_mul_ps(xvfill(a), b); }

/// performs division
inline xvector operator/(const xvector& a, const xvector& b)
  noexcept { return _mm_div_ps(a, b); }

/// performs division
inline xvector operator/(const xvector& a, numeric auto&& b)
  noexcept { return _mm_div_ps(a, xvfill(b)); }

/// returns `{abs(x), ...}`
inline xvector xvabs(const xvector& v)
  noexcept { return _mm_andnot_ps(xvnegzero, v); }

/// returns `{min(a.x, b.x), ...}`
inline xvector xvmin(const xvector& a, const xvector& b)
  noexcept { return _mm_min_ps(a, b); }

/// returns `{max(a.x, b.x), ...}`
inline xvector xvmax(const xvector& a, const xvector& b)
  noexcept { return _mm_max_ps(a, b); }

/// returns `{a.x * b.x + c.x), ...}`
inline xvector xvfma(const xvector& a, const xvector& b, const xvector& c)
  noexcept { return _mm_fmadd_ps(a, b, c); }

/// returns `{a.x * b.x - c.x), ...}`
inline xvector xvfms(const xvector& a, const xvector& b, const xvector& c)
  noexcept { return _mm_fmsub_ps(a, b, c); }

/// returns `{-a.x * b.x + c.x), ...}`
inline xvector xvfnma(const xvector& a, const xvector& b, const xvector& c)
  noexcept { return _mm_fnmadd_ps(a, b, c); }

/// returns `{-a.x * b.x - c.x), ...}`
inline xvector xvfnms(const xvector& a, const xvector& b, const xvector& c)
  noexcept { return _mm_fnmsub_ps(a, b, c); }

///





struct xvector {

  __m128 r;

  /// default constructor
  xvector() noexcept = default;

  /// constructor with an intrinsic vector
  xvector(const __m128& v) noexcept : r(v) {}

  /// constructor with a value to fill
  explicit xvector(const fat4 Fill) noexcept : r(_mm_set1_ps(Fill)) {}

  /// constructor with a value to fill
  explicit xvector(numeric auto&& Fill) noexcept : xvector(fat4(Fill)) {}

  /// constructor with the first two value
  xvector(numeric auto&& X, numeric auto&& Y)
    noexcept : r(_mm_set_ps(0, 0, fat4(Y), fat4(X))) {}

  /// constructor with the first three value
  xvector(numeric auto&& X, numeric auto&& Y, numeric auto&& Z)
    noexcept : r(_mm_set_ps(0, fat4(Z), fat4(Y), fat4(X))) {}

  /// constructor with the four value
  xvector(numeric auto&& X, numeric auto&& Y, numeric auto&& Z, numeric auto&& W)
    noexcept : r(_mm_set_ps(fat4(W), fat4(Z), fat4(Y), fat4(X))) {}

  /// conversion to `__m128`
  operator __m128&() noexcept { return r; }

  /// conversion to `__m128`
  operator const __m128&() const noexcept { return r; }

  /// sets a value to this
  template<nat To> requires (To < 4)
  xvector set(numeric auto&& Value) const noexcept {
    return _mm_castsi128_ps(_mm_insert_epi32(
      _mm_castps_si128(r), bitcast<int4>(fat4(Value)), To));
  }

  /// sets an value of another vector to this
  template<nat To, nat From, nat Zero = 0b0000>
  requires ((To | From) < 4 && Zero < 16)
  xvector set(const __m128& v) noexcept {
    return _mm_insert_ps(r, v, To << 4 | From << 6 | Zero);
  }

  /// extracts a value from this
  template<nat I> requires (I < 4)
  fat4 get() const noexcept { return _mm_extract_ps(r, I); }

  /// stores the vector to the memory
  friend void store(fat4* Ptr, const __m128& v)
    noexcept { _mm_storeu_ps(Ptr, v); }

  /// blends two vectors
  template<nat Mask> requires (Mask < 16)
  friend xvector blend(const __m128& a, const __m128& b)
    noexcept { return _mm_blend_ps(a, b, Mask); }

  /// blends two vectors

  template<bool X, bool Y, bool Z, bool W>
  friend xvector blend(const __m128& a, const __m128& b)
    noexcept { return blend<X | Y * 2 | Z * 4 | W * 8>(a, b); }

  /// permutes a vector
  template<int X, int Y, int Z, int W>
  requires (X < 4 && Y < 4 && Z < 4 && W < 4)
  friend xvector permute(const __m128& v) noexcept {
    return _::_xvpermute<X, Y, Z, W>(v);
  }

  /// permutes two vectors
  template<int X, int Y, int Z, int W>
  requires (X < 8 && Y < 8 && Z < 8 && W < 8)
  friend xvector permute(const __m128& a, const __m128& b) noexcept {
    return _::_xvpermute<X, Y, Z, W>(a, b);
  }

  /// performs equality comparison
  friend bool operator==(const __m128& a, const __m128& b) noexcept {
    return _mm_test_all_ones(_mm_castps_si128(_mm_cmpeq_ps(a, b)));
  }

  /// performs three-way comparison
  friend po_t operator<=>(const __m128& a, const __m128& b) noexcept {
    return _mm_movemask_ps(_mm_permute_ps(_mm_cmpgt_ps(a, b), 0b00011011)) <=>
           _mm_movemask_ps(_mm_permute_ps(_mm_cmplt_ps(a, b), 0b00011011));
  }

  /// constant vector; negative zero
  inline static __m128 neg_zero = _mm_set1_ps(-0.f);

  /// returns `{abs(x), ...}`
  friend xvector abs(const __m128& v) noexcept {
    return _mm_andnot_ps(neg_zero, v);
  }

  /// unary plus
  friend __m128 operator+(const __m128& v) noexcept { return v; }

  /// unary minus
  friend __m128 operator-(const __m128& v)
    noexcept { return _mm_xor_ps(v, neg_zero); }

  /// addition
  friend __m128 operator+(const __m128& a, const __m128& b)
    noexcept { return _mm_add_ps(a, b); }

  /// subtraction
  friend __m128 operator-(const __m128& a, const __m128& b)
    noexcept { return _mm_sub_ps(a, b); }

  /// multiplication
  friend __m128 operator*(const __m128& a, const __m128& b)
    noexcept { return _mm_mul_ps(a, b); }

  /// division
  friend __m128 operator/(const __m128& a, const __m128& b)
    noexcept { return _mm_div_ps(a, b); }

  /// addition assignment
  xvector& operator+=(const __m128& v)
    noexcept { return r = _mm_add_ps(r, v), *this; }

  ///
};


} ////////////////////////////////////////////////////////////////////////////// namespace yw
