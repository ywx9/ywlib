#pragma once

#if defined(YWSTD_IMPORT) && YWSTD_IMPORT
#pragma message ("YWSTD_IMPORT")
import ywstd;
#else
#define _BUILD_STD_MODULE

#include <algorithm>
#include <any>
#include <array>
#include <atomic>
#include <barrier>
#include <bit>
#include <bitset>
#include <cassert>
#include <cctype>
#include <cerrno>
#include <cfenv>
#include <cfloat>
#include <charconv>
#include <chrono>
#include <cinttypes>
#include <climits>
#include <clocale>
#include <cmath>
#include <codecvt>
#include <compare>
#include <complex>
#include <concepts>
#include <condition_variable>
#include <coroutine>
#include <csetjmp>
#include <csignal>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cuchar>
#include <cwchar>
#include <cwctype>
#include <deque>
#include <exception>
#include <execution>
#include <expected>
#include <filesystem>
#include <format>
#include <forward_list>
#include <fstream>
#include <functional>
#include <future>
#include <initializer_list>
#include <iomanip>
#include <ios>
#include <iosfwd>
#include <iostream>
#include <istream>
#include <iterator>
#include <latch>
#include <limits>
#include <list>
#include <locale>
#include <map>
#include <memory>
#include <memory_resource>
#include <mutex>
#include <new>
#include <numbers>
#include <numeric>
#include <optional>
#include <ostream>
#include <print>
#include <queue>
#include <random>
#include <ranges>
#include <ratio>
#include <regex>
#include <scoped_allocator>
#include <semaphore>
#include <set>
#include <shared_mutex>
#include <source_location>
#include <span>
#include <spanstream>
#include <sstream>
#include <stack>
#include <stacktrace>
#include <stdexcept>
#include <stdfloat>
#include <stop_token>
#include <streambuf>
#include <string>
#include <string_view>
#include <strstream>
#include <syncstream>
#include <system_error>
#include <thread>
#include <tuple>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <valarray>
#include <variant>
#include <vector>
#include <version>

export namespace std {}

#include <immintrin.h>

