#pragma once
#include <ywx/icon.h>
#include <ywx/control.h>

namespace yw::ui {

class icon : public control {
public:
  struct slot : control::slot {
    yw::icon content{};
    optional<color> fill_color;
    optional<color> stroke_color;
    alignment icon_align = center;

    //-- override functions --//

    virtual color get_fill_color(const interface::slot* Window) const noexcept {
      if (fill_color) return *fill_color;
      if (auto theme = get_color_theme(Window)) return (*theme)->text;
      return colors::transparent;
    }

    virtual color get_stroke_color(const interface::slot* Window) const noexcept {
      if (stroke_color) return *stroke_color;
      if (auto theme = get_color_theme(Window)) return (*theme)->text;
      return colors::transparent;
    }

    virtual color get_border_color(const interface::slot*) const noexcept override {
      return border_color ? *border_color : colors::transparent;
    }

    virtual std::expected<void, error> draw_backcontent(interface::slot* Window) override {
      const auto origin = pos + padding.xy();
      const auto area = size - padding.xy() - padding.zw();
      const auto pos = align_position(origin, area, content.size(), icon_align);
      if (content.is_bitmap()) {
        if (auto res = draw_bitmap(pos, content.get_bitmap()); !res) return res.error().relay();
      } else if (content.is_vector())
        if (const auto fill = get_fill_color(Window); fill.a > 0.0f) {
          const auto& vector = content.get_vector();
          if (auto res = fill_svgpath(pos, vector.path, fill); !res) return res.error().relay();
        }
      return {};
    }

    virtual std::expected<void, error> draw_forecontent(interface::slot* Window) override {
      const auto origin = pos + padding.xy();
      const auto area = size - padding.xy() - padding.zw();
      const auto pos = align_position(origin, area, content.size(), icon_align);
      if (content.is_vector())
        if (const auto stroke = get_stroke_color(Window); stroke.a > 0.0f) {
          const auto& vector = content.get_vector();
          if (auto res = stroke_svgpath(pos, vector.path, stroke, vector.stroke_width); !res)
            return res.error().relay();
        }
      return {};
    }

    virtual std::expected<float2, error> get_necessary_size() const override {
      const auto inner = content.size() + padding.xy() + padding.zw();
      return calc_necessary_size_by_policy(inner);
    }

    //-- shared functions --//
    //-- internal functions --//
  };

  class proxy : public control::proxy {
    friend class icon;
    using control::proxy::proxy;
    icon::slot* _get_slot() const noexcept { return static_cast<icon::slot*>(_slot); }

  public:
    //-- getter --//

    const auto& content() const&& noexcept { return _get_slot()->content; }
    alignment icon_align() const&& noexcept { return _get_slot()->icon_align; }

    //-- setter --//

    auto content(this auto&& Self, yw::icon Icon) noexcept {
      Self._get_slot()->content = std::move(Icon);
      Self._messy = true;
      return std::move(Self);
    }

    auto icon_align(this auto&& Self, alignment Align) noexcept {
      Self._get_slot()->icon_align = Align;
      Self._dirty = true;
      return std::move(Self);
    }
  };

  icon() noexcept = default;

  icon(derived_from<interface> auto& Parent, const source_line& sl = here()) {
    if (auto res = create(Parent)) *this = std::move(*res);
    else res.error().add_footprint().go_off(sl);
  }

  static std::expected<icon, error> create() {
    icon i;
    icon::slot* sp;
    if (auto res = create_control<icon>()) sp = *res;
    else return res.error().relay();
    i._id = sp->id;
    sp->policy = {ui::size_policy::fit, ui::size_policy::fit};
    return i;
  }

  static std::expected<icon, error> create(derived_from<interface> auto& Parent) {
    auto res = create();
    if (!res) return res.error().relay();
    if (auto attached = res->attach(Parent); !attached) return attached.error().relay();
    return res;
  }

  yw_control_getter_setter(content, yw::icon);
  yw_control_getter_setter(icon_align, alignment);
};
} // namespace yw::ui
