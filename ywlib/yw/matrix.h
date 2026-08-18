#pragma once
#include <yw/math.h>
#include <yw/mm_vector.h>
#include <yw/tuple.h>
#include <yw/vector.h>

namespace yw {

using float4x4 = vector4<vector4<float>>;

inline constexpr float4 quaternion_normalize(float4 q) noexcept {
  const auto len = yw::sqrt(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
  if (len <= 0.0f) return float4(0, 0, 0, 1);
  return q / len;
}

inline constexpr float4 quaternion_multiply(float4 a, float4 b) noexcept {
  return quaternion_normalize(float4(
    a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y, a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,
    a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w, a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z));
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

/// Calculates `Out = M * N`.

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

//-- matrix functions --//

using float4x4 = vector4<vector4<float>>;
template<std::regular T, size_t Rows, size_t Cols> using matrix = vector<vector<T, Cols>, Rows>;

template<typename T, typename U, size_t Rows, size_t Cols>
requires(!variation_of<T, vector<int, 1>> && !variation_of<U, vector<int, 1>>)
constexpr auto outer(const vector<T, Rows>& a, const vector<U, Cols>& b) {
  matrix<decltype(T{} * U{}), Rows, Cols> result;
  for (size_t i = 0; i < Rows; ++i)
    for (size_t j = 0; j < Cols; ++j) result[i][j] = a[i] * b[j];
  return result;
}

/// MARK: transpose

template<typename T, size_t Rows, size_t Cols>
constexpr matrix<T, Cols, Rows> transpose(const matrix<T, Rows, Cols>& m) noexcept {
  matrix<T, Cols, Rows> out;
  for (size_t r = 0; r < Rows; ++r)
    for (size_t c = 0; c < Cols; ++c) out[c][r] = m[r][c];
  return out;
}

template<typename T, size_t Row, size_t Col>
constexpr void transpose(const matrix<T, Row, Col>& m, matrix<T, Col, Row>& out) noexcept {
  for (size_t r = 0; r < Row; ++r)
    for (size_t c = 0; c < Col; ++c) out[c][r] = m[r][c];
}

template<typename T, size_t Rows, size_t Cols> constexpr void transpose_inplace(matrix<T, Rows, Cols>& m) noexcept {
  for (size_t r = 0; r < Rows; ++r)
    for (size_t c = r + 1; c < Cols; ++c) std::swap(m[r][c], m[c][r]);
}

/// MARK: transform

template<arithmetic T, arithmetic U, size_t Rows, size_t Cols>
requires(!variation_of<T, vector<int, 1>> && !variation_of<U, vector<int, 1>>)
constexpr vector<math_type<T, U>, Rows> transform(const matrix<T, Rows, Cols>& m, const vector<U, Cols>& v) noexcept {
  vector<math_type<T, U>, Rows> out;
  for (size_t r = 0; r < Rows; ++r) out[r] = dot(m[r], v);
  return out;
}

/// MARK: dot

template<arithmetic T, arithmetic U, size_t Rows, size_t Cols>
requires(!variation_of<T, vector<int, 1>> && !variation_of<U, vector<int, 1>>)
constexpr vector<math_type<T, U>, Cols> dot(const vector<T, Rows>& v, const matrix<U, Rows, Cols>& m) noexcept {
  vector<math_type<T, U>, Cols> out;
  for (size_t c = 0; c < Cols; ++c)
    for (size_t r = 0; r < Rows; ++r) out[c] += v[r] * m[r][c];
  return out;
}

template<arithmetic T, arithmetic U, size_t Rows, size_t Cols, size_t Cols2>
requires(!variation_of<T, vector<int, 1>> && !variation_of<U, vector<int, 1>>)
constexpr matrix<math_type<T, U>, Rows, Cols2> dot(
  const matrix<T, Rows, Cols>& a, const matrix<U, Cols, Cols2>& b) noexcept {
  matrix<math_type<T, U>, Rows, Cols2> out;
  for (size_t r = 0; r < Rows; ++r)
    for (size_t c = 0; c < Cols2; ++c)
      for (size_t k = 0; k < Cols; ++k) out[r][c] += a[r][k] * b[k][c];
  return out;
}

template<arithmetic T, arithmetic U, arithmetic V, size_t Rows, size_t Cols, size_t Cols2>
requires(!variation_of<T, vector<int, 1>> && !variation_of<U, vector<int, 1>> && convertible_to<decltype(T{} * U{}), V>)
constexpr void dot(
  const matrix<T, Rows, Cols>& a, const matrix<U, Cols, Cols2>& b, matrix<V, Rows, Cols2>& out) noexcept {
  for (size_t r = 0; r < Rows; ++r)
    for (size_t c = 0; c < Cols2; ++c) {
      out[r][c] = 0;
      for (size_t k = 0; k < Cols; ++k) out[r][c] += a[r][k] * b[k][c];
    }
}
} // namespace yw
