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
      for (const auto& id : system::windows) {
        const auto slot = ui::windows.get(id);
        if (slot) slot->draw();
      }
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
inline void wm_mousemove(ui::window::slot& w_slot, WPARAM wp, LPARAM lp) {
  if (const auto fui_slot_p = ui::system::controls.get(w_slot.focused_ui)) fui_slot_p->move_event(event::move(wp, lp));
  TRACKMOUSEEVENT tme{sizeof(TRACKMOUSEEVENT), TME_LEAVE, w_slot.hwnd, 0};
  ::TrackMouseEvent(&tme);
  const auto pt = float2(std::bit_cast<short2>(static_cast<uint32_t>(lp & 0xFFFFFFFF)));
  ui::system::cursor_pos = int2(pt);
  ::ClientToScreen(w_slot.hwnd, reinterpret_cast<POINT*>(&ui::system::cursor_pos));
  if (const auto hit_test_reuslt = w_slot.hit_test(pt)) {
    if (w_slot.hovered_ui != hit_test_reuslt) {
      if (w_slot.hovered_ui)
        if (const auto hui_slot_p = ui::system::controls.get(w_slot.hovered_ui))
          hui_slot_p->hover_event(event::hover((wp & 0xff) | 0x200, lp)); // leave
      w_slot.hovered_ui = hit_test_reuslt;
      if (const auto ui_slot_p = ui::system::controls.get(w_slot.hovered_ui))
        ui_slot_p->hover_event(event::hover((wp & 0xff) | 0x100, lp)); // enter
    } else if (const auto ui_slot_p = ui::system::controls.get(w_slot.hovered_ui))
      ui_slot_p->hover_event(event::hover((wp & 0xff) | 0x400, lp)); // move
    return;
  }

  for (auto ui_slot_id : w_slot.children | std::views::reverse) {
    if (const auto ui_slot_p = ui::system::controls.get(ui_slot_id);
      ui_slot_p && ui_slot_p->visible && ui_slot_p->hit_test(pt)) {
      if (w_slot.hovered_ui != ui_slot_p->id) {
        if (w_slot.hovered_ui)
          if (const auto hui_slot_p = ui::system::controls.get(w_slot.hovered_ui))
            hui_slot_p->hover_event(event::hover((wp & 0xff) | 0x200, lp)); // leave
        w_slot.hovered_ui = ui_slot_p->id;
        ui_slot_p->hover_event(event::hover((wp & 0xff) | 0x100, lp)); // enter
      } else ui_slot_p->hover_event(event::hover((wp & 0xff) | 0x400, lp)); // move
      return;
    }
  }
  if (w_slot.hovered_ui) {
    if (auto ui_slot_p = ui::system::controls.get(w_slot.hovered_ui))
      ui_slot_p->hover_event(event::hover((wp & 0xff) | 0x200, lp)); // leave
    w_slot.hovered_ui = {};
  }
}

inline void wm_mouseleave(ui::window::slot& w_slot, WPARAM wp, LPARAM lp) {
  ::GetCursorPos(reinterpret_cast<POINT*>(&system::cursor_pos));
  if (w_slot.hovered_ui) {
    if (auto ui_slot_p = ui::system::controls.get(w_slot.hovered_ui))
      ui_slot_p->hover_event(event::hover((wp & 0xff) | 0x200, lp)); // leave
    w_slot.hovered_ui = {};
  }
}

// inline void wm_keydown_tab(ui::window::slot& w_slot, WPARAM wp, LPARAM lp) {
//   const bool shift = (::GetKeyState(VK_SHIFT) & 0x8000) != 0;
//   const auto& uis = w_slot.uis;
//   int cur = shift ? static_cast<int>(uis.size()) : -1;
//   w_slot.dirty = true;
//   if (w_slot.focused_ui) {
//     for (size_t i{}; i < uis.size(); ++i)
//       if (uis[i] == w_slot.focused_ui) {
//         cur = static_cast<int>(i);
//         break;
//       }
//     if (const auto fui_slot_p = ui::system::controls.get(w_slot.focused_ui)) fui_slot_p->focus_event(false);
//   }
//   if (shift) {
//     while (--cur >= 0)
//       if (const auto ui_slot_p = ui::system::controls.get(uis[cur]))
//         if (ui_slot_p->focus_event(true)) {
//           w_slot.focused_ui = uis[cur];
//           return;
//         }
//     w_slot.focused_ui = {};
//   } else {
//     const int n = static_cast<int>(uis.size());
//     while (++cur < n)
//       if (const auto ui_slot_p = ui::system::controls.get(uis[cur]))
//         if (ui_slot_p->focus_event(true)) {
//           w_slot.focused_ui = uis[cur];
//           return;
//         }
//     w_slot.focused_ui = {};
//   }
// }

