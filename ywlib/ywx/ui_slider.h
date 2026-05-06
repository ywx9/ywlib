#pragma once
#include "ywx/ui_control.h"
#include <cmath>

namespace yw::ui {

class slider : public control {
public:
  class slot : public control::slot {
    static constexpr double epsilon = 1e-9;

    void normalize_bounds() noexcept {
      if (min_value > max_value) std::swap(min_value, max_value);
    }

    double clamp_value(double v) const noexcept {
      if (wrap && max_value > min_value) {
        const auto range = max_value - min_value;
        while (v > max_value) v -= range;
        while (v < min_value) v += range;
        return v;
      }
      return std::clamp(v, min_value, max_value);
    }

    double normalized_value() const noexcept {
      const auto range = max_value - min_value;
      if (range <= epsilon) return 0.0;
      return (value - min_value) / range;
    }

    float2 content_pos() const noexcept {
      return pos + padding.xy();
    }

    float2 content_size() const noexcept {
      return vapply_r<float2>(yw::max, float2(), size - padding.xy() - padding.zw());
    }

    float2 track_pos() const noexcept {
      const auto cp = content_pos();
      const auto cs = content_size();
      if (vertical) {
        return {cp.x + (cs.x - track_thickness) * 0.5f, cp.y + thumb_size.y * 0.5f};
      }
      return {cp.x + thumb_size.x * 0.5f, cp.y + (cs.y - track_thickness) * 0.5f};
    }

    float2 track_size() const noexcept {
      const auto cs = content_size();
      if (vertical) {
        return {track_thickness, yw::max(1.0f, cs.y - thumb_size.y)};
      }
      return {yw::max(1.0f, cs.x - thumb_size.x), track_thickness};
    }

    float2 thumb_center() const noexcept {
      const auto tp = track_pos();
      const auto ts = track_size();
      const auto t = float(normalized_value());
      if (vertical) {
        return {tp.x + ts.x * 0.5f, tp.y + (1.0f - t) * ts.y};
      }
      return {tp.x + t * ts.x, tp.y + ts.y * 0.5f};
    }

    double point_to_value(float2 pt) const noexcept {
      const auto tp = track_pos();
      const auto ts = track_size();
      double t = 0.0;
      if (vertical) {
        if (ts.y <= 1.0f) t = 0.0;
        else t = 1.0 - std::clamp(double((pt.y - tp.y) / ts.y), 0.0, 1.0);
      } else {
        if (ts.x <= 1.0f) t = 0.0;
        else t = std::clamp(double((pt.x - tp.x) / ts.x), 0.0, 1.0);
      }
      return min_value + (max_value - min_value) * t;
    }

  public:
    float4 padding = float4::fill(4.0f);

    color track_color = color(0.82f, 0.82f, 0.82f, 1.0f);
    color fill_color = color(0.25f, 0.48f, 0.95f, 1.0f);
    color thumb_color = colors::white;
    color thumb_border_color = colors::black;
    float thumb_border_width = 1.0f;
    float track_thickness = 6.0f;
    float2 thumb_size{14.0f, 14.0f};

    double min_value = 0.0;
    double max_value = 100.0;
    double value = 0.0;
    double step = 1.0;
    double page_step = 10.0;
    bool vertical = false;
    bool wrap = false;

    function<void, event::button> on_button;
    function<void, double> on_change;
    function<void, bool> on_focus;
    function<void, event::key> on_keydown;
    function<void, event::key> on_keyup;
    function<void, event::move> on_move;
    function<void, event::wheel> on_wheel;

    key captured_key{};
    bool dragging = false;

    bool apply_value(double next, bool notify = true) {
      normalize_bounds();
      next = clamp_value(next);
      if (std::fabs(next - value) <= epsilon) return false;
      value = next;
      make_dirty();
      if (notify && on_change) on_change(value);
      return true;
    }

    void step_by(double delta, bool notify = true) {
      apply_value(value + delta, notify);
    }

    bool set_by_point(float2 pt, bool notify = true) {
      return apply_value(point_to_value(pt), notify);
    }

    virtual float2 calculate_size() const noexcept override {
      const float2 prefer = vertical ? float2(32.0f, 120.0f) : float2(120.0f, 32.0f);
      const auto inner = prefer + padding.xy() + padding.zw();
      return vapply_r<float2>(yw::max, float2(), min_size, inner, size * constrained);
    }

