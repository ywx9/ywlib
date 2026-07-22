#pragma once
#include "ywx/core.h"

namespace yw {

/// d2d/d3d描画を管理するクラス
/// class that manages d2d/d3d drawing
class drawing {
  inline static void* _target = nullptr;
  inline static int _type = 0; // 0: none, 2: d2d, 3: d3d, other: invalid (undefined behavior)
  source_line _source = here();
  bool _active = false;

  static std::expected<void, error> initialize(ID2D1Image* Target) {
    if (_target || _type != 0) return std::unexpected(error(errors::invalid_operation, "already in drawing"));
    d2d::context()->SetTarget(Target);
    d2d::context()->BeginDraw();
    _target = Target;
    _type = 2;
    return {};
  }

  static std::expected<void, error> initialize(ID3D11RenderTargetView* Target) {
    if (_target || _type != 0) return std::unexpected(error(errors::invalid_operation, "already in drawing"));
    d3d::context()->OMSetRenderTargets(1, &Target, nullptr);
    _target = Target;
    _type = 3;
    return {};
  }

  static std::expected<void, error> initialize(ID3D11RenderTargetView* Target, ID3D11DepthStencilView* DepthStencil) {
    if (_target || _type != 0) return std::unexpected(error(errors::invalid_operation, "already in drawing"));
    d3d::context()->OMSetRenderTargets(1, &Target, DepthStencil);
    _target = Target;
    _type = 3;
    return {};
  }

  drawing(const source_line& sl) : _source(sl), _active(true) {}

public:
  drawing() noexcept = default;
  ~drawing() { close(); }

  drawing(const drawing&) = delete;
  drawing& operator=(const drawing&) = delete;

  drawing(drawing&& Other) noexcept : _source(Other._source), _active(std::exchange(Other._active, false)) {}

  drawing& operator=(drawing&& Other) noexcept {
    if (this == &Other) return *this;
    close();
    _source = Other._source;
    _active = std::exchange(Other._active, false);
    return *this;
  }

  drawing(ID2D1Image* Target, const source_line& sl = here()) : _source(sl), _active(true) {
    if (auto res = initialize(Target); !res) res.error().add_footprint().go_off(sl);
  }

  drawing(ID3D11RenderTargetView* Target, const source_line& sl = here()) : _source(sl), _active(true) {
    if (auto res = initialize(Target); !res) res.error().add_footprint().go_off(sl);
  }

  drawing(ID3D11RenderTargetView* Target, ID3D11DepthStencilView* Depth, const source_line& sl = here())
    : _source(sl), _active(true) {
    if (auto res = initialize(Target, Depth); !res) res.error().add_footprint().go_off(sl);
  }

  explicit operator bool() const noexcept { return _target != nullptr && _type != 0; }

  static bool not_drawing() noexcept { return _target == nullptr && _type == 0; }
  static bool d2d_drawing() noexcept { return _target != nullptr && _type == 2; }
  static bool d3d_drawing() noexcept { return _target != nullptr && _type == 3; }

  template<typename... As> requires constructible<drawing, As...>
  static std::expected<drawing, error> create(As&&... Args) {
    drawing d{};
    if (auto res = d.initialize(static_cast<As&&>(Args)...)) {
      d._active = true;
      return d;
    } else return res.error().relay();
  }

