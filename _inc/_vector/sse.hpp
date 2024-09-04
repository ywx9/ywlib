/// \file vector/sse.hpp

#pragma once

#include <immintrin.h>

export namespace yw::mm {

using m128 = __m128;

/// \return `{a.x + b.x, ...}`
inline m128 add(const m128& a, const m128& b)
  noexcept { return _mm_add_ps(a, b); }

/// \return `{a.x + b.x, a.y, a.z, a.w}`
inline m128 add_single(const m128& a, const m128& b)
  noexcept { return _mm_add_ss(a, b); }

/// \return `{a.x - b.x, a.y + b.y, a.z - b.z, a.w + b.w}`
inline m128 addsub(const m128& a, const m128& b)
  noexcept { return _mm_addsub_ps(a, b); }

/// \return `{a.x & b.x, ...}`
inline m128 and_(const m128& a, const m128& b)
  noexcept { return _mm_and_ps(a, b); }

/// \return `{a.x & ~b.x, ...}`
inline m128 andnot(const m128& a, const m128& b)
  noexcept { return _mm_andnot_ps(a, b); }

/// \return `{imm8[0] ? b.x : a.x, imm8[1] ? b.y : a.y, imm8[2] ? b.z : a.z, imm8[3] ? b.w : a.w}`
template<int imm8>
inline m128 blend(const m128& a, const m128& b)
  noexcept { return _mm_blend_ps(a, b, imm8); }

/// \return `{mask.x < 0 ? b.x : a.x, ...`
inline m128 blendv(const m128& a, const m128& b, const m128& mask)
  noexcept { return _mm_blendv_ps(a, b, mask); }

/// \return `{ceil(a.x), ...`
inline m128 ceil(const m128& a)
  noexcept { return _mm_ceil_ps(a); }

/// \return `{ceil(b.x), a.y, a.z, a.w}`
inline m128 ceil_single(const m128& a, const m128& b)
  noexcept { return _mm_ceil_ss(a, b); }

/// \return `{a.x == b.x ? 0xFFFFFFFF : 0, ...}`
inline m128 cmpeq(const m128& a, const m128& b)
  noexcept { return _mm_cmpeq_ps(a, b); }

/// \return `{a.x == b.x ? 0xFFFFFFFF : 0, a.y, a.z, a.w}`
inline m128 cmpeq_single(const m128& a, const m128& b)
  noexcept { return _mm_cmpeq_ss(a, b); }

/// \return `{a.x >= b.x ? 0xFFFFFFFF : 0, ...}`
inline m128 cmpge(const m128& a, const m128& b)
  noexcept { return _mm_cmpge_ps(a, b); }

/// \return `{a.x >= b.x ? 0xFFFFFFFF : 0, a.y, a.z, a.w}`
inline m128 cmpge_single(const m128& a, const m128& b)
  noexcept { return _mm_cmpge_ss(a, b); }

/// \return `{a.x > b.x ? 0xFFFFFFFF : 0, ...}`
inline m128 cmpgt(const m128& a, const m128& b)
  noexcept { return _mm_cmpgt_ps(a, b); }

/// \return `{a.x > b.x ? 0xFFFFFFFF : 0, a.y, a.z, a.w}`
inline m128 cmpgt_single(const m128& a, const m128& b)
  noexcept { return _mm_cmpgt_ss(a, b); }

/// \return `{a.x <= b.x ? 0xFFFFFFFF : 0, ...}`
inline m128 cmple(const m128& a, const m128& b)
  noexcept { return _mm_cmple_ps(a, b); }

/// \return `{a.x <= b.x ? 0xFFFFFFFF : 0, a.y, a.z, a.w}`
inline m128 cmple_single(const m128& a, const m128& b)
  noexcept { return _mm_cmple_ss(a, b); }

/// \return `{a.x < b.x ? 0xFFFFFFFF : 0, ...}`
inline m128 cmplt(const m128& a, const m128& b)
  noexcept { return _mm_cmplt_ps(a, b); }

/// \return `{a.x < b.x ? 0xFFFFFFFF : 0, a.y, a.z, a.w}`
inline m128 cmplt_single(const m128& a, const m128& b)
  noexcept { return _mm_cmplt_ss(a, b); }

/// \return `{a.x != b.x ? 0xFFFFFFFF : 0, ...}`
inline m128 cmpneq(const m128& a, const m128& b)
  noexcept { return _mm_cmpneq_ps(a, b); }

/// \return `{a.x != b.x ? 0xFFFFFFFF : 0, a.y, a.z, a.w}`
inline m128 cmpneq_single(const m128& a, const m128& b)
  noexcept { return _mm_cmpneq_ss(a, b); }

/// \return `{!(a.x >= b.x) ? 0xFFFFFFFF : 0, ...}`
inline m128 cmpnge(const m128& a, const m128& b)
  noexcept { return _mm_cmpnge_ps(a, b); }

/// \return `{!(a.x >= b.x) ? 0xFFFFFFFF : 0, a.y, a.z, a.w}`
inline m128 cmpnge_single(const m128& a, const m128& b)
  noexcept { return _mm_cmpnge_ss(a, b); }

/// \return `{!(a.x > b.x) ? 0xFFFFFFFF : 0, ...}`
inline m128 cmpngt(const m128& a, const m128& b)
  noexcept { return _mm_cmpngt_ps(a, b); }

/// \return `{!(a.x > b.x) ? 0xFFFFFFFF : 0, a.y, a.z, a.w}`
inline m128 cmpngt_single(const m128& a, const m128& b)
  noexcept { return _mm_cmpngt_ss(a, b); }

/// \return `{!(a.x <= b.x) ? 0xFFFFFFFF : 0, ...}`
inline m128 cmpnle(const m128& a, const m128& b)
  noexcept { return _mm_cmpnle_ps(a, b); }

/// \return `{!(a.x <= b.x) ? 0xFFFFFFFF : 0, a.y, a.z, a.w}`
inline m128 cmpnle_single(const m128& a, const m128& b)
  noexcept { return _mm_cmpnle_ss(a, b); }

/// \return `{!(a.x < b.x) ? 0xFFFFFFFF : 0, ...}`
inline m128 cmpnlt(const m128& a, const m128& b)
  noexcept { return _mm_cmpnlt_ps(a, b); }

/// \return `{!(a.x < b.x) ? 0xFFFFFFFF : 0, a.y, a.z, a.w}`
inline m128 cmpnlt_single(const m128& a, const m128& b)
  noexcept { return _mm_cmpnlt_ss(a, b); }

/// \return `{a.x != nan && b.x != nan ? 0xFFFFFFFF : 0, ...}`
inline m128 cmpord(const m128& a, const m128& b)
  noexcept { return _mm_cmpord_ps(a, b); }

/// \return `{a.x != nan && b.x != nan ? 0xFFFFFFFF : 0, a.y, a.z, a.w}`
inline m128 cmpord_single(const m128& a, const m128& b)
  noexcept { return _mm_cmpord_ss(a, b); }

/// \return `{a.x == nan || b.x == nan ? 0xFFFFFFFF : 0, ...}`
inline m128 cmpunord(const m128& a, const m128& b)
  noexcept { return _mm_cmpunord_ps(a, b); }

/// \return `{a.x == nan || b.x == nan ? 0xFFFFFFFF : 0, a.y, a.z, a.w}`
inline m128 cmpunord_single(const m128& a, const m128& b)
  noexcept { return _mm_cmpunord_ss(a, b); }

/// \return `a.x != nan && b.x != nan && a.x == b.x`
inline int comieq_single(const m128& a, const m128& b)
  noexcept { return _mm_comieq_ss(a, b); }

/// \return `a.x != nan && b.x != nan && a.x >= b.x`
inline int comige_single(const m128& a, const m128& b)
  noexcept { return _mm_comige_ss(a, b); }

/// \return `a.x != nan && b.x != nan && a.x > b.x`
inline int comigt_single(const m128& a, const m128& b)
  noexcept { return _mm_comigt_ss(a, b); }

/// \return `a.x != nan && b.x != nan && a.x <= b.x`
inline int comile_single(const m128& a, const m128& b)
  noexcept { return _mm_comile_ss(a, b); }

/// \return `a.x != nan && b.x != nan && a.x < b.x`
inline int comilt_single(const m128& a, const m128& b)
  noexcept { return _mm_comilt_ss(a, b); }

/// \return `a.x != nan && b.x != nan && a.x != b.x`
inline int comineq_single(const m128& a, const m128& b)
  noexcept { return _mm_comineq_ss(a, b); }

/// \return `{int(bitcast<float>(a.x)), ...}`
inline m128 cvt_i32(const m128& a)
  noexcept { return _mm_castsi128_ps(_mm_cvtps_epi32(a)); }

/// \return `{float(bitcast<int>(a.x)), ...}`
inline m128 cvt_f32(const m128& a)
  noexcept { return _mm_cvtepi32_ps(_mm_castps_si128(a)); }

/// \return `{a.x / b.x, ...}`
inline m128 div(const m128& a, const m128& b)
  noexcept { return _mm_div_ps(a, b); }

/// \return `{a.x / b.x, a.y, a.z, a.w}`
inline m128 div_single(const m128& a, const m128& b)
  noexcept { return _mm_div_ss(a, b); }

/// \return `{imm8[0] ? t : 0, imm8[1] ? t : 0, imm8[2] ? t : 0, imm8[3] ? t : 0}` where `t = (imm8[4] ? a.x * b.x : 0) + (imm8[5] ? a.y * b.y : 0) + (imm8[6] ? a.z * b.z : 0) + (imm8[7] ? a.w * b.w : 0)`
template<int imm8>
inline m128 dp(const m128& a, const m128& b)
  noexcept { return _mm_dp_ps(a, b, imm8); }

/// \return `bitcast<int>(a[imm8[0:1]])`
template<int imm8> inline int extract(const m128& a)
  noexcept { return _mm_extract_ps(a, imm8); }

/// \return `{floor(a.x), ...}`
inline m128 floor(const m128& a)
  noexcept { return _mm_floor_ps(a); }

/// \return `{floor(b.x), a.y, a.z, a.w}`
inline m128 floor_single(const m128& a, const m128& b)
  noexcept { return _mm_floor_ss(a, b); }

/// \return `{a.y + a.x, a.w + a.z, b.y + b.x, b.w + b.z}`
inline m128 hadd(const m128& a, const m128& b)
  noexcept { return _mm_hadd_ps(a, b); }

/// \return `{a.y - a.x, a.w - a.z, b.y - b.x, b.w - b.z}`
inline m128 hsub(const m128& a, const m128& b)
  noexcept { return _mm_hsub_ps(a, b); }

/// \return `{imm8[0] ? 0 : t.x, imm8[1] ? 0 : t.y, imm8[2] ? 0 : t.z, imm8[3] ? 0 : t.w}` where `t = a, t[imm8[4:5]] = b[imm8[6:7]]`
template<int imm8>
inline m128 insert(const m128& a, const m128& b)
  noexcept { return _mm_insert_ps(a, b, imm8); }

/// \return `{ptr[0], ptr[1], ptr[2], ptr[3]}`
/// \note `ptr` must be aligned to 16 bytes
inline m128 load(const float* ptr)
  noexcept { return _mm_load_ps(ptr); }

/// \return `{ptr[0], ptr[0], ptr[0], ptr[0]}`
inline m128 load1(const float* ptr)
  noexcept { return _mm_load_ps1(ptr); }

/// \return `{ptr[0], 0, 0, 0}`
inline m128 load_single(const float* ptr)
  noexcept { return _mm_load_ss(ptr); }

/// \return `{ptr[3], ptr[2], ptr[1], ptr[0]}`
/// \note `ptr` must be aligned to 16 bytes
inline m128 loadr(const float* ptr)
  noexcept { return _mm_loadr_ps(ptr); }

/// \return `{ptr[0], ptr[1], ptr[2], ptr[3]}`
inline m128 loadu(const float* ptr)
  noexcept { return _mm_loadu_ps(ptr); }

/// \return `{max(a.x, b.x), ...}`
inline m128 max(const m128& a, const m128& b)
  noexcept { return _mm_max_ps(a, b); }

/// \return `{max(a.x, b.x), a.y, a.z, a.w}`
inline m128 max_single(const m128& a, const m128& b)
  noexcept { return _mm_max_ss(a, b); }

/// \return `{min(a.x, b.x), ...}`
inline m128 min(const m128& a, const m128& b)
  noexcept { return _mm_min_ps(a, b); }

/// \return `{min(a.x, b.x), a.y, a.z, a.w}`
inline m128 min_single(const m128& a, const m128& b)
  noexcept { return _mm_min_ss(a, b); }

/// \return `{b.x, a.y, a.z, a.w}`
inline m128 move_single(const m128& a, const m128& b)
  noexcept { return _mm_move_ss(a, b); }

/// \return `{a.z, a.z, a.w, a.w}`
inline m128 move_high(const m128& a)
  noexcept { return _mm_movehdup_ps(a); }

/// \return `{b.z, b.w, a.z, a.w}`
inline m128 move_high(const m128& a, const m128& b)
  noexcept { return _mm_movehl_ps(a, b); }

/// \return `{a.x, a.x, a.y, a.y}`
inline m128 move_low(const m128& a)
  noexcept { return _mm_moveldup_ps(a); }

/// \return `{a.x, a.y, b.x, b.y}`
inline m128 move_low(const m128& a, const m128& b)
  noexcept { return _mm_movelh_ps(a, b); }

/// \return `bitcast<int>((a.x < 0) & (a.y < 0) << 1 & (a.z < 0) << 2 & (a.w < 0) << 3)`
inline int movemask(const m128& a)
  noexcept { return _mm_movemask_ps(a); }

/// \return `{a.x * b.x, ...}`
inline m128 mul(const m128& a, const m128& b)
  noexcept { return _mm_mul_ps(a, b); }

/// \return `{a.x * b.x, a.y, a.z, a.w}`
inline m128 mul_single(const m128& a, const m128& b)
  noexcept { return _mm_mul_ss(a, b); }

/// \return `{a.x | b.x, ...}`
inline m128 or_(const m128& a, const m128& b)
  noexcept { return _mm_or_ps(a, b); }

/// \return `{1 / a.x, ...}`
inline m128 rcp(const m128& a)
  noexcept { return _mm_rcp_ps(a); }

/// \return `{1 / a.x, a.y, a.z, a.w}`
inline m128 rcp_single(const m128& a)
  noexcept { return _mm_rcp_ss(a); }

/// \return `{round(a.x), ...}`
inline m128 round(const m128& a)
  noexcept { return _mm_round_ps(a, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC); }

/// \return `{round(b.x), a.y, a.z, a.w}`
inline m128 round_single(const m128& a, const m128& b)
  noexcept { return _mm_round_ss(a, b, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC); }

/// \return `{rsqrt(a.x), ...}`
inline m128 rsqrt(const m128& a)
  noexcept { return _mm_rsqrt_ps(a); }

/// \return `{rsqrt(a.x), a.y, a.z, a.w}`
inline m128 rsqrt_single(const m128& a)
  noexcept { return _mm_rsqrt_ss(a); }

/// \return `{d, c, b, a}`
inline m128 set(float a, float b, float c, float d)
  noexcept { return _mm_set_ps(a, b, c, d); }

/// \return `{a, a, a, a}`
inline m128 set1(float a)
  noexcept { return _mm_set1_ps(a); }

/// \return `{a, 0, 0, 0}`
inline m128 set_single(float a)
  noexcept { return _mm_set_ss(a); }

/// \return `{a, b, c, d}`
inline m128 setr(float a, float b, float c, float d)
  noexcept { return _mm_setr_ps(a, b, c, d); }

/// \return `{0, 0, 0, 0}`
inline m128 setzero()
  noexcept { return _mm_setzero_ps(); }

/// \return `{extract<imm8[0:1]>(a), extract<imm8[2:3]>(a), extract<imm8[4:5]>(b), extract<imm8[6:7]>(b)}`
template<int imm8>
inline m128 shuffle(const m128& a, const m128& b)
  noexcept { return _mm_shuffle_ps(a, b, imm8); }

/// \return `{sqrt(a.x), ...}`
inline m128 sqrt(const m128& a)
  noexcept { return _mm_sqrt_ps(a); }

/// \return `{sqrt(a.x), a.y, a.z, a.w}`
inline m128 sqrt_single(const m128& a)
  noexcept { return _mm_sqrt_ss(a); }

/// stores `a` to `ptr`; `{a.x, a.y, a.z, a.w}`
/// \note `ptr` must be aligned to 16 bytes
inline void store(float* ptr, const m128& a)
  noexcept { _mm_store_ps(ptr, a); }

/// stores `a` to `ptr`; `{a.x, a.x, a.x, a.x}`
/// \note `ptr` must be aligned to 16 bytes
inline void store1(float* ptr, const m128& a)
  noexcept { _mm_store_ps1(ptr, a); }

/// stores `a` to `ptr`; `{a.x, 0, 0, 0}`
inline void store_single(float* ptr, const m128& a)
  noexcept { _mm_store_ss(ptr, a); }

/// stores `a` to `ptr`; `{a.w, a.z, a.y, a.x}`
/// \note `ptr` must be aligned to 16 bytes
inline void storer(float* ptr, const m128& a)
  noexcept { _mm_storer_ps(ptr, a); }

/// stores `a` to `ptr`; `{a.w, a.z, a.y, a.x}`
inline void storeu(float* ptr, const m128& a)
  noexcept { _mm_storeu_ps(ptr, a); }

/// \return `{a.x - b.x, ...}`
inline m128 sub(const m128& a, const m128& b)
  noexcept { return _mm_sub_ps(a, b); }

/// \return `{a.x - b.x, a.y, a.z, a.w}`
inline m128 sub_single(const m128& a, const m128& b)
  noexcept { return _mm_sub_ss(a, b); }

/// \return `{a.z, b.z, a.w, b.w}`
inline m128 unpack_high(const m128& a, const m128& b)
  noexcept { return _mm_unpackhi_ps(a, b); }

/// \return `{a.x, b.x, a.y, b.y}`
inline m128 unpack_low(const m128& a, const m128& b)
  noexcept { return _mm_unpacklo_ps(a, b); }

/// \return `{a.x ^ b.x, ...}`
inline m128 xor_(const m128& a, const m128& b)
  noexcept { return _mm_xor_ps(a, b); }

} ////////////////////////////////////////////////////////////////////////////// namespace yw::mm
