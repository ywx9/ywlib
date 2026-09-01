#pragma once
#include <unordered_map>
#include <vector>
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

protected:
  friend class geometry_base<remove_backend<prism, Face>::template type, Backend>;
  select_type<same_as<Face<cpu>, polygon<cpu>>, array1<double4, cpu>, none> _points;
  double _height = 1.0;
  std::expected<void, error> _triangulate(
    const geom::remeshing_option<remove_backend<prism, Face>::template type>& ro) noexcept;
  template<floating T> constexpr std::expected<array1<vector4<T>, cpu>, error> _make_vertices() noexcept;
  constexpr std::expected<array1<uint3>, error> _make_triangles() noexcept;
  constexpr std::expected<array1<uint2>, error> _make_edges() noexcept { return {}; }
};

template<template<backend> typename Face, backend Backend> requires Face<Backend>::has_bounded_surface
template<floating T>
constexpr std::expected<array1<vector4<T>, cpu>, error> prism<Face, Backend>::_make_vertices() noexcept {
  size_t n = 0;
  if constexpr (same_as<Face<cpu>, circle<cpu>>) n = this->_remeshing_option.subdivisions.x;
  else if constexpr (same_as<Face<cpu>, square<cpu>>) n = 4;
  else if constexpr (same_as<Face<cpu>, polygon<cpu>>) n = _points.size();
  else return std::unexpected(error(errors::invalid_argument, "unsupported prism face"));

  if (n < 3) return std::unexpected(error(errors::invalid_argument, "prism vertex count must be at least 3"));
  if (n > (std::numeric_limits<uint32_t>::max() - 2) / 2)
    return std::unexpected(error(errors::invalid_argument, "prism vertex count is too large"));

  array1<vector4<T>, cpu> vertices;
  if (auto res = vertices.resize(n * 2 + 2); !res) return res.error().relay();

  const auto h = T(_height);
  vertices[0] = {0, 0, 0, 1};
  vertices[n + 1] = {0, 0, h, 1};
  if constexpr (same_as<Face<cpu>, circle<cpu>>) {
    for (size_t i = 0; i < n; ++i) {
      const auto angle = yw::pi2 * double(i) / double(n);
      const auto x = T(yw::cos(angle));
      const auto y = T(yw::sin(angle));
      vertices[i + 1] = {x, y, 0, 1};
      vertices[n + i + 2] = {x, y, h, 1};
    }
  } else if constexpr (same_as<Face<cpu>, square<cpu>>) {
    constexpr vector4<T> points[] = {{-1, -1, 0, 1}, {1, -1, 0, 1}, {1, 1, 0, 1}, {-1, 1, 0, 1}};
    for (size_t i = 0; i < n; ++i) {
      vertices[i + 1] = points[i];
      vertices[n + i + 2] = {points[i].x, points[i].y, h, 1};
    }
  } else if constexpr (same_as<Face<cpu>, polygon<cpu>>) {
    for (size_t i = 0; i < n; ++i) {
      vertices[i + 1] = vector4<T>(_points[i]);
      vertices[n + i + 2] = {T(_points[i].x), T(_points[i].y), h, 1};
    }
  }
  return vertices;
}

template<template<backend> typename Face, backend Backend> requires Face<Backend>::has_bounded_surface
constexpr std::expected<array1<uint3>, error> prism<Face, Backend>::_make_triangles() noexcept {
  size_t n = 0;
  if constexpr (same_as<Face<cpu>, circle<cpu>>) n = this->_remeshing_option.subdivisions.x;
  else if constexpr (same_as<Face<cpu>, square<cpu>>) n = 4;
  else if constexpr (same_as<Face<cpu>, polygon<cpu>>) n = _points.size();
  else return std::unexpected(error(errors::invalid_argument, "unsupported prism face"));

  if (n < 3) return std::unexpected(error(errors::invalid_argument, "prism vertex count must be at least 3"));
  if (n > (std::numeric_limits<uint32_t>::max() - 2) / 2)
    return std::unexpected(error(errors::invalid_argument, "prism vertex count is too large"));

  array1<uint3> triangles;
  if (auto res = triangles.resize(n * 4); !res) return res.error().relay();

  const auto top_center = uint32_t(n + 1);
  for (size_t i = 0; i < n; ++i) {
    const auto current = uint32_t(i + 1);
    const auto next = uint32_t((i + 1) % n + 1);
    const auto top_current = uint32_t(n + i + 2);
    const auto top_next = uint32_t(n + (i + 1) % n + 2);
    triangles[i] = {0, next, current};
    triangles[n + i] = {top_center, top_current, top_next};
    triangles[n * 2 + i] = {current, next, top_current};
    triangles[n * 3 + i] = {next, top_next, top_current};
  }
  return triangles;
}

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

