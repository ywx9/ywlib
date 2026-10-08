#pragma once
#include <apps/bitmap.h>
#include <apps/control.h>
#include <apps/directx.h>
#include <core/array.h>
#include <core/color.h>
#include <core/property.h>

namespace yw {

///--------------------------------------------------------------------------///
/// MARK: wclass

class wclass {
public:
  static LRESULT __stdcall wndproc(HWND hw, UINT msg, WPARAM wp, LPARAM lp);

private:
  inline static WNDCLASS wndclass{
    .style = CS_DBLCLKS,
    .lpfnWndProc = wndproc,
    .hInstance = ::GetModuleHandle(nullptr),
    .hCursor = ::LoadCursor(nullptr, IDC_ARROW),
    .lpszClassName = L"ywlib_wndclass",
  };
  inline static bool _initialized{};

public:
  static HINSTANCE hinstance() noexcept {
    if (auto res = initialize(); !res) res.error().print_and_abort();
    return wndclass.hInstance;
  }
  static const wchar_t* name() noexcept {
    if (auto res = initialize(); !res) res.error().print_and_abort();
    return wndclass.lpszClassName;
  }
  static result<void> initialize() {
    if (_initialized) return {};
    const auto atom = ::RegisterClassW(&wndclass);
    if (!atom && ::GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
      return error(errors::operation_failed, "RegisterClass failed");
    _initialized = true;
    return {};
  }
};

///--------------------------------------------------------------------------///
/// MARK: window system

class window;

namespace window_system {
inline array<HWND> windows;
inline result<void> destroy(HWND hwnd) {
  if (!hwnd) return {};
  if (const auto it = std::ranges::find(windows, hwnd); it != windows.end()) windows.erase(it);
  ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
  ::DestroyWindow(hwnd);
  return {};
}
inline result<tuple<HWND, int4>> create(DWORD style, DWORD exstyle, const wchar_t* title) {
  const auto hwnd = ::CreateWindowExW(
    exstyle, wclass::name(), title, style, 0, 0, 0, 0, nullptr, nullptr, wclass::hinstance(), nullptr);
  if (!hwnd) return error(errors::operation_failed, "CreateWindowEx failed");
  RECT wr, cr;
  if (!::GetWindowRect(hwnd, &wr)) return error(errors::operation_failed, "GetWindowRect failed");
  if (!::GetClientRect(hwnd, &cr)) return error(errors::operation_failed, "GetClientRect failed");
  const auto left = (wr.right - wr.left - cr.right) / 2;
  window_system::windows.push_back(hwnd);
  return tuple(hwnd, int4(left, wr.bottom - wr.top - cr.bottom - left, left, left));
}
inline window* get_window_pointer(HWND hwnd) {
  const auto p = ::GetWindowLongPtrW(hwnd, GWLP_USERDATA);
  return reinterpret_cast<window*>(p);
}
inline void set_window_pointer(HWND hwnd, window* ptr) {
  ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(ptr));
}

inline int2 cursor_pos() {
  int2 pos;
  ::GetCursorPos(reinterpret_cast<POINT*>(&pos));
  return pos;
}
inline void cursor_pos(int2 pos) { ::SetCursorPos(pos.x(), pos.y()); }
} // namespace window_system

///--------------------------------------------------------------------------///
/// MARK: window

class window {
public:
  struct options {
    bool borderless : 1 = false;
    bool captionless : 1 = false;
    bool resizable : 1 = true;
    bool visible : 1 = true;
    DWORD get_style() const noexcept {
      DWORD style = captionless ? WS_POPUP : (WS_CAPTION | WS_SYSMENU);
      style |= !borderless * WS_BORDER;
      style |= resizable * WS_THICKFRAME;
      style |= visible * WS_VISIBLE;
      return style;
    }
    DWORD get_exstyle() const noexcept { return WS_EX_ACCEPTFILES; }
  };

  const_property<HWND, window> hwnd = nullptr;
  const_property<DWORD, window> style;
  const_property<DWORD, window> exstyle;
  const_property<uint4, window> frame_thickness;

