#pragma once
#include "ywx/text_format.h"

namespace yw {

inline std::unexpected<error_trace> unexpected_win32_error(const char* msg, const source& src = {}) {
  return unexpected_error(errors::operation_failed, msg, int32_t(::GetLastError()), {}, src);
}

enum class window_style : uint32_t {
  unknown,
  regular = WS_OVERLAPPEDWINDOW,                    // flexible-sized window with title bar and border
  fixed = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, // fixed-sized window with title bar and border
  borderless = WS_POPUP,                            // fixed-sized window without title bar and border
};

class window;
class subwindow;

//////////////////////////////////////// MARK: control

class control {
public:
  virtual ~control() = default;
  control() = default;
};

//////////////////////////////////////// MARK: window_class

inline class {
  bool _initialized = false;
  static LRESULT __stdcall _proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

public:
  const HINSTANCE hinstance = ::GetModuleHandleW(nullptr);
  const std::wstring_view name = L"ywlib_window";
  slotlist<window> active_windows{};
  error_trace last_error{};

  std::expected<void, error_trace> initialize() {
    if (_initialized) return {};
    WNDCLASSW wc{};
    wc.lpfnWndProc = _proc;
    wc.hInstance = hinstance;
    wc.hCursor = ::LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wc.lpszClassName = name.data();
    if (!::RegisterClassW(&wc)) return unexpected_win32_error("RegisterClassW failed");
    else return _initialized = true, std::expected<void, error_trace>();
  }
} window_class;

//////////////////////////////////////// MARK: window_base

class window_base {
  window_base(const window_base&) = delete;
  window_base& operator=(const window_base&) = delete;
protected:
  window_base() = default;
public:
  HWND hwnd{};
  window_style style{};
  int4 margin{};
  stopwatch timer{};
  bitmap rendertarget{};
  comptr<::IDXGISwapChain1> swapchain{};
  slotlist<window>::id main_id{};
  slotlist<subwindow>::id sub_id{};

  window_base(window_base&&) noexcept = default;
  window_base& operator=(window_base&&) noexcept = default;

  std::expected<void, error_trace> create_window(const wchar_t* t, window_style s) {
    switch (style = s) {
    case window_style::regular:
    case window_style::fixed:
    case window_style::borderless: break;
    default: return unexpected_error(errors::invalid_argument, "invalid window style");
    }
    hwnd = ::CreateWindowExW(0, window_class.name.data(), t, DWORD(style), 0, 0, 0, 0, 0, 0, window_class.hinstance, 0);
    if (!hwnd) return unexpected_win32_error("CreateWindowExW failed");
    ::SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    return {};
  }
  std::expected<void, error_trace> calculate_padding() {
    RECT cr{}, wr{};
    if (!::GetClientRect(hwnd, &cr)) return unexpected_win32_error("GetClientRect failed");
    if (!::GetWindowRect(hwnd, &wr)) return unexpected_win32_error("GetWindowRect failed");
    const auto left = (wr.right - wr.left - cr.right) / 2, top = wr.bottom - wr.top - cr.bottom - left;
    margin = int4(left, top, 2 * left, left + top);
    return {};
  }
  std::expected<void, error_trace> set_sizepos(int2 s, int2 p) {
    if (::SetWindowPos(hwnd, nullptr, p.x, p.y, s.x + margin.z, s.y + margin.w, SWP_NOZORDER)) return {};
    else return unexpected_win32_error("SetWindowPos failed");
  }
  std::expected<void, error_trace> set_sizepos(int2 s) {
    if (HWND desktop; !(desktop = ::GetDesktopWindow())) return unexpected_win32_error("GetDesktopWindow failed");
    else if (RECT r; !::GetClientRect(desktop, &r)) return unexpected_win32_error("GetClientRect failed");
    else return set_sizepos(s, int2((r.right - s.x - margin.z) / 2, (r.bottom - s.y - margin.w) / 2));
  }
  void _show() {
    ::ShowWindow(hwnd, SW_SHOW);
    ::SetForegroundWindow(hwnd);
    ::SetActiveWindow(hwnd);
  }
  std::expected<void, error_trace> resize_d3d(uint2 size) {
    if (swapchain) {
      rendertarget = {};
      if (auto hr = swapchain->ResizeBuffers(0, size.x, size.y, DXGI_FORMAT_UNKNOWN, 0); FAILED(hr))
        return unexpected_error(errors::operation_failed, "ResizeBuffers failed", int32_t(hr));
    } else {
      if (auto res = dxgi.initialize(); !res) return unexpected_error(res.error());
      auto desc = DXGI_SWAP_CHAIN_DESC1(size.x, size.y, bitmap::dxgiformat, false, DXGI_SAMPLE_DESC(1, 0), {}, 2);
      desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT, desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
      auto hr = dxgi.factory()->CreateSwapChainForHwnd(d3d.device(), hwnd, &desc, nullptr, nullptr, &swapchain.get());
      if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateSwapChainForHwnd failed", int32_t(hr));
    }
    if (auto res = bitmap::create(swapchain.get()); !res) return unexpected_error(res.error());
    else return rendertarget = std::move(*res), std::expected<void, error_trace>{};
  }
  bool is_master() const noexcept { return sub_id == slotlist<subwindow>::id{}; }
};

///////////////////////////////////////// MARK: subwindow

class subwindow : public window_base {
  subwindow() = default;
public:
  slotlist<control> controls{};

