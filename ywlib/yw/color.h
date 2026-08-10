#pragma once
#include "yw/core.h"
#include "yw/math.h"

#include <cstdint>

namespace yw {

inline constexpr uint8_t _premultiply_channel(uint8_t c, uint8_t a) noexcept {
  return static_cast<uint8_t>((uint32_t(c) * uint32_t(a) + 127) / 255);
}

inline constexpr uint8_t _straighten_channel(uint8_t c, uint8_t a) noexcept {
  if (a == 0) return 0;
  const auto v = (uint32_t(c) * 255 + uint32_t(a) / 2) / uint32_t(a);
  return static_cast<uint8_t>(v > 255 ? 255 : v);
}

//////////////////////////////////////// MARK: rgba

struct rgba {
  uint8_t r{};
  uint8_t g{};
  uint8_t b{};
  uint8_t a{255};
  constexpr rgba() noexcept = default;
  constexpr rgba(integral auto red, integral auto green, integral auto blue, integral auto alpha) noexcept
    : r(static_cast<uint8_t>(red)), g(static_cast<uint8_t>(green)), b(static_cast<uint8_t>(blue)),
      a(static_cast<uint8_t>(alpha)) {}
  constexpr rgba(integral auto red, integral auto green, integral auto blue) noexcept : rgba(red, green, blue, 255) {}
  constexpr rgba(integral auto rrggbb, integral auto alpha) noexcept
    : r(static_cast<uint8_t>((rrggbb >> 16) & 0xFF)), g(static_cast<uint8_t>((rrggbb >> 8) & 0xFF)),
      b(static_cast<uint8_t>(rrggbb & 0xFF)), a(static_cast<uint8_t>(alpha)) {}
  constexpr rgba(integral auto rrggbb) noexcept : rgba(rrggbb, 255) {}

  constexpr rgba premultiplied_alpha() const noexcept {
    return {_premultiply_channel(r, a), _premultiply_channel(g, a), _premultiply_channel(b, a), a};
  }

  constexpr rgba straight_alpha() const noexcept {
    return {_straighten_channel(r, a), _straighten_channel(g, a), _straighten_channel(b, a), a};
  }

  template<uint64_t I> requires(I < 4) constexpr uint8_t& get() noexcept { return select<I>(r, g, b, a); }
  template<uint64_t I> requires(I < 4) constexpr const uint8_t& get() const noexcept { return select<I>(r, g, b, a); }
};
static_assert(sizeof(rgba) == 4);

//////////////////////////////////////// MARK: bgra

struct bgra {
  uint8_t b{};
  uint8_t g{};
  uint8_t r{};
  uint8_t a{255};
  constexpr bgra() noexcept = default;
  constexpr bgra(integral auto blue, integral auto green, integral auto red, integral auto alpha) noexcept
    : b(static_cast<uint8_t>(blue)), g(static_cast<uint8_t>(green)), r(static_cast<uint8_t>(red)),
      a(static_cast<uint8_t>(alpha)) {}
  constexpr bgra(integral auto blue, integral auto green, integral auto red) noexcept : bgra(blue, green, red, 255) {}
  constexpr bgra(integral auto rrggbb, integral auto alpha) noexcept
    : b(static_cast<uint8_t>(rrggbb & 0xFF)), g(static_cast<uint8_t>((rrggbb >> 8) & 0xFF)),
      r(static_cast<uint8_t>((rrggbb >> 16) & 0xFF)), a(static_cast<uint8_t>(alpha)) {}
  constexpr bgra(integral auto rrggbb) noexcept : bgra(rrggbb, 255) {}

  explicit constexpr bgra(const rgba& color) noexcept : bgra(color.b, color.g, color.r, color.a) {}
  explicit constexpr operator rgba() const noexcept { return rgba{r, g, b, a}; }

  constexpr bgra premultiplied_alpha() const noexcept {
    return {_premultiply_channel(b, a), _premultiply_channel(g, a), _premultiply_channel(r, a), a};
  }

  constexpr bgra straight_alpha() const noexcept {
    return {_straighten_channel(b, a), _straighten_channel(g, a), _straighten_channel(r, a), a};
  }

