/// @file  ywlib/vector.h
/// @brief yw::vector and yw::quaternion

#pragma once
#include "ywlib@core.h"
#include <immintrin.h>

namespace yw {

using xvector = __m128;
using xwector = __m256d;
using xrect = __m128i;
using xmatrix = array<__m128, 4>;

// using xmatrix = __m128[4];

/// XV型のオブジェクトを作成する関数。
/// @note 1.  `(fat4 A) -> xvector(A, A, A, A);`
/// @note 2.  `(fat8 A) -> xwector(A, A, A, A);`
/// @note 3.  `(int4 A) -> xrect(A, A, A, A);`
/// @note 4.  `(fat4* P) -> xvector(P[0], P[1], P[2], P[3]);`
/// @note 5.  `(fat8* P) -> xwector(P[0], P[1], P[2], P[3]);`
/// @note 6.  `(int4* P) -> xrect(P[0], P[1], P[2], P[3]);`
/// @note 7.  `(fat4 A, B, C, D) -> xvector(A, B, C, D);`
/// @note 8.  `(fat8 A, B, C, D) -> xwector(A, B, C, D);`
/// @note 9.  `(int4 A, B, C, D) -> xrect(A, B, C, D);`
/// @note 10. `(xvector A, B, C, D) -> xmatrix(A, B, C, D);`
inline constexpr overload xv{
  [](const fat4 A) noexcept { return _mm_set1_ps(A); },
  [](const fat8 A) noexcept { return _mm256_set1_pd(A); },
  [](const int4 A) noexcept { return _mm_set1_epi32(A); },
  [](const fat4* A) noexcept { return _mm_loadu_ps(A); },
  [](const fat8* A) noexcept { return _mm256_loadu_pd(A); },
  [](const int4* A) noexcept { return _mm_loadu_epi32(A); },
  [](const fat4 X, const fat4 Y, const fat4 Z, const fat4 W) noexcept { return _mm_set_ps(W, Z, Y, X); },
  [](const fat8 X, const fat8 Y, const fat8 Z, const fat8 W) noexcept { return _mm256_set_pd(W, Z, Y, X); },
  [](const int4 X, const int4 Y, const int4 Z, const int4 W) noexcept { return _mm_set_epi32(W, Z, Y, X); },
  [](const xvector& X, const xvector& Y, const xvector& Z, const xvector& W) noexcept { return xmatrix{X, Y, Z, W}; }};

/// XV型のオブジェクトを別のXV型に変換する関数
/// @note 1. `<Xv>(xvector) -> Xv;`
/// @note 2. `<Xv>(xwector) -> Xv;`
/// @note 3. `<Xv>(xrect)   -> Xv;`
template<included_in<xvector, xwector, xrect> Xv> inline constexpr overload xvcast{
  [](const xvector& A) noexcept {
    if constexpr (same_as<Xv, xvector>) return A;
    else if constexpr (same_as<Xv, xwector>) return _mm256_cvtps_pd(A);
    else if constexpr (same_as<Xv, xrect>) return _mm_cvtps_epi32(A);
  },
  [](const xwector& A) noexcept {
    if constexpr (same_as<Xv, xvector>) return _mm256_cvtpd_ps(A);
    else if constexpr (same_as<Xv, xwector>) return A;
    else if constexpr (same_as<Xv, xrect>) return _mm256_cvtpd_epi32(A);
  },
  [](const xrect& A) noexcept {
    if constexpr (same_as<Xv, xvector>) return _mm_cvtepi32_ps(A);
    else if constexpr (same_as<Xv, xwector>) return _mm256_cvtepi32_pd(A);
    else if constexpr (same_as<Xv, xrect>) return A;
  }};

/// `xvector`を`xrect`に、`xrect`を`xvector`に`bitcast`する関数
inline constexpr overload xvbitcast{
  [](const xvector& A) noexcept -> xrect { return _mm_castps_si128(A); },
  [](const xrect& A) noexcept -> xvector { return _mm_castsi128_ps(A); }};

/// XV型のゼロ定数; `[0, 0, 0, 0]`
inline constexpr caster xv_zero{
  []() noexcept { return _mm_setzero_ps(); },
  []() noexcept { return _mm256_setzero_pd(); },
  []() noexcept { return _mm_setzero_si128(); },
  []() noexcept -> const xmatrix& { static const xmatrix static_{_mm_setzero_ps(), _mm_setzero_ps(), _mm_setzero_ps(), _mm_setzero_ps()}; return static_; }};

/// XV型の定数テンプレート; `[X, Y, Z, W]`
template<value X, value Y = X, value Z = Y, value W = Z> inline constexpr caster xv_constant{
  []() noexcept -> const xvector& { static const xvector static_ = xv(fat4(X), fat4(Y), fat4(Z), fat4(W)); return static_; },
  []() noexcept -> const xwector& { static const xwector static_ = xv(fat8(X), fat8(Y), fat8(Z), fat8(W)); return static_; },
  []() noexcept -> const xrect& { static const xrect static_ = xv(int4(X), int4(Y), int4(Z), int4(W)); return static_; }};

/// XV型の定数; `[1, 0, 0, 0]`
inline constexpr auto xv_x = xv_constant<1, 0, 0, 0>;

/// XV型の定数; `[0, 1, 0, 0]`
inline constexpr auto xv_y = xv_constant<0, 1, 0, 0>;

/// XV型の定数; `[0, 0, 1, 0]`
inline constexpr auto xv_z = xv_constant<0, 0, 1, 0>;

/// XV型の定数; `[0, 0, 0, 1]`
inline constexpr auto xv_w = xv_constant<0, 0, 0, 1>;

/// XV型の定数; `[-0, -0, -0, -0]`
inline constexpr auto xv_neg_zero = xv_constant<-0.0, -0.0, -0.0, -0.0>;

/// XV型の定数; `[-1, 0, 0, 0]`
inline constexpr auto xv_neg_x = xv_constant<-1, 0, 0, 0>;

/// XV型の定数; `[0, -1, 0, 0]`
inline constexpr auto xv_neg_y = xv_constant<0, -1, 0, 0>;

/// XV型の定数; `[0, 0, -1, 0]`
inline constexpr auto xv_neg_z = xv_constant<0, 0, -1, 0>;

/// XV型の定数; `[0, 0, 0, -1]`
inline constexpr auto xv_neg_w = xv_constant<0, 0, 0, -1>;

/// XV型の定数; `[xv_x, xv_y, xv_z, xv_w]`
inline constexpr caster xv_identity{
  []() noexcept -> const xmatrix& {
    static const xmatrix m{xv_x, xv_y, xv_z, xv_w};
    return m;
  }};

/// XV型の一要素から値を抽出する
template<natt I> requires(I < 4) inline constexpr overload xvget{
  [](const xvector& A) noexcept {
    if constexpr (I == 0) return _mm_cvtss_f32(A);
    else return bitcast<fat4>(_mm_extract_ps(A, int(I)));
  },
  [](const xwector& A) noexcept {
    return bitcast<fat8>(_mm256_extract_epi64(_mm256_castpd_si256(A), int(I)));
  },
  [](const xrect& A) noexcept {
    if constexpr (I == 0) return _mm_cvtsi128_si32(A);
    else return _mm_extract_epi32(A, int(I));
  }};

/// XV型の一要素に値を設定する
template<natt I> requires(I < 4) inline constexpr overload xvset{
  [](const xvector& A, fat4 B) noexcept { return xvbitcast(_mm_insert_epi32(xvbitcast(A), bitcast<int4>(B), int(I))); },
  [](const xwector& A, fat8 B) noexcept { return _mm256_castsi256_pd(_mm256_insert_epi64(_mm256_castpd_si256(A), bitcast<int8>(B), int(I))); },
  [](const xrect& A, int4 B) noexcept { return _mm_insert_epi32(A, B, int(I)); }};

/// XV型の各要素を混ぜる
template<bool X, bool Y, bool Z, bool W> inline constexpr overload xvblend{
  [](const xvector& A, const xvector& B) noexcept {
    if constexpr (!(X | Y | Z | W)) return A;
    else if constexpr (X & Y & Z & W) return B;
    else return _mm_blend_ps(A, B, int(X + Y * 2 + Z * 4 + W * 8));
  },
  [](const xwector& A, const xwector& B) noexcept {
    if constexpr (!(X | Y | Z | W)) return A;
    else if constexpr (X & Y & Z & W) return B;
    else return _mm256_blend_pd(A, B, int(X + Y * 2 + Z * 4 + W * 8));
  },
  [](const xrect& A, const xrect& B) noexcept {
    if constexpr (!(X | Y | Z | W)) return A;
    else if constexpr (X & Y & Z & W) return B;
    else return _mm_blend_epi32(A, B, int(X + Y * 2 + Z * 4 + W * 8));
  }};

/// XV型の指定した要素にゼロを設定する
template<bool X, bool Y, bool Z, bool W> inline constexpr auto xvsetzero =
  [](const included_in<xvector, xwector, xrect> auto& A) noexcept { return xvblend<X, Y, Z, W>(A, xv_zero); };

template<intt X, intt Y, intt Z, intt W> struct t_xvpermute {
private:
  template<intt A, intt B, intt C, intt D> friend struct t_xvpermute;
  static xvector call(const xvector& A) noexcept {
    constexpr auto f = [](auto a, auto b) { return a == b || a < 0 || a >= 4; };
    constexpr natt x = (f(X, 0) ? 0 : X), y = (f(Y, 1) ? 1 : Y), z = (f(Z, 2) ? 2 : Z), w = (f(W, 3) ? 3 : W);
    if constexpr (f(X, 0) && f(Y, 1) && f(Z, 2) && f(W, 3)) return A;
    else if constexpr (f(X, 0) && f(Y, 0) && f(Z, 2) && f(W, 2)) return _mm_moveldup_ps(A);
    else if constexpr (f(X, 1) && f(Y, 1) && f(Z, 3) && f(W, 3)) return _mm_movehdup_ps(A);
    else return _mm_permute_ps(A, int(x + y * 4 + z * 16 + w * 64));
  }
  static xwector call(const xwector& A) noexcept {
    constexpr auto f = [](auto a, auto b) { return a == b || a < 0 || a >= 4; };
    constexpr natt x = (f(X, 0) ? 0 : X), y = (f(Y, 1) ? 1 : Y), z = (f(Z, 2) ? 2 : Z), w = (f(W, 3) ? 3 : W);
    if constexpr (x == 0 && y == 1 && z == 2 && w == 3) return A;
    else if constexpr ((x | y) < 2 && 2 <= (z & w)) return _mm256_permute_pd(A, int(x + y * 2 + (z - 2) * 4 + (w - 2) * 8));
    else return _mm256_permute4x64_pd(A, int(x + y * 4 + z * 16 + w * 64));
  }
public:
  xvector operator()(const xvector& A) const noexcept requires(lt(max(X, Y, Z, W), 4)) { return call(A); }
  xwector operator()(const xwector& A) const noexcept requires(lt(max(X, Y, Z, W), 4)) { return call(A); }
  xrect operator()(const xrect& A) const noexcept requires(lt(max(X, Y, Z, W), 4)) { return xvbitcast(call(xvbitcast(A))); }
  xvector operator()(const xvector& A, const xvector& B) const noexcept requires(lt(max(X, Y, Z, W), 8)) {
    constexpr auto f = [](auto a, auto b) { return a == b || a < 0; };
    constexpr natt xa = (f(X, 0) ? 0 : X), ya = (f(Y, 1) ? 1 : Y), za = (f(Z, 2) ? 2 : Z), wa = (f(W, 3) ? 3 : W),
                   xb = (f(X, 4) ? 4 : X), yb = (f(Y, 5) ? 5 : Y), zb = (f(Z, 6) ? 6 : Z), wb = (f(W, 7) ? 7 : W);
    if constexpr ((xa | ya | za | wa) < 4) return t_xvpermute<xa, ya, za, wa>::call(A);
    else if constexpr (4 <= (xb & yb & zb & wb)) return t_xvpermute<xb - 4, yb - 4, zb - 4, wb - 4>::call(B);
    else if constexpr ((xa & 3) == 0 && (ya & 3) == 1 && (za & 3) == 2 && (wa & 3) == 3) return xvblend<lt(xa, 4), lt(ya, 4), lt(za, 4), lt(wa, 4)>(B, A);
    else if constexpr ((xa | ya) < 4 && 4 <= (zb & wb)) return _mm_shuffle_ps(A, B, xa + ya * 4 + (zb - 4) * 16 + (wb - 4) * 64);
    else if constexpr (4 <= (xb & yb) && (za | wa) < 4) return _mm_shuffle_ps(B, A, xb - 4 | (yb - 4) * 4 | za * 16 | wa * 64);
    else if constexpr ((xa == 0) + (ya == 1) + (za == 2) + (wa == 3) == 3) {
      constexpr natt i = inspects<xa != 0, ya != 1, za != 2, wa != 3>;
      return _mm_insert_ps(A, B, int((value_switch<i, xa, ya, za, wa> - 4) << 6 | i << 4));
    } else if constexpr ((xb == 4) + (yb == 5) + (zb == 6) + (wb == 7) == 3) {
      constexpr natt i = inspects<xb != 4, yb != 5, zb != 6, wb != 7>;
      return _mm_insert_ps(A, B, int(value_switch<i, xb, yb, zb, wb> << 6 | i << 4));
    } else if constexpr ((xa < 4 || xb == 4) && (ya < 4 || yb == 5) && (za < 4 || zb == 6) && (wa < 4 || wb == 7))
      return xvblend<xb == 4, yb == 5, zb == 6, wb == 7>(t_xvpermute<xa, ya, za, wa>::call(A), B);
    else if constexpr ((4 <= xb || xa == 0) && (4 <= yb || ya == 1) && (4 <= zb || za == 2) && (4 <= wb || wa == 3))
      return xvblend<xa == 0, ya == 1, za == 2, wa == 3>(t_xvpermute<xb - 4, yb - 4, zb - 4, wb - 4>::call(B), A);
    else if constexpr ((xa < 4) + (ya < 4) + (za < 4) + (wa < 4) == 3) {
      constexpr natt i = inspects<(xa > 3), (ya > 3), (za > 3), (wa > 3)>;
      return _mm_insert_ps(t_xvpermute<xa, ya, za, wa>::call(A), B, int((value_switch<i, xa, ya, za, wa> - 4) << 6 | i << 4));
    } else if constexpr ((xb < 4) + (yb < 4) + (zb < 4) + (wb < 4) == 1) {
      constexpr natt i = inspects<(xb < 4), (yb < 4), (zb < 4), (wb < 4)>;
      return _mm_insert_ps(t_xvpermute<xb - 4, yb - 4, zb - 4, wb - 4>::call(B), A, int(value_switch<i, xb, yb, zb, wb> << 6 | i << 4));
    } else return xvblend<lt(xa, 4), lt(ya, 4), lt(za, 4), lt(wa, 4)>(
      t_xvpermute<xb - 4, yb - 4, zb - 4, wb - 4>::call(B), t_xvpermute<xa, ya, za, wa>::call(A));
  };
  xwector operator()(const xwector& A, const xwector& B) const noexcept requires(lt(max(X, Y, Z, W), 8)) {
    constexpr auto f = [](auto a, auto b) noexcept { return a == b || a < 0; };
    constexpr natt xa = (f(X, 0) ? 0 : X), ya = (f(Y, 1) ? 1 : Y), za = (f(Z, 2) ? 2 : Z), wa = (f(W, 3) ? 3 : W),
                   xb = (f(X, 4) ? 4 : X), yb = (f(Y, 5) ? 5 : Y), zb = (f(Z, 6) ? 6 : Z), wb = (f(W, 7) ? 7 : W);
    if constexpr (xa == 0 && ya == 1 && za == 2 && wa == 3) return A;
    else if constexpr (xb == 4 && yb == 5 && zb == 6 && wb == 7) return B;
    else if constexpr ((xa & 3) == 0 && (ya & 3) == 1 && (za & 3) == 2 && (wa & 3) == 3) return xvblend<lt(xa, 4), lt(ya, 4), lt(za, 4), lt(wa, 4)>(B, A);
    else if constexpr (xa < 2 && (yb == 4 || yb == 5) && (za == 2 || zb == 3) && wb > 5)
      return _mm256_shuffle_pd(A, B, (xa == 1) + (yb == 5) * 2 + (za == 3) * 4 + (wb == 7) * 8);
    else if constexpr ((xb == 4 || xb == 5) && ya < 2 && zb > 5 && (wa == 2 || wa == 3))
      return _mm256_shuffle_pd(B, A, (xb == 5) + (ya == 1) * 2 + (zb == 7) * 4 + (wa == 3) * 8);
    else if constexpr ((xa | ya | za | wa) < 4) return t_xvpermute<xa, ya, za, wa>::call(A);
    else if constexpr ((xb & yb & zb & wb) > 3) return t_xvpermute<xb - 4, yb - 4, zb - 4, wb - 4>::call(B);
    else if constexpr (((xa == 0 && ya == 1) || (xa == 2 && ya == 3)) && ((zb == 4 && wb == 5) || (zb == 6 && wb == 7)))
      return _mm256_shuffle_f64x2(A, B, (xa == 2) + (zb == 6) * 2);
    else if constexpr (((xb == 4 && yb == 5) || (xb == 6 && yb == 7)) && ((za == 0 && wa == 1) || (za == 2 && wa == 3)))
      return _mm256_shuffle_f64x2(B, A, (xb == 6) + (za == 2) * 2);
    else return xvblend<lt(xa, 4), lt(ya, 4), lt(za, 4), lt(wa, 4)>(t_xvpermute<xb - 4, yb - 4, zb - 4, wb - 4>::call(B), t_xvpermute<xa, ya, za, wa>::call(A));
  }
  xrect operator()(const xrect& A, const xrect& B) const noexcept requires(lt(max(X, Y, Z, W), 8)) {
    return xvbitcast((*this)(xvbitcast(A), xvbitcast(B)));
  }
};

/// XVオブジェクトの要素を組替える
template<intt X, intt Y, intt Z, intt W> inline constexpr t_xvpermute<X, Y, Z, W> xvpermute;

/// あるXVの指定した要素に別のXVの指定した要素を代入する
template<natt To, natt From, natt Zero = 0b0000> requires((To | From) < 4 && Zero < 16) inline constexpr overload xvinsert{
  [](const xvector& Inserted, const xvector& Picked) noexcept {
  constexpr auto f = [](auto a) { return To == a ? From + 4 : a; };
  if constexpr (Zero == 0) return xvpermute<f(0), f(1), f(2), f(3)>(Inserted, Picked);
  else return _mm_insert_ps(Inserted, Picked, From << 6 | To << 4 | Zero); },
  [](const xwector& Inserted, const xwector& Picked) noexcept {
  constexpr auto f = [](auto a) { return To == a ? From + 4 : a; };
  if constexpr (Zero == 0) return xvpermute<f(0), f(1), f(2), f(3)>(Inserted, Picked);
  else return xvblend<!(Zero & 1), !(Zero & 2), !(Zero & 4), !(Zero & 8)>(xv_zero, xvpermute<f(0), f(1), f(2), f(3)>(Inserted, Picked)); },
  [](const xrect& Inserted, const xrect& Picked) noexcept {
    constexpr auto f = [](auto a) { return To == a ? From + 4 : a; };
    if constexpr (Zero == 0) return xvpermute<f(0), f(1), f(2), f(3)>(Inserted, Picked);
    else return xvbitcast(_mm_insert_ps(xvbitcast(Inserted), xvbitcast(Picked), From << 6 | To << 4 | Zero));
  }};

/// XVオブジェクトが完全に同一であるか確認する
inline constexpr overload xveq{
  [](const xvector& A, const xvector& B) noexcept -> bool { return _mm_test_all_ones(xvbitcast(_mm_cmpeq_ps(A, B))); },
  [](const xwector& A, const xwector& B) noexcept -> bool { return _mm256_testc_pd(_mm256_cmp_pd(A, B, 0), xv_neg_zero); },
  [](const xrect& A, const xrect& B) noexcept -> bool { return _mm_test_all_ones(_mm_cmpeq_epi32(A, B)); },
  []<natt N>(const xvector (&A)[N], const xvector (&B)[N]) noexcept -> bool { return xveq(A[0], B[0]) && xveq(A[1], B[1]); }};

/// XVオブジェクトが完全で同一でないことを確認する
inline constexpr overload xvne{
  [](const xvector& A, const xvector& B) noexcept -> bool { return !xveq(A, B); },
  [](const xwector& A, const xwector& B) noexcept -> bool { return !xveq(A, B); },
  [](const xrect& A, const xrect& B) noexcept -> bool { return !xveq(A, B); },
  []<natt N>(const xvector (&A)[N], const xvector (&B)[N]) noexcept -> bool { return !xveq(A, B); }};

/// XVオブジェクトを辞書順で比較して、AがBより小さいことを確認する
inline constexpr overload xvlt{
  [](const xvector& A, const xvector& B) noexcept -> bool { return _mm_movemask_ps(_mm_cmpgt_ps(A, B)) < _mm_movemask_ps(_mm_cmplt_ps(A, B)); },
  [](const xwector& A, const xwector& B) noexcept -> bool { return _mm256_movemask_pd(_mm256_cmp_pd(B, A, 17)) < _mm256_movemask_pd(_mm256_cmp_pd(A, B, 17)); },
  [](const xrect& A, const xrect& B) noexcept -> bool {
    return _mm_movemask_ps(xvbitcast(_mm_cmpgt_epi32(A, B))) < _mm_movemask_ps(xvbitcast(_mm_cmplt_epi32(A, B)));
  }};

/// XVオブジェクトを辞書順で比較して、AがB以下であることを確認する
inline constexpr overload xvle{
  [](const xvector& A, const xvector& B) noexcept -> bool {
    return _mm_movemask_ps(_mm_cmpgt_ps(A, B)) <= _mm_movemask_ps(_mm_cmplt_ps(A, B));
  },
  [](const xwector& A, const xwector& B) noexcept -> bool {
    return _mm256_movemask_pd(_mm256_cmp_pd(B, A, 17)) <= _mm256_movemask_pd(_mm256_cmp_pd(A, B, 17));
  },
  [](const xrect& A, const xrect& B) noexcept -> bool {
    return _mm_movemask_ps(xvbitcast(_mm_cmpgt_epi32(A, B))) <= _mm_movemask_ps(xvbitcast(_mm_cmplt_epi32(A, B)));
  }};

/// XVオブジェクトを辞書順で比較して、AがBより大きいことを確認する
inline constexpr overload xvgt{
  [](const xvector& A, const xvector& B) noexcept -> bool {
    return _mm_movemask_ps(_mm_cmpgt_ps(A, B)) > _mm_movemask_ps(_mm_cmplt_ps(A, B));
  },
  [](const xwector& A, const xwector& B) noexcept -> bool {
    return _mm256_movemask_pd(_mm256_cmp_pd(B, A, 17)) > _mm256_movemask_pd(_mm256_cmp_pd(A, B, 17));
  },
  [](const xrect& A, const xrect& B) noexcept -> bool {
    return _mm_movemask_ps(xvbitcast(_mm_cmpgt_epi32(A, B))) > _mm_movemask_ps(xvbitcast(_mm_cmplt_epi32(A, B)));
  }};

/// XVオブジェクトを辞書順で比較して、AがB以上であることを確認する
inline constexpr overload xvge{
  [](const xvector& A, const xvector& B) noexcept -> bool {
    return _mm_movemask_ps(_mm_cmpgt_ps(A, B)) >= _mm_movemask_ps(_mm_cmplt_ps(A, B));
  },
  [](const xwector& A, const xwector& B) noexcept -> bool {
    return _mm256_movemask_pd(_mm256_cmp_pd(B, A, 17)) >= _mm256_movemask_pd(_mm256_cmp_pd(A, B, 17));
  },
  [](const xrect& A, const xrect& B) noexcept -> bool {
    return _mm_movemask_ps(xvbitcast(_mm_cmpgt_epi32(A, B))) >= _mm_movemask_ps(xvbitcast(_mm_cmplt_epi32(A, B)));
  }};

/// XVオブジェクトを辞書順で三方比較する
inline constexpr overload xvtw{
  [](const xvector& A, const xvector& B) noexcept {
    return _mm_movemask_ps(_mm_cmpgt_ps(A, B)) <=> _mm_movemask_ps(_mm_cmplt_ps(A, B));
  },
  [](const xwector& A, const xwector& B) noexcept {
    return _mm256_movemask_pd(_mm256_cmp_pd(B, A, 17)) <=> _mm256_movemask_pd(_mm256_cmp_pd(A, B, 17));
  },
  [](const xrect& A, const xrect& B) noexcept {
    return _mm_movemask_ps(xvbitcast(_mm_cmpgt_epi32(A, B))) <=> _mm_movemask_ps(xvbitcast(_mm_cmplt_epi32(A, B)));
  }};

/// XVオブジェクトの各要素の絶対値を取る
/// @note 1. `xvabs(const Xv& A)`
/// @note 2. `xvabs(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result`の参照先は`A`と同一であってもよい
inline constexpr overload xvabs{
  [](const xvector& A) noexcept { return _mm_andnot_ps(xv_neg_zero, A); },
  [](const xwector& A) noexcept { return _mm256_andnot_pd(xv_neg_zero, A); },
  [](const xrect& A) noexcept { return _mm_abs_epi32(A); },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_andnot_ps(xv_neg_zero, A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_andnot_pd(xv_neg_zero, A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xrect (&A)[N], xrect (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_abs_epi32(A[Is])), ...); }(make_sequence<N>{});
  }};

