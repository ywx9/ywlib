#pragma once
#include <ywx/clipboard.h>
#include <ywx/window.h>

namespace yw::ui {

class edit : public frame {
public:
  struct slot : frame::slot {
    yw::text text = yw::text(L"");
    font_config font = font_config::default_;
    string<wchar_t> value{};
    color text_color = colors::black;
    color selection_color = color(0.20f, 0.47f, 0.86f, 0.35f);
    color caret_color = colors::black;
    uint32_t caret = 0;
    uint32_t anchor = 0;
    bool selecting = false;

    struct insert_command {
      interface::slotid target{};
      uint32_t pos = 0;
      ministr<wchar_t> value{};
    };

    struct erase_command {
      interface::slotid target{};
      uint2 range{};
      ministr<wchar_t> erased{};
    };

    struct redo_insert {
      insert_command cmd{};
      void operator()() const { (void)apply_insert(cmd, true); }
    };

    struct undo_insert {
      insert_command cmd{};
      void operator()() const {
        (void)apply_erase({cmd.target, uint2(cmd.pos, cmd.pos + uint32_t(cmd.value.size())), {}});
      }
    };

    struct redo_erase {
      erase_command cmd{};
      void operator()() const { (void)apply_erase(cmd); }
    };

    struct undo_erase {
      erase_command cmd{};
      void operator()() const { (void)apply_insert({cmd.target, cmd.range.x, cmd.erased}, true); }
    };

    float2 _text_offset() const noexcept {
      const auto inner_height = size.y - padding.y - padding.w;
      return float2(padding.x, padding.y + yw::max(0.0f, inner_height - text.size().y) * 0.5f);
    }

    float2 _text_origin() const noexcept { return pos + _text_offset(); }

    string_view<wchar_t> _selection_view() const noexcept {
      const auto sel = selection();
      return string_view<wchar_t>(value).substr(sel.x, sel.y - sel.x);
    }

    uint2 selection() const noexcept { return uint2(yw::min(caret, anchor), yw::max(caret, anchor)); }

    bool has_selection() const noexcept { return caret != anchor; }

    std::expected<void, error> refresh_text() {
      if (auto res = text.string(value); !res) return res.error().relay();
      return {};
    }

    virtual bool focusable() const override { return enabled && visible; }

    virtual std::expected<float2, error> get_necessary_size() const override {
      const auto inner = float2(yw::max(text.size().x, font.get_size() * 8.0f), yw::max(text.size().y, font.get_size())) +
                         padding.xy() + padding.zw();
      return vapply_r<float2>(_necessary_size, policy, minimum_size, required_size, inner);
    }

    virtual std::optional<float3> caret_pos() const override {
      if (caret == 0 || value.empty()) {
        const auto origin = _text_origin();
        return float3(origin.x, origin.y, yw::max(text.size().y, font.get_size()));
      }
      if (auto res = text.hittest(uint1{static_cast<uint32_t>(yw::min(caret - 1, uint32_t(value.size() - 1)))}, true)) {
        const auto origin = _text_origin();
        return float3(origin.x + res->z, origin.y + res->y, res->w);
      }
      return std::nullopt;
    }

    virtual std::expected<void, error> redraw() override {
      if (geometry_dirty) {
        geometry_dirty = false;
        if (auto res = relocate(); !res) return res.error().relay();
      }
      if (!visible) return {};
      if (auto res = _draw_background(); !res) return res.error().relay();

      const auto origin = _text_origin();
      if (has_selection() && !value.empty()) {
        const auto sel = selection();
        if (sel.x < sel.y) {
          if (auto rects = text.hittest_range(sel, origin)) {
            brush::color(selection_color);
            for (const auto& rect : *rects)
              if (auto res = fill_rectangle(rect.xy(), rect.zw()); !res) return res.error().relay();
          } else return rects.error().relay();
        }
      }

      brush::color(text_color);
      if (auto res = text.draw(origin); !res) return res.error().relay();

      if (const auto wsp = interface::slot::get<window>(window_id); wsp && wsp->focused_control_id == id) {
        const auto cp = caret_pos();
        if (cp && caret_color.a > 0.0f) {
          brush::color(caret_color);
          if (auto res = draw_line(cp->xy(), cp->xy() + float2(0.0f, cp->z), 1.0f); !res) return res.error().relay();
        }
      }

      if (auto res = _draw_foreground(); !res) return res.error().relay();
      return {};
    }

