#pragma once
#include "ywx/window.h"

namespace yw {

/// cursor position in screen coordinates (pixels)
/// \note This can only be obtained when a window created by ywlib is active.
inline short2 cursor_pos{};

namespace internal {

inline modifiers get_modifiers() {
  return modifiers{
    .ctrl = (::GetKeyState(VK_CONTROL) & 0x8000) != 0,
    .shift = (::GetKeyState(VK_SHIFT) & 0x8000) != 0,
    .alt = (::GetKeyState(VK_MENU) & 0x8000) != 0};
}

inline modifiers get_modifiers(WPARAM wp) {
  return modifiers{
    .ctrl = (wp & MK_CONTROL) != 0, .shift = (wp & MK_SHIFT) != 0, .alt = (::GetKeyState(VK_MENU) & 0x8000) != 0};
}

inline void report(error&& err) { err.add_footprint().print_as_fatal(); }

inline void report(std::expected<void, error>&& res) {
  if (res) return;
  report(std::move(res.error()));
}

inline short2 local_point(LPARAM lp) noexcept { return std::bit_cast<short2>(static_cast<uint32_t>(lp & 0xFFFFFFFF)); }

inline bool moved_too_far(short2 from, short2 to) noexcept {
  const auto delta = to - from;
  return std::abs(int(delta.x)) > 4 || std::abs(int(delta.y)) > 4;
}

inline control::slot* control_of(interface::slotid id) noexcept { return interface::slot::get<control>(id); }

inline std::wstring get_ime_string(HIMC imc, DWORD index) {
  const LONG bytes = ::ImmGetCompositionStringW(imc, index, nullptr, 0);
  if (bytes <= 0) return {};
  std::wstring s(static_cast<size_t>(bytes / sizeof(wchar_t)), L'\0');
  ::ImmGetCompositionStringW(imc, index, s.data(), bytes);
  return s;
}

inline void cancel_mouse_interaction(window::handle<window::type::unknown>::slot* wsp) {
  const auto owner = control_of(wsp->mouse.owner);
  if (owner) owner->button_cancel_event();
  wsp->reset_mouse_state();
  if (::GetCapture() == wsp->hwnd) ::ReleaseCapture();
}

inline void cancel_key_interaction(window::handle<window::type::unknown>::slot* wsp) {
  const auto owner = control_of(wsp->keyboard.owner);
  if (owner) owner->key_cancel_event();
  wsp->reset_key_state();
}

inline control::slot* text_target(window::handle<window::type::unknown>::slot* wsp) noexcept {
  if (const auto fcsp = control_of(wsp->focused_control); fcsp && fcsp->wants_text_input()) return fcsp;
  return nullptr;
}

inline void on_button_down(
  window::handle<window::type::unknown>::slot* wsp, key k, WPARAM wp, LPARAM lp, bool dbl = false) {
  const auto pt = local_point(lp);
  if (wsp->focused_control != wsp->hovered_control) {
    if (const auto fcsp = control_of(wsp->focused_control)) fcsp->focus_event(false);
    wsp->focused_control = wsp->hovered_control;
    if (const auto fcsp = control_of(wsp->focused_control)) fcsp->focus_event(true);
  }
  if (wsp->mouse.active) {
    wsp->mouse.multi_button = true;
  } else {
    wsp->mouse.owner = wsp->hovered_control;
    wsp->mouse.down_pos = pt;
    wsp->mouse.last_pos = pt;
    wsp->mouse.primary_button = k;
    wsp->mouse.active = true;
    wsp->mouse.click_candidate = wsp->hovered_control != interface::slotid{};
    wsp->mouse.multi_button = false;
    wsp->mouse.moved_too_far = false;
  }
  const auto hcsp = control_of(wsp->mouse.active ? wsp->mouse.owner : wsp->hovered_control);
  wsp->set_button_pressed(k, true);
  wsp->mouse_capture = wsp->mouse.owner;
  if (wsp->mouse_capture) ::SetCapture(wsp->hwnd);
  if (hcsp) {
    const auto ev = button_event{pt, k, get_modifiers(wp), true};
    hcsp->button_event(ev);
    if (dbl) hcsp->double_click_event(ev);
  }
}

inline void on_button_up(window::handle<window::type::unknown>::slot* wsp, key k, WPARAM wp, LPARAM lp) {
  const auto owner_id = wsp->mouse.owner;
  const auto ccsp = control_of(owner_id);
  if (!ccsp || ccsp->window_id != wsp->id) {
    wsp->reset_mouse_state();
    if (::GetCapture() == wsp->hwnd) ::ReleaseCapture();
    return;
  }
  const auto pt = local_point(lp);
  wsp->mouse.last_pos = pt;
  if (moved_too_far(wsp->mouse.down_pos, pt)) wsp->mouse.moved_too_far = true;
  const auto ev = button_event{pt, k, get_modifiers(wp), false};
  if (owner_id == wsp->hovered_control && wsp->mouse.click_candidate && !wsp->mouse.multi_button &&
      !wsp->mouse.moved_too_far && wsp->mouse.primary_button == k) {
    ccsp->click_event(ev);
    ccsp->activate_event(yw::activate_event{activation_source::mouse, k, pt});
  }
  ccsp->button_event(ev);
  wsp->set_button_pressed(k, false);
  if (wsp->mouse.buttons == 0) {
    wsp->reset_mouse_state();
    if (::GetCapture() == wsp->hwnd) ::ReleaseCapture();
  }
}

inline LRESULT on_mouse_move(window::handle<window::type::unknown>::slot* wsp, HWND hwnd, WPARAM wp, LPARAM lp) {
  if (!wsp->tracking) {
    TRACKMOUSEEVENT tme{sizeof(TRACKMOUSEEVENT), TME_LEAVE, hwnd, 0};
    ::TrackMouseEvent(&tme);
    wsp->tracking = true;
  }
  const auto pt = local_point(lp);
  const auto old_global_pt = std::exchange(yw::cursor_pos, pt);
  ::ClientToScreen(hwnd, reinterpret_cast<POINT*>(&yw::cursor_pos));
  if (wsp->mouse.active && moved_too_far(wsp->mouse.down_pos, pt)) wsp->mouse.moved_too_far = true;
  if (wsp->mouse.active) wsp->mouse.last_pos = pt;
  if (const auto fcsp = control_of(wsp->mouse.active ? wsp->mouse.owner : wsp->focused_control)) {
    const auto delta = yw::cursor_pos - old_global_pt;
    fcsp->move_event(move_event{pt, delta});
    const auto mods = get_modifiers(wp);
    if ((wp & MK_LBUTTON) == MK_LBUTTON) fcsp->drag_event(drag_event{delta, keys::lbutton, mods});
    else if ((wp & MK_RBUTTON) == MK_RBUTTON) fcsp->drag_event(drag_event{delta, keys::rbutton, mods});
    else if ((wp & MK_MBUTTON) == MK_MBUTTON) fcsp->drag_event(drag_event{delta, keys::mbutton, mods});
  }
  interface::slotid new_hcid{};
  if (const auto csp = control_of(wsp->child_control)) new_hcid = csp->hittest(pt);
  if (wsp->hovered_control) {
    if (wsp->hovered_control != new_hcid) {
      if (const auto hcsp = control_of(wsp->hovered_control))
        hcsp->hover_event(hover_event{pt, hover_event::type::leave});
      wsp->hovered_control = new_hcid;
      if (const auto hcsp = control_of(new_hcid)) hcsp->hover_event(hover_event{pt, hover_event::type::enter});
    } else if (const auto hcsp = control_of(new_hcid)) hcsp->hover_event(hover_event{pt, hover_event::type::move});
  } else if (const auto hcsp = control_of(new_hcid)) {
    wsp->hovered_control = new_hcid;
    hcsp->hover_event(hover_event{pt, hover_event::type::enter});
  }
  return 0;
}

inline LRESULT on_mouse_leave(window::handle<window::type::unknown>::slot* wsp, HWND hwnd, LPARAM lp) {
  wsp->tracking = false;
  if (const auto hcsp = control_of(wsp->hovered_control)) {
    auto pt = local_point(lp);
    ::ScreenToClient(hwnd, reinterpret_cast<POINT*>(&pt));
    hcsp->hover_event(hover_event{pt, hover_event::type::leave});
    wsp->hovered_control = {};
  }
  return 0;
}

template<bool Horizontal>
inline LRESULT on_mouse_wheel(window::handle<window::type::unknown>::slot* wsp, HWND hwnd, WPARAM wp, LPARAM lp) {
  if (const auto hcsp = control_of(wsp->hovered_control)) {
    auto pt = local_point(lp);
    ::ScreenToClient(hwnd, reinterpret_cast<POINT*>(&pt));
    const auto delta = static_cast<short>(GET_WHEEL_DELTA_WPARAM(wp));
    hcsp->wheel_event(wheel_event{pt, delta, get_modifiers(wp), Horizontal});
  }
  return 0;
}

inline LRESULT on_key_down(window::handle<window::type::unknown>::slot* wsp, WPARAM wp) {
  cancel_key_interaction(wsp);
  if (wp == VK_TAB) {
    if (const auto fcsp = control_of(wsp->focused_control)) fcsp->focus_event(false);
    if (const auto csp = control_of(wsp->child_control)) {
      const bool shift = keys::shift.pressed();
      bool found = wsp->focused_control == interface::slotid{};
      if (found) wsp->focused_control = wsp->child_control;
      const auto next = csp->find_next_tabstop(wsp->focused_control, !shift, found);
      if (next != wsp->focused_control) wsp->dirty = true;
      if (const auto fcsp = control_of(next)) {
        wsp->focused_control = next;
        fcsp->focus_event(true);
      }
    }
    return 0;
  }
  if (wp == VK_ESCAPE) {
    if (const auto fcsp = control_of(wsp->focused_control)) {
      cancel_key_interaction(wsp);
      fcsp->focus_event(false);
      wsp->focused_control = {};
      wsp->dirty = true;
    } else if (wsp->on_keydown) wsp->on_keydown(key_event{static_cast<uint8_t>(wp), get_modifiers(), true});
    return 0;
  }
  const auto ev = key_event{static_cast<uint8_t>(wp), get_modifiers(), true};
  if (const auto fcsp = control_of(wsp->focused_control)) {
    if ((ev.key == keys::space && fcsp->wants_space_activate()) || (ev.key == keys::enter && fcsp->wants_enter_activate())) {
      wsp->key_capture = wsp->focused_control;
      wsp->keyboard.owner = wsp->focused_control;
      wsp->keyboard.code = ev.key;
      wsp->keyboard.active = true;
      wsp->keyboard.activate_candidate = true;
      return 0;
    }
    wsp->key_capture = wsp->focused_control;
    wsp->keyboard.owner = wsp->focused_control;
    wsp->keyboard.code = ev.key;
    wsp->keyboard.active = true;
    wsp->keyboard.activate_candidate = ev.key == keys::enter || ev.key == keys::space;
    if (!fcsp->key_event(ev) && wsp->on_keydown) wsp->on_keydown(ev);
  } else if (wsp->on_keydown) wsp->on_keydown(ev);
  return 0;
}

inline LRESULT on_key_up(window::handle<window::type::unknown>::slot* wsp, WPARAM wp) {
  const auto ev = key_event{static_cast<uint8_t>(wp), get_modifiers(), false};
  const auto target_id = wsp->keyboard.active ? wsp->keyboard.owner : wsp->focused_control;
  if (const auto fcsp = control_of(target_id)) {
    if (wsp->keyboard.active && wsp->keyboard.code == ev.key && wsp->keyboard.activate_candidate) {
      if (wsp->focused_control == target_id)
        fcsp->activate_event(yw::activate_event{activation_source::keyboard, ev.key, {}});
      wsp->reset_key_state();
      return 0;
    }
    if (!fcsp->key_event(ev) && wsp->on_keyup) wsp->on_keyup(ev);
  } else if (wsp->on_keyup) wsp->on_keyup(ev);
  if (wsp->keyboard.code == ev.key) wsp->reset_key_state();
  return 0;
}

inline LRESULT on_char(window::handle<window::type::unknown>::slot* wsp, WPARAM wp) {
  if (const auto fcsp = text_target(wsp)) fcsp->char_event(static_cast<wchar_t>(wp));
  return 0;
}

inline LRESULT on_ime_start(window::handle<window::type::unknown>::slot* wsp) {
  if (const auto target = text_target(wsp)) {
    wsp->ime_owner = target->id;
    wsp->ime_composing = true;
    target->on_ime_start();
  } else {
    wsp->reset_ime_state();
  }
  return 0;
}

inline LRESULT on_ime_composition(window::handle<window::type::unknown>::slot* wsp, HWND hwnd, WPARAM wp, LPARAM lp) {
  auto owner = control_of(wsp->ime_owner);
  if (!owner) {
    owner = text_target(wsp);
    if (owner) wsp->ime_owner = owner->id;
  }
  if (!owner) return ::DefWindowProcW(hwnd, WM_IME_COMPOSITION, wp, lp);
  HIMC imc = ::ImmGetContext(hwnd);
  if (!imc) return 0;
  wsp->ime_composing = true;
  if (lp & GCS_COMPSTR) {
    const auto s = get_ime_string(imc, GCS_COMPSTR);
    owner->on_ime_composition(s);
  }
  if (lp & GCS_RESULTSTR) {
    const auto s = get_ime_string(imc, GCS_RESULTSTR);
    owner->on_ime_commit(s);
  }
  ::ImmReleaseContext(hwnd, imc);
  return 0;
}

inline LRESULT on_ime_end(window::handle<window::type::unknown>::slot* wsp) {
  if (const auto owner = control_of(wsp->ime_owner)) owner->on_ime_end();
  wsp->reset_ime_state();
  return 0;
}

inline LRESULT on_getminmaxinfo(window::handle<window::type::unknown>::slot* wsp, LPARAM lp) {
  auto* mmi = reinterpret_cast<MINMAXINFO*>(lp);
  if (!mmi) return 0;
  const auto csp = control_of(wsp->child_control);
  if (!csp) return 0;
  auto res = csp->calculate_necessary_size();
  if (!res) {
    report(std::move(res.error()));
    return 0;
  }
  const auto minimum_area = vapply_r<int2>(yw::ceil, *res + csp->margin.xy() + csp->margin.zw());
  const auto frame_area = wsp->frame_thickness.xy() + wsp->frame_thickness.zw();
  const auto minimum_track = minimum_area + frame_area;
  mmi->ptMinTrackSize.x = LONG(minimum_track.x);
  mmi->ptMinTrackSize.y = LONG(minimum_track.y);
  return 0;
}

inline LRESULT on_size(window::handle<window::type::unknown>::slot* wsp, LPARAM lp) {
  if (wsp->resizing) return 0;
  wsp->size = int2(LOWORD(lp), HIWORD(lp));
  wsp->messy = true;
  report(wsp->resize_rendertarget());
  return 0;
}

inline LRESULT on_move(window::handle<window::type::unknown>::slot* wsp, LPARAM lp) {
  wsp->pos = int2(static_cast<int16_t>(LOWORD(lp)), static_cast<int16_t>(HIWORD(lp))) - wsp->frame_thickness.xy();
  return 0;
}

inline LRESULT on_enter_size_move(window::handle<window::type::unknown>::slot* wsp) {
  wsp->resizing = true;
  return 0;
}

inline LRESULT on_exit_size_move(window::handle<window::type::unknown>::slot* wsp, HWND hwnd) {
  wsp->resizing = false;
  RECT cr{};
  if (!::GetClientRect(hwnd, &cr)) {
    report(error(errors::operation_failed, "GetClientRect failed", int32_t(::GetLastError())));
    return 0;
  }
  const auto size = int2(cr.right, cr.bottom);
  if (wsp->size != size) return on_size(wsp, MAKELPARAM(cr.right, cr.bottom));
  return 0;
}

inline LRESULT on_close(window::handle<window::type::unknown>::slot* wsp, HWND hwnd) {
  if (wsp->on_close && !wsp->on_close()) return 0;
  ::DestroyWindow(hwnd);
  return 0;
}

inline LRESULT on_kill_focus(window::handle<window::type::unknown>::slot* wsp) {
  if (const auto owner = control_of(wsp->ime_owner)) {
    if (wsp->ime_composing) owner->commit_ime_composition();
    owner->on_ime_end();
  }
  wsp->reset_ime_state();
  cancel_mouse_interaction(wsp);
  cancel_key_interaction(wsp);
  if (const auto fcsp = control_of(wsp->focused_control)) fcsp->focus_event(false);
  wsp->focused_control = {};
  wsp->dirty = true;
  return 0;
}

inline LRESULT on_capture_changed(window::handle<window::type::unknown>::slot* wsp) {
  cancel_mouse_interaction(wsp);
  return 0;
}

inline LRESULT on_cancel_mode(window::handle<window::type::unknown>::slot* wsp) {
  cancel_mouse_interaction(wsp);
  cancel_key_interaction(wsp);
  if (const auto owner = control_of(wsp->ime_owner)) {
    owner->cancel_event();
    if (wsp->ime_composing) owner->on_ime_end();
  }
  wsp->reset_ime_state();
  return 0;
}

inline LRESULT on_nc_destroy(window::handle<window::type::unknown>::slot* wsp, HWND hwnd) {
  const auto child = wsp->child_control;
  const auto id = wsp->id;
  if (child) interface::slot::slots.erase(child);
  interface::slot::slots.erase(id);
  std::erase(window::handle<window::type::unknown>::slot::primals, id);
  ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
  if (window::handle<window::type::unknown>::slot::primals.empty()) ::PostQuitMessage(0);
  return 0;
}

} // namespace internal

inline LRESULT __stdcall wclass::wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) noexcept {
  const auto wid = static_cast<interface::slotid>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA));
  const auto wsp = interface::slot::get<window::handle<window::type::unknown>>(wid);
  if (!wsp) return ::DefWindowProcW(hwnd, msg, wp, lp);
  switch (msg) {
  case WM_MOUSEMOVE: return internal::on_mouse_move(wsp, hwnd, wp, lp);
  case WM_MOUSELEAVE: return internal::on_mouse_leave(wsp, hwnd, lp);
  case WM_MOUSEWHEEL: return internal::on_mouse_wheel<false>(wsp, hwnd, wp, lp);
  case WM_MOUSEHWHEEL: return internal::on_mouse_wheel<true>(wsp, hwnd, wp, lp);

  case WM_KEYDOWN: return internal::on_key_down(wsp, wp);
  case WM_KEYUP: return internal::on_key_up(wsp, wp);
  case WM_CHAR: return internal::on_char(wsp, wp);
  case WM_IME_STARTCOMPOSITION: return internal::on_ime_start(wsp);
  case WM_IME_COMPOSITION: return internal::on_ime_composition(wsp, hwnd, wp, lp);
  case WM_IME_ENDCOMPOSITION: return internal::on_ime_end(wsp);

  case WM_LBUTTONDOWN: internal::on_button_down(wsp, keys::lbutton, wp, lp); return 0;
  case WM_LBUTTONUP: internal::on_button_up(wsp, keys::lbutton, wp, lp); return 0;
  case WM_LBUTTONDBLCLK: internal::on_button_down(wsp, keys::lbutton, wp, lp, true); return 0;

  case WM_RBUTTONDOWN: internal::on_button_down(wsp, keys::rbutton, wp, lp); return 0;
  case WM_RBUTTONUP: internal::on_button_up(wsp, keys::rbutton, wp, lp); return 0;
  case WM_RBUTTONDBLCLK: internal::on_button_down(wsp, keys::rbutton, wp, lp, true); return 0;

  case WM_MBUTTONDOWN: internal::on_button_down(wsp, keys::mbutton, wp, lp); return 0;
  case WM_MBUTTONUP: internal::on_button_up(wsp, keys::mbutton, wp, lp); return 0;
  case WM_MBUTTONDBLCLK: internal::on_button_down(wsp, keys::mbutton, wp, lp, true); return 0;

  case WM_XBUTTONDOWN:
    internal::on_button_down(wsp, HIWORD(wp) == XBUTTON1 ? keys::xbutton1 : keys::xbutton2, wp, lp);
    return TRUE;
  case WM_XBUTTONUP:
    internal::on_button_up(wsp, HIWORD(wp) == XBUTTON1 ? keys::xbutton1 : keys::xbutton2, wp, lp);
    return TRUE;
  case WM_XBUTTONDBLCLK:
    internal::on_button_down(wsp, HIWORD(wp) == XBUTTON1 ? keys::xbutton1 : keys::xbutton2, wp, lp, true);
    return TRUE;

  case WM_GETMINMAXINFO: return internal::on_getminmaxinfo(wsp, lp);
  case WM_SIZE: return internal::on_size(wsp, lp);
  case WM_MOVE: return internal::on_move(wsp, lp);
  case WM_ENTERSIZEMOVE: return internal::on_enter_size_move(wsp);
  case WM_EXITSIZEMOVE: return internal::on_exit_size_move(wsp, hwnd);
  case WM_KILLFOCUS: return internal::on_kill_focus(wsp);
  case WM_CAPTURECHANGED: return internal::on_capture_changed(wsp);
  case WM_CANCELMODE: return internal::on_cancel_mode(wsp);

  case WM_CLOSE: return internal::on_close(wsp, hwnd);
  case WM_NCDESTROY: return internal::on_nc_destroy(wsp, hwnd);
  }
  return ::DefWindowProcW(hwnd, msg, wp, lp);
}
} // namespace yw
