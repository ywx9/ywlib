#pragma once
#include "ywx/brush.h"

namespace yw {

class drawing {
  inline static std::variant<std::monostate, ID2D1Image*, ID3D11RenderTargetView*> _rendertarget{};
  source _source;
  bool _active = false;
  drawing(const source& src) : _source(src), _active(true) {}
  drawing(const drawing&) = delete;
  drawing& operator=(const drawing&) = delete;

public:
  ~drawing() {
    if (!_active) return;
    _active = false;
    if (d2d_drawing()) {
      if (auto hr = d2d.context()->EndDraw(); FAILED(hr))
        print_fallback("drawing failed (code={}) that starts at {}", hr, _source);
      d2d.context()->SetTarget(nullptr);
    } else if (d3d_drawing()) d3d.context()->OMSetRenderTargets(0, nullptr, nullptr);
    _rendertarget = std::monostate{};
  }

  drawing() noexcept = default;
  drawing(drawing&& other) : _source(other._source), _active(std::exchange(other._active, false)) {}

  drawing& operator=(drawing&& other) {
    if (this == &other) return *this;
    _source = other._source;
    _active = std::exchange(other._active, false);
    return *this;
  }

  static std::expected<drawing, error_trace> create(ID2D1Image* rendertarget, const source& src) {
    if (auto res = d2d.initialize(); !res) return unexpected_error(res.error());
    if (_rendertarget.index() != 0) {
      if (d2d_drawing()) return unexpected_error(errors::invalid_operation, "another d2d rendertarget already set");
      else return unexpected_error(errors::invalid_operation, "d3d rendertarget already set");
    } else if (rendertarget == nullptr) return unexpected_error(errors::invalid_argument, "null rendertarget");
    _rendertarget = rendertarget;
    d2d.context()->SetTarget(rendertarget);
    d2d.context()->BeginDraw();
    return drawing(src);
  }

  static std::expected<drawing, error_trace> create(ID3D11RenderTargetView* rtv, const source& src) {
    if (_rendertarget.index() != 0) return unexpected_error(errors::invalid_operation, "rendertarget already set");
    if (rtv == nullptr) return unexpected_error(errors::invalid_argument, "null rendertarget");
    if (auto res = d3d.initialize(); !res) return unexpected_error(res.error());
    _rendertarget = rtv;
    d3d.context()->OMSetRenderTargets(1, &rtv, nullptr);
    return drawing(src);
  }

  static std::expected<drawing, error_trace> create(ID3D11RenderTargetView* rtv, ID3D11DepthStencilView* dsv, const source& src) {
    if (_rendertarget.index() != 0) return unexpected_error(errors::invalid_operation, "rendertarget already set");
    if (rtv == nullptr) return unexpected_error(errors::invalid_argument, "null rendertarget");
    if (auto res = d3d.initialize(); !res) return unexpected_error(res.error());
    _rendertarget = rtv;
    d3d.context()->OMSetRenderTargets(1, &rtv, dsv);
    return drawing(src);
  }

