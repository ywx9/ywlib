#define ywlib_debug
#include "ywlib/control.h"
#include "ywlib/ff-stl.h"
using namespace yw;

struct facet { // CADデータの要素
  nat4 id;
  array<fat4, 3> a, b, c;
  array<fat4, 3> ab, bc, ca;
  array<fat4, 3> sq, nisq;
  vector plane;
};

struct vertex { // CMTデータの要素
  vector p, n;
};

struct margin { // 取代
  float margin, perp;
  nat4 id, inside;
};

namespace {

constexpr auto app_name = L"test";

constexpr natt app_width = 960, app_height = 540;

auto key_configs = [] { key::escape.down = []() { yes(L"終了しますか？") ? main::terminate() : void(); };
                        return none{}; }();

button ui_button_cmt{};
button ui_button_cad{};
label ui_label_cmt{};
label ui_label_cad{};
radiobutton ui_radiobutton_flip{};
checkbox ui_checkbox_reverse{};
checkbox ui_checkbox_transparent{};
button ui_button_align{};
label ui_label_align{}; // 計算時間
label ui_label_rotation{};
label ui_label_rot_x{};
label ui_label_rot_y{};
label ui_label_rot_z{};
valuebox ui_valuebox_rot[3];
label ui_label_offset{};
label ui_label_off_x{};
label ui_label_off_y{};
label ui_label_off_z{};
valuebox ui_valuebox_off[3];
label ui_label_min_whole{};
label ui_label_min_over{};
label ui_label_max_over{};
button ui_button_result{};
textbox ui_textbox_result[7]; // Group 2

structured_buffer<facet> sb_facets{};
unordered_buffer<facet> ub_facets{};
structured_buffer<vertex> sb_vertices{};
unordered_buffer<vertex> ub_vertices{};
structured_buffer<margin> sb_margins{};
unordered_buffer<margin> ub_margins{};
constant_buffer<xmatrix> cb_world{};
constant_buffer<list<xmatrix, xmatrix>> cb_camera{};
constant_buffer<list<nat4, fat4, nat8>> cb_options{};

camera Camera;
}

unordered_buffer<facet> facets_from_stl(const ff::stl& Stl) {
  static constexpr stv1 hlsl = R"(
#pragma pack_matrix(row_major)
struct UB0 {
  uint id;           // 面の識別番号
  float3 a, b, c;    // 頂点座標
  float3 ab, bc, ca; // 辺ベクトル
  float3 sq, nisq;   // 辺の長さの二乗、辺の長さの二乗の負の逆数
  float4 plane;      // 標準化した平面パラメータ
};
RWStructuredBuffer<UB0> ub0 : register(u0);
struct SB0 {
  uint id;        // 面の識別番号
  float3 a, b, c; // 頂点座標
};
StructuredBuffer<SB0> sb0 : register(t0);
[numthreads(1024, 1, 1)] void csmain(uint i : SV_DispatchThreadID) {
  ub0[i].id = sb0[i].id;
  ub0[i].a = sb0[i].a; ub0[i].b = sb0[i].b; ub0[i].c = sb0[i].c;
  ub0[i].ab = sb0[i].b - sb0[i].a; ub0[i].bc = sb0[i].c - sb0[i].b; ub0[i].ca = sb0[i].a - sb0[i].c;
  ub0[i].sq = float3(dot(ub0[i].ab, ub0[i].ab), dot(ub0[i].bc, ub0[i].bc), dot(ub0[i].ca, ub0[i].ca));
  ub0[i].nisq = -1.0 / ub0[i].sq;
  float4 t = float4(cross(sb0[i].a, sb0[i].b), 0);
  t = float4(t.xyz + cross(sb0[i].b, sb0[i].c) + cross(sb0[i].c, sb0[i].a), dot(t.xyz, sb0[i].c));
  ub0[i].plane = t / length(t.xyz);
})";
  if (Stl.empty()) return {};
  static auto gp = gpgpu<typepack<facet>, typepack<list<nat4, array<fat4, 9>>>>(hlsl);
  unordered_buffer<facet> ub(Stl.size());
  array<list<nat4, array<fat4, 9>>> t(Stl.size());
  for (natt i{}; i < t.size(); ++i) t[i].first = Stl[i].attribute, memcpy(t[i].second.data(), Stl[i].vertex, 36);
  structured_buffer sb(t);
  gp(t.size(), {ub}, {sb}, {});
  return ub;
}

