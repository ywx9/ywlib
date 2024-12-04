#pragma once

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

#ifndef nat
#define nat size_t
#endif

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

template<typename T, typename... Ts> concept same_as = (std::same_as<T, Ts> && ...);
template<typename T, typename... Ts> concept included_in = (std::same_as<T, Ts> || ...);
template<typename T, typename... Ts> concept different_from = !included_in<T, Ts...>;
template<typename T, typename... Ts> concept derived_from = (std::derived_from<T, Ts> && ...);
template<typename T, typename... Ts> concept convertible_to = (std::convertible_to<T, Ts> && ...);
template<typename T, typename... Ts> concept nt_convertible_to = convertible_to<T, Ts...> && (std::is_nothrow_convertible_v<T, Ts> && ...);
template<typename T, typename... Ts> concept castable_to = requires(T (&f)()) { ((static_cast<Ts>(f())) && ...); };
template<typename T, typename... Ts> concept nt_castable_to = castable_to<T, Ts...> && requires(T (&f)() noexcept) { requires (noexcept(static_cast<Ts>(f())) && ...); };
template<typename T> concept numeric = nt_castable_to<T, bool, int4, int8, nat4, nat8, fat4, fat8>;

struct pass {
  template<typename T> constexpr T&& operator()(T&& a) const noexcept { return static_cast<T&&>(a); }
};

