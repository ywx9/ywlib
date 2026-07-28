#pragma once
#include <ywx/text.h>
#include <ywx/ui_scrollbar.h>

namespace yw::ui {

class listbox : public scrollbar<orientation::vertical> {
public:
  struct slot : scrollbar<orientation::vertical>::slot {
    std::vector<yw::text> items{};
    color text_color = colors::black;
    color selection_color = color(colors::dodgerblue, 0.35f);
    color pressed_item_color = color(colors::black, 0.08f);
    float4 item_padding = float4::fill(arbitrary_value);
    size_t selected_index = npos;
    size_t pressed_index = npos;
    size_t hovered_index = npos;

    function<void, size_t> change_event{};

    virtual bool is_focusable() const override { return enabled && visible; }

    virtual std::expected<void, error> apply_color_theme(const color_theme& Theme, bool Recursive) override {
      if (auto res = scrollbar<orientation::vertical>::slot::apply_color_theme(Theme, Recursive); !res)
        return res.error().relay();
      background_color = Theme.surface;
      text_color = Theme.text;
      selection_color = color(Theme.accent, default_overlay_opacity.selection);
      pressed_item_color = color(Theme.accent, default_overlay_opacity.press);
      for (auto& item : items) item.color(text_color);
      make_dirty();
      return {};
    }

    virtual std::expected<void, error> draw_forecontent() override {
      if (auto res = draw_items(); !res) return res.error().relay();
      if (auto res = draw_item_hover_overlay(); !res) return res.error().relay();
      if (auto res = scrollbar<orientation::vertical>::slot::draw_forecontent(); !res) return res.error().relay();
      return {};
    }

    virtual std::expected<float2, error> get_necessary_size() const override {
      float2 inner{};
      size_t count = 0;
      for (size_t i = 0; i < items.size(); ++i) {
        inner.x = yw::max(inner.x, items[i].size().x + item_padding.x + item_padding.z);
        if (count++ < 4) inner.y += item_height(i);
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
        select(hit);
        make_dirty();
        return true;
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
      if (e.key == keys::up) return select(old == 0 ? 0 : old - 1), true;
      if (e.key == keys::down) return select(yw::min(old + 1, items.size() - 1)), true;
      if (e.key == keys::home) return select(0), true;
      if (e.key == keys::end) return select(items.size() - 1), true;
      if (e.key == keys::page_up || e.key == keys::page_down) return select(page_item(old, e.key == keys::page_down)), true;
      return control::slot::handle_key_event(e);
    }

    virtual float2 get_content_size() const override { return float2(content_width(), content_height()); }

    bool contains_content(float2 Pt) const noexcept {
      const auto lt = content_origin();
      const auto rb = lt + content_area();
      return lt.x <= Pt.x && Pt.x < rb.x && lt.y <= Pt.y && Pt.y < rb.y;
    }

    std::expected<void, error> draw_item_hover_overlay() const {
      if (hovered_index == npos || hovered_index >= items.size()) return {};
      const auto wsp = get_slot<window>(window_id);
      if (!wsp) return std::unexpected(error(errors::invalid_slotid));
      if (wsp->hover_overlay_color.a <= 0.0f) return {};
      const auto rect = item_rect(hovered_index);
      if (!visible_rect(rect)) return {};
      brush::color(wsp->hover_overlay_color);
      if (auto res = fill_rectangle(clipped_item_rect(rect)); !res) return res.error().relay();
      return {};
    }

    std::expected<void, error> draw_items() const {
      const auto origin = content_origin();
      const auto area = content_area();
      float y = origin.y - scroll_offset;
      for (size_t i = 0; i < items.size(); ++i) {
        const auto h = item_height(i);
        const auto row = float4(origin.x, y, origin.x + area.x, y + h);
        if (visible_rect(row)) {
          if (i == selected_index) {
            brush::color(selection_color);
            if (auto res = fill_rectangle(clipped_item_rect(row)); !res) return res.error().relay();
          } else if (i == pressed_index) {
            brush::color(pressed_item_color);
            if (auto res = fill_rectangle(clipped_item_rect(row)); !res) return res.error().relay();
          }
          const auto text_pos = float2(origin.x + item_padding.x, y + item_padding.y);
          if (auto res = yw::draw_text(text_pos, items[i]); !res) return res.error().relay();
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

    void select(size_t Index, bool Notify = true) noexcept {
      if (Index >= items.size() || selected_index == Index) return;
      selected_index = Index;
      ensure_visible(Index);
      make_dirty();
      if (Notify && change_event) change_event(selected_index);
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
  listbox() noexcept = default;

  listbox(derived_from<interface> auto& Parent, const source_line& sl = here()) {
    if (auto res = create(Parent)) *this = std::move(*res);
    else res.error().add_footprint().go_off(sl);
  }

  static std::expected<listbox, error> create(derived_from<interface> auto& Parent) {
    listbox l;
    const auto temp_id = make_slot<listbox>();
    const auto sp = get_slot<listbox>(temp_id);
    if (!sp) return std::unexpected(error(errors::slot_creation_failed));
    const auto psp = get_slot<control>(Parent.id());
    if (!psp) return std::unexpected(error(errors::invalid_slotid));
    if (auto res = psp->attach(temp_id); !res) {
      slot::slots.erase(temp_id);
      return res.error().relay();
    }
    l._id = temp_id;
    sp->id = temp_id;
    sp->window_id = psp->get_window_id();
    sp->policy = {ui::fit, ui::free};
    if (auto theme = sp->get_color_theme(); !theme) return theme.error().relay();
    else if (auto res = sp->apply_color_theme(*(*theme), false); !res) return res.error().relay();
    return l;
  }

  size_t item_count() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return {};
    }
    return sp->items.size();
  }

  size_t selected_index() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return npos;
    }
    return sp->selected_index;
  }

  const auto& change_event() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->change_event;
  }