unordered_buffer<vertex> vertices_from_stl(const ff::stl& Stl) {
  static constexpr stv1 hlsl = R"(
#pragma pack_matrix(row_major)
struct UB0 {
  float4 v, n; // 頂点座標、法線ベクトル
};
RWStructuredBuffer<UB0> ub0 : register(u0);
struct SB0 {
  float3 a, b, c; // 頂点座標、法線ベクトル
};
StructuredBuffer<SB0> sb0 : register(t0);
[numthreads(1024, 1, 1)] void csmain(uint i : SV_DispatchThreadID) {
  const uint j = i * 3;
  float4 normal = float4(normalize(cross(sb0[i].b - sb0[i].a, sb0[i].c - sb0[i].a)), 0);
  ub0[j].v = float4(sb0[i].a, 1); ub0[j].n = normal;
  ub0[j + 1].v = float4(sb0[i].b, 1); ub0[j + 1].n = normal;
  ub0[j + 2].v = float4(sb0[i].c, 1); ub0[j + 2].n = normal;
})";
  ywlib_try_begin;
  if (Stl.empty()) return {};
  static auto gp = gpgpu<typepack<vertex>, typepack<array<fat4, 9>>>(hlsl);
  unordered_buffer<vertex> ub(Stl.size() * 3);
  array<array<fat4, 9>> t(Stl.size());
  for (natt i{}; i < t.size(); ++i) memcpy(t[i].data(), Stl[i].vertex, 36);
  structured_buffer sb(t);
  gp(t.size(), {ub}, {sb}, {});
  return ub;
  ywlib_try_end;
  return {};
}

void render_facets(const structured_buffer<facet>& Facets,
                   const constant_buffer<list<xmatrix, xmatrix>>& Camera,
                   const constant_buffer<list<nat4, fat4, nat8>>& Options, source Source = {}) {
  static constexpr stv1 hlsl = R"(
#pragma pack_matrix(row_major)
struct SB0 {
  uint id;           // 面の識別番号
  float3 a[3];       // 頂点座標
  float3 ab, bc, ca; // 辺ベクトル
  float3 sq, nisq;   // 辺の長さの二乗、辺の長さの二乗の負の逆数
  float4 plane;      // 標準化した平面パラメータ
};
StructuredBuffer<SB0> sb0 : register(t0);
struct GSIN {
  float4 p : POSITION; // 頂点座標
  float4 n : NORMAL;   // 法線ベクトル
};
void vsmain(uint i : SV_VertexID, out GSIN Out) {
  const SB0 t = sb0[i / 3];
  Out.p = float4(t.a[i % 3], 1);
  Out.n = float4(t.plane.xyz, 0);
}
cbuffer CB0 : register(b0) {
  matrix v;  // ビュー行列
  matrix vp; // ビュー射影行列
};
cbuffer CB1 : register(b1) {
  uint hide;   // 1ならx>0の面を描画しない, 2ならx<0の面を描画しない
  uint pad[3];
};
struct PSIN {
  float4 p : SV_POSITION; // ピクセル座標
  float4 n : NORMAL;      // 法線ベクトル
  float4 c : COLOR;       // 色
};
[maxvertexcount(3)] void gsmain(triangle GSIN In[3], inout TriangleStream<PSIN> Str) {
  if (hide == 1 && In[0].p.x >= 0 && In[1].p.x >= 0 && In[2].p.x >= 0) return;
  if (hide == 2 && In[0].p.x <= 0 && In[1].p.x <= 0 && In[2].p.x <= 0) return;
  PSIN Out;
  for (uint i = 0; i < 3; ++i) {
    Out.p = mul(vp, In[i].p);
    Out.n = mul(v, In[i].n);
    Out.c = float4(0.8f, 0, 0.8f, 1);
    Str.Append(Out);
  } Str.RestartStrip();
}
float4 psmain(PSIN In) : SV_TARGET {
  return float4(In.c.xyz * (0.3 - 0.6 * In.n.z), In.c.w);
})";
  static auto rd = renderer<typepack<structured_buffer<facet>>, typepack<>,
                            typepack<>, typepack<constant_buffer<list<xmatrix, xmatrix>>, constant_buffer<list<nat4, nat4, nat8>>>>(hlsl);
  try {
    rd(Facets.count * 3, {Facets}, {}, {}, {Camera, Options});
  } catch (const std::exception& E) { throw except(E, mv(Source)); }
}

