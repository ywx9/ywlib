#pragma once
#include <ywx/control.h>
#include <ywx/text.h>

namespace yw::ui {

class label : public control {
public:
  struct slot : control::slot {
    yw::text text = yw::text(L"");
    alignment text_align = center;

    //-- override functions --//

    virtual std::expected<void, error> apply_color_theme(const yw::ui::color_theme& Theme, bool) override {
      background_color = Theme.surface;
      border_color = colors::transparent;
      text.color(Theme.text);
      make_dirty();
      return {};
    }

    virtual std::expected<float2, error> get_necessary_size() const override {
      const auto inner = text.size() + padding.xy() + padding.zw();
      return calc_necessary_size_by_policy(inner);
    }

    virtual std::expected<void, error> draw_forecontent() override {
      const auto origin = pos + padding.xy();
      const auto area = size - padding.xy() - padding.zw();
      const auto pos = align_position(origin, area, text.size(), text_align);
      if (auto res = draw_text(pos, text); !res) return res.error().relay();
      return {};
    }

    //-- virtual functions --//
    //-- shared functions --//
    //-- internal functions --//
  };

  label() noexcept = default;

  label(derived_from<interface> auto& Parent, const source_line& sl = here()) {
    if (auto res = create(Parent)) *this = std::move(*res);
    else res.error().add_footprint().go_off(sl);
  }

  static std::expected<label, error> create(derived_from<interface> auto& Parent) {
    label l;
    const auto temp_id = make_slot<label>();
    const auto sp = get_slot<label>(temp_id);
    if (!sp) return std::unexpected(error(errors::slot_creation_failed));
    const auto psp = get_slot<control>(Parent.id());
    if (!psp) return std::unexpected(error(errors::invalid_slotid));
    if (auto res = psp->attach(temp_id); !res) {
      slot::slots.erase(temp_id);
      return res.error().relay();
    }
    l._id = temp_id;
    sp->id = temp_id;
    sp->window_id = psp->get_window_id();
    sp->policy = {ui::size_policy::fit, ui::size_policy::fit};
    if (auto theme = sp->get_color_theme(); !theme) return theme.error().relay();
    else if (auto res = sp->apply_color_theme(*(*theme), false); !res) return res.error().relay();
    return l;
  }

  //-- getter --//

  const auto& text() const noexcept {
    const auto sp = get_slot(&*this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->text;
  }

  const auto& text_color() const noexcept {
    const auto sp = get_slot(&*this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->text.color();
  }

  alignment text_align() const noexcept {
    const auto sp = get_slot(&*this);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return {};
    }
    return sp->text_align;
  }

  const auto& string() const noexcept {
    const auto sp = get_slot(&*this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->text.string();
  }

  const auto& font() const noexcept {
    const auto sp = get_slot(&*this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->text.font();
  }

  //-- setter --//

  auto& text(this auto& self, yw::text Text) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    sp->text = std::move(Text);
    sp->make_messy();
    return self;
  }

  auto& text_color(this auto& self, const color& c) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    sp->text.color(c);
    sp->make_dirty();
    return self;
  }

  auto& text_align(this auto& self, alignment v) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    sp->text_align = v;
    sp->make_dirty();
    return self;
  }

  auto& string(this auto& self, yw::string<wchar_t> s) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    sp->text.string(std::move(s));
    sp->make_messy();
    return self;
  }

  auto& font(this auto& self, font_config f) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    sp->text.font(std::move(f));
    sp->make_messy();
    return self;
  }
};
} // namespace yw::ui
