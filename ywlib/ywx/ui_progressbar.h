#pragma once
#include "ywx/background.h"
#include "ywx/text.h"
#include "ywx/ui_control.h"
#include <cmath>
#include <iomanip>
#include <sstream>

namespace yw::ui {

class progressbar : public control {
public:
  class slot : public control::slot {
    static constexpr double epsilon = 1e-9;

    void normalize_bounds() noexcept {
      if (min_value > max_value) std::swap(min_value, max_value);
    }

    double clamp_value(double v) const noexcept {
      return std::clamp(v, min_value, max_value);
    }

    double ratio() const noexcept {
      const auto range = max_value - min_value;
      if (range <= epsilon) return 0.0;
      return std::clamp((value - min_value) / range, 0.0, 1.0);
    }

  public:
    void update_text_label() {
      if (!show_text) return;
      std::wostringstream ss;
      ss << std::fixed << std::setprecision(precision) << ratio() * 100.0 << L"%";
      text = ss.str();
    }

  public:
    yw::background background = colors::white;
    color border_color = colors::black;
    float border_width = 1.0f;
    float4 padding{4.0f, 4.0f, 4.0f, 4.0f};

    color track_color = color(0.86f, 0.86f, 0.86f, 1.0f);
    color fill_color = color(0.20f, 0.55f, 0.95f, 1.0f);

    bool show_text = true;
    unsigned precision = 0;
    color text_color = colors::black;
    yw::text text = assume(yw::text::create(L"0%"));

    double min_value = 0.0;
    double max_value = 100.0;
    double value = 0.0;

    function<void, double> on_change;

    bool apply_value(double v, bool notify = true) {
      normalize_bounds();
      v = clamp_value(v);
      if (std::fabs(v - value) <= epsilon) return false;
      value = v;
      update_text_label();
      make_dirty();
      if (notify && on_change) on_change(value);
      return true;
    }

    virtual float2 calculate_size() const noexcept override {
      const float2 prefer{160.0f, 28.0f};
      const auto inner = prefer + padding.xy() + padding.zw();
      return vapply_r<float2>(yw::max, float2(), min_size, inner, size * constrained);
    }

    virtual void update_size() noexcept override {
      min_size = vapply_r<float2>(yw::max, min_size, float2());
      const float2 prefer{160.0f, 28.0f};
      const auto inner = prefer + padding.xy() + padding.zw();
      size = vapply_r<float2>(yw::max, min_size, inner, size * constrained);
    }

    virtual void draw() const override {
      if (!visible) return;

      draw_background(pos, size, background);
      brush.color(border_color);
      draw_round_rectangle(pos, size, radius, border_width);

      const auto cp = pos + padding.xy();
      const auto cs = vapply_r<float2>(yw::max, float2(), size - padding.xy() - padding.zw());

      brush.color(track_color);
      fill_rectangle(cp, cs);

      const auto fw = float(cs.x * ratio());
      if (fw > 0.0f) {
        brush.color(fill_color);
        fill_rectangle(cp, float2(fw, cs.y));
      }

      if (show_text) {
        brush.color(text_color);
        const auto tsz = text.size();
        draw_text(cp + (cs - tsz) * 0.5f, text);
      }
    }
  };

  using control::operator bool;
  progressbar() noexcept = default;
  progressbar(derived_from<unknown> auto& Layout) {
    if (auto res = create_control<progressbar>(Layout)) _id = *res;
  }

  const auto& background() const { return unsafe_get(&slot::background); }
  void background(yw::background bg) { safe_set(&slot::background, std::move(bg)); }

  const auto& border_color() const { return unsafe_get(&slot::border_color); }
  void border_color(const color& c) { safe_set(&slot::border_color, c); }

  const auto& border_width() const { return unsafe_get(&slot::border_width); }
  void border_width(float value) { safe_set(&slot::border_width, value); }

  const auto& padding() const { return unsafe_get(&slot::padding); }
  void padding(const float4& value) { safe_set_size(&slot::padding, value); }

  const auto& track_color() const { return unsafe_get(&slot::track_color); }
  void track_color(const color& value) { safe_set(&slot::track_color, value); }

  const auto& fill_color() const { return unsafe_get(&slot::fill_color); }
  void fill_color(const color& value) { safe_set(&slot::fill_color, value); }

  bool show_text() const { return unsafe_get(&slot::show_text); }
  void show_text(bool value) {
    if (auto csp = system::slot_address<slot>(_id)) {
      csp->show_text = value;
      csp->update_text_label();
      csp->make_dirty();
    }
  }

  unsigned precision() const { return unsafe_get(&slot::precision); }
  void precision(unsigned digits) {
    if (auto csp = system::slot_address<slot>(_id)) {
      csp->precision = digits;
      csp->update_text_label();
      csp->make_dirty();
    }
  }

  const auto& text_color() const { return unsafe_get(&slot::text_color); }
  void text_color(const color& value) { safe_set(&slot::text_color, value); }

  double min_value() const { return unsafe_get(&slot::min_value); }
  void min_value(double v) {
    if (auto csp = system::slot_address<slot>(_id)) {
      csp->min_value = v;
      csp->apply_value(csp->value, false);
      csp->update_text_label();
      csp->make_dirty();
    }
  }

  double max_value() const { return unsafe_get(&slot::max_value); }
  void max_value(double v) {
    if (auto csp = system::slot_address<slot>(_id)) {
      csp->max_value = v;
      csp->apply_value(csp->value, false);
      csp->update_text_label();
      csp->make_dirty();
    }
  }

  void range(double min_v, double max_v) {
    if (auto csp = system::slot_address<slot>(_id)) {
      csp->min_value = min_v;
      csp->max_value = max_v;
      csp->apply_value(csp->value, false);
      csp->update_text_label();
      csp->make_dirty();
    }
  }

  double value() const { return unsafe_get(&slot::value); }
  void value(double v) {
    if (auto csp = system::slot_address<slot>(_id)) csp->apply_value(v);
  }

  const auto& on_change() const { return unsafe_get(&slot::on_change); }
  void on_change(function<void, double> f) { safe_set(&slot::on_change, std::move(f)); }
};
} // namespace yw::ui