/// XVオブジェクトの符号を反転する
/// @note 1. `xvneg(const Xv& A)`
/// @note 2. `xvneg(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result`の参照先は`A`と同一であってもよい
inline constexpr overload xvneg{
  [](const xvector& A) noexcept { return _mm_xor_ps(A, xv_neg_zero); },
  [](const xwector& A) noexcept { return _mm256_xor_pd(A, xv_neg_zero); },
  [](const xrect& A) noexcept { return _mm_xor_epi32(A, xv_neg_zero); },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_xor_ps(A[Is], xv_neg_zero)), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_xor_pd(A[Is], xv_neg_zero)), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xrect (&A)[N], xrect (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_xor_epi32(A[Is], xv_neg_zero)), ...); }(make_sequence<N>{});
  }};

/// XVオブジェクトを加算する
/// @note 1. `xvadd(const Xv& A, const Xv& B)`
/// @note 2. `xvadd(const Xv (&A)[N], const Xv (&B)[N], Xv (&Result)[N])`
/// @note `Result`の参照先は`A`あるいは`B`と同一であってもよい
inline constexpr overload xvadd{
  [](const xvector& A, const xvector& B) noexcept { return _mm_add_ps(A, B); },
  [](const xwector& A, const xwector& B) noexcept { return _mm256_add_pd(A, B); },
  [](const xrect& A, const xrect& B) noexcept { return _mm_add_epi32(A, B); },
  []<natt N>(const xvector (&A)[N], const xvector (&B)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_add_ps(A[Is], B[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], const xwector (&B)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_add_pd(A[Is], B[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xrect (&A)[N], const xrect (&B)[N], xrect (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_add_epi32(A[Is], B[Is])), ...); }(make_sequence<N>{});
  }};

/// XVオブジェクトを減算する
/// @note 1. `xvsub(const Xv& A, const Xv& B)`
/// @note 2. `xvsub(const Xv (&A)[N], const Xv (&B)[N], Xv (&Result)[N])`
/// @note `Result`の参照先は`A`あるいは`B`と同一であってもよい
inline constexpr overload xvsub{
  [](const xvector& A, const xvector& B) noexcept { return _mm_sub_ps(A, B); },
  [](const xwector& A, const xwector& B) noexcept { return _mm256_sub_pd(A, B); },
  [](const xrect& A, const xrect& B) noexcept { return _mm_sub_epi32(A, B); },
  []<natt N>(const xvector (&A)[N], const xvector (&B)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_sub_ps(A[Is], B[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], const xwector (&B)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_sub_pd(A[Is], B[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xrect (&A)[N], const xrect (&B)[N], xrect (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_sub_epi32(A[Is], B[Is])), ...); }(make_sequence<N>{});
  }};

/// XVオブジェクトを乗算する
/// @note 1. `xvmul(const Xv& A, const Xv& B)`
/// @note 2. `xvmul(const Xv (&A)[N], const Xv (&B)[N], Xv (&Result)[N])`
/// @note `Result`の参照先は`A`あるいは`B`と同一であってもよい
inline constexpr overload xvmul{
  [](const xvector& A, const xvector& B) noexcept { return _mm_mul_ps(A, B); },
  [](const xwector& A, const xwector& B) noexcept { return _mm256_mul_pd(A, B); },
  [](const xrect& A, const xrect& B) noexcept { return _mm_mul_epi32(A, B); },
  []<natt N>(const xvector (&A)[N], const xvector (&B)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_mul_ps(A[Is], B[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], const xwector (&B)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_mul_pd(A[Is], B[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xrect (&A)[N], const xrect (&B)[N], xrect (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_mul_epi32(A[Is], B[Is])), ...); }(make_sequence<N>{});
  }};

/// XVオブジェクトを除算する
/// @note 1. `xvdiv(const Xv& A, const Xv& B)`
/// @note 2. `xvdiv(const Xv (&A)[N], const Xv (&B)[N], Xv (&Result)[N])`
/// @note `Result`の参照先は`A`あるいは`B`と同一であってもよい
inline constexpr overload xvdiv{
  [](const xvector& A, const xvector& B) noexcept { return _mm_div_ps(A, B); },
  [](const xwector& A, const xwector& B) noexcept { return _mm256_div_pd(A, B); },
  [](const xrect& A, const xrect& B) noexcept { return _mm_div_epi32(A, B); },
  []<natt N>(const xvector (&A)[N], const xvector (&B)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_div_ps(A[Is], B[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], const xwector (&B)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_div_pd(A[Is], B[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xrect (&A)[N], const xrect (&B)[N], xrect (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_div_epi32(A[Is], B[Is])), ...); }(make_sequence<N>{});
  }};

/// XVオブジェクトの指定した要素を絶対値にする
/// @note 1. `xvabsolute<X, Y, Z, W>(const Xv& A)`
/// @note 2. `xvabsolute<X, Y, Z, W>(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result`の参照先は`A`と同一であってもよい
template<bool X, bool Y, bool Z, bool W> inline constexpr overload xvabsolute{
  [](const xvector& A) noexcept { return xvblend<X, Y, Z, W>(A, xvabs(A)); },
  [](const xwector& A) noexcept { return xvblend<X, Y, Z, W>(A, xvabs(A)); },
  [](const xrect& A) noexcept { return xvblend<X, Y, Z, W>(A, xvabs(A)); },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = xvblend<X, Y, Z, W>(A[Is], xvabs(A[Is]))), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = xvblend<X, Y, Z, W>(A[Is], xvabs(A[Is]))), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xrect (&A)[N], xrect (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = xvblend<X, Y, Z, W>(A[Is], xvabs(A[Is]))), ...); }(make_sequence<N>{});
  }};