    virtual void update_size() noexcept override {
      min_size = vapply_r<float2>(yw::max, min_size, float2());
      const float2 prefer = vertical ? float2(32.0f, 120.0f) : float2(120.0f, 32.0f);
      const auto inner = prefer + padding.xy() + padding.zw();
      size = vapply_r<float2>(yw::max, min_size, inner, size * constrained);
      update_geometry();
    }

    virtual void draw() const override {
      if (!visible) return;
      draw_background();

      const auto tp = track_pos();
      const auto ts = track_size();
      const auto center = thumb_center();

      brush.color(track_color);
      fill_rectangle(tp, ts);

      brush.color(fill_color);
      if (vertical) {
        const auto top = center.y;
        fill_rectangle(float2(tp.x, top), float2(ts.x, tp.y + ts.y - top));
      } else {
        fill_rectangle(tp, float2(center.x - tp.x, ts.y));
      }

      brush.color(thumb_color);
      fill_ellipse(center, thumb_size * 0.5f);
      brush.color(thumb_border_color);
      draw_ellipse(center, thumb_size * 0.5f, thumb_border_width);
    }

    virtual void button_event(event::button e) override {
      if (!enabled) return;
      if (e.down) {
        captured_key = e.code;
        if (captured_key == key::lbutton) {
          dragging = true;
          set_by_point(float2(e.pos));
        }
      } else {
        captured_key = {};
        dragging = false;
      }
      if (on_button) on_button(e);
    }

    virtual void click_event(event::button) override {
      captured_key = {};
      dragging = false;
    }

    virtual void drag_event(event::drag e) override {
      if (!enabled || !dragging || e.code != key::lbutton) return;
      set_by_point(float2(e.delta));
    }

    virtual bool focus_event(bool focused) override {
      if (!focused) {
        captured_key = {};
        dragging = false;
      }
      if (enabled && on_focus) on_focus(focused);
      return enabled;
    }

    virtual bool key_event(event::key e) override {
      if (!enabled) return false;
      if (e.down) {
        switch (e.code) {
        case key::left:
          if (!vertical) step_by(-std::fabs(step));
          break;
        case key::right:
          if (!vertical) step_by(std::fabs(step));
          break;
        case key::up:
          if (vertical) step_by(std::fabs(step));
          else step_by(std::fabs(step));
          break;
        case key::down:
          if (vertical) step_by(-std::fabs(step));
          else step_by(-std::fabs(step));
          break;
        case key::page_up: step_by(std::fabs(page_step)); break;
        case key::page_down: step_by(-std::fabs(page_step)); break;
        case key::home: apply_value(min_value); break;
        case key::end: apply_value(max_value); break;
        default: break;
        }
        if (e.first && on_keydown) on_keydown(e);
        return true;
      }
      if (on_keyup) on_keyup(e);
      return true;
    }

    virtual void move_event(event::move e) override {
      if (enabled && on_move) on_move(e);
    }

    virtual void wheel_event(event::wheel e) override {
      if (!enabled || e.horizontal) return;
      const auto delta = e.delta > 0 ? 1.0 : -1.0;
      const auto amount = e.shift ? std::fabs(page_step) : std::fabs(step);
      step_by(amount * delta);
      if (on_wheel) on_wheel(e);
    }
  };

  using control::operator bool;
  slider() noexcept = default;
  slider(derived_from<unknown> auto& Layout) {
    if (auto res = create_control<slider>(Layout)) _id = *res;
  }

  const auto& border_color() const { return unsafe_get(&slot::border_color); }
  void border_color(const color& c) { safe_set(&slot::border_color, c); }

  const auto& border_width() const { return unsafe_get(&slot::border_width); }
  void border_width(float w) { safe_set(&slot::border_width, w); }

  const auto& padding() const { return unsafe_get(&slot::padding); }
  void padding(const float4& p) { safe_set_size(&slot::padding, p); }

  const auto& track_color() const { return unsafe_get(&slot::track_color); }
  void track_color(const color& c) { safe_set(&slot::track_color, c); }

