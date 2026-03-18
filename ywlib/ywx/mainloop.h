#pragma once
#include "ywx/ui_layout.h"
#include "ywx/ui_window.h"

namespace yw {

namespace system {
short2 cursor_pos{};
short2 cursor_delta{};
}

//////////////////////////////////////// MARK: mainloop

inline class {
public:
  enum class state { pre_initialize, running, error, quit };

private:
  state _state = state::pre_initialize;
  bool _updated = true;
  stopwatch _timer;

public:
  error_trace last_error;
  uint32_t max_messages_per_frame = 100;
  uint32_t max_frames_per_second = 1000;

  bool running() const noexcept { return _state == state::running; }
  bool error() const noexcept { return _state == state::error; }
  bool quit() const noexcept { return _state == state::quit; }

  /// returns true when drawing of windows is updated in the current frame
  bool updated() const noexcept { return _updated; }

  /// runs the mainloop
  bool operator()() {
    if (_state == state::pre_initialize) {
      _timer.start();
      _state = state::running;
      for (const auto& wid : system::primal_windows)
        if (const auto wsp = system::slot_address<ui::window>(wid); wsp && (wsp->dirty || wsp->messy)) {
          wsp->draw();
          wsp->swapchain->Present(0, 0);
          wsp->messy = wsp->dirty = false;
        }
    } else {
      _state = state::running;
      if (_updated = _timer.elapsed() >= 1.0 / max_frames_per_second) {
        _timer.restart();
        for (const auto& wid : system::primal_windows)
          if (const auto wsp = system::slot_address<ui::window>(wid); wsp && (wsp->dirty || wsp->messy)) wsp->draw();
      }
      uint32_t processed_messages = 0;
      for (MSG msg; ::PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE);) {
        if (msg.message == WM_QUIT) return _state = state::quit, false;
        ::TranslateMessage(&msg);
        ::DispatchMessageW(&msg);
        if (last_error) return _state = state::error, false;
        if (++processed_messages >= max_messages_per_frame) break;
      }
      if (_updated)
        for (const auto& wid : system::primal_windows)
          if (const auto wsp = system::slot_address<ui::window>(wid); wsp && (wsp->dirty || wsp->messy))
            wsp->swapchain->Present(0, 0), wsp->messy = wsp->dirty = false;

    }
    return _state == state::running;
  }

  /// runs the mainloop
  explicit operator bool() { return operator()(); }
} mainloop;

// //////////////////////////////////////// MARK: internal::wm_mousemove

