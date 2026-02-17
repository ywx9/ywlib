#pragma once
#include "ywx/window/slot.h"

namespace yw::window {

class slave {
  friend decltype(window::open);
  friend decltype(window::system);
  friend class ::yw::control::base;

protected:
  slotid _id{};

  slave(const slotid& id, bool show) noexcept {
    if (const auto window_slot = id.get_window()) {
      _id.master = id.master;
      window_slot->id.master = id.master;
      window_slot->id.slave = id.slave;
      window_slot->timer.start();
      if (show) this->show();
    } // slotが登録済みでなければ初期化しない
  }

  slot* _window() const noexcept { return _id.get_window(); }

  HWND _hwnd() const noexcept {
    const auto w = _window();
    return w ? w->hwnd : nullptr;
  }

public:
  ~slave() noexcept { ::DestroyWindow(_hwnd()); }
  slave() noexcept = default;
  slave(slave&&) noexcept = default;

  slave& operator=(slave&& w) noexcept {
    if (this == &w) return *this;
    ::DestroyWindow(_hwnd());
    _id = std::move(w._id);
    return *this;
  }

  explicit operator bool() const noexcept {
    const auto ms = system.windows.get(_id.master);
    return _id.slave.is_zero() ? bool(ms) : ms->slaves.contains(_id.slave);
  }

  int2 position() const noexcept {
    const auto hwnd = _hwnd();
    RECT r{};
    ::GetWindowRect(hwnd, &r);
    return int2(r.left, r.top);
  }

  int2 size() const noexcept {
    const auto hwnd = _hwnd();
    RECT r;
    ::GetClientRect(hwnd, &r);
    return int2(r.right, r.bottom);
  }

  int2 cursor() const noexcept {
    const auto hwnd = _hwnd();
    int2 pt;
    ::GetCursorPos(reinterpret_cast<LPPOINT>(&pt));
    ::ScreenToClient(hwnd, reinterpret_cast<LPPOINT>(&pt));
    return pt;
  }

  std::wstring title() const noexcept {
    const auto hwnd = _hwnd();
    const auto len = ::GetWindowTextLengthW(hwnd);
    if (len == 0) return {};
    std::wstring t(len, L'\0');
    ::GetWindowTextW(hwnd, t.data(), int(len + 1));
    return t;
  }

  void show(bool b = true) noexcept {
    const auto hwnd = _hwnd();
    if (b) ::ShowWindow(hwnd, SW_SHOW), ::SetForegroundWindow(hwnd), ::SetActiveWindow(hwnd);
    else ::ShowWindow(hwnd, SW_HIDE);
  }

  void enable(bool b = true) noexcept {
    const auto hwnd = _hwnd();
    ::EnableWindow(hwnd, b ? TRUE : FALSE);
  }

  void position(int2 p) noexcept {
    const auto hwnd = _hwnd();
    ::SetWindowPos(hwnd, nullptr, p.x, p.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
  }

  void size(int2 s) noexcept {
    const auto hwnd = _hwnd();
    ::SetWindowPos(hwnd, nullptr, 0, 0, s.x, s.y, SWP_NOZORDER | SWP_NOMOVE);
  }

  void cursor(int2 c) noexcept {
    const auto hwnd = _hwnd();
    POINT pt{c.x, c.y};
    ::ClientToScreen(hwnd, &pt);
    ::SetCursorPos(pt.x, pt.y);
  }

  void title(null_terminated<wchar_t> t) noexcept {
    const auto hwnd = _hwnd();
    ::SetWindowTextW(hwnd, t.data());
  }

  double time() const noexcept {
    if (const auto w = _window()) return w->timer.elapsed();
    else return 0.0;
  }

  std::expected<drawing, error_trace> begin_draw(const source& src = {}) {
    if (const auto w = _window()) return w->rendertarget.begin_draw(src);
    else return unexpected_error(errors::not_initialized, "window not initialized");
  }

  std::expected<drawing, error_trace> begin_draw(color clear_color, const source& src = {}) {
    if (const auto w = _window()) return w->rendertarget.begin_draw(clear_color, src);
    else return unexpected_error(errors::not_initialized, "window not initialized");
  }

  void screenshot(const std::filesystem::path& Png) const noexcept {
    if (const auto w = _window()) w->rendertarget.save_as_png(Png);
  }

  void close_confirmation(bool b = true) noexcept {
    if (const auto w = _window()) w->close_confirmation = b;
  }
};
}
