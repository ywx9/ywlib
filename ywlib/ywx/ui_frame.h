#pragma once
#include "ywx/window.h"

namespace yw::ui {

//////////////////////////////////////// MARK: ui::frame

class frame : public base {
public:
  class slot : public base::slot {
  public:
    std::wstring tooltip{};
    float2 radius{};
    color bg_color = colors::white;
    color border_color = colors::black;
    float border_width = 1.0f;

    function<void, event::button> on_button;
    function<void, event::hover> on_hover;
    function<void, event::move> on_move;
    function<void, event::key> on_key;
    function<void, bool> on_focus;
    function<void, event::button> on_click;

    virtual void draw() const override {
      fill_round_rectangle(pos, size, radius, bg_color);
      draw_round_rectangle(pos, size, radius, border_color, border_width);
    }

    virtual void draw_focus() const override {
      constexpr float2 margin = {2.5f, 2.5f};
      const auto focus_pos = pos - margin;
      const auto focus_size = size + margin * 2.0f;
      const auto focus_radius = radius + margin;
      draw_round_rectangle(focus_pos, focus_size, focus_radius, border_color, 1.0f, d2d.dashed_stroke_style());
    }

    virtual void button_event(event::button e) override {
      if (is_enabled() && on_button) on_button(e);
    }

    virtual void hover_event(event::hover e) override {
      if (is_enabled() && on_hover) on_hover(e);
      if (tooltip.empty()) return;
      if (e.move()) {
        if (const auto w = system::windows.get(window_id)) system::tooltip.show(pos + w->pos() + w->margin.xy(), size);
      } else if (e.enter()) {
        if (const auto w = system::windows.get(window_id))
          system::tooltip.show(pos + w->pos() + w->margin.xy(), size, tooltip);
      } else if (e.leave()) system::tooltip.hide();
    }

    virtual void move_event(event::move e) override {
      if (is_focusable() && on_move) on_move(e);
    }

    virtual void key_event(event::key e) override {
      if (is_focusable() && on_key) on_key(e);
    }

    virtual bool focus_event(bool focused) override {
      if (is_focusable() && on_focus) on_focus(focused);
      return is_focusable();
    }

    virtual void click_event(event::button e) {
      if (is_enabled() && on_click) on_click(e);
    }
  };

protected:
  using base::base;
  slot* _ui_slot() const noexcept { return dynamic_cast<slot*>(base::_ui_slot()); }

public:
  using base::operator bool;

  const auto& tooltip() const { return unsafe_get(&slot::tooltip); }
  const auto& radius() const { return unsafe_get(&slot::radius); }
  const auto& bg_color() const { return unsafe_get(&slot::bg_color); }
  const auto& border_color() const { return unsafe_get(&slot::border_color); }
  const auto& border_width() const { return unsafe_get(&slot::border_width); }
  const auto& on_button() const { return unsafe_get(&slot::on_button); }
  const auto& on_hover() const { return unsafe_get(&slot::on_hover); }
  const auto& on_move() const { return unsafe_get(&slot::on_move); }
  const auto& on_key() const { return unsafe_get(&slot::on_key); }
  const auto& on_focus() const { return unsafe_get(&slot::on_focus); }
  const auto& on_click() const { return unsafe_get(&slot::on_click); }

  template<stringable S> void tooltip(S&& s) { _set(&slot::tooltip, std::forward<S>(s)); }
  void radius(float2 value) { _set(&slot::radius, value); }
  void bg_color(const color& value) { _set(&slot::bg_color, value); }
  void border_color(const color& value) { _set(&slot::border_color, value); }
  void border_width(float value) { _set(&slot::border_width, value); }
  void on_button(function<void, event::button> f) { _set(&slot::on_button, std::move(f)); }
  void on_hover(function<void, event::hover> f) { _set(&slot::on_hover, std::move(f)); }
  void on_move(function<void, event::move> f) { _set(&slot::on_move, std::move(f)); }
  void on_key(function<void, event::key> f) { _set(&slot::on_key, std::move(f)); }
  void on_focus(function<void, bool> f) { _set(&slot::on_focus, std::move(f)); }
  void on_click(function<void, event::button> f) { _set(&slot::on_click, std::move(f)); }

  template<included_in<window&, none> Window>
  static std::expected<frame, error_trace> add(Window&& w, float2 Pos, float2 Size) {
    if (auto res = base::add<frame>(w, Pos, Size)) return std::move(yw::get<0>(*res));
    else return unexpected_error(res.error());
  }
};
} // namespace yw::ui
