#pragma once
#include "ywx/bitmap.h"
#include "ywx/svgpath.h"

namespace yw::ui {

//////////////////////////////////////// MARK: ui::icon

class icon : public base {
public:
  using icon_variant = std::variant<std::monostate, yw::bitmap, yw::svgpath>;

  class slot : public base::slot {
  public:
    icon_variant image{};
    color path_fill_color = colors::black;
    color path_border_color = colors::black;
    float2 padding{};

    virtual void draw() const override {
      if (std::holds_alternative<std::monostate>(image)) {
        // Empty icon
      } else if (const auto* bitmap_ptr = std::get_if<yw::bitmap>(&image)) {
        if (*bitmap_ptr) {
          const auto draw_pos = pos + padding;
          const auto draw_size = size - padding * 2.0f;
          draw_bitmap(draw_pos, draw_size, *bitmap_ptr);
        }
      } else if (const auto* svgpath_ptr = std::get_if<yw::svgpath>(&image)) {
        if (*svgpath_ptr) {
          const auto draw_pos = pos + padding;
          const auto draw_size = size - padding * 2.0f;
          fill_svgpath(draw_pos, draw_size, *svgpath_ptr, path_fill_color);
          draw_svgpath(draw_pos, draw_size, *svgpath_ptr, path_border_color, 1.0f);
        }
      }
    }
  };

public:
  using base::operator bool;

  const auto& image() const { return unsafe_get(&slot::image); }
  const auto& path_fill_color() const { return unsafe_get(&slot::path_fill_color); }
  const auto& path_border_color() const { return unsafe_get(&slot::path_border_color); }
  const auto& padding() const { return unsafe_get(&slot::padding); }

  void image(icon_variant value) { _set(&slot::image, std::move(value)); }
  void path_fill_color(const color& value) { _set(&slot::path_fill_color, value); }
  void path_border_color(const color& value) { _set(&slot::path_border_color, value); }
  void padding(float2 value) { _set(&slot::padding, value); }

  template<included_in<window&, none> Window>
  static std::expected<icon, error_trace> add(Window&& w, float2 Pos, float2 Size) {
    if (auto res = base::add<icon>(w, Pos, Size)) return icon(std::move(res->first));
    else return unexpected_error(res.error());
  }
};
} // namespace yw::ui
