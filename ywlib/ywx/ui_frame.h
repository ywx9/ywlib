#pragma once
#include <ywx/control.h>

namespace yw::ui {

class frame : public control {
public:
  struct slot : control::slot {
    float4 padding = float4::fill(arbitrary_value);
    color background_color = colors::transparent;
    color border_color = colors::transparent;
    color hovered_overlay_color = colors::transparent;
    float border_thickness = 1.0f;
    bool border_dashed = false;

    std::expected<void, error> draw_frame_background() {
      if (background_color.a > 0.0f) {
        brush::color(background_color);
        if (auto res = fill_geometry(geometry.get()); !res) return res.error().relay();
      }
      if (auto res = draw_hovered_overlay(); !res) return res.error().relay();
      d2d::push_layer(geometry.get());
      return {};
    }

    /// \note en) This is defined to be overloaded when drawing each part, such as a scrollbar.
    virtual std::expected<void, error> draw_hovered_overlay() {
      if (!hovered() || hovered_overlay_color.a <= 0.0f) return {};
      brush::color(hovered_overlay_color);
      if (auto res = fill_geometry(geometry.get()); !res) return res.error().relay();
      return {};
    }

    std::expected<void, error> draw_frame_foreground() {
      d2d::pop_layer();
      if (border_color.a > 0.0f && border_thickness > 0.0f) {
        brush::color(border_color);
        brush::dashed(border_dashed);
        if (auto res = draw_geometry(geometry.get(), border_thickness); !res) return res.error().relay();
        brush::dashed(false);
      }
      return {};
    }

    //-- override functions --//

    virtual std::expected<float2, error> get_necessary_size() const override {
      const auto inner = padding.xy() + padding.zw();
      return calc_necessary_size_by_policy(inner);
    }

    virtual std::expected<void, error> redraw() override {
      if (geometry_dirty) {
        geometry_dirty = false;
        if (auto res = relocate(); !res) return res.error().relay();
      }
      if (!visible) return {};
      if (auto res = draw_frame_background(); !res) return res.error().relay();
      if (auto res = draw_frame_foreground(); !res) return res.error().relay();
      return {};
    }

    virtual std::expected<void, error> apply_color_theme(const yw::ui::color_theme& Theme, bool Recursive) override {
      background_color = Theme.surface;
      border_color = Theme.outline;
      hovered_overlay_color = color(Theme.accent, default_overlay_opacity.hover);
      make_dirty();
      return {};
    }
  };

  frame() noexcept = default;

  frame(derived_from<interface> auto& Parent, const source_line& sl = here()) {
    if (auto res = create(Parent)) *this = std::move(*res);
    else res.error().add_footprint().go_off(sl);
  }

  static std::expected<frame, error> create(derived_from<interface> auto& Parent) {
    frame f;
    const auto temp_id = make_slot<frame>();
    const auto sp = get_slot<frame>(temp_id);
    if (!sp) return std::unexpected(error(errors::slot_creation_failed));
    const auto psp = get_slot<control>(Parent.id());
    if (!psp) return std::unexpected(error(errors::invalid_slotid));
    if (auto res = psp->attach(temp_id); !res) {
      slot::slots.erase(temp_id);
      return res.error().relay();
    }
    f._id = temp_id;
    sp->id = temp_id;
    sp->window_id = psp->get_window_id();
    if (auto res = sp->apply_current_color_theme(false); !res) return res.error().relay();
    return f;
  }

  //-- getter --//

  const auto& padding() const noexcept { ywlib_control_get(padding); }
  const auto& background_color() const noexcept { ywlib_control_get(background_color); }
  const auto& border_color() const noexcept { ywlib_control_get(border_color); }
  const auto& hovered_overlay_color() const noexcept { ywlib_control_get(hovered_overlay_color); }
  const auto& border_thickness() const noexcept { ywlib_control_get(border_thickness); }
  const auto& border_dashed() const noexcept { ywlib_control_get(border_dashed); }

  //-- setter --//

  auto& padding(this auto& self, float4 f) noexcept { ywlib_control_set(padding, f, messy); }
  auto& background_color(this auto& self, const color& c) noexcept { ywlib_control_set(background_color, c, dirty); }
  auto& border_color(this auto& self, const color& c) noexcept { ywlib_control_set(border_color, c, dirty); }
  auto& hovered_overlay_color(this auto& self, const color& c) noexcept {
    ywlib_control_set(hovered_overlay_color, c, dirty);
  }
  auto& border_thickness(this auto& self, float1 f) noexcept { ywlib_control_set(border_thickness, f.x, dirty); }
  auto& border_dashed(this auto& self, bool b) noexcept { ywlib_control_set(border_dashed, b, dirty); }
};
} // namespace yw::ui
