#pragma once
#include "ywx/control.h"

namespace yw {

class window final {
public:
  class slot;
  using slotid = slotset<slot>::slotid;

  enum class style : uint32_t {
    unknown,
    regular = WS_OVERLAPPEDWINDOW,
    fixed = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
    borderless = WS_POPUP
  };

  class slot {
    std::expected<void, error_trace> _create_window() {
      if (auto res = wclass.initialize(); !res) return unexpected_error(res.error());
      if (auto res = d2d.initialize(); !res) return unexpected_error(res.error());
      switch (style) {
      case style::regular:
      case style::fixed:
      case style::borderless: break;
      default: return unexpected_error(errors::invalid_argument, "Invalid window style.");
      }
      hwnd = CreateWindowExW(
        WS_EX_ACCEPTFILES, wclass.name().c_str(), title.c_str(), DWORD(style), 0, 0, 0, 0, nullptr, nullptr,
        wclass.hinstance(), nullptr);
      if (!hwnd) return unexpected_win32_error("CreateWindowExW failed");
      ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, std::bit_cast<LONG_PTR>(id));
      return {};
    }

    std::expected<void, error_trace> _calculate_margin() {
      RECT cr{}, wr{};
      if (!::GetClientRect(hwnd, &cr)) return unexpected_win32_error("GetClientRect failed");
      if (!::GetWindowRect(hwnd, &wr)) return unexpected_win32_error("GetWindowRect failed");
      const auto left = (wr.right - wr.left - cr.right) / 2;
      const auto top = wr.bottom - wr.top - cr.bottom - left;
      margin = int4(left, top, left, left);
      return {};
    }

    std::expected<void, error_trace> _set_position() {
      const auto sz = int2(size) + margin.xy() + margin.zw();
      if (!::SetWindowPos(hwnd, nullptr, pos.x, pos.y, sz.x, sz.y, SWP_NOZORDER))
        return unexpected_win32_error("SetWindowPos failed");
      return {};
    }

  public:
    slotid id{};

    HWND hwnd{};
    int2 pos{};
    uint2 size{};
    int4 margin{};
    window::style style{};
    std::wstring title{};
    bitmap rendertarget{};
    comptr<IDXGISwapChain1> swapchain{};
    stopwatch timer{};
    color bg_color = colors::white;
    std::vector<control::slotid> controls;
    control::slotid focused_control{};
    control::slotid hovered_control{};
    control::slotid captured_control{};
    bool visible = true;
    bool enabled = true;
    bool dirty = true;
    bool messy = true;
    bool resizing = false;
    bool manual_draw = false;

    key captured_key{};
    int capture_count{};

    function<bool> on_close;
    function<void, event::key> on_key;

    ~slot() noexcept {
      try {
        ::DestroyWindow(hwnd);
        for (const auto& cid : controls) system::controls.erase(cid);
      } catch (...) {} // noexcept destructor
    }

    std::expected<void, error_trace> initialize(slotid Id, std::optional<int2> Pos, uint2 Size, window::style Style) {
      id = Id, style = Style, size = Size;
      if (auto res = _create_window(); !res) return res;
      if (auto res = _calculate_margin(); !res) return res;
      if (!Pos) {
        const auto sz = int2(Size) + margin.xy() + margin.zw();
        const auto dcs = int2(desktop_client_size());
        pos = (dcs - sz) / 2;
      } else pos = *Pos;
      if (auto res = _set_position(); !res) return res;
      return {};
    }

    tuple<float2, uint2> minimum_size() const noexcept {
      tuple<float2, uint2> result{};
      for (const auto& cid : controls)
        if (const auto csp = system::controls.get(cid)) {
          const auto [ms, uc] = csp->minimum_size();
          result.first.x = yw::max(result.first.x, ms.x);
          result.first.y += yw::max(ms.y, 0.0f);
          result.second.x |= uc.x;
          result.second.y += uc.y;
        }
      return result;
    }

