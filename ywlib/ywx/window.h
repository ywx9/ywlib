#pragma once
#include "ywx/ui_control.h"

namespace yw {

class window final {
public:
  class slot;
  using slotid = slotset<slot>::slotid;

  enum class style : uint32_t {
    unknown,
    regular = WS_OVERLAPPEDWINDOW,
    fixed = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
    borderless = WS_POPUP
  };

  class slot {
  public:
    slotid id{};

    HWND hwnd{};
    int2 pos{};
    uint2 size{};
    int4 margin{};
    window::style style{};
    std::wstring title{};
    bitmap rendertarget{};
    comptr<IDXGISwapChain1> swapchain{};
    stopwatch timer{};
    color bg_color = colors::white;
    ui::control::slotid layout{};
    ui::control::slotid focused_ui{};
    ui::control::slotid hovered_ui{};
    ui::control::slotid captured_ui{};
    bool visible = true;
    bool enabled = true;
    bool dirty = true;
    bool messy = true;
    bool resizing = false;
    bool manual_draw = false;

    key captured_key{};
    int capture_count{};

    function<bool> on_close;
    function<void, event::key> on_key;

    ~slot() noexcept {
      try {
        ::DestroyWindow(hwnd);
        system::controls.erase(layout);
      } catch (...) {} // noexcept destructor
    }
  };

private:
  slotid _id;
  window(slotid Id) : _id(Id) {}

  slot* _slot_address() const noexcept;

  template<typename Mp> auto& unsafe_get(Mp mp) const {
    if (const auto s = _slot_address()) return s->*mp;
    else throw std::runtime_error("invalid member access");
  }

public:
  ~window() noexcept { destroy(); }
  explicit window() noexcept = default;

  window(const window&) = delete;
  window& operator=(const window&) = delete;

  window(window&& other) noexcept : _id(std::exchange(other._id, {})) {}
  window& operator=(window&& other) noexcept {
    if (this == &other) return *this;
    destroy();
    _id = std::exchange(other._id, {});
    return *this;
  }

  explicit operator bool() const noexcept;
  const slotid& id() const noexcept { return _id; }
  const HWND& hwnd() const { return unsafe_get(&slot::hwnd); }
  const int4& margin() const { return unsafe_get(&slot::margin); }
  float time() const { return unsafe_get(&slot::timer).elapsed(); }

  const int2& pos() const { return unsafe_get(&slot::pos); }
  std::expected<void, error_trace> pos(int2 Pos) const {
    if (auto s = _slot_address()) {
      s->pos = Pos;
      ::SetWindowPos(s->hwnd, nullptr, Pos.x, Pos.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
      return {};
    } else return unexpected_error(errors::operation_failed, "Failed to access window slot.");
  }

  const uint2& size() const { return unsafe_get(&slot::size); }
  std::expected<void, error_trace> size(uint2 Size) const {
    if (auto s = _slot_address()) {
      s->messy = true; // layout can be changed
      s->size = Size;
      const auto sz = int2(Size) + s->margin.xy() + s->margin.zw();
      ::SetWindowPos(s->hwnd, nullptr, 0, 0, sz.x, sz.y, SWP_NOZORDER | SWP_NOMOVE);
      return {};
    } else return unexpected_error(errors::operation_failed, "Failed to access window slot.");
  }

  const std::wstring& title() const { return unsafe_get(&slot::title); }
  template<stringable S> std::expected<void, error_trace> title(S&& Title) const {
    if (auto s = _slot_address()) {
      s->title = unicode<wchar_t>(static_cast<S&&>(Title));
      ::SetWindowTextW(s->hwnd, s->title.c_str());
      return {};
    } else return unexpected_error(errors::operation_failed, "Failed to access window slot.");
  }

  const color& bg_color() const { return unsafe_get(&slot::bg_color); }
  std::expected<void, error_trace> bg_color(color BgColor) const {
    if (auto s = _slot_address()) {
      s->messy = true; // needs redraw
      s->bg_color = BgColor;
      s->dirty = true;
      return {};
    } else return unexpected_error(errors::operation_failed, "Failed to access window slot.");
  }

  const bool& visible() const { return unsafe_get(&slot::visible); }
  std::expected<void, error_trace> visible(bool Visible) const {
    if (auto s = _slot_address()) {
      s->messy = Visible; // needs redraw when becoming visible
      s->visible = Visible;
      ::ShowWindow(s->hwnd, Visible ? SW_SHOW : SW_HIDE);
      return {};
    } else return unexpected_error(errors::operation_failed, "Failed to access window slot.");
  }

  const bool& enabled() const { return unsafe_get(&slot::enabled); }
  std::expected<void, error_trace> enabled(bool Enabled) const {
    if (auto s = _slot_address()) {
      s->enabled = Enabled;
      if (Enabled) s->timer.start();
      else s->timer.stop();
      ::EnableWindow(s->hwnd, Enabled);
      return {};
    } else return unexpected_error(errors::operation_failed, "Failed to access window slot.");
  }

  std::expected<void, error_trace> destroy() noexcept;

  std::expected<drawing, error_trace> begin_draw() {
    if (const auto wsp = _slot_address()) {
      wsp->messy = true;
      if (wsp->manual_draw) {
        if (auto d = wsp->rendertarget.begin_draw()) return std::move(d);
        else return unexpected_error(d.error());
      } else if (auto d = wsp->rendertarget.begin_draw(wsp->bg_color)) return std::move(d);
      else return unexpected_error(d.error());
    } else return unexpected_error(errors::invalid_operation, "window slot not found");
  }
};

namespace system {
inline slotset<window::slot> windows{};
}

inline window::slot* window::_slot_address() const noexcept {
  return system::windows.get(_id);
}

inline explicit window::operator bool() const noexcept {
  return system::windows.contains(_id);
}

inline std::expected<void, error_trace> window::destroy() noexcept {
  if (const auto wsp = _slot_address()) {
    system::windows.erase(wsp->id);
    return {};
  } else return unexpected_error(errors::invalid_operation, "window slot not found");
}
} // namespace yw
