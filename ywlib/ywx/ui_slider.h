#pragma once
#include <ywx/control.h>
#include <ywx/window.h>

namespace yw::ui {

class slider : public control {
public:
  struct slot : control::slot {
    double value = 0.0;
    double minimum = 0.0;
    double maximum = 1.0;
    double step = 0.1;
    float bar_width = 16.0f;
    ui::orientation orientation = ui::horizontal;
    optional<color> track_color;
    optional<color> fill_color;
    optional<color> thumb_color;
    bool pressed = false;

    function<bool, double> change_event{};

    virtual bool is_focusable() const override { return enabled && visible; }
    virtual bool is_interactive() const override { return true; }

    virtual color get_track_color(const interface::slot* Window) const noexcept {
      if (track_color) return *track_color;
      if (auto theme = get_color_theme(Window)) return (*theme)->outline;
      return colors::transparent;
    }

    virtual color get_fill_color(const interface::slot* Window) const noexcept {
      if (fill_color) return *fill_color;
      if (auto theme = get_color_theme(Window)) return (*theme)->accent;
      return colors::transparent;
    }

    virtual color get_thumb_color(const interface::slot* Window) const noexcept {
      if (thumb_color) return *thumb_color;
      if (auto theme = get_color_theme(Window)) return (*theme)->part;
      return colors::transparent;
    }

    virtual color get_border_color(const interface::slot*) const noexcept override {
      return border_color ? *border_color : colors::transparent;
    }

    virtual std::expected<void, error> draw_backcontent(interface::slot* Window) override {
      const auto track = track_rect();
      brush::color(get_track_color(Window));
      if (auto res = fill_round_rectangle(track.xy(), track.zw() - track.xy(), float2::fill(track_radius())); !res)
        return res.error().relay();

      const auto fill = fill_rect();
      if (fill.z > fill.x && fill.w > fill.y) {
        brush::color(get_fill_color(Window));
        if (auto res = fill_round_rectangle(fill.xy(), fill.zw() - fill.xy(), float2::fill(track_radius())); !res)
          return res.error().relay();
      }

      const auto center = thumb_center();
      const auto radius = thumb_radius();
      brush::color(get_thumb_color(Window));
      if (auto res = fill_ellipse(center, float2::fill(radius)); !res) return res.error().relay();
      return {};
    }

    virtual std::expected<void, error> draw_overlay(interface::slot* Window) override {
      const auto center = thumb_center();
      const auto radius = thumb_radius();
      if (pressed) {
        if (const auto wsp = static_cast<window::slot*>(Window); !wsp) return {};
        else if (wsp->press_overlay_color.a > 0.0f) {
          brush::color(wsp->press_overlay_color);
          if (auto res = fill_ellipse(center, float2::fill(radius)); !res) return res.error().relay();
        }
      } else if (is_hovered()) {
        if (const auto wsp = static_cast<window::slot*>(Window); !wsp) return {};
        else if (wsp->hover_overlay_color.a > 0.0f) {
          brush::color(wsp->hover_overlay_color);
          if (auto res = fill_ellipse(center, float2::fill(radius)); !res) return res.error().relay();
        }
      }
      return {};
    }

    virtual std::expected<void, error> draw_forecontent(interface::slot* Window) override {
      const auto center = thumb_center();
      const auto radius = thumb_radius();
      brush::color(get_border_color(Window));
      if (auto res = stroke_ellipse(center, float2::fill(radius), border_thickness); !res) return res.error().relay();
      return {};
    }

    virtual float2 get_minimum_size() const override {
      if (orientation == ui::vertical) return float2{bar_width, bar_width * 4.0f};
      else return float2{bar_width * 4.0f, bar_width};
    }

    virtual void reset_state() override {
      if (!pressed) return;
      pressed = false;
      make_dirty();
    }

    virtual bool handle_button_event(yw::button_event e) override {
      if (!enabled || !visible || e.key != keys::lbutton) return false;
      if (e.down) {
        pressed = true;
        make_dirty();
        return set_value(value_from_point(float2(float(e.pos.x), float(e.pos.y))));
      }
      if (!pressed) return false;
      pressed = false;
      make_dirty();
      return true;
    }

    virtual bool handle_click_event(yw::button_event e) override {
      return enabled && visible && !e.down && e.key == keys::lbutton;
    }

    virtual bool handle_drag_event(yw::drag_event e) override {
      if (!enabled || !visible || !pressed) return false;
      const auto axis = orientation == ui::vertical;
      const auto direction = axis ? -1.0 : 1.0;
      return set_value(value + direction * double(e.delta[axis]) * value_range() / double(track_length()));
    }

    virtual bool handle_focus_event(yw::focus_event e) override {
      if (!e.focused && pressed) {
        pressed = false;
        make_dirty();
      }
      return control::slot::handle_focus_event(e);
    }