  template<uint64_t I> requires(I < 4) constexpr uint8_t& get() noexcept { return select<I>(b, g, r, a); }
  template<uint64_t I> requires(I < 4) constexpr const uint8_t& get() const noexcept { return select<I>(b, g, r, a); }
};
static_assert(sizeof(bgra) == 4);

//////////////////////////////////////// MARK: color

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

  constexpr color(const rgba& col) noexcept
    : r(static_cast<float>(col.r) / 255.0f), g(static_cast<float>(col.g) / 255.0f),
      b(static_cast<float>(col.b) / 255.0f), a(static_cast<float>(col.a) / 255.0f) {}

  constexpr color(const bgra& col) noexcept
    : r(static_cast<float>(col.r) / 255.0f), g(static_cast<float>(col.g) / 255.0f),
      b(static_cast<float>(col.b) / 255.0f), a(static_cast<float>(col.a) / 255.0f) {}

  explicit operator rgba() const noexcept {
    return rgba{static_cast<uint8_t>(r * 255.0f), static_cast<uint8_t>(g * 255.0f), static_cast<uint8_t>(b * 255.0f),
      static_cast<uint8_t>(a * 255.0f)};
  }
  explicit operator bgra() const noexcept {
    return bgra{static_cast<uint8_t>(b * 255.0f), static_cast<uint8_t>(g * 255.0f), static_cast<uint8_t>(r * 255.0f),
      static_cast<uint8_t>(a * 255.0f)};
  }

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

//////////////////////////////////////// MARK: hsl

struct hsl {
  float h{}; // in radians
  float s{};
  float l{};
  float a{1.0f};

  constexpr hsl() noexcept = default;
  constexpr hsl(arithmetic auto H, arithmetic auto S, arithmetic auto L, arithmetic auto A) noexcept
    : h(static_cast<float>(H)), s(static_cast<float>(S)), l(static_cast<float>(L)), a(static_cast<float>(A)) {}
  constexpr hsl(arithmetic auto H, arithmetic auto S, arithmetic auto L) noexcept : hsl(H, S, L, 1.0f) {}

  static constexpr hsl from_srgb(const color& srgb) noexcept {
    const auto max = yw::max(srgb.r, srgb.g, srgb.b);
    const auto min = yw::min(srgb.r, srgb.g, srgb.b);
    hsl result;
    result.l = (max + min) / 2.0f;
    if (max == min) {
      result.h = 0.0f;
      result.s = 0.0f;
    } else {
      const auto d = max - min;
      result.s = result.l > 0.5f ? d / (2.0f - max - min) : d / (max + min);
      if (max == srgb.r) result.h = (srgb.g - srgb.b) / d + (srgb.g < srgb.b ? 6.0f : 0.0f);
      else if (max == srgb.g) result.h = (srgb.b - srgb.r) / d + 2.0f;
      else if (max == srgb.b) result.h = (srgb.r - srgb.g) / d + 4.0f;
      result.h *= yw::pi / 3.0f;
    }
    result.a = srgb.a;
    return result;
  }

  constexpr color to_srgb() const noexcept {
    auto hue = yw::fmod(h, 2.0f * yw::pi);
    if (hue < 0.0f) hue += 2.0f * yw::pi;
    const auto c = (1.0f - std::fabs(2.0f * l - 1.0f)) * s;
    const auto x = c * (1.0f - std::fabs(std::fmod(hue / (yw::pi / 3.0f), 2.0f) - 1.0f));
    const auto m = l - c / 2.0f;
    float r{}, g{}, b{};
    if (hue < yw::pi / 3.0f) r = c, g = x, b = 0;
    else if (hue < 2.0f * yw::pi / 3.0f) r = x, g = c, b = 0;
    else if (hue < yw::pi) r = 0, g = c, b = x;
    else if (hue < 4.0f * yw::pi / 3.0f) r = 0, g = x, b = c;
    else if (hue < 5.0f * yw::pi / 3.0f) r = x, g = 0, b = c;
    else r = c, g = 0, b = x;
    return color(r + m, g + m, b + m, a);
  }
};

//////////////////////////////////////// MARK: oklab

struct oklab {
  float l{};
  float a{};
  float b{};
  float alpha{1.0f};

