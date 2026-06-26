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

    virtual slotid find_next_tabstop(slotid Focused, bool Forward, bool& Found) const override {
      (void)Forward;
      if (Focused == id) Found = true;
      else if (Found && visible) return id;
      return {};
    }

    virtual std::expected<void, error> draw() const override {
      if (!visible) return {};
      if (auto res = draw_background(); !res) return res.error().relay();
      if (auto res = text.draw(text_origin()); !res) return res.error().relay();
      if (pressed && pressed_overlay_color.a > 0.0f) {
        brush::color(pressed_overlay_color);
        if (auto res = fill_geometry(geometry.get()); !res) return res.error().relay();
      }
      if (auto res = draw_foreground(); !res) return res.error().relay();
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

    virtual void button_cancel_event() override {
      if (pressed) make_dirty();
      captured_key = {};
      pressed = false;
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

    virtual bool wants_space_activate() const override { return enabled; }
    virtual bool wants_enter_activate() const override { return enabled; }

    virtual void activate_event(yw::activate_event e) override {
      if (!enabled) return;
      if (on_click) on_click(e.key);
      captured_key = {};
      if (pressed) make_dirty();
      pressed = false;
    }

    virtual void key_cancel_event() override {
      if (pressed) make_dirty();
      captured_key = {};
      pressed = false;
    }
  };

  /// MARK: handle functions

  using control::operator bool;
  button() noexcept = default;

  template<typename Layout> static std::expected<button, error> add(
    Layout& Layout_, const color_pair& Colors = color_pair::auto_color()) {
    auto res = label::add(Layout_);
    if (!res) return res.error().relay();
    button btn;
    btn._id = res->id();
    const auto csp = interface::slot::get<button>(btn._id);
    if (!csp) return std::unexpected(error(errors::invalid_slotid));
    csp->background_color = Colors.background;
    csp->border_color = Colors.foreground;
    csp->text.color(Colors.foreground);
    return btn;
  }

  const auto& pressed_overlay_color() const {
    const auto csp = interface::slot::get<button>(_id);
    if (!csp) error(errors::invalid_slotid).go_off();
    return csp->pressed_overlay_color;
  }
  std::expected<void, error> pressed_overlay_color(const color& c) {
    const auto csp = interface::slot::get<button>(_id);
    if (!csp) return std::unexpected(error(errors::invalid_slotid));
    csp->pressed_overlay_color = c;
    if (auto res = csp->make_dirty(); !res) return res.error().relay();
    return {};
  }

  const auto& on_click() const {
    const auto csp = interface::slot::get<button>(_id);
    if (!csp) error(errors::invalid_slotid).go_off();
    return csp->on_click;
  }
  std::expected<void, error> on_click(function<void, yw::key> f) {
    const auto csp = interface::slot::get<button>(_id);
    if (!csp) return std::unexpected(error(errors::invalid_slotid));
    csp->on_click = std::move(f);
    return {};
  }

  const auto& on_focus() const {
    const auto csp = interface::slot::get<button>(_id);
    if (!csp) error(errors::invalid_slotid).go_off();
    return csp->on_focus;
  }
  std::expected<void, error> on_focus(function<void, bool> f) {
    const auto csp = interface::slot::get<button>(_id);
    if (!csp) return std::unexpected(error(errors::invalid_slotid));
    csp->on_focus = std::move(f);
    return {};
  }
};
} // namespace yw::ui
