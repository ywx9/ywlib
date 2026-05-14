#pragma once
#include "ywx/command_manager.h"
#include "ywx/tooltip.h"
#include "ywx/ui_parts.h"

/// \note `window::open(options)` creates a standard window.
/// \note `window::open<type>(...)` to create special windows, such as modal windows.

namespace yw {

class window : public unknown {
public:
  enum class type : uint32_t {
    standard,  // you can select style from `appearance`
    custom,    // you can define your own style by `WS_*`
    subwindow, // standard window whose lifetime is tied to its parent
    modal,     // standard window which disables its parent and siblings
  };

  enum class appearance : uint32_t {
    unknown,
    regular = WS_OVERLAPPEDWINDOW,
    size_fixed = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
    borderless = WS_POPUP
  };

  template<type Type> struct options {
    std::optional<int2> pos = {};
    std::optional<uint2> size = {};
    std::optional<std::wstring> title = {};
    appearance appearance = window::appearance::regular;
    bool show = true;
  };

  std::expected<window, error_trace> open(options<type::standard> Options);
  template<type Type> std::expected<window, error_trace> open(options<Type> Options);

  struct slot : public unknown::slot {
    struct core : public ui::part_base {
      HWND hwnd{};
      int4 frame_thickness{};
      int2 pos{};
      int2 size{};
      DWORD style{};
      DWORD exstyle{};
      std::wstring title{};
      bitmap rendertarget{};
      comptr<IDXGISwapChain1> swapchain{};

      class handle : public ui::part_base::handle<core> {
        friend struct core;
        using ui::part_base::handle<core>::handle;

      public:
        const auto& hwnd() const { return _p->hwnd; }
        const auto& frame_thickness() const { return _p->frame_thickness; }
        const auto& pos() const { return _p->pos; }
        const auto& style() const { return _p->style; }
        const auto& exstyle() const { return _p->exstyle; }

        const auto& size() const { return _p->size; }
        auto& size(float2 Size) {
          _p->size = Size;
          _p->layout_changed = true;
          return *this;
        }
      };

      handle handle() noexcept { return *this; }

      std::expected<void, error_trace> resize_window(uint2 Size) {
        if (size == Size) return {};
        const auto area = int2(Size) + frame_thickness.xy() + frame_thickness.zw();
        if (!::SetWindowPos(hwnd, nullptr, 0, 0, area.x, area.y, SWP_NOMOVE | SWP_NOACTIVATE))
          return unexpected_win32_error("SetWindowPos failed");
        /// \note SetWindowPos によって WM_SIZE が発生する。
        /// \note size 更新や resize_rendertarget は WM_SIZE 内で行う。(GUIでのサイズ変更と合わせるため)
        return {};
      }

      std::expected<void, error_trace> resize_rendertarget() {
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

      std::expected<void, error_trace> calculate_frame_thickness() {
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

      std::expected<void, error_trace> create_window() {
        if (hwnd) return {};
        if (auto res = wclass.initialize(); !res) return unexpected_error(res.error());
        if (auto res = dxgi.initialize(); !res) return unexpected_error(res.error());
        hwnd = ::CreateWindowExW(
          exstyle, wclass.name(), title.data(), style, pos.x, pos.y, size.x, size.y, 0, 0, wclass.hinstance(), 0);
        if (!hwnd) return unexpected_win32_error("CreateWindowExW failed");
        ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, control_id);
        return {};
      }

      std::expected<void, error_trace> update_ui_layout() {
        const auto csp = system::slot_address<ui::control>(control_id);
        if (!csp) return unexpected_error(errors::ui_invalid_slotid);
        csp->ensure_minimum_size();
        const auto minimum_area = csp->core.area();
        const auto available_area = vapply_r<float2>(yw::max, float2(size), minimum_area);
        csp->update_layout({}, available_area);
        if (size.x < minimum_area.x || size.y < minimum_area.y) {
          size = vapply_r<int2>(yw::max, size, vapply_r<int2>(yw::ceil, minimum_area));
          const auto area = size + frame_thickness.xy() + frame_thickness.zw();
          if (!::SetWindowPos(hwnd, nullptr, 0, 0, area.x, area.y, SWP_NOZORDER | SWP_NOMOVE))
            return unexpected_win32_error("SetWindowPos failed");
        }
        return {};
      }

      std::expected<void, error_trace> draw_ui_layout() {
        if (const auto csp = system::slot_address<ui::control>(control_id)) {
          if (!csp) return unexpected_error(errors::operation_failed, "Failed to get control slot address");
          if (!ui_bitmap || ui_bitmap.size() != size) {
            if (auto res = bitmap::create(size)) ui_bitmap = std::move(*res);
            else return unexpected_error(res.error());
          }
          if (auto d = ui_bitmap.begin_draw(colors::transparent)) csp->draw();
          else return unexpected_error(d.error());
        }
        /// \note dirty フラグのリセットは draw 関数まで待つ。
        return {};
      }
    } core;

    ui::parts::background background;
    ui::parts::focus_ring focus_ring;

    bitmap ui_bitmap{};
    ui::slotid control_id{};

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

    //-- override functions --//

    virtual const char* attachable() const override {
      if (control_id) return "No more controls can be attached to this window";
      else return nullptr;
    }

    virtual void attach(ui::slotid Child) override {
      control_id = Child;
      make_messy();
    }

