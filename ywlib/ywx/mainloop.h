#pragma once
#include "ywx/window.h"

namespace yw {

//////////////////////////////////////// MARK: mainloop

inline class {
public:
  enum class state { running, error, quit };

private:
  uint64_t _frame_count = 0;
  state _state = state::running;
  bool _updating = true;
  stopwatch _timer;

public:
  error_trace last_error;
  uint32_t max_messages_per_frame = 100;
  uint32_t max_frames_per_second = 1000;

  uint64_t frame_count() const noexcept { return _frame_count; }
  bool running() const noexcept { return _state == state::running; }
  bool error() const noexcept { return _state == state::error; }
  bool quit() const noexcept { return _state == state::quit; }

  /// returns true when drawing of windows is updated in the current frame
  bool updating() const noexcept { return _updating; }

  /// runs the mainloop
  bool operator()() {
    if (_updating) {
      for (const auto& wid : system::primal_windows)
        if (const auto wsp = system::windows.get(wid)) wsp->draw();
      _timer.restart();
    }
    ++_frame_count;
    _state = state::running;
    uint32_t processed_messages = 0;
    for (MSG msg; ::PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE);) {
      if (msg.message == WM_QUIT) return _state = state::quit, false;
      ::TranslateMessage(&msg);
      ::DispatchMessageW(&msg);
      if (last_error) return _state = state::error, false;
      if (++processed_messages >= max_messages_per_frame) break;
    }
    _updating = _timer.elapsed() >= 1.0 / max_frames_per_second;
    return _state == state::running;
  }

  /// runs the mainloop
  explicit operator bool() { return operator()(); }
} mainloop;

// //////////////////////////////////////// MARK: internal::wm_mousemove

