#pragma once
#include "yw/vector.h"

namespace yw {

struct matrix {
  union {
    __m128 rows[4];
    struct {
      __m128 x, y, z, w;
    };
  };

  __m128& operator[](size_t i) { return rows[i]; }
  const __m128& operator[](size_t i) const { return rows[i]; }

  void t(matrix& Out) const {
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

inline void rotation_matrix(const __m128 Cos, const __m128 Sin, matrix& Out) {
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

inline void inverse_rotation_matrix(const __m128 Cos, const __m128 Sin, matrix& Out) {
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

using float4x4 = vector4<vector4<float>>;

inline constexpr void rotation_matrix(float4 rad, float4x4& out) {
  const auto c = vapply_r<float4>(yw::cos, rad);
  const auto s = vapply_r<float4>(yw::sin, rad);
  if (std::is_constant_evaluated()) {
    out.x.x = c.y * c.z;
    out.x.y = s.x * s.y * c.z - c.x * s.z;
    out.x.z = c.x * s.y * c.z + s.x * s.z;
    out.x.w = 0;
    out.y.x = c.y * s.z;
    out.y.y = s.x * s.y * s.z + c.x * c.z;
    out.y.z = c.x * s.y * s.z - s.x * c.z;
    out.y.w = 0;
    out.z.x = -s.y;
    out.z.y = s.x * c.y;
    out.z.z = c.x * c.y;
    out.z.w = 0;
    out.w = float4(0, 0, 0, 1);
  } else {
    auto t0 = _mm_loadu_ps(s.data());                // [sx, sy, sz, any]
    auto t1 = _mm_loadu_ps(c.data());                // [cx, cy, cz, any]
    auto t2 = mm_permute<4, 6, 0, 2>(t0, t1);        // [cx, cz, sx, sz]
    auto t3 = mm_permute<3, 0, 1, 2>(t2);            // [sz, cx, cz, sx]
    t2 = _mm_mul_ps(t2, t3);                         // [cxsz, cxcz, sxcz, sxsz]
    t1 = _mm_mul_ps(mm_permute<1, 1, 1, 1>(t1), t3); // [cysz, cxcy, cycz, sxcy]
    t0 = mm_permute<1, 1, 1, 1>(t0);                 // [sy, sy, sy, sy]
    t3 = _mm_addsub_ps(
      mm_permute<2, 3, 0, 1>(_mm_mul_ps(t0, t2)), t2); // [sxsycz - cxsz, sxsysz + cxcz, cxsysz - sxcz, cxsycz + sxsz]
    t2 = mm_insert<0, 0, 0b1000>(mm_neg(t0), mm_permute<-1, 3, 1, -1>(t1)); // [ -sy,          sxcy,          cxcy, 0]
    t0 = mm_insert<2, 0, 0b1000>(t1, mm_permute<-1, 0, 3, -1>(t3));         // [cycz, sxsycz - cxsz, cxsycz + sxsz, 0]
    t1 = mm_insert<0, 0, 0b1000>(t1, t3);                                   // [cysz, sxsysz + cxcz, cxsysz - sxcz, 0]
    _mm_storeu_ps(out.x.data(), t0);
    _mm_storeu_ps(out.y.data(), t1);
    _mm_storeu_ps(out.z.data(), t2);
    out.w = float4(0, 0, 0, 1);
  }
}

inline void inverse_rotation_matrix(float4 rad, float4x4& out) {
  const auto c = vapply_r<float4>(yw::cos, rad);
  const auto s = vapply_r<float4>(yw::sin, rad);
  if (std::is_constant_evaluated()) {
    out.x.x = c.y * c.z;
    out.x.y = c.y * s.z;
    out.x.z = -s.y;
    out.x.w = 0;
    out.y.x = s.x * s.y * c.z - c.x * s.z;
    out.y.y = s.x * s.y * s.z + c.x * c.z;
    out.y.z = s.x * c.y;
    out.y.w = 0;
    out.z.x = c.x * s.y * c.z + s.x * s.z;
    out.z.y = c.x * s.y * s.z - s.x * c.z;
    out.z.z = c.x * c.y;
    out.z.w = 0;
    out.w = float4(0, 0, 0, 1);
  } else {
    auto t0 = _mm_loadu_ps(s.data());
    auto t1 = _mm_loadu_ps(c.data());
    auto t2 = mm_permute<4, 6, 0, 2>(t0, t1);
    auto t3 = mm_permute<3, 0, 1, 2>(t2);
    t2 = _mm_mul_ps(t2, t3);
    t1 = _mm_mul_ps(mm_permute<1, 1, 1, 1>(t1), t3);
    t0 = mm_permute<1, 1, 1, 1>(t0);
    t3 = _mm_addsub_ps(mm_permute<2, 3, 0, 1>(_mm_mul_ps(t0, t2)), t2);
    t0 = mm_insert<2, 2, 0b1000>(mm_neg(t0), mm_permute<2, 0, -1, -1>(t1));
    t2 = mm_insert<1, 2, 0b1000>(t1, mm_permute<3, 2, -1, -1>(t3));
    t1 = mm_insert<3, 2, 0b1000>(t1, t3);
    _mm_storeu_ps(out.x.data(), t0);
    _mm_storeu_ps(out.y.data(), t1);
    _mm_storeu_ps(out.z.data(), t2);
    out.w = float4(0, 0, 0, 1);
  }
}
} // namespace yw
