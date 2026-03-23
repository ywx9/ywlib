#pragma once
#include "ywx/ui_control.h"

namespace yw::ui {

class spacer : public control {
public:
  class slot : public control::slot {};

  using control::operator bool;
  spacer() noexcept = default;
  spacer(derived_from<unknown> auto& Layout) noexcept {
    if (auto res = create_control<spacer>(Layout)) _id = *res;
  }
};
} // namespace yw::ui
