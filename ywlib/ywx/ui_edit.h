#pragma once
#include <optional>
#include <ywx/ui_selectable_label.h>

namespace yw::ui {

class edit : public selectable_label {
public:
  struct slot : selectable_label::slot {
    yw::text placeholder_string = yw::text(L"");
    color placeholder_color = color(colors::black, 0.5f);
    std::optional<uint32_t> max_length = std::nullopt;
    function<bool, wchar_t> filter{};
    function<void, string_view<wchar_t>> on_change{};
    function<void, yw::key_event> on_enter{};
    bool readonly = false;

    float2 _placeholder_offset() const noexcept {
      constexpr float c[]{0.5f, 0.0f, 1.0f};
      const float2 cc{c[unsigned(text_align) % 3], c[unsigned(text_align) / 3 % 3]};
      return (size - placeholder_string.size() - padding.xy() - padding.zw()) * cc + padding.xy();
    }

    virtual std::expected<float2, error> get_necessary_size() const override {
      const auto inner =
        vapply_r<float2>(yw::max, placeholder_string.size()) + padding.xy() + padding.zw();
      return calc_necessary_size_by_policy(inner);
    }

    virtual std::expected<void, error> draw_focusring(const color& Color, float Thickness, float2 Offset) override {
      if (auto res = control::slot::draw_focusring(Color, Thickness, Offset); !res) return res.error().relay();
      return {};
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
      if (!focused && text.string().empty()) {
        brush::color(placeholder_color);
        if (auto res = placeholder_string.draw(pos + _placeholder_offset()); !res) return res.error().relay();
      } else {
        brush::color(text_color);
        if (auto res = text.draw(pos + calc_text_offset() - scroll_offset); !res) return res.error().relay();
      }
      if (focused)
        if (auto res = _draw_caret(); !res) return res.error().relay();
      if (auto res = draw_frame_foreground(); !res) return res.error().relay();
      return {};
    }

    std::expected<void, error> _replace(uint2 Range, string_view<wchar_t> Value, bool PlaceCaretAfter) {
      const auto erase_length = Range.y - Range.x;
      const auto insert_length = static_cast<uint32_t>(Value.size());
      if (erase_length == 0 && insert_length == 0) return {};
      yw::string<wchar_t> new_string;
      new_string.reserve(text.string().size() - erase_length + insert_length);
      new_string.append(string_view<wchar_t>(text.string()).substr(0, Range.x));
      new_string.append(Value);
      new_string.append(string_view<wchar_t>(text.string()).substr(Range.y));
      if (auto res = text.string(std::move(new_string)); !res) return res.error().relay();
      const auto next_pos = static_cast<int32_t>(Range.x + insert_length);
      caret = anchor = PlaceCaretAfter ? next_pos : static_cast<int32_t>(Range.x);
      make_messy();
      if (!readonly && on_change) on_change(text.string());
      return {};
    }

    struct replace_command {
      slotid target{};
      uint2 range{};
      ministr<wchar_t> value{};
      void operator()() const {
        if (const auto sp = get_slot<edit>(target); !sp) return;
        else if (auto res = sp->_replace(range, value, true); !res) res.error().go_off();
      }
    };

    std::expected<void, error> replace(uint2 Range, string_view<wchar_t> Value, bool PlaceCaretAfter) {
      const auto cmds = commands();
      if (!cmds) return std::unexpected(error(errors::invalid_argument));
      Value = _limited_value(Range, Value);
      if (Range.x == Range.y && Value.empty()) return {};
      auto erased = ministr<wchar_t>(string_view<wchar_t>(text.string()).substr(Range.x, Range.y - Range.x));
      if (auto res = _replace(Range, Value, PlaceCaretAfter)) {
        cmds->push(
          replace_command{.target = id, .range = Range, .value = ministr<wchar_t>(Value)},
          replace_command{.target = id, .range = uint2(Range.x, Range.x + Value.size()), .value = erased});
        return {};
      } else return res.error().relay();
    }

