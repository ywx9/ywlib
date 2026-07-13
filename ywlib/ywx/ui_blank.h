#pragma once
#include <ywx/control.h>

namespace yw::ui {

class blank : public control {
public:
  struct slot : control::slot {};

  blank() noexcept = default;

  blank(derived_from<interface> auto& Parent, const source_line& sl = here()) {
    if (auto res = create(Parent, sl)) *this = std::move(*res);
    else res.error().add_footprint().go_off(sl);
  }

  static std::expected<blank, error> create(derived_from<interface> auto& Parent, const source_line& sl = here()) {
    blank b;
    const auto temp_id = make_slot<blank>();
    const auto sp = get_slot<blank>(temp_id);
    if (!sp) return std::unexpected(error(errors::slot_creation_failed));
    const auto psp = get_slot<control>(Parent.id());
    if (!psp) return std::unexpected(error(errors::invalid_slotid));
    if (auto res = psp->attach(temp_id); !res) {
      slot::slots.erase(temp_id);
      return res.error().relay();
    }
    b._id = temp_id;
    sp->id = temp_id;
    sp->window_id = psp->get_window_id();
    return b;
  }
};
} // namespace yw::ui
