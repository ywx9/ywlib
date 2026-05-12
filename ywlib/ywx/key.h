#pragma once
#include "ywx/core.h"

namespace yw {

struct key {
  uint8_t code;
  friend constexpr bool operator==(key lhs, key rhs) noexcept = default;
  bool pressed() const noexcept { return (::GetKeyState(int(code)) & 0x8000) != 0; }
  constexpr std::string_view name() const noexcept {
    static constexpr const char* names[] = {
      "_00", "lbutton", "rbutton", "_03", "mbutton", "xbutton1", "xbutton2", "_07", "backspace", "tab", "_0A", "_0B",
      "_0C", "enter", "_0E", "_0F", "shift", "ctrl", "alt", "_13", "caps_lock", "_15", "_16", "_17", "_18", "_19",
      "_1A", "escape", "_1C", "_1D", "_1E", "_1F", "space", "page_up", "page_down", "end", "home", "left", "up",
      "right", "down", "_29", "_2A", "_2B", "print_screen", "insert", "delete_", "_2F", "0", "1", "2", "3", "4", "5",
      "6", "7", "8", "9", "_3A", "_3B", "_3C", "_3D", "_3E", "_3F", "_40", "A", "B", "C", "D", "E", "F", "G", "H", "I",
      "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "win", "win", "menu", "_5E",
      "_5F", "_60", "_61", "_62", "_63", "_64", "_65", "_66", "_67", "_68", "_69", "_6A", "_6B", "_6C", "_6D", "_6E",
      "_6F", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12", "_7C", "_7D", "_7E", "_7F",
      "_80", "_81", "_82", "_83", "_84", "_85", "_86", "_87", "_88", "_89", "_8A", "_8B", "_8C", "_8D", "_8E", "_8F",
      "num_lock", "scroll_lock"};
    return names[static_cast<uint8_t>(code)];
  }
};
static_assert(sizeof(key) == 1);

namespace keys {
inline constexpr key lbutton{VK_LBUTTON};
inline constexpr key rbutton{VK_RBUTTON};
inline constexpr key mbutton{VK_MBUTTON};
inline constexpr key xbutton1{VK_XBUTTON1};
inline constexpr key xbutton2{VK_XBUTTON2};

inline constexpr key backspace{VK_BACK};
inline constexpr key tab{VK_TAB};
inline constexpr key enter{VK_RETURN};
inline constexpr key shift{VK_SHIFT};
inline constexpr key ctrl{VK_CONTROL};
inline constexpr key alt{VK_MENU};
inline constexpr key caps_lock{VK_CAPITAL};
inline constexpr key escape{VK_ESCAPE};
inline constexpr key space{VK_SPACE};
inline constexpr key page_up{VK_PRIOR};
inline constexpr key page_down{VK_NEXT};
inline constexpr key end{VK_END};
inline constexpr key home{VK_HOME};
inline constexpr key left{VK_LEFT};
inline constexpr key up{VK_UP};
inline constexpr key right{VK_RIGHT};
inline constexpr key down{VK_DOWN};
inline constexpr key print_screen{VK_SNAPSHOT};
inline constexpr key insert{VK_INSERT};
inline constexpr key delete_{VK_DELETE};
inline constexpr key win{VK_LWIN};
inline constexpr key menu{VK_APPS};
inline constexpr key num_lock{VK_NUMLOCK};
inline constexpr key scroll_lock{VK_SCROLL};

inline constexpr key n0{'0'};
inline constexpr key n1{'1'};
inline constexpr key n2{'2'};
inline constexpr key n3{'3'};
inline constexpr key n4{'4'};
inline constexpr key n5{'5'};
inline constexpr key n6{'6'};
inline constexpr key n7{'7'};
inline constexpr key n8{'8'};
inline constexpr key n9{'9'};

inline constexpr key a{'A'};
inline constexpr key b{'B'};
inline constexpr key c{'C'};
inline constexpr key d{'D'};
inline constexpr key e{'E'};
inline constexpr key f{'F'};
inline constexpr key g{'G'};
inline constexpr key h{'H'};
inline constexpr key i{'I'};
inline constexpr key j{'J'};
inline constexpr key k{'K'};
inline constexpr key l{'L'};
inline constexpr key m{'M'};
inline constexpr key n{'N'};
inline constexpr key o{'O'};
inline constexpr key p{'P'};
inline constexpr key q{'Q'};
inline constexpr key r{'R'};
inline constexpr key s{'S'};
inline constexpr key t{'T'};
inline constexpr key u{'U'};
inline constexpr key v{'V'};
inline constexpr key w{'W'};
inline constexpr key x{'X'};
inline constexpr key y{'Y'};
inline constexpr key z{'Z'};

inline constexpr key f1{VK_F1};
inline constexpr key f2{VK_F2};
inline constexpr key f3{VK_F3};
inline constexpr key f4{VK_F4};
inline constexpr key f5{VK_F5};
inline constexpr key f6{VK_F6};
inline constexpr key f7{VK_F7};
inline constexpr key f8{VK_F8};
inline constexpr key f9{VK_F9};
inline constexpr key f10{VK_F10};
inline constexpr key f11{VK_F11};
inline constexpr key f12{VK_F12};

inline constexpr key hiphen{VK_OEM_MINUS};
inline constexpr key semicolon{VK_OEM_1};
inline constexpr key comma{VK_OEM_COMMA};
inline constexpr key period{VK_OEM_PERIOD};
inline constexpr key slash{VK_OEM_2};
} // namespace keys
} // namespace yw

namespace std {

template<> struct formatter<yw::key> {
  formatter<std::string_view> fmt;
  constexpr auto parse(auto& ctx) { return fmt.parse(ctx); }
  auto format(const yw::key& k, auto& ctx) const { return fmt.format(k.name(), ctx); }
};
} // namespace std
