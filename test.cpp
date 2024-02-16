#define ywlib_debug
#include "ywstd.h"
using namespace yw;

// #include <thread>

////////////////////////////////////////////////////////////////////////////////
//  STRUCTURE

// using type1_panels = typepack<
//   panel<0, L"BOT基準面">,
//   panel<1, L"BOTバックチャンファ">,
//   panel<2, L"BOT内股">,
//   panel<3, L"BOTジャーナル穴">,
//   panel<4, L"BOT外周R">,
//   panel<5, L"BOTピントップ">,
//   panel<6, L"BOT左側面">,
//   panel<7, L"BOT左側面チャンファ1">,
//   panel<8, L"BOT左側面チャンファ2">,
//   panel<9, L"BOT右側面">,
//   panel<10, L"BOT右側面チャンファ1">,
//   panel<11, L"BOT右側面チャンファ2">,
//   panel<12, L"TOP基準面">,
//   panel<13, L"TOPバックチャンファ">,
//   panel<14, L"TOP内股">,
//   panel<15, L"TOPジャーナル穴">,
//   panel<16, L"TOP外周R">,
//   panel<17, L"TOPピントップ">,
//   panel<18, L"TOP左側面">,
//   panel<19, L"TOP左側面チャンファ1">,
//   panel<20, L"TOP左側面チャンファ2">,
//   panel<21, L"TOP右側面">,
//   panel<22, L"TOP右側面チャンファ1">,
//   panel<23, L"TOP右側面チャンファ2">,
//   panel<24, L"ピン">>;

struct facet {
  nat4 id;
  array<fat4, 3> a, b, c;
  array<fat4, 3> ab, bc, ca;
  array<fat4, 3> sq, nisq;
  vector plane;
  static unordered_buffer<facet> from_stl(const ff::stl& Stl);
};

struct vertex {
  vector p, n;
  static unordered_buffer<vertex> from_stl(const ff::stl& Stl);
};

struct margin {
  vector p, n;
  fat4 dist, vdist;
  nat4 id, over;
};

struct maxmin {
  array<vector, 26> maxs, mins;
  vector maximum, maximum_n;
  vector minimum, minimum_n;
  nat4 maximum_i, minimum_i;
};

////////////////////////////////////////////////////////////////////////////////
//  GLOBAL

path cmt_file, cad_file;
ui::label label_cmt_file, label_cad_file;
ui::radiobutton rb_flip;
ui::checkbox cb_reverse;
ui::checkbox cb_opacity;
ff::stl StlFacet, StlVertex;

bool ResultMode = false;
bool Reversed = false;

fat4 RadiusJH{}, HalfGap{}, HalfStroke{};
fat4 XMax{}, YMax{}, ZMax{}, XMin{}, YMin{}, ZMin{};

vector Degrees{}, Offsets{};

yw::bitmap Result;
yw::camera Camera;
ui::coordinator Coord;
ui::progressbar Progress;
ui::valuebox VB_Rot[3], VB_Mov[3];

ui::label lb_margin_all;
ui::label lb_margin_over;
ui::label lb_margin_over_x;
ui::label lb_margins[3];

ui::button bt_save_result;

maxmin MaxMin;

structured_buffer<facet> Facet;
structured_buffer<vertex> Vertex[2]; // regular, reverse
structured_buffer<margin> Margin;
unordered_buffer<margin> UB_Margin;

constant_buffer<xmatrix> CB_Matrix;
constant_buffer<list<xmatrix, xmatrix>> CB_Camera;
constant_buffer<list<nat4, fat4, nat8>> CB_Option;

static constexpr const vector rr{5, 85, 805, 535};
static constexpr const vector r2{87, 85, 723, 535};


////////////////////////////////////////////////////////////////////////////////
//  FORWARD DECLARATION

void calc_margin(const structured_buffer<facet>& Facet,
                 const structured_buffer<vertex>& Vertex,
                 const constant_buffer<xmatrix>& Matrix,
                 unordered_buffer<margin>& Margin);

void obtain_maxmin(const structured_buffer<margin>& Margin, maxmin& Maxmin);

void reverse(bool b);
void render();
void align();

void create_result();
void result_mode();

/**
 * @brief 反転させた頂点データ`Vertex[1]`を作成する。
 * @note  `Vertex[0]`を参照して、`Vertex[1]`を更新する。
 * ****************************************************************************/
void prepare_reversed_vertex() {
  constexpr stv1 hlsl = R"(
#pragma pack_matrix(row_major)
struct SB { float4 p, n; };
RWStructuredBuffer<SB> ub : register(u0);
  StructuredBuffer<SB> sb : register(t0);
[numthreads(1024, 1, 1)] void csmain(uint i : SV_DispatchThreadID) {
  ub[i].p = float4(-sb[i].p.x, sb[i].p.y, -sb[i].p.z, 1);
  ub[i].n = float4(-sb[i].n.x, sb[i].n.y, -sb[i].n.z, 0);
})";
  static auto gp = gpgpu<typepack<vertex>, typepack<vertex>>(hlsl);
  static unordered_buffer<vertex> ub{};
  if (!Vertex[0]) return;
  Vertex[1] = structured_buffer<vertex>(nullptr, Vertex[0].count);
  if (ub.count != Vertex[1].count) ub = unordered_buffer<vertex>(nullptr, Vertex[1].count);
  gp(Vertex[1].count, {ub}, {Vertex[0]}, {});
  Vertex[1].from(ub);
}

/**
 * @brief 反転フラグを元に頂点データを切り替える。
 ******************************************************************************/
inline structured_buffer<vertex>& get_vertex() noexcept { return Vertex[Reversed]; }

/**
 * @brief 取代ラベルの表示を更新する。
 ******************************************************************************/
inline void update_margin_labels(const maxmin& MaxMin) {
  fat4 a = apply([&](auto&&... as) { return yw::max(as.w...); }, projector(MaxMin.maxs, make_sequence<25>{}));
  fat4 b = apply([&](auto&&... as) { return yw::min(as.w...); }, projector(MaxMin.mins, make_sequence<25>{}));
  lb_margins[0].text(std::format(L"{:.3f}", MaxMin.minimum.w));
  lb_margins[1].text(std::format(L"{:.3f}", b));
  lb_margins[2].text(std::format(L"{:.3f}", a));
}

/**
 * @brief 回転角を更新する。
 ******************************************************************************/
inline void update_rotation(natt i, const str2& t) {
  for (natt j{}; j < 3; ++j) {
    if (i == j) {
      auto s = std::format(L"{:.4f}", stov<fat8>(t));
      VB_Rot[i].text(s), Degrees[i] = stov<fat4>(s);
    } else Degrees[i] = stov<fat4>(VB_Rot[i].text());
  }
  CB_Matrix.from([&](xmatrix& m) { xvworld(Offsets, xvradian(Degrees), m); });
  calc_margin(Facet, get_vertex(), CB_Matrix, UB_Margin), Margin.from(UB_Margin);
  obtain_maxmin(Margin, MaxMin), update_margin_labels(MaxMin), render();
}

////////////////////////////////////////////////////////////////////////////////
//  MAIN-WINDOW


