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
class control;

//////////////////////////////////////// MARK: window_class

inline class {
  bool _initialized = false;
public:
  static LRESULT __stdcall proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
  const HINSTANCE hinstance = ::GetModuleHandleW(nullptr);
  const std::wstring_view name = L"ywlib_window";
  pointerlist<window> windows{};
  error_trace last_error{};

  std::expected<void, error_trace> initialize() {
    if (_initialized) return {};
    WNDCLASSW wc{};
    wc.lpfnWndProc = proc;
    wc.hInstance = hinstance;
    wc.hCursor = ::LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wc.lpszClassName = name.data();
    if (!::RegisterClassW(&wc)) return unexpected_win32_error("RegisterClassW failed");
    else return _initialized = true, std::expected<void, error_trace>();
  }
} window_class;

//////////////////////////////////////// MARK: control

class control {
  pointerlist<window>::id _window_id{};
  pointerlist<subwindow>::id _subwindow_id{};
  pointerlist<control>::id _control_id{};
  pointerlist<control>* _get_control_list();

  control(const control&) = delete;
  control& operator=(const control&) = delete;
public:
  float2 position{}, size{}, radius{};
  color background = colors::white, border = colors::black;
  float border_width{};
  bool visible = true, enabled = true;

  control() = default;

  control(control&& other) noexcept
    : _window_id(std::exchange(other._window_id, pointerlist<window>::id{})),
      _subwindow_id(std::exchange(other._subwindow_id, pointerlist<subwindow>::id{})),
      _control_id(std::exchange(other._control_id, pointerlist<control>::id{})),
      position(other.position), size(other.size), radius(other.radius),
      background(other.background), border(other.border), border_width(other.border_width), visible(other.visible),
      enabled(other.enabled) {
    if (const auto list = _get_control_list(); !list) return;
    else list->update(_control_id, *this);
  }

  control& operator=(control&& other) noexcept {
    if (this == &other) return *this;
    if (const auto list = _get_control_list(); !list) return *this;
    else list->erase(_control_id);
    _window_id = std::exchange(other._window_id, pointerlist<window>::id{});
    _subwindow_id = std::exchange(other._subwindow_id, pointerlist<subwindow>::id{});
    _control_id = std::exchange(other._control_id, pointerlist<control>::id{});
    position = other.position, size = other.size, radius = other.radius;
    background = other.background, border = other.border;
    border_width = other.border_width;
    visible = other.visible, enabled = other.enabled;
    if (const auto list = _get_control_list(); !list) return *this;
    else list->update(_control_id, *this);
    return *this;
  }

  virtual ~control() {
    if (const auto list = _get_control_list(); !list) return;
    else list->erase(_control_id);
  }

  virtual void draw() {
    if (!visible) return;
    fill_round_rectangle(position, size, radius, background);
    draw_round_rectangle(position, size, radius, border, border_width);
  }
};

//////////////////////////////////////// MARK: window

class window {
  friend class subwindow;
  friend class control;
  friend LRESULT __stdcall decltype(window_class)::proc(HWND, UINT, WPARAM, LPARAM);

  window(const window&) = delete;
  window& operator=(const window&) = delete;

protected:
  HWND _hwnd{};
  window_style _style{};
  int4 _margin{};
  stopwatch _timer{};
  bitmap _rendertarget{};
  comptr<::IDXGISwapChain1> _swapchain{};
  pointerlist<window>::id _window_id{};
  pointerlist<subwindow>::id _subwindow_id{};
  pointerlist<subwindow> _subwindows{};
  pointerlist<control> _controls{};

