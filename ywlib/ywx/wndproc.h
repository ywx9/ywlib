#pragma once
#include <ywx/mainloop.h>

namespace yw {
namespace internal {
inline modifiers _make_mods_from_wparam(WPARAM wp) noexcept {
  return modifiers{
    .ctrl = static_cast<bool>(wp & MK_CONTROL),
    .shift = static_cast<bool>(wp & MK_SHIFT),
    .alt = (::GetKeyState(VK_MENU) & 0x8000) != 0};
}
inline modifiers _make_mods() noexcept {
  modifiers m{
    .ctrl = (::GetKeyState(VK_CONTROL) & 0x8000) != 0,
    .shift = (::GetKeyState(VK_SHIFT) & 0x8000) != 0,
    .alt = (::GetKeyState(VK_MENU) & 0x8000) != 0};
  return m;
}

template<UINT Msg> inline LRESULT handle_wm_pointer(window::slot* wsp, WPARAM wp, LPARAM lp) {
  if constexpr (Msg == WM_MOUSEMOVE) {
    const auto previous_cursor_pos = wsp->last_cursor_pos;
    const auto local_pos = short2(std::bit_cast<int16_t>(LOWORD(lp)), std::bit_cast<int16_t>(HIWORD(lp)));
    window::slot::cursor_pos = local_pos;
    ::ClientToScreen(wsp->hwnd, reinterpret_cast<POINT*>(&window::slot::cursor_pos));
    const auto delta = window::slot::cursor_pos - previous_cursor_pos;
    if ((wsp->mouse_capture_control_id || wsp->window_mouse_capture) &&
        (wp & (MK_LBUTTON | MK_RBUTTON | MK_MBUTTON | MK_XBUTTON1 | MK_XBUTTON2))) {
      drag_event e{};
      e.delta = delta;
      e.mods = internal::_make_mods_from_wparam(wp);
      if (wp & MK_LBUTTON) e.key = keys::lbutton;
      else if (wp & MK_RBUTTON) e.key = keys::rbutton;
      else if (wp & MK_MBUTTON) e.key = keys::mbutton;
      else if (wp & MK_XBUTTON1) e.key = keys::xbutton1;
      else if (wp & MK_XBUTTON2) e.key = keys::xbutton2;
      if (auto res = wsp->handle_drag_event(e); !res) res.error().go_off();
    }
    if (delta != int2{}) {
      pointer_event e{};
      e.pos = local_pos;
      e.delta = delta;
      if (auto res = wsp->handle_pointer_event(e); !res) res.error().go_off();
    }
    if (!wsp->track_mouse_event.hwndTrack) {
      wsp->track_mouse_event.hwndTrack = wsp->hwnd;
      ::TrackMouseEvent(&wsp->track_mouse_event);
    }
    const auto csp = interface::slot::get_as<control>(wsp->control_id);
    if (!csp) {
      if (wsp->cursor_locked) wsp->center_locked_cursor();
      else wsp->last_cursor_pos = window::slot::cursor_pos;
      return 0;
    }
    const auto hit = csp->hittest(local_pos);
    if (auto res = wsp->update_hovered_control(hit, local_pos, true, mainloop.elapsed()); !res) res.error().go_off();
    if (wsp->cursor_locked) wsp->center_locked_cursor();
    else wsp->last_cursor_pos = window::slot::cursor_pos;
  } else if constexpr (Msg == WM_MOUSELEAVE) {
    wsp->track_mouse_event.hwndTrack = nullptr;
    if (const auto hcsp = interface::slot::get_as<control>(wsp->hovered_control_id)) {
      hcsp->handle_hover_event(hover_event::create::leave(wsp->get_local_pointer_pos()));
      wsp->hovered_control_id = {};
    }
    wsp->hide_tooltip();
  }
  return 0;
}

template<UINT Msg> LRESULT handle_wm_focus(window::slot* wsp, WPARAM wp, LPARAM lp) {
  if constexpr (Msg == WM_SETFOCUS) {
    if (auto res = wsp->handle_focus_event({true}); !res) res.error().go_off();
  } else if constexpr (Msg == WM_KILLFOCUS || Msg == WM_ACTIVATEAPP) {
    if constexpr (Msg == WM_ACTIVATEAPP)
      if (wp) return ::DefWindowProcW(wsp->hwnd, Msg, wp, lp);
    if (const auto hcsp = interface::slot::get_as<control>(wsp->hovered_control_id)) {
      hcsp->handle_hover_event(hover_event::create::leave(wsp->get_local_pointer_pos()));
      wsp->hovered_control_id = {};
    }
    wsp->track_mouse_event.hwndTrack = nullptr;
    wsp->hide_tooltip();
    if (auto res = wsp->handle_focus_event({false}); !res) res.error().go_off();
  } else return ::DefWindowProcW(wsp->hwnd, Msg, wp, lp);
  return 0;
}

template<UINT Msg> LRESULT handle_wm_size_move(window::slot* wsp, WPARAM wp, LPARAM lp) {
  if constexpr (Msg == WM_GETMINMAXINFO) {
    if (auto res = wsp->get_necessary_size()) {
      auto& mmi = *reinterpret_cast<MINMAXINFO*>(lp);
      const auto area = *res + wsp->frame_thickness.xy() + wsp->frame_thickness.zw();
      mmi.ptMinTrackSize.x = area.x, mmi.ptMinTrackSize.y = area.y;
    } else res.error().go_off();
  } else if constexpr (Msg == WM_SIZE) {
    if (wsp->resizing) return 0;
    wsp->size = int2(LOWORD(lp), HIWORD(lp));
    wsp->messy = true;
  } else if constexpr (Msg == WM_MOVE) {
    wsp->pos = int2(static_cast<int16_t>(LOWORD(lp)), static_cast<int16_t>(HIWORD(lp))) - wsp->frame_thickness.xy();
  } else if constexpr (Msg == WM_ENTERSIZEMOVE) {
    wsp->resizing = true;
  } else if constexpr (Msg == WM_EXITSIZEMOVE) {
    wsp->resizing = false;
    if (RECT cr{}; ::GetClientRect(wsp->hwnd, &cr)) wsp->size = int2(cr.right, cr.bottom);
    else error(errors::operation_failed, "GetClientRect failed").go_off();
    wsp->messy = true;
  } else return ::DefWindowProcW(wsp->hwnd, Msg, wp, lp);
  return 0;
}

inline LRESULT handle_wm_showwindow(window::slot* wsp, WPARAM wp, LPARAM) {
  const bool next_visible = static_cast<bool>(wp);
  if (wsp->visible == next_visible) return 0;
  if (!next_visible) wsp->clear_window_state();
  wsp->visible = next_visible;
  if (next_visible) wsp->messy = true;
  return 0;
}

inline LRESULT handle_wm_enable(window::slot* wsp, WPARAM wp, LPARAM) {
  const bool next_enabled = static_cast<bool>(wp);
  if (wsp->enabled == next_enabled) return 0;
  if (!next_enabled) wsp->clear_window_state();
  wsp->enabled = next_enabled;
  wsp->dirty = true;
  return 0;
}

inline LRESULT handle_wm_char(window::slot* wsp, WPARAM wp, LPARAM lp) {
  if (auto res = wsp->handle_char_event(static_cast<wchar_t>(wp)); !res) res.error().go_off();
  if (auto res = wsp->update_caret_pos(); !res) res.error().go_off();
  return 0;
}

inline LRESULT handle_wm_ime(window::slot* wsp, UINT msg, WPARAM wp, LPARAM lp) {
  if (auto res = wsp->update_caret_pos(); !res) res.error().go_off();
  return ::DefWindowProcW(wsp->hwnd, msg, wp, lp);
}

template<UINT Msg> LRESULT handle_wm_double_click(window::slot* wsp, WPARAM wp, LPARAM lp) {
  button_event e{};
  e.pos = short2(std::bit_cast<int16_t>(LOWORD(lp)), std::bit_cast<int16_t>(HIWORD(lp)));
  e.mods = internal::_make_mods_from_wparam(wp);
  e.down = true;
  if constexpr (Msg == WM_LBUTTONDBLCLK) e.key = keys::lbutton;
  else if constexpr (Msg == WM_RBUTTONDBLCLK) e.key = keys::rbutton;
  else if constexpr (Msg == WM_MBUTTONDBLCLK) e.key = keys::mbutton;
  else return ::DefWindowProcW(wsp->hwnd, Msg, wp, lp);
  bool handled = false;
  if (auto res = wsp->handle_button_event(e); !res) res.error().go_off();
  else handled = *res;
  if (auto res = wsp->handle_double_click_event(e); !res) res.error().go_off();
  else handled = handled || *res;
  return handled ? 0 : ::DefWindowProcW(wsp->hwnd, Msg, wp, lp);
}

template<UINT Msg> LRESULT handle_wm_button_event(window::slot* wsp, WPARAM wp, LPARAM lp) {
  button_event e{};
  e.pos = short2(std::bit_cast<int16_t>(LOWORD(lp)), std::bit_cast<int16_t>(HIWORD(lp)));
  e.mods = internal::_make_mods_from_wparam(wp);
  if constexpr (Msg == WM_LBUTTONDOWN) e.key = keys::lbutton, e.down = true;
  else if constexpr (Msg == WM_LBUTTONUP) e.key = keys::lbutton, e.down = false;
  else if constexpr (Msg == WM_RBUTTONDOWN) e.key = keys::rbutton, e.down = true;
  else if constexpr (Msg == WM_RBUTTONUP) e.key = keys::rbutton, e.down = false;
  else if constexpr (Msg == WM_MBUTTONDOWN) e.key = keys::mbutton, e.down = true;
  else if constexpr (Msg == WM_MBUTTONUP) e.key = keys::mbutton, e.down = false;
  else if constexpr (Msg == WM_XBUTTONDOWN)
    (e.key = (HIWORD(wp) == XBUTTON1) ? keys::xbutton1 : keys::xbutton2), e.down = true;
  else if constexpr (Msg == WM_XBUTTONUP)
    (e.key = (HIWORD(wp) == XBUTTON1) ? keys::xbutton1 : keys::xbutton2), e.down = false;
  else return ::DefWindowProcW(wsp->hwnd, Msg, wp, lp);
  if (auto res = wsp->handle_button_event(e); !res) {
    res.error().go_off();
    return 0;
  } else if (*res) return 0;
  return ::DefWindowProcW(wsp->hwnd, Msg, wp, lp);
}

template<UINT Msg> LRESULT handle_wm_wheel_event(window::slot* wsp, WPARAM wp, LPARAM lp) {
  wheel_event e{};
  POINT pos{std::bit_cast<int16_t>(LOWORD(lp)), std::bit_cast<int16_t>(HIWORD(lp))};
  ::ScreenToClient(wsp->hwnd, &pos);
  e.pos = short2(pos.x, pos.y);
  e.delta = short(HIWORD(wp));
  e.mods = internal::_make_mods();
  e.horizontal = (Msg == WM_MOUSEHWHEEL);
  if (auto res = wsp->handle_wheel_event(e); !res) {
    res.error().go_off();
    return 0;
  } else if (*res) return 0;
  return ::DefWindowProcW(wsp->hwnd, Msg, wp, lp);
}

template<UINT Msg> LRESULT handle_wm_key_event(window::slot* wsp, WPARAM wp, LPARAM lp) {
  key_event e{};
  e.key = key{static_cast<uint8_t>(wp)};
  e.mods = internal::_make_mods();
  e.down = (Msg == WM_KEYDOWN || Msg == WM_SYSKEYDOWN);
  if (auto res = wsp->handle_key_event(e); !res) {
    res.error().go_off();
    return 0;
  } else if (*res) return 0;
  return ::DefWindowProcW(wsp->hwnd, Msg, wp, lp);
}
} // namespace internal

inline LRESULT __stdcall wclass::wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
  auto& windows = window::slot::windows;
  const auto wid = std::bit_cast<interface::slotid>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA));
  const auto wsp = static_cast<window::slot*>(interface::slot::slots.get(wid));
  if (!wsp) return ::DefWindowProcW(hwnd, msg, wp, lp);
  switch (msg) {

  case WM_MOUSEMOVE: return internal::handle_wm_pointer<WM_MOUSEMOVE>(wsp, wp, lp);
  case WM_MOUSELEAVE: return internal::handle_wm_pointer<WM_MOUSELEAVE>(wsp, wp, lp);

  case WM_GETMINMAXINFO: return internal::handle_wm_size_move<WM_GETMINMAXINFO>(wsp, wp, lp);
  case WM_SIZE: return internal::handle_wm_size_move<WM_SIZE>(wsp, wp, lp);
  case WM_MOVE: return internal::handle_wm_size_move<WM_MOVE>(wsp, wp, lp);
  case WM_ENTERSIZEMOVE: return internal::handle_wm_size_move<WM_ENTERSIZEMOVE>(wsp, wp, lp);
  case WM_EXITSIZEMOVE: return internal::handle_wm_size_move<WM_EXITSIZEMOVE>(wsp, wp, lp);
  case WM_SHOWWINDOW: return internal::handle_wm_showwindow(wsp, wp, lp);
  case WM_ENABLE: return internal::handle_wm_enable(wsp, wp, lp);

  case WM_KEYDOWN: return internal::handle_wm_key_event<WM_KEYDOWN>(wsp, wp, lp);
  case WM_KEYUP: return internal::handle_wm_key_event<WM_KEYUP>(wsp, wp, lp);
  case WM_SYSKEYDOWN: return internal::handle_wm_key_event<WM_SYSKEYDOWN>(wsp, wp, lp);
  case WM_SYSKEYUP: return internal::handle_wm_key_event<WM_SYSKEYUP>(wsp, wp, lp);

  case WM_CHAR:
  case WM_SYSCHAR: return internal::handle_wm_char(wsp, wp, lp);

  case WM_IME_STARTCOMPOSITION:
  case WM_IME_COMPOSITION:
  case WM_INPUTLANGCHANGE: return internal::handle_wm_ime(wsp, msg, wp, lp);

  case WM_LBUTTONDBLCLK: return internal::handle_wm_double_click<WM_LBUTTONDBLCLK>(wsp, wp, lp);
  case WM_RBUTTONDBLCLK: return internal::handle_wm_double_click<WM_RBUTTONDBLCLK>(wsp, wp, lp);
  case WM_MBUTTONDBLCLK: return internal::handle_wm_double_click<WM_MBUTTONDBLCLK>(wsp, wp, lp);

  case WM_LBUTTONDOWN: return internal::handle_wm_button_event<WM_LBUTTONDOWN>(wsp, wp, lp);
  case WM_LBUTTONUP: return internal::handle_wm_button_event<WM_LBUTTONUP>(wsp, wp, lp);
  case WM_RBUTTONDOWN: return internal::handle_wm_button_event<WM_RBUTTONDOWN>(wsp, wp, lp);
  case WM_RBUTTONUP: return internal::handle_wm_button_event<WM_RBUTTONUP>(wsp, wp, lp);
  case WM_MBUTTONDOWN: return internal::handle_wm_button_event<WM_MBUTTONDOWN>(wsp, wp, lp);
  case WM_MBUTTONUP: return internal::handle_wm_button_event<WM_MBUTTONUP>(wsp, wp, lp);
  case WM_XBUTTONDOWN: return internal::handle_wm_button_event<WM_XBUTTONDOWN>(wsp, wp, lp);
  case WM_XBUTTONUP: return internal::handle_wm_button_event<WM_XBUTTONUP>(wsp, wp, lp);

  case WM_MOUSEWHEEL: return internal::handle_wm_wheel_event<WM_MOUSEWHEEL>(wsp, wp, lp);
  case WM_MOUSEHWHEEL: return internal::handle_wm_wheel_event<WM_MOUSEHWHEEL>(wsp, wp, lp);

  case WM_SETFOCUS: return internal::handle_wm_focus<WM_SETFOCUS>(wsp, wp, lp);
  case WM_KILLFOCUS: return internal::handle_wm_focus<WM_KILLFOCUS>(wsp, wp, lp);
  case WM_ACTIVATEAPP: return internal::handle_wm_focus<WM_ACTIVATEAPP>(wsp, wp, lp);

  case WM_CAPTURECHANGED:
    if (const auto ccsp = static_cast<control::slot*>(interface::slot::slots.get(wsp->mouse_capture_control_id)))
      ccsp->reset_state();
    wsp->mouse_capture_control_id = {};
    wsp->window_mouse_capture = false;
    wsp->dirty = true;
    return 0;

  case WM_NCDESTROY:
    if (auto res = wsp->close_subwindows(); !res) res.error().go_off();
    if (const auto psp = static_cast<window::slot*>(interface::slot::slots.get(wsp->parent_id)))
      psp->subwindows.erase(std::remove(psp->subwindows.begin(), psp->subwindows.end(), wid), psp->subwindows.end());
    ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
    if (const auto csp = static_cast<control::slot*>(interface::slot::slots.get(wsp->control_id))) {
      csp->close_child_controls();
      interface::slot::slots.erase(wsp->control_id);
    }
    wsp->hwnd = 0;
    windows.erase(std::remove(windows.begin(), windows.end(), wid), windows.end());
    if (auto res = interface::slot::slots.erase(wid); !res) res.error().go_off();
    if (windows.empty()) ::PostQuitMessage(0);
    return 0;
  }
  return ::DefWindowProcW(hwnd, msg, wp, lp);
}
} // namespace yw
