#pragma once
#include <ywx/ui_icon.h>
#include <ywx/ui_label.h>
#include <ywx/window.h>

namespace yw::ui {

class checkbox : public control {
public:
  struct slot : control::slot {
    yw::text text = yw::text(L"");
    optional<color> text_color;
    yw::icon box;
    yw::icon check;
    optional<color> box_fill_color;
    optional<color> box_stroke_color;
    optional<color> check_fill_color;
    optional<color> check_stroke_color;
    float2 icon_size{common_size_value, common_size_value};
    float icon_gap = arbitrary_value;
    bool checked = false;
    bool pressed = false;

    function<bool, bool> change_event{};

    //-- override functions --//

    virtual bool is_focusable() const override { return enabled && visible; }
    virtual bool is_interactive() const override { return true; }

    virtual color get_text_color(const interface::slot* Window) const noexcept {
      if (text_color) return *text_color;
      if (auto theme = get_color_theme(Window)) return (*theme)->text;
      return colors::transparent;
    }

    virtual color get_border_color(const interface::slot*) const noexcept override {
      return border_color ? *border_color : colors::transparent;
    }

    virtual color get_box_fill_color(const interface::slot*) const noexcept {
      return box_fill_color ? *box_fill_color : colors::transparent;
    }

    virtual color get_box_stroke_color(const interface::slot* Window) const noexcept {
      if (box_stroke_color) return *box_stroke_color;
      if (auto theme = get_color_theme(Window)) return (*theme)->outline;
      return colors::transparent;
    }

    virtual color get_check_fill_color(const interface::slot* Window) const noexcept {
      if (check_fill_color) return *check_fill_color;
      if (auto theme = get_color_theme(Window)) return (*theme)->accent;
      return colors::transparent;
    }

    virtual color get_check_stroke_color(const interface::slot* Window) const noexcept {
      if (check_stroke_color) return *check_stroke_color;
      if (auto theme = get_color_theme(Window)) return (*theme)->accent;
      return colors::transparent;
    }

    virtual std::expected<void, error> draw_backcontent(interface::slot* Window) override {
      const auto origin = float2(pos.x + padding.x, pos.y + (size.y - icon_size.y) * 0.5f);
      if (box.is_bitmap()) {
        if (auto res = draw_bitmap(origin, icon_size, box.get_bitmap()); !res) return res.error().relay();
      } else if (box.is_vector())
        if (const auto fill = get_box_fill_color(Window); fill.a > 0.0f) {
          const auto& vector = box.get_vector();
          if (auto res = fill_svgpath(origin, vector.path, fill); !res) return res.error().relay();
        }
      return {};
    }

    virtual std::expected<void, error> draw_forecontent(interface::slot* Window) override {
      const auto origin = float2(pos.x + padding.x, pos.y + (size.y - icon_size.y) * 0.5f);
      if (box.is_vector())
        if (const auto stroke = get_box_stroke_color(Window); stroke.a > 0.0f) {
          const auto& vector = box.get_vector();
          if (auto res = stroke_svgpath(origin, vector.path, stroke, vector.stroke_width); !res)
            return res.error().relay();
        }
      if (checked) {
        if (check.is_bitmap()) {
          if (auto res = draw_bitmap(origin, icon_size, check.get_bitmap()); !res) return res.error().relay();
        } else if (check.is_vector()) {
          const auto& vector = check.get_vector();
          if (const auto fill = get_check_fill_color(Window); fill.a > 0.0f)
            if (auto res = fill_svgpath(origin, icon_size, vector.path, fill); !res) return res.error().relay();
          if (const auto stroke = get_check_stroke_color(Window); stroke.a > 0.0f && vector.stroke_width > 0.0f)
            if (auto res = stroke_svgpath(origin, icon_size, vector.path, stroke, vector.stroke_width); !res)
              return res.error().relay();
        }
      }
      const auto text_origin = float2(pos.x + padding.x + icon_size.x + icon_gap, pos.y + padding.y);
      const auto text_area = pos + size - padding.zw() - text_origin;
      const auto text_pos = align_position(text_origin, text_area, text.size(), left);
      if (auto res = draw_text(text_pos, text, get_text_color(Window)); !res) return res.error().relay();
      return {};
    }

    virtual std::expected<void, error> draw_overlay(interface::slot* Window) override {
      const auto wsp = static_cast<window::slot*>(Window);
      if (!wsp) return {};
      if (pressed && wsp->press_overlay_color.a > 0.0f) {
        brush::color(wsp->press_overlay_color);
        if (auto res = fill_geometry(geometry.get()); !res) return res.error().relay();
      } else if (id == wsp->hovered_control_id && wsp->hover_overlay_color.a > 0.0f) {
        brush::color(wsp->hover_overlay_color);
        if (auto res = fill_geometry(geometry.get()); !res) return res.error().relay();
      }
      return {};
    }

    virtual float2 get_minimum_size() const override {
      return icon_size + padding.xy() + padding.zw();
    }

    virtual std::expected<float2, error> get_necessary_size() const override {
      const auto text_size = text.size();
      const auto gap = text_size.x > 0.0f ? icon_gap : 0.0f;
      const auto inner =
        float2{icon_size.x + gap + text_size.x, yw::max(icon_size.y, text_size.y)} + padding.xy() + padding.zw();
      return calc_necessary_size_by_policy(inner);
    }

    virtual void reset_state() override {
      if (!pressed) return;
      pressed = false;
      make_dirty();
    }

    virtual bool handle_button_event(yw::button_event e) override {
      if (!enabled || !visible || e.key != keys::lbutton) return false;
      const bool next_pressed = e.down;
      if (pressed == next_pressed) return true;
      pressed = next_pressed;
      make_dirty();
      return true;
    }

