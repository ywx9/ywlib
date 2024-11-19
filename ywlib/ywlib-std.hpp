#pragma once

#ifndef nat
#define nat size_t
#endif

#include <algorithm>
#include <any>
#include <array>
#include <atomic>
#include <barrier>
#include <bit>
#include <bitset>
#include <cassert>
#include <cctype>
#include <cerrno>
#include <cfenv>
#include <cfloat>
#include <charconv>
#include <chrono>
#include <cinttypes>
#include <climits>
#include <clocale>
#include <cmath>
#include <codecvt>
#include <compare>
#include <complex>
#include <concepts>
#include <condition_variable>
#include <coroutine>
#include <csetjmp>
#include <csignal>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cuchar>
#include <cwchar>
#include <cwctype>
#include <deque>
#include <exception>
#include <execution>
#include <expected>
#include <filesystem>
#include <format>
#include <forward_list>
#include <fstream>
#include <functional>
#include <future>
#include <initializer_list>
#include <iomanip>
#include <ios>
#include <iosfwd>
#include <iostream>
#include <istream>
#include <iterator>
#include <latch>
#include <limits>
#include <list>
#include <locale>
#include <map>
#include <memory>
#include <memory_resource>
#include <mutex>
#include <new>
#include <numbers>
#include <numeric>
#include <optional>
#include <ostream>
#include <print>
#include <queue>
#include <random>
#include <ranges>
#include <ratio>
#include <regex>
#include <scoped_allocator>
#include <semaphore>
#include <set>
#include <shared_mutex>
#include <source_location>
#include <span>
#include <spanstream>
#include <sstream>
#include <stack>
#include <stacktrace>
#include <stdexcept>
#include <stdfloat>
#include <stop_token>
#include <streambuf>
#include <string>
#include <string_view>
#include <strstream>
#include <syncstream>
#include <system_error>
#include <thread>
#include <tuple>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <valarray>
#include <variant>
#include <vector>
#include <version>

#define ywlib_wrap_auto(...)                                                                 \
  noexcept(noexcept(__VA_ARGS__)) requires requires { __VA_ARGS__; } { return __VA_ARGS__; }
#define ywlib_wrap_void(...)                                                          \
  noexcept(noexcept(__VA_ARGS__)) requires requires { __VA_ARGS__; } { __VA_ARGS__; }
#define ywlib_wrap_ref(...)                                                                                  \
  noexcept(noexcept(__VA_ARGS__))->decltype(auto) requires requires { __VA_ARGS__; } { return __VA_ARGS__; }

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

inline constexpr nat npos = nat(-1);
inline constexpr auto unordered = std::partial_ordering::unordered;

consteval cat1 operator""_c1(nat n) noexcept { return static_cast<cat1>(n); }
consteval cat2 operator""_c2(nat n) noexcept { return static_cast<cat2>(n); }
consteval uct1 operator""_u1(nat n) noexcept { return static_cast<uct1>(n); }
consteval uct2 operator""_u2(nat n) noexcept { return static_cast<uct2>(n); }
consteval uct4 operator""_u4(nat n) noexcept { return static_cast<uct4>(n); }
consteval int1 operator""_i1(nat n) noexcept { return static_cast<int1>(n); }
consteval int2 operator""_i2(nat n) noexcept { return static_cast<int2>(n); }
consteval int4 operator""_i4(nat n) noexcept { return static_cast<int4>(n); }
consteval int8 operator""_i8(nat n) noexcept { return static_cast<int8>(n); }
consteval nat1 operator""_n1(nat n) noexcept { return static_cast<nat1>(n); }
consteval nat2 operator""_n2(nat n) noexcept { return static_cast<nat2>(n); }
consteval nat4 operator""_n4(nat n) noexcept { return static_cast<nat4>(n); }
consteval nat8 operator""_n8(nat n) noexcept { return static_cast<nat8>(n); }
consteval fat4 operator""_f4(nat n) noexcept { return static_cast<fat4>(n); }
consteval fat8 operator""_f8(nat n) noexcept { return static_cast<fat8>(n); }
consteval fat4 operator""_f4(ld_t n) noexcept { return static_cast<fat4>(n); }
consteval fat8 operator""_f8(ld_t n) noexcept { return static_cast<fat8>(n); }
consteval nat operator""_n(nat n) noexcept { return n; }

struct pass {
  template<typename T> constexpr T&& operator()(T&& a) const noexcept { return static_cast<T&&>(a); }
};

struct none {
  constexpr none() noexcept = default;
  constexpr none(auto&&...) noexcept {}
  constexpr none& operator=(auto&&) noexcept { return *this; }
  explicit constexpr operator bool() const noexcept { return false; }
  constexpr none operator()() const noexcept { return {}; }
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

}

namespace std {

template<typename T> struct common_type<yw::none, T> : type_identity<yw::none> {};
template<typename T> struct common_type<T, yw::none> : type_identity<yw::none> {};

}

namespace yw::_ {

template<typename T> struct _type : std::type_identity<T> {};
template<typename T> concept _valid = requires { typename _type<T>::type; };

template<nat N> struct _cat_type : _type<none> {};
template<> struct _cat_type<1> : _type<yw::cat1> {};
template<> struct _cat_type<2> : _type<yw::cat2> {};

template<nat N> struct _uct_type : _type<none> {};
template<> struct _uct_type<1> : _type<yw::uct1> {};
template<> struct _uct_type<2> : _type<yw::uct2> {};
template<> struct _uct_type<4> : _type<yw::uct4> {};

template<nat N> struct _int_type : _type<none> {};
template<> struct _int_type<1> : _type<yw::int1> {};
template<> struct _int_type<2> : _type<yw::int2> {};
template<> struct _int_type<4> : _type<yw::int4> {};
template<> struct _int_type<8> : _type<yw::int8> {};

template<nat N> struct _nat_type : _type<none> {};
template<> struct _nat_type<1> : _type<yw::nat1> {};
template<> struct _nat_type<2> : _type<yw::nat2> {};
template<> struct _nat_type<4> : _type<yw::nat4> {};
template<> struct _nat_type<8> : _type<yw::nat8> {};

template<nat N> struct _fat_type : _type<none> {};
template<> struct _fat_type<4> : _type<yw::fat4> {};
template<> struct _fat_type<8> : _type<yw::fat8> {};

template<typename T> struct _remove_all_pointers : _type<T> {};
template<typename T> struct _remove_all_pointers<T*> : _remove_all_pointers<T> {};
template<typename T> struct _remove_all_pointers<T* const> : _remove_all_pointers<T> {};
template<typename T> struct _remove_all_pointers<T* volatile> : _remove_all_pointers<T> {};
template<typename T> struct _remove_all_pointers<T* const volatile> : _remove_all_pointers<T> {};

}

export namespace yw {

template<typename... Ts> concept valid = (_::_valid<Ts> && ...);

template<nat N> using cat_type = typename _::_cat_type<N>::type;
template<nat N> using uct_type = typename _::_uct_type<N>::type;
template<nat N> using int_type = typename _::_int_type<N>::type;
template<nat N> using nat_type = typename _::_nat_type<N>::type;
template<nat N> using fat_type = typename _::_fat_type<N>::type;

template<typename T, typename... Ts> concept same_as = (std::same_as<T, Ts> && ...);
template<typename T, typename... Ts> concept included_in = (std::same_as<T, Ts> || ...);
template<typename T, typename... Ts> concept different_from = !included_in<T, Ts...>;
template<typename T, typename... Ts> concept derived_from = (std::derived_from<T, Ts> && ...);
template<typename T, typename... Ts> concept convertible_to = (std::convertible_to<T, Ts> && ...);
template<typename T, typename... Ts> concept nt_convertible_to = convertible_to<T, Ts...> && (std::is_nothrow_convertible_v<T, Ts> && ...);
template<typename T, typename... Ts> concept castable_to = requires(T (&f)()) { ((static_cast<Ts>(f())) && ...); };
template<typename T, typename... Ts> concept nt_castable_to = castable_to<T, Ts...> && requires(T (&f)() noexcept) { requires (noexcept(static_cast<Ts>(f())) && ...); };

template<typename T> using remove_cv = std::remove_cv_t<T>;
template<typename T> using remove_const = std::remove_const_t<T>;
template<typename T> using remove_volatile = std::remove_volatile_t<T>;
template<typename T> using remove_ref = std::remove_reference_t<T>;
template<typename T> using remove_cvref = std::remove_cvref_t<T>;
template<typename T> using remove_extent = std::remove_extent_t<T>;
template<typename T> using remove_all_extents = std::remove_all_extents_t<T>;
template<typename T> using remove_pointer = std::remove_pointer_t<T>;
template<typename T> using remove_all_pointers = typename _::_remove_all_pointers<T>::type;

template<typename T> concept is_const = std::is_const_v<T>;
template<typename T> concept is_volatile = std::is_volatile_v<T>;
template<typename T> concept is_cv = is_const<T> && is_volatile<T>;
template<typename T> concept is_lvref = std::is_lvalue_reference_v<T>;
template<typename T> concept is_rvref = std::is_rvalue_reference_v<T>;
template<typename T> concept is_reference = is_lvref<T> || is_rvref<T>;
template<typename T> concept is_pointer = std::is_pointer_v<T>;
template<typename T> concept is_bounded_array = std::is_bounded_array_v<T>;
template<typename T> concept is_unbounded_array = std::is_unbounded_array_v<T>;
template<typename T> concept is_array = is_bounded_array<T> || is_unbounded_array<T>;
template<typename T> concept is_function = std::is_function_v<T>;

template<typename T> concept integral = std::integral<T>;
template<typename T> concept signed_integral = std::signed_integral<T>;
template<typename T> concept unsigned_integral = std::unsigned_integral<T>;
template<typename T> concept floating_point = std::floating_point<T>;
template<typename T> concept arithmetic = integral<T> || floating_point<T>;

template<typename T> concept is_void = same_as<remove_cv<T>, void>;
template<typename T> concept is_bool = same_as<remove_cv<T>, bool>;
template<typename T> concept is_none = same_as<remove_cv<T>, none>;
template<typename T> concept is_nullptr = same_as<remove_cv<T>, decltype(nullptr)>;
template<typename T> concept is_cat = included_in<remove_cv<T>, cat1, cat2>;
template<typename T> concept is_uct = included_in<remove_cv<T>, uct1, uct2, uct4>;
template<typename T> concept character = is_cat<T> || is_uct<T>;
template<typename T> concept is_int = included_in<remove_cv<T>, int1, int2, int4, int8>;
template<typename T> concept is_nat = included_in<remove_cv<T>, nat1, nat2, nat4, nat8>;
template<typename T> concept is_fat = included_in<remove_cv<T>, fat4, fat8>;
template<typename T> concept numeric = nt_castable_to<T, bool, int4, int8, nat4, nat8, fat4, fat8>;
template<typename T> concept scalar = arithmetic<T> || is_pointer<T> || is_nullptr<T> || is_none<T>;

template<typename T> concept is_class = std::is_class_v<T>;
template<typename T> concept is_union = std::is_union_v<T>;
template<typename T> concept is_abstract = is_class<T> && std::is_abstract_v<T>;
template<typename T> concept is_aggregate = std::is_aggregate_v<T>;
template<typename T> concept is_empty = is_class<T> && std::is_empty_v<T>;
template<typename T> concept is_final = is_class<T> && std::is_final_v<T>;
template<typename T> concept is_polymorphic = is_class<T> && std::is_polymorphic_v<T>;
template<typename T> concept has_virtual_destructor = is_class<T> && std::has_virtual_destructor_v<T>;
template<typename T> concept standard_layout = std::is_standard_layout_v<T>;
template<typename T> concept trivially_copyable = std::is_trivially_copyable_v<T>;

// clang-format off

template<typename... Fs> struct overload : public Fs... { using Fs::operator()...; };

struct equal { template<typename T, typename U> constexpr bool operator()(T&& a, U&& b) const ywlib_wrap_auto(a == b) };
struct not_equal { template<typename T, typename U> constexpr bool operator()(T&& a, U&& b) const ywlib_wrap_auto(a != b) };
struct less { template<typename T, typename U> constexpr bool operator()(T&& a, U&& b) const ywlib_wrap_auto(a < b) };
struct less_equal { template<typename T, typename U> constexpr bool operator()(T&& a, U&& b) const ywlib_wrap_auto(a <= b) };
struct greater { template<typename T, typename U> constexpr bool operator()(T&& a, U&& b) const ywlib_wrap_auto(a > b) };
struct greater_equal { template<typename T, typename U> constexpr bool operator()(T&& a, U&& b) const ywlib_wrap_auto(a >= b) };

// clang-format on

inline constexpr equal eq;
inline constexpr not_equal ne;
inline constexpr less lt;
inline constexpr less_equal le;
inline constexpr greater gt;
inline constexpr greater_equal ge;

template<auto V, typename T = decltype(V)> requires convertible_to<decltype(V), T> struct constant {
  using type = T;
  static constexpr type value = V;
  consteval operator type() const noexcept { return value; }
  consteval type operator()() const noexcept { return value; }
};

struct value {
  fat8 _value{};
  constexpr value() noexcept = default;
  constexpr value(none) noexcept : _value(std::numeric_limits<fat8>::quiet_NaN()) {}
  template<numeric T> requires different_from<T, value> constexpr value(T&& Value) noexcept : _value(static_cast<fat8>(Value)) {}
  template<numeric T> requires different_from<T, value> constexpr value& operator=(T&& Value) noexcept { return _value = static_cast<fat8>(Value), *this; }
  constexpr operator fat8&() & noexcept { return _value; }
  constexpr operator const fat8&() const& noexcept { return _value; }
  constexpr operator fat8&&() && noexcept { return static_cast<fat8&&>(_value); }
  constexpr operator const fat8&&() const&& noexcept { return static_cast<const fat8&&>(_value); }
  template<typename T> requires castable_to<fat8, T> constexpr operator T() const noexcept(nt_castable_to<fat8, T>) { return static_cast<T>(_value); }
  template<numeric T> constexpr value& operator+=(T&& Value) noexcept { return _value += static_cast<fat8>(Value), *this; }
  template<numeric T> constexpr value& operator-=(T&& Value) noexcept { return _value -= static_cast<fat8>(Value), *this; }
  template<numeric T> constexpr value& operator*=(T&& Value) noexcept { return _value *= static_cast<fat8>(Value), *this; }
  template<numeric T> constexpr value& operator/=(T&& Value) noexcept { return _value /= static_cast<fat8>(Value), *this; }
};

inline constexpr value e = std::numbers::e_v<fat8>;
inline constexpr value pi = std::numbers::pi_v<fat8>;
inline constexpr value phi = std::numbers::phi_v<fat8>;
inline constexpr value gamma = std::numbers::egamma_v<fat8>;
inline constexpr value ln2 = std::numbers::ln2_v<fat8>;
inline constexpr value ln10 = std::numbers::ln10_v<fat8>;
inline constexpr value log2_10 = 3.3219280948873623478703194294894;
inline constexpr value log10_2 = 0.30102999566398119521373889472449;
inline constexpr value sqrt2 = std::numbers::sqrt2_v<fat8>;
inline constexpr value sqrt3 = std::numbers::sqrt3_v<fat8>;
inline constexpr value inf = std::numeric_limits<fat8>::infinity();
inline constexpr value nan = std::numeric_limits<fat8>::quiet_NaN();

}

