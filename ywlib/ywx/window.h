#pragma once
#include "ywx/core.h"

namespace yw {

bool mainloop();

namespace control{
class slot;
class base;
}

namespace window {

class slot;
class master;
class slave;

//////////////////////////////////////// MARK: window::system

inline class {
  friend class slot;

  static LRESULT __stdcall proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
  bool _initialized = false;

public:
  const HINSTANCE hinstance = ::GetModuleHandleW(nullptr);
  const std::wstring_view name = L"ywlib_window";

  slotlist<window::slot> windows{};
  error_trace last_error{};
  int2 cursor_pos{};
  uint64_t frame_count = 0;

  struct {
    HWND window = nullptr;
    slotlist<control::slot> control;
  } focused;

  std::expected<void, error_trace> initialize() {
    if (_initialized) return {};
    WNDCLASSW wc{};
    wc.lpfnWndProc = proc;
    wc.hInstance = hinstance;
    wc.hCursor = ::LoadCursorW(nullptr, IDC_ARROW);
    wc.lpszClassName = name.data();
    if (::RegisterClassW(&wc)) {
      _initialized = true;
      return {};
    } else return unexpected_win32_error("RegisterClassW failed");
  }
} system;

//////////////////////////////////////// MARK: window::style

enum class style : uint32_t {
  unknown,
  regular = WS_OVERLAPPEDWINDOW,                    // flexible-sized window with title bar and border
  fixed = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, // fixed-sized window with title bar and border
  borderless = WS_POPUP,                            // fixed-sized window without title bar and border
};

///////////////////////////////////////// MARK: window::slot

class slot {
  friend class master;
  friend class slave;
  friend class control::slot;
  friend bool yw::mainloop();
  friend LRESULT __stdcall decltype(system)::proc(HWND, UINT, WPARAM, LPARAM);

  HWND _hwnd{};
  window::style _style{};
  int4 _margin{};
  bitmap _rendertarget;
  comptr<IDXGISwapChain1> _swapchain;
  stopwatch _timer;
  int2 _cursor_pos{};
  bool _is_slave = false;

  slotlist<slot> _slaves;
  slotlist<control::slot> _controls;

  slot(slot&&) noexcept = default;
  slot& operator=(slot&&) noexcept = default;

  slot(const slot&) = delete;
  slot& operator=(const slot&) = delete;

