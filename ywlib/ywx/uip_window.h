#pragma once
#include "ywx/ui_parts.h"

namespace yw::ui::part {

struct window {
  slotid window_id{}; // [in]
  HWND hwnd{};
  int2 pos{};
  int2 size{};
  int4 margin{};
  DWORD style{};    // [in]
  DWORD ex_style{}; // [in]
  std::wstring title;
  bitmap rendertarget{};
  comptr<IDXGISwapChain1> swapchain{};
  bool visible = false, enabled = false;
  bool dirty = true, messy = true, manually_drawn = false;

  int2 area() const noexcept { return size + margin.xy() + margin.zw(); }

  /// `window_id`, `style`, and `ex_style` must be set before calling this function.
  std::expected<void, error_trace> initialize() {
    if (hwnd) return {};
    if (auto res = wclass.initialize(); !res) return unexpected_error(res.error());
    if (auto res = dxgi.initialize(); !res) return unexpected_error(res.error());
    hwnd = ::CreateWindowExW(ex_style, wclass.name().c_str(), L"", style, 0, 0, 10, 10, 0, 0, wclass.hinstance(), 0);
    if (!hwnd) return unexpected_win32_error("CreateWindowExW failed");
    RECT wr, cr;
    if (!::GetClientRect(hwnd, &cr)) return unexpected_win32_error("GetClientRect failed");
    if (!::GetWindowRect(hwnd, &wr)) return unexpected_win32_error("GetWindowRect failed");
    const auto left = (wr.right - wr.left - cr.right) / 2;
    const auto top = wr.bottom - wr.top - cr.bottom - left;
    margin = int4(left, top, left, left);
    auto desc = DXGI_SWAP_CHAIN_DESC1(size.x, size.y, bitmap::dxgiformat, false, DXGI_SAMPLE_DESC(1, 0), {}, 2);
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT, desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    const auto hr = dxgi.factory()->CreateSwapChainForHwnd(d3d.device(), hwnd, &desc, 0, 0, &swapchain.get());
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "Failed to create swapchain", int(hr));
    return {};
  }

  std::expected<void, error_trace> locate(int2 Pos) {
    if (auto res = initialize(); !res) return unexpected_error(res.error());
    if (!::SetWindowPos(hwnd, HWND_TOPMOST, Pos.x, Pos.y, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE))
      return unexpected_win32_error("SetWindowPos failed");
    pos = Pos;
    return {};
  }

  std::expected<void, error_trace> resize(uint2 Size) {
    if (auto res = initialize(); !res) return unexpected_error(res.error());
    if (Size == uint2()) return {};
    bool need_update_rt = !bool(rendertarget);
    if (Size != rendertarget.size()) {
      rendertarget = {};
      const auto hr = swapchain->ResizeBuffers(0, Size.x, Size.y, DXGI_FORMAT_UNKNOWN, 0);
      if (FAILED(hr)) return unexpected_error(errors::operation_failed, "Failed to resize swapchain buffers", int(hr));
      const auto sz = int2(Size) + margin.xy() + margin.zw();
      if (!::SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, sz.x, sz.y, SWP_NOMOVE | SWP_NOACTIVATE))
        return unexpected_win32_error("SetWindowPos failed");
      need_update_rt = true;
    }
    if (need_update_rt) {
      if (auto res = bitmap::create(swapchain.get())) rendertarget = std::move(*res);
      else return unexpected_error(res.error());
    }
    size = Size;
    manually_drawn = false;
    return {};
  }

  void show(bool Show = true) {
    ::ShowWindow(hwnd, Show ? SW_SHOW : SW_HIDE);
    visible = Show;
  }

  class handle {
    friend struct window;
    window* _p = nullptr;
    handle(window& Ref) : _p(&Ref) {}

  public:
    ~handle() noexcept {}

    handle(handle&& Other) noexcept : _p(std::exchange(Other._p, nullptr)) {}
    handle& operator=(handle&& Other) noexcept {
      if (this != &Other) _p = std::exchange(Other._p, nullptr);
      return *this;
    }

    const auto& pos() const { return _p->pos; }
    auto& pos(int2 Pos) {
      if (auto res = _p->locate(Pos); !res) fatal_error(res.error());
      return *this;
    }

    auto& centering() {
      const auto p = (int2(desktop_client_size()) - _p->area()) / 2;
      if (auto res = _p->locate(p); !res) fatal_error(res.error());
      return *this;
    }

    const auto& size() const { return _p->size; }
    auto& size(uint2 Size) {
      if (auto res = _p->resize(Size); !res) fatal_error(res.error());
      _p->messy = true;
      return *this;
    }

    const auto& title() const { return _p->title; }
    auto& title(std::wstring Title) {
      ::SetWindowTextW(_p->hwnd, Title.c_str());
      _p->title = std::move(Title);
      return *this;
    }
  };

  handle handle() { return *this; }
};
} // namespace yw::ui::part
