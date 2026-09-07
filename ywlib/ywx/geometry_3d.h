#pragma once
#include <yw/geometry_3d.h>
#include <ywx/geometry.h>
#include <ywx/texture.h>

namespace yw::geom {

namespace internal {
inline constexpr string_view<char> _hlsl_fill_texture_template = yw_hlsl_row_major_pragma yw_hlsl_vertex
  R"(
StructuredBuffer<Vertex> vertices : register(t0);
StructuredBuffer<uint3> triangles : register(t1);
cbuffer Constants : register(b0) {
  float4 bbox_min;
  float4 bbox_size;
};
struct VSOut {
  float4 position : SV_POSITION;
  float4 color : COLOR0;
};
void vsmain(uint i : SV_VertexID, out VSOut output) {
  Vertex v = vertices[triangles[i / 3][i % 3]];
  float2 uv = v.uv;
  output.position = float4(uv.x * 2.0f - 1.0f, 1.0f - uv.y * 2.0f, 0.0f, 1.0f);
  output.color = float4(saturate((v.position.xyz - bbox_min.xyz) / max(abs(bbox_size.xyz), 1e-6f)), 1.0f);
}
float4 psmain(VSOut input) : SV_TARGET {
  return input.color;
}
)";
inline constexpr string_view<char> _hlsl_stroke_texture_template = yw_hlsl_row_major_pragma yw_hlsl_vertex
  R"(
StructuredBuffer<Vertex> vertices : register(t0);
StructuredBuffer<uint3> triangles : register(t1);
struct VSOut {
  float4 position : SV_POSITION;
};
void vsmain(uint i : SV_VertexID, out VSOut output) {
  Vertex v = vertices[triangles[i / 3][i % 3]];
  float2 uv = v.uv;
  output.position = float4(uv.x * 2.0f - 1.0f, 1.0f - uv.y * 2.0f, 0.0f, 1.0f);
}
struct GSOut {
  float4 position : SV_POSITION;
};
void append_edge(VSOut a, VSOut b, inout LineStream<GSOut> output) {
  GSOut v;
  v.position = a.position;
  output.Append(v);
  v.position = b.position;
  output.Append(v);
  output.RestartStrip();
}
[maxvertexcount(6)]
void gsmain(triangle VSOut input[3], inout LineStream<GSOut> output) {
  append_edge(input[0], input[1], output);
  append_edge(input[1], input[2], output);
  append_edge(input[2], input[0], output);
}
float4 psmain(GSOut input) : SV_TARGET {
  return float4(0.0f, 0.0f, 0.0f, 1.0f);
}
)";

inline constexpr float2 prism_cap_center_uv(bool top) noexcept {
  return {0.5f, top ? 1.0f / 6.0f : 5.0f / 6.0f};
}

inline constexpr float2 prism_cap_vertex_uv(size_t i, size_t n, bool top) noexcept {
  constexpr auto r = 1.0f / 6.0f;
  const auto center = prism_cap_center_uv(top);
  // Align the cap's belt-facing direction with U = 1/2. For odd n this
  // bisects an edge; for even n it passes through a vertex.
  const auto phase = double(i % n) / double(n) - 0.5;
  const auto a = float((top ? yw::pi * 0.5 : -yw::pi * 0.5) + (top ? -yw::pi2 : yw::pi2) * phase);
  return {center.x + yw::cos(a) * r, center.y + yw::sin(a) * r};
}

inline constexpr float prism_side_u(size_t i, size_t n) noexcept {
  return float(i) / float(n);
}

inline constexpr float2 prism_side_uv(float u, bool top) noexcept {
  return {u, top ? 1.0f / 3.0f : 2.0f / 3.0f};
}

inline constexpr float2 pyramid_base_center_uv() noexcept {
  return prism_cap_center_uv(false);
}

inline constexpr float2 pyramid_base_vertex_uv(size_t i, size_t n) noexcept {
  return prism_cap_vertex_uv(i, n, false);
}

