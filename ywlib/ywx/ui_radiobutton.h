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
    optional<color> text_color;
    optional<color> circle_fill_color;
    optional<color> circle_stroke_color;
    optional<color> dot_fill_color;
    optional<color> dot_stroke_color;
    float2 icon_size{common_size_value, common_size_value};
    float icon_gap = arbitrary_value;
    float item_gap = arbitrary_value;
    size_t checked_index = npos;
    size_t selected_index = npos;
    size_t pressed_index = npos;
    size_t hovered_index = npos;

    function<bool, size_t> change_event{};

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

    virtual color get_circle_fill_color(const interface::slot*) const noexcept {
      return circle_fill_color ? *circle_fill_color : colors::transparent;
    }

    virtual color get_circle_stroke_color(const interface::slot* Window) const noexcept {
      if (circle_stroke_color) return *circle_stroke_color;
      if (auto theme = get_color_theme(Window)) return (*theme)->outline;
      return colors::transparent;
    }

    virtual color get_dot_fill_color(const interface::slot* Window) const noexcept {
      if (dot_fill_color) return *dot_fill_color;
      if (auto theme = get_color_theme(Window)) return (*theme)->accent;
      return colors::transparent;
    }

    virtual color get_dot_stroke_color(const interface::slot* Window) const noexcept {
      if (dot_stroke_color) return *dot_stroke_color;
      if (auto theme = get_color_theme(Window)) return (*theme)->accent;
      return colors::transparent;
    }

    virtual std::expected<void, error> draw_backcontent(interface::slot* Window) override {
      if (circle.is_bitmap()) {
        for (const auto& it : items)
          if (auto res = draw_bitmap(it.icon_pos, icon_size, circle.get_bitmap()); !res) return res.error().relay();
      } else if (circle.is_vector())
        if (const auto fill = get_circle_fill_color(Window); fill.a > 0.0f) {
          const auto& vector = circle.get_vector();
          for (const auto& it : items)
            if (auto res = fill_svgpath(it.icon_pos, vector.path, fill); !res) return res.error().relay();
        }
      return {};
    }

    virtual std::expected<void, error> draw_forecontent(interface::slot* Window) override {
      if (circle.is_vector())
        if (const auto stroke = get_circle_stroke_color(Window); stroke.a > 0.0f) {
          const auto& vector = circle.get_vector();
          for (const auto& it : items)
            if (auto res = stroke_svgpath(it.icon_pos, vector.path, stroke, vector.stroke_width); !res)
              return res.error().relay();
        }
      if (checked_index < items.size()) {
        const auto& it = items[checked_index];
        if (dot.is_bitmap()) {
          if (auto res = draw_bitmap(it.icon_pos, icon_size, dot.get_bitmap()); !res) return res.error().relay();
        } else if (dot.is_vector()) {
          const auto& vector = dot.get_vector();
          if (const auto fill = get_dot_fill_color(Window); fill.a > 0.0f)
            if (auto res = fill_svgpath(it.icon_pos, icon_size, vector.path, fill); !res) return res.error().relay();
          if (const auto stroke = get_dot_stroke_color(Window); stroke.a > 0.0f && vector.stroke_width > 0.0f)
            if (auto res = stroke_svgpath(it.icon_pos, icon_size, vector.path, stroke, vector.stroke_width); !res)
              return res.error().relay();
        }
      }
      for (const auto& it : items)
        if (auto res = draw_text(it.text_pos, it.caption, get_text_color(Window)); !res) return res.error().relay();
      return {};
    }

    virtual std::expected<void, error> draw_overlay(interface::slot* Window) override {
      const auto wsp = static_cast<window::slot*>(Window);
      if (!wsp) return {};
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

    virtual std::expected<void, error> draw_foreground(interface::slot* Window) override {
      const auto border = get_border_color(Window);
      if (border.a > 0.0f && border_thickness > 0.0f) {
        brush::color(border);
        if (auto res = stroke_geometry(geometry.get(), border_thickness); !res) return res.error().relay();
      }
      if (!is_focused() || selected_index >= items.size()) return {};
      auto theme = get_color_theme(Window);
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
      return check(hit);
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
      make_dirty();
      return was_pressed ? check(target) : true;
    }

    //-- shared functions --//

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

    bool check(size_t Index) {
      if (Index >= items.size() || checked_index == Index) return true;
      checked_index = Index;
      make_dirty();
      return change_event ? call_event(change_event, checked_index) : true;
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

  class proxy : public control::proxy {
    friend class radiobutton;
    using control::proxy::proxy;
    radiobutton::slot* _get_slot() const noexcept { return static_cast<radiobutton::slot*>(_slot); }

  public:
    //-- getter --//

    size_t checked_index() const&& noexcept { return _get_slot()->checked_index; }
    size_t selected_index() const&& noexcept { return _get_slot()->selected_index; }
    size_t item_count() const&& noexcept { return _get_slot()->items.size(); }
    const auto& change_event() const&& noexcept { return _get_slot()->change_event; }
    const auto& circle() const&& noexcept { return _get_slot()->circle; }
    const auto& dot() const&& noexcept { return _get_slot()->dot; }
    float2 icon_size() const&& noexcept { return _get_slot()->icon_size; }
    float icon_gap() const&& noexcept { return _get_slot()->icon_gap; }
    float item_gap() const&& noexcept { return _get_slot()->item_gap; }
    color text_color() const&& noexcept {
      return _get_slot()->get_text_color(interface::slot::slots.get(_get_slot()->window_id));
    }
    const auto& string(size_t Index) const&& noexcept {
      if (Index >= _get_slot()->items.size()) error(errors::invalid_argument, format("invalid item index: ", Index)).go_off();
      return _get_slot()->items[Index].caption.string();
    }

    //-- setter --//

    auto string(this auto&& Self, size_t Index, yw::string<wchar_t> String) noexcept {
      if (Index >= Self._get_slot()->items.size())
        error(errors::invalid_argument, format("invalid item index: ", Index)).fizzle_out();
      else {
        Self._get_slot()->items[Index].caption.string(std::move(String));
        Self._messy = true;
      }
      return std::move(Self);
    }

    auto checked_index(this auto&& Self, size_t Index) noexcept {
      if (Index >= Self._get_slot()->items.size())
        error(errors::invalid_argument, format("invalid item index: ", Index)).fizzle_out();
      else {
        Self._get_slot()->selected_index = Index;
        Self._get_slot()->check(Index);
      }
      return std::move(Self);
    }

    auto selected_index(this auto&& Self, size_t Index) noexcept {
      if (Index >= Self._get_slot()->items.size())
        error(errors::invalid_argument, format("invalid item index: ", Index)).fizzle_out();
      else {
        Self._get_slot()->selected_index = Index;
        Self._dirty = true;
      }
      return std::move(Self);
    }

    auto change_event(this auto&& Self, function<bool, size_t> Event) noexcept {
      Self._get_slot()->change_event = std::move(Event);
      return std::move(Self);
    }

    auto font(this auto&& Self, font_config Font) noexcept {
      for (auto& it : Self._get_slot()->items) it.caption.font(Font);
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

    auto circle(this auto&& Self, yw::icon Icon) noexcept {
      Self._get_slot()->circle = std::move(Icon);
      Self._dirty = true;
      return std::move(Self);
    }

    auto dot(this auto&& Self, yw::icon Icon) noexcept {
      Self._get_slot()->dot = std::move(Icon);
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

    auto item_gap(this auto&& Self, float1 Gap) noexcept {
      Self._get_slot()->item_gap = Gap.x;
      Self._messy = true;
      return std::move(Self);
    }
  };

  radiobutton() noexcept = default;

  radiobutton(derived_from<interface> auto& Parent, const source_line& sl = here()) {
    if (auto res = create(Parent)) *this = std::move(*res);
    else res.error().add_footprint().go_off(sl);
  }

  static std::expected<radiobutton, error> create() {
    radiobutton r;
    radiobutton::slot* sp;
    if (auto res = create_control<radiobutton>()) sp = *res;
    else return res.error().relay();
    r._id = sp->id;
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
    return r;
  }

  static std::expected<radiobutton, error> create(derived_from<interface> auto& Parent) {
    auto res = create();
    if (!res) return res.error().relay();
    if (auto attached = res->attach(Parent); !attached) return attached.error().relay();
    return res;
  }

  yw_control_getter_setter(checked_index, size_t);
  yw_control_getter_setter(selected_index, size_t);
  yw_control_getter(item_count);
  yw_control_getter_setter(change_event, function<bool, size_t>);
  yw_control_getter_setter(circle, yw::icon);
  yw_control_getter_setter(dot, yw::icon);
  yw_control_getter_setter(icon_size, float2);
  yw_control_getter_setter(icon_gap, float1);
  yw_control_getter_setter(item_gap, float1);
  yw_control_getter_setter(text_color, color);
  auto text_color(this auto& Self, none None) noexcept {
    return typename remove_cvref<decltype(Self)>::proxy(get_slot(&Self)).text_color(None);
  }
  const auto& string(size_t Index) const noexcept { return proxy(get_slot(this)).string(Index); }

  auto& add(this auto& self, yw::string<wchar_t> s) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    item it;
    it.caption = yw::text(std::move(s));
    sp->items.push_back(std::move(it));
    if (sp->selected_index == npos) sp->selected_index = 0;
    if (sp->checked_index == npos) sp->checked_index = 0;
    sp->make_messy();
    return self;
  }

  auto string(this auto& Self, size_t Index, yw::string<wchar_t> String) noexcept {
    return typename remove_cvref<decltype(Self)>::proxy(get_slot(&Self)).string(Index, std::move(String));
  }
  auto font(this auto& Self, font_config Font) noexcept {
    return typename remove_cvref<decltype(Self)>::proxy(get_slot(&Self)).font(std::move(Font));
  }

};
} // namespace yw::ui
