#pragma once
#include "ywx/bitmap.h"

namespace yw {

struct hover_event;
struct key_event;

namespace window {

class slot;
class slave;
class master;

enum class style : DWORD {
  unknown,
  regular = WS_OVERLAPPEDWINDOW,
  fixed = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
  borderless = WS_POPUP
};

struct slotid {
  yw::slotid master;
  yw::slotid slave;
};

inline struct {
  static std::expected<master, error_trace> operator()(int2, int2, const null_terminated<wchar_t>&, style, bool);
  static std::expected<master, error_trace> operator()(int2, const null_terminated<wchar_t>&, style, bool);
  static std::expected<slave, error_trace> subwindow(master&, int2, int2, const null_terminated<wchar_t>&, style, bool);
} open;
} // namespace window

namespace control {

//////////////////////////////////////// MARK: control::slotid

struct slotid : public window::slotid {
  yw::slotid control;
};

//////////////////////////////////////// MARK: control::base

class base {
  base(const base&) = delete;
  base& operator=(const base&) = delete;

public:
  class slot {
    friend class ::yw::window::slot;
    slot(const slot&) = delete;
    slot& operator=(const slot&) = delete;

  public:
    control::slotid id{};
    float2 position{};
    float2 size{};
    float2 radius{};
    color background_color = colors::white;
    color border_color = colors::black;
    float border_width = 1.0f;
    bool visible = true;
    bool enabled = true;

    function<void, hover_event> on_hover;

    virtual ~slot() noexcept = default;
    slot(slot&&) noexcept = default;
    slot& operator=(slot&&) noexcept = default;

    slot(const yw::slotid& mid, const yw::slotid& sid, float2 p, float2 s) noexcept {
      id.master = mid;
      id.slave = sid;
      position = p;
      size = s;
    }

    virtual bool focusable() const noexcept { return false; }
    virtual bool hit_test(float2 pt) const noexcept {
      return pt.x >= position.x && pt.x <= position.x + size.x && pt.y >= position.y && pt.y <= position.y + size.y;
    }

    virtual std::expected<void, error_trace> draw() const {
      if (auto res = fill_round_rectangle(position, size, radius, background_color); !res)
        return unexpected_error(res.error().push());
      if (auto res = draw_round_rectangle(position, size, radius, border_color, border_width); !res)
        return unexpected_error(res.error().push());
      return {};
    }

    virtual std::expected<bool, error_trace> proc(UINT msg, WPARAM wparam, LPARAM lparam) { return true; }
  };

protected:
  control::slotid _id;

  base(const yw::slotid& mid, const yw::slotid& sid, const yw::slotid& cid) noexcept {
    _id.master = mid;
    _id.slave = sid;
    _id.control = cid;
  }

  window::slot* _window() const noexcept;
  control::base::slot* _control() const noexcept;
  template<typename Mp> const auto* _get(Mp mp) const noexcept {
    if (const auto s = dynamic_cast<class_type<Mp>*>(_control())) return &(s->*mp);
    else return static_cast<decltype(&(s->*mp))>(nullptr);
  }
  template<typename Mp, typename T> void _set(Mp mp, T&& value) const noexcept;
  template<typename Ctrl> static std::expected<Ctrl, error_trace> add(window::slave& w, float2 p, float2 s);

public:
  base() noexcept = default;
  base(base&&) noexcept = default;
  base& operator=(base&&) noexcept = default;

  explicit operator bool() const noexcept;

  const auto* position() const { return _get(&slot::position); }
  const auto* size() const { return _get(&slot::size); }
  const auto* radius() const { return _get(&slot::radius); }
  const auto* background_color() const { return _get(&slot::background_color); }
  const auto* border_color() const { return _get(&slot::border_color); }
  const auto* border_width() const { return _get(&slot::border_width); }
  const auto* visible() const { return _get(&slot::visible); }
  const auto* enabled() const { return _get(&slot::enabled); }
  const auto* on_hover() const { return _get(&slot::on_hover); }

  void position(float2 p) { _set(&slot::position, p); }
  void size(float2 s) { _set(&slot::size, s); }
  void radius(float2 r) { _set(&slot::radius, r); }
  void background_color(color c) { _set(&slot::background_color, c); }
  void border_color(color c) { _set(&slot::border_color, c); }
  void border_width(float w) { _set(&slot::border_width, w); }
  void visible(bool b) { _set(&slot::visible, b); }
  void enabled(bool e) { _set(&slot::enabled, e); }
  void on_hover(function<void, bool> f) { _set(&slot::on_hover, std::move(f)); }

  static std::expected<base, error_trace> add(window::slave& w, float2 Pos, float2 Size) {
    return add<base>(w, Pos, Size);
  }
};
} // namespace control

namespace window {

inline struct {
  friend class slot;
  static LRESULT __stdcall proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
  bool _initialized{};

public:
  HINSTANCE hinstance{};
  std::wstring_view name{};

  slotlist<slot> windows{};
  error_trace last_error{};
  uint64_t frame_count{};
  bool escape_loop = false;
  uint32_t max_frames_per_second = 1000;  // mutable
  uint32_t max_messages_per_frame = 1000; // mutable

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
} system;

//////////////////////////////////////// MARK: window::slot

class slot {
  friend decltype(::yw::window::open);
  friend decltype(::yw::window::system);

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
  window::slotid id{};

  HWND hwnd{};
  int4 margin{};
  window::style style{};
  bitmap rendertarget{};
  comptr<IDXGISwapChain1> swapchain{};
  stopwatch timer{};

  slotlist<window::slot> slaves{};
  slotlist<control::base::slot> controls{};

  yw::slotid hovered_control{};
  yw::slotid focused_control{};

  uint32_t width{}, height{}; // raed only, updated on WM_SIZE
  bool resizing = false;      // updated on WM_ENTERSIZEMOVE / WM_EXITSIZEMOVE

  color background_color = colors::white;
  bool close_confirmation = false;

  function<void, key_event> on_key;

  mutable bool dirty = true;

  slot() noexcept = default;
  slot(slot&&) noexcept = default;
  slot& operator=(slot&&) noexcept = default;

  slot(const slot&) = delete;
  slot& operator=(const slot&) = delete;

  control::base::slot* hit_test(float2 pt) noexcept {
    for (auto& control_slot : controls)
      if (control_slot.hit_test(pt)) return &control_slot;
    return nullptr;
  }

  const control::base::slot* hit_test(float2 pt) const noexcept {
    for (const auto& control_slot : controls)
      if (control_slot.hit_test(pt)) return &control_slot;
    return nullptr;
  }

  std::expected<void, error_trace> update() {
    for (auto& slave_slot : slaves) slave_slot.update();
    if (!dirty) return {};
    if (auto d = rendertarget.begin_draw())
      for (auto& control : controls)
        if (control.visible) control.draw();
        else return unexpected_error(d.error().push());
    dirty = false;
    return {};
  }
};

//////////////////////////////////////// MARK: control_slot::_window

} // namespace window

inline window::slot* control::base::_window() const noexcept {
  if (const auto ms = window::system.windows.get(_id.master); !ms) return nullptr;
  else return _id.slave ? ms->slaves.get(_id.slave) : ms;
}

inline control::base::slot* control::base::_control() const noexcept {
  if (const auto ws = _window(); !ws) return nullptr;
  else return ws->controls.get(_id.control);
}
} // namespace yw
