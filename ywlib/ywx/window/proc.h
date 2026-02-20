#pragma once
#include "ywx/window/slot.h"

namespace yw::window {

inline LRESULT __stdcall decltype(system)::proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  auto self = reinterpret_cast<window::slot*>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA));
  if (!self) return ::DefWindowProcW(hwnd, msg, wparam, lparam);
  switch (msg) {

  case WM_MOUSEMOVE: {
    if (self->focused_control) {
      if (const auto control_slot = self->controls.get(self->focused_control)) {
        if (auto res = control_slot->proc(msg, wparam, lparam); !res) {
          window::system.last_error = std::move(res.error().push());
          return 0;
        }
      }
    }
    const auto pt = float2(std::bit_cast<short2>(static_cast<uint32_t>(std::bit_cast<size_t>(lparam))));
    for (auto& control_slot : self->controls | std::views::reverse)
      if (control_slot.hit_test(pt)) {
        if (self->hovered_control != control_slot.id.control) {
          if (self->hovered_control) {
            const auto hovered_slot = self->controls.get(self->hovered_control);
            if (hovered_slot && hovered_slot->on_hover) hovered_slot->on_hover(*hovered_slot, false);
          }
          self->hovered_control = control_slot.id.control;
          if (control_slot.on_hover) control_slot.on_hover(control_slot, true);
        }
        return 0;
      }
    if (self->hovered_control) {
      if (auto hovered_slot = self->controls.get(self->hovered_control))
        if (hovered_slot->on_hover) hovered_slot->on_hover(*hovered_slot, false);
      self->hovered_control = {};
    }
    return 0;
  }

  case WM_LBUTTONDOWN: {
    ::SetCapture(hwnd);
    if (!self->focused_control.is_zero()) self->controls.get(self->focused_control)->proc(msg, wparam, lparam);
    return 0;
  }

  case WM_LBUTTONUP: {
    ::ReleaseCapture();
    if (!self->focused_control.is_zero()) self->controls.get(self->focused_control)->proc(msg, wparam, lparam);
    return 0;
  }

  case WM_ACTIVATE: {
    if (LOWORD(wparam) == WA_INACTIVE) window::system.focused_window = {};
    return 0;
  }

  case WM_SETFOCUS: {
    window::system.focused_window = self->id;
    break;
  }

  ////////////////////////////////////// MARK: サイズ変更

  case WM_ENTERSIZEMOVE:
    self->resizing = true;
    return 0;

  case WM_SIZE:
    self->width = LOWORD(lparam), self->height = HIWORD(lparam);
    if (self->resizing) return 0;
    if (auto res = self->_resize_rendertarget({self->width, self->height}); !res)
      window::system.last_error = std::move(res.error().push());
    return 0;

  case WM_EXITSIZEMOVE:
    self->resizing = false;
    if (auto res = self->_resize_rendertarget({self->width, self->height}); !res)
      window::system.last_error = std::move(res.error().push());
    return 0;

  ////////////////////////////////////// MARK: ウィンドウ破棄

  case WM_CLOSE:
    if (self->close_confirmation && ::MessageBoxW(hwnd, L"Close window?", L"Confirmation", MB_YESNO) == IDNO) return 0;
    return ::DestroyWindow(hwnd), 0;

  case WM_NCDESTROY:
    if (self->id.slave.is_zero()) {
      for (auto& slave_slot : self->slaves) {
        ::SetWindowLongPtrW(slave_slot.hwnd, GWLP_USERDATA, 0);
        ::DestroyWindow(slave_slot.hwnd);
      }
      self->slaves.clear();
      window::system.windows.erase(self->id.master);
      if (window::system.windows.empty()) { ::PostQuitMessage(0); }
    } else if (const auto mw = window::system.windows.get(self->id.master)) {
      mw->slaves.erase(self->id.slave);
      ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
    }
    break;
  }
  return ::DefWindowProcW(hwnd, msg, wparam, lparam);
}
} // namespace yw::window
