#pragma once
#include "ywx/ui_control.h"

namespace yw::ui {

using icon_variant = std::variant<std::monostate, bitmap, svgpath>;

class icon : public control {
public:
  class slot : public control::slot {
    std::expected<void, error_trace> draw_icon(float2 Pos, float2 Size) const {
      if (std::holds_alternative<svgpath>(icon)) {
        brush.color(fill_color);
        if (auto res = fill_svgpath(Pos, Size, std::get<svgpath>(icon)); !res) return unexpected_error(res.error());
        brush.color(stroke_color);
        if (auto res = stroke_svgpath(Pos, Size, std::get<svgpath>(icon), stroke_width); !res) return unexpected_error(res.error());
      } else if (std::holds_alternative<bitmap>(icon))
        if (auto res = draw_bitmap(Pos, Size, std::get<bitmap>(icon)); !res) return unexpected_error(res.error());
      return {};
    }

  public:
    icon_variant icon;
    color fill_color = colors::transparent;
    color stroke_color = colors::black;
    float stroke_width = 1.0f;

    float2 icon_size() const {
      return std::visit([](auto&& i) -> float2 {
        if constexpr (std::same_as<std::decay_t<decltype(i)>, std::monostate>) return {};
        else return i ? float2(i.size()) : float2();
      }, icon);
    }

    virtual void draw(float2 Pos, float2 Size) const override {
      if (Size.x <= 0.0f || Size.y <= 0.0f) return;
      update_last_rect(Pos, Size);
      draw();
    }

    virtual void draw() const override {
      const auto ctrl_sz = last_rect.zw() - last_rect.xy();
      auto icon_sz = icon_size();
      const float scale = yw::min(ctrl_sz.x / icon_sz.x, ctrl_sz.y / icon_sz.y);
      icon_sz *= scale;
      const auto offset = (ctrl_sz - icon_sz) * 0.5f;
      draw_icon(last_rect.xy() + offset, icon_sz);
    }
  };

  using control::operator bool;
  icon() noexcept = default;

  icon(derived_from<unknown> auto& Layout) {
    if (auto res = create_control<icon>(Layout)) _id = *res;
  }

  const auto& object() const { return unsafe_get(&slot::icon); }
  const auto& fill_color() const { return unsafe_get(&slot::fill_color); }
  const auto& stroke_color() const { return unsafe_get(&slot::stroke_color); }
  float stroke_width() const { return unsafe_get(&slot::stroke_width); }

  void object(icon_variant new_icon) { safe_set(&slot::icon, std::move(new_icon)); }

  void fill_color(const color& c) { safe_set(&slot::fill_color, c); }
  void stroke_color(const color& c) { safe_set(&slot::stroke_color, c); }
  void stroke_width(float w) { safe_set(&slot::stroke_width, w); }
};
}