  constexpr oklab() noexcept = default;
  constexpr oklab(arithmetic auto L, arithmetic auto A, arithmetic auto B, arithmetic auto Alpha) noexcept
    : l(L), a(A), b(B), alpha(Alpha) {}
  constexpr oklab(arithmetic auto L, arithmetic auto A, arithmetic auto B) noexcept : oklab(L, A, B, 1.0f) {}

  static constexpr oklab from_linear_rgb(const color& rgb) noexcept {
    oklab lab;
    const auto l_ = 0.4122214708f * rgb.r + 0.5363325363f * rgb.g + 0.0514459929f * rgb.b;
    const auto m_ = 0.2119034982f * rgb.r + 0.6806995451f * rgb.g + 0.1073969566f * rgb.b;
    const auto s_ = 0.0883024619f * rgb.r + 0.2817188376f * rgb.g + 0.6299787005f * rgb.b;
    const auto l_cbrt = yw::cbrt(l_);
    const auto m_cbrt = yw::cbrt(m_);
    const auto s_cbrt = yw::cbrt(s_);
    lab.l = 0.2104542553f * l_cbrt + 0.7936177850f * m_cbrt - 0.0040720468f * s_cbrt;
    lab.a = 1.9779984951f * l_cbrt - 2.4285922050f * m_cbrt + 0.4505937099f * s_cbrt;
    lab.b = 0.0259040371f * l_cbrt + 0.7827717662f * m_cbrt - 0.8086757660f * s_cbrt;
    lab.alpha = rgb.a;
    return lab;
  }

  static constexpr oklab from_srgb(const color& srgb) noexcept { return from_linear_rgb(srgb.srgb_to_linear()); }

  constexpr color to_linear_rgb() const noexcept {
    const auto l_ = this->l + 0.3963377774f * this->a + 0.2158037573f * this->b;
    const auto m_ = this->l - 0.1055613458f * this->a - 0.0638541728f * this->b;
    const auto s_ = this->l - 0.0894841775f * this->a - 1.2914855480f * this->b;
    const auto l_cube = l_ * l_ * l_;
    const auto m_cube = m_ * m_ * m_;
    const auto s_cube = s_ * s_ * s_;
    const auto r = -1.2684380046f * l_cube + 2.6097574011f * m_cube - 0.3413193965f * s_cube;
    const auto g = 4.0767416621f * l_cube - 3.3077115913f * m_cube + 0.2309699292f * s_cube;
    const auto b = -0.0041960863f * l_cube - 0.7034186147f * m_cube + 1.7076147010f * s_cube;
    return color(r, g, b, alpha);
  }

  constexpr color to_srgb() const noexcept { return to_linear_rgb().linear_to_srgb(); }
};

static_assert(sizeof(oklab) == 16);

//////////////////////////////////////// MARK: oklch

struct oklch {
  float l{};
  float c{};
  float h{}; // in radians
  float a{1.0f};
  constexpr oklch() noexcept = default;
  constexpr oklch(arithmetic auto L, arithmetic auto C, arithmetic auto H, arithmetic auto A) noexcept
    : l(static_cast<float>(L)), c(static_cast<float>(C)), h(static_cast<float>(H)), a(static_cast<float>(A)) {}
  constexpr oklch(arithmetic auto L, arithmetic auto C, arithmetic auto H) noexcept : oklch(L, C, H, 1.0f) {}
  constexpr oklch(const oklab& Lab) noexcept
    : l(Lab.l), c(yw::hypot(Lab.a, Lab.b)), h(yw::atan2(Lab.b, Lab.a)), a(Lab.alpha) {}
  constexpr operator oklab() const noexcept { return oklab(l, c * cos(h), c * sin(h), a); }

  static constexpr oklch from_linear_rgb(const color& rgb) noexcept { return oklch(oklab::from_linear_rgb(rgb)); }
  static constexpr oklch from_srgb(const color& srgb) noexcept { return from_linear_rgb(srgb.srgb_to_linear()); }

