#pragma once
#include "ywx/text.h"
#include "ywx/ui_control.h"

namespace yw::ui {

class button : public control {
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
    function<void, key> on_click;
    function<void, bool> on_focus;
    function<void, event::key> on_keydown;
    function<void, event::key> on_keyup;
    function<void, event::move> on_move;
    function<void, event::wheel> on_wheel;

    key captured_key{};

    virtual float2 calculate_size() const noexcept override {
      const auto inner = text.size() + padding.xy() + padding.zw();
      return vapply_r<float2>(yw::max, float2(), min_size, inner, size * constrained);
    }

    virtual void update_size() noexcept override {
      min_size = vapply_r<float2>(yw::max, min_size, float2());
      const auto inner = text.size() + padding.xy() + padding.zw();
      size = vapply_r<float2>(yw::max, min_size, inner, size * constrained);
    }

    virtual void draw() const override {
      if (!visible) return;
      draw_background(pos, size, background);
      brush.color(border_color);
      draw_round_rectangle(pos, size, radius, border_width);
      brush.color(text_color);
      const auto tsz = text.size() + padding.xy() + padding.zw();
      draw_text(pos + padding.xy() + (size - tsz) * 0.5f, text);
    }

    virtual void click_event(event::button e) override {
      if (enabled && e.code == captured_key) click_action();
      captured_key = {};
    }

    virtual void button_event(event::button e) override {
      if (!enabled) return;
      if (e.down) captured_key = e.code;
      else captured_key = {};
      if (on_button) on_button(e);
    }

    virtual bool focus_event(bool focused) override {
      if (!focused) captured_key = {};
      if (enabled && on_focus) on_focus(focused);
      return enabled;
    }

    virtual void key_event(event::key e) override {
      if (!enabled) return;
      if (e.down) {
        captured_key = e.code;
        if (e.first && on_keydown) on_keydown(e);
      } else {
        if (e.code == captured_key) click_action();
        else captured_key = {};
        if (on_keyup) on_keyup(e);
      }
    }

    virtual void wheel_event(event::wheel Event) override {
      if (enabled && on_wheel) on_wheel(Event);
    }

    virtual void click_action() {
      switch (captured_key) {
      case key::lbutton:
      case key::enter:
      case key::space:
        if (on_click) on_click(captured_key);
      }
    }
  };

  using control::operator bool;
  button() noexcept = default;
  button(derived_from<unknown> auto& Layout) {
    if (auto res = create_control<button>(Layout)) _id = *res;
  }

  const auto& background() const { return unsafe_get(&slot::background); }
  void background(yw::background bg) { safe_set(&slot::background, std::move(bg)); }

  const auto& border_color() const { return unsafe_get(&slot::border_color); }
  void border_color(const color& c) { safe_set(&slot::border_color, c); }

  const auto& border_width() const { return unsafe_get(&slot::border_width); }
  void border_width(float w) { safe_set(&slot::border_width, w); }

  const auto& padding() const { return unsafe_get(&slot::padding); }
  void padding(const float4& p) { safe_set_size(&slot::padding, p); }

  const auto& text() const { return unsafe_get(&slot::text); }
  auto& text() { return unsafe_get_size(&slot::text); }
  template<stringable S> void text(S&& t) { safe_set(&slot::text, static_cast<S&&>(t)); }

  const auto& text_color() const { return unsafe_get(&slot::text_color); }
  void text_color(const color& c) { safe_set(&slot::text_color, c); }

  const auto& on_button() const { return unsafe_get(&slot::on_button); }
  void on_button(function<void, event::button> f) { safe_set(&slot::on_button, std::move(f)); }

  const auto& on_click() const { return unsafe_get(&slot::on_click); }
  void on_click(function<void, key> f) { safe_set(&slot::on_click, std::move(f)); }

  const auto& on_focus() const { return unsafe_get(&slot::on_focus); }
  void on_focus(function<void, bool> f) { safe_set(&slot::on_focus, std::move(f)); }

  const auto& on_keydown() const { return unsafe_get(&slot::on_keydown); }
  void on_keydown(function<void, event::key> f) { safe_set(&slot::on_keydown, std::move(f)); }

  const auto& on_keyup() const { return unsafe_get(&slot::on_keyup); }
  void on_keyup(function<void, event::key> f) { safe_set(&slot::on_keyup, std::move(f)); }

  const auto& on_move() const { return unsafe_get(&slot::on_move); }
  void on_move(function<void, event::move> f) { safe_set(&slot::on_move, std::move(f)); }

  const auto& on_wheel() const { return unsafe_get(&slot::on_wheel); }
  void on_wheel(function<void, event::wheel> f) { safe_set(&slot::on_wheel, std::move(f)); }
};
} // namespace yw::ui
