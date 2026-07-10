#pragma once
#include <ywx/window.h>

namespace yw::ui {

class edit : public selectable_label {
public:
  struct slot : selectable_label::slot {
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
      return {};
    }

    struct replace_command {
      slotid target{};
      uint2 range{};
      ministr<wchar_t> value{};
      void operator()() const {
        if (const auto sp = interface::slot::get<edit>(target); !sp) return;
        else if (auto res = sp->_replace(range, value, true); !res) res.error().go_off();
      }
    };

    std::expected<void, error> replace(uint2 Range, string_view<wchar_t> Value, bool PlaceCaretAfter) {
      const auto wsp = interface::slot::get<window>(window_id);
      if (!wsp) return std::unexpected(error(errors::invalid_argument));
      auto erased = ministr<wchar_t>(string_view<wchar_t>(text.string()).substr(Range.x, Range.y - Range.x));
      if (auto res = _replace(Range, Value, PlaceCaretAfter)) {
        wsp->commands.push(
          replace_command{.target = id, .range = Range, .value = ministr<wchar_t>(Value)},
          replace_command{.target = id, .range = uint2(Range.x, Range.x + Value.size()), .value = erased});
        return {};
      } else return res.error().relay();
    }

    std::expected<void, error> insert(uint32_t Pos, string_view<wchar_t> Value, bool PlaceCaretAfter) {
      const auto wsp = interface::slot::get<window>(window_id);
      if (!wsp) return std::unexpected(error(errors::invalid_argument));
      if (auto res = _replace({Pos, Pos}, Value, PlaceCaretAfter)) {
        wsp->commands.push(
          replace_command{.target = id, .range = uint2(Pos, Pos), .value = ministr<wchar_t>(Value)},
          replace_command{.target = id, .range = uint2(Pos, Pos + Value.size()), .value = {}});
        return {};
      } else return res.error().relay();
    }

    std::expected<void, error> erase(uint2 Range) {
      const auto wsp = interface::slot::get<window>(window_id);
      if (!wsp) return std::unexpected(error(errors::invalid_argument));
      auto erased = ministr<wchar_t>(string_view<wchar_t>(text.string()).substr(Range.x, Range.y - Range.x));
      if (auto res = _replace(Range, {}, true)) {
        wsp->commands.push(
          replace_command{.target = id, .range = Range, .value = {}},
          replace_command{.target = id, .range = uint2(Range.x, Range.x), .value = erased});
        return {};
      } else return res.error().relay();
    }

    virtual bool char_event(wchar_t c) override {
      if (!enabled || c < 0x20 || c == 0x7f || c == L'\r' || c == L'\n' || c == L'\t') return false;
      if (auto res = replace(_selected_range(), string_view<wchar_t>(&c, 1), true); !res) res.error().go_off();
      return true;
    }

    virtual bool key_event(yw::key_event e) override {
      if (!enabled || !e.down) return false;
      if (e.mods.ctrl) {
        if (e.key == keys::backspace) {
          if (auto res = _erase_word_backward(); !res) res.error().go_off();
          return true;
        } else if (e.key == keys::delete_) {
          if (auto res = _erase_word_forward(); !res) res.error().go_off();
          return true;
        } else if (e.key == keys::x) {
          if (caret == anchor) return false;
          clipboard.text(_selected_string());
          if (auto res = erase(_selected_range()); !res) res.error().go_off();
          return true;
        } else if (e.key == keys::v) {
          if (auto txt = clipboard.text())
            if (auto res = replace(_selected_range(), string_view<wchar_t>(*txt), false); !res) res.error().go_off();
          return true;
        } else if (e.key == keys::z) {
          if (const auto wsp = interface::slot::get<window>(window_id)) wsp && wsp->commands.undo();
          return true;
        } else if (e.key == keys::y) {
          if (const auto wsp = interface::slot::get<window>(window_id)) wsp && wsp->commands.redo();
          return true;
        }
      }
      if (selectable_label::slot::key_event(e)) return true;
      switch (e.key.code) {
      case VK_BACK:
        if (auto res = _erase_backward(); !res) res.error().go_off();
        return true;
      case VK_DELETE:
        if (auto res = _erase_forward(); !res) res.error().go_off();
        return true;
      }
      return false;
    }

  private:
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

  edit(derived_from<interface> auto& Parent, bool AutoColor = true, const source_line& sl = here()) {
    if (auto res = slot::create<edit>(Parent, AutoColor, sl)) {
      const auto sp = *res;
      _id = sp->id;
      sp->text_color = std::exchange(sp->colors.border, colors::transparent);
    } else res.error().add_footprint().go_off(sl);
  }

  static std::expected<edit, error> create(
    derived_from<interface> auto& Parent, bool AutoColor = true, const source_line& sl = here()) {
    edit e;
    if (auto res = slot::create<edit>(Parent, AutoColor, sl)) {
      const auto sp = *res;
      e._id = sp->id;
      sp->text_color = std::exchange(sp->colors.border, colors::transparent);
      return e;
    } else return res.error().relay();
  }
};
} // namespace yw::ui
