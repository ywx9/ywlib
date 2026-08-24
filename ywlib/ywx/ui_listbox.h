#pragma once
#include <ywx/text.h>
#include <ywx/ui_scrollbar.h>

namespace yw::ui {

class listbox : public scrollbar<orientation::vertical> {
public:
  struct slot : scrollbar<orientation::vertical>::slot {
    std::vector<yw::text> items{};
    optional<color> text_color;
    optional<color> selection_color;
    optional<color> pressed_item_color;
    float4 item_padding = float4::fill(arbitrary_value);
    size_t visible_item_count = 4;
    size_t selected_index = npos;
    size_t pressed_index = npos;
    size_t hovered_index = npos;

    function<bool, size_t> change_event{};

    virtual bool is_focusable() const override { return enabled && visible; }

    virtual color get_text_color(const interface::slot* Window) const noexcept {
      if (text_color) return *text_color;
      if (auto theme = get_color_theme(Window)) return (*theme)->text;
      return colors::transparent;
    }

    virtual color get_selection_color(const interface::slot* Window) const noexcept {
      if (selection_color) return *selection_color;
      if (auto theme = get_color_theme(Window)) return color((*theme)->accent, default_overlay_opacity.selection);
      return colors::transparent;
    }

    virtual color get_pressed_item_color(const interface::slot* Window) const noexcept {
      if (pressed_item_color) return *pressed_item_color;
      if (auto theme = get_color_theme(Window)) return color((*theme)->accent, default_overlay_opacity.press);
      return colors::transparent;
    }

    virtual std::expected<void, error> draw_forecontent(interface::slot* Window) override {
      if (auto res = draw_items(Window); !res) return res.error().relay();
      if (auto res = draw_item_hover_overlay(Window); !res) return res.error().relay();
      if (auto res = scrollbar<orientation::vertical>::slot::draw_forecontent(Window); !res) return res.error().relay();
      return {};
    }

    virtual std::expected<float2, error> get_necessary_size() const override {
      float2 inner{};
      size_t count = 0;
      for (size_t i = 0; i < items.size(); ++i) {
        inner.x = yw::max(inner.x, items[i].size().x + item_padding.x + item_padding.z);
        if (count++ < visible_item_count) inner.y += item_height(i);
      }
      inner += padding.xy() + padding.zw();
      return calc_necessary_size_by_policy(inner.add<0>(bar_width));
    }

    virtual void reset_state() override {
      scrollbar<orientation::vertical>::slot::reset_state();
      if (pressed_index == npos) return;
      pressed_index = npos;
      make_dirty();
    }

    virtual bool handle_button_event(yw::button_event e) override {
      if (!enabled || !visible || e.key != keys::lbutton) return false;
      update_content_size();
      if (scrollbar<orientation::vertical>::slot::handle_button_event(e)) return true;
      const auto hit = item_at(float2(float(e.pos.x), float(e.pos.y)));
      if (e.down) {
        if (hit == npos) return false;
        pressed_index = hit;
        make_dirty();
        return select(hit);
      }
      if (pressed_index == npos) return false;
      pressed_index = npos;
      make_dirty();
      return true;
    }

    virtual bool handle_click_event(yw::button_event e) override {
      if (!enabled || !visible || e.down || e.key != keys::lbutton) return false;
      update_content_size();
      if (scrollbar<orientation::vertical>::slot::handle_click_event(e)) return true;
      pressed_index = npos;
      return contains_content(float2(float(e.pos.x), float(e.pos.y)));
    }

    virtual bool handle_drag_event(yw::drag_event e) override {
      if (scrollbar<orientation::vertical>::slot::handle_drag_event(e)) return true;
      return pressed_index != npos;
    }

    virtual bool handle_focus_event(yw::focus_event e) override {
      if (!e.focused && pressed_index != npos) {
        pressed_index = npos;
        make_dirty();
      }
      return scrollbar<orientation::vertical>::slot::handle_focus_event(e);
    }

    virtual bool handle_hover_event(yw::hover_event e) override {
      update_content_size();
      const auto next = e.leave() ? npos : item_at(float2(float(e.pos.x), float(e.pos.y)));
      if (hovered_index != next) {
        hovered_index = next;
        make_dirty();
      }
      return scrollbar<orientation::vertical>::slot::handle_hover_event(e);
    }

