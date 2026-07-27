#pragma once
#include <ywx/icon.h>
#include <ywx/control.h>

namespace yw::ui {

class icon : public control {
public:
  struct slot : control::slot {
    yw::icon content{};
    alignment icon_align = center;

    //-- override functions --//

    virtual std::expected<void, error> apply_color_theme(const yw::ui::color_theme& Theme, bool) override {
      background_color = Theme.surface;
      border_color = colors::transparent;
      if (auto vector = content.get_if_vector()) {
        vector->fill_color(Theme.text);
        vector->stroke_color(Theme.text);
      }
      make_dirty();
      return {};
    }

    virtual std::expected<float2, error> get_necessary_size() const override {
      const auto inner = content.size() + padding.xy() + padding.zw();
      return calc_necessary_size_by_policy(inner);
    }

    virtual std::expected<void, error> draw_content() override {
      const auto origin = pos + padding.xy();
      const auto area = size - padding.xy() - padding.zw();
      if (auto res = icon::slot::draw_icon(content, origin, area, icon_align); !res) return res.error().relay();
      return {};
    }

    //-- shared functions --//

    static std::expected<void, error> draw_icon(const yw::icon& Icon, float2 Pos, float2 Area, alignment Align) {
      if (Icon.empty()) return {};
      constexpr float c[]{0.5f, 0.0f, 1.0f};
      const float2 cc{c[unsigned(Align) % 3], c[unsigned(Align) / 3 % 3]};
      const auto icon_size = Icon.size();
      const float2 pos = Pos + (Area - icon_size) * cc;
      if (auto res = yw::draw_icon(pos, icon_size, Icon); !res) return res.error().relay();
      return {};
    }
  };

  icon() noexcept = default;

  icon(derived_from<interface> auto& Parent, const source_line& sl = here()) {
    if (auto res = create(Parent)) *this = std::move(*res);
    else res.error().add_footprint().go_off(sl);
  }

  static std::expected<icon, error> create(derived_from<interface> auto& Parent) {
    icon i;
    const auto temp_id = make_slot<icon>();
    const auto sp = get_slot<icon>(temp_id);
    if (!sp) return std::unexpected(error(errors::slot_creation_failed));
    const auto psp = get_slot<control>(Parent.id());
    if (!psp) return std::unexpected(error(errors::invalid_slotid));
    if (auto res = psp->attach(temp_id); !res) {
      slot::slots.erase(temp_id);
      return res.error().relay();
    }
    i._id = temp_id;
    sp->id = temp_id;
    sp->window_id = psp->get_window_id();
    sp->policy = {ui::size_policy::fit, ui::size_policy::fit};
    if (auto theme = sp->get_color_theme(); !theme) return theme.error().relay();
    else if (auto res = sp->apply_color_theme(*(*theme), false); !res) return res.error().relay();
    return i;
  }

  //-- getter --//

  const auto& content() const noexcept {
    const auto sp = get_slot(&*this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->content;
  }

  auto icon_align() const noexcept {
    const auto sp = get_slot(&*this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->icon_align;
  }

  //-- setter --//

  auto& content(this auto& self, yw::icon Icon) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->content = std::move(Icon);
    sp->make_messy();
    return self;
  }

  auto& icon_align(this auto& self, alignment v) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->icon_align = v;
    sp->make_dirty();
    return self;
  }
};
} // namespace yw::ui
