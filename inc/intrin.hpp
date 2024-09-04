/// \file intrin.hpp

#pragma once

#ifndef YWLIB
#include <immintrin.h>
export namespace mm {

// TYPES ///////////////////////////////////////////////////////////////////////

using m128 = __m128;
using m128i = __m128i;
using m128d = __m128d;
using m256 = __m256;
using m256i = __m256i;
using m256d = __m256d;

// ARITHMETIC //////////////////////////////////////////////////////////////////

inline __m128i add_epi16(const __m128i& a, const __m128i& b) noexcept { return _mm_add_epi16(a, b); }
inline __m256i add_epi16(const __m256i& a, const __m256i& b) noexcept { return _mm256_add_epi16(a, b); }
inline __m128i add_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_add_epi32(a, b); }
inline __m256i add_epi32(const __m256i& a, const __m256i& b) noexcept { return _mm256_add_epi32(a, b); }
inline __m128i add_epi64(const __m128i& a, const __m128i& b) noexcept { return _mm_add_epi64(a, b); }
inline __m256i add_epi64(const __m256i& a, const __m256i& b) noexcept { return _mm256_add_epi64(a, b); }
inline __m128i add_epi8(const __m128i& a, const __m128i& b) noexcept { return _mm_add_epi8(a, b); }
inline __m256i add_epi8(const __m256i& a, const __m256i& b) noexcept { return _mm256_add_epi8(a, b); }
inline __m128d add_pd(const __m128d& a, const __m128d& b) noexcept { return _mm_add_pd(a, b); }
inline __m256d add_pd(const __m256d& a, const __m256d& b) noexcept { return _mm256_add_pd(a, b); }
inline __m128 add_ps(const __m128& a, const __m128& b) noexcept { return _mm_add_ps(a, b); }
inline __m256 add_ps(const __m256& a, const __m256& b) noexcept { return _mm256_add_ps(a, b); }
inline __m128d add_sd(const __m128d& a, const __m128d& b) noexcept { return _mm_add_sd(a, b); }
inline __m128 add_ss(const __m128& a, const __m128& b) noexcept { return _mm_add_ss(a, b); }
inline __m128i adds_epi16(const __m128i& a, const __m128i& b) noexcept { return _mm_adds_epi16(a, b); }
inline __m256i adds_epi16(const __m256i& a, const __m256i& b) noexcept { return _mm256_adds_epi16(a, b); }
inline __m128i adds_epi8(const __m128i& a, const __m128i& b) noexcept { return _mm_adds_epi8(a, b); }
inline __m256i adds_epi8(const __m256i& a, const __m256i& b) noexcept { return _mm256_adds_epi8(a, b); }
inline __m128i adds_epu16(const __m128i& a, const __m128i& b) noexcept { return _mm_adds_epu16(a, b); }
inline __m256i adds_epu16(const __m256i& a, const __m256i& b) noexcept { return _mm256_adds_epu16(a, b); }
inline __m128i adds_epu8(const __m128i& a, const __m128i& b) noexcept { return _mm_adds_epu8(a, b); }
inline __m256i adds_epu8(const __m256i& a, const __m256i& b) noexcept { return _mm256_adds_epu8(a, b); }
inline __m128d addsub_pd(const __m128d& a, const __m128d& b) noexcept { return _mm_addsub_pd(a, b); }
inline __m256d addsub_pd(const __m256d& a, const __m256d& b) noexcept { return _mm256_addsub_pd(a, b); }
inline __m128 addsub_ps(const __m128& a, const __m128& b) noexcept { return _mm_addsub_ps(a, b); }
inline __m256 addsub_ps(const __m256& a, const __m256& b) noexcept { return _mm256_addsub_ps(a, b); }
inline __m128i div_epi16(const __m128i& a, const __m128i& b) noexcept { return _mm_div_epi16(a, b); }
inline __m256i div_epi16(const __m256i& a, const __m256i& b) noexcept { return _mm256_div_epi16(a, b); }
inline __m128i div_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_div_epi32(a, b); }
inline __m256i div_epi32(const __m256i& a, const __m256i& b) noexcept { return _mm256_div_epi32(a, b); }
inline __m128i div_epi64(const __m128i& a, const __m128i& b) noexcept { return _mm_div_epi64(a, b); }
inline __m256i div_epi64(const __m256i& a, const __m256i& b) noexcept { return _mm256_div_epi64(a, b); }
inline __m128i div_epi8(const __m128i& a, const __m128i& b) noexcept { return _mm_div_epi8(a, b); }
inline __m256i div_epi8(const __m256i& a, const __m256i& b) noexcept { return _mm256_div_epi8(a, b); }
inline __m128i div_epu16(const __m128i& a, const __m128i& b) noexcept { return _mm_div_epu16(a, b); }
inline __m256i div_epu16(const __m256i& a, const __m256i& b) noexcept { return _mm256_div_epu16(a, b); }
inline __m128i div_epu32(const __m128i& a, const __m128i& b) noexcept { return _mm_div_epu32(a, b); }
inline __m256i div_epu32(const __m256i& a, const __m256i& b) noexcept { return _mm256_div_epu32(a, b); }
inline __m128i div_epu64(const __m128i& a, const __m128i& b) noexcept { return _mm_div_epu64(a, b); }
inline __m256i div_epu64(const __m256i& a, const __m256i& b) noexcept { return _mm256_div_epu64(a, b); }
inline __m128i div_epu8(const __m128i& a, const __m128i& b) noexcept { return _mm_div_epu8(a, b); }
inline __m256i div_epu8(const __m256i& a, const __m256i& b) noexcept { return _mm256_div_epu8(a, b); }
inline __m128d div_pd(const __m128d& a, const __m128d& b) noexcept { return _mm_div_pd(a, b); }
inline __m256d div_pd(const __m256d& a, const __m256d& b) noexcept { return _mm256_div_pd(a, b); }
inline __m128 div_ps(const __m128& a, const __m128& b) noexcept { return _mm_div_ps(a, b); }
inline __m256 div_ps(const __m256& a, const __m256& b) noexcept { return _mm256_div_ps(a, b); }
inline __m128d div_sd(const __m128d& a, const __m128d& b) noexcept { return _mm_div_sd(a, b); }
inline __m128 div_ss(const __m128& a, const __m128& b) noexcept { return _mm_div_ss(a, b); }
template<int imm8> __m128d dp_pd(const __m128d& a, const __m128d& b) noexcept { return _mm_dp_pd(a, b, imm8); }
template<int imm8> __m128 dp_ps(const __m128& a, const __m128& b) noexcept { return _mm_dp_ps(a, b, imm8); }
template<int imm8> __m256 dp_ps(const __m256& a, const __m256& b) noexcept { return _mm256_dp_ps(a, b, imm8); }
inline __m128i dpbssd_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_dpbssd_epi32(a, b, 0); }
inline __m256i dpbssd_epi32(const __m256i& a, const __m256i& b) noexcept { return _mm256_dpbssd_epi32(a, b, 0); }
inline __m128i dpbssds_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_dpbssds_epi32(a, b, 0); }
inline __m256i dpbssds_epi32(const __m256i& a, const __m256i& b) noexcept { return _mm256_dpbssds_epi32(a, b, 0); }
inline __m128i dpbsud_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_dpbsud_epi32(a, b, 0); }
inline __m256i dpbsud_epi32(const __m256i& a, const __m256i& b) noexcept { return _mm256_dpbsud_epi32(a, b, 0); }
inline __m128i dpbsuds_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_dpbsuds_epi32(a, b, 0); }
inline __m256i dpbsuds_epi32(const __m256i& a, const __m256i& b) noexcept { return _mm256_dpbsuds_epi32(a, b, 0); }
inline __m128i dpbusd_avx_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_dpbusd_avx_epi32(a, b, 0); }
inline __m256i dpbusd_avx_epi32(const __m256i& a, const __m256i& b) noexcept { return _mm256_dpbusd_avx_epi32(a, b, 0); }
inline __m128i dpbusd_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_dpbusd_epi32(a, b, 0); }
inline __m256i dpbusd_epi32(const __m256i& a, const __m256i& b) noexcept { return _mm256_dpbusd_epi32(a, b, 0); }
inline __m128i dpbusds_avx_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_dpbusds_avx_epi32(a, b, 0); }
inline __m256i dpbusds_avx_epi32(const __m256i& a, const __m256i& b) noexcept { return _mm256_dpbusds_avx_epi32(a, b, 0); }
inline __m128i dpbusds_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_dpbusds_epi32(a, b, 0); }
inline __m256i dpbusds_epi32(const __m256i& a, const __m256i& b) noexcept { return _mm256_dpbusds_epi32(a, b, 0); }
inline __m128i dpbuud_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_dpbuud_epi32(a, b, 0); }
inline __m256i dpbuud_epi32(const __m256i& a, const __m256i& b) noexcept { return _mm256_dpbuud_epi32(a, b, 0); }
inline __m128i dpbuuds_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_dpbuuds_epi32(a, b, 0); }
inline __m256i dpbuuds_epi32(const __m256i& a, const __m256i& b) noexcept { return _mm256_dpbuuds_epi32(a, b, 0); }
inline __m128i dpwssd_avx_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_dpwssd_avx_epi32(a, b, 0); }
inline __m256i dpwssd_avx_epi32(const __m256i& a, const __m256i& b) noexcept { return _mm256_dpwssd_avx_epi32(a, b, 0); }
inline __m128i dpwssd_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_dpwssd_epi32(a, b, 0); }
inline __m256i dpwssd_epi32(const __m256i& a, const __m256i& b) noexcept { return _mm256_dpwssd_epi32(a, b, 0); }
inline __m128i dpwssds_avx_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_dpwssds_avx_epi32(a, b, 0); }
inline __m256i dpwssds_avx_epi32(const __m256i& a, const __m256i& b) noexcept { return _mm256_dpwssds_avx_epi32(a, b, 0); }
inline __m128i dpwssds_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_dpwssds_epi32(a, b, 0); }
inline __m256i dpwssds_epi32(const __m256i& a, const __m256i& b) noexcept { return _mm256_dpwssds_epi32(a, b, 0); }
inline __m128i dpwsud_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_dpwsud_epi32(a, b, 0); }
inline __m256i dpwsud_epi32(const __m256i& a, const __m256i& b) noexcept { return _mm256_dpwsud_epi32(a, b, 0); }
inline __m128i dpwsuds_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_dpwsuds_epi32(a, b, 0); }
inline __m256i dpwsuds_epi32(const __m256i& a, const __m256i& b) noexcept { return _mm256_dpwsuds_epi32(a, b, 0); }
inline __m128i dpwusd_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_dpwusd_epi32(a, b, 0); }
inline __m256i dpwusd_epi32(const __m256i& a, const __m256i& b) noexcept { return _mm256_dpwusd_epi32(a, b, 0); }
inline __m128i dpwusds_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_dpwusds_epi32(a, b, 0); }
inline __m256i dpwusds_epi32(const __m256i& a, const __m256i& b) noexcept { return _mm256_dpwusds_epi32(a, b, 0); }
inline __m128i dpwuud_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_dpwuud_epi32(a, b, 0); }
inline __m256i dpwuud_epi32(const __m256i& a, const __m256i& b) noexcept { return _mm256_dpwuud_epi32(a, b, 0); }
inline __m128i dpwuuds_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_dpwuuds_epi32(a, b, 0); }
inline __m256i dpwuuds_epi32(const __m256i& a, const __m256i& b) noexcept { return _mm256_dpwuuds_epi32(a, b, 0); }
inline __m128d erf_pd(const __m128d& a) noexcept { return _mm_erf_pd(a); }
inline __m128d fmadd_pd(const __m128d& a, const __m128d& b, const __m128d& c) noexcept { return _mm_fmadd_pd(a, b, c); }
inline __m256d fmadd_pd(const __m256d& a, const __m256d& b, const __m256d& c) noexcept { return _mm256_fmadd_pd(a, b, c); }
inline __m128 fmadd_ps(const __m128& a, const __m128& b, const __m128& c) noexcept { return _mm_fmadd_ps(a, b, c); }
inline __m256 fmadd_ps(const __m256& a, const __m256& b, const __m256& c) noexcept { return _mm256_fmadd_ps(a, b, c); }
inline __m128d fmadd_sd(const __m128d& a, const __m128d& b, const __m128d& c) noexcept { return _mm_fmadd_sd(a, b, c); }
inline __m128 fmadd_ss(const __m128& a, const __m128& b, const __m128& c) noexcept { return _mm_fmadd_ss(a, b, c); }
inline __m128d fmaddsub_pd(const __m128d& a, const __m128d& b, const __m128d& c) noexcept { return _mm_fmaddsub_pd(a, b, c); }
inline __m256d fmaddsub_pd(const __m256d& a, const __m256d& b, const __m256d& c) noexcept { return _mm256_fmaddsub_pd(a, b, c); }
inline __m128 fmaddsub_ps(const __m128& a, const __m128& b, const __m128& c) noexcept { return _mm_fmaddsub_ps(a, b, c); }
inline __m256 fmaddsub_ps(const __m256& a, const __m256& b, const __m256& c) noexcept { return _mm256_fmaddsub_ps(a, b, c); }
inline __m128d fmsub_pd(const __m128d& a, const __m128d& b, const __m128d& c) noexcept { return _mm_fmsub_pd(a, b, c); }
inline __m256d fmsub_pd(const __m256d& a, const __m256d& b, const __m256d& c) noexcept { return _mm256_fmsub_pd(a, b, c); }
inline __m128 fmsub_ps(const __m128& a, const __m128& b, const __m128& c) noexcept { return _mm_fmsub_ps(a, b, c); }
inline __m256 fmsub_ps(const __m256& a, const __m256& b, const __m256& c) noexcept { return _mm256_fmsub_ps(a, b, c); }
inline __m128d fmsub_sd(const __m128d& a, const __m128d& b, const __m128d& c) noexcept { return _mm_fmsub_sd(a, b, c); }
inline __m128 fmsub_ss(const __m128& a, const __m128& b, const __m128& c) noexcept { return _mm_fmsub_ss(a, b, c); }
inline __m128d fmsubadd_pd(const __m128d& a, const __m128d& b, const __m128d& c) noexcept { return _mm_fmsubadd_pd(a, b, c); }
inline __m128 fmsubadd_ps(const __m128& a, const __m128& b, const __m128& c) noexcept { return _mm_fmsubadd_ps(a, b, c); }
inline __m256 fmsubadd_ps(const __m256& a, const __m256& b, const __m256& c) noexcept { return _mm256_fmsubadd_ps(a, b, c); }
inline __m128d fnmadd_pd(const __m128d& a, const __m128d& b, const __m128d& c) noexcept { return _mm_fnmadd_pd(a, b, c); }
inline __m256d fnmadd_pd(const __m256d& a, const __m256d& b, const __m256d& c) noexcept { return _mm256_fnmadd_pd(a, b, c); }
inline __m128 fnmadd_ps(const __m128& a, const __m128& b, const __m128& c) noexcept { return _mm_fnmadd_ps(a, b, c); }
inline __m256 fnmadd_ps(const __m256& a, const __m256& b, const __m256& c) noexcept { return _mm256_fnmadd_ps(a, b, c); }
inline __m128d fnmadd_sd(const __m128d& a, const __m128d& b, const __m128d& c) noexcept { return _mm_fnmadd_sd(a, b, c); }
inline __m128 fnmadd_ss(const __m128& a, const __m128& b, const __m128& c) noexcept { return _mm_fnmadd_ss(a, b, c); }
inline __m128d fnmsub_pd(const __m128d& a, const __m128d& b, const __m128d& c) noexcept { return _mm_fnmsub_pd(a, b, c); }
inline __m256d fnmsub_pd(const __m256d& a, const __m256d& b, const __m256d& c) noexcept { return _mm256_fnmsub_pd(a, b, c); }
inline __m128 fnmsub_ps(const __m128& a, const __m128& b, const __m128& c) noexcept { return _mm_fnmsub_ps(a, b, c); }
inline __m256 fnmsub_ps(const __m256& a, const __m256& b, const __m256& c) noexcept { return _mm256_fnmsub_ps(a, b, c); }
inline __m128d fnmsub_sd(const __m128d& a, const __m128d& b, const __m128d& c) noexcept { return _mm_fnmsub_sd(a, b, c); }
inline __m128 fnmsub_ss(const __m128& a, const __m128& b, const __m128& c) noexcept { return _mm_fnmsub_ss(a, b, c); }
inline __m128i hadd_epi16(const __m128i& a, const __m128i& b) noexcept { return _mm_hadd_epi16(a, b); }
inline __m256i hadd_epi16(const __m256i& a, const __m256i& b) noexcept { return _mm256_hadd_epi16(a, b); }
inline __m128i hadd_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_hadd_epi32(a, b); }
inline __m256i hadd_epi32(const __m256i& a, const __m256i& b) noexcept { return _mm256_hadd_epi32(a, b); }
inline __m128d hadd_pd(const __m128d& a, const __m128d& b) noexcept { return _mm_hadd_pd(a, b); }
inline __m256d hadd_pd(const __m256d& a, const __m256d& b) noexcept { return _mm256_hadd_pd(a, b); }
inline __m128 hadd_ps(const __m128& a, const __m128& b) noexcept { return _mm_hadd_ps(a, b); }
inline __m256 hadd_ps(const __m256& a, const __m256& b) noexcept { return _mm256_hadd_ps(a, b); }
inline __m128i hadds_epi16(const __m128i& a, const __m128i& b) noexcept { return _mm_hadds_epi16(a, b); }
inline __m256i hadds_epi16(const __m256i& a, const __m256i& b) noexcept { return _mm256_hadds_epi16(a, b); }
inline __m128i hsub_epi16(const __m128i& a, const __m128i& b) noexcept { return _mm_hsub_epi16(a, b); }
inline __m256i hsub_epi16(const __m256i& a, const __m256i& b) noexcept { return _mm256_hsub_epi16(a, b); }
inline __m128i hsub_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_hsub_epi32(a, b); }
inline __m256i hsub_epi32(const __m256i& a, const __m256i& b) noexcept { return _mm256_hsub_epi32(a, b); }
inline __m128d hsub_pd(const __m128d& a, const __m128d& b) noexcept { return _mm_hsub_pd(a, b); }
inline __m256d hsub_pd(const __m256d& a, const __m256d& b) noexcept { return _mm256_hsub_pd(a, b); }
inline __m128 hsub_ps(const __m128& a, const __m128& b) noexcept { return _mm_hsub_ps(a, b); }
inline __m256 hsub_ps(const __m256& a, const __m256& b) noexcept { return _mm256_hsub_ps(a, b); }
inline __m128i hsubs_epi16(const __m128i& a, const __m128i& b) noexcept { return _mm_hsubs_epi16(a, b); }
inline __m256i hsubs_epi16(const __m256i& a, const __m256i& b) noexcept { return _mm256_hsubs_epi16(a, b); }
inline __m128i idiv_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_idiv_epi32(a, b); }
inline __m256i idiv_epi32(const __m256i& a, const __m256i& b) noexcept { return _mm256_idiv_epi32(a, b); }
inline __m128i idivrem_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_idivrem_epi32(a, b); }
inline __m256i idivrem_epi32(const __m256i& a, const __m256i& b) noexcept { return _mm256_idivrem_epi32(a, b); }
inline __m128i irem_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_irem_epi32(a, b); }
inline __m256i irem_epi32(const __m256i& a, const __m256i& b) noexcept { return _mm256_irem_epi32(a, b); }
inline __m128i madd_epi16(const __m128i& a, const __m128i& b) noexcept { return _mm_madd_epi16(a, b); }
inline __m256i madd_epi16(const __m256i& a, const __m256i& b) noexcept { return _mm256_madd_epi16(a, b); }
inline __m128i madd52hi_avx_epu64(const __m128i& a, const __m128i& b) noexcept { return _mm_madd52hi_avx_epu64(a, b); }
inline __m256i madd52hi_avx_epu64(const __m256i& a, const __m256i& b) noexcept { return _mm256_madd52hi_avx_epu64(a, b); }
inline __m128i madd52hi_epu64(const __m128i& a, const __m128i& b) noexcept { return _mm_madd52hi_epu64(a, b); }
inline __m256i madd52hi_epu64(const __m256i& a, const __m256i& b) noexcept { return _mm256_madd52hi_epu64(a, b); }
inline __m128i madd52lo_avx_epu64(const __m128i& a, const __m128i& b) noexcept { return _mm_madd52lo_avx_epu64(a, b); }
inline __m256i madd52lo_avx_epu64(const __m256i& a, const __m256i& b) noexcept { return _mm256_madd52lo_avx_epu64(a, b); }
inline __m128i madd52lo_epu64(const __m128i& a, const __m128i& b) noexcept { return _mm_madd52lo_epu64(a, b); }
inline __m256i madd52lo_epu64(const __m256i& a, const __m256i& b) noexcept { return _mm256_madd52lo_epu64(a, b); }
inline __m128i maddubs_epi16(const __m128i& a, const __m128i& b) noexcept { return _mm_maddubs_epi16(a, b); }
inline __m256i maddubs_epi16(const __m256i& a, const __m256i& b) noexcept { return _mm256_maddubs_epi16(a, b); }
inline __m128i mpsadbw_epu8(const __m128i& a, const __m128i& b) noexcept { return _mm_mpsadbw_epu8(a, b, 0); }
inline __m128i mul_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_mul_epi32(a, b); }
inline __m256i mul_epi32(const __m256i& a, const __m256i& b) noexcept { return _mm256_mul_epi32(a, b); }
inline __m128i mul_epu32(const __m128i& a, const __m128i& b) noexcept { return _mm_mul_epu32(a, b); }
inline __m256i mul_epu32(const __m256i& a, const __m256i& b) noexcept { return _mm256_mul_epu32(a, b); }
inline __m128d mul_pd(const __m128d& a, const __m128d& b) noexcept { return _mm_mul_pd(a, b); }
inline __m256d mul_pd(const __m256d& a, const __m256d& b) noexcept { return _mm256_mul_pd(a, b); }
inline __m128 mul_ps(const __m128& a, const __m128& b) noexcept { return _mm_mul_ps(a, b); }
inline __m256 mul_ps(const __m256& a, const __m256& b) noexcept { return _mm256_mul_ps(a, b); }
inline __m128d mul_sd(const __m128d& a, const __m128d& b) noexcept { return _mm_mul_sd(a, b); }
inline __m128 mul_ss(const __m128& a, const __m128& b) noexcept { return _mm_mul_ss(a, b); }
inline __m128i mulhi_epi16(const __m128i& a, const __m128i& b) noexcept { return _mm_mulhi_epi16(a, b); }
inline __m256i mulhi_epi16(const __m256i& a, const __m256i& b) noexcept { return _mm256_mulhi_epi16(a, b); }
inline __m128i mulhi_epu16(const __m128i& a, const __m128i& b) noexcept { return _mm_mulhi_epu16(a, b); }
inline __m256i mulhi_epu16(const __m256i& a, const __m256i& b) noexcept { return _mm256_mulhi_epu16(a, b); }
inline __m128i mulhrs_epi16(const __m128i& a, const __m128i& b) noexcept { return _mm_mulhrs_epi16(a, b); }
inline __m256i mulhrs_epi16(const __m256i& a, const __m256i& b) noexcept { return _mm256_mulhrs_epi16(a, b); }
inline __m128i mullo_epi16(const __m128i& a, const __m128i& b) noexcept { return _mm_mullo_epi16(a, b); }
inline __m256i mullo_epi16(const __m256i& a, const __m256i& b) noexcept { return _mm256_mullo_epi16(a, b); }
inline __m128i mullo_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_mullo_epi32(a, b); }
inline __m256i mullo_epi32(const __m256i& a, const __m256i& b) noexcept { return _mm256_mullo_epi32(a, b); }
inline __m128i rem_epi16(const __m128i& a, const __m128i& b) noexcept { return _mm_rem_epi16(a, b); }
inline __m256i rem_epi16(const __m256i& a, const __m256i& b) noexcept { return _mm256_rem_epi16(a, b); }
inline __m128i rem_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_rem_epi32(a, b); }
inline __m256i rem_epi32(const __m256i& a, const __m256i& b) noexcept { return _mm256_rem_epi32(a, b); }
inline __m128i rem_epi64(const __m128i& a, const __m128i& b) noexcept { return _mm_rem_epi64(a, b); }
inline __m256i rem_epi64(const __m256i& a, const __m256i& b) noexcept { return _mm256_rem_epi64(a, b); }
inline __m128i rem_epi8(const __m128i& a, const __m128i& b) noexcept { return _mm_rem_epi8(a, b); }
inline __m256i rem_epi8(const __m256i& a, const __m256i& b) noexcept { return _mm256_rem_epi8(a, b); }
inline __m128i rem_epu16(const __m128i& a, const __m128i& b) noexcept { return _mm_rem_epu16(a, b); }
inline __m256i rem_epu16(const __m256i& a, const __m256i& b) noexcept { return _mm256_rem_epu16(a, b); }
inline __m128i rem_epu32(const __m128i& a, const __m128i& b) noexcept { return _mm_rem_epu32(a, b); }
inline __m256i rem_epu32(const __m256i& a, const __m256i& b) noexcept { return _mm256_rem_epu32(a, b); }
inline __m128i rem_epu64(const __m128i& a, const __m128i& b) noexcept { return _mm_rem_epu64(a, b); }
inline __m256i rem_epu64(const __m256i& a, const __m256i& b) noexcept { return _mm256_rem_epu64(a, b); }
inline __m128i rem_epu8(const __m128i& a, const __m128i& b) noexcept { return _mm_rem_epu8(a, b); }
inline __m256i rem_epu8(const __m256i& a, const __m256i& b) noexcept { return _mm256_rem_epu8(a, b); }
inline __m128i sad_epu8(const __m128i& a, const __m128i& b) noexcept { return _mm_sad_epu8(a, b); }
inline __m256i sad_epu8(const __m256i& a, const __m256i& b) noexcept { return _mm256_sad_epu8(a, b); }
inline __m128i sign_epi16(const __m128i& a, const __m128i& b) noexcept { return _mm_sign_epi16(a, b); }
inline __m256i sign_epi16(const __m256i& a, const __m256i& b) noexcept { return _mm256_sign_epi16(a, b); }
inline __m128i sign_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_sign_epi32(a, b); }
inline __m256i sign_epi32(const __m256i& a, const __m256i& b) noexcept { return _mm256_sign_epi32(a, b); }
inline __m128i sign_epi8(const __m128i& a, const __m128i& b) noexcept { return _mm_sign_epi8(a, b); }
inline __m256i sign_epi8(const __m256i& a, const __m256i& b) noexcept { return _mm256_sign_epi8(a, b); }
inline __m128i sub_epi16(const __m128i& a, const __m128i& b) noexcept { return _mm_sub_epi16(a, b); }
inline __m256i sub_epi16(const __m256i& a, const __m256i& b) noexcept { return _mm256_sub_epi16(a, b); }
inline __m128i sub_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_sub_epi32(a, b); }
inline __m256i sub_epi32(const __m256i& a, const __m256i& b) noexcept { return _mm256_sub_epi32(a, b); }
inline __m128i sub_epi64(const __m128i& a, const __m128i& b) noexcept { return _mm_sub_epi64(a, b); }
inline __m256i sub_epi64(const __m256i& a, const __m256i& b) noexcept { return _mm256_sub_epi64(a, b); }
inline __m128i sub_epi8(const __m128i& a, const __m128i& b) noexcept { return _mm_sub_epi8(a, b); }
inline __m256i sub_epi8(const __m256i& a, const __m256i& b) noexcept { return _mm256_sub_epi8(a, b); }
inline __m128d sub_pd(const __m128d& a, const __m128d& b) noexcept { return _mm_sub_pd(a, b); }
inline __m256d sub_pd(const __m256d& a, const __m256d& b) noexcept { return _mm256_sub_pd(a, b); }
inline __m128 sub_ps(const __m128& a, const __m128& b) noexcept { return _mm_sub_ps(a, b); }
inline __m256 sub_ps(const __m256& a, const __m256& b) noexcept { return _mm256_sub_ps(a, b); }
inline __m128d sub_sd(const __m128d& a, const __m128d& b) noexcept { return _mm_sub_sd(a, b); }
inline __m128 sub_ss(const __m128& a, const __m128& b) noexcept { return _mm_sub_ss(a, b); }
inline __m128i subs_epi16(const __m128i& a, const __m128i& b) noexcept { return _mm_subs_epi16(a, b); }
inline __m256i subs_epi16(const __m256i& a, const __m256i& b) noexcept { return _mm256_subs_epi16(a, b); }
inline __m128i subs_epi8(const __m128i& a, const __m128i& b) noexcept { return _mm_subs_epi8(a, b); }
inline __m256i subs_epi8(const __m256i& a, const __m256i& b) noexcept { return _mm256_subs_epi8(a, b); }
inline __m128i subs_epu16(const __m128i& a, const __m128i& b) noexcept { return _mm_subs_epu16(a, b); }
inline __m256i subs_epu16(const __m256i& a, const __m256i& b) noexcept { return _mm256_subs_epu16(a, b); }
inline __m128i subs_epu8(const __m128i& a, const __m128i& b) noexcept { return _mm_subs_epu8(a, b); }
inline __m256i subs_epu8(const __m256i& a, const __m256i& b) noexcept { return _mm256_subs_epu8(a, b); }
inline __m128i udiv_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_udiv_epi32(a, b); }
inline __m256i udiv_epi32(const __m256i& a, const __m256i& b) noexcept { return _mm256_udiv_epi32(a, b); }
inline __m128i udivrem_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_udivrem_epi32(a, b); }
inline __m256i udivrem_epi32(const __m256i& a, const __m256i& b) noexcept { return _mm256_udivrem_epi32(a, b); }
inline __m128i urem_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_urem_epi32(a, b); }
inline __m256i urem_epi32(const __m256i& a, const __m256i& b) noexcept { return _mm256_urem_epi32(a, b); }

