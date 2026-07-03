#pragma once
#include <ywx/control.h>

namespace yw::ui {

struct color_pair {
  color background;
  color border;
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
    bitmap bg_image;
    float bg_image_opacity = 1.0f;
    float border_thickness = 1.0f;
    bool border_dashed = false;
    bool crop_content = true;

    std::expected<void, error> _draw_background() {
      if (colors.background.a > 0.0f) {
        brush::color(colors.background);
        if (auto res = fill_geometry(geometry.get()); !res) return res.error().relay();
      }
      if (crop_content) d2d::push_layer(geometry.get());
      if (bg_image && bg_image_opacity > 0.0f)
        if (auto res = draw_bitmap(pos, size, bg_image, bg_image_opacity); !res) return res.error().relay();
      return {};
    }

    std::expected<void, error> _draw_foreground() {
      if (crop_content) d2d::pop_layer();
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
      return vapply_r<float2>(_necessary_size, policy, minimum_size, required_size, inner);
    }

    virtual std::expected<void, error> redraw() override {
      if (geometry_dirty) {
        geometry_dirty = false;
        if (auto res = relocate(); !res) return res.error().relay();
      }
      if (!visible) return {};
      if (auto res = _draw_background(); !res) return res.error().relay();
      if (auto res = _draw_foreground(); !res) return res.error().relay();
      return {};
    }

    //-- create function --//

    template<derived_from<frame> H, derived_from<interface> L>
    static std::expected<typename H::slot*, error> create(L& Layout, bool AutoColor, const source_line& sl) {
      const auto lsp = slot::get<interface>(Layout.id());
      if (!lsp) return std::unexpected(error(errors::invalid_slotid));
      if (!lsp->attachable()) return std::unexpected(error(errors::invalid_operation, "not attachable"));
      const auto temp_id = slot::add<H>();
      const auto sp = slot::get<H>(temp_id);
      if (!sp) return std::unexpected(error(errors::slot_creation_failed));
      sp->id = temp_id;
      sp->source = sl;
      if (AutoColor) sp->colors = color_pair(none());
      if (auto res = lsp->attach(temp_id); !res) {
        slot::slots.erase(temp_id);
        return res.error().relay();
      }
      return sp;
    }
  };

  frame() noexcept = default;

  frame(derived_from<interface> auto& Parent, bool AutoColor = true, const source_line& sl = here()) {
    if (auto res = slot::create<frame>(Parent, AutoColor, sl)) _id = (*res)->id;
    else res.error().add_footprint().go_off(sl);
  }

  template<typename... Ts> requires constructible<frame, Ts...>
  static std::expected<frame, error> create(Ts&&... Args) {
    if (auto res = slot::create<frame>(static_cast<Ts&&>(Args)...)) {
      frame f;
      f._id = (*res)->id;
      return f;
    } else return res.error().relay();
  }

  //-- getter --//

  const auto& padding() const noexcept { ywlib_control_get(padding); }
  const auto& background_color() const noexcept { ywlib_control_get(colors.background); }
  const auto& background_image() const noexcept { ywlib_control_get(bg_image); }
  const auto& background_image_opacity() const noexcept { ywlib_control_get(bg_image_opacity); }
  const auto& border_color() const noexcept { ywlib_control_get(colors.border); }
  const auto& border_thickness() const noexcept { ywlib_control_get(border_thickness); }
  const auto& border_dashed() const noexcept { ywlib_control_get(border_dashed); }
  const auto& crop_content() const noexcept { ywlib_control_get(crop_content); }

  //-- setter --//

  auto& padding(this auto& self, float4 f) noexcept { ywlib_control_set(padding, f, messy); }
  auto& background_color(this auto& self, const color& c) noexcept { ywlib_control_set(colors.background, c, dirty); }
  auto& background_image(this auto& self, bitmap b) noexcept { ywlib_control_set(bg_image, std::move(b), dirty); }
  auto& background_image_opacity(this auto& self, float1 f) noexcept {
    ywlib_control_set(bg_image_opacity, f.x, dirty);
  }
  auto& border_color(this auto& self, const color& c) noexcept { ywlib_control_set(colors.border, c, dirty); }
  auto& border_thickness(this auto& self, float1 f) noexcept { ywlib_control_set(border_thickness, f.x, dirty); }
  auto& border_dashed(this auto& self, bool b) noexcept { ywlib_control_set(border_dashed, b, dirty); }
  auto& crop_content(this auto& self, bool b) noexcept { ywlib_control_set(crop_content, b, dirty); }
};
} // namespace yw::ui
