#pragma once
#include <ywx/control.h>

namespace yw::ui {

class blank : public control {
public:
  struct slot : control::slot {
    virtual std::expected<float2, error> get_necessary_size() const override { return {}; }
    virtual std::expected<void, error> redraw() override { return {}; }
  };

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
    sp->margin = {};
    return b;
  }

  //-- getter --//

  float2 pos() const noexcept { return control::pos(); }
  float2 size() const noexcept { return control::size(); }

private:
  using control::align;
  using control::background_color;
  using control::border_color;
  using control::button_event;
  using control::drag_event;
  using control::enabled;
  using control::focus_event;
  using control::focused;
  using control::height;
  using control::hover_event;
  using control::hovered;
  using control::key_event;
  using control::margin;
  using control::minimum_size;
  using control::padding;
  using control::pointer_event;
  using control::policy;
  using control::radius;
  using control::visible;
  using control::wheel_event;
  using control::width;
};
} // namespace yw::ui