    virtual bool handle_key_event(yw::key_event e) override {
      if (!enabled || !visible) return false;
      const bool slider_key = e.key == keys::up || e.key == keys::down || e.key == keys::left || e.key == keys::right ||
                              e.key == keys::page_up || e.key == keys::page_down || e.key == keys::home ||
                              e.key == keys::end;
      if (!e.down) return slider_key || control::slot::handle_key_event(e);
      if (e.key == keys::left || e.key == keys::down) return step_by(-1, e.mods.shift ? 10 : 1);
      if (e.key == keys::right || e.key == keys::up) return step_by(+1, e.mods.shift ? 10 : 1);
      if (e.key == keys::page_down) return step_by(-1, 10);
      if (e.key == keys::page_up) return step_by(+1, 10);
      if (e.key == keys::home) return set_value(minimum);
      if (e.key == keys::end) return set_value(maximum);
      return control::slot::handle_key_event(e);
    }

    virtual bool handle_wheel_event(yw::wheel_event e) override {
      if (!enabled || !visible || e.delta == 0) return false;
      if ((orientation == ui::horizontal) != e.horizontal) return false;
      return step_by(e.delta > 0 ? +1 : -1, e.mods.shift ? 10 : 1);
    }

    double clamp_value(double v) const noexcept { return yw::clamp(v, minimum, maximum); }

    float4 fill_rect() const noexcept {
      const auto track = track_rect();
      if (orientation == ui::vertical) {
        const auto y = thumb_center().y;
        return float4(track.x, y, track.z, track.w);
      }
      const auto x = thumb_center().x;
      return float4(track.x, track.y, x, track.w);
    }

    double ratio() const noexcept {
      if (const double range = value_range(); range <= 0.0) return 0.0;
      else return yw::clamp((value - minimum) / range, 0.0, 1.0);
    }

    bool set_value(double v, bool Notify = true) noexcept {
      const auto next = clamp_value(v);
      if (value == next) return true;
      value = next;
      make_dirty();
      return Notify && change_event ? call_event(change_event, value) : true;
    }

    bool step_by(int Direction, int Multiplier = 1) noexcept {
      if (Direction == 0 || step <= 0.0) return true;
      return set_value(value + step * double(Direction) * double(Multiplier));
    }

    float thumb_radius() const noexcept { return bar_width * 0.5f; }
    float track_radius() const noexcept { return bar_width * 0.125f; }

    float track_length() const noexcept {
      if (orientation == ui::vertical) return yw::max(1.0f, size.y - bar_width);
      else return yw::max(1.0f, size.x - bar_width);
    }

    float4 track_rect() const noexcept {
      const auto r = yw::max(1.0f, bar_width * 0.125f);
      if (orientation == ui::vertical) {
        const auto cx = pos.x + size.x * 0.5f;
        return float4(cx - r, pos.y + thumb_radius(), cx + r, pos.y + size.y - thumb_radius());
      } else {
        const auto cy = pos.y + size.y * 0.5f;
        return float4(pos.x + thumb_radius(), cy - r, pos.x + size.x - thumb_radius(), cy + r);
      }
    }

    float2 thumb_center() const noexcept {
      const auto t = float(ratio());
      if (orientation == ui::vertical) {
        const auto y = pos.y + size.y - thumb_radius() - track_length() * t;
        return float2(pos.x + size.x * 0.5f, y);
      } else {
        const auto x = pos.x + thumb_radius() + track_length() * t;
        return float2(x, pos.y + size.y * 0.5f);
      }
    }

    double value_from_point(float2 Pt) const noexcept {
      if (orientation == ui::vertical) {
        const auto p = yw::clamp((pos.y + size.y - thumb_radius() - Pt.y) / track_length(), 0.0f, 1.0f);
        return minimum + value_range() * double(p);
      } else {
        const auto p = yw::clamp((Pt.x - pos.x - thumb_radius()) / track_length(), 0.0f, 1.0f);
        return minimum + value_range() * double(p);
      }
    }

    double value_range() const noexcept { return maximum - minimum; }
  };

  using control::attached;
  using control::initialized;
  class proxy : public control::proxy {
    friend class slider;
    using control::proxy::proxy;
    slider::slot* _get_slot() const noexcept { return static_cast<slider::slot*>(_slot); }

  public:
    //-- getter --//

    double value() const&& noexcept { return _get_slot()->value; }
    double minimum() const&& noexcept { return _get_slot()->minimum; }
    double maximum() const&& noexcept { return _get_slot()->maximum; }
    double step() const&& noexcept { return _get_slot()->step; }
    float bar_width() const&& noexcept { return _get_slot()->bar_width; }
    ui::orientation orientation() const&& noexcept { return _get_slot()->orientation; }
    color track_color() const&& noexcept {
      return _get_slot()->get_track_color(interface::slot::slots.get(_get_slot()->window_id));
    }
    color fill_color() const&& noexcept {
      return _get_slot()->get_fill_color(interface::slot::slots.get(_get_slot()->window_id));
    }
    color thumb_color() const&& noexcept {
      return _get_slot()->get_thumb_color(interface::slot::slots.get(_get_slot()->window_id));
    }
    const auto& change_event() const&& noexcept { return _get_slot()->change_event; }

    //-- setter --//

    auto value(this auto&& Self, double1 Value) noexcept {
      Self._get_slot()->set_value(Value.x);
      return std::move(Self);
    }

