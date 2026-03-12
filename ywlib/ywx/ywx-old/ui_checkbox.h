#pragma once
#include "ywx/ui_base.h"
#include "ywx/ui_part_icon.h"
#include "ywx/ui_part_label.h"

namespace yw::ui {

class checkbox : public base {
public:
  class slot : public base::slot {
  public:
    inline static svgpath default_box_icon{};
    inline static svgpath default_mark_icon{};

    part::icon box_icon;
    part::icon mark_icon;
    part::label text;
    bool checked = false;

    slot() { focusable = true; }

    virtual std::expected<void, error_trace> draw() const override {
      if (auto res = base::slot::draw(); !res) return unexpected_error(res.error());
      if (auto res = box_icon.draw(pos); !res) return unexpected_error(res.error());
      if (checked)
        if (auto res = mark_icon.draw(pos); !res) return unexpected_error(res.error());
      if (auto res = text.draw({pos.x + box_icon.size().x, pos.y}); !res) return unexpected_error(res.error());
      return {};
    }

    virtual void click_event(event::button e) override {
      if (enabled && e.code == key::lbutton) checked = !checked;
      base::slot::click_event(e);
    }
  };

public:
  using base::operator bool;

  auto* edit_box_icon() noexcept { return safe_get(&slot::box_icon); }
  const auto& box_icon() const { return unsafe_get(&slot::box_icon); }

  auto* edit_mark_icon() noexcept { return safe_get(&slot::mark_icon); }
  const auto& mark_icon() const { return unsafe_get(&slot::mark_icon); }

  auto* edit_text() noexcept { return safe_get(&slot::text); }
  const auto& text() const { return unsafe_get(&slot::text); }

  bool checked() const { return unsafe_get(&slot::checked); }
  void checked(bool value) { safe_set(&slot::checked, value); }

  void size(float2 Size) {
    if (auto s = slot_address(this)) {
      Size.x = yw::max(Size.x, s->box_icon.size().x);
      s->size = Size;
      s->text.size({Size.x - s->box_icon.size().x, Size.y});
      s->make_window_dirty();
    }
  }

  void icon_size(float1 Size) {
    if (auto s = slot_address(this)) {
      s->box_icon.size({Size.x, s->size.y});
      s->mark_icon.size({Size.x, s->size.y});
      s->text.size({s->size.x - Size.x, s->size.y});
      s->make_window_dirty();
    }
  }

  template<included_in<window&, none> Window>
  static std::expected<checkbox, error_trace> add(Window&& w, float2 Pos, float2 Size, float1 IconSize = 20.0f) {
    Size.x = yw::max(Size.x, IconSize.x * 2.0f);
    if (auto res = base::add<checkbox>(w, Pos, Size)) {
      if (!slot::default_box_icon) {
        auto unchecked_svg = svgpath::create({16, 16}, "M 8 1 A 7 7 0 1 1 7.999 1 Z");
        if (unchecked_svg) slot::default_box_icon = std::move(*unchecked_svg);
      }
      if (!slot::default_mark_icon) {
        auto checked_svg = svgpath::create({16, 16}, "M 8 4.5 A 3.5 3.5 0 1 1 7.999 4.5 Z");
        if (checked_svg) slot::default_mark_icon = std::move(*checked_svg);
      }
      const auto slot_p = res->second;

      slot_p->box_icon.size({IconSize.x, Size.y});
      if (auto res = svgpath::create(slot::default_box_icon)) slot_p->box_icon.image = std::move(*res);
      else return unexpected_error(res.error());
      slot_p->box_icon.padding({2.0f, 2.0f});
      slot_p->box_icon.fill_color(colors::white);
      slot_p->box_icon.border_color(colors::black);

      slot_p->mark_icon.size({IconSize.x, Size.y});
      if (auto res = svgpath::create(slot::default_mark_icon)) slot_p->mark_icon.image = std::move(*res);
      else return unexpected_error(res.error());
      slot_p->mark_icon.padding({2.0f, 2.0f});
      slot_p->mark_icon.fill_color(colors::black);
      slot_p->mark_icon.border_color(colors::transparent);

      slot_p->text.size({Size.x - IconSize.x, Size.y});
      slot_p->text.format(dwrite.text_format());
      slot_p->text.layout().text_alignment(DWRITE_TEXT_ALIGNMENT_LEADING);
      slot_p->text.layout().paragraph_alignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

      slot_p->make_window_dirty();
      return checkbox{std::move(res->first)};
    } else return unexpected_error(res.error());
  }
};
} // namespace yw::ui
