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
  // constexpr double4 translation() const noexcept { return {_rigid[0][3], _rigid[1][3], _rigid[2][3], 0}; }
  // constexpr void translation(const double3& t) noexcept { _translation(t.x, t.y, t.z); }
  // constexpr void translation(const double4& t) noexcept { _translation(t.x, t.y, t.z); }

  // constexpr double4 rotation() const noexcept {
  //   double4 result;
  //   result.y = yw::asin(yw::clamp(-_rigid[2][0], -1.0, 1.0));
  //   if (yw::abs(_rigid[2][0]) < 1.0 - 1e-10) {
  //     result.x = yw::atan2(_rigid[2][1], _rigid[2][2]);
  //     result.z = yw::atan2(_rigid[1][0], _rigid[0][0]);
  //   } else result.x = yw::atan2(-_rigid[1][2], _rigid[1][1]);
  //   return result;
  // }
  // constexpr void rotation(const double3& Radians) noexcept { _rotation(Radians); }
  // constexpr void rotation(const double4& Radians) noexcept { _rotation(Radians); }

  // constexpr const double4& scale() const noexcept { return _scale; }
  // constexpr void scale(const double3& s) noexcept { _set_scale(s.x, s.y, s.z); }
  // constexpr void scale(const double4& s) noexcept { _set_scale(s.x, s.y, s.z); }

  // constexpr matrix<double, 3, 4> transformation3() const noexcept {
  //   return {
  //     matrix_row(double4(_rigid[0]) * _scale), matrix_row(double4(_rigid[1]) * _scale),
  //     matrix_row(double4(_rigid[2]) * _scale)};
  // }
  // constexpr matrix<double, 3, 4> transformation() const noexcept { return transformation3(); }
};

// /// MARK: geom::segment

// template<backend Backend> class segment : public geometry_base<segment<Backend>, Backend> {
//   friend class geometry_base<segment<Backend>, Backend>;
//   double4 _start = {-1, 0, 0, 1}, _end = {1, 0, 0, 1};
//   std::expected<void, error> _make_mesh() noexcept;
//   std::expected<void, error> _make_mesh_default() noexcept { return _make_mesh(); }

// public:
//   /// returns start point
//   constexpr const double4& start() const noexcept { return _start; }
//   /// sets start point
//   constexpr void start(const double3& s) noexcept {
//     _start = {s.x, s.y, s.z, 1};
//     this->_messy = true;
//   }
//   constexpr void start(const double4& s) noexcept { start(s.xyz()); }

//   /// returns end point
//   constexpr const double4& end() const noexcept { return _end; }
//   /// sets end point
//   constexpr void end(const double3& e) noexcept {
//     _end = {e.x, e.y, e.z, 1};
//     this->_messy = true;
//   }
//   constexpr void end(const double4& e) noexcept { end(e.xyz()); }

//   /// returns center point
//   constexpr double4 center() const noexcept { return (_start + _end) * 0.5; }
//   /// returns direction vector (from start to end)
//   constexpr double4 direction() const noexcept { return _end - _start; }
//   /// returns length of segment
//   constexpr double length() const noexcept { return direction().length(); }
//   /// returns squared length of segment
//   constexpr double squared_length() const noexcept { return direction().squared_length(); }
// };

// /// MARK: geom::ray

// template<backend Backend> class ray : public geometry_base<ray<Backend>, Backend> {
//   friend class geometry_base<ray<Backend>, Backend>;
//   double4 _origin = {0, 0, 0, 1}, _direction = {1, 0, 0, 0};
//   float _length = 1e10f;
//   std::expected<void, error> _make_mesh(float1 Length) noexcept;
//   std::expected<void, error> _make_mesh_default() noexcept { return _make_mesh(_length); }

