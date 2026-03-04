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
      for (const auto& w_id : system::master_windows)
        if (const auto w_slot_p = system::windows.get(w_id)) w_slot_p->draw();
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

//////////////////////////////////////// MARK: wclass::proc

inline LRESULT CALLBACK decltype(wclass)::proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
  const auto w_slot_id = std::bit_cast<typename slotset<window_slot>::slotid>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA));
  auto w_slot_p = system::windows.get(w_slot_id);
  if (!w_slot_p) return ::DefWindowProcW(hwnd, msg, wp, lp);
  const auto change_focus = [&](decltype(w_slot_p->focused_ui) new_id) {
    if (w_slot_p->focused_ui == new_id) return;
    if (w_slot_p->focused_ui)
      if (const auto s = system::uis.get(w_slot_p->focused_ui)) s->proc(WM_KILLFOCUS, 0, 0);
    w_slot_p->focused_ui = new_id;
    if (new_id)
      if (const auto s = system::uis.get(new_id)) s->proc(WM_SETFOCUS, 0, 0);
    w_slot_p->dirty = true;
  };
  switch (msg) {
  /// MARK:
  case WM_MOUSEMOVE: {
    // focused_uiにWM_MOUSEMOVEを転送。
    if (w_slot_p->focused_ui)
      if (const auto fui_slot_p = system::uis.get(w_slot_p->focused_ui))
        if (auto res = fui_slot_p->proc(WM_MOUSEMOVE, wp, lp); !res)
          mainloop.last_error = std::move(unexpected_error(res.error()).error());
    // WM_MOUSELEAVEを受け取るために登録。
    TRACKMOUSEEVENT tme{sizeof(TRACKMOUSEEVENT), TME_LEAVE, hwnd, 0};
    ::TrackMouseEvent(&tme);
    // カーソル直下のコントロールを探してhovered_controlを更新。
    const auto pt = float2(std::bit_cast<short2>(static_cast<uint32_t>(lp & 0xFFFFFFFF)));
    // システムカーソル位置（スクリーン座標）を更新。
    { POINT sp{(LONG)pt.x, (LONG)pt.y}; ::ClientToScreen(hwnd, &sp); system::cursor_pos = {sp.x, sp.y}; }
    for (auto ui_slot_id : w_slot_p->uis | std::views::reverse)
      if (const auto ui_slot_p = system::uis.get(ui_slot_id))
        if (ui_slot_p->visible && ui_slot_p->hit_test(pt)) {
          if (w_slot_p->hovered_ui != ui_slot_p->id) {
            if (w_slot_p->hovered_ui) {
              const auto hovered_slot = system::uis.get(w_slot_p->hovered_ui);
              if (hovered_slot) hovered_slot->proc(WM_MOUSELEAVE, (wp & 0xffff) | 0x20000, lp);
            }
            w_slot_p->hovered_ui = ui_slot_p->id;
            ui_slot_p->proc(WM_MOUSEMOVE, (wp & 0xffff) | 0x10000, lp);
            w_slot_p->dirty = true;
          }
          return 0;
        }
    // コントロールが見つからない場合
    if (w_slot_p->hovered_ui) {
      if (auto ui_slot_p = system::uis.get(w_slot_p->hovered_ui))
        ui_slot_p->proc(WM_MOUSELEAVE, (wp & 0xffff) | 0x20000, lp);
      w_slot_p->hovered_ui = {};
      w_slot_p->dirty = true;
    }
    return 0;
  }
  case WM_MOUSELEAVE: {
    { POINT sp; ::GetCursorPos(&sp); system::cursor_pos = {sp.x, sp.y}; }
    if (w_slot_p->hovered_ui) {
      if (const auto ui_slot_p = system::uis.get(w_slot_p->hovered_ui))
        ui_slot_p->proc(WM_MOUSELEAVE, (wp & 0xffff) | 0x20000, lp);
      w_slot_p->hovered_ui = {};
      w_slot_p->dirty = true;
    }
    return 0;
  }

  case WM_KEYDOWN: {
    if (wp == VK_TAB) {
      const bool shift = (::GetKeyState(VK_SHIFT) & 0x8000) != 0;
      const auto& uis = w_slot_p->uis;
      const int n = static_cast<int>(uis.size());
      const int total = n + 1; // indices 0..n-1 are uis, n means "window focus" find current position in total space
      int cur = n; // default: window has focus
      if (w_slot_p->focused_ui) {
        for (int i = 0; i < n; ++i) {
          if (uis[i] == w_slot_p->focused_ui) { cur = i; break; }
        }
      }
      // advance one step at a time, skipping non-(focusable && visible)
      for (int step = 1; step <= total; ++step) {
        const int next = ((cur + (shift ? -step : step)) % total + total) % total;
        if (next == n) {
          // focus goes to window
          change_focus({});
          break;
        }
        if (const auto ui_slot_p = system::uis.get(uis[next]))
          if (ui_slot_p->focusable() && ui_slot_p->visible) {
            change_focus(uis[next]);
            break;
          }
      }
      return 0;
    }
    // TAB以外のキーをfocused_uiに転送。
    if (w_slot_p->focused_ui)
      if (const auto ui_slot_p = system::uis.get(w_slot_p->focused_ui))
        if (auto res = ui_slot_p->proc(WM_KEYDOWN, wp, lp); !res)
          mainloop.last_error = std::move(unexpected_error(res.error()).error());
    break;
  }
  case WM_KEYUP: {
    if (w_slot_p->focused_ui)
      if (const auto ui_slot_p = system::uis.get(w_slot_p->focused_ui))
        if (auto res = ui_slot_p->proc(WM_KEYUP, wp, lp); !res)
          mainloop.last_error = std::move(unexpected_error(res.error()).error());
    break;
  }

  case WM_LBUTTONDOWN: {
    if (w_slot_p->hovered_ui) {
      if (const auto ui_slot_p = system::uis.get(w_slot_p->hovered_ui)) {
        if (ui_slot_p->enabled) {
          if (ui_slot_p->focusable()) change_focus(w_slot_p->hovered_ui);
          if (auto res = ui_slot_p->proc(WM_LBUTTONDOWN, wp, lp); !res)
            mainloop.last_error = std::move(unexpected_error(res.error()).error());
          ::SetCapture(hwnd);
        }
      }
    } else {
      change_focus({});
    }
    return 0;
  }
  case WM_LBUTTONUP: {
    ::ReleaseCapture();
    if (w_slot_p->focused_ui) {
      if (const auto ui_slot_p = system::uis.get(w_slot_p->focused_ui))
        if (auto res = ui_slot_p->proc(WM_LBUTTONUP, wp, lp); !res)
          mainloop.last_error = std::move(unexpected_error(res.error()).error());
    }
    return 0;
  }

  case WM_SIZE:
    w_slot_p->size.x = LOWORD(lp), w_slot_p->size.y = HIWORD(lp);
    if (w_slot_p->resizing) return 0;
    if (auto res = w_slot_p->_resize_rendertarget({w_slot_p->size.x, w_slot_p->size.y}); !res)
      mainloop.last_error = std::move(res.error().push());
    else w_slot_p->dirty = true;
    return 0;
  case WM_ENTERSIZEMOVE:
    w_slot_p->resizing = true;
    return 0;
  case WM_EXITSIZEMOVE:
    w_slot_p->resizing = false;
    if (auto res = w_slot_p->_resize_rendertarget({w_slot_p->size.x, w_slot_p->size.y}); !res)
      mainloop.last_error = std::move(res.error().push());
    else w_slot_p->dirty = true;
    return 0;

  case WM_CLOSE:
    if (w_slot_p->on_close && !w_slot_p->on_close()) return 0;
    ::DestroyWindow(hwnd);
    return 0;
  case WM_NCDESTROY:
    if (w_slot_p->master_id) {
      // Delete all UIs belonging to this sub window
      for (auto ui_id : w_slot_p->uis) {
        system::uis.erase(ui_id);
      }
      if (const auto mw_slot_p = system::windows.get(w_slot_p->master_id))
        mw_slot_p->subs.erase(std::ranges::find(mw_slot_p->subs, w_slot_p->id));
      system::windows.erase(w_slot_p->id);
      ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
    } else {
      // Delete all UIs belonging to this master window
      for (auto ui_id : w_slot_p->uis) {
        system::uis.erase(ui_id);
      }
      for (auto sw_slot_id : w_slot_p->subs) {
        if (const auto sw_slot_p = system::windows.get(sw_slot_id)) {
          // Delete all UIs belonging to sub windows
          for (auto ui_id : sw_slot_p->uis) {
            system::uis.erase(ui_id);
          }
          ::SetWindowLongPtrW(sw_slot_p->hwnd, GWLP_USERDATA, 0);
          ::DestroyWindow(sw_slot_p->hwnd);
          system::windows.erase(sw_slot_id);
        }
      }
      const auto id = w_slot_p->id;
      system::windows.erase(id);
      std::erase(system::master_windows, id);
      ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
      if (system::master_windows.empty()) { ::PostQuitMessage(0); }
    }
    break;
  }
  return ::DefWindowProcW(hwnd, msg, wp, lp);
}