  const auto& item_padding() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->item_padding;
  }

  const auto& selection_color() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->selection_color;
  }

  const auto& text(size_t Index) const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    if (Index >= sp->items.size()) error(errors::invalid_argument, format("invalid item index: ", Index)).go_off();
    return sp->items[Index];
  }

  const auto& string(size_t Index) const noexcept { return text(Index).string(); }

  auto& add(this auto& self, yw::string<wchar_t> String) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).fizzle_out();
    else {
      sp->items.emplace_back(std::move(String));
      sp->items.back().color(sp->text_color);
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
      sp->items.back().color(sp->text_color);
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

  auto& selected_index(this auto& self, size_t Index) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).fizzle_out();
    else if (Index >= sp->items.size()) error(errors::invalid_argument, format("invalid item index: ", Index)).fizzle_out();
    else sp->select(Index);
    return self;
  }

  auto& change_event(this auto& self, function<void, size_t> f) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).fizzle_out();
    else sp->change_event = std::move(f);
    return self;
  }

  auto& item_padding(this auto& self, float4 Padding) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).fizzle_out();
    else sp->item_padding = Padding, sp->update_content_size(), sp->make_messy();
    return self;
  }

  auto& selection_color(this auto& self, const color& Color) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).fizzle_out();
    else sp->selection_color = Color, sp->make_dirty();
    return self;
  }

  auto& text_color(this auto& self, const color& Color) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).fizzle_out();
    else {
      sp->text_color = Color;
      for (auto& item : sp->items) item.color(Color);
      sp->make_dirty();
    }
    return self;
  }

  auto& string(this auto& self, size_t Index, yw::string<wchar_t> String) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).fizzle_out();
    else if (Index >= sp->items.size()) error(errors::invalid_argument, format("invalid item index: ", Index)).fizzle_out();
    else sp->items[Index].string(std::move(String)), sp->update_content_size(), sp->make_messy();
    return self;
  }
};
} // namespace yw::ui