    virtual bool handle_key_event(yw::key_event e) override {
      if (!enabled || !visible || items.empty()) return false;
      const bool list_key = e.key == keys::up || e.key == keys::down || e.key == keys::page_up ||
                            e.key == keys::page_down || e.key == keys::home || e.key == keys::end;
      if (!e.down) return list_key || control::slot::handle_key_event(e);
      const auto old = selected_index == npos ? 0 : selected_index;
      if (e.key == keys::up) return select(old == 0 ? 0 : old - 1);
      if (e.key == keys::down) return select(yw::min(old + 1, items.size() - 1));
      if (e.key == keys::home) return select(0);
      if (e.key == keys::end) return select(items.size() - 1);
      if (e.key == keys::page_up || e.key == keys::page_down) return select(page_item(old, e.key == keys::page_down));
      return control::slot::handle_key_event(e);
    }

    virtual float2 get_content_size() const override { return float2(content_width(), content_height()); }

    bool contains_content(float2 Pt) const noexcept {
      const auto lt = content_origin();
      const auto rb = lt + content_area();
      return lt.x <= Pt.x && Pt.x < rb.x && lt.y <= Pt.y && Pt.y < rb.y;
    }

    std::expected<void, error> draw_item_hover_overlay(interface::slot* Window) const {
      if (hovered_index == npos || hovered_index >= items.size()) return {};
      const auto wsp = static_cast<window::slot*>(Window);
      if (!wsp) return {};
      if (wsp->hover_overlay_color.a <= 0.0f) return {};
      const auto rect = item_rect(hovered_index);
      if (!visible_rect(rect)) return {};
      brush::color(wsp->hover_overlay_color);
      if (auto res = fill_rectangle(clipped_item_rect(rect)); !res) return res.error().relay();
      return {};
    }

    std::expected<void, error> draw_items(interface::slot* Window) const {
      const auto origin = content_origin();
      const auto area = content_area();
      float y = origin.y - scroll_offset;
      for (size_t i = 0; i < items.size(); ++i) {
        const auto h = item_height(i);
        const auto row = float4(origin.x, y, origin.x + area.x, y + h);
        if (visible_rect(row)) {
          if (i == selected_index) {
            brush::color(get_selection_color(Window));
            if (auto res = fill_rectangle(clipped_item_rect(row)); !res) return res.error().relay();
          } else if (i == pressed_index) {
            brush::color(get_pressed_item_color(Window));
            if (auto res = fill_rectangle(clipped_item_rect(row)); !res) return res.error().relay();
          }
          const auto text_pos = float2(origin.x + item_padding.x, y + item_padding.y);
          if (auto res = yw::draw_text(text_pos, items[i], get_text_color(Window)); !res) return res.error().relay();
        }
        y += h;
        if (y > origin.y + area.y) break;
      }
      return {};
    }

    void ensure_visible(size_t Index) noexcept {
      if (Index >= items.size()) return;
      const auto top = item_top(Index);
      const auto bottom = top + item_height(Index);
      auto next = scroll_offset;
      const auto area = content_area();
      if (top < next) next = top;
      if (bottom > next + area.y) next = bottom - area.y;
      scroll_to(next);
    }

    float item_height(size_t Index) const noexcept { return items[Index].size().y + item_padding.y + item_padding.w; }

    float4 item_rect(size_t Index) const noexcept {
      const auto origin = content_origin();
      const auto y = origin.y + item_top(Index) - scroll_offset;
      return float4(origin.x, y, origin.x + content_area().x, y + item_height(Index));
    }

    float item_top(size_t Index) const noexcept {
      float y = 0.0f;
      for (size_t i = 0; i < Index && i < items.size(); ++i) y += item_height(i);
      return y;
    }

    size_t item_at(float2 Pt) const noexcept {
      if (!contains_content(Pt)) return npos;
      const auto y = Pt.y - content_origin().y + scroll_offset;
      float top = 0.0f;
      for (size_t i = 0; i < items.size(); ++i) {
        const auto bottom = top + item_height(i);
        if (top <= y && y < bottom) return i;
        top = bottom;
      }
      return npos;
    }

