#pragma once
#include "ywx/ui_icon.h"
#include "ywx/ui_text.h"

namespace yw::ui {

//////////////////////////////////////// MARK: ui::checkbox

class checkbox : public frame {
public:
  class slot : public frame::slot {
  public:
    inline static svgpath default_unchecked_icon{};
    inline static svgpath default_checked_icon{};

    ui::icon unchecked_icon;
    ui::icon checked_icon;
    ui::text text;
    bool checked = false;

    slot() { focusable = true; }

    virtual void draw() const override {
      frame::slot::draw();
      if (auto icon_slot_p = unchecked_icon.slot_adress(&unchecked_icon)) icon_slot_p->draw();
      if (checked)
        if (auto icon_slot_p = checked_icon.slot_adress(&checked_icon)) icon_slot_p->draw();
      if (auto text_slot_p = text.slot_adress(&text)) text_slot_p->draw();
    }

    virtual void click_event(event::button e) override {
      if (is_enabled() && e.code == key::lbutton) checked = !checked;
      frame::slot::click_event(e);
    }
  };

public:
  using base::operator bool;
  slot* slot_address() const noexcept { return dynamic_cast<slot*>(system::uis.get(_id)); }

  const auto& unchecked_icon() const { return unsafe_get(&slot::unchecked_icon); }
  const auto& checked_icon() const { return unsafe_get(&slot::checked_icon); }
  const auto& text() const { return unsafe_get(&slot::text); }
  bool checked() const { return unsafe_get(&slot::checked); }

  void checked(bool value) { _set(&slot::checked, value); }

  void text_alignment(DWRITE_TEXT_ALIGNMENT align) {
    if (const auto s = slot_address()) s->text.text_alignment(align);
  }
  void paragraph_alignment(DWRITE_PARAGRAPH_ALIGNMENT align) {
    if (const auto s = slot_address()) s->text.paragraph_alignment(align);
  }

  template<included_in<window&, none> Window, stringable S> static std::expected<checkbox, error_trace> add(
    Window&& w, float2 Pos, float2 Size, S&& Text, float2 IconSize = {18, 18}, float2 Padding = {4, 4}) {
    if (auto res = base::add<checkbox>(w, Pos, Size)) {
      const auto slot_p = res->second;
      // Initialize default icons on first create
      if (!slot::default_unchecked_icon) {
        auto unchecked_svg = svgpath::create(
          {16, 16}, "M 4 1 L 12 1 Q 15 1 15 4 L 15 12 Q 15 15 12 15 L 4 15 Q 1 15 1 12 L 1 4 Q 1 1 4 1 Z");
        if (unchecked_svg) slot::default_unchecked_icon = std::move(*unchecked_svg);
      }
      if (!slot::default_checked_icon) {
        auto checked_svg = svgpath::create({16, 16}, "M 4 1 L 12 1 Q 15 1 15 4 L 15 12 Q 15 15 12 15 L 4 15 Q 1 15 1 "
                                                     "12 L 1 4 Q 1 1 4 1 Z M 4 8 L 7 11 L 12 5 L 11 4 L 7 9 L 5 7 Z");
        if (checked_svg) slot::default_checked_icon = std::move(*checked_svg);
      }

      // Center icon and text vertically
      const auto icon_y = Pos.y + (Size.y - IconSize.y) * 0.5f;
      const auto icon_pos = float2(Pos.x + Padding.x, icon_y);
      auto unchecked_icon_res = ui::icon::add(none{}, icon_pos, IconSize);
      if (!unchecked_icon_res) return unexpected_error(unchecked_icon_res.error());
      slot_p->unchecked_icon = std::move(*unchecked_icon_res);
      if (auto res = svgpath::create(slot::default_unchecked_icon); !res) return unexpected_error(res.error());
      else slot_p->unchecked_icon.image(std::move(*res));

      auto checked_icon_res = ui::icon::add(none{}, icon_pos, IconSize);
      if (!checked_icon_res) return unexpected_error(checked_icon_res.error());
      slot_p->checked_icon = std::move(*checked_icon_res);
      if (auto res = svgpath::create(slot::default_checked_icon); !res) return unexpected_error(res.error());
      else slot_p->checked_icon.image(std::move(*res));

      const auto text_pos = float2(icon_pos.x + IconSize.x + Padding.x, Pos.y);
      const auto text_size = float2(std::max(0.0f, Size.x - (IconSize.x + Padding.x * 2.0f)), Size.y);
      auto text_res = ui::text::add(none{}, text_pos, text_size, static_cast<S&&>(Text), {});
      if (!text_res) return unexpected_error(text_res.error());
      slot_p->text = std::move(*text_res);
      slot_p->text.paragraph_alignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

      return checkbox{std::move(res->first)};
    } else return unexpected_error(res.error());
  }
};

} // namespace yw::ui
