#pragma once
#include <apps/window.h>

namespace yw {

///--------------------------------------------------------------------------///
/// MARK: wndproc

namespace internal {
inline key_state _wndproc_get_key_state(bool down, WPARAM wp) {
  return {down, bool(wp & MK_SHIFT), bool(wp & MK_CONTROL), (::GetKeyState(VK_MENU) & 0x8000) != 0};
}
inline short2 _wndproc_get_cursor_pos(LPARAM lp) {
  return {bitcast<int16_t>(LOWORD(lp)), bitcast<int16_t>(HIWORD(lp))};
}
} // namespace internal

LRESULT CALLBACK wclass::wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
  const auto win = window_system::get_window_pointer(hwnd);
  if (!win) {
    if (msg == WM_DESTROY && window_system::windows.empty()) return ::PostQuitMessage(0), 0;
    else return ::DefWindowProcW(hwnd, msg, wp, lp);
  }
  switch (msg) {
  /// MARK:
  case WM_MOUSEMOVE: {
    const auto window_pos = win->pos();
    const auto previous_cursor_pos = win->_previous_cursor_pos;
    const auto current_cursor_pos = int2(bitcast<int16_t>(LOWORD(lp)), bitcast<int16_t>(HIWORD(lp)));
    const auto cursor_delta = current_cursor_pos - previous_cursor_pos;
    win->_previous_cursor_pos = current_cursor_pos;
    if (cursor_delta != int2()) {
      if ((win->_captured_control_id || win->_window_captured) &&
          (wp & (MK_LBUTTON | MK_RBUTTON | MK_MBUTTON | MK_XBUTTON1 | MK_XBUTTON2))) {
        yw::drag_event e;
        e.delta = cursor_delta;
        e.state = internal::_wndproc_get_key_state(true, wp);
        if (wp & MK_LBUTTON) e.key = keys::lbutton;
        else if (wp & MK_RBUTTON) e.key = keys::rbutton;
        else if (wp & MK_MBUTTON) e.key = keys::mbutton;
        else if (wp & MK_XBUTTON1) e.key = keys::xbutton1;
        else if (wp & MK_XBUTTON2) e.key = keys::xbutton2;
        if (auto res = win->_handle_drag_event(e); !res) res.error().print_and_abort();
      }
      yw::hover_event e{current_cursor_pos};
      if (auto res = win->_handle_hover_event(e); !res) res.error().print_and_abort();
    }
    if (!win->_track_mouse_event.hwndTrack) {
      win->_track_mouse_event.hwndTrack = hwnd;
      ::TrackMouseEvent(&win->_track_mouse_event);
    }
    return 0;
  }
  /// MARK:
  case WM_MOUSELEAVE: {
    win->_track_mouse_event.hwndTrack = nullptr;
    auto current_cursor_pos = window_system::cursor_pos();
    ::ScreenToClient(hwnd, reinterpret_cast<POINT*>(&current_cursor_pos));
    if (auto res = win->_handle_hover_event({current_cursor_pos, hover_event::leave}); !res)
      res.error().print_and_abort();
    return 0;
  }
  /// MARK:
  case WM_GETMINMAXINFO: {
    if (auto res = win->_get_minimum_size()) {
      auto mmip = reinterpret_cast<MINMAXINFO*>(lp);
      const auto area = *res + win->frame_thickness().xy() + win->frame_thickness().zw();
      mmip->ptMinTrackSize.x = area.x(), mmip->ptMinTrackSize.y = area.y();
    } else res.error().print_and_abort();
    return 0;
  }
  /// MARK:
  case WM_SIZE: {
    if (win->_window_resizing) return 0;
    win->_current_size = uint2(LOWORD(lp), HIWORD(lp));
    win->_messy = true;
    return 0;
  }
  /// MARK:
  case WM_ENTERSIZEMOVE: {
    win->_window_resizing = true;
    return 0;
  }
  /// MARK:
  case WM_EXITSIZEMOVE: {
    win->_window_resizing = false;
    if (RECT cr; ::GetClientRect(hwnd, &cr)) win->_current_size = int2(cr.right, cr.bottom);
    else error(errors::operation_failed, "GetClientRect failed").print_and_abort();
    win->_messy = true;
    return 0;
  }
  /// MARK:
  case WM_KEYDOWN:
  case WM_KEYUP:
  case WM_SYSKEYDOWN:
  case WM_SYSKEYUP: {
    key_event e;
    e.key = key{static_cast<uint8_t>(wp)};
    e.state.down = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
    e.state.shift = (::GetKeyState(VK_SHIFT) & 0x8000) != 0;
    e.state.ctrl = (::GetKeyState(VK_CONTROL) & 0x8000) != 0;
    e.state.alt = (::GetKeyState(VK_MENU) & 0x8000) != 0;
    if (auto res = win->_handle_key_event(e)) {
      if (*res) return 0;
      else return ::DefWindowProcW(hwnd, msg, wp, lp);
    } else res.error().print_and_abort();
  }
  /// MARK:
  case WM_CHAR:
  case WM_SYSCHAR: {
    if (auto res = win->_handle_char_event(static_cast<wchar_t>(wp)); !res) res.error().print_and_abort();
    return 0;
  }
  /// MARK:
  case WM_MOUSEWHEEL:
  case WM_MOUSEHWHEEL: {
    const auto cursor_screen_pos = internal::_wndproc_get_cursor_pos(lp);
    wheel_event e;
    if (auto res = win->pos(); !res) res.error().print_and_abort();
    else e.pos = {cursor_screen_pos.x() - res->x(), cursor_screen_pos.y() - res->y()};
    e.delta[msg == WM_MOUSEHWHEEL] = bitcast<int16_t>(HIWORD(wp));
    e.state.shift = (::GetKeyState(VK_SHIFT) & 0x8000) != 0;
    e.state.ctrl = (::GetKeyState(VK_CONTROL) & 0x8000) != 0;
    e.state.alt = (::GetKeyState(VK_MENU) & 0x8000) != 0;
    if (auto res = win->_handle_wheel_event(e); !res) res.error().print_and_abort();
    return 0;
  }
  /// MARK:
  case WM_LBUTTONDOWN:
  case WM_LBUTTONUP:
  case WM_RBUTTONDOWN:
  case WM_RBUTTONUP:
  case WM_MBUTTONDOWN:
  case WM_MBUTTONUP:
  case WM_XBUTTONDOWN:
  case WM_XBUTTONUP: {
    button_event e;
    e.pos = internal::_wndproc_get_cursor_pos(lp);
    e.state = internal::_wndproc_get_key_state(
      msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg == WM_MBUTTONDOWN || msg == WM_XBUTTONDOWN, wp);
    if (msg == WM_LBUTTONDOWN || msg == WM_LBUTTONUP) e.key = keys::lbutton;
    else if (msg == WM_RBUTTONDOWN || msg == WM_RBUTTONUP) e.key = keys::rbutton;
    else if (msg == WM_MBUTTONDOWN || msg == WM_MBUTTONUP) e.key = keys::mbutton;
    else if (msg == WM_XBUTTONDOWN || msg == WM_XBUTTONUP) {
      if (HIWORD(wp) == XBUTTON1) e.key = keys::xbutton1;
      else if (HIWORD(wp) == XBUTTON2) e.key = keys::xbutton2;
    }
    if (auto res = win->_handle_button_event(e); !res) res.error().print_and_abort();
    return 0;
  }
  /// MARK:
  case WM_LBUTTONDBLCLK:
  case WM_RBUTTONDBLCLK:
  case WM_MBUTTONDBLCLK: {
    button_event e;
    e.pos = internal::_wndproc_get_cursor_pos(lp);
    e.state = internal::_wndproc_get_key_state(true, wp);
    if (msg == WM_LBUTTONDBLCLK) e.key = keys::lbutton;
    else if (msg == WM_RBUTTONDBLCLK) e.key = keys::rbutton;
    else if (msg == WM_MBUTTONDBLCLK) e.key = keys::mbutton;
    bool handled = false;
    if (auto res = win->_handle_button_event(e)) handled = *res;
    else res.error().print_and_abort();
    if (auto res = win->_handle_double_click_event(e)) handled |= *res;
    else res.error().print_and_abort();
    return handled ? 0 : ::DefWindowProcW(hwnd, msg, wp, lp);
  }
  /// MARK:
  case WM_CLOSE: win->close(); return 0;
  }
  return ::DefWindowProcW(hwnd, msg, wp, lp);
}

