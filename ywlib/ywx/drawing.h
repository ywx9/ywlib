#pragma once
#include "ywx/core.h"

namespace yw {

class drawing : public general_handle {
  inline static general_slot* _sp = nullptr;

public:
  struct slot : general_slot {
    static slot* get() noexcept { return static_cast<slot*>(_sp); }
    std::variant<std::monostate, ID2D1Image*, ID3D11RenderTargetView*> target;

    std::expected<void, error> release() {
      if (target.index() == 1) {
        const auto d2d = yw::d2d();
        hresult_test(d2d.context()->EndDraw);
        d2d.context()->SetTarget(nullptr);
      } else if (target.index() == 2) d3d().context()->OMSetRenderTargets(0, nullptr, nullptr);
      target = std::monostate();
      return {};
    }
  };

  ~drawing() { close(); }
  drawing(const source_line& sl = source_line::here()) {
    if (_sp) return;
    const auto sp = create_slot<drawing>(sl);
    if (!sp) error(errors::slot_creation_failed).print_as_fatal(sl);
    _sp = sp;
  }
  drawing(drawing&& Other) noexcept = default;
  drawing& operator=(drawing&& Other) noexcept = default;

  drawing(ID2D1Image* Target, const source_line& sl) : drawing(sl) {
    const auto sp = slot::get();
    if (sp->target.index() != 0) error(errors::invalid_operation, "already in drawing").print_as_fatal(sl);
    const auto d2d = yw::d2d();
    d2d.context()->SetTarget(Target);
    d2d.context()->BeginDraw();
    sp->target = Target;
    _id = sp->id;
  }

  drawing(ID3D11RenderTargetView* Target, const source_line& sl) : drawing(sl) {
    const auto sp = slot::get();
    if (sp->target.index() != 0) error(errors::invalid_operation, "already in drawing").print_as_fatal(sl);
    const auto d3d = yw::d3d();
    d3d.context()->OMSetRenderTargets(1, &Target, nullptr);
    sp->target = Target;
    _id = sp->id;
  }

  drawing(ID3D11RenderTargetView* Target, ID3D11DepthStencilView* DepthStencil, const source_line& sl) : drawing(sl) {
    const auto sp = slot::get();
    if (sp->target.index() != 0) error(errors::invalid_operation, "already in drawing").print_as_fatal(sl);
    const auto d3d = yw::d3d();
    d3d.context()->OMSetRenderTargets(1, &Target, DepthStencil);
    sp->target = Target;
    _id = sp->id;
  }

  static bool not_drawing() noexcept { return !_sp || slot::get()->target.index() == 0; }
  static bool d2d_drawing() noexcept { return _sp && slot::get()->target.index() == 1; }
  static bool d3d_drawing() noexcept { return _sp && slot::get()->target.index() == 2; }

