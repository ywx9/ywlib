#pragma once
#include "ywx/ui_control.h"
#include "ywx/uip_text.h"

namespace yw::ui {

class label : public control {
public:
  struct slot : public control::slot {
    parts::background background{.color = colors::transparent};
    parts::border border{.color = colors::transparent};
    parts::text text;

    //-- overrides --//

    virtual std::expected<void, error_trace> draw() override {
      if (!visible) return {};
      if (auto res = background.draw(core); !res) return unexpected_error(res.error());
      if (auto res = text.draw(core.pos, core.size); !res) return unexpected_error(res.error());
      if (auto res = border.draw(core); !res) return unexpected_error(res.error());
      return {};
    }

    virtual float2 calculate_minimum_size() const override {
      const float2 inner = text.bounds() * (int2(1, 1) - core.constrained);
      return vapply_r<float2>(yw::max, core.min_size, core.required_size * core.constrained, inner);
    }

    virtual void ensure_minimum_size() override {
      core.size = calculate_minimum_size();
    }

    virtual slotid next_tab_stop(slotid, bool, bool&) const override { return {}; }
  };

  using control::operator bool;
  label() noexcept = default;

  static std::expected<label, error_trace> add(derived_from<unknown> auto& Layout) {
    label lbl;
    if (auto res = create_control<label>(Layout)) lbl._id = *res;
    else return unexpected_error(res.error());
    if (const auto csp = system::slot_address<label>(lbl._id)) {
      csp->background.control_id = lbl._id;
      csp->border.control_id = lbl._id;
      csp->text.control_id = lbl._id;
    } else return unexpected_error(errors::ui_invalid_slotid);
    return std::move(lbl);
  }

  template<typename Self> decltype(auto) background(this Self& self) {
    const auto csp = system::slot_address<label>(self._id);
    if (!csp) fatal_error(errors::ui_invalid_slotid);
    if constexpr (!is_const<Self>) return csp->background.access();
    else return std::as_const(csp->background.access());
  }
  template<typename Self> decltype(auto) border(this Self& self) {
    const auto csp = system::slot_address<label>(self._id);
    if (!csp) fatal_error(errors::ui_invalid_slotid);
    if constexpr (!is_const<Self>) return csp->border.access();
    else return std::as_const(csp->border.access());
  }
  template<typename Self> decltype(auto) text(this Self& self) {
    const auto csp = system::slot_address<label>(self._id);
    if (!csp) fatal_error(errors::ui_invalid_slotid);
    if constexpr (!is_const<Self>) return csp->text.access();
    else return std::as_const(csp->text.access());
  }
  auto& fit_to_text() {
    const auto csp = system::slot_address<label>(_id);
    if (!csp) fatal_error(errors::ui_invalid_slotid);
    this->core().size(csp->text.bounds());
    return *this;
  }
};
} // namespace yw::ui
