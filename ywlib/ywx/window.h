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

class window_slot;
class control_slot;

//////////////////////////////////////// MARK: window_class

inline class {
  bool _initialized = false;

public:
  static LRESULT __stdcall proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
  const HINSTANCE hinstance = ::GetModuleHandleW(nullptr);
  const std::wstring_view name = L"ywlib_window";
  slotlist<window_slot> windows{};
  error_trace last_error{};

  std::expected<void, error_trace> initialize() {
    if (_initialized) return {};
    WNDCLASSW wc{};
    wc.lpfnWndProc = proc;
    wc.cbWndExtra = sizeof(slotlist<window_slot>::id) * 2;
    wc.hInstance = hinstance;
    wc.hCursor = ::LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wc.lpszClassName = name.data();
    if (!::RegisterClassW(&wc)) return unexpected_win32_error("RegisterClassW failed");
    else return _initialized = true, std::expected<void, error_trace>();
  }
} window_class;

//////////////////////////////////////// MARK: window_slot

class window_slot {
  std::expected<void, error_trace> create_window(const wchar_t* t, window_style s) {
    switch (this->style = s) {
    case window_style::regular:
    case window_style::fixed:
    case window_style::borderless: break;
    default: return unexpected_error(errors::invalid_argument, "invalid window style");
    }
    hwnd = ::CreateWindowExW(
      WS_EX_ACCEPTFILES, window_class.name.data(), t, DWORD(s), 0, 0, 0, 0, 0, 0, window_class.hinstance, 0);
    if (!hwnd) return unexpected_win32_error("CreateWindowExW failed");
    ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    RECT cr{}, wr{};
    if (!::GetClientRect(hwnd, &cr)) return unexpected_win32_error("GetClientRect failed");
    if (!::GetWindowRect(hwnd, &wr)) return unexpected_win32_error("GetWindowRect failed");
    const auto left = (wr.right - wr.left - cr.right) / 2;
    const auto top = wr.bottom - wr.top - cr.bottom - left;
    margin = int4(left, top, 2 * left, left + top);
    return {};
  }

  std::expected<void, error_trace> set_possize(int2 p, int2 s) {
    if (!::SetWindowPos(hwnd, nullptr, p.x, p.y, s.x + margin.z, s.y + margin.w, SWP_NOZORDER))
      return unexpected_win32_error("SetWindowPos failed");
    else return {};
  }

  std::expected<void, error_trace> set_possize(int2 s) {
    if (HWND desktop; !(desktop = ::GetDesktopWindow())) return unexpected_win32_error("GetDesktopWindow failed");
    else if (RECT r; !::GetClientRect(desktop, &r)) return unexpected_win32_error("GetClientRect failed");
    else return set_possize(int2((r.right - s.x - margin.z) / 2, (r.bottom - s.y - margin.w) / 2), s);
  }

public:
  HWND hwnd;
  window_style style;
  int4 margin;
  bitmap rendertarget;
  comptr<IDXGISwapChain1> swapchain;
  bool is_slave = false;
  bool close_confirmation = false;

  slotlist<window_slot> slaves;
  slotlist<control_slot> controls;

  ~window_slot() noexcept = default;
  window_slot() noexcept = default;
  window_slot(window_slot&&) noexcept = default;
  window_slot& operator=(window_slot&&) noexcept = default;

  window_slot(const window_slot&) = delete;
  window_slot& operator=(const window_slot&) = delete;

  explicit operator bool() const noexcept { return hwnd != nullptr; }

  int2 position() const noexcept {
    if (RECT r; ::GetWindowRect(hwnd, &r)) return int2(r.left, r.top);
    else return {};
  }

  std::expected<void, error_trace> resize_rendertarget(uint2 size) {
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
    else rendertarget = std::move(*res);
    return {};
  }