void ywmain() {
  std::cout << source{} << std::endl, tiff(main::d3d_device->GetDeviceRemovedReason());
  main::rename(L"ACMA-TEST");
  std::cout << source{} << std::endl, tiff(main::d3d_device->GetDeviceRemovedReason());
  main::resize(960, 540);
  std::cout << source{} << std::endl, tiff(main::d3d_device->GetDeviceRemovedReason());
  key::escape.down = []() { ui::yes(L"終了しますか？") ? main::terminate() : void(); };
  std::cout << source{} << std::endl, tiff(main::d3d_device->GetDeviceRemovedReason());

  Result = yw::bitmap(1400, 980);
  Camera = yw::camera(1920, 1080, 8);
  Camera.orthographic = true;
  Camera.rotation.z = -fat4(pi / 2);
  Camera.offset.z = -10000;
  Camera.factor = 0.5f;
  Camera.update();
  Camera.begin_render(color::white);
  Camera.end_render();
  CB_Camera.from(list<>::asref(Camera.view, Camera.view_proj));
  Coord = ui::coordinator(100, 100, 8);
  Coord(Camera.rotation);

  CB_Option.from(list<>::asref(0_n4, 1.0f, 0_n8));
  CB_Matrix.from(xv_identity);

  ui::button button_cmt_load(0, rect{5, 5, 45, 25}, L"CMT");  // CMTファイルを読み込むボタン
  ui::button button_cad_load(0, rect{5, 30, 45, 50}, L"CAD"); // CADファイルを読み込むボタン
  label_cmt_file = ui::label(0, rect{50, 5, 300, 25}, L"");   // CMTファイル名を表示するラベル
  label_cad_file = ui::label(0, rect{50, 30, 300, 50}, L"");  // CADファイル名を表示するラベル
  button_cmt_load.setfocus();


  // CMTボタンを押したときの処理
  button_cmt_load.enter = [](const ui::button&) {
    if (auto filename = ui::open_file(std::format(LR"(C:\Users\{}\Desktop\表面ポイント)", main::username.data())); !filename.empty()) {
      cmt_file = mv(filename);
      label_cmt_file.text(cmt_file.filename().wstring());
      StlVertex = ff::stl(cmt_file);
      Vertex[0] = structured_buffer<vertex>(vertex::from_stl(StlVertex));
      prepare_reversed_vertex();
      UB_Margin = unordered_buffer<margin>(nullptr, Vertex[0].count);
      Margin = structured_buffer<margin>(nullptr, Vertex[0].count);
      if (Facet) {
        calc_margin(Facet, get_vertex(), CB_Matrix, UB_Margin), Margin.from(UB_Margin), obtain_maxmin(Margin, MaxMin);
        XMax = MaxMin.maxs[25].x, YMax = MaxMin.maxs[25].y, ZMax = MaxMin.maxs[25].z;
        XMin = MaxMin.mins[25].x, YMin = MaxMin.mins[25].y, ZMin = MaxMin.mins[25].z;
        update_margin_labels(MaxMin);
      }
      render();
    }
  };

  // CADボタンを押したときの処理
  button_cad_load.enter = [](const ui::button&) {
    try {
      if (auto filename = ui::open_file(LR"(\\Z1255101\apps\CMT\3Dモデル\組立型クランク軸\スロー\ACMA)"); !filename.empty()) {
        std::cout << source{} << std::endl, tiff(main::d3d_device->GetDeviceRemovedReason());
        cad_file = mv(filename);
        std::cout << source{} << std::endl, tiff(main::d3d_device->GetDeviceRemovedReason());
        label_cad_file.text(cad_file.filename().wstring());
        std::cout << source{} << std::endl, tiff(main::d3d_device->GetDeviceRemovedReason());
        StlFacet = ff::stl(cad_file);
        std::cout << source{} << std::endl, tiff(main::d3d_device->GetDeviceRemovedReason());
        Facet = structured_buffer(facet::from_stl(StlFacet));
        std::cout << source{} << std::endl, tiff(main::d3d_device->GetDeviceRemovedReason());
        const fat4* a = reinterpret_cast<const fat4*>(StlFacet.header().data() + 16);
        std::cout << source{} << std::endl, tiff(main::d3d_device->GetDeviceRemovedReason());
        RadiusJH = a[0], HalfGap = a[1], HalfStroke = a[2];
        std::cout << source{} << std::endl, tiff(main::d3d_device->GetDeviceRemovedReason());
        if (Vertex[0]) {
          std::cout << source{} << std::endl, tiff(main::d3d_device->GetDeviceRemovedReason());
          calc_margin(Facet, get_vertex(), CB_Matrix, UB_Margin), Margin.from(UB_Margin), obtain_maxmin(Margin, MaxMin);
          std::cout << source{} << std::endl, tiff(main::d3d_device->GetDeviceRemovedReason());
          XMax = MaxMin.maxs[25].x, YMax = MaxMin.maxs[25].y, ZMax = MaxMin.maxs[25].z;
          std::cout << source{} << std::endl, tiff(main::d3d_device->GetDeviceRemovedReason());
          XMin = MaxMin.mins[25].x, YMin = MaxMin.mins[25].y, ZMin = MaxMin.mins[25].z;
          std::cout << source{} << std::endl, tiff(main::d3d_device->GetDeviceRemovedReason());
          update_margin_labels(MaxMin);
          std::cout << source{} << std::endl, tiff(main::d3d_device->GetDeviceRemovedReason());
        }
        std::cout << source{} << std::endl, tiff(main::d3d_device->GetDeviceRemovedReason());
        render();
        std::cout << source{} << std::endl, tiff(main::d3d_device->GetDeviceRemovedReason());
      }
    } catch (const std::exception& E) { std::cout << E.what() << std::endl, throw except(E); }
  };

  rb_flip = ui::radiobutton(0, rect{305, 5, 405, 75}, L"フリップ", L"全表示", L"＋X側を非表示", L"－X側を非表示");
  rb_flip.enter = [](const ui::radiobutton& R) { CB_Option.from(list<>::asref(R.state, cb_opacity.state ? 0.6f : 1.0f, 0_n8)), render(); };

  cb_reverse = ui::checkbox(0, rect{410, 5, 490, 25}, L"T/B反転");
  cb_reverse.enter = [](const ui::checkbox& C) {
    // std::cout << C.state << std::endl;
    Reversed = C.state;
    Degrees = xvnegate<1, 0, 1, 0>(Degrees), Offsets = xvnegate<1, 0, 1, 0>(Offsets);
    VB_Rot[0].text(std::format(L"{:.4f}", Degrees.x)), VB_Rot[2].text(std::format(L"{:.4f}", Degrees.z));
    VB_Mov[0].text(std::format(L"{:.3f}", Offsets.x)), VB_Mov[2].text(std::format(L"{:.3f}", Offsets.z));
    CB_Matrix.from([&](xmatrix& m) { xvworld(Offsets, xvradian(Degrees), m); });
    if (!(Facet && Vertex[0])) return;
    calc_margin(Facet, get_vertex(), CB_Matrix, UB_Margin), Margin.from(UB_Margin);
    obtain_maxmin(Margin, MaxMin), update_margin_labels(MaxMin), render();
  };

  cb_opacity = ui::checkbox(0, rect{410, 30, 490, 50}, L"半透明");
  cb_opacity.enter = [](const ui::checkbox& C) {
    CB_Option.from(list<>::asref(rb_flip.state, C.state ? 0.6f : 1.0f, 0_n8)), render();
  };

  Progress = ui::progressbar(145, 10, color::skyblue), Progress(1.0f);

  ui::button bt_align(0, rect{820, 20, 945, 75}, L"自動芯合わせ", WS_BORDER);
  bt_align.enter = [](const ui::button&) { align(); };

  ui::label lb_rotation(0, rect{810, 85, 955, 105}, L"------ 回転 [deg] ------", SS_CENTER | SS_CENTERIMAGE);
  ui::label lb_rot[3]{{0, rect{810, 110, 850, 130}, L"X", SS_CENTER | SS_CENTERIMAGE},
                      {0, rect{810, 135, 850, 155}, L"Y", SS_CENTER | SS_CENTERIMAGE},
                      {0, rect{810, 160, 850, 180}, L"Z", SS_CENTER | SS_CENTERIMAGE}};
  VB_Rot[0] = ui::valuebox(0, rect{855, 110, 955, 130}, 0);
  VB_Rot[1] = ui::valuebox(0, rect{855, 135, 955, 155}, 0);
  VB_Rot[2] = ui::valuebox(0, rect{855, 160, 955, 180}, 0);
  ui::label lb_movement(0, rect{810, 190, 955, 210}, L"------ 移動 [mm] ------", SS_CENTER | SS_CENTERIMAGE);
  ui::label lb_mov[3]{{0, rect{810, 215, 850, 235}, L"X", SS_CENTER | SS_CENTERIMAGE},
                      {0, rect{810, 240, 850, 260}, L"Y", SS_CENTER | SS_CENTERIMAGE},
                      {0, rect{810, 265, 850, 285}, L"Z", SS_CENTER | SS_CENTERIMAGE}};
  VB_Mov[0] = ui::valuebox(0, rect{855, 215, 955, 235}, 0);
  VB_Mov[1] = ui::valuebox(0, rect{855, 240, 955, 260}, 0);
  VB_Mov[2] = ui::valuebox(0, rect{855, 265, 955, 285}, 0);

  VB_Rot[0].enter = [](const ui::valuebox& V) {
    auto s = std::format(L"{:.4f}", stov<fat8>(V.text()));
    Degrees.x = stov<fat4>(s), VB_Rot[0].text(s);
    CB_Matrix.from([&](xmatrix& m) { xvworld(Offsets, xvradian(Degrees), m); });
    if (!(Facet && Vertex[0])) return;
    calc_margin(Facet, get_vertex(), CB_Matrix, UB_Margin), Margin.from(UB_Margin);
    obtain_maxmin(Margin, MaxMin), update_margin_labels(MaxMin), render();
  };
  VB_Rot[1].enter = [](const ui::valuebox& V) {
    auto s = std::format(L"{:.4f}", stov<fat8>(V.text()));
    Degrees.y = stov<fat4>(s), VB_Rot[1].text(s);
    CB_Matrix.from([&](xmatrix& m) { xvworld(Offsets, xvradian(Degrees), m); });
    if (!(Facet && Vertex[0])) return;
    calc_margin(Facet, get_vertex(), CB_Matrix, UB_Margin), Margin.from(UB_Margin);
    obtain_maxmin(Margin, MaxMin), update_margin_labels(MaxMin), render();
  };
  VB_Rot[2].enter = [](const ui::valuebox& V) {
    auto s = std::format(L"{:.4f}", stov<fat8>(V.text()));
    Degrees.z = stov<fat4>(s), VB_Rot[2].text(s);
    CB_Matrix.from([&](xmatrix& m) { xvworld(Offsets, xvradian(Degrees), m); });
    if (!(Facet && Vertex[0])) return;
    calc_margin(Facet, get_vertex(), CB_Matrix, UB_Margin), Margin.from(UB_Margin);
    obtain_maxmin(Margin, MaxMin), update_margin_labels(MaxMin), render();
  };
  VB_Mov[0].enter = [](const ui::valuebox& V) {
    auto s = std::format(L"{:.3f}", stov<fat8>(V.text()));
    Offsets.x = stov<fat4>(s), VB_Mov[0].text(s);
    CB_Matrix.from([&](xmatrix& m) { xvworld(Offsets, xvradian(Degrees), m); });
    if (!(Facet && Vertex[0])) return;
    calc_margin(Facet, get_vertex(), CB_Matrix, UB_Margin), Margin.from(UB_Margin);
    obtain_maxmin(Margin, MaxMin), update_margin_labels(MaxMin), render();
  };
  VB_Mov[1].enter = [](const ui::valuebox& V) {
    auto s = std::format(L"{:.3f}", stov<fat8>(V.text()));
    Offsets.y = stov<fat4>(s), VB_Mov[1].text(s);
    CB_Matrix.from([&](xmatrix& m) { xvworld(Offsets, xvradian(Degrees), m); });
    if (!(Facet && Vertex[0])) return;
    calc_margin(Facet, get_vertex(), CB_Matrix, UB_Margin), Margin.from(UB_Margin);
    obtain_maxmin(Margin, MaxMin), update_margin_labels(MaxMin), render();
  };
  VB_Mov[2].enter = [](const ui::valuebox& V) {
    auto s = std::format(L"{:.3f}", stov<fat8>(V.text()));
    Offsets.z = stov<fat4>(s), VB_Mov[2].text(s);
    CB_Matrix.from([&](xmatrix& m) { xvworld(Offsets, xvradian(Degrees), m); });
    if (!(Facet && Vertex[0])) return;
    if (!(Facet && Vertex[0])) return;
    calc_margin(Facet, get_vertex(), CB_Matrix, UB_Margin), Margin.from(UB_Margin);
    obtain_maxmin(Margin, MaxMin), update_margin_labels(MaxMin), render();
  };


  lb_margin_all = ui::label(0, rect{810, 295, 890, 315}, L"全体最小取代", SS_CENTER | SS_CENTERIMAGE);
  lb_margin_over = ui::label(0, rect{810, 320, 890, 340}, L"面上最小取代", SS_CENTER | SS_CENTERIMAGE);
  lb_margin_over_x = ui::label(0, rect{810, 345, 890, 365}, L"面上最大取代", SS_CENTER | SS_CENTERIMAGE);
  lb_margins[0] = ui::label{0, rect{895, 295, 955, 315}, L""};
  lb_margins[1] = ui::label{0, rect{895, 320, 955, 340}, L""};
  lb_margins[2] = ui::label{0, rect{895, 345, 955, 365}, L""};

  ui::button bt_start_result(0, rect{820, 495, 945, 535}, L"成績表モード");
  bt_start_result.enter = [](const ui::button&) {
    if (!(Facet && Vertex[0])) return;
    ui::control::hide(0, ResultMode = true);
    result_mode();
    main::resize(960, 540);
    ui::control::hide(0, ResultMode = false);
    render();
  };

  mouse::moved = [](int4 dx, int4 dy) {
    if (ResultMode) return;
    auto x = mouse::x - dx, y = mouse::y - dy;
    if (5 <= x && x < 805 && 85 <= y && y < 535) {
      bool b = false;
      if (mouse::left) {
        Camera.rotation.y -= dx * 0.003f / Camera.factor;
        Camera.rotation.x += dy * 0.003f / Camera.factor;
        if (Camera.rotation.x < -fat4(pi / 2)) Camera.rotation.x = -fat4(pi / 2);
        else if (fat4(pi / 2) < Camera.rotation.x) Camera.rotation.x = fat4(pi / 2);
        Coord(Camera.rotation);
        b = true;
      } else if (mouse::right) {
        Camera.offset.x += 2.4f * dx / Camera.factor;
        Camera.offset.y += 2.4f * dy / Camera.factor;
        b = true;
      }
      if (b) {
        Camera.update();
        CB_Camera.from(list<>::asref(Camera.view, Camera.view_proj));
        render();
      }
    }
  };

  mouse::wheeled = [](fat4 dw) {
    if (ResultMode) return;
    if (5 <= mouse::x && mouse::x < 805 && 85 <= mouse::y && mouse::y < 535) {
      if (dw > 0) Camera.factor *= 1.2f;
      else Camera.factor /= 1.2f;
      Camera.update();
      CB_Camera.from(list<>::asref(Camera.view, Camera.view_proj));
      render();
    }
  };

  std::cout << source{} << std::endl, tiff(main::d3d_device->GetDeviceRemovedReason());
  main::update();
  std::cout << source{} << std::endl, tiff(main::d3d_device->GetDeviceRemovedReason());

  std::cout << source{} << std::endl, tiff(main::d3d_device->GetDeviceRemovedReason());
  main::update();
  std::cout << source{} << std::endl, tiff(main::d3d_device->GetDeviceRemovedReason());


  std::cout << source{} << std::endl, tiff(main::d3d_device->GetDeviceRemovedReason());
  render();
  std::cout << source{} << std::endl, tiff(main::d3d_device->GetDeviceRemovedReason());
  while (main::update) {
    // std::cout << std::format("{} - {}", bool(cb_reverse.state), rb_flip.state) << std::endl;
  }
}

////////////////////////////////////////////////////////////////////////////////
//  FUNCTION

unordered_buffer<facet> facet::from_stl(const ff::stl& Stl) {
  static constexpr stv1 hlsl = R"(
#pragma pack_matrix(row_major)
struct UB {
  uint id;
  float3 a, b, c;
  float3 ab, bc, ca;
  float3 sq, nisq;
  float4 plane;
};
struct SB {
  uint id;
  float3 a, b, c;
};
RWStructuredBuffer<UB> ub : register(u0);
  StructuredBuffer<SB> sb : register(t0);
[numthreads(1024, 1, 1)] void csmain(uint i : SV_DispatchThreadID) {
  ub[i].id = sb[i].id;
  ub[i].a = sb[i].a, ub[i].b = sb[i].b, ub[i].c = sb[i].c;
  ub[i].ab = sb[i].b - sb[i].a, ub[i].bc = sb[i].c - sb[i].b, ub[i].ca = sb[i].a - sb[i].c;
  ub[i].sq = float3(dot(ub[i].ab, ub[i].ab), dot(ub[i].bc, ub[i].bc), dot(ub[i].ca, ub[i].ca));
  ub[i].nisq = -1.0f / ub[i].sq;
  float4 t = float4(cross(sb[i].a, sb[i].b), 0);
  t = float4(t.xyz + cross(sb[i].b, sb[i].c) + cross(sb[i].c, sb[i].a), dot(t.xyz, sb[i].c));
  ub[i].plane = t / length(t.xyz);
})";
  if (!Stl.valid()) return {};
  std::cout << source{} << std::endl;
  static auto gp = gpgpu<typepack<facet>, typepack<list<nat4, array<fat4, 9>>>>(hlsl);
  std::cout << source{} << std::endl;
  try {
    std::cout << source{} << std::endl;
    unordered_buffer<facet> ub(nullptr, Stl.size());
    std::cout << source{} << std::endl;
    array<list<nat4, array<fat4, 9>>> t(Stl.size());
    std::cout << source{} << std::endl;
    for (natt i{}; i < Stl.size(); ++i)
      t[i].first = Stl[i].attribute, memcpy(t[i].second.data(), Stl[i].vertex, 36);
    std::cout << source{} << std::endl;
    structured_buffer sb(t.data(), t.size());
    std::cout << source{} << std::endl;
    gp(t.size(), {ub}, {sb}, {});
    std::cout << source{} << std::endl;
    return ub;
  } catch (const std::exception& E) { throw except(E); }
}

unordered_buffer<vertex> vertex::from_stl(const ff::stl& Stl) {
  static constexpr stv1 hlsl = R"(
#pragma pack_matrix(row_major)
struct UB { float4 v, n; };
struct SB { float3 a, b, c; };
RWStructuredBuffer<UB> ub : register(u0);
  StructuredBuffer<SB> sb : register(t0);
[numthreads(1024, 1, 1)] void csmain(uint i : SV_DispatchThreadID) {
  uint j = i * 3;
  float4 normal = float4(normalize(cross(sb[i].b - sb[i].a, sb[i].c - sb[i].a)), 0.0f);
  ub[j + 0].v = float4(sb[i].a, 1.0f), ub[j + 0].n = normal;
  ub[j + 1].v = float4(sb[i].b, 1.0f), ub[j + 1].n = normal;
  ub[j + 2].v = float4(sb[i].c, 1.0f), ub[j + 2].n = normal;
})";
  if (!Stl.valid()) return {};
  static auto gp = gpgpu<typepack<vertex>, typepack<array<fat4, 9>>>(hlsl);
  try {
    unordered_buffer<vertex> ub(nullptr, Stl.size() * 3);
    array<array<fat4, 9>> t(Stl.size());
    for (natt i{}; i < Stl.size(); ++i) memcpy(t[i].data(), Stl[i].vertex, 36);
    structured_buffer sb(t.data(), t.size());
    gp(sb.count, {ub}, {sb}, {});
    return ub;
  } catch (const std::exception& E) { throw except(E); }
}