/// XVオブジェクトの指定した要素の符号を反転する
/// @note 1. `xvnegate<X, Y, Z, W>(const Xv& A)`
/// @note 2. `xvnegate<X, Y, Z, W>(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result`の参照先は`A`と同一であってもよい
template<bool X, bool Y, bool Z, bool W> inline constexpr overload xvnegate{
  [](const xvector& A) noexcept { return xvblend<X, Y, Z, W>(A, xvneg(A)); },
  [](const xwector& A) noexcept { return xvblend<X, Y, Z, W>(A, xvneg(A)); },
  [](const xrect& A) noexcept { return xvblend<X, Y, Z, W>(A, xvneg(A)); },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = xvblend<X, Y, Z, W>(A[Is], xvneg(A[Is]))), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = xvblend<X, Y, Z, W>(A[Is], xvneg(A[Is]))), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xrect (&A)[N], xrect (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = xvblend<X, Y, Z, W>(A[Is], xvneg(A[Is]))), ...); }(make_sequence<N>{});
  }};

/// XVオブジェクトでFMA(`A[i] * B[i] + C[i]`)を行う
/// @note 1. `xvfma(const Xv& A, const Xv& B, const Xv& C)`
/// @note 2. `xvfma(const Xv (&A)[N], const Xv (&B)[N], const Xv (&C)[N], Xv (&Result)[N])`
/// @note `Result`の参照先は`A`、`B`あるいは`C`と同一であってもよい
inline constexpr overload xvfma{
  [](const xvector& A, const xvector& B, const xvector& C) noexcept { return _mm_fmadd_ps(A, B, C); },
  [](const xwector& A, const xwector& B, const xwector& C) noexcept { return _mm256_fmadd_pd(A, B, C); },
  [](const xrect& A, const xrect& B, const xrect& C) noexcept { return xvadd(xvmul(A, B), C); },
  []<natt N>(const xvector (&A)[N], const xvector (&B)[N], const xvector (&C)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_fmadd_ps(A[Is], B[Is], C[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], const xwector (&B)[N], const xwector (&C)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_fmadd_pd(A[Is], B[Is], C[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xrect (&A)[N], const xrect (&B)[N], const xrect (&C)[N], xrect (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = xvadd(xvmul(A[Is], B[Is]), C[Is])), ...); }(make_sequence<N>{});
  }};

/// XVオブジェクトでFMS(`A[i] * B[i] - C[i]`)を行う
/// @note 1. `xvfms(const Xv& A, const Xv& B, const Xv& C)`
/// @note 2. `xvfms(const Xv (&A)[N], const Xv (&B)[N], const Xv (&C)[N], Xv (&Result)[N])`
/// @note `Result`の参照先は`A`、`B`あるいは`C`と同一であってもよい
inline constexpr overload xvfms{
  [](const xvector& A, const xvector& B, const xvector& C) noexcept { return _mm_fmsub_ps(A, B, C); },
  [](const xwector& A, const xwector& B, const xwector& C) noexcept { return _mm256_fmsub_pd(A, B, C); },
  [](const xrect& A, const xrect& B, const xrect& C) noexcept { return xvsub(xvmul(A, B), C); },
  []<natt N>(const xvector (&A)[N], const xvector (&B)[N], const xvector (&C)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_fmsub_ps(A[Is], B[Is], C[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], const xwector (&B)[N], const xwector (&C)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_fmsub_pd(A[Is], B[Is], C[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xrect (&A)[N], const xrect (&B)[N], const xrect (&C)[N], xrect (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = xvsub(xvmul(A[Is], B[Is]), C[Is])), ...); }(make_sequence<N>{});
  }};

/// XVオブジェクトでFNMA(`-A[i] * B[i] + C[i]`)を行う
/// @note 1. `xvfnma(const Xv& A, const Xv& B, const Xv& C)`
/// @note 2. `xvfnma(const Xv (&A)[N], const Xv (&B)[N], const Xv (&C)[N], Xv (&Result)[N])`
/// @note `Result`の参照先は`A`、`B`あるいは`C`と同一であってもよい
inline constexpr overload xvfnma{
  [](const xvector& A, const xvector& B, const xvector& C) noexcept { return _mm_fnmadd_ps(A, B, C); },
  [](const xwector& A, const xwector& B, const xwector& C) noexcept { return _mm256_fnmadd_pd(A, B, C); },
  [](const xrect& A, const xrect& B, const xrect& C) noexcept { return xvsub(xvmul(xvneg(A), B), C); },
  []<natt N>(const xvector (&A)[N], const xvector (&B)[N], const xvector (&C)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_fnmadd_ps(A[Is], B[Is], C[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], const xwector (&B)[N], const xwector (&C)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_fnmadd_pd(A[Is], B[Is], C[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xrect (&A)[N], const xrect (&B)[N], const xrect (&C)[N], xrect (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = xvsub(xvmul(xvneg(A[Is]), B[Is]), C[Is])), ...); }(make_sequence<N>{});
  }};

/// XVオブジェクトでFNMS(`-A[i] * B[i] - C[i]`)を行う
/// @note 1. `xvfnms(const Xv& A, const Xv& B, const Xv& C)`
/// @note 2. `xvfnms(const Xv (&A)[N], const Xv (&B)[N], const Xv (&C)[N], Xv (&Result)[N])`
/// @note `Result`の参照先は`A`、`B`あるいは`C`と同一であってもよい
inline constexpr overload xvfnms{
  [](const xvector& A, const xvector& B, const xvector& C) noexcept { return _mm_fnmsub_ps(A, B, C); },
  [](const xwector& A, const xwector& B, const xwector& C) noexcept { return _mm256_fnmsub_pd(A, B, C); },
  [](const xrect& A, const xrect& B, const xrect& C) noexcept { return xvsub(xvmul(xvneg(A), B), C); },
  []<natt N>(const xvector (&A)[N], const xvector (&B)[N], const xvector (&C)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_fnmsub_ps(A[Is], B[Is], C[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], const xwector (&B)[N], const xwector (&C)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_fnmsub_pd(A[Is], B[Is], C[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xrect (&A)[N], const xrect (&B)[N], const xrect (&C)[N], xrect (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = xvsub(xvmul(xvneg(A[Is]), B[Is]), C[Is])), ...); }(make_sequence<N>{});
  }};

/// XVオブジェクトの各要素に天井関数を適用する
/// @note 1. `xvceil(const Xv& A)`
/// @note 2. `xvceil(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result`の参照先は`A`と同一であってもよい
inline constexpr overload xvceil{
  [](const xvector& A) noexcept { return _mm_ceil_ps(A); },
  [](const xwector& A) noexcept { return _mm256_ceil_pd(A); },
  [](const xrect& A) noexcept { return A; },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_ceil_ps(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_ceil_pd(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xrect (&A)[N], xrect (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = A[Is]), ...); }(make_sequence<N>{});
  }};

/// XVオブジェクトの各要素に床関数を適用する
/// @note 1. `xvfloor(const Xv& A)`
/// @note 2. `xvfloor(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result`の参照先は`A`と同一であってもよい
inline constexpr overload xvfloor{
  [](const xvector& A) noexcept { return _mm_floor_ps(A); },
  [](const xwector& A) noexcept { return _mm256_floor_pd(A); },
  [](const xrect& A) noexcept { return A; },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_floor_ps(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_floor_pd(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xrect (&A)[N], xrect (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = A[Is]), ...); }(make_sequence<N>{});
  }};

/// XVオブジェクトの各要素を切り捨てる
/// @note 1. `xvtrunc(const Xv& A)`
/// @note 2. `xvtrunc(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result`の参照先は`A`と同一であってもよい
inline constexpr overload xvtrunc{
  [](const xvector& A) noexcept { return _mm_trunc_ps(A); },
  [](const xwector& A) noexcept { return _mm256_trunc_pd(A); },
  [](const xrect& A) noexcept { return A; },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_trunc_ps(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_trunc_pd(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xrect (&A)[N], xrect (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = A[Is]), ...); }(make_sequence<N>{});
  }};

/// XVオブジェクトの各要素を四捨五入する
/// @note 1. `xvround(const Xv& A)`
/// @note 2. `xvround(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result`の参照先は`A`と同一であってもよい
/// @note 3. `xvround(const Xv& A, constant<int I>)`
/// @note 小数点から右側に`I`の桁数で四捨五入する
inline constexpr overload xvround{
  [](const xvector& A) noexcept { return _mm_round_ps(A, 8); },
  [](const xwector& A) noexcept { return _mm256_round_pd(A, 8); },
  [](const xrect& A) noexcept { return A; },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_round_ps(A[Is], 8)), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_round_pd(A[Is]), 8), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xrect (&A)[N], xrect (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = A[Is]), ...); }(make_sequence<N>{});
  }};

