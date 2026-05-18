#pragma once
#include "ywx/command_manager.h"
#include "ywx/tooltip.h"
#include "ywx/ui_parts.h"

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

    function<bool> on_close;
    function<void, events::key> on_keydown;
    function<void, events::key> on_keyup;

    command_manager commands;

    //-- overrides --//

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

  void close() noexcept {
    system::uis.erase(_id);
    std::erase(system::primal_windows, _id);
  }
};

//////////////////////////////////////// MARK: standard window

template<> struct options<type::standard> {
  std::optional<int2> pos = {};
  std::optional<uint2> size = {};
  std::optional<std::wstring> title = {};
  bool header = true; // title, minimize box, close button
  bool border = true; // thin border
  bool frame = true;  // to resize window
  bool show = true;
};

template<> class handle<type::standard> : public unknown {
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
    bitmap ui_bitmap{};
    comptr<IDXGISwapChain1> swapchain{};

    ui::parts::background background;
    ui::parts::focus_ring focus_ring;

    ui::slotid child_control{};
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

    //-- overrides --//

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
  };

  ~handle() noexcept { close(); }
  handle() noexcept = default;
  handle(handle&& Other) noexcept = default;

  handle& operator=(handle&& Other) noexcept {
    if (this != &Other) {
      close();
      _id = std::exchange(Other._id, {});
    }
    return *this;
  }

  void close() noexcept {
    system::uis.erase(_id);
    std::erase(system::primal_windows, _id);
  }
};

inline std::expected<handle<type::standard>, error_trace> open(options<type::standard> Options) {
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
  wsp->hwnd = ::CreateWindowExW(
    wsp->exstyle, wclass.name(), wsp->title.c_str(), wsp->style, 0, 0, 0, 0, 0, 0, wclass.hinstance(), 0);
  if (!wsp->hwnd) return unexpected_error(errors::operation_failed, "CreateWindowExW failed");
  system::primal_windows.push_back(id);
}
} // namespace yw::window

// namespace yw {

// class window : public unknown {
// public:
//   enum class type : uint32_t {
//     standard,  // you can select style from `window::appearance`
//     custom,    // you can define your own style by `WS_*`
//     subwindow, // standard window whose lifetime is tied to its parent
//     modal,     // standard window which disables its all relatives
//   };

//   enum class appearance : uint32_t {
//     unknown,
//     regular = WS_OVERLAPPEDWINDOW,
//     size_fixed = WS_OVERLAPPED | WS_BORDER | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
//     borderless = WS_POPUP
//   };

//   template<type Type> struct options {
//     std::optional<int2> pos = {};
//     std::optional<uint2> size = {};
//     std::optional<std::wstring> title = {};
//     appearance appearance = window::appearance::regular;
//     bool show = true;
//   };

//   template<type Type> static std::expected<window, error_trace> open(options<Type> Options);
//   static std::expected<window, error_trace> open(options<type::standard> Options) {
//     if (auto res = open<type::standard>(Options)) return std::move(*res);
//     else return unexpected_error(res.error());
//   }

//   void close() noexcept;

//   struct slot : public unknown::slot {
//     HWND hwnd{};
//     int4 frame_thickness{};
//     int2 pos{};
//     int2 size{};
//     type type{};
//     DWORD style{};
//     DWORD exstyle{};
//     std::wstring title{};
//     bitmap rendertarget{};
//     bitmap ui_bitmap{};
//     comptr<IDXGISwapChain1> swapchain{};

//     ui::parts::background background;
//     ui::parts::focus_ring focus_ring;

//     ui::slotid child_control{};
//     ui::slotid focused_control{};
//     ui::slotid hovered_control{};
//     ui::slotid captured_control{};

//     bool dirty = true, messy = true, manually_drawn = false;
//     bool visible = false, active = false;
//     bool resizing = false;
//     bool tracking = false;

//     function<bool> on_close;
//     function<void, events::key> on_keydown;
//     function<void, events::key> on_keyup;

//     command_manager commands;
//     std::vector<ui::slotid> parents;
//     std::vector<ui::slotid> subwindows;