  std::expected<void, error_trace> _create_window(const wchar_t* t, window_style s) {
    switch (_style = s) {
    case window_style::regular:
    case window_style::fixed:
    case window_style::borderless: break;
    default: return unexpected_error(errors::invalid_argument, "invalid window style");
    }
    _hwnd = ::CreateWindowExW(
      WS_EX_ACCEPTFILES, window_class.name.data(), t, DWORD(_style), 0, 0, 0, 0, 0, 0, window_class.hinstance, 0);
    if (!_hwnd) return unexpected_win32_error("CreateWindowExW failed");
    ::SetWindowLongPtrW(_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    return {};
  }

  std::expected<void, error_trace> _calculate_padding() {
    RECT cr{}, wr{};
    if (!::GetClientRect(_hwnd, &cr)) return unexpected_win32_error("GetClientRect failed");
    if (!::GetWindowRect(_hwnd, &wr)) return unexpected_win32_error("GetWindowRect failed");
    const auto left = (wr.right - wr.left - cr.right) / 2, top = wr.bottom - wr.top - cr.bottom - left;
    _margin = int4(left, top, 2 * left, left + top);
    return {};
  }

  std::expected<void, error_trace> _set_sizepos(int2 s, int2 p) {
    if (::SetWindowPos(_hwnd, nullptr, p.x, p.y, s.x + _margin.z, s.y + _margin.w, SWP_NOZORDER)) return {};
    else return unexpected_win32_error("SetWindowPos failed");
  }

  std::expected<void, error_trace> _set_sizepos(int2 s) {
    if (HWND desktop; !(desktop = ::GetDesktopWindow())) return unexpected_win32_error("GetDesktopWindow failed");
    else if (RECT r; !::GetClientRect(desktop, &r)) return unexpected_win32_error("GetClientRect failed");
    else return _set_sizepos(s, int2((r.right - s.x - _margin.z) / 2, (r.bottom - s.y - _margin.w) / 2));
  }

  std::expected<void, error_trace> _resize_d3d(uint2 size) {
    if (_swapchain) {
      _rendertarget = {};
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
    else return _rendertarget = std::move(*res), std::expected<void, error_trace>{};
  }

public:
  bool close_confirmation{};

  window() noexcept = default;

  window(window&& other) noexcept
    : _hwnd(std::exchange(other._hwnd, nullptr)), _style(std::exchange(other._style, window_style::unknown)),
      _margin(std::exchange(other._margin, int4{})), _timer(std::move(other._timer)),
      _rendertarget(std::move(other._rendertarget)), _swapchain(std::move(other._swapchain)),
      _window_id(std::exchange(other._window_id, pointerlist<window>::id{})),
      _subwindow_id(std::exchange(other._subwindow_id, pointerlist<subwindow>::id{})),
      _subwindows(std::move(other._subwindows)), _controls(std::move(other._controls)) {
    if (!_hwnd) return;
    window_class.windows.update(_window_id, *this);
    ::SetWindowLongPtrW(_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
  }

  window& operator=(window&& other) noexcept {
    if (this == &other) return *this;
    if (_hwnd) ::DestroyWindow(_hwnd); // pointerlistの操作はWM_DESTROYで行う
    _hwnd = std::exchange(other._hwnd, nullptr);
    _style = std::exchange(other._style, window_style::unknown);
    _margin = std::exchange(other._margin, int4{});
    _timer = std::move(other._timer);
    _rendertarget = std::move(other._rendertarget);
    _swapchain = std::move(other._swapchain);
    _window_id = std::exchange(other._window_id, pointerlist<window>::id{});
    _subwindow_id = std::exchange(other._subwindow_id, pointerlist<subwindow>::id{});
    _subwindows = std::move(other._subwindows);
    _controls = std::move(other._controls);
    if (!_hwnd) return *this;
    window_class.windows.update(_window_id, *this);
    ::SetWindowLongPtrW(_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    return *this;
  }

  ~window() {
    if (_hwnd) ::DestroyWindow(_hwnd); // pointerlistの操作はWM_DESTROYで行う
  }

  explicit operator bool() const noexcept { return _hwnd != nullptr; }
  window_style style() const noexcept { return _style; }
  int2 margin() const noexcept { return int2(_margin.x, _margin.y); }
  const stopwatch& timer() const noexcept { return _timer; }
  const bitmap& rendertarget() const noexcept { return _rendertarget; }
  bool is_subwindow() const noexcept { return _subwindow_id != pointerlist<subwindow>::id{}; }

  void show() {
    if (!operator bool()) return;
    ::ShowWindow(_hwnd, SW_SHOW);
    ::SetForegroundWindow(_hwnd);
    ::SetActiveWindow(_hwnd);
  }

  void hide() {
    if (!operator bool()) return;
    ::ShowWindow(_hwnd, SW_HIDE);
  }

  void enable() {
    if (!operator bool()) return;
    _timer.start();
    ::EnableWindow(_hwnd, TRUE);
  }

  void disable() {
    if (!operator bool()) return;
    _timer.stop();
    ::EnableWindow(_hwnd, FALSE);
  }

  int2 size() const {
    if (!operator bool()) return {};
    if (RECT r; !::GetClientRect(_hwnd, &r)) return {};
    else return int2{r.right - r.left, r.bottom - r.top};
  }

  void size(int2 size) {
    if (!operator bool()) return;
    ::SetWindowPos(_hwnd, nullptr, 0, 0, size.x + _margin.z, size.y + _margin.w, SWP_NOMOVE | SWP_NOZORDER);
  }

  /// gets the left-top position of the window including non-client area
  virtual int2 position() const {
    if (!operator bool()) return {};
    if (RECT r; !::GetWindowRect(_hwnd, &r)) return {};
    else return int2{r.left, r.top};
  }

  /// sets the left-top position of the window including non-client area
  virtual void position(int2 pos) {
    if (!operator bool()) return;
    ::SetWindowPos(_hwnd, nullptr, pos.x, pos.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
  }

  std::expected<drawing, error_trace> begin_draw(const source& src = {}) {
    if (!operator bool()) return unexpected_error(errors::not_initialized, "window not initialized");
    return _rendertarget.begin_draw(src);
  }

  std::expected<drawing, error_trace> begin_draw(const color& clear_color, const source& src = {}) {
    if (!operator bool()) return unexpected_error(errors::not_initialized, "window not initialized");
    return _rendertarget.begin_draw(clear_color, src);
  }

  void present();
  void close() { *this = {}; }

  static std::expected<window, error_trace> open(int2 pos, int2 size, null_terminated<wchar_t> title,
    window_style style = window_style::regular, bool hidden = false) {
    if (auto res = window_class.initialize(); !res) return unexpected_error(res.error());
    window w;
    if (auto res = w._create_window(title.data(), style); !res) return unexpected_error(res.error());
    if (auto res = w._calculate_padding(); !res) return unexpected_error(res.error());
    if (auto res = w._set_sizepos(size, pos); !res) return unexpected_error(res.error());
    if (!hidden) w.show();
    w._window_id = window_class.windows.push(w);
    return std::move(w);
  }

  static std::expected<window, error_trace> open(
    int2 size, null_terminated<wchar_t> title, window_style style = window_style::regular, bool hidden = false) {
    if (auto res = window_class.initialize(); !res) return unexpected_error(res.error());
    window w;
    if (auto res = w._create_window(title.data(), style); !res) return unexpected_error(res.error());
    if (auto res = w._calculate_padding(); !res) return unexpected_error(res.error());
    if (auto res = w._set_sizepos(size); !res) return unexpected_error(res.error());
    if (!hidden) w.show();
    w._window_id = window_class.windows.push(w);
    return std::move(w);
  }
};

//////////////////////////////////////// MARK: subwindow

class subwindow : public window {
  using window::open;

public:
  /// gets the left-top position of the subwindow client area relative to the master window
  int2 position() const override {
    if (!operator bool()) return {};
    if (const auto master = window_class.windows.get(_window_id); !master) return {};
    else if (RECT r; !::GetWindowRect(_hwnd, &r)) return {};
    else return int2(r.left, r.top) - master->position();
  }

  /// sets the left-top position of the subwindow client area relative to the master window
  void position(int2 pos) override {
    if (!operator bool()) return;
    const auto master = window_class.windows.get(_window_id);
    if (!master) return;
    const auto mpos = master->position();
    ::SetWindowPos(_hwnd, 0, mpos.x + pos.x, mpos.y + pos.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
  }

  void present() {
    if (!operator bool()) return;
    if (auto d = begin_draw(); d)
      for (const auto& c : _controls) _controls.get(c)->draw();
    _swapchain->Present(1, 0);
  }

  static std::expected<subwindow, error_trace> open(window& master, int2 pos, int2 size,
    null_terminated<wchar_t> title, window_style style = window_style::unknown, bool hidden = false) {
    if (!master) return unexpected_error(errors::invalid_argument, "master window is not valid");
    if (master.is_subwindow()) return unexpected_error(errors::invalid_argument, "cannot create subwindow of a subwindow");
    if (style == window_style::unknown) style = master.style();
    if (style == window_style::unknown) return unexpected_error(errors::invalid_argument, "invalid window style");
    subwindow w;
    if (auto res = w._create_window(title.data(), style); !res) return unexpected_error(res.error());
    if (auto res = w._calculate_padding(); !res) return unexpected_error(res.error());
    if (auto res = w._set_sizepos(size, pos + master.position()); !res) return unexpected_error(res.error());
    if (!hidden) w.show();
    w._window_id = master._window_id;
    w._subwindow_id = master._subwindows.push(w);
    return std::move(w);
  }
};

inline pointerlist<control>* control::_get_control_list() {
  if (const auto master = window_class.windows.get(_window_id); !master) return nullptr;
  else if (_subwindow_id != pointerlist<subwindow>::id{}) {
    if (const auto subwin = master->_subwindows.get(_subwindow_id); !subwin) return nullptr;
    else return &subwin->_controls;
  } else return &master->_controls;
}

inline void window::present()  {
  if (!operator bool()) return;
  if (auto d = begin_draw(); d)
    for (const auto& ctrl : _controls) _controls.get(ctrl)->draw();
  _swapchain->Present(1, 0);
  for (auto& subwin : _subwindows) _subwindows.get(subwin)->present();
}

//////////////////////////////////////// MARK: window procedure

inline LRESULT __stdcall decltype(window_class)::proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  auto self = reinterpret_cast<window*>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA));
  if (!self) return ::DefWindowProcW(hwnd, msg, wparam, lparam);
  switch (msg) {
  case WM_SIZE: {
    const auto width = LOWORD(lparam), height = HIWORD(lparam);
    if (auto res = self->_resize_d3d(uint2(width, height)); !res) window_class.last_error = res.error().push();
    return 0;
  }
  case WM_CLOSE:
    if (self->close_confirmation && ::MessageBoxW(hwnd, L"Close window?", L"Confirmation", MB_YESNO) == IDNO) return 0;
    return ::DestroyWindow(hwnd), 0;
  case WM_NCDESTROY:
    ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
    if (!self->is_subwindow()) {
      for (auto& subwin : self->_subwindows) self->_subwindows.get(subwin)->close();
      window_class.windows.erase(self->_window_id);
      if (window_class.windows.empty()) ::PostQuitMessage(0);
    } else if (const auto master = window_class.windows.get(self->_window_id); master)
      master->_subwindows.erase(self->_subwindow_id);
    break;
  }
  return ::DefWindowProcW(hwnd, msg, wparam, lparam);
}

//////////////////////////////////////// MARK: mainloop

inline bool mainloop() {
  for (auto id : window_class.windows) window_class.windows.get(id)->present();
  for (MSG msg; ::PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE);) {
    if (msg.message == WM_QUIT) return false;
    if (window_class.last_error.error.code != errors::success) return false;
    ::TranslateMessage(&msg), ::DispatchMessageW(&msg);
  }
  return true;
}
} // namespace yw

// //////////////////////////////////////// MARK: control

// class control {
//   slotlist<window>::id _window_id{};
//   slotlist<subwindow>::id _subwindow_id{};
//   slotlist<control>::id _control_id{};
// public:
//   float2 position{}, size{}, radius{};
//   color background = colors::white, border = colors::black;
//   float border_width{};
//   bool visible = true, enabled = true;

//   virtual ~control() = default;
//   control() = default;

//   virtual void draw() {
//     if (!visible) return;
//     fill_round_rectangle(position, size, radius, background);
//     draw_round_rectangle(position, size, radius, border, border_width);
//   }
// };

// ///////////////////////////////////////// MARK: window

// class window : public window_base {
//   window() = default;

// public:
//   class master;
//   class slave;

//   slotlist<subwindow> slaves{};
//   slotlist<control> controls{};

//   void present();

//   static std::expected<master, error_trace> open(int2 pos, int2 size, null_terminated<wchar_t> title,
//     window_style style = window_style::regular, bool hidden = false);
//   static std::expected<master, error_trace> open(
//     int2 size, null_terminated<wchar_t> title, window_style style = window_style::regular, bool hidden = false);
// };

// ///////////////////////////////////////// MARK: subwindow

// class subwindow : public window_base {
//   subwindow() = default;

// public:
//   slotlist<control> controls{};

//   void present() {
//     if (!(rendertarget && swapchain)) return;
//     if (auto d = rendertarget.begin_draw(); d)
//       for (const auto& c : controls) controls.get(c)->draw();
//     swapchain->Present(1, 0);
//   }

//   /// \param style if unknown, inherits the style of the master window.
//   static std::expected<window::slave, error_trace> open(const window::master& master, int2 pos, int2 size,
//     null_terminated<wchar_t> title, window_style style = window_style::unknown, bool hidden = false);
// };

// inline void window::present() {
//   if (!(rendertarget && swapchain)) return;
//   if (auto d = rendertarget.begin_draw(); d)
//     for (const auto& c : controls) controls.get(c)->draw();
//   swapchain->Present(1, 0);
//   for (auto s : slaves) slaves.get(s)->present();
// }

// //////////////////////////////////////// MARK: window::master

// class window::master {
//   friend class window;
//   friend class subwindow;
//   slotlist<window>::id _id{};
//   master(slotlist<window>::id id) : _id(id) {}
//   master(const master&) = delete;
//   master& operator=(const master&) = delete;
//   window* _window() const { return window_class.windows.get(_id); }

// public:
//   master() = default;
//   master(master&& other) noexcept : _id(std::exchange(other._id, slotlist<window>::id{})) {}
//   master& operator=(master&& other) noexcept {
//     if (this != &other) _id = std::exchange(other._id, slotlist<window>::id{});
//     return *this;
//   }
//   ~master() {
//     if (const auto w = _window(); w && w->hwnd) ::DestroyWindow(w->hwnd);
//   }

//   explicit operator bool() const noexcept { return _window() != nullptr; }

//   window_style style() const {
//     const auto w = _window();
//     return w ? w->style : window_style::unknown;
//   }

//   std::expected<void, error_trace> show() {
//     if (const auto w = _window(); !w) return ::ShowWindow(w->hwnd, SW_SHOW), std::expected<void, error_trace>{};
//     else return unexpected_error(errors::invalid_operation, "window not found");
//   }

//   std::expected<void, error_trace> hide() {
//     if (const auto w = _window(); !w) return ::ShowWindow(w->hwnd, SW_HIDE), std::expected<void, error_trace>{};
//     else return unexpected_error(errors::invalid_operation, "window not found");
//   }

//   std::expected<void, error_trace> enable() {
//     if (const auto w = _window(); !w)
//       return w->timer.start(), ::EnableWindow(w->hwnd, TRUE), std::expected<void, error_trace>{};
//     else return unexpected_error(errors::invalid_operation, "window not found");
//   }

//   std::expected<void, error_trace> disable() {
//     if (const auto w = _window(); !w)
//       return w->timer.stop(), ::EnableWindow(w->hwnd, FALSE), std::expected<void, error_trace>{};
//     else return unexpected_error(errors::invalid_operation, "window not found");
//   }

//   std::expected<int2, error_trace> size() const {
//     if (const auto w = _window(); w) {
//       if (RECT r; ::GetClientRect(w->hwnd, &r)) return int2{r.right - r.left, r.bottom - r.top};
//       else return unexpected_win32_error("GetClientRect failed");
//     } else return unexpected_error(errors::invalid_operation, "window not found");
//   }

//   std::expected<void, error_trace> size(int2 size) {
//     if (const auto w = _window(); w) {
//       if (::SetWindowPos(w->hwnd, nullptr, 0, 0, size.x, size.y, SWP_NOMOVE | SWP_NOZORDER)) return {};
//       else return unexpected_win32_error("SetWindowPos failed");
//     } else return unexpected_error(errors::invalid_operation, "window not found");
//   }

//   std::expected<int2, error_trace> position() const {
//     if (const auto w = _window(); w) {
//       if (RECT r; ::GetWindowRect(w->hwnd, &r)) return int2(r.left, r.top);
//       else return unexpected_win32_error("GetWindowRect failed");
//     } else return unexpected_error(errors::invalid_operation, "window not found");
//   }

//   std::expected<void, error_trace> position(int2 pos) {
//     if (const auto w = _window(); w) {
//       if (::SetWindowPos(w->hwnd, nullptr, pos.x, pos.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER)) return {};
//       else return unexpected_win32_error("SetWindowPos failed");
//     } else return unexpected_error(errors::invalid_operation, "window not found");
//   }

//   std::expected<drawing, error_trace> begin_draw(const source& src = {}) {
//     if (const auto w = _window(); w) {
//       return w->rendertarget.begin_draw(src);
//     } else return unexpected_error(errors::invalid_operation, "window not found");
//   }

//   std::expected<drawing, error_trace> begin_draw(const color& clear_color, const source& src = {}) {
//     if (const auto w = _window(); w) {
//       return w->rendertarget.begin_draw(clear_color, src);
//     } else return unexpected_error(errors::invalid_operation, "window not found");
//   }

//   std::expected<void, error_trace> close() {
//     if (const auto w = _window(); w) {
//       if (::DestroyWindow(w->hwnd)) return {};
//       else return unexpected_win32_error("DestroyWindow failed");
//     } else return unexpected_error(errors::invalid_operation, "window not found");
//   }

//   void set_close_confirmation(bool enable) {
//     if (const auto w = _window(); w) w->close_confirmation = enable;
//   }
// };

// //////////////////////////////////////// MARK: window::slave

// class window::slave {
//   friend class subwindow;
//   slotlist<subwindow>::id _id{};
//   slotlist<window>::id _master{};
//   slave(slotlist<subwindow>::id id, slotlist<window>::id master) : _id(id), _master(master) {}
//   slave(const slave&) = delete;
//   slave& operator=(const slave&) = delete;
//   window* _window() const { return window_class.windows.get(_master); }
//   subwindow* _subwindow(window* w) const { return w->slaves.get(_id); }

// public:
//   slave() = default;
//   slave(slave&& other) noexcept
//     : _id(std::exchange(other._id, slotlist<subwindow>::id{})),
//       _master(std::exchange(other._master, slotlist<window>::id{})) {}
//   slave& operator=(slave&& other) noexcept {
//     if (this == &other) return *this;
//     _id = std::exchange(other._id, slotlist<subwindow>::id{});
//     _master = std::exchange(other._master, slotlist<window>::id{});
//     return *this;
//   }
//   ~slave() {
//     if (const auto w = _window(); !w) return;
//     else if (const auto sw = _subwindow(w); sw && sw->hwnd) ::DestroyWindow(sw->hwnd);
//   }

//   explicit operator bool() const noexcept {
//     const auto w = _window();
//     return w && _subwindow(w);
//   }

//   window_style style() const {
//     const auto w = _window();
//     if (!w) return window_style::unknown;
//     const auto sw = _subwindow(w);
//     return sw ? sw->style : window_style::unknown;
//   }

//   std::expected<void, error_trace> show() {
//     if (const auto w = _window(); !w) return unexpected_error(errors::invalid_operation, "master window not found");
//     else if (const auto sw = _subwindow(w); sw)
//       return ::ShowWindow(sw->hwnd, SW_SHOW), std::expected<void, error_trace>{};
//     else return unexpected_error(errors::invalid_operation, "subwindow not found");
//   }

//   std::expected<void, error_trace> hide() {
//     if (const auto w = _window(); !w) return unexpected_error(errors::invalid_operation, "master window not found");
//     else if (const auto sw = _subwindow(w); sw)
//       return ::ShowWindow(sw->hwnd, SW_HIDE), std::expected<void, error_trace>{};
//     else return unexpected_error(errors::invalid_operation, "subwindow not found");
//   }

//   std::expected<void, error_trace> enable() {
//     if (const auto w = _window(); !w) return unexpected_error(errors::invalid_operation, "master window not found");
//     else if (const auto sw = _subwindow(w); sw)
//       return sw->timer.start(), ::EnableWindow(sw->hwnd, TRUE), std::expected<void, error_trace>{};
//     else return unexpected_error(errors::invalid_operation, "subwindow not found");
//   }

//   std::expected<void, error_trace> disable() {
//     if (const auto w = _window(); !w) return unexpected_error(errors::invalid_operation, "master window not found");
//     else if (const auto sw = _subwindow(w); sw)
//       return sw->timer.stop(), ::EnableWindow(sw->hwnd, FALSE), std::expected<void, error_trace>{};
//     else return unexpected_error(errors::invalid_operation, "subwindow not found");
//   }

//   std::expected<int2, error_trace> size() const {
//     if (const auto w = _window(); !w) return unexpected_error(errors::invalid_operation, "master window not found");
//     else if (const auto sw = _subwindow(w); sw) {
//       if (RECT r; ::GetClientRect(sw->hwnd, &r)) return int2{r.right - r.left, r.bottom - r.top};
//       else return unexpected_win32_error("GetClientRect failed");
//     } else return unexpected_error(errors::invalid_operation, "subwindow not found");
//   }

//   std::expected<void, error_trace> size(int2 size) {
//     if (const auto w = _window(); !w) return unexpected_error(errors::invalid_operation, "master window not found");
//     else if (const auto sw = _subwindow(w); sw) {
//       if (::SetWindowPos(sw->hwnd, nullptr, 0, 0, size.x, size.y, SWP_NOMOVE | SWP_NOZORDER)) return {};
//       else return unexpected_win32_error("SetWindowPos failed");
//     } else return unexpected_error(errors::invalid_operation, "subwindow not found");
//   }

//   std::expected<int2, error_trace> position() const {
//     if (const auto w = _window(); !w) return unexpected_error(errors::invalid_operation, "master window not found");
//     else if (const auto sw = _subwindow(w); sw) {
//       if (RECT mr; !::GetWindowRect(w->hwnd, &mr)) return unexpected_win32_error("GetWindowRect failed");
//       else if (RECT r; ::GetWindowRect(sw->hwnd, &r)) return int2(r.left, r.top);
//       else return unexpected_win32_error("GetWindowRect failed");
//     } else return unexpected_error(errors::invalid_operation, "subwindow not found");
//   }

//   std::expected<void, error_trace> position(int2 pos) {
//     if (const auto w = _window(); !w) return unexpected_error(errors::invalid_operation, "master window not found");
//     else if (const auto sw = _subwindow(w); sw) {
//       if (::SetWindowPos(sw->hwnd, nullptr, pos.x, pos.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER)) return {};
//       else return unexpected_win32_error("SetWindowPos failed");
//     } else return unexpected_error(errors::invalid_operation, "subwindow not found");
//   }

//   std::expected<drawing, error_trace> begin_draw(const source& src = {}) {
//     if (const auto w = _window(); !w) return unexpected_error(errors::invalid_operation, "master window not found");
//     else if (const auto sw = _subwindow(w); sw) return sw->rendertarget.begin_draw(src);
//     else return unexpected_error(errors::invalid_operation, "subwindow not found");
//   }

//   std::expected<drawing, error_trace> begin_draw(const color& clear_color, const source& src = {}) {
//     if (const auto w = _window(); !w) return unexpected_error(errors::invalid_operation, "master window not found");
//     else if (const auto sw = _subwindow(w); sw) return sw->rendertarget.begin_draw(clear_color, src);
//     else return unexpected_error(errors::invalid_operation, "subwindow not found");
//   }

//   std::expected<void, error_trace> close() {
//     if (const auto w = _window(); !w) return unexpected_error(errors::invalid_operation, "master window not found");
//     else if (const auto sw = _subwindow(w); sw) {
//       if (::DestroyWindow(sw->hwnd)) return {};
//       else return unexpected_win32_error("DestroyWindow failed");
//     } else return unexpected_error(errors::invalid_operation, "subwindow not found");
//   }

//   void set_close_confirmation(bool enable) {
//     if (const auto w = _window(); !w) return;
//     else if (const auto sw = _subwindow(w); sw) sw->close_confirmation = enable;
//   }
// };

// inline std::expected<window::slave, error_trace> subwindow::open(
//   const window::master& master, int2 pos, int2 size, null_terminated<wchar_t> title, window_style style, bool hidden)
//   { const auto mw = master._window(); if (!master) return unexpected_error(errors::invalid_argument, "invalid master
//   window"); style = style == window_style::unknown ? master.style() : style; if (style == window_style::unknown)
//   return unexpected_error(errors::invalid_argument, "invalid window style"); if (RECT r; !::GetWindowRect(mw->hwnd,
//   &r)) return unexpected_win32_error("GetWindowRect failed"); else pos = pos + int2(r.left, r.top);
//   std::unique_ptr<subwindow> sw = std::make_unique<subwindow>(subwindow());
//   if (auto res = sw->create_window(title.data(), style); !res) return unexpected_error(res.error());
//   if (auto res = sw->calculate_padding(); !res) return unexpected_error(res.error());
//   if (auto res = sw->set_sizepos(size, pos); !res) return unexpected_error(res.error());
//   if (!hidden) sw->_show();
//   auto sub_id = mw->slaves.push(std::move(sw));
//   auto p = mw->slaves.get(sub_id);
//   return window::slave(p->sub_id = sub_id, p->main_id = master._id);
// }
// } // namespace yw
