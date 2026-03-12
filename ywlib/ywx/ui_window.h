#pragma once
#include "ywx/bitmap.h"
#include "ywx/key.h"
#include "ywx/ui_vertical.h"

namespace yw::ui {

class window : public vertical {
public:
  enum class style : uint32_t {
    unknown,
    regular = WS_OVERLAPPEDWINDOW,
    fixed = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
    borderless = WS_POPUP
  };

  class slot : public vertical::slot {
  public:
    HWND hwnd{};
    int2 pos{};
    window::style style = window::style::unknown;
    std::wstring title{};
    color bg_color = colors::white;
    bitmap rendertarget{};
    comptr<IDXGISwapChain1> swapchain{};
    stopwatch timer{};
    slotset<ui::control::slot>::slotid focused_ui{};
    slotset<ui::control::slot>::slotid hovered_ui{};
    slotset<ui::control::slot>::slotid captured_ui{};
    key captured_key{};
    int captured_count{};
    bool resizing{};
    bool manually_drawn{};
    mutable bool dirty = true;

    function<bool> on_close;
    function<void, event::key> on_key;

    virtual void make_dirty() noexcept override { dirty = true; }

    std::expected<void, error_trace> _resize_rendertarget(uint2 size) {
      if (swapchain) {
        rendertarget = {};
        if (auto hr = swapchain->ResizeBuffers(0, size.x, size.y, DXGI_FORMAT_UNKNOWN, 0); FAILED(hr))
          return unexpected_error(errors::operation_failed, "ResizeBuffers failed", int32_t(hr));
      } else {
        if (auto res = dxgi.initialize(); !res) return unexpected_error(res.error());
        auto desc = DXGI_SWAP_CHAIN_DESC1(size.x, size.y, bitmap::dxgiformat, false, DXGI_SAMPLE_DESC(1, 0), {}, 2);
        desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT, desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        auto hr = dxgi.factory()->CreateSwapChainForHwnd(d3d.device(), hwnd, &desc, nullptr, nullptr, &swapchain.get());
        if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateSwapChainForHwnd failed", int32_t(hr));
      }
      if (auto res = bitmap::create(swapchain.get())) rendertarget = std::move(*res);
      else return unexpected_error(res.error());
      return {};
    }

    void draw() {
      if (dirty) {
        if (visible)
          if (auto d = manually_drawn ? rendertarget.begin_draw() : rendertarget.begin_draw(bg_color))
            vertical::slot::draw(float2(), {width, height});
        dirty = false;
        manually_drawn = false;
      }
    }

    std::expected<void, error_trace> initialize(window::style Style) {
      if (auto res = wclass.initialize(); !res) return unexpected_error(res.error());
      if (auto res = d2d.initialize(); !res) return unexpected_error(res.error());
      switch (Style) {
      case window::style::regular:
      case window::style::fixed:
      case window::style::borderless: break;
      default: return unexpected_error(errors::invalid_argument, "Invalid window style");
      }
      hwnd = CreateWindowExW(
        ExStyle, wclass.name().data(), title.data(), DWORD(style), 0, 0, 0, 0, nullptr, nullptr, wclass.hinstance(),
        nullptr);
      if (!hwnd) return unexpected_win32_error("CreateWindowExW failed");
      ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, std::bit_cast<LONG_PTR>(id));
      RECT cr{}, wr{};
      if (!::GetClientRect(hwnd, &cr)) return unexpected_win32_error("GetClientRect failed");
      if (!::GetWindowRect(hwnd, &wr)) return unexpected_win32_error("GetWindowRect failed");
      const auto left = (wr.right - wr.left - cr.right) / 2;
      const auto top = wr.bottom - wr.top - cr.bottom - left;
      margin = uint4(left, top, left * 2, left + top);
      const auto total_size = int2(size.x + margin.z, size.y + margin.w);
      if (centering) {
        const auto dcs = desktop_client_size();
        pos = {(dcs.x - total_size.x) / 2, (dcs.y - total_size.y) / 2};
      }
      if (!::SetWindowPos(hwnd, nullptr, pos.x, pos.y, total_size.x, total_size.y, SWP_NOZORDER | SWP_NOACTIVATE))
        return unexpected_win32_error("SetWindowPos failed");
      if (show) ::ShowWindow(hwnd, SW_SHOW), ::SetForegroundWindow(hwnd), ::SetActiveWindow(hwnd);
      timer.start();
      return {};
    }
  };

  window() noexcept = default;
  window(style Style = style::regular) noexcept {

  }

private:

};
}