namespace std {

template<typename T> struct common_type<yw::value, T> : std::common_type<yw::fat8, T> {};
template<typename T> struct common_type<T, yw::value> : std::common_type<T, yw::fat8> {};

}

namespace yw::_ {

template<typename T> struct _add_const : _type<const T> {};
template<typename T> struct _add_const<T&> : _type<const T&> {};
template<typename T> struct _add_const<T&&> : _type<const T&&> {};

template<typename T> struct _add_volatile : _type<volatile T> {};
template<typename T> struct _add_volatile<T&> : _type<volatile T&> {};
template<typename T> struct _add_volatile<T&&> : _type<volatile T&&> {};

template<typename T> struct _add_pointer : _type<T> {};
template<typename T> requires valid<T*> struct _add_pointer<T> : _type<T*> {};
template<typename T> requires valid<T*> struct _add_pointer<T&> : _type<T*&> {};
template<typename T> requires valid<T*> struct _add_pointer<T&&> : _type<T*&&> {};

template<typename T, nat N> struct _add_extent : _type<T> {};
template<typename T, nat N> requires valid<T[N]> struct _add_extent<T, N> : _type<T[N]> {};
template<typename T, nat N> requires valid<T[N]> struct _add_extent<T&, N> : std::add_lvalue_reference<T[N]> {};
template<typename T, nat N> requires valid<T[N]> struct _add_extent<T&&, N> : std::add_rvalue_reference<T[N]> {};
template<typename T> requires valid<T[]> struct _add_extent<T, 0> : _type<T[]> {};
template<typename T> requires valid<T[]> struct _add_extent<T&, 0> : std::add_lvalue_reference<T[]> {};
template<typename T> requires valid<T[]> struct _add_extent<T&&, 0> : std::add_rvalue_reference<T[]> {};

template<typename Ct, typename Mt> struct _class_member_type {
  using class_type = Ct;
  using member_type = Mt;
};

template<typename T> struct _member_pointer : constant<false> {};
template<typename T, typename C> struct _member_pointer<T C::*> : constant<true>, _class_member_type<C, T> {};
template<typename T, typename C> struct _member_pointer<T C::* const> : constant<true>, _class_member_type<C, T> {};
template<typename T, typename C> struct _member_pointer<T C::* volatile> : constant<true>, _class_member_type<C, T> {};
template<typename T, typename C> struct _member_pointer<T C::* const volatile> : constant<true>, _class_member_type<C, T> {};

template<typename T, T I, typename... Ts> struct _select_type : _select_type<nat, I, Ts...> {};
template<bool B, typename Tt, typename Tf> struct _select_type<bool, B, Tt, Tf> : std::conditional<B, Tt, Tf> {};
template<nat I, typename T, typename... Ts> struct _select_type<nat, I, T, Ts...> : _select_type<nat, I - 1, Ts...> {};
template<typename T, typename... Ts> struct _select_type<nat, 0, T, Ts...> : _type<T> {};

template<typename... Ts> struct _common_type : _type<none> {};
template<typename... Ts> requires valid<std::common_reference_t<Ts...>> struct _common_type<Ts...> : std::common_reference<Ts...> {};

template<typename T, template<typename...> typename Tm> struct _specialization_of : constant<false> {};
template<template<typename...> typename Tm, typename... Ts> struct _specialization_of<Tm<Ts...>, Tm> : constant<true> {};

template<typename T, typename U> struct _variation_of : constant<false> {};
template<template<typename...> typename Tm, typename... Ts, typename... Us> struct _variation_of<Tm<Ts...>, Tm<Us...>> : constant<true> {};
template<template<typename, auto...> typename Tm, typename T, auto... Vs, typename U, auto... Ws> struct _variation_of<Tm<T, Vs...>, Tm<U, Ws...>> : constant<true> {};
template<template<auto, typename...> typename Tm, auto V, typename... Ts, auto W, typename... Us> struct _variation_of<Tm<V, Ts...>, Tm<W, Us...>> : constant<true> {};

}

