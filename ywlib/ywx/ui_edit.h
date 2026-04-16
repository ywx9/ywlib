#pragma once
#include "ywx/background.h"
#include "ywx/clipboard.h"
#include "ywx/ime.h"
#include "ywx/text_layout.h"
#include "ywx/ui_control.h"
#include "ywx/window.h"
#include <print>

namespace yw::ui {

/// \note edit::max_lengthは、npos以外を設定した場合、そのサイズだけのバッファを確保する。

class edit : public control {
public:
  class slot : public control::slot {
    // 文字グループ: 0=alnum, 1=ascii記号, 2=その他
    static int char_group(wchar_t c) noexcept { return !is_alnum(c) + !is_ascii(c); }

    template<bool Foward> size_t next_char_group(size_t pos) const {
      const auto& t = text();
      if (t.empty()) return 0;
      if constexpr (Foward) {
        if (pos >= t.size()) return t.size();
        const auto cg = char_group(t[pos]);
        while (pos < t.size() && cg == char_group(t[pos])) ++pos;
      } else {
        if (pos == 0) return 0;
        const auto cg = char_group(t[pos - 1]);
        while (pos > 0 && cg == char_group(t[pos - 1])) --pos;
      }
      return pos;
    }

    float2 preferred_size() const noexcept {
      const auto tsz = vapply_r<float2>(yw::max, float2(), text.size(), placeholder.size());
      const auto inner = tsz + padding.xy() + padding.zw();
      return vapply_r<float2>(yw::max, min_size, inner, size * constrained);
    }

    float2 text_origin() const noexcept {
      const auto extra = size.y - padding.y - padding.w - text.size().y;
      return pos + padding.xy() + float2(-scroll, extra * 0.5f);
    }

    std::wstring_view selected_text() const noexcept {
      if (caret_pos < selection_anchor)
        return std::wstring_view(text().data() + caret_pos, selection_anchor - caret_pos);
      else if (caret_pos > selection_anchor)
        return std::wstring_view(text().data() + selection_anchor, caret_pos - selection_anchor);
      else return {};
    }

    void erase_text(size_t Pos, size_t Count) {
      if (auto h = text.open_handle()) {
        const auto i = h->begin() + Pos;
        h->erase(i, i + Count);
        make_dirty();
      }
    }

    void insert_text(size_t Pos, std::wstring_view Str) {
      if (auto h = text.open_handle()) {
        const auto i = h->begin() + Pos;
        h->insert_range(i, Str);
        make_dirty();
      }
    }

    void insert_char(size_t Pos, wchar_t Char) {
      if (auto h = text.open_handle()) {
        h->insert(h->begin() + Pos, Char);
        make_dirty();
      }
    }

    void erase_char(size_t Pos) {
      if (auto h = text.open_handle()) {
        h->erase(h->begin() + Pos);
        make_dirty();
      }
    }

  public:
    yw::background background = colors::white;
    color border_color = colors::black;
    float border_width = 1.0f;
    float4 padding = float4::fill(5.0f);

    yw::text text = assume(yw::text::create(L""));
    color text_color = colors::black;
    size_t max_length = npos;

    yw::text placeholder = assume(yw::text::create(L""));
    color placeholder_color = colors::gray;

    color caret_color = colors::black;
    float caret_width = 1.0f;
    uint32_t caret_pos = 0;

    color selection_color = color(0.2f, 0.45f, 1.0f, 0.35f);
    uint32_t selection_anchor = 0;

    float scroll = 0.0f;
    bool selecting = false;

    function<void, event::button> on_button;
    function<void> on_change;
    function<void> on_enter;
    function<void, bool> on_focus;
    function<bool, wchar_t> filter;

    virtual float2 calculate_size() const noexcept override { return preferred_size(); }
    virtual void update_size() noexcept override {
      min_size = vapply_r<float2>(yw::max, min_size, float2());
      size = preferred_size();
    }

