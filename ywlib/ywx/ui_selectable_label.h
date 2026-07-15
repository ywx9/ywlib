#pragma once
#include <ywx/ui_label.h>

namespace yw::ui {

class selectable_label : public label {
  enum class char_group : uint8_t {
    none = 0,
    alnum = 1 << 0,
    ascii = 1 << 1,
    katakana = 1 << 2,
    hiragana = 1 << 3,
    other = 1 << 4,
  };
  static flags<char_group> char_groups(wchar_t c) noexcept {
    if (is_alnum(c)) return char_group::alnum;
    else if (is_ascii(c)) return char_group::ascii;
    else if (c == L'ー') return flags<char_group>(char_group::katakana) | char_group::hiragana;
    else if (0x30A0 <= c && c <= 0x30FF) return char_group::katakana;
    else if (0x3040 <= c && c <= 0x309F) return char_group::hiragana;
    else return char_group::other;
  }
  static bool is_whitespace(wchar_t c) noexcept { return c == L' ' || c == L'\t' || c == L'\r' || c == L'\n'; }

public:
  struct slot : label::slot {

    color selection_overlay_color = color(colors::dodgerblue, 0.35f);
    color caret_color = colors::black;
    float caret_thickness = 1.0f;
    uint32_t caret = 0;
    uint32_t anchor = 0;
    float2 scroll_offset{};
    float2 pointer_pos{};
    bool selecting = false;
    // bool caret_visible = false;

    virtual bool focusable() const noexcept override { return enabled && visible; }

    static bool _passes_key_to_window(yw::key_event e) noexcept {
      return e.mods.alt || e.key == keys::alt || (VK_F1 <= e.key.code && e.key.code <= VK_F24);
    }

    uint2 _selected_range() const noexcept { return {yw::min(anchor, caret), yw::max(anchor, caret)}; }

    string_view<wchar_t> _selected_string() const noexcept {
      const auto [s, e] = _selected_range();
      return string_view<wchar_t>(text.string()).substr(s, e - s);
    }

    uint32_t _char_index_from_point(float2 pt) const noexcept {
      const auto local = pt - (pos + calc_text_offset() - scroll_offset);
      const auto len = text.string().size();
      if (len <= 0) return 0;
      if (auto hit = text.hittest(local)) return hit->index;
      if (local.x < 0.0f) return 0;
      return static_cast<uint32_t>(len - 1);
    }

    void _select_group_at_point(float2 pt) {
      const auto len = text.string().size();
      if (len == 0) return;
      const auto center = _char_index_from_point(pt);
      const auto sv = string_view<wchar_t>(text.string());
      const auto primary = char_groups(sv[center]);
      uint32_t left = center, right = center + 1;
      while (left > 0 && char_groups(sv[left - 1]).contains(primary)) --left;
      while (right < len && char_groups(sv[right]).contains(primary)) ++right;
      anchor = left, caret = right;
      make_dirty();
    }

    uint32_t _caret_from_point(float2 pt) const noexcept {
      const auto local = pt - (pos + calc_text_offset() - scroll_offset);
      if (text.string().empty()) return 0;
      if (auto hit = text.hittest(local)) return hit->index + uint32_t(hit->trailing);
      return static_cast<uint32_t>(text.string().size());
    }

    void _set_caret(uint32_t pos, bool keep_anchor) {
      caret = pos;
      if (!keep_anchor) anchor = caret;
      make_dirty();
    }

    void _move_horizontal(int32_t dir, bool keep_anchor) {
      if (dir >= 0) {
        const auto len = static_cast<uint32_t>(text.string().size());
        _set_caret(caret < len ? caret + 1 : len, keep_anchor);
      } else _set_caret(caret > 0 ? caret - 1 : 0, keep_anchor);
    }

    uint32_t _find_word_boundary(uint32_t pos, int32_t dir) const noexcept {
      const auto sv = string_view<wchar_t>(text.string());
      const auto len = sv.size();
      if (dir < 0) {
        while (pos > 0 && is_whitespace(sv[pos - 1])) --pos;
        if (pos == 0) return 0;
        const auto primary = char_groups(sv[pos - 1]);
        while (pos > 0 && !is_whitespace(sv[pos - 1]) && char_groups(sv[pos - 1]).contains(primary)) --pos;
        return pos;
      }
      while (pos < len && is_whitespace(sv[pos])) ++pos;
      if (pos >= len) return len;
      const auto primary = char_groups(sv[pos]);
      while (pos < len && !is_whitespace(sv[pos]) && char_groups(sv[pos]).contains(primary)) ++pos;
      return pos;
    }

    void _move_word(int32_t dir, bool keep_anchor) { _set_caret(_find_word_boundary(caret, dir), keep_anchor); }

    void _select_all() {
      anchor = 0, caret = static_cast<uint32_t>(text.string().size());
      make_dirty();
    }