/// XVオブジェクトの各要素を比較して最大値を取る
/// @note 1. `xvmax(const Xv& A, const Xv& B)`
/// @note 2. `xvmax(const Xv (&A)[N], const Xv (&B)[N], Xv (&Result)[N])`
/// @note `Result`の参照先は`A`あるいは`B`と同一であってもよい
inline constexpr overload xvmax{
  [](const xvector& A, const xvector& B) noexcept { return _mm_max_ps(A, B); },
  [](const xwector& A, const xwector& B) noexcept { return _mm256_max_pd(A, B); },
  [](const xrect& A, const xrect& B) noexcept { return _mm_max_epi32(A, B); },
  []<natt N>(const xvector (&A)[N], const xvector (&B)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_max_ps(A[Is], B[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], const xwector (&B)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_max_pd(A[Is], B[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xrect (&A)[N], const xrect (&B)[N], xrect (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_max_epi32(A[Is], B[Is])), ...); }(make_sequence<N>{});
  }};

/// XVオブジェクトの各要素を比較して最小値を取る
/// @note 1. `xvmin(const Xv& A, const Xv& B)`
/// @note 2. `xvmin(const Xv (&A)[N], const Xv (&B)[N], Xv (&Result)[N])`
/// @note `Result`の参照先は`A`あるいは`B`と同一であってもよい
inline constexpr overload xvmin{
  [](const xvector& A, const xvector& B) noexcept { return _mm_min_ps(A, B); },
  [](const xwector& A, const xwector& B) noexcept { return _mm256_min_pd(A, B); },
  [](const xrect& A, const xrect& B) noexcept { return _mm_min_epi32(A, B); },
  []<natt N>(const xvector (&A)[N], const xvector (&B)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_min_ps(A[Is], B[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], const xwector (&B)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_min_pd(A[Is], B[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xrect (&A)[N], const xrect (&B)[N], xrect (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_min_epi32(A[Is], B[Is])), ...); }(make_sequence<N>{});
  }};

/// XVオブジェクトの各要素に`sin`関数を適用する
/// @note 1. `xvsin(const Xv& A)`
/// @note 2. `xvsin(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result`の参照先は`A`と同一であってもよい
/// @note 3. `xvsin(const Xv& A, Xv& Cos)`
/// @note `Cos`には`cos`関数の結果が格納される
/// @note 4. `xvsin(const Xv (&A)[N], Xv (&Result)[N], Xv (&Cos)[N])`
/// @note `Result`あるいは`Cos`の参照先は`A`と同一であってもよい
inline constexpr overload xvsin{
  [](const xvector& A) noexcept { return _mm_sin_ps(A); },
  [](const xwector& A) noexcept { return _mm256_sin_pd(A); },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_sin_ps(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_sin_pd(A[Is])), ...); }(make_sequence<N>{});
  },
  [](const xvector& A, xvector& Cos) noexcept {
    return _mm_sincos_ps(&Cos, A);
  },
  [](const xwector& A, xwector& Cos) noexcept {
    return _mm256_sincos_pd(&Cos, A);
  },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N], xvector (&Cos)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_sincos_ps(&Cos[Is], A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N], xwector (&Cos)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_sin_pd(&Cos[Is], A[Is])), ...); }(make_sequence<N>{});
  }};

/// XVオブジェクトの各要素に`asin`関数を適用する
/// @note 1. `xvasin(const Xv& A)`
/// @note 2. `xvasin(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result`の参照先は`A`と同一であってもよい
inline constexpr overload xvasin{
  [](const xvector& A) noexcept { return _mm_asin_ps(A); },
  [](const xwector& A) noexcept { return _mm256_asin_pd(A); },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_asin_ps(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_asin_pd(A[Is])), ...); }(make_sequence<N>{});
  }};

/// XVオブジェクトの各要素に`sinh`関数を適用する
/// @note 1. `xvsinh(const Xv& A)`
/// @note 2. `xvsinh(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result`の参照先は`A`と同一であってもよい
inline constexpr overload xvsinh{
  [](const xvector& A) noexcept { return _mm_sinh_ps(A); },
  [](const xwector& A) noexcept { return _mm256_sinh_pd(A); },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_sinh_ps(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_sinh_pd(A[Is])), ...); }(make_sequence<N>{});
  }};

/// XVオブジェクトの各要素に`asinh`関数を適用する
/// @note 1. `xvasinh(const Xv& A)`
/// @note 2. `xvasinh(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result`の参照先は`A`と同一であってもよい
inline constexpr overload xvasinh{
  [](const xvector& A) noexcept { return _mm_asinh_ps(A); },
  [](const xwector& A) noexcept { return _mm256_asinh_pd(A); },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_asinh_ps(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_asinh_pd(A[Is])), ...); }(make_sequence<N>{});
  }};

/// XVオブジェクトの各要素に`cos`関数を適用する
/// @note 1. `xvcos(const Xv& A)`
/// @note 2. `xvcos(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result`の参照先は`A`と同一であってもよい
inline constexpr overload xvcos{
  [](const xvector& A) noexcept { return _mm_cos_ps(A); },
  [](const xwector& A) noexcept { return _mm256_cos_pd(A); },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_cos_ps(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_cos_pd(A[Is])), ...); }(make_sequence<N>{});
  }};

/// XVオブジェクトの各要素に`acos`関数を適用する
/// @note 1. `xvacos(const Xv& A)`
/// @note 2. `xvacos(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result`の参照先は`A`と同一であってもよい
inline constexpr overload xvacos{
  [](const xvector& A) noexcept { return _mm_acos_ps(A); },
  [](const xwector& A) noexcept { return _mm256_acos_pd(A); },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_acos_ps(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_acos_pd(A[Is])), ...); }(make_sequence<N>{});
  }};

/// XVオブジェクトの各要素に`cosh`関数を適用する
/// @note 1. `xvcosh(const Xv& A)`
/// @note 2. `xvcosh(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result`の参照先は`A`と同一であってもよい
inline constexpr overload xvcosh{
  [](const xvector& A) noexcept { return _mm_cosh_ps(A); },
  [](const xwector& A) noexcept { return _mm256_cosh_pd(A); },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_cosh_ps(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_cosh_pd(A[Is])), ...); }(make_sequence<N>{});
  }};

/// XVオブジェクトの各要素に`acosh`関数を適用する
/// @note 1. `xvacosh(const Xv& A)`
/// @note 2. `xvacosh(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result`の参照先は`A`と同一であってもよい
inline constexpr overload xvacosh{
  [](const xvector& A) noexcept { return _mm_acosh_ps(A); },
  [](const xwector& A) noexcept { return _mm256_acosh_pd(A); },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_acosh_ps(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_acosh_pd(A[Is])), ...); }(make_sequence<N>{});
  }};

/// XVオブジェクトの各要素に`tan`関数を適用する
/// @note 1. `xvtan(const Xv& A)`
/// @note 2. `xvtan(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result`の参照先は`A`と同一であってもよい
inline constexpr overload xvtan{
  [](const xvector& A) noexcept { return _mm_tan_ps(A); },
  [](const xwector& A) noexcept { return _mm256_tan_pd(A); },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_tan_ps(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_tan_pd(A[Is])), ...); }(make_sequence<N>{});
  }};

/// XVオブジェクトの各要素に`atan`関数を適用する
/// @note 1. `xvatan(const Xv& A)`
/// @note 2. `xvatan(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result`の参照先は`A`あるいは`B`と同一であってもよい
/// @note 3. `xvatan(const Xv& Sin, const Xv& Cos)`
/// @note 第2引数までを指定した場合、`atan2(Sin, Cos)`を計算する
/// @note 4. `xvatan(const Xv (&Sin)[N], const Xv (&Cos)[N], Xv (&Result)[N])`
/// @note `Result`の参照先は`Sin`あるいは`Cos`と同一であってもよい
inline constexpr overload xvatan{
  [](const xvector& A) noexcept { return _mm_atan_ps(A); },
  [](const xwector& A) noexcept { return _mm256_atan_pd(A); },
  [](const xvector& Sin, const xvector& Cos) noexcept { return _mm_atan2_ps(Sin, Cos); },
  [](const xwector& Sin, const xwector& Cos) noexcept { return _mm256_atan2_pd(Sin, Cos); },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_atan_ps(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_atan_pd(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xvector (&Sin)[N], const xvector (&Cos)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_atan2_ps(Sin[Is], Cos[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&Sin)[N], const xwector (&Cos)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_atan2_pd(Sin[Is], Cos[Is])), ...); }(make_sequence<N>{});
  }};

/// XVオブジェクトの各要素に`tanh`関数を適用する
/// @note 1. `xvtanh(const Xv& A)`
/// @note 2. `xvtanh(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result`の参照先は`A`と同一であってもよい
inline constexpr overload xvtanh{
  [](const xvector& A) noexcept { return _mm_tanh_ps(A); },
  [](const xwector& A) noexcept { return _mm256_tanh_pd(A); },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_tanh_ps(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_tanh_pd(A[Is])), ...); }(make_sequence<N>{});
  }};

/// XVオブジェクトの各要素に`atanh`関数を適用する
/// @note 1. `xvatanh(const Xv& A)`
/// @note 2. `xvatanh(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result`の参照先は`A`と同一であってもよい
inline constexpr overload xvatanh{
  [](const xvector& A) noexcept { return _mm_atanh_ps(A); },
  [](const xwector& A) noexcept { return _mm256_atanh_pd(A); },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_atanh_ps(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_atanh_pd(A[Is])), ...); }(make_sequence<N>{});
  }};

