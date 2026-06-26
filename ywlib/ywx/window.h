#pragma once
#include "ywx/accessor.h"
#include "ywx/bitmap.h"
#include "ywx/command_manager.h"
#include "ywx/control.h"

namespace yw::window {

enum class type {
  unknown,
  standard,
  custom,
};

template<type Type> struct options;
template<type Type> class handle;

/// MARK: unknown window (base class)

template<> struct options<type::unknown> {
  std::optional<int2> pos = {};
  std::optional<uint2> size = {};
  std::optional<std::wstring> title = {};
};

template<> class handle<type::unknown> : public interface {
public:
  struct slot : interface::slot {
    inline static std::vector<slotid> primals{};

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

    color background_color = colors::white;
    bitmap background_image;
    float background_image_opacity = 1.0f;

    color focusring_color = {0.0f, 0.0f, 1.0f, 0.5f};
    float focusring_offset = arbitrary_value;
    float focusring_width = 2.0f;
    bool focusring_dashed = false;

    interface::slotid child_control{};
    interface::slotid focused_control{};
    interface::slotid hovered_control{};
    interface::slotid mouse_capture{};
    interface::slotid key_capture{};

    struct mouse_state {
      interface::slotid owner{};
      short2 down_pos{};
      short2 last_pos{};
      key primary_button{};
      uint8_t buttons = 0;
      bool active = false;
      bool click_candidate = false;
      bool multi_button = false;
      bool moved_too_far = false;
    } mouse;

    struct key_state {
      interface::slotid owner{};
      key code{};
      bool active = false;
      bool activate_candidate = false;
    } keyboard;

    interface::slotid ime_owner{};
    bool ime_composing = false;

    bool dirty = true, messy = true, manually_drawn = false;
    bool visible = false, active = false;
    bool resizing = false, tracking = false;

    function<bool> on_close;
    function<void, yw::key_event> on_keydown;
    function<void, yw::key_event> on_keyup;

    command_manager commands;

    //-- overrides --//

    virtual bool attachable() const override { return !child_control; }

    virtual std::expected<void, error> attach(interface::slotid Child) override {
      child_control = Child;
      messy = true;
      return {};
    }

    virtual std::expected<void, error> detach(interface::slotid Child) override {
      if (child_control != Child) return std::unexpected(error(errors::invalid_argument, "Invalid child slot ID"));
      interface::slot::slots.erase(Child);
      child_control = {};
      messy = true;
      return {};
    }

    virtual std::expected<void, error> make_dirty() override { return dirty = true, std::expected<void, error>(); }
    virtual std::expected<void, error> make_messy() override { return messy = true, std::expected<void, error>(); }

    //-- functions --//

    static constexpr uint8_t button_bit(key k) noexcept {
      return k == keys::lbutton    ? 0x01
             : k == keys::rbutton  ? 0x02
             : k == keys::mbutton  ? 0x04
             : k == keys::xbutton1 ? 0x08
             : k == keys::xbutton2 ? 0x10
                                   : 0x00;
    }

    bool button_pressed(key k) const noexcept { return (mouse.buttons & button_bit(k)) != 0; }

    void set_button_pressed(key k, bool down) noexcept {
      const auto bit = button_bit(k);
      if (!bit) return;
      if (down) mouse.buttons |= bit;
      else mouse.buttons &= static_cast<uint8_t>(~bit);
    }

    void reset_mouse_state() noexcept {
      mouse_capture = {};
      mouse = {};
    }

    void reset_key_state() noexcept {
      key_capture = {};
      keyboard = {};
    }

    void reset_ime_state() noexcept {
      ime_owner = {};
      ime_composing = false;
    }

    std::expected<void, error> set_default_title() {
      title.resize(MAX_PATH, L'\0');
      if (const auto n = ::GetModuleFileNameW(nullptr, title.data(), MAX_PATH)) title.resize(n);
      else return std::unexpected(error(errors::operation_failed, "GetModuleFileNameW failed"));
      return {};
    }

    std::expected<void, error> create_window() {
      if (hwnd) return std::unexpected(error(errors::invalid_operation, "Window already created"));
      hwnd = ::CreateWindowExW(
        exstyle, wclass().name().data(), title.c_str(), style, CW_USEDEFAULT, CW_USEDEFAULT, int(arbitrary_value),
        int(arbitrary_value), 0, 0, wclass().hinstance(), 0);
      if (!hwnd) return std::unexpected(error(errors::operation_failed, "CreateWindowExW failed"));
      ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, std::bit_cast<LONG_PTR>(id));
      return {};
    }

    std::expected<void, error> calculate_frame_thickness() {
      RECT wr, cr;
      if (!::GetWindowRect(hwnd, &wr)) return std::unexpected(error(errors::operation_failed, "GetWindowRect failed"));
      if (!::GetClientRect(hwnd, &cr)) return std::unexpected(error(errors::operation_failed, "GetClientRect failed"));
      const auto left = (wr.right - wr.left - cr.right) / 2;
      frame_thickness = int4(left, wr.bottom - wr.top - cr.bottom - left, left, left);
      size = int2(cr.right, cr.bottom);
      pos = int2(wr.left, wr.top);
      return {};
    }

    std::expected<void, error> set_window_pos() {
      if (!::SetWindowPos(hwnd, nullptr, pos.x, pos.y, size.x, size.y, SWP_NOZORDER | SWP_NOACTIVATE))
        return std::unexpected(error(errors::operation_failed, "SetWindowPos failed"));
      return {};
    }

    std::expected<void, error> resize_rendertarget() {
      /// \note Avoid setting the render target size to zero
      const auto usz = vapply_r<uint2>(yw::max, size, uint2::fill(arbitrary_value));
      if (usz != rendertarget.size()) {
        rendertarget = {};
        if (!swapchain) {
          auto desc = DXGI_SWAP_CHAIN_DESC1(usz.x, usz.y, bitmap::dxgiformat, false, DXGI_SAMPLE_DESC(1, 0), {}, 2);
          desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT, desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
          hresult_test(dxgi::factory()->CreateSwapChainForHwnd, d3d().device(), hwnd, &desc, 0, 0, &swapchain.get());
        } else hresult_test(swapchain->ResizeBuffers, 0, usz.x, usz.y, DXGI_FORMAT_UNKNOWN, 0);
        if (auto res = bitmap::create(swapchain.get())) rendertarget = std::move(*res);
        else return res.error().relay();
        messy = true, manually_drawn = false;
      }
      if (usz != controllayer.size()) {
        if (auto res = bitmap::create(usz)) controllayer = std::move(*res);
        else return res.error().relay();
        messy = true, manually_drawn = false;
      }
      return {};
    }

    std::expected<void, error> update_controllayer() {
      if (!messy) return {};
      if (child_control) {
        const auto csp = interface::slot::get<control>(child_control);
        if (!csp) return std::unexpected(error(errors::invalid_slotid));
        if (auto res = csp->ensure_necessary_size(); !res) return res.error().relay();
        const auto new_size = vapply_r<int2>(yw::max, csp->bounds(), size);
        if (auto res = csp->update_geometry({}, new_size); !res) return res.error().relay();
        if (size != new_size)
          ::SetWindowPos(hwnd, nullptr, 0, 0, new_size.x, new_size.y, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
      }
      messy = false;
      dirty = true;
      return {};
    }

    std::expected<void, error> draw_controllayer() {
      if (!dirty || !child_control) return {};
      const auto csp = interface::slot::get<control>(child_control);
      if (!csp) return std::unexpected(error(errors::invalid_slotid));
      auto d = controllayer.begin_draw(colors::transparent);
      if (auto res = csp->draw(); !res) return res.error().relay();
      if (auto res = d.close(); !res) return res.error().relay();
      return {};
    }

    std::expected<void, error> draw() {
      if (!dirty) return {};
      auto d = rendertarget.begin_draw(background_color);
      if (background_image)
        if (auto res = draw_bitmap({}, size, background_image, background_image_opacity); !res)
          return res.error().relay();
      if (auto res = draw_bitmap({}, controllayer); !res) return res.error().relay();
      if (const auto fcsp = interface::slot::get<control>(focused_control)) {
        auto res = fcsp->draw_focusring(focusring_color, focusring_offset, focusring_width, focusring_dashed);
        if (!res) return res.error().relay();
      }
      if (auto res = d.close(); !res) return res.error().relay();
      if (swapchain) swapchain->Present(0, 0);
      dirty = false;
      return {};
    }

    /// explicitly performs redrawing
    std::expected<void, error> redraw(bool Messy = false) {
      Messy ? messy = true : dirty = true;
      if (auto res = update_controllayer(); !res) return res.error().relay();
      if (auto res = draw_controllayer(); !res) return res.error().relay();
      if (auto res = draw(); !res) return res.error().relay();
      return {};
    }
  };

  class background_accessor;
  class focusring_accessor;

  ywlib_make_accessor(background_accessor, handle);
  ywlib_make_accessor(focusring_accessor, handle);

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

  /// explicitly closes window
  virtual void close() noexcept {
    if (const auto wsp = interface::slot::get<handle>(_id)) ::DestroyWindow(wsp->hwnd);
    _id = {};
  }

  std::expected<void, error> show(bool Visible = true) {
    const auto wsp = interface::slot::get<handle>(_id);
    if (!wsp) return std::unexpected(error(errors::invalid_slotid));
    if (wsp->visible = Visible) ::ShowWindow(wsp->hwnd, SW_SHOW);
    else ::ShowWindow(wsp->hwnd, SW_HIDE);
    return {};
  }
  std::expected<void, error> hide() {
    if (auto res = show(false); !res) return res.error().relay();
    return {};
  }

  auto pos() const {
    if (const auto wsp = interface::slot::get<handle>(_id)) return wsp->pos;
    return int2{};
  }
  auto size() const {
    if (const auto wsp = interface::slot::get<handle>(_id)) return uint2(wsp->size);
    return uint2{};
  }

  std::expected<void, error> pos(int2 Pos) {
    const auto wsp = interface::slot::get<handle>(_id);
    if (!wsp) return std::unexpected(error(errors::invalid_slotid));
    win32_bool_test(::SetWindowPos, wsp->hwnd, nullptr, Pos.x, Pos.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
    return {};
  }
  std::expected<void, error> pos(int2 Pos, uint2 Size) {
    const auto wsp = interface::slot::get<handle>(_id);
    if (!wsp) return std::unexpected(error(errors::invalid_slotid));
    win32_bool_test(::SetWindowPos, wsp->hwnd, nullptr, Pos.x, Pos.y, Size.x, Size.y, SWP_NOZORDER | SWP_NOACTIVATE);
    return {};
  }

  std::expected<void, error> redraw(bool Messy = false) {
    const auto wsp = interface::slot::get<handle>(_id);
    if (!wsp) return std::unexpected(error(errors::invalid_slotid));
    if (auto res = wsp->redraw(Messy); !res) return res.error().relay();
    return {};
  }

  drawing begin_draw(const source_line& sl = here()) {
    const auto wsp = interface::slot::get<handle>(_id);
    if (!wsp) error(errors::invalid_slotid).go_off(sl);
    return wsp->rendertarget.begin_draw(sl);
  }
};

/// MARK: standard window

template<> struct options<type::standard> {
  std::optional<int2> pos = {};
  std::optional<uint2> size = {};
  std::optional<std::wstring> title = {};
  bool header = true; // title, minimize box, close button
  bool border = true; // thin border
  bool frame = true;  // thick frame to resize window
  bool show = true;   // automatically show window after creation
};

template<> class handle<type::standard> : public handle<type::unknown> {
public:
  struct slot : public handle<type::unknown>::slot {
    static std::expected<handle, error> open(options<type::standard>&& Options, const yw::source_line& sl) {
      const auto id = interface::slot::add<handle<type::standard>>();
      const auto wsp = interface::slot::get<handle<type::standard>>(id);
      if (!wsp) return std::unexpected(error(errors::invalid_slotid));
      wsp->id = id;
      wsp->source_line = sl;
      wsp->type = type::standard;
      wsp->exstyle = WS_EX_ACCEPTFILES;
      wsp->style = Options.header ? WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX : WS_POPUP;
      if (Options.border) wsp->style |= WS_BORDER;
      if (Options.frame) wsp->style |= WS_THICKFRAME;
      if (!Options.title.has_value()) {
        auto& t = wsp->title;
        t.resize(MAX_PATH, L'\0');
        if (const auto n = ::GetModuleFileNameW(nullptr, t.data(), MAX_PATH)) t.resize(n);
        else return std::unexpected(error(errors::operation_failed, "GetModuleFileNameW failed"));
      } else wsp->title = std::move(*Options.title);
      if (auto res = wsp->create_window(); !res) return res.error().relay();
      if (auto res = wsp->calculate_frame_thickness(); !res) return res.error().relay();
      if (Options.pos.has_value()) wsp->pos = *Options.pos;
      if (Options.size.has_value()) wsp->size = vapply_r<int2>(yw::max, *Options.size, wsp->size);
      if (auto res = wsp->set_window_pos(); !res) return res.error().relay();
      /// \note rendertarget and controllayer are getting updated in WM_SIZE
      wsp->messy = true, wsp->manually_drawn = false, wsp->active = true;
      if (wsp->visible = Options.show) ::ShowWindow(wsp->hwnd, SW_SHOW);
      primals.push_back(id);
      handle h;
      h._id = id;
      return h;
    }
  };

  handle() noexcept = default;

  virtual void close() noexcept override {
    if (const auto wsp = interface::slot::get<handle>(_id)) ::DestroyWindow(wsp->hwnd);
    _id = {};
  }

  uint2 size() const {
    if (const auto wsp = interface::slot::get<handle>(_id)) return wsp->size;
    return {};
  }
}; // standard window

inline handle<type::standard> open(options<type::standard> Options, const yw::source_line& sl = here()) {
  auto res = handle<type::standard>::slot::open(std::move(Options), sl);
  if (!res) res.error().add_footprint().go_off(sl);
  return std::move(*res);
}

// /// MARK: custom window

// template<> struct options<type::custom> {
//   std::optional<int2> pos = {};
//   std::optional<uint2> size = {};
//   std::optional<std::wstring> title = {};
//   std::optional<DWORD> style = {};
//   std::optional<DWORD> exstyle = {};
//   bool is_primal_window = true;
// };

// template<> class handle<type::custom> : public handle<type::unknown> {
//   handle(interface::slotid Id) : handle<type::unknown>(Id) {}

// public:
//   struct slot : public handle<type::unknown>::slot {
//     static std::expected<handle, error> open(options<type::custom> Options) {
//       const auto id = system::unknowns.add(std::make_unique<handle<type::custom>::slot>());
//       const auto wsp = system::get_slot_pointer<handle<type::custom>>(id);
//       if (!wsp) return std::unexpected(error(errors::invalid_slotid));
//       wsp->id = id;
//       wsp->type = type::custom;
//       wsp->style = Options.style.value_or(WS_OVERLAPPEDWINDOW);
//       wsp->exstyle = Options.exstyle.value_or(WS_EX_ACCEPTFILES);
//       if (!Options.title.has_value()) {
//         if (auto res = wsp->set_default_title(); !res) return res.error().relay();
//       } else wsp->title = std::move(*Options.title);
//       if (auto res = wsp->create_window(); !res) return res.error().relay();
//       if (auto res = wsp->calculate_frame_thickness(); !res) return res.error().relay();
//       if (Options.pos.has_value()) wsp->pos = *Options.pos;
//       if (Options.size.has_value()) wsp->size = vapply_r<int2>(yw::max, *Options.size, wsp->size);
//       if (auto res = wsp->set_window_pos(); !res) return res.error().relay();
//       /// \note rendertarget and controllayer are getting updated in WM_SIZE
//       wsp->messy = true, wsp->manually_drawn = false, wsp->active = true;
//       wsp->visible = bool(wsp->style & WS_VISIBLE);
//       if (Options.is_primal_window) system::primal_windows.push_back(id);
//       return handle<type::custom>(id);
//     }
//   };
// }; // custom window

// /// \note window::open

// template<type Type> std::expected<handle<Type>, error> open(options<Type> Options) {
//   if (auto res = handle<Type>::slot::open(std::move(Options))) return std::move(*res);
//   else return res.error().relay();
// }
// } // namespace yw::window

// namespace yw {

// namespace system {

// window::handle<window::type::unknown>::slot* window_get_slot_pointer(interface::slotid WindowId) noexcept {
//   return system::get_slot_pointer<window::handle<window::type::unknown>>(WindowId);
// }
// } // namespace system

// inline void ui::control::slot::hover_event(yw::hover_event e) {
//   if (!enabled) return;
//   if (on_hover) on_hover(e);
//   if (tooltip.empty()) return;
//   if (e.enter()) {
//     if (const auto w = system::get_slot_pointer<window::handle<window::type::unknown>>(window_id))
//       system::show_tooltip(pos + w->pos + w->frame_thickness.xy(), size, tooltip);
//   } else if (e.leave()) system::show_tooltip({}, {}, {});
// }
} // namespace yw::window
