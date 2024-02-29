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

struct maxmin {
  array<vector, 26> maxs, mins;
  vector minimum, minimum_n;
  nat4 minimum_i;
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
radiobutton ui_radiobutton_hide{};
checkbox ui_checkbox_reverse{};
checkbox ui_checkbox_alpha{};
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
label ui_label_timer{};
label ui_label_margins[4];
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

ff::stl stl_facets{};

camera Camera{};
maxmin MaxMin{};

bool ResultMode = false;
intt AlignMode = -1;
}

unordered_buffer<facet> facets_from_stl(const ff::stl& Stl, bool Reverse) {
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
  ywlib_try_begin;
  if (Stl.empty()) return {};
  static constexpr auto a = array{
    array<nat4, 25>{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24},
    array<nat4, 25>{12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 24}};
  static auto gp = gpgpu<typepack<facet>, typepack<list<nat4, array<fat4, 9>>>>(hlsl);
  unordered_buffer<facet> ub(Stl.size());
  array<list<nat4, array<fat4, 9>>> t(Stl.size());
  for (natt i{}; i < t.size(); ++i)
    t[i].first = a[Reverse][Stl[i].attribute],
    memcpy(t[i].second.data(), Stl[i].vertex, 36);
  structured_buffer sb(t);
  gp(t.size(), {ub}, {sb}, {});
  return ub;
  ywlib_try_end;
  return {};
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
  const float m = sb1[i].margin;
  Out.p = mul(w, sb0[i].p);
  Out.n = mul(w, sb0[i].n);
  if (m > 60) Out.c = float4(0, 0, 0.6, 1);
  else if (m > 45) Out.c = float4(0, 0.5, 1, 1);
  else if (m > 24) Out.c = float4(0.2, 0.8, 0.3, 1);
  else if (m > 10) Out.c = float4(0.8, 0.8, 0, 1);
  else if (m > 5) Out.c = float4(0.8, 0.4, 0, 1);
  else Out.c = float4(0.8, 0, 0, 1);
}
struct PSIN {
  float4 p : SV_POSITION; // ピクセル座標
  float4 n : NORMAL;      // 法線ベクトル
  float4 c : COLOR;       // 色
};
cbuffer CB0 : register(b0) {
  matrix v;  // ビュー行列
  matrix vp; // ビュー射影行列
};
cbuffer CB1 : register(b1) {
  uint hide;   // 1ならx>0の面を描画しない, 2ならx<0の面を描画しない
  float alpha; // 透明度
  uint pad[2];
};
[maxvertexcount(3)] void gsmain(triangle GSIN In[3], inout TriangleStream<PSIN> Str) {
  if (hide == 1 && In[0].p.x >= 0 && In[1].p.x >= 0 && In[2].p.x >= 0) return;
  if (hide == 2 && In[0].p.x <= 0 && In[1].p.x <= 0 && In[2].p.x <= 0) return;
  PSIN Out;
  for (uint i = 0; i < 3; ++i) {
    Out.p = mul(vp, In[i].p);
    Out.n = mul(v, In[i].n);
    Out.c = float4(In[i].c.xyz, alpha);
    Str.Append(Out);
  } Str.RestartStrip();
}
float4 psmain(PSIN In) : SV_TARGET {
  return float4(In.c.xyz * (0.3 - 0.6 * In.n.z), In.c.w);
})";
  ywlib_try_begin;
  static auto rd = renderer<typepack<structured_buffer<vertex>, structured_buffer<margin>>,
                            typepack<constant_buffer<xmatrix>>,
                            typepack<>,
                            typepack<constant_buffer<list<xmatrix, xmatrix>>,
                                     constant_buffer<list<nat4, fat4, nat8>>>>(hlsl);
  rd(Vertices.count, {Vertices, Margins}, {World}, {}, {Camera, Options});
  ywlib_try_end;
}

