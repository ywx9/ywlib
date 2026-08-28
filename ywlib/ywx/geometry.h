#pragma once
#include <ywx/array.h>
#include <ywx/buffer.h>

namespace yw::geom {

template<typename Geometry> class geometry_base<Geometry, gpu> : public geometry_base<Geometry, cpu> {
protected:
  constant_buffer<matrix<float, 4, 4>> _world;
  structured_buffer<float4> _vertices;
  structured_buffer<uint4> _triangles;
  /// updates world matrix
  std::expected<void, error> update_world() noexcept {
    if (!this->_dirty && _world) return {};
    if (!_world) {
      if (auto res = decltype(_world)::create(world())) _world = std::move(*res);
      else return res.error().relay();
    }
    if (auto res = _world.copy_from(world())) {
      this->_dirty = false;
      return {};
    }
    else return res.error().relay();
  }

public:
  /// gets world matrix
  constexpr matrix<float, 4, 4> world() const noexcept {
    return {
      matrix_row(float4(this->_rigid[0]) * this->_scale), matrix_row(float4(this->_rigid[1]) * this->_scale),
      matrix_row(float4(this->_rigid[2]) * this->_scale), matrix_row{0, 0, 0, 1}};
  }
  std::expected<void, error> update_gpu() noexcept { return update_world(); }
  const auto& world_buffer() const noexcept { return _world; }
  const auto& vertices() const noexcept { return _vertices; }
  const auto& triangles() const noexcept { return _triangles; }
  uint32_t triangle_count() const noexcept { return _triangles.size(); }
};

/// MARK: circle::make_mesh

template<> inline std::expected<void, error> circle<cpu>::make_mesh(circle<cpu>&, uint1) noexcept { return {}; }

template<> inline std::expected<void, error> circle<gpu>::make_mesh(circle<gpu>& self, uint1 SegmentCount) noexcept {
  const uint32_t segment_count = SegmentCount.x;
  if (segment_count < 3) return std::unexpected(error(errors::invalid_argument, "segment count must be at least 3"));
  std::vector<float4> vertices(segment_count + 1);
  std::vector<uint4> triangles(segment_count);
  vertices[0] = {0, 0, 0, 1};
  for (uint32_t i = 0; i < segment_count; ++i) {
    const auto angle = float(yw::pi2 * double(i) / double(segment_count));
    vertices[i + 1] = {yw::cos(angle), yw::sin(angle), 0, 1};
    triangles[i] = {0, i + 1, (i + 1) % segment_count + 1, 0};
  }
  if (auto res = decltype(self._vertices)::create(vertices)) self._vertices = std::move(*res);
  else return res.error().relay();
  if (auto res = decltype(self._triangles)::create(triangles)) self._triangles = std::move(*res);
  else return res.error().relay();
  return {};
}
} // namespace yw::geom
