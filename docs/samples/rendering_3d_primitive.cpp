#include <ywxlib>
using namespace yw;

const char* const hlsl_textured_geometry = yw_hlsl_row_major_pragma yw_hlsl_vertex
  R"(
cbuffer CameraConstants : register(b0) {
  float4x4 view;
  float4x4 projection;
  float4x4 view_projection;
  float4x4 inverse_view;
  float4x4 inverse_projection;
  float4x4 inverse_view_projection;
  float4 camera_pos;
  float4 camera_dir;
  float4 viewport_size;
};
cbuffer WorldMatrix : register(b1) {
  float4x4 world;
};
StructuredBuffer<Vertex> vertices : register(t0);
StructuredBuffer<uint3> triangles : register(t1);
Texture2D diffuse_texture : register(t0);
SamplerState diffuse_sampler : register(s0);

struct VSOUT {
  float4 position : SV_POSITION;
  float2 uv : TEXCOORD0;
};

VSOUT vsmain(uint vertex_id : SV_VertexID) {
  Vertex v = vertices[triangles[vertex_id / 3][vertex_id % 3]];
  VSOUT o;
  o.position = mul(view_projection, mul(world, v.position));
  o.uv = v.uv;
  return o;
}

float4 psmain(VSOUT input) : SV_TARGET {
  return diffuse_texture.Sample(diffuse_sampler, input.uv);
}
)";

int main() {
  auto cam = camera({1024, 768});
  double zoom = 230.0;
  if (auto res = cam.orthographic(float(zoom)); !res) res.error().go_off();
  if (auto res = cam.position({0.0f, 0.0f, -6.0f, 1.0f}); !res) res.error().go_off();
  if (auto res = cam.look_at({0.0f, 0.0f, 0.5f, 1.0f}); !res) res.error().go_off();

  auto target = bitmap_texture(cam.size());
  auto depth = depth_texture(cam.size());

  auto prism =
  geom::prism<geom::circle, gpu>();
  // geom::pyramid<geom::circle, gpu>();
  // geom::sphere<gpu>();
  // prism.remeshing_option().subdivisions = 0;
  if (auto tex = geom::make_texture_template(prism, {1024, 1024}); !tex) tex.error().go_off();
  else save_bitmap_png(*tex, L"test_texture.png");
  auto texture = bitmap_texture(L"test_texture.png");

  double pitch = -yw::pi / 5.0;
  double yaw = yw::pi / 6.0;
  const auto update_prism = [&] {
    prism.rotation({pitch, yaw, 0.0});
    if (auto res = prism.update_gpu(); !res) res.error().go_off();
  };
  prism.scale({1.0, 1.0, 1.8});
  update_prism();

  auto vs = vertex_shader(hlsl_textured_geometry);
  auto ps = pixel_shader(hlsl_textured_geometry);
  auto w = window({.title = L"circle prism texture", .size = int2(cam.size())});

  w.drag_event([&](drag_event e) {
    if (e.key != keys::lbutton) return false;
    yaw += double(e.delta.x) * 0.01;
    pitch += double(e.delta.y) * 0.01;
    constexpr double limit = yw::pi * 0.49;
    pitch = yw::clamp(pitch, -limit, limit);
    update_prism();
    return true;
  });

  w.wheel_event([&](wheel_event e) {
    if (e.horizontal) return false;
    zoom *= e.delta > 0 ? 1.1 : 1.0 / 1.1;
    zoom = yw::clamp(zoom, 60.0, 600.0);
    if (auto res = cam.orthographic(float(zoom)); !res) res.error().go_off();
    return true;
  });

  while (mainloop) {
    if (auto res = target.clear(colors::black); !res) res.error().go_off();
    if (auto res = depth.clear(0.0f); !res) res.error().go_off();

    if (auto r = cam.begin_render(target, depth)) {
      if (auto res = r->set_vs(vs, cam.constant_buffer(), prism.cb_world(), prism.gpu_vertices(), prism.gpu_triangles()); !res)
        res.error().go_off();
      if (auto res = r->set_ps(ps, texture); !res) res.error().go_off();
      if (auto res = r->render_triangles(uint32_t(prism.gpu_triangles().size())); !res) res.error().go_off();
    } else r.error().go_off();

    if (auto d = w.begin_draw()) {
      if (auto res = draw_bitmap({}, target); !res) res.error().go_off();
    } else d.error().go_off();
  }
}
