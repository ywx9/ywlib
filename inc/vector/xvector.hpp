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
  if constexpr (X < 4 && Y < 4 && Z < 4 && W < 4) return _xvpermute<X, Y, Z, W>(a);
  else if constexpr ((bx || X >= 4) && (by || Y >= 4) && (bz || Z >= 4) && (bw || W >= 4))
    return _xvpermute<X - 4, Y - 4, Z - 4, W - 4>(b);
  else if constexpr ((bx || !(X & 3 ^ 0)) && (by || !(Y & 3 ^ 1)) && (bz || !(Z & 3 ^ 2)) && (bw || !(W & 3 ^ 3)))
    return _mm_blend_ps(a, b, (X >= 4) | (Y >= 4) << 1 | (Z >= 4) << 2 | (W >= 4) << 3);
  else if constexpr (X < 4 && Y < 4 && (bz || Z >= 4) && (bw || W >= 4)) {
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
  } else if constexpr ((X < 4 || X == 4) && (Y < 4 || Y == 5) && (Z < 4 || Z == 6) && (W < 4 || W == 7))
    return _mm_blend_ps(_xvpermute<X, Y, Z, W>(a), b, X >= 4 | (Y >= 4) << 1 | (Z >= 4) << 2 | (W >= 4) << 3);
  else if constexpr ((bx || X >= 4 || X == 0) && (by || Y >= 4 || Y == 1) &&
                     (bz || Z >= 4 || Z == 2) && (bw || W >= 4 || W == 3))
    return _mm_blend_ps(a, _xvpermute<X - 4, Y - 4, Z - 4, W - 4>(b),
                        X >= 4 | (Y >= 4) << 1 | (Z >= 4) << 2 | (W >= 4) << 3);
  else if constexpr ((0 <= X && X < 4) + (0 <= Y && Y < 4) + (0 <= Z && Z < 4) + (0 <= W && W < 4) == 1) {
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




struct xvector {

  __m128&& m128;

  xvector()
};


/// extended vector type
using xvector = __m128;


/// constant extended vector type
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


/// constant extended vector; {0, 0, 0, 0}
inline constexpr xvconstant<0> xvzero;


/// constant extended vector; {-0, -0, -0, -0}
inline constexpr xvconstant<-0.> xvnegzero;


/// loads an extended vector from `ptr`
inline xvector xvload(const fat4* ptr) noexcept { return _mm_loadu_ps(ptr); }


/// fills an extended vector with `val`
inline xvector xvfill(const fat4 val) noexcept { return _mm_set1_ps(val); }


/// sets an extended vector with four values
inline xvector xvset(const fat4 x, const fat4 y, const fat4 z, const fat4 w)
  noexcept { return _mm_set_ps(w, z, y, x); }


/// stores an extended vector to `ptr`
inline void xvstore(fat4* ptr, const xvector& v)
  noexcept { _mm_storeu_ps(ptr, v); }


/// inserts the `J`-th element of `b` to `I`-th element of `a`
template<nat I, nat J, nat Zero = 0b0000>
requires ((I | J) < 4 && Zero < 16)
xvector xvinsert(const xvector& a, const xvector& b)
  noexcept { return _mm_insert_ps(a, b, (I << 4) | (J << 6) | Zero); }


/// extracts the `I`-th element of `a`
template<nat I> requires (I < 4)
fat4 xvextract(const xvector& a) noexcept {
  if constexpr (I == 0) return _mm_cvtss_f32(a);
  else return bitcast<fat4>(_mm_extract_ps(a, I));
}


/// blends two extended vectors
/// \return `{X ? b.x : a.x, Y ? b.y : a.y, Z ? b.z : a.z, W ? b.w : a.w}`
template<bool X, bool Y, bool Z, bool W>
xvector xvblend(const xvector& a, const xvector& b) noexcept {
  if constexpr (X && Y && Z && W) return b;
  else if constexpr (!(X || Y || Z || W)) return a;
  else return _mm_blend_ps(a, b, X | Y << 1 | Z << 2 | W << 3);
}


/// permutes an extended vector
/// \return `{a[X], a[Y], a[Z], a[W]}`
template<int X, int Y, int Z, int W>
requires ((X | Y | Z | W) < 4)
xvector xvpermute(const xvector& a) noexcept {
  return _::_xvpermute<X, Y, Z, W>(a);
}


/// permutes two extended vectors
template<int X, int Y, int Z, int W>
requires ((X | Y | Z | W) < 8)
xvector xvpermute(const xvector& a, const xvector& b) noexcept {
  return _::_xvpermute<X, Y, Z, W>(a, b);
}


/// checks if all elements of `A` are equal to `B`
inline xvector xveq(const xvector& a, const xvector& b)
  noexcept { return _mm_cmpeq_ps(a, b); }


/// checks if all elements of `A` are not equal to `B`
inline xvector xvne(const xvector& a, const xvector& b)
  noexcept { return _mm_cmpneq_ps(a, b); }


/// checks if all elements of `A` are less than `B`
inline xvector xvlt(const xvector& a, const xvector& b)
  noexcept { return _mm_cmplt_ps(a, b); }


/// checks if all elements of `A` are less than or equal to `B`
inline xvector xvle(const xvector& a, const xvector& b)
  noexcept { return _mm_cmple_ps(a, b); }


/// checks if all elements of `A` are greater than `B`
inline xvector xvgt(const xvector& a, const xvector& b)
  noexcept { return _mm_cmpgt_ps(a, b); }


/// checks if all elements of `A` are greater than or equal to `B`
inline xvector xvge(const xvector& a, const xvector& b)
  noexcept { return _mm_cmpge_ps(a, b); }


/// returns `{abs(a.x), ...}`
inline xvector xvabs(const xvector& a)
  noexcept { return _mm_andnot_ps(xvnegzero, a); }


/// returns `{-a.x, ...}`
inline xvector xvneg(const xvector& a)
  noexcept { return _mm_xor_ps(a, xvnegzero); }


/// returns `{a.x + b.x, ...}`
inline xvector xvadd(const xvector& a, const xvector& b)
  noexcept { return _mm_add_ps(a, b); }


/// returns `{a.x - b.x, ...}`
inline xvector xvsub(const xvector& a, const xvector& b)
  noexcept { return _mm_sub_ps(a, b); }


/// returns `{a.x * b.x, ...}`
inline xvector xvmul(const xvector& a, const xvector& b)
  noexcept { return _mm_mul_ps(a, b); }


/// returns `{a.x / b.x, ...}`
inline xvector xvdiv(const xvector& a, const xvector& b)
  noexcept { return _mm_div_ps(a, b); }


/// returns `{a.x * b.x + c.x, ...}`
inline xvector xvfma(const xvector& a, const xvector& b, const xvector& c)
  noexcept { return _mm_fmadd_ps(a, b, c); }


/// returns `{a.x * b.x - c.x, ...}`
inline xvector xvfms(const xvector& a, const xvector& b, const xvector& c)
  noexcept { return _mm_fmsub_ps(a, b, c); }


/// returns `{-a.x * b.x + c.x, ...}`
inline xvector xvfnma(const xvector& a, const xvector& b, const xvector& c)
  noexcept { return _mm_fnmadd_ps(a, b, c); }


/// returns `{-a.x * b.x - c.x, ...}`
inline xvector xvfnms(const xvector& a, const xvector& b, const xvector& c)
  noexcept { return _mm_fnmsub_ps(a, b, c); }


/// returns `{ceil(a.x), ...}`
inline xvector xvceil(const xvector& a)
  noexcept { return _mm_ceil_ps(a); }


/// returns `{floor(a.x), ...}`
inline xvector xvfloor(const xvector& a)
  noexcept { return _mm_floor_ps(a); }


/// returns `{round(a.x), ...}`
inline xvector xvround(const xvector& a)
  noexcept { return _mm_round_ps(
    a, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC); }


/// returns `{trunc(a.x), ...}`
inline xvector xvtrunc(const xvector& a)
  noexcept { return _mm_round_ps(
    a, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC); }


/// returns `{max(a.x, b.x), ...}`
inline xvector xvmax(const xvector& a, const xvector& b)
  noexcept { return _mm_max_ps(a, b); }


/// returns `{min(a.x, b.x), ...}`
inline xvector xvmin(const xvector& a, const xvector& b)
  noexcept { return _mm_min_ps(a, b); }


/// returns `{cos(a.x), ...}`
inline xvector xvcos(const xvector& a)
  noexcept { return _mm_cos_ps(a); }


/// returns `{sin(a.x), ...}`
inline xvector xvsin(const xvector& a)
  noexcept { return _mm_sin_ps(a); }


/// returns `{tan(a.x), ...}`
inline xvector xvtan(const xvector& a)
  noexcept { return _mm_tan_ps(a); }


/// returns `{acos(a.x), ...}`
inline xvector xvacos(const xvector& a)
  noexcept { return _mm_acos_ps(a); }


/// returns `{asin(a.x), ...}`
inline xvector xvasin(const xvector& a)
  noexcept { return _mm_asin_ps(a); }


/// returns `{atan(a.x), ...}`
inline xvector xvatan(const xvector& a)
  noexcept { return _mm_atan_ps(a); }


/// returns `{cosh(a.x), ...}`
inline xvector xvcosh(const xvector& a)
  noexcept { return _mm_cosh_ps(a); }


/// returns `{sinh(a.x), ...}`
inline xvector xvsinh(const xvector& a)
  noexcept { return _mm_sinh_ps(a); }


/// returns `{tanh(a.x), ...}`
inline xvector xvtanh(const xvector& a)
  noexcept { return _mm_tanh_ps(a); }


/// returns `{acosh(a.x), ...}`
inline xvector xvacosh(const xvector& a)
  noexcept { return _mm_acosh_ps(a); }


/// returns `{asinh(a.x), ...}`
inline xvector xvasinh(const xvector& a)
  noexcept { return _mm_asinh_ps(a); }


/// returns `{atanh(a.x), ...}`
inline xvector xvatanh(const xvector& a)
  noexcept { return _mm_atanh_ps(a); }


/// returns `xvsin(a)` and assigns `xvcos(a)` to `cos`
inline xvector xvsincos(const xvector& a, xvector& cos)
  noexcept { return _mm_sincos_ps(&cos, a); }


/// return `{atan2(y.x, x.x), ...}`
inline xvector xvatan2(const xvector& y, const xvector& x)
  noexcept { return _mm_atan2_ps(y, x); }


/// returns `{pow(a.x, b.x), ...}`
inline xvector xvpow(const xvector& a, const xvector& b)
  noexcept { return _mm_pow_ps(a, b); }


/// returns `{exp(a.x), ...}`
inline xvector xvexp(const xvector& a)
  noexcept { return _mm_exp_ps(a); }


/// returns `{exp2(a.x), ...}`
inline xvector xvexp2(const xvector& a)
  noexcept { return _mm_exp2_ps(a); }


/// returns `{exp10(a.x), ...}`
inline xvector xvexp10(const xvector& a)
  noexcept { return _mm_exp10_ps(a); }


/// returns `{expm1(a.x), ...}`
inline xvector xvexpm1(const xvector& a)
  noexcept { return _mm_expm1_ps(a); }


/// returns `{ln(a.x), ...}`
inline xvector xvln(const xvector& a)
  noexcept { return _mm_log_ps(a); }


/// returns `{log2(a.x), ...}`
inline xvector xvlog2(const xvector& a)
  noexcept { return _mm_log2_ps(a); }


/// returns `{log10(a.x), ...}`
inline xvector xvlog10(const xvector& a)
  noexcept { return _mm_log10_ps(a); }


/// returns `{log1p(a.x), ...}`
inline xvector xvlog1p(const xvector& a)
  noexcept { return _mm_log1p_ps(a); }


/// returns `{logb(a.x), ...}`
inline xvector xvlogb(const xvector& a)
  noexcept { return _mm_logb_ps(a); }


/// returns `{sqrt(a.x), ...}`
inline xvector xvsqrt(const xvector& a)
  noexcept { return _mm_sqrt_ps(a); }


/// returns `{1 / sqrt(a.x), ...}`
inline xvector xvsqrt_r(const xvector& a)
  noexcept { return _mm_invsqrt_ps(a); }


/// returns `{cbrt(a.x), ...}`
inline xvector xvcbrt(const xvector& a)
  noexcept { return _mm_cbrt_ps(a); }


/// returns `{1 / cbrt(a.x), ...}`
inline xvector xvcbrt_r(const xvector& a)
  noexcept { return _mm_invcbrt_ps(a); }


/// returns `{hypot(a.x, b.x), ...}`
inline xvector xvhypot(const xvector& a, const xvector& b)
  noexcept { return _mm_hypot_ps(a, b); }


/// returns `{erf(a.x), ...}`
inline xvector xverf(const xvector& a)
  noexcept { return _mm_erf_ps(a); }


/// returns `{erfc(a.x), ...}`
inline xvector xverfc(const xvector& a)
  noexcept { return _mm_erfc_ps(a); }


/// returns `{1 / erf(a.x), ...}`
inline xvector xverf_r(const xvector& a)
  noexcept { return _mm_erfinv_ps(a); }


/// returns `{1 / erfc(a.x), ...}`
inline xvector xverfc_r(const xvector& a)
  noexcept { return _mm_erfcinv_ps(a); }


/// returns dot product of two extended vectors
template<nat N, nat Zero = 0b0000>
requires (N <= 4 && Zero < 16)
inline xvector xvdot(const xvector& a, const xvector& b) noexcept {
  if constexpr (N == 0) return _mm_setzero_ps();
  else if constexpr (N == 1 && (bitcount(Zero) == 3)) {
    constexpr nat i = inspects<!(Zero & 1), !(Zero & 2), !(Zero & 4), !(Zero & 8)>;
    return xvinsert<i, 0, Zero>(a, xvmul(a, b));
  } else if constexpr (N == 1 && Zero == 0) return xvpermute<0, 0, 0, 0>(xvmul(a, b));
  else return _mm_dp_ps(a, b, value_switch<N, 0, 16, 48, 112, 240> | (15 ^ Zero));
}


/// returns cross product of two extended vectors
inline xvector xvcross(const xvector& a, const xvector& b) noexcept {
  return xvfms(xvpermute<2, 0, 1, 3>(a), xvpermute<1, 2, 0, 3>(b),
               xvmul(xvpermute<1, 2, 0, 3>(a), xvpermute<2, 0, 1, 3>(b)));
}


/// returns length of an extended vector; `xvsqrt(xvdot<N, Zero>(a, a))`
template<nat N, nat Zero = 0> requires (N <= 4 && Zero < 16)
constexpr xvector xvlength(const xvector& a) noexcept {
  return xvsqrt(xvdot<N, Zero>(a, a));
}


/// returns reciprocal length of an extended vector; `xvsqrt_r(xvdot<N, Zero>(a, a))`
template<nat N, nat Zero = 0> requires (0 < N && N <= 4 && Zero < 16)
constexpr xvector xvlength_r(const xvector& a) noexcept {
  return xvsqrt_r(xvdot<N, Zero>(a, a));
}


/// returns normalized vector; `xvmul(a, xvlength_r<N>(a))`
template<nat N> requires(0 < N && N <= 4)
constexpr xvector xvnormalize(const xvector& a) noexcept {
  return xvmul(a, xvlength_r<N>(a));
}


/// converts degrees to radians
inline xvector xvradian(const xvector& a) noexcept {
  return xvmul(a, xvconstant<pi / 180>{});
}


/// converts radians to degrees
inline xvector xvdegree(const xvector& a) noexcept {
  return xvmul(a, xvconstant<180 / pi>{});
}



inline constexpr auto xvtranslate = []<typename T>(const xvector& Offset, T&& Result) noexcept requires(nt_tuple_for<T, xvector&>) {
  constexpr nat N = extent<T>;
  if constexpr (N >= 1) get<0>(Result) = xvinsert<3, 0>(xv_x, Offset);
  if constexpr (N >= 2) get<1>(Result) = xvinsert<3, 1>(xv_y, Offset);
  if constexpr (N >= 3) get<2>(Result) = xvinsert<3, 2>(xv_z, Offset);
  if constexpr (N >= 4) get<3>(Result) = xv_w; };

inline constexpr auto xvrotate = []<nt_tuple_for<xvector&> T>(const xvector& Radian, T&& Result) noexcept {
  constexpr nat N = extent<T>;
  if constexpr (N == 2) {
    get<1>(Result) = xvsin(Radian, get<0>(Result));
    get<0>(Result) = xvinsert<1, 0, 0b1100>(get<0>(Result), get<1>(Result));
    get<1>(Result) = xvinsert<1, 0, 0b1100>(get<1>(Result), get<0>(Result));
    get<0>(Result) = xvnegate<0, 1, 0, 0>(get<0>(Result));
  } else if constexpr (N == 3) {
    get<0>(Result) = xvsin(Radian, get<1>(Result));
    get<2>(Result) = xvpermute<4, 6, 0, 2>(get<0>(Result), get<1>(Result));
    auto temp = xvpermute<3, 0, 1, 2>(get<2>(Result));
    get<2>(Result) = xvmul(get<2>(Result), temp);
    get<1>(Result) = xvmul(xvpermute<1, 1, 1, 1>(get<1>(Result)), temp);
    get<0>(Result) = xvpermute<1, 1, 1, 1>(get<0>(Result));
    temp = _mm_addsub_ps(xvpermute<2, 3, 0, 1>(xvmul(get<0>(Result), get<2>(Result))), get<2>(Result));
    get<2>(Result) = xvinsert<0, 0, 0b1000>(xvpermute<-1, 3, 1, -1>(get<1>(Result)), xvneg(get<0>(Result)));
    get<0>(Result) = xvinsert<0, 2, 0b1000>(xvpermute<-1, 0, 3, -1>(temp), get<1>(Result));
    get<1>(Result) = xvinsert<0, 0, 0b1000>(temp, get<1>(Result));
  } else if constexpr (N == 4) {
    get<0>(Result) = xvsin(Radian, get<1>(Result));
    get<2>(Result) = xvpermute<4, 6, 0, 2>(get<0>(Result), get<1>(Result));
    get<3>(Result) = xvpermute<3, 0, 1, 2>(get<2>(Result));
    get<2>(Result) = xvmul(get<2>(Result), get<3>(Result));
    get<1>(Result) = xvmul(xvpermute<1, 1, 1, 1>(get<1>(Result)), get<3>(Result));
    get<0>(Result) = xvpermute<1, 1, 1, 1>(get<0>(Result));
    get<3>(Result) = _mm_addsub_ps(xvpermute<2, 3, 0, 1>(xvmul(get<0>(Result), get<2>(Result))), get<2>(Result));
    get<2>(Result) = xvinsert<0, 0, 0b1000>(xvpermute<-1, 3, 1, -1>(get<1>(Result)), xvneg(get<0>(Result)));
    get<0>(Result) = xvinsert<0, 2, 0b1000>(xvpermute<-1, 0, 3, -1>(get<3>(Result)), get<1>(Result));
    get<1>(Result) = xvinsert<0, 0, 0b1000>(get<3>(Result), get<1>(Result));
    get<3>(Result) = xv_w;
  } else static_assert(1 < N && N < 5); };

inline constexpr auto xvrotate_inv = []<nt_tuple_for<xvector&> T>(const xvector& Radian, T&& Result) noexcept {
  constexpr nat N = extent<T>;
  if constexpr (N == 2) {
    get<1>(Result) = xvsin(Radian, get<0>(Result));
    get<0>(Result) = xvinsert<1, 0, 0b1100>(get<0>(Result), get<1>(Result));
    get<1>(Result) = xvinsert<1, 0, 0b1100>(get<1>(Result), get<0>(Result));
    get<1>(Result) = xvnegate<0, 1, 0, 0>(get<1>(Result));
  } else if constexpr (N == 3) {
    get<0>(Result) = xvsin(Radian, get<1>(Result));
    get<2>(Result) = xvpermute<4, 6, 0, 2>(get<0>(Result), get<1>(Result));
    auto temp = xvpermute<3, 0, 1, 2>(get<2>(Result));
    get<2>(Result) = xvmul(get<2>(Result), temp);
    get<1>(Result) = xvmul(xvpermute<1, 1, 1, 1>(get<1>(Result)), temp);
    get<0>(Result) = xvpermute<1, 1, 1, 1>(get<0>(Result));
    temp = _mm_addsub_ps(xvpermute<2, 3, 0, 1>(xvmul(get<0>(Result), get<2>(Result))), get<2>(Result));
    get<2>(Result) = xvinsert<2, 1, 0b1000>(xvpermute<3, 2, -1, -1>(temp), get<1>(Result));
    get<0>(Result) = xvinsert<2, 2, 0b1000>(xvpermute<2, 0, -1, -1>(get<1>(Result)), xvneg(get<0>(Result)));
    get<1>(Result) = xvinsert<2, 3, 0b1000>(temp, get<1>(Result));
  } else if constexpr (N == 4) {
    get<0>(Result) = xvsin(Radian, get<1>(Result));
    get<2>(Result) = xvpermute<4, 6, 0, 2>(get<0>(Result), get<1>(Result));
    get<3>(Result) = xvpermute<3, 0, 1, 2>(get<2>(Result));
    get<2>(Result) = xvmul(get<2>(Result), get<3>(Result));
    get<1>(Result) = xvmul(xvpermute<1, 1, 1, 1>(get<1>(Result)), get<3>(Result));
    get<0>(Result) = xvpermute<1, 1, 1, 1>(get<0>(Result));
    get<3>(Result) = _mm_addsub_ps(xvpermute<2, 3, 0, 1>(xvmul(get<0>(Result), get<2>(Result))), get<2>(Result));
    get<2>(Result) = xvinsert<2, 1, 0b1000>(xvpermute<3, 2, -1, -1>(get<3>(Result)), get<1>(Result));
    get<0>(Result) = xvinsert<2, 2, 0b1000>(xvpermute<2, 0, -1, -1>(get<1>(Result)), xvneg(get<0>(Result)));
    get<1>(Result) = xvinsert<2, 3, 0b1000>(get<3>(Result), get<1>(Result));
    get<3>(Result) = xv_w;
  } else static_assert(1 < N && N < 5); };

inline constexpr auto xvscale = []<nt_tuple_for<xvector&> T>(const xvector& Scalar, T&& Result) noexcept {
  constexpr nat N = extent<T>;
  if constexpr (N >= 2) { get<0>(Result) = xvinsert<0, 0, 0b1110>(Scalar, Scalar); get<1>(Result) = xvinsert<1, 1, 0b1101>(Scalar, Scalar); }
  if constexpr (N >= 3) get<2>(Result) = xvinsert<2, 2, 0b1011>(Scalar, Scalar);
  if constexpr (N == 4) get<3>(Result) = xv_w;
  static_assert(1 < N && N < 5); };

inline constexpr overload xvworld{
  []<nt_tuple_for<xvector&> T>(T&& Result) noexcept {
    constexpr nat N = extent<T>;
    if constexpr (N >= 2) Result[0] = xv_x, Result[1] = xv_y;
    if constexpr (N >= 3) Result[2] = xv_z;
    if constexpr (N == 4) Result[3] = xv_w;
    static_assert(1 < N && N < 5); },
  []<nt_tuple_for<xvector&> T>(const xvector& Offset, T&& Result) noexcept { xvtranslate(Offset, Result); },
  []<nt_tuple_for<xvector&> T>(const xvector& Offset, const xvector& Radian, T&& Result) noexcept {
    xvrotate(Radian, Result), Result[0] = xvinsert<3, 0>(Result[0], Offset), Result[1] = xvinsert<3, 1>(Result[1], Offset);
    if constexpr (extent<T> >= 3) Result[2] = xvinsert<3, 2>(Result[2], Offset);
    static_assert(1 < extent<T> && extent<T> < 5); },
  []<nt_tuple_for<xvector&> T>(const xvector& Offset, const xvector& Radian, const xvector& Scalar, T&& Result) noexcept {
    xvrotate(Radian, Result), Result[0] = xvinsert<3, 0>(Result[0], Offset), Result[1] = xvinsert<3, 1>(Result[1], Offset);
    if constexpr (extent<T> >= 3) Result[2] = xvinsert<3, 2>(Result[2], Offset);
    Result[0] = xvmul(Result[0], xvpermute<0, 0, 0, 0>(Scalar)), Result[1] = xvmul(Result[1], xvpermute<1, 1, 1, 1>(Scalar));
    if constexpr (extent<T> >= 3) Result[2] = xvmul(Result[2], xvpermute<2, 2, 2, 2>(Scalar));
    static_assert(1 < extent<T> && extent<T> < 5); }};

// clang-format off

inline constexpr overload xvview{
  []<nt_tuple_for<xvector&> T, nat N = extent<T>>(const xvector& Radian, T&& Result) noexcept requires (le(2, N) && le(N, 4)) { xvrotate_inv(Radian, Result); },
  []<nt_tuple_for<xvector&> T, nat N = extent<T>>(const xvector& Position, const xvector& Radian, T&& Result) noexcept requires (le(2, N) && le(N, 4)) {
    xvrotate_inv(Radian, Result);
    if constexpr (N < 4) {
      auto temp = xvneg(Position);
      Result[0] = xvinsert<3, 3>(Result[0], xvdot<3>(Result[0], temp));
      Result[1] = xvinsert<3, 3>(Result[1], xvdot<3>(Result[1], temp));
      if constexpr (N == 3) Result[2] = xvinsert<3, 3>(Result[2], xvdot<3>(Result[2], temp));
    } else {
      Result[3] = xvneg(Position);
      Result[0] = xvinsert<3, 3>(Result[0], xvdot<3>(Result[0], Result[3]));
      Result[1] = xvinsert<3, 3>(Result[1], xvdot<3>(Result[1], Result[3]));
      Result[2] = xvinsert<3, 3>(Result[2], xvdot<3>(Result[2], Result[3]));
      Result[3] = xv_w;
    }},
  []<nt_tuple_for<xvector&> T, nat N = extent<T>>(const xvector& Position, const xvector& Radian, const xvector& Offset, T&& Result) noexcept requires (le(2, N) && le(N, 4)) {
    xvrotate_inv(Radian, Result);
    if constexpr (N < 4) {
      auto temp = xvneg(Position);
      Result[0] = xvsub(xvinsert<3, 3>(Result[0], xvdot<3>(Result[0], temp)), xvinsert<3, 0, 0b0111>(Offset, Offset));
      Result[1] = xvsub(xvinsert<3, 3>(Result[1], xvdot<3>(Result[1], temp)), xvinsert<3, 1, 0b0111>(Offset, Offset));
      if constexpr (N == 3) Result[2] = xvsub(xvinsert<3, 3>(Result[2], xvdot<3>(Result[2], temp)), xvinsert<3, 2, 0b0111>(Offset, Offset));
    } else {
      Result[3] = xvneg(Position);
      Result[0] = xvsub(xvinsert<3, 3>(Result[0], xvdot<3>(Result[0], Result[3])), xvinsert<3, 0, 0b0111>(Offset, Offset));
      Result[1] = xvsub(xvinsert<3, 3>(Result[1], xvdot<3>(Result[1], Result[3])), xvinsert<3, 1, 0b0111>(Offset, Offset));
      Result[2] = xvsub(xvinsert<3, 3>(Result[2], xvdot<3>(Result[2], Result[3])), xvinsert<3, 2, 0b0111>(Offset, Offset));
      Result[3] = xv_w;
    }}};

inline constexpr overload xvcamera{
  []<nat N>(const float Width, const float Height, const float Factor, xvector (&Result)[N]) noexcept requires (le(3, N) && le(N, 4)) {
    constexpr value f = 1048576.0, n = 0.25;
    Result[0] = xvset<0>(xv_zero, -Factor * Height / Width);
    Result[1] = xvset<1>(xv_zero, Factor);
    Result[2] = xv_constant<0, 0, f / (f - n), -f * n / (f - n)>;
    if constexpr (N == 4) Result[3] = xv_z; },
  []<nat N>(const float Width, const float Height, const float Factor, none OrthgraphicMode, xvector (&Result)[N]) noexcept requires (le(3, N) && le(N, 4)) {
    constexpr value f = 1048576.0, n = 0.25;
    Result[0] = xvset<0>(xv_zero, -2.0f * Factor / Width);
    Result[1] = xvset<1>(xv_zero, 2.0f * Factor / Height);
    Result[2] = xv_constant<0, 0, 1 / (f - n), -n / (f - n)>;
    if constexpr (N == 4) Result[3] = xv_w; }};

// clang-format on

/// @brief converts a rotation matrix to euler angles
inline constexpr auto xveuler = []<tuple Tp>(Tp&& Matrix) noexcept {
  if (extent<Tp> >= 3) {
    xvector v = xvneg(xvpermute<3, 0, 3, 3>(get<2>(Matrix)));
    if (xveq(v, xv_y)) return xvasin(xvpermute<5, 1, -1, -1>(v, get<0>(Matrix)));
    v = xvpermute<0, 1, 6, 3>(xvpermute<4, 1, -1, 3>(v, xvdiv(get<1>(Matrix), get<0>(Matrix))), xvdiv(xvpermute<-1, -1, 1, -1>(get<2>(Matrix)), get<2>(Matrix)));
    return xvpermute<6, 1, 4, 3>(xvasin(v), xvatan(xvpermute<0, -1, 5, -1>(get<1>(Matrix), get<2>(Matrix)), xvblend<0, 0, 1, 1>(get<0>(Matrix), get<2>(Matrix))));
  }
};



} ////////////////////////////////////////////////////////////////////////////// namespace yw
