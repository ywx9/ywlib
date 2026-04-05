#pragma once
#include "ywx/background.h"
#include "ywx/clipboard.h"
#include "ywx/text_layout.h"
#include "ywx/ui_control.h"
#include "ywx/window.h"

namespace yw::ui {

class edit : public control {
public:
  class slot : public control::slot {
    static text_layout make_layout(const yw::text& value) {
      auto result = assume(text_layout::create(value(), value));
      assume(result.text_alignment(text_alignment::left));
      assume(result.paragraph_alignment(paragraph_alignment::top));
      return result;
    }

  public:
    yw::background background = colors::white;
    color border_color = colors::black;
    float border_width = 1.0f;
    float4 padding{8.0f, 6.0f, 8.0f, 6.0f};

    yw::text text = assume(yw::text::create(L""));
    yw::text_layout layout = make_layout(text);
    color text_color = colors::black;
    color caret_color = colors::black;
    color selection_color = color(0.2f, 0.45f, 1.0f, 0.35f);

    uint32_t caret = 0;
    uint32_t selection_anchor = 0;
    float scroll_x = 0.0f;
    bool focused = false;
    bool selecting = false;

    function<void, event::button> on_button;
    function<void, std::wstring_view> on_change;
    function<void, bool> on_focus;
    function<void, event::key> on_keydown;
    function<void, event::key> on_keyup;
    function<void, event::move> on_move;
    function<void, event::wheel> on_wheel;

    slot() { min_size = {120.0f, 32.0f}; }

    float2 inner_size() const noexcept {
      return vapply_r<float2>(yw::max, float2(), size - padding.xy() - padding.zw());
    }

    float2 inner_pos() const noexcept { return pos + padding.xy(); }

    float2 text_origin() const noexcept {
      const auto content_pos = inner_pos();
      const auto content_size = inner_size();
      const auto text_h = yw::max(text.size().y, text.font_size());
      return float2(content_pos.x - scroll_x, content_pos.y + (content_size.y - text_h) * 0.5f);
    }

    uint32_t text_length() const noexcept { return static_cast<uint32_t>(text().size()); }

    uint32_t clamp_caret(size_t value) const noexcept {
      return static_cast<uint32_t>(yw::min(value, static_cast<size_t>(text_length())));
    }

    void refresh_layout() { layout = make_layout(text); }

    bool has_selection() const noexcept { return caret != selection_anchor; }

    uint2 selection_range() const noexcept {
      return {yw::min(caret, selection_anchor), yw::max(caret, selection_anchor)};
    }

    void clear_selection() noexcept { selection_anchor = caret; }

    float4 caret_rect() const {
      if (text_length() == 0) {
        const auto th = yw::max(text.size().y, text.font_size());
        return {0.0f, 0.0f, 1.0f, yw::max(th, 1.0f)};
      }
      if (auto res = layout.hit_test(uint1(caret), false)) return *res;
      const auto tsz = text.size();
      return {tsz.x, 0.0f, 1.0f, yw::max(tsz.y, 1.0f)};
    }

    void clamp_scroll() noexcept {
      const auto iw = inner_size().x;
      const auto max_scroll = yw::max(0.0f, text.size().x - iw);
      scroll_x = std::clamp(scroll_x, 0.0f, max_scroll);
    }

    void ensure_caret_visible() {
      const auto iw = inner_size().x;
      if (iw <= 0.0f) {
        scroll_x = 0.0f;
        if (focused) update_ime_position();
        return;
      }
      const auto rect = caret_rect();
      if (rect.x < scroll_x) scroll_x = rect.x;
      else if (rect.x + rect.z > scroll_x + iw) scroll_x = rect.x + rect.z - iw;
      clamp_scroll();
      if (focused) update_ime_position();
    }

    void update_ime_position() const {
      if (!focused || !enabled) return;
      const auto wsp = system::slot_address<window>(window_id);
      if (!wsp || !wsp->hwnd) return;
      const auto text_pos = text_origin();
      const auto rect = caret_rect();
      const LONG x = LONG(text_pos.x + rect.x);
      const LONG top = LONG(text_pos.y + rect.y);
      const LONG bottom = LONG(text_pos.y + rect.y + yw::max(rect.w, 1.0f));

      HIMC imc = ::ImmGetContext(wsp->hwnd);
      if (!imc) return;

      COMPOSITIONFORM comp{};
      comp.dwStyle = CFS_POINT;
      comp.ptCurrentPos = POINT{x, bottom};
      ::ImmSetCompositionWindow(imc, &comp);

      CANDIDATEFORM cand{};
      cand.dwIndex = 0;
      cand.dwStyle = CFS_CANDIDATEPOS;
      cand.ptCurrentPos = POINT{x, bottom};

      print("IME position updated: ({}, {})", x, bottom);

      // CANDIDATEFORM exclude{};
      // exclude.dwIndex = 0;
      // exclude.dwStyle = CFS_EXCLUDE;
      // exclude.ptCurrentPos = POINT{x, bottom};
      // exclude.rcArea = RECT{x, top, x + 1, bottom};
      // ::ImmSetCandidateWindow(imc, &exclude);

      ::ImmReleaseContext(wsp->hwnd, imc);
    }

