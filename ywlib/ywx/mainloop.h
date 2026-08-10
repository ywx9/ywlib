#pragma once
#include <ywx/window.h>

namespace yw {

inline class {
  MSG _msg{};
  uint64_t _count = 0;
  stopwatch _timer{};
  double _last_elapsed = 0.0;
  double _spf = 0.0;

public:
  /// permits 'while (mainloop) { ... }' to run the main loop
  explicit operator bool() { return operator()(); }
  double elapsed() const noexcept { return _timer.elapsed(); }
  double spf() const noexcept { return _spf; }
  double fps() const noexcept { return _spf > 0.0 ? 1.0 / _spf : 0.0; }

  bool operator()() {
    if (window::slot::windows.empty()) return false;
    if (!(_count++)) {
      _timer.restart();
      _last_elapsed = 0.0;
      _spf = 0.0;
      ::GetCursorPos(reinterpret_cast<POINT*>(&window::slot::cursor_pos));
    }
    const auto now = elapsed();
    _spf = now - _last_elapsed;
    _last_elapsed = now;
    while (::PeekMessageW(&_msg, nullptr, 0, 0, PM_REMOVE)) {
      if (_msg.message == WM_QUIT) return false;
      ::TranslateMessage(&_msg);
      ::DispatchMessageW(&_msg);
    }
    for (auto it = window::slot::windows.begin(); it != window::slot::windows.end();) {
      const auto wsp = static_cast<window::slot*>(window::slot::slots.get(*it));
      if (!wsp || !wsp->hwnd) it = window::slot::windows.erase(it);
      else if (auto res = wsp->update(now); !res) res.error().go_off();
      else ++it;
    }

    return true;
  }
} mainloop;
} // namespace yw