  constexpr color to_linear_rgb() const noexcept {
    const auto a_ = c * cos(h);
    const auto b_ = c * sin(h);
    return oklab(l, a_, b_, a).to_linear_rgb();
  }
  constexpr color to_srgb() const noexcept { return to_linear_rgb().linear_to_srgb(); }
};

static_assert(sizeof(oklch) == 16);

//////////////////////////////////////// MARK: predefined colors

namespace colors {
inline constexpr auto black = color(rgba(0x000000));
inline constexpr auto dimgray = color(rgba(0x696969));
inline constexpr auto gray = color(rgba(0x808080));
inline constexpr auto darkgray = color(rgba(0xa9a9a9));
inline constexpr auto silver = color(rgba(0xc0c0c0));
inline constexpr auto lightgray = color(rgba(0xd3d3d3));
inline constexpr auto gainsboro = color(rgba(0xdcdcdc));
inline constexpr auto whitesmoke = color(rgba(0xf5f5f5));
inline constexpr auto white = color(rgba(0xffffff));
inline constexpr auto snow = color(rgba(0xfffafa));
inline constexpr auto ghostwhite = color(rgba(0xf8f8ff));
inline constexpr auto floralwhite = color(rgba(0xfffaf0));
inline constexpr auto linen = color(rgba(0xfaf0e6));
inline constexpr auto antiquewhite = color(rgba(0xfaebd7));
inline constexpr auto papayawhip = color(rgba(0xffefd5));
inline constexpr auto blanchedalmond = color(rgba(0xffebcd));
inline constexpr auto bisque = color(rgba(0xffe4c4));
inline constexpr auto moccasin = color(rgba(0xffe4b5));
inline constexpr auto navajowhite = color(rgba(0xffdead));
inline constexpr auto peachpuff = color(rgba(0xffdab9));
inline constexpr auto mistyrose = color(rgba(0xffe4e1));
inline constexpr auto lavenderblush = color(rgba(0xfff0f5));
inline constexpr auto seashell = color(rgba(0xfff5ee));
inline constexpr auto oldlace = color(rgba(0xfdf5e6));
inline constexpr auto ivory = color(rgba(0xfffff0));
inline constexpr auto honeydew = color(rgba(0xf0fff0));
inline constexpr auto mintcream = color(rgba(0xf5fffa));
inline constexpr auto azure = color(rgba(0xf0ffff));
inline constexpr auto aliceblue = color(rgba(0xf0f8ff));
inline constexpr auto lavender = color(rgba(0xe6e6fa));
inline constexpr auto lightsteelblue = color(rgba(0xb0c4de));
inline constexpr auto lightslategray = color(rgba(0x778899));
inline constexpr auto slategray = color(rgba(0x708090));
inline constexpr auto steelblue = color(rgba(0x4682b4));
inline constexpr auto royalblue = color(rgba(0x4169e1));
inline constexpr auto midnightblue = color(rgba(0x191970));
inline constexpr auto navy = color(rgba(0x000080));
inline constexpr auto darkblue = color(rgba(0x00008b));
inline constexpr auto mediumblue = color(rgba(0x0000cd));
inline constexpr auto blue = color(rgba(0x0000ff));
inline constexpr auto dodgerblue = color(rgba(0x1e90ff));
inline constexpr auto cornflowerblue = color(rgba(0x6495ed));
inline constexpr auto deepskyblue = color(rgba(0x00bfff));
inline constexpr auto lightskyblue = color(rgba(0x87cefa));
inline constexpr auto skyblue = color(rgba(0x87ceeb));
inline constexpr auto lightblue = color(rgba(0xadd8e6));
inline constexpr auto powderblue = color(rgba(0xb0e0e6));
inline constexpr auto paleturquoise = color(rgba(0xafeeee));
inline constexpr auto lightcyan = color(rgba(0xe0ffff));
inline constexpr auto cyan = color(rgba(0x00ffff));
inline constexpr auto aqua = color(rgba(0x00ffff));
inline constexpr auto turquoise = color(rgba(0x40e0d0));
inline constexpr auto mediumturquoise = color(rgba(0x48d1cc));
inline constexpr auto darkturquoise = color(rgba(0x00ced1));
inline constexpr auto lightseagreen = color(rgba(0x20b2aa));
inline constexpr auto cadetblue = color(rgba(0x5f9ea0));
inline constexpr auto darkcyan = color(rgba(0x008b8b));
inline constexpr auto teal = color(rgba(0x008080));
inline constexpr auto darkslategray = color(rgba(0x2f4f4f));
inline constexpr auto darkgreen = color(rgba(0x006400));
inline constexpr auto green = color(rgba(0x008000));
inline constexpr auto forestgreen = color(rgba(0x228b22));
inline constexpr auto seagreen = color(rgba(0x2e8b57));
inline constexpr auto mediumseagreen = color(rgba(0x3cb371));
inline constexpr auto mediumaquamarine = color(rgba(0x66cdaa));
inline constexpr auto darkseagreen = color(rgba(0x8fbc8f));
inline constexpr auto aquamarine = color(rgba(0x7fffd4));
inline constexpr auto palegreen = color(rgba(0x98fb98));
inline constexpr auto lightgreen = color(rgba(0x90ee90));
inline constexpr auto springgreen = color(rgba(0x00ff7f));
inline constexpr auto mediumspringgreen = color(rgba(0x00fa9a));
inline constexpr auto lawngreen = color(rgba(0x7cfc00));
inline constexpr auto chartreuse = color(rgba(0x7fff00));
inline constexpr auto greenyellow = color(rgba(0xadff2f));
inline constexpr auto lime = color(rgba(0x00ff00));
inline constexpr auto limegreen = color(rgba(0x32cd32));
inline constexpr auto yellowgreen = color(rgba(0x9acd32));
inline constexpr auto darkolivegreen = color(rgba(0x556b2f));
inline constexpr auto olivedrab = color(rgba(0x6b8e23));
inline constexpr auto olive = color(rgba(0x808000));
inline constexpr auto darkkhaki = color(rgba(0xbdb76b));
inline constexpr auto palegoldenrod = color(rgba(0xeee8aa));
inline constexpr auto cornsilk = color(rgba(0xfff8dc));
inline constexpr auto beige = color(rgba(0xf5f5dc));
inline constexpr auto lightyellow = color(rgba(0xffffe0));
inline constexpr auto lightgoldenrodyellow = color(rgba(0xfafad2));
inline constexpr auto lemonchiffon = color(rgba(0xfffacd));
inline constexpr auto wheat = color(rgba(0xf5deb3));
inline constexpr auto burlywood = color(rgba(0xdeb887));
inline constexpr auto tan = color(rgba(0xd2b48c));
inline constexpr auto khaki = color(rgba(0xf0e68c));
inline constexpr auto yellow = color(rgba(0xffff00));
inline constexpr auto gold = color(rgba(0xffd700));
inline constexpr auto orange = color(rgba(0xffa500));
inline constexpr auto sandybrown = color(rgba(0xf4a460));
inline constexpr auto darkorange = color(rgba(0xff8c00));
inline constexpr auto goldenrod = color(rgba(0xdaa520));
inline constexpr auto peru = color(rgba(0xcd853f));
inline constexpr auto darkgoldenrod = color(rgba(0xb8860b));
inline constexpr auto chocolate = color(rgba(0xd2691e));
inline constexpr auto sienna = color(rgba(0xa0522d));
inline constexpr auto saddlebrown = color(rgba(0x8b4513));
inline constexpr auto maroon = color(rgba(0x800000));
inline constexpr auto darkred = color(rgba(0x8b0000));
inline constexpr auto brown = color(rgba(0xa52a2a));
inline constexpr auto firebrick = color(rgba(0xb22222));
inline constexpr auto indianred = color(rgba(0xcd5c5c));
inline constexpr auto rosybrown = color(rgba(0xbc8f8f));
inline constexpr auto darksalmon = color(rgba(0xe9967a));
inline constexpr auto lightcoral = color(rgba(0xf08080));
inline constexpr auto salmon = color(rgba(0xfa8072));
inline constexpr auto lightsalmon = color(rgba(0xffa07a));
inline constexpr auto coral = color(rgba(0xff7f50));
inline constexpr auto tomato = color(rgba(0xff6347));
inline constexpr auto orangered = color(rgba(0xff4500));
inline constexpr auto red = color(rgba(0xff0000));
inline constexpr auto crimson = color(rgba(0xdc143c));
inline constexpr auto mediumvioletred = color(rgba(0xc71585));
inline constexpr auto deeppink = color(rgba(0xff1493));
inline constexpr auto hotpink = color(rgba(0xff69b4));
inline constexpr auto palevioletred = color(rgba(0xdb7093));
inline constexpr auto pink = color(rgba(0xffc0cb));
inline constexpr auto lightpink = color(rgba(0xffb6c1));
inline constexpr auto thistle = color(rgba(0xd8bfd8));
inline constexpr auto magenta = color(rgba(0xff00ff));
inline constexpr auto fuchsia = color(rgba(0xff00ff));
inline constexpr auto violet = color(rgba(0xee82ee));
inline constexpr auto plum = color(rgba(0xdda0dd));
inline constexpr auto orchid = color(rgba(0xda70d6));
inline constexpr auto mediumorchid = color(rgba(0xba55d3));
inline constexpr auto darkorchid = color(rgba(0x9932cc));
inline constexpr auto darkviolet = color(rgba(0x9400d3));
inline constexpr auto darkmagenta = color(rgba(0x8b008b));
inline constexpr auto purple = color(rgba(0x800080));
inline constexpr auto indigo = color(rgba(0x4b0082));
inline constexpr auto darkslateblue = color(rgba(0x483d8b));
inline constexpr auto blueviolet = color(rgba(0x8a2be2));
inline constexpr auto mediumpurple = color(rgba(0x9370db));
inline constexpr auto slateblue = color(rgba(0x6a5acd));
inline constexpr auto mediumslateblue = color(rgba(0x7b68ee));
inline constexpr auto transparent = color(0, 0, 0, 0);
inline constexpr auto yw = color(rgba(0x081020));
} // namespace colors
} // namespace yw

