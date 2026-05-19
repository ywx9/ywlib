#pragma once
#include "ywx/ui_control.h"

namespace yw::ui {

class blank : public control {
public:
  struct slot : public control::slot {
    virtual std::expected<void, error_trace> draw() override { return {}; }
    virtual slotid next_tab_stop(slotid, bool, bool&) const override { return {}; }
  };

  using control::operator bool;
  blank() noexcept = default;

  static std::expected<blank, error_trace> add(derived_from<unknown> auto& Layout) {
    blank b;
    if (auto res = create_control<blank>(Layout)) b._id = *res;
    else return unexpected_error(res.error());
    return b;
  }

private:
  using control::tooltip;
  using control::on_hover;
};
} // namespace yw::ui