    virtual void detach(ui::slotid Child) override {
      if (control_id == Child) {
        system::uis.erase(std::exchange(control_id, {}));
        make_messy();
      }
    }

    void make_dirty() override { dirty = true; }
    void make_moved() override { dirty = true; }
    void make_messy() override { messy = true; }

    //-- functions --//

    std::expected<void, error_trace> draw() {
      if (!dirty) return {};
      if (!rendertarget) {
        if (auto res = bitmap::create(swapchain.get())) rendertarget = std::move(*res);
        else return unexpected_error(res.error());
      }
      drawing d;
      if (!manually_drawn) {
        if (auto res = rendertarget.begin_draw(background.color)) d = std::move(*res);
        else return unexpected_error(res.error());
        if (background.image) draw_bitmap({}, size, background.image, background.image_opacity);
      } else if (auto res = rendertarget.begin_draw()) d = std::move(*res);
      else return unexpected_error(res.error());
      draw_bitmap({}, ui_bitmap);
      d.close();
      swapchain->Present(0, 0);
      manually_drawn = false;
      dirty = false;
      return {};
    }
  };

  void close() noexcept {
    if (const auto wsp = system::slot_address<window>(_id)) ::DestroyWindow(wsp->core.hwnd);
  }

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

  // const auto& commands() const { return unsafe_get(&slot::commands); }
  // auto& commands() { return unsafe_get(&slot::commands); }

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
      draw_bitmap({}, wsp->ui_bitmap); // UIレイヤーのサイズはレンダーターゲットと同じ
      wsp->dirty = false;
    }
    d.close();
    if (auto res = wsp->core.rendertarget.save_as_png(PngPath); !res) return unexpected_error(res.error());
    return {};
  }
};
} // namespace yw

// namespace yw {

// class window : public unknown {
// public:
//   /// window for display. No events except for `on_close` happen.
//   class sub;
//   ///
//   class main;

//   struct slot : public unknown::slot {
//     struct core : public ui::part_base {
//       HWND hwnd{};
//       int4 frame_thickness{};
//       int2 pos{};
//       int2 size{};
//       DWORD style{};
//       DWORD exstyle{};
//       bitmap rendertarget{};
//       comptr<IDXGISwapChain1> swapchain{};

//       class handle : public ui::part_base::handle<core> {
//         friend struct core;
//         using ui::part_base::handle<core>::handle;

//       public:
//         const auto& hwnd() const { return _p->hwnd; }
//         const auto& frame_thickness() const { return _p->frame_thickness; }
//         const auto& pos() const { return _p->pos; }
//         const auto& style() const { return _p->style; }
//         const auto& exstyle() const { return _p->exstyle; }

//         const auto& size() const { return _p->size; }
//         auto& size(float2 Size) {
//           _p->size = Size;
//           _p->layout_changed = true;
//           return *this;
//         }
//       };

//       handle handle() noexcept { return *this; }

//       std::expected<void, error_trace> initialize_window() {
//         if (hwnd) return {};
//         if (auto res = wclass.initialize(); !res) return unexpected_error(res.error());
//         hwnd = ::CreateWindowExW(exstyle, wclass.name().data(), L"", style, 0, 0, 0, 0, 0, 0, wclass.hinstance(), 0);
//         if (!hwnd) return unexpected_win32_error("CreateWindowExW failed");
//         RECT wr, cr;
//         if (!::GetWindowRect(hwnd, &wr)) return unexpected_win32_error("GetWindowRect failed");
//         if (!::GetClientRect(hwnd, &cr)) return unexpected_win32_error("GetClientRect failed");
//         const auto left = (wr.right - wr.left - cr.right) / 2;
//         const auto top = wr.bottom - wr.top - cr.bottom - left;
//         frame_thickness = int4(left, top, left, left); // 決め打ちだが、例外はないはず
//         const auto sz = size + frame_thickness.xy() + frame_thickness.zw();
//         if (!::SetWindowPos(hwnd, nullptr, pos.x, pos.y, sz.x, sz.y, SWP_NOZORDER))
//           return unexpected_win32_error("SetWindowPos failed");
//         return {};
//       }

//       std::expected<void, error_trace> initialize_rendertarget() {
//         if (rendertarget) return {};
//         if (auto res = initialize_window(); !res) return unexpected_error(res.error());
//         if (auto res = d2d.initialize(); !res) return unexpected_error(res.error());
//         const auto usz = uint2(size);
//         rendertarget = {};
//         if (!swapchain) {
//           auto desc = DXGI_SWAP_CHAIN_DESC1(usz.x, usz.y, bitmap::dxgiformat, false, DXGI_SAMPLE_DESC(1, 0), {}, 2);
//           desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT, desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
//           const auto hr = dxgi.factory()->CreateSwapChainForHwnd(d3d.device(), hwnd, &desc, 0, 0, &swapchain.get());
//           if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateSwapChain failed", int(hr));
//           if (!swapchain) return unexpected_error(errors::operation_failed, "CreateSwapChain failed");
//         } else {
//           const auto hr = swapchain->ResizeBuffers(0, usz.x, usz.y, DXGI_FORMAT_UNKNOWN, 0);
//           if (FAILED(hr)) return unexpected_error(errors::operation_failed, "ResizeBuffers failed", int(hr));
//         }
//         if (auto res = bitmap::create(swapchain.get())) rendertarget = std::move(*res);
//         else return unexpected_error(res.error());
//         return {};
//       }

