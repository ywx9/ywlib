/// \file intrin.hpp

#pragma once

#ifndef YWLIB
#include <immintrin.h>
export namespace mm {

template<typename T, typename U> constexpr bool _same = false;
template<typename T> constexpr bool _same<T, T> = true;

// TYPES ///////////////////////////////////////////////////////////////////////

using m128 = __m128;
using m128i = __m128i;
using m128d = __m128d;
using m256 = __m256;
using m256i = __m256i;
using m256d = __m256d;

// LOAD ////////////////////////////////////////////////////////////////////////

template<int scale> m128i i32gather_epi32(const int* a, const m128i& index) noexcept { return _mm_i32gather_epi32(a, index, scale); }
template<int scale> m256i i32gather_epi32(const int* a, const m256i& index) noexcept { return _mm256_i32gather_epi32(a, index, scale); }
template<int scale> m128i i32gather_epi64(const long long* a, const m128i& index) noexcept { return _mm_i32gather_epi64(a, index, scale); }
template<int scale> m256i i32gather_epi64(const long long* a, const m256i& index) noexcept { return _mm256_i32gather_epi64(a, index, scale); }
template<int scale> m128d i32gather_pd(const double* a, const m128i& index) noexcept { return _mm_i32gather_pd(a, index, scale); }
template<int scale> m256d i32gather_pd(const double* a, const m256i& index) noexcept { return _mm256_i32gather_pd(a, index, scale); }
template<int scale> m128 i32gather_ps(const float* a, const m128i& index) noexcept { return _mm_i32gather_ps(a, index, scale); }
template<int scale> m256 i32gather_ps(const float* a, const m256i& index) noexcept { return _mm256_i32gather_ps(a, index, scale); }
template<int scale> m128i i64gather_epi32(const int* a, const m128i& index) noexcept { return _mm_i64gather_epi32(a, index, scale); }
template<int scale> m128i i64gather_epi32(const int* a, const m256i& index) noexcept { return _mm256_i64gather_epi32(a, index, scale); }
template<int scale> m128i i64gather_epi64(const long long* a, const m128i& index) noexcept { return _mm_i64gather_epi64(a, index, scale); }
template<int scale> m256i i64gather_epi64(const long long* a, const m256i& index) noexcept { return _mm256_i64gather_epi64(a, index, scale); }
template<int scale> m128d i64gather_pd(const double* a, const m128i& index) noexcept { return _mm_i64gather_pd(a, index, scale); }
template<int scale> m256d i64gather_pd(const double* a, const m256i& index) noexcept { return _mm256_i64gather_pd(a, index, scale); }
template<int scale> m128 i64gather_ps(const float* a, const m128i& index) noexcept { return _mm_i64gather_ps(a, index, scale); }
template<int scale> m128 i64gather_ps(const float* a, const m256i& index) noexcept { return _mm256_i64gather_ps(a, index, scale); }
inline m128i lddqu_si128(const m128i* a) noexcept { return _mm_lddqu_si128(a); }
inline m256i lddqu_si256(const m256i* a) noexcept { return _mm256_lddqu_si256(a); }

/// \note `<m128d>` -> `{a[0], a[1]}`
/// \note `<m256d>` -> `{a[0], a[1], a[2], a[3]}`
template<typename T> inline constexpr auto load_pd = [](const double* a) noexcept -> T {
  if constexpr (_same<T, m128d>) return _mm_load_pd(a);
  else if constexpr (_same<T, m256d>) return _mm256_load_pd(a);
};

/// \return `{a[0], a[0]}`
inline m128d load_pd1(const double* a) noexcept { return _mm_load_pd1(a); }

/// \note `<m128>` -> `{a[0], a[1], a[2], a[3]}`
/// \note `<m256>` -> `{a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7]}`
template<typename T> inline constexpr auto load_ps = [](const float* a) noexcept -> T {
  if constexpr (_same<T, m128>) return _mm_load_ps(a);
  else if constexpr (_same<T, m256>) return _mm256_load_ps(a);
};

inline m128d load_sd(const double* a) noexcept { return _mm_load_sd(a); }
inline m128i load_si128(const m128i* a) noexcept { return _mm_load_si128(a); }
inline m256i load_si256(const m256i* a) noexcept { return _mm256_load_si256(a); }
inline m128 load_ss(const float* a) noexcept { return _mm_load_ss(a); }
inline m128d load1_pd(const double* a) noexcept { return _mm_load1_pd(a); }
inline m128 load1_ps(const float* a) noexcept { return _mm_load1_ps(a); }
inline m128d loaddup_pd(const double* a) noexcept { return _mm_loaddup_pd(a); }
inline m128i loadl_epi64(const m128i* a) noexcept { return _mm_loadl_epi64(a); }

/// \return `{mem_addr[0], a[1], a[2], a[3]}`
inline m128d loadl_pd(m128d a, const double* mem_addr) noexcept { return _mm_loadl_pd(a, mem_addr); }

inline m128d loadr_pd(const double* a) noexcept { return _mm_loadr_pd(a); }
inline m128 loadr_ps(const float* a) noexcept { return _mm_loadr_ps(a); }

/// \note `<m128d>` -> `{a[1], a[0]}`
/// \note `<m256d>` -> `{a[3], a[2], a[1], a[0]}`
template<typename T> inline constexpr auto loadu_pd = [](const double* a) noexcept -> T {
  if constexpr (_same<T, m128d>) return _mm_loadu_pd(a);
  else if constexpr (_same<T, m256d>) return _mm256_loadu_pd(a);
};

/// \note `<m128>` -> `{a[3], a[2], a[1], a[0]}`
/// \note `<m256>` -> `{a[7], a[6], a[5], a[4], a[3], a[2], a[1], a[0]}`
template<typename T> inline constexpr auto loadu_ps = [](const float* a) noexcept -> T {
  if constexpr (_same<T, m128>) return _mm_loadu_ps(a);
  else if constexpr (_same<T, m256>) return _mm256_loadu_ps(a);
};

inline m128i loadu_si128(const m128i* a) noexcept { return _mm_loadu_si128(a); }
inline m128i loadu_si16(const short* a) noexcept { return _mm_loadu_si16(a); }
inline m256i loadu_si256(const m256i* a) noexcept { return _mm256_loadu_si256(a); }
inline m128i loadu_si32(const int* a) noexcept { return _mm_loadu_si32(a); }
inline m128i loadu_si64(const long long* a) noexcept { return _mm_loadu_si64(a); }
inline m128i stream_load_si128(const m128i* a) noexcept { return _mm_stream_load_si128(a); }
inline m256i stream_load_si256(const m256i* a) noexcept { return _mm256_stream_load_si256(a); }

// ARITHMETIC //////////////////////////////////////////////////////////////////////////////////////////////////////////

// ADD /////////////////////////////////////////////////////////////////////////

inline m128 add_ps(const m128& a, const m128& b) noexcept { return _mm_add_ps(a, b); }
inline m128d add_pd(const m128d& a, const m128d& b) noexcept { return _mm_add_pd(a, b); }
inline m128i add_epi8(const m128i& a, const m128i& b) noexcept { return _mm_add_epi8(a, b); }
inline m128i add_epi16(const m128i& a, const m128i& b) noexcept { return _mm_add_epi16(a, b); }
inline m128i add_epi32(const m128i& a, const m128i& b) noexcept { return _mm_add_epi32(a, b); }
inline m128i add_epi64(const m128i& a, const m128i& b) noexcept { return _mm_add_epi64(a, b); }

inline m256 add_ps(const m256& a, const m256& b) noexcept { return _mm256_add_ps(a, b); }
inline m256d add_pd(const m256d& a, const m256d& b) noexcept { return _mm256_add_pd(a, b); }
inline m256i add_epi8(const m256i& a, const m256i& b) noexcept { return _mm256_add_epi8(a, b); }
inline m256i add_epi16(const m256i& a, const m256i& b) noexcept { return _mm256_add_epi16(a, b); }
inline m256i add_epi32(const m256i& a, const m256i& b) noexcept { return _mm256_add_epi32(a, b); }
inline m256i add_epi64(const m256i& a, const m256i& b) noexcept { return _mm256_add_epi64(a, b); }

inline m128d add_sd(const m128d& a, const m128d& b) noexcept { return _mm_add_sd(a, b); }
inline m128 add_ss(const m128& a, const m128& b) noexcept { return _mm_add_ss(a, b); }

inline m128i adds_epi16(const m128i& a, const m128i& b) noexcept { return _mm_adds_epi16(a, b); }
inline m256i adds_epi16(const m256i& a, const m256i& b) noexcept { return _mm256_adds_epi16(a, b); }
inline m128i adds_epi8(const m128i& a, const m128i& b) noexcept { return _mm_adds_epi8(a, b); }
inline m256i adds_epi8(const m256i& a, const m256i& b) noexcept { return _mm256_adds_epi8(a, b); }
inline m128i adds_epu16(const m128i& a, const m128i& b) noexcept { return _mm_adds_epu16(a, b); }
inline m256i adds_epu16(const m256i& a, const m256i& b) noexcept { return _mm256_adds_epu16(a, b); }
inline m128i adds_epu8(const m128i& a, const m128i& b) noexcept { return _mm_adds_epu8(a, b); }
inline m256i adds_epu8(const m256i& a, const m256i& b) noexcept { return _mm256_adds_epu8(a, b); }
inline m128d addsub_pd(const m128d& a, const m128d& b) noexcept { return _mm_addsub_pd(a, b); }
inline m256d addsub_pd(const m256d& a, const m256d& b) noexcept { return _mm256_addsub_pd(a, b); }
inline m128 addsub_ps(const m128& a, const m128& b) noexcept { return _mm_addsub_ps(a, b); }
inline m256 addsub_ps(const m256& a, const m256& b) noexcept { return _mm256_addsub_ps(a, b); }
inline m128i div_epi16(const m128i& a, const m128i& b) noexcept { return _mm_div_epi16(a, b); }
inline m256i div_epi16(const m256i& a, const m256i& b) noexcept { return _mm256_div_epi16(a, b); }
inline m128i div_epi32(const m128i& a, const m128i& b) noexcept { return _mm_div_epi32(a, b); }
inline m256i div_epi32(const m256i& a, const m256i& b) noexcept { return _mm256_div_epi32(a, b); }
inline m128i div_epi64(const m128i& a, const m128i& b) noexcept { return _mm_div_epi64(a, b); }
inline m256i div_epi64(const m256i& a, const m256i& b) noexcept { return _mm256_div_epi64(a, b); }
inline m128i div_epi8(const m128i& a, const m128i& b) noexcept { return _mm_div_epi8(a, b); }
inline m256i div_epi8(const m256i& a, const m256i& b) noexcept { return _mm256_div_epi8(a, b); }
inline m128i div_epu16(const m128i& a, const m128i& b) noexcept { return _mm_div_epu16(a, b); }
inline m256i div_epu16(const m256i& a, const m256i& b) noexcept { return _mm256_div_epu16(a, b); }
inline m128i div_epu32(const m128i& a, const m128i& b) noexcept { return _mm_div_epu32(a, b); }
inline m256i div_epu32(const m256i& a, const m256i& b) noexcept { return _mm256_div_epu32(a, b); }
inline m128i div_epu64(const m128i& a, const m128i& b) noexcept { return _mm_div_epu64(a, b); }
inline m256i div_epu64(const m256i& a, const m256i& b) noexcept { return _mm256_div_epu64(a, b); }
inline m128i div_epu8(const m128i& a, const m128i& b) noexcept { return _mm_div_epu8(a, b); }
inline m256i div_epu8(const m256i& a, const m256i& b) noexcept { return _mm256_div_epu8(a, b); }
inline m128d div_pd(const m128d& a, const m128d& b) noexcept { return _mm_div_pd(a, b); }
inline m256d div_pd(const m256d& a, const m256d& b) noexcept { return _mm256_div_pd(a, b); }
inline m128 div_ps(const m128& a, const m128& b) noexcept { return _mm_div_ps(a, b); }
inline m256 div_ps(const m256& a, const m256& b) noexcept { return _mm256_div_ps(a, b); }
inline m128d div_sd(const m128d& a, const m128d& b) noexcept { return _mm_div_sd(a, b); }
inline m128 div_ss(const m128& a, const m128& b) noexcept { return _mm_div_ss(a, b); }
template<int imm8> m128d dp_pd(const m128d& a, const m128d& b) noexcept { return _mm_dp_pd(a, b, imm8); }
template<int imm8> m128 dp_ps(const m128& a, const m128& b) noexcept { return _mm_dp_ps(a, b, imm8); }
template<int imm8> m256 dp_ps(const m256& a, const m256& b) noexcept { return _mm256_dp_ps(a, b, imm8); }
inline m128d erf_pd(const m128d& a) noexcept { return _mm_erf_pd(a); }
inline m128d fmadd_pd(const m128d& a, const m128d& b, const m128d& c) noexcept { return _mm_fmadd_pd(a, b, c); }
inline m256d fmadd_pd(const m256d& a, const m256d& b, const m256d& c) noexcept { return _mm256_fmadd_pd(a, b, c); }
inline m128 fmadd_ps(const m128& a, const m128& b, const m128& c) noexcept { return _mm_fmadd_ps(a, b, c); }
inline m256 fmadd_ps(const m256& a, const m256& b, const m256& c) noexcept { return _mm256_fmadd_ps(a, b, c); }
inline m128d fmadd_sd(const m128d& a, const m128d& b, const m128d& c) noexcept { return _mm_fmadd_sd(a, b, c); }
inline m128 fmadd_ss(const m128& a, const m128& b, const m128& c) noexcept { return _mm_fmadd_ss(a, b, c); }
inline m128d fmaddsub_pd(const m128d& a, const m128d& b, const m128d& c) noexcept { return _mm_fmaddsub_pd(a, b, c); }
inline m256d fmaddsub_pd(const m256d& a, const m256d& b, const m256d& c) noexcept { return _mm256_fmaddsub_pd(a, b, c); }
inline m128 fmaddsub_ps(const m128& a, const m128& b, const m128& c) noexcept { return _mm_fmaddsub_ps(a, b, c); }
inline m256 fmaddsub_ps(const m256& a, const m256& b, const m256& c) noexcept { return _mm256_fmaddsub_ps(a, b, c); }
inline m128d fmsub_pd(const m128d& a, const m128d& b, const m128d& c) noexcept { return _mm_fmsub_pd(a, b, c); }
inline m256d fmsub_pd(const m256d& a, const m256d& b, const m256d& c) noexcept { return _mm256_fmsub_pd(a, b, c); }
inline m128 fmsub_ps(const m128& a, const m128& b, const m128& c) noexcept { return _mm_fmsub_ps(a, b, c); }
inline m256 fmsub_ps(const m256& a, const m256& b, const m256& c) noexcept { return _mm256_fmsub_ps(a, b, c); }
inline m128d fmsub_sd(const m128d& a, const m128d& b, const m128d& c) noexcept { return _mm_fmsub_sd(a, b, c); }
inline m128 fmsub_ss(const m128& a, const m128& b, const m128& c) noexcept { return _mm_fmsub_ss(a, b, c); }
inline m128d fmsubadd_pd(const m128d& a, const m128d& b, const m128d& c) noexcept { return _mm_fmsubadd_pd(a, b, c); }
inline m128 fmsubadd_ps(const m128& a, const m128& b, const m128& c) noexcept { return _mm_fmsubadd_ps(a, b, c); }
inline m256 fmsubadd_ps(const m256& a, const m256& b, const m256& c) noexcept { return _mm256_fmsubadd_ps(a, b, c); }
inline m128d fnmadd_pd(const m128d& a, const m128d& b, const m128d& c) noexcept { return _mm_fnmadd_pd(a, b, c); }
inline m256d fnmadd_pd(const m256d& a, const m256d& b, const m256d& c) noexcept { return _mm256_fnmadd_pd(a, b, c); }
inline m128 fnmadd_ps(const m128& a, const m128& b, const m128& c) noexcept { return _mm_fnmadd_ps(a, b, c); }
inline m256 fnmadd_ps(const m256& a, const m256& b, const m256& c) noexcept { return _mm256_fnmadd_ps(a, b, c); }
inline m128d fnmadd_sd(const m128d& a, const m128d& b, const m128d& c) noexcept { return _mm_fnmadd_sd(a, b, c); }
inline m128 fnmadd_ss(const m128& a, const m128& b, const m128& c) noexcept { return _mm_fnmadd_ss(a, b, c); }
inline m128d fnmsub_pd(const m128d& a, const m128d& b, const m128d& c) noexcept { return _mm_fnmsub_pd(a, b, c); }
inline m256d fnmsub_pd(const m256d& a, const m256d& b, const m256d& c) noexcept { return _mm256_fnmsub_pd(a, b, c); }
inline m128 fnmsub_ps(const m128& a, const m128& b, const m128& c) noexcept { return _mm_fnmsub_ps(a, b, c); }
inline m256 fnmsub_ps(const m256& a, const m256& b, const m256& c) noexcept { return _mm256_fnmsub_ps(a, b, c); }
inline m128d fnmsub_sd(const m128d& a, const m128d& b, const m128d& c) noexcept { return _mm_fnmsub_sd(a, b, c); }
inline m128 fnmsub_ss(const m128& a, const m128& b, const m128& c) noexcept { return _mm_fnmsub_ss(a, b, c); }
inline m128i hadd_epi16(const m128i& a, const m128i& b) noexcept { return _mm_hadd_epi16(a, b); }
inline m256i hadd_epi16(const m256i& a, const m256i& b) noexcept { return _mm256_hadd_epi16(a, b); }
inline m128i hadd_epi32(const m128i& a, const m128i& b) noexcept { return _mm_hadd_epi32(a, b); }
inline m256i hadd_epi32(const m256i& a, const m256i& b) noexcept { return _mm256_hadd_epi32(a, b); }
inline m128d hadd_pd(const m128d& a, const m128d& b) noexcept { return _mm_hadd_pd(a, b); }
inline m256d hadd_pd(const m256d& a, const m256d& b) noexcept { return _mm256_hadd_pd(a, b); }
inline m128 hadd_ps(const m128& a, const m128& b) noexcept { return _mm_hadd_ps(a, b); }
inline m256 hadd_ps(const m256& a, const m256& b) noexcept { return _mm256_hadd_ps(a, b); }
inline m128i hadds_epi16(const m128i& a, const m128i& b) noexcept { return _mm_hadds_epi16(a, b); }
inline m256i hadds_epi16(const m256i& a, const m256i& b) noexcept { return _mm256_hadds_epi16(a, b); }
inline m128i hsub_epi16(const m128i& a, const m128i& b) noexcept { return _mm_hsub_epi16(a, b); }
inline m256i hsub_epi16(const m256i& a, const m256i& b) noexcept { return _mm256_hsub_epi16(a, b); }
inline m128i hsub_epi32(const m128i& a, const m128i& b) noexcept { return _mm_hsub_epi32(a, b); }
inline m256i hsub_epi32(const m256i& a, const m256i& b) noexcept { return _mm256_hsub_epi32(a, b); }
inline m128d hsub_pd(const m128d& a, const m128d& b) noexcept { return _mm_hsub_pd(a, b); }
inline m256d hsub_pd(const m256d& a, const m256d& b) noexcept { return _mm256_hsub_pd(a, b); }
inline m128 hsub_ps(const m128& a, const m128& b) noexcept { return _mm_hsub_ps(a, b); }
inline m256 hsub_ps(const m256& a, const m256& b) noexcept { return _mm256_hsub_ps(a, b); }
inline m128i hsubs_epi16(const m128i& a, const m128i& b) noexcept { return _mm_hsubs_epi16(a, b); }
inline m256i hsubs_epi16(const m256i& a, const m256i& b) noexcept { return _mm256_hsubs_epi16(a, b); }
inline m128i idiv_epi32(const m128i& a, const m128i& b) noexcept { return _mm_idiv_epi32(a, b); }
inline m256i idiv_epi32(const m256i& a, const m256i& b) noexcept { return _mm256_idiv_epi32(a, b); }
inline m128i idivrem_epi32(m128i* mem_addr, const m128i& a, const m128i& b) noexcept { return _mm_idivrem_epi32(mem_addr, a, b); }
inline m256i idivrem_epi32(m256i* mem_addr, const m256i& a, const m256i& b) noexcept { return _mm256_idivrem_epi32(mem_addr, a, b); }
inline m128i irem_epi32(const m128i& a, const m128i& b) noexcept { return _mm_irem_epi32(a, b); }
inline m256i irem_epi32(const m256i& a, const m256i& b) noexcept { return _mm256_irem_epi32(a, b); }
inline m128i madd_epi16(const m128i& a, const m128i& b) noexcept { return _mm_madd_epi16(a, b); }
inline m256i madd_epi16(const m256i& a, const m256i& b) noexcept { return _mm256_madd_epi16(a, b); }
inline m128i maddubs_epi16(const m128i& a, const m128i& b) noexcept { return _mm_maddubs_epi16(a, b); }
inline m256i maddubs_epi16(const m256i& a, const m256i& b) noexcept { return _mm256_maddubs_epi16(a, b); }
inline m128i mpsadbw_epu8(const m128i& a, const m128i& b) noexcept { return _mm_mpsadbw_epu8(a, b, 0); }
inline m128i mul_epi32(const m128i& a, const m128i& b) noexcept { return _mm_mul_epi32(a, b); }
inline m256i mul_epi32(const m256i& a, const m256i& b) noexcept { return _mm256_mul_epi32(a, b); }
inline m128i mul_epu32(const m128i& a, const m128i& b) noexcept { return _mm_mul_epu32(a, b); }
inline m256i mul_epu32(const m256i& a, const m256i& b) noexcept { return _mm256_mul_epu32(a, b); }
inline m128d mul_pd(const m128d& a, const m128d& b) noexcept { return _mm_mul_pd(a, b); }
inline m256d mul_pd(const m256d& a, const m256d& b) noexcept { return _mm256_mul_pd(a, b); }
inline m128 mul_ps(const m128& a, const m128& b) noexcept { return _mm_mul_ps(a, b); }
inline m256 mul_ps(const m256& a, const m256& b) noexcept { return _mm256_mul_ps(a, b); }
inline m128d mul_sd(const m128d& a, const m128d& b) noexcept { return _mm_mul_sd(a, b); }
inline m128 mul_ss(const m128& a, const m128& b) noexcept { return _mm_mul_ss(a, b); }
inline m128i mulhi_epi16(const m128i& a, const m128i& b) noexcept { return _mm_mulhi_epi16(a, b); }
inline m256i mulhi_epi16(const m256i& a, const m256i& b) noexcept { return _mm256_mulhi_epi16(a, b); }
inline m128i mulhi_epu16(const m128i& a, const m128i& b) noexcept { return _mm_mulhi_epu16(a, b); }
inline m256i mulhi_epu16(const m256i& a, const m256i& b) noexcept { return _mm256_mulhi_epu16(a, b); }
inline m128i mulhrs_epi16(const m128i& a, const m128i& b) noexcept { return _mm_mulhrs_epi16(a, b); }
inline m256i mulhrs_epi16(const m256i& a, const m256i& b) noexcept { return _mm256_mulhrs_epi16(a, b); }
inline m128i mullo_epi16(const m128i& a, const m128i& b) noexcept { return _mm_mullo_epi16(a, b); }
inline m256i mullo_epi16(const m256i& a, const m256i& b) noexcept { return _mm256_mullo_epi16(a, b); }
inline m128i mullo_epi32(const m128i& a, const m128i& b) noexcept { return _mm_mullo_epi32(a, b); }
inline m256i mullo_epi32(const m256i& a, const m256i& b) noexcept { return _mm256_mullo_epi32(a, b); }
inline m128i rem_epi16(const m128i& a, const m128i& b) noexcept { return _mm_rem_epi16(a, b); }
inline m256i rem_epi16(const m256i& a, const m256i& b) noexcept { return _mm256_rem_epi16(a, b); }
inline m128i rem_epi32(const m128i& a, const m128i& b) noexcept { return _mm_rem_epi32(a, b); }
inline m256i rem_epi32(const m256i& a, const m256i& b) noexcept { return _mm256_rem_epi32(a, b); }
inline m128i rem_epi64(const m128i& a, const m128i& b) noexcept { return _mm_rem_epi64(a, b); }
inline m256i rem_epi64(const m256i& a, const m256i& b) noexcept { return _mm256_rem_epi64(a, b); }
inline m128i rem_epi8(const m128i& a, const m128i& b) noexcept { return _mm_rem_epi8(a, b); }
inline m256i rem_epi8(const m256i& a, const m256i& b) noexcept { return _mm256_rem_epi8(a, b); }
inline m128i rem_epu16(const m128i& a, const m128i& b) noexcept { return _mm_rem_epu16(a, b); }
inline m256i rem_epu16(const m256i& a, const m256i& b) noexcept { return _mm256_rem_epu16(a, b); }
inline m128i rem_epu32(const m128i& a, const m128i& b) noexcept { return _mm_rem_epu32(a, b); }
inline m256i rem_epu32(const m256i& a, const m256i& b) noexcept { return _mm256_rem_epu32(a, b); }
inline m128i rem_epu64(const m128i& a, const m128i& b) noexcept { return _mm_rem_epu64(a, b); }
inline m256i rem_epu64(const m256i& a, const m256i& b) noexcept { return _mm256_rem_epu64(a, b); }
inline m128i rem_epu8(const m128i& a, const m128i& b) noexcept { return _mm_rem_epu8(a, b); }
inline m256i rem_epu8(const m256i& a, const m256i& b) noexcept { return _mm256_rem_epu8(a, b); }
inline m128i sad_epu8(const m128i& a, const m128i& b) noexcept { return _mm_sad_epu8(a, b); }
inline m256i sad_epu8(const m256i& a, const m256i& b) noexcept { return _mm256_sad_epu8(a, b); }
inline m128i sign_epi16(const m128i& a, const m128i& b) noexcept { return _mm_sign_epi16(a, b); }
inline m256i sign_epi16(const m256i& a, const m256i& b) noexcept { return _mm256_sign_epi16(a, b); }
inline m128i sign_epi32(const m128i& a, const m128i& b) noexcept { return _mm_sign_epi32(a, b); }
inline m256i sign_epi32(const m256i& a, const m256i& b) noexcept { return _mm256_sign_epi32(a, b); }
inline m128i sign_epi8(const m128i& a, const m128i& b) noexcept { return _mm_sign_epi8(a, b); }
inline m256i sign_epi8(const m256i& a, const m256i& b) noexcept { return _mm256_sign_epi8(a, b); }
inline m128i sub_epi16(const m128i& a, const m128i& b) noexcept { return _mm_sub_epi16(a, b); }
inline m256i sub_epi16(const m256i& a, const m256i& b) noexcept { return _mm256_sub_epi16(a, b); }
inline m128i sub_epi32(const m128i& a, const m128i& b) noexcept { return _mm_sub_epi32(a, b); }
inline m256i sub_epi32(const m256i& a, const m256i& b) noexcept { return _mm256_sub_epi32(a, b); }
inline m128i sub_epi64(const m128i& a, const m128i& b) noexcept { return _mm_sub_epi64(a, b); }
inline m256i sub_epi64(const m256i& a, const m256i& b) noexcept { return _mm256_sub_epi64(a, b); }
inline m128i sub_epi8(const m128i& a, const m128i& b) noexcept { return _mm_sub_epi8(a, b); }
inline m256i sub_epi8(const m256i& a, const m256i& b) noexcept { return _mm256_sub_epi8(a, b); }
inline m128d sub_pd(const m128d& a, const m128d& b) noexcept { return _mm_sub_pd(a, b); }
inline m256d sub_pd(const m256d& a, const m256d& b) noexcept { return _mm256_sub_pd(a, b); }
inline m128 sub_ps(const m128& a, const m128& b) noexcept { return _mm_sub_ps(a, b); }
inline m256 sub_ps(const m256& a, const m256& b) noexcept { return _mm256_sub_ps(a, b); }
inline m128d sub_sd(const m128d& a, const m128d& b) noexcept { return _mm_sub_sd(a, b); }
inline m128 sub_ss(const m128& a, const m128& b) noexcept { return _mm_sub_ss(a, b); }
inline m128i subs_epi16(const m128i& a, const m128i& b) noexcept { return _mm_subs_epi16(a, b); }
inline m256i subs_epi16(const m256i& a, const m256i& b) noexcept { return _mm256_subs_epi16(a, b); }
inline m128i subs_epi8(const m128i& a, const m128i& b) noexcept { return _mm_subs_epi8(a, b); }
inline m256i subs_epi8(const m256i& a, const m256i& b) noexcept { return _mm256_subs_epi8(a, b); }
inline m128i subs_epu16(const m128i& a, const m128i& b) noexcept { return _mm_subs_epu16(a, b); }
inline m256i subs_epu16(const m256i& a, const m256i& b) noexcept { return _mm256_subs_epu16(a, b); }
inline m128i subs_epu8(const m128i& a, const m128i& b) noexcept { return _mm_subs_epu8(a, b); }
inline m256i subs_epu8(const m256i& a, const m256i& b) noexcept { return _mm256_subs_epu8(a, b); }
inline m128i udiv_epi32(const m128i& a, const m128i& b) noexcept { return _mm_udiv_epi32(a, b); }
inline m256i udiv_epi32(const m256i& a, const m256i& b) noexcept { return _mm256_udiv_epi32(a, b); }
inline m128i udivrem_epi32(m128i* mem_addr, const m128i& a, const m128i& b) noexcept { return _mm_udivrem_epi32(mem_addr, a, b); }
inline m256i udivrem_epi32(m256i* mem_addr, const m256i& a, const m256i& b) noexcept { return _mm256_udivrem_epi32(mem_addr, a, b); }
inline m128i urem_epi32(const m128i& a, const m128i& b) noexcept { return _mm_urem_epi32(a, b); }
inline m256i urem_epi32(const m256i& a, const m256i& b) noexcept { return _mm256_urem_epi32(a, b); }

// BIT MANIPULATION ////////////////////////////////////////////////////////////

// BROADCAST ///////////////////////////////////////////////////////////////////

/// \return `{a, a}`
inline m256d broadcast_pd(const m128d* mem_addr) noexcept { return _mm256_broadcast_pd(mem_addr); }

/// \return `{a, a}`
inline m256 broadcast_ps(const m128* mem_addr) noexcept { return _mm256_broadcast_ps(mem_addr); }

/// \return `{a, a, a, a}`
inline m256d broadcast_sd(const double* mem_addr) noexcept { return _mm256_broadcast_sd(mem_addr); }

/// \note `<m128>` -> `{a, a, a, a}`
/// \note `<m256>` -> `{a, a, a, a, a, a, a, a}`
template<typename T> inline constexpr auto broadcast_ss = [](const float* mem_addr) noexcept -> T {
  if constexpr (_same<T, m128>) return _mm_broadcast_ss(mem_addr);
  else if constexpr (_same<T, m256>) return _mm256_broadcast_ss(mem_addr);
};

/// \note `<m128i>` -> `{a[0], ...x32}`
/// \note `<m256i>` -> `{a[0], ...x64}`
template<typename T> inline constexpr auto broadcastb_epi8 = [](const m128i& a) noexcept -> T {
  if constexpr (_same<T, m128i>) return _mm_broadcastb_epi8(a);
  else if constexpr (_same<T, m256i>) return _mm256_broadcastb_epi8(a);
};

/// \note `<m128i>` -> `{a[0], a[0], a[0], a[0]}`
/// \note `<m256i>` -> `{a[0], a[0], a[0], a[0], a[0], a[0], a[0], a[0]}`
template<typename T> inline constexpr auto broadcastd_epi32 = [](const m128i& a) noexcept -> T {
  if constexpr (_same<T, m128i>) return _mm_broadcastd_epi32(a);
  else if constexpr (_same<T, m256i>) return _mm256_broadcastd_epi32(a);
};

/// \note `<m128i>` -> `{a[0], a[0]}`
/// \note `<m256i>` -> `{a[0], a[0], a[0], a[0]}`
template<typename T> inline constexpr auto broadcastq_epi64 = [](const m128i& a) noexcept -> T {
  if constexpr (_same<T, m128i>) return _mm_broadcastq_epi64(a);
  else if constexpr (_same<T, m256i>) return _mm256_broadcastq_epi64(a);
};

/// \note `<m128d>` -> `{a[0], a[0]}`
/// \note `<m256d>` -> `{a[0], a[0], a[0], a[0]}`
template<typename T> inline constexpr auto broadcastsd_pd = [](const m128d& a) noexcept -> T {
  if constexpr (_same<T, m128d>) return _mm_broadcastsd_pd(a);
  else if constexpr (_same<T, m256d>) return _mm256_broadcastsd_pd(a);
};

/// \return `{a, 0}`
inline m256i broadcastsi128_si256(const m128i& a) noexcept { return _mm256_broadcastsi128_si256(a); }

/// \note `<m128>` -> `{a[0], a[0], a[0], a[0]}`
/// \note `<m256>` -> `{a[0], a[0], a[0], a[0], a[0], a[0], a[0], a[0]}`
template<typename T> inline constexpr auto broadcastss_ps = [](const m128& a) noexcept -> T {
  if constexpr (_same<T, m128>) return _mm_broadcastss_ps(a);
  else if constexpr (_same<T, m256>) return _mm256_broadcastss_ps(a);
};

/// \note `<m128i>` -> `{a[0], a[0], ...}`
/// \note `<m256i>` -> `{a[0], a[0], ...}`
template<typename T> inline constexpr auto broadcastw_epi16 = [](const m128i& a) noexcept -> T {
  if constexpr (_same<T, m128i>) return _mm_broadcastw_epi16(a);
  else if constexpr (_same<T, m256i>) return _mm256_broadcastw_epi16(a);
};


// CAST ////////////////////////////////////////////////////////////////////////

inline m128 castpd_ps(const m128d& a) noexcept { return _mm_castpd_ps(a); }
inline m256 castpd_ps(const m256d& a) noexcept { return _mm256_castpd_ps(a); }
inline m128i castpd_si128(const m128d& a) noexcept { return _mm_castpd_si128(a); }
inline m256i castpd_si256(const m256d& a) noexcept { return _mm256_castpd_si256(a); }
inline m256d castpd128_pd256(const m128d& a) noexcept { return _mm256_castpd128_pd256(a); }
inline m128d castpd256_pd128(const m256d& a) noexcept { return _mm256_castpd256_pd128(a); }
inline m128d castps_pd(const m128& a) noexcept { return _mm_castps_pd(a); }
inline m256d castps_pd(const m256& a) noexcept { return _mm256_castps_pd(a); }
inline m128i castps_si128(const m128& a) noexcept { return _mm_castps_si128(a); }
inline m256i castps_si256(const m256& a) noexcept { return _mm256_castps_si256(a); }
inline m256 castps128_ps256(const m128& a) noexcept { return _mm256_castps128_ps256(a); }
inline m128 castps256_ps128(const m256& a) noexcept { return _mm256_castps256_ps128(a); }
inline m128d castsi128_pd(const m128i& a) noexcept { return _mm_castsi128_pd(a); }
inline m128 castsi128_ps(const m128i& a) noexcept { return _mm_castsi128_ps(a); }
inline m256i castsi128_si256(const m128i& a) noexcept { return _mm256_castsi128_si256(a); }
inline m256d castsi256_pd(const m256i& a) noexcept { return _mm256_castsi256_pd(a); }
inline m256 castsi256_ps(const m256i& a) noexcept { return _mm256_castsi256_ps(a); }
inline m128i castsi256_si128(const m256i& a) noexcept { return _mm256_castsi256_si128(a); }
inline m256d zextpd128_pd256(const m128d& a) noexcept { return _mm256_zextpd128_pd256(a); }
inline m256 zextps128_ps256(const m128& a) noexcept { return _mm256_zextps128_ps256(a); }
inline m256i zextsi128_si256(const m128i& a) noexcept { return _mm256_zextsi128_si256(a); }

// COMPARE /////////////////////////////////////////////////////////////////////

template<int imm8> m128d cmp_pd(const m128d& a, const m128d& b) noexcept { return _mm_cmp_pd(a, b, imm8); }
template<int imm8> m256d cmp_pd(const m256d& a, const m256d& b) noexcept { return _mm256_cmp_pd(a, b, imm8); }
template<int imm8> m128 cmp_ps(const m128& a, const m128& b) noexcept { return _mm_cmp_ps(a, b, imm8); }
template<int imm8> m256 cmp_ps(const m256& a, const m256& b) noexcept { return _mm256_cmp_ps(a, b, imm8); }
template<int imm8> m128d cmp_sd(const m128d& a, const m128d& b) noexcept { return _mm_cmp_sd(a, b, imm8); }
template<int imm8> m128 cmp_ss(const m128& a, const m128& b) noexcept { return _mm_cmp_ss(a, b, imm8); }
inline m128i cmpeq_epi16(const m128i& a, const m128i& b) noexcept { return _mm_cmpeq_epi16(a, b); }
inline m256i cmpeq_epi16(const m256i& a, const m256i& b) noexcept { return _mm256_cmpeq_epi16(a, b); }
inline m128i cmpeq_epi32(const m128i& a, const m128i& b) noexcept { return _mm_cmpeq_epi32(a, b); }
inline m256i cmpeq_epi32(const m256i& a, const m256i& b) noexcept { return _mm256_cmpeq_epi32(a, b); }
inline m128i cmpeq_epi64(const m128i& a, const m128i& b) noexcept { return _mm_cmpeq_epi64(a, b); }
inline m256i cmpeq_epi64(const m256i& a, const m256i& b) noexcept { return _mm256_cmpeq_epi64(a, b); }
inline m128i cmpeq_epi8(const m128i& a, const m128i& b) noexcept { return _mm_cmpeq_epi8(a, b); }
inline m256i cmpeq_epi8(const m256i& a, const m256i& b) noexcept { return _mm256_cmpeq_epi8(a, b); }
inline m128d cmpeq_pd(const m128d& a, const m128d& b) noexcept { return _mm_cmpeq_pd(a, b); }
inline m128 cmpeq_ps(const m128& a, const m128& b) noexcept { return _mm_cmpeq_ps(a, b); }
inline m128d cmpeq_sd(const m128d& a, const m128d& b) noexcept { return _mm_cmpeq_sd(a, b); }
inline m128 cmpeq_ss(const m128& a, const m128& b) noexcept { return _mm_cmpeq_ss(a, b); }
inline m128d cmpge_pd(const m128d& a, const m128d& b) noexcept { return _mm_cmpge_pd(a, b); }
inline m128 cmpge_ps(const m128& a, const m128& b) noexcept { return _mm_cmpge_ps(a, b); }
inline m128d cmpge_sd(const m128d& a, const m128d& b) noexcept { return _mm_cmpge_sd(a, b); }
inline m128 cmpge_ss(const m128& a, const m128& b) noexcept { return _mm_cmpge_ss(a, b); }
inline m128i cmpgt_epi16(const m128i& a, const m128i& b) noexcept { return _mm_cmpgt_epi16(a, b); }
inline m256i cmpgt_epi16(const m256i& a, const m256i& b) noexcept { return _mm256_cmpgt_epi16(a, b); }
inline m128i cmpgt_epi32(const m128i& a, const m128i& b) noexcept { return _mm_cmpgt_epi32(a, b); }
inline m256i cmpgt_epi32(const m256i& a, const m256i& b) noexcept { return _mm256_cmpgt_epi32(a, b); }
inline m128i cmpgt_epi64(const m128i& a, const m128i& b) noexcept { return _mm_cmpgt_epi64(a, b); }
inline m256i cmpgt_epi64(const m256i& a, const m256i& b) noexcept { return _mm256_cmpgt_epi64(a, b); }
inline m128i cmpgt_epi8(const m128i& a, const m128i& b) noexcept { return _mm_cmpgt_epi8(a, b); }
inline m256i cmpgt_epi8(const m256i& a, const m256i& b) noexcept { return _mm256_cmpgt_epi8(a, b); }
inline m128d cmpgt_pd(const m128d& a, const m128d& b) noexcept { return _mm_cmpgt_pd(a, b); }
inline m128 cmpgt_ps(const m128& a, const m128& b) noexcept { return _mm_cmpgt_ps(a, b); }
inline m128d cmpgt_sd(const m128d& a, const m128d& b) noexcept { return _mm_cmpgt_sd(a, b); }
inline m128 cmpgt_ss(const m128& a, const m128& b) noexcept { return _mm_cmpgt_ss(a, b); }
inline m128d cmple_pd(const m128d& a, const m128d& b) noexcept { return _mm_cmple_pd(a, b); }
inline m128 cmple_ps(const m128& a, const m128& b) noexcept { return _mm_cmple_ps(a, b); }
inline m128d cmple_sd(const m128d& a, const m128d& b) noexcept { return _mm_cmple_sd(a, b); }
inline m128 cmple_ss(const m128& a, const m128& b) noexcept { return _mm_cmple_ss(a, b); }
inline m128i cmplt_epi16(const m128i& a, const m128i& b) noexcept { return _mm_cmplt_epi16(a, b); }
inline m128i cmplt_epi32(const m128i& a, const m128i& b) noexcept { return _mm_cmplt_epi32(a, b); }
inline m128i cmplt_epi8(const m128i& a, const m128i& b) noexcept { return _mm_cmplt_epi8(a, b); }
inline m128d cmplt_pd(const m128d& a, const m128d& b) noexcept { return _mm_cmplt_pd(a, b); }
inline m128 cmplt_ps(const m128& a, const m128& b) noexcept { return _mm_cmplt_ps(a, b); }
inline m128d cmplt_sd(const m128d& a, const m128d& b) noexcept { return _mm_cmplt_sd(a, b); }
inline m128 cmplt_ss(const m128& a, const m128& b) noexcept { return _mm_cmplt_ss(a, b); }
inline m128d cmpneq_pd(const m128d& a, const m128d& b) noexcept { return _mm_cmpneq_pd(a, b); }
inline m128 cmpneq_ps(const m128& a, const m128& b) noexcept { return _mm_cmpneq_ps(a, b); }
inline m128d cmpneq_sd(const m128d& a, const m128d& b) noexcept { return _mm_cmpneq_sd(a, b); }
inline m128 cmpneq_ss(const m128& a, const m128& b) noexcept { return _mm_cmpneq_ss(a, b); }
inline m128d cmpnge_pd(const m128d& a, const m128d& b) noexcept { return _mm_cmpnge_pd(a, b); }
inline m128 cmpnge_ps(const m128& a, const m128& b) noexcept { return _mm_cmpnge_ps(a, b); }
inline m128d cmpnge_sd(const m128d& a, const m128d& b) noexcept { return _mm_cmpnge_sd(a, b); }
inline m128 cmpnge_ss(const m128& a, const m128& b) noexcept { return _mm_cmpnge_ss(a, b); }
inline m128d cmpngt_pd(const m128d& a, const m128d& b) noexcept { return _mm_cmpngt_pd(a, b); }
inline m128 cmpngt_ps(const m128& a, const m128& b) noexcept { return _mm_cmpngt_ps(a, b); }
inline m128d cmpngt_sd(const m128d& a, const m128d& b) noexcept { return _mm_cmpngt_sd(a, b); }
inline m128 cmpngt_ss(const m128& a, const m128& b) noexcept { return _mm_cmpngt_ss(a, b); }
inline m128d cmpnle_pd(const m128d& a, const m128d& b) noexcept { return _mm_cmpnle_pd(a, b); }
inline m128 cmpnle_ps(const m128& a, const m128& b) noexcept { return _mm_cmpnle_ps(a, b); }
inline m128d cmpnle_sd(const m128d& a, const m128d& b) noexcept { return _mm_cmpnle_sd(a, b); }
inline m128 cmpnle_ss(const m128& a, const m128& b) noexcept { return _mm_cmpnle_ss(a, b); }
inline m128d cmpnlt_pd(const m128d& a, const m128d& b) noexcept { return _mm_cmpnlt_pd(a, b); }
inline m128 cmpnlt_ps(const m128& a, const m128& b) noexcept { return _mm_cmpnlt_ps(a, b); }
inline m128d cmpnlt_sd(const m128d& a, const m128d& b) noexcept { return _mm_cmpnlt_sd(a, b); }
inline m128 cmpnlt_ss(const m128& a, const m128& b) noexcept { return _mm_cmpnlt_ss(a, b); }
inline m128d cmpord_pd(const m128d& a, const m128d& b) noexcept { return _mm_cmpord_pd(a, b); }
inline m128 cmpord_ps(const m128& a, const m128& b) noexcept { return _mm_cmpord_ps(a, b); }
inline m128d cmpord_sd(const m128d& a, const m128d& b) noexcept { return _mm_cmpord_sd(a, b); }
inline m128 cmpord_ss(const m128& a, const m128& b) noexcept { return _mm_cmpord_ss(a, b); }
inline m128d cmpunord_pd(const m128d& a, const m128d& b) noexcept { return _mm_cmpunord_pd(a, b); }
inline m128 cmpunord_ps(const m128& a, const m128& b) noexcept { return _mm_cmpunord_ps(a, b); }
inline m128d cmpunord_sd(const m128d& a, const m128d& b) noexcept { return _mm_cmpunord_sd(a, b); }
inline m128 cmpunord_ss(const m128& a, const m128& b) noexcept { return _mm_cmpunord_ss(a, b); }
inline int comieq_sd(const m128d& a, const m128d& b) noexcept { return _mm_comieq_sd(a, b); }
inline int comieq_ss(const m128& a, const m128& b) noexcept { return _mm_comieq_ss(a, b); }
inline int comige_sd(const m128d& a, const m128d& b) noexcept { return _mm_comige_sd(a, b); }
inline int comige_ss(const m128& a, const m128& b) noexcept { return _mm_comige_ss(a, b); }
inline int comigt_sd(const m128d& a, const m128d& b) noexcept { return _mm_comigt_sd(a, b); }
inline int comigt_ss(const m128& a, const m128& b) noexcept { return _mm_comigt_ss(a, b); }
inline int comile_sd(const m128d& a, const m128d& b) noexcept { return _mm_comile_sd(a, b); }
inline int comile_ss(const m128& a, const m128& b) noexcept { return _mm_comile_ss(a, b); }
inline int comilt_sd(const m128d& a, const m128d& b) noexcept { return _mm_comilt_sd(a, b); }
inline int comilt_ss(const m128& a, const m128& b) noexcept { return _mm_comilt_ss(a, b); }
inline int comineq_sd(const m128d& a, const m128d& b) noexcept { return _mm_comineq_sd(a, b); }
inline int comineq_ss(const m128& a, const m128& b) noexcept { return _mm_comineq_ss(a, b); }
inline int ucomieq_sd(const m128d& a, const m128d& b) noexcept { return _mm_ucomieq_sd(a, b); }
inline int ucomieq_ss(const m128& a, const m128& b) noexcept { return _mm_ucomieq_ss(a, b); }
inline int ucomige_sd(const m128d& a, const m128d& b) noexcept { return _mm_ucomige_sd(a, b); }
inline int ucomige_ss(const m128& a, const m128& b) noexcept { return _mm_ucomige_ss(a, b); }
inline int ucomigt_sd(const m128d& a, const m128d& b) noexcept { return _mm_ucomigt_sd(a, b); }
inline int ucomigt_ss(const m128& a, const m128& b) noexcept { return _mm_ucomigt_ss(a, b); }
inline int ucomile_sd(const m128d& a, const m128d& b) noexcept { return _mm_ucomile_sd(a, b); }
inline int ucomile_ss(const m128& a, const m128& b) noexcept { return _mm_ucomile_ss(a, b); }
inline int ucomilt_sd(const m128d& a, const m128d& b) noexcept { return _mm_ucomilt_sd(a, b); }
inline int ucomilt_ss(const m128& a, const m128& b) noexcept { return _mm_ucomilt_ss(a, b); }
inline int ucomineq_sd(const m128d& a, const m128d& b) noexcept { return _mm_ucomineq_sd(a, b); }
inline int ucomineq_ss(const m128& a, const m128& b) noexcept { return _mm_ucomineq_ss(a, b); }

// CONVERT /////////////////////////////////////////////////////////////////////

/// \return `{float(a.i4[0]), a.e1, a.e2, a.e3}`
inline m128 cvt_si2ss(const m128& a, int b) noexcept { return _mm_cvt_si2ss(a, b); }

/// \return `int(bitcast<float>(a.e0))`
inline int cvt_ss2si(const m128& a) noexcept { return _mm_cvt_ss2si(a); }

/// \note `<m128i>` -> `{int(a.i2[0]), int(a.i2[1]), int(a.i2[2]), int(a.i2[3])}`
/// \note `<m256i>` -> `{int(a.i2[0]), int(a.i2[1]), int(a.i2[2]), int(a.i2[3]), int(a.i2[4]), int(a.i2[5]), int(a.i2[6]), int(a.i2[7])}`
template<typename T> constexpr auto cvtepi16_epi32 = [](const m128i& a) noexcept -> T {
  if constexpr (_same<T, m128i>) return _mm_cvtepi16_epi32(a);
  else if constexpr (_same<T, m256i>) return _mm256_cvtepi16_epi32(a);
};

/// \note `<m128i>` -> `{long long(a.i2[0]), long long(a.i2[1])}`
/// \note `<m256i>` -> `{long long(a.i2[0]), long long(a.i2[1]), long long(a.i2[2]), long long(a.i2[3])}`
template<typename T> constexpr auto cvtepi16_epi64 = [](const m128i& a) noexcept -> T {
  if constexpr (_same<T, m128i>) return _mm_cvtepi16_epi64(a);
  else if constexpr (_same<T, m256i>) return _mm256_cvtepi16_epi64(a);
};

/// \note `<m128i>` -> `{long long(a.i4[0]), long long(a.i4[1])}`
/// \note `<m256i>` -> `{long long(a.i4[0]), long long(a.i4[1]), long long(a.i4[2]), long long(a.i4[3])}`
template<typename T> constexpr auto cvtepi32_epi64 = [](const m128i& a) noexcept -> T {
  if constexpr (_same<T, m128i>) return _mm_cvtepi32_epi64(a);
  else if constexpr (_same<T, m256i>) return _mm256_cvtepi32_epi64(a);
};

/// \note `<m128d>` -> `{double(a.i4[0]), double(a.i4[1])}`
/// \note `<m256d>` -> `{double(a.i4[0]), double(a.i4[1]), double(a.i4[2]), double(a.i4[3])}`
template<typename T> constexpr auto cvtepi32_pd = [](const m128i& a) noexcept -> T {
  if constexpr (_same<T, m128d>) return _mm_cvtepi32_pd(a);
  else if constexpr (_same<T, m256d>) return _mm256_cvtepi32_pd(a);
};

/// \return `{float(a.i4[0]), float(a.i4[1]), float(a.i4[2]), float(a.i4[3])}`
inline m128 cvtepi32_ps(const m128i& a) noexcept { return _mm_cvtepi32_ps(a); }

/// \return `{float(a.i4[0]), float(a.i4[1]), float(a.i4[2]), float(a.i4[3]), float(a.i4[4]), float(a.i4[5]), float(a.i4[6]), float(a.i4[7])}`
inline m256 cvtepi32_ps(const m256i& a) noexcept { return _mm256_cvtepi32_ps(a); }

/// \note `<m128i>` -> `{short(a.i1[0]), short(a.i1[1]), short(a.i1[2]), short(a.i1[3]), short(a.i1[4]), short(a.i1[5]), short(a.i1[6]), short(a.i1[7])}`
/// \note `<m256i>` -> `{short(a.i1[0]), short(a.i1[1]), short(a.i1[2]), short(a.i1[3]), short(a.i1[4]), short(a.i1[5]), short(a.i1[6]), short(a.i1[7]), short(a.i1[8]), short(a.i1[9]), short(a.i1[10]), short(a.i1[11]), short(a.i1[12]), short(a.i1[13]), short(a.i1[14]), short(a.i1[15])}`
template<typename T> constexpr auto cvtepi8_epi16 = [](const m128i& a) noexcept -> T {
  if constexpr (_same<T, m128i>) return _mm_cvtepi8_epi16(a);
  else if constexpr (_same<T, m256i>) return _mm256_cvtepi8_epi16(a);
};

/// \note `<m128i>` -> `{int(a.i1[0]), int(a.i1[1]), int(a.i1[2]), int(a.i1[3])}`
/// \note `<m256i>` -> `{int(a.i1[0]), int(a.i1[1]), int(a.i1[2]), int(a.i1[3]), int(a.i1[4]), int(a.i1[5]), int(a.i1[6]), int(a.i1[7])}`
template<typename T> constexpr auto cvtepi8_epi32 = [](const m128i& a) noexcept -> T {
  if constexpr (_same<T, m128i>) return _mm_cvtepi8_epi32(a);
  else if constexpr (_same<T, m256i>) return _mm256_cvtepi8_epi32(a);
};

/// \note `<m128i>` -> `{long long(a.i1[0]), long long(a.i1[1])}`
/// \note `<m256i>` -> `{long long(a.i1[0]), long long(a.i1[1]), long long(a.i1[2]), long long(a.i1[3])}`
template<typename T> constexpr auto cvtepi8_epi64 = [](const m128i& a) noexcept -> T {
  if constexpr (_same<T, m128i>) return _mm_cvtepi8_epi64(a);
  else if constexpr (_same<T, m256i>) return _mm256_cvtepi8_epi64(a);
};

/// \note `<m128i>` -> `{int(a.u2[0]), int(a.u2[1]), int(a.u2[2]), int(a.u2[3])}`
/// \note `<m256i>` -> `{int(a.u2[0]), int(a.u2[1]), int(a.u2[2]), int(a.u2[3]), int(a.u2[4]), int(a.u2[5]), int(a.u2[6]), int(a.u2[7])}`
template<typename T> constexpr auto cvtepu16_epi32 = [](const m128i& a) noexcept -> T {
  if constexpr (_same<T, m128i>) return _mm_cvtepu16_epi32(a);
  else if constexpr (_same<T, m256i>) return _mm256_cvtepu16_epi32(a);
};

/// \note `<m128i>` -> `{long long(a.u2[0]), long long(a.u2[1])}`
/// \note `<m256i>` -> `{long long(a.u2[0]), long long(a.u2[1]), long long(a.u2[2]), long long(a.u2[3])}`
template<typename T> constexpr auto cvtepu16_epi64 = [](const m128i& a) noexcept -> T {
  if constexpr (_same<T, m128i>) return _mm_cvtepu16_epi64(a);
  else if constexpr (_same<T, m256i>) return _mm256_cvtepu16_epi64(a);
};

/// \note `<m128i>` -> `{int(a.u4[0]), int(a.u4[1]), int(a.u4[2]), int(a.u4[3])}`
/// \note `<m256i>` -> `{int(a.u4[0]), int(a.u4[1]), int(a.u4[2]), int(a.u4[3]), int(a.u4[4]), int(a.u4[5]), int(a.u4[6]), int(a.u4[7])}`
template<typename T> constexpr auto cvtepu32_epi64 = [](const m128i& a) noexcept -> T {
  if constexpr (_same<T, m128i>) return _mm_cvtepu32_epi64(a);
  else if constexpr (_same<T, m256i>) return _mm256_cvtepu32_epi64(a);
};

/// \note `<m128i>` -> `{int(a.u1[0]), int(a.u1[1]), int(a.u1[2]), int(a.u1[3]), int(a.u1[4]), int(a.u1[5]), int(a.u1[6]), int(a.u1[7])}`
/// \note `<m256i>` -> `{int(a.u1[0]), int(a.u1[1]), int(a.u1[2]), int(a.u1[3]), int(a.u1[4]), int(a.u1[5]), int(a.u1[6]), int(a.u1[7]), int(a.u1[8]), int(a.u1[9]), int(a.u1[10]), int(a.u1[11]), int(a.u1[12]), int(a.u1[13]), int(a.u1[14]), int(a.u1[15])}`
template<typename T> constexpr auto cvtepu8_epi16 = [](const m128i& a) noexcept -> T {
  if constexpr (_same<T, m128i>) return _mm_cvtepu8_epi16(a);
  else if constexpr (_same<T, m256i>) return _mm256_cvtepu8_epi16(a);
};

/// \note `<m128i>` -> `{int(a.u1[0]), int(a.u1[1]), int(a.u1[2]), int(a.u1[3])}`
/// \note `<m256i>` -> `{int(a.u1[0]), int(a.u1[1]), int(a.u1[2]), int(a.u1[3]), int(a.u1[4]), int(a.u1[5]), int(a.u1[6]), int(a.u1[7])}`
template<typename T> constexpr auto cvtepu8_epi32 = [](const m128i& a) noexcept -> T {
  if constexpr (_same<T, m128i>) return _mm_cvtepu8_epi32(a);
  else if constexpr (_same<T, m256i>) return _mm256_cvtepu8_epi32(a);
};

/// \note `<m128i>` -> `{long long(a.u1[0]), long long(a.u1[1])}`
/// \note `<m256i>` -> `{long long(a.u1[0]), long long(a.u1[1]), long long(a.u1[2]), long long(a.u1[3])}`
template<typename T> constexpr auto cvtepu8_epi64 = [](const m128i& a) noexcept -> T {
  if constexpr (_same<T, m128i>) return _mm_cvtepu8_epi64(a);
  else if constexpr (_same<T, m256i>) return _mm256_cvtepu8_epi64(a);
};

/// \return `{int(a.f8[0]), int(a.f8[1]), 0, 0}`
inline m128i cvtpd_epi32(const m128d& a) noexcept { return _mm_cvtpd_epi32(a); }

/// \return `<m128>` -> `{int(a.f8[0]), int(a.f8[1]), int(a.f8[2]), int(a.f8[3])}`
inline m128i cvtpd_epi32(const m256d& a) noexcept { return _mm256_cvtpd_epi32(a); }

/// \return `{float(a.f8[0]), float(a.f8[1]), 0, 0}`
inline m128 cvtpd_ps(const m128d& a) noexcept { return _mm_cvtpd_ps(a); }

/// \return `{float(a.f8[0]), float(a.f8[1]), float(a.f8[2]), float(a.f8[3])}`
inline m128 cvtpd_ps(const m256d& a) noexcept { return _mm256_cvtpd_ps(a); }

/// \return `{int(a.f4[0]), int(a.f4[1]), int(a.f4[2]), int(a.f4[3])}`
inline m128i cvtps_epi32(const m128& a) noexcept { return _mm_cvtps_epi32(a); }

/// \return `{int(a.f4[0]), int(a.f4[1]), int(a.f4[2]), int(a.f4[3]), int(a.f4[4]), int(a.f4[5]), int(a.f4[6]), int(a.f4[7])}`
inline m256i cvtps_epi32(const m256& a) noexcept { return _mm256_cvtps_epi32(a); }

/// \note `<m128d>` -> `{double(a.f4[0]), double(a.f4[1])}`
/// \note `<m256d>` -> `{double(a.f4[0]), double(a.f4[1]), double(a.f4[2]), double(a.f4[3])}`
template<typename T> inline constexpr auto cvtps_pd = [](const m128& a) noexcept -> T {
  if constexpr (_same<T, m128d>) return _mm_cvtps_pd(a);
  else if constexpr (_same<T, m256d>) return _mm256_cvtps_pd(a);
};

/// \return `double(a.f8[0])`
inline double cvtsd_f64(const m128d& a) noexcept { return _mm_cvtsd_f64(a); }

/// \return `double(a.f8[0])`
inline double cvtsd_f64(const m256d& a) noexcept { return _mm256_cvtsd_f64(a); }

/// \return `{int(a.f8[0]), 0}`
inline int cvtsd_si32(const m128d& a) noexcept { return _mm_cvtsd_si32(a); }

/// \return `{long long(a.f8[0]), 0}`
inline long long cvtsd_si64(const m128d& a) noexcept { return _mm_cvtsd_si64(a); }

/// \return `{float(b.f8[0]), a.f4[1], a.f4[2], a.f4[3]}`
inline m128 cvtsd_ss(const m128& a, const m128d& b) noexcept { return _mm_cvtsd_ss(a, b); }

/// \return `a.i4[0]`
inline int cvtsi128_si32(const m128i& a) noexcept { return _mm_cvtsi128_si32(a); }

/// \return `a.i8[0]`
inline long long cvtsi128_si64(const m128i& a) noexcept { return _mm_cvtsi128_si64(a); }

/// \return `a.i4[0]`
inline int cvtsi256_si32(const m256i& a) noexcept { return _mm256_cvtsi256_si32(a); }

/// \return `{double(b), a.f8[1]}`
inline m128d cvtsi32_sd(const m128d& a, int b) noexcept { return _mm_cvtsi32_sd(a, b); }

/// \return `{a, 0, 0, 0}`
inline m128i cvtsi32_si128(int a) noexcept { return _mm_cvtsi32_si128(a); }

/// \return `{float(b), a.f4[1], a.f4[2], a.f4[3]}`
inline m128 cvtsi32_ss(const m128& a, int b) noexcept { return _mm_cvtsi32_ss(a, b); }

/// \return `{double(b), a.f8[1]}`
inline m128d cvtsi64_sd(const m128d& a, long long b) noexcept { return _mm_cvtsi64_sd(a, b); }

/// \return `{a, long long(0)}`
inline m128i cvtsi64_si128(long long a) noexcept { return _mm_cvtsi64_si128(a); }

/// \return `{float(b), a.f4[1], a.f4[2], a.f4[3]}`
inline m128 cvtsi64_ss(const m128& a, long long b) noexcept { return _mm_cvtsi64_ss(a, b); }

/// \return `a.i4[0]`
inline float cvtss_f32(const m128& a) noexcept { return _mm_cvtss_f32(a); }

/// \return `a.i4[0]`
inline float cvtss_f32(const m256& a) noexcept { return _mm256_cvtss_f32(a); }

/// \return `{double(a.f4[0]), a.f8[1]}`
inline m128d cvtss_sd(const m128d& a, const m128& b) noexcept { return _mm_cvtss_sd(a, b); }

/// \return `int(a.f4[0])`
inline int cvtss_si32(const m128& a) noexcept { return _mm_cvtss_si32(a); }

/// \return `long long(a.f4[0])`
inline long long cvtss_si64(const m128& a) noexcept { return _mm_cvtss_si64(a); }

/// \return `int(a.f4[0])`
inline int cvtt_ss2si(const m128& a) noexcept { return _mm_cvtt_ss2si(a); }

/// \return `{int(a.f8[0]), int(a.f8[1]), 0, 0}`
inline m128i cvttpd_epi32(const m128d& a) noexcept { return _mm_cvttpd_epi32(a); }

/// \return `{int(a.f8[0]), int(a.f8[1]), int(a.f8[2]), int(a.f8[3])}`
inline m128i cvttpd_epi32(const m256d& a) noexcept { return _mm256_cvttpd_epi32(a); }

/// \return `{int(a.f4[0]), int(a.f4[1]), int(a.f4[2]), int(a.f4[3])}`
inline m128i cvttps_epi32(const m128& a) noexcept { return _mm_cvttps_epi32(a); }

/// \return `{int(a.f4[0]), int(a.f4[1]), int(a.f4[2]), int(a.f4[3]), int(a.f4[4]), int(a.f4[5]), int(a.f4[6]), int(a.f4[7])}`
inline m256i cvttps_epi32(const m256& a) noexcept { return _mm256_cvttps_epi32(a); }

/// \return `int(a.f8[0])`
inline int cvttsd_si32(const m128d& a) noexcept { return _mm_cvttsd_si32(a); }

/// \return `long long(a.f8[0])`
inline long long cvttsd_si64(const m128d& a) noexcept { return _mm_cvttsd_si64(a); }

/// \return `int(a.f4[0])`
inline int cvttss_si32(const m128& a) noexcept { return _mm_cvttss_si32(a); }

/// \return `long long(a.f4[0])`
inline long long cvttss_si64(const m128& a) noexcept { return _mm_cvttss_si64(a); }

// CRYPTOGRAPHY ////////////////////////////////////////////////////////////////

inline unsigned int crc32_u16(unsigned int a, unsigned short b) noexcept { return _mm_crc32_u16(a, b); }
inline unsigned int crc32_u32(unsigned int a, unsigned int b) noexcept { return _mm_crc32_u32(a, b); }
inline unsigned long long crc32_u64(unsigned long long a, unsigned long long b) noexcept { return _mm_crc32_u64(a, b); }
inline unsigned int crc32_u8(unsigned int a, unsigned char b) noexcept { return _mm_crc32_u8(a, b); }

// ELEMENTARY MATH FUNCTIONS ///////////////////////////////////////////////////

inline m128d cbrt_pd(const m128d& a) noexcept { return _mm_cbrt_pd(a); }
inline m256d cbrt_pd(const m256d& a) noexcept { return _mm256_cbrt_pd(a); }
inline m128 cbrt_ps(const m128& a) noexcept { return _mm_cbrt_ps(a); }
inline m256 cbrt_ps(const m256& a) noexcept { return _mm256_cbrt_ps(a); }
inline m128 cexp_ps(const m128& a) noexcept { return _mm_cexp_ps(a); }
inline m256 cexp_ps(const m256& a) noexcept { return _mm256_cexp_ps(a); }
inline m128 clog_ps(const m128& a) noexcept { return _mm_clog_ps(a); }
inline m256 clog_ps(const m256& a) noexcept { return _mm256_clog_ps(a); }
inline m128 csqrt_ps(const m128& a) noexcept { return _mm_csqrt_ps(a); }
inline m256 csqrt_ps(const m256& a) noexcept { return _mm256_csqrt_ps(a); }
inline m128d exp_pd(const m128d& a) noexcept { return _mm_exp_pd(a); }
inline m256d exp_pd(const m256d& a) noexcept { return _mm256_exp_pd(a); }
inline m128 exp_ps(const m128& a) noexcept { return _mm_exp_ps(a); }
inline m256 exp_ps(const m256& a) noexcept { return _mm256_exp_ps(a); }
inline m128d exp10_pd(const m128d& a) noexcept { return _mm_exp10_pd(a); }
inline m256d exp10_pd(const m256d& a) noexcept { return _mm256_exp10_pd(a); }
inline m128 exp10_ps(const m128& a) noexcept { return _mm_exp10_ps(a); }
inline m256 exp10_ps(const m256& a) noexcept { return _mm256_exp10_ps(a); }
inline m128d exp2_pd(const m128d& a) noexcept { return _mm_exp2_pd(a); }
inline m256d exp2_pd(const m256d& a) noexcept { return _mm256_exp2_pd(a); }
inline m128 exp2_ps(const m128& a) noexcept { return _mm_exp2_ps(a); }
inline m256 exp2_ps(const m256& a) noexcept { return _mm256_exp2_ps(a); }
inline m128d expm1_pd(const m128d& a) noexcept { return _mm_expm1_pd(a); }
inline m256d expm1_pd(const m256d& a) noexcept { return _mm256_expm1_pd(a); }
inline m128 expm1_ps(const m128& a) noexcept { return _mm_expm1_ps(a); }
inline m256 expm1_ps(const m256& a) noexcept { return _mm256_expm1_ps(a); }
inline m128d invcbrt_pd(const m128d& a) noexcept { return _mm_invcbrt_pd(a); }
inline m256d invcbrt_pd(const m256d& a) noexcept { return _mm256_invcbrt_pd(a); }
inline m128 invcbrt_ps(const m128& a) noexcept { return _mm_invcbrt_ps(a); }
inline m256 invcbrt_ps(const m256& a) noexcept { return _mm256_invcbrt_ps(a); }
inline m128d invsqrt_pd(const m128d& a) noexcept { return _mm_invsqrt_pd(a); }
inline m256d invsqrt_pd(const m256d& a) noexcept { return _mm256_invsqrt_pd(a); }
inline m128 invsqrt_ps(const m128& a) noexcept { return _mm_invsqrt_ps(a); }
inline m256 invsqrt_ps(const m256& a) noexcept { return _mm256_invsqrt_ps(a); }
inline m128d log_pd(const m128d& a) noexcept { return _mm_log_pd(a); }
inline m256d log_pd(const m256d& a) noexcept { return _mm256_log_pd(a); }
inline m128 log_ps(const m128& a) noexcept { return _mm_log_ps(a); }
inline m256 log_ps(const m256& a) noexcept { return _mm256_log_ps(a); }
inline m128d log10_pd(const m128d& a) noexcept { return _mm_log10_pd(a); }
inline m256d log10_pd(const m256d& a) noexcept { return _mm256_log10_pd(a); }
inline m128 log10_ps(const m128& a) noexcept { return _mm_log10_ps(a); }
inline m256 log10_ps(const m256& a) noexcept { return _mm256_log10_ps(a); }
inline m128d log1p_pd(const m128d& a) noexcept { return _mm_log1p_pd(a); }
inline m256d log1p_pd(const m256d& a) noexcept { return _mm256_log1p_pd(a); }
inline m128 log1p_ps(const m128& a) noexcept { return _mm_log1p_ps(a); }
inline m256 log1p_ps(const m256& a) noexcept { return _mm256_log1p_ps(a); }
inline m128d log2_pd(const m128d& a) noexcept { return _mm_log2_pd(a); }
inline m256d log2_pd(const m256d& a) noexcept { return _mm256_log2_pd(a); }
inline m128 log2_ps(const m128& a) noexcept { return _mm_log2_ps(a); }
inline m256 log2_ps(const m256& a) noexcept { return _mm256_log2_ps(a); }
inline m128d logb_pd(const m128d& a) noexcept { return _mm_logb_pd(a); }
inline m256d logb_pd(const m256d& a) noexcept { return _mm256_logb_pd(a); }
inline m128 logb_ps(const m128& a) noexcept { return _mm_logb_ps(a); }
inline m256 logb_ps(const m256& a) noexcept { return _mm256_logb_ps(a); }
inline m128d pow_pd(const m128d& a, const m128d& b) noexcept { return _mm_pow_pd(a, b); }
inline m256d pow_pd(const m256d& a, const m256d& b) noexcept { return _mm256_pow_pd(a, b); }
inline m128 pow_ps(const m128& a, const m128& b) noexcept { return _mm_pow_ps(a, b); }
inline m256 pow_ps(const m256& a, const m256& b) noexcept { return _mm256_pow_ps(a, b); }
inline m128 rcp_ps(const m128& a) noexcept { return _mm_rcp_ps(a); }
inline m256 rcp_ps(const m256& a) noexcept { return _mm256_rcp_ps(a); }
inline m128 rcp_ss(const m128& a) noexcept { return _mm_rcp_ss(a); }
inline m128 rsqrt_ps(const m128& a) noexcept { return _mm_rsqrt_ps(a); }
inline m256 rsqrt_ps(const m256& a) noexcept { return _mm256_rsqrt_ps(a); }
inline m128 rsqrt_ss(const m128& a) noexcept { return _mm_rsqrt_ss(a); }
inline m128d sqrt_pd(const m128d& a) noexcept { return _mm_sqrt_pd(a); }
inline m256d sqrt_pd(const m256d& a) noexcept { return _mm256_sqrt_pd(a); }
inline m128 sqrt_ps(const m128& a) noexcept { return _mm_sqrt_ps(a); }
inline m256 sqrt_ps(const m256& a) noexcept { return _mm256_sqrt_ps(a); }

/// \return `{sqrt(b.f8[0]), a.f8[1]}`
inline m128d sqrt_sd(const m128d& a, const m128d& b) noexcept { return _mm_sqrt_sd(a, b); }

/// \return `{sqrt(a.f4[0]), a.f4[1], a.f4[2], a.f4[3]}`
inline m128 sqrt_ss(const m128& a) noexcept { return _mm_sqrt_ss(a); }

inline m128d svml_sqrt_pd(const m128d& a) noexcept { return _mm_svml_sqrt_pd(a); }
inline m256d svml_sqrt_pd(const m256d& a) noexcept { return _mm256_svml_sqrt_pd(a); }
inline m128 svml_sqrt_ps(const m128& a) noexcept { return _mm_svml_sqrt_ps(a); }
inline m256 svml_sqrt_ps(const m256& a) noexcept { return _mm256_svml_sqrt_ps(a); }

// GENERAL SUPPORT /////////////////////////////////////////////////////////////

// LOGICAL //////////////////////////////////////////////////////////////////////

inline m128d and_pd(const m128d& a, const m128d& b) noexcept { return _mm_and_pd(a, b); }
inline m256d and_pd(const m256d& a, const m256d& b) noexcept { return _mm256_and_pd(a, b); }
inline m128 and_ps(const m128& a, const m128& b) noexcept { return _mm_and_ps(a, b); }
inline m256 and_ps(const m256& a, const m256& b) noexcept { return _mm256_and_ps(a, b); }
inline m128i and_si128(const m128i& a, const m128i& b) noexcept { return _mm_and_si128(a, b); }
inline m256i and_si256(const m256i& a, const m256i& b) noexcept { return _mm256_and_si256(a, b); }
inline m128d andnot_pd(const m128d& a, const m128d& b) noexcept { return _mm_andnot_pd(a, b); }
inline m256d andnot_pd(const m256d& a, const m256d& b) noexcept { return _mm256_andnot_pd(a, b); }
inline m128 andnot_ps(const m128& a, const m128& b) noexcept { return _mm_andnot_ps(a, b); }
inline m256 andnot_ps(const m256& a, const m256& b) noexcept { return _mm256_andnot_ps(a, b); }
inline m128i andnot_si128(const m128i& a, const m128i& b) noexcept { return _mm_andnot_si128(a, b); }
inline m256i andnot_si256(const m256i& a, const m256i& b) noexcept { return _mm256_andnot_si256(a, b); }
inline m128d or_pd(const m128d& a, const m128d& b) noexcept { return _mm_or_pd(a, b); }
inline m256d or_pd(const m256d& a, const m256d& b) noexcept { return _mm256_or_pd(a, b); }
inline m128 or_ps(const m128& a, const m128& b) noexcept { return _mm_or_ps(a, b); }
inline m256 or_ps(const m256& a, const m256& b) noexcept { return _mm256_or_ps(a, b); }
inline m128i or_si128(const m128i& a, const m128i& b) noexcept { return _mm_or_si128(a, b); }
inline m256i or_si256(const m256i& a, const m256i& b) noexcept { return _mm256_or_si256(a, b); }
inline int test_all_ones(const m128i& a) noexcept { return _mm_test_all_ones(a); }

/// \return `(a & mask) == 0 ? 1 : 0`
inline int test_all_zeros(const m128i& mask, const m128i& a) noexcept { return _mm_test_all_zeros(mask, a); }
inline int test_mix_ones_zeros(const m128i& a, const m128i& b) noexcept { return _mm_test_mix_ones_zeros(a, b); }
inline int testc_pd(const m128d& a, const m128d& b) noexcept { return _mm_testc_pd(a, b); }
inline int testc_pd(const m256d& a, const m256d& b) noexcept { return _mm256_testc_pd(a, b); }
inline int testc_ps(const m128& a, const m128& b) noexcept { return _mm_testc_ps(a, b); }
inline int testc_ps(const m256& a, const m256& b) noexcept { return _mm256_testc_ps(a, b); }
inline int testc_si128(const m128i& a, const m128i& b) noexcept { return _mm_testc_si128(a, b); }
inline int testc_si256(const m256i& a, const m256i& b) noexcept { return _mm256_testc_si256(a, b); }
inline int testnzc_pd(const m128d& a, const m128d& b) noexcept { return _mm_testnzc_pd(a, b); }
inline int testnzc_pd(const m256d& a, const m256d& b) noexcept { return _mm256_testnzc_pd(a, b); }
inline int testnzc_ps(const m128& a, const m128& b) noexcept { return _mm_testnzc_ps(a, b); }
inline int testnzc_ps(const m256& a, const m256& b) noexcept { return _mm256_testnzc_ps(a, b); }
inline int testnzc_si128(const m128i& a, const m128i& b) noexcept { return _mm_testnzc_si128(a, b); }
inline int testnzc_si256(const m256i& a, const m256i& b) noexcept { return _mm256_testnzc_si256(a, b); }
inline int testz_pd(const m128d& a, const m128d& b) noexcept { return _mm_testz_pd(a, b); }
inline int testz_pd(const m256d& a, const m256d& b) noexcept { return _mm256_testz_pd(a, b); }
inline int testz_ps(const m128& a, const m128& b) noexcept { return _mm_testz_ps(a, b); }
inline int testz_ps(const m256& a, const m256& b) noexcept { return _mm256_testz_ps(a, b); }
inline int testz_si128(const m128i& a, const m128i& b) noexcept { return _mm_testz_si128(a, b); }
inline int testz_si256(const m256i& a, const m256i& b) noexcept { return _mm256_testz_si256(a, b); }
inline m128d xor_pd(const m128d& a, const m128d& b) noexcept { return _mm_xor_pd(a, b); }
inline m256d xor_pd(const m256d& a, const m256d& b) noexcept { return _mm256_xor_pd(a, b); }
inline m128 xor_ps(const m128& a, const m128& b) noexcept { return _mm_xor_ps(a, b); }
inline m256 xor_ps(const m256& a, const m256& b) noexcept { return _mm256_xor_ps(a, b); }
inline m128i xor_si128(const m128i& a, const m128i& b) noexcept { return _mm_xor_si128(a, b); }
inline m256i xor_si256(const m256i& a, const m256i& b) noexcept { return _mm256_xor_si256(a, b); }

// MASK ////////////////////////////////////////////////////////////////////////

// MISCELLANEOUS ///////////////////////////////////////////////////////////////

template<int imm8> m128i alignr_epi8(const m128i& a, const m128i& b) noexcept { return _mm_alignr_epi8(a, b, imm8); }
template<int imm8> m256i alignr_epi8(const m256i& a, const m256i& b) noexcept { return _mm256_alignr_epi8(a, b, imm8); }
inline m128i minpos_epu16(const m128i& a) noexcept { return _mm_minpos_epu16(a); }
inline int movemask_epi8(const m128i& a) noexcept { return _mm_movemask_epi8(a); }
inline int movemask_epi8(const m256i& a) noexcept { return _mm256_movemask_epi8(a); }
inline int movemask_pd(const m128d& a) noexcept { return _mm_movemask_pd(a); }
inline int movemask_pd(const m256d& a) noexcept { return _mm256_movemask_pd(a); }
inline int movemask_ps(const m128& a) noexcept { return _mm_movemask_ps(a); }
inline int movemask_ps(const m256& a) noexcept { return _mm256_movemask_ps(a); }
template<int imm8> m128i mpsadbw_epu8(const m128i& a, const m128i& b) noexcept { return _mm_mpsadbw_epu8(a, b, imm8); }
template<int imm8> m256i mpsadbw_epu8(const m256i& a, const m256i& b) noexcept { return _mm256_mpsadbw_epu8(a, b, imm8); }
inline m128i packs_epi16(const m128i& a, const m128i& b) noexcept { return _mm_packs_epi16(a, b); }
inline m256i packs_epi16(const m256i& a, const m256i& b) noexcept { return _mm256_packs_epi16(a, b); }
inline m128i packs_epi32(const m128i& a, const m128i& b) noexcept { return _mm_packs_epi32(a, b); }
inline m256i packs_epi32(const m256i& a, const m256i& b) noexcept { return _mm256_packs_epi32(a, b); }
inline m128i packus_epi16(const m128i& a, const m128i& b) noexcept { return _mm_packus_epi16(a, b); }
inline m256i packus_epi16(const m256i& a, const m256i& b) noexcept { return _mm256_packus_epi16(a, b); }
inline m128i packus_epi32(const m128i& a, const m128i& b) noexcept { return _mm_packus_epi32(a, b); }
inline m256i packus_epi32(const m256i& a, const m256i& b) noexcept { return _mm256_packus_epi32(a, b); }
inline m128d trunc_pd(const m128d& a) noexcept { return _mm_trunc_pd(a); }
inline m256d trunc_pd(const m256d& a) noexcept { return _mm256_trunc_pd(a); }
inline m128 trunc_ps(const m128& a) noexcept { return _mm_trunc_ps(a); }
inline m256 trunc_ps(const m256& a) noexcept { return _mm256_trunc_ps(a); }

// MOVE ////////////////////////////////////////////////////////////////////////

inline m128i move_epi64(const m128i& a) noexcept { return _mm_move_epi64(a); }
inline m128d move_sd(const m128d& a, const m128d& b) noexcept { return _mm_move_sd(a, b); }
inline m128 move_ss(const m128& a, const m128& b) noexcept { return _mm_move_ss(a, b); }
inline m128d movedup_pd(const m128d& a) noexcept { return _mm_movedup_pd(a); }
inline m256d movedup_pd(const m256d& a) noexcept { return _mm256_movedup_pd(a); }
inline m128 movehdup_ps(const m128& a) noexcept { return _mm_movehdup_ps(a); }
inline m256 movehdup_ps(const m256& a) noexcept { return _mm256_movehdup_ps(a); }
inline m128 movehl_ps(const m128& a, const m128& b) noexcept { return _mm_movehl_ps(a, b); }
inline m128 moveldup_ps(const m128& a) noexcept { return _mm_moveldup_ps(a); }
inline m256 moveldup_ps(const m256& a) noexcept { return _mm256_moveldup_ps(a); }
inline m128 movelh_ps(const m128& a, const m128& b) noexcept { return _mm_movelh_ps(a, b); }

// OS-TARGETED /////////////////////////////////////////////////////////////////

// PROBABILITY / STATISTICS ////////////////////////////////////////////////////

inline m128i avg_epu16(const m128i& a, const m128i& b) noexcept { return _mm_avg_epu16(a, b); }
inline m256i avg_epu16(const m256i& a, const m256i& b) noexcept { return _mm256_avg_epu16(a, b); }
inline m128i avg_epu8(const m128i& a, const m128i& b) noexcept { return _mm_avg_epu8(a, b); }
inline m256i avg_epu8(const m256i& a, const m256i& b) noexcept { return _mm256_avg_epu8(a, b); }
inline m128d cdfnorm_pd(const m128d& a) noexcept { return _mm_cdfnorm_pd(a); }
inline m256d cdfnorm_pd(const m256d& a) noexcept { return _mm256_cdfnorm_pd(a); }
inline m128 cdfnorm_ps(const m128& a) noexcept { return _mm_cdfnorm_ps(a); }
inline m256 cdfnorm_ps(const m256& a) noexcept { return _mm256_cdfnorm_ps(a); }
inline m128d cdfnorminv_pd(const m128d& a) noexcept { return _mm_cdfnorminv_pd(a); }
inline m256d cdfnorminv_pd(const m256d& a) noexcept { return _mm256_cdfnorminv_pd(a); }
inline m128 cdfnorminv_ps(const m128& a) noexcept { return _mm_cdfnorminv_ps(a); }
inline m256 cdfnorminv_ps(const m256& a) noexcept { return _mm256_cdfnorminv_ps(a); }
inline m256d erf_pd(const m256d& a) noexcept { return _mm256_erf_pd(a); }
inline m128 erf_ps(const m128& a) noexcept { return _mm_erf_ps(a); }
inline m256 erf_ps(const m256& a) noexcept { return _mm256_erf_ps(a); }
inline m128d erfc_pd(const m128d& a) noexcept { return _mm_erfc_pd(a); }
inline m256d erfc_pd(const m256d& a) noexcept { return _mm256_erfc_pd(a); }
inline m128 erfc_ps(const m128& a) noexcept { return _mm_erfc_ps(a); }
inline m256 erfc_ps(const m256& a) noexcept { return _mm256_erfc_ps(a); }
inline m128d erfcinv_pd(const m128d& a) noexcept { return _mm_erfcinv_pd(a); }
inline m256d erfcinv_pd(const m256d& a) noexcept { return _mm256_erfcinv_pd(a); }
inline m128 erfcinv_ps(const m128& a) noexcept { return _mm_erfcinv_ps(a); }
inline m256 erfcinv_ps(const m256& a) noexcept { return _mm256_erfcinv_ps(a); }
inline m128d erfinv_pd(const m128d& a) noexcept { return _mm_erfinv_pd(a); }
inline m256d erfinv_pd(const m256d& a) noexcept { return _mm256_erfinv_pd(a); }
inline m128 erfinv_ps(const m128& a) noexcept { return _mm_erfinv_ps(a); }
inline m256 erfinv_ps(const m256& a) noexcept { return _mm256_erfinv_ps(a); }

// RANDOM //////////////////////////////////////////////////////////////////////

// SET /////////////////////////////////////////////////////////////////////////

// SET /////////////////////////////////////////////////////////////////////////

/// \return `{e0, e1, e2, e3, e4, e5, e6, e7}`
inline m128i set_epi16(
  short e7, short e6, short e5, short e4,
  short e3, short e2, short e1, short e0) noexcept {
  return _mm_set_epi16(e7, e6, e5, e4, e3, e2, e1, e0);
}

/// \return `{e0, e1, e2, e3, e4, e5, e6, e7, e8, e9, e10, e11, e12, e13, e14, e15}`
inline m256i set_epi16(
  short e15, short e14, short e13, short e12,
  short e11, short e10, short e9, short e8,
  short e7, short e6, short e5, short e4,
  short e3, short e2, short e1, short e0) noexcept {
  return _mm256_set_epi16(
    e15, e14, e13, e12, e11, e10, e9, e8,
    e7, e6, e5, e4, e3, e2, e1, e0);
}

/// \return `{e0, e1, e2, e3}`
inline m128i set_epi32(int e3, int e2, int e1, int e0) noexcept {
  return _mm_set_epi32(e3, e2, e1, e0);
}

/// \return `{e0, e1, e2, e3, e4, e5, e6, e7}`
inline m256i set_epi32(
  int e7, int e6, int e5, int e4, int e3, int e2, int e1, int e0) noexcept {
  return _mm256_set_epi32(e7, e6, e5, e4, e3, e2, e1, e0);
}

/// \return `{e0, e1}`
inline m128i set_epi64x(long long e1, long long e0) noexcept {
  return _mm_set_epi64x(e1, e0);
}

/// \return `{e0, e1, e2, e3}`
inline m256i set_epi64x(
  long long e3, long long e2, long long e1, long long e0) noexcept {
  return _mm256_set_epi64x(e3, e2, e1, e0);
}

/// \return `{e0, e1, e2, e3, e4, e5, e6, e7, e8, e9, e10, e11, e12, e13, e14, e15}`
inline m128i set_epi8(
  char e15, char e14, char e13, char e12,
  char e11, char e10, char e9, char e8,
  char e7, char e6, char e5, char e4,
  char e3, char e2, char e1, char e0) noexcept {
  return _mm_set_epi8(
    e15, e14, e13, e12, e11, e10, e9, e8,
    e7, e6, e5, e4, e3, e2, e1, e0);
}

/// \return `{e0, e1, e2, e3, e4, e5, e6, e7, e8, e9, e10, e11, e12, e13, e14, e15, e16, e17, e18, e19, e20, e21, e22, e23, e24, e25, e26, e27, e28, e29, e30, e31}`
inline m256i set_epi8(
  char e31, char e30, char e29, char e28,
  char e27, char e26, char e25, char e24,
  char e23, char e22, char e21, char e20,
  char e19, char e18, char e17, char e16,
  char e15, char e14, char e13, char e12,
  char e11, char e10, char e9, char e8,
  char e7, char e6, char e5, char e4,
  char e3, char e2, char e1, char e0) noexcept {
  return _mm256_set_epi8(
    e31, e30, e29, e28, e27, e26, e25, e24,
    e23, e22, e21, e20, e19, e18, e17, e16,
    e15, e14, e13, e12, e11, e10, e9, e8,
    e7, e6, e5, e4, e3, e2, e1, e0);
}

inline m256 set_m128(const m128& hi, const m128& lo) noexcept { return _mm256_set_m128(hi, lo); }
inline m256d set_m128d(const m128d& hi, const m128d& lo) noexcept { return _mm256_set_m128d(hi, lo); }
inline m256i set_m128i(const m128i& hi, const m128i& lo) noexcept { return _mm256_set_m128i(hi, lo); }
inline m128d set_pd(double e1, double e0) noexcept { return _mm_set_pd(e1, e0); }
inline m256d set_pd(double e3, double e2, double e1, double e0) noexcept { return _mm256_set_pd(e3, e2, e1, e0); }
inline m128d set_pd1(double a) noexcept { return _mm_set_pd1(a); }

/// \return `{e0, e1, e2, e3}`
inline m128 set_ps(float e3, float e2, float e1, float e0) noexcept {
  return _mm_set_ps(e3, e2, e1, e0);
}

/// \return `{e0, e1, e2, e3, e4, e5, e6, e7}`
inline m256 set_ps(
  float e7, float e6, float e5, float e4,
  float e3, float e2, float e1, float e0) noexcept {
  return _mm256_set_ps(e7, e6, e5, e4, e3, e2, e1, e0);
}

/// \return `{a, a, a, a}`
inline m128 set_ps1(float a) noexcept { return _mm_set_ps1(a); }

/// \return `{a, a}`
inline m128d set_sd(double a) noexcept { return _mm_set_sd(a); }

/// \return `{a, 0, 0, 0}`
inline m128 set_ss(float a) noexcept { return _mm_set_ss(a); }

/// \note `<m128i>` -> `{a, a, a, a, a, a, a, a}`
/// \note `<m256i>` -> `{a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a}`
template<typename T> inline constexpr auto set1_epi16 = [](short a) noexcept -> T {
  if constexpr (_same<T, m128i>) return _mm_set1_epi16(a);
  else if constexpr (_same<T, m256i>) return _mm256_set1_epi16(a);
};

/// \note `<m128i>` -> `{a, a, a, a}`
/// \note `<m256i>` -> `{a, a, a, a, a, a, a, a}`
template<typename T> inline constexpr auto set1_epi32 = [](int a) noexcept -> T {
  if constexpr (_same<T, m128i>) return _mm_set1_epi32(a);
  else if constexpr (_same<T, m256i>) return _mm256_set1_epi32(a);
};

/// \note `<m128i>` -> `{a, a}`
/// \note `<m256i>` -> `{a, a, a, a}`
template<typename T> inline constexpr auto set1_epi64x = [](long long a) noexcept -> T {
  if constexpr (_same<T, m128i>) return _mm_set1_epi64x(a);
  else if constexpr (_same<T, m256i>) return _mm256_set1_epi64x(a);
};

/// \note `<m128i>` -> `{a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a}`
/// \note `<m256i>` -> `{a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a}`
template<typename T> inline constexpr auto set1_epi8 = [](char a) noexcept -> T {
  if constexpr (_same<T, m128i>) return _mm_set1_epi8(a);
  else if constexpr (_same<T, m256i>) return _mm256_set1_epi8(a);
};

/// \note `<m128d>` -> `{a, a}`
/// \note `<m256d>` -> `{a, a, a, a}`
template<typename T> inline constexpr auto set1_pd = [](double a) noexcept -> T {
  if constexpr (_same<T, m128d>) return _mm_set1_pd(a);
  else if constexpr (_same<T, m256d>) return _mm256_set1_pd(a);
};

/// \note `<m128>` -> `{a, a, a, a}`
/// \note `<m256>` -> `{a, a, a, a, a, a, a, a}`
template<typename T> inline constexpr auto set1_ps = [](float a) noexcept -> T {
  if constexpr (_same<T, m128>) return _mm_set1_ps(a);
  else if constexpr (_same<T, m256>) return _mm256_set1_ps(a);
};

/// \return `{e0, e1, e2, e3, e4, e5, e6, e7}`
inline m128i setr_epi16(
  short e0, short e1, short e2, short e3,
  short e4, short e5, short e6, short e7) noexcept {
  return _mm_setr_epi16(e0, e1, e2, e3, e4, e5, e6, e7);
}

/// \return `{e0, e1, e2, e3, e4, e5, e6, e7, e8, e9, e10, e11, e12, e13, e14, e15}`
inline m256i setr_epi16(
  short e0, short e1, short e2, short e3,
  short e4, short e5, short e6, short e7,
  short e8, short e9, short e10, short e11,
  short e12, short e13, short e14, short e15) noexcept {
  return _mm256_setr_epi16(
    e0, e1, e2, e3, e4, e5, e6, e7,
    e8, e9, e10, e11, e12, e13, e14, e15);
}

/// \return `{e0, e1, e2, e3}`
inline m128i setr_epi32(int e0, int e1, int e2, int e3) noexcept {
  return _mm_setr_epi32(e0, e1, e2, e3);
}

/// \return `{e0, e1, e2, e3, e4, e5, e6, e7}`
inline m256i setr_epi32(
  int e0, int e1, int e2, int e3,
  int e4, int e5, int e6, int e7) noexcept {
  return _mm256_setr_epi32(e0, e1, e2, e3, e4, e5, e6, e7);
}

/// \return `{e0, e1, e2, e3}`
inline m256i setr_epi64(
  long long e0, long long e1, long long e2, long long e3) noexcept {
  return _mm256_setr_epi64x(e0, e1, e2, e3);
}

/// \return `{e0, e1, e2, e3, e4, e5, e6, e7, e8, e9, e10, e11, e12, e13, e14, e15}`
inline m128i setr_epi8(
  char e0, char e1, char e2, char e3,
  char e4, char e5, char e6, char e7,
  char e8, char e9, char e10, char e11,
  char e12, char e13, char e14, char e15) noexcept {
  return _mm_setr_epi8(
    e0, e1, e2, e3, e4, e5, e6, e7,
    e8, e9, e10, e11, e12, e13, e14, e15);
}

/// \return `{e0, e1, e2, e3, e4, e5, e6, e7, e8, e9, e10, e11, e12, e13, e14, e15, e16, e17, e18, e19, e20, e21, e22, e23, e24, e25, e26, e27, e28, e29, e30, e31}`
inline m256i setr_epi8(
  char e0, char e1, char e2, char e3, char e4, char e5, char e6, char e7,
  char e8, char e9, char e10, char e11, char e12, char e13, char e14, char e15,
  char e16, char e17, char e18, char e19, char e20, char e21, char e22, char e23,
  char e24, char e25, char e26, char e27, char e28, char e29, char e30, char e31) noexcept {
  return _mm256_setr_epi8(
    e0, e1, e2, e3, e4, e5, e6, e7, e8, e9, e10, e11, e12, e13, e14, e15, e16,
    e17, e18, e19, e20, e21, e22, e23, e24, e25, e26, e27, e28, e29, e30, e31);
}

inline m256 setr_m128(const m128& lo, const m128& hi) noexcept { return _mm256_setr_m128(lo, hi); }
inline m256d setr_m128d(const m128d& lo, const m128d& hi) noexcept { return _mm256_setr_m128d(lo, hi); }
inline m256i setr_m128i(const m128i& lo, const m128i& hi) noexcept { return _mm256_setr_m128i(lo, hi); }
inline m128d setr_pd(double e0, double e1) noexcept { return _mm_setr_pd(e0, e1); }
inline m256d setr_pd(double e0, double e1, double e2, double e3) noexcept { return _mm256_setr_pd(e0, e1, e2, e3); }
inline m128 setr_ps(float e0, float e1, float e2, float e3) noexcept { return _mm_setr_ps(e0, e1, e2, e3); }
inline m256 setr_ps(float e0, float e1, float e2, float e3, float e4, float e5, float e6, float e7) noexcept { return _mm256_setr_ps(e0, e1, e2, e3, e4, e5, e6, e7); }

/// \return zero vector
template<typename T> inline constexpr auto setzero = []() noexcept -> T {
  if constexpr (_same<T, m128d>) return _mm_setzero_pd();
  else if constexpr (_same<T, m256d>) return _mm256_setzero_pd();
  else if constexpr (_same<T, m128>) return _mm_setzero_ps();
  else if constexpr (_same<T, m256>) return _mm256_setzero_ps();
  else if constexpr (_same<T, m128i>) return _mm_setzero_si128();
  else if constexpr (_same<T, m256i>) return _mm256_setzero_si256();
};

// SHIFT ///////////////////////////////////////////////////////////////////////

template<int imm8> m256i bslli_epi128(const m256i& a) noexcept { return _mm256_bslli_epi128(a, imm8); }
template<int imm8> m128i bslli_si128(const m128i& a) noexcept { return _mm_bslli_si128(a, imm8); }
template<int imm8> m256i bsrli_epi128(const m256i& a) noexcept { return _mm256_bsrli_epi128(a, imm8); }
template<int imm8> m128i bsrli_si128(const m128i& a) noexcept { return _mm_bsrli_si128(a, imm8); }
inline m128i sll_epi16(const m128i& a, const m128i& count) noexcept { return _mm_sll_epi16(a, count); }
inline m256i sll_epi16(const m256i& a, const m128i& count) noexcept { return _mm256_sll_epi16(a, count); }
inline m128i sll_epi32(const m128i& a, const m128i& count) noexcept { return _mm_sll_epi32(a, count); }
inline m256i sll_epi32(const m256i& a, const m128i& count) noexcept { return _mm256_sll_epi32(a, count); }
inline m128i sll_epi64(const m128i& a, const m128i& count) noexcept { return _mm_sll_epi64(a, count); }
inline m256i sll_epi64(const m256i& a, const m128i& count) noexcept { return _mm256_sll_epi64(a, count); }
template<int imm8> m128i slli_epi16(const m128i& a) noexcept { return _mm_slli_epi16(a, imm8); }
template<int imm8> m256i slli_epi16(const m256i& a) noexcept { return _mm256_slli_epi16(a, imm8); }
template<int imm8> m128i slli_epi32(const m128i& a) noexcept { return _mm_slli_epi32(a, imm8); }
template<int imm8> m256i slli_epi32(const m256i& a) noexcept { return _mm256_slli_epi32(a, imm8); }
template<int imm8> m128i slli_epi64(const m128i& a) noexcept { return _mm_slli_epi64(a, imm8); }
template<int imm8> m256i slli_epi64(const m256i& a) noexcept { return _mm256_slli_epi64(a, imm8); }
template<int imm8> m128i slli_si128(const m128i& a) noexcept { return _mm_slli_si128(a, imm8); }
template<int imm8> m256i slli_si256(const m256i& a) noexcept { return _mm256_slli_si256(a, imm8); }
inline m128i sllv_epi32(const m128i& a, const m128i& count) noexcept { return _mm_sllv_epi32(a, count); }
inline m256i sllv_epi32(const m256i& a, const m256i& count) noexcept { return _mm256_sllv_epi32(a, count); }
inline m128i sllv_epi64(const m128i& a, const m128i& count) noexcept { return _mm_sllv_epi64(a, count); }
inline m256i sllv_epi64(const m256i& a, const m256i& count) noexcept { return _mm256_sllv_epi64(a, count); }
inline m128i sra_epi16(const m128i& a, const m128i& count) noexcept { return _mm_sra_epi16(a, count); }
inline m256i sra_epi16(const m256i& a, const m128i& count) noexcept { return _mm256_sra_epi16(a, count); }
inline m128i sra_epi32(const m128i& a, const m128i& count) noexcept { return _mm_sra_epi32(a, count); }
inline m256i sra_epi32(const m256i& a, const m128i& count) noexcept { return _mm256_sra_epi32(a, count); }
template<int imm8> m128i srai_epi16(const m128i& a) noexcept { return _mm_srai_epi16(a, imm8); }
template<int imm8> m256i srai_epi16(const m256i& a) noexcept { return _mm256_srai_epi16(a, imm8); }
template<int imm8> m128i srai_epi32(const m128i& a) noexcept { return _mm_srai_epi32(a, imm8); }
template<int imm8> m256i srai_epi32(const m256i& a) noexcept { return _mm256_srai_epi32(a, imm8); }
inline m128i srav_epi32(const m128i& a, const m128i& count) noexcept { return _mm_srav_epi32(a, count); }
inline m256i srav_epi32(const m256i& a, const m256i& count) noexcept { return _mm256_srav_epi32(a, count); }
inline m128i srl_epi16(const m128i& a, const m128i& count) noexcept { return _mm_srl_epi16(a, count); }
inline m256i srl_epi16(const m256i& a, const m128i& count) noexcept { return _mm256_srl_epi16(a, count); }
inline m128i srl_epi32(const m128i& a, const m128i& count) noexcept { return _mm_srl_epi32(a, count); }
inline m256i srl_epi32(const m256i& a, const m128i& count) noexcept { return _mm256_srl_epi32(a, count); }
inline m128i srl_epi64(const m128i& a, const m128i& count) noexcept { return _mm_srl_epi64(a, count); }
inline m256i srl_epi64(const m256i& a, const m128i& count) noexcept { return _mm256_srl_epi64(a, count); }
template<int imm8> m128i srli_epi16(const m128i& a) noexcept { return _mm_srli_epi16(a, imm8); }
template<int imm8> m256i srli_epi16(const m256i& a) noexcept { return _mm256_srli_epi16(a, imm8); }
template<int imm8> m128i srli_epi32(const m128i& a) noexcept { return _mm_srli_epi32(a, imm8); }
template<int imm8> m256i srli_epi32(const m256i& a) noexcept { return _mm256_srli_epi32(a, imm8); }
template<int imm8> m128i srli_epi64(const m128i& a) noexcept { return _mm_srli_epi64(a, imm8); }
template<int imm8> m256i srli_epi64(const m256i& a) noexcept { return _mm256_srli_epi64(a, imm8); }
template<int imm8> m128i srli_si128(const m128i& a) noexcept { return _mm_srli_si128(a, imm8); }
template<int imm8> m256i srli_si256(const m256i& a) noexcept { return _mm256_srli_si256(a, imm8); }
inline m128i srlv_epi32(const m128i& a, const m128i& count) noexcept { return _mm_srlv_epi32(a, count); }
inline m256i srlv_epi32(const m256i& a, const m256i& count) noexcept { return _mm256_srlv_epi32(a, count); }
inline m128i srlv_epi64(const m128i& a, const m128i& count) noexcept { return _mm_srlv_epi64(a, count); }
inline m256i srlv_epi64(const m256i& a, const m256i& count) noexcept { return _mm256_srlv_epi64(a, count); }

// SPECIAL MATH FUNCTIONS //////////////////////////////////////////////////////

inline m128i abs_epi16(const m128i& a) noexcept { return _mm_abs_epi16(a); }
inline m256i abs_epi16(const m256i& a) noexcept { return _mm256_abs_epi16(a); }
inline m128i abs_epi32(const m128i& a) noexcept { return _mm_abs_epi32(a); }
inline m256i abs_epi32(const m256i& a) noexcept { return _mm256_abs_epi32(a); }
inline m128i abs_epi8(const m128i& a) noexcept { return _mm_abs_epi8(a); }
inline m256i abs_epi8(const m256i& a) noexcept { return _mm256_abs_epi8(a); }
inline m128d ceil_pd(const m128d& a) noexcept { return _mm_ceil_pd(a); }
inline m256d ceil_pd(const m256d& a) noexcept { return _mm256_ceil_pd(a); }
inline m128 ceil_ps(const m128& a) noexcept { return _mm_ceil_ps(a); }
inline m256 ceil_ps(const m256& a) noexcept { return _mm256_ceil_ps(a); }

/// \return `{ceil(b[0]), a[1]}`
inline m128d ceil_sd(const m128d& a, const m128d b) noexcept { return _mm_ceil_sd(a, b); }

/// \return `{ceil(b[0]), a[1], a[2], a[3]}`
inline m128 ceil_ss(const m128& a, const m128& b) noexcept { return _mm_ceil_ss(a, b); }
inline m128d floor_pd(const m128d& a) noexcept { return _mm_floor_pd(a); }
inline m256d floor_pd(const m256d& a) noexcept { return _mm256_floor_pd(a); }
inline m128 floor_ps(const m128& a) noexcept { return _mm_floor_ps(a); }
inline m256 floor_ps(const m256& a) noexcept { return _mm256_floor_ps(a); }

/// \return `{floor(b[0]), a[1]}`
inline m128d floor_sd(const m128d& a, const m128d& b) noexcept { return _mm_floor_sd(a, b); }

/// \return `{floor(b[0]), a[1], a[2], a[3]}`
inline m128 floor_ss(const m128& a, const m128& b) noexcept { return _mm_floor_ss(a, b); }

inline m128i max_epi16(const m128i& a, const m128i& b) noexcept { return _mm_max_epi16(a, b); }
inline m256i max_epi16(const m256i& a, const m256i& b) noexcept { return _mm256_max_epi16(a, b); }
inline m128i max_epi32(const m128i& a, const m128i& b) noexcept { return _mm_max_epi32(a, b); }
inline m256i max_epi32(const m256i& a, const m256i& b) noexcept { return _mm256_max_epi32(a, b); }
inline m128i max_epi8(const m128i& a, const m128i& b) noexcept { return _mm_max_epi8(a, b); }
inline m256i max_epi8(const m256i& a, const m256i& b) noexcept { return _mm256_max_epi8(a, b); }
inline m128i max_epu16(const m128i& a, const m128i& b) noexcept { return _mm_max_epu16(a, b); }
inline m256i max_epu16(const m256i& a, const m256i& b) noexcept { return _mm256_max_epu16(a, b); }
inline m128i max_epu32(const m128i& a, const m128i& b) noexcept { return _mm_max_epu32(a, b); }
inline m256i max_epu32(const m256i& a, const m256i& b) noexcept { return _mm256_max_epu32(a, b); }
inline m128i max_epu8(const m128i& a, const m128i& b) noexcept { return _mm_max_epu8(a, b); }
inline m256i max_epu8(const m256i& a, const m256i& b) noexcept { return _mm256_max_epu8(a, b); }
inline m128d max_pd(const m128d& a, const m128d& b) noexcept { return _mm_max_pd(a, b); }
inline m256d max_pd(const m256d& a, const m256d& b) noexcept { return _mm256_max_pd(a, b); }
inline m128 max_ps(const m128& a, const m128& b) noexcept { return _mm_max_ps(a, b); }
inline m256 max_ps(const m256& a, const m256& b) noexcept { return _mm256_max_ps(a, b); }
inline m128d max_sd(const m128d& a, const m128d& b) noexcept { return _mm_max_sd(a, b); }
inline m128 max_ss(const m128& a, const m128& b) noexcept { return _mm_max_ss(a, b); }
inline m128i min_epi16(const m128i& a, const m128i& b) noexcept { return _mm_min_epi16(a, b); }
inline m256i min_epi16(const m256i& a, const m256i& b) noexcept { return _mm256_min_epi16(a, b); }
inline m128i min_epi32(const m128i& a, const m128i& b) noexcept { return _mm_min_epi32(a, b); }
inline m256i min_epi32(const m256i& a, const m256i& b) noexcept { return _mm256_min_epi32(a, b); }
inline m128i min_epi8(const m128i& a, const m128i& b) noexcept { return _mm_min_epi8(a, b); }
inline m256i min_epi8(const m256i& a, const m256i& b) noexcept { return _mm256_min_epi8(a, b); }
inline m128i min_epu16(const m128i& a, const m128i& b) noexcept { return _mm_min_epu16(a, b); }
inline m256i min_epu16(const m256i& a, const m256i& b) noexcept { return _mm256_min_epu16(a, b); }
inline m128i min_epu32(const m128i& a, const m128i& b) noexcept { return _mm_min_epu32(a, b); }
inline m256i min_epu32(const m256i& a, const m256i& b) noexcept { return _mm256_min_epu32(a, b); }
inline m128i min_epu8(const m128i& a, const m128i& b) noexcept { return _mm_min_epu8(a, b); }
inline m256i min_epu8(const m256i& a, const m256i& b) noexcept { return _mm256_min_epu8(a, b); }
inline m128d min_pd(const m128d& a, const m128d& b) noexcept { return _mm_min_pd(a, b); }
inline m256d min_pd(const m256d& a, const m256d& b) noexcept { return _mm256_min_pd(a, b); }
inline m128 min_ps(const m128& a, const m128& b) noexcept { return _mm_min_ps(a, b); }
inline m256 min_ps(const m256& a, const m256& b) noexcept { return _mm256_min_ps(a, b); }
inline m128d min_sd(const m128d& a, const m128d& b) noexcept { return _mm_min_sd(a, b); }
inline m128 min_ss(const m128& a, const m128& b) noexcept { return _mm_min_ss(a, b); }
template<int rounding> inline m128d round_pd(const m128d& a) noexcept { return _mm_round_pd(a, rounding); }
template<int rounding> inline m256d round_pd(const m256d& a) noexcept { return _mm256_round_pd(a, rounding); }
template<int rounding> inline m128 round_ps(const m128& a) noexcept { return _mm_round_ps(a, rounding); }
template<int rounding> inline m256 round_ps(const m256& a) noexcept { return _mm256_round_ps(a, rounding); }
template<int rounding> inline m128d round_sd(const m128d& a) noexcept { return _mm_round_sd(a, rounding); }
template<int rounding> inline m128 round_ss(const m128& a) noexcept { return _mm_round_ss(a, rounding); }
inline m128d svml_ceil_pd(const m128d& a) noexcept { return _mm_svml_ceil_pd(a); }
inline m256d svml_ceil_pd(const m256d& a) noexcept { return _mm256_svml_ceil_pd(a); }
inline m128 svml_ceil_ps(const m128& a) noexcept { return _mm_svml_ceil_ps(a); }
inline m256 svml_ceil_ps(const m256& a) noexcept { return _mm256_svml_ceil_ps(a); }
inline m128d svml_floor_pd(const m128d& a) noexcept { return _mm_svml_floor_pd(a); }
inline m256d svml_floor_pd(const m256d& a) noexcept { return _mm256_svml_floor_pd(a); }
inline m128 svml_floor_ps(const m128& a) noexcept { return _mm_svml_floor_ps(a); }
inline m256 svml_floor_ps(const m256& a) noexcept { return _mm256_svml_floor_ps(a); }
inline m128d svml_round_pd(const m128d& a) noexcept { return _mm_svml_round_pd(a); }
inline m256d svml_round_pd(const m256d& a) noexcept { return _mm256_svml_round_pd(a); }
inline m128 svml_round_ps(const m128& a) noexcept { return _mm_svml_round_ps(a); }
inline m256 svml_round_ps(const m256& a) noexcept { return _mm256_svml_round_ps(a); }

// STORE ///////////////////////////////////////////////////////////////////////

inline void maskmoveu_si128(const m128i& a, const m128i& mask, char* mem_addr) noexcept { _mm_maskmoveu_si128(a, mask, mem_addr); }
inline void maskstore_epi32(int* mem_addr, const m128i& a, const m128i& mask) noexcept { _mm_maskstore_epi32(mem_addr, mask, a); }
inline void maskstore_epi32(int* mem_addr, const m256i& a, const m256i& mask) noexcept { _mm256_maskstore_epi32(mem_addr, mask, a); }
inline void maskstore_epi64(long long* mem_addr, const m128i& a, const m128i& mask) noexcept { _mm_maskstore_epi64(mem_addr, mask, a); }
inline void maskstore_epi64(long long* mem_addr, const m256i& a, const m256i& mask) noexcept { _mm256_maskstore_epi64(mem_addr, mask, a); }
inline void maskstore_pd(double* mem_addr, const m128d& a, const m128i& mask) noexcept { _mm_maskstore_pd(mem_addr, mask, a); }
inline void maskstore_pd(double* mem_addr, const m256d& a, const m256i& mask) noexcept { _mm256_maskstore_pd(mem_addr, mask, a); }
inline void maskstore_ps(float* mem_addr, const m128& a, const m128i& mask) noexcept { _mm_maskstore_ps(mem_addr, mask, a); }
inline void maskstore_ps(float* mem_addr, const m256& a, const m256i& mask) noexcept { _mm256_maskstore_ps(mem_addr, mask, a); }
inline void store_pd(double* mem_addr, const m128d& a) noexcept { _mm_store_pd(mem_addr, a); }
inline void store_pd(double* mem_addr, const m256d& a) noexcept { _mm256_store_pd(mem_addr, a); }
inline void store_pd1(double* mem_addr, const m128d& a) noexcept { _mm_store_pd1(mem_addr, a); }
inline void store_ps(float* mem_addr, const m128& a) noexcept { _mm_store_ps(mem_addr, a); }
inline void store_ps(float* mem_addr, const m256& a) noexcept { _mm256_store_ps(mem_addr, a); }
inline void store_ps1(float* mem_addr, const m128& a) noexcept { _mm_store_ps1(mem_addr, a); }
inline void store_sd(double* mem_addr, const m128d& a) noexcept { _mm_store_sd(mem_addr, a); }
inline void store_si128(m128i* mem_addr, const m128i& a) noexcept { _mm_store_si128(mem_addr, a); }
inline void store_si256(m256i* mem_addr, const m256i& a) noexcept { _mm256_store_si256(mem_addr, a); }
inline void store_ss(float* mem_addr, const m128& a) noexcept { _mm_store_ss(mem_addr, a); }
inline void store1_pd(double* mem_addr, const m128d& a) noexcept { _mm_store1_pd(mem_addr, a); }
inline void store1_ps(float* mem_addr, const m128& a) noexcept { _mm_store1_ps(mem_addr, a); }
inline void storeh_pd(double* mem_addr, const m128d& a) noexcept { _mm_storeh_pd(mem_addr, a); }
inline void storel_epi64(m128i* mem_addr, const m128i& a) noexcept { _mm_storel_epi64(mem_addr, a); }
inline void storel_pd(double* mem_addr, const m128d& a) noexcept { _mm_storel_pd(mem_addr, a); }
inline void storer_pd(double* mem_addr, const m128d& a) noexcept { _mm_storer_pd(mem_addr, a); }
inline void storer_ps(float* mem_addr, const m128& a) noexcept { _mm_storer_ps(mem_addr, a); }
inline void storeu_pd(double* mem_addr, const m128d& a) noexcept { _mm_storeu_pd(mem_addr, a); }
inline void storeu_pd(double* mem_addr, const m256d& a) noexcept { _mm256_storeu_pd(mem_addr, a); }
inline void storeu_ps(float* mem_addr, const m128& a) noexcept { _mm_storeu_ps(mem_addr, a); }
inline void storeu_ps(float* mem_addr, const m256& a) noexcept { _mm256_storeu_ps(mem_addr, a); }
inline void storeu_si128(m128i* mem_addr, const m128i& a) noexcept { _mm_storeu_si128(mem_addr, a); }
inline void storeu_si16(short* mem_addr, const m128i& a) noexcept { _mm_storeu_si16(mem_addr, a); }
inline void storeu_si256(m256i* mem_addr, const m256i& a) noexcept { _mm256_storeu_si256(mem_addr, a); }
inline void storeu_si32(int* mem_addr, const m128i& a) noexcept { _mm_storeu_si32(mem_addr, a); }
inline void storeu_si64(long long* mem_addr, const m128i& a) noexcept { _mm_storeu_si64(mem_addr, a); }
inline void storeu2_m128(float* hiaddr, float* loaddr, m256 a) noexcept { _mm256_storeu2_m128(hiaddr, loaddr, a); }
inline void storeu2_m128d(double* hiaddr, double* loaddr, m256d a) noexcept { _mm256_storeu2_m128d(hiaddr, loaddr, a); }
inline void storeu2_m128i(m128i* hiaddr, m128i* loaddr, m256i a) noexcept { _mm256_storeu2_m128i(hiaddr, loaddr, a); }
inline void stream_pd(double* mem_addr, const m128d& a) noexcept { _mm_stream_pd(mem_addr, a); }
inline void stream_pd(double* mem_addr, const m256d& a) noexcept { _mm256_stream_pd(mem_addr, a); }
inline void stream_ps(float* mem_addr, const m128& a) noexcept { _mm_stream_ps(mem_addr, a); }
inline void stream_ps(float* mem_addr, const m256& a) noexcept { _mm256_stream_ps(mem_addr, a); }
inline void stream_si128(m128i* mem_addr, const m128i& a) noexcept { _mm_stream_si128(mem_addr, a); }
inline void stream_si256(m256i* mem_addr, const m256i& a) noexcept { _mm256_stream_si256(mem_addr, a); }
inline void stream_si32(void* mem_addr, int a) noexcept { _mm_stream_si32((int*)mem_addr, a); }

// STRING COMPARE //////////////////////////////////////////////////////////////

template<int imm8> int cmpestra(const m128i& a, int la, const m128i& b, int lb) noexcept { return _mm_cmpestra(a, la, b, lb, imm8); }
template<int imm8> int cmpestrc(const m128i& a, int la, const m128i& b, int lb) noexcept { return _mm_cmpestrc(a, la, b, lb, imm8); }
template<int imm8> int cmpestri(const m128i& a, int la, const m128i& b, int lb) noexcept { return _mm_cmpestri(a, la, b, lb, imm8); }
template<int imm8> m128i cmpestrm(const m128i& a, int la, const m128i& b, int lb) noexcept { return _mm_cmpestrm(a, la, b, lb, imm8); }
template<int imm8> int cmpestro(const m128i& a, int la, const m128i& b, int lb) noexcept { return _mm_cmpestro(a, la, b, lb, imm8); }
template<int imm8> int cmpestrs(const m128i& a, int la, const m128i& b, int lb) noexcept { return _mm_cmpestrs(a, la, b, lb, imm8); }
template<int imm8> int cmpestrz(const m128i& a, int la, const m128i& b, int lb) noexcept { return _mm_cmpestrz(a, la, b, lb, imm8); }
template<int imm8> int cmpistra(const m128i& a, const m128i& b) noexcept { return _mm_cmpistra(a, b, imm8); }
template<int imm8> int cmpistrc(const m128i& a, const m128i& b) noexcept { return _mm_cmpistrc(a, b, imm8); }
template<int imm8> int cmpistri(const m128i& a, const m128i& b) noexcept { return _mm_cmpistri(a, b, imm8); }
template<int imm8> m128i cmpistrm(const m128i& a, const m128i& b) noexcept { return _mm_cmpistrm(a, b, imm8); }
template<int imm8> int cmpistro(const m128i& a, const m128i& b) noexcept { return _mm_cmpistro(a, b, imm8); }
template<int imm8> int cmpistrs(const m128i& a, const m128i& b) noexcept { return _mm_cmpistrs(a, b, imm8); }
template<int imm8> int cmpistrz(const m128i& a, const m128i& b) noexcept { return _mm_cmpistrz(a, b, imm8); }

// SWIZZLE /////////////////////////////////////////////////////////////////////

template<int imm8> m128i blend_epi16(const m128i& a, const m128i& b) noexcept { return _mm_blend_epi16(a, b, imm8); }
template<int imm8> m256i blend_epi16(const m256i& a, const m256i& b) noexcept { return _mm256_blend_epi16(a, b, imm8); }
template<int imm8> m128i blend_epi32(const m128i& a, const m128i& b) noexcept { return _mm_blend_epi32(a, b, imm8); }
template<int imm8> m256i blend_epi32(const m256i& a, const m256i& b) noexcept { return _mm256_blend_epi32(a, b, imm8); }
template<int imm8> m128d blend_pd(const m128d& a, const m128d& b) noexcept { return _mm_blend_pd(a, b, imm8); }
template<int imm8> m256d blend_pd(const m256d& a, const m256d& b) noexcept { return _mm256_blend_pd(a, b, imm8); }
template<int imm8> m128 blend_ps(const m128& a, const m128& b) noexcept { return _mm_blend_ps(a, b, imm8); }
template<int imm8> m256 blend_ps(const m256& a, const m256& b) noexcept { return _mm256_blend_ps(a, b, imm8); }
inline m128i blendv_epi8(const m128i& a, const m128i& b, const m128i& mask) noexcept { return _mm_blendv_epi8(a, b, mask); }
inline m256i blendv_epi8(const m256i& a, const m256i& b, const m256i& mask) noexcept { return _mm256_blendv_epi8(a, b, mask); }
inline m128d blendv_pd(const m128d& a, const m128d& b, const m128d& mask) noexcept { return _mm_blendv_pd(a, b, mask); }
inline m256d blendv_pd(const m256d& a, const m256d& b, const m256d& mask) noexcept { return _mm256_blendv_pd(a, b, mask); }
inline m128 blendv_ps(const m128& a, const m128& b, const m128& mask) noexcept { return _mm_blendv_ps(a, b, mask); }
inline m256 blendv_ps(const m256& a, const m256& b, const m256& mask) noexcept { return _mm256_blendv_ps(a, b, mask); }

template<int imm8> int extract_epi16(const m128i& a) noexcept { return _mm_extract_epi16(a, imm8); }
template<int imm8> int extract_epi16(const m256i& a) noexcept { return _mm256_extract_epi16(a, imm8); }
template<int imm8> int extract_epi32(const m128i& a) noexcept { return _mm_extract_epi32(a, imm8); }
template<int imm8> int extract_epi32(const m256i& a) noexcept { return _mm256_extract_epi32(a, imm8); }
template<int imm8> long long extract_epi64(const m128i& a) noexcept { return _mm_extract_epi64(a, imm8); }
template<int imm8> long long extract_epi64(const m256i& a) noexcept { return _mm256_extract_epi64(a, imm8); }
template<int imm8> int extract_epi8(const m128i& a) noexcept { return _mm_extract_epi8(a, imm8); }
template<int imm8> int extract_epi8(const m256i& a) noexcept { return _mm256_extract_epi8(a, imm8); }
template<int imm8> int extract_ps(const m128& a) noexcept { return _mm_extract_ps(a, imm8); }
template<int imm8> m128d extractf128_pd(const m256d& a) noexcept { return _mm256_extractf128_pd(a, imm8); }
template<int imm8> m128 extractf128_ps(const m256& a) noexcept { return _mm256_extractf128_ps(a, imm8); }
template<int imm8> m128i extractf128_si256(const m256i& a) noexcept { return _mm256_extractf128_si256(a, imm8); }
template<int imm8> m128i extracti128_si256(const m256i& a) noexcept { return _mm256_extracti128_si256(a, imm8); }
template<int imm8> m128i insert_epi16(const m128i& a, int i) noexcept { return _mm_insert_epi16(a, i, imm8); }
template<int imm8> m256i insert_epi16(const m256i& a, int i) noexcept { return _mm256_insert_epi16(a, i, imm8); }
template<int imm8> m128i insert_epi32(const m128i& a, int i) noexcept { return _mm_insert_epi32(a, i, imm8); }
template<int imm8> m256i insert_epi32(const m256i& a, int i) noexcept { return _mm256_insert_epi32(a, i, imm8); }
template<int imm8> m128i insert_epi64(const m128i& a, long long i) noexcept { return _mm_insert_epi64(a, i, imm8); }
template<int imm8> m256i insert_epi64(const m256i& a, long long i) noexcept { return _mm256_insert_epi64(a, i, imm8); }
template<int imm8> m128i insert_epi8(const m128i& a, int i) noexcept { return _mm_insert_epi8(a, i, imm8); }
template<int imm8> m256i insert_epi8(const m256i& a, int i) noexcept { return _mm256_insert_epi8(a, i, imm8); }
template<int imm8> m128 insert_ps(const m128& a, int i) noexcept { return _mm_insert_ps(a, i, imm8); }
template<int imm8> m256d insertf128_pd(const m256d& a, const m128d& b) noexcept { return _mm256_insertf128_pd(a, b, imm8); }
template<int imm8> m256 insertf128_ps(const m256& a, const m128& b) noexcept { return _mm256_insertf128_ps(a, b, imm8); }
template<int imm8> m256i insertf128_si256(const m256i& a, const m128i& b) noexcept { return _mm256_insertf128_si256(a, b, imm8); }
template<int imm8> m256i inserti128_si256(const m256i& a, const m128i& b) noexcept { return _mm256_inserti128_si256(a, b, imm8); }
template<int imm8> m128d permute_pd(const m128d& a) noexcept { return _mm_permute_pd(a, imm8); }
template<int imm8> m256d permute_pd(const m256d& a) noexcept { return _mm256_permute_pd(a, imm8); }
template<int imm8> m128 permute_ps(const m128& a) noexcept { return _mm_permute_ps(a, imm8); }
template<int imm8> m256 permute_ps(const m256& a) noexcept { return _mm256_permute_ps(a, imm8); }
template<int imm8> m256d permute2f128_pd(const m256d& a) noexcept { return _mm256_permute2f128_pd(a, imm8); }
template<int imm8> m256 permute2f128_ps(const m256& a) noexcept { return _mm256_permute2f128_ps(a, imm8); }
template<int imm8> m256i permute2f128_si256(const m256i& a) noexcept { return _mm256_permute2f128_si256(a, imm8); }
template<int imm8> m256i permute2x128_si256(const m256i& a) noexcept { return _mm256_permute2x128_si256(a, imm8); }
template<int imm8> m256i permute4x64_epi64(const m256i& a) noexcept { return _mm256_permute4x64_epi64(a, imm8); }
template<int imm8> m256d permute4x64_pd(const m256d& a) noexcept { return _mm256_permute4x64_pd(a, imm8); }
inline m128d permutevar_pd(const m128d& a, const m128i& b) noexcept { return _mm_permutevar_pd(a, b); }
inline m256d permutevar_pd(const m256d& a, const m256i& b) noexcept { return _mm256_permutevar_pd(a, b); }
inline m128 permutevar_ps(const m128& a, const m128i& b) noexcept { return _mm_permutevar_ps(a, b); }
inline m256 permutevar_ps(const m256& a, const m256i& b) noexcept { return _mm256_permutevar_ps(a, b); }
inline m256i permutevar8x32_epi32(const m256i& a, const m256i& b) noexcept { return _mm256_permutevar8x32_epi32(a, b); }
inline m256 permutevar8x32_ps(const m256& a, const m256i& b) noexcept { return _mm256_permutevar8x32_ps(a, b); }
template<int imm8> m128i shuffle_epi32(const m128i& a) noexcept { return _mm_shuffle_epi32(a, imm8); }
template<int imm8> m256i shuffle_epi32(const m256i& a) noexcept { return _mm256_shuffle_epi32(a, imm8); }
inline m128i shuffle_epi8(const m128i& a, const m128i& b) noexcept { return _mm_shuffle_epi8(a, b); }
inline m256i shuffle_epi8(const m256i& a, const m256i& b) noexcept { return _mm256_shuffle_epi8(a, b); }
template<int imm8> m128d shuffle_pd(const m128d& a, const m128d& b) noexcept { return _mm_shuffle_pd(a, b, imm8); }
template<int imm8> m256d shuffle_pd(const m256d& a, const m256d& b) noexcept { return _mm256_shuffle_pd(a, b, imm8); }
template<int imm8> m128 shuffle_ps(const m128& a, const m128& b) noexcept { return _mm_shuffle_ps(a, b, imm8); }
template<int imm8> m256 shuffle_ps(const m256& a, const m256& b) noexcept { return _mm256_shuffle_ps(a, b, imm8); }
template<int imm8> m128i shufflehi_epi16(const m128i& a) noexcept { return _mm_shufflehi_epi16(a, imm8); }
template<int imm8> m256i shufflehi_epi16(const m256i& a) noexcept { return _mm256_shufflehi_epi16(a, imm8); }
template<int imm8> m128i shufflelo_epi16(const m128i& a) noexcept { return _mm_shufflelo_epi16(a, imm8); }
template<int imm8> m256i shufflelo_epi16(const m256i& a) noexcept { return _mm256_shufflelo_epi16(a, imm8); }
inline m128i unpackhi_epi16(const m128i& a, const m128i& b) noexcept { return _mm_unpackhi_epi16(a, b); }
inline m256i unpackhi_epi16(const m256i& a, const m256i& b) noexcept { return _mm256_unpackhi_epi16(a, b); }
inline m128i unpackhi_epi32(const m128i& a, const m128i& b) noexcept { return _mm_unpackhi_epi32(a, b); }
inline m256i unpackhi_epi32(const m256i& a, const m256i& b) noexcept { return _mm256_unpackhi_epi32(a, b); }
inline m128i unpackhi_epi64(const m128i& a, const m128i& b) noexcept { return _mm_unpackhi_epi64(a, b); }
inline m256i unpackhi_epi64(const m256i& a, const m256i& b) noexcept { return _mm256_unpackhi_epi64(a, b); }
inline m128i unpackhi_epi8(const m128i& a, const m128i& b) noexcept { return _mm_unpackhi_epi8(a, b); }
inline m256i unpackhi_epi8(const m256i& a, const m256i& b) noexcept { return _mm256_unpackhi_epi8(a, b); }
inline m128d unpackhi_pd(const m128d& a, const m128d& b) noexcept { return _mm_unpackhi_pd(a, b); }
inline m256d unpackhi_pd(const m256d& a, const m256d& b) noexcept { return _mm256_unpackhi_pd(a, b); }
inline m128 unpackhi_ps(const m128& a, const m128& b) noexcept { return _mm_unpackhi_ps(a, b); }
inline m256 unpackhi_ps(const m256& a, const m256& b) noexcept { return _mm256_unpackhi_ps(a, b); }
inline m128i unpacklo_epi16(const m128i& a, const m128i& b) noexcept { return _mm_unpacklo_epi16(a, b); }
inline m256i unpacklo_epi16(const m256i& a, const m256i& b) noexcept { return _mm256_unpacklo_epi16(a, b); }
inline m128i unpacklo_epi32(const m128i& a, const m128i& b) noexcept { return _mm_unpacklo_epi32(a, b); }
inline m256i unpacklo_epi32(const m256i& a, const m256i& b) noexcept { return _mm256_unpacklo_epi32(a, b); }
inline m128i unpacklo_epi64(const m128i& a, const m128i& b) noexcept { return _mm_unpacklo_epi64(a, b); }
inline m256i unpacklo_epi64(const m256i& a, const m256i& b) noexcept { return _mm256_unpacklo_epi64(a, b); }
inline m128i unpacklo_epi8(const m128i& a, const m128i& b) noexcept { return _mm_unpacklo_epi8(a, b); }
inline m256i unpacklo_epi8(const m256i& a, const m256i& b) noexcept { return _mm256_unpacklo_epi8(a, b); }
inline m128d unpacklo_pd(const m128d& a, const m128d& b) noexcept { return _mm_unpacklo_pd(a, b); }
inline m256d unpacklo_pd(const m256d& a, const m256d& b) noexcept { return _mm256_unpacklo_pd(a, b); }
inline m128 unpacklo_ps(const m128& a, const m128& b) noexcept { return _mm_unpacklo_ps(a, b); }
inline m256 unpacklo_ps(const m256& a, const m256& b) noexcept { return _mm256_unpacklo_ps(a, b); }

// TRIGONOMETRY ////////////////////////////////////////////////////////////////

inline m128d acos_pd(const m128d& a) noexcept { return _mm_acos_pd(a); }
inline m256d acos_pd(const m256d& a) noexcept { return _mm256_acos_pd(a); }
inline m128 acos_ps(const m128& a) noexcept { return _mm_acos_ps(a); }
inline m256 acos_ps(const m256& a) noexcept { return _mm256_acos_ps(a); }
inline m128d acosh_pd(const m128d& a) noexcept { return _mm_acosh_pd(a); }
inline m256d acosh_pd(const m256d& a) noexcept { return _mm256_acosh_pd(a); }
inline m128 acosh_ps(const m128& a) noexcept { return _mm_acosh_ps(a); }
inline m256 acosh_ps(const m256& a) noexcept { return _mm256_acosh_ps(a); }
inline m128d asin_pd(const m128d& a) noexcept { return _mm_asin_pd(a); }
inline m256d asin_pd(const m256d& a) noexcept { return _mm256_asin_pd(a); }
inline m128 asin_ps(const m128& a) noexcept { return _mm_asin_ps(a); }
inline m256 asin_ps(const m256& a) noexcept { return _mm256_asin_ps(a); }
inline m128d asinh_pd(const m128d& a) noexcept { return _mm_asinh_pd(a); }
inline m256d asinh_pd(const m256d& a) noexcept { return _mm256_asinh_pd(a); }
inline m128 asinh_ps(const m128& a) noexcept { return _mm_asinh_ps(a); }
inline m256 asinh_ps(const m256& a) noexcept { return _mm256_asinh_ps(a); }
inline m128d atan_pd(const m128d& a) noexcept { return _mm_atan_pd(a); }
inline m256d atan_pd(const m256d& a) noexcept { return _mm256_atan_pd(a); }
inline m128 atan_ps(const m128& a) noexcept { return _mm_atan_ps(a); }
inline m256 atan_ps(const m256& a) noexcept { return _mm256_atan_ps(a); }
inline m128d atan2_pd(const m128d& a, const m128d& b) noexcept { return _mm_atan2_pd(a, b); }
inline m256d atan2_pd(const m256d& a, const m256d& b) noexcept { return _mm256_atan2_pd(a, b); }
inline m128 atan2_ps(const m128& a, const m128& b) noexcept { return _mm_atan2_ps(a, b); }
inline m256 atan2_ps(const m256& a, const m256& b) noexcept { return _mm256_atan2_ps(a, b); }
inline m128d atanh_pd(const m128d& a) noexcept { return _mm_atanh_pd(a); }
inline m256d atanh_pd(const m256d& a) noexcept { return _mm256_atanh_pd(a); }
inline m128 atanh_ps(const m128& a) noexcept { return _mm_atanh_ps(a); }
inline m256 atanh_ps(const m256& a) noexcept { return _mm256_atanh_ps(a); }
inline m128d cos_pd(const m128d& a) noexcept { return _mm_cos_pd(a); }
inline m256d cos_pd(const m256d& a) noexcept { return _mm256_cos_pd(a); }
inline m128 cos_ps(const m128& a) noexcept { return _mm_cos_ps(a); }
inline m256 cos_ps(const m256& a) noexcept { return _mm256_cos_ps(a); }
inline m128d cosd_pd(const m128d& a) noexcept { return _mm_cosd_pd(a); }
inline m256d cosd_pd(const m256d& a) noexcept { return _mm256_cosd_pd(a); }
inline m128 cosd_ps(const m128& a) noexcept { return _mm_cosd_ps(a); }
inline m256 cosd_ps(const m256& a) noexcept { return _mm256_cosd_ps(a); }
inline m128d cosh_pd(const m128d& a) noexcept { return _mm_cosh_pd(a); }
inline m256d cosh_pd(const m256d& a) noexcept { return _mm256_cosh_pd(a); }
inline m128 cosh_ps(const m128& a) noexcept { return _mm_cosh_ps(a); }
inline m256 cosh_ps(const m256& a) noexcept { return _mm256_cosh_ps(a); }
inline m128d hypot_pd(const m128d& a, const m128d& b) noexcept { return _mm_hypot_pd(a, b); }
inline m256d hypot_pd(const m256d& a, const m256d& b) noexcept { return _mm256_hypot_pd(a, b); }
inline m128 hypot_ps(const m128& a, const m128& b) noexcept { return _mm_hypot_ps(a, b); }
inline m256 hypot_ps(const m256& a, const m256& b) noexcept { return _mm256_hypot_ps(a, b); }
inline m128d sin_pd(const m128d& a) noexcept { return _mm_sin_pd(a); }
inline m256d sin_pd(const m256d& a) noexcept { return _mm256_sin_pd(a); }
inline m128 sin_ps(const m128& a) noexcept { return _mm_sin_ps(a); }
inline m256 sin_ps(const m256& a) noexcept { return _mm256_sin_ps(a); }
inline m128d sincos_pd(m128d* mem_addr, const m128d& a) noexcept { return _mm_sincos_pd(mem_addr, a); }
inline m256d sincos_pd(m256d* mem_addr, const m256d& a) noexcept { return _mm256_sincos_pd(mem_addr, a); }
inline m128 sincos_ps(m128* mem_addr, const m128& a) noexcept { return _mm_sincos_ps(mem_addr, a); }
inline m256 sincos_ps(m256* mem_addr, const m256& a) noexcept { return _mm256_sincos_ps(mem_addr, a); }
inline m128d sind_pd(const m128d& a) noexcept { return _mm_sind_pd(a); }
inline m256d sind_pd(const m256d& a) noexcept { return _mm256_sind_pd(a); }
inline m128 sind_ps(const m128& a) noexcept { return _mm_sind_ps(a); }
inline m256 sind_ps(const m256& a) noexcept { return _mm256_sind_ps(a); }
inline m128d sinh_pd(const m128d& a) noexcept { return _mm_sinh_pd(a); }
inline m256d sinh_pd(const m256d& a) noexcept { return _mm256_sinh_pd(a); }
inline m128 sinh_ps(const m128& a) noexcept { return _mm_sinh_ps(a); }
inline m256 sinh_ps(const m256& a) noexcept { return _mm256_sinh_ps(a); }
inline m128d tan_pd(const m128d& a) noexcept { return _mm_tan_pd(a); }
inline m256d tan_pd(const m256d& a) noexcept { return _mm256_tan_pd(a); }
inline m128 tan_ps(const m128& a) noexcept { return _mm_tan_ps(a); }
inline m256 tan_ps(const m256& a) noexcept { return _mm256_tan_ps(a); }
inline m128d tand_pd(const m128d& a) noexcept { return _mm_tand_pd(a); }
inline m256d tand_pd(const m256d& a) noexcept { return _mm256_tand_pd(a); }
inline m128 tand_ps(const m128& a) noexcept { return _mm_tand_ps(a); }
inline m256 tand_ps(const m256& a) noexcept { return _mm256_tand_ps(a); }
inline m128d tanh_pd(const m128d& a) noexcept { return _mm_tanh_pd(a); }
inline m256d tanh_pd(const m256d& a) noexcept { return _mm256_tanh_pd(a); }
inline m128 tanh_ps(const m128& a) noexcept { return _mm_tanh_ps(a); }
inline m256 tanh_ps(const m256& a) noexcept { return _mm256_tanh_ps(a); }



};
#else
import intrin;
#endif