void render_vertices(const structured_buffer<vertex>& Vertices, const structured_buffer<margin>& Margins,
                     const constant_buffer<xmatrix>& World, const constant_buffer<list<xmatrix, xmatrix>>& Camera,
                     const constant_buffer<list<nat4, fat4, nat8>>& Options) {
  static constexpr stv1 hlsl = R"(
#pragma pack_matrix(row_major)
struct SB0 {
  float4 p; // 頂点座標
  float4 n; // 法線ベクトル
};
StructuredBuffer<SB0> sb0 : register(t0);
struct SB1 {
  float margin; // 取代
  float perp;   // 垂直方向の取代
  uint id;      // 直近の面の識別番号
  uint inside;  // 1なら面の直上にある
};
StructuredBuffer<SB1> sb1 : register(t1);
cbuffer CB0 : register(b0) {
  matrix w; // ワールド行列
};
struct GSIN {
  float4 p : POSITION; // 頂点座標
  float4 n : NORMAL;   // 法線ベクトル
  float4 c : COLOR;    // 色
};
void vsmain(uint i : SV_VertexID, out GSIN Out) {
  const SB0 t = sb0[i];
  const float m = sb1[i].margin;
  Out.p = mul(w, t.p);
  Out.n = mul(w, t.n);
  if (m > 60) Out.c = float4(0, 0, 0.6, 1);
  else if (m > 45) Out.c = float4(0, 0.5, 1, 1);
  else if (m > 24) Out.c = float4(0, 0.8, 0.5, 1);
  else if (m > 10) Out.c = float4(0.8, 0.8, 0, 1);
  else if (m > 5) Out.c = float4(0.8, 0.5, 0, 1);
  else Out.c = float4(0.8, 0, 0, 1);
}
cbuffer CB0 : register(b0) {
  matrix v;  // ビュー行列
  matrix vp; // ビュー射影行列
};
cbuffer CB1 : register(b1) {
  uint hide;   // 1ならx>0の面を描画しない, 2ならx<0の面を描画しない
  float alpha; // 透明度
  uint pad[2];
};
struct PSIN {
  float4 p : SV_POSITION; // ピクセル座標
  float4 n : NORMAL;      // 法線ベクトル
  float4 c : COLOR;       // 色
};
[maxvertexcount(3)] void gsmain(triangle GSIN In[3], inout TriangleStream<PSIN> Str) {
  // if (hide == 1 && In[0].p.x >= 0 && In[1].p.x >= 0 && In[2].p.x >= 0) return;
  // if (hide == 2 && In[0].p.x <= 0 && In[1].p.x <= 0 && In[2].p.x <= 0) return;
  PSIN Out;
  for (uint i = 0; i < 3; ++i) {
    Out.p = mul(vp, In[i].p);
    Out.n = mul(v, In[i].n);
    Out.c = float4(In[i].c.xyz, 1);
    Str.Append(Out);
  } Str.RestartStrip();
}
float4 psmain(PSIN In) : SV_TARGET {
  // return float4(In.c.xyz * (0.3 - 0.6 * In.n.z), 1);
  return float4(1, 0, 0, 1);
})";
  static auto rd = renderer<typepack<structured_buffer<vertex>, structured_buffer<margin>>, typepack<>,
                            typepack<>, typepack<constant_buffer<xmatrix>, constant_buffer<list<xmatrix, xmatrix>>, constant_buffer<list<nat4, fat4, nat8>>>>(hlsl);
  ywlib_try_begin;
  rd(Vertices.count, {Vertices, Margins}, {}, {}, {World, Camera, Options});
  ywlib_try_end;
}

