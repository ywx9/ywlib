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
    auto captured = bitmap::create(_back_buffer.size());
    if (!captured) return captured.relay();
    if (auto res = _compose(*captured, control, overlay); !res) return res.relay();
    return move(*captured);
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

  static result<window> create(stringable auto&& Title, optional<int2> Size, options Options = {}) {
    const auto title = unicode<wchar_t>(static_cast<decltype(Title)&&>(Title));
    const auto visible = Options.visible;
    const auto style = Options.get_style() ^ (visible ? WS_VISIBLE : 0); // remove WS_VISIBLE for delayed show
    const auto exstyle = Options.get_exstyle();
    window win;
    if (auto res = window_system::create(style, exstyle, title.c_str()); !res) return res.relay();
    else win.hwnd = res->first, win.style = style, win.exstyle = exstyle, win.frame_thickness = res->second;
    window_system::set_window_pointer(win.hwnd(), &win);
    if (auto res = win.size(Size ? *Size : int2(640, 480)); !res) return res.relay();
    if (auto res = win.update(); !res) return res.relay();
    if (visible) ::ShowWindow(win.hwnd(), SW_SHOW);
    return win;
  }

  window(stringable auto&& Title, optional<int2> Size, options Options = {}, const std::source_location& sl = here()) {
    if (auto res = create(Title, Size, Options)) *this = move(*res);
    else res.error().add_footprint().print_and_abort(sl);
  }

  result<void> close() {
    if (!hwnd()) return {};
    for (auto c : window_system::controls)
      if (c && c->_window == hwnd()) c->_window = nullptr;
    _root_control = {};
    if (auto res = window_system::destroy(hwnd()); !res) return res.relay();
    hwnd = nullptr;
    _back_buffer = bitmap();
    control_layer = bitmap();
    overlay_layer = bitmap();
    underlay_layer = bitmap();
    swap_chain.ref().release();
    return {};
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

private:
  friend class control;
  friend void window_system::make_dirty(HWND hwnd);
  friend void window_system::make_messy(HWND hwnd);
  friend const yw::color_theme* window_system::get_color_theme(HWND hwnd);
  friend result<void> window_system::attach_control(control& c, window& w);
  friend LRESULT CALLBACK wclass::wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

  yw::color_theme _color_theme;
  bitmap _back_buffer;
  control::slotid _root_control{};
  int2 _desired_size{};                    // 指定されたウィンドウサイズ
  int2 _current_size{};                    // 更新はWM_SIZEでのみ行われる
  bool2 _desired{};                        // ウィンドウサイズが指定されているか
  bool _dirty = false;                   // control_layerが再描画される必要があるか
  bool _messy = false;                   // control_layerのレイアウト再計算が必要な状態か
  bool _overlay_has_been_drawn = false;  // overlay_layerがユーザーによって描画済みか
  bool _underlay_has_been_drawn = false; // underlay_layerがユーザーによって描画済みか

  /// _dirty/_messyフラグがあるならクリアする。
  result<void> _clean() {
    control* root = nullptr;
    if (const auto sp = window_system::controls.get(_root_control)) root = *sp;
    int2 area;
    if (auto res = size()) area = *res;
    else return res.relay();
    if (area.x() <= 0 || area.y() <= 0) return {};
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
          if (auto res = root->_draw(); !res) return res.relay();
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

  void _move_from(window&& o) noexcept {
    hwnd = exchange(o.hwnd, {});
    window_system::set_window_pointer(hwnd, this);
    style = o.style;
    exstyle = o.exstyle;
    frame_thickness = o.frame_thickness;

    _back_buffer = move(o._back_buffer);
    swap_chain = move(o.swap_chain);
    control_layer = move(o.control_layer);
    overlay_layer = move(o.overlay_layer);
    underlay_layer = move(o.underlay_layer);

    _color_theme = o._color_theme;
    _root_control = exchange(o._root_control, {});
    _desired_size = o._desired_size;
    _current_size = o._current_size;
    _desired = o._desired;
    _dirty = o._dirty;
    _messy = o._messy;
    _overlay_has_been_drawn = o._overlay_has_been_drawn;
    _underlay_has_been_drawn = o._underlay_has_been_drawn;
  }
};

///--------------------------------------------------------------------------///
/// MARK: from control.h

inline result<void> control::attach_to(window& w) {
  if (auto res = window_system::attach_control(*this, w); !res) return res.relay();
  if (_window != w.hwnd()) {
    if (const auto old = window_system::get_window_pointer(_window); old && old->_root_control == id()) {
      old->_root_control = {};
      old->_dirty = true;
    }
  }
  _window = w.hwnd();
  return {};
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
  if (const auto win = get_window_pointer(w.hwnd())) win->_root_control = c.id(), win->_messy = true;
  else return error(errors::operation_failed, "Window not found");
  return {};
}

///--------------------------------------------------------------------------///
/// MARK: wndproc

LRESULT CALLBACK wclass::wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
  const auto win = window_system::get_window_pointer(hwnd);
  if (!win) {
    if (msg == WM_DESTROY && window_system::windows.empty()) return ::PostQuitMessage(0), 0;
    else return ::DefWindowProcW(hwnd, msg, wp, lp);
  }
  switch (msg) {
  case WM_SIZE:
    win->_current_size = uint2(LOWORD(lp), HIWORD(lp));
    win->_messy = true;
    break;
  case WM_CLOSE: win->close(); return 0;
  }
  return ::DefWindowProcW(hwnd, msg, wp, lp);
}

///--------------------------------------------------------------------------///
/// MARK: mainloop

class mainloop {
  inline static uint64_t _frame_count = 0;
  inline static double _last_elapsed = 0.0;

public:
  inline static const_property<yw::stopwatch, mainloop> stopwatch = {};
  inline static const_property<bool, mainloop> running = false;
  explicit operator bool() const noexcept { return running(); }
  inline static const_property<double, mainloop> spf = 0.0;
  static double fps() noexcept { return spf() > 0.0 ? 1.0 / spf() : 0.0; }

  mainloop() {
    if (window_system::windows.empty()) {
      running = false;
      return;
    }
    if (!running) {
      stopwatch.ref().restart();
      _frame_count = 0;
      _last_elapsed = 0.0;
      spf = 0.0;
    }
    const auto now = stopwatch().elapsed();
    spf = now - _last_elapsed, _last_elapsed = now;
    for (MSG msg; ::PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE);)
      if (msg.message == WM_QUIT) {
        running = false;
        return;
      } else ::TranslateMessage(&msg), ::DispatchMessageW(&msg);
    for (auto hwnd : window_system::windows)
      if (const auto win = window_system::get_window_pointer(hwnd); win)
        if (auto res = win->update(); !res) res.error().print_and_abort();
    ++_frame_count;
    running = true;
  }
};
} // namespace yw