void calc_margin(const structured_buffer<facet>& Facet,
                 const structured_buffer<vertex>& Vertex,
                 const constant_buffer<xmatrix>& Matrix,
                 unordered_buffer<margin>& Margin) {
  static constexpr stv1 hlsl = R"(
#pragma pack_matrix(row_major)
struct UB  {
  float4 p, n;
  float2 r;
  uint2  ru;
};
struct SB0 {
  uint id;
  float3 a,  b,  c;
  float3 ab, bc, ca;
  float3 sq, nisq;
  float4 plane;
};
struct SB1 {
  float4 p, n;
};
RWStructuredBuffer<UB > ub  : register(u0);
  StructuredBuffer<SB0> sb0 : register(t0);
  StructuredBuffer<SB1> sb1 : register(t1);
cbuffer CB0 : register(b0) {
  uint m, n;
  uint _0, _1;
};
cbuffer CB1 : register(b1) {
  matrix mat;
};
[numthreads(1024, 1, 1)] void csmain(uint i : SV_DispatchThreadID) {
  if (i >= n) return;
  float3 a, b, c, p = mul(mat, float4(sb1[i].p.xyz, 1)).xyz, nn = mul(mat, float4(sb1[i].n.xyz, 0));
  float2 r = float2(1e10, 0), t;
  uint2 ru;
  for (uint j = 0; j < m; ++j) {
    a = p - sb0[j].a;
    b = p - sb0[j].b;
    c = p - sb0[j].c;
    t.y = dot(p, sb0[j].plane.xyz) - sb0[j].plane.w;
    if (r.x < t.y * t.y) continue;
    if (dot(cross(a, b), sb0[j].plane.xyz) < 0) {
      t.x = dot(a, sb0[j].ab);
      if (t.x < 0) t.x = dot(a, a);
      else if (sb0[j].sq.x < t.x) t.x = dot(b, b);
      else t.x = dot(a, a) + t.x * t.x * sb0[j].nisq.x;
      if (t.x < r.x) r = t, ru = uint2(sb0[j].id, false);
    } else if (dot(cross(b, c), sb0[j].plane.xyz) < 0) {
      t.x = dot(b, sb0[j].bc);
      if (t.x < 0) t.x = dot(b, b);
      else if (sb0[j].sq.y < t.x) t.x = dot(c, c);
      else t.x = dot(b, b) + t.x * t.x * sb0[j].nisq.y;
      if (t.x < r.x) r = t, ru = uint2(sb0[j].id, false);
    } else if (dot(cross(c, a), sb0[j].plane.xyz) < 0) {
      t.x = dot(c, sb0[j].ca);
      if (t.x < 0) t.x = dot(c, c);
      else if (sb0[j].sq.z < t.x) t.x = dot(a, a);
      else t.x = dot(c, c) + t.x * t.x * sb0[j].nisq.z;
      if (t.x < r.x) r = t, ru = uint2(sb0[j].id, false);
    } else r = float2(t.y * t.y, t.y), ru = uint2(sb0[j].id, true);
  }
  r.x = ru.y ? sign(r.y) * sqrt(r.x) : sqrt(r.x);
  ub[i].p = float4(p, 1), ub[i].n = float4(nn, 0), ub[i].r = r; ub[i].ru = ru;
})";
  static auto gp = gpgpu<typepack<margin>, typepack<facet, vertex>, typepack<list<nat4, nat4, nat8>, xmatrix>>(hlsl);
  static auto cb = constant_buffer(list<nat4, nat4, nat8>{});
  try {
    tiff(Vertex.count == Margin.count);
    cb.from(list{Facet.count, Vertex.count, 0_n8});
    gp(Vertex.count, {Margin}, {Facet, Vertex}, {cb, Matrix});
  } catch (const std::exception& E) { throw except(E); }
}

void obtain_maxmin(const structured_buffer<margin>& Margin, maxmin& Maxmin) {
  static constexpr stv1 hlsl = R"(
#pragma pack_matrix(row_major)
struct UB {
  float4 maxs[26];
  float4 mins[26];
  float4 maximum, maximum_n;
  float4 minimum, minimum_n;
  uint maximum_i, minimum_i;
};
struct SB {
  float4 p, n;
  float2 r;
  uint2  ru;
};
cbuffer CB : register(b0) { uint m, n, _2, _3; };
RWStructuredBuffer<UB> ub : register(u0);
  StructuredBuffer<SB> sb : register(t0);
[numthreads(1024, 1, 1)] void csmain(uint i : SV_DispatchThreadID) {
  for (uint k = 0; k < 25; ++k) ub[i].maxs[k].w = -1e10, ub[i].mins[k].w = 1e10;
  ub[i].maxs[25] = float4(-1e10, -1e10, -1e10, 1), ub[i].mins[25] = float4(1e10, 1e10, 1e10, 1);
  ub[i].maximum.w = -1e10, ub[i].minimum.w = 1e10;
  for (uint j = i * n, jj = j + n; j < jj && j < m; ++j) {
    if (ub[i].maxs[sb[j].ru.x].w < sb[j].r.x && sb[j].ru.y) ub[i].maxs[sb[j].ru.x] = float4(sb[j].p.xyz, sb[j].r.x);
    if (ub[i].mins[sb[j].ru.x].w > sb[j].r.x && sb[j].ru.y) ub[i].mins[sb[j].ru.x] = float4(sb[j].p.xyz, sb[j].r.x);
    if (ub[i].maximum.w < sb[j].r.x) ub[i].maximum = float4(sb[j].p.xyz, sb[j].r.x), ub[i].maximum_n = sb[j].n, ub[i].maximum_i = sb[j].ru.x;
    if (ub[i].minimum.w > sb[j].r.x) ub[i].minimum = float4(sb[j].p.xyz, sb[j].r.x), ub[i].minimum_n = sb[j].n, ub[i].minimum_i = sb[j].ru.x;
    ub[i].maxs[25] = float4(max(ub[i].maxs[25].xyz, sb[j].p.xyz), 0);
    ub[i].mins[25] = float4(min(ub[i].mins[25].xyz, sb[j].p.xyz), 0);
  }
})";
  static auto gp = gpgpu<typepack<maxmin>, typepack<margin>, typepack<list<nat4, nat4, nat8>>>(hlsl);
  static auto cb = constant_buffer(list<nat4, nat4, nat8>{});
  static auto ub = unordered_buffer<maxmin>(nullptr, 1024);
  try {
    cb.from(list{Margin.count, (Margin.count - 1) / 1024 + 1, 0_n8});
    gp(1024, {ub}, {Margin}, {cb});
    auto t = ub.to_cpu();
    for (natt j{}; j < 25; ++j) Maxmin.maxs[j] = t[0].maxs[j], Maxmin.mins[j] = t[0].mins[j];
    Maxmin.maxs[25] = t[0].maxs[25], Maxmin.mins[25] = t[0].mins[25];
    Maxmin.maximum = t[0].maximum, Maxmin.maximum_n = t[0].maximum_n, Maxmin.maximum_i = t[0].maximum_i;
    Maxmin.minimum = t[0].minimum, Maxmin.minimum_n = t[0].minimum_n, Maxmin.minimum_i = t[0].minimum_i;
    for (natt i{1}; i < 1024; ++i) {
      for (natt j{}; j < 25; ++j) {
        if (Maxmin.maxs[j].w < t[i].maxs[j].w) Maxmin.maxs[j] = t[i].maxs[j];
        if (Maxmin.mins[j].w > t[i].mins[j].w) Maxmin.mins[j] = t[i].mins[j];
      }
      Maxmin.maxs[25] = _mm_max_ps(Maxmin.maxs[25], t[i].maxs[25]);
      Maxmin.mins[25] = _mm_min_ps(Maxmin.mins[25], t[i].mins[25]);
      if (Maxmin.maximum.w < t[i].maximum.w) Maxmin.maximum = t[i].maximum, Maxmin.maximum_n = t[i].maximum_n, Maxmin.maximum_i = t[i].maximum_i;
      if (Maxmin.minimum.w > t[i].minimum.w) Maxmin.minimum = t[i].minimum, Maxmin.minimum_n = t[i].minimum_n, Maxmin.minimum_i = t[i].minimum_i;
    }
  } catch (const std::exception& E) { throw except(E); }
}

void obtain_maxmin(const unordered_buffer<margin>& Margin, maxmin& Maxmin) {
  static constexpr stv1 hlsl = R"(
#pragma pack_matrix(row_major)
struct UBOUT {
  float4 maxs[26];
  float4 mins[26];
  float4 maximum, maximum_n;
  float4 minimum, minimum_n;
  uint maximum_i, minimum_i;
};
struct UBIN1 {
  float4 p, n;
  float2 r;
  uint2  ru;
};
cbuffer CB : register(b0) { uint m, n, _2, _3; };
RWStructuredBuffer<UBOUT> ub0 : register(u0);
RWStructuredBuffer<UBIN1> ub1 : register(u1);
[numthreads(1024, 1, 1)] void csmain(uint i : SV_DispatchThreadID) {
  for (uint k = 0; k < 25; ++k) ub0[i].maxs[k].w = -1e10, ub0[i].mins[k].w = 1e10;
  ub0[i].maxs[25] = float4(-1e10, -1e10, -1e10, 1), ub0[i].mins[25] = float4(1e10, 1e10, 1e10, 1);
  ub0[i].maximum.w = -1e10, ub0[i].minimum.w = 1e10;
  for (uint j = i * n, jj = j + n; j < jj && j < m; ++j) {
    if (ub0[i].maxs[ub1[j].ru.x].w < ub1[j].r.x && ub1[j].ru.y) ub0[i].maxs[ub1[j].ru.x] = float4(ub1[j].p.xyz, ub1[j].r.x);
    if (ub0[i].mins[ub1[j].ru.x].w > ub1[j].r.x && ub1[j].ru.y) ub0[i].mins[ub1[j].ru.x] = float4(ub1[j].p.xyz, ub1[j].r.x);
    if (ub0[i].maximum.w < ub1[j].r.x) ub0[i].maximum = float4(ub1[j].p.xyz, ub1[j].r.x), ub0[i].maximum_n = ub1[j].n, ub0[i].maximum_i = ub1[j].ru.x;
    if (ub0[i].minimum.w > ub1[j].r.x) ub0[i].minimum = float4(ub1[j].p.xyz, ub1[j].r.x), ub0[i].minimum_n = ub1[j].n, ub0[i].minimum_i = ub1[j].ru.x;
    ub0[i].maxs[25] = float4(max(ub0[i].maxs[25], ub1[j].p).xyz, 0);
    ub0[i].mins[25] = float4(min(ub0[i].mins[25], ub1[j].p).xyz, 0);
  }
})";
  static auto gp = gpgpu<typepack<maxmin, margin>, typepack<>, typepack<list<nat4, nat4, nat8>>>(hlsl);
  static auto cb = constant_buffer(list<nat4, nat4, nat8>{});
  static auto ub = unordered_buffer<maxmin>(nullptr, 1024);
  static auto tb = staging_buffer<maxmin>(1024);
  try {
    cb.from(list{Margin.count, (Margin.count - 1) / 1024 + 1, 0_n8});
    gp(1024, {ub, Margin}, {}, {cb});
    auto t = ub.to_cpu(tb);
    for (natt j{}; j < 25; ++j) Maxmin.maxs[j] = t[0].maxs[j], Maxmin.mins[j] = t[0].mins[j];
    Maxmin.maxs[25] = t[0].maxs[25], Maxmin.mins[25] = t[0].mins[25];
    Maxmin.maximum = t[0].maximum, Maxmin.maximum_n = t[0].maximum_n, Maxmin.maximum_i = t[0].maximum_i;
    Maxmin.minimum = t[0].minimum, Maxmin.minimum_n = t[0].minimum_n, Maxmin.minimum_i = t[0].minimum_i;
    for (natt i{1}; i < 1024; ++i) {
      for (natt j{}; j < 25; ++j) {
        if (Maxmin.maxs[j].w < t[i].maxs[j].w) Maxmin.maxs[j] = t[i].maxs[j];
        if (Maxmin.mins[j].w > t[i].mins[j].w) Maxmin.mins[j] = t[i].mins[j];
      }
      Maxmin.maxs[25] = _mm_max_ps(Maxmin.maxs[25], t[i].maxs[25]);
      Maxmin.mins[25] = _mm_min_ps(Maxmin.mins[25], t[i].mins[25]);
      if (Maxmin.maximum.w < t[i].maximum.w) Maxmin.maximum = t[i].maximum, Maxmin.maximum_n = t[i].maximum_n, Maxmin.maximum_i = t[i].maximum_i;
      if (Maxmin.minimum.w > t[i].minimum.w) Maxmin.minimum = t[i].minimum, Maxmin.minimum_n = t[i].minimum_n, Maxmin.minimum_i = t[i].minimum_i;
    }
  } catch (const std::exception& E) { throw except(E); }
}