// BIT MANIPULATION ////////////////////////////////////////////////////////////

// CAST ////////////////////////////////////////////////////////////////////////

inline __m128 castpd_ps(const __m128d& a) noexcept { return _mm_castpd_ps(a); }
inline __m256 castpd_ps(const __m256d& a) noexcept { return _mm256_castpd_ps(a); }
inline __m128i castpd_si128(const __m128d& a) noexcept { return _mm_castpd_si128(a); }
inline __m256i castpd_si256(const __m256d& a) noexcept { return _mm256_castpd_si256(a); }
inline __m256d castpd128_pd256(const __m128d& a) noexcept { return _mm256_castpd128_pd256(a); }
inline __m128d castpd256_pd128(const __m256d& a) noexcept { return _mm256_castpd256_pd128(a); }
inline __m128d castps_pd(const __m128& a) noexcept { return _mm_castps_pd(a); }
inline __m256d castps_pd(const __m256& a) noexcept { return _mm256_castps_pd(a); }
inline __m128i castps_si128(const __m128& a) noexcept { return _mm_castps_si128(a); }
inline __m256i castps_si256(const __m256& a) noexcept { return _mm256_castps_si256(a); }
inline __m256 castps128_ps256(const __m128& a) noexcept { return _mm256_castps128_ps256(a); }
inline __m128 castps256_ps128(const __m256& a) noexcept { return _mm256_castps256_ps128(a); }
inline __m128d castsi128_pd(const __m128i& a) noexcept { return _mm_castsi128_pd(a); }
inline __m128 castsi128_ps(const __m128i& a) noexcept { return _mm_castsi128_ps(a); }
inline __m256i castsi128_si256(const __m128i& a) noexcept { return _mm256_castsi128_si256(a); }
inline __m256d castsi256_pd(const __m256i& a) noexcept { return _mm256_castsi256_pd(a); }
inline __m256 castsi256_ps(const __m256i& a) noexcept { return _mm256_castsi256_ps(a); }
inline __m128i castsi256_si128(const __m256i& a) noexcept { return _mm256_castsi256_si128(a); }
inline __m256d zextpd128_pd256(const __m128d& a) noexcept { return _mm256_zextpd128_pd256(a); }
inline __m256 zextps128_ps256(const __m128& a) noexcept { return _mm256_zextps128_ps256(a); }
inline __m256i zextsi128_si256(const __m128i& a) noexcept { return _mm256_zextsi128_si256(a); }

