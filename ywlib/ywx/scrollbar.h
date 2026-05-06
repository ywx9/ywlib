#pragma once
#include "ywx/core.h"
#include "ywx/event.h"

namespace yw {

/// スクロールバー (UI部品)
class scrollbar {
public:
  enum class orientation : uint8_t { horizontal, vertical };
  enum class part : uint8_t { none, decrement_button, increment_button, track, thumb };

private:
  static constexpr double epsilon = 1e-9;

  orientation _orientation = orientation::vertical;
  bool _enabled = true;
  bool _visible = true;
  bool _auto_hide = true;

  float2 _pos{};
  float2 _size{};

  double _min_value = 0.0;
  double _max_value = 0.0;
  double _value = 0.0;
  double _step = 32.0;
  double _page_step = 0.0;

  float _button_extent = 14.0f;
  float _min_thumb_extent = 18.0f;

  color _track_color = color(0.88f, 0.88f, 0.88f, 1.0f);
  color _thumb_color = color(0.58f, 0.58f, 0.58f, 1.0f);
  color _active_color = color(0.36f, 0.58f, 0.96f, 1.0f);
  color _border_color = color(0.20f, 0.20f, 0.20f, 1.0f);
  float _border_width = 1.0f;
  float _radius = 3.0f;

  part _hot_part = part::none;
  part _captured_part = part::none;
  double _drag_grab_axis = 0.0;

  static double clampd(double v, double lo, double hi) {
    if (lo > hi) std::swap(lo, hi);
    return std::clamp(v, lo, hi);
  }

  bool vertical() const noexcept { return _orientation == orientation::vertical; }
  float axis_size(const float2& sz) const noexcept { return vertical() ? sz.y : sz.x; }
  float cross_size(const float2& sz) const noexcept { return vertical() ? sz.x : sz.y; }
  float axis_pos(const float2& pt) const noexcept { return vertical() ? pt.y : pt.x; }

  float2 make_point(float axis, float cross) const noexcept {
    return vertical() ? float2(cross, axis) : float2(axis, cross);
  }

  float2 make_size(float axis, float cross) const noexcept {
    return vertical() ? float2(cross, axis) : float2(axis, cross);
  }

  bool point_inside(float2 pt, float2 pos, float2 size) const noexcept {
    return pt.x >= pos.x && pt.y >= pos.y && pt.x <= pos.x + size.x && pt.y <= pos.y + size.y;
  }

  float track_axis_begin() const noexcept { return axis_pos(_pos) + button_extent(); }

  float track_axis_length() const noexcept {
    const auto len = axis_size(_size) - button_extent() * 2.0f;
    return yw::max(0.0f, len);
  }

  double range_length() const noexcept { return yw::max(0.0, _max_value - _min_value); }

  bool scrollable() const noexcept { return range_length() > epsilon; }

  float thumb_axis_length() const noexcept {
    const auto track_len = track_axis_length();
    if (track_len <= 0.0f) return 0.0f;
    if (!scrollable()) return track_len;
    const auto content = range_length() + page_size();
    if (content <= epsilon) return track_len;
    const auto raw = float(double(track_len) * (page_size() / content));
    return std::clamp(raw, _min_thumb_extent, track_len);
  }

  float thumb_axis_offset() const noexcept {
    const auto movable = track_axis_length() - thumb_axis_length();
    if (movable <= 0.0f || !scrollable()) return 0.0f;
    const auto t = (_value - _min_value) / range_length();
    return float(std::clamp(t, 0.0, 1.0) * double(movable));
  }

  float2 track_pos() const noexcept {
    const auto a = track_axis_begin();
    const auto c = vertical() ? _pos.x : _pos.y;
    return make_point(a, c);
  }

  float2 track_size() const noexcept { return make_size(track_axis_length(), cross_size(_size)); }

  float2 thumb_pos() const noexcept {
    const auto a = track_axis_begin() + thumb_axis_offset();
    const auto c = vertical() ? _pos.x : _pos.y;
    return make_point(a, c);
  }

  float2 thumb_size() const noexcept { return make_size(thumb_axis_length(), cross_size(_size)); }

  float2 decrement_pos() const noexcept {
    const auto a = axis_pos(_pos);
    const auto c = vertical() ? _pos.x : _pos.y;
    return make_point(a, c);
  }

  float2 increment_pos() const noexcept {
    const auto a = axis_pos(_pos) + axis_size(_size) - button_extent();
    const auto c = vertical() ? _pos.x : _pos.y;
    return make_point(a, c);
  }

  float2 button_size() const noexcept { return make_size(button_extent(), cross_size(_size)); }