  void present() {
    if (!(rendertarget && swapchain)) return;
    if (auto d = rendertarget.begin_draw(); d) for (const auto& c : controls) c->draw();
    swapchain->Present(1, 0);
  }

  /// \param style if unknown, inherits the style of the master window.
  static std::expected<window::slave, error_trace> open(const window::master& master, int2 pos, int2 size,
    null_terminated<wchar_t> title, window_style style = window_style::unknown, bool hidden = false);
};

///////////////////////////////////////// MARK: window

class window : public window_base {
  window() = default;
public:
  class master;
  class slave;

  slotlist<subwindow> slaves{};
  slotlist<control> controls{};

  void present() {
    if (!(rendertarget && swapchain)) return;
    if (auto d = rendertarget.begin_draw(); d) for (const auto& c : controls) c->draw();
    swapchain->Present(1, 0);
  }

  static std::expected<master, error_trace> open(int2 pos, int2 size, null_terminated<wchar_t> title,
    window_style style = window_style::regular, bool hidden = false);
  static std::expected<master, error_trace> open(
    int2 size, null_terminated<wchar_t> title, window_style style = window_style::regular, bool hidden = false);
};

//////////////////////////////////////// MARK: window procedure

inline LRESULT __stdcall decltype(window_class)::_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  auto self = reinterpret_cast<window_base*>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA));
  if (!self) return ::DefWindowProcW(hwnd, msg, wparam, lparam);
  switch (msg) {
  case WM_SIZE: {
    const auto width = LOWORD(lparam), height = HIWORD(lparam);
    if (auto res = self->resize_d3d(uint2(width, height)); !res) {
      window_class.last_error = res.error().push();
      print_error("Window resize failed", window_class.last_error);
    }
    return 0;
  }
  case WM_NCDESTROY:
    ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
    if (self->is_master()) window_class.active_windows.erase(self->main_id);
    else window_class.active_windows.get(self->main_id)->slaves.erase(self->sub_id);
    if (window_class.active_windows.empty()) ::PostQuitMessage(0);
    break;
  }
  return ::DefWindowProcW(hwnd, msg, wparam, lparam);
}

//////////////////////////////////////// MARK: mainloop

inline bool mainloop() {
  for (auto id : window_class.active_windows)
    if (auto wnd = window_class.active_windows.get(id); wnd) wnd->present();
  for (MSG msg; ::PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE);) {
    if (msg.message == WM_QUIT) return false;
    if (window_class.last_error.error.code != errors::success) return false;
    ::TranslateMessage(&msg), ::DispatchMessageW(&msg);
  }
  return true;
}

//////////////////////////////////////// MARK: window::master

class window::master {
  friend class window;
  friend class subwindow;
  slotlist<window>::id _id{};
  master(slotlist<window>::id id) : _id(id) {}
  master(const master&) = delete;
  master& operator=(const master&) = delete;
  window* _window() const { return window_class.active_windows.get(_id); }

public:
  master() = default;
  master(master&& other) noexcept : _id(std::exchange(other._id, slotlist<window>::id{})) {}
  master& operator=(master&& other) noexcept {
    if (this != &other) _id = std::exchange(other._id, slotlist<window>::id{});
    return *this;
  }
  ~master() {
    if (const auto w = _window(); w && w->hwnd) ::DestroyWindow(w->hwnd);
  }

