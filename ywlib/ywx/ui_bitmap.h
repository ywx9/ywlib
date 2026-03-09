#pragma once
#include "ywx/window.h"

namespace yw::ui {

//////////////////////////////////////// MARK: ui::bitmap

/// control to display a bitmap image.
class bitmap : public base {
public:
  class slot : public base::slot {
  public:
    yw::bitmap image{};
    float2 padding{};

    virtual void draw() const override {
      if (!image) return;
      const auto draw_pos = pos + padding;
      const auto draw_size = size - padding * 2.0f;
      draw_bitmap(draw_pos, draw_size, image);
    }
  };

public:
  using base::operator bool;

  const auto& image() const { return unsafe_get(&slot::image); }
  const auto& padding() const { return unsafe_get(&slot::padding); }

  void image(yw::bitmap value) { _set(&slot::image, std::move(value)); }
  void padding(float2 value) { _set(&slot::padding, value); }

  template<included_in<window&, none> Window>
  static std::expected<bitmap, error_trace> add(Window&& w, float2 Pos, float2 Size) {
    if (auto res = base::add<bitmap>(w, Pos, Size)) return bitmap{std::move(res->first)};
    else return unexpected_error(res.error());
  }
};

} // namespace yw::ui
