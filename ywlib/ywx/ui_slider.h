#pragma once
#include <ywx/ui_frame.h>

namespace yw::ui {

class slider : public frame {
public:
  struct slot : frame::slot {
    float value = 0.0f;
    float minimum = 0.0f;
    float maximum = 1.0f;
    float step = 0.0f;
    float thumb_size = 16.0f;
    float track_thickness = 4.0f;
    float wheel_step = 0.0f;
    ui::orientation orientation = ui::orientation::horizontal;
    color track_color = color(0.0f, 0.0f, 0.0f, 0.10f);
    color fill_color = color(colors::dodgerblue, 0.45f);
    color thumb_color = color(colors::white, 0.95f);
    color thumb_border_color = color(0.0f, 0.0f, 0.0f, 0.30f);
    color pressed_color = color(0.0f, 0.0f, 0.0f, 0.12f);
    float drag_value = 0.0f;
    bool pressed = false;

    function<void, float> on_value_change{};

    float clamp_value(float v) const noexcept { return yw::clamp(v, minimum, maximum); }

    float quantize_value(float v) const noexcept {
      v = clamp_value(v);
      if (step <= 0.0f || maximum <= minimum) return v;
      const auto n = yw::round((v - minimum) / step);
      return clamp_value(minimum + float(n) * step);
    }

    float ratio() const noexcept {
      const float range = maximum - minimum;
      if (range <= 0.0f) return 0.0f;
      return yw::clamp((value - minimum) / range, 0.0f, 1.0f);
    }

    float default_step() const noexcept {
      const float range = maximum - minimum;
      if (step > 0.0f) return step;
      return range > 0.0f ? range / 100.0f : 0.0f;
    }

    void set_value(float v, bool Notify = true) {
      const auto next = quantize_value(v);
      if (value == next) return;
      value = next;
      make_dirty();
      if (Notify && on_value_change) on_value_change(value);
    }

    void step_by(float Direction, int Multiplier = 1) {
      const auto amount = default_step() * Direction * float(Multiplier);
      if (amount == 0.0f) return;
      set_value(value + amount);
    }

    float4 inner_rect() const noexcept {
      const auto p = pos + padding.xy();
      const auto s = size - padding.xy() - padding.zw();
      return {p.x, p.y, p.x + yw::max(0.0f, s.x), p.y + yw::max(0.0f, s.y)};
    }

    float4 track_rect() const noexcept {
      const auto inner = inner_rect();
      if (orientation == ui::orientation::vertical) {
        const float cx = (inner.x + inner.z) * 0.5f;
        const float t = yw::min(track_thickness, yw::max(0.0f, inner.z - inner.x));
        return {cx - t * 0.5f, inner.y + thumb_size * 0.5f, cx + t * 0.5f, inner.w - thumb_size * 0.5f};
      }
      const float cy = (inner.y + inner.w) * 0.5f;
      const float t = yw::min(track_thickness, yw::max(0.0f, inner.w - inner.y));
      return {inner.x + thumb_size * 0.5f, cy - t * 0.5f, inner.z - thumb_size * 0.5f, cy + t * 0.5f};
    }

    float4 fill_rect() const noexcept {
      const auto track = track_rect();
      const auto r = ratio();
      if (orientation == ui::orientation::vertical) {
        const float y = track.w - (track.w - track.y) * r;
        return {track.x, y, track.z, track.w};
      }
      return {track.x, track.y, track.x + (track.z - track.x) * r, track.w};
    }

    float4 thumb_rect() const noexcept {
      const auto track = track_rect();
      const auto r = ratio();
      if (orientation == ui::orientation::vertical) {
        const float y = track.w - (track.w - track.y) * r;
        const float cx = (track.x + track.z) * 0.5f;
        return {cx - thumb_size * 0.5f, y - thumb_size * 0.5f, cx + thumb_size * 0.5f, y + thumb_size * 0.5f};
      }
      const float x = track.x + (track.z - track.x) * r;
      const float cy = (track.y + track.w) * 0.5f;
      return {x - thumb_size * 0.5f, cy - thumb_size * 0.5f, x + thumb_size * 0.5f, cy + thumb_size * 0.5f};
    }

