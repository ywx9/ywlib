/**
 * @file ywlib@core.h
 * @brief YWLIBのコア機能を定義したヘッダファイル
 **********************************************************************************************************************/

#pragma once

#include <algorithm>
#include <bit>
#include <compare>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <numbers>
#include <tuple>
#include <vector>

#undef max
#undef min

#define ywlib_wrapper(A) \
  noexcept(noexcept(A))->decltype(A) requires requires { A; } { return A; }

inline constexpr auto yw00 = 1627776;

namespace yw {

using cat1 = char;
using cat2 = wchar_t;
using uct1 = char8_t;
using uct2 = char16_t;
using uct4 = char32_t;
using int1 = signed char;
using int2 = short;
using int4 = int;
using int8 = long long;
using nat1 = unsigned char;
using nat2 = unsigned short;
using nat4 = unsigned int;
using nat8 = unsigned long long;
using fat4 = float;
using fat8 = double;
using intt = decltype((int*)0 - (int*)0);
using natt = decltype(sizeof(int));
using sl_t = signed long;
using ul_t = unsigned long;
using ld_t = long double;
using np_t = decltype(nullptr);

constexpr cat1 operator""_c1(nat8 a) noexcept { return static_cast<cat1>(a); }
constexpr cat2 operator""_c2(nat8 a) noexcept { return static_cast<cat2>(a); }
constexpr uct1 operator""_u1(nat8 a) noexcept { return static_cast<uct1>(a); }
constexpr uct2 operator""_u2(nat8 a) noexcept { return static_cast<uct2>(a); }
constexpr uct4 operator""_u4(nat8 a) noexcept { return static_cast<uct4>(a); }
constexpr int1 operator""_i1(nat8 a) noexcept { return static_cast<int1>(a); }
constexpr int2 operator""_i2(nat8 a) noexcept { return static_cast<int2>(a); }
constexpr int4 operator""_i4(nat8 a) noexcept { return static_cast<int4>(a); }
constexpr int8 operator""_i8(nat8 a) noexcept { return static_cast<int8>(a); }
constexpr nat1 operator""_n1(nat8 a) noexcept { return static_cast<nat1>(a); }
constexpr nat2 operator""_n2(nat8 a) noexcept { return static_cast<nat2>(a); }
constexpr nat4 operator""_n4(nat8 a) noexcept { return static_cast<nat4>(a); }
constexpr nat8 operator""_n8(nat8 a) noexcept { return static_cast<nat8>(a); }
constexpr fat4 operator""_f4(nat8 a) noexcept { return static_cast<fat4>(a); }
constexpr fat8 operator""_f8(nat8 a) noexcept { return static_cast<fat8>(a); }
constexpr fat4 operator""_f4(ld_t a) noexcept { return static_cast<fat4>(a); }
constexpr fat8 operator""_f8(ld_t a) noexcept { return static_cast<fat8>(a); }
constexpr intt operator""_ii(natt a) noexcept { return static_cast<intt>(a); }
constexpr natt operator""_nn(nat8 a) noexcept { return static_cast<natt>(a); }

inline constexpr natt npos(-1);

struct none {
  constexpr none() noexcept = default;
  constexpr none(auto&&...) noexcept {}
  constexpr none& operator=(auto&&) noexcept { return *this; }
  explicit constexpr operator bool() const noexcept { return false; }
  friend constexpr bool operator==(none, none) noexcept { return false; }
  friend constexpr auto operator<=>(none, none) noexcept { return std::partial_ordering::unordered; }
  friend constexpr none operator+(none) noexcept { return {}; }
  friend constexpr none operator-(none) noexcept { return {}; }
  friend constexpr none operator+(none, none) noexcept { return {}; }
  friend constexpr none operator-(none, none) noexcept { return {}; }
  friend constexpr none operator*(none, none) noexcept { return {}; }
  friend constexpr none operator/(none, none) noexcept { return {}; }
  friend constexpr none& operator+=(none& a, none) noexcept { return a; }
  friend constexpr none& operator-=(none& a, none) noexcept { return a; }
  friend constexpr none& operator*=(none& a, none) noexcept { return a; }
  friend constexpr none& operator/=(none& a, none) noexcept { return a; }
};

template<typename T, auto V = none{}> struct trait {
  using type = T;
  static constexpr auto value = V;
  constexpr operator decltype(value)() const noexcept { return value; }
};


template<typename T> using type = typename trait<T>::type;
template<typename T> concept valid = requires { typename type<T>; };
template<typename T, typename... Ts> concept same_as = (std::same_as<T, Ts> && ...);
template<typename T, typename... Ts> concept included_in = (std::same_as<T, Ts> || ...);
template<typename T, typename... Ts> concept derived_from = (std::derived_from<T, Ts> && ...);
template<typename T, typename... Ts> concept castable_to = requires(T (&f)()) { ((static_cast<Ts>(f())), ...); };
template<typename T, typename... Ts> concept nt_castable_to = requires(T (&f)() noexcept) { { ((static_cast<Ts>(f())), ...) }noexcept; };
template<typename T, typename... Ts> concept convertible_to = castable_to<T, Ts...> && (std::convertible_to<T, Ts> && ...);
template<typename T, typename... Ts> concept nt_convertible_to = convertible_to<T> && nt_castable_to<T, Ts...> && (std::is_nothrow_convertible_v<T, Ts> && ...);
template<auto V, typename T = decltype(V)> requires convertible_to<decltype(V), T> struct constant : trait<T, static_cast<T>(V)> {};

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

namespace _ { // clang-format off
template<typename T> struct _memptr;
template<typename M, typename C> struct _memptr<M C::*> { using member_type = M; using class_type = C; };
template<typename M, typename C> struct _memptr<M C::* const> { using member_type = M; using class_type = C; };
template<typename M, typename C> struct _memptr<M C::* volatile> { using member_type = M; using class_type = C; };
template<typename M, typename C> struct _memptr<M C::* const volatile> { using member_type = M; using class_type = C; };
template<class T, template<class...> class Tm> struct _specialization_of : constant<false> {};
template<template<class...> class Tm, class... Ts> struct _specialization_of<Tm<Ts...>, Tm> : constant<true> {};
template<class T, class U> struct _variation_of : constant<false> {};
template<template<class...> class Tm, class... Ts, class... Us> struct _variation_of<Tm<Ts...>, Tm<Us...>> : constant<true> {};
template<template<auto...> class Tm, auto... Vs, auto... Ws> struct _variation_of<Tm<Vs...>, Tm<Ws...>> : constant<true> {};
template<template<class, auto...> class Tm, class T, class U, auto... Vs, auto... Ws> struct _variation_of<Tm<T, Vs...>, Tm<U, Ws...>> : constant<true> {};
template<template<auto, class...> class Tm, auto V, auto W, class... Ts, class... Us> struct _variation_of<Tm<V, Ts...>, Tm<W, Us...>> : constant<true> {};
template<typename T> struct _remove_all_pointers : trait<T> {};
template<typename T> struct _remove_all_pointers<T*> : _remove_all_pointers<T> {};
template<typename T> struct _remove_all_pointers<T* const> : _remove_all_pointers<T> {};
template<typename T> struct _remove_all_pointers<T* volatile> : _remove_all_pointers<T> {};
template<typename T> struct _remove_all_pointers<T* const volatile> : _remove_all_pointers<T> {};
template<typename T> struct _add_cv { using c = const T; using v = volatile T; using cv = const volatile T; };
template<typename T> struct _add_cv<T&> { using c = const T&; using v = volatile T&; using cv = const volatile T&; };
template<typename T> struct _add_cv<T&&> { using c = const T&&; using v = volatile T&&; using cv = const volatile T&&; };
template<typename T> struct _add_pointer : trait<T> {};
template<typename T> requires valid<T*> struct _add_pointer<T> : trait<T*> {};
template<typename T> requires valid<T*> struct _add_pointer<T&> : trait<T*&> {};
template<typename T> requires valid<T*> struct _add_pointer<T&&> : trait<T*&&> {};
template<typename T, natt N> struct _add_extent : trait<T> {};
template<typename T> requires valid<T[]> struct _add_extent<T, 0> : trait<T[]> {};
template<typename T> requires valid<T[]> struct _add_extent<T&, 0> : trait<T (&)[]> {};
template<typename T> requires valid<T[]> struct _add_extent<T&&, 0> : trait<T (&&)[]> {};
template<typename T, natt N> requires valid<T[N]> struct _add_extent<T, N> : trait<T[N]> {};
template<typename T, natt N> requires valid<T[N]> struct _add_extent<T&, N> : trait<T (&)[N]> {};
template<typename T, natt N> requires valid<T[N]> struct _add_extent<T&&, N> : trait<T (&&)[N]> {};
template<typename T, T I, typename... Ts> struct _type_switch : _type_switch<natt, I, Ts...> {};
template<bool B, typename Tt, typename Tf> struct _type_switch<bool, B, Tt, Tf> : std::conditional<B, Tt, Tf> {};
template<natt I, typename T, typename... Ts> struct _type_switch<natt, I, T, Ts...> : _type_switch<natt, I - 1, Ts...> {};
template<typename T, typename... Ts> struct _type_switch<natt, 0, T, Ts...> : trait<T> {};
template<typename... Ts> struct _common_type : trait<none> {};
template<typename... Ts> requires valid<std::common_reference_t<Ts...>> struct _common_type<Ts...> : trait<std::common_reference_t<Ts...>> {};
} // clang-format on

template<typename T> concept is_memptr = is_pointer<T> && std::is_member_pointer_v<T>;
template<is_memptr T> using member_type = typename _::_memptr<T>::member_type;
template<is_memptr T> using class_type = typename _::_memptr<T>::class_type;
template<typename T> concept is_member_function_pointer = is_memptr<T> && is_function<member_type<T>>;
template<typename T> concept is_member_object_pointer = is_memptr<T> && !is_member_function_pointer<T>;

template<typename T, template<typename...> typename Tm> concept specialization_of = _::_specialization_of<T, Tm>::value;
template<typename T, typename U> concept variation_of = _::_variation_of<T, U>::value;

template<typename T> concept is_enum = std::is_enum_v<T>;
template<typename T> concept is_class = std::is_class_v<T>;
template<typename T> concept is_union = std::is_union_v<T>;
template<typename T> concept is_abstract = is_class<T> && std::is_abstract_v<T>;
template<typename T> concept is_aggregate = is_class<T> && std::is_aggregate_v<T>;
template<typename T> concept is_empty = is_class<T> && std::is_empty_v<T>;
template<typename T> concept is_final = is_class<T> && std::is_final_v<T>;
template<typename T> concept is_polymorphic = is_class<T> && std::is_polymorphic_v<T>;
template<typename T> concept has_virtual_destructor = is_class<T> && std::has_virtual_destructor_v<T>;
template<typename T> concept trivial = std::is_trivially_copyable_v<T>;

template<typename T> using remove_cv = std::remove_cv_t<T>;
template<typename T> using remove_const = std::remove_const_t<T>;
template<typename T> using remove_volatile = std::remove_volatile_t<T>;
template<typename T> using remove_ref = std::remove_reference_t<T>;
template<typename T> using remove_cvref = std::remove_cvref_t<T>;
template<typename T> using remove_pointer = std::remove_pointer_t<T>;
template<typename T> using remove_all_pointers = typename _::_remove_all_pointers<T>::type;
template<typename T> using remove_extent = std::remove_extent_t<T>;
template<typename T> using remove_all_extents = std::remove_all_extents_t<T>;
template<typename T> using decay = std::decay_t<T>;

template<typename T> using add_cv = typename _::_add_cv<T>::cv;
template<typename T> using add_const = typename _::_add_cv<T>::c;
template<typename T> using add_volatile = typename _::_add_cv<T>::v;
template<typename T> using add_lvref = std::add_lvalue_reference_t<T>;
template<typename T> using add_rvref = std::add_rvalue_reference_t<remove_ref<T>>;
template<typename T> using add_fwref = std::add_rvalue_reference_t<T>;
template<typename T> using add_pointer = typename _::_add_pointer<T>::type;
template<typename T, natt N> using add_extent = typename _::_add_extent<T, N>::type;

template<typename T> concept is_void = same_as<remove_cv<T>, void>;
template<typename T> concept is_bool = same_as<remove_cv<T>, bool>;
template<typename T> concept is_none = same_as<remove_cv<T>, none>;
template<typename T> concept character = included_in<remove_cv<T>, cat1, cat2, uct1, uct2, uct4>;
template<typename T> concept integral = is_bool<T> || character<T> || std::integral<T>;
template<typename T> concept unsigned_integral = integral<T> && std::unsigned_integral<T>;
template<typename T> concept arithmetic = integral<T> || std::floating_point<T>;

inline constexpr auto addressof = []<is_lvref T>(T&& Reference) noexcept { return std::addressof(Reference); };
inline constexpr auto asconst = []<typename T>(T&& Reference) noexcept -> add_const<T&&> { return Reference; };
inline constexpr auto mv = []<typename T>(T&& Reference) noexcept -> decltype(auto) { return static_cast<add_rvref<T>>(Reference); };
template<typename T> inline constexpr auto fwd = [](auto&& A) noexcept -> decltype(auto) { return static_cast<T&&>(A); };
template<typename T> inline constexpr auto declval = []() noexcept -> add_fwref<T> {};
template<trivial T> inline constexpr auto bitcast = [](const trivial auto& A) noexcept requires(sizeof(T) == sizeof(A)) { return std::bit_cast<T>(A); };
inline constexpr auto bitceil = [](const unsigned_integral auto& A) noexcept { return std::bit_ceil(A); };
inline constexpr auto bitfloor = [](const unsigned_integral auto& A) noexcept { return std::bit_floor(A); };
inline constexpr auto popcount = [](const unsigned_integral auto& A) noexcept -> nat4 { return std::popcount(A); };

template<convertible_to<natt> auto I, typename... Ts>
requires((is_bool<decltype(I)> && sizeof...(Ts) == 2) ||
         (!is_bool<decltype(I)> && static_cast<natt>(I) < sizeof...(Ts)))
using type_switch = typename _::_type_switch<decltype(I), I, Ts...>::type;

template<convertible_to<natt> auto I, auto... Vs>
requires((is_bool<decltype(I)> && sizeof...(Vs) == 2) ||
         (!is_bool<decltype(I)> && static_cast<natt>(I) < sizeof...(Vs)))
inline constexpr auto value_switch = type_switch<I, constant<Vs>...>::value;

// clang-format off

template<typename F, typename... As> concept invocable = std::invocable<F, As...>;
template<typename F, typename... As> concept nt_invocable = invocable<F, As...> && std::is_nothrow_invocable_v<F, As...>;
template<typename F, typename... As> using invoke_result = std::invoke_result_t<F, As...>;
template<typename F, typename R, typename... As> concept invocable_r = invocable<F, As...> && convertible_to<invoke_result<F, As...>, R>;
template<typename F, typename R, typename... As> concept nt_invocable_r = nt_invocable<F, As...> && nt_convertible_to<invoke_result<F, As...>, R>;
template<typename F, typename... As> concept predicate = invocable_r<F, bool, As...>;

inline constexpr auto invoke = []<typename F, typename... As>(F&& f, As&&... as) noexcept(nt_invocable<F, As...>)
  -> invoke_result<F, As...> requires invocable<F, As...> { return std::invoke(fwd<F>(f), fwd<As>(as)...); };
template<typename R> inline constexpr auto invoke_r = []<typename F, typename... As>(F&& f, As&&... as) noexcept(nt_invocable_r<F, R, As...>)
  -> R requires invocable_r<F, R, As...> { return std::invoke(fwd<F>(f), fwd<As>(as)...); };

struct pass { template<typename T> constexpr T&& operator()(T&& a) const noexcept { return static_cast<T&&>(a); } };
template<typename... Fs> struct overload : public Fs... { using Fs::operator()...; };

/// 実体を隠蔽したオブジェクト。設定した関数を通してアクセスする
template<typename T, typename... Setters> class ghost {
  static constexpr overload<Setters...> func;
  T value;
public:
  template<convertible_to<T> U, typename... Fs> constexpr ghost(U&& Init, Fs&&...) : value(fwd<U>(Init)) {}
  template<typename... Ts> constexpr auto operator()(Ts&&... Args) ywlib_wrapper(void(func(value, fwd<Ts>(Args)...)));
  template<typename U> constexpr auto operator=(U&& Arg) ywlib_wrapper((func(value, fwd<U>(Arg)), *this));
  constexpr operator const T&() const noexcept { return value; }
};
template<typename T, typename... Fs> ghost(T&&, Fs&&...) -> ghost<T, Fs...>;

/// パラメータを選択する関数。`parameter_switch<I>(Args...)`
template<convertible_to<natt> auto I> inline constexpr auto parameter_switch =
  []<typename T, typename... Ts>(T&& t, Ts&&... ts) noexcept -> decltype(auto) requires(natt(I) <= sizeof...(Ts)) {
    static_assert(is_bool<decltype(I)> ? sizeof...(ts) == 1 : true);
    if constexpr (is_bool<decltype(I)>) return parameter_switch<natt(!I)>(fwd<T>(t), fwd<Ts>(ts)...);
    else if constexpr (same_as<decltype(I), natt>) {
      if constexpr (I == 0) return fwd<T>(t);
      else return parameter_switch<I - 1>(fwd<Ts>(ts)...);
    } else return parameter_switch<natt(I)>(fwd<T>(t), fwd<Ts>(ts)...);
  };

/// テンプレート引数に含まれる`true`の数を求めるメタ関数
template<bool... Bs> inline constexpr natt counts = 0;
template<bool B, bool... Bs> inline constexpr natt counts<B, Bs...> = B ? 1 + counts<Bs...> : counts<Bs...>;

/// テンプレート引数に含まれる最初の`true`のインデックスを求めるメタ関数
template<bool... Bs> inline constexpr natt inspects = 0;
template<bool B, bool... Bs> inline constexpr natt inspects<B, Bs...> = B ? 0 : 1 + inspects<Bs...>;

template<typename T, typename Src> using copy_const = type_switch<is_const<Src>, add_const<T>, T>;
template<typename T, typename Src> using copy_volatile = type_switch<is_volatile<Src>, add_volatile<T>, T>;
template<typename T, typename Src> using copy_cv = copy_const<copy_volatile<T, Src>, Src>;
template<typename T, typename Src> using copy_reference = type_switch<inspects<is_lvref<Src>, is_rvref<Src>>, add_lvref<T>, add_rvref<T>, remove_ref<T>>;
template<typename T, typename Src> using copy_cvref = copy_reference<copy_cv<T, remove_ref<Src>>, Src>;
template<typename T, typename Src> using copy_pointer = type_switch<is_pointer<T>, copy_cv<add_pointer<T>, Src>, T>;
template<typename T, typename Src> using copy_extent = type_switch<is_array<T>, copy_cv<add_extent<T, std::extent_v<Src>>, Src>, T>;

template<typename... Ts> using common_type = typename _::_common_type<Ts...>::type;
template<typename... Ts> concept common_with = !is_none<common_type<Ts...>> && (std::common_reference_with<common_type<Ts...>, Ts> && ...);

template<typename T> concept destructible = std::is_destructible_v<T>;
template<typename T> concept nt_destructible = destructible<T> && std::destructible<T>;
template<typename T, typename A> concept assignable = std::is_assignable_v<T, A>;
template<typename T, typename A> concept nt_assignable = assignable<T, A> && std::is_nothrow_assignable_v<T, A>;
template<typename T, typename... As> concept constructible = requires { T{declval<As>()...}; };
template<typename T, typename... As> concept nt_constructible = constructible<T, As...>&& noexcept(T{declval<As>()...});

inline constexpr auto exchange = []<typename T, typename U = T>(T & A, U&& B) noexcept(nt_constructible<T, T> && nt_assignable<T&, U>)
                                   requires constructible<T, T> && assignable<T&, U> { auto a = mv(A); return A = fwd<U>(B), mv(a); };

// clang-format on

inline constexpr auto nt = [](const auto& A) noexcept(noexcept(!A)) -> convertible_to<bool> auto { return !A; };
inline constexpr auto eq = [](const auto& A, const auto& B) noexcept(noexcept(A == B)) -> convertible_to<bool> auto { return A == B; };
inline constexpr auto ne = [](const auto& A, const auto& B) noexcept(noexcept(A != B)) -> convertible_to<bool> auto { return A != B; };
inline constexpr auto lt = [](const auto& A, const auto& B) noexcept(noexcept(A < B, 1 > 0)) -> convertible_to<bool> auto { return A < B; };
inline constexpr auto gt = [](const auto& A, const auto& B) noexcept(noexcept(A > B)) -> convertible_to<bool> auto { return A > B; };
inline constexpr auto le = [](const auto& A, const auto& B) noexcept(noexcept(A <= B, 1 > 0)) -> convertible_to<bool> auto { return A <= B; };
inline constexpr auto ge = [](const auto& A, const auto& B) noexcept(noexcept(A >= B)) -> convertible_to<bool> auto { return A >= B; };
inline constexpr auto neg = []<typename T>(const T& A) noexcept(noexcept(-A)) -> same_as<T> auto { return -A; };
inline constexpr auto add = []<typename T, typename U>(const T& A, const U& B) noexcept(noexcept(A + B)) -> common_type<T, U> { return A + B; };
inline constexpr auto sub = []<typename T, typename U>(const T& A, const U& B) noexcept(noexcept(A - B)) -> common_type<T, U> { return A - B; };
inline constexpr auto mul = []<typename T, typename U>(const T& A, const U& B) noexcept(noexcept(A * B)) -> common_type<T, U> { return A * B; };
inline constexpr auto div = []<typename T, typename U>(const T& A, const U& B) noexcept(noexcept(A / B)) -> common_type<T, U> { return A / B; };

struct t_min {
  constexpr none operator()() const { return {}; }
  template<typename T> constexpr T operator()(T&& t) const { return t; }
  template<typename T0, typename T1, typename... Ts> constexpr auto operator()(T0&& t0, T1&& t1, Ts&&... ts) const -> common_type<T0, T1, Ts...>
    requires convertible_to<decltype(t1 < t0), bool> && (convertible_to<decltype(lt(ts, declval<common_type<T0, T1, Ts...>>())), bool> && ...)
  { return this->operator()(common_type<T0, T1, Ts...>(t1 < t0 ? t1 : t0), ts...); }
};

struct t_max {
  constexpr none operator()() const { return {}; }
  template<typename T> constexpr T operator()(T&& t) const { return t; }
  template<typename T0, typename T1, typename... Ts> constexpr auto operator()(T0&& t0, T1&& t1, Ts&&... ts) const -> common_type<T0, T1, Ts...>
    requires convertible_to<decltype(t1 > t0), bool> && (convertible_to<decltype(lt(declval<common_type<T0, T1, Ts...>>(), ts)), bool> && ...)
  { return this->operator()(common_type<T0, T1, Ts...>(t0 < t1 ? t1 : t0), ts...); }
};

inline constexpr t_min min;
inline constexpr t_max max;

namespace _::_get { // clang-format off
template<natt I> void get(auto&&) = delete;
template<natt I, typename T> inline constexpr auto pattern = []() -> int4 {
  if constexpr (is_bounded_array<remove_ref<T>> && I < std::extent_v<remove_ref<T>>) return 4;
  else if constexpr (requires { get<I>(declval<T>()); }) return 1 | noexcept(get<I>(declval<T>())) * 4;
  else if constexpr (requires { declval<T>().template get<I>(); }) return 2 | noexcept(declval<T>().template get<I>()) * 4;
  else return 8; }();
template<natt I, typename T, natt P = pattern<I, T>> requires(P < 8)
constexpr decltype(auto) call(T&& A) noexcept(bool(P & 4)) {
  if constexpr ((P & 3) == 0) return static_cast<T&&>(A)[I];
  else if constexpr ((P & 3) == 1) return get<I>(static_cast<T&&>(A));
  else if constexpr ((P & 3) == 2) return static_cast<T&&>(A).template get<I>(); }
} // clang-format on

template<natt I> inline constexpr auto get = []<typename T>(T&& Object) ywlib_wrapper(_::_get::call<I>(static_cast<T&&>(Object)));
template<typename T, natt I> concept gettable = requires { get<I>(declval<T>()); };
template<typename T, natt I> concept nt_gettable = requires { requires gettable<T, I>; requires noexcept(get<I>(declval<T>())); };
template<typename T, natt I> requires gettable<T, I> using element_t = decltype(get<I>(declval<T>()));

template<auto... Vs> struct sequence;
template<typename... Ts> struct typepack;
template<typename... Ts> struct list;

namespace _ { // clang-format off
template<typename T, typename U = remove_ref<T>> inline constexpr natt _extent =
  type_switch<inspects<is_array<U>, requires { std::tuple_size<U>::value; }>, std::extent<U>, std::tuple_size<U>, constant<0>>::value;
template<typename S, typename T = none> struct _to_sequence;
template<template<auto...> typename Tm, typename T, auto... Vs> struct _to_sequence<Tm<Vs...>, T> : trait<sequence<static_cast<T>(Vs)...>> {};
template<template<typename, auto...> typename Tm, typename T, typename U, auto... Vs> struct _to_sequence<Tm<U, Vs...>, T> : trait<sequence<static_cast<T>(Vs)...>> {};
template<template<auto...> typename Tm, auto... Vs> struct _to_sequence<Tm<Vs...>, none> : trait<sequence<Vs...>> {};
template<template<typename, auto...> typename Tm, typename U, auto... Vs> struct _to_sequence<Tm<U, Vs...>, none> : trait<sequence<Vs...>> {};
template<typename S, typename T, natt N> struct _indices_for : constant<false> {};
template<natt... Is, typename T, natt N> struct _indices_for<sequence<Is...>, T, N> : constant<(lt(Is, N) && ...)> {};
template<natt End, natt Begin = 0, invocable<natt> auto Proj = pass{}, natt... Vs> struct _make_sequence : _make_sequence<End, Begin + 1, Proj, Vs..., Proj(Begin)> {};
template<natt End, auto Proj, natt... Vs> struct _make_sequence<End, End, Proj, Vs...> : trait<sequence<Vs...>> {};
template<natt I, natt N, typename S, typename... T> struct _comb_indices;
template<natt I, natt N, bool... Bs, natt... Is> struct _comb_indices<I, N, sequence<Bs...>, sequence<Is...>> : _comb_indices<I + 1, N, sequence<Bs...>, type_switch<value_switch<I, Bs...>, sequence<Is..., I>, sequence<Is...>>> {};
template<natt N, bool... Bs, natt... Is> struct _comb_indices<N, N, sequence<Bs...>, sequence<Is...>> : trait<sequence<Is...>> {};
template<typename T, typename S> struct _to_typepack;
template<typename T, natt... Is> struct _to_typepack<T, sequence<Is...>> : trait<typepack<element_t<T, Is>...>> {};

} // clang-format on

template<typename T> inline constexpr natt extent = _::_extent<T>;
template<typename T> concept tuple = extent<T> != 0;

/// Checks if all `Tps` have the same extent.
template<typename... Tps> concept same_size_tuples = requires {
  requires(tuple<Tps> && ...);
  requires((extent<Tps> == extent<type_switch<0, Tps...>>) && ...);
};

template<typename S, typename T = none> using to_sequence = typename _::_to_sequence<S, T>::type;
template<typename S, typename T = none> concept sequence_of = variation_of<to_sequence<S, T>, sequence<0>>;
template<typename S, typename T> concept indices_for = tuple<T> && _::_indices_for<to_sequence<S, natt>, T, extent<T>>::value;
template<natt End, natt Begin = 0, invocable<natt> auto Proj = pass{}> requires(Begin <= End) using make_sequence = typename _::_make_sequence<End, Begin, Proj>::type;
template<typename T> using make_indices_for = make_sequence<extent<T>>;
template<sequence_of<bool> S> using comb_indices = typename _::_comb_indices<0, extent<to_sequence<S, bool>>, to_sequence<S, bool>, sequence<>>::type;
template<tuple T> using to_typepack = typename _::_to_typepack<T, make_indices_for<T>>::type;

template<auto... Vs> struct sequence {
private:
  template<typename S> struct _append : _append<to_sequence<S, none>> {};
  template<auto... Ws> struct _append<sequence<Ws...>> : trait<sequence<Vs..., Ws...>> {};
  template<> struct _append<none> : trait<none> {};
  template<typename S> struct _pickup : _pickup<to_sequence<S, none>> {};
  template<natt... Is> struct _pickup<sequence<Is...>> : trait<sequence<value_switch<Is, Vs...>...>> {};
  template<> struct _pickup<none> : trait<none> {};
public:
  static constexpr natt size = sizeof...(Vs);
  template<natt I> requires(lt(I, size)) static constexpr auto at = value_switch<I, Vs...>;
  template<natt I> requires(lt(I, size)) using type_at = type_switch<I, decltype(Vs)...>;
  template<sequence_of S> using append = typename _append<to_sequence<S>>::type;
  template<indices_for<sequence> S> using pickup = typename _pickup<to_sequence<S, natt>>::type;
  template<natt N> requires(N < size) using fore = pickup<make_sequence<N>>;
  template<natt N> requires(N < size) using back = pickup<make_sequence<size, size - N>>;
  template<natt I, sequence_of S> requires(I < size) using insert =
    typename fore<I>::template append<S>::template append<back<size - I>>;
  template<template<auto...> typename Tm> using expand = Tm<Vs...>;
  template<natt I> requires(I < size) constexpr const auto&& get() const noexcept { return mv(at<I>); }
};

namespace _ { // clang-format off
template<typename T, typename U, typename Sq> struct _tuple_for : constant<false> {};
template<typename T, typename U, natt... Is> struct _tuple_for<T, U, sequence<Is...>> : constant<(convertible_to<element_t<T, Is>, U> && ...)> {};
template<typename T, typename U, typename Sq> struct _nt_tuple_for : constant<false> {};
template<typename T, typename U, natt... Is> struct _nt_tuple_for<T, U, sequence<Is...>>
  : constant<(nt_gettable<T, Is> && ...) && (nt_convertible_to<element_t<T, Is>, U> && ...)> {};
} // clang-format on

/// Checks if all elements of `Tp` can be converted to `U`.
template<typename Tp, typename U> concept tuple_for = requires {
  requires tuple<Tp>;
  requires _::_tuple_for<Tp, U, make_indices_for<Tp>>::value;
};

/// Checks if all elements of `Tp` can be converted to `U` without throwing exceptions.
template<typename Tp, typename U> concept nt_tuple_for = requires {
  requires tuple_for<Tp, U>;
  requires _::_nt_tuple_for<Tp, U, make_indices_for<Tp>>::value;
};

template<typename... Ts> struct typepack {
private:
  template<typename T, typename U> struct t_append {};
  template<typename... Us, typename... Vs> struct t_append<typepack<Us...>, typepack<Vs...>> : trait<typepack<Us..., Vs...>> {};
  template<typename T, typename S> struct t_pickup {};
  template<typename... Us, natt... Is> struct t_pickup<typepack<Us...>, sequence<Is...>> : trait<typepack<type_switch<Is, Us...>...>> {};
public:
  static constexpr natt size = sizeof...(Ts);
  using common = common_type<Ts...>;
  template<natt I> requires(I < size) using at = type_switch<I, Ts...>;
  template<tuple T> using append = typename t_append<typepack, to_typepack<T>>::type;
  template<indices_for<typepack> S> using pickup = typename t_pickup<typepack, to_sequence<S, natt>>::type;
  template<natt N> requires(N <= size) using fore = pickup<make_sequence<N>>;
  template<natt N> requires(N <= size) using back = pickup<make_sequence<size, size - N>>;
  template<natt I, tuple T> requires(I < size) using insert = typename fore<I>::template append<T>::template append<back<size - I>>;
  template<template<typename...> typename Tm> using expand = Tm<Ts...>;
  template<natt I> requires(I < size) constexpr at<I> get() const noexcept;
};

template<typename... Ts> struct list : typepack<Ts...>::template fore<sizeof...(Ts) - 1>::template expand<list> {
  using base = typepack<Ts...>::template fore<sizeof...(Ts) - 1>::template expand<list>;
  static constexpr natt size = sizeof...(Ts);
  using last_type = type_switch<size - 1, Ts...>;
  last_type last;
  template<natt I> constexpr auto get() & noexcept -> type_switch<I, Ts...>& {
    if constexpr (I == size - 1) return last;
    else return base::template get<I>();
  }
  template<natt I> constexpr auto get() const& noexcept -> add_const<type_switch<I, Ts...>&> {
    if constexpr (I == size - 1) return last;
    else return base::template get<I>();
  }
  template<natt I> constexpr auto get() && noexcept -> type_switch<I, Ts...>&& {
    if constexpr (I == size - 1) return fwd<type_switch<I, Ts...>>(last);
    else return mv(*this).base::template get<I>();
  }
  template<natt I> constexpr auto get() const&& noexcept -> add_const<type_switch<I, Ts...>&&> {
    if constexpr (I == size - 1) return fwd<add_const<type_switch<I, Ts...>>>(last);
    else return mv(*this).base::template get<I>();
  }
};

template<typename T1, typename T2, typename T3> struct list<T1, T2, T3> : list<T1, T2> {
  using base = list<T1, T2>;
  static constexpr natt size = 3;
  using third_type = T3;
  third_type third;
  template<natt I> constexpr auto get() & noexcept -> type_switch<I, T1, T2, T3>& {
    if constexpr (I == 2) return third;
    else return base::template get<I>();
  }
  template<natt I> constexpr auto get() const& noexcept -> add_const<type_switch<I, T1, T2, T3>&> {
    if constexpr (I == 2) return third;
    else return base::template get<I>();
  }
  template<natt I> constexpr auto get() && noexcept -> type_switch<I, T1, T2, T3>&& {
    if constexpr (I == 2) return fwd<type_switch<I, T1, T2, T3>>(third);
    else return mv(*this).base::template get<I>();
  }
  template<natt I> constexpr auto get() const&& noexcept -> add_const<type_switch<I, T1, T2, T3>&&> {
    if constexpr (I == 2) return fwd<add_const<type_switch<I, T1, T2, T3>>>(third);
    else return mv(*this).base::template get<I>();
  }
};

template<typename T1, typename T2> struct list<T1, T2> : public list<T1> {
  using base = list<T1>;
  static constexpr natt size = 2;
  using second_type = T2;
  second_type second;
  template<natt I> constexpr auto get() & noexcept -> type_switch<I, T1, T2>& {
    if constexpr (I == 1) return second;
    else return base::template get<I>();
  }
  template<natt I> constexpr auto get() const& noexcept -> add_const<type_switch<I, T1, T2>&> {
    if constexpr (I == 1) return second;
    else return base::template get<I>();
  }
  template<natt I> constexpr auto get() && noexcept -> type_switch<I, T1, T2>&& {
    if constexpr (I == 1) return fwd<type_switch<I, T1, T2>>(second);
    else return mv(*this).base::template get<I>();
  }
  template<natt I> constexpr auto get() const&& noexcept -> add_const<type_switch<I, T1, T2>&&> {
    if constexpr (I == 1) return fwd<add_const<type_switch<I, T1, T2>>>(second);
    else return mv(*this).base::template get<I>();
  }
};

template<typename T1> struct list<T1> {
  static constexpr natt size = 1;
  using first_type = T1;
  first_type first;
  template<natt I> requires(I == 0) constexpr first_type& get() & noexcept { return first; }
  template<natt I> requires(I == 0) constexpr add_const<first_type&> get() const& noexcept { return first; }
  template<natt I> requires(I == 0) constexpr first_type&& get() && noexcept { return fwd<first_type&&>(first); }
  template<natt I> requires(I == 0) constexpr add_const<first_type&&> get() const&& noexcept { return fwd<add_const<first_type&&>>(first); }
};

template<typename... Ts> list(Ts...) -> list<Ts...>;

template<> struct list<> {
private:
  template<typename T, typename U, natt... Is, natt... Js>
  static constexpr auto _list_concat(T&& t, U&& u, sequence<Is...>, sequence<Js...>) noexcept(noexcept(yw::list{get<Is>(fwd<T>(t))..., get<Js>(fwd<U>(u))...}))
    requires requires { yw::list{get<Is>(fwd<T>(t))..., get<Js>(fwd<U>(u))...}; } { return yw::list{get<Is>(fwd<T>(t))..., get<Js>(fwd<U>(u))...}; }
  template<typename T, typename U, typename V> struct _from_typepack {};
  template<typename... Ts, typename U, template<typename...> typename Tm, typename... Vs> struct _from_typepack<typepack<Ts...>, U, Tm<Vs...>> : trait<list<copy_cvref<Tm<Ts>, U>...>> {};
  template<typename... Ts, typename U, typename V> struct _from_typepack<typepack<Ts...>, U, V> : trait<list<copy_cvref<Ts, U>...>> {};
public:
  static constexpr natt size = 0;
  template<tuple T, tuple U> static constexpr auto concat(T&& Fore, U&& Back) ywlib_wrapper(_list_concat(fwd<T>(Fore), fwd<U>(Back), make_indices_for<T>{}, make_indices_for<U>{}));
  template<typename... Ts> static constexpr auto asref(Ts&&... Args) noexcept { return list<Ts&&...>{fwd<Ts>(Args)...}; }
  template<specialization_of<typepack> T, typename Template = none> using from_typepack = _from_typepack<T, Template, remove_cvref<Template>>::type;
};

// clang-format off

struct t_apply {
  template<natt... Is, typename F, typename T> constexpr auto operator()(sequence<Is...>, F&& f, T&& t) const ywlib_wrapper(invoke(fwd<F>(f), get<Is>(fwd<T>(t))...));
  template<typename S, typename F, typename T> constexpr auto operator()(S, F&& f, T&& t) const ywlib_wrapper((*this)(to_sequence<T, natt>{}, fwd<F>(f), fwd<T>(t)));
  template<typename F, typename T> constexpr auto operator()(F&& f, T&& t) const ywlib_wrapper((*this)(make_indices_for<T>{}, fwd<F>(f), fwd<T>(t)));
};

inline constexpr t_apply apply;
template<typename Fn, typename Tp, typename S = make_indices_for<Tp>>
concept applyable = requires { requires tuple<Tp>; requires indices_for<S, Tp>; apply(S{}, declval<Fn>(), declval<Tp>()); };
template<typename Fn, typename Tp, typename S = make_indices_for<Tp>> concept nt_applyable = requires { requires applyable<Fn, Tp, S>; { apply(S{}, declval<Fn>(), declval<Tp>()) } noexcept; };
template<typename Fn, typename Tp, typename S = make_indices_for<Tp>> requires applyable<Fn, Tp, S> using apply_result = decltype(apply(S{}, declval<Fn>(), declval<Tp>()));

struct t_vapply {
private:
  template<natt I, typename Fn, typename... Ts> static constexpr auto call(constant<I>, Fn& f, Ts&&... ts)
    noexcept((nt_gettable<Ts, I> && ...) && nt_invocable<Fn&, element_t<Ts, I>...>) -> decltype(auto)
  { if constexpr (is_void<invoke_result<Fn&, element_t<Ts, I>...>>) return invoke(f, get<I>(fwd<Ts>(ts)...)), none{}; else return invoke(f, get<I>(fwd<Ts>(ts)...)); }
public:
  template<natt... Is, typename Fn, tuple... Ts> constexpr auto operator()(sequence<Is...>, Fn&& f, Ts&&... ts) const
    noexcept((noexcept(call(constant<Is>{}, f, fwd<Ts>(ts)...)) && ...)) requires requires { ((call(constant<Is>{}, f, fwd<Ts>(ts)...)), ...); }
  { return list<add_fwref<decltype(call(constant<Is>{}, f, fwd<Ts>(ts)...))>...>{call(constant<Is>{}, f, fwd<Ts>(ts)...)...}; }
  template<sequence_of<natt> S, typename Fn, tuple... Ts> constexpr auto operator()(S, Fn&& f, Ts&&... ts) const ywlib_wrapper((*this)(to_sequence<S, natt>{}, f, fwd<Ts>(ts)...));
  template<typename Fn, tuple T, tuple... Ts> constexpr auto operator()(Fn&& f, T&& t, Ts&&... ts) const ywlib_wrapper((*this)(make_indices_for<T>{}, f, fwd<T>(t), fwd<Ts>(ts)...));
};

inline constexpr t_vapply vapply;
template<typename Fn, typename... Ts> concept vapplyable = requires { requires(tuple<Ts> && ...); vapply(declval<Fn>(), declval<Ts>()...); };
template<typename Fn, typename... Ts> concept nt_vapplyable = requires { requires vapplyable<Fn, Ts...>; { vapply(declval<Fn>(), declval<Ts>()...) } noexcept; };
template<typename Fn, typename... Ts> requires vapplyable<Fn, Ts...> using vapply_reresult = decltype(vapply(declval<Fn>(), declval<Ts>()...));

struct t_vassign {
  template<natt... Is, typename T, typename U> constexpr T&& operator()(sequence<Is...>, T&& t, U&& u) const
    noexcept((nt_gettable<T, Is> && ...) && (nt_gettable<U, Is> && ...) && (nt_assignable<element_t<T, Is>, element_t<U, Is>> && ...))
    requires (gettable<T, Is> && ...) && (gettable<U, Is> && ...) && (assignable<element_t<T, Is>, element_t<U, Is>> && ...)
  { return ((get<Is>(fwd<T>(t)) = get<Is>(fwd<U>(u))), ...), fwd<T>(t); }
  template<typename S, typename T, typename U> constexpr auto operator()(S, T&& t, U&& u) const ywlib_wrapper((*this)(to_sequence<S, natt>{}, fwd<T>(t), fwd<U>(u)));
  template<typename T, typename U> constexpr auto operator()(T&& t, U&& u) const ywlib_wrapper((*this)(make_indices_for<T>{}, fwd<T>(t), fwd<U>(u)));
};


inline constexpr t_vassign vassign;

/// Checks if `T` is vertically assignable from `U`.
template<typename T, typename U, typename S = make_indices_for<T>>
concept vassignable = requires {
  requires tuple<T>;
  requires tuple<U>;
  requires indices_for<S, T>;
  requires indices_for<S, U>;
  vassign(S{}, declval<T>(), declval<U>());
};

/// Checks if `T` is vertically assignable from `U` without throwing exceptions.
template<typename T, typename U, typename S = make_indices_for<T>>
concept nt_vassignable = requires {
  requires vassignable<T, U, S>;
  { vassign(S{}, declval<T>(), declval<U>()) } noexcept;
};

template<typename T> struct t_build {
  template<natt... Is, typename U> constexpr T operator()(sequence<Is...>, U&& t) const
    noexcept((nt_gettable<U, Is> && ...) && nt_constructible<T, element_t<U, Is>...>)
    requires requires { requires(gettable<U, Is> && ...); requires constructible<T, element_t<U, Is>...>; }
  { return T{get<Is>(fwd<U>(t))...}; }
  template<typename S, typename U> constexpr auto operator()(S, U&& t) const ywlib_wrapper((*this)(to_sequence<S, natt>{}, fwd<U>(t)));
  template<tuple U> constexpr auto operator()(U&& t) const ywlib_wrapper((*this)(make_indices_for<U>{}, fwd<U>(t)));
};

template<typename T> inline constexpr t_build<T> build;
template<typename T, typename Tp, typename S = make_indices_for<Tp>> concept buildable = requires { requires tuple<Tp>; requires indices_for<S, Tp>; build<T>(S{}, declval<Tp>()); };
template<typename T, typename Tp, typename S = make_indices_for<Tp>> concept nt_buildable = requires { requires buildable<T, Tp, S>; { build<T>(S{}, declval<Tp>()) } noexcept; };

template<typename T, sequence_of<natt> S = make_indices_for<T>, typename F = none> struct projector {
  using indices_type = to_sequence<S, natt>;
  static constexpr natt size = indices_type::size;
  add_fwref<T> ref;
  remove_ref<F> proj;
  constexpr projector(T&& Tuple, F Proj) noexcept(nt_constructible<add_fwref<T>, T> && nt_constructible<F, F>)
    requires constructible<add_fwref<T>, T> && constructible<F, F> : ref(fwd<T>(Tuple)), proj(fwd<F>(Proj)) {}
  constexpr projector(T&& Tuple, F Proj, sequence_of<natt> auto) noexcept(nt_constructible<add_fwref<T>, T> && nt_constructible<F, F>)
    requires constructible<add_fwref<T>, T> && constructible<F, F> : ref(fwd<T>(Tuple)), proj(fwd<F>(Proj)) {}
  template<natt I> auto get() const ywlib_wrapper(invoke(proj, yw::get<indices_type::template at<I>>(static_cast<add_fwref<T>>(ref))));
};

template<tuple T, typename S> struct projector<T, S, none> {
  using indices_type = to_sequence<S, natt>;
  static constexpr natt size = indices_type::size;
  add_fwref<T> ref;
  constexpr projector(T&& t) noexcept : ref{fwd<T>(t)} {}
  constexpr projector(T&& Tuple, indices_for<T> auto) noexcept : ref{fwd<T>(Tuple)} {}
  template<natt I> auto get() const ywlib_wrapper(yw::get<indices_type::template at<I>>(static_cast<add_fwref<T>>(ref)));
};

template<tuple T> projector(T&&) -> projector<T, make_indices_for<T>, none>;
template<tuple T, indices_for<T> S> projector(T&&, S) -> projector<T, S, none>;
template<tuple T, typename F> requires(!sequence_of<F, natt>) projector(T&&, F&&) -> projector<T, make_indices_for<T>, remove_ref<F>>;
template<tuple T, typename F, sequence_of<natt> S> requires(!sequence_of<F, natt>) projector(T&&, F&&, S) -> projector<T, remove_ref<F>, to_sequence<S, natt>>;

template<natt I, invocable_r<bool, natt> auto Cond, invocable_r<natt, natt> auto Incrementer = [](natt i) { return i + 1; }>
inline constexpr auto cfor = []<typename F, gettable<I>... Ts>(F&& Func, Ts&&... Tuples) requires invocable<F&, element_t<Ts&, I>...> {
  Func(yw::get<I>(Tuples)...);
  if constexpr (Cond(Incrementer(I))) {
    static_assert(((gettable<Ts&, Incrementer(I)>) && ...), "Not satisfied `gettable<Ts, I>`. Check `Cond` or `extent<Ts>`.");
    cfor<Incrementer(I), Cond, Incrementer>(fwd<F>(Func), fwd<Ts>(Tuples)...);
  }
};

// clang-format on

template<typename It> concept iterator = std::input_or_output_iterator<It>;
template<typename Se, typename It> concept sentinel_for = std::sentinel_for<Se, It>;
template<typename Se, typename It> concept sized_sentinel_for = std::sized_sentinel_for<Se, It>;
template<typename Rg> concept range = std::ranges::range<Rg>;
template<typename Rg> concept borrowed_range = std::ranges::borrowed_range<Rg>;
template<typename Rg> concept sized_range = std::ranges::sized_range<Rg>;
template<range Rg> using iterator_t = std::ranges::iterator_t<Rg>;
template<range Rg> using borrowed_iterator_t = std::ranges::borrowed_iterator_t<Rg>;
template<range Rg> using sentinel_t = std::ranges::sentinel_t<Rg>;
template<typename T> struct t_iter_value_t {};
template<iterator It> struct t_iter_value_t<It> : trait<std::iter_value_t<It>> {};
template<range Rg> struct t_iter_value_t<Rg> : t_iter_value_t<iterator_t<Rg>> {};
template<typename T> requires iterator<T> || range<T> using iter_value_t = typename t_iter_value_t<T>::type;
template<typename T> struct t_iter_difference_t {};
template<iterator It> struct t_iter_difference_t<It> : trait<std::iter_difference_t<It>> {};
template<range Rg> struct t_iter_difference_t<Rg> : t_iter_difference_t<iterator_t<Rg>> {};
template<typename T> requires iterator<T> || range<T> using iter_difference_t = typename t_iter_difference_t<T>::type;
template<typename T> struct t_iter_reference_t {};
template<iterator It> struct t_iter_reference_t<It> : trait<std::iter_reference_t<It>> {};
template<range Rg> struct t_iter_reference_t<Rg> : t_iter_reference_t<iterator_t<Rg>> {};
template<typename T> requires iterator<T> || range<T> using iter_reference_t = typename t_iter_reference_t<T>::type;
template<typename T> struct t_iter_rvref_t {};
template<iterator It> struct t_iter_rvref_t<It> : trait<std::iter_rvalue_reference_t<It>> {};
template<range Rg> struct t_iter_rvref_t<Rg> : t_iter_rvref_t<iterator_t<Rg>> {};
template<typename T> requires iterator<T> || range<T> using iter_rvref_t = typename t_iter_rvref_t<T>::type;
template<typename T> struct t_iter_common_t {};
template<iterator It> struct t_iter_common_t<It> : trait<std::iter_common_reference_t<It>> {};
template<range Rg> struct t_iter_common_t<Rg> : t_iter_common_t<iterator_t<Rg>> {};
template<typename T> requires iterator<T> || range<T> using iter_common_t = typename t_iter_common_t<T>::type;
template<typename It, typename T> concept output_iterator = std::output_iterator<It, T>;
template<typename It> concept input_iterator = std::input_iterator<It>;
template<typename It> concept fwd_iterator = std::forward_iterator<It>;
template<typename It> concept bid_iterator = std::bidirectional_iterator<It>;
template<typename It> concept rnd_iterator = std::random_access_iterator<It>;
template<typename It> concept cnt_iterator = std::contiguous_iterator<It>;
template<typename Rg, typename T> concept output_range = std::ranges::output_range<Rg, T>;
template<typename Rg> concept input_range = std::ranges::input_range<Rg>;
template<typename Rg> concept fwd_range = std::ranges::forward_range<Rg>;
template<typename Rg> concept bid_range = std::ranges::bidirectional_range<Rg>;
template<typename Rg> concept rnd_range = std::ranges::random_access_range<Rg>;
template<typename Rg> concept cnt_range = std::ranges::contiguous_range<Rg>;
template<typename It, typename In> concept iter_copyable = iterator<It> && iterator<In> && std::indirectly_copyable<In, It>;
template<typename It, typename In> concept iter_movable = iterator<It> && iterator<In> && std::indirectly_movable<In, It>;
template<typename Fn, typename It> concept iter_unary_invocable = iterator<It> && std::indirectly_unary_invocable<Fn, It>;
template<typename Fn, typename It> concept iter_unary_predicate = iterator<It> && std::indirect_unary_predicate<Fn, It>;
inline constexpr auto begin = []<range Rg>(Rg&& r) ywlib_wrapper(std::ranges::begin(fwd<Rg>(r)));
inline constexpr auto end = []<range Rg>(Rg&& r) ywlib_wrapper(std::ranges::end(fwd<Rg>(r)));
inline constexpr auto rbegin = []<range Rg>(Rg&& r) ywlib_wrapper(std::ranges::rbegin(fwd<Rg>(r)));
inline constexpr auto rend = []<range Rg>(Rg&& r) ywlib_wrapper(std::ranges::rend(fwd<Rg>(r)));
inline constexpr auto size = []<range Rg>(Rg&& r) ywlib_wrapper(std::ranges::size(fwd<Rg>(r)));
inline constexpr auto empty = []<range Rg>(Rg&& r) ywlib_wrapper(std::ranges::empty(fwd<Rg>(r)));
inline constexpr auto data = []<range Rg>(Rg&& r) ywlib_wrapper(std::ranges::data(fwd<Rg>(r)));
inline constexpr auto iter_move = []<iterator It>(It&& i) ywlib_wrapper(std::ranges::iter_move(fwd<It>(i)));
inline constexpr auto iter_swap = []<iterator It, iterator Jt>(It&& i, Jt&& j) ywlib_wrapper(std::ranges::iter_swap(fwd<It>(i), fwd<Jt>(j)));
template<typename It, typename Uy> concept iterator_for = iterator<It> && convertible_to<iter_reference_t<It>, Uy>;
template<typename It, typename Uy> concept iterator_of = iterator<It> && same_as<iter_value_t<It>, Uy>;
template<typename Rg, typename Uy> concept range_for = iterator_for<iterator_t<Rg>, Uy>;
template<typename Rg, typename Uy> concept range_of = iterator_of<iterator_t<Rg>, Uy>;
template<typename Rg, typename Uy> concept cnt_range_of = cnt_range<Rg> && range_of<Rg, Uy>;


/// 範囲のサイズを取得する
inline constexpr overload distance{
  []<iterator It, sentinel_for<It> Se>(It i, Se s) ywlib_wrapper(std::ranges::distance(mv(i), mv(s))),
  []<iterator It, sized_sentinel_for<decay<It>> Se>(It&& i, Se s) ywlib_wrapper(std::ranges::distance(mv(i), mv(s))),
  []<range Rg>(Rg&& r) noexcept(noexcept(std::ranges::distance(fwd<Rg>(r)))) { return std::ranges::distance(fwd<Rg>(r)); }};


/// Array class; `N == 0 ? std::vector<T> : static-array<T, N>`
template<typename T, natt N = 0> class array {
public:
  using value_type = T;
  T cpp_array[N]{};
  template<convertible_to<T> U> constexpr auto operator=(const U (&A)[N]) ywlib_wrapper((std::ranges::copy(A, cpp_array), *this));
  constexpr operator add_lvref<T[N]>() noexcept { return cpp_array; }
  constexpr operator add_lvref<const T[N]>() const noexcept { return cpp_array; }
  constexpr T& operator[](natt i) { return cpp_array[i]; }
  constexpr const T& operator[](natt i) const { return cpp_array[i]; }
  constexpr natt size() const noexcept { return N; }
  constexpr bool empty() const noexcept { return false; }
  constexpr T* data() noexcept { return cpp_array; }
  constexpr const T* data() const noexcept { return cpp_array; }
  constexpr T* begin() noexcept { return cpp_array; }
  constexpr const T* begin() const noexcept { return cpp_array; }
  constexpr T* end() noexcept { return cpp_array + N; }
  constexpr const T* end() const noexcept { return cpp_array + N; }
  constexpr T& front() noexcept { return *cpp_array; }
  constexpr const T& front() const noexcept { return *cpp_array; }
  constexpr T& back() noexcept { return cpp_array[N - 1]; }
  constexpr const T& back() const noexcept { return cpp_array[N - 1]; }
  template<natt I> requires(I < N) constexpr T& get() & noexcept { return cpp_array[I]; }
  template<natt I> requires(I < N) constexpr T&& get() && noexcept { return mv(cpp_array[I]); }
  template<natt I> requires(I < N) constexpr const T& get() const& noexcept { return cpp_array[I]; }
  template<natt I> requires(I < N) constexpr const T&& get() const&& noexcept { return mv(cpp_array[I]); }
};

/// Dynamic array class
template<typename T> class array<T, 0> : public std::vector<T> {
public:
  using std::vector<T>::vector;
};

template<typename T, convertible_to<T>... Ts> array(T, Ts...) -> array<T, 1 + sizeof...(Ts)>;
template<typename T> array(const std::vector<T>&) -> array<T, 0>;
template<typename T> array(natt, const T&) -> array<T, 0>;
template<typename It> array(It, It) -> array<iter_value_t<It>>;

/// String view class
template<character Ct> using string_view = std::basic_string_view<Ct>;
using stv1 = string_view<cat1>;
using stv2 = string_view<cat2>;

/// String class; `N == 0 ? std::basic_string<T> : static-string<T, N>`
template<character Ct, natt N = 0> class string {
public:
  using value_type = Ct;
  Ct cpp_array[N];
  constexpr string(const Ct (&String)[N]) noexcept { std::ranges::copy(String, cpp_array); }
  constexpr string(const Ct (&&String)[N]) noexcept { std::ranges::copy(String, cpp_array); }
  constexpr string& operator=(const Ct (&String)[N]) noexcept { return std::ranges::copy(String, cpp_array), *this; }
  constexpr operator std::basic_string_view<Ct>() const noexcept { return cpp_array; }
  constexpr operator std::basic_string<Ct>() const noexcept { return std::basic_string<Ct>(cpp_array, N - 1); }
  constexpr Ct& operator[](natt i) { return cpp_array[i]; }
  constexpr const Ct& operator[](natt i) const { return cpp_array[i]; }
  constexpr natt size() const noexcept { return N - 1; }
  constexpr bool empty() const noexcept { return N == 1; }
  constexpr Ct* data() noexcept { return cpp_array; }
  constexpr const Ct* data() const noexcept { return cpp_array; }
  constexpr Ct* begin() noexcept { return cpp_array; }
  constexpr const Ct* begin() const noexcept { return cpp_array; }
  constexpr Ct* end() noexcept { return cpp_array + (N - 1); }
  constexpr const Ct* end() const noexcept { return cpp_array + (N - 1); }
  constexpr bool operator==(const string_view<Ct>& s) const noexcept {
    return std::ranges::equal(begin(), end(), s.begin(), s.end());
  }
  constexpr auto operator<=>(const string_view<Ct>& s) const noexcept {
    return std::lexicographical_compare_three_way(begin(), end(), s.begin(), s.end());
  }
  template<typename Tr> friend std::basic_ostream<Ct, Tr>& operator<<(
    std::basic_ostream<Ct, Tr>& os, const string& s) { return os << s.cpp_array; }
};

/// Dynamic string class
template<character Ct> class string<Ct, 0> : public std::basic_string<Ct> {
public:
  constexpr string() noexcept = default;
  constexpr string(const string&) = default;
  constexpr string(string&&) noexcept = default;
  constexpr string(const std::basic_string<Ct>& s) : std::basic_string<Ct>(s) {}
  constexpr string(std::basic_string<Ct>&& s) noexcept : std::basic_string<Ct>(mv(s)) {}
  constexpr string& operator=(const string&) = default;
  constexpr string& operator=(string&&) noexcept = default;
  constexpr string& operator=(const std::basic_string<Ct>& s) { return *this = string(s); }
  constexpr string& operator=(std::basic_string<Ct>&& s) noexcept { return *this = string(mv(s)); }
  constexpr string(const Ct* s) : std::basic_string<Ct>(s) {}
  constexpr string(natt n, Ct c) : std::basic_string<Ct>(n, c) {}
  constexpr string& operator=(const Ct* s) { return *this = string(s); }
  template<iterator_of<Ct> It> constexpr string(It i, It s) : std::basic_string<Ct>(mv(i), mv(s)) {}
  template<range_of<Ct> Rg> constexpr string(Rg&& r) : std::basic_string<Ct>(yw::begin(r), yw::end(r)) {}
};
using str1 = string<cat1>;
using str2 = string<cat2>;

template<character Ct, natt N> string(const Ct (&)[N]) -> string<Ct, N>;
template<character Ct, natt N> string(const Ct (&&)[N]) -> string<Ct, N>;
template<character Ct> string(const std::basic_string<Ct>&) -> string<Ct, 0>;
template<character Ct> string(const std::basic_string_view<Ct>&) -> string<Ct, 0>;
template<character Ct> requires(!is_const<Ct>) string(Ct*) -> string<Ct, 0>;
template<character Ct> string(natt, Ct) -> string<Ct, 0>;
template<iterator It> string(It, It) -> string<iter_value_t<It>>;
template<range Rg> string(Rg&&) -> string<iter_value_t<Rg>>;

/// 文字列の長さを取得する
inline constexpr overload strlen{
  [](const character auto* const Str) { return std::char_traits<cat1>::length(Str); },
  []<character Ct, natt N>(const Ct (&Str)[N]) noexcept { return N - 1; },
  []<character Ct, natt N>(const string<Ct, N>& Str) { return Str.size(); }};


/// ソースコード位置情報
struct source {
  nat4 line, column;
  stv1 file, func;
  constexpr source(nat4 Line = __builtin_LINE(), nat4 Column = __builtin_COLUMN(),
                   stv1 File = __builtin_FILE(), stv1 Func = __builtin_FUNCTION()) noexcept
    : line(Line), column(Column), file(mv(File)), func(mv(Func)) {}
  template<typename Tr> friend std::basic_ostream<cat1, Tr>&
  operator<<(std::basic_ostream<cat1, Tr>& OS, const source& S) {
    return OS << std::format("file={},func={},line={},column={}", S.file, S.func, S.line, S.column);
  }
};


/// 例外クラス
class except : public std::exception {
public:
  explicit except(const std::string& s, source _ = {}) noexcept : except(s.data(), mv(_)){};
  explicit except(const cat1* s, source _ = {}) noexcept : std::exception(std::format("{}->{}\n", s, _).data()) {}
  explicit except(const std::exception& Base, source _ = {}) noexcept : except(Base.what(), mv(_)){};
};

/// File path class
class path : public std::filesystem::path {
public:
  using string_type = std::filesystem::path::string_type;
  path() noexcept = default;
  path(const path&) = default;
  path(path&&) noexcept = default;
  path(const std::filesystem::path& p) : std::filesystem::path(p) {}
  path(std::filesystem::path&& p) noexcept : std::filesystem::path(mv(p)) {}
  path(string_type&& s) : std::filesystem::path(mv(s)) {}
  path& operator=(const path&) = default;
  path& operator=(path&&) noexcept = default;
  path& operator=(const std::filesystem::path& p) { return *this = path(p); }
  path& operator=(std::filesystem::path&& p) noexcept { return *this = path(mv(p)); }
  path& operator=(string_type&& s) { return *this = path(mv(s)); }
  /// Creates from a string-like object.
  template<range Rg> requires included_in<iter_value_t<Rg>, cat1, cat2, uct1, uct2, uct4>
  path(Rg&& r) : std::filesystem::path(yw::begin(r), yw::end(r)) {}
  /// Creates from a null-terminated string.
  template<included_in<cat1, cat2, uct1, uct2, uct4> Ct> path(const Ct* s) : std::filesystem::path(s) {}
  /// Assigns from a string-like object.
  template<range Rg> requires included_in<iter_value_t<Rg>, cat1, cat2, uct1, uct2, uct4>
  path& operator=(Rg&& r) { return *this = path(fwd<Rg>(r)); }
  /// Assigns from a null-terminated string.
  template<included_in<cat1, cat2, uct1, uct2, uct4> Ct> path& operator=(const Ct* s) { return *this = path(s); }
};

namespace file {

/// ディレクトリ内のすべてのファイルパスを取得する
inline constexpr auto listup = [](const path& Directory, bool Recursive) -> array<path> {
  namespace fs = std::filesystem;
  array<path> a;
  if (Recursive)
    for (const fs::directory_entry& e : fs::recursive_directory_iterator(Directory)) a.emplace_back(e.path());
  else
    for (const fs::directory_entry& e : fs::directory_iterator(Directory)) a.emplace_back(e.path());
  return a;
};

/// 指定したファイルが存在することを確認する
inline constexpr auto exists = [](const path& p) -> bool { return std::filesystem::exists(p); };

/// 指定したファイルが通常のファイルであることを確認する
inline constexpr auto is_file = [](const path& p) -> bool { return std::filesystem::is_regular_file(p); };

/// 指定したファイルがディレクトリであることを確認する
inline constexpr auto is_directory = [](const path& p) -> bool { return std::filesystem::is_directory(p); };

/// 指定したファイルのサイズを確認する
inline constexpr auto size = [](const path& p) -> natt { return std::filesystem::file_size(p); };

/// 指定したファイルを文字列/配列として出力する
inline constexpr auto read =
  []<cnt_range Rg>(const path& p, Rg& Out)
    requires requires { Out.clear(), Out.resize(natt{}); } {
      if (!exists(p)) return Out.clear();
      std::ifstream ifs(p, std::ios_base::binary);
      auto n = size(p);
      Out.resize((n - 1) / sizeof(iter_value_t<Rg>) + 1);
      ifs.read(reinterpret_cast<cat1*>(yw::data(Out)), n);
    };

/// 指定したデータをファイルに出力する
inline constexpr overload write{
  [](const path& p, const void* a, natt n) {
    if (std::ofstream ofs(p, std::ios_base::binary); ofs) ofs.write(reinterpret_cast<const cat1*>(a), n);
  },
  []<cnt_iterator It, sized_sentinel_for<It> Se>(const path& p, It i, Se s) { write(p, &(*i), s - i); },
  []<cnt_range Rg>(const path& p, Rg&& a) { write(p, data(a), yw::size(a)); }};

/// 指定したファイルを削除する
inline constexpr auto remove = [](const path& Path) { return std::filesystem::remove(Path); };

}

/// 任意の数値を表す構造体
struct value {
  fat8 cpp_double{};
  constexpr value() noexcept = default;
  constexpr value(none) : cpp_double(bitcast<fat8>(0x7ff8000000000001)) {}
  constexpr value(const arithmetic auto VALUE) : cpp_double(fat8(VALUE)) {}
  constexpr value& operator=(none) noexcept { return cpp_double = bitcast<fat8>(0x7ff8000000000001), *this; }
  constexpr value& operator=(const fat8 FAT8) noexcept { return cpp_double = FAT8, *this; }
  constexpr operator fat8() const noexcept { return cpp_double; }
  template<arithmetic T> constexpr operator T() const noexcept { return T(cpp_double); }
  friend constexpr value& operator+=(value& L, const arithmetic auto R) noexcept { return L.cpp_double += R, L; }
  friend constexpr value& operator-=(value& L, const arithmetic auto R) noexcept { return L.cpp_double -= R, L; }
  friend constexpr value& operator*=(value& L, const arithmetic auto R) noexcept { return L.cpp_double *= R, L; }
  friend constexpr value& operator/=(value& L, const arithmetic auto R) noexcept { return L.cpp_double /= R, L; }
  static const value inf;
  static const value ninf;
  static const value snan;
  static const value qnan;
  static const value pi;
  static const value pi_2;
  static const value e;
  /// 10のべき乗
  template<intt I> static const value powerof10;
};
constexpr value operator""_vv(nat8 A) noexcept { return value(A); }
constexpr value operator""_vv(ld_t A) noexcept { return value(A); }

inline constexpr value value::inf = bitcast<fat8>(0x7ff0000000000000);
inline constexpr value value::ninf = bitcast<fat8>(0xfff0000000000000);
inline constexpr value value::snan = bitcast<fat8>(0x7ff0000000000001);
inline constexpr value value::qnan = bitcast<fat8>(0x7ff8000000000001);
inline constexpr value value::pi = std::numbers::pi;
inline constexpr value value::pi_2 = std::numbers::pi * 0.5;
inline constexpr value value::e = std::numbers::e;

template<intt I> inline constexpr value value::powerof10 = [] {
  fat8 v = 1;
  for (intt i = I < 0 ? -I : I; 0 < i; --i) v *= 10;
  return I < 0 ? 1 / v : v;
}();

/// 一つないし複数の型に静的に変換可能なオブジェクト
template<invocable... Fs> struct caster : public Fs... {
  template<typename T> static constexpr natt i = []<typename... Ts>(typepack<Ts...>) { return inspects<same_as<Ts, T>...>; }(typepack<invoke_result<Fs>...>{});
  template<typename T> static constexpr natt j = i<T> < sizeof...(Fs) ? i<T> : []<typename... Ts>(typepack<Ts...>) { return inspects<convertible_to<Ts, T>...>; }(typepack<invoke_result<Fs>...>{});
public:
  template<typename T> constexpr operator T() const noexcept(nt_convertible_to<invoke_result<type_switch<j<T>, Fs...>>, T>)
    requires(lt(j<T>, sizeof...(Fs))) { return type_switch<j<T>, Fs...>::operator()(); }
};

/// コンパイル時のみ`true`と評価される変数
inline constexpr caster is_cev{[]() noexcept { return std::is_constant_evaluated(); }};

/// Converts a string to a number.
template<arithmetic T> inline constexpr overload stov{
  [](str1 s) noexcept {
    for (; s.size(); s.erase(0, 1)) {
      try {
        if constexpr (same_as<remove_cv<T>, fat8>) return std::stod(s);
        else if constexpr (same_as<remove_cv<T>, fat4>) return std::stof(s);
        else if constexpr (same_as<remove_cv<T>, ld_t>) return std::stold(s);
        else if constexpr (same_as<remove_cv<T>, int8>) return std::stoll(s);
        else if constexpr (same_as<remove_cv<T>, int4>) return std::stoi(s);
        else if constexpr (same_as<remove_cv<T>, sl_t>) return std::stol(s);
        else if constexpr (included_in<remove_cv<T>, int2, int1>) return static_cast<T>(std::stoi(s));
        else if constexpr (same_as<remove_cv<T>, nat8>) return std::stoull(s);
        else if constexpr (same_as<remove_cv<T>, nat4>) return static_cast<T>(std::stoul(s));
        else if constexpr (same_as<remove_cv<T>, ul_t>) return std::stoul(s);
        else return static_cast<T>(std::stoul(s));
      } catch (const std::exception&) { continue; }
    }
    return T{};
  },
  [](str2 s) noexcept {
    for (; s.size(); s.erase(0, 1)) {
      try {
        if constexpr (same_as<remove_cv<T>, fat8>) return std::stod(s);
        else if constexpr (same_as<remove_cv<T>, fat4>) return std::stof(s);
        else if constexpr (same_as<remove_cv<T>, ld_t>) return std::stold(s);
        else if constexpr (same_as<remove_cv<T>, int8>) return std::stoll(s);
        else if constexpr (same_as<remove_cv<T>, int4>) return std::stoi(s);
        else if constexpr (same_as<remove_cv<T>, sl_t>) return std::stol(s);
        else if constexpr (included_in<remove_cv<T>, int2, int1>) return static_cast<T>(std::stoi(s));
        else if constexpr (same_as<remove_cv<T>, nat8>) return std::stoull(s);
        else if constexpr (same_as<remove_cv<T>, nat4>) return static_cast<T>(std::stoul(s));
        else if constexpr (same_as<remove_cv<T>, ul_t>) return std::stoul(s);
        else return static_cast<T>(std::stoul(s));
      } catch (const std::exception&) { continue; }
    }
    return T{};
  }};

/// Converts a number to a string.
template<included_in<cat1, cat2> Ct>
inline constexpr auto vtos = []<arithmetic T>(const T v) noexcept {
  if constexpr (same_as<Ct, cat1>) return std::to_string(v);
  else return std::to_wstring(v);
};
}

