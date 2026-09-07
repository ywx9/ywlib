#pragma once
#include <yw/geometry_2d.h>

namespace yw::geom {

/// MARK: geom::prism

template<template<backend> typename Face, backend Backend> requires Face<Backend>::has_bounded_surface class prism;
template<template<backend> typename Face, backend Backend> requires Face<Backend>::has_bounded_surface class pyramid;

template<template<template<backend> typename, backend> typename Geom, template<backend> typename Face>
struct remove_backend {
  template<backend Backend> using type = Geom<Face, Backend>;
};

template<> struct remeshing_option<remove_backend<prism, circle>::template type> : remeshing_option<circle> {};
template<> struct remeshing_option<remove_backend<prism, square>::template type> : remeshing_option<square> {};
template<> struct remeshing_option<remove_backend<prism, polygon>::template type> : remeshing_option<polygon> {};
template<> struct remeshing_option<remove_backend<pyramid, circle>::template type> : remeshing_option<circle> {};
template<> struct remeshing_option<remove_backend<pyramid, square>::template type> : remeshing_option<square> {};
template<> struct remeshing_option<remove_backend<pyramid, polygon>::template type> : remeshing_option<polygon> {};

template<template<backend> typename Face, backend Backend> requires Face<Backend>::has_bounded_surface
class prism : public geometry_base<remove_backend<prism, Face>::template type, Backend> {
public:
  constexpr prism() noexcept = default;
  constexpr prism(const Face<Backend>& face, double1 height = 1.0) noexcept : _height(height.x) {
    this->copy_transformation_from(face);
    this->remeshing_option(face.remeshing_option());
    if constexpr (same_as<Face<cpu>, polygon<cpu>>) _points = face.points();
  }
  /// gets the local-coordinate bounding box.
  constexpr std::expected<bbox<cpu>, error> local_bbox() const noexcept {
    bbox<cpu> face_bbox;
    if constexpr (same_as<Face<cpu>, circle<cpu>> || same_as<Face<cpu>, square<cpu>>) {
      face_bbox = {{-1, -1, 0, 1}, {1, 1, 0, 1}};
    } else if constexpr (same_as<Face<cpu>, polygon<cpu>>) {
      if (_points.empty()) return std::unexpected(error(errors::invalid_operation, "prism face polygon has no points"));
      face_bbox = {{_points[0].x, _points[0].y, 0, 1}, {_points[0].x, _points[0].y, 0, 1}};
      for (size_t i = 1; i < _points.size(); ++i) {
        const auto& p = _points[i];
        face_bbox.min.x = yw::min(face_bbox.min.x, p.x);
        face_bbox.min.y = yw::min(face_bbox.min.y, p.y);
        face_bbox.max.x = yw::max(face_bbox.max.x, p.x);
        face_bbox.max.y = yw::max(face_bbox.max.y, p.y);
      }
    } else return std::unexpected(error(errors::invalid_argument, "unsupported prism face"));
    const auto z0 = yw::min(0.0, _height);
    const auto z1 = yw::max(0.0, _height);
    face_bbox.min.z = z0;
    face_bbox.max.z = z1;
    return face_bbox;
  }

protected:
  friend class geometry_base<remove_backend<prism, Face>::template type, Backend>;
  select_type<same_as<Face<cpu>, polygon<cpu>>, array1<double4, cpu>, none> _points;
  double _height = 2.0;
  std::expected<void, error> _triangulate() noexcept;
};

/// MARK: geom::pyramid

template<template<backend> typename Face, backend Backend> requires Face<Backend>::has_bounded_surface
class pyramid : public geometry_base<remove_backend<pyramid, Face>::template type, Backend> {
public:
  constexpr pyramid() noexcept = default;
  constexpr pyramid(const Face<Backend>& face, double1 height = 1.0) noexcept : _height(height.x) {
    this->copy_transformation_from(face);
    this->remeshing_option(face.remeshing_option());
    if constexpr (same_as<Face<cpu>, polygon<cpu>>) _points = face.points();
  }
  /// gets the local-coordinate bounding box.
  constexpr std::expected<bbox<cpu>, error> local_bbox() const noexcept {
    bbox<cpu> face_bbox;
    if constexpr (same_as<Face<cpu>, circle<cpu>> || same_as<Face<cpu>, square<cpu>>) {
      face_bbox = {{-1, -1, 0, 1}, {1, 1, 0, 1}};
    } else if constexpr (same_as<Face<cpu>, polygon<cpu>>) {
      if (_points.empty()) return std::unexpected(error(errors::invalid_operation, "pyramid face polygon has no points"));
      face_bbox = {{_points[0].x, _points[0].y, 0, 1}, {_points[0].x, _points[0].y, 0, 1}};
      for (size_t i = 1; i < _points.size(); ++i) {
        const auto& p = _points[i];
        face_bbox.min.x = yw::min(face_bbox.min.x, p.x);
        face_bbox.min.y = yw::min(face_bbox.min.y, p.y);
        face_bbox.max.x = yw::max(face_bbox.max.x, p.x);
        face_bbox.max.y = yw::max(face_bbox.max.y, p.y);
      }
    } else return std::unexpected(error(errors::invalid_argument, "unsupported pyramid face"));
    face_bbox.min.x = yw::min(face_bbox.min.x, 0.0);
    face_bbox.min.y = yw::min(face_bbox.min.y, 0.0);
    face_bbox.max.x = yw::max(face_bbox.max.x, 0.0);
    face_bbox.max.y = yw::max(face_bbox.max.y, 0.0);
    const auto z0 = yw::min(0.0, _height);
    const auto z1 = yw::max(0.0, _height);
    face_bbox.min.z = z0;
    face_bbox.max.z = z1;
    return face_bbox;
  }

protected:
  friend class geometry_base<remove_backend<pyramid, Face>::template type, Backend>;
  select_type<same_as<Face<cpu>, polygon<cpu>>, array1<double4, cpu>, none> _points;
  double _height = 2.0;
  std::expected<void, error> _triangulate() noexcept;
};

/// MARK: geom::sphere

template<backend Backend> class sphere;
template<> struct remeshing_option<sphere> {
  uint1 level = 2; // recursive subdivision level; each edge has 2^level segments
};

/// represents a unit sphere centered at the origin; use scale() to change its radii.
template<backend Backend> class sphere : public geometry_base<sphere, Backend> {
public:
  constexpr sphere() noexcept = default;
  constexpr double4 center() const noexcept {
    return {this->_rigid[0][3], this->_rigid[1][3], this->_rigid[2][3], 1};
  }
  constexpr std::expected<bbox<cpu>, error> local_bbox() const noexcept {
    return bbox<cpu>{{-1, -1, -1, 1}, {1, 1, 1, 1}};
  }
  const geom::remeshing_option<sphere>& remeshing_option() const noexcept { return this->_remeshing_option; }
  geom::remeshing_option<sphere>& remeshing_option() noexcept {
    this->_messy = true;
    return this->_remeshing_option;
  }
  void remeshing_option(const geom::remeshing_option<sphere>& option) noexcept {
    this->_remeshing_option = option;
    this->_messy = true;
  }

protected:
  friend class geometry_base<sphere, Backend>;
  std::expected<void, error> _triangulate() noexcept;
};

/// MARK: geom::polyline

template<backend Backend> class polyline;
template<> struct remeshing_option<polyline> {};

/// A 3D polyline built by sampling finite 2D curve elements.
///
/// An element's transformation is baked into the appended local points. The
/// polyline then has its own, independent transformation for drawing.
template<backend Backend> class polyline : public geometry_base<polyline, Backend> {
  array1<double4, cpu> _points;

  template<typename Sample>
  std::expected<void, error> _append_samples(size_t count, Sample&& sample, double merge_threshold) noexcept {
    if (merge_threshold < 0)
      return std::unexpected(error(errors::invalid_argument, "polyline merge threshold must be non-negative"));
    if (count == 0) return {};

    const auto first_point = sample(0);
    size_t first = 0;
    if (!_points.empty()) {
      const auto delta = first_point - _points.back();
      if (delta.squared_length() <= merge_threshold * merge_threshold) first = 1;
    }
    if (first == count) return {};
    if (count - first > std::numeric_limits<size_t>::max() - _points.size())
      return std::unexpected(error(errors::invalid_argument, "polyline has too many points"));
    if (auto res = _points.reserve(_points.size() + count - first); !res) return res.error().relay();
    for (size_t i = first; i < count; ++i)
      if (auto res = _points.push_back(sample(i)); !res) return res.error().relay();
    this->_messy = true;
    return {};
  }

  template<backend ElementBackend, typename Curve>
  std::expected<void, error> _append_curve(
    const Curve& curve, uint32_t subdivisions, double merge_threshold, string_view<char> name) noexcept {
    if (subdivisions < 1)
      return std::unexpected(error(errors::invalid_argument, format(name, " subdivisions must be at least 1")));
    const auto transformation = curve.transformation4();
    return _append_samples(size_t(subdivisions) + 1,
      [&](size_t i) noexcept { return transform(transformation, curve.point(double(i) / double(subdivisions))); },
      merge_threshold);
  }

protected:
  friend class geometry_base<polyline, Backend>;
  std::expected<void, error> _triangulate() noexcept;

public:
  constexpr polyline() noexcept = default;

  /// gets the local points with each appended element's transformation baked in.
  constexpr const auto& points() const noexcept { return _points; }
  constexpr size_t size() const noexcept { return _points.size(); }
  constexpr bool empty() const noexcept { return _points.empty(); }
  constexpr const double4& operator[](size_t index) const noexcept { return _points[index]; }

  /// gets the local-coordinate bounding box.
  constexpr std::expected<bbox<cpu>, error> local_bbox() const noexcept {
    if (_points.empty()) return std::unexpected(error(errors::invalid_operation, "polyline has no points"));
    bbox<cpu> result{_points[0], _points[0]};
    for (size_t i = 1; i < _points.size(); ++i) {
      const auto& p = _points[i];
      result.min.x = yw::min(result.min.x, p.x), result.min.y = yw::min(result.min.y, p.y),
      result.min.z = yw::min(result.min.z, p.z);
      result.max.x = yw::max(result.max.x, p.x), result.max.y = yw::max(result.max.y, p.y),
      result.max.z = yw::max(result.max.z, p.z);
    }
    return result;
  }

  /// appends a transformed finite segment.
  template<backend ElementBackend>
  std::expected<void, error> push_back(const segment<ElementBackend>& element, double merge_threshold = 1e-10) noexcept {
    const auto transformation = element.transformation4();
    return _append_samples(2,
      [&](size_t i) noexcept { return transform(transformation, double4{double(i), 0, 0, 1}); }, merge_threshold);
  }

  /// samples and appends a transformed circular arc.
  template<backend ElementBackend>
  std::expected<void, error> push_back(const arc<ElementBackend>& element, double merge_threshold = 1e-10) noexcept {
    return _append_curve<ElementBackend>(
      element, element.remeshing_option().subdivisions.x, merge_threshold, "arc");
  }

  /// samples and appends a transformed quadratic Bezier curve.
  template<backend ElementBackend>
  std::expected<void, error> push_back(
    const quadratic_bezier<ElementBackend>& element, double merge_threshold = 1e-10) noexcept {
    return _append_curve<ElementBackend>(
      element, element.remeshing_option().subdivisions.x, merge_threshold, "quadratic Bezier");
  }

  /// samples and appends a transformed cubic Bezier curve.
  template<backend ElementBackend>
  std::expected<void, error> push_back(
    const cubic_bezier<ElementBackend>& element, double merge_threshold = 1e-10) noexcept {
    return _append_curve<ElementBackend>(
      element, element.remeshing_option().subdivisions.x, merge_threshold, "cubic Bezier");
  }

  /// removes all sampled points.
  void clear() noexcept {
    _points.clear();
    this->_messy = true;
  }
};

/// MARK: geom::mesh

template<backend Backend> class mesh;
template<> struct remeshing_option<mesh> {};

template<backend Backend> class mesh : public geometry_base<mesh, Backend> {
  array1<vertex<Backend>, cpu> _vertices;
  array1<uint3, cpu> _triangles;

protected:
  friend class geometry_base<mesh, Backend>;
  std::expected<void, error> _triangulate() noexcept;

public:
  constexpr const auto& vertices() const noexcept { return _vertices; }
  constexpr const auto& triangles() const noexcept { return _triangles; }
  constexpr size_t vertex_count() const noexcept { return _vertices.size(); }
  constexpr size_t triangle_count() const noexcept { return _triangles.size(); }
  std::expected<void, error> push_back(const vector3<vertex<Backend>>& triangle) noexcept {
    if (_vertices.size() > std::numeric_limits<uint32_t>::max() - 3)
      return std::unexpected(error(errors::invalid_argument, "mesh vertex count is too large"));
    const auto base = uint32_t(_vertices.size());
    if (auto r = _vertices.reserve(_vertices.size() + 3); !r) return r.error().relay();
    if (auto r = _triangles.reserve(_triangles.size() + 1); !r) return r.error().relay();
    for (const auto& v : triangle)
      if (auto r = _vertices.push_back(v); !r) return r.error().relay();
    if (auto r = _triangles.push_back({base, base + 1, base + 2}); !r) return r.error().relay();
    this->_messy = true;
    return {};
  }
  void clear() noexcept { _vertices.clear(); _triangles.clear(); this->_messy = true; }
};
} // namespace yw::geom