inline constexpr float2 pyramid_side_apex_uv() noexcept {
  return {0.5f, 1.0f / 3.0f};
}

inline constexpr float2 pyramid_side_vertex_uv(size_t i, size_t n) noexcept {
  constexpr float2 center{0.5f, 1.0f / 3.0f};
  constexpr float2 radius{0.5f, 1.0f / 3.0f};
  // Open at ring vertex 0, from the left endpoint to the right endpoint.
  // The middle of the arc faces the base, with the same ring phase.
  const auto a = float(yw::pi * (1.0 - double(i) / double(n)));
  if (i == 0) return {0, center.y};
  if (i == n) return {1, center.y};
  return {center.x + yw::cos(a) * radius.x, center.y + yw::sin(a) * radius.y};
}
} // namespace internal

struct texture_template_constants {
  float4 bbox_min;
  float4 bbox_size;
};

template<typename Geometry>
std::expected<bitmap_texture, error> make_texture_template(Geometry& geom, int2 TextureSize) noexcept {
  if (auto res = geom.update_gpu(); !res) return res.error().relay();
  auto bbox = geom.local_bbox();
  if (!bbox) return bbox.error().relay();
  auto rt_res = bitmap_texture::create(TextureSize);
  if (!rt_res) return rt_res.error().relay();
  auto rt = std::move(*rt_res);
  if (auto res = rt.clear(colors::transparent); !res) return res.error().relay();

  texture_template_constants constants{
    .bbox_min = float4(bbox->min),
    .bbox_size = float4(bbox->size())};
  auto cb_res = constant_buffer<texture_template_constants>::create(constants);
  if (!cb_res) return cb_res.error().relay();
  auto cb = std::move(*cb_res);

  D3D11_VIEWPORT vp{0.0f, 0.0f, float(TextureSize.x), float(TextureSize.y), 0.0f, 1.0f};
  d3d::context()->RSSetViewports(1, &vp);

  auto vs = vertex_shader::create(internal::_hlsl_fill_texture_template);
  if (!vs) return vs.error().relay();
  auto ps = pixel_shader::create(internal::_hlsl_fill_texture_template, "psmain");
  if (!ps) return ps.error().relay();
  if (auto r = rendering::create(rt)) {
    if (auto res = r->set_vs(*vs, cb, geom.gpu_vertices(), geom.gpu_triangles()); !res) return res.error().relay();
    if (auto res = r->set_ps(*ps); !res) return res.error().relay();
    if (auto res = r->render_triangles(uint32_t(geom.gpu_triangles().size())); !res) return res.error().relay();
  } else return r.error().relay();

  vs = vertex_shader::create(internal::_hlsl_stroke_texture_template);
  if (!vs) return vs.error().relay();
  auto gs = geometry_shader::create(internal::_hlsl_stroke_texture_template, "gsmain");
  if (!gs) return gs.error().relay();
  ps = pixel_shader::create(internal::_hlsl_stroke_texture_template, "psmain");
  if (!ps) return ps.error().relay();
  if (auto r = rendering::create(rt)) {
    if (auto res = r->set_vs(*vs, geom.gpu_vertices(), geom.gpu_triangles()); !res) return res.error().relay();
    if (auto res = r->set_gs(*gs); !res) return res.error().relay();
    if (auto res = r->set_ps(*ps); !res) return res.error().relay();
    if (auto res = r->render_triangles(uint32_t(geom.gpu_triangles().size())); !res) return res.error().relay();
  } else return r.error().relay();
  return rt;
}

