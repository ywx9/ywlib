#pragma once
#include "ywx/ime.h"
#include "ywx/ui_layout.h"
#include "ywx/window.h"

namespace yw {

namespace system {} // namespace system

//////////////////////////////////////// MARK: mainloop

inline class {
public:
  enum class state { running, error, quit };

private:
  state _state = state::quit;
  bool _updated = true;
  stopwatch _timer{};

public:
  error_trace last_error{};
  uint32_t max_messages_per_frame = 100;

  double fps{};
  double spf{};

  bool running() const noexcept { return _state == state::running; }
  bool error() const noexcept { return _state == state::error; }
  bool quit() const noexcept { return _state == state::quit; }

  /// runs the mainloop
  bool operator()() {
    if (system::primal_windows.empty()) return _state = state::quit, false;
    if (_state == state::quit) _timer.restart();
    _state = state::running;

    for (const auto& wid : system::primal_windows)
      if (const auto wsp = system::slot_address<window>(wid); wsp && wsp->visible && (wsp->dirty || wsp->messy)) {
        if (auto res = wsp->draw_layout_bitmap(); !res) {
          last_error = std::move(res.error().push());
          return _state = state::error, false;
        }
      }

    uint32_t processed_messages = 0;
    for (MSG msg; ::PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE);) {
      if (msg.message == WM_QUIT) return _state = state::quit, false;
      ::TranslateMessage(&msg);
      ::DispatchMessageW(&msg);
      if (last_error) return _state = state::error, false;
      if (++processed_messages >= max_messages_per_frame) break;
    }

    for (const auto& wid : system::primal_windows)
      if (const auto wsp = system::slot_address<window>(wid); wsp && wsp->visible) {
        wsp->draw();
        wsp->swapchain->Present(0, 0);
      }
    fps = 1.0 / (spf = _timer.lap());
    return _state == state::running;
  }

  /// runs the mainloop
  explicit operator bool() { return operator()(); }
} mainloop;

// //////////////////////////////////////// MARK: internal::wm_mousemove

namespace internal {
inline void wm_size(window::slot& ws, WPARAM, LPARAM lp) {
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
  }
  // lsp->size = float2(ws.size);
  ws.messy = true;
}

inline void wm_mousemove(window::slot& ws, WPARAM wp, LPARAM lp) {
  if (!ws.tracking) {
    TRACKMOUSEEVENT tme{sizeof(TRACKMOUSEEVENT), TME_LEAVE, ws.hwnd, 0};
    ::TrackMouseEvent(&tme);
    ws.tracking = true;
  }
  const auto local_pt = std::bit_cast<short2>(static_cast<uint32_t>(lp & 0xFFFFFFFF));
  const auto old_global_pt = system::cursor_pos;
  system::cursor_pos = local_pt;
  ::ClientToScreen(ws.hwnd, reinterpret_cast<POINT*>(&system::cursor_pos));
  system::cursor_delta = system::cursor_pos - old_global_pt;
  const auto pt = float2(local_pt);
  if (const auto fcsp = system::slot_address<ui::control>(ws.focused_control)) {
    fcsp->move_event(event::move(local_pt, system::cursor_delta));
    const bool c = (wp & MK_CONTROL) == MK_CONTROL, s = (wp & MK_SHIFT) == MK_SHIFT, a = (wp & MK_ALT) == MK_ALT;
    if ((wp & MK_LBUTTON) == MK_LBUTTON) fcsp->drag_event(event::drag(local_pt, key::lbutton, c, s, a));
    else if ((wp & MK_RBUTTON) == MK_RBUTTON) fcsp->drag_event(event::drag(local_pt, key::rbutton, c, s, a));
    else if ((wp & MK_MBUTTON) == MK_MBUTTON) fcsp->drag_event(event::drag(local_pt, key::mbutton, c, s, a));
  }
  ui::slotid new_hcid{};
  if (const auto lsp = system::slot_address<ui::layout>(ws.layout_id)) new_hcid = lsp->hit_test(pt);
  if (ws.hovered_control) {
    if (ws.hovered_control != new_hcid) {
      if (const auto hcsp = system::slot_address<ui::control>(ws.hovered_control))
        hcsp->hover_event(event::hover(local_pt, event::hover::type::leave));
      ws.hovered_control = new_hcid;
      if (const auto hcsp = system::slot_address<ui::control>(ws.hovered_control))
        hcsp->hover_event(event::hover(local_pt, event::hover::type::enter));
    } else if (const auto hcsp = system::slot_address<ui::control>(ws.hovered_control))
      hcsp->hover_event(event::hover(local_pt, event::hover::type::move));
  } else if (const auto hcsp = system::slot_address<ui::control>(new_hcid)) {
    ws.hovered_control = new_hcid;
    hcsp->hover_event(event::hover(local_pt, event::hover::type::enter));
  }
}

