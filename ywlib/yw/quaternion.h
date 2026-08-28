#pragma once
#include <yw/matrix.h>

namespace yw {

template<arithmetic T> struct quaternion {
  using value_type = T;
  static constexpr size_t count{4};

  T x{}, y{}, z{}, w{1};

  constexpr quaternion() noexcept = default;
  constexpr quaternion(T X, T Y, T Z, T W) noexcept : x(X), y(Y), z(Z), w(W) {}

  template<castable_to<T> U>
  explicit constexpr quaternion(const vector<U, 4>& v) noexcept(nt_castable_to<U, T>)
    : x(T(v.x)), y(T(v.y)), z(T(v.z)), w(T(v.w)) {}

  template<castable_to<T> U>
  constexpr quaternion(const quaternion<U>& q) noexcept(nt_castable_to<U, T>)
    : x(T(q.x)), y(T(q.y)), z(T(q.z)), w(T(q.w)) {}

  constexpr T* data() noexcept { return &x; }
  constexpr const T* data() const noexcept { return &x; }
  constexpr T& operator[](integral auto i) noexcept { return data()[size_t((i % 4) + 4) % 4]; }
  constexpr const T& operator[](integral auto i) const noexcept { return data()[size_t((i % 4) + 4) % 4]; }
  constexpr vector<T, 4> vector() const noexcept { return {x, y, z, w}; }
};

using quatf = quaternion<float>;
using quatd = quaternion<double>;

template<arithmetic T> constexpr quaternion<T> quaternion_normalize(quaternion<T> q) noexcept {
  const auto len = yw::sqrt(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
  if (len <= 0) return quaternion<T>(0, 0, 0, 1);
  return quaternion<T>(q.x / len, q.y / len, q.z / len, q.w / len);
}

template<arithmetic T, arithmetic U>
constexpr quaternion<math_type<T, U>> operator*(quaternion<T> a, quaternion<U> b) noexcept {
  using R = math_type<T, U>;
  return quaternion_normalize(quaternion<R>(
    a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y, a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,
    a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w, a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z));
}

template<arithmetic T, arithmetic U>
constexpr quaternion<math_type<T, U>> quaternion_multiply(quaternion<T> a, quaternion<U> b) noexcept {
  return a * b;
}

template<arithmetic T, arithmetic U>
constexpr quaternion<math_type<T, U>> quaternion_from_axis_angle(vector<T, 3> Axis, U AngleRad) noexcept {
  using R = math_type<T, U>;
  const auto axis = Axis.normalized();
  if (axis.x == 0 && axis.y == 0 && axis.z == 0) return quaternion<R>(0, 0, 0, 1);
  const auto half = AngleRad * R(0.5);
  const auto s = yw::sin(half);
  return quaternion_normalize(quaternion<R>(axis.x * s, axis.y * s, axis.z * s, yw::cos(half)));
}

template<arithmetic T> constexpr quaternion<T> quaternion_from_rotation_matrix(const matrix<T, 4, 4>& m) noexcept {
  const auto trace = m[0][0] + m[1][1] + m[2][2];
  quaternion<T> q;
  if (trace > 0) {
    const auto s = yw::sqrt(trace + T(1)) * T(2);
    q.w = T(0.25) * s;
    q.x = (m[2][1] - m[1][2]) / s;
    q.y = (m[0][2] - m[2][0]) / s;
    q.z = (m[1][0] - m[0][1]) / s;
  } else if (m[0][0] > m[1][1] && m[0][0] > m[2][2]) {
    const auto s = yw::sqrt(T(1) + m[0][0] - m[1][1] - m[2][2]) * T(2);
    q.w = (m[2][1] - m[1][2]) / s;
    q.x = T(0.25) * s;
    q.y = (m[0][1] + m[1][0]) / s;
    q.z = (m[0][2] + m[2][0]) / s;
  } else if (m[1][1] > m[2][2]) {
    const auto s = yw::sqrt(T(1) + m[1][1] - m[0][0] - m[2][2]) * T(2);
    q.w = (m[0][2] - m[2][0]) / s;
    q.x = (m[0][1] + m[1][0]) / s;
    q.y = T(0.25) * s;
    q.z = (m[1][2] + m[2][1]) / s;
  } else {
    const auto s = yw::sqrt(T(1) + m[2][2] - m[0][0] - m[1][1]) * T(2);
    q.w = (m[1][0] - m[0][1]) / s;
    q.x = (m[0][2] + m[2][0]) / s;
    q.y = (m[1][2] + m[2][1]) / s;
    q.z = T(0.25) * s;
  }
  return quaternion_normalize(q);
}

template<arithmetic T> constexpr void rotation_matrix(vector<T, 4> rad, matrix<T, 4, 4>& out) {
  const auto c = vapply_r<vector<T, 4>>(yw::cos, rad);
  const auto s = vapply_r<vector<T, 4>>(yw::sin, rad);
  out[0] = matrix_row<T, 4>(c.y * c.z, s.x * s.y * c.z - c.x * s.z, c.x * s.y * c.z + s.x * s.z, 0);
  out[1] = matrix_row<T, 4>(c.y * s.z, s.x * s.y * s.z + c.x * c.z, c.x * s.y * s.z - s.x * c.z, 0);
  out[2] = matrix_row<T, 4>(-s.y, s.x * c.y, c.x * c.y, 0);
  out[3] = matrix_row<T, 4>(0, 0, 0, 1);
}

template<arithmetic T> constexpr quaternion<T> quaternion_from_euler(vector<T, 4> rad) noexcept {
  matrix<T, 4, 4> m;
  rotation_matrix(rad, m);
  return quaternion_from_rotation_matrix(m);
}

template<arithmetic T> constexpr void rotation_matrix_from_quaternion(quaternion<T> q, matrix<T, 4, 4>& out) noexcept {
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

  out[0] = matrix_row<T, 4>(T(1) - T(2) * (yy + zz), T(2) * (xy - wz), T(2) * (xz + wy), 0);
  out[1] = matrix_row<T, 4>(T(2) * (xy + wz), T(1) - T(2) * (xx + zz), T(2) * (yz - wx), 0);
  out[2] = matrix_row<T, 4>(T(2) * (xz - wy), T(2) * (yz + wx), T(1) - T(2) * (xx + yy), 0);
  out[3] = matrix_row<T, 4>(0, 0, 0, 1);
}

template<arithmetic T>
constexpr void inverse_rotation_matrix_from_quaternion(quaternion<T> q, matrix<T, 4, 4>& out) noexcept {
  rotation_matrix_from_quaternion(q, out);
  std::swap(out[0][1], out[1][0]);
  std::swap(out[0][2], out[2][0]);
  std::swap(out[1][2], out[2][1]);
}

template<arithmetic T> constexpr vector<T, 4> euler_from_quaternion(quaternion<T> q) noexcept {
  matrix<T, 4, 4> m;
  rotation_matrix_from_quaternion(q, m);
  constexpr T eps = T(1e-6);
  const auto sy = yw::clamp(-m[2][0], T(-1), T(1));
  const auto cy = yw::sqrt(yw::max(T(0), T(1) - sy * sy));
  vector<T, 4> rad;
  rad.y = yw::asin(sy);
  if (cy > eps) {
    rad.x = yw::atan2(m[2][1], m[2][2]);
    rad.z = yw::atan2(m[1][0], m[0][0]);
  } else {
    rad.x = yw::atan2(-m[1][2], m[1][1]);
    rad.z = 0;
  }
  rad.w = 0;
  return rad;
}

template<arithmetic T, arithmetic U>
constexpr vector<math_type<T, U>, 3> rotate(vector<T, 3> v, vector<T, 3> Axis, U AngleRad) noexcept {
  const auto q = quaternion_from_axis_angle(Axis, AngleRad);
  const auto u = vector<math_type<T, U>, 3>(q.x, q.y, q.z);
  return v + 2 * q.w * cross(u, v) + 2 * cross(u, cross(u, v));
}
} // namespace yw
