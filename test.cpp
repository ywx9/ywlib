#include "ywlib"
using namespace yw;

struct facet { // CADデータの要素
  unsigned id;
  std::array<float, 3> a, b, c;
  std::array<float, 3> ab, bc, ca;
  std::array<float, 3> sq, nisq;
  vector plane;
};

struct vertex { // CMTデータの要素
  vector p, n;
};

struct margin { // 取代
  float margin, perp;
  unsigned id, inside;
};

struct maxmin {
  std::array<vector, 26> maxs, mins;
  vector minimum, minimum_n;
  unsigned minimum_i;
};

namespace {

constexpr auto app_name = L"ACMA-TEST-V0.6";

constexpr nat app_width = 960, app_height = 540;

path cmt_path;

button ui_button_cmt{};
button ui_button_cad{};
label ui_label_cmt{};
label ui_label_cad{};
radiobutton ui_radiobutton_hide{};
checkbox ui_checkbox_reverse{};
checkbox ui_checkbox_alpha{};
checkbox ui_checkbox_autoalignmode{};
progressbar ui_progressbar_align{};
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
label ui_label_min_over{};
label ui_label_min_side{};
label ui_label_max_side{};
label ui_label_timer{};
label ui_label_margins[4];
button ui_button_result{};

// Group 2

radiobutton ui_radiobutton_result[2];
textbox ui_textbox_result[8];
button ui_button_output;
button ui_button_endresult;

structured_buffer<facet> sb_facets{};
unordered_buffer<facet> ub_facets{};
structured_buffer<vertex> sb_vertices{};
unordered_buffer<vertex> ub_vertices{};
structured_buffer<margin> sb_margins{};
unordered_buffer<margin> ub_margins{};
constant_buffer<xmatrix> cb_world{};
constant_buffer<list<xmatrix, xmatrix>> cb_camera{};
constant_buffer<list<unsigned, float, unsigned long long>> cb_options{};

ff::stl stl_facets{};
float facets_y_center{};

camera Camera{};
maxmin MaxMin{};

coordinator Coord{};

bool Reversed = false;
bool ResultMode = false;
int AlignMode = -1;

float journal_hole_radius{};
float half_gap{};
float half_stroke{};
bool has_second_side_chamfer = false;

constexpr rect rect_textbox_result[8] = {
  {10, 890, 110, 940},
  {110, 890, 260, 940},
  {260, 890, 410, 940},
  {410, 890, 560, 940},
  {560, 890, 760, 940},
  {760, 890, 960, 940},
  {960, 890, 1190, 940},
  {1190, 890, 1390, 940}};
}

unordered_buffer<facet> facets_from_stl(const ff::stl& Stl, bool Reverse) {
  static constexpr std::string_view hlsl = R"(
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
  try {
    if (Stl.empty()) return {};
    static constexpr auto a = std::array{
      std::array<unsigned, 25>{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24},
      std::array<unsigned, 25>{12, 13, 14, 15, 16, 17, 21, 22, 23, 18, 19, 20, 0, 1, 2, 3, 4, 5, 9, 10, 11, 6, 7, 8, 24}};
    static auto gp = gpgpu<typepack<facet>, typepack<list<unsigned, std::array<float, 9>>>>(hlsl);
    unordered_buffer<facet> ub(Stl.size());
    std::vector<list<unsigned, std::array<float, 9>>> t(Stl.size());
    float ymax, ymin;
    has_second_side_chamfer = false;
    for (nat i{}; i < t.size(); ++i)
      ymax = max(ymax, Stl[i].vertex[0][1], Stl[i].vertex[1][1], Stl[i].vertex[2][2]),
      ymin = min(ymin, Stl[i].vertex[0][1], Stl[i].vertex[1][1], Stl[i].vertex[2][2]),
      t[i].first = a[Reverse][Stl[i].attribute], memcpy(t[i].second.data(), Stl[i].vertex.data(), 36),
      has_second_side_chamfer |= t[i].first == 8;
    structured_buffer sb(t);
    gp(t.size(), {ub}, {sb}, {});
    facets_y_center = (ymax + ymin) / 2;
    journal_hole_radius = *reinterpret_cast<const float*>(Stl.header().data() + 16);
    half_gap = *reinterpret_cast<const float*>(Stl.header().data() + 20);
    half_stroke = *reinterpret_cast<const float*>(Stl.header().data() + 24);
    return ub;
  } catch (const std::exception& E) {
    except(E).receive();
    return {};
  }
}

unordered_buffer<vertex> vertices_from_stl(const ff::stl& Stl) {
  static constexpr std::string_view hlsl = R"(
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
  try {
    if (Stl.empty()) return {};
    static auto gp = gpgpu<typepack<vertex>, typepack<std::array<float, 9>>>(hlsl);
    unordered_buffer<vertex> ub(Stl.size() * 3);
    std::vector<std::array<float, 9>> t(Stl.size());
    for (nat i{}; i < t.size(); ++i) memcpy(t[i].data(), Stl[i].vertex.data(), 36);
    structured_buffer sb(t);
    gp(t.size(), {ub}, {sb}, {});
    return ub;
  } catch (const std::exception& E) {
    except(E).receive();
    return {};
  }
}

void render_facets(const structured_buffer<facet>& Facets,
                   const constant_buffer<list<xmatrix, xmatrix>>& Camera,
                   const constant_buffer<list<unsigned, float, unsigned long long>>& Options, source Source = {}) {
  static constexpr std::string_view hlsl = R"(
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
                            typepack<>, typepack<constant_buffer<list<xmatrix, xmatrix>>, constant_buffer<list<unsigned, unsigned, unsigned long long>>>>(hlsl);
  try {
    rd(Facets.count * 3, {Facets}, {}, {}, {Camera, Options});
  } catch (const std::exception& E) { except(E).receive(); }
}

void render_vertices(const structured_buffer<vertex>& Vertices, const structured_buffer<margin>& Margins,
                     const constant_buffer<xmatrix>& World, const constant_buffer<list<xmatrix, xmatrix>>& Camera,
                     const constant_buffer<list<unsigned, float, unsigned long long>>& Options) {
  static constexpr std::string_view hlsl = R"(
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
  if (m > 60) Out.c = float4(0, 0, 0.545098039, 1);
  else if (m > 45) Out.c = float4(0.254901961, 0.411764706, 0.882352941, 1);
  else if (m > 24) Out.c = float4(0.235294118, 0.701960784, 0.443137255, 1);
  else if (m > 10) Out.c = float4(1, 1, 0, 1);
  else if (m > 5) Out.c = float4(1, 0.549019608, 0, 1);
  else Out.c = float4(1, 0, 0, 1);
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
  if (In.n.z > 0) return float4(0.5, 0.5, 0.5, In.c.w);
  else return float4(In.c.xyz * (0.3 - 0.6 * In.n.z), In.c.w);
})";
  try {
    static auto rd = renderer<typepack<structured_buffer<vertex>, structured_buffer<margin>>,
                              typepack<constant_buffer<xmatrix>>,
                              typepack<>,
                              typepack<constant_buffer<list<xmatrix, xmatrix>>,
                                       constant_buffer<list<unsigned, float, unsigned long long>>>>(hlsl);
    rd(Vertices.count, {Vertices, Margins}, {World}, {}, {Camera, Options});
  } catch (const std::exception& E) { except(E).receive(); }
}

void calc_maxmin(const structured_buffer<vertex>& Vertices, const unordered_buffer<margin>& Margins,
                 const constant_buffer<xmatrix>& World, maxmin& MaxMin) {
  static constexpr std::string_view hlsl = R"(
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
cbuffer CB1 :register(b1) {
  matrix w; // ワールド行列
};
[numthreads(1024, 1, 1)] void csmain(uint i : SV_DispatchThreadID) {
  for (uint k = 0; k < 25; ++k) ub1[i].maxs[k].w = -1e10, ub1[i].mins[k].w = 1e10;
  ub1[i].maxs[25] = float4(-1e10, -1e10, -1e10, 1), ub1[i].mins[25] = float4(1e10, 1e10, 1e10, 1);
  ub1[i].minimum.w = 1e10;
  const uint j0 = i * n, jj = j0 + n;
  for (uint j = j0; j < jj && j < m; ++j) {
    const UB0 t = ub0[j]; const SB0 s = {mul(w, sb0[j].p), mul(w, sb0[j].n)};
    if (ub1[i].maxs[t.id].w < t.margin && t.inside) ub1[i].maxs[t.id] = float4(s.p.xyz, t.margin);
    if (ub1[i].mins[t.id].w > t.margin && t.inside) ub1[i].mins[t.id] = float4(s.p.xyz, t.margin);
    if (ub1[i].minimum.w > t.margin) ub1[i].minimum = float4(s.p.xyz, t.margin),
                                     ub1[i].minimum_n = s.n, ub1[i].minimum_i = t.id;
    ub1[i].maxs[25] = max(ub1[i].maxs[25], s.p); ub1[i].mins[25] = min(ub1[i].mins[25], s.p);
  }
})";
  try {
    static auto gp = gpgpu<typepack<margin, maxmin>, typepack<vertex>, typepack<list<unsigned, unsigned, unsigned long long>, xmatrix>>(hlsl);
    static auto cb = constant_buffer(list<unsigned, unsigned, unsigned long long>{}); // 頂点の数、スレッド内の処理数、パディング
    static auto ub = unordered_buffer<maxmin>(1024);
    static auto st = staging_buffer<maxmin>(1024);
    cb.from(list<>::asref(Vertices.count, (Vertices.count - 1) / 1024 + 1, nat(0)));
    gp(ub.count, {Margins, ub}, {Vertices}, {cb, World});
    auto t = ub.to_cpu(st);
    for (nat i{512}; i != 0; i /= 2) {
      for (nat j{}; j < i; ++j) {
        for (nat k{}; k < 25; ++k) {
          if (t[j].maxs[k].w < t[j + i].maxs[k].w) t[j].maxs[k] = t[j + i].maxs[k];
          if (t[j].mins[k].w > t[j + i].mins[k].w) t[j].mins[k] = t[j + i].mins[k];
        }
        if (t[j].minimum.w > t[j + i].minimum.w)
          t[j].minimum = t[j + i].minimum, t[j].minimum_n = t[j + i].minimum_n, t[j].minimum_i = t[j + i].minimum_i;
        t[j].maxs[25] = xvmax(t[j].maxs[25], t[j + i].maxs[25]), t[j].mins[25] = xvmin(t[j].mins[25], t[j + i].mins[25]);
      }
    }
    MaxMin = mv(t[0]);
  } catch (const std::exception& E) { except(E).receive(); }
}

void calc_margin(const structured_buffer<facet>& Facets, const structured_buffer<vertex>& Vertices,
                 const constant_buffer<xmatrix>& World, unordered_buffer<margin>& Margins) {
  static constexpr std::string_view hlsl = R"(
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
  try {
    static auto gp = gpgpu<typepack<margin>, typepack<facet, vertex>, typepack<xmatrix, list<unsigned, unsigned, unsigned long long>>>(hlsl);
    static auto cb = constant_buffer(list<unsigned, unsigned, unsigned long long>{});
    cb.from(list<>::asref(Facets.count, Vertices.count, 0));
    gp(Vertices.count, {Margins}, {Facets, Vertices}, {World, cb});
  } catch (const std::exception& E) { except(E).receive(); }
}

void update_world() {
  static constexpr auto f = [](xmatrix& m) {
    auto t = xvload(ui_valuebox_off[0].value<float>(), ui_valuebox_off[1].value<float>(), ui_valuebox_off[2].value<float>(), 0.f);
    auto r = xvload(ui_valuebox_rot[0].value<float>(), ui_valuebox_rot[1].value<float>(), ui_valuebox_rot[2].value<float>(), 0.f);
    xvworld(t, xvneg(xvradian(r)), m); };
  cb_world.from(f);
}