// public:
//   /// returns origin point
//   constexpr const double4& origin() const noexcept { return _origin; }
//   /// sets origin point
//   constexpr void origin(const double3& o) noexcept {
//     _origin = {o.x, o.y, o.z, 1};
//     this->_messy = true;
//   }
//   constexpr void origin(const double4& o) noexcept { origin(o.xyz()); }

//   /// returns direction vector
//   constexpr const double4& direction() const noexcept { return _direction; }
//   /// sets direction vector
//   constexpr void direction(const double3& d) noexcept {
//     _direction = double4(d.x, d.y, d.z, 0).normalized();
//     this->_messy = true;
//   }
//   constexpr void direction(const double4& d) noexcept { direction(d.xyz()); }
// };

// /// MARK: geom::line

// template<backend Backend> class line : public geometry_base<line<Backend>, Backend> {
//   friend class geometry_base<line<Backend>, Backend>;
//   double4 _point = {0, 0, 0, 1}, _direction = {1, 0, 0, 0};
//   float _half_length = 1e10f;
//   std::expected<void, error> _make_mesh(float1 HalfLength) noexcept;
//   std::expected<void, error> _make_mesh_default() noexcept { return _make_mesh(_half_length); }

// public:
//   /// returns point on line
//   constexpr const double4& point() const noexcept { return _point; }
//   /// sets point on line
//   constexpr void point(const double3& p) noexcept {
//     _point = {p.x, p.y, p.z, 1};
//     this->_messy = true;
//   }
//   constexpr void point(const double4& p) noexcept { point(p.xyz()); }
//   /// returns direction vector
//   constexpr const double4& direction() const noexcept { return _direction; }
//   /// sets direction vector
//   constexpr void direction(const double3& d) noexcept {
//     _direction = double4(d.x, d.y, d.z, 0).normalized();
//     this->_messy = true;
//   }
//   constexpr void direction(const double4& d) noexcept { direction(d.xyz()); }
// };

// /// MARK: geom::polyline

// template<backend Backend> class polyline : public geometry_base<polyline<Backend>, Backend> {
//   friend class geometry_base<polyline<Backend>, Backend>;
//   array1<double4> _points;
//   std::expected<void, error> _make_mesh() noexcept;
//   std::expected<void, error> _make_mesh_default() noexcept { return _make_mesh(); }

// public:
//   constexpr const array1<double4>& points() const noexcept { return _points; }
//   constexpr size_t size() const noexcept { return _points.size(); }
//   constexpr bool empty() const noexcept { return _points.empty(); }

//   constexpr const double4& operator[](size_t Index) const noexcept { return _points[Index]; }

//   std::expected<void, error> resize(size_t Size) noexcept {
//     if (auto res = _points.resize(Size); !res) return res.error().relay();
//     this->_messy = true;
//     return {};
//   }

//   std::expected<void, error> assign(const double4* Points, size_t Count) noexcept {
//     if (auto res = _points.assign(Points, Count); !res) return res.error().relay();
//     this->_messy = true;
//     return {};
//   }

//   template<contiguous_range<double4> Rg> std::expected<void, error> assign(Rg&& Points) noexcept {
//     return assign(yw::data(Points), yw::size(Points));
//   }

//   std::expected<void, error> push_back(const double3& Point) noexcept {
//     return push_back({Point.x, Point.y, Point.z, 1});
//   }

//   std::expected<void, error> push_back(const double4& Point) noexcept {
//     if (auto res = _points.push_back({Point.x, Point.y, Point.z, 1}); !res) return res.error().relay();
//     this->_messy = true;
//     return {};
//   }

//   constexpr void set(size_t Index, const double3& Point) noexcept { set(Index, {Point.x, Point.y, Point.z, 1}); }

//   constexpr void set(size_t Index, const double4& Point) noexcept {
//     _points[Index] = {Point.x, Point.y, Point.z, 1};
//     this->_messy = true;
//   }

//   constexpr void clear() noexcept {
//     _points.clear();
//     this->_messy = true;
//   }
// };

// /// MARK: geom::arc