  std::expected<void, error> close() {
    if (d2d_drawing()) {
      const auto d2d = yw::d2d();
      hresult_test(d2d.context()->EndDraw);
      d2d.context()->SetTarget(nullptr);
    } else if (d3d_drawing()) {
      const auto d3d = yw::d3d();
      d3d.context()->OMSetRenderTargets(0, nullptr, nullptr);
    } else if (!_sp) return{};
    slot::get()->target = std::monostate();
    return {};
  }
};

// class drawing {
//   inline static void* _target = nullptr;
//   enum class target_type { none, d2d, d3d };
//   inline static target_type _target_type = target_type::none;
//   /// \note Errors often occur in the destructor -> close-> EndDraw. In this case,
//   ///       the source location cannot be determined, so the position at the time of creation is retained instead.
//   source _source;
//   bool _active = false;
//   drawing(const drawing&) = delete;
//   drawing& operator=(const drawing&) = delete;
//   explicit drawing(const source& src) : _source(src), _active(true) {}

// public:
//   ~drawing() noexcept { close(); }
//   drawing() noexcept = default;
//   drawing(drawing&& Other) : _source(std::move(Other._source)) {}
//   drawing& operator=(drawing&& Other) {
//     if (this == &Other) return *this;
//     _source = Other._source;
//     _active = std::exchange(Other._active, false);
//     return *this;
//   }

//   static std::expected<drawing, error> create(ID2D1Image* Target, const source& Src = {}) {
//     if (!not_drawing()) {
//       if (d2d_drawing()) return unexpected_error(errors::invalid_operation, "already begun d2d drawing");
//       else return unexpected_error(errors::invalid_operation, "already begun d3d drawing");
//     } else if (Target == nullptr) return unexpected_error(errors::invalid_argument, "null rendertarget");
//     const auto& d2d = yw::d2d();
//     _target = Target;
//     _target_type = target_type::d2d;
//     d2d.context()->SetTarget(Target);
//     d2d.context()->BeginDraw();
//     return drawing(Src);
//   }

//   static std::expected<drawing, error> create(ID3D11RenderTargetView* Target, const source& Src = {}) {
//     if (!not_drawing()) {
//       if (d2d_drawing()) return unexpected_error(errors::invalid_operation, "already begun d2d drawing");
//       else return unexpected_error(errors::invalid_operation, "already begun d3d drawing");
//     } else if (Target == nullptr) return unexpected_error(errors::invalid_argument, "null rendertarget");
//     const auto& d3d = yw::d3d();
//     _target = Target;
//     _target_type = target_type::d3d;
//     d3d.context()->OMSetRenderTargets(1, &Target, nullptr);
//     return drawing(Src);
//   }

//   static std::expected<drawing, error> create(
//     ID3D11RenderTargetView* Target, ID3D11DepthStencilView* DepthStencil, const source& Src = {}) {
//     if (!not_drawing()) {
//       if (d2d_drawing()) return unexpected_error(errors::invalid_operation, "already begun d2d drawing");
//       else return unexpected_error(errors::invalid_operation, "already begun d3d drawing");
//     } else if (Target == nullptr) return unexpected_error(errors::invalid_argument, "null rendertarget");
//     else if (DepthStencil == nullptr) return unexpected_error(errors::invalid_argument, "null depth stencil view");
//     const auto& d3d = yw::d3d();
//     _target = Target;
//     _target_type = target_type::d3d;
//     d3d.context()->OMSetRenderTargets(1, &Target, DepthStencil);
//     return drawing(Src);
//   }

//   static bool d2d_drawing() noexcept { return _target_type == target_type::d2d; }
//   static bool d3d_drawing() noexcept { return _target_type == target_type::d3d; }
//   static bool not_drawing() noexcept { return _target_type == target_type::none; }

//   std::expected<void, error> close() noexcept {
//     try {
//       if (!_active) return {};
//       _active = false;
//       if (d2d_drawing()) {
//         const auto& d2d = yw::d2d();
//         hresult_test(d2d.context()->EndDraw);
//         d2d.context()->SetTarget(nullptr);
//       } else if (d3d_drawing()) d3d().context()->OMSetRenderTargets(0, nullptr, nullptr);
//       _target = nullptr;
//       _target_type = target_type::none;
//     } catch (...) { return unexpected_error(errors::operation_failed, "unknown error occurred while closing
//     drawing"); } return {};
//   }
// };

/// MARK: draw line

inline std::expected<void, error> draw_line(float2 p0, float2 p1, float1 width = 1.0f) {
  if (!drawing::d2d_drawing()) return std::unexpected(error(errors::invalid_operation, "not in d2d drawing"));
  const auto d2d = yw::d2d();
  const auto brush = yw::brush();
  d2d.context()->DrawLine({p0.x, p0.y}, {p1.x, p1.y}, brush.d2d_brush(), width.x, brush.d2d_stroke());
  return {};
}

/// MARK: draw/fill_rectangle

inline std::expected<void, error> draw_rectangle(float2 pos, float2 size, float1 border_width = 1.0f) {
  if (!drawing::d2d_drawing()) return std::unexpected(error(errors::invalid_operation, "not in d2d drawing"));
  const auto d2d = yw::d2d();
  const auto brush = yw::brush();
  D2D1_RECT_F rect = D2D1::RectF(pos.x, pos.y, pos.x + size.x, pos.y + size.y);
  d2d.context()->DrawRectangle(&rect, brush.d2d_brush(), border_width.x, brush.d2d_stroke());
  return {};
}

inline std::expected<void, error> fill_rectangle(float2 pos, float2 size) {
  if (!drawing::d2d_drawing()) return std::unexpected(error(errors::invalid_operation, "not in d2d drawing"));
  const auto d2d = yw::d2d();
  const auto brush = yw::brush();
  d2d.context()->FillRectangle(D2D1_RECT_F(pos.x, pos.y, pos.x + size.x, pos.y + size.y), brush.d2d_brush());
  return {};
}

//////////////////////////////////// MARK: draw/fill_round_rectangle

inline std::expected<void, error> draw_round_rectangle(
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

//////////////////////////////////////// MARK: draw/fill_ellipse

inline std::expected<void, error> draw_ellipse(float2 center, float2 radius, float1 border_width = 1.0f) {
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

//////////////////////////////////////// MARK: draw/fill_geometry

inline std::expected<void, error> draw_geometry(ID2D1Geometry* geometry, float1 border_width = 1.0f) {
  if (!drawing::d2d_drawing()) return std::unexpected(error(errors::invalid_operation, "not in d2d drawing"));
  const auto d2d = yw::d2d();
  const auto brush = yw::brush();
  d2d.context()->DrawGeometry(geometry, brush.d2d_brush(), border_width.x, brush.d2d_stroke());
  return {};
}

inline std::expected<void, error> fill_geometry(ID2D1Geometry* geometry) {
  if (!drawing::d2d_drawing()) return std::unexpected(error(errors::invalid_operation, "not in d2d drawing"));
  const auto d2d = yw::d2d();
  const auto brush = yw::brush();
  d2d.context()->FillGeometry(geometry, brush.d2d_brush(), nullptr);
  return {};
}

//////////////////////////////////////// MARK: draw_text

inline std::expected<void, error> draw_text(
  float2 Pos, float2 Size, string_view<wchar_t> Text, IDWriteTextFormat* Format,
  D2D1_DRAW_TEXT_OPTIONS Options = D2D1_DRAW_TEXT_OPTIONS_NONE) {
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
