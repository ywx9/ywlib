/// \file vector/avx.hpp

#pragma once

#include "sse.hpp"

export namespace yw::mm {

/// \return `{*ptr, ...}`
inline m128 broadcast(const float* ptr)
  noexcept { return _mm_broadcast_ss(ptr); }

/// \return `{a.x, a.x, a.x, a.x}`
inline m128 broadcast(const m128& a)
  noexcept { return _mm_broadcastss_ps(a); }

/// \return `{a.x * b.x + c.x, ...}`
inline m128 fmadd(const m128& a, const m128& b, const m128& c)
  noexcept { return _mm_fmadd_ps(a, b, c); }

/// \return `{a.x * b.x + c.x, a.y, a.z, a.w}`
inline m128 fmadd_single(const m128& a, const m128& b, const m128& c)
  noexcept { return _mm_fmadd_ss(a, b, c); }

/// \return `{a.x * b.x - c.x, a.x * b.y + c.y, a.x * b.z - c.z, a.x * b.w + c.w}`
inline m128 fmaddsub(const m128& a, const m128& b, const m128& c)
  noexcept { return _mm_fmsubadd_ps(a, b, c); }

/// \return `{a.x * b.x - c.x, ...}`
inline m128 fmsub(const m128& a, const m128& b, const m128& c)
  noexcept { return _mm_fmsub_ps(a, b, c); }

/// \return `{a.x * b.x - c.x, a.y, a.z, a.w}`
inline m128 fmsub_single(const m128& a, const m128& b, const m128& c)
  noexcept { return _mm_fmsub_ss(a, b, c); }

/// \return `{a.x * b.x + c.x, a.x * b.y - c.y, a.x * b.z + c.z, a.x * b.w - c.w}`
inline m128 fmsubadd(const m128& a, const m128& b, const m128& c)
  noexcept { return _mm_fmsubadd_ps(a, b, c); }

/// \return `{-a.x * b.x + c.x, ...}`
inline m128 fnmadd(const m128& a, const m128& b, const m128& c)
  noexcept { return _mm_fnmadd_ps(a, b, c); }

/// \return `{-a.x * b.x + c.x, a.y, a.z, a.w}`
inline m128 fnmadd_single(const m128& a, const m128& b, const m128& c)
  noexcept { return _mm_fnmadd_ss(a, b, c); }

/// \return `{-a.x * b.x - c.x, ...}`
inline m128 fnmsub(const m128& a, const m128& b, const m128& c)
  noexcept { return _mm_fnmsub_ps(a, b, c); }

/// \return `{-a.x * b.x - c.x, a.y, a.z, a.w}`
inline m128 fnmsub_single(const m128& a, const m128& b, const m128& c)
  noexcept { return _mm_fnmsub_ss(a, b, c); }

/// \return `{extract<imm8[0:1]>(a), extract<imm8[2:3]>(a), extract<imm8[4:5]>(a), extract<imm8[6:7]>(a)}`
template<int imm8> inline m128 permute(const m128& a)
  noexcept { return _mm_permute_ps(a, imm8); }

/// \return `t.x == 0 && t.y == 0 && t.z == 0 && t.w == 0` where `t = andnot(a, b)`
inline bool testc(const m128& a, const m128& b)
  noexcept { return _mm_testc_ps(a, b); }

/// \return `t.x == 0 && t.y == 0 && t.z == 0 && t.w == 0` where `t = and(a, b)`
inline bool testz(const m128& a, const m128& b)
  noexcept { return _mm_testz_ps(a, b); }

/// \return `testc(a, b) == false && testz(a, b) == false`
inline bool testnzc(const m128& a, const m128& b)
  noexcept { return _mm_testnzc_ps(a, b); }

} ////////////////////////////////////////////////////////////////////////////// namespace yw::mm