protected:
  friend class geometry_base<remove_backend<pyramid, Face>::template type, Backend>;
  select_type<same_as<Face<cpu>, polygon<cpu>>, array1<double4, cpu>, none> _points;
  double _height = 1.0;
  std::expected<void, error> _triangulate(
    const geom::remeshing_option<remove_backend<pyramid, Face>::template type>& ro) noexcept;
  template<floating T> constexpr std::expected<array1<vector4<T>, cpu>, error> _make_vertices() noexcept;
  constexpr std::expected<array1<uint3>, error> _make_triangles() noexcept;
  constexpr std::expected<array1<uint2>, error> _make_edges() noexcept { return {}; }
};

template<template<backend> typename Face, backend Backend> requires Face<Backend>::has_bounded_surface
template<floating T>
constexpr std::expected<array1<vector4<T>, cpu>, error> pyramid<Face, Backend>::_make_vertices() noexcept {
  size_t n = 0;
  if constexpr (same_as<Face<cpu>, circle<cpu>>) n = this->_remeshing_option.subdivisions.x;
  else if constexpr (same_as<Face<cpu>, square<cpu>>) n = 4;
  else if constexpr (same_as<Face<cpu>, polygon<cpu>>) n = _points.size();
  else return std::unexpected(error(errors::invalid_argument, "unsupported pyramid face"));

  if (n < 3) return std::unexpected(error(errors::invalid_argument, "pyramid vertex count must be at least 3"));
  if (n > std::numeric_limits<uint32_t>::max() - 2)
    return std::unexpected(error(errors::invalid_argument, "pyramid vertex count is too large"));

  array1<vector4<T>, cpu> vertices;
  if (auto res = vertices.resize(n + 2); !res) return res.error().relay();

  const auto h = T(_height);
  vertices[0] = {0, 0, 0, 1};
  vertices[n + 1] = {0, 0, h, 1};
  if constexpr (same_as<Face<cpu>, circle<cpu>>) {
    for (size_t i = 0; i < n; ++i) {
      const auto angle = yw::pi2 * double(i) / double(n);
      vertices[i + 1] = {T(yw::cos(angle)), T(yw::sin(angle)), 0, 1};
    }
  } else if constexpr (same_as<Face<cpu>, square<cpu>>) {
    constexpr vector4<T> points[] = {{-1, -1, 0, 1}, {1, -1, 0, 1}, {1, 1, 0, 1}, {-1, 1, 0, 1}};
    for (size_t i = 0; i < n; ++i) vertices[i + 1] = points[i];
  } else if constexpr (same_as<Face<cpu>, polygon<cpu>>) {
    for (size_t i = 0; i < n; ++i) vertices[i + 1] = vector4<T>(_points[i]);
  }
  return vertices;
}

template<template<backend> typename Face, backend Backend> requires Face<Backend>::has_bounded_surface
constexpr std::expected<array1<uint3>, error> pyramid<Face, Backend>::_make_triangles() noexcept {
  size_t n = 0;
  if constexpr (same_as<Face<cpu>, circle<cpu>>) n = this->_remeshing_option.subdivisions.x;
  else if constexpr (same_as<Face<cpu>, square<cpu>>) n = 4;
  else if constexpr (same_as<Face<cpu>, polygon<cpu>>) n = _points.size();
  else return std::unexpected(error(errors::invalid_argument, "unsupported pyramid face"));

  if (n < 3) return std::unexpected(error(errors::invalid_argument, "pyramid vertex count must be at least 3"));
  if (n > std::numeric_limits<uint32_t>::max() - 2)
    return std::unexpected(error(errors::invalid_argument, "pyramid vertex count is too large"));

  array1<uint3> triangles;
  if (auto res = triangles.resize(n * 2); !res) return res.error().relay();

  const auto apex = uint32_t(n + 1);
  for (size_t i = 0; i < n; ++i) {
    const auto current = uint32_t(i + 1);
    const auto next = uint32_t((i + 1) % n + 1);
    triangles[i] = {0, next, current};
    triangles[n + i] = {current, next, apex};
  }
  return triangles;
}

// /// MARK: geom::sphere

// template<backend> class sphere : public geometry3_base<sphere<Backend>, Backend> {
// public:
//   struct remeshing_option_t {
//     uint1 subdivisions = 2;
//   };