//       std::expected<void, error_trace> resize_window(std::optional<int2> Pos, std::optional<uint2> Size) {
//         if (auto res = initialize_window(); !res) return unexpected_error(res.error());
//         const auto [x, y] = Pos.value_or(int2());
//         const auto [w, h] = int2(Size.value_or(uint2()));
//         const auto op = SWP_NOZORDER | (Pos.has_value() ? SWP_NOMOVE : 0) | (Size.has_value() ? SWP_NOSIZE : 0);
//         if (!::SetWindowPos(hwnd, nullptr, x, y, w, h, op)) return unexpected_win32_error("SetWindowPos failed");
//         return {};
//       }

//       std::expected<void, error_trace> resize_rendertarget(uint2 Size) {
//         if (rendertarget.size() == Size) return {};
//         rendertarget = {};
//         size = Size;
//         if (auto res = initialize_rendertarget(); !res) return unexpected_error(res.error());
//         return {};
//       }
//     } core;

//     std::wstring title;
//     ui::slotid control_id;

//     bool visible = false;
//     bool enabled = false;
//     bool dirty = true;
//     bool messy = true;
//     bool manually_drawn = false;

//     function<bool, events::key> on_keydown;
//     function<bool, events::key> on_keyup;

//     //-- overrides --//

//     virtual const char* attachable() const override {
//       if (control_id) return "No more controls can be attached to this window";
//       else return nullptr;
//     }

//     virtual void attach(ui::slotid Child) override {
//       control_id = Child;
//       make_messy();
//     }

//     virtual void detach(ui::slotid Child) override {
//       if (control_id == Child) {
//         system::uis.erase(std::exchange(control_id, {}));
//         make_messy();
//       }
//     }

//     virtual void make_dirty() override { dirty = true; }
//     virtual void make_messy() override { messy = true; }
//   };
// };
// }

// // namespace yw::window {

// // class simple : public unknown {
// // public:
// //   struct slot : public unknown::slot {
// //     struct core : public ui::part_base {
// //       HWND hwnd{};
// //       int4 frame_thickness{};
// //       int2 pos{};
// //       int2 size{};
// //       DWORD style{};
// //       DWORD exstyle{};
// //       bitmap rendertarget{};
// //       comptr<IDXGISwapChain1> swapchain{};

// //       class handle : public ui::part_base::handle<core> {
// //         friend struct core;
// //         using ui::part_base::handle<core>::handle;

// //       public:
// //         const auto& hwnd() const { return _p->hwnd; }
// //         const auto& frame_thickness() const { return _p->frame_thickness; }
// //         const auto& pos() const { return _p->pos; }
// //         const auto& style() const { return _p->style; }
// //         const auto& exstyle() const { return _p->exstyle; }

// //         const auto& size() const { return _p->size; }
// //         auto& size(float2 Size) {
// //           _p->size = Size;
// //           _p->layout_changed = true;
// //           return *this;
// //         }
// //       };

// //       handle handle() noexcept { return *this; }

// //       std::expected<void, error_trace> initialize_window() {
// //         if (hwnd) return {};
// //         if (auto res = wclass.initialize(); !res) return unexpected_error(res.error());
// //         hwnd = ::CreateWindowExW(exstyle, wclass.name().data(), L"", style, 0, 0, 0, 0, 0, 0, wclass.hinstance(),
// 0);
// //         if (!hwnd) return unexpected_win32_error("CreateWindowExW failed");
// //         RECT wr, cr;
// //         if (!::GetWindowRect(hwnd, &wr)) return unexpected_win32_error("GetWindowRect failed");
// //         if (!::GetClientRect(hwnd, &cr)) return unexpected_win32_error("GetClientRect failed");
// //         const auto left = (wr.right - wr.left - cr.right) / 2;
// //         const auto top = wr.bottom - wr.top - cr.bottom - left;
// //         frame_thickness = int4(left, top, left, left); // 決め打ちだが、例外はないはず
// //         const auto sz = size + frame_thickness.xy() + frame_thickness.zw();
// //         if (!::SetWindowPos(hwnd, nullptr, pos.x, pos.y, sz.x, sz.y, SWP_NOZORDER))
// //           return unexpected_win32_error("SetWindowPos failed");
// //         return {};
// //       }

// //       std::expected<void, error_trace> initialize_rendertarget() {
// //         if (rendertarget) return {};
// //         if (auto res = initialize_window(); !res) return unexpected_error(res.error());
// //         if (auto res = d2d.initialize(); !res) return unexpected_error(res.error());
// //         const auto usz = uint2(size);
// //         rendertarget = {};
// //         if (!swapchain) {
// //           auto desc = DXGI_SWAP_CHAIN_DESC1(usz.x, usz.y, bitmap::dxgiformat, false, DXGI_SAMPLE_DESC(1, 0), {},
// 2);
// //           desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT, desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
// //           const auto hr = dxgi.factory()->CreateSwapChainForHwnd(d3d.device(), hwnd, &desc, 0, 0,
// &swapchain.get());
// //           if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateSwapChain failed", int(hr));
// //           if (!swapchain) return unexpected_error(errors::operation_failed, "CreateSwapChain failed");
// //         } else {
// //           const auto hr = swapchain->ResizeBuffers(0, usz.x, usz.y, DXGI_FORMAT_UNKNOWN, 0);
// //           if (FAILED(hr)) return unexpected_error(errors::operation_failed, "ResizeBuffers failed", int(hr));
// //         }
// //         if (auto res = bitmap::create(swapchain.get())) rendertarget = std::move(*res);
// //         else return unexpected_error(res.error());
// //         return {};
// //       }