    std::expected<void, error> _update_scroll_offset() {
      const auto base = pos + calc_text_offset();
      const auto visible_min = pos + padding.xy();
      const auto visible_max = pos + size - padding.zw();
      float2 caret_pos{};
      float caret_height = text.size().y;
      if (caret > 0 && !text.string().empty()) {
        const auto index = static_cast<uint32_t>(yw::min(caret - 1, text.string().size() - 1));
        if (auto hit = text.hittest(uint1{index})) {
          caret_pos = hit->pos + float2(hit->size.x, 0.0f);
          caret_height = hit->size.y;
        } else return hit.error().relay();
      }
      const auto p = base + caret_pos - scroll_offset;
      if (p.x < visible_min.x) scroll_offset.x += p.x - visible_min.x;
      if (p.x > visible_max.x) scroll_offset.x += p.x - visible_max.x;
      if (p.y < visible_min.y) scroll_offset.y += p.y - visible_min.y;
      if (p.y + caret_height > visible_max.y) scroll_offset.y += p.y + caret_height - visible_max.y;
      return {};
    }

    std::expected<float3, error> _caret_pos() {
      if (auto res = _update_scroll_offset(); !res) return res.error().relay();
      const auto origin = pos + calc_text_offset() - scroll_offset;
      if (caret <= 0 || text.string().empty()) return float3(origin.x, origin.y, text.size().y);
      const auto index = static_cast<uint32_t>(yw::min(caret - 1, text.string().size() - 1));
      auto ht = text.hittest(uint1{index});
      if (!ht) return ht.error().relay();
      const auto p = origin + ht->pos + float2(ht->size.x, 0.0f);
      return float3(p.x, p.y, ht->size.y);
    }

    virtual std::expected<std::optional<float3>, error> get_caret_pos() override {
      if (!enabled || !visible) return std::nullopt;
      if (auto res = _caret_pos()) return std::optional<float3>(*res);
      else return res.error().relay();
    }

    virtual std::expected<void, error> redraw() override {
      if (geometry_dirty) {
        geometry_dirty = false;
        if (auto res = relocate(); !res) return res.error().relay();
      }
      if (!visible) return {};
      const bool focused = control::slot::focused();
      if (auto res = draw_frame_background(); !res) return res.error().relay();
      if (auto res = _update_scroll_offset(); !res) return res.error().relay();
      if (auto res = _draw_selection(); !res) return res.error().relay();
      brush::color(text_color);
      if (auto res = text.draw(pos + calc_text_offset() - scroll_offset); !res) return res.error().relay();
      if (focused) if (auto res = _draw_caret(); !res) return res.error().relay();
      if (auto res = draw_frame_foreground(); !res) return res.error().relay();
      return {};
    }

    std::expected<void, error> _draw_selection() {
      if (caret == anchor) return {};
      if (auto res = text.hittest_range(_selected_range(), pos + calc_text_offset() - scroll_offset); res) {
        brush::color(selection_overlay_color);
        for (const auto& r : *res)
          if (auto res = fill_rectangle(r.pos, r.size); !res) return res.error().relay();
      } else return res.error().relay();
      return {};
    }

    std::expected<void, error> _draw_caret() {
      if (caret_color.a <= 0.0f || caret_thickness <= 0.0f) return {};
      auto cp = _caret_pos();
      if (!cp) return cp.error().relay();
      brush::color(caret_color);
      const auto p = cp->xy();
      if (auto res = draw_line(p, p + float2(0.0f, cp->z), caret_thickness); !res) return res.error().relay();
      return {};
    }

    virtual bool button_event(yw::button_event e) override {
      if (!enabled || e.key != keys::lbutton) return false;
      pointer_pos = float2(float(e.pos.x), float(e.pos.y));
      if (e.down) {
        selecting = true;
        const auto next = _caret_from_point(pointer_pos);
        if (e.mods.shift) caret = next;
        else anchor = caret = next;
        make_dirty();
      } else selecting = false;
      return true;
    }

    virtual bool click_event(yw::button_event e) override {
      if (!enabled || e.down || e.key != keys::lbutton) return false;
      pointer_pos = float2(float(e.pos.x), float(e.pos.y));
      selecting = false;
      return true;
    }

    virtual bool double_click_event(yw::button_event e) override {
      if (!enabled || e.key != keys::lbutton) return false;
      pointer_pos = float2(float(e.pos.x), float(e.pos.y));
      selecting = false;
      _select_group_at_point(pointer_pos);
      return true;
    }

    virtual bool drag_event(yw::drag_event e) override {
      if (!enabled || !selecting || e.key != keys::lbutton) return false;
      pointer_pos += float2(float(e.delta.x), float(e.delta.y));
      caret = _caret_from_point(pointer_pos);
      make_dirty();
      return true;
    }

    virtual std::expected<void, error> draw_focusring(const color&, float, float2) override { return {};}

    virtual void focus_event(bool Focused) override {
      selecting = false;
      label::slot::focus_event(Focused);
      make_dirty();
    }

    virtual std::expected<void, error> reset_state() override {
      if (!selecting) return {};
      selecting = false;
      make_dirty();
      return {};
    }