inline void wm_mouseleave(window::slot& ws, WPARAM wp, LPARAM lp) {
  const auto local_pt = ws.cursor_pos();
  ws.tracking = false;
  if (ws.hovered_control) {
    if (const auto hcsp = system::slot_address<ui::control>(ws.hovered_control))
      hcsp->hover_event(event::hover(local_pt, event::hover::type::leave));
    ws.hovered_control = {};
  }
}

inline void wm_mousewheel(window::slot& ws, WPARAM wp, LPARAM lp, bool horizontal) {
  const auto local_pt = ws.cursor_pos();
  const auto delta = static_cast<short>(GET_WHEEL_DELTA_WPARAM(wp));
  const bool c = (GET_KEYSTATE_WPARAM(wp) & MK_CONTROL) == MK_CONTROL;
  const bool s = (GET_KEYSTATE_WPARAM(wp) & MK_SHIFT) == MK_SHIFT;
  const bool a = (::GetKeyState(VK_MENU) & 0x8000) != 0;
  if (const auto hcsp = system::slot_address<ui::control>(ws.hovered_control))
    hcsp->wheel_event(event::wheel(local_pt, delta, horizontal, c, s, a));
}

inline void wm_keydown(window::slot& ws, WPARAM wp, LPARAM lp) {
  if (wp == VK_TAB) {
    if (const auto fsp = system::slot_address<ui::control>(ws.focused_control)) fsp->focus_event(false);
    const bool shift = is_key_down(key::shift);
    ws.next_tab_stop(!shift);
    ws.dirty = true;
  } else if (wp == VK_ESCAPE) {
    if (const auto fsp = system::slot_address<ui::control>(ws.focused_control)) fsp->focus_event(false);
    ws.focused_control = {};
    ws.dirty = true;
  } else {
    const auto c = is_key_down(key::ctrl);
    const auto s = is_key_down(key::shift);
    const auto a = is_key_down(key::alt);
    const auto first = (lp & (1u << 30)) == 0;
    const auto e = event::key(key(wp), c, s, a, true, first);
    bool handled = false;
    if (const auto p = system::slot_address<ui::control>(ws.focused_control)) handled = p->key_event(e);
    if (!handled && ws.on_keydown) ws.on_keydown(e);
  }
}

template<key K, bool DBL = false> void wm_button_down(window::slot& ws, WPARAM wp, LPARAM lp) {
  const auto local_pt = std::bit_cast<short2>(static_cast<uint32_t>(uint_cast(lp)));
  if (ws.capture_count++ == 0) ::SetCapture(ws.hwnd);
  // ws.captured_key = K;
  ws.dirty = true;
  const bool c = (wp & MK_CONTROL) == MK_CONTROL;
  const bool s = (wp & MK_SHIFT) == MK_SHIFT;
  const bool a = (wp & MK_ALT) == MK_ALT;
  if (const auto fcsp = system::slot_address<ui::control>(ws.focused_control)) {
    if (ws.focused_control == ws.hovered_control) {
      ws.captured_control = fcsp->id;
      fcsp->button_event(event::button(local_pt, K, c, s, a, true, DBL));
      return;
    }
    ws.focused_control = {};
    fcsp->focus_event(false);
  }
  if (const auto hcsp = system::slot_address<ui::control>(ws.hovered_control)) {
    ws.captured_control = hcsp->id;
    hcsp->button_event(event::button(local_pt, K, c, s, a, true, DBL));
    ws.focused_control = (hcsp->focus_event(true) ? hcsp->id : ui::slotid());
  } else ws.captured_control = {}, ws.focused_control = {};
}