// inline void wm_lbuttondown(ui::window::slot& w_slot, WPARAM wp, LPARAM lp) {
//   if (w_slot.capture_count++ == 0) ::SetCapture(w_slot.hwnd);
//   w_slot.captured_key = key::lbutton;
//   w_slot.dirty = true;
//   // priority: focused_ui > hovered_ui > others
//   if (const auto p = ui::system::controls.get(w_slot.focused_ui)) {
//     if (w_slot.focused_ui == w_slot.hovered_ui) { // inside focused_ui, so send event to it
//       w_slot.captured_ui = p->id;
//       p->button_event(event::button(key::lbutton, true, wp, lp));
//       return;
//     } else { // outside of focused_ui, so unfocus it
//       w_slot.focused_ui = {};
//       p->focus_event(false);
//     }
//   }
//   // send event to hovered_ui and focus it, if exists
//   if (const auto p = ui::system::controls.get(w_slot.hovered_ui)) {
//     w_slot.captured_ui = p->id;
//     p->button_event(event::button(key::lbutton, true, wp, lp));
//     if (p->focus_event(true)) w_slot.focused_ui = p->id;
//     else w_slot.focused_ui = {};
//   } else w_slot.captured_ui = {}, w_slot.focused_ui = {};
// }

// inline void wm_lbuttonup(ui::window::slot& w_slot, WPARAM wp, LPARAM lp) {
//   w_slot.capture_count = yw::max(0, w_slot.capture_count - 1);
//   if (w_slot.capture_count == 0) ::ReleaseCapture();
//   if (const auto p = ui::system::controls.get(w_slot.captured_ui); p && w_slot.captured_key == key::lbutton) {
//     p->button_event(event::button(key::lbutton, false, wp, lp));
//     if (w_slot.captured_ui == w_slot.hovered_ui)
//       p->click_event(event::button(key::lbutton, false, wp, lp));
//   }
//   w_slot.captured_ui = {};
//   w_slot.captured_key = {};
//   w_slot.dirty = true;
// }

// inline void wm_rbuttondown(ui::window::slot& w_slot, WPARAM wp, LPARAM lp) {
//   if (w_slot.capture_count++ == 0) ::SetCapture(w_slot.hwnd);
//   w_slot.captured_key = key::rbutton;
//   w_slot.dirty = true;
//   if (const auto p = ui::system::controls.get(w_slot.focused_ui)) {
//     if (w_slot.focused_ui == w_slot.hovered_ui) {
//       w_slot.captured_ui = p->id;
//       p->button_event(event::button(key::rbutton, true, wp, lp));
//       return;
//     } else {
//       w_slot.focused_ui = {};
//       p->focus_event(false);
//     }
//   }
//   if (const auto p = ui::system::controls.get(w_slot.hovered_ui)) {
//     w_slot.captured_ui = p->id;
//     p->button_event(event::button(key::rbutton, true, wp, lp));
//     if (p->focus_event(true)) w_slot.focused_ui = p->id;
//     else w_slot.focused_ui = {};
//   } else w_slot.captured_ui = {}, w_slot.focused_ui = {};
// }

// inline void wm_rbuttonup(ui::window::slot& w_slot, WPARAM wp, LPARAM lp) {
//   w_slot.capture_count = yw::max(0, w_slot.capture_count - 1);
//   if (w_slot.capture_count == 0) ::ReleaseCapture();
//   if (const auto p = ui::system::controls.get(w_slot.captured_ui); p && w_slot.captured_key == key::rbutton) {
//     p->button_event(event::button(key::rbutton, false, wp, lp));
//     if (w_slot.captured_ui == w_slot.hovered_ui)
//       p->click_event(event::button(key::rbutton, false, wp, lp));
//   }
//   w_slot.captured_ui = {};
//   w_slot.captured_key = {};
//   w_slot.dirty = true;
// }

