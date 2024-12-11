#pragma once

#include "ywlib-std.hpp"
#include <immintrin.h>

export namespace yw {

using xvector = __m128;
using xmatrix = array<xvector, 4>;

namespace _ {
template<value X, value Y, value Z, value W> struct _xv_constant {
  inline static const xvector f = _mm_set_ps(fat4(W), fat4(Z), fat4(Y), fat4(X));
  inline static const __m256d d = _mm256_set_pd(fat8(W), fat8(Z), fat8(Y), fat8(X));
  inline static const __m128i i = _mm_set_epi32(int4(static_cast<int8>(W) & 0xffffffff), int4(static_cast<int8>(Z) & 0xffffffff), //
                                                int4(static_cast<int8>(Y) & 0xffffffff), int4(static_cast<int8>(X) & 0xffffffff));
  operator add_const<xvector&>() const noexcept { return f; }
  operator add_const<__m128i&>() const noexcept { return i; }
  operator add_const<__m256d&>() const noexcept { return d; }
};
}

template<value X, value Y = X, value Z = Y, value W = Z> //
inline constexpr _::_xv_constant<X, Y, Z, W> xv_constant{};

inline constexpr auto xv_zero = xv_constant<0>;
inline constexpr auto xv_negzero = xv_constant<-0.0>;
inline constexpr auto xv_one = xv_constant<1>;
inline constexpr auto xv_negone = xv_constant<-1>;
inline constexpr auto xv_x = xv_constant<1, 0, 0, 0>;
inline constexpr auto xv_y = xv_constant<0, 1, 0, 0>;
inline constexpr auto xv_z = xv_constant<0, 0, 1, 0>;
inline constexpr auto xv_w = xv_constant<0, 0, 0, 1>;
inline constexpr auto xv_negx = xv_constant<-1, 0, 0, 0>;
inline constexpr auto xv_negy = xv_constant<0, -1, 0, 0>;
inline constexpr auto xv_negz = xv_constant<0, 0, -1, 0>;
inline constexpr auto xv_negw = xv_constant<0, 0, 0, -1>;

inline xvector xvload(const fat4* p) noexcept { return _mm_load_ps(p); }
inline xvector xvfill(const fat4 v) noexcept { return _mm_set1_ps(v); }
inline xvector xvset(const fat4 x, const fat4 y, const fat4 z, const fat4 w) noexcept { return _mm_set_ps(w, z, y, x); }
inline void xvstore(const xvector& a, fat4* p) noexcept { _mm_store_ps(p, a); }

template<nat To> requires (To < 4) xvector xvinsert(const xvector& v, const fat4 x) noexcept { //
  return _mm_castsi128_ps(_mm_insert_epi32<To>(_mm_castps_si128(v), bitcast<int4>(x)));
}

template<nat To, nat From, nat Zero = 0> requires (To < 4 && From < 4 && Zero < 16) //
xvector xvinsert(const xvector& a, const xvector& b) noexcept {
  return _mm_insert_ps(a, b, Zero | To << 4 | From << 6);
}

template<nat I> requires (I < 4) //
fat4 xvextract(const xvector& v) noexcept {
  if constexpr (I == 0) return _mm_cvtss_f32(v);
  else return bitcast<fat4>(_mm_extract_ps(v, I));
}

template<nat I> requires (I < 4) int4 xvextract(const __m128i& m) noexcept { return _mm_extract_epi32(m, I); }

template<nat Mask> requires (Mask < 16) //
xvector xvblend(const xvector& a, const xvector& b) noexcept {
  return _mm_blend_ps(a, b, Mask);
}

template<bool X, bool Y, bool Z, bool W> //
xvector xvblend(const xvector& a, const xvector& b) noexcept {
  return _mm_blend_ps(a, b, X | Y << 1 | Z << 2 | W << 3);
}
}