  std::expected<void, error_trace> initialize(
    int2 pos, int2 size, const wchar_t* title, window_style style, bool hidden, bool nopos, bool slave) {
    if (*this) return unexpected_error(errors::invalid_operation, "window is already initialized");
    this->is_slave = slave;
    if (auto res = window_class.initialize(); !res) return unexpected_error(res.error());
    if (auto res = create_window(title, style); !res) return unexpected_error(res.error());
    if (auto res = nopos ? set_possize(size) : set_possize(pos, size); !res) return unexpected_error(res.error());
    if (auto res = resize_rendertarget(uint2(size.x, size.y)); !res) return unexpected_error(res.error());
    if (!hidden) ::ShowWindow(hwnd, SW_SHOW), ::SetForegroundWindow(hwnd), ::SetActiveWindow(hwnd);
    return {};
  }
};

template<typename Window> concept is_window = requires (Window& w) {
  typename Window::slot_type;
  requires derived_from<typename Window::slot_type, window_slot>;
  { w.hwnd() } -> convertible_to<HWND>;
};

namespace window {

class master;
class slave;

inline std::expected<master, error_trace> open(
  int2 pos, int2 size, null_terminated<wchar_t> title, window_style style = window_style::regular, bool hidden = false);

inline std::expected<master, error_trace> open(
  int2 size, null_terminated<wchar_t> title, window_style style = window_style::regular, bool hidden = false);
}

//////////////////////////////////////// MARK: window::master

class window::master {
  slotlist<window_slot>::id _master_id;
  window_slot* _window() const noexcept { return window_class.windows.get(_master_id); }

public:
  using slot_type = window_slot;

  ~master() noexcept { this->close(); }
  master() noexcept : _master_id() {}
  explicit master(slotlist<window_slot>::id mid) noexcept : _master_id(mid) {}

  master(master&& other) noexcept : _master_id(std::exchange(other._master_id, {})) {}

  master& operator=(master&& other) noexcept {
    if (this == &other) return *this;
    this->close();
    _master_id = std::exchange(other._master_id, {});
    return *this;
  }

  explicit operator bool() const noexcept { return window_class.windows.contains(_master_id); }

  void close() noexcept {
    if (const auto hw = hwnd(); hw) ::DestroyWindow(hw);
  }

  HWND hwnd() const noexcept {
    const auto w = _window();
    return w ? w->hwnd : HWND();
  }

  window_style style() const noexcept {
    const auto w = _window();
    return w ? w->style : window_style::unknown;
  }

  int2 margin() const noexcept {
    const auto w = _window();
    return w ? int2(w->margin.x, w->margin.y) : int2();
  }

  int2 position() const noexcept {
    if (const auto hw = hwnd(); hw)
      if (RECT r; ::GetWindowRect(hw, &r)) return int2(r.left, r.top);
    return {};
  }

  int2 size() const noexcept {
    if (const auto hw = hwnd(); hw)
      if (RECT r; ::GetClientRect(hw, &r)) return int2(r.right - r.left, r.bottom - r.top);
    return {};
  }

  void show(bool b) {
    if (const auto hw = hwnd(); hw)
      if (b) ::ShowWindow(hw, SW_SHOW), ::SetForegroundWindow(hw), ::SetActiveWindow(hw);
      else ::ShowWindow(hw, SW_HIDE);
  }

  void enable(bool b) {
    if (const auto hw = hwnd(); hw) ::EnableWindow(hw, b ? TRUE : FALSE);
  }

  void close_confirmation(bool b) {
    if (const auto w = _window(); w) w->close_confirmation = b;
  }