// COMPARE /////////////////////////////////////////////////////////////////////

template<int imm8> __m128d cmp_pd(const __m128d& a, const __m128d& b) noexcept { return _mm_cmp_pd(a, b, imm8); }
template<int imm8> __m256d cmp_pd(const __m256d& a, const __m256d& b) noexcept { return _mm256_cmp_pd(a, b, imm8); }
template<int imm8> __m128 cmp_ps(const __m128& a, const __m128& b) noexcept { return _mm_cmp_ps(a, b, imm8); }
template<int imm8> __m256 cmp_ps(const __m256& a, const __m256& b) noexcept { return _mm256_cmp_ps(a, b, imm8); }
template<int imm8> __m128d cmp_sd(const __m128d& a, const __m128d& b) noexcept { return _mm_cmp_sd(a, b, imm8); }
template<int imm8> __m128 cmp_ss(const __m128& a, const __m128& b) noexcept { return _mm_cmp_ss(a, b, imm8); }
inline __m128i cmpeq_epi16(const __m128i& a, const __m128i& b) noexcept { return _mm_cmpeq_epi16(a, b); }
inline __m256i cmpeq_epi16(const __m256i& a, const __m256i& b) noexcept { return _mm256_cmpeq_epi16(a, b); }
inline __m128i cmpeq_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_cmpeq_epi32(a, b); }
inline __m256i cmpeq_epi32(const __m256i& a, const __m256i& b) noexcept { return _mm256_cmpeq_epi32(a, b); }
inline __m128i cmpeq_epi64(const __m128i& a, const __m128i& b) noexcept { return _mm_cmpeq_epi64(a, b); }
inline __m256i cmpeq_epi64(const __m256i& a, const __m256i& b) noexcept { return _mm256_cmpeq_epi64(a, b); }
inline __m128i cmpeq_epi8(const __m128i& a, const __m128i& b) noexcept { return _mm_cmpeq_epi8(a, b); }
inline __m256i cmpeq_epi8(const __m256i& a, const __m256i& b) noexcept { return _mm256_cmpeq_epi8(a, b); }
inline __m128d cmpeq_pd(const __m128d& a, const __m128d& b) noexcept { return _mm_cmpeq_pd(a, b); }
inline __m128 cmpeq_ps(const __m128& a, const __m128& b) noexcept { return _mm_cmpeq_ps(a, b); }
inline __m128d cmpeq_sd(const __m128d& a, const __m128d& b) noexcept { return _mm_cmpeq_sd(a, b); }
inline __m128 cmpeq_ss(const __m128& a, const __m128& b) noexcept { return _mm_cmpeq_ss(a, b); }
inline __m128d cmpge_pd(const __m128d& a, const __m128d& b) noexcept { return _mm_cmpge_pd(a, b); }
inline __m128 cmpge_ps(const __m128& a, const __m128& b) noexcept { return _mm_cmpge_ps(a, b); }
inline __m128d cmpge_sd(const __m128d& a, const __m128d& b) noexcept { return _mm_cmpge_sd(a, b); }
inline __m128 cmpge_ss(const __m128& a, const __m128& b) noexcept { return _mm_cmpge_ss(a, b); }
inline __m128i cmpgt_epi16(const __m128i& a, const __m128i& b) noexcept { return _mm_cmpgt_epi16(a, b); }
inline __m256i cmpgt_epi16(const __m256i& a, const __m256i& b) noexcept { return _mm256_cmpgt_epi16(a, b); }
inline __m128i cmpgt_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_cmpgt_epi32(a, b); }
inline __m256i cmpgt_epi32(const __m256i& a, const __m256i& b) noexcept { return _mm256_cmpgt_epi32(a, b); }
inline __m128i cmpgt_epi64(const __m128i& a, const __m128i& b) noexcept { return _mm_cmpgt_epi64(a, b); }
inline __m256i cmpgt_epi64(const __m256i& a, const __m256i& b) noexcept { return _mm256_cmpgt_epi64(a, b); }
inline __m128i cmpgt_epi8(const __m128i& a, const __m128i& b) noexcept { return _mm_cmpgt_epi8(a, b); }
inline __m256i cmpgt_epi8(const __m256i& a, const __m256i& b) noexcept { return _mm256_cmpgt_epi8(a, b); }
inline __m128d cmpgt_pd(const __m128d& a, const __m128d& b) noexcept { return _mm_cmpgt_pd(a, b); }
inline __m128 cmpgt_ps(const __m128& a, const __m128& b) noexcept { return _mm_cmpgt_ps(a, b); }
inline __m128d cmpgt_sd(const __m128d& a, const __m128d& b) noexcept { return _mm_cmpgt_sd(a, b); }
inline __m128 cmpgt_ss(const __m128& a, const __m128& b) noexcept { return _mm_cmpgt_ss(a, b); }
inline __m128d cmple_pd(const __m128d& a, const __m128d& b) noexcept { return _mm_cmple_pd(a, b); }
inline __m128 cmple_ps(const __m128& a, const __m128& b) noexcept { return _mm_cmple_ps(a, b); }
inline __m128d cmple_sd(const __m128d& a, const __m128d& b) noexcept { return _mm_cmple_sd(a, b); }
inline __m128 cmple_ss(const __m128& a, const __m128& b) noexcept { return _mm_cmple_ss(a, b); }
inline __m128i cmplt_epi16(const __m128i& a, const __m128i& b) noexcept { return _mm_cmplt_epi16(a, b); }
inline __m128i cmplt_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_cmplt_epi32(a, b); }
inline __m128i cmplt_epi8(const __m128i& a, const __m128i& b) noexcept { return _mm_cmplt_epi8(a, b); }
inline __m128d cmplt_pd(const __m128d& a, const __m128d& b) noexcept { return _mm_cmplt_pd(a, b); }
inline __m128 cmplt_ps(const __m128& a, const __m128& b) noexcept { return _mm_cmplt_ps(a, b); }
inline __m128d cmplt_sd(const __m128d& a, const __m128d& b) noexcept { return _mm_cmplt_sd(a, b); }
inline __m128 cmplt_ss(const __m128& a, const __m128& b) noexcept { return _mm_cmplt_ss(a, b); }
inline __m128d cmpneq_pd(const __m128d& a, const __m128d& b) noexcept { return _mm_cmpneq_pd(a, b); }
inline __m128 cmpneq_ps(const __m128& a, const __m128& b) noexcept { return _mm_cmpneq_ps(a, b); }
inline __m128d cmpneq_sd(const __m128d& a, const __m128d& b) noexcept { return _mm_cmpneq_sd(a, b); }
inline __m128 cmpneq_ss(const __m128& a, const __m128& b) noexcept { return _mm_cmpneq_ss(a, b); }
inline __m128d cmpnge_pd(const __m128d& a, const __m128d& b) noexcept { return _mm_cmpnge_pd(a, b); }
inline __m128 cmpnge_ps(const __m128& a, const __m128& b) noexcept { return _mm_cmpnge_ps(a, b); }
inline __m128d cmpnge_sd(const __m128d& a, const __m128d& b) noexcept { return _mm_cmpnge_sd(a, b); }
inline __m128 cmpnge_ss(const __m128& a, const __m128& b) noexcept { return _mm_cmpnge_ss(a, b); }
inline __m128d cmpngt_pd(const __m128d& a, const __m128d& b) noexcept { return _mm_cmpngt_pd(a, b); }
inline __m128 cmpngt_ps(const __m128& a, const __m128& b) noexcept { return _mm_cmpngt_ps(a, b); }
inline __m128d cmpngt_sd(const __m128d& a, const __m128d& b) noexcept { return _mm_cmpngt_sd(a, b); }
inline __m128 cmpngt_ss(const __m128& a, const __m128& b) noexcept { return _mm_cmpngt_ss(a, b); }
inline __m128d cmpnle_pd(const __m128d& a, const __m128d& b) noexcept { return _mm_cmpnle_pd(a, b); }
inline __m128 cmpnle_ps(const __m128& a, const __m128& b) noexcept { return _mm_cmpnle_ps(a, b); }
inline __m128d cmpnle_sd(const __m128d& a, const __m128d& b) noexcept { return _mm_cmpnle_sd(a, b); }
inline __m128 cmpnle_ss(const __m128& a, const __m128& b) noexcept { return _mm_cmpnle_ss(a, b); }
inline __m128d cmpnlt_pd(const __m128d& a, const __m128d& b) noexcept { return _mm_cmpnlt_pd(a, b); }
inline __m128 cmpnlt_ps(const __m128& a, const __m128& b) noexcept { return _mm_cmpnlt_ps(a, b); }
inline __m128d cmpnlt_sd(const __m128d& a, const __m128d& b) noexcept { return _mm_cmpnlt_sd(a, b); }
inline __m128 cmpnlt_ss(const __m128& a, const __m128& b) noexcept { return _mm_cmpnlt_ss(a, b); }
inline __m128d cmpord_pd(const __m128d& a, const __m128d& b) noexcept { return _mm_cmpord_pd(a, b); }
inline __m128 cmpord_ps(const __m128& a, const __m128& b) noexcept { return _mm_cmpord_ps(a, b); }
inline __m128d cmpord_sd(const __m128d& a, const __m128d& b) noexcept { return _mm_cmpord_sd(a, b); }
inline __m128 cmpord_ss(const __m128& a, const __m128& b) noexcept { return _mm_cmpord_ss(a, b); }
inline __m128d cmpunord_pd(const __m128d& a, const __m128d& b) noexcept { return _mm_cmpunord_pd(a, b); }
inline __m128 cmpunord_ps(const __m128& a, const __m128& b) noexcept { return _mm_cmpunord_ps(a, b); }
inline __m128d cmpunord_sd(const __m128d& a, const __m128d& b) noexcept { return _mm_cmpunord_sd(a, b); }
inline __m128 cmpunord_ss(const __m128& a, const __m128& b) noexcept { return _mm_cmpunord_ss(a, b); }
inline int comieq_sd(const __m128d& a, const __m128d& b) noexcept { return _mm_comieq_sd(a, b); }
inline int comieq_ss(const __m128& a, const __m128& b) noexcept { return _mm_comieq_ss(a, b); }
inline int comige_sd(const __m128d& a, const __m128d& b) noexcept { return _mm_comige_sd(a, b); }
inline int comige_ss(const __m128& a, const __m128& b) noexcept { return _mm_comige_ss(a, b); }
inline int comigt_sd(const __m128d& a, const __m128d& b) noexcept { return _mm_comigt_sd(a, b); }
inline int comigt_ss(const __m128& a, const __m128& b) noexcept { return _mm_comigt_ss(a, b); }
inline int comile_sd(const __m128d& a, const __m128d& b) noexcept { return _mm_comile_sd(a, b); }
inline int comile_ss(const __m128& a, const __m128& b) noexcept { return _mm_comile_ss(a, b); }
inline int comilt_sd(const __m128d& a, const __m128d& b) noexcept { return _mm_comilt_sd(a, b); }
inline int comilt_ss(const __m128& a, const __m128& b) noexcept { return _mm_comilt_ss(a, b); }
inline int comineq_sd(const __m128d& a, const __m128d& b) noexcept { return _mm_comineq_sd(a, b); }
inline int comineq_ss(const __m128& a, const __m128& b) noexcept { return _mm_comineq_ss(a, b); }
inline int ucomieq_sd(const __m128d& a, const __m128d& b) noexcept { return _mm_ucomieq_sd(a, b); }
inline int ucomieq_ss(const __m128& a, const __m128& b) noexcept { return _mm_ucomieq_ss(a, b); }
inline int ucomige_sd(const __m128d& a, const __m128d& b) noexcept { return _mm_ucomige_sd(a, b); }
inline int ucomige_ss(const __m128& a, const __m128& b) noexcept { return _mm_ucomige_ss(a, b); }
inline int ucomigt_sd(const __m128d& a, const __m128d& b) noexcept { return _mm_ucomigt_sd(a, b); }
inline int ucomigt_ss(const __m128& a, const __m128& b) noexcept { return _mm_ucomigt_ss(a, b); }
inline int ucomile_sd(const __m128d& a, const __m128d& b) noexcept { return _mm_ucomile_sd(a, b); }
inline int ucomile_ss(const __m128& a, const __m128& b) noexcept { return _mm_ucomile_ss(a, b); }
inline int ucomilt_sd(const __m128d& a, const __m128d& b) noexcept { return _mm_ucomilt_sd(a, b); }
inline int ucomilt_ss(const __m128& a, const __m128& b) noexcept { return _mm_ucomilt_ss(a, b); }
inline int ucomineq_sd(const __m128d& a, const __m128d& b) noexcept { return _mm_ucomineq_sd(a, b); }
inline int ucomineq_ss(const __m128& a, const __m128& b) noexcept { return _mm_ucomineq_ss(a, b); }

