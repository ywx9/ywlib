#pragma once
#include "ywx/drawing.h"

namespace yw {

/// MARK: svgpath

class svgpath : public handle_base {
public:
  struct slot : public handle_base::slot {
    comptr<ID2D1PathGeometry> geometry;
    color fill_color = colors::black;
    color stroke_color = colors::black;
    D2D1_RECT_F bounds{};
    float2 size{};
    float stroke_width = 1.0f;

    std::expected<comptr<ID2D1PathGeometry>, error> parse_svg_path(string_view<char> Path);
  };

  explicit operator bool() const noexcept {
    const auto sp = slot::get_as<svgpath>(id());
    return sp && static_cast<bool>(sp->geometry);
  }

  auto d2d_geometry(this auto&& self) noexcept -> ID2D1PathGeometry* {
    if (const auto sp = slot::template get_as<svgpath>(self.id())) return sp->geometry.get();
    else return nullptr;
  }

  float4 bounds() const noexcept {
    if (const auto sp = slot::get_as<svgpath>(id()); !sp) {
      error(errors::invalid_slotid, "invalid slotid").fizzle_out();
      return {};
    } else return std::bit_cast<float4>(sp->bounds);
  }

  float2 size() const noexcept {
    if (const auto sp = slot::get_as<svgpath>(id()); !sp) {
      error(errors::invalid_slotid, "invalid slotid").fizzle_out();
      return {};
    } else return sp->size;
  }

  svgpath() noexcept = default;
  svgpath(svgpath&& Other) noexcept = default;
  svgpath& operator=(svgpath&& Other) noexcept = default;

  std::expected<svgpath, error> create(float2 Size, string_view<char> Svg) {
    const auto sp = make_slot<svgpath>();
    if (!sp) return std::unexpected(error(errors::slot_creation_failed));
    if (auto res = sp->parse_svg_path(Svg)) {
      sp->geometry = std::move(*res);
      sp->size = Size;
      sp->geometry->GetBounds(nullptr, &sp->bounds);
      return make_handle<svgpath>(sp->id);
    } else return res.error().relay();
  }

  svgpath(float2 Size, string_view<char> Svg, const source_line& sl = here()) {
    if (auto res = create(Size, Svg); !res) res.error().add_footprint().go_off(sl);
    else *this = std::move(*res);
  }

  std::expected<svgpath, error> create(const svgpath& Other) {
    const auto sp = make_slot<svgpath>();
    if (!sp) return std::unexpected(error(errors::slot_creation_failed));
    const auto osp = slot::get_as<svgpath>(Other.id());
    if (!osp) return std::unexpected(error(errors::invalid_slotid));
    comptr<ID2D1PathGeometry> new_geometry;
    hresult_test(d2d::factory()->CreatePathGeometry, &new_geometry.get());
    comptr<ID2D1GeometrySink> sink;
    hresult_test(new_geometry->Open, &sink.get());
    if (const auto hr = osp->geometry->Simplify({}, nullptr, sink.get()); FAILED(hr)) {
      sink.release();
      return std::unexpected(error(errors::operation_failed, "Simplify failed", int32_t(hr)));
    }
    const auto hr = sink->Close();
    sink.release();
    if (FAILED(hr)) return std::unexpected(error(errors::operation_failed, "Close sink failed", int32_t(hr)));
    sp->geometry = std::move(new_geometry);
    sp->size = osp->size;
    sp->bounds = osp->bounds;
    return make_handle<svgpath>(sp->id);
  }

  svgpath(const svgpath& Other, const source_line& sl = here()) {
    if (auto res = create(Other); !res) res.error().add_footprint().go_off(sl);
    else *this = std::move(*res);
  }

  //-- getter --//

  const color& fill_color() const noexcept {
    const auto sp = slot::get_as<svgpath>(id());
    if (!sp) error(errors::invalid_slotid, "invalid slotid").go_off();
    return sp->fill_color;
  }

  const color& stroke_color() const noexcept {
    const auto sp = slot::get_as<svgpath>(id());
    if (!sp) error(errors::invalid_slotid, "invalid slotid").go_off();
    return sp->stroke_color;
  }

  float stroke_width() const noexcept {
    if (const auto sp = slot::get_as<svgpath>(id()); !sp) {
      error(errors::invalid_slotid, "invalid slotid").fizzle_out();
      return 0.0f;
    } else return sp->stroke_width;
  }