  void position(int2 Pos) {
    if (const auto hw = hwnd(); hw)
      ::SetWindowPos(hw, nullptr, Pos.x, Pos.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
  }

  void size(int2 Size) {
    const auto w = _window();
    if (const auto hw = w ? w->hwnd : HWND(); hw) {
      const auto width = Size.x + w->margin.z, height = Size.y + w->margin.w;
      ::SetWindowPos(hw, nullptr, 0, 0, width, height, SWP_NOZORDER | SWP_NOMOVE);
    }
  }

  std::expected<drawing, error_trace> begin_draw(const source& src = {}) {
    if (const auto w = _window(); !w) return unexpected_error(errors::not_initialized, "window is not initialized");
    else if (auto& rt = w->rendertarget; rt) return rt.begin_draw(src);
    else return unexpected_error(errors::not_initialized, "window render target is not initialized");
  }

  std::expected<drawing, error_trace> begin_draw(const color& clear_color, const source& src = {}) {
    if (const auto w = _window(); !w) return unexpected_error(errors::not_initialized, "window is not initialized");
    else if (auto& rt = w->rendertarget; rt) return rt.begin_draw(clear_color, src);
    else return unexpected_error(errors::not_initialized, "window render target is not initialized");
  }

  std::expected<slave, error_trace> open_subwindow(int2 offset, int2 size,
    null_terminated<wchar_t> title, window_style style = window_style::unknown, bool hidden = false);
};

inline std::expected<window::master, error_trace> window::open(
  int2 pos, int2 size, null_terminated<wchar_t> title, window_style style, bool hidden) {
  if (auto res = window_class.initialize(); !res) return unexpected_error(res.error());
  std::unique_ptr<window_slot> master_window = std::make_unique<window_slot>();
  auto res = master_window->initialize(pos, size, title.data(), style, hidden, false, false);
  if (!res) return unexpected_error(res.error());
  const auto hwnd = master_window->hwnd;
  const auto id = window_class.windows.push(std::move(master_window));
  ::SetWindowLongPtrW(hwnd, 0, id.index);
  ::SetWindowLongPtrW(hwnd, sizeof(id.index), id.generation);
  return master(id);
}

inline std::expected<window::master, error_trace> window::open(
  int2 size, null_terminated<wchar_t> title, window_style style, bool hidden) {
  if (auto res = window_class.initialize(); !res) return unexpected_error(res.error());
  std::unique_ptr<window_slot> master_window = std::make_unique<window_slot>();
  auto res = master_window->initialize({}, size, title.data(), style, hidden, true, false);
  if (!res) return unexpected_error(res.error());
  const auto hwnd = master_window->hwnd;
  const auto id = window_class.windows.push(std::move(master_window));
  ::SetWindowLongPtrW(hwnd, 0, id.index);
  ::SetWindowLongPtrW(hwnd, sizeof(id.index), id.generation);
  return master(id);
}

//////////////////////////////////////// MARK: window::slave

class window::slave {
  slotlist<window_slot>::id _master_id;
  slotlist<window_slot>::id _slave_id;

  window_slot* _window() const noexcept {
    const auto master = window_class.windows.get(_master_id);
    return master ? master->slaves.get(_slave_id) : nullptr;
  }

public:
  using slot_type = window_slot;

  ~slave() noexcept { this->close(); }
  slave() noexcept : _master_id(), _slave_id() {}

  explicit slave(slotlist<window_slot>::id mid, slotlist<window_slot>::id sid) noexcept
    : _master_id(mid), _slave_id(sid) {}

  slave(slave&& other) noexcept
    : _master_id(std::exchange(other._master_id, {})), _slave_id(std::exchange(other._slave_id, {})) {}

  slave& operator=(slave&& other) noexcept {
    if (this == &other) return *this;
    this->close();
    _master_id = std::exchange(other._master_id, {});
    _slave_id = std::exchange(other._slave_id, {});
    return *this;
  }

  explicit operator bool() const noexcept {
    const auto master = window_class.windows.get(_master_id);
    return master ? master->slaves.contains(_slave_id) : false;
  }

  void close() noexcept {
    if (const auto hw = hwnd(); hw) ::DestroyWindow(hw);
  }

  HWND hwnd() const noexcept {
    const auto w = _window();
    return w ? w->hwnd : HWND();
  }

  window_style style() const noexcept {
    const auto w = _window();
    return w ? w->style : window_style::unknown;
  }

  int2 margin() const noexcept {
    const auto w = _window();
    return w ? int2(w->margin.x, w->margin.y) : int2();
  }

  int2 position() const noexcept {
    if (const auto hw = hwnd(); hw)
      if (RECT r; ::GetWindowRect(hw, &r)) return int2(r.left, r.top);
    return {};
  }

  int2 size() const noexcept {
    if (const auto hw = hwnd(); hw)
      if (RECT r; ::GetClientRect(hw, &r)) return int2(r.right - r.left, r.bottom - r.top);
    return {};
  }

