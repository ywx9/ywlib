#pragma once
#include "ywx/core.h"

namespace yw::window {

//////////////////////////////////////// MARK: window::style

enum class style : uint32_t {
  unknown,
  regular = WS_OVERLAPPEDWINDOW,                    // flexible-sized window with title bar and border
  fixed = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, // fixed-sized window with title bar and border
  borderless = WS_POPUP,                            // fixed-sized window without title bar and border
};

//////////////////////////////////////// MARK: window::system

class master;
class slave;
class slot;

struct slot_id {
  slotlist<slot>::id master_id{};
  slotlist<slot>::id slave_id{};
  slot* get() const noexcept;
};

struct event;

inline struct {
  friend class slot;

  static LRESULT __stdcall proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
  bool _initialized{};

public:
  HINSTANCE hinstance{};
  std::wstring_view name{};

  slotlist<slot> windows{};
  slotlist<event> events{};
  error_trace last_error{};
  uint64_t frame_count{};
  bool dirty{};

  slot_id focused_window{};

  std::expected<void, error_trace> initialize() {
    if (_initialized) return {};
    hinstance = ::GetModuleHandleW(nullptr);
    name = L"ywlib_window_class";
    WNDCLASSW wc{};
    wc.lpfnWndProc = proc;
    wc.hInstance = hinstance;
    wc.hCursor = ::LoadCursorW(nullptr, IDC_ARROW);
    wc.lpszClassName = name.data();
    if (!::RegisterClassW(&wc)) return unexpected_win32_error("RegisterClassW failed");
    _initialized = true;
    return {};
  }
} system{};
} // namespace yw::window

namespace yw::control {

inline uint64_t counter() {
  static uint64_t _counter = 1;
  return _counter++;
}

//////////////////////////////////////// MARK: control::base

class base {
public:
  class slot {
    slot(const slot&) = delete;
    slot& operator=(const slot&) = delete;

  protected:
    window::slot* _window() const noexcept;

  public:
    window::slot_id window_id{};
    slotlist<slot>::id control_id{};

    float2 position{};
    float2 size{};
    float2 radius{};
    color background_color = colors::white;
    color border_color = colors::black;
    float border_width = 1.0f;
    bool visible = true;
    bool enabled = true;

    uint64_t counter = 0;

    virtual ~slot() noexcept = default;
    slot() noexcept = default;
    slot(slot&&) noexcept = default;
    slot& operator=(slot&&) noexcept = default;

    bool hit_test(float2 pt) const noexcept {
      return pt.x >= position.x && pt.x <= position.x + size.x && pt.y >= position.y && pt.y <= position.y + size.y;
    }

    virtual std::expected<void, error_trace> draw() const {
      if (auto res = fill_round_rectangle(position, size, radius, background_color); !res)
        return unexpected_error(res.error());
      if (auto res = draw_round_rectangle(position, size, radius, border_color, border_width); !res)
        return unexpected_error(res.error());
      return {};
    }

    virtual std::expected<bool, error_trace> proc(const MSG& msg) { return false; }
  };

protected:
  window::slot_id _window_id{};
  slotlist<slot>::id _control_id{};
  uint64_t _counter = 0;

  base(window::slot_id wid, slotlist<slot>::id cid) noexcept
    : _window_id(wid), _control_id(cid), _counter(control::counter()) {}

  window::slot* _window() const noexcept { return _window_id.get(); }
  slot* _control() const noexcept;

  template<typename F> std::invoke_result_t<F, slot*> _get_member(F&& method) const noexcept {
    if (const auto c = _control()) return std::invoke(method, c);
    else return {};
  }

  template<typename T> void _set_member(auto&& method, T&& value) noexcept {
    if (const auto c = _control()) std::invoke(method, c) = static_cast<T&&>(value);
    window::system.dirty = true;
  }

  template<typename Ctrl, derived_from<slot> Slot>
  static std::expected<Ctrl, error_trace> _add(window::slave& w, float2 Pos, float2 Size);

public:
  base() noexcept = default;

  base(base&& c) noexcept
    : _window_id(std::exchange(c._window_id, {})), _control_id(std::exchange(c._control_id, {})),
      _counter(std::exchange(c._counter, 0)) {}

