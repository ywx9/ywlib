#pragma once
#include "ywx/ui_control.h"

namespace yw::ui {

/// MARK: frame

class frame : public control {
public:
  /// MARK: slot

  struct slot : public control::slot {
    bitmap background_image;
    color background_color = colors::transparent;
    float background_image_opacity = 1.0f;

    color border_color = colors::transparent;
    float border_thickness = 1.0f;
    bool border_dashed = false;

    std::expected<void, error_trace> draw() const override {
      if (!visible) return {};
      if (auto res = draw_background(); !res) return unexpected_error(res.error());
      if (auto res = draw_foreground(); !res) return unexpected_error(res.error());
      return {};
    }

    std::expected<void, error_trace> draw_background() const {
      if (background_color.a > 0.0f) {
        brush.color(background_color);
        if (auto res = fill_geometry(geometry.get()); !res) return unexpected_error(res.error());
      }
      if (crop_content) d2d.push_layer(geometry.get());
      if (background_image_opacity > 0.0f && background_image) {
        if (auto res = draw_bitmap(pos, size, background_image, background_image_opacity); !res)
          return unexpected_error(res.error());
      }
      return {};
    }

    std::expected<void, error_trace> draw_foreground() const {
      if (crop_content) d2d.pop_layer();
      if (border_color.a > 0.0f && border_thickness > 0.0f) {
        brush.color(border_color).dashed(border_dashed);
        if (auto res = draw_geometry(geometry.get(), border_thickness); !res) return unexpected_error(res.error());
        brush.dashed(false);
      }
      return {};
    }
  };

  /// MARK: background accessor

  class background_accessor : public accessor<frame> {
    using accessor<frame>::slot;

  public:
    const auto& image() const { return slot.background_image; }
    auto& image(bitmap Image) {
      slot.background_image = std::move(Image);
      dirty = true;
      return *this;
    }
    const auto& image_opacity() const { return slot.background_image_opacity; }
    auto& image_opacity(float Opacity) {
      slot.background_image_opacity = Opacity;
      dirty = true;
      return *this;
    }
    const auto& color() const { return slot.background_color; }
    auto& color(const yw::color Color) {
      slot.background_color = Color;
      dirty = true;
      return *this;
    }
  };

  /// MARK: border accessor

  class border_accessor : public accessor<frame> {
    using accessor<frame>::slot;

  public:
    const auto& color() const { return slot.border_color; }
    auto& color(const yw::color Color) {
      slot.border_color = Color;
      dirty = true;
      return *this;
    }
    const auto& thickness() const { return slot.border_thickness; }
    auto& thickness(float Thickness) {
      slot.border_thickness = Thickness;
      dirty = true;
      return *this;
    }
    const auto& dashed() const { return slot.border_dashed; }
    auto& dashed(bool Dashed) {
      slot.border_dashed = Dashed;
      dirty = true;
      return *this;
    }
  };

  /// MARK: handle functions

  template<typename Self> decltype(auto) background(this Self&& self) {
    return create_accessor<background_accessor>(self);
  }
  template<typename Self> decltype(auto) border(this Self&& self) { return create_accessor<border_accessor>(self); }
};
} // namespace yw::ui
