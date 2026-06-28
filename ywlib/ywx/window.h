#pragma once
#include "ywx/bitmap.h"
#include "ywx/keys.h"

namespace yw {

class raw_window : public interface {
public:
  struct slot : interface::slot {
    inline static std::vector<slotid> windows{};
    HWND hwnd{};
    int4 frame_thickness{};
    int2 pos{}, size{};
    DWORD style{}, exstyle{};
    string<wchar_t> title{};
    bitmap rendertarget{};
    comptr<IDXGISwapChain1> swapchain{};

    std::expected<void, error> create_window() {
      if (hwnd) return std::unexpected(error(errors::invalid_operation, "window already created"));
      hwnd = ::CreateWindowExW(exstyle, wclass::name(), title.c_str(), style, CW_USEDEFAULT, CW_USEDEFAULT,
        int(arbitrary_value), int(arbitrary_value), nullptr, nullptr, wclass::hinstance(), nullptr);
      if (!hwnd) return std::unexpected(error(errors::operation_failed, "CreateWindowExW failed"));
      ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, std::bit_cast<LONG_PTR>(id));
      windows.push_back(id);
      return {};
    }

    std::expected<void, error> calculate_frame_thickness() {
      RECT wr{}, cr{};
      if (!::GetWindowRect(hwnd, &wr)) return std::unexpected(error(errors::operation_failed, "GetWindowRect failed"));
      if (!::GetClientRect(hwnd, &cr)) return std::unexpected(error(errors::operation_failed, "GetClientRect failed"));
      const auto left = (wr.right - wr.left) / 2;
      frame_thickness = int4(left, wr.bottom - wr.top - cr.bottom - left, left, left);
      size = int2(cr.right, cr.bottom);
      pos = int2(wr.left, wr.top);
      return {};
    }

    std::expected<void, error> update_position() {
      const auto area = size + frame_thickness.xy() + frame_thickness.zw();
      if (::SetWindowPos(hwnd, nullptr, pos.x, pos.y, area.x, area.y, SWP_NOZORDER | SWP_NOACTIVATE)) return {};
      else return std::unexpected(error(errors::operation_failed, "SetWindowPos failed"));
    }

    std::expected<void, error> update_rendertarget() {
      const auto usz = vapply_r<uint2>(yw::max, size, uint2::fill(arbitrary_value));
      if (!rendertarget) (void)0;
      else if (rendertarget.size() == usz) return {};
      else rendertarget = {};
      if (!swapchain) {
        auto desc = DXGI_SWAP_CHAIN_DESC1(usz.x, usz.y, bitmap::dxgiformat, false, DXGI_SAMPLE_DESC(1, 0), {}, 2);
        desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT, desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        hresult_test(dxgi::factory()->CreateSwapChainForHwnd, d3d().device(), hwnd, &desc, 0, 0, &swapchain.get());
      } else hresult_test(swapchain->ResizeBuffers, 0, usz.x, usz.y, DXGI_FORMAT_UNKNOWN, 0);
      if (auto res = bitmap::create(swapchain.get())) rendertarget = std::move(*res);
      else return res.error().relay();
      return {};
    }
  };

  struct options {
    string<wchar_t> title{};
    std::optional<int2> pos{};
    std::optional<int2> size{};
  };

  virtual ~raw_window() { close(); }
  raw_window() noexcept = default;
  raw_window(raw_window&&) noexcept = default;

  raw_window& operator=(raw_window&& Other) noexcept {
    if (this == &Other) return *this;
    close();
    _id = std::exchange(Other._id, {});
    return *this;
  }

  raw_window(options&& Options, const source_line& sl = here()) {
    const auto temp_id = slot::add<raw_window>();
    const auto sp = slot::get<raw_window>(temp_id);
    if (!sp) error(errors::slot_creation_failed).go_off(sl); // fatal
    sp->id = temp_id;
    sp->source_line = sl;
    sp->title = std::move(Options.title);
    if (auto res = sp->create_window(); !res) res.error().go_off(sl);
    if (auto res = sp->calculate_frame_thickness(); !res) res.error().go_off(sl);
    if (Options.pos) sp->pos = *Options.pos;
    if (Options.size) sp->size = *Options.size;
    if (auto res = sp->update_position(); !res) res.error().go_off(sl);
    if (auto res = sp->update_rendertarget(); !res) res.error().go_off(sl);

  }
};
} // namespace yw