// CONVERT /////////////////////////////////////////////////////////////////////


inline __m128 cvt_si2ss(const __m128& a, int b) noexcept { return _mm_cvt_si2ss(a, b); }
inline int cvt_ss2si(const __m128& a) noexcept { return _mm_cvt_ss2si(a); }
inline __m128i cvtepi16_epi32(const __m128i& a) noexcept { return _mm_cvtepi16_epi32(a); }
inline __m256i cvtepi16_epi32(const __m256i& a) noexcept { return _mm256_cvtepi16_epi32(a); }
inline __m128i cvtepi16_epi64(const __m128i& a) noexcept { return _mm_cvtepi16_epi64(a); }
inline __m256i cvtepi16_epi64(const __m256i& a) noexcept { return _mm256_cvtepi16_epi64(a); }
inline __m128i cvtepi32_epi64(const __m128i& a) noexcept { return _mm_cvtepi32_epi64(a); }
inline __m256i cvtepi32_epi64(const __m256i& a) noexcept { return _mm256_cvtepi32_epi64(a); }
inline __m128d cvtepi32_pd(const __m128i& a) noexcept { return _mm_cvtepi32_pd(a); }
inline __m256d cvtepi32_pd(const __m128i& a) noexcept { return _mm256_cvtepi32_pd(a); }
inline __m128 cvtepi32_ps(const __m128i& a) noexcept { return _mm_cvtepi32_ps(a); }
inline __m256 cvtepi32_ps(const __m256i& a) noexcept { return _mm256_cvtepi32_ps(a); }
inline __m128i cvtepi8_epi16(const __m128i& a) noexcept { return _mm_cvtepi8_epi16(a); }
inline __m256i cvtepi8_epi16(const __m256i& a) noexcept { return _mm256_cvtepi8_epi16(a); }
inline __m128i cvtepi8_epi32(const __m128i& a) noexcept { return _mm_cvtepi8_epi32(a); }
inline __m256i cvtepi8_epi32(const __m256i& a) noexcept { return _mm256_cvtepi8_epi32(a); }
inline __m128i cvtepi8_epi64(const __m128i& a) noexcept { return _mm_cvtepi8_epi64(a); }
inline __m256i cvtepi8_epi64(const __m256i& a) noexcept { return _mm256_cvtepi8_epi64(a); }
inline __m128i cvtepu16_epi32(const __m128i& a) noexcept { return _mm_cvtepu16_epi32(a); }
inline __m256i cvtepu16_epi32(const __m256i& a) noexcept { return _mm256_cvtepu16_epi32(a); }
inline __m128i cvtepu16_epi64(const __m128i& a) noexcept { return _mm_cvtepu16_epi64(a); }
inline __m256i cvtepu16_epi64(const __m256i& a) noexcept { return _mm256_cvtepu16_epi64(a); }
inline __m128i cvtepu32_epi64(const __m128i& a) noexcept { return _mm_cvtepu32_epi64(a); }
inline __m256i cvtepu32_epi64(const __m256i& a) noexcept { return _mm256_cvtepu32_epi64(a); }
inline __m128i cvtepu8_epi16(const __m128i& a) noexcept { return _mm_cvtepu8_epi16(a); }
inline __m256i cvtepu8_epi16(const __m256i& a) noexcept { return _mm256_cvtepu8_epi16(a); }
inline __m128i cvtepu8_epi32(const __m128i& a) noexcept { return _mm_cvtepu8_epi32(a); }
inline __m256i cvtepu8_epi32(const __m256i& a) noexcept { return _mm256_cvtepu8_epi32(a); }
inline __m128i cvtepu8_epi64(const __m128i& a) noexcept { return _mm_cvtepu8_epi64(a); }
inline __m256i cvtepu8_epi64(const __m256i& a) noexcept { return _mm256_cvtepu8_epi64(a); }
inline __m128i cvtpd_epi32(const __m128d& a) noexcept { return _mm_cvtpd_epi32(a); }
inline __m128i cvtpd_epi32(const __m256d& a) noexcept { return _mm256_cvtpd_epi32(a); }
inline __m128 cvtpd_ps(const __m128d& a) noexcept { return _mm_cvtpd_ps(a); }
inline __m128 cvtpd_ps(const __m256d& a) noexcept { return _mm256_cvtpd_ps(a); }
inline __m128 cvtph_ps(const __m128i& a) noexcept { return _mm_cvtph_ps(a); }
inline __m256 cvtph_ps(const __m128i& a) noexcept { return _mm256_cvtph_ps(a); }
inline __m128i cvtps_epi32(const __m128& a) noexcept { return _mm_cvtps_epi32(a); }
inline __m256i cvtps_epi32(const __m256& a) noexcept { return _mm256_cvtps_epi32(a); }
inline __m128d cvtps_pd(const __m128& a) noexcept { return _mm_cvtps_pd(a); }
inline __m256d cvtps_pd(const __m256& a) noexcept { return _mm256_cvtps_pd(a); }
inline __m128i cvtps_ph(const __m128& a, int b) noexcept { return _mm_cvtps_ph(a, b); }
inline __m128i cvtps_ph(const __m256& a, int b) noexcept { return _mm256_cvtps_ph(a, b); }
inline double cvtsd_f64(const __m128d& a) noexcept { return _mm_cvtsd_f64(a); }
inline double cvtsd_f64(const __m256d& a) noexcept { return _mm256_cvtsd_f64(a); }
inline int cvtsd_si32(const __m128d& a) noexcept { return _mm_cvtsd_si32(a); }
inline long long cvtsd_si64(const __m128d& a) noexcept { return _mm_cvtsd_si64(a); }
inline long long cvtsd_si64x(const __m128d& a) noexcept { return _mm_cvtsd_si64x(a); }
inline __m128 cvtsd_ss(const __m128& a, const __m128d& b) noexcept { return _mm_cvtsd_ss(a, b); }
inline int cvtsi128_si32(const __m128i& a) noexcept { return _mm_cvtsi128_si32(a); }
inline long long cvtsi128_si64(const __m128i& a) noexcept { return _mm_cvtsi128_si64(a); }
inline long long cvtsi128_si64x(const __m128i& a) noexcept { return _mm_cvtsi128_si64x(a); }
inline int cvtsi256_si32(const __m256i& a) noexcept { return _mm256_cvtsi256_si32(a); }
inline __m128d cvtsi32_sd(const __m128d& a, int b) noexcept { return _mm_cvtsi32_sd(a, b); }
inline __m128i cvtsi32_si128(int a) noexcept { return _mm_cvtsi32_si128(a); }
inline __m128 cvtsi32_ss(const __m128& a, int b) noexcept { return _mm_cvtsi32_ss(a, b); }
inline __m128d cvtsi64_sd(const __m128d& a, long long b) noexcept { return _mm_cvtsi64_sd(a, b); }
inline __m128i cvtsi64_si128(long long a) noexcept { return _mm_cvtsi64_si128(a); }
inline __m128 cvtsi64_ss(const __m128& a, long long b) noexcept { return _mm_cvtsi64_ss(a, b); }
inline __m128d cvtsi64x_sd(const __m128d& a, long long b) noexcept { return _mm_cvtsi64x_sd(a, b); }
inline __m128i cvtsi64x_si128(long long a) noexcept { return _mm_cvtsi64x_si128(a); }
inline float cvtss_f32(const __m128& a) noexcept { return _mm_cvtss_f32(a); }
inline float cvtss_f32(const __m256& a) noexcept { return _mm256_cvtss_f32(a); }
inline __m128d cvtss_sd(const __m128d& a, const __m128& b) noexcept { return _mm_cvtss_sd(a, b); }
inline int cvtss_si32(const __m128& a) noexcept { return _mm_cvtss_si32(a); }
inline long long cvtss_si64(const __m128& a) noexcept { return _mm_cvtss_si64(a); }
inline int cvtt_ss2si(const __m128& a) noexcept { return _mm_cvtt_ss2si(a); }
inline __m128i cvttpd_epi32(const __m128d& a) noexcept { return _mm_cvttpd_epi32(a); }
inline __m128i cvttpd_epi32(const __m256d& a) noexcept { return _mm256_cvttpd_epi32(a); }
inline __m128i cvttps_epi32(const __m128& a) noexcept { return _mm_cvttps_epi32(a); }
inline __m256i cvttps_epi32(const __m256& a) noexcept { return _mm256_cvttps_epi32(a); }
inline int cvttsd_si32(const __m128d& a) noexcept { return _mm_cvttsd_si32(a); }
inline long long cvttsd_si64(const __m128d& a) noexcept { return _mm_cvttsd_si64(a); }
inline long long cvttsd_si64x(const __m128d& a) noexcept { return _mm_cvttsd_si64x(a); }
inline int cvttss_si32(const __m128& a) noexcept { return _mm_cvttss_si32(a); }
inline long long cvttss_si64(const __m128& a) noexcept { return _mm_cvttss_si64(a); }
inline __m128i packus_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_packus_epi32(a, b); }