    virtual bool button_event(yw::button_event e) override {
      print(e);
      if (!enabled || e.key != keys::lbutton) return false;
      if (e.down) {
        selecting = true;
        const auto pos = _caret_from_point(float2(e.pos.x, e.pos.y));
        caret = pos;
        anchor = pos;
        if (auto res = make_dirty(); !res) res.error().go_off();
      } else selecting = false;
      return true;
    }

    virtual bool click_event(yw::button_event e) override {
      if (!enabled || e.down || e.key != keys::lbutton) return false;
      selecting = false;
      return true;
    }

    virtual bool double_click_event(yw::button_event e) override {
      if (!enabled || e.key != keys::lbutton) return false;
      anchor = 0;
      caret = static_cast<uint32_t>(value.size());
      if (auto res = make_dirty(); !res) res.error().go_off();
      return true;
    }

    virtual bool drag_event(yw::drag_event e) override {
      if (!enabled || !selecting || e.key != keys::lbutton) return false;
      const auto pos = _current_cursor_local_pos();
      caret = _caret_from_point(pos);
      if (auto res = make_dirty(); !res) res.error().go_off();
      return true;
    }

    virtual void focus_event(bool Focused) override {
      selecting = false;
      frame::slot::focus_event(Focused);
      if (auto res = make_dirty(); !res) res.error().go_off();
    }

    virtual bool char_event(wchar_t c) override {
      if (!enabled || c < 0x20 || c == 0x7f || c == L'\r' || c == L'\n' || c == L'\t') return false;
      return _replace_selection(ministr<wchar_t>(string<wchar_t>(1, c)), true);
    }

    virtual bool key_event(yw::key_event e) override {
      if (!enabled || !e.down) return false;
      if (e.mods.ctrl) {
        if (e.key == keys::a) {
          anchor = 0;
          caret = static_cast<uint32_t>(value.size());
          if (auto res = make_dirty(); !res) res.error().go_off();
          return true;
        }
        if (e.key == keys::c) {
          if (has_selection()) clipboard.text(_selection_view());
          return has_selection();
        }
        if (e.key == keys::x) {
          if (!has_selection()) return false;
          clipboard.text(_selection_view());
          const auto sel = selection();
          const auto removed = ministr<wchar_t>(_selection_view());
          _push_erase(sel, removed, false);
          return true;
        }
        if (e.key == keys::v) {
          if (auto txt = clipboard.text()) return _replace_selection(ministr<wchar_t>(*txt), false);
          return false;
        }
        if (e.key == keys::z) {
          const auto wsp = interface::slot::get<window>(window_id);
          return wsp && wsp->commands.undo();
        }
        if (e.key == keys::y) {
          const auto wsp = interface::slot::get<window>(window_id);
          return wsp && wsp->commands.redo();
        }
      }

      switch (e.key.code) {
      case VK_LEFT: _move_horizontal(-1, e.mods.shift); return true;
      case VK_RIGHT: _move_horizontal(+1, e.mods.shift); return true;
      case VK_HOME: _set_caret(0, e.mods.shift); return true;
      case VK_END: _set_caret(static_cast<uint32_t>(value.size()), e.mods.shift); return true;
      case VK_BACK: return _erase_backward();
      case VK_DELETE: return _erase_forward();
      }
      return false;
    }

    std::expected<void, error> set_value(string<wchar_t> next) {
      value = std::move(next);
      caret = static_cast<uint32_t>(yw::min(caret, static_cast<uint32_t>(value.size())));
      anchor = static_cast<uint32_t>(yw::min(anchor, static_cast<uint32_t>(value.size())));
      if (auto res = refresh_text(); !res) return res.error().relay();
      if (auto res = make_messy(); !res) return res.error().relay();
      return {};
    }