void optimize_margin(const structured_buffer<facet>& Facet, const structured_buffer<vertex>& Vertex,
                     maxmin& MaxMin, vector& Radians, vector& Offsets,
                     fat4 Delta, bool(checker)(const maxmin& Now, const maxmin& New), natt Blocker) {
  tiff(Vertex.count == Margin.count);
  maxmin mm[13];
  mm[12] = MaxMin;
  constant_buffer<xmatrix> cb;
  cb.from([&](xmatrix& m) { xvworld(Offsets, Radians, m); });
  fat4 rad[3] = {Delta / max(YMax, -YMin, ZMax, -ZMin), Delta / max(ZMax, -ZMin, XMax, -XMin), Delta / max(XMax, -XMin, YMax, -YMin)};
  vector r_delta[12] = {{rad[0], 0, 0}, {0, rad[1], 0}, {0, 0, rad[2]}, {}, {}, {}, {-rad[0], 0, 0}, {0, -rad[1], 0}, {0, 0, -rad[2]}},
         o_delta[12] = {{}, {}, {}, {Delta, 0, 0}, {0, Delta, 0}, {0, 0, Delta}, {}, {}, {}, {-Delta, 0, 0}, {0, -Delta, 0}, {0, 0, -Delta}};
  while (1) {
    intt best{12};
    for (intt i{}; i < 12; ++i) {
      if ((Blocker >> (i % 6)) & 1) continue;
      cb.from([&](xmatrix& m) { xvworld(xvadd(Offsets, o_delta[i]), xvadd(Radians, r_delta[i]), m); });
      calc_margin(Facet, Vertex, cb, UB_Margin);
      obtain_maxmin(UB_Margin, mm[i]);
      if (checker(mm[best], mm[i])) best = i;
    }
    std::cout << best << std::endl;
    if (best != 12) {
      Radians = xvadd(Radians, r_delta[best]);
      Offsets = xvadd(Offsets, o_delta[best]);
      mm[12] = mm[best];
    } else break;
  }
  MaxMin = mm[12];
  std::cout << std::endl;
}

void render_facet(const structured_buffer<facet>& Facet,
                  const constant_buffer<list<xmatrix, xmatrix>>& Camera,
                  const constant_buffer<list<nat4, fat4, nat8>>& Option) {
  static constexpr stv1 hlsl = R"(
#pragma pack_matrix(row_major)
struct SB {
  uint id;
  float3 a[3];
  float3 ab, bc, ca;
  float3 sq, nisq;
  float4 plane;
};
struct GSIN {
  float4 p : POSITION;
  float4 n : NORMAL;
};
struct PSIN {
  float4 p : SV_Position;
  float4 n : NORMAL;
  float4 c : COLOR;
};
StructuredBuffer<SB> sb : register(t0);
void vsmain(uint i : SV_VertexID, out GSIN Out) {
  Out.p = float4(sb[i / 3].a[i % 3], 1);
  Out.n = float4(sb[i / 3].plane.xyz, 0);
}
cbuffer CB : register(b0) { matrix v; matrix vp; };
cbuffer CB1 : register(b1) { uint flip; float opacity; uint _2, _3; }
[maxvertexcount(3)] void gsmain(triangle GSIN In[3], inout TriangleStream<PSIN> Str) {
  if (flip == 1 && In[0].p.x >= 0 && In[1].p.x >= 0 && In[2].p.x >= 0) return;
  else if (flip == 2 && In[0].p.x <= 0 && In[1].p.x <= 0 && In[2].p.x <= 0) return;
  PSIN Out;
  for (uint i = 0; i < 3; ++i) {
    Out.p = mul(vp, In[i].p);
    Out.n = mul(v, In[i].n);
    Out.c = float4(0.8f, 0, 0.8f, 1.0f);
    Str.Append(Out);
  } Str.RestartStrip();
}
float4 psmain(PSIN In) : SV_Target {
  return float4(In.c.xyz * (0.3 - 0.7 * In.n.z), In.c.w);
})";
  static auto rd = renderer<
    typepack<structured_buffer<facet>>,
    typepack<>,
    typepack<>,
    typepack<constant_buffer<list<xmatrix, xmatrix>>, constant_buffer<list<nat4, fat4, nat8>>>>(hlsl);
  try {
    rd(Facet.count * 3, {Facet}, {}, {}, {Camera, Option});
  } catch (const std::exception& E) { throw except(E); }
}

void render_vertex(const structured_buffer<vertex>& Vertex,
                   const structured_buffer<margin>& Margin,
                   const constant_buffer<xmatrix>& Matrix,
                   const constant_buffer<list<xmatrix, xmatrix>>& Camera,
                   const constant_buffer<list<nat4, fat4, nat8>>& Option) {
  static constexpr stv1 hlsl = R"(
#pragma pack_matrix(row_major)
struct SB0 {
  float4 p, n;
};
struct SB1 {
  float4 p, n;
  float2 r;
  uint2  ru;
};
struct GSIN {
  float4 p : POSITION;
  float4 n : NORMAL;
  float4 c : COLOR;
};
struct PSIN {
  float4 p : SV_Position;
  float4 n : NORMAL;
  float4 c : COLOR;
};
cbuffer Tmatrix : register(b0) {
  matrix mm;
};
cbuffer CB : register(b0) {
  matrix v;
  matrix vp;
};
cbuffer CB1 : register(b1) {
  uint flip;
  float opacity;
  uint _2, _3;
};
StructuredBuffer<SB0> sb0 : register(t0);
StructuredBuffer<SB1> sb1 : register(t1);
void vsmain(uint i : SV_VertexID, out GSIN Out) {
  Out.p = mul(mm, sb0[i].p);
  Out.n = mul(mm, float4(sb0[i].n.xyz, 0));
  if (sb1[i].r.x > 60.f) Out.c = float4(0.0f, 0.0f, 0.5f, 1.0f);
  else if (sb1[i].r.x > 45.f) Out.c = float4(0.0f, 0.5f, 1.0f, 1.0f);
  else if (sb1[i].r.x > 24.f) Out.c = float4(0.0f, 1.0f, 0.5f, 1.0f);
  else if (sb1[i].r.x > 10.f) Out.c = float4(1.0f, 1.0f, 0.0f, 1.0f);
  else if (sb1[i].r.x > 5.f) Out.c = float4(1.0f, 0.5f, 0.0f, 1.0f);
  else Out.c = float4(1.0f, 0.0f, 0.0f, 1.0f);
}
[maxvertexcount(3)] void gsmain(triangle GSIN In[3], inout TriangleStream<PSIN> Str) {
  if (flip == 1 && In[0].p.x >= 0 && In[1].p.x >= 0 && In[2].p.x >= 0) return;
  else if (flip == 2 && In[0].p.x <= 0 && In[1].p.x <= 0 && In[2].p.x <= 0) return;
  PSIN Out;
  for (uint i = 0; i < 3; ++i) {
    Out.p = mul(vp, In[i].p);
    Out.n = mul(v, In[i].n);
    Out.c = float4(In[i].c.xyz, opacity);
    Str.Append(Out);
  } Str.RestartStrip();
}
float4 psmain(PSIN In) : SV_Target {
  return float4(In.c.xyz * (0.3 - 0.7 * In.n.z), In.c.w);
})";
  static auto rd = renderer<
    typepack<structured_buffer<vertex>, structured_buffer<margin>>,
    typepack<constant_buffer<xmatrix>>,
    typepack<>,
    typepack<constant_buffer<list<xmatrix, xmatrix>>, constant_buffer<list<nat4, fat4, nat8>>>>(hlsl);
  try {
    tiff(Vertex.count == Margin.count);
    rd(Vertex.count, {Vertex, Margin}, {Matrix}, {}, {Camera, Option});
  } catch (const std::exception& E) { throw except(E); }
}


void render() {
  std::cout << source{} << std::endl, tiff(main::d3d_device->GetDeviceRemovedReason());
  Camera.begin_render(color::white);
  std::cout << source{} << std::endl, tiff(main::d3d_device->GetDeviceRemovedReason());
  if (Facet) render_facet(Facet, CB_Camera, CB_Option);
  std::cout << source{} << std::endl, tiff(main::d3d_device->GetDeviceRemovedReason());
  if (Vertex[0]) render_vertex(get_vertex(), Margin, CB_Matrix, CB_Camera, CB_Option);
  std::cout << source{} << std::endl, tiff(main::d3d_device->GetDeviceRemovedReason());
  Camera.end_render();
  std::cout << source{} << std::endl, tiff(main::d3d_device->GetDeviceRemovedReason());
  main::begin_draw(color::yw);
  std::cout << source{} << std::endl, tiff(main::d3d_device->GetDeviceRemovedReason());
  Camera.draw(vector{5, 85, 805, 535}), Progress(vector{810, 5, 955, 15}), Coord(vector{5, 85, 105, 185});
  std::cout << source{} << std::endl, tiff(main::d3d_device->GetDeviceRemovedReason());
  main::end_draw();
  std::cout << source{} << std::endl, tiff(main::d3d_device->GetDeviceRemovedReason());
  std::cout << source{} << std::endl, tiff(main::d3d_device->GetDeviceRemovedReason());
  Sleep(1000);
  std::cout << source{} << std::endl, tiff(main::d3d_device->GetDeviceRemovedReason());
  std::cout << source{} << std::endl, tiff(main::d3d_device->GetDeviceRemovedReason());
  std::cout << source{} << std::endl, tiff(main::d3d_device->GetDeviceRemovedReason());
  std::cout << source{} << std::endl, tiff(main::d3d_device->GetDeviceRemovedReason());
  main::update();
  std::cout << source{} << std::endl, tiff(main::d3d_device->GetDeviceRemovedReason());
  main::update();
  std::cout << source{} << std::endl, tiff(main::d3d_device->GetDeviceRemovedReason());
}


