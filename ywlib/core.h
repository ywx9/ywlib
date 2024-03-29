/// @file ywlib/core.h

#pragma once

#include <algorithm>
#include <bit>
#include <compare>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <numbers>
#include <ranges>
#include <vector>

#define ywlib_wrapper(A) \
  noexcept(noexcept(A))->decltype(A) requires requires { A; } { return A; }

namespace yw { // clang-format off

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
using intt = decltype((int*)0 - (int*)0);
using natt = decltype(sizeof(int));
using sl_t = signed long;
using ul_t = unsigned long;
using ld_t = long double;
using np_t = decltype(nullptr);
using po_t = std::partial_ordering;
using so_t = std::strong_ordering;
using wo_t = std::weak_ordering;

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

/// empty structure representing the absence of a value
/// @note `nt_constructible<none, AnyTypes...>`
/// @note `nt_assignable<none, AnyType>`
/// @note `nt_convertible_to<AnyType, none>`
/// @note `same_as<common_type<none, AnyType>, none>`
struct none {
  constexpr none() noexcept = default;
  constexpr none(auto&&...) noexcept {}
  constexpr none& operator=(auto&&) noexcept { return *this; }
  explicit constexpr operator bool() const noexcept { return false; }
  constexpr none operator()(auto&&...) const noexcept { return {}; }
  friend constexpr bool operator==(none, none) noexcept { return false; }
  friend constexpr auto operator<=>(none, none) noexcept { return po_t::unordered; }
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

/// empty structure holding a type and a constant value
template<typename T, auto V = none{}> struct trait {
  using type = T;
  static constexpr auto value = V;
  constexpr operator auto() const noexcept { return value; }
  constexpr auto operator()() const noexcept { return value; }
};

/// alias template representing the type
template<typename T> using type = typename trait<T>::type;

/// checks if `T` is a valid type
template<typename T> concept valid = requires { typename type<T>; };

/// checks if all of `Ts...` are the same type as `T`
template<typename T, typename... Ts> concept same_as = (std::same_as<T, Ts> && ...);

/// checks if any of `Ts...` are the same type as `T`
template<typename T, typename... Ts> concept included_in = (std::same_as<T, Ts> || ...);

/// checks if `T` is derived from all of `Ts...`
template<typename T, typename... Ts> concept derived_from = (std::derived_from<T, Ts> && ...);

/// checks if `T` is explicitly convertible to all of `Ts...`
template<typename T, typename... Ts> concept castable_to = requires(T (&f)()) { ((static_cast<Ts>(f())), ...); };

/// checks if `T` is explicitly convertible to all of `Ts...` without throwing exceptions
template<typename T, typename... Ts> concept nt_castable_to = requires(T (&f)() noexcept) {
  requires castable_to<T, Ts...>; { ((static_cast<Ts>(f())), ...) } noexcept; };

/// checks if `T` is implicitly convertible to all of `Ts...`
template<typename T, typename... Ts> concept convertible_to = requires {
  requires castable_to<T, Ts...>;
  requires(std::convertible_to<T, Ts> && ...); };

/// checks if `T` is implicitly convertible to all of `Ts...` without throwing exceptions
template<typename T, typename... Ts> concept nt_convertible_to = requires {
  requires nt_castable_to<T, Ts...>;
  requires convertible_to<T, Ts...>;
  requires (std::is_nothrow_convertible_v<T, Ts> && ...); };

/// empty structure representing a constant value
template<auto V, typename T = decltype(V)>
requires convertible_to<decltype(V), T>
struct constant : trait<T, static_cast<T>(V)> {};

/// checks if `T` is a `const`-qualified type
template<typename T> concept is_const = std::is_const_v<T>;

/// checks if `T` is a `volatile`-qualified type
template<typename T> concept is_volatile = std::is_volatile_v<T>;

/// checks if `T` is a cv-qualified type
template<typename T> concept is_cv = is_const<T> && is_volatile<T>;

/// checks if `T` is an lvalue reference
template<typename T> concept is_lvref = std::is_lvalue_reference_v<T>;

/// checks if `T` is an rvalue reference
template<typename T> concept is_rvref = std::is_rvalue_reference_v<T>;

/// checks if `T` is a reference
template<typename T> concept is_reference = is_lvref<T> || is_rvref<T>;

/// checks if `T` is a pointer
template<typename T> concept is_pointer = std::is_pointer_v<T>;

/// checks if `T` is a bounded array
template<typename T> concept is_bounded_array = std::is_bounded_array_v<T>;

/// checks if `T` is an unbounded array
template<typename T> concept is_unbounded_array = std::is_unbounded_array_v<T>;

/// checks if `T` is an array
template<typename T> concept is_array = is_bounded_array<T> || is_unbounded_array<T>;

/// checks if `T` is a function
template<typename T> concept is_function = std::is_function_v<T>;

/// alias template removing `const` and `volatile` from `T`
template<typename T> using remove_cv = std::remove_cv_t<T>;

/// alias template removing `const` from `T`
template<typename T> using remove_const = std::remove_const_t<T>;

/// alias template removing `volatile` from `T`
template<typename T> using remove_volatile = std::remove_volatile_t<T>;

/// alias template removing the reference from `T`
template<typename T> using remove_ref = std::remove_reference_t<T>;

/// alias template removing the const- and volatile-qualification and the reference from `T`
template<typename T> using remove_cvref = std::remove_cvref_t<T>;

/// alias template removing the pointer from `T`
template<typename T> using remove_pointer = std::remove_pointer_t<T>;

namespace _ {
template<typename T, typename U = remove_cv<T>> struct _remove_all_pointers : trait<T> {};
template<typename T, typename U> struct _remove_all_pointers<T, U*> : _remove_all_pointers<U> {};
template<typename T, typename U = remove_cv<T>> struct _memptr;
template<typename T, typename M, typename C> struct _memptr<T, M C::*> { using m = M; using c = C; };
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
}

/// alias template removing all pointers from `T`
template<typename T> using remove_all_pointers = typename _::_remove_all_pointers<T>::type;

/// alias template removing the first extent from `T`
template<typename T> using remove_extent = std::remove_extent_t<T>;

/// alias template removing all extents from `T`
template<typename T> using remove_all_extents = std::remove_all_extents_t<T>;

/// alias template decaying `T`
template<typename T> using decay = std::decay_t<T>;

/// checks if `T` is a member pointer
template<typename T> concept is_memptr = is_pointer<T> && std::is_member_pointer_v<T>;

/// alias template returning the member type of `T`
template<is_memptr T> using member_type = typename _::_memptr<T>::m;

/// alias template returning the class type of `T`
template<is_memptr T> using class_type = typename _::_memptr<T>::c;

/// checks if `T` is a member function pointer
template<typename T> concept is_member_function_pointer = is_memptr<T> && is_function<member_type<T>>;

/// checks if `T` is a member object pointer
template<typename T> concept is_member_object_pointer = is_memptr<T> && !is_member_function_pointer<T>;

/// checks if `T` is an enumeration
template<typename T> concept is_enum = std::is_enum_v<T>;

/// checks if `T` is a class
template<typename T> concept is_class = std::is_class_v<T>;

/// checks if `T` is a union
template<typename T> concept is_union = std::is_union_v<T>;

/// checks if `T` is an abstract class
template<typename T> concept is_abstract = is_class<T> && std::is_abstract_v<T>;

/// checks if `T` is an aggregate
template<typename T> concept is_aggregate = is_class<T> && std::is_aggregate_v<T>;

/// checks if `T` is an empty class
template<typename T> concept is_empty = is_class<T> && std::is_empty_v<T>;

/// checks if `T` is a final class
template<typename T> concept is_final = is_class<T> && std::is_final_v<T>;

/// checks if `T` is a polymorphic class
template<typename T> concept is_polymorphic = is_class<T> && std::is_polymorphic_v<T>;

/// checks if `T` has a virtual destructor
template<typename T> concept has_virtual_destructor = is_class<T> && std::has_virtual_destructor_v<T>;

/// checks if `T` is a trivial type
template<typename T> concept trivial = std::is_trivially_copyable_v<T>;

/// checks if `T` is `void`
template<typename T> concept is_void = same_as<remove_cv<T>, void>;

/// checks if `T` is `bool`
template<typename T> concept is_bool = same_as<remove_cv<T>, bool>;

/// checks if `T` is `none`
template<typename T> concept is_none = same_as<remove_cv<T>, none>;

/// checks if `T` is a character type
template<typename T> concept is_cat = included_in<remove_cv<T>, cat1, cat2>;

/// checks if `T` is a universal character type
template<typename T> concept is_uct = included_in<remove_cv<T>, uct1, uct2, uct4>;

/// checks if `T` is a signed integral type
template<typename T> concept is_int = included_in<remove_cv<T>, int1, int2, int4, int8, sl_t>;

/// checks if `T` is an unsigned integral type
template<typename T> concept is_nat = included_in<remove_cv<T>, nat1, nat2, nat4, nat8, ul_t>;

/// checks if `T` is a floating-point type
template<typename T> concept is_fat = included_in<remove_cv<T>, fat4, fat8, ld_t>;

/// checks if `T` is a character type
template<typename T> concept character = is_cat<T> || is_uct<T>;

/// checks if `T` is an integral type
template<typename T> concept integral = is_bool<T> || character<T> || is_int<T> || is_nat<T>;

/// checks if `T` is an unsigned integral type
template<typename T> concept unsigned_integral = integral<T> && std::unsigned_integral<T>;

/// checks if `T` is an arithmetic type
template<typename T> concept arithmetic = integral<T> || std::floating_point<T>;

/// alias template adding `const` and `volatile` to `T`
template<typename T> using add_cv = typename _::_add_cv<T>::cv;

/// alias template adding `const` to `T`
template<typename T> using add_const = typename _::_add_cv<T>::c;

/// alias template adding `volatile` to `T`
template<typename T> using add_volatile = typename _::_add_cv<T>::v;

/// alias template adding lvalue reference to `T`
template<typename T> using add_lvref = std::add_lvalue_reference_t<T>;

/// alias template adding rvalue reference to `T`
template<typename T> using add_rvref = std::add_rvalue_reference_t<remove_ref<T>>;

/// alias template adding forwarding reference to `T`
template<typename T> using add_fwref = std::add_rvalue_reference_t<T>;

/// alias template adding pointer to `T`
template<typename T> using add_pointer = typename _::_add_pointer<T>::type;

/// alias template adding extent to `T`
template<typename T, natt N> using add_extent = typename _::_add_extent<T, N>::type;

/// returns the address of `Ref`
inline constexpr auto addressof = []<is_lvref T>(T&& Ref) noexcept { return std::addressof(Ref); };

/// returns `Ref` as `const`
inline constexpr auto asconst = []<typename T>(T&& Ref) noexcept -> add_const<T&&> { return Ref; };

/// returns `Ref` as an rvalue reference
inline constexpr auto mv = []<typename T>(T&& Ref) noexcept
  -> decltype(auto) { return static_cast<add_rvref<T>>(Ref); };

/// returns `Ref` as a forwarding reference
template<typename T> inline constexpr auto fwd = [](auto&& Ref)
  noexcept -> decltype(auto) { return static_cast<T&&>(Ref); };

/// returns a reference to an object of type `T`
template<typename T> inline constexpr auto declval = []() noexcept -> add_fwref<T> {};

/// returns the bit representation of `Src` as `T`
template<trivial T> inline constexpr auto bitcast = [](const trivial auto& Src)
  noexcept requires(sizeof(T) == sizeof(Src)) { return std::bit_cast<T>(Src); };

/// returns the smallest power of 2 greater than or equal to `A`
inline constexpr auto bit_ceil = [](const unsigned_integral auto& A) noexcept { return std::bit_ceil(A); };

/// returns the largest power of 2 less than or equal to `A`
inline constexpr auto bit_floor = [](const unsigned_integral auto& A) noexcept { return std::bit_floor(A); };

/// returns the number of 1-bits in `A`
inline constexpr auto bit_count = [](const unsigned_integral auto& A) noexcept { return std::popcount(A); };

/// checks if `T` is destructible
template<typename T> concept destructible = std::is_destructible_v<T>;

/// checks if `T` is nothrow destructible
template<typename T> concept nt_destructible = destructible<T> && std::destructible<T>;

/// checks if `T` is assignable from `A`
template<typename T, typename A> concept assignable = std::is_assignable_v<T, A>;

/// checks if `T` is constructible from `As...`
template<typename T, typename... As> concept constructible = requires { T{declval<As>()...}; };

/// checks if `T` is nothrow assignable from `A`
template<typename T, typename A> concept nt_assignable = requires {
  requires assignable<T, A>;
  requires std::is_nothrow_assignable_v<T, A>; };

/// checks if `T` is nothrow constructible from `As...`
template<typename T, typename... As> concept nt_constructible = requires {
  requires constructible<T, As...>;
  { T{declval<As>()...} } noexcept; };

/// assigns `B` to `A` and returns the previous value of `A`
inline constexpr auto exchange =
  []<typename T, typename U = T>(T & A, U&& B) noexcept(nt_constructible<T, T> && nt_assignable<T&, U>)
  requires constructible<T, T> && assignable<T&, U> { if (auto a = mv(A); 1) return A = fwd<U>(B), mv(a); };

/// alias template representing the `I`-th type in `Ts...`
template<castable_to<natt> auto I, typename... Ts>
requires ((is_bool<decltype(I)> && sizeof...(Ts) == 2) ||
          (!is_bool<decltype(I)> && static_cast<natt>(I) < sizeof...(Ts)))
using type_switch = typename _::_type_switch<decltype(I), I, Ts...>::type;

/// variable template representing the `I`-th value in `Vs...`
template<castable_to<natt> auto I, auto... Vs>
requires ((is_bool<decltype(I)> && sizeof...(Vs) == 2) ||
          (!is_bool<decltype(I)> && static_cast<natt>(I) < sizeof...(Vs)))
inline constexpr auto value_switch = type_switch<I, constant<Vs>...>::value;

/// returns the `I`-th parameter in `Arg, Args...`
template<castable_to<natt> auto I> inline constexpr auto parameter_switch =
  []<typename A, typename... As>(A&& Arg, As&&... Args) noexcept -> decltype(auto)
    requires ((is_bool<decltype(I)> && sizeof...(Args) == 1) ||
              (!is_bool<decltype(I)> && static_cast<natt>(I) <= sizeof...(Args))) {
    if constexpr (is_bool<decltype(I)>) return parameter_switch<natt(!I)>(fwd<A>(Arg), fwd<Args>(Args)...);
    else if constexpr (same_as<decltype(I), natt>) {
      if constexpr (I == 0) return fwd<A>(Arg);
      else return parameter_switch<I - 1>(fwd<Args>(Args)...);
    } else return parameter_switch<natt(I)>(fwd<A>(Arg), fwd<Args>(Args)...); };

/// function object that returns its argument as is
struct pass { template<typename T> constexpr T&& operator()(T&& a) const noexcept { return static_cast<T&&>(a); } };

/// function object that overloads other function objects
template<typename... Fs> struct overload : public Fs... { using Fs::operator()...; };

/// variable template representing the number of `true` in `Bs...`
template<bool... Bs> inline constexpr natt counts = 0;
template<bool B, bool... Bs> inline constexpr natt counts<B, Bs...> = B ? 1 + counts<Bs...> : counts<Bs...>;

/// variable template representing the index of the first `true` in `Bs...`
template<bool... Bs> inline constexpr natt inspects = 0;
template<bool B, bool... Bs> inline constexpr natt inspects<B, Bs...> = B ? 0 : 1 + inspects<Bs...>;

/// alias template copying `const` from `Src` to `T`
template<typename T, typename Src> using copy_const = type_switch<is_const<Src>, add_const<T>, T>;

/// alias template copying `volatile` from `Src` to `T`
template<typename T, typename Src> using copy_volatile = type_switch<is_volatile<Src>, add_volatile<T>, T>;

/// alias template copying `const` and `volatile` from `Src` to `T
template<typename T, typename Src> using copy_cv = copy_const<copy_volatile<T, Src>, Src>;

namespace _ {
template<typename T, typename U> struct _copy_reference : trait<remove_ref<T>> {};
template<typename T, typename U> struct _copy_reference<T, U&> : trait<add_lvref<T>> {};
template<typename T, typename U> struct _copy_reference<T, U&&> : trait<add_rvref<T>> {};
template<typename T, typename U> struct _copy_extent : trait<T> {};
template<typename T, is_array U> struct _copy_extent<T, U> : trait<add_extent<T, std::extent_v<U>>> {};
template<typename T, typename U> struct _copy_pointer : trait<T> {};
template<typename T, is_pointer U> struct _copy_pointer<T, U> : trait<copy_cv<add_pointer<T>, U>> {};
template<typename T, typename U> struct _copy_all_extents : trait<T> {};
template<typename T, is_array U> struct _copy_all_extents<T, U> : _copy_extent<typename _copy_all_extents<T, remove_extent<U>>::type, U> {};
template<typename T, typename U> struct _copy_all_pointers : trait<T> {};
template<typename T, is_pointer U> struct _copy_all_pointers<T, U> : _copy_pointer<typename _copy_all_pointers<T, remove_pointer<U>>::type, U> {};
template<typename... Ts> struct _common_type : trait<none> {};
template<typename... Ts> requires valid<std::common_reference_t<Ts...>> struct _common_type<Ts...> : trait<std::common_reference_t<Ts...>> {};
template<class T, template<class...> class Tm> struct _specialization_of : constant<false> {};
template<template<class...> class Tm, class... Ts> struct _specialization_of<Tm<Ts...>, Tm> : constant<true> {};
template<class T, class U> struct _variation_of : constant<false> {};
template<template<class...> class Tm, class... Ts, class... Us> struct _variation_of<Tm<Ts...>, Tm<Us...>> : constant<true> {};
template<template<auto...> class Tm, auto... Vs, auto... Ws> struct _variation_of<Tm<Vs...>, Tm<Ws...>> : constant<true> {};
template<template<class, auto...> class Tm, class T, class U, auto... Vs, auto... Ws> struct _variation_of<Tm<T, Vs...>, Tm<U, Ws...>> : constant<true> {};
template<template<auto, class...> class Tm, auto V, auto W, class... Ts, class... Us> struct _variation_of<Tm<V, Ts...>, Tm<W, Us...>> : constant<true> {};
}

/// alias template copying reference from `Src` to `T`
template<typename T, typename Src> using copy_reference = typename _::_copy_reference<T, Src>::type;

/// alias template copying `const` and `volatile` from `Src` to `T
template<typename T, typename Src> using copy_cvref = copy_reference<copy_cv<T, remove_ref<Src>>, Src>;

/// alias template copying extent from `Src` to `T`
template<typename T, typename Src> using copy_extent = typename _::_copy_extent<T, Src>::type;

/// alias template copying pointer from `Src` to `T`
template<typename T, typename Src> using copy_pointer = type_switch<is_pointer<T>, copy_cv<add_pointer<T>, Src>, T>;

/// checks if `F` is invocable with `As...`
template<typename F, typename... As> concept invocable = std::invocable<F, As...>;

/// checks if `F` is nothrow invocable with `As...`
template<typename F, typename... As> concept nt_invocable =
  invocable<F, As...> && std::is_nothrow_invocable_v<F, As...>;

/// invokes `Func` with `Args...`
/// @note If `Func` returns `void`, returns `none{}` instead.
inline constexpr auto invoke = []<typename F, typename... As>(F&& Func, As&&... Args)
  noexcept(nt_invocable<F, As...>) -> decltype(auto) requires invocable<F, As...> {
  if constexpr (is_void<std::invoke_result<F, As...>>) return std::invoke(fwd<F>(Func), fwd<As>(Args)...), none{};
  else return std::invoke(fwd<F>(Func), fwd<As>(Args)...); };

/// returns the result of invoking `Func` with `Args...`.
template<typename F, typename... As> using invoke_result = decltype(invoke(declval<F>(), declval<As>()...));

/// checks if `F` is invocable with `As...` and returns `R`
template<typename F, typename R, typename... As> concept invocable_r =
  invocable<F, As...> && convertible_to<invoke_result<F, As...>, R>;

/// checks if `F` is nothrow invocable with `As...` and returns `R`
template<typename F, typename R, typename... As> concept nt_invocable_r =
  nt_invocable<F, As...> && nt_convertible_to<invoke_result<F, As...>, R>;

/// checks if `F` is a predicate
template<typename F, typename... As> concept predicate = invocable_r<F, bool, As...>;

/// invokes `f` with `as...` and returns the result as `R`
template<typename R> inline constexpr auto invoke_r = []<typename F, typename... As>(F&& f, As&&... as)
  noexcept(nt_invocable_r<F, R, As...>) -> R requires invocable_r<F, R, As...> {
  return static_cast<R>(invoke(fwd<F>(f), fwd<As>(as)...)); };

/// alias template representing the common type of `Ts...`
template<typename... Ts> using common_type = typename _::_common_type<Ts...>::type;

/// checks if there is a common type of `Ts...`
template<typename... Ts> concept common_with = requires {
  requires !is_none<common_type<Ts...>>;
  requires (std::common_reference_with<common_type<Ts...>, Ts> && ...); };

/// checks if `T` is a specialization of `Tm`
/// @note ex) `specialization_of<list<int, float>, list>` is `true`
template<typename T, template<typename...> typename Tm> concept specialization_of = _::_specialization_of<T, Tm>::value;

/// checks if `T` is a variation of `U`
/// @note ex) `variation_of<list<wchar_t, bool>, list<int, double>>` is `true`
template<typename T, typename U> concept variation_of = _::_variation_of<T, U>::value;

/// returns the logical negation of `A`
inline constexpr auto nt = [](const auto& A) noexcept(noexcept(!A)) -> convertible_to<bool> auto { return !A; };

/// returns `true` if `A` is equal to `B`
inline constexpr auto eq = [](const auto& A, const auto& B)
  noexcept(noexcept(A == B)) -> convertible_to<bool> auto { return A == B; };

/// returns `true` if `A` is not equal to `B`
inline constexpr auto ne = [](const auto& A, const auto& B)
  noexcept(noexcept(A != B)) -> convertible_to<bool> auto { return A != B; };

/// returns `true` if `A` is less than `B`
inline constexpr auto lt = [](const auto& A, const auto& B)
  noexcept(noexcept(A < B, 1 > 0)) -> convertible_to<bool> auto { return A < B; };

/// returns `true` if `A` is greater than `B`
inline constexpr auto gt = [](const auto& A, const auto& B)
  noexcept(noexcept(A > B)) -> convertible_to<bool> auto { return A > B; };

/// returns `true` if `A` is less than or equal to `B`
inline constexpr auto le = [](const auto& A, const auto& B)
  noexcept(noexcept(A <= B, 1 > 0)) -> convertible_to<bool> auto { return A <= B; };

/// returns `true` if `A` is greater than or equal to `B`
inline constexpr auto ge = [](const auto& A, const auto& B)
  noexcept(noexcept(A >= B)) -> convertible_to<bool> auto { return A >= B; };

/// returns the negation of `A`
inline constexpr auto neg = []<typename T>(const T& A) noexcept(noexcept(-A)) -> same_as<T> auto { return -A; };

/// returns the sum of `A` and `B`
inline constexpr auto add = []<typename T, typename U>(const T& A, const U& B)
  noexcept(noexcept(A + B)) -> common_type<T, U> { return A + B; };

/// returns the difference of `A` and `B`
inline constexpr auto sub = []<typename T, typename U>(const T& A, const U& B)
  noexcept(noexcept(A - B)) -> common_type<T, U> { return A - B; };

/// returns the product of `A` and `B`
inline constexpr auto mul = []<typename T, typename U>(const T& A, const U& B)
  noexcept(noexcept(A * B)) -> common_type<T, U> { return A * B; };

/// returns the quotient of `A` and `B`
inline constexpr auto div = []<typename T, typename U>(const T& A, const U& B)
  noexcept(noexcept(A / B)) -> common_type<T, U> { return A / B; };

/// helper structure for `bit_and`, `bit_xor`, and `bit_or`
template<natt Op> struct t_bit_and {
  constexpr none operator()() const noexcept { return {}; }
  template<typename T> constexpr T operator()(T&& t) const noexcept { return t; }
  template<typename T0, typename T1, typename... Ts> constexpr auto operator()(T0&& t0, T1&& t1, Ts&&... ts) const
    -> common_type<T0, T1, Ts...> { return (*this)(call<common_type<T0, T1, Ts...>>(fwd<T0>(t0), fwd<T1>(t1)), fwd<Ts>(ts)...); }
  template<typename F, typename T, typename... Ts> constexpr auto operator()(F&& f, T&& t, Ts&&... ts) const
    -> common_type<invoke_result<F&, T>, invoke_result<F&, Ts>...> { return (*this)(invoke(f, fwd<T>(t)), invoke(f, fwd<Ts>(ts))...); }
private:
  template<typename R, typename T, typename U> static constexpr auto call(T&& t, U&& u) -> R {
    if constexpr (Op == 0) return fwd<T>(t) | fwd<U>(u); else if constexpr (Op == 1) return fwd<T>(t) ^ fwd<U>(u); else return fwd<T>(t) & fwd<U>(u); }
};

/// returns the bitwise AND
inline constexpr t_bit_and<0> bit_and;

/// returns the bitwise XOR
inline constexpr t_bit_and<1> bit_xor;

/// returns the bitwise OR
inline constexpr t_bit_and<2> bit_or;

/// helper structure for `max` and `min`
template<bool Max> struct t_max {
  constexpr none operator()() const noexcept { return {}; }
  template<typename T> constexpr T operator()(T&& t) const noexcept { return t; }
  template<typename T0, typename T1, typename... Ts> constexpr auto operator()(T0&& t0, T1&& t1, Ts&&... ts) const
    -> common_type<T0, T1, Ts...> { return (*this)(call<common_type<T0, T1, Ts...>>(t0, t1), ts...); }
  template<typename F, typename T, typename... Ts> constexpr auto operator()(F&& f, T&& t, Ts&&... ts) const
    -> common_type<invoke_result<F&, T>, invoke_result<F&, Ts>...> { return (*this)(invoke(f, t), invoke(f, ts)...); }
private:
  template<typename R> static constexpr auto call(auto&& t, auto&& u) -> R { if constexpr (Max) return t < u ? u : t; else return u < t ? u : t; }
};

/// returns the maximum of `A` and `B`
inline constexpr t_max<1> max;

/// returns the minimum of `A` and `B`
inline constexpr t_max<0> min;


/// generic value type that can be used for non-type template parameters
struct value {
  fat8 _cpp_double{};

