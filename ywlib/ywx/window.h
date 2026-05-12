#pragma once
#include "ywx/ui_parts.h"

namespace yw {

class window : public unknown {
public:
  enum class appearance : uint32_t {
    unknown,
    regular = WS_OVERLAPPEDWINDOW,
    size_fixed = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
    borderless = WS_POPUP
  };

  struct options {
    std::optional<int2> pos = {};
    std::optional<uint2> size = {};
    std::optional<std::wstring> title = {};
    appearance appearance = window::appearance::regular;
    bool show = true;
  };

  struct slot : public unknown::slot {
    struct core {
      ui::slotid window_id{}; // [in]
      HWND hwnd{};
      int2 pos{};
      int2 size{};
      int4 frame_thickness{};
      DWORD style{};    // [in]
      DWORD ex_style{}; // [in]
      std::wstring title;
      bitmap rendertarget{};
      comptr<IDXGISwapChain1> swapchain{};
      bool dirty = true, messy = true, manually_drawn = false;

      int2 area() const noexcept { return size + frame_thickness.xy() + frame_thickness.zw(); }

      /// `owner_window_id`, `style`, and `ex_style` must be set before calling this function.
      std::expected<void, error_trace> initialize(
        std::optional<int2> Pos, std::optional<uint2> Size, null_terminated<wchar_t> Title) {
        if (hwnd) return {};
        if (auto res = wclass.initialize(); !res) return unexpected_error(res.error());
        if (auto res = dxgi.initialize(); !res) return unexpected_error(res.error());
        bool centering_flag = Size.has_value() && !Pos;
        if (!Pos) Pos = int2(CW_USEDEFAULT, CW_USEDEFAULT);
        size = int2(Size.value_or(uint2(ui::arbitrary_value, ui::arbitrary_value)));
        hwnd = ::CreateWindowExW(
          ex_style, wclass.name().c_str(), Title.data(), style, Pos->x, Pos->y, size.x, size.y, 0, 0,
          wclass.hinstance(), 0);
        if (!hwnd) return unexpected_win32_error("CreateWindowExW failed");
        ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, std::bit_cast<LONG_PTR>(window_id));
        RECT wr, cr;
        if (!::GetClientRect(hwnd, &cr)) return unexpected_win32_error("GetClientRect failed");
        if (!::GetWindowRect(hwnd, &wr)) return unexpected_win32_error("GetWindowRect failed");
        const auto left = (wr.right - wr.left - cr.right) / 2;
        const auto top = wr.bottom - wr.top - cr.bottom - left;
        pos = int2(wr.left, wr.top);
        if (size.x < cr.right) size.x = cr.right;
        if (size.y < cr.bottom) size.y = cr.bottom;
        frame_thickness = int4(left, top, left, left);
        auto desc = DXGI_SWAP_CHAIN_DESC1(size.x, size.y, bitmap::dxgiformat, false, DXGI_SAMPLE_DESC(1, 0), {}, 2);
        desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT, desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        const auto hr = dxgi.factory()->CreateSwapChainForHwnd(d3d.device(), hwnd, &desc, 0, 0, &swapchain.get());
        if (FAILED(hr)) return unexpected_error(errors::operation_failed, "Failed to create swapchain", int(hr));
        const auto a = area();
        if (!::SetWindowPos(hwnd, nullptr, 0, 0, a.x, a.y, SWP_NOMOVE | SWP_NOACTIVATE))
          return unexpected_win32_error("SetWindowPos failed");
        return {};
      }

      std::expected<void, error_trace> locate(int2 Pos) {
        if (!::SetWindowPos(hwnd, nullptr, Pos.x, Pos.y, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE))
          return unexpected_win32_error("SetWindowPos failed");
        pos = Pos;
        return {};
      }

      std::expected<void, error_trace> resize_rt() {
        if (size == int2()) return {};
        bool need_update_rt = !rendertarget;
        if (size != rendertarget.size()) {
          rendertarget = {};
          const auto hr = swapchain->ResizeBuffers(0, size.x, size.y, DXGI_FORMAT_UNKNOWN, 0);
          if (FAILED(hr)) return unexpected_error(errors::operation_failed, "ResizeBuffers failed", int(hr));
          need_update_rt = true;
        }
        if (need_update_rt) {
          if (auto res = bitmap::create(swapchain.get())) rendertarget = std::move(*res);
          else return unexpected_error(res.error());
        }
        messy = true;
        manually_drawn = false;
        return {};
      }

