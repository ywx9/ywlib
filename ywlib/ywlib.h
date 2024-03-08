/// @file ywlib/ywlib.h
#pragma once

namespace yw {

using ct = char8_t;
using it = decltype((int*)0 - (int*)0);
using nt = decltype(sizeof(int));
using ft = double;

constexpr ct operator""_ct(unsigned long long n) noexcept { return static_cast<ct>(n); }
constexpr it operator""_it(unsigned long long n) noexcept { return static_cast<it>(n); }
constexpr nt operator""_nt(unsigned long long n) noexcept { return static_cast<nt>(n); }
constexpr ft operator""_ft(unsigned long long n) noexcept { return static_cast<ft>(n); }
constexpr ft operator""_ft(long double n) noexcept { return static_cast<ft>(n); }


}
