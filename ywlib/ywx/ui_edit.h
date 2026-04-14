#pragma once
#include "ywx/background.h"
#include "ywx/clipboard.h"
#include "ywx/ime.h"
#include "ywx/text_layout.h"
#include "ywx/ui_control.h"
#include "ywx/window.h"

namespace yw::ui {

class edit : public control {
public:
  class slot : public control::slot {
    // 文字グループ: 0=alnum, 1=ascii記号, 2=その他
    static int char_group(wchar_t c) noexcept { return !is_alnum(c) + !is_ascii(c); }

    float2 preferred_size() const noexcept {
      const auto tsz = vapply_r<float2>(yw::max, float2(), text.size(), placeholder.size());
      const auto inner = tsz + padding.xy() + padding.zw();
      return vapply_r<float2>(yw::max, min_size, inner, size * constrained);
    }

    float2 text_origin() const noexcept {
      const auto extra = size.y - padding.y - padding.w - text.size().y;
      return pos + padding.xy() + float2(-scroll, extra * 0.5f);
    }

    void insert_text(std::wstring_view Str) {
      if (Str.empty()) return;
      std::wstring s;
      const auto& o = text();
      s.reserve(o.size() + Str.size());
      std::memcpy(s.data(), o.data(), caret_pos * sizeof(wchar_t));
      std::memcpy(s.data() + caret_pos, Str.data(), Str.size() * sizeof(wchar_t));
      const auto old_caret_pos = caret_pos;
      caret_pos += Str.size();
      std::memcpy(s.data() + caret_pos, o.data() + old_caret_pos, (o.size() - old_caret_pos) * sizeof(wchar_t));
      // フィルター適用の必要あり
      if (s.size() > max_length) s.resize(max_length);
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

    function<void, event::button> on_button;
    function<void, std::wstring_view> on_change;
    function<void, bool> on_focus;
    function<void, event::key> on_keydown;
    function<void, event::key> on_keyup;
    function<void, event::move> on_move;
    function<void, event::wheel> on_wheel;
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
        if (wsp->focused_control == id) {
          brush.color(caret_color);
          draw_line(text_pos, text_pos + float2(0, text.size().y));
        } else if (!placeholder.empty()) {
          brush.color(placeholder_color);
          draw_text(text_pos, placeholder);
        }
        return;
      }
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

    virtual void button_event(event::button e) override {
      if (!enabled) return;
      if (text.empty()) {
        if (on_button) on_button(e);
        return;
      }
      const auto wsp = system::slot_address<window>(window_id);
      if (!wsp) return;
      if (e.code == key::lbutton) {
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
          }
          else selection_anchor = caret_pos = hit_pos;
        }
      }
      make_dirty();
    }

    virtual bool focus_event(bool focused) override {
      if (const auto wsp = system::slot_address<window>(window_id); wsp && !focused) wsp->commands.end_grouping();
      make_dirty();
      if (enabled && on_focus) on_focus(focused);
      return enabled;
    }

    virtual void char_event(wchar_t c) override {
      if (!enabled) return;
      if (c < 0x20 || c == 0x7f || (filter && !filter(c))) return;
      begin_typing_group();
      const wchar_t value[2] = {c, L'\0'};
      insert_text(std::wstring_view(value, 1));
    }
  };
};
} // namespace yw::ui