// protected:
//   friend class geometry_base<sphere<Backend>, Backend>;
//   double _radius = 1.0;
//   remeshing_option_t _remeshing_option{};
//   std::expected<void, error> _triangulate(const remeshing_option_t& ro) noexcept;
//   std::expected<void, error> _triangulate_default() noexcept { return _triangulate(_remeshing_option); }

// public:
//   constexpr sphere() noexcept = default;
//   constexpr explicit sphere(double radius) noexcept : _radius(radius) {}
//   constexpr double radius() const noexcept { return _radius; }
//   constexpr void radius(double r) noexcept {
//     _radius = r;
//     this->_messy = true;
//   }
//   constexpr double4 center() const noexcept { return {this->_rigid[0][3], this->_rigid[1][3], this->_rigid[2][3], 1};
//   }

//   std::expected<mesh<cpu>, error> to_mesh(const remeshing_option_t& ro = {}) const noexcept {
//     if (_radius <= 0) return std::unexpected(error(errors::invalid_argument, "sphere radius must be positive"));
//     constexpr double x = 0.525731112119133606;
//     constexpr double z = 0.850650808352039932;
//     std::vector<double3> vertices{
//       {-x, 0, z}, {x, 0, z}, {-x, 0, -z}, {x, 0, -z}, {0, z, x}, {0, z, -x}, {0, -z, x}, {0, -z, -x}, {z, x, 0},
//       {-z, x, 0}, {z, -x, 0}, {-z, -x, 0}};
//     std::vector<uint3> triangles{
//       {0, 4, 1}, {0, 9, 4}, {9, 5, 4}, {4, 5, 8}, {4, 8, 1}, {8, 10, 1}, {8, 3, 10}, {5, 3, 8}, {5, 2, 3}, {2, 7, 3},
//       {7, 10, 3}, {7, 6, 10}, {7, 11, 6}, {11, 0, 6}, {0, 1, 6}, {6, 1, 10}, {9, 0, 11}, {9, 11, 2}, {9, 2, 5},
//       {7, 2, 11}};
//     for (uint32_t s = 0; s < ro.subdivisions.x; ++s) {
//       std::unordered_map<uint64_t, uint32_t> midpoints;
//       auto midpoint = [&](uint32_t a, uint32_t b) -> std::expected<uint32_t, error> {
//         if (b < a) std::swap(a, b);
//         const auto key = (uint64_t(a) << 32) | b;
//         if (const auto it = midpoints.find(key); it != midpoints.end()) return it->second;
//         if (vertices.size() >= std::numeric_limits<uint32_t>::max())
//           return std::unexpected(error(errors::invalid_argument, "sphere mesh vertex count is too large"));
//         const auto m = ((vertices[a] + vertices[b]) * 0.5).normalized();
//         const auto index = uint32_t(vertices.size());
//         vertices.push_back(m);
//         midpoints.emplace(key, index);
//         return index;
//       };
//       std::vector<uint3> next;
//       next.reserve(triangles.size() * 4);
//       for (const auto& t : triangles) {
//         auto ab = midpoint(t.x, t.y);
//         if (!ab) return ab.error().relay();
//         auto bc = midpoint(t.y, t.z);
//         if (!bc) return bc.error().relay();
//         auto ca = midpoint(t.z, t.x);
//         if (!ca) return ca.error().relay();
//         next.push_back({t.x, *ab, *ca});
//         next.push_back({t.y, *bc, *ab});
//         next.push_back({t.z, *ca, *bc});
//         next.push_back({*ab, *bc, *ca});
//       }
//       triangles = std::move(next);
//     }
//     mesh<cpu> out;
//     for (const auto& v : vertices)
//       if (auto res = out.add_vertex(v * _radius); !res) return res.error().relay();
//     if (auto res = out.assign_triangles(triangles); !res) return res.error().relay();
//     if (auto res = out.rebuild_edges(); !res) return res.error().relay();
//     return out;
//   }
// };

// /// MARK: geom::polyline

// template<backend> class polyline : public geometry3_base<polyline<Backend>, Backend> {
// public:
//   struct remeshing_option_t {}; // no options for remeshing

// protected:
//   friend class geometry_base<polyline<Backend>, Backend>;
//   array1<double4, cpu> _points;
//   remeshing_option_t _remeshing_option{};
//   std::expected<void, error> _triangulate(const remeshing_option_t& ro) noexcept;
//   std::expected<void, error> _triangulate_default() noexcept { return _triangulate(_remeshing_option); }