namespace detail {
inline constexpr uint32_t pack_vector3(float x, float y, float z) noexcept {
  const auto len = yw::sqrt(x * x + y * y + z * z);
  if (len <= 0) return normal_z;
  return pack_vector_to_uint(float4{x / len, y / len, z / len, 1});
}

inline constexpr uint32_t pack_vector3(float3 v) noexcept { return pack_vector3(v.x, v.y, v.z); }

inline constexpr uint32_t pack_tangent3(float3 v) noexcept {
  const auto len = yw::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
  if (len <= 0) return tangent_x;
  return pack_vector_to_uint(float4{v.x / len, v.y / len, v.z / len, 1});
}

inline constexpr float2 atlas_uv(float2 uv, float y0, float y1) noexcept { return {uv.x, y0 + uv.y * (y1 - y0)}; }

inline constexpr float2 unit_disk_uv(float x, float y, float y0, float y1) noexcept {
  return atlas_uv({x * 0.5f + 0.5f, y * 0.5f + 0.5f}, y0, y1);
}

inline constexpr float2 bbox_uv(float x, float y, const bbox<cpu>& b, float y0, float y1) noexcept {
  const auto s = b.size();
  const float u = s.x > 0 ? float((double(x) - b.min.x) / s.x) : 0.5f;
  const float v = s.y > 0 ? float((double(y) - b.min.y) / s.y) : 0.5f;
  return atlas_uv({u, v}, y0, y1);
}

inline constexpr float2 bbox_unit_xy(float x, float y, const bbox<cpu>& b) noexcept {
  const auto s = b.size();
  const float u = s.x > 0 ? float((double(x) - b.min.x) / s.x) * 2.0f - 1.0f : 0.0f;
  const float v = s.y > 0 ? float((double(y) - b.min.y) / s.y) * 2.0f - 1.0f : 0.0f;
  return {u, v};
}

inline constexpr float2 cap_net_uv(float2 p, bool top) noexcept {
  constexpr auto r = 1.0f / 6.0f;
  if (top) return {0.5f - p.y * r, 1.0f / 6.0f - p.x * r};
  else return {0.5f - p.y * r, 5.0f / 6.0f + p.x * r};
}

inline constexpr float3 normal_from_points(float4 a, float4 b, float4 c) noexcept {
  return cross((b - a).xyz(), (c - a).xyz()).normalized();
}

inline constexpr uint32_t pack_projected_tangent(float3 tangent, float3 normal) noexcept {
  return pack_tangent3((tangent - normal * dot(normal, tangent)).normalized());
}
} // namespace detail

/// MARK: geom::prism