  std::expected<void, error> close() {
    if (!_active) return {};
    if (d2d_drawing()) {
      hresult_test(d2d::context()->EndDraw);
      d2d::context()->SetTarget(nullptr);
    } else if (d3d_drawing()) {
      d3d::context()->OMSetRenderTargets(0, nullptr, nullptr);
    }
    _target = nullptr;
    _type = 0;
    _active = false;
    return {};
  }
};

/// MARK: draw line

inline std::expected<void, error> stroke_line(float2 p0, float2 p1, float1 width = 1.0f) {
  if (!drawing::d2d_drawing()) return std::unexpected(error(errors::invalid_operation, "not in d2d drawing"));
  const auto d2d = yw::d2d();
  const auto brush = yw::brush();
  d2d.context()->DrawLine({p0.x, p0.y}, {p1.x, p1.y}, brush.d2d_brush(), width.x, brush.d2d_stroke());
  return {};
}

/// MARK: draw rectangle

inline std::expected<void, error> stroke_rectangle(float2 pos, float2 size, float1 border_width = 1.0f) {
  if (!drawing::d2d_drawing()) return std::unexpected(error(errors::invalid_operation, "not in d2d drawing"));
  const auto d2d = yw::d2d();
  const auto brush = yw::brush();
  D2D1_RECT_F rect = D2D1::RectF(pos.x, pos.y, pos.x + size.x, pos.y + size.y);
  d2d.context()->DrawRectangle(&rect, brush.d2d_brush(), border_width.x, brush.d2d_stroke());
  return {};
}

/// \param Rect `{left, top, right, bottom}`
inline std::expected<void, error> stroke_rectangle(const float4& Rect, float1 Width = 1.0f) {
  if (!drawing::d2d_drawing()) return std::unexpected(error(errors::invalid_operation, "not in d2d drawing"));
  const auto d2d = yw::d2d();
  const auto brush = yw::brush();
  d2d.context()->DrawRectangle((const D2D1_RECT_F*)&Rect, brush.d2d_brush(), Width.x, brush.d2d_stroke());
  return {};
}

inline std::expected<void, error> fill_rectangle(float2 pos, float2 size) {
  if (!drawing::d2d_drawing()) return std::unexpected(error(errors::invalid_operation, "not in d2d drawing"));
  const auto d2d = yw::d2d();
  const auto brush = yw::brush();
  d2d.context()->FillRectangle(D2D1_RECT_F(pos.x, pos.y, pos.x + size.x, pos.y + size.y), brush.d2d_brush());
  return {};
}

/// \param Rect `{left, top, right, bottom}`
inline std::expected<void, error> fill_rectangle(const float4& Rect) {
  if (!drawing::d2d_drawing()) return std::unexpected(error(errors::invalid_operation, "not in d2d drawing"));
  const auto d2d = yw::d2d();
  const auto brush = yw::brush();
  d2d.context()->FillRectangle((const D2D1_RECT_F*)&Rect, brush.d2d_brush());
  return {};
}

/// MARK: draw round_rectangle

inline std::expected<void, error> stroke_round_rectangle(
  float2 pos, float2 size, float2 radius, float1 border_width = 1.0f) {
  if (!drawing::d2d_drawing()) return std::unexpected(error(errors::invalid_operation, "not in d2d drawing"));
  const auto d2d = yw::d2d();
  const auto brush = yw::brush();
  D2D1_ROUNDED_RECT r{D2D1::RectF(pos.x, pos.y, pos.x + size.x, pos.y + size.y), radius.x, radius.y};
  d2d.context()->DrawRoundedRectangle(&r, brush.d2d_brush(), border_width.x, brush.d2d_stroke());
  return {};
}

inline std::expected<void, error> fill_round_rectangle(float2 pos, float2 size, float2 radius) {
  if (!drawing::d2d_drawing()) return std::unexpected(error(errors::invalid_operation, "not in d2d drawing"));
  const auto d2d = yw::d2d();
  const auto brush = yw::brush();
  D2D1_ROUNDED_RECT r{D2D1::RectF(pos.x, pos.y, pos.x + size.x, pos.y + size.y), radius.x, radius.y};
  d2d.context()->FillRoundedRectangle(&r, brush.d2d_brush());
  return {};
}

/// MARK: draw ellipse

inline std::expected<void, error> stroke_ellipse(float2 center, float2 radius, float1 border_width = 1.0f) {
  if (!drawing::d2d_drawing()) return std::unexpected(error(errors::invalid_operation, "not in d2d drawing"));
  const auto d2d = yw::d2d();
  const auto brush = yw::brush();
  D2D1_ELLIPSE ellipse = D2D1::Ellipse({center.x, center.y}, radius.x, radius.y);
  d2d.context()->DrawEllipse(&ellipse, brush.d2d_brush(), border_width.x, brush.d2d_stroke());
  return {};
}

inline std::expected<void, error> fill_ellipse(float2 center, float2 radius) {
  if (!drawing::d2d_drawing()) return std::unexpected(error(errors::invalid_operation, "not in d2d drawing"));
  const auto d2d = yw::d2d();
  const auto brush = yw::brush();
  D2D1_ELLIPSE ellipse = D2D1::Ellipse(D2D1::Point2F(center.x, center.y), radius.x, radius.y);
  d2d.context()->FillEllipse(&ellipse, brush.d2d_brush());
  return {};
}

/// MARK: draw geometry

inline std::expected<void, error> stroke_geometry(ID2D1Geometry* geometry, float1 Thickness = 1.0f) {
  if (!drawing::d2d_drawing()) return std::unexpected(error(errors::invalid_operation, "not in d2d drawing"));
  const auto d2d = yw::d2d();
  const auto brush = yw::brush();
  d2d.context()->DrawGeometry(geometry, brush.d2d_brush(), Thickness.x, brush.d2d_stroke());
  return {};
}

inline std::expected<void, error> fill_geometry(ID2D1Geometry* geometry) {
  if (!drawing::d2d_drawing()) return std::unexpected(error(errors::invalid_operation, "not in d2d drawing"));
  const auto d2d = yw::d2d();
  const auto brush = yw::brush();
  d2d.context()->FillGeometry(geometry, brush.d2d_brush(), nullptr);
  return {};
}

/// MARK: draw text

inline std::expected<void, error> draw_text(float2 Pos, float2 Size, string_view<wchar_t> Text,
  IDWriteTextFormat* Format, D2D1_DRAW_TEXT_OPTIONS Options = D2D1_DRAW_TEXT_OPTIONS_NONE) {
  if (!drawing::d2d_drawing()) return std::unexpected(error(errors::invalid_operation, "not in d2d drawing"));
  const auto d2d = yw::d2d();
  const auto brush = yw::brush();
  const auto rect = D2D1_RECT_F(Pos.x, Pos.y, Pos.x + Size.x, Pos.y + Size.y);
  d2d.context()->DrawTextW(Text.data(), Text.size(), Format, rect, brush.d2d_brush(), Options);
  return {};
}

inline std::expected<void, error> draw_text(float2 Pos, IDWriteTextLayout* Layout) {
  if (!drawing::d2d_drawing()) return std::unexpected(error(errors::invalid_operation, "not in d2d drawing"));
  const auto d2d = yw::d2d();
  const auto brush = yw::brush();
  d2d.context()->DrawTextLayout(D2D1::Point2F(Pos.x, Pos.y), Layout, brush.d2d_brush());
  return {};
}
} // namespace yw
