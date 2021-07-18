#pragma once
#include "yw.hpp"
#include "yw.cpp_type.hpp"
#include "yw.cpp_bit.hpp"

namespace yw {

template<quantable T> constexpr T abs(T x)noexcept {
  if constexpr (is_nat<T>) return x;
  return (x < 0) ? -x : x;
}

template<quantable T> constexpr intt ilogb(T val)noexcept {
  if (val == 0) return 0;
  using ValueType = cpp::remove_cv<T>;
  using NatType = nat_type<sizeof T>;
  NatType a = cpp::bit_cast<NatType>(val);
  if constexpr (countable<T>) {
    if constexpr (is_int<T>) {
      static constexpr NatType b = NatType(-1) >> 1;
      a &= b;
    }
    constexpr intt c = sizeof(T) - 1;
    return intt(c - cpp::countl_zero(a));
  }
  else if constexpr (is_fat<T>) {
    static constexpr NatType mask = same_as<cpp::remove_cv<T>, fat4>
      ? NatType(0b0111'1111'1000'0000'0000'0000'0000'0000)
      : NatType(0b0111'1111'1111'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000);
    static constexpr NatType shift = same_as<cpp::remove_cv<T>, fat4> ? NatType(23) : NatType(52);
    static constexpr intt offset = same_as<cpp::remove_cv<T>, fat4> ? -127 : -1023;
    return ((a & mask) >> shift) + offset;
  }
  else static_assert(false);
}

template<quantable T> constexpr auto ldexp(T x, intt exp)noexcept -> decltype(x + 1.0f) {

}

template<quantable T> constexpr auto cos(T x)noexcept -> decltype(x + 1.0f) {

}

}