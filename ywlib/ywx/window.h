#pragma once
#include "ywx/event.h"
#include "ywx/tooltip.h"
#include "ywx/ui_base.h"

namespace yw {

namespace system {
inline int2 cursor_pos;
inline slotset<ui::base::slot> uis;
inline slotset<window_slot> windows;
inline std::vector<slotset<window_slot>::slotid> master_windows;
} // namespace system

//////////////////////////////////////// MARK: window

class window final {
protected:
  slotset<window_slot>::slotid _id;
  window(slotset<window_slot>::slotid id) : _id(id) {}

  window_slot* _window_slot_address() const noexcept {
    if (const auto s = system::windows.get(_id)) return s;
    else return nullptr;
  }

  template<typename Mp> auto& unsafe_get(Mp mp) {
    if (const auto s = system::windows.get(_id)) return s->*mp;
    else throw std::runtime_error("invalid member access");
  }

  template<typename Mp> const auto& unsafe_get(Mp mp) const {
    if (const auto s = system::windows.get(_id)) return s->*mp;
    else throw std::runtime_error("invalid member access");
  }

public:
  enum class style : uint32_t {
    unknown,
    regular = WS_OVERLAPPEDWINDOW,
    fixed = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
    borderless = WS_POPUP
  };

  template<stringable S> static std::expected<window, error_trace> open(
    int2 Pos, uint2 Size, S&& Title, style Style = style::regular, bool Show = true);

  template<stringable S>
  static std::expected<window, error_trace> open(uint2 Size, S&& Title, style Style = style::regular, bool Show = true);

  ~window() noexcept { close(); }
  explicit window() noexcept = default;
  window(const window&) = delete;
  window& operator=(const window&) = delete;
  window(window&& other) noexcept : _id(std::exchange(other._id, {})) {}
  window& operator=(window&& other) noexcept {
    if (this != &other) {
      close();
      _id = std::exchange(other._id, {});
    }
    return *this;
  }
  explicit operator bool() const noexcept { return system::windows.contains(_id); }

  const auto& id() const noexcept { return _id; }

  const HWND& hwnd() const;
  const int2& pos() const;
  const uint2& size() const;
  const uint4& margin() const;
  const std::wstring& title() const;
  const stopwatch& timer() const;
  const bool& visible() const;
  const bool& enabled() const;
  const color& bg_color() const;
  const function<bool>& on_close() const;
  const function<void, event::key>& on_key() const;

  int2 cursor_pos() const;

  void pos(int2 value);
  void size(uint2 value);
  template<stringable S> void title(S&& s);
  void visible(bool value);
  void enabled(bool value);
  void bg_color(const color& value);
  void on_close(function<bool> f);
  void on_key(function<void, event::key> f);

  std::expected<drawing, error_trace> begin_draw();
  std::expected<drawing, error_trace> begin_draw(const color& clear_color);

  void close() noexcept;

  std::expected<void, error_trace> screenshot(const std::filesystem::path& PngFileName) const;
  bitmap screenshot() const;

  /// opens a subwindow which is closed when the master window is closed.
  /// \param LocalPos position relative to the master window's position.
  ///                 `{w.size().x, 0}` places it to the right of the master if they have the same style.
  /// \param Size size of the subwindow.
  /// \param Title title of the subwindow.
  /// \param Style style of the subwindow.　`style::unknown` copies the master's style.
  /// \param Show whether to show the subwindow immediately.
  template<stringable S> std::expected<window, error_trace> open_subwindow(
    int2 LocalPos, uint2 Size, S&& Title, style Style = style::unknown, bool Show = true);
};

//////////////////////////////////////// MARK: window_slot

class window_slot final {
  friend class window;

