#pragma once
#include "ywx/ui_control.h"

namespace yw::ui {

class window : public unknown {
public:
  enum class style : uint32_t {
    unknown,
    regular = WS_OVERLAPPEDWINDOW,
    fixed = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
    borderless = WS_POPUP
  };

  class slot : public unknown::slot {
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
    HWND hwnd{};
    int2 pos{};
    uint2 size{};
    int4 margin{};
    window::style style{};
    std::wstring title{};
    mutable bitmap rendertarget{};
    comptr<IDXGISwapChain1> swapchain{};
    stopwatch timer{};
    color bg_color = colors::white;
    slotid layout_id{};
    slotid focused_control{};
    slotid hovered_control{};
    slotid captured_control{};
    bool visible = true;
    bool enabled = true;
    mutable bool dirty = true;
    mutable bool messy = true;
    bool resizing = false;
    bool tracking = false;
    bool manual_draw = false;

    key captured_key{};
    int capture_count{};

    struct {
      float offset = 2.5f;
      float width = 0.5f;
      yw::color color = colors::blue;
    } focus_ring;

    function<bool> on_close;
    function<void, event::key> on_key;

    virtual ~slot() noexcept override {
      try {
        ::DestroyWindow(hwnd);
      } catch (...) {} // noexcept destructor
    }

    std::expected<void, error_trace> initialize(slotid Id, std::optional<int2> Pos, uint2 Size, window::style Style) {
      id = Id, style = Style, size = Size;
      if (auto res = _create_window(); !res) return unexpected_error(res.error());
      if (auto res = _calculate_margin(); !res) return unexpected_error(res.error());
      if (!Pos) {
        const auto sz = int2(Size) + margin.xy() + margin.zw();
        pos = (int2(desktop_client_size()) - sz) / 2;
      } else pos = *Pos;
      if (auto res = _set_position(); !res) return unexpected_error(res.error());
      timer.restart();
      return {};
    }

    virtual void draw() const override {
      const auto lsp = system::slot_address<ui::control>(layout_id);
      if (!lsp) return;
      const auto [rs, _] = lsp->require_size();
      if (size.x < rs.x || size.y < rs.y) {
        const auto sz = int2(yw::max(size.x, rs.x), yw::max(size.y, rs.y)) + margin.xy() + margin.zw();
        ::SetWindowPos(hwnd, nullptr, 0, 0, sz.x, sz.y, SWP_NOZORDER | SWP_NOMOVE);
      }
      if (auto d = manual_draw ? rendertarget.begin_draw() : rendertarget.begin_draw(bg_color)) {
        if (const auto lsp = system::uis.get(layout_id)) {
          if (messy) lsp->draw({}, float2(size));
          else if (dirty) lsp->draw();
        }
        if (const auto fcsp = system::slot_address<ui::control>(focused_control)) {
          const auto off = float2::fill(focus_ring.offset);
          brush.color(focus_ring.color);
          const auto sz = (fcsp->last_rect.zw() - fcsp->last_rect.xy()) + off * 2;
          const auto pos = fcsp->last_rect.xy() - off;
          const auto radius = fcsp->get_radius() + off;
          draw_round_rectangle(pos, sz, radius, focus_ring.width);
        }
      } else throw unexpected_error(d.error());
      return;
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
        desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT, desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        auto hr = dxgi.factory()->CreateSwapChainForHwnd(d3d.device(), hwnd, &desc, nullptr, nullptr, &swapchain.get());
        if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateSwapChainForHwnd failed", int32_t(hr));
      }
      if (auto res = bitmap::create(swapchain.get())) rendertarget = std::move(*res);
      else return unexpected_error(res.error());
      size = sz;
      messy = true;
      return {};
    }

    virtual bool attach(slotid child_id) override {
      if (const auto lsp = system::uis.get(layout_id)) return lsp->attach(child_id);
      return false;
    }

    virtual void detach(slotid child_id) override {
      if (const auto lsp = system::uis.get(layout_id)) lsp->detach(child_id);
    }

    short2 cursor_pos() const noexcept {
      RECT r;
      ::GetWindowRect(hwnd, &r);
      return short2(r.left + margin.x, r.top + margin.y);
    }
  };

public:
  virtual ~window() noexcept override { destroy(); }
  explicit window() noexcept = default;

  /// \param Pos If not specified, the window will be centered on the screen.
  window(std::optional<int2> Pos, uint2 Size, style Style = style::regular, bool Show = true) {
    _id = system::uis.add(std::make_unique<slot>());
    const auto wsp = system::slot_address<window>(_id);
    if (!wsp) throw std::runtime_error("failed to create window slot");
    if (auto res = wsp->initialize(_id, Pos, Size, Style); !res) {
      system::uis.erase(_id);
      throw unexpected_error(res.error());
    }
    system::primal_windows.push_back(_id);
    if (!Show) return;
    ::ShowWindow(wsp->hwnd, SW_SHOW);
    ::SetForegroundWindow(wsp->hwnd);
    ::SetActiveWindow(wsp->hwnd);
  }