/// XVオブジェクトの各要素に`pow`関数を適用する
/// @note 1. `xvpow(const Xv& A, const Xv& B)`
/// @note 2. `xvpow(const Xv (&A)[N], const Xv (&B)[N], Xv (&Result)[N])`
/// @note `Result`の参照先は`A`あるいは`B`と同一であってもよい
inline constexpr overload xvpow{
  [](const xvector& A, const xvector& B) noexcept { return _mm_pow_ps(A, B); },
  [](const xwector& A, const xwector& B) noexcept { return _mm256_pow_pd(A, B); },
  []<natt N>(const xvector (&A)[N], const xvector (&B)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_pow_ps(A[Is], B[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], const xwector (&B)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_pow_pd(A[Is], B[Is])), ...); }(make_sequence<N>{});
  }};

/// XVオブジェクトの各要素に`exp`関数を適用する
/// @note 1. `xvexp(const Xv& A)`
/// @note 2. `xvexp(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result`の参照先は`A`あるいは`B`と同一であってもよい
inline constexpr overload xvexp{
  [](const xvector& A) noexcept { return _mm_exp_ps(A); },
  [](const xwector& A) noexcept { return _mm256_exp_pd(A); },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_exp_ps(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_exp_pd(A[Is])), ...); }(make_sequence<N>{});
  }};

/// XVオブジェクトの各要素に`exp2`関数を適用する
/// @note 1. `xvexp2(const Xv& A)`
/// @note 2. `xvexp2(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result`の参照先は`A`あるいは`B`と同一であってもよい
inline constexpr overload xvexp2{
  [](const xvector& A) noexcept { return _mm_exp2_ps(A); },
  [](const xwector& A) noexcept { return _mm256_exp2_pd(A); },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_exp2_ps(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_exp2_pd(A[Is])), ...); }(make_sequence<N>{});
  }};

/// XVオブジェクトの各要素に`exp10`関数を適用する
/// @note 1. `xvexp10(const Xv& A)`
/// @note 2. `xvexp10(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result`の参照先は`A`あるいは`B`と同一であってもよい
inline constexpr overload xvexp10{
  [](const xvector& A) noexcept { return _mm_exp10_ps(A); },
  [](const xwector& A) noexcept { return _mm256_exp10_pd(A); },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_exp10_ps(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_exp10_pd(A[Is])), ...); }(make_sequence<N>{});
  }};

/// XVオブジェクトの各要素に`expm1`関数を適用する
/// @note 1. `xvexpm1(const Xv& A)`
/// @note 2. `xvexpm1(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result`の参照先は`A`あるいは`B`と同一であってもよい
inline constexpr overload xvexpm1{
  [](const xvector& A) noexcept { return _mm_expm1_ps(A); },
  [](const xwector& A) noexcept { return _mm256_expm1_pd(A); },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_expm1_ps(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_expm1_pd(A[Is])), ...); }(make_sequence<N>{});
  }};

/// XVオブジェクトの各要素に`ln`関数を適用する
/// @note 1. `xvln(const Xv& A)`
/// @note 2. `xvln(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result`の参照先は`A`と同一であってもよい
inline constexpr overload xvln{
  [](const xvector& A) noexcept { return _mm_log_ps(A); },
  [](const xwector& A) noexcept { return _mm256_log_pd(A); },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_log_ps(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_log_pd(A[Is])), ...); }(make_sequence<N>{});
  }};

/// XVオブジェクトについて、底を指定した対数を計算する
/// @note 1. `xvlog(const Xv& A, const Xv& Base)`
/// @note 2. `xvlog(const Xv (&A)[N], const Xv (&Base)[N], Xv (&Result)[N])`
/// @note `Result`の参照先は`A`あるいは`Base`と同一であってもよい
inline constexpr overload xvlog{
  [](const xvector& A, const xvector& Base) noexcept { return xvdiv(xvln(A), xvln(Base)); },
  [](const xwector& A, const xwector& Base) noexcept { return xvdiv(xvln(A), xvln(Base)); },
  []<natt N>(const xvector (&A)[N], const xvector (&Base)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = xvdiv(xvln(A[Is]), xvln(Base[Is]))), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], const xwector (&Base)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = xvdiv(xvln(A[Is]), xvln(Base[Is]))), ...); }(make_sequence<N>{});
  }};

/// XVオブジェクトの各要素に`log2`関数を適用する
/// @note 1. `xvlog2(const Xv& A)`
/// @note 2. `xvlog2(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result`の参照先は`A`と同一であってもよい
inline constexpr overload xvlog2{
  [](const xvector& A) noexcept { return _mm_log2_ps(A); },
  [](const xwector& A) noexcept { return _mm256_log2_pd(A); },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_log2_ps(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_log2_pd(A[Is])), ...); }(make_sequence<N>{});
  }};

/// XVオブジェクトの各要素に`log10`関数を適用する
/// @note 1. `xvlog10(const Xv& A)`
/// @note 2. `xvlog10(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result`の参照先は`A`と同一であってもよい
inline constexpr overload xvlog10{
  [](const xvector& A) noexcept { return _mm_log10_ps(A); },
  [](const xwector& A) noexcept { return _mm256_log10_pd(A); },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_log10_ps(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_log10_pd(A[Is])), ...); }(make_sequence<N>{});
  }};

/// XVオブジェクトの各要素に`log1p`関数を適用する
/// @note 1. `xvlog1p(const Xv& A)`
/// @note 2. `xvlog1p(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result`の参照先は`A`と同一であってもよい
inline constexpr overload xvlog1p{
  [](const xvector& A) noexcept { return _mm_log1p_ps(A); },
  [](const xwector& A) noexcept { return _mm256_log1p_pd(A); },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_log1p_ps(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_log1p_pd(A[Is])), ...); }(make_sequence<N>{});
  }};

/// XVオブジェクトの各要素に`logb`関数を適用する
/// @note 1. `xvlogb(const Xv& A)`
/// @note 2. `xvlogb(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result`の参照先は`A`あるいは`B`と同一であってもよい
inline constexpr overload xvlogb{
  [](const xvector& A) noexcept { return _mm_logb_ps(A); },
  [](const xwector& A) noexcept { return _mm256_logb_pd(A); },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_logb_ps(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_logb_pd(A[Is])), ...); }(make_sequence<N>{});
  }};

/// XVオブジェクトの各要素に`sqrt`関数を適用する
/// @note 1. `xvsqrt(const Xv& A)`
/// @note 2. `xvsqrt(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result`の参照先は`A`あるいは`B`と同一であってもよい
inline constexpr overload xvsqrt{
  [](const xvector& A) noexcept { return _mm_sqrt_ps(A); },
  [](const xwector& A) noexcept { return _mm256_sqrt_pd(A); },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_sqrt_ps(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_sqrt_pd(A[Is])), ...); }(make_sequence<N>{});
  }};

/// XVオブジェクトの各要素に`cbrt`関数を適用する
/// @note 1. `xvcbrt(const Xv& A)`
/// @note 2. `xvcbrt(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result`の参照先は`A`あるいは`B`と同一であってもよい
inline constexpr overload xvcbrt{
  [](const xvector& A) noexcept { return _mm_cbrt_ps(A); },
  [](const xwector& A) noexcept { return _mm256_cbrt_pd(A); },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_cbrt_ps(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_cbrt_pd(A[Is])), ...); }(make_sequence<N>{});
  }};

/// XVオブジェクトの各要素に`hypot`関数を適用する
/// @note 1. `xvhypot(const Xv& A, const Xv& B)`
/// @note 2. `xvhypot(const Xv (&A)[N], const Xv (&B)[N], Xv (&Result)[N])`
/// @note `Result`の参照先は`A`あるいは`B`と同一であってもよい
inline constexpr overload xvhypot{
  [](const xvector& A, const xvector& B) noexcept { return _mm_hypot_ps(A, B); },
  [](const xwector& A, const xwector& B) noexcept { return _mm256_hypot_pd(A, B); },
  []<natt N>(const xvector (&A)[N], const xvector (&B)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_hypot_ps(A[Is], B[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], const xwector (&B)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_hypot_pd(A[Is], B[Is])), ...); }(make_sequence<N>{});
  }};

/// XVオブジェクトの各要素に`sqrt_r`関数を適用する
/// @note 1. `xvsqrt_r(const Xv& A)`
/// @note 2. `xvsqrt_r(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result`の参照先は`A`と同一であってもよい
inline constexpr overload xvsqrt_r{
  [](const xvector& A) noexcept { return _mm_invsqrt_ps(A); },
  [](const xwector& A) noexcept { return _mm256_invsqrt_pd(A); },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_invsqrt_ps(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_invsqrt_pd(A[Is])), ...); }(make_sequence<N>{});
  }};

/// XVオブジェクトの各要素に`cbrt_r`関数を適用する
/// @note 1. `xvcbrt_r(const Xv& A)`
/// @note 2. `xvcbrt_r(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result`の参照先は`A`と同一であってもよい
inline constexpr overload xvcbrt_r{
  [](const xvector& A) noexcept { return _mm_invcbrt_ps(A); },
  [](const xwector& A) noexcept { return _mm256_invcbrt_pd(A); },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_invcbrt_ps(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_invcbrt_pd(A[Is])), ...); }(make_sequence<N>{});
  }};

/// XVオブジェクトの各要素に`erf`関数を適用する
/// @note 1. `xverf(const Xv& A)`
/// @note 2. `xverf(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result`の参照先は`A`と同一であってもよい
inline constexpr overload xverf{
  [](const xvector& A) noexcept { return _mm_erf_ps(A); },
  [](const xwector& A) noexcept { return _mm256_erf_pd(A); },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_erf_ps(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_erf_pd(A[Is])), ...); }(make_sequence<N>{});
  }};

/// XVオブジェクトの各要素に`erfc`関数を適用する
/// @note 1. `xverfc(const Xv& A)`
/// @note 2. `xverfc(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result`の参照先は`A`と同一であってもよい
inline constexpr overload xverfc{
  [](const xvector& A) noexcept { return _mm_erfc_ps(A); },
  [](const xwector& A) noexcept { return _mm256_erfc_pd(A); },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_erfc_ps(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_erfc_pd(A[Is])), ...); }(make_sequence<N>{});
  }};

/// XVオブジェクトの各要素に`erf_r`関数を適用する
/// @note 1. `xverf_r(const Xv& A)`
/// @note 2. `xverf_r(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result`の参照先は`A`と同一であってもよい
inline constexpr overload xverf_r{
  [](const xvector& A) noexcept { return _mm_erfinv_ps(A); },
  [](const xwector& A) noexcept { return _mm256_erfinv_pd(A); },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_erfinv_ps(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_erfinv_pd(A[Is])), ...); }(make_sequence<N>{});
  }};

/// XVオブジェクトの各要素に`erfc_r`関数を適用する
/// @note 1. `xverfc_r(const Xv& A)`
/// @note 2. `xverfc_r(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result`の参照先は`A`と同一であってもよい
inline constexpr overload xverfc_r{
  [](const xvector& A) noexcept { return _mm_erfcinv_ps(A); },
  [](const xwector& A) noexcept { return _mm256_erfcinv_pd(A); },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_erfcinv_ps(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_erfcinv_pd(A[Is])), ...); }(make_sequence<N>{});
  }};

/// XVオブジェクトを`N`次のベクトルとみなして内積を計算する
/// @note `Zero`を指定することで、ビットが`1`の要素をゼロ埋めできる
/// @note `xvdot<3, 0b1101>({1,2,3,4}, {1,2,3,4}) -> {14, 0, 14, 14}`
template<natt N, natt Zero = 0> requires(0 < N && N <= 4 && Zero < 16)
xvector xvdot(const xvector& A, const xvector& B) noexcept {
  constexpr natt fz = inspects<!(Zero & 1), !(Zero & 2), !(Zero & 4), !(Zero & 8)>;
  if constexpr (N == 1 && (Zero == 14 || Zero == 13 || Zero == 11 || Zero == 7)) return xvinsert<fz, 0, Zero>(A, xvmul(A, B));
  else return _mm_dp_ps(A, B, value_switch<N, 0, 16, 48, 112, 240> | (15 ^ Zero));
}