  const_property<comptr<IDXGISwapChain1>, window> swap_chain;
  const_property<bitmap, window> control_layer;
  const_property<bitmap, window> overlay_layer;
  const_property<bitmap, window> underlay_layer;

  property<function<bool, yw::button_event>, window> button_event;
  property<function<bool, yw::drag_event>, window> drag_event;
  property<function<bool, yw::hover_event>, window> hover_event;
  property<function<bool, yw::key_event>, window> key_event;
  property<function<bool, yw::wheel_event>, window> wheel_event;

protected:
  friend class control;
  friend void window_system::make_dirty(HWND hwnd);
  friend void window_system::make_messy(HWND hwnd);
  friend const yw::color_theme* window_system::get_color_theme(HWND hwnd);
  friend result<void> window_system::attach_control(control& c, window& w);
  friend LRESULT CALLBACK wclass::wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

  yw::color_theme _color_theme;
  float _overlay_opacity = 0.15f;
  float _disabled_color_fade = 0.8f;

  bitmap _back_buffer;
  control::slotid _root_control_id{};

  TRACKMOUSEEVENT _track_mouse_event{sizeof(TRACKMOUSEEVENT), TME_LEAVE};
  int2 _previous_cursor_pos{};

  int2 _desired_size{};                  // 指定されたウィンドウサイズ
  int2 _current_size{};                  // 更新はWM_SIZEでのみ行われる
  bool2 _desired{};                      // ウィンドウサイズが指定されているか
  bool _dirty = false;                   // control_layerが再描画される必要があるか
  bool _messy = false;                   // control_layerのレイアウト再計算が必要な状態か
  bool _overlay_has_been_drawn = false;  // overlay_layerがユーザーによって描画済みか
  bool _underlay_has_been_drawn = false; // underlay_layerがユーザーによって描画済みか

  control::slotid _focused_control_id{};
  control::slotid _hovered_control_id{};
  control::slotid _pressed_control_id{};
  int _pressed_buttons = 0;
  bool _window_pressed = false;
  bool _window_resizing = false;

  /// _dirty/_messyフラグがあるならクリアする。
  result<void> _clean() {
    control* root = nullptr;
    if (const auto sp = window_system::controls.get(_root_control_id)) root = *sp;
    int2 area;
    if (auto res = size()) area = *res;
    else return res.relay();
    if (root && _messy) {
      int2 minimum;
      if (auto res = root->_calculate_minimum_area()) minimum = vapply_r<int2>(yw::ceil, *res);
      else return res.relay();
      const auto required = vapply_r<int2>(yw::max, area, minimum);
      if (required != area) {
        if (auto res = size(required); !res) return res.relay();
        area = required;
      }
    }
    if (area.x() <= 0 || area.y() <= 0) return {};
    if (auto res = _update_swapchain(area); !res) return res.relay();
    if (auto res = _update_render_targets(area); !res) return res.relay();
    if (root && _messy) {
      if (auto res = root->_update_layout({}, area); !res) return res.relay();
      if (auto res = root->_update_geometry(); !res) return res.relay();
      _dirty = true;
    }
    if (_dirty) {
      if (auto d = control_layer.ref().begin_draw(colors::transparent)) {
        if (root)
          if (auto res = root->_draw(*this); !res) return res.relay();
        if (auto res = d->close(); !res) return res.relay();
      } else return d.relay();
    }
    _dirty = false;
    _messy = false;
    return {};
  }

  result<void> _draw_layer() { return _compose(_back_buffer, true, true); }

  result<void> _compose(bitmap& target, bool control, bool overlay) {
    auto d = target.begin_draw(_color_theme.canvas);
    if (!d) return d.relay();
    if (_underlay_has_been_drawn)
      if (auto res = draw_bitmap({}, underlay_layer()); !res) return res.relay();
    if (control)
      if (auto res = draw_bitmap({}, control_layer()); !res) return res.relay();
    if (overlay && _overlay_has_been_drawn)
      if (auto res = draw_bitmap({}, overlay_layer()); !res) return res.relay();
    return d->close();
  }