  void show(bool b) {
    if (const auto hw = hwnd(); hw)
      if (b) ::ShowWindow(hw, SW_SHOW), ::SetForegroundWindow(hw), ::SetActiveWindow(hw);
      else ::ShowWindow(hw, SW_HIDE);
  }

  void enable(bool b) {
    if (const auto hw = hwnd(); hw) ::EnableWindow(hw, b ? TRUE : FALSE);
  }

  void close_confirmation(bool b) {
    if (const auto w = _window(); w) w->close_confirmation = b;
  }

  void position(int2 Pos) {
    if (const auto hw = hwnd(); hw)
      ::SetWindowPos(hw, nullptr, Pos.x, Pos.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
  }

  void size(int2 Size) {
    const auto w = _window();
    if (const auto hw = w ? w->hwnd : HWND(); hw) {
      const auto width = Size.x + w->margin.z, height = Size.y + w->margin.w;
      ::SetWindowPos(hw, nullptr, 0, 0, width, height, SWP_NOZORDER | SWP_NOMOVE);
    }
  }

  std::expected<drawing, error_trace> begin_draw(const source& src = {}) {
    if (const auto w = _window(); !w) return unexpected_error(errors::not_initialized, "window is not initialized");
    else if (auto& rt = w->rendertarget; rt) return rt.begin_draw(src);
    else return unexpected_error(errors::not_initialized, "window render target is not initialized");
  }

  std::expected<drawing, error_trace> begin_draw(const color& clear_color, const source& src = {}) {
    if (const auto w = _window(); !w) return unexpected_error(errors::not_initialized, "window is not initialized");
    else if (auto& rt = w->rendertarget; rt) return rt.begin_draw(clear_color, src);
    else return unexpected_error(errors::not_initialized, "window render target is not initialized");
  }
};

inline std::expected<window::slave, error_trace> window::master::open_subwindow(int2 offset, int2 size,
  null_terminated<wchar_t> title, window_style style, bool hidden) {
  auto master_window = window_class.windows.get(_master_id);
  if (!master_window) return unexpected_error(errors::invalid_argument, "master window is not valid");
  switch (style) {
  case window_style::unknown: style = master_window->style; break;
  case window_style::regular:
  case window_style::fixed:
  case window_style::borderless: break;
  default: return unexpected_error(errors::invalid_argument, "invalid window style");
  }
  std::unique_ptr<window_slot> slave_window = std::make_unique<window_slot>();
  auto res = slave_window->initialize(master_window->position() + offset, size, title.data(), style, hidden, false, true);
  if (!res) return unexpected_error(res.error());
  const auto slave_hwnd = slave_window->hwnd;
  const auto slave_id = master_window->slaves.push(std::move(slave_window));
  ::SetWindowLongPtrW(slave_hwnd, 0, _master_id.index);
  ::SetWindowLongPtrW(slave_hwnd, sizeof(_master_id.index), _master_id.generation);
  ::SetWindowLongPtrW(slave_hwnd, sizeof(_master_id), slave_id.index);
  ::SetWindowLongPtrW(slave_hwnd, sizeof(_master_id) + sizeof(slave_id.index), slave_id.generation);
  return slave(_master_id, slave_id);
}

//////////////////////////////////////// MARK: control_slot

class control_slot {
  control_slot(const control_slot&) = delete;
  control_slot& operator=(const control_slot&) = delete;
protected:
  slotlist<window_slot>::id master_id;
  slotlist<window_slot>::id slave_id;
  slotlist<control_slot>::id control_id;
public:

  float2 position{}, size{}, radius{}, padding{};
  color background_color = colors::white, border_color = colors::black;
  float border_width = 1.0f;
  bool visible = true, enabled = true;

  virtual ~control_slot() noexcept = default;
  control_slot() noexcept = default;
  control_slot(control_slot&&) noexcept = default;
  control_slot& operator=(control_slot&&) noexcept = default;

  bool hit_test(float2 pt) const {
    return pt.x >= position.x && pt.x <= position.x + size.x && pt.y >= position.y && pt.y <= position.y + size.y;
  }

  virtual std::expected<void, error_trace> draw() const {
    if (auto res = fill_round_rectangle(position, size, radius, background_color); !res) return unexpected_error(res.error());
    if (auto res = draw_round_rectangle(position, size, radius, border_color, border_width); !res) return unexpected_error(res.error());
    return {};
  }