// /// circular arc on XY plane with radius 1 and normal along +Z direction
// template<backend Backend> class arc : public geometry_base<arc<Backend>, Backend> {
//   friend class geometry_base<arc<Backend>, Backend>;
//   double _start_angle = 0;
//   double _sweep_angle = yw::pi * 0.5;
//   uint32_t _segment_count = 8;
//   std::expected<void, error> _make_mesh(uint1 SegmentCount) noexcept;
//   std::expected<void, error> _make_mesh_default() noexcept { return _make_mesh(_segment_count); }

// public:
//   constexpr double start_angle() const noexcept { return _start_angle; }
//   constexpr void start_angle(double Radians) noexcept {
//     _start_angle = Radians;
//     this->_messy = true;
//   }

//   constexpr double sweep_angle() const noexcept { return _sweep_angle; }
//   constexpr void sweep_angle(double Radians) noexcept {
//     _sweep_angle = Radians;
//     this->_messy = true;
//   }

//   constexpr double end_angle() const noexcept { return _start_angle + _sweep_angle; }
//   constexpr void angles(double StartRadians, double SweepRadians) noexcept {
//     _start_angle = StartRadians;
//     _sweep_angle = SweepRadians;
//     this->_messy = true;
//   }
// };

// /// MARK: geom::quadratic_bezier

// template<backend Backend> class quadratic_bezier : public geometry_base<quadratic_bezier<Backend>, Backend> {
//   friend class geometry_base<quadratic_bezier<Backend>, Backend>;
//   double4 _p0 = {-1, 0, 0, 1}, _p1 = {0, 1, 0, 1}, _p2 = {1, 0, 0, 1};
//   uint32_t _segment_count = 16;
//   std::expected<void, error> _make_mesh(uint1 SegmentCount) noexcept;
//   std::expected<void, error> _make_mesh_default() noexcept { return _make_mesh(_segment_count); }

// public:
//   constexpr const double4& p0() const noexcept { return _p0; }
//   constexpr const double4& p1() const noexcept { return _p1; }
//   constexpr const double4& p2() const noexcept { return _p2; }

//   constexpr void p0(const double3& Point) noexcept { p0({Point.x, Point.y, Point.z, 1}); }
//   constexpr void p0(const double4& Point) noexcept {
//     _p0 = {Point.x, Point.y, Point.z, 1};
//     this->_messy = true;
//   }

//   constexpr void p1(const double3& Point) noexcept { p1({Point.x, Point.y, Point.z, 1}); }
//   constexpr void p1(const double4& Point) noexcept {
//     _p1 = {Point.x, Point.y, Point.z, 1};
//     this->_messy = true;
//   }

//   constexpr void p2(const double3& Point) noexcept { p2({Point.x, Point.y, Point.z, 1}); }
//   constexpr void p2(const double4& Point) noexcept {
//     _p2 = {Point.x, Point.y, Point.z, 1};
//     this->_messy = true;
//   }

//   constexpr void points(const double4& P0, const double4& P1, const double4& P2) noexcept {
//     _p0 = {P0.x, P0.y, P0.z, 1};
//     _p1 = {P1.x, P1.y, P1.z, 1};
//     _p2 = {P2.x, P2.y, P2.z, 1};
//     this->_messy = true;
//   }

//   constexpr double4 point(double T) const noexcept {
//     const auto u = 1.0 - T;
//     return _p0 * (u * u) + _p1 * (2.0 * u * T) + _p2 * (T * T);
//   }
// };

// /// MARK: geom::cubic_bezier

// template<backend Backend> class cubic_bezier : public geometry_base<cubic_bezier<Backend>, Backend> {
//   friend class geometry_base<cubic_bezier<Backend>, Backend>;
//   double4 _p0 = {-1, 0, 0, 1}, _p1 = {-0.5, 1, 0, 1}, _p2 = {0.5, -1, 0, 1}, _p3 = {1, 0, 0, 1};
//   uint32_t _segment_count = 24;
//   std::expected<void, error> _make_mesh(uint1 SegmentCount) noexcept;
//   std::expected<void, error> _make_mesh_default() noexcept { return _make_mesh(_segment_count); }

