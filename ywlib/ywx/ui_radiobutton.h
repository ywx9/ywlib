#pragma once
#include <ywx/ui_icon.h>
#include <ywx/ui_label.h>
#include <ywx/window.h>

namespace yw::ui {

class radiobutton : public control {
public:
  struct item {
    yw::text caption{};
    float2 pos{};
    float2 size{};
    float2 icon_pos{};
    float2 text_pos{};
  };

  struct slot : control::slot {
    std::vector<item> items{};
    yw::icon circle;
    yw::icon dot;
    float2 icon_size{common_size_value, common_size_value};
    float icon_gap = arbitrary_value;
    float item_gap = arbitrary_value;
    size_t checked_index = npos;
    size_t selected_index = npos;
    size_t pressed_index = npos;
    size_t hovered_index = npos;

    function<void, size_t> change_event{};

    //-- override functions --//

    virtual bool is_focusable() const override { return enabled && visible; }
    virtual bool is_interactive() const override { return true; }

    virtual std::expected<void, error> apply_color_theme(const yw::ui::color_theme& Theme, bool) override {
      background_color = Theme.surface;
      border_color = colors::transparent;
      for (auto& it : items) it.caption.color(Theme.text);
      apply_icon_color(circle, colors::transparent, Theme.outline);
      apply_icon_color(dot, Theme.accent, Theme.accent);
      make_dirty();
      return {};
    }

    virtual std::expected<void, error> draw_backcontent() override {
      if (circle.is_bitmap()) {
        for (const auto& it : items)
          if (auto res = draw_bitmap(it.icon_pos, icon_size, circle.get_bitmap()); !res) return res.error().relay();
      } else if (circle.is_vector())
        if (const auto& svg = circle.get_vector(); svg.fill_color().a > 0.0f) {
          brush::color(svg.fill_color());
          for (const auto& it : items)
            if (auto res = fill_svgpath(it.icon_pos, circle.get_vector()); !res) return res.error().relay();
        }
      return {};
    }

    virtual std::expected<void, error> draw_forecontent() override {
      if (circle.is_vector())
        if (const auto& svg = circle.get_vector(); svg.stroke_color().a > 0.0f)
          for (const auto& it : items)
            if (auto res = stroke_svgpath(it.icon_pos, circle.get_vector()); !res) return res.error().relay();
      if (checked_index < items.size())
        if (auto res = draw_icon(items[checked_index].icon_pos, icon_size, dot); !res) return res.error().relay();
      for (const auto& it : items)
        if (auto res = draw_text(it.text_pos, it.caption); !res) return res.error().relay();
      return {};
    }

    virtual std::expected<void, error> draw_overlay() override {
      const auto wsp = get_slot<window>(window_id);
      if (!wsp) return std::unexpected(error(errors::invalid_slotid));
      if (pressed_index < items.size() && wsp->press_overlay_color.a > 0.0f) {
        brush::color(wsp->press_overlay_color);
        if (auto res = fill_rectangle(items[pressed_index].pos, items[pressed_index].size); !res)
          return res.error().relay();
      } else if (hovered_index < items.size() && wsp->hover_overlay_color.a > 0.0f) {
        brush::color(wsp->hover_overlay_color);
        if (auto res = fill_rectangle(items[hovered_index].pos, items[hovered_index].size); !res)
          return res.error().relay();
      }
      return {};
    }

    virtual std::expected<void, error> draw_foreground() override {
      if (border_color.a > 0.0f && border_thickness > 0.0f) {
        brush::color(border_color);
        if (auto res = stroke_geometry(geometry.get(), border_thickness); !res) return res.error().relay();
      }
      if (!is_focused() || selected_index >= items.size()) return {};
      auto theme = get_color_theme();
      if (!theme) return theme.error().relay();
      brush::color((*theme)->accent);
      if (auto res = stroke_rectangle(items[selected_index].pos, items[selected_index].size, border_thickness + 1.0f);
        !res)
        return res.error().relay();
      return {};
    }

    virtual std::expected<float2, error> get_necessary_size() const override {
      float2 inner{};
      for (size_t i = 0; i < items.size(); ++i) {
        const auto row_size = get_row_size(items[i]);
        inner.x = yw::max(inner.x, row_size.x);
        inner.y += row_size.y;
        if (i + 1 < items.size()) inner.y += item_gap;
      }
      inner += padding.xy() + padding.zw();
      return calc_necessary_size_by_policy(inner);
    }

    virtual std::expected<void, error> relocate() override {
      if (auto res = control::slot::relocate(); !res) return res.error().relay();
      relocate_items();
      return {};
    }

    virtual void reset_state() override {
      if (pressed_index == npos) return;
      pressed_index = npos;
      make_dirty();
    }