// //       std::expected<void, error_trace> resize_window(std::optional<int2> Pos, std::optional<uint2> Size) {
// //         if (auto res = initialize_window(); !res) return unexpected_error(res.error());
// //         const auto [x, y] = Pos.value_or(int2());
// //         const auto [w, h] = int2(Size.value_or(uint2()));
// //         const auto op = SWP_NOZORDER | (Pos.has_value() ? SWP_NOMOVE : 0) | (Size.has_value() ? SWP_NOSIZE : 0);
// //         if (!::SetWindowPos(hwnd, nullptr, x, y, w, h, op)) return unexpected_win32_error("SetWindowPos failed");
// //         return {};
// //       }

// //       std::expected<void, error_trace> resize_rendertarget(uint2 Size) {
// //         if (rendertarget.size() == Size) return {};
// //         rendertarget = {};
// //         size = Size;
// //         if (auto res = initialize_rendertarget(); !res) return unexpected_error(res.error());
// //         return {};
// //       }
// //     } core;

// //     std::wstring title;
// //     ui::slotid control_id;

// //     bool visible = false;
// //     bool enabled = false;
// //     bool dirty = true;
// //     bool messy = true;
// //     bool manually_drawn = false;

// //     function<bool, events::key> on_keydown;
// //     function<bool, events::key> on_keyup;

// //     //-- overrides --//

// //     virtual const char* attachable() const override {
// //       if (control_id) return "No more controls can be attached to this window";
// //       else return nullptr;
// //     }

// //     virtual void attach(ui::slotid Child) override {
// //       control_id = Child;
// //       make_messy();
// //     }

// //     virtual void detach(ui::slotid Child) override {
// //       if (control_id == Child) {
// //         system::uis.erase(std::exchange(control_id, {}));
// //         make_messy();
// //       }
// //     }

// //     virtual void make_dirty() override { dirty = true; }
// //     virtual void make_messy() override { messy = true; }
// //   };

// //   void destroy() noexcept {
// //     if (const auto wsp = system::slot_address<simple>(_id)) ::DestroyWindow(wsp->core.hwnd);
// //   }

// //   virtual ~simple() noexcept override { destroy(); }
// //   explicit simple() noexcept = default;
// //   simple(simple&& other) noexcept { _id = std::exchange(other._id, {}); }
// //   simple& operator=(simple&& other) noexcept {
// //     if (this == &other) return *this;
// //     destroy();
// //     _id = std::exchange(other._id, {});
// //     return *this;
// //   }

// //   static std::expected<simple, error_trace> open(
// //     int2 Pos, uint2 Size, std::wstring Title, DWORD Style, DWORD ExStyle) {
// //     simple dw;
// //     const auto id = system::uis.add(std::make_unique<slot>());
// //     const auto wsp = system::slot_address<simple>(id);
// //     if (!wsp) return unexpected_error(errors::ui_invalid_slotid);
// //     wsp->id = wsp->layout_id = wsp->window_id = id;
// //     auto& core = wsp->core;
// //     core.pos = Pos, core.size = int2(Size), core.style = Style, core.exstyle = ExStyle;
// //     if (auto res = core.initialize_window(); !res) return unexpected_error(res.error());
// //     if (auto res = core.initialize_rendertarget(); !res) return unexpected_error(res.error());
// //     if (!Title.empty()) {
// //       ::SetWindowTextW(core.hwnd, Title.c_str());
// //       wsp->title = std::move(Title);
// //     }
// //     return dw;
// //   }
// // };
// // } // namespace yw::window

// // namespace yw {

// // class display_window : public unknown {
// // public:
// //   struct slot : public unknown::slot {
// //     struct core : public ui::part_base {
// //       HWND hwnd{};
// //       int4 frame_thickness{};
// //       int2 pos{};
// //       int2 size{};
// //       DWORD style{};
// //       DWORD exstyle{};
// //       bitmap rendertarget{};
// //       comptr<IDXGISwapChain1> swapchain{};

// //       class handle : public ui::part_base::handle<core> {
// //         friend struct core;
// //         using ui::part_base::handle<core>::handle;

// //       public:
// //         const auto& hwnd() const { return _p->hwnd; }
// //         const auto& frame_thickness() const { return _p->frame_thickness; }
// //         const auto& pos() const { return _p->pos; }
// //         const auto& style() const { return _p->style; }
// //         const auto& exstyle() const { return _p->exstyle; }

// //         const auto& size() const { return _p->size; }
// //         auto& size(float2 Size) {
// //           _p->size = Size;
// //           _p->layout_changed = true;
// //           return *this;
// //         }
// //       };

// //       handle handle() noexcept { return *this; }

