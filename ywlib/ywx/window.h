#pragma once
#include "ywx/command_manager.h"
#include "ywx/tooltip.h"
#include "ywx/ui_parts.h"

/// \note `window::open(options)` creates a standard window.
/// \note `window::open<type>(...)` to create special windows, such as modal windows.

/// \note slot::id means id of itself
/// \note slot::layout_id means id of child control
/// \note slot::window_id means id of parent window if it has

namespace yw {

class window : public unknown {
public:
  enum class type : uint32_t {
    standard,  // you can select style from `appearance`
    custom,    // you can define your own style by `WS_*`
    subwindow, // standard window whose lifetime is tied to its parent
    modal,     // standard window which disables its all relatives
  };

  enum class appearance : uint32_t {
    unknown,
    regular = WS_OVERLAPPEDWINDOW,
    size_fixed = WS_OVERLAPPED | WS_BORDER | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
    borderless = WS_POPUP
  };

  template<type Type> struct options {
    std::optional<int2> pos = {};
    std::optional<uint2> size = {};
    std::optional<std::wstring> title = {};
    appearance appearance = window::appearance::regular;
    bool show = true;
  };

  template<type Type> static std::expected<window, error_trace> open(options<Type> Options);
  static std::expected<window, error_trace> open(options<type::standard> Options) {
    if (auto res = open<type::standard>(Options)) return std::move(*res);
    else return unexpected_error(res.error());
  }

  void close() noexcept;

  struct slot : public unknown::slot {
    struct core : public ui::part_base {
      HWND hwnd{};
      int4 frame_thickness{};
      int2 pos{};
      int2 size{};
      type type{};
      DWORD style{};
      DWORD exstyle{};
      std::wstring title{};
      bitmap rendertarget{};
      bitmap ui_bitmap{};
      comptr<IDXGISwapChain1> swapchain{};

      int2 area() const noexcept { return size + frame_thickness.xy() + frame_thickness.zw(); }

      std::expected<void, error_trace> init_window() {
        if (hwnd) return unexpected_error(errors::operation_failed, "Window already initialized");
        hwnd = ::CreateWindowExW(
          exstyle, wclass.name(), title.c_str(), style, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, 0, 0, wclass.hinstance(),
          0);
        if (!hwnd) return unexpected_win32_error("CreateWindowExW failed");
        return {};
      }

      std::expected<void, error_trace> init_frame_thickness() {
        RECT wr, cr;
        if (!::GetWindowRect(hwnd, &wr)) return unexpected_win32_error("GetWindowRect failed");
        if (!::GetClientRect(hwnd, &cr)) return unexpected_win32_error("GetClientRect failed");
        const auto left = (wr.right - wr.left - cr.right) / 2;
        const auto top = wr.bottom - wr.top - cr.bottom - left;
        frame_thickness = int4(left, top, left, left); // 決め打ちだが、例外はないはず
        size = int2(cr.right, cr.bottom);
        pos = int2(wr.left, wr.top);
        return {};
      }

      std::expected<void, error_trace> init_pos_size() {
        if (!::SetWindowPos(hwnd, nullptr, pos.x, pos.y, size.x, size.y, SWP_NOZORDER | SWP_NOACTIVATE))
          return unexpected_win32_error("SetWindowPos failed");
        return {};
      }

      std::expected<void, error_trace> init_rendertarget() {
        const auto usz = uint2(size);
        rendertarget = {};
        if (!swapchain) {
          auto desc = DXGI_SWAP_CHAIN_DESC1(usz.x, usz.y, bitmap::dxgiformat, false, DXGI_SAMPLE_DESC(1, 0), {}, 2);
          desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT, desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
          const auto hr = dxgi.factory()->CreateSwapChainForHwnd(d3d.device(), hwnd, &desc, 0, 0, &swapchain.get());
          if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateSwapChain failed", int(hr));
          if (!swapchain) return unexpected_error(errors::operation_failed, "CreateSwapChain failed");
        } else if (const auto hr = swapchain->ResizeBuffers(0, usz.x, usz.y, DXGI_FORMAT_UNKNOWN, 0); FAILED(hr))
          return unexpected_error(errors::operation_failed, "ResizeBuffers failed", int(hr));
        if (auto res = bitmap::create(swapchain.get())) rendertarget = std::move(*res);
        else return unexpected_error(res.error());
        if (auto res = bitmap::create(size)) ui_bitmap = std::move(*res);
        else return unexpected_error(res.error());
        return {};
      }