// public:
//   constexpr polyline() noexcept = default;
//   constexpr const auto& points() const noexcept { return _points; }
//   constexpr size_t size() const noexcept { return _points.size(); }
//   constexpr bool empty() const noexcept { return _points.empty(); }
//   constexpr const double4& operator[](size_t index) const noexcept { return _points[index]; }

//   std::expected<void, error> resize(size_t size) noexcept {
//     if (const auto old_size = _points.size(); size == old_size) return {};
//     else if (auto res = _points.resize(size); !res) return res.error().relay();
//     else if (size > old_size)
//       for (size_t i = old_size; i < size; ++i) _points[i] = {0, 0, 0, 1};
//     this->_messy = true;
//     return {};
//   }

//   std::expected<void, error> assign(const double4* points, size_t count) noexcept {
//     if (auto res = _points.assign(points, count); !res) return res.error().relay();
//     for (auto& p : _points) p = {p.x, p.y, p.z, 1};
//     this->_messy = true;
//     return {};
//   }
//   template<contiguous_range<double4> Rg> std::expected<void, error> assign(Rg&& points) noexcept {
//     return assign(yw::data(points), yw::size(points));
//   }

//   std::expected<void, error> push_back(const double3& point) noexcept {
//     if (auto res = _points.push_back({point.x, point.y, point.z, 1}); !res) return res.error().relay();
//     this->_messy = true;
//     return {};
//   }
//   std::expected<void, error> push_back(const double4& point) noexcept {
//     if (auto res = _points.push_back({point.x, point.y, point.z, 1}); !res) return res.error().relay();
//     this->_messy = true;
//     return {};
//   }

//   std::expected<void, error> set(size_t index, const double3& point) noexcept {
//     if (index >= _points.size())
//       return std::unexpected(error(errors::invalid_argument, "polyline point index out of bounds"));
//     _points[index] = {point.x, point.y, point.z, 1};
//     this->_messy = true;
//     return {};
//   }
//   std::expected<void, error> set(size_t index, const double4& point) noexcept { return set(index, point.xyz()); }

//   std::expected<void, error> append_quadratic_bezier(
//     const double3& p0, const double3& p1, const double3& p2, uint1 subdivisions) noexcept {
//     const auto n = subdivisions.x;
//     if (n < 1)
//       return std::unexpected(error(errors::invalid_argument, "quadratic Bezier subdivisions must be at least 1"));
//     const size_t first = _points.empty() ? 0 : 1;
//     for (size_t i = first; i <= n; ++i) {
//       const auto t = double(i) / double(n);
//       const auto u = 1.0 - t;
//       if (auto res = push_back(p0 * (u * u) + p1 * (2.0 * u * t) + p2 * (t * t)); !res) return res.error().relay();
//     }
//     return {};
//   }

//   std::expected<void, error> append_cubic_bezier(
//     const double3& p0, const double3& p1, const double3& p2, const double3& p3, uint1 subdivisions) noexcept {
//     const auto n = subdivisions.x;
//     if (n < 1) return std::unexpected(error(errors::invalid_argument, "cubic Bezier subdivisions must be at least
//     1")); const size_t first = _points.empty() ? 0 : 1; for (size_t i = first; i <= n; ++i) {
//       const auto t = double(i) / double(n);
//       const auto u = 1.0 - t;
//       const auto p = p0 * (u * u * u) + p1 * (3.0 * u * u * t) + p2 * (3.0 * u * t * t) + p3 * (t * t * t);
//       if (auto res = push_back(p); !res) return res.error().relay();
//     }
//     return {};
//   }

//   std::expected<void, error> remove_short_segments(double threshold = 0.0) noexcept {
//     if (threshold < 0)
//       return std::unexpected(error(errors::invalid_argument, "polyline threshold must be non-negative"));
//     if (_points.size() < 2) return {};
//     const auto limit = threshold * threshold;
//     std::vector<double4> next;
//     next.reserve(_points.size());
//     next.push_back(_points[0]);
//     for (size_t i = 1; i < _points.size(); ++i) {
//       if ((_points[i] - next.back()).squared_length() > limit) next.push_back(_points[i]);
//     }
//     if (auto res = _points.assign(next); !res) return res.error().relay();
//     this->_messy = true;
//     return {};
//   }

//   void clear() noexcept {
//     _points.clear();
//     this->_messy = true;
//   }
// };

// /// MARK: geom::mesh

// template<backend> class mesh : public geometry3_base<mesh<Backend>, Backend> {
// public:
//   struct remeshing_option_t {}; // no options for remeshing

