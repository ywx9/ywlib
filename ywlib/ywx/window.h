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

class window_body;
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
  slotlist<window_body> windows{};
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

//////////////////////////////////////// MARK: window_body

class window_body {
public:
  class master;
  class slave;

  slotlist<window_body>::id master_id{};
  slotlist<window_body>::id slave_id{};

  HWND hwnd{};
  window_style style{};
  int4 margin{};
  bitmap rendertarget{};
  comptr<IDXGISwapChain1> swapchain{};

  bool close_confirmation{};

  slotlist<window_body> slaves{};
  slotlist<control_body> controls{};

  ~window_body() = default;
  window_body() = default;
  window_body(window_body&&) = default;
  window_body& operator=(window_body&&) = default;

  window_body(const window_body&) = delete;
  window_body& operator=(const window_body&) = delete;

  bool is_master() const noexcept { return slave_id == slotlist<window_body>::id{}; }

  std::expected<void, error_trace> create(const wchar_t* t, window_style s) {
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

  std::expected<void, error_trace> set_sizepos(int2 s, int2 p) {
    if (::SetWindowPos(hwnd, nullptr, p.x, p.y, s.x + margin.z, s.y + margin.w, SWP_NOZORDER)) return {};
    else return unexpected_win32_error("SetWindowPos failed");
  }

  std::expected<void, error_trace> set_sizepos(int2 s) {
    if (HWND desktop; !(desktop = ::GetDesktopWindow())) return unexpected_win32_error("GetDesktopWindow failed");
    else if (RECT r; !::GetClientRect(desktop, &r)) return unexpected_win32_error("GetClientRect failed");
    else return set_sizepos(s, int2((r.right - s.x - margin.z) / 2, (r.bottom - s.y - margin.w) / 2));
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
    else return rendertarget = std::move(*res), std::expected<void, error_trace>();
  }
};

//////////////////////////////////////// MARK: window

class window {
  friend class subwindow;

protected:
  slotlist<window_body>::id _master_id;
  window(slotlist<window_body>::id mid) : _master_id(mid) {}
  window_body* _get_body() const noexcept { return window_class.windows.get(_master_id); }

public:
  window(window&& other) noexcept : _master_id(std::exchange(other._master_id, {})) {}
  window& operator=(window&& other) noexcept {
    if (this != &other) _master_id = std::exchange(other._master_id, {});
    return *this;
  }
  explicit operator bool() const noexcept { return window_class.windows.contains(_master_id); }

  HWND hwnd() const noexcept {
    const auto body = _get_body();
    return body ? body->hwnd : HWND();
  }

  window_style style() const noexcept {
    const auto body = _get_body();
    return body ? body->style : window_style::unknown;
  }

  int2 margin() const noexcept {
    const auto body = _get_body();
    return body ? int2(body->margin.x, body->margin.y) : int2();
  }

  int2 size() const noexcept {
    if (const auto body = _get_body(); body && body->hwnd)
      if (RECT r; ::GetClientRect(body->hwnd, &r)) return int2(r.right - r.left, r.bottom - r.top);
    return {};
  }

  std::expected<void, error_trace> size(int2 Size) {
    if (const auto body = _get_body(); body && body->hwnd) {
      if (::SetWindowPos(body->hwnd, nullptr, 0, 0, Size.x, Size.y, SWP_NOZORDER | SWP_NOMOVE)) return {};
      else return unexpected_win32_error("SetWindowPos failed");
    } else return unexpected_error(errors::not_initialized, "window is not initialized");
  }

  int2 position() const noexcept {
    if (const auto body = _get_body(); body && body->hwnd)
      if (RECT r; ::GetWindowRect(body->hwnd, &r)) return int2(r.left, r.top);
    return {};
  }