void calc_margin(const structured_buffer<facet>& Facets, const structured_buffer<vertex>& Vertices,
                 const constant_buffer<xmatrix>& World, unordered_buffer<margin>& Margins) {
  static constexpr stv1 hlsl = R"(
#pragma pack_matrix(row_major)
struct SB0 {
  uint id;           // 面の識別番号
  float3 a, b, c;    // 頂点座標
  float3 ab, bc, ca; // 辺ベクトル
  float3 sq, nisq;   // 辺の長さの二乗、辺の長さの二乗の負の逆数
  float4 plane;      // 標準化した平面パラメータ
};
StructuredBuffer<SB0> sb0 : register(t0);
struct Sb1 {
  float4 p; // 頂点座標
  float4 n; // 法線ベクトル
};
StructuredBuffer<Sb1> sb1 : register(t1);
struct UB0 {
  float margin; // 取代
  float perp;   // 垂直方向の取代
  uint id;      // 直近の面の識別番号
  uint inside;  // 1なら面の直上にある
};
RWStructuredBuffer<UB0> ub0 : register(u0);
cbuffer CB0 : register(b0) {
  matrix w; // ワールド行列
};
cbuffer CB1 : register(b1) {
  uint m; // 面の数
  uint n; // 頂点の数
  uint pad[2];
};
[numthreads(1024, 1, 1)] void csmain(uint i : SV_DispatchThreadID) {
  if (i >= n) return;
  const float3 p = mul(w, sb1[i].p).xyz;  // ワールド座標系での頂点Vの座標
  const float3 nn = mul(w, sb1[i].n).xyz; // ワールド座標系での頂点Vの法線ベクトル
  float3 a, b, c;
  float2 t;
  ub0[i].margin = 1e10; // 計算途中では取代の二乗として扱う
  for (uint j = 0; j < m; ++j) {
    a = p - sb0[j].a; // 面の頂点Aから頂点Vへのベクトル
    b = p - sb0[j].b; // 面の頂点Bから頂点Vへのベクトル
    c = p - sb0[j].c; // 面の頂点Cから頂点Vへのベクトル
    t.y = dot(p, sb0[j].plane.xyz) - sb0[j].plane.w;     // 頂点Vと面の距離
    if (ub0[i].margin < t.y * t.y) continue;             // 面と点の距離は必ず面と点の法線距離より長いから
    if (dot(cross(a, b), sb0[j].plane.xyz) < 0) {        // 点Vが辺ABの外側にいる
      t.x = dot(a, sb0[j].ab);                           // AVとABの内積
      if (t.x < 0) t.x = dot(a, a);                      // 点Vにとって点Aが一番近い
      else if (sb0[j].sq.x < t.x) t.x = dot(b, b);       // 点Vにとって点Bが一番近い
      else t.x = dot(a, a) + t.x * t.x * sb0[j].nisq.x;  // 点Vにとって辺AB上の点が一番近い
      if (t.x < ub0[i].margin) ub0[i].margin = t.x, ub0[i].perp = t.y, ub0[i].id = sb0[j].id, ub0[i].inside = 0;
    } else if (dot(cross(b, c), sb0[j].plane.xyz) < 0) { // 点Vが辺BCの外側にいる
      t.x = dot(b, sb0[j].bc);                           // BVとBCの内積
      if (t.x < 0) t.x = dot(b, b);                      // 点Vにとって点Bが一番近い
      else if (sb0[j].sq.y < t.x) t.x = dot(c, c);       // 点Vにとって点Cが一番近い
      else t.x = dot(b, b) + t.x * t.x * sb0[j].nisq.y;  // 点Vにとって辺BC上の点が一番近い
      if (t.x < ub0[i].margin) ub0[i].margin = t.x, ub0[i].perp = t.y, ub0[i].id = sb0[j].id, ub0[i].inside = 0;
    } else if (dot(cross(c, a), sb0[j].plane.xyz) < 0) { // 点Vが辺CAの外側にいる
      t.x = dot(c, sb0[j].ca);                           // CVとCAの内積
      if (t.x < 0) t.x = dot(c, c);                      // 点Vにとって点Cが一番近い
      else if (sb0[j].sq.z < t.x) t.x = dot(a, a);       // 点Vにとって点Aが一番近い
      else t.x = dot(c, c) + t.x * t.x * sb0[j].nisq.z;  // 点Vにとって辺CA上の点が一番近い
      if (t.x < ub0[i].margin) ub0[i].margin = t.x, ub0[i].perp = t.y, ub0[i].id = sb0[j].id, ub0[i].inside = 0;
    } else ub0[i].margin = t.y * t.y, ub0[i].perp = t.y, ub0[i].id = sb0[j].id, ub0[i].inside = 1;
  }
  ub0[i].margin = ub0[i].inside ? ub0[i].perp : sqrt(ub0[i].margin);
})";
  static auto gp = gpgpu<typepack<margin>, typepack<facet, vertex>, typepack<xmatrix, list<nat4, nat4, nat8>>>(hlsl);
  static auto cb = constant_buffer(list<nat4, nat4, nat8>{});
  ywlib_try_begin;
  cb.from(list<>::asref(Facets.count, Vertices.count, 0));
  gp(Vertices.count, {Margins}, {Facets, Vertices}, {World, cb});
  ywlib_try_end;
}

void update_world() {
  static constexpr auto f = [](xmatrix& m) {
    xvworld(vector(ui_valuebox_off[0].value<fat4>(), ui_valuebox_off[1].value<fat4>(), ui_valuebox_off[2].value<fat4>()),
            vector(ui_valuebox_rot[0].value<fat4>(), ui_valuebox_rot[1].value<fat4>(), ui_valuebox_rot[2].value<fat4>()), m);
  };
  cb_world.from(f);
}