template<auto V, typename T = decltype(V)> requires convertible_to<decltype(V), T> struct constant {
  using type = T;
  static constexpr type value = V;
  consteval operator type() const noexcept { return value; }
  consteval type operator()() const noexcept { return value; }
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

struct value {
  fat8 _{};
  constexpr value() noexcept = default;
  constexpr value(none) noexcept : _(std::numeric_limits<fat8>::quiet_NaN()) {}
  template<numeric T> requires different_from<T, value> constexpr value(T&& v) noexcept : _(static_cast<fat8>(v)) {}
  template<numeric T> requires different_from<T, value> constexpr value& operator=(T&& v) noexcept { return _ = static_cast<fat8>(v), *this; }
  constexpr operator fat8&() & noexcept { return _; }
  constexpr operator const fat8&() const& noexcept { return _; }
  constexpr operator fat8&&() && noexcept { return static_cast<fat8&&>(_); }
  constexpr operator const fat8&&() const&& noexcept { return static_cast<const fat8&&>(_); }
  template<typename T> requires castable_to<fat8, T> explicit constexpr operator T() const noexcept(nt_castable_to<fat8, T>) { return static_cast<T>(_); }
  template<numeric T> constexpr value& operator+=(T&& v) noexcept { return _ += static_cast<fat8>(v), *this; }
  template<numeric T> constexpr value& operator-=(T&& v) noexcept { return _ -= static_cast<fat8>(v), *this; }
  template<numeric T> constexpr value& operator*=(T&& v) noexcept { return _ *= static_cast<fat8>(v), *this; }
  template<numeric T> constexpr value& operator/=(T&& v) noexcept { return _ /= static_cast<fat8>(v), *this; }
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

template<typename T> struct t_type : std::type_identity<T> {};
template<typename... Ts> concept valid = (requires { typename t_type<Ts>::type; } && ...);

template<nat N> struct t_cat_type : t_type<none> {};
template<> struct t_cat_type<1> : t_type<cat1> {};
template<> struct t_cat_type<2> : t_type<cat2> {};
template<nat N> using cat_type = typename t_cat_type<N>::type;

template<nat N> struct t_uct_type : t_type<none> {};
template<> struct t_uct_type<1> : t_type<uct1> {};
template<> struct t_uct_type<2> : t_type<uct2> {};
template<> struct t_uct_type<4> : t_type<uct4> {};
template<nat N> using uct_type = typename t_uct_type<N>::type;

template<nat N> struct t_int_type : t_type<none> {};
template<> struct t_int_type<1> : t_type<int1> {};
template<> struct t_int_type<2> : t_type<int2> {};
template<> struct t_int_type<4> : t_type<int4> {};
template<> struct t_int_type<8> : t_type<int8> {};
template<nat N> using int_type = typename t_int_type<N>::type;

template<nat N> struct t_nat_type : t_type<none> {};
template<> struct t_nat_type<1> : t_type<nat1> {};
template<> struct t_nat_type<2> : t_type<nat2> {};
template<> struct t_nat_type<4> : t_type<nat4> {};
template<> struct t_nat_type<8> : t_type<nat8> {};
template<nat N> using nat_type = typename t_nat_type<N>::type;

template<nat N> struct t_fat_type : t_type<none> {};
template<> struct t_fat_type<4> : t_type<fat4> {};
template<> struct t_fat_type<8> : t_type<fat8> {};
template<nat N> using fat_type = typename t_fat_type<N>::type;

template<typename T> struct t_remove_const : t_type<T> {};
template<typename T> struct t_remove_const<const T> : t_type<T> {};
template<typename T> using remove_const = typename t_remove_const<T>::type;

template<typename T> struct t_remove_volatile : t_type<T> {};
template<typename T> struct t_remove_volatile<volatile T> : t_type<T> {};
template<typename T> using remove_volatile = typename t_remove_volatile<T>::type;

template<typename T> struct t_remove_cv : t_remove_const<remove_volatile<T>> {};
template<typename T> using remove_cv = typename t_remove_cv<T>::type;

template<typename T> struct t_remove_ref : t_type<T> {};
template<typename T> struct t_remove_ref<T&> : t_type<T> {};
template<typename T> struct t_remove_ref<T&&> : t_type<T> {};
template<typename T> using remove_ref = typename t_remove_ref<T>::type;

template<typename T> struct t_remove_cvref : t_remove_cv<remove_ref<T>> {};
template<typename T> using remove_cvref = typename t_remove_cvref<T>::type;

template<typename T> struct t_remove_pointer : t_type<T> {};
template<typename T> struct t_remove_pointer<T*> : t_type<T> {};
template<typename T> struct t_remove_pointer<T* const> : t_type<T> {};
template<typename T> struct t_remove_pointer<T* volatile> : t_type<T> {};
template<typename T> struct t_remove_pointer<T* const volatile> : t_type<T> {};
template<typename T> using remove_pointer = typename t_remove_pointer<T>::type;

template<typename T> struct t_remove_all_pointers : t_type<T> {};
template<typename T> struct t_remove_all_pointers<T*> : t_remove_all_pointers<T> {};
template<typename T> struct t_remove_all_pointers<T* const> : t_remove_all_pointers<T> {};
template<typename T> struct t_remove_all_pointers<T* volatile> : t_remove_all_pointers<T> {};
template<typename T> struct t_remove_all_pointers<T* const volatile> : t_remove_all_pointers<T> {};
template<typename T> using remove_all_pointers = typename t_remove_all_pointers<T>::type;

template<typename T> struct t_remove_extent : t_type<T> {};
template<typename T> struct t_remove_extent<T[]> : t_type<T> {};
template<typename T, nat N> struct t_remove_extent<T[N]> : t_type<T> {};
template<typename T> using remove_extent = typename t_remove_extent<T>::type;

template<typename T> struct t_remove_all_extents : t_type<T> {};
template<typename T> struct t_remove_all_extents<T[]> : t_remove_all_extents<T> {};
template<typename T, nat N> struct t_remove_all_extents<T[N]> : t_remove_all_extents<T> {};
template<typename T> using remove_all_extents = typename t_remove_all_extents<T>::type;

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

template<typename T> struct t_is_member_pointer : constant<false> {};
template<typename M, typename C> struct t_is_member_pointer<M C::*> : constant<true> {
  using class_type = C;
  using member_type = M;
};
template<typename M, typename C> struct t_is_member_pointer<M C::* const> : t_is_member_pointer<M C::*> {};
template<typename M, typename C> struct t_is_member_pointer<M C::* volatile> : t_is_member_pointer<M C::*> {};
template<typename M, typename C> struct t_is_member_pointer<M C::* const volatile> : t_is_member_pointer<M C::*> {};
template<typename T> concept is_member_pointer = t_is_member_pointer<T>::value && is_pointer<T>;
template<is_member_pointer T> using class_type = typename t_is_member_pointer<T>::class_type;
template<is_member_pointer T> using member_type = typename t_is_member_pointer<T>::member_type;
template<typename T> concept is_member_function_pointer = is_member_pointer<T> && is_function<member_type<T>>;
template<typename T> concept is_member_object_pointer = is_member_pointer<T> && !is_member_function_pointer<T>;

template<typename T> concept is_enum = std::is_enum_v<T>;
template<typename T> concept is_scoped_enum = is_enum<T> && !convertible_to<T, bool>;
template<is_enum T> using underlying_type = std::underlying_type_t<T>;

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
template<typename T> concept scalar = arithmetic<T> || is_pointer<T> || is_nullptr<T> || is_none<T> || is_enum<T>;

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

template<character Ct> using string = std::basic_string<Ct>;
template<character Ct> using string_view = std::basic_string_view<Ct>;
template<character Ct, typename... Args> using format_string = std::basic_format_string<Ct, Args...>;

template<typename... Fs> struct overload : public Fs... {
  using Fs::operator()...;
};

#define ywlib_compare_op(Name, Op)                                                     \
  struct Name {                                                                        \
    constexpr bool operator()(auto&& a, auto&& b) const ywlib_wrap_auto(bool(a Op b)); \
  }

ywlib_compare_op(equal, ==);
ywlib_compare_op(not_equal, !=);
ywlib_compare_op(less, <);
ywlib_compare_op(greater, >);
ywlib_compare_op(less_equal, <=);
ywlib_compare_op(greater_equal, >=);

inline constexpr equal eq;
inline constexpr not_equal ne;
inline constexpr less lt;
inline constexpr greater gt;
inline constexpr less_equal le;
inline constexpr greater_equal ge;

template<typename T> struct t_add_const : t_type<const T> {};
template<typename T> struct t_add_const<T&> : t_type<const T&> {};
template<typename T> struct t_add_const<T&&> : t_type<const T&&> {};
template<typename T> using add_const = typename t_add_const<T>::type;

template<typename T> struct t_add_volatile : t_type<volatile T> {};
template<typename T> struct t_add_volatile<T&> : t_type<volatile T&> {};
template<typename T> struct t_add_volatile<T&&> : t_type<volatile T&&> {};
template<typename T> using add_volatile = typename t_add_volatile<T>::type;

template<typename T> struct t_add_cv : t_add_const<add_volatile<T>> {};
template<typename T> using add_cv = add_const<add_volatile<T>>;

template<typename T> struct t_add_lvref : t_type<T> {};
template<typename T> requires valid<T&> struct t_add_lvref<T> : t_type<T&> {};
template<typename T> using add_lvref = typename t_add_lvref<T>::type;

template<typename T> struct t_add_rvref : t_type<T> {};
template<typename T> requires valid<T&&> struct t_add_rvref<T> : t_type<T&&> {};
template<typename T> requires valid<T&&> struct t_add_rvref<T&> : t_type<T&&> {};
template<typename T> using add_rvref = typename t_add_rvref<T>::type;

template<typename T> struct t_add_fwref : t_type<T> {};
template<typename T> requires valid<T&&> struct t_add_fwref<T> : t_type<T&&> {};
template<typename T> using add_fwref = typename t_add_fwref<T>::type;

template<typename T> struct t_add_pointer : t_type<T> {};
template<typename T> requires valid<T*> struct t_add_pointer<T> : t_type<T*> {};
template<typename T> requires valid<T*> struct t_add_pointer<T&> : t_type<T*&> {};
template<typename T> requires valid<T*> struct t_add_pointer<T&&> : t_type<T*&&> {};
template<typename T> using add_pointer = typename t_add_pointer<T>::type;

template<typename T, nat N> struct t_add_extent : t_type<T> {};
template<typename T, nat N> requires valid<T[N]> struct t_add_extent<T, N> : t_type<T[N]> {};
template<typename T, nat N> requires valid<T[N]> struct t_add_extent<T&, N> : std::add_lvalue_reference<T[N]> {};
template<typename T, nat N> requires valid<T[N]> struct t_add_extent<T&&, N> : std::add_rvalue_reference<T[N]> {};
template<typename T> requires valid<T[]> struct t_add_extent<T, 0> : t_type<T[]> {};
template<typename T> requires valid<T[]> struct t_add_extent<T&, 0> : std::add_lvalue_reference<T[]> {};
template<typename T> requires valid<T[]> struct t_add_extent<T&&, 0> : std::add_rvalue_reference<T[]> {};
template<typename T, nat N> using add_extent = typename t_add_extent<T, N>::type;

inline consteval auto cev(auto a) noexcept { return a; }
inline consteval auto arraysize(const is_bounded_array auto& a) noexcept { return std::extent_v<std::remove_reference_t<decltype(a)>>; }
inline constexpr auto addressof = []<is_lvref T>(T&& Ref) noexcept { return __builtin_addressof(Ref); };
inline constexpr auto asconst = []<typename T>(T&& Ref) noexcept -> add_const<T&&> { return static_cast<add_const<T&&>>(Ref); };
inline constexpr auto mv = []<typename T>(T&& Ref) noexcept -> add_rvref<T> { return static_cast<add_rvref<T>>(Ref); };
template<typename T> inline constexpr auto fwd = [](auto&& Ref) noexcept -> T&& { return static_cast<T&&>(Ref); };
template<typename T> inline constexpr auto declval = []() noexcept -> add_fwref<T> {};

template<bool... Bs> inline constexpr nat counts = (Bs + ...);
template<bool... Bs> inline constexpr nat inspects = 0;
template<bool B, bool... Bs> inline constexpr nat inspects<B, Bs...> = B ? 0 : 1 + inspects<Bs...>;
template<convertible_to<nat> auto I, nat N> inline constexpr bool selectable = (is_bool<decltype(I)> && N == 2) || (!is_bool<decltype(I)> && nat(I) < N);

} // namespace yw

namespace std {

template<typename T> struct common_type<yw::none, T> : type_identity<yw::none> {};
template<typename T> struct common_type<T, yw::none> : type_identity<yw::none> {};

template<typename T> struct common_type<yw::value, T> : std::common_type<yw::fat8, T> {};
template<typename T> struct common_type<T, yw::value> : std::common_type<T, yw::fat8> {};

}

namespace yw::_ {
// clang-format off

template<typename T, T I, typename... Ts> struct _select_type : _select_type<nat, I, Ts...> {};
template<bool B, typename Tt, typename Tf> struct _select_type<bool, B, Tt, Tf> : std::conditional<B, Tt, Tf> {};
template<nat I, typename T, typename... Ts> struct _select_type<nat, I, T, Ts...> : _select_type<nat, I - 1, Ts...> {};
template<typename T, typename... Ts> struct _select_type<nat, 0, T, Ts...> : t_type<T> {};

// clang-format on
}

export namespace yw {

/// selects a type from the list
template<convertible_to<nat> auto I, typename... Ts> requires selectable<I, sizeof...(Ts)> //
using select_type = typename _::_select_type<decltype(I), I, Ts...>::type;

/// selects a value from the list
template<convertible_to<nat> auto I, auto... Vs> requires selectable<I, sizeof...(Vs)> //
inline constexpr auto select_value = select_type<I, constant<Vs>...>::value;

/// selects a parameter from the list
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

template<typename T> concept destructible = requires { requires std::is_destructible_v<T>; };
template<typename T> concept nt_destructible = destructible<T> && std::destructible<T>;

template<typename T, typename A> concept assignable = requires { requires std::is_assignable_v<T, A>; };
template<typename T, typename A> concept nt_assignable = assignable<T, A> && std::is_nothrow_assignable_v<T, A>;
inline constexpr auto assign = []<typename T, typename A>(T& Ref, A&& Arg) noexcept(nt_assignable<T&, A>) requires assignable<T&, A> { Ref = fwd<A>(Arg); };

template<typename T, typename... As> concept constructible = requires { T{declval<As>()...}; };
template<typename T, typename... As> concept nt_constructible = constructible<T, As...> && requires { requires noexcept(T{declval<As>()...}); };
template<typename T> inline constexpr auto construct = []<typename... As>(As&&... Args) noexcept(nt_constructible<T, As...>) -> T requires constructible<T, As...> { return T{fwd<As>(Args)...}; };

template<typename T, typename U = T> concept exchangeable = constructible<T, T> && assignable<T&, U>;
template<typename T, typename U = T> concept nt_exchangeable = nt_constructible<T, T> && nt_assignable<T&, U>;
inline constexpr auto exchange = []<typename Tr, typename Tv = Tr>(Tr& Ref, Tv&& Value) noexcept(nt_exchangeable<Tr, Tv>) -> Tr requires exchangeable<Tr&, Tv> {
  auto a = mv(Ref);
  Ref = fwd<Tv>(Value);
  return a;
};

template<typename T, typename U = T> concept swappable = std::swappable_with<T, U>;
template<typename T, typename U = T> concept nt_swappable = swappable<T, U> && std::is_nothrow_swappable_with_v<T, U>;
inline constexpr auto swap = []<typename Tr, typename Tv = Tr>(Tr& Ref, Tv& Value) noexcept(nt_swappable<Tr, Tv>) requires swappable<Tr, Tv> { std::swap(Ref, Value); };

template<typename F, typename... As> concept invocable = std::invocable<F, As...>;
template<typename F, typename... As> concept nt_invocable = invocable<F, As...> && requires { requires std::is_nothrow_invocable_v<F, As...>; };
template<typename F, typename... As> requires invocable<F, As...> using invoke_result = select_type<is_void<std::invoke_result_t<F, As...>>, none, std::invoke_result_t<F, As...>>;
template<typename F, typename R, typename... As> concept invocable_r = invocable<F, As...> && convertible_to<invoke_result<F, As...>, R>;
template<typename F, typename R, typename... As> concept nt_invocable_r = nt_invocable<F, As...> && nt_convertible_to<invoke_result<F, As...>, R>;
inline constexpr auto invoke = []<typename F, typename... As>(F&& Func, As&&... Args) noexcept(nt_invocable<F, As...>) requires invocable<F, As...> {
  if constexpr (is_void<invoke_result<F, As...>>) return std::invoke(fwd<F>(Func), fwd<As>(Args)...), none{};
  else return std::invoke(fwd<F>(Func), fwd<As>(Args)...);
};
template<typename R> inline constexpr auto invoke_r = []<typename F, typename... As>(F&& f, As&&... as) //
  noexcept(nt_invocable_r<F, R, As...>) -> R requires invocable_r<F, R, As...> { return std::invoke_r<R>(fwd<F>(f), fwd<As>(as)...); };

template<typename... Ts> struct t_common_type : t_type<none> {};
template<typename... Ts> requires valid<std::common_type_t<Ts...>> struct t_common_type<Ts...> : std::common_type<Ts...> {};
template<typename... Ts> using common_type = typename t_common_type<Ts...>::type;
template<typename... Ts> concept common_with = !is_none<common_type<Ts...>> && (std::common_reference_with<common_type<Ts...>, Ts> && ...);

template<typename T, template<typename...> typename Tm> struct t_specialization_of : constant<false> {};
template<template<typename...> typename Tm, typename... Ts> struct t_specialization_of<Tm<Ts...>, Tm> : constant<true> {};
template<typename T, template<typename...> typename Tm> concept specialization_of = t_specialization_of<T, Tm>::value;

template<typename T, typename U> struct t_variation_of : constant<false> {};
template<template<typename...> typename Tm, typename... Ts, typename... Us> struct t_variation_of<Tm<Ts...>, Tm<Us...>> : constant<true> {};
template<template<typename, auto...> typename Tm, typename T, auto... Vs, typename U, auto... Ws> struct t_variation_of<Tm<T, Vs...>, Tm<U, Ws...>> : constant<true> {};
template<template<auto, typename...> typename Tm, auto V, typename... Ts, auto W, typename... Us> struct t_variation_of<Tm<V, Ts...>, Tm<W, Us...>> : constant<true> {};
template<typename T, typename U> concept variation_of = t_variation_of<T, U>::value;

template<trivially_copyable T> constexpr auto bitcast = []<trivially_copyable U>(const U& Val) noexcept requires (sizeof(T) == sizeof(U)) { return __builtin_bit_cast(T, Val); };
inline constexpr auto natcast = []<trivially_copyable T>(const T& Val) ywlib_wrap_auto(bitcast<nat_type<sizeof(T)>>(Val));
inline constexpr auto popcount = [](const integral auto& Value) ywlib_wrap_auto(std::popcount(natcast(Value)));
inline constexpr auto countlz = [](const integral auto& Value) ywlib_wrap_auto(std::countl_zero(natcast(Value)));
inline constexpr auto countrz = [](const integral auto& Value) ywlib_wrap_auto(std::countr_zero(natcast(Value)));
inline constexpr auto bitfloor = [](const integral auto& Value) ywlib_wrap_auto(std::bit_floor(natcast(Value)));
inline constexpr auto bitceil = [](const integral auto& Value) ywlib_wrap_auto(std::bit_ceil(natcast(Value)));
} // namespace yw