//     //-- overrides --//

//     virtual std::expected<void, error_trace> attachable() const override {
//       if (layout_id) return unexpected_error(errors::ui_not_attachable, "Window already has a control attached");
//       return {};
//     }
//     virtual std::expected<void, error_trace> attach(ui::slotid Child) override {
//       layout_id = Child;
//       messy = true;
//       return {};
//     }
//     virtual std::expected<void, error_trace> detach(ui::slotid Child) override {
//       if (layout_id != Child) return unexpected_error(errors::invalid_argument, "Invalid child slot ID");
//       system::uis.erase(std::exchange(layout_id, {}));
//       messy = true;
//       return {};
//     }
//     virtual std::expected<void, error_trace> make_dirty() override {
//       dirty = true;
//       return {};
//     }
//     virtual std::expected<void, error_trace> make_moved() override {
//       dirty = true;
//       return {};
//     }
//     virtual std::expected<void, error_trace> make_messy() override {
//       messy = true;
//       return {};
//     }

//     //-- functions --//

//     int2 area() const noexcept { return size + frame_thickness.xy() + frame_thickness.zw(); }

//     std::expected<void, error_trace> init_window() {
//       if (hwnd) return unexpected_error(errors::operation_failed, "Window already initialized");
//       hwnd = ::CreateWindowExW(
//         exstyle, wclass.name(), title.c_str(), style, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, 0, 0, wclass.hinstance(),
//         0);
//       if (!hwnd) return unexpected_win32_error("CreateWindowExW failed");
//       return {};
//     }

//     std::expected<void, error_trace> init_frame_thickness() {
//       RECT wr, cr;
//       if (!::GetWindowRect(hwnd, &wr)) return unexpected_win32_error("GetWindowRect failed");
//       if (!::GetClientRect(hwnd, &cr)) return unexpected_win32_error("GetClientRect failed");
//       const auto left = (wr.right - wr.left - cr.right) / 2;
//       const auto top = wr.bottom - wr.top - cr.bottom - left;
//       frame_thickness = int4(left, top, left, left); // 決め打ちだが、例外はないはず
//       size = int2(cr.right, cr.bottom);
//       pos = int2(wr.left, wr.top);
//       return {};
//     }

//     std::expected<void, error_trace> init_pos_size() {
//       if (!::SetWindowPos(hwnd, nullptr, pos.x, pos.y, size.x, size.y, SWP_NOZORDER | SWP_NOACTIVATE))
//         return unexpected_win32_error("SetWindowPos failed");
//       return {};
//     }

//     std::expected<void, error_trace> resize(uint2 Size) {
//       /// \note WM_SIZE 内で使用されることを想定している
//       if (bool(rendertarget) && Size == rendertarget.size()) return {};
//       rendertarget = {}, size = int2(Size);
//       if (Size.x == 0 || Size.y == 0) return {};
//       if (!swapchain) {
//         auto desc = DXGI_SWAP_CHAIN_DESC1(Size.x, Size.y, bitmap::dxgiformat, false, DXGI_SAMPLE_DESC(1, 0), {}, 2);
//         desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT, desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
//         const auto hr = dxgi.factory()->CreateSwapChainForHwnd(d3d.device(), hwnd, &desc, 0, 0, &swapchain.get());
//         if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateSwapChain failed", int(hr));
//       } else if (const auto hr = swapchain->ResizeBuffers(0, Size.x, Size.y, DXGI_FORMAT_UNKNOWN, 0); FAILED(hr))
//         return unexpected_error(errors::operation_failed, "ResizeBuffers failed", int(hr));
//       if (auto res = bitmap::create(swapchain.get())) rendertarget = std::move(*res);
//       else return unexpected_error(res.error());
//       if (auto res = bitmap::create(size)) ui_bitmap = std::move(*res);
//       else return unexpected_error(res.error());
//       messy = true, manually_drawn = false;
//       return {};
//     }
//   };