export namespace yw {

template<bool... Bs> inline constexpr nat counts = (Bs + ...);
template<bool... Bs> inline constexpr nat inspects = 0;
template<bool B, bool... Bs> inline constexpr nat inspects<B, Bs...> = B ? 0 : 1 + inspects<Bs...>;

template<typename T> using add_const = typename _::_add_const<T>::type;
template<typename T> using add_volatile = typename _::_add_volatile<T>::type;
template<typename T> using add_cv = add_const<add_volatile<T>>;
template<typename T> using add_lvref = std::add_lvalue_reference_t<T>;
template<typename T> using add_rvref = std::add_rvalue_reference_t<remove_ref<T>>;
template<typename T> using add_fwref = std::add_rvalue_reference_t<T>;
template<typename T> using add_pointer = typename _::_add_pointer<T>::type;
template<typename T, nat N> using add_extent = typename _::_add_extent<T, N>::type;

template<typename T> concept is_member_pointer = _::_member_pointer<T>::value && is_pointer<T>;
template<is_member_pointer T> using class_type = typename _::_member_pointer<T>::class_type;
template<is_member_pointer T> using member_type = typename _::_member_pointer<T>::member_type;
template<typename T> concept is_member_function_pointer = is_member_pointer<T> && is_function<member_type<T>>;
template<typename T> concept is_member_object_pointer = is_member_pointer<T> && !is_member_function_pointer<T>;

inline consteval auto cev(auto a) noexcept { return a; }
inline consteval auto arraysize(const is_bounded_array auto& a) noexcept { return std::extent_v<std::remove_reference_t<decltype(a)>>; }

inline constexpr auto addressof = []<is_lvref T>(T&& Ref) noexcept { return __builtin_addressof(Ref); };
inline constexpr auto asconst = []<typename T>(T&& Ref) noexcept -> add_const<T&&> { return static_cast<add_const<T&&>>(Ref); };
inline constexpr auto mv = []<typename T>(T&& Ref) noexcept -> decltype(auto) { return static_cast<add_rvref<T>>(Ref); };
template<typename T> inline constexpr auto fwd = []<typename U>(U&& Ref) noexcept -> decltype(auto) { return static_cast<add_fwref<T>>(Ref); };
template<typename T> inline constexpr auto declval = []() noexcept -> add_fwref<T> {};

template<convertible_to<nat> auto I, nat N> inline constexpr bool selectable = //
  (is_bool<decltype(I)> && N == 2) || (!is_bool<decltype(I)> && static_cast<nat>(I) < N);

template<convertible_to<nat> auto I, typename... Ts> requires selectable<I, sizeof...(Ts)> //
using select_type = typename _::_select_type<decltype(I), I, Ts...>::type;

template<convertible_to<nat> auto I, auto... Vs> requires selectable<I, sizeof...(Vs)> //
inline constexpr auto select_value = select_type<I, constant<Vs>...>::value;

template<convertible_to<nat> auto I> inline constexpr auto select = //
  []<typename A, typename... As>(A&& Arg, As&&... Args) noexcept    //
  -> decltype(auto) requires selectable<I, 1 + sizeof...(As)> {     //
  if constexpr (is_bool<decltype(I)>) return select<nat(!I)>(fwd<A>(Arg), fwd<As>(Args)...);
  else if constexpr (same_as<decltype(I), nat>) {
    if constexpr (I == 0) return fwd<A>(Arg);
    else return select<I - 1>(fwd<As>(Args)...);
  } else return select<nat(I)>(fwd<A>(Arg), fwd<As>(Args)...);
};

template<typename T, typename To> using copy_const = select_type<is_const<remove_ref<T>>, add_const<To>, To>;
template<typename T, typename To> using copy_volatile = select_type<is_volatile<remove_ref<T>>, add_volatile<To>, To>;
template<typename T, typename To> using copy_cv = copy_const<T, copy_volatile<T, To>>;
template<typename T, typename To> using copy_ref = select_type<inspects<is_lvref<T>, is_rvref<T>>, add_lvref<To>, add_rvref<To>, remove_ref<To>>;
template<typename T, typename To> using copy_cvref = copy_cv<T, copy_ref<T, To>>;

template<typename T> concept is_enum = std::is_enum_v<T>;
template<typename T> concept is_scoped_enum = is_enum<T> && !convertible_to<T, bool>;
template<typename T> using underlying_type = select_type<is_enum<T>, std::underlying_type_t<T>, T>;

template<typename T> concept destructible = requires { requires std::is_destructible_v<T>; };
template<typename T> concept nt_destructible = destructible<T> && std::destructible<T>;

template<typename T, typename A> concept assignable = requires { requires std::is_assignable_v<T, A>; };
template<typename T, typename A> concept nt_assignable = assignable<T, A> && requires { requires std::is_nothrow_assignable_v<T, A>; };

template<typename T, typename... As> concept constructible = requires { T{declval<As>()...}; };
template<typename T, typename... As> concept nt_constructible = constructible<T, As...> && requires { requires noexcept(T{declval<As>()...}); };

template<typename T, typename U = T> concept exchangeable = constructible<T, T> && assignable<T&, U>;
template<typename T, typename U = T> concept nt_exchangeable = nt_constructible<T, T> && nt_assignable<T&, U>;

template<typename T> inline constexpr auto construct = []<typename... As>(As&&... Args) noexcept(nt_constructible<T, As...>) -> T requires constructible<T, As...> { return T{static_cast<As&&>(Args)...}; };
inline constexpr auto assign = []<typename T, typename A>(T&& Ref, A&& Arg) noexcept(nt_assignable<T, A>) requires assignable<T, A> { static_cast<T&&>(Ref) = static_cast<A&&>(Arg); };

#pragma warning(push)
#pragma warning(disable : 4244)

inline constexpr auto exchange = []<typename Tr, typename Tv = Tr>(Tr& Ref, Tv&& Value) //
  noexcept(nt_exchangeable<Tr, Tv>) -> Tr requires exchangeable<Tr&, Tv> {
  auto a = mv(Ref);
  Ref = fwd<Tv>(Value);
  return a;
};

#pragma warning(pop)

template<typename F, typename... As> concept invocable = std::invocable<F, As...>;
template<typename F, typename... As> concept nt_invocable = invocable<F, As...> && requires { requires std::is_nothrow_invocable_v<F, As...>; };
template<typename F, typename... As> requires invocable<F, As...> using invoke_result = select_type<is_void<std::invoke_result_t<F, As...>>, none, std::invoke_result_t<F, As...>>;
template<typename F, typename R, typename... As> concept invocable_r = invocable<F, As...> && convertible_to<invoke_result<F, As...>, R>;
template<typename F, typename R, typename... As> concept nt_invocable_r = nt_invocable<F, As...> && nt_convertible_to<invoke_result<F, As...>, R>;

inline constexpr auto invoke = []<typename F, typename... As>(F&& Func, As&&... Args) //
  noexcept(nt_invocable<F, As...>) -> invoke_result<F, As...> {
  if constexpr (is_void<std::invoke_result_t<F, As...>>) //
    return std::invoke(fwd<F>(Func), fwd<As>(Args)...), none{};
  else return std::invoke(fwd<F>(Func), fwd<As>(Args)...);
};

template<typename R> inline constexpr auto invoke_r =                                          //
  []<typename F, typename... As>(F&& f, As&&... as) noexcept(nt_invocable_r<F, R, As...>) -> R //
  requires invocable_r<F, R, As...> { return static_cast<R>(invoke(fwd<F>(f), fwd<As>(as)...)); };

template<typename... Ts> using common_type = typename _::_common_type<Ts...>::type;
template<typename... Ts> concept common_with = !is_none<common_type<Ts...>> && (std::common_reference_with<common_type<Ts...>, Ts> && ...);

template<typename T, template<typename...> typename Tm> concept specialization_of = _::_specialization_of<T, Tm>::value;
template<typename T, typename U> concept variation_of = _::_variation_of<T, U>::value;

/// \brief bitwise casting
template<trivially_copyable T, trivially_copyable U> requires (sizeof(T) == sizeof(U)) //
constexpr T bitcast(const U& Val) noexcept {
  return __builtin_bit_cast(T, Val);
};

inline constexpr auto natcast = []<trivially_copyable T>(const T& Val) ywlib_wrap_auto(bitcast<nat_type<sizeof(T)>>(Val));

inline constexpr auto popcount = [](const integral auto& Value) ywlib_wrap_auto(std::popcount(natcast(Value)));
inline constexpr auto countlz = [](const integral auto& Value) ywlib_wrap_auto(std::countl_zero(natcast(Value)));
inline constexpr auto countrz = [](const integral auto& Value) ywlib_wrap_auto(std::countr_zero(natcast(Value)));
inline constexpr auto bitfloor = [](const integral auto& Value) ywlib_wrap_auto(std::bit_floor(natcast(Value)));
inline constexpr auto bitceil = [](const integral auto& Value) ywlib_wrap_auto(std::bit_ceil(natcast(Value)));

template<typename... Fs> struct caster : public Fs... {
private:
  template<typename T> static constexpr nat i = inspects<same_as<T, invoke_result<Fs>>...>;
  template<typename T> static constexpr nat j = i<T> < sizeof...(Fs) ? i<T> : inspects<convertible_to<T, invoke_result<Fs>>...>;
  template<typename... As> static constexpr nat k = inspects<invocable<Fs, As...>...>;

public:
  using Fs::operator()...;
  // template<typename T> requires (j<T> < sizeof...(Fs)) constexpr operator T() const ywlib_wrap_auto(select_type<j<T>, Fs...>::operator());

  template<typename T> requires (j<T> < sizeof...(Fs)) //
  constexpr operator T() const                         //
    noexcept(noexcept(T{select_type<j<T>, Fs...>::operator()()})) {
    return select_type<j<T>, Fs...>::operator()();
  }

  template<typename... As> requires (k<As...> < sizeof...(Fs)) //
  constexpr decltype(auto) operator()(As&&... Args) const      //
    noexcept(noexcept(select_type<k<As...>, Fs...>::operator()(fwd<As>(Args)...))) {
    return select_type<k<As...>, Fs...>::operator()(fwd<As>(Args)...);
  }
};

inline constexpr caster is_cev{[]() noexcept { return std::is_constant_evaluated(); }};

constexpr void cannot_be_constant_evaluated(auto&&...) {
  if (is_cev) throw "This code cannot be constant evaluated.";
}

template<auto... Vs> struct sequence;
template<typename... Ts> struct typepack;
template<typename... Ts> struct list;

}

namespace yw::_ {

template<typename S, typename T> struct _to_sequence;
template<template<auto...> typename Tm, typename T, auto... Vs> struct _to_sequence<Tm<Vs...>, T> : _type<sequence<T(Vs)...>> {};
template<template<auto...> typename Tm, auto... Vs> struct _to_sequence<Tm<Vs...>, none> : _type<sequence<Vs...>> {};
template<template<typename, auto...> typename Tm, typename T, typename U, auto... Vs> struct _to_sequence<Tm<U, Vs...>, T> : _type<sequence<T(Vs)...>> {};
template<template<typename, auto...> typename Tm, typename U, auto... Vs> struct _to_sequence<Tm<U, Vs...>, none> : _type<sequence<Vs...>> {};

template<typename S, nat N> struct _indices_for : constant<false> {};
template<nat... Vs, nat N> struct _indices_for<sequence<Vs...>, N> : constant<(lt(Vs, N) && ...)> {};

template<nat I, nat N, auto F, nat... Vs> struct _make_sequence : _make_sequence<I + 1, N, F, Vs..., F(I)> {};
template<nat N, auto F, nat... Vs> struct _make_sequence<N, N, F, Vs...> : _type<sequence<Vs...>> {};

template<nat I, nat N, typename S, nat... Is> struct _extracting_indices;
template<nat I, nat N, bool... Bs, nat... Is> struct _extracting_indices<I, N, sequence<Bs...>, Is...> //
  : select_type<select_value<I, Bs...>, _extracting_indices<I + 1, N, sequence<Bs...>, Is..., I>, _extracting_indices<I + 1, N, sequence<Bs...>, Is...>> {};
template<nat N, bool... B, nat... Is> struct _extracting_indices<N, N, sequence<B...>, Is...> : _type<sequence<Is...>> {};

export namespace _get {

template<nat I> void get() = delete;

template<nat I, typename T> inline constexpr auto pattern = []() -> int {
  using t = remove_ref<T>;
  if constexpr (is_class<t> || is_union<t>) {
    if constexpr (requires { get<I>(declval<T>()); }) return 1 | noexcept(get<I>(declval<T>())) * 16;
    else if constexpr (requires { declval<T>().template get<I>(); }) return 2 | noexcept(declval<T>().template get<I>()) * 16;
    else return I == 0 ? 20 : 8;
  } else if constexpr (is_bounded_array<t>) return I < std::extent_v<t> ? 16 : 8;
  else return I == 0 ? 20 : 8;
}();

template<nat I, typename T, int P = pattern<I, T>> requires (P != 8) //
constexpr decltype(auto) call(T&& t) noexcept(bool(P & 16)) {
  if constexpr ((P & 15) == 0) return fwd<T>(t)[I];
  else if constexpr ((P & 15) == 1) return get<I>(fwd<T>(t));
  else if constexpr ((P & 15) == 2) return fwd<T>(t).template get<I>();
  else return fwd<T>(t);
}

template<typename T, nat I> struct extent : std::tuple_size<T> {};
template<typename T> struct extent<T, 0> : std::extent<T> {};
template<typename T> struct extent<T, 4> : constant<1> {};
}

template<typename S, auto... Vs> struct _sequence_append : _sequence_append<typename _to_sequence<S, none>::type, Vs...> {};
template<auto... Vs, auto... Ws> struct _sequence_append<sequence<Ws...>, Vs...> : _type<sequence<Vs..., Ws...>> {};

template<typename S, auto... Vs> struct _sequence_extract : _sequence_extract<typename _to_sequence<S, nat>::type, Vs...> {};
template<auto... Vs, nat... Is> struct _sequence_extract<sequence<Is...>, Vs...> : _type<sequence<select_value<Is, Vs...>...>> {};

template<typename T, typename S> struct _to_typepack;
template<typename T, nat... Is> struct _to_typepack<T, sequence<Is...>> : _type<typepack<decltype(_get::call<Is>(declval<T>()))...>> {};

template<typename T, typename... Ts> struct _tuple_for : constant<(convertible_to<Ts, T> && ...)> {};
template<typename T, typename... Ts> struct _nt_tuple_for : constant<(nt_convertible_to<Ts, T> && ...)> {};

template<typename T, typename... Ts> struct _typepack_append;
template<typename... Us, typename... Ts> struct _typepack_append<typepack<Us...>, Ts...> : _type<typepack<Ts..., Us...>> {};

template<typename S, typename... Ts> struct _typepack_extract;
template<nat... Is, typename... Ts> struct _typepack_extract<sequence<Is...>, Ts...> : _type<typepack<select_type<Is, Ts...>...>> {};

}