void calc_maxmin(const structured_buffer<vertex>& Vertices, const unordered_buffer<margin>& Margins, maxmin& MaxMin) {
  static constexpr stv1 hlsl = R"(
#pragma pack_matrix(row_major)
struct SB0 {
  float4 p; // 頂点座標
  float4 n; // 法線ベクトル
};
StructuredBuffer<SB0> sb0 : register(t0);
struct UB0 {
  float margin; // 取代
  float perp;   // 垂直方向の取代
  uint id;      // 直近の面の識別番号
  uint inside;  // 1なら面の直上にある
};
RWStructuredBuffer<UB0> ub0 : register(u0);
struct UB1 {
  float4 maxs[26]; // 最大値
  float4 mins[26]; // 最小値
  float4 minimum;  // 最小値
  float4 minimum_n; // 最小値の法線ベクトル
  uint minimum_i;   // 最小値の面の識別番号
};
RWStructuredBuffer<UB1> ub1 : register(u1);
cbuffer CB0 : register(b0) {
  uint m; // 頂点の数
  uint n; // スレッド内の処理数
  uint pad[2];
};
[numthreads(1024, 1, 1)] void csmain(uint i : SV_DispatchThreadID) {
  for (uint k = 0; k < 25; ++k) ub1[i].maxs[k].w = -1e10, ub1[i].mins[k].w = 1e10;
  ub1[i].maxs[25] = float4(-1e10, -1e10, -1e10, 1), ub1[i].mins[25] = float4(1e10, 1e10, 1e10, 1);
  ub1[i].minimum.w = 1e10;
  const uint j0 = i * n, jj = j0 + n;
  for (uint j = j0; j < jj && j < m; ++j) {
    const UB0 t = ub0[j]; const SB0 s = sb0[j];
    if (ub1[i].maxs[t.id].w < t.margin && t.inside) ub1[i].maxs[t.id] = float4(s.p.xyz, t.margin);
    if (ub1[i].mins[t.id].w > t.margin && t.inside) ub1[i].mins[t.id] = float4(s.p.xyz, t.margin);
    if (ub1[i].minimum.w > t.margin) ub1[i].minimum = float4(s.p.xyz, t.margin),
                                     ub1[i].minimum_n = s.n, ub1[i].minimum_i = t.id;
    ub1[i].maxs[25] = max(ub1[i].maxs[25], s.p); ub1[i].mins[25] = min(ub1[i].mins[25], s.p);
  }
})";
  ywlib_try_begin;
  static auto gp = gpgpu<typepack<margin, maxmin>, typepack<vertex>, typepack<list<nat4, nat4, nat8>>>(hlsl);
  static auto cb = constant_buffer(list<nat4, nat4, nat8>{}); // 頂点の数、スレッド内の処理数、パディング
  static auto ub = unordered_buffer<maxmin>(1024);
  static auto st = staging_buffer<maxmin>(1024);
  cb.from(list<>::asref(Vertices.count, (Vertices.count - 1) / 1024 + 1, 0_n8));
  gp(ub.count, {Margins, ub}, {Vertices}, {cb});
  auto t = ub.to_cpu(st);
  for (natt i{512}; i != 0; i /= 2) {
    for (natt j{}; j < i; ++j) {
      for (natt k{}; k < 25; ++k) {
        if (t[j].maxs[k].w < t[j + i].maxs[k].w) t[j].maxs[k] = t[j + i].maxs[k];
        if (t[j].mins[k].w > t[j + i].mins[k].w) t[j].mins[k] = t[j + i].mins[k];
      }
      if (t[j].minimum.w > t[j + i].minimum.w)
        t[j].minimum = t[j + i].minimum, t[j].minimum_n = t[j + i].minimum_n, t[j].minimum_i = t[j + i].minimum_i;
      t[j].maxs[25] = xvmax(t[j].maxs[25], t[j + i].maxs[25]), t[j].mins[25] = xvmin(t[j].mins[25], t[j + i].mins[25]);
    }
  }
  MaxMin = mv(t[0]);
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
  ywlib_try_begin;
  static auto gp = gpgpu<typepack<margin>, typepack<facet, vertex>, typepack<xmatrix, list<nat4, nat4, nat8>>>(hlsl);
  static auto cb = constant_buffer(list<nat4, nat4, nat8>{});
  cb.from(list<>::asref(Facets.count, Vertices.count, 0));
  gp(Vertices.count, {Margins}, {Facets, Vertices}, {World, cb});
  ywlib_try_end;
}

