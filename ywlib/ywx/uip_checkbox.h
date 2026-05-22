#pragma once
#include "ywx/bitmap.h"
#include "ywx/svgpath.h"
#include "ywx/uip_icon.h"

namespace yw::ui::parts {

struct checkbox : public base {
  parts::icon box{.fill_color = colors::white};
  parts::icon check{};
  bool checked = false;

  float2 bounds() const noexcept { return box.bounds(); }

  std::expected<void, error_trace> draw(float2 Pos, float2 Area) const {
    if (box.size.x <= 0.0f || box.size.y <= 0.0f) return {};
    constexpr float c[] = {0.5f, 0.0f, 1.0f};
    const auto cc = float2(c[unsigned(box.alignment) % 3], c[(unsigned(box.alignment) / 3) % 3]);
    const float2 origin = Pos + box.padding.xy() + (Area - bounds()) * cc;
    if (auto res = box.draw_icon_data(origin, box.size); !res) return unexpected_error(res.error());
    if (checked)
      if (auto res = check.draw_icon_data(origin, check.size); !res) return unexpected_error(res.error());
    return {};
  }

  class accessor : public base::accessor<checkbox> {
    using base::accessor<checkbox>::part;

  public:
    const auto& box_icon() const noexcept { return part.box.data; }
    auto& box_icon(parts::icon::icon_data Data) noexcept {
      part.box.data = std::move(Data);
      part.view_changed = true;
      return *this;
    }

    const auto& box_fill_color() const noexcept { return part.box.fill_color; }
    auto& box_fill_color(yw::color Color) noexcept {
      part.box.fill_color = Color;
      part.view_changed = true;
      return *this;
    }

    const auto& box_stroke_color() const noexcept { return part.box.stroke_color; }
    auto& box_stroke_color(yw::color Color) noexcept {
      part.box.stroke_color = Color;
      part.view_changed = true;
      return *this;
    }

    const auto& box_stroke_width() const noexcept { return part.box.stroke_width; }
    auto& box_stroke_width(float1 Width) noexcept {
      part.box.stroke_width = yw::max(0.0f, Width.x);
      part.view_changed = true;
      return *this;
    }

    const auto& check_icon() const noexcept { return part.check.data; }
    auto& check_icon(parts::icon::icon_data Data) noexcept {
      part.check.data = std::move(Data);
      part.view_changed = true;
      return *this;
    }

    const auto& check_fill_color() const noexcept { return part.check.fill_color; }
    auto& check_fill_color(yw::color Color) noexcept {
      part.check.fill_color = Color;
      part.view_changed = true;
      return *this;
    }

    const auto& check_stroke_color() const noexcept { return part.check.stroke_color; }
    auto& check_stroke_color(yw::color Color) noexcept {
      part.check.stroke_color = Color;
      part.view_changed = true;
      return *this;
    }

    const auto& check_stroke_width() const noexcept { return part.check.stroke_width; }
    auto& check_stroke_width(float1 Width) noexcept {
      part.check.stroke_width = yw::max(0.0f, Width.x);
      part.view_changed = true;
      return *this;
    }

    const auto& padding() const noexcept { return part.box.padding; }
    auto& padding(float2 Padding) noexcept {
      part.box.padding = Padding;
      part.check.padding = Padding;
      part.layout_changed = true;
      return *this;
    }

    const auto& size() const noexcept { return part.box.size; }
    auto& size(float2 Size) noexcept {
      part.box.size = Size;
      part.check.size = Size;
      part.layout_changed = true;
      return *this;
    }

    const auto& alignment() const noexcept { return part.box.alignment; }
    auto& alignment(ui::alignment Alignment) noexcept {
      part.box.alignment = Alignment;
      part.check.alignment = Alignment;
      part.layout_changed = true;
      return *this;
    }

    const auto& checked() const noexcept { return part.checked; }
  };

  accessor access() & noexcept { return {*this}; }
};
} // namespace yw::ui::parts