  std::expected<void, error_trace> position(int2 Pos) {
    if (const auto body = _get_body(); body && body->hwnd) {
      if (::SetWindowPos(body->hwnd, nullptr, Pos.x, Pos.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE)) return {};
      else return unexpected_win32_error("SetWindowPos failed");
    } else return unexpected_error(errors::not_initialized, "window is not initialized");
  }

  void show() const noexcept {
    const auto body = _get_body();
    const auto hwnd = body ? body->hwnd : HWND();
    if (hwnd) ::ShowWindow(hwnd, SW_SHOW), ::SetForegroundWindow(hwnd), ::SetActiveWindow(hwnd);
  }

  void hide() const noexcept {
    if (const auto body = _get_body(); body && body->hwnd) ::ShowWindow(body->hwnd, SW_HIDE);
  }

  void enable() const noexcept {
    if (const auto body = _get_body(); body && body->hwnd) ::EnableWindow(body->hwnd, TRUE);
  }

  void disable() const noexcept {
    if (const auto body = _get_body(); body && body->hwnd) ::EnableWindow(body->hwnd, FALSE);
  }

  void close() const noexcept {
    if (const auto body = _get_body(); body && body->hwnd) ::DestroyWindow(body->hwnd);
  }

  bool close_confirmation() const noexcept {
    const auto body = _get_body();
    return body ? body->close_confirmation : false;
  }

  void close_confirmation(bool v) noexcept {
    const auto body = _get_body();
    if (body) body->close_confirmation = v;
  }

  std::expected<drawing, error_trace> begin_draw(const source& src = {}) {
    const auto body = _get_body();
    if (!body) return unexpected_error(errors::not_initialized, "window is not initialized");
    if (auto& rendertarget = body->rendertarget; rendertarget) return rendertarget.begin_draw(src);
    else return unexpected_error(errors::not_initialized, "rendertarget is not initialized");
  }

  std::expected<drawing, error_trace> begin_draw(const color& clear_color, const source& src = {}) {
    const auto body = _get_body();
    if (!body) return unexpected_error(errors::not_initialized, "window is not initialized");
    if (auto& rendertarget = body->rendertarget; rendertarget) return rendertarget.begin_draw(clear_color, src);
    else return unexpected_error(errors::not_initialized, "rendertarget is not initialized");
  }

  static std::expected<window, error_trace> open(
    int2 pos, int2 size, null_terminated<wchar_t> title, window_style style = window_style::regular, bool hidden) {
    if (auto res = window_class.initialize(); !res) return unexpected_error(res.error());
    std::unique_ptr<window_body> body = std::make_unique<window_body>();
    if (auto res = body->create(title.data(), style); !res) return unexpected_error(res.error());
    if (auto res = body->set_sizepos(size, pos); !res) return unexpected_error(res.error());
    if (!hidden) ::ShowWindow(body->hwnd, SW_SHOW), ::SetForegroundWindow(body->hwnd), ::SetActiveWindow(body->hwnd);
    auto id = window_class.windows.push(std::move(body));
    if (auto master = window_class.windows.get(id); master) master->master_id = id;
    else return unexpected_error(errors::operation_failed, "failed to get window body after creation");
    return window(id);
  }

  static std::expected<window, error_trace> open(
    int2 size, null_terminated<wchar_t> title, window_style style = window_style::regular, bool hidden) {
    if (auto res = window_class.initialize(); !res) return unexpected_error(res.error());
    std::unique_ptr<window_body> body = std::make_unique<window_body>();
    if (auto res = body->create(title.data(), style); !res) return unexpected_error(res.error());
    if (auto res = body->set_sizepos(size); !res) return unexpected_error(res.error());
    if (!hidden) ::ShowWindow(body->hwnd, SW_SHOW), ::SetForegroundWindow(body->hwnd), ::SetActiveWindow(body->hwnd);
    auto id = window_class.windows.push(std::move(body));
    if (auto master = window_class.windows.get(id); master) master->master_id = id;
    else return unexpected_error(errors::operation_failed, "failed to get window body after creation");
    return window(id);
  }
};

//////////////////////////////////////// MARK: subwindow

class subwindow : public window {
  slotlist<window_body>::id _slave_id;
  subwindow(slotlist<window_body>::id mid, slotlist<window_body>::id sid) noexcept : window(mid), _slave_id(sid) {}
  window_body* _get_slave_body() const noexcept {
    if (const auto master = _get_body(); !master) return nullptr;
    else return master->slaves.get(_slave_id);
  }
  using window::open;

public:
  subwindow(subwindow&& other) noexcept
    : window(std::exchange(other._master_id, {})), _slave_id(std::exchange(other._slave_id, {})) {}