template<template<backend> typename Face, backend Backend> requires Face<Backend>::has_bounded_surface
inline std::expected<void, error> prism<Face, Backend>::_triangulate() noexcept {
  if constexpr (Backend == cpu) {
    return {};
  } else {
    size_t n = 0;
    if constexpr (same_as<Face<cpu>, circle<cpu>>) n = this->_remeshing_option.subdivisions.x;
    else if constexpr (same_as<Face<cpu>, square<cpu>>) n = 4;
    else if constexpr (same_as<Face<cpu>, polygon<cpu>>) n = _points.size();
    else return std::unexpected(error(errors::invalid_argument, "unsupported prism face"));

    if (n < 3) return std::unexpected(error(errors::invalid_argument, "prism vertex count must be at least 3"));
    if (n > (std::numeric_limits<uint32_t>::max() - 2) / 6)
      return std::unexpected(error(errors::invalid_argument, "prism vertex count is too large"));

    const auto h = float(_height);
    const auto zsign = h < 0 ? -1.0f : 1.0f;
    const auto bottom_normal = detail::pack_vector3(0, 0, -zsign);
    const auto top_normal = detail::pack_vector3(0, 0, zsign);
    bbox<cpu> face_bbox{{-1, -1, 0, 1}, {1, 1, 0, 1}};
    if constexpr (same_as<Face<cpu>, polygon<cpu>>) {
      auto lb = local_bbox();
      if (!lb) return lb.error().relay();
      face_bbox = *lb;
    }

    const auto cap_count = n + 1;
    const auto side_count = n * 4;
    array1<vertex<gpu>, cpu> vertices(cap_count * 2 + side_count);
    array1<uint3, cpu> triangles(n * 4);
    array1<uint2, cpu> edges;

    auto point = [&](size_t i, float z) noexcept -> float4 {
      if constexpr (same_as<Face<cpu>, circle<cpu>>) {
        const auto a = float(yw::pi2 * double(i % n) / double(n));
        return {yw::cos(a), yw::sin(a), z, 1};
      } else if constexpr (same_as<Face<cpu>, square<cpu>>) {
        constexpr float2 ps[] = {{-1, -1}, {1, -1}, {1, 1}, {-1, 1}};
        return {ps[i % 4].x, ps[i % 4].y, z, 1};
      } else {
        const auto& p = _points[i % n];
        return {float(p.x), float(p.y), z, 1};
      }
    };

    vertices[0] = {.position = {0, 0, 0, 1},
      .uv = internal::prism_cap_center_uv(false),
      .normal = bottom_normal,
      .tangent = tangent_x};
    vertices[cap_count] = {
      .position = {0, 0, h, 1}, .uv = internal::prism_cap_center_uv(true), .normal = top_normal, .tangent = tangent_x};
    for (size_t i = 0; i < n; ++i) {
      const auto bp = point(i, 0);
      const auto tp = point(i, h);
      vertices[i + 1] = {
        .position = bp, .uv = internal::prism_cap_vertex_uv(i, n, false), .normal = bottom_normal, .tangent = tangent_x};
      vertices[cap_count + i + 1] = {
        .position = tp, .uv = internal::prism_cap_vertex_uv(i, n, true), .normal = top_normal, .tangent = tangent_x};
    }

    const auto side_base = cap_count * 2;
    for (size_t i = 0; i < n; ++i) {
      const auto p0 = point(i, 0);
      const auto p1 = point(i + 1, 0);
      const auto q0 = point(i, h);
      const auto q1 = point(i + 1, h);
      const auto edge = (p1 - p0).xyz();
      const auto u0 = internal::prism_side_u(i, n);
      const auto u1 = internal::prism_side_u(i + 1, n);
      const auto base = side_base + i * 4;
      if constexpr (same_as<Face<cpu>, circle<cpu>>) {
        const auto normal0 = detail::pack_vector3(p0.x, p0.y, 0);
        const auto normal1 = detail::pack_vector3(p1.x, p1.y, 0);
        const auto tangent0 = detail::pack_tangent3({-p0.y, p0.x, 0});
        const auto tangent1 = detail::pack_tangent3({-p1.y, p1.x, 0});
        vertices[base + 0] = {
          .position = p0, .uv = internal::prism_side_uv(u0, false), .normal = normal0, .tangent = tangent0};
        vertices[base + 1] = {
          .position = p1, .uv = internal::prism_side_uv(u1, false), .normal = normal1, .tangent = tangent1};
        vertices[base + 2] = {
          .position = q0, .uv = internal::prism_side_uv(u0, true), .normal = normal0, .tangent = tangent0};
        vertices[base + 3] = {
          .position = q1, .uv = internal::prism_side_uv(u1, true), .normal = normal1, .tangent = tangent1};
      } else {
        const auto normal3 = detail::normal_from_points(p0, p1, q0) * zsign;
        const auto normal = detail::pack_vector3(normal3);
        const auto tangent = detail::pack_projected_tangent(edge, normal3);
        vertices[base + 0] = {
          .position = p0, .uv = internal::prism_side_uv(u0, false), .normal = normal, .tangent = tangent};
        vertices[base + 1] = {
          .position = p1, .uv = internal::prism_side_uv(u1, false), .normal = normal, .tangent = tangent};
        vertices[base + 2] = {
          .position = q0, .uv = internal::prism_side_uv(u0, true), .normal = normal, .tangent = tangent};
        vertices[base + 3] = {
          .position = q1, .uv = internal::prism_side_uv(u1, true), .normal = normal, .tangent = tangent};
      }
    }

    for (uint32_t i = 0; i < n; ++i) {
      triangles[i] = {0, uint32_t((i + 1) % n + 1), i + 1};
      triangles[n + i] = {uint32_t(cap_count), uint32_t(cap_count + i + 1), uint32_t(cap_count + (i + 1) % n + 1)};
      const auto base = uint32_t(side_base + i * 4);
      triangles[n * 2 + i] = {base + 0, base + 1, base + 2};
      triangles[n * 3 + i] = {base + 1, base + 3, base + 2};
    }

    if (auto res = decltype(this->_gpu_vertices)::create(vertices)) this->_gpu_vertices = std::move(*res);
    else return res.error().relay();
    if (auto res = decltype(this->_gpu_triangles)::create(triangles)) this->_gpu_triangles = std::move(*res);
    else return res.error().relay();
    if (auto res = decltype(this->_gpu_edges)::create(edges)) this->_gpu_edges = std::move(*res);
    else return res.error().relay();
    return {};
  }
}