    std::expected<void, error_trace> draw() {
      if (!(rendertarget)) return {};
      if (auto d = manual_draw ? rendertarget.begin_draw() : rendertarget.begin_draw(bg_color)) {
        if (messy) {
          const auto [min_size, unconstrained] = minimum_size();
          const auto extra_size = float2(size) - min_size;
          float offset_y = 0.0f;
          if (unconstrained.y == 0) {
            for (const auto& cid : controls)
              if (const auto csp = system::controls.get(cid); csp && csp->visible) {
                const auto [ms, _] = csp->minimum_size();
                const auto control_size = float2(size.x, ms.y);
                csp->draw({0, offset_y}, control_size);
                offset_y += ms.y;
              }
          } else {
            const auto extra_per_ucc = yw::max(extra_size.y / unconstrained.y, 0.0f);
            for (const auto& cid : controls)
              if (const auto csp = system::controls.get(cid); csp && csp->visible) {
                const auto [ms, ucc] = csp->minimum_size();
                const auto control_size = float2(size.x, ms.y + ucc.y * extra_per_ucc);
                csp->draw({0, offset_y}, control_size);
                offset_y += control_size.y;
              }
          }
          messy = dirty = false;
        } else if (dirty) {
          for (const auto& cid : controls)
            if (const auto csp = system::controls.get(cid); csp && csp->visible)
              csp->draw(csp->last_rect.xy(), csp->last_rect.zw() - csp->last_rect.xy());
          dirty = false;
        } else return {};
      } else return unexpected_error(d.error());
      swapchain->Present(0, 0);
      return {};
    }

    std::expected<void, error_trace> resize_rendertarget(uint2 sz) {
      if (sz.x <= 0 || sz.y <= 0) return {};
      if (swapchain) {
        rendertarget = {}; // releases the back buffer.
        auto hr = swapchain->ResizeBuffers(0, sz.x, sz.y, DXGI_FORMAT_UNKNOWN, 0);
        if (FAILED(hr)) return unexpected_win32_error("ResizeBuffers failed");
      } else {
        if (auto res = dxgi.initialize(); !res) return unexpected_error(res.error());
        auto desc = DXGI_SWAP_CHAIN_DESC1(sz.x, sz.y, bitmap::dxgiformat, false, DXGI_SAMPLE_DESC(1, 0), {}, 2);
        desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT, desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        auto hr = dxgi.factory()->CreateSwapChainForHwnd(d3d.device(), hwnd, &desc, nullptr, nullptr, &swapchain.get());
        if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateSwapChainForHwnd failed", int32_t(hr));
      }
      if (auto res = bitmap::create(swapchain.get())) rendertarget = std::move(*res);
      else return unexpected_error(res.error());
      messy = true;
      return {};
    }
  };

private:
  slotid _id;
  window(slotid Id) : _id(Id) {}

  slot* _slot_address() const noexcept;

  template<typename Mp> auto& unsafe_get(Mp mp) const {
    if (const auto s = _slot_address()) return s->*mp;
    else throw std::runtime_error("invalid member access");
  }

public:
  ~window() noexcept { destroy(); }
  explicit window() noexcept = default;

  window(const window&) = delete;
  window& operator=(const window&) = delete;

  window(window&& other) noexcept : _id(std::exchange(other._id, {})) {}
  window& operator=(window&& other) noexcept {
    if (this == &other) return *this;
    destroy();
    _id = std::exchange(other._id, {});
    return *this;
  }

  /// \param Pos If not specified, the window will be centered on the screen.
  window(std::optional<int2> Pos, uint2 Size, style Style = style::regular, bool Show = true);

  explicit operator bool() const noexcept;
  const slotid& id() const noexcept { return _id; }
  const HWND& hwnd() const { return unsafe_get(&slot::hwnd); }
  const int4& margin() const { return unsafe_get(&slot::margin); }
  float time() const { return unsafe_get(&slot::timer).elapsed(); }

