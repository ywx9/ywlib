#pragma once
#include "ywx/ui_text.h"
#include "ywx/ui_frame.h"

namespace yw::ui {

//////////////////////////////////////// MARK: ui::label

class label : public frame {
public:
  class slot : public frame::slot {
  public:
    ui::text text;
    virtual void draw() const override {
      frame::slot::draw();
      if (auto text_slot_p = text.slot_adress(&text)) text_slot_p->draw();
    }
  };

public:
  using base::operator bool;
  slot* slot_address() const noexcept { return dynamic_cast<slot*>(system::uis.get(_id)); }

  const auto& text() const { return unsafe_get(&slot::text); }

  void text_alignment(DWRITE_TEXT_ALIGNMENT align) {
    if (const auto s = slot_address()) s->text.text_alignment(align);
  }
  void paragraph_alignment(DWRITE_PARAGRAPH_ALIGNMENT align) {
    if (const auto s = slot_address()) s->text.paragraph_alignment(align);
  }

  template<included_in<window&, none> Window, stringable S> static std::expected<label, error_trace> add(
    Window&& w, float2 Pos, float2 Size, S&& Text, float2 Padding = {}) {
    if (auto res = base::add<label>(w, Pos, Size)) {
      const auto slot_p = res->second;
      if (auto text_res = ui::text::add(none{}, Pos, Size, static_cast<S&&>(Text), Padding)) {
        slot_p->text = std::move(*text_res);
        return label{std::move(res->first)};
      } else return unexpected_error(text_res.error());
    } else return unexpected_error(res.error());
  }
};
} // namespace yw::ui