/// XVオブジェクトを`N`次のベクトルとみなして内積を計算する
/// @note `Zero`を指定することで、ビットが`1`の要素をゼロ埋めできる
/// @note `xvdot<3, 0b1101>({1,2,3,4}, {1,2,3,4}) -> {14, 0, 14, 14}`
template<natt N, natt Zero = 0> requires(0 < N && N <= 4 && Zero < 16)
xwector xvdot(const xwector& A, const xwector& B) noexcept {
  if constexpr (N == 0 || Zero == 0b1111) return xv_zero;
  xwector a;
  if constexpr (N == 1) a = xvmul(A, B);
  else if constexpr (N == 2) a = _mm256_hadd_pd(xvmul(A, B), A);
  else if constexpr (N == 3) (a = xvmul(A, B)), a = xvadd(_mm256_hadd_pd(a, A), _mm256_castpd128_pd256(_mm256_extractf128_pd(a, 1)));
  else if constexpr (N == 4) (a = _mm256_hadd_pd(xvmul(A, B), A)), a = xvadd(a, _mm256_castpd128_pd256(_mm256_extractf128_pd(a, 1)));
  return xvsetzero<Zero & 1, Zero & 2, Zero & 4, Zero & 8>(xvpermute<0, 0, 0, 0>(a));
}

/// XVオブジェクトを`N`次のベクトルとみなして`M`行の行列との内積を計算する
/// @note `Result`の参照先が`A`や`B`に含まれる場合、正しい計算結果は得られない
/// @note `xvdot<3>({{1,2,3,4},{5,6,7,8}}, {1,2,3,4}, R) -> R{14,38,0,0}`
template<natt N, natt M> requires(0 < N && N <= 4 && M <= 4)
void xvdot(const xvector (&A)[M], const xvector& B, xvector& Result) noexcept {
  if constexpr (1 <= N) Result = xvdot<N, 0b1110>(A[0], B);
  if constexpr (2 <= N) Result = xvpermute<0, 5, 2, 3>(Result, xvdot<N>(A[1], B));
  if constexpr (3 <= N) Result = xvpermute<0, 1, 6, 3>(Result, xvdot<N>(A[2], B));
  if constexpr (4 == N) Result = xvpermute<0, 1, 2, 7>(Result, xvdot<N>(A[3], B));
}

/// XVオブジェクトを`M`次のベクトルとみなして`M`行`N`列の行列との内積を計算する
/// @note `Result`の参照先が`A`や`B`に含まれる場合、正しい計算結果は得られない
/// @note `xvdot<3>({1,2,3,4}, {{1,2,3,4},{5,6,7,8}}, R) -> R{11,14,17,0}`
template<natt N, natt M> requires(0 < N && N <= 4 && M <= 4)
void xvdot(const xvector& A, const xvector (&B)[M], xvector& Result) noexcept {
  if constexpr (1 <= M) Result = xvmul(xvpermute<0, 0, 0, 0>(A), B[0]);
  if constexpr (2 <= M) Result = xvfma(xvpermute<1, 1, 1, 1>(A), B[1], Result);
  if constexpr (3 <= M) Result = xvfma(xvpermute<2, 2, 2, 2>(A), B[2], Result);
  if constexpr (4 == M) Result = xvfma(xvpermute<3, 3, 3, 3>(A), B[3], Result);
  if constexpr (N != 4) Result = _mm_blend_ps(xv_zero, Result, (1 << N) - 1);
}

/// `L`行`M`列の行列と`M`行`N`列の行列の内積を計算する
/// @note `Result`の参照先が`A`や`B`と重なる場合、正しい計算結果は得られない
/// @note `ywdot<3>({{1,2,3,4},{5,6,7,8}}, {{1,2,3,4},{5,6,7,8},{9,10,11,12}}, R) -> R{{38,44,50,0},{98,116,134,0}}
template<natt N, natt M, natt L> requires(0 < N && N <= 4 && M <= 4 && L <= 4)
void xvdot(const xvector (&A)[L], const xvector (&B)[M], xvector (&Result)[L]) noexcept {
  if constexpr (1 <= L) xvdot<N>(A[0], B, L[0]);
  if constexpr (2 <= L) xvdot<N>(A[1], B, L[1]);
  if constexpr (3 <= L) xvdot<N>(A[2], B, L[2]);
  if constexpr (4 == L) xvdot<N>(A[3], B, L[3]);
}

/// XVオブジェクトを三次のベクトルとみなして外積を計算する
inline constexpr overload xvcross{
  [](const xvector& A, const xvector& B) noexcept {
    auto a = xvpermute<2, 0, 1, 3>(A), b = xvpermute<1, 2, 0, 3>(B);
    return xvfms(xvpermute<1, 2, 0, 3>(A), xvpermute<2, 0, 1, 3>(B), xvmul(a, b));
  },
  [](const xwector& A, const xwector& B) noexcept {
    auto a = xvpermute<2, 0, 1, 3>(A), b = xvpermute<1, 2, 0, 3>(B);
    return xvfms(xvpermute<1, 2, 0, 3>(A), xvpermute<2, 0, 1, 3>(B), xvmul(a, b));
  }};

/// XVオブジェクトを`N`次のベクトルとみなして長さを計算する
/// @note `Zero`を指定することで、ビットが`1`の要素をゼロ埋めできる
template<natt N, natt Zero = 0> requires(N <= 4 && Zero < 16)
inline constexpr overload xvlength{[](const xvector& A) noexcept { return xvsqrt(xvdot<N, Zero>(A, A)); },
                                   [](const xwector& A) noexcept { return xvsqrt(xvdot<N, Zero>(A, A)); }};

/// XVオブジェクトを`N`次のベクトルとみなして長さの逆数を計算する
/// @note `Zero`を指定することで、ビットが`1`の要素をゼロ埋めできる
template<natt N, natt Zero = 0> requires(N <= 4 && Zero < 16)
inline constexpr overload xvlength_r{[](const xvector& A) noexcept { return xvsqrt_r(xvdot<N, Zero>(A, A)); },
                                     [](const xwector& A) noexcept { return xvsqrt_r(xvdot<N, Zero>(A, A)); }};

/// XVオブジェクトを`N`次のベクトルとみなして正規化する
template<natt N> requires(N <= 4) inline constexpr overload xvnormalize{
  [](const xvector& A) noexcept { return xvmul(A, xvlength_r<N>(A)); },
  [](const xwector& A) noexcept { return xvmul(A, xvlength_r<N>(A)); }};

/// XVオブジェクトが角度を表しているとみなして、ラジアンに変換する
inline constexpr overload xvradian{
  [](const xvector& A) noexcept { return xvmul(A, xv_constant<value::pi / 180_vv>); },
  [](const xwector& A) noexcept { return xvmul(A, xv_constant<value::pi / 180_vv>); }};

/// XVオブジェクトがラジアンを表しているとみなして、度数に変換する
inline constexpr overload xvdegree{
  [](const xvector& A) noexcept { return xvmul(A, xv_constant<180_vv / value::pi>); },
  [](const xwector& A) noexcept { return xvmul(A, xv_constant<180_vv / value::pi>); }};

// clang-format off

/// XV行列を転置する
inline constexpr overload xvtranspose{
  []<typename T, typename U>(T&& A, U&& Result) noexcept
    requires((nt_tuple_for<T, const xvector&> && nt_tuple_for<U, xvector&>) ||
             (nt_tuple_for<T, const xwector&> && nt_tuple_for<U, xwector&>)) {
    constexpr natt M = extent<T>, N = extent<U>;
    if constexpr (1 == M) {
      if constexpr (1 <= N) get<0>(Result) = xvsetzero<0, 1, 1, 1>(get<0>(A));
      if constexpr (2 <= N) get<1>(Result) = xvinsert<0, 1, 0b1110>(get<0>(A), get<0>(A));
      if constexpr (3 <= N) get<2>(Result) = xvinsert<0, 2, 0b1110>(get<0>(A), get<0>(A));
      if constexpr (4 == N) get<3>(Result) = xvinsert<0, 3, 0b1110>(get<0>(A), get<0>(A));
    } else if constexpr (2 == M) {
      if constexpr (1 <= N) get<0>(Result) = xvinsert<1, 0, 0b1100>(get<0>(A), get<1>(A));
      if constexpr (2 <= N) get<1>(Result) = xvinsert<0, 1, 0b1100>(get<1>(A), get<0>(A));
      if constexpr (3 <= N) get<2>(Result) = xvinsert<1, 2, 0b1100>(xvpermute<2, -1, -1, -1>(get<0>(A)), get<1>(A));
      if constexpr (4 == N) get<3>(Result) = xvinsert<1, 3, 0b1100>(xvpermute<3, -1, -1, -1>(get<0>(A)), get<1>(A));
    } else if constexpr (3 == M) {
      if constexpr (1 <= N) get<0>(Result) = xvinsert<2, 0, 0b1000>(xvpermute<0, 4, -1, -1>(get<0>(A), get<1>(A)), get<2>(A));
      if constexpr (2 <= N) get<1>(Result) = xvinsert<2, 1, 0b1000>(xvpermute<1, 5, -1, -1>(get<0>(A), get<1>(A)), get<2>(A));
      if constexpr (3 <= N) get<2>(Result) = xvinsert<2, 2, 0b1000>(xvpermute<2, 6, -1, -1>(get<0>(A), get<1>(A)), get<2>(A));
      if constexpr (4 == N) get<3>(Result) = xvinsert<2, 3, 0b1000>(xvpermute<3, 7, -1, -1>(get<0>(A), get<1>(A)), get<2>(A));
    } else {
      if constexpr (N == 1) get<0>(Result) = xvpermute<0, 1, 4, 5>(xvpermute<0, 4, -1, -1>(get<0>(A), get<1>(A)), xvpermute<0, 4, -1, -1>(get<2>(A), get<3>(A)));
      if constexpr (N >= 2) {
        auto a = xvpermute<0, 1, 4, 5>(get<0>(A), get<1>(A)), b = xvpermute<0, 1, 4, 5>(get<2>(A), get<3>(A));
        get<0>(Result) = xvpermute<0, 2, 4, 6>(a, b), get<1>(Result) = xvpermute<1, 3, 5, 7>(a, b);
      }
      if constexpr (N == 3) get<3>(Result) = xvpermute<0, 1, 4, 5>(xvpermute<2, 6, -1, -1>(get<0>(A), get<1>(A)), xvpermute<2, 6, -1, -1>(get<2>(A), get<3>(A)));
      if constexpr (N == 4) {
        auto a = xvpermute<2, 3, 6, 7>(get<0>(A), get<1>(A)), b = xvpermute<2, 3, 6, 7>(get<2>(A), get<3>(A));
        get<2>(Result) = xvpermute<0, 2, 4, 6>(a, b), get<3>(Result) = xvpermute<1, 3, 5, 7>(a, b);
      }
    }
  }};

// clang-format on

/// Generates a transformation matrix for translation.
inline constexpr auto xvtranslate =
  []<typename T>(const xvector& Offset, T&& Result) noexcept requires(nt_tuple_for<T, xvector&>) {
    constexpr natt N = extent<T>;
    if constexpr (N >= 1) get<0>(Result) = xvinsert<3, 0>(xv_x, Offset);
    if constexpr (N >= 2) get<1>(Result) = xvinsert<3, 1>(xv_y, Offset);
    if constexpr (N >= 3) get<2>(Result) = xvinsert<3, 2>(xv_z, Offset);
    if constexpr (N >= 4) get<3>(Result) = xv_w;
  };

/// Generates a transformation matrix for rotation.
/// @note If `N == 2`, the two-dimensional rotation matrix at `Radian[0]`.
/// @note If `N >= 3`, the three-dimensional rotation matrix at `Radian[0-2]`.
/// @note If `N == 4`, `Result[3] = {0,0,0,1}`
inline constexpr auto xvrotate = []<nt_tuple_for<xvector&> T>(const xvector& Radian, T&& Result) noexcept {
  constexpr natt N = extent<T>;
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
  } else static_assert(1 < N && N < 5);
};

/// Generates the inverse matrix of the transformation matrix generated by `xvrotate`.
inline constexpr auto xvrotate_inv = []<nt_tuple_for<xvector&> T>(const xvector& Radian, T&& Result) noexcept {
  constexpr natt N = extent<T>;
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
  } else static_assert(1 < N && N < 5);
};

/// 拡大・縮小のための変形行列を生成する
/// @note `N == 2`なら`Scalar[0-1]`での二次元拡大・縮小行列
/// @note `N >= 3`なら`Scalar[0-2]`での三次元拡大・縮小行列
/// @note `N == 4`のとき`Result[3] = {0,0,0,1}`