///--------------------------------------------------------------------------///
/// MARK: mainloop

class mainloop {
  inline static uint64_t _frame_count = 0;
  inline static double _last_elapsed = 0.0;

public:
  inline static const_property<yw::stopwatch, mainloop> stopwatch = {};
  inline static const_property<bool, mainloop> running = false;
  explicit operator bool() const noexcept { return running(); }
  inline static const_property<double, mainloop> spf = 0.0;
  static double fps() noexcept { return spf() > 0.0 ? 1.0 / spf() : 0.0; }

  mainloop() {
    if (window_system::windows.empty()) {
      running = false;
      return;
    }
    if (!running) {
      stopwatch.ref().restart();
      _frame_count = 0;
      _last_elapsed = 0.0;
      spf = 0.0;
    }
    const auto now = stopwatch().elapsed();
    spf = now - _last_elapsed, _last_elapsed = now;
    for (MSG msg; ::PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE);)
      if (msg.message == WM_QUIT) {
        running = false;
        return;
      } else ::TranslateMessage(&msg), ::DispatchMessageW(&msg);
    for (auto hwnd : window_system::windows)
      if (const auto win = window_system::get_window_pointer(hwnd); win)
        if (auto res = win->update(); !res) res.error().print_and_abort();
    ++_frame_count;
    running = true;
  }
};
} // namespace yw