// protected:
//   friend class geometry_base<mesh<Backend>, Backend>;
//   array1<double4, cpu> _mesh_vertices;
//   array1<uint3, cpu> _mesh_triangles;
//   array1<uint2, cpu> _mesh_edges;
//   remeshing_option_t _remeshing_option{};
//   std::expected<void, error> _triangulate(const remeshing_option_t& ro) noexcept;
//   std::expected<void, error> _triangulate_default() noexcept { return _triangulate(_remeshing_option); }

//   std::expected<void, error> _validate_index(uint32_t index) const noexcept {
//     if (index >= _mesh_vertices.size())
//       return std::unexpected(error(errors::invalid_argument, "mesh index out of bounds"));
//     return {};
//   }

// public:
//   constexpr mesh() noexcept = default;
//   constexpr const auto& vertices() const noexcept { return _mesh_vertices; }
//   constexpr const auto& triangles() const noexcept { return _mesh_triangles; }
//   constexpr const auto& edges() const noexcept { return _mesh_edges; }
//   constexpr size_t vertex_count() const noexcept { return _mesh_vertices.size(); }
//   constexpr size_t triangle_count() const noexcept { return _mesh_triangles.size(); }
//   constexpr size_t edge_count() const noexcept { return _mesh_edges.size(); }

//   /// creates a mesh from a prism with a circular base.
//   static std::expected<mesh, error> create(
//     const prism<circle<cpu>, Backend>& p, circle<cpu>::remeshing_option_t ro) noexcept {
//     mesh result;
//     result.copy_transformation_from(p);
//     const auto n = ro.subdivisions.x;
//     const auto h = p.height();
//     result._mesh_vertices.resize(2 * n + 2);
//     result._mesh_vertices[0] = {0, 0, 0, 1};
//     result._mesh_vertices[n + 1] = {0, 0, h, 1};
//     for (uint32_t i = 0; i < n; ++i) {
//       const auto theta = 2.0 * yw::pi * i / n;
//       const auto x = yw::cos(theta), y = yw::sin(theta);
//       result._mesh_vertices[i + 1] = {x, y, 0, 1};
//       result._mesh_vertices[n + i + 2] = {x, y, h, 1};
//     }
//     result._mesh_triangles.resize(4 * n);
//     for (uint32_t i = 0; i < n - 1; ++i) {
//       result._mesh_triangles[i] = {i + 2, i + 1, 0};                     // bottom
//       result._mesh_triangles[n + i] = {n + 1, n + i + 2, n + i + 3};     // top
//       result._mesh_triangles[2 * n + i] = {i + 1, i + 2, n + i + 2};     // side 1
//       result._mesh_triangles[3 * n + i] = {i + 2, n + i + 3, n + i + 2}; // side 2
//     }
//     result._mesh_triangles[n - 1] = {1, n, 0};                     // bottom
//     result._mesh_triangles[2 * n - 1] = {n + 1, 2 * n + 1, n + 2}; // top
//     result._mesh_triangles[3 * n - 1] = {n, 1, 2 * n + 1};         // side 1
//     result._mesh_triangles[4 * n - 1] = {1, n + 2, 2 * n + 1};     // side 2
//     if constexpr (Backend == gpu) {
//       if (auto res = decltype(result._gpu_vertices)::create(result._mesh_vertices))
//         result._gpu_vertices = std::move(*res);
//       else return res.error().relay();
//       if (auto res = decltype(result._gpu_triangles)::create(result._mesh_triangles))
//         result._gpu_triangles = std::move(*res);
//       else return res.error().relay();
//     }
//     return result;
//   }

//   /// creates a mesh from a pyramid with a circular base.
//   template<backend OtherBackend> static std::expected<mesh, error> create(
//     const pyramid<circle<cpu>, OtherBackend>& p, circle<cpu>::remeshing_option_t ro) noexcept {
//     mesh result;
//     result.copy_transformation_from(p);
//     const auto n = ro.subdivisions.x;
//     const auto h = p.height();
//     result._mesh_vertices.resize(n + 2);
//     result._mesh_vertices[0] = {0, 0, 0, 1};
//     result._mesh_vertices[n + 1] = {0, 0, h, 1};
//     for (uint32_t i = 0; i < n; ++i) {
//       const auto theta = 2.0 * yw::pi * i / n;
//       const auto x = yw::cos(theta), y = yw::sin(theta);
//       result._mesh_vertices[i + 1] = {x, y, 0, 1};
//     }
//     result._mesh_triangles.resize(2 * n);
//     for (uint32_t i = 0; i < n - 1; ++i) {
//       result._mesh_triangles[i] = {i + 2, i + 1, 0};         // bottom
//       result._mesh_triangles[n + i] = {i + 1, i + 2, n + 1}; // side
//     }
//     result._mesh_triangles[n - 1] = {1, n, 0};         // bottom
//     result._mesh_triangles[2 * n - 1] = {n, 1, n + 1}; // side
//     if constexpr (Backend == gpu) {
//       if (auto res = decltype(result._gpu_vertices)::create(result._mesh_vertices))
//         result._gpu_vertices = std::move(*res);
//       else return res.error().relay();
//       if (auto res = decltype(result._gpu_triangles)::create(result._mesh_triangles))
//         result._gpu_triangles = std::move(*res);
//       else return res.error().relay();
//     }
//     return result;
//   }