// //       std::expected<void, error_trace> initialize_window() {
// //         if (hwnd) return {};
// //         if (auto res = wclass.initialize(); !res) return unexpected_error(res.error());
// //         hwnd = ::CreateWindowExW(exstyle, wclass.name().data(), L"", style, 0, 0, 0, 0, 0, 0, wclass.hinstance(),
// 0);
// //         if (!hwnd) return unexpected_win32_error("CreateWindowExW failed");
// //         RECT wr, cr;
// //         if (!::GetWindowRect(hwnd, &wr)) return unexpected_win32_error("GetWindowRect failed");
// //         if (!::GetClientRect(hwnd, &cr)) return unexpected_win32_error("GetClientRect failed");
// //         const auto left = (wr.right - wr.left - cr.right) / 2;
// //         const auto top = wr.bottom - wr.top - cr.bottom - left;
// //         frame_thickness = int4(left, top, left, left); // 決め打ちだが、例外はないはず
// //         const auto sz = size + frame_thickness.xy() + frame_thickness.zw();
// //         if (!::SetWindowPos(hwnd, nullptr, pos.x, pos.y, sz.x, sz.y, SWP_NOZORDER))
// //           return unexpected_win32_error("SetWindowPos failed");
// //         return {};
// //       }

// //       std::expected<void, error_trace> initialize_rendertarget() {
// //         if (rendertarget) return {};
// //         if (auto res = initialize_window(); !res) return unexpected_error(res.error());
// //         if (auto res = d2d.initialize(); !res) return unexpected_error(res.error());
// //         const auto usz = uint2(size);
// //         rendertarget = {};
// //         if (!swapchain) {
// //           auto desc = DXGI_SWAP_CHAIN_DESC1(usz.x, usz.y, bitmap::dxgiformat, false, DXGI_SAMPLE_DESC(1, 0), {},
// 2);
// //           desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT, desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
// //           const auto hr = dxgi.factory()->CreateSwapChainForHwnd(d3d.device(), hwnd, &desc, 0, 0,
// &swapchain.get());
// //           if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateSwapChain failed", int(hr));
// //           if (!swapchain) return unexpected_error(errors::operation_failed, "CreateSwapChain failed");
// //         } else {
// //           const auto hr = swapchain->ResizeBuffers(0, usz.x, usz.y, DXGI_FORMAT_UNKNOWN, 0);
// //           if (FAILED(hr)) return unexpected_error(errors::operation_failed, "ResizeBuffers failed", int(hr));
// //         }
// //         if (auto res = bitmap::create(swapchain.get())) rendertarget = std::move(*res);
// //         else return unexpected_error(res.error());
// //         return {};
// //       }

// //       std::expected<void, error_trace> resize_window(std::optional<int2> Pos, std::optional<uint2> Size) {
// //         if (auto res = initialize_window(); !res) return unexpected_error(res.error());
// //         const auto [x, y] = Pos.value_or(int2());
// //         const auto [w, h] = int2(Size.value_or(uint2()));
// //         const auto op = SWP_NOZORDER | (Pos.has_value() ? SWP_NOMOVE : 0) | (Size.has_value() ? SWP_NOSIZE : 0);
// //         if (!::SetWindowPos(hwnd, nullptr, x, y, w, h, op)) return unexpected_win32_error("SetWindowPos failed");
// //         return {};
// //       }

// //       std::expected<void, error_trace> resize_rendertarget(uint2 Size) {
// //         if (rendertarget.size() == Size) return {};
// //         rendertarget = {};
// //         size = Size;
// //         if (auto res = initialize_rendertarget(); !res) return unexpected_error(res.error());
// //         return {};
// //       }
// //     } core;

// //     std::wstring title;
// //     ui::slotid control_id;

// //     bool visible = false;
// //     bool enabled = false;
// //     bool dirty = true;
// //     bool messy = true;
// //     bool manually_drawn = false;

// //     function<bool, events::key> on_keydown;
// //     function<bool, events::key> on_keyup;

// //     //-- overrides --//

// //     virtual const char* attachable() const override {
// //       if (control_id) return "No more controls can be attached to this window";
// //       else return nullptr;
// //     }

// //     virtual void attach(ui::slotid Child) override {
// //       control_id = Child;
// //       make_messy();
// //     }

// //     virtual void detach(ui::slotid Child) override {
// //       if (control_id == Child) {
// //         system::uis.erase(std::exchange(control_id, {}));
// //         make_messy();
// //       }
// //     }

// //     virtual void make_dirty() override { dirty = true; }
// //     virtual void make_messy() override { messy = true; }
// //   };

// //   void destroy() noexcept {
// //     if (const auto wsp = system::slot_address<display_window>(_id)) ::DestroyWindow(wsp->core.hwnd);
// //   }

// //   virtual ~display_window() noexcept override { destroy(); }
// //   explicit display_window() noexcept = default;
// //   display_window(display_window&& other) noexcept { _id = std::exchange(other._id, {}); }
// //   display_window& operator=(display_window&& other) noexcept {
// //     if (this == &other) return *this;
// //     destroy();
// //     _id = std::exchange(other._id, {});
// //     return *this;
// //   }

// //   static std::expected<display_window, error_trace> open(
// //     int2 Pos, uint2 Size, std::wstring Title, DWORD Style, DWORD ExStyle) {
// //     display_window dw;
// //     const auto id = system::uis.add(std::make_unique<slot>());
// //     const auto wsp = system::slot_address<display_window>(id);
// //     if (!wsp) return unexpected_error(errors::ui_invalid_slotid);
// //     wsp->id = wsp->layout_id = wsp->window_id = id;
// //     auto& core = wsp->core;
// //     core.pos = Pos, core.size = int2(Size), core.style = Style, core.exstyle = ExStyle;
// //     if (auto res = core.initialize_window(); !res) return unexpected_error(res.error());
// //     if (auto res = core.initialize_rendertarget(); !res) return unexpected_error(res.error());
// //     if (!Title.empty()) {
// //       ::SetWindowTextW(core.hwnd, Title.c_str());
// //       wsp->title = std::move(Title);
// //     }
// //     return dw;
// //   }
// // };