namespace internal {
inline void wm_size(ui::window::slot& ws, WPARAM, LPARAM lp) {
  if (ws.resizing) return;
  ws.size.x = LOWORD(lp);
  ws.size.y = HIWORD(lp);
  if (auto res = ws.resize_rendertarget(ws.size); !res) {
    mainloop.last_error = std::move(res.error().push());
    return;
  }
  auto lsp = system::slot_address<ui::layout>(ws.layout_id);
  if (!lsp) {
    ws.layout_id = system::uis.add(std::make_unique<ui::layout::slot>());
    lsp = dynamic_cast<ui::layout::slot*>(system::uis.get(ws.layout_id));
    if (!lsp) {
      mainloop.last_error = unexpected_error(errors::operation_failed, "Failed to create root layout.").error();
      return;
    }
    lsp->id = ws.layout_id;
    lsp->window_id = ws.id;
    lsp->margin = {};
    lsp->bg_color = colors::transparent;
    lsp->border_color = colors::transparent;
  }
  lsp->size = float2(ws.size);
  ws.messy = true;
}

inline void wm_mousemove(ui::window::slot& ws, WPARAM wp, LPARAM lp) {
  const auto pt_short = std::bit_cast<short2>(static_cast<uint32_t>(lp & 0xFFFFFFFF));
  system::cursor_delta = pt_short - std::exchange(system::cursor_pos, pt_short);
  const auto pt = float2(pt_short);
  if (const auto fcsp = system::slot_address<ui::control>(ws.focused_control)) {
    fcsp->move_event(event::move(system::cursor_pos, system::cursor_delta));
    const bool c = (wp & MK_CONTROL) == MK_CONTROL, s = (wp & MK_SHIFT) == MK_SHIFT, a = (wp & MK_ALT) == MK_ALT;
    if ((wp & MK_LBUTTON) == MK_LBUTTON) fcsp->drag_event(event::drag(system::cursor_delta, key::lbutton, c, s, a));
    else if ((wp & MK_RBUTTON) == MK_RBUTTON) fcsp->drag_event(event::drag(system::cursor_delta, key::rbutton, c, s, a));
    else if ((wp & MK_MBUTTON) == MK_MBUTTON) fcsp->drag_event(event::drag(system::cursor_delta, key::mbutton, c, s, a));
  }
  TRACKMOUSEEVENT tme{sizeof(TRACKMOUSEEVENT), TME_LEAVE, ws.hwnd, 0};
  ::TrackMouseEvent(&tme);
  system::cursor_pos = int2(pt);
  ::ClientToScreen(ws.hwnd, reinterpret_cast<POINT*>(&system::cursor_pos));
  ui::slotid new_hcid{};
  if (const auto lsp = system::slot_address<ui::layout>(ws.layout_id)) new_hcid = lsp->hit_test(pt);
  if (ws.hovered_control) {
    if (ws.hovered_control != new_hcid) {
      if (const auto hcsp = system::slot_address<ui::control>(ws.hovered_control))
        hcsp->hover_event(event::hover(system::cursor_pos, event::hover::type::leave));
      ws.hovered_control = new_hcid;
      if (const auto hcsp = system::slot_address<ui::control>(ws.hovered_control))
        hcsp->hover_event(event::hover(system::cursor_pos, event::hover::type::enter));
    } else if (const auto hcsp = system::slot_address<ui::control>(ws.hovered_control))
      hcsp->hover_event(event::hover(system::cursor_pos, event::hover::type::move));
  } else if (const auto hcsp = system::slot_address<ui::control>(new_hcid)) {
    ws.hovered_control = new_hcid;
    hcsp->hover_event(event::hover(system::cursor_pos, event::hover::type::enter));
  }
}

inline void wm_mouseleave(ui::window::slot& ws, WPARAM wp, LPARAM lp) {
  ::GetCursorPos(reinterpret_cast<POINT*>(&system::cursor_pos));
  if (ws.hovered_control) {
    if (const auto hcsp = system::slot_address<ui::control>(ws.hovered_control))
      hcsp->hover_event(event::hover(system::cursor_pos, event::hover::type::leave));
    ws.hovered_control = {};
  }
}

inline void wm_keydown_tab(ui::window::slot& ws, WPARAM wp, LPARAM lp) {
  const auto lsp = system::slot_address<ui::layout>(ws.layout_id);
  if (!lsp) {
    ws.focused_control = {};
    return;
  }
  const bool shift = (::GetKeyState(VK_SHIFT) & 0x8000) != 0;
  int cur = shift ? static_cast<int>(lsp->controls.size()) : -1;
  ws.dirty = true;
  if (const auto fcsp = system::slot_address<ui::control>(ws.focused_control)) fcsp->focus_event(false);
  if (shift) {
    while (--cur >= 0)
      if (const auto ui_slot_p = system::slot_address<ui::control>(lsp->controls[cur]))
        if (ui_slot_p->focus_event(true)) {
          ws.focused_control = lsp->controls[cur];
          return;
        }
    ws.focused_control = {};
  } else {
    const int n = static_cast<int>(lsp->controls.size());
    while (++cur < n)
      if (const auto ui_slot_p = system::slot_address<ui::control>(lsp->controls[cur]))
        if (ui_slot_p->focus_event(true)) {
          ws.focused_control = lsp->controls[cur];
          return;
        }
    ws.focused_control = {};
  }
}

inline void wm_lbuttondown(ui::window::slot& ws, WPARAM wp, LPARAM lp) {
  if (ws.capture_count++ == 0) ::SetCapture(ws.hwnd);
  ws.captured_key = key::lbutton;
  ws.dirty = true;
  const bool c = (wp & MK_CONTROL) == MK_CONTROL, s = (wp & MK_SHIFT) == MK_SHIFT, a = (wp & MK_ALT) == MK_ALT;
  // priority: focused_control > hovered_control > others
  if (const auto p = system::slot_address<ui::control>(ws.focused_control)) {
    if (ws.focused_control == ws.hovered_control) { // inside focused_control, so send event to it
      ws.captured_control = p->id;
      p->button_event(event::button(system::cursor_pos, key::lbutton, true, c, s, a));
      return;
    } else { // outside of focused_control, so unfocus it
      ws.focused_control = {};
      p->focus_event(false);
    }
  }
  // send event to hovered_control and focus it, if exists
  if (const auto p = system::slot_address<ui::control>(ws.hovered_control)) {
    ws.captured_control = p->id;
    p->button_event(event::button(system::cursor_pos, key::lbutton, true, c, s, a));
    if (p->focus_event(true)) ws.focused_control = p->id;
    else ws.focused_control = {};
  } else ws.captured_control = {}, ws.focused_control = {};
}

inline void wm_lbuttonup(ui::window::slot& ws, WPARAM wp, LPARAM lp) {
  ws.capture_count = yw::max(0, ws.capture_count - 1);
  if (ws.capture_count == 0) ::ReleaseCapture();
  const bool c = (wp & MK_CONTROL) == MK_CONTROL, s = (wp & MK_SHIFT) == MK_SHIFT, a = (wp & MK_ALT) == MK_ALT;
  if (const auto p = system::slot_address<ui::control>(ws.captured_control); p && ws.captured_key == key::lbutton) {
    p->button_event(event::button(system::cursor_pos, key::lbutton, false, c, s, a));
    if (ws.captured_control == ws.hovered_control) p->click_event(event::button(system::cursor_pos, key::lbutton, false, c, s, a));
  }
  ws.captured_control = {};
  ws.captured_key = {};
  ws.dirty = true;
}

inline void wm_rbuttondown(ui::window::slot& ws, WPARAM wp, LPARAM lp) {
  if (ws.capture_count++ == 0) ::SetCapture(ws.hwnd);
  ws.captured_key = key::rbutton;
  ws.dirty = true;
  const bool c = (wp & MK_CONTROL) == MK_CONTROL, s = (wp & MK_SHIFT) == MK_SHIFT, a = (wp & MK_ALT) == MK_ALT;
  if (const auto p = system::slot_address<ui::control>(ws.focused_control)) {
    if (ws.focused_control == ws.hovered_control) {
      ws.captured_control = p->id;
      p->button_event(event::button(system::cursor_pos, key::rbutton, true, c, s, a));
      return;
    } else {
      ws.focused_control = {};
      p->focus_event(false);
    }
  }
  if (const auto p = system::slot_address<ui::control>(ws.hovered_control)) {
    ws.captured_control = p->id;
    p->button_event(event::button(system::cursor_pos, key::rbutton, true, c, s, a));
    if (p->focus_event(true)) ws.focused_control = p->id;
    else ws.focused_control = {};
  } else ws.captured_control = {}, ws.focused_control = {};
}

inline void wm_rbuttonup(ui::window::slot& ws, WPARAM wp, LPARAM lp) {
  ws.capture_count = yw::max(0, ws.capture_count - 1);
  if (ws.capture_count == 0) ::ReleaseCapture();
  const bool c = (wp & MK_CONTROL) == MK_CONTROL, s = (wp & MK_SHIFT) == MK_SHIFT, a = (wp & MK_ALT) == MK_ALT;
  if (const auto p = system::slot_address<ui::control>(ws.captured_control); p && ws.captured_key == key::rbutton) {
    p->button_event(event::button(system::cursor_pos, key::rbutton, false, c, s, a));
    if (ws.captured_control == ws.hovered_control) p->click_event(event::button(system::cursor_pos, key::rbutton, false, c, s, a));
  }
  ws.captured_control = {};
  ws.captured_key = {};
  ws.dirty = true;
}

inline void wm_mbuttondown(ui::window::slot& ws, WPARAM wp, LPARAM lp) {
  if (ws.capture_count++ == 0) ::SetCapture(ws.hwnd);
  ws.captured_key = key::mbutton;
  ws.dirty = true;
  const bool c = (wp & MK_CONTROL) == MK_CONTROL, s = (wp & MK_SHIFT) == MK_SHIFT, a = (wp & MK_ALT) == MK_ALT;
  if (const auto p = system::slot_address<ui::control>(ws.focused_control)) {
    if (ws.focused_control == ws.hovered_control) {
      ws.captured_control = p->id;
      p->button_event(event::button(system::cursor_pos, key::mbutton, true, c, s, a));
      return;
    } else {
      ws.focused_control = {};
      p->focus_event(false);
    }
  }
  if (const auto p = system::slot_address<ui::control>(ws.hovered_control)) {
    ws.captured_control = p->id;
    p->button_event(event::button(system::cursor_pos, key::mbutton, true, c, s, a));
    if (p->focus_event(true)) ws.focused_control = p->id;
    else ws.focused_control = {};
  } else ws.captured_control = {}, ws.focused_control = {};
}

inline void wm_mbuttonup(ui::window::slot& ws, WPARAM wp, LPARAM lp) {
  ws.capture_count = yw::max(0, ws.capture_count - 1);
  if (ws.capture_count == 0) ::ReleaseCapture();
  const bool c = (wp & MK_CONTROL) == MK_CONTROL, s = (wp & MK_SHIFT) == MK_SHIFT, a = (wp & MK_ALT) == MK_ALT;
  if (const auto p = system::slot_address<ui::control>(ws.captured_control); p && ws.captured_key == key::mbutton) {
    p->button_event(event::button(system::cursor_pos, key::mbutton, false, c, s, a));
    if (ws.captured_control == ws.hovered_control) p->click_event(event::button(system::cursor_pos, key::mbutton, false, c, s, a));
  }
  ws.captured_control = {};
  ws.captured_key = {};
  ws.dirty = true;
}

inline void wm_xbuttondown(ui::window::slot& ws, WPARAM wp, LPARAM lp) {
  const bool x1 = HIWORD(wp) == XBUTTON1;
  const auto code = x1 ? key::xbutton1 : key::xbutton2;
  if (ws.capture_count++ == 0) ::SetCapture(ws.hwnd);
  ws.captured_key = code;
  ws.dirty = true;
  if (const auto p = system::slot_address<ui::control>(ws.focused_control)) {
    if (ws.focused_control == ws.hovered_control) {
      ws.captured_control = p->id;
      p->button_event(event::button(code, true, wp, lp));
      return;
    } else {
      ws.focused_control = {};
      p->focus_event(false);
    }
  }
  if (const auto p = system::slot_address<ui::control>(ws.hovered_control)) {
    ws.captured_control = p->id;
    p->button_event(event::button(code, true, wp, lp));
    if (p->focus_event(true)) ws.focused_control = p->id;
    else ws.focused_control = {};
  } else ws.captured_control = {}, ws.focused_control = {};
}

inline void wm_xbuttonup(ui::window::slot& ws, WPARAM wp, LPARAM lp) {
  const bool x1 = HIWORD(wp) == XBUTTON1;
  const auto code = x1 ? key::xbutton1 : key::xbutton2;
  ws.capture_count = yw::max(0, ws.capture_count - 1);
  if (ws.capture_count == 0) ::ReleaseCapture();
  if (const auto p = system::slot_address<ui::control>(ws.captured_control); p && ws.captured_key == code) {
    p->button_event(event::button(code, false, wp, lp));
    if (ws.captured_control == ws.hovered_control) p->click_event(event::button(code, false, wp, lp));
  }
  ws.captured_control = {};
  ws.captured_key = {};
  ws.dirty = true;
}
} // namespace internal