    float value_from_point(float2 Pt) const noexcept {
      const auto track = track_rect();
      if (orientation == ui::orientation::vertical) {
        const float length = track.w - track.y;
        if (length <= 0.0f) return minimum;
        const float r = yw::clamp((track.w - Pt.y) / length, 0.0f, 1.0f);
        return minimum + (maximum - minimum) * r;
      }
      const float length = track.z - track.x;
      if (length <= 0.0f) return minimum;
      const float r = yw::clamp((Pt.x - track.x) / length, 0.0f, 1.0f);
      return minimum + (maximum - minimum) * r;
    }

    static float2 rect_pos(float4 Rect) noexcept { return {Rect.x, Rect.y}; }
    static float2 rect_size(float4 Rect) noexcept { return {Rect.z - Rect.x, Rect.w - Rect.y}; }

    std::expected<void, error> fill_round_rect(float4 Rect, const color& Color, float2 Radius) const {
      if (Color.a <= 0.0f || Rect.z <= Rect.x || Rect.w <= Rect.y) return {};
      brush::color(Color);
      if (auto res = fill_round_rectangle(rect_pos(Rect), rect_size(Rect), Radius); !res) return res.error().relay();
      return {};
    }

    std::expected<void, error> draw_round_rect(float4 Rect, const color& Color, float2 Radius, float Width) const {
      if (Color.a <= 0.0f || Width <= 0.0f || Rect.z <= Rect.x || Rect.w <= Rect.y) return {};
      brush::color(Color);
      if (auto res = draw_round_rectangle(rect_pos(Rect), rect_size(Rect), Radius, Width); !res) return res.error().relay();
      return {};
    }

    virtual bool focusable() const noexcept override { return enabled && visible; }

    virtual std::expected<float2, error> get_necessary_size() const override {
      const auto base = orientation == ui::orientation::vertical ? float2{thumb_size, thumb_size * 3.0f}
                                                                 : float2{thumb_size * 3.0f, thumb_size};
      return calc_necessary_size_by_policy(padding.xy() + padding.zw() + base);
    }

    virtual std::expected<void, error> redraw() override {
      if (geometry_dirty) {
        geometry_dirty = false;
        if (auto res = relocate(); !res) return res.error().relay();
      }
      if (!visible) return {};
      if (auto res = draw_frame_background(); !res) return res.error().relay();
      const auto track_radius = float2::fill(track_thickness * 0.5f);
      if (auto res = fill_round_rect(track_rect(), track_color, track_radius); !res) return res.error().relay();
      if (auto res = fill_round_rect(fill_rect(), fill_color, track_radius); !res) return res.error().relay();
      const auto thumb = thumb_rect();
      const auto thumb_radius = float2::fill(thumb_size * 0.5f);
      if (auto res = fill_round_rect(thumb, thumb_color, thumb_radius); !res) return res.error().relay();
      if (pressed)
        if (auto res = fill_round_rect(thumb, pressed_color, thumb_radius); !res) return res.error().relay();
      if (auto res = draw_round_rect(thumb, thumb_border_color, thumb_radius, 1.0f); !res) return res.error().relay();
      if (auto res = draw_frame_foreground(); !res) return res.error().relay();
      return {};
    }

    virtual bool button_event(yw::button_event e) override {
      if (!enabled || !visible || e.key != keys::lbutton) return frame::slot::button_event(e);
      if (e.down) {
        pressed = true;
        drag_value = value_from_point(float2(float(e.pos.x), float(e.pos.y)));
        set_value(drag_value);
        make_dirty();
        return true;
      }
      if (!pressed) return frame::slot::button_event(e);
      pressed = false;
      drag_value = value;
      make_dirty();
      return true;
    }