// inline void wm_mbuttondown(ui::window::slot& w_slot, WPARAM wp, LPARAM lp) {
//   if (w_slot.capture_count++ == 0) ::SetCapture(w_slot.hwnd);
//   w_slot.captured_key = key::mbutton;
//   w_slot.dirty = true;
//   if (const auto p = ui::system::controls.get(w_slot.focused_ui)) {
//     if (w_slot.focused_ui == w_slot.hovered_ui) {
//       w_slot.captured_ui = p->id;
//       p->button_event(event::button(key::mbutton, true, wp, lp));
//       return;
//     } else {
//       w_slot.focused_ui = {};
//       p->focus_event(false);
//     }
//   }
//   if (const auto p = ui::system::controls.get(w_slot.hovered_ui)) {
//     w_slot.captured_ui = p->id;
//     p->button_event(event::button(key::mbutton, true, wp, lp));
//     if (p->focus_event(true)) w_slot.focused_ui = p->id;
//     else w_slot.focused_ui = {};
//   } else w_slot.captured_ui = {}, w_slot.focused_ui = {};
// }

// inline void wm_mbuttonup(ui::window::slot& w_slot, WPARAM wp, LPARAM lp) {
//   w_slot.capture_count = yw::max(0, w_slot.capture_count - 1);
//   if (w_slot.capture_count == 0) ::ReleaseCapture();
//   if (const auto p = ui::system::controls.get(w_slot.captured_ui); p && w_slot.captured_key == key::mbutton) {
//     p->button_event(event::button(key::mbutton, false, wp, lp));
//     if (w_slot.captured_ui == w_slot.hovered_ui)
//       p->click_event(event::button(key::mbutton, false, wp, lp));
//   }
//   w_slot.captured_ui = {};
//   w_slot.captured_key = {};
//   w_slot.dirty = true;
// }

// inline void wm_xbuttondown(ui::window::slot& w_slot, WPARAM wp, LPARAM lp) {
//   const bool x1 = HIWORD(wp) == XBUTTON1;
//   const auto code = x1 ? key::xbutton1 : key::xbutton2;
//   if (w_slot.capture_count++ == 0) ::SetCapture(w_slot.hwnd);
//   w_slot.captured_key = code;
//   w_slot.dirty = true;
//   if (const auto p = ui::system::controls.get(w_slot.focused_ui)) {
//     if (w_slot.focused_ui == w_slot.hovered_ui) {
//       w_slot.captured_ui = p->id;
//       p->button_event(event::button(code, true, wp, lp));
//       return;
//     } else {
//       w_slot.focused_ui = {};
//       p->focus_event(false);
//     }
//   }
//   if (const auto p = ui::system::controls.get(w_slot.hovered_ui)) {
//     w_slot.captured_ui = p->id;
//     p->button_event(event::button(code, true, wp, lp));
//     if (p->focus_event(true)) w_slot.focused_ui = p->id;
//     else w_slot.focused_ui = {};
//   } else w_slot.captured_ui = {}, w_slot.focused_ui = {};
// }

// inline void wm_xbuttonup(ui::window::slot& w_slot, WPARAM wp, LPARAM lp) {
//   const bool x1 = HIWORD(wp) == XBUTTON1;
//   const auto code = x1 ? key::xbutton1 : key::xbutton2;
//   w_slot.capture_count = yw::max(0, w_slot.capture_count - 1);
//   if (w_slot.capture_count == 0) ::ReleaseCapture();
//   if (const auto p = ui::system::controls.get(w_slot.captured_ui); p && w_slot.captured_key == code) {
//     p->button_event(event::button(code, false, wp, lp));
//     if (w_slot.captured_ui == w_slot.hovered_ui)
//       p->click_event(event::button(code, false, wp, lp));
//   }
//   w_slot.captured_ui = {};
//   w_slot.captured_key = {};
//   w_slot.dirty = true;
// }
} // namespace internal

//////////////////////////////////////// MARK: wclass::proc

inline LRESULT CALLBACK decltype(wclass)::proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
//   const auto w_slot_id = std::bit_cast<typename slotset<window_slot>::slotid>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA));
//   auto w_slot_p = ui::system::windows.get(w_slot_id);
//   if (!w_slot_p) return ::DefWindowProcW(hwnd, msg, wp, lp);

//   switch (msg) {
//   case WM_MOUSEMOVE: internal::wm_mousemove(*w_slot_p, wp, lp); return 0;
//   case WM_MOUSELEAVE: internal::wm_mouseleave(*w_slot_p, wp, lp); return 0;

