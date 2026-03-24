#pragma once
#include "ywx/tooltip.h"
#include "ywx/ui_window.h"

namespace yw::ui {

class plain : public control {
public:
  class slot : public control::slot {
  protected:
    void draw_plain(float2 Pos, float2 Size) const {
      brush.color(bg_color);
      fill_round_rectangle(Pos, Size, radius);
      brush.color(border_color);
      draw_round_rectangle(Pos, Size, radius, border_width);
    }

  public:
    float2 radius{5.0f, 5.0f};
    color bg_color = colors::white;
    color border_color = colors::transparent;
    float1 border_width = 1.0f;
    std::wstring tooltip{};

    function<void, event::button> on_button;
    function<void, event::hover> on_hover;
    function<void, event::wheel> on_wheel;

    virtual slotid hit_test(float2 Pt) const noexcept override {
      if (!visible || Pt.x < last_rect.x || Pt.y < last_rect.y || Pt.x > last_rect.z || Pt.y > last_rect.w) return {};
      return id;
    }

    virtual void draw(float2 Pos, float2 Size) const override {
      update_last_rect(Pos, Size);
      draw_plain(last_rect.xy(), last_rect.zw() - last_rect.xy());
    }

    virtual void draw() const override { draw_plain(last_rect.xy(), last_rect.zw() - last_rect.xy()); }
    virtual float2 get_radius() const noexcept override { return radius; } // for drawing focus ring

    virtual void button_event(event::button Event) override {
      if (enabled && on_button) on_button(Event);
    }

    virtual void hover_event(event::hover Event) override {
      if (enabled && on_hover) on_hover(Event);
      if (tooltip.empty()) return;
      if (Event.move()) {
        if (const auto w = system::slot_address<ui::window>(window_id))
          system::tooltip.show(last_rect.xy() + w->pos + w->margin.xy(), last_rect.zw() - last_rect.xy());
      } else if (Event.enter()) {
        if (const auto w = system::slot_address<ui::window>(window_id))
          system::tooltip.show(last_rect.xy() + w->pos() + w->margin.xy(), last_rect.zw() - last_rect.xy(), tooltip);
      } else if (Event.leave()) system::tooltip.hide();
    }

    virtual void wheel_event(event::wheel Event) override {
      if (enabled && on_wheel) on_wheel(Event);
    }
  };

  plain() noexcept = default;

  plain(derived_from<unknown> auto& Layout) {
    if (auto res = create_control<plain>(Layout)) _id = *res;
  }

  using control::operator bool;

  const float2& radius() const { return unsafe_get(&slot::radius); }
  void radius(float2 value) { safe_set(&slot::radius, value); }

  const color& bg_color() const { return unsafe_get(&slot::bg_color); }
  void bg_color(const color& value) { safe_set(&slot::bg_color, value); }

  const color& border_color() const { return unsafe_get(&slot::border_color); }
  void border_color(const color& value) { safe_set(&slot::border_color, value); }

  const float1& border_width() const { return unsafe_get(&slot::border_width); }
  void border_width(float1 value) { safe_set(&slot::border_width, value); }

  const auto& tooltip() const { return unsafe_get(&slot::tooltip); }
  void tooltip(std::wstring_view value) { safe_set(&slot::tooltip, value); }

  const auto& on_button() const { return unsafe_get(&slot::on_button); }
  void on_button(function<void, event::button> value) { safe_set(&slot::on_button, std::move(value)); }

  const auto& on_hover() const { return unsafe_get(&slot::on_hover); }
  void on_hover(function<void, event::hover> value) { safe_set(&slot::on_hover, std::move(value)); }

  const auto& on_wheel() const { return unsafe_get(&slot::on_wheel); }
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

  const auto& on_focus() const { return unsafe_get(&slot::on_focus); }
  void on_focus(function<void, bool> value) { safe_set(&slot::on_focus, std::move(value)); }

  const auto& on_move() const { return unsafe_get(&slot::on_move); }
  void on_move(function<void, event::move> value) { safe_set(&slot::on_move, std::move(value)); }

  using plain::operator bool;
  focusable_plain() noexcept = default;
  focusable_plain(derived_from<unknown> auto& Layout) {
    if (auto res = create_control<focusable_plain>(Layout)) _id = *res;
  }
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

  const auto& on_click() const { return unsafe_get(&slot::on_click); }
  void on_click(function<void> value) { safe_set(&slot::on_click, std::move(value)); }

  using plain::operator bool;
  clickable_plain() noexcept = default;
  clickable_plain(derived_from<unknown> auto& Layout) {
    if (auto res = create_control<clickable_plain>(Layout)) _id = *res;
  }
};
} // namespace yw::ui