  /// default constructor
  constexpr value() noexcept = default;

  /// constructor taking `none`; sets the value to NaN
  explicit constexpr value(none) noexcept : _cpp_double(bitcast<fat8>(0x7ff8000000000001)) {}

  /// constructor taking an arithmetic value
  template<arithmetic T> constexpr value(T Value) noexcept : _cpp_double(static_cast<fat8>(Value)) {}

  /// constructor taking a type that can be cast to `fat8`
  template<castable_to<fat8> T> requires(!arithmetic<remove_ref<T>>) constexpr value(T&& V)
    noexcept(nt_castable_to<T, fat8>) : _cpp_double(static_cast<fat8>(fwd<T>(V))) {}

  /// assignment operator taking `none`; sets the value to NaN
  constexpr value& operator=(none) noexcept { return _cpp_double = bitcast<fat8>(0x7ff8000000000001), *this; }

  /// conversion operator to `fat8`
  constexpr operator const fat8&() const noexcept { return _cpp_double; }

  /// explicit conversion operator to an arithmetic type
  template<arithmetic T> explicit operator T() const noexcept { return static_cast<T>(_cpp_double); }

  /// adds `Value` to this
  template<castable_to<fat8> T> constexpr value& operator+=(T&& Value)
    noexcept { return _cpp_double += static_cast<fat8>(fwd<T>(Value)), *this; }