  std::expected<void, error_trace> _init(bool centering, bool show, DWORD ExStyle = WS_EX_ACCEPTFILES) {
    if (auto res = wclass.initialize(); !res) return unexpected_error(res.error());
    if (hwnd) return {};
    switch (style) {
    case window::style::regular:
    case window::style::fixed:
    case window::style::borderless: break;
    default: return unexpected_error(errors::invalid_argument, "invalid window style");
    }
    hwnd = CreateWindowExW(
      ExStyle, wclass.name().data(), title.data(), DWORD(style), 0, 0, 0, 0, nullptr, nullptr, wclass.hinstance(),
      nullptr);
    if (!hwnd) return unexpected_win32_error("CreateWindowExW failed");
    ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, std::bit_cast<LONG_PTR>(id));
    RECT cr{}, wr{};
    if (!::GetClientRect(hwnd, &cr)) return unexpected_win32_error("GetClientRect failed");
    if (!::GetWindowRect(hwnd, &wr)) return unexpected_win32_error("GetWindowRect failed");
    const auto left = (wr.right - wr.left - cr.right) / 2;
    const auto top = wr.bottom - wr.top - cr.bottom - left;
    margin = uint4(left, top, left * 2, left + top);
    const auto total_size = int2(size.x + margin.z, size.y + margin.w);
    if (centering) {
      const auto dcs = desktop_client_size();
      pos = {(dcs.x - total_size.x) / 2, (dcs.y - total_size.y) / 2};
    }
    if (!::SetWindowPos(hwnd, nullptr, pos.x, pos.y, total_size.x, total_size.y, SWP_NOZORDER | SWP_NOACTIVATE))
      return unexpected_win32_error("SetWindowPos failed");
    if (show) ::ShowWindow(hwnd, SW_SHOW), ::SetForegroundWindow(hwnd), ::SetActiveWindow(hwnd);
    timer.start();
    return {};
  }

public:
  slotset<window_slot>::slotid id{};
  slotset<window_slot>::slotid master_id{};

  HWND hwnd{};
  int2 pos{};
  uint2 size{};
  uint4 margin{};
  window::style style{};
  std::wstring title{};
  bitmap rendertarget{};
  comptr<IDXGISwapChain1> swapchain{};
  stopwatch timer{};
  std::vector<slotset<ui::base::slot>::slotid> uis{};
  std::vector<slotset<window_slot>::slotid> subs{};
  slotset<ui::base::slot>::slotid focused_ui{};
  slotset<ui::base::slot>::slotid hovered_ui{};
  bool visible = true;
  bool enabled = true;
  bool resizing = false;
  mutable bool dirty = true;
  color bg_color = colors::white;
  bool _user_began_draw = false;

  slotset<ui::base::slot>::slotid captured_ui{};
  key captured_key{};
  int capture_count{};

  function<bool> on_close;
  function<void, event::key> on_key;

  std::expected<void, error_trace> _resize_rendertarget(uint2 size) {
    if (swapchain) {
      rendertarget = {};
      if (auto hr = swapchain->ResizeBuffers(0, size.x, size.y, DXGI_FORMAT_UNKNOWN, 0); FAILED(hr))
        return unexpected_error(errors::operation_failed, "ResizeBuffers failed", int32_t(hr));
    } else {
      if (auto res = dxgi.initialize(); !res) return unexpected_error(res.error());
      auto desc = DXGI_SWAP_CHAIN_DESC1(size.x, size.y, bitmap::dxgiformat, false, DXGI_SAMPLE_DESC(1, 0), {}, 2);
      desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT, desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
      auto hr = dxgi.factory()->CreateSwapChainForHwnd(d3d.device(), hwnd, &desc, nullptr, nullptr, &swapchain.get());
      if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateSwapChainForHwnd failed", int32_t(hr));
    }
    if (auto res = bitmap::create(swapchain.get())) rendertarget = std::move(*res);
    else return unexpected_error(res.error());
    return {};
  }

  void draw() {
    if (dirty) {
      if (visible) {
        if (auto d = _user_began_draw ? rendertarget.begin_draw() : rendertarget.begin_draw(bg_color)) {
          for (const auto ui_slot_id : uis)
            if (const auto ui_slot_p = system::uis.get(ui_slot_id)) ui_slot_p->draw();
          if (const auto fui_slot_p = system::uis.get(focused_ui)) fui_slot_p->draw_focus();
        }
        if (swapchain) swapchain->Present(0, 0);
      }
      dirty = false;
      _user_began_draw = false;
    }
    for (const auto sub_slot_id : subs)
      if (const auto sub_slot_p = system::windows.get(sub_slot_id)) sub_slot_p->draw();
  }

