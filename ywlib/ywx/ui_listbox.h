#pragma once
#include <ywx/text.h>
#include <ywx/ui_scrollbar.h>

namespace yw::ui {

class listbox : public scrollbar {
public:
  struct slot : scrollbar::slot {
    std::vector<yw::text> items{};
    color text_color = colors::black;
    color selection_color = color(colors::dodgerblue, 0.35f);
    color pressed_color_item = color(colors::black, 0.08f);
    float4 item_padding = float4::fill(arbitrary_value);
    size_t selected = npos;
    size_t pressed = npos;
    size_t hovered_item = npos;

    function<void, size_t> on_change{};

    float _item_height(size_t Index) const noexcept {
      return items[Index].size().y + item_padding.y + item_padding.w;
    }

    float _item_top(size_t Index) const noexcept {
      float y = 0.0f;
      for (size_t i = 0; i < Index && i < items.size(); ++i) y += _item_height(i);
      return y;
    }

    float _content_height() const noexcept {
      float h = 0.0f;
      for (size_t i = 0; i < items.size(); ++i) h += _item_height(i);
      return h;
    }

    float2 _content_pos() const noexcept { return pos + padding.xy(); }

    bool _contains_content(float2 Pt) const noexcept {
      const auto p = _content_pos();
      const auto s = content_area;
      return Pt.x >= p.x && Pt.y >= p.y && Pt.x <= p.x + s.x && Pt.y <= p.y + s.y;
    }

    virtual bool focusable() const noexcept override { return enabled && visible; }

    virtual float2 get_content_size() const override {
      float2 content{};
      for (size_t i = 0; i < items.size(); ++i) {
        content.x = yw::max(content.x, items[i].size().x + item_padding.x + item_padding.z);
        content.y += _item_height(i);
      }
      return content;
    }

    size_t item_at(float2 Pt) const noexcept {
      if (!_contains_content(Pt)) return npos;
      const auto content_pos = _content_pos();
      const float y = Pt.y - content_pos.y + scroll_offset.y;
      float top = 0.0f;
      for (size_t i = 0; i < items.size(); ++i) {
        const float bottom = top + _item_height(i);
        if (y >= top && y < bottom) return i;
        top = bottom;
      }
      return npos;
    }

    void ensure_visible(size_t Index) {
      if (Index >= items.size()) return;
      const float top = _item_top(Index);
      const float bottom = top + _item_height(Index);
      auto next = scroll_offset;
      const auto content_size = content_area;
      if (top < next.y) next.y = top;
      if (bottom > next.y + content_size.y) next.y = bottom - content_size.y;
      scroll_to(next);
    }

    void select(size_t Index, bool Notify = true) {
      if (Index >= items.size() || selected == Index) return;
      selected = Index;
      ensure_visible(Index);
      make_dirty();
      if (Notify && on_change) on_change(selected);
    }

    std::expected<void, error> draw_items() {
      const auto content_pos = _content_pos();
      const auto content_size = content_area;
      float y = content_pos.y - scroll_offset.y;
      for (size_t i = 0; i < items.size(); ++i) {
        const float h = _item_height(i);
        if (y + h >= content_pos.y && y <= content_pos.y + content_size.y) {
          const float2 row_pos{content_pos.x, y};
          const float2 row_size{content_size.x, h};
          if (i == selected) {
            brush::color(selection_color);
            if (auto res = fill_rectangle(row_pos, row_size); !res) return res.error().relay();
          } else if (i == pressed) {
            brush::color(pressed_color_item);
            if (auto res = fill_rectangle(row_pos, row_size); !res) return res.error().relay();
          }
          brush::color(text_color);
          const float2 text_pos{content_pos.x + item_padding.x - scroll_offset.x, y + item_padding.y};
          if (auto res = items[i].draw(text_pos); !res) return res.error().relay();
        }
        y += h;
        if (y > content_pos.y + content_size.y) break;
      }
      return {};
    }

    virtual std::expected<void, error> draw_hovered_overlay() override {
      if (hovered_item == npos || hovered_overlay_color.a <= 0.0f) return {};
      if (hovered_item >= items.size()) return {};
      const auto content_pos = _content_pos();
      const auto content_size = content_area;
      const float y = content_pos.y + _item_top(hovered_item) - scroll_offset.y;
      const float h = _item_height(hovered_item);
      if (y + h < content_pos.y || y > content_pos.y + content_size.y) return {};
      brush::color(hovered_overlay_color);
      if (auto res = fill_rectangle({content_pos.x, y}, {content_size.x, h}); !res) return res.error().relay();
      return {};
    }

