#pragma once
#include "ywx/ui_label.h"

namespace yw::ui {

class button : public label {
public:
  class slot : public label::slot {
  public:
    color pressed_overlay_color = color(0, 0, 0, 0.5f);
    key captured_key{};
    bool pressed = false;

    function<void, bool> on_focus;
    function<void, key> on_click;

    //-- overrides --//

    virtual bool focusable() const override { return true; }

    virtual slotid next_tab_stop(slotid Focused, bool Forward, bool& Found) const override {
      if (Focused == id) Found = true;
      else if (Found && visible) return id;
      return {};
    }

    virtual std::expected<void, error_trace> draw() override {
      if (!visible) return {};
      if (auto res = background.draw(core); !res) return unexpected_error(res.error());
      if (auto res = text.draw(core.pos, core.size); !res) return unexpected_error(res.error());
      if (pressed && pressed_overlay_color.a > 0.0f) {
        brush.color(pressed_overlay_color);
        if (auto res = fill_geometry(core.geometry.get()); !res) return unexpected_error(res.error());
      }
      if (auto res = border.draw(core); !res) return unexpected_error(res.error());
      return {};
    }

    virtual void click_event(events::button e) override {
      if (enabled && e.code == captured_key) click_action();
      captured_key = {};
      pressed = false;
    }

    virtual void button_event(events::button e) override {
      if (!enabled) return;
      if (e.down) {
        captured_key = e.code;
        const auto b = e.code == keys::lbutton;
        if (pressed != b) make_dirty();
        pressed = b;
      } else {
        if (captured_key == e.code) click_action();
        if (pressed) make_dirty();
        captured_key = {};
        pressed = false;
      }
    }

    virtual void focus_event(bool focused) override {
      make_dirty();
      if (!focused) {
        captured_key = {};
        pressed = false;
      }
      if (enabled && on_focus) on_focus(focused);
    }

    virtual bool key_event(events::key e) override {
      if (!enabled) return false;
      if (e.down) {
        captured_key = e.code;
        const auto b = e.code == keys::enter || e.code == keys::space;
        if (pressed != b) make_dirty();
        pressed = b;
      } else {
        if (captured_key == e.code) click_action();
        if (pressed) make_dirty();
        captured_key = {};
        pressed = false;
      }
      return e.code == keys::space || e.code == keys::enter;
    }

    virtual void click_action() {
      switch (captured_key.code) {
      case keys::lbutton.code:
      case keys::enter.code:
      case keys::space.code:
        if (on_click) on_click(captured_key);
      }
    }
  };

  using control::operator bool;
  button() noexcept = default;

  static std::expected<button, error_trace> add(derived_from<unknown> auto& Layout) {
    button btn;
    if (auto res = create_control<button>(Layout)) btn._id = *res;
    else return unexpected_error(res.error());
    if (const auto csp = system::slot_address<button>(btn._id)) {
      const auto [bg_color, border_color] = control::get_auto_color();
      csp->background.control_id = btn._id;
      csp->background.color = bg_color;
      csp->border.control_id = btn._id;
      csp->border.color = border_color;
      csp->text.control_id = btn._id;
      csp->text.color = border_color;
    } else return unexpected_error(errors::ui_invalid_slotid);
    return btn;
  }

  const auto& pressed_overlay_color() const { return unsafe_get(&button::slot::pressed_overlay_color); }
  void pressed_overlay_color(const color& c) { unsafe_set(&button::slot::pressed_overlay_color, c); }

  const auto& on_click() const { return unsafe_get(&button::slot::on_click); }
  void on_click(function<void, key> f) { unsafe_set(&button::slot::on_click, std::move(f)); }

  const auto& on_focus() const { return unsafe_get(&button::slot::on_focus); }
  void on_focus(function<void, bool> f) { unsafe_set(&button::slot::on_focus, std::move(f)); }
};
} // namespace yw::ui