/// Generates a transformation matrix for scaling.
inline constexpr auto xvscale = []<nt_tuple_for<xvector&> T>(const xvector& Scalar, T&& Result) noexcept {
  constexpr natt N = extent<T>;
  if constexpr (N >= 2) {
    get<0>(Result) = xvinsert<0, 0, 0b1110>(Scalar, Scalar);
    get<1>(Result) = xvinsert<1, 1, 0b1101>(Scalar, Scalar);
  }
  if constexpr (N >= 3) get<2>(Result) = xvinsert<2, 2, 0b1011>(Scalar, Scalar);
  if constexpr (N == 4) get<3>(Result) = xv_w;
  static_assert(1 < N && N < 5);
};

// clang-format off

/// Generates a transformation matrix.
/// @note `xvworld(Result) -> Result = xv_identity`
/// @note `xvworld(Offset, Result) -> Result = xvtranslate(Offset)`
/// @note `xvworld(Offset, Radian, Result) -> Result = xvtranslate(Offset) * xvrotate(Radian)`
/// @note `xvworld(Offset, Radian, Scalar, Result) -> Result = xvtranslate(Offset) * xvrotate(Radian) * xvscale(Scalar)`
inline constexpr overload xvworld{
  []<nt_tuple_for<xvector&> T>(T&& Result) noexcept{
    constexpr natt N = extent<T>;
    if constexpr (N >= 2) Result[0] = xv_x, Result[1] = xv_y;
    if constexpr (N >= 3) Result[2] = xv_z;
    if constexpr (N == 4) Result[3] = xv_w;
    static_assert(1 < N && N < 5);
  },
  []<nt_tuple_for<xvector&> T>(const xvector& Offset, T&& Result) noexcept { xvtranslate(Offset, Result); },
  []<nt_tuple_for<xvector&> T>(const xvector& Offset, const xvector& Radian, T&& Result) noexcept {
    constexpr natt N = extent<T>;
    xvrotate(Radian, Result);
    Result[0] = xvinsert<3, 0>(Result[0], Offset);
    Result[1] = xvinsert<3, 1>(Result[1], Offset);
    if constexpr (N >= 3) Result[2] = xvinsert<3, 2>(Result[2], Offset);
    static_assert(1 < N && N < 5);
  },
  []<nt_tuple_for<xvector&> T>(const xvector& Offset, const xvector& Radian, const xvector& Scalar, T&& Result) noexcept {
    constexpr natt N = extent<T>;
    xvrotate(Radian, Result);
    Result[0] = xvinsert<3, 0>(Result[0], Offset);
    Result[1] = xvinsert<3, 1>(Result[1], Offset);
    if constexpr (N >= 3) Result[2] = xvinsert<3, 2>(Result[2], Offset);
    Result[0] = xvmul(Result[0], xvpermute<0, 0, 0, 0>(Scalar));
    Result[1] = xvmul(Result[1], xvpermute<1, 1, 1, 1>(Scalar));
    if constexpr (N >= 3) Result[2] = xvmul(Result[2], xvpermute<2, 2, 2, 2>(Scalar));
    static_assert(1 < N && N < 5);
  }};

/// Generates a view matrix.
/// @note `xvview(Radian, Result) -> Result = xvrotate_inv(Radian)`
/// @note `xvview(Position, Radian, Result) -> Result = xvrotate_inv(Radian) * xvtranslate(-Position)`
/// @note `xvview(Position, Radian, Offset, Result) -> Result = xvtranslate(-Offset) * xvrotate_inv(Radian) * xvtranslate(-Position)`
inline constexpr overload xvview{
  []<nt_tuple_for<xvector&> T, natt N = extent<T>>(const xvector& Radian, T&& Result)
    noexcept requires (le(2, N) && le(N, 4)) { xvrotate_inv(Radian, Result); },
  []<nt_tuple_for<xvector&> T, natt N = extent<T>>(const xvector& Position, const xvector& Radian, T&& Result)
    noexcept requires (le(2, N) && le(N, 4)) { xvrotate_inv(Radian, Result);
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
    } },
  []<nt_tuple_for<xvector&> T, natt N = extent<T>>(const xvector& Position, const xvector& Radian, const xvector& Offset, T&& Result)
    noexcept requires (le(2, N) && le(N, 4)) { xvrotate_inv(Radian, Result);
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
    } } };

/// カメラ行列を生成する
/// @note 入力用の引数が３個の場合、Perspectiveなカメラ行列を生成する
/// @note 入力用の引数が４個の場合、Orthographicなカメラ行列を生成する
inline constexpr overload xvcamera{
  []<natt N>(const fat4 Width, const fat4 Height, const fat4 Factor, xvector (&Result)[N]) noexcept requires(le(3, N) && le(N, 4)) {
    constexpr value f = 1048576.0, n = 0.25;
    Result[0] = xvset<0>(xv_zero, -Factor * Height / Width);
    Result[1] = xvset<1>(xv_zero, Factor);
    Result[2] = xv_constant<0, 0, f / (f - n), -f * n / (f - n)>;
    if constexpr (N == 4) Result[3] = xv_z;
  },
                                                                                                            []<natt N>(const fat4 Width, const fat4 Height, const fat4 Factor, none OrthgraphicMode, xvector (&Result)[N]) noexcept requires(le(3, N) && le(N, 4)) {
    constexpr value f = 1048576.0, n = 0.25;
    Result[0] = xvset<0>(xv_zero, -2.0f * Factor / Width);
    Result[1] = xvset<1>(xv_zero, 2.0f * Factor / Height);
    Result[2] = xv_constant<0, 0, 1_vv / (f - n), -n / (f - n)>;
    if constexpr (N == 4) Result[3] = xv_w;
  }};

// clang-format on

/// 回転行列をオイラー角に変換する
// struct t_xveuler {
//   xvector operator()(const xvector (&Rotation)[4]) const noexcept {
//     xvector v = xvneg(xvpermute<3, 0, 3, 3>(Rotation[2]));
//     if (xveq(v, xv_y)) return xvasin(xvpermute<5, 1, -1, -1>(v, Rotation[0]));
//     v = xvpermute<0, 1, 6, 3>(xvpermute<4, 1, -1, 3>(v, xvdiv(Rotation[1], Rotation[0])), xvdiv(xvpermute<-1, -1, 1, -1>(Rotation[2]), Rotation[2]));
//     return xvpermute<6, 1, 4, 3>(xvasin(v), xvatan(xvpermute<0, -1, 5, -1>(Rotation[1], Rotation[2]), xvblend<0, 0, 1, 1>(Rotation[0], Rotation[2])));
//   }
// };

/// 単精度浮動小数点数を４要素(`x`,`y`,`z`,`w`)を持つ構造体
struct vector {
  fat4 x{}, y{}, z{}, w{};
  constexpr vector() noexcept = default;
  explicit constexpr vector(const fat4 Fill) noexcept : x(Fill), y(Fill), z(Fill), w(Fill) {}
  explicit constexpr vector(const arithmetic auto Fill) noexcept : vector(fat4(Fill)) {}
  template<castable_to<int4> T0, castable_to<int4> T1, castable_to<int4> T2, castable_to<int4> T3>
  constexpr vector(T0&& X, T1&& Y, T2&& Z, T3&& W) noexcept(
    nt_castable_to<T0, int4> && nt_castable_to<T1, int4> && nt_castable_to<T2, int4> && nt_castable_to<T3, int4>)
    : x(static_cast<fat4>(fwd<T0>(X))), y(static_cast<fat4>(fwd<T1>(Y))),
      z(static_cast<fat4>(fwd<T2>(Z))), w(static_cast<fat4>(fwd<T3>(W))) {}
  template<castable_to<int4> T0, castable_to<int4> T1, castable_to<int4> T2>
  constexpr vector(T0&& X, T1&& Y, T2&& Z) noexcept(
    nt_castable_to<T0, int4> && nt_castable_to<T1, int4> && nt_castable_to<T2, int4>)
    : x(static_cast<fat4>(fwd<T0>(X))), y(static_cast<fat4>(fwd<T1>(Y))), z(static_cast<fat4>(fwd<T2>(Z))) {}
  template<castable_to<int4> T0, castable_to<int4> T1> constexpr vector(
    T0&& X, T1&& Y) noexcept(nt_castable_to<T0, int4> && nt_castable_to<T1, int4>)
    : x(static_cast<fat4>(fwd<T0>(X))), y(static_cast<fat4>(fwd<T1>(Y))) {}

  vector(const xvector& A) noexcept { _mm_storeu_ps(&x, A); }
  vector& operator=(const xvector& A) noexcept { return _mm_storeu_ps(&x, A), *this; }
  operator xvector() const noexcept { return xv(&x); }
  explicit operator bool() const noexcept { return x || y || z || w; }
  constexpr natt size() const noexcept { return 4; }
  fat4* data() noexcept { return &x; }
  const fat4* data() const noexcept { return &x; }
  fat4* begin() noexcept { return &x; }
  const fat4* begin() const noexcept { return &x; }
  fat4* end() noexcept { return &x + 4; }
  const fat4* end() const noexcept { return &x + 4; }
  fat4& operator[](natt i) { return *(&x + i); }
  const fat4& operator[](natt i) const { return *(&x + i); }
  template<natt Ix> requires(Ix < 4) constexpr fat4& get() noexcept {
    if constexpr (Ix == 0) return x;
    else if constexpr (Ix == 1) return y;
    else if constexpr (Ix == 2) return z;
    else return w;
  }
  template<natt Ix> requires(Ix < 4) constexpr fat4 get() const noexcept {
    if constexpr (Ix == 0) return x;
    else if constexpr (Ix == 1) return y;
    else if constexpr (Ix == 2) return z;
    else return w;
  }
  friend constexpr bool operator==(const vector& A, const vector& B) noexcept {
    if (!is_cev) return xveq(A, B);
    return A.x == B.x && A.y == B.y && A.z == B.z && A.w == B.w;
  }
  friend constexpr auto operator<=>(const vector& A, const vector& B) noexcept {
    if (!is_cev) return std::partial_ordering(xvtw(A, B));
    if (auto a = A.x <=> B.x; a != 0) return a;
    else if (a = A.y <=> B.y; a != 0) return a;
    else if (a = A.z <=> B.z; a != 0) return a;
    else return A.w <=> B.w;
  }
  friend constexpr vector operator+(const vector& A) noexcept { return A; }
  friend constexpr vector operator-(const vector& A) noexcept {
    if (!is_cev) return xvneg(A);
    else return {-A.x, -A.y, -A.z, -A.w};
  }
  friend constexpr vector operator+(const vector& A, const vector& B) noexcept {
    if (!is_cev) return xvadd(A, B);
    else return {A.x + B.x, A.y + B.y, A.z + B.z, A.w + B.w};
  }
  friend constexpr vector operator-(const vector& A, const vector& B) noexcept {
    if (!is_cev) return xvsub(A, B);
    else return {A.x - B.x, A.y - B.y, A.z - B.z, A.w - B.w};
  }
  friend constexpr vector operator*(const vector& A, const vector& B) noexcept {
    if (!is_cev) return xvmul(A, B);
    else return {A.x * B.x, A.y * B.y, A.z * B.z, A.w * B.w};
  }
  friend constexpr vector operator/(const vector& A, const vector& B) noexcept {
    if (!is_cev) return xvdiv(A, B);
    else return {A.x / B.x, A.y / B.y, A.z / B.z, A.w / B.w};
  }
  friend constexpr vector operator+(const vector& A, const fat4& B) noexcept {
    if (!is_cev) return xvadd(A, xv(B));
    else return {A.x + B, A.y + B, A.z + B, A.w + B};
  }
  friend constexpr vector operator-(const vector& A, const fat4& B) noexcept {
    if (!is_cev) return xvsub(A, xv(B));
    else return {A.x - B, A.y - B, A.z - B, A.w - B};
  }
  friend constexpr vector operator*(const vector& A, const fat4& B) noexcept {
    if (!is_cev) return xvmul(A, xv(B));
    else return {A.x * B, A.y * B, A.z * B, A.w * B};
  }
  friend constexpr vector operator/(const vector& A, const fat4& B) noexcept {
    if (!is_cev) return xvdiv(A, xv(B));
    else return {A.x / B, A.y / B, A.z / B, A.w / B};
  }
  friend constexpr vector operator+(const fat4& A, const vector& B) noexcept {
    if (!is_cev) return xvadd(xv(A), B);
    else return {A + B.x, A + B.y, A + B.z, A + B.w};
  }
  friend constexpr vector operator-(const fat4& A, const vector& B) noexcept {
    if (!is_cev) return xvsub(xv(A), B);
    else return {A - B.x, A - B.y, A - B.z, A - B.w};
  }
  friend constexpr vector operator*(const fat4& A, const vector& B) noexcept {
    if (!is_cev) return xvmul(xv(A), B);
    else return {A * B.x, A * B.y, A * B.z, A * B.w};
  }
  friend constexpr vector operator/(const fat4& A, const vector& B) noexcept {
    if (!is_cev) return xvdiv(xv(A), B);
    else return {A / B.x, A / B.y, A / B.z, A / B.w};
  }
  friend constexpr vector& operator+=(vector& A, const vector& B) noexcept { return A = A + B; }
  friend constexpr vector& operator-=(vector& A, const vector& B) noexcept { return A = A - B; }
  friend constexpr vector& operator*=(vector& A, const vector& B) noexcept { return A = A * B; }
  friend constexpr vector& operator/=(vector& A, const vector& B) noexcept { return A = A / B; }
  friend constexpr vector& operator+=(vector& A, const fat4& B) noexcept { return A = A + B; }
  friend constexpr vector& operator-=(vector& A, const fat4& B) noexcept { return A = A - B; }
  friend constexpr vector& operator*=(vector& A, const fat4& B) noexcept { return A = A * B; }
  friend constexpr vector& operator/=(vector& A, const fat4& B) noexcept { return A = A / B; }
};

