#pragma once
#include <core/core.h>
#include <core/math.h>
#include <core/string.h>

namespace yw {

enum class color_name : uint32_t {
  black = 0x000000,
  dimgray = 0x696969,
  gray = 0x808080,
  darkgray = 0xa9a9a9,
  silver = 0xc0c0c0,
  lightgray = 0xd3d3d3,
  gainsboro = 0xdcdcdc,
  whitesmoke = 0xf5f5f5,
  white = 0xffffff,
  snow = 0xfffafa,
  ghostwhite = 0xf8f8ff,
  floralwhite = 0xfffaf0,
  linen = 0xfaf0e6,
  antiquewhite = 0xfaebd7,
  papayawhip = 0xffefd5,
  blanchedalmond = 0xffebcd,
  bisque = 0xffe4c4,
  moccasin = 0xffe4b5,
  navajowhite = 0xffdead,
  peachpuff = 0xffdab9,
  mistyrose = 0xffe4e1,
  lavenderblush = 0xfff0f5,
  seashell = 0xfff5ee,
  oldlace = 0xfdf5e6,
  ivory = 0xfffff0,
  honeydew = 0xf0fff0,
  mintcream = 0xf5fffa,
  azure = 0xf0ffff,
  aliceblue = 0xf0f8ff,
  lavender = 0xe6e6fa,
  lightsteelblue = 0xb0c4de,
  lightslategray = 0x778899,
  slategray = 0x708090,
  steelblue = 0x4682b4,
  royalblue = 0x4169e1,
  midnightblue = 0x191970,
  navy = 0x000080,
  darkblue = 0x00008b,
  mediumblue = 0x0000cd,
  blue = 0x0000ff,
  dodgerblue = 0x1e90ff,
  cornflowerblue = 0x6495ed,
  deepskyblue = 0x00bfff,
  lightskyblue = 0x87cefa,
  skyblue = 0x87ceeb,
  lightblue = 0xadd8e6,
  powderblue = 0xb0e0e6,
  paleturquoise = 0xafeeee,
  lightcyan = 0xe0ffff,
  cyan = 0x00ffff,
  aqua = 0x00ffff,
  turquoise = 0x40e0d0,
  mediumturquoise = 0x48d1cc,
  darkturquoise = 0x00ced1,
  lightseagreen = 0x20b2aa,
  cadetblue = 0x5f9ea0,
  darkcyan = 0x008b8b,
  teal = 0x008080,
  darkslategray = 0x2f4f4f,
  darkgreen = 0x006400,
  green = 0x008000,
  forestgreen = 0x228b22,
  seagreen = 0x2e8b57,
  mediumseagreen = 0x3cb371,
  mediumaquamarine = 0x66cdaa,
  darkseagreen = 0x8fbc8f,
  aquamarine = 0x7fffd4,
  palegreen = 0x98fb98,
  lightgreen = 0x90ee90,
  springgreen = 0x00ff7f,
  mediumspringgreen = 0x00fa9a,
  lawngreen = 0x7cfc00,
  chartreuse = 0x7fff00,
  greenyellow = 0xadff2f,
  lime = 0x00ff00,
  limegreen = 0x32cd32,
  yellowgreen = 0x9acd32,
  darkolivegreen = 0x556b2f,
  olivedrab = 0x6b8e23,
  olive = 0x808000,
  darkkhaki = 0xbdb76b,
  palegoldenrod = 0xeee8aa,
  cornsilk = 0xfff8dc,
  beige = 0xf5f5dc,
  lightyellow = 0xffffe0,
  lightgoldenrodyellow = 0xfafad2,
  lemonchiffon = 0xfffacd,
  wheat = 0xf5deb3,
  burlywood = 0xdeb887,
  tan = 0xd2b48c,
  khaki = 0xf0e68c,
  yellow = 0xffff00,
  gold = 0xffd700,
  orange = 0xffa500,
  sandybrown = 0xf4a460,
  darkorange = 0xff8c00,
  goldenrod = 0xdaa520,
  peru = 0xcd853f,
  darkgoldenrod = 0xb8860b,
  chocolate = 0xd2691e,
  sienna = 0xa0522d,
  saddlebrown = 0x8b4513,
  maroon = 0x800000,
  darkred = 0x8b0000,
  brown = 0xa52a2a,
  firebrick = 0xb22222,
  indianred = 0xcd5c5c,
  rosybrown = 0xbc8f8f,
  darksalmon = 0xe9967a,
  lightcoral = 0xf08080,
  salmon = 0xfa8072,
  lightsalmon = 0xffa07a,
  coral = 0xff7f50,
  tomato = 0xff6347,
  orangered = 0xff4500,
  red = 0xff0000,
  crimson = 0xdc143c,
  mediumvioletred = 0xc71585,
  deeppink = 0xff1493,
  hotpink = 0xff69b4,
  palevioletred = 0xdb7093,
  pink = 0xffc0cb,
  lightpink = 0xffb6c1,
  thistle = 0xd8bfd8,
  magenta = 0xff00ff,
  fuchsia = 0xff00ff,
  violet = 0xee82ee,
  plum = 0xdda0dd,
  orchid = 0xda70d6,
  mediumorchid = 0xba55d3,
  darkorchid = 0x9932cc,
  darkviolet = 0x9400d3,
  darkmagenta = 0x8b008b,
  purple = 0x800080,
  indigo = 0x4b0082,
  darkslateblue = 0x483d8b,
  blueviolet = 0x8a2be2,
  mediumpurple = 0x9370db,
  slateblue = 0x6a5acd,
  mediumslateblue = 0x7b68ee,
  transparent = 0xFF000000,
  yw = 0x081020,
};

struct color {
  float r{};
  float g{};
  float b{};
  float a{1.0f};

