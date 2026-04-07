#pragma once
#include "ywx/background.h"
#include "ywx/text.h"
#include "ywx/ui_control.h"
#include <cmath>
#include <iomanip>
#include <sstream>

namespace yw::ui {

class spinbox : public control {
public:
  class slot : public control::slot {
    enum class part : uint8_t { none, decrement, increment };

    static constexpr double epsilon = 1e-9;

    part captured_part = part::none;

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

    double round_value(double v) const noexcept {
      if (precision > 12) return v;
      double scale = 1.0;
      for (unsigned i = 0; i < precision; ++i) scale *= 10.0;
      return std::round(v * scale) / scale;
    }

    std::wstring format_value() const {
      std::wostringstream ss;
      ss << std::fixed << std::setprecision(precision) << value;
      return ss.str();
    }

    void sync_text() {
      text(format_value());
    }

    part hit_part(float2 pt) const noexcept {
      if (pt.x < pos.x || pt.y < pos.y || pt.x > pos.x + size.x || pt.y > pos.y + size.y) return part::none;
      if (pt.x < pos.x + size.x - button_width) return part::none;
      const auto mid_y = pos.y + size.y * 0.5f;
      return pt.y < mid_y ? part::increment : part::decrement;
    }

  public:
    yw::background background = colors::white;
    color border_color = colors::black;
    float border_width = 1.0f;
    float4 padding{8.0f, 6.0f, 8.0f, 6.0f};

    color text_color = colors::black;
    yw::text text = assume(yw::text::create(L"0"));

    color button_color = color(0.93f, 0.93f, 0.93f, 1.0f);
    color button_border_color = color(0.7f, 0.7f, 0.7f, 1.0f);
    color button_symbol_color = colors::black;
    color pressed_button_color = color(0.78f, 0.86f, 1.0f, 1.0f);
    float button_width = 24.0f;

    double value = 0.0;
    double min_value = 0.0;
    double max_value = 100.0;
    double step = 1.0;
    double page_step = 10.0;
    unsigned precision = 0;
    bool wrap = false;

    function<void, event::button> on_button;
    function<void, double> on_change;
    function<void, bool> on_focus;
    function<void, event::key> on_keydown;
    function<void, event::key> on_keyup;
    function<void, event::move> on_move;
    function<void, event::wheel> on_wheel;

    key captured_key{};

    bool apply_value(double next, bool notify = true) {
      normalize_bounds();
      next = round_value(clamp_value(next));
      if (std::fabs(next - value) <= epsilon) return false;
      value = next;
      sync_text();
      make_messy();
      if (notify && on_change) on_change(value);
      return true;
    }

    void step_by(double delta, bool notify = true) {
      apply_value(value + delta, notify);
    }

    virtual float2 calculate_size() const noexcept override {
      const auto inner = text.size() + padding.xy() + padding.zw() + float2(button_width, 0.0f);
      return vapply_r<float2>(yw::max, float2(), min_size, inner, size * constrained);
    }

    virtual void update_size() noexcept override {
      min_size = vapply_r<float2>(yw::max, min_size, float2());
      const auto inner = text.size() + padding.xy() + padding.zw() + float2(button_width, 0.0f);
      size = vapply_r<float2>(yw::max, min_size, inner, size * constrained);
    }

    virtual void draw() const override {
      if (!visible) return;

      draw_background(pos, size, background);

      const auto x_sep = pos.x + size.x - button_width;
      const auto half_h = size.y * 0.5f;
      const auto inc_pos = float2(x_sep, pos.y);
      const auto dec_pos = float2(x_sep, pos.y + half_h);
      const auto btn_size = float2(button_width, half_h);

      brush.color(captured_part == part::increment ? pressed_button_color : button_color);
      fill_rectangle(inc_pos, btn_size);
      brush.color(captured_part == part::decrement ? pressed_button_color : button_color);
      fill_rectangle(dec_pos, btn_size);

      brush.color(button_border_color);
      draw_line(float2(x_sep, pos.y), float2(x_sep, pos.y + size.y), 1.0f);
      draw_line(float2(x_sep, pos.y + half_h), float2(pos.x + size.x, pos.y + half_h), 1.0f);

      brush.color(border_color);
      draw_round_rectangle(pos, size, radius, border_width);

      brush.color(text_color);
      const auto tsz = text.size();
      const auto text_area = float2(size.x - button_width, size.y);
      const auto text_pos = pos + float2(
                               padding.x + (text_area.x - (padding.x + padding.z) - tsz.x) * 0.5f,
                               padding.y + (text_area.y - (padding.y + padding.w) - tsz.y) * 0.5f);
      draw_text(text_pos, text);

      brush.color(button_symbol_color);
      const auto cx = x_sep + button_width * 0.5f;
      const auto cy_up = pos.y + half_h * 0.5f;
      const auto cy_dn = pos.y + half_h + half_h * 0.5f;
      const auto sx = yw::max(3.0f, button_width * 0.2f);
      const auto sy = yw::max(2.0f, half_h * 0.14f);

      draw_line(float2(cx - sx, cy_up + sy), float2(cx, cy_up - sy), 1.5f);
      draw_line(float2(cx, cy_up - sy), float2(cx + sx, cy_up + sy), 1.5f);
      draw_line(float2(cx - sx, cy_dn - sy), float2(cx, cy_dn + sy), 1.5f);
      draw_line(float2(cx, cy_dn + sy), float2(cx + sx, cy_dn - sy), 1.5f);
    }