// public:
//   constexpr const double4& p0() const noexcept { return _p0; }
//   constexpr const double4& p1() const noexcept { return _p1; }
//   constexpr const double4& p2() const noexcept { return _p2; }
//   constexpr const double4& p3() const noexcept { return _p3; }

//   constexpr void p0(const double3& Point) noexcept { p0({Point.x, Point.y, Point.z, 1}); }
//   constexpr void p0(const double4& Point) noexcept {
//     _p0 = {Point.x, Point.y, Point.z, 1};
//     this->_messy = true;
//   }

//   constexpr void p1(const double3& Point) noexcept { p1({Point.x, Point.y, Point.z, 1}); }
//   constexpr void p1(const double4& Point) noexcept {
//     _p1 = {Point.x, Point.y, Point.z, 1};
//     this->_messy = true;
//   }

//   constexpr void p2(const double3& Point) noexcept { p2({Point.x, Point.y, Point.z, 1}); }
//   constexpr void p2(const double4& Point) noexcept {
//     _p2 = {Point.x, Point.y, Point.z, 1};
//     this->_messy = true;
//   }

//   constexpr void p3(const double3& Point) noexcept { p3({Point.x, Point.y, Point.z, 1}); }
//   constexpr void p3(const double4& Point) noexcept {
//     _p3 = {Point.x, Point.y, Point.z, 1};
//     this->_messy = true;
//   }

//   constexpr void points(const double4& P0, const double4& P1, const double4& P2, const double4& P3) noexcept {
//     _p0 = {P0.x, P0.y, P0.z, 1};
//     _p1 = {P1.x, P1.y, P1.z, 1};
//     _p2 = {P2.x, P2.y, P2.z, 1};
//     _p3 = {P3.x, P3.y, P3.z, 1};
//     this->_messy = true;
//   }

//   constexpr double4 point(double T) const noexcept {
//     const auto u = 1.0 - T;
//     return _p0 * (u * u * u) + _p1 * (3.0 * u * u * T) + _p2 * (3.0 * u * T * T) + _p3 * (T * T * T);
//   }
// };

// /// MARK: geom::circle

// /// circle centered at origin with radius 1 and normal along +Z direction
// template<backend Backend> class circle : public geometry_base<circle<Backend>, Backend> {
//   friend class geometry_base<circle<Backend>, Backend>;
//   uint32_t _segment_count = 8;
//   std::expected<void, error> _make_mesh(uint1 SegmentCount) noexcept;
//   std::expected<void, error> _make_mesh_default() noexcept { return _make_mesh(_segment_count); }

// public:
//   /// gets center
//   constexpr double4 center() const noexcept { return {this->_rigid[0][3], this->_rigid[1][3], this->_rigid[2][3], 1};
//   }
//   /// gets normal vector
//   constexpr double4 normal() const noexcept { return {this->_rigid[0][2], this->_rigid[1][2], this->_rigid[2][2], 0};
//   }
//   /// gets signed area
//   constexpr double area() const noexcept { return yw::pi * this->_scale.x * this->_scale.y; }
// };

// /// MARK: geom::square

// /// square on XY plane with vertices at (±1, ±1, 0)
// template<backend Backend> class square : public geometry_base<square<Backend>, Backend> {
//   friend class geometry_base<square<Backend>, Backend>;
//   std::expected<void, error> _make_mesh() noexcept;
//   std::expected<void, error> _make_mesh_default() noexcept { return _make_mesh(); }