    virtual bool click_event(yw::button_event e) override {
      if (!enabled || !visible || e.down || e.key != keys::lbutton) return frame::slot::click_event(e);
      return true;
    }

    virtual bool drag_event(yw::drag_event e) override {
      if (!enabled || !visible || !pressed || e.key != keys::lbutton) return frame::slot::drag_event(e);
      const auto track = track_rect();
      const float length = orientation == ui::orientation::vertical ? track.w - track.y : track.z - track.x;
      if (length <= 0.0f || maximum <= minimum) return true;
      const float delta = orientation == ui::orientation::vertical ? -float(e.delta.y) : float(e.delta.x);
      drag_value = clamp_value(drag_value + (maximum - minimum) * delta / length);
      set_value(drag_value);
      return true;
    }

    virtual std::expected<void, error> reset_state() override {
      if (!pressed) return {};
      pressed = false;
      drag_value = value;
      make_dirty();
      return {};
    }

    virtual void focus_event(bool Focused) override {
      if (!Focused && pressed) {
        pressed = false;
        drag_value = value;
        make_dirty();
      }
      frame::slot::focus_event(Focused);
    }

    virtual bool key_event(yw::key_event e) override {
      if (!enabled || !visible) return false;
      const bool decrease_key =
        orientation == ui::orientation::vertical ? e.key == keys::down || e.key == keys::left
                                                 : e.key == keys::left || e.key == keys::down;
      const bool increase_key =
        orientation == ui::orientation::vertical ? e.key == keys::up || e.key == keys::right
                                                 : e.key == keys::right || e.key == keys::up;
      if (!e.down) {
        if (decrease_key || increase_key || e.key == keys::page_up || e.key == keys::page_down || e.key == keys::home ||
            e.key == keys::end)
          return true;
        return frame::slot::key_event(e);
      }
      if (decrease_key) return step_by(-1.0f, e.mods.shift ? 10 : 1), true;
      if (increase_key) return step_by(+1.0f, e.mods.shift ? 10 : 1), true;
      if (e.key == keys::page_up) return step_by(+1.0f, 10), true;
      if (e.key == keys::page_down) return step_by(-1.0f, 10), true;
      if (e.key == keys::home) return set_value(minimum), true;
      if (e.key == keys::end) return set_value(maximum), true;
      return frame::slot::key_event(e);
    }

    virtual bool wheel_event(yw::wheel_event e) override {
      if (!enabled || !visible || e.delta == 0) return false;
      if ((orientation == ui::orientation::horizontal) != e.horizontal && !e.mods.shift) return false;
      const auto amount = wheel_step > 0.0f ? wheel_step : default_step();
      if (amount <= 0.0f) return false;
      set_value(value + amount * (e.delta > 0 ? +1.0f : -1.0f) * (e.mods.shift ? 10.0f : 1.0f));
      return true;
    }

    virtual std::expected<void, error> apply_color_theme(const yw::ui::color_theme& Theme, bool Recursive) override {
      background_color = colors::transparent;
      border_color = colors::transparent;
      hovered_overlay_color = colors::transparent;
      track_color = color(Theme.outline, 0.18f);
      fill_color = color(Theme.accent, 0.55f);
      thumb_color = Theme.surface;
      thumb_border_color = Theme.outline;
      pressed_color = color(Theme.accent, default_overlay_opacity.pressed);
      make_dirty();
      return {};
    }
  };

  using frame::operator bool;
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
    sp->policy = {ui::size_policy::fit, ui::size_policy::fit};
    if (auto res = sp->apply_current_color_theme(false); !res) return res.error().relay();
    return s;
  }

  //-- getter --//

