#pragma once
#include <ywx/control.h>
#include <ywx/text.h>

namespace yw::ui {

class label : public control {
public:
  struct slot : control::slot {
    yw::text text = yw::text(L"");
    optional<color> text_color;
    alignment text_align = center;

    //-- override functions --//

    virtual color get_text_color(const interface::slot* Window) const noexcept {
      if (text_color) return *text_color;
      if (auto theme = get_color_theme(Window)) return (*theme)->text;
      return colors::transparent;
    }

    virtual color get_border_color(const interface::slot*) const noexcept override {
      return border_color ? *border_color : colors::transparent;
    }

    virtual std::expected<float2, error> get_necessary_size() const override {
      const auto inner = text.size() + padding.xy() + padding.zw();
      return calc_necessary_size_by_policy(inner);
    }

    virtual std::expected<void, error> draw_forecontent(interface::slot* Window) override {
      const auto origin = pos + padding.xy();
      const auto area = size - padding.xy() - padding.zw();
      const auto pos = align_position(origin, area, text.size(), text_align);
      if (auto res = draw_text(pos, text, get_text_color(Window)); !res) return res.error().relay();
      return {};
    }

    //-- virtual functions --//
    //-- shared functions --//
    //-- internal functions --//
  };

  class proxy : public control::proxy {
    friend class label;
  protected:
    using control::proxy::proxy;
    label::slot* _get_slot() const noexcept { return static_cast<label::slot*>(_slot); }

  public:
    //-- getter --//

    const auto& text() const&& noexcept { return _get_slot()->text; }
    color text_color() const&& noexcept {
      return _get_slot()->get_text_color(interface::slot::slots.get(_get_slot()->window_id));
    }
    alignment text_align() const&& noexcept { return _get_slot()->text_align; }
    const auto& string() const&& noexcept { return _get_slot()->text.string(); }
    const auto& font() const&& noexcept { return _get_slot()->text.font(); }

    //-- setter --//

    auto text(this auto&& Self, yw::text Text) noexcept {
      Self._get_slot()->text = std::move(Text);
      Self._messy = true;
      return std::move(Self);
    }

    auto text_color(this auto&& Self, const color& Color) noexcept {
      Self._get_slot()->text_color = Color;
      Self._dirty = true;
      return std::move(Self);
    }

    auto text_color(this auto&& Self, none) noexcept {
      Self._get_slot()->text_color = none();
      Self._dirty = true;
      return std::move(Self);
    }

    auto text_align(this auto&& Self, alignment Align) noexcept {
      Self._get_slot()->text_align = Align;
      Self._dirty = true;
      return std::move(Self);
    }

    auto string(this auto&& Self, yw::string<wchar_t> String) noexcept {
      Self._get_slot()->text.string(std::move(String));
      Self._messy = true;
      return std::move(Self);
    }

    auto font(this auto&& Self, font_config Font) noexcept {
      Self._get_slot()->text.font(std::move(Font));
      Self._messy = true;
      return std::move(Self);
    }
  };

  label() noexcept = default;

  label(derived_from<interface> auto& Parent, const source_line& sl = here()) {
    if (auto res = create(Parent)) *this = std::move(*res);
    else res.error().add_footprint().go_off(sl);
  }

  static std::expected<label, error> create(derived_from<interface> auto& Parent) {
    label l;
    label::slot* sp;
    if (auto res = create_control<label>(Parent)) sp = *res;
    else return res.error().relay();
    l._id = sp->id;
    sp->policy = {ui::size_policy::fit, ui::size_policy::fit};
    return l;
  }

  yw_control_getter_setter(text, yw::text);
  yw_control_getter_setter(text_color, color);
  auto text_color(this auto& Self, none None) noexcept {
    return typename remove_cvref<decltype(Self)>::proxy(get_slot(&Self)).text_color(None);
  }
  yw_control_getter_setter(text_align, alignment);
  yw_control_getter_setter(string, yw::string<wchar_t>);
  yw_control_getter_setter(font, font_config);
};
} // namespace yw::ui
