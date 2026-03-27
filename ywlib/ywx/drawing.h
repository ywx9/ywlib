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
    } else if (d3d_drawing()) {
      // nothing to do yet
    }
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
    return drawing{src};
  }

  static std::expected<drawing, error_trace> create(ID3D11RenderTargetView* rendertarget, const source& src) {
    if (_rendertarget.index() != 0) return unexpected_error(errors::invalid_operation, "rendertarget already set");
    if (rendertarget == nullptr) return unexpected_error(errors::invalid_argument, "null rendertarget");
    if (auto res = d3d.initialize(); !res) return unexpected_error(res.error());
    // nothing to do yet
    return drawing{src};
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
}
