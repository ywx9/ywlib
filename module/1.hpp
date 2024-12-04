#pragma once

#include "0.hpp"

#include <immintrin.h>

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
} // namespace intrin