    auto range(this auto&& Self, double1 Min, double1 Max) noexcept {
      if (Max.x < Min.x) {
        error(errors::invalid_argument, "slider range maximum must be greater than or equal to minimum").fizzle_out();
        return std::move(Self);
      }
      Self._get_slot()->minimum = Min.x;
      Self._get_slot()->maximum = Max.x;
      Self._get_slot()->set_value(Self._get_slot()->value, false);
      return std::move(Self);
    }

    auto minimum(this auto&& Self, double1 Value) noexcept {
      if (Self._get_slot()->maximum < Value.x) {
        error(errors::invalid_argument, "slider minimum must be less than or equal to maximum").fizzle_out();
        return std::move(Self);
      }
      Self._get_slot()->minimum = Value.x;
      Self._get_slot()->set_value(Self._get_slot()->value, false);
      return std::move(Self);
    }

    auto maximum(this auto&& Self, double1 Value) noexcept {
      if (Value.x < Self._get_slot()->minimum) {
        error(errors::invalid_argument, "slider maximum must be greater than or equal to minimum").fizzle_out();
        return std::move(Self);
      }
      Self._get_slot()->maximum = Value.x;
      Self._get_slot()->set_value(Self._get_slot()->value, false);
      return std::move(Self);
    }

    auto step(this auto&& Self, double1 Step) noexcept {
      if (Step.x <= 0.0) {
        error(errors::invalid_argument, "slider step must be positive").fizzle_out();
        return std::move(Self);
      }
      Self._get_slot()->step = Step.x;
      return std::move(Self);
    }

    auto bar_width(this auto&& Self, float1 Width) noexcept {
      if (Width.x <= 0.0f) {
        error(errors::invalid_argument, "slider bar_width must be positive").fizzle_out();
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

    auto track_color(this auto&& Self, const color& Color) noexcept {
      Self._get_slot()->track_color = Color;
      Self._dirty = true;
      return std::move(Self);
    }

    auto track_color(this auto&& Self, none) noexcept {
      Self._get_slot()->track_color = none();
      Self._dirty = true;
      return std::move(Self);
    }

    auto fill_color(this auto&& Self, const color& Color) noexcept {
      Self._get_slot()->fill_color = Color;
      Self._dirty = true;
      return std::move(Self);
    }

    auto fill_color(this auto&& Self, none) noexcept {
      Self._get_slot()->fill_color = none();
      Self._dirty = true;
      return std::move(Self);
    }

    auto thumb_color(this auto&& Self, const color& Color) noexcept {
      Self._get_slot()->thumb_color = Color;
      Self._dirty = true;
      return std::move(Self);
    }

    auto thumb_color(this auto&& Self, none) noexcept {
      Self._get_slot()->thumb_color = none();
      Self._dirty = true;
      return std::move(Self);
    }

    auto change_event(this auto&& Self, function<bool, double> Event) noexcept {
      Self._get_slot()->change_event = std::move(Event);
      return std::move(Self);
    }
  };

  slider() noexcept = default;

  slider(derived_from<interface> auto& Parent, const source_line& sl = here()) {
    if (auto res = create(Parent)) *this = std::move(*res);
    else res.error().add_footprint().go_off(sl);
  }

  static std::expected<slider, error> create() {
    slider s;
    slider::slot* sp;
    if (auto res = create_control<slider>()) sp = *res;
    else return res.error().relay();
    s._id = sp->id;
    sp->policy[sp->orientation == ui::horizontal] = ui::size_policy::fit;
    return s;
  }

  static std::expected<slider, error> create(derived_from<interface> auto& Parent) {
    auto res = create();
    if (!res) return res.error().relay();
    if (auto attached = res->attach(Parent); !attached) return attached.error().relay();
    return res;
  }

  yw_control_getter_setter(value, double1);
  auto range(this auto& Self, double1 Min, double1 Max) noexcept {
    return typename remove_cvref<decltype(Self)>::proxy(get_slot(&Self)).range(Min, Max);
  }
  yw_control_getter_setter(minimum, double1);
  yw_control_getter_setter(maximum, double1);
  yw_control_getter_setter(step, double1);
  yw_control_getter_setter(bar_width, float1);
  yw_control_getter_setter(orientation, ui::orientation);
  yw_control_getter_setter(track_color, color);
  auto track_color(this auto& Self, none None) noexcept {
    return typename remove_cvref<decltype(Self)>::proxy(get_slot(&Self)).track_color(None);
  }
  yw_control_getter_setter(fill_color, color);
  auto fill_color(this auto& Self, none None) noexcept {
    return typename remove_cvref<decltype(Self)>::proxy(get_slot(&Self)).fill_color(None);
  }
  yw_control_getter_setter(thumb_color, color);
  auto thumb_color(this auto& Self, none None) noexcept {
    return typename remove_cvref<decltype(Self)>::proxy(get_slot(&Self)).thumb_color(None);
  }
  yw_control_getter_setter(change_event, function<bool, double>);

private:
  using control::padding;
};
} // namespace yw::ui
