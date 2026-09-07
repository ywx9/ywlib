#pragma once
#include <ywx/geometry.h>

namespace yw::geom {

namespace detail {
inline constexpr uint32_t pack_tangent_xy(double x, double y) noexcept {
  const auto len = yw::sqrt(x * x + y * y);
  if (len <= 0) return tangent_x;
  return pack_vector_to_uint(float4{float(x / len), float(y / len), 0, 1});
}

inline constexpr uint32_t pack_tangent_xy(const double4& tangent) noexcept {
  return pack_tangent_xy(tangent.x, tangent.y);
}

inline constexpr float2 uv_from_unit_xy(float x, float y) noexcept { return {x * 0.5f + 0.5f, y * 0.5f + 0.5f}; }

inline constexpr float2 uv_from_bbox_xy(float x, float y, const bbox<cpu>& bbox) noexcept {
  const auto size = bbox.size();
  return {
    size.x > 0 ? float((double(x) - bbox.min.x) / size.x) : 0.5f,
    size.y > 0 ? float((double(y) - bbox.min.y) / size.y) : 0.5f};
}
} // namespace detail

/// MARK: geom::segment

template<> inline std::expected<void, error> segment<cpu>::_triangulate() noexcept { return {}; }
template<> inline std::expected<void, error> segment<gpu>::_triangulate() noexcept {
  array1<vertex<gpu>, cpu> vertices(2);
  array1<uint2, cpu> edges(1);
  vertices[0] = {.position = {0, 0, 0, 1}, .uv = {0, 0}, .normal = normal_z, .tangent = tangent_x};
  vertices[1] = {.position = {1, 0, 0, 1}, .uv = {1, 0}, .normal = normal_z, .tangent = tangent_x};
  edges[0] = {0, 1};
  if (auto res = decltype(this->_gpu_vertices)::create(vertices)) this->_gpu_vertices = std::move(*res);
  else return res.error().relay();
  if (auto res = decltype(this->_gpu_edges)::create(edges)) this->_gpu_edges = std::move(*res);
  else return res.error().relay();
  return {};
}

/// MARK: geom::ray

template<> inline std::expected<void, error> ray<cpu>::_triangulate() noexcept { return {}; }
template<> inline std::expected<void, error> ray<gpu>::_triangulate() noexcept {
  const auto l = this->_remeshing_option.length.x;
  if (l <= 0) return std::unexpected(error(errors::invalid_argument, "ray length must be positive"));
  array1<vertex<gpu>, cpu> vertices(2);
  array1<uint2, cpu> edges(1);
  vertices[0] = {.position = {0, 0, 0, 1}, .uv = {0, 0}, .normal = normal_z, .tangent = tangent_x};
  vertices[1] = {.position = {l, 0, 0, 1}, .uv = {1, 0}, .normal = normal_z, .tangent = tangent_x};
  edges[0] = {0, 1};
  if (auto res = decltype(this->_gpu_vertices)::create(vertices)) this->_gpu_vertices = std::move(*res);
  else return res.error().relay();
  if (auto res = decltype(this->_gpu_edges)::create(edges)) this->_gpu_edges = std::move(*res);
  else return res.error().relay();
  return {};
}

/// MARK: geom::line

template<> inline std::expected<void, error> line<cpu>::_triangulate() noexcept { return {}; }
template<> inline std::expected<void, error> line<gpu>::_triangulate() noexcept {
  const auto hl = this->_remeshing_option.half_length.x;
  if (hl <= 0) return std::unexpected(error(errors::invalid_argument, "line half length must be positive"));
  array1<vertex<gpu>, cpu> vertices(2);
  array1<uint2, cpu> edges(1);
  vertices[0] = {.position = {-hl, 0, 0, 1}, .uv = {0, 0}, .normal = normal_z, .tangent = tangent_x};
  vertices[1] = {.position = {hl, 0, 0, 1}, .uv = {1, 0}, .normal = normal_z, .tangent = tangent_x};
  edges[0] = {0, 1};
  if (auto res = decltype(this->_gpu_vertices)::create(vertices)) this->_gpu_vertices = std::move(*res);
  else return res.error().relay();
  if (auto res = decltype(this->_gpu_edges)::create(edges)) this->_gpu_edges = std::move(*res);
  else return res.error().relay();
  return {};
}

/// MARK: geom::arc

template<> inline std::expected<void, error> arc<cpu>::_triangulate() noexcept { return {}; }
template<> inline std::expected<void, error> arc<gpu>::_triangulate() noexcept {
  const auto n = this->_remeshing_option.subdivisions.x;
  if (n < 1) return std::unexpected(error(errors::invalid_argument, "arc subdivisions must be at least 1"));
  array1<vertex<gpu>, cpu> vertices(size_t(n) + 1);
  array1<uint2, cpu> edges(n);
  for (uint32_t i = 0; i <= n; ++i) {
    const auto t = double(i) / double(n);
    vertices[i] = {
      .position = float4(point(t)),
      .uv = {float(t), 0},
      .normal = normal_z,
      .tangent = detail::pack_tangent_xy(tangent(t))};
  }
  for (uint32_t i = 0; i < n; ++i) edges[i] = {i, i + 1};
  if (auto res = decltype(this->_gpu_vertices)::create(vertices)) this->_gpu_vertices = std::move(*res);
  else return res.error().relay();
  if (auto res = decltype(this->_gpu_edges)::create(edges)) this->_gpu_edges = std::move(*res);
  else return res.error().relay();
  return {};
}

/// MARK: geom::quadratic_bezier

template<> inline std::expected<void, error> quadratic_bezier<cpu>::_triangulate() noexcept { return {}; }
template<> inline std::expected<void, error> quadratic_bezier<gpu>::_triangulate() noexcept {
  const auto n = this->_remeshing_option.subdivisions.x;
  if (n < 1)
    return std::unexpected(error(errors::invalid_argument, "quadratic Bezier subdivisions must be at least 1"));
  array1<vertex<gpu>, cpu> vertices(size_t(n) + 1);
  array1<uint2, cpu> edges(n);
  for (uint32_t i = 0; i <= n; ++i) {
    const auto t = double(i) / double(n);
    vertices[i] = {
      .position = float4(point(t)),
      .uv = {float(t), 0},
      .normal = normal_z,
      .tangent = detail::pack_tangent_xy(tangent(t))};
  }
  for (uint32_t i = 0; i < n; ++i) edges[i] = {i, i + 1};
  if (auto res = decltype(this->_gpu_vertices)::create(vertices)) this->_gpu_vertices = std::move(*res);
  else return res.error().relay();
  if (auto res = decltype(this->_gpu_edges)::create(edges)) this->_gpu_edges = std::move(*res);
  else return res.error().relay();
  return {};
}

/// MARK: geom::cubic_bezier

template<> inline std::expected<void, error> cubic_bezier<cpu>::_triangulate() noexcept { return {}; }
template<> inline std::expected<void, error> cubic_bezier<gpu>::_triangulate() noexcept {
  const auto n = this->_remeshing_option.subdivisions.x;
  if (n < 1) return std::unexpected(error(errors::invalid_argument, "cubic Bezier subdivisions must be at least 1"));
  array1<vertex<gpu>, cpu> vertices(size_t(n) + 1);
  array1<uint2, cpu> edges(n);
  for (uint32_t i = 0; i <= n; ++i) {
    const auto t = double(i) / double(n);
    vertices[i] = {
      .position = float4(point(t)),
      .uv = {float(t), 0},
      .normal = normal_z,
      .tangent = detail::pack_tangent_xy(tangent(t))};
  }
  for (uint32_t i = 0; i < n; ++i) edges[i] = {i, i + 1};
  if (auto res = decltype(this->_gpu_vertices)::create(vertices)) this->_gpu_vertices = std::move(*res);
  else return res.error().relay();
  if (auto res = decltype(this->_gpu_edges)::create(edges)) this->_gpu_edges = std::move(*res);
  else return res.error().relay();
  return {};
}

/// MARK: geom::plane

template<> inline std::expected<void, error> plane<cpu>::_triangulate() noexcept { return {}; }
template<> inline std::expected<void, error> plane<gpu>::_triangulate() noexcept {
  const auto h = this->_remeshing_option.half_extent.x;
  if (h <= 0) return std::unexpected(error(errors::invalid_argument, "plane half extent must be positive"));
  array1<vertex<gpu>, cpu> vertices(4);
  array1<uint3, cpu> triangles(2);
  array1<uint2, cpu> edges(4);
  vertices[0] = {.position = {-h, -h, 0, 1}, .uv = {0, 0}, .normal = normal_z, .tangent = tangent_x};
  vertices[1] = {.position = {h, -h, 0, 1}, .uv = {1, 0}, .normal = normal_z, .tangent = tangent_x};
  vertices[2] = {.position = {h, h, 0, 1}, .uv = {1, 1}, .normal = normal_z, .tangent = tangent_x};
  vertices[3] = {.position = {-h, h, 0, 1}, .uv = {0, 1}, .normal = normal_z, .tangent = tangent_x};
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
  return {};
}

/// MARK: geom::square

template<> inline std::expected<void, error> square<cpu>::_triangulate() noexcept { return {}; }
template<> inline std::expected<void, error> square<gpu>::_triangulate() noexcept {
  array1<vertex<gpu>, cpu> vertices(5);
  array1<uint3, cpu> triangles(4);
  array1<uint2, cpu> edges(4);
  vertices[0] = {.position = {0, 0, 0, 1}, .uv = {0.5f, 0.5f}, .normal = normal_z, .tangent = tangent_x};
  vertices[1] = {.position = {-1, -1, 0, 1}, .uv = {0, 0}, .normal = normal_z, .tangent = tangent_x};
  vertices[2] = {.position = {1, -1, 0, 1}, .uv = {1, 0}, .normal = normal_z, .tangent = tangent_x};
  vertices[3] = {.position = {1, 1, 0, 1}, .uv = {1, 1}, .normal = normal_z, .tangent = tangent_x};
  vertices[4] = {.position = {-1, 1, 0, 1}, .uv = {0, 1}, .normal = normal_z, .tangent = tangent_x};
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
  return {};
}

/// MARK: geom::circle

template<> inline std::expected<void, error> circle<cpu>::_triangulate() noexcept { return {}; }
template<> inline std::expected<void, error> circle<gpu>::_triangulate() noexcept {
  const auto n = this->_remeshing_option.subdivisions.x;
  if (n < 3) return std::unexpected(error(errors::invalid_argument, "circle subdivisions must be at least 3"));
  array1<vertex<gpu>, cpu> vertices(size_t(n) + 1);
  array1<uint3, cpu> triangles(n);
  array1<uint2, cpu> edges(n);
  vertices[0] = {.position = {0, 0, 0, 1}, .uv = {0.5f, 0.5f}, .normal = normal_z, .tangent = tangent_x};
  for (uint32_t i = 0; i < n; ++i) {
    const auto angle = float(yw::pi2 * double(i) / double(n));
    const auto x = yw::cos(angle);
    const auto y = yw::sin(angle);
    vertices[i + 1] = {
      .position = {x, y, 0, 1}, .uv = detail::uv_from_unit_xy(x, y), .normal = normal_z, .tangent = tangent_x};
    triangles[i] = {0, i + 1, (i + 1) % n + 1};
    edges[i] = {i + 1, (i + 1) % n + 1};
  }
  if (auto res = decltype(this->_gpu_vertices)::create(vertices)) this->_gpu_vertices = std::move(*res);
  else return res.error().relay();
  if (auto res = decltype(this->_gpu_triangles)::create(triangles)) this->_gpu_triangles = std::move(*res);
  else return res.error().relay();
  if (auto res = decltype(this->_gpu_edges)::create(edges)) this->_gpu_edges = std::move(*res);
  else return res.error().relay();
  return {};
}

/// MARK: geom::polygon

template<> inline std::expected<void, error> polygon<cpu>::_triangulate() noexcept { return {}; }
template<> inline std::expected<void, error> polygon<gpu>::_triangulate() noexcept {
  if (_points.size() >= std::numeric_limits<uint32_t>::max())
    return std::unexpected(error(errors::invalid_argument, "polygon point count is too large"));
  const auto vertex_count = _points.size();
  const auto triangle_count = vertex_count < 3 ? size_t(0) : vertex_count;
  const auto edge_count = vertex_count < 2 ? size_t(0) : vertex_count;
  auto bbox = local_bbox();
  if (!bbox && vertex_count != 0) return bbox.error().relay();
  array1<vertex<gpu>, cpu> vertices(vertex_count + 1);
  array1<uint3, cpu> triangles(triangle_count);
  array1<uint2, cpu> edges(edge_count);
  vertices[0] = {.position = {0, 0, 0, 1}, .uv = {0.5f, 0.5f}, .normal = normal_z, .tangent = tangent_x};
  for (size_t i = 0; i < vertex_count; ++i) {
    const auto p = float4(_points[i]);
    vertices[i + 1] = {
      .position = p, .uv = detail::uv_from_bbox_xy(p.x, p.y, *bbox), .normal = normal_z, .tangent = tangent_x};
  }
  for (size_t i = 0; i < triangle_count; ++i) triangles[i] = {0, uint32_t(i + 1), uint32_t((i + 1) % vertex_count + 1)};
  for (size_t i = 0; i < edge_count; ++i) edges[i] = {uint32_t(i + 1), uint32_t((i + 1) % vertex_count + 1)};
  if (auto res = decltype(this->_gpu_vertices)::create(vertices)) this->_gpu_vertices = std::move(*res);
  else return res.error().relay();
  if (auto res = decltype(this->_gpu_triangles)::create(triangles)) this->_gpu_triangles = std::move(*res);
  else return res.error().relay();
  if (auto res = decltype(this->_gpu_edges)::create(edges)) this->_gpu_edges = std::move(*res);
  else return res.error().relay();
  return {};
}
} // namespace yw::geom