void render_all() {
  try {
    Camera.begin_render(color::white);
    if (sb_facets) render_facets(sb_facets, cb_camera, cb_options);
    if (sb_vertices) render_vertices(sb_vertices, sb_margins, cb_world, cb_camera, cb_options);
    Camera.end_render();
    main::begin_draw(color::yw);
    Camera.draw({5, 85, 805, 535});
    // Coord.texture.draw({5, 5, 805, 85});
    Coord.draw({5, 85, 105, 185});
    main::end_draw();
    main::update(), main::update();
  } catch (const std::exception& E) { except(E).receive(); }
}

template<nat I> bool checker(const maxmin& Now, const maxmin& New) {
  if constexpr (I == 0) // ジャーナル穴最小取代最大化 (yz軸移動)
    return min(Now.mins[3].w + Now.mins[15].w) < min(New.mins[3].w + New.mins[15].w);
  else if constexpr (I == 1) // 基準面厚み最小化 (yz軸回転)
    return (Now.maxs[25].x - Now.mins[25].x) > (New.maxs[25].x - New.mins[25].x);
  else if constexpr (I == 2) // 基準面最大取代最小化 (x軸移動)
    return max(Now.maxs[0].w, Now.maxs[12].w) > max(New.maxs[0].w, New.maxs[12].w);
  else if constexpr (I == 3) // 側面最大取代最小化 (x軸回転)
    return max(Now.maxs[6].w, Now.maxs[7].w, Now.maxs[8].w, Now.maxs[9].w, Now.maxs[10].w, Now.maxs[11].w) >
           max(New.maxs[6].w, New.maxs[7].w, New.maxs[8].w, New.maxs[9].w, New.maxs[10].w, New.maxs[11].w);
  else if constexpr (I == 4) // 基準面+内股最小取代最大化 (yz軸回転、x軸移動)
    return min(Now.mins[0].w, Now.mins[2].w, Now.mins[12].w, Now.mins[14].w) <
           min(New.mins[0].w, New.mins[2].w, New.mins[12].w, New.mins[14].w);
  else if constexpr (I == 5) // 　　最小取代最大化 (x軸回転、XYZ軸移動)
    return apply(min, projector(Now.mins, &vector::w, make_sequence<25>{})) < apply(min, projector(New.mins, &vector::w, make_sequence<25>{}));
}

template<nat I> bool select_best(
  unordered_buffer<margin>& Margins, std::array<maxmin, 13>& Mm,
  xmatrix& Matrix, const xvector& Theta, float Delta, unsigned Blocker) {
  nat best{12};
  auto off = xvload(Delta), rot = xvmul(Theta, off);
  xmatrix mtemp, original = Matrix;
  // std::cout << std::format("{}\n", original);
  if (!(Blocker & 1)) {
    auto d = xvsetzero<0, 1, 1, 1>(off);
    xvworld(d, xv_zero, mtemp);
    cb_world.from([&](xmatrix& m) { xvdot<4>(mtemp, original, m); });
    calc_margin(sb_facets, sb_vertices, cb_world, Margins);
    calc_maxmin(sb_vertices, Margins, cb_world, Mm[0]);
    if (checker<I>(Mm[best], Mm[0])) best = 0, sb_margins.from(Margins), Matrix = mtemp;
    else {
      d = xvneg(d);
      xvworld(d, xv_zero, mtemp);
      cb_world.from([&](xmatrix& m) { xvdot<4>(mtemp, original, m); });
      calc_margin(sb_facets, sb_vertices, cb_world, Margins);
      calc_maxmin(sb_vertices, Margins, cb_world, Mm[6]);
      if (checker<I>(Mm[best], Mm[6])) best = 6, sb_margins.from(Margins), Matrix = mtemp;
    }
  }
  if (!(Blocker & 2)) {
    auto d = xvsetzero<1, 0, 1, 1>(off);
    xvworld(d, xv_zero, mtemp);
    cb_world.from([&](xmatrix& m) { xvdot<4>(mtemp, original, m); });
    calc_margin(sb_facets, sb_vertices, cb_world, Margins);
    calc_maxmin(sb_vertices, Margins, cb_world, Mm[1]);
    if (checker<I>(Mm[best], Mm[1])) best = 1, sb_margins.from(Margins), Matrix = mtemp;
    else {
      d = xvneg(d);
      xvworld(d, xv_zero, mtemp);
      cb_world.from([&](xmatrix& m) { xvdot<4>(mtemp, original, m); });
      calc_margin(sb_facets, sb_vertices, cb_world, Margins);
      calc_maxmin(sb_vertices, Margins, cb_world, Mm[7]);
      if (checker<I>(Mm[best], Mm[7])) best = 7, sb_margins.from(Margins), Matrix = mtemp;
    }
  }
  if (!(Blocker & 4)) {
    auto d = xvsetzero<1, 1, 0, 1>(off);
    xvworld(d, xv_zero, mtemp);
    cb_world.from([&](xmatrix& m) { xvdot<4>(mtemp, original, m); });
    calc_margin(sb_facets, sb_vertices, cb_world, Margins);
    calc_maxmin(sb_vertices, Margins, cb_world, Mm[2]);
    if (checker<I>(Mm[best], Mm[2])) best = 2, sb_margins.from(Margins), Matrix = mtemp;
    else {
      d = xvneg(d);
      xvworld(d, xv_zero, mtemp);
      cb_world.from([&](xmatrix& m) { xvdot<4>(mtemp, original, m); });
      calc_margin(sb_facets, sb_vertices, cb_world, Margins);
      calc_maxmin(sb_vertices, Margins, cb_world, Mm[8]);
      if (checker<I>(Mm[best], Mm[8])) best = 8, sb_margins.from(Margins), Matrix = mtemp;
    }
  }
  if (!(Blocker & 8)) {
    auto d = xvsetzero<0, 1, 1, 1>(rot);
    xvworld(xv_zero, d, mtemp);
    cb_world.from([&](xmatrix& m) { xvdot<4>(mtemp, original, m); });
    calc_margin(sb_facets, sb_vertices, cb_world, Margins);
    calc_maxmin(sb_vertices, Margins, cb_world, Mm[3]);
    if (checker<I>(Mm[best], Mm[3])) best = 3, sb_margins.from(Margins), Matrix = mtemp;
    else {
      d = xvneg(d);
      xvworld(xv_zero, d, mtemp);
      cb_world.from([&](xmatrix& m) { xvdot<4>(mtemp, original, m); });
      calc_margin(sb_facets, sb_vertices, cb_world, Margins);
      calc_maxmin(sb_vertices, Margins, cb_world, Mm[9]);
      if (checker<I>(Mm[best], Mm[9])) best = 9, sb_margins.from(Margins), Matrix = mtemp;
    }
  }
  if (!(Blocker & 16)) {
    auto d = xvsetzero<1, 0, 1, 1>(rot);
    xvworld(xv_zero, d, mtemp);
    cb_world.from([&](xmatrix& m) { xvdot<4>(mtemp, original, m); });
    calc_margin(sb_facets, sb_vertices, cb_world, Margins);
    calc_maxmin(sb_vertices, Margins, cb_world, Mm[4]);
    if (checker<I>(Mm[best], Mm[4])) best = 4, sb_margins.from(Margins), Matrix = mtemp;
    else {
      d = xvneg(d);
      xvworld(xv_zero, d, mtemp);
      cb_world.from([&](xmatrix& m) { xvdot<4>(mtemp, original, m); });
      calc_margin(sb_facets, sb_vertices, cb_world, Margins);
      calc_maxmin(sb_vertices, Margins, cb_world, Mm[10]);
      if (checker<I>(Mm[best], Mm[10])) best = 10, sb_margins.from(Margins), Matrix = mtemp;
    }
  }
  if (!(Blocker & 32)) {
    auto d = xvsetzero<1, 1, 0, 1>(rot);
    xvworld(xv_zero, d, mtemp);
    cb_world.from([&](xmatrix& m) { xvdot<4>(mtemp, original, m); });
    calc_margin(sb_facets, sb_vertices, cb_world, Margins);
    calc_maxmin(sb_vertices, Margins, cb_world, Mm[5]);
    if (checker<I>(Mm[best], Mm[5])) best = 5, sb_margins.from(Margins), Matrix = mtemp;
    else {
      d = xvneg(d);
      xvworld(xv_zero, d, mtemp);
      cb_world.from([&](xmatrix& m) { xvdot<4>(mtemp, original, m); });
      calc_margin(sb_facets, sb_vertices, cb_world, Margins);
      calc_maxmin(sb_vertices, Margins, cb_world, Mm[11]);
      if (checker<I>(Mm[best], Mm[11])) best = 11, sb_margins.from(Margins), Matrix = mtemp;
    }
  }
  if (best == 12) return cb_world.from(Matrix = original), true;
  else return xvdot<4>(Matrix, original, mtemp), cb_world.from(Matrix = mtemp), Mm[12] = Mm[best], false;
}

