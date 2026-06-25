#pragma once
#include "ywx/accessor.h"
#include "ywx/control.h"

namespace yw::ui {

/// MARK: frame

class frame : public control {
public:
  struct slot : public control::slot {
    bitmap background_image;
    color background_color = colors::transparent;
    float background_image_opacity = 1.0f;

    color border_color = colors::transparent;
    float border_thickness = 1.0f;
    bool border_dashed = false;

    bool crop_content = false;

    virtual std::expected<void, error> draw() const override {
      if (!visible) return {};
      if (auto res = draw_background(); !res) return res.error().relay();
      if (auto res = draw_foreground(); !res) return res.error().relay();
      return {};
    }

    std::expected<void, error> draw_background() const {
      if (background_color.a > 0.0f) {
        brush::color(background_color);
        if (auto res = fill_geometry(geometry.get()); !res) return res.error().relay();
      }
      if (crop_content) d2d::push_layer(geometry.get());
      if (background_image_opacity > 0.0f && background_image)
        if (auto res = draw_bitmap(pos, size, background_image, background_image_opacity); !res)
          return res.error().relay();
      return {};
    }

    std::expected<void, error> draw_foreground() const {
      if (crop_content) d2d::pop_layer();
      if (border_color.a > 0.0f && border_thickness > 0.0f) {
        brush::color(border_color);
        brush::dashed(border_dashed);
        if (auto res = draw_geometry(geometry.get(), border_thickness); !res) return res.error().relay();
        brush::dashed(false);
      }
      return {};
    }
  };

  class background_accessor;
  class border_accessor;

  ywlib_make_accessor(background_accessor, frame);
  ywlib_make_accessor(border_accessor, frame);
};

/// MARK: background accessor

class frame::background_accessor : public accessor<frame> {
  using accessor<frame>::slot;

public:
  const auto& color() const&& { return slot.background_color; }
  const auto& image() const&& { return slot.background_image; }
  const auto& image_opacity() const&& { return slot.background_image_opacity; }

  auto&& color(const yw::color Color) && {
    slot.background_color = Color;
    dirty = true;
    return std::move(*this);
  }
  auto&& image(bitmap Image) && {
    slot.background_image = std::move(Image);
    dirty = true;
    return std::move(*this);
  }
  auto&& image_opacity(float1 Opacity) && {
    slot.background_image_opacity = Opacity.x;
    dirty = true;
    return std::move(*this);
  }
};

/// MARK: border accessor

class frame::border_accessor : public accessor<frame> {
  using accessor<frame>::slot;

public:
  const auto& color() const&& { return slot.border_color; }
  const auto& thickness() const&& { return slot.border_thickness; }
  const auto& dashed() const&& { return slot.border_dashed; }

  auto&& color(const yw::color Color) {
    slot.border_color = Color;
    dirty = true;
    return std::move(*this);
  }
  auto&& thickness(float1 Thickness) {
    slot.border_thickness = Thickness.x;
    dirty = true;
    return std::move(*this);
  }
  auto&& dashed(bool Dashed) {
    slot.border_dashed = Dashed;
    dirty = true;
    return std::move(*this);
  }
};
} // namespace yw::ui
