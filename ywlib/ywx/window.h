#pragma once
#include "ywx/command_manager.h"
#include "ywx/tooltip.h"
#include "ywx/ui_control.h"

namespace yw::window {

enum class type {
  unknown,
  standard,
  custom,
};

template<type Type> struct options;
template<type Type> class handle;

//////////////////////////////////////// MARK: unknown window (base class)

template<> struct options<type::unknown> {
  std::optional<int2> pos = {};
  std::optional<uint2> size = {};
  std::optional<std::wstring> title = {};
};

template<> class handle<type::unknown> : public unknown {
protected:
  handle(ui::slotid Id) : unknown(Id) {}

public:
  struct slot : public unknown::slot {
    HWND hwnd{};
    int4 frame_thickness{};
    int2 pos{};
    int2 size{};
    type type{};
    DWORD style{};
    DWORD exstyle{};
    std::wstring title{};
    bitmap rendertarget{};
    bitmap controllayer{};
    comptr<IDXGISwapChain1> swapchain{};

    ui::parts::background background;
    ui::parts::focus_ring focus_ring;

    ui::slotid child_control{};
    ui::slotid focused_control{};
    ui::slotid hovered_control{};
    ui::slotid captured_control{};

    bool dirty = true, messy = true, manually_drawn = false;
    bool visible = false, active = false;
    bool resizing = false, tracking = false;

    function<bool> on_close;
    function<void, events::key> on_keydown;
    function<void, events::key> on_keyup;

    command_manager commands;

    //-- overrides --//

    virtual ui::slotid get_window_id() const override { return id; }

    virtual std::expected<void, error_trace> attachable() const override {
      if (child_control) return unexpected_error(errors::invalid_operation, "Window already has a control attached");
      return {};
    }
    virtual std::expected<void, error_trace> attach(ui::slotid Child) override {
      child_control = Child;
      messy = true;
      return {};
    }
    virtual std::expected<void, error_trace> detach(ui::slotid Child) override {
      if (child_control != Child) return unexpected_error(errors::invalid_argument, "Invalid child slot ID");
      system::uis.erase(std::exchange(child_control, {}));
      messy = true;
      return {};
    }
    virtual std::expected<void, error_trace> make_dirty() override {
      dirty = true;
      return {};
    }
    virtual std::expected<void, error_trace> make_moved() override {
      dirty = true;
      return {};
    }
    virtual std::expected<void, error_trace> make_messy() override {
      messy = true;
      return {};
    }

    //-- functions --//

    std::expected<void, error_trace> resize_rendertarget() {
      /// \note Avoid setting the render target size to zero
      const auto usz = vapply_r<uint2>(yw::max, size, uint2::fill(ui::arbitrary_value));
      if (usz != rendertarget.size()) {
        rendertarget = {};
        if (!swapchain) {
          auto desc = DXGI_SWAP_CHAIN_DESC1(usz.x, usz.y, bitmap::dxgiformat, false, DXGI_SAMPLE_DESC(1, 0), {}, 2);
          desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT, desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
          const auto hr = dxgi.factory()->CreateSwapChainForHwnd(d3d.device(), hwnd, &desc, 0, 0, &swapchain.get());
          if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateSwapChain failed", int(hr));
        } else if (const auto hr = swapchain->ResizeBuffers(0, usz.x, usz.y, DXGI_FORMAT_UNKNOWN, 0); FAILED(hr))
          return unexpected_error(errors::operation_failed, "ResizeBuffers failed", int(hr));
        if (auto res = bitmap::create(swapchain.get())) rendertarget = std::move(*res);
        else return unexpected_error(res.error());
        messy = true, manually_drawn = false;
      }
      if (usz != controllayer.size()) {
        if (auto res = bitmap::create(usz)) controllayer = std::move(*res);
        else return unexpected_error(res.error());
        messy = true, manually_drawn = false;
      }
      return {};
    }

    std::expected<void, error_trace> update_controllayer() {
      if (!messy) return {};
      if (child_control) {
        const auto csp = system::slot_address<ui::control>(child_control);
        if (!csp) return unexpected_error(errors::ui_invalid_slotid);
        csp->ensure_minimum_size();
        const auto new_size = vapply_r<int2>(yw::max, csp->core.area(), size);
        csp->update_layout({}, new_size);
        if (size != new_size)
          ::SetWindowPos(hwnd, nullptr, 0, 0, new_size.x, new_size.y, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
      }
      messy = false;
      dirty = true;
      return {};
    }

    std::expected<void, error_trace> draw_controllayer() {
      if (!dirty || !child_control) return {};
      const auto csp = system::slot_address<ui::control>(child_control);
      if (!csp) return unexpected_error(errors::ui_invalid_slotid);
      auto d = controllayer.begin_draw(colors::transparent);
      if (!d) return unexpected_error(d.error());
      if (auto res = csp->draw(); !res) return unexpected_error(res.error());
      if (auto res = d->close(); !res) return unexpected_error(res.error());
      return {};
    }

    std::expected<void, error_trace> draw() {
      if (!dirty) return {};
      auto d = rendertarget.begin_draw(background.color);
      if (!d) return unexpected_error(d.error());
      if (background.image)
        if (auto res = draw_bitmap({}, size, background.image, background.image_opacity); !res)
          return unexpected_error(res.error());
      if (auto res = draw_bitmap({}, controllayer); !res) return unexpected_error(res.error());
      if (const auto fcsp = system::slot_address<ui::control>(focused_control)) {
        if (auto res = fcsp->draw_focus_ring(focus_ring); !res) return unexpected_error(res.error());
      }
      if (auto res = d->close(); !res) return unexpected_error(res.error());
      if (swapchain) swapchain->Present(0, 0);
      dirty = false;
      return {};
    }
  };

  virtual ~handle() noexcept { close(); }
  handle() noexcept = default;
  handle(handle&& Other) noexcept = default;

  handle& operator=(handle&& Other) noexcept {
    if (this != &Other) {
      close();
      _id = std::exchange(Other._id, {});
    }
    return *this;
  }

  virtual void close() noexcept {
    if (const auto wsp = system::slot_address<handle>(_id)) ::DestroyWindow(wsp->hwnd);
  }

  auto background() {
    const auto wsp = system::slot_address<handle>(_id);
    if (!wsp) fatal_error(errors::ui_invalid_slotid);
    return wsp->background.handle();
  }
};

//////////////////////////////////////// MARK: standard window

template<> struct options<type::standard> {
  /// \note 初期化時の記法の都合により、options<type::unknown>を継承するわけにはいかない

  std::optional<int2> pos = {};
  std::optional<uint2> size = {};
  std::optional<std::wstring> title = {};
  bool header = true; // title, minimize box, close button
  bool border = true; // thin border
  bool frame = true;  // thick frame to resize window
  bool show = true;
};

template<> class handle<type::standard> : public handle<type::unknown> {
  handle(ui::slotid Id) : handle<type::unknown>(Id) {}

public:
  struct slot : public handle<type::unknown>::slot {
    static std::expected<handle, error_trace> open(options<type::standard> Options) {
      if (auto res = wclass.initialize(); !res) return unexpected_error(res.error());
      if (auto res = d2d.initialize(); !res) return unexpected_error(res.error());
      const auto id = system::uis.add(std::make_unique<handle<type::standard>::slot>());
      const auto wsp = system::slot_address<handle<type::standard>>(id);
      if (!wsp) return unexpected_error(errors::ui_invalid_slotid);
      wsp->id = id;
      wsp->type = type::standard;
      wsp->exstyle = WS_EX_ACCEPTFILES;
      wsp->style = Options.header ? WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX : WS_POPUP;
      if (Options.border) wsp->style |= WS_BORDER;
      if (Options.frame) wsp->style |= WS_THICKFRAME;
      if (!Options.title.has_value()) {
        wsp->title.resize(MAX_PATH, L'\0');
        const auto n = ::GetModuleFileNameW(nullptr, wsp->title.data(), MAX_PATH);
        if (n == 0) return unexpected_error(errors::operation_failed, "GetModuleFileNameW failed");
        wsp->title.resize(n);
      } else wsp->title = std::move(*Options.title);
      const auto hwnd = ::CreateWindowExW(
        wsp->exstyle, wclass.name(), wsp->title.c_str(), wsp->style, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, 0, 0,
        wclass.hinstance(), 0);
      if (!hwnd) return unexpected_error(errors::operation_failed, "CreateWindowExW failed");
      ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, std::bit_cast<LONG_PTR>(id));
      wsp->hwnd = hwnd;
      RECT wr, cr;
      if (!::GetWindowRect(hwnd, &wr)) return unexpected_error(errors::operation_failed, "GetWindowRect failed");
      if (!::GetClientRect(hwnd, &cr)) return unexpected_error(errors::operation_failed, "GetClientRect failed");
      const auto left = (wr.right - wr.top - cr.right) / 2;
      wsp->frame_thickness = int4(left, wr.bottom - wr.top - cr.bottom - left, left, left);
      wsp->pos = Options.pos.value_or(int2(wr.left, wr.top));
      if (!Options.size.has_value()) wsp->size = int2(cr.right, cr.bottom);
      else wsp->size = vapply_r<int2>(yw::max, *Options.size, int2(cr.right, cr.bottom));
      ::SetWindowPos(hwnd, nullptr, wsp->pos.x, wsp->pos.y, wsp->size.x, wsp->size.y, SWP_NOZORDER | SWP_NOACTIVATE);
      /// \note rendertarget and controllayer are getting updated in WM_SIZE
      wsp->messy = true, wsp->manually_drawn = false, wsp->active = true;
      if (Options.show) {
        ::ShowWindow(hwnd, SW_SHOW);
        wsp->visible = true;
      }
      system::primal_windows.push_back(id);
      return handle<type::standard>(id);
    }
  };

  handle() noexcept = default;

  virtual void close() noexcept override {
    if (const auto wsp = system::slot_address<handle>(_id)) ::DestroyWindow(wsp->hwnd);
    _id = {};
  }

  uint2 size() const {
    if (const auto wsp = system::slot_address<handle>(_id)) return wsp->size;
    return {};
  }

  std::expected<drawing, error_trace> begin_draw() {
    const auto wsp = system::slot_address<handle>(_id);
    if (!wsp) return unexpected_error(errors::ui_invalid_slotid);
    if (auto d = wsp->rendertarget.begin_draw()) return d;
    else return unexpected_error(d.error());
  }
};

inline std::expected<handle<type::standard>, error_trace> open(options<type::standard> Options) {
  if (auto res = handle<type::standard>::slot::open(std::move(Options))) return std::move(*res);
  else return unexpected_error(res.error());
}
} // namespace yw::window

namespace yw {

inline void ui::control::slot::hover_event(events::hover Event) {
  if (enabled && on_hover) on_hover(Event);
  if (tooltip.empty()) return;
  if (Event.enter()) {
    if (const auto w = system::slot_address<window::handle<window::type::unknown>>(window_id))
      system::tooltip.show(core.pos + w->pos + w->frame_thickness.xy(), core.size, tooltip);
  } else if (Event.leave()) system::tooltip.hide();
}
} // namespace yw