  double value_from_axis(float axis_value, double grab) const noexcept {
    const auto t_begin = track_axis_begin();
    const auto movable = track_axis_length() - thumb_axis_length();
    if (movable <= 0.0f || !scrollable()) return _min_value;
    const auto off = std::clamp(double(axis_value - t_begin) - grab, 0.0, double(movable));
    const auto t = off / double(movable);
    return _min_value + range_length() * t;
  }

  bool set_value_internal(double next, bool notify) {
    if (_min_value > _max_value) std::swap(_min_value, _max_value);
    next = clampd(next, _min_value, _max_value);
    if (std::fabs(next - _value) <= epsilon) return false;
    _value = next;
    if (notify && on_change) on_change(_value);
    return true;
  }

public:
  function<void, double> on_change;

  scrollbar() = default;
  explicit scrollbar(orientation o) : _orientation(o) {}

  orientation direction() const noexcept { return _orientation; }
  void direction(orientation o) noexcept { _orientation = o; }

  bool enabled() const noexcept { return _enabled; }
  void enabled(bool v) noexcept { _enabled = v; }

  bool visible() const noexcept { return _visible && (!_auto_hide || scrollable()); }
  void visible(bool v) noexcept { _visible = v; }

  bool auto_hide() const noexcept { return _auto_hide; }
  void auto_hide(bool v) noexcept { _auto_hide = v; }

  float2 pos() const noexcept { return _pos; }
  float2 size() const noexcept { return _size; }
  void rect(float2 p, float2 s) noexcept {
    _pos = p;
    _size = vapply_r<float2>(yw::max, s, float2());
  }

  double min_value() const noexcept { return _min_value; }
  double max_value() const noexcept { return _max_value; }
  void range(double min_v, double max_v) {
    _min_value = min_v;
    _max_value = max_v;
    if (_min_value > _max_value) std::swap(_min_value, _max_value);
    set_value_internal(_value, false);
  }

  double value() const noexcept { return _value; }
  bool value(double v, bool notify = true) { return set_value_internal(v, notify); }

  double page_size() const noexcept { return yw::max(0.0, _page_step); }
  void page_size(double v) noexcept { _page_step = yw::max(0.0, v); }

  double step() const noexcept { return yw::max(0.0, _step); }
  void step(double v) noexcept { _step = yw::max(0.0, v); }

  void content(double content_extent, double viewport_extent, double offset) {
    const auto content = yw::max(0.0, content_extent);
    const auto view = yw::max(0.0, viewport_extent);
    const auto max_off = yw::max(0.0, content - view);
    range(0.0, max_off);
    page_size(view);
    value(offset, false);
  }

  float button_extent() const noexcept { return yw::max(0.0f, _button_extent); }
  void button_extent(float v) noexcept { _button_extent = yw::max(0.0f, v); }

  float min_thumb_extent() const noexcept { return yw::max(1.0f, _min_thumb_extent); }
  void min_thumb_extent(float v) noexcept { _min_thumb_extent = yw::max(1.0f, v); }

  const color& track_color() const noexcept { return _track_color; }
  void track_color(const color& v) noexcept { _track_color = v; }

  const color& thumb_color() const noexcept { return _thumb_color; }
  void thumb_color(const color& v) noexcept { _thumb_color = v; }

  const color& active_color() const noexcept { return _active_color; }
  void active_color(const color& v) noexcept { _active_color = v; }

  const color& border_color() const noexcept { return _border_color; }
  void border_color(const color& v) noexcept { _border_color = v; }

  float border_width() const noexcept { return _border_width; }
  void border_width(float v) noexcept { _border_width = yw::max(0.0f, v); }

  float radius() const noexcept { return _radius; }
  void radius(float v) noexcept { _radius = yw::max(0.0f, v); }

  part hot_part() const noexcept { return _hot_part; }
  part captured_part() const noexcept { return _captured_part; }

  part hit_test(float2 pt) const noexcept {
    if (!visible() || !point_inside(pt, _pos, _size)) return part::none;
    if (point_inside(pt, decrement_pos(), button_size())) return part::decrement_button;
    if (point_inside(pt, increment_pos(), button_size())) return part::increment_button;
    if (point_inside(pt, thumb_pos(), thumb_size())) return part::thumb;
    if (point_inside(pt, track_pos(), track_size())) return part::track;
    return part::none;
  }

  bool move_event(event::move e) {
    if (!visible()) return false;
    _hot_part = hit_test(float2(e.pos));
    return _hot_part != part::none;
  }