  std::expected<void, error_trace> _create_window(const wchar_t* t, window::style s) {
    switch (this->_style = s) {
    case window::style::regular:
    case window::style::fixed:
    case window::style::borderless: break;
    default: return unexpected_error(errors::invalid_argument, "invalid window style");
    }
    _hwnd = ::CreateWindowExW(
      WS_EX_ACCEPTFILES, system.name.data(), t, DWORD(s), 0, 0, 0, 0, 0, 0, system.hinstance, 0);
    if (!_hwnd) return unexpected_win32_error("CreateWindowExW failed");
    ::SetWindowLongPtrW(_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    RECT cr{}, wr{};
    if (!::GetClientRect(_hwnd, &cr)) return unexpected_win32_error("GetClientRect failed");
    if (!::GetWindowRect(_hwnd, &wr)) return unexpected_win32_error("GetWindowRect failed");
    const auto left = (wr.right - wr.left - cr.right) / 2;
    const auto top = wr.bottom - wr.top - cr.bottom - left;
    _margin = int4(left, top, 2 * left, left + top);
    return {};
  }

  std::expected<void, error_trace> _set_possize(int2 p, int2 s) {
    if (!::SetWindowPos(_hwnd, nullptr, p.x, p.y, s.x + _margin.z, s.y + _margin.w, SWP_NOZORDER))
      return unexpected_win32_error("SetWindowPos failed");
    else return {};
  }

  std::expected<void, error_trace> _set_possize(int2 s) {
    if (HWND desktop; !(desktop = ::GetDesktopWindow())) return unexpected_win32_error("GetDesktopWindow failed");
    else if (RECT r; !::GetClientRect(desktop, &r)) return unexpected_win32_error("GetClientRect failed");
    else return _set_possize(int2((r.right - s.x - _margin.z) / 2, (r.bottom - s.y - _margin.w) / 2), s);
  }

  std::expected<void, error_trace> _resize_rendertarget(uint2 size) {
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
    else _rendertarget = std::move(*res);
    return {};
  }

public:
  bool close_confirmation = false;

  slot() noexcept = default;
  ~slot() noexcept = default;

  void close() noexcept {
    if (_hwnd) ::DestroyWindow(std::exchange(_hwnd, nullptr));
  }

  explicit operator bool() const noexcept { return _hwnd != nullptr; }
  HWND hwnd() const noexcept { return _hwnd; }
  window::style style() const noexcept { return _style; }
  int4 margin() const noexcept { return _margin; }
  bool is_slave() const noexcept { return _is_slave; }

  int2 position() const noexcept {
    if (!_hwnd) return {};
    if (RECT r; ::GetWindowRect(_hwnd, &r)) return int2(r.left, r.top);
    else return {};
  }

  int2 size() const noexcept {
    if (!_hwnd) return {};
    if (RECT r; ::GetClientRect(_hwnd, &r)) return int2(r.right - r.left, r.bottom - r.top);
    else return {};
  }

  int2 cursor() const noexcept { return _cursor_pos; }

  std::wstring title() const noexcept {
    if (!_hwnd) return {};
    const auto len = ::GetWindowTextLengthW(_hwnd);
    std::wstring t(len, L'\0');
    ::GetWindowTextW(_hwnd, t.data(), int(len + 1));
    return t;
  }

  void show(bool b) {
    if (!_hwnd) return;
    if (b) ::ShowWindow(_hwnd, SW_SHOW), ::SetForegroundWindow(_hwnd), ::SetActiveWindow(_hwnd);
    else ::ShowWindow(_hwnd, SW_HIDE);
  }

  void enable(bool b) {
    if (!_hwnd) return;
    if (b) ::EnableWindow(_hwnd, TRUE), _timer.start();
    else ::EnableWindow(_hwnd, FALSE), _timer.stop();
  }

  void position(int2 Pos) {
    if (_hwnd) ::SetWindowPos(_hwnd, nullptr, Pos.x, Pos.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
  }

  void size(int2 Size) {
    if (!_hwnd) return;
    const auto width = Size.x + _margin.z;
    const auto height = Size.y + _margin.w;
    ::SetWindowPos(_hwnd, nullptr, 0, 0, width, height, SWP_NOZORDER | SWP_NOMOVE);
  }

  void cursor(int2 Pos) {
    if (!_hwnd) return;
    _cursor_pos = Pos;
    ::ClientToScreen(_hwnd, reinterpret_cast<POINT*>(&Pos));
    ::SetCursorPos(Pos.x, Pos.y);
    system.cursor_pos = Pos;
  }

  void title(null_terminated<wchar_t> Title) {
    if (_hwnd) ::SetWindowTextW(_hwnd, Title.data());
  }

  double elapsed_seconds() const noexcept { return _timer.seconds(); }

  std::expected<drawing, error_trace> begin_draw(const source& src = {}) { return _rendertarget.begin_draw(src); }

  std::expected<drawing, error_trace> begin_draw(const color& clear_color, const source& src = {}) {
    return _rendertarget.begin_draw(clear_color, src);
  }

  static std::expected<std::unique_ptr<slot>, error_trace> create(int2 Pos, int2 Size, null_terminated<wchar_t> Title,
    window::style Style = window::style::regular, bool Hidden = false, bool Slave = false) {
    if (auto res = system.initialize(); !res) return unexpected_error(res.error());
    std::unique_ptr<slot> master_slot = std::make_unique<slot>();
    if (auto res = master_slot->_create_window(Title.data(), Style); !res) return unexpected_error(res.error());
    if (auto res = master_slot->_set_possize(Pos, Size); !res) return unexpected_error(res.error());
    master_slot->show(!Hidden);
    master_slot->_is_slave = Slave;
    return std::move(master_slot);
  }

  static std::expected<std::unique_ptr<slot>, error_trace> create(
    int2 Size, null_terminated<wchar_t> Title, window::style Style = window::style::regular, bool Hidden = false, bool Slave = false) {
    if (auto res = system.initialize(); !res) return unexpected_error(res.error());
    std::unique_ptr<slot> master_slot = std::make_unique<slot>();
    if (auto res = master_slot->_create_window(Title.data(), Style); !res) return unexpected_error(res.error());
    if (auto res = master_slot->_set_possize(Size); !res) return unexpected_error(res.error());
    master_slot->show(!Hidden);
    master_slot->_is_slave = Slave;
    return std::move(master_slot);
  }
};

//////////////////////////////////////// MARK: window::master

class slave;

inline std::expected<master, error_trace> open(
  int2 Pos, int2 Size, null_terminated<wchar_t> Title, style Style = style::regular, bool Hidden = false);

inline std::expected<master, error_trace> open(
  int2 Size, null_terminated<wchar_t> Title, style Style = style::regular, bool Hidden = false);

class master {
  friend std::expected<master, error_trace> open(int2, int2, null_terminated<wchar_t>, style, bool);
  friend std::expected<master, error_trace> open(int2, null_terminated<wchar_t>, style, bool);

  slotlist<slot>::id _master_id;
  master(slotlist<slot>::id mid) noexcept : _master_id(mid) {}

public:
  ~master() noexcept {
    if (const auto slot = operator->(); slot) slot->close();
  }
  master(master&& mw) noexcept : _master_id(std::exchange(mw._master_id, {})) {}

  master& operator=(master&& mw) noexcept {
    if (this == &mw) return *this;
    if (const auto self = operator->(); self) self->close();
    _master_id = std::exchange(mw._master_id, {});
    return *this;
  }

  slot* operator->() noexcept { return system.windows.get(_master_id); }

  std::expected<slave, error_trace> open_subwindow(int2 Offset, int2 Size, null_terminated<wchar_t> Title,
    style Style = style::unknown, bool Hidden = false);
};

///////////////////////////////////////// MARK: window::slave

class slave {
  friend class master;

  slotlist<slot>::id _master_id;
  slotlist<slot>::id _slave_id;
  slave(slotlist<slot>::id mid, slotlist<slot>::id sid) noexcept : _master_id(mid), _slave_id(sid) {}

public:
  ~slave() noexcept {
    if (const auto slot = operator->(); slot) slot->close();
  }
  slave(slave&& sw) noexcept
    : _master_id(std::exchange(sw._master_id, {})), _slave_id(std::exchange(sw._slave_id, {})) {}

  slave& operator=(slave&& sw) noexcept {
    if (this == &sw) return *this;
    if (const auto self = operator->(); self) self->close();
    _master_id = std::exchange(sw._master_id, {});
    _slave_id = std::exchange(sw._slave_id, {});
    return *this;
  }

  slot* operator->() noexcept {
    const auto master = system.windows.get(_master_id);
    return master ? master->_slaves.get(_slave_id) : nullptr;
  }

  int2 relative_position() const noexcept {
    if (const auto master = system.windows.get(_master_id); !master) return {};
    else if (const auto slave = master->_slaves.get(_slave_id); !slave) return {};
    else return slave->position() - master->position();
  }

  void relative_position(int2 offset) {
    if (const auto master = system.windows.get(_master_id); !master) return;
    else if (const auto slave = master->_slaves.get(_slave_id); !slave) return;
    else slave->position(master->position() + offset);
  }
};

inline std::expected<slave, error_trace> master::open_subwindow(
  int2 Offset, int2 Size, null_terminated<wchar_t> Title, style Style, bool Hidden) {
  const auto mw = system.windows.get(_master_id);
  if (!mw) return unexpected_error(errors::invalid_argument, "invalid master window");
  switch (Style) {
  case style::unknown: Style = mw->style(); break;
  case style::regular:
  case style::fixed:
  case style::borderless: break;
  default: return unexpected_error(errors::invalid_argument, "invalid window style");
  }
  auto res = slot::create(mw->position() + Offset, Size, Title, Style, Hidden, true);
  if (!res) return unexpected_error(res.error());
  auto sid = mw->_slaves.push(std::move(*res));
  return slave(_master_id, sid);
}

//////////////////////////////////////// MARK: window::open

inline std::expected<master, error_trace> open(
  int2 pos, int2 size, null_terminated<wchar_t> title, style style, bool hidden) {
  if (auto res = system.initialize(); !res) return unexpected_error(res.error());
  auto res = slot::create(pos, size, title, style, hidden);
  if (!res) return unexpected_error(res.error());
  auto mid = system.windows.push(std::move(*res));
  return master(mid);
}

inline std::expected<master, error_trace> open(
  int2 size, null_terminated<wchar_t> title, style style, bool hidden) {
  print(source());
  if (auto res = system.initialize(); !res) return unexpected_error(res.error());
  print(source());
  auto res = slot::create(size, title, style, hidden);
  print(source());
  if (!res) return unexpected_error(res.error());
  print(source());
  auto mid = system.windows.push(std::move(*res));
  print(source());
  return master(mid);
}
} // namespace window

//////////////////////////////////////// MARK: control::slot

class control::slot {
  slot(const slot&) = delete;
  slot& operator=(const slot&) = delete;

protected:
  slotlist<window::slot>::id master_id;
  slotlist<window::slot>::id slave_id;
  slotlist<slot>::id control_id;

