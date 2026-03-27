#pragma once
#include "ywx/core.h"

namespace yw {

inline class {
  struct pointers {
    ID2D1SolidColorBrush* solid_brush{};
    ID2D1StrokeStyle* stroke_style{};
    ID2D1StrokeStyle* dashed_stroke_style{};
    bool dashed = false;
    bool initialized = false;

    ~pointers() {
      if (dashed_stroke_style) dashed_stroke_style->Release();
      if (stroke_style) stroke_style->Release();
      if (solid_brush) solid_brush->Release();
      initialized = false;
    }
  } p{};

public:

  std::expected<void, error_trace> initialize() {
    if (p.initialized) return {};
    if (auto res = d2d.initialize(); !res) return unexpected_error(res.error());
    if (auto hr = d2d.context()->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &p.solid_brush); FAILED(hr))
      return unexpected_error(errors::operation_failed, "CreateSolidColorBrush failed", int32_t(hr));
    D2D1_STROKE_STYLE_PROPERTIES stroke_style_props{};
    stroke_style_props.startCap = D2D1_CAP_STYLE_ROUND;
    stroke_style_props.endCap = D2D1_CAP_STYLE_ROUND;
    stroke_style_props.dashCap = D2D1_CAP_STYLE_ROUND;
    stroke_style_props.lineJoin = D2D1_LINE_JOIN_ROUND;
    stroke_style_props.miterLimit = 10.0f;
    if (auto hr = d2d.factory()->CreateStrokeStyle(&stroke_style_props, nullptr, 0, &p.stroke_style); FAILED(hr))
      return unexpected_error(errors::operation_failed, "CreateStrokeStyle failed", int32_t(hr));
    stroke_style_props.dashStyle = D2D1_DASH_STYLE_DASH;
    if (auto hr = d2d.factory()->CreateStrokeStyle(&stroke_style_props, nullptr, 0, &p.dashed_stroke_style); FAILED(hr))
      return unexpected_error(errors::operation_failed, "CreateStrokeStyle (dashed) failed", int32_t(hr));
    p.initialized = true;
    return {};
  }

  yw::color color() {
    if (!initialize()) return colors::black;
    return std::bit_cast<yw::color>(p.solid_brush->GetColor());
  }

  void color(const yw::color& c) {
    if (!initialize()) return;
    p.solid_brush->SetColor(reinterpret_cast<const D2D1_COLOR_F*>(&c));
  }

  bool dashed() {
    if (!initialize()) return false;
    return p.dashed;
  }

  void dashed(bool d) {
    if (!initialize()) return;
    p.dashed = d;
  }

  ID2D1SolidColorBrush* d2d_brush() {
    if (!initialize()) return nullptr;
    return p.solid_brush;
  }

  ID2D1StrokeStyle* d2d_stroke() {
    if (!initialize()) return nullptr;
    return p.dashed ? p.dashed_stroke_style : p.stroke_style;
  }
} brush;
} // namespace yw
