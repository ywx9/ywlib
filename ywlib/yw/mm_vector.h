#pragma once
#include <yw/vector.h>

namespace yw {

/// MARK: mm_vector

using mm_vector = __m128;

/// MARK: mm_get

template<size_t I> requires(lt(I, 4)) float mm_get(__m128 m) noexcept {
  if constexpr (I == 0) return _mm_cvtss_f32(m);
  else return std::bit_cast<float>(_mm_extract_ps(m, int(I)));
}

/// MARK: mm_set

inline __m128 mm_set1(float v) noexcept { return _mm_set1_ps(v); }

inline __m128 mm_set(float x, float y, float z = 0.0f, float w = 0.0f) noexcept { return _mm_set_ps(w, z, y, x); }

template<size_t I> requires(lt(I, 4)) __m128 mm_set(__m128 m, float v) noexcept {
  return _mm_castsi128_ps(_mm_insert_epi32(_mm_castps_si128(m), std::bit_cast<int>(v), int(I)));
}

template<size_t I> requires(lt(I, 4)) __m128 mm_set(float v) noexcept {
  return _mm_castsi128_ps(_mm_insert_epi32(_mm_setzero_si128(), std::bit_cast<int>(v), int(I)));
}

/// MARK: mm_blend

template<size_t Mask> requires(lt(Mask, 16)) __m128 mm_blend(__m128 a, __m128 b) noexcept {
  if constexpr (Mask == 0b1111) return b;
  else if constexpr (Mask == 0b0000) return a;
  else return _mm_blend_ps(a, b, Mask);
}

template<bool X, bool Y, bool Z, bool W> __m128 mm_blend(__m128 a, __m128 b) noexcept {
  if constexpr (X && Y && Z && W) return b;
  else if constexpr (!(X || Y || Z || W)) return a;
  else return mm_blend<size_t(X + Y * 2 + Z * 4 + W * 8)>(a, b);
}

/// MARK: mm_setzero

template<size_t Zero> requires(lt(Zero, 16)) __m128 mm_setzero(__m128 m) noexcept {
  if constexpr (Zero == 0b1111) return _mm_setzero_ps();
  else return mm_blend<Zero>(m, _mm_setzero_ps());
}

template<bool X, bool Y, bool Z, bool W> __m128 mm_setzero(__m128 m) noexcept {
  if constexpr (X && Y && Z && W) return _mm_setzero_ps();
  else return mm_setzero<size_t(X + Y * 2 + Z * 4 + W * 8)>(m);
}

/// MARK: mm_insert

template<size_t From, size_t To, size_t Zero = 0> requires(lt((From | To), 4) && lt(Zero, 16))
__m128 mm_insert(__m128 extracted, __m128 inserted) noexcept {
  if constexpr (Zero == 15) return _mm_setzero_ps();
  else return _mm_insert_ps(inserted, extracted, int(From << 6 | To << 4 | Zero));
}

/// MARK: mm_permute

template<int X, int Y, int Z, int W> __m128 mm_permute(__m128 m) noexcept {
  constexpr bool bx = (X < 0 || 3 < X), by = (Y < 0 || 3 < Y), bz = (Z < 0 || 3 < Z), bw = (W < 0 || 3 < W);
  if constexpr ((bx || X == 0) && (by || Y == 1) && (bz || Z == 2) && (bw || W == 3)) return m;
  else if constexpr ((bx || X == 0) && (by || Y == 0) && (bz || Z == 2) && (bw || W == 2)) return _mm_moveldup_ps(m);
  else if constexpr ((bx || X == 1) && (by || Y == 1) && (bz || Z == 3) && (bw || W == 3)) return _mm_movehdup_ps(m);
  else return _mm_permute_ps(m, (bx ? 0 : X) + (by ? 1 : Y) * 4 + (bz ? 2 : Z) * 16 + (bw ? 3 : W) * 64);
}

template<size_t Flag> requires(lt(Flag, 256)) __m128 mm_permute(__m128 m) noexcept {
  return mm_permute<int(Flag & 3), int((Flag >> 2) & 3), int((Flag >> 4) & 3), int((Flag >> 6) & 3)>(m);
}

template<int X, int Y, int Z, int W> __m128 mm_permute(__m128 a, __m128 b) noexcept {
  constexpr auto x = X - 4, y = Y - 4, z = Z - 4, w = W - 4;
  constexpr bool bx = (X < 0 || 7 < X), by = (Y < 0 || 7 < Y), bz = (Z < 0 || 7 < Z), bw = (W < 0 || 7 < W);
  constexpr int match_a = (bx || (X < 4)) + (by || (Y < 4)) + (bz || (Z < 4)) + (bw || (W < 4));
  constexpr int match_b = (bx || (3 < X)) + (by || (3 < Y)) + (bz || (3 < Z)) + (bw || (3 < W));
  if constexpr (match_a == 4) return mm_permute<X, Y, Z, W>(a);
  else if constexpr (match_b == 4) return mm_permute<x, y, z, w>(b);
  else if constexpr ((bx || (X & 3) == 0) + (by || (Y & 3) == 1) + (bz || (Z & 3) == 2) + (bw || (W & 3) == 3) == 4)
    return mm_blend<lt(X, 4), lt(Y, 4), lt(Z, 4), lt(W, 4)>(b, a);
  else if constexpr ((bx || X == 0) + (by || Y == 4) + (bz || Z == 1) + (bw || W == 5) == 4)
    return _mm_unpacklo_ps(a, b);
  else if constexpr ((bx || X == 4) + (by || Y == 0) + (bz || Z == 5) + (bw || W == 1) == 4)
    return _mm_unpacklo_ps(b, a);
  else if constexpr ((bx || X == 2) + (by || Y == 6) + (bz || Z == 3) + (bw || W == 7) == 4)
    return _mm_unpackhi_ps(a, b);
  else if constexpr ((bx || X == 6) + (by || Y == 2) + (bz || Z == 7) + (bw || W == 3) == 4)
    return _mm_unpackhi_ps(b, a);
  else if constexpr ((bx || X < 4) + (by || Y < 4) + (bz || 3 < Z) + (bw || 3 < W) == 4)
    return _mm_shuffle_ps(a, b, (bx ? 0 : X) + (by ? 0 : Y) * 4 + (bz ? 0 : z) * 16 + (bw ? 0 : w) * 64);
  else if constexpr ((bx || 3 < X) + (by || 3 < Y) + (bz || Z < 4) + (bw || W < 4) == 4)
    return _mm_shuffle_ps(b, a, (bx ? 0 : x) + (by ? 0 : y) * 4 + (bz ? 0 : Z) * 16 + (bw ? 0 : W) * 64);
  else if constexpr ((bx || X == 1) + (by || Y == 2) + (bz || Z == 4) + (bw || W == 5) == 4)
    return _mm_castsi128_ps(_mm_alignr_epi8(_mm_castps_si128(b), _mm_castps_si128(a), 4));
  else if constexpr ((bx || X == 3) + (by || Y == 0) + (bz || Z == 6) + (bw || W == 7) == 4)
    return _mm_castsi128_ps(_mm_alignr_epi8(_mm_castps_si128(b), _mm_castps_si128(a), 12));
  else if constexpr ((bx || X == 5) + (by || Y == 6) + (bz || Z == 7) + (bw || W == 0) == 4)
    return _mm_castsi128_ps(_mm_alignr_epi8(_mm_castps_si128(a), _mm_castps_si128(b), 4));
  else if constexpr ((bx || X == 7) + (by || Y == 0) + (bz || Z == 1) + (bw || W == 2) == 4)
    return _mm_castsi128_ps(_mm_alignr_epi8(_mm_castps_si128(a), _mm_castps_si128(b), 12));
  else if constexpr ((bx || X == 0) + (by || Y == 1) + (bz || Z == 2) + (bw || W == 3) == 3) {
    constexpr size_t i = inspect<!(bx || X == 0), !(by || Y == 1), !(bz || Z == 2), !(bw || W == 3)>;
    return _mm_insert_ps(a, b, int((select_value<i, X, Y, Z, W> - 4) << 6 | i << 4));
  } else if constexpr ((bx || X == 4) + (by || Y == 5) + (bz || Z == 6) + (bw || W == 7) == 3) {
    constexpr size_t i = inspect<!(bx || X == 4), !(by || Y == 5), !(bz || Z == 6), !(bw || W == 7)>;
    return _mm_insert_ps(b, a, int(select_value<i, X, Y, Z, W> << 6 | i << 4));
  } else if constexpr ((bx || X < 4 || X == 4) && (by || Y < 4 || Y == 5) && //
                       (bz || Z < 4 || Z == 6) && (bw || W < 4 || W == 7))
    return mm_blend<X == 4, Y == 5, Z == 6, W == 7>(mm_permute<X, Y, Z, W>(a), b);
  else if constexpr ((bx || 3 < X || X == 0) && (by || 3 < Y || Y == 1) && //
                     (bz || 3 < Z || Z == 2) && (bw || 3 < W || W == 3))
    return mm_blend<X == 0, Y == 1, Z == 2, W == 3>(mm_permute<x, y, z, w>(b), a);
  else if constexpr (match_a == 3) {
    constexpr size_t i = inspect<!(bx || (X < 4)), !(by || (Y < 4)), !(bz || (Z < 4)), !(bw || (W < 4))>;
    return _mm_insert_ps((mm_permute<X, Y, Z, W>(a)), b, int((select_value<i, X, Y, Z, W> - 4) << 6 | i << 4));
  } else if constexpr (match_b == 3) {
    constexpr size_t i = inspect<!(bx || (3 < X)), !(by || (3 < Y)), !(bz || (3 < Z)), !(bw || (3 < W))>;
    return _mm_insert_ps((mm_permute<x, y, z, w>(b)), a, int(select_value<i, X, Y, Z, W> << 6 | i << 4));
  } else { // match_a == 2 && match_b == 2
    constexpr auto sxi = inspect<(X < 4), (Y < 4), (Z < 4)>;
    constexpr auto syi = 3 - inspect<(W < 4), (Z < 4), (Y < 4)>;
    constexpr auto szi = inspect<(X > 3), (Y > 3), (Z > 3)>;
    constexpr auto swi = 3 - inspect<(W > 3), (Z > 3), (Y > 3)>;
    constexpr auto sxj = inspect<sxi == 0, syi == 0, szi == 0, swi == 0>;
    constexpr auto syj = inspect<sxi == 1, syi == 1, szi == 1, swi == 1>;
    constexpr auto szj = inspect<sxi == 2, syi == 2, szi == 2, swi == 2>;
    constexpr auto swj = inspect<sxi == 3, syi == 3, szi == 3, swi == 3>;
    return mm_permute<sxj, syj, szj, swj>(mm_permute<
      select_value<sxi, X, Y, Z, W>, select_value<syi, X, Y, Z, W>, select_value<szi, X, Y, Z, W>,
      select_value<swi, X, Y, Z, W>>(a, b));
  }
}

/// MARK: mm_abs

inline __m128 mm_abs(__m128 m) noexcept { return _mm_andnot_ps(_mm_set1_ps(-0.f), m); }

/// MARK: mm_neg

inline __m128 mm_neg(__m128 m) noexcept { return _mm_xor_ps(_mm_set1_ps(-0.f), m); }

/// MARK: mm_dot

template<size_t N, size_t Zero = 0> requires(le(N, 4) && lt(Zero, 16)) __m128 mm_dot(__m128 a, __m128 b) noexcept {
  if constexpr (N == 0) return mm_setzero<Zero>(mm_set1(1.0f));
  else if constexpr (N == 1) return mm_setzero<Zero>(mm_permute<0, 0, 0, 0>(_mm_mul_ps(a, b)));
  else if constexpr (N == 2) return _mm_dp_ps(a, b, int((0b0011 << 4) | (~Zero & 0b1111)));
  else if constexpr (N == 3) return _mm_dp_ps(a, b, int((0b0111 << 4) | (~Zero & 0b1111)));
  else return _mm_dp_ps(a, b, int((0b1111 << 4) | (~Zero & 0b1111)));
}

/// MARK: dot

template<size_t N> requires(le(N, 4)) float dot(__m128 a, __m128 b) noexcept { return mm_get<0>(mm_dot<N>(a, b)); }
inline float dot(__m128 a, __m128 b) noexcept { return mm_get<0>(mm_dot<4>(a, b)); }

/// MARK: mm_matrix

struct mm_matrix {
  union {
    mm_vector rows[4];
    struct {
      mm_vector x, y, z, w;
    };
  };

