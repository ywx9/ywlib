#pragma once
#include "ywx/core.h"

namespace yw {

template<typename Interface> class accessor {
protected:
  friend Interface;
  typename Interface::slot& slot;
  bool dirty{}, messy{};
  accessor(typename Interface::slot& Slot) noexcept : slot(Slot) {}

  template<typename Accessor> static Accessor create(Interface& i) {
    const auto sp = Interface::slot::get(&i);
    if (!sp) error(errors::invalid_slotid).print_as_fatal();
    return Accessor(*sp);
  }

  template<typename Accessor> static const Accessor create(const Interface& i) {
    const auto sp = Interface::slot::get(&i);
    if (!sp) error(errors::invalid_slotid).print_as_fatal();
    return Accessor(*sp);
  }

public:
  accessor(accessor&&) noexcept = default;
  accessor& operator=(accessor&&) noexcept = default;
  ~accessor() noexcept {
    if (messy) {
      if (auto res = slot.make_messy(); !res) res.error().print_as_fatal();
    } else if (dirty) {
      if (auto res = slot.make_dirty(); !res) res.error().print_as_fatal();
    }
  }
};

#define ywlib_make_accessor(Accessor, Interface)                             \
  template<typename Self> decltype(auto) Accessor(this Self&& self) {        \
    return accessor<Interface>::create<Accessor>(static_cast<Self&&>(self)); \
  }

} // namespace yw