void align() {
  auto& vv = get_vertex();
  if (!(Facet && vv)) return;
  vector radian = xvradian(vector{VB_Rot[0].value<fat4>(), VB_Rot[1].value<fat4>(), VB_Rot[2].value<fat4>()});
  vector offset{VB_Mov[0].value<fat4>(), VB_Mov[1].value<fat4>(), VB_Mov[2].value<fat4>()};
  CB_Matrix.from([&](xmatrix& m) { xvworld(offset, radian, m); });
  calc_margin(Facet, vv, CB_Matrix, UB_Margin), Margin.from(UB_Margin);
  obtain_maxmin(Margin, MaxMin), update_margin_labels(MaxMin), Progress(0.0f), render();

  auto update_values = [&]() {
    vector degree = xvdegree(radian);
    VB_Rot[0].value(degree.x), VB_Rot[1].value(degree.y), VB_Rot[2].value(degree.z);
    VB_Mov[0].value(offset.x), VB_Mov[1].value(offset.y), VB_Mov[2].value(offset.z);
    CB_Matrix.from([&](xmatrix& m) { xvworld(offset, radian, m); });
    calc_margin(Facet, vv, CB_Matrix, UB_Margin), Margin.from(UB_Margin);
    fat4 a = apply([&](auto&&... as) { return yw::max(as.w...); }, projector(MaxMin.maxs, make_sequence<25>{}));
    fat4 b = apply([&](auto&&... as) { return yw::min(as.w...); }, projector(MaxMin.mins, make_sequence<25>{}));
    lb_margins[0].text(std::format(L"{:.3f}", MaxMin.minimum.w));
    lb_margins[1].text(std::format(L"{:.3f}", b));
    lb_margins[2].text(std::format(L"{:.3f}", a));
  };
  stopwatch sw;

  // 基準面最大取代最小化（YZ回転+X移動）
  auto checker_0xt = [](const maxmin& Now, const maxmin& New) {
    return max(Now.maxs[0].w, Now.maxs[12].w) > max(New.maxs[0].w, New.maxs[12].w);
  };
  optimize_margin(Facet, vv, MaxMin, radian, offset, 125.f, checker_0xt, 0b110001);
  update_values(), Progress(1 / 24.0f), render();
  optimize_margin(Facet, vv, MaxMin, radian, offset, 25.f, checker_0xt, 0b110001);
  update_values(), Progress(2 / 24.0f), render();
  optimize_margin(Facet, vv, MaxMin, radian, offset, 5.f, checker_0xt, 0b110001);
  update_values(), Progress(3 / 24.0f), render();
  optimize_margin(Facet, vv, MaxMin, radian, offset, 1.f, checker_0xt, 0b110001);
  update_values(), Progress(4 / 24.0f), render();

  // 側面最大取代最小化（X回転のみ）
  auto checker_0xr = [](const maxmin& Now, const maxmin& New) {
    return max(Now.maxs[6].w, Now.maxs[9].w, Now.maxs[18].w, Now.maxs[21].w) >
           max(New.maxs[6].w, New.maxs[9].w, New.maxs[18].w, New.maxs[21].w);
  };
  optimize_margin(Facet, vv, MaxMin, radian, offset, 125.f, checker_0xr, 0b111110);
  update_values(), Progress(5 / 24.0f), render();
  optimize_margin(Facet, vv, MaxMin, radian, offset, 25.0f, checker_0xr, 0b111110);
  update_values(), Progress(6 / 24.0f), render();
  optimize_margin(Facet, vv, MaxMin, radian, offset, 5.0f, checker_0xr, 0b111110);
  update_values(), Progress(7 / 24.0f), render();
  optimize_margin(Facet, vv, MaxMin, radian, offset, 1.0f, checker_0xr, 0b111110);
  update_values(), Progress(8 / 24.0f), render();

  // 側面最大取代最小化（Z移動のみ）
  auto checker_0zt = [](const maxmin& Now, const maxmin& New) {
    return max(Now.maxs[6].w, Now.maxs[9].w, Now.maxs[18].w, Now.maxs[21].w) >
           max(New.maxs[6].w, New.maxs[9].w, New.maxs[18].w, New.maxs[21].w);
  };
  optimize_margin(Facet, vv, MaxMin, radian, offset, 25.f, checker_0zt, 0b011111);
  update_values(), Progress(9 / 24.0f), render();
  optimize_margin(Facet, vv, MaxMin, radian, offset, 5.f, checker_0zt, 0b011111);
  update_values(), Progress(10 / 24.0f), render();
  optimize_margin(Facet, vv, MaxMin, radian, offset, 1.f, checker_0zt, 0b011111);
  update_values(), Progress(11 / 24.0f), render();

  // 前後最大取代最小化（Y移動のみ）
  auto checker_0yt = [](const maxmin& Now, const maxmin& New) {
    return max(Now.maxs[4].w, Now.maxs[7].w, Now.maxs[10].w, Now.maxs[16].w, Now.maxs[19].w, Now.maxs[22].w, Now.maxs[24].w) >
           max(New.maxs[4].w, New.maxs[7].w, New.maxs[10].w, New.maxs[16].w, New.maxs[19].w, New.maxs[22].w, New.maxs[24].w);
  };
  optimize_margin(Facet, vv, MaxMin, radian, offset, 25.f, checker_0yt, 0b101111);
  update_values(), Progress(12 / 24.0f), render();
  optimize_margin(Facet, vv, MaxMin, radian, offset, 5.f, checker_0yt, 0b101111);
  update_values(), Progress(13 / 24.0f), render();
  optimize_margin(Facet, vv, MaxMin, radian, offset, 1.f, checker_0yt, 0b101111);
  update_values(), Progress(14 / 24.0f), render();

  // 基準面＋内股最小取代最大化（YZ回転＋X移動）
  auto checker_1a = [](const maxmin& Now, const maxmin& New) {
    return min(Now.mins[0].w, Now.mins[2].w, Now.mins[12].w, Now.mins[14].w) <
           min(New.mins[0].w, New.mins[2].w, New.mins[12].w, New.mins[14].w);
  };
  optimize_margin(Facet, vv, MaxMin, radian, offset, 25.f, checker_1a, 0b110001);
  update_values(), Progress(15 / 24.0f), render();
  optimize_margin(Facet, vv, MaxMin, radian, offset, 5.f, checker_1a, 0b110001);
  update_values(), Progress(16 / 24.0f), render();
  optimize_margin(Facet, vv, MaxMin, radian, offset, 1.f, checker_1a, 0b110001);
  update_values(), Progress(17 / 24.0f), render();
  optimize_margin(Facet, vv, MaxMin, radian, offset, 0.2f, checker_1a, 0b110001);
  update_values(), Progress(18 / 24.0f), render();
  optimize_margin(Facet, vv, MaxMin, radian, offset, 0.04f, checker_1a, 0b110001);
  update_values(), Progress(19 / 24.0f), render();

  // 側面＋前後最小取代最大化（X回転＋XYZ移動）
  auto checker_1b = [](const maxmin& Now, const maxmin& New) {
    return min(Now.mins[0].w, Now.mins[1].w, Now.mins[2].w, Now.mins[3].w, Now.mins[4].w, Now.mins[5].w,
               Now.mins[6].w, Now.mins[7].w, Now.mins[8].w, Now.mins[9].w, Now.mins[10].w, Now.mins[11].w,
               Now.mins[12].w, Now.mins[13].w, Now.mins[14].w, Now.mins[15].w, Now.mins[16].w, Now.mins[17].w,
               Now.mins[18].w, Now.mins[19].w, Now.mins[20].w, Now.mins[21].w, Now.mins[22].w, Now.mins[23].w, Now.mins[24].w) <
           min(New.mins[0].w, New.mins[1].w, New.mins[2].w, New.mins[3].w, New.mins[4].w, New.mins[5].w,
               New.mins[6].w, New.mins[7].w, New.mins[8].w, New.mins[9].w, New.mins[10].w, New.mins[11].w,
               New.mins[12].w, New.mins[13].w, New.mins[14].w, New.mins[15].w, New.mins[16].w, New.mins[17].w,
               New.mins[18].w, New.mins[19].w, New.mins[20].w, New.mins[21].w, New.mins[22].w, New.mins[23].w, New.mins[24].w);
  };
  optimize_margin(Facet, vv, MaxMin, radian, offset, 25.f, checker_1b, 0b000110);
  update_values(), Progress(20 / 24.0f), render();
  optimize_margin(Facet, vv, MaxMin, radian, offset, 5.f, checker_1b, 0b000110);
  update_values(), Progress(21 / 24.0f), render();
  optimize_margin(Facet, vv, MaxMin, radian, offset, 1.f, checker_1b, 0b000110);
  update_values(), Progress(22 / 24.0f), render();
  optimize_margin(Facet, vv, MaxMin, radian, offset, 0.2f, checker_1b, 0b000110);
  update_values(), Progress(23 / 24.0f), render();
  optimize_margin(Facet, vv, MaxMin, radian, offset, 0.04f, checker_1b, 0b000110);
  update_values(), Progress(24 / 24.0f), render();

  { // 仕上げ
    Degrees = xvdegree(radian), Offsets = offset;
    VB_Rot[0].text(std::format(L"{:.4f}", Degrees.x));
    VB_Rot[1].text(std::format(L"{:.4f}", Degrees.y));
    VB_Rot[2].text(std::format(L"{:.4f}", Degrees.z));
    VB_Mov[0].text(std::format(L"{:.3f}", Offsets.x));
    VB_Mov[1].text(std::format(L"{:.3f}", Offsets.y));
    VB_Mov[2].text(std::format(L"{:.3f}", Offsets.z));
    Degrees = vector{VB_Rot[0].value<fat4>(), VB_Rot[1].value<fat4>(), VB_Rot[2].value<fat4>()};
    Offsets = vector{VB_Mov[0].value<fat4>(), VB_Mov[1].value<fat4>(), VB_Mov[2].value<fat4>()};
    CB_Matrix.from([&](xmatrix& m) { xvworld(Offsets, xvradian(Degrees), m); });
    calc_margin(Facet, vv, CB_Matrix, UB_Margin), Margin.from(UB_Margin);
    obtain_maxmin(Margin, MaxMin), update_margin_labels(MaxMin), render();
  }

  ui::ok(std::format(L"芯合わせ完了 ({}秒)", sw.read()));
}


fat4 calc_volume(const structured_buffer<vertex>& Vertex) {
  static constexpr stv1 hlsl = R"(
#pragma pack_matrix(row_major)
struct UB { float volume; };
struct SB { float4 v, n; };
cbuffer CB : register(b0) { uint m, n, _0, _1; };
RWStructuredBuffer<UB> ub : register(u0);
  StructuredBuffer<SB> sb : register(t0);
[numthreads(1024, 1, 1)] void csmain(uint i : SV_DispatchThreadID) {
  if (i >= m) ub[i].volume = 0;
  else ub[i].volume = dot(cross(sb[3 * i + 1].v.xyz, sb[3 * i + 2].v.xyz), sb[3 * i].v.xyz);
})";
  static auto gp = gpgpu<typepack<fat4>, typepack<vertex>, typepack<list<nat4, nat4, nat8>>>(hlsl);
  static auto cb = constant_buffer(list<nat4, nat4, nat8>{});
  static auto ub = unordered_buffer<fat4>(nullptr, std::bit_ceil(Vertex.count / 3));
  try {
    cb.from(list{Vertex.count / 3, 0_n4, 0_n8});
    gp(std::bit_ceil(Vertex.count / 3), {ub}, {Vertex}, {cb});
    auto temp = ub.to_cpu();
    for (natt i{temp.size() / 2}; i != 0; i >>= 1)
      for (natt j{0}; j < i; ++j) temp[j] += temp[j + i];
    return temp[0] * fat4(1e-9) / 6;
  } catch (std::exception& e) { throw except(e); }
}


