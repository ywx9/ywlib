#pragma once
#include "ui_parts.h"

namespace yw::ui {

class control : public unknown {
protected:
  control() noexcept = default;
public:
  struct slot : public unknown::slot {
    part::core core;
  };

  auto&& core() {
    const auto sp = system::slot_address<slot>(_id);
    if (!sp) fatal_error(errors::invalid_operation, "Invalid slot address");
    return sp->core.handle();
  }

  const auto&& core() const {
    const auto sp = system::slot_address<slot>(_id);
    if (!sp) fatal_error(errors::invalid_operation, "Invalid slot address");
    return sp->core.handle();
  }
};
}