    std::expected<void, error> insert_text(uint32_t pos, string_view<wchar_t> sv, bool place_caret_after) {
      pos = static_cast<uint32_t>(yw::min(pos, static_cast<uint32_t>(value.size())));
      string<wchar_t> next;
      next.reserve(value.size() + sv.size());
      next.append(string_view<wchar_t>(value).substr(0, pos));
      next.append(sv);
      next.append(string_view<wchar_t>(value).substr(pos));
      value = std::move(next);
      caret = anchor = place_caret_after ? pos + static_cast<uint32_t>(sv.size()) : pos;
      if (auto res = refresh_text(); !res) return res.error().relay();
      if (auto res = make_messy(); !res) return res.error().relay();
      return {};
    }

    std::expected<ministr<wchar_t>, error> erase_text(uint2 range) {
      range.x = static_cast<uint32_t>(yw::min(range.x, static_cast<uint32_t>(value.size())));
      range.y = static_cast<uint32_t>(yw::min(range.y, static_cast<uint32_t>(value.size())));
      if (range.x > range.y) std::swap(range.x, range.y);
      const auto removed = ministr<wchar_t>(string_view<wchar_t>(value).substr(range.x, range.y - range.x));
      string<wchar_t> next;
      next.reserve(value.size() - (range.y - range.x));
      next.append(string_view<wchar_t>(value).substr(0, range.x));
      next.append(string_view<wchar_t>(value).substr(range.y));
      value = std::move(next);
      caret = anchor = range.x;
      if (auto res = refresh_text(); !res) return res.error().relay();
      if (auto res = make_messy(); !res) return res.error().relay();
      return removed;
    }

    static bool apply_insert(insert_command cmd, bool place_caret_after) {
      const auto sp = interface::slot::get<edit>(cmd.target);
      if (!sp) return false;
      if (auto res = sp->insert_text(cmd.pos, cmd.value, place_caret_after); !res) res.error().go_off();
      return true;
    }

    static bool apply_erase(erase_command cmd) {
      const auto sp = interface::slot::get<edit>(cmd.target);
      if (!sp) return false;
      if (auto res = sp->erase_text(cmd.range); !res) res.error().go_off();
      return true;
    }

  private:
    bool _execute(command_manager::command cmd, bool groupable) {
      if (!cmd.redo || !cmd.undo) return false;
      cmd.redo();
      const auto wsp = interface::slot::get<window>(window_id);
      if (!wsp) return false;
      wsp->commands.push(std::move(cmd), groupable);
      return true;
    }

    bool _execute(command_manager::group g) {
      if (g.commands.empty()) return false;
      for (const auto& cmd : g.commands)
        if (cmd.redo) cmd.redo();
      const auto wsp = interface::slot::get<window>(window_id);
      if (!wsp) return false;
      wsp->commands.push(std::move(g));
      return true;
    }

    float2 _current_cursor_local_pos() const noexcept {
      if (const auto wsp = interface::slot::get<window>(window_id)) {
        const auto screen = window::slot::cursor_pos;
        return float2(float(screen.x - wsp->pos.x), float(screen.y - wsp->pos.y));
      }
      return pos;
    }

    uint32_t _caret_from_point(float2 pt) const noexcept {
      const auto origin = _text_origin();
      const auto local = pt - origin;
      if (value.empty()) return 0;
      if (auto hit = text.hittest(local))
        return static_cast<uint32_t>(yw::min(yw::get<0>(*hit), static_cast<uint32_t>(value.size())));
      return static_cast<uint32_t>(value.size());
    }

    void _set_caret(uint32_t pos, bool keep_anchor) {
      caret = static_cast<uint32_t>(yw::min(pos, static_cast<uint32_t>(value.size())));
      if (!keep_anchor) anchor = caret;
      if (auto res = make_dirty(); !res) res.error().go_off();
    }

    void _move_horizontal(int dir, bool keep_anchor) {
      const auto next = dir < 0 ? (caret > 0 ? caret - 1 : 0)
                                : static_cast<uint32_t>(yw::min(caret + 1, static_cast<uint32_t>(value.size())));
      _set_caret(next, keep_anchor);
    }