void create_result() {
  static constexpr vector title(10, 10, 700, 100);
  static constexpr vector views[10] = {
    {10, 100, 240, 600},
    {240, 100, 470, 600},
    {470, 100, 700, 600},
    {700, 100, 930, 600},
    {930, 100, 1160, 600},
    {1160, 100, 1390, 600},
    {10, 600, 240, 850},
    {240, 600, 470, 850},
    {470, 600, 700, 850},
    {700, 600, 930, 850}};
  static constexpr vector rotation[10] = {
    {},
    {0, -pi / 2},
    {0, pi / 2},
    {0, -pi / 2},
    {0, pi / 2},
    {0, pi},
    {0, -pi / 2, -pi / 2},
    {0, -pi / 2, -pi / 2},
    {0, pi / 2, -pi / 2},
    {0, pi / 2, -pi / 2}};
  static constexpr vector csq[6] = {
    {940, 640, 965, 665},
    {965, 640, 965 + (400 * 15) / 55, 665},
    {965 + (400 * 15) / 55, 640, 965 + (400 * 36) / 55, 665},
    {965 + (400 * 36) / 55, 640, 965 + (400 * 50) / 55, 665},
    {965 + (400 * 50) / 55, 640, 965 + (400 * 55) / 55, 665},
    {965 + (400 * 55) / 55, 640, 1390, 665}};
  static constexpr auto csqc = list<brush<color{0, 0, 0.5f, 1}>,
                                    brush<color{0, 0.5f, 1, 1}>,
                                    brush<color{0, 1, 0.5f, 1}>,
                                    brush<color{1, 1, 0, 1}>,
                                    brush<color{1, 0.5f, 0, 1}>,
                                    brush<color{1, 0, 0, 1}>>{};
  static constexpr auto csqf = font<20, L"Yu Gothic UI", 0>{};
  try {
    auto& vv = get_vertex();
    Result.begin_draw(color::white), Result.end_draw();
    camera cc(nat4(views[0].z - views[0].x), nat4(views[0].w - views[0].y), 8);
    cc.orthographic = true;
    cc.factor = yw::min((views[0].z - views[0].x) / (MaxMin.maxs[25].x - MaxMin.mins[25].x),
                        (views[0].w - views[0].y) / (MaxMin.maxs[25].y - MaxMin.mins[25].y),
                        (views[0].z - views[0].x) / (MaxMin.maxs[25].z - MaxMin.mins[25].z));
    cc.factor *= 0.9f;
    fat4 off_y = (MaxMin.maxs[25].y + MaxMin.mins[25].y) / 2;
    vector offset[10] = {
      {0, off_y, -5000},
      {0, off_y, -5000},
      {0, off_y},
      {0, off_y},
      {0, off_y, -5000},
      {0, off_y, -5000},
      {0, 0, -5000},
      {},
      {},
      {0, 0, -5000}};
    constant_buffer<list<xmatrix, xmatrix>> cb;
    for (natt i{}; i < 10; ++i) {
      if (i == 6) {
        camera cd(nat4(views[6].z - views[6].x), nat4(views[6].w - views[6].y), 8);
        cd.orthographic = true;
        cd.factor = cc.factor;
        cc = mv(cd);
      }
      cc.offset = offset[i];
      cc.rotation = rotation[i];
      cc.update();
      cb.from(list<>::asref(cc.view, cc.view_proj));
      cc.begin_render(color::white);
      render_vertex(vv, Margin, CB_Matrix, cb, CB_Option);
      cc.end_render();
      Result.begin_draw();
      cc.draw(views[i]);
      brush<color::black>{}.draw_rectangle(views[i], 0.4f);
      Result.end_draw();
    }
    natt minimum_i = MaxMin.minimum_i;
    vector minimum_n = MaxMin.minimum_n;
    vector minimum = MaxMin.minimum;
    Result.begin_draw();
    { // タイトルを表示する
      static constexpr auto font = yw::font<30, L"Yu Gothic UI", -1, false, true>{};
      brush<color::black>{}.draw_text(vector(10, 50, 800, 90), font, L" 芯出成績表 ／ 株式会社 神戸製鋼所 高砂鋳鍛鋼工場");
    }
    { // 判定基準を表示する
      static constexpr auto font = yw::font<18, L"Yu Gothic UI", 0>{};
      brush<color::black>{}.draw_rectangle(vector(1200, 40, 1390, 65), 1.0f);
      brush<color::red>{}.draw_text(vector(1200, 40, 1390, 65), font, L"判定基準");
      brush<color::black>{}.draw_rectangle(vector(1200, 65, 1390, 90), 1.0f);
      brush<color::red>{}.draw_text(vector(1200, 65, 1390, 90), font, L"TRS-FP-TE-042. Rev.12");
    }
    { // コンタースケールを表示する
      static constexpr vector rects[] = {
        vector(csq[0].z - 100, csq[0].w, csq[0].z + 100, csq[0].w + 25),
        vector(csq[1].z - 100, csq[1].w, csq[1].z + 100, csq[1].w + 25),
        vector(csq[2].z - 100, csq[2].w, csq[2].z + 100, csq[2].w + 25),
        vector(csq[3].z - 100, csq[3].w, csq[3].z + 100, csq[3].w + 25),
        vector(csq[4].z - 100, csq[4].w, csq[4].z + 100, csq[4].w + 25)};
      static constexpr stv2 texts[] = {L"60", L"45", L"24", L"10", L"5"};
      static constexpr auto font = yw::font<20, L"Yu Gothic UI", 0>{};
      cfor<0, [](natt i) { return i < 5; }>([](auto&& a, auto&& b) { brush<color::black>{}.draw_text(a, font, b); }, rects, texts);
      brush<color::black>{}.draw_text(vector(1200, 690, 1390, 715), yw::font<20, L"Yu Gothic UI", 1>{}, L"[mm]");
      brush<color::black>{}.draw_text(vector(1150, 600, 1390, 620), yw::font<16, L"Yu Gothic UI", 1>{}, L"（外周最大/最小取代表記）");
      cfor<0, [](natt i) { return i < 6; }>([](auto&& a, auto&& b) { b.draw_rectangle(a), brush<color::black>{}.draw_rectangle(a, 0.4f); }, csq, csqc);
    }
    { // 全体最小取り代を表示する
      static constexpr auto font = yw::font<20, L"Yu Gothic UI", -1>{};
      brush<color::black>{}.draw_text(vector(950, 700, 1390, 725), font, std::format(L"全体最小取代： {:.2f} mm (◇)", minimum.w));
      brush<color::black>{}.draw_text(vector(950, 730, 1930, 755), font, std::format(L"　　素材重量： {:.2f} Ton", calc_volume(vv) * 7.85));
      static constexpr auto f2 = yw::font<15, L"Yu Gothic UI", 0>{};
      static constexpr vector rr(0, 0, 100, 20);
      static constexpr auto getcolor = [](fat4 f) {
        if (f > 60.f) return color(0.0f, 0.0f, 0.5f, 1.0f);
        else if (f > 45.f) return color(0.0f, 0.5f, 1.0f, 1.0f);
        else if (f > 24.f) return color(0.0f, 1.0f, 0.5f, 1.0f);
        else if (f > 10.f) return color(1.0f, 1.0f, 0.0f, 1.0f);
        else if (f > 5.f) return color(1.0f, 0.5f, 0.0f, 1.0f);
        else return color(1.0f, 0.0f, 0.0f, 1.0f);
      };
      vector lrmm[4];
      natt kaku = (MaxMin.maxs[8].w != -1e10) + 1;
      // std::cout << std::format("kaku={}\n", kaku);
      { // 左側最大
        lrmm[0] = MaxMin.maxs[6];
        if (kaku == 2 && lrmm[0].w < MaxMin.maxs[7].w) lrmm[0] = MaxMin.maxs[7];
        if (lrmm[0].w < MaxMin.maxs[18].w) lrmm[0] = MaxMin.maxs[18];
        if (kaku == 2 && lrmm[0].w < MaxMin.maxs[19].w) lrmm[0] = MaxMin.maxs[19];
        if (MaxMin.maxs[4].z > 0 && lrmm[0].w < MaxMin.maxs[4].w) lrmm[0] = MaxMin.maxs[4];
        if (MaxMin.maxs[16].z > 0 && lrmm[0].w < MaxMin.maxs[16].w) lrmm[0] = MaxMin.maxs[16];
        yw::bitmap bm(nat4(rr.z - rr.x), nat4(rr.w - rr.y));
        Result.end_draw();
        bm.begin_draw(color::white);
        brush(getcolor(lrmm[0].w)).draw_rounded_rectangle(rr, 2.5f, 2.5f);
        brush<color::black>{}.draw_text(rr, f2, std::format(L"{:.2f}", lrmm[0].w));
        bm.end_draw();
        Result.begin_draw();
        if (lrmm[0].x > 0) {
          main::d2d_context->SetTransform(D2D1::Matrix3x2F::Rotation(90.f, D2D1::Point2F(1375.f, 360.f)));
          bm.draw(vector(1325, 350, 1425, 370));
          main::d2d_context->SetTransform((D2D1::Matrix3x2F::Identity()));
          main::d2d_context->DrawLine({1275 + lrmm[0].x * cc.factor, 350 - (lrmm[0].y - off_y) * cc.factor}, {1365, 360}, brush<color::black>{});
        } else {
          main::d2d_context->SetTransform(D2D1::Matrix3x2F::Rotation(90.f, D2D1::Point2F(1175.f, 360.f)));
          bm.draw(vector(1125, 350, 1225, 370));
          main::d2d_context->SetTransform((D2D1::Matrix3x2F::Identity()));
          main::d2d_context->DrawLine({1275 + lrmm[0].x * cc.factor, 350 - (lrmm[0].y - off_y) * cc.factor}, {1185, 360}, brush<color::black>{});
        }
      }
      { // 左側最小
        lrmm[1] = MaxMin.mins[6];
        if (kaku == 2 && lrmm[1].w > MaxMin.mins[7].w) lrmm[1] = MaxMin.mins[7];
        if (lrmm[1].w > MaxMin.mins[18].w) lrmm[1] = MaxMin.mins[18];
        if (kaku == 2 && lrmm[1].w > MaxMin.mins[19].w) lrmm[1] = MaxMin.mins[19];
        if (lrmm[1].w > MaxMin.mins[20].w) lrmm[1] = MaxMin.mins[20];
        if (MaxMin.mins[4].z > 0 && lrmm[1].w > MaxMin.mins[4].w) lrmm[1] = MaxMin.mins[4];
        if (MaxMin.mins[16].z > 0 && lrmm[1].w > MaxMin.mins[16].w) lrmm[1] = MaxMin.mins[16];
        yw::bitmap bm(nat4(rr.z - rr.x), nat4(rr.w - rr.y));
        Result.end_draw();
        bm.begin_draw(color::white);
        brush(getcolor(lrmm[1].w)).draw_rounded_rectangle(rr, 2.5f, 2.5f);
        brush<color::black>{}.draw_text(rr, f2, std::format(L"{:.2f}", lrmm[1].w));
        bm.end_draw();
        Result.begin_draw();
        if (lrmm[1].x > 0) {
          main::d2d_context->SetTransform(D2D1::Matrix3x2F::Rotation(90.f, D2D1::Point2F(1375.f, 240.f)));
          bm.draw(vector(1325, 230, 1425, 250));
          main::d2d_context->SetTransform((D2D1::Matrix3x2F::Identity()));
          main::d2d_context->DrawLine({1275 + lrmm[1].x * cc.factor, 350 - (lrmm[1].y - off_y) * cc.factor}, {1365, 240}, brush<color::black>{});
        } else {
          main::d2d_context->SetTransform(D2D1::Matrix3x2F::Rotation(90.f, D2D1::Point2F(1175.f, 240.f)));
          bm.draw(vector(1125, 230, 1225, 250));
          main::d2d_context->SetTransform((D2D1::Matrix3x2F::Identity()));
          main::d2d_context->DrawLine({1275 + lrmm[1].x * cc.factor, 350 - (lrmm[1].y - off_y) * cc.factor}, {1185, 240}, brush<color::black>{});
        }
      }
      { // 右側最大
        lrmm[2] = MaxMin.maxs[9];
        if (kaku == 2 && lrmm[2].w < MaxMin.maxs[10].w) lrmm[2] = MaxMin.maxs[10];
        if (lrmm[2].w < MaxMin.maxs[21].w) lrmm[2] = MaxMin.maxs[21];
        if (kaku == 2 && lrmm[2].w < MaxMin.maxs[22].w) lrmm[2] = MaxMin.maxs[22];
        if (MaxMin.maxs[4].z < 0 && lrmm[2].w < MaxMin.maxs[4].w) lrmm[2] = MaxMin.maxs[4];
        if (MaxMin.maxs[16].z < 0 && lrmm[2].w < MaxMin.maxs[16].w) lrmm[2] = MaxMin.maxs[16];
        yw::bitmap bm(nat4(rr.z - rr.x), nat4(rr.w - rr.y));
        Result.end_draw();
        bm.begin_draw(color::white);
        brush(getcolor(lrmm[2].w)).draw_rounded_rectangle(rr, 2.5f, 2.5f);
        brush<color::black>{}.draw_text(rr, f2, std::format(L"{:.2f}", lrmm[2].w));
        bm.end_draw();
        Result.begin_draw();
        if (lrmm[2].x > 0) {
          main::d2d_context->SetTransform(D2D1::Matrix3x2F::Rotation(-90.f, D2D1::Point2F(25.f, 360.f)));
          bm.draw(vector(-25, 350, 75, 370));
          main::d2d_context->SetTransform((D2D1::Matrix3x2F::Identity()));
          main::d2d_context->DrawLine({125 - lrmm[2].x * cc.factor, 350 - (lrmm[2].y - off_y) * cc.factor}, {35, 360}, brush<color::black>{});
        } else {
          main::d2d_context->SetTransform(D2D1::Matrix3x2F::Rotation(-90.f, D2D1::Point2F(225.f, 360.f)));
          bm.draw(vector(175, 350, 275, 370));
          main::d2d_context->SetTransform((D2D1::Matrix3x2F::Identity()));
          main::d2d_context->DrawLine({125 - lrmm[2].x * cc.factor, 350 - (lrmm[2].y - off_y) * cc.factor}, {215, 360}, brush<color::black>{});
        }
      }
      { // 右側最小
        lrmm[3] = MaxMin.mins[9];
        if (kaku == 2 && lrmm[3].w > MaxMin.mins[10].w) lrmm[3] = MaxMin.mins[10];
        if (lrmm[3].w > MaxMin.mins[21].w) lrmm[3] = MaxMin.mins[21];
        if (kaku == 2 && lrmm[3].w > MaxMin.mins[22].w) lrmm[3] = MaxMin.mins[22];
        if (MaxMin.mins[4].z < 0 && lrmm[3].w > MaxMin.mins[4].w) lrmm[3] = MaxMin.mins[4];
        if (MaxMin.mins[16].z < 0 && lrmm[3].w > MaxMin.mins[16].w) lrmm[3] = MaxMin.mins[16];
        yw::bitmap bm(nat4(rr.z - rr.x), nat4(rr.w - rr.y));
        Result.end_draw();
        bm.begin_draw(color::white);
        brush(getcolor(lrmm[3].w)).draw_rounded_rectangle(rr, 2.5f, 2.5f);
        brush<color::black>{}.draw_text(rr, f2, std::format(L"{:.2f}", lrmm[3].w));
        bm.end_draw();
        Result.begin_draw();
        if (lrmm[3].x > 0) {
          main::d2d_context->SetTransform(D2D1::Matrix3x2F::Rotation(-90.f, D2D1::Point2F(25.f, 240.f)));
          bm.draw(vector(-25, 230, 75, 250));
          main::d2d_context->SetTransform((D2D1::Matrix3x2F::Identity()));
          main::d2d_context->DrawLine({125 - lrmm[3].x * cc.factor, 350 - (lrmm[3].y - off_y) * cc.factor}, {35, 240}, brush<color::black>{});
        } else {
          main::d2d_context->SetTransform(D2D1::Matrix3x2F::Rotation(-90.f, D2D1::Point2F(225.f, 240.f)));
          bm.draw(vector(175, 230, 275, 250));
          main::d2d_context->SetTransform((D2D1::Matrix3x2F::Identity()));
          main::d2d_context->DrawLine({125 - lrmm[3].x * cc.factor, 350 - (lrmm[3].y - off_y) * cc.factor}, {215, 240}, brush<color::black>{});
        }
      }
      if (auto w = min(MaxMin.mins[12].w, MaxMin.mins[13].w); w < 10.f) { // TOP基準面+バックチャンファ
        const auto& a = MaxMin.mins[12].w == w ? MaxMin.mins[12] : MaxMin.mins[13];
        constexpr const auto& v = views[1];
        constexpr auto xh = (v.x + v.z) / 2, yh = (v.y + v.w) / 2;
        constexpr auto r = vector{xh - 50, v.y + 5, xh + 50, v.y + 25}; // views[1]の上部
        brush(getcolor(a.w)).draw_rounded_rectangle(r, 2.5f, 2.5f);
        brush<color::black>{}.draw_text(r, f2, std::format(L"{:.2f}", a.w));
        brush<color::black>{}.draw_line(vector{xh, r.w, xh - a.z * cc.factor, yh - (a.y - off_y) * cc.factor}, 1.0f);
      }
      if (MaxMin.mins[14].w < 10.f) { // TOP内股
        const auto& a = MaxMin.mins[14];
        constexpr const auto& v = views[2];
        constexpr auto xh = (v.x + v.z) / 2, yh = (v.y + v.w) / 2;
        constexpr auto r = vector{xh - 50, v.y + 5, xh + 50, v.y + 25}; // views[2]の上部
        brush(getcolor(a.w)).draw_rounded_rectangle(r, 2.5f, 2.5f);
        brush<color::black>{}.draw_text(r, f2, std::format(L"{:.2f}", a.w));
        brush<color::black>{}.draw_line(vector{xh, r.w, xh + a.z * cc.factor, yh - (a.y - off_y) * cc.factor}, 1.0f);
      }
      if (MaxMin.mins[2].w < 10.f) { // BOT内股
        const auto& a = MaxMin.mins[2];
        constexpr const auto& v = views[3];
        constexpr auto xh = (v.x + v.z) / 2, yh = (v.y + v.w) / 2;
        constexpr auto r = vector{xh - 50, v.y + 5, xh + 50, v.y + 25}; // views[3]の上部
        brush(getcolor(a.w)).draw_rounded_rectangle(r, 2.5f, 2.5f);
        brush<color::black>{}.draw_text(r, f2, std::format(L"{:.2f}", a.w));
        brush<color::black>{}.draw_line(vector{xh, r.w, xh - a.z * cc.factor, yh - (a.y - off_y) * cc.factor}, 1.0f);
      }
      if (auto w = min(MaxMin.mins[0].w, MaxMin.mins[1].w); w < 10.f) { // BOT基準面
        const auto& a = MaxMin.mins[0].w == w ? MaxMin.mins[0] : MaxMin.mins[1];
        constexpr const auto& v = views[4];
        constexpr auto xh = (v.x + v.z) / 2, yh = (v.y + v.w) / 2;
        constexpr auto r = vector{xh - 50, v.y + 5, xh + 50, v.y + 25}; // views[4]の上部
        brush(getcolor(a.w)).draw_rounded_rectangle(r, 2.5f, 2.5f);
        brush<color::black>{}.draw_text(r, f2, std::format(L"{:.2f}", a.w));
        brush<color::black>{}.draw_line(vector{xh, r.w, xh + a.z * cc.factor, yh - (a.y - off_y) * cc.factor}, 1.0f);
      }
      if (auto w = min(MaxMin.mins[4].w, MaxMin.mins[16].w); w < 10.f) { // バックチャンファ
        const auto& a = MaxMin.mins[4].w == w ? MaxMin.mins[4] : MaxMin.mins[16];
        constexpr const auto& v = views[6];
        constexpr auto xh = (v.x + v.z) / 2, yh = (v.y + v.w) / 2;
        constexpr auto r = vector{xh - 50, v.w - 25, xh + 50, v.w - 5}; // views[6]の下部
        brush(getcolor(a.w)).draw_rounded_rectangle(r, 2.5f, 2.5f);
        brush<color::black>{}.draw_text(r, f2, std::format(L"{:.2f}", a.w));
        brush<color::black>{}.draw_line(vector{xh, r.y, xh - a.z * cc.factor, yh - a.x * cc.factor}, 1.0f);
      }
      if (auto w = min(MaxMin.mins[5].w, MaxMin.mins[17].w, MaxMin.mins[24].w); w < 10.f) { // ピントップ
        const auto& a = w == MaxMin.mins[5].w ? MaxMin.mins[5] : (w == MaxMin.mins[17].w ? MaxMin.mins[17] : MaxMin.mins[24]);
        constexpr const auto& v = views[9];
        constexpr auto xh = (v.x + v.z) / 2, yh = (v.y + v.w) / 2;
        constexpr auto r = vector{xh - 50, v.w - 25, xh + 50, v.w - 5}; // views[9]の下部
        brush(getcolor(a.w)).draw_rounded_rectangle(r, 2.5f, 2.5f);
        brush<color::black>{}.draw_text(r, f2, std::format(L"{:.2f}", a.w));
        brush<color::black>{}.draw_line(vector{xh, r.y, xh + a.z * cc.factor, yh - a.x * cc.factor}, 1.0f);
      }
    }
    { // 情報欄を表示する
      brush<color::black>{}.draw_rectangle(vector(10, 860, 1390, 940), 1.0f);
      static constexpr vector rects[8] = {
        {10, 860, 110, 890},
        {110, 860, 260, 890},
        {260, 860, 410, 890},
        {410, 860, 560, 890},
        {560, 860, 760, 890},
        {760, 860, 960, 890},
        {960, 860, 1190, 890},
        {1190, 860, 1390, 890}};
      static constexpr stv2 texts[] = {L"判定", L"工程", L"型入鍛造", L"芯出", L"O. No.", L"Ch. No.", L"品名／型式", L"所内図番"};
      static constexpr auto font = yw::font<20, L"Yu Gothic UI", 0>{};
      cfor<0, [](natt i) { return i < 8; }>([](auto&& a, auto&& b) { brush<color::black>{}.draw_text(a, font, b); }, rects, texts);
      cfor<0, [](natt i) { return i < 8; }>([](auto&& a) { brush<color::black>{}.draw_rectangle(a, 1.0f); }, rects);
    }
    { // 情報欄を表示する (2段目)
      static constexpr vector rects[8] = {
        {10, 890, 110, 940},
        {110, 890, 260, 940},
        {260, 890, 410, 940},
        {410, 890, 560, 940},
        {560, 890, 760, 940},
        {760, 890, 960, 940},
        {960, 890, 1190, 940},
        {1190, 890, 1390, 940}};
      cfor<0, [](natt i) { return i < 8; }>([](auto&& a) { brush<color::black>{}.draw_rectangle(a, 1.0f); }, rects);
    }
    { // 文字表示
      static constexpr auto font = yw::font<15, L"Yu Gothic UI", 0>{};
      brush<color::black>{}.draw_text(vector(330, 580, 380, 600), font, L"矢視A");
      brush<color::black>{}.draw_text(vector(560, 580, 610, 600), font, L"矢視B");
      brush<color::black>{}.draw_text(vector(790, 580, 840, 600), font, L"矢視C");
      brush<color::black>{}.draw_text(vector(1020, 580, 1070, 600), font, L"矢視D");
      brush<color::black>{}.draw_text(vector(100, 600, 150, 620), font, L"矢視E");
      brush<color::black>{}.draw_text(vector(330, 600, 380, 620), font, L"矢視F");
      brush<color::black>{}.draw_text(vector(560, 600, 610, 620), font, L"矢視G");
      brush<color::black>{}.draw_text(vector(790, 600, 840, 620), font, L"矢視H");
      main::d2d_context->SetTransform(D2D1::Matrix3x2F::Rotation(-90.f, D2D1::Point2F(0, 0)));
      brush<color::black>{}.draw_text(vector(-420, 15, -450, 35), font, L"↓A");
      brush<color::black>{}.draw_text(vector(-420, 115, -450, 135), font, L"↑B");
      brush<color::black>{}.draw_text(vector(-460, 115, -490, 135), font, L"↓C");
      brush<color::black>{}.draw_text(vector(-460, 215, -490, 235), font, L"↑D");
      brush<color::black>{}.draw_text(vector(-560, 115, -590, 135), font, L"E→");
      brush<color::black>{}.draw_text(vector(-110, 115, -140, 135), font, L"←H");
      brush<color::black>{}.draw_text(vector(-(340 + off_y * cc.factor), 345, -(310 + off_y * cc.factor), 365), font, L"F→");
      brush<color::black>{}.draw_text(vector(-(360 + off_y * cc.factor), 345, -(390 + off_y * cc.factor), 365), font, L"←G");
      brush<color::black>{}.draw_text(vector(-590, 70, -560, 90), font, L"TOP");
      brush<color::black>{}.draw_text(vector(-590, 160, -560, 180), font, L"BOT");
      main::d2d_context->SetTransform(D2D1::Matrix3x2F::Rotation(90.f, D2D1::Point2F(0, 0)));
      brush<color::black>{}.draw_text(vector(590, -1310, 560, -1330), font, L"TOP");
      brush<color::black>{}.draw_text(vector(590, -1220, 560, -1240), font, L"BOT");
      main::d2d_context->SetTransform((D2D1::Matrix3x2F::Identity()));
    }
    { // 最小取代が5mm以下のときはその位置を表示する
      static constexpr auto ff = font<18, L"Yu Gothic UI", 0, true>{};
      static constexpr int4 dd = 10;
      vector r;
      vector temp(std::abs(minimum_n.x), std::abs(minimum_n.y), std::abs(minimum_n.z));
      intt hoko = temp.x < temp.y ? (temp.y < temp.z ? 3 : 2) : (temp.x < temp.z ? 3 : 1);
      if (minimum_n[hoko - 1] < 0) hoko = -hoko;
      if ((hoko == 1 && minimum.x > 0) || minimum_i == 12 || minimum_i == 13) { // TOP基準面 or バックチャンファ
        array<fat4, 2> v2{355 + minimum.z * cc.factor,
                          350 + (off_y - minimum.y) * cc.factor};
        r = vector(int4(v2[0] - dd), int4(v2[1] - dd), int4(v2[0] + dd), int4(v2[1] + dd));
      } else if ((hoko == -1 && minimum.x < 0) || minimum_i == 0 || minimum_i == 1) { // BOT基準面 or バックチャンファ
        array<fat4, 2> v2{1045 - minimum.z * cc.factor,
                          350 + (off_y - minimum.y) * cc.factor};
        r = vector(int4(v2[0] - dd), int4(v2[1] - dd), int4(v2[0] + dd), int4(v2[1] + dd));
      } else if ((hoko == -1 && minimum.x > 0) || minimum_i == 14) { // TOP内股
        array<fat4, 2> v2{585 + minimum.z * cc.factor,
                          350 + (off_y - minimum.y) * cc.factor};
        r = vector(int4(v2[0] - dd), int4(v2[1] - dd), int4(v2[0] + dd), int4(v2[1] + dd));
      } else if ((hoko == 1 && minimum.x < 0) || minimum_i == 2) { // BOT内股
        array<fat4, 2> v2{815 - minimum.z * cc.factor,
                          350 + (off_y - minimum.y) * cc.factor};
        r = vector(int4(v2[0] - dd), int4(v2[1] - dd), int4(v2[0] + dd), int4(v2[1] + dd));
      } else if (minimum_i == 3 || minimum_i == 15) { // ジャーナル穴
        if (minimum.y > 0) {
          array<fat4, 2> v2{355 - minimum.z * cc.factor,
                            725 - minimum.x * cc.factor};
          r = vector(int4(v2[0] - dd), int4(v2[1] - dd), int4(v2[0] + dd), int4(v2[1] + dd));
        } else {
          array<fat4, 2> v2{585 + minimum.z * cc.factor,
                            725 - minimum.x * cc.factor};
          r = vector(int4(v2[0] - dd), int4(v2[1] - dd), int4(v2[0] + dd), int4(v2[1] + dd));
        }
      } else if (minimum_i == 5 || minimum_i == 17) { // ピントップ
        array<fat4, 2> v2{815 + minimum.z * cc.factor,
                          725 - minimum.x * cc.factor};
        r = vector(int4(v2[0] - dd), int4(v2[1] - dd), int4(v2[0] + dd), int4(v2[1] + dd));
      } else if (minimum.z < 0) { // 右サイド
        array<fat4, 2> v2{125 - minimum.x * cc.factor,
                          350 + (off_y - minimum.y) * cc.factor};
        r = vector(int4(v2[0] - dd), int4(v2[1] - dd), int4(v2[0] + dd), int4(v2[1] + dd));
      } else { // 左サイド
        array<fat4, 2> v2{1275 + minimum.x * cc.factor,
                          350 + (off_y - minimum.y) * cc.factor};
        r = vector(int4(v2[0] - dd), int4(v2[1] - dd), int4(v2[0] + dd), int4(v2[1] + dd));
      }
      brush<color::black>{}.draw_text(r, ff, L"◇");
    }
    Result.end_draw();
  } catch (const std::exception& E) { throw except(E); }
}