  subwindow& operator=(subwindow&& other) noexcept {
    if (this == &other) return *this;
    _master_id = std::exchange(other._master_id, {});
    _slave_id = std::exchange(other._slave_id, {});
    return *this;
  }

  explicit operator bool() const noexcept {
    const auto master = _get_body();
    return master && master->slaves.contains(_slave_id);
  }

  int2 position() const noexcept {
    if (const auto master = _get_body(); !master) return {};
    else if (const auto master_hwnd = master->hwnd; !master_hwnd) return {};
    else if (const auto slave = _get_slave_body(); !slave || !slave->hwnd) return {};
    else if (const auto slave_hwnd = slave->hwnd; !slave_hwnd) return {};
    else if (RECT mr; !::GetWindowRect(master_hwnd, &mr)) return {};
    else if (RECT sr; !::GetWindowRect(slave_hwnd, &sr)) return {};
    else return int2(sr.left - mr.left, sr.top - mr.top);
  }

  void position(int2 pos) noexcept {
    if (const auto master = _get_body(); !master) return;
    else if (const auto master_hwnd = master->hwnd; !master_hwnd) return;
    else if (const auto slave = _get_slave_body(); !slave || !slave->hwnd) return;
    else if (const auto slave_hwnd = slave->hwnd; !slave_hwnd) return;
    else if (RECT mr; !::GetWindowRect(master_hwnd, &mr)) return;
    else ::SetWindowPos(slave_hwnd, nullptr, mr.left + pos.x, mr.top + pos.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
  }

  static std::expected<subwindow, error_trace> open(window& master, int2 pos, int2 size, null_terminated<wchar_t> title,
    window_style style = window_style::regular, bool hidden) {
    if (!master) return unexpected_error(errors::invalid_argument, "master window is not valid");
    std::unique_ptr<window_body> slave_body = std::make_unique<window_body>();
    if (auto res = slave_body->create(title.data(), style); !res) return unexpected_error(res.error());
    if (auto res = slave_body->set_sizepos(size, pos + master.position()); !res) return unexpected_error(res.error());
    if (!hidden) ::ShowWindow(slave_body->hwnd, SW_SHOW);
    if (const auto master_body = master._get_body(); master_body) {
      auto sid = master_body->slaves.push(std::move(slave_body));
      if (auto slave = master_body->slaves.get(sid); slave) slave->master_id = master._master_id, slave->slave_id = sid;
      else return unexpected_error(errors::operation_failed, "failed to get slave window body after creation");
      return subwindow(master._master_id, sid);
    } else return unexpected_error(errors::invalid_argument, "master window body is not valid");
  }
};

//////////////////////////////////////// MARK: control_body

class control_body {
public:
  slotlist<window_body>::id master_id{};
  slotlist<window_body>::id slave_id{};
  slotlist<control_body>::id control_id{};

  float2 position{}, size{}, radius{};
  color background{}, border{};
  float border_width{};
  bool visible{}, enabled{};

  bool hit_test(float2 point) const {
    return point.x >= position.x && point.x <= position.x + size.x && point.y >= position.y &&
           point.y <= position.y + size.y;
  }

