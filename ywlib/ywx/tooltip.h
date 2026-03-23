#pragma once
#include "ywx/bitmap.h"
#include "ywx/text_layout.h"

/// \note
/// コントロールに重なるようにツールチップウィンドウを表示すると、
/// ツールチップ上にカーソルが乗ってWM_MOUSELEAVEが発生してしまうケースがある。
/// これによるチラつきを防止するためにも、
/// ツールチップはコントロールの外側に表示することを決定した。

namespace yw::system {

inline class {
  HWND _hwnd{};
  bitmap _rendertarget;
  comptr<IDXGISwapChain1> _swapchain;
  text_layout _layout;
  int2 _layout_size;
  int2 _window_size;
  int2 _window_pos;

  bool _visible = false;

  const DWORD _style = WS_POPUP;
  const DWORD _ex_style = WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE | WS_EX_TRANSPARENT;

public:
  HWND hwnd() const noexcept { return _hwnd; }
  int2 window_size() const noexcept { return _window_size; }
  int2 window_pos() const noexcept { return _window_pos; }
  bool visible() const noexcept { return _visible; }

  std::expected<void, error_trace> initialize() {
    if (_hwnd) return {};
    WNDCLASSW wc{};
    wc.lpfnWndProc = ::DefWindowProcW;
    wc.hInstance = ::GetModuleHandleW(nullptr);
    wc.hCursor = ::LoadCursorW(nullptr, IDC_ARROW);
    wc.lpszClassName = L"ywlib_tooltip_class";
    if (!::RegisterClassW(&wc) && ::GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
      return unexpected_win32_error("RegisterClassW failed");
    _hwnd = ::CreateWindowExW(_ex_style, wc.lpszClassName, L"", _style, 0, 0, 100, 100, 0, 0, wc.hInstance, 0);
    if (!_hwnd) return unexpected_win32_error("CreateWindowExW failed");
    if (auto res = text_layout::create(L"", float2(1e6f, 1e6f))) _layout = std::move(*res);
    else return unexpected_error(res.error());
    _layout.text_alignment(text_alignment::left);
    _layout.paragraph_alignment(paragraph_alignment::top);
    return {};
  }

  std::expected<void, error_trace> show(int2 Pos, uint2 Size) {
    if (auto res = initialize(); !res) return unexpected_error(res.error());
    if (!_rendertarget) return unexpected_error(errors::invalid_operation, "tooltip not initialized");
    const auto dc_center = desktop_client_size() / 2;
    if (Pos.x < dc_center.x) {
      if (Pos.y < dc_center.y) _window_pos = {Pos.x, Pos.y + Size.y + 4}; // top-left
      else _window_pos = {Pos.x, Pos.y - _window_size.y - 4}; // bottom-left
    } else {
      if (Pos.y < dc_center.y) _window_pos = {Pos.x - _window_size.x - 4, Pos.y}; // top-right
      else _window_pos = {Pos.x - _window_size.x - 4, Pos.y + Size.y - _window_size.y}; // bottom-right
    }
    // const auto ui_center = Pos + int2(Size) / 2;
    // if (ui_center.x < dc_center.x) {
    //   if (ui_center.y < dc_center.y) _window_pos = {Pos.x, Pos.y + Size.y + 4}; // top-left
    //   else _window_pos = {Pos.x, Pos.y - _window_size.y - 4};                   // bottom-left
    // } else {
    //   if (ui_center.y < dc_center.y) _window_pos = {Pos.x - _window_size.x - 4, Pos.y}; // top-right
    //   else _window_pos = {Pos.x - _window_size.x - 4, Pos.y + Size.y - _window_size.y}; // bottom-right
    // }
    if (!::SetWindowPos(_hwnd, HWND_TOPMOST, _window_pos.x, _window_pos.y, _window_size.x, _window_size.y,
          SWP_NOACTIVATE | SWP_SHOWWINDOW))
      return unexpected_win32_error("SetWindowPos failed");
    return {};
  }

  std::expected<void, error_trace> show(int2 Pos, uint2 Size, std::wstring_view Text) {
    _visible = true;
    if (auto res = initialize(); !res) return unexpected_error(res.error());
    if (auto res = text_layout::create(Text, _layout, float2(1e6f, 1e6f))) _layout = std::move(*res);
    else return unexpected_error(res.error());
    if (auto res = _layout.metrics_size()) _layout_size = int2(*res);
    else return unexpected_error(res.error());
    _window_size = _layout_size + int2(8, 4);
    if (_swapchain) {
      _rendertarget = {};
      auto hr = _swapchain->ResizeBuffers(0, _window_size.x, _window_size.y, DXGI_FORMAT_UNKNOWN, 0);
      if (FAILED(hr)) return unexpected_error(errors::operation_failed, "ResizeBuffers failed", int32_t(hr));
    } else {
      if (auto res = dxgi.initialize(); !res) return unexpected_error(res.error());
      DXGI_SWAP_CHAIN_DESC1 desc(_window_size.x, _window_size.y, bitmap::dxgiformat, 0, DXGI_SAMPLE_DESC(1, 0), {}, 2);
      desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT, desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
      auto hr = dxgi.factory()->CreateSwapChainForHwnd(d3d.device(), _hwnd, &desc, nullptr, nullptr, &_swapchain.get());
      if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateSwapChainForHwnd failed", int32_t(hr));
    }
    if (auto res = bitmap::create(_swapchain.get())) _rendertarget = std::move(*res);
    else return unexpected_error(res.error());
    if (auto d = _rendertarget.begin_draw(colors::whitesmoke)) {
      brush.color(colors::black);
      if (auto res = draw_text({4, 2}, _layout); !res) return unexpected_error(res.error());
      if (auto res = draw_rectangle({0, 0}, _window_size, 1.0f); !res) return unexpected_error(res.error());
    } else return unexpected_error(errors::operation_failed, "begin_draw failed");
    _swapchain->Present(0, 0);
    if (auto res = show(Pos, Size); !res) return unexpected_error(res.error());
    return {};
  }

  void hide() {
    if (!_hwnd) return;
    ::ShowWindow(_hwnd, SW_HIDE);
    _visible = false;
  }
} tooltip;
} // namespace yw::system
