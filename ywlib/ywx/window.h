#pragma once
#include "ywx/bitmap.h"
#include "ywx/keys.h"

namespace yw {

template<typename T> concept window_options_like = requires(T t) {
  { t.title } -> convertible_to<string<wchar_t>>;
  { t.pos } -> convertible_to<std::optional<int2>>;
  { t.size } -> convertible_to<std::optional<int2>>;
  { t.style() } -> convertible_to<DWORD>;
  { t.exstyle() } -> convertible_to<DWORD>;
};

class window : public interface {
public:
  struct options {
    string<wchar_t> title{};
    std::optional<int2> pos{};
    std::optional<int2> size{};
    bool has_border = true;
    bool has_caption = true;
    bool resizeable = true;
    bool visible = true;
    DWORD style() const noexcept {
      DWORD s = has_caption ? WS_CAPTION | WS_SYSMENU : WS_POPUP;
      if (has_border) s |= WS_BORDER;
      if (resizeable) s |= WS_THICKFRAME;
      if (visible) s |= WS_VISIBLE;
      return s;
    }
    DWORD exstyle() const noexcept { return WS_EX_ACCEPTFILES; }
  };

  struct custom_options {
    string<wchar_t> title{};
    std::optional<int2> pos{};
    std::optional<int2> size{};
    DWORD style = WS_OVERLAPPEDWINDOW;
    DWORD exstyle = WS_EX_ACCEPTFILES;
    DWORD style() const noexcept { return style; }
    DWORD exstyle() const noexcept { return exstyle; }
  };

  struct slot : interface::slot {
    inline static std::vector<slotid> windows{};
    HWND hwnd{};
    int4 frame_thickness{};
    int2 pos{}, size{};
    DWORD style{}, exstyle{};
    string<wchar_t> title{};
    bitmap controllayer{};
    bitmap rendertarget{};
    comptr<IDXGISwapChain1> swapchain{};

    slotid control_id{};

    bool messy = false;
    bool dirty = false;
    bool resizing = false;

    virtual std::expected<float2, error> get_necessary_size() const override {
      if (const auto csp = slot::get<control>(control_id); !csp) return size;
      else if (auto res = csp->get_necessary_size(); !res) return res.error().relay();
      else return vapply_r<float2>(yw::max, size, *res);
    }

    template<typename T> static std::expected<slot*, error> create(T&& Options, const yw::source_line& sl) {
      static_assert(is_rvref<T>, "Unreachable");
      const auto temp_id = slot::add<window>();
      const auto sp = slot::get<window>(temp_id);
      if (!sp) return std::unexpected(error(errors::slot_creation_failed));
      sp->id = temp_id;
      sp->source_line = sl;
      sp->title = std::move(Options.title);
      sp->style = Options.style();
      sp->exstyle = Options.exstyle();
      sp->hwnd = ::CreateWindowExW(
        sp->exstyle, wclass::name(), sp->title.c_str(), sp->style, CW_USEDEFAULT, CW_USEDEFAULT, int(arbitrary_value),
        int(arbitrary_value), nullptr, nullptr, wclass::hinstance(), nullptr);
      if (!sp->hwnd) return std::unexpected(error(errors::operation_failed, "CreateWindowExW failed"));
      ::SetWindowLongPtrW(sp->hwnd, GWLP_USERDATA, std::bit_cast<LONG_PTR>(sp->id));
      windows.push_back(sp->id);
      RECT wr{}, cr{};
      win32_bool_test(::GetWindowRect, sp->hwnd, &wr);
      win32_bool_test(::GetClientRect, sp->hwnd, &cr);
      const auto left = (wr.right - wr.left) / 2;
      sp->frame_thickness = int4(left, wr.bottom - wr.top - cr.bottom - left, left, left);
      sp->pos = Options.pos.value_or(int2(wr.left, wr.top));
      sp->size = Options.size.value_or(int2(cr.right, cr.bottom));
      const auto area = sp->size + sp->frame_thickness.xy() + sp->frame_thickness.zw();
      win32_bool_test(::SetWindowPos, sp->hwnd, 0, sp->pos.x, sp->pos.y, area.x, area.y, SWP_NOZORDER | SWP_NOACTIVATE);
      const auto usz = vapply_r<uint2>(yw::max, sp->size, uint2::fill(arbitrary_value));
      auto desc = DXGI_SWAP_CHAIN_DESC1(usz.x, usz.y, bitmap::dxgiformat, false, DXGI_SAMPLE_DESC(1, 0), {}, 2);
      desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT, desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
      hresult_test(dxgi::factory()->CreateSwapChainForHwnd, d3d::device(), sp->hwnd, &desc, 0, 0, &sp->swapchain.get());
      if (auto res = bitmap::create(sp->swapchain.get())) sp->rendertarget = std::move(*res);
      else return res.error().relay();
      if (auto res = bitmap::create(sp->rendertarget.size())) sp->controllayer = std::move(*res);
      else return res.error().relay();
      return sp;
    }

    std::expected<void, error> update() {
      if (!swapchain) return std::unexpected(error(errors::operation_failed, "swapchain is null"));
      if (auto res = get_necessary_size(); !res) return res.error().relay();
      else size = vapply_r<uint2>(yw::ceil, *res);
      if (sp && messy) {
        const auto necessary_size = sp->get_necessary_size();
        size = vapply_r<uint2>(yw::max, size, necessary_size);
      }
      if (!resizing && size != rendertarget.size()) {
        if (auto res = bitmap::create(swapchain.get())) rendertarget = std::move(*res);
        else return res.error().relay();
        if (auto res = bitmap::create(rendertarget.size())) controllayer = std::move(*res);
        else return res.error().relay();
        messy = true;
      }
      if (messy) {

      }
    }
  };

  virtual ~window() { close(); }
  explicit window() noexcept = default;
  window(window&&) noexcept = default;

  window& operator=(window&& Other) noexcept {
    if (this == &Other) return *this;
    close();
    _id = std::exchange(Other._id, {});
    return *this;
  }

  window(options Options, const source_line& sl = here()) {
    if (auto res = slot::create(std::move(Options), sl)) _id = (*res)->id;
    else res.error().add_footprint().go_off(sl);
  }

  window(window_options_like auto Options, const source_line& sl = here()) {
    if (auto res = slot::create(std::move(Options), sl)) _id = (*res)->id;
    else res.error().add_footprint().go_off(sl);
  }

  std::expected<window, error> create(options Options, const source_line& sl = here()) {
    window w;
    if (auto res = slot::create(std::move(Options), sl)) w._id = (*res)->id;
    else return res.error().relay();
    return w;
  }

  std::expected<window, error> create(window_options_like auto Options, const source_line& sl = here()) {
    window w;
    if (auto res = slot::create(std::move(Options), sl)) w._id = (*res)->id;
    else return res.error().relay();
    return w;
  }

  std::expected<void, error> close() {
    const auto sp = slot::get<window>(_id);
    if (!sp || !sp->hwnd) return {}; // nothing to do
    if (::DestroyWindow(sp->hwnd)) {
      ::SetWindowLongPtrW(std::exchange(sp->hwnd, nullptr), GWLP_USERDATA, 0);
      return {};
    } else return std::unexpected(error(errors::operation_failed, "DestroyWindow failed"));
  }
};
} // namespace yw