//   case WM_KEYDOWN:
//     if (wp == VK_TAB) internal::wm_keydown_tab(*w_slot_p, wp, lp);
//     else if (const auto p = ui::system::controls.get(w_slot_p->focused_ui)) p->key_event(event::key(true, wp, lp));
//     return 0;
//   case WM_KEYUP:
//     if (const auto p = ui::system::controls.get(w_slot_p->focused_ui)) p->key_event(event::key(false, wp, lp));
//     return 0;

//   case WM_CHAR:
//     if (const auto p = ui::system::controls.get(w_slot_p->focused_ui)) p->char_event(static_cast<wchar_t>(wp));
//     return 0;

//   case WM_LBUTTONDOWN: internal::wm_lbuttondown(*w_slot_p, wp, lp); return 0;
//   case WM_LBUTTONUP: internal::wm_lbuttonup(*w_slot_p, wp, lp); return 0;

//   case WM_RBUTTONDOWN: internal::wm_rbuttondown(*w_slot_p, wp, lp); return 0;
//   case WM_RBUTTONUP: internal::wm_rbuttonup(*w_slot_p, wp, lp); return 0;

//   case WM_MBUTTONDOWN: internal::wm_mbuttondown(*w_slot_p, wp, lp); return 0;
//   case WM_MBUTTONUP: internal::wm_mbuttonup(*w_slot_p, wp, lp); return 0;

//   case WM_XBUTTONDOWN: internal::wm_xbuttondown(*w_slot_p, wp, lp); return 0;
//   case WM_XBUTTONUP: internal::wm_xbuttonup(*w_slot_p, wp, lp); return 0;

//   case WM_KILLFOCUS:
//     w_slot_p->captured_ui = {};
//     w_slot_p->captured_key = {};
//     w_slot_p->capture_count = 0;
//     ::ReleaseCapture();
//     return 0;

//   case WM_SIZE:
//     w_slot_p->size.x = LOWORD(lp);
//     w_slot_p->size.y = HIWORD(lp);
//     if (w_slot_p->resizing) return 0;
//     if (auto res = w_slot_p->_resize_rendertarget({w_slot_p->size.x, w_slot_p->size.y}); !res)
//       mainloop.last_error = std::move(res.error().push());
//     else w_slot_p->dirty = true;
//     return 0;

//   case WM_MOVE:
//     w_slot_p->pos = int2(static_cast<int16_t>(LOWORD(lp)), static_cast<int16_t>(HIWORD(lp))) - w_slot_p->margin.xy();
//     return 0;

//   case WM_ENTERSIZEMOVE: w_slot_p->resizing = true; return 0;

//   case WM_EXITSIZEMOVE:
//     w_slot_p->resizing = false;
//     if (auto res = w_slot_p->_resize_rendertarget({w_slot_p->size.x, w_slot_p->size.y}); !res)
//       mainloop.last_error = std::move(res.error().push());
//     else w_slot_p->dirty = true;
//     return 0;

//   case WM_CLOSE:
//     if (w_slot_p->on_close && !w_slot_p->on_close()) return 0;
//     ::DestroyWindow(hwnd);
//     return 0;

//   case WM_NCDESTROY:
//     if (w_slot_p->master_id) {
//       // Delete all UIs belonging to this sub window
//       for (auto ui_id : w_slot_p->uis) { ui::system::controls.erase(ui_id); }
//       if (const auto mw_slot_p = ui::system::windows.get(w_slot_p->master_id))
//         mw_slot_p->subs.erase(std::ranges::find(mw_slot_p->subs, w_slot_p->id));
//       ui::system::windows.erase(w_slot_p->id);
//       ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
//     } else {
//       // Delete all UIs belonging to this master window
//       for (auto ui_id : w_slot_p->uis) { ui::system::controls.erase(ui_id); }
//       for (auto sw_slot_id : w_slot_p->subs) {
//         if (const auto sw_slot_p = ui::system::windows.get(sw_slot_id)) {
//           // Delete all UIs belonging to sub windows
//           for (auto ui_id : sw_slot_p->uis) { ui::system::controls.erase(ui_id); }
//           ::SetWindowLongPtrW(sw_slot_p->hwnd, GWLP_USERDATA, 0);
//           ::DestroyWindow(sw_slot_p->hwnd);
//           ui::system::windows.erase(sw_slot_id);
//         }
//       }
//       const auto id = w_slot_p->id;
//       ui::system::windows.erase(id);
//       std::erase(system::master_windows, id);
//       ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
//       if (system::master_windows.empty()) { ::PostQuitMessage(0); }
//     }
//     break;
//   }
  return ::DefWindowProcW(hwnd, msg, wp, lp);
}

} // namespace yw