namespace yw::_ {

template<int X, int Y, int Z, int W> xvector _xvpermute(const xvector& a) noexcept {
  constexpr auto f = [](auto i, auto j) { return i < 0 || i == j || 4 <= i; };
  constexpr auto x = f(X, 0) ? 0 : X, y = f(Y, 1) ? 1 : Y, z = f(Z, 2) ? 2 : Z, w = f(W, 3) ? 3 : W;
  if constexpr (f(X, 0) && f(Y, 1) && f(Z, 2) && f(W, 3)) return a;
  else if constexpr (f(X, 0) && f(Y, 0) && f(Z, 0) && f(W, 0)) return _mm_broadcastss_ps(a);
  else if constexpr (f(x, 0) && f(y, 0) && f(z, 2) && f(w, 2)) return _mm_moveldup_ps(a);
  else if constexpr (f(x, 1) && f(y, 1) && f(z, 3) && f(w, 3)) return _mm_movehdup_ps(a);
  else return _mm_permute_ps(a, x | y << 2 | z << 4 | w << 6);
}

template<int X, int Y, int Z, int W> xvector _xvpermute(const xvector& a, const xvector& b) noexcept {
  constexpr bool bx = X < 0, by = Y < 0, bz = Z < 0, bw = W < 0;
  if constexpr (X < 4 && Y < 4 && Z < 4 && W < 4) {
    return _xvpermute<X, Y, Z, W>(a);
  } else if constexpr ((bx || X >= 4) && (by || Y >= 4) && (bz || Z >= 4) && (bw || W >= 4)) {
    return _xvpermute<X - 4, Y - 4, Z - 4, W - 4>(b);
  } else if constexpr ((bx || !(X & 3 ^ 0)) && (by || !(Y & 3 ^ 1)) && (bz || !(Z & 3 ^ 2)) && (bw || !(W & 3 ^ 3))) {
    return xvblend<ge(X, 4), ge(Y, 4), ge(Z, 4), ge(W, 4)>(a, b);
  } else if constexpr (X < 4 && Y < 4 && (bz || Z >= 4) && (bw || W >= 4)) {
    constexpr auto x = bx ? 0 : X, y = by ? 1 : Y, z = bz ? 2 : Z - 4, w = bw ? 3 : W - 4;
    return _mm_shuffle_ps(a, b, x | y << 2 | z << 4 | w << 6);
  } else if constexpr ((bx || X >= 4) && (by || Y >= 4) && Z < 4 && W < 4) {
    constexpr auto x = bx ? 0 : X - 4, y = by ? 1 : Y - 4, z = f(Z) ? 2 : Z, w = f(W) ? 3 : W;
    return _mm_shuffle_ps(b, a, x | y << 2 | z << 4 | w << 6);
  } else if constexpr ((bx || X == 0) + (by || Y == 1) + (f(Z) || Z == 2) + (f(W) || W == 3) == 3) {
    constexpr auto x = bx ? 0 : X, y = by ? 1 : Y, z = f(Z) ? 2 : Z, w = f(W) ? 3 : W;
    constexpr nat i = inspects<ge(x, 4), ge(y, 4), ge(z, 4), ge(w, 4)>;
    return xvinsert<i, int(select_value<i, x, y, z, w> - 4)>(a, b);
  } else if constexpr ((bx || X == 4) + (by || Y == 5) + (f(Z) || Z == 6) + (f(W) || W == 7) == 3) {
    constexpr auto x = bx ? 4 : X, y = by ? 5 : Y, z = f(Z) ? 6 : Z, w = f(W) ? 7 : W;
    constexpr nat i = inspects<lt(x, 4), lt(y, 4), lt(z, 4), lt(w, 4)>;
    return xvinsert<i, select_value<i, x, y, z, w>>(b, a);
  } else if constexpr ((X < 4 || X == 4) && (Y < 4 || Y == 5) && (Z < 4 || Z == 6) && (W < 4 || W == 7)) {
    return xvblend<ge(X, 4), ge(Y, 4), ge(Z, 4), ge(W, 4)>(_xvpermute<X, Y, Z, W>(a), b);
  } else if constexpr ((bx || X >= 4 || X == 0) && (by || Y >= 4 || Y == 1) && (bz || Z >= 4 || Z == 2) && (bw || W >= 4 || W == 3)) {
    return xvblend<ge(X, 4), ge(Y, 4), ge(Z, 4), ge(W, 4)>(a, _xvpermute<X - 4, Y - 4, Z - 4, W - 4>(b));
  } else if constexpr ((0 <= X && X < 4) + (0 <= Y && Y < 4) + (0 <= Z && Z < 4) + (0 <= W && W < 4) == 1) {
    constexpr nat i = inspects<(0 <= X && X < 4), (0 <= Y && Y < 4), (0 <= Z && Z < 4), (0 <= W && W < 4)>;
    return xvinsert<i, select_value<i, X, Y, Z, W>>(_xvpermute<X - 4, Y - 4, Z - 4, W - 4>(b), a);
  } else if constexpr ((4 <= X) + (4 <= Y) + (4 <= Z) + (4 <= W) == 1) {
    constexpr nat i = inspects<(4 <= X), (4 <= Y), (4 <= Z), (4 <= W)>;
    return xvinsert<i, select_value<i, X, Y, Z, W> - 4>(_xvpermute<X, Y, Z, W>(a), b);
  } else return xvblend<ge(X, 4), ge(Y, 4), ge(Z, 4), ge(W, 4)>(_xvpermute<X, Y, Z, W>(a), _xvpermute<X - 4, Y - 4, Z - 4, W - 4>(b));
}

}

