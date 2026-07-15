#pragma once
#include <ywx/ui_frame.h>

namespace yw::ui {

class progressbar : public frame {
public:
  struct slot : frame::slot {
    float value = 0.0f;
    float minimum = 0.0f;
    float maximum = 1.0f;
    float bar_width = 16.0f;
    ui::orientation orientation = ui::orientation::horizontal;
    color track_color = color(0.0f, 0.0f, 0.0f, 0.08f);
    color progress_color = color(colors::dodgerblue, 0.65f);

    float clamp_value(float v) const noexcept { return yw::clamp(v, minimum, maximum); }

    float ratio() const noexcept {
      const float range = maximum - minimum;
      if (range <= 0.0f) return 0.0f;
      return yw::clamp((value - minimum) / range, 0.0f, 1.0f);
    }

    float4 inner_rect() const noexcept {
      const auto p = pos + padding.xy();
      const auto s = size - padding.xy() - padding.zw();
      return {p.x, p.y, p.x + yw::max(0.0f, s.x), p.y + yw::max(0.0f, s.y)};
    }

    static float2 rect_pos(float4 Rect) noexcept { return {Rect.x, Rect.y}; }
    static float2 rect_size(float4 Rect) noexcept { return {Rect.z - Rect.x, Rect.w - Rect.y}; }

    float4 progress_rect(float4 Rect) const noexcept {
      const auto r = ratio();
      if (orientation == ui::orientation::vertical) {
        const float h = (Rect.w - Rect.y) * r;
        return {Rect.x, Rect.w - h, Rect.z, Rect.w};
      }
      return {Rect.x, Rect.y, Rect.x + (Rect.z - Rect.x) * r, Rect.w};
    }

    std::expected<void, error> fill_rect(float4 Rect, const color& Color) const {
      if (Color.a <= 0.0f || Rect.z <= Rect.x || Rect.w <= Rect.y) return {};
      brush::color(Color);
      if (auto res = fill_round_rectangle(rect_pos(Rect), rect_size(Rect), radius); !res) return res.error().relay();
      return {};
    }

    virtual std::expected<float2, error> get_necessary_size() const override {
      const auto base = orientation == ui::orientation::vertical ? float2{bar_width, bar_width * 3.0f}
                                                                 : float2{bar_width * 3.0f, bar_width};
      return calc_necessary_size_by_policy(padding.xy() + padding.zw() + base);
    }

    virtual std::expected<void, error> redraw() override {
      if (geometry_dirty) {
        geometry_dirty = false;
        if (auto res = relocate(); !res) return res.error().relay();
      }
      if (!visible) return {};
      if (auto res = draw_frame_background(); !res) return res.error().relay();
      const auto inner = inner_rect();
      if (auto res = fill_rect(inner, track_color); !res) return res.error().relay();
      if (auto res = fill_rect(progress_rect(inner), progress_color); !res) return res.error().relay();
      if (auto res = draw_frame_foreground(); !res) return res.error().relay();
      return {};
    }
  };

  using frame::operator bool;
  progressbar() noexcept = default;

  progressbar(derived_from<interface> auto& Parent, strict<bool> AutoColor = true, const source_line& sl = here()) {
    if (auto res = create(Parent, AutoColor)) *this = std::move(*res);
    else res.error().add_footprint().go_off(sl);
  }

  static std::expected<progressbar, error> create(derived_from<interface> auto& Parent, strict<bool> AutoColor = true) {
    progressbar p;
    const auto temp_id = make_slot<progressbar>();
    const auto sp = get_slot<progressbar>(temp_id);
    if (!sp) return std::unexpected(error(errors::slot_creation_failed));
    const auto psp = get_slot<control>(Parent.id());
    if (!psp) return std::unexpected(error(errors::invalid_slotid));
    if (auto res = psp->attach(temp_id); !res) {
      slot::slots.erase(temp_id);
      return res.error().relay();
    }
    p._id = temp_id;
    sp->id = temp_id;
    sp->window_id = psp->get_window_id();
    sp->policy = {ui::size_policy::fit, ui::size_policy::fit};
    if (AutoColor) {
      sp->colors = color_pair(none{});
      sp->progress_color = sp->colors.border;
      sp->colors.border = colors::transparent;
    }
    return p;
  }

  //-- getter --//

  float value() const noexcept { ywlib_control_get(value); }
  float minimum() const noexcept { ywlib_control_get(minimum); }
  float maximum() const noexcept { ywlib_control_get(maximum); }
  float bar_width() const noexcept { ywlib_control_get(bar_width); }
  ui::orientation orientation() const noexcept { ywlib_control_get(orientation); }
  const auto& track_color() const noexcept { ywlib_control_get(track_color); }
  const auto& progress_color() const noexcept { ywlib_control_get(progress_color); }

  //-- setter --//

  auto& value(this auto& self, float v) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->value = sp->clamp_value(v);
    sp->make_dirty();
    return self;
  }

  auto& range(this auto& self, float Min, float Max) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    if (Max < Min) {
      error(errors::invalid_argument, "progressbar range maximum must be greater than or equal to minimum").go_off();
      return self;
    }
    sp->minimum = Min;
    sp->maximum = Max;
    sp->value = sp->clamp_value(sp->value);
    sp->make_dirty();
    return self;
  }

  auto& minimum(this auto& self, float v) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    if (sp->maximum < v) {
      error(errors::invalid_argument, "progressbar minimum must be less than or equal to maximum").go_off();
      return self;
    }
    sp->minimum = v;
    sp->value = sp->clamp_value(sp->value);
    sp->make_dirty();
    return self;
  }

  auto& maximum(this auto& self, float v) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    if (v < sp->minimum) {
      error(errors::invalid_argument, "progressbar maximum must be greater than or equal to minimum").go_off();
      return self;
    }
    sp->maximum = v;
    sp->value = sp->clamp_value(sp->value);
    sp->make_dirty();
    return self;
  }

  auto& bar_width(this auto& self, float1 v) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    if (v.x <= 0.0f) {
      error(errors::invalid_argument, "progressbar bar_width must be positive").go_off();
      return self;
    }
    sp->bar_width = v.x;
    sp->make_messy();
    return self;
  }

  auto& orientation(this auto& self, ui::orientation v) noexcept { ywlib_control_set(orientation, v, messy); }
  auto& track_color(this auto& self, const color& c) noexcept { ywlib_control_set(track_color, c, dirty); }
  auto& progress_color(this auto& self, const color& c) noexcept { ywlib_control_set(progress_color, c, dirty); }
};
} // namespace yw::ui

