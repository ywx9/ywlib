#pragma once
#include <apps/ui_label.h>

namespace yw::ui {

class selectable_label : public label {
public:
  property<function<int, wchar_t>, selectable_label> character_group{[](wchar_t c) {
    return is_alnum(c) ? 0 : (is_ascii(c) ? 1 : 2);
  }};

protected:
  uint32_t _anchor = 0;
  uint32_t _caret = 0;
  bool _selecting = false;

  result<float2> _text_origin() {
    result<float2> offset = _calculate_text_origin();
    if (!offset) return offset.relay();
    return _current_pos + *offset;
  }

  uint32_t _length() const { return uint32_t(string().size()); }
  void _clamp_selection() {
    _anchor = yw::min(_anchor, _length());
    _caret = yw::min(_caret, _length());
  }
  void _set_selection(uint32_t anchor, uint32_t caret) {
    _anchor = anchor;
    _caret = caret;
    _clamp_selection();
    window_system::make_dirty(_window);
  }

  int _character_group(wchar_t c) {
    if (character_group()) return character_group.ref()(c);
    return is_alnum(c) ? 0 : (is_ascii(c) ? 1 : 2);
  }

  uint2 _group_range(uint32_t index) {
    if (string().empty()) return {};
    uint32_t start = yw::min(index, _length() - 1), end = start + 1;
    const auto kind = _character_group(string()[start]);
    while (start && _character_group(string()[start - 1]) == kind) --start;
    while (end < _length() && _character_group(string()[end]) == kind) ++end;
    return {start, end};
  }

  uint32_t _group_position(uint32_t position, bool forward) {
    if (forward) return position < _length() ? _group_range(position).y() : _length();
    return position ? _group_range(position - 1).x() : 0;
  }

  result<uint32_t> _position_at(float2 point) {
    if (!_text || string().empty()) return uint32_t(0);
    auto origin = _text_origin();
    if (!origin) return origin.relay();
    point -= *origin;
    DWRITE_HIT_TEST_METRICS metrics{};
    BOOL trailing, inside;
    if (FAILED(_text.dwrite_text_layout()->HitTestPoint(point.x(), point.y(), &trailing, &inside, &metrics)))
      return error(errors::operation_failed, "HitTestPoint failed");
    return yw::min(metrics.textPosition + (trailing ? metrics.length : 0), _length());
  }

  result<uint32_t> _step_position(uint32_t position, bool forward) {
    if (!_text) return uint32_t(0);
    uint32_t count = 0;
    auto layout = _text.dwrite_text_layout();
    auto hr = layout->GetClusterMetrics(nullptr, 0, &count);
    if (FAILED(hr) && hr != E_NOT_SUFFICIENT_BUFFER)
      return error(errors::operation_failed, "GetClusterMetrics failed");
    if (!count) return uint32_t(0);
    array<DWRITE_CLUSTER_METRICS> clusters(count);
    if (FAILED(layout->GetClusterMetrics(clusters.data(), count, &count)))
      return error(errors::operation_failed, "GetClusterMetrics failed");
    uint32_t start = 0;
    for (const auto& cluster : clusters) {
      const auto end = start + cluster.length;
      if (forward ? position < end : position <= end) return forward ? end : start;
      start = end;
    }
    return _length();
  }

  virtual result<void> _draw_accent(window& win, color accent_color) override {
    if (accent_color.a *= float(_focused(win) + _hovered(win)); accent_color.a <= 0.0f) return {};
    if (auto res = fill_geometry(_geometry, accent_color); !res) return res.relay();
    return {};
  }

  virtual result<void> _draw_content(window& win) override {
    _clamp_selection();
    if (auto res = frame::_draw_content(win); !res) return res.relay();
    auto origin = _text_origin();
    if (!origin) return origin.relay();
    const auto range = selection();
    if (_text && range.x() != range.y()) {
      auto rectangles = _text.hittest_range(range, *origin);
      if (!rectangles) return rectangles.relay();
      // auto c = color(win.color_theme().accent, win.overlay_opacity());
      auto c = win.color_theme().accent;
      if (!_enabled) c = _get_disabled_color(win, c);
      for (const auto& rectangle : *rectangles)
        if (auto res = fill_rectangle(rectangle.pos, rectangle.size, c); !res) return res.relay();
    }
    const auto& c = text_color();
    if (_text && c.a > 0)
      if (auto res = draw_text(*origin, _text, _enabled ? c : _get_disabled_color(win, c)); !res)
        return res.relay();
    return {};
  }

  virtual result<bool> _handle_button_event(window& win, yw::button_event e) override {
    if (!_visible || !_enabled) { _clear_state(win); return false; }
    if (e.key != keys::lbutton) {
      _selecting = false;
      return label::_handle_button_event(win, e);
    }
    auto position = _position_at(e.pos);
    if (!position) return position.relay();
    _clamp_selection();
    if (e.state.down) {
      _selecting = true;
      _set_selection(e.state.shift ? _anchor : *position, *position);
    } else {
      if (_selecting) _set_selection(_anchor, *position);
      _selecting = false;
    }
    return true;
  }

