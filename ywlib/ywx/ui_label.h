#pragma once
#include "ywx/background.h"
#include "ywx/text.h"
#include "ywx/ui_plain.h"

namespace yw::ui {

class label : public control {
public:
  class slot : public control::slot {
  public:
    yw::background background = colors::white;
    color border_color = colors::black;
    float border_width = 1.0f;
    float4 padding = float4::fill(5.0f);

    yw::text text = assume(yw::text::create(L""));
    color text_color = colors::black;

    function<void, event::button> on_button;
    function<void, event::wheel> on_wheel;

    virtual float2 get_minimum_draw_size() const noexcept {
      const auto tsz = text.size() + padding.xy() + padding.zw();
      auto result = float2(ucc.x ? 0.0f : size.x, ucc.y ? 0.0f : size.y);
      result.x = yw::max(result.x, minimum_size.x, tsz.x);
      result.y = yw::max(result.y, minimum_size.y, tsz.y);
      return result;
    }

    virtual void draw() const override {
      if (!visible) return;
      draw_background(pos, size, background);
      brush.color(text_color);
      const auto tsz = text.size() + padding.xy() + padding.zw();
      draw_text(pos + padding.xy() + (size - tsz) * 0.5f, text);
    }

    virtual void button_event(event::button Event) override {
      if (enabled && on_button) on_button(Event);
    }

    virtual void wheel_event(event::wheel Event) override {
      if (enabled && on_wheel) on_wheel(Event);
    }
  };

  using control::operator bool;
  label() noexcept = default;
  label(derived_from<unknown> auto& Layout) {
    if (auto res = create_control<label>(Layout)) _id = *res;
  }

  const auto& background() const { return unsafe_get(&slot::background); }
  void background(yw::background bg) { safe_set(&slot::background, bg); }

  const auto& border_color() const { return unsafe_get(&slot::border_color); }
  void border_color(const color& c) { safe_set(&slot::border_color, c); }

  const auto& border_width() const { return unsafe_get(&slot::border_width); }
  void border_width(float1 w) { safe_set(&slot::border_width, w.x); }

  const auto& padding() const { return unsafe_get(&slot::padding); }
  void padding(const float4& p) { safe_set_size(&slot::padding, p); }

  const auto& text() const { return unsafe_get(&slot::text); }
  auto& text() { return unsafe_get_size(&slot::text); }
  template<stringable S> void text(S&& Text) { safe_set_size(&slot::text, static_cast<S&&>(Text)); }

  const auto& text_color() const { return unsafe_get(&slot::text_color); }
  void text_color(const color& c) { safe_set(&slot::text_color, c); }

  const auto& on_button() const { return unsafe_get(&slot::on_button); }
  void on_button(function<void, event::button> cb) { safe_set(&slot::on_button, cb); }

  const auto& on_wheel() const { return unsafe_get(&slot::on_wheel); }
  void on_wheel(function<void, event::wheel> cb) { safe_set(&slot::on_wheel, cb); }
};
} // namespace yw::ui