    virtual void click_event(event::button e) override {
      if (!enabled || e.code != captured_key) {
        captured_key = {};
        captured_part = part::none;
        return;
      }

      if (captured_key == key::lbutton && hit_part(float2(e.pos)) == captured_part) {
        if (captured_part == part::increment) step_by(step);
        else if (captured_part == part::decrement) step_by(-step);
      }

      captured_key = {};
      captured_part = part::none;
    }

    virtual void button_event(event::button e) override {
      if (!enabled) return;
      if (e.down) {
        captured_key = e.code;
        captured_part = hit_part(float2(e.pos));
      } else {
        captured_key = {};
        captured_part = part::none;
      }
      if (on_button) on_button(e);
    }

    virtual bool focus_event(bool focused) override {
      if (!focused) {
        captured_key = {};
        captured_part = part::none;
      }
      if (enabled && on_focus) on_focus(focused);
      return enabled;
    }

    virtual bool key_event(event::key e) override {
      if (!enabled) return false;

      if (e.down) {
        switch (e.code) {
        case key::up: step_by(step); break;
        case key::down: step_by(-step); break;
        case key::page_up: step_by(page_step); break;
        case key::page_down: step_by(-page_step); break;
        case key::home: apply_value(min_value); break;
        case key::end: apply_value(max_value); break;
        default: break;
        }
        if (e.first && on_keydown) on_keydown(e);
        return true;
      } else {
        if (on_keyup) on_keyup(e);
        return true;
      }
    }

    virtual void move_event(event::move e) override {
      if (enabled && on_move) on_move(e);
    }

    virtual void wheel_event(event::wheel e) override {
      if (!enabled || e.horizontal) return;

      const auto amount = e.shift ? page_step : step;
      const int tick = e.delta >= 0 ? 1 : -1;
      step_by(amount * double(tick));

      if (on_wheel) on_wheel(e);
    }
  };

  using control::operator bool;
  spinbox() noexcept = default;
  spinbox(derived_from<unknown> auto& Layout) {
    if (auto res = create_control<spinbox>(Layout)) _id = *res;
  }

  const auto& background() const { return unsafe_get(&slot::background); }
  void background(yw::background bg) { safe_set(&slot::background, std::move(bg)); }

  const auto& border_color() const { return unsafe_get(&slot::border_color); }
  void border_color(const color& c) { safe_set(&slot::border_color, c); }

  const auto& border_width() const { return unsafe_get(&slot::border_width); }
  void border_width(float w) { safe_set(&slot::border_width, w); }

  const auto& padding() const { return unsafe_get(&slot::padding); }
  void padding(const float4& p) { safe_set_size(&slot::padding, p); }

  const auto& text() const { return unsafe_get(&slot::text); }
  auto& text() { return unsafe_get_size(&slot::text); }

  const auto& text_color() const { return unsafe_get(&slot::text_color); }
  void text_color(const color& c) { safe_set(&slot::text_color, c); }

  const auto& button_color() const { return unsafe_get(&slot::button_color); }
  void button_color(const color& c) { safe_set(&slot::button_color, c); }

  const auto& button_border_color() const { return unsafe_get(&slot::button_border_color); }
  void button_border_color(const color& c) { safe_set(&slot::button_border_color, c); }

  const auto& button_symbol_color() const { return unsafe_get(&slot::button_symbol_color); }
  void button_symbol_color(const color& c) { safe_set(&slot::button_symbol_color, c); }

  const auto& pressed_button_color() const { return unsafe_get(&slot::pressed_button_color); }
  void pressed_button_color(const color& c) { safe_set(&slot::pressed_button_color, c); }

  float button_width() const { return unsafe_get(&slot::button_width); }
  void button_width(float value) { safe_set_size(&slot::button_width, yw::max(value, 8.0f)); }

  double value() const { return unsafe_get(&slot::value); }
  void value(double v) {
    if (auto csp = system::slot_address<slot>(_id)) csp->apply_value(v);
  }

  double min_value() const { return unsafe_get(&slot::min_value); }
  void min_value(double v) {
    if (auto csp = system::slot_address<slot>(_id)) {
      csp->min_value = v;
      csp->apply_value(csp->value, false);
      csp->make_messy();
    }
  }

  double max_value() const { return unsafe_get(&slot::max_value); }
  void max_value(double v) {
    if (auto csp = system::slot_address<slot>(_id)) {
      csp->max_value = v;
      csp->apply_value(csp->value, false);
      csp->make_messy();
    }
  }

  void range(double min_v, double max_v) {
    if (auto csp = system::slot_address<slot>(_id)) {
      csp->min_value = min_v;
      csp->max_value = max_v;
      csp->apply_value(csp->value, false);
      csp->make_messy();
    }
  }

  double step() const { return unsafe_get(&slot::step); }
  void step(double v) {
    if (auto csp = system::slot_address<slot>(_id)) {
      csp->step = std::fabs(v);
      csp->make_dirty();
    }
  }

  double page_step() const { return unsafe_get(&slot::page_step); }
  void page_step(double v) {
    if (auto csp = system::slot_address<slot>(_id)) {
      csp->page_step = std::fabs(v);
      csp->make_dirty();
    }
  }

  unsigned precision() const { return unsafe_get(&slot::precision); }
  void precision(unsigned digits) {
    if (auto csp = system::slot_address<slot>(_id)) {
      csp->precision = digits;
      csp->apply_value(csp->value, false);
      csp->make_messy();
    }
  }

  bool wrap() const { return unsafe_get(&slot::wrap); }
  void wrap(bool value) { safe_set(&slot::wrap, value); }

  void increment() {
    if (auto csp = system::slot_address<slot>(_id)) csp->step_by(csp->step);
  }

  void decrement() {
    if (auto csp = system::slot_address<slot>(_id)) csp->step_by(-csp->step);
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