namespace internal {
inline void wm_mousemove(window::slot& ws, WPARAM wp, LPARAM lp) {
  if (const auto fui_slot_p = system::controls.get(ws.focused_control)) fui_slot_p->move_event(event::move(wp, lp));
  TRACKMOUSEEVENT tme{sizeof(TRACKMOUSEEVENT), TME_LEAVE, ws.hwnd, 0};
  ::TrackMouseEvent(&tme);
  const auto pt = float2(std::bit_cast<short2>(static_cast<uint32_t>(lp & 0xFFFFFFFF)));
  system::cursor_pos = int2(pt);
  ::ClientToScreen(ws.hwnd, reinterpret_cast<POINT*>(&system::cursor_pos));
  control::slotid new_hovered_control{};
  for (const auto& cid : ws.controls | std::views::reverse)
    if (const auto csp = system::controls.get(cid); csp && csp->visible && csp->hit_test(pt)) {
      new_hovered_control = cid;
      break;
    }
  if (ws.hovered_control) {
    if (ws.hovered_control != new_hovered_control) {
      if (const auto hcsp = system::controls.get(ws.hovered_control))
        hcsp->hover_event(event::hover((wp & 0xff) | 0x200, lp)); // leave
      ws.hovered_control = new_hovered_control;
      if (const auto hcsp = system::controls.get(ws.hovered_control))
        hcsp->hover_event(event::hover((wp & 0xff) | 0x100, lp)); // enter
    } else if (const auto hcsp = system::controls.get(ws.hovered_control))
      hcsp->hover_event(event::hover((wp & 0xff) | 0x400, lp)); // move
  } else if (const auto hcsp = system::controls.get(new_hovered_control)) {
    ws.hovered_control = new_hovered_control;
    hcsp->hover_event(event::hover((wp & 0xff) | 0x100, lp)); // enter
  }
}

inline void wm_mouseleave(window::slot& ws, WPARAM wp, LPARAM lp) {
  ::GetCursorPos(reinterpret_cast<POINT*>(&system::cursor_pos));
  if (ws.hovered_control) {
    if (const auto hcsp = system::controls.get(ws.hovered_control))
      hcsp->hover_event(event::hover((wp & 0xff) | 0x200, lp)); // leave
    ws.hovered_control = {};
  }
}

inline void wm_keydown_tab(window::slot& ws, WPARAM wp, LPARAM lp) {
  const bool shift = (::GetKeyState(VK_SHIFT) & 0x8000) != 0;
  int cur = shift ? static_cast<int>(ws.controls.size()) : -1;
  ws.dirty = true;
  if (const auto fcsp = system::controls.get(ws.focused_control)) fcsp->focus_event(false);
  if (shift) {
    while (--cur >= 0)
      if (const auto ui_slot_p = system::controls.get(ws.controls[cur]))
        if (ui_slot_p->focus_event(true)) {
          ws.focused_control = ws.controls[cur];
          return;
        }
    ws.focused_control = {};
  } else {
    const int n = static_cast<int>(ws.controls.size());
    while (++cur < n)
      if (const auto ui_slot_p = system::controls.get(ws.controls[cur]))
        if (ui_slot_p->focus_event(true)) {
          ws.focused_control = ws.controls[cur];
          return;
        }
    ws.focused_control = {};
  }
}

inline void wm_lbuttondown(window::slot& ws, WPARAM wp, LPARAM lp) {
  if (ws.capture_count++ == 0) ::SetCapture(ws.hwnd);
  ws.captured_key = key::lbutton;
  ws.dirty = true;
  // priority: focused_control > hovered_control > others
  if (const auto p = system::controls.get(ws.focused_control)) {
    if (ws.focused_control == ws.hovered_control) { // inside focused_control, so send event to it
      ws.captured_control = p->id;
      p->button_event(event::button(key::lbutton, true, wp, lp));
      return;
    } else { // outside of focused_control, so unfocus it
      ws.focused_control = {};
      p->focus_event(false);
    }
  }
  // send event to hovered_control and focus it, if exists
  if (const auto p = system::controls.get(ws.hovered_control)) {
    ws.captured_control = p->id;
    p->button_event(event::button(key::lbutton, true, wp, lp));
    if (p->focus_event(true)) ws.focused_control = p->id;
    else ws.focused_control = {};
  } else ws.captured_control = {}, ws.focused_control = {};
}

inline void wm_lbuttonup(window::slot& ws, WPARAM wp, LPARAM lp) {
  ws.capture_count = yw::max(0, ws.capture_count - 1);
  if (ws.capture_count == 0) ::ReleaseCapture();
  if (const auto p = system::controls.get(ws.captured_control); p && ws.captured_key == key::lbutton) {
    p->button_event(event::button(key::lbutton, false, wp, lp));
    if (ws.captured_control == ws.hovered_control)
      p->click_event(event::button(key::lbutton, false, wp, lp));
  }
  ws.captured_control = {};
  ws.captured_key = {};
  ws.dirty = true;
}

inline void wm_rbuttondown(window::slot& ws, WPARAM wp, LPARAM lp) {
  if (ws.capture_count++ == 0) ::SetCapture(ws.hwnd);
  ws.captured_key = key::rbutton;
  ws.dirty = true;
  if (const auto p = system::controls.get(ws.focused_control)) {
    if (ws.focused_control == ws.hovered_control) {
      ws.captured_control = p->id;
      p->button_event(event::button(key::rbutton, true, wp, lp));
      return;
    } else {
      ws.focused_control = {};
      p->focus_event(false);
    }
  }
  if (const auto p = system::controls.get(ws.hovered_control)) {
    ws.captured_control = p->id;
    p->button_event(event::button(key::rbutton, true, wp, lp));
    if (p->focus_event(true)) ws.focused_control = p->id;
    else ws.focused_control = {};
  } else ws.captured_control = {}, ws.focused_control = {};
}

inline void wm_rbuttonup(window::slot& ws, WPARAM wp, LPARAM lp) {
  ws.capture_count = yw::max(0, ws.capture_count - 1);
  if (ws.capture_count == 0) ::ReleaseCapture();
  if (const auto p = system::controls.get(ws.captured_control); p && ws.captured_key == key::rbutton) {
    p->button_event(event::button(key::rbutton, false, wp, lp));
    if (ws.captured_control == ws.hovered_control)
      p->click_event(event::button(key::rbutton, false, wp, lp));
  }
  ws.captured_control = {};
  ws.captured_key = {};
  ws.dirty = true;
}

inline void wm_mbuttondown(window::slot& ws, WPARAM wp, LPARAM lp) {
  if (ws.capture_count++ == 0) ::SetCapture(ws.hwnd);
  ws.captured_key = key::mbutton;
  ws.dirty = true;
  if (const auto p = system::controls.get(ws.focused_control)) {
    if (ws.focused_control == ws.hovered_control) {
      ws.captured_control = p->id;
      p->button_event(event::button(key::mbutton, true, wp, lp));
      return;
    } else {
      ws.focused_control = {};
      p->focus_event(false);
    }
  }
  if (const auto p = system::controls.get(ws.hovered_control)) {
    ws.captured_control = p->id;
    p->button_event(event::button(key::mbutton, true, wp, lp));
    if (p->focus_event(true)) ws.focused_control = p->id;
    else ws.focused_control = {};
  } else ws.captured_control = {}, ws.focused_control = {};
}

inline void wm_mbuttonup(window::slot& ws, WPARAM wp, LPARAM lp) {
  ws.capture_count = yw::max(0, ws.capture_count - 1);
  if (ws.capture_count == 0) ::ReleaseCapture();
  if (const auto p = system::controls.get(ws.captured_control); p && ws.captured_key == key::mbutton) {
    p->button_event(event::button(key::mbutton, false, wp, lp));
    if (ws.captured_control == ws.hovered_control)
      p->click_event(event::button(key::mbutton, false, wp, lp));
  }
  ws.captured_control = {};
  ws.captured_key = {};
  ws.dirty = true;
}

inline void wm_xbuttondown(window::slot& ws, WPARAM wp, LPARAM lp) {
  const bool x1 = HIWORD(wp) == XBUTTON1;
  const auto code = x1 ? key::xbutton1 : key::xbutton2;
  if (ws.capture_count++ == 0) ::SetCapture(ws.hwnd);
  ws.captured_key = code;
  ws.dirty = true;
  if (const auto p = system::controls.get(ws.focused_control)) {
    if (ws.focused_control == ws.hovered_control) {
      ws.captured_control = p->id;
      p->button_event(event::button(code, true, wp, lp));
      return;
    } else {
      ws.focused_control = {};
      p->focus_event(false);
    }
  }
  if (const auto p = system::controls.get(ws.hovered_control)) {
    ws.captured_control = p->id;
    p->button_event(event::button(code, true, wp, lp));
    if (p->focus_event(true)) ws.focused_control = p->id;
    else ws.focused_control = {};
  } else ws.captured_control = {}, ws.focused_control = {};
}

inline void wm_xbuttonup(window::slot& ws, WPARAM wp, LPARAM lp) {
  const bool x1 = HIWORD(wp) == XBUTTON1;
  const auto code = x1 ? key::xbutton1 : key::xbutton2;
  ws.capture_count = yw::max(0, ws.capture_count - 1);
  if (ws.capture_count == 0) ::ReleaseCapture();
  if (const auto p = system::controls.get(ws.captured_control); p && ws.captured_key == code) {
    p->button_event(event::button(code, false, wp, lp));
    if (ws.captured_control == ws.hovered_control)
      p->click_event(event::button(code, false, wp, lp));
  }
  ws.captured_control = {};
  ws.captured_key = {};
  ws.dirty = true;
}
} // namespace internal

