#pragma once
#include <ywx/control.h>

namespace yw::ui {

/** \note progressbarについて
- paddingは無効
- bar_width : 3*bar_widthが最小サイズ
- cross方向にfree
*/

class progressbar : public control {
public:
  struct slot : control::slot {
    double value = 0.0;
    double minimum = 0.0;
    double maximum = 1.0;
    float bar_width = 16.0f;
    ui::orientation orientation = ui::horizontal;
    color progress_color;

    //-- override functions --//

    virtual std::expected<void, error> apply_color_theme(const ui::color_theme& Theme, bool Recursive) override {
      background_color = Theme.surface;
      border_color = Theme.outline;
      progress_color = Theme.accent;
      make_dirty();
      return {};
    }

    virtual std::expected<void, error> draw_backcontent() override {
      const auto ratio = float(this->ratio());
      if (ratio == 0.0f) return {};
      brush::color(progress_color);
      if (ratio == 1.0f) {
        if (auto res = fill_geometry(geometry.get()); !res) return res.error().relay();
      } else if (orientation == ui::horizontal) {
        const auto bar_size = float2(size.x * ratio, size.y);
        if (auto res = fill_rectangle(pos, bar_size); !res) return res.error().relay();
      } else {
        const auto bar_size = float2(size.x, size.y * ratio);
        const auto bar_origin = float2(pos.x, pos.y + size.y - bar_size.y);
        if (auto res = fill_rectangle(bar_origin, bar_size); !res) return res.error().relay();
      }
      return {};
    }

    virtual float2 get_minimum_size() const override {
      if (orientation == ui::vertical) return float2{bar_width, bar_width * 3.0f};
      else return float2{bar_width * 3.0f, bar_width};
    }

    //-- shared functions --//

    double clamp_value(double v) noexcept { return yw::clamp(v, minimum, maximum); }

    double ratio() const noexcept {
      if (const double range = maximum - minimum; range <= 0.0) return 0.0;
      else return yw::clamp((value - minimum) / range, 0.0, 1.0);
    }
  };

  using control::operator bool;
  progressbar() noexcept = default;

  progressbar(derived_from<interface> auto& Parent, const source_line& sl = here()) {
    if (auto res = create(Parent)) *this = std::move(*res);
    else res.error().add_footprint().go_off(sl);
  }

  static std::expected<progressbar, error> create(derived_from<interface> auto& Parent) {
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
    sp->policy[sp->orientation == ui::horizontal] = ui::size_policy::fit;
    if (auto theme = sp->get_color_theme(); !theme) return theme.error().relay();
    else if (auto res = sp->apply_color_theme(*(*theme), false); !res) return res.error().relay();
    return p;
  }

  //-- getter --//

  double value() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return {};
    }
    return sp->value;
  }

  double minimum() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return {};
    }
    return sp->minimum;
  }

  double maximum() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return {};
    }
    return sp->maximum;
  }

  float bar_width() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return {};
    }
    return sp->bar_width;
  }

  ui::orientation orientation() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return {};
    }
    return sp->orientation;
  }

  const auto& progress_color() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->progress_color;
  }

  //-- setter --//

  auto& value(this auto& self, double1 v) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    sp->value = sp->clamp_value(v.x);
    sp->make_dirty();
    return self;
  }

  auto& range(this auto& self, double1 Min, double1 Max) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    if (Max.x < Min.x) {
      error(errors::invalid_argument, "progressbar range maximum must be greater than or equal to minimum").fizzle_out();
      return self;
    }
    sp->minimum = Min.x;
    sp->maximum = Max.x;
    sp->value = sp->clamp_value(sp->value);
    sp->make_dirty();
    return self;
  }

  auto& minimum(this auto& self, double1 v) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    if (sp->maximum < v.x) {
      error(errors::invalid_argument, "progressbar minimum must be less than or equal to maximum").fizzle_out();
      return self;
    }
    sp->minimum = v.x;
    sp->value = sp->clamp_value(sp->value);
    sp->make_dirty();
    return self;
  }

  auto& maximum(this auto& self, double1 v) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    if (v.x < sp->minimum) {
      error(errors::invalid_argument, "progressbar maximum must be greater than or equal to minimum").fizzle_out();
      return self;
    }
    sp->maximum = v.x;
    sp->value = sp->clamp_value(sp->value);
    sp->make_dirty();
    return self;
  }

  auto& bar_width(this auto& self, float1 v) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    if (v.x <= 0.0f) {
      error(errors::invalid_argument, "progressbar bar_width must be positive").fizzle_out();
      return self;
    }
    sp->bar_width = v.x;
    sp->make_messy();
    return self;
  }

  auto& orientation(this auto& self, ui::orientation v) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    if (sp->orientation != v) {
      sp->orientation = v;
      sp->swap_dimensions();
    }
    return self;
  }

  auto& progress_color(this auto& self, const color& c) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    sp->progress_color = c;
    sp->make_dirty();
    return self;
  }

private:
  using control::padding;
};
} // namespace yw::ui