    virtual bool handle_button_event(yw::button_event e) override {
      if (!enabled || !visible || e.key != keys::lbutton) return false;
      const auto next_pressed = e.down ? item_at(float2(float(e.pos.x), float(e.pos.y))) : npos;
      if (pressed_index == next_pressed) return true;
      pressed_index = next_pressed;
      make_dirty();
      return true;
    }

    virtual bool handle_click_event(yw::button_event e) override {
      if (!enabled || !visible || e.down || e.key != keys::lbutton) return false;
      const auto hit = item_at(float2(float(e.pos.x), float(e.pos.y)));
      if (hit == npos) return false;
      selected_index = hit;
      check(hit);
      return true;
    }

    virtual bool handle_focus_event(yw::focus_event e) override {
      if (!e.focused && pressed_index != npos) pressed_index = npos;
      make_dirty();
      return control::slot::handle_focus_event(e);
    }

    virtual bool handle_hover_event(yw::hover_event e) override {
      const auto next_hovered = e.leave() ? npos : item_at(e.pos);
      if (hovered_index == next_hovered) return control::slot::handle_hover_event(e);
      hovered_index = next_hovered;
      make_dirty();
      return control::slot::handle_hover_event(e);
    }

    virtual bool handle_key_event(yw::key_event e) override {
      if (!enabled || !visible || items.empty()) return false;
      if (e.key == keys::up || e.key == keys::left || e.key == keys::down || e.key == keys::right) {
        if (!e.down) return true;
        move_selection(e.key == keys::up || e.key == keys::left);
        return true;
      }
      if (e.key != keys::space && e.key != keys::enter) return false;
      const auto target = selected_index == npos ? 0 : selected_index;
      if (e.down) {
        if (pressed_index != target) {
          pressed_index = target;
          make_dirty();
        }
        return true;
      }
      const bool was_pressed = pressed_index == target;
      pressed_index = npos;
      if (was_pressed) check(target);
      make_dirty();
      return true;
    }

    //-- shared functions --//

    static void apply_icon_color(yw::icon& Icon, const color& FillColor, const color& StrokeColor) noexcept {
      if (auto vector = Icon.get_if_vector()) {
        vector->fill_color(FillColor);
        vector->stroke_color(StrokeColor);
      }
    }

    std::expected<void, error> apply_current_theme_to_content() {
      if (auto theme = get_color_theme(); !theme) return theme.error().relay();
      else {
        for (auto& it : items) it.caption.color((*theme)->text);
        apply_icon_color(circle, colors::transparent, (*theme)->outline);
        apply_icon_color(dot, (*theme)->accent, (*theme)->accent);
      }
      return {};
    }

    float2 get_row_size(const item& Item) const noexcept {
      const auto text_size = Item.caption.size();
      const auto gap = text_size.x > 0.0f ? icon_gap : 0.0f;
      return {icon_size.x + gap + text_size.x, yw::max(icon_size.y, text_size.y)};
    }

    size_t item_at(float2 Pt) const noexcept {
      for (size_t i = 0; i < items.size(); ++i) {
        const auto r = float4(items[i].pos, items[i].pos + items[i].size);
        if (Pt.x >= r.x && Pt.y >= r.y && Pt.x <= r.z && Pt.y <= r.w) return i;
      }
      return npos;
    }

    void check(size_t Index) {
      if (Index >= items.size() || checked_index == Index) return;
      checked_index = Index;
      make_dirty();
      if (change_event) change_event(checked_index);
    }

    void move_selection(bool Backward) {
      if (items.empty()) return;
      if (selected_index == npos || selected_index >= items.size()) selected_index = checked_index;
      if (selected_index == npos || selected_index >= items.size()) selected_index = 0;
      else if (Backward) selected_index = selected_index == 0 ? items.size() - 1 : selected_index - 1;
      else selected_index = (selected_index + 1) % items.size();
      make_dirty();
    }

    void relocate_items() noexcept {
      const auto content_pos = pos + padding.xy();
      const auto content_size = size - padding.xy() - padding.zw();
      float y = content_pos.y;
      for (size_t i = 0; i < items.size(); ++i) {
        auto& it = items[i];
        const auto row_size = get_row_size(it);
        const auto text_size = it.caption.size();
        const auto gap = text_size.x > 0.0f ? icon_gap : 0.0f;
        it.pos = {content_pos.x, y};
        it.size = {content_size.x, row_size.y};
        it.icon_pos = {content_pos.x, y + (row_size.y - icon_size.y) * 0.5f};
        it.text_pos = {content_pos.x + icon_size.x + gap, y + (row_size.y - text_size.y) * 0.5f};
        // it.text_pos = align_position(
        //   it.icon_pos.add<0>(icon_size.x + gap), content_size.add<0>(-icon_size.x - gap), text_size, left);
        y += row_size.y + (i + 1 < items.size() ? item_gap : 0.0f);
      }
    }
  };

  radiobutton() noexcept = default;

