#pragma once
#include <yw/slotset.h>

namespace yw {

class handle_base {
public:
  struct slot {
    inline static slotset<slot> slots{};

    slotset<slot>::slotid id;

    virtual ~slot() noexcept = default;

    template<derived_from<handle_base> Handle> static typename Handle::slot* get_as(slotset<slot>::slotid Id) noexcept {
      return static_cast<typename Handle::slot*>(slots.get(Id));
    }
  };

  using slotid = slotset<slot>::slotid;

protected:
  slotid _id{};

  template<typename Handle> static typename Handle::slot* make_slot() {
    const auto temp_id = slot::slots.add(std::make_unique<typename Handle::slot>());
    const auto sp = static_cast<typename Handle::slot*>(slot::slots.get(temp_id));
    if (sp) sp->id = temp_id;
    return sp;
  }

  template<typename Handle> static Handle make_handle(slotid Id) noexcept {
    Handle h;
    h._id = Id;
    return h;
  }

  static void erase_slot(slotid Id) noexcept { slot::slots.erase(Id); }

  template<typename Handle> static typename Handle::slot* get_slot(const Handle* h) {
    return static_cast<typename Handle::slot*>(slot::slots.get(h->_id));
  }

  template<typename Handle> static typename Handle::slot* get_slot(slotid Id) {
    return static_cast<typename Handle::slot*>(slot::slots.get(Id));
  }

  handle_base(const handle_base&) = delete;
  handle_base& operator=(const handle_base&) = delete;

public:
  virtual ~handle_base() noexcept { slot::slots.erase(_id); }

  handle_base() noexcept = default;

  handle_base(handle_base&& Other) noexcept : _id(std::exchange(Other._id, {})) {}

  handle_base& operator=(handle_base&& Other) noexcept {
    if (this != &Other) {
      slot::slots.erase(_id);
      _id = std::exchange(Other._id, {});
    }
    return *this;
  }

  slotid id() const noexcept { return _id; }
  explicit operator bool() const noexcept { return slot::slots.contains(_id); }
};
}
