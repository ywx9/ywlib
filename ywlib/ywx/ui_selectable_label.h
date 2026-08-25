#pragma once
#include <ywx/clipboard.h>
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
    optional<color> selection_overlay_color;
    optional<color> caret_color;
    float caret_thickness = 1.0f;
    uint32_t caret = 0;
    uint32_t anchor = 0;
    float2 scroll_offset{};
    float2 pointer_pos{};
    bool selecting = false;

    //-- override functions --//

    virtual bool is_focusable() const override { return enabled && visible; }
    virtual bool is_interactive() const override { return true; }

    virtual color get_selection_overlay_color(const interface::slot* Window) const noexcept {
      if (selection_overlay_color) return *selection_overlay_color;
      if (auto theme = get_color_theme(Window)) return color((*theme)->accent, default_overlay_opacity.selection);
      return colors::transparent;
    }

    virtual color get_caret_color(const interface::slot* Window) const noexcept {
      if (caret_color) return *caret_color;
      if (auto theme = get_color_theme(Window)) return (*theme)->text;
      return colors::transparent;
    }

    virtual std::expected<void, error> draw_forecontent(interface::slot* Window) override {
      if (auto res = update_scroll_offset(); !res) return res.error().relay();
      const auto origin = text_origin();
      if (auto res = draw_selection(Window, origin); !res) return res.error().relay();
      if (auto res = yw::draw_text(origin, text, get_text_color(Window)); !res) return res.error().relay();
      if (is_focused()) {
        if (auto res = draw_caret(Window); !res) return res.error().relay();
      }
      return {};
    }

    virtual std::optional<float3> get_caret_pos() const override {
      if (!enabled || !visible) return std::nullopt;
      if (auto res = caret_pos()) return std::optional<float3>(*res);
      else {
        res.error().fizzle_out();
        return std::nullopt;
      }
    }

    virtual void reset_state() override {
      if (!selecting) return;
      selecting = false;
      make_dirty();
    }

    virtual bool handle_button_event(yw::button_event e) override {
      if (!enabled || !visible || e.key != keys::lbutton) return false;
      pointer_pos = float2(float(e.pos.x), float(e.pos.y));
      if (e.down) {
        selecting = true;
        const auto next = caret_from_point(pointer_pos);
        if (e.mods.shift) caret = next;
        else anchor = caret = next;
        make_dirty();
      } else {
        selecting = false;
        make_dirty();
      }
      return true;
    }

    virtual bool handle_click_event(yw::button_event e) override {
      if (!enabled || !visible || e.down || e.key != keys::lbutton) return false;
      pointer_pos = float2(float(e.pos.x), float(e.pos.y));
      selecting = false;
      make_dirty();
      return true;
    }

    virtual bool handle_double_click_event(yw::button_event e) override {
      if (!enabled || !visible || e.key != keys::lbutton) return false;
      pointer_pos = float2(float(e.pos.x), float(e.pos.y));
      selecting = false;
      select_group_at_point(pointer_pos);
      return true;
    }

    virtual bool handle_drag_event(yw::drag_event e) override {
      if (!enabled || !visible || !selecting || e.key != keys::lbutton) return false;
      pointer_pos += float2(float(e.delta.x), float(e.delta.y));
      caret = caret_from_point(pointer_pos);
      make_dirty();
      return true;
    }

    virtual bool handle_focus_event(yw::focus_event e) override {
      selecting = false;
      make_dirty();
      return label::slot::handle_focus_event(e);
    }

    virtual bool handle_key_event(yw::key_event e) override {
      if (!enabled || !visible) return false;
      if (passes_key_to_window(e)) return false;
      if (!e.down) return true;
      if (e.mods.ctrl) {
        if (e.key == keys::a) {
          select_all();
          return true;
        }
        if (e.key == keys::c || e.key == keys::x) {
          if (caret != anchor) clipboard.text(selected_string());
          return true;
        }
      }
      switch (e.key.code) {
      case VK_LEFT:
        if (e.mods.ctrl) set_caret(find_word_boundary(caret, -1), e.mods.shift);
        else move_horizontal(-1, e.mods.shift);
        return true;
      case VK_RIGHT:
        if (e.mods.ctrl) set_caret(find_word_boundary(caret, +1), e.mods.shift);
        else move_horizontal(+1, e.mods.shift);
        return true;
      case VK_HOME: set_caret(0, e.mods.shift); return true;
      case VK_END: set_caret(static_cast<uint32_t>(text.string().size()), e.mods.shift); return true;
      }
      return false;
    }

    //-- shared functions --//

    static bool passes_key_to_window(yw::key_event e) noexcept {
      return e.mods.alt || e.key == keys::alt || (VK_F1 <= e.key.code && e.key.code <= VK_F24);
    }

    uint2 selected_range() const noexcept { return {yw::min(anchor, caret), yw::max(anchor, caret)}; }

    string_view<wchar_t> selected_string() const noexcept {
      const auto [s, e] = selected_range();
      return string_view<wchar_t>(text.string()).substr(s, e - s);
    }

    float2 text_area() const noexcept { return size - padding.xy() - padding.zw(); }

    float2 text_offset() const noexcept {
      constexpr float c[]{0.5f, 0.0f, 1.0f};
      const float2 cc{c[unsigned(text_align) % 3], c[unsigned(text_align) / 3 % 3]};
      return padding.xy() + (text_area() - text.size()) * cc;
    }

    float2 text_origin() const noexcept { return pos + text_offset() - scroll_offset; }

    uint32_t char_index_from_point(float2 pt) const noexcept {
      const auto local = pt - text_origin();
      const auto len = text.string().size();
      if (len <= 0) return 0;
      if (auto hit = text.hittest(local)) return hit->index;
      if (local.x < 0.0f) return 0;
      return static_cast<uint32_t>(len - 1);
    }

    uint32_t caret_from_point(float2 pt) const noexcept {
      const auto local = pt - text_origin();
      if (text.string().empty()) return 0;
      if (auto hit = text.hittest(local)) return hit->index + uint32_t(hit->trailing);
      return static_cast<uint32_t>(text.string().size());
    }

    void select_group_at_point(float2 pt) {
      const auto len = text.string().size();
      if (len == 0) return;
      const auto center = char_index_from_point(pt);
      const auto sv = string_view<wchar_t>(text.string());
      const auto primary = char_groups(sv[center]);
      uint32_t left = center;
      uint32_t right = center + 1;
      while (left > 0 && char_groups(sv[left - 1]).contains(primary)) --left;
      while (right < len && char_groups(sv[right]).contains(primary)) ++right;
      anchor = left;
      caret = right;
      make_dirty();
    }

    void set_caret(uint32_t Pos, bool KeepAnchor) {
      caret = Pos;
      if (!KeepAnchor) anchor = caret;
      make_dirty();
    }

    void move_horizontal(int32_t Dir, bool KeepAnchor) {
      if (Dir >= 0) {
        const auto len = static_cast<uint32_t>(text.string().size());
        set_caret(caret < len ? caret + 1 : len, KeepAnchor);
      } else set_caret(caret > 0 ? caret - 1 : 0, KeepAnchor);
    }

    uint32_t find_word_boundary(uint32_t Pos, int32_t Dir) const noexcept {
      const auto sv = string_view<wchar_t>(text.string());
      const auto len = sv.size();
      if (Dir < 0) {
        while (Pos > 0 && is_whitespace(sv[Pos - 1])) --Pos;
        if (Pos == 0) return 0;
        const auto primary = char_groups(sv[Pos - 1]);
        while (Pos > 0 && !is_whitespace(sv[Pos - 1]) && char_groups(sv[Pos - 1]).contains(primary)) --Pos;
        return Pos;
      }
      while (Pos < len && is_whitespace(sv[Pos])) ++Pos;
      if (Pos >= len) return static_cast<uint32_t>(len);
      const auto primary = char_groups(sv[Pos]);
      while (Pos < len && !is_whitespace(sv[Pos]) && char_groups(sv[Pos]).contains(primary)) ++Pos;
      return static_cast<uint32_t>(Pos);
    }

    void select_all() {
      anchor = 0;
      caret = static_cast<uint32_t>(text.string().size());
      make_dirty();
    }

    std::expected<void, error> update_scroll_offset() const {
      auto& self = const_cast<slot&>(*this);
      if (text.size().x <= text_area().x) self.scroll_offset.x = 0.0f;
      const auto base = pos + text_offset();
      const auto visible_min = pos + padding.xy();
      const auto visible_max = pos + size - padding.zw();
      float2 caret_local_pos{};
      float caret_height = text.size().y;
      if (caret > 0 && !text.string().empty()) {
        const auto index = static_cast<uint32_t>(yw::min(caret - 1, text.string().size() - 1));
        if (auto hit = text.hittest(uint1{index})) {
          caret_local_pos = hit->pos + float2(hit->size.x, 0.0f);
          caret_height = hit->size.y;
        } else return hit.error().relay();
      }
      const auto p = base + caret_local_pos - scroll_offset;
      if (p.x < visible_min.x) self.scroll_offset.x += p.x - visible_min.x;
      if (p.x > visible_max.x) self.scroll_offset.x += p.x - visible_max.x;
      if (p.y < visible_min.y) self.scroll_offset.y += p.y - visible_min.y;
      if (p.y + caret_height > visible_max.y) self.scroll_offset.y += p.y + caret_height - visible_max.y;
      return {};
    }

    std::expected<float3, error> caret_pos() const {
      if (auto res = update_scroll_offset(); !res) return res.error().relay();
      const auto origin = text_origin();
      if (caret <= 0 || text.string().empty()) return float3(origin.x, origin.y, text.size().y);
      const auto index = static_cast<uint32_t>(yw::min(caret - 1, text.string().size() - 1));
      auto ht = text.hittest(uint1{index});
      if (!ht) return ht.error().relay();
      const auto p = origin + ht->pos + float2(ht->size.x, 0.0f);
      return float3(p.x, p.y, ht->size.y);
    }

    std::expected<void, error> draw_selection(interface::slot* Window, float2 Origin) {
      if (caret == anchor) return {};
      if (auto res = text.hittest_range(selected_range(), Origin)) {
        brush::color(get_selection_overlay_color(Window));
        for (const auto& r : *res)
          if (auto fill = fill_rectangle(r.pos, r.size); !fill) return fill.error().relay();
        return {};
      } else return res.error().relay();
    }

    std::expected<void, error> draw_caret(interface::slot* Window) {
      const auto color = get_caret_color(Window);
      if (color.a <= 0.0f || caret_thickness <= 0.0f) return {};
      auto cp = caret_pos();
      if (!cp) return cp.error().relay();
      brush::color(color);
      const auto p = cp->xy();
      if (auto res = stroke_line(p, p + float2(0.0f, cp->z), caret_thickness); !res) return res.error().relay();
      return {};
    }
  };

  class proxy : public label::proxy {
    friend class selectable_label;
  protected:
    using label::proxy::proxy;
    selectable_label::slot* _get_slot() const noexcept { return static_cast<selectable_label::slot*>(_slot); }

  public:
    using label::proxy::string;
    using label::proxy::text;

    //-- getter --//

    color selection_overlay_color() const&& noexcept {
      return _get_slot()->get_selection_overlay_color(interface::slot::slots.get(_get_slot()->window_id));
    }
    color caret_color() const&& noexcept {
      return _get_slot()->get_caret_color(interface::slot::slots.get(_get_slot()->window_id));
    }
    auto caret_thickness() const&& noexcept { return _get_slot()->caret_thickness; }
    auto caret() const&& noexcept { return _get_slot()->caret; }
    auto anchor() const&& noexcept { return _get_slot()->anchor; }
    auto selected_string() const&& noexcept { return _get_slot()->selected_string(); }

    //-- setter --//

    auto selection_overlay_color(this auto&& Self, const color& Color) noexcept {
      Self._get_slot()->selection_overlay_color = Color;
      Self._dirty = true;
      return std::move(Self);
    }

    auto selection_overlay_color(this auto&& Self, none) noexcept {
      Self._get_slot()->selection_overlay_color = none();
      Self._dirty = true;
      return std::move(Self);
    }

    auto caret_color(this auto&& Self, const color& Color) noexcept {
      Self._get_slot()->caret_color = Color;
      Self._dirty = true;
      return std::move(Self);
    }

    auto caret_color(this auto&& Self, none) noexcept {
      Self._get_slot()->caret_color = none();
      Self._dirty = true;
      return std::move(Self);
    }

    auto caret_thickness(this auto&& Self, float Thickness) noexcept {
      Self._get_slot()->caret_thickness = Thickness;
      Self._dirty = true;
      return std::move(Self);
    }

    auto caret(this auto&& Self, int1 Caret) noexcept {
      Self._get_slot()->caret = yw::clamp(Caret.x, 0, Self._get_slot()->text.string().size());
      Self._dirty = true;
      return std::move(Self);
    }

    auto anchor(this auto&& Self, int1 Anchor) noexcept {
      Self._get_slot()->anchor = yw::clamp(Anchor.x, 0, Self._get_slot()->text.string().size());
      Self._dirty = true;
      return std::move(Self);
    }

    auto text(this auto&& Self, yw::text Text) noexcept {
      Self._get_slot()->text = std::move(Text);
      Self._get_slot()->caret = yw::clamp(Self._get_slot()->caret, 0, Self._get_slot()->text.string().size());
      Self._get_slot()->anchor = yw::clamp(Self._get_slot()->anchor, 0, Self._get_slot()->text.string().size());
      Self._messy = true;
      return std::move(Self);
    }

    auto string(this auto&& Self, yw::string<wchar_t> String) noexcept {
      Self._get_slot()->text.string(std::move(String));
      Self._get_slot()->caret = yw::clamp(Self._get_slot()->caret, 0, Self._get_slot()->text.string().size());
      Self._get_slot()->anchor = yw::clamp(Self._get_slot()->anchor, 0, Self._get_slot()->text.string().size());
      Self._messy = true;
      return std::move(Self);
    }
  };

  selectable_label() noexcept = default;

  selectable_label(derived_from<interface> auto& Parent, const source_line& sl = here()) {
    if (auto res = create(Parent)) *this = std::move(*res);
    else res.error().add_footprint().go_off(sl);
  }

  static std::expected<selectable_label, error> create() {
    selectable_label l;
    selectable_label::slot* sp;
    if (auto res = create_control<selectable_label>()) sp = *res;
    else return res.error().relay();
    l._id = sp->id;
    sp->policy = {ui::size_policy::fit, ui::size_policy::fit};
    return l;
  }

  static std::expected<selectable_label, error> create(derived_from<interface> auto& Parent) {
    auto res = create();
    if (!res) return res.error().relay();
    if (auto attached = res->attach(Parent); !attached) return attached.error().relay();
    return res;
  }

  yw_control_getter_setter(selection_overlay_color, color);
  auto selection_overlay_color(this auto& Self, none None) noexcept {
    return typename remove_cvref<decltype(Self)>::proxy(get_slot(&Self)).selection_overlay_color(None);
  }
  yw_control_getter_setter(caret_color, color);
  auto caret_color(this auto& Self, none None) noexcept {
    return typename remove_cvref<decltype(Self)>::proxy(get_slot(&Self)).caret_color(None);
  }
  yw_control_getter_setter(caret_thickness, float);
  yw_control_getter_setter(caret, int1);
  yw_control_getter_setter(anchor, int1);
  yw_control_getter(selected_string);
  yw_control_getter_setter(text, yw::text);
  yw_control_getter_setter(string, yw::string<wchar_t>);
};
} // namespace yw::ui
