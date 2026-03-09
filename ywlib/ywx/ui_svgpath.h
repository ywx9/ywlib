#pragma once
#include "ywx/window.h"
#include "ywx/svgpath.h"

namespace yw::ui {

//////////////////////////////////////// MARK: ui::svgpath

/// control to display an SVG path geometry.
class svgpath : public base {
public:
  class slot : public base::slot {
  public:
    yw::svgpath path{};
    color fill_color = colors::black;
    color border_color = colors::black;
    float border_width = 1.0f;
    float2 padding{};

    virtual void draw() const override {
      if (!path) return;
      const auto draw_pos = pos + padding;
      const auto draw_size = size - padding * 2.0f;
      fill_svgpath(draw_pos, draw_size, path, fill_color);
      draw_svgpath(draw_pos, draw_size, path, border_color, border_width);
    }
  };

public:
  using base::operator bool;

  const auto& path() const { return unsafe_get(&slot::path); }
  const auto& fill_color() const { return unsafe_get(&slot::fill_color); }
  const auto& border_color() const { return unsafe_get(&slot::border_color); }
  const auto& border_width() const { return unsafe_get(&slot::border_width); }
  const auto& padding() const { return unsafe_get(&slot::padding); }

  void path(yw::svgpath value) { _set(&slot::path, std::move(value)); }
  void fill_color(const color& value) { _set(&slot::fill_color, value); }
  void border_color(const color& value) { _set(&slot::border_color, value); }
  void border_width(float value) { _set(&slot::border_width, value); }
  void padding(float2 value) { _set(&slot::padding, value); }

  template<included_in<window&, none> Window>
  static std::expected<svgpath, error_trace> add(Window&& w, float2 Pos, float2 Size) {
    if (auto res = base::add<svgpath>(w, Pos, Size)) return svgpath{std::move(res->first)};
    else return unexpected_error(res.error());
  }
};

} // namespace yw::ui
