#pragma once
#include <apps/ui_label.h>

namespace yw::ui {

class button : public label {
public:
protected:
  int _pressed_keys = 0;

  virtual result<bool> _handle_button_event(window& win, yw::button_event e) override {
    if (!_visible || !_enabled) {
      _clear_state(win);
      return false;
    }
    if (e.key == keys::lbutton) pressed = e.state.down;
    if (auto res = label::_handle_button_event(win, e)) return *res;
    else return res.relay();
  }

  virtual result<bool> _handle_click_event(window& win, yw::button_event e) override {
    if (!_visible || !_enabled) {
      _clear_state(win);
      return false;
    }
    if (click_event.ref()) return click_event.ref()(e);
    if (auto res = label::_handle_click_event(win, e)) return *res;
    else return res.relay();
  }

  virtual result<bool> _handle_focus_event(window& win, yw::focus_event e) override {
    if (!_visible || !_enabled) {
      _clear_state(win);
      return false;
    }
    if (!e.focused) _reset_button_state();
    if (auto res = label::_handle_focus_event(win, e)) return *res;
    else return res.relay();
  }

  virtual result<bool> _handle_key_event(window& win, yw::key_event e) override {
    if (!_visible || !_enabled) {
      _clear_state(win);
      return false;
    }
    if (e.key == keys::space) {
      if (e.state.down) _pressed_keys |= 1;
      else _pressed_keys &= ~1;
      pressed = _pressed_keys != 0;
    } else if (e.key == keys::enter) {
      if (e.state.down) _pressed_keys |= 2;
      else _pressed_keys &= ~2;
      pressed = _pressed_keys != 0;
    }
    if (auto res = label::_handle_key_event(win, e)) return *res;
    else return res.relay();
  }

  virtual void _clear_state(window& win) noexcept override {
    label::_clear_state(win);
    _reset_button_state();
  }

  void _reset_button_state() noexcept {
    _pressed_keys = 0;
    pressed = false;
  }

public:
  property<function<bool, yw::button_event>, button> click_event;
  property<function<bool, yw::key_event>, button> key_event;
  const_property<bool, button> pressed;

  virtual bool focusable() const override { return true; }
  virtual bool interactive() const override { return true; }
};
} // namespace yw::ui
