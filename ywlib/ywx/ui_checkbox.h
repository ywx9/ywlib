#pragma once
#include "ywx/icon.h"
#include "ywx/text.h"
#include "ywx/ui_control.h"

namespace yw::ui {

class checkbox : public control {
  static constexpr float2 default_icon_size = {20.0f, 20.0f};
  static constexpr std::string_view box_path = "M 10 0 A 10 10 0 1 1 9.999 0 Z";
  static constexpr std::string_view mark_path = "M 10 6 A 6 6 0 1 1 9.999 6 Z";

public:
  class slot : public control::slot {
  public:
    yw::background background = colors::white;
    color border_color = colors::black;
    float border_width = 1.0f;
    float4 padding = float4::fill(10.0f);

    yw::icon box = assume(svgpath::create(default_icon_size, box_path));
    color box_fill_color = colors::transparent;
    color box_stroke_color = colors::black;
    float box_stroke_width = 1.0f;

    yw::icon mark = assume(svgpath::create(default_icon_size, mark_path));
    color mark_fill_color = colors::black;
    color mark_stroke_color = colors::black;
    float mark_stroke_width = 1.0f;

    float2 icon_size = default_icon_size;
    float icon_offset = 5.0f;
    bool checked{};

    yw::text text = assume(yw::text::create(L""));
    color text_color = colors::black;

    function<void, event::button> on_button;
    function<void, bool> on_change;
    function<void, key> on_click;
    function<void, bool> on_focus;
    function<void, event::key> on_key;
    function<void, event::move> on_move;
    function<void, event::wheel> on_wheel;

    key captured_key{};

    virtual void update_size() noexcept override {
      min_size = vapply_r<float2>(yw::max, min_size, float2());
      const auto tsz = text.size();
      const auto inner_x = icon_size.x + icon_offset + tsz.x + padding.x + padding.z;
      const auto inner = float2(inner_x, yw::max(icon_size.y, tsz.y) + padding.y + padding.w);
      size = vapply_r<float2>(yw::max, min_size, inner, size * constrained);
    }

    virtual void draw() const override {
      if (!visible) return;
      draw_background(pos, size, background);
      brush.color(border_color);
      draw_round_rectangle(pos, size, radius, border_width);

      const auto tsz = text.size();
      const auto content_h = yw::max(icon_size.y, tsz.y);
      const auto icon_pos = float2(pos.x + padding.x, pos.y + (size.y - icon_size.y) * 0.5f);
      const auto text_pos = float2(icon_pos.x + icon_size.x + icon_offset, pos.y + (size.y - tsz.y) * 0.5f);

      brush.color(box_fill_color);
      draw_icon(icon_pos, icon_size, box);
      brush.color(box_stroke_color);
      stroke_icon(icon_pos, icon_size, box);
      if (checked) {
        brush.color(mark_fill_color);
        draw_icon(icon_pos, icon_size, mark);
        brush.color(mark_stroke_color);
        stroke_icon(icon_pos, icon_size, mark);
      }
      brush.color(text_color);
      draw_text(text_pos, text);
    }

    virtual void click_event(event::button e) override {
      if (enabled && e.code == captured_key) click_action();
      captured_key = {};
    }

    virtual void button_event(event::button e) override {
      if (!enabled) return;
      if (e.down) captured_key = e.code;
      else captured_key = {};
      if (on_button) on_button(e);
    }

    virtual bool focus_event(bool focused) override {
      if (!focused) captured_key = {};
      if (enabled && on_focus) on_focus(focused);
      return enabled;
    }

    virtual void key_event(event::key e) override {
      if (!enabled) return;
      if (e.first) {
        if (e.down) captured_key = e.code;
        else if (e.code == captured_key) click_action();
        else captured_key = {};
      }
      if (on_key) on_key(e);
    }

    virtual void wheel_event(event::wheel Event) override {
      if (enabled && on_wheel) on_wheel(Event);
    }

    virtual void click_action() {
      switch (captured_key) {
      case key::lbutton:
      case key::enter:
      case key::space:
        checked = !checked;
        if (on_click) on_click(captured_key);
        if (on_change) on_change(checked);
      }
    }
  };

  using control::operator bool;
  checkbox() noexcept = default;
  checkbox(derived_from<unknown> auto& Layout) {
    if (auto res = create_control<checkbox>(Layout)) _id = *res;
  }