      std::expected<void, error_trace> update_rendertarget() {
        if (bool(rendertarget) && size == rendertarget.size()) return {};
        if (size.x == 0 || size.y == 0) return {};
        const auto usz = uint2(size);
        rendertarget = {};
        if (!swapchain) {
          auto desc = DXGI_SWAP_CHAIN_DESC1(usz.x, usz.y, bitmap::dxgiformat, false, DXGI_SAMPLE_DESC(1, 0), {}, 2);
          desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT, desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
          const auto hr = dxgi.factory()->CreateSwapChainForHwnd(d3d.device(), hwnd, &desc, 0, 0, &swapchain.get());
          if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateSwapChain failed", int(hr));
          if (!swapchain) return unexpected_error(errors::operation_failed, "CreateSwapChain failed");
        } else if (const auto hr = swapchain->ResizeBuffers(0, usz.x, usz.y, DXGI_FORMAT_UNKNOWN, 0); FAILED(hr))
          return unexpected_error(errors::operation_failed, "ResizeBuffers failed", int(hr));
        if (auto res = bitmap::create(swapchain.get())) rendertarget = std::move(*res);
        else return unexpected_error(res.error());
        return {};
      }

      class handle : public ui::part_base::handle<core> {
        friend struct core;
        using ui::part_base::handle<core>::handle;

      public:
        ~handle() {
          if (!_p) return;
          if (_p->layout_changed) {
            if (!::SetWindowPos(
                  _p->hwnd, nullptr, _p->pos.x, _p->pos.y, _p->size.x, _p->size.y, SWP_NOZORDER | SWP_NOACTIVATE))
              fatal_error(errors::operation_failed, "SetWindowPos failed");
          }
        }

        const auto& hwnd() const { return _p->hwnd; }
        const auto& frame_thickness() const { return _p->frame_thickness; }
        const auto& type() const { return _p->type; }
        const auto& style() const { return _p->style; }
        const auto& exstyle() const { return _p->exstyle; }

        const auto& pos() const { return _p->pos; }
        auto& pos(int2 Pos) {
          _p->pos = Pos;
          _p->layout_changed = true;
          return *this;
        }

        const auto& size() const { return _p->size; }
        auto& size(uint2 Size) {
          _p->size = Size;
          _p->layout_changed = true;
          return *this;
        }

        const auto& title() const { return _p->title; }
        auto& title(std::wstring Title) {
          ::SetWindowTextW(_p->hwnd, Title.c_str());
          _p->title = std::move(Title);
          return *this;
        }
      };

      handle handle() noexcept { return *this; }
    } core;

    ui::parts::background background;
    ui::parts::focus_ring focus_ring;

    ui::slotid focused_control{};
    ui::slotid hovered_control{};
    ui::slotid captured_control{};

    bool dirty = true, messy = true, manually_drawn = false;
    bool visible = false, active = false;
    bool resizing = false;
    bool tracking = false;

    function<bool> on_close;
    function<void, events::key> on_keydown;
    function<void, events::key> on_keyup;

    command_manager commands;
    std::vector<ui::slotid> subwindows;

    //-- override functions --//

    virtual const char* attachable() const override {
      if (layout_id) return "No more controls can be attached to this window";
      else return nullptr;
    }

    virtual void attach(ui::slotid Child) override {
      layout_id = Child;
      make_messy();
    }

    virtual void detach(ui::slotid Child) override {
      if (layout_id == Child) {
        system::uis.erase(std::exchange(layout_id, {}));
        make_messy();
      }
    }

    void make_dirty() override { dirty = true; }
    void make_moved() override { dirty = true; }
    void make_messy() override { messy = true; }

    //-- functions --//

