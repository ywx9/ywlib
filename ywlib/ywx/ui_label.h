#pragma once
#include "ywx/ui_base.h"
#include "ywx/ui_part_label.h"

namespace yw::ui {

//////////////////////////////////////// MARK: ui::label

class label : public base {
public:
  class slot : public base::slot {
  public:
    part::label text;

    virtual std::expected<void, error_trace> draw() const override {
      if (auto res = base::slot::draw(); !res) return unexpected_error(res.error());
      if (auto res = text.draw(pos); !res) return unexpected_error(res.error());
      return {};
    }
  };

public:
  using base::operator bool;

  void size(float2 Size) {
    if (auto s = slot_address(this)) s->size = Size, s->text.size(Size);
  }

  auto& text() { return unsafe_get(&slot::text); }
  const auto& text() const { return unsafe_get(&slot::text); }

  template<included_in<window&, none> Window>
  static std::expected<label, error_trace> add(Window&& w, float2 Pos, float2 Size) {
    if (auto res = base::add<label>(w, Pos, Size)) {
      const auto slot_p = res->second;
      slot_p->text.size(Size);
      return label{std::move(res->first)};
    } else return unexpected_error(res.error());
  }
};
} // namespace yw::ui
