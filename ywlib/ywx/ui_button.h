#pragma once
#include <ywx/ui_label.h>

namespace yw::ui {

class button : public label {
public:
  struct slot : label::slot {
    color pressed_overlay_color = color(0.0f, 0.0f, 0.0f, 0.2f);
    color focus_ring_color = colors::dodgerblue;
    float focus_ring_thickness = 1.5f;
    bool pressed = false;
    bool focused = false;
    function<void, yw::button_event> on_click{};

    virtual bool focusable() const override { return enabled && visible; }

    virtual std::expected<void, error> redraw() override {
      if (geometry_dirty) {
        geometry_dirty = false;
        if (auto res = relocate(); !res) return res.error().relay();
      }
      if (!visible) return {};
      if (auto res = _draw_background(); !res) return res.error().relay();
      brush::color(text_color);
      if (auto res = text.draw(pos + _text_offset()); !res) return res.error().relay();
      if (pressed && pressed_overlay_color.a > 0.0f) {
        brush::color(pressed_overlay_color);
        if (auto res = fill_geometry(geometry.get()); !res) return res.error().relay();
      }
      if (focused && focus_ring_color.a > 0.0f && focus_ring_thickness > 0.0f) {
        brush::color(focus_ring_color);
        if (auto res = draw_round_rectangle(
              pos - float2::fill(focus_ring_thickness), size + float2::fill(focus_ring_thickness * 2.0f),
              radius + float2::fill(focus_ring_thickness), focus_ring_thickness);
            !res)
          return res.error().relay();
      }
      if (auto res = _draw_foreground(); !res) return res.error().relay();
      return {};
    }

    virtual void focus_event(bool Focused) override {
      if (focused == Focused) return;
      focused = Focused;
      if (!focused) pressed = false;
      if (auto res = make_dirty(); !res) res.error().go_off();
    }

    virtual bool button_event(yw::button_event e) override {
      if (!enabled || e.key != keys::lbutton) return false;
      if (e.down) {
        if (!pressed) {
          pressed = true;
          if (auto res = make_dirty(); !res) res.error().go_off();
        }
        return true;
      }
      const bool was_pressed = pressed;
      pressed = false;
      if (was_pressed) {
        if (auto res = make_dirty(); !res) res.error().go_off();
        if (hittest(float2(e.pos)) == id) return click_event(e);
      }
      return true;
    }

    virtual bool click_event(yw::button_event e) override {
      if (!enabled) return false;
      if (on_click) on_click(e);
      return true;
    }

    virtual bool key_event(yw::key_event e) override {
      if (!enabled) return false;
      if (e.key != keys::space && e.key != keys::enter) return false;
      if (e.down) {
        if (!pressed) {
          pressed = true;
          if (auto res = make_dirty(); !res) res.error().go_off();
        }
      } else {
        const bool was_pressed = pressed;
        pressed = false;
        if (was_pressed) {
          if (auto res = make_dirty(); !res) res.error().go_off();
          if (on_click) on_click({{}, e.key, e.mods, false});
        }
      }
      return true;
    }
  };

  button() noexcept = default;

  button(derived_from<interface> auto& Parent, bool AutoColor = true, const source_line& sl = here()) {
    if (auto res = slot::create<button>(Parent, AutoColor, sl)) {
      const auto sp = *res;
      _id = sp->id;
      sp->text_color = std::exchange(sp->colors.border, colors::transparent);
    } else res.error().add_footprint().go_off(sl);
  }

  static std::expected<button, error> create(
    derived_from<interface> auto& Parent, bool AutoColor = true, const source_line& sl = here()) {
    button b;
    if (auto res = slot::create<button>(Parent, AutoColor, sl)) {
      const auto sp = *res;
      b._id = sp->id;
      sp->text_color = std::exchange(sp->colors.border, colors::transparent);
      return b;
    } else return res.error().relay();
  }

  const auto& pressed_overlay_color() const noexcept { ywlib_control_get(pressed_overlay_color); }
  const auto& focus_ring_color() const noexcept { ywlib_control_get(focus_ring_color); }
  const auto& focus_ring_thickness() const noexcept { ywlib_control_get(focus_ring_thickness); }
  bool pressed() const noexcept { ywlib_control_get(pressed); }
  const auto& on_click() const noexcept { ywlib_control_get(on_click); }

  auto& pressed_overlay_color(this auto& self, const color& c) noexcept { ywlib_control_set(pressed_overlay_color, c, dirty); }
  auto& focus_ring_color(this auto& self, const color& c) noexcept { ywlib_control_set(focus_ring_color, c, dirty); }
  auto& focus_ring_thickness(this auto& self, float1 v) noexcept {
    ywlib_control_set(focus_ring_thickness, yw::max(0.0f, v.x), dirty);
  }

  auto& on_click(this auto& self, function<void, yw::button_event> f) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->on_click = std::move(f);
    return self;
  }
};
} // namespace yw::ui