    uint32_t hit_caret(float2 point) const {
      if (text_length() == 0) return 0;

      const auto text_h = yw::max(text.size().y, text.font_size());
      const auto text_pos = text_origin();
      const auto text_end_x = text_pos.x + text.size().x;
      const auto text_end_y = text_pos.y + text_h;

      if (point.x < text_pos.x || point.y < text_pos.y) return 0;
      if (point.x > text_end_x || point.y > text_end_y) return text_length();

      auto local = point - text_pos;
      local.x = yw::max(0.0f, local.x);
      local.y = yw::max(0.0f, local.y);
      if (auto res = layout.hit_test(local)) return clamp_caret(*res);
      return text_length();
    }

    void set_caret(size_t value) {
      caret = clamp_caret(value);
      clear_selection();
      ensure_caret_visible();
      make_dirty();
    }

    void move_caret(size_t value, bool keep_selection) {
      caret = clamp_caret(value);
      if (!keep_selection) clear_selection();
      ensure_caret_visible();
      make_dirty();
    }

    void commit_text(std::wstring value, size_t new_caret) {
      text = std::move(value);
      refresh_layout();
      caret = clamp_caret(new_caret);
      clear_selection();
      ensure_caret_visible();
      make_messy();
      if (on_change) on_change(text());
    }

    void erase_selection() {
      if (!has_selection()) return;
      const auto range = selection_range();
      auto value = text();
      value.erase(range.x, range.y - range.x);
      commit_text(std::move(value), range.x);
    }

    std::wstring selected_text() const {
      if (!has_selection()) return {};
      const auto range = selection_range();
      return text().substr(range.x, range.y - range.x);
    }

    void copy_selection() const {
      if (!has_selection()) return;
      const auto selected = selected_text();
      if (!selected.empty()) clipboard.text(selected);
    }

    void cut_selection() {
      if (!has_selection()) return;
      copy_selection();
      erase_selection();
    }

    void paste_from_clipboard() {
      if (auto value = clipboard.text()) insert_text(*value);
    }

    void erase_at(size_t first, size_t last) {
      if (last <= first) return;
      auto value = text();
      value.erase(first, last - first);
      commit_text(std::move(value), first);
    }

    void insert_text(std::wstring_view value) {
      if (value.empty()) return;
      if (has_selection()) erase_selection();
      auto current = text();
      current.insert(caret, value);
      commit_text(std::move(current), caret + value.size());
    }

    virtual float2 calculate_size() const noexcept override {
      const auto inner = vapply_r<float2>(yw::max, text.size(), float2(24.0f, text.font_size()));
      return vapply_r<float2>(yw::max, float2(), min_size, inner + padding.xy() + padding.zw(), size * constrained);
    }

    virtual void update_size() noexcept override {
      refresh_layout();
      min_size = vapply_r<float2>(yw::max, min_size, float2());
      const auto inner = vapply_r<float2>(yw::max, text.size(), float2(24.0f, text.font_size()));
      size = vapply_r<float2>(yw::max, min_size, inner + padding.xy() + padding.zw(), size * constrained);
      clamp_scroll();
      ensure_caret_visible();
    }