// public:
//   constexpr double4 center() const noexcept { return {this->_rigid[0][3], this->_rigid[1][3], this->_rigid[2][3], 1};
//   } constexpr double4 normal() const noexcept { return {this->_rigid[0][2], this->_rigid[1][2], this->_rigid[2][2],
//   0}; } constexpr double area() const noexcept { return 4.0 * this->_scale.x * this->_scale.y; }
// };

// /// MARK: geom::plane

// /// XY plane with normal along +Z direction
// template<backend Backend> class plane : public geometry_base<plane<Backend>, Backend> {
//   friend class geometry_base<plane<Backend>, Backend>;
//   float _half_extent = 1e5f;
//   std::expected<void, error> _make_mesh(float1 HalfExtent) noexcept;
//   std::expected<void, error> _make_mesh_default() noexcept { return _make_mesh(_half_extent); }

// public:
//   constexpr double4 point() const noexcept { return {this->_rigid[0][3], this->_rigid[1][3], this->_rigid[2][3], 1};
//   } constexpr double4 normal() const noexcept { return {this->_rigid[0][2], this->_rigid[1][2], this->_rigid[2][2],
//   0}; }
// };

// /// MARK: geom::polygon

// template<backend Backend> class polygon : public geometry_base<polygon<Backend>, Backend> {
//   friend class geometry_base<polygon<Backend>, Backend>;
//   array1<double4> _points;
//   std::expected<void, error> _make_mesh() noexcept;
//   std::expected<void, error> _make_mesh_default() noexcept { return _make_mesh(); }

// public:
//   constexpr const array1<double4>& points() const noexcept { return _points; }
//   constexpr size_t size() const noexcept { return _points.size(); }
//   constexpr bool empty() const noexcept { return _points.empty(); }

//   constexpr const double4& operator[](size_t Index) const noexcept { return _points[Index]; }

//   std::expected<void, error> resize(size_t Size) noexcept {
//     if (const auto old_size = _points.size(); Size == old_size) return {};
//     else if (auto res = _points.resize(Size); !res) return res.error().relay();
//     else if (Size > old_size)
//       for (size_t i = old_size; i < Size; ++i) _points[i] = {0, 0, 0, 1};
//     this->_messy = true;
//     return {};
//   }

//   /// assigns points from given array, replacing existing points
//   /// \note `w` component of each point must be 1.
//   std::expected<void, error> assign(const double4* Points, size_t Count) noexcept {
//     if (auto res = _points.assign(Points, Count); !res) return res.error().relay();
//     this->_messy = true;
//     return {};
//   }

//   template<contiguous_range<double4> Rg> std::expected<void, error> assign(Rg&& Points) noexcept {
//     return assign(yw::data(Points), yw::size(Points));
//   }

//   std::expected<void, error> push_back(const double3& Point) noexcept {
//     if (auto res = _points.push_back(double4(Point, 1)); !res) return res.error().relay();
//     this->_messy = true;
//     return {};
//   }

//   /// assigns a point to the end of the polygon, replacing existing points
//   /// \note `w` component of the point must be 1.
//   std::expected<void, error> push_back(const double4& Point) noexcept {
//     if (auto res = _points.push_back(Point); !res) return res.error().relay();
//     this->_messy = true;
//     return {};
//   }

//   constexpr std::expected<void, error> set(size_t Index, const double3& Point) noexcept {
//     if (Index >= _points.size()) return std::unexpected(error(errors::invalid_argument, "Index out of bounds"));
//     _points[Index] = double4(Point, 1);
//     this->_messy = true;
//     return {};
//   }

//   /// assigns a point to the polygon at the given index
//   /// \note `w` component of the point must be 1.
//   constexpr std::expected<void, error> set(size_t Index, const double4& Point) noexcept {
//     if (Index >= _points.size()) return std::unexpected(error(errors::invalid_argument, "Index out of bounds"));
//     _points[Index] = Point;
//     this->_messy = true;
//     return {};
//   }

//   constexpr void clear() noexcept {
//     _points.clear();
//     this->_messy = true;
//   }
// };

} // namespace yw::geom
