#pragma once
#include "ywx/svgpath.h"
#include "ywx/ui_frame.h"

namespace yw::ui {

class icon : public frame {
public:
  /// MARK: slot

  struct slot : public frame::slot {
    std::variant<bitmap, svgpath> icon;
    std::optional<float2> icon_size;
    color fill_color = colors::black;
    color stroke_color = colors::black;
    float stroke_width = 1.0f;

    virtual std::expected<float2, error_trace> calculate_necessary_size() const override {
      const auto _icon_size = std::visit([](auto&& i) { return float2(i.size()); }, icon);
      const auto inner = icon_size.value_or(_icon_size) + padding.xy() + padding.zw();
      return vapply_r<float2>(yw::max, minimum_size, required_size * constrained, inner);
    }

    virtual std::expected<void, error_trace> ensure_necessary_size() override {
      if (auto res = calculate_necessary_size()) size = *res;
      else return unexpected_error(res.error());
      return {};
    }

    virtual std::expected<void, error_trace> draw() const override {
      if (!visible) return {};
      if (auto res = draw_background(); !res) return unexpected_error(res.error());
      const auto _pos = pos + padding.xy();
      const auto _size = size - padding.xy() - padding.zw();
      if (std::holds_alternative<svgpath>(icon)) {
        const auto& i = std::get<svgpath>(icon);
        if (!i) return {};
        if (fill_color.a > 0.0f) {
          brush.color(fill_color);
          if (auto res = fill_svgpath(_pos, _size, i); !res) return unexpected_error(res.error());
        }
        if (stroke_color.a > 0.0f) {
          brush.color(stroke_color);
          if (auto res = stroke_svgpath(_pos, _size, i, stroke_width); !res) return unexpected_error(res.error());
        }
      } else if (std::holds_alternative<bitmap>(icon)) {
        const auto& i = std::get<bitmap>(icon);
        if (!i) return {};
        if (fill_color.a > 0.0f) {
          brush.color(fill_color);
          if (auto res = fill_rectangle(_pos, _size); !res) return unexpected_error(res.error());
        }
        if (auto res = draw_bitmap(_pos, _size, i); !res) return unexpected_error(res.error());
        if (stroke_color.a > 0.0f) {
          brush.color(stroke_color);
          if (auto res = draw_rectangle(_pos, _size, stroke_width); !res) return unexpected_error(res.error());
        }
      }
      if (auto res = draw_foreground(); !res) return unexpected_error(res.error());
      return {};
    }
  };

  /// MARK: icon accessor

  class icon_accessor : public accessor<icon> {
    using accessor<icon>::slot;

  public:
    const auto& icon() const { return slot.icon; }
    auto& icon(std::variant<bitmap, svgpath> Icon) {
      slot.icon = std::move(Icon);
      this->dirty = true;
      return *this;
    }
    const auto& icon_size() const { return slot.icon_size; }
    auto& icon_size(std::optional<float2> IconSize) {
      slot.icon_size = std::move(IconSize);
      this->dirty = true;
      return *this;
    }
    const auto& stroke_color() const { return slot.stroke_color; }
    auto& stroke_color(color StrokeColor) {
      slot.stroke_color = StrokeColor;
      this->dirty = true;
      return *this;
    }
    const auto& fill_color() const { return slot.fill_color; }
    auto& fill_color(color FillColor) {
      slot.fill_color = FillColor;
      this->dirty = true;
      return *this;
    }
    const auto& stroke_width() const { return slot.stroke_width; }
    auto& stroke_width(float StrokeWidth) {
      slot.stroke_width = StrokeWidth;
      this->dirty = true;
      return *this;
    }
  };

  /// MARK: handle functions

  using control::operator bool;
  icon() noexcept = default;

  static std::expected<icon, error_trace> add(derived_from<unknown> auto& Layout) {
    icon icn;
    if (auto res = create_control<icon>(Layout)) icn._id = *res;
    else return unexpected_error(res.error());
    return std::move(icn);
  }

  template<typename Self> decltype(auto) icon_(this Self&& self) { return create_accessor<icon_accessor>(self); }
};
} // namespace yw::ui