// CRYPTOGRAPHY ////////////////////////////////////////////////////////////////

inline unsigned int crc32_u16(unsigned int a, unsigned short b) noexcept { return _mm_crc32_u16(a, b); }
inline unsigned int crc32_u32(unsigned int a, unsigned int b) noexcept { return _mm_crc32_u32(a, b); }
inline unsigned long long crc32_u64(unsigned long long a, unsigned long long b) noexcept { return _mm_crc32_u64(a, b); }
inline unsigned int crc32_u8(unsigned int a, unsigned char b) noexcept { return _mm_crc32_u8(a, b); }
inline __m256i sha512msg1_epi64(const __m256i& a, const __m256i& b) noexcept { return _mm_sha512msg1_epi64(a, b); }
inline __m256i sha512msg2_epi64(const __m256i& a, const __m256i& b) noexcept { return _mm_sha512msg2_epi64(a, b); }
inline __m256i sha512rnds2_epi64(const __m256i& a, const __m256i& b) noexcept { return _mm_sha512rnds2_epi64(a, b); }
inline __m128i sm3msg1_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_sm3msg1_epi32(a, b); }
inline __m128i sm3msg2_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_sm3msg2_epi32(a, b); }
inline __m128i sm3rnds2_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_sm3rnds2_epi32(a, b); }
inline __m128i sm4key4_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_sm4key4_epi32(a, b); }
inline __m256i sm4key4_epi32(const __m256i& a, const __m256i& b) noexcept { return _mm256_sm4key4_epi32(a, b); }
inline __m128i sm4rnds4_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_sm4rnds4_epi32(a, b); }
inline __m256i sm4rnds4_epi32(const __m256i& a, const __m256i& b) noexcept { return _mm256_sm4rnds4_epi32(a, b); }

// ELEMENTARY MATH FUNCTIONS ///////////////////////////////////////////////////


inline __m128d cbrt_pd(const __m128d& a) noexcept { return _mm_cbrt_pd(a); }
inline __m256d cbrt_pd(const __m256d& a) noexcept { return _mm256_cbrt_pd(a); }
inline __m128 cbrt_ps(const __m128& a) noexcept { return _mm_cbrt_ps(a); }
inline __m256 cbrt_ps(const __m256& a) noexcept { return _mm256_cbrt_ps(a); }
inline __m128 cexp_ps(const __m128& a) noexcept { return _mm_cexp_ps(a); }
inline __m256 cexp_ps(const __m256& a) noexcept { return _mm256_cexp_ps(a); }
inline __m128 clog_ps(const __m128& a) noexcept { return _mm_clog_ps(a); }
inline __m256 clog_ps(const __m256& a) noexcept { return _mm256_clog_ps(a); }
inline __m128 csqrt_ps(const __m128& a) noexcept { return _mm_csqrt_ps(a); }
inline __m256 csqrt_ps(const __m256& a) noexcept { return _mm256_csqrt_ps(a); }
inline __m128d exp_pd(const __m128d& a) noexcept { return _mm_exp_pd(a); }
inline __m256d exp_pd(const __m256d& a) noexcept { return _mm256_exp_pd(a); }
inline __m128 exp_ps(const __m128& a) noexcept { return _mm_exp_ps(a); }
inline __m256 exp_ps(const __m256& a) noexcept { return _mm256_exp_ps(a); }
inline __m128d exp10_pd(const __m128d& a) noexcept { return _mm_exp10_pd(a); }
inline __m256d exp10_pd(const __m256d& a) noexcept { return _mm256_exp10_pd(a); }
inline __m128 exp10_ps(const __m128& a) noexcept { return _mm_exp10_ps(a); }
inline __m256 exp10_ps(const __m256& a) noexcept { return _mm256_exp10_ps(a); }
inline __m128d exp2_pd(const __m128d& a) noexcept { return _mm_exp2_pd(a); }
inline __m256d exp2_pd(const __m256d& a) noexcept { return _mm256_exp2_pd(a); }
inline __m128 exp2_ps(const __m128& a) noexcept { return _mm_exp2_ps(a); }
inline __m256 exp2_ps(const __m256& a) noexcept { return _mm256_exp2_ps(a); }
inline __m128d expm1_pd(const __m128d& a) noexcept { return _mm_expm1_pd(a); }
inline __m256d expm1_pd(const __m256d& a) noexcept { return _mm256_expm1_pd(a); }
inline __m128 expm1_ps(const __m128& a) noexcept { return _mm_expm1_ps(a); }
inline __m256 expm1_ps(const __m256& a) noexcept { return _mm256_expm1_ps(a); }
inline __m128d invcbrt_pd(const __m128d& a) noexcept { return _mm_invcbrt_pd(a); }
inline __m256d invcbrt_pd(const __m256d& a) noexcept { return _mm256_invcbrt_pd(a); }
inline __m128 invcbrt_ps(const __m128& a) noexcept { return _mm_invcbrt_ps(a); }
inline __m256 invcbrt_ps(const __m256& a) noexcept { return _mm256_invcbrt_ps(a); }
inline __m128d invsqrt_pd(const __m128d& a) noexcept { return _mm_invsqrt_pd(a); }
inline __m256d invsqrt_pd(const __m256d& a) noexcept { return _mm256_invsqrt_pd(a); }
inline __m128 invsqrt_ps(const __m128& a) noexcept { return _mm_invsqrt_ps(a); }
inline __m256 invsqrt_ps(const __m256& a) noexcept { return _mm256_invsqrt_ps(a); }
inline __m128d log_pd(const __m128d& a) noexcept { return _mm_log_pd(a); }
inline __m256d log_pd(const __m256d& a) noexcept { return _mm256_log_pd(a); }
inline __m128 log_ps(const __m128& a) noexcept { return _mm_log_ps(a); }
inline __m256 log_ps(const __m256& a) noexcept { return _mm256_log_ps(a); }
inline __m128d log10_pd(const __m128d& a) noexcept { return _mm_log10_pd(a); }
inline __m256d log10_pd(const __m256d& a) noexcept { return _mm256_log10_pd(a); }
inline __m128 log10_ps(const __m128& a) noexcept { return _mm_log10_ps(a); }
inline __m256 log10_ps(const __m256& a) noexcept { return _mm256_log10_ps(a); }
inline __m128d log1p_pd(const __m128d& a) noexcept { return _mm_log1p_pd(a); }
inline __m256d log1p_pd(const __m256d& a) noexcept { return _mm256_log1p_pd(a); }
inline __m128 log1p_ps(const __m128& a) noexcept { return _mm_log1p_ps(a); }
inline __m256 log1p_ps(const __m256& a) noexcept { return _mm256_log1p_ps(a); }
inline __m128d log2_pd(const __m128d& a) noexcept { return _mm_log2_pd(a); }
inline __m256d log2_pd(const __m256d& a) noexcept { return _mm256_log2_pd(a); }
inline __m128 log2_ps(const __m128& a) noexcept { return _mm_log2_ps(a); }
inline __m256 log2_ps(const __m256& a) noexcept { return _mm256_log2_ps(a); }
inline __m128d logb_pd(const __m128d& a) noexcept { return _mm_logb_pd(a); }
inline __m256d logb_pd(const __m256d& a) noexcept { return _mm256_logb_pd(a); }
inline __m128 logb_ps(const __m128& a) noexcept { return _mm_logb_ps(a); }
inline __m256 logb_ps(const __m256& a) noexcept { return _mm256_logb_ps(a); }
inline __m128d pow_pd(const __m128d& a, const __m128d& b) noexcept { return _mm_pow_pd(a, b); }
inline __m256d pow_pd(const __m256d& a, const __m256d& b) noexcept { return _mm256_pow_pd(a, b); }
inline __m128 pow_ps(const __m128& a, const __m128& b) noexcept { return _mm_pow_ps(a, b); }
inline __m256 pow_ps(const __m256& a, const __m256& b) noexcept { return _mm256_pow_ps(a, b); }
inline __m128 rcp_ps(const __m128& a) noexcept { return _mm_rcp_ps(a); }
inline __m256 rcp_ps(const __m256& a) noexcept { return _mm256_rcp_ps(a); }
inline __m128 rcp_ss(const __m128& a) noexcept { return _mm_rcp_ss(a); }
inline __m128 rsqrt_ps(const __m128& a) noexcept { return _mm_rsqrt_ps(a); }
inline __m256 rsqrt_ps(const __m256& a) noexcept { return _mm256_rsqrt_ps(a); }
inline __m128 rsqrt_ss(const __m128& a) noexcept { return _mm_rsqrt_ss(a); }
inline __m128d sqrt_pd(const __m128d& a) noexcept { return _mm_sqrt_pd(a); }
inline __m256d sqrt_pd(const __m256d& a) noexcept { return _mm256_sqrt_pd(a); }
inline __m128 sqrt_ps(const __m128& a) noexcept { return _mm_sqrt_ps(a); }
inline __m256 sqrt_ps(const __m256& a) noexcept { return _mm256_sqrt_ps(a); }
inline __m128d sqrt_sd(const __m128d& a, const __m128d& b) noexcept { return _mm_sqrt_sd(a, b); }
inline __m128 sqrt_ss(const __m128& a, const __m128& b) noexcept { return _mm_sqrt_ss(a, b); }
inline __m128d svml_sqrt_pd(const __m128d& a) noexcept { return _mm_svml_sqrt_pd(a); }
inline __m256d svml_sqrt_pd(const __m256d& a) noexcept { return _mm256_svml_sqrt_pd(a); }
inline __m128 svml_sqrt_ps(const __m128& a) noexcept { return _mm_svml_sqrt_ps(a); }
inline __m256 svml_sqrt_ps(const __m256& a) noexcept { return _mm256_svml_sqrt_ps(a); }

