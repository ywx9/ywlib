#pragma once
#include "ywx/core.h"

namespace yw {

class drawing {
  inline static void* _target = nullptr;
  bool _active = false;

public:
  drawing() noexcept = default;
  ~drawing() { close(); }

  drawing(const drawing&) = delete;
  drawing& operator=(const drawing&) = delete;

  drawing(drawing&& Other) noexcept : _active(std::exchange(Other._active, false)) {}

  drawing& operator=(drawing&& Other) noexcept {
    if (this == &Other) return *this;
    if (_active) if (auto res = close(); !res) res.error().go_off();
    _active = std::exchange(Other._active, false);
    return *this;
  }

  static std::expected<drawing, error> create(ID2D1Image* Target) {
    if (_target) return std::unexpected(error(errors::invalid_operation, "already in drawing"));
    if (!Target) return std::unexpected(error(errors::invalid_argument, "invalid target"));
    d2d::context()->SetTarget(Target);
    d2d::context()->BeginDraw();
    _target = Target;
    drawing d;
    d._active = true;
    return d;
  }

  drawing(ID2D1Image* Target, const source_line& sl = here()) {
    if (auto res = create(Target)) *this = std::move(*res);
    else res.error().add_footprint().go_off(sl);
  }

  explicit operator bool() const noexcept { return _active && _target != nullptr; }
  static bool d2d_drawing() noexcept { return _target != nullptr; }