void render_all() {
  ywlib_try_begin;
  Camera.begin_render(color::white);
  if (sb_facets) render_facets(sb_facets, cb_camera, cb_options);
  if (sb_vertices) render_vertices(sb_vertices, sb_margins, cb_world, cb_camera, cb_options);
  Camera.end_render();
  main::begin_draw(color::yw);
  Camera.draw({5, 85, 805, 535});
  main::end_draw();
  main::update(), main::update();
  ywlib_try_end;
}

void ywmain() {
  main::rename(app_name);
  main::resize(app_width, app_height);
  key::escape.down = []() { yes(L"終了しますか？") ? main::terminate() : void(); };

  Camera = camera(1920, 1080, 8);
  Camera.orthographic = true;
  Camera.factor = 0.5f;
  Camera.offset.z = -10000;
  Camera.rotation.z = -fat4(pi / 2);
  Camera.update();
  Camera.begin_render(color::white);
  Camera.end_render();

  static_assert(vassignable<list<xmatrix, xmatrix>&, decltype(list<>::asref(Camera.view, Camera.view_proj))>);
  static_assert(constructible<constant_buffer<list<xmatrix, xmatrix>>, list<xmatrix&, xmatrix&>>);
  cb_camera = constant_buffer<list<xmatrix, xmatrix>>(list<>::asref(Camera.view, Camera.view_proj));
  cb_world = decltype(cb_world)(xv_identity);
  cb_options = decltype(cb_options)(list<nat4, fat4, nat8>{0, 0.8f, 0});


  ui_button_cmt = button(0, {5, 5, 45, 25}, L"CMT");
  ui_button_cmt.input = [](const button& This) {
    ywlib_try_begin;
    auto file = open_file(std::format(LR"(C:\Users\{}\Desktop\表面ポイント)", main::username));
    if (file.empty()) return;
    auto stl = ff::stl(file);
    if (stl.empty()) return (void)ok(L"正しくないSTLファイルです。");
    ui_label_cmt.text(file.filename().wstring());
    ub_vertices = vertices_from_stl(stl);
    sb_vertices = decltype(sb_vertices)(ub_vertices);
    ub_margins = decltype(ub_margins)(ub_vertices.count);
    sb_margins = decltype(sb_margins)(ub_vertices.count);
    for (auto& e : ui_valuebox_off) e.text(L"0");
    for (auto& e : ui_valuebox_rot) e.text(L"0");
    update_world();
    if (sb_facets) {
      calc_margin(sb_facets, sb_vertices, cb_world, ub_margins);
      sb_margins.from(ub_margins);
    }
    render_all();
    ywlib_try_end;
  };


  ui_button_cad = button(0, {5, 30, 45, 50}, L"CAD");
  ui_button_cad.input = [](const button& This) {
    ywlib_try_begin;
    auto file = open_file(LR"(\\Z1255101\apps\CMT\3Dモデル\組立型クランク軸\スロー\ACMA)");
    if (file.empty()) return;
    auto stl = ff::stl(file);
    if (stl.empty()) return (void)ok(L"正しくないSTLファイルです。");
    ui_label_cad.text(file.filename().wstring());
    ub_facets = facets_from_stl(stl);
    sb_facets = decltype(sb_facets)(ub_facets);
    if (sb_vertices) {
      calc_margin(sb_facets, sb_vertices, cb_world, ub_margins);
      sb_margins.from(ub_margins);
    }
    render_all();
    ywlib_try_end;
  };

  ui_label_cmt = label(0, {50, 5, 300, 25}, L"");
  ui_label_cad = label(0, {50, 30, 300, 50}, L"");

  ui_valuebox_rot[0] = valuebox(0, {855, 110, 955, 130}, L"0");
  ui_valuebox_rot[1] = valuebox(0, {855, 135, 955, 155}, L"0");
  ui_valuebox_rot[2] = valuebox(0, {855, 160, 955, 180}, L"0");

  ui_valuebox_off[0] = valuebox(0, {855, 215, 955, 235}, L"0");
  ui_valuebox_off[1] = valuebox(0, {855, 240, 955, 260}, L"0");
  ui_valuebox_off[2] = valuebox(0, {855, 265, 955, 285}, L"0");

  control::show(0);
  render_all();

  while (main::update) {
    // auto a = cb_options.to_cpu();
    // std::cout << std::format("{},{},{}\n", a[0].first, a[0].second, a[0].third);
  }

  main::terminate();
}