  base& operator=(base&& c) noexcept {
    if (this == &c) return *this;
    _window_id = std::exchange(c._window_id, {});
    _control_id = std::exchange(c._control_id, {});
    _counter = std::exchange(c._counter, 0);
    return *this;
  }

  explicit operator bool() const noexcept;
  uint64_t counter() const noexcept { return _counter; }

  float2 position() const noexcept { return _get_member(&slot::position); }
  float2 size() const noexcept { return _get_member(&slot::size); }
  float2 radius() const noexcept { return _get_member(&slot::radius); }
  color background_color() const noexcept { return _get_member(&slot::background_color); }
  color border_color() const noexcept { return _get_member(&slot::border_color); }
  float border_width() const noexcept { return _get_member(&slot::border_width); }
  bool visible() const noexcept { return _get_member(&slot::visible); }
  bool enabled() const noexcept { return _get_member(&slot::enabled); }

  void position(float2 p) noexcept { _set_member(&slot::position, p); }
  void size(float2 s) noexcept { _set_member(&slot::size, s); }
  void radius(float2 r) noexcept { _set_member(&slot::radius, r); }
  void background_color(const color& c) noexcept { _set_member(&slot::background_color, c); }
  void border_color(const color& c) noexcept { _set_member(&slot::border_color, c); }
  void border_width(float w) noexcept { _set_member(&slot::border_width, w); }
  void visible(bool v) noexcept { _set_member(&slot::visible, v); }
  void enabled(bool e) noexcept { _set_member(&slot::enabled, e); }

  static std::expected<base, error_trace> add(window::slave& w, float2 position, float2 size) {
    if (auto res = _add<base, slot>(w, position, size); res) return std::move(*res);
    else return unexpected_error(res.error());
  }
};
} // namespace yw::control

namespace yw::window {

//////////////////////////////////////// MARK: window::slot

std::expected<master, error_trace> open(int2, int2, const null_terminated<wchar_t>&, style, bool);
std::expected<master, error_trace> open(int2, const null_terminated<wchar_t>&, style, bool);

class slot {
  friend class master;
  friend LRESULT __stdcall decltype(window::system)::proc(HWND, UINT, WPARAM, LPARAM);
  friend std::expected<master, error_trace> open(int2, int2, const null_terminated<wchar_t>&, style, bool);
  friend std::expected<master, error_trace> open(int2, const null_terminated<wchar_t>&, style, bool);

  std::expected<void, error_trace> _create_window(const wchar_t* t, window::style s) {
    switch (this->style = s) {
    case window::style::regular:
    case window::style::fixed:
    case window::style::borderless: break;
    default: return unexpected_error(errors::invalid_argument, "invalid window style");
    }
    hwnd = ::CreateWindowExW(WS_EX_ACCEPTFILES, system.name.data(), t, DWORD(s), 0, 0, 0, 0, 0, 0, system.hinstance, 0);
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

  std::expected<void, error_trace> _set_possize(int2 p, int2 s) {
    if (!::SetWindowPos(hwnd, nullptr, p.x, p.y, s.x + margin.z, s.y + margin.w, SWP_NOZORDER))
      return unexpected_win32_error("SetWindowPos failed");
    else return {};
  }

  std::expected<void, error_trace> _set_possize(int2 s) {
    if (RECT r; !::GetClientRect(::GetDesktopWindow(), &r)) return unexpected_win32_error("GetClientRect failed");
    else return _set_possize(int2((r.right - s.x - margin.z) / 2, (r.bottom - s.y - margin.w) / 2), s);
  }

  std::expected<void, error_trace> _resize_rendertarget(uint2 size) {
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
    if (auto res = bitmap::create(swapchain.get())) rendertarget = std::move(*res);
    else return unexpected_error(res.error());
    return {};
  }

public:
  slot_id id{};

  HWND hwnd{};
  int4 margin{};
  window::style style{};
  bitmap rendertarget{};
  comptr<IDXGISwapChain1> swapchain{};
  stopwatch timer{};

  slotlist<slot> slaves{};
  slotlist<control::base::slot> controls{};
  slotlist<control::base::slot>::id focused_control{};

  bool close_confirmation = false;

  slot() noexcept = default;
  slot(slot&&) noexcept = default;
  slot& operator=(slot&&) noexcept = default;

  slot(const slot&) = delete;
  slot& operator=(const slot&) = delete;

  control::base::slot* hit_test(float2 pt) noexcept {
    for (auto& c : controls)
      if (c.hit_test(pt)) return &c;
    return nullptr;
  }

  const control::base::slot* hit_test(float2 pt) const noexcept {
    for (auto& c : controls)
      if (c.hit_test(pt)) return &c;
    return nullptr;
  }

  std::expected<void, error_trace> draw() const {
    for (auto& c : controls)
  }

  std::expected<bool, error_trace> proc(const MSG& msg);
};

//////////////////////////////////////// MARK: window::slave

class slave {
  friend class master;
  friend class control::base;

protected:
  slot_id _id{};