export namespace yw {

template<typename T> inline constexpr nat extent = _::_get::extent<remove_ref<T>, _::_get::pattern<0, T> & 15>::value;
template<typename T, typename... Ts> concept same_extent = ((extent<T> == extent<Ts>) && ...);
template<typename T> concept tuple = (_::_get::pattern<0, T> & 15) < 3;
template<typename T, nat I> concept gettable = _::_get::pattern<I, T> != 8;
template<typename T, nat I> concept nt_gettable = gettable<T, I> && bool(_::_get::pattern<I, T> & 16);
template<typename T, nat I> requires gettable<T, I> using element_t = decltype(_::_get::call<I>(declval<T>()));

template<nat I> inline constexpr auto get = []<typename T>(T&& Obj) //
  noexcept(nt_gettable<T, I>) -> element_t<T, I> { return _::_get::call<I>(fwd<T>(Obj)); };

template<typename S, typename T = none> using to_sequence = typename _::_to_sequence<S, T>::type;
template<typename S, typename T = none> concept sequence_of = variation_of<to_sequence<S, T>, sequence<>>;
template<typename S, typename T> concept indices_for = _::_indices_for<to_sequence<S>, extent<T>>::value;

template<nat Begin, nat End, invocable<nat> auto Proj = pass{}> requires (Begin <= End) //
using make_sequence = typename _::_make_sequence<Begin, End, Proj>::type;
template<typename Tp> using make_indices_for = make_sequence<0, extent<Tp>>;
template<sequence_of<bool> Sq> using extracting_indices = typename _::_extracting_indices<0, extent<Sq>, Sq>::type;

template<auto... Vs> struct sequence {
  static constexpr nat count = sizeof...(Vs);
  template<nat I> requires (I < sizeof...(Vs)) static constexpr auto at = select_value<I, Vs...>;
  template<nat I> requires (I < sizeof...(Vs)) using type_at = select_type<I, decltype(Vs)...>;
  template<sequence_of Sq> using append = typename _::_sequence_append<Sq, Vs...>::type;
  template<indices_for<sequence> Ix> using extract = typename _::_sequence_extract<Ix, Vs...>::type;
  template<nat N> requires (N <= sizeof...(Vs)) using fore = extract<make_sequence<0, N>>;
  template<nat N> requires (N <= sizeof...(Vs)) using back = extract<make_sequence<sizeof...(Vs) - N, sizeof...(Vs)>>;
  template<nat I> requires (I < sizeof...(Vs)) using remove = typename fore<I>::template append<back<sizeof...(Vs) - I - 1>>;
  template<nat I, sequence_of Sq> requires (I <= sizeof...(Vs)) using insert = typename fore<I>::template append<Sq>::template append<back<sizeof...(Vs) - I>>;
  template<template<auto...> typename Tm> using expand = Tm<Vs...>;
  template<nat I> requires (I < sizeof...(Vs)) constexpr const auto&& get() const noexcept { return mv(at<I>); }
};

template<typename T> using to_typepack = typename _::_to_typepack<T, make_indices_for<T>>::type;
template<typename T> using common_element = to_typepack<T>::common;
template<typename T, typename U> concept tuple_for = to_typepack<T>::template expand<_::_tuple_for>::value;
template<typename T, typename U> concept nt_tuple_for = tuple_for<T, U> && to_typepack<T>::template expand<_::_nt_tuple_for>::value;

template<typename... Ts> struct typepack {
  static constexpr nat count = sizeof...(Ts);
  using common = common_type<Ts...>;
  template<nat I> requires (I < sizeof...(Ts)) using at = select_type<I, Ts...>;
  template<tuple Tp> using append = typename _::_typepack_append<to_typepack<Tp>, Ts...>::type;
  template<indices_for<typepack> Ix> using extract = typename _::_typepack_extract<to_sequence<Ix, nat>, Ts...>::type;
  template<nat N> requires (N <= sizeof...(Ts)) using fore = extract<make_sequence<0, N>>;
  template<nat N> requires (N <= sizeof...(Ts)) using back = extract<make_sequence<sizeof...(Ts) - N, sizeof...(Ts)>>;
  template<nat I> requires (I < sizeof...(Ts)) using remove = typename fore<I>::template append<back<sizeof...(Ts) - I - 1>>;
  template<nat I, typename T> requires (I <= sizeof...(Ts)) using insert = typename fore<I>::template append<T>::template append<back<sizeof...(Ts) - I>>;
  template<template<typename...> typename Tm> using expand = Tm<Ts...>;
  template<nat I> requires (I < sizeof...(Ts)) constexpr const at<I> get() const noexcept;
};

template<typename T, typename Pj = pass, sequence_of<nat> Sq = make_indices_for<T>> struct projector {
  static_assert(to_sequence<Sq, nat>::count > 0);
  static_assert(!tuple<T> || indices_for<Sq, T>);
  using sequence = to_sequence<Sq, nat>;
  static constexpr nat count = sequence::count;
  add_fwref<T> ref;
  remove_ref<Pj> proj = {};
  template<typename U = T> constexpr projector(U&& Ref, Pj p, Sq) noexcept(nt_constructible<Pj, Pj>) : ref(fwd<U>(Ref)), proj(mv(p)) {}
  template<typename U = T> constexpr projector(U&& Ref, Pj p) noexcept(nt_constructible<Pj, Pj>) : ref(fwd<U>(Ref)), proj(mv(p)) {}
  template<typename U = T> constexpr projector(U&& Ref, Sq) noexcept : ref(fwd<U>(Ref)) {}
  template<typename U = T> constexpr projector(U&& Ref) noexcept : ref(fwd<U>(Ref)) {}
  template<nat I> requires (I < count && tuple<T>) constexpr decltype(auto) get() const                            //
    noexcept(nt_gettable<T, sequence::template at<I>> && nt_invocable<Pj, element_t<T, sequence::template at<I>>>) //
    requires gettable<T, sequence::template at<I>> && invocable<Pj, element_t<T, sequence::template at<I>>> {
    return invoke(proj, yw::get<sequence::template at<I>>(ref));
  }
  template<nat I> requires (I < count && !tuple<T>) constexpr decltype(auto) get() const //
    noexcept(nt_invocable<Pj, T>) requires invocable<Pj, T> {
    return invoke(proj, ref);
  }
};

template<typename T, typename Pj, sequence_of<nat> Sq> projector(T&&, Pj, Sq) -> projector<T, Pj, Sq>;
template<typename T, typename Pj> requires (!sequence_of<Pj, nat>) projector(T&&, Pj) -> projector<T, Pj>;
template<typename T, sequence_of<nat> Sq> projector(T&&, Sq) -> projector<T, pass, Sq>;
template<typename T> projector(T&&) -> projector<T>;

}

namespace std {

template<auto... Vs> struct tuple_size<yw::sequence<Vs...>> : integral_constant<nat, sizeof...(Vs)> {};
template<nat I, auto... Vs> struct tuple_element<I, yw::sequence<Vs...>> : type_identity<decltype(yw::sequence<Vs...>::template at<I>)> {};

template<typename... Ts> struct tuple_size<yw::typepack<Ts...>> : integral_constant<nat, sizeof...(Ts)> {};
template<nat I, typename... Ts> struct tuple_element<I, yw::typepack<Ts...>> : type_identity<typename yw::typepack<Ts...>::template at<I>> {};

template<typename T, typename Pj, yw::sequence_of<size_t> Sq> struct tuple_size<yw::projector<T, Pj, Sq>> //
  : integral_constant<size_t, yw::projector<T, Pj, Sq>::count> {};
template<size_t I, typename T, typename Pj, yw::sequence_of<size_t> Sq> struct tuple_element<I, yw::projector<T, Pj, Sq>> //
  : type_identity<decltype(declval<yw::projector<T, Pj, Sq>>().template get<I>())> {};

}

namespace yw::_ { // clang-format off

template<typename F, typename... Ts> requires (!(tuple<Ts> || ...)) constexpr decltype(auto) _apply(F&& f, Ts&&... ts)
  noexcept(nt_invocable<F, Ts...>) requires invocable<F, Ts...> { return invoke(fwd<F>(f), fwd<Ts>(ts)...); }

template<nat I, nat... Is, nat... Js, nat... Ks, typename F, typename... Ts>
constexpr decltype(auto) _apply_b(sequence<Is...>, sequence<Js...>, sequence<Ks...>, F&& f, Ts&&... ts)
  ywlib_wrap_auto(_apply(fwd<F>(f), select<Is>(fwd<Ts>(ts)...)..., get<Js>(fwd<select_type<I, Ts...>>(select<I>(fwd<Ts>(ts)...)))..., select<Ks>(fwd<Ts>(ts)...)...));

template<nat I, typename F, typename... Ts> constexpr decltype(auto) _apply_a(F&& f, Ts&&... ts)
  ywlib_wrap_auto(_apply_b<I>(make_sequence<0, I>{}, make_indices_for<select_type<I, Ts...>>{}, make_sequence<I + 1, sizeof...(Ts)>{}, fwd<F>(f), fwd<Ts>(ts)...));

template<typename F, typename... Ts> requires (tuple<Ts> || ...)
constexpr decltype(auto) _apply(F&& f, Ts&&... ts) ywlib_wrap_auto(_apply_a<inspects<tuple<Ts>...>>(fwd<F>(f), fwd<Ts>(ts)...));

template<nat I, typename F, typename... Ts> constexpr decltype(auto) _vapply_i(F&& f, Ts&&... ts) ywlib_wrap_auto(invoke(fwd<F>(f), get<I>(fwd<Ts>(ts))...));

template<nat... Is, typename F, typename... Ts>
constexpr decltype(auto) _vapply_is(sequence<Is...>, F&& f, Ts&&... ts) ywlib_wrap_auto(((_vapply_i<Is>(fwd<F>(f), fwd<Ts>(ts)...)), ...));

template<typename F, typename T, typename... Ts> requires same_extent<T, Ts...>
constexpr decltype(auto) _vapply(F&& f, T&& t, Ts&&... ts) ywlib_wrap_auto(_vapply_is(make_indices_for<T>(), fwd<F>(f), fwd<T>(t), fwd<Ts>(ts)...));

template<typename F, typename T, typename... Ts> requires (!same_extent<T, Ts...> && tuple<T> && (tuple<Ts> && ...)) void _vapply(F&& f, T&& t, Ts&&... ts) = delete;

template<typename F, typename... Ts> requires (!(tuple<Ts> || ...)) void _vapply(F&& f, Ts&&... ts) = delete;

template<nat I, nat N, nat... Is, nat... Js, typename F, typename... Ts>
constexpr decltype(auto) _vapply_b(sequence<Is...>, sequence<Js...>, F&& f, Ts&&... ts)
  ywlib_wrap_auto(_vapply(fwd<F>(f), select<Is>(fwd<Ts>(ts)...)..., projector(select<I>(fwd<Ts>(ts)...), make_sequence<0, N>{}), select<Js>(fwd<Ts>(ts)...)...));

template<nat I, nat J, typename F, typename... Ts> constexpr decltype(auto) _vapply_a(F&& f, Ts&&... ts)
  ywlib_wrap_auto(_vapply_b<I, extent<select_type<J, Ts...>>>(make_sequence<0, I>{}, make_sequence<I + 1, sizeof...(Ts)>{}, fwd<F>(f), fwd<Ts>(ts)...));

template<typename F, typename... Ts> requires (!(tuple<Ts> && ...) && (tuple<Ts> || ...))
constexpr decltype(auto) _vapply(F&& f, Ts&&... ts) ywlib_wrap_auto(_vapply_a<inspects<!tuple<Ts>...>, inspects<tuple<Ts>...>>(fwd<F>(f), fwd<Ts>(ts)...));

template<typename... Ts> using list_base = typepack<Ts...>::template fore<sizeof...(Ts) - 1>::template expand<list>;

template<typename T, typename U, typename V> struct list_from_typepack;
template<typename... Ts, typename U, template<typename...> typename Tm, typename... Vs> struct list_from_typepack<typepack<Ts...>, U, Tm<Vs...>> : _type<list<copy_cvref<U, Tm<Ts>>...>> {};
template<typename... Ts, typename U, template<typename, auto...> typename Tm, typename V, auto... Vs> struct list_from_typepack<typepack<Ts...>, U, Tm<V, Vs...>> : _type<list<copy_cvref<U, Tm<Ts, Vs...>>...>> {};
template<typename... Ts, typename U, typename V> struct list_from_typepack<typepack<Ts...>, U, V> : _type<list<copy_cvref<U, Ts>...>> {};

} // clang-format on