    size_t page_item(size_t Base, bool Down) const noexcept {
      auto y = item_top(Base) + (Down ? content_area().y : -content_area().y);
      y = yw::clamp(y, 0.0f, yw::max(0.0f, content_height() - 1.0f));
      float top = 0.0f;
      for (size_t i = 0; i < items.size(); ++i) {
        const auto bottom = top + item_height(i);
        if (y < bottom) return i;
        top = bottom;
      }
      return items.empty() ? npos : items.size() - 1;
    }

    bool select(size_t Index, bool Notify = true) noexcept {
      if (Index >= items.size() || selected_index == Index) return true;
      selected_index = Index;
      ensure_visible(Index);
      make_dirty();
      return Notify && change_event ? change_event(selected_index) : true;
    }

    float content_height() const noexcept {
      float h = 0.0f;
      for (size_t i = 0; i < items.size(); ++i) h += item_height(i);
      return h;
    }

    float content_width() const noexcept {
      float w = 0.0f;
      for (size_t i = 0; i < items.size(); ++i) w = yw::max(w, items[i].size().x + item_padding.x + item_padding.z);
      return w;
    }

    float2 content_origin() const noexcept { return pos + padding.xy(); }

    float2 content_area() const noexcept { return get_viewport_size(); }

    void update_content_size() noexcept { scroll_to(scroll_offset); }

  private:
    float4 clipped_item_rect(float4 Rect) const noexcept {
      const auto origin = content_origin();
      const auto area = content_area();
      return float4(
        yw::max(Rect.x, origin.x), yw::max(Rect.y, origin.y), yw::min(Rect.z, origin.x + area.x),
        yw::min(Rect.w, origin.y + area.y));
    }

    bool visible_rect(float4 Rect) const noexcept {
      const auto origin = content_origin();
      const auto area = content_area();
      return Rect.z > origin.x && Rect.x < origin.x + area.x && Rect.w > origin.y && Rect.y < origin.y + area.y;
    }
  };

  using scrollbar<orientation::vertical>::operator bool;
  class proxy : public control::proxy {
    friend class listbox;
    using control::proxy::proxy;
    listbox::slot* _get_slot() const noexcept { return static_cast<listbox::slot*>(_slot); }

  public:
    //-- getter --//

    size_t item_count() const&& noexcept { return _get_slot()->items.size(); }
    size_t selected_index() const&& noexcept { return _get_slot()->selected_index; }
    const auto& change_event() const&& noexcept { return _get_slot()->change_event; }
    const auto& item_padding() const&& noexcept { return _get_slot()->item_padding; }
    color selection_color() const&& noexcept {
      return _get_slot()->get_selection_color(interface::slot::slots.get(_get_slot()->window_id));
    }
    color text_color() const&& noexcept {
      return _get_slot()->get_text_color(interface::slot::slots.get(_get_slot()->window_id));
    }
    size_t visible_item_count() const&& noexcept { return _get_slot()->visible_item_count; }
    const auto& text(size_t Index) const&& noexcept {
      if (Index >= _get_slot()->items.size()) error(errors::invalid_argument, format("invalid item index: ", Index)).go_off();
      return _get_slot()->items[Index];
    }
    const auto& string(size_t Index) const&& noexcept {
      if (Index >= _get_slot()->items.size()) error(errors::invalid_argument, format("invalid item index: ", Index)).go_off();
      return _get_slot()->items[Index].string();
    }

    //-- setter --//

    auto selected_index(this auto&& Self, size_t Index) noexcept {
      if (Index >= Self._get_slot()->items.size())
        error(errors::invalid_argument, format("invalid item index: ", Index)).fizzle_out();
      else Self._get_slot()->select(Index);
      return std::move(Self);
    }

    auto change_event(this auto&& Self, function<bool, size_t> Event) noexcept {
      Self._get_slot()->change_event = std::move(Event);
      return std::move(Self);
    }

    auto item_padding(this auto&& Self, float4 Padding) noexcept {
      Self._get_slot()->item_padding = Padding;
      Self._get_slot()->update_content_size();
      Self._messy = true;
      return std::move(Self);
    }

    auto selection_color(this auto&& Self, const color& Color) noexcept {
      Self._get_slot()->selection_color = Color;
      Self._dirty = true;
      return std::move(Self);
    }

