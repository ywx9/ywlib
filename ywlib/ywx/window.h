#pragma once
#include "ywx/command_manager.h"
#include "ywx/ui_control.h"

namespace yw {

class window : public ui::unknown {
public:
  enum class style : uint32_t {
    unknown,
    regular = WS_OVERLAPPEDWINDOW,
    fixed = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
    borderless = WS_POPUP
  };

  class slot : public ui::unknown::slot {
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
    bitmap rendertarget{};
    bitmap layout_bitmap{};
    comptr<IDXGISwapChain1> swapchain{};
    yw::background background = colors::white;
    ui::slotid layout_id{};
    ui::slotid focused_control{};
    ui::slotid hovered_control{};
    ui::slotid captured_control{};
    bool visible = true;
    bool enabled = true;
    bool dirty = true;
    bool messy = true;
    bool resizing = false;
    bool tracking = false;
    bool drawn = false;

    int capture_count{};

    struct {
      float offset = 3.0f;
      float width = 1.0f;
      yw::color color = yw::color(0.0f, 0.0f, 1.0f, 0.5f);
    } focus_ring;

    function<bool> on_close;
    function<void, event::key> on_keydown;
    function<void, event::key> on_keyup;
    yw::command_manager commands;

    virtual ~slot() noexcept override { ::DestroyWindow(hwnd); }

    std::expected<void, error_trace> initialize(ui::slotid Id, std::optional<int2> Pos, uint2 Size, window::style Style) {
      id = Id, style = Style, size = Size;
      if (auto res = _create_window(); !res) return unexpected_error(res.error());
      if (auto res = _calculate_margin(); !res) return unexpected_error(res.error());
      if (!Pos) {
        const auto sz = int2(Size) + margin.xy() + margin.zw();
        pos = (int2(desktop_client_size()) - sz) / 2;
      } else pos = *Pos;
      if (auto res = _set_position(); !res) return unexpected_error(res.error());
      return {};
    }

    std::expected<void, error_trace> draw() {
      if (messy || dirty) {
        if (auto res = draw_layout_bitmap(); !res) return unexpected_error(res.error());
      }
      if (auto d = rendertarget.begin_draw()) {
        if (drawn) drawn = false;
        draw_bitmap({}, float2(size), layout_bitmap);
        if (const auto fcsp = system::slot_address<ui::control>(focused_control)) {
          brush.color(focus_ring.color);
          fcsp->draw_focus_ring(focus_ring.offset, focus_ring.width);
        }
      } else return unexpected_error(d.error());
      return {};
    }

    std::expected<void, error_trace> draw_layout_bitmap() {
      const auto lsp = system::slot_address<ui::control>(layout_id);
      if (!lsp) return unexpected_error(errors::not_initialized, "Window layout not initialized");
      if (messy) {
        lsp->update_size();
        if (size.x < lsp->size.x || size.y < lsp->size.y) {
          const auto sz = int2(yw::max(size.x, lsp->size.x), yw::max(size.y, lsp->size.y)) + margin.xy() + margin.zw();
          ::SetWindowPos(hwnd, nullptr, 0, 0, sz.x, sz.y, SWP_NOZORDER | SWP_NOMOVE);
        }
      } else if (!dirty) return {};
      if (!layout_bitmap || layout_bitmap.size() != size) {
        if (auto res = bitmap::create(size)) layout_bitmap = std::move(*res);
        else return unexpected_error(res.error());
      }
      if (auto d = layout_bitmap.begin_draw(colors::transparent)) {
        if (!drawn) draw_background({}, float2(size), background);
        if (messy) {
          lsp->update_layout({}, float2(size));
          lsp->draw();
        } else if (dirty) lsp->draw();
      } else return unexpected_error(d.error());
      dirty = messy = false;
      return {};
    }