export namespace yw {

template<typename F, typename... Ts> concept applyable = requires(F&& f, Ts&&... ts) { _::_apply(fwd<F>(f), fwd<Ts>(ts)...); };
template<typename F, typename... Ts> concept nt_applyable = applyable<F, Ts...> && requires(F&& f, Ts&&... ts) { requires noexcept(_::_apply(fwd<F>(f), fwd<Ts>(ts)...)); };
template<typename F, typename... Ts> using apply_result = decltype(_::_apply(declval<F>(), declval<Ts>()...));
inline constexpr auto apply = []<typename F, typename... Ts>(F&& Func, Ts&&... Args) noexcept(nt_applyable<F, Ts...>) -> decltype(auto) requires applyable<F, Ts...> { return _::_apply(fwd<F>(Func), fwd<Ts>(Args)...); };

template<typename T> inline constexpr auto build = []<typename Tp>(Tp&& Tuple) ywlib_wrap_auto(construct<T>(fwd<Tp>(Tuple)));
template<typename T, typename Tp> concept buildable = applyable<decltype(construct<T>), Tp>;
template<typename T, typename Tp> concept nt_buildable = nt_applyable<decltype(construct<T>), Tp>;

template<typename T, typename... Ts> concept vapplyable = requires(T&& t, Ts&&... ts) { _::_vapply(fwd<T>(t), fwd<Ts>(ts)...); };
template<typename T, typename... Ts> concept nt_vapplyable = vapplyable<T, Ts...> && requires(T&& t, Ts&&... ts) { requires noexcept(_::_vapply(fwd<T>(t), fwd<Ts>(ts)...)); };
inline constexpr auto vapply = []<typename F, typename... Ts>(F&& f, Ts&&... ts) noexcept(nt_vapplyable<F, Ts...>) requires vapplyable<F, Ts...> { return _::_vapply(fwd<F>(f), fwd<Ts>(ts)...); };

inline constexpr auto vassign = []<typename Lt, typename Rt>(Lt&& Lhs, Rt&& Rhs) ywlib_wrap_void(vapply(assign, fwd<Lt>(Lhs), fwd<Rt>(Rhs)));
template<typename Lt, typename Rt> concept vassignable = vapplyable<decltype(vassign), Lt, Rt>;
template<typename Lt, typename Rt> concept nt_vassignable = nt_vapplyable<decltype(vassign), Lt, Rt>;

// clang-format off

template<typename... Ts> struct list : _::list_base<Ts...> {
  static constexpr nat count = sizeof...(Ts);
  using last_type = select_type<sizeof...(Ts) - 1, Ts...>;
  select_type<sizeof...(Ts) - 1, Ts...> last;
  template<nat I> requires (I < sizeof...(Ts)) constexpr auto get() & noexcept -> select_type<I, Ts...>& { if constexpr (I == sizeof...(Ts) - 1) return last; else return _::list_base<Ts...>::template get<I>(); }
  template<nat I> requires (I < sizeof...(Ts)) constexpr auto get() const & noexcept -> add_const<select_type<I, Ts...>&> { if constexpr (I == sizeof...(Ts) - 1) return last; else return _::list_base<Ts...>::template get<I>(); }
  template<nat I> requires (I < sizeof...(Ts)) constexpr auto get() && noexcept -> select_type<I, Ts...>&& { if constexpr (I == sizeof...(Ts) - 1) return mv(last); else return static_cast<_::list_base<Ts...>&&>(*this).template get<I>(); }
  template<nat I> requires (I < sizeof...(Ts)) constexpr auto get() const && noexcept -> add_const<select_type<I, Ts...>&&> { if constexpr (I == sizeof...(Ts) - 1) return mv(last); else return static_cast<_::list_base<Ts...>&&>(*this).template get<I>(); }
  template<typename A> constexpr list& operator=(A&& Arg) noexcept(nt_vassignable<list&, A>) requires vassignable<list&, A> { return vassign(*this, fwd<A>(Arg)), *this; }
  template<typename A> constexpr const list& operator=(A&& Arg) const noexcept(nt_vassignable<const list&, A>) requires vassignable<const list&, A> { return vassign(*this, fwd<A>(Arg)), *this; }
};

template<typename T1, typename T2, typename T3> struct list<T1, T2, T3> : list<T1, T2> {
  static constexpr nat count = 3;
  using third_type = T3;
  third_type third;
  template<nat I> requires (I < 3) constexpr auto get() & noexcept -> select_type<I, T1, T2, T3>& { if constexpr (I == 3) return third; else return list<T1, T2>::template get<I>(); }
  template<nat I> requires (I < 3) constexpr auto get() const & noexcept -> add_const<select_type<I, T1, T2, T3>&> { if constexpr (I == 3) return third; else return list<T1, T2>::template get<I>(); }
  template<nat I> requires (I < 3) constexpr auto get() && noexcept -> select_type<I, T1, T2, T3>&& { if constexpr (I == 3) return mv(third); else return static_cast<list<T1, T2>&&>(*this).template get<I>(); }
  template<nat I> requires (I < 3) constexpr auto get() const && noexcept -> add_const<select_type<I, T1, T2, T3>&&> { if constexpr (I == 3) return mv(third); else return static_cast<list<T1, T2>&&>(*this).template get<I>(); }
  template<typename A> constexpr list& operator=(A&& Arg) noexcept(nt_vassignable<list&, A>) requires vassignable<list&, A> { return vassign(*this, fwd<A>(Arg)), *this; }
  template<typename A> constexpr const list& operator=(A&& Arg) const noexcept(nt_vassignable<const list&, A>) requires vassignable<const list&, A> { return vassign(*this, fwd<A>(Arg)), *this; }
};

template<typename T1, typename T2> struct list<T1, T2> : list<T1> {
  static constexpr nat count = 2;
  using second_type = T2;
  second_type second;
  template<nat I> requires (I < 2) constexpr auto get() & noexcept -> select_type<I, T1, T2>& { if constexpr (I == 2) return second; else return list<T1>::template get<I>(); }
  template<nat I> requires (I < 2) constexpr auto get() const & noexcept -> add_const<select_type<I, T1, T2>&> { if constexpr (I == 2) return second; else return list<T1>::template get<I>(); }
  template<nat I> requires (I < 2) constexpr auto get() && noexcept -> select_type<I, T1, T2>&& { if constexpr (I == 2) return mv(second); else return static_cast<list<T1>&&>(*this).template get<I>(); }
  template<nat I> requires (I < 2) constexpr auto get() const && noexcept -> add_const<select_type<I, T1, T2>&&> { if constexpr (I == 2) return mv(second); else return static_cast<list<T1>&&>(*this).template get<I>(); }
  template<typename A> constexpr list& operator=(A&& Arg) noexcept(nt_vassignable<list&, A>) requires vassignable<list&, A> { return vassign(*this, fwd<A>(Arg)), *this; }
  template<typename A> constexpr const list& operator=(A&& Arg) const noexcept(nt_vassignable<const list&, A>) requires vassignable<const list&, A> { return vassign(*this, fwd<A>(Arg)), *this; }
};

template<typename T> struct list<T> {
  static constexpr nat count = 1;
  T first;
  template<nat I> requires (I < 1) constexpr auto get() & noexcept -> select_type<I, T>& { if constexpr (I == 0) return first; else return first; }
  template<nat I> requires (I < 1) constexpr auto get() const & noexcept -> add_const<select_type<I, T>&> { if constexpr (I == 0) return first; else return first; }
  template<nat I> requires (I < 1) constexpr auto get() && noexcept -> select_type<I, T>&& { if constexpr (I == 0) return mv(first); else return mv(first); }
  template<nat I> requires (I < 1) constexpr auto get() const && noexcept -> add_const<select_type<I, T>&&> { if constexpr (I == 0) return mv(first); else return mv(first); }
  template<typename A> constexpr list& operator=(A&& Arg) noexcept(nt_vassignable<list&, A>) requires vassignable<list&, A> { return vassign(*this, fwd<A>(Arg)), *this; }
  template<typename A> constexpr const list& operator=(A&& Arg) const noexcept(nt_vassignable<const list&, A>) requires vassignable<const list&, A> { return vassign(*this, fwd<A>(Arg)), *this; }
};

template<> struct list<> {
  static constexpr nat count = 0;
  template<typename... Ts> static constexpr auto asref(Ts&&... Args) noexcept { return list<Ts&&...>{fwd<Ts>(Args)...}; }
  template<specialization_of<typepack> Tp, typename Qualifier = none> using from_typepack = _::list_from_typepack<Tp, Qualifier, remove_cvref<Qualifier>>::type;
};

template<typename... Ts> list(Ts...) -> list<Ts...>;

}

namespace std {

template<typename... Ts> struct tuple_size<yw::list<Ts...>> : integral_constant<size_t, yw::list<Ts...>::count> {};
template<size_t I, typename... Ts> struct tuple_element<I, yw::list<Ts...>> : type_identity<yw::select_type<I, Ts...>> {};

}

namespace yw::_ { // clang-format off

template<typename T> struct _iter_t {
  using v = std::iter_value_t<T>; using d = std::iter_difference_t<T>;
  using r = std::iter_reference_t<T>; using rr = std::iter_rvalue_reference_t<T>;
};
template<std::ranges::range Rg> struct _iter_t<Rg> : _iter_t<std::ranges::iterator_t<Rg>> {};

} // clang-format on

