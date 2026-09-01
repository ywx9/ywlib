#pragma once
#include <yw/geometry_3d.h>
#include <ywx/geometry.h>

namespace yw::geom {

/// MARK: geom::prism

template<template<backend> typename Face, backend Backend>
  requires Face<Backend>::has_bounded_surface
inline std::expected<void, error> prism<Face, Backend>::_triangulate(
  const geom::remeshing_option<remove_backend<prism, Face>::template type>& ro) noexcept {
  this->_remeshing_option = ro;
  auto vertices = this->template _make_vertices<float>();
  if (!vertices) return vertices.error().relay();
  auto triangles = this->_make_triangles();
  if (!triangles) return triangles.error().relay();
  auto edges = this->_make_edges();
  if (!edges) return edges.error().relay();

  if constexpr (Backend == gpu) {
    if (auto res = decltype(this->_gpu_vertices)::create(*vertices)) this->_gpu_vertices = std::move(*res);
    else return res.error().relay();
    if (auto res = decltype(this->_gpu_triangles)::create(*triangles)) this->_gpu_triangles = std::move(*res);
    else return res.error().relay();
    if (auto res = decltype(this->_gpu_edges)::create(*edges)) this->_gpu_edges = std::move(*res);
    else return res.error().relay();
  }
  return {};
}

/// MARK: geom::pyramid

template<template<backend> typename Face, backend Backend>
  requires Face<Backend>::has_bounded_surface
inline std::expected<void, error> pyramid<Face, Backend>::_triangulate(
  const geom::remeshing_option<remove_backend<pyramid, Face>::template type>& ro) noexcept {
  this->_remeshing_option = ro;
  auto vertices = this->template _make_vertices<float>();
  if (!vertices) return vertices.error().relay();
  auto triangles = this->_make_triangles();
  if (!triangles) return triangles.error().relay();
  auto edges = this->_make_edges();
  if (!edges) return edges.error().relay();

  if constexpr (Backend == gpu) {
    if (auto res = decltype(this->_gpu_vertices)::create(*vertices)) this->_gpu_vertices = std::move(*res);
    else return res.error().relay();
    if (auto res = decltype(this->_gpu_triangles)::create(*triangles)) this->_gpu_triangles = std::move(*res);
    else return res.error().relay();
    if (auto res = decltype(this->_gpu_edges)::create(*edges)) this->_gpu_edges = std::move(*res);
    else return res.error().relay();
  }
  return {};
}
} // namespace yw::geom