    std::expected<void, error> erase(uint2 Range) {
      const auto cmds = commands();
      if (!cmds) return std::unexpected(error(errors::invalid_argument));
      auto erased = ministr<wchar_t>(string_view<wchar_t>(text.string()).substr(Range.x, Range.y - Range.x));
      if (auto res = _replace(Range, {}, true)) {
        cmds->push(
          replace_command{.target = id, .range = Range, .value = {}},
          replace_command{.target = id, .range = uint2(Range.x, Range.x), .value = erased});
        return {};
      } else return res.error().relay();
    }

    yw::string<wchar_t> _limited_input(uint2 Range, string_view<wchar_t> Value) const {
      yw::string<wchar_t> result;
      result.reserve(Value.size());
      for (const auto c : Value)
        if (!filter || filter(c)) result.push_back(c);
      const auto limited = _limited_value(Range, result);
      if (limited.size() == result.size()) return result;
      return yw::string<wchar_t>(limited);
    }

    virtual bool char_event(wchar_t c) override {
      if (!enabled || readonly || c < 0x20 || c == 0x7f || c == L'\r' || c == L'\n' || c == L'\t') return false;
      if (filter && !filter(c)) return true;
      if (auto res = replace(_selected_range(), string_view<wchar_t>(&c, 1), true); !res) res.error().go_off();
      return true;
    }

    virtual bool key_event(yw::key_event e) override {
      if (!enabled) return false;
      if (_passes_key_to_window(e)) return false;
      if (!e.down) return true;
      if (readonly) return selectable_label::slot::key_event(e);
      if (e.key == keys::enter) {
        if (on_enter) on_enter(e);
        return true;
      }
      if (e.mods.ctrl) {
        if (e.key == keys::backspace) {
          if (auto res = _erase_word_backward(); !res) res.error().go_off();
          return true;
        } else if (e.key == keys::delete_) {
          if (auto res = _erase_word_forward(); !res) res.error().go_off();
          return true;
        } else if (e.key == keys::x) {
          if (caret != anchor) {
            clipboard.text(_selected_string());
            if (auto res = erase(_selected_range()); !res) res.error().go_off();
          }
          return true;
        } else if (e.key == keys::v) {
          if (auto txt = clipboard.text()) {
            const auto value = _limited_input(_selected_range(), *txt);
            if (auto res = replace(_selected_range(), value, true); !res) res.error().go_off();
          }
          return true;
        } else if (e.key == keys::z) {
          if (const auto cmds = commands()) cmds->undo();
          return true;
        } else if (e.key == keys::y) {
          if (const auto cmds = commands()) cmds->redo();
          return true;
        }
      }
      switch (e.key.code) {
      case VK_BACK:
        if (auto res = _erase_backward(); !res) res.error().go_off();
        return true;
      case VK_DELETE:
        if (auto res = _erase_forward(); !res) res.error().go_off();
        return true;
      }
      if (selectable_label::slot::key_event(e)) return true;
      return true;
    }

  private:
    string_view<wchar_t> _limited_value(uint2 Range, string_view<wchar_t> Value) const noexcept {
      if (!max_length) return Value;
      const auto erase_length = Range.y - Range.x;
      const auto base_length = static_cast<uint32_t>(text.string().size()) - erase_length;
      if (base_length >= *max_length) return {};
      return Value.substr(0, *max_length - base_length);
    }

    std::expected<bool, error> _erase_selected() {
      if (caret == anchor) return false;
      if (auto res = erase(_selected_range()); !res) return res.error().relay();
      return true;
    }

    std::expected<void, error> _erase_backward() {
      if (auto res = _erase_selected(); !res) res.error().go_off();
      else if (*res) return {};
      if (caret == 0) return {};
      if (auto res = erase({caret - 1, caret}); !res) res.error().relay();
      return {};
    }

    std::expected<void, error> _erase_forward() {
      if (auto res = _erase_selected(); !res) res.error().go_off();
      else if (*res) return {};
      if (caret >= static_cast<uint32_t>(text.string().size())) return {};
      if (auto res = erase({caret, caret + 1}); !res) res.error().relay();
      return {};
    }

    std::expected<void, error> _erase_word_backward() {
      if (auto res = _erase_selected(); !res) res.error().go_off();
      else if (*res) return {};
      if (const auto next = _find_word_boundary(caret, -1); next == caret) return {};
      else if (auto res = erase({next, caret})) return {};
      else return res.error().relay();
    }

