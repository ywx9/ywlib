#pragma once
#include <ywx/mainloop.h>

namespace yw {
namespace internal {
inline modifiers _make_mods_from_wparam(WPARAM wp) noexcept {
  return modifiers{.ctrl = static_cast<bool>(wp & MK_CONTROL),
    .shift = static_cast<bool>(wp & MK_SHIFT),
    .alt = (::GetKeyState(VK_MENU) & 0x8000) != 0};
}
inline modifiers _make_mods() noexcept {
  modifiers m{.ctrl = (::GetKeyState(VK_CONTROL) & 0x8000) != 0,
    .shift = (::GetKeyState(VK_SHIFT) & 0x8000) != 0,
    .alt = (::GetKeyState(VK_MENU) & 0x8000) != 0};
  return m;
}

inline LRESULT _process_wm_cursor(window::slot* wsp, UINT msg, WPARAM wp, LPARAM lp) {
  switch (msg) {
  case WM_MOUSEMOVE: {
    window::slot::cursor_pos.x = std::bit_cast<int16_t>(LOWORD(lp));
    window::slot::cursor_pos.y = std::bit_cast<int16_t>(HIWORD(lp));
    const auto local_pos = window::slot::cursor_pos;
    ::ClientToScreen(wsp->hwnd, reinterpret_cast<POINT*>(&window::slot::cursor_pos));
    if (!wsp->track_mouse_event.hwndTrack) {
      wsp->track_mouse_event.hwndTrack = wsp->hwnd;
      ::TrackMouseEvent(&wsp->track_mouse_event);
    }
    const auto csp = interface::slot::get<control>(wsp->control_id);
    if (!csp) return 0;
    const auto hit = csp->hittest(local_pos);
    if (auto res = wsp->update_hovered_control(hit, local_pos, true, mainloop.elapsed()); !res) res.error().go_off();
    return 0;
  }
  case WM_MOUSELEAVE:
    wsp->track_mouse_event.hwndTrack = nullptr;
    if (const auto hcsp = interface::slot::get<control>(wsp->hovered_control_id)) {
      const auto pos = window::slot::cursor_pos - wsp->pos;
      hcsp->hover_event({.pos = pos, .type = hover_event::type::leave});
      wsp->hovered_control_id = {};
    }
    wsp->hide_tooltip();
    return 0;
  }
  return 0;
}

template<UINT Msg> LRESULT _process_wm_focus(window::slot* wsp, WPARAM wp, LPARAM lp) {
  if constexpr (Msg == WM_ACTIVATEAPP)
    if (!wp) return ::DefWindowProcW(wsp->hwnd, Msg, wp, lp);
  if constexpr (Msg == WM_KILLFOCUS || Msg == WM_ACTIVATEAPP) {
    if (const auto hcsp = interface::slot::get<control>(wsp->hovered_control_id)) {
      const auto pos = window::slot::cursor_pos - wsp->pos;
      hcsp->hover_event({.pos = pos, .type = hover_event::type::leave});
      wsp->hovered_control_id = {};
    }
    wsp->track_mouse_event.hwndTrack = nullptr;
    wsp->hide_tooltip();
  } else return ::DefWindowProcW(wsp->hwnd, Msg, wp, lp);
  return 0;
}

template<UINT Msg> LRESULT _process_wm_size_move(window::slot* wsp, WPARAM wp, LPARAM lp) {
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

template<UINT Msg> LRESULT _process_wm_button_event(window::slot* wsp, WPARAM wp, LPARAM lp) {
  button_event e{};
  e.pos = short2(std::bit_cast<int16_t>(LOWORD(lp)), std::bit_cast<int16_t>(HIWORD(lp)));
  e.mods = internal::_make_mods_from_wparam(wp);
  if constexpr (Msg == WM_LBUTTONDOWN) {
    const auto hcsp = interface::slot::get<control>(wsp->hovered_control_id);
    if (hcsp && hcsp->focusable()) {
      if (auto res = wsp->update_focused_control(wsp->hovered_control_id); !res) res.error().go_off();
    } else if (auto res = wsp->update_focused_control(none{}); !res) res.error().go_off();
    e.key = keys::lbutton, e.down = true;
  } else if constexpr (Msg == WM_LBUTTONUP) e.key = keys::lbutton, e.down = false;
  else if constexpr (Msg == WM_RBUTTONDOWN) e.key = keys::rbutton, e.down = true;
  else if constexpr (Msg == WM_RBUTTONUP) e.key = keys::rbutton, e.down = false;
  else if constexpr (Msg == WM_MBUTTONDOWN) e.key = keys::mbutton, e.down = true;
  else if constexpr (Msg == WM_MBUTTONUP) e.key = keys::mbutton, e.down = false;
  else if constexpr (Msg == WM_XBUTTONDOWN)
    (e.key = (HIWORD(wp) == XBUTTON1) ? keys::xbutton1 : keys::xbutton2), e.down = true;
  else if constexpr (Msg == WM_XBUTTONUP)
    (e.key = (HIWORD(wp) == XBUTTON1) ? keys::xbutton1 : keys::xbutton2), e.down = false;
  else return ::DefWindowProcW(wsp->hwnd, Msg, wp, lp);
  wsp->button_event(e);
  return ::DefWindowProcW(wsp->hwnd, Msg, wp, lp);
}
} // namespace internal

inline LRESULT __stdcall wclass::wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
  auto& windows = window::slot::windows;
  const auto wid = std::bit_cast<interface::slotid>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA));
  const auto wsp = interface::slot::get<window>(wid);
  if (!wsp) return ::DefWindowProcW(hwnd, msg, wp, lp);
  switch (msg) {

  case WM_MOUSEMOVE:
  case WM_MOUSELEAVE: return internal::_process_wm_cursor(wsp, msg, wp, lp);

  case WM_GETMINMAXINFO: return internal::_process_wm_size_move<WM_GETMINMAXINFO>(wsp, wp, lp);
  case WM_SIZE: return internal::_process_wm_size_move<WM_SIZE>(wsp, wp, lp);
  case WM_MOVE: return internal::_process_wm_size_move<WM_MOVE>(wsp, wp, lp);
  case WM_ENTERSIZEMOVE: return internal::_process_wm_size_move<WM_ENTERSIZEMOVE>(wsp, wp, lp);
  case WM_EXITSIZEMOVE: return internal::_process_wm_size_move<WM_EXITSIZEMOVE>(wsp, wp, lp);

  case WM_LBUTTONDOWN: return internal::_process_wm_button_event<WM_LBUTTONDOWN>(wsp, wp, lp);
  case WM_LBUTTONUP: return internal::_process_wm_button_event<WM_LBUTTONUP>(wsp, wp, lp);
  case WM_RBUTTONDOWN: return internal::_process_wm_button_event<WM_RBUTTONDOWN>(wsp, wp, lp);
  case WM_RBUTTONUP: return internal::_process_wm_button_event<WM_RBUTTONUP>(wsp, wp, lp);
  case WM_MBUTTONDOWN: return internal::_process_wm_button_event<WM_MBUTTONDOWN>(wsp, wp, lp);
  case WM_MBUTTONUP: return internal::_process_wm_button_event<WM_MBUTTONUP>(wsp, wp, lp);
  case WM_XBUTTONDOWN: return internal::_process_wm_button_event<WM_XBUTTONDOWN>(wsp, wp, lp);
  case WM_XBUTTONUP: return internal::_process_wm_button_event<WM_XBUTTONUP>(wsp, wp, lp);

  case WM_KILLFOCUS: return internal::_process_wm_focus<WM_KILLFOCUS>(wsp, wp, lp);
  case WM_ACTIVATEAPP: return internal::_process_wm_focus<WM_ACTIVATEAPP>(wsp, wp, lp);

  case WM_NCDESTROY:
    ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
    wsp->hwnd = 0;
    windows.erase(std::remove(windows.begin(), windows.end(), wid), windows.end());
    if (auto res = interface::slot::slots.erase(wid); !res) res.error().go_off();
    if (windows.empty()) ::PostQuitMessage(0);
    return 0;
  }
  return ::DefWindowProcW(hwnd, msg, wp, lp);
}
} // namespace yw