array<vector, 15> calc_punch(const array<vector, 15>& Ref) {
  static unordered_buffer<array<fat4, 15>> ub1(nullptr, 1024);
  array<vector, 15> out{Ref};
  nat4 count = Vertex[0].count;
  unordered_buffer<array<fat4, 15>> ub0(nullptr, count);
  constant_buffer<list<nat4, nat4, nat8, array<vector, 15>>> cb(list{count, (count - 1) / 1024 + 1, 0_n8, Ref});
  {
    static constexpr stv1 hlsl = R"(
    #pragma pack_matrix(row_major)
    struct UB { float d[15]; };
    struct SB { float4 v, n; };
    cbuffer CB : register(b0) { uint m, n, _2, _3; float4 cb[15]; };
    cbuffer CB1 : register(b1) { matrix mm; };
    RWStructuredBuffer<UB> ub : register(u0);
      StructuredBuffer<SB> sb : register(t0);
    float f0(float3 a, float3 b, float3 c) {
      float t = a.x * b.y - a.y * b.x;
      if (t < 0) {
        t = b.x * c.y - b.y * c.x;
        if (t > 0) return 0.0;
        else if (t < 0) {
          t = c.x * a.y - c.y * a.x;
          if (t > 0) return 0.0;
        }
      } else if (t > 0) {
        t = b.x * c.y - b.y * c.x;
        if (t < 0) return 0.0;
        else if (t > 0) {
          t = c.x * a.y - c.y * a.x;
          if (t < 0) return 0.0;
        }
      }
      return dot(cross(a, b), c) / (a.x * b.y - a.y * b.x + b.x * c.y - b.y * c.x + c.x * a.y - c.y * a.x);
    }
    [numthreads(1024, 1, 1)] void csmain(uint i : SV_DispatchThreadID) {
      uint j = 3 * i;
      float3 a = mul(mm, sb[j].v).xyz, b = mul(mm, sb[j + 1].v).xyz, c = mul(mm, sb[j + 2].v).xyz;
      for (uint k{}; k < 15; ++k) {
        if (abs(cb[k].w) == 1) ub[i].d[k] = f0(a.yzx - cb[k].yzx, b.yzx - cb[k].yzx, c.yzx - cb[k].yzx);
        else if (abs(cb[k].w) == 2) ub[i].d[k] = f0(a.zxy - cb[k].zxy, b.zxy - cb[k].zxy, c.zxy - cb[k].zxy);
        else ub[i].d[k] = f0(a.xyz - cb[k].xyz, b.xyz - cb[k].xyz, c.xyz - cb[k].xyz);
      }
    })";
    static auto gp = gpgpu<typepack<array<fat4, 15>>, typepack<vertex>, typepack<decltype(cb)::value_type, xmatrix>>(hlsl);
    static constant_buffer<xmatrix> cb1(xv_identity);
    std::cout << std::format("{} - {}\n", Offsets, Degrees);
    cb1.from([&](xmatrix& m) { xvworld(xvnegate<1, 0, 1, 0>(Offsets), xvnegate<1, 0, 1, 0>(xvradian(Degrees)), m); });
    std::cout << std::format("{}\n", cb1.to_cpu()[0]);
    gp(count, {ub0}, {Vertex[0]}, {cb, cb1});
  }
  {
    static constexpr stv1 hlsl = R"(
#pragma pack_matrix(row_major)
struct UB { float d[15]; };
cbuffer CB : register(b0) { uint m, n, _2, _3; float4 cb[15]; };
RWStructuredBuffer<UB> ub0 : register(u0);
RWStructuredBuffer<UB> ub1 : register(u1);
[numthreads(1024, 1, 1)] void csmain(uint i : SV_DispatchThreadID) {
  for (uint k = 0; k < 15; ++k) ub1[i].d[k] = 0.0f;
  for (uint j = i * n, jj = j + n; j < jj && j < m; ++j) {
    for (uint k = 0; k < 15; ++k) {
      if (cb[k].w < 0) ub1[i].d[k] = min(ub1[i].d[k], ub0[j].d[k]);
      else ub1[i].d[k] = max(ub1[i].d[k], ub0[j].d[k]);
    }
  }
})";
    static auto gp = gpgpu<typepack<array<fat4, 15>, array<fat4, 15>>, typepack<>, typepack<decltype(cb)::value_type>>(hlsl);
    static auto st = staging_buffer<array<fat4, 15>>(1024);
    gp(1024, {ub0, ub1}, {}, {cb});
    auto temp = ub1.to_cpu(st);
    // for (auto& e : temp) std::cout << e[0] << std::endl;
    for (natt i{512}; i != 0; i >>= 1)
      for (natt j{}; j < i; ++j)
        for (natt k{}; k < 15; ++k) {
          if (Ref[k].w < 0) temp[j][k] = min(temp[j][k], temp[j + i][k]);
          else temp[j][k] = max(temp[j][k], temp[j + i][k]);
        }
    for (natt k{}; k < 15; ++k) {
      if (std::abs(Ref[k].w) == 1) out[k].x = temp[0][k];
      else if (std::abs(Ref[k].w) == 2) out[k].y = temp[0][k];
      else if (std::abs(Ref[k].w) == 3) out[k].z = temp[0][k];
    }
  }
  return out;
}


