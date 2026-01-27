#pragma once
#include "ywx/bitmap.h"
#include "ywx/core.h"

namespace yw {

class window;

namespace app {
inline error_trace last_error;
inline std::vector<HWND> active_window_list;

inline bool loop() noexcept {
  for (MSG msg; ::PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE);) {
    if (msg.message == WM_QUIT) return false;
    ::TranslateMessage(&msg);
    ::DispatchMessageW(&msg);
  }
  return true;
}
} // namespace app

//////////////////////////////////////// MARK: window_class

inline class {
  bool _initialized{false};

public:
  const null_terminated<wchar_t> name = L"ywlib_window_class";
  const HINSTANCE hinstance = ::GetModuleHandleW(nullptr);

  static LRESULT __stdcall proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    auto self = reinterpret_cast<window*>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    switch (msg) {
    case WM_NCDESTROY: {
      ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
      auto it = std::find(app::active_window_list.begin(), app::active_window_list.end(), hwnd);
      if (it != app::active_window_list.end()) app::active_window_list.erase(it);
      if (app::active_window_list.empty()) ::PostQuitMessage(0);
      break;
    }
    case WM_CREATE: app::active_window_list.push_back(hwnd); return 0;
    }
    return ::DefWindowProcW(hwnd, msg, wparam, lparam);
  }

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

enum class window_style {
  unknown,
  regular,      // overlapped window with title bar and border
  fixed,        // fixed-size window with title bar and border
  fixed_border, // fixed-size window with border but no title
  borderless,   // borderless window
};