// // class window : public unknown {
// // public:
// //   enum class appearance : uint32_t {
// //     unknown,
// //     regular = WS_OVERLAPPEDWINDOW,
// //     size_fixed = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
// //     borderless = WS_POPUP
// //   };

// //   struct options {
// //     std::optional<int2> pos = {};
// //     std::optional<uint2> size = {};
// //     std::optional<std::wstring> title = {};
// //     appearance appearance = window::appearance::regular;
// //     bool show = true;
// //   };

// //   struct slot : public unknown::slot {
// //     HWND hwnd{};
// //     int4 frame_thickness{};
// //     int2 pos{};
// //     int2 size{};
// //     DWORD style{};
// //     DWORD ex_style{};
// //     std::wstring title;
// //     bitmap rendertarget{};
// //     comptr<IDXGISwapChain1> swapchain{};
// //     bool dirty = true, messy = true, manually_drawn = false;
// //     bool visible = false, active = false;

// //     ui::parts::background background;
// //     ui::parts::focus_ring focus_ring;

// //     bitmap ui_bitmap{};
// //     ui::slotid control_id{};
// //     bool resizing = false;

// //     ui::slotid focused_control{};
// //     ui::slotid hovered_control{};
// //     ui::slotid captured_control{};
// //     bool tracking = false;

// //     function<bool> on_close;
// //     function<void, events::key> on_keydown;
// //     function<void, events::key> on_keyup;

// //     std::expected<void, error_trace> resize_rendertarget() {
// //       if (bool(rendertarget) && size == rendertarget.size()) return {};
// //       if (size.x == 0 || size.y == 0) return {};
// //       const auto usz = uint2(size);
// //       rendertarget = {};
// //       if (!swapchain) {
// //         auto desc = DXGI_SWAP_CHAIN_DESC1(usz.x, usz.y, bitmap::dxgiformat, false, DXGI_SAMPLE_DESC(1, 0), {}, 2);
// //         desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT, desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
// //         const auto hr = dxgi.factory()->CreateSwapChainForHwnd(d3d.device(), hwnd, &desc, 0, 0, &swapchain.get());
// //         if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateSwapChain failed", int(hr));
// //         if (!swapchain) return unexpected_error(errors::operation_failed, "CreateSwapChain failed");
// //       } else {
// //         const auto hr = swapchain->ResizeBuffers(0, usz.x, usz.y, DXGI_FORMAT_UNKNOWN, 0);
// //         if (FAILED(hr)) return unexpected_error(errors::operation_failed, "ResizeBuffers failed", int(hr));
// //       }
// //       if (auto res = bitmap::create(swapchain.get())) rendertarget = std::move(*res);
// //       else return unexpected_error(res.error());
// //       manually_drawn = false;
// //       messy = true;
// //       return {};
// //     }

// //     std::expected<void, error_trace> resize(uint2 Size) {
// //       if (size == Size) return {};
// //       const auto area = int2(Size) + frame_thickness.xy() + frame_thickness.zw();
// //       if (!::SetWindowPos(hwnd, nullptr, 0, 0, area.x, area.y, SWP_NOMOVE | SWP_NOACTIVATE))
// //         return unexpected_win32_error("SetWindowPos failed");
// //       /// \note SetWindowPos によって WM_SIZE が発生する。
// //       /// \note size 更新や resize_rendertarget は WM_SIZE 内で行う。(GUIでのサイズ変更と合わせるため)
// //       return {};
// //     }

// //     std::expected<void, error_trace> calculate_frame_thickness() {
// //       RECT wr, cr;
// //       if (!::GetWindowRect(hwnd, &wr)) return unexpected_win32_error("GetWindowRect failed");
// //       if (!::GetClientRect(hwnd, &cr)) return unexpected_win32_error("GetClientRect failed");
// //       const auto left = (wr.right - wr.left - cr.right) / 2;
// //       const auto top = wr.bottom - wr.top - cr.bottom - left;
// //       frame_thickness = int4(left, top, left, left); // 決め打ちだが、例外はないはず
// //       size = int2(cr.right, cr.bottom);
// //       pos = int2(wr.left, wr.top);
// //       return {};
// //     }

// //     std::expected<void, error_trace> create_window() {
// //       if (hwnd) return {};
// //       if (auto res = wclass.initialize(); !res) return unexpected_error(res.error());
// //       if (auto res = dxgi.initialize(); !res) return unexpected_error(res.error());
// //       hwnd = ::CreateWindowExW(
// //         ex_style, wclass.name().data(), title.data(), style, pos.x, pos.y, size.x, size.y, 0, 0,
// wclass.hinstance(),
// //         0);
// //       if (!hwnd) return unexpected_win32_error("CreateWindowExW failed");
// //       ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, std::bit_cast<LONG_PTR>(window_id));
// //       return {};
// //     }