/// MARK: geom::pyramid

template<template<backend> typename Face, backend Backend> requires Face<Backend>::has_bounded_surface
inline std::expected<void, error> pyramid<Face, Backend>::_triangulate() noexcept {
  if constexpr (Backend == cpu) {
    return {};
  } else {
    size_t n = 0;
    if constexpr (same_as<Face<cpu>, circle<cpu>>) n = this->_remeshing_option.subdivisions.x;
    else if constexpr (same_as<Face<cpu>, square<cpu>>) n = 4;
    else if constexpr (same_as<Face<cpu>, polygon<cpu>>) n = _points.size();
    else return std::unexpected(error(errors::invalid_argument, "unsupported pyramid face"));

    if (n < 3) return std::unexpected(error(errors::invalid_argument, "pyramid vertex count must be at least 3"));
    if (n > (std::numeric_limits<uint32_t>::max() - 2) / 4)
      return std::unexpected(error(errors::invalid_argument, "pyramid vertex count is too large"));

    const auto h = float(_height);
    const float4 apex{0, 0, h, 1};
    const auto zsign = h < 0 ? -1.0f : 1.0f;
    const auto bottom_normal = detail::pack_vector3(0, 0, -zsign);

    array1<vertex<gpu>, cpu> vertices((n + 1) + n * 3);
    array1<uint3, cpu> triangles(n * 2);
    array1<uint2, cpu> edges;

    auto point = [&](size_t i) noexcept -> float4 {
      if constexpr (same_as<Face<cpu>, circle<cpu>>) {
        const auto a = float(yw::pi2 * double(i % n) / double(n));
        return {yw::cos(a), yw::sin(a), 0, 1};
      } else if constexpr (same_as<Face<cpu>, square<cpu>>) {
        constexpr float2 ps[] = {{-1, -1}, {1, -1}, {1, 1}, {-1, 1}};
        return {ps[i % 4].x, ps[i % 4].y, 0, 1};
      } else {
        const auto& p = _points[i % n];
        return {float(p.x), float(p.y), 0, 1};
      }
    };

    vertices[0] = {
      .position = {0, 0, 0, 1},
      .uv = internal::pyramid_base_center_uv(),
      .normal = bottom_normal,
      .tangent = tangent_x};
    for (size_t i = 0; i < n; ++i) {
      const auto p = point(i);
      vertices[i + 1] = {
        .position = p, .uv = internal::pyramid_base_vertex_uv(i, n), .normal = bottom_normal, .tangent = tangent_x};
    }

    const auto side_base = n + 1;
    for (size_t i = 0; i < n; ++i) {
      const auto p0 = point(i);
      const auto p1 = point(i + 1);
      const auto edge = (p1 - p0).xyz();
      float3 normal3;
      if constexpr (same_as<Face<cpu>, circle<cpu>>) {
        normal3 = float3{p0.x, p0.y, float(1.0 / yw::max(yw::abs(double(h)), 1e-12)) * zsign}.normalized();
      } else {
        normal3 = detail::normal_from_points(p0, p1, apex) * zsign;
      }
      const auto normal = detail::pack_vector3(normal3);
      const auto tangent = detail::pack_projected_tangent(edge, normal3);
      const auto base = side_base + i * 3;
      vertices[base + 0] = {
        .position = p0, .uv = internal::pyramid_side_vertex_uv(i, n), .normal = normal, .tangent = tangent};
      vertices[base + 1] = {
        .position = p1, .uv = internal::pyramid_side_vertex_uv(i + 1, n), .normal = normal, .tangent = tangent};
      vertices[base + 2] = {
        .position = apex, .uv = internal::pyramid_side_apex_uv(), .normal = normal, .tangent = tangent};
    }

    for (uint32_t i = 0; i < n; ++i) {
      triangles[i] = {0, uint32_t((i + 1) % n + 1), i + 1};
      const auto base = uint32_t(side_base + i * 3);
      triangles[n + i] = {base, base + 1, base + 2};
    }

    if (auto res = decltype(this->_gpu_vertices)::create(vertices)) this->_gpu_vertices = std::move(*res);
    else return res.error().relay();
    if (auto res = decltype(this->_gpu_triangles)::create(triangles)) this->_gpu_triangles = std::move(*res);
    else return res.error().relay();
    if (auto res = decltype(this->_gpu_edges)::create(edges)) this->_gpu_edges = std::move(*res);
    else return res.error().relay();
    return {};
  }
}
/// MARK: geom::sphere