    // std::expected<void, error_trace> resize_window(uint2 Size) {
    //   if (core.size == Size) return {};
    //   core.size = int2(Size);
    //   const auto area = core.area();
    //   if (!::SetWindowPos(core.hwnd, nullptr, 0, 0, area.x, area.y, SWP_NOMOVE | SWP_NOACTIVATE))
    //     return unexpected_win32_error("SetWindowPos failed");
    //   /// \note SetWindowPos によって WM_SIZE が発生する。
    //   /// \note size 更新や resize_rendertarget は WM_SIZE 内で行う。(GUIでのサイズ変更と合わせるため)
    //   return {};
    // }

    // std::expected<void, error_trace> initialize() {
    //   if (core.hwnd) return {};
    //   if (auto res = wclass.initialize(); !res) return unexpected_error(res.error());
    //   if (auto res = dxgi.initialize(); !res) return unexpected_error(res.error());
    //   core.hwnd = ::CreateWindowExW(
    //     core.exstyle, wclass.name(), core.title.data(), core.style, core.pos.x, core.pos.y, core.size.x, core.size.y,
    //     0, 0, wclass.hinstance(), 0);
    //   if (!core.hwnd) return unexpected_win32_error("CreateWindowExW failed");
    //   ::SetWindowLongPtrW(core.hwnd, GWLP_USERDATA, id);
    //   return {};
    // }

    // std::expected<void, error_trace> update_ui_layout() {
    //   const auto csp = system::slot_address<ui::control>(id);
    //   if (!csp) return unexpected_error(errors::ui_invalid_slotid);
    //   csp->ensure_minimum_size();
    //   const auto minimum_area = csp->core.area();
    //   const auto available_area = vapply_r<float2>(yw::max, float2(core.size), minimum_area);
    //   csp->update_layout({}, available_area);
    //   if (auto res = resize_window(vapply_r<int2>(yw::ceil, available_area)); !res)
    //     return unexpected_error(res.error());
    //   messy = false;
    //   return {};
    // }

    // std::expected<void, error_trace> draw_ui_layout() {
    //   const auto csp = system::slot_address<ui::control>(id);
    //   if (!csp) return unexpected_error(errors::ui_invalid_slotid);
    //   if (!ui_bitmap || ui_bitmap.size() != core.size) {
    //     if (auto res = bitmap::create(core.size)) ui_bitmap = std::move(*res);
    //     else return unexpected_error(res.error());
    //   }
    //   if (auto d = ui_bitmap.begin_draw(colors::transparent)) csp->draw();
    //   else return unexpected_error(d.error());
    //   return {};
    // }

    // std::expected<void, error_trace> draw() {
    //   if (!dirty) return {};
    //   if (!core.rendertarget) {
    //     if (auto res = bitmap::create(core.swapchain.get())) core.rendertarget = std::move(*res);
    //     else return unexpected_error(res.error());
    //   }
    //   drawing d;
    //   if (!manually_drawn) {
    //     if (auto res = core.rendertarget.begin_draw(background.color)) d = std::move(*res);
    //     else return unexpected_error(res.error());
    //     if (background.image) draw_bitmap({}, core.size, background.image, background.image_opacity);
    //   } else if (auto res = core.rendertarget.begin_draw()) d = std::move(*res);
    //   else return unexpected_error(res.error());
    //   draw_bitmap({}, ui_bitmap);
    //   d.close();
    //   core.swapchain->Present(0, 0);
    //   manually_drawn = false;
    //   dirty = false;
    //   return {};
    // }
  };

  virtual ~window() noexcept override { close(); }
  explicit window() noexcept = default;
  window(window&& other) noexcept { _id = std::exchange(other._id, {}); }
  window& operator=(window&& other) noexcept {
    if (this == &other) return *this;
    close(), _id = std::exchange(other._id, {});
    return *this;
  }

  explicit operator bool() const noexcept { return system::uis.contains(_id); }

  auto& show() {
    if (auto wsp = system::slot_address<window>(_id)) {
      ::ShowWindow(wsp->core.hwnd, SW_SHOW);
    } else fatal_error(errors::ui_invalid_slotid);
    return *this;
  }

  auto core() {
    const auto wsp = system::slot_address<window>(_id);
    if (!wsp) fatal_error(errors::ui_invalid_slotid);
    return wsp->core.handle();
  }
  const auto core() const {
    const auto wsp = system::slot_address<window>(_id);
    if (!wsp) fatal_error(errors::ui_invalid_slotid);
    return wsp->core.handle();
  }