  bool button_event(event::button e) {
    if (!enabled() || !visible()) return false;

    const auto pt = float2(e.pos);
    if (e.down) {
      _captured_part = hit_test(pt);
      if (_captured_part == part::none) return false;
      const bool at_min = _value <= _min_value + epsilon;
      const bool at_max = _value >= _max_value - epsilon;
      if (_captured_part == part::thumb) {
        _drag_grab_axis = double(axis_pos(pt) - axis_pos(thumb_pos()));
      } else if (_captured_part == part::decrement_button) {
        if (!at_min) set_value_internal(_value - step(), true);
      } else if (_captured_part == part::increment_button) {
        if (!at_max) set_value_internal(_value + step(), true);
      } else if (_captured_part == part::track) {
        if (axis_pos(pt) < axis_pos(thumb_pos())) set_value_internal(_value - page_size(), true);
        else set_value_internal(_value + page_size(), true);
      }
      return true;
    }

    const bool handled = _captured_part != part::none;
    _captured_part = part::none;
    _drag_grab_axis = 0.0;
    return handled;
  }

  bool drag_event(event::drag e) {
    if (!enabled() || !visible() || _captured_part != part::thumb) return false;
    // event::drag currently transports cursor position in delta.
    const auto pt = float2(e.delta);
    set_value_internal(value_from_axis(axis_pos(pt), _drag_grab_axis), true);
    return true;
  }

  bool wheel_event(event::wheel e) {
    if (!enabled() || !visible()) return false;
    if (vertical() && e.horizontal) return false;
    if (!vertical() && !e.horizontal) return false;
    if (hit_test(float2(e.pos)) == part::none) return false;
    const auto amount = e.shift ? page_size() : step();
    const auto dir = e.delta >= 0 ? -1.0 : 1.0;
    return set_value_internal(_value + amount * dir, true);
  }

  void draw() const {
    if (!visible()) return;

    const auto bsz = button_size();
    const auto bpos_dec = decrement_pos();
    const auto bpos_inc = increment_pos();
    const auto tpos = track_pos();
    const auto tsz = track_size();
    const auto thp = thumb_pos();
    const auto ths = thumb_size();

    brush.color(_track_color);
    fill_round_rectangle(tpos, tsz, float2::fill(_radius));

    const bool at_min = _value <= _min_value + epsilon;
    const bool at_max = _value >= _max_value - epsilon;

    auto btn_dec_color = _border_color;
    auto btn_inc_color = _border_color;
    if (at_min) btn_dec_color = color(0.55f, 0.55f, 0.55f, 1.0f);
    if (at_max) btn_inc_color = color(0.55f, 0.55f, 0.55f, 1.0f);
    if (_captured_part == part::decrement_button || _hot_part == part::decrement_button) btn_dec_color = _active_color;
    if (_captured_part == part::increment_button || _hot_part == part::increment_button) btn_inc_color = _active_color;

    const float cx_dec = bpos_dec.x + bsz.x * 0.5f;
    const float cy_dec = bpos_dec.y + bsz.y * 0.5f;
    const float cx_inc = bpos_inc.x + bsz.x * 0.5f;
    const float cy_inc = bpos_inc.y + bsz.y * 0.5f;
    const float icon_span = yw::max(2.0f, yw::min(bsz.x, bsz.y) * 0.22f);
    const float icon_half = icon_span * 0.5f;
    const float icon_w = 1.5f;

    if (vertical()) {
      brush.color(btn_dec_color);
      draw_line(float2(cx_dec - icon_span, cy_dec + icon_half), float2(cx_dec, cy_dec - icon_half), icon_w);
      draw_line(float2(cx_dec, cy_dec - icon_half), float2(cx_dec + icon_span, cy_dec + icon_half), icon_w);

      brush.color(btn_inc_color);
      draw_line(float2(cx_inc - icon_span, cy_inc - icon_half), float2(cx_inc, cy_inc + icon_half), icon_w);
      draw_line(float2(cx_inc, cy_inc + icon_half), float2(cx_inc + icon_span, cy_inc - icon_half), icon_w);
    } else {
      brush.color(btn_dec_color);
      draw_line(float2(cx_dec + icon_half, cy_dec - icon_span), float2(cx_dec - icon_half, cy_dec), icon_w);
      draw_line(float2(cx_dec - icon_half, cy_dec), float2(cx_dec + icon_half, cy_dec + icon_span), icon_w);

      brush.color(btn_inc_color);
      draw_line(float2(cx_inc - icon_half, cy_inc - icon_span), float2(cx_inc + icon_half, cy_inc), icon_w);
      draw_line(float2(cx_inc + icon_half, cy_inc), float2(cx_inc - icon_half, cy_inc + icon_span), icon_w);
    }

    brush.color((_captured_part == part::thumb || _hot_part == part::thumb) ? _active_color : _thumb_color);
    fill_round_rectangle(thp, ths, float2::fill(_radius));

    if (_border_width > 0.0f) {
      brush.color(_border_color);
      draw_round_rectangle(_pos, _size, float2::fill(_radius), _border_width);
    }
  }
};
} // namespace yw
