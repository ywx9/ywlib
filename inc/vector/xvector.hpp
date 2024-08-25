/// \file vector/xvector.hpp

#pragma once

#include "../core.hpp"
#include "sse.hpp"
#include "avx.hpp"

namespace _ {

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
  constexpr auto f = [](auto a) { return a < 0 || 8 <= a; };
  if constexpr ((f(X) || X < 4) && (f(Y) || Y < 4) && (f(Z) || Z < 4) && (f(W) || W < 4))
    return _xvpermute<X, Y, Z, W>(a);
  else if constexpr ((f(X) || X >= 4) && (f(Y) || Y >= 4) && (f(Z) || Z >= 4) && (f(W) || W >= 4))
    return _xvpermute<X - 4, Y - 4, Z - 4, W - 4>(b);
  else if constexpr ((f(X) || !(X & 3 ^ 0)) && (f(Y) || !(Y & 3 ^ 1)) && (f(Z) || !(Z & 3 ^ 2)) && (f(W) || !(W & 3 ^ 3))) {
    constexpr auto x = f(X) ? false : X < 4, y = f(Y) ? false : Y < 4, z = f(Z) ? false : Z < 4, w = f(W) ? false : W < 4;
    return _mm_blend_ps(a, b, x | y << 1 | z << 2 | w << 3);
  } else if constexpr ((f(X) || X < 4) && (f(Y) || Y < 4) && (f(Z) || Z >= 4) && (f(W) || W >= 4)) {
    constexpr auto x = f(X) ? 0 : X, y = f(Y) ? 1 : Y, z = f(Z) ? 2 : Z - 4, w = f(W) ? 3 : W - 4;
    return _mm_shuffle_ps(a, b, x | y << 2 | z << 4 | w << 6);
  } else if constexpr ((f(X) || X >= 4) && (f(Y) || Y >= 4) && (f(Z) || Z < 4) && (f(W) || W < 4)) {
    constexpr auto x = f(X) ? 0 : X - 4, y = f(Y) ? 1 : Y - 4, z = f(Z) ? 2 : Z, w = f(W) ? 3 : W;
    return _mm_shuffle_ps(b, a, x | y << 2 | z << 4 | w << 6);
  } else if constexpr ((f(X) || X == 0) + (f(Y) || Y == 1) + (f(Z) || Z == 2) + (f(W) || W == 3) == 3) {
    constexpr auto x = f(X) ? 0 : X, y = f(Y) ? 1 : Y, z = f(Z) ? 2 : Z, w = f(W) ? 3 : W;
    constexpr nat i = inspects<ge(x, 4), ge(y, 4), ge(z, 4), ge(w, 4)>;
    return _mm_insert_ps(a, b, int((select_value<i, x, y, z, w> - 4) << 6 | i << 4));
  } else if constexpr ((f(X) || X == 4) + (f(Y) || Y == 5) + (f(Z) || Z == 6) + (f(W) || W == 7) == 3) {
    constexpr auto x = f(X) ? 4 : X, y = f(Y) ? 5 : Y, z = f(Z) ? 6 : Z, w = f(W) ? 7 : W;
    constexpr nat i = inspects<lt(x, 4), lt(y, 4), lt(z, 4), lt(w, 4)>;
    return _mm_insert_ps(b, a, int(select_value<i, x, y, z, w>) << 6 | i << 4);
  } else if constexpr ((X < 4 || X == 4) && (Y < 4 || Y == 5) && (Z < 4 || Z == 6) && (w < 4 || w == 7))
    return xvblend<x == 4, y == 5, z == 6, w == 7>(_::_xvpermute1<x, y, z, w>(a), b);
  else if constexpr ((x < 0 || x == 0 || 4 <= x) && (y < 0 || y == 1 || 4 <= y) && (z < 0 || z == 2 || 4 <= z) && (w < 0 || w == 3 || 4 <= w))
    return xvblend<x == 0, y == 1, z == 2, w == 3>(_::_xvpermute1<x - 4, y - 4, z - 4, w - 4>(b), a);
  else if constexpr ((0 <= x && x < 4) + (0 <= y && y < 4) + (0 <= z && z < 4) + (0 <= w && w < 4) == 1) {
    constexpr nat i = inspects<(0 <= x && x < 4), (0 <= y && y < 4), (0 <= z && z < 4), (0 <= w && w < 4)>;
    return _mm_insert_ps(_::_xvpermute1<x - 4, y - 4, z - 4, w - 4>(b), a, i << 4 | select_value<i, x, y, z, w> << 6);
  } else if constexpr ((4 <= x) + (4 <= y) + (4 <= z) + (4 <= w) == 1) {
    constexpr nat i = inspects<(4 <= x), (4 <= y), (4 <= z), (4 <= w)>;
    return _mm_insert_ps(_::_xvpermute1<x, y, z, w>(a), b, i << 4 | (select_value<i, x, y, z, w> - 4) << 6);
  } else return xvblend<lt(x, 4), lt(y, 4), lt(z, 4), lt(w, 4)>(_::_xvpermute1<x - 4, y - 4, z - 4, w - 4>(b), _::_xvpermute1<x, y, z, w>(a));
  return xvload(-1.f, -1.f, -1.f, -1.f);
}

} ////////////////////////////////////////////////////////////////////////////// namespace _