  virtual result<bool> _handle_drag_event(window& win, yw::drag_event e) override {
    if (!_visible || !_enabled) { _clear_state(win); return false; }
    if (!_selecting || e.key != keys::lbutton) return label::_handle_drag_event(win, e);
    // Query the current point, rather than accumulating deltas from a stale initial mouse position.
    auto point = window_system::cursor_pos();
    if (!::ScreenToClient(win.hwnd(), reinterpret_cast<POINT*>(&point)))
      return error(errors::operation_failed, "ScreenToClient failed");
    auto position = _position_at(point);
    if (!position) return position.relay();
    _set_selection(_anchor, *position);
    return true;
  }

  virtual result<bool> _handle_double_click_event(window& win, yw::button_event e) override {
    if (!_visible || !_enabled) { _clear_state(win); return false; }
    if (e.key != keys::lbutton) return label::_handle_double_click_event(win, e);
    auto position = _position_at(e.pos);
    if (!position) return position.relay();
    if (string().empty()) return true;
    const auto range = _group_range(*position);
    _set_selection(range.x(), range.y());
    _selecting = false;
    return true;
  }

  virtual result<bool> _handle_key_event(window& win, yw::key_event e) override {
    if (!_visible || !_enabled) { _clear_state(win); return false; }
    if (!e.state.down || e.state.alt) return label::_handle_key_event(win, e);
    _clamp_selection();
    if (e.state.ctrl && e.key == keys::a) { _set_selection(0, _length()); return true; }
    if (e.state.ctrl && e.key == keys::c) {
      if (auto res = copy_selection(); !res) return res.relay();
      return true;
    }
    if (e.state.ctrl && e.key != keys::left && e.key != keys::right)
      return label::_handle_key_event(win, e);
    uint32_t next;
    if (e.key == keys::home) next = 0;
    else if (e.key == keys::end) next = _length();
    else if (e.key == keys::left || e.key == keys::right) {
      if (e.state.ctrl)
        next = _group_position(_caret, e.key == keys::right);
      else if (!e.state.shift && _anchor != _caret)
        next = e.key == keys::left ? selection().x() : selection().y();
      else {
        auto position = e.key == keys::left ? _step_position(_caret, false) : _step_position(_caret, true);
        if (!position) return position.relay();
        next = *position;
      }
    } else return label::_handle_key_event(win, e);
    _set_selection(e.state.shift ? _anchor : next, next);
    return true;
  }

  virtual result<bool> _handle_focus_event(window& win, yw::focus_event e) override {
    if (!e.focused) _selecting = false;
    return label::_handle_focus_event(win, e);
  }
  virtual void _clear_state(window& win) noexcept override {
    label::_clear_state(win);
    _selecting = false;
  }

public:
  selectable_label() noexcept = default;
  selectable_label(selectable_label&&) noexcept = default;
  selectable_label& operator=(selectable_label&&) noexcept = default;

  // Half-open UTF-16 range; direction is retained internally by anchor and caret.
  uint2 selection() const noexcept {
    const auto a = yw::min(_anchor, _length()), c = yw::min(_caret, _length());
    return {yw::min(a, c), yw::max(a, c)};
  }
  result<void> selection(uint2 range) {
    if (range.x() > _length() || range.y() > _length())
      return error(errors::invalid_argument, "Selection is out of bounds");
    _set_selection(range.x(), range.y());
    return {};
  }
  result<void> select_all() { _set_selection(0, _length()); return {}; }
  result<void> clear_selection() { _set_selection(0, 0); return {}; }

  result<void> copy_selection() const {
    const auto range = selection();
    if (range.x() == range.y()) return {};
    const auto count = range.y() - range.x();
    const auto memory = ::GlobalAlloc(GMEM_MOVEABLE, (size_t(count) + 1) * sizeof(wchar_t));
    if (!memory) return error(errors::operation_failed, "GlobalAlloc failed");
    auto buffer = static_cast<wchar_t*>(::GlobalLock(memory));
    if (!buffer) { ::GlobalFree(memory); return error(errors::operation_failed, "GlobalLock failed"); }
    std::copy_n(string().data() + range.x(), count, buffer);
    buffer[count] = L'\0';
    ::GlobalUnlock(memory);
    if (!::OpenClipboard(_window)) { ::GlobalFree(memory); return error(errors::operation_failed, "OpenClipboard failed"); }
    const bool success = ::EmptyClipboard() && ::SetClipboardData(CF_UNICODETEXT, memory);
    ::CloseClipboard();
    if (!success) { ::GlobalFree(memory); return error(errors::operation_failed, "SetClipboardData failed"); }
    return {};
  }

  virtual bool focusable() const override { return true; }
  virtual bool interactive() const override { return true; }
};
} // namespace yw::ui
