#pragma once
#include <yw/math.h>
#include <yw/tuple.h>
#include <yw/vector.h>

namespace yw {

using float4x4 = vector4<vector4<float>>;

// inline constexpr float3 normalize(float3 v) noexcept {
//   const auto len = yw::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
//   if (len <= 0.0f) return {};
//   return v / len;
// }

inline constexpr float4 quaternion_normalize(float4 q) noexcept {
  const auto len = yw::sqrt(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
  if (len <= 0.0f) return float4(0, 0, 0, 1);
  return q / len;
}

inline constexpr float4 quaternion_multiply(float4 a, float4 b) noexcept {
  return quaternion_normalize(
    float4(
      a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
      a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,
      a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w,
      a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z));
}

inline constexpr float4 quaternion_from_axis_angle(float3 Axis, float AngleRad) noexcept {
  const auto axis = Axis.normalized();
  if (axis.x == 0.0f && axis.y == 0.0f && axis.z == 0.0f) return float4(0, 0, 0, 1);
  const auto half = AngleRad * 0.5f;
  const auto s = yw::sin(half);
  return quaternion_normalize(float4(axis.x * s, axis.y * s, axis.z * s, yw::cos(half)));
}

inline constexpr float4 quaternion_from_rotation_matrix(const float4x4& m) noexcept {
  const auto trace = m.x.x + m.y.y + m.z.z;
  float4 q;
  if (trace > 0.0f) {
    const auto s = yw::sqrt(trace + 1.0f) * 2.0f;
    q.w = 0.25f * s;
    q.x = (m.z.y - m.y.z) / s;
    q.y = (m.x.z - m.z.x) / s;
    q.z = (m.y.x - m.x.y) / s;
  } else if (m.x.x > m.y.y && m.x.x > m.z.z) {
    const auto s = yw::sqrt(1.0f + m.x.x - m.y.y - m.z.z) * 2.0f;
    q.w = (m.z.y - m.y.z) / s;
    q.x = 0.25f * s;
    q.y = (m.x.y + m.y.x) / s;
    q.z = (m.x.z + m.z.x) / s;
  } else if (m.y.y > m.z.z) {
    const auto s = yw::sqrt(1.0f + m.y.y - m.x.x - m.z.z) * 2.0f;
    q.w = (m.x.z - m.z.x) / s;
    q.x = (m.x.y + m.y.x) / s;
    q.y = 0.25f * s;
    q.z = (m.y.z + m.z.y) / s;
  } else {
    const auto s = yw::sqrt(1.0f + m.z.z - m.x.x - m.y.y) * 2.0f;
    q.w = (m.y.x - m.x.y) / s;
    q.x = (m.x.z + m.z.x) / s;
    q.y = (m.y.z + m.z.y) / s;
    q.z = 0.25f * s;
  }
  return quaternion_normalize(q);
}

struct matrix {
  union {
    __m128 rows[4];
    struct {
      __m128 x, y, z, w;
    };
  };

  __m128& operator[](size_t i) { return rows[i]; }
  const __m128& operator[](size_t i) const { return rows[i]; }

  void store(float4x4& Out) const noexcept {
    _mm_storeu_ps(Out.x.data(), x);
    _mm_storeu_ps(Out.y.data(), y);
    _mm_storeu_ps(Out.z.data(), z);
    _mm_storeu_ps(Out.w.data(), w);
  }

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

inline __m128 matrix_transform(const matrix& M, const __m128 V) {
  auto t0 = mm_permute<0, 1, 4, 5>(M.x, M.y);
  auto t1 = mm_permute<0, 1, 4, 5>(M.z, M.w);
  auto out = _mm_mul_ps(mm_permute<0, 2, 4, 6>(t0, t1), mm_permute<0, 0, 0, 0>(V));
  out = _mm_add_ps(out, _mm_mul_ps(mm_permute<1, 3, 5, 7>(t0, t1), mm_permute<1, 1, 1, 1>(V)));
  t0 = mm_permute<2, 3, 6, 7>(M.x, M.y);
  t1 = mm_permute<2, 3, 6, 7>(M.z, M.w);
  out = _mm_add_ps(out, _mm_mul_ps(mm_permute<0, 2, 4, 6>(t0, t1), mm_permute<2, 2, 2, 2>(V)));
  return _mm_add_ps(out, _mm_mul_ps(mm_permute<1, 3, 5, 7>(t0, t1), mm_permute<3, 3, 3, 3>(V)));
}

inline __m128 matrix_transform(const __m128 V, const matrix& M) {
  auto out = _mm_mul_ps(mm_permute<0, 0, 0, 0>(V), M.x);
  out = _mm_add_ps(out, _mm_mul_ps(mm_permute<1, 1, 1, 1>(V), M.y));
  out = _mm_add_ps(out, _mm_mul_ps(mm_permute<2, 2, 2, 2>(V), M.z));
  return _mm_add_ps(out, _mm_mul_ps(mm_permute<3, 3, 3, 3>(V), M.w));
}

/// Calculates `Out = M * N`.
inline void matrix_transform(const matrix& M, const matrix& N, matrix& Out) {
  Out.x = matrix_transform(M.x, N);
  Out.y = matrix_transform(M.y, N);
  Out.z = matrix_transform(M.z, N);
  Out.w = matrix_transform(M.w, N);
}

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
    t3 = _mm_addsub_ps(mm_permute<2, 3, 0, 1>(_mm_mul_ps(t0, t2)), t2);
    t2 = mm_insert<0, 0, 0b1000>(mm_neg(t0), mm_permute<-1, 3, 1, -1>(t1));
    t0 = mm_insert<2, 0, 0b1000>(t1, mm_permute<-1, 0, 3, -1>(t3));
    t1 = mm_insert<0, 0, 0b1000>(t1, t3);
    _mm_storeu_ps(out.x.data(), t0);
    _mm_storeu_ps(out.y.data(), t1);
    _mm_storeu_ps(out.z.data(), t2);
    out.w = float4(0, 0, 0, 1);
  }
}

inline constexpr float4 quaternion_from_euler(float4 rad) noexcept {
  float4x4 m;
  rotation_matrix(rad, m);
  return quaternion_from_rotation_matrix(m);
}

inline constexpr void rotation_matrix_from_quaternion(float4 q, float4x4& out) noexcept {
  q = quaternion_normalize(q);
  const auto xx = q.x * q.x;
  const auto yy = q.y * q.y;
  const auto zz = q.z * q.z;
  const auto xy = q.x * q.y;
  const auto xz = q.x * q.z;
  const auto yz = q.y * q.z;
  const auto wx = q.w * q.x;
  const auto wy = q.w * q.y;
  const auto wz = q.w * q.z;

  out.x = float4(1.0f - 2.0f * (yy + zz), 2.0f * (xy - wz), 2.0f * (xz + wy), 0.0f);
  out.y = float4(2.0f * (xy + wz), 1.0f - 2.0f * (xx + zz), 2.0f * (yz - wx), 0.0f);
  out.z = float4(2.0f * (xz - wy), 2.0f * (yz + wx), 1.0f - 2.0f * (xx + yy), 0.0f);
  out.w = float4(0.0f, 0.0f, 0.0f, 1.0f);
}

inline constexpr void inverse_rotation_matrix_from_quaternion(float4 q, float4x4& out) noexcept {
  rotation_matrix_from_quaternion(q, out);
  auto t = out.x.y;
  out.x.y = out.y.x;
  out.y.x = t;
  t = out.x.z;
  out.x.z = out.z.x;
  out.z.x = t;
  t = out.y.z;
  out.y.z = out.z.y;
  out.z.y = t;
}

inline constexpr float4 euler_from_quaternion(float4 q) noexcept {
  float4x4 m;
  rotation_matrix_from_quaternion(q, m);
  constexpr float eps = 1e-6f;
  const auto sy = yw::clamp(-m.z.x, -1.0f, 1.0f);
  const auto cy = yw::sqrt(yw::max(0.0f, 1.0f - sy * sy));
  float4 rad;
  rad.y = yw::asin(sy);
  if (cy > eps) {
    rad.x = yw::atan2(m.z.y, m.z.z);
    rad.z = yw::atan2(m.y.x, m.x.x);
  } else {
    rad.x = yw::atan2(-m.y.z, m.y.y);
    rad.z = 0.0f;
  }
  rad.w = 0.0f;
  return rad;
}

inline constexpr float3 rotate(float3 v, float3 Axis, float AngleRad) noexcept {
  const auto q = quaternion_from_axis_angle(Axis, AngleRad);
  const auto u = float3(q.x, q.y, q.z);
  return v + 2.0f * q.w * cross(u, v) + 2.0f * cross(u, cross(u, v));
}

inline constexpr void inverse_rotation_matrix(float4 rad, float4x4& out) {
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

inline constexpr float4 matrix_transform(const float4x4& M, const float4 V) {
  if (std::is_constant_evaluated()) {
    return {M.x.x * V.x + M.y.x * V.y + M.z.x * V.z + M.w.x * V.w,
    M.x.y * V.x + M.y.y * V.y + M.z.y * V.z + M.w.y * V.w,
     M.x.z * V.x + M.y.z * V.y + M.z.z * V.z + M.w.z * V.w,
     M.x.w * V.x + M.y.w * V.y + M.z.w * V.z + M.w.w * V.w};
  } else {
    auto m0 = _mm_loadu_ps(M.x.data());
    auto m1 = _mm_loadu_ps(M.y.data());
    auto m2 = _mm_loadu_ps(M.z.data());
    auto m3 = _mm_loadu_ps(M.w.data());
    auto v = _mm_loadu_ps(V.data());
    auto t0 = mm_permute<0, 1, 4, 5>(m0, m1);
    auto t1 = mm_permute<0, 1, 4, 5>(m2, m3);
    auto t2 = _mm_mul_ps(mm_permute<0, 2, 4, 6>(t0, t1), mm_permute<0, 0, 0, 0>(v));
    t2 = _mm_add_ps(t2, _mm_mul_ps(mm_permute<1, 3, 5, 7>(t0, t1), mm_permute<1, 1, 1, 1>(v)));
    t0 = mm_permute<2, 3, 6, 7>(m0, m1);
    t1 = mm_permute<2, 3, 6, 7>(m2, m3);
    t2 = _mm_add_ps(t2, _mm_mul_ps(mm_permute<0, 2, 4, 6>(t0, t1), mm_permute<2, 2, 2, 2>(v)));
    float4 out;
    _mm_storeu_ps(out.data(), _mm_add_ps(t2, _mm_mul_ps(mm_permute<1, 3, 5, 7>(t0, t1), mm_permute<3, 3, 3, 3>(v))));
    return out;
  }
}

inline constexpr float4 matrix_transform(const float4 V, const float4x4& M) {
  if (std::is_constant_evaluated()) {
    return V.x * M.x + V.y * M.y + V.z * M.z + V.w * M.w;
  } else {
    auto v = _mm_loadu_ps(V.data());
    auto out = _mm_mul_ps(mm_permute<0, 0, 0, 0>(v), _mm_loadu_ps(M.x.data()));
    out = _mm_add_ps(out, _mm_mul_ps(mm_permute<1, 1, 1, 1>(v), _mm_loadu_ps(M.y.data())));
    out = _mm_add_ps(out, _mm_mul_ps(mm_permute<2, 2, 2, 2>(v), _mm_loadu_ps(M.z.data())));
    out = _mm_add_ps(out, _mm_mul_ps(mm_permute<3, 3, 3, 3>(v), _mm_loadu_ps(M.w.data())));
    float4 result;
    _mm_storeu_ps(result.data(), out);
    return result;
  }
}

/// Calculates `Out = M * N`.
inline constexpr void matrix_transform(const float4x4& M, const float4x4& N, float4x4& Out) {
  Out.x = matrix_transform(M.x, N);
  Out.y = matrix_transform(M.y, N);
  Out.z = matrix_transform(M.z, N);
  Out.w = matrix_transform(M.w, N);
}
} // namespace yw
