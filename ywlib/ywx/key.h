#pragma once
#include "ywx/core.h"

namespace yw {

enum class key : std::uint_fast16_t {

  // control keys

  escape = 0x1B,
  tab = 0x09,
  enter = 0x0D,
  space = 0x20,
  backspace = 0x08,
  delete_ = 0x2E,
  left = 0x25,
  up = 0x26,
  right = 0x27,
  down = 0x28,
  home = 0x24,
  end = 0x23,
  page_up = 0x21,
  page_down = 0x22,
  insert = 0x2D,

  // character keys

  a = 'a',
  b = 'b',
  c = 'c',
  d = 'd',
  e = 'e',
  f = 'f',
  g = 'g',
  h = 'h',
  i = 'i',
  j = 'j',
  k = 'k',
  l = 'l',
  m = 'm',
  n = 'n',
  o = 'o',
  p = 'p',
  q = 'q',
  r = 'r',
  s = 's',
  t = 't',
  u = 'u',
  v = 'v',
  w = 'w',
  x = 'x',
  y = 'y',
  z = 'z',

  // number keys

  n0 = '0',
  n1 = '1',
  n2 = '2',
  n3 = '3',
  n4 = '4',
  n5 = '5',
  n6 = '6',
  n7 = '7',
  n8 = '8',
  n9 = '9',

  // function keys

  f1 = 0x70,
  f2 = 0x71,
  f3 = 0x72,
  f4 = 0x73,
  f5 = 0x74,
  f6 = 0x75,
  f7 = 0x76,
  f8 = 0x77,
  f9 = 0x78,
  f10 = 0x79,
  f11 = 0x7A,
  f12 = 0x7B,

  // other keys whose position is same in most layouts (US, UK, JP)

  hyphen = 0xBD,
  semicolon = 0xBA,
  comma = 0xBC,
  period = 0xBE,
  slash = 0xBF,
};
}