  ui::base::slot* ui_hit_test(float2 pt) const {
    for (auto ui_slot_id : uis | std::views::reverse)
      if (const auto ui_slot_p = system::uis.get(ui_slot_id))
        if (ui_slot_p->visible && ui_slot_p->hit_test(pt)) return ui_slot_p;
    return nullptr;
  }

  void close() noexcept {
    try {
      for (const auto sub_slot_id : subs)
        if (const auto sub_slot_p = system::windows.get(sub_slot_id)) sub_slot_p->close();
      ::DestroyWindow(hwnd);
      system::windows.erase(id);
    } catch (...) {} // noexcept
  }
};

///////////////////////////////////////// MARK: ui::base

inline void ui::base::slot::hover_event(event::hover e) {
  if (enabled && on_hover) on_hover(e);
  if (tooltip.empty()) return;
  if (e.move()) {
    if (const auto w = system::windows.get(window_id)) system::tooltip.show(pos + w->pos() + w->margin.xy(), size);
  } else if (e.enter()) {
    if (const auto w = system::windows.get(window_id))
      system::tooltip.show(pos + w->pos() + w->margin.xy(), size, tooltip);
  } else if (e.leave()) system::tooltip.hide();
}

inline void ui::base::slot::make_window_dirty() const noexcept {
  if (const auto w = system::windows.get(window_id)) w->dirty = true;
}

inline ui::base::slot* ui::base::_base_slot_address() const noexcept { return system::uis.get(_id); }

inline window_slot* ui::base::_window_slot_address() const noexcept {
  if (const auto ui_slot_p = system::uis.get(_id)) return system::windows.get(ui_slot_p->window_id);
  else return nullptr;
}

inline void ui::base::_clear() {
  if (auto w_slot_p = _window_slot_address())
    if (const auto fr = std::ranges::find(w_slot_p->uis, _id); fr != w_slot_p->uis.end()) w_slot_p->uis.erase(fr);
  system::uis.erase(_id);
}

template<typename Ui, included_in<window, none> Window>
std::expected<tuple<Ui, typename Ui::slot*>, error_trace> ui::base::add(Window& w, float2 Pos, float2 Size) {
  const auto ui_slot_id = system::uis.add(std::make_unique<typename Ui::slot>());
  auto ui_slot_p = dynamic_cast<typename Ui::slot*>(system::uis.get(ui_slot_id));
  if (!ui_slot_p) return unexpected_error(errors::operation_failed, "ui slot creation failed");
  ui_slot_p->id = ui_slot_id;
  ui_slot_p->pos = Pos;
  ui_slot_p->size = Size;
  if constexpr (same_as<Window, window>) {
    if (const auto w_slot_p = system::windows.get(w.id())) {
      ui_slot_p->window_id = w_slot_p->id;
      w_slot_p->uis.push_back(ui_slot_id);
    } else return unexpected_error(errors::invalid_operation, "window slot not found");
  }
  return tuple{Ui(ui_slot_id), ui_slot_p};
}

inline ui::base::operator bool() const noexcept { return system::uis.contains(_id); }

inline void ui::base::make_window_dirty() const noexcept {
  if (const auto s = system::uis.get(_id)) s->make_window_dirty();
}

//////////////////////////////////////// MARK: window