// GENERAL SUPPORT /////////////////////////////////////////////////////////////

// LOAD ////////////////////////////////////////////////////////////////////////

inline __m256d broadcast_pd(const __m128d& a) noexcept { return _mm_broadcast_pd(a); }
inline __m256 broadcast_ps(const __m128& a) noexcept { return _mm_broadcast_ps(a); }
inline __m256d broadcast_sd(const double* a) noexcept { return _mm_broadcast_sd(a); }
template<typename T> void broadcast_ss(auto&&...) = delete;
template<> m128 broadcast_ss<m128>(const float* a) noexcept { return _mm_broadcast_ss(a); }
template<> m256 broadcast_ss<m256>(const float* a) noexcept { return _mm256_broadcast_ss(a); }
template<int scale> __m128i i32gather_epi32(const int* a, const __m128i& index) noexcept { return _mm_i32gather_epi32(a, index, scale); }
template<int scale> __m256i i32gather_epi32(const int* a, const __m256i& index) noexcept { return _mm256_i32gather_epi32(a, index, scale); }
template<int scale> __m128i i32gather_epi64(const long long* a, const __m128i& index) noexcept { return _mm_i32gather_epi64(a, index, scale); }
template<int scale> __m256i i32gather_epi64(const long long* a, const __m256i& index) noexcept { return _mm256_i32gather_epi64(a, index, scale); }
template<int scale> __m128d i32gather_pd(const double* a, const __m128i& index) noexcept { return _mm_i32gather_pd(a, index, scale); }
template<int scale> __m256d i32gather_pd(const double* a, const __m256i& index) noexcept { return _mm256_i32gather_pd(a, index, scale); }
template<int scale> __m128 i32gather_ps(const float* a, const __m128i& index) noexcept { return _mm_i32gather_ps(a, index, scale); }
template<int scale> __m256 i32gather_ps(const float* a, const __m256i& index) noexcept { return _mm256_i32gather_ps(a, index, scale); }
template<int scale> __m128i i64gather_epi32(const int* a, const __m128i& index) noexcept { return _mm_i64gather_epi32(a, index, scale); }
template<int scale> __m128i i64gather_epi32(const int* a, const __m256i& index) noexcept { return _mm256_i64gather_epi32(a, index, scale); }
template<int scale> __m128i i64gather_epi64(const long long* a, const __m128i& index) noexcept { return _mm_i64gather_epi64(a, index, scale); }
template<int scale> __m256i i64gather_epi64(const long long* a, const __m256i& index) noexcept { return _mm256_i64gather_epi64(a, index, scale); }
template<int scale> __m128d i64gather_pd(const double* a, const __m128i& index) noexcept { return _mm_i64gather_pd(a, index, scale); }
template<int scale> __m256d i64gather_pd(const double* a, const __m256i& index) noexcept { return _mm256_i64gather_pd(a, index, scale); }
template<int scale> __m128 i64gather_ps(const float* a, const __m128i& index) noexcept { return _mm_i64gather_ps(a, index, scale); }
template<int scale> __m128 i64gather_ps(const float* a, const __m256i& index) noexcept { return _mm256_i64gather_ps(a, index, scale); }
inline __m128i lddqu_si128(const __m128i* a) noexcept { return _mm_lddqu_si128(a); }
inline __m256i lddqu_si256(const __m256i* a) noexcept { return _mm256_lddqu_si256(a); }
template<typename T> void load_pd(auto&&...) = delete;
template<> m128d load_pd<m128d>(const double* a) noexcept { return _mm_load_pd(a); }
template<> m256d load_pd<m256d>(const double* a) noexcept { return _mm256_load_pd(a); }
inline __m128d load_pd1(const double* a) noexcept { return _mm_load_pd1(a); }
template<typename T> void load_ps(auto&&...) = delete;
template<> m128 load_ps<m128>(const float* a) noexcept { return _mm_load_ps(a); }
template<> m256 load_ps<m256>(const float* a) noexcept { return _mm256_load_ps(a); }
inline __m128d load_sd(const double* a) noexcept { return _mm_load_sd(a); }
inline __m128i load_si128(const __m128i* a) noexcept { return _mm_load_si128(a); }
inline __m256i load_si256(const __m256i* a) noexcept { return _mm256_load_si256(a); }
inline __m128 load_ss(const float* a) noexcept { return _mm_load_ss(a); }
inline __m128d load1_pd(const double* a) noexcept { return _mm_load1_pd(a); }
inline __m128 load1_ps(const float* a) noexcept { return _mm_load1_ps(a); }
inline __m128d loaddup_pd(const double* a) noexcept { return _mm_loaddup_pd(a); }
inline __m128i loadl_epi64(const long long* a) noexcept { return _mm_loadl_epi64(a); }
inline __m128d loadl_pd(const double* a) noexcept { return _mm_loadl_pd(a); }
inline __m128d loadr_pd(const double* a) noexcept { return _mm_loadr_pd(a); }
inline __m128 loadr_ps(const float* a) noexcept { return _mm_loadr_ps(a); }
template<typename T> void loadu_pd(auto&&...) = delete;
template<> m128d loadu_pd<__m128d>(const double* a) noexcept { return _mm_loadu_pd(a); }
template<> m256d loadu_pd<__m256d>(const double* a) noexcept { return _mm256_loadu_pd(a); }
template<typename T> void loadu_ps(auto&&...) = delete;
template<> m128 loadu_ps<__m128>(const float* a) noexcept { return _mm_loadu_ps(a); }
template<> m256 loadu_ps<__m256>(const float* a) noexcept { return _mm256_loadu_ps(a); }
inline __m128i loadu_si128(const __m128i* a) noexcept { return _mm_loadu_si128(a); }
inline __m128i loadu_si16(const short* a) noexcept { return _mm_loadu_si16(a); }
inline __m256i loadu_si256(const __m256i* a) noexcept { return _mm256_loadu_si256(a); }
inline __m128i loadu_si32(const int* a) noexcept { return _mm_loadu_si32(a); }
inline __m128i loadu_si64(const long long* a) noexcept { return _mm_loadu_si64(a); }
inline __m256 loadu2_m128(const float* a, const float* b) noexcept { return _mm_loadu2_m128(a, b); }
inline __m256d loadu2_m128d(const double* a, const double* b) noexcept { return _mm_loadu2_m128d(a, b); }
inline __m256i loadu2_m128i(const __m128i* a, const __m128i* b) noexcept { return _mm_loadu2_m128i(a, b); }
inline __m128i stream_load_si128(const __m128i* a) noexcept { return _mm_stream_load_si128(a); }
inline __m256i stream_load_si256(const __m256i* a) noexcept { return _mm256_stream_load_si256(a); }

// LOGICAL //////////////////////////////////////////////////////////////////////

inline __m128d and_pd(const __m128d& a, const __m128d& b) noexcept { return _mm_and_pd(a, b); }
inline __m256d and_pd(const __m256d& a, const __m256d& b) noexcept { return _mm256_and_pd(a, b); }
inline __m128 and_ps(const __m128& a, const __m128& b) noexcept { return _mm_and_ps(a, b); }
inline __m256 and_ps(const __m256& a, const __m256& b) noexcept { return _mm256_and_ps(a, b); }
inline __m128i and_si128(const __m128i& a, const __m128i& b) noexcept { return _mm_and_si128(a, b); }
inline __m256i and_si256(const __m256i& a, const __m256i& b) noexcept { return _mm256_and_si256(a, b); }
inline __m128d andnot_pd(const __m128d& a, const __m128d& b) noexcept { return _mm_andnot_pd(a, b); }
inline __m256d andnot_pd(const __m256d& a, const __m256d& b) noexcept { return _mm256_andnot_pd(a, b); }
inline __m128 andnot_ps(const __m128& a, const __m128& b) noexcept { return _mm_andnot_ps(a, b); }
inline __m256 andnot_ps(const __m256& a, const __m256& b) noexcept { return _mm256_andnot_ps(a, b); }
inline __m128i andnot_si128(const __m128i& a, const __m128i& b) noexcept { return _mm_andnot_si128(a, b); }
inline __m256i andnot_si256(const __m256i& a, const __m256i& b) noexcept { return _mm256_andnot_si256(a, b); }
inline __m128d or_pd(const __m128d& a, const __m128d& b) noexcept { return _mm_or_pd(a, b); }
inline __m256d or_pd(const __m256d& a, const __m256d& b) noexcept { return _mm256_or_pd(a, b); }
inline __m128 or_ps(const __m128& a, const __m128& b) noexcept { return _mm_or_ps(a, b); }
inline __m256 or_ps(const __m256& a, const __m256& b) noexcept { return _mm256_or_ps(a, b); }
inline __m128i or_si128(const __m128i& a, const __m128i& b) noexcept { return _mm_or_si128(a, b); }
inline __m256i or_si256(const __m256i& a, const __m256i& b) noexcept { return _mm256_or_si256(a, b); }
inline int test_all_ones(const __m128i& a) noexcept { return _mm_test_all_ones(a); }
inline int test_all_zeros(const __m128i& a) noexcept { return _mm_test_all_zeros(a); }
inline int test_mix_ones_zeros(const __m128i& a, const __m128i& b) noexcept { return _mm_test_mix_ones_zeros(a, b); }
inline int testc_pd(const __m128d& a, const __m128d& b) noexcept { return _mm_testc_pd(a, b); }
inline int testc_pd(const __m256d& a, const __m256d& b) noexcept { return _mm256_testc_pd(a, b); }
inline int testc_ps(const __m128& a, const __m128& b) noexcept { return _mm_testc_ps(a, b); }
inline int testc_ps(const __m256& a, const __m256& b) noexcept { return _mm256_testc_ps(a, b); }
inline int testc_si128(const __m128i& a, const __m128i& b) noexcept { return _mm_testc_si128(a, b); }
inline int testc_si256(const __m256i& a, const __m256i& b) noexcept { return _mm256_testc_si256(a, b); }
inline int testnzc_pd(const __m128d& a, const __m128d& b) noexcept { return _mm_testnzc_pd(a, b); }
inline int testnzc_pd(const __m256d& a, const __m256d& b) noexcept { return _mm256_testnzc_pd(a, b); }
inline int testnzc_ps(const __m128& a, const __m128& b) noexcept { return _mm_testnzc_ps(a, b); }
inline int testnzc_ps(const __m256& a, const __m256& b) noexcept { return _mm256_testnzc_ps(a, b); }
inline int testnzc_si128(const __m128i& a, const __m128i& b) noexcept { return _mm_testnzc_si128(a, b); }
inline int testnzc_si256(const __m256i& a, const __m256i& b) noexcept { return _mm256_testnzc_si256(a, b); }
inline int testz_pd(const __m128d& a, const __m128d& b) noexcept { return _mm_testz_pd(a, b); }
inline int testz_pd(const __m256d& a, const __m256d& b) noexcept { return _mm256_testz_pd(a, b); }
inline int testz_ps(const __m128& a, const __m128& b) noexcept { return _mm_testz_ps(a, b); }
inline int testz_ps(const __m256& a, const __m256& b) noexcept { return _mm256_testz_ps(a, b); }
inline int testz_si128(const __m128i& a, const __m128i& b) noexcept { return _mm_testz_si128(a, b); }
inline int testz_si256(const __m256i& a, const __m256i& b) noexcept { return _mm256_testz_si256(a, b); }
inline __m128d xor_pd(const __m128d& a, const __m128d& b) noexcept { return _mm_xor_pd(a, b); }
inline __m256d xor_pd(const __m256d& a, const __m256d& b) noexcept { return _mm256_xor_pd(a, b); }
inline __m128 xor_ps(const __m128& a, const __m128& b) noexcept { return _mm_xor_ps(a, b); }
inline __m256 xor_ps(const __m256& a, const __m256& b) noexcept { return _mm256_xor_ps(a, b); }
inline __m128i xor_si128(const __m128i& a, const __m128i& b) noexcept { return _mm_xor_si128(a, b); }
inline __m256i xor_si256(const __m256i& a, const __m256i& b) noexcept { return _mm256_xor_si256(a, b); }