//   /// creates a mesh from a cube.
//   template<backend OtherBackend>
//   static std::expected<mesh, error> create(const prism<square<cpu>, OtherBackend>& p) noexcept {
//     mesh result;
//     result.copy_transformation_from(p);
//     const auto n = 4;
//     const auto h = p.height();
//     result._mesh_vertices.resize(n * 2);
//     result._mesh_vertices[0] = {-1, -1, 0, 1};
//     result._mesh_vertices[1] = {1, -1, 0, 1};
//     result._mesh_vertices[2] = {1, 1, 0, 1};
//     result._mesh_vertices[3] = {-1, 1, 0, 1};
//     result._mesh_vertices[4] = {-1, -1, h, 1};
//     result._mesh_vertices[5] = {1, -1, h, 1};
//     result._mesh_vertices[6] = {1, 1, h, 1};
//     result._mesh_vertices[7] = {-1, 1, h, 1};
//     result._mesh_triangles.resize(12);
//     result._mesh_triangles[0] = {2, 1, 0};
//     result._mesh_triangles[1] = {3, 2, 0};
//     result._mesh_triangles[2] = {0, 1, 4};
//     result._mesh_triangles[3] = {1, 5, 4};
//     result._mesh_triangles[4] = {1, 2, 5};
//     result._mesh_triangles[5] = {2, 6, 5};
//     result._mesh_triangles[6] = {2, 3, 6};
//     result._mesh_triangles[7] = {3, 7, 6};
//     result._mesh_triangles[8] = {3, 0, 7};
//     result._mesh_triangles[9] = {0, 4, 7};
//     result._mesh_triangles[10] = {4, 5, 6};
//     result._mesh_triangles[11] = {4, 6, 7};
//     if constexpr (Backend == gpu) {
//       if (auto res = decltype(result._gpu_vertices)::create(result._mesh_vertices))
//         result._gpu_vertices = std::move(*res);
//       else return res.error().relay();
//       if (auto res = decltype(result._gpu_triangles)::create(result._mesh_triangles))
//         result._gpu_triangles = std::move(*res);
//       else return res.error().relay();
//     }
//     return result;
//   }

//   /// creates a mesh from a pyramid.
//   template<backend OtherBackend>
//   static std::expected<mesh, error> create(const pyramid<square<cpu>, OtherBackend>& p) noexcept {
//     mesh result;
//     result.copy_transformation_from(p);
//     const auto n = 4;
//     const auto h = p.height();
//     result._mesh_vertices.resize(n + 1);
//     result._mesh_vertices[0] = {-1, -1, 0, 1};
//     result._mesh_vertices[1] = {1, -1, 0, 1};
//     result._mesh_vertices[2] = {1, 1, 0, 1};
//     result._mesh_vertices[3] = {-1, 1, 0, 1};
//     result._mesh_vertices[4] = {0, 0, h, 1};
//     result._mesh_triangles.resize(6);
//     result._mesh_triangles[0] = {0, 1, 4};
//     result._mesh_triangles[1] = {1, 2, 4};
//     result._mesh_triangles[2] = {2, 3, 4};
//     result._mesh_triangles[3] = {3, 0, 4};
//     result._mesh_triangles[4] = {0, 3, 2};
//     result._mesh_triangles[5] = {0, 2, 1};
//     if constexpr (Backend == gpu) {
//       if (auto res = decltype(result._gpu_vertices)::create(result._mesh_vertices))
//         result._gpu_vertices = std::move(*res);
//       else return res.error().relay();
//       if (auto res = decltype(result._gpu_triangles)::create(result._mesh_triangles))
//         result._gpu_triangles = std::move(*res);
//       else return res.error().relay();
//     }
//     return result;
//   }

