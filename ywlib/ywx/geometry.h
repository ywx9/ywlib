#pragma once
#include <ywx/array.h>
#include <ywx/buffer.h>

namespace yw::geom {

template<typename Geometry> class geometry_base<Geometry, gpu> : public geometry_base<Geometry, cpu> {
protected:
  constant_buffer<matrix<float, 4, 4>> _world;
  array1<float4, gpu> _vertices;
  array1<uint3, gpu> _triangles;
  array1<uint2, gpu> _edges;

public:
  /// gets world matrix
  constexpr matrix<float, 4, 4> world_matrix() const noexcept {
    return {
      matrix_row(float4(this->_rigid[0]) * this->_scale), matrix_row(float4(this->_rigid[1]) * this->_scale),
      matrix_row(float4(this->_rigid[2]) * this->_scale), matrix_row{0, 0, 0, 1}};
  }
  const auto& world() const noexcept { return _world; }
  const auto& vertices() const noexcept { return _vertices; }
  const auto& triangles() const noexcept { return _triangles; }
  const auto& edges() const noexcept { return _edges; }

  /// \param RemeshConfigs (optional) Configuration parameters for remeshing
  template<typename Self, typename... Ts>
  requires(sizeof...(Ts) == 0 || invocable_r<decltype(&Self::_make_mesh), std::expected<void, error>, Self&, Ts...>)
  std::expected<void, error> update_gpu(this Self& self, Ts&&... RemeshConfigs) noexcept {
    if constexpr (sizeof...(Ts) != 0) {
      if (auto res = self._make_mesh(static_cast<Ts&&>(RemeshConfigs)...); !res) return res.error().relay();
    } else if (self._messy || !self._vertices)
      if (auto res = self._make_mesh_default(); !res) return res.error().relay();
    if (!self._world) {
      if (auto res = decltype(self._world)::create(self.world_matrix())) self._world = std::move(*res);
      else return res.error().relay();
    } else if (self._dirty)
      if (auto res = self._world.copy_from(self.world_matrix()); !res) return res.error().relay();
    self._dirty = false, self._messy = false;
    return {};
  }
};

/// MARK: segment::make_mesh

template<> inline std::expected<void, error> segment<cpu>::_make_mesh() noexcept { return {}; }

template<> inline std::expected<void, error> segment<gpu>::_make_mesh() noexcept {
  array1<float4> vertices(2);
  array1<uint2> edges(1);
  vertices[0] = _start;
  vertices[1] = _end;
  edges[0] = {0, 1};
  if (auto res = decltype(_vertices)::create(vertices)) _vertices = std::move(*res);
  else return res.error().relay();
  if (auto res = decltype(_edges)::create(edges)) _edges = std::move(*res);
  else return res.error().relay();
  return {};
}

/// MARK: ray::make_mesh

template<> inline std::expected<void, error> ray<cpu>::_make_mesh(float1) noexcept { return {}; }

template<> inline std::expected<void, error> ray<gpu>::_make_mesh(float1 Length) noexcept {
  array1<float4> vertices(2);
  array1<uint2> edges(1);
  vertices[0] = _origin;
  vertices[1] = _origin + _direction * Length.x;
  edges[0] = {0, 1};
  if (auto res = decltype(_vertices)::create(vertices)) _vertices = std::move(*res);
  else return res.error().relay();
  if (auto res = decltype(_edges)::create(edges)) _edges = std::move(*res);
  else return res.error().relay();
  _length = Length.x;
  return {};
}

/// MARK: line::make_mesh

template<> inline std::expected<void, error> line<cpu>::_make_mesh(float1) noexcept { return {}; }

template<> inline std::expected<void, error> line<gpu>::_make_mesh(float1 HalfLength) noexcept {
  array1<float4> vertices(3);
  array1<uint2> edges(2);
  _half_length = HalfLength.x;
  vertices[0] = _point;
  vertices[1] = _point - _direction * HalfLength.x;
  vertices[2] = _point + _direction * HalfLength.x;
  edges[0] = {0, 1};
  edges[1] = {0, 2};
  if (auto res = decltype(_vertices)::create(vertices)) _vertices = std::move(*res);
  else return res.error().relay();
  if (auto res = decltype(_edges)::create(edges)) _edges = std::move(*res);
  else return res.error().relay();
  return {};
}

/// MARK: polyline::make_mesh

template<> inline std::expected<void, error> polyline<cpu>::_make_mesh() noexcept { return {}; }

template<> inline std::expected<void, error> polyline<gpu>::_make_mesh() noexcept {
  if (_points.size() > std::numeric_limits<uint32_t>::max())
    return std::unexpected(error(errors::invalid_argument, "polyline point count is too large"));
  const auto vertex_count = _points.size();
  const auto edge_count = vertex_count == 0 ? size_t(0) : vertex_count - 1;
  array1<float4> vertices(vertex_count);
  array1<uint2> edges(edge_count);
  for (size_t i = 0; i < vertex_count; ++i) vertices[i] = float4(_points[i]);
  for (size_t i = 0; i < edge_count; ++i) edges[i] = {uint32_t(i), uint32_t(i + 1)};
  if (auto res = decltype(_vertices)::create(vertices)) _vertices = std::move(*res);
  else return res.error().relay();
  if (auto res = decltype(_edges)::create(edges)) _edges = std::move(*res);
  else return res.error().relay();
  return {};
}

/// MARK: arc::make_mesh

template<> inline std::expected<void, error> arc<cpu>::_make_mesh(uint1) noexcept { return {}; }

template<> inline std::expected<void, error> arc<gpu>::_make_mesh(uint1 SegmentCount) noexcept {
  const auto n = SegmentCount.x;
  if (n < 1) return std::unexpected(error(errors::invalid_argument, "segment count must be at least 1"));
  if (_vertices && !this->_messy && n == _segment_count) return {};
  array1<float4> vertices(size_t(n) + 1);
  array1<uint2> edges(n);
  for (uint32_t i = 0; i <= n; ++i) {
    const auto t = double(i) / double(n);
    const auto angle = _start_angle + _sweep_angle * t;
    vertices[i] = {float(yw::cos(angle)), float(yw::sin(angle)), 0, 1};
  }
  for (uint32_t i = 0; i < n; ++i) edges[i] = {i, i + 1};
  if (auto res = decltype(_vertices)::create(vertices)) _vertices = std::move(*res);
  else return res.error().relay();
  if (auto res = decltype(_edges)::create(edges)) _edges = std::move(*res);
  else return res.error().relay();
  _segment_count = n;
  return {};
}

/// MARK: quadratic_bezier::make_mesh

template<> inline std::expected<void, error> quadratic_bezier<cpu>::_make_mesh(uint1) noexcept { return {}; }

template<> inline std::expected<void, error> quadratic_bezier<gpu>::_make_mesh(uint1 SegmentCount) noexcept {
  const auto n = SegmentCount.x;
  if (n < 1) return std::unexpected(error(errors::invalid_argument, "segment count must be at least 1"));
  if (_vertices && !this->_messy && n == _segment_count) return {};
  array1<float4> vertices(size_t(n) + 1);
  array1<uint2> edges(n);
  for (uint32_t i = 0; i <= n; ++i) vertices[i] = float4(point(double(i) / double(n)));
  for (uint32_t i = 0; i < n; ++i) edges[i] = {i, i + 1};
  if (auto res = decltype(_vertices)::create(vertices)) _vertices = std::move(*res);
  else return res.error().relay();
  if (auto res = decltype(_edges)::create(edges)) _edges = std::move(*res);
  else return res.error().relay();
  _segment_count = n;
  return {};
}

/// MARK: cubic_bezier::make_mesh

template<> inline std::expected<void, error> cubic_bezier<cpu>::_make_mesh(uint1) noexcept { return {}; }

template<> inline std::expected<void, error> cubic_bezier<gpu>::_make_mesh(uint1 SegmentCount) noexcept {
  const auto n = SegmentCount.x;
  if (n < 1) return std::unexpected(error(errors::invalid_argument, "segment count must be at least 1"));
  if (_vertices && !this->_messy && n == _segment_count) return {};
  array1<float4> vertices(size_t(n) + 1);
  array1<uint2> edges(n);
  for (uint32_t i = 0; i <= n; ++i) vertices[i] = float4(point(double(i) / double(n)));
  for (uint32_t i = 0; i < n; ++i) edges[i] = {i, i + 1};
  if (auto res = decltype(_vertices)::create(vertices)) _vertices = std::move(*res);
  else return res.error().relay();
  if (auto res = decltype(_edges)::create(edges)) _edges = std::move(*res);
  else return res.error().relay();
  _segment_count = n;
  return {};
}

/// MARK: circle::make_mesh

template<> inline std::expected<void, error> circle<cpu>::_make_mesh(uint1) noexcept { return {}; }

template<> inline std::expected<void, error> circle<gpu>::_make_mesh(uint1 SegmentCount) noexcept {
  const auto n = SegmentCount.x;
  if (n < 3) return std::unexpected(error(errors::invalid_argument, "segment count must be at least 3"));
  if (_vertices && !this->_messy && n == _segment_count) return {};
  array1<float4> vertices(n + 1);
  array1<uint3> triangles(n);
  array1<uint2> edges(n);
  vertices[0] = {0, 0, 0, 1};
  for (uint32_t i = 0; i < n; ++i) {
    const auto angle = float(yw::pi2 * double(i) / double(n));
    vertices[i + 1] = {yw::cos(angle), yw::sin(angle), 0, 1};
    triangles[i] = {0, i + 1, (i + 1) % n + 1};
    edges[i] = {i + 1, (i + 1) % n + 1};
  }
  if (auto res = decltype(_vertices)::create(vertices)) _vertices = std::move(*res);
  else return res.error().relay();
  if (auto res = decltype(_triangles)::create(triangles)) _triangles = std::move(*res);
  else return res.error().relay();
  if (auto res = decltype(_edges)::create(edges)) _edges = std::move(*res);
  else return res.error().relay();
  _segment_count = n;
  return {};
}

/// MARK: square::make_mesh

template<> inline std::expected<void, error> square<cpu>::_make_mesh() noexcept { return {}; }

template<> inline std::expected<void, error> square<gpu>::_make_mesh() noexcept {
  array1<float4> vertices(4);
  array1<uint3> triangles(2);
  array1<uint2> edges(4);
  vertices[0] = {-1, -1, 0, 1};
  vertices[1] = {1, -1, 0, 1};
  vertices[2] = {1, 1, 0, 1};
  vertices[3] = {-1, 1, 0, 1};
  triangles[0] = {0, 1, 2};
  triangles[1] = {0, 2, 3};
  edges[0] = {0, 1};
  edges[1] = {1, 2};
  edges[2] = {2, 3};
  edges[3] = {3, 0};
  if (auto res = decltype(_vertices)::create(vertices)) _vertices = std::move(*res);
  else return res.error().relay();
  if (auto res = decltype(_triangles)::create(triangles)) _triangles = std::move(*res);
  else return res.error().relay();
  if (auto res = decltype(_edges)::create(edges)) _edges = std::move(*res);
  else return res.error().relay();
  return {};
}

/// MARK: plane::make_mesh

template<> inline std::expected<void, error> plane<cpu>::_make_mesh(float1) noexcept { return {}; }

template<> inline std::expected<void, error> plane<gpu>::_make_mesh(float1 HalfExtent) noexcept {
  const auto h = HalfExtent.x;
  if (h <= 0) return std::unexpected(error(errors::invalid_argument, "plane half extent must be positive"));
  array1<float4> vertices(4);
  array1<uint3> triangles(2);
  array1<uint2> edges(4);
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
  if (auto res = decltype(_vertices)::create(vertices)) _vertices = std::move(*res);
  else return res.error().relay();
  if (auto res = decltype(_triangles)::create(triangles)) _triangles = std::move(*res);
  else return res.error().relay();
  if (auto res = decltype(_edges)::create(edges)) _edges = std::move(*res);
  else return res.error().relay();
  _half_extent = h;
  return {};
}

/// MARK: polygon::make_mesh

template<> inline std::expected<void, error> polygon<cpu>::_make_mesh() noexcept { return {}; }

template<> inline std::expected<void, error> polygon<gpu>::_make_mesh() noexcept {
  if (_points.size() > std::numeric_limits<uint32_t>::max())
    return std::unexpected(error(errors::invalid_argument, "polygon point count is too large"));
  const auto vertex_count = _points.size();
  const auto triangle_count = vertex_count < 3 ? size_t(0) : vertex_count - 2;
  const auto edge_count = vertex_count < 2 ? size_t(0) : vertex_count;
  array1<float4> vertices(vertex_count);
  array1<uint3> triangles(triangle_count);
  array1<uint2> edges(edge_count);
  for (size_t i = 0; i < vertex_count; ++i) vertices[i] = float4(_points[i].xyz(), 1);
  for (size_t i = 0; i < triangle_count; ++i) triangles[i] = {0, uint32_t(i + 1), uint32_t(i + 2)};
  for (size_t i = 0; i < edge_count; ++i) edges[i] = {uint32_t(i), uint32_t((i + 1) % vertex_count)};
  if (auto res = decltype(_vertices)::create(vertices)) _vertices = std::move(*res);
  else return res.error().relay();
  if (auto res = decltype(_triangles)::create(triangles)) _triangles = std::move(*res);
  else return res.error().relay();
  if (auto res = decltype(_edges)::create(edges)) _edges = std::move(*res);
  else return res.error().relay();
  return {};
}
} // namespace yw::geom
