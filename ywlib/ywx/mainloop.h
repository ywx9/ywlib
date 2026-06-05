#pragma once
// #include "ywx/ime.h"
#include "ywx/window.h"

namespace yw {

namespace system {
inline int2 cursor_pos;
inline int2 cursor_delta;
}

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
      if (const auto wsp = system::get_slot_pointer<window::handle<window::type::unknown>>(wid)) {
        if (auto res = wsp->update_controllayer(); !res) {
          last_error = std::move(res.error().push());
          return _state = state::error, false;
        }
        if (auto res = wsp->draw_controllayer(); !res) {
          last_error = std::move(res.error().push());
          return _state = state::error, false;
        }
        if (auto res = wsp->draw(); !res) {
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
      if (const auto wsp = system::get_slot_pointer<window::handle<window::type::unknown>>(wid))
        if (auto res = wsp->update_controllayer(); !res) {
          last_error = std::move(res.error().push());
          return _state = state::error, false;
        }
    fps = 1.0 / (spf = _timer.lap());
    return _state == state::running;
  }

  /// runs the mainloop
  explicit operator bool() { return operator()(); }
} mainloop;

// //////////////////////////////////////// MARK: internal::wm_mousemove

namespace internal {
inline void wm_size(window::handle<window::type::unknown>::slot& ws, WPARAM, LPARAM lp) {
  if (ws.resizing) return;
  ws.size = int2(LOWORD(lp), HIWORD(lp));
  ws.messy = true;
  if (auto res = ws.resize_rendertarget(); !res) {
    mainloop.last_error = std::move(res.error().push());
    return;
  }
}

inline void wm_getminmaxinfo(window::handle<window::type::unknown>::slot& ws, LPARAM lp) {
  auto* mmi = reinterpret_cast<MINMAXINFO*>(lp);
  if (!mmi) return;
  if (const auto csp = system::get_slot_pointer<ui::control>(ws.child_control)) {
    const auto minimum_size = assume(csp->calculate_necessary_size());
    const auto minimum_area = vapply_r<int2>(yw::ceil, minimum_size + csp->margin.xy() + csp->margin.zw());
    const auto frame_area = ws.frame_thickness.xy() + ws.frame_thickness.zw();
    const auto minimum_track = minimum_area + frame_area;
    mmi->ptMinTrackSize.x = LONG(minimum_track.x);
    mmi->ptMinTrackSize.y = LONG(minimum_track.y);
  }
}

inline void wm_mousemove(window::handle<window::type::unknown>::slot& ws, WPARAM wp, LPARAM lp) {
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
  if (const auto fcsp = system::get_slot_pointer<ui::control>(ws.focused_control)) {
    fcsp->move_event(move_event{local_pt, system::cursor_delta});
    const modifiers mods{(wp & MK_CONTROL) == MK_CONTROL, (wp & MK_SHIFT) == MK_SHIFT, keys::alt.pressed()};
    if ((wp & MK_LBUTTON) == MK_LBUTTON) fcsp->drag_event(drag_event{system::cursor_delta, keys::lbutton, mods});
    else if ((wp & MK_RBUTTON) == MK_RBUTTON) fcsp->drag_event(drag_event{system::cursor_delta, keys::rbutton, mods});
    else if ((wp & MK_MBUTTON) == MK_MBUTTON) fcsp->drag_event(drag_event{system::cursor_delta, keys::mbutton, mods});
  }
  unknown_slotid new_hcid{};
  if (const auto csp = system::get_slot_pointer<ui::control>(ws.child_control)) new_hcid = csp->hittest(pt);
  if (ws.hovered_control) {
    if (ws.hovered_control != new_hcid) {
      if (const auto hcsp = system::get_slot_pointer<ui::control>(ws.hovered_control))
        hcsp->hover_event(hover_event{local_pt, hover_event::type::leave});
      ws.hovered_control = new_hcid;
      if (const auto hcsp = system::get_slot_pointer<ui::control>(ws.hovered_control))
        hcsp->hover_event(hover_event{local_pt, hover_event::type::enter});
    } else if (const auto hcsp = system::get_slot_pointer<ui::control>(ws.hovered_control))
      hcsp->hover_event(hover_event{local_pt, hover_event::type::move});
  } else if (const auto hcsp = system::get_slot_pointer<ui::control>(new_hcid)) {
    ws.hovered_control = new_hcid;
    hcsp->hover_event(hover_event{local_pt, hover_event::type::enter});
  }
}

inline void wm_mouseleave(window::handle<window::type::unknown>::slot& ws, WPARAM wp, LPARAM lp) {
  auto local_pt = system::cursor_pos;
  ::ScreenToClient(ws.hwnd, reinterpret_cast<POINT*>(&local_pt));
  ws.tracking = false;
  if (ws.hovered_control) {
    if (const auto hcsp = system::get_slot_pointer<ui::control>(ws.hovered_control))
      hcsp->hover_event(hover_event{local_pt, hover_event::type::leave});
    ws.hovered_control = {};
  }
}

template<bool Horizontal> void wm_mousewheel(window::handle<window::type::unknown>::slot& ws, WPARAM wp, LPARAM lp) {
  auto local_pt = system::cursor_pos;
  ::ScreenToClient(ws.hwnd, reinterpret_cast<POINT*>(&local_pt));
  const auto delta = static_cast<short>(GET_WHEEL_DELTA_WPARAM(wp));
  const modifiers mods{(wp & MK_CONTROL) == MK_CONTROL, (wp & MK_SHIFT) == MK_SHIFT, keys::alt.pressed()};
  if (const auto hcsp = system::get_slot_pointer<ui::control>(ws.hovered_control))
    hcsp->wheel_event(wheel_event{local_pt, delta, mods, Horizontal});
}

inline void wm_keydown(window::handle<window::type::unknown>::slot& ws, WPARAM wp, LPARAM lp) {
  if (wp == VK_TAB) {
    if (const auto fsp = system::get_slot_pointer<ui::control>(ws.focused_control)) fsp->focus_event(false);
    const bool shift = keys::shift.pressed();
    if (const auto csp = system::get_slot_pointer<ui::control>(ws.child_control)) {
      bool found = ws.focused_control == unknown_slotid{};
      auto next = csp->next_tab_stop(ws.focused_control, !shift, found);
      if (next != ws.focused_control) {
        ws.focused_control = next;
        ws.dirty = true;
      }
    }
  } else if (wp == VK_ESCAPE) {
    if (const auto fsp = system::get_slot_pointer<ui::control>(ws.focused_control)) fsp->focus_event(false);
    ws.focused_control = {};
    ws.dirty = true;
  } else {
    const modifiers mods{keys::ctrl.pressed(), keys::shift.pressed(), keys::alt.pressed()};
    const auto e = key_event{key{static_cast<uint8_t>(wp)}, mods, true};
    bool handled = false;
    if (const auto p = system::get_slot_pointer<ui::control>(ws.focused_control)) handled = p->key_event(e);
    if (!handled && ws.on_keydown) ws.on_keydown(e);
  }
}

inline void wm_keyup(window::handle<window::type::unknown>::slot& ws, WPARAM wp, LPARAM lp) {
  const modifiers mods{keys::ctrl.pressed(), keys::shift.pressed(), keys::alt.pressed()};
  const auto e = key_event{key{static_cast<uint8_t>(wp)}, mods, false};
  bool handled = false;
  if (const auto p = system::get_slot_pointer<ui::control>(ws.focused_control)) handled = p->key_event(e);
  if (!handled && ws.on_keyup) ws.on_keyup(e);
}

template<key K, bool DBL = false>
void wm_button_down(window::handle<window::type::unknown>::slot& ws, WPARAM wp, LPARAM lp) {
  const auto local_pt = std::bit_cast<short2>(static_cast<uint32_t>(uint_cast(lp)));
  // if (ws.capture_count++ == 0) ::SetCapture(ws.hwnd);
  // ws.captured_key = K;
  ws.dirty = true;
  const modifiers mods{(wp & MK_CONTROL) == MK_CONTROL, (wp & MK_SHIFT) == MK_SHIFT, keys::alt.pressed()};
  const button_event event{local_pt, K, mods, true};
  if (const auto fcsp = system::get_slot_pointer<ui::control>(ws.focused_control)) {
    if (ws.focused_control == ws.hovered_control) {
      ws.captured_control = fcsp->id;
      fcsp->button_event(event);
      if constexpr (DBL) fcsp->double_click_event(event);
      return;
    }
    ws.focused_control = {};
    fcsp->focus_event(false);
  }
  if (const auto hcsp = system::get_slot_pointer<ui::control>(ws.hovered_control)) {
    ws.captured_control = hcsp->id;
    hcsp->button_event(event);
    if constexpr (DBL) hcsp->double_click_event(event);
    ws.focused_control = (hcsp->focusable() ? hcsp->id : unknown_slotid());
  } else ws.captured_control = {}, ws.focused_control = {};
}

template<key K, bool DBL = false>
void wm_button_up(window::handle<window::type::unknown>::slot& ws, WPARAM wp, LPARAM lp) {
  const auto local_pt = std::bit_cast<short2>(static_cast<uint32_t>(uint_cast(lp)));
  // ws.capture_count = yw::max(0, ws.capture_count - 1);
  // if (ws.capture_count == 0) ::ReleaseCapture();
  const modifiers mods{(wp & MK_CONTROL) == MK_CONTROL, (wp & MK_SHIFT) == MK_SHIFT, keys::alt.pressed()};
  const button_event event{local_pt, K, mods, false};
  if (const auto ccsp = system::get_slot_pointer<ui::control>(ws.captured_control)) {
    if (ws.captured_control == ws.hovered_control) ccsp->click_event(event);
    ccsp->button_event(event);
  }
  ws.captured_control = {};
  ws.dirty = true;
}
} // namespace internal

/// MARK: system::wndproc

inline LRESULT CALLBACK system::wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
  const auto wsid = std::bit_cast<unknown_slotid>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA));
  auto wsp = system::get_slot_pointer<window::handle<window::type::unknown>>(wsid);
  if (!wsp) return ::DefWindowProcW(hwnd, msg, wp, lp);

  switch (msg) {
  case WM_MOUSEMOVE: internal::wm_mousemove(*wsp, wp, lp); return 0;
  case WM_MOUSELEAVE: internal::wm_mouseleave(*wsp, wp, lp); return 0;
  case WM_MOUSEWHEEL: internal::wm_mousewheel<false>(*wsp, wp, lp); return 0;
  case WM_MOUSEHWHEEL: internal::wm_mousewheel<true>(*wsp, wp, lp); return 0;

  case WM_KEYDOWN: internal::wm_keydown(*wsp, wp, lp); return 0;
  case WM_KEYUP: internal::wm_keyup(*wsp, wp, lp); return 0;

  case WM_CHAR:
    if (const auto p = system::get_slot_pointer<ui::control>(wsp->focused_control)) p->char_event(static_cast<wchar_t>(wp));
    return 0;

    /// MARK: Button events

  case WM_LBUTTONDOWN: internal::wm_button_down<keys::lbutton>(*wsp, wp, lp); return 0;
  case WM_LBUTTONUP: internal::wm_button_up<keys::lbutton>(*wsp, wp, lp); return 0;

  case WM_RBUTTONDOWN: internal::wm_button_down<keys::rbutton>(*wsp, wp, lp); return 0;
  case WM_RBUTTONUP: internal::wm_button_up<keys::rbutton>(*wsp, wp, lp); return 0;
  case WM_MBUTTONDOWN: internal::wm_button_down<keys::mbutton>(*wsp, wp, lp); return 0;
  case WM_MBUTTONUP: internal::wm_button_up<keys::mbutton>(*wsp, wp, lp); return 0;

  case WM_XBUTTONDOWN:
    if (HIWORD(wp) == XBUTTON1) internal::wm_button_down<keys::xbutton1>(*wsp, wp, lp);
    else internal::wm_button_down<keys::xbutton2>(*wsp, wp, lp);
    return 0;
  case WM_XBUTTONUP:
    if (HIWORD(wp) == XBUTTON1) internal::wm_button_up<keys::xbutton1>(*wsp, wp, lp);
    else internal::wm_button_up<keys::xbutton2>(*wsp, wp, lp);
    return 0;

  case WM_LBUTTONDBLCLK: internal::wm_button_down<keys::lbutton, true>(*wsp, wp, lp); return 0;
  case WM_RBUTTONDBLCLK: internal::wm_button_down<keys::rbutton, true>(*wsp, wp, lp); return 0;
  case WM_KILLFOCUS:
    wsp->captured_control = {};
    // wsp->captured_key = {};
    // wsp->capture_count = 0;
    ::ReleaseCapture();
    return 0;

  case WM_GETMINMAXINFO: internal::wm_getminmaxinfo(*wsp, lp); return 0;
  case WM_SIZE: internal::wm_size(*wsp, wp, lp); return 0;

  case WM_MOVE:
    wsp->pos = int2(static_cast<int16_t>(LOWORD(lp)), static_cast<int16_t>(HIWORD(lp))) - wsp->frame_thickness.xy();
    return 0;

  case WM_ENTERSIZEMOVE: wsp->resizing = true; return 0;

  case WM_EXITSIZEMOVE:
    wsp->resizing = false;
    if (RECT cr{}; !::GetClientRect(hwnd, &cr)) {
      mainloop.last_error = std::move(unexpected_win32_error("GetClientRect failed").error());
      return 0;
    } else if (wsp->size != int2(cr.right, cr.bottom)) internal::wm_size(*wsp, 0, MAKELPARAM(cr.right, cr.bottom));
    return 0;

    /// MARK: IME

    // case WM_IME_SETCONTEXT: lp &= ~ISC_SHOWUICOMPOSITIONWINDOW; return ::DefWindowProcW(hwnd, msg, wp, lp);

    // case WM_IME_STARTCOMPOSITION: {
    //   system::ime.hide();
    //   system::ime.reset_state();
    //   // 必要なら edit 側の通常キャレットを消す
    //   return 0;
    // }

    // case WM_IME_COMPOSITION:
    //   if (const auto fcsp = system::get_slot_pointer<ui::control>(wsp->focused_control); !fcsp) system::ime.hide();
    //   else if (HIMC himc = ::ImmGetContext(hwnd); !himc) system::ime.hide();
    //   else {
    //     if (lp & GCS_COMPSTR) {
    //       if (auto bytes = ::ImmGetCompositionStringW(himc, GCS_COMPSTR, nullptr, 0); bytes > 0) {
    //         std::wstring s(bytes / sizeof(wchar_t), L'\0');
    //         ::ImmGetCompositionStringW(himc, GCS_COMPSTR, s.data(), bytes);
    //         system::ime.update_text(s);
    //       } else system::ime.update_text(L"");
    //     }
    //     if (lp & GCS_COMPATTR) {
    //       std::vector<uint8_t> attrs;
    //       if (auto bytes = ::ImmGetCompositionStringW(himc, GCS_COMPATTR, nullptr, 0); bytes > 0) {
    //         attrs.resize(static_cast<size_t>(bytes));
    //         ::ImmGetCompositionStringW(himc, GCS_COMPATTR, attrs.data(), bytes);
    //       }
    //       system::ime.update_attrs(attrs);
    //     }
    //     if (lp & GCS_CURSORPOS) {
    //       LONG pos = ::ImmGetCompositionStringW(himc, GCS_CURSORPOS, nullptr, 0);
    //       system::ime.update_cursor_pos(pos);
    //     }
    //     if (lp & GCS_RESULTSTR) {
    //       if (auto bytes = ::ImmGetCompositionStringW(himc, GCS_RESULTSTR, nullptr, 0); bytes > 0) {
    //         std::wstring s(bytes / sizeof(wchar_t), L'\0');
    //         ::ImmGetCompositionStringW(himc, GCS_RESULTSTR, s.data(), bytes);
    //         fcsp->ime_insert_text(s);
    //       }
    //     }
    //     const auto local_caret_pos = fcsp->ime_position();
    //     COMPOSITIONFORM comp_form{};
    //     comp_form.dwStyle = CFS_POINT;
    //     comp_form.ptCurrentPos.x = LONG(local_caret_pos.x);
    //     comp_form.ptCurrentPos.y = LONG(local_caret_pos.y);
    //     ::ImmSetCompositionWindow(himc, &comp_form);

    //     CANDIDATEFORM cand_form{};
    //     cand_form.dwIndex = 0;
    //     cand_form.dwStyle = CFS_CANDIDATEPOS;
    //     cand_form.ptCurrentPos.x = LONG(local_caret_pos.x);
    //     cand_form.ptCurrentPos.y = LONG(local_caret_pos.y);
    //     ::ImmSetCandidateWindow(himc, &cand_form);

    //     ::ImmReleaseContext(hwnd, himc);

    //     system::ime.update_window_size();
    //     system::ime.draw();
    //     const auto global_caret_pos = wsp->pos + wsp->margin.xy() + int2(local_caret_pos);
    //     if (system::ime.window_size().x > 1) system::ime.show(global_caret_pos);
    //     else system::ime.hide();
    //   }
    //   return 0;

    // case WM_IME_ENDCOMPOSITION:
    //   system::ime.hide();
    //   system::ime.reset_state();
    //   return 0;

    /// MARK: closing events

  case WM_CLOSE:
    if (wsp->on_close && !wsp->on_close()) return 0;
    ::DestroyWindow(hwnd);
    return 0;

  case WM_NCDESTROY:
    system::unknowns.erase(wsp->child_control);
    const auto id = wsp->id;
    system::unknowns.erase(id);
    std::erase(system::primal_windows, id);
    ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
    if (system::primal_windows.empty()) ::PostQuitMessage(0);
    break;
  }
  return ::DefWindowProcW(hwnd, msg, wp, lp);
}
} // namespace yw