  const int2& pos() const { return unsafe_get(&slot::pos); }
  std::expected<void, error_trace> pos(int2 Pos) const {
    if (auto s = _slot_address()) {
      s->pos = Pos;
      ::SetWindowPos(s->hwnd, nullptr, Pos.x, Pos.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
      return {};
    } else return unexpected_error(errors::operation_failed, "Failed to access window slot.");
  }

  const uint2& size() const { return unsafe_get(&slot::size); }
  std::expected<void, error_trace> size(uint2 Size) const {
    if (auto s = _slot_address()) {
      s->messy = true; // layout can be changed
      s->size = Size;
      const auto sz = int2(Size) + s->margin.xy() + s->margin.zw();
      ::SetWindowPos(s->hwnd, nullptr, 0, 0, sz.x, sz.y, SWP_NOZORDER | SWP_NOMOVE);
      return {};
    } else return unexpected_error(errors::operation_failed, "Failed to access window slot.");
  }

  const std::wstring& title() const { return unsafe_get(&slot::title); }
  template<stringable S> std::expected<void, error_trace> title(S&& Title) const {
    if (auto s = _slot_address()) {
      s->title = unicode<wchar_t>(static_cast<S&&>(Title));
      ::SetWindowTextW(s->hwnd, s->title.c_str());
      return {};
    } else return unexpected_error(errors::operation_failed, "Failed to access window slot.");
  }

  const color& bg_color() const { return unsafe_get(&slot::bg_color); }
  std::expected<void, error_trace> bg_color(color BgColor) const {
    if (auto s = _slot_address()) {
      s->messy = true; // needs redraw
      s->bg_color = BgColor;
      s->dirty = true;
      return {};
    } else return unexpected_error(errors::operation_failed, "Failed to access window slot.");
  }

  const bool& visible() const { return unsafe_get(&slot::visible); }
  std::expected<void, error_trace> visible(bool Visible) const {
    if (auto s = _slot_address()) {
      s->messy = Visible; // needs redraw when becoming visible
      s->visible = Visible;
      ::ShowWindow(s->hwnd, Visible ? SW_SHOW : SW_HIDE);
      return {};
    } else return unexpected_error(errors::operation_failed, "Failed to access window slot.");
  }

  const bool& enabled() const { return unsafe_get(&slot::enabled); }
  std::expected<void, error_trace> enabled(bool Enabled) const {
    if (auto s = _slot_address()) {
      s->enabled = Enabled;
      if (Enabled) s->timer.start();
      else s->timer.stop();
      ::EnableWindow(s->hwnd, Enabled);
      return {};
    } else return unexpected_error(errors::operation_failed, "Failed to access window slot.");
  }

  std::expected<drawing, error_trace> begin_draw() {
    if (const auto wsp = _slot_address()) {
      wsp->messy = true;
      wsp->manual_draw = true;
      if (auto d = wsp->rendertarget.begin_draw(wsp->bg_color)) return std::move(d);
      else return unexpected_error(d.error());
    } else return unexpected_error(errors::invalid_operation, "window slot not found");
  }

  void destroy() noexcept;
};

namespace system {
inline slotset<window::slot> windows{};
inline std::vector<window::slotid> primal_windows{};
inline int2 cursor_pos{};
}

inline window::slot* window::_slot_address() const noexcept { return system::windows.get(_id); }

inline window::operator bool() const noexcept { return system::windows.contains(_id); }

inline void window::destroy() noexcept {
  if (const auto wsp = _slot_address()) {
    system::windows.erase(wsp->id);
    std::erase(system::primal_windows, wsp->id);
  }
}

inline window::window(std::optional<int2> Pos, uint2 Size, style Style, bool Show) {
  _id = system::windows.add(std::make_unique<slot>());
  const auto slot_p = system::windows.get(_id);
  if (!slot_p) throw std::runtime_error("failed to create window slot");
  if (auto res = slot_p->initialize(_id, Pos, Size, Style); !res) {
    system::windows.erase(_id);
    throw unexpected_error(res.error());
  }
  system::primal_windows.push_back(_id);
  if (!Show) return;
  ::ShowWindow(slot_p->hwnd, SW_SHOW);
  ::SetForegroundWindow(slot_p->hwnd);
  ::SetActiveWindow(slot_p->hwnd);
}
} // namespace yw
