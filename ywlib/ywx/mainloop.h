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
  bool _updating = false;
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
      for (auto& w_slot : system::windows) w_slot.draw();
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
  const auto w_slot_id = std::bit_cast<typename slotlist<window_slot>::slotid>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA));
  auto w_slot_p = system::windows.get(w_slot_id);
  if (!w_slot_p) return ::DefWindowProcW(hwnd, msg, wp, lp);
  switch (msg) {
  /// MARK:
  case WM_MOUSEMOVE: {
    // focused_uiにWM_MOUSEMOVEを転送。
    if (w_slot_p->focused_ui)
      if (const auto fui_slot_p = system::uis.get(w_slot_p->focused_ui))
        if (auto res = fui_slot_p->proc(WM_MOUSEMOVE, wp, lp); !res)
          mainloop.last_error = std::move(unexpected_error(res.error()).error());
    // カーソル直下のコントロールを探してhovered_controlを更新。
    const auto pt = float2(std::bit_cast<short2>(static_cast<uint32_t>(lp & 0xFFFFFFFF)));
    for (auto ui_slot_id : w_slot_p->uis | std::views::reverse)
      if (const auto ui_slot_p = system::uis.get(ui_slot_id))
        if (ui_slot_p->visible && ui_slot_p->hit_test(pt)) {
          if (w_slot_p->hovered_ui != ui_slot_p->id) {
            if (w_slot_p->hovered_ui) {
              const auto hovered_slot = system::uis.get(w_slot_p->hovered_ui);
              if (hovered_slot && hovered_slot->on_hover) hovered_slot->on_hover(false);
            }
            w_slot_p->hovered_ui = ui_slot_p->id;
            if (ui_slot_p->on_hover) ui_slot_p->on_hover(true);
          }
          return 0;
        }
    // コントロールが見つからない場合
    if (w_slot_p->hovered_ui) {
      if (auto ui_slot_p = system::uis.get(w_slot_p->hovered_ui))
        if (ui_slot_p->on_hover) ui_slot_p->on_hover(false);
      w_slot_p->hovered_ui = {};
    }
    return 0;
  }


  case WM_SIZE:
    w_slot_p->size.x = LOWORD(lp), w_slot_p->size.y = HIWORD(lp);
    if (w_slot_p->resizing) return 0;
    if (auto res = w_slot_p->_resize_rendertarget({w_slot_p->size.x, w_slot_p->size.y}); !res)
      mainloop.last_error = std::move(res.error().push());
    return 0;
  case WM_ENTERSIZEMOVE:
    w_slot_p->resizing = true;
    return 0;
  case WM_EXITSIZEMOVE:
    w_slot_p->resizing = false;
    if (auto res = w_slot_p->_resize_rendertarget({w_slot_p->size.x, w_slot_p->size.y}); !res)
      mainloop.last_error = std::move(res.error().push());
    return 0;

  case WM_CLOSE:
    if (w_slot_p->on_close && !w_slot_p->on_close()) return 0;
    ::DestroyWindow(hwnd);
    return 0;
  case WM_NCDESTROY:
    if (w_slot_p->master_id) {
      if (const auto mw_slot_p = system::windows.get(w_slot_p->master_id))
        mw_slot_p->subs.erase(std::ranges::find(mw_slot_p->subs, w_slot_p->id));
      system::windows.erase(w_slot_p->id);
      ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
    } else {
      for (auto sw_slot_id : w_slot_p->subs) {
        if (const auto sw_slot_p = system::windows.get(sw_slot_id)) {
          ::SetWindowLongPtrW(sw_slot_p->hwnd, GWLP_USERDATA, 0);
          ::DestroyWindow(sw_slot_p->hwnd);
          system::windows.erase(sw_slot_id);
        }
      }
      system::windows.erase(w_slot_p->id);
      ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
      if (system::windows.empty()) { ::PostQuitMessage(0); }
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
