#pragma once
#include "ywx/clipboard.h"
#include "ywx/ime.h"
#include "ywx/scrollbar.h"
#include "ywx/text_layout.h"
#include "ywx/ui_control.h"
#include "ywx/window.h"

namespace yw::ui {

class multiline : public control {
public:
  class slot : public control::slot {
  public:
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

    float2 client_pos() const noexcept { return pos + padding.xy(); }

    float2 client_size() const noexcept {
      const auto bars = float2(scrollbar_thickness, scrollbar_thickness);
      return vapply_r<float2>(yw::max, float2(), size - padding.xy() - padding.zw() - bars);
    }

    float2 text_origin() const noexcept { return client_pos() - scroll; }

    float2 text_size_estimate() const noexcept {
      auto ts = text.size();
      ts.y = yw::max(ts.y, text.font_size());
      return vapply_r<float2>(yw::max, float2(), ts);
    }

    void sync_text_metrics() {
      clamp_scroll();
      update_scrollbars(false);
      make_dirty();
    }

    void clamp_scroll() {
      const auto cs = client_size();
      const auto ts = text_size_estimate();
      const float max_x = yw::max(0.0f, ts.x - cs.x);
      const float max_y = yw::max(0.0f, ts.y - cs.y);
      scroll.x = std::clamp(scroll.x, 0.0f, max_x);
      scroll.y = std::clamp(scroll.y, 0.0f, max_y);
    }

    void update_scrollbars(bool notify = false) {
      const auto cp = client_pos();
      const auto cs = client_size();

      vscroll.button_extent(scrollbar_thickness);
      hscroll.button_extent(scrollbar_thickness);
      vscroll.min_thumb_extent(18.0f);
      hscroll.min_thumb_extent(18.0f);
      vscroll.rect(float2(cp.x + cs.x, cp.y), float2(scrollbar_thickness, cs.y));
      hscroll.rect(float2(cp.x, cp.y + cs.y), float2(cs.x, scrollbar_thickness));

      const auto ts = text_size_estimate();
      vscroll.content(ts.y, cs.y, scroll.y);
      hscroll.content(ts.x, cs.x, scroll.x);
      if (notify) {
        vscroll.value(scroll.y, true);
        hscroll.value(scroll.x, true);
      } else {
        vscroll.value(scroll.y, false);
        hscroll.value(scroll.x, false);
      }
    }

    std::wstring_view selected_text() const noexcept {
      if (caret_pos < selection_anchor)
        return std::wstring_view(text().data() + caret_pos, selection_anchor - caret_pos);
      if (caret_pos > selection_anchor)
        return std::wstring_view(text().data() + selection_anchor, caret_pos - selection_anchor);
      return {};
    }

    uint32_t hit_caret(float2 point) const {
      if (text.empty()) return 0;
      const auto cp = client_pos();
      const auto cs = client_size();
      if (point.x < cp.x || point.y < cp.y) return 0;
      if (point.x > cp.x + cs.x || point.y > cp.y + cs.y) return static_cast<uint32_t>(text().size());

      auto local = point - text_origin();
      local.x = yw::max(0.0f, local.x);
      local.y = yw::max(0.0f, local.y);
      if (auto res = text.hit_test(local, true)) return *res;
      return static_cast<uint32_t>(text().size());
    }

    void ensure_caret_visible() {
      if (auto htr = text.hit_test(caret_pos)) {
        const auto cp = client_pos();
        const auto cs = client_size();
        const auto caret = text_origin() + htr->xy();
        const float caret_h = yw::max(1.0f, htr->w);

        if (caret.x < cp.x) scroll.x -= (cp.x - caret.x);
        else if (caret.x > cp.x + cs.x) scroll.x += (caret.x - (cp.x + cs.x));

        if (caret.y < cp.y) scroll.y -= (cp.y - caret.y);
        else if (caret.y + caret_h > cp.y + cs.y) scroll.y += (caret.y + caret_h - (cp.y + cs.y));

        clamp_scroll();
        update_scrollbars(false);
      }
    }

    void erase_text(size_t Pos, size_t Count) {
      if (auto h = text.open_handle()) {
        const auto i = h->begin() + Pos;
        h->erase(i, i + Count);
        sync_text_metrics();
      }
    }

    void insert_text(size_t Pos, std::wstring_view Str) {
      if (auto h = text.open_handle()) {
        const auto i = h->begin() + Pos;
        h->insert_range(i, Str);
        sync_text_metrics();
      }
    }