    virtual void draw() const override {
      if (!visible) return;
      draw_background(pos, size, background);
      brush.color(border_color);
      draw_round_rectangle(pos, size, radius, border_width);

      const auto content_pos = inner_pos();
      const auto content_size = inner_size();
      const auto text_pos = text_origin();

      const D2D1_RECT_F clip = D2D1::RectF(content_pos.x, content_pos.y, content_pos.x + content_size.x, content_pos.y + content_size.y);
      d2d.context()->PushAxisAlignedClip(clip, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
      if (has_selection()) {
        const auto range = selection_range();
        if (auto rects = layout.hit_test_range(range, text_pos)) {
          brush.color(selection_color);
          for (const auto& rect : *rects) {
            fill_rectangle(rect.xy(), rect.zw());
          }
        }
      }
      brush.color(text_color);
      draw_text_layout(text_pos, layout, false);
      if (focused && !has_selection()) {
        const auto rect = caret_rect();
        const auto x = text_pos.x + rect.x;
        const auto h = yw::max(rect.w, yw::max(text.size().y, text.font_size()));
        const auto y = content_pos.y + (content_size.y - h) * 0.5f;
        brush.color(caret_color);
        draw_line({x, y}, {x, y + h}, 1.0f);
      }
      d2d.context()->PopAxisAlignedClip();
    }

    virtual void button_event(event::button e) override {
      if (!enabled) return;
      if (e.code == key::lbutton) {
        if (e.down) {
          const auto pos = hit_caret(float2(e.pos));
          if (!e.shift) selection_anchor = pos;
          move_caret(pos, e.shift);
          selecting = true;
        } else selecting = false;
      }
      if (on_button) on_button(e);
    }

    virtual bool focus_event(bool value) override {
      focused = value && enabled;
      if (focused) ensure_caret_visible();
      else {
        selecting = false;
        clear_selection();
      }
      make_dirty();
      if (enabled && on_focus) on_focus(focused);
      return enabled;
    }

    virtual void char_event(wchar_t c) override {
      if (!enabled) return;
      if (c < 0x20 || c == 0x7f) return;
      const wchar_t value[2] = {c, L'\0'};
      insert_text(std::wstring_view(value, 1));
    }

    virtual void key_event(event::key e) override {
      if (!enabled) return;
      if (e.down) {
        if (e.ctrl) {
          switch (e.code) {
          case key::c:
            copy_selection();
            if (e.first && on_keydown) on_keydown(e);
            return;
          case key::x:
            cut_selection();
            if (e.first && on_keydown) on_keydown(e);
            return;
          case key::v:
            paste_from_clipboard();
            if (e.first && on_keydown) on_keydown(e);
            return;
          default:
            break;
          }
        }
        if (e.ctrl && e.code == key::a) {
          selection_anchor = 0;
          move_caret(text_length(), true);
          if (e.first && on_keydown) on_keydown(e);
          return;
        }
        switch (e.code) {
        case key::left:
          if (caret > 0) move_caret(caret - 1, e.shift);
          break;
        case key::right:
          if (caret < text_length()) move_caret(caret + 1, e.shift);
          break;
        case key::home:
          move_caret(0, e.shift);
          break;
        case key::end:
          move_caret(text_length(), e.shift);
          break;
        case key::backspace:
          if (has_selection()) erase_selection();
          else if (caret > 0) erase_at(caret - 1, caret);
          break;
        case key::delete_:
          if (has_selection()) erase_selection();
          else if (caret < text_length()) erase_at(caret, caret + 1);
          break;
        default:
          break;
        }
        if (e.first && on_keydown) on_keydown(e);
      } else if (on_keyup) on_keyup(e);
    }

    virtual void move_event(event::move e) override {
      if (enabled && selecting) move_caret(hit_caret(float2(e.pos)), true);
      if (enabled && on_move) on_move(e);
    }

    virtual void wheel_event(event::wheel e) override {
      if (enabled && on_wheel) on_wheel(e);
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
  void text(yw::text value) {
    if (auto csp = system::slot_address<slot>(_id)) {
      csp->text = std::move(value);
      csp->refresh_layout();
      csp->caret = csp->clamp_caret(csp->caret);
      csp->ensure_caret_visible();
      csp->make_messy();
    }
  }
  template<stringable S> void text(S&& value) {
    if (auto csp = system::slot_address<slot>(_id)) {
      csp->text = static_cast<S&&>(value);
      csp->refresh_layout();
      csp->caret = csp->clamp_caret(csp->caret);
      csp->ensure_caret_visible();
      csp->make_messy();
    }
  }

  const auto& text_color() const { return unsafe_get(&slot::text_color); }
  void text_color(const color& c) { safe_set(&slot::text_color, c); }

  const auto& caret_color() const { return unsafe_get(&slot::caret_color); }
  void caret_color(const color& c) { safe_set(&slot::caret_color, c); }

  size_t caret() const { return unsafe_get(&slot::caret); }
  void caret(size_t value) {
    if (auto csp = system::slot_address<slot>(_id)) csp->set_caret(value);
  }

  const auto& on_button() const { return unsafe_get(&slot::on_button); }
  void on_button(function<void, event::button> f) { safe_set(&slot::on_button, std::move(f)); }

  const auto& on_change() const { return unsafe_get(&slot::on_change); }
  void on_change(function<void, std::wstring_view> f) { safe_set(&slot::on_change, std::move(f)); }

  const auto& on_focus() const { return unsafe_get(&slot::on_focus); }
  void on_focus(function<void, bool> f) { safe_set(&slot::on_focus, std::move(f)); }

  const auto& on_keydown() const { return unsafe_get(&slot::on_keydown); }
  void on_keydown(function<void, event::key> f) { safe_set(&slot::on_keydown, std::move(f)); }

  const auto& on_keyup() const { return unsafe_get(&slot::on_keyup); }
  void on_keyup(function<void, event::key> f) { safe_set(&slot::on_keyup, std::move(f)); }

  const auto& on_move() const { return unsafe_get(&slot::on_move); }
  void on_move(function<void, event::move> f) { safe_set(&slot::on_move, std::move(f)); }

  const auto& on_wheel() const { return unsafe_get(&slot::on_wheel); }
  void on_wheel(function<void, event::wheel> f) { safe_set(&slot::on_wheel, std::move(f)); }
};
} // namespace yw::ui