    virtual bool key_event(yw::key_event e) override {
      if (!enabled) return false;
      if (_passes_key_to_window(e)) return false;
      if (!e.down) return true;
      if (e.mods.ctrl) {
        if (e.key == keys::a) {
          _select_all();
          return true;
        }
        if (e.key == keys::c || e.key == keys::x) {
          if (caret != anchor) clipboard.text(_selected_string());
          return true;
        }
      }
      switch (e.key.code) {
      case VK_LEFT:
        if (e.mods.ctrl) _set_caret(_find_word_boundary(caret, -1), e.mods.shift);
        else _move_horizontal(-1, e.mods.shift);
        return true;
      case VK_RIGHT:
        if (e.mods.ctrl) _set_caret(_find_word_boundary(caret, +1), e.mods.shift);
        else _move_horizontal(+1, e.mods.shift);
        return true;
      case VK_HOME: _set_caret(0, e.mods.shift); return true;
      case VK_END: _set_caret(static_cast<uint32_t>(text.string().size()), e.mods.shift); return true;
      }
      return true;
    }
  };

  selectable_label() noexcept = default;

  selectable_label(derived_from<interface> auto& Parent, strict<bool> AutoColor = true, const source_line& sl = here()) {
    if (auto res = create(Parent, AutoColor)) *this = std::move(*res);
    else res.error().add_footprint().go_off(sl);
  }

  static std::expected<selectable_label, error> create(derived_from<interface> auto& Parent, strict<bool> AutoColor = true) {
    selectable_label l;
    const auto temp_id = make_slot<selectable_label>();
    const auto sp = get_slot<selectable_label>(temp_id);
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
    sp->policy = {ui::size_policy::fit, ui::size_policy::fit};
    if (AutoColor) {
      sp->colors = color_pair(none());
      sp->text_color = std::exchange(sp->colors.border, colors::transparent);
    }
    return l;
  }

  //-- getter --//

  /// gets/sets selection overlay color
  const auto& selection_overlay_color() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->selection_overlay_color;
  }

  /// gets/sets caret color
  const auto& caret_color() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->caret_color;
  }

  /// gets/sets caret thickness in pixels
  auto caret_thickness() const noexcept {
    if (const auto sp = get_slot(this); !sp) {
      error(errors::invalid_slotid).fizzle_out();
      return 0.0f;
    } else return sp->caret_thickness;
  }

  /// gets/sets caret position in text
  auto caret() const noexcept {
    if (const auto sp = get_slot(this); !sp) {
      error(errors::invalid_slotid).fizzle_out();
      return 0u;
    } else return sp->caret;
  }

  /// gets/sets anchor position in text
  auto anchor() const noexcept {
    if (const auto sp = get_slot(this); !sp) {
      error(errors::invalid_slotid).fizzle_out();
      return 0u;
    } else return sp->anchor;
  }

  /// gets the selected string in text
  auto selected_string() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return string_view<wchar_t>();
    } else if (sp->anchor == sp->caret) return string_view<wchar_t>();
    const auto s = yw::min(sp->anchor, sp->caret);
    const auto e = yw::max(sp->anchor, sp->caret);
    return string_view<wchar_t>(sp->text.string()).substr(s, e - s);
  }

  //-- setter --//

  /// sets/gets selection overlay color
  auto& selection_overlay_color(this auto& self, const color& c) noexcept {
    if (const auto sp = get_slot(&self)) {
      sp->selection_overlay_color = c;
      sp->make_dirty();
    } else error(errors::invalid_slotid).fizzle_out();
    return self;
  }

  /// sets/gets caret color
  auto& caret_color(this auto& self, const color& c) noexcept {
    if (const auto sp = get_slot(&self)) {
      sp->caret_color = c;
      sp->make_dirty();
    } else error(errors::invalid_slotid).fizzle_out();
    return self;
  }

  /// sets/gets caret thickness in pixels
  auto& caret_thickness(this auto& self, float v) noexcept {
    if (const auto sp = get_slot(&self)) {
      sp->caret_thickness = v;
      sp->make_dirty();
    } else error(errors::invalid_slotid).fizzle_out();
    return self;
  }

  auto& caret(this auto& self, int1 Caret) noexcept {
    if (const auto sp = get_slot(&self)) {
      sp->caret = yw::clamp(Caret.x, 0, sp->text.string().size());
      sp->make_dirty();
    } else error(errors::invalid_slotid).fizzle_out();
    return self;
  }

  auto& anchor(this auto& self, int1 Anchor) noexcept {
    if (const auto sp = get_slot(&self)) {
      sp->anchor = yw::clamp(Anchor.x, 0, sp->text.string().size());
      sp->make_dirty();
    } else error(errors::invalid_slotid).fizzle_out();
    return self;
  }

  auto& text(this auto& self, yw::text Text) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->text = std::move(Text);
    sp->caret = yw::clamp(sp->caret, 0, sp->text.string().size());
    sp->anchor = yw::clamp(sp->anchor, 0, sp->text.string().size());
    sp->make_messy();
    return self;
  }

  auto& string(this auto& self, yw::string<wchar_t> s) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    if (auto res = sp->text.string(std::move(s)); !res) res.error().go_off();
    sp->caret = yw::clamp(sp->caret, 0, sp->text.string().size());
    sp->anchor = yw::clamp(sp->anchor, 0, sp->text.string().size());
    sp->make_messy();
    return self;
  }
};
}
