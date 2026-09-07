#pragma once
#include <yw/geometry.h>

namespace yw::geom {

namespace detail {
constexpr double positive_tolerance(double tolerance) noexcept { return yw::max(tolerance < 0 ? -tolerance : tolerance, 1e-12); }

constexpr double local_tolerance(const double4& scale, double tolerance) noexcept {
  static_cast<void>(scale);
  return positive_tolerance(tolerance);
}

constexpr double point_segment_distance(double2 p, double2 a, double2 b) noexcept {
  const auto ab = b - a;
  const auto len2 = ab.squared_length();
  if (len2 <= 0) return (p - a).length();
  const auto t = yw::clamp(dot(p - a, ab) / len2, 0.0, 1.0);
  return (p - (a + ab * t)).length();
}

constexpr bool point_on_segment(double2 p, double2 a, double2 b, double tolerance) noexcept {
  return point_segment_distance(p, a, b) <= positive_tolerance(tolerance);
}

constexpr double cross_xy(double2 a, double2 b) noexcept { return a.x * b.y - a.y * b.x; }

constexpr bool angle_in_sweep(double angle, double start, double sweep, double tolerance) noexcept {
  if (sweep == 0) return yw::abs(yw::atan2(yw::sin(angle - start), yw::cos(angle - start))) <= tolerance;
  const auto two_pi = yw::pi2;
  auto delta = yw::fmod(angle - start, two_pi);
  if (delta < 0) delta += two_pi;
  if (sweep > 0) return delta <= sweep + tolerance || delta >= two_pi - tolerance;
  auto neg_delta = delta == 0 ? 0.0 : delta - two_pi;
  return neg_delta >= sweep - tolerance || neg_delta <= -two_pi + tolerance;
}

template<typename Curve> constexpr bool point_on_curve(double2 p, const Curve& curve, double tolerance) noexcept {
  constexpr uint32_t n = 64;
  auto prev = curve.point(0.0).xy();
  for (uint32_t i = 1; i <= n; ++i) {
    const auto next = curve.point(double(i) / double(n)).xy();
    if (point_on_segment(p, prev, next, tolerance)) return true;
    prev = next;
  }
  return false;
}
} // namespace detail

/// MARK: geom::segment

/// represents a 2D line segment as a transformed unit segment from (0, 0) to (1, 0).
template<backend Backend> class segment : public geometry_base<segment, Backend> {
public:
  /// constructs a 2D line segment as a transformed unit segment from (0, 0) to (1, 0).
  constexpr segment() noexcept = default;
  /// configures the unit segment transformation from XY endpoints; input z/w are ignored.
  constexpr segment(const double2& start, const double2& end) noexcept { _set(start.x, start.y, end.x, end.y); }
  constexpr segment(const double3& start, const double3& end) noexcept { _set(start.x, start.y, end.x, end.y); }
  constexpr segment(const double4& start, const double4& end) noexcept { _set(start.x, start.y, end.x, end.y); }

  /// replaces the unit segment transformation from XY endpoints; input z/w are ignored.
  constexpr void set(const double2& start, const double2& end) noexcept { _set(start.x, start.y, end.x, end.y); }
  constexpr void set(const double3& start, const double3& end) noexcept { _set(start.x, start.y, end.x, end.y); }
  constexpr void set(const double4& start, const double4& end) noexcept { _set(start.x, start.y, end.x, end.y); }

  /// gets the start point of the unit segment in local coordinates.
  constexpr double4 begin() const noexcept { return {0, 0, 0, 1}; }
  /// gets the end point of the unit segment in local coordinates.
  constexpr double4 end() const noexcept { return {1, 0, 0, 1}; }

  /// gets the local signed length of the unit segment.
  constexpr double signed_length() const noexcept { return 1; }
  /// gets the local length of the unit segment.
  constexpr double length() const noexcept { return 1; }
  /// gets the signed length after scaling.
  constexpr double world_signed_length() const noexcept { return this->_scale.x; }
  /// gets the absolute length in world coordinates.
  constexpr double world_length() const noexcept { return yw::abs(this->_scale.x); }
  /// gets the local-coordinate bounding box.
  constexpr geom::bbox<cpu> bbox() const noexcept { return {{0, 0, 0, 1}, {1, 0, 0, 1}}; }
  /// classifies a world coordinate against this segment.
  constexpr point_relation contains(const double4& world, double tolerance = 0) const noexcept {
    const auto p = this->local_point(world).xy();
    const auto t = detail::local_tolerance(this->_scale, tolerance);
    return detail::point_on_segment(p, {0, 0}, {1, 0}, t) ? point_relation::boundary : point_relation::outside;
  }
  constexpr point_relation contains(const double3& world, double tolerance = 0) const noexcept {
    return contains(double4(world, 1), tolerance);
  }
  constexpr point_relation contains(const double2& world, double tolerance = 0) const noexcept {
    return contains(double4(world.x, world.y, 0, 1), tolerance);
  }

protected:
  friend class geometry_base<segment, Backend>;
  std::expected<void, error> _triangulate() noexcept;

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
  /// configures the unit ray transformation from an XY origin and direction.
  constexpr ray(const double2& origin, const double2& direction) noexcept {
    this->_translation(origin.x, origin.y, 0);
    this->_rotation(double3(0, 0, yw::atan2(direction.y, direction.x)));
  }
  constexpr ray(const double3& origin, const double3& direction) noexcept : ray(origin.xy(), direction.xy()) {}
  constexpr ray(const double4& origin, const double4& direction) noexcept : ray(origin.xy(), direction.xy()) {}

  /// gets the origin point in local coordinates.
  constexpr double4 origin() const noexcept { return {0, 0, 0, 1}; }
  /// sets the XY translation component; the local origin remains zero and input z/w are ignored.
  constexpr void origin(const double2& origin) noexcept { this->_translation(origin.x, origin.y, 0); }
  constexpr void origin(const double3& origin) noexcept { this->_translation(origin.x, origin.y, 0); }
  constexpr void origin(const double4& origin) noexcept { this->_translation(origin.x, origin.y, 0); }

  /// gets the unit direction in local coordinates.
  constexpr double4 direction() const noexcept { return {1, 0, 0, 0}; }
  /// sets the Z rotation from an XY direction; the local direction remains +X.
  constexpr void direction(const double2& direction) noexcept {
    this->_rotation(double3(0, 0, yw::atan2(direction.y, direction.x)));
  }
  constexpr void direction(const double3& direction) noexcept {
    this->_rotation(double3(0, 0, yw::atan2(direction.y, direction.x)));
  }
  constexpr void direction(const double4& direction) noexcept {
    this->_rotation(double3(0, 0, yw::atan2(direction.y, direction.x)));
  }
  /// classifies a world coordinate against this ray.
  constexpr point_relation contains(const double4& world, double tolerance = 0) const noexcept {
    const auto p = this->local_point(world).xy();
    const auto t = detail::local_tolerance(this->_scale, tolerance);
    return yw::abs(p.y) <= t && p.x >= -t ? point_relation::boundary : point_relation::outside;
  }
  constexpr point_relation contains(const double3& world, double tolerance = 0) const noexcept {
    return contains(double4(world, 1), tolerance);
  }
  constexpr point_relation contains(const double2& world, double tolerance = 0) const noexcept {
    return contains(double4(world.x, world.y, 0, 1), tolerance);
  }

protected:
  friend class geometry_base<ray, Backend>;
  std::expected<void, error> _triangulate() noexcept;
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
  /// configures the unit line transformation from an XY point and direction.
  constexpr line(const double2& point, const double2& direction) noexcept {
    this->_translation(point.x, point.y, 0);
    this->_rotation(double3(0, 0, yw::atan2(direction.y, direction.x)));
  }
  constexpr line(const double3& point, const double3& direction) noexcept : line(point.xy(), direction.xy()) {}
  constexpr line(const double4& point, const double4& direction) noexcept : line(point.xy(), direction.xy()) {}

  /// gets a point on the line in local coordinates.
  constexpr double4 point() const noexcept { return {0, 0, 0, 1}; }
  /// sets the XY translation component; the local point remains zero and input z/w are ignored.
  constexpr void point(const double2& point) noexcept { this->_translation(point.x, point.y, 0); }
  constexpr void point(const double3& point) noexcept { this->_translation(point.x, point.y, 0); }
  constexpr void point(const double4& point) noexcept { this->_translation(point.x, point.y, 0); }

  /// gets the unit direction in local coordinates.
  constexpr double4 direction() const noexcept { return {1, 0, 0, 0}; }
  /// sets the Z rotation from an XY direction; the local direction remains +X.
  constexpr void direction(const double2& direction) noexcept {
    this->_rotation(double3(0, 0, yw::atan2(direction.y, direction.x)));
  }
  constexpr void direction(const double3& direction) noexcept {
    this->_rotation(double3(0, 0, yw::atan2(direction.y, direction.x)));
  }
  constexpr void direction(const double4& direction) noexcept {
    this->_rotation(double3(0, 0, yw::atan2(direction.y, direction.x)));
  }
  /// classifies a world coordinate against this line.
  constexpr point_relation contains(const double4& world, double tolerance = 0) const noexcept {
    const auto p = this->local_point(world).xy();
    const auto t = detail::local_tolerance(this->_scale, tolerance);
    return yw::abs(p.y) <= t ? point_relation::boundary : point_relation::outside;
  }
  constexpr point_relation contains(const double3& world, double tolerance = 0) const noexcept {
    return contains(double4(world, 1), tolerance);
  }
  constexpr point_relation contains(const double2& world, double tolerance = 0) const noexcept {
    return contains(double4(world.x, world.y, 0, 1), tolerance);
  }

protected:
  friend class geometry_base<line, Backend>;
  std::expected<void, error> _triangulate() noexcept;
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

  /// gets a local point on the arc for t in [0, 1].
  constexpr double4 point(double t) const noexcept {
    const auto a = _start_angle + _sweep_angle * t;
    return {yw::cos(a), yw::sin(a), 0, 1};
  }
  /// gets a local unit tangent vector on the arc for t in [0, 1].
  constexpr double4 tangent(double t) const noexcept {
    const auto a = _start_angle + _sweep_angle * t;
    return (double4{-yw::sin(a), yw::cos(a), 0, 0} * _sweep_angle).normalized();
  }
  /// classifies a world coordinate against this arc.
  constexpr point_relation contains(const double4& world, double tolerance = 0) const noexcept {
    const auto p = this->local_point(world).xy();
    const auto t = detail::local_tolerance(this->_scale, tolerance);
    const auto r = p.length();
    const auto a = yw::atan2(p.y, p.x);
    return yw::abs(r - 1.0) <= t && detail::angle_in_sweep(a, _start_angle, _sweep_angle, t)
           ? point_relation::boundary
           : point_relation::outside;
  }
  constexpr point_relation contains(const double3& world, double tolerance = 0) const noexcept {
    return contains(double4(world, 1), tolerance);
  }
  constexpr point_relation contains(const double2& world, double tolerance = 0) const noexcept {
    return contains(double4(world.x, world.y, 0, 1), tolerance);
  }

protected:
  friend class geometry_base<arc, Backend>;
  double _start_angle = 0;
  double _sweep_angle = yw::pi * 0.5;
  std::expected<void, error> _triangulate() noexcept;
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
  /// gets the control point in world coordinates.
  constexpr double4 world_p0() const noexcept { return transform(this->transformation4(), p0()); }
  /// sets the first control point.
  constexpr void p0(const double2& p) noexcept { p0({p.x, p.y, 0, 1}); }
  constexpr void p0(const double3& p) noexcept { p0({p.x, p.y, 0, 1}); }
  constexpr void p0(const double4& p) noexcept {
    _p0 = {p.x, p.y, 0, 1};
    this->_messy = true;
  }

  /// gets the second control point.
  constexpr const double4& p1() const noexcept { return _p1; }
  /// gets the control point in world coordinates.
  constexpr double4 world_p1() const noexcept { return transform(this->transformation4(), p1()); }
  /// sets the second control point.
  constexpr void p1(const double2& p) noexcept { p1({p.x, p.y, 0, 1}); }
  constexpr void p1(const double3& p) noexcept { p1({p.x, p.y, 0, 1}); }
  constexpr void p1(const double4& p) noexcept {
    _p1 = {p.x, p.y, 0, 1};
    this->_messy = true;
  }

  /// gets the third control point.
  constexpr const double4& p2() const noexcept { return _p2; }
  /// gets the control point in world coordinates.
  constexpr double4 world_p2() const noexcept { return transform(this->transformation4(), p2()); }
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
  /// gets a local point on the curve for t in [0, 1].
  constexpr double4 point(double t) const noexcept {
    const auto u = 1.0 - t;
    return _p0 * (u * u) + _p1 * (2.0 * u * t) + _p2 * (t * t);
  }
  /// gets a local unit tangent vector on the curve for t in [0, 1].
  constexpr double4 tangent(double t) const noexcept {
    return ((_p1 - _p0) * (2.0 * (1.0 - t)) + (_p2 - _p1) * (2.0 * t)).normalized();
  }
  /// classifies a world coordinate against this curve.
  constexpr point_relation contains(const double4& world, double tolerance = 0) const noexcept {
    const auto p = this->local_point(world).xy();
    const auto t = detail::local_tolerance(this->_scale, tolerance);
    return detail::point_on_curve(p, *this, t) ? point_relation::boundary : point_relation::outside;
  }
  constexpr point_relation contains(const double3& world, double tolerance = 0) const noexcept {
    return contains(double4(world, 1), tolerance);
  }
  constexpr point_relation contains(const double2& world, double tolerance = 0) const noexcept {
    return contains(double4(world.x, world.y, 0, 1), tolerance);
  }

protected:
  friend class geometry_base<quadratic_bezier, Backend>;
  double4 _p0 = {-1, 0, 0, 1}, _p1 = {0, 1, 0, 1}, _p2 = {1, 0, 0, 1};
  std::expected<void, error> _triangulate() noexcept;
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
  /// gets the control point in world coordinates.
  constexpr double4 world_p0() const noexcept { return transform(this->transformation4(), p0()); }
  /// sets the first control point.
  constexpr void p0(const double2& p) noexcept { p0({p.x, p.y, 0, 1}); }
  constexpr void p0(const double3& p) noexcept { p0({p.x, p.y, 0, 1}); }
  constexpr void p0(const double4& p) noexcept {
    _p0 = {p.x, p.y, 0, 1};
    this->_messy = true;
  }

  /// gets the second control point.
  constexpr const double4& p1() const noexcept { return _p1; }
  /// gets the control point in world coordinates.
  constexpr double4 world_p1() const noexcept { return transform(this->transformation4(), p1()); }
  /// sets the second control point.
  constexpr void p1(const double2& p) noexcept { p1({p.x, p.y, 0, 1}); }
  constexpr void p1(const double3& p) noexcept { p1({p.x, p.y, 0, 1}); }
  constexpr void p1(const double4& p) noexcept {
    _p1 = {p.x, p.y, 0, 1};
    this->_messy = true;
  }

  /// gets the third control point.
  constexpr const double4& p2() const noexcept { return _p2; }
  /// gets the control point in world coordinates.
  constexpr double4 world_p2() const noexcept { return transform(this->transformation4(), p2()); }
  /// sets the third control point.
  constexpr void p2(const double2& p) noexcept { p2({p.x, p.y, 0, 1}); }
  constexpr void p2(const double3& p) noexcept { p2({p.x, p.y, 0, 1}); }
  constexpr void p2(const double4& p) noexcept {
    _p2 = {p.x, p.y, 0, 1};
    this->_messy = true;
  }

  /// gets the fourth control point.
  constexpr const double4& p3() const noexcept { return _p3; }
  /// gets the control point in world coordinates.
  constexpr double4 world_p3() const noexcept { return transform(this->transformation4(), p3()); }
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

  /// gets a local point on the curve for t in [0, 1].
  constexpr double4 point(double t) const noexcept {
    const auto u = 1.0 - t;
    return _p0 * (u * u * u) + _p1 * (3.0 * u * u * t) + _p2 * (3.0 * u * t * t) + _p3 * (t * t * t);
  }
  /// gets a local unit tangent vector on the curve for t in [0, 1].
  constexpr double4 tangent(double t) const noexcept {
    const auto u = 1.0 - t;
    return ((_p1 - _p0) * (3.0 * u * u) + (_p2 - _p1) * (6.0 * u * t) + (_p3 - _p2) * (3.0 * t * t))
      .normalized();
  }
  /// classifies a world coordinate against this curve.
  constexpr point_relation contains(const double4& world, double tolerance = 0) const noexcept {
    const auto p = this->local_point(world).xy();
    const auto t = detail::local_tolerance(this->_scale, tolerance);
    return detail::point_on_curve(p, *this, t) ? point_relation::boundary : point_relation::outside;
  }
  constexpr point_relation contains(const double3& world, double tolerance = 0) const noexcept {
    return contains(double4(world, 1), tolerance);
  }
  constexpr point_relation contains(const double2& world, double tolerance = 0) const noexcept {
    return contains(double4(world.x, world.y, 0, 1), tolerance);
  }

protected:
  friend class geometry_base<cubic_bezier, Backend>;
  double4 _p0 = {-1, 0, 0, 1}, _p1 = {-0.5, 1, 0, 1}, _p2 = {0.5, -1, 0, 1}, _p3 = {1, 0, 0, 1};
  std::expected<void, error> _triangulate() noexcept;
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
  /// gets the center point in local coordinates.
  constexpr double4 center() const noexcept { return {0, 0, 0, 1}; }
  /// gets the local unit normal of the XY plane.
  constexpr double4 normal() const noexcept { return {0, 0, 1, 0}; }
  /// gets the unit plane normal after rotation.
  constexpr double4 world_normal() const noexcept { return {this->_rigid[0][2], this->_rigid[1][2], this->_rigid[2][2], 0}; }
  /// gets the finite local-coordinate bounding box used for remeshing.
  constexpr geom::bbox<cpu> bbox() const noexcept {
    const auto h = double(this->_remeshing_option.half_extent.x);
    return {{-h, -h, 0, 1}, {h, h, 0, 1}};
  }
  /// classifies a world coordinate against this plane.
  constexpr point_relation contains(const double4& world, double tolerance = 0) const noexcept {
    const auto p = this->local_point(world);
    const auto t = detail::local_tolerance(this->_scale, tolerance);
    return yw::abs(p.z) <= t ? point_relation::inside : point_relation::outside;
  }
  constexpr point_relation contains(const double3& world, double tolerance = 0) const noexcept {
    return contains(double4(world, 1), tolerance);
  }
  constexpr point_relation contains(const double2& world, double tolerance = 0) const noexcept {
    return contains(double4(world.x, world.y, 0, 1), tolerance);
  }

protected:
  friend class geometry_base<plane, Backend>;
  std::expected<void, error> _triangulate() noexcept;
};

/// MARK: geom::square

/// represents a 2D square centered at the origin with corners at (+/-1, +/-1).
template<backend Backend> class square : public geometry_base<square, Backend> {
public:
  static constexpr bool has_bounded_surface = true;
  /// constructs a square centered at the origin with corners at (+/-1, +/-1).
  constexpr square() noexcept = default;
  /// gets the center point in local coordinates.
  constexpr double4 center() const noexcept { return {0, 0, 0, 1}; }
  /// gets the local unit normal of the XY plane.
  constexpr double4 normal() const noexcept { return {0, 0, 1, 0}; }
  /// gets the unit plane normal after rotation.
  constexpr double4 world_normal() const noexcept { return {this->_rigid[0][2], this->_rigid[1][2], this->_rigid[2][2], 0}; }
  /// gets the local area of the unit square.
  constexpr double area() const noexcept { return 4.0; }
  /// gets the signed area after scaling.
  constexpr double world_area() const noexcept { return area() * this->_scale.x * this->_scale.y; }
  /// gets the local-coordinate bounding box.
  constexpr geom::bbox<cpu> bbox() const noexcept { return {{-1, -1, 0, 1}, {1, 1, 0, 1}}; }
  /// classifies a world coordinate against this square.
  constexpr point_relation contains(const double4& world, double tolerance = 0) const noexcept {
    const auto t = detail::local_tolerance(this->_scale, tolerance);
    if (this->_scale.x == 0 || this->_scale.y == 0) {
      const auto sp = this->local_scaled_point(world).xy();
      const auto x = this->_scale.x == 0 ? 0.0 : sp.x / this->_scale.x;
      const auto y = this->_scale.y == 0 ? 0.0 : sp.y / this->_scale.y;
      if (this->_scale.x == 0 && this->_scale.y == 0)
        return sp.length() <= t ? point_relation::boundary : point_relation::outside;
      if (this->_scale.x == 0)
        return yw::abs(sp.x) <= t && y >= -1.0 - t && y <= 1.0 + t ? point_relation::boundary : point_relation::outside;
      return yw::abs(sp.y) <= t && x >= -1.0 - t && x <= 1.0 + t ? point_relation::boundary : point_relation::outside;
    }
    const auto p = this->local_point(world).xy();
    if (p.x < -1.0 - t || p.x > 1.0 + t || p.y < -1.0 - t || p.y > 1.0 + t) return point_relation::outside;
    if (yw::abs(yw::abs(p.x) - 1.0) <= t || yw::abs(yw::abs(p.y) - 1.0) <= t) return point_relation::boundary;
    return point_relation::inside;
  }
  constexpr point_relation contains(const double3& world, double tolerance = 0) const noexcept {
    return contains(double4(world, 1), tolerance);
  }
  constexpr point_relation contains(const double2& world, double tolerance = 0) const noexcept {
    return contains(double4(world.x, world.y, 0, 1), tolerance);
  }

protected:
  friend class geometry_base<square, Backend>;
  std::expected<void, error> _triangulate() noexcept;
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
  /// gets the center point in local coordinates.
  constexpr double4 center() const noexcept { return {0, 0, 0, 1}; }
  /// gets the local unit normal of the XY plane.
  constexpr double4 normal() const noexcept { return {0, 0, 1, 0}; }
  /// gets the unit plane normal after rotation.
  constexpr double4 world_normal() const noexcept { return {this->_rigid[0][2], this->_rigid[1][2], this->_rigid[2][2], 0}; }
  /// gets the local area of the unit disk.
  constexpr double area() const noexcept { return yw::pi; }
  /// gets the signed area after scaling.
  constexpr double world_area() const noexcept { return area() * this->_scale.x * this->_scale.y; }
  /// gets the local-coordinate bounding box.
  constexpr geom::bbox<cpu> bbox() const noexcept { return {{-1, -1, 0, 1}, {1, 1, 0, 1}}; }
  /// classifies a world coordinate against this circle.
  constexpr point_relation contains(const double4& world, double tolerance = 0) const noexcept {
    const auto t = detail::local_tolerance(this->_scale, tolerance);
    if (this->_scale.x == 0 || this->_scale.y == 0) {
      const auto sp = this->local_scaled_point(world).xy();
      const auto x = this->_scale.x == 0 ? 0.0 : sp.x / this->_scale.x;
      const auto y = this->_scale.y == 0 ? 0.0 : sp.y / this->_scale.y;
      if (this->_scale.x == 0 && this->_scale.y == 0)
        return sp.length() <= t ? point_relation::boundary : point_relation::outside;
      if (this->_scale.x == 0)
        return yw::abs(sp.x) <= t && yw::abs(y) <= 1.0 + t ? point_relation::boundary : point_relation::outside;
      return yw::abs(sp.y) <= t && yw::abs(x) <= 1.0 + t ? point_relation::boundary : point_relation::outside;
    }
    const auto p = this->local_point(world).xy();
    const auto r = p.length();
    if (r > 1.0 + t) return point_relation::outside;
    if (yw::abs(r - 1.0) <= t) return point_relation::boundary;
    return point_relation::inside;
  }
  constexpr point_relation contains(const double3& world, double tolerance = 0) const noexcept {
    return contains(double4(world, 1), tolerance);
  }
  constexpr point_relation contains(const double2& world, double tolerance = 0) const noexcept {
    return contains(double4(world.x, world.y, 0, 1), tolerance);
  }

protected:
  friend class geometry_base<circle, Backend>;
  std::expected<void, error> _triangulate() noexcept;
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
  /// gets the local-coordinate bounding box.
  constexpr std::expected<geom::bbox<cpu>, error> bbox() const noexcept {
    if (_points.empty()) return std::unexpected(error(errors::invalid_operation, "polygon has no points"));
    geom::bbox<cpu> result{{_points[0].x, _points[0].y, 0, 1}, {_points[0].x, _points[0].y, 0, 1}};
    for (size_t i = 1; i < _points.size(); ++i) {
      const auto& p = _points[i];
      result.min.x = yw::min(result.min.x, p.x);
      result.min.y = yw::min(result.min.y, p.y);
      result.max.x = yw::max(result.max.x, p.x);
      result.max.y = yw::max(result.max.y, p.y);
    }
    return result;
  }

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
  /// classifies a world coordinate against this polygon.
  constexpr point_relation contains(const double4& world, double tolerance = 0) const noexcept {
    const auto p = this->local_point(world).xy();
    const auto t = detail::local_tolerance(this->_scale, tolerance);
    if (_points.empty()) return point_relation::outside;
    if (_points.size() == 1) return (p - _points[0].xy()).length() <= t ? point_relation::boundary : point_relation::outside;
    bool inside = false;
    for (size_t i = 0, j = _points.size() - 1; i < _points.size(); j = i++) {
      const auto a = _points[j].xy();
      const auto b = _points[i].xy();
      if (detail::point_on_segment(p, a, b, t)) return point_relation::boundary;
      if ((a.y > p.y) != (b.y > p.y)) {
        const auto x = (b.x - a.x) * (p.y - a.y) / (b.y - a.y) + a.x;
        if (p.x < x) inside = !inside;
      }
    }
    return inside ? point_relation::inside : point_relation::outside;
  }
  constexpr point_relation contains(const double3& world, double tolerance = 0) const noexcept {
    return contains(double4(world, 1), tolerance);
  }
  constexpr point_relation contains(const double2& world, double tolerance = 0) const noexcept {
    return contains(double4(world.x, world.y, 0, 1), tolerance);
  }

protected:
  friend class geometry_base<polygon, Backend>;
  array1<double4, cpu> _points;
  std::expected<void, error> _triangulate() noexcept;
};
} // namespace yw::geom
