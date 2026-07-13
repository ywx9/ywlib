#pragma once
#include <yw/flags.h>
#include <ywx/clipboard.h>
#include <ywx/text.h>
#include <ywx/ui_frame.h>

namespace yw::ui {

class label : public frame {
public:
  struct slot : frame::slot {
    yw::text text = yw::text(L"");
    color text_color = colors::black;
    alignment text_align = alignment::center;

    float2 calc_text_offset() const noexcept {
      constexpr float c[]{0.5f, 0.0f, 1.0f};
      const float2 cc{c[unsigned(text_align) % 3], c[unsigned(text_align) / 3 % 3]};
      return (size - text.size() - padding.xy() - padding.zw()) * cc + padding.xy();
    }

    virtual std::expected<float2, error> get_necessary_size() const override {
      const auto inner = text.size() + padding.xy() + padding.zw();
      return calc_necessary_size_by_policy(inner);
    }

    virtual std::expected<void, error> redraw() override {
      if (geometry_dirty) {
        geometry_dirty = false;
        if (auto res = relocate(); !res) return res.error().relay();
      }
      if (!visible) return {};
      if (auto res = draw_frame_background(); !res) return res.error().relay();
      brush::color(text_color);
      if (auto res = text.draw(pos + calc_text_offset()); !res) return res.error().relay();
      if (auto res = draw_frame_foreground(); !res) return res.error().relay();
      return {};
    }
  };

  label() noexcept = default;

  label(derived_from<interface> auto& Parent, strict<bool> AutoColor = true, const source_line& sl = here()) {
    if (auto res = create(Parent, AutoColor)) *this = std::move(*res);
    else res.error().add_footprint().go_off(sl);
  }

  static std::expected<label, error> create(derived_from<interface> auto& Parent, strict<bool> AutoColor = true) {
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
    if (AutoColor) {
      sp->colors = color_pair(none());
      sp->text_color = std::exchange(sp->colors.border, colors::transparent);
    }
    return l;
  }

  //-- getter --//

  const auto& text() const noexcept { ywlib_control_get(text); }
  const auto& text_color() const noexcept { ywlib_control_get(text_color); }
  const auto& text_align() const noexcept { ywlib_control_get(text_align); }

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
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->text = std::move(Text);
    sp->make_messy();
    return self;
  }

  auto& text_color(this auto& self, const color& c) noexcept { ywlib_control_set(text_color, c, dirty); }

  auto& text_align(this auto& self, alignment v) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->text_align = v;
    sp->make_dirty();
    return self;
  }

  auto& string(this auto& self, yw::string<wchar_t> s) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    if (auto res = sp->text.string(std::move(s)); !res) res.error().go_off();
    sp->make_messy();
    return self;
  }

  auto& font(this auto& self, font_config f) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    if (auto res = sp->text.font(std::move(f)); !res) res.error().go_off();
    sp->make_messy();
    return self;
  }
};
} // namespace yw::ui