  std::expected<void, error> close() {
    if (!_active) return {};
    hresult_test(d2d::context()->EndDraw);
    d2d::context()->SetTarget(nullptr);
    _target = nullptr, _active = false;
    return {};
  }
};

/// MARK: draw line

inline std::expected<void, error> stroke_line(float2 p0, float2 p1, float1 width = 1.0f) {
  if (!drawing::d2d_drawing()) return std::unexpected(error(errors::invalid_operation, "not in d2d drawing"));
  d2d::context()->DrawLine({p0.x, p0.y}, {p1.x, p1.y}, brush::d2d_brush(), width.x, brush::d2d_stroke());
  return {};
}

inline std::expected<void, error> stroke_line(float2 p0, float2 p1, const color& Color, float1 width = 1.0f) {
  brush::color(Color);
  return stroke_line(p0, p1, width);
}

/// MARK: draw rectangle

inline std::expected<void, error> stroke_rectangle(float2 pos, float2 size, float1 border_width = 1.0f) {
  if (!drawing::d2d_drawing()) return std::unexpected(error(errors::invalid_operation, "not in d2d drawing"));
  D2D1_RECT_F rect = D2D1::RectF(pos.x, pos.y, pos.x + size.x, pos.y + size.y);
  d2d::context()->DrawRectangle(&rect, brush::d2d_brush(), border_width.x, brush::d2d_stroke());
  return {};
}

inline std::expected<void, error> stroke_rectangle(float2 pos, float2 size, const color& Color, float1 border_width = 1.0f) {
  brush::color(Color);
  return stroke_rectangle(pos, size, border_width);
}

/// \param Rect `{left, top, right, bottom}`
inline std::expected<void, error> stroke_rectangle(const float4& Rect, float1 Width = 1.0f) {
  if (!drawing::d2d_drawing()) return std::unexpected(error(errors::invalid_operation, "not in d2d drawing"));
  d2d::context()->DrawRectangle((const D2D1_RECT_F*)&Rect, brush::d2d_brush(), Width.x, brush::d2d_stroke());
  return {};
}

inline std::expected<void, error> stroke_rectangle(const float4& Rect, const color& Color, float1 Width = 1.0f) {
  brush::color(Color);
  return stroke_rectangle(Rect, Width);
}

inline std::expected<void, error> fill_rectangle(float2 pos, float2 size) {
  if (!drawing::d2d_drawing()) return std::unexpected(error(errors::invalid_operation, "not in d2d drawing"));
  d2d::context()->FillRectangle(D2D1_RECT_F(pos.x, pos.y, pos.x + size.x, pos.y + size.y), brush::d2d_brush());
  return {};
}

inline std::expected<void, error> fill_rectangle(float2 pos, float2 size, const color& Color) {
  brush::color(Color);
  return fill_rectangle(pos, size);
}

/// \param Rect `{left, top, right, bottom}`
inline std::expected<void, error> fill_rectangle(const float4& Rect) {
  if (!drawing::d2d_drawing()) return std::unexpected(error(errors::invalid_operation, "not in d2d drawing"));
  d2d::context()->FillRectangle((const D2D1_RECT_F*)&Rect, brush::d2d_brush());
  return {};
}

inline std::expected<void, error> fill_rectangle(const float4& Rect, const color& Color) {
  brush::color(Color);
  return fill_rectangle(Rect);
}

/// MARK: draw round_rectangle

inline std::expected<void, error> stroke_round_rectangle(
  float2 pos, float2 size, float2 radius, float1 border_width = 1.0f) {
  if (!drawing::d2d_drawing()) return std::unexpected(error(errors::invalid_operation, "not in d2d drawing"));
  D2D1_ROUNDED_RECT r{D2D1::RectF(pos.x, pos.y, pos.x + size.x, pos.y + size.y), radius.x, radius.y};
  d2d::context()->DrawRoundedRectangle(&r, brush::d2d_brush(), border_width.x, brush::d2d_stroke());
  return {};
}

inline std::expected<void, error> stroke_round_rectangle(
  float2 pos, float2 size, float2 radius, const color& Color, float1 border_width = 1.0f) {
  brush::color(Color);
  return stroke_round_rectangle(pos, size, radius, border_width);
}

inline std::expected<void, error> fill_round_rectangle(float2 pos, float2 size, float2 radius) {
  if (!drawing::d2d_drawing()) return std::unexpected(error(errors::invalid_operation, "not in d2d drawing"));
  D2D1_ROUNDED_RECT r{D2D1::RectF(pos.x, pos.y, pos.x + size.x, pos.y + size.y), radius.x, radius.y};
  d2d::context()->FillRoundedRectangle(&r, brush::d2d_brush());
  return {};
}

inline std::expected<void, error> fill_round_rectangle(float2 pos, float2 size, float2 radius, const color& Color) {
  brush::color(Color);
  return fill_round_rectangle(pos, size, radius);
}

/// MARK: draw ellipse

inline std::expected<void, error> stroke_ellipse(float2 center, float2 radius, float1 border_width = 1.0f) {
  if (!drawing::d2d_drawing()) return std::unexpected(error(errors::invalid_operation, "not in d2d drawing"));
  D2D1_ELLIPSE ellipse = D2D1::Ellipse({center.x, center.y}, radius.x, radius.y);
  d2d::context()->DrawEllipse(&ellipse, brush::d2d_brush(), border_width.x, brush::d2d_stroke());
  return {};
}

inline std::expected<void, error> stroke_ellipse(float2 center, float2 radius, const color& Color, float1 border_width = 1.0f) {
  brush::color(Color);
  return stroke_ellipse(center, radius, border_width);
}

inline std::expected<void, error> fill_ellipse(float2 center, float2 radius) {
  if (!drawing::d2d_drawing()) return std::unexpected(error(errors::invalid_operation, "not in d2d drawing"));
  D2D1_ELLIPSE ellipse = D2D1::Ellipse(D2D1::Point2F(center.x, center.y), radius.x, radius.y);
  d2d::context()->FillEllipse(&ellipse, brush::d2d_brush());
  return {};
}

inline std::expected<void, error> fill_ellipse(float2 center, float2 radius, const color& Color) {
  brush::color(Color);
  return fill_ellipse(center, radius);
}

/// MARK: draw geometry

inline std::expected<void, error> stroke_geometry(ID2D1Geometry* geometry, float1 Thickness = 1.0f) {
  if (!drawing::d2d_drawing()) return std::unexpected(error(errors::invalid_operation, "not in d2d drawing"));
  d2d::context()->DrawGeometry(geometry, brush::d2d_brush(), Thickness.x, brush::d2d_stroke());
  return {};
}

inline std::expected<void, error> stroke_geometry(ID2D1Geometry* geometry, const color& Color, float1 Thickness = 1.0f) {
  brush::color(Color);
  return stroke_geometry(geometry, Thickness);
}

inline std::expected<void, error> fill_geometry(ID2D1Geometry* geometry) {
  if (!drawing::d2d_drawing()) return std::unexpected(error(errors::invalid_operation, "not in d2d drawing"));
  d2d::context()->FillGeometry(geometry, brush::d2d_brush(), nullptr);
  return {};
}

inline std::expected<void, error> fill_geometry(ID2D1Geometry* geometry, const color& Color) {
  brush::color(Color);
  return fill_geometry(geometry);
}

/// MARK: draw text

inline std::expected<void, error> draw_text(
  float2 Pos, float2 Size, string_view<wchar_t> Text, IDWriteTextFormat* Format,
  D2D1_DRAW_TEXT_OPTIONS Options = D2D1_DRAW_TEXT_OPTIONS_NONE) {
  if (!drawing::d2d_drawing()) return std::unexpected(error(errors::invalid_operation, "not in d2d drawing"));
  const auto rect = D2D1_RECT_F(Pos.x, Pos.y, Pos.x + Size.x, Pos.y + Size.y);
  d2d::context()->DrawTextW(Text.data(), Text.size(), Format, rect, brush::d2d_brush(), Options);
  return {};
}

inline std::expected<void, error> draw_text(
  float2 Pos, float2 Size, string_view<wchar_t> Text, IDWriteTextFormat* Format, const color& Color,
  D2D1_DRAW_TEXT_OPTIONS Options = D2D1_DRAW_TEXT_OPTIONS_NONE) {
  brush::color(Color);
  return draw_text(Pos, Size, Text, Format, Options);
}

inline std::expected<void, error> draw_text(float2 Pos, IDWriteTextLayout* Layout) {
  if (!drawing::d2d_drawing()) return std::unexpected(error(errors::invalid_operation, "not in d2d drawing"));
  d2d::context()->DrawTextLayout(D2D1::Point2F(Pos.x, Pos.y), Layout, brush::d2d_brush());
  return {};
}

inline std::expected<void, error> draw_text(float2 Pos, IDWriteTextLayout* Layout, const color& Color) {
  brush::color(Color);
  return draw_text(Pos, Layout);
}
} // namespace yw