  slave(slotlist<slot>::id mid, slotlist<slot>::id sid, bool show) noexcept : _id(mid, sid) {
    if (const auto w = _window()) {
      w->id.master_id = mid;
      w->id.slave_id = sid;
      w->timer.start();
      if (show) this->show();
    }
  }

  slot* _window() const noexcept { return _id.get(); }

  HWND _hwnd() const noexcept {
    const auto w = _window();
    return w ? w->hwnd : nullptr;
  }

public:
  slave() noexcept = default;

  ~slave() noexcept {
    if (const auto hwnd = _hwnd()) ::DestroyWindow(hwnd);
  }

  slave(slave&& w) noexcept : _id(std::exchange(w._id, {})) {}

  slave& operator=(slave&& w) noexcept {
    if (this == &w) return *this;
    ::DestroyWindow(_hwnd());
    _id = std::exchange(w._id, {});
    return *this;
  }

  explicit operator bool() const noexcept {
    const auto ms = system.windows.get(_id.master_id);
    return _id.slave_id.is_zero() ? bool(ms) : ms->slaves.contains(_id.slave_id);
  }

  int2 position() const noexcept {
    const auto hwnd = _hwnd();
    RECT r{};
    ::GetWindowRect(hwnd, &r);
    return int2(r.left, r.top);
  }

  int2 size() const noexcept {
    const auto hwnd = _hwnd();
    RECT r;
    ::GetClientRect(hwnd, &r);
    return int2(r.right, r.bottom);
  }

  int2 cursor() const noexcept {
    const auto hwnd = _hwnd();
    int2 pt;
    ::GetCursorPos(reinterpret_cast<LPPOINT>(&pt));
    ::ScreenToClient(hwnd, reinterpret_cast<LPPOINT>(&pt));
    return pt;
  }

  std::wstring title() const noexcept {
    const auto hwnd = _hwnd();
    const auto len = ::GetWindowTextLengthW(hwnd);
    std::wstring t(len, L'\0');
    ::GetWindowTextW(hwnd, t.data(), int(len + 1));
    return t;
  }

  void show(bool b = true) noexcept {
    const auto hwnd = _hwnd();
    if (b) ::ShowWindow(hwnd, SW_SHOW), ::SetForegroundWindow(hwnd), ::SetActiveWindow(hwnd);
    else ::ShowWindow(hwnd, SW_HIDE);
  }

  void enable(bool b = true) noexcept {
    const auto hwnd = _hwnd();
    ::EnableWindow(hwnd, b ? TRUE : FALSE);
  }