  result<void> _update_swapchain(uint2 Size) {
    if (!swap_chain()) {
      auto d = DXGI_SWAP_CHAIN_DESC1(Size[0], Size[1], bitmap::dxgiformat, false, DXGI_SAMPLE_DESC(1, 0), {}, 2);
      d.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT, d.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
      const auto hr = dxgi::factory()->CreateSwapChainForHwnd(d3d::device(), hwnd(), &d, 0, 0, &swap_chain.ref().get());
      if (FAILED(hr)) return error(errors::operation_failed, "CreateSwapChainForHwnd failed");
    } else {
      DXGI_SWAP_CHAIN_DESC1 d;
      if (const auto hr = swap_chain()->GetDesc1(&d); FAILED(hr))
        return error(errors::operation_failed, "GetDesc1 failed");
      if (d.Width == Size[0] && d.Height == Size[1]) return {};
      _back_buffer = bitmap();
      if (const auto hr = swap_chain()->ResizeBuffers(2, Size[0], Size[1], bitmap::dxgiformat, 0); FAILED(hr))
        return error(errors::operation_failed, "ResizeBuffers failed");
    }
    _dirty = true;
    return {};
  }

  result<void> _update_render_targets(uint2 Size) {
    if (!_back_buffer || _back_buffer.size() != Size) {
      if (auto res = bitmap::create_from_swapchain(swap_chain().get())) _back_buffer = move(*res);
      else return res.relay();
    }
    if (!underlay_layer() || underlay_layer().size() != Size) {
      if (auto res = bitmap::create(Size)) underlay_layer = move(*res);
      else return res.relay();
      _underlay_has_been_drawn = false;
    }
    if (!overlay_layer() || overlay_layer().size() != Size) {
      if (auto res = bitmap::create(Size)) overlay_layer = move(*res);
      else return res.relay();
      _overlay_has_been_drawn = false;
    }
    if (!control_layer() || control_layer().size() != Size) {
      if (auto res = bitmap::create(Size)) control_layer = move(*res);
      else return res.relay();
    }
    return {};
  }

  // Keep this list in member declaration order; update it when adding members.
  void _move_from(window&& o) noexcept {
    hwnd = exchange(o.hwnd, {});
    if (hwnd()) window_system::set_window_pointer(hwnd(), this);
    style = o.style;
    exstyle = o.exstyle;
    frame_thickness = o.frame_thickness;

    swap_chain = move(o.swap_chain);
    control_layer = move(o.control_layer);
    overlay_layer = move(o.overlay_layer);
    underlay_layer = move(o.underlay_layer);

    button_event = move(o.button_event.ref());
    drag_event = move(o.drag_event.ref());
    hover_event = move(o.hover_event.ref());
    key_event = move(o.key_event.ref());
    wheel_event = move(o.wheel_event.ref());

    _color_theme = o._color_theme;
    _overlay_opacity = o._overlay_opacity;

    _back_buffer = move(o._back_buffer);
    _root_control_id = exchange(o._root_control_id, {});

    _track_mouse_event = exchange(o._track_mouse_event, TRACKMOUSEEVENT{sizeof(TRACKMOUSEEVENT), TME_LEAVE});
    _previous_cursor_pos = exchange(o._previous_cursor_pos, {});

    _desired_size = o._desired_size;
    _current_size = o._current_size;
    _desired = o._desired;
    _dirty = o._dirty;
    _messy = o._messy;
    _overlay_has_been_drawn = o._overlay_has_been_drawn;
    _underlay_has_been_drawn = o._underlay_has_been_drawn;

    _focused_control_id = exchange(o._focused_control_id, {});
    _hovered_control_id = exchange(o._hovered_control_id, {});
    _pressed_control_id = exchange(o._pressed_control_id, {});
    _pressed_buttons = exchange(o._pressed_buttons, 0);
    _window_pressed = exchange(o._window_pressed, false);
    _window_resizing = exchange(o._window_resizing, false);
  }