  mm_vector& operator[](size_t i) { return rows[i]; }
  const mm_vector& operator[](size_t i) const { return rows[i]; }

  void store(vector4<float4>& Out) const noexcept {
    _mm_storeu_ps(Out.x.data(), x);
    _mm_storeu_ps(Out.y.data(), y);
    _mm_storeu_ps(Out.z.data(), z);
    _mm_storeu_ps(Out.w.data(), w);
  }

  void t(mm_matrix& Out) const {
    Out[0] = mm_permute<2, 3, 6, 7>(x, y);
    Out[1] = mm_permute<2, 3, 6, 7>(z, w);
    Out[2] = mm_permute<0, 2, 4, 6>(Out[0], Out[1]);
    Out[3] = mm_permute<1, 3, 5, 7>(Out[0], Out[1]);
    auto a = mm_permute<0, 1, 4, 5>(x, y);
    Out[0] = mm_permute<0, 1, 4, 5>(z, w);
    Out[1] = mm_permute<1, 3, 5, 7>(a, Out[0]);
    Out[0] = mm_permute<0, 2, 4, 6>(a, Out[0]);
  }

  void t_inplace() {
    auto a = mm_permute<0, 1, 4, 5>(x, y);
    auto b = mm_permute<0, 1, 4, 5>(z, w);
    auto c = mm_permute<2, 3, 6, 7>(a, b);
    x = mm_permute<0, 2, 0, 2>(a, b);
    y = mm_permute<1, 3, 1, 3>(a, b);
    a = mm_permute<2, 3, 6, 7>(z, w);
    z = mm_permute<0, 2, 0, 2>(c, a);
    w = mm_permute<1, 3, 1, 3>(c, a);
  }
};

/// MARK: rotation_matrix

inline void rotation_matrix(const mm_vector Cos, const mm_vector Sin, mm_matrix& Out) {
  Out.y = mm_permute<0, 2, 4, 6>(Cos, Sin);
  Out.x = mm_permute<3, 0, 1, 2>(Out.y);
  Out.w = _mm_mul_ps(Out.y, Out.x);
  Out.y = _mm_mul_ps(mm_permute<1, 1, 1, 1>(Sin), Out.w);
  Out.z = _mm_addsub_ps(mm_permute<2, 3, 0, 1>(Out.y), Out.w);
  Out.w = _mm_mul_ps(mm_permute<1, 1, 1, 1>(Cos), Out.x);
  Out.x = mm_insert<2, 0, 0b1000>(Out.w, mm_permute<-1, 0, 3, -1>(Out.z));
  Out.y = mm_insert<0, 0, 0b1000>(Out.w, Out.z);
  Out.z = mm_insert<1, 0, 0b1000>(mm_neg(Sin), mm_permute<-1, 3, 1, -1>(Out.w));
  Out.w = mm_set<3>(1.0f);
}

/// MARK: inverse_rotation_matrix

inline void inverse_rotation_matrix(const mm_vector Cos, const mm_vector Sin, mm_matrix& Out) {
  Out.y = mm_permute<0, 2, 4, 6>(Cos, Sin);
  Out.z = mm_permute<3, 0, 1, 2>(Out.y);
  Out.w = _mm_mul_ps(Out.y, Out.z);
  Out.y = _mm_mul_ps(mm_permute<1, 1, 1, 1>(Sin), Out.w);
  Out.x = _mm_addsub_ps(mm_permute<2, 3, 0, 1>(Out.y), Out.w);
  Out.w = _mm_mul_ps(mm_permute<1, 1, 1, 1>(Cos), Out.z);
  Out.z = mm_insert<1, 2, 0b1000>(Out.w, mm_permute<3, 2, -1, -1>(Out.x));
  Out.y = mm_insert<3, 2, 0b1000>(Out.w, Out.x);
  Out.x = mm_insert<1, 2, 0b1000>(mm_neg(Sin), mm_permute<2, 0, -1, -1>(Out.w));
  Out.w = mm_set<3>(1.0f);
}

/// MARK: transform

inline mm_vector transform(const mm_matrix& M, const mm_vector V) {
  auto t0 = mm_permute<0, 1, 4, 5>(M.x, M.y);
  auto t1 = mm_permute<0, 1, 4, 5>(M.z, M.w);
  auto out = _mm_mul_ps(mm_permute<0, 2, 4, 6>(t0, t1), mm_permute<0, 0, 0, 0>(V));
  out = _mm_add_ps(out, _mm_mul_ps(mm_permute<1, 3, 5, 7>(t0, t1), mm_permute<1, 1, 1, 1>(V)));
  t0 = mm_permute<2, 3, 6, 7>(M.x, M.y);
  t1 = mm_permute<2, 3, 6, 7>(M.z, M.w);
  out = _mm_add_ps(out, _mm_mul_ps(mm_permute<0, 2, 4, 6>(t0, t1), mm_permute<2, 2, 2, 2>(V)));
  return _mm_add_ps(out, _mm_mul_ps(mm_permute<1, 3, 5, 7>(t0, t1), mm_permute<3, 3, 3, 3>(V)));
}

/// MARK: dot

inline mm_vector dot(const mm_vector V, const mm_matrix& M) {
  auto out = _mm_mul_ps(mm_permute<0, 0, 0, 0>(V), M.x);
  out = _mm_add_ps(out, _mm_mul_ps(mm_permute<1, 1, 1, 1>(V), M.y));
  out = _mm_add_ps(out, _mm_mul_ps(mm_permute<2, 2, 2, 2>(V), M.z));
  return _mm_add_ps(out, _mm_mul_ps(mm_permute<3, 3, 3, 3>(V), M.w));
}

inline void dot(const mm_matrix& M, const mm_matrix& N, mm_matrix& Out) {
  Out.x = dot(M.x, N);
  Out.y = dot(M.y, N);
  Out.z = dot(M.z, N);
  Out.w = dot(M.w, N);
}
} // namespace yw
