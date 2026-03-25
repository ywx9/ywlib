#pragma once
#include "ywx/ui_control.h"

namespace yw::ui {

class blank : public control {
public:
  class slot : public control::slot {};

  using control::operator bool;
  blank() noexcept = default;
  blank(derived_from<unknown> auto& Layout) noexcept {
    if (auto res = create_control<blank>(Layout)) _id = *res;
  }
};
} // namespace yw::ui
