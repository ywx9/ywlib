#pragma once
#include "ywx/bitmap.h"
#include "ywx/text_format.h"

namespace yw::system {

inline class {
  HWND hwnd{};
  bitmap rendertarget;
  comptr<IDXGISwapChain1> swapchain;
  text_layout layout;
  int2 layout_size;
  int2 window_size;

  const DWORD _style = WS_POPUP;
  const DWORD _ex_style = WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE | WS_EX_TRANSPARENT;

public:
  std::expected<void, error_trace> initialize() {
    if (hwnd) return {};
    WNDCLASSW wc{};
    wc.lpfnWndProc = ::DefWindowProcW;
    wc.hInstance = ::GetModuleHandleW(nullptr);
    wc.hCursor = ::LoadCursorW(nullptr, IDC_ARROW);
    wc.lpszClassName = L"ywlib_tooltip_class";
    if (!::RegisterClassW(&wc) && ::GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
      return unexpected_win32_error("RegisterClassW failed");
    hwnd = ::CreateWindowExW(_ex_style, wc.lpszClassName, L"", _style, 0, 0, 100, 100, 0, 0, wc.hInstance, 0);
    if (!hwnd) return unexpected_win32_error("CreateWindowExW failed");
    if (auto res = text_layout::create(L"")) layout = std::move(*res);
    else return unexpected_error(res.error());
    layout.text_alignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    layout.paragraph_alignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    return {};
  }

  std::expected<void, error_trace> show(int2 Pos, uint2 Size) {
    // UIの外側にツールチップを表示する。
    if (auto res = initialize(); !res) return unexpected_error(res.error());
    if (!rendertarget) return unexpected_error(errors::invalid_operation, "tooltip not initialized");
    const auto ui_center = Pos + int2(Size) / 2;
    const auto dc_center = desktop_client_size() / 2;
    if (ui_center.x < dc_center.x) {
      if (ui_center.y < dc_center.y) // top-left
        ::SetWindowPos(
          hwnd, HWND_TOPMOST, Pos.x, Pos.y + Size.y + 4, window_size.x, window_size.y, SWP_NOACTIVATE | SWP_SHOWWINDOW);
      else // bottom-left
        ::SetWindowPos(hwnd, HWND_TOPMOST, Pos.x, Pos.y - window_size.y - 4, window_size.x, window_size.y,
          SWP_NOACTIVATE | SWP_SHOWWINDOW);
    } else {
      if (ui_center.y < dc_center.y) // top-right
        ::SetWindowPos(hwnd, HWND_TOPMOST, Pos.x - window_size.x + Size.x, Pos.y + Size.y + 4, window_size.x,
          window_size.y + 8, SWP_NOACTIVATE | SWP_SHOWWINDOW);
      else // bottom-right
        ::SetWindowPos(hwnd, HWND_TOPMOST, Pos.x - window_size.x + Size.x, Pos.y - window_size.y - 4, window_size.x,
          window_size.y + 8, SWP_NOACTIVATE | SWP_SHOWWINDOW);
    }
    return {};
  }

  std::expected<void, error_trace> show(int2 Pos, uint2 Size, std::wstring_view Text) {
    if (auto res = initialize(); !res) return unexpected_error(res.error());
    if (auto res = text_layout::create(Text, layout, none())) layout = std::move(*res);
    else return unexpected_error(res.error());
    if (auto res = layout.metrics_size()) layout_size = int2(*res);
    else return unexpected_error(res.error());
    window_size = layout_size + int2(8, 4);
    if (swapchain) {
      rendertarget = {};
      auto hr = swapchain->ResizeBuffers(0, window_size.x, window_size.y, DXGI_FORMAT_UNKNOWN, 0);
      if (FAILED(hr)) return unexpected_error(errors::operation_failed, "ResizeBuffers failed", int32_t(hr));
    } else {
      if (auto res = dxgi.initialize(); !res) return unexpected_error(res.error());
      DXGI_SWAP_CHAIN_DESC1 desc(window_size.x, window_size.y, bitmap::dxgiformat, 0, DXGI_SAMPLE_DESC(1, 0), {}, 2);
      desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT, desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
      auto hr = dxgi.factory()->CreateSwapChainForHwnd(d3d.device(), hwnd, &desc, nullptr, nullptr, &swapchain.get());
      if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateSwapChainForHwnd failed", int32_t(hr));
    }
    if (auto res = bitmap::create(swapchain.get())) rendertarget = std::move(*res);
    else return unexpected_error(res.error());
    if (auto d = rendertarget.begin_draw(colors::whitesmoke)) {
      if (auto res = draw_text({4, 2}, layout, colors::black); !res) return unexpected_error(res.error());
    } else return unexpected_error(errors::operation_failed, "begin_draw failed");
    swapchain->Present(0, 0);
    if (auto res = show(Pos, Size); !res) return unexpected_error(res.error());
    return {};
  }

  void hide() {
    if (!hwnd) return;
    ::ShowWindow(hwnd, SW_HIDE);
  }
} tooltip;
} // namespace yw::system