/// 色の表現を目的に単精度浮動小数点数を４要素(`r`,`g`,`b`,`a`)を持つ構造体
/// @note `r`,`g`,`b`,`a`はそれぞれ赤、緑、青、アルファ値を表す
/// @note `r`,`g`,`b`,`a`はそれぞれ`0.0`から`1.0`の範囲で表現される
struct color {
  fat4 r{}, g{}, b{}, a = 1.f;
  constexpr color() noexcept = default;
  constexpr color(fat4 R, fat4 G, fat4 B, fat4 A = 1.f) noexcept : r(R), g(G), b(B), a(A) {}
  explicit constexpr color(nat4 xRRGGBB, fat4 A = 1.f) noexcept : color(from_code(xRRGGBB)) { a = A; }
  explicit constexpr operator nat4() const noexcept {
    if (is_cev) return (nat4)(r * 255.f) << 16 | (nat4)(g * 255.f) << 6 | (nat4)(b * 255.f) | (nat4)(a * 255.f) << 24;
    else return bitcast<nat4>(_mm_cvtsi128_si32(_mm_shuffle_epi8(_mm_cvtps_epi32(xvmul(xv(&r), xv_constant<255>)), xv_constant<0, 0, 0, 0x03020100>)));
  }
  explicit color(const xvector& Xv) noexcept { _mm_storeu_ps(&r, Xv); }
  operator xvector() const noexcept { return xv(&r); }
  friend constexpr bool operator==(const color& A, const color& B) noexcept {
    if (!is_cev) return xveq(A, B);
    else return A.r == B.r && A.g == B.g && A.b == B.b && A.a == B.a;
  }
  static const color black, dimgray, gray, darkgray, silver, lightgray, gainsboro, whitesmoke, white, snow, ghostwhite,
    floralwhite, linen, antiquewhite, papayawhip, blanchedalmond, bisque, moccasin, navajowhite, peachpuff, mistyrose,
    lavenderblush, seashell, oldlace, ivory, honeydew, mintcream, azure, aliceblue, lavender, lightsteelblue,
    lightslategray, slategray, steelblue, royalblue, midnightblue, navy, darkblue, mediumblue, blue, dodgerblue,
    cornflowerblue, deepskyblue, lightskyblue, skyblue, lightblue, powderblue, paleturquoise, lightcyan, cyan, aqua,
    turquoise, mediumturquoise, darkturquoise, lightseagreen, cadetblue, darkcyan, teal, darkslategray, darkgreen,
    green, forestgreen, seagreen, mediumseagreen, mediumaquamarine, darkseagreen, aquamarine, palegreen, lightgreen,
    springgreen, mediumspringgreen, lawngreen, chartreuse, greenyellow, lime, limegreen, yellowgreen, darkolivegreen,
    olivedrab, olive, darkkhaki, palegoldenrod, cornsilk, beige, lightyellow, lightgoldenrodyellow, lemonchiffon, wheat,
    burlywood, tan, khaki, yellow, gold, orange, sandybrown, darkorange, goldenrod, peru, darkgoldenrod, chocolate,
    sienna, saddlebrown, maroon, darkred, brown, firebrick, indianred, rosybrown, darksalmon, lightcoral, salmon,
    lightsalmon, coral, tomato, orangered, red, crimson, mediumvioletred, deeppink, hotpink, palevioletred, pink,
    lightpink, thistle, magenta, fuchsia, violet, plum, orchid, mediumorchid, darkorchid, darkviolet, darkmagenta,
    purple, indigo, darkslateblue, blueviolet, mediumpurple, slateblue, mediumslateblue, transparent, undefined;
private:
  static constexpr color from_code(const nat4& a) noexcept {
    if (is_cev) return color(fat4((0xff0000 & a) >> 16) / 255.f, fat4((0xff00 & a) >> 8) / 255.f, fat4(0xff & a) / 255.f, fat4((0xff000000 & a) >> 24) / 255.f);
    else return color(xvmul(xv_constant<1.0 / 255.0>, _mm_cvtepi32_ps(_mm_cvtepu8_epi32(xvbitcast(_mm_broadcast_ss((const fat4*)&a))))));
  }
};

inline constexpr color color::black{0x000000}, color::dimgray{0x696969}, color::gray{0x808080}, color::darkgray{0xa9a9a9}, color::silver{0xc0c0c0},
  color::lightgray{0xd3d3d3}, color::gainsboro{0xdcdcdc}, color::whitesmoke{0xf5f5f5}, color::white{0xffffff}, color::snow{0xfffafa},
  color::ghostwhite{0xf8f8ff}, color::floralwhite{0xfffaf0}, color::linen{0xfaf0e6}, color::antiquewhite{0xfaebd7}, color::papayawhip{0xffefd5},
  color::blanchedalmond{0xffebcd}, color::bisque{0xffe4c4}, color::moccasin{0xffe4b5}, color::navajowhite{0xffdead}, color::peachpuff{0xffdab9},
  color::mistyrose{0xffe4e1}, color::lavenderblush{0xfff0f5}, color::seashell{0xfff5ee}, color::oldlace{0xfdf5e6}, color::ivory{0xfffff0},
  color::honeydew{0xf0fff0}, color::mintcream{0xf5fffa}, color::azure{0xf0ffff}, color::aliceblue{0xf0f8ff}, color::lavender{0xe6e6fa},
  color::lightsteelblue{0xb0c4de}, color::lightslategray{0x778899}, color::slategray{0x708090}, color::steelblue{0x4682b4}, color::royalblue{0x4169e1},
  color::midnightblue{0x191970}, color::navy{0x000080}, color::darkblue{0x00008b}, color::mediumblue{0x0000cd}, color::blue{0x0000ff},
  color::dodgerblue{0x1e90ff}, color::cornflowerblue{0x6495ed}, color::deepskyblue{0x00bfff}, color::lightskyblue{0x87cefa}, color::skyblue{0x87ceeb},
  color::lightblue{0xadd8e6}, color::powderblue{0xb0e0e6}, color::paleturquoise{0xafeeee}, color::lightcyan{0xe0ffff}, color::cyan{0x00ffff},
  color::aqua{0x00ffff}, color::turquoise{0x40e0d0}, color::mediumturquoise{0x48d1cc}, color::darkturquoise{0x00ced1}, color::lightseagreen{0x20b2aa},
  color::cadetblue{0x5f9ea0}, color::darkcyan{0x008b8b}, color::teal{0x008080}, color::darkslategray{0x2f4f4f}, color::darkgreen{0x006400},
  color::green{0x008000}, color::forestgreen{0x228b22}, color::seagreen{0x2e8b57}, color::mediumseagreen{0x3cb371}, color::mediumaquamarine{0x66cdaa},
  color::darkseagreen{0x8fbc8f}, color::aquamarine{0x7fffd4}, color::palegreen{0x98fb98}, color::lightgreen{0x90ee90}, color::springgreen{0x00ff7f},
  color::mediumspringgreen{0x00fa9a}, color::lawngreen{0x7cfc00}, color::chartreuse{0x7fff00}, color::greenyellow{0xadff2f}, color::lime{0x00ff00},
  color::limegreen{0x32cd32}, color::yellowgreen{0x9acd32}, color::darkolivegreen{0x556b2f}, color::olivedrab{0x6b8e23}, color::olive{0x808000},
  color::darkkhaki{0xbdb76b}, color::palegoldenrod{0xeee8aa}, color::cornsilk{0xfff8dc}, color::beige{0xf5f5dc}, color::lightyellow{0xffffe0},
  color::lightgoldenrodyellow{0xfafad2}, color::lemonchiffon{0xfffacd}, color::wheat{0xf5deb3}, color::burlywood{0xdeb887}, color::tan{0xd2b48c},
  color::khaki{0xf0e68c}, color::yellow{0xffff00}, color::gold{0xffd700}, color::orange{0xffa500}, color::sandybrown{0xf4a460}, color::darkorange{0xff8c00},
  color::goldenrod{0xdaa520}, color::peru{0xcd853f}, color::darkgoldenrod{0xb8860b}, color::chocolate{0xd2691e}, color::sienna{0xa0522d},
  color::saddlebrown{0x8b4513}, color::maroon{0x800000}, color::darkred{0x8b0000}, color::brown{0xa52a2a}, color::firebrick{0xb22222},
  color::indianred{0xcd5c5c}, color::rosybrown{0xbc8f8f}, color::darksalmon{0xe9967a}, color::lightcoral{0xf08080}, color::salmon{0xfa8072},
  color::lightsalmon{0xffa07a}, color::coral{0xff7f50}, color::tomato{0xff6347}, color::orangered{0xff4500}, color::red{0xff0000}, color::crimson{0xdc143c},
  color::mediumvioletred{0xc71585}, color::deeppink{0xff1493}, color::hotpink{0xff69b4}, color::palevioletred{0xdb7093}, color::pink{0xffc0cb},
  color::lightpink{0xffb6c1}, color::thistle{0xd8bfd8}, color::magenta{0xff00ff}, color::fuchsia{0xff00ff}, color::violet{0xee82ee}, color::plum{0xdda0dd},
  color::orchid{0xda70d6}, color::mediumorchid{0xba55d3}, color::darkorchid{0x9932cc}, color::darkviolet{0x9400d3}, color::darkmagenta{0x8b008b},
  color::purple{0x800080}, color::indigo{0x4b0082}, color::darkslateblue{0x483d8b}, color::blueviolet{0x8a2be2}, color::mediumpurple{0x9370db},
  color::slateblue{0x6a5acd}, color::mediumslateblue{0x7b68ee}, color::undefined = color(0x0, 0.f);
}

inline constexpr yw::color ywcolor{0x081020};

namespace std {
template<> struct tuple_size<yw::vector> : public integral_constant<size_t, 4> {};
template<size_t I> requires(I < 4) struct tuple_element<I, yw::vector> : public type_identity<float> {};
template<> struct formatter<yw::vector> : public formatter<string> {
  auto format(const yw::vector& A, format_context& Ctx) const { return formatter<string>::format(std::format("[{},{},{},{}]", A.x, A.y, A.z, A.w), Ctx); }
};
template<> struct formatter<yw::color> : formatter<string> {
  auto format(const yw::color& C, format_context& Ctx) const {
    return formatter<string>::format(std::format("[{},{},{},{}]", C.r, C.g, C.b, C.a), Ctx);
  }
};
template<> struct formatter<yw::xvector> : public formatter<string> {
  auto format(const yw::xvector& A, format_context& C) const {
    return formatter<string>::format(std::format("{}", yw::vector(A)), C);
  }
};
template<> struct formatter<yw::xmatrix> : public formatter<string> {
  auto format(const yw::xmatrix& A, format_context& C) const {
    return formatter<string>::format(std::format("[{},{},{},{}]", A[0], A[1], A[2], A[3]), C);
  }
};
}