namespace internal {
// Two pentagonal crowns followed by an open belt. Crown ring r has 5*r
// vertices; the belt has n+1 rows of 5*n+1 vertices (including both cut edges).
struct sphere_mesh_layout {
  size_t n;
  size_t crown_size() const noexcept { return 1 + 5 * n * (n + 1) / 2; }
  size_t vertex_count() const noexcept { return 2 * crown_size() + (n + 1) * (5 * n + 1); }
  size_t crown(bool top, size_t sector, uint2 rc) const noexcept {
    const auto r = size_t(rc.x), c = size_t(rc.y);
    const auto base = top ? crown_size() : 0;
    return base + (r ? 1 + 5 * r * (r - 1) / 2 + (sector * r + c) % (5 * r) : 0);
  }
  size_t belt(uint2 xy) const noexcept { return 2 * crown_size() + size_t(xy.y) * (5 * n + 1) + xy.x; }
};

inline std::expected<void, error> make_sphere_mesh(
  uint32_t level, array1<vertex<gpu>, cpu>& vertices, array1<uint3, cpu>& triangles) noexcept {
  // Check before shifting or allocating, including D3D's 32-bit buffer byte sizes.
  if (level >= 16) return std::unexpected(error(errors::invalid_argument, "sphere subdivision level is too large"));
  const uint32_t n = uint32_t(1) << level;
  const uint64_t vertex_count = 2 + uint64_t(5) * n * (n + 1) + uint64_t(n + 1) * (5 * n + 1);
  const uint64_t triangle_count = uint64_t(20) * n * n;
  if (vertex_count > std::numeric_limits<uint32_t>::max() / sizeof(vertex<gpu>) ||
      triangle_count > std::numeric_limits<uint32_t>::max() / sizeof(uint3))
    return std::unexpected(error(errors::invalid_argument, "sphere mesh is too large"));
  if (auto res = vertices.resize(size_t(vertex_count)); !res) return res.error().relay();
  if (auto res = triangles.resize(size_t(triangle_count)); !res) return res.error().relay();
  const sphere_mesh_layout layout{n};

  struct sample {
    double3 position;
    float2 uv;
    uint2 grid;
  };
  auto midpoint = [](const sample& a, const sample& b) noexcept -> sample {
    return {(a.position + b.position).normalized(), (a.uv + b.uv) * 0.5f,
      {(a.grid.x + b.grid.x) / 2, (a.grid.y + b.grid.y) / 2}};
  };
  size_t triangle = 0;
  // Logical grid coordinates identify shared vertices without an edge map.
  // Repeated writes along face boundaries produce the same midpoint and UV.
  auto subdivide = [&](auto&& self, const auto& index, const sample& a, const sample& b,
                       const sample& c, uint32_t depth) noexcept -> void {
    if (depth) {
      const auto ab = midpoint(a, b), bc = midpoint(b, c), ca = midpoint(c, a);
      self(self, index, a, ab, ca, depth - 1);
      self(self, index, ab, b, bc, depth - 1);
      self(self, index, ca, bc, c, depth - 1);
      self(self, index, ab, bc, ca, depth - 1);
      return;
    }
    const auto ia = uint32_t(index(a.grid)), ib = uint32_t(index(b.grid)), ic = uint32_t(index(c.grid));
    auto write = [&](uint32_t i, const sample& s) noexcept {
      vertices[i] = {.position = {float(s.position.x), float(s.position.y), float(s.position.z), 1}, .uv = s.uv};
    };
    write(ia, a), write(ib, b), write(ic, c);
    triangles[triangle++] = {ia, ib, ic};
  };

  // Icosahedron with poles on Z and two staggered pentagonal rings.
  double3 rings[2][5];
  const auto z = 1.0 / yw::sqrt(5.0), radius = 2.0 / yw::sqrt(5.0);
  for (size_t top = 0; top < 2; ++top)
    for (size_t i = 0; i < 5; ++i) {
      const auto angle = yw::pi2 * (double(i) + double(top) * 0.5) / 5.0;
      rings[top][i] = {radius * yw::cos(angle), radius * yw::sin(angle), top ? z : -z};
    }

  // Separate UV islands: congruent pentagons in the outer thirds and a
  // parallelogram in the middle third. All level-zero edges remain straight.
  // Each pentagon has a circumcircle of diameter 1/3 and points upward.
  // Reversed top numbering preserves the outward surface's UV orientation.
  constexpr auto uv_radius = 1.0 / 6.0;
  for (size_t top = 0; top < 2; ++top) {
    // The lower crown's upward tip is ring vertex 2, at belt U = 4/11.
    const float2 center{top ? 5.0f / 11.0f : 4.0f / 11.0f, top ? 1.0f / 6.0f : 5.0f / 6.0f};
    float2 rim[5];
    for (size_t i = 0; i < 5; ++i) {
      const auto sector = top ? -double(i) : double(i) - 2.0;
      const auto angle = -yw::pi * 0.5 + yw::pi2 * sector / 5.0;
      rim[i] = center + float2{float(uv_radius * yw::cos(angle)), float(uv_radius * yw::sin(angle))};
    }
    for (size_t sector = 0; sector < 5; ++sector) {
      const auto next = (sector + 1) % 5;
      const sample pole{{0, 0, top ? 1.0 : -1.0}, center, {0, 0}};
      const sample a{rings[top][sector], rim[sector], {n, 0}};
      const sample b{rings[top][next], rim[next], {n, n}};
      const auto index = [&](uint2 rc) noexcept { return layout.crown(top != 0, sector, rc); };
      if (top) subdivide(subdivide, index, pole, a, b, level);
      else subdivide(subdivide, index, pole, b, a, level);
    }
  }
  const auto belt_index = [&](uint2 xy) noexcept { return layout.belt(xy); };
  for (uint32_t i = 0; i < 5; ++i) {
    const sample a{rings[0][i], {float(2 * i) / 11.0f, 2.0f / 3.0f}, {i * n, 0}};
    const sample b{rings[0][(i + 1) % 5], {float(2 * i + 2) / 11.0f, 2.0f / 3.0f}, {(i + 1) * n, 0}};
    const sample c{rings[1][i], {float(2 * i + 1) / 11.0f, 1.0f / 3.0f}, {i * n, n}};
    const sample d{rings[1][(i + 1) % 5], {float(2 * i + 3) / 11.0f, 1.0f / 3.0f}, {(i + 1) * n, n}};
    subdivide(subdivide, belt_index, a, b, c, level);
    subdivide(subdivide, belt_index, b, d, c, level);
  }

  // Smooth radial normals, and UV derivatives averaged only within each island.
  array1<float3, cpu> tangents;
  if (auto res = tangents.resize(vertices.size()); !res) return res.error().relay();
  for (const auto& t : triangles) {
    const auto& a = vertices[t.x];
    const auto& b = vertices[t.y];
    const auto& c = vertices[t.z];
    const auto ab = b.uv - a.uv, ac = c.uv - a.uv;
    const auto tangent = ((b.position - a.position).xyz() * ac.y - (c.position - a.position).xyz() * ab.y) /
      (ab.x * ac.y - ab.y * ac.x);
    tangents[t.x] += tangent, tangents[t.y] += tangent, tangents[t.z] += tangent;
  }
  for (size_t i = 0; i < vertices.size(); ++i) {
    auto& v = vertices[i];
    const auto normal = v.position.xyz().normalized();
    v.normal = detail::pack_vector3(normal);
    const auto tangent = (tangents[i] - normal * dot(normal, tangents[i])).normalized();
    // V points down in all three islands, hence negative tangent handedness.
    v.tangent = pack_vector_to_uint(float4{tangent.x, tangent.y, tangent.z, -1});
  }
  return {};
}
} // namespace internal

