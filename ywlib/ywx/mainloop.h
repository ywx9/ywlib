#pragma once
#include <ywx/window.h>

namespace yw {

inline class {
  MSG _msg{};
  uint64_t _count = 0;
  stopwatch _timer{};

public:
  /// permits 'while (mainloop) { ... }' to run the main loop
  explicit operator bool() { return operator()(); }
  double elapsed() const noexcept { return _timer.elapsed(); }

  bool operator()() {
    if (window::slot::windows.empty()) return false;
    if (!(_count++)) {
      _timer.restart();
      ::GetCursorPos(reinterpret_cast<POINT*>(&window::slot::cursor_pos));
    }
    while (::PeekMessageW(&_msg, nullptr, 0, 0, PM_REMOVE)) {
      if (_msg.message == WM_QUIT) return false;
      ::TranslateMessage(&_msg);
      ::DispatchMessageW(&_msg);
    }
    for (auto it = window::slot::windows.begin(); it != window::slot::windows.end();) {
      const auto wsp = static_cast<window::slot*>(window::slot::slots.get(*it));
      if (!wsp || !wsp->hwnd) it = window::slot::windows.erase(it);
      else if (auto res = wsp->update(elapsed()); !res) res.error().go_off();
      else ++it;
    }

    return true;
  }
} mainloop;
} // namespace yw