// inline mainloop_result mainloop() {
//   static stopwatch frame_timer = [] {
//     stopwatch t;
//     t.start();
//     return t;
//   }();
//   ++window::system.frame_count;
//   uint32_t message_count = 0;
//   for (MSG msg; ::PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE);) {
//     if (msg.message == WM_QUIT) return mainloop_result(mainloop_result::state::quit);
//     ::TranslateMessage(&msg), ::DispatchMessageW(&msg);
//     if (window::system.last_error) return mainloop_result(mainloop_result::state::error);
//     if (++message_count > window::system.max_messages_per_frame) break;
//   }
//   // 負荷軽減のため、前回の描画から十分な時間が経過していない場合は描画をスキップする。
//   if (frame_timer.elapsed() < 1.0 / window::system.max_frames_per_second)
//     return mainloop_result(mainloop_result::state::running, true);
//   frame_timer.restart();
//   // 各ウィンドウについてコントロールを描画して更新する。
//   // 描画の失敗は扱わないが、error_traceのデストラクタによって自動でエラー出力される。
//   for (auto& master_slot : window::system.windows) {
//     if (auto d = master_slot.rendertarget.begin_draw())
//       for (auto& control : master_slot.controls)
//         if (control.visible) control.draw();
//     if (master_slot.swapchain) master_slot.swapchain->Present(0, 0);
//     for (auto& slave_slot : master_slot.slaves) {
//       if (window::system.last_error) return mainloop_result(mainloop_result::state::error);
//       if (auto d = slave_slot.rendertarget.begin_draw())
//         for (auto& control : slave_slot.controls)
//           if (control.visible) control.draw();
//       if (slave_slot.swapchain) slave_slot.swapchain->Present(0, 0);
//     }
//   }
//   // ウィンドウ背景を初期化する。
//   for (auto& master_slot : window::system.windows) {
//     if (auto d = master_slot.rendertarget.begin_draw(master_slot.background_color); !d) {
//       window::system.last_error = std::move(d.error().push());
//       return mainloop_result(mainloop_result::state::error);
//     }
//     for (auto& slave_slot : master_slot.slaves) {
//       if (window::system.last_error) return mainloop_result(mainloop_result::state::error);
//       if (auto d = slave_slot.rendertarget.begin_draw(slave_slot.background_color); !d) {
//         window::system.last_error = std::move(d.error().push());
//         return mainloop_result(mainloop_result::state::error);
//       }
//     }
//   }
//   return mainloop_result(mainloop_result::state::running);
// }
} // namespace yw
