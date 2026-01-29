#pragma once
#include "ywx/bitmap.h"
#include "ywx/core.h"

namespace yw {

class window;
class control;
class subwindow;

bool mainloop();

//////////////////////////////////////// MARK: window_class

inline class {
  bool _initialized{false};

public:
  const null_terminated<wchar_t> name = L"ywlib_window_class";
  const HINSTANCE hinstance = ::GetModuleHandleW(nullptr);

  static LRESULT __stdcall proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

  std::expected<void, error_trace> initialize() {
    if (_initialized) return {};
    if (!hinstance) return unexpected_error(errors::not_initialized, "window: instance handle is null");
    WNDCLASSW wc{};
    wc.lpfnWndProc = proc;
    wc.hInstance = hinstance;
    wc.hCursor = ::LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wc.lpszClassName = name.data();
    if (!::RegisterClassW(&wc))
      return unexpected_error(errors::operation_failed, "RegisterClassW failed", ::GetLastError());
    _initialized = true;
    return {};
  };
} window_class;

//////////////////////////////////////// MARK: window

enum class window_style : uint32_t {
  unknown,
  regular = WS_OVERLAPPEDWINDOW,
  fixed = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
  borderless = WS_POPUP,
};

class window {
  friend class control;
  friend class subwindow;
  friend bool ::yw::mainloop();
  friend LRESULT __stdcall decltype(window_class)::proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
  static constexpr int initial_size = 400;
  inline static std::vector<HWND> _active_window_list;

  HWND _hwnd = nullptr;
  window_style _style = window_style::unknown;
  int4 _pad;
  bitmap _rendertarget;
  comptr<::IDXGISwapChain1> _swapchain;
  std::vector<HWND> _subwindows;
  std::vector<control> _controls;

