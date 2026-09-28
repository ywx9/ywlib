#pragma once
#include <base/color.h>
#include <base/format.h>
#include <base/vector.h>
#include <core/core.h>
#include <core/string.h>

#ifdef _WIN32

namespace yw {

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
/// MARK: color_theme

struct color_theme {
  color canvas{0xf0f0f0};              // ex) background of window
  color surface{0xf8f8f8};             // ex) background of control
  color surface_popup{0xffffff};       // ex) background of popup
  color outline = colors::darkgray;    // ex) border of control
  color part = colors::gray;           // ex) button of checkbox, thumb of scrollbar
  color text = colors::black;          // ex) text, icon
  color text_muted = colors::darkgray; // ex) placeholder text
  color accent = colors::dodgerblue;   // ex) focus, selection
  color warning = colors::darkorange;
  color error = colors::crimson;
  color success = colors::seagreen;
};

///--------------------------------------------------------------------------///
/// MARK: key

enum class key : uint8_t {
  unknown = 0,

  lbutton = 0x01,
  rbutton = 0x02,
  mbutton = 0x04,
  xbutton1 = 0x05,
  xbutton2 = 0x06,

  backspace = 0x08,
  tab = 0x09,
  clear = 0x0C,
  enter = 0x0D,
  shift = 0x10,
  ctrl = 0x11,
  alt = 0x12,
  pause = 0x13,
  caps_lock = 0x14,
  escape = 0x1B,

  space = 0x20,
  page_up = 0x21,
  page_down = 0x22,
  end = 0x23,
  home = 0x24,
  left = 0x25,
  up = 0x26,
  right = 0x27,
  down = 0x28,
  print_screen = 0x2A,
  insert = 0x2D,
  delete_ = 0x2E,

  win = 0x5B,
  menu = 0x5D,
  num_lock = 0x90,
  scroll_lock = 0x91,

  n0 = '0', n1 = '1', n2 = '2', n3 = '3', n4 = '4', n5 = '5', n6 = '6', n7 = '7', n8 = '8', n9 = '9',

  a = 'A', b = 'B', c = 'C', d = 'D', e = 'E', f = 'F', g = 'G', h = 'H', i = 'I', j = 'J', k = 'K', l = 'L', m = 'M',
  n = 'N', o = 'O', p = 'P', q = 'Q', r = 'R', s = 'S', t = 'T', u = 'U', v = 'V', w = 'W', x = 'X', y = 'Y', z = 'Z',

  np0 = 0x60, np1 = 0x61, np2 = 0x62, np3 = 0x63, np4 = 0x64,
  np5 = 0x65, np6 = 0x66, np7 = 0x67, np8 = 0x68, np9 = 0x69,

  f1 = 0x70, f2 = 0x71, f3 = 0x72, f4 = 0x73, f5 = 0x74, f6 = 0x75,
  f7 = 0x76, f8 = 0x77, f9 = 0x78, f10 = 0x79, f11 = 0x7A, f12 = 0x7B,

  // OEMからはUS,UK,JPで共通するキーのみ定義

  semicolon = 0xBA,
  plus = 0xBB,
  comma = 0xBC,
  hiphen = 0xBD,
  period = 0xBE,
  slash = 0xBF,
};

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

namespace internal {
constexpr string_view<char> _get_key_name(const key k) {
#ifdef __cpp_lib_meta
  template for (constexpr auto e : std::define_static_array(std::meta::numerators_of(^^key)))
    if (k == [:e:]) return std::meta::identifier_of(e);
#endif
  return "unknown";
}
}

///--------------------------------------------------------------------------///
/// MARK: button_event

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
}

#endif