  constexpr color() noexcept = default;

  constexpr color(const color& Color, arithmetic auto Alpha) noexcept
    : r(Color.r), g(Color.g), b(Color.b), a(static_cast<float>(Alpha)) {}

  constexpr color(arithmetic auto red, arithmetic auto green, arithmetic auto blue, arithmetic auto alpha) noexcept
    : r(static_cast<float>(red)), g(static_cast<float>(green)), b(static_cast<float>(blue)),
      a(static_cast<float>(alpha)) {}

  constexpr color(arithmetic auto red, arithmetic auto green, arithmetic auto blue) noexcept
    : color(red, green, blue, 1.0f) {}

  constexpr color(integral auto rrggbb, arithmetic auto alpha) noexcept
    : r(static_cast<float>((rrggbb >> 16) & 0xFF) / 255.0f), g(static_cast<float>((rrggbb >> 8) & 0xFF) / 255.0f),
      b(static_cast<float>(rrggbb & 0xFF) / 255.0f), a(static_cast<float>(alpha)) {}

  constexpr color(integral auto rrggbb) noexcept : color(rrggbb, 1.0f) {}

  constexpr color(color_name c) noexcept
    : r(float((uint32_t(c) >> 16) & 0xFF) / 255.0f), g(float((uint32_t(c) >> 8) & 0xFF) / 255.0f),
      b(float(uint32_t(c) & 0xFF) / 255.0f), a(1.0f - float((uint32_t(c) >> 24) & 0xFF) / 255.0f) {}

  constexpr color srgb_to_linear() const noexcept {
    constexpr auto fn = [](float c) noexcept {
      return c <= 0.04045f ? c / 12.92f : yw::pow((c + 0.055f) / 1.055f, 2.4f);
    };
    return color(fn(r), fn(g), fn(b), a);
  }

  constexpr color linear_to_srgb() const noexcept {
    constexpr auto fn = [](float c) noexcept {
      return c <= 0.0031308f ? c * 12.92f : 1.055f * yw::pow(c, 1.0f / 2.4f) - 0.055f;
    };
    return color(fn(r), fn(g), fn(b), a);
  }

  template<uint64_t I> requires(I < 4) constexpr float& get() noexcept { return select<I>(r, g, b, a); }
  template<uint64_t I> requires(I < 4) constexpr const float& get() const noexcept { return select<I>(r, g, b, a); }

  template<char_type C> string<C> to_string() const {
    string<C> result;
    result += C('('), result += vtos<C>(r);
    result += C(','), result += vtos<C>(g);
    result += C(','), result += vtos<C>(b);
    result += C(','), result += vtos<C>(a), result += C(')');
    return result;
  }
};
static_assert(sizeof(color) == 16);

} // namespace yw