template<stringable S>
std::expected<window, error_trace> window::open(int2 Pos, uint2 Size, S&& Title, style Style, bool Show) {
  if (auto res = wclass.initialize(); !res) return unexpected_error(res.error());
  const auto w_slot_id = system::windows.add(std::make_unique<window_slot>());
  auto w_slot_p = system::windows.get(w_slot_id);
  if (!w_slot_p) return unexpected_error(errors::operation_failed, "window slot creation failed");
  w_slot_p->id = w_slot_id;
  w_slot_p->pos = Pos;
  w_slot_p->size = Size;
  w_slot_p->style = Style;
  w_slot_p->title = unicode<wchar_t>(static_cast<S&&>(Title));
  if (auto res = w_slot_p->_init(false, Show); !res) {
    system::windows.erase(w_slot_id);
    return unexpected_error(res.error());
  }
  system::master_windows.push_back(w_slot_id);
  return window(w_slot_id);
}

template<stringable S> std::expected<window, error_trace> window::open(uint2 Size, S&& Title, style Style, bool Show) {
  if (auto res = wclass.initialize(); !res) return unexpected_error(res.error());
  const auto w_slot_id = system::windows.add(std::make_unique<window_slot>());
  auto w_slot_p = system::windows.get(w_slot_id);
  if (!w_slot_p) return unexpected_error(errors::operation_failed, "window slot creation failed");
  w_slot_p->id = w_slot_id;
  w_slot_p->size = Size;
  w_slot_p->style = Style;
  w_slot_p->title = unicode<wchar_t>(static_cast<S&&>(Title));
  if (auto res = w_slot_p->_init(true, Show); !res) {
    system::windows.erase(w_slot_id);
    return unexpected_error(res.error());
  }
  system::master_windows.push_back(w_slot_id);
  return window(w_slot_id);
}

inline const HWND& window::hwnd() const { return unsafe_get(&window_slot::hwnd); }
inline const int2& window::pos() const { return unsafe_get(&window_slot::pos); }
inline const uint2& window::size() const { return unsafe_get(&window_slot::size); }
inline const uint4& window::margin() const { return unsafe_get(&window_slot::margin); }
inline const std::wstring& window::title() const { return unsafe_get(&window_slot::title); }
inline const stopwatch& window::timer() const { return unsafe_get(&window_slot::timer); }
inline const bool& window::visible() const { return unsafe_get(&window_slot::visible); }
inline const bool& window::enabled() const { return unsafe_get(&window_slot::enabled); }
inline const color& window::bg_color() const { return unsafe_get(&window_slot::bg_color); }
inline const function<bool>& window::on_close() const { return unsafe_get(&window_slot::on_close); }
inline const function<void, event::key>& window::on_key() const { return unsafe_get(&window_slot::on_key); }

inline int2 window::cursor_pos() const {
  if (const auto w_slot_p = _window_slot_address())
    return system::cursor_pos - w_slot_p->pos - int2{w_slot_p->margin.x, w_slot_p->margin.y};
  return {};
}