//////////////////////////////////////// MARK: wclass::proc

inline LRESULT CALLBACK decltype(wclass)::proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
  const auto wsid = std::bit_cast<ui::slotid>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA));
  auto wsp = system::slot_address<ui::window::slot>(wsid);
  if (!wsp) return ::DefWindowProcW(hwnd, msg, wp, lp);

  switch (msg) {
  case WM_MOUSEMOVE: internal::wm_mousemove(*wsp, wp, lp); return 0;
  case WM_MOUSELEAVE: internal::wm_mouseleave(*wsp, wp, lp); return 0;

  case WM_KEYDOWN:
    if (wp == VK_TAB) internal::wm_keydown_tab(*wsp, wp, lp);
    else if (const auto p = system::slot_address<ui::control>(wsp->focused_control))
      p->key_event(event::key(true, wp, lp));
    return 0;
  case WM_KEYUP:
    if (const auto p = system::slot_address<ui::control>(wsp->focused_control)) p->key_event(event::key(false, wp, lp));
    return 0;

  case WM_CHAR:
    if (const auto p = system::slot_address<ui::control>(wsp->focused_control)) p->char_event(static_cast<wchar_t>(wp));
    return 0;

  case WM_LBUTTONDOWN: internal::wm_lbuttondown(*wsp, wp, lp); return 0;
  case WM_LBUTTONUP: internal::wm_lbuttonup(*wsp, wp, lp); return 0;

  case WM_RBUTTONDOWN: internal::wm_rbuttondown(*wsp, wp, lp); return 0;
  case WM_RBUTTONUP: internal::wm_rbuttonup(*wsp, wp, lp); return 0;

  case WM_MBUTTONDOWN: internal::wm_mbuttondown(*wsp, wp, lp); return 0;
  case WM_MBUTTONUP: internal::wm_mbuttonup(*wsp, wp, lp); return 0;

  case WM_XBUTTONDOWN: internal::wm_xbuttondown(*wsp, wp, lp); return 0;
  case WM_XBUTTONUP: internal::wm_xbuttonup(*wsp, wp, lp); return 0;

  case WM_KILLFOCUS:
    wsp->captured_control = {};
    wsp->captured_key = {};
    wsp->capture_count = 0;
    ::ReleaseCapture();
    return 0;

  case WM_SIZE: internal::wm_size(*wsp, wp, lp); return 0;

  case WM_MOVE:
    wsp->pos = int2(static_cast<int16_t>(LOWORD(lp)), static_cast<int16_t>(HIWORD(lp))) - wsp->margin.xy();
    return 0;

  case WM_ENTERSIZEMOVE: wsp->resizing = true; return 0;

  case WM_EXITSIZEMOVE:
    wsp->resizing = false;
    if (RECT cr{}; ::GetClientRect(hwnd, &cr)) {
      const auto cx = static_cast<uint16_t>(yw::max(0L, static_cast<long>(cr.right - cr.left)));
      const auto cy = static_cast<uint16_t>(yw::max(0L, static_cast<long>(cr.bottom - cr.top)));
      internal::wm_size(*wsp, 0, MAKELPARAM(cx, cy));
    } else internal::wm_size(*wsp, 0, MAKELPARAM(wsp->size.x, wsp->size.y));
    return 0;

  case WM_CLOSE:
    if (wsp->on_close && !wsp->on_close()) return 0;
    ::DestroyWindow(hwnd);
    return 0;

  case WM_NCDESTROY:
    // Delete root layout/control tree belonging to this master window
    system::uis.erase(wsp->layout_id);
    const auto id = wsp->id;
    system::uis.erase(id);
    std::erase(system::primal_windows, id);
    ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
    if (system::primal_windows.empty()) { ::PostQuitMessage(0); }
    break;
  }
  return ::DefWindowProcW(hwnd, msg, wp, lp);
}
} // namespace yw