template<backend Backend> inline std::expected<void, error> sphere<Backend>::_triangulate() noexcept {
  if constexpr (Backend == cpu) return {};
  else {
    array1<vertex<gpu>, cpu> vertices;
    array1<uint3, cpu> triangles;
    if (auto res = internal::make_sphere_mesh(this->_remeshing_option.level.x, vertices, triangles); !res)
      return res.error().relay();
    auto gpu_vertices = decltype(this->_gpu_vertices)::create(vertices);
    if (!gpu_vertices) return gpu_vertices.error().relay();
    auto gpu_triangles = decltype(this->_gpu_triangles)::create(triangles);
    if (!gpu_triangles) return gpu_triangles.error().relay();
    this->_gpu_vertices = std::move(*gpu_vertices);
    this->_gpu_triangles = std::move(*gpu_triangles);
    return {};
  }
}

/// MARK: geom::polyline

template<backend Backend> inline std::expected<void, error> polyline<Backend>::_triangulate() noexcept {
  if constexpr (Backend == cpu) return {};
  else {
    if (_points.size() > std::numeric_limits<uint32_t>::max())
      return std::unexpected(error(errors::invalid_argument, "polyline vertex count is too large"));
    array1<vertex<gpu>, cpu> vertices;
    array1<uint2, cpu> edges;
    if (auto res = vertices.resize(_points.size()); !res) return res.error().relay();
    if (_points.size() > 1)
      if (auto res = edges.resize(_points.size() - 1); !res) return res.error().relay();

    for (size_t i = 0; i < _points.size(); ++i)
      vertices[i] = {.position = float4(_points[i]), .uv = {0, 0}, .normal = normal_z, .tangent = tangent_x};
    for (uint32_t i = 0; i + 1 < _points.size(); ++i) edges[i] = {i, i + 1};

    auto gpu_vertices = decltype(this->_gpu_vertices)::create(vertices);
    if (!gpu_vertices) return gpu_vertices.error().relay();
    auto gpu_edges = decltype(this->_gpu_edges)::create(edges);
    if (!gpu_edges) return gpu_edges.error().relay();
    this->_gpu_vertices = std::move(*gpu_vertices);
    this->_gpu_edges = std::move(*gpu_edges);
    return {};
  }
}

/// MARK: geom::mesh

template<backend Backend> inline std::expected<void, error> mesh<Backend>::_triangulate() noexcept {
  if constexpr (Backend == cpu) return {};
  else {
    auto vertices = decltype(this->_gpu_vertices)::create(_vertices);
    if (!vertices) return vertices.error().relay();
    auto triangles = decltype(this->_gpu_triangles)::create(_triangles);
    if (!triangles) return triangles.error().relay();
    this->_gpu_vertices = std::move(*vertices);
    this->_gpu_triangles = std::move(*triangles);
    return {};
  }
}
} // namespace yw::geom