  auto background() {
    const auto wsp = system::slot_address<window>(_id);
    if (!wsp) fatal_error(errors::ui_invalid_slotid);
    return wsp->background.handle();
  }
  const auto background() const {
    const auto wsp = system::slot_address<window>(_id);
    if (!wsp) fatal_error(errors::ui_invalid_slotid);
    return wsp->background.handle();
  }

  const auto& on_close() const {
    const auto wsp = system::slot_address<window>(_id);
    if (!wsp) fatal_error(errors::ui_invalid_slotid);
    return wsp->on_close;
  }
  std::expected<void, error_trace> on_close(function<bool> OnClose) const {
    if (auto wsp = system::slot_address<window>(_id)) {
      wsp->on_close = std::move(OnClose);
      return {};
    } else return unexpected_error(errors::ui_invalid_slotid);
  }

  const auto& on_keydown() const {
    const auto wsp = system::slot_address<window>(_id);
    if (!wsp) fatal_error(errors::ui_invalid_slotid);
    return wsp->on_keydown;
  }
  std::expected<void, error_trace> on_keydown(function<void, events::key> OnKeyDown) const {
    if (auto wsp = system::slot_address<window>(_id)) {
      wsp->on_keydown = std::move(OnKeyDown);
      return {};
    } else return unexpected_error(errors::ui_invalid_slotid);
  }

  const auto& on_keyup() const {
    const auto wsp = system::slot_address<window>(_id);
    if (!wsp) fatal_error(errors::ui_invalid_slotid);
    return wsp->on_keyup;
  }
  std::expected<void, error_trace> on_keyup(function<void, events::key> OnKeyUp) const {
    if (auto wsp = system::slot_address<window>(_id)) {
      wsp->on_keyup = std::move(OnKeyUp);
      return {};
    } else return unexpected_error(errors::ui_invalid_slotid);
  }

  const auto& commands() const {
    const auto wsp = system::slot_address<window>(_id);
    if (!wsp) fatal_error(errors::ui_invalid_slotid);
    return wsp->commands;
  }
  std::expected<void, error_trace> commands() { return unsafe_get(&slot::commands); }

  std::expected<drawing, error_trace> begin_draw() {
    if (const auto wsp = system::slot_address<window>(_id)) {
      wsp->dirty = true;
      wsp->manually_drawn = true;
      if (auto d = wsp->core.rendertarget.begin_draw(wsp->background.color)) {
        if (wsp->background.image)
          draw_bitmap({}, wsp->core.size, wsp->background.image, wsp->background.image_opacity);
        return std::move(d);
      } else return unexpected_error(d.error());
    } else return unexpected_error(errors::ui_invalid_slotid);
  }