  result<bool> _handle_button_event(yw::button_event e) {
    bool handled = false;
    if (const auto capture_cp = window_system::get_control(_pressed_control_id)) { // コントロールへの長押し有効中
      const auto hit_cid = _get_hit_control_id(e.pos);
      const auto old_pressed_control_id = _pressed_control_id;
      if (auto res = _press_changed({}); !res) return res.relay();
      if (e.state.down) { // 別のボタンが押された->キャプチャ無効化
        _pressed_buttons |= 1 << int(e.key.code);
        if (const auto hit_cp = window_system::get_control(hit_cid)) {
          const auto focus_cid = (hit_cp && hit_cp->focusable()) ? hit_cid : control::slotid();
          if (auto res = _focus_changed(_focused_control_id, focus_cid); !res) return res.relay();
          if (auto res = hit_cp->_handle_button_event(*this, e)) handled = *res;
          else return res.relay();
        }
      } else { // ボタンが解放された
        _pressed_buttons &= ~(1 << int(e.key.code));
        if (auto res = capture_cp->_handle_button_event(*this, e)) handled = *res;
        else return res.relay();
        if (hit_cid == old_pressed_control_id) {
          if (auto res = capture_cp->_handle_click_event(*this, e)) handled |= *res;
          else return res.relay();
        }
        if (_pressed_buttons == 0) ::ReleaseCapture();
      }
    } else if (_window_pressed) { // ウィンドウへの長押し有効中
      if (auto res = _press_changed({}); !res) return res.relay();
      if (e.state.down) { // 別のボタンが押された->長押し無効化
        _pressed_buttons |= 1 << int(e.key.code);
        const auto hit_cid = _get_hit_control_id(e.pos);
        if (const auto hit_cp = window_system::get_control(hit_cid)) {
          const auto focus_cid = hit_cp->focusable() ? hit_cid : control::slotid();
          if (auto res = _focus_changed(_focused_control_id, focus_cid); !res) return res.relay();
          if (auto res = hit_cp->_handle_button_event(*this, e)) handled = *res;
          else return res.relay();
        }
      } else { // ボタンが解放された
        _pressed_buttons &= ~(1 << int(e.key.code));
        if (_pressed_buttons == 0) ::ReleaseCapture();
      }
    } else if (_pressed_buttons != 0) { // キャプチャ無効化中
      const auto hit_cid = _get_hit_control_id(e.pos);
      if (_pressed_control_id)
        if (auto res = _press_changed({}); !res) return res.relay();
      if (e.state.down) {
        _pressed_buttons |= 1 << int(e.key.code);
        if (const auto hit_cp = window_system::get_control(hit_cid)) {
          const auto focus_cid = (hit_cp && hit_cp->focusable()) ? hit_cid : control::slotid();
          if (auto res = _focus_changed(_focused_control_id, focus_cid); !res) return res.relay();
          if (auto res = hit_cp->_handle_button_event(*this, e)) handled = *res;
          else return res.relay();
        }
      } else {
        _pressed_buttons &= ~(1 << int(e.key.code));
        if (const auto hit_cp = window_system::get_control(hit_cid)) {
          const auto focus_cid = (hit_cp && hit_cp->focusable()) ? hit_cid : control::slotid();
          if (auto res = _focus_changed(_focused_control_id, focus_cid); !res) return res.relay();
          if (auto res = hit_cp->_handle_button_event(*this, e)) handled = *res;
          else return res.relay();
        }
        if (_pressed_buttons == 0) ::ReleaseCapture();
      }
    } else if (e.state.down) { // 新しくキャプチャされる場合
      _pressed_buttons |= 1 << int(e.key.code);
      const auto hit_cid = _get_hit_control_id(e.pos);
      if (const auto hit_cp = window_system::get_control(hit_cid)) {
        if (auto res = _press_changed(hit_cid); !res) return res.relay();
        if (auto res = _focus_changed(_focused_control_id, hit_cid); !res) return res.relay();
        if (auto res = hit_cp->_handle_button_event(*this, e)) handled = *res;
        else return res.relay();
      } else {
        _window_pressed = true;
        if (auto res = _focus_changed(_focused_control_id, {}); !res) return res.relay();
      }
      ::SetCapture(hwnd);
    }
    if (!handled && button_event()) return button_event.ref()(e);
    return handled;
  }

