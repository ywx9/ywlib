#pragma once
#include <ywx/control.h>

namespace yw::ui {

class blank : public control {
public:
  struct slot : control::slot {
    // virtual std::expected<float2, error> get_necessary_size() const override { return {}; }
    virtual std::expected<void, error> redraw(interface::slot*) override { return {}; }
  };

  blank() noexcept = default;

  blank(derived_from<interface> auto& Parent, const source_line& sl = here()) {
    if (auto res = create(Parent, sl)) *this = std::move(*res);
    else res.error().add_footprint().go_off(sl);
  }

  static std::expected<blank, error> create() {
    blank b;
    blank::slot* sp;
    if (auto res = create_control<blank>()) sp = *res;
    else return res.error().relay();
    b._id = sp->id;
    sp->margin = {};
    return b;
  }

  static std::expected<blank, error> create(derived_from<interface> auto& Parent, const source_line& sl = here()) {
    auto res = create();
    if (!res) return res.error().relay();
    if (auto attached = res->attach(Parent); !attached) return attached.error().relay();
    return res;
  }

private:
  using control::align;
  using control::background_color;
  using control::border_color;
  using control::button_event;
  using control::drag_event;
  using control::enabled;
  using control::focus_event;
  using control::focused;
  using control::hover_event;
  using control::hovered;
  using control::key_event;
  using control::margin;
  using control::minimum_size;
  using control::padding;
  using control::pointer_event;
  using control::radius;
  using control::visible;
  using control::wheel_event;
};
} // namespace yw::ui
