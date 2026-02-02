#pragma once
#include "ywx/text_format.h"

namespace yw {

class window;
class subwindow;
class control;

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
      return unexpected_error(errors::operation_failed, "RegisterClassW failed", int32_t(::GetLastError()));
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
protected:
  friend class subwindow;
  friend class control;
  friend bool ::yw::mainloop();
  friend LRESULT __stdcall decltype(window_class)::proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
  static constexpr int initial_size = 400;
  inline static std::vector<HWND> _active_window_list{};

  HWND _hwnd = nullptr;
  window_style _style = window_style::unknown;
  int4 _pad{};
  bitmap _rendertarget{};
  comptr<::IDXGISwapChain1> _swapchain{};
  std::vector<HWND> _subwindows{};
  std::vector<std::unique_ptr<control>> _controls{};

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
    if (!hwnd) return unexpected_error(errors::operation_failed, "CreateWindowExW failed", int32_t(::GetLastError()));
    ::SetWindowLongPtrW(_hwnd = hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    _active_window_list.push_back(_hwnd);
    _style = style;
    return {};
  }

  std::expected<void, error_trace> _calculate_padding() {
    RECT client_rect{}, window_rect{};
    if (!::GetClientRect(_hwnd, &client_rect))
      return unexpected_error(errors::operation_failed, "GetClientRect failed", int32_t(::GetLastError()));
    if (!::GetWindowRect(_hwnd, &window_rect))
      return unexpected_error(errors::operation_failed, "GetWindowRect failed", int32_t(::GetLastError()));
    const auto width = window_rect.right - window_rect.left;
    const auto height = window_rect.bottom - window_rect.top;
    const auto pad_left = (width - client_rect.right) / 2; // left, right and bottom are same
    const auto pad_top = height - client_rect.bottom - pad_left;
    _pad = int4{pad_left, pad_top, 2 * pad_left, pad_left + pad_top};
    return {};
  }

  std::expected<void, error_trace> _resize_d3d(uint2 size) {
    _rendertarget = {};
    if (_swapchain) {
      if (auto hr = _swapchain->ResizeBuffers(0, size.x, size.y, DXGI_FORMAT_UNKNOWN, 0); FAILED(hr))
        return unexpected_error(errors::operation_failed, "ResizeBuffers failed", int32_t(hr));
    } else {
      if (auto res = dxgi.initialize(); !res) return unexpected_error(res.error());
      auto desc = DXGI_SWAP_CHAIN_DESC1(size.x, size.y, bitmap::dxgiformat, false, DXGI_SAMPLE_DESC(1, 0), {}, 2);
      desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT, desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
      auto hr = dxgi.factory()->CreateSwapChainForHwnd(d3d.device(), _hwnd, &desc, nullptr, nullptr, &_swapchain.get());
      if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateSwapChainForHwnd failed", int32_t(hr));
    }
    if (auto res = bitmap::create(_swapchain.get()); !res) return unexpected_error(res.error());
    else _rendertarget = std::move(*res);
    return {};
  }

  std::expected<void, error_trace> _resize(uint2 size) {
    const auto w = size.x + _pad.z, h = size.y + _pad.w;
    if (!::SetWindowPos(_hwnd, nullptr, 0, 0, w, h, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE))
      return unexpected_error(errors::operation_failed, "SetWindowPos failed", int32_t(::GetLastError()));
    if (auto res = _resize_d3d(size); !res) return unexpected_error(res.error());
    return {};
  }

  std::expected<void, error_trace> _move_to(int2 pos) {
    if (::SetWindowPos(_hwnd, nullptr, pos.x, pos.y, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE)) return {};
    else return unexpected_error(errors::operation_failed, "SetWindowPos failed", int32_t(::GetLastError()));
  }

  std::expected<void, error_trace> _move_to_center() {
    RECT r;
    if (auto desktop = ::GetDesktopWindow(); !desktop)
      return unexpected_error(errors::operation_failed, "GetDesktopWindow failed", int32_t(::GetLastError()));
    else if (!::GetClientRect(desktop, &r))
      return unexpected_error(errors::operation_failed, "GetClientRect failed", int32_t(::GetLastError()));
    const auto wh = int2(_rendertarget.size()) + int2(_pad.z, _pad.w);
    const auto x = (r.right - wh.x) / 2, y = (r.bottom - wh.y) / 2;
    if (::SetWindowPos(_hwnd, nullptr, x, y, wh.x, wh.y, SWP_NOZORDER | SWP_NOACTIVATE)) return {};
    else return unexpected_error(errors::operation_failed, "SetWindowPos failed", int32_t(::GetLastError()));
  }

  void _show() {
    ::ShowWindow(_hwnd, SW_SHOW);
    ::SetForegroundWindow(_hwnd);
    ::SetActiveWindow(_hwnd);
  }

public:
  inline static error_trace last_error{error()};

