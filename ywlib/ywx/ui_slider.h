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
    color track_color;
    color fill_color;
    color thumb_color;
    bool pressed = false;

    function<bool, double> change_event{};

    virtual bool is_focusable() const override { return enabled && visible; }
    virtual bool is_interactive() const override { return true; }

    virtual std::expected<void, error> apply_color_theme(const ui::color_theme& Theme, bool) override {
      background_color = Theme.surface;
      border_color = colors::transparent;
      track_color = Theme.outline;
      fill_color = Theme.accent;
      thumb_color = Theme.part;
      make_dirty();
      return {};
    }

    virtual std::expected<void, error> draw_backcontent() override {
      const auto track = track_rect();
      brush::color(track_color);
      if (auto res = fill_round_rectangle(track.xy(), track.zw() - track.xy(), float2::fill(track_radius())); !res)
        return res.error().relay();

      const auto fill = fill_rect();
      if (fill.z > fill.x && fill.w > fill.y) {
        brush::color(fill_color);
        if (auto res = fill_round_rectangle(fill.xy(), fill.zw() - fill.xy(), float2::fill(track_radius())); !res)
          return res.error().relay();
      }

      const auto center = thumb_center();
      const auto radius = thumb_radius();
      brush::color(thumb_color);
      if (auto res = fill_ellipse(center, float2::fill(radius)); !res) return res.error().relay();
      return {};
    }

    virtual std::expected<void, error> draw_overlay() override {
      const auto center = thumb_center();
      const auto radius = thumb_radius();
      if (pressed) {
        if (const auto wsp = get_slot<window>(window_id); !wsp) return std::unexpected(error(errors::invalid_slotid));
        else if (wsp->press_overlay_color.a > 0.0f) {
          brush::color(wsp->press_overlay_color);
          if (auto res = fill_ellipse(center, float2::fill(radius)); !res) return res.error().relay();
        }
      } else if (is_hovered()) {
        if (const auto wsp = get_slot<window>(window_id); !wsp) return std::unexpected(error(errors::invalid_slotid));
        else if (wsp->hover_overlay_color.a > 0.0f) {
          brush::color(wsp->hover_overlay_color);
          if (auto res = fill_ellipse(center, float2::fill(radius)); !res) return res.error().relay();
        }
      }
      return {};
    }

    virtual std::expected<void, error> draw_forecontent() override {
      const auto center = thumb_center();
      const auto radius = thumb_radius();
      brush::color(border_color);
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
      return Notify && change_event ? change_event(value) : true;
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

  using control::operator bool;
  slider() noexcept = default;

  slider(derived_from<interface> auto& Parent, const source_line& sl = here()) {
    if (auto res = create(Parent)) *this = std::move(*res);
    else res.error().add_footprint().go_off(sl);
  }

  static std::expected<slider, error> create(derived_from<interface> auto& Parent) {
    slider s;
    const auto temp_id = make_slot<slider>();
    const auto sp = get_slot<slider>(temp_id);
    if (!sp) return std::unexpected(error(errors::slot_creation_failed));
    const auto psp = get_slot<control>(Parent.id());
    if (!psp) return std::unexpected(error(errors::invalid_slotid));
    if (auto res = psp->attach(temp_id); !res) {
      slot::slots.erase(temp_id);
      return res.error().relay();
    }
    s._id = temp_id;
    sp->id = temp_id;
    sp->window_id = psp->get_window_id();
    sp->policy[sp->orientation == ui::horizontal] = ui::size_policy::fit;
    if (auto theme = sp->get_color_theme(); !theme) return theme.error().relay();
    else if (auto res = sp->apply_color_theme(*(*theme), false); !res) return res.error().relay();
    return s;
  }

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

  double step() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return {};
    }
    return sp->step;
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

  const auto& track_color() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->track_color;
  }

  const auto& fill_color() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->fill_color;
  }

  const auto& thumb_color() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->thumb_color;
  }

  const auto& change_event() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->change_event;
  }

  auto& value(this auto& self, double1 v) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    sp->set_value(v.x);
    return self;
  }

  auto& range(this auto& self, double1 Min, double1 Max) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    if (Max.x < Min.x) {
      error(errors::invalid_argument, "slider range maximum must be greater than or equal to minimum").fizzle_out();
      return self;
    }
    sp->minimum = Min.x;
    sp->maximum = Max.x;
    sp->set_value(sp->value, false);
    return self;
  }

  auto& minimum(this auto& self, double1 v) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    if (sp->maximum < v.x) {
      error(errors::invalid_argument, "slider minimum must be less than or equal to maximum").fizzle_out();
      return self;
    }
    sp->minimum = v.x;
    sp->set_value(sp->value, false);
    return self;
  }

  auto& maximum(this auto& self, double1 v) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    if (v.x < sp->minimum) {
      error(errors::invalid_argument, "slider maximum must be greater than or equal to minimum").fizzle_out();
      return self;
    }
    sp->maximum = v.x;
    sp->set_value(sp->value, false);
    return self;
  }

  auto& step(this auto& self, double1 v) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    if (v.x <= 0.0) {
      error(errors::invalid_argument, "slider step must be positive").fizzle_out();
      return self;
    }
    sp->step = v.x;
    return self;
  }

  auto& bar_width(this auto& self, float1 v) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    if (v.x <= 0.0f) {
      error(errors::invalid_argument, "slider bar_width must be positive").fizzle_out();
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

  auto& track_color(this auto& self, const color& c) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    sp->track_color = c;
    sp->make_dirty();
    return self;
  }

  auto& fill_color(this auto& self, const color& c) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    sp->fill_color = c;
    sp->make_dirty();
    return self;
  }

  auto& thumb_color(this auto& self, const color& c) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    sp->thumb_color = c;
    sp->make_dirty();
    return self;
  }

  auto& change_event(this auto& self, function<bool, double> f) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    sp->change_event = std::move(f);
    return self;
  }

private:
  using control::padding;
};
} // namespace yw::ui