  /// subtracts `Value` from this
  template<castable_to<fat8> T> constexpr value& operator-=(T&& Value)
    noexcept { return _cpp_double -= static_cast<fat8>(fwd<T>(Value)), *this; }

  /// multiplies this by `Value`
  template<castable_to<fat8> T> constexpr value& operator*=(T&& Value)
    noexcept { return _cpp_double *= static_cast<fat8>(fwd<T>(Value)), *this; }

  /// divides this by `Value`
  template<castable_to<fat8> T> constexpr value& operator/=(T&& Value)
    noexcept { return _cpp_double /= static_cast<fat8>(fwd<T>(Value)), *this; }
};

/// constant representing the maximum value of `natt`
inline constexpr natt npos(-1);

/// constant representing the speed of light in a vacuum
inline constexpr value c = 299792458.0;

/// constant representing the base of the natural logarithm
inline constexpr value e = std::numbers::e_v<fat8>;

/// constant representing the ratio of the circumference of a circle to its diameter
inline constexpr value pi = std::numbers::pi_v<fat8>;

/// constant representing the golden ratio
inline constexpr value phi = std::numbers::phi_v<fat8>;

/// constant representing Euler's constant
inline constexpr value gamma = std::numbers::egamma_v<fat8>;

/// constant representing the natural logarithm of 2
inline constexpr value ln2 = std::numbers::ln2_v<fat8>;

/// constant representing the natural logarithm of 10
inline constexpr value ln10 = std::numbers::ln10_v<fat8>;

/// constant representing the square root of 2
inline constexpr value sqrt2 = std::numbers::sqrt2_v<fat8>;

/// constant representing the square root of 3
inline constexpr value sqrt3 = std::numbers::sqrt3_v<fat8>;

/// constant representing positive infinity
inline constexpr value inf = std::numeric_limits<fat8>::infinity();

/// constant representing NaN
inline constexpr value nan = std::numeric_limits<fat8>::quiet_NaN();

/// checks if `It` is an iterator
template<typename It> concept iterator = std::input_or_output_iterator<It>;

/// checks if `Se` is a sentinel for `It`
template<typename Se, typename It> concept sentinel_for = std::sentinel_for<Se, It>;

/// checks if `Se` is a sized sentinel for `It`
template<typename Se, typename It> concept sized_sentinel_for = std::sized_sentinel_for<Se, It>;

/// checks if `Rg` is a range
template<typename Rg> concept range = std::ranges::range<Rg>;

/// checks if `Rg` is a borrowed range
template<typename Rg> concept borrowed_range = std::ranges::borrowed_range<Rg>;

/// checks if `Rg` is a sized range
template<typename Rg> concept sized_range = std::ranges::sized_range<Rg>;

/// alias template representing the iterator type of `Rg`
template<range Rg> using iterator_t = std::ranges::iterator_t<Rg>;

/// alias template representing the borrowed iterator type of `Rg`
template<range Rg> using borrowed_iterator_t = std::ranges::borrowed_iterator_t<Rg>;

/// alias template representing the sentinel type of `Rg`
template<range Rg> using sentinel_t = std::ranges::sentinel_t<Rg>;

/// checks if `It` is an output iterator for `T`
template<typename It, typename T> concept output_iterator = std::output_iterator<It, T>;

/// checks if `It` is an input iterator
template<typename It> concept input_iterator = std::input_iterator<It>;

/// checks if `It` is a forward iterator
template<typename It> concept fwd_iterator = std::forward_iterator<It>;

/// checks if `It` is a bidirectional iterator
template<typename It> concept bid_iterator = std::bidirectional_iterator<It>;

/// checks if `It` is a random access iterator
template<typename It> concept rnd_iterator = std::random_access_iterator<It>;

/// checks if `It` is a contiguous iterator
template<typename It> concept cnt_iterator = std::contiguous_iterator<It>;

/// checks if `Rg` is an output range for `T`
template<typename Rg, typename T> concept output_range = std::ranges::output_range<Rg, T>;

/// checks if `Rg` is an input range
template<typename Rg> concept input_range = std::ranges::input_range<Rg>;

/// checks if `Rg` is a forward range
template<typename Rg> concept fwd_range = std::ranges::forward_range<Rg>;

/// checks if `Rg` is a bidirectional range
template<typename Rg> concept bid_range = std::ranges::bidirectional_range<Rg>;

/// checks if `Rg` is a random access range
template<typename Rg> concept rnd_range = std::ranges::random_access_range<Rg>;

/// checks if `Rg` is a contiguous range
template<typename Rg> concept cnt_range = std::ranges::contiguous_range<Rg>;

/// checks if `In` is indirectly copyable to `It`
template<typename It, typename In> concept iter_copyable =
  iterator<It> && iterator<In> && std::indirectly_copyable<In, It>;

/// checks if `In` is indirectly movable to `It`
template<typename It, typename In> concept iter_movable =
  iterator<It> && iterator<In> && std::indirectly_movable<In, It>;

/// checks if `Fn` is indirectly unary invocable with `It`
template<typename Fn, typename It> concept iter_unary_invocable =
  iterator<It> && std::indirectly_unary_invocable<Fn, It>;

/// checks if `Fn` is an indirect unary predicate with `It`
template<typename Fn, typename It> concept iter_unary_predicate =
  iterator<It> && std::indirect_unary_predicate<Fn, It>;

/// returns the beginning of the range
inline constexpr auto begin = []<range Rg>(Rg&& r) ywlib_wrapper(std::ranges::begin(fwd<Rg>(r)));

/// returns the end of the range
inline constexpr auto end = []<range Rg>(Rg&& r) ywlib_wrapper(std::ranges::end(fwd<Rg>(r)));

/// returns the reverse beginning of the range
inline constexpr auto rbegin = []<range Rg>(Rg&& r) ywlib_wrapper(std::ranges::rbegin(fwd<Rg>(r)));

/// returns the reverse end of the range
inline constexpr auto rend = []<range Rg>(Rg&& r) ywlib_wrapper(std::ranges::rend(fwd<Rg>(r)));

/// returns the size of the range
inline constexpr auto size = []<range Rg>(Rg&& r) ywlib_wrapper(std::ranges::size(fwd<Rg>(r)));

/// returns `true` if the range is empty
inline constexpr auto empty = []<range Rg>(Rg&& r) ywlib_wrapper(std::ranges::empty(fwd<Rg>(r)));

/// returns the data of the range
inline constexpr auto data = []<range Rg>(Rg&& r) ywlib_wrapper(std::ranges::data(fwd<Rg>(r)));

/// moves the value of the iterator
inline constexpr auto iter_move = []<iterator It>(It&& i) ywlib_wrapper(std::ranges::iter_move(fwd<It>(i)));

/// swaps the values of the iterators
inline constexpr auto iter_swap = []<iterator It, iterator Jt>(It&& i, Jt&& j) ywlib_wrapper(std::ranges::iter_swap(fwd<It>(i), fwd<Jt>(j)));

namespace _ {
template<typename T> struct _iter_t {};
template<range Rg> struct _iter_t<Rg> : _iter_t<iterator_t<Rg>> {};
template<iterator It> struct _iter_t<It> {
  using v = std::iter_value_t<It>; using d = std::iter_difference_t<It>;
  using r = std::iter_reference_t<It>; using rr = std::iter_rvalue_reference_t<It>; };
}

/// alias template representing the value type of `T`
template<typename T> requires iterator<T> || range<T> using iter_value = typename _::_iter_t<T>::v;

/// alias template representing the difference type of `T`
template<typename T> requires iterator<T> || range<T> using iter_difference = typename _::_iter_t<T>::d;

/// alias template representing the reference type of `T`
template<typename T> requires iterator<T> || range<T> using iter_reference = typename _::_iter_t<T>::r;

/// alias template representing the rvalue reference type of `T`
template<typename T> requires iterator<T> || range<T> using iter_rvref = typename _::_iter_t<T>::rr;

/// alias template representing the common type of `iter_reference_t<T>` and `iter_value_t<T>`
template<typename T> requires iterator<T> || range<T> using iter_common = common_type<iter_reference<T>, iter_value<T>>;

/// checks if `It` is an iterator for `U`
template<typename It, typename U> concept iterator_for = iterator<It> && convertible_to<iter_reference<It>, U>;

/// checks if `It` is an iterator of `U`
template<typename It, typename U> concept iterator_of = iterator_for<It, U> && same_as<iter_value<It>, U>;

/// checks if `It` is a contiguous iterator of `U`
template<typename It, typename U> concept cnt_iterator_of = cnt_iterator<It> && iterator_of<It, U>;

/// checks if `Rg` is a range for `U`
template<typename Rg, typename U> concept range_for = iterator_for<iterator_t<Rg>, U>;

/// checks if `Rg` is a range of `U`
template<typename Rg, typename U> concept range_of = iterator_of<iterator_t<Rg>, U>;

/// checks if `Rg` is a contiguous range of `U`
template<typename Rg, typename U> concept cnt_range_of = cnt_range<Rg> && range_of<Rg, U>;

/// checks if `Rg` is a contiguous sized range
template<typename Rg> concept cnt_sized_range = cnt_range<Rg> && sized_range<Rg>;

/// checks if `Rg` is a contiguous sized range of `U`
template<typename Rg, typename U> concept cnt_sized_range_of = cnt_sized_range<Rg> && range_of<Rg, U>;

/// static array structure if `N > 0`, otherwise dynamic array class derived from `std::vector`
template<typename T, natt N = 0> class array {
public:
  T _cpp_array[N]{};

  /// number of elements
  static constexpr natt count = N;

  /// value type
  using value_type = T;

  /// copies the elements from `Array`
  template<convertible_to<T>> constexpr array& operator=(const T (&Array)[N]) {
    for (natt i{}; i < N; ++i) _cpp_array[i] = Array[i]; return *this; }

  /// conversion operator to `T[N]`
  constexpr operator add_lvref<T[N]>() noexcept { return _cpp_array; }

  /// conversion operator to `const T[N]`
  constexpr operator add_lvref<const T[N]>() const noexcept { return _cpp_array; }

  /// returns the `I`-th element
  constexpr T& operator[](natt I) { return _cpp_array[I]; }
  constexpr const T& operator[](natt I) const { return _cpp_array[I]; }

  /// returns the number of elements
  constexpr natt size() const noexcept { return N; }

  /// returns `false`
  constexpr bool empty() const noexcept { return false; }

  /// returns the pointer to the first element
  constexpr T* data() noexcept { return _cpp_array; }
  constexpr const T* data() const noexcept { return _cpp_array; }

  /// returns the iterator to the first element
  constexpr T* begin() noexcept { return _cpp_array; }
  constexpr const T* begin() const noexcept { return _cpp_array; }

  /// returns the iterator to the last element
  constexpr T* end() noexcept { return _cpp_array + N; }
  constexpr const T* end() const noexcept { return _cpp_array + N; }

  /// returns the first element
  constexpr T& front() noexcept { return *_cpp_array; }
  constexpr const T& front() const noexcept { return *_cpp_array; }

  /// returns the last element
  constexpr T& back() noexcept { return _cpp_array[N - 1]; }
  constexpr const T& back() const noexcept { return _cpp_array[N - 1]; }

  /// returns the `I`-th element
  template<natt I> requires(I < N) constexpr T& get() & noexcept { return _cpp_array[I]; }
  template<natt I> requires(I < N) constexpr T&& get() && noexcept { return mv(_cpp_array[I]); }
  template<natt I> requires(I < N) constexpr const T& get() const& noexcept { return _cpp_array[I]; }
  template<natt I> requires(I < N) constexpr const T&& get() const&& noexcept { return mv(_cpp_array[I]); }
};

/// dynamic array class derived from `std::vector`
template<typename T> class array<T, 0> : public std::vector<T> {
public:
  /// default constructor
  constexpr array() noexcept = default;

  /// constructor taking rvalue reference to `std::vector`
  constexpr array(std::vector<T>&& v) : std::vector<T>(mv(v)) {}

  /// constructor taking the number of elements
  constexpr explicit array(natt n) : std::vector<T>(n) {}

  /// constructor taking the number of elements and the value to initialize
  constexpr array(natt n, const T& v) : std::vector<T>(n, v) {}

  /// constructor taking a range
  template<iterator_for<T> It> constexpr array(It i, It s) : std::vector<T>(i, s) {}
  template<iterator_for<T> It, sentinel_for<It> Se> requires(!same_as<It, Se>)
  constexpr array(It i, Se s) : std::vector<T>(std::common_iterator<It, Se>(i), std::common_iterator<It, Se>(s)) {}
  template<range_for<T> Rg> constexpr array(Rg&& r) : std::vector<T>(yw::begin(r), yw::end(r)) {}
};

/// deduction guide for `array`
template<typename T, convertible_to<T>... Ts> array(T, Ts...) -> array<T, 1 + sizeof...(Ts)>;
template<typename T> array(natt, const T&) -> array<T, 0>;
template<iterator It, sentinel_for<It> Se> array(It, Se) -> array<iter_value<It>, 0>;
template<range Rg> array(Rg&&) -> array<iter_value<Rg>, 0>;

template<auto... Vs> struct sequence;
template<typename... Ts> struct typepack;
template<typename... Ts> struct list;

namespace _::_get {
template<natt I> void get(auto&&) = delete;
template<natt I, typename T> inline constexpr auto pattern = []() -> int4 {
  if constexpr (is_bounded_array<remove_ref<T>> && I < std::extent_v<remove_ref<T>>) return 4;
  else if constexpr (requires { get<I>(declval<T>()); }) return 1 | noexcept(get<I>(declval<T>())) * 4;
  else if constexpr (requires { declval<T>().template get<I>(); }) return 2 | noexcept(declval<T>().template get<I>()) * 4;
  else return 8; }();
template<natt I, typename T, natt P = pattern<I, T>> requires(P < 8) constexpr decltype(auto) call(T&& A) noexcept(bool(P & 4)) {
  if constexpr ((P & 3) == 0) return static_cast<T&&>(A)[I];
  else if constexpr ((P & 3) == 1) return get<I>(static_cast<T&&>(A));
  else if constexpr ((P & 3) == 2) return static_cast<T&&>(A).template get<I>(); }
}

/// returns the `I`-th element of `Tuple`
template<natt I> inline constexpr auto get = []<typename Tp>(Tp&& Tuple)
  ywlib_wrapper(_::_get::call<I>(static_cast<Tp&&>(Tuple)));

/// checks if `get<I>(Tp)` is valid
template<typename Tp, natt I> concept gettable = requires { get<I>(declval<Tp>()); };

/// checks if `get<I>(Tp)` is valid and noexcept
template<typename Tp, natt I> concept nt_gettable = requires {
  requires gettable<Tp, I>;
  { get<I>(declval<Tp>()) } noexcept; };

/// alias template representing the `I`-th element type of `Tp`
template<typename Tp, natt I> requires gettable<Tp, I> using element_t = decltype(get<I>(declval<Tp>()));

namespace _ {
template<typename T> inline constexpr natt _extent =
  type_switch<inspects<is_array<T>, requires { std::tuple_size<T>::value; }>, std::extent<T>, std::tuple_size<T>, constant<0_nn>>::value;
template<typename T, typename U, typename Sq> struct _tuple_for : constant<false> {}; template<typename T, typename U, natt... Is>
struct _tuple_for<T, U, sequence<Is...>> : constant<(convertible_to<element_t<T, Is>, U> && ...)> {};
template<typename T, typename U, typename Sq> struct _nt_tuple_for : constant<false> {};
template<typename T, typename U, natt... Is> struct _nt_tuple_for<T, U, sequence<Is...>>
  : constant<(nt_gettable<T, Is> && ...) && (nt_convertible_to<element_t<T, Is>, U> && ...)> {};
template<typename S, typename T = none> struct _to_sequence;
template<template<auto...> typename Tm, typename T, auto... Vs> struct _to_sequence<Tm<Vs...>, T> : trait<sequence<static_cast<T>(Vs)...>> {};
template<template<typename, auto...> typename Tm, typename T, typename U, auto... Vs>
struct _to_sequence<Tm<U, Vs...>, T> : trait<sequence<static_cast<T>(Vs)...>> {};
template<template<auto...> typename Tm, auto... Vs> struct _to_sequence<Tm<Vs...>, none> : trait<sequence<Vs...>> {};
template<template<typename, auto...> typename Tm, typename U, auto... Vs> struct _to_sequence<Tm<U, Vs...>, none> : trait<sequence<Vs...>> {};
template<typename S, natt N> struct _indices_for : constant<false> {};
template<natt... Is, natt N> struct _indices_for<sequence<Is...>, N> : constant<(lt(Is, N) && ...)> {};
template<natt End, natt Begin = 0, invocable<natt> auto Proj = pass{}, natt... Vs>
struct _make_sequence : _make_sequence<End, Begin + 1, Proj, Vs..., Proj(Begin)> {};
template<natt End, auto Proj, natt... Vs> struct _make_sequence<End, End, Proj, Vs...> : trait<sequence<Vs...>> {};
template<natt I, natt N, typename S, typename... T> struct _cond_indices;
template<natt I, natt N, bool... Bs, natt... Is> struct _cond_indices<I, N, sequence<Bs...>, sequence<Is...>>
  : _cond_indices<I + 1, N, sequence<Bs...>, type_switch<value_switch<I, Bs...>, sequence<Is..., I>, sequence<Is...>>> {};
template<natt N, bool... Bs, natt... Is> struct _cond_indices<N, N, sequence<Bs...>, sequence<Is...>> : trait<sequence<Is...>> {};
template<typename T, typename S> struct _to_typepack; template<typename T, natt... Is>
struct _to_typepack<T, sequence<Is...>> : trait<typepack<element_t<T, Is>...>> {};
}

/// returns the size of `Tp`
template<typename Tp> inline constexpr natt extent = _::_extent<remove_ref<Tp>>;

/// checks if `Tp` is a tuple
template<typename Tp> concept tuple = extent<Tp> != 0;
template<typename... Tps> concept same_size_tuple = requires {
  requires(tuple<Tps> && ...);
  requires((extent<Tps> == extent<type_switch<0, Tps...>>) && ...); };

/// converts `Sq` to the specialization of `sequence` with the components converted to `T` if specified
template<typename Sq, typename T = none> using to_sequence = typename _::_to_sequence<Sq, T>::type;

/// checks if `to_sequence<Sq, T>` is a valid sequence
template<typename Sq, typename T = none> concept sequence_of = variation_of<to_sequence<Sq, T>, sequence<0>>;

/// checks if the number of components of `Sq` is equal to or less than the extent of `Tp`
template<typename Sq, typename Tp> concept indices_for = requires {
  requires tuple<Tp>;
  requires sequence_of<Sq, natt>;
  requires _::_indices_for<to_sequence<Sq>, extent<Tp>>::value; };

/// returns `sequence<Proj(Is)...>` where `Is = [Begin, End)`
template<natt End, natt Begin = 0, invocable<natt> auto Proj = pass{}> requires(Begin <= End)
using make_sequence = typename _::_make_sequence<End, Begin, Proj>::type;

/// returns `sequence<Is...>` where `Is = [0, extent<Tp>)`
template<typename Tp> using make_indices_for = make_sequence<extent<Tp>>;

/// returns `sequence<Is...>` where `{Is | Sq.at<Is> == true}`
template<sequence_of<bool> Sq> using cond_indices =
  typename _::_cond_indices<0, extent<to_sequence<Sq, bool>>, to_sequence<Sq, bool>, sequence<>>::type;

/// checks if `Tp` is a tuple for `U`
template<typename Tp, typename U> concept tuple_for = requires {
  requires tuple<Tp>;
  requires _::_tuple_for<Tp, U, make_indices_for<Tp>>::value; };

/// checks if `Tp` is a tuple for `U` and `get<I>(Tp)` is noexcept for all `I`
template<typename Tp, typename U> concept nt_tuple_for = requires {
  requires tuple_for<Tp, U>;
  requires _::_nt_tuple_for<Tp, U, make_indices_for<Tp>>::value; };

/// converts `Tp` to the specialization of `typepack` those components are the element types of `Tp`
template<tuple Tp> using to_typepack = typename _::_to_typepack<Tp, make_indices_for<Tp>>::type;

/// value sequence structure
template<auto... Vs> struct sequence {
private:
  template<typename S> struct _append : _append<to_sequence<S, none>> {};
  template<auto... Ws> struct _append<sequence<Ws...>> : trait<sequence<Vs..., Ws...>> {};
  template<> struct _append<none> : trait<none> {};
  template<typename S> struct _pickup : _pickup<to_sequence<S, none>> {};
  template<natt... Is> struct _pickup<sequence<Is...>> : trait<sequence<value_switch<Is, Vs...>...>> {};
  template<> struct _pickup<none> : trait<none> {};
public:
  /// number of values
  static constexpr natt count = sizeof...(Vs);

  /// returns the `I`-th value
  template<natt I> requires(lt(I, count)) static constexpr auto at = value_switch<I, Vs...>;

  /// returns the type of `I`-th value
  template<natt I> requires(lt(I, count)) using type_at = type_switch<I, decltype(Vs)...>;

  /// appends another sequence
  template<sequence_of Sq> using append = typename _append<to_sequence<Sq>>::type;

  /// picks up the values at the specified indices
  template<indices_for<sequence> Sq> using pickup = typename _pickup<to_sequence<Sq, natt>>::type;

  /// returns the first `N` values
  template<natt N> requires(N < count) using fore = pickup<make_sequence<N>>;

  /// returns the last `N` values
  template<natt N> requires(N < count) using back = pickup<make_sequence<count, count - N>>;

  /// inserts another sequence at the specified index
  template<natt I, sequence_of Sq> requires(I < count) using insert =
    typename fore<I>::template append<Sq>::template append<back<count - I>>;

  /// expands the sequence to the specified template
  template<template<auto...> typename Tm> using expand = Tm<Vs...>;

  /// returns the `I`-th value
  template<natt I> requires(I < count) constexpr const auto&& get() const noexcept { return mv(at<I>); }
};

/// type pack structure
template<typename... Ts> struct typepack {
private:
  template<typename T, typename U> struct t_append {};
  template<typename... Us, typename... Vs> struct t_append<typepack<Us...>, typepack<Vs...>> : trait<typepack<Us..., Vs...>> {};
  template<typename T, typename S> struct t_pickup {};
  template<typename... Us, natt... Is> struct t_pickup<typepack<Us...>, sequence<Is...>> : trait<typepack<type_switch<Is, Us...>...>> {};
public:
  /// number of types
  static constexpr natt count = sizeof...(Ts);

  /// common type of the types
  using common = common_type<Ts...>;

  /// returns the `I`-th type
  template<natt I> requires(I < count) using at = type_switch<I, Ts...>;

  /// appends another pack
  template<tuple T> using append = typename t_append<typepack, to_typepack<T>>::type;

  /// picks up the types at the specified indices
  template<indices_for<typepack> S> using pickup = typename t_pickup<typepack, to_sequence<S, natt>>::type;

  /// returns the first `N` types
  template<natt N> requires(N <= count) using fore = pickup<make_sequence<N>>;

  /// returns the last `N` types
  template<natt N> requires(N <= count) using back = pickup<make_sequence<count, count - N>>;

  /// inserts the element types of `Tp` at the specified index
  template<natt I, tuple T> requires(I < count) using insert =
    typename fore<I>::template append<T>::template append<back<count - I>>;

  /// expands the pack to the specified template
  template<template<typename...> typename Tm> using expand = Tm<Ts...>;

  /// unevaluable function; returns the `I`-th type
  template<natt I> requires(I < count) constexpr at<I> get() const noexcept;
};

/// basic tuple structure
template<typename... Ts> struct list : typepack<Ts...>::template fore<sizeof...(Ts) - 1>::template expand<list> {
  using base = typepack<Ts...>::template fore<sizeof...(Ts) - 1>::template expand<list>;

  /// number of the elements
  static constexpr natt count = sizeof...(Ts);

  /// type of the last element
  using last_type = type_switch<count - 1, Ts...>;

  /// last element
  last_type last;

  /// returns the `I`-th element
  template<natt I> constexpr auto get() & noexcept -> type_switch<I, Ts...>& {
    if constexpr (I == count - 1) return last; else return base::template get<I>(); }
  template<natt I> constexpr auto get() const& noexcept -> add_const<type_switch<I, Ts...>&> {
    if constexpr (I == count - 1) return last; else return base::template get<I>(); }
  template<natt I> constexpr auto get() && noexcept -> type_switch<I, Ts...>&& {
    if constexpr (I == count - 1) return fwd<type_switch<I, Ts...>>(last);
    else return mv(*this).base::template get<I>(); }
  template<natt I> constexpr auto get() const&& noexcept -> add_const<type_switch<I, Ts...>&&> {
    if constexpr (I == count - 1) return fwd<add_const<type_switch<I, Ts...>>>(last);
    else return mv(*this).base::template get<I>(); }
};

template<typename T1, typename T2, typename T3> struct list<T1, T2, T3> : list<T1, T2> {
  using base = list<T1, T2>;

  /// number of the elements
  static constexpr natt count = 3;

  /// type of the third element
  using third_type = T3;

  /// third element
  third_type third;

  /// returns the `I`-th element
  template<natt I> constexpr auto get() & noexcept -> type_switch<I, T1, T2, T3>& {
    if constexpr (I == 2) return third; else return base::template get<I>(); }
  template<natt I> constexpr auto get() const& noexcept -> add_const<type_switch<I, T1, T2, T3>&> {
    if constexpr (I == 2) return third; else return base::template get<I>(); }
  template<natt I> constexpr auto get() && noexcept -> type_switch<I, T1, T2, T3>&& {
    if constexpr (I == 2) return fwd<type_switch<I, T1, T2, T3>>(third);
    else return mv(*this).base::template get<I>(); }
  template<natt I> constexpr auto get() const&& noexcept -> add_const<type_switch<I, T1, T2, T3>&&> {
    if constexpr (I == 2) return fwd<add_const<type_switch<I, T1, T2, T3>>>(third);
    else return mv(*this).base::template get<I>(); }
};

template<typename T1, typename T2> struct list<T1, T2> : public list<T1> {
  using base = list<T1>;

  /// number of the elements
  static constexpr natt count = 2;

  /// type of the second element
  using second_type = T2;

  /// second element
  second_type second;

  /// returns the `I`-th element
  template<natt I> constexpr auto get() & noexcept -> type_switch<I, T1, T2>& {
    if constexpr (I == 1) return second; else return base::template get<I>(); }
  template<natt I> constexpr auto get() const& noexcept -> add_const<type_switch<I, T1, T2>&> {
    if constexpr (I == 1) return second; else return base::template get<I>(); }
  template<natt I> constexpr auto get() && noexcept -> type_switch<I, T1, T2>&& {
    if constexpr (I == 1) return fwd<type_switch<I, T1, T2>>(second); else return mv(*this).base::template get<I>(); }
  template<natt I> constexpr auto get() const&& noexcept -> add_const<type_switch<I, T1, T2>&&> {
    if constexpr (I == 1) return fwd<add_const<type_switch<I, T1, T2>>>(second);
    else return mv(*this).base::template get<I>(); }
};

/// specialization for a list of 1 element
template<typename T1> struct list<T1> {

  /// number of the elements
  static constexpr natt count = 1;

  /// type of the first element
  using first_type = T1;

  /// first element
  first_type first;

  /// returns the `I`-th element
  template<natt I> requires(I == 0) constexpr first_type& get() & noexcept { return first; }
  template<natt I> requires(I == 0) constexpr add_const<first_type&> get() const& noexcept { return first; }
  template<natt I> requires(I == 0) constexpr first_type&& get() && noexcept { return fwd<first_type&&>(first); }
  template<natt I> requires(I == 0) constexpr add_const<first_type&&> get() const&&
    noexcept { return fwd<add_const<first_type&&>>(first); }
};

template<> struct list<> {
private:
  template<typename T, typename U, natt... Is, natt... Js> static constexpr auto
  _concat(T&& t, U&& u, sequence<Is...>, sequence<Js...>) ywlib_wrapper((yw::list{get<Is>(fwd<T>(t))..., get<Js>(fwd<U>(u))...}));
  template<typename T, typename U, typename V> struct _from_typepack {};
  template<typename... Ts, typename U, template<typename...> typename Tm, typename... Vs>
  struct _from_typepack<typepack<Ts...>, U, Tm<Vs...>> : trait<list<copy_cvref<Tm<Ts>, U>...>> {};
  template<typename... Ts, typename U, template<typename, auto...> typename Tm, typename V, auto... Vs>
  struct _from_typepack<typepack<Ts...>, U, Tm<V, Vs...>> : trait<list<copy_cvref<Tm<Ts, Vs...>, U>...>> {};
  template<typename... Ts, typename U, typename V> struct _from_typepack<typepack<Ts...>, U, V> : trait<list<copy_cvref<Ts, U>...>> {};
public:

  /// number of the elements
  static constexpr natt count = 0;

  /// concatenates `Fore` and `Back`
  template<tuple T, tuple U> static constexpr auto concat(T&& Fore, U&& Back)
    ywlib_wrapper(_concat(fwd<T>(Fore), fwd<U>(Back), make_indices_for<T>{}, make_indices_for<U>{}));

  /// packs `Args` into a reference list
  template<typename... Ts> static constexpr auto asref(Ts&&... Args)
    noexcept { return list<Ts&&...>{fwd<Ts>(Args)...}; }

  /// returns a specialization of `list` from the typepack `Tp`
  /// @note `list<>::from_typepack<typepack<Ts...>, const int&>` is `list<const Ts&, ...>`
  /// @note `list<>::from_typepack<typepack<Ts...>, const Tm<Us...>&>` is `list<const Tm<Ts>&, ...>`
  template<specialization_of<typepack> Tp, typename Template = none> using from_typepack = _from_typepack<Tp, Template, remove_cvref<Template>>::type;
};

/// deduction guide for `list`
template<typename... Ts> list(Ts...) -> list<Ts...>;

struct t_apply {
  template<natt... Is, typename Fn, typename Tp> constexpr auto operator()(sequence<Is...>, Fn&& Func, Tp&& Tuple) const
    ywlib_wrapper(invoke(fwd<Fn>(Func), get<Is>(fwd<Tp>(Tuple))...));
  template<typename Sq, typename Fn, typename Tp> constexpr auto operator()(Sq, Fn&& Func, Tp&& Tuple) const
    ywlib_wrapper((*this)(to_sequence<Sq, natt>{}, fwd<Fn>(Func), fwd<Tp>(Tuple)));
  template<typename Fn, typename Tp> constexpr auto operator()(Fn&& Func, Tp&& Tuple) const
    ywlib_wrapper((*this)(make_indices_for<Tp>{}, fwd<Fn>(Func), fwd<Tp>(Tuple)));
};

/// returns the result of `invoke(Func, get<Is>(Tuple)...)`
inline constexpr t_apply apply;

/// checks if `apply(Sq, Fn, Tp, ) is valid
template<typename Fn, typename Tp, typename Sq = make_indices_for<Tp>> concept applyable = requires {
  requires tuple<Tp>;
  requires indices_for<Sq, Tp>;
  apply(Sq{}, declval<Fn>(), declval<Tp>()); };

/// checks if `apply(Sq, Fn, Tp) is valid
template<typename Fn, typename Tp, typename Sq = make_indices_for<Tp>> concept nt_applyable =
  requires { requires applyable<Fn, Tp, Sq>; { apply(Sq{}, declval<Fn>(), declval<Tp>()) } noexcept; };

/// returns the result type of `apply(Func, Tuple)`
template<typename Fn, typename Tp, typename Sq = make_indices_for<Tp>> requires applyable<Fn, Tp, Sq>
using apply_result = decltype(apply(Sq{}, declval<Fn>(), declval<Tp>()));

struct t_vapply {
private:
  template<natt I, typename Fn, typename... Ts> constexpr static auto call(constant<I>, Fn&& f, Ts&&... ts)
    ywlib_wrapper(invoke(f, get<I>(fwd<Ts>(ts)...)));
  template<natt... Is, typename Fn, tuple... Ts> constexpr static auto call(sequence<Is...>, Fn&& f, Ts&&... ts)
    ywlib_wrapper(list<>::asref(call(constant<Is>{}, f, fwd<Ts>(ts)...)...));
public:
  template<typename Fn, tuple Tp, same_size_tuple<Tp>... Tps> constexpr auto operator()(Fn&& f, Tp&& tp, Tps&&... tps)
    const ywlib_wrapper(call(make_indices_for<Tp>{}, fwd<Fn>(f), fwd<Tp>(tp), fwd<Tps>(tps)...));
};

/// returns `list<>::asref(invoke(Func, get<Is>(Tuples)...)...)`
inline constexpr t_vapply vapply;

/// checks if `vapply(Func, Tuples...)` is valid
template<typename Fn, typename... Ts> concept vapplyable = requires {
  requires (tuple<Ts> && ...);
  vapply(declval<Fn>(), declval<Ts>()...); };

/// checks if `vapply(Func, Tuples...)` is valid and noexcept
template<typename Fn, typename... Ts> concept nt_vapplyable = requires {
  requires vapplyable<Fn, Ts...>;
  { vapply(declval<Fn>(), declval<Ts>()...) } noexcept; };

/// returns the result type of `vapply(Func, Tuples...)`
template<typename Fn, typename... Ts> requires vapplyable<Fn, Ts...>
using vapply_result = decltype(vapply(declval<Fn>(), declval<Ts>()...));

struct t_vassign {
  template<natt... Is, typename TpL, typename TpR, typename Pj = pass>
  constexpr TpL&& operator()(sequence<Is...>, TpL&& Left, TpR&& Right, Pj proj = {}) const
    noexcept(noexcept(((get<Is>(Left) = invoke(proj, get<Is>(Right))), ...)))
    requires requires { ((get<Is>(Left) = invoke(proj, get<Is>(Right))), ...); }
  { return ((get<Is>(Left) = invoke(proj, get<Is>(Right))), ...), fwd<TpL>(Left); }

