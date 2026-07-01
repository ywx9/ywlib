#pragma once
#include <ywx/window.h>

namespace yw {

inline LRESULT __stdcall wclass::wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
  auto& windows = window::slot::windows;
  const auto wid = std::bit_cast<interface::slotid>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA));
  const auto wsp = interface::slot::get<window>(wid);
  if (!wsp) return ::DefWindowProcW(hwnd, msg, wp, lp);
  switch (msg) {

    //-- size and move --//

  case WM_GETMINMAXINFO:
    if (auto res = wsp->get_necessary_size(); !res) res.error().go_off();
    else {
      auto& mmi = *reinterpret_cast<MINMAXINFO*>(lp);
      const auto area = *res + wsp->frame_thickness.xy() + wsp->frame_thickness.zw();
      mmi.ptMinTrackSize.x = area.x;
      mmi.ptMinTrackSize.y = area.y;
    }
    return 0;

  case WM_SIZE:
    if (wsp->resizing) break;
    wsp->size = int2(LOWORD(lp), HIWORD(lp));
    // if (auto res = wsp->update_rendertarget(); !res) res.error().go_off();
    wsp->layout_dirty = true;
    wsp->geometry_dirty = true;
    wsp->paint_dirty = true;
    return 0;

  case WM_MOVE:
    wsp->pos = int2(static_cast<int16_t>(LOWORD(lp)), static_cast<int16_t>(HIWORD(lp))) - wsp->frame_thickness.xy();
    return 0;

  case WM_ENTERSIZEMOVE: wsp->resizing = true; return 0;

  case WM_EXITSIZEMOVE:
    wsp->resizing = false;
    if (RECT cr{}; ::GetClientRect(hwnd, &cr)) {
      wsp->size = int2(cr.right, cr.bottom);
      // if (auto res = wsp->update_rendertarget(); !res) res.error().go_off();
    } else error(errors::operation_failed, "GetClientRect failed").go_off();
    wsp->layout_dirty = true;
    wsp->geometry_dirty = true;
    wsp->paint_dirty = true;
    return 0;

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
