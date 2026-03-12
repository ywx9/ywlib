#pragma once
#include "ywx/ui_base.h"
#include "ywx/ui_part_label.h"

namespace yw::ui {

//////////////////////////////////////// MARK: ui::button

class button : public base {
public:
  class slot : public base::slot {
  public:
    part::label text;

    slot() { focusable = true, enabled = true; }

    virtual std::expected<void, error_trace> draw() const override {
      if (auto res = base::slot::draw(); !res) return unexpected_error(res.error());
      if (auto res = text.draw(pos); !res) return unexpected_error(res.error());
      return {};
    }
  };

public:
  using base::operator bool;

  auto* edit_text() noexcept { return safe_get(&slot::text); }
  const auto& text() const { return unsafe_get(&slot::text); }

  template<included_in<window&, none> Window>
  static std::expected<button, error_trace> add(Window&& w, float2 Pos, float2 Size) {
    if (auto res = base::add<button>(w, Pos, Size)) {
      const auto slot_p = res->second;
      slot_p->text.size(Size);
      slot_p->radius = {8.0f, 8.0f};
      slot_p->make_window_dirty();
      return button{std::move(res->first)};
    } else return unexpected_error(res.error());
  }
};
} // namespace yw::ui
