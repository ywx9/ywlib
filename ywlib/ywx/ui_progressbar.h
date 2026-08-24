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
    optional<color> progress_color;

    //-- override functions --//

    virtual color get_progress_color(const interface::slot* Window) const noexcept {
      if (progress_color) return *progress_color;
      if (auto theme = get_color_theme(Window)) return (*theme)->accent;
      return colors::transparent;
    }

    virtual std::expected<void, error> draw_backcontent(interface::slot* Window) override {
      const auto ratio = float(this->ratio());
      if (ratio == 0.0f) return {};
      brush::color(get_progress_color(Window));
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
  class proxy : public control::proxy {
    friend class progressbar;
    using control::proxy::proxy;
    progressbar::slot* _get_slot() const noexcept { return static_cast<progressbar::slot*>(_slot); }

  public:
    //-- getter --//

    double value() const&& noexcept { return _get_slot()->value; }
    double minimum() const&& noexcept { return _get_slot()->minimum; }
    double maximum() const&& noexcept { return _get_slot()->maximum; }
    float bar_width() const&& noexcept { return _get_slot()->bar_width; }
    ui::orientation orientation() const&& noexcept { return _get_slot()->orientation; }
    color progress_color() const&& noexcept {
      return _get_slot()->get_progress_color(interface::slot::slots.get(_get_slot()->window_id));
    }

    //-- setter --//

    auto value(this auto&& Self, double1 Value) noexcept {
      Self._get_slot()->value = Self._get_slot()->clamp_value(Value.x);
      Self._dirty = true;
      return std::move(Self);
    }

    auto range(this auto&& Self, double1 Min, double1 Max) noexcept {
      if (Max.x < Min.x) {
        error(errors::invalid_argument, "progressbar range maximum must be greater than or equal to minimum").fizzle_out();
        return std::move(Self);
      }
      Self._get_slot()->minimum = Min.x;
      Self._get_slot()->maximum = Max.x;
      Self._get_slot()->value = Self._get_slot()->clamp_value(Self._get_slot()->value);
      Self._dirty = true;
      return std::move(Self);
    }

    auto minimum(this auto&& Self, double1 Value) noexcept {
      if (Self._get_slot()->maximum < Value.x) {
        error(errors::invalid_argument, "progressbar minimum must be less than or equal to maximum").fizzle_out();
        return std::move(Self);
      }
      Self._get_slot()->minimum = Value.x;
      Self._get_slot()->value = Self._get_slot()->clamp_value(Self._get_slot()->value);
      Self._dirty = true;
      return std::move(Self);
    }

    auto maximum(this auto&& Self, double1 Value) noexcept {
      if (Value.x < Self._get_slot()->minimum) {
        error(errors::invalid_argument, "progressbar maximum must be greater than or equal to minimum").fizzle_out();
        return std::move(Self);
      }
      Self._get_slot()->maximum = Value.x;
      Self._get_slot()->value = Self._get_slot()->clamp_value(Self._get_slot()->value);
      Self._dirty = true;
      return std::move(Self);
    }

    auto bar_width(this auto&& Self, float1 Width) noexcept {
      if (Width.x <= 0.0f) {
        error(errors::invalid_argument, "progressbar bar_width must be positive").fizzle_out();
        return std::move(Self);
      }
      Self._get_slot()->bar_width = Width.x;
      Self._messy = true;
      return std::move(Self);
    }

    auto orientation(this auto&& Self, ui::orientation Orientation) noexcept {
      if (Self._get_slot()->orientation != Orientation) {
        Self._get_slot()->orientation = Orientation;
        Self._get_slot()->swap_dimensions();
      }
      return std::move(Self);
    }

    auto progress_color(this auto&& Self, const color& Color) noexcept {
      Self._get_slot()->progress_color = Color;
      Self._dirty = true;
      return std::move(Self);
    }

    auto progress_color(this auto&& Self, none) noexcept {
      Self._get_slot()->progress_color = none();
      Self._dirty = true;
      return std::move(Self);
    }
  };

  progressbar() noexcept = default;

  progressbar(derived_from<interface> auto& Parent, const source_line& sl = here()) {
    if (auto res = create(Parent)) *this = std::move(*res);
    else res.error().add_footprint().go_off(sl);
  }

  static std::expected<progressbar, error> create(derived_from<interface> auto& Parent) {
    progressbar p;
    progressbar::slot* sp;
    if (auto res = create_control<progressbar>(Parent)) sp = *res;
    else return res.error().relay();
    p._id = sp->id;
    sp->policy[sp->orientation == ui::horizontal] = ui::size_policy::fit;
    return p;
  }

  yw_control_getter_setter(value, double1);
  auto range(this auto& Self, double1 Min, double1 Max) noexcept {
    return typename remove_cvref<decltype(Self)>::proxy(get_slot(&Self)).range(Min, Max);
  }
  yw_control_getter_setter(minimum, double1);
  yw_control_getter_setter(maximum, double1);
  yw_control_getter_setter(bar_width, float1);
  yw_control_getter_setter(orientation, ui::orientation);
  yw_control_getter_setter(progress_color, color);
  auto progress_color(this auto& Self, none None) noexcept {
    return typename remove_cvref<decltype(Self)>::proxy(get_slot(&Self)).progress_color(None);
  }

private:
  using control::padding;
};
} // namespace yw::ui
