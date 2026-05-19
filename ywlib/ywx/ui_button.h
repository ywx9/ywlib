#pragma once
#include "ywx/ui_label.h"

namespace yw::ui {

class button : public label {
public:
  class slot : public label::slot {
  public:
    color pressed_overlay_color = color(0, 0, 0, 0.5f);

    function<void, bool> on_focus;
    function<void, key> on_click;

    key captured_key{};
    bool is_pushed() const {
      return captured_key == keys::lbutton || captured_key == keys::enter || captured_key == keys::space;
    }

    //-- overrides --//

    virtual bool focusable() const override { return true; }

    virtual std::expected<void, error_trace> draw() override {
      if (!visible) return {};
      brush.color(background.color);
      fill_geometry(core.geometry.get());
      d2d.push_layer(core.geometry.get());
      if (background.image) draw_bitmap(core.pos, core.size, background.image, background.image_opacity);
      text.draw(core.pos, core.size);
      brush.color(pressed_overlay_color);
      fill_geometry(core.geometry.get());
      d2d.pop_layer();
      border.draw(core.geometry.get());
      return {};
    }

    virtual void click_event(events::button e) override {
      if (enabled && e.code == captured_key) click_action();
      captured_key = {};
    }

    virtual void button_event(events::button e) override {
      if (enabled && e.down) captured_key = e.code;
      else captured_key = {};
    }

    virtual void focus_event(bool focused) override {
      if (!focused) captured_key = {};
      if (enabled && on_focus) on_focus(focused);
    }

    virtual bool key_event(events::key e) override {
      if (enabled && e.down) captured_key = e.code;
      else captured_key = {};
      return e.code == keys::space || e.code == keys::enter; // spaceとenter以外はwindowでの処理を許す
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
  button(derived_from<unknown> auto& Layout) {
    if (auto res = create_control<button>(Layout)) _id = *res;
  }

  const auto& pressed_overlay_color() const { return unsafe_get(&button::slot::pressed_overlay_color); }
  void pressed_overlay_color(const color& c) { unsafe_set(&button::slot::pressed_overlay_color, c); }

  const auto& on_click() const { return unsafe_get(&button::slot::on_click); }
  void on_click(function<void, key> f) { unsafe_set(&button::slot::on_click, std::move(f)); }

  const auto& on_focus() const { return unsafe_get(&button::slot::on_focus); }
  void on_focus(function<void, bool> f) { unsafe_set(&button::slot::on_focus, std::move(f)); }
};
} // namespace yw::ui