    void insert_char(size_t Pos, wchar_t Char) {
      if (auto h = text.open_handle()) {
        h->insert(h->begin() + Pos, Char);
        sync_text_metrics();
      }
    }

    void erase_char(size_t Pos) {
      if (auto h = text.open_handle()) {
        h->erase(h->begin() + Pos);
        sync_text_metrics();
      }
    }

    bool handle_scroll_input(event::button e) {
      const bool used_v = vscroll.button_event(e);
      const bool used_h = hscroll.button_event(e);
      if (!used_v && !used_h) return false;
      scroll.y = float(vscroll.value());
      scroll.x = float(hscroll.value());
      clamp_scroll();
      update_scrollbars(false);
      make_dirty();
      return true;
    }

    float4 padding = float4::fill(4.0f);
    float scrollbar_thickness = 14.0f;

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

    float2 scroll{};
    bool selecting = false;
    mutable comptr<ID2D1Layer> text_layer{};

    yw::scrollbar vscroll{yw::scrollbar::orientation::vertical};
    yw::scrollbar hscroll{yw::scrollbar::orientation::horizontal};

    function<void, event::button> on_button;
    function<void> on_change;
    function<void, bool> on_focus;
    function<bool, wchar_t> filter;

    slot() {
      vscroll.auto_hide(false);
      hscroll.auto_hide(false);
      vscroll.on_change = [this](double v) {
        scroll.y = float(v);
        clamp_scroll();
        make_dirty();
      };
      hscroll.on_change = [this](double v) {
        scroll.x = float(v);
        clamp_scroll();
        make_dirty();
      };
    }

    virtual float2 calculate_size() const noexcept override {
      const float2 prefer{220.0f, 140.0f};
      const auto inner = prefer + padding.xy() + padding.zw() + float2(scrollbar_thickness, scrollbar_thickness);
      return vapply_r<float2>(yw::max, min_size, inner, size * constrained);
    }

    virtual void update_size() noexcept override {
      min_size = vapply_r<float2>(yw::max, min_size, float2());
      size = calculate_size();
      clamp_scroll();
      update_scrollbars(false);
      update_geometry();
    }

    virtual void update_layout(float2 Pos, float2 Area) override {
      control::slot::update_layout(Pos, Area);
      clamp_scroll();
      update_scrollbars(false);
    }

    virtual void draw() const override {
      if (!visible) return;
      const auto wsp = system::slot_address<window>(window_id);
      if (!wsp) return;
      draw_background();

      const auto cp = client_pos();
      const auto cs = client_size();
      const auto clip = D2D1::RectF(cp.x, cp.y, cp.x + cs.x, cp.y + cs.y);
      if (!text_layer) {
        const auto hr = d2d.context()->CreateLayer(nullptr, &text_layer.get());
        if (FAILED(hr)) return;
      }
      const auto params = D2D1::LayerParameters(clip, nullptr, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
      d2d.context()->PushLayer(params, text_layer.get());

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
            draw_line(pt, pt + float2(0, yw::max(1.0f, htr->w)), caret_width);
          }
        }
        brush.color(text_color);
        draw_text(text_pos, text);
      }
      d2d.context()->PopLayer();

      vscroll.draw();
      hscroll.draw();
    }

    virtual void move_event(event::move e) override {
      const bool hv = vscroll.move_event(e);
      const bool hh = hscroll.move_event(e);
      if (hv || hh) make_dirty();
    }

    virtual void button_event(event::button e) override {
      if (!enabled) return;
      if (handle_scroll_input(e)) {
        selecting = false;
        if (on_button) on_button(e);
        return;
      }

      if (e.code == key::lbutton && !e.down) selecting = false;
      if (text.empty()) {
        if (on_button) on_button(e);
        return;
      }
      const auto wsp = system::slot_address<window>(window_id);
      if (!wsp) return;
      if (e.code == key::lbutton && e.down) {
        const auto cp = client_pos();
        const auto cs = client_size();
        const auto pt = float2(e.pos);
        if (pt.x < cp.x || pt.y < cp.y || pt.x > cp.x + cs.x || pt.y > cp.y + cs.y) {
          if (on_button) on_button(e);
          return;
        }
        selecting = true;
        wsp->commands.end_grouping();
        const auto hit_pos = hit_caret(pt);
        if (e.double_click) {
          const auto& s = text();
          const auto len = static_cast<uint32_t>(s.size());
          if (len > 0) {
            const auto i = yw::min(hit_pos, len - 1);
            const auto cg = char_group(s[i]);
            auto first = i;
            while (first > 0 && char_group(s[first - 1]) == cg) --first;
            auto last = i + 1;
            while (last < len && char_group(s[last]) == cg) ++last;
            selection_anchor = first;
            caret_pos = last;
          }
        } else {
          if (e.shift) caret_pos = hit_pos;
          else selection_anchor = caret_pos = hit_pos;
        }
      }
      ensure_caret_visible();
      make_dirty();
      if (on_button) on_button(e);
    }

