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

  button(yw::key Code, bool Down, WPARAM Wp, LPARAM Lp) noexcept
    : pos(std::bit_cast<short>(uint16_t(uint_cast(Lp))), std::bit_cast<short>(uint16_t(uint_cast(Lp) >> 16))),
      code(Code), down(Down), ctrl((Wp & MK_CONTROL) != 0), shift((Wp & MK_SHIFT) != 0),
      alt((::GetKeyState(VK_MENU) & 0x8000) != 0) {}
};

//////////////////////////////////////// MARK: event::key

struct key {
  uint16_t repeat : 16;
  yw::key code;
  bool down : 1;
  bool ctrl : 1;
  bool shift : 1;
  bool alt : 1;

  key(bool Down, WPARAM Wp, LPARAM Lp) noexcept
    : repeat(uint16_t(uint_cast(Lp) & 0xffff)), code(yw::key(uint_cast(Wp) & 0xff)), down(Down),
      ctrl((::GetKeyState(VK_CONTROL) & 0x8000) != 0), shift((::GetKeyState(VK_SHIFT) & 0x8000) != 0),
      alt((::GetKeyState(VK_MENU) & 0x8000) != 0) {}
};

//////////////////////////////////////// MARK: event::move

struct move {
  short2 pos;
  union {
    uint32_t wp;
    struct {
      bool lbutton : 1;
      bool rbutton : 1;
      bool shift : 1;
      bool ctrl : 1;
      bool mbutton : 1;
      bool xbutton1 : 1;
      bool xbutton2 : 1;
    };
  };
  move(WPARAM Wp, LPARAM Lp) : pos(std::bit_cast<short2>(uint32_t(uint_cast(Lp)))), wp(uint32_t(uint_cast(Wp))) {}
};
static_assert(sizeof(move) == 8);

//////////////////////////////////////// MARK: event::hover

struct hover {
  enum class type : uint8_t { enter = 0x1, leave = 0x2, move = 0x4 };
  short2 pos;
  union {
    uint32_t wp;
    struct {
      bool lbutton : 1;
      bool rbutton : 1;
      bool shift : 1;
      bool ctrl : 1;
      bool mbutton : 1;
      bool xbutton1 : 1;
      bool xbutton2 : 1;
      bool _unused : 1; // 0x80
      type type;
    };
  };
  hover(WPARAM Wp, LPARAM Lp) : pos(std::bit_cast<short2>(uint32_t(uint_cast(Lp)))), wp(uint32_t(uint_cast(Wp))) {}
  bool enter() const noexcept { return type == type::enter; }
  bool leave() const noexcept { return type == type::leave; }
  bool move() const noexcept { return type == type::move; }
};
static_assert(sizeof(hover) == 8);
static_assert(offsetof(hover, type) == sizeof(short2 /*pos*/) + (8 /*bools*/ / 8 /*bits per byte*/));

} // namespace yw::event
