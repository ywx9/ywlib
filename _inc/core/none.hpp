/// \file none.hpp

#pragma once

#ifndef YWLIB
#include <compare>
#include <format>
#include <type_traits>
#else
import std;
#endif


export namespace yw {


using nat = decltype(sizeof(int));


/// struct to represent a null value
struct none {

  /// default constructor
  constexpr none() noexcept = default;

  /// constructor from any arguments
  constexpr none(auto&&...) noexcept {}

  /// assignment from any argument
  constexpr none& operator=(auto&&) noexcept { return *this; }

  /// conversion to `false`
  constexpr explicit operator bool() const noexcept { return false; }

  /// function call operator to do nothing
  constexpr none operator()(auto&&...) const noexcept { return {}; }

  /// equality comparison; always `false`
  friend constexpr bool operator==(none, none) noexcept { return false; }

  /// three-way comparison; always `unordered`
  friend constexpr auto operator<=>(none, none) noexcept {
    return std::partial_ordering::unordered;
  }

  /// unary plus operator; always `none`
  friend constexpr none operator+(none) noexcept { return {}; }

  /// unary minus operator; always `none`
  friend constexpr none operator-(none) noexcept { return {}; }

  /// addition operator; always `none`
  friend constexpr none operator+(none, none) noexcept { return {}; }

  /// subtraction operator; always `none`
  friend constexpr none operator-(none, none) noexcept { return {}; }

  /// multiplication operator; always `none`
  friend constexpr none operator*(none, none) noexcept { return {}; }

  /// division operator; always `none`
  friend constexpr none operator/(none, none) noexcept { return {}; }

  /// addition assignment operator
  constexpr none& operator+=(none) noexcept { return *this; }

  /// subtraction assignment operator
  constexpr none& operator-=(none) noexcept { return *this; }

  /// multiplication assignment operator
  constexpr none& operator*=(none) noexcept { return *this; }

  /// division assignment operator
  constexpr none& operator/=(none) noexcept { return *this; }

}; ///////////////////////////////////////////////////////////////////////////// struct none


/// checks if the type is `none`
template<typename T> concept is_none =
  std::same_as<std::remove_cvref_t<T>, none>;


} ////////////////////////////////////////////////////////////////////////////// namespace yw


namespace std {

// std::common_type

template<typename T> struct common_type<T, yw::none> : type_identity<yw::none> {};
template<typename T> struct common_type<yw::none, T> : type_identity<yw::none> {};

// std::formatter

template<typename Ct> struct formatter<yw::none, Ct> : formatter<const Ct*, Ct> {
  static constexpr const Ct text[] = {'n', 'o', 'n', 'e', '\0'};
  auto format(yw::none, auto& ctx) const {
    return formatter<const Ct*, Ct>::format(text, ctx);
  }
};

} ////////////////////////////////////////////////////////////////////////////// namespace std
