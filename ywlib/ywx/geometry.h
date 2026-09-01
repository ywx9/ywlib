#pragma once
#include <ywx/array.h>
#include <ywx/buffer.h>

namespace yw::geom {

template<template<backend> typename Geometry> class geometry_base<Geometry, gpu> : public geometry_base<Geometry, cpu> {
protected:
  constant_buffer<matrix<float, 4, 4>> _cb_world;
  array1<float4, gpu> _gpu_vertices;
  array1<uint3, gpu> _gpu_triangles;
  array1<uint2, gpu> _gpu_edges;
  bool _mesh_ready = false;

public:
  const auto world_matrix() const noexcept { return matrix<float, 4, 4>(this->transformation4()); }
  const auto& cb_world() const noexcept { return _cb_world; }
  const auto& gpu_vertices() const noexcept { return _gpu_vertices; }
  const auto& gpu_triangles() const noexcept { return _gpu_triangles; }
  const auto& gpu_edges() const noexcept { return _gpu_edges; }

  template<typename Self> std::expected<void, error> update_gpu(this Self& self) noexcept {
    if (self._messy || !self._mesh_ready) {
      if (auto res = self._triangulate(self._remeshing_option); !res) return res.error().relay();
      self._mesh_ready = true;
    }
    if (!self._cb_world) {
      if (auto res = decltype(self._cb_world)::create(self.world_matrix())) self._cb_world = std::move(*res);
      else return res.error().relay();
    } else if (self._dirty)
      if (auto res = self._cb_world.copy_from(self.world_matrix()); !res) return res.error().relay();
    self._dirty = false, self._messy = false;
    return {};
  }

  // template<typename Self>
  // std::expected<void, error> update_gpu(this Self& self, const geom::remeshing_option<Geometry>& ro) noexcept {
  //   if (auto res = self._triangulate(ro); !res) return res.error().relay();
  //   self._mesh_ready = true;
  //   if (!self._cb_world) {
  //     if (auto res = decltype(self._cb_world)::create(self.world_matrix())) self._cb_world = std::move(*res);
  //     else return res.error().relay();
  //   } else if (self._dirty)
  //     if (auto res = self._cb_world.copy_from(self.world_matrix()); !res) return res.error().relay();
  //   self._dirty = false, self._messy = false;
  //   return {};
  // }

  // std::expected<void, error> update_gpu(geom::remeshing_option<Geometry> ro) noexcept {
  //   if (auto res = static_cast<Geometry<gpu>&>(*this)._triangulate(ro); !res) return res.error().relay();
  //   _mesh_ready = true;
  //   if (!this->_cb_world) {
  //     if (auto res = decltype(this->_cb_world)::create(this->world_matrix())) this->_cb_world = std::move(*res);
  //     else return res.error().relay();
  //   } else if (this->_dirty)
  //     if (auto res = this->_cb_world.copy_from(this->world_matrix()); !res) return res.error().relay();
  //   this->_dirty = false, this->_messy = false;
  //   return {};
  // }
};
} // namespace yw::geom