      std::expected<void, error_trace> resize(uint2 Size) {
        if (Size != size && Size != int2()) {
          const auto a = int2(Size) + frame_thickness.xy() + frame_thickness.zw();
          if (!::SetWindowPos(hwnd, nullptr, 0, 0, a.x, a.y, SWP_NOMOVE | SWP_NOACTIVATE))
            return unexpected_win32_error("SetWindowPos failed");
        }
        return {};
      }

      class handle {
        friend struct core;
        core* _p = nullptr;
        handle(core& Ref) : _p(&Ref) {}

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
    } core;

    struct focus_ring {
      ui::slotid window_id{};
      yw::color color = yw::color(0.0f, 0.0f, 1.0f, 0.5f);
      float offset = 3.0f;
      float width = 1.0f;
      bool view_changed = false;

      class handle {
        friend struct focus_ring;
        focus_ring* _p = nullptr;
        handle(focus_ring& Ref) : _p(&Ref) {}

      public:
        ~handle() noexcept {
          if (_p && _p->view_changed)
            if (const auto csp = system::slot_address<window>(_p->window_id)) csp->make_dirty();
        }

        handle(handle&& Other) noexcept : _p(std::exchange(Other._p, nullptr)) {}
        handle& operator=(handle&& Other) noexcept {
          if (this != &Other) _p = std::exchange(Other._p, nullptr);
          return *this;
        }

        const auto& color() const { return _p->color; }
        auto& color(yw::color Color) { return _p->color = Color, _p->view_changed = true, *this; }

        const auto& offset() const { return _p->offset; }
        auto& offset(float1 Offset) { return _p->offset = Offset.x, _p->view_changed = true, *this; }

        const auto& width() const { return _p->width; }
        auto& width(float1 Width) { return _p->width = Width.x, _p->view_changed = true, *this; }
      };

      handle handle() noexcept { return *this; }
    } focus_ring;

    ui::part::background background;

    bitmap ui_bitmap{};
    ui::slotid control_id{};
    bool resizing = false;

    ui::slotid focused_control{};
    ui::slotid hovered_control{};
    ui::slotid captured_control{};
    bool tracking = false;

    function<bool> on_close;
    function<void, event::key> on_keydown;
    function<void, event::key> on_keyup;

    std::expected<void, error_trace> initialize(
      ui::slotid Id, std::optional<int2> Pos, std::optional<int2> Size, //
      null_terminated<wchar_t> Title, window::appearance Style) {
      id = Id;
      core.owner_window_id = Id;
      core.style = DWORD(Style);
      core.ex_style = WS_EX_ACCEPTFILES;
      background.owner_window_id = Id;
      focus_ring.owner_window_id = Id;
      if (auto res = core.initialize(Pos, Size, Title); !res) return unexpected_error(res.error());
      return {};
    }

    std::expected<void, error_trace> update_ui_layout() {
      if (!core.messy) return {};
      if (const auto csp = system::slot_address<ui::control>(control_id)) {
        csp->ensure_minimum_size();
        csp->update_layout({}, csp->core.size);
        if (core.size.x < csp->core.size.x || core.size.y < csp->core.size.y) {
          core.size = vapply_r<int2>(yw::max, core.size, csp->core.size);
          if (!::SetWindowPos(core.hwnd, nullptr, 0, 0, core.size.x, core.size.y, SWP_NOZORDER | SWP_NOMOVE))
            return unexpected_win32_error("SetWindowPos failed");
        }
      }
      core.messy = false;
      core.dirty = true;
      return {};
    }

    std::expected<void, error_trace> draw_ui_layout() {
      if (!core.dirty) return {};
      if (const auto csp = system::slot_address<ui::control>(control_id)) {
        if (!csp) return unexpected_error(errors::operation_failed, "Failed to get control slot address");
        if (!ui_bitmap || ui_bitmap.size() != core.size) {
          if (auto res = bitmap::create(core.size)) ui_bitmap = std::move(*res);
          else return unexpected_error(res.error());
        }
        if (auto d = ui_bitmap.begin_draw(colors::transparent)) csp->draw();
        else return unexpected_error(d.error());
      }
      return {};
    }

