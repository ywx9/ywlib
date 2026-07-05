#pragma once
#include <ywx/text.h>
#include <ywx/ui_frame.h>

namespace yw::ui {

class label : public frame {
public:
  struct slot : frame::slot {
    yw::text text = yw::text(L" ");
    color text_color = colors::black;
    alignment text_align = alignment::center;

    float2 _text_offset() const noexcept {
      constexpr float c[]{0.5f, 0.0f, 1.0f};
      const float2 cc{c[unsigned(text_align) % 3], c[unsigned(text_align) / 3 % 3]};
      return (size - text.size() - padding.xy() - padding.zw()) * cc + padding.xy();
    }

    virtual std::expected<float2, error> get_necessary_size() const override {
      const auto inner = text.size() + padding.xy() + padding.zw();
      return vapply_r<float2>(_necessary_size, policy, minimum_size, required_size, inner);
    }

    virtual std::expected<void, error> redraw() override {
      if (geometry_dirty) {
        geometry_dirty = false;
        if (auto res = relocate(); !res) return res.error().relay();
      }
      if (!visible) return {};
      if (auto res = _draw_background(); !res) return res.error().relay();
      brush::color(text_color);
      if (auto res = text.draw(pos + _text_offset()); !res) return res.error().relay();
      if (auto res = _draw_foreground(); !res) return res.error().relay();
      return {};
    }
  };

  label() noexcept = default;

  label(derived_from<interface> auto& Parent, bool AutoColor = true, const source_line& sl = here()) {
    if (auto res = slot::create<label>(Parent, AutoColor, sl)) {
      const auto sp = *res;
      _id = sp->id;
      sp->text_color = std::exchange(sp->colors.border, colors::transparent);
    } else res.error().add_footprint().go_off(sl);
  }

  static std::expected<label, error> create(
    derived_from<interface> auto& Parent, bool AutoColor = true, const source_line& sl = here()) {
    label l;
    if (auto res = slot::create<label>(Parent, AutoColor, sl)) {
      const auto sp = *res;
      l._id = sp->id;
      sp->text_color = std::exchange(sp->colors.border, colors::transparent);
      return l;
    } else return res.error().relay();
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

  //-- setter --//

  auto& text(this auto& self, yw::text Text) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->text = std::move(Text);
    if (auto res = sp->make_messy(); !res) res.error().go_off();
    return self;
  }

  auto& text_color(this auto& self, const color& c) noexcept { ywlib_control_set(text_color, c, dirty); }

  auto& text_align(this auto& self, alignment v) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->text_align = v;
    if (auto res = sp->make_dirty(); !res) res.error().go_off();
    return self;
  }

  auto& string(this auto& self, yw::string<wchar_t> s) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    if (auto res = sp->text.string(std::move(s)); !res) res.error().go_off();
    if (auto res = sp->make_messy(); !res) res.error().go_off();
    return self;
  }
};
} // namespace yw::ui