export namespace yw {

template<int X, int Y, int Z, int W> requires (X < 4 && Y < 4 && Z < 4 && W < 4) //
xvector xvpermute(const xvector& v) noexcept {
  return _::_xvpermute<X, Y, Z, W>(v);
}

template<int X, int Y, int Z, int W> requires (X < 8 && Y < 8 && Z < 8 && W < 8) //
xvector xvpermute(const xvector& a, const xvector& b) noexcept {
  return _::_xvpermute<X, Y, Z, W>(a, b);
}

inline xvector xvabs(const xvector& v) noexcept { return _mm_andnot_ps(xv_negzero, v); }
inline xvector xvneg(const xvector& a) noexcept { return _mm_xor_ps(a, xv_negzero); }

inline xvector xvadd(const xvector& a, const xvector& b) noexcept { return _mm_add_ps(a, b); }
inline xvector xvsub(const xvector& a, const xvector& b) noexcept { return _mm_sub_ps(a, b); }
inline xvector xvmul(const xvector& a, const xvector& b) noexcept { return _mm_mul_ps(a, b); }
inline xvector xvdiv(const xvector& a, const xvector& b) noexcept { return _mm_div_ps(a, b); }

inline xvector xvand(const xvector& a, const xvector& b) noexcept { return _mm_and_ps(a, b); }
inline xvector xvor(const xvector& a, const xvector& b) noexcept { return _mm_or_ps(a, b); }
inline xvector xvxor(const xvector& a, const xvector& b) noexcept { return _mm_xor_ps(a, b); }

inline xvector operator+(const xvector& a) noexcept { return a; }
inline xvector operator-(const xvector& a) noexcept { return xvneg(a); }
inline xvector operator+(const xvector& a, const xvector& b) noexcept { return xvadd(a, b); }
inline xvector operator-(const xvector& a, const xvector& b) noexcept { return xvsub(a, b); }
inline xvector operator*(const xvector& a, const xvector& b) noexcept { return xvmul(a, b); }
inline xvector operator/(const xvector& a, const xvector& b) noexcept { return xvdiv(a, b); }

inline bool operator==(const xvector& a, const xvector& b) noexcept { //
  return _mm_test_all_ones(_mm_castps_si128(_mm_cmpeq_ps(a, b)));
}
inline auto operator<=>(const xvector& a, const xvector& b) noexcept {      //
  return _mm_movemask_ps(_::_xvpermute<3, 2, 1, 0>(_mm_cmpgt_ps(a, b))) <=> //
         _mm_movemask_ps(_::_xvpermute<3, 2, 1, 0>(_mm_cmplt_ps(a, b)));
}

inline xvector xvceil(const xvector& a) noexcept { return _mm_ceil_ps(a); }
inline xvector xvfloor(const xvector& a) noexcept { return _mm_floor_ps(a); }
inline xvector xvround(const xvector& a) noexcept { return _mm_round_ps(a, 8); }
inline xvector xvtrunc(const xvector& a) noexcept { return _mm_trunc_ps(a); }

inline xvector xvmax(const xvector& a, const xvector& b) noexcept { return _mm_max_ps(a, b); }
inline xvector xvmin(const xvector& a, const xvector& b) noexcept { return _mm_min_ps(a, b); }

inline xvector xvfma(const xvector& a, const xvector& b, const xvector& c) noexcept { return _mm_fmadd_ps(a, b, c); }
inline xvector xvfms(const xvector& a, const xvector& b, const xvector& c) noexcept { return _mm_fmsub_ps(a, b, c); }
inline xvector xvfnma(const xvector& a, const xvector& b, const xvector& c) noexcept { return _mm_fnmadd_ps(a, b, c); }
inline xvector xvfnms(const xvector& a, const xvector& b, const xvector& c) noexcept { return _mm_fnmsub_ps(a, b, c); }

inline xvector xvsin(const xvector& a) noexcept { return _mm_sin_ps(a); }
inline xvector xvcos(const xvector& a) noexcept { return _mm_cos_ps(a); }
inline xvector xvtan(const xvector& a) noexcept { return _mm_tan_ps(a); }
inline xvector xvasin(const xvector& a) noexcept { return _mm_asin_ps(a); }
inline xvector xvacos(const xvector& a) noexcept { return _mm_acos_ps(a); }
inline xvector xvatan(const xvector& a) noexcept { return _mm_atan_ps(a); }
inline xvector xvatan(const xvector& y, const xvector& x) noexcept { return _mm_atan2_ps(y, x); }
inline xvector xvsincos(const xvector& a, xvector& Cos) noexcept { return _mm_sincos_ps(&Cos, a); }

inline xvector xvsinh(const xvector& a) noexcept { return _mm_sinh_ps(a); }
inline xvector xvcosh(const xvector& a) noexcept { return _mm_cosh_ps(a); }
inline xvector xvtanh(const xvector& a) noexcept { return _mm_tanh_ps(a); }
inline xvector xvasinh(const xvector& a) noexcept { return _mm_asinh_ps(a); }
inline xvector xvacosh(const xvector& a) noexcept { return _mm_acosh_ps(a); }
inline xvector xvatanh(const xvector& a) noexcept { return _mm_atanh_ps(a); }

inline xvector xvpow(const xvector& Base, const xvector& Exp) noexcept { return _mm_pow_ps(Base, Exp); }
inline xvector xvexp(const xvector& a) noexcept { return _mm_exp_ps(a); }
inline xvector xvexp2(const xvector& a) noexcept { return _mm_exp2_ps(a); }
inline xvector xvexp10(const xvector& a) noexcept { return _mm_exp10_ps(a); }
inline xvector xvexpm1(const xvector& a) noexcept { return _mm_expm1_ps(a); }
inline xvector xvsqrt(const xvector& a) noexcept { return _mm_sqrt_ps(a); }
inline xvector xvcbrt(const xvector& a) noexcept { return _mm_cbrt_ps(a); }
inline xvector xvsqrt_r(const xvector& a) noexcept { return _mm_invsqrt_ps(a); }
inline xvector xvcbrt_r(const xvector& a) noexcept { return _mm_invcbrt_ps(a); }
inline xvector xvhypot(const xvector& a, const xvector& b) noexcept { return _mm_hypot_ps(a, b); }

inline xvector xvln(const xvector& a) noexcept { return _mm_log_ps(a); }
inline xvector xvlog(const xvector& a, const xvector& Base) noexcept { return xvln(a) / xvln(Base); }
inline xvector xvlog2(const xvector& a) noexcept { return _mm_log2_ps(a); }
inline xvector xvlog10(const xvector& a) noexcept { return _mm_log10_ps(a); }
inline xvector xvlog1p(const xvector& a) noexcept { return _mm_log1p_ps(a); }
inline xvector xvlogb(const xvector& a) noexcept { return _mm_logb_ps(a); }

inline xvector xverf(const xvector& a) noexcept { return _mm_erf_ps(a); }
inline xvector xverfc(const xvector& a) noexcept { return _mm_erfc_ps(a); }
inline xvector xverf_r(const xvector& a) noexcept { return _mm_erfinv_ps(a); }
inline xvector xverfc_r(const xvector& a) noexcept { return _mm_erfcinv_ps(a); }

inline xvector xvcross(const xvector& a, const xvector& b) noexcept { //
  return xvfms(xvpermute<1, 2, 0, 3>(a), xvpermute<2, 0, 1, 3>(b),    //
               xvpermute<2, 0, 1, 3>(a) * xvpermute<1, 2, 0, 3>(b));
}

/// \brief dot product of two vectors
/// \tparam N dimension of the vectors
/// \tparam Zero zero mask of the result
/// \example `xvdot<2>(a, b)` returns `{c, c, c, c}` where `c = a.x * b.x + a.y * b.y`
/// \example `xvdot<3, 0b1010>(a, b)` returns `{c, 0, c, 0}` where `c = a.x * b.x + a.y * b.y + a.z * b.z`
template<nat N, nat Zero = 0> requires (1 <= N && N <= 4 && Zero < 16) //
xvector xvdot(const xvector& a, const xvector& b) noexcept {
  if constexpr (Zero == 15) return xv_zero;
  else return _mm_dp_ps(a, b, (Zero ^ 15) | ((1 << N) - 1) << 4);
}

template<nat N, nt_tuple_for<xvector> Matrix, nat M = extent<Matrix>>               //
requires (1 <= N && N <= 4 && !convertible_to<Matrix, xvector> && 1 <= M && M <= 4) //
xvector xvdot(Matrix&& m, const xvector& v) noexcept {
  auto r = xvdot<N, 0b1110>(get<0>(m), v);
  if constexpr (2 <= M) r = xvor(r, xvdot<N, 0b1101>(get<1>(m), v));
  if constexpr (3 <= M) r = xvor(r, xvdot<N, 0b1011>(get<2>(m), v));
  if constexpr (4 == M) r = xvor(r, xvdot<N, 0b0111>(get<3>(m), v));
  return r;
}

template<nat N, nt_tuple_for<xvector> Matrix, nat M = extent<Matrix>>               //
requires (1 <= N && N <= 4 && !convertible_to<Matrix, xvector> && 1 <= M && M <= 4) //
xvector xvdot(const xvector& v, Matrix&& m) noexcept {
  auto r = xvmul(xvpermute<0, 0, 0, 0>(v), get<0>(m));
  if constexpr (2 <= M) r = xvfma(xvpermute<1, 1, 1, 1>(v), get<1>(m), r);
  if constexpr (3 <= M) r = xvfma(xvpermute<2, 2, 2, 2>(v), get<2>(m), r);
  if constexpr (4 == M) r = xvfma(xvpermute<3, 3, 3, 3>(v), get<3>(m), r);
  return xvblend<(1 << N) - 1>(xv_zero, r);
}

template<nat N, nt_tuple_for<xvector> Mt1, nt_tuple_for<xvector> Mt2, nat L = extent<Mt1>, nat M = extent<Mt2>>   //
requires (1 <= N && N <= 4 && !convertible_to<Mt1, xvector> && !convertible_to<Mt2, xvector> && 1 <= M && M <= 4) //
void xvdot(const Mt1& m1, const Mt2& m2, Mt1& out) noexcept requires nt_vassignable<Mt1&, const array<xvector, L>&> {
  [&]<nat... Is>(sequence<Is...>) { ((get<Is>(out) = xvdot<N>(get<Is>(m1), m2)), ...); }(make_sequence<0, L>{});
}

/// \brief converts a hex rgb color to a vector
/// \param Color hex rgb color: `0xAARRGGBB`
/// \return vector with components: `R / 255, G / 255, B / 255, A / 255`
inline xvector xvhex2rgb(const nat4 Color) noexcept { //
  auto a = _mm_cvtepi32_ps(_mm_cvtepu8_epi32(_mm_castps_si128(_mm_broadcast_ss((const fat4*)&Color))));
  return xvpermute<2, 1, 0, 3>(a * xv_constant<1 / 255.0>);
}

/// \brief converts a vector to a hex rgb color
/// \param v vector: `{R, G, B, A}`
/// \return hex rgb color: `0xAARRGGBB`
inline nat4 xvrgb2hex(const xvector& v) noexcept { //
  // multiply by 255 (value rage: 0-255)
  auto a = xvmul(v, xv_constant<255>);
  // fat4 to int4 (0x000000FF 0x000000FF 0x000000FF 0x000000FF)
  auto b = _mm_cvtps_epi32(a);
  // collect low 8 bits of each component
  auto c = _mm_shuffle_epi8(b, xv_constant<0x0c000408, 0>);
  return bitcast<nat4>(_mm_cvtsi128_si32(c));
}

template<typename T> struct vector {
  static constexpr nat count = 4;
  using value_type = T;
  T x{}, y{}, z{}, w{};
  constexpr vector() noexcept = default;
  explicit constexpr vector(const T& v) noexcept : x(v), y(v), z(v), w(v) {}
  constexpr vector(const T& X, const T& Y) noexcept : x(X), y(Y) {}
  constexpr vector(const T& X, const T& Y, const T& Z) noexcept : x(X), y(Y), z(Z) {}
  constexpr vector(const T& X, const T& Y, const T& Z, const T& W) noexcept : x(X), y(Y), z(Z), w(W) {}
  template<nat I> requires (I < count) constexpr T& get() & noexcept { return select<I>(x, y, z, w); }
  template<nat I> requires (I < count) constexpr const T& get() const& noexcept { return select<I>(x, y, z, w); }
  template<nat I> requires (I < count) constexpr T&& get() && noexcept { return mv(select<I>(x, y, z, w)); }
  template<nat I> requires (I < count) constexpr const T&& get() const&& noexcept { return mv(select<I>(x, y, z, w)); }
  constexpr nat size() const noexcept { return count; }
  T* data() noexcept { return &x; }
  const T* data() const noexcept { return &x; }
  T* begin() noexcept { return &x; }
  const T* begin() const noexcept { return &x; }
  T* end() noexcept { return &w + 1; }
  const T* end() const noexcept { return &w + 1; }
  constexpr T& operator[](nat i) {
    if (is_cev) {
      if (4 <= i) cannot_be_constant_evaluated();
      return i == 0 ? x : i == 1 ? y : i == 2 ? z : w;
    } else return data()[i];
  }
  constexpr const T& operator[](nat i) const {
    if (is_cev) {
      if (4 <= i) cannot_be_constant_evaluated();
      return i == 0 ? x : i == 1 ? y : i == 2 ? z : w;
    } else return data()[i];
  }
};

template<> struct vector<fat4> {
  static constexpr nat count = 4;
  using value_type = fat4;
  fat4 x{}, y{}, z{}, w{};
  constexpr vector() noexcept = default;
  explicit constexpr vector(const fat4 Fill) noexcept : x(Fill), y(Fill), z(Fill), w(Fill) {}
  explicit constexpr vector(numeric auto&& Fill) noexcept : vector(fat4(Fill)) {}
  constexpr vector(numeric auto&& X, numeric auto&& Y) noexcept : x(fat4(X)), y(fat4(Y)) {}
  constexpr vector(numeric auto&& X, numeric auto&& Y, numeric auto&& Z) noexcept : x(fat4(X)), y(fat4(Y)), z(fat4(Z)) {}
  constexpr vector(numeric auto&& X, numeric auto&& Y, numeric auto&& Z, numeric auto&& W) noexcept : x(fat4(X)), y(fat4(Y)), z(fat4(Z)), w(fat4(W)) {}
  vector(const xvector& Xv) noexcept { xvstore(Xv, &x); }
  operator xvector() const noexcept { return xvload(&x); }
  template<nat I> requires (I < count) constexpr fat4& get() & noexcept { return select<I>(x, y, z, w); }
  template<nat I> requires (I < count) constexpr const fat4& get() const& noexcept { return select<I>(x, y, z, w); }
  template<nat I> requires (I < count) constexpr fat4&& get() && noexcept { return mv(select<I>(x, y, z, w)); }
  template<nat I> requires (I < count) constexpr const fat4&& get() const&& noexcept { return mv(select<I>(x, y, z, w)); }
  constexpr nat size() const noexcept { return count; }
  fat4* data() noexcept { return &x; }
  const fat4* data() const noexcept { return &x; }
  fat4* begin() noexcept { return &x; }
  const fat4* begin() const noexcept { return &x; }
  fat4* end() noexcept { return &w + 1; }
  const fat4* end() const noexcept { return &w + 1; }
  constexpr fat4& operator[](nat i) {
    if (is_cev) {
      if (4 <= i) cannot_be_constant_evaluated();
      return i == 0 ? x : i == 1 ? y : i == 2 ? z : w;
    } else return data()[i];
  }
  constexpr const fat4& operator[](nat i) const {
    if (is_cev) {
      if (4 <= i) cannot_be_constant_evaluated();
      return i == 0 ? x : i == 1 ? y : i == 2 ? z : w;
    } else return data()[i];
  }
  friend constexpr bool operator==(const vector& a, const vector& b) noexcept {
    if (is_cev) return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
    else return xvector(a) == xvector(b);
  }
  friend constexpr auto operator<=>(const vector& a, const vector& b) noexcept {
    if (is_cev) {
      if (auto c = a.x <=> b.x; c != 0) return c;
      else if (c = a.y <=> b.y; c != 0) return c;
      else if (c = a.z <=> b.z; c != 0) return c;
      else return a.w <=> b.w;
    } else return std::partial_ordering(xvector(a) <=> xvector(b));
  }
  friend constexpr vector operator+(const vector& a) noexcept { return a; }
  friend constexpr vector operator-(const vector& a) noexcept {
    if (is_cev) return {-a.x, -a.y, -a.z, -a.w};
    else return xvneg(a);
  }
  friend constexpr vector operator+(const vector& a, const vector& b) noexcept {
    if (is_cev) return {a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w};
    else return xvadd(a, b);
  }
  friend constexpr vector operator-(const vector& a, const vector& b) noexcept {
    if (is_cev) return {a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w};
    else return xvsub(a, b);
  }
  friend constexpr vector operator*(const vector& a, const vector& b) noexcept {
    if (is_cev) return {a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w};
    else return xvmul(a, b);
  }
  friend constexpr vector operator/(const vector& a, const vector& b) noexcept {
    if (is_cev) return {a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w};
    else return xvdiv(a, b);
  }
  friend constexpr vector operator*(const vector& a, const fat4 b) noexcept {
    if (is_cev) return {a.x * b, a.y * b, a.z * b, a.w * b};
    else return xvmul(a, xvfill(b));
  }
  friend constexpr vector operator/(const vector& a, const fat4 b) noexcept {
    if (is_cev) return {a.x / b, a.y / b, a.z / b, a.w / b};
    else return xvmul(a, xvfill(1.0f / b));
  }
  friend constexpr vector operator*(numeric auto&& a, const vector& b) noexcept { return b * fat4(a); }
  friend constexpr vector operator*(const vector& a, numeric auto&& b) noexcept { return a * fat4(b); }
  friend constexpr vector operator/(const vector& a, numeric auto&& b) noexcept { return a / fat4(b); }
  constexpr vector abs() const noexcept {
    if (is_cev) return {std::fabs(x), std::fabs(y), std::fabs(z), std::fabs(w)};
    else return xvabs(*this);
  }
  fat4 length() const noexcept {
    if (is_cev) return std::sqrt(x * x + y * y + z * z + w * w);
    else return [](const xvector& v) { return xvextract<0>(xvsqrt(xvdot<4>(v, v))); }(*this);
  }
  fat4 length3() const noexcept {
    if (is_cev) return std::sqrt(x * x + y * y + z * z);
    else return [](const xvector& v) { return xvextract<0>(xvsqrt(xvdot<3>(v, v))); }(*this);
  }
  vector normalize() const noexcept {
    if (is_cev) return *this / length();
    else return [](const xvector& v) { return v * xvsqrt_r(xvdot<4>(v, v)); }(*this);
  }
  vector normalize3() const noexcept {
    if (is_cev) return *this / length3();
    else return [](const xvector& v) { return v * xvsqrt_r(xvdot<3>(v, v)); }(*this);
  }
};

template<typename... Ts> vector(Ts...) -> vector<fat4>;

////////////////////////////////////////////////////////////////////////////////
/// \brief RGBA color
struct color {