// //     std::expected<void, error_trace> initialize(const options& Options) {
// //       switch (Options.appearance) {
// //       case appearance::regular:
// //       case appearance::size_fixed:
// //       case appearance::borderless: break;
// //       default: return unexpected_error(errors::invalid_argument, "invalid appearance");
// //       }
// //       ex_style = WS_EX_ACCEPTFILES;
// //       style = DWORD(Options.appearance);
// //       if (!Options.title) {
// //         std::wstring t(MAX_PATH, L'\0');
// //         if (const auto n = ::GetModuleFileNameW(nullptr, t.data(), MAX_PATH); n > 0) t.resize(n);
// //         else return unexpected_error(errors::operation_failed, "GetModuleFileNameW failed");
// //         title = std::filesystem::path(std::move(t)).stem().native();
// //       } else title = std::move(*Options.title);
// //       pos = Options.pos.value_or(int2(CW_USEDEFAULT, CW_USEDEFAULT));
// //       size = Options.size.value_or(uint2(ui::arbitrary_value, ui::arbitrary_value));
// //       if (auto res = create_window(); !res) return unexpected_error(res.error());
// //       if (auto res = calculate_frame_thickness(); !res) return unexpected_error(res.error());
// //       if (Options.size.has_value() && *Options.size != size)
// //         if (auto res = resize(*Options.size); !res) return unexpected_error(res.error());
// //       if (Options.show) ::ShowWindow(hwnd, SW_SHOW);
// //       return {};
// //     }

// //     std::expected<void, error_trace> update_ui_layout() {
// //       if (!messy) return {};
// //       if (const auto csp = system::slot_address<ui::control>(control_id)) {
// //         csp->ensure_minimum_size();
// //         const auto minimum_area = csp->core.area();
// //         const auto available_area = vapply_r<float2>(yw::max, float2(size), minimum_area);
// //         csp->update_layout({}, available_area);
// //         if (size.x < minimum_area.x || size.y < minimum_area.y) {
// //           size = vapply_r<int2>(yw::max, size, vapply_r<int2>(yw::ceil, minimum_area));
// //           const auto area = size + frame_thickness.xy() + frame_thickness.zw();
// //           if (!::SetWindowPos(hwnd, nullptr, 0, 0, area.x, area.y, SWP_NOZORDER | SWP_NOMOVE))
// //             return unexpected_win32_error("SetWindowPos failed");
// //         }
// //       }
// //       messy = false;
// //       dirty = true;
// //       return {};
// //     }

// //     std::expected<void, error_trace> draw_ui_layout() {
// //       if (!dirty) return {};
// //       if (const auto csp = system::slot_address<ui::control>(control_id)) {
// //         if (!csp) return unexpected_error(errors::operation_failed, "Failed to get control slot address");
// //         if (!ui_bitmap || ui_bitmap.size() != size) {
// //           if (auto res = bitmap::create(size)) ui_bitmap = std::move(*res);
// //           else return unexpected_error(res.error());
// //         }
// //         if (auto d = ui_bitmap.begin_draw(colors::transparent)) csp->draw();
// //         else return unexpected_error(d.error());
// //       }
// //       /// \note dirty フラグのリセットは draw 関数まで待つ。
// //       return {};
// //     }

// //     std::expected<void, error_trace> draw() {
// //       if (!dirty) return {};
// //       if (!rendertarget) {
// //         if (auto res = bitmap::create(swapchain.get())) rendertarget = std::move(*res);
// //         else return unexpected_error(res.error());
// //       }
// //       drawing d;
// //       if (!manually_drawn) {
// //         if (auto res = rendertarget.begin_draw(background.color)) d = std::move(*res);
// //         else return unexpected_error(res.error());
// //         if (background.image) draw_bitmap({}, size, background.image, background.image_opacity);
// //       } else if (auto res = rendertarget.begin_draw()) d = std::move(*res);
// //       else return unexpected_error(res.error());
// //       draw_bitmap({}, ui_bitmap);
// //       d.close();
// //       swapchain->Present(0, 0);
// //       manually_drawn = false;
// //       dirty = false;
// //       return {};
// //     }

// //     //-- override functions --//

// //     virtual const char* attachable() const override {
// //       if (control_id) return "No more controls can be attached to this window";
// //       else return nullptr;
// //     }

// //     virtual void attach(ui::slotid Child) override {
// //       control_id = Child;
// //       make_messy();
// //     }
// //     virtual void detach(ui::slotid Child) override {
// //       if (control_id == Child) {
// //         system::uis.erase(std::exchange(control_id, {}));
// //         make_messy();
// //       }
// //     }

// //     void make_dirty() override { dirty = true; }
// //     void make_moved() override { dirty = true; }
// //     void make_messy() override { messy = true; }
// //   };

// //   void destroy() noexcept {
// //     if (const auto wsp = system::slot_address<window>(_id)) ::DestroyWindow(wsp->hwnd);
// //   }

// //   virtual ~window() noexcept override { destroy(); }
// //   explicit window() noexcept = default;
// //   window(window&& other) noexcept { _id = std::exchange(other._id, {}); }
// //   window& operator=(window&& other) noexcept {
// //     if (this == &other) return *this;
// //     destroy();
// //     _id = std::exchange(other._id, {});
// //     return *this;
// //   }