  window() noexcept = default;
  window(const window&) = delete;
  window& operator=(const window&) = delete;

  virtual ~window() {
    if (!_hwnd) return;
    auto it = std::find(_active_window_list.begin(), _active_window_list.end(), _hwnd);
    if (it != _active_window_list.end()) _active_window_list.erase(it);
    ::DestroyWindow(std::exchange(_hwnd, nullptr));
  }

  window(window&& other) noexcept
    : _hwnd(std::exchange(other._hwnd, nullptr)), _style(other._style), _pad(other._pad),
      _rendertarget(std::move(other._rendertarget)), _swapchain(std::move(other._swapchain)),
      _controls(std::move(other._controls)) {
    ::SetWindowLongPtrW(_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
  }

  window& operator=(window&& other) noexcept {
    if (this == &other) return *this;
    if (_hwnd) ::DestroyWindow(_hwnd);
    _hwnd = std::exchange(other._hwnd, nullptr);
    _style = other._style;
    _pad = other._pad;
    _rendertarget = std::move(other._rendertarget);
    _swapchain = std::move(other._swapchain);
    _controls = std::move(other._controls);
    if (_hwnd) ::SetWindowLongPtrW(_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    return *this;
  }

  explicit operator bool() const noexcept { return _hwnd != nullptr; }

  HWND handle() const noexcept { return _hwnd; }
  window_style style() const noexcept { return _style; }

  static std::expected<window, error_trace> create(int2 pos, uint2 size, null_terminated<wchar_t> title,
    window_style style = window_style::regular, bool hidden = false) {
    window w;
    if (auto res = w._create_window(std::move(title), style); !res) return unexpected_error(res.error());
    else if (auto res = w._calculate_padding(); !res) return unexpected_error(res.error());
    else if (auto res = w._resize(size); !res) return unexpected_error(res.error());
    else if (auto res = w._move_to(pos); !res) return unexpected_error(res.error());
    if (!hidden) w._show();
    return std::move(w);
  }

  static std::expected<window, error_trace> create(
    uint2 size, null_terminated<wchar_t> title, window_style style = window_style::regular, bool hidden = false) {
    window w;
    if (auto res = w._create_window(std::move(title), style); !res) return unexpected_error(res.error());
    else if (auto res = w._calculate_padding(); !res) return unexpected_error(res.error());
    else if (auto res = w._resize(size); !res) return unexpected_error(res.error());
    else if (auto res = w._move_to_center(); !res) return unexpected_error(res.error());
    if (!hidden) w._show();
    return std::move(w);
  }

  /// gets current client size
  std::expected<uint2, error_trace> size() const {
    if (!_hwnd) return unexpected_error(errors::not_initialized, "window not initialized");
    if (RECT rect{}; ::GetClientRect(_hwnd, &rect)) return uint2(rect.right, rect.bottom);
    else return unexpected_error(errors::operation_failed, "GetClientRect failed", int32_t(::GetLastError()));
  }

  /// sets client size
  std::expected<void, error_trace> size(uint2 Size) {
    if (!_hwnd) return unexpected_error(errors::not_initialized, "window not initialized");
    return _resize(uint2(Size.x, Size.y));
  }

  /// gets current window position; left-top corner of the window (including non-client area)
  std::expected<int2, error_trace> position() const {
    if (!_hwnd) return unexpected_error(errors::not_initialized, "window not initialized");
    if (RECT rect{}; ::GetWindowRect(_hwnd, &rect)) return int2{rect.left, rect.top};
    else return unexpected_error(errors::operation_failed, "GetWindowRect failed", int32_t(::GetLastError()));
  }

  /// sets window position; left-top corner of the window (including non-client area)
  std::expected<void, error_trace> position(int2 Pos) {
    if (!_hwnd) return unexpected_error(errors::not_initialized, "window not initialized");
    return _move_to(int2(Pos.x, Pos.y));
  }

  auto begin_draw(const source& src = {}) { return _rendertarget.begin_draw(src); }
  auto begin_draw(const color& clear_color, const source& src = {}) {
    return _rendertarget.begin_draw(clear_color, src);
  }

  virtual void close() {
    if (!_hwnd) return;
    ::DestroyWindow(std::exchange(_hwnd, nullptr));
  }
};

//////////////////////////////////////// MARK: subwindow

enum class subwindow_style : uint32_t {
  unknown,
  subwindow, // ex) secondary windows
  modal,     // ex) message box
  temporary, // ex) context menu
};

class subwindow : public window {
  static window_style _select_window_style(window_style parent_style, subwindow_style style) {
    switch (style) {
    case subwindow_style::subwindow: return parent_style;
    case subwindow_style::modal: return window_style::fixed;
    case subwindow_style::temporary: return window_style::borderless;
    default: return window_style::unknown;
    }
  }
  HWND _main{nullptr};
  subwindow_style _style{subwindow_style::unknown};

public:
  ~subwindow() = default;
  subwindow() noexcept = default;
  subwindow(const subwindow&) = delete;
  subwindow& operator=(const subwindow&) = delete;
  subwindow(subwindow&&) noexcept = default;
  subwindow& operator=(subwindow&&) noexcept = default;

  /// creates subwindow attached to the given main_window
  /// \param pos position relative to the main_window's client area
  static std::expected<subwindow, error_trace> create(window& main_window, int2 pos, uint2 size,
    null_terminated<wchar_t> title, subwindow_style style = subwindow_style::subwindow, bool hidden = false) {
    if (!main_window) return unexpected_error(errors::not_initialized, "main window is not initialized");
    subwindow sw;
    if (const auto ws = _select_window_style(main_window.style(), style); ws == window_style::unknown)
      return unexpected_error(errors::invalid_argument, "invalid subwindow style");
    else if (auto res = sw._create_window(std::move(title), ws); !res) return unexpected_error(res.error());
    if (auto res = sw._calculate_padding(); !res) return unexpected_error(res.error());
    if (auto res = sw._resize(size); !res) return unexpected_error(res.error());
    if (auto main_pos = main_window.position(); !main_pos) return unexpected_error(main_pos.error());
    else if (auto res = sw._move_to(*main_pos + pos); !res) return unexpected_error(res.error());
    if (!hidden) sw._show();
    sw._main = main_window._hwnd, sw._style = style;
    main_window._subwindows.push_back(sw._hwnd);
    return std::move(sw);
  }
};

//////////////////////////////////////// MARK: control

class control {
  control(const control&) = delete;
  control& operator=(const control&) = delete;

protected:
  HWND _owner = nullptr;
  control(window& owner) noexcept : _owner(owner._hwnd) {
    if (owner) owner._controls.push_back(std::unique_ptr<control>(this));
  }

public:
  control() noexcept = default;
  control(control&&) noexcept = default;
  control& operator=(control&&) noexcept = default;

  float2 position{}, size{}, padding = float2::fill(1.0f), rounded_radius{};
  float border_width = 1.0f;
  color background_color = colors::white, border_color = colors::black;
  bool visible{true}, enabled{true};

  virtual ~control() {
    if (!_owner) return;
    if (auto p = reinterpret_cast<window*>(::GetWindowLongPtrW(_owner, GWLP_USERDATA)); !p) return;
    else if (auto it = std::find(p->_controls.begin(), p->_controls.end(), this); it != p->_controls.end())
      p->_controls.erase(it);
  }

  control(control&& other) noexcept
    : _owner(std::exchange(other._owner, nullptr)), position(other.position), size(other.size), padding(other.padding),
      rounded_radius(other.rounded_radius), border_width(other.border_width), background_color(other.background_color),
      border_color(other.border_color), visible(other.visible), enabled(other.enabled) {}

  control& operator=(control&& other) noexcept {
    if (this == &other) return *this;
    _owner = std::exchange(other._owner, nullptr);
    position = other.position;
    size = other.size;
    padding = other.padding;
    rounded_radius = other.rounded_radius;
    border_width = other.border_width;
    background_color = other.background_color;
    border_color = other.border_color;
    visible = other.visible;
    enabled = other.enabled;
    return *this;
  }

  bool hit_test(float2 point) const {
    return point.x >= position.x && point.x <= position.x + size.x && point.y >= position.y &&
           point.y <= position.y + size.y;
  }

  virtual std::expected<void, error_trace> draw() = 0;
};

//////////////////////////////////////// MARK: window_class proc

inline LRESULT __stdcall decltype(window_class)::proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  auto self = reinterpret_cast<window*>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA));
  if (!self) return ::DefWindowProcW(hwnd, msg, wparam, lparam);
  switch (msg) {
  case WM_SIZE: {
    const auto width = LOWORD(lparam), height = HIWORD(lparam);
    if (auto res = self->_resize_d3d(uint2(width, height)); !res) {
      window::last_error = res.error().push();
      print_error("Window resize failed", window::last_error);
    }
    return 0;
  }
  case WM_NCDESTROY: {
    for (auto& subs : self->_subwindows) ::DestroyWindow(subs);
    self->_subwindows.clear();
    auto it = std::find(window::_active_window_list.begin(), window::_active_window_list.end(), hwnd);
    if (it != window::_active_window_list.end()) window::_active_window_list.erase(it);
    else window::last_error = error(errors::invalid_operation, "window not found in active window list");
    print("Window destroyed: hwnd = {}", uintptr_t(hwnd));
    ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
    if (window::_active_window_list.empty()) ::PostQuitMessage(0);
    break;
  }
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
    if (window::last_error.error.code != errors::success) return false;
    ::TranslateMessage(&msg), ::DispatchMessageW(&msg);
  }
  return true;
}
} // namespace yw
