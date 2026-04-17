#pragma once
#include "ywx/bitmap.h"
#include "ywx/text.h"

namespace yw::system {

class {
  HWND _hwnd{};
  int2 _window_pos{};
  int2 _window_size{};
  bool _visible = false;

  bitmap _rendertarget;
  comptr<IDXGISwapChain1> _swapchain;

  text _composition;
  uint32_t _cursor_pos = 0;

  const DWORD _style = WS_POPUP;
  const DWORD _ex_style = WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE;

  struct ime_attr_run {
    uint32_t begin, end;
    uint8_t type;
  };

  std::vector<ime_attr_run> _attrs;

public:
  HWND hwnd() const noexcept { return _hwnd; }
  int2 window_size() const noexcept { return _window_size; }
  int2 window_pos() const noexcept { return _window_pos; }
  bool visible() const noexcept { return _visible; }

  void reset_state() {
    _cursor_pos = 0;
    _attrs.clear();
    check_error(update_text(L""));
  }

  void hide() {
    if (!_hwnd) return;
    ::ShowWindow(_hwnd, SW_HIDE);
    _visible = false;
  }

  std::expected<void, error_trace> initialize() {
    if (_hwnd) return {};
    WNDCLASSW wc{};
    wc.lpfnWndProc = ::DefWindowProcW;
    wc.hInstance = ::GetModuleHandleW(nullptr);
    wc.hCursor = ::LoadCursorW(nullptr, IDC_ARROW);
    wc.lpszClassName = L"ywlib_ime_class";
    if (!::RegisterClassW(&wc) && ::GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
      return unexpected_win32_error("RegisterClassW failed");
    _hwnd = ::CreateWindowExW(_ex_style, wc.lpszClassName, L"", _style, 0, 0, 100, 100, 0, 0, wc.hInstance, 0);
    if (!_hwnd) return unexpected_win32_error("CreateWindowExW failed");
    if (auto res = text::create(L"")) _composition = std::move(*res);
    else return unexpected_error(res.error());
    return {};
  }

  void update_cursor_pos(int32_t pos) noexcept { _cursor_pos = pos; }

  template<stringable S> std::expected<void, error_trace> update_text(S&& Text) {
    if (auto res = _composition(static_cast<S&&>(Text)); !res) return unexpected_error(res.error());
    return {};
  }

  template<text_format_like TF> std::expected<void, error_trace> update_text_format(TF&& Format) {
    if (auto res = text::create(_composition(), static_cast<TF&&>(Format)); !res) return unexpected_error(res.error());
    return {};
  }

  std::expected<void, error_trace> update_attrs(const std::vector<uint8_t>& attrs) {
    _attrs.clear();
    if (attrs.empty()) {
      _attrs.clear();
      return {};
    }
    _attrs.reserve(attrs.size());
    uint8_t a = attrs[0];
    for (uint32_t i = 0, j = 1; true; ++j) {
      if (j == attrs.size()) {
        _attrs.push_back({i, j, a});
        break;
      }
      if (a != attrs[j]) {
        _attrs.push_back({i, j, a});
        a = attrs[j];
        i = j;
      }
    }
    return {};
  }

  std::expected<void, error_trace> update_window_size() {
    if (auto res = initialize(); !res) return unexpected_error(res.error());
    const auto window_size = int2(_composition.size()) + int2(5, 0); // キャレットを右端に描画用するための余裕
    if (window_size == _window_size || window_size == int2{}) goto skip_resize;
    _window_size = window_size;
    if (_swapchain) {
      _rendertarget = {};
      auto hr = _swapchain->ResizeBuffers(0, window_size.x, window_size.y, DXGI_FORMAT_UNKNOWN, 0);
      if (FAILED(hr)) return unexpected_error(errors::operation_failed, "ResizeBuffers failed", int32_t(hr));
    } else {
      if (auto res = dxgi.initialize(); !res) return unexpected_error(res.error());
      DXGI_SWAP_CHAIN_DESC1 desc(window_size.x, window_size.y, bitmap::dxgiformat, 0, DXGI_SAMPLE_DESC(1, 0), {}, 2);
      desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT, desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
      auto hr = dxgi.factory()->CreateSwapChainForHwnd(d3d.device(), _hwnd, &desc, nullptr, nullptr, &_swapchain.get());
      if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateSwapChainForHwnd failed", int32_t(hr));
    }
    if (auto res = bitmap::create(_swapchain.get())) _rendertarget = std::move(*res);
    else return unexpected_error(res.error());
  skip_resize:
    return {};
  }

  std::expected<void, error_trace> draw() {
    if (auto res = initialize(); !res) return unexpected_error(res.error());
    if (!_rendertarget) return unexpected_error(errors::operation_failed, "Render target not initialized");
    if (auto d = _rendertarget.begin_draw(colors::whitesmoke)) {
      brush.color(colors::black);
      for (const auto& attr : _attrs) {
        auto res = _composition.hit_test_range({attr.begin, attr.end});
        if (!res) return unexpected_error(res.error());
        if (res->empty()) continue;
        const auto pt = res->operator[](0).xy();
        const auto sz = res->operator[](0).zw();
        switch (attr.type) {
        case ATTR_TARGET_CONVERTED:
        case ATTR_TARGET_NOTCONVERTED:
          brush.color(color(0.5f, 0.5f, 1.0f, 0.5f));
          fill_rectangle(pt, sz);
          brush.color(colors::black);
          draw_line({pt.x, sz.y}, {pt.x + sz.x, sz.y}, 4.0f);
          break;
        case ATTR_CONVERTED: draw_line({pt.x, sz.y}, {pt.x + sz.x, sz.y}, 2.0f); break;
        default:
          brush.dashed(true);
          draw_line({pt.x, sz.y}, {pt.x + sz.x, sz.y}, 2.0f);
          brush.dashed(false);
        }
      }
      brush.color(colors::black);
      if (auto res = draw_text({}, _composition); !res) return unexpected_error(res.error());
      auto ht = _composition.hit_test(_cursor_pos);
      if (!ht) return unexpected_error(ht.error());
      if (auto res = draw_line(ht->xy(), ht->xw()); !res)
        return unexpected_error(res.error());
    }
    _swapchain->Present(0, 0);
    return {};
  }

  std::expected<void, error_trace> show(int2 GlobalPos) {
    if (auto res = initialize(); !res) return unexpected_error(res.error());
    const auto [w, h] = _window_size;
    if (!::SetWindowPos(_hwnd, HWND_TOPMOST, GlobalPos.x, GlobalPos.y, w, h, SWP_NOACTIVATE | SWP_SHOWWINDOW))
      return unexpected_win32_error("SetWindowPos failed");
    return {};
  }
} ime;
} // namespace yw::system