//   virtual ~window() noexcept override { close(); }
//   explicit window() noexcept = default;
//   window(window&& other) noexcept { _id = std::exchange(other._id, {}); }
//   window& operator=(window&& other) noexcept {
//     if (this == &other) return *this;
//     close();
//     _id = std::exchange(other._id, {});
//     return *this;
//   }

//   explicit operator bool() const noexcept { return system::uis.contains(_id); }

//   auto& show(bool Show = true) {
//     if (auto wsp = system::slot_address<window>(_id)) {
//       ::ShowWindow(wsp->hwnd, Show ? SW_SHOW : SW_HIDE);
//       wsp->visible = Show;
//     } else fatal_error(errors::ui_invalid_slotid);
//     return *this;
//   }
//   auto& hide() { return show(false); }

//   int2 pos() const {
//     if (const auto wsp = system::slot_address<window>(_id)) return int2(wsp->pos);
//     return {};
//   }
//   uint2 size() const {
//     if (const auto wsp = system::slot_address<window>(_id)) return uint2(wsp->size);
//     return {};
//   }

//   std::expected<void, error_trace> resize(uint2 Size) {
//     if (const auto wsp = system::slot_address<window>(_id))
//       ::SetWindowPos(wsp->hwnd, nullptr, 0, 0, Size.x, Size.y, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
//     return unexpected_error(errors::ui_invalid_slotid);
//   }

//   auto background() {
//     const auto wsp = system::slot_address<window>(_id);
//     if (!wsp) fatal_error(errors::ui_invalid_slotid);
//     return wsp->background.handle();
//   }
//   const auto background() const {
//     const auto wsp = system::slot_address<window>(_id);
//     if (!wsp) fatal_error(errors::ui_invalid_slotid);
//     return wsp->background.handle();
//   }

//   const auto& on_close() const {
//     const auto wsp = system::slot_address<window>(_id);
//     if (!wsp) fatal_error(errors::ui_invalid_slotid);
//     return wsp->on_close;
//   }
//   std::expected<void, error_trace> on_close(function<bool> OnClose) const {
//     if (auto wsp = system::slot_address<window>(_id)) {
//       wsp->on_close = std::move(OnClose);
//       return {};
//     } else return unexpected_error(errors::ui_invalid_slotid);
//   }

//   const auto& on_keydown() const {
//     const auto wsp = system::slot_address<window>(_id);
//     if (!wsp) fatal_error(errors::ui_invalid_slotid);
//     return wsp->on_keydown;
//   }
//   std::expected<void, error_trace> on_keydown(function<void, events::key> OnKeyDown) const {
//     if (auto wsp = system::slot_address<window>(_id)) {
//       wsp->on_keydown = std::move(OnKeyDown);
//       return {};
//     } else return unexpected_error(errors::ui_invalid_slotid);
//   }

//   const auto& on_keyup() const {
//     const auto wsp = system::slot_address<window>(_id);
//     if (!wsp) fatal_error(errors::ui_invalid_slotid);
//     return wsp->on_keyup;
//   }
//   std::expected<void, error_trace> on_keyup(function<void, events::key> OnKeyUp) const {
//     if (auto wsp = system::slot_address<window>(_id)) {
//       wsp->on_keyup = std::move(OnKeyUp);
//       return {};
//     } else return unexpected_error(errors::ui_invalid_slotid);
//   }

//   const auto& commands() const {
//     const auto wsp = system::slot_address<window>(_id);
//     if (!wsp) fatal_error(errors::ui_invalid_slotid);
//     return wsp->commands;
//   }

//   std::expected<drawing, error_trace> begin_draw() {
//     if (const auto wsp = system::slot_address<window>(_id)) {
//       wsp->dirty = true;
//       wsp->manually_drawn = true;
//       if (auto d = wsp->rendertarget.begin_draw(wsp->background.color)) {
//         if (wsp->background.image)
//           draw_bitmap({}, wsp->size, wsp->background.image, wsp->background.image_opacity);
//         return std::move(d);
//       } else return unexpected_error(d.error());
//     } else return unexpected_error(errors::ui_invalid_slotid);
//   }
// };

