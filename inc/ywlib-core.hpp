/// \file ywlib-core.hpp

#pragma once

#include "intrin.hpp"
#include "std.hpp"

#define nat size_t

export namespace yw {

using cat1 = char;
using cat2 = wchar_t;
using uct1 = char8_t;
using uct2 = char16_t;
using uct4 = char32_t;
using int1 = signed char;
using int2 = signed short;
using int4 = signed int;
using int8 = signed long long;
using nat1 = unsigned char;
using nat2 = unsigned short;
using nat4 = unsigned int;
using nat8 = unsigned long long;
using fat4 = float;
using fat8 = double;

using sl_t = long;
using ul_t = unsigned long;
using ld_t = long double;
using np_t = decltype(nullptr);
using po_t = std::partial_ordering;
using wo_t = std::weak_ordering;
using so_t = std::strong_ordering;

consteval cat1 operator""_c1(nat8 n) { return static_cast<cat1>(n); }
consteval cat2 operator""_c2(nat8 n) { return static_cast<cat2>(n); }
consteval uct1 operator""_u1(nat8 n) { return static_cast<uct1>(n); }
consteval uct2 operator""_u2(nat8 n) { return static_cast<uct2>(n); }
consteval uct4 operator""_u4(nat8 n) { return static_cast<uct4>(n); }
consteval int1 operator""_i1(nat8 n) { return static_cast<int1>(n); }
consteval int2 operator""_i2(nat8 n) { return static_cast<int2>(n); }
consteval int4 operator""_i4(nat8 n) { return static_cast<int4>(n); }
consteval int8 operator""_i8(nat8 n) { return static_cast<int8>(n); }
consteval nat1 operator""_n1(nat8 n) { return static_cast<nat1>(n); }
consteval nat2 operator""_n2(nat8 n) { return static_cast<nat2>(n); }
consteval nat4 operator""_n4(nat8 n) { return static_cast<nat4>(n); }
consteval nat8 operator""_n8(nat8 n) { return static_cast<nat8>(n); }
consteval fat4 operator""_f4(nat8 n) { return static_cast<fat4>(n); }
consteval fat8 operator""_f8(nat8 n) { return static_cast<fat8>(n); }
consteval fat4 operator""_f4(ld_t n) { return static_cast<fat4>(n); }
consteval fat8 operator""_f8(ld_t n) { return static_cast<fat8>(n); }
consteval nat operator""_nn(nat8 n) { return static_cast<nat>(n); }

inline constexpr auto npos = nat(-1);
inline constexpr auto unordered = std::partial_ordering::unordered;

/// struct to represent a constant
template<auto V, typename T = decltype(V)>
requires std::convertible_to<decltype(V), T>
struct constant {
  using type = T;
  static constexpr T value = V;
  constexpr operator T() const noexcept { return V; }
  constexpr T operator()() const noexcept { return V; }
};



}
