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
    for (const auto& wid : window::slot::windows) {
      const auto sp = window::slot::get<window>(wid);
      if (!sp) {
        error(errors::invalid_slotid).go_off(true); // warning
        continue;
      } else if (!sp->hwnd) {
        error(errors::unreachable).go_off(true); // warning
        continue;
      }


    }
  }
} mainloop;
}