// MASK ////////////////////////////////////////////////////////////////////////

// MISCELLANEOUS ///////////////////////////////////////////////////////////////

template<int imm8> __m128i alignr_epi8(const __m128i& a, const __m128i& b) noexcept { return _mm_alignr_epi8(a, b, imm8); }
template<int imm8> __m256i alignr_epi8(const __m256i& a, const __m256i& b) noexcept { return _mm256_alignr_epi8(a, b, imm8); }
inline __m128i minpos_epu16(const __m128i& a) noexcept { return _mm_minpos_epu16(a); }
inline int movemask_epi8(const __m128i& a) noexcept { return _mm_movemask_epi8(a); }
inline int movemask_epi8(const __m256i& a) noexcept { return _mm256_movemask_epi8(a); }
inline int movemask_pd(const __m128d& a) noexcept { return _mm_movemask_pd(a); }
inline int movemask_pd(const __m256d& a) noexcept { return _mm256_movemask_pd(a); }
inline int movemask_ps(const __m128& a) noexcept { return _mm_movemask_ps(a); }
inline int movemask_ps(const __m256& a) noexcept { return _mm256_movemask_ps(a); }
template<int imm8> __m128i mpsadbw_epu8(const __m128i& a, const __m128i& b) noexcept { return _mm_mpsadbw_epu8(a, b, imm8); }
template<int imm8> __m256i mpsadbw_epu8(const __m256i& a, const __m256i& b) noexcept { return _mm256_mpsadbw_epu8(a, b, imm8); }
inline __m128i packs_epi16(const __m128i& a, const __m128i& b) noexcept { return _mm_packs_epi16(a, b); }
inline __m256i packs_epi16(const __m256i& a, const __m256i& b) noexcept { return _mm256_packs_epi16(a, b); }
inline __m128i packs_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_packs_epi32(a, b); }
inline __m256i packs_epi32(const __m256i& a, const __m256i& b) noexcept { return _mm256_packs_epi32(a, b); }
inline __m128i packus_epi16(const __m128i& a, const __m128i& b) noexcept { return _mm_packus_epi16(a, b); }
inline __m256i packus_epi16(const __m256i& a, const __m256i& b) noexcept { return _mm256_packus_epi16(a, b); }
inline __m128i packus_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_packus_epi32(a, b); }
inline __m256i packus_epi32(const __m256i& a, const __m256i& b) noexcept { return _mm256_packus_epi32(a, b); }
inline __m128i sad_epu8(const __m128i& a, const __m128i& b) noexcept { return _mm_sad_epu8(a, b); }
inline __m128d trunc_pd(const __m128d& a) noexcept { return _mm_trunc_pd(a); }
inline __m256d trunc_pd(const __m256d& a) noexcept { return _mm256_trunc_pd(a); }
inline __m128 trunc_ps(const __m128& a) noexcept { return _mm_trunc_ps(a); }
inline __m256 trunc_ps(const __m256& a) noexcept { return _mm256_trunc_ps(a); }

// MOVE ////////////////////////////////////////////////////////////////////////

inline __m128i move_epi64(const __m128i& a) noexcept { return _mm_move_epi64(a); }
inline __m128d move_sd(const __m128d& a, const __m128d& b) noexcept { return _mm_move_sd(a, b); }
inline __m128 move_ss(const __m128& a, const __m128& b) noexcept { return _mm_move_ss(a, b); }
inline __m128d movedup_pd(const __m128d& a) noexcept { return _mm_movedup_pd(a); }
inline __m256d movedup_pd(const __m256d& a) noexcept { return _mm256_movedup_pd(a); }
inline __m128 movehdup_ps(const __m128& a) noexcept { return _mm_movehdup_ps(a); }
inline __m256 movehdup_ps(const __m256& a) noexcept { return _mm256_movehdup_ps(a); }
inline __m128 movehl_ps(const __m128& a, const __m128& b) noexcept { return _mm_movehl_ps(a, b); }
inline __m128 moveldup_ps(const __m128& a) noexcept { return _mm_moveldup_ps(a); }
inline __m256 moveldup_ps(const __m256& a) noexcept { return _mm256_moveldup_ps(a); }
inline __m128 movelh_ps(const __m128& a, const __m128& b) noexcept { return _mm_movelh_ps(a, b); }
inline __m128i movpi64_epi64(const __m128i& a) noexcept { return _mm_movpi64_epi64(a); }

// OS-TARGETED /////////////////////////////////////////////////////////////////

// PROBABILITY / STATISTICS ////////////////////////////////////////////////////

inline __m128i avg_epu16(const __m128i& a, const __m128i& b) noexcept { return _mm_avg_epu16(a, b); }
inline __m256i avg_epu16(const __m256i& a, const __m256i& b) noexcept { return _mm256_avg_epu16(a, b); }
inline __m128i avg_epu8(const __m128i& a, const __m128i& b) noexcept { return _mm_avg_epu8(a, b); }
inline __m256i avg_epu8(const __m256i& a, const __m256i& b) noexcept { return _mm256_avg_epu8(a, b); }
inline __m128d cdfnorm_pd(const __m128d& a) noexcept { return _mm_cdfnorm_pd(a); }
inline __m256d cdfnorm_pd(const __m256d& a) noexcept { return _mm256_cdfnorm_pd(a); }
inline __m128 cdfnorm_ps(const __m128& a) noexcept { return _mm_cdfnorm_ps(a); }
inline __m256 cdfnorm_ps(const __m256& a) noexcept { return _mm256_cdfnorm_ps(a); }
inline __m128d cdfnorminv_pd(const __m128d& a) noexcept { return _mm_cdfnorminv_pd(a); }
inline __m256d cdfnorminv_pd(const __m256d& a) noexcept { return _mm256_cdfnorminv_pd(a); }
inline __m128 cdfnorminv_ps(const __m128& a) noexcept { return _mm_cdfnorminv_ps(a); }
inline __m256 cdfnorminv_ps(const __m256& a) noexcept { return _mm256_cdfnorminv_ps(a); }
inline __m256d erf_pd(const __m256d& a) noexcept { return _mm_erf_pd(a); }
inline __m128 erf_ps(const __m128& a) noexcept { return _mm_erf_ps(a); }
inline __m256 erf_ps(const __m256& a) noexcept { return _mm256_erf_ps(a); }
inline __m128d erfc_pd(const __m128d& a) noexcept { return _mm_erfc_pd(a); }
inline __m256d erfc_pd(const __m256d& a) noexcept { return _mm256_erfc_pd(a); }
inline __m128 erfc_ps(const __m128& a) noexcept { return _mm_erfc_ps(a); }
inline __m256 erfc_ps(const __m256& a) noexcept { return _mm256_erfc_ps(a); }
inline __m128d erfcinv_pd(const __m128d& a) noexcept { return _mm_erfcinv_pd(a); }
inline __m256d erfcinv_pd(const __m256d& a) noexcept { return _mm256_erfcinv_pd(a); }
inline __m128 erfcinv_ps(const __m128& a) noexcept { return _mm_erfcinv_ps(a); }
inline __m256 erfcinv_ps(const __m256& a) noexcept { return _mm256_erfcinv_ps(a); }
inline __m128d erfinv_pd(const __m128d& a) noexcept { return _mm_erfinv_pd(a); }
inline __m256d erfinv_pd(const __m256d& a) noexcept { return _mm256_erfinv_pd(a); }
inline __m128 erfinv_ps(const __m128& a) noexcept { return _mm_erfinv_ps(a); }
inline __m256 erfinv_ps(const __m256& a) noexcept { return _mm256_erfinv_ps(a); }

// RANDOM //////////////////////////////////////////////////////////////////////

// SET /////////////////////////////////////////////////////////////////////////

// SET /////////////////////////////////////////////////////////////////////////