  window::slot* _window() const noexcept {
    const auto master = window::system.windows.get(master_id);
    return !master || slave_id == slotlist<window::slot>::id() ? master : master->_slaves.get(slave_id);
  }

public:
  float2 position{}, size{}, radius{}, padding{};
  color background_color = colors::white, border_color = colors::black;
  float border_width = 1.0f;
  bool visible = true, enabled = true;

  virtual ~slot() noexcept = default;
  slot() noexcept = default;
  slot(slot&&) noexcept = default;
  slot& operator=(slot&&) noexcept = default;

  bool hit_test(float2 pt) const {
    return pt.x >= position.x && pt.x <= position.x + size.x && pt.y >= position.y && pt.y <= position.y + size.y;
  }

  virtual std::expected<void, error_trace> draw() const {
    if (auto res = fill_round_rectangle(position, size, radius, background_color); !res)
      return unexpected_error(res.error());
    if (auto res = draw_round_rectangle(position, size, radius, border_color, border_width); !res)
      return unexpected_error(res.error());
    return {};
  }

  virtual std::expected<bool, error_trace> proc(const MSG& msg) { return {}; }
};

//////////////////////////////////////// MARK: mainloop

inline bool mainloop() {
  ++window::system.frame_count;
  for (auto& master_window : window::system.windows) {
    if (auto d = master_window._rendertarget.begin_draw(); !d) {
      window::system.last_error = d.error().push();
      break;
    } else
      for (auto& control : master_window._controls)
        if (control.visible) control.draw();
    master_window._swapchain->Present(1, 0);
    for (auto& slave_window : master_window._slaves) {
      if (auto d = slave_window._rendertarget.begin_draw(); !d) {
        window::system.last_error = d.error().push();
        break;
      }
      for (auto& control : slave_window._controls)
        if (control.visible) control.draw();
      slave_window._swapchain->Present(1, 0);
    }
  }
  for (MSG msg; ::PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE);) {
    if (msg.message == WM_QUIT) return false;
    if (window::system.last_error.error.code != errors::success) return false;
    ::TranslateMessage(&msg);
    ::DispatchMessageW(&msg);
  }
  { // updates cursor position
    static_assert(sizeof(POINT) == sizeof(window::system.cursor_pos));
    ::GetCursorPos((POINT*)&window::system.cursor_pos);
    for (auto& mw : window::system.windows) {
      mw._cursor_pos = window::system.cursor_pos;
      ::ScreenToClient(mw._hwnd, (POINT*)&mw._cursor_pos);
      for (auto& sw : mw._slaves) ::ScreenToClient(sw._hwnd, (POINT*)&sw._cursor_pos);
    }
  }
  return window::system.last_error.error.code == errors::success;
}

//////////////////////////////////////// MARK: window procedures

inline LRESULT __stdcall decltype(window::system)::proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  struct ids {
    slotlist<window::slot>::id master_id{};
    slotlist<window::slot>::id slave_id{};
    slotlist<control::slot>::id control_id{};
  };
  auto self = reinterpret_cast<window::slot*>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA));
  if (!self) return ::DefWindowProcW(hwnd, msg, wparam, lparam);
  switch (msg) {
  case WM_SIZE:
    if (auto res = self->_resize_rendertarget(uint2(LOWORD(lparam), HIWORD(lparam))); !res)
      system.last_error = res.error().push();
    return 0;
  case WM_CLOSE:
    if (self->close_confirmation && ::MessageBoxW(hwnd, L"Close window?", L"Confirmation", MB_YESNO) == IDNO) return 0;
    return ::DestroyWindow(hwnd), 0;
  case WM_NCDESTROY:
    slotlist<window::slot>::id master_id;
    constexpr int offset = sizeof(master_id.index);
    master_id.index = size_t(::GetWindowLongPtrW(hwnd, offset * 0));
    master_id.generation = size_t(::GetWindowLongPtrW(hwnd, offset * 1));
    if (self->_is_slave) {
      slotlist<window::slot>::id slave_id;
      slave_id.index = size_t(::GetWindowLongPtrW(hwnd, offset * 2));
      slave_id.generation = size_t(::GetWindowLongPtrW(hwnd, offset * 3));
      if (const auto master = system.windows.get(master_id); master) master->_slaves.erase(slave_id);
    } else {
      std::vector<HWND> slave_handles;
      slave_handles.reserve(self->_slaves.size());
      for (auto& slave : self->_slaves) slave_handles.push_back(slave.hwnd());
      for (const auto sh : slave_handles) {
        ::SetWindowLongPtrW(sh, GWLP_USERDATA, 0);
        ::DestroyWindow(sh);
      }
      system.windows.erase(master_id);
      if (system.windows.empty()) ::PostQuitMessage(0);
    }
    break;
  }
  return ::DefWindowProcW(hwnd, msg, wparam, lparam);
}

} // namespace yw
