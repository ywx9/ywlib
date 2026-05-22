#pragma once
#include "ywx/bitmap.h"
#include "ywx/svgpath.h"
#include "ywx/ui_parts.h"

namespace yw::ui::parts {

struct icon : public base {
  using icon_data = std::variant<std::monostate, yw::svgpath, yw::bitmap>;
  static constexpr float2 default_size = {20.0f, 20.0f};

  icon_data data{};
  float4 padding = float4::fill(arbitrary_value);
  float2 size = default_size;
  yw::color fill_color = colors::black;
  yw::color stroke_color = colors::black;
  float stroke_width = 1.0f;
  ui::alignment alignment = ui::alignment::left;

  float2 bounds() const noexcept { return size + padding.xy() + padding.zw(); }

  bool empty() const noexcept { return data.index() == 0; }
  bool is_svgpath() const noexcept { return std::holds_alternative<yw::svgpath>(data); }
  bool is_bitmap() const noexcept { return std::holds_alternative<yw::bitmap>(data); }

  std::expected<void, error_trace> draw_icon_data(float2 Origin, float2 Size) const {
    if (std::holds_alternative<svgpath>(data)) {
      const auto& svg = std::get<svgpath>(data);
      brush.color(fill_color);
      if (auto res = fill_svgpath(Origin, Size, svg); !res) return unexpected_error(res.error());
      if (stroke_width <= 0.0f || stroke_color.a <= 0) return {};
      brush.color(stroke_color);
      if (auto res = stroke_svgpath(Origin, Size, svg, stroke_width); !res) return unexpected_error(res.error());
    } else if (std::holds_alternative<bitmap>(data)) {
      const auto& bmp = std::get<bitmap>(data);
      if (auto res = draw_bitmap(Origin, Size, bmp); !res) return unexpected_error(res.error());
      if (stroke_width <= 0.0f || stroke_color.a <= 0) return {};
      brush.color(stroke_color);
      if (auto res = draw_rectangle(Origin, Size, stroke_width); !res) return unexpected_error(res.error());
    }
    return {};
  }

  std::expected<void, error_trace> draw(float2 Pos, float2 Area) const {
    if (size.x <= 0.0f || size.y <= 0.0f) return {};
    static const float c[] = {0.5f, 0.0f, 1.0f};
    const auto cc = float2(c[unsigned(alignment) % 3], c[(unsigned(alignment) / 3) % 3]);
    const float2 origin = Pos + padding.xy() + (Area - bounds()) * cc;
    if (auto res = draw_icon_data(origin, size); !res) return unexpected_error(res.error());
    return {};
  }

  class accessor : public base::accessor<icon> {
    using base::accessor<icon>::part;

  public:
    const auto& data() const { return part.data; }
    auto& data(icon_data Data) {
      part.data = std::move(Data);
      part.view_changed = true;
      return *this;
    }

    auto& svgpath(yw::svgpath Path) {
      part.data.template emplace<1>(std::move(Path));
      part.view_changed = true;
      return *this;
    }

    auto& bitmap(yw::bitmap Bitmap) {
      part.data.template emplace<2>(std::move(Bitmap));
      part.view_changed = true;
      return *this;
    }

    auto& clear() {
      part.data = std::monostate{};
      part.view_changed = true;
      return *this;
    }

    const auto& alignment() const { return part.alignment; }
    auto& alignment(ui::alignment Alignment) {
      part.alignment = Alignment;
      part.view_changed = true;
      return *this;
    }

    const auto& fill_color() const { return part.fill_color; }
    auto& fill_color(yw::color Color) {
      part.fill_color = Color;
      part.view_changed = true;
      return *this;
    }

    const auto& stroke_color() const { return part.stroke_color; }
    auto& stroke_color(yw::color Color) {
      part.stroke_color = Color;
      part.view_changed = true;
      return *this;
    }

    const auto& stroke_width() const { return part.stroke_width; }
    auto& stroke_width(float1 Width) {
      part.stroke_width = yw::max(0.0f, Width.x);
      part.view_changed = true;
      return *this;
    }

    const auto& size() const { return part.size; }
    auto& size(float2 Size) {
      part.size = vapply_r<float2>(yw::max, float2(), Size);
      part.layout_changed = true;
      return *this;
    }

    const auto& width() const { return part.size.x; }
    auto& width(float1 Width) {
      part.size.x = yw::max(0.0f, Width.x);
      part.layout_changed = true;
      return *this;
    }

    const auto& height() const { return part.size.y; }
    auto& height(float1 Height) {
      part.size.y = yw::max(0.0f, Height.x);
      part.layout_changed = true;
      return *this;
    }

    const auto& padding() const { return part.padding; }
    auto& padding(float4 Padding) {
      part.padding = Padding;
      part.layout_changed = true;
      return *this;
    }

    auto bounds() const { return part.bounds(); }

    bool empty() const noexcept { return part.empty(); }
    bool is_svgpath() const noexcept { return part.is_svgpath(); }
    bool is_bitmap() const noexcept { return part.is_bitmap(); }
  };

  accessor access() & noexcept { return {*this}; }
};
} // namespace yw::ui::parts
