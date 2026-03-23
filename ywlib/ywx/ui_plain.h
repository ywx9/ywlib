#pragma once
#include "ywx/ui_window.h"
#include "ywx/tooltip.h"

namespace yw::ui {

class plain : public control {
public:
  class slot : public control::slot {
  protected:
    void draw_plain(float2 Pos, float2 Size) const {
      brush.color(bg_color);
      fill_round_rectangle(Pos, Size, radius);
      brush.color(border_color);
      draw_round_rectangle(Pos, Size, radius, border_width);
    }

  public:
    float2 radius{5.0f, 5.0f};
    color bg_color = colors::white;
    color border_color = colors::transparent;
    float1 border_width = 1.0f;
    std::wstring tooltip{};

    function<void, event::button> on_button;
    function<void, event::hover> on_hover;
    function<void, event::wheel> on_wheel;

    virtual slotid hit_test(float2 Pt) const noexcept override {
      if (!visible || Pt.x < last_rect.x || Pt.y < last_rect.y || Pt.x > last_rect.z || Pt.y > last_rect.w) return {};
      return id;
    }

    virtual void draw(float2 Pos, float2 Size) const override {
      update_last_rect(Pos, Size);
      draw_plain(last_rect.xy(), last_rect.zw() - last_rect.xy());
    }

    virtual void draw() const override { draw_plain(last_rect.xy(), last_rect.zw() - last_rect.xy()); }
    virtual float2 get_radius() const noexcept override { return radius; } // for drawing focus ring

    virtual void button_event(event::button Event) override {
      if (enabled && on_button) on_button(Event);
    }

    virtual void hover_event(event::hover Event) override {
      if (enabled && on_hover) on_hover(Event);
      if (tooltip.empty()) return;
      if (Event.move()) {
        if (const auto w = system::slot_address<ui::window>(window_id))
          system::tooltip.show(last_rect.xy() + w->pos + w->margin.xy(), last_rect.zw() - last_rect.xy());
      } else if (Event.enter()) {
        if (const auto w = system::slot_address<ui::window>(window_id))
          system::tooltip.show(last_rect.xy() + w->pos() + w->margin.xy(), last_rect.zw() - last_rect.xy(), tooltip);
      } else if (Event.leave()) system::tooltip.hide();
    }

    virtual void wheel_event(event::wheel Event) override {
      if (enabled && on_wheel) on_wheel(Event);
    }
  };

  plain() noexcept = default;

  plain(derived_from<unknown> auto& Layout) {
    if (auto res = create_control<plain>(Layout)) _id = *res;
  }

  using control::operator bool;

  const float2& radius() const { return unsafe_get(&slot::radius); }
  void radius(float2 value) { safe_set(&slot::radius, value); }

  const color& bg_color() const { return unsafe_get(&slot::bg_color); }
  void bg_color(const color& value) { safe_set(&slot::bg_color, value); }

  const color& border_color() const { return unsafe_get(&slot::border_color); }
  void border_color(const color& value) { safe_set(&slot::border_color, value); }

  const float1& border_width() const { return unsafe_get(&slot::border_width); }
  void border_width(float1 value) { safe_set(&slot::border_width, value); }

  const auto& tooltip() const { return unsafe_get(&slot::tooltip); }
  void tooltip(std::wstring_view value) { safe_set(&slot::tooltip, value); }

  const auto& on_button() const { return unsafe_get(&slot::on_button); }
  void on_button(function<void, event::button> value) { safe_set(&slot::on_button, std::move(value)); }

  const auto& on_hover() const { return unsafe_get(&slot::on_hover); }
  void on_hover(function<void, event::hover> value) { safe_set(&slot::on_hover, std::move(value)); }

  const auto& on_wheel() const { return unsafe_get(&slot::on_wheel); }
  void on_wheel(function<void, event::wheel> value) { safe_set(&slot::on_wheel, std::move(value)); }
};
} // namespace yw::ui