  /// \note コールバック関数内で使用しても機能しない。フラグを立て、ループ内で実行すること。
  std::expected<void, error_trace> screenshot(const std::filesystem::path& PngPath, bool WriteUI = true) {
    const auto wsp = system::slot_address<window>(_id);
    if (!wsp) return unexpected_error(errors::ui_invalid_slotid);
    drawing d;
    if (!wsp->manually_drawn) {
      if (auto res = wsp->core.rendertarget.begin_draw(wsp->background.color)) d = std::move(*res);
      else return unexpected_error(res.error());
      if (wsp->background.image) // 背景画像の表示サイズは拡大縮小する必要がある
        draw_bitmap({}, wsp->core.size, wsp->background.image, wsp->background.image_opacity);
    } else if (auto res = wsp->core.rendertarget.begin_draw()) d = std::move(*res);
    else return unexpected_error(res.error());
    wsp->manually_drawn = false;
    if (WriteUI) {
      wsp->draw_ui_layout();
      draw_bitmap({}, wsp->core.ui_bitmap); // UIレイヤーのサイズはレンダーターゲットと同じ
      wsp->dirty = false;
    }
    d.close();
    if (auto res = wsp->core.rendertarget.save_as_png(PngPath); !res) return unexpected_error(res.error());
    return {};
  }
};

template<> struct window::options<window::type::standard> {
  std::optional<int2> pos = {};
  std::optional<uint2> size = {};
  std::optional<std::wstring> title = {};
  appearance appearance = window::appearance::regular;
  bool show = true;
};

template<> struct window::options<window::type::custom> {
  std::optional<int2> pos = {};
  std::optional<uint2> size = {};
  std::optional<std::wstring> title = {};
  DWORD style = 0;
  DWORD exstyle = 0;
};

template<> struct window::options<window::type::subwindow> {
  ui::slotid parent; // required
  std::optional<int2> pos = {};
  std::optional<uint2> size = {};
  std::optional<std::wstring> title = {};
  appearance appearance = window::appearance::regular;
  bool show = true;
};

template<> struct window::options<window::type::modal> {
  ui::slotid parent = {}; // optional. zero means all windows get disabled
  std::optional<int2> pos = {};
  std::optional<uint2> size = {};
  std::optional<std::wstring> title = {};
  appearance appearance = window::appearance::regular;
};

template<window::type Type> std::expected<window, error_trace> window::open(window::options<Type> Options) {
  if (auto res = wclass.initialize(); !res) return unexpected_error(res.error());
  if (auto res = d2d.initialize(); !res) return unexpected_error(res.error());
  const auto id = system::uis.add(std::make_unique<window::slot>());
  const auto wsp = system::slot_address<window>(id);
  if (!wsp) return unexpected_error(errors::ui_invalid_slotid);
  wsp->id = id;
  wsp->core.control_id = id;
  wsp->core.type = Type;
  if constexpr (Type == window::type::standard) {
    wsp->core.style = static_cast<DWORD>(Options.appearance) | (Options.show ? WS_VISIBLE : 0);
    wsp->core.exstyle = WS_EX_ACCEPTFILES;
    system::primal_windows.push_back(wsp->id);
  } else if constexpr (Type == window::type::custom) {
    wsp->core.style = Options.style;
    wsp->core.exstyle = Options.exstyle;
  } else if constexpr (Type == window::type::subwindow) {
    wsp->core.style = static_cast<DWORD>(Options.appearance) | (Options.show ? WS_VISIBLE : 0);
    wsp->core.exstyle = WS_EX_ACCEPTFILES;
    const auto psp = system::slot_address<window>(Options.parent);
    if (!psp) return unexpected_error(errors::invalid_argument, "Invalid parent window");
    wsp->window_id = Options.parent;
    psp->subwindows.push_back(wsp->id);
  } else if constexpr (Type == window::type::modal) {
    wsp->core.style = static_cast<DWORD>(Options.appearance) | (Options.show ? WS_VISIBLE : 0);
    wsp->core.exstyle = WS_EX_ACCEPTFILES;
    if (!Options.parent) {
      for (const auto wid : system::primal_windows) {
        const auto sp = system::slot_address<window>(wid);
        if (!sp) return unexpected_error(errors::operation_failed, "unknown window slot is found");
        if (auto res = sp->disable(); !res) return unexpected_error(res.error());
      }
    } else if (const auto psp = system::slot_address<window>(Options.parent)) {
      if (auto res = psp->disable(); !res) return unexpected_error(res.error());
    } else return unexpected_error(errors::invalid_argument, "Invalid parent window");
  } else static_assert(always_false<decltype(Type)>);
  if (!Options.title) {
    std::wstring t(MAX_PATH, L'\0');
    if (const auto n = ::GetModuleFileNameW(nullptr, t.data(), MAX_PATH)) t.resize(n);
    else return unexpected_win32_error("GetModuleFileNameW failed");
    wsp->core.title = std::move(t);
  } else wsp->core.title = std::move(*Options.title);
  if (auto res = core.init_window(); !res) return unexpected_error(res.error());
  if (auto res = core.init_frame_thickness(); !res) return unexpected_error(res.error());
  if (Options.pos.has_value()) core.pos = *Options.pos;
  if (Options.size.has_value()) core.size = vapply_r<int2>(yw::max, core.size, *Options.size);
  if (auto res = core.init_pos_size(); !res) return unexpected_error(res.error());
  return {};
}

void window::close() noexcept {
  /// \note Other actions besides window destruction are executed by the slot::close function called via WM_NCDESTROY.
  const auto wsp = system::slot_address<window>(_id);
  if (!wsp) return;
  ::DestroyWindow(wsp->core.hwnd);
  wsp->core.hwnd = nullptr;
}
} // namespace yw
