#pragma once
#include "ywx/key.h"
#include "ywx/event.h"
#include "ywx/window/system.h"

namespace yw::window {

//////////////////////////////////////// MARK: control_slot

class control_slot {
  control_slot(const control_slot&) = delete;
  control_slot& operator=(const control_slot&) = delete;
protected:
  slot* _window() const noexcept;

public:
  control_slotid id{};

  float2 position{};
  float2 size{};
  float2 radius{};
  color background_color = colors::white;
  color border_color = colors::black;
  float border_width = 1.0f;
  bool visible = true;
  bool enabled = true;

  function<void, control::base&, hover_event> on_hover;

  // control_slot() noexcept = default;

  virtual ~control_slot() noexcept = default;
  control_slot(control_slot&&) noexcept = default;
  control_slot& operator=(control_slot&&) noexcept = default;

  control_slot(const slotid& window_id, float2 Pos, float2 Size) : position(Pos), size(Size) {
    id.master = window_id.master;
    id.slave = window_id.slave;
  }

  bool hit_test(float2 pt) const noexcept {
    return pt.x >= position.x && pt.x <= position.x + size.x && pt.y >= position.y && pt.y <= position.y + size.y;
  }

  virtual std::expected<void, error_trace> draw() const {
    if (auto res = fill_round_rectangle(position, size, radius, background_color); !res)
      return unexpected_error(res.error().push());
    if (auto res = draw_round_rectangle(position, size, radius, border_color, border_width); !res)
      return unexpected_error(res.error().push());
    return {};
  }

  virtual std::expected<bool, error_trace> proc(UINT, WPARAM, LPARAM) { return true; }
};

//////////////////////////////////////// MARK: slot

class slot {
  using _window_open_struct = decltype(::yw::window::open);
  using _window_system_struct = decltype(::yw::window::system);
  friend _window_open_struct;
  friend _window_system_struct;

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
  slotid id{};

  HWND hwnd{};
  int4 margin{};
  window::style style{};
  bitmap rendertarget{};
  comptr<IDXGISwapChain1> swapchain{};
  stopwatch timer{};

  slotlist<slot> slaves{};
  slotlist<control_slot> controls{};

  slotlist<control_slot>::id hovered_control{};
  slotlist<control_slot>::id focused_control{};

  uint32_t width{}, height{}; // raed only, updated on WM_SIZE
  bool resizing = false;      // updated on WM_ENTERSIZEMOVE / WM_EXITSIZEMOVE

  color background_color = colors::white;
  bool close_confirmation = false;

  function<void, const slot&, key, bool /*shift*/, bool /*ctrl*/> on_key;

  mutable bool dirty = true;

  slot() noexcept = default;
  slot(slot&&) noexcept = default;
  slot& operator=(slot&&) noexcept = default;

  slot(const slot&) = delete;
  slot& operator=(const slot&) = delete;

  control_slot* hit_test(float2 pt) noexcept {
    for (auto& control_slot : controls)
      if (control_slot.hit_test(pt)) return &control_slot;
    return nullptr;
  }

  const control_slot* hit_test(float2 pt) const noexcept {
    for (const auto& control_slot : controls)
      if (control_slot.hit_test(pt)) return &control_slot;
    return nullptr;
  }

  std::expected<void, error_trace> update() {
    for (auto& slave_slot : slaves) slave_slot.update(); // サブウィンドウの描画は失敗しても扱わない。表示はされる。
    if (!dirty) return {};
    if (auto d = rendertarget.begin_draw())
      for(auto& control : controls)
        if (control.visible) control.draw(); // コントロール起因のエラーは扱わない。表示はされる。
    else return unexpected_error(d.error().push());
    dirty = false;
    return {};
  }
};

//////////////////////////////////////// MARK: control_slot::_window

inline slot* control_slot::_window() const noexcept {
  if (const auto master_slot = system.windows.get(id.master); !master_slot) return nullptr;
  else return id.slave.is_zero() ? master_slot : master_slot->slaves.get(id.slave);
}
} // namespace yw::window
