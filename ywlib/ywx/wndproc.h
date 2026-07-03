#pragma once
#include <ywx/window.h>

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
    if (auto res = wsp->update_hovered_control(hit, local_pos, true); !res) res.error().go_off();
    return 0;
  }
  case WM_MOUSELEAVE:
    wsp->track_mouse_event.hwndTrack = nullptr;
    if (const auto hcsp = interface::slot::get<control>(wsp->hovered_control_id)) {
      const auto pos = window::slot::cursor_pos - wsp->pos;
      hcsp->hover_event({.pos = pos, .type = hover_event::type::leave});
      wsp->hovered_control_id = {};
    }
    return 0;
  }
  return 0;
}

inline LRESULT _process_wm_size_move(window::slot* wsp, UINT msg, WPARAM wp, LPARAM lp) {
  switch (msg) {
  case WM_GETMINMAXINFO:
    if (auto res = wsp->get_necessary_size()) {
      auto& mmi = *reinterpret_cast<MINMAXINFO*>(lp);
      const auto area = *res + wsp->frame_thickness.xy() + wsp->frame_thickness.zw();
      mmi.ptMinTrackSize.x = area.x;
      mmi.ptMinTrackSize.y = area.y;
    } else res.error().go_off();
    return 0;
  case WM_SIZE:
    if (wsp->resizing) break;
    wsp->size = int2(LOWORD(lp), HIWORD(lp));
    wsp->messy = true;
    return 0;
  case WM_MOVE:
    wsp->pos = int2(static_cast<int16_t>(LOWORD(lp)), static_cast<int16_t>(HIWORD(lp))) - wsp->frame_thickness.xy();
    return 0;
  case WM_ENTERSIZEMOVE: wsp->resizing = true; return 0;
  case WM_EXITSIZEMOVE:
    wsp->resizing = false;
    if (RECT cr{}; ::GetClientRect(wsp->hwnd, &cr)) wsp->size = int2(cr.right, cr.bottom);
    else error(errors::operation_failed, "GetClientRect failed").go_off();
    wsp->messy = true;
    return 0;
  }
  return ::DefWindowProcW(wsp->hwnd, msg, wp, lp);
}

inline LRESULT _process_wm_button_event(window::slot* wsp, UINT msg, WPARAM wp, LPARAM lp) {
  button_event e{};
  e.pos = short2(std::bit_cast<int16_t>(LOWORD(lp)), std::bit_cast<int16_t>(HIWORD(lp)));
  e.down = (msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg == WM_MBUTTONDOWN || msg == WM_XBUTTONDOWN);
  e.mods = internal::_make_mods_from_wparam(wp);
  switch (msg) {
  case WM_LBUTTONDOWN:
  case WM_LBUTTONUP: e.key = keys::lbutton; break;
  case WM_RBUTTONDOWN:
  case WM_RBUTTONUP: e.key = keys::rbutton; break;
  case WM_MBUTTONDOWN:
  case WM_MBUTTONUP: e.key = keys::mbutton; break;
  case WM_XBUTTONDOWN:
  case WM_XBUTTONUP: e.key = (HIWORD(wp) == XBUTTON1) ? keys::xbutton1 : keys::xbutton2; break;
  default: return ::DefWindowProcW(wsp->hwnd, msg, wp, lp);
  }
  wsp->button_event(e);
  return ::DefWindowProcW(wsp->hwnd, msg, wp, lp);
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

  case WM_GETMINMAXINFO:
  case WM_SIZE:
  case WM_MOVE:
  case WM_ENTERSIZEMOVE:
  case WM_EXITSIZEMOVE: return internal::_process_wm_size_move(wsp, msg, wp, lp);

  case WM_LBUTTONDOWN:
  case WM_LBUTTONUP:
  case WM_RBUTTONDOWN:
  case WM_RBUTTONUP:
  case WM_MBUTTONDOWN:
  case WM_MBUTTONUP:
  case WM_XBUTTONDOWN:
  case WM_XBUTTONUP:
    return internal::_process_wm_button_event(wsp, msg, wp, lp);

    //-- close and destroy --//

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
