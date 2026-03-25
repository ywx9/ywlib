#pragma once
#include "ywx/tooltip.h"
#include "ywx/ui_window.h"

namespace yw::ui {

class plain : public control {
public:
  class slot : public control::slot {
  protected:
    void draw_plain(float2 Pos, float2 Size) const {
      print(source());
      brush.color(bg_color);
      fill_round_rectangle(Pos, Size, radius);
      brush.color(border_color);
      draw_round_rectangle(Pos, Size, radius, border_width);
      print(source());
    }

  public:
    color bg_color = colors::white;
    color border_color = colors::transparent;
    float1 border_width = 1.0f;

    function<void, event::button> on_button;
    function<void, event::wheel> on_wheel;

    virtual void draw() const override {
      print(source());
      draw_plain(pos, size);
      print(source());
    }

    virtual void button_event(event::button Event) override {
      if (enabled && on_button) on_button(Event);
    }

    virtual void wheel_event(event::wheel Event) override {
      if (enabled && on_wheel) on_wheel(Event);
    }
  };

  using control::operator bool;
  plain() noexcept = default;
  plain(derived_from<unknown> auto& Layout) {
    if (auto res = create_control<plain>(Layout)) _id = *res;
  }


  const auto& bg_color() const { return unsafe_get(&slot::bg_color); }
  const auto& border_color() const { return unsafe_get(&slot::border_color); }
  const auto& border_width() const { return unsafe_get(&slot::border_width); }
  const auto& on_button() const { return unsafe_get(&slot::on_button); }
  const auto& on_wheel() const { return unsafe_get(&slot::on_wheel); }

  void bg_color(const color& value) { safe_set(&slot::bg_color, value); }
  void border_color(const color& value) { safe_set(&slot::border_color, value); }
  void border_width(float1 value) { safe_set(&slot::border_width, value); }
  void on_button(function<void, event::button> value) { safe_set(&slot::on_button, std::move(value)); }
  void on_wheel(function<void, event::wheel> value) { safe_set(&slot::on_wheel, std::move(value)); }
};

//////////////////////////////////////// MARK: focusable_plain

class focusable_plain : public plain {
public:
  class slot : public plain::slot {
  public:
    function<void, bool> on_focus;
    function<void, event::key> on_key;
    function<void, event::move> on_move;

    virtual bool focus_event(bool focused) override {
      if (enabled && on_focus) on_focus(focused);
      return enabled;
    }

    virtual void key_event(event::key e) override {
      if (enabled && on_key) on_key(e);
    }

    virtual void move_event(event::move e) override {
      if (enabled && on_move) on_move(e);
    }
  };

  using plain::operator bool;
  focusable_plain() noexcept = default;
  focusable_plain(derived_from<unknown> auto& Layout) {
    if (auto res = create_control<focusable_plain>(Layout)) _id = *res;
  }

  const auto& on_focus() const { return unsafe_get(&slot::on_focus); }
  const auto& on_move() const { return unsafe_get(&slot::on_move); }

  void on_focus(function<void, bool> value) { safe_set(&slot::on_focus, std::move(value)); }
  void on_move(function<void, event::move> value) { safe_set(&slot::on_move, std::move(value)); }
};

//////////////////////////////////////// MARK: clickable_plain

class clickable_plain : public focusable_plain {
public:
  class slot : public focusable_plain::slot {
  public:
    function<void> on_click;
    key captured_key{};

    virtual void click_event(event::button e) override {
      if (enabled && e.code == captured_key) click_action();
      captured_key = {};
    }

    virtual void button_event(event::button e) override {
      if (enabled && e.down) captured_key = e.code;
      else captured_key = {};
      focusable_plain::slot::button_event(e);
    }

    virtual bool focus_event(bool focused) override {
      if (!focused) captured_key = {};
      return focusable_plain::slot::focus_event(focused);
    }

    virtual void key_event(event::key e) override {
      if (enabled && e.first) {
        if (e.down) captured_key = e.code;
        else if (e.code == captured_key) click_action();
        else captured_key = {};
      } else focusable_plain::slot::key_event(e);
    }

    virtual void click_action() {
      switch (captured_key) {
      case key::lbutton:
      case key::enter:
      case key::space:
        if (on_click) on_click();
      }
    }
  };

  using plain::operator bool;
  clickable_plain() noexcept = default;
  clickable_plain(derived_from<unknown> auto& Layout) {
    if (auto res = create_control<clickable_plain>(Layout)) _id = *res;
  }

  const auto& on_click() const { return unsafe_get(&slot::on_click); }
  void on_click(function<void> value) { safe_set(&slot::on_click, std::move(value)); }
};
} // namespace yw::ui