    virtual void draw() const override {
      if (!visible) return;
      const auto wsp = system::slot_address<window>(window_id);
      if (!wsp) return;
      draw_background(pos, size, background);
      brush.color(border_color);
      draw_round_rectangle(pos, size, radius, border_width);
      const auto clip = D2D1::RectF(pos.x, pos.y, pos.x + size.x, pos.y + size.y);
      d2d.context()->PushAxisAlignedClip(clip, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
      const auto text_pos = text_origin();
      if (text.empty()) {
        if (wsp->focused_control != id && !placeholder.empty()) {
          brush.color(placeholder_color);
          draw_text(text_pos, placeholder);
        }
      } else {
        if (caret_pos != selection_anchor) {
          const auto range = float2(yw::min(caret_pos, selection_anchor), yw::max(caret_pos, selection_anchor));
          if (auto htrr = text.hit_test_range(range, text_pos)) {
            brush.color(selection_color);
            for (const auto& xyhw : *htrr) fill_rectangle(xyhw.xy(), xyhw.zw());
          }
        } else if (wsp->focused_control == id) {
          if (auto htr = text.hit_test(caret_pos)) {
            brush.color(caret_color);
            const auto pt = text_pos + htr->xy();
            draw_line(pt, pt + float2(0, htr->w));
          }
        }
        brush.color(text_color);
        draw_text(text_pos, text);
      }
      d2d.context()->PopAxisAlignedClip();
    }

    virtual void button_event(event::button e) override {
      if (!enabled) return;
      if (e.code == key::lbutton && !e.down) selecting = false;
      if (text.empty()) {
        if (on_button) on_button(e);
        return;
      }
      const auto wsp = system::slot_address<window>(window_id);
      if (!wsp) return;
      if (e.code == key::lbutton && e.down) {
        selecting = true;
        wsp->commands.end_grouping();
        const auto htr = text.hit_test(float2(e.pos));
        if (!htr) return;
        auto hit_pos = *htr;
        if (e.double_click) {
          const auto& s = text();
          const auto len = static_cast<uint32_t>(s.size());
          const auto i = yw::min(hit_pos, len - 1);
          const auto cg = char_group(s[i]);
          auto first = i;
          while (first > 0 && char_group(s[first - 1]) == cg) --first;
          auto last = i + 1;
          while (last < len && char_group(s[last]) == cg) ++last;
          if (e.shift) {
            if (selection_anchor < first) caret_pos = last;
            else if (selection_anchor > last) caret_pos = first;
            else selection_anchor = first, caret_pos = last;
          } else selection_anchor = first, caret_pos = last;
        } else {
          if (e.shift) {
            if (selection_anchor < hit_pos) caret_pos = hit_pos + 1;
            else caret_pos = hit_pos;
          } else selection_anchor = caret_pos = hit_pos;
        }
      }
      make_dirty();
    }

    virtual void drag_event(event::drag e) override {
      if (!enabled || !selecting || e.code != key::lbutton || text.empty()) return;
      const auto htr = text.hit_test(float2(e.delta));
      if (!htr) return;
      const auto hit_pos = *htr;
      if (caret_pos != hit_pos) {
        caret_pos = hit_pos;
        make_dirty();
      }
    }

    virtual bool focus_event(bool focused) override {
      if (const auto wsp = system::slot_address<window>(window_id); wsp && !focused) wsp->commands.end_grouping();
      if (!focused) selecting = false;
      make_dirty();
      if (enabled && on_focus) on_focus(focused);
      return enabled;
    }

    virtual void char_event(wchar_t c) override {
      if (!enabled) return;
      if (c < 0x20 || c == 0x7f || text().size() >= max_length || (filter && !filter(c))) return;
      if (caret_pos != selection_anchor) {
        const wchar_t s[2] = {c, L'\0'};
        if (caret_pos < selection_anchor) selection_anchor = caret_pos = replace_text(caret_pos, selection_anchor - caret_pos, s);
        else selection_anchor = caret_pos = replace_text(selection_anchor, caret_pos - selection_anchor, s);
      } else {
        if (const auto wsp = system::slot_address<window>(window_id); wsp)
          wsp->commands.push(
            [id = id, pos = caret_pos, c = c]() {
              if (const auto esp = system::slot_address<edit>(id); esp) esp->insert_char(pos, c);
            },
            [id = id, pos = caret_pos]() {
              if (const auto esp = system::slot_address<edit>(id); esp) esp->erase_char(pos);
            },
            true);
        insert_char(caret_pos, c);
        selection_anchor = ++caret_pos;
      }
    }

    virtual bool key_event(event::key e) override {
      if (!enabled || !e.down) return false;
      const auto& t = text();
      const bool upper = is_upper(static_cast<char8_t>(e.code));
      if (upper && e.ctrl) {
        switch (e.code) {
        case key::c:
          if (const auto sv = selected_text(); !sv.empty()) clipboard.text(sv);
          return true;
        case key::x:
          if (const auto sv = selected_text(); !sv.empty()) {
            clipboard.text(sv);
            selection_anchor = caret_pos = replace_text(yw::min(caret_pos, selection_anchor), sv.size(), {});
          }
          return true;
        case key::v:
          if (const auto cbt = clipboard.text()) {
            const auto sv = *cbt;
            if (caret_pos == selection_anchor) selection_anchor = caret_pos = replace_text(caret_pos, 0, sv);
            else if (caret_pos < selection_anchor)
              selection_anchor = caret_pos = replace_text(caret_pos, selection_anchor - caret_pos, sv);
            else selection_anchor = caret_pos = replace_text(selection_anchor, caret_pos - selection_anchor, sv);
          }
          return true;
        case key::z:
          if (const auto wsp = system::slot_address<window>(window_id)) {
            if (e.shift) wsp->commands.redo();
            else wsp->commands.undo();
          }
          return true;
        case key::y:
          if (const auto wsp = system::slot_address<window>(window_id)) wsp->commands.redo();
          return true;
        case key::a:
          selection_anchor = 0;
          caret_pos = t.size();
          make_dirty();
          return true;
        default: return false;
        }
      }
      switch (e.code) {
      case key::left:
        if (e.ctrl) caret_pos = next_char_group<false>(caret_pos);
        else if (caret_pos > 0) --caret_pos;
        if (!e.shift) selection_anchor = caret_pos;
        make_dirty();
        return true;
      case key::right:
        if (e.ctrl) caret_pos = next_char_group<true>(caret_pos);
        else if (caret_pos < t.size()) ++caret_pos;
        if (!e.shift) selection_anchor = caret_pos;
        make_dirty();
        return true;
      case key::home:
        caret_pos = 0;
        if (!e.shift) selection_anchor = caret_pos;
        make_dirty();
        return true;
      case key::end:
        caret_pos = t.size();
        if (!e.shift) selection_anchor = caret_pos;
        make_dirty();
        return true;
      case key::backspace:
        if (caret_pos == selection_anchor) {
          if (caret_pos == 0) return true;
          if (e.ctrl) {
            const auto new_pos = next_char_group<false>(caret_pos);
            selection_anchor = caret_pos = replace_text(new_pos, caret_pos - new_pos, {});
          } else {
            --caret_pos;
            if (const auto wsp = system::slot_address<window>(window_id); wsp)
              wsp->commands.push(
                [id = id, pos = caret_pos]() {
                  if (const auto esp = system::slot_address<edit>(id); esp) esp->erase_char(pos);
                },
                [id = id, pos = caret_pos, c = t[caret_pos]]() {
                  if (const auto esp = system::slot_address<edit>(id); esp) esp->insert_char(pos, c);
                },
                true);
            erase_char(caret_pos);
            selection_anchor = caret_pos;
            make_dirty();
          }
        } else if (caret_pos < selection_anchor) {
          selection_anchor = caret_pos = replace_text(caret_pos, selection_anchor - caret_pos, {});
        } else {
          selection_anchor = caret_pos = replace_text(selection_anchor, caret_pos - selection_anchor, {});
        }
        return true;
      case key::delete_:
        if (caret_pos == selection_anchor) {
          if (caret_pos == t.size()) return true;
          if (e.ctrl) {
            const auto new_pos = next_char_group<true>(caret_pos);
            selection_anchor = caret_pos = replace_text(caret_pos, new_pos - caret_pos, {});
          } else {
            if (const auto wsp = system::slot_address<window>(window_id); wsp)
              wsp->commands.push(
                [id = id, pos = caret_pos]() {
                  if (const auto esp = system::slot_address<edit>(id); esp) esp->erase_char(pos);
                },
                [id = id, pos = caret_pos, c = t[caret_pos]]() {
                  if (const auto esp = system::slot_address<edit>(id); esp) esp->insert_char(pos, c);
                },
                true);
            erase_char(caret_pos);
            make_dirty();
          }
        } else if (caret_pos < selection_anchor) {
          selection_anchor = caret_pos = replace_text(caret_pos, selection_anchor - caret_pos, {});
        } else {
          selection_anchor = caret_pos = replace_text(selection_anchor, caret_pos - selection_anchor, {});
        }
        return true;
      case key::enter:
        if (on_enter) on_enter();
        return true;
      }
      return false;
    }

    virtual float2 ime_position() const override {
      if (auto res = text.hit_test(caret_pos)) return text_origin() + res->xy();
      return {};
    }

    virtual void ime_insert_text(std::wstring_view text) override {
      if (caret_pos == selection_anchor) selection_anchor = caret_pos = replace_text(caret_pos, 0, text);
      else if (caret_pos < selection_anchor)
        selection_anchor = caret_pos = replace_text(caret_pos, selection_anchor - caret_pos, text);
      else selection_anchor = caret_pos = replace_text(selection_anchor, caret_pos - selection_anchor, text);
    }

    size_t replace_text(size_t Pos, size_t Count, std::wstring_view Str) {
      command_manager::group g;
      ministr erased;
      ministr inserted;
      ministr trimmed;
      bool changed = false;
      size_t new_caret_pos = Pos;
      if (auto h = text.open_handle()) {
        if (Count != 0) erased = ministr(std::wstring_view(h->data() + Pos, Count));
        if (!Str.empty()) {
          if (filter) inserted = ministr(Str | std::views::filter(filter));
          else inserted = ministr(Str);
          if (Pos + inserted.size() > max_length) inserted.resize(max_length - Pos);
        }
        const auto erased_size = erased.size();
        const auto temp_size = h->size() + inserted.size() - erased_size;
        if (temp_size > max_length) {
          trimmed = ministr(std::wstring_view(h->data() + Pos + erased_size, max_length - temp_size));
        }
        auto i = h->begin() + Pos;
        if (erased_size != 0) {
          changed = true;
          i = h->erase(i, i + erased_size);
          g.commands.push_back(
            {[id = id, pos = Pos, count = Count]() {
               if (const auto esp = system::slot_address<edit>(id)) esp->erase_text(pos, count);
             },
              [id = id, pos = Pos, str = std::move(erased)]() {
                if (const auto esp = system::slot_address<edit>(id)) esp->insert_text(pos, str);
              }});
        }
        if (!inserted.empty()) {
          changed = true;
          if (!trimmed.empty()) { // textのバッファ再確保を避けるため、削除から先に進める
            h->resize(h->size() - trimmed.size());
            g.commands.push_back(
              {[id = id, pos = Pos, count = trimmed.size()]() {
                 if (const auto esp = system::slot_address<edit>(id)) esp->erase_text(pos, count);
               },
                [id = id, pos = Pos, str = std::move(trimmed)]() {
                  if (const auto esp = system::slot_address<edit>(id)) esp->insert_text(pos, str);
                }});
          }
          const auto inserted_size = inserted.size();
          new_caret_pos = Pos + inserted_size;
          h->insert_range(i, inserted);
          g.commands.push_back(
            {[id = id, pos = Pos, str = std::move(inserted)]() {
               if (const auto esp = system::slot_address<edit>(id)) esp->insert_text(pos, str);
             },
              [id = id, pos = Pos, count = inserted_size]() {
                if (const auto esp = system::slot_address<edit>(id)) esp->erase_text(pos, count);
              }});
        }
      }
      if (changed) {
        if (const auto wsp = system::slot_address<window>(window_id)) wsp->commands.push(std::move(g));
      }
      make_dirty();
      if (changed && on_change) on_change();
      return new_caret_pos;
    }
  };