  const auto& background() const { return unsafe_get(&slot::background); }
  void background(yw::background bg) { safe_set(&slot::background, std::move(bg)); }

  const auto& border_color() const { return unsafe_get(&slot::border_color); }
  void border_color(const color& c) { safe_set(&slot::border_color, c); }

  const auto& border_width() const { return unsafe_get(&slot::border_width); }
  void border_width(float w) { safe_set(&slot::border_width, w); }

  const auto& padding() const { return unsafe_get(&slot::padding); }
  void padding(const float4& p) { safe_set_size(&slot::padding, p); }

  const auto& box() const { return unsafe_get(&slot::box); }
  auto& box() { return unsafe_get_size(&slot::box); }
  void box(icon b) { safe_set(&slot::box, std::move(b)); }

  const auto& box_fill_color() const { return unsafe_get(&slot::box_fill_color); }
  void box_fill_color(const color& c) { safe_set(&slot::box_fill_color, c); }

  const auto& box_stroke_color() const { return unsafe_get(&slot::box_stroke_color); }
  void box_stroke_color(const color& c) { safe_set(&slot::box_stroke_color, c); }

  const auto& box_stroke_width() const { return unsafe_get(&slot::box_stroke_width); }
  void box_stroke_width(float w) { safe_set(&slot::box_stroke_width, w); }

  const auto& mark() const { return unsafe_get(&slot::mark); }
  auto& mark() { return unsafe_get_size(&slot::mark); }
  void mark(icon b) { safe_set(&slot::mark, std::move(b)); }

  const auto& mark_fill_color() const { return unsafe_get(&slot::mark_fill_color); }
  void mark_fill_color(const color& c) { safe_set(&slot::mark_fill_color, c); }

  const auto& mark_stroke_color() const { return unsafe_get(&slot::mark_stroke_color); }
  void mark_stroke_color(const color& c) { safe_set(&slot::mark_stroke_color, c); }

  const auto& mark_stroke_width() const { return unsafe_get(&slot::mark_stroke_width); }
  void mark_stroke_width(float1 w) { safe_set(&slot::mark_stroke_width, w.x); }

  const auto& icon_size() const { return unsafe_get(&slot::icon_size); }
  void icon_size(float2 size) { safe_set_size(&slot::icon_size, size); }

  const auto& icon_offset() const { return unsafe_get(&slot::icon_offset); }
  void icon_offset(float1 offset) { safe_set_size(&slot::icon_offset, offset.x); }

  const auto& checked() const { return unsafe_get(&slot::checked); }
  void checked(bool c) { safe_set(&slot::checked, c); }

  const auto& text() const { return unsafe_get(&slot::text); }
  auto& text() { return unsafe_get_size(&slot::text); }
  void text(yw::text t) { safe_set_size(&slot::text, std::move(t)); }
  template<stringable S> void text(S&& t) { safe_set_size(&slot::text, static_cast<S&&>(t)); }

  const auto& text_color() const { return unsafe_get(&slot::text_color); }
  void text_color(const color& c) { safe_set(&slot::text_color, c); }

  const auto& on_button() const { return unsafe_get(&slot::on_button); }
  void on_button(function<void, event::button> f) { safe_set(&slot::on_button, std::move(f)); }

  const auto& on_change() const { return unsafe_get(&slot::on_change); }
  void on_change(function<void, bool> f) { safe_set(&slot::on_change, std::move(f)); }

  const auto& on_click() const { return unsafe_get(&slot::on_click); }
  void on_click(function<void, key> f) { safe_set(&slot::on_click, std::move(f)); }

  const auto& on_focus() const { return unsafe_get(&slot::on_focus); }
  void on_focus(function<void, bool> f) { safe_set(&slot::on_focus, std::move(f)); }

  const auto& on_key() const { return unsafe_get(&slot::on_key); }
  void on_key(function<void, event::key> f) { safe_set(&slot::on_key, std::move(f)); }

  const auto& on_move() const { return unsafe_get(&slot::on_move); }
  void on_move(function<void, event::move> f) { safe_set(&slot::on_move, std::move(f)); }

  const auto& on_wheel() const { return unsafe_get(&slot::on_wheel); }
  void on_wheel(function<void, event::wheel> f) { safe_set(&slot::on_wheel, std::move(f)); }
};
} // namespace yw::ui