// template<> struct window::options<window::type::standard> {
//   std::optional<int2> pos = {};
//   std::optional<uint2> size = {};
//   std::optional<std::wstring> title = {};
//   appearance appearance = window::appearance::regular;
//   bool show = true;
// };

// template<> struct window::options<window::type::custom> {
//   std::optional<int2> pos = {};
//   std::optional<uint2> size = {};
//   std::optional<std::wstring> title = {};
//   DWORD style = 0;
//   DWORD exstyle = 0;
// };

// template<> struct window::options<window::type::subwindow> {
//   ui::slotid parent; // required
//   std::optional<int2> pos = {};
//   std::optional<uint2> size = {};
//   std::optional<std::wstring> title = {};
//   appearance appearance = window::appearance::regular;
//   bool show = true;
// };

// template<> struct window::options<window::type::modal> {
//   ui::slotid parent = {}; // optional. zero means all windows get disabled
//   std::optional<int2> pos = {};
//   std::optional<uint2> size = {};
//   std::optional<std::wstring> title = {};
//   appearance appearance = window::appearance::regular;
// };

// template<window::type Type> std::expected<window, error_trace> window::open(window::options<Type> Options) {
//   if (auto res = wclass.initialize(); !res) return unexpected_error(res.error());
//   if (auto res = d2d.initialize(); !res) return unexpected_error(res.error());
//   const auto id = system::uis.add(std::make_unique<window::slot>());
//   const auto wsp = system::slot_address<window>(id);
//   if (!wsp) return unexpected_error(errors::ui_invalid_slotid);
//   wsp->id = id;
//   wsp->type = Type;
//   switch (Type) {
//   case window::type::standard:
//   case window::type::subwindow:
//   case window::type::modal:
//     wsp->style = static_cast<DWORD>(Options.appearance) | (Options.show ? WS_VISIBLE : 0);
//     wsp->exstyle = WS_EX_ACCEPTFILES;
//     break;
//   case window::type::custom:
//     wsp->style = Options.style;
//     wsp->exstyle = Options.exstyle;
//     break;
//   default: return unexpected_error(errors::invalid_argument, "Invalid window type");
//   }
//   if (auto res = wsp->init_window(); !res) return unexpected_error(res.error());
//   if (auto res = wsp->init_frame_thickness(); !res) return unexpected_error(res.error());
//   if (Options.pos.has_value()) wsp->pos = *Options.pos;
//   if (Options.size.has_value()) wsp->size = vapply_r<int2>(yw::max, wsp->size, *Options.size);
//   if (auto res = wsp->init_pos_size(); !res) return unexpected_error(res.error());
//   if constexpr (Type == window::type::subwindow) {
//     if (const auto parent = system::slot_address<window>(Options.parent)) {
//       wsp->parents.push_back(Options.parent);
//       parent->subwindows.push_back(id);
//     } else return unexpected_error(errors::ui_invalid_slotid);
//   } else if constexpr (Type == window::type::modal) {
//     if (const auto pwsp = system::slot_address<window>(Options.parent)) {
//       wsp->parents.push_back(pwsp->id);
//       pwsp->subwindows.push_back(id);
//       pwsp->disable();
//       for (const auto& swid : pwsp->subwindows)
//         if (const auto swsp = system::slot_address<window>(swid)) {
//           wsp->parents.push_back(swsp->id);
//           swsp->disable();
//         }
//     } else {
//       for (const auto& pwid : system::primal_windows)
//         if (const auto pwsp = system::slot_address<window>(pwid)) {
//           pwsp->disable();
//           for (const auto& swid : pwsp->subwindows)
//             if (const auto swsp = system::slot_address<window>(swid)) swsp->disable();
//         }
//     }
//   }
//   return {};
// }

// void window::close() noexcept {
//   /// \note Other actions besides window destruction are executed by the slot::close function called via
//   WM_NCDESTROY. const auto wsp = system::slot_address<window>(_id); if (!wsp) return;
//   ::DestroyWindow(wsp->hwnd);
//   wsp->hwnd = nullptr;
// }
// } // namespace yw
