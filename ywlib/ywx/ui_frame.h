#pragma once
#include <ywx/control.h>

namespace yw::ui {

struct color_pair {
  color background = colors::transparent;
  color border = colors::transparent;
  constexpr color_pair() noexcept = default;
  constexpr color_pair(color bg, color bd) noexcept : background(bg), border(bd) {}
  constexpr color_pair(is_none auto Auto) noexcept {
    if (std::is_constant_evaluated()) return;
    constinit static float h = static_cast<float>(220.0 / 180.0 * yw::pi);
    constexpr float dh = static_cast<float>(131.0 / 180.0 * yw::pi);
    background = hsl(h, 0.5f, 0.9f).to_srgb();
    border = hsl(h, 0.5f, 0.2f).to_srgb();
    h += dh;
  }
};

class frame : public control {
public:
  struct slot : control::slot {
    float4 padding = float4::fill(arbitrary_value);
    color_pair colors;
    float border_thickness = 1.0f;
    bool border_dashed = false;

    std::expected<void, error> draw_frame_background() {
      if (colors.background.a > 0.0f) {
        brush::color(colors.background);
        if (auto res = fill_geometry(geometry.get()); !res) return res.error().relay();
      }
      d2d::push_layer(geometry.get());
      return {};
    }

    std::expected<void, error> draw_frame_foreground() {
      d2d::pop_layer();
      if (colors.border.a > 0.0f && border_thickness > 0.0f) {
        brush::color(colors.border);
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
  };

  frame() noexcept = default;

  frame(derived_from<interface> auto& Parent, strict<bool> AutoColor = true, const source_line& sl = here()) {
    if (auto res = create(Parent, AutoColor)) *this = std::move(*res);
    else res.error().add_footprint().go_off(sl);
  }

  static std::expected<frame, error> create(derived_from<interface> auto& Parent, strict<bool> AutoColor = true) {
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
    if (AutoColor) sp->colors = color_pair(none());
    return f;
  }

  //-- getter --//

  const auto& padding() const noexcept { ywlib_control_get(padding); }
  const auto& background_color() const noexcept { ywlib_control_get(colors.background); }
  const auto& border_color() const noexcept { ywlib_control_get(colors.border); }
  const auto& border_thickness() const noexcept { ywlib_control_get(border_thickness); }
  const auto& border_dashed() const noexcept { ywlib_control_get(border_dashed); }

  //-- setter --//

  auto& padding(this auto& self, float4 f) noexcept { ywlib_control_set(padding, f, messy); }
  auto& background_color(this auto& self, const color& c) noexcept { ywlib_control_set(colors.background, c, dirty); }
  auto& border_color(this auto& self, const color& c) noexcept { ywlib_control_set(colors.border, c, dirty); }
  auto& border_thickness(this auto& self, float1 f) noexcept { ywlib_control_set(border_thickness, f.x, dirty); }
  auto& border_dashed(this auto& self, bool b) noexcept { ywlib_control_set(border_dashed, b, dirty); }
  auto& crop_content(this auto& self, bool b) noexcept { ywlib_control_set(crop_content, b, dirty); }
};
} // namespace yw::ui
