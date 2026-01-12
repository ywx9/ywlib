#pragma once
#include "ywx/bitmap.h"
#include "ywx/core.h"

namespace yw {

inline class {
  HWND _hwnd{nullptr};
  comptr<IDXGISwapChain1> _swapchain;
  ATOM _class_atom{0};
  int4 _pad;
public:

  bool _error(const char* msg) const {
    std::print("window: {}\n", msg);
    return false;
  }

  bool _fatal(const char* msg) {
    std::print("window: {}\n", msg);
    if (_hwnd) {
      ::DestroyWindow(_hwnd);
      _hwnd = nullptr;
    }
    return false;
  }

  static LRESULT __stdcall _wndproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    switch (msg) {
    case WM_DESTROY: ::PostQuitMessage(0); return 0;
    default: return ::DefWindowProcW(hwnd, msg, wparam, lparam);
    }
  }

public:
  bool open(uint2 Size, null_terminated<wchar_t> Title, is_bool auto Hidden) {
    if (_hwnd) return _error("window already opened");
    if (!dxgi.initialize()) return _fatal("dxgi not initialized");
    const HINSTANCE hinstance = ::GetModuleHandleW(nullptr);
    const wchar_t* class_name = L"ywlib_window_class";
    if (!_class_atom) {
      WNDCLASSW wc{};
      wc.lpfnWndProc = _wndproc;
      wc.hinstance = hinstance;
      wc.hCursor = ::LoadCursorW(nullptr, IDC_ARROW);
      wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
      wc.lpszClassName = class_name;
      if (!::RegisterClassW(&wc)) return _fatal("RegisterClassW failed");
    }
    _hwnd = ::CreateWindowExW(WS_EX_ACCEPTFILES, class_name, Title.data(), WS_CAPTION | WS_SYSMENU, 0, 0, 400, 400,
                              nullptr, nullptr, hinstance, 0);
    if (!_hwnd) return _fatal("CreateWindowExW failed");
    DXGI_SWAP_CHAIN_DESC1 desc{UINT(Size.x), UINT(Size.y), bitmap::dxgiformat, false, DXGI_SAMPLE_DESC(1, 0), {}, 2};
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT, desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    auto hr = dxgi_factory->CreateSwapChainForHwnd(d3d_device.get(), _hwnd, &desc, 0, 0, &_swapchain.get());
    if (hr != 0) return _fatal("CreateSwapChainForHwnd failed");
    ::GetClientRect(_hwnd, (RECT*)&_pad);
    _pad.x = (400 - _pad.z) / 2;
    _pad.z = 2 * _pad.x;
    _pad.y = 400 - _pad.w - _pad.x;
    _pad.w = _pad.x + _pad.y;
    const auto w = Size.x + _pad.z, h = Size.y + _pad.w;
    RECT desktop{};
    ::GetWindowRect(::GetDesktopWindow(), &desktop);
    ::SetWindowPos(_hwnd, nullptr, (desktop.right - w) / 2, (desktop.bottom - h) / 2, w, h,
                   SWP_NOZORDER | SWP_NOACTIVATE);
    if (!Hidden) ::ShowWindow(_hwnd, SW_SHOW), ::SetForegroundWindow(_hwnd), ::SetActiveWindow(_hwnd);
    return {};
  }

  bool open(uint2 Size, null_terminated<wchar_t> Title = args.program_name) {
    return open(Size, std::move(Title), false);
  }

  bool open(uint2 Size, is_bool auto Hidden) { return open(Size, args.program_name, Hidden); }

  bool close() {
    if (!_hwnd) throw std::runtime_error("window not opened");
    ::DestroyWindow(std::exchange(_hwnd, nullptr));
    _swapchain.release();
    return true;
  }

  HWND hwnd() const { return _hwnd; }

  bool show() {
    if (!_hwnd) {
      std::print("window: window not opened\n");
      return false;
    }
    ::ShowWindow(_hwnd, SW_SHOW);
    ::SetForegroundWindow(_hwnd);
    ::SetActiveWindow(_hwnd);
    return true;
  }

  bool hide() {
    if (!_hwnd) {
      std::print("window: window not opened\n");
      return false;
    }
    ::ShowWindow(_hwnd, SW_HIDE);
    return true;
  }

  uint2 size() {
    if (!_hwnd) {
      std::print("window: window not opened\n");
      return uint2{};
    }
    RECT rect{};
    ::GetClientRect(_hwnd, &rect);
    return uint2(rect.right - rect.left, rect.bottom - rect.top);
  }

  bool size(uint2 Size) {
    if (!_hwnd) {
      std::print("window: window not opened\n");
      return false;
    }
    ::SetWindowPos(_hwnd, nullptr, 0, 0, Size.x + _pad.z, Size.y + _pad.w, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
    if (auto hr = _swapchain->ResizeBuffers(0, Size.x, Size.y, DXGI_FORMAT_UNKNOWN, 0); hr != 0) {
      std::print("window: ResizeBuffers failed\n");
      return false;
    }
  }
} window;
} // namespace yw
