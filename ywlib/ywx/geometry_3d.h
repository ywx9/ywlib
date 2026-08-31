#pragma once
#include <yw/geometry_3d.h>
#include <ywx/geometry.h>

namespace yw::geom {

// /// MARK: geom::polyline

// template<> inline std::expected<void, error> polyline<cpu>::_make_mesh(const remeshing_option_t& ro) noexcept {
//   _remeshing_option = ro;
//   return {};
// }

// template<> inline std::expected<void, error> polyline<gpu>::_make_mesh(const remeshing_option_t& ro) noexcept {
//   if (_points.size() > std::numeric_limits<uint32_t>::max())
//     return std::unexpected(error(errors::invalid_argument, "polyline point count is too large"));
//   const auto vertex_count = _points.size();
//   const auto edge_count = vertex_count == 0 ? size_t(0) : vertex_count - 1;
//   array1<float4, cpu> vertices(vertex_count);
//   array1<uint2, cpu> edges(edge_count);
//   for (size_t i = 0; i < vertex_count; ++i) vertices[i] = float4(_points[i]);
//   for (size_t i = 0; i < edge_count; ++i) edges[i] = {uint32_t(i), uint32_t(i + 1)};
//   if (auto res = decltype(this->_gpu_vertices)::create(vertices)) this->_gpu_vertices = std::move(*res);
//   else return res.error().relay();
//   if (auto res = decltype(this->_gpu_edges)::create(edges)) this->_gpu_edges = std::move(*res);
//   else return res.error().relay();
//   this->_remeshing_option = ro;
//   return {};
// }

// /// MARK: geom::mesh

// template<> inline std::expected<void, error> mesh<cpu>::_make_mesh(const remeshing_option_t& ro) noexcept {
//   _remeshing_option = ro;
//   return {};
// }

// template<> inline std::expected<void, error> mesh<gpu>::_make_mesh(const remeshing_option_t& ro) noexcept {
//   // mesh<cpu> m;
//   // if (auto res = m.assign_gpu_vertices(_mesh_gpu_vertices); !res) return res.error().relay();
//   // if (auto res = m.assign_gpu_triangles(_mesh_gpu_triangles); !res) return res.error().relay();
//   // if (auto res = m.rebuild_gpu_edges(); !res) return res.error().relay();
//   // array1<float4, cpu> vertices(m.vertex_count());
//   // for (size_t i = 0; i < m.vertex_count(); ++i) vertices[i] = float4(m.vertices()[i]);
//   // if (auto res = decltype(this->_gpu_vertices)::create(vertices)) this->_gpu_vertices = std::move(*res);
//   // else return res.error().relay();
//   // if (auto res = decltype(this->_gpu_triangles)::create(m.triangles())) this->_gpu_triangles = std::move(*res);
//   // else return res.error().relay();
//   // if (auto res = decltype(this->_gpu_edges)::create(m.edges())) this->_gpu_edges = std::move(*res);
//   // else return res.error().relay();
//   // this->_remeshing_option = ro;
//   // return {};
//   return {};
// }

// /// MARK: geom::prism

// template<typename Face, backend Backend> requires requires {
//   requires derived_from<Face, geometry2_base<Face, cpu>>;
//   requires Face::has_bounded_surface;
// } inline std::expected<void, error> prism<Face, Backend>::_make_mesh(const remeshing_option_t& ro) noexcept {
//   _remeshing_option = ro;
//   if constexpr (Backend == cpu) {
//     if (auto m = to_mesh(ro); !m) return m.error().relay();
//   } else {
//     if (auto m = to_mesh(ro)) {
//       array1<float4, cpu> vertices(m->vertex_count());
//       for (size_t i = 0; i < m->vertex_count(); ++i) vertices[i] = float4(m->vertices()[i]);
//       if (auto res = decltype(this->_gpu_vertices)::create(vertices)) this->_gpu_vertices = std::move(*res);
//       else return res.error().relay();
//       if (auto res = decltype(this->_gpu_triangles)::create(m->triangles())) this->_gpu_triangles = std::move(*res);
//       else return res.error().relay();
//       if (auto res = decltype(this->_gpu_edges)::create(m->edges())) this->_gpu_edges = std::move(*res);
//       else return res.error().relay();
//     } else return m.error().relay();
//   }
//   return {};
// }

// /// MARK: geom::pyramid

// template<typename Face, backend Backend> requires requires {
//   requires derived_from<Face, geometry2_base<Face, cpu>>;
//   requires Face::has_bounded_surface;
// } inline std::expected<void, error> pyramid<Face, Backend>::_make_mesh(const remeshing_option_t& ro) noexcept {
//   _remeshing_option = ro;
//   if (auto m = mesh<cpu>::create(*this, ro)) {
//     if (auto res = decltype(this->_gpu_vertices)::create(m->vertices())) this->_gpu_vertices = std::move(*res);
//     else return res.error().relay();
//     if (auto res = decltype(this->_gpu_triangles)::create(m->triangles())) this->_gpu_triangles = std::move(*res);
//     else return res.error().relay();
//     // if (auto res = decltype(this->_gpu_edges)::create(m->edges())) this->_gpu_edges = std::move(*res);
//     // else return res.error().relay();
//   } else return m.error().relay();
//   return {};
// }

// /// MARK: geom::sphere

// template<> inline std::expected<void, error> sphere<cpu>::_make_mesh(const remeshing_option_t& ro) noexcept {
//   _remeshing_option = ro;
//   if (auto m = to_mesh(ro); !m) return m.error().relay();
//   return {};
// }

// template<> inline std::expected<void, error> sphere<gpu>::_make_mesh(const remeshing_option_t& ro) noexcept {
//   if (auto m = to_mesh(ro)) {
//     array1<float4, cpu> vertices(m->vertex_count());
//     for (size_t i = 0; i < m->vertex_count(); ++i) vertices[i] = float4(m->vertices()[i]);
//     if (auto res = decltype(this->_gpu_vertices)::create(vertices)) this->_gpu_vertices = std::move(*res);
//     else return res.error().relay();
//     if (auto res = decltype(this->_gpu_triangles)::create(m->triangles())) this->_gpu_triangles = std::move(*res);
//     else return res.error().relay();
//     if (auto res = decltype(this->_gpu_edges)::create(m->edges())) this->_gpu_edges = std::move(*res);
//     else return res.error().relay();
//   } else return m.error().relay();
//   this->_remeshing_option = ro;
//   return {};
// }
} // namespace yw::geom