  virtual std::expected<bool, error_trace> proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) const { return {}; }
};

template<typename Control> concept is_control = requires {
  typename Control::slot_type;
  requires derived_from<typename Control::slot_type, control_slot>;
};

//////////////////////////////////////// MARK: mainloop

inline bool mainloop() {
  for (auto& mb : window_class.windows) {
    if (auto d = mb.rendertarget.begin_draw(); !d) {
      window_class.last_error = d.error().push();
      break;
    } else
      for (auto& cb : mb.controls)
        if (cb.visible) cb.draw();
    mb.swapchain->Present(1, 0);
    for (auto& sb : mb.slaves) {
      if (auto d = sb.rendertarget.begin_draw(); !d) {
      window_class.last_error = d.error().push();
      break;
      }
      for (auto& cb : sb.controls)
        if (cb.visible) cb.draw();
      sb.swapchain->Present(1, 0);
    }
  }
  for (MSG msg; ::PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE);) {
    if (msg.message == WM_QUIT) return false;
    if (window_class.last_error.error.code != errors::success) return false;
    ::TranslateMessage(&msg), ::DispatchMessageW(&msg);
  }
  return true;
}

//////////////////////////////////////// MARK: window procedures

inline LRESULT __stdcall decltype(window_class)::proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  struct ids {
    slotlist<window_slot>::id master_id{};
    slotlist<window_slot>::id slave_id{};
    slotlist<control_slot>::id control_id{};
  };
  auto self = reinterpret_cast<window_slot*>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA));
  if (!self) return ::DefWindowProcW(hwnd, msg, wparam, lparam);
  POINT pt{};
  ::GetCursorPos(&pt);
  ::ScreenToClient(hwnd, &pt);
  for (const auto& cb : self->controls | std::ranges::views::reverse) {
    if (cb.enabled && cb.visible && cb.hit_test(float2(pt.x, pt.y))) {
      if (auto res = cb.proc(hwnd, msg, wparam, lparam); !res) {
        window_class.last_error = res.error().push();
        break;
      } else if (*res) return 0;
    }
  }
  switch (msg) {
  case WM_SIZE: {
    const auto width = LOWORD(lparam), height = HIWORD(lparam);
    if (auto res = self->resize_rendertarget(uint2(width, height)); !res) window_class.last_error = res.error().push();
    return 0;
  }
  case WM_CLOSE:
    if (self->close_confirmation && ::MessageBoxW(hwnd, L"Close window?", L"Confirmation", MB_YESNO) == IDNO) return 0;
    return ::DestroyWindow(hwnd), 0;
  case WM_NCDESTROY:
    if (self->is_slave) {
      slotlist<window_slot>::id master_id, slave_id;
      master_id.index = size_t(::GetWindowLongPtrW(hwnd, 0));
      master_id.generation = size_t(::GetWindowLongPtrW(hwnd, sizeof(master_id.index)));
      slave_id.index = size_t(::GetWindowLongPtrW(hwnd, sizeof(master_id)));
      slave_id.generation = size_t(::GetWindowLongPtrW(hwnd, sizeof(master_id) + sizeof(slave_id.index)));
      if (const auto master = window_class.windows.get(master_id); master) master->slaves.erase(slave_id);
    } else {
      std::vector<HWND> slave_handles;
      slave_handles.reserve(self->slaves.size());
      for (auto& slave : self->slaves) slave_handles.push_back(slave.hwnd);
      for (const auto sh : slave_handles) {
        ::SetWindowLongPtrW(sh, GWLP_USERDATA, 0);
        ::DestroyWindow(sh);
      }
      slotlist<window_slot>::id master_id;
      master_id.index = size_t(::GetWindowLongPtrW(hwnd, 0));
      master_id.generation = size_t(::GetWindowLongPtrW(hwnd, sizeof(master_id.index)));
      window_class.windows.erase(master_id);
      if (window_class.windows.empty()) ::PostQuitMessage(0);
    }
    break;
  }
  return ::DefWindowProcW(hwnd, msg, wparam, lparam);
}
} // namespace yw
