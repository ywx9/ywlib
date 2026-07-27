#pragma once
#include <optional>

#include <ywx/ui_selectable_label.h>

namespace yw::ui {

class edit : public selectable_label {
public:
  struct slot : selectable_label::slot {
    yw::text placeholder_text = yw::text(L"");
    std::optional<uint32_t> max_length = std::nullopt;
    function<bool, wchar_t> filter{};
    function<void, string_view<wchar_t>> change_event{};
    function<void, yw::key_event> enter_event{};
    bool readonly = false;

    //-- override functions --//

    virtual std::expected<void, error> apply_color_theme(const yw::ui::color_theme& Theme, bool Recursive) override {
      if (auto res = selectable_label::slot::apply_color_theme(Theme, Recursive); !res) return res.error().relay();
      background_color = Theme.surface;
      border_color = Theme.outline;
      placeholder_text.color(color(Theme.text_muted, default_overlay_opacity.muted_text));
      make_dirty();
      return {};
    }

    virtual std::expected<void, error> draw_content() override {
      if (!is_focused() && text.string().empty()) {
        if (auto res = update_scroll_offset(); !res) return res.error().relay();
        if (auto res = label::slot::draw_text(
              placeholder_text, pos + padding.xy(), size - padding.xy() - padding.zw(), text_align);
          !res)
          return res.error().relay();
        return {};
      }
      return selectable_label::slot::draw_content();
    }

    virtual std::expected<float2, error> get_necessary_size() const override {
      const auto inner_height = text.size().y + padding.y + padding.w;
      return calc_necessary_size_by_policy({yw::max(placeholder_text.size().x, common_size_value * 3), inner_height});
    }

    virtual bool handle_char_event(wchar_t c) override {
      if (!enabled || readonly || c < 0x20 || c == 0x7f || c == L'\r' || c == L'\n' || c == L'\t') return false;
      if (filter && !filter(c)) return true;
      if (auto res = replace(selected_range(), string_view<wchar_t>(&c, 1), true); !res) res.error().go_off();
      return true;
    }

    virtual bool handle_key_event(yw::key_event e) override {
      if (!enabled || !visible) return false;
      if (passes_key_to_window(e)) return false;
      if (!e.down) return true;
      if (readonly) return selectable_label::slot::handle_key_event(e);
      if (e.key == keys::enter) {
        if (enter_event) enter_event(e);
        return true;
      }
      if (e.mods.ctrl) {
        if (e.key == keys::backspace) {
          if (auto res = erase_word_backward(); !res) res.error().go_off();
          return true;
        }
        if (e.key == keys::delete_) {
          if (auto res = erase_word_forward(); !res) res.error().go_off();
          return true;
        }
        if (e.key == keys::x) {
          if (caret != anchor) {
            clipboard.text(selected_string());
            if (auto res = erase(selected_range()); !res) res.error().go_off();
          }
          return true;
        }
        if (e.key == keys::v) {
          if (auto txt = clipboard.text()) {
            const auto value = limited_input(selected_range(), *txt);
            if (auto res = replace(selected_range(), value, true); !res) res.error().go_off();
          }
          return true;
        }
        if (e.key == keys::z) {
          if (const auto cmds = get_command_manager()) (*cmds)->undo();
          return true;
        }
        if (e.key == keys::y) {
          if (const auto cmds = get_command_manager()) (*cmds)->redo();
          return true;
        }
      }
      switch (e.key.code) {
      case VK_BACK:
        if (auto res = erase_backward(); !res) res.error().go_off();
        return true;
      case VK_DELETE:
        if (auto res = erase_forward(); !res) res.error().go_off();
        return true;
      }
      if (selectable_label::slot::handle_key_event(e)) return true;
      return true;
    }

    //-- shared functions --//

    string_view<wchar_t> limited_value(uint2 Range, string_view<wchar_t> Value) const noexcept {
      if (!max_length) return Value;
      const auto erase_length = Range.y - Range.x;
      const auto base_length = static_cast<uint32_t>(text.string().size()) - erase_length;
      if (base_length >= *max_length) return {};
      return Value.substr(0, *max_length - base_length);
    }

