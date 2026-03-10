#pragma once
#include "ywx/bitmap.h"
#include "ywx/svgpath.h"

namespace yw::ui::part {

class icon {
public:
  using icon_variant = std::variant<std::monostate, yw::bitmap, yw::svgpath>;

  float2 size{};
  float2 padding{};
  icon_variant image{};
  color fill_color = colors::black; // used for svgpath fill
  color border_color = colors::black; // used for svgpath border

  std::expected<void, error_trace> draw(float2 Pos) const {
    if (std::holds_alternative<std::monostate>(image)) return {};
    const auto draw_pos = Pos + padding;
    const auto draw_size = size - padding * 2.0f;
    if (const auto* bitmap_ptr = std::get_if<yw::bitmap>(&image)) {
      const auto ratio = yw::min(draw_size.x / bitmap_ptr->size().x, draw_size.y / bitmap_ptr->size().y);
      const auto bitmap_draw_size = float2(bitmap_ptr->size()) * ratio;
      const auto bitmap_draw_pos = draw_pos + (draw_size - bitmap_draw_size) * 0.5f;
      if (auto res = draw_bitmap(bitmap_draw_pos, bitmap_draw_size, *bitmap_ptr); !res) return unexpected_error(res.error());
    } else if (const auto* svgpath_ptr = std::get_if<yw::svgpath>(&image)) {
      const auto ratio = yw::min(draw_size.x / svgpath_ptr->size().x, draw_size.y / svgpath_ptr->size().y);
      const auto svg_draw_size = float2(svgpath_ptr->size()) * ratio;
      const auto svg_draw_pos = draw_pos + (draw_size - svg_draw_size) * 0.5f;
      brush.color(fill_color);
      if (auto res = fill_svgpath(svg_draw_pos, svg_draw_size, *svgpath_ptr); !res) return unexpected_error(res.error());
      brush.color(border_color);
      if (auto res = draw_svgpath(svg_draw_pos, svg_draw_size, *svgpath_ptr, 1.0f); !res) return unexpected_error(res.error());
    }
    return {};
  }
};
} // namespace yw::ui::part