  const auto& fill_color() const { return unsafe_get(&slot::fill_color); }
  void fill_color(const color& c) { safe_set(&slot::fill_color, c); }

  const auto& thumb_color() const { return unsafe_get(&slot::thumb_color); }
  void thumb_color(const color& c) { safe_set(&slot::thumb_color, c); }

  const auto& thumb_border_color() const { return unsafe_get(&slot::thumb_border_color); }
  void thumb_border_color(const color& c) { safe_set(&slot::thumb_border_color, c); }

  float thumb_border_width() const { return unsafe_get(&slot::thumb_border_width); }
  void thumb_border_width(float value) { safe_set(&slot::thumb_border_width, value); }

  float track_thickness() const { return unsafe_get(&slot::track_thickness); }
  void track_thickness(float value) { safe_set_size(&slot::track_thickness, yw::max(value, 1.0f)); }

  float2 thumb_size() const { return unsafe_get(&slot::thumb_size); }
  void thumb_size(float2 value) {
    if (auto csp = system::slot_address<slot>(_id)) {
      csp->thumb_size = vapply_r<float2>(yw::max, value, float2::fill(4.0f));
      csp->make_messy();
    }
  }

  double min_value() const { return unsafe_get(&slot::min_value); }
  void min_value(double v) {
    if (auto csp = system::slot_address<slot>(_id)) {
      csp->min_value = v;
      csp->apply_value(csp->value, false);
      csp->make_dirty();
    }
  }

  double max_value() const { return unsafe_get(&slot::max_value); }
  void max_value(double v) {
    if (auto csp = system::slot_address<slot>(_id)) {
      csp->max_value = v;
      csp->apply_value(csp->value, false);
      csp->make_dirty();
    }
  }

  void range(double min_v, double max_v) {
    if (auto csp = system::slot_address<slot>(_id)) {
      csp->min_value = min_v;
      csp->max_value = max_v;
      csp->apply_value(csp->value, false);
      csp->make_dirty();
    }
  }

  double value() const { return unsafe_get(&slot::value); }
  void value(double v) {
    if (auto csp = system::slot_address<slot>(_id)) csp->apply_value(v);
  }

  double step() const { return unsafe_get(&slot::step); }
  void step(double v) { safe_set(&slot::step, std::fabs(v)); }

  double page_step() const { return unsafe_get(&slot::page_step); }
  void page_step(double v) { safe_set(&slot::page_step, std::fabs(v)); }

  bool vertical() const { return unsafe_get(&slot::vertical); }
  void vertical(bool value) { safe_set_size(&slot::vertical, value); }

  bool wrap() const { return unsafe_get(&slot::wrap); }
  void wrap(bool value) { safe_set(&slot::wrap, value); }

  void increment() {
    if (auto csp = system::slot_address<slot>(_id)) csp->step_by(std::fabs(csp->step));
  }

  void decrement() {
    if (auto csp = system::slot_address<slot>(_id)) csp->step_by(-std::fabs(csp->step));
  }

  const auto& on_button() const { return unsafe_get(&slot::on_button); }
  void on_button(function<void, event::button> f) { safe_set(&slot::on_button, std::move(f)); }

  const auto& on_change() const { return unsafe_get(&slot::on_change); }
  void on_change(function<void, double> f) { safe_set(&slot::on_change, std::move(f)); }

  const auto& on_focus() const { return unsafe_get(&slot::on_focus); }
  void on_focus(function<void, bool> f) { safe_set(&slot::on_focus, std::move(f)); }

  const auto& on_keydown() const { return unsafe_get(&slot::on_keydown); }
  void on_keydown(function<void, event::key> f) { safe_set(&slot::on_keydown, std::move(f)); }

  const auto& on_keyup() const { return unsafe_get(&slot::on_keyup); }
  void on_keyup(function<void, event::key> f) { safe_set(&slot::on_keyup, std::move(f)); }

  const auto& on_move() const { return unsafe_get(&slot::on_move); }
  void on_move(function<void, event::move> f) { safe_set(&slot::on_move, std::move(f)); }

  const auto& on_wheel() const { return unsafe_get(&slot::on_wheel); }
  void on_wheel(function<void, event::wheel> f) { safe_set(&slot::on_wheel, std::move(f)); }
};
} // namespace yw::ui
