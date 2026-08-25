#pragma once
#include <optional>

#include <ywx/ui_selectable_label.h>

namespace yw::ui {

class edit : public selectable_label {
public:
  struct slot : selectable_label::slot {
    yw::text placeholder_text = yw::text(L"");
    optional<color> placeholder_color;
    std::optional<uint32_t> max_length = std::nullopt;
    function<bool, wchar_t> filter{};
    function<bool, string_view<wchar_t>> change_event{};
    function<bool, yw::key_event> enter_event{};
    bool readonly = false;

    //-- override functions --//

    virtual color get_placeholder_color(const interface::slot* Window) const noexcept {
      if (placeholder_color) return *placeholder_color;
      if (auto theme = get_color_theme(Window)) return color((*theme)->text_muted, default_overlay_opacity.muted_text);
      return colors::transparent;
    }

    virtual color get_border_color(const interface::slot* Window) const noexcept override {
      if (border_color) return *border_color;
      if (auto theme = get_color_theme(Window)) return (*theme)->outline;
      return colors::transparent;
    }

    virtual std::expected<void, error> draw_forecontent(interface::slot* Window) override {
      if (!is_focused() && text.string().empty()) {
        if (auto res = update_scroll_offset(); !res) return res.error().relay();
        const auto origin = text_origin();
        const auto area = size - padding.xy() - padding.zw();
        const auto text_pos = align_position(origin, area, placeholder_text.size(), text_align);
        if (auto res = draw_text(text_pos, placeholder_text, get_placeholder_color(Window)); !res)
          return res.error().relay();
        return {};
      }
      return selectable_label::slot::draw_forecontent(Window);
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
        return enter_event ? enter_event(e) : true;
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

  class proxy : public selectable_label::proxy {
    friend class edit;
  protected:
    using selectable_label::proxy::proxy;
    edit::slot* _get_slot() const noexcept { return static_cast<edit::slot*>(_slot); }

  public:
    using selectable_label::proxy::font;
    using selectable_label::proxy::string;
    using selectable_label::proxy::text;

    //-- getter --//

    const auto& placeholder_text() const&& noexcept { return _get_slot()->placeholder_text; }
    const auto& placeholder_string() const&& noexcept { return _get_slot()->placeholder_text.string(); }
    color placeholder_color() const&& noexcept {
      return _get_slot()->get_placeholder_color(interface::slot::slots.get(_get_slot()->window_id));
    }
    const auto& max_length() const&& noexcept { return _get_slot()->max_length; }
    const auto& filter() const&& noexcept { return _get_slot()->filter; }
    const auto& change_event() const&& noexcept { return _get_slot()->change_event; }
    const auto& enter_event() const&& noexcept { return _get_slot()->enter_event; }
    bool readonly() const&& noexcept { return _get_slot()->readonly; }

    //-- setter --//

    auto placeholder_string(this auto&& Self, yw::string<wchar_t> String) noexcept {
      Self._get_slot()->placeholder_text.string(std::move(String));
      Self._messy = true;
      return std::move(Self);
    }

    auto placeholder_color(this auto&& Self, const color& Color) noexcept {
      Self._get_slot()->placeholder_color = Color;
      Self._dirty = true;
      return std::move(Self);
    }

    auto placeholder_color(this auto&& Self, none) noexcept {
      Self._get_slot()->placeholder_color = none();
      Self._dirty = true;
      return std::move(Self);
    }

    auto max_length(this auto&& Self, std::optional<uint32_t> MaxLength) noexcept {
      Self._get_slot()->max_length = MaxLength;
      if (MaxLength && Self._get_slot()->text.string().size() > *MaxLength) {
        Self._get_slot()->text.string(
          yw::string<wchar_t>(string_view<wchar_t>(Self._get_slot()->text.string()).substr(0, *MaxLength)));
        Self._get_slot()->caret = yw::clamp(Self._get_slot()->caret, 0, Self._get_slot()->text.string().size());
        Self._get_slot()->anchor = yw::clamp(Self._get_slot()->anchor, 0, Self._get_slot()->text.string().size());
        Self._messy = true;
      }
      return std::move(Self);
    }

    auto filter(this auto&& Self, function<bool, wchar_t> Filter) noexcept {
      Self._get_slot()->filter = std::move(Filter);
      return std::move(Self);
    }

    auto change_event(this auto&& Self, function<bool, string_view<wchar_t>> Event) noexcept {
      Self._get_slot()->change_event = std::move(Event);
      return std::move(Self);
    }

    auto enter_event(this auto&& Self, function<bool, yw::key_event> Event) noexcept {
      Self._get_slot()->enter_event = std::move(Event);
      return std::move(Self);
    }

    auto text(this auto&& Self, yw::text Text) noexcept {
      Self._get_slot()->text = std::move(Text);
      Self._get_slot()->placeholder_text.font(Self._get_slot()->text.font());
      Self._get_slot()->caret = yw::clamp(Self._get_slot()->caret, 0, Self._get_slot()->text.string().size());
      Self._get_slot()->anchor = yw::clamp(Self._get_slot()->anchor, 0, Self._get_slot()->text.string().size());
      Self._messy = true;
      return std::move(Self);
    }

    auto font(this auto&& Self, font_config Font) noexcept {
      Self._get_slot()->text.font(Font);
      Self._get_slot()->placeholder_text.font(std::move(Font));
      Self._messy = true;
      return std::move(Self);
    }

    auto string(this auto&& Self, yw::string<wchar_t> String) noexcept {
      String = Self._get_slot()->limited_input(
        {0, static_cast<uint32_t>(Self._get_slot()->text.string().size())}, String);
      const bool changed = string_view<wchar_t>(Self._get_slot()->text.string()) != string_view<wchar_t>(String);
      Self._get_slot()->text.string(std::move(String));
      Self._get_slot()->caret = yw::clamp(Self._get_slot()->caret, 0, Self._get_slot()->text.string().size());
      Self._get_slot()->anchor = yw::clamp(Self._get_slot()->anchor, 0, Self._get_slot()->text.string().size());
      Self._messy = true;
      if (changed && !Self._get_slot()->readonly && Self._get_slot()->change_event)
        Self._get_slot()->change_event(Self._get_slot()->text.string());
      return std::move(Self);
    }

    auto readonly(this auto&& Self, bool Readonly) noexcept {
      Self._get_slot()->readonly = Readonly;
      Self._dirty = true;
      return std::move(Self);
    }
  };

  edit() noexcept = default;

  edit(derived_from<interface> auto& Parent, const source_line& sl = here()) {
    if (auto res = create(Parent)) *this = std::move(*res);
    else res.error().add_footprint().go_off(sl);
  }

  using selectable_label::font;
  using selectable_label::text;

  static std::expected<edit, error> create() {
    edit e;
    edit::slot* sp;
    if (auto res = create_control<edit>()) sp = *res;
    else return res.error().relay();
    e._id = sp->id;
    sp->policy = {ui::free, ui::fit};
    sp->text_align = alignment::left;
    return e;
  }

  static std::expected<edit, error> create(derived_from<interface> auto& Parent) {
    auto res = create();
    if (!res) return res.error().relay();
    if (auto attached = res->attach(Parent); !attached) return attached.error().relay();
    return res;
  }

  yw_control_getter(placeholder_text);
  yw_control_getter_setter(placeholder_string, yw::string<wchar_t>);
  yw_control_getter_setter(placeholder_color, color);
  auto placeholder_color(this auto& Self, none None) noexcept {
    return typename remove_cvref<decltype(Self)>::proxy(get_slot(&Self)).placeholder_color(None);
  }
  yw_control_getter_setter(max_length, std::optional<uint32_t>);
  yw_control_getter_setter(filter, function<bool, wchar_t>);
  yw_control_getter_setter(change_event, function<bool, string_view<wchar_t>>);
  yw_control_getter_setter(enter_event, function<bool, yw::key_event>);
  yw_control_getter_setter(text, yw::text);
  yw_control_getter_setter(font, font_config);
  yw_control_getter_setter(string, yw::string<wchar_t>);
  yw_control_getter_setter(readonly, bool);
};
} // namespace yw::ui