  static bool d2d_drawing() { return _rendertarget.index() == 1; }
  static bool d3d_drawing() { return _rendertarget.index() == 2; }
  static bool not_drawing() { return _rendertarget.index() == 0; }
};

//////////////////////////////////////// MARK: draw line

inline std::expected<void, error_trace> draw_line(float2 p0, float2 p1, float1 width = 1.0f) {
  if (!drawing::d2d_drawing()) return unexpected_error(errors::invalid_operation, "drawing not begun");
  d2d.context()->DrawLine({p0.x, p0.y}, {p1.x, p1.y}, brush.d2d_brush(), width.x, brush.d2d_stroke());
  return {};
}

/////////////////////////////////////// MARK: draw/fill_rectangle

inline std::expected<void, error_trace> draw_rectangle(float2 pos, float2 size, float1 border_width = 1.0f) {
  if (!drawing::d2d_drawing()) return unexpected_error(errors::invalid_operation, "drawing not begun");
  D2D1_RECT_F rect = D2D1::RectF(pos.x, pos.y, pos.x + size.x, pos.y + size.y);
  d2d.context()->DrawRectangle(&rect, brush.d2d_brush(), border_width.x, brush.d2d_stroke());
  return {};
}

inline std::expected<void, error_trace> fill_rectangle(float2 pos, float2 size) {
  if (!drawing::d2d_drawing()) return unexpected_error(errors::invalid_operation, "drawing not begun");
  d2d.context()->FillRectangle(D2D1::RectF(pos.x, pos.y, pos.x + size.x, pos.y + size.y), brush.d2d_brush());
  return {};
}

//////////////////////////////////// MARK: draw/fill_round_rectangle

inline std::expected<void, error_trace> draw_round_rectangle(
  float2 pos, float2 size, float2 radius, float1 border_width = 1.0f) {
  if (!drawing::d2d_drawing()) return unexpected_error(errors::invalid_operation, "drawing not begun");
  D2D1_ROUNDED_RECT r{D2D1::RectF(pos.x, pos.y, pos.x + size.x, pos.y + size.y), radius.x, radius.y};
  d2d.context()->DrawRoundedRectangle(&r, brush.d2d_brush(), border_width.x, brush.d2d_stroke());
  return {};
}

inline std::expected<void, error_trace> fill_round_rectangle(float2 pos, float2 size, float2 radius) {
  if (!drawing::d2d_drawing()) return unexpected_error(errors::invalid_operation, "drawing not begun");
  D2D1_ROUNDED_RECT r{D2D1::RectF(pos.x, pos.y, pos.x + size.x, pos.y + size.y), radius.x, radius.y};
  d2d.context()->FillRoundedRectangle(&r, brush.d2d_brush());
  return {};
}

//////////////////////////////////////// MARK: draw/fill_ellipse

inline std::expected<void, error_trace> draw_ellipse(float2 center, float2 radius, float1 border_width = 1.0f) {
  if (!drawing::d2d_drawing()) return unexpected_error(errors::invalid_operation, "drawing not begun");
  D2D1_ELLIPSE ellipse = D2D1::Ellipse({center.x, center.y}, radius.x, radius.y);
  d2d.context()->DrawEllipse(&ellipse, brush.d2d_brush(), border_width.x, brush.d2d_stroke());
  return {};
}

inline std::expected<void, error_trace> fill_ellipse(float2 center, float2 radius) {
  if (!drawing::d2d_drawing()) return unexpected_error(errors::invalid_operation, "drawing not begun");
  D2D1_ELLIPSE ellipse = D2D1::Ellipse(D2D1::Point2F(center.x, center.y), radius.x, radius.y);
  d2d.context()->FillEllipse(&ellipse, brush.d2d_brush());
  return {};
}

//////////////////////////////////////// MARK: draw/fill_geometry

template<typename Geometry> concept geometry_like = castable_to<Geometry, ID2D1Geometry*>;

inline std::expected<void, error_trace> draw_geometry(geometry_like auto geometry, float1 border_width = 1.0f) {
  if (!drawing::d2d_drawing()) return unexpected_error(errors::invalid_operation, "drawing not begun");
  d2d.context()->DrawGeometry(geometry, brush.d2d_brush(), border_width.x, brush.d2d_stroke());
  return {};
}

inline std::expected<void, error_trace> fill_geometry(geometry_like auto geometry) {
  if (!drawing::d2d_drawing()) return unexpected_error(errors::invalid_operation, "drawing not begun");
  d2d.context()->FillGeometry(geometry, brush.d2d_brush(), nullptr);
  return {};
}

//////////////////////////////////////// MARK: draw_text

template<typename TextFormat> concept text_format_like = castable_to<TextFormat, IDWriteTextFormat*>;
template<typename TextLayout> concept text_layout_like = castable_to<TextLayout, IDWriteTextLayout*>;

inline std::expected<void, error_trace> draw_text(float2 Pos, text_layout_like auto Layout) {
  if (auto res = dwrite.initialize(); !res) return unexpected_error(res.error());
  if (!drawing::d2d_drawing()) return unexpected_error(errors::invalid_operation, "drawing not begun");
  d2d.context()->DrawTextLayout(D2D1::Point2F(Pos.x, Pos.y), Layout, brush.d2d_brush());
  return {};
}

//////////////////////////////////////// MARK: render

inline std::expected<void, error_trace> set_primitive_topology_pointlist() {
  if (auto res = d3d.initialize(); !res) return unexpected_error(res.error());
  d3d.context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
  return {};
}

inline std::expected<void, error_trace> set_primitive_topology_linelist() {
  if (auto res = d3d.initialize(); !res) return unexpected_error(res.error());
  d3d.context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
  return {};
}

inline std::expected<void, error_trace> set_primitive_topology_linestrip() {
  if (auto res = d3d.initialize(); !res) return unexpected_error(res.error());
  d3d.context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
  return {};
}

inline std::expected<void, error_trace> set_primitive_topology_trianglelist() {
  if (auto res = d3d.initialize(); !res) return unexpected_error(res.error());
  d3d.context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  return {};
}

inline std::expected<void, error_trace> set_primitive_topology_trianglestrip() {
  if (auto res = d3d.initialize(); !res) return unexpected_error(res.error());
  d3d.context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
  return {};
}

inline std::expected<void, error_trace> render(uint1 NumVertices) {
  if (!drawing::d3d_drawing()) return unexpected_error(errors::invalid_operation, "drawing not begun");
  d3d.context()->Draw(NumVertices.x, 0);
  return {};
}
}
