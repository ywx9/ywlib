#pragma once
#include <yw/backend.h>
#include <yw/matrix.h>

namespace yw::geom {

/// MARK: geom::geometry_base

template<typename Geometry, backend Backend = default_backend> class geometry_base;

template<typename Geometry> class geometry_base<Geometry, cpu> {
  template<typename T> constexpr void _rotation(const T& Radians) noexcept {
    const auto c = vapply_r<T>(yw::cos, Radians);
    const auto s = vapply_r<T>(yw::sin, Radians);
    _rigid[0][0] = c.y * c.z, _rigid[0][1] = s.x * s.y * c.z - c.x * s.z, _rigid[0][2] = c.x * s.y * c.z + s.x * s.z;
    _rigid[1][0] = c.y * s.z, _rigid[1][1] = s.x * s.y * s.z + c.x * c.z, _rigid[1][2] = c.x * s.y * s.z - s.x * c.z;
    _rigid[2][0] = -s.y, _rigid[2][1] = s.x * c.y, _rigid[2][2] = c.x * c.y;
    _dirty = true;
  }

protected:
  matrix<double, 3, 4> _rigid = {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}};
  double4 _scale = {1, 1, 1, 1};
  bool _dirty = false;
  /// applies full transformation (scale, rotation, translation) to given vector
  constexpr double4 transform(const double4& v) const noexcept {
    const auto sv = _scale * v;
    return {dot(double4(_rigid[0]), sv), dot(double4(_rigid[1]), sv), dot(double4(_rigid[2]), sv), v.w};
  }

public:
  /// returns translation component of transformation matrix
  constexpr double4 translation() const noexcept { return {_rigid[0][3], _rigid[1][3], _rigid[2][3], 0}; }
  /// sets translation component of transformation matrix
  constexpr void translation(const double3& t) noexcept {
    _rigid[0][3] = t.x, _rigid[1][3] = t.y, _rigid[2][3] = t.z;
    _dirty = true;
  }
  constexpr void translation(const double4& t) noexcept {
    _rigid[0][3] = t.x, _rigid[1][3] = t.y, _rigid[2][3] = t.z;
    _dirty = true;
  }
  /// returns rotation component of transformation matrix
  constexpr double4 rotation() const noexcept {
    double4 result;
    result.y = yw::asin(yw::clamp(-_rigid[2][0], -1.0, 1.0));
    if (yw::abs(_rigid[2][0]) < 1.0 - 1e-10) {
      result.x = yw::atan2(_rigid[2][1], _rigid[2][2]);
      result.z = yw::atan2(_rigid[1][0], _rigid[0][0]);
    } else result.x = yw::atan2(-_rigid[1][2], _rigid[1][1]);
    return result;
  }
  /// sets rotation component of transformation matrix
  constexpr void rotation(const double3& Radians) noexcept { _rotation(Radians); }
  constexpr void rotation(const double4& Radians) noexcept { _rotation(Radians); }
  /// sets scale component of transformation matrix
  constexpr void scale(const double3& s) noexcept {
    _scale = {s.x, s.y, s.z, 1};
    _dirty = true;
  }
  constexpr void scale(const double4& s) noexcept {
    _scale = s;
    _dirty = true;
  }
  /// gets scale component of transformation matrix
  constexpr const double4& scale() const noexcept { return _scale; }
  /// gets full transformation matrix ([T][R][S])
  constexpr matrix<double, 3, 4> transformation() const noexcept {
    return {
      matrix_row(double4(_rigid[0]) * _scale), matrix_row(double4(_rigid[1]) * _scale),
      matrix_row(double4(_rigid[2]) * _scale)};
  }
};

/// MARK: geom::circle

/// circle centered at origin with radius 1 and normal along +Z direction
template<backend Backend> class circle : public geometry_base<circle<Backend>, Backend> {
  friend class geometry_base<circle<Backend>, Backend>;
  select_type<Backend == cpu, none, uint32_t> _segment_count{};
  std::expected<void, error> _make_mesh(uint1 SegmentCount) noexcept;

public:
  /// gets center
  constexpr double4 center() const noexcept { return {this->_rigid[0][3], this->_rigid[1][3], this->_rigid[2][3], 1}; }
  /// gets normal vector
  constexpr double4 normal() const noexcept { return {this->_rigid[0][2], this->_rigid[1][2], this->_rigid[2][2], 0}; }
  /// gets signed area
  constexpr double area() const noexcept { return yw::pi * this->_scale.x * this->_scale.y; }
  /// projects point onto plane
  constexpr double4 project_to_plane(const double4& p) const noexcept {
    const auto n = normal();
    return p - n * dot(p - center(), n);
  }
  /// returns whether point is on plane
  constexpr bool on_plane(const double4& p, double eps = 1e-10) const noexcept {
    return yw::abs(dot(p - center(), normal())) <= eps;
  }
  /// returns whether point is inside disk
  constexpr bool contains(const double4& p, double eps = 1e-10) const noexcept {
    if (yw::abs(this->_scale.x) <= eps || yw::abs(this->_scale.y) <= eps) return false;
    if (!on_plane(p, eps)) return false;
    return local_radius2(p) <= 1.0 + eps;
  }
  /// returns whether point is on circumference
  constexpr bool on_boundary(const double4& p, double eps = 1e-10) const noexcept {
    if (yw::abs(this->_scale.x) <= eps || yw::abs(this->_scale.y) <= eps) return false;
    if (!on_plane(p, eps)) return false;
    return yw::abs(local_radius2(p) - 1.0) <= eps;
  }

private:
  constexpr double local_radius2(const double4& p) const noexcept {
    const auto d = p - center();
    const auto axis_x = double4{this->_rigid[0][0], this->_rigid[1][0], this->_rigid[2][0], 0};
    const auto axis_y = double4{this->_rigid[0][1], this->_rigid[1][1], this->_rigid[2][1], 0};
    const auto x = dot(d, axis_x) / this->_scale.x;
    const auto y = dot(d, axis_y) / this->_scale.y;
    return x * x + y * y;
  }
};

} // namespace yw::geom
