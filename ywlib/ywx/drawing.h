#pragma once
#include "ywx/core.h"

namespace yw {

class drawing {
  inline static void* _target = nullptr;
  enum class target_type { none, d2d, d3d };
  inline static target_type _target_type = target_type::none;
  /// \note Errors often occur in the destructor -> close-> EndDraw. In this case,
  ///       the source location cannot be determined, so the position at the time of creation is retained instead.
  source _source;
  bool _active = false;
  drawing(const drawing&) = delete;
  drawing& operator=(const drawing&) = delete;
  explicit drawing(const source& src) : _source(src), _active(true) {}

public:
  ~drawing() noexcept { close(); }
  drawing() noexcept = default;
  drawing(drawing&& Other) : _source(std::move(Other._source)) {}
  drawing& operator=(drawing&& Other) {
    if (this == &Other) return *this;
    _source = Other._source;
    _active = std::exchange(Other._active, false);
    return *this;
  }

  static std::expected<drawing, error_trace> create(ID2D1Image* Target, const source& Src = {}) {
    if (!not_drawing()) {
      if (d2d_drawing()) return unexpected_error(errors::invalid_operation, "already begun d2d drawing");
      else return unexpected_error(errors::invalid_operation, "already begun d3d drawing");
    } else if (Target == nullptr) return unexpected_error(errors::invalid_argument, "null rendertarget");
    const auto& d2d = yw::d2d();
    _target = Target;
    _target_type = target_type::d2d;
    d2d.context()->SetTarget(Target);
    d2d.context()->BeginDraw();
    return drawing(Src);
  }

  static std::expected<drawing, error_trace> create(ID3D11RenderTargetView* Target, const source& Src = {}) {
    if (!not_drawing()) {
      if (d2d_drawing()) return unexpected_error(errors::invalid_operation, "already begun d2d drawing");
      else return unexpected_error(errors::invalid_operation, "already begun d3d drawing");
    } else if (Target == nullptr) return unexpected_error(errors::invalid_argument, "null rendertarget");
    const auto& d3d = yw::d3d();
    _target = Target;
    _target_type = target_type::d3d;
    d3d.context()->OMSetRenderTargets(1, &Target, nullptr);
    return drawing(Src);
  }

  static std::expected<drawing, error_trace> create(
    ID3D11RenderTargetView* Target, ID3D11DepthStencilView* DepthStencil, const source& Src = {}) {
    if (!not_drawing()) {
      if (d2d_drawing()) return unexpected_error(errors::invalid_operation, "already begun d2d drawing");
      else return unexpected_error(errors::invalid_operation, "already begun d3d drawing");
    } else if (Target == nullptr) return unexpected_error(errors::invalid_argument, "null rendertarget");
    else if (DepthStencil == nullptr) return unexpected_error(errors::invalid_argument, "null depth stencil view");
    const auto& d3d = yw::d3d();
    _target = Target;
    _target_type = target_type::d3d;
    d3d.context()->OMSetRenderTargets(1, &Target, DepthStencil);
    return drawing(Src);
  }

  static bool d2d_drawing() noexcept { return _target_type == target_type::d2d; }
  static bool d3d_drawing() noexcept { return _target_type == target_type::d3d; }
  static bool not_drawing() noexcept { return _target_type == target_type::none; }