  result<bool> _handle_char_event(wchar_t c) {
    if (const auto focus_cp = window_system::get_control(_focused_control_id)) {
      if (auto res = focus_cp->_handle_char_event(*this, c)) return *res;
      else return res.relay();
    } else return false;
  }

  result<bool> _handle_double_click_event(yw::button_event e) {
    if (const auto root_cp = window_system::get_control(_root_control_id)) {
      const auto hit_cid = root_cp->_hit_test(e.pos);
      if (const auto hit_cp = window_system::get_control(hit_cid)) {
        if (auto res = hit_cp->_handle_double_click_event(*this, e); !res) return res.relay();
        else return *res;
      } else return false;
    } else return false;
  }

  result<bool> _handle_drag_event(yw::drag_event e) {
    bool handled = false;
    if (const auto capture_cp = window_system::get_control(_pressed_control_id)) {
      if (auto res = capture_cp->_handle_drag_event(*this, e); !res) return res.relay();
      else handled = *res;
    }
    if (!handled && drag_event()) return drag_event.ref()(e);
    return handled;
  }

  result<bool> _handle_hover_event(yw::hover_event e) {
    if (e.state == hover_event::leave) { // ウィンドウの外に出た
      if (const auto old_hover_cp = window_system::get_control(_hovered_control_id)) {
        _hovered_control_id = {};
        _dirty = true;
        bool leave_handled = false;
        if (auto res = old_hover_cp->_handle_hover_event(*this, {e.pos, hover_event::leave})) leave_handled = *res;
        else return res.relay();
        if (!leave_handled && hover_event()) return hover_event.ref()({e.pos, hover_event::leave});
        return leave_handled;
      } else if (hover_event()) return hover_event.ref()({e.pos, hover_event::leave});
      else return false;
    }
    const auto hit_cid = _get_hit_control_id(e.pos);
    if (hit_cid == _hovered_control_id) {
      bool handled = false;
      if (const auto hit_cp = window_system::get_control(hit_cid)) {
        if (auto res = hit_cp->_handle_hover_event(*this, {e.pos, hover_event::hover})) handled = *res;
        else return res.relay();
      }
      if (!handled && hover_event()) return hover_event.ref()({e.pos, hover_event::hover});
      return handled;
    }
    _dirty = true;
    bool leave_handled = false, enter_handled = false;
    if (const auto old_hover_cp = window_system::get_control(_hovered_control_id)) {
      if (auto res = old_hover_cp->_handle_hover_event(*this, {e.pos, hover_event::leave})) leave_handled = *res;
      else return res.relay();
    }
    if (const auto hit_cp = window_system::get_control(hit_cid)) {
      _hovered_control_id = hit_cid;
      if (auto res = hit_cp->_handle_hover_event(*this, {e.pos, hover_event::enter})) enter_handled = *res;
      else return res.relay();
    } else _hovered_control_id = {};
    if (leave_handled) {
      if (!enter_handled && hover_event()) hover_event.ref()({e.pos, hover_event::enter});
      return true;
    }
    if (enter_handled) {
      if (hover_event()) hover_event.ref()({e.pos, hover_event::leave});
      return true;
    }
    if (hover_event()) return hover_event.ref()({e.pos, hover_event::hover});
    return false;
  }

  result<bool> _handle_key_event(yw::key_event e) {
    bool handled = false;
    if (const auto focus_cp = window_system::get_control(_focused_control_id)) {
      if (auto res = focus_cp->_handle_key_event(*this, e); !res) return res.relay();
      else handled = *res;
      if (!handled && e.state.down && !e.state.ctrl && !e.state.alt) {
        if (e.key == keys::tab) {
          if (auto res = _tab_pressed(e.state.shift); !res) return res.relay();
          handled = true;
        } else if (e.key == keys::escape) {
          if (auto rse = _focus_changed(_focused_control_id, {}); !rse) return rse.relay();
          handled = true;
        }
      }
      if (!handled && key_event()) handled = key_event.ref()(e);
      return handled;
    } else { // フォーカス中のコントロールがない場合
      if (e.state.down && e.key == keys::tab && !e.state.ctrl && !e.state.alt) {
        if (auto res = _tab_pressed(e.state.shift); !res) return res.relay();
        handled = true;
      }
      if (!handled && key_event()) handled = key_event.ref()(e);
      return handled;
    }
  }