    std::expected<void, error> _erase_word_forward() {
      if (auto res = _erase_selected(); !res) res.error().go_off();
      else if (*res) return {};
      if (const auto next = _find_word_boundary(caret, +1); next == caret) return {};
      else if (auto res = erase({caret, next})) return {};
      else return res.error().relay();
    }
  };

  edit() noexcept = default;

  edit(derived_from<interface> auto& Parent, strict<bool> AutoColor = true, const source_line& sl = here()) {
    if (auto res = create(Parent, AutoColor)) *this = std::move(*res);
    else res.error().add_footprint().go_off(sl);
  }

  static std::expected<edit, error> create(derived_from<interface> auto& Parent, strict<bool> AutoColor = true) {
    edit e;
    const auto temp_id = make_slot<edit>();
    const auto sp = get_slot<edit>(temp_id);
    if (!sp) return std::unexpected(error(errors::slot_creation_failed));
    const auto psp = get_slot<control>(Parent.id());
    if (!psp) return std::unexpected(error(errors::invalid_slotid));
    if (auto res = psp->attach(temp_id); !res) {
      slot::slots.erase(temp_id);
      return res.error().relay();
    }
    e._id = temp_id;
    sp->id = temp_id;
    sp->window_id = psp->get_window_id();
    sp->policy = {ui::size_policy::free, ui::size_policy::fit};
    sp->text_align = alignment::left;
    if (AutoColor) {
      sp->colors = color_pair(none());
      sp->text_color = sp->colors.border;
    }
    return e;
  }

  //-- getter --//

  const auto& placeholder_string() const noexcept { ywlib_control_get(placeholder_string); }
  const auto& placeholder_color() const noexcept { ywlib_control_get(placeholder_color); }
  const auto& max_length() const noexcept { ywlib_control_get(max_length); }
  const auto& filter() const noexcept { ywlib_control_get(filter); }
  const auto& on_change() const noexcept { ywlib_control_get(on_change); }
  const auto& on_enter() const noexcept { ywlib_control_get(on_enter); }
  bool readonly() const noexcept { ywlib_control_get(readonly); }

  const auto& string() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->text.string();
  }

  //-- setter --//

  auto& placeholder_string(this auto& self, yw::string<wchar_t> s) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    if (auto res = sp->placeholder_string.string(std::move(s)); !res) res.error().go_off();
    sp->make_messy();
    return self;
  }

  auto& placeholder_color(this auto& self, const color& c) noexcept { ywlib_control_set(placeholder_color, c, dirty); }

  auto& max_length(this auto& self, std::optional<uint32_t> v) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->max_length = v;
    if (v && sp->text.string().size() > *v) {
      if (auto res = sp->text.string(yw::string<wchar_t>(string_view<wchar_t>(sp->text.string()).substr(0, *v))); !res)
        res.error().go_off();
      sp->caret = yw::clamp(sp->caret, 0, sp->text.string().size());
      sp->anchor = yw::clamp(sp->anchor, 0, sp->text.string().size());
      sp->make_messy();
    }
    return self;
  }

  auto& filter(this auto& self, function<bool, wchar_t> f) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->filter = std::move(f);
    return self;
  }

  auto& on_change(this auto& self, function<void, string_view<wchar_t>> f) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->on_change = std::move(f);
    return self;
  }

  auto& on_enter(this auto& self, function<void, yw::key_event> f) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->on_enter = std::move(f);
    return self;
  }

  auto& string(this auto& self, yw::string<wchar_t> s) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    s = sp->_limited_input({0, static_cast<uint32_t>(sp->text.string().size())}, s);
    const bool changed = string_view<wchar_t>(sp->text.string()) != string_view<wchar_t>(s);
    if (auto res = sp->text.string(std::move(s)); !res) res.error().go_off();
    sp->caret = yw::clamp(sp->caret, 0, sp->text.string().size());
    sp->anchor = yw::clamp(sp->anchor, 0, sp->text.string().size());
    sp->make_messy();
    if (changed && !sp->readonly && sp->on_change) sp->on_change(sp->text.string());
    return self;
  }

  auto& readonly(this auto& self, bool v) noexcept { ywlib_control_set(readonly, v, dirty); }
};
} // namespace yw::ui
