#pragma once
#include "ywx/ui_label.h"

namespace yw::ui {

class button : public label {
public:
  /// MARK: slot

  class slot : public label::slot {
  public:
    color pressed_overlay_color = color(0, 0, 0, 0.5f);
    key captured_key{};
    bool pressed = false;

    function<void, bool> on_focus;
    function<void, key> on_click;

    virtual bool focusable() const override { return true; }

    virtual unknown_slotid next_tab_stop(unknown_slotid Focused, bool Forward, bool& Found) const override {
      if (Focused == id) Found = true;
      else if (Found && visible) return id;
      return {};
    }

    virtual std::expected<void, error_trace> draw() const override {
      if (!visible) return {};
      if (auto res = draw_background(); !res) return unexpected_error(res.error());
      const auto tx_origin = calculate_content_origin(text.bounds(), padding, text_alignment);
      if (auto res = text.draw(tx_origin); !res) return unexpected_error(res.error());
      if (pressed && pressed_overlay_color.a > 0.0f) {
        brush().color(pressed_overlay_color);
        if (auto res = fill_geometry(geometry.get()); !res) return unexpected_error(res.error());
      }
      if (auto res = draw_foreground(); !res) return unexpected_error(res.error());
      return {};
    }

    virtual void click_event(yw::button_event e) override {
      if (!enabled) return;
      const auto key = e.key;
      if (key == captured_key && key == keys::lbutton)
        if (on_click) on_click(key);
      captured_key = {};
      pressed = false;
    }

    virtual void button_event(yw::button_event e) override {
      if (!enabled) return;
      const auto key = e.key;
      if (e.down) {
        captured_key = key;
        const auto b = key == keys::lbutton;
        if (pressed != b) make_dirty();
        pressed = b;
      } else {
        if (key == captured_key && key == keys::lbutton)
          if (on_click) on_click(key);
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

    virtual bool key_event(yw::key_event e) override {
      if (!enabled) return false;
      const auto key = e.key;
      if (e.down) {
        captured_key = key;
        const auto b = key == keys::enter || key == keys::space;
        if (pressed != b) make_dirty();
        pressed = b;
      } else {
        if (key == captured_key && (key == keys::enter || key == keys::space))
          if (on_click) on_click(captured_key);
        if (pressed) make_dirty();
        captured_key = {};
        pressed = false;
      }
      return key == keys::space || key == keys::enter;
    }
  };

  /// MARK: handle functions

  using control::operator bool;
  button() noexcept = default;

  static std::expected<button, error_trace> add(
    derived_from<unknown> auto& Layout, const color_pair& Colors = color_pair::auto_color()) {
    button btn;
    if (auto res = create_control<button>(Layout)) btn._id = *res;
    else return unexpected_error(res.error());
    if (const auto csp = system::get_slot_pointer<button>(btn._id)) {
      csp->background_color = Colors.background;
      csp->border_color = Colors.foreground;
      csp->text.color(Colors.foreground);
    } else return unexpected_error(errors::invalid_slotid);
    return btn;
  }

  const auto& pressed_overlay_color() const { return unsafe_get(&button::slot::pressed_overlay_color); }
  std::expected<void, error_trace> pressed_overlay_color(const color& c) {
    if (auto res = safe_set(&button::slot::pressed_overlay_color, c)) return {};
    else return unexpected_error(res.error());
  }

  const auto& on_click() const { return unsafe_get(&button::slot::on_click); }
  std::expected<void, error_trace> on_click(function<void, yw::key> f) {
    if (auto res = safe_set(&button::slot::on_click, std::move(f))) return {};
    else return unexpected_error(res.error());
  }
  const auto& on_focus() const { return unsafe_get(&button::slot::on_focus); }
  std::expected<void, error_trace> on_focus(function<void, bool> f) {
    if (auto res = safe_set(&button::slot::on_focus, std::move(f))) return {};
    else return unexpected_error(res.error());
  }
};
} // namespace yw::ui