  result<bool> _handle_wheel_event(yw::wheel_event e) {
    bool handled = false;
    const auto hit_cid = _get_hit_control_id(e.pos);
    if (const auto hover_cp = window_system::get_control(hit_cid)) {
      if (auto res = hover_cp->_handle_wheel_event(*this, e); !res) return res.relay();
      else handled = *res;
    }
    if (!handled && wheel_event()) return wheel_event.ref()(e);
    return handled;
  }

  result<void> _focus_changed(control::slotid Old, control::slotid New) {
    if (const auto cp = window_system::get_control(New); cp && (!cp->visible() || !cp->enabled() || !cp->focusable()))
      New = {};
    if (Old == New) return {};
    if (const auto old_cp = window_system::get_control(Old))
      if (auto res = old_cp->_handle_focus_event(*this, {false}); !res) return res.relay();
    if (const auto new_cp = window_system::get_control(New))
      if (auto res = new_cp->_handle_focus_event(*this, {true}); !res) return res.relay();
    _focused_control_id = New;
    _dirty = true;
    return {};
  }

  result<void> _press_changed(control::slotid New) {
    _pressed_control_id = New;
    _window_pressed = false;
    _dirty = true;
    return {};
  }
  result<void> _press_changed() {
    _pressed_control_id = {};
    _window_pressed = true;
    _dirty = true;
    return {};
  }

  control::slotid _get_hit_control_id(float2 Pt) const {
    if (const auto root_cp = window_system::get_control(_root_control_id)) return root_cp->_hit_test(Pt);
    return {};
  }

  result<void> _tab_pressed(bool Shift) {
    const auto root_cp = window_system::get_control(_root_control_id);
    if (!root_cp) return {};
    bool found = !bool(window_system::get_control(_focused_control_id));
    const auto next_cid = root_cp->_find_tab_stop(_focused_control_id, Shift, found);
    if (auto res = _focus_changed(_focused_control_id, next_cid); !res) return res.relay();
    return {};
  }

  result<int2> _get_minimum_size() {
    if (const auto root_cp = window_system::get_control(_root_control_id)) {
      if (auto res = root_cp->_calculate_minimum_area(); res) return *res;
      else return res.relay();
    } else return {};
  }

public:
  result<drawing> begin_draw_overlay() {
    if (auto res = overlay_layer.ref().begin_draw(colors::transparent)) {
      _overlay_has_been_drawn = true;
      return move(*res);
    } else return res.relay();
  }

  result<drawing> begin_draw_underlay() {
    if (auto res = underlay_layer.ref().begin_draw(_color_theme.canvas)) {
      _underlay_has_been_drawn = true;
      return move(*res);
    } else return res.relay();
  }

  /// Captures the client area, optionally including controls and the overlay.
  /// Finish any active drawing before calling this function.
  result<bitmap> screenshot(bool control = true, bool overlay = true) {
    if (!hwnd()) return error(errors::not_initialized, "Window is closed");
    if (drawing::target_exists()) return error(errors::invalid_operation, "Drawing is still active");
    if (auto res = _clean(); !res) return res.relay();
    if (!_back_buffer) return error(errors::not_initialized, "Window has no render target");
    if (auto captured = bitmap::create(_back_buffer.size()); !captured) return captured.relay();
    else if (auto res = _compose(*captured, control, overlay); !res) return res.relay();
    else return move(*captured);
  }

  ~window() { close(); }
  window() = default;

  window(window&& o) noexcept { _move_from(std::move(o)); }