// //   static std::expected<window, error_trace> open(options Options) {
// //     window w{};
// //     const auto id = system::uis.add(std::make_unique<slot>());
// //     const auto wsp = system::slot_address<window>(id);
// //     if (!wsp) return unexpected_error(errors::operation_failed, "Failed to create window slot");
// //     wsp->id = id;
// //     wsp->layout_id = id;
// //     wsp->window_id = id;
// //     if (auto res = wsp->initialize(Options); !res) return unexpected_error(res.error());
// //     w._id = id;
// //     system::primal_windows.push_back(id);
// //     if (Options.show) ::ShowWindow(wsp->hwnd, SW_SHOW);
// //     return w;
// //   }

// //   explicit operator bool() const noexcept { return system::uis.contains(_id); }

// //   auto& show() {
// //     if (auto wsp = system::slot_address<window>(_id)) {
// //       ::ShowWindow(wsp->hwnd, SW_SHOW);
// //     } else fatal_error(errors::ui_invalid_slotid);
// //     return *this;
// //   }

// //   auto background() {
// //     const auto wsp = system::slot_address<window>(_id);
// //     if (!wsp) fatal_error(errors::ui_invalid_slotid);
// //     return wsp->background.handle();
// //   }
// //   const auto background() const {
// //     const auto wsp = system::slot_address<window>(_id);
// //     if (!wsp) fatal_error(errors::ui_invalid_slotid);
// //     return wsp->background.handle();
// //   }

// //   const auto& on_close() const {
// //     const auto wsp = system::slot_address<window>(_id);
// //     if (!wsp) fatal_error(errors::ui_invalid_slotid);
// //     return wsp->on_close;
// //   }
// //   std::expected<void, error_trace> on_close(function<bool> OnClose) const {
// //     if (auto wsp = system::slot_address<window>(_id)) {
// //       wsp->on_close = std::move(OnClose);
// //       return {};
// //     } else return unexpected_error(errors::ui_invalid_slotid);
// //   }

// //   const auto& on_keydown() const {
// //     const auto wsp = system::slot_address<window>(_id);
// //     if (!wsp) fatal_error(errors::ui_invalid_slotid);
// //     return wsp->on_keydown;
// //   }
// //   std::expected<void, error_trace> on_keydown(function<void, events::key> OnKeyDown) const {
// //     if (auto wsp = system::slot_address<window>(_id)) {
// //       wsp->on_keydown = std::move(OnKeyDown);
// //       return {};
// //     } else return unexpected_error(errors::ui_invalid_slotid);
// //   }

// //   const auto& on_keyup() const {
// //     const auto wsp = system::slot_address<window>(_id);
// //     if (!wsp) fatal_error(errors::ui_invalid_slotid);
// //     return wsp->on_keyup;
// //   }
// //   std::expected<void, error_trace> on_keyup(function<void, events::key> OnKeyUp) const {
// //     if (auto wsp = system::slot_address<window>(_id)) {
// //       wsp->on_keyup = std::move(OnKeyUp);
// //       return {};
// //     } else return unexpected_error(errors::ui_invalid_slotid);
// //   }

// //   // const auto& commands() const { return unsafe_get(&slot::commands); }
// //   // auto& commands() { return unsafe_get(&slot::commands); }

// //   std::expected<drawing, error_trace> begin_draw() {
// //     if (const auto wsp = system::slot_address<window>(_id)) {
// //       wsp->dirty = true;
// //       wsp->manually_drawn = true;
// //       if (auto d = wsp->rendertarget.begin_draw(wsp->background.color)) {
// //         if (wsp->background.image) draw_bitmap({}, wsp->size, wsp->background.image,
// wsp->background.image_opacity);
// //         return std::move(d);
// //       } else return unexpected_error(d.error());
// //     } else return unexpected_error(errors::ui_invalid_slotid);
// //   }

// //   /// \note コールバック関数内で使用しても機能しない。フラグを立て、ループ内で実行すること。
// //   std::expected<void, error_trace> screenshot(const std::filesystem::path& PngPath, bool WriteUI = true) {
// //     const auto wsp = system::slot_address<window>(_id);
// //     if (!wsp) return unexpected_error(errors::ui_invalid_slotid);
// //     drawing d;
// //     if (!wsp->manually_drawn) {
// //       if (auto res = wsp->rendertarget.begin_draw(wsp->background.color)) d = std::move(*res);
// //       else return unexpected_error(res.error());
// //       if (wsp->background.image) // 背景画像の表示サイズは拡大縮小する必要がある
// //         draw_bitmap({}, wsp->size, wsp->background.image, wsp->background.image_opacity);
// //     } else if (auto res = wsp->rendertarget.begin_draw()) d = std::move(*res);
// //     else return unexpected_error(res.error());
// //     wsp->manually_drawn = false;
// //     if (WriteUI) {
// //       wsp->draw_ui_layout();
// //       draw_bitmap({}, wsp->ui_bitmap); // UIレイヤーのサイズはレンダーターゲットと同じ
// //       wsp->dirty = false;
// //     }
// //     d.close();
// //     if (auto res = wsp->rendertarget.save_as_png(PngPath); !res) return unexpected_error(res.error());
// //     return {};
// //   }
// // };

// // inline void ui::control::slot::hover_event(events::hover Event) {
// //   if (enabled && on_hover) on_hover(Event);
// //   if (tooltip.empty()) return;
// //   if (Event.enter()) {
// //     if (const auto w = system::slot_address<window>(window_id))
// //       system::tooltip.show(core.pos + w->pos + w->frame_thickness.xy(), core.size, tooltip);
// //   } else if (Event.leave()) system::tooltip.hide();
// // }
// // } // namespace yw
