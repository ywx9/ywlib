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