  using control::operator bool;
  edit() noexcept = default;
  edit(derived_from<unknown> auto& Layout) {
    if (auto res = create_control<edit>(Layout)) _id = *res;
  }

  const auto& background() const { return unsafe_get(&slot::background); }
  void background(yw::background bg) { safe_set(&slot::background, std::move(bg)); }

  const auto& border_color() const { return unsafe_get(&slot::border_color); }
  void border_color(const color& c) { safe_set(&slot::border_color, c); }

  const auto& border_width() const { return unsafe_get(&slot::border_width); }
  void border_width(float w) { safe_set(&slot::border_width, w); }

  const auto& padding() const { return unsafe_get(&slot::padding); }
  void padding(const float4& p) { safe_set_size(&slot::padding, p); }

  const auto& text() const { return unsafe_get(&slot::text); }
  auto& text() { return unsafe_get_size(&slot::text); }
  template<stringable S> void text(S&& value) {
    if (auto csp = system::slot_address<slot>(_id)) {
      auto s = unicode<wchar_t>(static_cast<S&&>(value));
      if (csp->max_length != npos && s.size() > csp->max_length) s.resize(csp->max_length);
      csp->text(std::move(s));
      csp->caret_pos = yw::min(csp->caret_pos, csp->text().size());
      csp->make_messy();
    }
  }

