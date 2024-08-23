/// \file constant.hpp

#pragma once

#ifndef YWLIB
#include <concepts>
#else
import std;
#endif


export namespace yw {


/// struct to represent a constant
template<auto V, typename T = decltype(V)>
requires std::convertible_to<decltype(V), T>
struct constant {
  using type = T;
  static constexpr T value = V;
  constexpr operator T() const noexcept { return V; }
  constexpr T operator()() const noexcept { return V; }
};


} ////////////////////////////////////////////////////////////////////////////// namespace yw