    yw::string<wchar_t> limited_input(uint2 Range, string_view<wchar_t> Value) const {
      yw::string<wchar_t> result;
      result.reserve(Value.size());
      for (const auto c : Value)
        if (!filter || filter(c)) result.push_back(c);
      const auto limited = limited_value(Range, result);
      if (limited.size() == result.size()) return result;
      return yw::string<wchar_t>(limited);
    }

    std::expected<void, error> replace_direct(uint2 Range, string_view<wchar_t> Value, bool PlaceCaretAfter) {
      const auto erase_length = Range.y - Range.x;
      const auto insert_length = static_cast<uint32_t>(Value.size());
      if (erase_length == 0 && insert_length == 0) return {};
      yw::string<wchar_t> new_string;
      new_string.reserve(text.string().size() - erase_length + insert_length);
      new_string.append(string_view<wchar_t>(text.string()).substr(0, Range.x));
      new_string.append(Value);
      new_string.append(string_view<wchar_t>(text.string()).substr(Range.y));
      text.string(std::move(new_string));
      const auto next_pos = static_cast<uint32_t>(Range.x + insert_length);
      caret = anchor = PlaceCaretAfter ? next_pos : Range.x;
      make_messy();
      if (!readonly && change_event) change_event(text.string());
      return {};
    }

    struct replace_command {
      slotid target{};
      uint2 range{};
      ministr<wchar_t> value{};

      void operator()() const {
        if (const auto sp = get_slot<edit>(target); !sp) return;
        else if (auto res = sp->replace_direct(range, value, true); !res) res.error().go_off();
      }
    };

    std::expected<void, error> replace(uint2 Range, string_view<wchar_t> Value, bool PlaceCaretAfter) {
      auto cmds = get_command_manager();
      if (!cmds) return cmds.error().relay();
      Value = limited_value(Range, Value);
      if (Range.x == Range.y && Value.empty()) return {};
      auto erased = ministr<wchar_t>(string_view<wchar_t>(text.string()).substr(Range.x, Range.y - Range.x));
      if (auto res = replace_direct(Range, Value, PlaceCaretAfter)) {
        (*cmds)->push(
          replace_command{.target = id, .range = Range, .value = ministr<wchar_t>(Value)},
          replace_command{.target = id, .range = uint2(Range.x, Range.x + Value.size()), .value = erased});
        return {};
      } else return res.error().relay();
    }

    std::expected<void, error> erase(uint2 Range) {
      auto cmds = get_command_manager();
      if (!cmds) return cmds.error().relay();
      auto erased = ministr<wchar_t>(string_view<wchar_t>(text.string()).substr(Range.x, Range.y - Range.x));
      if (auto res = replace_direct(Range, {}, true)) {
        (*cmds)->push(
          replace_command{.target = id, .range = Range, .value = {}},
          replace_command{.target = id, .range = uint2(Range.x, Range.x), .value = erased});
        return {};
      } else return res.error().relay();
    }

    std::expected<bool, error> erase_selected() {
      if (caret == anchor) return false;
      if (auto res = erase(selected_range()); !res) return res.error().relay();
      return true;
    }

    std::expected<void, error> erase_backward() {
      if (auto res = erase_selected(); !res) return res.error().relay();
      else if (*res) return {};
      if (caret == 0) return {};
      if (auto res = erase({caret - 1, caret}); !res) return res.error().relay();
      return {};
    }

    std::expected<void, error> erase_forward() {
      if (auto res = erase_selected(); !res) return res.error().relay();
      else if (*res) return {};
      if (caret >= static_cast<uint32_t>(text.string().size())) return {};
      if (auto res = erase({caret, caret + 1}); !res) return res.error().relay();
      return {};
    }

    std::expected<void, error> erase_word_backward() {
      if (auto res = erase_selected(); !res) return res.error().relay();
      else if (*res) return {};
      if (const auto next = find_word_boundary(caret, -1); next == caret) return {};
      else if (auto res = erase({next, caret}); !res) return res.error().relay();
      return {};
    }

