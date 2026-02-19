#pragma once
#include "ywx/window/slot.h"

namespace yw::window {

inline LRESULT __stdcall decltype(system)::proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  auto self = reinterpret_cast<window::slot*>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA));
  if (!self) return ::DefWindowProcW(hwnd, msg, wparam, lparam);
  switch (msg) {

  /// \note MOUSEMOVE
  /// 1. フォーカスされたコントロールがある場合、そのコントロールのprocを呼ぶ。
  /// 2. 移動先のコントロールを調べ、hovered_controlと異なるなら処理する。
  /// 3. 1の戻り値を返す
  case WM_MOUSEMOVE:{
    std::expected<bool, error_trace> result = true;
    if (self->focused_control) {
      const auto control_slot = self->controls.get(self->focused_control);
      if (control_slot) result = control_slot->proc(msg, wparam, lparam);
    }
    const auto pt = float2(std::bit_cast<short2>(static_cast<uint32_t>(std::bit_cast<size_t>(lparam))));
    for (auto& control_slot : self->controls | std::views::reverse)
      if (control_slot.hit_test(pt)) {
        if (self->hovered_control != control_slot.id.control) {
          if (self->hovered_control) {
            if (auto hovered_slot = self->controls.get(self->hovered_control))
              if (hovered_slot->on_hover) hovered_slot->on_hover(*hovered_slot, false);
          }
          self->hovered_control = control_slot.id.control;
          if (control_slot.on_hover) control_slot.on_hover(control_slot, true);
        }
        break;
      }
    return result;
  }

  /// \note KEYDOWN
  /// 1. フォーカスされたコントロールがある場合、そのコントロールのprocを呼ぶ。
  case WM_KEYDOWN: {
    if (self->focused_control) {
      const auto control_slot = self->controls.get(self->focused_control);
      if (control_slot) return control_slot->proc(msg, wparam, lparam) ? 0 : ::DefWindowProcW(hwnd, msg, wparam, lparam);
    }
    break;
  }

  // MOUSE INPUT
  case WM_LBUTTONDOWN:
    ::SetCapture(hwnd);
    if (!self->focused_control.is_zero()) self->controls.get(self->focused_control)->proc(msg, wparam, lparam);
    break;
  case WM_LBUTTONUP:
    ::ReleaseCapture();
    if (!self->focused_control.is_zero()) self->controls.get(self->focused_control)->proc(msg, wparam, lparam);
    break;

  // FOCUS
  case WM_ACTIVATE:
    if (LOWORD(wparam) == WA_INACTIVE) window::system.focused_window = {};
    break;
  case WM_SETFOCUS: window::system.focused_window = self->id; break;

  // RESIZE
  case WM_SIZE:
    if (auto res = self->_resize_rendertarget(uint2(LOWORD(lparam), HIWORD(lparam))); !res)
      window::system.last_error = std::move(res.error().push());
    return 0;

  // CLOSE / DESTROY
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
}
