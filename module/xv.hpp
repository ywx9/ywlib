#pragma once

#include <immintrin.h>

#include "std.hpp"

#define ywstd_intrin_macro(return_type, name, args, params)        \
  inline return_type name args noexcept { return _##name params; }
#define ywstd_intrin_macro_imm8(return_type, name, args, params)               \
  template<int imm8> return_type name args noexcept { return _##name params; }

export namespace intrin {

using m128 = __m128;
using m128i = __m128i;
using m128d = __m128d;
using m256 = __m256;
using m256i = __m256i;
using m256d = __m256d;

inline m128i mm_add_epi8(const m128i& a, const m128i& b) noexcept { return _mm_add_epi8(a, b); }
inline m256i mm256_add_epi8(const m256i& a, const m256i& b) noexcept { return _mm256_add_epi8(a, b); }
inline m128i mm_add_epi16(const m128i& a, const m128i& b) noexcept { return _mm_add_epi16(a, b); }
inline m256i mm256_add_epi16(const m256i& a, const m256i& b) noexcept { return _mm256_add_epi16(a, b); }
inline m128i mm_add_epi32(const m128i& a, const m128i& b) noexcept { return _mm_add_epi32(a, b); }
inline m256i mm256_add_epi32(const m256i& a, const m256i& b) noexcept { return _mm256_add_epi32(a, b); }
inline m128i mm_add_epi64(const m128i& a, const m128i& b) noexcept { return _mm_add_epi64(a, b); }
inline m256i mm256_add_epi64(const m256i& a, const m256i& b) noexcept { return _mm256_add_epi64(a, b); }

inline m128 mm_add_ss(const m128& a, const m128& b) noexcept { return _mm_add_ss(a, b); }
inline m128 mm_add_ps(const m128& a, const m128& b) noexcept { return _mm_add_ps(a, b); }
inline m256 mm256_add_ps(const m256& a, const m256& b) noexcept { return _mm256_add_ps(a, b); }
inline m128d mm_add_sd(const m128d& a, const m128d& b) noexcept { return _mm_add_sd(a, b); }
inline m128d mm_add_pd(const m128d& a, const m128d& b) noexcept { return _mm_add_pd(a, b); }
inline m256d mm256_add_pd(const m256d& a, const m256d& b) noexcept { return _mm256_add_pd(a, b); }

inline m128i mm_adds_epi8(const m128i& a, const m128i& b) noexcept { return _mm_adds_epi8(a, b); }
inline m256i mm256_adds_epi8(const m256i& a, const m256i& b) noexcept { return _mm256_adds_epi8(a, b); }
inline m128i mm_adds_epi16(const m128i& a, const m128i& b) noexcept { return _mm_adds_epi16(a, b); }
inline m256i mm256_adds_epi16(const m256i& a, const m256i& b) noexcept { return _mm256_adds_epi16(a, b); }

inline m128i mm_adds_epu8(const m128i& a, const m128i& b) noexcept { return _mm_adds_epu8(a, b); }
inline m256i mm256_adds_epu8(const m256i& a, const m256i& b) noexcept { return _mm256_adds_epu8(a, b); }
inline m128i mm_adds_epu16(const m128i& a, const m128i& b) noexcept { return _mm_adds_epu16(a, b); }
inline m256i mm256_adds_epu16(const m256i& a, const m256i& b) noexcept { return _mm256_adds_epu16(a, b); }

inline m128 mm_addsub_ps(const m128& a, const m128& b) noexcept { return _mm_addsub_ps(a, b); }
inline m256 mm256_addsub_ps(const m256& a, const m256& b) noexcept { return _mm256_addsub_ps(a, b); }
inline m128d mm_addsub_pd(const m128d& a, const m128d& b) noexcept { return _mm_addsub_pd(a, b); }
inline m256d mm256_addsub_pd(const m256d& a, const m256d& b) noexcept { return _mm256_addsub_pd(a, b); }

inline m128i mm_div_epi8(const m128i& a, const m128i& b) noexcept { return _mm_div_epi8(a, b); }
inline m256i mm256_div_epi8(const m256i& a, const m256i& b) noexcept { return _mm256_div_epi8(a, b); }
inline m128i mm_div_epi16(const m128i& a, const m128i& b) noexcept { return _mm_div_epi16(a, b); }
inline m256i mm256_div_epi16(const m256i& a, const m256i& b) noexcept { return _mm256_div_epi16(a, b); }
inline m128i mm_div_epi32(const m128i& a, const m128i& b) noexcept { return _mm_div_epi32(a, b); }
inline m256i mm256_div_epi32(const m256i& a, const m256i& b) noexcept { return _mm256_div_epi32(a, b); }
inline m128i mm_div_epi64(const m128i& a, const m128i& b) noexcept { return _mm_div_epi64(a, b); }
inline m256i mm256_div_epi64(const m256i& a, const m256i& b) noexcept { return _mm256_div_epi64(a, b); }

inline m128i mm_div_epu8(const m128i& a, const m128i& b) noexcept { return _mm_div_epu8(a, b); }
inline m256i mm256_div_epu8(const m256i& a, const m256i& b) noexcept { return _mm256_div_epu8(a, b); }
inline m128i mm_div_epu16(const m128i& a, const m128i& b) noexcept { return _mm_div_epu16(a, b); }
inline m256i mm256_div_epu16(const m256i& a, const m256i& b) noexcept { return _mm256_div_epu16(a, b); }
inline m128i mm_div_epu32(const m128i& a, const m128i& b) noexcept { return _mm_div_epu32(a, b); }
inline m256i mm256_div_epu32(const m256i& a, const m256i& b) noexcept { return _mm256_div_epu32(a, b); }
inline m128i mm_div_epu64(const m128i& a, const m128i& b) noexcept { return _mm_div_epu64(a, b); }
inline m256i mm256_div_epu64(const m256i& a, const m256i& b) noexcept { return _mm256_div_epu64(a, b); }

inline m128 mm_div_ss(const m128& a, const m128& b) noexcept { return _mm_div_ss(a, b); }
inline m128 mm_div_ps(const m128& a, const m128& b) noexcept { return _mm_div_ps(a, b); }
inline m256 mm256_div_ps(const m256& a, const m256& b) noexcept { return _mm256_div_ps(a, b); }
inline m128d mm_div_sd(const m128d& a, const m128d& b) noexcept { return _mm_div_sd(a, b); }
inline m128d mm_div_pd(const m128d& a, const m128d& b) noexcept { return _mm_div_pd(a, b); }
inline m256d mm256_div_pd(const m256d& a, const m256d& b) noexcept { return _mm256_div_pd(a, b); }

template<int imm8> m128 mm_dp_ps(const m128& a, const m128& b) noexcept { return _mm_dp_ps(a, b, imm8); }
template<int imm8> m256 mm256_dp_ps(const m256& a, const m256& b) noexcept { return _mm256_dp_ps(a, b, imm8); }
template<int imm8> m128d mm_dp_pd(const m128d& a, const m128d& b) noexcept { return _mm_dp_pd(a, b, imm8); }

inline m128 mm_fmadd_ss(const m128& a, const m128& b, const m128& c) noexcept { return _mm_fmadd_ss(a, b, c); }
inline m128 mm_fmadd_ps(const m128& a, const m128& b, const m128& c) noexcept { return _mm_fmadd_ps(a, b, c); }
inline m256 mm256_fmadd_ps(const m256& a, const m256& b, const m256& c) noexcept { return _mm256_fmadd_ps(a, b, c); }
inline m128d mm_fmadd_sd(const m128d& a, const m128d& b, const m128d& c) noexcept { return _mm_fmadd_sd(a, b, c); }
inline m128d mm_fmadd_pd(const m128d& a, const m128d& b, const m128d& c) noexcept { return _mm_fmadd_pd(a, b, c); }
inline m256d mm256_fmadd_pd(const m256d& a, const m256d& b, const m256d& c) noexcept { return _mm256_fmadd_pd(a, b, c); }

inline m128 mm_fmaddsub_ps(const m128& a, const m128& b, const m128& c) noexcept { return _mm_fmaddsub_ps(a, b, c); }
inline m256 mm256_fmaddsub_ps(const m256& a, const m256& b, const m256& c) noexcept { return _mm256_fmaddsub_ps(a, b, c); }
inline m128d mm_fmaddsub_pd(const m128d& a, const m128d& b, const m128d& c) noexcept { return _mm_fmaddsub_pd(a, b, c); }
inline m256d mm256_fmaddsub_pd(const m256d& a, const m256d& b, const m256d& c) noexcept { return _mm256_fmaddsub_pd(a, b, c); }

inline m128 mm_fmsub_ss(const m128& a, const m128& b, const m128& c) noexcept { return _mm_fmsub_ss(a, b, c); }
inline m128 mm_fmsub_ps(const m128& a, const m128& b, const m128& c) noexcept { return _mm_fmsub_ps(a, b, c); }
inline m256 mm256_fmsub_ps(const m256& a, const m256& b, const m256& c) noexcept { return _mm256_fmsub_ps(a, b, c); }
inline m128d mm_fmsub_sd(const m128d& a, const m128d& b, const m128d& c) noexcept { return _mm_fmsub_sd(a, b, c); }
inline m128d mm_fmsub_pd(const m128d& a, const m128d& b, const m128d& c) noexcept { return _mm_fmsub_pd(a, b, c); }
inline m256d mm256_fmsub_pd(const m256d& a, const m256d& b, const m256d& c) noexcept { return _mm256_fmsub_pd(a, b, c); }

inline m128 mm_fmsubadd_ps(const m128& a, const m128& b, const m128& c) noexcept { return _mm_fmsubadd_ps(a, b, c); }
inline m256 mm256_fmsubadd_ps(const m256& a, const m256& b, const m256& c) noexcept { return _mm256_fmsubadd_ps(a, b, c); }
inline m128d mm_fmsubadd_pd(const m128d& a, const m128d& b, const m128d& c) noexcept { return _mm_fmsubadd_pd(a, b, c); }
inline m256d mm256_fmsubadd_pd(const m256d& a, const m256d& b, const m256d& c) noexcept { return _mm256_fmsubadd_pd(a, b, c); }

inline m128 mm_fnmadd_ss(const m128& a, const m128& b, const m128& c) noexcept { return _mm_fnmadd_ss(a, b, c); }
inline m128 mm_fnmadd_ps(const m128& a, const m128& b, const m128& c) noexcept { return _mm_fnmadd_ps(a, b, c); }
inline m256 mm256_fnmadd_ps(const m256& a, const m256& b, const m256& c) noexcept { return _mm256_fnmadd_ps(a, b, c); }
inline m128d mm_fnmadd_sd(const m128d& a, const m128d& b, const m128d& c) noexcept { return _mm_fnmadd_sd(a, b, c); }
inline m128d mm_fnmadd_pd(const m128d& a, const m128d& b, const m128d& c) noexcept { return _mm_fnmadd_pd(a, b, c); }
inline m256d mm256_fnmadd_pd(const m256d& a, const m256d& b, const m256d& c) noexcept { return _mm256_fnmadd_pd(a, b, c); }

inline m128 mm_fnmsub_ss(const m128& a, const m128& b, const m128& c) noexcept { return _mm_fnmsub_ss(a, b, c); }
inline m128 mm_fnmsub_ps(const m128& a, const m128& b, const m128& c) noexcept { return _mm_fnmsub_ps(a, b, c); }
inline m256 mm256_fnmsub_ps(const m256& a, const m256& b, const m256& c) noexcept { return _mm256_fnmsub_ps(a, b, c); }
inline m128d mm_fnmsub_sd(const m128d& a, const m128d& b, const m128d& c) noexcept { return _mm_fnmsub_sd(a, b, c); }
inline m128d mm_fnmsub_pd(const m128d& a, const m128d& b, const m128d& c) noexcept { return _mm_fnmsub_pd(a, b, c); }
inline m256d mm256_fnmsub_pd(const m256d& a, const m256d& b, const m256d& c) noexcept { return _mm256_fnmsub_pd(a, b, c); }

inline m128i mm_hadd_epi16(const m128i& a, const m128i& b) noexcept { return _mm_hadd_epi16(a, b); }
inline m256i mm256_hadd_epi16(const m256i& a, const m256i& b) noexcept { return _mm256_hadd_epi16(a, b); }
inline m128i mm_hadd_epi32(const m128i& a, const m128i& b) noexcept { return _mm_hadd_epi32(a, b); }
inline m256i mm256_hadd_epi32(const m256i& a, const m256i& b) noexcept { return _mm256_hadd_epi32(a, b); }

inline m128 mm_hadd_ps(const m128& a, const m128& b) noexcept { return _mm_hadd_ps(a, b); }
inline m256 mm256_hadd_ps(const m256& a, const m256& b) noexcept { return _mm256_hadd_ps(a, b); }
inline m128d mm_hadd_pd(const m128d& a, const m128d& b) noexcept { return _mm_hadd_pd(a, b); }
inline m256d mm256_hadd_pd(const m256d& a, const m256d& b) noexcept { return _mm256_hadd_pd(a, b); }

inline m128i mm_hadds_epi16(const m128i& a, const m128i& b) noexcept { return _mm_hadds_epi16(a, b); }
inline m256i mm256_hadds_epi16(const m256i& a, const m256i& b) noexcept { return _mm256_hadds_epi16(a, b); }

inline m128i mm_hsub_epi16(const m128i& a, const m128i& b) noexcept { return _mm_hsub_epi16(a, b); }
inline m256i mm256_hsub_epi16(const m256i& a, const m256i& b) noexcept { return _mm256_hsub_epi16(a, b); }
inline m128i mm_hsub_epi32(const m128i& a, const m128i& b) noexcept { return _mm_hsub_epi32(a, b); }
inline m256i mm256_hsub_epi32(const m256i& a, const m256i& b) noexcept { return _mm256_hsub_epi32(a, b); }

inline m128 mm_hsub_ps(const m128& a, const m128& b) noexcept { return _mm_hsub_ps(a, b); }
inline m256 mm256_hsub_ps(const m256& a, const m256& b) noexcept { return _mm256_hsub_ps(a, b); }
inline m128d mm_hsub_pd(const m128d& a, const m128d& b) noexcept { return _mm_hsub_pd(a, b); }
inline m256d mm256_hsub_pd(const m256d& a, const m256d& b) noexcept { return _mm256_hsub_pd(a, b); }

inline m128i mm_hsubs_epi16(const m128i& a, const m128i& b) noexcept { return _mm_hsubs_epi16(a, b); }
inline m256i mm256_hsubs_epi16(const m256i& a, const m256i& b) noexcept { return _mm256_hsubs_epi16(a, b); }

inline m128i mm_idiv_epi32(const m128i& a, const m128i& b) noexcept { return _mm_idiv_epi32(a, b); }
inline m256i mm256_idiv_epi32(const m256i& a, const m256i& b) noexcept { return _mm256_idiv_epi32(a, b); }
inline m128i mm_idivrem_epi32(m128i* mem_addr, const m128i& a, const m128i& b) noexcept { return _mm_idivrem_epi32(mem_addr, a, b); }
inline m256i mm256_idivrem_epi32(m256i* mem_addr, const m256i& a, const m256i& b) noexcept { return _mm256_idivrem_epi32(mem_addr, a, b); }
inline m128i mm_irem_epi32(const m128i& a, const m128i& b) noexcept { return _mm_irem_epi32(a, b); }
inline m256i mm256_irem_epi32(const m256i& a, const m256i& b) noexcept { return _mm256_irem_epi32(a, b); }

inline m128i mm_madd_epi16(const m128i& a, const m128i& b) noexcept { return _mm_madd_epi16(a, b); }
inline m256i mm256_madd_epi16(const m256i& a, const m256i& b) noexcept { return _mm256_madd_epi16(a, b); }
inline m128i mm_maddubs_epi16(const m128i& a, const m128i& b) noexcept { return _mm_maddubs_epi16(a, b); }
inline m256i mm256_maddubs_epi16(const m256i& a, const m256i& b) noexcept { return _mm256_maddubs_epi16(a, b); }

inline m128i mm_mul_epi32(const m128i& a, const m128i& b) noexcept { return _mm_mul_epi32(a, b); }
inline m256i mm256_mul_epi32(const m256i& a, const m256i& b) noexcept { return _mm256_mul_epi32(a, b); }
inline m128i mm_mul_epu32(const m128i& a, const m128i& b) noexcept { return _mm_mul_epu32(a, b); }
inline m256i mm256_mul_epu32(const m256i& a, const m256i& b) noexcept { return _mm256_mul_epu32(a, b); }

inline m128 mm_mul_ss(const m128& a, const m128& b) noexcept { return _mm_mul_ss(a, b); }
inline m128 mm_mul_ps(const m128& a, const m128& b) noexcept { return _mm_mul_ps(a, b); }
inline m256 mm256_mul_ps(const m256& a, const m256& b) noexcept { return _mm256_mul_ps(a, b); }
inline m128d mm_mul_sd(const m128d& a, const m128d& b) noexcept { return _mm_mul_sd(a, b); }
inline m128d mm_mul_pd(const m128d& a, const m128d& b) noexcept { return _mm_mul_pd(a, b); }
inline m256d mm256_mul_pd(const m256d& a, const m256d& b) noexcept { return _mm256_mul_pd(a, b); }

inline m128i mm_mulhi_epi16(const m128i& a, const m128i& b) noexcept { return _mm_mulhi_epi16(a, b); }
inline m256i mm256_mulhi_epi16(const m256i& a, const m256i& b) noexcept { return _mm256_mulhi_epi16(a, b); }
inline m128i mm_mulhi_epu16(const m128i& a, const m128i& b) noexcept { return _mm_mulhi_epu16(a, b); }
inline m256i mm256_mulhi_epu16(const m256i& a, const m256i& b) noexcept { return _mm256_mulhi_epu16(a, b); }

inline m128i mm_mulhrs_epi16(const m128i& a, const m128i& b) noexcept { return _mm_mulhrs_epi16(a, b); }
inline m256i mm256_mulhrs_epi16(const m256i& a, const m256i& b) noexcept { return _mm256_mulhrs_epi16(a, b); }

inline m128i mm_mullo_epi16(const m128i& a, const m128i& b) noexcept { return _mm_mullo_epi16(a, b); }
inline m256i mm256_mullo_epi16(const m256i& a, const m256i& b) noexcept { return _mm256_mullo_epi16(a, b); }
inline m128i mm_mullo_epi32(const m128i& a, const m128i& b) noexcept { return _mm_mullo_epi32(a, b); }
inline m256i mm256_mullo_epi32(const m256i& a, const m256i& b) noexcept { return _mm256_mullo_epi32(a, b); }
inline m128i mm_mullo_epi64(const m128i& a, const m128i& b) noexcept { return _mm_mullo_epi64(a, b); }
inline m256i mm256_mullo_epi64(const m256i& a, const m256i& b) noexcept { return _mm256_mullo_epi64(a, b); }

inline m128i mm_rem_epi8(const m128i& a, const m128i& b) noexcept { return _mm_rem_epi8(a, b); }
inline m256i mm256_rem_epi8(const m256i& a, const m256i& b) noexcept { return _mm256_rem_epi8(a, b); }
inline m128i mm_rem_epi16(const m128i& a, const m128i& b) noexcept { return _mm_rem_epi16(a, b); }
inline m256i mm256_rem_epi16(const m256i& a, const m256i& b) noexcept { return _mm256_rem_epi16(a, b); }
inline m128i mm_rem_epi32(const m128i& a, const m128i& b) noexcept { return _mm_rem_epi32(a, b); }
inline m256i mm256_rem_epi32(const m256i& a, const m256i& b) noexcept { return _mm256_rem_epi32(a, b); }
inline m128i mm_rem_epi64(const m128i& a, const m128i& b) noexcept { return _mm_rem_epi64(a, b); }
inline m256i mm256_rem_epi64(const m256i& a, const m256i& b) noexcept { return _mm256_rem_epi64(a, b); }

inline m128i mm_rem_epu8(const m128i& a, const m128i& b) noexcept { return _mm_rem_epu8(a, b); }
inline m256i mm256_rem_epu8(const m256i& a, const m256i& b) noexcept { return _mm256_rem_epu8(a, b); }
inline m128i mm_rem_epu16(const m128i& a, const m128i& b) noexcept { return _mm_rem_epu16(a, b); }
inline m256i mm256_rem_epu16(const m256i& a, const m256i& b) noexcept { return _mm256_rem_epu16(a, b); }
inline m128i mm_rem_epu32(const m128i& a, const m128i& b) noexcept { return _mm_rem_epu32(a, b); }
inline m256i mm256_rem_epu32(const m256i& a, const m256i& b) noexcept { return _mm256_rem_epu32(a, b); }
inline m128i mm_rem_epu64(const m128i& a, const m128i& b) noexcept { return _mm_rem_epu64(a, b); }
inline m256i mm256_rem_epu64(const m256i& a, const m256i& b) noexcept { return _mm256_rem_epu64(a, b); }

inline m128i mm_sad_epu8(const m128i& a, const m128i& b) noexcept { return _mm_sad_epu8(a, b); }
inline m256i mm256_sad_epu8(const m256i& a, const m256i& b) noexcept { return _mm256_sad_epu8(a, b); }

inline m128i mm_sign_epi8(const m128i& a, const m128i& b) noexcept { return _mm_sign_epi8(a, b); }
inline m256i mm256_sign_epi8(const m256i& a, const m256i& b) noexcept { return _mm256_sign_epi8(a, b); }
inline m128i mm_sign_epi16(const m128i& a, const m128i& b) noexcept { return _mm_sign_epi16(a, b); }
inline m256i mm256_sign_epi16(const m256i& a, const m256i& b) noexcept { return _mm256_sign_epi16(a, b); }
inline m128i mm_sign_epi32(const m128i& a, const m128i& b) noexcept { return _mm_sign_epi32(a, b); }
inline m256i mm256_sign_epi32(const m256i& a, const m256i& b) noexcept { return _mm256_sign_epi32(a, b); }

inline m128i mm_sub_epi8(const m128i& a, const m128i& b) noexcept { return _mm_sub_epi8(a, b); }
inline m256i mm256_sub_epi8(const m256i& a, const m256i& b) noexcept { return _mm256_sub_epi8(a, b); }
inline m128i mm_sub_epi16(const m128i& a, const m128i& b) noexcept { return _mm_sub_epi16(a, b); }
inline m256i mm256_sub_epi16(const m256i& a, const m256i& b) noexcept { return _mm256_sub_epi16(a, b); }
inline m128i mm_sub_epi32(const m128i& a, const m128i& b) noexcept { return _mm_sub_epi32(a, b); }
inline m256i mm256_sub_epi32(const m256i& a, const m256i& b) noexcept { return _mm256_sub_epi32(a, b); }
inline m128i mm_sub_epi64(const m128i& a, const m128i& b) noexcept { return _mm_sub_epi64(a, b); }
inline m256i mm256_sub_epi64(const m256i& a, const m256i& b) noexcept { return _mm256_sub_epi64(a, b); }

inline m128 mm_sub_ss(const m128& a, const m128& b) noexcept { return _mm_sub_ss(a, b); }
inline m128 mm_sub_ps(const m128& a, const m128& b) noexcept { return _mm_sub_ps(a, b); }
inline m256 mm256_sub_ps(const m256& a, const m256& b) noexcept { return _mm256_sub_ps(a, b); }
inline m128d mm_sub_sd(const m128d& a, const m128d& b) noexcept { return _mm_sub_sd(a, b); }
inline m128d mm_sub_pd(const m128d& a, const m128d& b) noexcept { return _mm_sub_pd(a, b); }
inline m256d mm256_sub_pd(const m256d& a, const m256d& b) noexcept { return _mm256_sub_pd(a, b); }

inline m128i mm_subs_epi8(const m128i& a, const m128i& b) noexcept { return _mm_subs_epi8(a, b); }
inline m256i mm256_subs_epi8(const m256i& a, const m256i& b) noexcept { return _mm256_subs_epi8(a, b); }
inline m128i mm_subs_epi16(const m128i& a, const m128i& b) noexcept { return _mm_subs_epi16(a, b); }
inline m256i mm256_subs_epi16(const m256i& a, const m256i& b) noexcept { return _mm256_subs_epi16(a, b); }

inline m128i mm_subs_epu8(const m128i& a, const m128i& b) noexcept { return _mm_subs_epu8(a, b); }
inline m256i mm256_subs_epu8(const m256i& a, const m256i& b) noexcept { return _mm256_subs_epu8(a, b); }
inline m128i mm_subs_epu16(const m128i& a, const m128i& b) noexcept { return _mm_subs_epu16(a, b); }
inline m256i mm256_subs_epu16(const m256i& a, const m256i& b) noexcept { return _mm256_subs_epu16(a, b); }

inline m128i mm_udiv_epi32(const m128i& a, const m128i& b) noexcept { return _mm_udiv_epi32(a, b); }
inline m256i mm256_udiv_epi32(const m256i& a, const m256i& b) noexcept { return _mm256_udiv_epi32(a, b); }
inline m128i mm_udivrem_epi32(m128i* mem_addr, const m128i& a, const m128i& b) noexcept { return _mm_udivrem_epi32(mem_addr, a, b); }
inline m256i mm256_udivrem_epi32(m256i* mem_addr, const m256i& a, const m256i& b) noexcept { return _mm256_udivrem_epi32(mem_addr, a, b); }
inline m128i mm_urem_epi32(const m128i& a, const m128i& b) noexcept { return _mm_urem_epi32(a, b); }
inline m256i mm256_urem_epi32(const m256i& a, const m256i& b) noexcept { return _mm256_urem_epi32(a, b); }

inline m128 mm_castpd_ps(const m128d& a) noexcept { return _mm_castpd_ps(a); }
inline m256 mm256_castpd_ps(const m256d& a) noexcept { return _mm256_castpd_ps(a); }
inline m128i mm_castpd_si128(const m128d& a) noexcept { return _mm_castpd_si128(a); }
inline m256i mm256_castpd_si256(const m256d& a) noexcept { return _mm256_castpd_si256(a); }
inline m256d mm256_castpd128_pd256(const m128d& a) noexcept { return _mm256_castpd128_pd256(a); }
inline m128d mm256_castpd256_pd128(const m256d& a) noexcept { return _mm256_castpd256_pd128(a); }
inline m128d mm_castps_pd(const m128& a) noexcept { return _mm_castps_pd(a); }
inline m256d mm256_castps_pd(const m256& a) noexcept { return _mm256_castps_pd(a); }
inline m128i mm_castps_si128(const m128& a) noexcept { return _mm_castps_si128(a); }
inline m256i mm256_castps_si256(const m256& a) noexcept { return _mm256_castps_si256(a); }
inline m256 mm256_castps128_ps256(const m128& a) noexcept { return _mm256_castps128_ps256(a); }
inline m128 mm256_castps256_ps128(const m256& a) noexcept { return _mm256_castps256_ps128(a); }
inline m128d mm_castsi128_pd(const m128i& a) noexcept { return _mm_castsi128_pd(a); }
inline m128 mm_castsi128_ps(const m128i& a) noexcept { return _mm_castsi128_ps(a); }
inline m256i mm256_castsi128_si256(const m128i& a) noexcept { return _mm256_castsi128_si256(a); }
inline m256d mm256_castsi256_pd(const m256i& a) noexcept { return _mm256_castsi256_pd(a); }
inline m256 mm256_castsi256_ps(const m256i& a) noexcept { return _mm256_castsi256_ps(a); }
inline m128i mm256_castsi256_si128(const m256i& a) noexcept { return _mm256_castsi256_si128(a); }

inline m256 mm256_zextps128_ps256(const m128& a) noexcept { return _mm256_zextps128_ps256(a); }
inline m256d mm256_zextpd128_pd256(const m128d& a) noexcept { return _mm256_zextpd128_pd256(a); }
inline m256i mm256_zextsi128_si256(const m128i& a) noexcept { return _mm256_zextsi128_si256(a); }

template<int imm8> m128 mm_cmp_ss(const m128& a, const m128& b) noexcept { return _mm_cmp_ss(a, b, imm8); }
template<int imm8> m128 mm_cmp_ps(const m128& a, const m128& b) noexcept { return _mm_cmp_ps(a, b, imm8); }
template<int imm8> m256 mm256_cmp_ps(const m256& a, const m256& b) noexcept { return _mm256_cmp_ps(a, b, imm8); }
template<int imm8> m128d mm_cmp_sd(const m128d& a, const m128d& b) noexcept { return _mm_cmp_sd(a, b, imm8); }
template<int imm8> m128d mm_cmp_pd(const m128d& a, const m128d& b) noexcept { return _mm_cmp_pd(a, b, imm8); }
template<int imm8> m256d mm256_cmp_pd(const m256d& a, const m256d& b) noexcept { return _mm256_cmp_pd(a, b, imm8); }

inline m128i mm_cmpeq_epi8(const m128i& a, const m128i& b) noexcept { return _mm_cmpeq_epi8(a, b); }
inline m256i mm256_cmpeq_epi8(const m256i& a, const m256i& b) noexcept { return _mm256_cmpeq_epi8(a, b); }
inline m128i mm_cmpeq_epi16(const m128i& a, const m128i& b) noexcept { return _mm_cmpeq_epi16(a, b); }
inline m256i mm256_cmpeq_epi16(const m256i& a, const m256i& b) noexcept { return _mm256_cmpeq_epi16(a, b); }
inline m128i mm_cmpeq_epi32(const m128i& a, const m128i& b) noexcept { return _mm_cmpeq_epi32(a, b); }
inline m256i mm256_cmpeq_epi32(const m256i& a, const m256i& b) noexcept { return _mm256_cmpeq_epi32(a, b); }
inline m128i mm_cmpeq_epi64(const m128i& a, const m128i& b) noexcept { return _mm_cmpeq_epi64(a, b); }
inline m256i mm256_cmpeq_epi64(const m256i& a, const m256i& b) noexcept { return _mm256_cmpeq_epi64(a, b); }

inline m128 mm_cmpeq_ss(const m128& a, const m128& b) noexcept { return _mm_cmpeq_ss(a, b); }
inline m128 mm_cmpeq_ps(const m128& a, const m128& b) noexcept { return _mm_cmpeq_ps(a, b); }
inline m128d mm_cmpeq_sd(const m128d& a, const m128d& b) noexcept { return _mm_cmpeq_sd(a, b); }
inline m128d mm_cmpeq_pd(const m128d& a, const m128d& b) noexcept { return _mm_cmpeq_pd(a, b); }

inline m128 mm_cmpge_ss(const m128& a, const m128& b) noexcept { return _mm_cmpge_ss(a, b); }
inline m128 mm_cmpge_ps(const m128& a, const m128& b) noexcept { return _mm_cmpge_ps(a, b); }
inline m128d mm_cmpge_sd(const m128d& a, const m128d& b) noexcept { return _mm_cmpge_sd(a, b); }
inline m128d mm_cmpge_pd(const m128d& a, const m128d& b) noexcept { return _mm_cmpge_pd(a, b); }

inline m128i mm_cmpgt_epi8(const m128i& a, const m128i& b) noexcept { return _mm_cmpgt_epi8(a, b); }
inline m256i mm256_cmpgt_epi8(const m256i& a, const m256i& b) noexcept { return _mm256_cmpgt_epi8(a, b); }
inline m128i mm_cmpgt_epi16(const m128i& a, const m128i& b) noexcept { return _mm_cmpgt_epi16(a, b); }
inline m256i mm256_cmpgt_epi16(const m256i& a, const m256i& b) noexcept { return _mm256_cmpgt_epi16(a, b); }
inline m128i mm_cmpgt_epi32(const m128i& a, const m128i& b) noexcept { return _mm_cmpgt_epi32(a, b); }
inline m256i mm256_cmpgt_epi32(const m256i& a, const m256i& b) noexcept { return _mm256_cmpgt_epi32(a, b); }
inline m128i mm_cmpgt_epi64(const m128i& a, const m128i& b) noexcept { return _mm_cmpgt_epi64(a, b); }
inline m256i mm256_cmpgt_epi64(const m256i& a, const m256i& b) noexcept { return _mm256_cmpgt_epi64(a, b); }

inline m128 mm_cmpgt_ss(const m128& a, const m128& b) noexcept { return _mm_cmpgt_ss(a, b); }
inline m128 mm_cmpgt_ps(const m128& a, const m128& b) noexcept { return _mm_cmpgt_ps(a, b); }
inline m128d mm_cmpgt_sd(const m128d& a, const m128d& b) noexcept { return _mm_cmpgt_sd(a, b); }
inline m128d mm_cmpgt_pd(const m128d& a, const m128d& b) noexcept { return _mm_cmpgt_pd(a, b); }

inline m128 mm_cmple_ss(const m128& a, const m128& b) noexcept { return _mm_cmple_ss(a, b); }
inline m128 mm_cmple_ps(const m128& a, const m128& b) noexcept { return _mm_cmple_ps(a, b); }
inline m128d mm_cmple_sd(const m128d& a, const m128d& b) noexcept { return _mm_cmple_sd(a, b); }
inline m128d mm_cmple_pd(const m128d& a, const m128d& b) noexcept { return _mm_cmple_pd(a, b); }

inline m128i mm_cmplt_epi8(const m128i& a, const m128i& b) noexcept { return _mm_cmplt_epi8(a, b); }
inline m128i mm_cmplt_epi16(const m128i& a, const m128i& b) noexcept { return _mm_cmplt_epi16(a, b); }
inline m128i mm_cmplt_epi32(const m128i& a, const m128i& b) noexcept { return _mm_cmplt_epi32(a, b); }

inline m128 mm_cmplt_ss(const m128& a, const m128& b) noexcept { return _mm_cmplt_ss(a, b); }
inline m128 mm_cmplt_ps(const m128& a, const m128& b) noexcept { return _mm_cmplt_ps(a, b); }
inline m128d mm_cmplt_sd(const m128d& a, const m128d& b) noexcept { return _mm_cmplt_sd(a, b); }
inline m128d mm_cmplt_pd(const m128d& a, const m128d& b) noexcept { return _mm_cmplt_pd(a, b); }

inline m128 mm_cmpneq_ss(const m128& a, const m128& b) noexcept { return _mm_cmpneq_ss(a, b); }
inline m128 mm_cmpneq_ps(const m128& a, const m128& b) noexcept { return _mm_cmpneq_ps(a, b); }
inline m128d mm_cmpneq_sd(const m128d& a, const m128d& b) noexcept { return _mm_cmpneq_sd(a, b); }
inline m128d mm_cmpneq_pd(const m128d& a, const m128d& b) noexcept { return _mm_cmpneq_pd(a, b); }

inline m128 mm_cmpnge_ss(const m128& a, const m128& b) noexcept { return _mm_cmpnge_ss(a, b); }
inline m128 mm_cmpnge_ps(const m128& a, const m128& b) noexcept { return _mm_cmpnge_ps(a, b); }
inline m128d mm_cmpnge_sd(const m128d& a, const m128d& b) noexcept { return _mm_cmpnge_sd(a, b); }
inline m128d mm_cmpnge_pd(const m128d& a, const m128d& b) noexcept { return _mm_cmpnge_pd(a, b); }

inline m128 mm_cmpngt_ss(const m128& a, const m128& b) noexcept { return _mm_cmpngt_ss(a, b); }
inline m128 mm_cmpngt_ps(const m128& a, const m128& b) noexcept { return _mm_cmpngt_ps(a, b); }
inline m128d mm_cmpngt_sd(const m128d& a, const m128d& b) noexcept { return _mm_cmpngt_sd(a, b); }
inline m128d mm_cmpngt_pd(const m128d& a, const m128d& b) noexcept { return _mm_cmpngt_pd(a, b); }

inline m128 mm_cmpnle_ss(const m128& a, const m128& b) noexcept { return _mm_cmpnle_ss(a, b); }
inline m128 mm_cmpnle_ps(const m128& a, const m128& b) noexcept { return _mm_cmpnle_ps(a, b); }
inline m128d mm_cmpnle_sd(const m128d& a, const m128d& b) noexcept { return _mm_cmpnle_sd(a, b); }
inline m128d mm_cmpnle_pd(const m128d& a, const m128d& b) noexcept { return _mm_cmpnle_pd(a, b); }

inline m128 mm_cmpnlt_ss(const m128& a, const m128& b) noexcept { return _mm_cmpnlt_ss(a, b); }
inline m128 mm_cmpnlt_ps(const m128& a, const m128& b) noexcept { return _mm_cmpnlt_ps(a, b); }
inline m128d mm_cmpnlt_sd(const m128d& a, const m128d& b) noexcept { return _mm_cmpnlt_sd(a, b); }
inline m128d mm_cmpnlt_pd(const m128d& a, const m128d& b) noexcept { return _mm_cmpnlt_pd(a, b); }

inline m128 mm_cmpord_ss(const m128& a, const m128& b) noexcept { return _mm_cmpord_ss(a, b); }
inline m128 mm_cmpord_ps(const m128& a, const m128& b) noexcept { return _mm_cmpord_ps(a, b); }
inline m128d mm_cmpord_sd(const m128d& a, const m128d& b) noexcept { return _mm_cmpord_sd(a, b); }
inline m128d mm_cmpord_pd(const m128d& a, const m128d& b) noexcept { return _mm_cmpord_pd(a, b); }

inline m128 mm_cmpunord_ss(const m128& a, const m128& b) noexcept { return _mm_cmpunord_ss(a, b); }
inline m128 mm_cmpunord_ps(const m128& a, const m128& b) noexcept { return _mm_cmpunord_ps(a, b); }
inline m128d mm_cmpunord_sd(const m128d& a, const m128d& b) noexcept { return _mm_cmpunord_sd(a, b); }
inline m128d mm_cmpunord_pd(const m128d& a, const m128d& b) noexcept { return _mm_cmpunord_pd(a, b); }

inline int mm_comieq_ss(const m128& a, const m128& b) noexcept { return _mm_comieq_ss(a, b); }
inline int mm_comieq_sd(const m128d& a, const m128d& b) noexcept { return _mm_comieq_sd(a, b); }

inline int mm_comige_ss(const m128& a, const m128& b) noexcept { return _mm_comige_ss(a, b); }
inline int mm_comige_sd(const m128d& a, const m128d& b) noexcept { return _mm_comige_sd(a, b); }

inline int mm_comigt_ss(const m128& a, const m128& b) noexcept { return _mm_comigt_ss(a, b); }
inline int mm_comigt_sd(const m128d& a, const m128d& b) noexcept { return _mm_comigt_sd(a, b); }

inline int mm_comile_ss(const m128& a, const m128& b) noexcept { return _mm_comile_ss(a, b); }
inline int mm_comile_sd(const m128d& a, const m128d& b) noexcept { return _mm_comile_sd(a, b); }

inline int mm_comilt_ss(const m128& a, const m128& b) noexcept { return _mm_comilt_ss(a, b); }
inline int mm_comilt_sd(const m128d& a, const m128d& b) noexcept { return _mm_comilt_sd(a, b); }

inline int mm_comineq_ss(const m128& a, const m128& b) noexcept { return _mm_comineq_ss(a, b); }
inline int mm_comineq_sd(const m128d& a, const m128d& b) noexcept { return _mm_comineq_sd(a, b); }

inline int mm_ucomieq_ss(const m128& a, const m128& b) noexcept { return _mm_ucomieq_ss(a, b); }
inline int mm_ucomieq_sd(const m128d& a, const m128d& b) noexcept { return _mm_ucomieq_sd(a, b); }

inline int mm_ucomige_ss(const m128& a, const m128& b) noexcept { return _mm_ucomige_ss(a, b); }
inline int mm_ucomige_sd(const m128d& a, const m128d& b) noexcept { return _mm_ucomige_sd(a, b); }

inline int mm_ucomigt_ss(const m128& a, const m128& b) noexcept { return _mm_ucomigt_ss(a, b); }
inline int mm_ucomigt_sd(const m128d& a, const m128d& b) noexcept { return _mm_ucomigt_sd(a, b); }

inline int mm_ucomile_ss(const m128& a, const m128& b) noexcept { return _mm_ucomile_ss(a, b); }
inline int mm_ucomile_sd(const m128d& a, const m128d& b) noexcept { return _mm_ucomile_sd(a, b); }

inline int mm_ucomilt_ss(const m128& a, const m128& b) noexcept { return _mm_ucomilt_ss(a, b); }
inline int mm_ucomilt_sd(const m128d& a, const m128d& b) noexcept { return _mm_ucomilt_sd(a, b); }

inline int mm_ucomineq_ss(const m128& a, const m128& b) noexcept { return _mm_ucomineq_ss(a, b); }
inline int mm_ucomineq_sd(const m128d& a, const m128d& b) noexcept { return _mm_ucomineq_sd(a, b); }

inline m128 mm_cvt_si2ss(const m128& a, const int b) noexcept { return _mm_cvt_si2ss(a, b); }
inline int mm_cvt_ss2si(const m128& a) noexcept { return _mm_cvt_ss2si(a); }

inline m128i mm_cvtepi8_epi16(const m128i& a) noexcept { return _mm_cvtepi8_epi16(a); }
inline m256i mm256_cvtepi8_epi16(const m128i& a) noexcept { return _mm256_cvtepi8_epi16(a); }
inline m128i mm_cvtepi8_epi32(const m128i& a) noexcept { return _mm_cvtepi8_epi32(a); }
inline m256i mm256_cvtepi8_epi32(const m128i& a) noexcept { return _mm256_cvtepi8_epi32(a); }
inline m128i mm_cvtepi8_epi64(const m128i& a) noexcept { return _mm_cvtepi8_epi64(a); }
inline m256i mm256_cvtepi8_epi64(const m128i& a) noexcept { return _mm256_cvtepi8_epi64(a); }

inline m128i mm_cvtepi16_epi32(const m128i& a) noexcept { return _mm_cvtepi16_epi32(a); }
inline m256i mm256_cvtepi16_epi32(const m128i& a) noexcept { return _mm256_cvtepi16_epi32(a); }
inline m128i mm_cvtepi16_epi64(const m128i& a) noexcept { return _mm_cvtepi16_epi64(a); }
inline m256i mm256_cvtepi16_epi64(const m128i& a) noexcept { return _mm256_cvtepi16_epi64(a); }

inline m128i mm_cvtepi32_epi64(const m128i& a) noexcept { return _mm_cvtepi32_epi64(a); }
inline m256i mm256_cvtepi32_epi64(const m128i& a) noexcept { return _mm256_cvtepi32_epi64(a); }
inline m128 mm_cvtepi32_ps(const m128i& a) noexcept { return _mm_cvtepi32_ps(a); }
inline m256 mm256_cvtepi32_ps(const m256i& a) noexcept { return _mm256_cvtepi32_ps(a); }
inline m128d mm_cvtepi32_pd(const m128i& a) noexcept { return _mm_cvtepi32_pd(a); }
inline m256d mm256_cvtepi32_pd(const m128i& a) noexcept { return _mm256_cvtepi32_pd(a); }

inline m128i mm_cvtepu8_epi16(const m128i& a) noexcept { return _mm_cvtepu8_epi16(a); }
inline m256i mm256_cvtepu8_epi16(const m128i& a) noexcept { return _mm256_cvtepu8_epi16(a); }
inline m128i mm_cvtepu8_epi32(const m128i& a) noexcept { return _mm_cvtepu8_epi32(a); }
inline m256i mm256_cvtepu8_epi32(const m128i& a) noexcept { return _mm256_cvtepu8_epi32(a); }
inline m128i mm_cvtepu8_epi64(const m128i& a) noexcept { return _mm_cvtepu8_epi64(a); }
inline m256i mm256_cvtepu8_epi64(const m128i& a) noexcept { return _mm256_cvtepu8_epi64(a); }

inline m128i mm_cvtepu16_epi32(const m128i& a) noexcept { return _mm_cvtepu16_epi32(a); }
inline m256i mm256_cvtepu16_epi32(const m128i& a) noexcept { return _mm256_cvtepu16_epi32(a); }
inline m128i mm_cvtepu16_epi64(const m128i& a) noexcept { return _mm_cvtepu16_epi64(a); }
inline m256i mm256_cvtepu16_epi64(const m128i& a) noexcept { return _mm256_cvtepu16_epi64(a); }

inline m128i mm_cvtepu32_epi64(const m128i& a) noexcept { return _mm_cvtepu32_epi64(a); }
inline m256i mm256_cvtepu32_epi64(const m128i& a) noexcept { return _mm256_cvtepu32_epi64(a); }
inline m128 mm_cvtepu32_ps(const m128i& a) noexcept { return _mm_cvtepu32_ps(a); }
inline m256 mm256_cvtepu32_ps(const m256i& a) noexcept { return _mm256_cvtepu32_ps(a); }
inline m128d mm_cvtepu32_pd(const m128i& a) noexcept { return _mm_cvtepu32_pd(a); }
inline m256d mm256_cvtepu32_pd(const m128i& a) noexcept { return _mm256_cvtepu32_pd(a); }

inline m128i mm_cvtps_epi32(const m128& a) noexcept { return _mm_cvtps_epi32(a); }
inline m256i mm256_cvtps_epi32(const m256& a) noexcept { return _mm256_cvtps_epi32(a); }
inline m128d mm_cvtps_pd(const m128& a) noexcept { return _mm_cvtps_pd(a); }
inline m256d mm256_cvtps_pd(const m128& a) noexcept { return _mm256_cvtps_pd(a); }

inline m128i mm_cvtpd_epi32(const m128d& a) noexcept { return _mm_cvtpd_epi32(a); }
inline m128i mm256_cvtpd_epi32(const m256d& a) noexcept { return _mm256_cvtpd_epi32(a); }
inline m128 mm_cvtpd_ps(const m128d& a) noexcept { return _mm_cvtpd_ps(a); }
inline m128 mm256_cvtpd_ps(const m256d& a) noexcept { return _mm256_cvtpd_ps(a); }

inline int mm_cvtsi128_si32(const m128i& a) noexcept { return _mm_cvtsi128_si32(a); }
inline int mm256_cvtsi256_si32(const m256i& a) noexcept { return _mm256_cvtsi256_si32(a); }
inline long long mm_cvtsi128_si64(const m128i& a) noexcept { return _mm_cvtsi128_si64(a); }
inline long long mm_cvtsi128_si64x(const m128i& a) noexcept { return _mm_cvtsi128_si64(a); }

inline int mm_cvtss_si32(const m128& a) noexcept { return _mm_cvtss_si32(a); }
inline long long mm_cvtss_si64(const m128& a) noexcept { return _mm_cvtss_si64(a); }
inline float mm_cvtss_f32(const m128& a) noexcept { return _mm_cvtss_f32(a); }
inline int mm_cvtsd_si32(const m128d& a) noexcept { return _mm_cvtsd_si32(a); }
inline long long mm_cvtsd_si64(const m128d& a) noexcept { return _mm_cvtsd_si64(a); }
inline double mm_cvtsd_f64(const m128d& a) noexcept { return _mm_cvtsd_f64(a); }

inline m128i mm_cvtsi32_si128(const int a) noexcept { return _mm_cvtsi32_si128(a); }
inline m128 mm_cvtsi32_ss(const m128& a, const int b) noexcept { return _mm_cvtsi32_ss(a, b); }
inline m128d mm_cvtsi32_sd(const m128d& a, const int b) noexcept { return _mm_cvtsi32_sd(a, b); }

inline m128 mm_cvtsi64_ss(const m128& a, const long long b) noexcept { return _mm_cvtsi64_ss(a, b); }
inline m128d mm_cvtsi64_sd(const m128d& a, const long long b) noexcept { return _mm_cvtsi64_sd(a, b); }
inline m128i mm_cvtsi64_si128(const long long a) noexcept { return _mm_cvtsi64_si128(a); }
inline m128 mm_cvtsi64x_ss(const m128& a, const long long b) noexcept { return _mm_cvtsi64_ss(a, b); }
inline m128d mm_cvtsi64x_sd(const m128d& a, const long long b) noexcept { return _mm_cvtsi64_sd(a, b); }
inline m128i mm_cvtsi64x_si128(const long long a) noexcept { return _mm_cvtsi64_si128(a); }

inline m128i mm_cvttps_epi32(const m128& a) noexcept { return _mm_cvttps_epi32(a); }
inline m256i mm256_cvttps_epi32(const m256& a) noexcept { return _mm256_cvttps_epi32(a); }
inline m128i mm_cvttpd_epi32(const m128d& a) noexcept { return _mm_cvttpd_epi32(a); }
inline m128i mm256_cvttpd_epi32(const m256d& a) noexcept { return _mm256_cvttpd_epi32(a); }
inline int mm_cvttsd_si32(const m128d& a) noexcept { return _mm_cvttsd_si32(a); }
inline long long mm_cvttsd_si64(const m128d& a) noexcept { return _mm_cvttsd_si64(a); }
inline long long mm_cvttsd_si64x(const m128d& a) noexcept { return _mm_cvttsd_si64(a); }
inline int mm_cvttss_si32(const m128& a) noexcept { return _mm_cvttss_si32(a); }
inline long long mm_cvttss_si64(const m128& a) noexcept { return _mm_cvttss_si64(a); }

inline unsigned mm_crc32_u8(const unsigned crc, const unsigned char v) noexcept { return _mm_crc32_u8(crc, v); }
inline unsigned mm_crc32_u16(const unsigned crc, const unsigned short v) noexcept { return _mm_crc32_u16(crc, v); }
inline unsigned mm_crc32_u32(const unsigned crc, const unsigned v) noexcept { return _mm_crc32_u32(crc, v); }
inline unsigned long long mm_crc32_u64(const unsigned long long crc, const unsigned long long v) noexcept { return _mm_crc32_u64(crc, v); }

inline m128 mm_cbrt_ps(const m128& a) noexcept { return _mm_cbrt_ps(a); }
inline m256 mm256_cbrt_ps(const m256& a) noexcept { return _mm256_cbrt_ps(a); }
inline m128d mm_cbrt_pd(const m128d& a) noexcept { return _mm_cbrt_pd(a); }
inline m256d mm256_cbrt_pd(const m256d& a) noexcept { return _mm256_cbrt_pd(a); }

inline m128 mm_hypot_ps(const m128& a, const m128& b) noexcept { return _mm_hypot_ps(a, b); }
inline m256 mm256_hypot_ps(const m256& a, const m256& b) noexcept { return _mm256_hypot_ps(a, b); }
inline m128d mm_hypot_pd(const m128d& a, const m128d& b) noexcept { return _mm_hypot_pd(a, b); }
inline m256d mm256_hypot_pd(const m256d& a, const m256d& b) noexcept { return _mm256_hypot_pd(a, b); }

inline m128 mm_cexp_ps(const m128& a) noexcept { return _mm_cexp_ps(a); }
inline m256 mm256_cexp_ps(const m256& a) noexcept { return _mm256_cexp_ps(a); }

inline m128 mm_clog_ps(const m128& a) noexcept { return _mm_clog_ps(a); }
inline m256 mm256_clog_ps(const m256& a) noexcept { return _mm256_clog_ps(a); }

inline m128 mm_csqrt_ps(const m128& a) noexcept { return _mm_csqrt_ps(a); }
inline m256 mm256_csqrt_ps(const m256& a) noexcept { return _mm256_csqrt_ps(a); }

inline m128 mm_exp_ps(const m128& a) noexcept { return _mm_exp_ps(a); }
inline m256 mm256_exp_ps(const m256& a) noexcept { return _mm256_exp_ps(a); }
inline m128d mm_exp_pd(const m128d& a) noexcept { return _mm_exp_pd(a); }
inline m256d mm256_exp_pd(const m256d& a) noexcept { return _mm256_exp_pd(a); }

inline m128 mm_exp10_ps(const m128& a) noexcept { return _mm_exp10_ps(a); }
inline m256 mm256_exp10_ps(const m256& a) noexcept { return _mm256_exp10_ps(a); }
inline m128d mm_exp10_pd(const m128d& a) noexcept { return _mm_exp10_pd(a); }
inline m256d mm256_exp10_pd(const m256d& a) noexcept { return _mm256_exp10_pd(a); }

inline m128 mm_exp2_ps(const m128& a) noexcept { return _mm_exp2_ps(a); }
inline m256 mm256_exp2_ps(const m256& a) noexcept { return _mm256_exp2_ps(a); }
inline m128d mm_exp2_pd(const m128d& a) noexcept { return _mm_exp2_pd(a); }
inline m256d mm256_exp2_pd(const m256d& a) noexcept { return _mm256_exp2_pd(a); }

inline m128 mm_expm1_ps(const m128& a) noexcept { return _mm_expm1_ps(a); }
inline m256 mm256_expm1_ps(const m256& a) noexcept { return _mm256_expm1_ps(a); }
inline m128d mm_expm1_pd(const m128d& a) noexcept { return _mm_expm1_pd(a); }
inline m256d mm256_expm1_pd(const m256d& a) noexcept { return _mm256_expm1_pd(a); }

inline m128 mm_invcbrt_ps(const m128& a) noexcept { return _mm_invcbrt_ps(a); }
inline m256 mm256_invcbrt_ps(const m256& a) noexcept { return _mm256_invcbrt_ps(a); }
inline m128d mm_invcbrt_pd(const m128d& a) noexcept { return _mm_invcbrt_pd(a); }
inline m256d mm256_invcbrt_pd(const m256d& a) noexcept { return _mm256_invcbrt_pd(a); }

inline m128 mm_invsqrt_ps(const m128& a) noexcept { return _mm_invsqrt_ps(a); }
inline m256 mm256_invsqrt_ps(const m256& a) noexcept { return _mm256_invsqrt_ps(a); }
inline m128d mm_invsqrt_pd(const m128d& a) noexcept { return _mm_invsqrt_pd(a); }
inline m256d mm256_invsqrt_pd(const m256d& a) noexcept { return _mm256_invsqrt_pd(a); }

inline m128 mm_log_ps(const m128& a) noexcept { return _mm_log_ps(a); }
inline m256 mm256_log_ps(const m256& a) noexcept { return _mm256_log_ps(a); }
inline m128d mm_log_pd(const m128d& a) noexcept { return _mm_log_pd(a); }
inline m256d mm256_log_pd(const m256d& a) noexcept { return _mm256_log_pd(a); }

inline m128 mm_log10_ps(const m128& a) noexcept { return _mm_log10_ps(a); }
inline m256 mm256_log10_ps(const m256& a) noexcept { return _mm256_log10_ps(a); }
inline m128d mm_log10_pd(const m128d& a) noexcept { return _mm_log10_pd(a); }
inline m256d mm256_log10_pd(const m256d& a) noexcept { return _mm256_log10_pd(a); }

inline m128 mm_log1p_ps(const m128& a) noexcept { return _mm_log1p_ps(a); }
inline m256 mm256_log1p_ps(const m256& a) noexcept { return _mm256_log1p_ps(a); }
inline m128d mm_log1p_pd(const m128d& a) noexcept { return _mm_log1p_pd(a); }
inline m256d mm256_log1p_pd(const m256d& a) noexcept { return _mm256_log1p_pd(a); }

inline m128 mm_log2_ps(const m128& a) noexcept { return _mm_log2_ps(a); }
inline m256 mm256_log2_ps(const m256& a) noexcept { return _mm256_log2_ps(a); }
inline m128d mm_log2_pd(const m128d& a) noexcept { return _mm_log2_pd(a); }
inline m256d mm256_log2_pd(const m256d& a) noexcept { return _mm256_log2_pd(a); }

inline m128 mm_logb_ps(const m128& a) noexcept { return _mm_logb_ps(a); }
inline m256 mm256_logb_ps(const m256& a) noexcept { return _mm256_logb_ps(a); }
inline m128d mm_logb_pd(const m128d& a) noexcept { return _mm_logb_pd(a); }
inline m256d mm256_logb_pd(const m256d& a) noexcept { return _mm256_logb_pd(a); }

inline m128 mm_pow_ps(const m128& a, const m128& b) noexcept { return _mm_pow_ps(a, b); }
inline m256 mm256_pow_ps(const m256& a, const m256& b) noexcept { return _mm256_pow_ps(a, b); }
inline m128d mm_pow_pd(const m128d& a, const m128d& b) noexcept { return _mm_pow_pd(a, b); }
inline m256d mm256_pow_pd(const m256d& a, const m256d& b) noexcept { return _mm256_pow_pd(a, b); }

inline m128 mm_rcp_ps(const m128& a) noexcept { return _mm_rcp_ps(a); }
inline m256 mm256_rcp_ps(const m256& a) noexcept { return _mm256_rcp_ps(a); }
inline m128 mm_rcp_ss(const m128& a) noexcept { return _mm_rcp_ss(a); }

inline m128 mm_rsqrt_ps(const m128& a) noexcept { return _mm_rsqrt_ps(a); }
inline m256 mm256_rsqrt_ps(const m256& a) noexcept { return _mm256_rsqrt_ps(a); }
inline m128 mm_rsqrt_ss(const m128& a) noexcept { return _mm_rsqrt_ss(a); }

inline m128 mm_sqrt_ss(const m128& a) noexcept { return _mm_sqrt_ss(a); }
inline m128 mm_sqrt_ps(const m128& a) noexcept { return _mm_sqrt_ps(a); }
inline m256 mm256_sqrt_ps(const m256& a) noexcept { return _mm256_sqrt_ps(a); }
inline m128d mm_sqrt_sd(const m128d& a, const m128d& b) noexcept { return _mm_sqrt_sd(a, b); }
inline m128d mm_sqrt_pd(const m128d& a) noexcept { return _mm_sqrt_pd(a); }
inline m256d mm256_sqrt_pd(const m256d& a) noexcept { return _mm256_sqrt_pd(a); }

inline m128 mm_svml_sqrt_ps(const m128& a) noexcept { return _mm_svml_sqrt_ps(a); }
inline m256 mm256_svml_sqrt_ps(const m256& a) noexcept { return _mm256_svml_sqrt_ps(a); }
inline m128d mm_svml_sqrt_pd(const m128d& a) noexcept { return _mm_svml_sqrt_pd(a); }
inline m256d mm256_svml_sqrt_pd(const m256d& a) noexcept { return _mm256_svml_sqrt_pd(a); }

inline m128i mm_broadcastb_epi8(const m128i& a) noexcept { return _mm_broadcastb_epi8(a); }
inline m256i mm256_broadcastb_epi8(const m128i& a) noexcept { return _mm256_broadcastb_epi8(a); }

inline m128i mm_broadcastw_epi16(const m128i& a) noexcept { return _mm_broadcastw_epi16(a); }
inline m256i mm256_broadcastw_epi16(const m128i& a) noexcept { return _mm256_broadcastw_epi16(a); }

inline m128i mm_broadcastd_epi32(const m128i& a) noexcept { return _mm_broadcastd_epi32(a); }
inline m256i mm256_broadcastd_epi32(const m128i& a) noexcept { return _mm256_broadcastd_epi32(a); }

inline m128i mm_broadcastq_epi64(const m128i& a) noexcept { return _mm_broadcastq_epi64(a); }
inline m256i mm256_broadcastq_epi64(const m128i& a) noexcept { return _mm256_broadcastq_epi64(a); }

inline m128 mm_broadcastss_ps(const m128& a) noexcept { return _mm_broadcastss_ps(a); }
inline m256 mm256_broadcastss_ps(const m128& a) noexcept { return _mm256_broadcastss_ps(a); }

inline m128d mm_broadcastsd_pd(const m128d& a) noexcept { return _mm_broadcastsd_pd(a); }
inline m256d mm256_broadcastsd_pd(const m128d& a) noexcept { return _mm256_broadcastsd_pd(a); }

inline m128 mm_broadcast_ss(const float* a) noexcept { return _mm_broadcast_ss(a); }
inline m256 mm256_broadcast_ps(const m128* a) noexcept { return _mm256_broadcast_ps(a); }
inline m256d mm256_broadcast_sd(const double* a) noexcept { return _mm256_broadcast_sd(a); }
inline m256d mm256_broadcast_pd(const m128d* a) noexcept { return _mm256_broadcast_pd(a); }

inline m256i mm_broadcastsi128_si256(const m128i& a) noexcept { return _mm256_broadcastsi128_si256(a); }
inline m256i mm256_broadcastsi128_si256(const m128i& a) noexcept { return _mm256_broadcastsi128_si256(a); }

template<int imm8> m128i mm_i32gather_epi32(const int* ptr, const m128i& vindex) noexcept { return _mm_i32gather_epi32(ptr, vindex, imm8); }
template<int imm8> m128i mm_mask_i32gather_epi32(const m128i& src, const int* ptr, const m128i& vindex, const m128i& mask) noexcept { return _mm_mask_i32gather_epi32(src, ptr, vindex, mask, imm8); }
template<int imm8> m256i mm256_i32gather_epi32(const int* ptr, const m256i& vindex) noexcept { return _mm256_i32gather_epi32(ptr, vindex, imm8); }
template<int imm8> m256i mm256_mask_i32gather_epi32(const m256i& src, const int* ptr, const m256i& vindex, const m256i& mask) noexcept { return _mm256_mask_i32gather_epi32(src, ptr, vindex, mask, imm8); }

template<int imm8> m128i mm_i32gather_epi64(const long long* ptr, const m128i& vindex) noexcept { return _mm_i32gather_epi64(ptr, vindex, imm8); }
template<int imm8> m128i mm_mask_i32gather_epi64(const m128i& src, const long long* ptr, const m128i& vindex, const m128i& mask) noexcept { return _mm_mask_i32gather_epi64(src, ptr, vindex, mask, imm8); }
template<int imm8> m256i mm256_i32gather_epi64(const long long* ptr, const m128i& vindex) noexcept { return _mm256_i32gather_epi64(ptr, vindex, imm8); }
template<int imm8> m256i mm256_mask_i32gather_epi64(const m256i& src, const long long* ptr, const m128i& vindex, const m256i& mask) noexcept { return _mm256_mask_i32gather_epi64(src, ptr, vindex, mask, imm8); }

template<int imm8> m128 mm_i32gather_ps(const float* ptr, const m128i& vindex) noexcept { return _mm_i32gather_ps(ptr, vindex, imm8); }
template<int imm8> m128 mm_mask_i32gather_ps(const m128& src, const float* ptr, const m128i& vindex, const m128& mask) noexcept { return _mm_mask_i32gather_ps(src, ptr, vindex, mask, imm8); }
template<int imm8> m256 mm256_i32gather_ps(const float* ptr, const m256i& vindex) noexcept { return _mm256_i32gather_ps(ptr, vindex, imm8); }
template<int imm8> m256 mm256_mask_i32gather_ps(const m256& src, const float* ptr, const m256i& vindex, const m256& mask) noexcept { return _mm256_mask_i32gather_ps(src, ptr, vindex, mask, imm8); }

template<int imm8> m128d mm_i32gather_pd(const double* ptr, const m128i& vindex) noexcept { return _mm_i32gather_pd(ptr, vindex, imm8); }
template<int imm8> m128d mm_mask_i32gather_pd(const m128d& src, const double* ptr, const m128i& vindex, const m128d& mask) noexcept { return _mm_mask_i32gather_pd(src, ptr, vindex, mask, imm8); }
template<int imm8> m256d mm256_i32gather_pd(const double* ptr, const m128i& vindex) noexcept { return _mm256_i32gather_pd(ptr, vindex, imm8); }
template<int imm8> m256d mm256_mask_i32gather_pd(const m256d& src, const double* ptr, const m128i& vindex, const m256d& mask) noexcept { return _mm256_mask_i32gather_pd(src, ptr, vindex, mask, imm8); }

template<int imm8> m128i mm_i64gather_epi32(const int* ptr, const m128i& vindex) noexcept { return _mm_i64gather_epi32(ptr, vindex, imm8); }
template<int imm8> m128i mm_mask_i64gather_epi32(const m128i& src, const int* ptr, const m128i& vindex, const m128i& mask) noexcept { return _mm_mask_i64gather_epi32(src, ptr, vindex, mask, imm8); }
template<int imm8> m256i mm256_i64gather_epi32(const int* ptr, const m256i& vindex) noexcept { return _mm256_i64gather_epi32(ptr, vindex, imm8); }
template<int imm8> m256i mm256_mask_i64gather_epi32(const m128i& src, const int* ptr, const m256i& vindex, const m128i& mask) noexcept { return _mm256_mask_i64gather_epi32(src, ptr, vindex, mask, imm8); }

template<int imm8> m128i mm_i64gather_epi64(const long long* ptr, const m128i& vindex) noexcept { return _mm_i64gather_epi64(ptr, vindex, imm8); }
template<int imm8> m128i mm_mask_i64gather_epi64(const m128i& src, const long long* ptr, const m128i& vindex, const m128i& mask) noexcept { return _mm_mask_i64gather_epi64(src, ptr, vindex, mask, imm8); }
template<int imm8> m256i mm256_i64gather_epi64(const long long* ptr, const m256i& vindex) noexcept { return _mm256_i64gather_epi64(ptr, vindex, imm8); }
template<int imm8> m256i mm256_mask_i64gather_epi64(const m256i& src, const long long* ptr, const m256i& vindex, const m256i& mask) noexcept { return _mm256_mask_i64gather_epi64(src, ptr, vindex, mask, imm8); }

template<int imm8> m128 mm_i64gather_ps(const float* ptr, const m128i& vindex) noexcept { return _mm_i64gather_ps(ptr, vindex, imm8); }
template<int imm8> m128 mm_mask_i64gather_ps(const m128& src, const float* ptr, const m128i& vindex, const m128& mask) noexcept { return _mm_mask_i64gather_ps(src, ptr, vindex, mask, imm8); }
template<int imm8> m128 mm256_i64gather_ps(const float* ptr, const m256i& vindex) noexcept { return _mm256_i64gather_ps(ptr, vindex, imm8); }
template<int imm8> m128 mm256_mask_i64gather_ps(const m128& src, const float* ptr, const m256i& vindex, const m128& mask) noexcept { return _mm256_mask_i64gather_ps(src, ptr, vindex, mask, imm8); }

template<int imm8> m128d mm_i64gather_pd(const double* ptr, const m128i& vindex) noexcept { return _mm_i64gather_pd(ptr, vindex, imm8); }
template<int imm8> m128d mm_mask_i64gather_pd(const m128d& src, const double* ptr, const m128i& vindex, const m128d& mask) noexcept { return _mm_mask_i64gather_pd(src, ptr, vindex, mask, imm8); }
template<int imm8> m256d mm256_i64gather_pd(const double* ptr, const m256i& vindex) noexcept { return _mm256_i64gather_pd(ptr, vindex, imm8); }
template<int imm8> m256d mm256_mask_i64gather_pd(const m256d& src, const double* ptr, const m256i& vindex, const m256d& mask) noexcept { return _mm256_mask_i64gather_pd(src, ptr, vindex, mask, imm8); }

inline m128 mm_load_ss(const float* mem_addr) noexcept { return _mm_load_ss(mem_addr); }
inline m128 mm_load_ps(const float* mem_addr) noexcept { return _mm_load_ps(mem_addr); }
inline m256 mm256_load_ps(const float* mem_addr) noexcept { return _mm256_load_ps(mem_addr); }
inline m128d mm_load_sd(const double* mem_addr) noexcept { return _mm_load_sd(mem_addr); }
inline m128d mm_load_pd(const double* mem_addr) noexcept { return _mm_load_pd(mem_addr); }
inline m256d mm256_load_pd(const double* mem_addr) noexcept { return _mm256_load_pd(mem_addr); }
inline m128i mm_load_si128(const m128i* mem_addr) noexcept { return _mm_load_si128(mem_addr); }
inline m256i mm256_load_si256(const m256i* mem_addr) noexcept { return _mm256_load_si256(mem_addr); }

inline m128 mm_load_ps1(const float* mem_addr) noexcept { return _mm_load_ps1(mem_addr); }
inline m128 mm_load1_ps(const float* mem_addr) noexcept { return _mm_load1_ps(mem_addr); }
inline m128d mm_load_pd1(const double* mem_addr) noexcept { return _mm_load_pd1(mem_addr); }
inline m128d mm_load1_pd(const double* mem_addr) noexcept { return _mm_load1_pd(mem_addr); }

inline m128d mm_loaddup_pd(const double* mem_addr) noexcept { return _mm_loaddup_pd(mem_addr); }
inline m128d mm_loadh_pd(const m128d& a, const double* mem_addr) noexcept { return _mm_loadh_pd(a, mem_addr); }
inline m128i mm_loadl_epi64(const m128i* mem_addr) noexcept { return _mm_loadl_epi64(mem_addr); }
inline m128d mm_loadl_pd(const m128d& a, const double* mem_addr) noexcept { return _mm_loadl_pd(a, mem_addr); }

inline m128 mm_loadr_ps(const float* mem_addr) noexcept { return _mm_loadr_ps(mem_addr); }
inline m128d mm_loadr_pd(const double* mem_addr) noexcept { return _mm_loadr_pd(mem_addr); }

inline m128 mm_loadu_ps(const float* mem_addr) noexcept { return _mm_loadu_ps(mem_addr); }
inline m256 mm256_loadu_ps(const float* mem_addr) noexcept { return _mm256_loadu_ps(mem_addr); }
inline m128d mm_loadu_pd(const double* mem_addr) noexcept { return _mm_loadu_pd(mem_addr); }
inline m256d mm256_loadu_pd(const double* mem_addr) noexcept { return _mm256_loadu_pd(mem_addr); }
inline m128i mm_loadu_si16(const void* mem_addr) noexcept { return _mm_loadu_si16(mem_addr); }
inline m128i mm_loadu_si32(const void* mem_addr) noexcept { return _mm_loadu_si32(mem_addr); }
inline m128i mm_loadu_si64(const void* mem_addr) noexcept { return _mm_loadu_si64(mem_addr); }
inline m128i mm_loadu_si128(const m128i* mem_addr) noexcept { return _mm_loadu_si128(mem_addr); }
inline m256i mm256_loadu_si256(const m256i* mem_addr) noexcept { return _mm256_loadu_si256(mem_addr); }

inline m256 mm256_loadu2_m128(const float* hiaddr, const float* loaddr) noexcept { return _mm256_loadu2_m128(hiaddr, loaddr); }
inline m256d mm256_loadu2_m128d(const double* hiaddr, const double* loaddr) noexcept { return _mm256_loadu2_m128d(hiaddr, loaddr); }
inline m256i mm256_loadu2_m128i(const m128i* hiaddr, const m128i* loaddr) noexcept { return _mm256_loadu2_m128i(hiaddr, loaddr); }

inline m128i mm_maskload_epi32(const int* mem_addr, const m128i& mask) noexcept { return _mm_maskload_epi32(mem_addr, mask); }
inline m256i mm256_maskload_epi32(const int* mem_addr, const m256i& mask) noexcept { return _mm256_maskload_epi32(mem_addr, mask); }
inline m128i mm_maskload_epi64(const long long* mem_addr, const m128i& mask) noexcept { return _mm_maskload_epi64(mem_addr, mask); }
inline m256i mm256_maskload_epi64(const long long* mem_addr, const m256i& mask) noexcept { return _mm256_maskload_epi64(mem_addr, mask); }
inline m128 mm_maskload_ps(const float* mem_addr, const m128i& mask) noexcept { return _mm_maskload_ps(mem_addr, mask); }
inline m256 mm256_maskload_ps(const float* mem_addr, const m256i& mask) noexcept { return _mm256_maskload_ps(mem_addr, mask); }
inline m128d mm_maskload_pd(const double* mem_addr, const m128i& mask) noexcept { return _mm_maskload_pd(mem_addr, mask); }
inline m256d mm256_maskload_pd(const double* mem_addr, const m256i& mask) noexcept { return _mm256_maskload_pd(mem_addr, mask); }

inline m128i mm_stream_load_si128(const m128i* mem_addr) noexcept { return _mm_stream_load_si128(mem_addr); }
inline m256i mm256_stream_load_si256(const m256i* mem_addr) noexcept { return _mm256_stream_load_si256(mem_addr); }

inline m128 mm_and_ps(const m128& a, const m128& b) noexcept { return _mm_and_ps(a, b); }
inline m256 mm256_and_ps(const m256& a, const m256& b) noexcept { return _mm256_and_ps(a, b); }
inline m128d mm_and_pd(const m128d& a, const m128d& b) noexcept { return _mm_and_pd(a, b); }
inline m256d mm256_and_pd(const m256d& a, const m256d& b) noexcept { return _mm256_and_pd(a, b); }
inline m128i mm_and_si128(const m128i& a, const m128i& b) noexcept { return _mm_and_si128(a, b); }
inline m256i mm256_and_si256(const m256i& a, const m256i& b) noexcept { return _mm256_and_si256(a, b); }

inline m128 mm_andnot_ps(const m128& a, const m128& b) noexcept { return _mm_andnot_ps(a, b); }
inline m256 mm256_andnot_ps(const m256& a, const m256& b) noexcept { return _mm256_andnot_ps(a, b); }
inline m128d mm_andnot_pd(const m128d& a, const m128d& b) noexcept { return _mm_andnot_pd(a, b); }
inline m256d mm256_andnot_pd(const m256d& a, const m256d& b) noexcept { return _mm256_andnot_pd(a, b); }
inline m128i mm_andnot_si128(const m128i& a, const m128i& b) noexcept { return _mm_andnot_si128(a, b); }
inline m256i mm256_andnot_si256(const m256i& a, const m256i& b) noexcept { return _mm256_andnot_si256(a, b); }

inline m128 mm_or_ps(const m128& a, const m128& b) noexcept { return _mm_or_ps(a, b); }
inline m256 mm256_or_ps(const m256& a, const m256& b) noexcept { return _mm256_or_ps(a, b); }
inline m128d mm_or_pd(const m128d& a, const m128d& b) noexcept { return _mm_or_pd(a, b); }
inline m256d mm256_or_pd(const m256d& a, const m256d& b) noexcept { return _mm256_or_pd(a, b); }
inline m128i mm_or_si128(const m128i& a, const m128i& b) noexcept { return _mm_or_si128(a, b); }
inline m256i mm256_or_si256(const m256i& a, const m256i& b) noexcept { return _mm256_or_si256(a, b); }

inline m128 mm_xor_ps(const m128& a, const m128& b) noexcept { return _mm_xor_ps(a, b); }
inline m256 mm256_xor_ps(const m256& a, const m256& b) noexcept { return _mm256_xor_ps(a, b); }
inline m128d mm_xor_pd(const m128d& a, const m128d& b) noexcept { return _mm_xor_pd(a, b); }
inline m256d mm256_xor_pd(const m256d& a, const m256d& b) noexcept { return _mm256_xor_pd(a, b); }
inline m128i mm_xor_si128(const m128i& a, const m128i& b) noexcept { return _mm_xor_si128(a, b); }
inline m256i mm256_xor_si256(const m256i& a, const m256i& b) noexcept { return _mm256_xor_si256(a, b); }

inline int mm_test_all_ones(const m128i& a) noexcept { return _mm_test_all_ones(a); }
inline int mm_test_all_zeros(const m128i& mask, const m128i& a) noexcept { return _mm_test_all_zeros(mask, a); }
inline int mm_test_mix_ones_zeros(const m128i& mask, const m128i& a) noexcept { return _mm_test_mix_ones_zeros(mask, a); }

inline int mm_testc_ps(const m128& a, const m128& b) noexcept { return _mm_testc_ps(a, b); }
inline int mm256_testc_ps(const m256& a, const m256& b) noexcept { return _mm256_testc_ps(a, b); }
inline int mm_testc_pd(const m128d& a, const m128d& b) noexcept { return _mm_testc_pd(a, b); }
inline int mm256_testc_pd(const m256d& a, const m256d& b) noexcept { return _mm256_testc_pd(a, b); }
inline int mm_testc_si128(const m128i& a, const m128i& b) noexcept { return _mm_testc_si128(a, b); }
inline int mm256_testc_si256(const m256i& a, const m256i& b) noexcept { return _mm256_testc_si256(a, b); }

inline int mm_testnzc_ps(const m128& a, const m128& b) noexcept { return _mm_testnzc_ps(a, b); }
inline int mm256_testnzc_ps(const m256& a, const m256& b) noexcept { return _mm256_testnzc_ps(a, b); }
inline int mm_testnzc_pd(const m128d& a, const m128d& b) noexcept { return _mm_testnzc_pd(a, b); }
inline int mm256_testnzc_pd(const m256d& a, const m256d& b) noexcept { return _mm256_testnzc_pd(a, b); }
inline int mm_testnzc_si128(const m128i& a, const m128i& b) noexcept { return _mm_testnzc_si128(a, b); }
inline int mm256_testnzc_si256(const m256i& a, const m256i& b) noexcept { return _mm256_testnzc_si256(a, b); }

inline int mm_testz_ps(const m128& a, const m128& b) noexcept { return _mm_testz_ps(a, b); }
inline int mm256_testz_ps(const m256& a, const m256& b) noexcept { return _mm256_testz_ps(a, b); }
inline int mm_testz_pd(const m128d& a, const m128d& b) noexcept { return _mm_testz_pd(a, b); }
inline int mm256_testz_pd(const m256d& a, const m256d& b) noexcept { return _mm256_testz_pd(a, b); }
inline int mm_testz_si128(const m128i& a, const m128i& b) noexcept { return _mm_testz_si128(a, b); }
inline int mm256_testz_si256(const m256i& a, const m256i& b) noexcept { return _mm256_testz_si256(a, b); }

template<int imm8> m128i mm_alignr_epi8(const m128i& a, const m128i& b) noexcept { return _mm_alignr_epi8(a, b, imm8); }
template<int imm8> m256i mm256_alignr_epi8(const m256i& a, const m256i& b) noexcept { return _mm256_alignr_epi8(a, b, imm8); }

inline m128i mm_minpos_epu16(const m128i& a) noexcept { return _mm_minpos_epu16(a); }

inline int mm_movemask_epi8(const m128i& a) noexcept { return _mm_movemask_epi8(a); }
inline int mm256_movemask_epi8(const m256i& a) noexcept { return _mm256_movemask_epi8(a); }
inline int mm_movemask_ps(const m128& a) noexcept { return _mm_movemask_ps(a); }
inline int mm256_movemask_ps(const m256& a) noexcept { return _mm256_movemask_ps(a); }
inline int mm_movemask_pd(const m128d& a) noexcept { return _mm_movemask_pd(a); }
inline int mm256_movemask_pd(const m256d& a) noexcept { return _mm256_movemask_pd(a); }

template<int imm8> m128i mm_mpsadbw_epu8(const m128i& a, const m128i& b) noexcept { return _mm_mpsadbw_epu8(a, b, imm8); }
template<int imm8> m256i mm256_mpsadbw_epu8(const m256i& a, const m256i& b) noexcept { return _mm256_mpsadbw_epu8(a, b, imm8); }

inline m128i mm_packs_epi16(const m128i& a, const m128i& b) noexcept { return _mm_packs_epi16(a, b); }
inline m256i mm256_packs_epi16(const m256i& a, const m256i& b) noexcept { return _mm256_packs_epi16(a, b); }
inline m128i mm_packs_epi32(const m128i& a, const m128i& b) noexcept { return _mm_packs_epi32(a, b); }
inline m256i mm256_packs_epi32(const m256i& a, const m256i& b) noexcept { return _mm256_packs_epi32(a, b); }

inline m128i mm_packus_epi16(const m128i& a, const m128i& b) noexcept { return _mm_packus_epi16(a, b); }
inline m256i mm256_packus_epi16(const m256i& a, const m256i& b) noexcept { return _mm256_packus_epi16(a, b); }
inline m128i mm_packus_epi32(const m128i& a, const m128i& b) noexcept { return _mm_packus_epi32(a, b); }
inline m256i mm256_packus_epi32(const m256i& a, const m256i& b) noexcept { return _mm256_packus_epi32(a, b); }

inline m128i mm_move_epi64(const m128i& a) noexcept { return _mm_move_epi64(a); }
inline m128 mm_move_ss(const m128& a, const m128& b) noexcept { return _mm_move_ss(a, b); }
inline m128d mm_move_sd(const m128d& a, const m128d& b) noexcept { return _mm_move_sd(a, b); }

inline m128 mm_movehdup_ps(const m128& a) noexcept { return _mm_movehdup_ps(a); }
inline m128 mm_moveldup_ps(const m128& a) noexcept { return _mm_moveldup_ps(a); }
inline m256 mm256_movehdup_ps(const m256& a) noexcept { return _mm256_movehdup_ps(a); }
inline m256 mm256_moveldup_ps(const m256& a) noexcept { return _mm256_moveldup_ps(a); }
inline m128d mm_movedup_pd(const m128d& a) noexcept { return _mm_movedup_pd(a); }
inline m256d mm256_movedup_pd(const m256d& a) noexcept { return _mm256_movedup_pd(a); }

inline m128 mm_movehl_ps(const m128& a, const m128& b) noexcept { return _mm_movehl_ps(a, b); }
inline m128 mm_movelh_ps(const m128& a, const m128& b) noexcept { return _mm_movelh_ps(a, b); }

inline m128i mm_avg_epu8(const m128i& a, const m128i& b) noexcept { return _mm_avg_epu8(a, b); }
inline m256i mm256_avg_epu8(const m256i& a, const m256i& b) noexcept { return _mm256_avg_epu8(a, b); }
inline m128i mm_avg_epu16(const m128i& a, const m128i& b) noexcept { return _mm_avg_epu16(a, b); }
inline m256i mm256_avg_epu16(const m256i& a, const m256i& b) noexcept { return _mm256_avg_epu16(a, b); }

inline m128 mm_cdfnorm_ps(const m128& a) noexcept { return _mm_cdfnorm_ps(a); }
inline m256 mm256_cdfnorm_ps(const m256& a) noexcept { return _mm256_cdfnorm_ps(a); }
inline m128d mm_cdfnorm_pd(const m128d& a) noexcept { return _mm_cdfnorm_pd(a); }
inline m256d mm256_cdfnorm_pd(const m256d& a) noexcept { return _mm256_cdfnorm_pd(a); }

inline m128 mm_cdfnorminv_ps(const m128& a) noexcept { return _mm_cdfnorminv_ps(a); }
inline m256 mm256_cdfnorminv_ps(const m256& a) noexcept { return _mm256_cdfnorminv_ps(a); }
inline m128d mm_cdfnorminv_pd(const m128d& a) noexcept { return _mm_cdfnorminv_pd(a); }
inline m256d mm256_cdfnorminv_pd(const m256d& a) noexcept { return _mm256_cdfnorminv_pd(a); }

inline m128 mm_erf_ps(const m128& a) noexcept { return _mm_erf_ps(a); }
inline m256 mm256_erf_ps(const m256& a) noexcept { return _mm256_erf_ps(a); }
inline m128d mm_erf_pd(const m128d& a) noexcept { return _mm_erf_pd(a); }
inline m256d mm256_erf_pd(const m256d& a) noexcept { return _mm256_erf_pd(a); }

inline m128 mm_erfc_ps(const m128& a) noexcept { return _mm_erfc_ps(a); }
inline m256 mm256_erfc_ps(const m256& a) noexcept { return _mm256_erfc_ps(a); }
inline m128d mm_erfc_pd(const m128d& a) noexcept { return _mm_erfc_pd(a); }
inline m256d mm256_erfc_pd(const m256d& a) noexcept { return _mm256_erfc_pd(a); }

inline m128 mm_erfcinv_ps(const m128& a) noexcept { return _mm_erfcinv_ps(a); }
inline m256 mm256_erfcinv_ps(const m256& a) noexcept { return _mm256_erfcinv_ps(a); }
inline m128d mm_erfcinv_pd(const m128d& a) noexcept { return _mm_erfcinv_pd(a); }
inline m256d mm256_erfcinv_pd(const m256d& a) noexcept { return _mm256_erfcinv_pd(a); }

inline m128 mm_erfinv_ps(const m128& a) noexcept { return _mm_erfinv_ps(a); }
inline m256 mm256_erfinv_ps(const m256& a) noexcept { return _mm256_erfinv_ps(a); }
inline m128d mm_erfinv_pd(const m128d& a) noexcept { return _mm_erfinv_pd(a); }
inline m256d mm256_erfinv_pd(const m256d& a) noexcept { return _mm256_erfinv_pd(a); }

inline m128i mm_set_epi8(const char e15, const char e14, const char e13, const char e12, const char e11, const char e10, const char e9, const char e8, const char e7, const char e6, const char e5, const char e4, const char e3, const char e2, const char e1, const char e0) noexcept { return _mm_set_epi8(e15, e14, e13, e12, e11, e10, e9, e8, e7, e6, e5, e4, e3, e2, e1, e0); }
inline m256i mm256_set_epi8(const char e31, const char e30, const char e29, const char e28, const char e27, const char e26, const char e25, const char e24, const char e23, const char e22, const char e21, const char e20, const char e19, const char e18, const char e17, const char e16, const char e15, const char e14, const char e13, const char e12, const char e11, const char e10, const char e9, const char e8, const char e7, const char e6, const char e5, const char e4, const char e3, const char e2, const char e1, const char e0) noexcept { return _mm256_set_epi8(e31, e30, e29, e28, e27, e26, e25, e24, e23, e22, e21, e20, e19, e18, e17, e16, e15, e14, e13, e12, e11, e10, e9, e8, e7, e6, e5, e4, e3, e2, e1, e0); }
inline m128i mm_set_epi16(const short e7, const short e6, const short e5, const short e4, const short e3, const short e2, const short e1, const short e0) noexcept { return _mm_set_epi16(e7, e6, e5, e4, e3, e2, e1, e0); }
inline m256i mm256_set_epi16(const short e15, const short e14, const short e13, const short e12, const short e11, const short e10, const short e9, const short e8, const short e7, const short e6, const short e5, const short e4, const short e3, const short e2, const short e1, const short e0) noexcept { return _mm256_set_epi16(e15, e14, e13, e12, e11, e10, e9, e8, e7, e6, e5, e4, e3, e2, e1, e0); }
inline m128i mm_set_epi32(const int e3, const int e2, const int e1, const int e0) noexcept { return _mm_set_epi32(e3, e2, e1, e0); }
inline m256i mm256_set_epi32(const int e7, const int e6, const int e5, const int e4, const int e3, const int e2, const int e1, const int e0) noexcept { return _mm256_set_epi32(e7, e6, e5, e4, e3, e2, e1, e0); }
inline m128i mm_set_epi64(const long long e1, const long long e0) noexcept { return _mm_set_epi64x(e1, e0); }
inline m128i mm_set_epi64x(const long long e1, const long long e0) noexcept { return _mm_set_epi64x(e1, e0); }
inline m256i mm256_set_epi64(const long long e3, const long long e2, const long long e1, const long long e0) noexcept { return _mm256_set_epi64x(e3, e2, e1, e0); }
inline m256i mm256_set_epi64x(const long long e3, const long long e2, const long long e1, const long long e0) noexcept { return _mm256_set_epi64x(e3, e2, e1, e0); }

inline m128 mm_set_ss(const float e0) noexcept { return _mm_set_ss(e0); }
inline m128 mm_set_ps(const float e3, const float e2, const float e1, const float e0) noexcept { return _mm_set_ps(e3, e2, e1, e0); }
inline m256 mm256_set_ps(const float e7, const float e6, const float e5, const float e4, const float e3, const float e2, const float e1, const float e0) noexcept { return _mm256_set_ps(e7, e6, e5, e4, e3, e2, e1, e0); }
inline m128d mm_set_sd(const double e0) noexcept { return _mm_set_sd(e0); }
inline m128d mm_set_pd(const double e1, const double e0) noexcept { return _mm_set_pd(e1, e0); }
inline m256d mm256_set_pd(const double e3, const double e2, const double e1, const double e0) noexcept { return _mm256_set_pd(e3, e2, e1, e0); }

inline m128i mm_set1_epi8(const char a) noexcept { return _mm_set1_epi8(a); }
inline m256i mm256_set1_epi8(const char a) noexcept { return _mm256_set1_epi8(a); }
inline m128i mm_set1_epi16(const short a) noexcept { return _mm_set1_epi16(a); }
inline m256i mm256_set1_epi16(const short a) noexcept { return _mm256_set1_epi16(a); }
inline m128i mm_set1_epi32(const int a) noexcept { return _mm_set1_epi32(a); }
inline m256i mm256_set1_epi32(const int a) noexcept { return _mm256_set1_epi32(a); }
inline m128i mm_set1_epi64(const long long a) noexcept { return _mm_set1_epi64x(a); }
inline m128i mm_set1_epi64x(const long long a) noexcept { return _mm_set1_epi64x(a); }
inline m256i mm256_set1_epi64(const long long a) noexcept { return _mm256_set1_epi64x(a); }
inline m256i mm256_set1_epi64x(const long long a) noexcept { return _mm256_set1_epi64x(a); }

inline m128 mm_set1_ps(const float a) noexcept { return _mm_set1_ps(a); }
inline m128 mm_set_ps1(const float a) noexcept { return _mm_set1_ps(a); }
inline m256 mm256_set1_ps(const float a) noexcept { return _mm256_set1_ps(a); }
inline m128d mm_set1_pd(const double a) noexcept { return _mm_set1_pd(a); }
inline m128d mm_set_pd1(const double a) noexcept { return _mm_set1_pd(a); }
inline m256d mm256_set1_pd(const double a) noexcept { return _mm256_set1_pd(a); }

inline m128i mm_setr_epi8(const char e0, const char e1, const char e2, const char e3, const char e4, const char e5, const char e6, const char e7, const char e8, const char e9, const char e10, const char e11, const char e12, const char e13, const char e14, const char e15) noexcept { return _mm_setr_epi8(e0, e1, e2, e3, e4, e5, e6, e7, e8, e9, e10, e11, e12, e13, e14, e15); }
inline m256i mm256_setr_epi8(const char e0, const char e1, const char e2, const char e3, const char e4, const char e5, const char e6, const char e7, const char e8, const char e9, const char e10, const char e11, const char e12, const char e13, const char e14, const char e15, const char e16, const char e17, const char e18, const char e19, const char e20, const char e21, const char e22, const char e23, const char e24, const char e25, const char e26, const char e27, const char e28, const char e29, const char e30, const char e31) noexcept { return _mm256_setr_epi8(e0, e1, e2, e3, e4, e5, e6, e7, e8, e9, e10, e11, e12, e13, e14, e15, e16, e17, e18, e19, e20, e21, e22, e23, e24, e25, e26, e27, e28, e29, e30, e31); }
inline m128i mm_setr_epi16(const short e0, const short e1, const short e2, const short e3, const short e4, const short e5, const short e6, const short e7) noexcept { return _mm_setr_epi16(e0, e1, e2, e3, e4, e5, e6, e7); }
inline m256i mm256_setr_epi16(const short e0, const short e1, const short e2, const short e3, const short e4, const short e5, const short e6, const short e7, const short e8, const short e9, const short e10, const short e11, const short e12, const short e13, const short e14, const short e15) noexcept { return _mm256_setr_epi16(e0, e1, e2, e3, e4, e5, e6, e7, e8, e9, e10, e11, e12, e13, e14, e15); }
inline m128i mm_setr_epi32(const int e0, const int e1, const int e2, const int e3) noexcept { return _mm_setr_epi32(e0, e1, e2, e3); }
inline m256i mm256_setr_epi32(const int e0, const int e1, const int e2, const int e3, const int e4, const int e5, const int e6, const int e7) noexcept { return _mm256_setr_epi32(e0, e1, e2, e3, e4, e5, e6, e7); }
inline m128i mm_setr_epi64(const long long e0, const long long e1) noexcept { return _mm_set_epi64x(e1, e0); }
inline m128i mm_setr_epi64x(const long long e0, const long long e1) noexcept { return _mm_set_epi64x(e1, e0); }
inline m256i mm256_setr_epi64(const long long e0, const long long e1, const long long e2, const long long e3) noexcept { return _mm256_setr_epi64x(e0, e1, e2, e3); }
inline m256i mm256_setr_epi64x(const long long e0, const long long e1, const long long e2, const long long e3) noexcept { return _mm256_setr_epi64x(e0, e1, e2, e3); }

inline m128 mm_setr_ps(const float e0, const float e1, const float e2, const float e3) noexcept { return _mm_setr_ps(e0, e1, e2, e3); }
inline m256 mm256_setr_ps(const float e0, const float e1, const float e2, const float e3, const float e4, const float e5, const float e6, const float e7) noexcept { return _mm256_setr_ps(e0, e1, e2, e3, e4, e5, e6, e7); }
inline m128d mm_setr_pd(const double e0, const double e1) noexcept { return _mm_setr_pd(e0, e1); }
inline m256d mm256_setr_pd(const double e0, const double e1, const double e2, const double e3) noexcept { return _mm256_setr_pd(e0, e1, e2, e3); }

inline m128 mm_setzero_ps() noexcept { return _mm_setzero_ps(); }
inline m256 mm256_setzero_ps() noexcept { return _mm256_setzero_ps(); }
inline m128d mm_setzero_pd() noexcept { return _mm_setzero_pd(); }
inline m256d mm256_setzero_pd() noexcept { return _mm256_setzero_pd(); }
inline m128i mm_setzero_si128() noexcept { return _mm_setzero_si128(); }
inline m256i mm256_setzero_si256() noexcept { return _mm256_setzero_si256(); }

template<int imm8> m128i mm_bslli_si128(const m128i& a) noexcept { return _mm_bslli_si128(a, imm8); }
template<int imm8> m256i mm256_bslli_si128(const m256i& a) noexcept { return _mm256_bslli_si128(a, imm8); }
template<int imm8> m128i mm_bsrli_si128(const m128i& a) noexcept { return _mm_bsrli_si128(a, imm8); }
template<int imm8> m256i mm256_bsrli_si128(const m256i& a) noexcept { return _mm256_bsrli_si128(a, imm8); }

inline m128i mm_sll_epi16(const m128i& a, const m128i& count) noexcept { return _mm_sll_epi16(a, count); }
inline m256i mm256_sll_epi16(const m256i& a, const m128i& count) noexcept { return _mm256_sll_epi16(a, count); }
inline m128i mm_sll_epi32(const m128i& a, const m128i& count) noexcept { return _mm_sll_epi32(a, count); }
inline m256i mm256_sll_epi32(const m256i& a, const m128i& count) noexcept { return _mm256_sll_epi32(a, count); }
inline m128i mm_sll_epi64(const m128i& a, const m128i& count) noexcept { return _mm_sll_epi64(a, count); }
inline m256i mm256_sll_epi64(const m256i& a, const m128i& count) noexcept { return _mm256_sll_epi64(a, count); }

template<int imm8> m128i mm_slli_epi16(const m128i& a) noexcept { return _mm_slli_epi16(a, imm8); }
template<int imm8> m256i mm256_slli_epi16(const m256i& a) noexcept { return _mm256_slli_epi16(a, imm8); }
template<int imm8> m128i mm_slli_epi32(const m128i& a) noexcept { return _mm_slli_epi32(a, imm8); }
template<int imm8> m256i mm256_slli_epi32(const m256i& a) noexcept { return _mm256_slli_epi32(a, imm8); }
template<int imm8> m128i mm_slli_epi64(const m128i& a) noexcept { return _mm_slli_epi64(a, imm8); }
template<int imm8> m256i mm256_slli_epi64(const m256i& a) noexcept { return _mm256_slli_epi64(a, imm8); }
template<int imm8> m128i mm_slli_si128(const m128i& a) noexcept { return _mm_slli_si128(a, imm8); }
template<int imm8> m256i mm256_slli_si256(const m256i& a) noexcept { return _mm256_slli_si256(a, imm8); }

inline m128i mm_sllv_epi32(const m128i& a, const m128i& count) noexcept { return _mm_sllv_epi32(a, count); }
inline m256i mm256_sllv_epi32(const m256i& a, const m256i& count) noexcept { return _mm256_sllv_epi32(a, count); }
inline m128i mm_sllv_epi64(const m128i& a, const m128i& count) noexcept { return _mm_sllv_epi64(a, count); }
inline m256i mm256_sllv_epi64(const m256i& a, const m256i& count) noexcept { return _mm256_sllv_epi64(a, count); }

inline m128i mm_sra_epi16(const m128i& a, const m128i& count) noexcept { return _mm_sra_epi16(a, count); }
inline m256i mm256_sra_epi16(const m256i& a, const m128i& count) noexcept { return _mm256_sra_epi16(a, count); }
inline m128i mm_sra_epi32(const m128i& a, const m128i& count) noexcept { return _mm_sra_epi32(a, count); }
inline m256i mm256_sra_epi32(const m256i& a, const m128i& count) noexcept { return _mm256_sra_epi32(a, count); }

template<int imm8> m128i mm_srai_epi16(const m128i& a) noexcept { return _mm_srai_epi16(a, imm8); }
template<int imm8> m256i mm256_srai_epi16(const m256i& a) noexcept { return _mm256_srai_epi16(a, imm8); }
template<int imm8> m128i mm_srai_epi32(const m128i& a) noexcept { return _mm_srai_epi32(a, imm8); }
template<int imm8> m256i mm256_srai_epi32(const m256i& a) noexcept { return _mm256_srai_epi32(a, imm8); }

inline m128i mm_srav_epi32(const m128i& a, const m128i& count) noexcept { return _mm_srav_epi32(a, count); }
inline m256i mm256_srav_epi32(const m256i& a, const m256i& count) noexcept { return _mm256_srav_epi32(a, count); }

inline m128i mm_srl_epi16(const m128i& a, const m128i& count) noexcept { return _mm_srl_epi16(a, count); }
inline m256i mm256_srl_epi16(const m256i& a, const m128i& count) noexcept { return _mm256_srl_epi16(a, count); }
inline m128i mm_srl_epi32(const m128i& a, const m128i& count) noexcept { return _mm_srl_epi32(a, count); }
inline m256i mm256_srl_epi32(const m256i& a, const m128i& count) noexcept { return _mm256_srl_epi32(a, count); }
inline m128i mm_srl_epi64(const m128i& a, const m128i& count) noexcept { return _mm_srl_epi64(a, count); }
inline m256i mm256_srl_epi64(const m256i& a, const m128i& count) noexcept { return _mm256_srl_epi64(a, count); }

template<int imm8> m128i mm_srli_epi16(const m128i& a) noexcept { return _mm_srli_epi16(a, imm8); }
template<int imm8> m256i mm256_srli_epi16(const m256i& a) noexcept { return _mm256_srli_epi16(a, imm8); }
template<int imm8> m128i mm_srli_epi32(const m128i& a) noexcept { return _mm_srli_epi32(a, imm8); }
template<int imm8> m256i mm256_srli_epi32(const m256i& a) noexcept { return _mm256_srli_epi32(a, imm8); }
template<int imm8> m128i mm_srli_epi64(const m128i& a) noexcept { return _mm_srli_epi64(a, imm8); }
template<int imm8> m256i mm256_srli_epi64(const m256i& a) noexcept { return _mm256_srli_epi64(a, imm8); }
template<int imm8> m128i mm_srli_si128(const m128i& a) noexcept { return _mm_srli_si128(a, imm8); }
template<int imm8> m256i mm256_srli_si256(const m256i& a) noexcept { return _mm256_srli_si256(a, imm8); }

inline m128i mm_srlv_epi32(const m128i& a, const m128i& count) noexcept { return _mm_srlv_epi32(a, count); }
inline m256i mm256_srlv_epi32(const m256i& a, const m256i& count) noexcept { return _mm256_srlv_epi32(a, count); }
inline m128i mm_srlv_epi64(const m128i& a, const m128i& count) noexcept { return _mm_srlv_epi64(a, count); }
inline m256i mm256_srlv_epi64(const m256i& a, const m256i& count) noexcept { return _mm256_srlv_epi64(a, count); }

inline m128i mm_abs_epi8(const m128i& a) noexcept { return _mm_abs_epi8(a); }
inline m256i mm256_abs_epi8(const m256i& a) noexcept { return _mm256_abs_epi8(a); }
inline m128i mm_abs_epi16(const m128i& a) noexcept { return _mm_abs_epi16(a); }
inline m256i mm256_abs_epi16(const m256i& a) noexcept { return _mm256_abs_epi16(a); }
inline m128i mm_abs_epi32(const m128i& a) noexcept { return _mm_abs_epi32(a); }
inline m256i mm256_abs_epi32(const m256i& a) noexcept { return _mm256_abs_epi32(a); }

inline m128 mm_ceil_ss(const m128& a, const m128& b) noexcept { return _mm_ceil_ss(a, b); }
inline m128 mm_ceil_ps(const m128& a) noexcept { return _mm_ceil_ps(a); }
inline m256 mm256_ceil_ps(const m256& a) noexcept { return _mm256_ceil_ps(a); }
inline m128d mm_ceil_sd(const m128d& a, const m128d& b) noexcept { return _mm_ceil_sd(a, b); }
inline m128d mm_ceil_pd(const m128d& a) noexcept { return _mm_ceil_pd(a); }
inline m256d mm256_ceil_pd(const m256d& a) noexcept { return _mm256_ceil_pd(a); }

inline m128 mm_floor_ss(const m128& a, const m128& b) noexcept { return _mm_floor_ss(a, b); }
inline m128 mm_floor_ps(const m128& a) noexcept { return _mm_floor_ps(a); }
inline m256 mm256_floor_ps(const m256& a) noexcept { return _mm256_floor_ps(a); }
inline m128d mm_floor_sd(const m128d& a, const m128d& b) noexcept { return _mm_floor_sd(a, b); }
inline m128d mm_floor_pd(const m128d& a) noexcept { return _mm_floor_pd(a); }
inline m256d mm256_floor_pd(const m256d& a) noexcept { return _mm256_floor_pd(a); }

inline m128i mm_max_epi8(const m128i& a, const m128i& b) noexcept { return _mm_max_epi8(a, b); }
inline m256i mm256_max_epi8(const m256i& a, const m256i& b) noexcept { return _mm256_max_epi8(a, b); }
inline m128i mm_max_epi16(const m128i& a, const m128i& b) noexcept { return _mm_max_epi16(a, b); }
inline m256i mm256_max_epi16(const m256i& a, const m256i& b) noexcept { return _mm256_max_epi16(a, b); }
inline m128i mm_max_epi32(const m128i& a, const m128i& b) noexcept { return _mm_max_epi32(a, b); }
inline m256i mm256_max_epi32(const m256i& a, const m256i& b) noexcept { return _mm256_max_epi32(a, b); }

inline m128i mm_max_epu8(const m128i& a, const m128i& b) noexcept { return _mm_max_epu8(a, b); }
inline m256i mm256_max_epu8(const m256i& a, const m256i& b) noexcept { return _mm256_max_epu8(a, b); }
inline m128i mm_max_epu16(const m128i& a, const m128i& b) noexcept { return _mm_max_epu16(a, b); }
inline m256i mm256_max_epu16(const m256i& a, const m256i& b) noexcept { return _mm256_max_epu16(a, b); }
inline m128i mm_max_epu32(const m128i& a, const m128i& b) noexcept { return _mm_max_epu32(a, b); }
inline m256i mm256_max_epu32(const m256i& a, const m256i& b) noexcept { return _mm256_max_epu32(a, b); }

inline m128 mm_max_ss(const m128& a, const m128& b) noexcept { return _mm_max_ss(a, b); }
inline m128 mm_max_ps(const m128& a, const m128& b) noexcept { return _mm_max_ps(a, b); }
inline m256 mm256_max_ps(const m256& a, const m256& b) noexcept { return _mm256_max_ps(a, b); }
inline m128d mm_max_sd(const m128d& a, const m128d& b) noexcept { return _mm_max_sd(a, b); }
inline m128d mm_max_pd(const m128d& a, const m128d& b) noexcept { return _mm_max_pd(a, b); }
inline m256d mm256_max_pd(const m256d& a, const m256d& b) noexcept { return _mm256_max_pd(a, b); }

inline m128i mm_min_epi8(const m128i& a, const m128i& b) noexcept { return _mm_min_epi8(a, b); }
inline m256i mm256_min_epi8(const m256i& a, const m256i& b) noexcept { return _mm256_min_epi8(a, b); }
inline m128i mm_min_epi16(const m128i& a, const m128i& b) noexcept { return _mm_min_epi16(a, b); }
inline m256i mm256_min_epi16(const m256i& a, const m256i& b) noexcept { return _mm256_min_epi16(a, b); }
inline m128i mm_min_epi32(const m128i& a, const m128i& b) noexcept { return _mm_min_epi32(a, b); }
inline m256i mm256_min_epi32(const m256i& a, const m256i& b) noexcept { return _mm256_min_epi32(a, b); }

inline m128i mm_min_epu8(const m128i& a, const m128i& b) noexcept { return _mm_min_epu8(a, b); }
inline m256i mm256_min_epu8(const m256i& a, const m256i& b) noexcept { return _mm256_min_epu8(a, b); }
inline m128i mm_min_epu16(const m128i& a, const m128i& b) noexcept { return _mm_min_epu16(a, b); }
inline m256i mm256_min_epu16(const m256i& a, const m256i& b) noexcept { return _mm256_min_epu16(a, b); }
inline m128i mm_min_epu32(const m128i& a, const m128i& b) noexcept { return _mm_min_epu32(a, b); }
inline m256i mm256_min_epu32(const m256i& a, const m256i& b) noexcept { return _mm256_min_epu32(a, b); }

inline m128 mm_min_ss(const m128& a, const m128& b) noexcept { return _mm_min_ss(a, b); }
inline m128 mm_min_ps(const m128& a, const m128& b) noexcept { return _mm_min_ps(a, b); }
inline m256 mm256_min_ps(const m256& a, const m256& b) noexcept { return _mm256_min_ps(a, b); }
inline m128d mm_min_sd(const m128d& a, const m128d& b) noexcept { return _mm_min_sd(a, b); }
inline m128d mm_min_pd(const m128d& a, const m128d& b) noexcept { return _mm_min_pd(a, b); }
inline m256d mm256_min_pd(const m256d& a, const m256d& b) noexcept { return _mm256_min_pd(a, b); }

template<int imm8> inline m128 mm_round_ss(const m128& a, const m128& b) noexcept { return _mm_round_ss(a, b, imm8); }
template<int imm8> inline m128 mm_round_ps(const m128& a) noexcept { return _mm_round_ps(a, imm8); }
template<int imm8> inline m256 mm256_round_ps(const m256& a) noexcept { return _mm256_round_ps(a, imm8); }
template<int imm8> inline m128d mm_round_sd(const m128d& a, const m128d& b) noexcept { return _mm_round_sd(a, b, imm8); }
template<int imm8> inline m128d mm_round_pd(const m128d& a) noexcept { return _mm_round_pd(a, imm8); }
template<int imm8> inline m256d mm256_round_pd(const m256d& a) noexcept { return _mm256_round_pd(a, imm8); }

inline m128 mm_svml_ceil_ps(const m128& a) noexcept { return _mm_svml_ceil_ps(a); }
inline m256 mm256_svml_ceil_ps(const m256& a) noexcept { return _mm256_svml_ceil_ps(a); }
inline m128d mm_svml_ceil_pd(const m128d& a) noexcept { return _mm_svml_ceil_pd(a); }
inline m256d mm256_svml_ceil_pd(const m256d& a) noexcept { return _mm256_svml_ceil_pd(a); }

inline m128 mm_svml_floor_ps(const m128& a) noexcept { return _mm_svml_floor_ps(a); }
inline m256 mm256_svml_floor_ps(const m256& a) noexcept { return _mm256_svml_floor_ps(a); }
inline m128d mm_svml_floor_pd(const m128d& a) noexcept { return _mm_svml_floor_pd(a); }
inline m256d mm256_svml_floor_pd(const m256d& a) noexcept { return _mm256_svml_floor_pd(a); }

inline m128 mm_svml_round_ps(const m128& a) noexcept { return _mm_svml_round_ps(a); }
inline m256 mm256_svml_round_ps(const m256& a) noexcept { return _mm256_svml_round_ps(a); }
inline m128d mm_svml_round_pd(const m128d& a) noexcept { return _mm_svml_round_pd(a); }
inline m256d mm256_svml_round_pd(const m256d& a) noexcept { return _mm256_svml_round_pd(a); }

inline m128 mm_trunc_ps(const m128& a) noexcept { return _mm_trunc_ps(a); }
inline m256 mm256_trunc_ps(const m256& a) noexcept { return _mm256_trunc_ps(a); }
inline m128d mm_trunc_pd(const m128d& a) noexcept { return _mm_trunc_pd(a); }
inline m256d mm256_trunc_pd(const m256d& a) noexcept { return _mm256_trunc_pd(a); }

inline void mm_maskmoveu_si128(const m128i& a, const m128i& mask, char* mem_addr) noexcept { _mm_maskmoveu_si128(a, mask, mem_addr); }

inline void mm_maskstore_epi32(int* mem_addr, const m128i& mask, const m128i& a) noexcept { _mm_maskstore_epi32(mem_addr, mask, a); }
inline void mm256_maskstore_epi32(int* mem_addr, const m256i& mask, const m256i& a) noexcept { _mm256_maskstore_epi32(mem_addr, mask, a); }
inline void mm_maskstore_epi64(__int64* mem_addr, const m128i& mask, const m128i& a) noexcept { _mm_maskstore_epi64(mem_addr, mask, a); }
inline void mm256_maskstore_epi64(__int64* mem_addr, const m256i& mask, const m256i& a) noexcept { _mm256_maskstore_epi64(mem_addr, mask, a); }
inline void mm_maskstore_ps(float* mem_addr, const m128i& mask, const m128& a) noexcept { _mm_maskstore_ps(mem_addr, mask, a); }
inline void mm256_maskstore_ps(float* mem_addr, const m256i& mask, const m256& a) noexcept { _mm256_maskstore_ps(mem_addr, mask, a); }
inline void mm_maskstore_pd(double* mem_addr, const m128i& mask, const m128d& a) noexcept { _mm_maskstore_pd(mem_addr, mask, a); }
inline void mm256_maskstore_pd(double* mem_addr, const m256i& mask, const m256d& a) noexcept { _mm256_maskstore_pd(mem_addr, mask, a); }

inline void mm_store_ss(float* mem_addr, const m128& a) noexcept { _mm_store_ss(mem_addr, a); }
inline void mm_store_ps(float* mem_addr, const m128& a) noexcept { _mm_store_ps(mem_addr, a); }
inline void mm256_store_ps(float* mem_addr, const m256& a) noexcept { _mm256_store_ps(mem_addr, a); }
inline void mm_store_sd(double* mem_addr, const m128d& a) noexcept { _mm_store_sd(mem_addr, a); }
inline void mm_store_pd(double* mem_addr, const m128d& a) noexcept { _mm_store_pd(mem_addr, a); }
inline void mm256_store_pd(double* mem_addr, const m256d& a) noexcept { _mm256_store_pd(mem_addr, a); }
inline void mm_store_si128(m128i* mem_addr, const m128i& a) noexcept { _mm_store_si128(mem_addr, a); }
inline void mm256_store_si256(m256i* mem_addr, const m256i& a) noexcept { _mm256_store_si256(mem_addr, a); }

inline void mm_store1_ps(float* mem_addr, const m128& a) noexcept { _mm_store1_ps(mem_addr, a); }
inline void mm_store1_pd(double* mem_addr, const m128d& a) noexcept { _mm_store1_pd(mem_addr, a); }

inline void mm_storel_epi64(m128i* mem_addr, const m128i& a) noexcept { _mm_storel_epi64(mem_addr, a); }
inline void mm_storel_pd(double* mem_addr, const m128d& a) noexcept { _mm_storel_pd(mem_addr, a); }
inline void mm_storeh_pd(double* mem_addr, const m128d& a) noexcept { _mm_storeh_pd(mem_addr, a); }

inline void mm_storer_ps(float* mem_addr, const m128& a) noexcept { _mm_storer_ps(mem_addr, a); }
inline void mm_storer_pd(double* mem_addr, const m128d& a) noexcept { _mm_storer_pd(mem_addr, a); }

inline void mm_storeu_si16(void* mem_addr, const m128i& a) noexcept { _mm_storeu_si16(mem_addr, a); }
inline void mm_storeu_si32(void* mem_addr, const m128i& a) noexcept { _mm_storeu_si32(mem_addr, a); }
inline void mm_storeu_si64(void* mem_addr, const m128i& a) noexcept { _mm_storeu_si64(mem_addr, a); }
inline void mm_storeu_ps(float* mem_addr, const m128& a) noexcept { _mm_storeu_ps(mem_addr, a); }
inline void mm256_storeu_ps(float* mem_addr, const m256& a) noexcept { _mm256_storeu_ps(mem_addr, a); }
inline void mm_storeu_pd(double* mem_addr, const m128d& a) noexcept { _mm_storeu_pd(mem_addr, a); }
inline void mm256_storeu_pd(double* mem_addr, const m256d& a) noexcept { _mm256_storeu_pd(mem_addr, a); }
inline void mm_storeu_si128(m128i* mem_addr, const m128i& a) noexcept { _mm_storeu_si128(mem_addr, a); }
inline void mm256_storeu_si256(m256i* mem_addr, const m256i& a) noexcept { _mm256_storeu_si256(mem_addr, a); }

inline void mm256_storeu2_m128(float* hiaddr, float* loaddr, const m256& a) noexcept { _mm256_storeu2_m128(hiaddr, loaddr, a); }
inline void mm256_storeu2_m128d(double* hiaddr, double* loaddr, const m256d& a) noexcept { _mm256_storeu2_m128d(hiaddr, loaddr, a); }
inline void mm256_storeu2_m128i(m128i* hiaddr, m128i* loaddr, const m256i& a) noexcept { _mm256_storeu2_m128i(hiaddr, loaddr, a); }

inline void mm_stream_si32(int* mem_addr, const int a) noexcept { _mm_stream_si32(mem_addr, a); }
inline void mm_stream_ps(float* mem_addr, const m128& a) noexcept { _mm_stream_ps(mem_addr, a); }
inline void mm256_stream_ps(float* mem_addr, const m256& a) noexcept { _mm256_stream_ps(mem_addr, a); }
inline void mm_stream_pd(double* mem_addr, const m128d& a) noexcept { _mm_stream_pd(mem_addr, a); }
inline void mm256_stream_pd(double* mem_addr, const m256d& a) noexcept { _mm256_stream_pd(mem_addr, a); }
inline void mm_stream_si128(m128i* mem_addr, const m128i& a) noexcept { _mm_stream_si128(mem_addr, a); }
inline void mm256_stream_si256(m256i* mem_addr, const m256i& a) noexcept { _mm256_stream_si256(mem_addr, a); }

template<int imm8> int mm_cmpestra(const m128i& a, int la, const m128i& b, int lb) noexcept { return _mm_cmpestra(a, la, b, lb, imm8); }
template<int imm8> int mm_cmpestrc(const m128i& a, int la, const m128i& b, int lb) noexcept { return _mm_cmpestrc(a, la, b, lb, imm8); }
template<int imm8> int mm_cmpestri(const m128i& a, int la, const m128i& b, int lb) noexcept { return _mm_cmpestri(a, la, b, lb, imm8); }
template<int imm8> m128i mm_cmpestrm(const m128i& a, int la, const m128i& b, int lb) noexcept { return _mm_cmpestrm(a, la, b, lb, imm8); }
template<int imm8> int mm_cmpestro(const m128i& a, int la, const m128i& b, int lb) noexcept { return _mm_cmpestro(a, la, b, lb, imm8); }
template<int imm8> int mm_cmpestrs(const m128i& a, int la, const m128i& b, int lb) noexcept { return _mm_cmpestrs(a, la, b, lb, imm8); }
template<int imm8> int mm_cmpestrz(const m128i& a, int la, const m128i& b, int lb) noexcept { return _mm_cmpestrz(a, la, b, lb, imm8); }

template<int imm8> int mm_cmpistra(const m128i& a, const m128i& b) noexcept { return _mm_cmpistra(a, b, imm8); }
template<int imm8> int mm_cmpistrc(const m128i& a, const m128i& b) noexcept { return _mm_cmpistrc(a, b, imm8); }
template<int imm8> int mm_cmpistri(const m128i& a, const m128i& b) noexcept { return _mm_cmpistri(a, b, imm8); }
template<int imm8> m128i mm_cmpistrm(const m128i& a, const m128i& b) noexcept { return _mm_cmpistrm(a, b, imm8); }
template<int imm8> int mm_cmpistro(const m128i& a, const m128i& b) noexcept { return _mm_cmpistro(a, b, imm8); }
template<int imm8> int mm_cmpistrs(const m128i& a, const m128i& b) noexcept { return _mm_cmpistrs(a, b, imm8); }
template<int imm8> int mm_cmpistrz(const m128i& a, const m128i& b) noexcept { return _mm_cmpistrz(a, b, imm8); }

template<int imm8> m128i mm_blend_epi16(const m128i& a, const m128i& b) noexcept { return _mm_blend_epi16(a, b, imm8); }
template<int imm8> m256i mm256_blend_epi16(const m256i& a, const m256i& b) noexcept { return _mm256_blend_epi16(a, b, imm8); }
template<int imm8> m128i mm_blend_epi32(const m128i& a, const m128i& b) noexcept { return _mm_blend_epi32(a, b, imm8); }
template<int imm8> m256i mm256_blend_epi32(const m256i& a, const m256i& b) noexcept { return _mm256_blend_epi32(a, b, imm8); }
template<int imm8> m128 mm_blend_ps(const m128& a, const m128& b) noexcept { return _mm_blend_ps(a, b, imm8); }
template<int imm8> m256 mm256_blend_ps(const m256& a, const m256& b) noexcept { return _mm256_blend_ps(a, b, imm8); }
template<int imm8> m128d mm_blend_pd(const m128d& a, const m128d& b) noexcept { return _mm_blend_pd(a, b, imm8); }
template<int imm8> m256d mm256_blend_pd(const m256d& a, const m256d& b) noexcept { return _mm256_blend_pd(a, b, imm8); }

inline m128i mm_blendv_epi8(const m128i& a, const m128i& b, const m128i& mask) noexcept { return _mm_blendv_epi8(a, b, mask); }
inline m256i mm256_blendv_epi8(const m256i& a, const m256i& b, const m256i& mask) noexcept { return _mm256_blendv_epi8(a, b, mask); }
inline m128 mm_blendv_ps(const m128& a, const m128& b, const m128& mask) noexcept { return _mm_blendv_ps(a, b, mask); }
inline m256 mm256_blendv_ps(const m256& a, const m256& b, const m256& mask) noexcept { return _mm256_blendv_ps(a, b, mask); }
inline m128d mm_blendv_pd(const m128d& a, const m128d& b, const m128d& mask) noexcept { return _mm_blendv_pd(a, b, mask); }
inline m256d mm256_blendv_pd(const m256d& a, const m256d& b, const m256d& mask) noexcept { return _mm256_blendv_pd(a, b, mask); }

template<int imm8> m128i mm_extract_epi8(const m128i& a) noexcept { return _mm_extract_epi8(a, imm8); }
template<int imm8> m256i mm256_extract_epi8(const m256i& a) noexcept { return _mm256_extract_epi8(a, imm8); }
template<int imm8> m128i mm_extract_epi16(const m128i& a) noexcept { return _mm_extract_epi16(a, imm8); }
template<int imm8> m256i mm256_extract_epi16(const m256i& a) noexcept { return _mm256_extract_epi16(a, imm8); }
template<int imm8> m128i mm_extract_epi32(const m128i& a) noexcept { return _mm_extract_epi32(a, imm8); }
template<int imm8> m256i mm256_extract_epi32(const m256i& a) noexcept { return _mm256_extract_epi32(a, imm8); }
template<int imm8> m128i mm_extract_epi64(const m128i& a) noexcept { return _mm_extract_epi64(a, imm8); }
template<int imm8> m256i mm256_extract_epi64(const m256i& a) noexcept { return _mm256_extract_epi64(a, imm8); }
template<int imm8> m128 mm_extract_ps(const m128& a) noexcept { return _mm_extract_ps(a, imm8); }
template<int imm8> m256 mm256_extractf128_ps(const m256& a) noexcept { return _mm256_extractf128_ps(a, imm8); }
template<int imm8> m256d mm256_extractf128_pd(const m256d& a) noexcept { return _mm256_extractf128_pd(a, imm8); }
template<int imm8> m256i mm256_extractf128_si256(const m256i& a) noexcept { return _mm256_extractf128_si256(a, imm8); }
template<int imm8> m256i mm256_extracti128_si256(const m256i& a) noexcept { return _mm256_extracti128_si256(a, imm8); }

template<int imm8> m128i mm_insert_epi8(const m128i& a, const int i) noexcept { return _mm_insert_epi8(a, i, imm8); }
template<int imm8> m256i mm256_insert_epi8(const m256i& a, const __int8 i) noexcept { return _mm256_insert_epi8(a, i, imm8); }
template<int imm8> m128i mm_insert_epi16(const m128i& a, const int i) noexcept { return _mm_insert_epi16(a, i, imm8); }
template<int imm8> m256i mm256_insert_epi16(const m256i& a, const __int16 i) noexcept { return _mm256_insert_epi16(a, i, imm8); }
template<int imm8> m128i mm_insert_epi32(const m128i& a, const int i) noexcept { return _mm_insert_epi32(a, i, imm8); }
template<int imm8> m256i mm256_insert_epi32(const m256i& a, const __int32 i) noexcept { return _mm256_insert_epi32(a, i, imm8); }
template<int imm8> m128i mm_insert_epi64(const m128i& a, const __int64 i) noexcept { return _mm_insert_epi64(a, i, imm8); }
template<int imm8> m256i mm256_insert_epi64(const m256i& a, const __int64 i) noexcept { return _mm256_insert_epi64(a, i, imm8); }
template<int imm8> m128 mm_insert_ps(const m128& a, const m128& b) noexcept { return _mm_insert_ps(a, b, imm8); }
template<int imm8> m256 mm256_insertf128_ps(const m256& a, const m128& b) noexcept { return _mm256_insertf128_ps(a, b, imm8); }
template<int imm8> m256d mm256_insertf128_pd(const m256d& a, const m128d& b) noexcept { return _mm256_insertf128_pd(a, b, imm8); }
template<int imm8> m256i mm256_insertf128_si256(const m256i& a, const m128i& b) noexcept { return _mm256_insertf128_si256(a, b, imm8); }
template<int imm8> m256i mm256_inserti128_si256(const m256i& a, const m128i& b) noexcept { return _mm256_inserti128_si256(a, b, imm8); }

template<int imm8> m128 mm_permute_ps(const m128& a) noexcept { return _mm_permute_ps(a, imm8); }
template<int imm8> m256 mm256_permute_ps(const m256& a) noexcept { return _mm256_permute_ps(a, imm8); }
template<int imm8> m128d mm_permute_pd(const m128d& a) noexcept { return _mm_permute_pd(a, imm8); }
template<int imm8> m256d mm256_permute_pd(const m256d& a) noexcept { return _mm256_permute_pd(a, imm8); }

template<int imm8> m128 mm_permutevar_ps(const m128& a, const m128i& b) noexcept { return _mm_permutevar_ps(a, b, imm8); }
template<int imm8> m256 mm256_permutevar_ps(const m256& a, const m256i& b) noexcept { return _mm256_permutevar_ps(a, b, imm8); }
template<int imm8> m128d mm_permutevar_pd(const m128d& a, const m128i& b) noexcept { return _mm_permutevar_pd(a, b, imm8); }
template<int imm8> m256d mm256_permutevar_pd(const m256d& a, const m256i& b) noexcept { return _mm256_permutevar_pd(a, b, imm8); }

template<int imm8> m256 mm256_permute2f128_ps(const m256& a, const m256& b) noexcept { return _mm256_permute2f128_ps(a, b, imm8); }
template<int imm8> m256d mm256_permute2f128_pd(const m256d& a, const m256d& b) noexcept { return _mm256_permute2f128_pd(a, b, imm8); }
template<int imm8> m128i mm_permute2x128_si128(const m128i& a, const m128i& b) noexcept { return _mm_permute2x128_si128(a, b, imm8); }
template<int imm8> m256i mm256_permute2x128_si256(const m256i& a, const m256i& b) noexcept { return _mm256_permute2x128_si256(a, b, imm8); }
template<int imm8> m256i mm256_permute2f128_si256(const m256i& a, const m256i& b) noexcept { return _mm256_permute2f128_si256(a, b, imm8); }

template<int imm8> m256i mm256_permute4x64_epi64(const m256i& a) noexcept { return _mm256_permute4x64_epi64(a, imm8); }
template<int imm8> m256d mm256_permute4x64_pd(const m256d& a) noexcept { return _mm256_permute4x64_pd(a, imm8); }

template<int imm8> m256i mm256_permutevar8x32_epi32(const m256i& a, const m256i& idx) noexcept { return _mm256_permutevar8x32_epi32(a, idx, imm8); }

inline m128 mm_sin_ps(const m128& a) noexcept { return _mm_sin_ps(a); }
inline m256 mm256_sin_ps(const m256& a) noexcept { return _mm256_sin_ps(a); }
inline m128d mm_sin_pd(const m128d& a) noexcept { return _mm_sin_pd(a); }
inline m256d mm256_sin_pd(const m256d& a) noexcept { return _mm256_sin_pd(a); }

inline m128 mm_cos_ps(const m128& a) noexcept { return _mm_cos_ps(a); }
inline m256 mm256_cos_ps(const m256& a) noexcept { return _mm256_cos_ps(a); }
inline m128d mm_cos_pd(const m128d& a) noexcept { return _mm_cos_pd(a); }
inline m256d mm256_cos_pd(const m256d& a) noexcept { return _mm256_cos_pd(a); }

inline m128 mm_tan_ps(const m128& a) noexcept { return _mm_tan_ps(a); }
inline m256 mm256_tan_ps(const m256& a) noexcept { return _mm256_tan_ps(a); }
inline m128d mm_tan_pd(const m128d& a) noexcept { return _mm_tan_pd(a); }
inline m256d mm256_tan_pd(const m256d& a) noexcept { return _mm256_tan_pd(a); }

inline m128 mm_asin_ps(const m128& a) noexcept { return _mm_asin_ps(a); }
inline m256 mm256_asin_ps(const m256& a) noexcept { return _mm256_asin_ps(a); }
inline m128d mm_asin_pd(const m128d& a) noexcept { return _mm_asin_pd(a); }
inline m256d mm256_asin_pd(const m256d& a) noexcept { return _mm256_asin_pd(a); }

inline m128 mm_acos_ps(const m128& a) noexcept { return _mm_acos_ps(a); }
inline m256 mm256_acos_ps(const m256& a) noexcept { return _mm256_acos_ps(a); }
inline m128d mm_acos_pd(const m128d& a) noexcept { return _mm_acos_pd(a); }
inline m256d mm256_acos_pd(const m256d& a) noexcept { return _mm256_acos_pd(a); }

inline m128 mm_atan_ps(const m128& a) noexcept { return _mm_atan_ps(a); }
inline m256 mm256_atan_ps(const m256& a) noexcept { return _mm256_atan_ps(a); }
inline m128d mm_atan_pd(const m128d& a) noexcept { return _mm_atan_pd(a); }
inline m256d mm256_atan_pd(const m256d& a) noexcept { return _mm256_atan_pd(a); }

inline m128 mm_atan2_ps(const m128& a, const m128& b) noexcept { return _mm_atan2_ps(a, b); }
inline m256 mm256_atan2_ps(const m256& a, const m256& b) noexcept { return _mm256_atan2_ps(a, b); }
inline m128d mm_atan2_pd(const m128d& a, const m128d& b) noexcept { return _mm_atan2_pd(a, b); }
inline m256d mm256_atan2_pd(const m256d& a, const m256d& b) noexcept { return _mm256_atan2_pd(a, b); }

inline void mm_sincos_ps(m128* mem_addr, const m128& a) noexcept { _mm_sincos_ps(mem_addr, a); }
inline void mm256_sincos_ps(m256* mem_addr, const m256& a) noexcept { _mm256_sincos_ps(mem_addr, a); }
inline void mm_sincos_pd(m128d* mem_addr, const m128d& a) noexcept { _mm_sincos_pd(mem_addr, a); }
inline void mm256_sincos_pd(m256d* mem_addr, const m256d& a) noexcept { _mm256_sincos_pd(mem_addr, a); }

inline m128 mm_sinh_ps(const m128& a) noexcept { return _mm_sinh_ps(a); }
inline m256 mm256_sinh_ps(const m256& a) noexcept { return _mm256_sinh_ps(a); }
inline m128d mm_sinh_pd(const m128d& a) noexcept { return _mm_sinh_pd(a); }
inline m256d mm256_sinh_pd(const m256d& a) noexcept { return _mm256_sinh_pd(a); }

inline m128 mm_cosh_ps(const m128& a) noexcept { return _mm_cosh_ps(a); }
inline m256 mm256_cosh_ps(const m256& a) noexcept { return _mm256_cosh_ps(a); }
inline m128d mm_cosh_pd(const m128d& a) noexcept { return _mm_cosh_pd(a); }
inline m256d mm256_cosh_pd(const m256d& a) noexcept { return _mm256_cosh_pd(a); }

inline m128 mm_tanh_ps(const m128& a) noexcept { return _mm_tanh_ps(a); }
inline m256 mm256_tanh_ps(const m256& a) noexcept { return _mm256_tanh_ps(a); }
inline m128d mm_tanh_pd(const m128d& a) noexcept { return _mm_tanh_pd(a); }
inline m256d mm256_tanh_pd(const m256d& a) noexcept { return _mm256_tanh_pd(a); }

inline m128 mm_asinh_ps(const m128& a) noexcept { return _mm_asinh_ps(a); }
inline m256 mm256_asinh_ps(const m256& a) noexcept { return _mm256_asinh_ps(a); }
inline m128d mm_asinh_pd(const m128d& a) noexcept { return _mm_asinh_pd(a); }
inline m256d mm256_asinh_pd(const m256d& a) noexcept { return _mm256_asinh_pd(a); }

inline m128 mm_acosh_ps(const m128& a) noexcept { return _mm_acosh_ps(a); }
inline m256 mm256_acosh_ps(const m256& a) noexcept { return _mm256_acosh_ps(a); }
inline m128d mm_acosh_pd(const m128d& a) noexcept { return _mm_acosh_pd(a); }
inline m256d mm256_acosh_pd(const m256d& a) noexcept { return _mm256_acosh_pd(a); }

inline m128 mm_atanh_ps(const m128& a) noexcept { return _mm_atanh_ps(a); }
inline m256 mm256_atanh_ps(const m256& a) noexcept { return _mm256_atanh_ps(a); }
inline m128d mm_atanh_pd(const m128d& a) noexcept { return _mm_atanh_pd(a); }
inline m256d mm256_atanh_pd(const m256d& a) noexcept { return _mm256_atanh_pd(a); }
}

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

template<nat N, tuple_for<xvector> Matrix, nat M = extent<Matrix>>                  //
requires (1 <= N && N <= 4 && !convertible_to<Matrix, xvector> && 1 <= M && M <= 4) //
xvector xvdot(Matrix&& m, const xvector& v) {
  auto r = xvdot<N, 0b1110>(get<0>(m), v);
  if constexpr (2 <= M) r = xvor(r, xvdot<N, 0b1101>(get<1>(m), v));
  if constexpr (3 <= M) r = xvor(r, xvdot<N, 0b1011>(get<2>(m), v));
  if constexpr (4 == M) r = xvor(r, xvdot<N, 0b0111>(get<3>(m), v));
  return r;
}

template<nat N, tuple_for<xvector> Matrix, nat M = extent<Matrix>>                  //
requires (1 <= N && N <= 4 && !convertible_to<Matrix, xvector> && 1 <= M && M <= 4) //
xvector xvdot(const xvector& v, Matrix&& m) {
  auto r = xvmul(xvpermute<0, 0, 0, 0>(v), get<0>(m));
  if constexpr (2 <= M) r = xvfma(xvpermute<1, 1, 1, 1>(v), get<1>(m), r);
  if constexpr (3 <= M) r = xvfma(xvpermute<2, 2, 2, 2>(v), get<2>(m), r);
  if constexpr (4 == M) r = xvfma(xvpermute<3, 3, 3, 3>(v), get<3>(m), r);
  return xvblend<(1 << N) - 1>(xv_zero, r);
}

template<nat N, tuple_for<xvector> Mt1, tuple_for<xvector> Mt2, nat L = extent<Mt1>, nat M = extent<Mt2>>         //
requires (1 <= N && N <= 4 && !convertible_to<Mt1, xvector> && !convertible_to<Mt2, xvector> && 1 <= M && M <= 4) //
void xvdot(const Mt1& m1, const Mt2& m2, Mt1& out) requires vassignable<Mt1&, const array<xvector, L>&> {
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
      if (4 <= i) throw "index out of range";
      return i == 0 ? x : i == 1 ? y : i == 2 ? z : w;
    } else return data()[i];
  }
  constexpr const T& operator[](nat i) const {
    if (is_cev) {
      if (4 <= i) throw "index out of range";
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
      if (4 <= i) throw "index out of range";
      return i == 0 ? x : i == 1 ? y : i == 2 ? z : w;
    } else return data()[i];
  }
  constexpr const fat4& operator[](nat i) const {
    if (is_cev) {
      if (4 <= i) throw "index out of range";
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