  std::expected<void, error_trace> close() noexcept {
    try {
      if (!_active) return {};
      _active = false;
      if (d2d_drawing()) {
        const auto& d2d = yw::d2d();
        hresult_test(d2d.context()->EndDraw);
        d2d.context()->SetTarget(nullptr);
      } else if (d3d_drawing()) d3d().context()->OMSetRenderTargets(0, nullptr, nullptr);
      _target = nullptr;
      _target_type = target_type::none;
    } catch (...) { return unexpected_error(errors::operation_failed, "unknown error occurred while closing drawing"); }
    return {};
  }
};

//////////////////////////////////////// MARK: draw line

inline std::expected<void, error_trace> draw_line(float2 p0, float2 p1, float1 width = 1.0f) {
  if (!drawing::d2d_drawing()) return unexpected_error(errors::invalid_operation, "drawing not begun");
  const auto& brush = yw::brush();
  d2d().context()->DrawLine({p0.x, p0.y}, {p1.x, p1.y}, brush.d2d_brush(), width.x, brush.d2d_stroke());
  return {};
}

/////////////////////////////////////// MARK: draw/fill_rectangle

inline std::expected<void, error_trace> draw_rectangle(float2 pos, float2 size, float1 border_width = 1.0f) {
  if (!drawing::d2d_drawing()) return unexpected_error(errors::invalid_operation, "drawing not begun");
  const auto& brush = yw::brush();
  D2D1_RECT_F rect = D2D1::RectF(pos.x, pos.y, pos.x + size.x, pos.y + size.y);
  d2d().context()->DrawRectangle(&rect, brush.d2d_brush(), border_width.x, brush.d2d_stroke());
  return {};
}

inline std::expected<void, error_trace> fill_rectangle(float2 pos, float2 size) {
  if (!drawing::d2d_drawing()) return unexpected_error(errors::invalid_operation, "drawing not begun");
  d2d().context()->FillRectangle(D2D1::RectF(pos.x, pos.y, pos.x + size.x, pos.y + size.y), brush().d2d_brush());
  return {};
}

//////////////////////////////////// MARK: draw/fill_round_rectangle

inline std::expected<void, error_trace> draw_round_rectangle(
  float2 pos, float2 size, float2 radius, float1 border_width = 1.0f) {
  if (!drawing::d2d_drawing()) return unexpected_error(errors::invalid_operation, "drawing not begun");
  const auto& brush = yw::brush();
  D2D1_ROUNDED_RECT r{D2D1::RectF(pos.x, pos.y, pos.x + size.x, pos.y + size.y), radius.x, radius.y};
  d2d().context()->DrawRoundedRectangle(&r, brush.d2d_brush(), border_width.x, brush.d2d_stroke());
  return {};
}

inline std::expected<void, error_trace> fill_round_rectangle(float2 pos, float2 size, float2 radius) {
  if (!drawing::d2d_drawing()) return unexpected_error(errors::invalid_operation, "drawing not begun");
  D2D1_ROUNDED_RECT r{D2D1::RectF(pos.x, pos.y, pos.x + size.x, pos.y + size.y), radius.x, radius.y};
  d2d().context()->FillRoundedRectangle(&r, brush().d2d_brush());
  return {};
}

//////////////////////////////////////// MARK: draw/fill_ellipse

inline std::expected<void, error_trace> draw_ellipse(float2 center, float2 radius, float1 border_width = 1.0f) {
  if (!drawing::d2d_drawing()) return unexpected_error(errors::invalid_operation, "drawing not begun");
  const auto& brush = yw::brush();
  D2D1_ELLIPSE ellipse = D2D1::Ellipse({center.x, center.y}, radius.x, radius.y);
  d2d().context()->DrawEllipse(&ellipse, brush.d2d_brush(), border_width.x, brush.d2d_stroke());
  return {};
}

inline std::expected<void, error_trace> fill_ellipse(float2 center, float2 radius) {
  if (!drawing::d2d_drawing()) return unexpected_error(errors::invalid_operation, "drawing not begun");
  D2D1_ELLIPSE ellipse = D2D1::Ellipse(D2D1::Point2F(center.x, center.y), radius.x, radius.y);
  d2d().context()->FillEllipse(&ellipse, brush().d2d_brush());
  return {};
}

//////////////////////////////////////// MARK: draw/fill_geometry

template<typename Geometry> concept geometry_like = castable_to<Geometry, ID2D1Geometry*>;

inline std::expected<void, error_trace> draw_geometry(geometry_like auto geometry, float1 border_width = 1.0f) {
  if (!drawing::d2d_drawing()) return unexpected_error(errors::invalid_operation, "drawing not begun");
  const auto& brush = yw::brush();
  d2d().context()->DrawGeometry(geometry, brush.d2d_brush(), border_width.x, brush.d2d_stroke());
  return {};
}

inline std::expected<void, error_trace> fill_geometry(geometry_like auto geometry) {
  if (!drawing::d2d_drawing()) return unexpected_error(errors::invalid_operation, "drawing not begun");
  d2d().context()->FillGeometry(geometry, brush().d2d_brush(), nullptr);
  return {};
}

//////////////////////////////////////// MARK: draw_text

template<typename TextFormat> concept text_format_like = castable_to<TextFormat, IDWriteTextFormat*>;
template<typename TextLayout> concept text_layout_like = castable_to<TextLayout, IDWriteTextLayout*>;

inline std::expected<void, error_trace> draw_text(float2 Pos, text_layout_like auto Layout) {
  if (!drawing::d2d_drawing()) return unexpected_error(errors::invalid_operation, "drawing not begun");
  d2d().context()->DrawTextLayout(D2D1::Point2F(Pos.x, Pos.y), Layout, brush().d2d_brush());
  return {};
}

//////////////////////////////////////// MARK: render

inline std::expected<void, error_trace> set_primitive_topology_pointlist() {
  d3d().context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
  return {};
}

inline std::expected<void, error_trace> set_primitive_topology_linelist() {
  d3d().context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
  return {};
}

inline std::expected<void, error_trace> set_primitive_topology_linestrip() {
  d3d().context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
  return {};
}

inline std::expected<void, error_trace> set_primitive_topology_trianglelist() {
  d3d().context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  return {};
}

inline std::expected<void, error_trace> set_primitive_topology_trianglestrip() {
  d3d().context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
  return {};
}

inline std::expected<void, error_trace> render(uint1 NumVertices) {
  d3d().context()->Draw(NumVertices.x, 0);
  return {};
}
} // namespace yw