    std::expected<void, error> erase_word_forward() {
      if (auto res = erase_selected(); !res) return res.error().relay();
      else if (*res) return {};
      if (const auto next = find_word_boundary(caret, +1); next == caret) return {};
      else if (auto res = erase({caret, next}); !res) return res.error().relay();
      return {};
    }
  };

  edit() noexcept = default;

  edit(derived_from<interface> auto& Parent, const source_line& sl = here()) {
    if (auto res = create(Parent)) *this = std::move(*res);
    else res.error().add_footprint().go_off(sl);
  }

  using selectable_label::font;
  using selectable_label::text;

  static std::expected<edit, error> create(derived_from<interface> auto& Parent) {
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
    if (auto theme = sp->get_color_theme(); !theme) return theme.error().relay();
    else if (auto res = sp->apply_color_theme(*(*theme), false); !res) return res.error().relay();
    return e;
  }

  //-- getter --//

  const auto& placeholder_text() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->placeholder_text;
  }

  const auto& placeholder_string() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->placeholder_text.string();
  }

  const auto& placeholder_color() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->placeholder_text.color();
  }

  const auto& max_length() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->max_length;
  }

  const auto& filter() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->filter;
  }

  const auto& change_event() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->change_event;
  }

  const auto& enter_event() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->enter_event;
  }

  bool readonly() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return {};
    }
    return sp->readonly;
  }

  //-- setter --//

  auto& placeholder_string(this auto& self, yw::string<wchar_t> s) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    sp->placeholder_text.string(std::move(s));
    sp->make_messy();
    return self;
  }

  auto& placeholder_color(this auto& self, const color& c) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    sp->placeholder_text.color(c);
    sp->make_dirty();
    return self;
  }

  auto& max_length(this auto& self, std::optional<uint32_t> v) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    sp->max_length = v;
    if (v && sp->text.string().size() > *v) {
      sp->text.string(yw::string<wchar_t>(string_view<wchar_t>(sp->text.string()).substr(0, *v)));
      sp->caret = yw::clamp(sp->caret, 0, sp->text.string().size());
      sp->anchor = yw::clamp(sp->anchor, 0, sp->text.string().size());
      sp->make_messy();
    }
    return self;
  }

  auto& filter(this auto& self, function<bool, wchar_t> f) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    sp->filter = std::move(f);
    return self;
  }

  auto& change_event(this auto& self, function<void, string_view<wchar_t>> f) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    sp->change_event = std::move(f);
    return self;
  }

  auto& enter_event(this auto& self, function<void, yw::key_event> f) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    sp->enter_event = std::move(f);
    return self;
  }

  auto& text(this auto& self, yw::text Text) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    sp->text = std::move(Text);
    sp->placeholder_text.font(sp->text.font());
    sp->caret = yw::clamp(sp->caret, 0, sp->text.string().size());
    sp->anchor = yw::clamp(sp->anchor, 0, sp->text.string().size());
    sp->make_messy();
    return self;
  }

  auto& font(this auto& self, font_config f) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    sp->text.font(f);
    sp->placeholder_text.font(std::move(f));
    sp->make_messy();
    return self;
  }

  auto& string(this auto& self, yw::string<wchar_t> s) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    s = sp->limited_input({0, static_cast<uint32_t>(sp->text.string().size())}, s);
    const bool changed = string_view<wchar_t>(sp->text.string()) != string_view<wchar_t>(s);
    sp->text.string(std::move(s));
    sp->caret = yw::clamp(sp->caret, 0, sp->text.string().size());
    sp->anchor = yw::clamp(sp->anchor, 0, sp->text.string().size());
    sp->make_messy();
    if (changed && !sp->readonly && sp->change_event) sp->change_event(sp->text.string());
    return self;
  }

  auto& readonly(this auto& self, bool v) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    sp->readonly = v;
    sp->make_dirty();
    return self;
  }
};
} // namespace yw::ui