//////////////////////////////////////// MARK: wclass::proc

inline LRESULT CALLBACK decltype(wclass)::proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
  const auto ws_id = std::bit_cast<window::slotid>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA));
  auto wsp = system::windows.get(ws_id);
  if (!wsp) return ::DefWindowProcW(hwnd, msg, wp, lp);

  switch (msg) {
  case WM_MOUSEMOVE: internal::wm_mousemove(*wsp, wp, lp); return 0;
  case WM_MOUSELEAVE: internal::wm_mouseleave(*wsp, wp, lp); return 0;

  case WM_KEYDOWN:
    if (wp == VK_TAB) internal::wm_keydown_tab(*wsp, wp, lp);
    else if (const auto p = system::controls.get(wsp->focused_control)) p->key_event(event::key(true, wp, lp));
    return 0;
  case WM_KEYUP:
    if (const auto p = system::controls.get(wsp->focused_control)) p->key_event(event::key(false, wp, lp));
    return 0;

  case WM_CHAR:
    if (const auto p = system::controls.get(wsp->focused_control)) p->char_event(static_cast<wchar_t>(wp));
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

  case WM_SIZE:
    wsp->size.x = LOWORD(lp);
    wsp->size.y = HIWORD(lp);
    if (wsp->resizing) return 0;
    if (auto res = wsp->resize_rendertarget({wsp->size.x, wsp->size.y}); !res)
      mainloop.last_error = std::move(res.error().push());
    return 0;

  case WM_MOVE:
    wsp->pos = int2(static_cast<int16_t>(LOWORD(lp)), static_cast<int16_t>(HIWORD(lp))) - wsp->margin.xy();
    return 0;

  case WM_ENTERSIZEMOVE: wsp->resizing = true; return 0;

  case WM_EXITSIZEMOVE:
    wsp->resizing = false;
    if (auto res = wsp->resize_rendertarget({wsp->size.x, wsp->size.y}); !res)
      mainloop.last_error = std::move(res.error().push());
    print(wsp->size);
    [&](RECT r) {
      ::GetClientRect(wsp->hwnd, &r);
      print(int2(r.right, r.bottom));
    }({});
    return 0;

  case WM_CLOSE:
    if (wsp->on_close && !wsp->on_close()) return 0;
    ::DestroyWindow(hwnd);
    return 0;

  case WM_NCDESTROY:
    // Delete all UIs belonging to this master window
    for (auto cid : wsp->controls) system::controls.erase(cid);
    const auto id = wsp->id;
    system::windows.erase(id);
    std::erase(system::primal_windows, id);
    ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
    if (system::primal_windows.empty()) { ::PostQuitMessage(0); }
    break;
  }
  return ::DefWindowProcW(hwnd, msg, wp, lp);
}
} // namespace yw