export namespace yw {

template<typename It> concept iterator = std::input_or_output_iterator<remove_ref<It>>;
template<typename Se, typename It> concept sentinel_for = std::sentinel_for<remove_ref<Se>, remove_ref<It>>;
template<typename Se, typename It> concept sized_sentinel_for = std::sized_sentinel_for<remove_ref<Se>, remove_ref<It>>;
template<typename Rg> concept range = std::ranges::range<Rg>;
template<typename Rg> concept borrowed_range = std::ranges::borrowed_range<Rg>;
template<typename Rg> concept sized_range = std::ranges::sized_range<Rg>;
template<range Rg> using iterator_t = std::ranges::iterator_t<Rg>;
template<range Rg> using borrowed_iterator_t = std::ranges::borrowed_iterator_t<Rg>;
template<range Rg> using sentinel_t = std::ranges::sentinel_t<Rg>;

template<typename T> requires iterator<T> || range<T> using iter_value = typename _::_iter_t<T>::v;
template<typename T> requires iterator<T> || range<T> using iter_difference = typename _::_iter_t<T>::d;
template<typename T> requires iterator<T> || range<T> using iter_reference = typename _::_iter_t<T>::r;
template<typename T> requires iterator<T> || range<T> using iter_rvref = typename _::_iter_t<T>::rr;
template<typename T> requires iterator<T> || range<T> using iter_common = common_type<iter_reference<T>, iter_value<T>>;

template<typename It, typename T> concept output_iterator = std::output_iterator<remove_ref<It>, T>;
template<typename It> concept input_iterator = std::input_iterator<remove_ref<It>>;
template<typename It> concept forward_iterator = std::forward_iterator<remove_ref<It>>;
template<typename It> concept bidirectional_iterator = std::bidirectional_iterator<remove_ref<It>>;
template<typename It> concept random_access_iterator = std::random_access_iterator<remove_ref<It>>;
template<typename It> concept contiguous_iterator = std::contiguous_iterator<remove_ref<It>>;
template<typename It, typename U> concept iterator_for = iterator<remove_ref<It>> && convertible_to<iter_reference<It>, U>;
template<typename Rg, typename T> concept output_range = std::ranges::output_range<Rg, T>;
template<typename Rg> concept input_range = std::ranges::input_range<Rg>;
template<typename Rg> concept forward_range = std::ranges::forward_range<Rg>;
template<typename Rg> concept bidirectional_range = std::ranges::bidirectional_range<Rg>;
template<typename Rg> concept random_access_range = std::ranges::random_access_range<Rg>;
template<typename Rg> concept contiguous_range = std::ranges::contiguous_range<Rg>;
template<typename Rg, typename U> concept range_for = iterator_for<iterator_t<Rg>, U>;
template<typename It, typename In> concept iter_copyable = iterator<It> && iterator<In> && std::indirectly_copyable<In, It>;
template<typename It, typename In> concept iter_movable = iterator<It> && iterator<In> && std::indirectly_movable<In, It>;
template<typename Fn, typename It> concept iter_unary_invocable = iterator<It> && std::indirectly_unary_invocable<Fn, It>;
template<typename Fn, typename It> concept iter_unary_predicate = iterator<It> && std::indirect_unary_predicate<Fn, It>;

inline constexpr auto begin = []<range Rg>(Rg&& r) ywlib_wrap_auto(std::ranges::begin(fwd<Rg>(r)));
inline constexpr auto end = []<range Rg>(Rg&& r) ywlib_wrap_auto(std::ranges::end(fwd<Rg>(r)));
inline constexpr auto rbegin = []<range Rg>(Rg&& r) ywlib_wrap_auto(std::ranges::rbegin(fwd<Rg>(r)));
inline constexpr auto rend = []<range Rg>(Rg&& r) ywlib_wrap_auto(std::ranges::rend(fwd<Rg>(r)));
inline constexpr auto size = []<range Rg>(Rg&& r) ywlib_wrap_auto(std::ranges::size(fwd<Rg>(r)));
inline constexpr auto empty = []<range Rg>(Rg&& r) ywlib_wrap_auto(std::ranges::empty(fwd<Rg>(r)));
inline constexpr auto data = []<range Rg>(Rg&& r) ywlib_wrap_auto(std::ranges::data(fwd<Rg>(r)));
inline constexpr auto iter_move = []<iterator It>(It&& i) ywlib_wrap_ref(std::ranges::iter_move(fwd<It>(i)));
inline constexpr auto iter_swap = []<iterator It, iterator Jt>(It&& i, Jt&& j) ywlib_wrap_void(std::ranges::iter_swap(fwd<It>(i), fwd<Jt>(j)));

template<typename T, nat N = npos> class array {
public:
  T _[N]{};
  static constexpr nat count = N;
  using value_type = T;
  template<typename U> requires assignable<T&, const U&> constexpr array& operator=(const U (&Other)[N]) { return std::ranges::copy(Other, _), *this; }
  constexpr operator add_lvref<T[N]>() noexcept { return _; }
  constexpr operator add_lvref<const T[N]>() const noexcept { return _; }
  constexpr T& operator[](nat I) { return _[I]; }
  constexpr const T& operator[](nat I) const { return _[I]; }
  constexpr nat size() const noexcept { return N; }
  constexpr bool empty() const noexcept { return false; }
  constexpr T* data() noexcept { return _; }
  constexpr const T* data() const noexcept { return _; }
  constexpr T* begin() noexcept { return _; }
  constexpr const T* begin() const noexcept { return _; }
  constexpr T* end() noexcept { return _ + N; }
  constexpr const T* end() const noexcept { return _ + N; }
  constexpr T& front() noexcept { return *_; }
  constexpr const T& front() const noexcept { return *_; }
  constexpr T& back() noexcept { return _[N - 1]; }
  constexpr const T& back() const noexcept { return _[N - 1]; }
  template<nat I> requires (I < N) constexpr T& get() & noexcept { return _[I]; }
  template<nat I> requires (I < N) constexpr T&& get() && noexcept { return mv(_[I]); }
  template<nat I> requires (I < N) constexpr const T& get() const& noexcept { return _[I]; }
  template<nat I> requires (I < N) constexpr const T&& get() const&& noexcept { return mv(_[I]); }
};

template<typename T> class array<T, npos> : public std::vector<T> {
public:
  constexpr array() noexcept = default;
  constexpr array(std::vector<T>&& v) : std::vector<T>(mv(v)) {}
  constexpr explicit array(nat n) : std::vector<T>(n) {}
  constexpr array(nat n, const T& v) : std::vector<T>(n, v) {}
  template<iterator_for<T> It> constexpr array(It i, It s) : std::vector<T>(i, s) {}
  template<iterator_for<T> It, sentinel_for<It> Se> requires (!same_as<It, Se>) //
  constexpr array(It i, Se s) : std::vector<T>(std::common_iterator<It, Se>(i), std::common_iterator<It, Se>(s)) {}
  template<range_for<T> Rg> constexpr array(Rg&& r) : std::vector<T>(yw::begin(r), yw::end(r)) {}
};

template<typename T> class array<T, 0> {
public:
  static constexpr nat count = 0;
  using value_type = T;
  constexpr nat size() const noexcept { return 0; }
  constexpr bool empty() const noexcept { return true; }
  constexpr T* data() noexcept { return nullptr; }
  constexpr const T* data() const noexcept { return nullptr; }
  constexpr T* begin() noexcept { return nullptr; }
  constexpr const T* begin() const noexcept { return nullptr; }
  constexpr T* end() noexcept { return nullptr; }
  constexpr const T* end() const noexcept { return nullptr; }
  template<nat I> constexpr void get() const = delete;
};

template<typename T, convertible_to<T>... Ts> array(T, Ts...) -> array<T, 1 + sizeof...(Ts)>;
template<typename T> array(nat, const T&) -> array<T, npos>;
template<iterator It, sentinel_for<It> Se> array(It, Se) -> array<iter_value<It>, npos>;
template<range Rg> array(Rg&&) -> array<iter_value<Rg>, npos>;

/// \brief standard string class
template<character Ct> using string = std::basic_string<Ct>;

/// \brief standard string view class
template<character Ct> using string_view = std::basic_string_view<Ct>;

/// \brief standard format string class
template<character Ct, typename... Args> using format_string = std::basic_format_string<Ct, Args...>;

/// \brief checks if `T` is stringable
template<typename T, typename Ct = iter_value<T>> concept stringable = nt_convertible_to<T&, string_view<Ct>>;

}

namespace std {

template<typename T, nat N> requires (N != yw::npos) struct tuple_size<yw::array<T, N>> : integral_constant<size_t, N> {};
template<size_t I, typename T, nat N> requires (N != yw::npos) struct tuple_element<I, yw::array<T, N>> : type_identity<T> {};

}

namespace yw::_ {

template<typename Ct> constexpr nat8 _ston(std::basic_string_view<Ct>& s) noexcept {
  nat8 v{};
  while (1) {
    if (s.empty()) return v;
    else if (is_digit(s.front())) {
      v = s.front() - '0';
      break;
    } else s.remove_prefix(1);
  }
  s.remove_prefix(1);
  while (1) {
    if (s.empty()) return v;
    else if (is_digit(s.front())) v = v * 10 + s.front() - '0', s.remove_prefix(1);
    else return v;
  }
}

template<typename Ct> constexpr int8 _stoi(std::basic_string_view<Ct>& s) noexcept {
  int8 v{};
  bool neg = false;
  while (1) {
    if (s.empty()) return {};
    else if (is_digit(s.front())) {
      neg = true, v = s.front() - '0';
      break;
    } else if (s.front() == Ct('-')) {
      neg = true;
      break;
    } else s.remove_prefix(1);
  }
  s.remove_prefix(1);
  while (1) {
    if (s.empty()) return neg ? -v : v;
    else if (is_digit(s.front())) v = v * 10 + s.front() - '0', s.remove_prefix(1);
    else return neg ? -v : v;
  }
}

template<typename Ct> constexpr fat8 _stof(std::basic_string_view<Ct>& s) noexcept {
  fat8 v{};
  bool neg = false, dot = false;
  while (1) {
    if (s.empty()) return {};
    else if (is_digit(s.front())) {
      v = s.front() - '0';
      break;
    } else if (s.front() == Ct('-')) {
      neg = true;
      break;
    } else if (s.front() == Ct('.')) {
      dot = true;
      break;
    } else s.remove_prefix(1);
  }
  s.remove_prefix(1);
  while (!dot) {
    if (s.empty()) return neg ? -v : v;
    else if (is_digit(s.front())) v = v * 10 + s.front() - '0', s.remove_prefix(1);
    else if (s.front() == Ct('.')) {
      dot = true;
      break;
    } else return neg ? -v : v;
  }
  s.remove_prefix(1);
  fat8 p = 0.1;
  while (1) {
    if (s.empty()) return neg ? -v : v;
    else if (is_digit(s.front())) {
      v += (s.front() - '0') * p, p *= 0.1, s.remove_prefix(1);
    } else if ((s.front() == Ct('e') || s.front() == Ct('E'))) break;
    else return neg ? -v : v;
  }
  s.remove_prefix(1);
  bool e_neg = false;
  int8 e = 0;
  if (s.empty()) return neg ? -v : v;
  else if (s.front() == Ct('-')) e_neg = true, s.remove_prefix(1);
  else if (s.front() == Ct('+')) s.remove_prefix(1);
  while (1) {
    if (s.empty()) break;
    else if (is_digit(s.front())) e = e * 10 + s.front() - '0', s.remove_prefix(1);
    else break;
  }
  for (int8 i = 0; i < e; ++i) v *= 10;
  for (int8 i = 0; i > e; --i) v *= 0.1;
  return neg ? -v : v;
}

inline constexpr cat1 _vtos_codes[36]{
  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', //
  'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', //
  'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', //
  'u', 'v', 'w', 'x', 'y', 'z',                     //
};

template<character Ct> constexpr std::basic_string<Ct> _ntos(nat8 v) noexcept {
  if (v == 0) return std::basic_string<Ct>(1, Ct('0'));
  Ct temp[20];
  nat it = 20;
  for (; v != 0; v /= 10) temp[--it] = Ct(v % 10 + '0');
  return std::basic_string<Ct>(temp + it, 20 - it);
}

template<character Ct> constexpr std::basic_string<Ct> _itos(int8 v) noexcept {
  if (v == 0) return std::basic_string<Ct>(1, Ct('0'));
  const bool neg = v < 0;
  Ct temp[20];
  nat it = 20;
  for (; v != 0; v /= 10) temp[--it] = Ct(v % 10 + '0');
  if (neg) temp[--it] = Ct('-');
  return std::basic_string<Ct>(temp + it, 20 - it);
}

constexpr fat8 _ftos_log10(fat8 v) noexcept {
  int8 t = bitcast<int8>(v);
  int8 exp = ((t & 0x7ff0000000000000) >> 52) - 1023;
  t = (t & 0x000fffffffffffff) | 0x3ff0000000000000;
  return exp * log10_2 - 0.652998017485824 + [&](const fat8 x) noexcept { //
    return x * (0.924793176223418 - 0.319987904734089 * x + 0.0480424904500012 * x * x);
  }(bitcast<fat8>(t));
}

template<character Ct> constexpr std::basic_string<Ct> _ftos(fat8 v) noexcept {
  if (v == 0) return std::basic_string<Ct>(array{Ct('0'), Ct('.'), Ct('0')});
  else if (v != v) return std::basic_string<Ct>(array{Ct('n'), Ct('a'), Ct('n')});
  else if (v == yw::inf) return std::basic_string<Ct>(array{Ct('i'), Ct('n'), Ct('f')});
  else if (v == -yw::inf) return std::basic_string<Ct>(array{Ct('-'), Ct('i'), Ct('n'), Ct('f')});
  Ct temp[32]{};
  const bool neg = v < 0;
  if (neg) v = -v;
  int8 exp = int8(_ftos_log10(v)), count{};
  exp += v < 1 ? -1 : 0;
  if (exp < -3) {
    nat mask = 1;
    for (int8 i = exp; i < 0; ++i) v *= 10;
    for (int8 i = 0; i < 15; ++i) v *= 10, mask *= 10;
    nat vv = static_cast<nat>(v), ii{};
    if (neg) temp[ii++] = Ct('-');
    temp[ii++] = Ct(vv / mask + '0');
    vv %= mask, mask /= 10;
    temp[ii++] = Ct('.');
    for (int8 i{}; i < 16 && vv != 0; ++i) {
      if (count == 6) ii -= 6;
      auto cc = vv / mask;
      count = cc == 0 ? count + 1 : 0;
      if (count == 3) return string<Ct>(temp, ii - 6);
      temp[ii++] = Ct(cc + '0'), vv %= mask, mask /= 10;
    }
    temp[ii++] = Ct('e');
    temp[ii++] = Ct('-');
    exp = -exp;
    temp[ii++] = Ct(exp / 100 + '0');
    exp %= 100;
    temp[ii++] = Ct(exp / 10 + '0');
    temp[ii++] = Ct(exp % 10 + '0');
    return string<Ct>(temp, ii);
  } else if (exp < 0) {
    nat mask = 1;
    for (int8 i{}; i < 15; ++i) mask *= 10;
    for (int8 i{15 - exp}; i > 0; --i) v *= 10;
    nat vv = static_cast<nat>(v), ii{};
    if (neg) temp[ii++] = Ct('-');
    temp[ii++] = Ct('0');
    temp[ii++] = Ct('.');
    for (int8 i{exp + 1}; i < 0; ++i) temp[ii++] = Ct('0');
    for (; vv != 0; vv %= mask, mask /= 10) {
      auto cc = vv / mask;
      count = cc == 0 ? count + 1 : 0;
      if (count == 3) return string<Ct>(temp, ii - 6);
      temp[ii++] = Ct(cc + '0');
    }
    return string<Ct>(temp, ii);
  } else if (exp < 16) {
    nat mask = 1;
    for (int8 i{}; i < 15; ++i) mask *= 10;
    for (int8 i{}, end = 15 - exp; i < end; ++i) v *= 10;
    nat vv = static_cast<nat>(v), ii{};
    if (neg) temp[ii++] = Ct('-');
    bool dot = false;
    for (int8 i{}; i < 16 && vv != 0; ++i) {
      if (!dot && i > exp) temp[ii++] = Ct('.'), dot = true;
      auto cc = vv / mask;
      if (dot) {
        count = cc == 0 ? count + 1 : 0;
        if (count == 3) return string<Ct>(temp, ii - 6);
      }
      temp[ii++] = Ct(cc + '0');
      vv %= mask;
      mask /= 10;
    }
    return string<Ct>(temp, ii);
  } else {
    nat mask = 1;
    for (int8 i{}; i < 15; ++i) mask *= 10;
    for (int8 i{}, end = exp - 15; i < end; ++i) v /= 10;
    nat vv = static_cast<nat>(v), ii{};
    if (neg) temp[ii++] = Ct('-');
    temp[ii++] = Ct(vv / mask + '0');
    vv %= mask, mask /= 10;
    temp[ii++] = Ct('.');
    for (int8 i{}; i < 15 && vv != 0; ++i) {
      if (count == 6) ii -= 6;
      auto cc = vv / mask;
      count = cc == 0 ? count + 1 : 0;
      if (count == 3) return string<Ct>(temp, ii - 6);
      temp[ii++] = Ct(cc + '0'), vv %= mask, mask /= 10;
    }
    temp[ii++] = Ct('e');
    temp[ii++] = Ct('+');
    temp[ii++] = Ct(exp / 100 + '0');
    exp %= 100;
    temp[ii++] = Ct(exp / 10 + '0');
    temp[ii++] = Ct(exp % 10 + '0');
    return string<Ct>(temp, ii);
  }
}

}

