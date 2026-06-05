#pragma once
#include "ywx/svgpath.h"
#include "ywx/ui_control.h"

namespace yw::ui {

class icon : public control {
public:
  /// MARK: slot

  struct slot : public control::slot {
    svgpath icon;
    color fill_color = colors::black;
    color stroke_color = colors::black;
    float stroke_width = 1.0f;

    virtual std::expected<float2, error_trace> calculate_necessary_size() const override {
      const auto inner = icon.size() + padding.xy() + padding.zw();
      return vapply_r<float2>(_calc_nec_size, size_policy, minimum_size, required_size, inner);
    }

    virtual std::expected<void, error_trace> draw() const override {
      if (!visible || !icon) return {};
      const auto _pos = pos + padding.xy();
      const auto _size = size - padding.xy() - padding.zw();
      if (fill_color.a > 0.0f) {
        brush().color(fill_color);
        if (auto res = fill_svgpath(_pos, _size, icon); !res) return unexpected_error(res.error());
      }
      if (stroke_color.a > 0.0f) {
        brush().color(stroke_color);
        if (auto res = stroke_svgpath(_pos, _size, icon, stroke_width); !res) return unexpected_error(res.error());
      }
      return {};
    }
  };

  /// MARK: icon accessor

  class icon_accessor : public accessor<icon> {
    using accessor<icon>::slot;

  public:
    const auto& icon() const { return slot.icon; }
    auto& icon(svgpath Icon) {
      slot.icon = std::move(Icon);
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

  static std::expected<icon, error_trace> add(
    derived_from<unknown> auto& Layout, const color_pair& Colors = color_pair::auto_color()) {
    icon icn;
    if (auto res = create_control<icon>(Layout)) icn._id = *res;
    else return unexpected_error(res.error());
    const auto csp = system::get_slot_pointer<icon>(icn._id);
    if (!csp) return unexpected_error(errors::invalid_slotid);
    csp->margin = {}, csp->padding = {};
    csp->fill_color = Colors.background;
    csp->stroke_color = Colors.foreground;
    csp->crop_content = false;
    return std::move(icn);
  }

  template<typename Self> decltype(auto) icon_(this Self&& self) { return create_accessor<icon_accessor>(self); }
};
} // namespace yw::ui
