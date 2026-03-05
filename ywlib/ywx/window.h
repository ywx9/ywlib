#pragma once
#include "ywx/bitmap.h"
#include "ywx/event.h"

namespace yw {

//////////////////////////////////////// MARK: ui::base

class window;
class window_slot;

namespace ui {
class base {
public:
  class slot {
  public:
    slotset<slot>::slotid id;
    slotset<window_slot>::slotid window_id;
    float2 pos{};
    float2 size{};
    bool visible = true;
    bool enabled = true;
    virtual bool hit_test(float2 pt) const noexcept {
      return pt.x >= pos.x && pt.x <= pos.x + size.x && pt.y >= pos.y && pt.y <= pos.y + size.y;
    }
    virtual void draw() const {}
    virtual void draw_focus() const {}

    virtual void button_event(const event::button& e) {}
    virtual void key_event(const event::key& e) {}
    virtual void move_event(const event::move& e) {}
    virtual void hover_event(const event::hover& e) {}
    /// \note Returning false means the control cannot receive focus.
    virtual bool focus_event(bool focused) { return false; }
  };

protected:
  slotset<slot>::slotid _id;
  base(slotset<slot>::slotid id) : _id(id) {}
  window_slot* _window_slot() const noexcept;
  slot* _ui_slot() const noexcept;
  void _clear() noexcept;
  template<typename Mp, typename T> void _set(Mp mp, T&& value);

  template<typename Mp> const auto& unsafe_get(Mp mp) const {
    if (const auto s = dynamic_cast<const class_type<Mp>*>(_ui_slot())) return s->*mp;
    else throw std::runtime_error("invalid member access");
  }

  template<typename Ui, included_in<window, none> Window>
  static std::expected<tuple<Ui, typename Ui::slot*>, error_trace> add(Window& w, float2 Pos, float2 Size);

public:
  ~base() noexcept { _clear(); }
  base() noexcept = default;
  base(const base&) = delete;
  base& operator=(const base&) = delete;
  base(base&& other) noexcept : _id(std::exchange(other._id, {})) {}
  base& operator=(base&& other) noexcept {
    if (this == &other) return *this;
    _clear();
    _id = std::exchange(other._id, {});
    return *this;
  }
  explicit operator bool() const noexcept;
  const auto& id() const noexcept { return _id; }
  const auto& pos() const { return unsafe_get(&slot::pos); }
  const auto& size() const { return unsafe_get(&slot::size); }
  const auto& visible() const { return unsafe_get(&slot::visible); }
  const auto& enabled() const { return unsafe_get(&slot::enabled); }
  float2 cursor_pos() const noexcept;
  bool focused() const noexcept;

  void pos(float2 value) { _set(&slot::pos, value); }
  void size(float2 value) { _set(&slot::size, value); }
  void visible(bool value) { _set(&slot::visible, value); }
  void enabled(bool value) { _set(&slot::enabled, value); }

  bool hit_test(float2 pt) const noexcept { return _ui_slot() ? _ui_slot()->hit_test(pt) : false; }

  template<included_in<window&, none> Window>
  static std::expected<base, error_trace> add(Window&& w, float2 Pos, float2 Size) {
    if (auto res = add<base>(w, Pos, Size)) return std::move(yw::get<0>(*res));
    else return unexpected_error(res.error());
  }
};

//////////////////////////////////////// MARK: ui::frame

class frame : public base {
public:
  class slot : public base::slot {
  public:
    float2 radius{};
    color bg_color = colors::white;
    color border_color = colors::black;
    float border_width = 1.0f;

    function<void, event::move> on_move;
    function<void, event::hover> on_hover;

    virtual void draw() const override {
      fill_round_rectangle(pos, size, radius, bg_color);
      draw_round_rectangle(pos, size, radius, border_color, border_width);
    }

    virtual void draw_focus() const override {
      constexpr float2 margin = {2.5f, 2.5f};
      const auto focus_pos = pos - margin;
      const auto focus_size = size + margin * 2.0f;
      const auto focus_radius = radius + margin;
      draw_round_rectangle(focus_pos, focus_size, focus_radius, border_color, 1.0f, d2d.dashed_stroke_style());
    }

    virtual void move_event(const event::move& e) override {
      if (on_move) on_move(e);
    }

    virtual void hover_event(const event::hover& e) override {
      if (on_hover) on_hover(e);
    }
  };

protected:
  using base::base;
  slot* _ui_slot() const noexcept { return dynamic_cast<slot*>(base::_ui_slot()); }

public:
  using base::operator bool;