void align() {
  static auto margins = unordered_buffer<margin>{};
  static std::array<maxmin, 13> mm;
  static xmatrix matrix;
  static xvector theta;
  static double time;
  static float minnow;

  if (!(sb_facets && sb_vertices)) return;

  if (AlignMode == -2) {
    AlignMode = -1;
    vector euler = xvneg(xvdegree(xveuler(matrix)));
    ui_valuebox_rot[0].text(std::format(L"{:.4f}  ", euler.x));
    ui_valuebox_rot[1].text(std::format(L"{:.4f}  ", euler.y));
    ui_valuebox_rot[2].text(std::format(L"{:.4f}  ", euler.z));
    ui_valuebox_off[0].text(std::format(L"{:.3f}  ", xvget<3>(matrix[0])));
    ui_valuebox_off[1].text(std::format(L"{:.3f}  ", xvget<3>(matrix[1])));
    ui_valuebox_off[2].text(std::format(L"{:.3f}  ", xvget<3>(matrix[2])));
    return;
  }

  stopwatch sw;

  if (AlignMode == 0) {
    minnow = -1e10;
    time = 0;
    ++AlignMode;
  } else if (AlignMode == 1) {
    xmatrix temp;
    xvworld(xvload(ui_valuebox_off[0].value<float>(), ui_valuebox_off[1].value<float>(), ui_valuebox_off[2].value<float>(), 0.f),
            xvneg(xvradian(xvload(ui_valuebox_rot[0].value<float>(), ui_valuebox_rot[1].value<float>(), ui_valuebox_rot[2].value<float>(), 0.f))), temp);
    // std::cout << std::format("{}, {}, {}, {}\n", vector(temp[0]), vector(temp[1]), vector(temp[2]), vector(temp[3]));
    cb_world.from(temp);
    if (margins.count != sb_vertices.count) margins = unordered_buffer<margin>(sb_vertices.count);
    calc_margin(sb_facets, sb_vertices, cb_world, margins), calc_maxmin(sb_vertices, margins, cb_world, mm[12]);
    // theta = xvadd(xvmul(xvadd(mm[12].maxs[25], mm[12].mins[25]), xv_constant<-0.5f>), xvset<1>(xv_zero, facets_y_center));
    theta = xvset<1>(xvmul(xvadd(mm[12].maxs[25], mm[12].mins[25]), xv_constant<-0.5f>), -1.2f * journal_hole_radius - mm[12].mins[25].y);
    xvworld(theta, matrix);
    xvdot<4>(matrix, temp); // 中心座標を原点に移す行列
    // std::cout << std::format("{}, {}, {}, {}\n", vector(temp[0]), vector(temp[1]), vector(temp[2]), vector(temp[3]));
    theta = xvmax(xvadd(theta, mm[12].maxs[25]), xvsub(theta, mm[12].mins[25]));         // 原点から各軸方向の最大距離
    theta = xvmul(theta, theta);                                                         // 最大距離の二乗
    theta = xvsqrt(xvadd(xvpermute<1, 2, 0, -1>(theta), xvpermute<2, 0, 1, -1>(theta))); // 各断面での最大距離
    theta = xvdiv(xv_constant<1>, theta);                                                // およそ1mmあたりの回転角度
    cb_world.from(matrix), ++AlignMode;
    calc_margin(sb_facets, sb_vertices, cb_world, ub_margins), sb_margins.from(ub_margins);
    calc_maxmin(sb_vertices, ub_margins, cb_world, mm[12]);
  }

  else if (AlignMode == 2) {
    if (select_best<0>(margins, mm, matrix, theta, 25.f, 0b111001)) ++AlignMode;
  } else if (AlignMode == 3) {
    if (select_best<0>(margins, mm, matrix, theta, 5.f, 0b111001)) ++AlignMode;
  } else if (AlignMode == 4) {
    if (select_best<0>(margins, mm, matrix, theta, 1.f, 0b111001)) ++AlignMode;
  }

  else if (AlignMode == 5) {
    if (select_best<1>(margins, mm, matrix, theta, 25.f, 0b001111)) ++AlignMode;
  } else if (AlignMode == 6) {
    if (select_best<1>(margins, mm, matrix, theta, 5.f, 0b001111)) ++AlignMode;
  } else if (AlignMode == 7) {
    if (select_best<1>(margins, mm, matrix, theta, 1.f, 0b001111)) ++AlignMode;
  }

  else if (AlignMode == 8) {
    if (select_best<2>(margins, mm, matrix, theta, 25.f, 0b111110)) ++AlignMode;
  } else if (AlignMode == 9) {
    if (select_best<2>(margins, mm, matrix, theta, 5.f, 0b111110)) ++AlignMode;
  } else if (AlignMode == 10) {
    if (select_best<2>(margins, mm, matrix, theta, 1.f, 0b111110)) ++AlignMode;
  }

  else if (AlignMode == 11) {
    if (select_best<3>(margins, mm, matrix, theta, 25.f, 0b110111)) ++AlignMode;
  } else if (AlignMode == 12) {
    if (select_best<3>(margins, mm, matrix, theta, 5.f, 0b110111)) ++AlignMode;
  } else if (AlignMode == 13) {
    if (select_best<3>(margins, mm, matrix, theta, 1.f, 0b110111)) ++AlignMode;
  }

  else if (AlignMode == 14) {
    if (select_best<4>(margins, mm, matrix, theta, 25.f, 0b001110)) ++AlignMode;
  } else if (AlignMode == 15) {
    if (select_best<4>(margins, mm, matrix, theta, 5.f, 0b001110)) ++AlignMode;
  } else if (AlignMode == 16) {
    if (select_best<4>(margins, mm, matrix, theta, 1.f, 0b001110)) ++AlignMode;
  } else if (AlignMode == 17) {
    if (select_best<4>(margins, mm, matrix, theta, .2f, 0b001110)) ++AlignMode;
  }

  else if (AlignMode == 18) {
    if (mm[12].minimum.w < 0) AlignMode = 21;
    else if (select_best<5>(margins, mm, matrix, theta, 5.f, 0)) ++AlignMode;
  } else if (AlignMode == 19) {
    if (select_best<5>(margins, mm, matrix, theta, 1.f, 0)) ++AlignMode;
  } else if (AlignMode == 20) {
    if (select_best<5>(margins, mm, matrix, theta, .2f, 0)) ++AlignMode;
  }

  else if (AlignMode == 21) {
    if (ui_checkbox_autoalignmode.state && minnow < mm[12].minimum.w) {
      minnow = mm[12].minimum.w;
      vector euler = xvneg(xvdegree(xveuler(matrix)));
      ui_valuebox_rot[0].text(std::format(L"{:.4f}  ", euler.x));
      ui_valuebox_rot[1].text(std::format(L"{:.4f}  ", euler.y));
      ui_valuebox_rot[2].text(std::format(L"{:.4f}  ", euler.z));
      ui_valuebox_off[0].text(std::format(L"{:.3f}  ", xvget<3>(matrix[0])));
      ui_valuebox_off[1].text(std::format(L"{:.3f}  ", xvget<3>(matrix[1])));
      ui_valuebox_off[2].text(std::format(L"{:.3f}  ", xvget<3>(matrix[2])));
      AlignMode = 1;
    } else ++AlignMode;
  }

  else {
    AlignMode = -1;
    vector euler = xvneg(xvdegree(xveuler(matrix)));
    ui_valuebox_rot[0].text(std::format(L"{:.4f}  ", euler.x));
    ui_valuebox_rot[1].text(std::format(L"{:.4f}  ", euler.y));
    ui_valuebox_rot[2].text(std::format(L"{:.4f}  ", euler.z));
    ui_valuebox_off[0].text(std::format(L"{:.3f}  ", xvget<3>(matrix[0])));
    ui_valuebox_off[1].text(std::format(L"{:.3f}  ", xvget<3>(matrix[1])));
    ui_valuebox_off[2].text(std::format(L"{:.3f}  ", xvget<3>(matrix[2])));
    update_world(), calc_margin(sb_facets, sb_vertices, cb_world, ub_margins), sb_margins.from(ub_margins), calc_maxmin(sb_vertices, ub_margins, cb_world, MaxMin);
  }

  time += sw();
  ui_progressbar_align.progress(AlignMode / 20.0);
  ui_label_margins[0].text(std::format(L"{:.3f}  ", apply(min, projector(mm[12].mins, &vector::w, make_sequence<25>{}))));
  ui_label_margins[1].text(std::format(L"{:.3f}  ", apply(min, projector(mm[12].mins, &vector::w, sequence<4, 6, 9, 16, 18, 21>{}))));
  ui_label_margins[2].text(std::format(L"{:.3f}  ", apply(max, projector(mm[12].maxs, &vector::w, sequence<4, 6, 9, 16, 18, 21>{}))));
  ui_label_margins[3].text(std::format(L"{:.1f}", time));
  render_all();
}


float calc_volume(const structured_buffer<vertex>& Vertices) {
  static constexpr std::string_view hlsl = R"(
#pragma pack_matrix(row_major)
struct UB0 {
  float volume; // 体積
};
RWStructuredBuffer<UB0> ub0 : register(u0);
struct SB0 {
  float4 p; // 頂点座標
  float4 n; // 法線ベクトル
};
StructuredBuffer<SB0> sb0 : register(t0);
cbuffer CB0 : register(b0) {
  uint m; // 頂点の数
  uint pad[3];
};
[numthreads(1024, 1, 1)] void csmain(uint i : SV_DispatchThreadID) {
  if (i < m) ub0[i].volume = dot(cross(sb0[3 * i + 1].p.xyz, sb0[3 * i + 2].p.xyz), sb0[3 * i].p.xyz);
  else ub0[i].volume = 0;
})";
  try {
    static auto gp = gpgpu<typepack<float>, typepack<vertex>, typepack<list<unsigned, unsigned, unsigned long long>>>(hlsl);
    static auto cb = constant_buffer(list<unsigned, unsigned, unsigned long long>{});
    static auto ub = unordered_buffer<float>{};
    auto m = std::bit_ceil(Vertices.count / 3);
    if (ub.count != m) ub = unordered_buffer<float>(m);
    cb.from(list{Vertices.count, unsigned(0), nat(0)});
    gp(m, {ub}, {Vertices}, {cb});
    auto temp = ub.to_cpu();
    for (nat i{temp.size() / 2}; i != 0; i /= 2)
      for (nat j{0}; j < i; ++j) temp[j] += temp[j + i];
    return temp[0] * float(1e-9 / 6);
  } catch (const std::exception& E) { except(E).receive(); }
  return {};
}


inline std::array<vector, 15> calc_punch(const structured_buffer<vertex>& Vertices, const std::array<vector, 15>& Ref) {
  std::array<vector, 15> punches = Ref;
  unordered_buffer<std::array<float, 15>> ub0(nullptr, Vertices.count);
  unordered_buffer<std::array<float, 15>> ub1(nullptr, 1024);
  constant_buffer<list<unsigned, unsigned, unsigned long long, std::array<vector, 15>>> cb(list{
    unsigned(Vertices.count), unsigned((Vertices.count - 1) / 1024 + 1), nat(0), Ref});
  try { // STEP 1
    static constexpr std::string_view hlsl = R"(
    #pragma pack_matrix(row_major)
    struct UB0 { float d[15]; };
    struct SB0 { float4 v, n; };
    cbuffer CB0 : register(b0) { float4 cad; };
    cbuffer CB1 : register(b1) { matrix mm; };
    RWStructuredBuffer<UB0> ub0 : register(u0);
      StructuredBuffer<SB0> sb0 : register(t0);
    float f0(float3 a, float3 b, float3 c) {
      float t = a.x * b.y - a.y * b.x;
      if (t < 0) { t = b.x * c.y - b.y * c.x;
                   if (t > 0) return 0.0;
                   else if (t < 0) { t = c.x * a.y - c.y * a.x;
                                     if (t > 0) return 0.0; } }
      else if (t > 0) { t = b.x * c.y - b.y * c.x;
                        if (t < 0) return 0.0;
                        else if (t > 0) { t = c.x * a.y - c.y * a.x;
                                          if (t < 0) return 0.0; } }
      return dot(cross(a, b), c) / (a.x * b.y - a.y * b.x + b.x * c.y - b.y * c.x + c.x * a.y - c.y * a.x);
    }
    struct Type { float4 a, b, c; };
    [numthreads(1024, 1, 1)] void csmain(uint i : SV_DispatchThreadID) {
      const uint j = 3 * i;
      Type t;
      t.a = mul(mm, sb0[j].v);
      t.b = mul(mm, sb0[j + 1].v);
      t.c = mul(mm, sb0[j + 2].v);
      ub0[i].d[0] = f0(float3(t.a.y+cad.x,t.a.z,t.a.x), float3(t.b.y+cad.x,t.b.z,t.b.x), float3(t.c.y+cad.x,t.c.z,t.c.x));
      ub0[i].d[1] = f0(float3(t.a.y,t.a.z-cad.x,t.a.x), float3(t.b.y,t.b.z-cad.x,t.b.x), float3(t.c.y,t.c.z-cad.x,t.c.x));
      ub0[i].d[2] = f0(float3(t.a.y-cad.x,t.a.z,t.a.x), float3(t.b.y-cad.x,t.b.z,t.b.x), float3(t.c.y-cad.x,t.c.z,t.c.x));
      ub0[i].d[3] = f0(float3(t.a.y,t.a.z+cad.x,t.a.x), float3(t.b.y,t.b.z+cad.x,t.b.x), float3(t.c.y,t.c.z+cad.x,t.c.x));
      ub0[i].d[4] = f0(float3(t.a.y-cad.z,t.a.z,t.a.x), float3(t.b.y-cad.z,t.b.z,t.b.x), float3(t.c.y-cad.z,t.c.z,t.c.x));
      ub0[i].d[5] = f0(float3(t.a.x+cad.y,t.a.y,t.a.z), float3(t.b.x+cad.y,t.b.y,t.b.z), float3(t.c.x+cad.y,t.c.y,t.c.z));
      ub0[i].d[6] = f0(float3(t.a.x+cad.y,t.a.y-cad.z,t.a.z), float3(t.b.x+cad.y,t.b.y-cad.z,t.b.z), float3(t.c.x+cad.y,t.c.y-cad.z,t.c.z));
      ub0[i].d[7] = ub0[i].d[5];
      ub0[i].d[8] = ub0[i].d[6];
      ub0[i].d[9] = f0(float3(t.a.z,t.a.x+cad.y,t.a.y), float3(t.b.z,t.b.x+cad.y,t.b.y), float3(t.c.z,t.c.x+cad.y,t.c.y));
      ub0[i].d[10] = f0(float3(t.a.z,t.a.x-cad.y,t.a.y), float3(t.b.z,t.b.x-cad.y,t.b.y), float3(t.c.z,t.c.x-cad.y,t.c.y));
      ub0[i].d[11] = ub0[i].d[9];
      ub0[i].d[12] = ub0[i].d[10];
      ub0[i].d[13] = f0(float3(t.a.z-50,t.a.x+cad.y,t.a.y), float3(t.b.z-50,t.b.x+cad.y,t.b.y), float3(t.c.z-50,t.c.x+cad.y,t.c.y));
      ub0[i].d[14] = f0(float3(t.a.z+50,t.a.x-cad.y,t.a.y), float3(t.b.z+50,t.b.x-cad.y,t.b.y), float3(t.c.z+50,t.c.x-cad.y,t.c.y));
    })";
    static auto gp = gpgpu<typepack<std::array<float, 15>>, typepack<vertex>, typepack<vector, xmatrix>>(hlsl);
    static auto cb0 = constant_buffer(vector{journal_hole_radius, half_gap, half_stroke});
    gp(Vertices.count / 3, {ub0}, {Vertices}, {cb0, cb_world});
  } catch (const std::exception& E) { throw except(E); }
  try {
    static constexpr std::string_view hlsl = R"(
#pragma pack_matrix(row_major)
struct UB0 { float d[15]; };
struct UB1 { float d[15]; };
RWStructuredBuffer<UB0> ub0 : register(u0);
RWStructuredBuffer<UB1> ub1 : register(u1);
cbuffer CB0 : register(b0) {
  uint m, n, _2, _3;
  float4 cb[15];
};
[numthreads(1024, 1, 1)] void csmain(uint i : SV_DispatchThreadID) {
  for (uint k = 0; k < 15; ++k) ub1[i].d[k] = 0.0f;
  for (uint j = i * n, jj = j + n; j < jj && j < m; ++j) {
    for (uint k = 0; k < 15; ++k) {
      if (cb[k].w < 0) ub1[i].d[k] = min(ub1[i].d[k], ub0[j].d[k]);
      else ub1[i].d[k] = max(ub1[i].d[k], ub0[j].d[k]);
    }
  }
})";
    static auto gp = gpgpu<typepack<std::array<float, 15>, std::array<float, 15>>, typepack<>, typepack<decltype(cb)::value_type>>(hlsl);
    gp(1024, {ub0, ub1}, {}, {cb});
  } catch (const std::exception& E) { throw except(E); }
  try {
    auto temp = ub1.to_cpu();
    // for (const auto& e : temp) {
    //   for (const auto& f : e) std::cout << std::format("{}-", f);
    //   std::cout << std::endl;
    // }
    for (nat i{512}; i != 0; i /= 2) {
      for (nat j{}; j < i; ++j) {
        for (nat k{}; k < 15; ++k) {
          if (Ref[k].w < 0) temp[j][k] = min(temp[j][k], temp[j + i][k]);
          else temp[j][k] = max(temp[j][k], temp[j + i][k]);
        }
      }
    }
    for (nat k{}; k < 15; ++k) {
      if (std::abs(Ref[k].w) == 1) punches[k].x = temp[0][k];
      else if (std::abs(Ref[k].w) == 2) punches[k].y = temp[0][k];
      else punches[k].z = temp[0][k];
    }
  } catch (const std::exception& E) { throw except(E); }
  return punches;
}