  std::expected<void, error_trace> _create_window(null_terminated<wchar_t> title, window_style style) {
    if (auto res = window_class.initialize(); !res) return unexpected_error(res.error());
    switch (style) {
    case window_style::regular:
    case window_style::fixed:
    case window_style::borderless: break;
    default: return unexpected_error(errors::invalid_argument, "invalid window style");
    }
    auto hwnd = ::CreateWindowExW(0, window_class.name.data(), title.data(), static_cast<DWORD>(style), 0, 0,
      initial_size, initial_size, nullptr, nullptr, window_class.hinstance, nullptr);
    if (!hwnd) return unexpected_error(errors::operation_failed, "CreateWindowExW failed", ::GetLastError());
    ::SetWindowLongPtrW(_hwnd = hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    return {};
  }

  std::expected<void, error_trace> _calculate_padding() {
    RECT client_rect{}, window_rect{};
    if (!::GetClientRect(_hwnd, &client_rect)) return unexpected_error(errors::operation_failed, "GetClientRect failed", ::GetLastError());
    if (!::GetWindowRect(_hwnd, &window_rect)) return unexpected_error(errors::operation_failed, "GetWindowRect failed", ::GetLastError());
    const auto width = window_rect.right - window_rect.left;
    const auto height = window_rect.bottom - window_rect.top;
    const auto pad_left = (width - client_rect.right) / 2; // left, right and bottom are same
    const auto pad_top = height - client_rect.bottom - pad_left;
    _pad = int4{pad_left, pad_top, 2 * pad_left, pad_left + pad_top};
    return {};
  }

  std::expected<void, error_trace> _resize(uint2 size) {
    print("required client size: {} x {}\npadding: left {}, top {}, right {}, bottom {}", size.x, size.y, _pad.x, _pad.y, _pad.z, _pad.w);
    const auto w = size.x + _pad.z, h = size.y + _pad.w;
    print("calculated window size: {} x {}", w, h);
    if (!::SetWindowPos(_hwnd, nullptr, 0, 0, w, h, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE))
      return unexpected_error(errors::operation_failed, "SetWindowPos failed", ::GetLastError());
    if (_swapchain) {
      if (auto hr = _swapchain->ResizeBuffers(0, size.x, size.y, DXGI_FORMAT_UNKNOWN, 0); FAILED(hr))
        return unexpected_error(errors::operation_failed, "ResizeBuffers failed", hr);
    } else {
      if (auto res = dxgi.initialize(); !res) return unexpected_error(res.error());
      auto desc = DXGI_SWAP_CHAIN_DESC1(size.x, size.y, bitmap::dxgiformat, false, DXGI_SAMPLE_DESC(1, 0), {}, 2);
      desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT, desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
      auto hr = dxgi.factory()->CreateSwapChainForHwnd(d3d.device(), _hwnd, &desc, nullptr, nullptr, &_swapchain.get());
      if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateSwapChainForHwnd failed", hr);
    }
    if (auto res = bitmap::create(_swapchain.get()); !res) return unexpected_error(res.error());
    else _rendertarget = std::move(*res);
    return {};
  }

  std::expected<void, error_trace> _move_to(int2 pos) {
    if (::SetWindowPos(_hwnd, nullptr, pos.x, pos.y, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE)) return {};
    else return unexpected_error(errors::operation_failed, "SetWindowPos failed", ::GetLastError());
  }

  std::expected<void, error_trace> _move_to_center() {
    RECT r;
    if (auto desktop = ::GetDesktopWindow(); !desktop) return unexpected_error(errors::operation_failed, "GetDesktopWindow failed", ::GetLastError());
    else if (!::GetClientRect(desktop, &r)) return unexpected_error(errors::operation_failed, "GetClientRect failed", ::GetLastError());
    const auto wh = int2(_rendertarget.size()) + int2(_pad.z, _pad.w);
    const auto x = (r.right - wh.x) / 2, y = (r.bottom - wh.y) / 2;
    if (::SetWindowPos(_hwnd, nullptr, x, y, wh.x, wh.y, SWP_NOZORDER | SWP_NOACTIVATE)) return {};
    else return unexpected_error(errors::operation_failed, "SetWindowPos failed", ::GetLastError());
  }

  void _show() {
    ::ShowWindow(_hwnd, SW_SHOW);
    ::SetForegroundWindow(_hwnd);
    ::SetActiveWindow(_hwnd);
  }

public:
  window() noexcept = default;
  window(const window&) = delete;
  window& operator=(const window&) = delete;

  virtual ~window() {
    if (!_hwnd) return;
    auto it = std::find(_active_window_list.begin(), _active_window_list.end(), _hwnd);
    if (it != _active_window_list.end()) _active_window_list.erase(it);
    ::DestroyWindow(std::exchange(_hwnd, nullptr));
  }

  window(window&& other) noexcept : _hwnd(std::exchange(other._hwnd, nullptr)), _pad(other._pad), _rendertarget(std::move(other._rendertarget)),
    _swapchain(std::move(other._swapchain)), _controls(std::move(other._controls)) {
    ::SetWindowLongPtrW(_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
  }

  window& operator=(window&& other) noexcept {
    if (this == &other) return *this;
    if (_hwnd) ::DestroyWindow(_hwnd);
    _hwnd = std::exchange(other._hwnd, nullptr);
    _pad = other._pad;
    _rendertarget = std::move(other._rendertarget);
    _swapchain = std::move(other._swapchain);
    _controls = std::move(other._controls);
    if (_hwnd) ::SetWindowLongPtrW(_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    return *this;
  }

  HWND handle() const noexcept { return _hwnd; }
  window_style style() const noexcept { return _style; }

  static std::expected<window, error_trace> create(int2 Pos, uint2 Size, null_terminated<wchar_t> Title,
    window_style Style = window_style::regular, bool Hidden = false) {
    window w;
    if (auto res = w._create_window(std::move(Title), Style); !res) return unexpected_error(res.error());
    else if (auto res = w._calculate_padding(); !res) return unexpected_error(res.error());
    else if (auto res = w._resize(Size); !res) return unexpected_error(res.error());
    else if (auto res = w._move_to(Pos); !res) return unexpected_error(res.error());
    if (!Hidden) w._show();
    return std::move(w);
  }

  static std::expected<window, error_trace> create(
    uint2 Size, null_terminated<wchar_t> Title, window_style Style = window_style::regular, bool Hidden = false) {
    window w;
    if (auto res = w._create_window(std::move(Title), Style); !res) return unexpected_error(res.error());
    else if (auto res = w._calculate_padding(); !res) return unexpected_error(res.error());
    else if (auto res = w._resize(Size); !res) return unexpected_error(res.error());
    else if (auto res = w._move_to_center(); !res) return unexpected_error(res.error());
    if (!Hidden) w._show();
    return std::move(w);
  }

  static std::expected<window, error_trace> create(HWND hwnd) {
    window w;
    w._hwnd = hwnd;
    ::SetWindowLongPtrW(w._hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(&w));
    if (auto res = w._calculate_padding(); !res) return unexpected_error(res.error());
    return std::move(w);
  }

  /// get current client size
  std::expected<uint2, error_trace> size() const {
    if (!_hwnd) return unexpected_error(errors::not_initialized, "window not initialized");
    if (RECT rect{}; ::GetClientRect(_hwnd, &rect)) return uint2(rect.right, rect.bottom);
    else return unexpected_error(errors::operation_failed, "GetClientRect failed", ::GetLastError());
  }

  /// set client size
  std::expected<void, error_trace> size(uint2 Size) {
    if (!_hwnd) return unexpected_error(errors::not_initialized, "window not initialized");
    return _resize(uint2(Size.x, Size.y));
  }

  auto begin_draw() { return _rendertarget.begin_draw(); }
  auto begin_draw(const color& clear_color) { return _rendertarget.begin_draw(clear_color);  }
  auto end_draw() { return _rendertarget.end_draw();  }
};

//////////////////////////////////////// MARK: subwindow

enum class subwindow_style : uint32_t {
  unknown,
  subwindow, // ex) secondary windows
  modal,     // ex) message box
  temporary, // ex) context menu
};

class subwindow {
  static window_style _select_window_style(window_style parent_style, subwindow_style style) {
    switch (style) {
    case subwindow_style::subwindow: return parent_style;
    case subwindow_style::modal: return window_style::fixed;
    case subwindow_style::temporary: return window_style::borderless;
    default: return window_style::unknown;
    }
  }
  window _owner;
  const window* _parent{nullptr};
  subwindow_style _style{subwindow_style::unknown};
  subwindow(const window& parent, window&& w, subwindow_style style)
    : _owner(std::move(w)), _parent(&parent), _style(style) {}
public:
  subwindow() noexcept = default;
  subwindow(const subwindow&) = delete;
  subwindow& operator=(const subwindow&) = delete;

  virtual ~subwindow() {

  }
};

//////////////////////////////////////// MARK: control

class control {
// protected:
//   std::variant<std::monostate, HWND, control*> _parent;
//   control(const control& group, float2 position, float2 size);
//   control(const window& wnd, float2 position, float2 size);
// public:
//   float2 position, size, rounded_radius;
//   float text_size{};
//   bool visible{}, enabled{};

//   control() noexcept = default;
//   control(const control&) = delete;
//   control& operator=(const control&) = delete;

//   virtual ~control() {
//     if (_parent.index() != 1) return;
//     if (auto p = reinterpret_cast<window*>(::GetWindowLongPtrW(std::get<1>(_parent), GWLP_USERDATA)); !p) return;
//     else if (auto it = std::find(p->_controls.begin(), p->_controls.end(), this); it != p->_controls.end()) p->_controls.erase(it);
//   }

//   virtual std::expected<void, error_trace> draw() = 0;
//   virtual std::expected<void, error_trace> focus() = 0;
//   virtual std::expected<LRESULT, error_trace> proc(UINT msg, WPARAM wparam, LPARAM lparam) = 0;
};

//////////////////////////////////////// MARK: window_class proc

inline LRESULT __stdcall decltype(window_class)::proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  auto self = reinterpret_cast<window*>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA));
  if (!self) return ::DefWindowProcW(hwnd, msg, wparam, lparam);
  switch (msg) {
  case WM_NCDESTROY: {
    ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
    auto it = std::find(window::_active_window_list.begin(), window::_active_window_list.end(), hwnd);
    if (it != window::_active_window_list.end()) window::_active_window_list.erase(it);
    if (window::_active_window_list.empty()) ::PostQuitMessage(0);
    break;
  }
  case WM_CREATE: window::_active_window_list.push_back(hwnd); return 0;
  }
  return ::DefWindowProcW(hwnd, msg, wparam, lparam);
}

//////////////////////////////////////// MARK: mainloop

inline bool mainloop() {
  for (auto hw : window::_active_window_list) {
    auto wnd = reinterpret_cast<window*>(::GetWindowLongPtrW(hw, GWLP_USERDATA));
    if (!wnd) continue;
    if (wnd->_swapchain) wnd->_swapchain->Present(1, 0);
    else print("No swapchain present for window {}", uintptr_t(hw));
  }
  for (MSG msg; ::PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE);) {
    if (msg.message == WM_QUIT) return false;
    ::TranslateMessage(&msg), ::DispatchMessageW(&msg);
  }
  return true;
}
} // namespace yw