  const auto& radius() const { return unsafe_get(&slot::radius); }
  const auto& bg_color() const { return unsafe_get(&slot::bg_color); }
  const auto& border_color() const { return unsafe_get(&slot::border_color); }
  const auto& border_width() const { return unsafe_get(&slot::border_width); }
  const auto& on_move() const { return unsafe_get(&slot::on_move); }
  const auto& on_hover() const { return unsafe_get(&slot::on_hover); }

  void radius(float2 value) { _set(&slot::radius, value); }
  void bg_color(const color& value) { _set(&slot::bg_color, value); }
  void border_color(const color& value) { _set(&slot::border_color, value); }
  void border_width(float value) { _set(&slot::border_width, value); }
  void on_move(function<void, event::move> f) { _set(&slot::on_move, std::move(f)); }
  void on_hover(function<void, event::hover> f) { _set(&slot::on_hover, std::move(f)); }

  template<included_in<window&, none> Window>
  static std::expected<frame, error_trace> add(Window&& w, float2 Pos, float2 Size) {
    if (auto res = base::add<frame>(w, Pos, Size)) return std::move(yw::get<0>(*res));
    else return unexpected_error(res.error());
  }
};
} // namespace ui

namespace system {
inline slotset<ui::base::slot> uis;
inline slotset<window_slot> windows;
inline std::vector<slotset<window_slot>::slotid> master_windows;
inline int2 cursor_pos;
} // namespace system

//////////////////////////////////////// MARK: window

class window final {
protected:
  slotset<window_slot>::slotid _id;
  window(slotset<window_slot>::slotid id) : _id(id) {}
  window_slot* _window_slot() const noexcept;
  template<typename Mp, typename T> void _set(Mp mp, T&& value);

  template<typename Mp> const auto& unsafe_get(Mp mp) const {
    if (const auto w = _window_slot()) return w->*mp;
    else throw std::runtime_error("invalid member access");
  }

public:
  enum class style : uint32_t {
    unknown,
    regular = WS_OVERLAPPEDWINDOW,
    fixed = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
    borderless = WS_POPUP
  };

  template<stringable S>
  static std::expected<window, error_trace> open(int2, uint2, S&&, style = style::regular, bool = true);
  template<stringable S>
  static std::expected<window, error_trace> open(uint2, S&&, style = style::regular, bool = true);

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

  std::expected<void, error_trace> _init(bool centering, bool show) {
    if (auto res = wclass.initialize(); !res) return unexpected_error(res.error());
    if (hwnd) return {};
    switch (style) {
    case window::style::regular:
    case window::style::fixed:
    case window::style::borderless: break;
    default: return unexpected_error(errors::invalid_argument, "invalid window style");
    }
    hwnd = CreateWindowExW(WS_EX_ACCEPTFILES, wclass.name().data(), title.data(), DWORD(style), 0, 0, 0, 0, nullptr,
      nullptr, wclass.hinstance(), nullptr);
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
      if (!::GetClientRect(::GetDesktopWindow(), &cr)) return unexpected_win32_error("GetClientRect failed");
      pos = {(cr.right - total_size.x) / 2, (cr.bottom - total_size.y) / 2};
    }
    if (!::SetWindowPos(hwnd, nullptr, pos.x, pos.y, total_size.x, total_size.y, SWP_NOZORDER | SWP_NOACTIVATE))
      return unexpected_win32_error("SetWindowPos failed");
    if (show) ::ShowWindow(hwnd, SW_SHOW), ::SetForegroundWindow(hwnd), ::SetActiveWindow(hwnd);
    timer.start();
    return {};
  }

public:
  typename slotset<window_slot>::slotid id{};
  typename slotset<window_slot>::slotid master_id{};
  HWND hwnd{};
  int2 pos{};
  uint2 size{};
  uint4 margin{};
  window::style style{};
  std::wstring title{};
  bitmap rendertarget{};
  comptr<IDXGISwapChain1> swapchain{};
  stopwatch timer{};
  std::vector<typename slotset<ui::base::slot>::slotid> uis{};
  std::vector<typename slotset<window_slot>::slotid> subs{};
  slotset<ui::base::slot>::slotid focused_ui{};
  slotset<ui::base::slot>::slotid hovered_ui{};
  bool visible = true;
  bool enabled = true;
  bool resizing = false;
  mutable bool dirty = true;
  color bg_color = colors::white;
  bool _user_began_draw = false;

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
};

///////////////////////////////////////// MARK: ui implementation