void output_punch(const path& Path, std::array<vector, 15> Points, const unsigned (&Arrange)[15]) {
  static constexpr auto ff = [](float f) {
    if (f == 1) return vector(1, 0, 0);
    else if (f == 2) return vector(0, 1, 0);
    else if (f == 3) return vector(0, 0, 1);
    else if (f == -1) return vector(-1, 0, 0);
    else if (f == -2) return vector(0, -1, 0);
    else if (f == -3) return vector(0, 0, -1);
    else return vector{};
  };
  try {
    for (nat i{}; i < 15; ++i) std::cout << std::format("{}\n", Points[i]);
    std::ofstream ofs(Path);
    ofs << R"("type";"name";"coord-x";"coord-y";"coord-z";"coord-x2";"coord-y2";"coord-z2";"normal-x";"normal-y";"normal-z";"trimming-x";"trimming-y";"trimming-z";"dir-x";"dir-y";"dir-z";"length";"width";"radius";"radius2";"angle";"orientation";"edge-radius";"num-points";"tol-x-lower";"tol-x-upper";"tol-y-lower";"tol-y-upper";"tol-z-lower";"tol-z-upper";"tol-all-lower";"tol-all-upper";"tol-normal-lower";"tol-normal-upper";"tol-trimming-lower";"tol-trimming-upper";"tol-inplane-lower";"tol-inplane-upper";"tol-length-lower";"tol-length-upper";"tol-width-lower";"tol-width-upper";"tol-diameter-lower";"tol-diameter-upper";"tol-angle-lower";"tol-angle-upper";"tol-xy-lower";"tol-xy-upper";"tol-xz-lower";"tol-xz-upper";"tol-yz-lower";"tol-yz-upper";"coord-x3";"coord-y3";"coord-z3";"tol-diameter2-lower";"tol-diameter2-upper")";
    ofs << std::endl;
    for (nat i{}; i < 15; ++i) {
      auto nn = ff(Points[i].w);
      ofs << std::format(R"("inspection_surface_point";"point {}";"{}";"{}";"{}";"";"";"";"{}";"{}";"{}";"";"";"";"";"";"";"";"";"";"";"";"";"";"";"";"";"";"";"";"";"";"";"";"";"";"";"";"";"";"";"";"";"";"";"";"";"";"";"";"";"";"";"";"";"";"";")",
                         Arrange[i], Points[i].x, Points[i].y, Points[i].z, nn.x, nn.y, nn.z)
          << std::endl;
    }
  } catch (const std::exception& E) { throw except(E); }
}


