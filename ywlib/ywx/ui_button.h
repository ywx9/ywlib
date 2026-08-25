#pragma once
#include <ywx/ui_label.h>
#include <ywx/window.h>

namespace yw::ui {

class button : public control {
public:
  struct slot : control::slot {
    yw::text text = yw::text(L"");
    optional<color> text_color;
    alignment text_align = center;
    bool pressed = false;

    function<bool, yw::button_event> click_event{};

    //-- override functions --//

    virtual color get_text_color(const interface::slot* Window) const noexcept {
      if (text_color) return *text_color;
      if (auto theme = get_color_theme(Window)) return (*theme)->text;
      return colors::transparent;
    }

    virtual std::expected<void, error> draw_forecontent(interface::slot* Window) override {
      const auto origin = pos + padding.xy();
      const auto area = size - padding.xy() - padding.zw();
      const auto text_pos = align_position(origin, area, text.size(), text_align);
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

    virtual std::expected<float2, error> get_necessary_size() const override {
      const auto inner = text.size() + padding.xy() + padding.zw();
      return calc_necessary_size_by_policy(inner);
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
      return invoke(e);
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
      if (was_pressed) {
        make_dirty();
        return invoke({{}, e.key, e.mods, false});
      }
      return true;
    }

    virtual bool is_focusable() const override { return enabled && visible; }
    virtual bool is_interactive() const override { return true; }

    virtual void reset_state() override {
      if (!pressed) return;
      pressed = false;
      make_dirty();
    }

    //-- vertual functions --//

    virtual bool invoke(yw::button_event e) {
      return click_event ? click_event(e) : true;
    }
  };

  class proxy : public control::proxy {
    friend class button;
  protected:
    using control::proxy::proxy;
    button::slot* _get_slot() const noexcept { return static_cast<button::slot*>(_slot); }

  public:
    //-- getter --//

    const auto& text() const&& noexcept { return _get_slot()->text; }
    alignment text_align() const&& noexcept { return _get_slot()->text_align; }
    color text_color() const&& noexcept {
      return _get_slot()->get_text_color(interface::slot::slots.get(_get_slot()->window_id));
    }
    const auto& string() const&& noexcept { return _get_slot()->text.string(); }
    const auto& font() const&& noexcept { return _get_slot()->text.font(); }
    bool pressed() const&& noexcept { return _get_slot()->pressed; }
    const auto& click_event() const&& noexcept { return _get_slot()->click_event; }

    //-- setter --//

    auto text(this auto&& Self, yw::text Text) noexcept {
      Self._get_slot()->text = std::move(Text);
      Self._messy = true;
      return std::move(Self);
    }

    auto text_align(this auto&& Self, alignment Align) noexcept {
      Self._get_slot()->text_align = Align;
      Self._dirty = true;
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

    auto click_event(this auto&& Self, function<bool, yw::button_event> f) noexcept {
      Self._get_slot()->click_event = std::move(f);
      return std::move(Self);
    }
  };

  button() noexcept = default;

  button(derived_from<interface> auto& Parent, const source_line& sl = here()) {
    if (auto res = create(Parent)) *this = std::move(*res);
    else res.error().add_footprint().go_off(sl);
  }

  static std::expected<button, error> create() {
    button b;
    button::slot* sp;
    if (auto res = create_control<button>()) sp = *res;
    else return res.error().relay();
    b._id = sp->id;
    sp->policy = {ui::size_policy::fit, ui::size_policy::fit};
    return b;
  }

  static std::expected<button, error> create(derived_from<interface> auto& Parent) {
    auto res = create();
    if (!res) return res.error().relay();
    if (auto attached = res->attach(Parent); !attached) return attached.error().relay();
    return res;
  }

  yw_control_getter_setter(text, yw::text);
  yw_control_getter_setter(text_align, alignment);
  yw_control_getter_setter(text_color, color);
  auto text_color(this auto& Self, none None) noexcept {
    return typename remove_cvref<decltype(Self)>::proxy(get_slot(&Self)).text_color(None);
  }
  yw_control_getter_setter(string, yw::string<wchar_t>);
  yw_control_getter_setter(font, font_config);
  yw_control_getter(pressed);
  yw_control_getter_setter(click_event, function<bool, yw::button_event>);
};
} // namespace yw::ui