  radiobutton(derived_from<interface> auto& Parent, const source_line& sl = here()) {
    if (auto res = create(Parent)) *this = std::move(*res);
    else res.error().add_footprint().go_off(sl);
  }

  static std::expected<radiobutton, error> create(derived_from<interface> auto& Parent) {
    radiobutton r;
    const auto temp_id = make_slot<radiobutton>();
    const auto sp = get_slot<radiobutton>(temp_id);
    if (!sp) return std::unexpected(error(errors::slot_creation_failed));
    const auto psp = get_slot<control>(Parent.id());
    if (!psp) return std::unexpected(error(errors::invalid_slotid));
    if (auto res = psp->attach(temp_id); !res) {
      slot::slots.erase(temp_id);
      return res.error().relay();
    }
    r._id = temp_id;
    sp->id = temp_id;
    sp->window_id = psp->get_window_id();
    sp->policy = {ui::size_policy::fit, ui::size_policy::fit};
    constexpr float2 init_icon_size{16.0f, 16.0f};
    sp->icon_size = init_icon_size;
    sp->circle = yw::icon(
      yw::svgpath(
        init_icon_size,
        "M8 1 C4.134 1 1 4.134 1 8 C1 11.866 4.134 15 8 15 C11.866 15 15 11.866 15 8 C15 4.134 11.866 1 8 1 Z"));
    sp->dot = yw::icon(
      yw::svgpath(
        init_icon_size,
        "M8 5 C6.343 5 5 6.343 5 8 C5 9.657 6.343 11 8 11 C9.657 11 11 9.657 11 8 C11 6.343 9.657 5 8 5 Z"));
    if (auto theme = sp->get_color_theme(); !theme) return theme.error().relay();
    else if (auto res = sp->apply_color_theme(*(*theme), false); !res) return res.error().relay();
    return r;
  }

  //-- getter --//

  size_t checked_index() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return {};
    }
    return sp->checked_index;
  }

  size_t selected_index() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return {};
    }
    return sp->selected_index;
  }

  size_t item_count() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return {};
    }
    return sp->items.size();
  }

  const auto& change_event() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->change_event;
  }

  const auto& circle() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->circle;
  }

  const auto& dot() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->dot;
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

  float item_gap() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return {};
    }
    return sp->item_gap;
  }

  const auto& string(size_t Index) const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    if (Index >= sp->items.size()) error(errors::invalid_argument, format("invalid item index: ", Index)).go_off();
    return sp->items[Index].caption.string();
  }

  //-- setter --//

  auto& add(this auto& self, yw::string<wchar_t> s) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    item it;
    it.caption = yw::text(std::move(s));
    sp->items.push_back(std::move(it));
    if (auto res = sp->apply_current_theme_to_content(); !res) {
      res.error().fizzle_out();
      return self;
    }
    if (sp->selected_index == npos) sp->selected_index = 0;
    if (sp->checked_index == npos) sp->checked_index = 0;
    sp->make_messy();
    return self;
  }

  auto& string(this auto& self, size_t Index, yw::string<wchar_t> s) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    if (Index >= sp->items.size()) {
      error(errors::invalid_argument, format("invalid item index: ", Index)).fizzle_out();
      return self;
    }
    sp->items[Index].caption.string(std::move(s));
    sp->make_messy();
    return self;
  }

  auto& checked_index(this auto& self, size_t Index) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    if (Index >= sp->items.size()) {
      error(errors::invalid_argument, format("invalid item index: ", Index)).fizzle_out();
      return self;
    }
    sp->selected_index = Index;
    sp->check(Index);
    return self;
  }

  auto& selected_index(this auto& self, size_t Index) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    if (Index >= sp->items.size()) {
      error(errors::invalid_argument, format("invalid item index: ", Index)).fizzle_out();
      return self;
    }
    sp->selected_index = Index;
    sp->make_dirty();
    return self;
  }

  auto& change_event(this auto& self, function<void, size_t> f) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    sp->change_event = std::move(f);
    return self;
  }

  auto& font(this auto& self, font_config f) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    for (auto& it : sp->items) it.caption.font(f);
    sp->make_messy();
    return self;
  }

  auto& text_color(this auto& self, const color& c) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    for (auto& it : sp->items) it.caption.color(c);
    sp->make_dirty();
    return self;
  }

  auto& circle(this auto& self, yw::icon Icon) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    sp->circle = std::move(Icon);
    if (auto res = sp->apply_current_theme_to_content(); !res) {
      res.error().fizzle_out();
      return self;
    }
    sp->make_dirty();
    return self;
  }

  auto& dot(this auto& self, yw::icon Icon) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    sp->dot = std::move(Icon);
    if (auto res = sp->apply_current_theme_to_content(); !res) {
      res.error().fizzle_out();
      return self;
    }
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

  auto& item_gap(this auto& self, float1 f) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    sp->item_gap = f.x;
    sp->make_messy();
    return self;
  }
};
} // namespace yw::ui