export namespace yw {

/// \brief returns the length of a stringable object
inline constexpr auto strlen = []<stringable St>(St&& Str) -> nat {
  if constexpr (is_array<remove_ref<St>>) return extent<St> - 1;
  else if constexpr (is_pointer<remove_ref<St>>) return std::char_traits<iter_value<St>>::length(Str);
  else if constexpr (specialization_of<remove_cvref<St>, std::basic_string>) return Str.size();
  else if constexpr (specialization_of<remove_cvref<St>, std::basic_string_view>) return Str.size();
  else return string_view<iter_value<St>>(Str).size();
};

/// \brief changes the encoding of a string
/// \param s string to change encoding
/// \return converted string
template<character Out> inline constexpr auto cvt = //
  [](stringable auto&& s) noexcept -> string<Out> requires same_as<Out, remove_cvref<Out>> {
  using In = remove_cvref<iter_value<decltype(s)>>;
  string_view<In> v(s);
  if constexpr (sizeof(In) == sizeof(Out)) return string<Out>(bitcast<string_view<Out>>(v));
  else if constexpr (included_in<In, cat1, uct1>) {
    if constexpr (same_as<Out, uct4>) {
      string<uct4> r(v.size(), {});
      auto p = r.data();
      for (auto i = v.data(), last = i + v.size(); i < last;) {
        uct1 c = *i++;
        if (c < 0x80) *p++ = c;
        else if (c < 0xc0) *p++ = 0xfffe;
        else if (c < 0xe0) *p++ = (c & 0x1f) << 6 | (*i++ & 0x3f);
        else if (c < 0xf0) *p++ = (c & 0x0f) << 12 | (*i++ & 0x3f) << 6 | (*i++ & 0x3f);
        else if (c < 0xf8) *p++ = (c & 0x07) << 18 | (*i++ & 0x3f) << 12 | (*i++ & 0x3f) << 6 | (*i++ & 0x3f);
        else *p++ = 0xfffe;
      }
      r.resize(p - r.data());
      return r;
    } else if constexpr (included_in<Out, cat2, uct2>) return cvt<Out>(cvt<uct4>(s));
  } else if constexpr (included_in<In, cat2, uct2>) {
    if constexpr (same_as<Out, uct4>) {
      string<uct4> r(v.size(), {});
      auto p = r.data();
      for (auto i = v.data(), last = i + v.size(); i < last;) {
        if (*i < 0xd800 || *i >= 0xe000) *p++ = *i++;
        else *p++ = *i++ & 0x03ff_u2 | (*i++ & 0x03ff_u2 << 10);
      }
      r.resize(p - r.data());
      return r;
    } else if constexpr (included_in<Out, cat1, uct1>) return cvt<Out>(cvt<uct4>(s));
  } else if constexpr (same_as<In, uct4>) {
    if constexpr (included_in<Out, cat1, uct1>) {
      string<Out> r(v.size() * 4, {});
      auto p = r.data();
      for (auto i = v.data(), last = i + v.size(); i < last;) {
        uct4 c = *i++;
        if (c < 0x80) *p++ = Out(c);
        else if (c < 0x800) *p++ = Out(0xc0 | (c >> 6)), *p++ = Out(0x80 | (c & 0x3f));
        else if (c < 0x10000) *p++ = Out(0xe0 | (c >> 12)), *p++ = Out(0x80 | ((c >> 6) & 0x3f)), *p++ = Out(0x80 | (c & 0x3f));
        else *p++ = Out(0xf0 | (c >> 18)), *p++ = Out(0x80 | ((c >> 12) & 0x3f)), *p++ = Out(0x80 | ((c >> 6) & 0x3f)), *p++ = Out(0x80 | (c & 0x3f));
      }
      r.resize(p - r.data());
      return r;
    } else if constexpr (included_in<Out, cat2, uct2>) {
      string<Out> r(v.size() * 2, {});
      auto p = r.data();
      for (auto i = v.data(), last = i + v.size(); i < last;) {
        auto c = *i++;
        if (c < 0x10000) *p++ = Out(c);
        else *p++ = Out(0xd800 | ((c - 0x10000) >> 10)), *p++ = Out(0xdc00_u2 | ((c - 0x10000) & 0x3ff));
      }
      r.resize(p - r.data());
      return r;
    }
  }
};

/// \brief checks if a character is a digit
inline constexpr auto is_digit = []<character Ct>(const Ct c) noexcept { return (natcast(c) ^ 0x30) < 10; };

/// \brief converts a string to a value
/// \tparam T arithmetic type to convert to
/// \param s string to convert
/// \return converted value
/// \note if `s` is `string_view&`, it represents the rest part not used to convert
template<arithmetic T> inline constexpr auto stov = []<stringable St>(St&& s) noexcept -> T {
  if constexpr (same_as<St, string_view<iter_value<decltype(s)>>&>) {
    if constexpr (floating_point<T>) return T(_::_stof(s));
    else if constexpr (std::signed_integral<T>) return T(_::_stoi(s));
    else if constexpr (std::unsigned_integral<T>) return T(_::_ston(s));
  } else {
    string_view<iter_value<remove_ref<decltype(s)>>> t(s);
    if constexpr (floating_point<T>) return T(_::_stof(t));
    else if constexpr (std::signed_integral<T>) return T(_::_stoi(t));
    else if constexpr (std::unsigned_integral<T>) return T(_::_ston(t));
  }
};

/// \brief converts a string to a integer value
inline constexpr auto stoi = stov<int8>;

/// \brief converts a string to a unsigned integer value
inline constexpr auto ston = stov<nat8>;

/// \brief converts a string to a floating point value
inline constexpr auto stof = stov<fat8>;

/// \brief converts a integer value to a hexadecimal string
template<character Ct> inline constexpr auto to_hex = []<integral T>(const T Val) noexcept -> string<Ct> {
  string<Ct> r(16, '0');
  for (nat i = 0, n = natcast(Val); i < 16; ++i, n >>= 4) r[15 - i] = _::_vtos_codes[n & 0xf];
  return r;
};

/// \brief converts a scalar value to a string
template<character Ct> inline constexpr auto to_str = []<scalar T>(const T Val) noexcept -> string<Ct> {
  if constexpr (is_nullptr<T>) return string<Ct>(16, '0');
  else if constexpr (is_pointer<T>) {
    if (!is_cev) return to_hex<Ct>(reinterpret_cast<nat>(Val));
  } else if constexpr (is_bool<T>) {
    if (Val) return string<Ct>(array<Ct, 5>{Ct('t'), Ct('r'), Ct('u'), Ct('e'), Ct{}});
    else return string<Ct>(array<Ct, 6>{Ct('f'), Ct('a'), Ct('l'), Ct('s'), Ct('e'), Ct{}});
  } else if constexpr (is_none<T>) return string<Ct>(array<Ct, 4>{Ct('n'), Ct('o'), Ct('n'), Ct('e'), Ct{}});
  else if constexpr (floating_point<T>) return _::_ftos<Ct>(Val);
  else if constexpr (signed_integral<T>) return _::_itos<Ct>(Val);
  else if constexpr (unsigned_integral<T>) return _::_ntos<Ct>(Val);
};

/// \brief file path class
using path = std::filesystem::path;

////////////////////////////////////////////////////////////////////////////////
/// \brief source location
struct source {
  explicit source(const cat1*) = delete;

  /// \brief file name
  const cat1* const file;

  /// \brief function name
  const cat1* const func;

  /// \brief line number
  const nat4 line;

  /// \brief column number
  const nat4 column;

  /// \brief default constructor
  source(const cat1* File = __builtin_FILE(), const cat1* Func = __builtin_FUNCTION(), //
         nat4 Line = __builtin_LINE(), nat4 Column = __builtin_COLUMN()) noexcept
    : file(File), func(Func), line(Line), column(Column) {}
};

}

