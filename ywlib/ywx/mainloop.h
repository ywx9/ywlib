#pragma once
#include "ywx/window.h"

namespace yw {

inline class {
  MSG _msg{};
  uint64_t _count = 0;
  double _last_elapsed = 0.0;
  stopwatch _timer{};

public:
  /// permits 'while (mainloop) { ... }' to run the main loop
  explicit operator bool() { return operator()(); }

  bool operator()() {
    if (window::slot::windows.empty()) {
      error(errors::invalid_operation, "no window exists").go_off(true);
      return false;
    }

    if (!(_count++)) _timer.restart();

    while (::PeekMessageW(&_msg, nullptr, 0, 0, PM_REMOVE)) {
      if (_msg.message == WM_QUIT) return false;
      ::TranslateMessage(&_msg);
      ::DispatchMessageW(&_msg);
    }

    for (auto it = window::slot::windows.begin(); it != window::slot::windows.end();) {
      const auto wsp = interface::slot::get<window>(*it);
      if (!wsp || !wsp->hwnd) {
        it = window::slot::windows.erase(it);
        continue;
      }
      if (auto res = wsp->update(); !res) res.error().go_off();
      ++it;
    }

    return true;
  }
} mainloop;
} // namespace yw
