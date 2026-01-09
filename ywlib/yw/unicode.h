#pragma once
#include "yw/core.h"

#include <string>
#include <string_view>

namespace yw {

template<char_type C>
inline constexpr auto unicode = []<stringable S>(S&& s) -> std::basic_string<C> {
  using From = iter_value_t<S>;
  if constexpr (same_as<S&&, std::basic_string<C>&&>) return std::move(s);
  if constexpr (same_as<From, C>) return std::basic_string<C>(std::basic_string_view<C>(s));
  const auto sv_original = std::basic_string_view<From>(s);
  if constexpr (sizeof(From) == sizeof(C))
    return std::basic_string<C>(std::bit_cast<std::basic_string_view<C>>(sv_original));
  using T = select_type<sizeof(From) / 2, char8_t, char16_t, char32_t>;
  const auto sv = std::bit_cast<std::basic_string_view<T>>(sv_original);
  constexpr auto scale = select_value<yw::max(int(sizeof(T)) - int(sizeof(C)), 0), 1, 3, 2, 4>;
  auto r = std::basic_string<C>(sv.size() * scale, C{});
  auto out = r.data();
  for (auto s = sv.data(), end = s + sv.size(); s < end;) {
    char32_t uc;
    if constexpr (same_as<T, char8_t>) {
      const auto c = char32_t(*s);
      const auto i = unsigned(c >= 0xc0) + unsigned(c >= 0xe0) + unsigned(c >= 0xf0);
      const auto j = i + 1 + unsigned(i != 0);
      uc = char32_t(-int(i == 3) & s[i < 3 ? i : 3] & 0x3f);
      uc |= char32_t((-int(i > 1) & s[i < 2 ? i : 2] & 0x3f)) << (6 * (i > 1 ? i - 2 : 0));
      uc |= char32_t((-int(i > 0) & s[i < 1 ? 1 : 1] & 0x3f)) << (6 * (i > 0 ? i - 1 : 0));
      uc |= char32_t(static_cast<char8_t>(c << j) >> j) << (6 * i);
      s += i + 1;
    } else if constexpr (same_as<T, char16_t>) {
      const auto c = char32_t(*s);
      const bool b = (c & 0xff00) == 0xd800;
      uc = c ^ (-int(b) & (c ^ (0x10000 | ((c - 0xd800) << 10 | char32_t(s[b] - 0xdc00)))));
      s += 1 + b;
    } else uc = char32_t(*s++);
    if constexpr (sizeof(C) == 1) {
      const auto i = unsigned(uc >= 0x80) + unsigned(uc >= 0x800) + unsigned(uc >= 0x10000);
      out[i < 3 ? i : 3] = static_cast<C>(0x80 | (uc & 0x3f));
      out[i < 2 ? i : 2] = static_cast<C>(0x80 | ((uc >> (6 * (i > 1 ? i - 2 : 0))) & 0x3f));
      out[i < 1 ? i : 1] = static_cast<C>(0x80 | ((uc >> (6 * (i > 0 ? i - 1 : 0))) & 0x3f));
      *out = static_cast<C>(uint32_t(((i + (i >> 1)) << 4) + (-i & 0xb0)) |
                            ((uc >> (6 * i)) & (0x3f >> i | -int(i == 0))));
      out += i + 1;
    } else if constexpr (sizeof(C) == 2) {
      const bool b = uc >= 0x10000;
      out[b] = static_cast<C>(0xdc00 | (uc & 0x3ff));
      *out = static_cast<C>(uc ^ ((uc ^ (0xd800 | (uc >> 10))) & -int(b)));
      out += 1 + b;
    } else *out++ = static_cast<C>(uc);
  }
  r.resize(out - r.data());
  return r;
};
} // namespace yw