    std::expected<void, error_trace> draw() {
      if (!core.dirty) return {};
      if (!core.rendertarget) {
        if (auto res = bitmap::create(core.swapchain.get())) core.rendertarget = std::move(*res);
        else return unexpected_error(res.error());
      }
      drawing d;
      if (!core.manually_drawn) {
        if (auto res = core.rendertarget.begin_draw(background.color)) d = std::move(*res);
        else return unexpected_error(res.error());
        if (background.image) draw_bitmap({}, core.size, background.image, background.image_opacity);
      } else if (auto res = core.rendertarget.begin_draw()) d = std::move(*res);
      else return unexpected_error(res.error());
      draw_bitmap({}, ui_bitmap);
      d.close();
      core.swapchain->Present(0, 0);
      core.manually_drawn = false;
      core.dirty = false;
      return {};
    }

    virtual bool attach_child(ui::slotid Child) override {
      if (control_id) return false;
      control_id = Child;
      return true;
    }

    virtual void detach_child(ui::slotid Child) override { control_id = {}; }

    void next_tab_stop(bool Forward) {
      if (const auto csp = system::slot_address<ui::control>(control_id)) {
        bool found = !focused_control;
        focused_control = csp->next_tab_stop(focused_control, Forward, found);
      }
    }
  };

  void destroy() noexcept {
    if (const auto wsp = system::slot_address<window>(_id)) ::DestroyWindow(wsp->core.hwnd);
  }

  virtual ~window() noexcept override { destroy(); }
  explicit window() noexcept = default;
  window(window&& other) noexcept { _id = std::exchange(other._id, {}); }
  window& operator=(window&& other) noexcept {
    if (this == &other) return *this;
    destroy();
    _id = std::exchange(other._id, {});
    return *this;
  }

  static std::expected<window, error_trace> open(options Options) {
    switch (Options.appearance) {
    case appearance::regular:
    case appearance::size_fixed:
    case appearance::borderless: break;
    default: return unexpected_error(errors::invalid_argument, "Invalid window appearance");
    }
    if (!Options.title) {
      std::wstring t(MAX_PATH, L'\0');
      if (const auto n = ::GetModuleFileNameW(nullptr, t.data(), MAX_PATH); n > 0) t.resize(n);
      else return unexpected_error(errors::operation_failed, "GetModuleFileNameW failed");
      Options.title = std::filesystem::path(std::move(t)).stem().native();
    }
    window w{};
    const auto id = system::uis.add(std::make_unique<slot>());
    const auto wsp = system::slot_address<window>(id);
    if (!wsp) return unexpected_error(errors::operation_failed, "Failed to create window slot");
    if (auto res = wsp->initialize(id, Options.pos, Options.size, *Options.title, Options.appearance); !res)
      return unexpected_error(res.error());
    w._id = id;
    system::primal_windows.push_back(id);
    if (Options.show) ::ShowWindow(wsp->core.hwnd, SW_SHOW);
    return w;
  }

  explicit operator bool() const noexcept { return system::uis.contains(_id); }

  auto& show() {
    if (auto wsp = system::slot_address<window>(_id)) {
      ::ShowWindow(wsp->core.hwnd, SW_SHOW);
    } else fatal_error(errors::operation_failed, "Failed to access window slot.");
    return *this;
  }

  auto core() {
    if (auto wsp = system::slot_address<window>(_id); !wsp)
      fatal_error(errors::operation_failed, "Failed to access window slot.");
    else return wsp->core.handle();
  }
  const auto core() const {
    if (auto wsp = system::slot_address<window>(_id); !wsp)
      fatal_error(errors::operation_failed, "Failed to access window slot.");
    else return wsp->core.handle();
  }

