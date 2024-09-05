/// \file ywlib.hpp
/// \brief ywlib header

#pragma once

#include "intrin.hpp"
#include "std.hpp"

#define nat size_t

#define ywlib_wrapper_auto(...) noexcept(noexcept(__VA_ARGS__)) \
  requires requires { __VA_ARGS__; } { return __VA_ARGS__; }

#define ywlib_wrapper_ref(...) noexcept(noexcept(__VA_ARGS__)) \
  requires requires { __VA_ARGS__; } -> decltype(auto) { return __VA_ARGS__; }

#define ywlib_wrapper_void(...) noexcept(noexcept(__VA_ARGS__)) \
  requires requires { __VA_ARGS__; } { __VA_ARGS__; }

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

inline constexpr nat npos = nat(-1);
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

#pragma region none
export namespace yw {

/// struct to represent a null value
struct none {
  constexpr none() noexcept = default;
  constexpr none(auto&&...) noexcept {}
  constexpr none& operator=(auto&&) noexcept { return *this; }
  constexpr explicit operator bool() const noexcept { return false; }
  constexpr none operator()(auto&&...) const noexcept { return {}; }
  friend constexpr bool operator==(none, none) noexcept { return false; }
  friend constexpr auto operator<=>(none, none) noexcept { return unordered; }
  friend constexpr none operator+(none) noexcept { return {}; }
  friend constexpr none operator-(none) noexcept { return {}; }
  friend constexpr none operator+(none, none) noexcept { return {}; }
  friend constexpr none operator-(none, none) noexcept { return {}; }
  friend constexpr none operator*(none, none) noexcept { return {}; }
  friend constexpr none operator/(none, none) noexcept { return {}; }
  constexpr none& operator+=(none) noexcept { return *this; }
  constexpr none& operator-=(none) noexcept { return *this; }
  constexpr none& operator*=(none) noexcept { return *this; }
  constexpr none& operator/=(none) noexcept { return *this; }
};

/// checks if the type is `none`
template<typename T> concept is_none = std::same_as<std::remove_cvref_t<T>, none>;
}
namespace std {
template<typename T> struct common_type<T, yw::none> : std::type_identity<yw::none> {};
template<typename T> struct common_type<yw::none, T> : std::type_identity<yw::none> {};
}
#pragma endregion
#pragma region select
namespace yw::_ {
template<typename T, T I, typename... Ts> struct _select_type : _select_type<nat, I, Ts...> {};
template<bool B, typename Tt, typename Tf> struct _select_type<bool, B, Tt, Tf> : std::conditional<B, Tt, Tf> {};
template<nat I, typename T, typename... Ts> struct _select_type<nat, I, T, Ts...> : _select_type<nat, I - 1, Ts...> {};
template<typename T, typename... Ts> struct _select_type<nat, 0, T, Ts...> { using type = T; };
}
export namespace yw {

/// checks if the value can be used for selection
template<std::convertible_to<nat> auto I, nat N>
inline constexpr bool selectable =
  (std::same_as<decltype(I), bool> && N == 2) ||
  (!std::same_as<decltype(I), bool> && nat(I) < N);

/// selects a type from the list
template<std::convertible_to<nat> auto I, typename... Ts>
requires selectable<I, sizeof...(Ts)>
using select_type = typename _::_select_type<decltype(I), I, Ts...>::type;

/// selects a constant from the list
template<std::convertible_to<nat> auto I, auto... Vs>
requires selectable<I, sizeof...(Vs)>
inline constexpr auto select_value = select_type<I, constant<Vs>...>::value;

/// selects a parameter from the arguments
template<std::convertible_to<nat> auto I> inline constexpr auto select =
[]<typename T, typename... Ts>(T&& Arg, Ts&&... Args)
  noexcept -> decltype(auto) requires selectable<I, sizeof...(Ts) + 1> {
  if constexpr (std::same_as<decltype(I), nat>) {
    if constexpr (I == 0) return std::forward<T>(Arg);
    else return select<I - 1>(std::forward<Ts>(Args)...);
  } else if constexpr (std::same_as<decltype(I), bool>) {
    return select<nat(!I)>(std::forward<T>(Arg), std::forward<Ts>(Args)...);
  } else return select<nat(I)>(std::forward<T>(Arg), std::forward<Ts>(Args)...);
};
}
#pragma endregion
#pragma region sequence
export namespace yw { template<auto... Vs> struct sequence; }
namespace yw::_ {

template<typename S, typename T> struct _to_sequence;

template<template<auto...> typename Tm, typename T, auto... Vs>
requires (std::convertible_to<decltype(Vs), T> && ...)
struct _to_sequence<Tm<Vs...>, T> { using type = sequence<T(Vs)...>; };

template<template<typename, auto...> typename Tm, typename T, typename U, auto... Vs>
requires (std::convertible_to<decltype(Vs), T> && ...)
struct _to_sequence<Tm<U, Vs...>, T> { using type = sequence<T(Vs)...>; };

template<template<auto...> typename Tm, auto... Vs>
struct _to_sequence<Tm<Vs...>, none> { using type = sequence<Vs...>; };

template<template<typename, auto...> typename Tm, typename U, auto... Vs>
struct _to_sequence<Tm<U, Vs...>, none> { using type = sequence<Vs...>; };

template<typename S, nat N> struct _indices_for : constant<false> {};

template<nat... Is, nat N> struct _indices_for<sequence<Is...>, N>
{ static constexpr bool value = ((Is < N) && ...); };

template<nat I, nat N, auto F, nat...Vs>
struct _make_sequence : _make_sequence<I + 1, N, F, Vs..., F(I)> {};

template<nat N, auto F, nat...Vs>
struct _make_sequence<N, N, F, Vs...> { using type = sequence<Vs...>; };

template<nat I, nat N, typename S, nat... Is> struct _extracting_indices;

template<nat I, nat N, bool... Bs, nat... Is>
struct _extracting_indices<I, N, sequence<Bs...>, Is...>
  : select_type<select_value<I, Bs...>,
                _extracting_indices<I + 1, N, sequence<Bs...>, Is..., I>,
                _extracting_indices<I + 1, N, sequence<Bs...>, Is...>> {};

template<nat N, bool... Bs, nat... Is>
struct _extracting_indices<N, N, sequence<Bs...>, Is...>
{ using type = sequence<Is...>; };

template<typename T, typename S> struct _common_element;

template<typename T, nat... Is> struct _common_element<T, sequence<Is...>>
  : std::common_reference<element_t<T, Is>...> {};

template<typename T, typename U, typename S>
struct _tuple_for : constant<false> {};

template<typename T, typename U, nat... Is>
struct _tuple_for<T, U, sequence<Is...>>
  : constant<(std::convertible_to<element_t<T, Is>, U> && ...) +
             (std::is_nothrow_constructible_v<element_t<T, Is>, U> && ...) * 2> {};
}
export namespace yw {

}

template<auto... Vs> struct sequence;
template<typename... Ts> struct typepack;
template<typename... Ts> struct list;


}

namespace std {
template<auto... Vs> struct tuple_size<yw::sequence<Vs...>> : std::integral_constant<size_t, sizeof...(Vs)> {};
template<typename... Ts> struct tuple_size<yw::typepack<Ts...>> : std::integral_constant<size_t, sizeof...(Ts)> {};
template<typename... Ts> struct tuple_size<yw::list<Ts...>> : std::integral_constant<size_t, sizeof...(Ts)> {};
template<size_t I, auto... Vs> struct tuple_element<I, yw::sequence<Vs...>> : std::type_identity<yw::select_type<I, decltype(Vs)...>> {};//
template<size_t I, typename... Ts> struct tuple_element<I, yw::typepack<Ts...>> : std::type_identity<yw::select_type<I, Ts...>> {};
template<size_t I, typename... Ts> struct tuple_element<I, yw::list<Ts...>> : std::type_identity<yw::select_type<I, Ts...>> {};
}//