export namespace yw {


/// extended vector type
using xvector = __m128;

/// extended matrix type
using xmatrix = array<xvector, 4>;


/// constant extended vector object
template<value X, value Y = X, value Z = Y, value W = Z>
inline constexpr caster xvconstant{
  []() noexcept -> const xvector& {
    static const xvector v = _mm_set_ps(W, Z, Y, X);
    return v;
  },
  []() noexcept -> const __m128i& {
    static const __m128i v = _mm_set_epi32(W, Z, Y, X);
    return v;
  }
};

/// specialization for zero vector
template<>
inline constexpr caster xvconstant<0, 0, 0, 0>{
  []() noexcept -> xvector {
    return _mm_setzero_ps();
  },
  []() noexcept -> __m128i {
    return _mm_setzero_si128();
  }
};

/// constant extended vector; {0, 0, 0, 0}
inline constexpr auto xvzero = xvconstant<0>;

/// constant extended vector; {-0, -0, -0, -0}
inline constexpr auto xvnegzero = xvconstant<-0.>;

/// constant extended vector; {1, 1, 1, 1}
inline constexpr auto xvone = xvconstant<1>;

/// constant extended vector; {-1, -1, -1, -1}
inline constexpr auto xvnegone = xvconstant<-1>;

/// constant extended vector; {1, 0, 0, 0}
inline constexpr auto xvx = xvconstant<1, 0>;

/// constant extended vector; {0, 1, 0, 0}
inline constexpr auto xvy = xvconstant<0, 1, 0>;

/// constant extended vector; {0, 0, 1, 0}
inline constexpr auto xvz = xvconstant<0, 0, 1, 0>;

/// constant extended vector; {0, 0, 0, 1}
inline constexpr auto xvw = xvconstant<0, 0, 0, 1>;

/// constant extended vector; {-1, 0, 0, 0}
inline constexpr auto xvnegx = xvconstant<-1, 0>;

/// constant extended vector; {0, -1, 0, 0}
inline constexpr auto xvnegy = xvconstant<0, -1, 0>;

/// constant extended vector; {0, 0, -1, 0}
inline constexpr auto xvnegz = xvconstant<0, 0, -1, 0>;

/// constant extended vector; {0, 0, 0, -1}
inline constexpr auto xvnegw = xvconstant<0, 0, 0, -1>;


/// loads an extended vector from `ptr`
inline xvector xvload(const fat4* ptr) noexcept { return _mm_loadu_ps(ptr); }

/// fills an extended vector with `val`
inline xvector xvfill(const fat4 val) noexcept { return _mm_set1_ps(val); }

/// sets an extended vector with four values
inline xvector xvset(const fat4 x, const fat4 y, const fat4 z, const fat4 w)
  noexcept { return _mm_set_ps(w, z, y, x); }

/// stores an extended vector to `ptr`
inline void xvstore(fat4* ptr, const xvector& v) noexcept { _mm_storeu_ps(ptr, v); }

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
template<nat X, nat Y, nat Z, nat W>
requires ((X | Y | Z | W) < 4)
xvector xvpermute(const xvector& a) noexcept {
  if constexpr (X == 0 && Y == 1 && Z == 2 && W == 3) return a;
  else if constexpr (X == 0 && Y == 0 && Z == 0 && W == 0)
    return _mm_broadcastss_ps(a);
  else if constexpr (X == 0 && Y == 0 && Z == 2 && W == 2)
    return _mm_moveldup_ps(a);
  else if constexpr (X == 1 && Y == 1 && Z == 3 && W == 3)
    return _mm_movehdup_ps(a);
  else return _mm_permute_ps(a, X | Y << 2 | Z << 4 | W << 6);
}

/// permutes two extended vectors
template<nat X, nat Y, nat Z, nat W>
requires ((X | Y | Z | W) < 8)
xvector xvpermute(const xvector& a, const xvector& b) noexcept {
  if constexpr ((X | Y | Z | W) < 4) return xvpermute<X, Y, Z, W>(a);
  else if constexpr ((X & Y & Z & W) >= 4)
    return xvpermute<X - 4, Y - 4, Z - 4, W - 4>(b);
  else if constexpr (((X & 3) | (Y & 3 ^ 1) | (Z & 3 ^ 2) | (W & 3 ^ 3)) == 0)
    return xvblend<lt(X, 4), lt(Y, 4), lt(Z, 4), lt(W, 4)>(b, a);
  else if constexpr ((X | Y) < 4 && (Z & W) >= 4)
    return _mm_shuffle_ps(a, b, X | Y << 2 | (Z & 3) << 4 | (W & 3) << 6);
  else if constexpr ((X & Y) >= 4 && (Z | W) < 4)
    return _mm_shuffle_ps(b, a, (X & 3) | (Y & 3) << 2 | Z << 4 | W << 6);
  else if constexpr (X == 0 && Y == 4 && X == 1 && Y == 5)
    return _mm_unpacklo_ps(a, b);
  else if constexpr (X == 4 && Y == 0 && X == 5 && Y == 1)
    return _mm_unpacklo_ps(b, a);
  else if constexpr (X == 2 && Y == 6 && X == 3 && Y == 7)
    return _mm_unpackhi_ps(a, b);
  else if constexpr (X == 6 && Y == 2 && X == 7 && Y == 3)
    return _mm_unpackhi_ps(b, a);
  else if constexpr ((X == 0) + (Y == 1) + (Z == 2) + (W == 3) == 3) {
    constexpr nat i = inspects<X != 0, Y != 1, Z != 2, W != 3>;
    constexpr nat j = select_value<i, X, Y, Z, W> - 4;
    return _mm_insert_ps(a, b, int(j << 6 | i << 4));
  } else if constexpr ((X == 4) + (Y == 5) + (Z == 6) + (W == 7) == 3) {
    constexpr nat i = inspects<X != 4, Y != 5, Z != 6, W != 7>;
    constexpr nat j = select_value<i, X, Y, Z, W>;
    return _mm_insert_ps(b, a, int(j << 6 | i << 4));
  } else if constexpr ((X < 4 || X == 4) && (Y < 4 || Y == 5) && (Z < 4 || Z == 6) && (W < 4 || W == 7))
    return xvblend<X == 4, Y == 5, Z == 6, W == 7>(xvpermute<X & 3, Y & 3, Z & 3, W & 3>(a), b);
  else if constexpr ((X >= 4 || X == 0) && (Y >= 4 || Y == 1) && (Z >= 4 || Z == 2) && (W >= 4 || W == 3))
    return xvblend<X == 0, Y == 1, Z == 2, W == 3>(xvpermute<X & 3, Y & 3, Z & 3, W & 3>(b), a);
  else if constexpr ((X < 4) + (Y < 4) + (Z < 4) + (W < 4) == 1) {
    constexpr nat i = inspects<X < 4, Y < 4, Z < 4, W < 4>;
    constexpr nat j = select_value<i, X, Y, Z, W>;
    return _mm_insert_ps(xvpermute<X & 3, Y & 3, Z & 3, W & 3>(b), a, int(j << 6 | i << 4));
  } else if constexpr ((X >= 4) + (Y >= 4) + (Z >= 4) + (W >= 4) == 1) {
    constexpr nat i = inspects<X >= 4, Y >= 4, Z >= 4, W >= 4>;
    constexpr nat j = select_value<i, X, Y, Z, W> - 4;
    return _mm_insert_ps(xvpermute<X & 3, Y & 3, Z & 3, W & 3>(a), b, int(j << 6 | i << 4));
  } else return xvblend<X < 4, Y < 4, Z < 4, W < 4>(
    xvpermute<X & 3, Y & 3, Z & 3, W & 3>(a), xvpermute<X & 3, Y & 3, Z & 3, W & 3>(b));
}

} ////////////////////////////////////////////////////////////////////////////// namespace yw
