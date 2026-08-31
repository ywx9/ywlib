#pragma once
#include <yw/array.h>
#include <yw/backend.h>
#include <yw/math.h>
#include <yw/matrix.h>

namespace yw::geom {

/// MARK: geom::geometry_base

template<typename Geometry, backend Backend = default_backend> class geometry_base;

template<typename Geometry> class geometry_base<Geometry, cpu> {
protected:
  template<typename OtherG, backend OtherB> friend class geometry_base;
  matrix<double, 3, 4> _rigid = {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}};
  double4 _scale = {1, 1, 1, 1};
  bool _dirty = false; // need to update world matrix
  bool _messy = false; // need to update mesh

  constexpr void _translation(double x, double y, double z) noexcept {
    _rigid[0][3] = x;
    _rigid[1][3] = y;
    _rigid[2][3] = z;
    _dirty = true;
  }
  constexpr void _translate(double x, double y, double z) noexcept {
    _rigid[0][3] += x;
    _rigid[1][3] += y;
    _rigid[2][3] += z;
    _dirty = true;
  }
  template<typename T> constexpr void _rotation(const T& radians) noexcept {
    const auto c = vapply_r<T>(yw::cos, radians);
    const auto s = vapply_r<T>(yw::sin, radians);
    _rigid[0][0] = c.y * c.z;
    _rigid[0][1] = s.x * s.y * c.z - c.x * s.z;
    _rigid[0][2] = c.x * s.y * c.z + s.x * s.z;
    _rigid[1][0] = c.y * s.z;
    _rigid[1][1] = s.x * s.y * s.z + c.x * c.z;
    _rigid[1][2] = c.x * s.y * s.z - s.x * c.z;
    _rigid[2][0] = -s.y;
    _rigid[2][1] = s.x * c.y;
    _rigid[2][2] = c.x * c.y;
    _dirty = true;
  }
  constexpr void _rotate_x(double c, double s) noexcept {
    const auto r10 = _rigid[1], r20 = _rigid[2];
    for (size_t i = 0; i < 3; ++i) {
      _rigid[1][i] = c * r10[i] - s * r20[i];
      _rigid[2][i] = s * r10[i] + c * r20[i];
    }
    _dirty = true;
  }
  constexpr void _rotate_y(double c, double s) noexcept {
    const auto r00 = _rigid[0], r20 = _rigid[2];
    for (size_t i = 0; i < 3; ++i) {
      _rigid[0][i] = c * r00[i] + s * r20[i];
      _rigid[2][i] = -s * r00[i] + c * r20[i];
    }
    _dirty = true;
  }
  constexpr void _rotate_z(double c, double s) noexcept {
    const auto r00 = _rigid[0], r10 = _rigid[1];
    for (size_t i = 0; i < 3; ++i) {
      _rigid[0][i] = c * r00[i] - s * r10[i];
      _rigid[1][i] = s * r00[i] + c * r10[i];
    }
    _dirty = true;
  }
  constexpr void _rotate(double x, double y, double z) noexcept {
    if (x != 0) _rotate_x(yw::cos(x), yw::sin(x));
    if (y != 0) _rotate_y(yw::cos(y), yw::sin(y));
    if (z != 0) _rotate_z(yw::cos(z), yw::sin(z));
  }
  constexpr void _revolve_x(double c, double s) noexcept {
    const auto y = _rigid[1][3], z = _rigid[2][3];
    _rigid[1][3] = c * y - s * z;
    _rigid[2][3] = s * y + c * z;
    _rotate_x(c, s);
  }
  constexpr void _revolve_y(double c, double s) noexcept {
    const auto x = _rigid[0][3], z = _rigid[2][3];
    _rigid[0][3] = c * x + s * z;
    _rigid[2][3] = -s * x + c * z;
    _rotate_y(c, s);
  }
  constexpr void _revolve_z(double c, double s) noexcept {
    const auto x = _rigid[0][3], y = _rigid[1][3];
    _rigid[0][3] = c * x - s * y;
    _rigid[1][3] = s * x + c * y;
    _rotate_z(c, s);
  }
  constexpr void _revolve(double x, double y, double z) noexcept {
    if (x != 0) _revolve_x(yw::cos(x), yw::sin(x));
    if (y != 0) _revolve_y(yw::cos(y), yw::sin(y));
    if (z != 0) _revolve_z(yw::cos(z), yw::sin(z));
  }
  constexpr void _set_scale(double x, double y, double z) noexcept {
    _scale = {x, y, z, 1};
    _dirty = true;
  }
  constexpr void _rescale(double x, double y, double z) noexcept {
    _scale.x *= x;
    _scale.y *= y;
    _scale.z *= z;
    _dirty = true;
  }
  /// applies full transformation (scale, rotation, translation) to given vector
  // constexpr double4 transform(const double4& v) const noexcept {
  //   const auto sv = _scale * v;
  //   return {dot(double4(_rigid[0]), sv), dot(double4(_rigid[1]), sv), dot(double4(_rigid[2]), sv), v.w};
  // }

public:
  /// gets the translation component.
  constexpr double4 translation() const noexcept { return {_rigid[0][3], _rigid[1][3], _rigid[2][3], 0}; }
  /// sets the translation component.
  constexpr void translation(const double3& t) noexcept { _translation(t.x, t.y, t.z); }
  constexpr void translation(const double4& t) noexcept { _translation(t.x, t.y, t.z); }
  /// adds to the translation component.
  constexpr void translate(const double3& t) noexcept { _translate(t.x, t.y, t.z); }
  constexpr void translate(const double4& t) noexcept { _translate(t.x, t.y, t.z); }
  /// gets the rotation component as XYZ Euler angles.
  constexpr double4 rotation() const noexcept {
    double4 result;
    result.y = yw::asin(yw::clamp(-_rigid[2][0], -1.0, 1.0));
    if (yw::abs(_rigid[2][0]) < 1.0 - 1e-10) {
      result.x = yw::atan2(_rigid[2][1], _rigid[2][2]);
      result.z = yw::atan2(_rigid[1][0], _rigid[0][0]);
    } else result.x = yw::atan2(-_rigid[1][2], _rigid[1][1]);
    return result;
  }
  /// sets the rotation component as XYZ Euler angles.
  constexpr void rotation(const double3& radians) noexcept { _rotation(radians); }
  constexpr void rotation(const double4& radians) noexcept { _rotation(radians); }
  /// adds an XYZ Euler rotation.
  constexpr void rotate(const double3& radians) noexcept { _rotate(radians.x, radians.y, radians.z); }
  constexpr void rotate(const double4& radians) noexcept { _rotate(radians.x, radians.y, radians.z); }
  /// rotates the entire transformation about the origin.
  constexpr void revolve(const double3& radians) noexcept { _revolve(radians.x, radians.y, radians.z); }
  constexpr void revolve(const double4& radians) noexcept { _revolve(radians.x, radians.y, radians.z); }
  /// gets the scale component.
  constexpr double4 scale() const noexcept { return _scale; }
  /// sets the scale component.
  constexpr void scale(const double3& s) noexcept { _set_scale(s.x, s.y, s.z); }
  constexpr void scale(const double4& s) noexcept { _set_scale(s.x, s.y, s.z); }
  /// multiplies the scale component.
  constexpr void rescale(const double3& s) noexcept { _rescale(s.x, s.y, s.z); }
  constexpr void rescale(const double4& s) noexcept { _rescale(s.x, s.y, s.z); }
  /// gets the 3D transformation matrix.
  constexpr matrix<double, 3, 4> transformation3() const noexcept {
    return {
      matrix_row(double4(_rigid[0]) * _scale), matrix_row(double4(_rigid[1]) * _scale),
      matrix_row(double4(_rigid[2]) * _scale)};
  }
  /// gets the 3D transformation matrix.
  constexpr matrix<double, 4, 4> transformation4() const noexcept {
    return {
      matrix_row(double4(_rigid[0]) * _scale), matrix_row(double4(_rigid[1]) * _scale),
      matrix_row(double4(_rigid[2]) * _scale), matrix_row{0, 0, 0, 1}};
  }
  /// copies the transformation from another geometry object.
  template<typename OtherG, backend OtherB>
  constexpr void copy_transformation_from(const geometry_base<OtherG, OtherB>& From) noexcept {
    _rigid = From._rigid;
    _scale = From._scale;
    _dirty = From._dirty;
  }
};
} // namespace yw::geom
