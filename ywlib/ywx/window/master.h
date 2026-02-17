#pragma once
#include "ywx/window/slave.h"

namespace yw::window {

class master : public slave {
  friend decltype(window::open);
  friend decltype(window::system);

public:
  using slave::slave;

  std::expected<slave, error_trace> open_subwindow(
    int2 Offset, int2 Size, const null_terminated<wchar_t>& Title, style Style = style::unknown, bool Show = true) {
    if (auto res = open.subwindow(*this, Offset, Size, Title, Style, Show)) return std::move(*res);
    else return unexpected_error(res.error());
  }
};
}