template<key K, bool DBL = false> void wm_button_up(window::slot& ws, WPARAM wp, LPARAM lp) {
  const auto local_pt = std::bit_cast<short2>(static_cast<uint32_t>(uint_cast(lp)));
  ws.capture_count = yw::max(0, ws.capture_count - 1);
  if (ws.capture_count == 0) ::ReleaseCapture();
  const bool c = (wp & MK_CONTROL) == MK_CONTROL;
  const bool s = (wp & MK_SHIFT) == MK_SHIFT;
  const bool a = (wp & MK_ALT) == MK_ALT;
  if (const auto ccsp = system::slot_address<ui::control>(ws.captured_control)) {
    if (ws.captured_control == ws.hovered_control) ccsp->click_event(event::button(local_pt, K, c, s, a, false, DBL));
    ccsp->button_event(event::button(local_pt, K, c, s, a, false, DBL));
  }
  ws.captured_control = {};
  ws.dirty = true;
}
} // namespace internal

//////////////////////////////////////// MARK: wclass::proc

inline LRESULT CALLBACK decltype(wclass)::proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
  const auto wsid = std::bit_cast<ui::slotid>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA));
  auto wsp = system::slot_address<window::slot>(wsid);
  if (!wsp) return ::DefWindowProcW(hwnd, msg, wp, lp);

  switch (msg) {
  case WM_MOUSEMOVE: internal::wm_mousemove(*wsp, wp, lp); return 0;
  case WM_MOUSELEAVE: internal::wm_mouseleave(*wsp, wp, lp); return 0;
  case WM_MOUSEWHEEL: internal::wm_mousewheel(*wsp, wp, lp, false); return 0;
  case WM_MOUSEHWHEEL: internal::wm_mousewheel(*wsp, wp, lp, true); return 0;

  case WM_KEYDOWN: internal::wm_keydown(*wsp, wp, lp); return 0;
  case WM_KEYUP: {
    const auto c = is_key_down(key::ctrl);
    const auto s = is_key_down(key::shift);
    const auto a = is_key_down(key::alt);
    const auto e = event::key(key(wp), c, s, a, false, false);
    bool handled = false;
    if (const auto p = system::slot_address<ui::control>(wsp->focused_control)) handled = p->key_event(e);
    if (!handled && wsp->on_keyup) wsp->on_keyup(e);
  }
    return 0;

  case WM_CHAR:
    if (const auto p = system::slot_address<ui::control>(wsp->focused_control)) p->char_event(static_cast<wchar_t>(wp));
    return 0;

    //////////////////////////////////// MARK: ボタンイベント

  case WM_LBUTTONDOWN: internal::wm_button_down<key::lbutton>(*wsp, wp, lp); return 0;
  case WM_LBUTTONUP: internal::wm_button_up<key::lbutton>(*wsp, wp, lp); return 0;

  case WM_RBUTTONDOWN: internal::wm_button_down<key::rbutton>(*wsp, wp, lp); return 0;
  case WM_RBUTTONUP: internal::wm_button_up<key::rbutton>(*wsp, wp, lp); return 0;

  case WM_MBUTTONDOWN: internal::wm_button_down<key::mbutton>(*wsp, wp, lp); return 0;
  case WM_MBUTTONUP: internal::wm_button_up<key::mbutton>(*wsp, wp, lp); return 0;

  case WM_XBUTTONDOWN:
    if (HIWORD(wp) == XBUTTON1) internal::wm_button_down<key::xbutton1>(*wsp, wp, lp);
    else internal::wm_button_down<key::xbutton2>(*wsp, wp, lp);
    return 0;
  case WM_XBUTTONUP:
    if (HIWORD(wp) == XBUTTON1) internal::wm_button_up<key::xbutton1>(*wsp, wp, lp);
    else internal::wm_button_up<key::xbutton2>(*wsp, wp, lp);
    return 0;

  case WM_LBUTTONDBLCLK: internal::wm_button_down<key::lbutton, true>(*wsp, wp, lp); return 0;
  case WM_RBUTTONDBLCLK: internal::wm_button_down<key::rbutton, true>(*wsp, wp, lp); return 0;

  case WM_KILLFOCUS:
    wsp->captured_control = {};
    // wsp->captured_key = {};
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

    ////////////////////////////////////// MARK: IME

  case WM_IME_SETCONTEXT: lp &= ~ISC_SHOWUICOMPOSITIONWINDOW; return ::DefWindowProcW(hwnd, msg, wp, lp);

  case WM_IME_STARTCOMPOSITION: {
    system::ime.hide();
    system::ime.reset_state();
    // 必要なら edit 側の通常キャレットを消す
    return 0;
  }

  case WM_IME_COMPOSITION:
    if (const auto fcsp = system::slot_address<ui::control>(wsp->focused_control); !fcsp) system::ime.hide();
    else if (HIMC himc = ::ImmGetContext(hwnd); !himc) system::ime.hide();
    else {
      if (lp & GCS_COMPSTR) {
        if (auto bytes = ::ImmGetCompositionStringW(himc, GCS_COMPSTR, nullptr, 0); bytes > 0) {
          std::wstring s(bytes / sizeof(wchar_t), L'\0');
          ::ImmGetCompositionStringW(himc, GCS_COMPSTR, s.data(), bytes);
          system::ime.update_text(s);
        } else system::ime.update_text(L"");
      }
      if (lp & GCS_COMPATTR) {
        std::vector<uint8_t> attrs;
        if (auto bytes = ::ImmGetCompositionStringW(himc, GCS_COMPATTR, nullptr, 0); bytes > 0) {
          attrs.resize(static_cast<size_t>(bytes));
          ::ImmGetCompositionStringW(himc, GCS_COMPATTR, attrs.data(), bytes);
        }
        system::ime.update_attrs(attrs);
      }
      if (lp & GCS_CURSORPOS) {
        LONG pos = ::ImmGetCompositionStringW(himc, GCS_CURSORPOS, nullptr, 0);
        system::ime.update_cursor_pos(pos);
      }
      if (lp & GCS_RESULTSTR) {
        if (auto bytes = ::ImmGetCompositionStringW(himc, GCS_RESULTSTR, nullptr, 0); bytes > 0) {
          std::wstring s(bytes / sizeof(wchar_t), L'\0');
          ::ImmGetCompositionStringW(himc, GCS_RESULTSTR, s.data(), bytes);
          fcsp->ime_insert_text(s);
        }
      }
      const auto local_caret_pos = fcsp->ime_position();
      COMPOSITIONFORM comp_form{};
      comp_form.dwStyle = CFS_POINT;
      comp_form.ptCurrentPos.x = LONG(local_caret_pos.x);
      comp_form.ptCurrentPos.y = LONG(local_caret_pos.y);
      ::ImmSetCompositionWindow(himc, &comp_form);

      CANDIDATEFORM cand_form{};
      cand_form.dwIndex = 0;
      cand_form.dwStyle = CFS_CANDIDATEPOS;
      cand_form.ptCurrentPos.x = LONG(local_caret_pos.x);
      cand_form.ptCurrentPos.y = LONG(local_caret_pos.y);
      ::ImmSetCandidateWindow(himc, &cand_form);

      ::ImmReleaseContext(hwnd, himc);

      system::ime.update_window_size();
      system::ime.draw();
      const auto global_caret_pos = wsp->pos + wsp->margin.xy() + int2(local_caret_pos);
      if (system::ime.window_size().x > 1) system::ime.show(global_caret_pos);
      else system::ime.hide();
    }
    return 0;

  case WM_IME_ENDCOMPOSITION:
    system::ime.hide();
    system::ime.reset_state();
    // 必要なら edit 側の通常キャレットを戻す
    return 0;

    //////////////////////////////////// MARK: 終了処理

  case WM_CLOSE:
    if (wsp->on_close && !wsp->on_close()) return 0;
    ::DestroyWindow(hwnd);
    return 0;

  case WM_NCDESTROY:
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
