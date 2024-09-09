/// \file ywlib-xv.hpp

#pragma once

#include "ywlib-core.hpp"

export namespace yw {

using xvector = intrin::m128;
using xmatrix = array<xvector, 4>;

/// \return `{p[0], p[1], p[2], p[3]}`
inline xvector xvload(const fat4* p) noexcept {
  return intrin::mm_loadu_ps(p);
}

/// \return `{x, y, z, w}`
inline xvector xvset(numeric auto&& x, numeric auto&& y,
                     numeric auto&& z, numeric auto&& w) noexcept  {
  return intrin::mm_set_ps(fat4(d), fat4(c), fat4(b), fat4(a));
}

/// \return `{x, y, z, 0}`
inline xvector xvset(numeric auto&& x, numeric auto&& y,
                     numeric auto&& z) noexcept {
  return intrin::mm_set_ps(fat4(c), fat4(b), fat4(a), 0.0f);
}

/// \return `{x, y, 0, 0}`
inline xvector xvset(numeric auto&& x, numeric auto&& y) noexcept {
  return intrin::mm_set_ps(fat4(b), fat4(a), 0.0f, 0.0f);
}

/// \return `{x, 0, 0, 0}`
inline xvector xvset(numeric auto&& x) noexcept {
  return intrin::mm_set_ss(fat4(a));
}

/// \return `{x, x, x, x}`
inline xvector xvfill(numeric auto&& x) noexcept {
  return intrin::mm_set1_ps(fat4(a));
}

/// \return `{0, 0, 0, 0}`
inline xvector xvfill() noexcept {
  return intrin::mm_setzero_ps();
}

/// stores `a` to `p`
inline void xvstore(fat4* p, const xvector& a) noexcept {
  intrin::mm_storeu_ps(p, a);
}

/// inserts `x` to `a` at `i`
template<nat i> requires (i < 4)
xvector xvinsert(const xvector& a, numeric auto&& v) noexcept {
  return intrin::mm_castsi128_ps(intrin::mm_insert_epi32(
    intrin::mm_castps_si128(a), xvcast<int>(fat4(v)), i));
}

/// inserts the `From`-th element of `src` to `dst` at `To`
template<nat To, nat From, nat Zero = 0>
requires ((To | From) < 4 && Zero < 16)
xvector xvinsert(const xvector& dst, const xvector& src) noexcept {
  return intrin::mm_insert_ps(dst, src, From << 6 | To << 4 | Zero);
}

/// extracts the `i`-th element of `a`
template<nat i> requires (i < 4)
fat4 xvextract(const xvector& a) noexcept {
  if constexpr (i == 0) return intrin::mm_cvtss_f32(a);
  else return bitcast<fat4>(intrin::mm_extract_ps(a, i));
}

/// \return `{Mask[0] ? b[0] : a[0], ...}`
template<nat Mask> requires (Mask < 16)
xvector xvblend(const xvector& a, const xvector& b) noexcept {
  return intrin::mm_blend_ps(a, b, Mask);
}

/// \return `{X ? b[0] : a[0], ...}`
template<bool X, bool Y, bool Z, bool W>
xvector xvblend(const xvector& a, const xvector& b) noexcept {
  return intrin::mm_blend_ps(a, b, X | Y << 1 | Z << 2 | W << 3);
}

ywlib_namespace_internal_begin
template<int X, int Y, int Z, int W>
xvector _xvpermute(const xvector& a) noexcept {
  constexpr auto f = [](auto a, auto b) { return a == b || a < 0 || 4 <= a; };
  constexpr auto x = f(X, 0) ? 0 : X, y = f(Y, 1) ? 1 : Y, z = f(Z, 2) ? 2 : Z, w = f(W, 3) ? 3 : W;
  if constexpr (f(X, 0) && f(Y, 1) && f(Z, 2) && f(W, 3)) return a;
  else if constexpr (f(X, 0) && f(Y, 0) && f(Z, 0) && f(W, 0)) return intrin::mm_broadcastss_ps(a);
  else if constexpr (f(X, 0) && f(Y, 0) && f(Z, 2) && f(W, 2)) return intrin::mm_moveldup_ps(a);
  else if constexpr (f(X, 1) && f(Y, 1) && f(Z, 3) && f(W, 3)) return intrin::mm_movehdup_ps(a);
  else return intrin::mm_permute_ps(a, x | y << 2 | z << 4 | w << 6);
}
template<int X, int Y, int Z, int W>
xvector _xvpermute(const xvector& a, const xvector& b) noexcept {
  constexpr bool bx = X < 0, by = Y < 0, bz = Z < 0, bw = W < 0;
  if constexpr (X < 4 && Y < 4 && Z < 4 && W < 4) {
    return _xvpermute<X, Y, Z, W>(a);
  } else if constexpr ((bx || X >= 4) && (by || Y >= 4) && (bz || Z >= 4) && (bw || W >= 4)) {
    return _xvpermute<X - 4, Y - 4, Z - 4, W - 4>(b);
  } else if constexpr ((bx || !(X & 3 ^ 0)) && (by || !(Y & 3 ^ 1)) && (bz || !(Z & 3 ^ 2)) && (bw || !(W & 3 ^ 3))) {
    return xvblend<ge(X, 4), ge(Y, 4), ge(Z, 4), ge(W, 4)>(a, b);
  } else if constexpr (X < 4 && Y < 4 && (bz || Z >= 4) && (bw || W >= 4)) {
    constexpr auto x = bx ? 0 : X, y = by ? 1 : Y, z = bz ? 2 : Z - 4, w = bw ? 3 : W - 4;
    return intrin::mm_shuffle_ps(a, b, x | y << 2 | z << 4 | w << 6);
  } else if constexpr ((bx || X >= 4) && (by || Y >= 4) && Z < 4 && W < 4) {
    constexpr auto x = bx ? 0 : X - 4, y = by ? 1 : Y - 4, z = f(Z) ? 2 : Z, w = f(W) ? 3 : W;
    return intrin::mm_shuffle_ps(b, a, x | y << 2 | z << 4 | w << 6);
  } else if constexpr ((bx || X == 0) + (by || Y == 1) + (f(Z) || Z == 2) + (f(W) || W == 3) == 3) {
    constexpr auto x = bx ? 0 : X, y = by ? 1 : Y, z = f(Z) ? 2 : Z, w = f(W) ? 3 : W;
    constexpr nat i = inspects<ge(x, 4), ge(y, 4), ge(z, 4), ge(w, 4)>;
    return xvinsert<i, select_value<i, x, y, z, w> - 4>(a, b);
  } else if constexpr ((bx || X == 4) + (by || Y == 5) + (f(Z) || Z == 6) + (f(W) || W == 7) == 3) {
    constexpr auto x = bx ? 4 : X, y = by ? 5 : Y, z = f(Z) ? 6 : Z, w = f(W) ? 7 : W;
    constexpr nat i = inspects<lt(x, 4), lt(y, 4), lt(z, 4), lt(w, 4)>;
    return xvinsert<i, select_value<i, x, y, z, w>>(b, a);
  } else if constexpr ((X < 4 || X == 4) && (Y < 4 || Y == 5) && (Z < 4 || Z == 6) && (W < 4 || W == 7)) {
    return xvblend<ge(X, 4), ge(Y, 4), ge(Z, 4), ge(W, 4)>(_xvpermute<X, Y, Z, W>(a), b);
  } else if constexpr ((bx || X >= 4 || X == 0) && (by || Y >= 4 || Y == 1) &&
                       (bz || Z >= 4 || Z == 2) && (bw || W >= 4 || W == 3)) {
    return xvblend<ge(X, 4), ge(Y, 4), ge(Z, 4), ge(W, 4)>(a, _xvpermute<X - 4, Y - 4, Z - 4, W - 4>(b));
  } else if constexpr ((0 <= X && X < 4) + (0 <= Y && Y < 4) + (0 <= Z && Z < 4) + (0 <= W && W < 4) == 1) {
    constexpr nat i = inspects<(0 <= X && X < 4), (0 <= Y && Y < 4), (0 <= Z && Z < 4), (0 <= W && W < 4)>;
    return xvinsert<i, select_value<i, X, Y, Z, W>>(_xvpermute<X - 4, Y - 4, Z - 4, W - 4>(b), a);
  } else if constexpr ((4 <= X) + (4 <= Y) + (4 <= Z) + (4 <= W) == 1) {
    constexpr nat i = inspects<(4 <= X), (4 <= Y), (4 <= Z), (4 <= W)>;
    return xvinsert<i, select_value<i, X, Y, Z, W> - 4>(_xvpermute<X, Y, Z, W>(a), b);
  } else return xvblend<ge(X, 4), ge(Y, 4), ge(Z, 4), ge(W, 4)>(
    _xvpermute<X, Y, Z, W>(a), _xvpermute<X - 4, Y - 4, Z - 4, W - 4>(b));
}
ywlib_namespace_internal_end

/// permutes the elements of `a`
/// \note Negative values of template parameters means ignoring the value, which may optimize the performance.
template<int X, int Y, int Z, int W>
requires (X < 4 && Y < 4 && Z < 4 && W < 4)
xvector xvpermute(const xvector& a) noexcept {
  return _xvpermute<X, Y, Z, W>(a);
}

/// permutes the elements of `a` and `b`
/// \note Negative values of template parameters means ignoring the value, which may optimize the performance.
template<int X, int Y, int Z, int W>
requires (X < 8 && Y < 8 && Z < 8 && W < 8)
xvector xvpermute(const xvector& a, const xvector& b) noexcept {
  return _xvpermute<X, Y, Z, W>(a, b);
}

/// constant vector
template<value X, value Y = X, value Z = Y, value W = Z>
struct xvconstant {
  operator const xvector&() const noexcept {
    static const xvector v = intrin::mm_set_ps(fat4(W), fat4(Z), fat4(Y), fat4(X));
    return v;
  }
};

/// performs equality comparison
inline bool operator==(const xvector& a, const xvector& b) noexcept {
  return intrin::mm_test_all_ones(
    intrin::mm_castps_si128(intrin::mm_cmpeq_ps(a, b)));
}

/// performs three-way comparison
inline po_t operator<=>(const xvector& a, const xvector& b) noexcept {
  using namespace intrin;
  return mm_movemask_ps(xvpermute<3, 2, 1, 0>(mm_cmpgt_ps(a, b))) <=>
         mm_movemask_ps(xvpermute<3, 2, 1, 0>(mm_cmplt_ps(a, b)));
}

/// performs unary plus
inline xvector operator+(const xvector& a) noexcept {
  return a;
}

/// performs unary minus
inline xvector operator-(const xvector& a) noexcept {
  return intrin::mm_xor_ps(a, xvconstant<-0.0f>());
}

/// performs addition
inline xvector operator+(const xvector& a, const xvector& b) noexcept {
  return intrin::mm_add_ps(a, b);
}

/// performs subtraction
inline xvector operator-(const xvector& a, const xvector& b) noexcept {
  return intrin::mm_sub_ps(a, b);
}

/// performs multiplication
inline xvector operator*(const xvector& a, const xvector& b) noexcept {
  return intrin::mm_mul_ps(a, b);
}

/// performs division
inline xvector operator/(const xvector& a, const xvector& b) noexcept {
  return intrin::mm_div_ps(a, b);
}

/// performs addition assignment
inline xvector& operator+=(xvector& a, const xvector& b) noexcept {
  return a = a + b, a;
}

/// performs subtraction assignment
inline xvector& operator-=(xvector& a, const xvector& b) noexcept {
  return a = a - b, a;
}

/// performs multiplication assignment
inline xvector& operator*=(xvector& a, const xvector& b) noexcept {
  return a = a * b, a;
}

/// performs division assignment
inline xvector& operator/=(xvector& a, const xvector& b) noexcept {
  return a = a / b, a;
}

/// performs bitwise AND
inline xvector operator&(const xvector& a, const xvector& b) noexcept {
  return intrin::mm_and_ps(a, b);
}

/// performs bitwise OR
inline xvector operator|(const xvector& a, const xvector& b) noexcept {
  return intrin::mm_or_ps(a, b);
}

/// performs bitwise XOR
inline xvector operator^(const xvector& a, const xvector& b) noexcept {
  return intrin::mm_xor_ps(a, b);
}

/// performs bitwise NOT
inline xvector operator~(const xvector& a) noexcept {
  return intrin::mm_xor_ps(a, xvconstant<-1.0f>());
}

/// performs right arithmetic shift as 32-bit integers
template<int n> requires (-32 < n && n < 32)
xvector xvsra(const xvector& a) noexcept {
  if constexpr (n == 0) return a;
  else if constexpr (n > 0) return intrin::mm_castsi128_ps(
    intrin::mm_sra_epi32(intrin::mm_castps_si128(a), n));
  else return intrin::mm_castsi128_ps(
    intrin::mm_sll_epi32(intrin::mm_castps_si128(a), -n));
}

/// performs right logical shift as 32-bit integers
template<int n> requires (-32 < n && n < 32)
xvector xvsrl(const xvector& a) noexcept {
  if constexpr (n == 0) return a;
  else if constexpr (n > 0) return intrin::mm_castsi128_ps(
    intrin::mm_srl_epi32(intrin::mm_castps_si128(a), n));
  else return intrin::mm_castsi128_ps(
    intrin::mm_sll_epi32(intrin::mm_castps_si128(a), -n));
}

/// applies absolute function to the vector
inline xvector xvabs(const xvector& a) noexcept {
  return intrin::mm_castsi128_ps(
    intrin::mm_abs_epi32(intrin::mm_castps_si128(a)));
}

/// returns the minimum value of the vector
inline xvector xvmin(const xvector& a, const xvector& b) noexcept {
  return intrin::mm_min_ps(a, b);
}

/// returns the maximum value of the vector
inline xvector xvmax(const xvector& a, const xvector& b) noexcept {
  return intrin::mm_max_ps(a, b);
}

/// returns the fused multiply-add
inline xvector xvfma(
  const xvector& a, const xvector& b, const xvector& c) noexcept {
  return intrin::mm_fmadd_ps(a, b, c);
}

/// returns the fused multiply-subtract
inline xvector xvfms(
  const xvector& a, const xvector& b, const xvector& c) noexcept {
  return intrin::mm_fmsub_ps(a, b, c);
}

/// returns the fused negative multiply-add
inline xvector xvfnma(
  const xvector& a, const xvector& b, const xvector& c) noexcept {
  return intrin::mm_fnmadd_ps(a, b, c);
}

/// returns the fused negative multiply-subtract
inline xvector xvfnms(
  const xvector& a, const xvector& b, const xvector& c) noexcept {
  return intrin::mm_fnmsub_ps(a, b, c);
}

/// peforms round operation
inline xvector xvround(const xvector& a) noexcept {
  return intrin::mm_svml_round_ps(a);
}

/// peforms truncation operation
inline xvector xvtrunc(const xvector& a) noexcept {
  return intrin::mm_trunc_ps(a);
}

/// peforms floor operation
inline xvector xvfloor(const xvector& a) noexcept {
  return intrin::mm_floor_ps(a);
}

/// peforms ceil operation
inline xvector xvceil(const xvector& a) noexcept {
  return intrin::mm_ceil_ps(a);
}

/// applies `sin` to the vector
inline xvector xvsin(const xvector& a) noexcept {
  return intrin::mm_sin_ps(a);
}

/// applies `cos` to the vector
inline xvector xvcos(const xvector& a) noexcept {
  return intrin::mm_cos_ps(a);
}

/// applies `tan` to the vector
inline xvector xvtan(const xvector& a) noexcept {
  return intrin::mm_tan_ps(a);
}

/// applies `asin` to the vector
inline xvector xvasin(const xvector& a) noexcept {
  return intrin::mm_asin_ps(a);
}

/// applies `acos` to the vector
inline xvector xvacos(const xvector& a) noexcept {
  return intrin::mm_acos_ps(a);
}

/// applies `atan` to the vector
inline xvector xvtan(const xvector& a) noexcept {
  return intrin::mm_atan_ps(a);
}

/// applies `sinh` to the vector
inline xvector xvsinh(const xvector& a) noexcept {
  return intrin::mm_sinh_ps(a);
}

/// applies `cosh` to the vector
inline xvector xvcosh(const xvector& a) noexcept {
  return intrin::mm_cosh_ps(a);
}

/// applies `tanh` to the vector
inline xvector xvtanh(const xvector& a) noexcept {
  return intrin::mm_tanh_ps(a);
}

/// applies `asinh` to the vector
inline xvector xvasinh(const xvector& a) noexcept {
  return intrin::mm_asinh_ps(a);
}

/// applies `acosh` to the vector
inline xvector xvacos(const xvector& a) noexcept {
  return intrin::mm_acosh_ps(a);
}

/// applies `atanh` to the vector
inline xvector xvtanh(const xvector& a) noexcept {
  return intrin::mm_atanh_ps(a);
}

/// applies `sincos` to the vector
/// \return `{sin(a.x), ...}`
inline xvector xvsincos(const xvector& a, xvector& Cos) noexcept {
  return intrin::mm_sincos_ps(&Cos, a);
}

/// applies `atan2` to the vector
inline xvector xvtan2(const xvector& y, const xvector& x) noexcept {
  return intrin::mm_atan2_ps(y, x);
}

/// applies `pow` to the vector
inline xvector xvpow(const xvector& a, const xvector& b) noexcept {
  return intrin::mm_pow_ps(a, b);
}

/// applies `exp` to the vector
inline xvector xvexp(const xvector& a) noexcept {
  return intrin::mm_exp_ps(a);
}

/// applies `exp2` to the vector
inline xvector xvexp2(const xvector& a) noexcept {
  return intrin::mm_exp2_ps(a);
}

/// applies `exp10` to the vector
inline xvector xvexp10(const xvector& a) noexcept {
  return intrin::mm_exp10_ps(a);
}

/// applies `expm1` to the vector
inline xvector xvexpm1(const xvector& a) noexcept {
  return intrin::mm_expm1_ps(a);
}

/// applies `ln` to the vector
inline xvector xvln(const xvector& a) noexcept {
  return intrin::mm_log_ps(a);
}

/// applies `log` to the vector
inline xvector xvlog(const xvector& a, const xvector& Base) noexcept {
  return intrin::mm_log_ps(a) / intrin::mm_log_ps(Base);
}

/// applies `log2` to the vector
inline xvector xvlog2(const xvector& a) noexcept {
  return intrin::mm_log2_ps(a);
}

/// applies `log10` to the vector
inline xvector xvlog10(const xvector& a) noexcept {
  return intrin::mm_log10_ps(a);
}

/// applies `log1p` to the vector
inline xvector xvlog1p(const xvector& a) noexcept {
  return intrin::mm_log1p_ps(a);
}

/// applies `logb` to the vector
inline xvector xvlogb(const xvector& a) noexcept {
  return intrin::mm_logb_ps(a);
}

/// applies `sqrt` to the vector
inline xvector xvsqrt(const xvector& a) noexcept {
  return intrin::mm_sqrt_ps(a);
}

/// applies `cbrt` to the vector
inline xvector xvcbrt(const xvector& a) noexcept {
  return intrin::mm_cbrt_ps(a);
}

/// returns the reciprocal of square root
inline xvector xvsqrt_r(const xvector& a) noexcept {
  return intrin::mm_invsqrt_ps(a);
}

/// returns the reciprocal of cube root
inline xvector xvcbrt_r(const xvector& a) noexcept {
  return intrin::mm_invcbrt_ps(a);
}

/// returns the hypotenuse
inline xvector xvhypot(const xvector& a, const xvector& b) noexcept {
  return intrin::mm_hypot_ps(a, b);
}

/// returns the error function
inline xvector xverf(const xvector& a) noexcept {
  return intrin::mm_erf_ps(a);
}

/// returns the complementary error function
inline xvector xverfc(const xvector& a) noexcept {
  return intrin::mm_erfc_ps(a);
}

/// returns the reciprocal of the error function
inline xvector xverf_r(const xvector& a) noexcept {
  return intrin::mm_erfinv_ps(a);
}

/// returns the reciprocal of the complementary error function
inline xvector xverfc_r(const xvector& a) noexcept {
  return intrin::mm_erfcinv_ps(a);
}

/// returns the dot product
template<nat N, nat Zero = 0>
requires (0 < N && N <= 4 && Zero < 16)
xvector xvdot(const xvector& a, const xvector& b) noexcept {
  if constexpr (Zero == 15) return xvfill();
  else return intrin::mm_dp_ps(a, b, ((1 << N) - 1) << 4 | Zero);
}

/// returns the cross product
inline xvector xvcross(const xvector& a, const xvector& b) noexcept {
  return xvpermute<1, 2, 0, 3>(a) * xvpermute<2, 0, 1, 3>(b) -
         xvpermute<2, 0, 1, 3>(a) * xvpermute<1, 2, 0, 3>(b);
}
