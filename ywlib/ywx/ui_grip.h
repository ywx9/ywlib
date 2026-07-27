#pragma once
#include <ywx/drawing.h>
// #include <ywx/ui_frame.h>

namespace yw::ui {

// class grip : public frame {
// public:
//   struct slot : frame::slot {
//     color pressed_overlay_color = color(colors::black, 0.15f);
//     bool pressed = false;
//
//     function<void, yw::drag_event> on_drag{};
//
//     virtual bool focusable() const noexcept override { return enabled && visible; }
//
//     virtual std::expected<void, error> redraw() override {
//       if (geometry_dirty) {
//         geometry_dirty = false;
//         if (auto res = relocate(); !res) return res.error().relay();
//       }
//       if (!visible) return {};
//       if (auto res = draw_frame_background(); !res) return res.error().relay();
//       if (pressed && pressed_overlay_color.a > 0.0f) {
//         brush::color(pressed_overlay_color);
//         if (auto res = fill_geometry(geometry.get()); !res) return res.error().relay();
//       }
//       if (auto res = draw_frame_foreground(); !res) return res.error().relay();
//       return {};
//     }
//
//     virtual bool button_event(yw::button_event e) override {
//       if (!enabled || !visible || e.key != keys::lbutton) return false;
//       const bool next_pressed = e.down;
//       if (pressed != next_pressed) {
//         pressed = next_pressed;
//         make_dirty();
//       }
//       return true;
//     }
//
//     virtual bool drag_event(yw::drag_event e) override {
//       if (!enabled || !visible || !pressed) return false;
//       if (on_drag) on_drag(e);
//       return true;
//     }
//
//     virtual std::expected<void, error> reset_state() override {
//       if (!pressed) return {};
//       pressed = false;
//       make_dirty();
//       return {};
//     }
//
//     virtual void focus_event(bool Focused) override {
//       if (!Focused && pressed) {
//         pressed = false;
//         make_dirty();
//       }
//       frame::slot::focus_event(Focused);
//     }
//
//     virtual std::expected<void, error> apply_color_theme(const yw::ui::color_theme& Theme, bool Recursive) override {
//       background_color = color(Theme.outline, 0.20f);
//       border_color = colors::transparent;
//       hovered_overlay_color = color(Theme.accent, default_overlay_opacity.hover);
//       pressed_overlay_color = color(Theme.accent, default_overlay_opacity.pressed);
//       make_dirty();
//       return {};
//     }
//   };
//
//   using frame::operator bool;
//   grip() noexcept = default;
//
//   grip(derived_from<interface> auto& Parent, const source_line& sl = here()) {
//     if (auto res = create(Parent)) *this = std::move(*res);
//     else res.error().add_footprint().go_off(sl);
//   }
//
//   static std::expected<grip, error> create(derived_from<interface> auto& Parent) {
//     grip g;
//     const auto temp_id = make_slot<grip>();
//     const auto sp = get_slot<grip>(temp_id);
//     if (!sp) return std::unexpected(error(errors::slot_creation_failed));
//     const auto psp = get_slot<control>(Parent.id());
//     if (!psp) return std::unexpected(error(errors::invalid_slotid));
//     if (auto res = psp->attach(temp_id); !res) {
//       slot::slots.erase(temp_id);
//       return res.error().relay();
//     }
//     g._id = temp_id;
//     sp->id = temp_id;
//     sp->window_id = psp->get_window_id();
//     sp->margin = {};
//     sp->padding = {};
//     sp->radius = {};
//     sp->policy = vector2<size_policy>::fill(size_policy::fixed);
//     sp->required_size = {arbitrary_value * 8.0f, arbitrary_value * 2.0f};
//     if (auto res = sp->apply_current_color_theme(false); !res) return res.error().relay();
//     return g;
//   }
//
//   //-- getter --//
//
//   bool pressed() const noexcept { ywlib_control_get(pressed); }
//   const auto& pressed_overlay_color() const noexcept { ywlib_control_get(pressed_overlay_color); }
//   const auto& on_drag() const noexcept { ywlib_control_get(on_drag); }
//
//   //-- setter --//
//
//   auto& pressed_overlay_color(this auto& self, const color& c) noexcept {
//     ywlib_control_set(pressed_overlay_color, c, dirty);
//   }
//
//   auto& on_drag(this auto& self, function<void, yw::drag_event> f) noexcept {
//     const auto sp = get_slot(&self);
//     if (!sp) error(errors::invalid_slotid).fizzle_out();
//     sp->on_drag = std::move(f);
//     return self;
//   }
// };
} // namespace yw::ui
