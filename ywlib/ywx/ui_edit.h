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

    struct recorded_char_edit {
      uint32_t pos{};
      wchar_t ch{};
      bool insert = false;
    };

    bool _typing_group_open = false;
    std::chrono::steady_clock::time_point _last_typed_at{};

    void apply_text(std::wstring value, size_t new_caret) {
      text(std::move(value));
      caret = clamp_caret(new_caret);
      selection_anchor = caret;
      ensure_caret_visible();
      make_messy();
      if (on_change) on_change(text());
    }

    void apply_insert_char(uint32_t pos, wchar_t ch) {
      auto value = text();
      pos = static_cast<uint32_t>(yw::min(pos, value.size()));
      value.insert(value.begin() + pos, ch);
      apply_text(std::move(value), pos + 1);
    }

    void apply_erase_char(uint32_t pos) {
      auto value = text();
      if (pos >= value.size()) return;
      value.erase(pos, 1);
      apply_text(std::move(value), pos);
    }

    void push_char_edit(const recorded_char_edit& op) {
      const auto wsp = system::slot_address<window>(window_id);
      if (!wsp || wsp->commands.is_replaying()) return;
      const auto my_id = id;
      const auto pos = op.pos;
      const auto ch = op.ch;
      if (op.insert) {
        wsp->commands.push(
          [my_id, pos, ch]() {
            if (const auto esp = system::slot_address<edit>(my_id)) esp->apply_insert_char(pos, ch);
          },
          [my_id, pos]() {
            if (const auto esp = system::slot_address<edit>(my_id)) esp->apply_erase_char(pos);
          });
      } else {
        wsp->commands.push(
          [my_id, pos]() {
            if (const auto esp = system::slot_address<edit>(my_id)) esp->apply_erase_char(pos);
          },
          [my_id, pos, ch]() {
            if (const auto esp = system::slot_address<edit>(my_id)) esp->apply_insert_char(pos, ch);
          });
      }
    }

    void begin_typing_group() {
      const auto wsp = system::slot_address<window>(window_id);
      if (!wsp || wsp->commands.is_replaying()) return;

      const auto now = std::chrono::steady_clock::now();
      constexpr auto typing_gap = std::chrono::milliseconds(800);

      if (_typing_group_open) {
        if (now - _last_typed_at > typing_gap) {
          wsp->commands.end_group();
          wsp->commands.begin_group(L"edit");
        }
      } else {
        wsp->commands.begin_group(L"edit");
        _typing_group_open = true;
      }
      _last_typed_at = now;
    }

    void end_typing_group() {
      if (!_typing_group_open) return;
      if (const auto wsp = system::slot_address<window>(window_id)) wsp->commands.end_group();
      _typing_group_open = false;
    }

  public:
    yw::background background = colors::white;
    color border_color = colors::black;
    float border_width = 1.0f;
    float4 padding{8.0f, 6.0f, 8.0f, 6.0f};

    yw::text text = assume(yw::text::create(L""));
    color text_color = colors::black;
    color caret_color = colors::black;
    color selection_color = color(0.2f, 0.45f, 1.0f, 0.35f);
    size_t max_length = npos;

    yw::text placeholder = assume(yw::text::create(L""));
    color placeholder_color = color(0.5f, 0.5f, 0.5f, 1.0f);

    uint32_t caret = 0;
    uint32_t selection_anchor = 0;
    float scroll_x = 0.0f;
    bool focused = false;
    bool selecting = false;
    mutable bool ime_position_dirty = false;
    mutable bool has_last_ime_position = false;
    mutable LONG last_ime_x = 0;
    mutable LONG last_ime_y = 0;
    std::chrono::steady_clock::time_point last_lbutton_down{};
    short2 last_lbutton_pos{};
    bool has_last_lbutton_down = false;

    function<void, event::button> on_button;
    function<void, std::wstring_view> on_change;
    function<void, bool> on_focus;
    function<void, event::key> on_keydown;
    function<void, event::key> on_keyup;
    function<void, event::move> on_move;
    function<void, event::wheel> on_wheel;
    function<bool, wchar_t> filter;

    float2 inner_size() const noexcept {
      return vapply_r<float2>(yw::max, float2(), size - padding.xy() - padding.zw());
    }

    float2 text_origin() const noexcept {
      const auto content_pos = pos + padding.xy();
      const auto content_size = inner_size();
      const auto text_h = yw::max(text.size().y, text.font_size());
      return float2(content_pos.x - scroll_x, content_pos.y + (content_size.y - text_h) * 0.5f);
    }

    uint32_t text_length() const noexcept { return static_cast<uint32_t>(text().size()); }

    uint32_t clamp_caret(size_t value) const noexcept {
      return static_cast<uint32_t>(yw::min(value, static_cast<size_t>(text_length())));
    }

    uint2 selection_range() const noexcept {
      return {yw::min(caret, selection_anchor), yw::max(caret, selection_anchor)};
    }

    float4 caret_rect() const {
      if (text_length() == 0) {
        const auto th = yw::max(text.size().y, text.font_size());
        return {0.0f, 0.0f, 1.0f, yw::max(th, 1.0f)};
      }
      if (auto res = text.hit_test(uint1(caret))) return *res;
      const auto tsz = text.size();
      return {tsz.x, 0.0f, 1.0f, yw::max(tsz.y, 1.0f)};
    }

    void clamp_scroll() noexcept {
      const auto iw = inner_size().x;
      const auto max_scroll = yw::max(0.0f, text.size().x - iw);
      scroll_x = std::clamp(scroll_x, 0.0f, max_scroll);
    }

    void request_ime_position_update() noexcept {
      if (focused && enabled) ime_position_dirty = true;
    }

    void ensure_caret_visible() {
      const auto iw = inner_size().x;
      if (iw <= 0.0f) {
        scroll_x = 0.0f;
        request_ime_position_update();
        return;
      }
      const auto rect = caret_rect();
      constexpr float caret_w = 1.0f;
      if (rect.x < scroll_x) scroll_x = rect.x;
      else if (rect.x + caret_w > scroll_x + iw) scroll_x = rect.x + caret_w - iw;
      clamp_scroll();
      request_ime_position_update();
    }

    void update_ime_position() const {
      if (!focused || !enabled) return;
      const auto wsp = system::slot_address<window>(window_id);
      if (!wsp || !wsp->hwnd) return;
      const auto text_pos = text_origin();
      const auto rect = caret_rect();
      const LONG x = LONG(text_pos.x + rect.x);
      const LONG bottom = LONG(text_pos.y + rect.y + yw::max(rect.w, 1.0f));

      if (has_last_ime_position && last_ime_x == x && last_ime_y == bottom) {
        ime_position_dirty = false;
        return;
      }

      HIMC imc = ::ImmGetContext(wsp->hwnd);
      if (!imc) return;

      COMPOSITIONFORM comp{};
      comp.dwStyle = CFS_POINT;
      comp.ptCurrentPos = POINT{x, bottom};
      ::ImmSetCompositionWindow(imc, &comp);

      print("IME position updated: ({}, {})", x, bottom);

      ::ImmReleaseContext(wsp->hwnd, imc);
      last_ime_x = x;
      last_ime_y = bottom;
      has_last_ime_position = true;
      ime_position_dirty = false;
    }

    void flush_ime_position_if_needed() const {
      if (!ime_position_dirty) return;
      update_ime_position();
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
      if (auto res = text.hit_test(local)) return clamp_caret(*res);
      return text_length();
    }

    void set_caret(size_t value) {
      caret = clamp_caret(value);
      selection_anchor = caret;
      ensure_caret_visible();
      make_dirty();
    }

    void move_caret(size_t value, bool keep_selection) {
      caret = clamp_caret(value);
      if (!keep_selection) selection_anchor = caret;
      ensure_caret_visible();
      make_dirty();
    }

    void commit_text(std::wstring value, size_t new_caret) {
      apply_text(std::move(value), new_caret);
    }

    void erase_range(uint32_t first, uint32_t last, bool allow_local_group = true) {
      if (last <= first) return;
      auto value = text();
      if (first >= value.size()) return;
      last = static_cast<uint32_t>(yw::min(last, value.size()));
      if (last <= first) return;

      const auto wsp = system::slot_address<window>(window_id);
      const bool record = wsp && !wsp->commands.is_replaying();
      const bool open_group = allow_local_group && record && !_typing_group_open && (last - first > 1);
      if (open_group) wsp->commands.begin_group(L"edit");

      auto removed = value.substr(first, last - first);
      for (size_t i = 0; i < removed.size(); ++i) {
        const wchar_t ch = removed[i];
        apply_erase_char(first);
        if (record) push_char_edit({first, ch, false});
      }

      if (open_group) wsp->commands.end_group();
    }

    void erase_selection() {
      if (caret == selection_anchor) return;
      const auto range = selection_range();
      erase_range(range.x, range.y);
    }

    std::wstring selected_text() const {
      if (caret == selection_anchor) return {};
      const auto range = selection_range();
      return text().substr(range.x, range.y - range.x);
    }

    void copy_selection() const {
      if (caret == selection_anchor) return;
      const auto selected = selected_text();
      if (!selected.empty()) clipboard.text(selected);
    }

    void cut_selection() {
      if (caret == selection_anchor) return;
      copy_selection();
      erase_selection();
    }

    void paste_from_clipboard() {
      if (auto value = clipboard.text()) insert_text(*value);
    }

    void erase_at(size_t first, size_t last) {
      erase_range(static_cast<uint32_t>(first), static_cast<uint32_t>(last));
    }

    void insert_text(std::wstring_view value) {
      if (value.empty()) return;
      auto current = text();
      const auto range = selection_range();
      const auto erase_len = (caret == selection_anchor) ? 0u : (range.y - range.x);
      const auto base_len = current.size() - erase_len;
      const auto insert_pos = (caret == selection_anchor) ? caret : range.x;

      size_t accepted = value.size();
      if (max_length != npos) {
        const auto remaining = max_length > base_len ? (max_length - base_len) : size_t(0);
        accepted = yw::min(accepted, remaining);
      }
      if (accepted == 0) return;

      const auto wsp = system::slot_address<window>(window_id);
      const bool record = wsp && !wsp->commands.is_replaying();
      const bool open_group = record && !_typing_group_open && (erase_len > 0 || accepted > 1);
      if (open_group) wsp->commands.begin_group(L"edit");

      if (caret != selection_anchor) erase_range(range.x, range.y, false);

      for (size_t i = 0; i < accepted; ++i) {
        const auto p = static_cast<uint32_t>(insert_pos + i);
        const auto ch = value[i];
        apply_insert_char(p, ch);
        if (record) push_char_edit({p, ch, true});
      }

      if (open_group) wsp->commands.end_group();
    }

    virtual float2 calculate_size() const noexcept override {
      const auto inner_h = yw::max(text.size().y, placeholder.size().y, text.font_size(), placeholder.font_size());
      const float2 prefer_size{0.0f, inner_h + padding.y + padding.w};
      return vapply_r<float2>(yw::max, float2(), min_size, prefer_size, size * constrained);
    }

    virtual void update_size() noexcept override {
      min_size = vapply_r<float2>(yw::max, min_size, float2());
      const auto inner_h = yw::max(text.size().y, placeholder.size().y, text.font_size(), placeholder.font_size());
      const float2 prefer_size{0.0f, inner_h + padding.y + padding.w};
      size = vapply_r<float2>(yw::max, min_size, prefer_size, size * constrained);
    }

    virtual void update_layout(float2 Pos, float2 Area) noexcept override {
      control::slot::update_layout(Pos, Area);
      clamp_scroll();
      ensure_caret_visible();
    }

    virtual void draw() const override {
      if (!visible) return;
      draw_background(pos, size, background);
      brush.color(border_color);
      draw_round_rectangle(pos, size, radius, border_width);

      const auto content_pos = pos + padding.xy();
      const auto content_size = inner_size();
      const auto text_pos = text_origin();

      const D2D1_RECT_F clip =
        D2D1::RectF(content_pos.x, content_pos.y, content_pos.x + content_size.x, content_pos.y + content_size.y);
      d2d.context()->PushAxisAlignedClip(clip, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
      if (caret != selection_anchor) {
        const auto range = selection_range();
        if (auto rects = text.hit_test_range(range, text_pos)) {
          brush.color(selection_color);
          for (const auto& rect : *rects) { fill_rectangle(rect.xy(), rect.zw()); }
        }
      }
      if (text_length() == 0 && !focused && !placeholder.empty()) {
        brush.color(placeholder_color);
        draw_text(text_pos, placeholder);
      } else if (!text.empty()) {
        brush.color(text_color);
        draw_text(text_pos, text);
      }
      if (focused && caret == selection_anchor) {
        const auto rect = caret_rect();
        const auto x = text_pos.x + rect.x;
        const auto h = yw::max(rect.w, yw::max(text.size().y, text.font_size()));
        const auto y = content_pos.y + (content_size.y - h) * 0.5f;
        brush.color(caret_color);
        draw_line({x, y}, {x, y + h}, 1.0f);
      }
      d2d.context()->PopAxisAlignedClip();
      flush_ime_position_if_needed();
    }

    virtual void button_event(event::button e) override {
      if (!enabled) return;
      if (e.code == key::lbutton) {
        if (e.down) {
          end_typing_group();
          const auto pos = hit_caret(float2(e.pos));
          const auto now = std::chrono::steady_clock::now();
          bool double_click = false;
          if (has_last_lbutton_down) {
            const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_lbutton_down).count();
            int dx = int(e.pos.x) - int(last_lbutton_pos.x);
            int dy = int(e.pos.y) - int(last_lbutton_pos.y);
            if (dx < 0) dx = -dx;
            if (dy < 0) dy = -dy;
            double_click = elapsed >= 0 && elapsed <= (long long)(::GetDoubleClickTime()) && dx <= 4 && dy <= 4;
          }
          last_lbutton_down = now;
          last_lbutton_pos = e.pos;
          has_last_lbutton_down = true;

          if (double_click && text_length() > 0) {
            const auto& s = text();
            const auto len = text_length();
            const auto i = yw::min(pos, len - 1);
            const auto grp = char_group(s[i]);
            auto first = i;
            while (first > 0 && char_group(s[first - 1]) == grp) --first;
            auto last = i + 1;
            while (last < len && char_group(s[last]) == grp) ++last;
            if (!e.shift) selection_anchor = first;
            move_caret(last, true);
          } else {
            if (!e.shift) selection_anchor = pos;
            move_caret(pos, e.shift);
          }
          selecting = true;
        } else selecting = false;
      }
      if (on_button) on_button(e);
    }

    virtual bool focus_event(bool value) override {
      focused = value && enabled;
      if (focused) ensure_caret_visible();
      else {
        end_typing_group();
        selecting = false;
        selection_anchor = caret;
        ime_position_dirty = false;
        has_last_ime_position = false;
        has_last_lbutton_down = false;
      }
      make_dirty();
      if (enabled && on_focus) on_focus(focused);
      return enabled;
    }

    virtual void char_event(wchar_t c) override {
      if (!enabled) return;
      if (c < 0x20 || c == 0x7f) return;
      if (filter && !filter(c)) return;
      begin_typing_group();
      const wchar_t value[2] = {c, L'\0'};
      insert_text(std::wstring_view(value, 1));
    }

    // 文字グループ: 0=alnum, 1=ascii記号, 2=その他(全角等)
    static int char_group(wchar_t c) noexcept {
      if (is_alnum(c)) return 0;
      if (is_ascii(c)) return 1;
      return 2;
    }

    uint32_t word_left(uint32_t pos) const noexcept {
      const auto& s = text();
      if (pos == 0) return 0;
      --pos;
      const int grp = char_group(s[pos]);
      while (pos > 0 && char_group(s[pos - 1]) == grp) --pos;
      return pos;
    }

    uint32_t word_right(uint32_t pos) const noexcept {
      const auto& s = text();
      const auto len = static_cast<uint32_t>(s.size());
      if (pos >= len) return len;
      const int grp = char_group(s[pos]);
      while (pos < len && char_group(s[pos]) == grp) ++pos;
      return pos;
    }

    virtual bool key_event(event::key e) override {
      if (!enabled) return false;
      if (e.down) {
        if (e.ctrl) {
          switch (e.code) {
          case key::c:
            end_typing_group();
            copy_selection();
            if (e.first && on_keydown) on_keydown(e);
            return true;
          case key::x:
            end_typing_group();
            cut_selection();
            if (e.first && on_keydown) on_keydown(e);
            return true;
          case key::v:
            end_typing_group();
            paste_from_clipboard();
            if (e.first && on_keydown) on_keydown(e);
            return true;
          case key::z:
            end_typing_group();
            if (const auto wsp = system::slot_address<window>(window_id)) {
              if (e.shift) wsp->commands.redo();
              else wsp->commands.undo();
            }
            if (e.first && on_keydown) on_keydown(e);
            return true;
          case key::y:
            end_typing_group();
            if (const auto wsp = system::slot_address<window>(window_id))
              wsp->commands.redo();
            if (e.first && on_keydown) on_keydown(e);
            return true;
          default: break;
          }
        }
        if (e.ctrl && e.code == key::a) {
          end_typing_group();
          selection_anchor = 0;
          move_caret(text_length(), true);
          if (e.first && on_keydown) on_keydown(e);
          return true;
        }
        switch (e.code) {
        case key::left:
          end_typing_group();
          if (e.ctrl) move_caret(word_left(caret), e.shift);
          else if (caret > 0) move_caret(caret - 1, e.shift);
          return true;
        case key::right:
          end_typing_group();
          if (e.ctrl) move_caret(word_right(caret), e.shift);
          else if (caret < text_length()) move_caret(caret + 1, e.shift);
          return true;
        case key::home:
          end_typing_group();
          move_caret(0, e.shift);
          return true;
        case key::end:
          end_typing_group();
          move_caret(text_length(), e.shift);
          return true;
        case key::backspace:
          end_typing_group();
          if (caret != selection_anchor) erase_selection();
          else if (e.ctrl && caret > 0) erase_at(word_left(caret), caret);
          else if (caret > 0) erase_at(caret - 1, caret);
          return true;
        case key::delete_:
          end_typing_group();
          if (caret != selection_anchor) erase_selection();
          else if (e.ctrl && caret < text_length()) erase_at(caret, word_right(caret));
          else if (caret < text_length()) erase_at(caret, caret + 1);
          return true;
        default: break;
        }
        if (e.first && on_keydown) on_keydown(e);
          return false;
      } else if (on_keyup) {
        on_keyup(e);
          return false;
      }
      return false;
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
  template<stringable S> void text(S&& value) {
    if (auto csp = system::slot_address<slot>(_id)) {
      auto s = unicode<wchar_t>(static_cast<S&&>(value));
      if (csp->max_length != npos && s.size() > csp->max_length) s.resize(csp->max_length);
      csp->text(std::move(s));
      csp->caret = csp->clamp_caret(csp->caret);
      csp->ensure_caret_visible();
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

  size_t caret() const { return unsafe_get(&slot::caret); }
  void caret(size_t value) {
    if (auto csp = system::slot_address<slot>(_id)) csp->set_caret(value);
  }

  size_t max_length() const { return unsafe_get(&slot::max_length); }
  void max_length(std::optional<size_t> value) {
    if (auto csp = system::slot_address<slot>(_id)) {
      const auto limit = value.value_or(npos);
      csp->max_length = limit;
      if (limit != npos && csp->text().size() > limit) {
        auto clipped = csp->text();
        clipped.resize(limit);
        csp->commit_text(std::move(clipped), yw::min(size_t(csp->caret), limit));
      } else csp->make_dirty();
    }
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

  const auto& filter() const { return unsafe_get(&slot::filter); }
  void filter(function<bool, wchar_t> f) { safe_set(&slot::filter, std::move(f)); }
};
} // namespace yw::ui