  window& operator=(window&& o) noexcept {
    if (this == &o) return *this;
    if (hwnd) close();
    _move_from(std::move(o));
    return *this;
  }

  static result<window> create(stringable auto&& Title, optional<int2> Size = {}, options Options = {}) {
    const auto title = unicode<wchar_t>(static_cast<decltype(Title)&&>(Title));
    const auto visible = Options.visible;
    const auto style = Options.get_style() ^ (visible ? WS_VISIBLE : 0); // remove WS_VISIBLE for delayed show
    const auto exstyle = Options.get_exstyle();
    window win;
    if (auto res = window_system::create(style, exstyle, title.c_str()); !res) return res.relay();
    else win.hwnd = res->first, win.style = style, win.exstyle = exstyle, win.frame_thickness = res->second;
    window_system::set_window_pointer(win.hwnd(), &win);
    if (Size)
      if (auto res = win.size(*Size); !res) return res.relay();
    if (auto res = win.update(); !res) return res.relay();
    if (visible) ::ShowWindow(win.hwnd(), SW_SHOW);
    return win;
  }

  window(
    stringable auto&& Title, optional<int2> Size = {}, options Options = {}, const std::source_location& sl = here()) {
    if (auto res = create(Title, Size, Options)) *this = move(*res);
    else res.error().add_footprint().print_and_abort(sl);
  }

  result<void> close() {
    if (!hwnd()) return {};
    for (auto c : window_system::controls)
      if (c && c->_window == hwnd()) c->_window = nullptr;
    _root_control_id = {};
    if (auto res = window_system::destroy(hwnd()); !res) return res.relay();
    hwnd = nullptr;
    _back_buffer = bitmap();
    control_layer = bitmap();
    overlay_layer = bitmap();
    underlay_layer = bitmap();
    swap_chain.ref().release();
    return {};
  }

  /// returns the position of the client area of the window.
  result<int2> pos() {
    if (RECT rect; ::GetWindowRect(hwnd(), &rect)) return int2(rect.left, rect.top) + frame_thickness().xy();
    else return error(errors::operation_failed, "GetWindowRect failed");
  }

  /// sets the position of the client area of the window.
  result<void> pos(int2 Pos) {
    const auto pos = Pos - frame_thickness().xy();
    if (::SetWindowPos(hwnd(), nullptr, pos.x(), pos.y(), 0, 0, SWP_NOSIZE | SWP_NOZORDER)) return {};
    else return error(errors::operation_failed, "SetWindowPos failed");
  }

  /// returns the size of the client area of the window.
  result<uint2> size() {
    if (RECT rect; ::GetClientRect(hwnd(), &rect)) return uint2(rect.right, rect.bottom);
    else return error(errors::operation_failed, "GetClientRect failed");
  }
  /// sets the size of the client area of the window.
  result<void> size(int2 Size) {
    if (Size.x() <= 0 || Size.y() <= 0) return error(errors::invalid_argument, "invalid window size");
    _desired_size = Size;
    _desired = true;
    int2 size = Size + frame_thickness().xy() + frame_thickness().zw();
    if (::SetWindowPos(hwnd(), nullptr, 0, 0, size.x(), size.y(), SWP_NOMOVE | SWP_NOZORDER)) return {};
    else return error(errors::operation_failed, "SetWindowPos failed");
  }

  result<void> update() {
    if (!hwnd() || ::IsIconic(hwnd())) return {};
    if (auto res = _clean(); !res) return res.relay();
    if (_current_size.x() <= 0 || _current_size.y() <= 0) return {};
    if (auto res = _draw_layer(); !res) return res.relay();
    if (const auto hr = swap_chain()->Present(1, 0); FAILED(hr))
      return error(errors::operation_failed, "Present failed");
    return {};
  }

  const yw::color_theme& color_theme() const { return _color_theme; }
  result<void> color_theme(const yw::color_theme& ct) {
    _color_theme = ct;
    _dirty = true;
    return {};
  }

