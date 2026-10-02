#pragma once
#include <core/core.h>
#include <core/string.h>

namespace yw {

///--------------------------------------------------------------------------///
/// MARK: key

namespace internal {
inline constexpr const char _key_names[] =
  "Unknown\0LButton\0RButton\0MButton\0XButton1\0XButton2\0"
  "Backspace\0Tab\0Clear\0Enter\0Shift\0Ctrl\0Alt\0Pause\0CapsLock\0Escape\0"
  "Space\0PageUp\0PageDown\0End\0Home\0Left\0Up\0Right\0Down\0PrintScreen\0Insert\0Delete\0"
  "Win\0Menu\0NumLock\0ScrollLock\0"
  "Semicolon\0Plus\0Comma\0Hyphen\0Period\0Slash";
inline constexpr array<uint8_t, 256> _key_code_table = [] {
  array<uint8_t, 256> table;
  std::ranges::fill(table, uint8_t(0));
  constexpr auto f = [](string_view<char> name) {
    if (const auto sr = std::ranges::search(_key_names, name); sr.empty()) throw "Key name not found";
    else if (const auto i = sr.begin() - _key_names; i >= 0xFE) throw "Key name index out of range";
    else return uint8_t(i);
  };
  table[VK_LBUTTON] = f("LButton");
  table[VK_RBUTTON] = f("RButton");
  table[VK_MBUTTON] = f("MButton");
  table[VK_XBUTTON1] = f("XButton1");
  table[VK_XBUTTON2] = f("XButton2");
  table[VK_BACK] = f("Backspace");
  table[VK_TAB] = f("Tab");
  table[VK_CLEAR] = f("Clear");
  table[VK_RETURN] = f("Enter");
  table[VK_SHIFT] = f("Shift");
  table[VK_CONTROL] = f("Ctrl");
  table[VK_MENU] = f("Alt");
  table[VK_PAUSE] = f("Pause");
  table[VK_CAPITAL] = f("CapsLock");
  table[VK_ESCAPE] = f("Escape");
  table[VK_SPACE] = f("Space");
  table[VK_PRIOR] = f("PageUp");
  table[VK_NEXT] = f("PageDown");
  table[VK_END] = f("End");
  table[VK_HOME] = f("Home");
  table[VK_LEFT] = f("Left");
  table[VK_UP] = f("Up");
  table[VK_RIGHT] = f("Right");
  table[VK_DOWN] = f("Down");
  table[VK_SNAPSHOT] = f("PrintScreen");
  table[VK_INSERT] = f("Insert");
  table[VK_DELETE] = f("Delete");
  table[VK_LWIN] = f("Win");
  table[VK_APPS] = f("Menu");
  table[VK_NUMLOCK] = f("NumLock");
  table[VK_SCROLL] = f("ScrollLock");
  table[VK_OEM_1] = f("Semicolon");
  table[VK_OEM_PLUS] = f("Plus");
  table[VK_OEM_COMMA] = f("Comma");
  table[VK_OEM_MINUS] = f("Hyphen");
  table[VK_OEM_PERIOD] = f("Period");
  table[VK_OEM_2] = f("Slash");
  for (size_t i = '0'; i <= '9'; ++i) table[i] = uint8_t(0xFF);
  for (size_t i = 'A'; i <= 'Z'; ++i) table[i] = uint8_t(0xFF);
  for (size_t i = VK_F1; i <= VK_F12; ++i) table[i] = uint8_t(0xFE);
  return table;
}();
} // namespace internal

struct key {
  uint8_t code; ///< virtual key code
  constexpr string<char> to_string() const noexcept {
    const auto i = internal::_key_code_table[code];
    if (i == 0xFF) return string<char>(1, char(code));
    if (i == 0xFE) {
      const auto n = i + 1u - VK_F1; // F1 -> 1
      auto s = string<char>("F00");
      if (n < 10) {
        s[1] = char('0' + n);
        s.resize(2); // Remove the leading '0' for single-digit F keys
      } else s[1] = char('0' + n / 10), s[2] = char('0' + n % 10);
      return s;
    } else return string<char>(internal::_key_names + i);
  }
  friend constexpr bool operator==(key a, key b) noexcept { return a.code == b.code; }
};

namespace keys {
inline constexpr key unknown{0};

inline constexpr key lbutton{0x01};
inline constexpr key rbutton{0x02};
inline constexpr key mbutton{0x04};
inline constexpr key xbutton1{0x05};
inline constexpr key xbutton2{0x06};

inline constexpr key backspace{0x08};
inline constexpr key tab{0x09};
inline constexpr key clear{0x0C};
inline constexpr key enter{0x0D};
inline constexpr key shift{0x10};
inline constexpr key ctrl{0x11};
inline constexpr key alt{0x12};
inline constexpr key pause{0x13};
inline constexpr key caps_lock{0x14};
inline constexpr key escape{0x1B};

inline constexpr key space{0x20};
inline constexpr key page_up{0x21};
inline constexpr key page_down{0x22};
inline constexpr key end{0x23};
inline constexpr key home{0x24};
inline constexpr key left{0x25};
inline constexpr key up{0x26};
inline constexpr key right{0x27};
inline constexpr key down{0x28};
inline constexpr key print_screen{0x2A};
inline constexpr key insert{0x2D};
inline constexpr key delete_{0x2E};

inline constexpr key win{0x5B};
inline constexpr key menu{0x5D};
inline constexpr key num_lock{0x90};
inline constexpr key scroll_lock{0x91};

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

inline constexpr key np0{0x60};
inline constexpr key np1{0x61};
inline constexpr key np2{0x62};
inline constexpr key np3{0x63};
inline constexpr key np4{0x64};
inline constexpr key np5{0x65};
inline constexpr key np6{0x66};
inline constexpr key np7{0x67};
inline constexpr key np8{0x68};
inline constexpr key np9{0x69};

inline constexpr key f1{0x70};
inline constexpr key f2{0x71};
inline constexpr key f3{0x72};
inline constexpr key f4{0x73};
inline constexpr key f5{0x74};
inline constexpr key f6{0x75};
inline constexpr key f7{0x76};
inline constexpr key f8{0x77};
inline constexpr key f9{0x78};
inline constexpr key f10{0x79};
inline constexpr key f11{0x7A};
inline constexpr key f12{0x7B};

/// \note OEMからはUS;UK;JPで共通するキーのみ定義

inline constexpr key semicolon{0xBA};
inline constexpr key plus{0xBB};
inline constexpr key comma{0xBC};
inline constexpr key hiphen{0xBD};
inline constexpr key period{0xBE};
inline constexpr key slash{0xBF};
}; // namespace keys

///--------------------------------------------------------------------------///
/// MARK: key_state

struct key_state {
  bool down : 1;
  bool shift : 1;
  bool ctrl : 1;
  bool alt : 1;
  constexpr string<char> to_string() const {
    auto s = string<char>("(down:0, shift:0, ctrl:0, alt:0)");
    s[6] = char('0' + down);
    s[15] = char('0' + shift);
    s[23] = char('0' + ctrl);
    s[30] = char('0' + alt);
    return s;
  }
};
} // namespace yw