namespace ui {
inline void base::_clear() noexcept {
  if (auto w_slot_p = _window_slot())
    if (const auto fr = std::ranges::find(w_slot_p->uis, _id); fr != w_slot_p->uis.end()) w_slot_p->uis.erase(fr);
  system::uis.erase(_id);
}

inline base::slot* base::_ui_slot() const noexcept { return system::uis.get(_id); }

inline window_slot* base::_window_slot() const noexcept {
  if (const auto ui_slot_p = _ui_slot(); !ui_slot_p) return nullptr;
  else return system::windows.get(ui_slot_p->window_id);
}

template<typename Mp, typename T> void base::_set(Mp mp, T&& value) {
  if (const auto ui_slot_p = dynamic_cast<class_type<Mp>*>(_ui_slot())) {
    ui_slot_p->*mp = static_cast<T&&>(value);
    if (const auto w_slot_p = system::windows.get(ui_slot_p->window_id)) w_slot_p->dirty = true;
  }
}

inline float2 base::cursor_pos() const noexcept {
  if (const auto s = _ui_slot())
    if (const auto w = system::windows.get(s->window_id))
      return float2(system::cursor_pos - w->pos - int2{w->margin.x, w->margin.y}) - s->pos;
  return {};
}

inline bool base::focused() const noexcept {
  if (const auto s = _ui_slot())
    if (const auto w = system::windows.get(s->window_id)) return w->focused_ui == _id;
  return false;
}

template<typename Ui, included_in<window, none> Window>
std::expected<tuple<Ui, typename Ui::slot*>, error_trace> base::add(Window& w, float2 Pos, float2 Size) {
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
} // namespace ui

//////////////////////////////////////// MARK: window implementation

inline window_slot* window::_window_slot() const noexcept { return system::windows.get(_id); }

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
  if (const auto w_slot_p = _window_slot())
    return system::cursor_pos - w_slot_p->pos - int2{w_slot_p->margin.x, w_slot_p->margin.y};
  return {};
}

inline void window::pos(int2 value) {
  if (const auto w_slot_p = _window_slot()) {
    w_slot_p->pos = value;
    ::SetWindowPos(w_slot_p->hwnd, nullptr, value.x, value.y, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
  }
}

inline void window::size(uint2 value) {
  if (const auto w_slot_p = _window_slot()) {
    w_slot_p->dirty = true;
    w_slot_p->size = value;
    ::SetWindowPos(w_slot_p->hwnd, nullptr, 0, 0, LONG(value.x + w_slot_p->margin.z),
      LONG(value.y + w_slot_p->margin.w), SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);
  }
}

template<stringable S> void window::title(S&& s) {
  if (const auto w_slot_p = _window_slot()) {
    w_slot_p->dirty = true;
    w_slot_p->title = unicode<wchar_t>(static_cast<S&&>(s));
    ::SetWindowTextW(w_slot_p->hwnd, w_slot_p->title.data());
  }
}

inline void window::visible(bool value) {
  if (const auto w_slot_p = _window_slot()) {
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
  if (const auto w_slot_p = _window_slot()) {
    w_slot_p->dirty = true;
    w_slot_p->enabled = value;
    ::EnableWindow(w_slot_p->hwnd, value);
  }
}

inline void window::bg_color(const color& value) {
  if (const auto w_slot_p = _window_slot()) {
    w_slot_p->dirty = true;
    w_slot_p->bg_color = value;
  }
}

inline void window::on_close(function<bool> f) {
  if (const auto w_slot_p = _window_slot()) w_slot_p->on_close = std::move(f);
}

inline void window::on_key(function<void, event::key> f) {
  if (const auto w_slot_p = _window_slot()) w_slot_p->on_key = std::move(f);
}

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

template<stringable S> std::expected<window, error_trace> window::open_subwindow(
  int2 LocalPos, uint2 Size, S&& Title, window::style Style, bool Show) {
  if (auto res = wclass.initialize(); !res) return unexpected_error(res.error());
  auto w_slot_p = _window_slot();
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

inline std::expected<drawing, error_trace> window::begin_draw() {
  if (const auto w_slot_p = _window_slot()) {
    w_slot_p->dirty = true;
    w_slot_p->_user_began_draw = true;
    if (auto d = w_slot_p->rendertarget.begin_draw(w_slot_p->bg_color)) return std::move(d);
    else return unexpected_error(d.error());
  } else return unexpected_error(errors::invalid_operation, "window slot not found");
}

inline std::expected<drawing, error_trace> window::begin_draw(const color& clear_color) {
  if (const auto w_slot_p = _window_slot()) {
    w_slot_p->dirty = true;
    w_slot_p->_user_began_draw = true;
    if (auto d = w_slot_p->rendertarget.begin_draw(clear_color)) return std::move(d);
    else return unexpected_error(d.error());
  } else return unexpected_error(errors::invalid_operation, "window slot not found");
}
} // namespace yw
