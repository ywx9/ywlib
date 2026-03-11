#pragma once
#include "ywx/bitmap.h"
#include "ywx/svgpath.h"

namespace yw::ui::part {

class icon {
public:
  using icon_variant = std::variant<std::monostate, yw::bitmap, yw::svgpath>;

private:
  float2 _size{};
  float2 _padding{};
  icon_variant _image{};
  color _fill_color = colors::black;
  color _border_color = colors::black;

public:
  const auto& size() const { return _size; }
  auto& size() { return _size; }
  void size(float2 value) { _size = value; }

  const auto& padding() const { return _padding; }
  auto& padding() { return _padding; }
  void padding(float2 value) { _padding = value; }

  const auto& image() const { return _image; }
  void image(icon_variant value) { _image = std::move(value); }

  const auto& fill_color() const { return _fill_color; }
  auto& fill_color() { return _fill_color; }
  void fill_color(color value) { _fill_color = value; }

  const auto& border_color() const { return _border_color; }
  auto& border_color() { return _border_color; }
  void border_color(color value) { _border_color = value; }

  std::expected<void, error_trace> draw(float2 Pos) const {
    if (std::holds_alternative<std::monostate>(_image)) return {};
    const auto draw_pos = Pos + _padding;
    const auto draw_size = _size - _padding * 2.0f;
    if (const auto* bitmap_ptr = std::get_if<yw::bitmap>(&_image)) {
      const auto ratio = yw::min(draw_size.x / bitmap_ptr->size().x, draw_size.y / bitmap_ptr->size().y);
      const auto bitmap_draw_size = float2(bitmap_ptr->size()) * ratio;
      const auto bitmap_draw_pos = draw_pos + (draw_size - bitmap_draw_size) * 0.5f;
      if (auto res = draw_bitmap(bitmap_draw_pos, bitmap_draw_size, *bitmap_ptr); !res) return unexpected_error(res.error());
    } else if (const auto* svgpath_ptr = std::get_if<yw::svgpath>(&_image)) {
      const auto ratio = yw::min(draw_size.x / svgpath_ptr->size().x, draw_size.y / svgpath_ptr->size().y);
      const auto svg_draw_size = float2(svgpath_ptr->size()) * ratio;
      const auto svg_draw_pos = draw_pos + (draw_size - svg_draw_size) * 0.5f;
      brush.color(_fill_color);
      if (auto res = fill_svgpath(svg_draw_pos, svg_draw_size, *svgpath_ptr); !res) return unexpected_error(res.error());
      brush.color(_border_color);
      if (auto res = draw_svgpath(svg_draw_pos, svg_draw_size, *svgpath_ptr, 1.0f); !res) return unexpected_error(res.error());
    }
    return {};
  }
};
} // namespace yw::ui::part
