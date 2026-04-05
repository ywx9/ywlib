#pragma once
#include "ywx/key.h"

namespace yw::event {

//////////////////////////////////////// MARK: event::button

struct button {
  short2 pos;
  yw::key code;
  bool down : 1;
  bool ctrl : 1;
  bool shift : 1;
  bool alt : 1;
};
static_assert(sizeof(button) <= 8);

//////////////////////////////////////// MARK: event::drag

struct drag {
  short2 delta;
  yw::key code;
  bool ctrl : 1;
  bool shift : 1;
  bool alt : 1;
};
static_assert(sizeof(drag) <= 8);

//////////////////////////////////////// MARK: event::hover

struct hover {
  short2 pos;
  enum class type : uint8_t { enter = 0x1, leave = 0x2, move = 0x4 } type;
  bool enter() const noexcept { return type == type::enter; }
  bool leave() const noexcept { return type == type::leave; }
  bool move() const noexcept { return type == type::move; }
};
static_assert(sizeof(hover) <= 8);

//////////////////////////////////////// MARK: event::key

struct key {
  yw::key code;
  bool down : 1;
  bool first : 1;
  bool ctrl : 1;
  bool shift : 1;
  bool alt : 1;
};
static_assert(sizeof(key) <= 8);

//////////////////////////////////////// MARK: event::move

struct move {
  short2 pos;
  short2 delta;
};
static_assert(sizeof(move) <= 8);

//////////////////////////////////////// MARK: event::wheel

struct wheel {
  short2 pos;
  short delta;
  bool horizontal : 1;
  bool ctrl : 1;
  bool shift : 1;
  bool alt : 1;
};
static_assert(sizeof(wheel) <= 8);
} // namespace yw::event

namespace std {

template<typename C> struct formatter<decltype(yw::event::hover::type), C> {
  formatter<basic_string_view<C>, C> fmt;
  constexpr auto parse(auto& ctx) { return fmt.parse(ctx); }
  auto format(decltype(yw::event::hover::type) value, auto& ctx) const {
    if (value == decltype(yw::event::hover::type)::enter) return fmt.format("enter", ctx);
    if (value == decltype(yw::event::hover::type)::leave) return fmt.format("leave", ctx);
    if (value == decltype(yw::event::hover::type)::move) return fmt.format("move", ctx);
    return fmt.format("unknown", ctx);
  }
};

template<typename C> struct formatter<yw::event::hover, C> {
  formatter<basic_string<C>, C> fmt;
  constexpr auto parse(auto& ctx) { return fmt.parse(ctx); }
  auto format(yw::event::hover value, auto& ctx) const {
    const auto s = std::format("hover(pos: {}, type: {})", value.pos, value.type);
    return fmt.format(s, ctx);
  }
};
}