  void position(int2 p) noexcept {
    const auto hwnd = _hwnd();
    ::SetWindowPos(hwnd, nullptr, p.x, p.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
  }

  void size(int2 s) noexcept {
    const auto hwnd = _hwnd();
    ::SetWindowPos(hwnd, nullptr, 0, 0, s.x, s.y, SWP_NOZORDER | SWP_NOMOVE);
  }

  void cursor(int2 c) noexcept {
    const auto hwnd = _hwnd();
    POINT pt{c.x, c.y};
    ::ClientToScreen(hwnd, &pt);
    ::SetCursorPos(pt.x, pt.y);
  }

  void title(null_terminated<wchar_t> t) noexcept {
    const auto hwnd = _hwnd();
    ::SetWindowTextW(hwnd, t.data());
  }

  double time() const noexcept {
    if (const auto w = _window()) return w->timer.seconds();
    else return 0.0;
  }

  std::expected<drawing, error_trace> begin_draw(const source& src = {}) {
    if (const auto w = _window()) return w->rendertarget.begin_draw(src);
    else return unexpected_error(errors::not_initialized, "window not initialized");
  }

  std::expected<drawing, error_trace> begin_draw(color clear_color, const source& src = {}) {
    if (const auto w = _window()) return w->rendertarget.begin_draw(clear_color, src);
    else return unexpected_error(errors::not_initialized, "window not initialized");
  }

  void screenshot(const std::filesystem::path& Png) const noexcept {
    if (const auto w = _window()) w->rendertarget.save_as_png(Png);
  }

  void close_confirmation(bool b = true) noexcept {
    if (const auto w = _window()) w->close_confirmation = b;
  }
};

//////////////////////////////////////// MARK: window::master

class master : public slave {
  friend std::expected<master, error_trace> open(int2, int2, const null_terminated<wchar_t>&, style, bool);
  friend std::expected<master, error_trace> open(int2, const null_terminated<wchar_t>&, style, bool);

public:
  using slave::slave;

  std::expected<slave, error_trace> open_subwindow(
    int2 Offset, int2 Size, const null_terminated<wchar_t>& Title, style Style = style::unknown, bool Show = true) {
    const auto ms = system.windows.get(_id.master_id);
    if (!ms) return unexpected_error(errors::invalid_argument, "invalid master window");
    switch (Style) {
    case style::unknown: Style = ms->style; break;
    case style::regular:
    case style::fixed:
    case style::borderless: break;
    default: return unexpected_error(errors::invalid_argument, "invalid window style");
    }
    auto ss = std::make_unique<slot>();
    if (auto res = ss->_create_window(Title.data(), Style); !res) return unexpected_error(res.error());
    if (auto res = ss->_set_possize(this->position() + Offset, Size); !res) return unexpected_error(res.error());
    const auto sid = ms->slaves.push(std::move(ss));
    return slave(_id.master_id, sid, Show);
  }
};

//////////////////////////////////////// MARK: window::event

struct event {
  slotlist<event>::id event_id{};
  slot_id window_id{};
  slotlist<control::base::slot>::id control_id{};
  MSG msg{};
};

//////////////////////////////////////// MARK: window::open

inline std::expected<master, error_trace> open(
  int2 Pos, int2 Size, const null_terminated<wchar_t>& Title, style Style = style::regular, bool Show = true) {
  if (auto res = system.initialize(); !res) return unexpected_error(res.error());
  auto ms = std::make_unique<slot>();
  if (auto res = ms->_create_window(Title.data(), Style); !res) return unexpected_error(res.error());
  if (auto res = ms->_set_possize(Pos, Size); !res) return unexpected_error(res.error());
  const auto mid = system.windows.push(std::move(ms));
  return master(mid, {}, Show);
}

inline std::expected<master, error_trace> open(
  int2 Size, const null_terminated<wchar_t>& Title, style Style = style::regular, bool Show = true) {
  if (auto res = system.initialize(); !res) return unexpected_error(res.error());
  auto ms = std::make_unique<slot>();
  if (auto res = ms->_create_window(Title.data(), Style); !res) return unexpected_error(res.error());
  if (auto res = ms->_set_possize(Size); !res) return unexpected_error(res.error());
  const auto mid = system.windows.push(std::move(ms));
  return master(mid, {}, Show);
}

//////////////////////////////////////// MARK: window procedures

inline LRESULT __stdcall decltype(window::system)::proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  auto self = reinterpret_cast<window::slot*>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA));
  if (!self) return ::DefWindowProcW(hwnd, msg, wparam, lparam);
  switch (msg) {
  case WM_ACTIVATE:
    if (LOWORD(wparam) == WA_INACTIVE) system.focused_window = {};
    break;
  case WM_SETFOCUS:
    system.focused_window = self->id;
    break;
  case WM_SIZE:
    if (auto res = self->_resize_rendertarget(uint2(LOWORD(lparam), HIWORD(lparam))); !res)
      system.last_error = std::move(res.error().push());
    return 0;
  case WM_CLOSE:
    if (self->close_confirmation && ::MessageBoxW(hwnd, L"Close window?", L"Confirmation", MB_YESNO) == IDNO) return
    0; return ::DestroyWindow(hwnd), 0;
  case WM_NCDESTROY:
    if (self->id.slave_id.is_zero()) {
      for (auto& slave : self->slaves) ::DestroyWindow(slave.hwnd);
      system.windows.erase(self->id.master_id);
      if (system.windows.empty()) ::PostQuitMessage(0);
    } else if (const auto mw = system.windows.get(self->id.master_id)) mw->slaves.erase(self->id.slave_id);
    break;
  }
  return ::DefWindowProcW(hwnd, msg, wparam, lparam);
}

