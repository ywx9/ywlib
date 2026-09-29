#pragma once
#include <apps/bitmap.h>
#include <apps/key.h>
#include <apps/text.h>
#include <core/array.h>
#include <core/core.h>
#include <core/format.h>
#include <core/function.h>
#include <core/optional.h>
#include <core/property.h>
#include <core/slotset.h>
#include <core/vector.h>

#ifdef _WIN32

#ifdef interface
#undef interface
#endif

namespace yw {

///--------------------------------------------------------------------------///
/// MARK: interface

class interface {
public:
  struct slot {
    inline static slotset<slot> slots{};
    slotset<slot>::slotid id;
    virtual ~slot() noexcept = default;
  };

  using slotid = slotset<slot>::slotid;
  const_property<slotid, interface> id;
  explicit operator bool() const noexcept { return slot::slots.exists(id); }

  virtual ~interface() noexcept { _destroy_slot(); }
  interface() noexcept = default;
  interface(const interface&) = delete;
  interface& operator=(const interface&) = delete;
  interface(interface&& o) noexcept : id(exchange(o.id.ref(), {})) {}
  interface& operator=(interface&& o) noexcept {
    if (this == &o) return *this;
    _destroy_slot();
    id = exchange(o.id.ref(), {});
    return *this;
  }

protected:
  explicit interface(slotid Id) : id(Id) {}
  void _destroy_slot() noexcept {
    if (const auto sp = slot::slots.get(id))
      if (auto res = slot::slots.erase(id); !res) // 設計上、エラーを想定していない
        res.error().print_and_abort("Failed to erase slot for interface");
  }
};

///--------------------------------------------------------------------------///
/// MARK: alignment

enum class alignment : uint8_t {
  center = 0b0000,
  left = 0b0001,
  right = 0b0010,
  top = 0b0100,
  bottom = 0b1000,
  left_top = 0b0101,
  left_bottom = 0b1001,
  right_top = 0b0110,
  right_bottom = 0b1010,
};

///--------------------------------------------------------------------------///
/// MARK: events

struct button_event {
  short2 pos;
  key key;
  key_state state;
  constexpr string<char> to_string() const {
    return format("button_event(pos:", pos, ", key:", internal::_get_key_name(key), ", state:", state, ")");
  }
};

struct cursor_event {
  short2 pos;
  short2 delta;
  constexpr string<char> to_string() const { return format("cursor_event(pos:", pos, ", delta:", delta, ")"); }
};

struct drag_event {
  short2 delta;
  key key;
  key_state state;
  constexpr string<char> to_string() const {
    return format("drag_event(delta:", delta, ", key:", internal::_get_key_name(key), ", state:", state, ")");
  }
};

struct focus_event {
  bool focused;
  constexpr string_view<char> to_string() const {
    if (focused) return "focus_event(focused:1)";
    else return "focus_event(focused:0)";
  }
};

struct hover_event {
  bool hovered;
  constexpr string_view<char> to_string() const {
    if (hovered) return "hover_event(hovered:1)";
    else return "hover_event(hovered:0)";
  }
};

struct key_event {
  key key;
  key_state state;
  constexpr string<char> to_string() const {
    return format("key_event(key:", internal::_get_key_name(key), ", state:", state, ")");
  }
};

struct wheel_event {
  short2 pos;
  short2 delta;
  key_state state;
  constexpr string<char> to_string() const {
    return format("wheel_event(pos:", pos, ", delta:", delta, ", state:", state, ")");
  }
};

///--------------------------------------------------------------------------///
/// MARK: control


} // namespace yw

#endif
