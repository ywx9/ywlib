#pragma once
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
    if (!::RegisterClassW(&wndclass) && ::GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
      return error(errors::operation_failed, "RegisterClass failed");
    _initialized = true;
    return {};
  }
};

///--------------------------------------------------------------------------///
/// MARK: window system

namespace window_system {
array<HWND> windows;
inline result<void> destroy(HWND hwnd) {
  if (!hwnd) return {};
  windows.erase(std::ranges::find(windows, hwnd));
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
} // namespace window_system

///--------------------------------------------------------------------------///
/// MARK: window

class window {
  bool _dirty = false;
  bool _messy = false;

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

  ~window() { window_system::destroy(hwnd()); }
  window() = default;

  static result<window> create(stringable auto&& Title, options Options) {
    const auto title = unicode<wchar_t>(static_cast<decltype(Title)&&>(Title));
    const auto visible = Options.visible;
    const auto style = Options.get_style() ^ (visible ? WS_VISIBLE : 0); // remove WS_VISIBLE for delayed show
    const auto exstyle = Options.get_exstyle();
    window win;
    if (auto res = window_system::create(style, exstyle, title.c_str()); !res) return res.relay();
    else win.hwnd = res->first, win.style = style, win.exstyle = exstyle, win.frame_thickness = res->second;
    if (auto res = win.update(); !res) return res.relay();
    if (visible) ::ShowWindow(win.hwnd(), SW_SHOW);
    return win;
  }

  window(stringable auto&& Title, options Options, const std::source_location& sl = here()) {
    if (auto res = create(Title, Options)) *this = move(*res);
    else res.error().add_footprint().print_and_abort(sl);
  }

  result<void> close() {
    // subwindows.clear();
    if (auto res = window_system::destroy(hwnd()); !res) return res.relay();
    return {};
  }

  result<void> update() { return {}; }

private:
  color_theme _theme;
};
} // namespace yw