export namespace intrin {

using m128 = __m128;
using m128d = __m128d;
using m128i = __m128i;
using m256 = __m256;
using m256d = __m256d;
using m256i = __m256i;

inline __m128i mm_aesenc_si128(const __m128i& a, const __m128i& b) noexcept { return _mm_aesenc_si128(a, b); }
inline __m128i mm_aesenclast_si128(const __m128i& a, const __m128i& b) noexcept { return _mm_aesenclast_si128(a, b); }
inline __m128i mm_aesdec_si128(const __m128i& a, const __m128i& b) noexcept { return _mm_aesdec_si128(a, b); }
inline __m128i mm_aesdeclast_si128(const __m128i& a, const __m128i& b) noexcept { return _mm_aesdeclast_si128(a, b); }
inline __m128i mm_aesimc_si128(const __m128i& a) noexcept { return _mm_aesimc_si128(a); }
template<int i> __m128i mm_aeskeygenassist_si128(const __m128i& a) noexcept { return _mm_aeskeygenassist_si128(a, i); }
inline __m256d mm256_acos_pd(const __m256d& a) noexcept { return _mm256_acos_pd(a); }
inline __m256 mm256_acos_ps(const __m256& a) noexcept { return _mm256_acos_ps(a); }
inline __m256d mm256_acosh_pd(const __m256d& a) noexcept { return _mm256_acosh_pd(a); }
inline __m256 mm256_acosh_ps(const __m256& a) noexcept { return _mm256_acosh_ps(a); }
inline __m256d mm256_asin_pd(const __m256d& a) noexcept { return _mm256_asin_pd(a); }
inline __m256 mm256_asin_ps(const __m256& a) noexcept { return _mm256_asin_ps(a); }
inline __m256d mm256_asinh_pd(const __m256d& a) noexcept { return _mm256_asinh_pd(a); }
inline __m256 mm256_asinh_ps(const __m256& a) noexcept { return _mm256_asinh_ps(a); }
inline __m256d mm256_atan_pd(const __m256d& a) noexcept { return _mm256_atan_pd(a); }
inline __m256 mm256_atan_ps(const __m256& a) noexcept { return _mm256_atan_ps(a); }
inline __m256d mm256_atan2_pd(const __m256d& a, const __m256d& b) noexcept { return _mm256_atan2_pd(a, b); }
inline __m256 mm256_atan2_ps(const __m256& a, const __m256& b) noexcept { return _mm256_atan2_ps(a, b); }
inline __m256d mm256_atanh_pd(const __m256d& a) noexcept { return _mm256_atanh_pd(a); }
inline __m256 mm256_atanh_ps(const __m256& a) noexcept { return _mm256_atanh_ps(a); }
inline __m256d mm256_cos_pd(const __m256d& a) noexcept { return _mm256_cos_pd(a); }
inline __m256 mm256_cos_ps(const __m256& a) noexcept { return _mm256_cos_ps(a); }
inline __m256d mm256_cosd_pd(const __m256d& a) noexcept { return _mm256_cosd_pd(a); }
inline __m256 mm256_cosd_ps(const __m256& a) noexcept { return _mm256_cosd_ps(a); }
inline __m256d mm256_cosh_pd(const __m256d& a) noexcept { return _mm256_cosh_pd(a); }
inline __m256 mm256_cosh_ps(const __m256& a) noexcept { return _mm256_cosh_ps(a); }
inline __m256d mm256_hypot_pd(const __m256d& a, const __m256d& b) noexcept { return _mm256_hypot_pd(a, b); }
inline __m256 mm256_hypot_ps(const __m256& a, const __m256& b) noexcept { return _mm256_hypot_ps(a, b); }
inline __m256d mm256_sin_pd(const __m256d& a) noexcept { return _mm256_sin_pd(a); }
inline __m256 mm256_sin_ps(const __m256& a) noexcept { return _mm256_sin_ps(a); }
inline __m256d mm256_sincos_pd(__m256d* a, const __m256d& b) noexcept { return _mm256_sincos_pd(a, b); }
inline __m256 mm256_sincos_ps(__m256* a, const __m256& b) noexcept { return _mm256_sincos_ps(a, b); }
inline __m256d mm256_sind_pd(const __m256d& a) noexcept { return _mm256_sind_pd(a); }
inline __m256 mm256_sind_ps(const __m256& a) noexcept { return _mm256_sind_ps(a); }
inline __m256d mm256_sinh_pd(const __m256d& a) noexcept { return _mm256_sinh_pd(a); }
inline __m256 mm256_sinh_ps(const __m256& a) noexcept { return _mm256_sinh_ps(a); }
inline __m256d mm256_tan_pd(const __m256d& a) noexcept { return _mm256_tan_pd(a); }
inline __m256 mm256_tan_ps(const __m256& a) noexcept { return _mm256_tan_ps(a); }
inline __m256d mm256_tand_pd(const __m256d& a) noexcept { return _mm256_tand_pd(a); }
inline __m256 mm256_tand_ps(const __m256& a) noexcept { return _mm256_tand_ps(a); }
inline __m256d mm256_tanh_pd(const __m256d& a) noexcept { return _mm256_tanh_pd(a); }
inline __m256 mm256_tanh_ps(const __m256& a) noexcept { return _mm256_tanh_ps(a); }
inline __m256d mm256_cbrt_pd(const __m256d& a) noexcept { return _mm256_cbrt_pd(a); }
inline __m256 mm256_cbrt_ps(const __m256& a) noexcept { return _mm256_cbrt_ps(a); }
inline __m256 mm256_cexp_ps(const __m256& a) noexcept { return _mm256_cexp_ps(a); }
inline __m256 mm256_clog_ps(const __m256& a) noexcept { return _mm256_clog_ps(a); }
inline __m256 mm256_csqrt_ps(const __m256& a) noexcept { return _mm256_csqrt_ps(a); }
inline __m256d mm256_exp_pd(const __m256d& a) noexcept { return _mm256_exp_pd(a); }
inline __m256 mm256_exp_ps(const __m256& a) noexcept { return _mm256_exp_ps(a); }
inline __m256d mm256_exp10_pd(const __m256d& a) noexcept { return _mm256_exp10_pd(a); }
inline __m256 mm256_exp10_ps(const __m256& a) noexcept { return _mm256_exp10_ps(a); }
inline __m256d mm256_exp2_pd(const __m256d& a) noexcept { return _mm256_exp2_pd(a); }
inline __m256 mm256_exp2_ps(const __m256& a) noexcept { return _mm256_exp2_ps(a); }
inline __m256d mm256_expm1_pd(const __m256d& a) noexcept { return _mm256_expm1_pd(a); }
inline __m256 mm256_expm1_ps(const __m256& a) noexcept { return _mm256_expm1_ps(a); }
inline __m256d mm256_invcbrt_pd(const __m256d& a) noexcept { return _mm256_invcbrt_pd(a); }
inline __m256 mm256_invcbrt_ps(const __m256& a) noexcept { return _mm256_invcbrt_ps(a); }
inline __m256d mm256_invsqrt_pd(const __m256d& a) noexcept { return _mm256_invsqrt_pd(a); }
inline __m256 mm256_invsqrt_ps(const __m256& a) noexcept { return _mm256_invsqrt_ps(a); }
inline __m256d mm256_log_pd(const __m256d& a) noexcept { return _mm256_log_pd(a); }
inline __m256 mm256_log_ps(const __m256& a) noexcept { return _mm256_log_ps(a); }
inline __m256d mm256_log10_pd(const __m256d& a) noexcept { return _mm256_log10_pd(a); }
inline __m256 mm256_log10_ps(const __m256& a) noexcept { return _mm256_log10_ps(a); }
inline __m256d mm256_log1p_pd(const __m256d& a) noexcept { return _mm256_log1p_pd(a); }
inline __m256 mm256_log1p_ps(const __m256& a) noexcept { return _mm256_log1p_ps(a); }
inline __m256d mm256_log2_pd(const __m256d& a) noexcept { return _mm256_log2_pd(a); }
inline __m256 mm256_log2_ps(const __m256& a) noexcept { return _mm256_log2_ps(a); }
inline __m256d mm256_logb_pd(const __m256d& a) noexcept { return _mm256_logb_pd(a); }
inline __m256 mm256_logb_ps(const __m256& a) noexcept { return _mm256_logb_ps(a); }
inline __m256d mm256_pow_pd(const __m256d& a, const __m256d& b) noexcept { return _mm256_pow_pd(a, b); }
inline __m256 mm256_pow_ps(const __m256& a, const __m256& b) noexcept { return _mm256_pow_ps(a, b); }
inline __m256d mm256_svml_sqrt_pd(const __m256d& a) noexcept { return _mm256_svml_sqrt_pd(a); }
inline __m256 mm256_svml_sqrt_ps(const __m256& a) noexcept { return _mm256_svml_sqrt_ps(a); }
inline __m256d mm256_cdfnorm_pd(const __m256d& a) noexcept { return _mm256_cdfnorm_pd(a); }
inline __m256 mm256_cdfnorm_ps(const __m256& a) noexcept { return _mm256_cdfnorm_ps(a); }
inline __m256d mm256_cdfnorminv_pd(const __m256d& a) noexcept { return _mm256_cdfnorminv_pd(a); }
inline __m256 mm256_cdfnorminv_ps(const __m256& a) noexcept { return _mm256_cdfnorminv_ps(a); }
inline __m256d mm256_erf_pd(const __m256d& a) noexcept { return _mm256_erf_pd(a); }
inline __m256 mm256_erf_ps(const __m256& a) noexcept { return _mm256_erf_ps(a); }
inline __m256d mm256_erfc_pd(const __m256d& a) noexcept { return _mm256_erfc_pd(a); }
inline __m256 mm256_erfc_ps(const __m256& a) noexcept { return _mm256_erfc_ps(a); }
inline __m256d mm256_erfcinv_pd(const __m256d& a) noexcept { return _mm256_erfcinv_pd(a); }
inline __m256 mm256_erfcinv_ps(const __m256& a) noexcept { return _mm256_erfcinv_ps(a); }
inline __m256d mm256_erfinv_pd(const __m256d& a) noexcept { return _mm256_erfinv_pd(a); }
inline __m256 mm256_erfinv_ps(const __m256& a) noexcept { return _mm256_erfinv_ps(a); }
inline __m256i mm256_div_epi8(const __m256i& a, const __m256i& b) noexcept { return _mm256_div_epi8(a, b); }
inline __m256i mm256_div_epi16(const __m256i& a, const __m256i& b) noexcept { return _mm256_div_epi16(a, b); }
inline __m256i mm256_div_epi32(const __m256i& a, const __m256i& b) noexcept { return _mm256_div_epi32(a, b); }
inline __m256i mm256_div_epi64(const __m256i& a, const __m256i& b) noexcept { return _mm256_div_epi64(a, b); }
inline __m256i mm256_div_epu8(const __m256i& a, const __m256i& b) noexcept { return _mm256_div_epu8(a, b); }
inline __m256i mm256_div_epu16(const __m256i& a, const __m256i& b) noexcept { return _mm256_div_epu16(a, b); }
inline __m256i mm256_div_epu32(const __m256i& a, const __m256i& b) noexcept { return _mm256_div_epu32(a, b); }
inline __m256i mm256_div_epu64(const __m256i& a, const __m256i& b) noexcept { return _mm256_div_epu64(a, b); }
inline __m256i mm256_idiv_epi32(const __m256i& a, const __m256i& b) noexcept { return _mm256_idiv_epi32(a, b); }
inline __m256i mm256_idivrem_epi32(__m256i* a, const __m256i& b, const __m256i& c) noexcept { return _mm256_idivrem_epi32(a, b, c); }
inline __m256i mm256_irem_epi32(const __m256i& a, const __m256i& b) noexcept { return _mm256_irem_epi32(a, b); }
inline __m256i mm256_rem_epi8(const __m256i& a, const __m256i& b) noexcept { return _mm256_rem_epi8(a, b); }
inline __m256i mm256_rem_epi16(const __m256i& a, const __m256i& b) noexcept { return _mm256_rem_epi16(a, b); }
inline __m256i mm256_rem_epi32(const __m256i& a, const __m256i& b) noexcept { return _mm256_rem_epi32(a, b); }
inline __m256i mm256_rem_epi64(const __m256i& a, const __m256i& b) noexcept { return _mm256_rem_epi64(a, b); }
inline __m256i mm256_rem_epu8(const __m256i& a, const __m256i& b) noexcept { return _mm256_rem_epu8(a, b); }
inline __m256i mm256_rem_epu16(const __m256i& a, const __m256i& b) noexcept { return _mm256_rem_epu16(a, b); }
inline __m256i mm256_rem_epu32(const __m256i& a, const __m256i& b) noexcept { return _mm256_rem_epu32(a, b); }
inline __m256i mm256_rem_epu64(const __m256i& a, const __m256i& b) noexcept { return _mm256_rem_epu64(a, b); }
inline __m256i mm256_udiv_epi32(const __m256i& a, const __m256i& b) noexcept { return _mm256_udiv_epi32(a, b); }
inline __m256i mm256_udivrem_epi32(__m256i* a, const __m256i& b, const __m256i& c) noexcept { return _mm256_udivrem_epi32(a, b, c); }
inline __m256i mm256_urem_epi32(const __m256i& a, const __m256i& b) noexcept { return _mm256_urem_epi32(a, b); }
inline __m256d mm256_svml_ceil_pd(const __m256d& a) noexcept { return _mm256_svml_ceil_pd(a); }
inline __m256 mm256_svml_ceil_ps(const __m256& a) noexcept { return _mm256_svml_ceil_ps(a); }
inline __m256d mm256_svml_floor_pd(const __m256d& a) noexcept { return _mm256_svml_floor_pd(a); }
inline __m256 mm256_svml_floor_ps(const __m256& a) noexcept { return _mm256_svml_floor_ps(a); }
inline __m256d mm256_svml_round_pd(const __m256d& a) noexcept { return _mm256_svml_round_pd(a); }
inline __m256 mm256_svml_round_ps(const __m256& a) noexcept { return _mm256_svml_round_ps(a); }
inline __m256d mm256_trunc_pd(const __m256d& a) noexcept { return _mm256_trunc_pd(a); }
inline __m256 mm256_trunc_ps(const __m256& a) noexcept { return _mm256_trunc_ps(a); }
inline __m256d mm256_add_pd(const __m256d& a, const __m256d& b) noexcept { return _mm256_add_pd(a, b); }
inline __m256 mm256_add_ps(const __m256& a, const __m256& b) noexcept { return _mm256_add_ps(a, b); }
inline __m256d mm256_addsub_pd(const __m256d& a, const __m256d& b) noexcept { return _mm256_addsub_pd(a, b); }
inline __m256 mm256_addsub_ps(const __m256& a, const __m256& b) noexcept { return _mm256_addsub_ps(a, b); }
inline __m256d mm256_div_pd(const __m256d& a, const __m256d& b) noexcept { return _mm256_div_pd(a, b); }
inline __m256 mm256_div_ps(const __m256& a, const __m256& b) noexcept { return _mm256_div_ps(a, b); }
template<int i> __m256 mm256_dp_ps(const __m256& a, const __m256& b) noexcept { return _mm256_dp_ps(a, b, i); }
inline __m256d mm256_hadd_pd(const __m256d& a, const __m256d& b) noexcept { return _mm256_hadd_pd(a, b); }
inline __m256 mm256_hadd_ps(const __m256& a, const __m256& b) noexcept { return _mm256_hadd_ps(a, b); }
inline __m256d mm256_hsub_pd(const __m256d& a, const __m256d& b) noexcept { return _mm256_hsub_pd(a, b); }
inline __m256 mm256_hsub_ps(const __m256& a, const __m256& b) noexcept { return _mm256_hsub_ps(a, b); }
inline __m256d mm256_mul_pd(const __m256d& a, const __m256d& b) noexcept { return _mm256_mul_pd(a, b); }
inline __m256 mm256_mul_ps(const __m256& a, const __m256& b) noexcept { return _mm256_mul_ps(a, b); }
inline __m256d mm256_sub_pd(const __m256d& a, const __m256d& b) noexcept { return _mm256_sub_pd(a, b); }
inline __m256 mm256_sub_ps(const __m256& a, const __m256& b) noexcept { return _mm256_sub_ps(a, b); }
inline __m256d mm256_and_pd(const __m256d& a, const __m256d& b) noexcept { return _mm256_and_pd(a, b); }
inline __m256 mm256_and_ps(const __m256& a, const __m256& b) noexcept { return _mm256_and_ps(a, b); }
inline __m256d mm256_andnot_pd(const __m256d& a, const __m256d& b) noexcept { return _mm256_andnot_pd(a, b); }
inline __m256 mm256_andnot_ps(const __m256& a, const __m256& b) noexcept { return _mm256_andnot_ps(a, b); }
inline __m256d mm256_or_pd(const __m256d& a, const __m256d& b) noexcept { return _mm256_or_pd(a, b); }
inline __m256 mm256_or_ps(const __m256& a, const __m256& b) noexcept { return _mm256_or_ps(a, b); }
inline __m256d mm256_xor_pd(const __m256d& a, const __m256d& b) noexcept { return _mm256_xor_pd(a, b); }
inline __m256 mm256_xor_ps(const __m256& a, const __m256& b) noexcept { return _mm256_xor_ps(a, b); }
inline int mm256_testz_si256(const __m256i& a, const __m256i& b) noexcept { return _mm256_testz_si256(a, b); }
inline int mm256_testc_si256(const __m256i& a, const __m256i& b) noexcept { return _mm256_testc_si256(a, b); }
inline int mm256_testnzc_si256(const __m256i& a, const __m256i& b) noexcept { return _mm256_testnzc_si256(a, b); }
inline int mm256_testz_pd(const __m256d& a, const __m256d& b) noexcept { return _mm256_testz_pd(a, b); }
inline int mm256_testc_pd(const __m256d& a, const __m256d& b) noexcept { return _mm256_testc_pd(a, b); }
inline int mm256_testnzc_pd(const __m256d& a, const __m256d& b) noexcept { return _mm256_testnzc_pd(a, b); }
inline int mm_testz_pd(const __m128d& a, const __m128d& b) noexcept { return _mm_testz_pd(a, b); }
inline int mm_testc_pd(const __m128d& a, const __m128d& b) noexcept { return _mm_testc_pd(a, b); }
inline int mm_testnzc_pd(const __m128d& a, const __m128d& b) noexcept { return _mm_testnzc_pd(a, b); }
inline int mm256_testz_ps(const __m256& a, const __m256& b) noexcept { return _mm256_testz_ps(a, b); }
inline int mm256_testc_ps(const __m256& a, const __m256& b) noexcept { return _mm256_testc_ps(a, b); }
inline int mm256_testnzc_ps(const __m256& a, const __m256& b) noexcept { return _mm256_testnzc_ps(a, b); }
inline int mm_testz_ps(const __m128& a, const __m128& b) noexcept { return _mm_testz_ps(a, b); }
inline int mm_testc_ps(const __m128& a, const __m128& b) noexcept { return _mm_testc_ps(a, b); }
inline int mm_testnzc_ps(const __m128& a, const __m128& b) noexcept { return _mm_testnzc_ps(a, b); }
template<int i> __m256d mm256_blend_pd(const __m256d& a, const __m256d& b) noexcept { return _mm256_blend_pd(a, b, i); }
template<int i> __m256 mm256_blend_ps(const __m256& a, const __m256& b) noexcept { return _mm256_blend_ps(a, b, i); }
inline __m256d mm256_blendv_pd(const __m256d& a, const __m256d& b, const __m256d& c) noexcept { return _mm256_blendv_pd(a, b, c); }
inline __m256 mm256_blendv_ps(const __m256& a, const __m256& b, const __m256& c) noexcept { return _mm256_blendv_ps(a, b, c); }
template<int i> __m256d mm256_shuffle_pd(const __m256d& a, const __m256d& b) noexcept { return _mm256_shuffle_pd(a, b, i); }
template<int i> __m256 mm256_shuffle_ps(const __m256& a, const __m256& b) noexcept { return _mm256_shuffle_ps(a, b, i); }
template<int i> __m128 mm256_extractf128_ps(const __m256& a) noexcept { return _mm256_extractf128_ps(a, i); }
template<int i> __m128d mm256_extractf128_pd(const __m256d& a) noexcept { return _mm256_extractf128_pd(a, i); }
template<int i> __m128i mm256_extractf128_si256(const __m256i& a) noexcept { return _mm256_extractf128_si256(a, i); }
template<int i> __int32 mm256_extract_epi32(const __m256i& a) noexcept { return _mm256_extract_epi32(a, i); }
template<int i> __int64 mm256_extract_epi64(const __m256i& a) noexcept { return _mm256_extract_epi64(a, i); }
inline __m256 mm256_permutevar_ps(const __m256& a, const __m256i& b) noexcept { return _mm256_permutevar_ps(a, b); }
inline __m128 mm_permutevar_ps(const __m128& a, const __m128i& b) noexcept { return _mm_permutevar_ps(a, b); }
template<int i> __m256 mm256_permute_ps(const __m256& a) noexcept { return _mm256_permute_ps(a, i); }
template<int i> __m128 mm_permute_ps(const __m128& a) noexcept { return _mm_permute_ps(a, i); }
inline __m256d mm256_permutevar_pd(const __m256d& a, const __m256i& b) noexcept { return _mm256_permutevar_pd(a, b); }
inline __m128d mm_permutevar_pd(const __m128d& a, const __m128i& b) noexcept { return _mm_permutevar_pd(a, b); }
template<int i> __m256d mm256_permute_pd(const __m256d& a) noexcept { return _mm256_permute_pd(a, i); }
template<int i> __m128d mm_permute_pd(const __m128d& a) noexcept { return _mm_permute_pd(a, i); }
template<int i> __m256 mm256_permute2f128_ps(const __m256& a, const __m256& b) noexcept { return _mm256_permute2f128_ps(a, b, i); }
template<int i> __m256d mm256_permute2f128_pd(const __m256d& a, const __m256d& b) noexcept { return _mm256_permute2f128_pd(a, b, i); }
template<int i> __m256i mm256_permute2f128_si256(const __m256i& a, const __m256i& b) noexcept { return _mm256_permute2f128_si256(a, b, i); }
template<int i> __m256 mm256_insertf128_ps(const __m256& a, const __m128& b) noexcept { return _mm256_insertf128_ps(a, b, i); }
template<int i> __m256d mm256_insertf128_pd(const __m256d& a, const __m128d& b) noexcept { return _mm256_insertf128_pd(a, b, i); }
template<int i> __m256i mm256_insertf128_si256(const __m256i& a, const __m128i& b) noexcept { return _mm256_insertf128_si256(a, b, i); }
template<int i> __m256i mm256_insert_epi8(const __m256i& a, __int8 b) noexcept { return _mm256_insert_epi8(a, b, i); }
template<int i> __m256i mm256_insert_epi16(const __m256i& a, __int16 b) noexcept { return _mm256_insert_epi16(a, b, i); }
template<int i> __m256i mm256_insert_epi32(const __m256i& a, __int32 b) noexcept { return _mm256_insert_epi32(a, b, i); }
template<int i> __m256i mm256_insert_epi64(const __m256i& a, __int64 b) noexcept { return _mm256_insert_epi64(a, b, i); }
inline __m256d mm256_unpackhi_pd(const __m256d& a, const __m256d& b) noexcept { return _mm256_unpackhi_pd(a, b); }
inline __m256 mm256_unpackhi_ps(const __m256& a, const __m256& b) noexcept { return _mm256_unpackhi_ps(a, b); }
inline __m256d mm256_unpacklo_pd(const __m256d& a, const __m256d& b) noexcept { return _mm256_unpacklo_pd(a, b); }
inline __m256 mm256_unpacklo_ps(const __m256& a, const __m256& b) noexcept { return _mm256_unpacklo_ps(a, b); }
inline __m256d mm256_max_pd(const __m256d& a, const __m256d& b) noexcept { return _mm256_max_pd(a, b); }
inline __m256 mm256_max_ps(const __m256& a, const __m256& b) noexcept { return _mm256_max_ps(a, b); }
inline __m256d mm256_min_pd(const __m256d& a, const __m256d& b) noexcept { return _mm256_min_pd(a, b); }
inline __m256 mm256_min_ps(const __m256& a, const __m256& b) noexcept { return _mm256_min_ps(a, b); }
template<int i> __m256d mm256_round_pd(const __m256d& a) noexcept { return _mm256_round_pd(a, i); }
template<int i> __m256 mm256_round_ps(const __m256& a) noexcept { return _mm256_round_ps(a, i); }
inline __m256 mm256_floor_ps(const __m256& a) noexcept { return _mm256_floor_ps(a); }
inline __m256 mm256_ceil_ps(const __m256& a) noexcept { return _mm256_ceil_ps(a); }
inline __m256d mm256_floor_pd(const __m256d& a) noexcept { return _mm256_floor_pd(a); }
inline __m256d mm256_ceil_pd(const __m256d& a) noexcept { return _mm256_ceil_pd(a); }
template<int i> __m128d mm_cmp_pd(const __m128d& a, const __m128d& b) noexcept { return _mm_cmp_pd(a, b, i); }
template<int i> __m256d mm256_cmp_pd(const __m256d& a, const __m256d& b) noexcept { return _mm256_cmp_pd(a, b, i); }
template<int i> __m128 mm_cmp_ps(const __m128& a, const __m128& b) noexcept { return _mm_cmp_ps(a, b, i); }
template<int i> __m256 mm256_cmp_ps(const __m256& a, const __m256& b) noexcept { return _mm256_cmp_ps(a, b, i); }
template<int i> __m128d mm_cmp_sd(const __m128d& a, const __m128d& b) noexcept { return _mm_cmp_sd(a, b, i); }
template<int i> __m128 mm_cmp_ss(const __m128& a, const __m128& b) noexcept { return _mm_cmp_ss(a, b, i); }
inline __m256d mm256_cvtepi32_pd(const __m128i& a) noexcept { return _mm256_cvtepi32_pd(a); }
inline __m256 mm256_cvtepi32_ps(const __m256i& a) noexcept { return _mm256_cvtepi32_ps(a); }
inline __m128 mm256_cvtpd_ps(const __m256d& a) noexcept { return _mm256_cvtpd_ps(a); }
inline __m256i mm256_cvtps_epi32(const __m256& a) noexcept { return _mm256_cvtps_epi32(a); }
inline __m256d mm256_cvtps_pd(const __m128& a) noexcept { return _mm256_cvtps_pd(a); }
inline __m128i mm256_cvttpd_epi32(const __m256d& a) noexcept { return _mm256_cvttpd_epi32(a); }
inline __m128i mm256_cvtpd_epi32(const __m256d& a) noexcept { return _mm256_cvtpd_epi32(a); }
inline __m256i mm256_cvttps_epi32(const __m256& a) noexcept { return _mm256_cvttps_epi32(a); }
inline float mm256_cvtss_f32(const __m256& a) noexcept { return _mm256_cvtss_f32(a); }
inline double mm256_cvtsd_f64(const __m256d& a) noexcept { return _mm256_cvtsd_f64(a); }
inline int mm256_cvtsi256_si32(const __m256i& a) noexcept { return _mm256_cvtsi256_si32(a); }
inline void mm256_zeroall() noexcept { _mm256_zeroall(); }
inline void mm256_zeroupper() noexcept { _mm256_zeroupper(); }
inline __m256 mm256_undefined_ps() noexcept { return _mm256_undefined_ps(); }
inline __m256d mm256_undefined_pd() noexcept { return _mm256_undefined_pd(); }
inline __m256i mm256_undefined_si256() noexcept { return _mm256_undefined_si256(); }
inline __m256 mm256_broadcast_ss(float const* a) noexcept { return _mm256_broadcast_ss(a); }
inline __m128 mm_broadcast_ss(float const* a) noexcept { return _mm_broadcast_ss(a); }
inline __m256d mm256_broadcast_sd(double const* a) noexcept { return _mm256_broadcast_sd(a); }
inline __m256 mm256_broadcast_ps(__m128 const* a) noexcept { return _mm256_broadcast_ps(a); }
inline __m256d mm256_broadcast_pd(__m128d const* a) noexcept { return _mm256_broadcast_pd(a); }
inline __m256d mm256_load_pd(double const* a) noexcept { return _mm256_load_pd(a); }
inline __m256 mm256_load_ps(float const* a) noexcept { return _mm256_load_ps(a); }
inline __m256d mm256_loadu_pd(double const* a) noexcept { return _mm256_loadu_pd(a); }
inline __m256 mm256_loadu_ps(float const* a) noexcept { return _mm256_loadu_ps(a); }
inline __m256i mm256_load_si256(__m256i const* a) noexcept { return _mm256_load_si256(a); }
inline __m256i mm256_loadu_si256(__m256i const* a) noexcept { return _mm256_loadu_si256(a); }
inline __m256d mm256_maskload_pd(double const* a, const __m256i& b) noexcept { return _mm256_maskload_pd(a, b); }
inline __m128d mm_maskload_pd(double const* a, const __m128i& b) noexcept { return _mm_maskload_pd(a, b); }
inline __m256 mm256_maskload_ps(float const* a, const __m256i& b) noexcept { return _mm256_maskload_ps(a, b); }
inline __m128 mm_maskload_ps(float const* a, const __m128i& b) noexcept { return _mm_maskload_ps(a, b); }
inline __m256i mm256_lddqu_si256(__m256i const* a) noexcept { return _mm256_lddqu_si256(a); }
inline __m256 mm256_loadu2_m128(float const* a, float const* b) noexcept { return _mm256_loadu2_m128(a, b); }
inline __m256d mm256_loadu2_m128d(double const* a, double const* b) noexcept { return _mm256_loadu2_m128d(a, b); }
inline __m256i mm256_loadu2_m128i(__m128i const* a, __m128i const* b) noexcept { return _mm256_loadu2_m128i(a, b); }
inline void mm256_store_pd(double* a, const __m256d& b) noexcept { _mm256_store_pd(a, b); }
inline void mm256_store_ps(float* a, const __m256& b) noexcept { _mm256_store_ps(a, b); }
inline void mm256_storeu_pd(double* a, const __m256d& b) noexcept { _mm256_storeu_pd(a, b); }
inline void mm256_storeu_ps(float* a, const __m256& b) noexcept { _mm256_storeu_ps(a, b); }
inline void mm256_store_si256(__m256i* a, const __m256i& b) noexcept { _mm256_store_si256(a, b); }
inline void mm256_storeu_si256(__m256i* a, const __m256i& b) noexcept { _mm256_storeu_si256(a, b); }
inline void mm256_maskstore_pd(double* a, const __m256i& b, const __m256d& c) noexcept { _mm256_maskstore_pd(a, b, c); }
inline void mm_maskstore_pd(double* a, const __m128i& b, const __m128d& c) noexcept { _mm_maskstore_pd(a, b, c); }
inline void mm256_maskstore_ps(float* a, const __m256i& b, const __m256& c) noexcept { _mm256_maskstore_ps(a, b, c); }
inline void mm_maskstore_ps(float* a, const __m128i& b, const __m128& c) noexcept { _mm_maskstore_ps(a, b, c); }
inline void mm256_stream_si256(void* a, const __m256i& b) noexcept { _mm256_stream_si256((__m256i*)a, b); }
inline void mm256_stream_pd(void* a, const __m256d& b) noexcept { _mm256_stream_pd((double*)a, b); }
inline void mm256_stream_ps(void* a, const __m256& b) noexcept { _mm256_stream_ps((float*)a, b); }
inline void mm256_storeu2_m128(float* a, float* b, const __m256& c) noexcept { _mm256_storeu2_m128(a, b, c); }
inline void mm256_storeu2_m128d(double* a, double* b, const __m256d& c) noexcept { _mm256_storeu2_m128d(a, b, c); }
inline void mm256_storeu2_m128i(__m128i* a, __m128i* b, const __m256i& c) noexcept { _mm256_storeu2_m128i(a, b, c); }
inline __m256 mm256_movehdup_ps(const __m256& a) noexcept { return _mm256_movehdup_ps(a); }
inline __m256 mm256_moveldup_ps(const __m256& a) noexcept { return _mm256_moveldup_ps(a); }
inline __m256d mm256_movedup_pd(const __m256d& a) noexcept { return _mm256_movedup_pd(a); }
inline __m256 mm256_rcp_ps(const __m256& a) noexcept { return _mm256_rcp_ps(a); }
inline __m256 mm256_rsqrt_ps(const __m256& a) noexcept { return _mm256_rsqrt_ps(a); }
inline __m256d mm256_sqrt_pd(const __m256d& a) noexcept { return _mm256_sqrt_pd(a); }
inline __m256 mm256_sqrt_ps(const __m256& a) noexcept { return _mm256_sqrt_ps(a); }
inline int mm256_movemask_pd(const __m256d& a) noexcept { return _mm256_movemask_pd(a); }
inline int mm256_movemask_ps(const __m256& a) noexcept { return _mm256_movemask_ps(a); }
inline __m256d mm256_setzero_pd() noexcept { return _mm256_setzero_pd(); }
inline __m256 mm256_setzero_ps() noexcept { return _mm256_setzero_ps(); }
inline __m256i mm256_setzero_si256() noexcept { return _mm256_setzero_si256(); }
inline __m256d mm256_set_pd(double e3, double e2, double e1, double e0) noexcept { return _mm256_set_pd(e3, e2, e1, e0); }
inline __m256 mm256_set_ps(float e7, float e6, float e5, float e4, float e3, float e2, float e1, float e0) noexcept { //
  return _mm256_set_ps(e7, e6, e5, e4, e3, e2, e1, e0);
}
inline __m256i mm256_set_epi8(char e31, char e30, char e29, char e28, char e27, char e26, char e25, char e24, //
                              char e23, char e22, char e21, char e20, char e19, char e18, char e17, char e16, //
                              char e15, char e14, char e13, char e12, char e11, char e10, char e9, char e8,   //
                              char e7, char e6, char e5, char e4, char e3, char e2, char e1, char e0) noexcept {
  return _mm256_set_epi8(e31, e30, e29, e28, e27, e26, e25, e24, e23, e22, e21, e20, e19, e18, e17, e16, //
                         e15, e14, e13, e12, e11, e10, e9, e8, e7, e6, e5, e4, e3, e2, e1, e0);
}
inline __m256i mm256_set_epi16(short e15, short e14, short e13, short e12, short e11, short e10, short e9, short e8, //
                               short e7, short e6, short e5, short e4, short e3, short e2, short e1, short e0) noexcept {
  return _mm256_set_epi16(e15, e14, e13, e12, e11, e10, e9, e8, e7, e6, e5, e4, e3, e2, e1, e0);
}
template<int i> __m256i mm256_set_epi32(int e7, int e6, int e5, int e4, int e3, int e2, int e1, int e0) noexcept { //
  return _mm256_set_epi32(e7, e6, e5, e4, e3, e2, e1, e0, i);
}
inline __m256i mm256_set_epi64(__int64 e3, __int64 e2, __int64 e1, __int64 e0) noexcept { return _mm256_set_epi64x(e3, e2, e1, e0); }
inline __m256i mm256_set_epi64x(__int64 e3, __int64 e2, __int64 e1, __int64 e0) noexcept { return _mm256_set_epi64x(e3, e2, e1, e0); }
inline __m256d mm256_setr_pd(double e0, double e1, double e2, double e3) noexcept { return _mm256_setr_pd(e0, e1, e2, e3); }
inline __m256 mm256_setr_ps(float e0, float e1, float e2, float e3, float e4, float e5, float e6, float e7) noexcept { //
  return _mm256_setr_ps(e0, e1, e2, e3, e4, e5, e6, e7);
}
inline __m256i mm256_setr_epi8(char e0, char e1, char e2, char e3, char e4, char e5, char e6, char e7,         //
                               char e8, char e9, char e10, char e11, char e12, char e13, char e14, char e15,   //
                               char e16, char e17, char e18, char e19, char e20, char e21, char e22, char e23, //
                               char e24, char e25, char e26, char e27, char e28, char e29, char e30, char e31) noexcept {
  return _mm256_setr_epi8(e0, e1, e2, e3, e4, e5, e6, e7, e8, e9, e10, e11, e12, e13, e14, e15, //
                          e16, e17, e18, e19, e20, e21, e22, e23, e24, e25, e26, e27, e28, e29, e30, e31);
}
inline __m256i mm256_setr_epi16(short e0, short e1, short e2, short e3, short e4, short e5, short e6, short e7, //
                                short e8, short e9, short e10, short e11, short e12, short e13, short e14, short e15) noexcept {
  return _mm256_setr_epi16(e0, e1, e2, e3, e4, e5, e6, e7, e8, e9, e10, e11, e12, e13, e14, e15);
}
template<int i> __m256i mm256_setr_epi32(int e0, int e1, int e2, int e3) noexcept { return _mm256_setr_epi32(e0, e1, e2, e3, i); }
inline __m256i mm256_setr_epi64x(__int64 e0, __int64 e1, __int64 e2, __int64 e3) noexcept { return _mm256_setr_epi64x(e0, e1, e2, e3); }

inline __m256d mm256_set1_pd(double a) noexcept { return _mm256_set1_pd(a); }
inline __m256 mm256_set1_ps(float a) noexcept { return _mm256_set1_ps(a); }
inline __m256i mm256_set1_epi8(char a) noexcept { return _mm256_set1_epi8(a); }
inline __m256i mm256_set1_epi16(short a) noexcept { return _mm256_set1_epi16(a); }
template<int i> __m256i mm256_set1_epi32() noexcept { return _mm256_set1_epi32(i); }
inline __m256i mm256_set1_epi64x(long long a) noexcept { return _mm256_set1_epi64x(a); }
inline __m256 mm256_set_m128(const __m128& a, const __m128& b) noexcept { return _mm256_set_m128(a, b); }
inline __m256d mm256_set_m128d(const __m128d& a, const __m128d& b) noexcept { return _mm256_set_m128d(a, b); }
inline __m256i mm256_set_m128i(const __m128i& a, const __m128i& b) noexcept { return _mm256_set_m128i(a, b); }
inline __m256 mm256_setr_m128(const __m128& a, const __m128& b) noexcept { return _mm256_setr_m128(a, b); }
inline __m256d mm256_setr_m128d(const __m128d& a, const __m128d& b) noexcept { return _mm256_setr_m128d(a, b); }
inline __m256i mm256_setr_m128i(const __m128i& a, const __m128i& b) noexcept { return _mm256_setr_m128i(a, b); }
inline __m256 mm256_castpd_ps(const __m256d& a) noexcept { return _mm256_castpd_ps(a); }
inline __m256d mm256_castps_pd(const __m256& a) noexcept { return _mm256_castps_pd(a); }
inline __m256i mm256_castps_si256(const __m256& a) noexcept { return _mm256_castps_si256(a); }
inline __m256i mm256_castpd_si256(const __m256d& a) noexcept { return _mm256_castpd_si256(a); }
inline __m256 mm256_castsi256_ps(const __m256i& a) noexcept { return _mm256_castsi256_ps(a); }
inline __m256d mm256_castsi256_pd(const __m256i& a) noexcept { return _mm256_castsi256_pd(a); }
inline __m128 mm256_castps256_ps128(const __m256& a) noexcept { return _mm256_castps256_ps128(a); }
inline __m128d mm256_castpd256_pd128(const __m256d& a) noexcept { return _mm256_castpd256_pd128(a); }
inline __m128i mm256_castsi256_si128(const __m256i& a) noexcept { return _mm256_castsi256_si128(a); }
inline __m256 mm256_castps128_ps256(const __m128& a) noexcept { return _mm256_castps128_ps256(a); }
inline __m256d mm256_castpd128_pd256(const __m128d& a) noexcept { return _mm256_castpd128_pd256(a); }
inline __m256i mm256_castsi128_si256(const __m128i& a) noexcept { return _mm256_castsi128_si256(a); }
inline __m256 mm256_zextps128_ps256(const __m128& a) noexcept { return _mm256_zextps128_ps256(a); }
inline __m256d mm256_zextpd128_pd256(const __m128d& a) noexcept { return _mm256_zextpd128_pd256(a); }
inline __m256i mm256_zextsi128_si256(const __m128i& a) noexcept { return _mm256_zextsi128_si256(a); }
template<int i> int mm256_extract_epi8(const __m256i& a) noexcept { return _mm256_extract_epi8(a, i); }
template<int i> int mm256_extract_epi16(const __m256i& a) noexcept { return _mm256_extract_epi16(a, i); }
template<int i> __m256i mm256_blend_epi16(const __m256i& a, const __m256i& b) noexcept { return _mm256_blend_epi16(a, b, i); }
template<int i> __m128i mm_blend_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_blend_epi32(a, b, i); }
template<int i> __m256i mm256_blend_epi32(const __m256i& a, const __m256i& b) noexcept { return _mm256_blend_epi32(a, b, i); }
inline __m256i mm256_blendv_epi8(const __m256i& a, const __m256i& b, const __m256i& c) noexcept { return _mm256_blendv_epi8(a, b, c); }
inline __m128i mm_broadcastb_epi8(const __m128i& a) noexcept { return _mm_broadcastb_epi8(a); }
inline __m256i mm256_broadcastb_epi8(const __m128i& a) noexcept { return _mm256_broadcastb_epi8(a); }
inline __m128i mm_broadcastd_epi32(const __m128i& a) noexcept { return _mm_broadcastd_epi32(a); }
inline __m256i mm256_broadcastd_epi32(const __m128i& a) noexcept { return _mm256_broadcastd_epi32(a); }
inline __m128i mm_broadcastq_epi64(const __m128i& a) noexcept { return _mm_broadcastq_epi64(a); }
inline __m256i mm256_broadcastq_epi64(const __m128i& a) noexcept { return _mm256_broadcastq_epi64(a); }
inline __m128d mm_broadcastsd_pd(const __m128d& a) noexcept { return _mm_broadcastsd_pd(a); }
inline __m256d mm256_broadcastsd_pd(const __m128d& a) noexcept { return _mm256_broadcastsd_pd(a); }
inline __m256i mm_broadcastsi128_si256(const __m128i& a) noexcept { return _mm256_broadcastsi128_si256(a); }
inline __m256i mm256_broadcastsi128_si256(const __m128i& a) noexcept { return _mm256_broadcastsi128_si256(a); }
inline __m128 mm_broadcastss_ps(const __m128& a) noexcept { return _mm_broadcastss_ps(a); }
inline __m256 mm256_broadcastss_ps(const __m128& a) noexcept { return _mm256_broadcastss_ps(a); }
inline __m128i mm_broadcastw_epi16(const __m128i& a) noexcept { return _mm_broadcastw_epi16(a); }
inline __m256i mm256_broadcastw_epi16(const __m128i& a) noexcept { return _mm256_broadcastw_epi16(a); }
template<int i> __m128i mm256_extracti128_si256(const __m256i& a) noexcept { return _mm256_extracti128_si256(a, i); }
template<int i> __m256i mm256_inserti128_si256(const __m256i& a, const __m128i& b) noexcept { return _mm256_inserti128_si256(a, b, i); }
template<int i> __m256i mm256_permute2x128_si256(const __m256i& a, const __m256i& b) noexcept { return _mm256_permute2x128_si256(a, b, i); }
template<int i> __m256i mm256_permute4x64_epi64(const __m256i& a) noexcept { return _mm256_permute4x64_epi64(a, i); }
template<int i> __m256d mm256_permute4x64_pd(const __m256d& a) noexcept { return _mm256_permute4x64_pd(a, i); }
inline __m256i mm256_permutevar8x32_epi32(const __m256i& a, const __m256i& b) noexcept { return _mm256_permutevar8x32_epi32(a, b); }
inline __m256 mm256_permutevar8x32_ps(const __m256& a, const __m256i& b) noexcept { return _mm256_permutevar8x32_ps(a, b); }
template<int i> __m256i mm256_shuffle_epi32(const __m256i& a) noexcept { return _mm256_shuffle_epi32(a, i); }
inline __m256i mm256_shuffle_epi8(const __m256i& a, const __m256i& b) noexcept { return _mm256_shuffle_epi8(a, b); }
template<int i> __m256i mm256_shufflehi_epi16(const __m256i& a) noexcept { return _mm256_shufflehi_epi16(a, i); }
template<int i> __m256i mm256_shufflelo_epi16(const __m256i& a) noexcept { return _mm256_shufflelo_epi16(a, i); }
inline __m256i mm256_unpackhi_epi8(const __m256i& a, const __m256i& b) noexcept { return _mm256_unpackhi_epi8(a, b); }
inline __m256i mm256_unpackhi_epi16(const __m256i& a, const __m256i& b) noexcept { return _mm256_unpackhi_epi16(a, b); }
inline __m256i mm256_unpackhi_epi32(const __m256i& a, const __m256i& b) noexcept { return _mm256_unpackhi_epi32(a, b); }
inline __m256i mm256_unpackhi_epi64(const __m256i& a, const __m256i& b) noexcept { return _mm256_unpackhi_epi64(a, b); }
inline __m256i mm256_unpacklo_epi8(const __m256i& a, const __m256i& b) noexcept { return _mm256_unpacklo_epi8(a, b); }
inline __m256i mm256_unpacklo_epi16(const __m256i& a, const __m256i& b) noexcept { return _mm256_unpacklo_epi16(a, b); }
inline __m256i mm256_unpacklo_epi32(const __m256i& a, const __m256i& b) noexcept { return _mm256_unpacklo_epi32(a, b); }
inline __m256i mm256_unpacklo_epi64(const __m256i& a, const __m256i& b) noexcept { return _mm256_unpacklo_epi64(a, b); }
inline __m256i mm256_abs_epi8(const __m256i& a) noexcept { return _mm256_abs_epi8(a); }
inline __m256i mm256_abs_epi16(const __m256i& a) noexcept { return _mm256_abs_epi16(a); }
inline __m256i mm256_abs_epi32(const __m256i& a) noexcept { return _mm256_abs_epi32(a); }
inline __m256i mm256_max_epi8(const __m256i& a, const __m256i& b) noexcept { return _mm256_max_epi8(a, b); }
inline __m256i mm256_max_epi16(const __m256i& a, const __m256i& b) noexcept { return _mm256_max_epi16(a, b); }
inline __m256i mm256_max_epi32(const __m256i& a, const __m256i& b) noexcept { return _mm256_max_epi32(a, b); }
inline __m256i mm256_max_epu8(const __m256i& a, const __m256i& b) noexcept { return _mm256_max_epu8(a, b); }
inline __m256i mm256_max_epu16(const __m256i& a, const __m256i& b) noexcept { return _mm256_max_epu16(a, b); }
inline __m256i mm256_max_epu32(const __m256i& a, const __m256i& b) noexcept { return _mm256_max_epu32(a, b); }
inline __m256i mm256_min_epi8(const __m256i& a, const __m256i& b) noexcept { return _mm256_min_epi8(a, b); }
inline __m256i mm256_min_epi16(const __m256i& a, const __m256i& b) noexcept { return _mm256_min_epi16(a, b); }
inline __m256i mm256_min_epi32(const __m256i& a, const __m256i& b) noexcept { return _mm256_min_epi32(a, b); }
inline __m256i mm256_min_epu8(const __m256i& a, const __m256i& b) noexcept { return _mm256_min_epu8(a, b); }
inline __m256i mm256_min_epu16(const __m256i& a, const __m256i& b) noexcept { return _mm256_min_epu16(a, b); }
inline __m256i mm256_min_epu32(const __m256i& a, const __m256i& b) noexcept { return _mm256_min_epu32(a, b); }
inline __m256i mm256_add_epi8(const __m256i& a, const __m256i& b) noexcept { return _mm256_add_epi8(a, b); }
inline __m256i mm256_add_epi16(const __m256i& a, const __m256i& b) noexcept { return _mm256_add_epi16(a, b); }
inline __m256i mm256_add_epi32(const __m256i& a, const __m256i& b) noexcept { return _mm256_add_epi32(a, b); }
inline __m256i mm256_add_epi64(const __m256i& a, const __m256i& b) noexcept { return _mm256_add_epi64(a, b); }
inline __m256i mm256_adds_epi8(const __m256i& a, const __m256i& b) noexcept { return _mm256_adds_epi8(a, b); }
inline __m256i mm256_adds_epi16(const __m256i& a, const __m256i& b) noexcept { return _mm256_adds_epi16(a, b); }
inline __m256i mm256_adds_epu8(const __m256i& a, const __m256i& b) noexcept { return _mm256_adds_epu8(a, b); }
inline __m256i mm256_adds_epu16(const __m256i& a, const __m256i& b) noexcept { return _mm256_adds_epu16(a, b); }
inline __m256i mm256_hadd_epi16(const __m256i& a, const __m256i& b) noexcept { return _mm256_hadd_epi16(a, b); }
inline __m256i mm256_hadd_epi32(const __m256i& a, const __m256i& b) noexcept { return _mm256_hadd_epi32(a, b); }
inline __m256i mm256_hadds_epi16(const __m256i& a, const __m256i& b) noexcept { return _mm256_hadds_epi16(a, b); }
inline __m256i mm256_hsub_epi16(const __m256i& a, const __m256i& b) noexcept { return _mm256_hsub_epi16(a, b); }
inline __m256i mm256_hsub_epi32(const __m256i& a, const __m256i& b) noexcept { return _mm256_hsub_epi32(a, b); }
inline __m256i mm256_hsubs_epi16(const __m256i& a, const __m256i& b) noexcept { return _mm256_hsubs_epi16(a, b); }
inline __m256i mm256_madd_epi16(const __m256i& a, const __m256i& b) noexcept { return _mm256_madd_epi16(a, b); }
inline __m256i mm256_maddubs_epi16(const __m256i& a, const __m256i& b) noexcept { return _mm256_maddubs_epi16(a, b); }
inline __m256i mm256_mul_epi32(const __m256i& a, const __m256i& b) noexcept { return _mm256_mul_epi32(a, b); }
inline __m256i mm256_mul_epu32(const __m256i& a, const __m256i& b) noexcept { return _mm256_mul_epu32(a, b); }
inline __m256i mm256_mulhi_epi16(const __m256i& a, const __m256i& b) noexcept { return _mm256_mulhi_epi16(a, b); }
inline __m256i mm256_mulhi_epu16(const __m256i& a, const __m256i& b) noexcept { return _mm256_mulhi_epu16(a, b); }
inline __m256i mm256_mulhrs_epi16(const __m256i& a, const __m256i& b) noexcept { return _mm256_mulhrs_epi16(a, b); }
inline __m256i mm256_mullo_epi16(const __m256i& a, const __m256i& b) noexcept { return _mm256_mullo_epi16(a, b); }
inline __m256i mm256_mullo_epi32(const __m256i& a, const __m256i& b) noexcept { return _mm256_mullo_epi32(a, b); }
inline __m256i mm256_sad_epu8(const __m256i& a, const __m256i& b) noexcept { return _mm256_sad_epu8(a, b); }
inline __m256i mm256_sign_epi8(const __m256i& a, const __m256i& b) noexcept { return _mm256_sign_epi8(a, b); }
inline __m256i mm256_sign_epi16(const __m256i& a, const __m256i& b) noexcept { return _mm256_sign_epi16(a, b); }
inline __m256i mm256_sign_epi32(const __m256i& a, const __m256i& b) noexcept { return _mm256_sign_epi32(a, b); }
inline __m256i mm256_sub_epi8(const __m256i& a, const __m256i& b) noexcept { return _mm256_sub_epi8(a, b); }
inline __m256i mm256_sub_epi16(const __m256i& a, const __m256i& b) noexcept { return _mm256_sub_epi16(a, b); }
inline __m256i mm256_sub_epi32(const __m256i& a, const __m256i& b) noexcept { return _mm256_sub_epi32(a, b); }
inline __m256i mm256_sub_epi64(const __m256i& a, const __m256i& b) noexcept { return _mm256_sub_epi64(a, b); }
inline __m256i mm256_subs_epi8(const __m256i& a, const __m256i& b) noexcept { return _mm256_subs_epi8(a, b); }
inline __m256i mm256_subs_epi16(const __m256i& a, const __m256i& b) noexcept { return _mm256_subs_epi16(a, b); }
inline __m256i mm256_subs_epu8(const __m256i& a, const __m256i& b) noexcept { return _mm256_subs_epu8(a, b); }
inline __m256i mm256_subs_epu16(const __m256i& a, const __m256i& b) noexcept { return _mm256_subs_epu16(a, b); }
template<int i> __m256i mm256_alignr_epi8(const __m256i& a, const __m256i& b) noexcept { return _mm256_alignr_epi8(a, b, i); }
inline int mm256_movemask_epi8(const __m256i& a) noexcept { return _mm256_movemask_epi8(a); }
template<int i> __m256i mm256_mpsadbw_epu8(const __m256i& a, const __m256i& b) noexcept { return _mm256_mpsadbw_epu8(a, b, i); }
inline __m256i mm256_packs_epi16(const __m256i& a, const __m256i& b) noexcept { return _mm256_packs_epi16(a, b); }
inline __m256i mm256_packs_epi32(const __m256i& a, const __m256i& b) noexcept { return _mm256_packs_epi32(a, b); }
inline __m256i mm256_packus_epi16(const __m256i& a, const __m256i& b) noexcept { return _mm256_packus_epi16(a, b); }
inline __m256i mm256_packus_epi32(const __m256i& a, const __m256i& b) noexcept { return _mm256_packus_epi32(a, b); }
inline __m256i mm256_and_si256(const __m256i& a, const __m256i& b) noexcept { return _mm256_and_si256(a, b); }
inline __m256i mm256_andnot_si256(const __m256i& a, const __m256i& b) noexcept { return _mm256_andnot_si256(a, b); }
inline __m256i mm256_or_si256(const __m256i& a, const __m256i& b) noexcept { return _mm256_or_si256(a, b); }
inline __m256i mm256_xor_si256(const __m256i& a, const __m256i& b) noexcept { return _mm256_xor_si256(a, b); }
inline __m256i mm256_avg_epu8(const __m256i& a, const __m256i& b) noexcept { return _mm256_avg_epu8(a, b); }
inline __m256i mm256_avg_epu16(const __m256i& a, const __m256i& b) noexcept { return _mm256_avg_epu16(a, b); }
inline __m256i mm256_cmpeq_epi8(const __m256i& a, const __m256i& b) noexcept { return _mm256_cmpeq_epi8(a, b); }
inline __m256i mm256_cmpeq_epi16(const __m256i& a, const __m256i& b) noexcept { return _mm256_cmpeq_epi16(a, b); }
inline __m256i mm256_cmpeq_epi32(const __m256i& a, const __m256i& b) noexcept { return _mm256_cmpeq_epi32(a, b); }
inline __m256i mm256_cmpeq_epi64(const __m256i& a, const __m256i& b) noexcept { return _mm256_cmpeq_epi64(a, b); }
inline __m256i mm256_cmpgt_epi8(const __m256i& a, const __m256i& b) noexcept { return _mm256_cmpgt_epi8(a, b); }
inline __m256i mm256_cmpgt_epi16(const __m256i& a, const __m256i& b) noexcept { return _mm256_cmpgt_epi16(a, b); }
inline __m256i mm256_cmpgt_epi32(const __m256i& a, const __m256i& b) noexcept { return _mm256_cmpgt_epi32(a, b); }
inline __m256i mm256_cmpgt_epi64(const __m256i& a, const __m256i& b) noexcept { return _mm256_cmpgt_epi64(a, b); }
inline __m256i mm256_cvtepi16_epi32(const __m128i& a) noexcept { return _mm256_cvtepi16_epi32(a); }
inline __m256i mm256_cvtepi16_epi64(const __m128i& a) noexcept { return _mm256_cvtepi16_epi64(a); }
inline __m256i mm256_cvtepi32_epi64(const __m128i& a) noexcept { return _mm256_cvtepi32_epi64(a); }
inline __m256i mm256_cvtepi8_epi16(const __m128i& a) noexcept { return _mm256_cvtepi8_epi16(a); }
inline __m256i mm256_cvtepi8_epi32(const __m128i& a) noexcept { return _mm256_cvtepi8_epi32(a); }
inline __m256i mm256_cvtepi8_epi64(const __m128i& a) noexcept { return _mm256_cvtepi8_epi64(a); }
inline __m256i mm256_cvtepu16_epi32(const __m128i& a) noexcept { return _mm256_cvtepu16_epi32(a); }
inline __m256i mm256_cvtepu16_epi64(const __m128i& a) noexcept { return _mm256_cvtepu16_epi64(a); }
inline __m256i mm256_cvtepu32_epi64(const __m128i& a) noexcept { return _mm256_cvtepu32_epi64(a); }
inline __m256i mm256_cvtepu8_epi16(const __m128i& a) noexcept { return _mm256_cvtepu8_epi16(a); }
inline __m256i mm256_cvtepu8_epi32(const __m128i& a) noexcept { return _mm256_cvtepu8_epi32(a); }
inline __m256i mm256_cvtepu8_epi64(const __m128i& a) noexcept { return _mm256_cvtepu8_epi64(a); }
template<int i> __m128d mm_i32gather_pd(double const* a, const __m128i& b) noexcept { return _mm_i32gather_pd(a, b, i); }
template<int i> __m256d mm256_i32gather_pd(double const* a, const __m128i& b) noexcept { return _mm256_i32gather_pd(a, b, i); }
template<int i> __m128 mm_i32gather_ps(float const* a, const __m128i& b) noexcept { return _mm_i32gather_ps(a, b, i); }
template<int i> __m256 mm256_i32gather_ps(float const* a, const __m256i& b) noexcept { return _mm256_i32gather_ps(a, b, i); }
template<int i> __m128i mm_i32gather_epi32(int const* a, const __m128i& b) noexcept { return _mm_i32gather_epi32(a, b, i); }
template<int i> __m256i mm256_i32gather_epi32(int const* a, const __m256i& b) noexcept { return _mm256_i32gather_epi32(a, b, i); }
template<int i> __m128i mm_i32gather_epi64(__int64 const* a, const __m128i& b) noexcept { return _mm_i32gather_epi64(a, b, i); }
template<int i> __m256i mm256_i32gather_epi64(__int64 const* a, const __m128i& b) noexcept { return _mm256_i32gather_epi64(a, b, i); }
template<int i> __m128d mm_i64gather_pd(double const* a, const __m128i& b) noexcept { return _mm_i64gather_pd(a, b, i); }
template<int i> __m256d mm256_i64gather_pd(double const* a, const __m256i& b) noexcept { return _mm256_i64gather_pd(a, b, i); }
template<int i> __m128 mm_i64gather_ps(float const* a, const __m128i& b) noexcept { return _mm_i64gather_ps(a, b, i); }
template<int i> __m128 mm256_i64gather_ps(float const* a, const __m256i& b) noexcept { return _mm256_i64gather_ps(a, b, i); }
template<int i> __m128i mm_i64gather_epi32(int const* a, const __m128i& b) noexcept { return _mm_i64gather_epi32(a, b, i); }
template<int i> __m128i mm256_i64gather_epi32(int const* a, const __m256i& b) noexcept { return _mm256_i64gather_epi32(a, b, i); }
template<int i> __m128i mm_i64gather_epi64(__int64 const* a, const __m128i& b) noexcept { return _mm_i64gather_epi64(a, b, i); }
template<int i> __m256i mm256_i64gather_epi64(__int64 const* a, const __m256i& b) noexcept { return _mm256_i64gather_epi64(a, b, i); }
template<int i> __m128d mm_mask_i32gather_pd(const __m128d& a, double const* b, const __m128i& c, const __m128d& d) noexcept { return _mm_mask_i32gather_pd(a, b, c, d, i); }
template<int i> __m256d mm256_mask_i32gather_pd(const __m256d& a, double const* b, const __m128i& c, const __m256d& d) noexcept { return _mm256_mask_i32gather_pd(a, b, c, d, i); }
template<int i> __m128 mm_mask_i32gather_ps(const __m128& a, float const* b, const __m128i& c, const __m128& d) noexcept { return _mm_mask_i32gather_ps(a, b, c, d, i); }
template<int i> __m256 mm256_mask_i32gather_ps(const __m256& a, float const* b, const __m256i& c, const __m256& d) noexcept { return _mm256_mask_i32gather_ps(a, b, c, d, i); }
template<int i> __m128i mm_mask_i32gather_epi32(const __m128i& a, int const* b, const __m128i& c, const __m128i& d) noexcept { return _mm_mask_i32gather_epi32(a, b, c, d, i); }
template<int i> __m256i mm256_mask_i32gather_epi32(const __m256i& a, int const* b, const __m256i& c, const __m256i& d) noexcept { return _mm256_mask_i32gather_epi32(a, b, c, d, i); }
template<int i> __m128i mm_mask_i32gather_epi64(const __m128i& a, __int64 const* b, const __m128i& c, const __m128i& d) noexcept { return _mm_mask_i32gather_epi64(a, b, c, d, i); }
template<int i> __m256i mm256_mask_i32gather_epi64(const __m256i& a, __int64 const* b, const __m128i& c, const __m256i& d) noexcept { return _mm256_mask_i32gather_epi64(a, b, c, d, i); }
template<int i> __m128d mm_mask_i64gather_pd(const __m128d& a, double const* b, const __m128i& c, const __m128d& d) noexcept { return _mm_mask_i64gather_pd(a, b, c, d, i); }
template<int i> __m256d mm256_mask_i64gather_pd(const __m256d& a, double const* b, const __m256i& c, const __m256d& d) noexcept { return _mm256_mask_i64gather_pd(a, b, c, d, i); }
template<int i> __m128 mm_mask_i64gather_ps(const __m128& a, float const* b, const __m128i& c, const __m128& d) noexcept { return _mm_mask_i64gather_ps(a, b, c, d, i); }
template<int i> __m128 mm256_mask_i64gather_ps(const __m128& a, float const* b, const __m256i& c, const __m128& d) noexcept { return _mm256_mask_i64gather_ps(a, b, c, d, i); }
template<int i> __m128i mm_mask_i64gather_epi32(const __m128i& a, int const* b, const __m128i& c, const __m128i& d) noexcept { return _mm_mask_i64gather_epi32(a, b, c, d, i); }
template<int i> __m128i mm256_mask_i64gather_epi32(const __m128i& a, int const* b, const __m256i& c, const __m128i& d) noexcept { return _mm256_mask_i64gather_epi32(a, b, c, d, i); }
template<int i> __m128i mm_mask_i64gather_epi64(const __m128i& a, __int64 const* b, const __m128i& c, const __m128i& d) noexcept { return _mm_mask_i64gather_epi64(a, b, c, d, i); }
template<int i> __m256i mm256_mask_i64gather_epi64(const __m256i& a, __int64 const* b, const __m256i& c, const __m256i& d) noexcept { return _mm256_mask_i64gather_epi64(a, b, c, d, i); }
inline __m128i mm_maskload_epi32(int const* a, const __m128i& b) noexcept { return _mm_maskload_epi32(a, b); }
inline __m256i mm256_maskload_epi32(int const* a, const __m256i& b) noexcept { return _mm256_maskload_epi32(a, b); }
inline __m128i mm_maskload_epi64(__int64 const* a, const __m128i& b) noexcept { return _mm_maskload_epi64(a, b); }
inline __m256i mm256_maskload_epi64(__int64 const* a, const __m256i& b) noexcept { return _mm256_maskload_epi64(a, b); }
inline __m256i mm256_stream_load_si256(void const* a) noexcept { return _mm256_stream_load_si256((const __m256i*)a); }
inline void mm_maskstore_epi32(int* a, const __m128i& b, const __m128i& c) noexcept { _mm_maskstore_epi32(a, b, c); }
inline void mm256_maskstore_epi32(int* a, const __m256i& b, const __m256i& c) noexcept { _mm256_maskstore_epi32(a, b, c); }
inline void mm_maskstore_epi64(__int64* a, const __m128i& b, const __m128i& c) noexcept { _mm_maskstore_epi64(a, b, c); }
inline void mm256_maskstore_epi64(__int64* a, const __m256i& b, const __m256i& c) noexcept { _mm256_maskstore_epi64(a, b, c); }
template<int i> __m256i mm256_slli_si256(const __m256i& a) noexcept { return _mm256_slli_si256(a, i); }
template<int i> __m256i mm256_bslli_epi128(const __m256i& a) noexcept { return _mm256_bslli_epi128(a, i); }
inline __m256i mm256_sll_epi16(const __m256i& a, const __m128i& b) noexcept { return _mm256_sll_epi16(a, b); }
template<int i> __m256i mm256_slli_epi16(const __m256i& a) noexcept { return _mm256_slli_epi16(a, i); }
inline __m256i mm256_sll_epi32(const __m256i& a, const __m128i& b) noexcept { return _mm256_sll_epi32(a, b); }
template<int i> __m256i mm256_slli_epi32(const __m256i& a) noexcept { return _mm256_slli_epi32(a, i); }
inline __m256i mm256_sll_epi64(const __m256i& a, const __m128i& b) noexcept { return _mm256_sll_epi64(a, b); }
template<int i> __m256i mm256_slli_epi64(const __m256i& a) noexcept { return _mm256_slli_epi64(a, i); }
inline __m128i mm_sllv_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_sllv_epi32(a, b); }
inline __m256i mm256_sllv_epi32(const __m256i& a, const __m256i& b) noexcept { return _mm256_sllv_epi32(a, b); }
inline __m128i mm_sllv_epi64(const __m128i& a, const __m128i& b) noexcept { return _mm_sllv_epi64(a, b); }
inline __m256i mm256_sllv_epi64(const __m256i& a, const __m256i& b) noexcept { return _mm256_sllv_epi64(a, b); }
inline __m256i mm256_sra_epi16(const __m256i& a, const __m128i& b) noexcept { return _mm256_sra_epi16(a, b); }
template<int i> __m256i mm256_srai_epi16(const __m256i& a) noexcept { return _mm256_srai_epi16(a, i); }
inline __m256i mm256_sra_epi32(const __m256i& a, const __m128i& b) noexcept { return _mm256_sra_epi32(a, b); }
template<int i> __m256i mm256_srai_epi32(const __m256i& a) noexcept { return _mm256_srai_epi32(a, i); }
inline __m128i mm_srav_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_srav_epi32(a, b); }
inline __m256i mm256_srav_epi32(const __m256i& a, const __m256i& b) noexcept { return _mm256_srav_epi32(a, b); }
template<int i> __m256i mm256_srli_si256(const __m256i& a) noexcept { return _mm256_srli_si256(a, i); }
template<int i> __m256i mm256_bsrli_epi128(const __m256i& a) noexcept { return _mm256_bsrli_epi128(a, i); }
inline __m256i mm256_srl_epi16(const __m256i& a, const __m128i& b) noexcept { return _mm256_srl_epi16(a, b); }
template<int i> __m256i mm256_srli_epi16(const __m256i& a) noexcept { return _mm256_srli_epi16(a, i); }
inline __m256i mm256_srl_epi32(const __m256i& a, const __m128i& b) noexcept { return _mm256_srl_epi32(a, b); }
template<int i> __m256i mm256_srli_epi32(const __m256i& a) noexcept { return _mm256_srli_epi32(a, i); }
inline __m256i mm256_srl_epi64(const __m256i& a, const __m128i& b) noexcept { return _mm256_srl_epi64(a, b); }
template<int i> __m256i mm256_srli_epi64(const __m256i& a) noexcept { return _mm256_srli_epi64(a, i); }
inline __m128i mm_srlv_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_srlv_epi32(a, b); }
inline __m256i mm256_srlv_epi32(const __m256i& a, const __m256i& b) noexcept { return _mm256_srlv_epi32(a, b); }
inline __m128i mm_srlv_epi64(const __m128i& a, const __m128i& b) noexcept { return _mm_srlv_epi64(a, b); }
inline __m256i mm256_srlv_epi64(const __m256i& a, const __m256i& b) noexcept { return _mm256_srlv_epi64(a, b); }
inline __m256i mm256_madd52hi_avx_epu64(const __m256i& a, const __m256i& b, const __m256i& c) noexcept { return _mm256_madd52hi_avx_epu64(a, b, c); }
inline __m256i mm256_madd52lo_avx_epu64(const __m256i& a, const __m256i& b, const __m256i& c) noexcept { return _mm256_madd52lo_avx_epu64(a, b, c); }
inline __m128i mm_madd52hi_avx_epu64(const __m128i& a, const __m128i& b, const __m128i& c) noexcept { return _mm_madd52hi_avx_epu64(a, b, c); }
inline __m128i mm_madd52lo_avx_epu64(const __m128i& a, const __m128i& b, const __m128i& c) noexcept { return _mm_madd52lo_avx_epu64(a, b, c); }
inline __m256i mm256_madd52hi_epu64(const __m256i& a, const __m256i& b, const __m256i& c) noexcept { return _mm256_madd52hi_epu64(a, b, c); }
inline __m256i mm256_madd52lo_epu64(const __m256i& a, const __m256i& b, const __m256i& c) noexcept { return _mm256_madd52lo_epu64(a, b, c); }
inline __m128i mm_madd52hi_epu64(const __m128i& a, const __m128i& b, const __m128i& c) noexcept { return _mm_madd52hi_epu64(a, b, c); }
inline __m128i mm_madd52lo_epu64(const __m128i& a, const __m128i& b, const __m128i& c) noexcept { return _mm_madd52lo_epu64(a, b, c); }
inline unsigned int mm_crc32_u8(unsigned int a, unsigned char b) noexcept { return _mm_crc32_u8(a, b); }
inline unsigned int mm_crc32_u16(unsigned int a, unsigned short b) noexcept { return _mm_crc32_u16(a, b); }
inline unsigned int mm_crc32_u32(unsigned int a, unsigned int b) noexcept { return _mm_crc32_u32(a, b); }
inline unsigned __int64 mm_crc32_u64(unsigned __int64 a, unsigned __int64 b) noexcept { return _mm_crc32_u64(a, b); }
inline __m256 mm256_cvtph_ps(const __m128i& a) noexcept { return _mm256_cvtph_ps(a); }
template<int i> __m128i mm256_cvtps_ph(const __m256& a) noexcept { return _mm256_cvtps_ph(a, i); }
inline __m128 mm_cvtph_ps(const __m128i& a) noexcept { return _mm_cvtph_ps(a); }
template<int i> __m128i mm_cvtps_ph(const __m128& a) noexcept { return _mm_cvtps_ph(a, i); }
inline __m128d mm_fmadd_pd(const __m128d& a, const __m128d& b, const __m128d& c) noexcept { return _mm_fmadd_pd(a, b, c); }
inline __m256d mm256_fmadd_pd(const __m256d& a, const __m256d& b, const __m256d& c) noexcept { return _mm256_fmadd_pd(a, b, c); }
inline __m128 mm_fmadd_ps(const __m128& a, const __m128& b, const __m128& c) noexcept { return _mm_fmadd_ps(a, b, c); }
inline __m256 mm256_fmadd_ps(const __m256& a, const __m256& b, const __m256& c) noexcept { return _mm256_fmadd_ps(a, b, c); }
inline __m128d mm_fmadd_sd(const __m128d& a, const __m128d& b, const __m128d& c) noexcept { return _mm_fmadd_sd(a, b, c); }
inline __m128 mm_fmadd_ss(const __m128& a, const __m128& b, const __m128& c) noexcept { return _mm_fmadd_ss(a, b, c); }
inline __m128d mm_fmaddsub_pd(const __m128d& a, const __m128d& b, const __m128d& c) noexcept { return _mm_fmaddsub_pd(a, b, c); }
inline __m256d mm256_fmaddsub_pd(const __m256d& a, const __m256d& b, const __m256d& c) noexcept { return _mm256_fmaddsub_pd(a, b, c); }
inline __m128 mm_fmaddsub_ps(const __m128& a, const __m128& b, const __m128& c) noexcept { return _mm_fmaddsub_ps(a, b, c); }
inline __m256 mm256_fmaddsub_ps(const __m256& a, const __m256& b, const __m256& c) noexcept { return _mm256_fmaddsub_ps(a, b, c); }
inline __m128d mm_fmsub_pd(const __m128d& a, const __m128d& b, const __m128d& c) noexcept { return _mm_fmsub_pd(a, b, c); }
inline __m256d mm256_fmsub_pd(const __m256d& a, const __m256d& b, const __m256d& c) noexcept { return _mm256_fmsub_pd(a, b, c); }
inline __m128 mm_fmsub_ps(const __m128& a, const __m128& b, const __m128& c) noexcept { return _mm_fmsub_ps(a, b, c); }
inline __m256 mm256_fmsub_ps(const __m256& a, const __m256& b, const __m256& c) noexcept { return _mm256_fmsub_ps(a, b, c); }
inline __m128d mm_fmsub_sd(const __m128d& a, const __m128d& b, const __m128d& c) noexcept { return _mm_fmsub_sd(a, b, c); }
inline __m128 mm_fmsub_ss(const __m128& a, const __m128& b, const __m128& c) noexcept { return _mm_fmsub_ss(a, b, c); }
inline __m128d mm_fmsubadd_pd(const __m128d& a, const __m128d& b, const __m128d& c) noexcept { return _mm_fmsubadd_pd(a, b, c); }
inline __m256d mm256_fmsubadd_pd(const __m256d& a, const __m256d& b, const __m256d& c) noexcept { return _mm256_fmsubadd_pd(a, b, c); }
inline __m128 mm_fmsubadd_ps(const __m128& a, const __m128& b, const __m128& c) noexcept { return _mm_fmsubadd_ps(a, b, c); }
inline __m256 mm256_fmsubadd_ps(const __m256& a, const __m256& b, const __m256& c) noexcept { return _mm256_fmsubadd_ps(a, b, c); }
inline __m128d mm_fnmadd_pd(const __m128d& a, const __m128d& b, const __m128d& c) noexcept { return _mm_fnmadd_pd(a, b, c); }
inline __m256d mm256_fnmadd_pd(const __m256d& a, const __m256d& b, const __m256d& c) noexcept { return _mm256_fnmadd_pd(a, b, c); }
inline __m128 mm_fnmadd_ps(const __m128& a, const __m128& b, const __m128& c) noexcept { return _mm_fnmadd_ps(a, b, c); }
inline __m256 mm256_fnmadd_ps(const __m256& a, const __m256& b, const __m256& c) noexcept { return _mm256_fnmadd_ps(a, b, c); }
inline __m128d mm_fnmadd_sd(const __m128d& a, const __m128d& b, const __m128d& c) noexcept { return _mm_fnmadd_sd(a, b, c); }
inline __m128 mm_fnmadd_ss(const __m128& a, const __m128& b, const __m128& c) noexcept { return _mm_fnmadd_ss(a, b, c); }
inline __m128d mm_fnmsub_pd(const __m128d& a, const __m128d& b, const __m128d& c) noexcept { return _mm_fnmsub_pd(a, b, c); }
inline __m256d mm256_fnmsub_pd(const __m256d& a, const __m256d& b, const __m256d& c) noexcept { return _mm256_fnmsub_pd(a, b, c); }
inline __m128 mm_fnmsub_ps(const __m128& a, const __m128& b, const __m128& c) noexcept { return _mm_fnmsub_ps(a, b, c); }
inline __m256 mm256_fnmsub_ps(const __m256& a, const __m256& b, const __m256& c) noexcept { return _mm256_fnmsub_ps(a, b, c); }
inline __m128d mm_fnmsub_sd(const __m128d& a, const __m128d& b, const __m128d& c) noexcept { return _mm_fnmsub_sd(a, b, c); }
inline __m128 mm_fnmsub_ss(const __m128& a, const __m128& b, const __m128& c) noexcept { return _mm_fnmsub_ss(a, b, c); }
inline __m512i mm512_maskz_gf2p8mul_epi8(const __mmask64& a, const __m512i& b, const __m512i& c) noexcept { return _mm512_maskz_gf2p8mul_epi8(a, b, c); }
inline __m512i mm512_mask_gf2p8mul_epi8(const __m512i& a, const __mmask64& b, const __m512i& c, const __m512i& d) noexcept { return _mm512_mask_gf2p8mul_epi8(a, b, c, d); }
inline __m512i mm512_gf2p8mul_epi8(const __m512i& a, const __m512i& b) noexcept { return _mm512_gf2p8mul_epi8(a, b); }
template<int i> __m512i mm512_maskz_gf2p8affine_epi64_epi8(const __mmask64& a, const __m512i& b, const __m512i& c) noexcept { return _mm512_maskz_gf2p8affine_epi64_epi8(a, b, c, i); }
template<int i> __m512i mm512_mask_gf2p8affine_epi64_epi8(const __m512i& a, const __mmask64& b, const __m512i& c, const __m512i& d) noexcept { return _mm512_mask_gf2p8affine_epi64_epi8(a, b, c, d, i); }
template<int i> __m512i mm512_gf2p8affine_epi64_epi8(const __m512i& a, const __m512i& b) noexcept { return _mm512_gf2p8affine_epi64_epi8(a, b, i); }
template<int i> __m512i mm512_maskz_gf2p8affineinv_epi64_epi8(const __mmask64& a, const __m512i& b, const __m512i& c) noexcept { return _mm512_maskz_gf2p8affineinv_epi64_epi8(a, b, c, i); }
template<int i> __m512i mm512_mask_gf2p8affineinv_epi64_epi8(const __m512i& a, const __mmask64& b, const __m512i& c, const __m512i& d) noexcept { return _mm512_mask_gf2p8affineinv_epi64_epi8(a, b, c, d, i); }
template<int i> __m512i mm512_gf2p8affineinv_epi64_epi8(const __m512i& a, const __m512i& b) noexcept { return _mm512_gf2p8affineinv_epi64_epi8(a, b, i); }
inline __m256i mm256_maskz_gf2p8mul_epi8(const __mmask32& a, const __m256i& b, const __m256i& c) noexcept { return _mm256_maskz_gf2p8mul_epi8(a, b, c); }
inline __m256i mm256_mask_gf2p8mul_epi8(const __m256i& a, const __mmask32& b, const __m256i& c, const __m256i& d) noexcept { return _mm256_mask_gf2p8mul_epi8(a, b, c, d); }
inline __m256i mm256_gf2p8mul_epi8(const __m256i& a, const __m256i& b) noexcept { return _mm256_gf2p8mul_epi8(a, b); }
inline __m128i mm_maskz_gf2p8mul_epi8(const __mmask16& a, const __m128i& b, const __m128i& c) noexcept { return _mm_maskz_gf2p8mul_epi8(a, b, c); }
inline __m128i mm_mask_gf2p8mul_epi8(const __m128i& a, const __mmask16& b, const __m128i& c, const __m128i& d) noexcept { return _mm_mask_gf2p8mul_epi8(a, b, c, d); }
inline __m128i mm_gf2p8mul_epi8(const __m128i& a, const __m128i& b) noexcept { return _mm_gf2p8mul_epi8(a, b); }
template<int i> __m256i mm256_maskz_gf2p8affine_epi64_epi8(const __mmask32& a, const __m256i& b, const __m256i& c) noexcept { return _mm256_maskz_gf2p8affine_epi64_epi8(a, b, c, i); }
template<int i> __m256i mm256_mask_gf2p8affine_epi64_epi8(const __m256i& a, const __mmask32& b, const __m256i& c, const __m256i& d) noexcept { return _mm256_mask_gf2p8affine_epi64_epi8(a, b, c, d, i); }
template<int i> __m256i mm256_gf2p8affine_epi64_epi8(const __m256i& a, const __m256i& b) noexcept { return _mm256_gf2p8affine_epi64_epi8(a, b, i); }
template<int i> __m128i mm_maskz_gf2p8affine_epi64_epi8(const __mmask16& a, const __m128i& b, const __m128i& c) noexcept { return _mm_maskz_gf2p8affine_epi64_epi8(a, b, c, i); }
template<int i> __m128i mm_mask_gf2p8affine_epi64_epi8(const __m128i& a, const __mmask16& b, const __m128i& c, const __m128i& d) noexcept { return _mm_mask_gf2p8affine_epi64_epi8(a, b, c, d, i); }
template<int i> __m128i mm_gf2p8affine_epi64_epi8(const __m128i& a, const __m128i& b) noexcept { return _mm_gf2p8affine_epi64_epi8(a, b, i); }
template<int i> __m256i mm256_maskz_gf2p8affineinv_epi64_epi8(const __mmask32& a, const __m256i& b, const __m256i& c) noexcept { return _mm256_maskz_gf2p8affineinv_epi64_epi8(a, b, c, i); }
template<int i> __m256i mm256_mask_gf2p8affineinv_epi64_epi8(const __m256i& a, const __mmask32& b, const __m256i& c, const __m256i& d) noexcept { return _mm256_mask_gf2p8affineinv_epi64_epi8(a, b, c, d, i); }
template<int i> __m256i mm256_gf2p8affineinv_epi64_epi8(const __m256i& a, const __m256i& b) noexcept { return _mm256_gf2p8affineinv_epi64_epi8(a, b, i); }
template<int i> __m128i mm_maskz_gf2p8affineinv_epi64_epi8(const __mmask16& a, const __m128i& b, const __m128i& c) noexcept { return _mm_maskz_gf2p8affineinv_epi64_epi8(a, b, c, i); }
template<int i> __m128i mm_mask_gf2p8affineinv_epi64_epi8(const __m128i& a, const __mmask16& b, const __m128i& c, const __m128i& d) noexcept { return _mm_mask_gf2p8affineinv_epi64_epi8(a, b, c, d, i); }
template<int i> __m128i mm_gf2p8affineinv_epi64_epi8(const __m128i& a, const __m128i& b) noexcept { return _mm_gf2p8affineinv_epi64_epi8(a, b, i); }
inline unsigned char mm_aesdec128kl_u8(__m128i* a, const __m128i& b, const void* c) noexcept { return _mm_aesdec128kl_u8(a, b, c); }
inline unsigned char mm_aesdec256kl_u8(__m128i* a, const __m128i& b, const void* c) noexcept { return _mm_aesdec256kl_u8(a, b, c); }
inline unsigned char mm_aesenc128kl_u8(__m128i* a, const __m128i& b, const void* c) noexcept { return _mm_aesenc128kl_u8(a, b, c); }
inline unsigned char mm_aesenc256kl_u8(__m128i* a, const __m128i& b, const void* c) noexcept { return _mm_aesenc256kl_u8(a, b, c); }
inline unsigned int mm_encodekey128_u32(unsigned int a, const __m128i& b, void* c) noexcept { return _mm_encodekey128_u32(a, b, c); }
inline unsigned int mm_encodekey256_u32(unsigned int a, const __m128i& b, const __m128i& c, void* d) noexcept { return _mm_encodekey256_u32(a, b, c, d); }
inline void mm_loadiwkey(unsigned int a, const __m128i& b, const __m128i& c, const __m128i& d) noexcept { _mm_loadiwkey(a, b, c, d); }
inline unsigned char mm_aesdecwide128kl_u8(__m128i* a, const __m128i* b, const void* c) noexcept { return _mm_aesdecwide128kl_u8(a, b, c); }
inline unsigned char mm_aesdecwide256kl_u8(__m128i* a, const __m128i* b, const void* c) noexcept { return _mm_aesdecwide256kl_u8(a, b, c); }
inline unsigned char mm_aesencwide128kl_u8(__m128i* a, const __m128i* b, const void* c) noexcept { return _mm_aesencwide128kl_u8(a, b, c); }
inline unsigned char mm_aesencwide256kl_u8(__m128i* a, const __m128i* b, const void* c) noexcept { return _mm_aesencwide256kl_u8(a, b, c); }
inline void mm_monitor(void const* a, unsigned b, unsigned c) noexcept { _mm_monitor(a, b, c); }
inline void mm_mwait(unsigned a, unsigned b) noexcept { _mm_mwait(a, b); }
template<int i> __m128i mm_clmulepi64_si128(const __m128i& a, const __m128i& b) noexcept { return _mm_clmulepi64_si128(a, b, i); }
inline int mm_popcnt_u32(unsigned int a) noexcept { return _mm_popcnt_u32(a); }
inline __int64 mm_popcnt_u64(unsigned __int64 a) noexcept { return _mm_popcnt_u64(a); }
inline __m128i mm_sha1msg1_epu32(const __m128i& a, const __m128i& b) noexcept { return _mm_sha1msg1_epu32(a, b); }
inline __m128i mm_sha1msg2_epu32(const __m128i& a, const __m128i& b) noexcept { return _mm_sha1msg2_epu32(a, b); }
inline __m128i mm_sha1nexte_epu32(const __m128i& a, const __m128i& b) noexcept { return _mm_sha1nexte_epu32(a, b); }
template<int i> __m128i mm_sha1rnds4_epu32(const __m128i& a, const __m128i& b) noexcept { return _mm_sha1rnds4_epu32(a, b, i); }
inline __m128i mm_sha256msg1_epu32(const __m128i& a, const __m128i& b) noexcept { return _mm_sha256msg1_epu32(a, b); }
inline __m128i mm_sha256msg2_epu32(const __m128i& a, const __m128i& b) noexcept { return _mm_sha256msg2_epu32(a, b); }
inline __m128i mm_sha256rnds2_epu32(const __m128i& a, const __m128i& b, const __m128i& c) noexcept { return _mm_sha256rnds2_epu32(a, b, c); }
inline __m128d mm_acos_pd(const __m128d& a) noexcept { return _mm_acos_pd(a); }
inline __m128 mm_acos_ps(const __m128& a) noexcept { return _mm_acos_ps(a); }
inline __m128d mm_acosh_pd(const __m128d& a) noexcept { return _mm_acosh_pd(a); }
inline __m128 mm_acosh_ps(const __m128& a) noexcept { return _mm_acosh_ps(a); }
inline __m128d mm_asin_pd(const __m128d& a) noexcept { return _mm_asin_pd(a); }
inline __m128 mm_asin_ps(const __m128& a) noexcept { return _mm_asin_ps(a); }
inline __m128d mm_asinh_pd(const __m128d& a) noexcept { return _mm_asinh_pd(a); }
inline __m128 mm_asinh_ps(const __m128& a) noexcept { return _mm_asinh_ps(a); }
inline __m128d mm_atan_pd(const __m128d& a) noexcept { return _mm_atan_pd(a); }
inline __m128 mm_atan_ps(const __m128& a) noexcept { return _mm_atan_ps(a); }
inline __m128d mm_atan2_pd(const __m128d& a, const __m128d& b) noexcept { return _mm_atan2_pd(a, b); }
inline __m128 mm_atan2_ps(const __m128& a, const __m128& b) noexcept { return _mm_atan2_ps(a, b); }
inline __m128d mm_atanh_pd(const __m128d& a) noexcept { return _mm_atanh_pd(a); }
inline __m128 mm_atanh_ps(const __m128& a) noexcept { return _mm_atanh_ps(a); }
inline __m128d mm_cos_pd(const __m128d& a) noexcept { return _mm_cos_pd(a); }
inline __m128 mm_cos_ps(const __m128& a) noexcept { return _mm_cos_ps(a); }
inline __m128d mm_cosd_pd(const __m128d& a) noexcept { return _mm_cosd_pd(a); }
inline __m128 mm_cosd_ps(const __m128& a) noexcept { return _mm_cosd_ps(a); }
inline __m128d mm_cosh_pd(const __m128d& a) noexcept { return _mm_cosh_pd(a); }
inline __m128 mm_cosh_ps(const __m128& a) noexcept { return _mm_cosh_ps(a); }
inline __m128d mm_hypot_pd(const __m128d& a, const __m128d& b) noexcept { return _mm_hypot_pd(a, b); }
inline __m128 mm_hypot_ps(const __m128& a, const __m128& b) noexcept { return _mm_hypot_ps(a, b); }
inline __m128d mm_sin_pd(const __m128d& a) noexcept { return _mm_sin_pd(a); }
inline __m128 mm_sin_ps(const __m128& a) noexcept { return _mm_sin_ps(a); }
inline __m128d mm_sincos_pd(__m128d* a, const __m128d& b) noexcept { return _mm_sincos_pd(a, b); }
inline __m128 mm_sincos_ps(__m128* a, const __m128& b) noexcept { return _mm_sincos_ps(a, b); }
inline __m128d mm_sind_pd(const __m128d& a) noexcept { return _mm_sind_pd(a); }
inline __m128 mm_sind_ps(const __m128& a) noexcept { return _mm_sind_ps(a); }
inline __m128d mm_sinh_pd(const __m128d& a) noexcept { return _mm_sinh_pd(a); }
inline __m128 mm_sinh_ps(const __m128& a) noexcept { return _mm_sinh_ps(a); }
inline __m128d mm_tan_pd(const __m128d& a) noexcept { return _mm_tan_pd(a); }
inline __m128 mm_tan_ps(const __m128& a) noexcept { return _mm_tan_ps(a); }
inline __m128d mm_tand_pd(const __m128d& a) noexcept { return _mm_tand_pd(a); }
inline __m128 mm_tand_ps(const __m128& a) noexcept { return _mm_tand_ps(a); }
inline __m128d mm_tanh_pd(const __m128d& a) noexcept { return _mm_tanh_pd(a); }
inline __m128 mm_tanh_ps(const __m128& a) noexcept { return _mm_tanh_ps(a); }
inline __m128d mm_cbrt_pd(const __m128d& a) noexcept { return _mm_cbrt_pd(a); }
inline __m128 mm_cbrt_ps(const __m128& a) noexcept { return _mm_cbrt_ps(a); }
inline __m128 mm_cexp_ps(const __m128& a) noexcept { return _mm_cexp_ps(a); }
inline __m128 mm_clog_ps(const __m128& a) noexcept { return _mm_clog_ps(a); }
inline __m128 mm_csqrt_ps(const __m128& a) noexcept { return _mm_csqrt_ps(a); }
inline __m128d mm_exp_pd(const __m128d& a) noexcept { return _mm_exp_pd(a); }
inline __m128 mm_exp_ps(const __m128& a) noexcept { return _mm_exp_ps(a); }
inline __m128d mm_exp10_pd(const __m128d& a) noexcept { return _mm_exp10_pd(a); }
inline __m128 mm_exp10_ps(const __m128& a) noexcept { return _mm_exp10_ps(a); }
inline __m128d mm_exp2_pd(const __m128d& a) noexcept { return _mm_exp2_pd(a); }
inline __m128 mm_exp2_ps(const __m128& a) noexcept { return _mm_exp2_ps(a); }
inline __m128d mm_expm1_pd(const __m128d& a) noexcept { return _mm_expm1_pd(a); }
inline __m128 mm_expm1_ps(const __m128& a) noexcept { return _mm_expm1_ps(a); }
inline __m128d mm_invcbrt_pd(const __m128d& a) noexcept { return _mm_invcbrt_pd(a); }
inline __m128 mm_invcbrt_ps(const __m128& a) noexcept { return _mm_invcbrt_ps(a); }
inline __m128d mm_invsqrt_pd(const __m128d& a) noexcept { return _mm_invsqrt_pd(a); }
inline __m128 mm_invsqrt_ps(const __m128& a) noexcept { return _mm_invsqrt_ps(a); }
inline __m128d mm_log_pd(const __m128d& a) noexcept { return _mm_log_pd(a); }
inline __m128 mm_log_ps(const __m128& a) noexcept { return _mm_log_ps(a); }
inline __m128d mm_log10_pd(const __m128d& a) noexcept { return _mm_log10_pd(a); }
inline __m128 mm_log10_ps(const __m128& a) noexcept { return _mm_log10_ps(a); }
inline __m128d mm_log1p_pd(const __m128d& a) noexcept { return _mm_log1p_pd(a); }
inline __m128 mm_log1p_ps(const __m128& a) noexcept { return _mm_log1p_ps(a); }
inline __m128d mm_log2_pd(const __m128d& a) noexcept { return _mm_log2_pd(a); }
inline __m128 mm_log2_ps(const __m128& a) noexcept { return _mm_log2_ps(a); }
inline __m128d mm_logb_pd(const __m128d& a) noexcept { return _mm_logb_pd(a); }
inline __m128 mm_logb_ps(const __m128& a) noexcept { return _mm_logb_ps(a); }
inline __m128d mm_pow_pd(const __m128d& a, const __m128d& b) noexcept { return _mm_pow_pd(a, b); }
inline __m128 mm_pow_ps(const __m128& a, const __m128& b) noexcept { return _mm_pow_ps(a, b); }
inline __m128d mm_svml_sqrt_pd(const __m128d& a) noexcept { return _mm_svml_sqrt_pd(a); }
inline __m128 mm_svml_sqrt_ps(const __m128& a) noexcept { return _mm_svml_sqrt_ps(a); }
inline __m128d mm_cdfnorm_pd(const __m128d& a) noexcept { return _mm_cdfnorm_pd(a); }
inline __m128 mm_cdfnorm_ps(const __m128& a) noexcept { return _mm_cdfnorm_ps(a); }
inline __m128d mm_cdfnorminv_pd(const __m128d& a) noexcept { return _mm_cdfnorminv_pd(a); }
inline __m128 mm_cdfnorminv_ps(const __m128& a) noexcept { return _mm_cdfnorminv_ps(a); }
inline __m128 mm_erf_ps(const __m128& a) noexcept { return _mm_erf_ps(a); }
inline __m128d mm_erfc_pd(const __m128d& a) noexcept { return _mm_erfc_pd(a); }
inline __m128 mm_erfc_ps(const __m128& a) noexcept { return _mm_erfc_ps(a); }
inline __m128d mm_erfcinv_pd(const __m128d& a) noexcept { return _mm_erfcinv_pd(a); }
inline __m128 mm_erfcinv_ps(const __m128& a) noexcept { return _mm_erfcinv_ps(a); }
inline __m128d mm_erfinv_pd(const __m128d& a) noexcept { return _mm_erfinv_pd(a); }
inline __m128 mm_erfinv_ps(const __m128& a) noexcept { return _mm_erfinv_ps(a); }
inline __m128i mm_div_epi8(const __m128i& a, const __m128i& b) noexcept { return _mm_div_epi8(a, b); }
inline __m128i mm_div_epi16(const __m128i& a, const __m128i& b) noexcept { return _mm_div_epi16(a, b); }
inline __m128i mm_div_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_div_epi32(a, b); }
inline __m128i mm_div_epi64(const __m128i& a, const __m128i& b) noexcept { return _mm_div_epi64(a, b); }
inline __m128i mm_div_epu8(const __m128i& a, const __m128i& b) noexcept { return _mm_div_epu8(a, b); }
inline __m128i mm_div_epu16(const __m128i& a, const __m128i& b) noexcept { return _mm_div_epu16(a, b); }
inline __m128i mm_div_epu32(const __m128i& a, const __m128i& b) noexcept { return _mm_div_epu32(a, b); }
inline __m128i mm_div_epu64(const __m128i& a, const __m128i& b) noexcept { return _mm_div_epu64(a, b); }
inline __m128d mm_erf_pd(const __m128d& a) noexcept { return _mm_erf_pd(a); }
inline __m128i mm_idiv_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_idiv_epi32(a, b); }
inline __m128i mm_idivrem_epi32(__m128i* a, const __m128i& b, const __m128i& c) noexcept { return _mm_idivrem_epi32(a, b, c); }
inline __m128i mm_irem_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_irem_epi32(a, b); }
inline __m128i mm_rem_epi8(const __m128i& a, const __m128i& b) noexcept { return _mm_rem_epi8(a, b); }
inline __m128i mm_rem_epi16(const __m128i& a, const __m128i& b) noexcept { return _mm_rem_epi16(a, b); }
inline __m128i mm_rem_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_rem_epi32(a, b); }
inline __m128i mm_rem_epi64(const __m128i& a, const __m128i& b) noexcept { return _mm_rem_epi64(a, b); }
inline __m128i mm_rem_epu8(const __m128i& a, const __m128i& b) noexcept { return _mm_rem_epu8(a, b); }
inline __m128i mm_rem_epu16(const __m128i& a, const __m128i& b) noexcept { return _mm_rem_epu16(a, b); }
inline __m128i mm_rem_epu32(const __m128i& a, const __m128i& b) noexcept { return _mm_rem_epu32(a, b); }
inline __m128i mm_rem_epu64(const __m128i& a, const __m128i& b) noexcept { return _mm_rem_epu64(a, b); }
inline __m128i mm_udiv_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_udiv_epi32(a, b); }
inline __m128i mm_udivrem_epi32(__m128i* a, const __m128i& b, const __m128i& c) noexcept { return _mm_udivrem_epi32(a, b, c); }
inline __m128i mm_urem_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_urem_epi32(a, b); }
inline __m128d mm_svml_ceil_pd(const __m128d& a) noexcept { return _mm_svml_ceil_pd(a); }
inline __m128 mm_svml_ceil_ps(const __m128& a) noexcept { return _mm_svml_ceil_ps(a); }
inline __m128d mm_svml_floor_pd(const __m128d& a) noexcept { return _mm_svml_floor_pd(a); }
inline __m128 mm_svml_floor_ps(const __m128& a) noexcept { return _mm_svml_floor_ps(a); }
inline __m128d mm_svml_round_pd(const __m128d& a) noexcept { return _mm_svml_round_pd(a); }
inline __m128 mm_svml_round_ps(const __m128& a) noexcept { return _mm_svml_round_ps(a); }
inline __m128d mm_trunc_pd(const __m128d& a) noexcept { return _mm_trunc_pd(a); }
inline __m128 mm_trunc_ps(const __m128& a) noexcept { return _mm_trunc_ps(a); }
template<unsigned i> inline __m128 mm_shuffle_ps(const __m128& a, const __m128& b) noexcept { return _mm_shuffle_ps(a, b, i); }
inline __m128 mm_unpackhi_ps(const __m128& a, const __m128& b) noexcept { return _mm_unpackhi_ps(a, b); }
inline __m128 mm_unpacklo_ps(const __m128& a, const __m128& b) noexcept { return _mm_unpacklo_ps(a, b); }
inline unsigned int mm_getcsr() noexcept { return _mm_getcsr(); }
inline void mm_setcsr(unsigned int a) noexcept { _mm_setcsr(a); }
template<int i> void mm_prefetch(char const* a) noexcept { _mm_prefetch(a, i); }
inline void mm_sfence() noexcept { _mm_sfence(); }
inline void* mm_malloc(size_t a, size_t b) noexcept { return _mm_malloc(a, b); }
inline void mm_free(void* a) noexcept { _mm_free(a); }
inline __m128 mm_undefined_ps() noexcept { return _mm_undefined_ps(); }
inline __m128 mm_min_ss(const __m128& a, const __m128& b) noexcept { return _mm_min_ss(a, b); }
inline __m128 mm_min_ps(const __m128& a, const __m128& b) noexcept { return _mm_min_ps(a, b); }
inline __m128 mm_max_ss(const __m128& a, const __m128& b) noexcept { return _mm_max_ss(a, b); }
inline __m128 mm_max_ps(const __m128& a, const __m128& b) noexcept { return _mm_max_ps(a, b); }
inline __m128 mm_add_ss(const __m128& a, const __m128& b) noexcept { return _mm_add_ss(a, b); }
inline __m128 mm_add_ps(const __m128& a, const __m128& b) noexcept { return _mm_add_ps(a, b); }
inline __m128 mm_sub_ss(const __m128& a, const __m128& b) noexcept { return _mm_sub_ss(a, b); }
inline __m128 mm_sub_ps(const __m128& a, const __m128& b) noexcept { return _mm_sub_ps(a, b); }
inline __m128 mm_mul_ss(const __m128& a, const __m128& b) noexcept { return _mm_mul_ss(a, b); }
inline __m128 mm_mul_ps(const __m128& a, const __m128& b) noexcept { return _mm_mul_ps(a, b); }
inline __m128 mm_div_ss(const __m128& a, const __m128& b) noexcept { return _mm_div_ss(a, b); }
inline __m128 mm_div_ps(const __m128& a, const __m128& b) noexcept { return _mm_div_ps(a, b); }
template<int i> __m128 mm_cvtsi32_ss(const __m128& a) noexcept { return _mm_cvtsi32_ss(a, i); }
template<int i> __m128 mm_cvt_si2ss(const __m128& a) noexcept { return _mm_cvt_si2ss(a, i); }
inline __m128 mm_cvtsi64_ss(const __m128& a, __int64 b) noexcept { return _mm_cvtsi64_ss(a, b); }
inline int mm_cvtss_si32(const __m128& a) noexcept { return _mm_cvtss_si32(a); }
inline int mm_cvt_ss2si(const __m128& a) noexcept { return _mm_cvt_ss2si(a); }
inline __int64 mm_cvtss_si64(const __m128& a) noexcept { return _mm_cvtss_si64(a); }
inline float mm_cvtss_f32(const __m128& a) noexcept { return _mm_cvtss_f32(a); }
inline int mm_cvttss_si32(const __m128& a) noexcept { return _mm_cvttss_si32(a); }
inline int mm_cvtt_ss2si(const __m128& a) noexcept { return _mm_cvtt_ss2si(a); }
inline __int64 mm_cvttss_si64(const __m128& a) noexcept { return _mm_cvttss_si64(a); }
inline void mm_stream_ps(void* a, const __m128& b) noexcept { _mm_stream_ps((float*)a, b); }
inline void mm_store_ss(float* a, const __m128& b) noexcept { _mm_store_ss(a, b); }
inline void mm_store1_ps(float* a, const __m128& b) noexcept { _mm_store1_ps(a, b); }
inline void mm_store_ps1(float* a, const __m128& b) noexcept { _mm_store_ps1(a, b); }
inline void mm_store_ps(float* a, const __m128& b) noexcept { _mm_store_ps(a, b); }
inline void mm_storeu_ps(float* a, const __m128& b) noexcept { _mm_storeu_ps(a, b); }
inline void mm_storer_ps(float* a, const __m128& b) noexcept { _mm_storer_ps(a, b); }
inline int mm_movemask_ps(const __m128& a) noexcept { return _mm_movemask_ps(a); }
inline __m128 mm_sqrt_ss(const __m128& a) noexcept { return _mm_sqrt_ss(a); }
inline __m128 mm_sqrt_ps(const __m128& a) noexcept { return _mm_sqrt_ps(a); }
inline __m128 mm_rcp_ss(const __m128& a) noexcept { return _mm_rcp_ss(a); }
inline __m128 mm_rcp_ps(const __m128& a) noexcept { return _mm_rcp_ps(a); }
inline __m128 mm_rsqrt_ss(const __m128& a) noexcept { return _mm_rsqrt_ss(a); }
inline __m128 mm_rsqrt_ps(const __m128& a) noexcept { return _mm_rsqrt_ps(a); }
inline __m128 mm_and_ps(const __m128& a, const __m128& b) noexcept { return _mm_and_ps(a, b); }
inline __m128 mm_andnot_ps(const __m128& a, const __m128& b) noexcept { return _mm_andnot_ps(a, b); }
inline __m128 mm_or_ps(const __m128& a, const __m128& b) noexcept { return _mm_or_ps(a, b); }
inline __m128 mm_xor_ps(const __m128& a, const __m128& b) noexcept { return _mm_xor_ps(a, b); }
inline __m128 mm_cmpeq_ss(const __m128& a, const __m128& b) noexcept { return _mm_cmpeq_ss(a, b); }
inline __m128 mm_cmpeq_ps(const __m128& a, const __m128& b) noexcept { return _mm_cmpeq_ps(a, b); }
inline __m128 mm_cmplt_ss(const __m128& a, const __m128& b) noexcept { return _mm_cmplt_ss(a, b); }
inline __m128 mm_cmplt_ps(const __m128& a, const __m128& b) noexcept { return _mm_cmplt_ps(a, b); }
inline __m128 mm_cmple_ss(const __m128& a, const __m128& b) noexcept { return _mm_cmple_ss(a, b); }
inline __m128 mm_cmple_ps(const __m128& a, const __m128& b) noexcept { return _mm_cmple_ps(a, b); }
inline __m128 mm_cmpgt_ss(const __m128& a, const __m128& b) noexcept { return _mm_cmpgt_ss(a, b); }
inline __m128 mm_cmpgt_ps(const __m128& a, const __m128& b) noexcept { return _mm_cmpgt_ps(a, b); }
inline __m128 mm_cmpge_ss(const __m128& a, const __m128& b) noexcept { return _mm_cmpge_ss(a, b); }
inline __m128 mm_cmpge_ps(const __m128& a, const __m128& b) noexcept { return _mm_cmpge_ps(a, b); }
inline __m128 mm_cmpneq_ss(const __m128& a, const __m128& b) noexcept { return _mm_cmpneq_ss(a, b); }
inline __m128 mm_cmpneq_ps(const __m128& a, const __m128& b) noexcept { return _mm_cmpneq_ps(a, b); }
inline __m128 mm_cmpnlt_ss(const __m128& a, const __m128& b) noexcept { return _mm_cmpnlt_ss(a, b); }
inline __m128 mm_cmpnlt_ps(const __m128& a, const __m128& b) noexcept { return _mm_cmpnlt_ps(a, b); }
inline __m128 mm_cmpnle_ss(const __m128& a, const __m128& b) noexcept { return _mm_cmpnle_ss(a, b); }
inline __m128 mm_cmpnle_ps(const __m128& a, const __m128& b) noexcept { return _mm_cmpnle_ps(a, b); }
inline __m128 mm_cmpngt_ss(const __m128& a, const __m128& b) noexcept { return _mm_cmpngt_ss(a, b); }
inline __m128 mm_cmpngt_ps(const __m128& a, const __m128& b) noexcept { return _mm_cmpngt_ps(a, b); }
inline __m128 mm_cmpnge_ss(const __m128& a, const __m128& b) noexcept { return _mm_cmpnge_ss(a, b); }
inline __m128 mm_cmpnge_ps(const __m128& a, const __m128& b) noexcept { return _mm_cmpnge_ps(a, b); }
inline __m128 mm_cmpord_ss(const __m128& a, const __m128& b) noexcept { return _mm_cmpord_ss(a, b); }
inline __m128 mm_cmpord_ps(const __m128& a, const __m128& b) noexcept { return _mm_cmpord_ps(a, b); }
inline __m128 mm_cmpunord_ss(const __m128& a, const __m128& b) noexcept { return _mm_cmpunord_ss(a, b); }
inline __m128 mm_cmpunord_ps(const __m128& a, const __m128& b) noexcept { return _mm_cmpunord_ps(a, b); }
inline int mm_comieq_ss(const __m128& a, const __m128& b) noexcept { return _mm_comieq_ss(a, b); }
inline int mm_comilt_ss(const __m128& a, const __m128& b) noexcept { return _mm_comilt_ss(a, b); }
inline int mm_comile_ss(const __m128& a, const __m128& b) noexcept { return _mm_comile_ss(a, b); }
inline int mm_comigt_ss(const __m128& a, const __m128& b) noexcept { return _mm_comigt_ss(a, b); }
inline int mm_comige_ss(const __m128& a, const __m128& b) noexcept { return _mm_comige_ss(a, b); }
inline int mm_comineq_ss(const __m128& a, const __m128& b) noexcept { return _mm_comineq_ss(a, b); }
inline int mm_ucomieq_ss(const __m128& a, const __m128& b) noexcept { return _mm_ucomieq_ss(a, b); }
inline int mm_ucomilt_ss(const __m128& a, const __m128& b) noexcept { return _mm_ucomilt_ss(a, b); }
inline int mm_ucomile_ss(const __m128& a, const __m128& b) noexcept { return _mm_ucomile_ss(a, b); }
inline int mm_ucomigt_ss(const __m128& a, const __m128& b) noexcept { return _mm_ucomigt_ss(a, b); }
inline int mm_ucomige_ss(const __m128& a, const __m128& b) noexcept { return _mm_ucomige_ss(a, b); }
inline int mm_ucomineq_ss(const __m128& a, const __m128& b) noexcept { return _mm_ucomineq_ss(a, b); }
inline __m128 mm_set_ss(float a) noexcept { return _mm_set_ss(a); }
inline __m128 mm_set1_ps(float a) noexcept { return _mm_set1_ps(a); }
inline __m128 mm_set_ps1(float a) noexcept { return _mm_set_ps1(a); }
inline __m128 mm_set_ps(float a, float b, float c, float d) noexcept { return _mm_set_ps(a, b, c, d); }
inline __m128 mm_setr_ps(float a, float b, float c, float d) noexcept { return _mm_setr_ps(a, b, c, d); }
inline __m128 mm_setzero_ps() noexcept { return _mm_setzero_ps(); }
inline __m128 mm_load_ss(float const* a) noexcept { return _mm_load_ss(a); }
inline __m128 mm_load1_ps(float const* a) noexcept { return _mm_load1_ps(a); }
inline __m128 mm_load_ps1(float const* a) noexcept { return _mm_load_ps1(a); }
inline __m128 mm_load_ps(float const* a) noexcept { return _mm_load_ps(a); }
inline __m128 mm_loadu_ps(float const* a) noexcept { return _mm_loadu_ps(a); }
inline __m128 mm_loadr_ps(float const* a) noexcept { return _mm_loadr_ps(a); }
inline __m128 mm_move_ss(const __m128& a, const __m128& b) noexcept { return _mm_move_ss(a, b); }
inline __m128 mm_movehl_ps(const __m128& a, const __m128& b) noexcept { return _mm_movehl_ps(a, b); }
inline __m128 mm_movelh_ps(const __m128& a, const __m128& b) noexcept { return _mm_movelh_ps(a, b); }
inline __m128d mm_undefined_pd() noexcept { return _mm_undefined_pd(); }
inline __m128i mm_undefined_si128() noexcept { return _mm_undefined_si128(); }
inline void mm_pause() noexcept { _mm_pause(); }
inline void mm_clflush(void const* a) noexcept { _mm_clflush(a); }
inline void mm_lfence() noexcept { _mm_lfence(); }
inline void mm_mfence() noexcept { _mm_mfence(); }
inline __m128i mm_loadu_si64(void const* a) noexcept { return _mm_loadu_si64(a); }
inline __m128i mm_loadu_si16(void const* a) noexcept { return _mm_loadu_si16(a); }
inline __m128i mm_loadu_si32(void const* a) noexcept { return _mm_loadu_si32(a); }
inline __m128i mm_loadl_epi64(__m128i const* a) noexcept { return _mm_loadl_epi64(a); }
inline __m128i mm_load_si128(__m128i const* a) noexcept { return _mm_load_si128(a); }
inline __m128i mm_loadu_si128(__m128i const* a) noexcept { return _mm_loadu_si128(a); }
inline __m128d mm_load_pd(double const* a) noexcept { return _mm_load_pd(a); }
inline __m128d mm_load1_pd(double const* a) noexcept { return _mm_load1_pd(a); }
inline __m128d mm_load_pd1(double const* a) noexcept { return _mm_load_pd1(a); }
inline __m128d mm_loadr_pd(double const* a) noexcept { return _mm_loadr_pd(a); }
inline __m128d mm_loadu_pd(double const* a) noexcept { return _mm_loadu_pd(a); }
inline __m128d mm_load_sd(double const* a) noexcept { return _mm_load_sd(a); }
inline __m128d mm_loadh_pd(const __m128d& a, double const* b) noexcept { return _mm_loadh_pd(a, b); }
inline __m128d mm_loadl_pd(const __m128d& a, double const* b) noexcept { return _mm_loadl_pd(a, b); }
inline void mm_storeu_si16(void* a, const __m128i& b) noexcept { _mm_storeu_si16(a, b); }
inline void mm_storeu_si64(void* a, const __m128i& b) noexcept { _mm_storeu_si64(a, b); }
inline void mm_storeu_si32(void* a, const __m128i& b) noexcept { _mm_storeu_si32(a, b); }
inline void mm_maskmoveu_si128(const __m128i& a, const __m128i& b, char* c) noexcept { _mm_maskmoveu_si128(a, b, c); }
inline void mm_store_si128(__m128i* a, const __m128i& b) noexcept { _mm_store_si128(a, b); }
inline void mm_storeu_si128(__m128i* a, const __m128i& b) noexcept { _mm_storeu_si128(a, b); }
inline void mm_storel_epi64(__m128i* a, const __m128i& b) noexcept { _mm_storel_epi64(a, b); }
inline void mm_stream_si128(void* a, const __m128i& b) noexcept { _mm_stream_si128((m128i*)a, b); }
template<int i> void mm_stream_si32(void* a) noexcept { _mm_stream_si32(a, i); }
// inline void mm_stream_si64(void* a, __int64 b) noexcept { _mm_stream_si64(a, b); }
inline void mm_stream_pd(void* a, const __m128d& b) noexcept { _mm_stream_pd((double*)a, b); }
inline void mm_store_sd(double* a, const __m128d& b) noexcept { _mm_store_sd(a, b); }
inline void mm_store1_pd(double* a, const __m128d& b) noexcept { _mm_store1_pd(a, b); }
inline void mm_store_pd1(double* a, const __m128d& b) noexcept { _mm_store_pd1(a, b); }
inline void mm_store_pd(double* a, const __m128d& b) noexcept { _mm_store_pd(a, b); }
inline void mm_storeu_pd(double* a, const __m128d& b) noexcept { _mm_storeu_pd(a, b); }
inline void mm_storer_pd(double* a, const __m128d& b) noexcept { _mm_storer_pd(a, b); }
inline void mm_storeh_pd(double* a, const __m128d& b) noexcept { _mm_storeh_pd(a, b); }
inline void mm_storel_pd(double* a, const __m128d& b) noexcept { _mm_storel_pd(a, b); }
inline __m128i mm_add_epi8(const __m128i& a, const __m128i& b) noexcept { return _mm_add_epi8(a, b); }
inline __m128i mm_add_epi16(const __m128i& a, const __m128i& b) noexcept { return _mm_add_epi16(a, b); }
inline __m128i mm_add_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_add_epi32(a, b); }
inline __m128i mm_add_epi64(const __m128i& a, const __m128i& b) noexcept { return _mm_add_epi64(a, b); }
inline __m128i mm_adds_epi8(const __m128i& a, const __m128i& b) noexcept { return _mm_adds_epi8(a, b); }
inline __m128i mm_adds_epi16(const __m128i& a, const __m128i& b) noexcept { return _mm_adds_epi16(a, b); }
inline __m128i mm_adds_epu8(const __m128i& a, const __m128i& b) noexcept { return _mm_adds_epu8(a, b); }
inline __m128i mm_adds_epu16(const __m128i& a, const __m128i& b) noexcept { return _mm_adds_epu16(a, b); }
inline __m128i mm_madd_epi16(const __m128i& a, const __m128i& b) noexcept { return _mm_madd_epi16(a, b); }
inline __m128i mm_mulhi_epi16(const __m128i& a, const __m128i& b) noexcept { return _mm_mulhi_epi16(a, b); }
inline __m128i mm_mulhi_epu16(const __m128i& a, const __m128i& b) noexcept { return _mm_mulhi_epu16(a, b); }
inline __m128i mm_mullo_epi16(const __m128i& a, const __m128i& b) noexcept { return _mm_mullo_epi16(a, b); }
inline __m128i mm_mul_epu32(const __m128i& a, const __m128i& b) noexcept { return _mm_mul_epu32(a, b); }
inline __m128i mm_sad_epu8(const __m128i& a, const __m128i& b) noexcept { return _mm_sad_epu8(a, b); }
inline __m128i mm_sub_epi8(const __m128i& a, const __m128i& b) noexcept { return _mm_sub_epi8(a, b); }
inline __m128i mm_sub_epi16(const __m128i& a, const __m128i& b) noexcept { return _mm_sub_epi16(a, b); }
inline __m128i mm_sub_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_sub_epi32(a, b); }
inline __m128i mm_sub_epi64(const __m128i& a, const __m128i& b) noexcept { return _mm_sub_epi64(a, b); }
inline __m128i mm_subs_epi8(const __m128i& a, const __m128i& b) noexcept { return _mm_subs_epi8(a, b); }
inline __m128i mm_subs_epi16(const __m128i& a, const __m128i& b) noexcept { return _mm_subs_epi16(a, b); }
inline __m128i mm_subs_epu8(const __m128i& a, const __m128i& b) noexcept { return _mm_subs_epu8(a, b); }
inline __m128i mm_subs_epu16(const __m128i& a, const __m128i& b) noexcept { return _mm_subs_epu16(a, b); }
inline __m128d mm_add_sd(const __m128d& a, const __m128d& b) noexcept { return _mm_add_sd(a, b); }
inline __m128d mm_add_pd(const __m128d& a, const __m128d& b) noexcept { return _mm_add_pd(a, b); }
inline __m128d mm_div_sd(const __m128d& a, const __m128d& b) noexcept { return _mm_div_sd(a, b); }
inline __m128d mm_div_pd(const __m128d& a, const __m128d& b) noexcept { return _mm_div_pd(a, b); }
inline __m128d mm_mul_sd(const __m128d& a, const __m128d& b) noexcept { return _mm_mul_sd(a, b); }
inline __m128d mm_mul_pd(const __m128d& a, const __m128d& b) noexcept { return _mm_mul_pd(a, b); }
inline __m128d mm_sub_sd(const __m128d& a, const __m128d& b) noexcept { return _mm_sub_sd(a, b); }
inline __m128d mm_sub_pd(const __m128d& a, const __m128d& b) noexcept { return _mm_sub_pd(a, b); }
inline __m128i mm_avg_epu8(const __m128i& a, const __m128i& b) noexcept { return _mm_avg_epu8(a, b); }
inline __m128i mm_avg_epu16(const __m128i& a, const __m128i& b) noexcept { return _mm_avg_epu16(a, b); }
inline __m128i mm_max_epi16(const __m128i& a, const __m128i& b) noexcept { return _mm_max_epi16(a, b); }
inline __m128i mm_max_epu8(const __m128i& a, const __m128i& b) noexcept { return _mm_max_epu8(a, b); }
inline __m128i mm_min_epi16(const __m128i& a, const __m128i& b) noexcept { return _mm_min_epi16(a, b); }
inline __m128i mm_min_epu8(const __m128i& a, const __m128i& b) noexcept { return _mm_min_epu8(a, b); }
inline __m128d mm_max_sd(const __m128d& a, const __m128d& b) noexcept { return _mm_max_sd(a, b); }
inline __m128d mm_max_pd(const __m128d& a, const __m128d& b) noexcept { return _mm_max_pd(a, b); }
inline __m128d mm_min_sd(const __m128d& a, const __m128d& b) noexcept { return _mm_min_sd(a, b); }
inline __m128d mm_min_pd(const __m128d& a, const __m128d& b) noexcept { return _mm_min_pd(a, b); }
template<int i> __m128i mm_slli_si128(const __m128i& a) noexcept { return _mm_slli_si128(a, i); }
template<int i> __m128i mm_bslli_si128(const __m128i& a) noexcept { return _mm_bslli_si128(a, i); }
template<int i> __m128i mm_bsrli_si128(const __m128i& a) noexcept { return _mm_bsrli_si128(a, i); }
template<int i> __m128i mm_slli_epi16(const __m128i& a) noexcept { return _mm_slli_epi16(a, i); }
inline __m128i mm_sll_epi16(const __m128i& a, const __m128i& b) noexcept { return _mm_sll_epi16(a, b); }
template<int i> __m128i mm_slli_epi32(const __m128i& a) noexcept { return _mm_slli_epi32(a, i); }
inline __m128i mm_sll_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_sll_epi32(a, b); }
template<int i> __m128i mm_slli_epi64(const __m128i& a) noexcept { return _mm_slli_epi64(a, i); }
inline __m128i mm_sll_epi64(const __m128i& a, const __m128i& b) noexcept { return _mm_sll_epi64(a, b); }
template<int i> __m128i mm_srai_epi16(const __m128i& a) noexcept { return _mm_srai_epi16(a, i); }
inline __m128i mm_sra_epi16(const __m128i& a, const __m128i& b) noexcept { return _mm_sra_epi16(a, b); }
template<int i> __m128i mm_srai_epi32(const __m128i& a) noexcept { return _mm_srai_epi32(a, i); }
inline __m128i mm_sra_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_sra_epi32(a, b); }
template<int i> __m128i mm_srli_si128(const __m128i& a) noexcept { return _mm_srli_si128(a, i); }
template<int i> __m128i mm_srli_epi16(const __m128i& a) noexcept { return _mm_srli_epi16(a, i); }
inline __m128i mm_srl_epi16(const __m128i& a, const __m128i& b) noexcept { return _mm_srl_epi16(a, b); }
template<int i> __m128i mm_srli_epi32(const __m128i& a) noexcept { return _mm_srli_epi32(a, i); }
inline __m128i mm_srl_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_srl_epi32(a, b); }
template<int i> __m128i mm_srli_epi64(const __m128i& a) noexcept { return _mm_srli_epi64(a, i); }
inline __m128i mm_srl_epi64(const __m128i& a, const __m128i& b) noexcept { return _mm_srl_epi64(a, b); }
inline __m128i mm_and_si128(const __m128i& a, const __m128i& b) noexcept { return _mm_and_si128(a, b); }
inline __m128i mm_andnot_si128(const __m128i& a, const __m128i& b) noexcept { return _mm_andnot_si128(a, b); }
inline __m128i mm_or_si128(const __m128i& a, const __m128i& b) noexcept { return _mm_or_si128(a, b); }
inline __m128i mm_xor_si128(const __m128i& a, const __m128i& b) noexcept { return _mm_xor_si128(a, b); }
inline __m128d mm_and_pd(const __m128d& a, const __m128d& b) noexcept { return _mm_and_pd(a, b); }
inline __m128d mm_andnot_pd(const __m128d& a, const __m128d& b) noexcept { return _mm_andnot_pd(a, b); }
inline __m128d mm_or_pd(const __m128d& a, const __m128d& b) noexcept { return _mm_or_pd(a, b); }
inline __m128d mm_xor_pd(const __m128d& a, const __m128d& b) noexcept { return _mm_xor_pd(a, b); }
inline __m128i mm_cmpeq_epi8(const __m128i& a, const __m128i& b) noexcept { return _mm_cmpeq_epi8(a, b); }
inline __m128i mm_cmpeq_epi16(const __m128i& a, const __m128i& b) noexcept { return _mm_cmpeq_epi16(a, b); }
inline __m128i mm_cmpeq_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_cmpeq_epi32(a, b); }
inline __m128i mm_cmpgt_epi8(const __m128i& a, const __m128i& b) noexcept { return _mm_cmpgt_epi8(a, b); }
inline __m128i mm_cmpgt_epi16(const __m128i& a, const __m128i& b) noexcept { return _mm_cmpgt_epi16(a, b); }
inline __m128i mm_cmpgt_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_cmpgt_epi32(a, b); }
inline __m128i mm_cmplt_epi8(const __m128i& a, const __m128i& b) noexcept { return _mm_cmplt_epi8(a, b); }
inline __m128i mm_cmplt_epi16(const __m128i& a, const __m128i& b) noexcept { return _mm_cmplt_epi16(a, b); }
inline __m128i mm_cmplt_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_cmplt_epi32(a, b); }
inline __m128d mm_cmpeq_sd(const __m128d& a, const __m128d& b) noexcept { return _mm_cmpeq_sd(a, b); }
inline __m128d mm_cmplt_sd(const __m128d& a, const __m128d& b) noexcept { return _mm_cmplt_sd(a, b); }
inline __m128d mm_cmple_sd(const __m128d& a, const __m128d& b) noexcept { return _mm_cmple_sd(a, b); }
inline __m128d mm_cmpgt_sd(const __m128d& a, const __m128d& b) noexcept { return _mm_cmpgt_sd(a, b); }
inline __m128d mm_cmpge_sd(const __m128d& a, const __m128d& b) noexcept { return _mm_cmpge_sd(a, b); }
inline __m128d mm_cmpord_sd(const __m128d& a, const __m128d& b) noexcept { return _mm_cmpord_sd(a, b); }
inline __m128d mm_cmpunord_sd(const __m128d& a, const __m128d& b) noexcept { return _mm_cmpunord_sd(a, b); }
inline __m128d mm_cmpneq_sd(const __m128d& a, const __m128d& b) noexcept { return _mm_cmpneq_sd(a, b); }
inline __m128d mm_cmpnlt_sd(const __m128d& a, const __m128d& b) noexcept { return _mm_cmpnlt_sd(a, b); }
inline __m128d mm_cmpnle_sd(const __m128d& a, const __m128d& b) noexcept { return _mm_cmpnle_sd(a, b); }
inline __m128d mm_cmpngt_sd(const __m128d& a, const __m128d& b) noexcept { return _mm_cmpngt_sd(a, b); }
inline __m128d mm_cmpnge_sd(const __m128d& a, const __m128d& b) noexcept { return _mm_cmpnge_sd(a, b); }
inline __m128d mm_cmpeq_pd(const __m128d& a, const __m128d& b) noexcept { return _mm_cmpeq_pd(a, b); }
inline __m128d mm_cmplt_pd(const __m128d& a, const __m128d& b) noexcept { return _mm_cmplt_pd(a, b); }
inline __m128d mm_cmple_pd(const __m128d& a, const __m128d& b) noexcept { return _mm_cmple_pd(a, b); }
inline __m128d mm_cmpgt_pd(const __m128d& a, const __m128d& b) noexcept { return _mm_cmpgt_pd(a, b); }
inline __m128d mm_cmpge_pd(const __m128d& a, const __m128d& b) noexcept { return _mm_cmpge_pd(a, b); }
inline __m128d mm_cmpord_pd(const __m128d& a, const __m128d& b) noexcept { return _mm_cmpord_pd(a, b); }
inline __m128d mm_cmpunord_pd(const __m128d& a, const __m128d& b) noexcept { return _mm_cmpunord_pd(a, b); }
inline __m128d mm_cmpneq_pd(const __m128d& a, const __m128d& b) noexcept { return _mm_cmpneq_pd(a, b); }
inline __m128d mm_cmpnlt_pd(const __m128d& a, const __m128d& b) noexcept { return _mm_cmpnlt_pd(a, b); }
inline __m128d mm_cmpnle_pd(const __m128d& a, const __m128d& b) noexcept { return _mm_cmpnle_pd(a, b); }
inline __m128d mm_cmpngt_pd(const __m128d& a, const __m128d& b) noexcept { return _mm_cmpngt_pd(a, b); }
inline __m128d mm_cmpnge_pd(const __m128d& a, const __m128d& b) noexcept { return _mm_cmpnge_pd(a, b); }
inline int mm_comieq_sd(const __m128d& a, const __m128d& b) noexcept { return _mm_comieq_sd(a, b); }
inline int mm_comilt_sd(const __m128d& a, const __m128d& b) noexcept { return _mm_comilt_sd(a, b); }
inline int mm_comile_sd(const __m128d& a, const __m128d& b) noexcept { return _mm_comile_sd(a, b); }
inline int mm_comigt_sd(const __m128d& a, const __m128d& b) noexcept { return _mm_comigt_sd(a, b); }
inline int mm_comige_sd(const __m128d& a, const __m128d& b) noexcept { return _mm_comige_sd(a, b); }
inline int mm_comineq_sd(const __m128d& a, const __m128d& b) noexcept { return _mm_comineq_sd(a, b); }
inline int mm_ucomieq_sd(const __m128d& a, const __m128d& b) noexcept { return _mm_ucomieq_sd(a, b); }
inline int mm_ucomilt_sd(const __m128d& a, const __m128d& b) noexcept { return _mm_ucomilt_sd(a, b); }
inline int mm_ucomile_sd(const __m128d& a, const __m128d& b) noexcept { return _mm_ucomile_sd(a, b); }
inline int mm_ucomigt_sd(const __m128d& a, const __m128d& b) noexcept { return _mm_ucomigt_sd(a, b); }
inline int mm_ucomige_sd(const __m128d& a, const __m128d& b) noexcept { return _mm_ucomige_sd(a, b); }
inline int mm_ucomineq_sd(const __m128d& a, const __m128d& b) noexcept { return _mm_ucomineq_sd(a, b); }
inline __m128d mm_cvtepi32_pd(const __m128i& a) noexcept { return _mm_cvtepi32_pd(a); }
template<int i> __m128d mm_cvtsi32_sd(const __m128d& a) noexcept { return _mm_cvtsi32_sd(a, i); }
inline __m128d mm_cvtsi64_sd(const __m128d& a, __int64 b) noexcept { return _mm_cvtsi64_sd(a, b); }
inline __m128d mm_cvtsi64x_sd(const __m128d& a, __int64 b) noexcept { return _mm_cvtsi64_sd(a, b); }
inline __m128 mm_cvtepi32_ps(const __m128i& a) noexcept { return _mm_cvtepi32_ps(a); }
template<int i> __m128i mm_cvtsi32_si128() noexcept { return _mm_cvtsi32_si128(i); }
inline __m128i mm_cvtsi64_si128(__int64 a) noexcept { return _mm_cvtsi64_si128(a); }
inline __m128i mm_cvtsi64x_si128(__int64 a) noexcept { return _mm_cvtsi64_si128(a); }
inline int mm_cvtsi128_si32(const __m128i& a) noexcept { return _mm_cvtsi128_si32(a); }
inline __int64 mm_cvtsi128_si64(const __m128i& a) noexcept { return _mm_cvtsi128_si64(a); }
inline __int64 mm_cvtsi128_si64x(const __m128i& a) noexcept { return _mm_cvtsi128_si64(a); }
inline __m128 mm_cvtpd_ps(const __m128d& a) noexcept { return _mm_cvtpd_ps(a); }
inline __m128d mm_cvtps_pd(const __m128& a) noexcept { return _mm_cvtps_pd(a); }
inline __m128i mm_cvtpd_epi32(const __m128d& a) noexcept { return _mm_cvtpd_epi32(a); }
inline int mm_cvtsd_si32(const __m128d& a) noexcept { return _mm_cvtsd_si32(a); }
inline __int64 mm_cvtsd_si64(const __m128d& a) noexcept { return _mm_cvtsd_si64(a); }
inline __int64 mm_cvtsd_si64x(const __m128d& a) noexcept { return _mm_cvtsd_si64(a); }
inline __m128 mm_cvtsd_ss(const __m128& a, const __m128d& b) noexcept { return _mm_cvtsd_ss(a, b); }
inline double mm_cvtsd_f64(const __m128d& a) noexcept { return _mm_cvtsd_f64(a); }
inline __m128d mm_cvtss_sd(const __m128d& a, const __m128& b) noexcept { return _mm_cvtss_sd(a, b); }
inline __m128i mm_cvttpd_epi32(const __m128d& a) noexcept { return _mm_cvttpd_epi32(a); }
inline int mm_cvttsd_si32(const __m128d& a) noexcept { return _mm_cvttsd_si32(a); }
inline __int64 mm_cvttsd_si64(const __m128d& a) noexcept { return _mm_cvttsd_si64(a); }
inline __int64 mm_cvttsd_si64x(const __m128d& a) noexcept { return _mm_cvttsd_si64(a); }
inline __m128i mm_cvtps_epi32(const __m128& a) noexcept { return _mm_cvtps_epi32(a); }
inline __m128i mm_cvttps_epi32(const __m128& a) noexcept { return _mm_cvttps_epi32(a); }
inline __m128i mm_set_epi64x(__int64 e1, __int64 e0) noexcept { return _mm_set_epi64x(e1, e0); }
inline __m128i mm_set_epi64(__int64 e1, __int64 e0) noexcept { return _mm_set_epi64x(e1, e0); }
inline __m128i mm_set_epi32(int e3, int e2, int e1, int e0) noexcept { return _mm_set_epi32(e3, e2, e1, e0); }
inline __m128i mm_set_epi16(short e7, short e6, short e5, short e4, short e3, short e2, short e1, short e0) noexcept { //
  return _mm_set_epi16(e7, e6, e5, e4, e3, e2, e1, e0);
}
inline __m128i mm_set_epi8(char e15, char e14, char e13, char e12, char e11, char e10, char e9, char e8,      //
                           char e7, char e6, char e5, char e4, char e3, char e2, char e1, char e0) noexcept { //
  return _mm_set_epi8(e15, e14, e13, e12, e11, e10, e9, e8, e7, e6, e5, e4, e3, e2, e1, e0);
}
inline __m128i mm_set1_epi64x(__int64 a) noexcept { return _mm_set1_epi64x(a); }
inline __m128i mm_set1_epi64(__int64 a) noexcept { return _mm_set1_epi64x(a); }
inline __m128i mm_set1_epi32(int a) noexcept { return _mm_set1_epi32(a); }
inline __m128i mm_set1_epi16(short a) noexcept { return _mm_set1_epi16(a); }
inline __m128i mm_set1_epi8(char a) noexcept { return _mm_set1_epi8(a); }
inline __m128i mm_setr_epi64x(__int64 e0, __int64 e1) noexcept { return _mm_setr_epi64x(e0, e1); }
inline __m128i mm_setr_epi64(__int64 e0, __int64 e1) noexcept { return _mm_setr_epi64x(e0, e1); }
inline __m128i mm_setr_epi32(int e0, int e1, int e2, int e3) noexcept { return _mm_setr_epi32(e0, e1, e2, e3); }
inline __m128i mm_setr_epi16(short e0, short e1, short e2, short e3, short e4, short e5, short e6, short e7) noexcept { //
  return _mm_setr_epi16(e0, e1, e2, e3, e4, e5, e6, e7);
}
inline __m128i mm_setr_epi8(char e0, char e1, char e2, char e3, char e4, char e5, char e6, char e7,                  //
                            char e8, char e9, char e10, char e11, char e12, char e13, char e14, char e15) noexcept { //
  return _mm_setr_epi8(e0, e1, e2, e3, e4, e5, e6, e7, e8, e9, e10, e11, e12, e13, e14, e15);
}
inline __m128i mm_setzero_si128() noexcept { return _mm_setzero_si128(); }
inline __m128d mm_set_sd(double a) noexcept { return _mm_set_sd(a); }
inline __m128d mm_set1_pd(double a) noexcept { return _mm_set1_pd(a); }
inline __m128d mm_set_pd1(double a) noexcept { return _mm_set_pd1(a); }
inline __m128d mm_set_pd(double a, double b) noexcept { return _mm_set_pd(a, b); }
inline __m128d mm_setr_pd(double a, double b) noexcept { return _mm_setr_pd(a, b); }
inline __m128d mm_setzero_pd() noexcept { return _mm_setzero_pd(); }
inline __m128i mm_packs_epi16(const __m128i& a, const __m128i& b) noexcept { return _mm_packs_epi16(a, b); }
inline __m128i mm_packs_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_packs_epi32(a, b); }
inline __m128i mm_packus_epi16(const __m128i& a, const __m128i& b) noexcept { return _mm_packus_epi16(a, b); }
inline int mm_movemask_epi8(const __m128i& a) noexcept { return _mm_movemask_epi8(a); }
inline int mm_movemask_pd(const __m128d& a) noexcept { return _mm_movemask_pd(a); }
inline __m128i mm_move_epi64(const __m128i& a) noexcept { return _mm_move_epi64(a); }
inline __m128d mm_move_sd(const __m128d& a, const __m128d& b) noexcept { return _mm_move_sd(a, b); }
template<int i> int mm_extract_epi16(const __m128i& a) noexcept { return _mm_extract_epi16(a, i); }
template<int i> __m128i mm_insert_epi16(const __m128i& a, int b) noexcept { return _mm_insert_epi16(a, b, i); }
template<int i> __m128i mm_shuffle_epi32(const __m128i& a) noexcept { return _mm_shuffle_epi32(a, i); }
template<int i> __m128i mm_shufflehi_epi16(const __m128i& a) noexcept { return _mm_shufflehi_epi16(a, i); }
template<int i> __m128i mm_shufflelo_epi16(const __m128i& a) noexcept { return _mm_shufflelo_epi16(a, i); }
inline __m128i mm_unpackhi_epi8(const __m128i& a, const __m128i& b) noexcept { return _mm_unpackhi_epi8(a, b); }
inline __m128i mm_unpackhi_epi16(const __m128i& a, const __m128i& b) noexcept { return _mm_unpackhi_epi16(a, b); }
inline __m128i mm_unpackhi_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_unpackhi_epi32(a, b); }
inline __m128i mm_unpackhi_epi64(const __m128i& a, const __m128i& b) noexcept { return _mm_unpackhi_epi64(a, b); }
inline __m128i mm_unpacklo_epi8(const __m128i& a, const __m128i& b) noexcept { return _mm_unpacklo_epi8(a, b); }
inline __m128i mm_unpacklo_epi16(const __m128i& a, const __m128i& b) noexcept { return _mm_unpacklo_epi16(a, b); }
inline __m128i mm_unpacklo_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_unpacklo_epi32(a, b); }
inline __m128i mm_unpacklo_epi64(const __m128i& a, const __m128i& b) noexcept { return _mm_unpacklo_epi64(a, b); }
inline __m128d mm_unpackhi_pd(const __m128d& a, const __m128d& b) noexcept { return _mm_unpackhi_pd(a, b); }
inline __m128d mm_unpacklo_pd(const __m128d& a, const __m128d& b) noexcept { return _mm_unpacklo_pd(a, b); }
template<int i> __m128d mm_shuffle_pd(const __m128d& a, const __m128d& b) noexcept { return _mm_shuffle_pd(a, b, i); }
inline __m128d mm_sqrt_sd(const __m128d& a, const __m128d& b) noexcept { return _mm_sqrt_sd(a, b); }
inline __m128d mm_sqrt_pd(const __m128d& a) noexcept { return _mm_sqrt_pd(a); }
inline __m128 mm_castpd_ps(const __m128d& a) noexcept { return _mm_castpd_ps(a); }
inline __m128i mm_castpd_si128(const __m128d& a) noexcept { return _mm_castpd_si128(a); }
inline __m128d mm_castps_pd(const __m128& a) noexcept { return _mm_castps_pd(a); }
inline __m128i mm_castps_si128(const __m128& a) noexcept { return _mm_castps_si128(a); }
inline __m128d mm_castsi128_pd(const __m128i& a) noexcept { return _mm_castsi128_pd(a); }
inline __m128 mm_castsi128_ps(const __m128i& a) noexcept { return _mm_castsi128_ps(a); }
inline __m128 mm_addsub_ps(const __m128& a, const __m128& b) noexcept { return _mm_addsub_ps(a, b); }
inline __m128d mm_addsub_pd(const __m128d& a, const __m128d& b) noexcept { return _mm_addsub_pd(a, b); }
inline __m128d mm_hadd_pd(const __m128d& a, const __m128d& b) noexcept { return _mm_hadd_pd(a, b); }
inline __m128 mm_hadd_ps(const __m128& a, const __m128& b) noexcept { return _mm_hadd_ps(a, b); }
inline __m128d mm_hsub_pd(const __m128d& a, const __m128d& b) noexcept { return _mm_hsub_pd(a, b); }
inline __m128 mm_hsub_ps(const __m128& a, const __m128& b) noexcept { return _mm_hsub_ps(a, b); }
inline __m128i mm_lddqu_si128(__m128i const* a) noexcept { return _mm_lddqu_si128(a); }
inline __m128d mm_loaddup_pd(double const* a) noexcept { return _mm_loaddup_pd(a); }
inline __m128d mm_movedup_pd(const __m128d& a) noexcept { return _mm_movedup_pd(a); }
inline __m128 mm_movehdup_ps(const __m128& a) noexcept { return _mm_movehdup_ps(a); }
inline __m128 mm_moveldup_ps(const __m128& a) noexcept { return _mm_moveldup_ps(a); }
template<int i> __m128d mm_blend_pd(const __m128d& a, const __m128d& b) noexcept { return _mm_blend_pd(a, b, i); }
template<int i> __m128 mm_blend_ps(const __m128& a, const __m128& b) noexcept { return _mm_blend_ps(a, b, i); }
inline __m128d mm_blendv_pd(const __m128d& a, const __m128d& b, const __m128d& c) noexcept { return _mm_blendv_pd(a, b, c); }
inline __m128 mm_blendv_ps(const __m128& a, const __m128& b, const __m128& c) noexcept { return _mm_blendv_ps(a, b, c); }
inline __m128i mm_blendv_epi8(const __m128i& a, const __m128i& b, const __m128i& c) noexcept { return _mm_blendv_epi8(a, b, c); }
template<int i> __m128i mm_blend_epi16(const __m128i& a, const __m128i& b) noexcept { return _mm_blend_epi16(a, b, i); }
template<int i> int mm_extract_ps(const __m128& a) noexcept { return _mm_extract_ps(a, i); }
template<int i> int mm_extract_epi8(const __m128i& a) noexcept { return _mm_extract_epi8(a, i); }
template<int i> int mm_extract_epi32(const __m128i& a) noexcept { return _mm_extract_epi32(a, i); }
template<int i> __int64 mm_extract_epi64(const __m128i& a) noexcept { return _mm_extract_epi64(a, i); }
template<int i> __m128 mm_insert_ps(const __m128& a, const __m128& b) noexcept { return _mm_insert_ps(a, b, i); }
template<int i> __m128i mm_insert_epi8(const __m128i& a, int b) noexcept { return _mm_insert_epi8(a, b, i); }
template<int i> __m128i mm_insert_epi32(const __m128i& a, int b) noexcept { return _mm_insert_epi32(a, b, i); }
template<int i> __m128i mm_insert_epi64(const __m128i& a, __int64 b) noexcept { return _mm_insert_epi64(a, b, i); }
template<int i> __m128d mm_dp_pd(const __m128d& a, const __m128d& b) noexcept { return _mm_dp_pd(a, b, i); }
template<int i> __m128 mm_dp_ps(const __m128& a, const __m128& b) noexcept { return _mm_dp_ps(a, b, i); }
inline __m128i mm_mul_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_mul_epi32(a, b); }
inline __m128i mm_mullo_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_mullo_epi32(a, b); }
template<int i> __m128i mm_mpsadbw_epu8(const __m128i& a, const __m128i& b) noexcept { return _mm_mpsadbw_epu8(a, b, i); }
inline __m128i mm_max_epi8(const __m128i& a, const __m128i& b) noexcept { return _mm_max_epi8(a, b); }
inline __m128i mm_max_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_max_epi32(a, b); }
inline __m128i mm_max_epu32(const __m128i& a, const __m128i& b) noexcept { return _mm_max_epu32(a, b); }
inline __m128i mm_max_epu16(const __m128i& a, const __m128i& b) noexcept { return _mm_max_epu16(a, b); }
inline __m128i mm_min_epi8(const __m128i& a, const __m128i& b) noexcept { return _mm_min_epi8(a, b); }
inline __m128i mm_min_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_min_epi32(a, b); }
inline __m128i mm_min_epu32(const __m128i& a, const __m128i& b) noexcept { return _mm_min_epu32(a, b); }
inline __m128i mm_min_epu16(const __m128i& a, const __m128i& b) noexcept { return _mm_min_epu16(a, b); }
template<int i> __m128d mm_round_pd(const __m128d& a) noexcept { return _mm_round_pd(a, i); }
inline __m128d mm_floor_pd(const __m128d& a) noexcept { return _mm_floor_pd(a); }
inline __m128d mm_ceil_pd(const __m128d& a) noexcept { return _mm_ceil_pd(a); }
template<int i> __m128 mm_round_ps(const __m128& a) noexcept { return _mm_round_ps(a, i); }
inline __m128 mm_floor_ps(const __m128& a) noexcept { return _mm_floor_ps(a); }
inline __m128 mm_ceil_ps(const __m128& a) noexcept { return _mm_ceil_ps(a); }
template<int i> __m128d mm_round_sd(const __m128d& a, const __m128d& b) noexcept { return _mm_round_sd(a, b, i); }
inline __m128d mm_floor_sd(const __m128d& a, const __m128d& b) noexcept { return _mm_floor_sd(a, b); }
inline __m128d mm_ceil_sd(const __m128d& a, const __m128d& b) noexcept { return _mm_ceil_sd(a, b); }
template<int i> __m128 mm_round_ss(const __m128& a, const __m128& b) noexcept { return _mm_round_ss(a, b, i); }
inline __m128 mm_floor_ss(const __m128& a, const __m128& b) noexcept { return _mm_floor_ss(a, b); }
inline __m128 mm_ceil_ss(const __m128& a, const __m128& b) noexcept { return _mm_ceil_ss(a, b); }
inline __m128i mm_packus_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_packus_epi32(a, b); }
inline __m128i mm_cvtepi8_epi16(const __m128i& a) noexcept { return _mm_cvtepi8_epi16(a); }
inline __m128i mm_cvtepi8_epi32(const __m128i& a) noexcept { return _mm_cvtepi8_epi32(a); }
inline __m128i mm_cvtepi8_epi64(const __m128i& a) noexcept { return _mm_cvtepi8_epi64(a); }
inline __m128i mm_cvtepi16_epi32(const __m128i& a) noexcept { return _mm_cvtepi16_epi32(a); }
inline __m128i mm_cvtepi16_epi64(const __m128i& a) noexcept { return _mm_cvtepi16_epi64(a); }
inline __m128i mm_cvtepi32_epi64(const __m128i& a) noexcept { return _mm_cvtepi32_epi64(a); }
inline __m128i mm_cvtepu8_epi16(const __m128i& a) noexcept { return _mm_cvtepu8_epi16(a); }
inline __m128i mm_cvtepu8_epi32(const __m128i& a) noexcept { return _mm_cvtepu8_epi32(a); }
inline __m128i mm_cvtepu8_epi64(const __m128i& a) noexcept { return _mm_cvtepu8_epi64(a); }
inline __m128i mm_cvtepu16_epi32(const __m128i& a) noexcept { return _mm_cvtepu16_epi32(a); }
inline __m128i mm_cvtepu16_epi64(const __m128i& a) noexcept { return _mm_cvtepu16_epi64(a); }
inline __m128i mm_cvtepu32_epi64(const __m128i& a) noexcept { return _mm_cvtepu32_epi64(a); }
inline __m128i mm_cmpeq_epi64(const __m128i& a, const __m128i& b) noexcept { return _mm_cmpeq_epi64(a, b); }
inline int mm_testz_si128(const __m128i& a, const __m128i& b) noexcept { return _mm_testz_si128(a, b); }
inline int mm_testc_si128(const __m128i& a, const __m128i& b) noexcept { return _mm_testc_si128(a, b); }
inline int mm_testnzc_si128(const __m128i& a, const __m128i& b) noexcept { return _mm_testnzc_si128(a, b); }
inline int mm_test_all_zeros(const __m128i& a, const __m128i& b) noexcept { return _mm_test_all_zeros(a, b); }
inline int mm_test_mix_ones_zeros(const __m128i& a, const __m128i& b) noexcept { return _mm_test_mix_ones_zeros(a, b); }
inline int mm_test_all_ones(const __m128i& a) noexcept { return _mm_test_all_ones(a); }
inline __m128i mm_minpos_epu16(const __m128i& a) noexcept { return _mm_minpos_epu16(a); }
inline __m128i mm_stream_load_si128(void* a) noexcept { return _mm_stream_load_si128((m128i*)a); }
template<int i> __m128i mm_cmpistrm(const __m128i& a, const __m128i& b) noexcept { return _mm_cmpistrm(a, b, i); }
template<int i> int mm_cmpistri(const __m128i& a, const __m128i& b) noexcept { return _mm_cmpistri(a, b, i); }
template<int i> int mm_cmpistrz(const __m128i& a, const __m128i& b) noexcept { return _mm_cmpistrz(a, b, i); }
template<int i> int mm_cmpistrc(const __m128i& a, const __m128i& b) noexcept { return _mm_cmpistrc(a, b, i); }
template<int i> int mm_cmpistrs(const __m128i& a, const __m128i& b) noexcept { return _mm_cmpistrs(a, b, i); }
template<int i> int mm_cmpistro(const __m128i& a, const __m128i& b) noexcept { return _mm_cmpistro(a, b, i); }
template<int i> int mm_cmpistra(const __m128i& a, const __m128i& b) noexcept { return _mm_cmpistra(a, b, i); }
template<int i> __m128i mm_cmpestrm(const __m128i& a, int b, const __m128i& c, int d) noexcept { return _mm_cmpestrm(a, b, c, d, i); }
template<int i> int mm_cmpestri(const __m128i& a, int b, const __m128i& c, int d) noexcept { return _mm_cmpestri(a, b, c, d, i); }
template<int i> int mm_cmpestrz(const __m128i& a, int b, const __m128i& c, int d) noexcept { return _mm_cmpestrz(a, b, c, d, i); }
template<int i> int mm_cmpestrc(const __m128i& a, int b, const __m128i& c, int d) noexcept { return _mm_cmpestrc(a, b, c, d, i); }
template<int i> int mm_cmpestrs(const __m128i& a, int b, const __m128i& c, int d) noexcept { return _mm_cmpestrs(a, b, c, d, i); }
template<int i> int mm_cmpestro(const __m128i& a, int b, const __m128i& c, int d) noexcept { return _mm_cmpestro(a, b, c, d, i); }
template<int i> int mm_cmpestra(const __m128i& a, int b, const __m128i& c, int d) noexcept { return _mm_cmpestra(a, b, c, d, i); }
inline __m128i mm_cmpgt_epi64(const __m128i& a, const __m128i& b) noexcept { return _mm_cmpgt_epi64(a, b); }
inline __m128i mm_abs_epi8(const __m128i& a) noexcept { return _mm_abs_epi8(a); }
inline __m128i mm_abs_epi16(const __m128i& a) noexcept { return _mm_abs_epi16(a); }
inline __m128i mm_abs_epi32(const __m128i& a) noexcept { return _mm_abs_epi32(a); }
inline __m128i mm_shuffle_epi8(const __m128i& a, const __m128i& b) noexcept { return _mm_shuffle_epi8(a, b); }
template<int i> __m128i mm_alignr_epi8(const __m128i& a, const __m128i& b) noexcept { return _mm_alignr_epi8(a, b, i); }
inline __m128i mm_hadd_epi16(const __m128i& a, const __m128i& b) noexcept { return _mm_hadd_epi16(a, b); }
inline __m128i mm_hadds_epi16(const __m128i& a, const __m128i& b) noexcept { return _mm_hadds_epi16(a, b); }
inline __m128i mm_hadd_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_hadd_epi32(a, b); }
inline __m128i mm_hsub_epi16(const __m128i& a, const __m128i& b) noexcept { return _mm_hsub_epi16(a, b); }
inline __m128i mm_hsubs_epi16(const __m128i& a, const __m128i& b) noexcept { return _mm_hsubs_epi16(a, b); }
inline __m128i mm_hsub_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_hsub_epi32(a, b); }
inline __m128i mm_maddubs_epi16(const __m128i& a, const __m128i& b) noexcept { return _mm_maddubs_epi16(a, b); }
inline __m128i mm_mulhrs_epi16(const __m128i& a, const __m128i& b) noexcept { return _mm_mulhrs_epi16(a, b); }
inline __m128i mm_sign_epi8(const __m128i& a, const __m128i& b) noexcept { return _mm_sign_epi8(a, b); }
inline __m128i mm_sign_epi16(const __m128i& a, const __m128i& b) noexcept { return _mm_sign_epi16(a, b); }
inline __m128i mm_sign_epi32(const __m128i& a, const __m128i& b) noexcept { return _mm_sign_epi32(a, b); }
inline __m256i mm256_aesenclast_epi128(const __m256i& a, const __m256i& b) noexcept { return _mm256_aesenclast_epi128(a, b); }
inline __m256i mm256_aesenc_epi128(const __m256i& a, const __m256i& b) noexcept { return _mm256_aesenc_epi128(a, b); }
inline __m256i mm256_aesdeclast_epi128(const __m256i& a, const __m256i& b) noexcept { return _mm256_aesdeclast_epi128(a, b); }
inline __m256i mm256_aesdec_epi128(const __m256i& a, const __m256i& b) noexcept { return _mm256_aesdec_epi128(a, b); }
template<int i> __m256i mm256_clmulepi64_epi128(const __m256i& a, const __m256i& b) noexcept { return _mm256_clmulepi64_epi128(a, b, i); }
template<int i> __m512i mm512_clmulepi64_epi128(const __m512i& a, const __m512i& b) noexcept { return _mm512_clmulepi64_epi128(a, b, i); }
} // namespace intrin

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "kernel32.lib")


export namespace win {

using hbrush = HBRUSH;
using hwnd = HWND;

inline bool destroy_brush(hbrush h) { return DeleteObject(h); }
inline bool destroy_window(hwnd h) { return DestroyWindow(h); }

}

#endif