namespace std {

template<typename Ct> struct formatter<yw::source, Ct> : formatter<basic_string<Ct>, Ct> {
  static constexpr Ct _fmt[]{Ct('{'), Ct('}'), Ct('('), Ct('{'), Ct('}'), Ct(','), Ct('{'), Ct('}'), Ct(')'), Ct('\0')};
  auto format(const yw::source& s, auto& Ctx) const { return formatter<basic_string<Ct>, Ct>::format(std::format(_fmt, s.file, s.line, s.column), Ctx); }
};

}

export namespace yw {

////////////////////////////////////////////////////////////////////////////////
/// \brief date structure
struct date {

  /// \brief year
  int4 year{};

  /// \brief month
  int4 month{};

  /// \brief day
  int4 day{};

  /// \brief default constructor
  date() : date(std::chrono::zoned_time(std::chrono::current_zone(), std::chrono::system_clock::now()).get_local_time()) {}

  /// \brief constructor from components
  date(numeric auto&& Year, numeric auto&& Month, numeric auto&& Day) noexcept : year(int4(Year)), month(int4(Month)), day(int4(Day)) {}

  /// \brief constructor from `std::chrono::time_point`
  template<typename Clock, typename Duration> date(const std::chrono::time_point<Clock, Duration>& tp) {
    const auto ymd = std::chrono::year_month_day(std::chrono::floor<std::chrono::days>(tp));
    year = int4(ymd.year()), month = int4(nat4(ymd.month())), day = int4(nat4(ymd.day()));
  }

  /// \brief converts to `string<cat1>`
  operator string<cat1>() const { return std::format("{:04d}-{:02d}-{:02d}", year, month, day); }

  /// \brief converts to `string<cat2>`
  explicit operator string<cat2>() const { return std::format(L"{:04d}-{:02d}-{:02d}", year, month, day); }

  /// \brief converts to `string<uct1>`
  explicit operator string<uct1>() const {
    return [this](auto s) { return std::format_to(s.data(), "{:04d}-{:02d}-{:02d}", year, month, day), mv(s); }(string<uct1>(10, {}));
  }

  /// \brief converts to `string<uct2>`
  explicit operator string<uct2>() const {
    return [this](auto s) { return std::format_to(s.data(), L"{:04d}-{:02d}-{:02d}", year, month, day), mv(s); }(string<uct2>(10, {}));
  }

  /// \brief converts to `string<uct4>`
  explicit operator string<uct4>() const {
    return [this](auto s) { return std::ranges::copy(std::format("{:04d}-{:02d}-{:02d}", year, month, day), s.data()), mv(s); }(string<uct4>(10, {}));
  }
};

////////////////////////////////////////////////////////////////////////////////
/// \brief clock structure
struct clock {
  /// \brief hour
  int4 hour{};

  /// \brief minute
  int4 minute{};

  /// \brief second
  int4 second{};

  /// \brief default constructor
  clock() : clock(std::chrono::zoned_time(std::chrono::current_zone(), std::chrono::system_clock::now()).get_local_time()) {}

  /// \brief constructor from components
  clock(numeric auto&& Hour, numeric auto&& Minute, numeric auto&& Second) noexcept : hour(int4(Hour)), minute(int4(Minute)), second(int4(Second)) {}

  /// \brief constructor from `std::chrono::time_point`
  template<typename Clock, typename Duration> clock(const std::chrono::time_point<Clock, Duration>& tp) {
    const auto hms = std::chrono::hh_mm_ss(std::chrono::floor<std::chrono::seconds>(tp - std::chrono::floor<std::chrono::days>(tp)));
    hour = int4(hms.hours().count()), minute = int4(hms.minutes().count()), second = int4(hms.seconds().count());
  }

  /// \brief converts to `string<cat1>`
  operator string<cat1>() const { return std::format("{:02d}:{:02d}:{:02d}", hour, minute, second); }

  /// \brief converts to `string<cat2>`
  explicit operator string<cat2>() const { return std::format(L"{:02d}:{:02d}:{:02d}", hour, minute, second); }

  /// \brief converts to `string<uct1>`
  explicit operator string<uct1>() const {
    return [this](auto s) { return std::format_to(s.data(), "{:02d}:{:02d}:{:02d}", hour, minute, second), mv(s); }(string<uct1>(8, {}));
  }

  /// \brief converts to `string<uct2>`
  explicit operator string<uct2>() const {
    return [this](auto s) { return std::format_to(s.data(), L"{:02d}:{:02d}:{:02d}", hour, minute, second), mv(s); }(string<uct2>(8, {}));
  }

  /// \brief converts to `string<uct4>`
  explicit operator string<uct4>() const {
    return [this](auto s) { return std::ranges::copy(std::format("{:02d}:{:02d}:{:02d}", hour, minute, second), s.data()), mv(s); }(string<uct4>(8, {}));
  }
};

////////////////////////////////////////////////////////////////////////////////
/// \brief date and clock structure
struct time {

  /// \brief date
  yw::date date;

  /// \brief clock
  yw::clock clock;

  /// \brief default constructor
  time() : time(std::chrono::zoned_time(std::chrono::current_zone(), std::chrono::system_clock::now()).get_local_time()) {}

  /// \brief constructor from components
  time(const yw::date& Date, const yw::clock& Clock) noexcept : date(Date), clock(Clock) {}

  /// \brief constructor from `std::chrono::time_point`
  template<typename Clock, typename Duration> time(const std::chrono::time_point<Clock, Duration>& tp) : date(tp), clock(tp) {}

  /// \brief converts to date
  operator yw::date() const { return date; }

  /// \brief converts to clock
  operator yw::clock() const { return clock; }

  /// \brief converts to `string<cat1>`
  operator string<cat1>() const {                                   //
    return std::format("{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}", //
                       date.year, date.month, date.day, clock.hour, clock.minute, clock.second);
  }

  /// \brief converts to `string<cat2>`
  explicit operator string<cat2>() const {                           //
    return std::format(L"{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}", //
                       date.year, date.month, date.day, clock.hour, clock.minute, clock.second);
  }

  /// \brief converts to `string<uct1>`
  explicit operator string<uct1>() const { //
    auto s = string<uct1>(19, {});
    std::format_to(s.data(), "{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}", //
                   date.year, date.month, date.day, clock.hour, clock.minute, clock.second);
    return s;
  }

  /// \brief converts to `string<uct2>`
  explicit operator string<uct2>() const { //
    auto s = string<uct2>(19, {});
    std::format_to(s.data(), L"{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}", //
                   date.year, date.month, date.day, clock.hour, clock.minute, clock.second);
    return s;
  }

  /// \brief converts to `string<uct4>`
  explicit operator string<uct4>() const { //
    auto s = string<uct4>(19, {});
    std::ranges::copy(std::format("{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}", //
                                  date.year, date.month, date.day, clock.hour, clock.minute, clock.second),
                      s.data());
    return s;
  }
};

/// \brief caster to obtain the current time
inline constexpr caster now{[] { return time{}; }};

}

namespace std {

template<typename Ct> struct formatter<yw::date, Ct> : formatter<basic_string<Ct>, Ct> {
  auto format(const yw::date& d, auto& Ctx) const { return formatter<basic_string<Ct>, Ct>::format(basic_string<Ct>(d), Ctx); }
};

template<typename Ct> struct formatter<yw::clock, Ct> : formatter<basic_string<Ct>, Ct> {
  auto format(const yw::clock& c, auto& Ctx) const { return formatter<basic_string<Ct>, Ct>::format(basic_string<Ct>(c), Ctx); }
};

template<typename Ct> struct formatter<yw::time, Ct> : formatter<basic_string<Ct>, Ct> {
  auto format(const yw::time& t, auto& Ctx) const { return formatter<basic_string<Ct>, Ct>::format(basic_string<Ct>(t), Ctx); }
};

}

export namespace yw {

////////////////////////////////////////////////////////////////////////////////
/// \brief logger
class logger {
protected:
  string<cat1> text{};
public:
  /// \brief logging level type
  struct level_t {
    string_view<cat1> name;
    int value;
    friend bool operator==(const level_t& l, const level_t& r) noexcept { return l.value == r.value; }
    friend auto operator<=>(const level_t& l, const level_t& r) noexcept { return l.value <=> r.value; }
  };

  /// \brief level to output all logs
  static constexpr level_t all{"all", 0};

  /// \brief level to output debug logs
  static constexpr level_t debug{"debug", 10};

  /// \brief level to output info logs
  static constexpr level_t info{"info", 20};

  /// \brief level to output warning logs
  static constexpr level_t warning{"warn", 30};

  /// \brief level to output error logs
  static constexpr level_t error{"error", 40};

  /// \brief level to output critical logs
  static constexpr level_t critical{"critical", 50};

  /// \brief file path
  path path;

  /// \brief level
  level_t level = info;

  /// \brief flag to output to console
  bool console = true;

  /// \brief constructor
  logger(const yw::path& File) noexcept : path(File) {}

  /// \brief destructor; writes to log
  ~logger() noexcept {
    try {
      if (std::basic_ofstream<cat1> ofs(path, std::ios::app); ofs) ofs << text, text.clear();
      else std::cerr << "failed to open log file: " << path << std::endl;
    } catch (...) { std::cerr << "failed to write log file: " << path << std::endl; }
  }

  /// \brief appends text to log
  void operator()(const level_t& Level, stringable auto&& Text) noexcept {
    if (Level < level) return;
    try {
      string<cat1> s;
      if constexpr (same_as<iter_value<decltype(Text)>, cat1>) {
        s = std::format("{} [{}] {}\n", now(), Level.name, string_view<cat1>(Text));
      } else s = std::format("{} [{}] {}\n", now(), Level.name, cvt<cat1>(Text));
      if (console) std::cout << s;
      text += s;
    } catch (...) { std::cerr << "failed to write log" << std::endl; }
  }

  /// \brief appends text to log
  void operator()(const level_t& Level, stringable auto&& Text, const source& Source) noexcept {
    if (Level < level) return;
    if (level > debug) return operator()(Level, fwd<decltype(Text)>(Text)); // avoid leaking source code information
    try {
      string<cat1> s = std::format("{} [{}] {}: {}\n", now(), Level.name, Source, cvt<cat1>(Text));
      if (console) std::cout << s;
      text += s;
    } catch (...) { std::cerr << "failed to write log" << std::endl; }
  }

  /// \brief checks level and appends formatted text to the log
  template<typename... Ts> void format(const level_t& Level, const std::format_string<Ts...> Format, Ts&&... Args) noexcept {
    if (Level < level) return;
    try {
      string<cat1> s;
      if constexpr (sizeof...(Ts) == 0) s = std::format("{} [{}] {}\n", now(), Level.name, Format);
      else s = std::format("{} [{}] {}\n", now(), Level.name, std::format(Format, fwd<Ts>(Args)...));
      if (console) std::cout << s;
      text += s;
    } catch (...) { std::cerr << "failed to write log" << std::endl; }
  }

  /// \brief checks level and appends formatted text to the log
  template<typename... Ts> void format(const level_t& Level, const std::wformat_string<Ts...> Format, Ts&&... Args) noexcept {
    if (Level < level) return;
    try {
      string<cat2> s;
      if constexpr (sizeof...(Ts) == 0) s = std::format(L"{} [{}] {}\n", now(), Level.name, Format);
      else s = std::format(L"{} [{}] {}\n", now(), cvt<cat2>(Level.name), std::format(Format, fwd<Ts>(Args)...));
      if (console) std::wcout << s;
      text += cvt<cat1>(s);
    } catch (...) { std::wcerr << "failed to write log" << std::endl; }
  }

  /// \brief flushes the log
  void flush() noexcept {
    try {
      if (std::basic_ofstream<cat1> ofs(path, std::ios::app); ofs) ofs << text, text.clear();
      else std::cerr << "failed to open log file: " << path << std::endl;
    } catch (...) { std::cerr << "failed to write log file: " << path << std::endl; }
  }
};

} // namespace yw