  /// \brief red component [0..1]
  fat4 r{};

  /// \brief green component [0..1]
  fat4 g{};

  /// \brief blue component [0..1]
  fat4 b{};

  /// \brief alpha component [0..1]; `0` is transparent
  fat4 a = 1.f;

  /// \brief default constructor
  constexpr color() noexcept = default;

  /// \brief constructs an untransparent color
  constexpr color(numeric auto&& R, numeric auto&& G, numeric auto&& B) noexcept : r(fat4(R)), g(fat4(G)), b(fat4(B)) {}

  /// \brief constructs a color with alpha
  constexpr color(numeric auto&& R, numeric auto&& G, numeric auto&& B, numeric auto&& A) noexcept //
    : r(fat4(R)), g(fat4(G)), b(fat4(B)), a(fat4(A)) {}

  /// \brief constructs an untransparent color from `0xRRGGBB`
  explicit constexpr color(nat4 xRRGGBB) noexcept { from_code(*this, xRRGGBB), a = 1.f; }

  /// \brief constructs a color from `0xRRGGBB` and alpha
  constexpr color(nat4 xRRGGBB, numeric auto&& Alpha) noexcept { from_code(*this, xRRGGBB), a = fat4(Alpha); }

  /// \brief converts the color to `0xAARRGGBB`
  explicit constexpr operator nat4() const noexcept {
    if (is_cev) return (nat4)(r * 255.f) << 16 | (nat4)(g * 255.f) << 8 | (nat4)(b * 255.f) | (nat4)(a * 255.f) << 24;
    else return xvrgb2hex(*this);
  }

