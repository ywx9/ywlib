#pragma once
#include <yw/geometry.h>

namespace yw::geom {

/// MARK: geom::segment

/// represents a 2D line segment as a transformed unit segment from (0, 0) to (1, 0).
template<backend Backend> class segment : public geometry_base<segment, Backend> {
public:
    /// constructs a 2D line segment as a transformed unit segment from (0, 0) to (1, 0).
  constexpr segment() noexcept = default;
  /// constructs a segment from the given endpoints.
  constexpr segment(const double2& start, const double2& end) noexcept { _set(start.x, start.y, end.x, end.y); }
  constexpr segment(const double3& start, const double3& end) noexcept { _set(start.x, start.y, end.x, end.y); }
  constexpr segment(const double4& start, const double4& end) noexcept { _set(start.x, start.y, end.x, end.y); }

  /// sets the endpoints of the segment.
  constexpr void set(const double2& start, const double2& end) noexcept { _set(start.x, start.y, end.x, end.y); }
  constexpr void set(const double3& start, const double3& end) noexcept { _set(start.x, start.y, end.x, end.y); }
  constexpr void set(const double4& start, const double4& end) noexcept { _set(start.x, start.y, end.x, end.y); }

  /// gets the start point of the segment.
  constexpr double4 begin() const noexcept { return {this->_rigid[0][3], this->_rigid[1][3], 0, 1}; }
  /// gets the end point of the segment.
  constexpr double4 end() const noexcept {
    return {
      this->_rigid[0][3] + this->_rigid[0][0] * this->_scale.x,
      this->_rigid[1][3] + this->_rigid[1][0] * this->_scale.x, 0, 1};
  }

  /// gets the signed length of the segment.
  constexpr double signed_length() const noexcept { return this->_scale.x; }
  /// gets the absolute length of the segment.
  constexpr double length() const noexcept { return yw::abs(this->_scale.x); }

protected:
  friend class geometry_base<segment, Backend>;
  std::expected<void, error> _triangulate(const geom::remeshing_option<segment>& ro) noexcept;

  constexpr void _set(double sx, double sy, double ex, double ey) noexcept {
    const auto dx = ex - sx, dy = ey - sy;
    const auto ln = yw::sqrt(dx * dx + dy * dy);
    this->_translation(sx, sy, 0);
    if (ln != 0) {
      this->_rotation(double3(0, 0, yw::atan2(dy, dx)));
      this->_set_scale(ln, 1, 1);
    } else {
      this->_rotation(double3(0, 0, 0));
      this->_set_scale(0, 1, 1);
    }
  }
};

/// MARK: geom::ray

template<backend Backend> class ray;
template<> struct remeshing_option<ray> {
  float1 length = 1e5f;
};

/// represents a 2D ray as a transformed unit ray from (0, 0) toward +X.
template<backend Backend> class ray : public geometry_base<ray, Backend> {
public:
  /// constructs a 2D ray from the origin toward +X.
  constexpr ray() noexcept = default;
  /// constructs a ray from the given origin and direction.
  constexpr ray(const double2& origin, const double2& direction) noexcept {
    this->_translation(origin.x, origin.y, 0);
    this->_rotation(double3(0, 0, yw::atan2(direction.y, direction.x)));
  }
  constexpr ray(const double3& origin, const double3& direction) noexcept : ray(origin.xy(), direction.xy()) {}
  constexpr ray(const double4& origin, const double4& direction) noexcept : ray(origin.xy(), direction.xy()) {}

  /// gets the origin point.
  constexpr double4 origin() const noexcept { return {this->_rigid[0][3], this->_rigid[1][3], 0, 1}; }
  /// sets the origin point.
  constexpr void origin(const double2& origin) noexcept { this->_translation(origin.x, origin.y, 0); }
  constexpr void origin(const double3& origin) noexcept { this->_translation(origin.x, origin.y, origin.z); }
  constexpr void origin(const double4& origin) noexcept { this->_translation(origin.x, origin.y, origin.z); }

  /// gets the direction vector.
  constexpr double4 direction() const noexcept { return {this->_rigid[0][0], this->_rigid[1][0], 0, 0}; }
  /// sets the direction vector.
  constexpr void direction(const double2& direction) noexcept {
    this->_rotation(double3(0, 0, yw::atan2(direction.y, direction.x)));
  }
  constexpr void direction(const double3& direction) noexcept {
    this->_rotation(double3(0, 0, yw::atan2(direction.y, direction.x)));
  }
  constexpr void direction(const double4& direction) noexcept {
    this->_rotation(double3(0, 0, yw::atan2(direction.y, direction.x)));
  }

protected:
  friend class geometry_base<ray, Backend>;
  std::expected<void, error> _triangulate(const geom::remeshing_option<ray>& ro) noexcept;
};

/// MARK: geom::line

template<backend Backend> class line;
template<> struct remeshing_option<line> {
  float1 half_length = 1e5f;
};

/// represents a 2D line as a transformed line through the origin along the X axis.
template<backend Backend> class line : public geometry_base<line, Backend> {
public:
  /// constructs a 2D line through the origin along the X axis.
  constexpr line() noexcept = default;
  /// constructs a line from the given point and direction.
  constexpr line(const double2& point, const double2& direction) noexcept {
    this->_translation(point.x, point.y, 0);
    this->_rotation(double3(0, 0, yw::atan2(direction.y, direction.x)));
  }
  constexpr line(const double3& point, const double3& direction) noexcept : line(point.xy(), direction.xy()) {}
  constexpr line(const double4& point, const double4& direction) noexcept : line(point.xy(), direction.xy()) {}

  /// gets a point on the line.
  constexpr double4 point() const noexcept { return {this->_rigid[0][3], this->_rigid[1][3], 0, 1}; }
  /// sets a point on the line.
  constexpr void point(const double2& point) noexcept { this->_translation(point.x, point.y, 0); }
  constexpr void point(const double3& point) noexcept { this->_translation(point.x, point.y, point.z); }
  constexpr void point(const double4& point) noexcept { this->_translation(point.x, point.y, point.z); }

  /// gets the direction vector.
  constexpr double4 direction() const noexcept { return {this->_rigid[0][0], this->_rigid[1][0], 0, 0}; }
  /// sets the direction vector.
  constexpr void direction(const double2& direction) noexcept {
    this->_rotation(double3(0, 0, yw::atan2(direction.y, direction.x)));
  }
  constexpr void direction(const double3& direction) noexcept {
    this->_rotation(double3(0, 0, yw::atan2(direction.y, direction.x)));
  }
  constexpr void direction(const double4& direction) noexcept {
    this->_rotation(double3(0, 0, yw::atan2(direction.y, direction.x)));
  }

protected:
  friend class geometry_base<line, Backend>;
  std::expected<void, error> _triangulate(const geom::remeshing_option<line>& ro) noexcept;
};

/// MARK: geom::arc

template<backend Backend> class arc;
template<> struct remeshing_option<arc> {
  uint1 subdivisions = 32;
};

/// represents a 2D circular arc centered at the origin with radius 1.
template<backend Backend> class arc : public geometry_base<arc, Backend> {
public:
  /// constructs a circular arc centered at the origin with radius 1.
  constexpr arc() noexcept = default;

  /// gets the start angle in radians.
  constexpr double start_angle() const noexcept { return _start_angle; }
  /// sets the start angle in radians.
  constexpr void start_angle(double radians) noexcept {
    _start_angle = radians;
    this->_messy = true;
  }
  /// gets the sweep angle in radians.
  constexpr double sweep_angle() const noexcept { return _sweep_angle; }
  /// sets the sweep angle in radians.
  constexpr void sweep_angle(double radians) noexcept {
    _sweep_angle = radians;
    this->_messy = true;
  }
  /// gets the end angle in radians.
  constexpr double end_angle() const noexcept { return _start_angle + _sweep_angle; }
  /// sets the start and sweep angles in radians.
  constexpr void angles(double start_radians, double sweep_radians) noexcept {
    _start_angle = start_radians;
    _sweep_angle = sweep_radians;
    this->_messy = true;
  }

  /// gets a point on the arc for t in [0, 1].
  constexpr double4 point(double t) const noexcept {
    const auto a = _start_angle + _sweep_angle * t;
    return {yw::cos(a), yw::sin(a), 0, 1};
  }

protected:
  friend class geometry_base<arc, Backend>;
  double _start_angle = 0;
  double _sweep_angle = yw::pi * 0.5;
  std::expected<void, error> _triangulate(const geom::remeshing_option<arc>& ro) noexcept;
};

/// MARK: geom::quadratic_bezier

template<backend Backend> class quadratic_bezier;
template<> struct remeshing_option<quadratic_bezier> {
  uint1 subdivisions = 32;
};

/// represents a 2D quadratic Bezier curve in local coordinates.
template<backend Backend> class quadratic_bezier : public geometry_base<quadratic_bezier, Backend> {
public:
  /// constructs a quadratic Bezier curve from (-1, 0) through (0, 1) to (1, 0).
  constexpr quadratic_bezier() noexcept = default;
  /// constructs a quadratic Bezier curve from the given control points.
  constexpr quadratic_bezier(const double2& p0, const double2& p1, const double2& p2) noexcept { points(p0, p1, p2); }
  constexpr quadratic_bezier(const double3& p0, const double3& p1, const double3& p2) noexcept { points(p0, p1, p2); }
  constexpr quadratic_bezier(const double4& p0, const double4& p1, const double4& p2) noexcept { points(p0, p1, p2); }

  /// gets the first control point.
  constexpr const double4& p0() const noexcept { return _p0; }
  /// sets the first control point.
  constexpr void p0(const double2& p) noexcept { p0({p.x, p.y, 0, 1}); }
  constexpr void p0(const double3& p) noexcept { p0({p.x, p.y, 0, 1}); }
  constexpr void p0(const double4& p) noexcept {
    _p0 = {p.x, p.y, 0, 1};
    this->_messy = true;
  }

  /// gets the second control point.
  constexpr const double4& p1() const noexcept { return _p1; }
  /// sets the second control point.
  constexpr void p1(const double2& p) noexcept { p1({p.x, p.y, 0, 1}); }
  constexpr void p1(const double3& p) noexcept { p1({p.x, p.y, 0, 1}); }
  constexpr void p1(const double4& p) noexcept {
    _p1 = {p.x, p.y, 0, 1};
    this->_messy = true;
  }

  /// gets the third control point.
  constexpr const double4& p2() const noexcept { return _p2; }
  /// sets the third control point.
  constexpr void p2(const double2& p) noexcept { p2({p.x, p.y, 0, 1}); }
  constexpr void p2(const double3& p) noexcept { p2({p.x, p.y, 0, 1}); }
  constexpr void p2(const double4& p) noexcept {
    _p2 = {p.x, p.y, 0, 1};
    this->_messy = true;
  }

  /// sets all control points.
  constexpr void points(const double2& p0, const double2& p1, const double2& p2) noexcept {
    points(double4(p0.x, p0.y, 0, 1), double4(p1.x, p1.y, 0, 1), double4(p2.x, p2.y, 0, 1));
  }
  constexpr void points(const double3& p0, const double3& p1, const double3& p2) noexcept {
    points(double4(p0.x, p0.y, 0, 1), double4(p1.x, p1.y, 0, 1), double4(p2.x, p2.y, 0, 1));
  }
  constexpr void points(const double4& p0, const double4& p1, const double4& p2) noexcept {
    _p0 = {p0.x, p0.y, 0, 1};
    _p1 = {p1.x, p1.y, 0, 1};
    _p2 = {p2.x, p2.y, 0, 1};
    this->_messy = true;
  }
  /// gets a point on the curve for t in [0, 1].
  constexpr double4 point(double t) const noexcept {
    const auto u = 1.0 - t;
    return _p0 * (u * u) + _p1 * (2.0 * u * t) + _p2 * (t * t);
  }

protected:
  friend class geometry_base<quadratic_bezier, Backend>;
  double4 _p0 = {-1, 0, 0, 1}, _p1 = {0, 1, 0, 1}, _p2 = {1, 0, 0, 1};
  std::expected<void, error> _triangulate(const geom::remeshing_option<quadratic_bezier>& ro) noexcept;
};

/// MARK: geom::cubic_bezier

template<backend Backend> class cubic_bezier;
template<> struct remeshing_option<cubic_bezier> {
  uint1 subdivisions = 32;
};

/// represents a 2D cubic Bezier curve in local coordinates.
template<backend Backend> class cubic_bezier : public geometry_base<cubic_bezier, Backend> {
public:
  /// constructs a cubic Bezier curve from (-1, 0) to (1, 0).
  constexpr cubic_bezier() noexcept = default;
  /// constructs a cubic Bezier curve from the given control points.
  constexpr cubic_bezier(const double2& p0, const double2& p1, const double2& p2, const double2& p3) noexcept {
    points(p0, p1, p2, p3);
  }
  constexpr cubic_bezier(const double3& p0, const double3& p1, const double3& p2, const double3& p3) noexcept {
    points(p0, p1, p2, p3);
  }
  constexpr cubic_bezier(const double4& p0, const double4& p1, const double4& p2, const double4& p3) noexcept {
    points(p0, p1, p2, p3);
  }

  /// gets the first control point.
  constexpr const double4& p0() const noexcept { return _p0; }
  /// sets the first control point.
  constexpr void p0(const double2& p) noexcept { p0({p.x, p.y, 0, 1}); }
  constexpr void p0(const double3& p) noexcept { p0({p.x, p.y, 0, 1}); }
  constexpr void p0(const double4& p) noexcept {
    _p0 = {p.x, p.y, 0, 1};
    this->_messy = true;
  }

  /// gets the second control point.
  constexpr const double4& p1() const noexcept { return _p1; }
  /// sets the second control point.
  constexpr void p1(const double2& p) noexcept { p1({p.x, p.y, 0, 1}); }
  constexpr void p1(const double3& p) noexcept { p1({p.x, p.y, 0, 1}); }
  constexpr void p1(const double4& p) noexcept {
    _p1 = {p.x, p.y, 0, 1};
    this->_messy = true;
  }

  /// gets the third control point.
  constexpr const double4& p2() const noexcept { return _p2; }
  /// sets the third control point.
  constexpr void p2(const double2& p) noexcept { p2({p.x, p.y, 0, 1}); }
  constexpr void p2(const double3& p) noexcept { p2({p.x, p.y, 0, 1}); }
  constexpr void p2(const double4& p) noexcept {
    _p2 = {p.x, p.y, 0, 1};
    this->_messy = true;
  }

  /// gets the fourth control point.
  constexpr const double4& p3() const noexcept { return _p3; }
  /// sets the fourth control point.
  constexpr void p3(const double2& p) noexcept { p3({p.x, p.y, 0, 1}); }
  constexpr void p3(const double3& p) noexcept { p3({p.x, p.y, 0, 1}); }
  constexpr void p3(const double4& p) noexcept {
    _p3 = {p.x, p.y, 0, 1};
    this->_messy = true;
  }

  /// sets all control points.
  constexpr void points(const double2& p0, const double2& p1, const double2& p2, const double2& p3) noexcept {
    points(double4(p0.x, p0.y, 0, 1), double4(p1.x, p1.y, 0, 1), double4(p2.x, p2.y, 0, 1), double4(p3.x, p3.y, 0, 1));
  }
  constexpr void points(const double3& p0, const double3& p1, const double3& p2, const double3& p3) noexcept {
    points(double4(p0.x, p0.y, 0, 1), double4(p1.x, p1.y, 0, 1), double4(p2.x, p2.y, 0, 1), double4(p3.x, p3.y, 0, 1));
  }
  constexpr void points(const double4& p0, const double4& p1, const double4& p2, const double4& p3) noexcept {
    _p0 = {p0.x, p0.y, 0, 1};
    _p1 = {p1.x, p1.y, 0, 1};
    _p2 = {p2.x, p2.y, 0, 1};
    _p3 = {p3.x, p3.y, 0, 1};
    this->_messy = true;
  }

  /// gets a point on the curve for t in [0, 1].
  constexpr double4 point(double t) const noexcept {
    const auto u = 1.0 - t;
    return _p0 * (u * u * u) + _p1 * (3.0 * u * u * t) + _p2 * (3.0 * u * t * t) + _p3 * (t * t * t);
  }

protected:
  friend class geometry_base<cubic_bezier, Backend>;
  double4 _p0 = {-1, 0, 0, 1}, _p1 = {-0.5, 1, 0, 1}, _p2 = {0.5, -1, 0, 1}, _p3 = {1, 0, 0, 1};
  std::expected<void, error> _triangulate(const geom::remeshing_option<cubic_bezier>& ro) noexcept;
};

/// MARK: geom::plane

template<backend Backend> class plane;
template<> struct remeshing_option<plane> {
  float1 half_extent = 1e5f;
};

/// represents a 2D plane as a finite square mesh centered at the origin.
template<backend Backend> class plane : public geometry_base<plane, Backend> {
public:
  /// constructs a finite mesh representation of the XY plane.
  constexpr plane() noexcept = default;
  /// gets the center point.
  constexpr double4 center() const noexcept { return {this->_rigid[0][3], this->_rigid[1][3], 0, 1}; }
  /// gets the normal vector.
  constexpr double4 normal() const noexcept { return {this->_rigid[0][2], this->_rigid[1][2], this->_rigid[2][2], 0}; }

protected:
  friend class geometry_base<plane, Backend>;
  std::expected<void, error> _triangulate(const geom::remeshing_option<plane>& ro) noexcept;
};

/// MARK: geom::square

/// represents a 2D square centered at the origin with corners at (+/-1, +/-1).
template<backend Backend> class square : public geometry_base<square, Backend> {
public:
  static constexpr bool has_bounded_surface = true;
  /// constructs a square centered at the origin with corners at (+/-1, +/-1).
  constexpr square() noexcept = default;
  /// gets the center point.
  constexpr double4 center() const noexcept { return {this->_rigid[0][3], this->_rigid[1][3], 0, 1}; }
  /// gets the normal vector.
  constexpr double4 normal() const noexcept { return {this->_rigid[0][2], this->_rigid[1][2], this->_rigid[2][2], 0}; }
  /// gets the signed area.
  constexpr double area() const noexcept { return 4.0 * this->_scale.x * this->_scale.y; }

protected:
  friend class geometry_base<square, Backend>;
  std::expected<void, error> _triangulate(const geom::remeshing_option<square>& ro) noexcept;
};

/// MARK: geom::circle

template<backend Backend> class circle;
template<> struct remeshing_option<circle> {
  uint1 subdivisions = 32;
};

/// represents a 2D circle centered at the origin with radius 1.
template<backend Backend> class circle : public geometry_base<circle, Backend> {
public:
  static constexpr bool has_bounded_surface = true;
  /// constructs a circle centered at the origin with radius 1.
  constexpr circle() noexcept = default;
  /// gets the center point.
  constexpr double4 center() const noexcept { return {this->_rigid[0][3], this->_rigid[1][3], 0, 1}; }
  /// gets the normal vector.
  constexpr double4 normal() const noexcept { return {this->_rigid[0][2], this->_rigid[1][2], this->_rigid[2][2], 0}; }
  /// gets the signed area.
  constexpr double area() const noexcept { return yw::pi * this->_scale.x * this->_scale.y; }

protected:
  friend class geometry_base<circle, Backend>;
  std::expected<void, error> _triangulate(const geom::remeshing_option<circle>& ro) noexcept;
};

/// MARK: geom::polygon

/// represents a 2D polygon with editable local-coordinate points.
template<backend Backend> class polygon : public geometry_base<polygon, Backend> {
public:
  static constexpr bool has_bounded_surface = true;
  /// constructs an empty polygon.
  constexpr polygon() noexcept = default;
  /// gets the polygon points.
  constexpr const auto& points() const noexcept { return _points; }
  /// gets the number of polygon points.
  constexpr size_t size() const noexcept { return _points.size(); }
  /// returns true if the polygon has no points.
  constexpr bool empty() const noexcept { return _points.empty(); }
  /// gets a polygon point.
  constexpr const double4& operator[](size_t index) const noexcept { return _points[index]; }

  /// resizes the polygon point list.
  std::expected<void, error> resize(size_t size) noexcept {
    if (const auto old_size = _points.size(); size == old_size) return {};
    else if (auto res = _points.resize(size); !res) return res.error().relay();
    else if (size > old_size)
      for (size_t i = old_size; i < size; ++i) _points[i] = {0, 0, 0, 1};
    this->_messy = true;
    return {};
  }
  /// assigns points from the given array, replacing existing points.
  std::expected<void, error> assign(const double4* points, size_t count) noexcept {
    if (auto res = _points.assign(points, count); !res) return res.error().relay();
    for (auto& p : _points) p = {p.x, p.y, 0, 1};
    this->_messy = true;
    return {};
  }
  template<contiguous_range<double4> Rg> std::expected<void, error> assign(Rg&& points) noexcept {
    return assign(yw::data(points), yw::size(points));
  }
  /// appends a point to the polygon.
  std::expected<void, error> push_back(const double2& point) noexcept {
    if (auto res = _points.push_back({point.x, point.y, 0, 1}); !res) return res.error().relay();
    this->_messy = true;
    return {};
  }
  std::expected<void, error> push_back(const double3& point) noexcept {
    if (auto res = _points.push_back({point.x, point.y, 0, 1}); !res) return res.error().relay();
    this->_messy = true;
    return {};
  }
  std::expected<void, error> push_back(const double4& point) noexcept {
    if (auto res = _points.push_back({point.x, point.y, 0, 1}); !res) return res.error().relay();
    this->_messy = true;
    return {};
  }
  /// sets a polygon point.
  std::expected<void, error> set(size_t index, const double2& point) noexcept {
    if (index >= _points.size())
      return std::unexpected(error(errors::invalid_argument, "polygon point index out of bounds"));
    _points[index] = {point.x, point.y, 0, 1};
    this->_messy = true;
    return {};
  }
  std::expected<void, error> set(size_t index, const double3& point) noexcept { return set(index, point.xy()); }
  std::expected<void, error> set(size_t index, const double4& point) noexcept { return set(index, point.xy()); }
  /// removes all polygon points.
  constexpr void clear() noexcept {
    _points.clear();
    this->_messy = true;
  }

protected:
  friend class geometry_base<polygon, Backend>;
  array1<double4, cpu> _points;
  std::expected<void, error> _triangulate(const geom::remeshing_option<polygon>& ro) noexcept;
};
} // namespace yw::geom