  explicit operator bool() const noexcept;
  const slotid& id() const noexcept { return _id; }
  const HWND& hwnd() const { return unsafe_get(&slot::hwnd); }
  const int4& margin() const { return unsafe_get(&slot::margin); }
  float time() const { return unsafe_get(&slot::timer).elapsed(); }

  const int2& pos() const { return unsafe_get(&slot::pos); }
  std::expected<void, error_trace> pos(int2 Pos) const {
    if (auto wsp = system::slot_address<window>(_id)) {
      wsp->pos = Pos;
      ::SetWindowPos(wsp->hwnd, nullptr, Pos.x, Pos.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
      return {};
    } else return unexpected_error(errors::operation_failed, "Failed to access window slot.");
  }

  const uint2& size() const { return unsafe_get(&slot::size); }
  std::expected<void, error_trace> size(uint2 Size) const {
    if (auto wsp = system::slot_address<window>(_id)) {
      wsp->size = Size;
      const auto sz = int2(Size) + wsp->margin.xy() + wsp->margin.zw();
      ::SetWindowPos(wsp->hwnd, nullptr, 0, 0, sz.x, sz.y, SWP_NOZORDER | SWP_NOMOVE);
      return {};
    } else return unexpected_error(errors::operation_failed, "Failed to access window slot.");
  }

  const std::wstring& title() const { return unsafe_get(&slot::title); }
  template<stringable S> std::expected<void, error_trace> title(S&& Title) const {
    if (auto wsp = system::slot_address<window>(_id)) {
      wsp->title = unicode<wchar_t>(static_cast<S&&>(Title));
      ::SetWindowTextW(wsp->hwnd, wsp->title.c_str());
      return {};
    } else return unexpected_error(errors::operation_failed, "Failed to access window slot.");
  }

  const color& bg_color() const { return unsafe_get(&slot::bg_color); }
  std::expected<void, error_trace> bg_color(color BgColor) const {
    if (auto wsp = system::slot_address<window>(_id)) {
      wsp->bg_color = BgColor;
      wsp->dirty = true;
      return {};
    } else return unexpected_error(errors::operation_failed, "Failed to access window slot.");
  }

  const bool& visible() const { return unsafe_get(&slot::visible); }
  std::expected<void, error_trace> visible(bool Visible) const {
    if (auto wsp = system::slot_address<window>(_id)) {
      if (wsp->visible != Visible) {
        if (Visible) {
          wsp->dirty = true;
          wsp->visible = true;
          ::ShowWindow(wsp->hwnd, SW_SHOW);
        } else {
          wsp->visible = false;
          ::ShowWindow(wsp->hwnd, SW_HIDE);
        }
      }
      return {};
    } else return unexpected_error(errors::operation_failed, "Failed to access window slot.");
  }

  const bool& enabled() const { return unsafe_get(&slot::enabled); }
  std::expected<void, error_trace> enabled(bool Enabled) const {
    if (auto wsp = system::slot_address<window>(_id)) {
      if (Enabled) {
        wsp->enabled = true;
        wsp->timer.start();
        ::EnableWindow(wsp->hwnd, true);
      } else {
        wsp->enabled = false;
        wsp->timer.stop();
        ::EnableWindow(wsp->hwnd, false);
      }
      return {};
    } else return unexpected_error(errors::operation_failed, "Failed to access window slot.");
  }

  const auto& on_close() const { return unsafe_get(&slot::on_close); }
  std::expected<void, error_trace> on_close(function<bool> OnClose) const {
    if (auto wsp = system::slot_address<window>(_id)) {
      wsp->on_close = std::move(OnClose);
      return {};
    } else return unexpected_error(errors::operation_failed, "Failed to access window slot.");
  }

  std::expected<drawing, error_trace> begin_draw() {
    if (const auto wsp = system::slot_address<window>(_id)) {
      wsp->dirty = true;
      wsp->manual_draw = true;
      if (auto d = wsp->rendertarget.begin_draw(wsp->bg_color)) return std::move(d);
      else return unexpected_error(d.error());
    } else return unexpected_error(errors::invalid_operation, "window slot not found");
  }

  virtual void destroy() noexcept override { system::uis.erase(_id); }

  void screenshot(const std::filesystem::path& PngPath) {
    if (const auto wsp = system::slot_address<window>(_id)) wsp->rendertarget.save_as_png(PngPath);
  }
};

inline void control::slot::make_dirty() noexcept {
  if (const auto wsp = system::slot_address<window>(window_id)) wsp->dirty = true;
}

inline void control::slot::make_messy() noexcept {
  if (const auto wsp = system::slot_address<window>(window_id)) wsp->messy = true;
}
}