  /// \brief constructs a color from `xvector`
  explicit color(const xvector& Xv) noexcept { _mm_storeu_ps(&r, Xv); }

  /// \brief converts the color to `xvector`
  operator xvector() const noexcept { return xvload(&r); }

  /// \brief checks if two colors are equal
  friend constexpr bool operator==(const color& A, const color& B) noexcept {
    if (!is_cev) return xvector(A) == xvector(B);
    else return A.r == B.r && A.g == B.g && A.b == B.b && A.a == B.a;
  }

  /// \brief `get` function
  template<nat I> requires (I < 4) constexpr fat4& get() noexcept { return select<I>(r, g, b, a); }

  /// \brief `get` function
  template<nat I> requires (I < 4) constexpr fat4 get() const noexcept { return select<I>(r, g, b, a); }

  static const color black, dimgray, gray, darkgray, silver, lightgray, gainsboro, whitesmoke, white, snow, ghostwhite,            //
    floralwhite, linen, antiquewhite, papayawhip, blanchedalmond, bisque, moccasin, navajowhite, peachpuff, mistyrose,             //
    lavenderblush, seashell, oldlace, ivory, honeydew, mintcream, azure, aliceblue, lavender, lightsteelblue, lightslategray,      //
    slategray, steelblue, royalblue, midnightblue, navy, darkblue, mediumblue, blue, dodgerblue, cornflowerblue, deepskyblue,      //
    lightskyblue, skyblue, lightblue, powderblue, paleturquoise, lightcyan, cyan, aqua, turquoise, mediumturquoise, darkturquoise, //
    lightseagreen, cadetblue, darkcyan, teal, darkslategray, darkgreen, green, forestgreen, seagreen, mediumseagreen,              //
    mediumaquamarine, darkseagreen, aquamarine, palegreen, lightgreen, springgreen, mediumspringgreen, lawngreen, chartreuse,      //
    greenyellow, lime, limegreen, yellowgreen, darkolivegreen, olivedrab, olive, darkkhaki, palegoldenrod, cornsilk, beige,        //
    lightyellow, lightgoldenrodyellow, lemonchiffon, wheat, burlywood, tan, khaki, yellow, gold, orange, sandybrown, darkorange,   //
    goldenrod, peru, darkgoldenrod, chocolate, sienna, saddlebrown, maroon, darkred, brown, firebrick, indianred, rosybrown,       //
    darksalmon, lightcoral, salmon, lightsalmon, coral, tomato, orangered, red, crimson, mediumvioletred, deeppink, hotpink,       //
    palevioletred, pink, lightpink, thistle, magenta, fuchsia, violet, plum, orchid, mediumorchid, darkorchid, darkviolet,         //
    darkmagenta, purple, indigo, darkslateblue, blueviolet, mediumpurple, slateblue, mediumslateblue, transparent, undefined, yw;

private:
  static constexpr void from_code(color& c, nat4 a) noexcept {
    if (!is_cev) xvstore(xvhex2rgb(a), &c.r);
    else c = {((0xff0000 & a) >> 16) / 255.f, ((0xff00 & a) >> 8) / 255.f, (0xff & a) / 255.f, ((0xff000000 & a) >> 24) / 255.f};
  }
};

inline constexpr color color::black{0x000000};
inline constexpr color color::dimgray{0x696969};
inline constexpr color color::gray{0x808080};
inline constexpr color color::darkgray{0xa9a9a9};
inline constexpr color color::silver{0xc0c0c0};
inline constexpr color color::lightgray{0xd3d3d3};
inline constexpr color color::gainsboro{0xdcdcdc};
inline constexpr color color::whitesmoke{0xf5f5f5};
inline constexpr color color::white{0xffffff};
inline constexpr color color::snow{0xfffafa};
inline constexpr color color::ghostwhite{0xf8f8ff};
inline constexpr color color::floralwhite{0xfffaf0};
inline constexpr color color::linen{0xfaf0e6};
inline constexpr color color::antiquewhite{0xfaebd7};
inline constexpr color color::papayawhip{0xffefd5};
inline constexpr color color::blanchedalmond{0xffebcd};
inline constexpr color color::bisque{0xffe4c4};
inline constexpr color color::moccasin{0xffe4b5};
inline constexpr color color::navajowhite{0xffdead};
inline constexpr color color::peachpuff{0xffdab9};
inline constexpr color color::mistyrose{0xffe4e1};
inline constexpr color color::lavenderblush{0xfff0f5};
inline constexpr color color::seashell{0xfff5ee};
inline constexpr color color::oldlace{0xfdf5e6};
inline constexpr color color::ivory{0xfffff0};
inline constexpr color color::honeydew{0xf0fff0};
inline constexpr color color::mintcream{0xf5fffa};
inline constexpr color color::azure{0xf0ffff};
inline constexpr color color::aliceblue{0xf0f8ff};
inline constexpr color color::lavender{0xe6e6fa};
inline constexpr color color::lightsteelblue{0xb0c4de};
inline constexpr color color::lightslategray{0x778899};
inline constexpr color color::slategray{0x708090};
inline constexpr color color::steelblue{0x4682b4};
inline constexpr color color::royalblue{0x4169e1};
inline constexpr color color::midnightblue{0x191970};
inline constexpr color color::navy{0x000080};
inline constexpr color color::darkblue{0x00008b};
inline constexpr color color::mediumblue{0x0000cd};
inline constexpr color color::blue{0x0000ff};
inline constexpr color color::dodgerblue{0x1e90ff};
inline constexpr color color::cornflowerblue{0x6495ed};
inline constexpr color color::deepskyblue{0x00bfff};
inline constexpr color color::lightskyblue{0x87cefa};
inline constexpr color color::skyblue{0x87ceeb};
inline constexpr color color::lightblue{0xadd8e6};
inline constexpr color color::powderblue{0xb0e0e6};
inline constexpr color color::paleturquoise{0xafeeee};
inline constexpr color color::lightcyan{0xe0ffff};
inline constexpr color color::cyan{0x00ffff};
inline constexpr color color::aqua{0x00ffff};
inline constexpr color color::turquoise{0x40e0d0};
inline constexpr color color::mediumturquoise{0x48d1cc};
inline constexpr color color::darkturquoise{0x00ced1};
inline constexpr color color::lightseagreen{0x20b2aa};
inline constexpr color color::cadetblue{0x5f9ea0};
inline constexpr color color::darkcyan{0x008b8b};
inline constexpr color color::teal{0x008080};
inline constexpr color color::darkslategray{0x2f4f4f};
inline constexpr color color::darkgreen{0x006400};
inline constexpr color color::green{0x008000};
inline constexpr color color::forestgreen{0x228b22};
inline constexpr color color::seagreen{0x2e8b57};
inline constexpr color color::mediumseagreen{0x3cb371};
inline constexpr color color::mediumaquamarine{0x66cdaa};
inline constexpr color color::darkseagreen{0x8fbc8f};
inline constexpr color color::aquamarine{0x7fffd4};
inline constexpr color color::palegreen{0x98fb98};
inline constexpr color color::lightgreen{0x90ee90};
inline constexpr color color::springgreen{0x00ff7f};
inline constexpr color color::mediumspringgreen{0x00fa9a};
inline constexpr color color::lawngreen{0x7cfc00};
inline constexpr color color::chartreuse{0x7fff00};
inline constexpr color color::greenyellow{0xadff2f};
inline constexpr color color::lime{0x00ff00};
inline constexpr color color::limegreen{0x32cd32};
inline constexpr color color::yellowgreen{0x9acd32};
inline constexpr color color::darkolivegreen{0x556b2f};
inline constexpr color color::olivedrab{0x6b8e23};
inline constexpr color color::olive{0x808000};
inline constexpr color color::darkkhaki{0xbdb76b};
inline constexpr color color::palegoldenrod{0xeee8aa};
inline constexpr color color::cornsilk{0xfff8dc};
inline constexpr color color::beige{0xf5f5dc};
inline constexpr color color::lightyellow{0xffffe0};
inline constexpr color color::lightgoldenrodyellow{0xfafad2};
inline constexpr color color::lemonchiffon{0xfffacd};
inline constexpr color color::wheat{0xf5deb3};
inline constexpr color color::burlywood{0xdeb887};
inline constexpr color color::tan{0xd2b48c};
inline constexpr color color::khaki{0xf0e68c};
inline constexpr color color::yellow{0xffff00};
inline constexpr color color::gold{0xffd700};
inline constexpr color color::orange{0xffa500};
inline constexpr color color::sandybrown{0xf4a460};
inline constexpr color color::darkorange{0xff8c00};
inline constexpr color color::goldenrod{0xdaa520};
inline constexpr color color::peru{0xcd853f};
inline constexpr color color::darkgoldenrod{0xb8860b};
inline constexpr color color::chocolate{0xd2691e};
inline constexpr color color::sienna{0xa0522d};
inline constexpr color color::saddlebrown{0x8b4513};
inline constexpr color color::maroon{0x800000};
inline constexpr color color::darkred{0x8b0000};
inline constexpr color color::brown{0xa52a2a};
inline constexpr color color::firebrick{0xb22222};
inline constexpr color color::indianred{0xcd5c5c};
inline constexpr color color::rosybrown{0xbc8f8f};
inline constexpr color color::darksalmon{0xe9967a};
inline constexpr color color::lightcoral{0xf08080};
inline constexpr color color::salmon{0xfa8072};
inline constexpr color color::lightsalmon{0xffa07a};
inline constexpr color color::coral{0xff7f50};
inline constexpr color color::tomato{0xff6347};
inline constexpr color color::orangered{0xff4500};
inline constexpr color color::red{0xff0000};
inline constexpr color color::crimson{0xdc143c};
inline constexpr color color::mediumvioletred{0xc71585};
inline constexpr color color::deeppink{0xff1493};
inline constexpr color color::hotpink{0xff69b4};
inline constexpr color color::palevioletred{0xdb7093};
inline constexpr color color::pink{0xffc0cb};
inline constexpr color color::lightpink{0xffb6c1};
inline constexpr color color::thistle{0xd8bfd8};
inline constexpr color color::magenta{0xff00ff};
inline constexpr color color::fuchsia{0xff00ff};
inline constexpr color color::violet{0xee82ee};
inline constexpr color color::plum{0xdda0dd};
inline constexpr color color::orchid{0xda70d6};
inline constexpr color color::mediumorchid{0xba55d3};
inline constexpr color color::darkorchid{0x9932cc};
inline constexpr color color::darkviolet{0x9400d3};
inline constexpr color color::darkmagenta{0x8b008b};
inline constexpr color color::purple{0x800080};
inline constexpr color color::indigo{0x4b0082};
inline constexpr color color::darkslateblue{0x483d8b};
inline constexpr color color::blueviolet{0x8a2be2};
inline constexpr color color::mediumpurple{0x9370db};
inline constexpr color color::slateblue{0x6a5acd};
inline constexpr color color::mediumslateblue{0x7b68ee};
inline constexpr color color::transparent = color(0x0, 0.0f);
inline constexpr color color::undefined = color(0x0, -1.f);
inline constexpr color color::yw{0x081020};

template<typename T> struct vector2 {
  T x{}, y{};
  constexpr vector2() noexcept = default;
  explicit constexpr vector2(const T& Val) noexcept : x(Val), y(Val) {}
  constexpr vector2(const T& X, const T& Y) noexcept : x(X), y(Y) {}
  template<convertible_to<T> U, convertible_to<T> V> constexpr vector2(U&& X, V&& Y) noexcept : x(fwd<U>(X)), y(fwd<V>(Y)) {}
  template<nat I> requires (I < 2) constexpr T& get() & noexcept { return select<I>(x, y); }
  template<nat I> requires (I < 2) constexpr T&& get() && noexcept { return mv(select<I>(x, y)); }
  template<nat I> requires (I < 2) constexpr const T& get() const& noexcept { return select<I>(x, y); }
  template<nat I> requires (I < 2) constexpr T get() const&& noexcept { return mv(select<I>(x, y)); }
};

template<typename T1, typename T2> vector2(T1, T2) -> vector2<fat4>;
}

namespace std {

template<typename T> struct tuple_size<yw::vector<T>> : integral_constant<nat, 4> {};
template<nat I, typename T> struct tuple_element<I, yw::vector<T>> : type_identity<T> {};

template<> struct tuple_size<yw::color> : integral_constant<nat, 4> {};
template<nat I> struct tuple_element<I, yw::color> : type_identity<float> {};

template<typename Ct> struct formatter<yw::color, Ct> : formatter<basic_string<Ct>, Ct> {
  static constexpr Ct fmt[] = {'R', 'G', 'B', 'A', '(', '{', '}', ',', ' ', '{', '}', ',', ' ', '{', '}', ',', ' ', '{', '}', ')', '\0'};
  auto format(const yw::color& c, auto& Ctx) const { return formatter<basic_string<Ct>, Ct>::format(std::format(fmt, c.r, c.g, c.b, c.a), Ctx); }
};

template<typename T> struct tuple_size<yw::vector2<T>> : integral_constant<nat, 2> {};
template<nat I, typename T> struct tuple_element<I, yw::vector2<T>> : type_identity<T> {};

}
