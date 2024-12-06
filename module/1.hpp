#pragma once

#include "0.hpp"

#include <immintrin.h>

export namespace intrin {

// wmmintrin.h

// immintrin.h

using m256 = __m256;
using m256d = __m256d;
using m256i = __m256i;

inline constexpr int cmp_eq_oq = _CMP_EQ_OQ;
inline constexpr int cmp_lt_os = _CMP_LT_OS;
inline constexpr int cmp_le_os = _CMP_LE_OS;
inline constexpr int cmp_unord_q = _CMP_UNORD_Q;
inline constexpr int cmp_neq_uq = _CMP_NEQ_UQ;
inline constexpr int cmp_nlt_us = _CMP_NLT_US;
inline constexpr int cmp_nle_us = _CMP_NLE_US;
inline constexpr int cmp_ord_q = _CMP_ORD_Q;
inline constexpr int cmp_eq_uq = _CMP_EQ_UQ;
inline constexpr int cmp_nge_us = _CMP_NGE_US;
inline constexpr int cmp_ngt_us = _CMP_NGT_US;
inline constexpr int cmp_false_oq = _CMP_FALSE_OQ;
inline constexpr int cmp_neq_oq = _CMP_NEQ_OQ;
inline constexpr int cmp_ge_os = _CMP_GE_OS;
inline constexpr int cmp_gt_os = _CMP_GT_OS;
inline constexpr int cmp_true_uq = _CMP_TRUE_UQ;
inline constexpr int cmp_eq_os = _CMP_EQ_OS;
inline constexpr int cmp_lt_oq = _CMP_LT_OQ;
inline constexpr int cmp_le_oq = _CMP_LE_OQ;
inline constexpr int cmp_unord_s = _CMP_UNORD_S;
inline constexpr int cmp_neq_us = _CMP_NEQ_US;
inline constexpr int cmp_nlt_uq = _CMP_NLT_UQ;
inline constexpr int cmp_nle_uq = _CMP_NLE_UQ;
inline constexpr int cmp_ord_s = _CMP_ORD_S;
inline constexpr int cmp_eq_us = _CMP_EQ_US;
inline constexpr int cmp_nge_uq = _CMP_NGE_UQ;
inline constexpr int cmp_ngt_uq = _CMP_NGT_UQ;
inline constexpr int cmp_false_os = _CMP_FALSE_OS;
inline constexpr int cmp_neq_os = _CMP_NEQ_OS;
inline constexpr int cmp_ge_oq = _CMP_GE_OQ;
inline constexpr int cmp_gt_oq = _CMP_GT_OQ;
inline constexpr int cmp_true_us = _CMP_TRUE_US;

inline m256d mm256_add_pd(const m256d& a, const m256d& b) { return _mm256_add_pd(a, b); }
inline m256 mm256_add_ps(const m256& a, const m256& b) { return _mm256_add_ps(a, b); }
inline m256d mm256_addsub_pd(const m256d& a, const m256d& b) { return _mm256_andsub_pd(a, b); }
inline m256 mm256_addsub_ps(const m256& a, const m256& b) { return _mm256_addsub_ps(a, b); }

inline m256d mm256_and_pd(const m256d& a, const m256d& b) { return _mm256_and_pd(a, b); }
inline m256 mm256_and_ps(const m256& a, const m256& b) { return _mm256_and_ps(a, b); }
inline m256d mm256_andnot_pd(const m256d& a, const m256d& b) { return _mm256_andnot_pd(a, b); }
inline m256 mm256_andnot_ps(const m256& a, const m256& b) { return _mm256_andnot_ps(a, b); }

template<int i> m256d mm256_blend_pd(const m256d& a, const m256d& b) { return _mm256_blend_pd(a, b, i); }
template<int i> m256 mm256_blend_ps(const m256& a, const m256& b) { return _mm256_blend_ps(a, b, i); }
inline m256d mm256_blendv_pd(const m256d& a, const m256d& b, const m256d& c) { return _mm256_blendv_pd(a, b, c); }
inline m256 mm256_blendv_ps(const m256& a, const m256& b, const m256& c) { return _mm256_blendv_ps(a, b, c); }

inline m256d mm256_div_pd(const m256d& a, const m256d& b) { return _mm256_div_pd(a, b); }
inline m256 mm256_div_ps(const m256& a, const m256& b) { return _mm256_div_ps(a, b); }

template<int i> m256 mm256_dp_ps(const m256& a, const m256& b) { return _mm256_dp_ps(a, b, i); }

inline m256d mm256_hadd_pd(const m256d& a, const m256d& b) { return _mm256_hadd_pd(a, b); }
inline m256 mm256_hadd_ps(const m256& a, const m256& b) { return _mm256_hadd_ps(a, b); }
inline m256d mm256_hsub_pd(const m256d& a, const m256d& b) { return _mm256_hsub_pd(a, b); }
inline m256 mm256_hsub_ps(const m256& a, const m256& b) { return _mm256_hsub_ps(a, b); }

inline m256d mm256_max_pd(const m256d& a, const m256d& b) { return _mm256_max_pd(a, b); }
inline m256 mm256_max_ps(const m256& a, const m256& b) { return _mm256_max_ps(a, b); }
inline m256d mm256_min_pd(const m256d& a, const m256d& b) { return _mm256_min_pd(a, b); }
inline m256 mm256_min_ps(const m256& a, const m256& b) { return _mm256_min_ps(a, b); }

inline m256d mm256_mul_pd(const m256d& a, const m256d& b) { return _mm256_mul_pd(a, b); }
inline m256 mm256_mul_ps(const m256& a, const m256& b) { return _mm256_mul_ps(a, b); }

inline m256d mm256_or_pd(const m256d& a, const m256d& b) { return _mm256_or_pd(a, b); }
inline m256 mm256_or_ps(const m256& a, const m256& b) { return _mm256_or_ps(a, b); }

template<int i> m256d mm256_shuffle_pd(const m256d& a, const m256d& b) { return _mm256_shuffle_pd(a, b, i); }
template<int i> m256 mm256_shuffle_ps(const m256& a, const m256& b) { return _mm256_shuffle_ps(a, b, i); }

inline m256d mm256_sub_pd(const m256d& a, const m256d& b) { return _mm256_sub_pd(a, b); }
inline m256 mm256_sub_ps(const m256& a, const m256& b) { return _mm256_sub_ps(a, b); }

inline m256d mm256_xor_pd(const m256d& a, const m256d& b) { return _mm256_xor_pd(a, b); }
inline m256 mm256_xor_ps(const m256& a, const m256& b) { return _mm256_xor_ps(a, b); }

template<int i> m128d mm_cmp_pd(const m128d& a, const m128d& b) { return _mm_cmp_pd(a, b, i); }
template<int i> m256d mm256_cmp_pd(const m256d& a, const m256d& b) { return _mm256_cmp_pd(a, b, i); }
template<int i> m128 mm_cmp_ps(const m128& a, const m128& b) { return _mm_cmp_ps(a, b, i); }
template<int i> m256 mm256_cmp_ps(const m256& a, const m256& b) { return _mm256_cmp_ps(a, b, i); }
template<int i> m128d mm_cmp_sd(const m128d& a, const m128d& b) { return _mm_cmp_sd(a, b, i); }
template<int i> m128d mm_cmp_ss(const m128d& a, const m128d& b) { return _mm_cmp_ss(a, b, i); }

template<int i> inline int mm_comi_sd(const m128d& a, const m128d& b) { return _mm_comi_sd(a, b, i); }
template<int i> inline int mm_comi_ss(const m128d& a, const m128d& b) { return _mm_comi_ss(a, b, i); }

}