  auto background() {
    const auto wsp = system::slot_address<window>(_id);
    if (!wsp) fatal_error(errors::operation_failed, "Failed to access window slot.");
    return wsp->background.handle();
  }
  const auto background() const {
    const auto wsp = system::slot_address<window>(_id);
    if (!wsp) fatal_error(errors::operation_failed, "Failed to access window slot.");
    return wsp->background.handle();
  }

  const auto& on_close() const {
    const auto wsp = system::slot_address<window>(_id);
    if (!wsp) fatal_error(errors::operation_failed, "Failed to access window slot.");
    return wsp->on_close;
  }
  std::expected<void, error_trace> on_close(function<bool> OnClose) const {
    if (auto wsp = system::slot_address<window>(_id)) {
      wsp->on_close = std::move(OnClose);
      return {};
    } else return unexpected_error(errors::operation_failed, "Failed to access window slot.");
  }

  const auto& on_keydown() const {
    const auto wsp = system::slot_address<window>(_id);
    if (!wsp) fatal_error(errors::operation_failed, "Failed to access window slot.");
    return wsp->on_keydown;
  }
  std::expected<void, error_trace> on_keydown(function<void, event::key> OnKeyDown) const {
    if (auto wsp = system::slot_address<window>(_id)) {
      wsp->on_keydown = std::move(OnKeyDown);
      return {};
    } else return unexpected_error(errors::operation_failed, "Failed to access window slot.");
  }

  const auto& on_keyup() const {
    const auto wsp = system::slot_address<window>(_id);
    if (!wsp) fatal_error(errors::operation_failed, "Failed to access window slot.");
    return wsp->on_keyup;
  }
  std::expected<void, error_trace> on_keyup(function<void, event::key> OnKeyUp) const {
    if (auto wsp = system::slot_address<window>(_id)) {
      wsp->on_keyup = std::move(OnKeyUp);
      return {};
    } else return unexpected_error(errors::operation_failed, "Failed to access window slot.");
  }

  // const auto& commands() const { return unsafe_get(&slot::commands); }
  // auto& commands() { return unsafe_get(&slot::commands); }

  std::expected<drawing, error_trace> begin_draw() {
    if (const auto wsp = system::slot_address<window>(_id)) {
      wsp->core.dirty = true;
      wsp->core.manually_drawn = true;
      if (auto d = wsp->core.rendertarget.begin_draw(wsp->background.color)) {
        if (wsp->background.image)
          draw_bitmap({}, wsp->core.size, wsp->background.image, wsp->background.image_opacity);
        return std::move(d);
      } else return unexpected_error(d.error());
    } else return unexpected_error(errors::operation_failed, "Failed to access window slot.");
  }

  /// \note コールバック関数内で使用しても機能しない。フラグを立て、ループ内で実行すること。
  std::expected<void, error_trace> screenshot(const std::filesystem::path& PngPath, bool WriteUI = true) {
    const auto wsp = system::slot_address<window>(_id);
    if (!wsp) return unexpected_error(errors::operation_failed, "Failed to access window slot.");
    drawing d;
    if (!wsp->core.manually_drawn) {
      if (auto res = wsp->core.rendertarget.begin_draw(wsp->background.color)) d = std::move(*res);
      else return unexpected_error(res.error());
      if (wsp->background.image) // 背景画像の表示サイズは拡大縮小する必要がある
        draw_bitmap({}, wsp->core.size, wsp->background.image, wsp->background.image_opacity);
    } else if (auto res = wsp->core.rendertarget.begin_draw()) d = std::move(*res);
    else return unexpected_error(res.error());
    wsp->core.manually_drawn = false;
    if (WriteUI) {
      wsp->draw_ui_layout();
      draw_bitmap({}, wsp->ui_bitmap); // UIレイヤーのサイズはレンダーターゲットと同じ
      wsp->core.dirty = false;
    }
    d.close();
    if (auto res = wsp->core.rendertarget.save_as_png(PngPath); !res) return unexpected_error(res.error());
    return {};
  }
};

void ui::make_dirty(ui::slotid Window) noexcept {
  if (auto wsp = system::slot_address<window>(Window)) wsp->core.dirty = true;
}

void ui::make_messy(ui::slotid Window) noexcept {
  if (auto wsp = system::slot_address<window>(Window)) wsp->core.dirty = false;
}
} // namespace yw