  template<typename Sq, typename TpL, typename TpR, typename Pj = pass>
  constexpr auto operator()(Sq, TpL&& Left, TpR&& Right, Pj proj = {}) const
    ywlib_wrapper((*this)(to_sequence<Sq, natt>{}, fwd<TpL>(Left), fwd<TpR>(Right), mv(proj)));

  template<typename TpL, typename TpR, typename Pj = pass>
  constexpr auto operator()(TpL&& Left, TpR&& Right, Pj proj = {}) const
    ywlib_wrapper((*this)(make_indices_for<TpL>{}, fwd<TpL>(Left), fwd<TpR>(Right), mv(proj)));
};

/// performs `get<Is>(Left) = get<Is>(Right)`
inline constexpr t_vassign vassign;

/// checks if `vassign(Sq, TpL, TpR)` is valid
template<typename TpL, typename TpR, typename Sq = make_indices_for<TpL>> concept vassignable = requires {
  requires indices_for<Sq, TpL>;
  requires indices_for<Sq, TpR>;
  vassign(Sq{}, declval<TpL>(), declval<TpR>()); };

/// checks if `vassign(Sq, TpL, TpR)` is valid and noexcept
template<typename TpL, typename TpR, typename Sq = make_indices_for<TpL>> concept nt_vassignable = requires {
  requires vassignable<TpL, TpR, Sq>;
  { vassign(Sq{}, declval<TpL>(), declval<TpR>()) } noexcept; };

#pragma region build, buildable, nt_buildable

template<typename T> struct t_build {
  template<natt... Is, typename Tp> constexpr T operator()(sequence<Is...>, Tp&& Tuple) const
    noexcept((nt_gettable<Tp, Is> && ...) && nt_constructible<T, element_t<Tp, Is>...>)
      requires(gettable<Tp, Is> && ...) && constructible<T, element_t<Tp, Is>...> { return T{get<Is>(fwd<Tp>(Tuple))...}; }
  template<typename Sq, typename Tp> constexpr auto operator()(Sq, Tp&& Tuple) const ywlib_wrapper((*this)(to_sequence<Sq, natt>{}, fwd<Tp>(Tuple)));
  template<tuple Tp> constexpr auto operator()(Tp&& Tuple) const ywlib_wrapper((*this)(make_indices_for<Tp>{}, fwd<Tp>(Tuple)));
};

/// returns `T{get<Is>(Tuple)...}`
template<typename T> inline constexpr t_build<T> build;

/// checks if `build<T>(Sq, Tp)` is valid
template<typename T, typename Tp, typename Sq = make_indices_for<Tp>> concept buildable = requires {
  requires indices_for<Sq, Tp>;
  build<T>(Sq{}, declval<Tp>()); };

/// checks if `build<T>(Sq, Tp)` is valid and noexcept
template<typename T, typename Tp, typename Sq = make_indices_for<Tp>> concept nt_buildable = requires {
  requires buildable<T, Tp, Sq>;
  { build<T>(Sq{}, declval<Tp>()) } noexcept; };

#pragma endregion build


/// structure for making a virtual tuple from a reference
template<typename T, typename Pj = pass, sequence_of<natt> Sq = make_indices_for<T>> struct projector {
  static_assert(to_sequence<Sq>::count > 0);
  static_assert(!tuple<T> || indices_for<Sq, T>);

  /// indices type
  using indices = to_sequence<Sq, natt>;

  /// number of elements
  static constexpr natt count = indices::count;

  /// reference to the object
  add_fwref<T> ref;

  /// projection function
  remove_ref<Pj> proj = {};

  /// constructor
  template<typename T_ = T> constexpr projector(T_&& t, Pj p, Sq) noexcept : ref(fwd<T_>(t)), proj(mv(p)) {}
  template<typename T_ = T> constexpr projector(T_&& t, Pj p) noexcept : ref(fwd<T_>(t)), proj(mv(p)) {}
  template<typename T_ = T> constexpr projector(T_&& t, Sq) noexcept : ref(fwd<T_>(t)) {}
  template<typename T_ = T> constexpr projector(T_&& t) noexcept : ref(fwd<T_>(t)) {}

  /// get function for tuple-to-tuple projection
  template<natt I> requires (I < count && tuple<T>) constexpr auto get() const
    noexcept(nt_gettable<T, indices::template at<I>> && nt_invocable<Pj, element_t<T, indices::template at<I>>>)
    requires gettable<T, indices::template at<I>> && invocable<Pj, element_t<T, indices::template at<I>>>
  { return invoke(proj, yw::get<indices::template at<I>>(ref)); }

  /// get function for non-tuple-to-tuple projection
  template<natt I> requires (I < count && !tuple<T>) constexpr auto get() const
    noexcept(nt_invocable<Pj, T>) requires invocable<Pj, T> { return invoke(proj, ref); }
};

/// deduction guide for `projector`
template<typename T, typename Pj, sequence_of<natt> Sq> projector(T&&, Pj, Sq) -> projector<T, Pj, Sq>;
template<typename T, typename Pj> requires (!sequence_of<Pj, natt>) projector(T&&, Pj) -> projector<T, Pj>;
template<typename T, sequence_of<natt> Sq> projector(T&&, Sq) -> projector<T, pass, Sq>;
template<typename T> projector(T&&) -> projector<T>;


/// string view class
template<character Ct> struct string_view {
  using value_type = Ct;
  const Ct* const pointer{};
  const natt count{};
  string_view(np_t) = delete;
  constexpr string_view() noexcept = default;
  constexpr string_view(const Ct* s) : pointer(s), count(std::char_traits<Ct>::length(s)) {}
  constexpr string_view(const Ct* s, natt n) noexcept : pointer(s), count(n) {}
  template<natt N> constexpr string_view(const Ct (&s)[N]) noexcept : pointer(s), count(N - 1) {}
  template<cnt_iterator_of<Ct> It> constexpr string_view(It i, natt n) : pointer(addressof(*i)), count(n) {}
  template<cnt_iterator_of<Ct> It, sized_sentinel_for<It> Se> constexpr string_view(It i, Se s) : pointer(addressof(*i)), count(s - i) {}
  template<cnt_sized_range_of<Ct> Rg> constexpr string_view(Rg&& r) : string_view(yw::begin(r), yw::end(r)) {}
  constexpr bool empty() const noexcept { return !count; }
  constexpr natt size() const noexcept { return count; }
  constexpr const Ct* data() const noexcept { return pointer; }
  constexpr const Ct* begin() const noexcept { return pointer; }
  constexpr const Ct* end() const noexcept { return pointer + count; }
  constexpr Ct operator[](natt i) const noexcept { return pointer[i]; }
  constexpr Ct front() const noexcept { return *pointer; }
  constexpr Ct back() const noexcept { return pointer[count - 1]; }
  constexpr string_view remove_prefix(natt n) const noexcept { return {pointer + n, count - n}; }
  constexpr string_view remove_suffix(natt n) const noexcept { return {pointer, count - n}; }
  constexpr string_view substr(natt pos, natt n = npos) const noexcept {
    if (pos > count) throw std::out_of_range("string_view::substr");
    return {pointer + pos, min(n, count - pos)};
  }
  constexpr natt find(Ct c, natt pos = 0) const noexcept {
    for (natt i = pos; i < count; ++i)
      if (pointer[i] == c) return i;
    return npos;
  }
  constexpr natt find(string_view s, natt pos = 0) const noexcept {
    if (s.count == 0) return pos;
    if (s.count > count) return npos;
    for (natt i = pos; i <= count - s.count; ++i)
      if (pointer[i] == s.pointer[0] && string_view(pointer + i, s.count) == s) return i;
    return npos;
  }
  constexpr natt rfind(Ct c, natt pos = npos) const noexcept {
    if (pos >= count) pos = count - 1;
    for (natt i = pos; i != npos; --i)
      if (pointer[i] == c) return i;
    return npos;
  }
  constexpr natt rfind(string_view s, natt pos = npos) const noexcept {
    if (s.count == 0) return pos;
    if (s.count > count) return npos;
    if (pos > count - s.count) pos = count - s.count;
    for (natt i = pos; i != npos; --i)
      if (pointer[i] == s.pointer[0] && string_view(pointer + i, s.count) == s) return i;
    return npos;
  }
  constexpr natt find_first_of(string_view s, natt pos = 0) const noexcept {
    for (natt i = pos; i < count; ++i)
      if (s.find(pointer[i]) != npos) return i;
    return npos;
  }
  constexpr natt find_first_not_of(string_view s, natt pos = 0) const noexcept {
    for (natt i = pos; i < count; ++i)
      if (s.find(pointer[i]) == npos) return i;
    return npos;
  }
  constexpr natt find_last_of(string_view s, natt pos = npos) const noexcept {
    if (pos >= count) pos = count - 1;
    for (natt i = pos; i != npos; --i)
      if (s.find(pointer[i]) != npos) return i;
    return npos;
  }
  constexpr natt find_last_not_of(string_view s, natt pos = npos) const noexcept {
    if (pos >= count) pos = count - 1;
    for (natt i = pos; i != npos; --i)
      if (s.find(pointer[i]) == npos) return i;
    return npos;
  }
  constexpr bool starts_with(Ct c) const noexcept { return count && pointer[0] == c; }
  constexpr bool starts_with(string_view s) const noexcept { return count >= s.count && string_view(pointer, s.count) == s; }
  constexpr bool ends_with(Ct c) const noexcept { return count && pointer[count - 1] == c; }
  constexpr bool ends_with(string_view s) const noexcept { return count >= s.count && string_view(pointer + count - s.count, s.count) == s; }
  constexpr bool null_terminated() const noexcept { return *end() == Ct{}; }
  constexpr bool contains(Ct c) const noexcept { return find(c) != npos; }
  constexpr bool contains(string_view s) const noexcept { return find(s) != npos; }
  constexpr bool operator==(string_view s) const noexcept { return count == s.count && std::char_traits<Ct>::compare(pointer, s.pointer, count) == 0; }
  constexpr auto operator<=>(string_view s) const noexcept {
    for (natt i{}, ii{min(count, s.count)}; i < ii; ++i)
      if (auto c = pointer[i] <=> s.pointer[i]; c != 0) return c;
    return so_t::equivalent;
  }
};

template<character Ct> string_view(const Ct*, natt) -> string_view<Ct>;
template<cnt_iterator It, sentinel_for<It> Se> string_view(It, Se) -> string_view<iter_value<It>>;
template<cnt_range Rg> string_view(Rg&&) -> string_view<iter_value<Rg>>;

using stv1 = string_view<cat1>;
using stv2 = string_view<cat2>;
constexpr stv1 operator""_sv(const cat1* s, natt n) noexcept { return stv1(s, n); }
constexpr stv2 operator""_sv(const cat2* s, natt n) noexcept { return stv2(s, n); }


/// dynamic string class
template<character Ct> class string : public std::basic_string<Ct> {
public:
  constexpr string() noexcept = default;
  constexpr string(std::basic_string<Ct> s) noexcept : std::basic_string<Ct>(mv(s)) {}
  constexpr string(natt n, Ct c) : std::basic_string<Ct>(n, c) {}
  constexpr string(const Ct* s) : std::basic_string<Ct>(s) {}
  constexpr string(const Ct* s, natt n) : std::basic_string<Ct>(s, n) {}
  template<natt N> constexpr string(const Ct (&s)[N]) : std::basic_string<Ct>(s, N) {}
  template<iterator_of<Ct> It> constexpr string(It i, It s) : std::basic_string<Ct>(mv(i), mv(s)) {}
  template<iterator_of<Ct> It, sentinel_for<It> Se> requires(!same_as<It, Se>) constexpr string(It i, Se s)
    : std::basic_string<Ct>(std::common_iterator<It, Se>(mv(i)), std::common_iterator<It, Se>(mv(s))) {}
  template<range_of<Ct> Rg> requires(!convertible_to<Rg, std::basic_string<Ct>> && !is_array<Rg>)
  constexpr string(Rg&& r) : string(yw::begin(r), yw::end(r)) {}
};

template<character Ct> string(natt, Ct) -> string<Ct>;
template<character Ct> string(const Ct*) -> string<Ct>;
template<character Ct> string(const Ct*, natt) -> string<Ct>;
template<iterator It, sentinel_for<It> Se> string(It, Se) -> string<iter_value<It>>;
template<range Rg> string(Rg&&) -> string<iter_value<Rg>>;

using str1 = string<cat1>;
using str2 = string<cat2>;
constexpr str1 operator""_s(const cat1* s, natt n) noexcept { return str1(s, n); }
constexpr str2 operator""_s(const cat2* s, natt n) noexcept { return str2(s, n); }


#pragma region stov, vtos

template<arithmetic T> struct t_stov {
  T operator()(str1 Str) const noexcept { return call(mv(Str)); }
  T operator()(str2 Str) const noexcept { return call(mv(Str)); }
private:
  static auto call(auto Str) noexcept {
    for (; Str.size(); Str.erase(0, 1)) {
      try {
        if constexpr (same_as<remove_cv<T>, fat8>) return std::stod(Str);
        else if constexpr (same_as<remove_cv<T>, fat4>) return std::stof(Str);
        else if constexpr (same_as<remove_cv<T>, ld_t>) return std::stold(Str);
        else if constexpr (same_as<remove_cv<T>, int8>) return std::stoll(Str);
        else if constexpr (same_as<remove_cv<T>, int4>) return std::stoi(Str);
        else if constexpr (same_as<remove_cv<T>, sl_t>) return std::stol(Str);
        else if constexpr (included_in<remove_cv<T>, int2, int1>) return static_cast<T>(std::stoi(Str));
        else if constexpr (same_as<remove_cv<T>, nat8>) return std::stoull(Str);
        else if constexpr (same_as<remove_cv<T>, nat4>) return static_cast<T>(std::stoul(Str));
        else if constexpr (same_as<remove_cv<T>, ul_t>) return std::stoul(Str);
        else return static_cast<T>(std::stoul(Str));
      } catch (const std::exception&) { continue; }
    }
    return T{};
  }
};

template<arithmetic T> inline constexpr t_stov<T> stov;
template<included_in<cat1, cat2> Ct> inline constexpr auto vtos = []<arithmetic T>(const T v) noexcept {
  if constexpr (same_as<Ct, cat1>) return std::to_string(v); else return std::to_wstring(v); };
inline constexpr overload strlen{
  [](const character auto* const Str) { return std::char_traits<cat1>::length(Str); },
  []<range Rg>(Rg&& Str) requires character<iter_value<Rg>> { return std::ranges::distance(Str); }};

#pragma endregion stov, vtos


/// source location class
struct source {
  nat4 line, column;
  stv1 file, func;
  constexpr source(nat4 Line = __builtin_LINE(), nat4 Column = __builtin_COLUMN(), stv1 File = __builtin_FILE(),
                   stv1 Func = __builtin_FUNCTION()) noexcept : line(Line), column(Column), file(mv(File)), func(mv(Func)) {}