  const auto& placeholder() const { return unsafe_get(&slot::placeholder); }
  auto& placeholder() { return unsafe_get_size(&slot::placeholder); }
  template<stringable S> void placeholder(S&& value) {
    if (auto csp = system::slot_address<slot>(_id)) {
      csp->placeholder(static_cast<S&&>(value));
      csp->make_messy();
    }
  }

  const auto& text_color() const { return unsafe_get(&slot::text_color); }
  void text_color(const color& c) { safe_set(&slot::text_color, c); }

  const auto& placeholder_color() const { return unsafe_get(&slot::placeholder_color); }
  void placeholder_color(const color& c) { safe_set(&slot::placeholder_color, c); }

  const auto& caret_color() const { return unsafe_get(&slot::caret_color); }
  void caret_color(const color& c) { safe_set(&slot::caret_color, c); }

  size_t caret_pos() const { return unsafe_get(&slot::caret_pos); }
  void caret_pos(uint1 value) {
    if (const auto csp = system::slot_address<slot>(_id))
      csp->caret_pos = csp->selection_anchor = yw::min(value.x, csp->text().size());
  }

  size_t max_length() const { return unsafe_get(&slot::max_length); }
  void max_length(std::optional<size_t> value) {
    if (const auto csp = system::slot_address<slot>(_id)) {
      const auto limit = value.value_or(npos);
      const auto text_size = csp->text().size();
      csp->max_length = limit;
      if (text_size > limit) csp->replace_text(limit, text_size - limit, {});
    }
  }

  const auto& on_button() const { return unsafe_get(&slot::on_button); }
  void on_button(function<void, event::button> f) { safe_set(&slot::on_button, std::move(f)); }

  const auto& on_change() const { return unsafe_get(&slot::on_change); }
  void on_change(function<void> f) { safe_set(&slot::on_change, std::move(f)); }

  const auto& on_enter() const { return unsafe_get(&slot::on_enter); }
  void on_enter(function<void> f) { safe_set(&slot::on_enter, std::move(f)); }

  const auto& on_focus() const { return unsafe_get(&slot::on_focus); }
  void on_focus(function<void, bool> f) { safe_set(&slot::on_focus, std::move(f)); }

  const auto& filter() const { return unsafe_get(&slot::filter); }
  void filter(function<bool, wchar_t> f) { safe_set(&slot::filter, std::move(f)); }
};
} // namespace yw::ui
