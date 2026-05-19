#pragma once
#include "ywx/bitmap.h"
#include "ywx/svgpath.h"
#include "ywx/ui_parts.h"

namespace yw::ui::parts {

struct icon : public part_base {
  using icon_data = std::variant<std::monostate, yw::svgpath, yw::bitmap>;

  icon_data data{};
  float4 padding = float4::fill(arbitrary_value);
  ui::alignment alignment = ui::alignment::center;
  yw::color fill_color = colors::black;
  yw::color stroke_color = colors::black;
  float stroke_width = 1.0f;
  float2 size = float2::fill(arbitrary_value * 2.0f);

  float2 bounds() const noexcept { return size + padding.xy() + padding.zw(); }

  bool empty() const noexcept { return data.index() == 0; }
  bool is_svgpath() const noexcept { return std::holds_alternative<yw::svgpath>(data); }
  bool is_bitmap() const noexcept { return std::holds_alternative<yw::bitmap>(data); }

  std::expected<void, error_trace> draw(float2 Pos, float2 Lot) const {
    if (size.x <= 0.0f || size.y <= 0.0f) return {};
    static const float c[] = {0.5f, 0.0f, 1.0f};
    const auto cc = float2(c[unsigned(alignment) % 3], c[(unsigned(alignment) / 3) % 3]);
    const float2 origin = Pos + padding.xy() + (Lot - bounds()) * cc;
    if (std::holds_alternative<svgpath>(data)) {
      brush.color(fill_color);
      if (auto res = fill_svgpath(origin, size, std::get<svgpath>(data)); !res) return unexpected_error(res.error());
      if (stroke_width > 0.0f && stroke_color.a > 0) {
        brush.color(stroke_color);
        if (auto res = stroke_svgpath(origin, size, std::get<svgpath>(data), stroke_width); !res)
          return unexpected_error(res.error());
      }
    } else if (std::holds_alternative<bitmap>(data)) {
      if (auto res = draw_bitmap(origin, size, std::get<bitmap>(data)); !res) return unexpected_error(res.error());
      if (stroke_width > 0.0f && stroke_color.a > 0) {
        brush.color(stroke_color);
        if (auto res = draw_rectangle(origin, size, stroke_width); !res) return unexpected_error(res.error());
      }
    }
    return {};
  }

  class handle : public part_base::handle<icon> {
    friend struct icon;
    using part_base::handle<icon>::handle;
    using part_base::handle<icon>::_p;

  public:
    const auto& data() const { return _p->data; }

    auto& data(icon_data Data) {
      _p->data = std::move(Data);
      _p->view_changed = true;
      return *this;
    }

    auto& svgpath(yw::svgpath Path) {
      _p->data.template emplace<1>(std::move(Path));
      _p->view_changed = true;
      return *this;
    }

    auto& bitmap(yw::bitmap Bitmap) {
      _p->data.template emplace<2>(std::move(Bitmap));
      _p->view_changed = true;
      return *this;
    }

    auto& clear() {
      _p->data = std::monostate{};
      _p->view_changed = true;
      return *this;
    }

    const auto& alignment() const { return _p->alignment; }
    auto& alignment(ui::alignment Alignment) {
      _p->alignment = Alignment;
      _p->view_changed = true;
      return *this;
    }

    const auto& fill_color() const { return _p->fill_color; }
    auto& fill_color(yw::color Color) {
      _p->fill_color = Color;
      _p->view_changed = true;
      return *this;
    }

    const auto& stroke_color() const { return _p->stroke_color; }
    auto& stroke_color(yw::color Color) {
      _p->stroke_color = Color;
      _p->view_changed = true;
      return *this;
    }

    const auto& stroke_width() const { return _p->stroke_width; }
    auto& stroke_width(float1 Width) {
      _p->stroke_width = yw::max(0.0f, Width.x);
      _p->view_changed = true;
      return *this;
    }

    const auto& size() const { return _p->size; }
    auto& size(float2 Size) {
      _p->size = vapply_r<float2>(yw::max, float2(), Size);
      _p->layout_changed = true;
      return *this;
    }

    const auto& width() const { return _p->size.x; }
    auto& width(float1 Width) {
      _p->size.x = yw::max(0.0f, Width.x);
      _p->layout_changed = true;
      return *this;
    }

    const auto& height() const { return _p->size.y; }
    auto& height(float1 Height) {
      _p->size.y = yw::max(0.0f, Height.x);
      _p->layout_changed = true;
      return *this;
    }

    const auto& padding() const { return _p->padding; }
    auto& padding(float4 Padding) {
      _p->padding = Padding;
      _p->layout_changed = true;
      return *this;
    }

    auto bounds() const { return _p->bounds(); }

    bool empty() const noexcept { return _p->empty(); }
    bool is_svgpath() const noexcept { return _p->is_svgpath(); }
    bool is_bitmap() const noexcept { return _p->is_bitmap(); }
  };

  handle handle() noexcept { return *this; }
};
} // namespace yw::ui::parts