    virtual std::expected<void, error> redraw() override {
      if (geometry_dirty) {
        geometry_dirty = false;
        if (auto res = relocate(); !res) return res.error().relay();
      }
      if (!visible) return {};
      if (auto res = draw_frame_background(); !res) return res.error().relay();
      if (auto res = draw_items(); !res) return res.error().relay();
      if (auto res = draw_scrollbars(); !res) return res.error().relay();
      if (auto res = draw_frame_foreground(); !res) return res.error().relay();
      return {};
    }

    virtual bool button_event(yw::button_event e) override {
      if (!enabled || !visible || e.key != keys::lbutton) return scrollbar::slot::button_event(e);
      if (scrollbar::slot::button_event(e)) return true;
      const auto hit = item_at(float2(float(e.pos.x), float(e.pos.y)));
      if (e.down) {
        if (hit == npos) return false;
        pressed = hit;
        select(hit);
        make_dirty();
        return true;
      }
      if (pressed == npos) return false;
      pressed = npos;
      make_dirty();
      return true;
    }

    virtual bool click_event(yw::button_event e) override {
      if (!enabled || !visible || e.down || e.key != keys::lbutton) return scrollbar::slot::click_event(e);
      if (scrollbar::slot::click_event(e)) return true;
      pressed = npos;
      return true;
    }

    virtual bool drag_event(yw::drag_event e) override {
      if (scrollbar::slot::drag_event(e)) return true;
      return pressed != npos;
    }

    virtual std::expected<void, error> reset_state() override {
      if (auto res = scrollbar::slot::reset_state(); !res) return res.error().relay();
      if (pressed == npos) return {};
      pressed = npos;
      make_dirty();
      return {};
    }

    virtual void focus_event(bool Focused) override {
      if (!Focused && pressed != npos) {
        pressed = npos;
        make_dirty();
      }
      scrollbar::slot::focus_event(Focused);
    }

    virtual bool hover_event(yw::hover_event e) override {
      const auto next_hovered = e.leave() ? npos : item_at(float2(float(e.pos.x), float(e.pos.y)));
      if (hovered_item != next_hovered) {
        hovered_item = next_hovered;
        make_dirty();
      }
      return scrollbar::slot::hover_event(e);
    }

    virtual bool key_event(yw::key_event e) override {
      if (!enabled || !visible || items.empty()) return false;
      if (!e.down) {
        if (e.key == keys::up || e.key == keys::down || e.key == keys::page_up || e.key == keys::page_down ||
            e.key == keys::home || e.key == keys::end)
          return true;
        return scrollbar::slot::key_event(e);
      }
      const auto old = selected == npos ? 0 : selected;
      if (e.key == keys::up) {
        select(old == 0 ? 0 : old - 1);
        return true;
      }
      if (e.key == keys::down) {
        select(yw::min(old + 1, items.size() - 1));
        return true;
      }
      if (e.key == keys::home) {
        select(0);
        return true;
      }
      if (e.key == keys::end) {
        select(items.size() - 1);
        return true;
      }
      if (e.key == keys::page_up || e.key == keys::page_down) {
        const float amount = page_amount<true>();
        float y = _item_top(old) + (e.key == keys::page_up ? -amount : amount);
        y = yw::clamp(y, 0.0f, yw::max(0.0f, _content_height() - 1.0f));
        float top = 0.0f;
        for (size_t i = 0; i < items.size(); ++i) {
          const float bottom = top + _item_height(i);
          if (y < bottom) {
            select(i);
            return true;
          }
          top = bottom;
        }
        select(items.size() - 1);
        return true;
      }
      return scrollbar::slot::key_event(e);
    }

    virtual std::expected<void, error> apply_color_theme(const yw::ui::color_theme& Theme, bool Recursive) override {
      if (auto res = scrollbar::slot::apply_color_theme(Theme, Recursive); !res) return res.error().relay();
      background_color = Theme.surface;
      hovered_overlay_color = color(Theme.accent, default_overlay_opacity.hover);
      text_color = Theme.text;
      selection_color = color(Theme.accent, default_overlay_opacity.selection);
      pressed_color_item = color(Theme.accent, default_overlay_opacity.pressed);
      make_dirty();
      return {};
    }
  };

  using scrollbar::operator bool;
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
    sp->policy = {ui::size_policy::fit, ui::size_policy::free};
    if (auto res = sp->apply_current_color_theme(false); !res) return res.error().relay();
    return l;
  }

  //-- getter --//

