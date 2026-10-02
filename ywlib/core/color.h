#pragma once
#include <core/core.h>
#include <core/math.h>
#include <core/string.h>

namespace yw {

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

  // constexpr color(colors c) noexcept
  //   : r(float((uint32_t(c) >> 16) & 0xFF) / 255.0f), g(float((uint32_t(c) >> 8) & 0xFF) / 255.0f),
  //     b(float(uint32_t(c) & 0xFF) / 255.0f), a(1.0f - float((uint32_t(c) >> 24) & 0xFF) / 255.0f) {}

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

namespace colors {
inline constexpr color black{0x000000};
inline constexpr color dimgray{0x696969};
inline constexpr color gray{0x808080};
inline constexpr color darkgray{0xa9a9a9};
inline constexpr color silver{0xc0c0c0};
inline constexpr color lightgray{0xd3d3d3};
inline constexpr color gainsboro{0xdcdcdc};
inline constexpr color whitesmoke{0xf5f5f5};
inline constexpr color white{0xffffff};
inline constexpr color snow{0xfffafa};
inline constexpr color ghostwhite{0xf8f8ff};
inline constexpr color floralwhite{0xfffaf0};
inline constexpr color linen{0xfaf0e6};
inline constexpr color antiquewhite{0xfaebd7};
inline constexpr color papayawhip{0xffefd5};
inline constexpr color blanchedalmond{0xffebcd};
inline constexpr color bisque{0xffe4c4};
inline constexpr color moccasin{0xffe4b5};
inline constexpr color navajowhite{0xffdead};
inline constexpr color peachpuff{0xffdab9};
inline constexpr color mistyrose{0xffe4e1};
inline constexpr color lavenderblush{0xfff0f5};
inline constexpr color seashell{0xfff5ee};
inline constexpr color oldlace{0xfdf5e6};
inline constexpr color ivory{0xfffff0};
inline constexpr color honeydew{0xf0fff0};
inline constexpr color mintcream{0xf5fffa};
inline constexpr color azure{0xf0ffff};
inline constexpr color aliceblue{0xf0f8ff};
inline constexpr color lavender{0xe6e6fa};
inline constexpr color lightsteelblue{0xb0c4de};
inline constexpr color lightslategray{0x778899};
inline constexpr color slategray{0x708090};
inline constexpr color steelblue{0x4682b4};
inline constexpr color royalblue{0x4169e1};
inline constexpr color midnightblue{0x191970};
inline constexpr color navy{0x000080};
inline constexpr color darkblue{0x00008b};
inline constexpr color mediumblue{0x0000cd};
inline constexpr color blue{0x0000ff};
inline constexpr color dodgerblue{0x1e90ff};
inline constexpr color cornflowerblue{0x6495ed};
inline constexpr color deepskyblue{0x00bfff};
inline constexpr color lightskyblue{0x87cefa};
inline constexpr color skyblue{0x87ceeb};
inline constexpr color lightblue{0xadd8e6};
inline constexpr color powderblue{0xb0e0e6};
inline constexpr color paleturquoise{0xafeeee};
inline constexpr color lightcyan{0xe0ffff};
inline constexpr color cyan{0x00ffff};
inline constexpr color aqua{0x00ffff};
inline constexpr color turquoise{0x40e0d0};
inline constexpr color mediumturquoise{0x48d1cc};
inline constexpr color darkturquoise{0x00ced1};
inline constexpr color lightseagreen{0x20b2aa};
inline constexpr color cadetblue{0x5f9ea0};
inline constexpr color darkcyan{0x008b8b};
inline constexpr color teal{0x008080};
inline constexpr color darkslategray{0x2f4f4f};
inline constexpr color darkgreen{0x006400};
inline constexpr color green{0x008000};
inline constexpr color forestgreen{0x228b22};
inline constexpr color seagreen{0x2e8b57};
inline constexpr color mediumseagreen{0x3cb371};
inline constexpr color mediumaquamarine{0x66cdaa};
inline constexpr color darkseagreen{0x8fbc8f};
inline constexpr color aquamarine{0x7fffd4};
inline constexpr color palegreen{0x98fb98};
inline constexpr color lightgreen{0x90ee90};
inline constexpr color springgreen{0x00ff7f};
inline constexpr color mediumspringgreen{0x00fa9a};
inline constexpr color lawngreen{0x7cfc00};
inline constexpr color chartreuse{0x7fff00};
inline constexpr color greenyellow{0xadff2f};
inline constexpr color lime{0x00ff00};
inline constexpr color limegreen{0x32cd32};
inline constexpr color yellowgreen{0x9acd32};
inline constexpr color darkolivegreen{0x556b2f};
inline constexpr color olivedrab{0x6b8e23};
inline constexpr color olive{0x808000};
inline constexpr color darkkhaki{0xbdb76b};
inline constexpr color palegoldenrod{0xeee8aa};
inline constexpr color cornsilk{0xfff8dc};
inline constexpr color beige{0xf5f5dc};
inline constexpr color lightyellow{0xffffe0};
inline constexpr color lightgoldenrodyellow{0xfafad2};
inline constexpr color lemonchiffon{0xfffacd};
inline constexpr color wheat{0xf5deb3};
inline constexpr color burlywood{0xdeb887};
inline constexpr color tan{0xd2b48c};
inline constexpr color khaki{0xf0e68c};
inline constexpr color yellow{0xffff00};
inline constexpr color gold{0xffd700};
inline constexpr color orange{0xffa500};
inline constexpr color sandybrown{0xf4a460};
inline constexpr color darkorange{0xff8c00};
inline constexpr color goldenrod{0xdaa520};
inline constexpr color peru{0xcd853f};
inline constexpr color darkgoldenrod{0xb8860b};
inline constexpr color chocolate{0xd2691e};
inline constexpr color sienna{0xa0522d};
inline constexpr color saddlebrown{0x8b4513};
inline constexpr color maroon{0x800000};
inline constexpr color darkred{0x8b0000};
inline constexpr color brown{0xa52a2a};
inline constexpr color firebrick{0xb22222};
inline constexpr color indianred{0xcd5c5c};
inline constexpr color rosybrown{0xbc8f8f};
inline constexpr color darksalmon{0xe9967a};
inline constexpr color lightcoral{0xf08080};
inline constexpr color salmon{0xfa8072};
inline constexpr color lightsalmon{0xffa07a};
inline constexpr color coral{0xff7f50};
inline constexpr color tomato{0xff6347};
inline constexpr color orangered{0xff4500};
inline constexpr color red{0xff0000};
inline constexpr color crimson{0xdc143c};
inline constexpr color mediumvioletred{0xc71585};
inline constexpr color deeppink{0xff1493};
inline constexpr color hotpink{0xff69b4};
inline constexpr color palevioletred{0xdb7093};
inline constexpr color pink{0xffc0cb};
inline constexpr color lightpink{0xffb6c1};
inline constexpr color thistle{0xd8bfd8};
inline constexpr color magenta{0xff00ff};
inline constexpr color fuchsia{0xff00ff};
inline constexpr color violet{0xee82ee};
inline constexpr color plum{0xdda0dd};
inline constexpr color orchid{0xda70d6};
inline constexpr color mediumorchid{0xba55d3};
inline constexpr color darkorchid{0x9932cc};
inline constexpr color darkviolet{0x9400d3};
inline constexpr color darkmagenta{0x8b008b};
inline constexpr color purple{0x800080};
inline constexpr color indigo{0x4b0082};
inline constexpr color darkslateblue{0x483d8b};
inline constexpr color blueviolet{0x8a2be2};
inline constexpr color mediumpurple{0x9370db};
inline constexpr color slateblue{0x6a5acd};
inline constexpr color mediumslateblue{0x7b68ee};
inline constexpr color transparent{0x000000, 0.0f};
inline constexpr color yw{0x081020};
}; // namespace colors

} // namespace yw
