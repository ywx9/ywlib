#pragma once
#include <ywx/ui_label.h>

namespace yw::ui {

class button : public label {
public:
  struct slot : label::slot {
    color pressed_overlay_color = color(0.0f, 0.0f, 0.0f, 0.2f);
    bool pressed = false;

    function<void, yw::button_event> on_click{};

    virtual bool focusable() const noexcept override { return enabled && visible; }

    virtual std::expected<void, error> redraw() override {
      if (geometry_dirty) {
        geometry_dirty = false;
        if (auto res = relocate(); !res) return res.error().relay();
      }
      if (!visible) return {};
      if (auto res = draw_frame_background(); !res) return res.error().relay();
      brush::color(text_color);
      if (auto res = text.draw(pos + calc_text_offset()); !res) return res.error().relay();
      if (pressed && pressed_overlay_color.a > 0.0f) {
        brush::color(pressed_overlay_color);
        if (auto res = fill_geometry(geometry.get()); !res) return res.error().relay();
      }
      if (auto res = draw_frame_foreground(); !res) return res.error().relay();
      return {};
    }

    virtual std::expected<void, error> reset_state() override {
      if (!pressed) return {};
      pressed = false;
      make_dirty();
      return {};
    }

    virtual bool button_event(yw::button_event e) override {
      if (!enabled || e.key != keys::lbutton) return false;
      const bool next_pressed = e.down;
      if (pressed == next_pressed) return true;
      pressed = next_pressed;
      make_dirty();
      return true;
    }

    virtual bool click_event(yw::button_event e) override {
      if (!enabled || e.down || e.key != keys::lbutton) return false;
      if (on_click) on_click(e);
      return true;
    }

    virtual void focus_event(bool Focused) override {
      if (!Focused) pressed = false;
    }

    virtual bool key_event(yw::key_event e) override {
      if (!enabled) return false;
      if (e.key != keys::space && e.key != keys::enter) return false;
      if (e.down) {
        if (!pressed) {
          pressed = true;
          make_dirty();
        }
        return true;
      }
      const bool was_pressed = pressed;
      pressed = false;
      if (was_pressed) {
        make_dirty();
        if (on_click) on_click({{}, e.key, e.mods, false});
      }
      return true;
    }
  };

  button() noexcept = default;

  button(derived_from<interface> auto& Parent, strict<bool> AutoColor = true, const source_line& sl = here()) {
    if (auto res = create(Parent, AutoColor)) *this = std::move(*res);
    else res.error().add_footprint().go_off(sl);
  }

  static std::expected<button, error> create(derived_from<interface> auto& Parent, strict<bool> AutoColor = true) {
    button b;
    const auto temp_id = make_slot<button>();
    const auto sp = get_slot<button>(temp_id);
    if (!sp) return std::unexpected(error(errors::slot_creation_failed));
    const auto psp = get_slot<control>(Parent.id());
    if (!psp) return std::unexpected(error(errors::invalid_slotid));
    if (auto res = psp->attach(temp_id); !res) {
      slot::slots.erase(temp_id);
      return res.error().relay();
    }
    b._id = temp_id;
    sp->id = temp_id;
    sp->window_id = psp->get_window_id();
    sp->policy = {ui::size_policy::fit, ui::size_policy::fit};
    if (AutoColor) {
      sp->colors = color_pair(none());
      sp->text_color = sp->colors.border;
    }
    return b;
  }

  //-- getter --//

  const auto& pressed_overlay_color() const noexcept { ywlib_control_get(pressed_overlay_color); }
  bool pressed() const noexcept { ywlib_control_get(pressed); }
  const auto& on_click() const noexcept { ywlib_control_get(on_click); }

  //-- setter --//

  auto& pressed_overlay_color(this auto& self, const color& c) noexcept {
    ywlib_control_set(pressed_overlay_color, c, dirty);
  }

  auto& on_click(this auto& self, function<void, yw::button_event> f) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->on_click = std::move(f);
    return self;
  }
};
} // namespace yw::ui
