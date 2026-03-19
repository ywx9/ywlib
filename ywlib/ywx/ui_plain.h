#pragma once
#include "ywx/ui_control.h"

namespace yw::ui {

// class layout;

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

    virtual void draw() const override {
      draw_plain(last_rect.xy(), last_rect.zw() - last_rect.xy());
    }

    virtual void button_event(event::button Event) override {
      if (enabled && on_button) on_button(Event);
    }

    virtual void hover_event(event::hover Event) override {
      if (enabled && on_hover) on_hover(Event);
    }

    virtual void wheel_event(event::wheel Event) override {
      if (enabled && on_wheel) on_wheel(Event);
    }
  };

  plain() noexcept = default;

  // plain(layout& Layout);

  // plain(unknown& Layout) {
  plain(derived_from<unknown> auto& Layout) {
    const auto cid = system::uis.add(std::make_unique<slot>());
    const auto csp = system::slot_address<plain>(cid);
    if (!csp) throw unexpected_error(errors::operation_failed, "Failed to create plain slot");
    csp->id = cid;
    const auto lid = Layout.id();
    const auto lsp = system::uis.get(lid);
    if (!lsp) throw unexpected_error(errors::operation_failed, "Failed to get layout slot");
    if (!lsp->attach(cid)) {
      system::uis.erase(cid);
      throw unexpected_error(errors::operation_failed, "Failed to attach plain slot to layout");
    } else _id = cid;
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

  const auto& on_button() const { return unsafe_get(&slot::on_button); }
  void on_button(function<void, event::button> value) { safe_set(&slot::on_button, std::move(value)); }

  const auto& on_hover() const { return unsafe_get(&slot::on_hover); }
  void on_hover(function<void, event::hover> value) { safe_set(&slot::on_hover, std::move(value)); }

  const auto& on_wheel() const { return unsafe_get(&slot::on_wheel); }
  void on_wheel(function<void, event::wheel> value) { safe_set(&slot::on_wheel, std::move(value)); }
};
} // namespace yw::ui