inline array<vector, 15> calc_punch(const structured_buffer<vertex>& Vertex, const array<vector, 15>& Ref) {
  array<vector, 15> out{Ref};
  unordered_buffer<array<fat4, 15>> ub0(nullptr, Vertex.count);
  unordered_buffer<array<fat4, 15>> ub1(nullptr, 1024);
  constant_buffer<list<nat4, nat4, nat8, array<vector, 15>>> cb(list{
    nat4(Vertex.count), nat4((Vertex.count - 1) / 1024 + 1), 0_n8, Ref});
  try { // STEP 1
    static constexpr stv1 hlsl = R"(
    #pragma pack_matrix(row_major)
    struct UB { float d[15]; };
    struct SB { float4 v, n; };
    cbuffer CB : register(b0) {
      uint m, n, _2, _3;
      float4 cb[15];
    };
    cbuffer CB1 : register(b1) {
      matrix mm;
    };
    RWStructuredBuffer<UB> ub : register(u0);
      StructuredBuffer<SB> sb : register(t0);
    float f0(float3 a, float3 b, float3 c) {
      float t = a.x * b.y - a.y * b.x;
      if (t < 0) {
        t = b.x * c.y - b.y * c.x;
        if (t > 0) return 0.0;
        else if (t < 0) {
          t = c.x * a.y - c.y * a.x;
          if (t > 0) return 0.0;
        }
      } else if (t > 0) {
        t = b.x * c.y - b.y * c.x;
        if (t < 0) return 0.0;
        else if (t > 0) {
          t = c.x * a.y - c.y * a.x;
          if (t < 0) return 0.0;
        }
      }
      return dot(cross(a, b), c) / (a.x * b.y - a.y * b.x + b.x * c.y - b.y * c.x + c.x * a.y - c.y * a.x);
    }
    [numthreads(1024, 1, 1)] void csmain(uint i : SV_DispatchThreadID) {
      uint j = 3 * i;
      for (uint k{}; k < 15; ++k) {
        if (abs(cb[k].w) == 1) ub[i].d[k] = f0(sb[j].v.yzx - cb[k].yzx, sb[j + 1].v.yzx - cb[k].yzx, sb[j + 2].v.yzx - cb[k].yzx);
        else if (abs(cb[k].w) == 2) ub[i].d[k] = f0(sb[j].v.zxy - cb[k].zxy, sb[j + 1].v.zxy - cb[k].zxy, sb[j + 2].v.zxy - cb[k].zxy);
        else ub[i].d[k] = f0(sb[j].v.xyz - cb[k].xyz, sb[j + 1].v.xyz - cb[k].xyz, sb[j + 2].v.xyz - cb[k].xyz);
      }
    })";
    static auto gp = gpgpu<typepack<array<fat4, 15>>, typepack<vertex>, typepack<decltype(cb)::value_type>>(hlsl);
    gp(Vertex.count, {ub0}, {Vertex}, {cb});
  } catch (const std::exception& E) { throw except(E); }
  try { // STEP 2
    static constexpr stv1 hlsl = R"(
#pragma pack_matrix(row_major)
struct UB { float d[15]; };
struct SB { float d[15]; };
cbuffer CB : register(b0) {
  uint m, n, _2, _3;
  float4 cb[15];
};
RWStructuredBuffer<UB> ub : register(u0);
  StructuredBuffer<SB> sb : register(t0);
[numthreads(1024, 1, 1)] void csmain(uint i : SV_DispatchThreadID) {
  for (uint k = 0; k < 15; ++k) ub[i].d[k] = 0.0f;
  for (uint j = i * n, jj = j + n; j < jj && j < m; ++j) {
    for (uint k = 0; k < 15; ++k) {
      if (cb[k].w < 0) ub[i].d[k] = min(ub[i].d[k], sb[j].d[k]);
      else ub[i].d[k] = max(ub[i].d[k], sb[j].d[k]);
    }
  }
})";
    static auto gp = gpgpu<typepack<array<fat4, 15>>, typepack<array<fat4, 15>>, typepack<decltype(cb)::value_type>>(hlsl);
    auto sb = structured_buffer(ub0);
    gp(1024, {ub1}, {sb}, {cb});
  } catch (const std::exception& E) { throw except(E); }
  try { // STEP 3
    auto temp = ub1.to_cpu();
    for (natt i{512}; i != 0; i >>= 1) {
      for (natt j{}; j < i; ++j) {
        for (natt k{}; k < 15; ++k) {
          if (Ref[k].w < 0) temp[j][k] = min(temp[j][k], temp[j + i][k]);
          else temp[j][k] = max(temp[j][k], temp[j + i][k]);
        }
      }
    }
    for (natt k{}; k < 15; ++k) {
      if (std::abs(Ref[k].w) == 1) out[k].x = temp[0][k];
      else if (std::abs(Ref[k].w) == 2) out[k].y = temp[0][k];
      else if (std::abs(Ref[k].w) == 3) out[k].z = temp[0][k];
    }

  } catch (const std::exception& E) { throw except(E); }
  return out;
}


void output_punch(const path& Path, array<vector, 15> Points, const nat4 (&Arrange)[15]) {
  static constexpr auto ff = [](fat4 f) {
    if (f == 1) return vector(1, 0, 0);
    else if (f == 2) return vector(0, 1, 0);
    else if (f == 3) return vector(0, 0, 1);
    else if (f == -1) return vector(-1, 0, 0);
    else if (f == -2) return vector(0, -1, 0);
    else if (f == -3) return vector(0, 0, -1);
    else return vector{};
  };
  try {
    for (natt i{}; i < 15; ++i) std::cout << std::format("{}\n", Points[i]);
    std::ofstream ofs(Path);
    ofs << R"("type";"name";"coord-x";"coord-y";"coord-z";"coord-x2";"coord-y2";"coord-z2";"normal-x";"normal-y";"normal-z";"trimming-x";"trimming-y";"trimming-z";"dir-x";"dir-y";"dir-z";"length";"width";"radius";"radius2";"angle";"orientation";"edge-radius";"num-points";"tol-x-lower";"tol-x-upper";"tol-y-lower";"tol-y-upper";"tol-z-lower";"tol-z-upper";"tol-all-lower";"tol-all-upper";"tol-normal-lower";"tol-normal-upper";"tol-trimming-lower";"tol-trimming-upper";"tol-inplane-lower";"tol-inplane-upper";"tol-length-lower";"tol-length-upper";"tol-width-lower";"tol-width-upper";"tol-diameter-lower";"tol-diameter-upper";"tol-angle-lower";"tol-angle-upper";"tol-xy-lower";"tol-xy-upper";"tol-xz-lower";"tol-xz-upper";"tol-yz-lower";"tol-yz-upper";"coord-x3";"coord-y3";"coord-z3";"tol-diameter2-lower";"tol-diameter2-upper")";
    ofs << std::endl;
    for (natt i{}; i < 15; ++i) {
      auto nn = ff(Points[i].w);
      ofs << std::format(R"("inspection_surface_point";"point {}";"{}";"{}";"{}";"";"";"";"{}";"{}";"{}";"";"";"";"";"";"";"";"";"";"";"";"";"";"";"";"";"";"";"";"";"";"";"";"";"";"";"";"";"";"";"";"";"";"";"";"";"";"";"";"";"";"";"";"";"";"";")",
                         Arrange[i], Points[i].x, Points[i].y, Points[i].z, nn.x, nn.y, nn.z)
          << std::endl;
    }
  } catch (const std::exception& E) { throw except(E); }
}


array<ui::textbox, 8> edits;


void result_mode() {
  static constexpr vector rects[8] = {
    {10, 890, 110, 940},
    {110, 890, 260, 940},
    {260, 890, 410, 940},
    {410, 890, 560, 940},
    {560, 890, 760, 940},
    {760, 890, 960, 940},
    {960, 890, 1190, 940},
    {1190, 890, 1390, 940}};
  main::resize(1400, 980), create_result();

  edits[0] = ui::textbox(rects[0].to_rect(), L"", ES_CENTER | WS_BORDER);
  edits[1] = ui::textbox(rects[1].to_rect(), L"", ES_CENTER | WS_BORDER);
  edits[2] = ui::textbox(rects[2].to_rect(), L"", ES_CENTER | WS_BORDER);
  edits[3] = ui::textbox(rects[3].to_rect(), L"", ES_CENTER | WS_BORDER);
  edits[4] = ui::textbox(rects[4].to_rect(), L"", ES_CENTER | WS_BORDER);
  edits[5] = ui::textbox(rects[5].to_rect(), L"", ES_CENTER | WS_BORDER);
  edits[6] = ui::textbox(rects[6].to_rect(), L"", ES_CENTER | WS_BORDER);
  edits[7] = ui::textbox(rects[7].to_rect(), L"", ES_CENTER | WS_BORDER);

  ui::button output_button{rect{1240, 800, 1390, 820}, L"成績表を保存"};
  ui::button back_button{rect{1240, 825, 1390, 845}, L"成績表モード終了"};
  back_button.enter = [](const ui::button&) { ResultMode = false; };

  output_button.enter = [](const ui::button&) {
    main::begin_draw();
    for (natt i{}; i < 8; ++i) brush<color::black>{}.draw_text(rects[i] + vector(0, 10, 0, 0), font<20, L"Yu Gothic UI", 0, true>{}, edits[i].text());
    main::end_draw();
    auto p = ui::save_file(cmt_file.parent_path(), std::format(L"{}.png", edits[5].text().data()));
    if (!p.empty()) {
      static constexpr nat4 arrange[] = {14, 13, 12, 15, 11, 9, 10, 4, 3, 2, 1, 8, 5, 7, 6};
      auto par = reinterpret_cast<const fat4*>(StlFacet.header().data() + 16);
      array<vector, 15> ref{
        vector(0.0f, -RadiusJH, 0.0f, 1.0f),
        vector(0.0f, 0.0f, RadiusJH, 1.0f),
        vector(0.0f, RadiusJH, 0.0f, 1.0f),
        vector(0.0f, 0.0f, -RadiusJH, 1.0f),
        vector(0.0f, HalfGap, 0.0f, 1.0f),
        vector(-HalfStroke, 0.0f, 0.0f, -3.0f),
        vector(-HalfStroke, HalfGap, 0.0f, -3.0f),
        vector(-HalfStroke, 0.0f, 0.0f, 3.0f),
        vector(-HalfStroke, HalfGap, 0.0f, 3.0f),
        vector(-HalfStroke, 0.0f, 0.0f, 2.0f),
        vector(HalfStroke, 0.0f, 0.0f, 2.0f),
        vector(-HalfStroke, 0.0f, 0.0f, -2.0f),
        vector(HalfStroke, 0.0f, 0.0f, -2.0f),
        vector(-HalfStroke, 0.0f, 50.0f, -2.0f),
        vector(HalfStroke, 0.0f, -50.0f, -2.0f)};
      output_punch(cmt_file.parent_path() / cmt_file.stem().concat(L".csv"), calc_punch(ref), arrange);
      // if (cb_reverse()) output_punch(cmt_file.parent_path() / cmt_file.stem().concat(L".csv"), calc_punch(Vertex, ref_rev), arrange, true);
      // else output_punch(cmt_file.parent_path() / cmt_file.stem().concat(L".csv"), calc_punch(Vertex, ref), arrange, false);
      main::screenshot(p);
      if (p.exists()) ui::ok(L"出力に成功しました");
      else ui::ok(L"出力に失敗したかもしれません");
    }
  };

  while (main::update && ResultMode) {
    main::begin_draw(color::white);
    Result.draw(vector{0, 0, main::width, main::height});
    main::end_draw();
  }

  for (auto& e : edits) e = {};
}