  explicit operator bool() const noexcept { return _window() != nullptr; }

  window_style style() const {
    const auto w = _window();
    return w ? w->style : window_style::unknown;
  }

  std::expected<void, error_trace> show() {
    if (const auto w = _window(); !w) return ::ShowWindow(w->hwnd, SW_SHOW), std::expected<void, error_trace>{};
    else return unexpected_error(errors::invalid_operation, "window not found");
  }

  std::expected<void, error_trace> hide() {
    if (const auto w = _window(); !w) return ::ShowWindow(w->hwnd, SW_HIDE), std::expected<void, error_trace>{};
    else return unexpected_error(errors::invalid_operation, "window not found");
  }

  std::expected<void, error_trace> enable() {
    if (const auto w = _window(); !w)
      return w->timer.start(), ::EnableWindow(w->hwnd, TRUE), std::expected<void, error_trace>{};
    else return unexpected_error(errors::invalid_operation, "window not found");
  }

  std::expected<void, error_trace> disable() {
    if (const auto w = _window(); !w)
      return w->timer.stop(), ::EnableWindow(w->hwnd, FALSE), std::expected<void, error_trace>{};
    else return unexpected_error(errors::invalid_operation, "window not found");
  }

  std::expected<int2, error_trace> size() const {
    if (const auto w = _window(); w) {
      if (RECT r; ::GetClientRect(w->hwnd, &r)) return int2{r.right - r.left, r.bottom - r.top};
      else return unexpected_win32_error("GetClientRect failed");
    } else return unexpected_error(errors::invalid_operation, "window not found");
  }

  std::expected<void, error_trace> size(int2 size) {
    if (const auto w = _window(); w) {
      if (::SetWindowPos(w->hwnd, nullptr, 0, 0, size.x, size.y, SWP_NOMOVE | SWP_NOZORDER)) return {};
      else return unexpected_win32_error("SetWindowPos failed");
    } else return unexpected_error(errors::invalid_operation, "window not found");
  }

  std::expected<int2, error_trace> position() const {
    if (const auto w = _window(); w) {
      if (RECT r; ::GetWindowRect(w->hwnd, &r)) return int2(r.left, r.top);
      else return unexpected_win32_error("GetWindowRect failed");
    } else return unexpected_error(errors::invalid_operation, "window not found");
  }

  std::expected<void, error_trace> position(int2 pos) {
    if (const auto w = _window(); w) {
      if (::SetWindowPos(w->hwnd, nullptr, pos.x, pos.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER)) return {};
      else return unexpected_win32_error("SetWindowPos failed");
    } else return unexpected_error(errors::invalid_operation, "window not found");
  }

  std::expected<drawing, error_trace> begin_draw(const source& src = {}) {
    if (const auto w = _window(); w) {
      return w->rendertarget.begin_draw(src);
    } else return unexpected_error(errors::invalid_operation, "window not found");
  }

  std::expected<drawing, error_trace> begin_draw(const color& clear_color, const source& src = {}) {
    if (const auto w = _window(); w) {
      return w->rendertarget.begin_draw(clear_color, src);
    } else return unexpected_error(errors::invalid_operation, "window not found");
  }

  std::expected<void, error_trace> close() {
    if (const auto w = _window(); w) {
      if (::DestroyWindow(w->hwnd)) return {};
      else return unexpected_win32_error("DestroyWindow failed");
    } else return unexpected_error(errors::invalid_operation, "window not found");
  }
};

inline std::expected<window::master, error_trace> window::open(
  int2 pos, int2 size, null_terminated<wchar_t> title, window_style style, bool hidden) {
  if (auto res = window_class.initialize(); !res) return unexpected_error(res.error());
  std::unique_ptr<window> w = std::make_unique<window>(window());
  if (auto res = w->create_window(title.data(), style); !res) return unexpected_error(res.error());
  if (auto res = w->calculate_padding(); !res) return unexpected_error(res.error());
  if (auto res = w->set_sizepos(size, pos); !res) return unexpected_error(res.error());
  if (!hidden) w->_show();
  auto main_id = window_class.active_windows.push(std::move(w));
  auto p = window_class.active_windows.get(main_id);
  return window::master(p->main_id = main_id);
}

