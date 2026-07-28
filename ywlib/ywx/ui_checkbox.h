#pragma once
#include <ywx/ui_icon.h>
#include <ywx/ui_label.h>
#include <ywx/window.h>

namespace yw::ui {

class checkbox : public control {
public:
  struct slot : control::slot {
    yw::text text = yw::text(L"");
    yw::icon box;
    yw::icon check;
    float2 icon_size{common_size_value, common_size_value};
    float icon_gap = arbitrary_value;
    bool checked = false;
    bool pressed = false;

    function<void, bool> change_event{};

    //-- override functions --//

    virtual bool is_focusable() const override { return enabled && visible; }
    virtual bool is_interactive() const override { return true; }

    virtual std::expected<void, error> apply_color_theme(const yw::ui::color_theme& Theme, bool) override {
      background_color = Theme.surface;
      border_color = colors::transparent;
      text.color(Theme.text);
      apply_icon_color(box, colors::transparent, Theme.outline);
      apply_icon_color(check, Theme.accent, Theme.accent);
      make_dirty();
      return {};
    }



    virtual std::expected<void, error> draw_backcontent() override {
      const auto origin = float2(pos.x + padding.x, pos.y + (size.y - icon_size.y) * 0.5f);
      if (box.is_bitmap()) {
        if (auto res = draw_bitmap(origin, icon_size, box.get_bitmap()); !res) return res.error().relay();
      } else if (box.is_vector())
        if (auto svg = box.get_vector(); svg.fill_color().a > 0.0f) {
          brush::color(svg.fill_color());
          if (auto res = fill_svgpath(origin, box.get_vector()); !res) return res.error().relay();
        }
      return {};
    }

    virtual std::expected<void, error> draw_forecontent() override {
      const auto origin = float2(pos.x + padding.x, pos.y + (size.y - icon_size.y) * 0.5f);
      if (box.is_vector())
        if (auto svg = box.get_vector(); svg.stroke_color().a > 0.0f) {
          brush::color(svg.stroke_color());
          if (auto res = stroke_svgpath(origin, box.get_vector()); !res) return res.error().relay();
        }
      if (checked)
        if (auto res = draw_icon(origin, icon_size, check); !res) return res.error().relay();
      const auto text_origin = float2(pos.x + padding.x + icon_size.x + icon_gap, pos.y + padding.y);
      const auto text_area = pos + size - padding.zw() - text_origin;
      const auto text_pos = align_position(text_origin, text_area, text.size(), left);
      if (auto res = draw_text(text_pos, text); !res) return res.error().relay();
      return {};
    }

    virtual std::expected<void, error> draw_overlay() override {
      const auto wsp = get_slot<window>(window_id);
      if (!wsp) return std::unexpected(error(errors::invalid_slotid));
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
      toggle();
      return true;
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
      if (was_pressed) toggle();
      return true;
    }

    //-- shared functions --//

    static void apply_icon_color(yw::icon& Icon, const color& FillColor, const color& StrokeColor) noexcept {
      if (auto vector = Icon.get_if_vector()) {
        vector->fill_color(FillColor);
        vector->stroke_color(StrokeColor);
      }
    }

    std::expected<void, error> apply_current_icon_theme() {
      if (auto theme = get_color_theme(); !theme) return theme.error().relay();
      else {
        apply_icon_color(box, colors::transparent, (*theme)->outline);
        apply_icon_color(check, (*theme)->accent, (*theme)->accent);
      }
      return {};
    }

    void toggle() {
      checked = !checked;
      make_dirty();
      if (change_event) change_event(checked);
    }
  };

  checkbox() noexcept = default;

  checkbox(derived_from<interface> auto& Parent, const source_line& sl = here()) {
    if (auto res = create(Parent)) *this = std::move(*res);
    else res.error().add_footprint().go_off(sl);
  }

  static std::expected<checkbox, error> create(derived_from<interface> auto& Parent) {
    checkbox c;
    const auto temp_id = make_slot<checkbox>();
    const auto sp = get_slot<checkbox>(temp_id);
    if (!sp) return std::unexpected(error(errors::slot_creation_failed));
    const auto psp = get_slot<control>(Parent.id());
    if (!psp) return std::unexpected(error(errors::invalid_slotid));
    if (auto res = psp->attach(temp_id); !res) {
      slot::slots.erase(temp_id);
      return res.error().relay();
    }
    c._id = temp_id;
    sp->id = temp_id;
    sp->window_id = psp->get_window_id();
    sp->policy = {ui::size_policy::fit, ui::size_policy::fit};
    constexpr float2 init_icon_size{16.0f, 16.0f};
    sp->icon_size = init_icon_size;
    sp->box = yw::icon(yw::svgpath(init_icon_size, "M1 1 L15 1 L15 15 L1 15 Z"));
    sp->check = yw::icon(yw::svgpath(init_icon_size, "M3 8 L7 12 L13 4 L7 10 Z"));
    if (auto theme = sp->get_color_theme(); !theme) return theme.error().relay();
    else if (auto res = sp->apply_color_theme(*(*theme), false); !res) return res.error().relay();
    return c;
  }

  //-- getter --//

  bool checked() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return {};
    }
    return sp->checked;
  }

  bool pressed() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return {};
    }
    return sp->pressed;
  }

  const auto& text() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->text;
  }

  const auto& box() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->box;
  }

  const auto& check() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->check;
  }

  const auto& change_event() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->change_event;
  }

  const auto& text_color() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->text.color();
  }

  float2 icon_size() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return {};
    }
    return sp->icon_size;
  }

  float icon_gap() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return {};
    }
    return sp->icon_gap;
  }

  const auto& string() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->text.string();
  }

  const auto& font() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->text.font();
  }

  //-- setter --//

  auto& checked(this auto& self, bool b) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    if (sp->checked == b) return self;
    sp->checked = b;
    sp->make_dirty();
    if (sp->change_event) sp->change_event(sp->checked);
    return self;
  }

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

  auto& box(this auto& self, yw::icon Icon) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    sp->box = std::move(Icon);
    if (auto res = sp->apply_current_icon_theme(); !res) {
      res.error().fizzle_out();
      return self;
    }
    sp->make_messy();
    return self;
  }

  auto& check(this auto& self, yw::icon Icon) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    sp->check = std::move(Icon);
    if (auto res = sp->apply_current_icon_theme(); !res) {
      res.error().fizzle_out();
      return self;
    }
    sp->make_messy();
    return self;
  }

  auto& change_event(this auto& self, function<void, bool> f) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    sp->change_event = std::move(f);
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

  auto& icon_size(this auto& self, float2 v) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    if (v.x <= 0.0f || v.y <= 0.0f) {
      error(errors::invalid_argument, format("icon_size must be positive: ", v)).fizzle_out();
      return self;
    }
    sp->icon_size = v;
    sp->make_messy();
    return self;
  }

  auto& icon_gap(this auto& self, float1 f) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    sp->icon_gap = f.x;
    sp->make_messy();
    return self;
  }
};
} // namespace yw::ui
