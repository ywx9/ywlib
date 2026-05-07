#pragma once
#include "ui_parts.h"

namespace yw::ui {

class control : public unknown {
protected:
  template<derived_from<control> Ctrl>
  static std::expected<slotid, error_trace> create_control(derived_from<unknown> auto& Layout) {
    const auto lid = Layout.id();
    const auto lsp = system::slot_address<unknown::slot>(lid);
    if (!lsp) return unexpected_error(errors::operation_failed, "Failed to access layout slot");
    const auto cid = system::uis.add(std::make_unique<typename Ctrl::slot>());
    const auto csp = system::slot_address<Ctrl>(cid);
    if (!csp) return unexpected_error(errors::operation_failed, "Failed to create control slot");
    if (!lsp->attach(cid)) {
      system::uis.erase(cid);
      return unexpected_error(errors::operation_failed, "Failed to attach control slot to layout");
    } else return cid;
  }

  control() noexcept = default;
public:
  struct slot : public unknown::slot {
    part::core core;
  };

  auto&& core() {
    const auto csp = system::slot_address<control>(_id);
    if (!csp) fatal_error(errors::invalid_operation, "Invalid slot address");
    return csp->core.handle();
  }

  const auto&& core() const {
    const auto csp = system::slot_address<control>(_id);
    if (!csp) fatal_error(errors::invalid_operation, "Invalid slot address");
    return csp->core.handle();
  }
};
}