inline std::expected<window::master, error_trace> window::open(
  int2 size, null_terminated<wchar_t> title, window_style style, bool hidden) {
  if (auto res = window_class.initialize(); !res) return unexpected_error(res.error());
  std::unique_ptr<window> w = std::make_unique<window>(window());
  if (auto res = w->create_window(title.data(), style); !res) return unexpected_error(res.error());
  if (auto res = w->calculate_padding(); !res) return unexpected_error(res.error());
  if (auto res = w->set_sizepos(size); !res) return unexpected_error(res.error());
  if (!hidden) w->_show();
  auto main_id = window_class.active_windows.push(std::move(w));
  auto p = window_class.active_windows.get(main_id);
  return window::master(p->main_id = main_id);
}

//////////////////////////////////////// MARK: window::slave

class window::slave {
  friend class subwindow;
  slotlist<subwindow>::id _id{};
  slotlist<window>::id _master{};
  slave(slotlist<subwindow>::id id, slotlist<window>::id master) : _id(id), _master(master) {}
  slave(const slave&) = delete;
  slave& operator=(const slave&) = delete;
  window* _window() const { return window_class.active_windows.get(_master); }
  subwindow* _subwindow(window* w) const { return w->slaves.get(_id); }

public:
  slave() = default;
  slave(slave&& other) noexcept
    : _id(std::exchange(other._id, slotlist<subwindow>::id{})),
      _master(std::exchange(other._master, slotlist<window>::id{})) {}
  slave& operator=(slave&& other) noexcept {
    if (this == &other) return *this;
    _id = std::exchange(other._id, slotlist<subwindow>::id{});
    _master = std::exchange(other._master, slotlist<window>::id{});
    return *this;
  }
  ~slave() {
    if (const auto w = _window(); !w) return;
    else if (const auto sw = _subwindow(w); sw && sw->hwnd) ::DestroyWindow(sw->hwnd);
  }

  explicit operator bool() const noexcept {
    const auto w = _window();
    return w && _subwindow(w);
  }

  window_style style() const {
    const auto w = _window();
    if (!w) return window_style::unknown;
    const auto sw = _subwindow(w);
    return sw ? sw->style : window_style::unknown;
  }

  std::expected<void, error_trace> show() {
    if (const auto w = _window(); !w) return unexpected_error(errors::invalid_operation, "master window not found");
    else if (const auto sw = _subwindow(w); sw)
      return ::ShowWindow(sw->hwnd, SW_SHOW), std::expected<void, error_trace>{};
    else return unexpected_error(errors::invalid_operation, "subwindow not found");
  }

  std::expected<void, error_trace> hide() {
    if (const auto w = _window(); !w) return unexpected_error(errors::invalid_operation, "master window not found");
    else if (const auto sw = _subwindow(w); sw)
      return ::ShowWindow(sw->hwnd, SW_HIDE), std::expected<void, error_trace>{};
    else return unexpected_error(errors::invalid_operation, "subwindow not found");
  }

  std::expected<void, error_trace> enable() {
    if (const auto w = _window(); !w) return unexpected_error(errors::invalid_operation, "master window not found");
    else if (const auto sw = _subwindow(w); sw)
      return sw->timer.start(), ::EnableWindow(sw->hwnd, TRUE), std::expected<void, error_trace>{};
    else return unexpected_error(errors::invalid_operation, "subwindow not found");
  }

  std::expected<void, error_trace> disable() {
    if (const auto w = _window(); !w) return unexpected_error(errors::invalid_operation, "master window not found");
    else if (const auto sw = _subwindow(w); sw)
      return sw->timer.stop(), ::EnableWindow(sw->hwnd, FALSE), std::expected<void, error_trace>{};
    else return unexpected_error(errors::invalid_operation, "subwindow not found");
  }