void show_result() {
  try {
    main::resize(1400, 980);
    key::escape.down = [] { ResultMode = false; };
    // if (key::escape.down) ResultMode = false;

    ui_radiobutton_result[0] = radiobutton{1, rect_textbox_result[0], list{L"合格", L"不合格"}, WS_BORDER};
    ui_radiobutton_result[1] = radiobutton{1, rect_textbox_result[1], list{L"打上後", L"熱処理後"}, WS_BORDER};
    // ui_textbox_result[0] = textbox(1, rect_textbox_result[0], L"", WS_BORDER | ES_CENTER);
    // ui_textbox_result[1] = textbox(1, rect_textbox_result[1], L"", WS_BORDER | ES_CENTER);
    ui_textbox_result[2] = textbox(1, rect_textbox_result[2], L"", WS_BORDER | ES_CENTER);
    ui_textbox_result[3] = textbox(1, rect_textbox_result[3], L"", WS_BORDER | ES_CENTER);
    ui_textbox_result[4] = textbox(1, rect_textbox_result[4], L"", WS_BORDER | ES_CENTER);
    ui_textbox_result[5] = textbox(1, rect_textbox_result[5], L"", WS_BORDER | ES_CENTER);
    ui_textbox_result[6] = textbox(1, rect_textbox_result[6], L"", WS_BORDER | ES_CENTER);
    ui_textbox_result[7] = textbox(1, rect_textbox_result[7], L"", WS_BORDER | ES_CENTER);
    for (int i{}; i < 2; ++i) ui_radiobutton_result[i].font(25);
    for (int i{2}; i < 8; ++i) ui_textbox_result[i].font(25);

    ui_button_output = button(1, {1240, 760, 1390, 800}, L"成績表出力", WS_BORDER);
    ui_button_output.input = [](const button&) noexcept {
      try {
        main::begin_draw();
        for (nat i{}; i < 2; ++i)
          draw_text((rect_textbox_result[i] + rect(0, 10, 0, 0)).to_vector(), font<20, L"Yu Gothic UI", 0, true>{}, ui_radiobutton_result[i].text());
        for (nat i{2}; i < 8; ++i)
          draw_text((rect_textbox_result[i] + rect(0, 10, 0, 0)).to_vector(), font<20, L"Yu Gothic UI", 0, true>{}, ui_textbox_result[i].text());
        main::end_draw();
        auto p = save_file(cmt_path.parent_path(), std::format(L"{}.png", ui_textbox_result[5].text()));
        if (!p.empty()) {
          static constexpr unsigned arrange[] = {14, 13, 12, 15, 11, 9, 10, 4, 3, 2, 1, 8, 5, 7, 6};
          std::array<vector, 15> ref{
            vector(0.0f, -journal_hole_radius, 0.0f, 1.0f),
            vector(0.0f, 0.0f, journal_hole_radius, 1.0f),
            vector(0.0f, journal_hole_radius, 0.0f, 1.0f),
            vector(0.0f, 0.0f, -journal_hole_radius, 1.0f),
            vector(0.0f, half_stroke, 0.0f, 1.0f),
            vector(-half_gap, 0.0f, 0.0f, -3.0f),
            vector(-half_gap, half_stroke, 0.0f, -3.0f),
            vector(-half_gap, 0.0f, 0.0f, 3.0f),
            vector(-half_gap, half_stroke, 0.0f, 3.0f),
            vector(-half_gap, 0.0f, 0.0f, 2.0f),
            vector(half_gap, 0.0f, 0.0f, 2.0f),
            vector(-half_gap, 0.0f, 0.0f, -2.0f),
            vector(half_gap, 0.0f, 0.0f, -2.0f),
            vector(-half_gap, 0.0f, 50.0f, -2.0f),
            vector(half_gap, 0.0f, -50.0f, -2.0f)};
          output_punch(cmt_path.parent_path() / cmt_path.stem().concat(L".csv"), calc_punch(sb_vertices, ref), arrange);
          std::cout << p << std::endl;
          main::screenshot(p);
          if (std::filesystem::exists(p)) ok(L"出力に成功しました");
          else ok(L"出力に失敗したかもしれません");
        }
      } catch (const except& E) { except(E).receive(); }
    };

    ui_button_endresult = button(1, {1240, 805, 1390, 845}, L"芯合わせモード", WS_BORDER);
    ui_button_endresult.input = [](const button&) noexcept { ResultMode = false; };

    main::begin_draw(color::white);

    // タイトル
    draw_text({10, 50, 800, 90}, font<30, L"Yu Gothic UI", -1, true, true>{}, L"芯出成績表 ／ 株式会社 神戸製鋼所 高砂鋳鍛鋼工場");

    // 素材重量
    draw_text({900, 60, 1200, 90}, font<25, L"Yu Gothic UI", -1>{}, std::format(L"素材重量：{:.2f} Ton", calc_volume(sb_vertices) * 7.85));

    // 判定基準
    draw_rectangle({1200, 40, 1390, 65}, brush<color::black>{}, 1.f);
    draw_text({1200, 40, 1390, 65}, font<18>{}, brush<color::red>{}, L"判定基準");
    draw_rectangle({1200, 65, 1390, 90}, brush<color::black>{}, 1.f);
    draw_text({1200, 65, 1390, 90}, font<18>{}, brush<color::red>{}, L"TRS-FP-TE-042. Rev.12");

    // コンタースケール
    draw_rectangle({940, 610, 965, 635}, brush<color::darkblue>{});
    draw_rectangle({940, 610, 965, 635}, brush<color::black>{}, 1.f);
    draw_rectangle({965, 610, 965 + (400 * 15) / 55, 635}, brush<color::royalblue>{});
    draw_rectangle({965, 610, 965 + (400 * 15) / 55, 635}, brush<color::black>{}, 1.f);
    draw_rectangle({965 + (400 * 15) / 55, 610, 965 + (400 * 36) / 55, 635}, brush<color::mediumseagreen>{});
    draw_rectangle({965 + (400 * 15) / 55, 610, 965 + (400 * 36) / 55, 635}, brush<color::black>{}, 1.f);
    draw_rectangle({965 + (400 * 36) / 55, 610, 965 + (400 * 50) / 55, 635}, brush<color::yellow>{});
    draw_rectangle({965 + (400 * 36) / 55, 610, 965 + (400 * 50) / 55, 635}, brush<color::black>{}, 1.f);
    draw_rectangle({965 + (400 * 50) / 55, 610, 965 + (400 * 55) / 55, 635}, brush<color::darkorange>{});
    draw_rectangle({965 + (400 * 50) / 55, 610, 965 + (400 * 55) / 55, 635}, brush<color::black>{}, 1.f);
    draw_rectangle({965 + (400 * 55) / 55, 610, 1390, 635}, brush<color::red>{});
    draw_rectangle({965 + (400 * 55) / 55, 610, 1390, 635}, brush<color::black>{}, 1.f);
    draw_text({965 - 20, 640, 965 + 20, 660}, font<18>{}, L"60");
    draw_text({965 + (400 * 15) / 55 - 20, 640, 965 + (400 * 15) / 55 + 20, 660}, font<18>{}, L"45");
    draw_text({965 + (400 * 36) / 55 - 20, 640, 965 + (400 * 36) / 55 + 20, 660}, font<18>{}, L"24");
    draw_text({965 + (400 * 50) / 55 - 20, 640, 965 + (400 * 50) / 55 + 20, 660}, font<18>{}, L"10");
    draw_text({965 + (400 * 55) / 55 - 20, 640, 965 + (400 * 55) / 55 + 20, 660}, font<18>{}, L"5");
    draw_text({1315, 665, 1395, 685}, font<18, L"Yu Gothic UI", 1>{}, L"[mm]");

    // 成績
    draw_text({950, 680, 1250, 705}, font<20, L"Yu Gothic UI", -1>{}, std::format(L"　　最小取代： {:.2f} mm", apply(min, projector(MaxMin.mins, &vector::w, make_sequence<25>{}))));
    if (has_second_side_chamfer) {
      draw_text({950, 710, 1250, 735}, font<20, L"Yu Gothic UI", -1>{}, std::format(L"外周最小取代： {:.2f} mm", apply(min, projector(MaxMin.mins, &vector::w, sequence<4, 6, 7, 9, 10, 16, 18, 19, 21, 22>{}))));
      draw_text({950, 740, 1250, 765}, font<20, L"Yu Gothic UI", -1>{}, std::format(L"外周最大取代： {:.2f} mm", apply(max, projector(MaxMin.maxs, &vector::w, sequence<4, 6, 7, 9, 10, 16, 18, 19, 21, 22>{}))));
    } else {
      draw_text({950, 710, 1250, 735}, font<20, L"Yu Gothic UI", -1>{}, std::format(L"外周最小取代： {:.2f} mm", apply(min, projector(MaxMin.mins, &vector::w, sequence<4, 6, 9, 16, 18, 21>{}))));
      draw_text({950, 740, 1250, 765}, font<20, L"Yu Gothic UI", -1>{}, std::format(L"外周最大取代： {:.2f} mm", apply(max, projector(MaxMin.maxs, &vector::w, sequence<4, 6, 9, 16, 18, 21>{}))));
    }

    constexpr vector ui_label_result_title[8] = {
      {10, 860, 110, 890},
      {110, 860, 260, 890},
      {260, 860, 410, 890},
      {410, 860, 560, 890},
      {560, 860, 760, 890},
      {760, 860, 960, 890},
      {960, 860, 1190, 890},
      {1190, 860, 1390, 890}};
    constexpr auto vector_frame_lower = std::array{
      rect_textbox_result[0].to_vector(), rect_textbox_result[1].to_vector(), rect_textbox_result[2].to_vector(), rect_textbox_result[3].to_vector(),
      rect_textbox_result[4].to_vector(), rect_textbox_result[5].to_vector(), rect_textbox_result[6].to_vector(), rect_textbox_result[7].to_vector()};
    constexpr float frame_width = 1.f;

    draw_text(ui_label_result_title[0], font<18>{}, L"判定");
    draw_text(ui_label_result_title[1], font<18>{}, L"工程");
    draw_text(ui_label_result_title[2], font<18>{}, L"型入鍛造");
    draw_text(ui_label_result_title[3], font<18>{}, L"芯出");
    draw_text(ui_label_result_title[4], font<18>{}, L"O. No.");
    draw_text(ui_label_result_title[5], font<18>{}, L"Ch. No.");
    draw_text(ui_label_result_title[6], font<18>{}, L"品名/型式");
    draw_text(ui_label_result_title[7], font<18>{}, L"所内図面");
    draw_rectangle(ui_label_result_title[0], brush<color::black>{}, frame_width);
    draw_rectangle(ui_label_result_title[1], brush<color::black>{}, frame_width);
    draw_rectangle(ui_label_result_title[2], brush<color::black>{}, frame_width);
    draw_rectangle(ui_label_result_title[3], brush<color::black>{}, frame_width);
    draw_rectangle(ui_label_result_title[4], brush<color::black>{}, frame_width);
    draw_rectangle(ui_label_result_title[5], brush<color::black>{}, frame_width);
    draw_rectangle(ui_label_result_title[6], brush<color::black>{}, frame_width);
    draw_rectangle(ui_label_result_title[7], brush<color::black>{}, frame_width);
    draw_rectangle(get<0>(vector_frame_lower), brush<color::black>{}, frame_width);
    draw_rectangle(get<1>(vector_frame_lower), brush<color::black>{}, frame_width);
    draw_rectangle(get<2>(vector_frame_lower), brush<color::black>{}, frame_width);
    draw_rectangle(get<3>(vector_frame_lower), brush<color::black>{}, frame_width);
    draw_rectangle(get<4>(vector_frame_lower), brush<color::black>{}, frame_width);
    draw_rectangle(get<5>(vector_frame_lower), brush<color::black>{}, frame_width);
    draw_rectangle(get<6>(vector_frame_lower), brush<color::black>{}, frame_width);
    draw_rectangle(get<7>(vector_frame_lower), brush<color::black>{}, frame_width);

    main::end_draw();

    auto cam_factor = .9f * min(500 / (MaxMin.maxs[25].y - MaxMin.mins[25].y), 230 / max(MaxMin.maxs[25].x - MaxMin.mins[25].x, MaxMin.maxs[25].z - MaxMin.mins[25].z));
    auto cam_offset_y = (MaxMin.maxs[25].y + MaxMin.mins[25].y) / 2;

    static constexpr auto get_color = [](float f) {
      if (f > 60.f) return color::darkblue;
      else if (f > 45.f) return color::royalblue;
      else if (f > 24.f) return color::mediumseagreen;
      else if (f > 10.f) return color::yellow;
      else if (f > 5.f) return color::darkorange;
      else return color::red;
    };

    constant_buffer<list<unsigned, float, unsigned long long>> cb_options(list<>::asref(0, 1.f, 0));
    constant_buffer<list<xmatrix, xmatrix>> cb_camera;

    { // 右サイド
      auto cam = camera(500, 230, 8);
      cam.offset.x = -cam_offset_y;
      cam.offset.z = -10000.f;
      cam.rotation = Reversed ? vector{0, pi, pi / 2} : vector{0, 0, -pi / 2};
      cam.orthographic = true;
      cam.factor = cam_factor;
      cam.update();
      cb_camera.from(list<>::asref(cam.view, cam.view_proj));
      cam.begin_render(color::white);
      render_vertices(sb_vertices, sb_margins, cb_world, cb_camera, cb_options);
      cam.end_render();
      cam.begin_draw();
      // TOP
      vector mx = MaxMin.maxs[21], mn = MaxMin.mins[21];
      if (has_second_side_chamfer && MaxMin.maxs[22].w > mx.w) mx = MaxMin.maxs[22];
      if (has_second_side_chamfer && MaxMin.mins[22].w < mn.w) mn = MaxMin.mins[22];
      if ((Reversed ? MaxMin.maxs[16].z > journal_hole_radius : MaxMin.maxs[16].z < -journal_hole_radius) && MaxMin.maxs[16].w > mx.w) mx = MaxMin.maxs[16];
      if ((Reversed ? MaxMin.mins[16].z > journal_hole_radius : MaxMin.mins[16].z < -journal_hole_radius) && MaxMin.mins[16].w < mn.w) mn = MaxMin.mins[16];
      draw_line(vector{350, 15, 250 + (mx.y - cam_offset_y) * cam_factor, 115 - (Reversed ? -mx.x : mx.x) * cam_factor}, brush<color::black>{}, 1.f);
      draw_rectangle({vector{300, 5, 400, 25}, 2.5f, 2.5f}, brush(get_color(mx.w)));
      draw_text({300, 5, 400, 25}, font<15>{}, std::format(L"{:.2f}", mx.w));
      draw_line({150, 15}, {250 + (mn.y - cam_offset_y) * cam_factor, 115 - (Reversed ? -mn.x : mn.x) * cam_factor}, brush<color::black>{}, 1.f);
      draw_rectangle({vector{100, 5, 200, 25}, 2.5f, 2.5f}, brush(get_color(mn.w)));
      draw_text({100, 5, 200, 25}, font<15>{}, std::format(L"{:.2f}", mn.w));
      // BOT
      mx = MaxMin.maxs[9], mn = MaxMin.mins[9];
      if (has_second_side_chamfer && MaxMin.maxs[10].w > mx.w) mx = MaxMin.maxs[10];
      if (has_second_side_chamfer && MaxMin.mins[10].w < mn.w) mn = MaxMin.mins[10];
      if ((Reversed ? MaxMin.maxs[4].z > journal_hole_radius : MaxMin.maxs[4].z < -journal_hole_radius) && MaxMin.maxs[4].w > mx.w) mx = MaxMin.maxs[4];
      if ((Reversed ? MaxMin.mins[4].z > journal_hole_radius : MaxMin.mins[4].z < -journal_hole_radius) && MaxMin.mins[4].w < mn.w) mn = MaxMin.mins[4];
      draw_line({350, 215}, {250 + (mx.y - cam_offset_y) * cam_factor, 115 - (Reversed ? -mx.x : mx.x) * cam_factor}, brush<color::black>{}, 1.f);
      draw_rectangle({vector{300, 205, 400, 225}, 2.5f, 2.5f}, brush(get_color(mx.w)));
      draw_text({300, 205, 400, 225}, font<15>{}, std::format(L"{:.2f}", mx.w));
      draw_line({150, 215}, {250 + (mn.y - cam_offset_y) * cam_factor, 115 - (Reversed ? -mn.x : mn.x) * cam_factor}, brush<color::black>{}, 1.f);
      draw_rectangle({vector{100, 205, 200, 225}, 2.5f, 2.5f}, brush(get_color(mn.w)));
      draw_text({100, 205, 200, 225}, font<15>{}, std::format(L"{:.2f}", mn.w));
      // 矢印とか
      draw_text({250, 5, 290, 25}, font<15>{}, L"↓A");
      draw_text({250, 105, 290, 125}, font<15>{}, L"↑B");
      draw_text({210, 105, 250, 125}, font<15>{}, L"↓C");
      draw_text({210, 205, 250, 225}, font<15>{}, L"↑D");
      draw_text({5, 105, 45, 125}, font<15>{}, L"E→");
      draw_text({455, 105, 495, 125}, font<15>{}, L"←H");
      draw_text({5, 50, 45, 70}, font<15>{}, L"Top");
      draw_text({5, 160, 45, 180}, font<15>{}, L"Bot");
      cam.end_draw();
      bitmap bmp = cam.rotate(-90);
      main::begin_draw();
      draw_bitmap({10, 100, 240, 600}, bmp);
      draw_rectangle({10, 100, 240, 600}, brush<color::black>{}, 1.f);
      main::end_draw();
    }

    { // 左サイド
      auto cam = camera(500, 230, 8);
      cam.offset.x = -cam_offset_y;
      cam.offset.z = -10000.f;
      cam.rotation = Reversed ? vector{0, 0, -pi / 2} : vector{0, pi, pi / 2};
      cam.orthographic = true;
      cam.factor = cam_factor;
      cam.update();
      cb_camera.from(list<>::asref(cam.view, cam.view_proj));
      cam.begin_render(color::white);
      render_vertices(sb_vertices, sb_margins, cb_world, cb_camera, cb_options);
      cam.end_render();
      cam.begin_draw();
      // TOP
      vector mx = MaxMin.maxs[18], mn = MaxMin.mins[18];
      if (has_second_side_chamfer && MaxMin.maxs[19].w > mx.w) mx = MaxMin.maxs[19];
      if (has_second_side_chamfer && MaxMin.mins[19].w < mn.w) mn = MaxMin.mins[19];
      if ((Reversed ? MaxMin.maxs[16].z < -journal_hole_radius : MaxMin.maxs[16].z > journal_hole_radius) && MaxMin.maxs[16].w > mx.w) mx = MaxMin.maxs[16];
      if ((Reversed ? MaxMin.mins[16].z < -journal_hole_radius : MaxMin.mins[16].z > journal_hole_radius) && MaxMin.mins[16].w < mn.w) mn = MaxMin.mins[16];
      draw_line({350, 215}, {250 + (mx.y - cam_offset_y) * cam_factor, 115 - (Reversed ? mx.x : -mx.x) * cam_factor}, brush<color::black>{}, 1.f);
      draw_rectangle({vector{300, 205, 400, 225}, 2.5f, 2.5f}, brush(get_color(mx.w)));
      draw_text({300, 205, 400, 225}, font<15>{}, std::format(L"{:.2f}", mx.w));
      draw_line({150, 215}, {250 + (mn.y - cam_offset_y) * cam_factor, 115 - (Reversed ? mn.x : -mn.x) * cam_factor}, brush<color::black>{}, 1.f);
      draw_rectangle({vector{100, 205, 200, 225}, 2.5f, 2.5f}, brush(get_color(mn.w)));
      draw_text({100, 205, 200, 225}, font<15>{}, std::format(L"{:.2f}", mn.w));
      // BOT
      mx = MaxMin.maxs[6], mn = MaxMin.mins[6];
      if (has_second_side_chamfer && MaxMin.maxs[7].w > mx.w) mx = MaxMin.maxs[7];
      if (has_second_side_chamfer && MaxMin.mins[7].w < mn.w) mn = MaxMin.mins[7];
      if ((Reversed ? MaxMin.maxs[4].z < -journal_hole_radius : MaxMin.maxs[4].z > journal_hole_radius) && MaxMin.maxs[4].w > mx.w) mx = MaxMin.maxs[4];
      if ((Reversed ? MaxMin.mins[4].z < -journal_hole_radius : MaxMin.mins[4].z > journal_hole_radius) && MaxMin.mins[4].w < mn.w) mn = MaxMin.mins[4];
      draw_line({350, 15}, {250 + (mx.y - cam_offset_y) * cam_factor, 115 - (Reversed ? mx.x : -mx.x) * cam_factor}, brush<color::black>{}, 1.f);
      draw_rectangle({vector{300, 5, 400, 25}, 2.5f, 2.5f}, brush(get_color(mx.w)));
      draw_text({300, 5, 400, 25}, font<15>{}, std::format(L"{:.2f}", mx.w));
      draw_line({150, 15}, {250 + (mn.y - cam_offset_y) * cam_factor, 115 - (Reversed ? mn.x : -mn.x) * cam_factor}, brush<color::black>{}, 1.f);
      draw_rectangle({vector{100, 5, 200, 25}, 2.5f, 2.5f}, brush(get_color(mn.w)));
      draw_text({100, 5, 200, 25}, font<15>{}, std::format(L"{:.2f}", mn.w));
      // 矢印とか
      draw_text({5, 50, 45, 70}, font<15>{}, L"Bot");
      draw_text({5, 160, 45, 180}, font<15>{}, L"Top");
      cam.end_draw();
      bitmap bmp = cam.rotate(-90);
      main::begin_draw();
      draw_bitmap({10 + 230 * 5, 100, 240 + 230 * 5, 600}, bmp);
      draw_rectangle({10 + 230 * 5, 100, 240 + 230 * 5, 600}, brush<color::black>{}, 1.f);
      main::end_draw();
    }

    { // TOP基準面
      auto cam = camera(230, 500, 8);
      cam.offset.y = cam_offset_y;
      cam.offset.z = -10000.f;
      cam.rotation = Reversed ? vector{0, pi / 2, 0} : vector{0, -pi / 2, 0};
      cam.orthographic = true;
      cam.factor = cam_factor;
      cam.update();
      cb_camera.from(list<>::asref(cam.view, cam.view_proj));
      cam.begin_render(color::white);
      render_vertices(sb_vertices, sb_margins, cb_world, cb_camera, cb_options);
      cam.end_render();
      cam.begin_draw();
      vector mx = MaxMin.maxs[12], mn = MaxMin.mins[12];
      if (MaxMin.mins[13].w < mn.w) mn = MaxMin.mins[13];
      draw_line({60, 15}, {115 - (Reversed ? -mn.z : mn.z) * cam_factor, 250 - (mn.y - cam_offset_y) * cam_factor}, brush<color::black>{}, 1.f);
      draw_rectangle({vector{10, 5, 110, 25}, 2.5f, 2.5f}, brush(get_color(mn.w)));
      draw_text({10, 5, 110, 25}, font<15>{}, std::format(L"{:.2f}", mn.w));
      draw_line({170, 15}, {115 - (Reversed ? -mx.z : mx.z) * cam_factor, 250 - (mx.y - cam_offset_y) * cam_factor}, brush<color::black>{}, 1.f);
      draw_rectangle({vector{120, 5, 220, 25}, 2.5f, 2.5f}, brush(get_color(mx.w)));
      draw_text({120, 5, 220, 25}, font<15>{}, std::format(L"{:.2f}", mx.w));
      // 矢印とか
      auto cy = 250 + cam_offset_y * cam_factor;
      draw_text({95, cy - 40, 135, cy - 20}, font<15>(), L"↑F");
      draw_text({95, cy + 20, 135, cy + 40}, font<15>(), L"↓G");
      draw_text({5, 475, 65, 495}, font<15>{}, L"矢視Ａ");
      cam.end_draw();
      main::begin_draw();
      draw_bitmap({10 + 230, 100, 240 + 230, 600}, cam);
      draw_rectangle({10 + 230, 100, 240 + 230, 600}, brush<color::black>{}, 1.f);
      main::end_draw();
    }

    { // TOP内股
      auto cam = camera(230, 500, 8);
      cam.offset.y = cam_offset_y;
      cam.rotation = Reversed ? vector{0, -pi / 2, 0} : vector{0, pi / 2, 0};
      cam.orthographic = true;
      cam.factor = cam_factor;
      cam.update();
      cb_camera.from(list<>::asref(cam.view, cam.view_proj));
      cam.begin_render(color::white);
      render_vertices(sb_vertices, sb_margins, cb_world, cb_camera, cb_options);
      cam.end_render();
      cam.begin_draw();
      vector mx = MaxMin.maxs[14], mn = MaxMin.mins[14];
      draw_line({60, 15}, {115 + (Reversed ? -mn.z : mn.z) * cam_factor, 250 - (mn.y - cam_offset_y) * cam_factor}, brush<color::black>{}, 1.f);
      draw_rectangle({vector{10, 5, 110, 25}, 2.5f, 2.5f}, brush(get_color(mn.w)));
      draw_text({10, 5, 110, 25}, font<15>{}, std::format(L"{:.2f}", mn.w));
      draw_line({170, 15}, {115 + (Reversed ? -mx.z : mx.z) * cam_factor, 250 - (mx.y - cam_offset_y) * cam_factor}, brush<color::black>{}, 1.f);
      draw_rectangle({vector{120, 5, 220, 25}, 2.5f, 2.5f}, brush(get_color(mx.w)));
      draw_text({120, 5, 220, 25}, font<15>{}, std::format(L"{:.2f}", mx.w));
      draw_text({5, 475, 65, 495}, font<15>{}, L"矢視Ｂ");
      cam.end_draw();
      main::begin_draw();
      draw_bitmap({10 + 230 * 2, 100, 240 + 230 * 2, 600}, cam);
      draw_rectangle({10 + 230 * 2, 100, 240 + 230 * 2, 600}, brush<color::black>{}, 1.f);
      main::end_draw();
    }

    { // BOT内股
      auto cam = camera(230, 500, 8);
      cam.offset.y = cam_offset_y;
      cam.rotation = Reversed ? vector{0, pi / 2, 0} : vector{0, -pi / 2, 0};
      cam.orthographic = true;
      cam.factor = cam_factor;
      cam.update();
      cb_camera.from(list<>::asref(cam.view, cam.view_proj));
      cam.begin_render(color::white);
      render_vertices(sb_vertices, sb_margins, cb_world, cb_camera, cb_options);
      cam.end_render();
      cam.begin_draw();
      vector mx = MaxMin.maxs[2], mn = MaxMin.mins[2];
      draw_line({60, 15}, {115 - (Reversed ? -mn.z : mn.z) * cam_factor, 250 - (mn.y - cam_offset_y) * cam_factor}, brush<color::black>{}, 1.f);
      draw_rectangle({vector{10, 5, 110, 25}, 2.5f, 2.5f}, brush(get_color(mn.w)));
      draw_text({10, 5, 110, 25}, font<15>{}, std::format(L"{:.2f}", mn.w));
      draw_line({170, 15}, {115 - (Reversed ? -mx.z : mx.z) * cam_factor, 250 - (mx.y - cam_offset_y) * cam_factor}, brush<color::black>{}, 1.f);
      draw_rectangle({vector{120, 5, 220, 25}, 2.5f, 2.5f}, brush(get_color(mx.w)));
      draw_text({120, 5, 220, 25}, font<15>{}, std::format(L"{:.2f}", mx.w));
      draw_text({5, 475, 65, 495}, font<15>{}, L"矢視Ｃ");
      cam.end_draw();
      main::begin_draw();
      draw_bitmap({10 + 230 * 3, 100, 240 + 230 * 3, 600}, cam);
      draw_rectangle({10 + 230 * 3, 100, 240 + 230 * 3, 600}, brush<color::black>{}, 1.f);
      main::end_draw();
    }

    { // BOT基準面
      auto cam = camera(230, 500, 8);
      cam.offset.y = cam_offset_y;
      cam.offset.z = -10000.f;
      cam.rotation = Reversed ? vector{0, -pi / 2, 0} : vector{0, pi / 2, 0};
      cam.orthographic = true;
      cam.factor = cam_factor;
      cam.update();
      cb_camera.from(list<>::asref(cam.view, cam.view_proj));
      cam.begin_render(color::white);
      render_vertices(sb_vertices, sb_margins, cb_world, cb_camera, cb_options);
      cam.end_render();
      cam.begin_draw();
      vector mx = MaxMin.maxs[0], mn = MaxMin.mins[0];
      if (MaxMin.mins[1].w < mn.w) mn = MaxMin.mins[1];
      draw_line({60, 15}, {115 + (Reversed ? -mn.z : mn.z) * cam_factor, 250 - (mn.y - cam_offset_y) * cam_factor}, brush<color::black>{}, 1.f);
      draw_rectangle({vector{10, 5, 110, 25}, 2.5f, 2.5f}, brush(get_color(mn.w)));
      draw_text({10, 5, 110, 25}, font<15>{}, std::format(L"{:.2f}", mn.w));
      draw_line({170, 15}, {115 + (Reversed ? -mx.z : mx.z) * cam_factor, 250 - (mx.y - cam_offset_y) * cam_factor}, brush<color::black>{}, 1.f);
      draw_rectangle({vector{120, 5, 220, 25}, 2.5f, 2.5f}, brush(get_color(mx.w)));
      draw_text({120, 5, 220, 25}, font<15>{}, std::format(L"{:.2f}", mx.w));
      draw_text({5, 475, 65, 495}, font<15>{}, L"矢視Ｄ");
      cam.end_draw();
      main::begin_draw();
      draw_bitmap({10 + 230 * 4, 100, 240 + 230 * 4, 600}, cam);
      draw_rectangle({10 + 230 * 4, 100, 240 + 230 * 4, 600}, brush<color::black>{}, 1.f);
      main::end_draw();
    }

    { // R面
      auto cam = camera(230, 250, 8);
      cam.offset.z = -10000.f;
      cam.rotation = Reversed ? vector{-pi / 2, pi / 2, 0} : vector{-pi / 2, -pi / 2, 0};
      cam.orthographic = true;
      cam.factor = cam_factor;
      cam.update();
      cb_camera.from(list<>::asref(cam.view, cam.view_proj));
      cam.begin_render(color::white);
      render_vertices(sb_vertices, sb_margins, cb_world, cb_camera, cb_options);
      cam.end_render();
      cam.begin_draw();
      vector mx = MaxMin.maxs[4], mn = MaxMin.mins[4];
      if (MaxMin.maxs[16].w > mx.w) mx = MaxMin.maxs[16];
      if (MaxMin.mins[16].w < mn.w) mn = MaxMin.mins[16];
      draw_line({60, 235}, {115 - (Reversed ? -mn.z : mn.z) * cam_factor, 125 - (Reversed ? -mn.x : mn.x) * cam_factor}, brush<color::black>{}, 1.f);
      draw_rectangle({vector{10, 225, 110, 245}, 2.5f, 2.5f}, brush(get_color(mn.w)));
      draw_text({10, 225, 110, 245}, font<15>{}, std::format(L"{:.2f}", mn.w));
      draw_line({170, 235}, {115 - (Reversed ? -mx.z : mx.z) * cam_factor, 125 - (Reversed ? -mx.x : mx.x) * cam_factor}, brush<color::black>{}, 1.f);
      draw_rectangle({vector{120, 225, 220, 245}, 2.5f, 2.5f}, brush(get_color(mx.w)));
      draw_text({120, 225, 220, 245}, font<15>{}, std::format(L"{:.2f}", mx.w));
      draw_text({5, 5, 65, 25}, font<15>{}, L"矢視Ｅ");
      cam.end_draw();
      main::begin_draw();
      draw_bitmap({10, 600, 240, 850}, cam);
      draw_rectangle({10, 600, 240, 850}, brush<color::black>{}, 1.f);
      main::end_draw();
    }

    { // J穴ピン側
      auto cam = camera(230, 250, 8);
      cam.rotation = Reversed ? vector{-pi / 2, pi / 2, 0} : vector{-pi / 2, -pi / 2, 0};
      cam.orthographic = true;
      cam.factor = cam_factor;
      cam.update();
      cb_camera.from(list<>::asref(cam.view, cam.view_proj));
      cam.begin_render(color::white);
      render_vertices(sb_vertices, sb_margins, cb_world, cb_camera, cb_options);
      cam.end_render();
      cam.begin_draw();
      vector mx = {-1, -1, -1, -1e10}, mn = {-1, -1, -1, 1e10};
      if (auto& t = MaxMin.maxs[3]; t.y > 0 && t.w > mx.w) mx = t;
      if (auto& t = MaxMin.mins[3]; t.y > 0 && t.w < mn.w) mn = t;
      if (auto& t = MaxMin.maxs[15]; t.y > 0 && t.w > mx.w) mx = t;
      if (auto& t = MaxMin.mins[15]; t.y > 0 && t.w < mn.w) mn = t;
      if (mn.y > 0) {
        draw_line({60, 235}, {115 - (Reversed ? -mn.z : mn.z) * cam_factor, 125 - (Reversed ? -mn.x : mn.x) * cam_factor}, brush<color::black>{}, 1.f);
        draw_rectangle({vector{10, 225, 110, 245}, 2.5f, 2.5f}, brush(get_color(mn.w)));
        draw_text({10, 225, 110, 245}, font<15>{}, std::format(L"{:.2f}", mn.w));
      }
      if (mx.y > 0) {
        draw_line({170, 235}, {115 - (Reversed ? -mx.z : mx.z) * cam_factor, 125 - (Reversed ? -mx.x : mx.x) * cam_factor}, brush<color::black>{}, 1.f);
        draw_rectangle({vector{120, 225, 220, 245}, 2.5f, 2.5f}, brush(get_color(mx.w)));
        draw_text({120, 225, 220, 245}, font<15>{}, std::format(L"{:.2f}", mx.w));
      }
      draw_text({5, 5, 65, 25}, font<15>{}, L"矢視Ｆ");
      cam.end_draw();
      main::begin_draw();
      draw_bitmap({10 + 230, 600, 240 + 230, 850}, cam);
      draw_rectangle({10 + 230, 600, 240 + 230, 850}, brush<color::black>{}, 1.f);
      main::end_draw();
    }

    { // J穴R側
      auto cam = camera(230, 250, 8);
      cam.rotation = Reversed ? vector{pi / 2, -pi / 2, 0} : vector{pi / 2, pi / 2, 0};
      cam.orthographic = true;
      cam.factor = cam_factor;
      cam.update();
      cb_camera.from(list<>::asref(cam.view, cam.view_proj));
      cam.begin_render(color::white);
      render_vertices(sb_vertices, sb_margins, cb_world, cb_camera, cb_options);
      cam.end_render();
      cam.begin_draw();
      vector mx = {1, 1, 1, -1e10}, mn = {1, 1, 1, 1e10};
      if (auto& t = MaxMin.maxs[3]; t.y < 0 && t.w > mx.w) mx = t;
      if (auto& t = MaxMin.mins[3]; t.y < 0 && t.w < mn.w) mn = t;
      if (auto& t = MaxMin.maxs[15]; t.y < 0 && t.w > mx.w) mx = t;
      if (auto& t = MaxMin.mins[15]; t.y < 0 && t.w < mn.w) mn = t;
      if (mn.y < 0) {
        draw_line({60, 235}, {115 + (Reversed ? -mn.z : mn.z) * cam_factor, 125 - (Reversed ? -mn.x : mn.x) * cam_factor}, brush<color::black>{}, 1.f);
        draw_rectangle({vector{10, 225, 110, 245}, 2.5f, 2.5f}, brush(get_color(mn.w)));
        draw_text({10, 225, 110, 245}, font<15>{}, std::format(L"{:.2f}", mn.w));
      }
      if (mx.y < 0) {
        draw_line({170, 235}, {115 + (Reversed ? -mx.z : mx.z) * cam_factor, 125 - (Reversed ? -mx.x : mx.x) * cam_factor}, brush<color::black>{}, 1.f);
        draw_rectangle({vector{120, 225, 220, 245}, 2.5f, 2.5f}, brush(get_color(mx.w)));
        draw_text({120, 225, 220, 245}, font<15>{}, std::format(L"{:.2f}", mx.w));
      }
      draw_text({5, 5, 65, 25}, font<15>{}, L"矢視Ｇ");
      cam.end_draw();
      main::begin_draw();
      draw_bitmap({10 + 230 * 2, 600, 240 + 230 * 2, 850}, cam);
      draw_rectangle({10 + 230 * 2, 600, 240 + 230 * 2, 850}, brush<color::black>{}, 1.f);
      main::end_draw();
    }

    { // ピントップ
      auto cam = camera(230, 250, 8);
      cam.offset.z = -10000.f;
      cam.rotation = Reversed ? vector{pi / 2, -pi / 2, 0} : vector{pi / 2, pi / 2, 0};
      cam.orthographic = true;
      cam.factor = cam_factor;
      cam.update();
      cb_camera.from(list<>::asref(cam.view, cam.view_proj));
      cam.begin_render(color::white);
      render_vertices(sb_vertices, sb_margins, cb_world, cb_camera, cb_options);
      cam.end_render();
      cam.begin_draw();
      vector mx = MaxMin.maxs[5], mn = MaxMin.mins[5];
      if (MaxMin.maxs[17].w > mx.w) mx = MaxMin.maxs[17];
      if (MaxMin.mins[17].w < mn.w) mn = MaxMin.mins[17];
      if (MaxMin.maxs[24].w > mx.w) mx = MaxMin.maxs[24];
      if (MaxMin.mins[24].w < mn.w) mn = MaxMin.mins[24];
      draw_line({60, 235}, {115 + (Reversed ? -mn.z : mn.z) * cam_factor, 125 - (Reversed ? -mn.x : mn.x) * cam_factor}, brush<color::black>{}, 1.f);
      draw_rectangle({vector{10, 225, 110, 245}, 2.5f, 2.5f}, brush(get_color(mn.w)));
      draw_text({10, 225, 110, 245}, font<15>{}, std::format(L"{:.2f}", mn.w));
      // draw_line({170, 235}, {115 - (Reversed ? -mx.z : mx.z) * cam_factor, 125 - (Reversed ? -mx.x : mx.x) * cam_factor}, brush<color::black>{}, 1.f);
      // draw_rectangle({vector{120, 225, 220, 245}, 2.5f, 2.5f}, brush(get_color(mx.w)));
      // draw_text({120, 225, 220, 245}, font<15>{}, std::format(L"{:.2f}", mx.w));
      draw_text({5, 5, 65, 25}, font<15>{}, L"矢視Ｈ");
      cam.end_draw();
      main::begin_draw();
      draw_bitmap({10 + 230 * 3, 600, 240 + 230 * 3, 850}, cam);
      draw_rectangle({10 + 230 * 3, 600, 240 + 230 * 3, 850}, brush<color::black>{}, 1.f);
      main::end_draw();
    }

    // 成績表のためのＵＩを表示する
    control::show(1);
    // メッセージループ
    while (ResultMode && main::update) {}
    // 成績表のためのＵＩを非表示にする
    control::hide(1);
    // 画面サイズを元に戻す
    main::resize(app_width, app_height);
    // 芯合わせモードのＵＩを表示する
    control::show(0);
    // 再描画を行う
    render_all();
    key::escape.down = [] { if (yes(L"終了しますか？")) main::terminate(); };
  } catch (const std::exception& E) { except(E).receive(); }
}


int wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int) {
  main::rename(app_name);
  main::resize(app_width, app_height);

  Camera = camera(1920, 1080, 8);
  Camera.orthographic = true;
  Camera.factor = 0.5f;
  Camera.offset.z = -10000;
  Camera.rotation.z = -float(pi / 2);
  Camera.update();
  Camera.begin_render(color::white);
  Camera.end_render();


  Coord = coordinator(100, 100, 8);
  Coord(Camera.rotation);


  static_assert(vassignable<list<xmatrix, xmatrix>&, decltype(list<>::asref(Camera.view, Camera.view_proj))>);
  static_assert(constructible<constant_buffer<list<xmatrix, xmatrix>>, list<xmatrix&, xmatrix&>>);
  cb_camera = constant_buffer<list<xmatrix, xmatrix>>(list<>::asref(Camera.view, Camera.view_proj));
  cb_world = decltype(cb_world)(xv_identity);
  cb_options = decltype(cb_options)(list<unsigned, float, unsigned long long>{0, 1.f, 0});


  ui_button_cmt = button(0, {5, 5, 45, 25}, L"CMT");
  ui_button_cmt.input = [](const button& This) noexcept {
    try {
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
        calc_maxmin(sb_vertices, ub_margins, cb_world, MaxMin);
        ui_label_margins[0].text(std::format(L"{:.3f}  ", apply(min, projector(MaxMin.mins, &vector::w, make_sequence<25>{}))));
        ui_label_margins[1].text(std::format(L"{:.3f}  ", apply(min, projector(MaxMin.mins, &vector::w, sequence<4, 6, 9, 16, 18, 21>{}))));
        ui_label_margins[2].text(std::format(L"{:.3f}  ", apply(max, projector(MaxMin.maxs, &vector::w, sequence<4, 6, 9, 16, 18, 21>{}))));
      }
      render_all();
      cmt_path = file;
    } catch (const std::exception& E) { except(E).receive(); }
  };


  ui_button_cad = button(0, {5, 30, 45, 50}, L"CAD");
  ui_button_cad.input = [](const button& This) noexcept {
    try {
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
        calc_maxmin(sb_vertices, ub_margins, cb_world, MaxMin);
        ui_label_margins[0].text(std::format(L"{:.3f}  ", apply(min, projector(MaxMin.mins, &vector::w, make_sequence<25>{}))));
        ui_label_margins[1].text(std::format(L"{:.3f}  ", apply(min, projector(MaxMin.mins, &vector::w, sequence<4, 6, 9, 16, 18, 21>{}))));
        ui_label_margins[2].text(std::format(L"{:.3f}  ", apply(max, projector(MaxMin.maxs, &vector::w, sequence<4, 6, 9, 16, 18, 21>{}))));
      }
      render_all();
    } catch (const std::exception& E) { except(E).receive(); }
  };


  ui_label_cmt = label(0, {50, 5, 300, 25}, L"");
  ui_label_cad = label(0, {50, 30, 300, 50}, L"");


  ui_radiobutton_hide = radiobutton(0, {305, 5, 405, 75}, list{L"全表示", L"＋Ｘ側を非表示", L"－Ｘ側を非表示"});
  ui_radiobutton_hide.input = [](const radiobutton& This) noexcept {
    try {
      cb_options.from(list<>::asref(This.state, ui_checkbox_alpha.state ? 0.7f : 1.0f, 0));
      render_all();
    } catch (const std::exception& E) { except(E).receive(); }
  };


  ui_checkbox_reverse = checkbox(0, {410, 5, 510, 25}, L"＋ＸがBOT側");
  ui_checkbox_reverse.input = [](const checkbox& This) noexcept {
    try {
      Reversed = This.state;
      if (AlignMode >= 0) AlignMode = -1, ok(L"自動芯合わせを中断しました。");
      if (stl_facets.empty()) return;
      ub_facets = facets_from_stl(stl_facets, This.state);
      sb_facets.from(ub_facets);
      calc_margin(sb_facets, sb_vertices, cb_world, ub_margins);
      sb_margins.from(ub_margins);
      calc_maxmin(sb_vertices, ub_margins, cb_world, MaxMin);
      render_all();
    } catch (const std::exception& E) { except(E).receive(); }
  };


  ui_checkbox_alpha = checkbox(0, {410, 30, 510, 50}, L"半透明");
  ui_checkbox_alpha.input = [](const checkbox& This) noexcept {
    try {
      cb_options.from(list<>::asref(ui_radiobutton_hide.state, This.state ? 0.7f : 1.0f, 0));
      render_all();
    } catch (const std::exception& E) { except(E).receive(); }
  };

  ui_checkbox_autoalignmode = checkbox(0, {410, 55, 510, 75}, L"オートリピート");

  ui_progressbar_align = progressbar(0, {820, 5, 945, 15});

  ui_button_align = button(0, {820, 20, 945, 75}, L"自動芯合わせ", WS_BORDER);
  ui_button_align.input = [](const button& This) noexcept { AlignMode = AlignMode < 0 ? 0 : -2; };

  ui_label_rotation = label(0, {810, 85, 955, 105}, L"回転 [deg]", SS_CENTER | SS_CENTERIMAGE);
  ui_label_rot_x = label(0, {810, 110, 850, 130}, L"X", SS_CENTER | SS_CENTERIMAGE);
  ui_label_rot_y = label(0, {810, 135, 850, 155}, L"Y", SS_CENTER | SS_CENTERIMAGE);
  ui_label_rot_z = label(0, {810, 160, 850, 180}, L"Z", SS_CENTER | SS_CENTERIMAGE);

  ui_valuebox_rot[0] = valuebox(0, {855, 110, 955, 130}, L"0", SS_RIGHT);
  ui_valuebox_rot[1] = valuebox(0, {855, 135, 955, 155}, L"0", SS_RIGHT);
  ui_valuebox_rot[2] = valuebox(0, {855, 160, 955, 180}, L"0", SS_RIGHT);

  ui_valuebox_rot[0].input = [](const valuebox& This) noexcept {
    update_world();
    if (sb_facets && sb_vertices) {
      calc_margin(sb_facets, sb_vertices, cb_world, ub_margins);
      sb_margins.from(ub_margins);
      calc_maxmin(sb_vertices, ub_margins, cb_world, MaxMin);
      ui_label_margins[0].text(std::format(L"{:.3f}  ", apply(min, projector(MaxMin.mins, &vector::w, make_sequence<25>{}))));
      ui_label_margins[1].text(std::format(L"{:.3f}  ", apply(min, projector(MaxMin.mins, &vector::w, sequence<4, 6, 9, 16, 18, 21>{}))));
      ui_label_margins[2].text(std::format(L"{:.3f}  ", apply(max, projector(MaxMin.maxs, &vector::w, sequence<4, 6, 9, 16, 18, 21>{}))));
    }
    render_all(); };
  ui_valuebox_rot[1].input = ui_valuebox_rot[0].input;
  ui_valuebox_rot[2].input = ui_valuebox_rot[0].input;
  ui_valuebox_rot[0].killfocus = ui_valuebox_rot[0].input;
  ui_valuebox_rot[1].killfocus = ui_valuebox_rot[0].input;
  ui_valuebox_rot[2].killfocus = ui_valuebox_rot[0].input;
  ui_valuebox_rot[0].intofocus = [](const valuebox& This) noexcept {
    if (AlignMode >= 0) AlignMode = -1, ok(L"自動芯合わせを中断しました。");
  };
  ui_valuebox_rot[1].intofocus = ui_valuebox_rot[0].intofocus;
  ui_valuebox_rot[2].intofocus = ui_valuebox_rot[0].intofocus;

  ui_label_offset = label(0, {810, 190, 955, 210}, L"移動 [mm]", SS_CENTER | SS_CENTERIMAGE);
  ui_label_off_x = label(0, {810, 215, 850, 235}, L"X", SS_CENTER | SS_CENTERIMAGE);
  ui_label_off_y = label(0, {810, 240, 850, 260}, L"Y", SS_CENTER | SS_CENTERIMAGE);
  ui_label_off_z = label(0, {810, 265, 850, 285}, L"Z", SS_CENTER | SS_CENTERIMAGE);

  ui_valuebox_off[0] = valuebox(0, {855, 215, 955, 235}, L"0", SS_RIGHT);
  ui_valuebox_off[1] = valuebox(0, {855, 240, 955, 260}, L"0", SS_RIGHT);
  ui_valuebox_off[2] = valuebox(0, {855, 265, 955, 285}, L"0", SS_RIGHT);

  ui_valuebox_off[0].input = ui_valuebox_rot[0].input;
  ui_valuebox_off[1].input = ui_valuebox_rot[0].input;
  ui_valuebox_off[2].input = ui_valuebox_rot[0].input;
  ui_valuebox_off[0].killfocus = ui_valuebox_rot[0].input;
  ui_valuebox_off[1].killfocus = ui_valuebox_rot[0].input;
  ui_valuebox_off[2].killfocus = ui_valuebox_rot[0].input;
  ui_valuebox_off[0].intofocus = ui_valuebox_rot[0].intofocus;
  ui_valuebox_off[1].intofocus = ui_valuebox_rot[0].intofocus;
  ui_valuebox_off[2].intofocus = ui_valuebox_rot[0].intofocus;

  ui_label_min_over = label(0, {810, 295, 890, 315}, L"　　最小取代", SS_CENTER | SS_CENTERIMAGE);
  ui_label_min_side = label(0, {810, 320, 890, 340}, L"外周最小取代", SS_CENTER | SS_CENTERIMAGE);
  ui_label_max_side = label(0, {810, 345, 890, 365}, L"外周最大取代", SS_CENTER | SS_CENTERIMAGE);
  ui_label_timer = label(0, {810, 370, 890, 390}, L"計算時間", SS_CENTER | SS_CENTERIMAGE);

  ui_label_margins[0] = label(0, {895, 295, 955, 315}, L"", SS_RIGHT);
  ui_label_margins[1] = label(0, {895, 320, 955, 340}, L"", SS_RIGHT);
  ui_label_margins[2] = label(0, {895, 345, 955, 365}, L"", SS_RIGHT);
  ui_label_margins[3] = label(0, {895, 370, 955, 390}, L"", SS_CENTER);


  ui_button_result = button(0, {820, 495, 945, 535}, L"成績表モード");
  ui_button_result.input = [](const button&) noexcept {
    if (!(sb_facets && sb_vertices)) return;
    ResultMode = true;
    control::hide(0);
  };


  control::show(0);

  ywlib_debug;
  render_all();
  ywlib_debug;

  mouse::moved = [](int dx, int dy) {
    if (ResultMode) return;
    auto x = mouse::x - dx, y = mouse::y - dy;
    if (5 <= x && x < 805 && 85 <= y && y < 535) {
      bool b = false;
      if (mouse::left) {
        Camera.rotation.y -= dx * 0.003f / Camera.factor;
        Camera.rotation.x += dy * 0.003f / Camera.factor;
        if (Camera.rotation.x < -pi / 2) Camera.rotation.x = float(-pi / 2);
        else if (pi / 2 < Camera.rotation.x) Camera.rotation.x = float(pi / 2);
        Coord(Camera.rotation);
        b = true;
      } else if (mouse::middle) {
        Camera.offset.x += 2.4f * dx / Camera.factor;
        Camera.offset.y += 2.4f * dy / Camera.factor;
        b = true;
      }
      if (b) {
        Camera.update();
        cb_camera.from(list<>::asref(Camera.view, Camera.view_proj));
        render_all();
      }
    }
  };

  mouse::wheeled = [](int delta) {
    if (ResultMode) return;
    if (5 <= mouse::x && mouse::x < 805 && 85 <= mouse::y && mouse::y < 535) {
      if (delta > 0) Camera.factor *= 1.2f;
      else Camera.factor /= 1.2f;
      Camera.update();
      cb_camera.from(list<>::asref(Camera.view, Camera.view_proj));
      render_all();
    }
  };

  key::escape.down = [] {
    yes(L"終了しますか？") ? main::terminate() : void();
  };

  while (main::update) {
    // エスケープキーで終了の確認
    if (AlignMode != -1) align();
    if (ResultMode) show_result();
  }

  main::terminate();
}