  //-- setter --//

  auto& fill_color(this auto& self, const color& c) noexcept {
    if (const auto sp = slot::get_as<svgpath>(self.id())) sp->fill_color = c;
    else error(errors::invalid_slotid, "invalid slotid").fizzle_out();
    return self;
  }

  auto& stroke_color(this auto& self, const color& c) noexcept {
    if (const auto sp = slot::get_as<svgpath>(self.id())) sp->stroke_color = c;
    else error(errors::invalid_slotid, "invalid slotid").fizzle_out();
    return self;
  }

  auto& stroke_width(this auto& self, float1 w) noexcept {
    if (const auto sp = slot::get_as<svgpath>(self.id())) sp->stroke_width = w.x;
    else error(errors::invalid_slotid, "invalid slotid").fizzle_out();
    return self;
  }
};

/// MARK: stroke_svgpath

inline std::expected<void, error> stroke_svgpath(
  float2 Pos, float2 Size, const svgpath& Path) {
  if (!drawing::d2d_drawing()) return std::unexpected(error(errors::invalid_operation, "drawing not begun"));
  const auto sp = svgpath::slot::get_as<svgpath>(Path.id());
  if (!sp) return std::unexpected(error(errors::invalid_slotid));
  const float2 scale = Size / sp->size;
  comptr<ID2D1TransformedGeometry> tg;
  D2D1_MATRIX_3X2_F matrix = D2D1::Matrix3x2F::Scale(scale.x, scale.y) * D2D1::Matrix3x2F::Translation(Pos.x, Pos.y);
  hresult_test(d2d::factory()->CreateTransformedGeometry, sp->geometry.get(), &matrix, &tg.get());
  brush::color(sp->stroke_color);
  d2d::context()->DrawGeometry(tg.get(), brush::d2d_brush(), sp->stroke_width, brush::d2d_stroke());
  return {};
}

inline std::expected<void, error> stroke_svgpath(float2 Pos, const svgpath& Path) {
  if (!drawing::d2d_drawing()) return std::unexpected(error(errors::invalid_operation, "drawing not begun"));
  const auto sp = svgpath::slot::get_as<svgpath>(Path.id());
  if (!sp) return std::unexpected(error(errors::invalid_slotid));
  comptr<ID2D1TransformedGeometry> tg;
  D2D1_MATRIX_3X2_F matrix = D2D1::Matrix3x2F::Translation(Pos.x, Pos.y);
  hresult_test(d2d::factory()->CreateTransformedGeometry, sp->geometry.get(), &matrix, &tg.get());
  brush::color(sp->stroke_color);
  d2d::context()->DrawGeometry(tg.get(), brush::d2d_brush(), sp->stroke_width, brush::d2d_stroke());
  return {};
}

/// MARK: fill_svgpath

inline std::expected<void, error> fill_svgpath(float2 Pos, float2 Size, const svgpath& Path) {
  if (!drawing::d2d_drawing()) return std::unexpected(error(errors::invalid_operation, "drawing not begun"));
  const auto sp = svgpath::slot::get_as<svgpath>(Path.id());
  if (!sp) return std::unexpected(error(errors::invalid_slotid));
  const float2 scale = Size / sp->size;
  comptr<ID2D1TransformedGeometry> tg;
  D2D1_MATRIX_3X2_F matrix = D2D1::Matrix3x2F::Scale(scale.x, scale.y) * D2D1::Matrix3x2F::Translation(Pos.x, Pos.y);
  hresult_test(d2d::factory()->CreateTransformedGeometry, sp->geometry.get(), &matrix, &tg.get());
  brush::color(sp->fill_color);
  d2d::context()->FillGeometry(tg.get(), brush::d2d_brush());
  return {};
}

inline std::expected<void, error> fill_svgpath(float2 Pos, const svgpath& Path) {
  if (!drawing::d2d_drawing()) return std::unexpected(error(errors::invalid_operation, "drawing not begun"));
  const auto sp = svgpath::slot::get_as<svgpath>(Path.id());
  if (!sp) return std::unexpected(error(errors::invalid_slotid));
  comptr<ID2D1TransformedGeometry> tg;
  D2D1_MATRIX_3X2_F matrix = D2D1::Matrix3x2F::Translation(Pos.x, Pos.y);
  hresult_test(d2d::factory()->CreateTransformedGeometry, sp->geometry.get(), &matrix, &tg.get());
  brush::color(sp->fill_color);
  d2d::context()->FillGeometry(tg.get(), brush::d2d_brush());
  return {};
}

/// MARK: draw_svgpath

inline std::expected<void, error> draw_svgpath(float2 Pos, float2 Size, const svgpath& Path) {
  if (!drawing::d2d_drawing()) return std::unexpected(error(errors::invalid_operation, "drawing not begun"));
  const auto sp = svgpath::slot::get_as<svgpath>(Path.id());
  if (!sp) return std::unexpected(error(errors::invalid_slotid));
  const float2 scale = Size / sp->size;
  comptr<ID2D1TransformedGeometry> tg;
  D2D1_MATRIX_3X2_F matrix = D2D1::Matrix3x2F::Scale(scale.x, scale.y) * D2D1::Matrix3x2F::Translation(Pos.x, Pos.y);
  hresult_test(d2d::factory()->CreateTransformedGeometry, sp->geometry.get(), &matrix, &tg.get());
  brush::color(sp->fill_color);
  d2d::context()->FillGeometry(tg.get(), brush::d2d_brush());
  brush::color(sp->stroke_color);
  d2d::context()->DrawGeometry(tg.get(), brush::d2d_brush(), sp->stroke_width, brush::d2d_stroke());
  return {};
}

inline std::expected<void, error> draw_svgpath(float2 Pos, const svgpath& Path) {
  if (!drawing::d2d_drawing()) return std::unexpected(error(errors::invalid_operation, "drawing not begun"));
  const auto sp = svgpath::slot::get_as<svgpath>(Path.id());
  if (!sp) return std::unexpected(error(errors::invalid_slotid));
  comptr<ID2D1TransformedGeometry> tg;
  D2D1_MATRIX_3X2_F matrix = D2D1::Matrix3x2F::Translation(Pos.x, Pos.y);
  hresult_test(d2d::factory()->CreateTransformedGeometry, sp->geometry.get(), &matrix, &tg.get());
  brush::color(sp->fill_color);
  d2d::context()->FillGeometry(tg.get(), brush::d2d_brush());
  brush::color(sp->stroke_color);
  d2d::context()->DrawGeometry(tg.get(), brush::d2d_brush(), sp->stroke_width, brush::d2d_stroke());
  return {};
}

/// MARK: parse_svg_path

inline std::expected<comptr<ID2D1PathGeometry>, error> svgpath::slot::parse_svg_path(string_view<char> Path) {
  comptr<ID2D1PathGeometry> geometry;
  hresult_test(d2d::factory()->CreatePathGeometry, &geometry.get());
  comptr<ID2D1GeometrySink> sink;
  hresult_test(geometry->Open, &sink.get());
  float current_x = 0.0f, current_y = 0.0f;
  float last_control_x = 0.0f, last_control_y = 0.0f;
  size_t pos = 0;
  char current_cmd = '\0';
  char last_cmd = '\0';
  bool use_relative = false;
  bool figure_open = false;

  auto skip_whitespace = [&]() {
    while (pos < Path.length() && std::isspace(Path[pos])) ++pos;
  };

  auto parse_number = [&]() -> std::expected<float, error> {
    skip_whitespace();
    if (pos < Path.length() && Path[pos] == '+') ++pos;
    if (pos >= Path.length()) return std::unexpected(error(errors::invalid_argument, "unexpected end of path"));
    float result;
    const auto fcr = std::from_chars(&Path[pos], &Path[Path.length()], result);
    if (fcr.ec != std::errc()) { return std::unexpected(error(errors::invalid_argument, "invalid number in path")); }
    pos += fcr.ptr - &Path[pos];
    return result;
  };

  auto parse_command = [&]() -> char {
    skip_whitespace();
    if (pos >= Path.length()) return '\0';
    char cmd = Path[pos];
    if (std::isalpha(cmd)) {
      ++pos;
      return cmd;
    } else return '\0';
  };

  while (pos < Path.length()) {
    skip_whitespace();
    if (pos >= Path.length()) break;
    char cmd = parse_command();
    if (cmd != '\0') {
      last_cmd = current_cmd;
      current_cmd = cmd;
      use_relative = std::islower(cmd);
      cmd = std::toupper(cmd);
    } else cmd = std::toupper(current_cmd);

    switch (cmd) {
    case 'M': { // moveto
      auto res_x = parse_number();
      if (!res_x) return res_x.error().relay();
      auto res_y = parse_number();
      if (!res_y) return res_y.error().relay();
      float x = *res_x, y = *res_y;
      if (use_relative) x += current_x, y += current_y;
      // End previous figure if one is open
      if (figure_open) sink->EndFigure(D2D1_FIGURE_END_OPEN);
      sink->BeginFigure({x, y}, D2D1_FIGURE_BEGIN_FILLED);
      figure_open = true;
      current_x = x, current_y = y;
      last_control_x = x, last_control_y = y;
      break;
    }

    case 'L': { // lineto
      auto res_x = parse_number();
      if (!res_x) return res_x.error().relay();
      auto res_y = parse_number();
      if (!res_y) return res_y.error().relay();
      float x = *res_x, y = *res_y;
      if (use_relative) x += current_x, y += current_y;
      sink->AddLine({x, y});
      current_x = x, current_y = y;
      last_control_x = x, last_control_y = y;
      break;
    }

    case 'H': { // horizontal lineto
      auto res_x = parse_number();
      if (!res_x) return res_x.error().relay();
      float x = *res_x;
      if (use_relative) x += current_x;
      sink->AddLine({x, current_y});
      current_x = x;
      last_control_x = x, last_control_y = current_y;
      break;
    }

    case 'V': { // vertical lineto
      auto res_y = parse_number();
      if (!res_y) return res_y.error().relay();
      float y = *res_y;
      if (use_relative) y += current_y;
      sink->AddLine({current_x, y});
      current_y = y;
      last_control_x = current_x, last_control_y = y;
      break;
    }

    case 'C': { // cubic bezier
      auto res_x1 = parse_number();
      if (!res_x1) return res_x1.error().relay();
      auto res_y1 = parse_number();
      if (!res_y1) return res_y1.error().relay();
      auto res_x2 = parse_number();
      if (!res_x2) return res_x2.error().relay();
      auto res_y2 = parse_number();
      if (!res_y2) return res_y2.error().relay();
      auto res_x = parse_number();
      if (!res_x) return res_x.error().relay();
      auto res_y = parse_number();
      if (!res_y) return res_y.error().relay();
      float x1 = *res_x1, y1 = *res_y1;
      float x2 = *res_x2, y2 = *res_y2;
      float x = *res_x, y = *res_y;
      if (use_relative) {
        x1 += current_x, y1 += current_y;
        x2 += current_x, y2 += current_y;
        x += current_x, y += current_y;
      }
      D2D1_BEZIER_SEGMENT segments[1];
      segments[0].point1 = D2D1::Point2F(x1, y1);
      segments[0].point2 = D2D1::Point2F(x2, y2);
      segments[0].point3 = D2D1::Point2F(x, y);
      sink->AddBezier(segments);
      last_control_x = x2, last_control_y = y2;
      current_x = x, current_y = y;
      break;
    }

    case 'S': { // smooth cubic bezier
      auto res_x2 = parse_number();
      if (!res_x2) return res_x2.error().relay();
      auto res_y2 = parse_number();
      if (!res_y2) return res_y2.error().relay();
      auto res_x = parse_number();
      if (!res_x) return res_x.error().relay();
      auto res_y = parse_number();
      if (!res_y) return res_y.error().relay();
      float x2 = *res_x2, y2 = *res_y2;
      float x = *res_x, y = *res_y;
      if (use_relative) {
        x2 += current_x, y2 += current_y;
        x += current_x, y += current_y;
      }
      // Calculate reflected control point
      float x1 = current_x, y1 = current_y;
      if (std::toupper(last_cmd) == 'C' || std::toupper(last_cmd) == 'S') {
        x1 = 2.0f * current_x - last_control_x;
        y1 = 2.0f * current_y - last_control_y;
      }
      D2D1_BEZIER_SEGMENT segments[1];
      segments[0].point1 = D2D1::Point2F(x1, y1);
      segments[0].point2 = D2D1::Point2F(x2, y2);
      segments[0].point3 = D2D1::Point2F(x, y);
      sink->AddBezier(segments);
      last_control_x = x2, last_control_y = y2;
      current_x = x, current_y = y;
      break;
    }

    case 'Q': { // quadratic bezier
      auto res_x1 = parse_number();
      if (!res_x1) return res_x1.error().relay();
      auto res_y1 = parse_number();
      if (!res_y1) return res_y1.error().relay();
      auto res_x = parse_number();
      if (!res_x) return res_x.error().relay();
      auto res_y = parse_number();
      if (!res_y) return res_y.error().relay();
      float x1 = *res_x1, y1 = *res_y1;
      float x = *res_x, y = *res_y;
      if (use_relative) {
        x1 += current_x, y1 += current_y;
        x += current_x, y += current_y;
        y += current_y;
      }
      D2D1_QUADRATIC_BEZIER_SEGMENT segment;
      segment.point1 = D2D1::Point2F(x1, y1);
      segment.point2 = D2D1::Point2F(x, y);
      sink->AddQuadraticBezier(segment);
      last_control_x = x1, last_control_y = y1;
      current_x = x, current_y = y;
      break;
    }

    case 'T': { // smooth quadratic bezier
      auto res_x = parse_number();
      if (!res_x) return res_x.error().relay();
      auto res_y = parse_number();
      if (!res_y) return res_y.error().relay();
      float x = *res_x, y = *res_y;
      if (use_relative) x += current_x, y += current_y;
      // Calculate reflected control point
      float x1 = current_x, y1 = current_y;
      if (std::toupper(last_cmd) == 'Q' || std::toupper(last_cmd) == 'T') {
        x1 = 2.0f * current_x - last_control_x;
        y1 = 2.0f * current_y - last_control_y;
      }
      D2D1_QUADRATIC_BEZIER_SEGMENT segment;
      segment.point1 = D2D1::Point2F(x1, y1);
      segment.point2 = D2D1::Point2F(x, y);
      sink->AddQuadraticBezier(segment);
      last_control_x = x1, last_control_y = y1;
      current_x = x, current_y = y;
      break;
    }

    case 'A': { // elliptical arc
      auto res_rx = parse_number();
      if (!res_rx) return res_rx.error().relay();
      auto res_ry = parse_number();
      if (!res_ry) return res_ry.error().relay();
      auto res_rot = parse_number();
      if (!res_rot) return res_rot.error().relay();
      auto res_large = parse_number();
      if (!res_large) return res_large.error().relay();
      auto res_sweep = parse_number();
      if (!res_sweep) return res_sweep.error().relay();
      auto res_x = parse_number();
      if (!res_x) return res_x.error().relay();
      auto res_y = parse_number();
      if (!res_y) return res_y.error().relay();
      float rx = std::abs(*res_rx), ry = std::abs(*res_ry);
      float rot = *res_rot;
      bool large_arc = *res_large != 0.0f;
      bool sweep = *res_sweep != 0.0f;
      float x = *res_x, y = *res_y;
      if (use_relative) x += current_x, y += current_y;
      if (rx <= 0.0f || ry <= 0.0f) sink->AddLine({x, y});
      else {
        D2D1_ARC_SEGMENT arc{};
        arc.point = D2D1::Point2F(x, y);
        arc.size = D2D1::SizeF(rx, ry);
        arc.rotationAngle = rot;
        arc.sweepDirection = sweep ? D2D1_SWEEP_DIRECTION_CLOCKWISE : D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE;
        arc.arcSize = large_arc ? D2D1_ARC_SIZE_LARGE : D2D1_ARC_SIZE_SMALL;
        sink->AddArc(arc);
      }
      current_x = x, current_y = y;
      last_control_x = x, last_control_y = y;
      break;
    }

    case 'Z': { // closepath
      if (figure_open) {
        sink->EndFigure(D2D1_FIGURE_END_CLOSED);
        figure_open = false;
      }
      break;
    }

    default: sink->Release(); return std::unexpected(error(errors::invalid_argument, "unsupported SVG path command"));
    }
  }
  // Close any remaining open figure before closing the sink
  if (figure_open) sink->EndFigure(D2D1_FIGURE_END_OPEN);
  if (const auto hr = sink->Close(); FAILED(hr)) {
    sink->Release();
    return std::unexpected(error(errors::operation_failed, "Close sink failed", int32_t(hr)));
  }
  sink.release();
  return geometry;
}
} // namespace yw