  float value() const noexcept { ywlib_control_get(value); }
  float minimum() const noexcept { ywlib_control_get(minimum); }
  float maximum() const noexcept { ywlib_control_get(maximum); }
  float step() const noexcept { ywlib_control_get(step); }
  float thumb_size() const noexcept { ywlib_control_get(thumb_size); }
  float track_thickness() const noexcept { ywlib_control_get(track_thickness); }
  float wheel_step() const noexcept { ywlib_control_get(wheel_step); }
  ui::orientation orientation() const noexcept { ywlib_control_get(orientation); }
  bool pressed() const noexcept { ywlib_control_get(pressed); }
  const auto& track_color() const noexcept { ywlib_control_get(track_color); }
  const auto& fill_color() const noexcept { ywlib_control_get(fill_color); }
  const auto& thumb_color() const noexcept { ywlib_control_get(thumb_color); }
  const auto& thumb_border_color() const noexcept { ywlib_control_get(thumb_border_color); }
  const auto& pressed_color() const noexcept { ywlib_control_get(pressed_color); }
  const auto& on_change() const noexcept { ywlib_control_get(on_value_change); }

  //-- setter --//

  auto& value(this auto& self, float v) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->set_value(v);
    return self;
  }

  auto& range(this auto& self, float Min, float Max) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    if (Max < Min) {
      error(errors::invalid_argument, "slider range maximum must be greater than or equal to minimum").go_off();
      return self;
    }
    sp->minimum = Min;
    sp->maximum = Max;
    sp->set_value(sp->value);
    sp->make_dirty();
    return self;
  }

  auto& minimum(this auto& self, float v) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    if (sp->maximum < v) {
      error(errors::invalid_argument, "slider minimum must be less than or equal to maximum").go_off();
      return self;
    }
    sp->minimum = v;
    sp->set_value(sp->value);
    sp->make_dirty();
    return self;
  }

  auto& maximum(this auto& self, float v) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    if (v < sp->minimum) {
      error(errors::invalid_argument, "slider maximum must be greater than or equal to minimum").go_off();
      return self;
    }
    sp->maximum = v;
    sp->set_value(sp->value);
    sp->make_dirty();
    return self;
  }

  auto& step(this auto& self, float1 v) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    if (v.x < 0.0f) {
      error(errors::invalid_argument, "slider step must be non-negative").go_off();
      return self;
    }
    sp->step = v.x;
    sp->set_value(sp->value, false);
    return self;
  }

  auto& thumb_size(this auto& self, float1 v) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    if (v.x <= 0.0f) {
      error(errors::invalid_argument, "slider thumb_size must be positive").go_off();
      return self;
    }
    sp->thumb_size = v.x;
    sp->make_messy();
    return self;
  }

  auto& track_thickness(this auto& self, float1 v) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    if (v.x <= 0.0f) {
      error(errors::invalid_argument, "slider track_thickness must be positive").go_off();
      return self;
    }
    sp->track_thickness = v.x;
    sp->make_dirty();
    return self;
  }

  auto& wheel_step(this auto& self, float1 v) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    if (v.x < 0.0f) {
      error(errors::invalid_argument, "slider wheel_step must be non-negative").go_off();
      return self;
    }
    sp->wheel_step = v.x;
    return self;
  }

  auto& orientation(this auto& self, ui::orientation v) noexcept { ywlib_control_set(orientation, v, messy); }
  auto& track_color(this auto& self, const color& c) noexcept { ywlib_control_set(track_color, c, dirty); }
  auto& fill_color(this auto& self, const color& c) noexcept { ywlib_control_set(fill_color, c, dirty); }
  auto& thumb_color(this auto& self, const color& c) noexcept { ywlib_control_set(thumb_color, c, dirty); }
  auto& thumb_border_color(this auto& self, const color& c) noexcept { ywlib_control_set(thumb_border_color, c, dirty); }
  auto& pressed_color(this auto& self, const color& c) noexcept { ywlib_control_set(pressed_color, c, dirty); }

  auto& on_change(this auto& self, function<void, float> f) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->on_value_change = std::move(f);
    return self;
  }
};
} // namespace yw::ui
