#pragma once
#include <apps/control.h>

#ifdef _WIN32

namespace yw {

namespace ui {
class label;
}

///--------------------------------------------------------------------------///
/// MARK: wclass

class wclass {
  static LRESULT __stdcall wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
  inline static WNDCLASSW wndclass{
    .style = CS_DBLCLKS,
    .lpfnWndProc = wndproc,
    .hInstance = ::GetModuleHandleW(nullptr),
    .hCursor = ::LoadCursorW(nullptr, IDC_ARROW),
    .lpszClassName = L"ywlib_window_class"};
  inline static bool _initialized = false;

public:
  static const wchar_t* name() noexcept {
    if (auto res = initialize(); !res) res.error().print_and_abort();
    return wndclass.lpszClassName;
  }
  static HINSTANCE hinstance() noexcept {
    if (auto res = initialize(); !res) res.error().print_and_abort();
    return wndclass.hInstance;
  }
  static result<void> initialize() {
    if (_initialized) return {};
    if (!::RegisterClassW(&wndclass) && ::GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
      return error(errors::operation_failed, "RegisterClassW failed");
    _initialized = true;
    return {};
  }
  static void release() {
    ::UnregisterClassW(wndclass.lpszClassName, wndclass.hInstance);
    _initialized = false;
  }
};

///--------------------------------------------------------------------------///
/// MARK: window

class window : public interface {
public:
  struct custom_config {
    string<wchar_t> title;
    optional<int2> pos;
    optional<int2> size;
    DWORD style = WS_OVERLAPPEDWINDOW;
    DWORD ex_style = WS_EX_ACCEPTFILES;
    const string<wchar_t>& get_title() const noexcept { return title; }
    DWORD get_style() const noexcept { return style; }
    DWORD get_ex_style() const noexcept { return ex_style; }
  };

  struct config {
    string<wchar_t> title;
    optional<int2> pos;
    optional<int2> size;
    bool has_border = true;
    bool has_caption = true;
    bool resizable = true;
    bool visible = true;
    bool enabled = true;
    bool topmost = false;
    const string<wchar_t>& get_title() const noexcept { return title; }
    DWORD get_style() const noexcept {
      DWORD s = has_caption ? WS_CAPTION | WS_SYSMENU : WS_POPUP;
      s |= WS_BORDER * has_border;
      s |= WS_THICKFRAME * resizable;
      s |= WS_VISIBLE * visible;
      s |= WS_DISABLED * !enabled;
      return s;
    }
    DWORD get_ex_style() const noexcept {
      DWORD s = WS_EX_ACCEPTFILES;
      s |= topmost ? WS_EX_TOPMOST : 0;
      return s;
    }
  };

  struct color_theme {
    color canvas{0xf0f0f0};                  // ex) background of window
    color surface{0xf8f8f8};                 // ex) background of control
    color surface_popup{0xffffff};           // ex) background of popup
    color outline = color_name::darkgray;    // ex) border of control
    color part = color_name::gray;           // ex) button of checkbox, thumb of scrollbar
    color text = color_name::black;          // ex) text, icon
    color text_muted = color_name::darkgray; // ex) placeholder text
    color accent = color_name::dodgerblue;   // ex) focus, selection
    color warning = color_name::darkorange;
    color error = color_name::crimson;
    color success = color_name::seagreen;
  };

  struct slot : interface::slot {
    inline static std::vector<slotid> windows{};

    slotid parent_id{};
    HWND hwnd{};

    int4 frame_thickness{};
    DWORD style{}, exstyle{};
    window::color_theme color_theme;

    bitmap control_layer;
    bitmap render_target;
    comptr<IDXGISwapChain1> swap_chain;

    slotid tooltip_id{};
    array<slotid> subwindows{};

    function<bool, yw::button_event> button_event;
    function<bool, yw::cursor_event> cursor_event;
    function<bool, yw::drag_event> drag_event;
    function<bool, yw::focus_event> focus_event;
    function<bool, yw::hover_event> hover_event;
    function<bool, yw::key_event> key_event;
    function<bool, yw::wheel_event> wheel_event;
  };
};
} // namespace yw

#endif
