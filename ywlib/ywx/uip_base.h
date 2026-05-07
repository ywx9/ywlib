#pragma once
#include "ywx/bitmap.h"

namespace yw::ui::part {

struct base {
  color background_color = colors::white;
  yw::bitmap background_image; // optional
  float background_image_opacity = 1.0f;
  color border_color = colors::black;
  float border_width = 1.0f;
  comptr<ID2D1Geometry> geometry;

  class handle {
    friend class base;
    base* _p = nullptr;
    handle(base& Ref) noexcept : _p(&Ref) {}

  public:
    const color& background_color() const { return _p->background_color; }
    handle& background_color(color Color) { return _p->background_color = Color, *this; }

    const yw::bitmap& background_image() const { return _p->background_image; }
    handle& background_image(yw::bitmap Bitmap) { return _p->background_image = std::move(Bitmap), *this; }

    const color& border_color() const { return _p->border_color; }
    handle& border_color(color Color) { return _p->border_color = Color, *this; }

    float border_width() const { return _p->border_width; }
    handle& border_width(float BorderWidth) { return _p->border_width = BorderWidth, *this; }
  };

  handle handle() noexcept { return *this; }
};
}