    virtual bool handle_click_event(yw::button_event e) override {
      if (!enabled || !visible || e.down || e.key != keys::lbutton) return false;
      return toggle();
    }

    virtual bool handle_focus_event(yw::focus_event e) override {
      if (!e.focused && pressed) {
        pressed = false;
        make_dirty();
      }
      return control::slot::handle_focus_event(e);
    }

    virtual bool handle_key_event(yw::key_event e) override {
      if (!enabled || !visible) return false;
      if (e.key != keys::space && e.key != keys::enter) return false;
      if (e.down) {
        if (!pressed) {
          pressed = true;
          make_dirty();
        }
        return true;
      }
      const bool was_pressed = pressed;
      pressed = false;
      return was_pressed ? toggle() : true;
    }

    //-- shared functions --//

    bool toggle() {
      checked = !checked;
      make_dirty();
      return change_event ? change_event(checked) : true;
    }
  };

  class proxy : public control::proxy {
    friend class checkbox;
    using control::proxy::proxy;
    checkbox::slot* _get_slot() const noexcept { return static_cast<checkbox::slot*>(_slot); }

  public:
    //-- getter --//

    bool checked() const&& noexcept { return _get_slot()->checked; }
    bool pressed() const&& noexcept { return _get_slot()->pressed; }
    const auto& text() const&& noexcept { return _get_slot()->text; }
    const auto& box() const&& noexcept { return _get_slot()->box; }
    const auto& check() const&& noexcept { return _get_slot()->check; }
    const auto& change_event() const&& noexcept { return _get_slot()->change_event; }
    color text_color() const&& noexcept {
      return _get_slot()->get_text_color(interface::slot::slots.get(_get_slot()->window_id));
    }
    float2 icon_size() const&& noexcept { return _get_slot()->icon_size; }
    float icon_gap() const&& noexcept { return _get_slot()->icon_gap; }
    const auto& string() const&& noexcept { return _get_slot()->text.string(); }
    const auto& font() const&& noexcept { return _get_slot()->text.font(); }

    //-- setter --//

    auto checked(this auto&& Self, bool Checked) noexcept {
      if (Self._get_slot()->checked == Checked) return std::move(Self);
      Self._get_slot()->checked = Checked;
      Self._dirty = true;
      if (Self._get_slot()->change_event) Self._get_slot()->change_event(Self._get_slot()->checked);
      return std::move(Self);
    }

    auto text(this auto&& Self, yw::text Text) noexcept {
      Self._get_slot()->text = std::move(Text);
      Self._messy = true;
      return std::move(Self);
    }

    auto box(this auto&& Self, yw::icon Icon) noexcept {
      Self._get_slot()->box = std::move(Icon);
      Self._messy = true;
      return std::move(Self);
    }

    auto check(this auto&& Self, yw::icon Icon) noexcept {
      Self._get_slot()->check = std::move(Icon);
      Self._messy = true;
      return std::move(Self);
    }

    auto change_event(this auto&& Self, function<bool, bool> Event) noexcept {
      Self._get_slot()->change_event = std::move(Event);
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

    auto icon_size(this auto&& Self, float2 Size) noexcept {
      if (Size.x <= 0.0f || Size.y <= 0.0f) {
        error(errors::invalid_argument, format("icon_size must be positive: ", Size)).fizzle_out();
        return std::move(Self);
      }
      Self._get_slot()->icon_size = Size;
      Self._messy = true;
      return std::move(Self);
    }

    auto icon_gap(this auto&& Self, float1 Gap) noexcept {
      Self._get_slot()->icon_gap = Gap.x;
      Self._messy = true;
      return std::move(Self);
    }
  };

  checkbox() noexcept = default;

  checkbox(derived_from<interface> auto& Parent, const source_line& sl = here()) {
    if (auto res = create(Parent)) *this = std::move(*res);
    else res.error().add_footprint().go_off(sl);
  }

  static std::expected<checkbox, error> create() {
    checkbox c;
    checkbox::slot* sp;
    if (auto res = create_control<checkbox>()) sp = *res;
    else return res.error().relay();
    c._id = sp->id;
    sp->policy = {ui::size_policy::fit, ui::size_policy::fit};
    constexpr float2 init_icon_size{16.0f, 16.0f};
    sp->icon_size = init_icon_size;
    sp->box = yw::icon(yw::svgpath(init_icon_size, "M1 1 L15 1 L15 15 L1 15 Z"));
    sp->check = yw::icon(yw::svgpath(init_icon_size, "M3 8 L7 12 L13 4 L7 10 Z"));
    return c;
  }

  static std::expected<checkbox, error> create(derived_from<interface> auto& Parent) {
    auto res = create();
    if (!res) return res.error().relay();
    if (auto attached = res->attach(Parent); !attached) return attached.error().relay();
    return res;
  }

  yw_control_getter_setter(checked, bool);
  yw_control_getter(pressed);
  yw_control_getter_setter(text, yw::text);
  yw_control_getter_setter(box, yw::icon);
  yw_control_getter_setter(check, yw::icon);
  yw_control_getter_setter(change_event, function<bool, bool>);
  yw_control_getter_setter(text_color, color);
  auto text_color(this auto& Self, none None) noexcept {
    return typename remove_cvref<decltype(Self)>::proxy(get_slot(&Self)).text_color(None);
  }
  yw_control_getter_setter(icon_size, float2);
  yw_control_getter_setter(icon_gap, float1);
  yw_control_getter_setter(string, yw::string<wchar_t>);
  yw_control_getter_setter(font, font_config);
};
} // namespace yw::ui