  virtual void draw() const {
    fill_round_rectangle(position, size, radius, background);
    draw_round_rectangle(position, size, radius, border, border_width);
  }
};

//////////////////////////////////////// MARK: control

class control {
  slotlist<window_body>::id _master_id;
  slotlist<window_body>::id _slave_id;
  slotlist<control_body>::id _control_id;
  control(slotlist<window_body>::id mid, slotlist<window_body>::id sid, slotlist<control_body>::id cid) noexcept
    : _master_id(mid), _slave_id(sid), _control_id(cid) {}
  control_body* _get_body() const noexcept {
    const auto master = window_class.windows.get(_master_id);
    if (_slave_id != slotlist<window_body>::id{}) {
      const auto slave = master ? master->slaves.get(_slave_id) : nullptr;
      return slave ? slave->controls.get(_control_id) : nullptr;
    } else return master ? master->controls.get(_control_id) : nullptr;
  }

public:
  control(control&& other) noexcept
    : _master_id(std::exchange(other._master_id, {})), _slave_id(std::exchange(other._slave_id, {})),
      _control_id(std::exchange(other._control_id, {})) {}

  control& operator=(control&& other) noexcept {
    if (this == &other) return *this;
    _master_id = std::exchange(other._master_id, {});
    _slave_id = std::exchange(other._slave_id, {});
    _control_id = std::exchange(other._control_id, {});
    return *this;
  }

  explicit operator bool() const noexcept {
    const auto master = window_class.windows.get(_master_id);
    if (_slave_id != slotlist<window_body>::id{}) {
      const auto slave = master ? master->slaves.get(_slave_id) : nullptr;
      return slave && slave->controls.contains(_control_id);
    } else return master && master->controls.contains(_control_id);
  }

  float2 position() const noexcept {
    return [body = _get_body()] { return body ? body->position : float2(); }();
  }

  void position(float2 pos) noexcept {
    if (const auto body = _get_body(); body) body->position = pos;
  }

  float2 size() const noexcept {
    return [body = _get_body()] { return body ? body->size : float2(); }();
  }

  void size(float2 s) noexcept {
    if (const auto body = _get_body(); body) body->size = s;
  }

  float2 radius() const noexcept {
    return [body = _get_body()] { return body ? body->radius : float2(); }();
  }

  void radius(float2 r) noexcept {
    if (const auto body = _get_body(); body) body->radius = r;
  }

  color background() const noexcept {
    return [body = _get_body()] { return body ? body->background : color(); }();
  }

  void background(const color& c) noexcept {
    if (const auto body = _get_body(); body) body->background = c;
  }

  color border() const noexcept {
    return [body = _get_body()] { return body ? body->border : color(); }();
  }

  void border(const color& c) noexcept {
    if (const auto body = _get_body(); body) body->border = c;
  }

  float border_width() const noexcept {
    return [body = _get_body()] { return body ? body->border_width : 0.0f; }();
  }

  void border_width(float w) noexcept {
    if (const auto body = _get_body(); body) body->border_width = w;
  }

  bool visible() const noexcept {
    return [body = _get_body()] { return body ? body->visible : false; }();
  }

  void visible(bool v) noexcept {
    if (const auto body = _get_body(); body) body->visible = v;
  }

  bool enabled() const noexcept {
    return [body = _get_body()] { return body ? body->enabled : false; }();
  }

  void enabled(bool v) noexcept {
    if (const auto body = _get_body(); body) body->enabled = v;
  }

  bool hit_test(float2 point) const noexcept {
    return [body = _get_body(), point]() { return body ? body->hit_test(point) : false; }();
  }
};

//////////////////////////////////////// MARK: mainloop

inline bool mainloop() {
  constexpr auto present = [&](window_body& body) -> bool {
    auto d = body.rendertarget.begin_draw();
    if (!d) return window_class.last_error = d.error().push(), false;

  };
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
      else {
        for (auto& cb : sb.controls)
          if (cb.visible) cb.draw();
      }
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
} // namespace yw