//   /// creates a mesh from a prism with a polygonal base.
//   template<backend OtherBackend>
//   static std::expected<mesh, error> create(const prism<polygon<cpu>, OtherBackend>& p) noexcept {
//     mesh result;
//     result.copy_transformation_from(p);
//     const auto points = p.points();
//     const auto n = points.size();
//     const auto m = n - 2;
//     const auto h = p.height();
//     result._mesh_vertices.resize(n * 2);
//     for (size_t i = 0; i < n; ++i) {
//       result._mesh_vertices[i] = points[i];
//       result._mesh_vertices[i + n] = {points[i].x, points[i].y, points[i].z + h, 1};
//     }
//     result._mesh_triangles.resize(n * 2 + m * 2);
//     for (size_t i = 0; i < n - 1; ++i) {
//       result._mesh_triangles[i] = {i, i + 1, n + i};
//       result._mesh_triangles[i + n] = {i + 1, n + i + 1, n + i};
//     }
//     result._mesh_triangles[n - 1] = {n - 1, 0, 2 * n - 1};
//     result._mesh_triangles[n * 2 - 1] = {0, n, 2 * n - 1};
//     for (size_t i = 0; i < m; ++i) {
//       result._mesh_triangles[n * 2 + i] = {i + 2, i + 1, 0};
//       result._mesh_triangles[n * 2 + m + i] = {n, n + i + 1, n + i + 2};
//     }
//     if constexpr (Backend == gpu) {
//       if (auto res = decltype(result._gpu_vertices)::create(result._mesh_vertices))
//         result._gpu_vertices = std::move(*res);
//       else return res.error().relay();
//       if (auto res = decltype(result._gpu_triangles)::create(result._mesh_triangles))
//         result._gpu_triangles = std::move(*res);
//       else return res.error().relay();
//     }
//     return result;
//   }

//   /// creates a mesh from a pyramid with a polygonal base.
//   template<backend OtherBackend>
//   static std::expected<mesh, error> create(const pyramid<polygon<cpu>, OtherBackend>& p) noexcept {
//     mesh result;
//     result.copy_transformation_from(p);
//     const auto points = p.points();
//     const auto n = points.size();
//     const auto m = n - 2;
//     const auto h = p.height();
//     result._mesh_vertices.resize(n + 1);
//     result._mesh_vertices[n] = {0, 0, h, 1};
//     for (size_t i = 0; i < n; ++i) { result._mesh_vertices[i] = points[i]; }
//     result._mesh_triangles.resize(n + m);
//     for (size_t i = 0; i < n - 1; ++i) { result._mesh_triangles[i] = {i, i + 1, n}; }
//     result._mesh_triangles[n - 1] = {n - 1, 0, n};
//     for (size_t i = 0; i < m; ++i) { result._mesh_triangles[n + i] = {i + 2, i + 1, 0}; }
//     if constexpr (Backend == gpu) {
//       if (auto res = decltype(result._gpu_vertices)::create(result._mesh_vertices))
//         result._gpu_vertices = std::move(*res);
//       else return res.error().relay();
//       if (auto res = decltype(result._gpu_triangles)::create(result._mesh_triangles))
//         result._gpu_triangles = std::move(*res);
//       else return res.error().relay();
//     }
//     return result;
//   }

//   std::expected<uint32_t, error> add_vertex(const double3& vertex) noexcept {
//     if (_mesh_vertices.size() >= std::numeric_limits<uint32_t>::max())
//       return std::unexpected(error(errors::invalid_argument, "mesh vertex count is too large"));
//     const auto index = uint32_t(_mesh_vertices.size());
//     if (auto res = _mesh_vertices.push_back({vertex.x, vertex.y, vertex.z, 1}); !res) return res.error().relay();
//     this->_messy = true;
//     return index;
//   }
//   std::expected<uint32_t, error> add_vertex(const double4& vertex) noexcept { return add_vertex(vertex.xyz()); }

//   std::expected<void, error> add_triangle(uint3 triangle) noexcept {
//     if (auto res = _validate_index(triangle.x); !res) return res.error().relay();
//     if (auto res = _validate_index(triangle.y); !res) return res.error().relay();
//     if (auto res = _validate_index(triangle.z); !res) return res.error().relay();
//     if (triangle.x == triangle.y || triangle.y == triangle.z || triangle.z == triangle.x) return {};
//     if (auto res = _mesh_triangles.push_back(triangle); !res) return res.error().relay();
//     this->_messy = true;
//     return {};
//   }

//   std::expected<void, error> add_quad(uint32_t a, uint32_t b, uint32_t c, uint32_t d) noexcept {
//     if (auto res = add_triangle({a, b, c}); !res) return res.error().relay();
//     if (auto res = add_triangle({a, c, d}); !res) return res.error().relay();
//     return {};
//   }