inline __m128i set_epi16(short e7, short e6, short e5, short e4, short e3, short e2, short e1, short e0) noexcept { return _mm_set_epi16(e7, e6, e5, e4, e3, e2, e1, e0); }
inline __m256i set_epi16(short e15, short e14, short e13, short e12, short e11, short e10, short e9, short e8, short e7, short e6, short e5, short e4, short e3, short e2, short e1, short e0) noexcept { return _mm256_set_epi16(e15, e14, e13, e12, e11, e10, e9, e8, e7, e6, e5, e4, e3, e2, e1, e0); }
inline __m128i set_epi32(int e3, int e2, int e1, int e0) noexcept { return _mm_set_epi32(e3, e2, e1, e0); }
inline __m256i set_epi32(int e7, int e6, int e5, int e4, int e3, int e2, int e1, int e0) noexcept { return _mm256_set_epi32(e7, e6, e5, e4, e3, e2, e1, e0); }
inline __m128i set_epi64x(long long e1, long long e0) noexcept { return _mm_set_epi64x(e1, e0); }
inline __m256i set_epi64x(long long e3, long long e2, long long e1, long long e0) noexcept { return _mm256_set_epi64x(e3, e2, e1, e0); }
inline __m128i set_epi8(char e15, char e14, char e13, char e12, char e11, char e10, char e9, char e8, char e7, char e6, char e5, char e4, char e3, char e2, char e1, char e0) noexcept { return _mm_set_epi8(e15, e14, e13, e12, e11, e10, e9, e8, e7, e6, e5, e4, e3, e2, e1, e0); }
inline __m256i set_epi8(char e31, char e30, char e29, char e28, char e27, char e26, char e25, char e24, char e23, char e22, char e21, char e20, char e19, char e18, char e17, char e16, char e15, char e14, char e13, char e12, char e11, char e10, char e9, char e8, char e7, char e6, char e5, char e4, char e3, char e2, char e1, char e0) noexcept { return _mm256_set_epi8(e31, e30, e29, e28, e27, e26, e25, e24, e23, e22, e21, e20, e19, e18, e17, e16, e15, e14, e13, e12, e11, e10, e9, e8, e7, e6, e5, e4, e3, e2, e1, e0); }
inline __m256 set_m128(const __m128& hi, const __m128& lo) noexcept { return _mm256_set_m128(hi, lo); }
inline __m256d set_m128d(const __m128d& hi, const __m128d& lo) noexcept { return _mm256_set_m128d(hi, lo); }
inline __m256i set_m128i(const __m128i& hi, const __m128i& lo) noexcept { return _mm256_set_m128i(hi, lo); }
inline __m128d set_pd(double e1, double e0) noexcept { return _mm_set_pd(e1, e0); }
inline __m256d set_pd(double e3, double e2, double e1, double e0) noexcept { return _mm256_set_pd(e3, e2, e1, e0); }
inline __m128d set_pd1(double a) noexcept { return _mm_set_pd1(a); }
inline __m128 set_ps(float e3, float e2, float e1, float e0) noexcept { return _mm_set_ps(e3, e2, e1, e0); }
inline __m256 set_ps(float e7, float e6, float e5, float e4, float e3, float e2, float e1, float e0) noexcept { return _mm256_set_ps(e7, e6, e5, e4, e3, e2, e1, e0); }
inline __m128 set_ps1(float a) noexcept { return _mm_set_ps1(a); }
inline __m128d set_sd(double a) noexcept { return _mm_set_sd(a); }
inline __m128 set_ss(float a) noexcept { return _mm_set_ss(a); }
template<typename T> void set1_epi16(short) = delete;
template<> __m128i set1_epi16<__m128i>(short a) noexcept { return _mm_set1_epi16(a); }
template<> __m256i set1_epi16<__m256i>(short a) noexcept { return _mm256_set1_epi16(a); }
template<typename T> void set1_epi32(int) = delete;
template<> __m128i set1_epi32<__m128i>(int a) noexcept { return _mm_set1_epi32(a); }
template<> __m256i set1_epi32<__m256i>(int a) noexcept { return _mm256_set1_epi32(a); }
template<typename T> void set1_epi64x(long long) = delete;
template<> __m128i set1_epi64x<__m128i>(long long a) noexcept { return _mm_set1_epi64x(a); }
template<> __m256i set1_epi64x<__m256i>(long long a) noexcept { return _mm256_set1_epi64x(a); }
template<typename T> void set1_epi8(char) = delete;
template<> __m128i set1_epi8<__m128i>(char a) noexcept { return _mm_set1_epi8(a); }
template<> __m256i set1_epi8<__m256i>(char a) noexcept { return _mm256_set1_epi8(a); }
template<typename T> void set1_pd(double) = delete;
template<> __m128d set1_pd<__m128d>(double a) noexcept { return _mm_set1_pd(a); }
template<> __m256d set1_pd<__m256d>(double a) noexcept { return _mm256_set1_pd(a); }
template<typename T> void set1_ps(float) = delete;
template<> __m128 set1_ps<__m128>(float a) noexcept { return _mm_set1_ps(a); }
template<> __m256 set1_ps<__m256>(float a) noexcept { return _mm256_set1_ps(a); }
inline __m128i setr_epi16(short e0, short e1, short e2, short e3, short e4, short e5, short e6, short e7) noexcept { return _mm_setr_epi16(e0, e1, e2, e3, e4, e5, e6, e7); }
inline __m256i setr_epi16(short e0, short e1, short e2, short e3, short e4, short e5, short e6, short e7, short e8, short e9, short e10, short e11, short e12, short e13, short e14, short e15) noexcept { return _mm256_setr_epi16(e0, e1, e2, e3, e4, e5, e6, e7, e8, e9, e10, e11, e12, e13, e14, e15); }
inline __m128i setr_epi32(int e0, int e1, int e2, int e3) noexcept { return _mm_setr_epi32(e0, e1, e2, e3); }
inline __m256i setr_epi32(int e0, int e1, int e2, int e3, int e4, int e5, int e6, int e7) noexcept { return _mm256_setr_epi32(e0, e1, e2, e3, e4, e5, e6, e7); }
inline __m256i setr_epi64x(long long e0, long long e1) noexcept { return _mm256_setr_epi64x(e0, e1); }
inline __m128i setr_epi8(char e0, char e1, char e2, char e3, char e4, char e5, char e6, char e7, char e8, char e9, char e10, char e11, char e12, char e13, char e14, char e15) noexcept { return _mm_setr_epi8(e0, e1, e2, e3, e4, e5, e6, e7, e8, e9, e10, e11, e12, e13, e14, e15); }

inline __m256i setr_epi8(
  char e0, char e1, char e2, char e3, char e4, char e5, char e6, char e7,
  char e8, char e9, char e10, char e11, char e12, char e13, char e14, char e15,
  char e16, char e17, char e18, char e19, char e20, char e21, char e22, char e23,
  char e24, char e25, char e26, char e27, char e28, char e29, char e30, char e31) noexcept {
  return _mm256_setr_epi8(
    e0, e1, e2, e3, e4, e5, e6, e7, e8, e9, e10, e11, e12, e13, e14, e15, e16,
    e17, e18, e19, e20, e21, e22, e23, e24, e25, e26, e27, e28, e29, e30, e31);
}

inline __m256 setr_m128(const __m128& lo, const __m128& hi) noexcept { return _mm256_setr_m128(lo, hi); }
inline __m256d setr_m128d(const __m128d& lo, const __m128d& hi) noexcept { return _mm256_setr_m128d(lo, hi); }
inline __m256i setr_m128i(const __m128i& lo, const __m128i& hi) noexcept { return _mm256_setr_m128i(lo, hi); }
inline __m128d setr_pd(double e0, double e1) noexcept { return _mm_setr_pd(e0, e1); }
inline __m256d setr_pd(double e0, double e1, double e2, double e3) noexcept { return _mm256_setr_pd(e0, e1, e2, e3); }
inline __m128 setr_ps(float e0, float e1, float e2, float e3) noexcept { return _mm_setr_ps(e0, e1, e2, e3); }
inline __m256 setr_ps(float e0, float e1, float e2, float e3, float e4, float e5, float e6, float e7) noexcept { return _mm256_setr_ps(e0, e1, e2, e3, e4, e5, e6, e7); }
template<typename T> void setzero_pd() = delete;
template<> __m128d setzero_pd<__m128d>() noexcept { return _mm_setzero_pd(); }
template<> __m256d setzero_pd<__m256d>() noexcept { return _mm256_setzero_pd(); }
template<> __m128 setzero_ps<__m128>() noexcept { return _mm_setzero_ps(); }
template<> __m256 setzero_ps<__m256>() noexcept { return _mm256_setzero_ps(); }
template<> __m128i setzero_si128<__m128i>() noexcept { return _mm_setzero_si128(); }
template<> __m256i setzero_si256<__m256i>() noexcept { return _mm256_setzero_si256(); }

// SHIFT ///////////////////////////////////////////////////////////////////////

template<int imm8> __m256i bslli_epi128(const __m256i& a) noexcept { return _mm256_bslli_epi128(a, imm8); }
template<int imm8> __m128i bslli_si128(const __m128i& a) noexcept { return _mm_bslli_si128(a, imm8); }
template<int imm8> __m256i bsrli_epi128(const __m256i& a) noexcept { return _mm256_bsrli_epi128(a, imm8); }
template<int imm8> __m128i bsrli_si128(const __m128i& a) noexcept { return _mm_bsrli_si128(a, imm8); }
inline __m128i sll_epi16(const __m128i& a, const __m128i& count) noexcept { return _mm_sll_epi16(a, count); }
inline __m256i sll_epi16(const __m256i& a, const __m128& count) noexcept { return _mm256_sll_epi16(a, count); }
inline __m128i sll_epi32(const __m128i& a, const __m128i& count) noexcept { return _mm_sll_epi32(a, count); }
inline __m256i sll_epi32(const __m256i& a, const __m128& count) noexcept { return _mm256_sll_epi32(a, count); }
inline __m128i sll_epi64(const __m128i& a, const __m128i& count) noexcept { return _mm_sll_epi64(a, count); }
inline __m256i sll_epi64(const __m256i& a, const __m128& count) noexcept { return _mm256_sll_epi64(a, count); }
template<int imm8> __m128i slli_epi16(const __m128i& a) noexcept { return _mm_slli_epi16(a, imm8); }
template<int imm8> __m256i slli_epi16(const __m256i& a) noexcept { return _mm256_slli_epi16(a, imm8); }
template<int imm8> __m128i slli_epi32(const __m128i& a) noexcept { return _mm_slli_epi32(a, imm8); }
template<int imm8> __m256i slli_epi32(const __m256i& a) noexcept { return _mm256_slli_epi32(a, imm8); }
template<int imm8> __m128i slli_epi64(const __m128i& a) noexcept { return _mm_slli_epi64(a, imm8); }
template<int imm8> __m256i slli_epi64(const __m256i& a) noexcept { return _mm256_slli_epi64(a, imm8); }
template<int imm8> __m128i slli_si128(const __m128i& a) noexcept { return _mm_slli_si128(a, imm8); }
template<int imm8> __m256i slli_si256(const __m256i& a) noexcept { return _mm256_slli_si256(a, imm8); }
inline __m128i sllv_epi32(const __m128i& a, const __m128i& count) noexcept { return _mm_sllv_epi32(a, count); }
inline __m256i sllv_epi32(const __m256i& a, const __m128& count) noexcept { return _mm256_sllv_epi32(a, count); }
inline __m128i sllv_epi64(const __m128i& a, const __m128i& count) noexcept { return _mm_sllv_epi64(a, count); }
inline __m256i sllv_epi64(const __m256i& a, const __m128& count) noexcept { return _mm256_sllv_epi64(a, count); }
inline __m128i sra_epi16(const __m128i& a, const __m128i& count) noexcept { return _mm_sra_epi16(a, count); }
inline __m256i sra_epi16(const __m256i& a, const __m128& count) noexcept { return _mm256_sra_epi16(a, count); }
inline __m128i sra_epi32(const __m128i& a, const __m128i& count) noexcept { return _mm_sra_epi32(a, count); }
inline __m256i sra_epi32(const __m256i& a, const __m128& count) noexcept { return _mm256_sra_epi32(a, count); }
template<int imm8> __m128i srai_epi16(const __m128i& a) noexcept { return _mm_srai_epi16(a, imm8); }
template<int imm8> __m256i srai_epi16(const __m256i& a) noexcept { return _mm256_srai_epi16(a, imm8); }
template<int imm8> __m128i srai_epi32(const __m128i& a) noexcept { return _mm_srai_epi32(a, imm8); }
template<int imm8> __m256i srai_epi32(const __m256i& a) noexcept { return _mm256_srai_epi32(a, imm8); }
inline __m128i srav_epi32(const __m128i& a, const __m128i& count) noexcept { return _mm_srav_epi32(a, count); }
inline __m256i srav_epi32(const __m256i& a, const __m128& count) noexcept { return _mm256_srav_epi32(a, count); }
inline __m128i srl_epi16(const __m128i& a, const __m128i& count) noexcept { return _mm_srl_epi16(a, count); }
inline __m256i srl_epi16(const __m256i& a, const __m128& count) noexcept { return _mm256_srl_epi16(a, count); }
inline __m128i srl_epi32(const __m128i& a, const __m128i& count) noexcept { return _mm_srl_epi32(a, count); }
inline __m256i srl_epi32(const __m256i& a, const __m128& count) noexcept { return _mm256_srl_epi32(a, count); }
inline __m128i srl_epi64(const __m128i& a, const __m128i& count) noexcept { return _mm_srl_epi64(a, count); }
inline __m256i srl_epi64(const __m256i& a, const __m128& count) noexcept { return _mm256_srl_epi64(a, count); }
template<int imm8> __m128i srli_epi16(const __m128i& a) noexcept { return _mm_srli_epi16(a, imm8); }
template<int imm8> __m256i srli_epi16(const __m256i& a) noexcept { return _mm256_srli_epi16(a, imm8); }
template<int imm8> __m128i srli_epi32(const __m128i& a) noexcept { return _mm_srli_epi32(a, imm8); }
template<int imm8> __m256i srli_epi32(const __m256i& a) noexcept { return _mm256_srli_epi32(a, imm8); }
template<int imm8> __m128i srli_epi64(const __m128i& a) noexcept { return _mm_srli_epi64(a, imm8); }
template<int imm8> __m256i srli_epi64(const __m256i& a) noexcept { return _mm256_srli_epi64(a, imm8); }
template<int imm8> __m128i srli_si128(const __m128i& a) noexcept { return _mm_srli_si128(a, imm8); }
template<int imm8> __m256i srli_si256(const __m256i& a) noexcept { return _mm256_srli_si256(a, imm8); }
inline __m128i srlv_epi32(const __m128i& a, const __m128i& count) noexcept { return _mm_srlv_epi32(a, count); }
inline __m256i srlv_epi32(const __m256i& a, const __m128& count) noexcept { return _mm256_srlv_epi32(a, count); }
inline __m128i srlv_epi64(const __m128i& a, const __m128i& count) noexcept { return _mm_srlv_epi64(a, count); }
inline __m256i srlv_epi64(const __m256i& a, const __m128& count) noexcept { return _mm256_srlv_epi64(a, count); }





};
#else
import intrin;
#endif
