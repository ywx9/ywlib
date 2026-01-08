#pragma once
#include "yw/color.h"

namespace yw {

struct oklab {
  float l{}, a{}, b{}, alpha{1.0f};

  constexpr oklab() noexcept = default;
  constexpr oklab(arithmetic auto L, arithmetic auto A, arithmetic auto B,
                  arithmetic auto Alpha) noexcept
    : l(L), a(A), b(B), alpha(Alpha) {}
  constexpr oklab(arithmetic auto L, arithmetic auto A, arithmetic auto B) noexcept
    : oklab(L, A, B, 1.0f) {}

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

  static constexpr oklab from_srgb(const color& srgb) noexcept {
    return from_linear_rgb(srgb.srgb_to_linear());
  }

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

//////////////////////////////////////// MARK: oklch

struct oklch {
  float l{}, c{}, h{}, a{1.0f};
  constexpr oklch() noexcept = default;
  constexpr oklch(arithmetic auto L, arithmetic auto C, arithmetic auto H,
                  arithmetic auto A) noexcept
    : l(L.x), c(C.x), h(H.x), a(A.x) {}
  constexpr oklch(arithmetic auto L, arithmetic auto C, arithmetic auto H) noexcept
    : oklch(L, C, H, 1.0f) {}
  constexpr oklch(const oklab& Lab) noexcept
    : l(Lab.l), c(yw::hypot(Lab.a, Lab.b)), h(yw::atan2(Lab.b, Lab.a)), a(Lab.alpha) {}
  constexpr operator oklab() const noexcept { return oklab(l, c * cos(h), c * sin(h), a); }

  static constexpr oklch from_linear_rgb(const color& rgb) noexcept {
    return oklch(oklab::from_linear_rgb(rgb));
  }
  static constexpr oklch from_srgb(const color& srgb) noexcept {
    return from_linear_rgb(srgb.srgb_to_linear());
  }

  constexpr color to_linear_rgb() const noexcept {
    const auto a_ = c * cos(h);
    const auto b_ = c * sin(h);
    return oklab(l, a_, b_, a).to_linear_rgb();
  }
  constexpr color to_srgb() const noexcept { return to_linear_rgb().linear_to_srgb(); }
};

} // namespace yw

namespace std {

template<typename C> struct formatter<yw::oklab, C> {
  std::formatter<float, C> fmt{};
  constexpr auto parse(auto& ctx) { return fmt.parse(ctx); }
  auto format(const yw::oklab& Lab, auto& ctx) {
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
  auto format(const yw::oklch& Lch, auto& ctx) {
    auto out = ctx.out();
    *out++ = '(', out = fmt.format(Lch.l, ctx);
    *out++ = ',', out = fmt.format(Lch.c, ctx);
    *out++ = ',', out = fmt.format(Lch.h, ctx);
    *out++ = ',', out = fmt.format(Lch.a, ctx), *out++ = ')';
    return out;
  }
};

} // namespace std