  /// outputs the source location to the stream
  template<typename Tr> friend std::basic_ostream<cat1, Tr>&
  operator<<(std::basic_ostream<cat1, Tr>& os, const source& s) {
    return os << std::format("{}({},{})", s.file, s.line, s.column); }
};


/// exception class which contains the source location
struct except : public std::exception {
  explicit except(const std::string& s, source _ = {}) noexcept : except(s.data(), mv(_)){};
  explicit except(const cat1* s, source _ = {}) noexcept : std::exception(std::format("{}\n->{}", s, _).data()) {}
  explicit except(const std::exception& Base, source _ = {}) noexcept : except(Base.what(), mv(_)){};
};


/// class for file path derived from `std::filesystem::path`
class path : public std::filesystem::path {
public:
  path() noexcept = default;
  path(const std::filesystem::path& p) : std::filesystem::path(p) {}
  path(std::filesystem::path&& p) noexcept : std::filesystem::path(mv(p)) {}
  path(string_type&& s) : std::filesystem::path(mv(s)) {}
  template<range Rg> requires character<iter_value<Rg>> path(Rg&& r) : std::filesystem::path(yw::begin(r), yw::end(r)) {}
  template<character Ct> path(const Ct* s) : std::filesystem::path(s) {}
  bool exists() const { return std::filesystem::exists(*this); }
  bool is_file() const { return std::filesystem::is_regular_file(*this); }
  bool is_directory() const { return std::filesystem::is_directory(*this); }
  nat8 file_size() const { return std::filesystem::file_size(*this); }
  array<path> list(bool Recursive = false) const {
    array<path> A;
    if (Recursive)
      for (const auto& E : std::filesystem::recursive_directory_iterator(*this)) A.push_back(E.path());
    else
      for (const auto& E : std::filesystem::directory_iterator(*this)) A.push_back(E.path());
    return A;
  }
  void read(cnt_range auto& Out) const requires(!is_const<decltype(yw::data(Out))>) {
    if (const auto n = yw::size(Out) * sizeof(iter_value<decltype(Out)>); file_size() == n) {
      if (std::ifstream ifs(*this, std::ios::binary); ifs) ifs.read(reinterpret_cast<char*>(yw::data(Out)), n);
      else throw except("failed to open the file");
    } else throw except("byte size of `Out` is not equal to the file size");
  }
  void write(cnt_range auto&& In) const { write(yw::data(In), yw::size(In) * sizeof(iter_value<decltype(In)>)); }
  void write(const void* In, natt Size) const {
    if (std::ofstream ofs(*this, std::ios_base::binary); ofs) ofs.write(reinterpret_cast<const char*>(In), Size);
    else throw except("failed to open the file");
  }
  void remove() const { std::filesystem::remove(*this); }
};


/// class for calling the specified function object when itself is evaluated
template<invocable... Fs> struct caster : public Fs... {
  template<typename T> static constexpr natt i = []<typename... Ts>(typepack<Ts...>) { return inspects<same_as<Ts, T>...>; }(typepack<invoke_result<Fs>...>{});
  template<typename T> static constexpr natt j = i<T> < sizeof...(Fs) ? i<T> : []<typename... Ts>(typepack<Ts...>) { return inspects<convertible_to<Ts, T>...>; }(typepack<invoke_result<Fs>...>{});
public:
  template<typename T> requires(j<T> < sizeof...(Fs)) constexpr operator T() const
    noexcept(nt_convertible_to<invoke_result<type_switch<j<T>, Fs...>>, T>) { return type_switch<j<T>, Fs...>::operator()(); }
};


/// checks if the current code is being evaluated at compile time
inline constexpr caster is_cev{[]() noexcept { return std::is_constant_evaluated(); }};


/// for each function for tuples
constexpr auto cfor = []<typename Fn, natt... Is, typename... Ts>(Fn&& Func, sequence<Is...>, Ts&&... Args)
  requires (invocable<Fn&, constant<Is>, Ts...> && ...) { ((invoke(Func, constant<Is>{}, fwd<Ts>(Args)...)), ...); };


template<bool Max> struct t_maxi {
private:
  template<natt I, typename Tp, typename R> static constexpr auto call(Tp&& L, R M) {
    if constexpr (I == extent<Tp>) return mv(M);
    else { if constexpr (Max) { if (M.second < get<I>(L)) return call<I + 1>(fwd<Tp>(L), R{I, get<I>(L)});
                                else return call<I + 1>(fwd<Tp>(L), mv(M));
           } else             { if (M.second > get<I>(L)) return call<I + 1>(fwd<Tp>(L), R{I, get<I>(L)});
                                else return call<I + 1>(fwd<Tp>(L), mv(M)); } } }
public:
  constexpr list<natt, none> operator()() const noexcept { return list<natt, none>{}; }
  template<typename T> constexpr list<natt, remove_ref<T>> operator()(T&& t) const noexcept { return {0, fwd<T>(t)}; }
  template<typename T0, typename T1, typename... Ts>
  constexpr list<natt, remove_ref<common_type<T0, T1, Ts...>>> operator()(T0&& t0, T1&& t1, Ts&&... ts) const {
    static_assert(extent<list<T0&, T1&, Ts&...>> > 0);
    return call<1>(list<>::asref(t0, t1, ts...),
                   list<natt, remove_ref<common_type<T0, T1, Ts...>>>{0, fwd<T0>(t0)}); }
};

inline constexpr t_maxi<1> maxi;
inline constexpr t_maxi<0> mini;

} // clang-format on

namespace std { // clang-format off
template<typename T> struct common_type<yw::none, T> : type_identity<yw::none> {};
template<typename T> struct common_type<T, yw::none> : type_identity<yw::none> {};
template<yw::arithmetic T> struct common_type<T, yw::value> : type_identity<yw::value> {};
template<yw::arithmetic T> struct common_type<yw::value, T> : type_identity<yw::value> {};
template<auto... Vs> struct tuple_size<yw::sequence<Vs...>> : integral_constant<size_t, sizeof...(Vs)> {};
template<size_t I, auto... Vs> struct tuple_element<I, yw::sequence<Vs...>> : type_identity<decltype(yw::value_switch<I, Vs...>)> {};
template<typename... Ts> struct tuple_size<yw::typepack<Ts...>> : integral_constant<size_t, sizeof...(Ts)> {};
template<size_t I, typename... Ts> struct tuple_element<I, yw::typepack<Ts...>> : type_identity<yw::type_switch<I, Ts...>> {};
template<typename... Ts> struct tuple_size<yw::list<Ts...>> : integral_constant<size_t, sizeof...(Ts)> {};
template<size_t I, typename... Ts> struct tuple_element<I, yw::list<Ts...>> : type_identity<yw::type_switch<I, Ts...>> {};
template<typename T, typename Pj, typename Sq> struct tuple_size<yw::projector<T, Pj, Sq>> : integral_constant<size_t, yw::to_sequence<Sq>::count> {};
template<size_t I, typename T, typename Pj, typename Sq> struct tuple_element<I, yw::projector<T, Pj, Sq>> : remove_cvref<yw::element_t<yw::projector<T, Pj, Sq>, I>> {};
template<typename T, size_t N> struct tuple_size<yw::array<T, N>> : integral_constant<size_t, N> {};
template<size_t I, typename T, size_t N> struct tuple_element<I, yw::array<T, N>> : type_identity<T> {};
template<typename Ct> struct formatter<yw::string_view<Ct>, Ct> : formatter<basic_string_view<Ct>, Ct> {
  auto format(const yw::string_view<Ct>& s, yw::type_switch<same_as<Ct, char>, format_context, wformat_context>& ctx) const {
    return formatter<basic_string_view<Ct>, Ct>::format(reinterpret_cast<const std::basic_string_view<Ct>&>(s), ctx); } };
template<typename Ct> struct formatter<yw::string<Ct>, Ct> : formatter<basic_string<Ct>, Ct> {
  auto format(const yw::string<Ct>& s, yw::type_switch<same_as<Ct, char>, format_context, wformat_context>& ctx) const {
    return formatter<basic_string<Ct>, Ct>::format(s, ctx); } };
template<> struct formatter<yw::source> : formatter<string> {
  auto format(const yw::source& s, format_context& ctx) const {
    return formatter<string>::format(std::format("{}({},{})", s.file, s.line, s.column), ctx); } };
} // clang-format on
