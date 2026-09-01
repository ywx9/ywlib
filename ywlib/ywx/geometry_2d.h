#pragma once
#include <ywx/geometry.h>

namespace yw::geom {

/// MARK: geom::segment

template<>
inline std::expected<void, error> segment<cpu>::_triangulate(const geom::remeshing_option<segment>& ro) noexcept {
  return {};
}
template<>
inline std::expected<void, error> segment<gpu>::_triangulate(const geom::remeshing_option<segment>& ro) noexcept {
  array1<float4, cpu> vertices(2);
  array1<uint2, cpu> edges(1);
  vertices[0] = {0, 0, 0, 1};
  vertices[1] = {1, 0, 0, 1};
  edges[0] = {0, 1};
  if (auto res = decltype(this->_gpu_vertices)::create(vertices)) this->_gpu_vertices = std::move(*res);
  else return res.error().relay();
  if (auto res = decltype(this->_gpu_edges)::create(edges)) this->_gpu_edges = std::move(*res);
  else return res.error().relay();
  return {};
}

/// MARK: geom::ray

template<> inline std::expected<void, error> ray<cpu>::_triangulate(const geom::remeshing_option<ray>& ro) noexcept {
  if (ro.length.x <= 0) return std::unexpected(error(errors::invalid_argument, "ray length must be positive"));
  this->_remeshing_option = ro;
  return {};
}

template<> inline std::expected<void, error> ray<gpu>::_triangulate(const geom::remeshing_option<ray>& ro) noexcept {
  if (ro.length.x <= 0) return std::unexpected(error(errors::invalid_argument, "ray length must be positive"));
  array1<float4, cpu> vertices(2);
  array1<uint2, cpu> edges(1);
  vertices[0] = {0, 0, 0, 1};
  vertices[1] = {ro.length.x, 0, 0, 1};
  edges[0] = {0, 1};
  if (auto res = decltype(this->_gpu_vertices)::create(vertices)) this->_gpu_vertices = std::move(*res);
  else return res.error().relay();
  if (auto res = decltype(this->_gpu_edges)::create(edges)) this->_gpu_edges = std::move(*res);
  else return res.error().relay();
  this->_remeshing_option = ro;
  return {};
}

/// MARK: geom::line

template<> inline std::expected<void, error> line<cpu>::_triangulate(const geom::remeshing_option<line>& ro) noexcept {
  if (ro.half_length.x <= 0)
    return std::unexpected(error(errors::invalid_argument, "line half length must be positive"));
  this->_remeshing_option = ro;
  return {};
}

template<> inline std::expected<void, error> line<gpu>::_triangulate(const geom::remeshing_option<line>& ro) noexcept {
  if (ro.half_length.x <= 0)
    return std::unexpected(error(errors::invalid_argument, "line half length must be positive"));
  array1<float4, cpu> vertices(2);
  array1<uint2, cpu> edges(1);
  vertices[0] = {-ro.half_length.x, 0, 0, 1};
  vertices[1] = {ro.half_length.x, 0, 0, 1};
  edges[0] = {0, 1};
  if (auto res = decltype(this->_gpu_vertices)::create(vertices)) this->_gpu_vertices = std::move(*res);
  else return res.error().relay();
  if (auto res = decltype(this->_gpu_edges)::create(edges)) this->_gpu_edges = std::move(*res);
  else return res.error().relay();
  this->_remeshing_option = ro;
  return {};
}

/// MARK: geom::plane

template<> inline std::expected<void, error> plane<cpu>::_triangulate(const geom::remeshing_option<plane>& ro) noexcept {
  if (ro.half_extent.x <= 0)
    return std::unexpected(error(errors::invalid_argument, "plane half extent must be positive"));
  this->_remeshing_option = ro;
  return {};
}

template<> inline std::expected<void, error> plane<gpu>::_triangulate(const geom::remeshing_option<plane>& ro) noexcept {
  const auto h = ro.half_extent.x;
  if (h <= 0) return std::unexpected(error(errors::invalid_argument, "plane half extent must be positive"));
  array1<float4, cpu> vertices(4);
  array1<uint3, cpu> triangles(2);
  array1<uint2, cpu> edges(4);
  vertices[0] = {-h, -h, 0, 1};
  vertices[1] = {h, -h, 0, 1};
  vertices[2] = {h, h, 0, 1};
  vertices[3] = {-h, h, 0, 1};
  triangles[0] = {0, 1, 2};
  triangles[1] = {0, 2, 3};
  edges[0] = {0, 1};
  edges[1] = {1, 2};
  edges[2] = {2, 3};
  edges[3] = {3, 0};
  if (auto res = decltype(this->_gpu_vertices)::create(vertices)) this->_gpu_vertices = std::move(*res);
  else return res.error().relay();
  if (auto res = decltype(this->_gpu_triangles)::create(triangles)) this->_gpu_triangles = std::move(*res);
  else return res.error().relay();
  if (auto res = decltype(this->_gpu_edges)::create(edges)) this->_gpu_edges = std::move(*res);
  else return res.error().relay();
  this->_remeshing_option = ro;
  return {};
}

/// MARK: geom::square

template<>
inline std::expected<void, error> square<cpu>::_triangulate(const geom::remeshing_option<square>& ro) noexcept {
  this->_remeshing_option = ro;
  return {};
}

template<>
inline std::expected<void, error> square<gpu>::_triangulate(const geom::remeshing_option<square>& ro) noexcept {
  array1<float4, cpu> vertices(5);
  array1<uint3, cpu> triangles(4);
  array1<uint2, cpu> edges(4);
  vertices[0] = {0, 0, 0, 1};
  vertices[1] = {-1, -1, 0, 1};
  vertices[2] = {1, -1, 0, 1};
  vertices[3] = {1, 1, 0, 1};
  vertices[4] = {-1, 1, 0, 1};
  triangles[0] = {0, 1, 2};
  triangles[1] = {0, 2, 3};
  triangles[2] = {0, 3, 4};
  triangles[3] = {0, 4, 1};
  edges[0] = {1, 2};
  edges[1] = {2, 3};
  edges[2] = {3, 4};
  edges[3] = {4, 1};
  if (auto res = decltype(this->_gpu_vertices)::create(vertices)) this->_gpu_vertices = std::move(*res);
  else return res.error().relay();
  if (auto res = decltype(this->_gpu_triangles)::create(triangles)) this->_gpu_triangles = std::move(*res);
  else return res.error().relay();
  if (auto res = decltype(this->_gpu_edges)::create(edges)) this->_gpu_edges = std::move(*res);
  else return res.error().relay();
  this->_remeshing_option = ro;
  return {};
}

/// MARK: geom::circle

template<>
inline std::expected<void, error> circle<cpu>::_triangulate(const geom::remeshing_option<circle>& ro) noexcept {
  if (ro.subdivisions.x < 3)
    return std::unexpected(error(errors::invalid_argument, "circle subdivisions must be at least 3"));
  this->_remeshing_option = ro;
  return {};
}

template<>
inline std::expected<void, error> circle<gpu>::_triangulate(const geom::remeshing_option<circle>& ro) noexcept {
  const auto n = ro.subdivisions.x;
  if (n < 3) return std::unexpected(error(errors::invalid_argument, "circle subdivisions must be at least 3"));
  array1<float4, cpu> vertices(size_t(n) + 1);
  array1<uint3, cpu> triangles(n);
  array1<uint2, cpu> edges(n);
  vertices[0] = {0, 0, 0, 1};
  for (uint32_t i = 0; i < n; ++i) {
    const auto angle = float(yw::pi2 * double(i) / double(n));
    vertices[i + 1] = {yw::cos(angle), yw::sin(angle), 0, 1};
    triangles[i] = {0, i + 1, (i + 1) % n + 1};
    edges[i] = {i + 1, (i + 1) % n + 1};
  }
  if (auto res = decltype(this->_gpu_vertices)::create(vertices)) this->_gpu_vertices = std::move(*res);
  else return res.error().relay();
  if (auto res = decltype(this->_gpu_triangles)::create(triangles)) this->_gpu_triangles = std::move(*res);
  else return res.error().relay();
  if (auto res = decltype(this->_gpu_edges)::create(edges)) this->_gpu_edges = std::move(*res);
  else return res.error().relay();
  this->_remeshing_option = ro;
  return {};
}

/// MARK: geom::polygon

template<>
inline std::expected<void, error> polygon<cpu>::_triangulate(const geom::remeshing_option<polygon>& ro) noexcept {
  this->_remeshing_option = ro;
  return {};
}

template<>
inline std::expected<void, error> polygon<gpu>::_triangulate(const geom::remeshing_option<polygon>& ro) noexcept {
  if (_points.size() >= std::numeric_limits<uint32_t>::max())
    return std::unexpected(error(errors::invalid_argument, "polygon point count is too large"));
  const auto vertex_count = _points.size();
  const auto triangle_count = vertex_count < 3 ? size_t(0) : vertex_count;
  const auto edge_count = vertex_count < 2 ? size_t(0) : vertex_count;
  array1<float4, cpu> vertices(vertex_count + 1);
  array1<uint3, cpu> triangles(triangle_count);
  array1<uint2, cpu> edges(edge_count);
  vertices[0] = {0, 0, 0, 1};
  for (size_t i = 0; i < vertex_count; ++i) vertices[i + 1] = float4(_points[i]);
  for (size_t i = 0; i < triangle_count; ++i) triangles[i] = {0, uint32_t(i + 1), uint32_t((i + 1) % vertex_count + 1)};
  for (size_t i = 0; i < edge_count; ++i) edges[i] = {uint32_t(i + 1), uint32_t((i + 1) % vertex_count + 1)};
  if (auto res = decltype(this->_gpu_vertices)::create(vertices)) this->_gpu_vertices = std::move(*res);
  else return res.error().relay();
  if (auto res = decltype(this->_gpu_triangles)::create(triangles)) this->_gpu_triangles = std::move(*res);
  else return res.error().relay();
  if (auto res = decltype(this->_gpu_edges)::create(edges)) this->_gpu_edges = std::move(*res);
  else return res.error().relay();
  this->_remeshing_option = ro;
  return {};
}

/// MARK: geom::arc

template<> inline std::expected<void, error> arc<cpu>::_triangulate(const geom::remeshing_option<arc>& ro) noexcept {
  if (ro.subdivisions.x < 1)
    return std::unexpected(error(errors::invalid_argument, "arc subdivisions must be at least 1"));
  this->_remeshing_option = ro;
  return {};
}

template<> inline std::expected<void, error> arc<gpu>::_triangulate(const geom::remeshing_option<arc>& ro) noexcept {
  const auto n = ro.subdivisions.x;
  if (n < 1) return std::unexpected(error(errors::invalid_argument, "arc subdivisions must be at least 1"));
  array1<float4, cpu> vertices(size_t(n) + 1);
  array1<uint2, cpu> edges(n);
  for (uint32_t i = 0; i <= n; ++i) vertices[i] = float4(point(double(i) / double(n)));
  for (uint32_t i = 0; i < n; ++i) edges[i] = {i, i + 1};
  if (auto res = decltype(this->_gpu_vertices)::create(vertices)) this->_gpu_vertices = std::move(*res);
  else return res.error().relay();
  if (auto res = decltype(this->_gpu_edges)::create(edges)) this->_gpu_edges = std::move(*res);
  else return res.error().relay();
  this->_remeshing_option = ro;
  return {};
}

/// MARK: geom::quadratic_bezier

template<> inline std::expected<void, error> quadratic_bezier<cpu>::_triangulate(
  const geom::remeshing_option<quadratic_bezier>& ro) noexcept {
  if (ro.subdivisions.x < 1)
    return std::unexpected(error(errors::invalid_argument, "quadratic Bezier subdivisions must be at least 1"));
  this->_remeshing_option = ro;
  return {};
}

template<> inline std::expected<void, error> quadratic_bezier<gpu>::_triangulate(
  const geom::remeshing_option<quadratic_bezier>& ro) noexcept {
  const auto n = ro.subdivisions.x;
  if (n < 1)
    return std::unexpected(error(errors::invalid_argument, "quadratic Bezier subdivisions must be at least 1"));
  array1<float4, cpu> vertices(size_t(n) + 1);
  array1<uint2, cpu> edges(n);
  for (uint32_t i = 0; i <= n; ++i) vertices[i] = float4(point(double(i) / double(n)));
  for (uint32_t i = 0; i < n; ++i) edges[i] = {i, i + 1};
  if (auto res = decltype(this->_gpu_vertices)::create(vertices)) this->_gpu_vertices = std::move(*res);
  else return res.error().relay();
  if (auto res = decltype(this->_gpu_edges)::create(edges)) this->_gpu_edges = std::move(*res);
  else return res.error().relay();
  this->_remeshing_option = ro;
  return {};
}

/// MARK: geom::cubic_bezier

template<> inline std::expected<void, error> cubic_bezier<cpu>::_triangulate(
  const geom::remeshing_option<cubic_bezier>& ro) noexcept {
  if (ro.subdivisions.x < 1)
    return std::unexpected(error(errors::invalid_argument, "cubic Bezier subdivisions must be at least 1"));
  this->_remeshing_option = ro;
  return {};
}

template<> inline std::expected<void, error> cubic_bezier<gpu>::_triangulate(
  const geom::remeshing_option<cubic_bezier>& ro) noexcept {
  const auto n = ro.subdivisions.x;
  if (n < 1) return std::unexpected(error(errors::invalid_argument, "cubic Bezier subdivisions must be at least 1"));
  array1<float4, cpu> vertices(size_t(n) + 1);
  array1<uint2, cpu> edges(n);
  for (uint32_t i = 0; i <= n; ++i) vertices[i] = float4(point(double(i) / double(n)));
  for (uint32_t i = 0; i < n; ++i) edges[i] = {i, i + 1};
  if (auto res = decltype(this->_gpu_vertices)::create(vertices)) this->_gpu_vertices = std::move(*res);
  else return res.error().relay();
  if (auto res = decltype(this->_gpu_edges)::create(edges)) this->_gpu_edges = std::move(*res);
  else return res.error().relay();
  this->_remeshing_option = ro;
  return {};
}
} // namespace yw::geom