//   std::expected<void, error> assign_vertices(const double4* vertices, size_t count) noexcept {
//     if (count > std::numeric_limits<uint32_t>::max())
//       return std::unexpected(error(errors::invalid_argument, "mesh vertex count is too large"));
//     if (auto res = _mesh_vertices.assign(vertices, count); !res) return res.error().relay();
//     for (auto& v : _mesh_vertices) v = {v.x, v.y, v.z, 1};
//     this->_messy = true;
//     return {};
//   }
//   template<contiguous_range<double4> Rg> std::expected<void, error> assign_vertices(Rg&& vertices) noexcept {
//     return assign_vertices(yw::data(vertices), yw::size(vertices));
//   }

//   std::expected<void, error> assign_triangles(const uint3* triangles, size_t count) noexcept {
//     for (size_t i = 0; i < count; ++i) {
//       if (triangles[i].x >= _mesh_vertices.size() || triangles[i].y >= _mesh_vertices.size() ||
//           triangles[i].z >= _mesh_vertices.size())
//         return std::unexpected(error(errors::invalid_argument, "mesh triangle index out of bounds"));
//     }
//     if (auto res = _mesh_triangles.assign(triangles, count); !res) return res.error().relay();
//     this->_messy = true;
//     return {};
//   }
//   template<contiguous_range<uint3> Rg> std::expected<void, error> assign_triangles(Rg&& triangles) noexcept {
//     return assign_triangles(yw::data(triangles), yw::size(triangles));
//   }

//   std::expected<void, error> rebuild_edges() noexcept {
//     struct edge_count {
//       uint32_t a{}, b{}, count{};
//     };
//     std::vector<edge_count> counts;
//     counts.reserve(_mesh_triangles.size() * 3);
//     auto add = [&](uint32_t a, uint32_t b) {
//       if (b < a) std::swap(a, b);
//       for (auto& e : counts)
//         if (e.a == a && e.b == b) {
//           ++e.count;
//           return;
//         }
//       counts.push_back({a, b, 1});
//     };
//     for (const auto& t : _mesh_triangles) {
//       add(t.x, t.y);
//       add(t.y, t.z);
//       add(t.z, t.x);
//     }
//     std::vector<uint2> edges;
//     for (const auto& e : counts)
//       if (e.count == 1) edges.push_back({e.a, e.b});
//     if (auto res = _mesh_edges.assign(edges); !res) return res.error().relay();
//     this->_messy = true;
//     return {};
//   }

//   std::expected<void, error> cleanup(double threshold = 0.0) noexcept {
//     if (threshold < 0)
//       return std::unexpected(error(errors::invalid_argument, "mesh cleanup threshold must be non-negative"));
//     const auto limit = threshold * threshold;
//     std::vector<double4> vertices;
//     std::vector<uint32_t> map(_mesh_vertices.size());
//     vertices.reserve(_mesh_vertices.size());
//     for (size_t i = 0; i < _mesh_vertices.size(); ++i) {
//       uint32_t index = uint32_t(vertices.size());
//       for (size_t j = 0; j < vertices.size(); ++j)
//         if ((_mesh_vertices[i] - vertices[j]).squared_length() <= limit) {
//           index = uint32_t(j);
//           break;
//         }
//       if (index == vertices.size()) vertices.push_back(_mesh_vertices[i]);
//       map[i] = index;
//     }
//     std::vector<uint3> triangles;
//     triangles.reserve(_mesh_triangles.size());
//     for (const auto& t : _mesh_triangles) {
//       const uint3 nt{map[t.x], map[t.y], map[t.z]};
//       if (nt.x == nt.y || nt.y == nt.z || nt.z == nt.x) continue;
//       bool duplicate = false;
//       for (const auto& old : triangles)
//         if (old == nt) {
//           duplicate = true;
//           break;
//         }
//       if (!duplicate) triangles.push_back(nt);
//     }
//     if (auto res = _mesh_vertices.assign(vertices); !res) return res.error().relay();
//     if (auto res = _mesh_triangles.assign(triangles); !res) return res.error().relay();
//     if (auto res = rebuild_edges(); !res) return res.error().relay();
//     this->_messy = true;
//     return {};
//   }

//   void clear() noexcept {
//     _mesh_vertices.clear();
//     _mesh_triangles.clear();
//     _mesh_edges.clear();
//     this->_messy = true;
//   }
// };
} // namespace yw::geom