  size_t item_count() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->items.size();
  }

  size_t selected_index() const noexcept { ywlib_control_get(selected); }
  const auto& on_change() const noexcept { ywlib_control_get(on_change); }
  const auto& text_color() const noexcept { ywlib_control_get(text_color); }
  const auto& selection_color() const noexcept { ywlib_control_get(selection_color); }
  const auto& pressed_color_item() const noexcept { ywlib_control_get(pressed_color_item); }
  const auto& item_padding() const noexcept { ywlib_control_get(item_padding); }

  const auto& text(size_t Index) const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    if (Index >= sp->items.size()) error(errors::invalid_argument, format("invalid item index: ", Index)).go_off();
    return sp->items[Index];
  }

  const auto& string(size_t Index) const noexcept { return text(Index).string(); }

  //-- setter --//

  auto& add(this auto& self, yw::string<wchar_t> s) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    if (auto res = yw::text::create(std::move(s))) sp->items.push_back(std::move(*res));
    else {
      res.error().go_off();
      return self;
    }
    if (sp->selected == npos) sp->selected = 0;
    sp->make_messy();
    return self;
  }

  auto& add(this auto& self, yw::text t) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->items.push_back(std::move(t));
    if (sp->selected == npos) sp->selected = 0;
    sp->make_messy();
    return self;
  }

  auto& insert(this auto& self, size_t Index, yw::string<wchar_t> s) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    if (Index > sp->items.size()) {
      error(errors::invalid_argument, format("invalid item index: ", Index)).go_off();
      return self;
    }
    if (auto res = yw::text::create(std::move(s))) sp->items.insert(sp->items.begin() + Index, std::move(*res));
    else {
      res.error().go_off();
      return self;
    }
    if (sp->selected == npos) sp->selected = 0;
    else if (Index <= sp->selected) ++sp->selected;
    if (sp->pressed != npos && Index <= sp->pressed) ++sp->pressed;
    if (sp->hovered_item != npos && Index <= sp->hovered_item) ++sp->hovered_item;
    sp->make_messy();
    return self;
  }

  auto& erase(this auto& self, size_t Index) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    if (Index >= sp->items.size()) {
      error(errors::invalid_argument, format("invalid item index: ", Index)).go_off();
      return self;
    }
    sp->items.erase(sp->items.begin() + Index);
    if (sp->items.empty()) {
      sp->selected = sp->pressed = sp->hovered_item = npos;
    } else {
      if (sp->selected == Index) sp->selected = yw::min(Index, sp->items.size() - 1);
      else if (Index < sp->selected) --sp->selected;
      if (sp->pressed == Index) sp->pressed = npos;
      else if (Index < sp->pressed) --sp->pressed;
      if (sp->hovered_item == Index) sp->hovered_item = npos;
      else if (Index < sp->hovered_item) --sp->hovered_item;
    }
    sp->make_messy();
    return self;
  }

  auto& clear(this auto& self) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->items.clear();
    sp->selected = sp->pressed = sp->hovered_item = npos;
    sp->scroll_to({});
    sp->make_messy();
    return self;
  }

  auto& selected_index(this auto& self, size_t Index) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    if (Index >= sp->items.size()) {
      error(errors::invalid_argument, format("invalid item index: ", Index)).go_off();
      return self;
    }
    sp->select(Index);
    return self;
  }

  auto& on_change(this auto& self, function<void, size_t> f) noexcept { ywlib_control_set(on_change, std::move(f), none); }
  auto& text_color(this auto& self, const color& c) noexcept { ywlib_control_set(text_color, c, dirty); }
  auto& selection_color(this auto& self, const color& c) noexcept { ywlib_control_set(selection_color, c, dirty); }
  auto& pressed_color_item(this auto& self, const color& c) noexcept { ywlib_control_set(pressed_color_item, c, dirty); }
  auto& item_padding(this auto& self, float4 f) noexcept { ywlib_control_set(item_padding, f, messy); }

  auto& text(this auto& self, size_t Index, yw::text t) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    if (Index >= sp->items.size()) {
      error(errors::invalid_argument, format("invalid item index: ", Index)).go_off();
      return self;
    }
    sp->items[Index] = std::move(t);
    sp->make_messy();
    return self;
  }

  auto& string(this auto& self, size_t Index, yw::string<wchar_t> s) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    if (Index >= sp->items.size()) {
      error(errors::invalid_argument, format("invalid item index: ", Index)).go_off();
      return self;
    }
    if (auto res = sp->items[Index].string(std::move(s)); !res) res.error().go_off();
    sp->make_messy();
    return self;
  }

  auto& font(this auto& self, font_config f) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    for (auto& item : sp->items)
      if (auto res = item.font(f); !res) res.error().go_off();
    sp->make_messy();
    return self;
  }
};
} // namespace yw::ui
