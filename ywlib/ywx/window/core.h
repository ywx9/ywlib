#pragma once
#include "ywx/core.h"

namespace yw::window {

class slot;
class slave;
class master;
class control_slot;

enum class style : DWORD {
  unknown,
  regular = WS_OVERLAPPEDWINDOW,
  fixed = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
  borderless = WS_POPUP
};

struct slotid {
  slotlist<slot>::id master;
  slotlist<slot>::id slave;
  slot* get_window() const noexcept;
};

struct control_slotid : slotid {
  slotlist<control_slot>::id control;
  control_slot* get_control() const noexcept;
};

inline struct {
  static std::expected<master, error_trace> operator()(int2, int2, const null_terminated<wchar_t>&, style, bool);
  static std::expected<master, error_trace> operator()(int2, const null_terminated<wchar_t>&, style, bool);
  static std::expected<slave, error_trace> subwindow(master&, int2, int2, const null_terminated<wchar_t>&, style, bool);
} open;
}

namespace yw::control {
class base;
}