    std::expected<void, error_trace> resize_rendertarget(uint2 sz) {
      if (sz.x <= 0 || sz.y <= 0) return {};
      if (swapchain) {
        rendertarget = {}; // releases the back buffer.
        layout_bitmap = {};
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
      if (auto res = bitmap::create(sz)) layout_bitmap = std::move(*res);
      else return unexpected_error(res.error());
      size = sz;
      messy = true;
      return {};
    }

    virtual bool attach(ui::slotid child_id) override {
      if (const auto lsp = system::uis.get(layout_id)) return lsp->attach(child_id);
      return true;
    }

    virtual void detach(ui::slotid child_id) override {
      if (const auto lsp = system::uis.get(layout_id)) lsp->detach(child_id);
    }

    short2 cursor_pos() const noexcept {
      RECT r;
      ::GetWindowRect(hwnd, &r);
      return short2(r.left + margin.x, r.top + margin.y);
    }

    void next_tab_stop(bool Forward) {
      if (const auto lsp = system::slot_address<ui::control>(layout_id)) {
        bool found = !focused_control;
        focused_control = lsp->next_tab_stop(focused_control, Forward, found);
      } else focused_control = {};
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
  const ui::slotid& id() const noexcept { return _id; }
  const HWND& hwnd() const { return unsafe_get(&slot::hwnd); }
  const int4& margin() const { return unsafe_get(&slot::margin); }

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
  template<stringable S> std::expected<void, error_trace> title(S&& Title) {
    if (auto wsp = system::slot_address<window>(_id)) {
      wsp->title = unicode<wchar_t>(static_cast<S&&>(Title));
      ::SetWindowTextW(wsp->hwnd, wsp->title.c_str());
      return {};
    } else return unexpected_error(errors::operation_failed, "Failed to access window slot.");
  }

  const auto& background() const { return unsafe_get(&slot::background); }
  std::expected<void, error_trace> background(yw::background bg) {
    if (auto wsp = system::slot_address<window>(_id)) {
      wsp->background = std::move(bg);
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
      wsp->enabled = Enabled;
      ::EnableWindow(wsp->hwnd, Enabled);
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

  const auto& on_keydown() const { return unsafe_get(&slot::on_keydown); }
  std::expected<void, error_trace> on_keydown(function<void, event::key> OnKeyDown) const {
    if (auto wsp = system::slot_address<window>(_id)) {
      wsp->on_keydown = std::move(OnKeyDown);
      return {};
    } else return unexpected_error(errors::operation_failed, "Failed to access window slot.");
  }

  const auto& on_keyup() const { return unsafe_get(&slot::on_keyup); }
  std::expected<void, error_trace> on_keyup(function<void, event::key> OnKeyUp) const {
    if (auto wsp = system::slot_address<window>(_id)) {
      wsp->on_keyup = std::move(OnKeyUp);
      return {};
    } else return unexpected_error(errors::operation_failed, "Failed to access window slot.");
  }

  const auto& commands() const { return unsafe_get(&slot::commands); }
  auto& commands() { return unsafe_get(&slot::commands); }

  std::expected<drawing, error_trace> begin_draw() {
    if (const auto wsp = system::slot_address<window>(_id)) {
      wsp->dirty = true;
      wsp->drawn = true;
      if (auto d = wsp->rendertarget.begin_draw()) {
        draw_background({}, float2(wsp->size), wsp->background);
        return std::move(d);
      } else return unexpected_error(d.error());
    } else return unexpected_error(errors::invalid_operation, "window slot not found");
  }

  virtual void destroy() noexcept {
    if (const auto wsp = system::slot_address<window>(_id)) ::DestroyWindow(wsp->hwnd);
  }

  void screenshot(const std::filesystem::path& PngPath, bool WriteUI = false) {
    if (const auto wsp = system::slot_address<window>(_id)) {
      if (!WriteUI) {
        wsp->rendertarget.save_as_png(PngPath);
        return;
      }

      if ((wsp->dirty || wsp->messy) && wsp->layout_id)
        if (auto res = wsp->draw_layout_bitmap(); !res) return;

      auto rt_copy_res = bitmap::create(wsp->rendertarget);
      if (!rt_copy_res) return;
      auto rt_copy = std::move(*rt_copy_res);

      if (auto res = bitmap::create(wsp->size)) {
        auto composed = std::move(*res);
        if (auto d = composed.begin_draw(color(0.0f, 0.0f, 0.0f, 0.0f))) {
          draw_bitmap({}, float2(wsp->size), rt_copy);
          draw_bitmap({}, float2(wsp->size), wsp->layout_bitmap);
        } else return;
        composed.save_as_png(PngPath);
      }
    }
  }
};

inline void ui::control::slot::make_dirty() noexcept {
  if (const auto wsp = system::slot_address<window>(window_id)) wsp->dirty = true;
}

inline void ui::control::slot::make_messy() noexcept {
  if (const auto wsp = system::slot_address<window>(window_id)) wsp->messy = true;
}

inline void ui::control::slot::hover_event(event::hover Event) {
  if (enabled && on_hover) on_hover(Event);
  if (tooltip.empty()) return;
  if (Event.enter()) {
    if (const auto w = system::slot_address<window>(window_id))
      system::tooltip.show(pos + w->pos + w->margin.xy(), size, tooltip);
  } else if (Event.leave()) system::tooltip.hide();
}
} // namespace yw::ui