    bool _erase_backward() {
      if (has_selection()) {
        const auto sel = selection();
        const auto removed = ministr<wchar_t>(_selection_view());
        _push_erase(sel, removed, false);
        return true;
      }
      if (caret == 0) return false;
      const uint2 range{caret - 1, caret};
      const auto removed = ministr<wchar_t>(string_view<wchar_t>(value).substr(range.x, 1));
      _push_erase(range, removed, false);
      return true;
    }

    bool _erase_forward() {
      if (has_selection()) {
        const auto sel = selection();
        const auto removed = ministr<wchar_t>(_selection_view());
        _push_erase(sel, removed, false);
        return true;
      }
      if (caret >= value.size()) return false;
      const uint2 range{caret, caret + 1};
      const auto removed = ministr<wchar_t>(string_view<wchar_t>(value).substr(range.x, 1));
      _push_erase(range, removed, false);
      return true;
    }

    bool _replace_selection(ministr<wchar_t> inserted, bool groupable) {
      const auto insert_sv = string_view<wchar_t>(inserted);
      command_manager::group g{};
      const auto sel = selection();
      if (sel.x < sel.y) {
        const erase_command erase{ id, sel, ministr<wchar_t>(_selection_view()) };
        g.commands.push_back(command_manager::command{
          .redo = redo_erase{erase},
          .undo = undo_erase{erase}});
      }
      if (!insert_sv.empty()) {
        const insert_command insert{ id, sel.x, std::move(inserted) };
        g.commands.push_back(command_manager::command{
          .redo = redo_insert{insert},
          .undo = undo_insert{insert}});
      }
      if (g.commands.empty()) return false;
      if (g.commands.size() == 1) return _execute(std::move(g.commands.front()), groupable);
      return _execute(std::move(g));
    }

    void _push_erase(uint2 range, ministr<wchar_t> removed, bool groupable) {
      const erase_command erase{ id, range, std::move(removed) };
      _execute(
        command_manager::command{
          .redo = redo_erase{erase},
          .undo = undo_erase{erase}},
        groupable);
    }
  };

  edit() noexcept = default;

  edit(derived_from<interface> auto& Parent, bool AutoColor = true, const source_line& sl = here()) {
    if (auto res = frame::slot::create<edit>(Parent, AutoColor, sl)) {
      const auto sp = *res;
      _id = sp->id;
      sp->text_color = std::exchange(sp->colors.border, colors::transparent);
    } else res.error().add_footprint().go_off(sl);
  }

  static std::expected<edit, error> create(
    derived_from<interface> auto& Parent, bool AutoColor = true, const source_line& sl = here()) {
    edit e;
    if (auto res = frame::slot::create<edit>(Parent, AutoColor, sl)) {
      const auto sp = *res;
      e._id = sp->id;
      sp->text_color = std::exchange(sp->colors.border, colors::transparent);
      return e;
    } else return res.error().relay();
  }

  const auto& text() const noexcept { ywlib_control_get(value); }
  const auto& text_color() const noexcept { ywlib_control_get(text_color); }
  const auto& selection_color() const noexcept { ywlib_control_get(selection_color); }
  const auto& caret_color() const noexcept { ywlib_control_get(caret_color); }
  uint32_t caret() const noexcept { ywlib_control_get(caret); }
  uint32_t anchor() const noexcept { ywlib_control_get(anchor); }

  auto& text(this auto& self, string<wchar_t> value) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    if (auto res = sp->set_value(std::move(value)); !res) res.error().go_off();
    return self;
  }

  auto& text_color(this auto& self, const color& c) noexcept { ywlib_control_set(text_color, c, dirty); }
  auto& selection_color(this auto& self, const color& c) noexcept { ywlib_control_set(selection_color, c, dirty); }
  auto& caret_color(this auto& self, const color& c) noexcept { ywlib_control_set(caret_color, c, dirty); }
};
} // namespace yw::ui