    virtual void drag_event(event::drag e) override {
      if (!enabled) return;
      if (vscroll.drag_event(e) || hscroll.drag_event(e)) {
        scroll.y = float(vscroll.value());
        scroll.x = float(hscroll.value());
        clamp_scroll();
        update_scrollbars(false);
        make_dirty();
        return;
      }
      if (!selecting || e.code != key::lbutton || text.empty()) return;
      const auto hit_pos = hit_caret(float2(e.delta));
      if (caret_pos != hit_pos) {
        caret_pos = hit_pos;
        ensure_caret_visible();
        make_dirty();
      }
    }

    virtual void wheel_event(event::wheel e) override {
      if (!enabled) return;
      if (vscroll.wheel_event(e) || hscroll.wheel_event(e)) {
        scroll.y = float(vscroll.value());
        scroll.x = float(hscroll.value());
        clamp_scroll();
        update_scrollbars(false);
        make_dirty();
        return;
      }

      const auto cp = client_pos();
      const auto cs = client_size();
      const auto pt = float2(e.pos);
      if (pt.x < cp.x || pt.y < cp.y || pt.x > cp.x + cs.x || pt.y > cp.y + cs.y) return;

      const auto line = yw::max(6.0f, text.font_size() * 1.2f);
      if (e.horizontal || e.shift) {
        scroll.x += e.delta > 0 ? -line * 2.0f : line * 2.0f;
      } else {
        scroll.y += e.delta > 0 ? -line * 2.0f : line * 2.0f;
      }
      clamp_scroll();
      update_scrollbars(false);
      make_dirty();
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
      if ((c < 0x20 && c != L'\n' && c != L'\t') || c == 0x7f) return;
      if (text().size() >= max_length || (filter && !filter(c))) return;

      const wchar_t s[2] = {c, L'\0'};
      if (caret_pos != selection_anchor) {
        if (caret_pos < selection_anchor) selection_anchor = caret_pos = replace_text(caret_pos, selection_anchor - caret_pos, s);
        else selection_anchor = caret_pos = replace_text(selection_anchor, caret_pos - selection_anchor, s);
      } else {
        if (const auto wsp = system::slot_address<window>(window_id); wsp)
          wsp->commands.push(
            [id = id, pos = caret_pos, c = c]() {
              if (const auto esp = system::slot_address<multiline>(id)) esp->insert_char(pos, c);
            },
            [id = id, pos = caret_pos]() {
              if (const auto esp = system::slot_address<multiline>(id)) esp->erase_char(pos);
            },
            true);
        insert_char(caret_pos, c);
        selection_anchor = ++caret_pos;
      }
      ensure_caret_visible();
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
          ensure_caret_visible();
          return true;
        case key::z:
          if (const auto wsp = system::slot_address<window>(window_id)) {
            if (e.shift) wsp->commands.redo();
            else wsp->commands.undo();
          }
          ensure_caret_visible();
          return true;
        case key::y:
          if (const auto wsp = system::slot_address<window>(window_id)) wsp->commands.redo();
          ensure_caret_visible();
          return true;
        case key::a:
          selection_anchor = 0;
          caret_pos = t.size();
          ensure_caret_visible();
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
        ensure_caret_visible();
        make_dirty();
        return true;
      case key::right:
        if (e.ctrl) caret_pos = next_char_group<true>(caret_pos);
        else if (caret_pos < t.size()) ++caret_pos;
        if (!e.shift) selection_anchor = caret_pos;
        ensure_caret_visible();
        make_dirty();
        return true;
      case key::up:
      case key::down:
        if (auto htr = text.hit_test(caret_pos)) {
          const auto local_x = htr->x;
          const auto line_h = yw::max(1.0f, htr->w);
          const auto local_y = htr->y + (e.code == key::up ? -line_h : line_h);
          if (auto next = text.hit_test(float2(local_x, local_y), true)) caret_pos = *next;
        }
        if (!e.shift) selection_anchor = caret_pos;
        ensure_caret_visible();
        make_dirty();
        return true;
      case key::home:
        caret_pos = 0;
        if (!e.shift) selection_anchor = caret_pos;
        ensure_caret_visible();
        make_dirty();
        return true;
      case key::end:
        caret_pos = t.size();
        if (!e.shift) selection_anchor = caret_pos;
        ensure_caret_visible();
        make_dirty();
        return true;
      case key::enter: {
        constexpr wchar_t s[2] = {L'\n', L'\0'};
        if (caret_pos == selection_anchor) selection_anchor = caret_pos = replace_text(caret_pos, 0, s);
        else if (caret_pos < selection_anchor)
          selection_anchor = caret_pos = replace_text(caret_pos, selection_anchor - caret_pos, s);
        else selection_anchor = caret_pos = replace_text(selection_anchor, caret_pos - selection_anchor, s);
        ensure_caret_visible();
        return true;
      }
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
                  if (const auto esp = system::slot_address<multiline>(id)) esp->erase_char(pos);
                },
                [id = id, pos = caret_pos, c = t[caret_pos]]() {
                  if (const auto esp = system::slot_address<multiline>(id)) esp->insert_char(pos, c);
                },
                true);
            erase_char(caret_pos);
            selection_anchor = caret_pos;
          }
        } else if (caret_pos < selection_anchor) {
          selection_anchor = caret_pos = replace_text(caret_pos, selection_anchor - caret_pos, {});
        } else {
          selection_anchor = caret_pos = replace_text(selection_anchor, caret_pos - selection_anchor, {});
        }
        ensure_caret_visible();
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
                  if (const auto esp = system::slot_address<multiline>(id)) esp->erase_char(pos);
                },
                [id = id, pos = caret_pos, c = t[caret_pos]]() {
                  if (const auto esp = system::slot_address<multiline>(id)) esp->insert_char(pos, c);
                },
                true);
            erase_char(caret_pos);
          }
        } else if (caret_pos < selection_anchor) {
          selection_anchor = caret_pos = replace_text(caret_pos, selection_anchor - caret_pos, {});
        } else {
          selection_anchor = caret_pos = replace_text(selection_anchor, caret_pos - selection_anchor, {});
        }
        ensure_caret_visible();
        return true;
      default: break;
      }
      return false;
    }

    virtual float2 ime_position() const override {
      if (auto res = text.hit_test(caret_pos)) return text_origin() + res->xy();
      return {};
    }

    virtual void ime_insert_text(std::wstring_view sv) override {
      if (caret_pos == selection_anchor) selection_anchor = caret_pos = replace_text(caret_pos, 0, sv);
      else if (caret_pos < selection_anchor)
        selection_anchor = caret_pos = replace_text(caret_pos, selection_anchor - caret_pos, sv);
      else selection_anchor = caret_pos = replace_text(selection_anchor, caret_pos - selection_anchor, sv);
      ensure_caret_visible();
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
        if (temp_size > max_length) trimmed = ministr(std::wstring_view(h->data() + Pos + erased_size, max_length - temp_size));
        auto i = h->begin() + Pos;
        if (erased_size != 0) {
          changed = true;
          i = h->erase(i, i + erased_size);
          g.commands.push_back(
            {[id = id, pos = Pos, count = Count]() {
               if (const auto esp = system::slot_address<multiline>(id)) esp->erase_text(pos, count);
             },
             [id = id, pos = Pos, str = std::move(erased)]() {
               if (const auto esp = system::slot_address<multiline>(id)) esp->insert_text(pos, str);
             }});
        }
        if (!inserted.empty()) {
          changed = true;
          if (!trimmed.empty()) {
            h->resize(h->size() - trimmed.size());
            g.commands.push_back(
              {[id = id, pos = Pos, count = trimmed.size()]() {
                 if (const auto esp = system::slot_address<multiline>(id)) esp->erase_text(pos, count);
               },
               [id = id, pos = Pos, str = std::move(trimmed)]() {
                 if (const auto esp = system::slot_address<multiline>(id)) esp->insert_text(pos, str);
               }});
          }
          const auto inserted_size = inserted.size();
          new_caret_pos = Pos + inserted_size;
          h->insert_range(i, inserted);
          g.commands.push_back(
            {[id = id, pos = Pos, str = std::move(inserted)]() {
               if (const auto esp = system::slot_address<multiline>(id)) esp->insert_text(pos, str);
             },
             [id = id, pos = Pos, count = inserted_size]() {
               if (const auto esp = system::slot_address<multiline>(id)) esp->erase_text(pos, count);
             }});
        }
      }
      if (changed) {
        if (const auto wsp = system::slot_address<window>(window_id)) wsp->commands.push(std::move(g));
      }
      sync_text_metrics();
      ensure_caret_visible();
      if (changed && on_change) on_change();
      return new_caret_pos;
    }
  };

  using control::operator bool;
  multiline() noexcept = default;
  multiline(derived_from<unknown> auto& Layout) {
    if (auto res = create_control<multiline>(Layout)) _id = *res;
  }

  const auto& border_color() const { return unsafe_get(&slot::border_color); }
  void border_color(const color& c) { safe_set(&slot::border_color, c); }

  const auto& border_width() const { return unsafe_get(&slot::border_width); }
  void border_width(float w) { safe_set(&slot::border_width, w); }

  const auto& padding() const { return unsafe_get(&slot::padding); }
  void padding(const float4& p) { safe_set_size(&slot::padding, p); }

  float scrollbar_thickness() const { return unsafe_get(&slot::scrollbar_thickness); }
  void scrollbar_thickness(float value) {
    if (auto csp = system::slot_address<slot>(_id)) {
      csp->scrollbar_thickness = yw::max(8.0f, value);
      csp->make_messy();
    }
  }

  const auto& text() const { return unsafe_get(&slot::text); }
  auto& text() { return unsafe_get_size(&slot::text); }
  template<stringable S> void text(S&& value) {
    if (auto csp = system::slot_address<slot>(_id)) {
      auto s = unicode<wchar_t>(static_cast<S&&>(value));
      if (csp->max_length != npos && s.size() > csp->max_length) s.resize(csp->max_length);
      csp->text(std::move(s));
      csp->caret_pos = yw::min(csp->caret_pos, csp->text().size());
      csp->selection_anchor = yw::min(csp->selection_anchor, csp->text().size());
      csp->sync_text_metrics();
    }
  }

  const auto& placeholder() const { return unsafe_get(&slot::placeholder); }
  auto& placeholder() { return unsafe_get_size(&slot::placeholder); }
  template<stringable S> void placeholder(S&& value) {
    if (auto csp = system::slot_address<slot>(_id)) {
      csp->placeholder(static_cast<S&&>(value));
      csp->make_dirty();
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
    if (const auto csp = system::slot_address<slot>(_id)) {
      csp->caret_pos = csp->selection_anchor = yw::min(value.x, csp->text().size());
      csp->ensure_caret_visible();
      csp->make_dirty();
    }
  }

  float2 scroll() const { return unsafe_get(&slot::scroll); }
  void scroll(float2 value) {
    if (auto csp = system::slot_address<slot>(_id)) {
      csp->scroll = value;
      csp->clamp_scroll();
      csp->update_scrollbars(false);
      csp->make_dirty();
    }
  }

  size_t max_length() const { return unsafe_get(&slot::max_length); }
  void max_length(std::optional<size_t> value) {
    if (const auto csp = system::slot_address<slot>(_id)) {
      const auto limit = value.value_or(npos);
      const auto text_size = csp->text().size();
      csp->max_length = limit;
      if (text_size > limit) csp->replace_text(limit, text_size - limit, {});
      csp->sync_text_metrics();
    }
  }

  const auto& on_button() const { return unsafe_get(&slot::on_button); }
  void on_button(function<void, event::button> f) { safe_set(&slot::on_button, std::move(f)); }

  const auto& on_change() const { return unsafe_get(&slot::on_change); }
  void on_change(function<void> f) { safe_set(&slot::on_change, std::move(f)); }

  const auto& on_focus() const { return unsafe_get(&slot::on_focus); }
  void on_focus(function<void, bool> f) { safe_set(&slot::on_focus, std::move(f)); }

  const auto& filter() const { return unsafe_get(&slot::filter); }
  void filter(function<bool, wchar_t> f) { safe_set(&slot::filter, std::move(f)); }
};
} // namespace yw::ui