    auto selection_color(this auto&& Self, none) noexcept {
      Self._get_slot()->selection_color = none();
      Self._dirty = true;
      return std::move(Self);
    }

    auto visible_item_count(this auto&& Self, size_t Count) noexcept {
      if (Count == 0) error(errors::invalid_argument, "listbox visible_item_count must be positive").fizzle_out();
      else Self._get_slot()->visible_item_count = Count, Self._messy = true;
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

    auto string(this auto&& Self, size_t Index, yw::string<wchar_t> String) noexcept {
      if (Index >= Self._get_slot()->items.size())
        error(errors::invalid_argument, format("invalid item index: ", Index)).fizzle_out();
      else {
        Self._get_slot()->items[Index].string(std::move(String));
        Self._get_slot()->update_content_size();
        Self._messy = true;
      }
      return std::move(Self);
    }
  };

  listbox() noexcept = default;

  listbox(derived_from<interface> auto& Parent, const source_line& sl = here()) {
    if (auto res = create(Parent)) *this = std::move(*res);
    else res.error().add_footprint().go_off(sl);
  }

  static std::expected<listbox, error> create(derived_from<interface> auto& Parent) {
    listbox l;
    listbox::slot* sp;
    if (auto res = create_control<listbox>(Parent)) sp = *res;
    else return res.error().relay();
    l._id = sp->id;
    sp->policy = {ui::fit, ui::free};
    return l;
  }

  yw_control_getter(item_count);
  yw_control_getter_setter(selected_index, size_t);
  yw_control_getter_setter(change_event, function<bool, size_t>);
  yw_control_getter_setter(item_padding, float4);
  yw_control_getter_setter(selection_color, color);
  auto selection_color(this auto& Self, none None) noexcept {
    return typename remove_cvref<decltype(Self)>::proxy(get_slot(&Self)).selection_color(None);
  }
  yw_control_getter_setter(text_color, color);
  auto text_color(this auto& Self, none None) noexcept {
    return typename remove_cvref<decltype(Self)>::proxy(get_slot(&Self)).text_color(None);
  }
  yw_control_getter_setter(visible_item_count, size_t);
  const auto& text(size_t Index) const noexcept { return proxy(get_slot(this)).text(Index); }
  const auto& string(size_t Index) const noexcept { return proxy(get_slot(this)).string(Index); }

  auto& add(this auto& self, yw::string<wchar_t> String) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).fizzle_out();
    else {
      sp->items.emplace_back(std::move(String));
      if (sp->selected_index == npos) sp->selected_index = 0;
      sp->update_content_size();
      sp->make_messy();
    }
    return self;
  }

  auto& add(this auto& self, yw::text Text) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).fizzle_out();
    else {
      sp->items.push_back(std::move(Text));
      if (sp->selected_index == npos) sp->selected_index = 0;
      sp->update_content_size();
      sp->make_messy();
    }
    return self;
  }

  auto& clear(this auto& self) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).fizzle_out();
    else {
      sp->items.clear();
      sp->selected_index = sp->pressed_index = sp->hovered_index = npos;
      sp->scroll_to(0.0f);
      sp->make_messy();
    }
    return self;
  }

  auto& erase(this auto& self, size_t Index) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).fizzle_out();
    else if (Index >= sp->items.size()) error(errors::invalid_argument, format("invalid item index: ", Index)).fizzle_out();
    else {
      sp->items.erase(sp->items.begin() + Index);
      if (sp->items.empty()) sp->selected_index = sp->pressed_index = sp->hovered_index = npos;
      else {
        if (sp->selected_index == Index) sp->selected_index = yw::min(Index, sp->items.size() - 1);
        else if (Index < sp->selected_index) --sp->selected_index;
        if (sp->pressed_index == Index) sp->pressed_index = npos;
        else if (Index < sp->pressed_index) --sp->pressed_index;
        if (sp->hovered_index == Index) sp->hovered_index = npos;
        else if (Index < sp->hovered_index) --sp->hovered_index;
      }
      sp->update_content_size();
      sp->make_messy();
    }
    return self;
  }

  auto string(this auto& Self, size_t Index, yw::string<wchar_t> String) noexcept {
    return typename remove_cvref<decltype(Self)>::proxy(get_slot(&Self)).string(Index, std::move(String));
  }
};
} // namespace yw::ui