inline void window::pos(int2 value) {
  if (const auto w_slot_p = _window_slot_address()) {
    w_slot_p->pos = value;
    ::SetWindowPos(w_slot_p->hwnd, nullptr, value.x, value.y, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
  }
}

inline void window::size(uint2 value) {
  if (const auto w_slot_p = _window_slot_address()) {
    w_slot_p->dirty = true;
    w_slot_p->size = value;
    ::SetWindowPos(
      w_slot_p->hwnd, nullptr, 0, 0, LONG(value.x + w_slot_p->margin.z), LONG(value.y + w_slot_p->margin.w),
      SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);
  }
}

template<stringable S> void window::title(S&& s) {
  if (const auto w_slot_p = _window_slot_address()) {
    w_slot_p->dirty = true;
    w_slot_p->title = unicode<wchar_t>(static_cast<S&&>(s));
    ::SetWindowTextW(w_slot_p->hwnd, w_slot_p->title.data());
  }
}

inline void window::visible(bool value) {
  if (const auto w_slot_p = _window_slot_address()) {
    w_slot_p->dirty = true;
    w_slot_p->visible = value;
    if (value) {
      ::ShowWindow(w_slot_p->hwnd, SW_SHOW);
      ::SetForegroundWindow(w_slot_p->hwnd);
      ::SetActiveWindow(w_slot_p->hwnd);
    } else ::ShowWindow(w_slot_p->hwnd, SW_HIDE);
  }
}

inline void window::enabled(bool value) {
  if (const auto w_slot_p = _window_slot_address()) {
    w_slot_p->dirty = true;
    w_slot_p->enabled = value;
    ::EnableWindow(w_slot_p->hwnd, value);
  }
}

inline void window::bg_color(const color& value) {
  if (const auto w_slot_p = _window_slot_address()) {
    w_slot_p->dirty = true;
    w_slot_p->bg_color = value;
  }
}

inline void window::on_close(function<bool> f) {
  if (const auto w_slot_p = _window_slot_address()) w_slot_p->on_close = std::move(f);
}

inline void window::on_key(function<void, event::key> f) {
  if (const auto w_slot_p = _window_slot_address()) w_slot_p->on_key = std::move(f);
}

inline std::expected<drawing, error_trace> window::begin_draw() {
  if (const auto w_slot_p = _window_slot_address()) {
    w_slot_p->dirty = true;
    w_slot_p->_user_began_draw = true;
    if (auto d = w_slot_p->rendertarget.begin_draw(w_slot_p->bg_color)) return std::move(d);
    else return unexpected_error(d.error());
  } else return unexpected_error(errors::invalid_operation, "window slot not found");
}

inline std::expected<drawing, error_trace> window::begin_draw(const color& clear_color) {
  if (const auto w_slot_p = _window_slot_address()) {
    w_slot_p->dirty = true;
    w_slot_p->_user_began_draw = true;
    if (auto d = w_slot_p->rendertarget.begin_draw(clear_color)) return std::move(d);
    else return unexpected_error(d.error());
  } else return unexpected_error(errors::invalid_operation, "window slot not found");
}

inline void window::close() noexcept {
  try {
    if (const auto w_slot_p = _window_slot_address()) {
      for (const auto sub_slot_id : w_slot_p->subs)
        if (const auto sub_slot_p = system::windows.get(sub_slot_id)) sub_slot_p->close();
      ::DestroyWindow(w_slot_p->hwnd);
      system::windows.erase(w_slot_p->id);
    }
  } catch (...) {} // noexcept
}

inline std::expected<void, error_trace> window::screenshot(const std::filesystem::path& PngFileName) const {
  if (const auto w = system::windows.get(_id)) {
    if (auto res = w->rendertarget.save_as_png(PngFileName)) return {};
    else return unexpected_error(res.error());
  } else return unexpected_error(errors::invalid_operation, "invalid window");
}

inline bitmap window::screenshot() const {
  if (const auto w = system::windows.get(_id)) {
    if (auto res = bitmap::create(w->rendertarget)) return std::move(*res);
    else return {};
  } else return {};
}

template<stringable S> std::expected<window, error_trace> window::open_subwindow(
  int2 LocalPos, uint2 Size, S&& Title, window::style Style, bool Show) {
  if (auto res = wclass.initialize(); !res) return unexpected_error(res.error());
  auto w_slot_p = _window_slot_address();
  if (!w_slot_p) return unexpected_error(errors::invalid_operation, "parent window slot not found");
  const auto sub_slot_id = system::windows.add(std::make_unique<window_slot>());
  auto sub_slot_p = system::windows.get(sub_slot_id);
  if (!sub_slot_p) return unexpected_error(errors::operation_failed, "subwindow slot creation failed");
  sub_slot_p->id = sub_slot_id;
  sub_slot_p->master_id = w_slot_p->id;
  sub_slot_p->pos = LocalPos + w_slot_p->pos;
  sub_slot_p->size = Size;
  sub_slot_p->style = (Style == style::unknown) ? w_slot_p->style : Style;
  sub_slot_p->title = unicode<wchar_t>(static_cast<S&&>(Title));
  if (auto res = sub_slot_p->_init(false, Show); !res) {
    system::windows.erase(sub_slot_id);
    return unexpected_error(res.error());
  }
  w_slot_p->subs.push_back(sub_slot_id);
  return window(sub_slot_id);
}
} // namespace yw