namespace std { // clang-format off
template<typename T> struct common_type<yw::none, T> : type_identity<yw::none> {};
template<typename T> struct common_type<T, yw::none> : type_identity<yw::none> {};
template<auto... Vs> struct tuple_size<yw::sequence<Vs...>> : integral_constant<size_t, sizeof...(Vs)> {};
template<size_t I, auto... Vs> struct tuple_element<I, yw::sequence<Vs...>> : type_identity<decltype(yw::value_switch<I, Vs...>)> {};
template<typename... Ts> struct tuple_size<yw::typepack<Ts...>> : integral_constant<size_t, sizeof...(Ts)> {};
template<size_t I, typename... Ts> struct tuple_element<I, yw::typepack<Ts...>> : type_identity<yw::type_switch<I, Ts...>> {};
template<typename... Ts> struct tuple_size<yw::list<Ts...>> : integral_constant<size_t, sizeof...(Ts)> {};
template<size_t I, typename... Ts> struct tuple_element<I, yw::list<Ts...>> : type_identity<yw::type_switch<I, Ts...>> {};
template<typename T, typename S, typename F> struct tuple_size<yw::projector<T, S, F>> : integral_constant<size_t, yw::to_sequence<S>::size> {};
template<size_t I, typename T, typename S, typename F> struct tuple_element<I, yw::projector<T, S, F>> : remove_cvref<yw::element_t<yw::projector<T, S, F>, I>> {};
template<typename T, size_t N> struct tuple_size<yw::array<T, N>> : integral_constant<size_t, N> {};
template<size_t I, typename T, size_t N> struct tuple_element<I, yw::array<T, N>> : type_identity<T> {};
template<typename C, size_t N> struct formatter<yw::string<C, N>, C> : formatter<basic_string_view<C>, C> {
  auto format(const yw::string<C, N>& s, yw::type_switch<same_as<C, wchar_t>, wformat_context, format_context>& ctx) const {
    return formatter<basic_string_view<C>, C>::format(basic_string_view<C>{s}, ctx); } };
template<> struct formatter<yw::source> : formatter<string> {
  auto format(const yw::source& s, format_context& ctx) const {
    return formatter<string>::format(std::format("source(file={},func={},line={},column={})", s.file, s.func, s.line, s.column), ctx); } };
template<yw::arithmetic T> struct common_type<T, yw::value> : type_identity<yw::value> {};
template<yw::arithmetic T> struct common_type<yw::value, T> : type_identity<yw::value> {};
} // clang-format on

namespace yw {

/// 絶対値を求める関数
inline constexpr auto abs = [](const arithmetic auto Value) noexcept { return std::abs(Value); };

/// 四捨五入する関数
inline constexpr overload round{
  [](const arithmetic auto Value) noexcept { return std::round(Value); },
  []<int4 I>(const arithmetic auto Value, constant<I> NumberOfDecimalPlaces) noexcept {
    return decltype(Value)(value::powerof10<-I> * std::round(Value * value::powerof10<I>));
  }};

/// ラジアンを求める関数
inline constexpr auto radian = [](const arithmetic auto Degree) noexcept {
  if constexpr (same_as<fat8, decltype(Degree + 0.4f)>) return Degree * fat8(value::pi / 180_vv);
  else return Degree * fat4(value::pi / 180_vv);
};

/// 角度を求める関数
inline constexpr auto degree = [](const arithmetic auto Radian) noexcept {
  if constexpr (same_as<fat8, decltype(Radian + 0.4f)>) return degree * fat8(180_vv / value::pi);
  else return Radian * fat4(180_vv / value::pi);
};
}
