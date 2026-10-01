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
  static LRESULT __stdcall wndproc(HWND hw, UINT msg, WPARAM wp, LPARAM lp);
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
array<HWND> windows;
inline result<void> destroy(HWND hwnd) {
  if (!hwnd) return {};
  windows.erase(std::ranges::find(windows, hwnd));
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

  struct color_theme {
    color canvas = color(0xf0f0f0);        // ex) background of window
    color surface = color(0xf8f8f8);       // ex) background of control
    color surface_popup = color(0xffffff); // ex) background of tooltip
    color outline = color_name::black;     // ex) border of control
    color part = color_name::gray;         // ex) button of checkbox, thumb of scrollbar
    color text = color_name::black;        // ex) text, icon
    color text_muted = color_name::gray;   // ex) placeholder text
    color accent = color_name::dodgerblue; // ex) focus, selection
    color warning = color_name::orange;
    color error = color_name::red;
    color success = color_name::green;
  };

  const_property<HWND, window> hwnd;
  const_property<DWORD, window> style;
  const_property<DWORD, window> exstyle;
  const_property<uint4, window> frame_thickness;

  const_property<comptr<IDXGISwapChain1>, window> swap_chain;
  const_property<bitmap, window> control_layer;
  const_property<bitmap, window> overlay_layer;
  const_property<bitmap, window> underlay_layer;

  result<drawing> begin_draw_overlay() {
    if (auto res = overlay_layer.ref().begin_draw(color_name::transparent)) {
      _overlay_has_been_drawn = true;
      return move(*res);
    } else return res.relay();
  }

  result<drawing> begin_draw_underlay() {
    if (auto res = underlay_layer.ref().begin_draw(_theme.canvas)) {
      _underlay_has_been_drawn = true;
      return move(*res);
    } else return res.relay();
  }

  ~window() { window_system::destroy(hwnd()); }
  window() = default;

  window(window&& o) noexcept
    : hwnd(exchange(o.hwnd.ref(), {})), style(o.style), exstyle(o.exstyle), //
      frame_thickness(o.frame_thickness), _theme(o._theme) {
    window_system::set_window_pointer(hwnd(), this);
  }

  window& operator=(window&& o) noexcept {
    if (this == &o) return *this;
    if (hwnd) window_system::destroy(hwnd());
    hwnd = exchange(o.hwnd.ref(), {});
    style = o.style;
    exstyle = o.exstyle;
    frame_thickness = o.frame_thickness;
    _theme = o._theme;
    window_system::set_window_pointer(hwnd(), this);
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
    if (Size) win.size(*Size);
    ::SetWindowLongPtrW(win.hwnd(), GWLP_USERDATA, reinterpret_cast<LONG_PTR>(&win));
    if (auto res = win.update(); !res) return res.relay();
    if (visible) ::ShowWindow(win.hwnd(), SW_SHOW);
    return win;
  }

  window(stringable auto&& Title, optional<int2> Size, options Options = {}, const std::source_location& sl = here()) {
    if (auto res = create(Title, Size, Options)) *this = move(*res);
    else res.error().add_footprint().print_and_abort(sl);
  }

  result<void> close() {
    // subwindows.clear();
    // _root_control should be cleared when the window is closed
    if (auto res = window_system::destroy(hwnd()); !res) return res.relay();
    return {};
  }

  /// returns the size of the client area of the window.
  result<uint2> size() {
    print(hwnd());
    if (RECT rect; ::GetClientRect(hwnd(), &rect)) return uint2(rect.right, rect.bottom);
    else return error(errors::operation_failed, "GetClientRect failed");
  }
  /// sets the size of the client area of the window.
  result<void> size(int2 Size) {
    int2 size = Size + frame_thickness().xy() + frame_thickness().zw();
    if (::SetWindowPos(hwnd(), nullptr, 0, 0, size.x(), size.y(), SWP_NOMOVE | SWP_NOZORDER)) return {};
    else return error(errors::operation_failed, "SetWindowPos failed");
  }

  result<void> update() {
    uint2 current_size;
    if (auto res = size()) current_size = *res;
    else return res.relay();
    // コントロールの配置位置、必要画面サイズを更新
    if (auto res = _update_controls(current_size); !res) return res.relay();
    else current_size = *res;
    if (auto res = _update_swapchain(current_size); !res) return res.relay();
    if (auto res = _update_render_targets(current_size); !res) return res.relay();
    // コントロールを描画
    if (auto res = _draw_control(); !res) return res.relay();
    // underlay layerに集約
    if (_overlay_has_been_drawn) if (auto res = _draw_layer<true>(); !res) return res.relay();

    return {};
  }

private:
  color_theme _theme;
  control* _root_control = nullptr;
  bool _dirty = false;
  bool _messy = false;
  bool _overlay_has_been_drawn = false;
  bool _underlay_has_been_drawn = false;

  result<void> _draw_control() {
    if (auto d = control_layer.ref().begin_draw(color_name::transparent)) {
      if (_root_control)
        if (auto res = _root_control->_draw(); !res) return res.relay();
      if (auto res = d.close(); !res) return res.relay();
    } else return d.relay();
    return {};
  }

  template<bool Drawn> result<void> _draw_layer() {
    drawing d;
    if constexpr (Drawn) d = underlay_layer.ref().begin_draw();
    else d = underlay_layer.ref().begin_draw(color_name::transparent);
    if (!d) return d.relay();
    if (auto res = draw_bitmap({}, control_layer()); !res) return res.relay();
    if (auto res = draw_bitmap({}, overlay_layer()); !res) return res.relay();
    if (auto res = d.close(); !res) return res.relay();
    return {};
  }

  result<uint2> _update_controls(uint2 Size) {
    if (!_root_control) return Size;
    if (auto res = _root_control->_update_geometry(Size)) return *res;
    else return res.relay();
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
    }
    control_layer = bitmap(), overlay_layer = bitmap(), underlay_layer = bitmap();
    _dirty = true;
    return {};
  }

  result<void> _update_render_targets(uint2 Size) {
    if (underlay_layer().size() != Size) {
      if (auto res = bitmap::create_from_swapchain(swap_chain().get())) underlay_layer = move(*res);
      else return res.relay();
    }
    if (overlay_layer().size() != Size) {
      if (auto res = bitmap::create(Size)) overlay_layer = move(*res);
      else return res.relay();
    }
    if (control_layer().size() != Size) {
      if (auto res = bitmap::create(Size)) control_layer = move(*res);
      else return res.relay();
    }
    return {};
  }
};

///--------------------------------------------------------------------------///
/// MARK: wndproc

LRESULT CALLBACK wclass::wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
  switch (msg) {
  case WM_DESTROY: ::PostQuitMessage(0); return 0;
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