  std::expected<int2, error_trace> size() const {
    if (const auto w = _window(); !w) return unexpected_error(errors::invalid_operation, "master window not found");
    else if (const auto sw = _subwindow(w); sw) {
      if (RECT r; ::GetClientRect(sw->hwnd, &r)) return int2{r.right - r.left, r.bottom - r.top};
      else return unexpected_win32_error("GetClientRect failed");
    } else return unexpected_error(errors::invalid_operation, "subwindow not found");
  }

  std::expected<void, error_trace> size(int2 size) {
    if (const auto w = _window(); !w) return unexpected_error(errors::invalid_operation, "master window not found");
    else if (const auto sw = _subwindow(w); sw) {
      if (::SetWindowPos(sw->hwnd, nullptr, 0, 0, size.x, size.y, SWP_NOMOVE | SWP_NOZORDER)) return {};
      else return unexpected_win32_error("SetWindowPos failed");
    } else return unexpected_error(errors::invalid_operation, "subwindow not found");
  }

  std::expected<int2, error_trace> position() const {
    if (const auto w = _window(); !w) return unexpected_error(errors::invalid_operation, "master window not found");
    else if (const auto sw = _subwindow(w); sw) {
      if (RECT mr; !::GetWindowRect(w->hwnd, &mr)) return unexpected_win32_error("GetWindowRect failed");
      else if (RECT r; ::GetWindowRect(sw->hwnd, &r)) return int2(r.left, r.top);
      else return unexpected_win32_error("GetWindowRect failed");
    } else return unexpected_error(errors::invalid_operation, "subwindow not found");
  }

  std::expected<void, error_trace> position(int2 pos) {
    if (const auto w = _window(); !w) return unexpected_error(errors::invalid_operation, "master window not found");
    else if (const auto sw = _subwindow(w); sw) {
      if (::SetWindowPos(sw->hwnd, nullptr, pos.x, pos.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER)) return {};
      else return unexpected_win32_error("SetWindowPos failed");
    } else return unexpected_error(errors::invalid_operation, "subwindow not found");
  }

  std::expected<drawing, error_trace> begin_draw(const source& src = {}) {
    if (const auto w = _window(); !w) return unexpected_error(errors::invalid_operation, "master window not found");
    else if (const auto sw = _subwindow(w); sw) return sw->rendertarget.begin_draw(src);
    else return unexpected_error(errors::invalid_operation, "subwindow not found");
  }

  std::expected<drawing, error_trace> begin_draw(const color& clear_color, const source& src = {}) {
    if (const auto w = _window(); !w) return unexpected_error(errors::invalid_operation, "master window not found");
    else if (const auto sw = _subwindow(w); sw) return sw->rendertarget.begin_draw(clear_color, src);
    else return unexpected_error(errors::invalid_operation, "subwindow not found");
  }

  std::expected<void, error_trace> close() {
    if (const auto w = _window(); !w) return unexpected_error(errors::invalid_operation, "master window not found");
    else if (const auto sw = _subwindow(w); sw) {
      if (::DestroyWindow(sw->hwnd)) return {};
      else return unexpected_win32_error("DestroyWindow failed");
    } else return unexpected_error(errors::invalid_operation, "subwindow not found");
  }
};

inline std::expected<window::slave, error_trace> subwindow::open(
  const window::master& master, int2 pos, int2 size, null_terminated<wchar_t> title, window_style style, bool hidden) {
  const auto mw = master._window();
  if (!master) return unexpected_error(errors::invalid_argument, "invalid master window");
  style = style == window_style::unknown ? master.style() : style;
  if (style == window_style::unknown) return unexpected_error(errors::invalid_argument, "invalid window style");
  if (RECT r; !::GetWindowRect(mw->hwnd, &r)) return unexpected_win32_error("GetWindowRect failed");
  else pos = pos + int2(r.left, r.top);
  std::unique_ptr<subwindow> sw = std::make_unique<subwindow>(subwindow());
  if (auto res = sw->create_window(title.data(), style); !res) return unexpected_error(res.error());
  if (auto res = sw->calculate_padding(); !res) return unexpected_error(res.error());
  if (auto res = sw->set_sizepos(size, pos); !res) return unexpected_error(res.error());
  if (!hidden) sw->_show();
  auto sub_id = mw->slaves.push(std::move(sw));
  auto p = mw->slaves.get(sub_id);
  return window::slave(p->sub_id = sub_id, p->main_id = master._id);
}
} // namespace yw