void update_world() {
  static constexpr auto f = [](xmatrix& m) {
    auto t = xv(ui_valuebox_off[0].value<fat4>(), ui_valuebox_off[1].value<fat4>(), ui_valuebox_off[2].value<fat4>(), 0.f);
    auto r = xv(ui_valuebox_rot[0].value<fat4>(), ui_valuebox_rot[1].value<fat4>(), ui_valuebox_rot[2].value<fat4>(), 0.f);
    xvworld(t, xvneg(xvradian(r)), m); };
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

void align() {
  if (AlignMode < 0 || !(sb_facets && sb_vertices)) return;
  auto original_off = xv(ui_valuebox_off[0].value<fat4>(), ui_valuebox_off[1].value<fat4>(), ui_valuebox_off[2].value<fat4>(), 0.f);
  auto original_rot = xv(ui_valuebox_rot[0].value<fat4>(), ui_valuebox_rot[1].value<fat4>(), ui_valuebox_rot[2].value<fat4>(), 0.f);
  original_rot = xvneg(xvradian(original_rot));

  auto checker = []<natt I>(constant<I>, const maxmin& Now, const maxmin& New) {
    if constexpr (I == 0) // 基準面厚み最小化 (yz軸回転)
      return (Now.maxs[25].x - Now.mins[25].x) > (New.maxs[25].x - New.mins[25].x);
    else if constexpr (I == 1) // 基準面最大取代最小化 (x軸移動)
      return max(Now.maxs[0].w, Now.maxs[12].w) > max(New.maxs[0].w, New.maxs[12].w);
    else if constexpr (I == 2) // 側面最大取代最小化 (x軸回転)
      return max(Now.maxs[6].w, Now.maxs[7], Now.maxs[8], Now.maxs[9], Now.maxs[10], Now.maxs[11]) >
             max(New.maxs[6].w, New.maxs[7], New.maxs[8], New.maxs[9], New.maxs[10], New.maxs[11]);
    else if constexpr (I == 3) // 側面最小取代最大化 (x軸回転、z軸移動)
      return min(Now.mins[6].w, Now.mins[7], Now.mins[8], Now.mins[9], Now.mins[10], Now.mins[11]) <
             min(New.mins[6].w, New.mins[7], New.mins[8], New.mins[9], New.mins[10], New.mins[11]);
    else if constexpr (I == 4) // 前後最小取代最大化 (y軸移動)
      return min(Now.mins[3].w, Now.mins[4].w, Now.mins[5].w, Now.mins[8].w, Now.mins[11].w,
                 Now.mins[15].w, Now.mins[16].w, Now.mins[17].w, Now.mins[20].w, Now.mins[23].w) <
             min(New.mins[3].w, New.mins[4].w, New.mins[5].w, New.mins[8].w, New.mins[11].w,
                 New.mins[15].w, New.mins[16].w, New.mins[17].w, New.mins[20].w, New.mins[23].w);
    else if constexpr (I == 5) // 基準面+内股最小取代最大化 (yz軸回転、x軸移動)
      return min(Now.mins[0].w, Now.mins[2].w, Now.mins[12].w, Now.mins[14].w) <
             min(New.mins[0].w, New.mins[2].w, New.mins[12].w, New.mins[14].w);
    else if constexpr (I == 6) // 面上最小取代最大化 (x軸回転、XYZ軸移動)
      return apply(min, projector(Now.mins, vector::w)) < apply(min, projector(New.mins, vector::w));
  };

  // 続き
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
  cb_options = decltype(cb_options)(list<nat4, fat4, nat8>{0, 1.f, 0});

  ui_button_cmt = button(0, {5, 5, 45, 25}, L"CMT");
  ui_button_cmt.input = [](const button& This) {
    ywlib_try_begin;
    if (AlignMode >= 0) AlignMode = -1, ok(L"自動芯合わせを中断しました。");
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
      calc_maxmin(sb_vertices, ub_margins, MaxMin);
    }
    render_all();
    ywlib_try_end;
  };

  ui_button_cad = button(0, {5, 30, 45, 50}, L"CAD");
  ui_button_cad.input = [](const button& This) {
    ywlib_try_begin;
    if (AlignMode >= 0) AlignMode = -1, ok(L"自動芯合わせを中断しました。");
    auto file = open_file(LR"(\\Z1255101\apps\CMT\3Dモデル\組立型クランク軸\スロー\ACMA)");
    if (file.empty()) return;
    stl_facets = ff::stl(file);
    if (stl_facets.empty()) return (void)ok(L"正しくないSTLファイルです。");
    ui_label_cad.text(file.filename().wstring());
    ub_facets = facets_from_stl(stl_facets, ui_checkbox_reverse.state);
    sb_facets = decltype(sb_facets)(ub_facets);
    if (sb_vertices) {
      calc_margin(sb_facets, sb_vertices, cb_world, ub_margins);
      sb_margins.from(ub_margins);
      calc_maxmin(sb_vertices, ub_margins, MaxMin);
    }
    render_all();
    ywlib_try_end;
  };

  ui_label_cmt = label(0, {50, 5, 300, 25}, L"");
  ui_label_cad = label(0, {50, 30, 300, 50}, L"");

  ui_radiobutton_hide = radiobutton(0, {305, 5, 405, 75}, L"", list{L"全表示", L"＋Ｘ側を非表示", L"－Ｘ側を非表示"});
  ui_radiobutton_hide.input = [](const radiobutton& This) {
    ywlib_try_begin;
    cb_options.from(list<>::asref(This.state, ui_checkbox_alpha.state, 0));
    render_all();
    ywlib_try_end;
  };

  ui_checkbox_reverse = checkbox(0, {410, 5, 510, 25}, L"＋ＸがBOT側");
  ui_checkbox_reverse.input = [](const checkbox& This) {
    ywlib_try_begin;
    if (AlignMode >= 0) AlignMode = -1, ok(L"自動芯合わせを中断しました。");
    if (stl_facets.empty()) return;
    ub_facets = facets_from_stl(stl_facets, This.state);
    sb_facets.from(ub_facets);
    calc_margin(sb_facets, sb_vertices, cb_world, ub_margins);
    sb_margins.from(ub_margins);
    calc_maxmin(sb_vertices, ub_margins, MaxMin);
    render_all();
    ywlib_try_end;
  };

  ui_checkbox_alpha = checkbox(0, {410, 30, 510, 50}, L"半透明");
  ui_checkbox_alpha.input = [](const checkbox& This) {
    ywlib_try_begin;
    cb_options.from(list<>::asref(ui_radiobutton_hide.state, This.state ? 0.7f : 1.0f, 0));
    render_all();
    ywlib_try_end;
  };

  ui_button_align = button(0, {820, 20, 945, 75}, L"自動芯合わせ", WS_BORDER);
  ui_button_align.input = [](const button& This) { AlignMode = 0; };

  ui_label_rotation = label(0, {810, 85, 955, 105}, L"回転 [deg]", SS_CENTER | SS_CENTERIMAGE);
  ui_label_rot_x = label(0, {810, 110, 850, 130}, L"X", SS_CENTER | SS_CENTERIMAGE);
  ui_label_rot_y = label(0, {810, 135, 850, 155}, L"Y", SS_CENTER | SS_CENTERIMAGE);
  ui_label_rot_z = label(0, {810, 160, 850, 180}, L"Z", SS_CENTER | SS_CENTERIMAGE);

  ui_valuebox_rot[0] = valuebox(0, {855, 110, 955, 130}, L"0");
  ui_valuebox_rot[1] = valuebox(0, {855, 135, 955, 155}, L"0");
  ui_valuebox_rot[2] = valuebox(0, {855, 160, 955, 180}, L"0");

  ui_valuebox_rot[0].input = [](const valuebox& This) {
    update_world();
    if (sb_facets && sb_vertices) {
      calc_margin(sb_facets, sb_vertices, cb_world, ub_margins);
      sb_margins.from(ub_margins);
      calc_maxmin(sb_vertices, ub_margins, MaxMin);
    }
    render_all(); };
  ui_valuebox_rot[1].input = ui_valuebox_rot[0].input;
  ui_valuebox_rot[2].input = ui_valuebox_rot[0].input;
  ui_valuebox_rot[0].killfocus = ui_valuebox_rot[0].input;
  ui_valuebox_rot[1].killfocus = ui_valuebox_rot[0].input;
  ui_valuebox_rot[2].killfocus = ui_valuebox_rot[0].input;
  ui_valuebox_rot[0].intofocus = [](const valuebox& This) {
    if (AlignMode >= 0) AlignMode = -1, ok(L"自動芯合わせを中断しました。");
  };
  ui_valuebox_rot[1].intofocus = ui_valuebox_rot[0].intofocus;
  ui_valuebox_rot[2].intofocus = ui_valuebox_rot[0].intofocus;

  ui_label_offset = label(0, {810, 190, 955, 210}, L"移動 [mm]", SS_CENTER | SS_CENTERIMAGE);
  ui_label_off_x = label(0, {810, 215, 850, 235}, L"X", SS_CENTER | SS_CENTERIMAGE);
  ui_label_off_y = label(0, {810, 240, 850, 260}, L"Y", SS_CENTER | SS_CENTERIMAGE);
  ui_label_off_z = label(0, {810, 265, 850, 285}, L"Z", SS_CENTER | SS_CENTERIMAGE);

  ui_valuebox_off[0] = valuebox(0, {855, 215, 955, 235}, L"0");
  ui_valuebox_off[1] = valuebox(0, {855, 240, 955, 260}, L"0");
  ui_valuebox_off[2] = valuebox(0, {855, 265, 955, 285}, L"0");

  ui_valuebox_off[0].input = ui_valuebox_rot[0].input;
  ui_valuebox_off[1].input = ui_valuebox_rot[0].input;
  ui_valuebox_off[2].input = ui_valuebox_rot[0].input;
  ui_valuebox_off[0].killfocus = ui_valuebox_rot[0].input;
  ui_valuebox_off[1].killfocus = ui_valuebox_rot[0].input;
  ui_valuebox_off[2].killfocus = ui_valuebox_rot[0].input;
  ui_valuebox_off[0].intofocus = ui_valuebox_rot[0].intofocus;
  ui_valuebox_off[1].intofocus = ui_valuebox_rot[0].intofocus;
  ui_valuebox_off[2].intofocus = ui_valuebox_rot[0].intofocus;

  ui_label_min_whole = label(0, {810, 295, 890, 315}, L"全体最小取代", SS_CENTER | SS_CENTERIMAGE);
  ui_label_min_over = label(0, {810, 320, 890, 340}, L"面上最小取代", SS_CENTER | SS_CENTERIMAGE);
  ui_label_max_over = label(0, {810, 345, 890, 365}, L"面上最大取代", SS_CENTER | SS_CENTERIMAGE);
  ui_label_timer = label(0, {810, 370, 890, 390}, L"計算時間", SS_CENTER | SS_CENTERIMAGE);

  ui_label_margins[0] = label(0, {895, 295, 955, 315}, L"");
  ui_label_margins[1] = label(0, {895, 320, 955, 340}, L"");
  ui_label_margins[2] = label(0, {895, 345, 955, 365}, L"");
  ui_label_margins[3] = label(0, {895, 370, 955, 390}, L"");

  control::show(0);
  render_all();

  mouse::moved = [](int4 dx, int4 dy) { // マウス操作
    if (ResultMode) return;
    auto x = mouse::x - dx, y = mouse::y - dy;
    if (5 <= x && x <= 805 && 85 <= y && y <= 535) {
      bool b = false;
      if (mouse::left) {
        b = true;
        Camera.rotation.y -= dx * 0.003f / Camera.factor;
        Camera.rotation.x += dy * 0.003f / Camera.factor;
        if (Camera.rotation.x < -pi / 2) Camera.rotation.x = fat4(-pi / 2);
        else if (Camera.rotation.x > pi / 2) Camera.rotation.x = fat4(pi / 2);
      } else if (mouse::middle) {
        b = true;
        Camera.offset.x += dx * 2.4f / Camera.factor;
        Camera.offset.y += dy * 2.4f / Camera.factor;
      }
      if (b) {
        Camera.update();
        cb_camera.from(list<>::asref(Camera.view, Camera.view_proj));
        render_all();
      }
    }
  };

  mouse::wheeled = [](int4 d) { // マウスホイール
    if (ResultMode) return;
    if (5 <= mouse::x && mouse::x <= 805 && 85 <= mouse::y && mouse::y <= 535) {
      if (d > 0) Camera.factor *= 1.2f;
      else Camera.factor /= 1.2f;
      Camera.update();
      cb_camera.from(list<>::asref(Camera.view, Camera.view_proj));
      render_all();
    }
  };

  while (main::update) {
    align();
  }

  main::terminate();
}