namespace std {

//////////////////////////////////////// MARK: formatter

template<typename C> struct formatter<yw::rgba, C> {
  formatter<int, C> fmt{};
  constexpr auto parse(auto& ctx) { return fmt.parse(ctx); }
  auto format(const yw::rgba& col, auto& ctx) const {
    auto out = ctx.out();
    *out++ = '(', out = fmt.format(col.r, ctx);
    *out++ = ',', out = fmt.format(col.g, ctx);
    *out++ = ',', out = fmt.format(col.b, ctx);
    *out++ = ',', out = fmt.format(col.a, ctx), *out++ = ')';
    return out;
  }
};

template<typename C> struct formatter<yw::bgra, C> {
  formatter<int, C> fmt{};
  constexpr auto parse(auto& ctx) { return fmt.parse(ctx); }
  auto format(const yw::bgra& col, auto& ctx) const {
    auto out = ctx.out();
    *out++ = '(', out = fmt.format(col.b, ctx);
    *out++ = ',', out = fmt.format(col.g, ctx);
    *out++ = ',', out = fmt.format(col.r, ctx);
    *out++ = ',', out = fmt.format(col.a, ctx), *out++ = ')';
    return out;
  }
};

template<typename C> struct formatter<yw::color, C> {
  formatter<float, C> fmt{};
  constexpr auto parse(auto& ctx) { return fmt.parse(ctx); }
  auto format(const yw::color& col, auto& ctx) const {
    auto out = ctx.out();
    *out++ = '(', out = fmt.format(col.r, ctx);
    *out++ = ',', out = fmt.format(col.g, ctx);
    *out++ = ',', out = fmt.format(col.b, ctx);
    *out++ = ',', out = fmt.format(col.a, ctx), *out++ = ')';
    return out;
  }
};

template<typename C> struct formatter<yw::oklab, C> {
  std::formatter<float, C> fmt{};
  constexpr auto parse(auto& ctx) { return fmt.parse(ctx); }
  auto format(const yw::oklab& Lab, auto& ctx) const {
    auto out = ctx.out();
    *out++ = '(', out = fmt.format(Lab.l, ctx);
    *out++ = ',', out = fmt.format(Lab.a, ctx);
    *out++ = ',', out = fmt.format(Lab.b, ctx);
    *out++ = ',', out = fmt.format(Lab.alpha, ctx), *out++ = ')';
    return out;
  }
};

template<typename C> struct formatter<yw::oklch, C> {
  std::formatter<float, C> fmt{};
  constexpr auto parse(auto& ctx) { return fmt.parse(ctx); }
  auto format(const yw::oklch& Lch, auto& ctx) const {
    auto out = ctx.out();
    *out++ = '(', out = fmt.format(Lch.l, ctx);
    *out++ = ',', out = fmt.format(Lch.c, ctx);
    *out++ = ',', out = fmt.format(Lch.h, ctx);
    *out++ = ',', out = fmt.format(Lch.a, ctx), *out++ = ')';
    return out;
  }
};
} // namespace std