//////////////////////////////////////// MARK: window_slot_id::get

inline window::slot* slot_id::get() const noexcept {
  if (const auto ms = system.windows.get(master_id); !ms) return nullptr;
  else return slave_id.is_zero() ? ms : ms->slaves.get(slave_id);
}
} // namespace yw::window

namespace yw {

//////////////////////////////////////// MARK: mainloop

inline bool mainloop() {
  ++window::system.frame_count;
  uint32_t message_count = 0;
  for (MSG msg; ::PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE);) {
    if (msg.message == WM_QUIT || bool(window::system.last_error)) return false;
    ::TranslateMessage(&msg);
    ::DispatchMessageW(&msg);
    if (++message_count > 1000) break;
  }
  for (auto& e : window::system.events) {
    if (window::system.last_error) return false;
    if (auto w = e.window_id.get())
      if (auto c = w->controls.get(e.control_id))
        if (auto res = c->proc(e.msg); !res) window::system.last_error = std::move(res.error().push());
    window::system.events.erase(e.event_id);
  }
  for (auto& mw : window::system.windows) {
    if (window::system.last_error) return false;
    if (auto d = mw.rendertarget.begin_draw()) {
      for (auto& control : mw.controls)
        if (control.visible) control.draw();
    } else window::system.last_error = std::move(d.error().push());
    if (mw.swapchain) mw.swapchain->Present(0, 0);
    for (auto& sw : mw.slaves) {
      if (window::system.last_error) return false;
      if (auto d = sw.rendertarget.begin_draw()) {
        for (auto& control : sw.controls)
          if (control.visible) control.draw();
      } else window::system.last_error = std::move(d.error().push());
      if (sw.swapchain) sw.swapchain->Present(0, 0);
    }
  }
  return !window::system.last_error;
}
}

namespace yw::control {

inline window::slot* base::slot::_window() const noexcept {
  if (const auto ms = window::system.windows.get(window_id.master_id); !ms) return nullptr;
  else return window_id.slave_id.is_zero() ? ms : ms->slaves.get(window_id.slave_id);
}

inline window::slot* base::_window() const noexcept {
  if (const auto ms = window::system.windows.get(_window_id.master_id); !ms) return nullptr;
  else return _window_id.slave_id.is_zero() ? ms : ms->slaves.get(_window_id.slave_id);
}

inline base::slot* base::_control() const noexcept {
  const auto w = _window();
  return w ? w->controls.get(_control_id) : nullptr;
}

template<typename Ctrl, derived_from<base::slot> Slot>
inline std::expected<Ctrl, error_trace> base::_add(window::slave& w, float2 Pos, float2 Size) {
  const auto ws = w._window();
  if (!ws) return unexpected_error(errors::invalid_argument, "invalid window");
  auto cs = std::make_unique<Slot>();
  cs->_window_id.master_id = w._id.master_id;
  cs->_window_id.slave_id = w._id.slave_id;
  cs->position = Pos;
  cs->size = Size;
  const auto cid = ws->controls.push(std::move(cs));
  return Ctrl(w._id.master_id, w._id.slave_id, cid);
}

inline base::operator bool() const noexcept {
  const auto w = _window();
  return w && w->controls.contains(_control_id);
}
} // namespace yw::control