class window {
  HWND _hwnd = nullptr;
  int4 _pad;
  window(HWND hwnd, int4 pad) : _hwnd(hwnd), _pad(pad) {
    ::SetWindowLongPtrW(_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
  }

  static constexpr int initial_size = 400;

  static std::expected<HWND, error_trace> _create_window(null_terminated<wchar_t> Title, window_style Style) {
    if (auto res = window_class.initialize(); !res) return unexpected_error(res.error());
    DWORD style = 0; // hide window initially
    switch (Style) {
    case window_style::regular: style |= WS_OVERLAPPEDWINDOW; break;
    case window_style::fixed: style |= WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX; break;
    case window_style::fixed_border: style |= WS_POPUP | WS_BORDER; break;
    case window_style::borderless: style |= WS_POPUP; break;
    default: return unexpected_error(errors::invalid_argument, "invalid window style");
    }
    auto hwnd = ::CreateWindowExW(0, window_class.name.data(), Title.data(), style, 0, 0, initial_size, initial_size,
      nullptr, nullptr, window_class.hinstance, nullptr);
    if (!hwnd) return unexpected_error(errors::operation_failed, "CreateWindowExW failed", ::GetLastError());
    return hwnd;
  }

  static std::expected<int4, error_trace> _calculate_padding(HWND hwnd, int2 Size) {
    if (RECT r; ::GetClientRect(hwnd, &r)) {
      const auto x = (Size.x - (r.right - r.left)) / 2;
      const auto y = Size.y - (r.bottom - r.top) - x;
      return int4{x, y, 2 * x, x + y};
    } else return unexpected_error(errors::operation_failed, "GetClientRect failed", ::GetLastError());
  }

  static std::expected<void, error_trace> _adjust_sizepos(HWND hwnd, int4 pad, int2 Size, int2 Pos) {
    const auto w = Size.x + pad.z, h = Size.y + pad.w;
    if (!::SetWindowPos(hwnd, nullptr, Pos.x, Pos.y, w, h, SWP_NOZORDER | SWP_NOACTIVATE))
      return unexpected_error(errors::operation_failed, "SetWindowPos failed", ::GetLastError());
    return {};
  }

  static std::expected<void, error_trace> _adjust_size(HWND hwnd, int4 pad, int2 Size) {
    const auto w = Size.x + pad.z, h = Size.y + pad.w;
    if (!::SetWindowPos(hwnd, nullptr, 0, 0, w, h, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE))
      return unexpected_error(errors::operation_failed, "SetWindowPos failed", ::GetLastError());
    return {};
  }

  static std::expected<void, error_trace> _adjust_pos(HWND hwnd, int4 pad, int2 Size) {
    const auto w = Size.x + pad.z, h = Size.y + pad.w;
    if (RECT r; !::GetWindowRect(::GetDesktopWindow(), &r))
      return unexpected_error(errors::operation_failed, "GetWindowRect failed", ::GetLastError());
    else if (!::SetWindowPos(hwnd, nullptr, (r.right - w) / 2, (r.bottom - h) / 2, w, h, SWP_NOZORDER | SWP_NOACTIVATE))
      return unexpected_error(errors::operation_failed, "SetWindowPos failed", ::GetLastError());
    return {};
  }

  static void _show(HWND hwnd) {
    ::ShowWindow(hwnd, SW_SHOW);
    ::SetForegroundWindow(hwnd);
    ::SetActiveWindow(hwnd);
  }

public:
  window() noexcept = default;
  window(const window&) = delete;
  window& operator=(const window&) = delete;

  ~window() {
    if (_hwnd) ::DestroyWindow(std::exchange(_hwnd, nullptr));
  }
  window(window&& other) noexcept : _hwnd(std::exchange(other._hwnd, nullptr)), _pad(other._pad) {
    ::SetWindowLongPtrW(_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
  }
  window& operator=(window&& other) noexcept {
    if (this == &other) return *this;
    if (_hwnd) ::DestroyWindow(_hwnd);
    _hwnd = std::exchange(other._hwnd, nullptr);
    _pad = other._pad;
    if (_hwnd) ::SetWindowLongPtrW(_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    return *this;
  }

  HWND handle() const noexcept { return _hwnd; }

  static std::expected<window, error_trace> create(int2 Pos, uint2 Size, null_terminated<wchar_t> Title,
    window_style Style = window_style::regular, bool Hidden = false) {
    auto hwnd = _create_window(std::move(Title), Style);
    if (!hwnd) return unexpected_error(hwnd.error());
    auto pad = _calculate_padding(*hwnd, int2(Size.x, Size.y));
    if (!pad) return unexpected_error(pad.error());
    if (!_adjust_sizepos(*hwnd, *pad, int2(Size.x, Size.y), Pos)) return unexpected_error(pad.error());
    if (!Hidden) _show(*hwnd);
    return window(*hwnd, *pad);
  }

  static std::expected<window, error_trace> create(
    uint2 Size, null_terminated<wchar_t> Title, window_style Style = window_style::regular, bool Hidden = false) {
    auto hwnd = _create_window(std::move(Title), Style);
    if (!hwnd) return unexpected_error(hwnd.error());
    auto pad = _calculate_padding(*hwnd, int2(Size.x, Size.y));
    if (!pad) return unexpected_error(pad.error());
    if (!_adjust_pos(*hwnd, *pad, int2(Size.x, Size.y))) return unexpected_error(pad.error());
    if (!Hidden) _show(*hwnd);
    return window(*hwnd, *pad);
  }

  static std::expected<window, error_trace> create(HWND hwnd) {
    auto pad = _calculate_padding(hwnd, int2(initial_size, initial_size));
    if (!pad) return unexpected_error(pad.error());
    return window(hwnd, *pad);
  }

  std::expected<uint2, error_trace> size() const {
    if (!_hwnd) return unexpected_error(errors::not_initialized, "window not initialized");
    if (RECT rect{}; ::GetClientRect(_hwnd, &rect)) return uint2(rect.right, rect.bottom);
    else return unexpected_error(errors::operation_failed, "GetClientRect failed", ::GetLastError());
  }

  std::expected<void, error_trace> size(uint2 Size) {
    if (!_hwnd) return unexpected_error(errors::not_initialized, "window not initialized");
    return _adjust_size(_hwnd, _pad, int2(Size.x, Size.y));
  }
};

} // namespace yw

// namespace yw {

// inline class {
//   HWND _hwnd{nullptr};
//   comptr<IDXGISwapChain1> _swapchain;
//   ATOM _class_atom{0};
//   int4 _pad;

// public:
//   bool _error(const char* msg) const {
//     std::print("window: {}\n", msg);
//     return false;
//   }

//   bool _fatal(const char* msg) {
//     std::print("window: {}\n", msg);
//     if (_hwnd) {
//       ::DestroyWindow(_hwnd);
//       _hwnd = nullptr;
//     }
//     return false;
//   }

//   static LRESULT __stdcall _wndproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
//     switch (msg) {
//     case WM_DESTROY: ::PostQuitMessage(0); return 0;
//     default: return ::DefWindowProcW(hwnd, msg, wparam, lparam);
//     }
//   }

// public:
//   bool open(uint2 Size, null_terminated<wchar_t> Title, is_bool auto Hidden) {
//     if (_hwnd) return _error("window already opened");
//     if (!dxgi.initialize()) return _fatal("dxgi not initialized");
//     const HINSTANCE hinstance = ::GetModuleHandleW(nullptr);
//     const wchar_t* class_name = L"ywlib_window_class";
//     if (!_class_atom) {
//       WNDCLASSW wc{};
//       wc.lpfnWndProc = _wndproc;
//       wc.hInstance = hinstance;
//       wc.hCursor = ::LoadCursorW(nullptr, IDC_ARROW);
//       wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
//       wc.lpszClassName = class_name;
//       if (!::RegisterClassW(&wc)) return _fatal("RegisterClassW failed");
//     }
//     _hwnd = ::CreateWindowExW(WS_EX_ACCEPTFILES, class_name, Title.data(), WS_CAPTION | WS_SYSMENU, 0, 0, 400, 400,
//       nullptr, nullptr, hinstance, 0);
//     if (!_hwnd) return _fatal("CreateWindowExW failed");
//     DXGI_SWAP_CHAIN_DESC1 desc{UINT(Size.x), UINT(Size.y), bitmap::dxgiformat, false, DXGI_SAMPLE_DESC(1, 0), {}, 2};
//     desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT, desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
//     auto hr = dxgi_factory->CreateSwapChainForHwnd(d3d_device.get(), _hwnd, &desc, 0, 0, &_swapchain.get());
//     if (hr != 0) return _fatal("CreateSwapChainForHwnd failed");
//     ::GetClientRect(_hwnd, (RECT*)&_pad);
//     _pad.x = (400 - _pad.z) / 2;
//     _pad.z = 2 * _pad.x;
//     _pad.y = 400 - _pad.w - _pad.x;
//     _pad.w = _pad.x + _pad.y;
//     const auto w = Size.x + _pad.z, h = Size.y + _pad.w;
//     RECT desktop{};
//     ::GetWindowRect(::GetDesktopWindow(), &desktop);
//     ::SetWindowPos(
//       _hwnd, nullptr, (desktop.right - w) / 2, (desktop.bottom - h) / 2, w, h, SWP_NOZORDER | SWP_NOACTIVATE);
//     if (!Hidden) ::ShowWindow(_hwnd, SW_SHOW), ::SetForegroundWindow(_hwnd), ::SetActiveWindow(_hwnd);
//     return {};
//   }

//   bool open(uint2 Size, null_terminated<wchar_t> Title = args::program_name) {
//     return open(Size, std::move(Title), false);
//   }

//   bool open(uint2 Size, is_bool auto Hidden) { return open(Size, args::program_name, Hidden); }
//   bool close() {
//     if (!_hwnd) throw std::runtime_error("window not opened");
//     ::DestroyWindow(std::exchange(_hwnd, nullptr));
//     _swapchain.release();
//     return true;
//   }

//   HWND hwnd() const { return _hwnd; }

//   bool show() {
//     if (!_hwnd) {
//       std::print("window: window not opened\n");
//       return false;
//     }
//     ::ShowWindow(_hwnd, SW_SHOW);
//     ::SetForegroundWindow(_hwnd);
//     ::SetActiveWindow(_hwnd);
//     return true;
//   }

//   bool hide() {
//     if (!_hwnd) {
//       std::print("window: window not opened\n");
//       return false;
//     }
//     ::ShowWindow(_hwnd, SW_HIDE);
//     return true;
//   }

//   uint2 size() {
//     if (!_hwnd) {
//       std::print("window: window not opened\n");
//       return uint2{};
//     }
//     RECT rect{};
//     ::GetClientRect(_hwnd, &rect);
//     return uint2(rect.right - rect.left, rect.bottom - rect.top);
//   }

//   bool size(uint2 Size) {
//     if (!_hwnd) {
//       std::print("window: window not opened\n");
//       return false;
//     }
//     ::SetWindowPos(_hwnd, nullptr, 0, 0, Size.x + _pad.z, Size.y + _pad.w, SWP_NOZORDER | SWP_NOMOVE |
//     SWP_NOACTIVATE); if (auto hr = _swapchain->ResizeBuffers(0, Size.x, Size.y, DXGI_FORMAT_UNKNOWN, 0); hr != 0) {
//       std::print("window: ResizeBuffers failed\n");
//       return false;
//     }
//   }

//   std::expected<bool, error_trace> update() {
//     _swapchain->Present(1, 0);
//     for (MSG msg; ::PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE);) {
//       ::TranslateMessage(&msg), ::DispatchMessageW(&msg);
//       if (msg.message == WM_QUIT) return false;
//     }
//     return true;
//   }
// } window;
// } // namespace yw
