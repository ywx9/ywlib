#pragma once
#include "ywx/ui_frame.h"
#include "ywx/ui_label.h"

namespace yw::ui {

class button : public frame {
public:
  class slot : public frame::slot {
  public:
    mutable label::part text;
    function<void> on_click;

    virtual void draw(float2 Pos, float2 Size) const override {
      frame::slot::draw(Pos, Size);
      text.size(last_rect.zw() - last_rect.xy());
      text.draw(last_rect.xy());
    }

    virtual void draw() const override {
      frame::slot::draw();
      text.draw(last_rect.xy());
    }

    virtual void click_event(event::button Event) override {
      if (enabled && on_click && Event.code == key::lbutton && !Event.down) on_click();
      frame::slot::click_event(Event);
    }
  };

  button() noexcept = default;

  button(derived_from<unknown> auto& Layout) {
    if (auto res = create_control<button>(Layout)) _id = *res;
  }

  using frame::operator bool;

  auto& text() { return unsafe_get(&slot::text); }
  const auto& text() const { return unsafe_get(&slot::text); }

  const auto& on_click() const { return unsafe_get(&slot::on_click); }
  void on_click(function<void> value) { safe_set(&slot::on_click, std::move(value)); }
};
} // namespace yw::ui