  float overlay_opacity() const { return _overlay_opacity; }
  result<void> overlay_opacity(float opacity) {
    _overlay_opacity = opacity;
    _dirty = true;
    return {};
  }

  float disabled_color_fade() const { return _disabled_color_fade; }
  result<void> disabled_color_fade(float fade) {
    _disabled_color_fade = fade;
    _dirty = true;
    return {};
  }
};

///--------------------------------------------------------------------------///
/// MARK: from control.h

inline void control::_clear_state(window& win) noexcept {
  if (win._focused_control_id == id()) win._focused_control_id = {};
  if (win._hovered_control_id == id()) win._hovered_control_id = {};
  if (win._pressed_control_id == id()) win._pressed_control_id = {};
  win._dirty = true;
}

inline void control::_clear() noexcept {
  if (const auto win = window_system::get_window_pointer(_window)) {
    _clear_state(*win);
    if (win->_root_control_id == id()) win->_root_control_id = {};
  }
  if (const auto sp = window_system::controls.get(id)) window_system::controls.erase(id);
}

inline result<bool> control::focus() {
  if (!_visible || !_enabled || !focusable()) return false;
  const auto win = window_system::get_window_pointer(_window);
  if (!win) return false;
  if (auto res = win->_focus_changed(win->_focused_control_id, id()); !res) return res.relay();
  return win->_focused_control_id == id();
}

inline result<void> control::visible(bool value) {
  if (_visible == value) return {};
  _visible = value;
  if (!value)
    if (const auto win = window_system::get_window_pointer(_window)) _clear_state(*win);
  window_system::make_messy(_window);
  window_system::make_dirty(_window);
  return {};
}

inline result<void> control::enabled(bool value) {
  if (_enabled == value) return {};
  _enabled = value;
  if (!value)
    if (const auto win = window_system::get_window_pointer(_window)) _clear_state(*win);
  window_system::make_dirty(_window);
  return {};
}

inline result<void> control::attach_to(window& w) {
  if (!window_system::get_window_pointer(w.hwnd())) return error(errors::operation_failed, "Window not found");
  if (const auto old = window_system::get_window_pointer(_window)) {
    _clear_state(*old);
    if (_window != w.hwnd() && old->_root_control_id == id()) {
      old->_root_control_id = {};
      old->_dirty = true;
    }
  }
  if (auto res = window_system::attach_control(*this, w); !res) return res.relay();
  _window = w.hwnd();
  _clear_state(w);
  return {};
}

inline result<void> control::attach_to(control& c) {
  if (auto res = c._attach(*this); !res) return res.relay();
  if (const auto old = window_system::get_window_pointer(_window)) {
    _clear_state(*old);
    if (old->_root_control_id == id()) old->_root_control_id = {};
  }
  _window = c._window;
  if (const auto win = window_system::get_window_pointer(_window)) _clear_state(*win);
  return {};
}

inline bool control::_focused(window& win) const { return win._focused_control_id == id() && focusable(); }
inline bool control::_hovered(window& win) const {
  return win._hovered_control_id == id() && (focusable() || interactive());
}
inline bool control::_pressed(window& win) const { return win._pressed_control_id == id() && interactive(); }

inline color control::_get_disabled_color(window& win, const color& c) {
  return vapply_r<color>(yw::lerp, c, colors::gray, projector(win._disabled_color_fade, sequence<0, 0, 0, 0>()));
}

inline void window_system::make_dirty(HWND hwnd) {
  if (const auto win = get_window_pointer(hwnd)) win->_dirty = true;
}

inline void window_system::make_messy(HWND hwnd) {
  if (const auto win = get_window_pointer(hwnd)) win->_messy = true;
}

inline const color_theme* window_system::get_color_theme(HWND hwnd) {
  if (const auto win = get_window_pointer(hwnd)) return &win->_color_theme;
  return nullptr;
}

inline result<void> window_system::attach_control(control& c, window& w) {
  if (const auto win = get_window_pointer(w.hwnd())) win->_root_control_id = c.id(), win->_messy = true;
  else return error(errors::operation_failed, "Window not found");
  return {};
}
} // namespace yw
