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

/// class representing the absence of a value
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

/// class holding a type and a constant value
template<typename T, auto V = none{}> struct trait {
  using type = T;
  static constexpr auto value = V;
  constexpr operator auto() const noexcept { return value; }
  constexpr auto operator()() const noexcept { return value; }
};

/// template alias representing the type
template<typename T> using type = typename trait<T>::type;

/// checks if `T` is a valid type
template<typename T> concept valid = requires { typename type<T>; };

/// checks if all of `Ts` are same as `T`
template<typename T, typename... Ts> concept same_as = (std::same_as<T, Ts> && ...);

/// checks if any of `Ts` are same as `T`
template<typename T, typename... Ts> concept included_in = (std::same_as<T, Ts> || ...);

/// checks if `T` is derived from all of `Ts`
template<typename T, typename... Ts> concept derived_from = (std::derived_from<T, Ts> && ...);

/// checks if `T` can be explicitly converted to all of `Ts`
template<typename T, typename... Ts> concept castable_to = requires(T (&f)()) { ((static_cast<Ts>(f())), ...); };

/// checks if `T` can be explicitly converted to all of `Ts` without throwing exceptions
template<typename T, typename... Ts> concept nt_castable_to = requires(T (&f)() noexcept) {
  requires castable_to<T, Ts...>;
  { ((static_cast<Ts>(f())), ...) } noexcept; };

/// chekcs if `T` can be implicitly converted to all of `Ts`
template<typename T, typename... Ts> concept convertible_to = requires {
  requires castable_to<T, Ts...>;
  requires(std::convertible_to<T, Ts> && ...); };

/// checks if `T` can be implicitly converted to all of `Ts` without throwing exceptions
template<typename T, typename... Ts> concept nt_convertible_to = requires {
  requires nt_castable_to<T, Ts...>;
  requires convertible_to<T, Ts...>;
  (std::is_nothrow_convertible_v<T, Ts> && ...); };

/// class representing a constant value
template<auto V, typename T = decltype(V)>
requires convertible_to<decltype(V), T>
struct constant : trait<T, static_cast<T>(V)> {};

/// checks if `T` is const
template<typename T> concept is_const = std::is_const_v<T>;

/// checks if `T` is volatile
template<typename T> concept is_volatile = std::is_volatile_v<T>;

/// checks if `T` is const volatile
template<typename T> concept is_cv = is_const<T> && is_volatile<T>;

/// checks if `T` is a lvalue reference
template<typename T> concept is_lvref = std::is_lvalue_reference_v<T>;

/// checks if `T` is a rvalue reference
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

namespace _ { // clang-format off
template<typename T> struct _memptr;
template<typename M, typename C> struct _memptr<M C::*> { using m = M; using c = C; };
template<typename M, typename C> struct _memptr<M C::* const> { using m = M; using c = C; };
template<typename M, typename C> struct _memptr<M C::* volatile> { using m = M; using c = C; };
template<typename M, typename C> struct _memptr<M C::* const volatile> { using m = M; using c = C; };
} // clang-format on

/// checks if `T` is a member pointer
template<typename T> concept is_memptr = is_pointer<T> && std::is_member_pointer_v<T>;

/// gets the member type of a member pointer
template<is_memptr T> using member_type = typename _::_memptr<T>::m;

/// gets the class type of a member pointer
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

/// checks if `T` is trivial
template<typename T> concept trivial = std::is_trivially_copyable_v<T>;

namespace _ { // clang-format off
template<typename T> struct _remove_all_pointers { using t = T; };
template<typename T> struct _remove_all_pointers<T*> : _remove_all_pointers<T> {};
template<typename T> struct _remove_all_pointers<T* const> : _remove_all_pointers<T> {};
template<typename T> struct _remove_all_pointers<T* volatile> : _remove_all_pointers<T> {};
template<typename T> struct _remove_all_pointers<T* const volatile> : _remove_all_pointers<T> {};
} // clang-format on

/// removes const and volatile qualifiers from `T`
template<typename T> using remove_cv = std::remove_cv_t<T>;

/// removes const qualifier from `T`
template<typename T> using remove_const = std::remove_const_t<T>;

/// removes volatile qualifier from `T`
template<typename T> using remove_volatile = std::remove_volatile_t<T>;

/// removes reference from `T`
template<typename T> using remove_ref = std::remove_reference_t<T>;

/// removes const, volatile and reference from `T`
template<typename T> using remove_cvref = std::remove_cvref_t<T>;

/// removes pointer from `T`
template<typename T> using remove_pointer = std::remove_pointer_t<T>;

/// removes all pointers from `T`
template<typename T> using remove_all_pointers = typename _::_remove_all_pointers<T>::t;

/// removes extent from `T`
template<typename T> using remove_extent = std::remove_extent_t<T>;

/// removes all extents from `T`
template<typename T> using remove_all_extents = std::remove_all_extents_t<T>;

/// decays `T`
template<typename T> using decay = std::decay_t<T>;

/// checks if `T` is `void`
template<typename T> concept is_void = same_as<remove_cv<T>, void>;

/// checks if `T` is `bool`
template<typename T> concept is_bool = same_as<remove_cv<T>, bool>;

/// checks if `T` is `none`
template<typename T> concept is_none = same_as<remove_cv<T>, none>;

/// checks if `T` is a character type
template<typename T> concept is_cat = included_in<remove_cv<T>, cat1, cat2>;

/// checks if `T` is a character type
template<typename T> concept is_uct = included_in<remove_cv<T>, uct1, uct2, uct4>;

/// checks if `T` is an integer type
template<typename T> concept is_int = included_in<remove_cv<T>, int1, int2, int4, int8, sl_t>;

/// checks if `T` is a natural number type
template<typename T> concept is_nat = included_in<remove_cv<T>, nat1, nat2, nat4, nat8, ul_t>;

/// checks if `T` is a floating point type
template<typename T> concept is_fat = included_in<remove_cv<T>, fat4, fat8, ld_t>;

/// checks if `T` is a character type
template<typename T> concept character = is_cat<T> || is_uct<T>;

/// checks if `T` is an integral type
template<typename T> concept integral = is_bool<T> || character<T> || is_int<T> || is_nat<T>;

/// checks if `T` is an unsigned integral type
template<typename T> concept unsigned_integral = integral<T> && std::unsigned_integral<T>;

/// checks if `T` is an arithmetic type
template<typename T> concept arithmetic = integral<T> || std::floating_point<T>;

namespace _ { // clang-format off
template<typename T> struct _add_cv { using c = const T; using v = volatile T; using cv = const volatile T; };
template<typename T> struct _add_cv<T&> { using c = const T&; using v = volatile T&; using cv = const volatile T&; };
template<typename T> struct _add_cv<T&&> { using c = const T&&; using v = volatile T&&; using cv = const volatile T&&; };
template<typename T> struct _add_pointer { using t = T; };
template<typename T> requires valid<T*> struct _add_pointer<T> { using t = T*; };
template<typename T> requires valid<T*> struct _add_pointer<T&> { using t = T*&; };
template<typename T> requires valid<T*> struct _add_pointer<T&&> { using t = T*&&; };
template<typename T, natt N> struct _add_extent { using t = T; };
template<typename T> requires valid<T[]> struct _add_extent<T, 0> { using t = T[]; };
template<typename T> requires valid<T[]> struct _add_extent<T&, 0> { using t = T (&)[]; };
template<typename T> requires valid<T[]> struct _add_extent<T&&, 0> { using t = T (&&)[]; };
template<typename T, natt N> requires valid<T[N]> struct _add_extent<T, N> { using t = T[N]; };
template<typename T, natt N> requires valid<T[N]> struct _add_extent<T&, N> { using t = T (&)[N]; };
template<typename T, natt N> requires valid<T[N]> struct _add_extent<T&&, N> { using t = T (&&)[N]; };
} // clang-format on

/// adds const and volatile qualifiers to `T`
template<typename T> using add_cv = typename _::_add_cv<T>::cv;

/// adds const qualifier to `T`
template<typename T> using add_const = typename _::_add_cv<T>::c;

/// adds volatile qualifier to `T`
template<typename T> using add_volatile = typename _::_add_cv<T>::v;

/// adds lvalue reference to `T`
template<typename T> using add_lvref = std::add_lvalue_reference_t<T>;

/// adds rvalue reference to `T`
template<typename T> using add_rvref = std::add_rvalue_reference_t<remove_ref<T>>;

/// adds forwarding reference to `T`
template<typename T> using add_fwref = std::add_rvalue_reference_t<T>;

/// adds pointer to `T`
template<typename T> using add_pointer = typename _::_add_pointer<T>::t;

/// adds extent to `T`
template<typename T, natt N> using add_extent = typename _::_add_extent<T, N>::t;

/// gets the address of `Ref`
inline constexpr auto addressof = []<is_lvref T>(T&& Ref) noexcept { return std::addressof(Ref); };

/// adds const qualifier to `Ref`
inline constexpr auto asconst = []<typename T>(T&& Ref) noexcept -> add_const<T&&> { return Ref; };

/// casts `Ref` to rvalue reference
inline constexpr auto mv = []<typename T>(T&& Ref) noexcept
  -> decltype(auto) { return static_cast<add_rvref<T>>(Ref); };

/// casts `Ref` to forwarding reference
template<typename T> inline constexpr auto fwd = [](auto&& Ref) noexcept
  -> decltype(auto) { return static_cast<T&&>(Ref); };

/// gets a reference to an object of type `T`
template<typename T> inline constexpr auto declval = []() noexcept -> add_fwref<T> {};

/// bit-casts `Src` to the type `T`
template<trivial T> inline constexpr auto bitcast = [](const trivial auto& Src) noexcept
  requires(sizeof(T) == sizeof(Src)) { return std::bit_cast<T>(Src); };

/// rounds `A` up to the nearest power of 2
inline constexpr auto bitceil = [](const unsigned_integral auto& A) noexcept { return std::bit_ceil(A); };

/// rounds `A` down to the nearest power of 2
inline constexpr auto bitfloor = [](const unsigned_integral auto& A) noexcept { return std::bit_floor(A); };

/// counts the number of 1 bits in `A`
inline constexpr auto popcount = [](const unsigned_integral auto& A) noexcept { return std::popcount(A); };

/// checks if `T` is destructible
template<typename T> concept destructible = std::is_destructible_v<T>;

/// checks if `T` is nothrow destructible
template<typename T> concept nt_destructible = destructible<T> && std::destructible<T>;

/// checks if `T` is assignable from `A`
template<typename T, typename A> concept assignable = std::is_assignable_v<T, A>;

/// checks if `T` is constructible from `As`
template<typename T, typename... As> concept constructible = requires { T{declval<As>()...}; };

/// checks if `T` is nothrow assignable from `A`
template<typename T, typename A> concept nt_assignable = requires {
  requires assignable<T, A>;
  requires std::is_nothrow_assignable_v<T, A>; };

/// checks if `T` is nothrow constructible from `As`
template<typename T, typename... As> concept nt_constructible = requires {
  requires constructible<T, As...>;
  { T{declval<As>()...} } noexcept; };

/// sets the value of `B` to `A` and returns the previous value of `A`
inline constexpr auto exchange = []<typename T, typename U = T>(T & A, U&& B) noexcept(
  nt_constructible<T, T> && nt_assignable<T&, U>) requires constructible<T, T> && assignable<T&, U> {
  if (auto a = mv(A); 1) return A = fwd<U>(B), mv(a);
};

namespace _ { // clang-format off
template<typename T, T I, typename... Ts> struct _type_switch : _type_switch<natt, I, Ts...> {};
template<bool B, typename Tt, typename Tf> struct _type_switch<bool, B, Tt, Tf> : std::conditional<B, Tt, Tf> {};
template<natt I, typename T, typename... Ts> struct _type_switch<natt, I, T, Ts...> : _type_switch<natt, I - 1, Ts...> {};
template<typename T, typename... Ts> struct _type_switch<natt, 0, T, Ts...> : trait<T> {};
} // clang-format on

// clang-format off

/// selects the type at index `I` from `Ts`
template<castable_to<natt> auto I, typename... Ts>
requires ((is_bool<decltype(I)> && sizeof...(Ts) == 2) ||
          (!is_bool<decltype(I)> && static_cast<natt>(I) < sizeof...(Ts)))
using type_switch = typename _::_type_switch<decltype(I), I, Ts...>::type;

/// selects the value at index `I` from `Vs`
template<castable_to<natt> auto I, auto... Vs>
requires ((is_bool<decltype(I)> && sizeof...(Vs) == 2) ||
          (!is_bool<decltype(I)> && static_cast<natt>(I) < sizeof...(Vs)))
inline constexpr auto value_switch = type_switch<I, constant<Vs>...>::value;

/// selects the parameter at index `I`
template<castable_to<natt> auto I> inline constexpr auto parameter_switch =
  []<typename T, typename... Ts>(T&& t, Ts&&... ts) noexcept -> decltype(auto)
requires ((is_bool<decltype(I)> && sizeof...(Ts) == 1) ||
          (!is_bool<decltype(I)> && static_cast<natt>(I) <= sizeof...(Ts))) {
  if constexpr (is_bool<decltype(I)>) return parameter_switch<natt(!I)>(fwd<T>(t), fwd<Ts>(ts)...);
  else if constexpr (same_as<decltype(I), natt>) {
    if constexpr (I == 0) return fwd<T>(t);
    else return parameter_switch<I - 1>(fwd<Ts>(ts)...);
  } else return parameter_switch<natt(I)>(fwd<T>(t), fwd<Ts>(ts)...);
};

/// function object that returns the parameter passed to it
struct pass { template<typename T> constexpr T&& operator()(T&& a) const noexcept { return static_cast<T&&>(a); } };

/// function object that overloads multiple functions
template<typename... Fs> struct overload : public Fs... { using Fs::operator()...; };

// clang-format on

/// counts the number of `true` values in `Bs`
template<bool... Bs> inline constexpr natt counts = 0;
template<bool B, bool... Bs> inline constexpr natt counts<B, Bs...> = B ? 1 + counts<Bs...> : counts<Bs...>;

/// gets the index of the first `true` value in `Bs`
template<bool... Bs> inline constexpr natt inspects = 0;
template<bool B, bool... Bs> inline constexpr natt inspects<B, Bs...> = B ? 0 : 1 + inspects<Bs...>;

/// copies const qualifier from `Src` to `T`
template<typename T, typename Src> using copy_const = type_switch<is_const<Src>, add_const<T>, T>;

/// copies volatile qualifier from `Src` to `T`
template<typename T, typename Src> using copy_volatile = type_switch<is_volatile<Src>, add_volatile<T>, T>;

/// copies const and volatile qualifiers from `Src` to `T`
template<typename T, typename Src> using copy_cv = copy_const<copy_volatile<T, Src>, Src>;

namespace _ { // clang-format off
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
} // clang-format on

/// copies reference from `Src` to `T`
template<typename T, typename Src> using copy_reference = typename _::_copy_reference<T, Src>::type;

/// copies const, volatile and reference from `Src` to `T`
template<typename T, typename Src> using copy_cvref = copy_reference<copy_cv<T, remove_ref<Src>>, Src>;

/// copies extent from `Src` to `T`
template<typename T, typename Src> using copy_extent = typename _::_copy_extent<T, Src>::type;

/// copies pointer from `Src` to `T`
template<typename T, typename Src> using copy_pointer = type_switch<is_pointer<T>, copy_cv<add_pointer<T>, Src>, T>;

/// checks if `F` is invocable with `As`
template<typename F, typename... As> concept invocable = std::invocable<F, As...>;

/// checks if `F` is nothrow invocable with `As`
template<typename F, typename... As> concept nt_invocable =
  invocable<F, As...> && std::is_nothrow_invocable_v<F, As...>;

/// gets the result type of invoking `F` with `As`
template<typename F, typename... As> using invoke_result_t =
  type_switch<is_void<std::invoke_result_t<F, As...>>, none, std::invoke_result_t<F, As...>>;

/// checks if `F` is invocable with `As` and the result is convertible to `R`
template<typename F, typename R, typename... As> concept invocable_r =
  invocable<F, As...> && convertible_to<invoke_result_t<F, As...>, R>;

/// checks if `F` is nothrow invocable with `As` and the result is nothrow convertible to `R`
template<typename F, typename R, typename... As> concept nt_invocable_r =
  nt_invocable<F, As...> && nt_convertible_to<invoke_result_t<F, As...>, R>;

/// checks if `F` is a predicate; i.e. the result of invoking `F` with `As` is convertible to `bool`
template<typename F, typename... As> concept predicate = invocable_r<F, bool, As...>;

// clang-format off

/// invokes `Func` with `Args`
inline constexpr auto invoke = []<typename F, typename... As>(F&& Func, As&&... Args)
  noexcept(nt_invocable<F, As...>) -> invoke_result_t<F, As...> requires invocable<F, As...> {
  if constexpr (is_void<std::invoke_result_t<F, As...>>) return std::invoke(fwd<F>(Func), fwd<As>(Args)...), none{};
  else return std::invoke(fwd<F>(Func), fwd<As>(Args)...);
};

/// invokes `Func` with `Args` and converts the result to `R`
template<typename R> inline constexpr auto invoke_r = []<typename F, typename... As>(F&& f, As&&... as)
  noexcept(nt_invocable_r<F, R, As...>) -> R requires invocable_r<F, R, As...> {
  return static_cast<R>(invoke(fwd<F>(f), fwd<As>(as)...));
};

// clang-format on

namespace _ { // clang-format off
template<typename... Ts> struct _common_type : trait<none> {};
template<typename... Ts> requires valid<std::common_reference_t<Ts...>>
struct _common_type<Ts...> : trait<std::common_reference_t<Ts...>> {};
template<class T, template<class...> class Tm> struct _specialization_of : constant<false> {};
template<template<class...> class Tm, class... Ts> struct _specialization_of<Tm<Ts...>, Tm> : constant<true> {};
template<class T, class U> struct _variation_of : constant<false> {};
template<template<class...> class Tm, class... Ts, class... Us> struct _variation_of<Tm<Ts...>, Tm<Us...>> : constant<true> {};
template<template<auto...> class Tm, auto... Vs, auto... Ws> struct _variation_of<Tm<Vs...>, Tm<Ws...>> : constant<true> {};
template<template<class, auto...> class Tm, class T, class U, auto... Vs, auto... Ws> struct _variation_of<Tm<T, Vs...>, Tm<U, Ws...>> : constant<true> {};
template<template<auto, class...> class Tm, auto V, auto W, class... Ts, class... Us> struct _variation_of<Tm<V, Ts...>, Tm<W, Us...>> : constant<true> {};
} // clang-format on

/// gets the common type of `Ts`
template<typename... Ts> using common_type = typename _::_common_type<Ts...>::type;

/// checks if `Ts` have a common type
template<typename... Ts> concept common_with = !is_none<common_type<Ts...>> &&
                                               (std::common_reference_with<common_type<Ts...>, Ts> && ...);

/// checks if `T` is a specialization of `Tm`
template<typename T, template<typename...> typename Tm> concept specialization_of = _::_specialization_of<T, Tm>::value;

/// checks if `T` is a variation of `U`
/// @note `variation_of<list<int, bool>, list<short, float, vector<char>>> == true`
template<typename T, typename U> concept variation_of = _::_variation_of<T, U>::value;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// not operation
inline constexpr auto nt = [](const auto& A) noexcept(noexcept(!A)) -> convertible_to<bool> auto { return !A; };

/// equality comparison
inline constexpr auto eq = [](const auto& A, const auto& B) noexcept(noexcept(A == B)) -> convertible_to<bool> auto { return A == B; };

/// inequality comparison
inline constexpr auto ne = [](const auto& A, const auto& B) noexcept(noexcept(A != B)) -> convertible_to<bool> auto { return A != B; };

/// less than comparison
inline constexpr auto lt = [](const auto& A, const auto& B) noexcept(noexcept(A < B, 1 > 0)) -> convertible_to<bool> auto { return A < B; };

/// greater than comparison
inline constexpr auto gt = [](const auto& A, const auto& B) noexcept(noexcept(A > B)) -> convertible_to<bool> auto { return A > B; };

/// less than or equal comparison
inline constexpr auto le = [](const auto& A, const auto& B) noexcept(noexcept(A <= B, 1 > 0)) -> convertible_to<bool> auto { return A <= B; };

/// greater than or equal comparison
inline constexpr auto ge = [](const auto& A, const auto& B) noexcept(noexcept(A >= B)) -> convertible_to<bool> auto { return A >= B; };

/// negation
inline constexpr auto neg = []<typename T>(const T& A) noexcept(noexcept(-A)) -> same_as<T> auto { return -A; };

/// addition
inline constexpr auto add = []<typename T, typename U>(const T& A, const U& B) noexcept(noexcept(A + B)) -> common_type<T, U> { return A + B; };

/// subtraction
inline constexpr auto sub = []<typename T, typename U>(const T& A, const U& B) noexcept(noexcept(A - B)) -> common_type<T, U> { return A - B; };

/// multiplication
inline constexpr auto mul = []<typename T, typename U>(const T& A, const U& B) noexcept(noexcept(A * B)) -> common_type<T, U> { return A * B; };

/// division
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

/// obtains the minimum value of the arguments
inline constexpr t_min min;

/// obtains the maximum value of the arguments
inline constexpr t_max max;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// clang-format off

/// generic value type that can be used for non-type template parameters
struct value {
  fat8 _cpp_double{};

  /// default constructor
  constexpr value() noexcept = default;

  /// constructs from `none`; sets the value to NaN
  explicit constexpr value(none) noexcept : _cpp_double(bitcast<fat8>(0x7ff8000000000001)) {}

  /// constructs from a arithmetic value
  template<arithmetic T> constexpr value(T V) noexcept : _cpp_double(static_cast<fat8>(V)) {}

  /// constructs from a value that can be casted to `fat8`
  template<castable_to<fat8> T> requires(!arithmetic<remove_cvref<T>>)
  constexpr value(T&& V) noexcept(nt_castable_to<T, fat8>) : _cpp_double(static_cast<fat8>(fwd<T>(V))) {}

  /// assigns `none` to the value; sets the value to NaN
  constexpr value& operator=(none) noexcept { return _cpp_double = bitcast<fat8>(0x7ff8000000000001), *this; }

  /// assigns a value that can be casted to `fat8`
  template<castable_to<fat8> T> constexpr value& operator=(T&& V)
    noexcept { return _cpp_double = static_cast<fat8>(fwd<T>(V)), *this; }

  /// `value` can be casted to any type that can be casted from `fat8`
  constexpr operator const fat8&() const noexcept { return _cpp_double; }

  /// adds a value to this
  template<castable_to<fat8> T> friend constexpr value& operator+=(value& L, T&& R)
    noexcept { return L._cpp_double += static_cast<fat8>(fwd<T>(R)), L; }

  /// subtracts a value from this
  template<castable_to<fat8> T> friend constexpr value& operator-=(value& L, T&& R)
  noexcept { return L._cpp_double -= static_cast<fat8>(fwd<T>(R)), L; }

  /// multiplies this by a value
  template<castable_to<fat8> T> friend constexpr value& operator*=(value& L, T&& R)
  noexcept { return L._cpp_double *= static_cast<fat8>(fwd<T>(R)), L; }

  /// divides this by a value
  template<castable_to<fat8> T> friend constexpr value& operator/=(value& L, T&& R)
  noexcept { return L._cpp_double /= static_cast<fat8>(fwd<T>(R)), L; }
};

// clang-format on

/// maximum value for natt
inline constexpr natt npos(-1);

/// speed of light in vacuum
inline constexpr value c = 299792458.0;

/// Euler's number
inline constexpr value e = std::numbers::e_v<fat8>;

/// pi
inline constexpr value pi = std::numbers::pi_v<fat8>;

/// golden ratio
inline constexpr value phi = std::numbers::phi_v<fat8>;

/// Euler-Mascheroni constant
inline constexpr value gamma = std::numbers::egamma_v<fat8>;

/// natural logarithm of 2
inline constexpr value ln2 = std::numbers::ln2_v<fat8>;

/// natural logarithm of 10
inline constexpr value ln10 = std::numbers::ln10_v<fat8>;

/// square root of 2
inline constexpr value sqrt2 = std::numbers::sqrt2_v<fat8>;

/// square root of 3
inline constexpr value sqrt3 = std::numbers::sqrt3_v<fat8>;

/// positive infinity
inline constexpr value inf = std::numeric_limits<fat8>::infinity();

/// quiet NaN
inline constexpr value nan = std::numeric_limits<fat8>::quiet_NaN();

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

/// gets the iterator type of `Rg`
template<range Rg> using iterator_t = std::ranges::iterator_t<Rg>;

/// gets the borrowed iterator type of `Rg`
template<range Rg> using borrowed_iterator_t = std::ranges::borrowed_iterator_t<Rg>;

/// gets the sentinel type of `Rg`
template<range Rg> using sentinel_t = std::ranges::sentinel_t<Rg>;

/// checks if `It` is an output iterator
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

/// checks if `Rg` is an output range
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

/// checks if `It` is copyable from `In`
template<typename It, typename In> concept iter_copyable =
  iterator<It> && iterator<In> && std::indirectly_copyable<In, It>;

/// checks if `It` is movable from `In`
template<typename It, typename In> concept iter_movable =
  iterator<It> && iterator<In> && std::indirectly_movable<In, It>;

/// checks if `Fn` is unary invocable with the reference type of `It`
template<typename Fn, typename It> concept iter_unary_invocable =
  iterator<It> && std::indirectly_unary_invocable<Fn, It>;

/// checks if `Fn` is a unary predicate with the reference type of `It`
template<typename Fn, typename It> concept iter_unary_predicate =
  iterator<It> && std::indirect_unary_predicate<Fn, It>;

/// same as `std::ranges::begin`
inline constexpr auto begin = []<range Rg>(Rg&& r) ywlib_wrapper(std::ranges::begin(fwd<Rg>(r)));

/// same as `std::ranges::end`
inline constexpr auto end = []<range Rg>(Rg&& r) ywlib_wrapper(std::ranges::end(fwd<Rg>(r)));

/// same as `std::ranges::rbegin`
inline constexpr auto rbegin = []<range Rg>(Rg&& r) ywlib_wrapper(std::ranges::rbegin(fwd<Rg>(r)));

/// same as `std::ranges::rend`
inline constexpr auto rend = []<range Rg>(Rg&& r) ywlib_wrapper(std::ranges::rend(fwd<Rg>(r)));

/// same as `std::ranges::size`
inline constexpr auto size = []<range Rg>(Rg&& r) ywlib_wrapper(std::ranges::size(fwd<Rg>(r)));

/// same as `std::ranges::empty`
inline constexpr auto empty = []<range Rg>(Rg&& r) ywlib_wrapper(std::ranges::empty(fwd<Rg>(r)));

/// same as `std::ranges::data`
inline constexpr auto data = []<range Rg>(Rg&& r) ywlib_wrapper(std::ranges::data(fwd<Rg>(r)));

/// same as `std::ranges::iter_move`
inline constexpr auto iter_move = []<iterator It>(It&& i) ywlib_wrapper(std::ranges::iter_move(fwd<It>(i)));

/// same as `std::ranges::iter_swap`
inline constexpr auto iter_swap = []<iterator It, iterator Jt>(It&& i, Jt&& j)
  ywlib_wrapper(std::ranges::iter_swap(fwd<It>(i), fwd<Jt>(j)));

namespace _ { // clang-format off
template<typename T> struct _iter_t {};
template<range Rg> struct _iter_t<Rg> : _iter_t<iterator_t<Rg>> {};
template<iterator It> struct _iter_t<It> { using v = std::iter_value_t<It>; using d = std::iter_difference_t<It>;
                                           using r = std::iter_reference_t<It>; using rr = std::iter_rvalue_reference_t<It>; };
} // clang-format on

/// iterator value type
template<typename T> requires iterator<T> || range<T> using iter_value_t = typename _::_iter_t<T>::v;

/// iterator difference type
template<typename T> requires iterator<T> || range<T> using iter_difference_t = typename _::_iter_t<T>::d;

/// iterator reference type
template<typename T> requires iterator<T> || range<T> using iter_reference_t = typename _::_iter_t<T>::r;

/// iterator rvalue reference type
template<typename T> requires iterator<T> || range<T> using iter_rvref_t = typename _::_iter_t<T>::rr;

/// iterator common type
template<typename T> requires iterator<T> || range<T>
using iter_common_t = common_type<iter_reference_t<T>, iter_value_t<T>>;

/// checks if `iter_value_t<It>` is convertible to `U`
template<typename It, typename U> concept iterator_for = iterator<It> && convertible_to<iter_reference_t<It>, U>;

/// checks if `iter_value_t<It>` is the same as `U`
template<typename It, typename U> concept iterator_of = iterator_for<It, U> && same_as<iter_value_t<It>, U>;

/// checks if `cnt_iterator<It> && iterator_of<It, U>`
template<typename It, typename U> concept cnt_iterator_of = cnt_iterator<It> && iterator_of<It, U>;

/// checks if `iter_value_t<Rg>` is convertible to `U`
template<typename Rg, typename U> concept range_for = iterator_for<iterator_t<Rg>, U>;

/// checks if `iter_value_t<Rg>` is the same as `U`
template<typename Rg, typename U> concept range_of = iterator_of<iterator_t<Rg>, U>;

/// checks if `cnt_range<Rg> && range_of<Rg, U>`
template<typename Rg, typename U> concept cnt_range_of = cnt_range<Rg> && range_of<Rg, U>;

/// checks if `cnt_range<Rg> && sized_range<Rg>`
template<typename Rg> concept cnt_sized_range = cnt_range<Rg> && sized_range<Rg>;

/// checks if `cnt_range<Rg> && sized_range<Rg> && range_of<Rg, U>`
template<typename Rg, typename U> concept cnt_sized_range_of = cnt_sized_range<Rg> && range_of<Rg, U>;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// clang-format off

/// static array class if `N > 0`; otherwise, dynamic array class
template<typename T, natt N = 0> class array {
public:
  using value_type = T;
  T _cpp_array[N]{};

  /// size of the array
  static constexpr natt count = N;

  /// copies elements of `Range` to this
  template<range_for<T> Rg> constexpr array& operator=(Rg&& Range) {
    auto i = begin(Range); auto e = end(Range);
    for (natt j{}; j < N && i != e; ++j, ++i) _cpp_array[j] = *i;
  }

  /// conversion to reference to array
  constexpr operator add_lvref<T[N]>() noexcept { return _cpp_array; }

  /// conversion to const reference to array
  constexpr operator add_lvref<const T[N]>() const noexcept { return _cpp_array; }

  /// gets the element at index `i`
  constexpr T& operator[](natt i) { return _cpp_array[i]; }
  constexpr const T& operator[](natt i) const { return _cpp_array[i]; }

  /// gets the size of the array
  constexpr natt size() const noexcept { return N; }

  /// checks if the array is empty
  constexpr bool empty() const noexcept { return false; }

  /// gets the pointer to the first element
  constexpr T* data() noexcept { return _cpp_array; }
  constexpr const T* data() const noexcept { return _cpp_array; }

  /// gets the iterator to the first element
  constexpr T* begin() noexcept { return _cpp_array; }
  constexpr const T* begin() const noexcept { return _cpp_array; }

  /// gets the iterator to the last element
  constexpr T* end() noexcept { return _cpp_array + N; }
  constexpr const T* end() const noexcept { return _cpp_array + N; }

  /// gets the reference to the first element
  constexpr T& front() noexcept { return *_cpp_array; }
  constexpr const T& front() const noexcept { return *_cpp_array; }

  /// gets the reference to the last element
  constexpr T& back() noexcept { return _cpp_array[N - 1]; }
  constexpr const T& back() const noexcept { return _cpp_array[N - 1]; }

  /// gets the reference to the element at index `I`
  template<natt I> requires(I < N) constexpr T& get() & noexcept { return _cpp_array[I]; }
  template<natt I> requires(I < N) constexpr T&& get() && noexcept { return mv(_cpp_array[I]); }
  template<natt I> requires(I < N) constexpr const T& get() const& noexcept { return _cpp_array[I]; }
  template<natt I> requires(I < N) constexpr const T&& get() const&& noexcept { return mv(_cpp_array[I]); }
};

/// specialization for dynamic array derived from `std::vector`
template<typename T> class array<T, 0> : public std::vector<T> {
public:
  /// default constructor
  constexpr array() noexcept = default;

  /// moves `std::vector` to this
  constexpr array(std::vector<T>&& v) : std::vector<T>(mv(v)) {}

  /// constructs with `n` default-initialized elements
  constexpr explicit array(natt n) : std::vector<T>(n) {}

  /// constructs with `n` copies of `v`
  constexpr array(natt n, const T& v) : std::vector<T>(n, v) {}

  /// constructs from a range
  template<iterator_for<T> It> constexpr array(It i, It s) : std::vector<T>(i, s) {}
  template<iterator_for<T> It, sentinel_for<It> Se> requires(!same_as<It, Se>)
  constexpr array(It i, Se s) : std::vector<T>(std::common_iterator<It, Se>(i), std::common_iterator<It, Se>(s)) {}
  template<range_for<T> Rg> constexpr array(Rg&& r) : std::vector<T>(yw::begin(r), yw::end(r)) {}

  /// copies elements of `Range` to this
  template<range_for<T> Rg> constexpr array& operator=(Rg&& r) { return *this = array(fwd<Rg>(r)); }
};

/// deduction guide for `array`
template<typename T, convertible_to<T>... Ts> array(T, Ts...) -> array<T, 1 + sizeof...(Ts)>;
template<typename T> array(natt, const T&) -> array<T, 0>;
template<iterator It, sentinel_for<It> Se> array(It, Se) -> array<iter_value_t<It>, 0>;
template<range Rg> array(Rg&&) -> array<iter_value_t<Rg>, 0>;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<auto... Vs> struct sequence;
template<typename... Ts> struct typepack;
template<typename... Ts> struct list;

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

/// gets the element at index `I` from `Tuple`
template<natt I> inline constexpr auto get = []<typename Tp>(Tp&& Tuple)
  ywlib_wrapper(_::_get::call<I>(static_cast<Tp&&>(Tuple)));

/// checks if `Tp` is gettable at index `I`
template<typename Tp, natt I> concept gettable = requires { get<I>(declval<Tp>()); };

/// checks if `Tp` is nothrow gettable at index `I`
template<typename Tp, natt I> concept nt_gettable =
  requires { requires gettable<Tp, I>; requires noexcept(get<I>(declval<Tp>())); };

/// gets the element of `Tp` at index `I`
template<typename Tp, natt I> requires gettable<Tp, I> using element_t = decltype(get<I>(declval<Tp>()));

namespace _ { // clang-format off
template<typename T, typename U = remove_ref<T>, natt I = inspects<is_array<U>, requires { std::tuple_size<U>::value; }>>
inline constexpr natt _extent = type_switch<I, std::extent<U>, std::tuple_size<U>, constant<0_nn>>::value;
template<typename T, typename U, typename Sq> struct _tuple_for : constant<false> {};
template<typename T, typename U, natt... Is> struct _tuple_for<T, U, sequence<Is...>> : constant<(convertible_to<element_t<T, Is>, U> && ...)> {};
template<typename T, typename U, typename Sq> struct _nt_tuple_for : constant<false> {};
template<typename T, typename U, natt... Is> struct _nt_tuple_for<T, U, sequence<Is...>>
  : constant<(nt_gettable<T, Is> && ...) && (nt_convertible_to<element_t<T, Is>, U> && ...)> {};
template<typename S, typename T = none> struct _to_sequence;
template<template<auto...> typename Tm, typename T, auto... Vs> struct _to_sequence<Tm<Vs...>, T> : trait<sequence<static_cast<T>(Vs)...>> {};
template<template<typename, auto...> typename Tm, typename T, typename U, auto... Vs>
struct _to_sequence<Tm<U, Vs...>, T> : trait<sequence<static_cast<T>(Vs)...>> {};
template<template<auto...> typename Tm, auto... Vs> struct _to_sequence<Tm<Vs...>, none> : trait<sequence<Vs...>> {};
template<template<typename, auto...> typename Tm, typename U, auto... Vs> struct _to_sequence<Tm<U, Vs...>, none> : trait<sequence<Vs...>> {};
template<typename S, typename T, natt N> struct _indices_for : constant<false> {};
template<natt... Is, typename T, natt N> struct _indices_for<sequence<Is...>, T, N> : constant<(lt(Is, N) && ...)> {};
template<natt End, natt Begin = 0, invocable<natt> auto Proj = pass{}, natt... Vs>
struct _make_sequence : _make_sequence<End, Begin + 1, Proj, Vs..., Proj(Begin)> {};
template<natt End, auto Proj, natt... Vs> struct _make_sequence<End, End, Proj, Vs...> : trait<sequence<Vs...>> {};
template<natt I, natt N, typename S, typename... T> struct _cond_indices;
template<natt I, natt N, bool... Bs, natt... Is> struct _cond_indices<I, N, sequence<Bs...>, sequence<Is...>>
  : _cond_indices<I + 1, N, sequence<Bs...>, type_switch<value_switch<I, Bs...>, sequence<Is..., I>, sequence<Is...>>> {};
template<natt N, bool... Bs, natt... Is> struct _cond_indices<N, N, sequence<Bs...>, sequence<Is...>> : trait<sequence<Is...>> {};
template<typename T, typename S> struct _to_typepack;
template<typename T, natt... Is> struct _to_typepack<T, sequence<Is...>> : trait<typepack<element_t<T, Is>...>> {};
} // clang-format on

/// gets the extent of `Tp`
template<typename Tp> inline constexpr natt extent = _::_extent<Tp>;

/// checks if `Tp` is a tuple; i.e. has a non-zero extent
template<typename Tp> concept tuple = extent<Tp> != 0;

/// checks if `Tps` are tuples with the same extent
template<typename... Tps> concept same_size_tuples = requires {
  requires(tuple<Tps> && ...);
  requires((extent<Tps> == extent<type_switch<0, Tps...>>) && ...); };

/// converts a sequence `Sq` to `sequence` of its values
template<typename Sq, typename T = none> using to_sequence = typename _::_to_sequence<Sq, T>::type;

/// checks if `Sq` is a sequence
template<typename Sq, typename T = none> concept sequence_of = variation_of<to_sequence<Sq, T>, sequence<0>>;

/// checks if `Sq` is indices for a tuple `Tp`
template<typename Sq, typename Tp> concept indices_for = requires {
  requires tuple<Tp>;
  requires _::_indices_for<to_sequence<Sq, natt>, Tp, extent<Tp>>::value; };

/// makes a sequence
/// @note `make_sequence<1, 4, pass{}> -> sequence<pass{}(1), pass{}(2), pass{}(3)>`
template<natt End, natt Begin = 0, invocable<natt> auto Proj = pass{}> requires(Begin <= End)
using make_sequence = typename _::_make_sequence<End, Begin, Proj>::type;

/// makes indices for a tuple `Tp`
template<typename Tp> using make_indices_for = make_sequence<extent<Tp>>;

/// makes a sequence with indices of `true` in `Sq`
/// @note `make_indices<sequence<true, false, false, true, false>> -> sequence<0, 3>`
template<sequence_of<bool> Sq> using cond_indices =
  typename _::_cond_indices<0, extent<to_sequence<Sq, bool>>, to_sequence<Sq, bool>, sequence<>>::type;

/// checks if all elements of `Tp` are convertible to `U`
template<typename Tp, typename U> concept tuple_for = requires {
  requires tuple<Tp>;
  requires _::_tuple_for<Tp, U, make_indices_for<Tp>>::value; };

/// checks if all elements of `Tp` are nothrow convertible to `U`
template<typename Tp, typename U> concept nt_tuple_for = requires {
  requires tuple_for<Tp, U>;
  requires _::_nt_tuple_for<Tp, U, make_indices_for<Tp>>::value; };

/// converts a tuple `Tp` to `typepack` of its elements
template<tuple Tp> using to_typepack = typename _::_to_typepack<Tp, make_indices_for<Tp>>::type;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// class for handling sequences of values
template<auto... Vs> struct sequence {
private:
  template<typename S> struct _append : _append<to_sequence<S, none>> {};
  template<auto... Ws> struct _append<sequence<Ws...>> : trait<sequence<Vs..., Ws...>> {};
  template<> struct _append<none> : trait<none> {};
  template<typename S> struct _pickup : _pickup<to_sequence<S, none>> {};
  template<natt... Is> struct _pickup<sequence<Is...>> : trait<sequence<value_switch<Is, Vs...>...>> {};
  template<> struct _pickup<none> : trait<none> {};
public:
  /// size of the sequence
  static constexpr natt count = sizeof...(Vs);

  /// value at the index `I`
  template<natt I> requires(lt(I, count)) static constexpr auto at = value_switch<I, Vs...>;

  /// type at the index `I`
  template<natt I> requires(lt(I, count)) using type_at = type_switch<I, decltype(Vs)...>;

  /// appends another sequence `Sq` to this
  template<sequence_of Sq> using append = typename _append<to_sequence<Sq>>::type;

  /// picks up values at indices of `Sq` and makes a new sequence
  template<indices_for<sequence> Sq> using pickup = typename _pickup<to_sequence<Sq, natt>>::type;

  /// picks up the first `N` values
  template<natt N> requires(N < count) using fore = pickup<make_sequence<N>>;

  /// picks up the last `N` values
  template<natt N> requires(N < count) using back = pickup<make_sequence<count, count - N>>;

  /// inserts a sequence `Sq` at index `I`
  template<natt I, sequence_of Sq> requires(I < count)
  using insert = typename fore<I>::template append<Sq>::template append<back<count - I>>;

  /// expands the values to a class template `Tm`
  template<template<auto...> typename Tm> using expand = Tm<Vs...>;

  /// gets the value at index `I`
  template<natt I> requires(I < count) constexpr const auto&& get() const noexcept { return mv(at<I>); }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// class for handling packs of types
template<typename... Ts> struct typepack {
private:
  template<typename T, typename U> struct t_append {};
  template<typename... Us, typename... Vs> struct t_append<typepack<Us...>, typepack<Vs...>> : trait<typepack<Us..., Vs...>> {};
  template<typename T, typename S> struct t_pickup {};
  template<typename... Us, natt... Is> struct t_pickup<typepack<Us...>, sequence<Is...>> : trait<typepack<type_switch<Is, Us...>...>> {};
public:
  /// size of the pack
  static constexpr natt count = sizeof...(Ts);

  /// common type of the pack
  using common = common_type<Ts...>;

  /// type at the index `I`
  template<natt I> requires(I < count) using at = type_switch<I, Ts...>;

  /// appends elements of a tuple `Tp` to this
  template<tuple T> using append = typename t_append<typepack, to_typepack<T>>::type;

  /// picks up types at indices of `Sq` and makes a new pack
  template<indices_for<typepack> S> using pickup = typename t_pickup<typepack, to_sequence<S, natt>>::type;

  /// picks up the first `N` types
  template<natt N> requires(N <= count) using fore = pickup<make_sequence<N>>;

  /// picks up the last `N` types
  template<natt N> requires(N <= count) using back = pickup<make_sequence<count, count - N>>;

  /// inserts elements of a tuple `Tp` at index `I`
  template<natt I, tuple T> requires(I < count)
  using insert = typename fore<I>::template append<T>::template append<back<count - I>>;

  /// expands the types to a class template `Tm`
  template<template<typename...> typename Tm> using expand = Tm<Ts...>;

  /// gets the type at index `I`
  template<natt I> requires(I < count) constexpr at<I> get() const noexcept;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// basic tuple structure
template<typename... Ts> struct list : typepack<Ts...>::template fore<sizeof...(Ts) - 1>::template expand<list> {

  /// base structure
  using base = typepack<Ts...>::template fore<sizeof...(Ts) - 1>::template expand<list>;

  /// number of elements
  static constexpr natt count = sizeof...(Ts);

  /// last type of the list
  using last_type = type_switch<count - 1, Ts...>;

  /// last element of the list
  last_type last;

  /// gets the element at index `I`
  template<natt I> constexpr auto get() & noexcept -> type_switch<I, Ts...>& {
    if constexpr (I == count - 1) return last;
    else return base::template get<I>();
  }
  template<natt I> constexpr auto get() const& noexcept -> add_const<type_switch<I, Ts...>&> {
    if constexpr (I == count - 1) return last;
    else return base::template get<I>();
  }
  template<natt I> constexpr auto get() && noexcept -> type_switch<I, Ts...>&& {
    if constexpr (I == count - 1) return fwd<type_switch<I, Ts...>>(last);
    else return mv(*this).base::template get<I>();
  }
  template<natt I> constexpr auto get() const&& noexcept -> add_const<type_switch<I, Ts...>&&> {
    if constexpr (I == count - 1) return fwd<add_const<type_switch<I, Ts...>>>(last);
    else return mv(*this).base::template get<I>();
  }
};

/// specialization for a list of 3 elements
template<typename T1, typename T2, typename T3> struct list<T1, T2, T3> : list<T1, T2> {

  /// base structure
  using base = list<T1, T2>;

  /// number of elements
  static constexpr natt count = 3;

  /// third type of the list
  using third_type = T3;

  /// third element of the list
  third_type third;

  /// gets the element at index `I`
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

/// specialization for a list of 2 elements
template<typename T1, typename T2> struct list<T1, T2> : public list<T1> {

  /// base structure
  using base = list<T1>;

  /// number of elements
  static constexpr natt count = 2;

  /// second type of the list
  using second_type = T2;

  /// second element of the list
  second_type second;

  /// gets the element at index `I`
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

/// specialization for a list of 1 element
template<typename T1> struct list<T1> {

  /// number of elements
  static constexpr natt count = 1;

  /// first type of the list
  using first_type = T1;

  /// first element of the list
  first_type first;

  /// gets the element at index `I`
  template<natt I> requires(I == 0) constexpr first_type& get() & noexcept { return first; }
  template<natt I> requires(I == 0) constexpr add_const<first_type&> get() const& noexcept { return first; }
  template<natt I> requires(I == 0) constexpr first_type&& get() && noexcept { return fwd<first_type&&>(first); }
  template<natt I> requires(I == 0) constexpr add_const<first_type&&> get() const&& noexcept {
    return fwd<add_const<first_type&&>>(first);
  }
};

/// specialization for an empty list
template<> struct list<> {
private:
  template<typename T, typename U, natt... Is, natt... Js> static constexpr auto _concat(T&& t, U&& u, sequence<Is...>, sequence<Js...>)
    ywlib_wrapper((yw::list{get<Is>(fwd<T>(t))..., get<Js>(fwd<U>(u))...}));
  template<typename T, typename U, typename V> struct _from_typepack {};
  template<typename... Ts, typename U, template<typename...> typename Tm, typename... Vs>
  struct _from_typepack<typepack<Ts...>, U, Tm<Vs...>> : trait<list<copy_cvref<Tm<Ts>, U>...>> {};
  template<typename... Ts, typename U, template<typename, auto...> typename Tm, typename V, auto... Vs>
  struct _from_typepack<typepack<Ts...>, U, Tm<V, Vs...>> : trait<list<copy_cvref<Tm<Ts, Vs...>, U>...>> {};
  template<typename... Ts, typename U, typename V> struct _from_typepack<typepack<Ts...>, U, V> : trait<list<copy_cvref<Ts, U>...>> {};
public:
  /// number of elements
  static constexpr natt count = 0;

  /// gets a `list` that is the concatenation of two tuples `Fore` and `Back`
  template<tuple T, tuple U> static constexpr auto concat(T&& Fore, U&& Back)
    ywlib_wrapper(_concat(fwd<T>(Fore), fwd<U>(Back), make_indices_for<T>{}, make_indices_for<U>{}));

  /// gets the list that contains references of `Args`
  template<typename... Ts> static constexpr auto asref(Ts&&... Args) noexcept {
    return list<Ts&&...>{fwd<Ts>(Args)...};
  }

  /// gets a specialized `list` whose parameters are the same as those of `Tp`
  /// @note `list<>::from_typepack<typepack<int, float>>` -> `list<int, float>`
  /// @note `list<>::from_typepack<typepack<int, float>, const char&>` -> `list<const int&, const float&>`
  /// @note `list<>::from_typepack<typepack<int, float>, const array<char, 3>>` -> list<const array<int, 3>, const array<float, 3>>
  template<specialization_of<typepack> Tp, typename Template = none>
  using from_typepack = _from_typepack<Tp, Template, remove_cvref<Template>>::type;
};

/// deduction guide for `list`
template<typename... Ts> list(Ts...) -> list<Ts...>;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// clang-format off

struct t_apply {
  template<natt... Is, typename Fn, typename Tp> constexpr auto operator()(sequence<Is...>, Fn&& Func, Tp&& Tuple) const
    ywlib_wrapper(invoke(fwd<Fn>(Func), get<Is>(fwd<Tp>(Tuple))...));
  template<typename Sq, typename Fn, typename Tp> constexpr auto operator()(Sq, Fn&& Func, Tp&& Tuple) const
    ywlib_wrapper((*this)(to_sequence<Sq, natt>{}, fwd<Fn>(Func), fwd<Tp>(Tuple)));
  template<typename Fn, typename Tp> constexpr auto operator()(Fn&& Func, Tp&& Tuple) const
    ywlib_wrapper((*this)(make_indices_for<Tp>{}, fwd<Fn>(Func), fwd<Tp>(Tuple)));
};

/// applies a function `Fn` to the elements of `Tp`
/// @note 1. `apply(Func, Tuple)` -> `invoke(Func, get<0>(Tuple), get<1>(Tuple), ...)`
/// @note 2. `apply(Sequence, Func, Tuple)`-> `invoke(Func, get<Sq::at<0>>(Tuple), get<Sq::at<1>>(Tuple), ...)`
inline constexpr t_apply apply;

/// checks if `Fn` is applicable to `Tp`
template<typename Fn, typename Tp, typename S = make_indices_for<Tp>> concept applyable = requires {
  requires tuple<Tp>;
  requires indices_for<S, Tp>;
  apply(S{}, declval<Fn>(), declval<Tp>()); };

/// checks if `Fn` is nothrow applicable to `Tp`
template<typename Fn, typename Tp, typename S = make_indices_for<Tp>> concept nt_applyable = requires {
  requires applyable<Fn, Tp, S>;
  { apply(S{}, declval<Fn>(), declval<Tp>()) } noexcept; };

/// result of applying `Fn` to `Tp`
template<typename Fn, typename Tp, typename S = make_indices_for<Tp>> requires applyable<Fn, Tp, S>
using apply_result_t = decltype(apply(S{}, declval<Fn>(), declval<Tp>()));

struct t_vapply {
  template<natt... Is, typename Fn, tuple... Tps>
  constexpr auto operator()(sequence<Is...>, Fn&& Func, Tps&&... Tuples) const
    noexcept((noexcept(call(constant<Is>{}, Func, fwd<Tps>(Tuples)...)) && ...))
    requires requires { ((call(constant<Is>{}, Func, fwd<Tps>(Tuples)...)), ...); }
  { return list<>::asref(call(constant<Is>{}, Func, fwd<Tps>(Tuples)...)...); }
  template<sequence_of<natt> Sq, typename Fn, tuple... Tps>
  constexpr auto operator()(Sq, Fn&& Func, Tps&&... Tuples) const
    ywlib_wrapper((*this)(to_sequence<Sq, natt>{}, Func, fwd<Tps>(Tuples)...));
  template<typename Fn, tuple... Tps> constexpr auto operator()(Fn&& Func, Tps&&... Tuples) const
    ywlib_wrapper((*this)(make_indices_for<type_switch<0, Tps...>>{}, Func, fwd<Tps>(Tuples)...));
private:
  template<natt I, typename Fn, typename... Tps> static constexpr auto call(constant<I>, Fn& Func, Tps&&... Tuples)
    noexcept((nt_gettable<Tps, I> && ...) && nt_invocable<Fn&, element_t<Tps, I>...>)
    -> decltype(auto) { return invoke(Func, get<I>(fwd<Tps>(Tuples)...)); }
};

/// vertically applies a function `Fn` to the elements of `Tps`
/// @note 1. `vapply(Func, Tuple1, Tuple2, ...)` -> `list<>::asref(invoke(Func, get<0>(Tuple1), get<0>(Tuple2), ...), ...)`
/// @note 2. `vapply(Sequence, Func, Tuple1, Tuple2, ...)` -> `list<>::asref(invoke(Func, get<Sq::at<0>>(Tuple1), get<Sq::at<0>>(Tuple2), ...), ...)`
inline constexpr t_vapply vapply;

/// checks if `Fn` is vertically applicable to `Tps`
template<typename Fn, typename... Ts> concept vapplyable = requires {
  requires(tuple<Ts> && ...);
  vapply(declval<Fn>(), declval<Ts>()...); };

/// checks if `Fn` is nothrow vertically applicable to `Tps`
template<typename Fn, typename... Ts> concept nt_vapplyable = requires {
  requires vapplyable<Fn, Ts...>;
  { vapply(declval<Fn>(), declval<Ts>()...) } noexcept; };

/// result of vertically applying `Fn` to `Tps`
template<typename Fn, typename... Ts> requires vapplyable<Fn, Ts...>
using vapply_result_t = decltype(vapply(declval<Fn>(), declval<Ts>()...));

struct t_vassign {
  template<natt... Is, typename TpL, typename TpR>
  constexpr TpL&& operator()(sequence<Is...>, TpL&& Left, TpR&& Right) const
    noexcept(((nt_gettable<TpL, Is> && nt_gettable<TpR, Is>) && ...) && (nt_assignable<element_t<TpL, Is>, element_t<TpR, Is>> && ...))
    requires((gettable<TpL, Is> && gettable<TpR, Is>) && ...) && (assignable<element_t<TpL, Is>, element_t<TpR, Is>> && ...)
  { return ((get<Is>(fwd<TpL>(Left)) = get<Is>(fwd<TpR>(Right))), ...), fwd<TpL>(Left); }
  template<typename Sq, typename TpL, typename TpR> constexpr auto operator()(Sq, TpL&& Left, TpR&& Right) const
    ywlib_wrapper((*this)(to_sequence<Sq, natt>{}, fwd<TpL>(Left), fwd<TpR>(Right)));
  template<typename TpL, typename TpR> constexpr auto operator()(TpL&& Left, TpR&& Right) const
    ywlib_wrapper((*this)(make_indices_for<TpL>{}, fwd<TpL>(Left), fwd<TpR>(Right)));
};

/// vertically assigns the elements of `Right` to `Left`
inline constexpr t_vassign vassign;

/// checks if `TpL` is vertically assignable to `TpR`
template<typename TpL, typename TpR, typename Sq = make_indices_for<TpL>> concept vassignable = requires {
  requires indices_for<Sq, TpL>;
  requires indices_for<Sq, TpR>;
  vassign(Sq{}, declval<TpL>(), declval<TpR>()); };

/// checks if `TpL` is nothrow vertically assignable to `TpR`
template<typename TpL, typename TpR, typename Sq = make_indices_for<TpL>> concept nt_vassignable = requires {
  requires vassignable<TpL, TpR, Sq>;
  { vassign(Sq{}, declval<TpL>(), declval<TpR>()) } noexcept; };

template<typename T> struct t_build {
  template<natt... Is, typename Tp> constexpr T operator()(sequence<Is...>, Tp&& Tuple) const
    noexcept((nt_gettable<Tp, Is> && ...) && nt_constructible<T, element_t<Tp, Is>...>)
    requires (gettable<Tp, Is> && ...) && constructible<T, element_t<Tp, Is>...>
  { return T{get<Is>(fwd<Tp>(Tuple))...}; }
  template<typename Sq, typename Tp> constexpr auto operator()(Sq, Tp&& Tuple) const
    ywlib_wrapper((*this)(to_sequence<Sq, natt>{}, fwd<Tp>(Tuple)));
  template<tuple Tp> constexpr auto operator()(Tp&& Tuple) const
    ywlib_wrapper((*this)(make_indices_for<Tp>{}, fwd<Tp>(Tuple)));
};

/// builds a type `T` from the elements of `Tp`
/// @note 1. `build<T>(Tuple)` -> `T{get<0>(Tuple), get<1>(Tuple), ...}`
/// @note 2. `build<T>(Sequence, Tuple)` -> `T{get<Sq::at<0>>(Tuple), get<Sq::at<1>>(Tuple), ...}`
template<typename T> inline constexpr t_build<T> build;

/// checks if `T` is buildable from `Tp`
template<typename T, typename Tp, typename Sq = make_indices_for<Tp>> concept buildable = requires {
  requires indices_for<Sq, Tp>;
  build<T>(Sq{}, declval<Tp>()); };

/// checks if `T` is nothrow buildable from `Tp`
template<typename T, typename Tp, typename Sq = make_indices_for<Tp>> concept nt_buildable = requires {
  requires buildable<T, Tp, Sq>;
  { build<T>(Sq{}, declval<Tp>()) } noexcept; };

// clang-format on

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// class for tuple projection
template<typename T, sequence_of<natt> Sq = make_indices_for<T>, typename Fn = none> struct projector;

/// makes a virtual tuple whose size is the same as `Sq` and whose elements are the specified reference.
template<typename T, sequence_of<natt> Sq> requires(!tuple<T>) struct projector<T, Sq, none> {
  static_assert(to_sequence<Sq, natt>::count != 0);

  /// indices type
  using indices_type = to_sequence<Sq, natt>;

  /// size of the tuple
  static constexpr natt count = indices_type::count;

  /// reference to the object
  add_fwref<T> ref;

  /// constructor
  constexpr projector(add_fwref<T> Ref, Sq) noexcept : ref(Ref) {}

  /// gets the element at index `I`
  template<natt I> requires(I < count) constexpr auto get() const ywlib_wrapper(ref);
};

/// makes a virtual tuple whose size is the same as `Sq` and whose elements are `invoke_result_t<Fn, T&>`
template<typename T, sequence_of<natt> Sq, invocable<T> Fn> requires(!tuple<T>) struct projector<T, Sq, Fn> {
  static_assert(to_sequence<Sq, natt>::count != 0);

  /// indices type
  using indices_type = to_sequence<Sq, natt>;

  /// size of the tuple
  static constexpr natt count = indices_type::count;

  /// reference to the object
  add_fwref<T> ref;

  /// projection function
  Fn proj;

  /// constructor
  constexpr projector(add_fwref<T> Ref, Sq, Fn Proj) noexcept : ref(Ref), proj(mv(Proj)) {}

  /// gets the element at index `I`
  template<natt I> requires(I < count) constexpr auto get() const ywlib_wrapper(invoke(proj, ref));
};


/// makes a virtual tuple whose elements are arranged according to `Sq`
template<tuple Tp, sequence_of<natt> Sq> struct projector<Tp, Sq, none> {
  static_assert(to_sequence<Sq, natt>::count != 0);

  /// indices type
  using indices_type = to_sequence<Sq, natt>;

  /// size of the tuple
  static constexpr natt count = indices_type::count;

  /// reference to the tuple
  add_fwref<Tp> ref;

  /// constructor
  constexpr projector(add_fwref<Tp> Ref) noexcept : ref(Ref) {}
  constexpr projector(add_fwref<Tp> Ref, Sq) noexcept : ref(Ref) {}

  /// gets the element at index `I`
  template<natt I> requires(I < count) constexpr auto get() const
    ywlib_wrapper(yw::get<indices_type::template at<I>>(ref));
};


/// makes a virtual tuple whose elements at index `I` are `invoke_result_t<Fn, element_t<Tp, Sq::at<I>>>`
template<tuple Tp, sequence_of<natt> Sq, vapplyable<Tp, Sq> Fn> struct projector<Tp, Sq, Fn> {
  static_assert(to_sequence<Sq, natt>::count != 0);

  /// indices type
  using indices_type = to_sequence<Sq, natt>;

  /// size of the tuple
  static constexpr natt count = indices_type::count;

  /// reference to the tuple
  add_fwref<Tp> ref;

  /// projection function
  Fn proj;

  /// constructor
  constexpr projector(add_fwref<Tp> Ref, Fn Proj) noexcept : ref(Ref), proj(mv(Proj)) {}
  constexpr projector(add_fwref<Tp> Ref, Sq, Fn Proj) noexcept : ref(Ref), proj(mv(Proj)) {}

  /// gets the element at index `I`
  template<natt I> requires(I < count) constexpr auto get() const
    ywlib_wrapper(invoke(proj, yw::get<indices_type::template at<I>>(ref)));
};

/// deduction guide for `projector`
template<typename T, sequence_of<natt> Sq> requires(!tuple<T>) projector(T&&, Sq) -> projector<T, Sq, none>;
template<typename T, sequence_of<natt> Sq, invocable<T> Fn> requires(!tuple<T>) projector(T&&, Sq, Fn) -> projector<T, Sq, Fn>;
template<tuple Tp> projector(Tp&&) -> projector<Tp, make_indices_for<Tp>, none>;
template<tuple Tp, sequence_of<natt> Sq> projector(Tp&&, Sq) -> projector<Tp, Sq, none>;
template<tuple Tp, vapplyable<Tp, make_indices_for<Tp>> Fn> projector(Tp&&, Fn) -> projector<Tp, make_indices_for<Tp>, Fn>;
template<tuple Tp, sequence_of<natt> Sq, vapplyable<Tp, Sq> Fn> projector(Tp&&, Sq, Fn) -> projector<Tp, Sq, Fn>;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// string view structure derived from `std::basic_string_view`
template<character Ct> struct string_view {
  using value_type = Ct;
  const Ct* const pointer{};
  const natt count{};

  /// constructor
  string_view(np_t) = delete;
  constexpr string_view() noexcept = default;
  constexpr string_view(const Ct* s) : pointer(s), count(std::char_traits<Ct>::length(s)) {}
  constexpr string_view(const Ct* s, natt n) noexcept : pointer(s), count(n) {}
  template<natt N> constexpr string_view(const Ct (&s)[N]) noexcept : pointer(s), count(N - 1) {}
  template<cnt_iterator_of<Ct> It> constexpr string_view(It i, natt n) : pointer(addressof(*i)), count(n) {}
  template<cnt_iterator_of<Ct> It, sized_sentinel_for<It> Se> constexpr string_view(It i, Se s) : pointer(addressof(*i)), count(s - i) {}
  template<cnt_sized_range_of<Ct> Rg> constexpr string_view(Rg&& r) : string_view(yw::begin(r), yw::end(r)) {}

  /// assignment
  constexpr string_view& operator=(const string_view& s) {
    const_cast<const Ct*>(pointer) = s.pointer;
    const_cast<natt&>(count) = s.count;
    return *this;
  }
  constexpr string_view& operator=(string_view&& s) {
    const_cast<const Ct*>(pointer) = s.pointer;
    const_cast<natt&>(count) = s.count;
    return *this;
  }

  /// checks if the string is empty
  constexpr bool empty() const noexcept { return !count; }

  /// gets the size of the string
  constexpr natt size() const noexcept { return count; }

  /// gets the pointer to the string
  constexpr const Ct* data() const noexcept { return pointer; }

  /// gets the iterator to the beginning of the string
  constexpr const Ct* begin() const noexcept { return pointer; }

  /// gets the iterator to the end of the string
  constexpr const Ct* end() const noexcept { return pointer + count; }

  /// gets the character at index `i`
  constexpr Ct operator[](natt i) const noexcept { return pointer[i]; }

  /// gets the first character
  constexpr Ct front() const noexcept { return *pointer; }

  /// gets the last character
  constexpr Ct back() const noexcept { return pointer[count - 1]; }

  /// removes the prefix of the string
  constexpr string_view remove_prefix(natt n) const noexcept { return {pointer + n, count - n}; }

  /// removes the suffix of the string
  constexpr string_view remove_suffix(natt n) const noexcept { return {pointer, count - n}; }

  /// gets the substring
  constexpr string_view substr(natt pos, natt n = npos) const noexcept {
    if (pos > count) throw std::out_of_range("string_view::substr");
    return {pointer + pos, min(n, count - pos)};
  }

  /// finds the first occurrence of the character `c`
  constexpr natt find(Ct c, natt pos = 0) const noexcept {
    for (natt i = pos; i < count; ++i)
      if (pointer[i] == c) return i;
    return npos;
  }

  /// finds the first occurrence of the string `s`
  constexpr natt find(string_view s, natt pos = 0) const noexcept {
    if (s.count == 0) return pos;
    if (s.count > count) return npos;
    for (natt i = pos; i <= count - s.count; ++i)
      if (pointer[i] == s.pointer[0] && string_view(pointer + i, s.count) == s) return i;
    return npos;
  }

  /// finds the last occurrence of the character `c`
  constexpr natt rfind(Ct c, natt pos = npos) const noexcept {
    if (pos >= count) pos = count - 1;
    for (natt i = pos; i != npos; --i)
      if (pointer[i] == c) return i;
    return npos;
  }

  /// finds the last occurrence of the string `s`
  constexpr natt rfind(string_view s, natt pos = npos) const noexcept {
    if (s.count == 0) return pos;
    if (s.count > count) return npos;
    if (pos > count - s.count) pos = count - s.count;
    for (natt i = pos; i != npos; --i)
      if (pointer[i] == s.pointer[0] && string_view(pointer + i, s.count) == s) return i;
    return npos;
  }

  /// finds the first occurrence of any character in `s`
  constexpr natt find_first_of(string_view s, natt pos = 0) const noexcept {
    for (natt i = pos; i < count; ++i)
      if (s.find(pointer[i]) != npos) return i;
    return npos;
  }

  /// finds the first occurrence of any character not in `s`
  constexpr natt find_first_not_of(string_view s, natt pos = 0) const noexcept {
    for (natt i = pos; i < count; ++i)
      if (s.find(pointer[i]) == npos) return i;
    return npos;
  }

  /// finds the last occurrence of any character in `s`
  constexpr natt find_last_of(string_view s, natt pos = npos) const noexcept {
    if (pos >= count) pos = count - 1;
    for (natt i = pos; i != npos; --i)
      if (s.find(pointer[i]) != npos) return i;
    return npos;
  }

  /// finds the last occurrence of any character not in `s`
  constexpr natt find_last_not_of(string_view s, natt pos = npos) const noexcept {
    if (pos >= count) pos = count - 1;
    for (natt i = pos; i != npos; --i)
      if (s.find(pointer[i]) == npos) return i;
    return npos;
  }

  /// checks if the string starts with the character `c`
  constexpr bool starts_with(Ct c) const noexcept { return count && pointer[0] == c; }

  /// checks if the string starts with the string `s`
  constexpr bool starts_with(string_view s) const noexcept { return count >= s.count && string_view(pointer, s.count) == s; }

  /// checks if the string ends with the character `c`
  constexpr bool ends_with(Ct c) const noexcept { return count && pointer[count - 1] == c; }

  /// checks if the string ends with the string `s`
  constexpr bool ends_with(string_view s) const noexcept { return count >= s.count && string_view(pointer + count - s.count, s.count) == s; }

  /// checks if the string is null-terminated
  constexpr bool null_terminated() const noexcept { return *end() == Ct{}; }

  /// checks if the string contains the character `c`
  constexpr bool contains(Ct c) const noexcept { return find(c) != npos; }

  /// checks if the string contains the string `s`
  constexpr bool contains(string_view s) const noexcept { return find(s) != npos; }

  /// comparison
  constexpr bool operator==(string_view s) const noexcept {
    return count == s.count && std::char_traits<Ct>::compare(pointer, s.pointer, count) == 0;
  }
  constexpr auto operator<=>(string_view s) const noexcept {
    for (natt i{}, ii{min(count, s.count)}; i < ii; ++i)
      if (auto c = pointer[i] <=> s.pointer[i]; c != 0) return c;
    return std::strong_ordering::equivalent;
  }
};

using stv1 = string_view<cat1>;
using stv2 = string_view<cat2>;

constexpr stv1 operator""_sv(const cat1* s, natt n) noexcept { return stv1(s, n); }
constexpr stv2 operator""_sv(const cat2* s, natt n) noexcept { return stv2(s, n); }

/// deduction guide for `string_view`
template<character Ct> string_view(const Ct*, natt) -> string_view<Ct>;
template<cnt_iterator It, sentinel_for<It> Se> string_view(It, Se) -> string_view<iter_value_t<It>>;
template<cnt_range Rg> string_view(Rg&&) -> string_view<iter_value_t<Rg>>;

/// dynamic string class
template<character Ct> class string : public std::basic_string<Ct> {
public:
  /// constructor
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

  /// assignment
  // constexpr string& operator=(std::basic_string<Ct> s) noexcept { return *this = string(mv(s)); }
  // template<natt N> constexpr string& operator=(const Ct (&s)[N]) noexcept { return *this = string(s, N); }
  // template<range_of<Ct> Rg> requires(!convertible_to<Rg, std::basic_string<Ct>> && !is_array<Rg>)
  // constexpr string& operator=(Rg&& r) { return *this = string(fwd<Rg>(r)); }
};

using str1 = string<cat1>;
using str2 = string<cat2>;

/// deduction guide for `string`
template<character Ct> string(natt, Ct) -> string<Ct>;
template<character Ct> string(const Ct*) -> string<Ct>;
template<character Ct> string(const Ct*, natt) -> string<Ct>;
template<iterator It, sentinel_for<It> Se> string(It, Se) -> string<iter_value_t<It>>;
template<range Rg> string(Rg&&) -> string<iter_value_t<Rg>>;

/// gets the size of the string
inline constexpr overload strlen{
  [](const character auto* const Str) { return std::char_traits<cat1>::length(Str); },
  []<range Rg>(Rg&& Str) requires character<iter_value_t<Rg>> { return std::ranges::distance(Str); }};

template<arithmetic T> struct t_stov {
  T operator()(str1 Str) noexcept { return call(mv(Str)); }
  T operator()(str2 Str) noexcept { return call(mv(Str)); }
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

/// converts a string to a value
template<arithmetic T> inline constexpr t_stov<T> stov;

/// converts a value to a string
template<included_in<cat1, cat2> Ct> inline constexpr auto vtos = []<arithmetic T>(const T v) noexcept {
  if constexpr (same_as<Ct, cat1>) return std::to_string(v);
  else return std::to_wstring(v);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// source location
struct source {
  nat4 line, column;
  stv1 file, func;
  constexpr source(nat4 Line = __builtin_LINE(), nat4 Column = __builtin_COLUMN(),
                   stv1 File = __builtin_FILE(), stv1 Func = __builtin_FUNCTION()) noexcept
    : line(Line), column(Column), file(mv(File)), func(mv(Func)) {}

  /// output stream
  template<typename Tr> friend std::basic_ostream<cat1, Tr>&
  operator<<(std::basic_ostream<cat1, Tr>& OS, const source& S) {
    return OS << std::format("{} ({}, {}, {})", S.file, S.line, S.column, S.func);
  }
};

/// exception class which contains the source location
class except : public std::exception {
public:
  explicit except(const std::string& s, source _ = {}) noexcept : except(s.data(), mv(_)){};
  explicit except(const cat1* s, source _ = {}) noexcept;
  explicit except(const std::exception& Base, source _ = {}) noexcept : except(Base.what(), mv(_)){};
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// class for file path derived from `std::filesystem::path`
class path : public std::filesystem::path {
public:
  /// constructor
  path() noexcept = default;
  path(const path&) = default;
  path(path&&) noexcept = default;
  path(const std::filesystem::path& p) : std::filesystem::path(p) {}
  path(std::filesystem::path&& p) noexcept : std::filesystem::path(mv(p)) {}
  path(string_type&& s) : std::filesystem::path(mv(s)) {}
  template<range Rg> requires character<iter_value_t<Rg>> path(Rg&& r)
    : std::filesystem::path(yw::begin(r), yw::end(r)) {}
  template<character Ct> path(const Ct* s) : std::filesystem::path(s) {}
  /// assignment
  path& operator=(const path&) = default;
  path& operator=(path&&) noexcept = default;
  path& operator=(const std::filesystem::path& p) { return *this = path(p); }
  path& operator=(std::filesystem::path&& p) noexcept { return *this = path(mv(p)); }
  path& operator=(string_type&& s) { return *this = path(mv(s)); }
  template<range Rg> requires character<iter_value_t<Rg>>
  path& operator=(Rg&& r) { return *this = path(fwd<Rg>(r)); }
  template<character Ct> path& operator=(const Ct* s) { return *this = path(s); }

  /// checks if any file exists at the path
  bool exists() const { return std::filesystem::exists(*this); }

  /// checks if this is a regular file
  bool is_file() const { return std::filesystem::is_regular_file(*this); }

  /// checks if this is a directory
  bool is_directory() const { return std::filesystem::is_directory(*this); }

  /// gets the file size
  nat8 file_size() const { return std::filesystem::file_size(*this); }

  /// lists the contents of the directory
  array<path> list(bool Recursive = false) const {
    array<path> A;
    if (Recursive)
      for (const auto& E : std::filesystem::recursive_directory_iterator(*this)) A.push_back(E.path());
    else
      for (const auto& E : std::filesystem::directory_iterator(*this)) A.push_back(E.path());
    return A;
  }

  /// reads the file
  /// @note `Out` must be a non-const container whose size is equal to the file size
  void read(cnt_range auto& Out) const requires(!is_const<decltype(yw::data(Out))>) {
    using rg = decltype(Out);
    if (const auto n = yw::size(Out) * sizeof(iter_value_t<rg>); file_size() == n) {
      if (std::ifstream ifs(*this, std::ios::binary); ifs) ifs.read(reinterpret_cast<char*>(yw::data(Out)), n);
      else throw except("failed to open the file");
    } else throw except("byte size of `Out` is not equal to the file size");
  }

  /// writes the file
  void write(cnt_range auto&& In) const { write(yw::data(In), yw::size(In) * sizeof(iter_value_t<decltype(In)>)); }
  void write(const void* In, natt Size) const {
    if (std::ofstream ofs(*this, std::ios_base::binary); ofs) ofs.write(reinterpret_cast<const char*>(In), Size);
    else throw except("failed to open the file");
  }

  /// removes the file
  void remove() const { std::filesystem::remove(*this); }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// clang-format off

/// class for calling the specified function object when itself is evaluated
template<invocable... Fs> struct caster : public Fs... {
  template<typename T> static constexpr natt i = []<typename... Ts>(typepack<Ts...>) {
    return inspects<same_as<Ts, T>...>; }(typepack<invoke_result_t<Fs>...>{});
  template<typename T> static constexpr natt j = i<T> < sizeof...(Fs) ? i<T> : []<typename... Ts>(typepack<Ts...>) {
    return inspects<convertible_to<Ts, T>...>; }(typepack<invoke_result_t<Fs>...>{});
public:
  template<typename T> requires(j<T> < sizeof...(Fs)) constexpr operator T() const
    noexcept(nt_convertible_to<invoke_result_t<type_switch<j<T>, Fs...>>, T>)
  { return type_switch<j<T>, Fs...>::operator()(); }
};

/// checks if this is constant evaluated
inline constexpr caster is_cev{[]() noexcept { return std::is_constant_evaluated(); }};

// clang-format on

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

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
template<typename T, typename S, typename F> struct tuple_size<yw::projector<T, S, F>> : integral_constant<size_t, yw::to_sequence<S>::size> {};
template<size_t I, typename T, typename S, typename F> struct tuple_element<I, yw::projector<T, S, F>> : remove_cvref<yw::element_t<yw::projector<T, S, F>, I>> {};
template<typename T, size_t N> struct tuple_size<yw::array<T, N>> : integral_constant<size_t, N> {};
template<size_t I, typename T, size_t N> struct tuple_element<I, yw::array<T, N>> : type_identity<T> {};
template<typename Ct> struct formatter<yw::string_view<Ct>> : formatter<basic_string_view<Ct>> {
  auto format(const yw::string_view<Ct>& s, format_context& ctx) const { return formatter<basic_string_view<Ct>>::format({s.pointer, s.count}, ctx); } };
template<typename Ct> struct formatter<yw::string<Ct>> : formatter<basic_string<Ct>> {
  auto format(const yw::string<Ct>& s, format_context& ctx) const { return formatter<basic_string<Ct>>::format(s, ctx); } };
template<> struct formatter<yw::source> : formatter<string> {
  auto format(const yw::source& s, format_context& ctx) const {
    return formatter<string>::format(std::format("{} ({}, {}, {})", s.file, s.line, s.column, s.func), ctx); } };
template<yw::iterator It, yw::iter_unary_invocable<It> Pj> struct incrementable_traits<yw::projector<It, yw::sequence<>, Pj>> { using difference_type = yw::iter_difference_t<It>; };
} // clang-format on

inline yw::except::except(const cat1* s, source _) noexcept
  : std::exception(std::format("{} -> {}\n", s, _).data()) {}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <immintrin.h>

namespace yw {
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using xvector = __m128;
using xwector = __m256d;
using xrect = __m128i;
using xmatrix = array<__m128, 4>;

/// makes a xvector type
inline constexpr overload xv{
  [](const fat4 A) noexcept { return _mm_set1_ps(A); },
  [](const fat8 A) noexcept { return _mm256_set1_pd(A); },
  [](const int4 A) noexcept { return _mm_set1_epi32(A); },
  [](const fat4* A) noexcept { return _mm_loadu_ps(A); },
  [](const fat8* A) noexcept { return _mm256_loadu_pd(A); },
  [](const int4* A) noexcept { return _mm_loadu_epi32(A); },
  [](const fat4 X, const fat4 Y, const fat4 Z, const fat4 W) noexcept { return _mm_set_ps(W, Z, Y, X); },
  [](const fat8 X, const fat8 Y, const fat8 Z, const fat8 W) noexcept { return _mm256_set_pd(W, Z, Y, X); },
  [](const int4 X, const int4 Y, const int4 Z, const int4 W) noexcept { return _mm_set_epi32(W, Z, Y, X); },
  [](const xvector& X, const xvector& Y, const xvector& Z, const xvector& W) noexcept { return xmatrix{X, Y, Z, W}; }};

/// casts a xvector type to another type
template<included_in<xvector, xwector, xrect> Xv> inline constexpr overload xvcast{
  [](const xvector& A) noexcept {
    if constexpr (same_as<Xv, xvector>) return A;
    else if constexpr (same_as<Xv, xwector>) return _mm256_cvtps_pd(A);
    else if constexpr (same_as<Xv, xrect>) return _mm_cvtps_epi32(A);
  },
  [](const xwector& A) noexcept {
    if constexpr (same_as<Xv, xvector>) return _mm256_cvtpd_ps(A);
    else if constexpr (same_as<Xv, xwector>) return A;
    else if constexpr (same_as<Xv, xrect>) return _mm256_cvtpd_epi32(A);
  },
  [](const xrect& A) noexcept {
    if constexpr (same_as<Xv, xvector>) return _mm_cvtepi32_ps(A);
    else if constexpr (same_as<Xv, xwector>) return _mm256_cvtepi32_pd(A);
    else if constexpr (same_as<Xv, xrect>) return A;
  }};

/// bit-casts a xvector type to another type
inline constexpr overload xvbitcast{
  [](const xvector& A) noexcept -> xrect { return _mm_castps_si128(A); },
  [](const xrect& A) noexcept -> xvector { return _mm_castsi128_ps(A); }};

/// caster for xvector whose elements are all zero
inline constexpr caster xv_zero{
  []() noexcept { return _mm_setzero_ps(); },
  []() noexcept { return _mm256_setzero_pd(); },
  []() noexcept { return _mm_setzero_si128(); },
  []() noexcept -> const xmatrix& {
    static const xmatrix _{_mm_setzero_ps(), _mm_setzero_ps(), _mm_setzero_ps(), _mm_setzero_ps()};
    return _; }};

/// caster for constant xvector
template<value X, value Y = X, value Z = Y, value W = Z> inline constexpr caster xv_constant{
  []() noexcept -> const xvector& { static const xvector _{xv(fat4(X), fat4(Y), fat4(Z), fat4(W))}; return _; },
  []() noexcept -> const xwector& { static const xwector _{xv(fat8(X), fat8(Y), fat8(Z), fat8(W))}; return _; },
  []() noexcept -> const xrect& { static const xrect _{xv(int4(X), int4(Y), int4(Z), int4(W))}; return _; }};

/// caster for xvector{1, 0, 0, 0}
inline constexpr auto xv_x = xv_constant<1, 0, 0, 0>;

/// caster for xvector{0, 1, 0, 0}
inline constexpr auto xv_y = xv_constant<0, 1, 0, 0>;

/// caster for xvector{0, 0, 1, 0}
inline constexpr auto xv_z = xv_constant<0, 0, 1, 0>;

/// caster for xvector{0, 0, 0, 1}
inline constexpr auto xv_w = xv_constant<0, 0, 0, 1>;

/// caster for xvector{-0, -0, -0, -0}
inline constexpr auto xv_neg_zero = xv_constant<-0.0, -0.0, -0.0, -0.0>;

/// caster for xvector{-1, 0, 0, 0}
inline constexpr auto xv_neg_x = xv_constant<-1, 0, 0, 0>;

/// caster for xvector{0, -1, 0, 0}
inline constexpr auto xv_neg_y = xv_constant<0, -1, 0, 0>;

/// caster for xvector{0, 0, -1, 0}
inline constexpr auto xv_neg_z = xv_constant<0, 0, -1, 0>;

/// caster for xvector{0, 0, 0, -1}
inline constexpr auto xv_neg_w = xv_constant<0, 0, 0, -1>;

/// caster for idenity matrix
inline constexpr caster xv_identity{
  []() noexcept -> const xmatrix& { static const xmatrix _{xv_x, xv_y, xv_z, xv_w}; return _; }};

/// gets the element at index `I` of `Xv`
template<natt I> requires(I < 4) inline constexpr overload xvget{
  [](const xvector& Xv) noexcept {
    if constexpr (I == 0) return _mm_cvtss_f32(Xv); else return bitcast<fat4>(_mm_extract_ps(Xv, int(I))); },
  [](const xwector& Xv) noexcept { return bitcast<fat8>(_mm256_extract_epi64(_mm256_castpd_si256(Xv), int(I))); },
  [](const xrect& Xv) noexcept {
    if constexpr (I == 0) return _mm_cvtsi128_si32(Xv); else return _mm_extract_epi32(Xv, int(I)); }};

/// sets `Value` at index `I` of `Xv`
template<natt I> requires(I < 4) inline constexpr overload xvset{
  [](const xvector& Xv, fat4 Value) noexcept {
    return xvbitcast(_mm_insert_epi32(xvbitcast(Xv), bitcast<int4>(Value), int(I)));
  },
  [](const xwector& Xv, fat8 Value) noexcept {
    return _mm256_castsi256_pd(_mm256_insert_epi64(_mm256_castpd_si256(Xv), bitcast<int8>(Value), int(I)));
  },
  [](const xrect& Xv, int4 Value) noexcept { return _mm_insert_epi32(Xv, Value, int(I)); }};

/// blends `A` and `B` according to `X`, `Y`, `Z`, and `W`
/// @note `xvblend<true, false, true, false>(A, B)` -> `xv{B[0], A[1], B[2], A[3]}`
template<bool X, bool Y, bool Z, bool W> inline constexpr overload xvblend{
  [](const xvector& A, const xvector& B) noexcept {
    if constexpr (!(X | Y | Z | W)) return A;
    else if constexpr (X & Y & Z & W) return B;
    else return _mm_blend_ps(A, B, int(X + Y * 2 + Z * 4 + W * 8));
  },
  [](const xwector& A, const xwector& B) noexcept {
    if constexpr (!(X | Y | Z | W)) return A;
    else if constexpr (X & Y & Z & W) return B;
    else return _mm256_blend_pd(A, B, int(X + Y * 2 + Z * 4 + W * 8));
  },
  [](const xrect& A, const xrect& B) noexcept {
    if constexpr (!(X | Y | Z | W)) return A;
    else if constexpr (X & Y & Z & W) return B;
    else return _mm_blend_epi32(A, B, int(X + Y * 2 + Z * 4 + W * 8));
  }};

/// sets the specified elements to zero
template<bool X, bool Y, bool Z, bool W> inline constexpr auto xvsetzero =
  [](const included_in<xvector, xwector, xrect> auto& Xv) noexcept { return xvblend<X, Y, Z, W>(Xv, xv_zero); };

template<intt X, intt Y, intt Z, intt W> struct t_xvpermute {
  template<intt A, intt B, intt C, intt D> friend struct t_xvpermute;
public:
  xvector operator()(const xvector& A) const noexcept requires(lt(max(X, Y, Z, W), 4)) { return call(A); }
  xwector operator()(const xwector& A) const noexcept requires(lt(max(X, Y, Z, W), 4)) { return call(A); }
  xrect operator()(const xrect& A) const noexcept requires(lt(max(X, Y, Z, W), 4)) { return xvbitcast(call(xvbitcast(A))); }
  xvector operator()(const xvector& A, const xvector& B) const noexcept requires(lt(max(X, Y, Z, W), 8)) {
    constexpr auto f = [](auto a, auto b) { return a == b || a < 0; };
    constexpr intt xa = (f(X, 0) ? 0 : X), ya = (f(Y, 1) ? 1 : Y), za = (f(Z, 2) ? 2 : Z), wa = (f(W, 3) ? 3 : W),
                   xb = (f(X, 4) ? 4 : X), yb = (f(Y, 5) ? 5 : Y), zb = (f(Z, 6) ? 6 : Z), wb = (f(W, 7) ? 7 : W);
    if constexpr ((xa | ya | za | wa) < 4) return t_xvpermute<xa, ya, za, wa>::call(A);
    else if constexpr (4 <= (xb & yb & zb & wb)) return t_xvpermute<xb - 4, yb - 4, zb - 4, wb - 4>::call(B);
    else if constexpr ((xa & 3) == 0 && (ya & 3) == 1 && (za & 3) == 2 && (wa & 3) == 3) return xvblend<lt(xa, 4), lt(ya, 4), lt(za, 4), lt(wa, 4)>(B, A);
    else if constexpr ((xa | ya) < 4 && 4 <= (zb & wb)) return _mm_shuffle_ps(A, B, xa + ya * 4 + (zb - 4) * 16 + (wb - 4) * 64);
    else if constexpr (4 <= (xb & yb) && (za | wa) < 4) return _mm_shuffle_ps(B, A, xb - 4 | (yb - 4) * 4 | za * 16 | wa * 64);
    else if constexpr ((xa == 0) + (ya == 1) + (za == 2) + (wa == 3) == 3) {
      constexpr natt i = inspects<xa != 0, ya != 1, za != 2, wa != 3>;
      return _mm_insert_ps(A, B, int((value_switch<i, xa, ya, za, wa> - 4) << 6 | i << 4));
    } else if constexpr ((xb == 4) + (yb == 5) + (zb == 6) + (wb == 7) == 3) {
      constexpr natt i = inspects<xb != 4, yb != 5, zb != 6, wb != 7>;
      return _mm_insert_ps(A, B, int(value_switch<i, xb, yb, zb, wb> << 6 | i << 4));
    } else if constexpr ((xa < 4 || xb == 4) && (ya < 4 || yb == 5) && (za < 4 || zb == 6) && (wa < 4 || wb == 7))
      return xvblend<xb == 4, yb == 5, zb == 6, wb == 7>(t_xvpermute<xa, ya, za, wa>::call(A), B);
    else if constexpr ((4 <= xb || xa == 0) && (4 <= yb || ya == 1) && (4 <= zb || za == 2) && (4 <= wb || wa == 3))
      return xvblend<xa == 0, ya == 1, za == 2, wa == 3>(t_xvpermute<xb - 4, yb - 4, zb - 4, wb - 4>::call(B), A);
    else if constexpr ((xa < 4) + (ya < 4) + (za < 4) + (wa < 4) == 3) {
      constexpr natt i = inspects<(xa > 3), (ya > 3), (za > 3), (wa > 3)>;
      return _mm_insert_ps(t_xvpermute<xa, ya, za, wa>::call(A), B, int((value_switch<i, xa, ya, za, wa> - 4) << 6 | i << 4));
    } else if constexpr ((xb < 4) + (yb < 4) + (zb < 4) + (wb < 4) == 1) {
      constexpr natt i = inspects<(xb < 4), (yb < 4), (zb < 4), (wb < 4)>;
      return _mm_insert_ps(t_xvpermute<xb - 4, yb - 4, zb - 4, wb - 4>::call(B), A, int(value_switch<i, xb, yb, zb, wb> << 6 | i << 4));
    } else return xvblend<lt(xa, 4), lt(ya, 4), lt(za, 4), lt(wa, 4)>(
      t_xvpermute<xb - 4, yb - 4, zb - 4, wb - 4>::call(B), t_xvpermute<xa, ya, za, wa>::call(A));
  };
  xwector operator()(const xwector& A, const xwector& B) const noexcept requires(lt(max(X, Y, Z, W), 8)) {
    constexpr auto f = [](auto a, auto b) noexcept { return a == b || a < 0; };
    constexpr intt xa = (f(X, 0) ? 0 : X), ya = (f(Y, 1) ? 1 : Y), za = (f(Z, 2) ? 2 : Z), wa = (f(W, 3) ? 3 : W),
                   xb = (f(X, 4) ? 4 : X), yb = (f(Y, 5) ? 5 : Y), zb = (f(Z, 6) ? 6 : Z), wb = (f(W, 7) ? 7 : W);
    if constexpr (xa == 0 && ya == 1 && za == 2 && wa == 3) return A;
    else if constexpr (xb == 4 && yb == 5 && zb == 6 && wb == 7) return B;
    else if constexpr ((xa & 3) == 0 && (ya & 3) == 1 && (za & 3) == 2 && (wa & 3) == 3) return xvblend<lt(xa, 4), lt(ya, 4), lt(za, 4), lt(wa, 4)>(B, A);
    else if constexpr (xa < 2 && (yb == 4 || yb == 5) && (za == 2 || zb == 3) && wb > 5)
      return _mm256_shuffle_pd(A, B, (xa == 1) + (yb == 5) * 2 + (za == 3) * 4 + (wb == 7) * 8);
    else if constexpr ((xb == 4 || xb == 5) && ya < 2 && zb > 5 && (wa == 2 || wa == 3))
      return _mm256_shuffle_pd(B, A, (xb == 5) + (ya == 1) * 2 + (zb == 7) * 4 + (wa == 3) * 8);
    else if constexpr ((xa | ya | za | wa) < 4) return t_xvpermute<xa, ya, za, wa>::call(A);
    else if constexpr ((xb & yb & zb & wb) > 3) return t_xvpermute<xb - 4, yb - 4, zb - 4, wb - 4>::call(B);
    else if constexpr (((xa == 0 && ya == 1) || (xa == 2 && ya == 3)) && ((zb == 4 && wb == 5) || (zb == 6 && wb == 7)))
      return _mm256_shuffle_f64x2(A, B, (xa == 2) + (zb == 6) * 2);
    else if constexpr (((xb == 4 && yb == 5) || (xb == 6 && yb == 7)) && ((za == 0 && wa == 1) || (za == 2 && wa == 3)))
      return _mm256_shuffle_f64x2(B, A, (xb == 6) + (za == 2) * 2);
    else return xvblend<lt(xa, 4), lt(ya, 4), lt(za, 4), lt(wa, 4)>(t_xvpermute<xb - 4, yb - 4, zb - 4, wb - 4>::call(B), t_xvpermute<xa, ya, za, wa>::call(A));
  }
  xrect operator()(const xrect& A, const xrect& B) const noexcept requires(lt(max(X, Y, Z, W), 8)) {
    return xvbitcast((*this)(xvbitcast(A), xvbitcast(B)));
  }
private:
  static xvector call(const xvector& A) noexcept {
    constexpr auto f = [](auto a, auto b) { return a == b || a < 0 || a >= 4; };
    constexpr natt x = (f(X, 0) ? 0 : X), y = (f(Y, 1) ? 1 : Y), z = (f(Z, 2) ? 2 : Z), w = (f(W, 3) ? 3 : W);
    if constexpr (f(X, 0) && f(Y, 1) && f(Z, 2) && f(W, 3)) return A;
    else if constexpr (f(X, 0) && f(Y, 0) && f(Z, 2) && f(W, 2)) return _mm_moveldup_ps(A);
    else if constexpr (f(X, 1) && f(Y, 1) && f(Z, 3) && f(W, 3)) return _mm_movehdup_ps(A);
    else return _mm_permute_ps(A, int(x + y * 4 + z * 16 + w * 64));
  }
  static xwector call(const xwector& A) noexcept {
    constexpr auto f = [](auto a, auto b) { return a == b || a < 0 || a >= 4; };
    constexpr natt x = (f(X, 0) ? 0 : X), y = (f(Y, 1) ? 1 : Y), z = (f(Z, 2) ? 2 : Z), w = (f(W, 3) ? 3 : W);
    if constexpr (x == 0 && y == 1 && z == 2 && w == 3) return A;
    else if constexpr ((x | y) < 2 && 2 <= (z & w)) return _mm256_permute_pd(A, int(x + y * 2 + (z - 2) * 4 + (w - 2) * 8));
    else return _mm256_permute4x64_pd(A, int(x + y * 4 + z * 16 + w * 64));
  }
};

/// permutes the elements of `Xv` according to `X`, `Y`, `Z`, and `W`
/// @note `X`, `Y`, `Z`, and/or `W` can be set negative if you are not concerned about the corresponding element.
template<intt X, intt Y, intt Z, intt W> inline constexpr t_xvpermute<X, Y, Z, W> xvpermute;

/// Inserts the specified element of `Picked` to the specified element of `Inserted`
template<natt To, natt From, natt Zero = 0b0000> requires((To | From) < 4 && Zero < 16)
inline constexpr overload xvinsert{
  [](const xvector& Inserted, const xvector& Picked) noexcept {
  constexpr auto f = [](auto a) { return To == a ? From + 4 : a; };
  if constexpr (Zero == 0) return xvpermute<f(0), f(1), f(2), f(3)>(Inserted, Picked);
  else return _mm_insert_ps(Inserted, Picked, From << 6 | To << 4 | Zero); },
  [](const xwector& Inserted, const xwector& Picked) noexcept {
  constexpr auto f = [](auto a) { return To == a ? From + 4 : a; };
  if constexpr (Zero == 0) return xvpermute<f(0), f(1), f(2), f(3)>(Inserted, Picked);
  else return xvblend<!(Zero & 1), !(Zero & 2), !(Zero & 4), !(Zero & 8)>(
    xv_zero, xvpermute<f(0), f(1), f(2), f(3)>(Inserted, Picked)); },
  [](const xrect& Inserted, const xrect& Picked) noexcept {
    constexpr auto f = [](auto a) { return To == a ? From + 4 : a; };
    if constexpr (Zero == 0) return xvpermute<f(0), f(1), f(2), f(3)>(Inserted, Picked);
    else return xvbitcast(_mm_insert_ps(xvbitcast(Inserted), xvbitcast(Picked), From << 6 | To << 4 | Zero));
  }};

/// checks if the two xvector are equal
inline constexpr overload xveq{
  [](const xvector& A, const xvector& B) noexcept -> bool { return _mm_test_all_ones(xvbitcast(_mm_cmpeq_ps(A, B))); },
  [](const xwector& A, const xwector& B) noexcept -> bool { return _mm256_testc_pd(_mm256_cmp_pd(A, B, 0), xv_neg_zero); },
  [](const xrect& A, const xrect& B) noexcept -> bool { return _mm_test_all_ones(_mm_cmpeq_epi32(A, B)); }};

/// checks if the two xvector are not equal
inline constexpr overload xvne{
  [](const xvector& A, const xvector& B) noexcept -> bool { return !xveq(A, B); },
  [](const xwector& A, const xwector& B) noexcept -> bool { return !xveq(A, B); },
  [](const xrect& A, const xrect& B) noexcept -> bool { return !xveq(A, B); },
  []<natt N>(const xvector (&A)[N], const xvector (&B)[N]) noexcept -> bool { return !xveq(A, B); }};

/// checks if `A` is less than `B`
inline constexpr overload xvlt{
  [](const xvector& A, const xvector& B) noexcept -> bool { return _mm_movemask_ps(_mm_cmpgt_ps(A, B)) < _mm_movemask_ps(_mm_cmplt_ps(A, B)); },
  [](const xwector& A, const xwector& B) noexcept -> bool { return _mm256_movemask_pd(_mm256_cmp_pd(B, A, 17)) < _mm256_movemask_pd(_mm256_cmp_pd(A, B, 17)); },
  [](const xrect& A, const xrect& B) noexcept -> bool { return _mm_movemask_ps(xvbitcast(_mm_cmpgt_epi32(A, B))) < _mm_movemask_ps(xvbitcast(_mm_cmplt_epi32(A, B))); }};

/// checks if `A` is less than or equal to `B`
inline constexpr overload xvle{
  [](const xvector& A, const xvector& B) noexcept -> bool { return _mm_movemask_ps(_mm_cmpgt_ps(A, B)) <= _mm_movemask_ps(_mm_cmplt_ps(A, B)); },
  [](const xwector& A, const xwector& B) noexcept -> bool { return _mm256_movemask_pd(_mm256_cmp_pd(B, A, 17)) <= _mm256_movemask_pd(_mm256_cmp_pd(A, B, 17)); },
  [](const xrect& A, const xrect& B) noexcept -> bool { return _mm_movemask_ps(xvbitcast(_mm_cmpgt_epi32(A, B))) <= _mm_movemask_ps(xvbitcast(_mm_cmplt_epi32(A, B))); }};

/// checks if `A` is greater than `B`
inline constexpr overload xvgt{
  [](const xvector& A, const xvector& B) noexcept -> bool { return _mm_movemask_ps(_mm_cmpgt_ps(A, B)) > _mm_movemask_ps(_mm_cmplt_ps(A, B)); },
  [](const xwector& A, const xwector& B) noexcept -> bool { return _mm256_movemask_pd(_mm256_cmp_pd(B, A, 17)) > _mm256_movemask_pd(_mm256_cmp_pd(A, B, 17)); },
  [](const xrect& A, const xrect& B) noexcept -> bool { return _mm_movemask_ps(xvbitcast(_mm_cmpgt_epi32(A, B))) > _mm_movemask_ps(xvbitcast(_mm_cmplt_epi32(A, B))); }};

/// checks if `A` is greater than or equal to `B`
inline constexpr overload xvge{
  [](const xvector& A, const xvector& B) noexcept -> bool { return _mm_movemask_ps(_mm_cmpgt_ps(A, B)) >= _mm_movemask_ps(_mm_cmplt_ps(A, B)); },
  [](const xwector& A, const xwector& B) noexcept -> bool { return _mm256_movemask_pd(_mm256_cmp_pd(B, A, 17)) >= _mm256_movemask_pd(_mm256_cmp_pd(A, B, 17)); },
  [](const xrect& A, const xrect& B) noexcept -> bool { return _mm_movemask_ps(xvbitcast(_mm_cmpgt_epi32(A, B))) >= _mm_movemask_ps(xvbitcast(_mm_cmplt_epi32(A, B))); }};

/// three-way comparison of `A` and `B`
inline constexpr overload xvtw{
  [](const xvector& A, const xvector& B) noexcept { return _mm_movemask_ps(_mm_cmpgt_ps(A, B)) <=> _mm_movemask_ps(_mm_cmplt_ps(A, B)); },
  [](const xwector& A, const xwector& B) noexcept { return _mm256_movemask_pd(_mm256_cmp_pd(B, A, 17)) <=> _mm256_movemask_pd(_mm256_cmp_pd(A, B, 17)); },
  [](const xrect& A, const xrect& B) noexcept { return _mm_movemask_ps(xvbitcast(_mm_cmpgt_epi32(A, B))) <=> _mm_movemask_ps(xvbitcast(_mm_cmplt_epi32(A, B))); }};

/// gets the absolute value of `A`
/// @note 1. `xvabs(const Xv& A)`
/// @note 2. `xvabs(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result` may be the same object as `A`.
inline constexpr overload xvabs{
  [](const xvector& A) noexcept { return _mm_andnot_ps(xv_neg_zero, A); },
  [](const xwector& A) noexcept { return _mm256_andnot_pd(xv_neg_zero, A); },
  [](const xrect& A) noexcept { return _mm_abs_epi32(A); },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_andnot_ps(xv_neg_zero, A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_andnot_pd(xv_neg_zero, A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xrect (&A)[N], xrect (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_abs_epi32(A[Is])), ...); }(make_sequence<N>{});
  }};

/// negates `A`
/// @note 1. `xvneg(const Xv& A)`
/// @note 2. `xvneg(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result` may be the same object as `A`.
inline constexpr overload xvneg{
  [](const xvector& A) noexcept { return _mm_xor_ps(A, xv_neg_zero); },
  [](const xwector& A) noexcept { return _mm256_xor_pd(A, xv_neg_zero); },
  [](const xrect& A) noexcept { return _mm_xor_epi32(A, xv_neg_zero); },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_xor_ps(A[Is], xv_neg_zero)), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_xor_pd(A[Is], xv_neg_zero)), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xrect (&A)[N], xrect (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_xor_epi32(A[Is], xv_neg_zero)), ...); }(make_sequence<N>{});
  }};

/// adds `A` and `B`
/// @note 1. `xvadd(const Xv& A, const Xv& B)`
/// @note 2. `xvadd(const Xv (&A)[N], const Xv (&B)[N], Xv (&Result)[N])`
/// @note `Result` may be the same object as `A` or `B`.
inline constexpr overload xvadd{
  [](const xvector& A, const xvector& B) noexcept { return _mm_add_ps(A, B); },
  [](const xwector& A, const xwector& B) noexcept { return _mm256_add_pd(A, B); },
  [](const xrect& A, const xrect& B) noexcept { return _mm_add_epi32(A, B); },
  []<natt N>(const xvector (&A)[N], const xvector (&B)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_add_ps(A[Is], B[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], const xwector (&B)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_add_pd(A[Is], B[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xrect (&A)[N], const xrect (&B)[N], xrect (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_add_epi32(A[Is], B[Is])), ...); }(make_sequence<N>{});
  }};

/// subtracts `B` from `A`
/// @note 1. `xvsub(const Xv& A, const Xv& B)`
/// @note 2. `xvsub(const Xv (&A)[N], const Xv (&B)[N], Xv (&Result)[N])`
/// @note `Result` may be the same object as `A` or `B`.
inline constexpr overload xvsub{
  [](const xvector& A, const xvector& B) noexcept { return _mm_sub_ps(A, B); },
  [](const xwector& A, const xwector& B) noexcept { return _mm256_sub_pd(A, B); },
  [](const xrect& A, const xrect& B) noexcept { return _mm_sub_epi32(A, B); },
  []<natt N>(const xvector (&A)[N], const xvector (&B)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_sub_ps(A[Is], B[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], const xwector (&B)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_sub_pd(A[Is], B[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xrect (&A)[N], const xrect (&B)[N], xrect (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_sub_epi32(A[Is], B[Is])), ...); }(make_sequence<N>{});
  }};

/// multiplies `A` and `B`
/// @note 1. `xvmul(const Xv& A, const Xv& B)`
/// @note 2. `xvmul(const Xv (&A)[N], const Xv (&B)[N], Xv (&Result)[N])`
/// @note `Result` may be the same object as `A` or `B`.
inline constexpr overload xvmul{
  [](const xvector& A, const xvector& B) noexcept { return _mm_mul_ps(A, B); },
  [](const xwector& A, const xwector& B) noexcept { return _mm256_mul_pd(A, B); },
  [](const xrect& A, const xrect& B) noexcept { return _mm_mul_epi32(A, B); },
  []<natt N>(const xvector (&A)[N], const xvector (&B)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_mul_ps(A[Is], B[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], const xwector (&B)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_mul_pd(A[Is], B[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xrect (&A)[N], const xrect (&B)[N], xrect (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_mul_epi32(A[Is], B[Is])), ...); }(make_sequence<N>{});
  }};

/// devide `A` by `B`
/// @note 1. `xvdiv(const Xv& A, const Xv& B)`
/// @note 2. `xvdiv(const Xv (&A)[N], const Xv (&B)[N], Xv (&Result)[N])`
/// @note `Result` may be the same object as `A` or `B`.
inline constexpr overload xvdiv{
  [](const xvector& A, const xvector& B) noexcept { return _mm_div_ps(A, B); },
  [](const xwector& A, const xwector& B) noexcept { return _mm256_div_pd(A, B); },
  [](const xrect& A, const xrect& B) noexcept { return _mm_div_epi32(A, B); },
  []<natt N>(const xvector (&A)[N], const xvector (&B)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_div_ps(A[Is], B[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], const xwector (&B)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_div_pd(A[Is], B[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xrect (&A)[N], const xrect (&B)[N], xrect (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_div_epi32(A[Is], B[Is])), ...); }(make_sequence<N>{});
  }};

/// changes the specified elements of `A` to the absolute value
/// @note 1. `xvabsolute<X, Y, Z, W>(const Xv& A)`
/// @note 2. `xvabsolute<X, Y, Z, W>(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result` may be the same object as `A`.
template<bool X, bool Y, bool Z, bool W> inline constexpr overload xvabsolute{
  [](const xvector& A) noexcept { return xvblend<X, Y, Z, W>(A, xvabs(A)); },
  [](const xwector& A) noexcept { return xvblend<X, Y, Z, W>(A, xvabs(A)); },
  [](const xrect& A) noexcept { return xvblend<X, Y, Z, W>(A, xvabs(A)); },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = xvblend<X, Y, Z, W>(A[Is], xvabs(A[Is]))), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = xvblend<X, Y, Z, W>(A[Is], xvabs(A[Is]))), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xrect (&A)[N], xrect (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = xvblend<X, Y, Z, W>(A[Is], xvabs(A[Is]))), ...); }(make_sequence<N>{});
  }};

/// changes the signs of the specified elements of `A`
/// @note 1. `xvnegate<X, Y, Z, W>(const Xv& A)`
/// @note 2. `xvnegate<X, Y, Z, W>(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result` may be the same object as `A`.
template<bool X, bool Y, bool Z, bool W> inline constexpr overload xvnegate{
  [](const xvector& A) noexcept { return xvblend<X, Y, Z, W>(A, xvneg(A)); },
  [](const xwector& A) noexcept { return xvblend<X, Y, Z, W>(A, xvneg(A)); },
  [](const xrect& A) noexcept { return xvblend<X, Y, Z, W>(A, xvneg(A)); },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = xvblend<X, Y, Z, W>(A[Is], xvneg(A[Is]))), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = xvblend<X, Y, Z, W>(A[Is], xvneg(A[Is]))), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xrect (&A)[N], xrect (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = xvblend<X, Y, Z, W>(A[Is], xvneg(A[Is]))), ...); }(make_sequence<N>{});
  }};

/// performs fused multiply-add operation
/// @note 1. `xvfma(const Xv& A, const Xv& B, const Xv& C)`
/// @note 2. `xvfma(const Xv (&A)[N], const Xv (&B)[N], const Xv (&C)[N], Xv (&Result)[N])`
/// @note `Result` may be the same object as `A`, `B` or `C`.
inline constexpr overload xvfma{
  [](const xvector& A, const xvector& B, const xvector& C) noexcept { return _mm_fmadd_ps(A, B, C); },
  [](const xwector& A, const xwector& B, const xwector& C) noexcept { return _mm256_fmadd_pd(A, B, C); },
  [](const xrect& A, const xrect& B, const xrect& C) noexcept { return xvadd(xvmul(A, B), C); },
  []<natt N>(const xvector (&A)[N], const xvector (&B)[N], const xvector (&C)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_fmadd_ps(A[Is], B[Is], C[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], const xwector (&B)[N], const xwector (&C)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_fmadd_pd(A[Is], B[Is], C[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xrect (&A)[N], const xrect (&B)[N], const xrect (&C)[N], xrect (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = xvadd(xvmul(A[Is], B[Is]), C[Is])), ...); }(make_sequence<N>{});
  }};

/// performs fused multiply-subtract operation
/// @note 1. `xvfms(const Xv& A, const Xv& B, const Xv& C)`
/// @note 2. `xvfms(const Xv (&A)[N], const Xv (&B)[N], const Xv (&C)[N], Xv (&Result)[N])`
/// @note `Result` may be the same object as `A`, `B` or `C`.
inline constexpr overload xvfms{
  [](const xvector& A, const xvector& B, const xvector& C) noexcept { return _mm_fmsub_ps(A, B, C); },
  [](const xwector& A, const xwector& B, const xwector& C) noexcept { return _mm256_fmsub_pd(A, B, C); },
  [](const xrect& A, const xrect& B, const xrect& C) noexcept { return xvsub(xvmul(A, B), C); },
  []<natt N>(const xvector (&A)[N], const xvector (&B)[N], const xvector (&C)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_fmsub_ps(A[Is], B[Is], C[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], const xwector (&B)[N], const xwector (&C)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_fmsub_pd(A[Is], B[Is], C[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xrect (&A)[N], const xrect (&B)[N], const xrect (&C)[N], xrect (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = xvsub(xvmul(A[Is], B[Is]), C[Is])), ...); }(make_sequence<N>{});
  }};

/// performs fused negative multiply-add operation
/// @note 1. `xvfnma(const Xv& A, const Xv& B, const Xv& C)`
/// @note 2. `xvfnma(const Xv (&A)[N], const Xv (&B)[N], const Xv (&C)[N], Xv (&Result)[N])`
/// @note `Result` may be the same object as `A`, `B` or `C`.
inline constexpr overload xvfnma{
  [](const xvector& A, const xvector& B, const xvector& C) noexcept { return _mm_fnmadd_ps(A, B, C); },
  [](const xwector& A, const xwector& B, const xwector& C) noexcept { return _mm256_fnmadd_pd(A, B, C); },
  [](const xrect& A, const xrect& B, const xrect& C) noexcept { return xvsub(xvmul(xvneg(A), B), C); },
  []<natt N>(const xvector (&A)[N], const xvector (&B)[N], const xvector (&C)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_fnmadd_ps(A[Is], B[Is], C[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], const xwector (&B)[N], const xwector (&C)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_fnmadd_pd(A[Is], B[Is], C[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xrect (&A)[N], const xrect (&B)[N], const xrect (&C)[N], xrect (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = xvsub(xvmul(xvneg(A[Is]), B[Is]), C[Is])), ...); }(make_sequence<N>{});
  }};

/// performs fused negative multiply-subtract operation
/// @note 1. `xvfnms(const Xv& A, const Xv& B, const Xv& C)`
/// @note 2. `xvfnms(const Xv (&A)[N], const Xv (&B)[N], const Xv (&C)[N], Xv (&Result)[N])`
/// @note `Result` may be the same object as `A`, `B` or `C`.
inline constexpr overload xvfnms{
  [](const xvector& A, const xvector& B, const xvector& C) noexcept { return _mm_fnmsub_ps(A, B, C); },
  [](const xwector& A, const xwector& B, const xwector& C) noexcept { return _mm256_fnmsub_pd(A, B, C); },
  [](const xrect& A, const xrect& B, const xrect& C) noexcept { return xvsub(xvmul(xvneg(A), B), C); },
  []<natt N>(const xvector (&A)[N], const xvector (&B)[N], const xvector (&C)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_fnmsub_ps(A[Is], B[Is], C[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], const xwector (&B)[N], const xwector (&C)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_fnmsub_pd(A[Is], B[Is], C[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xrect (&A)[N], const xrect (&B)[N], const xrect (&C)[N], xrect (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = xvsub(xvmul(xvneg(A[Is]), B[Is]), C[Is])), ...); }(make_sequence<N>{});
  }};

/// performs ceiling operation
/// @note 1. `xvceil(const Xv& A)`
/// @note 2. `xvceil(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result` may be the same object as `A`.
inline constexpr overload xvceil{
  [](const xvector& A) noexcept { return _mm_ceil_ps(A); },
  [](const xwector& A) noexcept { return _mm256_ceil_pd(A); },
  [](const xrect& A) noexcept { return A; },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_ceil_ps(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_ceil_pd(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xrect (&A)[N], xrect (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = A[Is]), ...); }(make_sequence<N>{});
  }};

/// performs floor operation
/// @note 1. `xvfloor(const Xv& A)`
/// @note 2. `xvfloor(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result` may be the same object as `A`.
inline constexpr overload xvfloor{
  [](const xvector& A) noexcept { return _mm_floor_ps(A); },
  [](const xwector& A) noexcept { return _mm256_floor_pd(A); },
  [](const xrect& A) noexcept { return A; },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_floor_ps(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_floor_pd(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xrect (&A)[N], xrect (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = A[Is]), ...); }(make_sequence<N>{});
  }};

/// performs truncation operation
/// @note 1. `xvtrunc(const Xv& A)`
/// @note 2. `xvtrunc(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result` may be the same object as `A`.
inline constexpr overload xvtrunc{
  [](const xvector& A) noexcept { return _mm_trunc_ps(A); },
  [](const xwector& A) noexcept { return _mm256_trunc_pd(A); },
  [](const xrect& A) noexcept { return A; },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_trunc_ps(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_trunc_pd(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xrect (&A)[N], xrect (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = A[Is]), ...); }(make_sequence<N>{});
  }};

/// performs rounding operation
/// @note 1. `xvround(const Xv& A)`
/// @note 2. `xvround(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result` may be the same object as `A`.
/// @note 3. `xvround(const Xv& A, constant<int I>)`
/// @note 小数点から右側に`I`の桁数で四捨五入する
inline constexpr overload xvround{
  [](const xvector& A) noexcept { return _mm_round_ps(A, 8); },
  [](const xwector& A) noexcept { return _mm256_round_pd(A, 8); },
  [](const xrect& A) noexcept { return A; },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_round_ps(A[Is], 8)), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_round_pd(A[Is]), 8), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xrect (&A)[N], xrect (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = A[Is]), ...); }(make_sequence<N>{});
  }};

/// performs maximum operation
/// @note 1. `xvmax(const Xv& A, const Xv& B)`
/// @note 2. `xvmax(const Xv (&A)[N], const Xv (&B)[N], Xv (&Result)[N])`
/// @note `Result` may be the same object as `A` or `B`.
inline constexpr overload xvmax{
  [](const xvector& A, const xvector& B) noexcept { return _mm_max_ps(A, B); },
  [](const xwector& A, const xwector& B) noexcept { return _mm256_max_pd(A, B); },
  [](const xrect& A, const xrect& B) noexcept { return _mm_max_epi32(A, B); },
  []<natt N>(const xvector (&A)[N], const xvector (&B)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_max_ps(A[Is], B[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], const xwector (&B)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_max_pd(A[Is], B[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xrect (&A)[N], const xrect (&B)[N], xrect (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_max_epi32(A[Is], B[Is])), ...); }(make_sequence<N>{});
  }};

/// performs minimum operation
/// @note 1. `xvmin(const Xv& A, const Xv& B)`
/// @note 2. `xvmin(const Xv (&A)[N], const Xv (&B)[N], Xv (&Result)[N])`
/// @note `Result` may be the same object as `A` or `B`.
inline constexpr overload xvmin{
  [](const xvector& A, const xvector& B) noexcept { return _mm_min_ps(A, B); },
  [](const xwector& A, const xwector& B) noexcept { return _mm256_min_pd(A, B); },
  [](const xrect& A, const xrect& B) noexcept { return _mm_min_epi32(A, B); },
  []<natt N>(const xvector (&A)[N], const xvector (&B)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_min_ps(A[Is], B[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], const xwector (&B)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_min_pd(A[Is], B[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xrect (&A)[N], const xrect (&B)[N], xrect (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_min_epi32(A[Is], B[Is])), ...); }(make_sequence<N>{});
  }};

/// performs sin operation
/// @note 1. `xvsin(const Xv& A)`
/// @note 2. `xvsin(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result` may be the same object as `A`.
/// @note 3. `xvsin(const Xv& A, Xv& Cos)`
/// @note `Cos`には`cos`関数の結果が格納される
/// @note 4. `xvsin(const Xv (&A)[N], Xv (&Result)[N], Xv (&Cos)[N])`
/// @note `Result`あるいは`Cos`の参照先は`A`と同一であってもよい
inline constexpr overload xvsin{
  [](const xvector& A) noexcept { return _mm_sin_ps(A); },
  [](const xwector& A) noexcept { return _mm256_sin_pd(A); },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_sin_ps(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_sin_pd(A[Is])), ...); }(make_sequence<N>{});
  },
  [](const xvector& A, xvector& Cos) noexcept {
    return _mm_sincos_ps(&Cos, A);
  },
  [](const xwector& A, xwector& Cos) noexcept {
    return _mm256_sincos_pd(&Cos, A);
  },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N], xvector (&Cos)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_sincos_ps(&Cos[Is], A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N], xwector (&Cos)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_sin_pd(&Cos[Is], A[Is])), ...); }(make_sequence<N>{});
  }};

/// performs asin operation
/// @note 1. `xvasin(const Xv& A)`
/// @note 2. `xvasin(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result` may be the same object as `A`.
inline constexpr overload xvasin{
  [](const xvector& A) noexcept { return _mm_asin_ps(A); },
  [](const xwector& A) noexcept { return _mm256_asin_pd(A); },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_asin_ps(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_asin_pd(A[Is])), ...); }(make_sequence<N>{});
  }};

/// performs sinh operation
/// @note 1. `xvsinh(const Xv& A)`
/// @note 2. `xvsinh(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result` may be the same object as `A`.
inline constexpr overload xvsinh{
  [](const xvector& A) noexcept { return _mm_sinh_ps(A); },
  [](const xwector& A) noexcept { return _mm256_sinh_pd(A); },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_sinh_ps(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_sinh_pd(A[Is])), ...); }(make_sequence<N>{});
  }};

/// performs asinh operation
/// @note 1. `xvasinh(const Xv& A)`
/// @note 2. `xvasinh(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result` may be the same object as `A`.
inline constexpr overload xvasinh{
  [](const xvector& A) noexcept { return _mm_asinh_ps(A); },
  [](const xwector& A) noexcept { return _mm256_asinh_pd(A); },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_asinh_ps(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_asinh_pd(A[Is])), ...); }(make_sequence<N>{});
  }};

/// performs cos operation
/// @note 1. `xvcos(const Xv& A)`
/// @note 2. `xvcos(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result` may be the same object as `A`.
inline constexpr overload xvcos{
  [](const xvector& A) noexcept { return _mm_cos_ps(A); },
  [](const xwector& A) noexcept { return _mm256_cos_pd(A); },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_cos_ps(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_cos_pd(A[Is])), ...); }(make_sequence<N>{});
  }};

/// performs acos operation
/// @note 1. `xvacos(const Xv& A)`
/// @note 2. `xvacos(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result` may be the same object as `A`.
inline constexpr overload xvacos{
  [](const xvector& A) noexcept { return _mm_acos_ps(A); },
  [](const xwector& A) noexcept { return _mm256_acos_pd(A); },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_acos_ps(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_acos_pd(A[Is])), ...); }(make_sequence<N>{});
  }};

/// performs cosh operation
/// @note 1. `xvcosh(const Xv& A)`
/// @note 2. `xvcosh(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result` may be the same object as `A`.
inline constexpr overload xvcosh{
  [](const xvector& A) noexcept { return _mm_cosh_ps(A); },
  [](const xwector& A) noexcept { return _mm256_cosh_pd(A); },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_cosh_ps(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_cosh_pd(A[Is])), ...); }(make_sequence<N>{});
  }};

/// performs acosh operation
/// @note 1. `xvacosh(const Xv& A)`
/// @note 2. `xvacosh(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result` may be the same object as `A`.
inline constexpr overload xvacosh{
  [](const xvector& A) noexcept { return _mm_acosh_ps(A); },
  [](const xwector& A) noexcept { return _mm256_acosh_pd(A); },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_acosh_ps(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_acosh_pd(A[Is])), ...); }(make_sequence<N>{});
  }};

/// performs tan operation
/// @note 1. `xvtan(const Xv& A)`
/// @note 2. `xvtan(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result` may be the same object as `A`.
inline constexpr overload xvtan{
  [](const xvector& A) noexcept { return _mm_tan_ps(A); },
  [](const xwector& A) noexcept { return _mm256_tan_pd(A); },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_tan_ps(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_tan_pd(A[Is])), ...); }(make_sequence<N>{});
  }};

/// performs atan operation
/// @note 1. `xvatan(const Xv& A)`
/// @note 2. `xvatan(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result` may be the same object as `A` or `B`.
/// @note 3. `xvatan(const Xv& Sin, const Xv& Cos)`
/// @note 第2引数までを指定した場合、`atan2(Sin, Cos)`を計算する
/// @note 4. `xvatan(const Xv (&Sin)[N], const Xv (&Cos)[N], Xv (&Result)[N])`
/// @note `Result` may be the same object as `Sin` or `Cos`.
inline constexpr overload xvatan{
  [](const xvector& A) noexcept { return _mm_atan_ps(A); },
  [](const xwector& A) noexcept { return _mm256_atan_pd(A); },
  [](const xvector& Sin, const xvector& Cos) noexcept { return _mm_atan2_ps(Sin, Cos); },
  [](const xwector& Sin, const xwector& Cos) noexcept { return _mm256_atan2_pd(Sin, Cos); },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_atan_ps(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_atan_pd(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xvector (&Sin)[N], const xvector (&Cos)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_atan2_ps(Sin[Is], Cos[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&Sin)[N], const xwector (&Cos)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_atan2_pd(Sin[Is], Cos[Is])), ...); }(make_sequence<N>{});
  }};

/// performs tanh operation
/// @note 1. `xvtanh(const Xv& A)`
/// @note 2. `xvtanh(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result` may be the same object as `A`.
inline constexpr overload xvtanh{
  [](const xvector& A) noexcept { return _mm_tanh_ps(A); },
  [](const xwector& A) noexcept { return _mm256_tanh_pd(A); },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_tanh_ps(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_tanh_pd(A[Is])), ...); }(make_sequence<N>{});
  }};

/// performs atanh operation
/// @note 1. `xvatanh(const Xv& A)`
/// @note 2. `xvatanh(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result` may be the same object as `A`.
inline constexpr overload xvatanh{
  [](const xvector& A) noexcept { return _mm_atanh_ps(A); },
  [](const xwector& A) noexcept { return _mm256_atanh_pd(A); },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_atanh_ps(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_atanh_pd(A[Is])), ...); }(make_sequence<N>{});
  }};

/// performs pow operation
/// @note 1. `xvpow(const Xv& A, const Xv& B)`
/// @note 2. `xvpow(const Xv (&A)[N], const Xv (&B)[N], Xv (&Result)[N])`
/// @note `Result` may be the same object as `A` or `B`.
inline constexpr overload xvpow{
  [](const xvector& A, const xvector& B) noexcept { return _mm_pow_ps(A, B); },
  [](const xwector& A, const xwector& B) noexcept { return _mm256_pow_pd(A, B); },
  []<natt N>(const xvector (&A)[N], const xvector (&B)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_pow_ps(A[Is], B[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], const xwector (&B)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_pow_pd(A[Is], B[Is])), ...); }(make_sequence<N>{});
  }};

/// performs exp operation
/// @note 1. `xvexp(const Xv& A)`
/// @note 2. `xvexp(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result` may be the same object as `A` or `B`.
inline constexpr overload xvexp{
  [](const xvector& A) noexcept { return _mm_exp_ps(A); },
  [](const xwector& A) noexcept { return _mm256_exp_pd(A); },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_exp_ps(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_exp_pd(A[Is])), ...); }(make_sequence<N>{});
  }};

/// performs exp2 operation
/// @note 1. `xvexp2(const Xv& A)`
/// @note 2. `xvexp2(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result` may be the same object as `A` or `B`.
inline constexpr overload xvexp2{
  [](const xvector& A) noexcept { return _mm_exp2_ps(A); },
  [](const xwector& A) noexcept { return _mm256_exp2_pd(A); },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_exp2_ps(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_exp2_pd(A[Is])), ...); }(make_sequence<N>{});
  }};

/// performs exp10 operation
/// @note 1. `xvexp10(const Xv& A)`
/// @note 2. `xvexp10(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result` may be the same object as `A` or `B`.
inline constexpr overload xvexp10{
  [](const xvector& A) noexcept { return _mm_exp10_ps(A); },
  [](const xwector& A) noexcept { return _mm256_exp10_pd(A); },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_exp10_ps(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_exp10_pd(A[Is])), ...); }(make_sequence<N>{});
  }};

/// performs expm1 operation
/// @note 1. `xvexpm1(const Xv& A)`
/// @note 2. `xvexpm1(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result` may be the same object as `A` or `B`.
inline constexpr overload xvexpm1{
  [](const xvector& A) noexcept { return _mm_expm1_ps(A); },
  [](const xwector& A) noexcept { return _mm256_expm1_pd(A); },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_expm1_ps(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_expm1_pd(A[Is])), ...); }(make_sequence<N>{});
  }};

/// performs natural logarithm operation
/// @note 1. `xvln(const Xv& A)`
/// @note 2. `xvln(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result` may be the same object as `A`.
inline constexpr overload xvln{
  [](const xvector& A) noexcept { return _mm_log_ps(A); },
  [](const xwector& A) noexcept { return _mm256_log_pd(A); },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_log_ps(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_log_pd(A[Is])), ...); }(make_sequence<N>{});
  }};

/// performs logarithm operation
/// @note 1. `xvlog(const Xv& A, const Xv& Base)`
/// @note 2. `xvlog(const Xv (&A)[N], const Xv (&Base)[N], Xv (&Result)[N])`
/// @note `Result` may be the same object as `A` or `Base`.
inline constexpr overload xvlog{
  [](const xvector& A, const xvector& Base) noexcept { return xvdiv(xvln(A), xvln(Base)); },
  [](const xwector& A, const xwector& Base) noexcept { return xvdiv(xvln(A), xvln(Base)); },
  []<natt N>(const xvector (&A)[N], const xvector (&Base)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = xvdiv(xvln(A[Is]), xvln(Base[Is]))), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], const xwector (&Base)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = xvdiv(xvln(A[Is]), xvln(Base[Is]))), ...); }(make_sequence<N>{});
  }};

/// performs log2 operation
/// @note 1. `xvlog2(const Xv& A)`
/// @note 2. `xvlog2(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result` may be the same object as `A`.
inline constexpr overload xvlog2{
  [](const xvector& A) noexcept { return _mm_log2_ps(A); },
  [](const xwector& A) noexcept { return _mm256_log2_pd(A); },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_log2_ps(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_log2_pd(A[Is])), ...); }(make_sequence<N>{});
  }};

/// performs log10 operation
/// @note 1. `xvlog10(const Xv& A)`
/// @note 2. `xvlog10(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result` may be the same object as `A`.
inline constexpr overload xvlog10{
  [](const xvector& A) noexcept { return _mm_log10_ps(A); },
  [](const xwector& A) noexcept { return _mm256_log10_pd(A); },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_log10_ps(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_log10_pd(A[Is])), ...); }(make_sequence<N>{});
  }};

/// performs log1p operation
/// @note 1. `xvlog1p(const Xv& A)`
/// @note 2. `xvlog1p(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result` may be the same object as `A`.
inline constexpr overload xvlog1p{
  [](const xvector& A) noexcept { return _mm_log1p_ps(A); },
  [](const xwector& A) noexcept { return _mm256_log1p_pd(A); },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_log1p_ps(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_log1p_pd(A[Is])), ...); }(make_sequence<N>{});
  }};

/// performs logb operation
/// @note 1. `xvlogb(const Xv& A)`
/// @note 2. `xvlogb(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result` may be the same object as `A` or `B`.
inline constexpr overload xvlogb{
  [](const xvector& A) noexcept { return _mm_logb_ps(A); },
  [](const xwector& A) noexcept { return _mm256_logb_pd(A); },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_logb_ps(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_logb_pd(A[Is])), ...); }(make_sequence<N>{});
  }};

/// performs sqrt operation
/// @note 1. `xvsqrt(const Xv& A)`
/// @note 2. `xvsqrt(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result` may be the same object as `A` or `B`.
inline constexpr overload xvsqrt{
  [](const xvector& A) noexcept { return _mm_sqrt_ps(A); },
  [](const xwector& A) noexcept { return _mm256_sqrt_pd(A); },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_sqrt_ps(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_sqrt_pd(A[Is])), ...); }(make_sequence<N>{});
  }};

/// performs cbrt operation
/// @note 1. `xvcbrt(const Xv& A)`
/// @note 2. `xvcbrt(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result` may be the same object as `A` or `B`.
inline constexpr overload xvcbrt{
  [](const xvector& A) noexcept { return _mm_cbrt_ps(A); },
  [](const xwector& A) noexcept { return _mm256_cbrt_pd(A); },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_cbrt_ps(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_cbrt_pd(A[Is])), ...); }(make_sequence<N>{});
  }};

/// performs hypothenuse operation
/// @note 1. `xvhypot(const Xv& A, const Xv& B)`
/// @note 2. `xvhypot(const Xv (&A)[N], const Xv (&B)[N], Xv (&Result)[N])`
/// @note `Result` may be the same object as `A` or `B`.
inline constexpr overload xvhypot{
  [](const xvector& A, const xvector& B) noexcept { return _mm_hypot_ps(A, B); },
  [](const xwector& A, const xwector& B) noexcept { return _mm256_hypot_pd(A, B); },
  []<natt N>(const xvector (&A)[N], const xvector (&B)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_hypot_ps(A[Is], B[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], const xwector (&B)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_hypot_pd(A[Is], B[Is])), ...); }(make_sequence<N>{});
  }};

/// performs reciprocal square root operation
/// @note 1. `xvsqrt_r(const Xv& A)`
/// @note 2. `xvsqrt_r(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result` may be the same object as `A`.
inline constexpr overload xvsqrt_r{
  [](const xvector& A) noexcept { return _mm_invsqrt_ps(A); },
  [](const xwector& A) noexcept { return _mm256_invsqrt_pd(A); },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_invsqrt_ps(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_invsqrt_pd(A[Is])), ...); }(make_sequence<N>{});
  }};

/// performs reciprocal cube root operation
/// @note 1. `xvcbrt_r(const Xv& A)`
/// @note 2. `xvcbrt_r(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result` may be the same object as `A`.
inline constexpr overload xvcbrt_r{
  [](const xvector& A) noexcept { return _mm_invcbrt_ps(A); },
  [](const xwector& A) noexcept { return _mm256_invcbrt_pd(A); },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_invcbrt_ps(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_invcbrt_pd(A[Is])), ...); }(make_sequence<N>{});
  }};

/// performs erf operation
/// @note 1. `xverf(const Xv& A)`
/// @note 2. `xverf(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result` may be the same object as `A`.
inline constexpr overload xverf{
  [](const xvector& A) noexcept { return _mm_erf_ps(A); },
  [](const xwector& A) noexcept { return _mm256_erf_pd(A); },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_erf_ps(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_erf_pd(A[Is])), ...); }(make_sequence<N>{});
  }};

/// performs erfc operation
/// @note 1. `xverfc(const Xv& A)`
/// @note 2. `xverfc(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result` may be the same object as `A`.
inline constexpr overload xverfc{
  [](const xvector& A) noexcept { return _mm_erfc_ps(A); },
  [](const xwector& A) noexcept { return _mm256_erfc_pd(A); },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_erfc_ps(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_erfc_pd(A[Is])), ...); }(make_sequence<N>{});
  }};

/// performs reciprocal erf operation
/// @note 1. `xverf_r(const Xv& A)`
/// @note 2. `xverf_r(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result` may be the same object as `A`.
inline constexpr overload xverf_r{
  [](const xvector& A) noexcept { return _mm_erfinv_ps(A); },
  [](const xwector& A) noexcept { return _mm256_erfinv_pd(A); },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_erfinv_ps(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_erfinv_pd(A[Is])), ...); }(make_sequence<N>{});
  }};

/// performs reciprocal erfc operation
/// @note 1. `xverfc_r(const Xv& A)`
/// @note 2. `xverfc_r(const Xv (&A)[N], Xv (&Result)[N])`
/// @note `Result` may be the same object as `A`.
inline constexpr overload xverfc_r{
  [](const xvector& A) noexcept { return _mm_erfcinv_ps(A); },
  [](const xwector& A) noexcept { return _mm256_erfcinv_pd(A); },
  []<natt N>(const xvector (&A)[N], xvector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm_erfcinv_ps(A[Is])), ...); }(make_sequence<N>{});
  },
  []<natt N>(const xwector (&A)[N], xwector (&Result)[N]) noexcept {
    return [&]<natt... Is>(sequence<Is...>) noexcept { return ((Result[Is] = _mm256_erfcinv_pd(A[Is])), ...); }(make_sequence<N>{});
  }};

template<natt N, natt Zero = 0> requires(0 < N && N <= 4 && Zero < 16) struct t_xvdot {
  xvector operator()(const xvector& A, const xvector& B) const noexcept {
    constexpr natt fz = inspects<!(Zero & 1), !(Zero & 2), !(Zero & 4), !(Zero & 8)>;
    if constexpr (N == 1 && (Zero == 14 || Zero == 13 || Zero == 11 || Zero == 7)) return xvinsert<fz, 0, Zero>(A, xvmul(A, B));
    else return _mm_dp_ps(A, B, value_switch<N, 0, 16, 48, 112, 240> | (15 ^ Zero));
  }
  xwector operator()(const xwector& A, const xwector& B) const noexcept {
    if constexpr (N == 0 || Zero == 0b1111) return xv_zero;
    xwector a;
    if constexpr (N == 1) a = xvmul(A, B);
    else if constexpr (N == 2) a = _mm256_hadd_pd(xvmul(A, B), A);
    else if constexpr (N == 3) (a = xvmul(A, B)), a = xvadd(_mm256_hadd_pd(a, A), _mm256_castpd128_pd256(_mm256_extractf128_pd(a, 1)));
    else if constexpr (N == 4) (a = _mm256_hadd_pd(xvmul(A, B), A)), a = xvadd(a, _mm256_castpd128_pd256(_mm256_extractf128_pd(a, 1)));
    return xvsetzero<Zero & 1, Zero & 2, Zero & 4, Zero & 8>(xvpermute<0, 0, 0, 0>(a));
  }
  template<natt M> requires(M <= 4) void operator()(const xvector (&A)[M], const xvector& B, xvector& Result) noexcept {
    if constexpr (1 <= N) Result = xvdot<N, 0b1110>(A[0], B);
    if constexpr (2 <= N) Result = xvpermute<0, 5, 2, 3>(Result, xvdot<N>(A[1], B));
    if constexpr (3 <= N) Result = xvpermute<0, 1, 6, 3>(Result, xvdot<N>(A[2], B));
    if constexpr (4 == N) Result = xvpermute<0, 1, 2, 7>(Result, xvdot<N>(A[3], B));
  }
  template<natt M> requires(M <= 4) void operator()(const xvector& A, const xvector (&B)[M], xvector& Result) noexcept {
    if constexpr (1 <= M) Result = xvmul(xvpermute<0, 0, 0, 0>(A), B[0]);
    if constexpr (2 <= M) Result = xvfma(xvpermute<1, 1, 1, 1>(A), B[1], Result);
    if constexpr (3 <= M) Result = xvfma(xvpermute<2, 2, 2, 2>(A), B[2], Result);
    if constexpr (4 == M) Result = xvfma(xvpermute<3, 3, 3, 3>(A), B[3], Result);
    if constexpr (N != 4) Result = _mm_blend_ps(xv_zero, Result, (1 << N) - 1);
  }
  template<natt M, natt L> requires(M <= 4 && L <= 4)
  void operator()(const xvector (&A)[L], const xvector (&B)[M], xvector (&Result)[L]) noexcept {
    if constexpr (1 <= L) t_xvdot<N, Zero>{}(A[0], B, L[0]);
    if constexpr (2 <= L) t_xvdot<N, Zero>{}(A[1], B, L[1]);
    if constexpr (3 <= L) t_xvdot<N, Zero>{}(A[2], B, L[2]);
    if constexpr (4 == L) t_xvdot<N, Zero>{}(A[3], B, L[3]);
  }
};

/// performs dot product operation
/// @note `xvdot<3, 0b1101>({1,2,3,4}, {1,2,3,4})` -> `{14, 0, 14, 14}`
/// @note `xvdot<3>({{1,2,3,4},{5,6,7,8}}, {1,2,3,4}, R)` -> `R{14,38,0,0}`
/// @note `ywdot<3>({{1,2,3,4},{5,6,7,8}}, {{1,2,3,4},{5,6,7,8},{9,10,11,12}}, R)` -> `R{{38,44,50,0},{98,116,134,0}}`
template<natt N, natt Zero = 0> requires(0 < N && N <= 4 && Zero < 16) inline constexpr t_xvdot<N> xvdot;

/// performs cross product operation
inline constexpr overload xvcross{
  [](const xvector& A, const xvector& B) noexcept {
    auto a = xvpermute<2, 0, 1, 3>(A), b = xvpermute<1, 2, 0, 3>(B);
    return xvfms(xvpermute<1, 2, 0, 3>(A), xvpermute<2, 0, 1, 3>(B), xvmul(a, b));
  },
  [](const xwector& A, const xwector& B) noexcept {
    auto a = xvpermute<2, 0, 1, 3>(A), b = xvpermute<1, 2, 0, 3>(B);
    return xvfms(xvpermute<1, 2, 0, 3>(A), xvpermute<2, 0, 1, 3>(B), xvmul(a, b));
  }};

/// calculates the length of the vector
template<natt N, natt Zero = 0> requires(N <= 4 && Zero < 16)
inline constexpr overload xvlength{[](const xvector& A) noexcept { return xvsqrt(xvdot<N, Zero>(A, A)); },
                                   [](const xwector& A) noexcept { return xvsqrt(xvdot<N, Zero>(A, A)); }};

/// calculates the reciprocal of the length of the vector
template<natt N, natt Zero = 0> requires(N <= 4 && Zero < 16)
inline constexpr overload xvlength_r{[](const xvector& A) noexcept { return xvsqrt_r(xvdot<N, Zero>(A, A)); },
                                     [](const xwector& A) noexcept { return xvsqrt_r(xvdot<N, Zero>(A, A)); }};

/// normalizes the vector
template<natt N> requires(N <= 4) inline constexpr overload xvnormalize{
  [](const xvector& A) noexcept { return xvmul(A, xvlength_r<N>(A)); },
  [](const xwector& A) noexcept { return xvmul(A, xvlength_r<N>(A)); }};

/// converts degrees to radians
inline constexpr overload xvradian{
  [](const xvector& A) noexcept { return xvmul(A, xv_constant<1. / 180. * pi>); },
  [](const xwector& A) noexcept { return xvmul(A, xv_constant<1. / 180. * pi>); }};

/// converts radians to degrees
inline constexpr overload xvdegree{
  [](const xvector& A) noexcept { return xvmul(A, xv_constant<180. / pi>); },
  [](const xwector& A) noexcept { return xvmul(A, xv_constant<180. / pi>); }};

// clang-format off

/// XV行列を転置する
inline constexpr overload xvtranspose{
  []<typename T, typename U>(T&& A, U&& Result) noexcept
    requires((nt_tuple_for<T, const xvector&> && nt_tuple_for<U, xvector&>) ||
             (nt_tuple_for<T, const xwector&> && nt_tuple_for<U, xwector&>)) {
    constexpr natt M = extent<T>, N = extent<U>;
    if constexpr (1 == M) {
      if constexpr (1 <= N) get<0>(Result) = xvsetzero<0, 1, 1, 1>(get<0>(A));
      if constexpr (2 <= N) get<1>(Result) = xvinsert<0, 1, 0b1110>(get<0>(A), get<0>(A));
      if constexpr (3 <= N) get<2>(Result) = xvinsert<0, 2, 0b1110>(get<0>(A), get<0>(A));
      if constexpr (4 == N) get<3>(Result) = xvinsert<0, 3, 0b1110>(get<0>(A), get<0>(A));
    } else if constexpr (2 == M) {
      if constexpr (1 <= N) get<0>(Result) = xvinsert<1, 0, 0b1100>(get<0>(A), get<1>(A));
      if constexpr (2 <= N) get<1>(Result) = xvinsert<0, 1, 0b1100>(get<1>(A), get<0>(A));
      if constexpr (3 <= N) get<2>(Result) = xvinsert<1, 2, 0b1100>(xvpermute<2, -1, -1, -1>(get<0>(A)), get<1>(A));
      if constexpr (4 == N) get<3>(Result) = xvinsert<1, 3, 0b1100>(xvpermute<3, -1, -1, -1>(get<0>(A)), get<1>(A));
    } else if constexpr (3 == M) {
      if constexpr (1 <= N) get<0>(Result) = xvinsert<2, 0, 0b1000>(xvpermute<0, 4, -1, -1>(get<0>(A), get<1>(A)), get<2>(A));
      if constexpr (2 <= N) get<1>(Result) = xvinsert<2, 1, 0b1000>(xvpermute<1, 5, -1, -1>(get<0>(A), get<1>(A)), get<2>(A));
      if constexpr (3 <= N) get<2>(Result) = xvinsert<2, 2, 0b1000>(xvpermute<2, 6, -1, -1>(get<0>(A), get<1>(A)), get<2>(A));
      if constexpr (4 == N) get<3>(Result) = xvinsert<2, 3, 0b1000>(xvpermute<3, 7, -1, -1>(get<0>(A), get<1>(A)), get<2>(A));
    } else {
      if constexpr (N == 1) get<0>(Result) = xvpermute<0, 1, 4, 5>(xvpermute<0, 4, -1, -1>(get<0>(A), get<1>(A)), xvpermute<0, 4, -1, -1>(get<2>(A), get<3>(A)));
      if constexpr (N >= 2) {
        auto a = xvpermute<0, 1, 4, 5>(get<0>(A), get<1>(A)), b = xvpermute<0, 1, 4, 5>(get<2>(A), get<3>(A));
        get<0>(Result) = xvpermute<0, 2, 4, 6>(a, b), get<1>(Result) = xvpermute<1, 3, 5, 7>(a, b);
      }
      if constexpr (N == 3) get<3>(Result) = xvpermute<0, 1, 4, 5>(xvpermute<2, 6, -1, -1>(get<0>(A), get<1>(A)), xvpermute<2, 6, -1, -1>(get<2>(A), get<3>(A)));
      if constexpr (N == 4) {
        auto a = xvpermute<2, 3, 6, 7>(get<0>(A), get<1>(A)), b = xvpermute<2, 3, 6, 7>(get<2>(A), get<3>(A));
        get<2>(Result) = xvpermute<0, 2, 4, 6>(a, b), get<3>(Result) = xvpermute<1, 3, 5, 7>(a, b);
      }
    }
  }};

// clang-format on

/// Generates a transformation matrix for translation.
inline constexpr auto xvtranslate =
  []<typename T>(const xvector& Offset, T&& Result) noexcept requires(nt_tuple_for<T, xvector&>) {
    constexpr natt N = extent<T>;
    if constexpr (N >= 1) get<0>(Result) = xvinsert<3, 0>(xv_x, Offset);
    if constexpr (N >= 2) get<1>(Result) = xvinsert<3, 1>(xv_y, Offset);
    if constexpr (N >= 3) get<2>(Result) = xvinsert<3, 2>(xv_z, Offset);
    if constexpr (N >= 4) get<3>(Result) = xv_w;
  };

/// Generates a transformation matrix for rotation.
/// @note If `N == 2`, the two-dimensional rotation matrix at `Radian[0]`.
/// @note If `N >= 3`, the three-dimensional rotation matrix at `Radian[0-2]`.
/// @note If `N == 4`, `Result[3] = {0,0,0,1}`
inline constexpr auto xvrotate = []<nt_tuple_for<xvector&> T>(const xvector& Radian, T&& Result) noexcept {
  constexpr natt N = extent<T>;
  if constexpr (N == 2) {
    get<1>(Result) = xvsin(Radian, get<0>(Result));
    get<0>(Result) = xvinsert<1, 0, 0b1100>(get<0>(Result), get<1>(Result));
    get<1>(Result) = xvinsert<1, 0, 0b1100>(get<1>(Result), get<0>(Result));
    get<0>(Result) = xvnegate<0, 1, 0, 0>(get<0>(Result));
  } else if constexpr (N == 3) {
    get<0>(Result) = xvsin(Radian, get<1>(Result));
    get<2>(Result) = xvpermute<4, 6, 0, 2>(get<0>(Result), get<1>(Result));
    auto temp = xvpermute<3, 0, 1, 2>(get<2>(Result));
    get<2>(Result) = xvmul(get<2>(Result), temp);
    get<1>(Result) = xvmul(xvpermute<1, 1, 1, 1>(get<1>(Result)), temp);
    get<0>(Result) = xvpermute<1, 1, 1, 1>(get<0>(Result));
    temp = _mm_addsub_ps(xvpermute<2, 3, 0, 1>(xvmul(get<0>(Result), get<2>(Result))), get<2>(Result));
    get<2>(Result) = xvinsert<0, 0, 0b1000>(xvpermute<-1, 3, 1, -1>(get<1>(Result)), xvneg(get<0>(Result)));
    get<0>(Result) = xvinsert<0, 2, 0b1000>(xvpermute<-1, 0, 3, -1>(temp), get<1>(Result));
    get<1>(Result) = xvinsert<0, 0, 0b1000>(temp, get<1>(Result));
  } else if constexpr (N == 4) {
    get<0>(Result) = xvsin(Radian, get<1>(Result));
    get<2>(Result) = xvpermute<4, 6, 0, 2>(get<0>(Result), get<1>(Result));
    get<3>(Result) = xvpermute<3, 0, 1, 2>(get<2>(Result));
    get<2>(Result) = xvmul(get<2>(Result), get<3>(Result));
    get<1>(Result) = xvmul(xvpermute<1, 1, 1, 1>(get<1>(Result)), get<3>(Result));
    get<0>(Result) = xvpermute<1, 1, 1, 1>(get<0>(Result));
    get<3>(Result) = _mm_addsub_ps(xvpermute<2, 3, 0, 1>(xvmul(get<0>(Result), get<2>(Result))), get<2>(Result));
    get<2>(Result) = xvinsert<0, 0, 0b1000>(xvpermute<-1, 3, 1, -1>(get<1>(Result)), xvneg(get<0>(Result)));
    get<0>(Result) = xvinsert<0, 2, 0b1000>(xvpermute<-1, 0, 3, -1>(get<3>(Result)), get<1>(Result));
    get<1>(Result) = xvinsert<0, 0, 0b1000>(get<3>(Result), get<1>(Result));
    get<3>(Result) = xv_w;
  } else static_assert(1 < N && N < 5);
};

/// Generates the inverse matrix of the transformation matrix generated by `xvrotate`.
inline constexpr auto xvrotate_inv = []<nt_tuple_for<xvector&> T>(const xvector& Radian, T&& Result) noexcept {
  constexpr natt N = extent<T>;
  if constexpr (N == 2) {
    get<1>(Result) = xvsin(Radian, get<0>(Result));
    get<0>(Result) = xvinsert<1, 0, 0b1100>(get<0>(Result), get<1>(Result));
    get<1>(Result) = xvinsert<1, 0, 0b1100>(get<1>(Result), get<0>(Result));
    get<1>(Result) = xvnegate<0, 1, 0, 0>(get<1>(Result));
  } else if constexpr (N == 3) {
    get<0>(Result) = xvsin(Radian, get<1>(Result));
    get<2>(Result) = xvpermute<4, 6, 0, 2>(get<0>(Result), get<1>(Result));
    auto temp = xvpermute<3, 0, 1, 2>(get<2>(Result));
    get<2>(Result) = xvmul(get<2>(Result), temp);
    get<1>(Result) = xvmul(xvpermute<1, 1, 1, 1>(get<1>(Result)), temp);
    get<0>(Result) = xvpermute<1, 1, 1, 1>(get<0>(Result));
    temp = _mm_addsub_ps(xvpermute<2, 3, 0, 1>(xvmul(get<0>(Result), get<2>(Result))), get<2>(Result));
    get<2>(Result) = xvinsert<2, 1, 0b1000>(xvpermute<3, 2, -1, -1>(temp), get<1>(Result));
    get<0>(Result) = xvinsert<2, 2, 0b1000>(xvpermute<2, 0, -1, -1>(get<1>(Result)), xvneg(get<0>(Result)));
    get<1>(Result) = xvinsert<2, 3, 0b1000>(temp, get<1>(Result));
  } else if constexpr (N == 4) {
    get<0>(Result) = xvsin(Radian, get<1>(Result));
    get<2>(Result) = xvpermute<4, 6, 0, 2>(get<0>(Result), get<1>(Result));
    get<3>(Result) = xvpermute<3, 0, 1, 2>(get<2>(Result));
    get<2>(Result) = xvmul(get<2>(Result), get<3>(Result));
    get<1>(Result) = xvmul(xvpermute<1, 1, 1, 1>(get<1>(Result)), get<3>(Result));
    get<0>(Result) = xvpermute<1, 1, 1, 1>(get<0>(Result));
    get<3>(Result) = _mm_addsub_ps(xvpermute<2, 3, 0, 1>(xvmul(get<0>(Result), get<2>(Result))), get<2>(Result));
    get<2>(Result) = xvinsert<2, 1, 0b1000>(xvpermute<3, 2, -1, -1>(get<3>(Result)), get<1>(Result));
    get<0>(Result) = xvinsert<2, 2, 0b1000>(xvpermute<2, 0, -1, -1>(get<1>(Result)), xvneg(get<0>(Result)));
    get<1>(Result) = xvinsert<2, 3, 0b1000>(get<3>(Result), get<1>(Result));
    get<3>(Result) = xv_w;
  } else static_assert(1 < N && N < 5);
};

/// Generates a transformation matrix for scaling.
inline constexpr auto xvscale = []<nt_tuple_for<xvector&> T>(const xvector& Scalar, T&& Result) noexcept {
  constexpr natt N = extent<T>;
  if constexpr (N >= 2) {
    get<0>(Result) = xvinsert<0, 0, 0b1110>(Scalar, Scalar);
    get<1>(Result) = xvinsert<1, 1, 0b1101>(Scalar, Scalar);
  }
  if constexpr (N >= 3) get<2>(Result) = xvinsert<2, 2, 0b1011>(Scalar, Scalar);
  if constexpr (N == 4) get<3>(Result) = xv_w;
  static_assert(1 < N && N < 5);
};

// clang-format off

/// Generates a transformation matrix.
/// @note `xvworld(Result) -> Result = xv_identity`
/// @note `xvworld(Offset, Result) -> Result = xvtranslate(Offset)`
/// @note `xvworld(Offset, Radian, Result) -> Result = xvtranslate(Offset) * xvrotate(Radian)`
/// @note `xvworld(Offset, Radian, Scalar, Result) -> Result = xvtranslate(Offset) * xvrotate(Radian) * xvscale(Scalar)`
inline constexpr overload xvworld{
  []<nt_tuple_for<xvector&> T>(T&& Result) noexcept{
    constexpr natt N = extent<T>;
    if constexpr (N >= 2) Result[0] = xv_x, Result[1] = xv_y;
    if constexpr (N >= 3) Result[2] = xv_z;
    if constexpr (N == 4) Result[3] = xv_w;
    static_assert(1 < N && N < 5);
  },
  []<nt_tuple_for<xvector&> T>(const xvector& Offset, T&& Result) noexcept { xvtranslate(Offset, Result); },
  []<nt_tuple_for<xvector&> T>(const xvector& Offset, const xvector& Radian, T&& Result) noexcept {
    constexpr natt N = extent<T>;
    xvrotate(Radian, Result);
    Result[0] = xvinsert<3, 0>(Result[0], Offset);
    Result[1] = xvinsert<3, 1>(Result[1], Offset);
    if constexpr (N >= 3) Result[2] = xvinsert<3, 2>(Result[2], Offset);
    static_assert(1 < N && N < 5);
  },
  []<nt_tuple_for<xvector&> T>(const xvector& Offset, const xvector& Radian, const xvector& Scalar, T&& Result) noexcept {
    constexpr natt N = extent<T>;
    xvrotate(Radian, Result);
    Result[0] = xvinsert<3, 0>(Result[0], Offset);
    Result[1] = xvinsert<3, 1>(Result[1], Offset);
    if constexpr (N >= 3) Result[2] = xvinsert<3, 2>(Result[2], Offset);
    Result[0] = xvmul(Result[0], xvpermute<0, 0, 0, 0>(Scalar));
    Result[1] = xvmul(Result[1], xvpermute<1, 1, 1, 1>(Scalar));
    if constexpr (N >= 3) Result[2] = xvmul(Result[2], xvpermute<2, 2, 2, 2>(Scalar));
    static_assert(1 < N && N < 5);
  }};

/// creates a view matrix
/// @note `xvview(Radian, Result) -> Result = xvrotate_inv(Radian)`
/// @note `xvview(Position, Radian, Result) -> Result = xvrotate_inv(Radian) * xvtranslate(-Position)`
/// @note `xvview(Position, Radian, Offset, Result) -> Result = xvtranslate(-Offset) * xvrotate_inv(Radian) * xvtranslate(-Position)`
inline constexpr overload xvview{
  []<nt_tuple_for<xvector&> T, natt N = extent<T>>(const xvector& Radian, T&& Result)
    noexcept requires (le(2, N) && le(N, 4)) { xvrotate_inv(Radian, Result); },
  []<nt_tuple_for<xvector&> T, natt N = extent<T>>(const xvector& Position, const xvector& Radian, T&& Result)
    noexcept requires (le(2, N) && le(N, 4)) { xvrotate_inv(Radian, Result);
    if constexpr (N < 4) {
      auto temp = xvneg(Position);
      Result[0] = xvinsert<3, 3>(Result[0], xvdot<3>(Result[0], temp));
      Result[1] = xvinsert<3, 3>(Result[1], xvdot<3>(Result[1], temp));
      if constexpr (N == 3) Result[2] = xvinsert<3, 3>(Result[2], xvdot<3>(Result[2], temp));
    } else {
      Result[3] = xvneg(Position);
      Result[0] = xvinsert<3, 3>(Result[0], xvdot<3>(Result[0], Result[3]));
      Result[1] = xvinsert<3, 3>(Result[1], xvdot<3>(Result[1], Result[3]));
      Result[2] = xvinsert<3, 3>(Result[2], xvdot<3>(Result[2], Result[3]));
      Result[3] = xv_w;
    } },
  []<nt_tuple_for<xvector&> T, natt N = extent<T>>(const xvector& Position, const xvector& Radian, const xvector& Offset, T&& Result)
    noexcept requires (le(2, N) && le(N, 4)) { xvrotate_inv(Radian, Result);
    if constexpr (N < 4) {
      auto temp = xvneg(Position);
      Result[0] = xvsub(xvinsert<3, 3>(Result[0], xvdot<3>(Result[0], temp)), xvinsert<3, 0, 0b0111>(Offset, Offset));
      Result[1] = xvsub(xvinsert<3, 3>(Result[1], xvdot<3>(Result[1], temp)), xvinsert<3, 1, 0b0111>(Offset, Offset));
      if constexpr (N == 3) Result[2] = xvsub(xvinsert<3, 3>(Result[2], xvdot<3>(Result[2], temp)), xvinsert<3, 2, 0b0111>(Offset, Offset));
    } else {
      Result[3] = xvneg(Position);
      Result[0] = xvsub(xvinsert<3, 3>(Result[0], xvdot<3>(Result[0], Result[3])), xvinsert<3, 0, 0b0111>(Offset, Offset));
      Result[1] = xvsub(xvinsert<3, 3>(Result[1], xvdot<3>(Result[1], Result[3])), xvinsert<3, 1, 0b0111>(Offset, Offset));
      Result[2] = xvsub(xvinsert<3, 3>(Result[2], xvdot<3>(Result[2], Result[3])), xvinsert<3, 2, 0b0111>(Offset, Offset));
      Result[3] = xv_w;
    } } };

/// creates a camera matrix
/// @note 入力用の引数が３個の場合、Perspectiveなカメラ行列を生成する
/// @note 入力用の引数が４個の場合、Orthographicなカメラ行列を生成する
inline constexpr overload xvcamera{
  []<natt N>(const fat4 Width, const fat4 Height, const fat4 Factor, xvector (&Result)[N]) noexcept requires(le(3, N) && le(N, 4)) {
    constexpr value f = 1048576.0, n = 0.25;
    Result[0] = xvset<0>(xv_zero, -Factor * Height / Width);
    Result[1] = xvset<1>(xv_zero, Factor);
    Result[2] = xv_constant<0, 0, f / (f - n), -f * n / (f - n)>;
    if constexpr (N == 4) Result[3] = xv_z;
  },
  []<natt N>(const fat4 Width, const fat4 Height, const fat4 Factor, none OrthgraphicMode, xvector (&Result)[N]) noexcept requires(le(3, N) && le(N, 4)) {
    constexpr value f = 1048576.0, n = 0.25;
    Result[0] = xvset<0>(xv_zero, -2.0f * Factor / Width);
    Result[1] = xvset<1>(xv_zero, 2.0f * Factor / Height);
    Result[2] = xv_constant<0, 0, 1 / (f - n), -n / (f - n)>;
    if constexpr (N == 4) Result[3] = xv_w;
  }};

// clang-format on

/// 回転行列をオイラー角に変換する
// struct t_xveuler {
//   xvector operator()(const xvector (&Rotation)[4]) const noexcept {
//     xvector v = xvneg(xvpermute<3, 0, 3, 3>(Rotation[2]));
//     if (xveq(v, xv_y)) return xvasin(xvpermute<5, 1, -1, -1>(v, Rotation[0]));
//     v = xvpermute<0, 1, 6, 3>(xvpermute<4, 1, -1, 3>(v, xvdiv(Rotation[1], Rotation[0])), xvdiv(xvpermute<-1, -1, 1, -1>(Rotation[2]), Rotation[2]));
//     return xvpermute<6, 1, 4, 3>(xvasin(v), xvatan(xvpermute<0, -1, 5, -1>(Rotation[1], Rotation[2]), xvblend<0, 0, 1, 1>(Rotation[0], Rotation[2])));
//   }
// };

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// vector structure with 4 elements; `x`, `y`, `z`, `w`
struct vector {
  fat4 x{}, y{}, z{}, w{};
  constexpr vector() noexcept = default;
  explicit constexpr vector(const fat4 Fill) noexcept : x(Fill), y(Fill), z(Fill), w(Fill) {}
  explicit constexpr vector(const arithmetic auto Fill) noexcept : vector(fat4(Fill)) {}
  template<castable_to<int4> T0, castable_to<int4> T1, castable_to<int4> T2, castable_to<int4> T3>
  constexpr vector(T0&& X, T1&& Y, T2&& Z, T3&& W) noexcept(
    nt_castable_to<T0, int4> && nt_castable_to<T1, int4> && nt_castable_to<T2, int4> && nt_castable_to<T3, int4>)
    : x(static_cast<fat4>(fwd<T0>(X))), y(static_cast<fat4>(fwd<T1>(Y))),
      z(static_cast<fat4>(fwd<T2>(Z))), w(static_cast<fat4>(fwd<T3>(W))) {}
  template<castable_to<int4> T0, castable_to<int4> T1, castable_to<int4> T2>
  constexpr vector(T0&& X, T1&& Y, T2&& Z) noexcept(
    nt_castable_to<T0, int4> && nt_castable_to<T1, int4> && nt_castable_to<T2, int4>)
    : x(static_cast<fat4>(fwd<T0>(X))), y(static_cast<fat4>(fwd<T1>(Y))), z(static_cast<fat4>(fwd<T2>(Z))) {}
  template<castable_to<int4> T0, castable_to<int4> T1> constexpr vector(
    T0&& X, T1&& Y) noexcept(nt_castable_to<T0, int4> && nt_castable_to<T1, int4>)
    : x(static_cast<fat4>(fwd<T0>(X))), y(static_cast<fat4>(fwd<T1>(Y))) {}

  vector(const xvector& A) noexcept { _mm_storeu_ps(&x, A); }
  vector& operator=(const xvector& A) noexcept { return _mm_storeu_ps(&x, A), *this; }
  operator xvector() const noexcept { return xv(&x); }
  explicit operator bool() const noexcept { return x || y || z || w; }
  constexpr natt size() const noexcept { return 4; }
  fat4* data() noexcept { return &x; }
  const fat4* data() const noexcept { return &x; }
  fat4* begin() noexcept { return &x; }
  const fat4* begin() const noexcept { return &x; }
  fat4* end() noexcept { return &x + 4; }
  const fat4* end() const noexcept { return &x + 4; }
  fat4& operator[](natt i) { return *(&x + i); }
  const fat4& operator[](natt i) const { return *(&x + i); }
  template<natt Ix> requires(Ix < 4) constexpr fat4& get() noexcept {
    if constexpr (Ix == 0) return x;
    else if constexpr (Ix == 1) return y;
    else if constexpr (Ix == 2) return z;
    else return w;
  }
  template<natt Ix> requires(Ix < 4) constexpr fat4 get() const noexcept {
    if constexpr (Ix == 0) return x;
    else if constexpr (Ix == 1) return y;
    else if constexpr (Ix == 2) return z;
    else return w;
  }
  friend constexpr bool operator==(const vector& A, const vector& B) noexcept {
    if (!is_cev) return xveq(A, B);
    return A.x == B.x && A.y == B.y && A.z == B.z && A.w == B.w;
  }
  friend constexpr auto operator<=>(const vector& A, const vector& B) noexcept {
    if (!is_cev) return std::partial_ordering(xvtw(A, B));
    if (auto a = A.x <=> B.x; a != 0) return a;
    else if (a = A.y <=> B.y; a != 0) return a;
    else if (a = A.z <=> B.z; a != 0) return a;
    else return A.w <=> B.w;
  }
  friend constexpr vector operator+(const vector& A) noexcept { return A; }
  friend constexpr vector operator-(const vector& A) noexcept {
    if (!is_cev) return xvneg(A);
    else return {-A.x, -A.y, -A.z, -A.w};
  }
  friend constexpr vector operator+(const vector& A, const vector& B) noexcept {
    if (!is_cev) return xvadd(A, B);
    else return {A.x + B.x, A.y + B.y, A.z + B.z, A.w + B.w};
  }
  friend constexpr vector operator-(const vector& A, const vector& B) noexcept {
    if (!is_cev) return xvsub(A, B);
    else return {A.x - B.x, A.y - B.y, A.z - B.z, A.w - B.w};
  }
  friend constexpr vector operator*(const vector& A, const vector& B) noexcept {
    if (!is_cev) return xvmul(A, B);
    else return {A.x * B.x, A.y * B.y, A.z * B.z, A.w * B.w};
  }
  friend constexpr vector operator/(const vector& A, const vector& B) noexcept {
    if (!is_cev) return xvdiv(A, B);
    else return {A.x / B.x, A.y / B.y, A.z / B.z, A.w / B.w};
  }
  friend constexpr vector operator+(const vector& A, const fat4& B) noexcept {
    if (!is_cev) return xvadd(A, xv(B));
    else return {A.x + B, A.y + B, A.z + B, A.w + B};
  }
  friend constexpr vector operator-(const vector& A, const fat4& B) noexcept {
    if (!is_cev) return xvsub(A, xv(B));
    else return {A.x - B, A.y - B, A.z - B, A.w - B};
  }
  friend constexpr vector operator*(const vector& A, const fat4& B) noexcept {
    if (!is_cev) return xvmul(A, xv(B));
    else return {A.x * B, A.y * B, A.z * B, A.w * B};
  }
  friend constexpr vector operator/(const vector& A, const fat4& B) noexcept {
    if (!is_cev) return xvdiv(A, xv(B));
    else return {A.x / B, A.y / B, A.z / B, A.w / B};
  }
  friend constexpr vector operator+(const fat4& A, const vector& B) noexcept {
    if (!is_cev) return xvadd(xv(A), B);
    else return {A + B.x, A + B.y, A + B.z, A + B.w};
  }
  friend constexpr vector operator-(const fat4& A, const vector& B) noexcept {
    if (!is_cev) return xvsub(xv(A), B);
    else return {A - B.x, A - B.y, A - B.z, A - B.w};
  }
  friend constexpr vector operator*(const fat4& A, const vector& B) noexcept {
    if (!is_cev) return xvmul(xv(A), B);
    else return {A * B.x, A * B.y, A * B.z, A * B.w};
  }
  friend constexpr vector operator/(const fat4& A, const vector& B) noexcept {
    if (!is_cev) return xvdiv(xv(A), B);
    else return {A / B.x, A / B.y, A / B.z, A / B.w};
  }
  friend constexpr vector& operator+=(vector& A, const vector& B) noexcept { return A = A + B; }
  friend constexpr vector& operator-=(vector& A, const vector& B) noexcept { return A = A - B; }
  friend constexpr vector& operator*=(vector& A, const vector& B) noexcept { return A = A * B; }
  friend constexpr vector& operator/=(vector& A, const vector& B) noexcept { return A = A / B; }
  friend constexpr vector& operator+=(vector& A, const fat4& B) noexcept { return A = A + B; }
  friend constexpr vector& operator-=(vector& A, const fat4& B) noexcept { return A = A - B; }
  friend constexpr vector& operator*=(vector& A, const fat4& B) noexcept { return A = A * B; }
  friend constexpr vector& operator/=(vector& A, const fat4& B) noexcept { return A = A / B; }
};

/// color structure with 4 elements; `r`, `g`, `b`, `a`
struct color {
  fat4 r{}, g{}, b{}, a = 1.f;
  constexpr color() noexcept = default;
  constexpr color(fat4 R, fat4 G, fat4 B, fat4 A = 1.f) noexcept : r(R), g(G), b(B), a(A) {}
  explicit constexpr color(nat4 xRRGGBB, fat4 A = 1.f) noexcept : color(from_code(xRRGGBB)) { a = A; }
  explicit constexpr operator nat4() const noexcept {
    if (is_cev) return (nat4)(r * 255.f) << 16 | (nat4)(g * 255.f) << 6 | (nat4)(b * 255.f) | (nat4)(a * 255.f) << 24;
    else return bitcast<nat4>(_mm_cvtsi128_si32(_mm_shuffle_epi8(_mm_cvtps_epi32(xvmul(xv(&r), xv_constant<255>)), xv_constant<0, 0, 0, 0x03020100>)));
  }
  explicit color(const xvector& Xv) noexcept { _mm_storeu_ps(&r, Xv); }
  operator xvector() const noexcept { return xv(&r); }
  friend constexpr bool operator==(const color& A, const color& B) noexcept {
    if (!is_cev) return xveq(A, B);
    else return A.r == B.r && A.g == B.g && A.b == B.b && A.a == B.a;
  }
  static const color black, dimgray, gray, darkgray, silver, lightgray, gainsboro, whitesmoke, white, snow, ghostwhite,
    floralwhite, linen, antiquewhite, papayawhip, blanchedalmond, bisque, moccasin, navajowhite, peachpuff, mistyrose,
    lavenderblush, seashell, oldlace, ivory, honeydew, mintcream, azure, aliceblue, lavender, lightsteelblue,
    lightslategray, slategray, steelblue, royalblue, midnightblue, navy, darkblue, mediumblue, blue, dodgerblue,
    cornflowerblue, deepskyblue, lightskyblue, skyblue, lightblue, powderblue, paleturquoise, lightcyan, cyan, aqua,
    turquoise, mediumturquoise, darkturquoise, lightseagreen, cadetblue, darkcyan, teal, darkslategray, darkgreen,
    green, forestgreen, seagreen, mediumseagreen, mediumaquamarine, darkseagreen, aquamarine, palegreen, lightgreen,
    springgreen, mediumspringgreen, lawngreen, chartreuse, greenyellow, lime, limegreen, yellowgreen, darkolivegreen,
    olivedrab, olive, darkkhaki, palegoldenrod, cornsilk, beige, lightyellow, lightgoldenrodyellow, lemonchiffon, wheat,
    burlywood, tan, khaki, yellow, gold, orange, sandybrown, darkorange, goldenrod, peru, darkgoldenrod, chocolate,
    sienna, saddlebrown, maroon, darkred, brown, firebrick, indianred, rosybrown, darksalmon, lightcoral, salmon,
    lightsalmon, coral, tomato, orangered, red, crimson, mediumvioletred, deeppink, hotpink, palevioletred, pink,
    lightpink, thistle, magenta, fuchsia, violet, plum, orchid, mediumorchid, darkorchid, darkviolet, darkmagenta,
    purple, indigo, darkslateblue, blueviolet, mediumpurple, slateblue, mediumslateblue, transparent, undefined;
private:
  static constexpr color from_code(const nat4& a) noexcept {
    if (is_cev) return color(fat4((0xff0000 & a) >> 16) / 255.f, fat4((0xff00 & a) >> 8) / 255.f, fat4(0xff & a) / 255.f, fat4((0xff000000 & a) >> 24) / 255.f);
    else return color(xvmul(xv_constant<1.0 / 255.0>, _mm_cvtepi32_ps(_mm_cvtepu8_epi32(xvbitcast(_mm_broadcast_ss((const fat4*)&a))))));
  }
};

inline constexpr color color::black{0x000000}, color::dimgray{0x696969}, color::gray{0x808080}, color::darkgray{0xa9a9a9}, color::silver{0xc0c0c0},
  color::lightgray{0xd3d3d3}, color::gainsboro{0xdcdcdc}, color::whitesmoke{0xf5f5f5}, color::white{0xffffff}, color::snow{0xfffafa},
  color::ghostwhite{0xf8f8ff}, color::floralwhite{0xfffaf0}, color::linen{0xfaf0e6}, color::antiquewhite{0xfaebd7}, color::papayawhip{0xffefd5},
  color::blanchedalmond{0xffebcd}, color::bisque{0xffe4c4}, color::moccasin{0xffe4b5}, color::navajowhite{0xffdead}, color::peachpuff{0xffdab9},
  color::mistyrose{0xffe4e1}, color::lavenderblush{0xfff0f5}, color::seashell{0xfff5ee}, color::oldlace{0xfdf5e6}, color::ivory{0xfffff0},
  color::honeydew{0xf0fff0}, color::mintcream{0xf5fffa}, color::azure{0xf0ffff}, color::aliceblue{0xf0f8ff}, color::lavender{0xe6e6fa},
  color::lightsteelblue{0xb0c4de}, color::lightslategray{0x778899}, color::slategray{0x708090}, color::steelblue{0x4682b4}, color::royalblue{0x4169e1},
  color::midnightblue{0x191970}, color::navy{0x000080}, color::darkblue{0x00008b}, color::mediumblue{0x0000cd}, color::blue{0x0000ff},
  color::dodgerblue{0x1e90ff}, color::cornflowerblue{0x6495ed}, color::deepskyblue{0x00bfff}, color::lightskyblue{0x87cefa}, color::skyblue{0x87ceeb},
  color::lightblue{0xadd8e6}, color::powderblue{0xb0e0e6}, color::paleturquoise{0xafeeee}, color::lightcyan{0xe0ffff}, color::cyan{0x00ffff},
  color::aqua{0x00ffff}, color::turquoise{0x40e0d0}, color::mediumturquoise{0x48d1cc}, color::darkturquoise{0x00ced1}, color::lightseagreen{0x20b2aa},
  color::cadetblue{0x5f9ea0}, color::darkcyan{0x008b8b}, color::teal{0x008080}, color::darkslategray{0x2f4f4f}, color::darkgreen{0x006400},
  color::green{0x008000}, color::forestgreen{0x228b22}, color::seagreen{0x2e8b57}, color::mediumseagreen{0x3cb371}, color::mediumaquamarine{0x66cdaa},
  color::darkseagreen{0x8fbc8f}, color::aquamarine{0x7fffd4}, color::palegreen{0x98fb98}, color::lightgreen{0x90ee90}, color::springgreen{0x00ff7f},
  color::mediumspringgreen{0x00fa9a}, color::lawngreen{0x7cfc00}, color::chartreuse{0x7fff00}, color::greenyellow{0xadff2f}, color::lime{0x00ff00},
  color::limegreen{0x32cd32}, color::yellowgreen{0x9acd32}, color::darkolivegreen{0x556b2f}, color::olivedrab{0x6b8e23}, color::olive{0x808000},
  color::darkkhaki{0xbdb76b}, color::palegoldenrod{0xeee8aa}, color::cornsilk{0xfff8dc}, color::beige{0xf5f5dc}, color::lightyellow{0xffffe0},
  color::lightgoldenrodyellow{0xfafad2}, color::lemonchiffon{0xfffacd}, color::wheat{0xf5deb3}, color::burlywood{0xdeb887}, color::tan{0xd2b48c},
  color::khaki{0xf0e68c}, color::yellow{0xffff00}, color::gold{0xffd700}, color::orange{0xffa500}, color::sandybrown{0xf4a460}, color::darkorange{0xff8c00},
  color::goldenrod{0xdaa520}, color::peru{0xcd853f}, color::darkgoldenrod{0xb8860b}, color::chocolate{0xd2691e}, color::sienna{0xa0522d},
  color::saddlebrown{0x8b4513}, color::maroon{0x800000}, color::darkred{0x8b0000}, color::brown{0xa52a2a}, color::firebrick{0xb22222},
  color::indianred{0xcd5c5c}, color::rosybrown{0xbc8f8f}, color::darksalmon{0xe9967a}, color::lightcoral{0xf08080}, color::salmon{0xfa8072},
  color::lightsalmon{0xffa07a}, color::coral{0xff7f50}, color::tomato{0xff6347}, color::orangered{0xff4500}, color::red{0xff0000}, color::crimson{0xdc143c},
  color::mediumvioletred{0xc71585}, color::deeppink{0xff1493}, color::hotpink{0xff69b4}, color::palevioletred{0xdb7093}, color::pink{0xffc0cb},
  color::lightpink{0xffb6c1}, color::thistle{0xd8bfd8}, color::magenta{0xff00ff}, color::fuchsia{0xff00ff}, color::violet{0xee82ee}, color::plum{0xdda0dd},
  color::orchid{0xda70d6}, color::mediumorchid{0xba55d3}, color::darkorchid{0x9932cc}, color::darkviolet{0x9400d3}, color::darkmagenta{0x8b008b},
  color::purple{0x800080}, color::indigo{0x4b0082}, color::darkslateblue{0x483d8b}, color::blueviolet{0x8a2be2}, color::mediumpurple{0x9370db},
  color::slateblue{0x6a5acd}, color::mediumslateblue{0x7b68ee}, color::undefined = color(-0.f, -0.f, -0.f, -0.f);

/// rectangle structure with 4 elements; `left`, `top`, `right`, `bottom`
struct rect {
  int4 left{}, top{}, right{}, bottom{};

  /// gets the width of the rectangle
  constexpr int4 width() const noexcept { return right - left; }

  /// gets the height of the rectangle
  constexpr int4 height() const noexcept { return bottom - top; }

  /// constructors
  constexpr rect() noexcept = default;
  explicit constexpr rect(const int4 Fill) noexcept : left(Fill), top(Fill), right(Fill), bottom(Fill) {}
  template<castable_to<int4> T> requires(!same_as<remove_cvref<T>, int4>)
  explicit constexpr rect(T&& Fill) noexcept(nt_castable_to<T, int4>) : rect(static_cast<int4>(fwd<T>(Fill))) {}
  template<castable_to<int4> T0, castable_to<int4> T1, castable_to<int4> T2, castable_to<int4> T3>
  constexpr rect(T0&& L, T1&& T, T2&& R, T3&& B) noexcept(nt_castable_to<T0, int4> && nt_castable_to<T1, int4> && nt_castable_to<T2, int4> && nt_castable_to<T3, int4>)
    : left(static_cast<int4>(fwd<T0>(L))), top(static_cast<int4>(fwd<T1>(T))), right(static_cast<int4>(fwd<T2>(R))), bottom(static_cast<int4>(fwd<T3>(B))) {}
  rect(const xrect& A) noexcept { _mm_storeu_epi32(&left, A); }

  /// assignment
  rect& operator=(const xrect& A) noexcept { return _mm_storeu_epi32(&left, A), *this; }

  /// conversion to `xrect`
  operator xrect() const noexcept { return xv(&left); }

  /// gets the size of the data
  constexpr natt size() const noexcept { return 4; }

  /// gets the pointer to the data
  int4* data() noexcept { return &left; }
  const int4* data() const noexcept { return &left; }

  /// gets the iterator to the begin of the data
  int4* begin() noexcept { return &left; }
  const int4* begin() const noexcept { return &left; }

  /// gets the iterator to the end of the data
  int4* end() noexcept { return &left + 4; }
  const int4* end() const noexcept { return &left + 4; }

  /// gets the element at the index `I`
  template<natt Ix> requires(Ix < 4) constexpr int4& get() noexcept {
    if constexpr (Ix == 0) return left;
    else if constexpr (Ix == 1) return top;
    else if constexpr (Ix == 2) return right;
    else return bottom;
  }
  template<natt Ix> requires(Ix < 4) constexpr int4 get() const noexcept {
    if constexpr (Ix == 0) return left;
    else if constexpr (Ix == 1) return top;
    else if constexpr (Ix == 2) return right;
    else return bottom;
  }

  /// comparisons
  friend constexpr bool operator==(const rect& A, const rect& B) noexcept {
    if (!is_cev) return xveq(A, B);
    return A.left == B.left && A.top == B.top && A.right == B.right && A.bottom == B.bottom;
  }
  friend constexpr auto operator<=>(const rect& A, const rect& B) noexcept {
    if (!is_cev) return xvtw(A, B);
    if (auto a = A.left <=> B.left; a != 0) return a;
    else if (a = A.top <=> B.top; a != 0) return a;
    else if (a = A.right <=> B.right; a != 0) return a;
    else return A.bottom <=> B.bottom;
  }

  /// operations
  friend constexpr rect operator+(const rect& A) noexcept { return A; }
  friend constexpr rect operator-(const rect& A) noexcept {
    if (!is_cev) return xvneg(A);
    else return {-A.left, -A.top, -A.right, -A.bottom};
  }
  friend constexpr rect operator+(const rect& A, const rect& B) noexcept {
    if (!is_cev) return xvadd(A, B);
    else return {A.left + B.left, A.top + B.top, A.right + B.right, A.bottom + B.bottom};
  }
  friend constexpr rect operator-(const rect& A, const rect& B) noexcept {
    if (!is_cev) return xvsub(A, B);
    else return {A.left - B.left, A.top - B.top, A.right - B.right, A.bottom - B.bottom};
  }
  friend constexpr rect operator*(const rect& A, const rect& B) noexcept {
    if (!is_cev) return xvmul(A, B);
    else return {A.left * B.left, A.top * B.top, A.right * B.right, A.bottom * B.bottom};
  }
  friend constexpr rect operator/(const rect& A, const rect& B) noexcept {
    if (!is_cev) return xvdiv(A, B);
    else return {A.left / B.left, A.top / B.top, A.right / B.right, A.bottom / B.bottom};
  }
  friend constexpr rect operator+(const rect& A, const int4& B) noexcept {
    if (!is_cev) return xvadd(A, xv(B));
    else return {A.left + B, A.top + B, A.right + B, A.bottom + B};
  }
  friend constexpr rect operator-(const rect& A, const int4& B) noexcept {
    if (!is_cev) return xvsub(A, xv(B));
    else return {A.left - B, A.top - B, A.right - B, A.bottom - B};
  }
  friend constexpr rect operator*(const rect& A, const int4& B) noexcept {
    if (!is_cev) return xvmul(A, xv(B));
    else return {A.left * B, A.top * B, A.right * B, A.bottom * B};
  }
  friend constexpr rect operator/(const rect& A, const int4& B) noexcept {
    if (!is_cev) return xvdiv(A, xv(B));
    else return {A.left / B, A.top / B, A.right / B, A.bottom / B};
  }
  friend constexpr rect operator+(const int4& A, const rect& B) noexcept {
    if (!is_cev) return xvadd(xv(A), B);
    else return {A + B.left, A + B.top, A + B.right, A + B.bottom};
  }
  friend constexpr rect operator-(const int4& A, const rect& B) noexcept {
    if (!is_cev) return xvsub(xv(A), B);
    else return {A - B.left, A - B.top, A - B.right, A - B.bottom};
  }
  friend constexpr rect operator*(const int4& A, const rect& B) noexcept {
    if (!is_cev) return xvmul(xv(A), B);
    else return {A * B.left, A * B.top, A * B.right, A * B.bottom};
  }
  friend constexpr rect operator/(const int4& A, const rect& B) noexcept {
    if (!is_cev) return xvdiv(xv(A), B);
    else return {A / B.left, A / B.top, A / B.right, A / B.bottom};
  }
  friend constexpr rect& operator+=(rect& A, const rect& B) noexcept { return A = A + B; }
  friend constexpr rect& operator-=(rect& A, const rect& B) noexcept { return A = A - B; }
  friend constexpr rect& operator*=(rect& A, const rect& B) noexcept { return A = A * B; }
  friend constexpr rect& operator/=(rect& A, const rect& B) noexcept { return A = A / B; }
  friend constexpr rect& operator+=(rect& A, const int4& B) noexcept { return A = A + B; }
  friend constexpr rect& operator-=(rect& A, const int4& B) noexcept { return A = A - B; }
  friend constexpr rect& operator*=(rect& A, const int4& B) noexcept { return A = A * B; }
  friend constexpr rect& operator/=(rect& A, const int4& B) noexcept { return A = A / B; }
};
}

namespace std { // clang-format off
template<> struct tuple_size<yw::vector> : public integral_constant<size_t, 4> {};
template<size_t I> requires(I < 4) struct tuple_element<I, yw::vector> : public type_identity<float> {};
template<> struct formatter<yw::vector> : public formatter<string> {
  auto format(const yw::vector& A, format_context& C) const { return formatter<string>::format(std::format("[{},{},{},{}]", A.x, A.y, A.z, A.w), C); } };
template<> struct formatter<yw::color> : formatter<string> {
  auto format(const yw::color& A, format_context& C) const { return formatter<string>::format(std::format("[{},{},{},{}]", A.r, A.g, A.b, A.a), C); } };
template<> struct formatter<yw::xvector> : public formatter<string> {
  auto format(const yw::xvector& A, format_context& C) const { return formatter<string>::format(std::format("{}", yw::vector(A)), C); } };
template<> struct formatter<yw::xmatrix> : public formatter<string> {
  auto format(const yw::xmatrix& A, format_context& C) const { return formatter<string>::format(std::format("[{},{},{},{}]", A[0], A[1], A[2], A[3]), C); } };
} // clang-format on

#ifndef ywlib_disable_windows

#include <map>

#define WIN32_LEAN_AND_MEAN
#include <Shlwapi.h>
#include <Windows.h>
#include <commdlg.h>
#include <d2d1_3.h>
#include <d3d11_3.h>
#include <d3dcompiler.h>
#include <dwrite_3.h>
#include <dxgi1_6.h>
#include <shellapi.h>
#include <wincodec.h>

#pragma comment(lib, "Comdlg32.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "uuid.lib")

#undef max
#undef min

#ifdef ywlib_debug
#define _DEBUG
#define ywlib_enable_console
#define ywlib_assert(Bool, Str) (Bool ? void() : void(std::cerr << Str << std::endl, throw yw::except(Str)))
#else
#define ywlib_assert(Bool, Str) (void())
#endif

namespace yw {
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// comptr
template<typename Com> class comptr {
  static_assert(requires(Com& a, IUnknown* b) {
    { a.QueryInterface(__uuidof(IUnknown), (void**)&b) } -> std::same_as<HRESULT>;
    { a.QueryInterface(&b) } -> same_as<HRESULT>;
    { a.Release() } -> same_as<ULONG>;
    { a.AddRef() } -> same_as<ULONG>;
  });
  template<typename U> friend class comptr;
  Com* _ptr = nullptr;
  void _addref() const noexcept { _ptr ? void(_ptr->AddRef()) : void(); }
  void _release() noexcept { _ptr ? void(exchange(_ptr, nullptr)->Release()) : void(); }
  void _copy(Com* a) noexcept { _ptr == a ? void() : (_release(), _ptr = a, _addref()); }
  void _move(auto& a) noexcept { _ptr == a._ptr ? void() : void((_release(), _ptr = a._ptr, a._ptr = nullptr)); }
public:
  /// pointer type
  using pointer = Com*;

  /// class for restricting Com methods
  class reference : public Com {
    void operator&() const = delete;
    ULONG __stdcall AddRef() { return Com::AddRef(); }
    ULONG __stdcall Release() { return Com::Release(); }
  };

  /// destructor
  ~comptr() noexcept { _release(); }

  /// constructors
  comptr() noexcept = default;
  comptr(comptr&& a) noexcept : _ptr{a._ptr} { a._ptr = nullptr; }
  comptr(const comptr& a) noexcept : _ptr{a._ptr} { _addref(); }
  comptr(Com*&& a) noexcept : _ptr(a) {}
  template<typename U> comptr(comptr<U>&& a) noexcept : _ptr{a._ptr} { a._ptr = nullptr; }
  template<typename U> comptr(const comptr<U>& a) noexcept : _ptr{a._ptr} { _addref(); }

  /// assignment
  comptr& operator=(const comptr& a) noexcept { return _copy(a._ptr), *this; }
  template<typename U> comptr& operator=(const comptr<U>& a) noexcept { return _copy(a._ptr), *this; }
  template<typename U> comptr& operator=(comptr<U>&& a) noexcept { return _move(a), *this; }
  template<typename U> bool operator==(const comptr<U>& a) noexcept { return _ptr == a._ptr; }

  /// conversion to `bool`
  explicit operator bool() const noexcept { return _ptr; }

  /// conversion to a restricted pointer to Com
  operator reference*() const noexcept { return static_cast<reference*>(_ptr); }

  /// restricted access to Com methods
  reference* operator->() const noexcept { return static_cast<reference*>(_ptr); }

  /// resets the pointer
  void reset() noexcept { _release(); }

  /// gets the pointer this holds
  Com* get() const noexcept { return _ptr; }

  /// gets the address of the pointer
  Com** addressof(source S = {}) & {
    if (_ptr != nullptr) throw except("You must reset comptr", mv(S));
    return &_ptr;
  }
  Com* const* operator&() const noexcept { return &_ptr; }

  /// swaps the pointers
  void swap(comptr& a) noexcept { std::ranges::swap(_ptr, a._ptr); }

  /// casts to `U`
  template<typename U> HRESULT as(comptr<U>& a) const noexcept { return _ptr->QueryInterface(IID_PPV_ARGS(a.addressof())); }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// throws an exception if `B` is false
inline constexpr auto tiff = overload{
  [](HRESULT B, source S = {}) {
    if (B < 0) {
      cat1 buf[128];
      FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, B,
                     MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buf, 128, nullptr);
      throw except(buf, mv(S)); } },
  [](const auto& B, source S = {}) requires requires { { !B } -> convertible_to<bool>; } {
    if (!B) throw except("FAILED", mv(S)); }};

/// checks if `T` is a rect-like type
template<typename T> concept rect_like = requires {
  requires extent<T> == 4;
  requires same_as<element_t<T, 0>, element_t<T, 1>, element_t<T, 2>, element_t<T, 3>>;
  requires arithmetic<remove_ref<element_t<T, 0>>>;
};

/// stopwatch class
class stopwatch {
  mutable LARGE_INTEGER _li;
  fat8 _freq;
  int8 _last;
public:
  /// constructor
  stopwatch() noexcept : _li{}, _freq{}, _last{} {
    QueryPerformanceFrequency(&_li), _freq = static_cast<fat8>(_li.QuadPart);
    QueryPerformanceCounter(&_li), _last = _li.QuadPart;
  }

  /// convertion to `fat8`; reads the time
  fat8 operator()() const noexcept { return read(); }

  /// reads the time
  fat8 read() const noexcept { return QueryPerformanceCounter(&_li), (_li.QuadPart - _last) / _freq; }

  /// reads the time and restarts
  fat8 push() noexcept { return QueryPerformanceCounter(&_li), (_li.QuadPart - exchange(_last, _li.QuadPart)) / _freq; }

  /// restarts
  void start() noexcept { QueryPerformanceCounter(&_li), _last = _li.QuadPart; }
};

namespace main {

/// username
inline const str2 username{};

/// command line arguments
inline const array<str2> args{};

/// application timer
inline const stopwatch timer{};

/// seconds per frame
inline const fat8 spf{1};

/// frames per second
inline const fat8 fps{1};

/// mouse position; `{x, y, na, na}`
inline const rect mouse{};

/// if the cursor is on the window
inline const bool hover{true};

/// window handle
inline const HWND hwnd{};

/// instance handle
inline const HINSTANCE hinstance{};

/// Direct3D device
inline const comptr<ID3D11Device1> d3d_device{};

/// Direct3D device context
inline const comptr<ID3D11DeviceContext1> d3d_context{};

/// DirectX swap chain
inline const comptr<IDXGISwapChain1> swap_chain{};

/// render target
// inline const comptr<ID2D1Bitmap1> render_target{};

/// Direct2D device
inline const comptr<ID2D1Device5> d2d_device{};

/// Direct2D device context
inline const comptr<ID2D1DeviceContext5> d2d_context{};

/// Direct2D factory
inline const comptr<ID2D1Factory6> d2d_factory{};

/// DirectWrite factory
inline const comptr<IDWriteFactory> dw_factory{};

/// WIC factory
inline const comptr<IWICImagingFactory2> wic_factory{};

/// procedure of main window
inline LRESULT WINAPI wndproc(HWND, UINT, WPARAM, LPARAM);

/// resizes the main window
void resize(natt Width, natt Height);

/// callback function for file drag and drop
inline void (*dropped)(array<path> Files) = nullptr;

/// callback function for hacking procedure
/// @note If this returns `true`, the default procedure will be ignored.
inline bool (*userproc)(HWND, nat4, natt, intt) = nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// brush class
/// @note `brush(color)` -> dynamic brush
/// @note `brush<color>()` -> static brush
template<color Color = color::undefined> class brush;

template<> class brush<color::undefined> {
  brush(const brush&) = delete;
  brush& operator=(const brush&) = delete;
protected:
  comptr<ID2D1SolidColorBrush> d2d_brush{};
public:
  /// checks if this is valid
  explicit operator bool() const noexcept { return bool(d2d_brush); }

  /// conversion to a restricted pointer to `ID2D1SolidColorBrush`
  operator comptr<ID2D1SolidColorBrush>::reference*() const noexcept {
    ywlib_assert(d2d_brush, "this brush is not valid");
    return d2d_brush;
  }

  /// constructor
  brush() noexcept = default;
  brush(brush&& Brush) noexcept { d2d_brush.swap(Brush.d2d_brush); }
  brush(color Color) { tiff(main::d2d_context->CreateSolidColorBrush(bitcast<D2D1_COLOR_F>(Color), d2d_brush.addressof())); }

  /// move assignment
  brush& operator=(brush&& Brush) noexcept { return d2d_brush.reset(), d2d_brush.swap(Brush.d2d_brush), *this; }

  /// gets the color of this brush
  yw::color color() const {
    ywlib_assert(d2d_brush, "this brush is not valid");
    return bitcast<yw::color>(d2d_brush->GetColor());
  }
};

template<color Color> requires(Color != color::undefined) class brush<Color> {
  inline static yw::brush<color::undefined> yw_brush{};
public:
  /// checks if this is valid
  explicit operator bool() const noexcept { return (yw_brush ? none{} : yw_brush = yw::brush<>(Color)), true; }

  /// conversion to a restricted pointer to `ID2D1SolidColorBrush`
  operator comptr<ID2D1SolidColorBrush>::reference*() const noexcept { return yw_brush; }

  /// gets the color of this brush
  yw::color color() const { return Color; }
};

/// deduction guide for `brush`
template<typename... Ts> brush(Ts&&...) -> brush<color::undefined>;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// font class
/// @note `font(Size, (Name, Alignment, Bold, Italic))` -> dynamic font
/// @note `font<Size, (Name, Alignment, Bold, Italic)>()` -> static font
template<value Size = value{}, stv2 Name = L"Yu Gothic UI"_sv, intt Alignment = 0, bool Bold = false, bool Italic = false> class font;

template<stv2 Name, intt Alignment, bool Bold, bool Italic> class font<value{}, Name, Alignment, Bold, Italic> {
  font(const font&) = delete;
  font& operator=(const font&) = delete;
protected:
  comptr<IDWriteTextFormat> dw_format{};
public:
  /// check if this is valid
  explicit operator bool() const noexcept { return bool(dw_format); }

  /// conversion to a restricted pointer to `IDWriteTextFormat`
  operator comptr<IDWriteTextFormat>::reference*() const noexcept {
    ywlib_assert(dw_format, "this font is not valid");
    return dw_format;
  }

  /// default constructor
  font() noexcept = default;

  /// move constructor
  font(font&& Font) noexcept { dw_format.swap(Font.dw_format); }

  /// constructor
  font(fat4 Size, const stv2& Name = L"Yu Gothic UI",
       intt Alignment = 0, bool Bold = false, bool Italic = false) {
    if (Name.null_terminated()) 1;
    tiff(main::dw_factory->CreateTextFormat(
      Name.data(), nullptr, Bold ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_NORMAL,
      Italic ? DWRITE_FONT_STYLE_OBLIQUE : DWRITE_FONT_STYLE_NORMAL,
      DWRITE_FONT_STRETCH_NORMAL, Size, L"", &dw_format));
    if (Alignment < 0) dw_format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
    else if (Alignment > 0) dw_format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
    else dw_format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
  }

  /// move assignment
  font& operator=(font&& Font) noexcept { return dw_format.reset(), dw_format.swap(Font.dw_format), *this; }

  /// gets the size of this font
  fat4 size() const {
    ywlib_assert(dw_format, "this font is not valid");
    return dw_format->GetFontSize();
  }

  /// gets the name of this font
  str2 name() const {
    ywlib_assert(dw_format, "this font is not valid");
    str2 out(dw_format->GetFontFamilyNameLength(), 0);
    tiff(dw_format->GetFontFamilyName(out.data(), out.size() + 1));
    return out;
  }

  /// gets the alignment of this font
  /// @note - : leading, 0 : center, + : trailing
  intt alignment() const {
    ywlib_assert(dw_format, "this font is not valid");
    if (auto a = dw_format->GetTextAlignment(); a == DWRITE_TEXT_ALIGNMENT_LEADING) return -1;
    else if (a == DWRITE_TEXT_ALIGNMENT_TRAILING) return 1;
    else return 0;
  }

  /// sets the alignment of this font
  /// @note - : leading, 0 : center, + : trailing
  void alignment(intt Alignment) {
    if (Alignment < 0) dw_format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
    else if (Alignment > 0) dw_format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
    else dw_format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
  }
};

template<value Size, stv2 Name, intt Alignment, bool Bold, bool Italic> requires(Size != 0)
class font<Size, Name, Alignment, Bold, Italic> {
  inline static yw::font<value{}> yw_font{};
  static void init() { yw_font ? none{} : yw_font = yw::font<value{}>(Size, Name, Alignment, Bold, Italic); }
public:
  /// check if this is valid
  explicit operator bool() const noexcept { return init(), true; }

  /// conversion to a restricted pointer to `IDWriteTextFormat`
  operator comptr<IDWriteTextFormat>::reference*() const noexcept { return init(), yw_font; }

  /// gets the size of this font
  constexpr fat4 size() const { return Size; }

  /// gets the name of this font
  constexpr stv2 name() const { return Name; }

  /// gets the alignment of this font
  /// @note - : leading, 0 : center, + : trailing
  constexpr intt alignment() const { return Alignment; }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T> class staging_buffer;

/// base class for creating gpu buffers
template<typename T> class buffer {
protected:
  comptr<ID3D11Buffer> d3d_buffer;
  buffer(const buffer&) = delete;
  buffer& operator=(const buffer&) = delete;
  buffer(natt Count) noexcept : count(static_cast<nat4>(Count)) {}
public:
  using value_type = T;
  const nat4 count{};

  /// constructors
  buffer() noexcept = default;
  buffer(buffer&& B) noexcept : d3d_buffer{mv(B.d3d_buffer)}, count{B.count} {}

  /// move assignment
  buffer& operator=(buffer&& B) noexcept {
    d3d_buffer = mv(B.d3d_buffer);
    const_cast<nat4&>(count) = B.count;
    return *this;
  }

  /// checks if this is valid
  explicit operator bool() const noexcept { return bool(d3d_buffer); }

  /// conversion to a restricted pointer to `ID3D11Buffer`
  operator comptr<ID3D11Buffer>::reference*() const noexcept {
    ywlib_assert(d3d_buffer, "this buffer is not valid");
    return d3d_buffer;
  }

  /// copies the contents of `Src` to this buffer.
  /// @note `Src` must have the same size as this buffer.
  void from(const buffer& Src) {
    ywlib_assert(d3d_buffer, "this buffer is not valid");
    ywlib_assert(count == Src.count, "the size of the source buffer must be the same as this buffer");
    main::d3d_context->CopyResource(*this, Src);
  }

  /// copies the contents to CPU.
  array<T> to_cpu() const;

  /// copies the contents to CPU by pre-allocated `staging_buffer`.
  array<T> to_cpu(staging_buffer<T>& Staging) const;
};

/// class for creating staging buffers.
template<typename T> class staging_buffer : public buffer<T> {
public:
  /// default constructor
  staging_buffer() noexcept = default;

  /// copies the contents of `Src`
  explicit staging_buffer(const buffer<T>& Src) : staging_buffer(Src.count) { buffer<T>::from(Src); }

  /// creates a staging buffer with `Count` elements
  explicit staging_buffer(natt Count) : buffer<T>(Count) {
    D3D11_BUFFER_DESC desc{nat4(sizeof(T) * Count), D3D11_USAGE_STAGING, 0, D3D11_CPU_ACCESS_READ};
    tiff(main::d3d_device->CreateBuffer(&desc, nullptr, buffer<T>::d3d_buffer.addressof()));
  }

  /// Copies this contents to CPU.
  array<T> to_cpu() const {
    ywlib_assert(*this, "this buffer is not valid");
    array<T> a(buffer<T>::count);
    D3D11_MAPPED_SUBRESOURCE mapped;
    tiff(main::d3d_context->Map(*this, 0, D3D11_MAP_READ, 0, &mapped));
    memcpy(a.data(), mapped.pData, nat4(sizeof(T)) * buffer<T>::count);
    main::d3d_context->Unmap(*this, 0);
    return a;
  }
};

template<typename T> array<T> buffer<T>::to_cpu() const {
  ywlib_assert(*this, "this buffer is not valid");
  return staging_buffer<T>(*this).to_cpu();
}
template<typename T> array<T> buffer<T>::to_cpu(staging_buffer<T>& S) const {
  ywlib_assert(*this, "this buffer is not valid");
  return S.from(*this), S.to_cpu();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// class for creating constant buffers
template<typename T> requires((sizeof(T) & 0xf) == 0) class constant_buffer : public buffer<T> {
public:
  /// default constructor
  constant_buffer() noexcept = default;

  /// initializes by `Value`
  constant_buffer(const T& Value) : buffer<T>(1) {
    static constexpr D3D11_BUFFER_DESC desc{nat4(sizeof(T)), D3D11_USAGE_DYNAMIC, D3D11_BIND_CONSTANT_BUFFER, D3D11_CPU_ACCESS_WRITE};
    [&](D3D11_SUBRESOURCE_DATA d) { tiff(main::d3d_device->CreateBuffer(&desc, &d, buffer<T>::d3d_buffer.addressof())); }({&Value});
  }

  /// copies `Value` to this buffer
  template<typename U> requires assignable<T&, U> || vassignable<T&, U> void from(U&& Value) {
    ywlib_assert(*this, "this buffer is not valid");
    if (*this) {
      D3D11_MAPPED_SUBRESOURCE mapped;
      tiff(main::d3d_context->Map(*this, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped));
      if constexpr (assignable<T&, U>) *reinterpret_cast<T*>(mapped.pData) = fwd<U>(Value);
      else vassign(*reinterpret_cast<T*>(mapped.pData), fwd<U>(Value));
      main::d3d_context->Unmap(*this, 0);
    } else {
      T temp;
      if constexpr (assignable<T&, U>) *this = constant_buffer(temp = fwd<U>(Value));
      else vassign(temp, fwd<U>(Value)), *this = constant_buffer(temp);
    }
  }

  /// sets the value through `Func`
  void from(invocable<T&> auto&& Func) {
    ywlib_assert(*this, "this buffer is not valid");
    if (*this) {
      D3D11_MAPPED_SUBRESOURCE mapped;
      tiff(main::d3d_context->Map(*this, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped));
      Func(*reinterpret_cast<T*>(mapped.pData)), main::d3d_context->Unmap(*this, 0);
    } else [&](T temp) { Func(temp), *this = constant_buffer(temp); }({});
  }

  /// sets this buffer to the vertex shader
  void to_vs(natt Slot = 0) const {
    ywlib_assert(*this, "this buffer is not valid");
    main::d3d_context->VSSetConstantBuffers(nat4(Slot), 1, &(buffer<T>::d3d_buffer));
  }

  /// sets this buffer to the geometry shader
  void to_gs(natt Slot = 0) const {
    ywlib_assert(*this, "this buffer is not valid");
    main::d3d_context->GSSetConstantBuffers(nat4(Slot), 1, &(buffer<T>::d3d_buffer));
  }

  /// sets this buffer to the pixel shader
  void to_ps(natt Slot = 0) const {
    ywlib_assert(*this, "this buffer is not valid");
    main::d3d_context->PSSetConstantBuffers(nat4(Slot), 1, &(buffer<T>::d3d_buffer));
  }

  /// sets this buffer to the compute shader
  void to_cs(natt Slot = 0) const {
    ywlib_assert(*this, "this buffer is not valid");
    main::d3d_context->CSSetConstantBuffers(nat4(Slot), 1, &(buffer<T>::d3d_buffer));
  }
};

/// deduction guide for `constant_buffer`
template<typename T> constant_buffer(const T&) -> constant_buffer<T>;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// class for creating structured buffers
template<typename T> class structured_buffer : public buffer<T> {
protected:
  comptr<ID3D11ShaderResourceView> d3d_srv;
public:
  /// default constructor
  structured_buffer() noexcept = default;

  /// copies from another buffer
  explicit structured_buffer(const buffer<T>& Src) : structured_buffer(Src.count) { buffer<T>::from(Src); }

  /// creates a structured buffer with `Count` elements
  explicit structured_buffer(natt Count) : buffer<T>(Count) {
    D3D11_BUFFER_DESC desc{nat4(sizeof(T) * Count), D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE,
                           0, D3D11_RESOURCE_MISC_BUFFER_STRUCTURED, sizeof(T)};
    tiff(main::d3d_device->CreateBuffer(&desc, nullptr, buffer<T>::d3d_buffer.addressof()));
    D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc{DXGI_FORMAT_UNKNOWN, D3D11_SRV_DIMENSION_BUFFER, {}};
    srv_desc.Buffer.NumElements = Count;
    tiff(main::d3d_device->CreateShaderResourceView(*this, &srv_desc, d3d_srv.addressof()));
  }

  /// creates a structured buffer with `Count` elements and initializes by `Data`
  structured_buffer(const T* Data, natt Count) : buffer<T>(Count) {
    D3D11_BUFFER_DESC desc{nat4(sizeof(T) * Count), D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE,
                           0, D3D11_RESOURCE_MISC_BUFFER_STRUCTURED, nat4(sizeof(T))};
    if (Data) {
      D3D11_SUBRESOURCE_DATA data{.pSysMem = Data, .SysMemPitch = nat4(sizeof(T))};
      tiff(main::d3d_device->CreateBuffer(&desc, &data, buffer<T>::d3d_buffer.addressof()));
    } else tiff(main::d3d_device->CreateBuffer(&desc, nullptr, buffer<T>::d3d_buffer.addressof()));
    D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc{
      DXGI_FORMAT_UNKNOWN, D3D11_SRV_DIMENSION_BUFFER, D3D11_BUFFER_SRV{0, nat4(Count)}};
    tiff(main::d3d_device->CreateShaderResourceView(*this, &srv_desc, d3d_srv.addressof()));
  }

  /// copies the contents of `Data` to this buffer
  /// @note `Data` must have the same or larger size as this buffer.
  void from(const T* Data) {
    ywlib_assert(*this, "this buffer is not valid");
    main::d3d_context->UpdateSubresource(*this, 0, 0, Data, nat4(sizeof(T)), nat4(sizeof(T)) * buffer<T>::count);
  }

  /// sets this buffer to the vertex shader
  void to_vs(nat4 Slot = 0) const {
    ywlib_assert(*this, "this buffer is not valid");
    main::d3d_context->VSSetShaderResources(Slot, 1, &d3d_srv);
  }

  /// sets this buffer to the geometry shader
  void to_gs(nat4 Slot = 0) const {
    ywlib_assert(*this, "this buffer is not valid");
    main::d3d_context->GSSetShaderResources(Slot, 1, &d3d_srv);
  }

  /// sets this buffer to the pixel shader
  void to_ps(nat4 Slot = 0) const {
    ywlib_assert(*this, "this buffer is not valid");
    main::d3d_context->PSSetShaderResources(Slot, 1, &d3d_srv);
  }

  /// sets this buffer to the compute shader
  void to_cs(nat4 Slot = 0) const {
    ywlib_assert(*this, "this buffer is not valid");
    main::d3d_context->CSSetShaderResources(Slot, 1, &d3d_srv);
  }
};

/// deduction guide for `structured_buffer`
template<typename T> structured_buffer(const buffer<T>&) -> structured_buffer<T>;
template<typename T> structured_buffer(const T*, natt) -> structured_buffer<T>;
template<cnt_range R> structured_buffer(R&&) -> structured_buffer<iter_value_t<R>>;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// class for creating unordered access buffers
template<typename T> class unordered_buffer : public buffer<T> {
protected:
  comptr<ID3D11UnorderedAccessView> d3d_uav;
public:
  /// default constructor
  unordered_buffer() noexcept = default;

  /// copies from another buffer
  explicit unordered_buffer(const buffer<T>& Src) : unordered_buffer(Src.count) { buffer<T>::from(Src); }

  /// creates an unordered access buffer with `Count` elements
  explicit unordered_buffer(natt Count) : buffer<T>(Count) {
    D3D11_BUFFER_DESC desc{nat4(sizeof(T) * Count), D3D11_USAGE_DEFAULT, D3D11_BIND_UNORDERED_ACCESS,
                           0, D3D11_RESOURCE_MISC_BUFFER_STRUCTURED, nat4(sizeof(T))};
    tiff(main::d3d_device->CreateBuffer(&desc, nullptr, buffer<T>::d3d_buffer.addressof()));
    tiff(main::d3d_device->CreateUnorderedAccessView(*this, nullptr, d3d_uav.addressof()));
  }

  /// creates an unordered access buffer with `Count` elements and initializes by `Data`
  unordered_buffer(const T* Data, natt Count) : buffer<T>(Count) {
    D3D11_BUFFER_DESC desc{nat4(sizeof(T) * Count), D3D11_USAGE_DEFAULT, D3D11_BIND_UNORDERED_ACCESS,
                           0, D3D11_RESOURCE_MISC_BUFFER_STRUCTURED, nat4(sizeof(T))};
    if (Data) {
      D3D11_SUBRESOURCE_DATA data{.pSysMem = Data, .SysMemPitch = nat4(sizeof(T))};
      tiff(main::d3d_device->CreateBuffer(&desc, &data, buffer<T>::d3d_buffer.addressof()));
    } else tiff(main::d3d_device->CreateBuffer(&desc, nullptr, buffer<T>::d3d_buffer.addressof()));
    tiff(main::d3d_device->CreateUnorderedAccessView(*this, nullptr, d3d_uav.addressof()));
  }

  /// copies the contents of `Data` to this buffer
  /// @note `Data` must have the same or larger size as this buffer.
  void from(const T* Data) {
    ywlib_assert(*this, "this buffer is not valid");
    main::d3d_context->UpdateSubresource(*this, 0, 0, Data, nat4(sizeof(T)), nat4(sizeof(T)) * buffer<T>::count);
  }

  /// sets this buffer to the compute shader
  void to_cs(nat4 Slot = 0) const {
    ywlib_assert(*this, "this buffer is not valid");
    main::d3d_context->CSSetUnorderedAccessViews(Slot, 1, &d3d_uav, nullptr);
  }
};

/// deduction guide for `unordered_buffer`
template<typename T> unordered_buffer(const buffer<T>&) -> unordered_buffer<T>;
template<typename T> unordered_buffer(const T*, natt) -> unordered_buffer<T>;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// clang-format off

/// class for rendering 3D objects
template<specialization_of<typepack> VSResources, specialization_of<typepack> VSCBuffers,
         specialization_of<typepack> GSResources = typepack<>, specialization_of<typepack> GSCBuffers = typepack<>,
         specialization_of<typepack> PSResources = typepack<>, specialization_of<typepack> PSCBuffers = typepack<>>
requires requires {
  requires []<natt... Is>(sequence<Is...>) { return (convertible_to<typename VSResources::template at<Is>, ID3D11ShaderResourceView*> && ...); }(make_indices_for<VSResources>{});
  requires []<natt... Is>(sequence<Is...>) { return (convertible_to<typename GSResources::template at<Is>, ID3D11ShaderResourceView*> && ...); }(make_indices_for<GSResources>{});
  requires []<natt... Is>(sequence<Is...>) { return (convertible_to<typename PSResources::template at<Is>, ID3D11ShaderResourceView*> && ...); }(make_indices_for<PSResources>{});
  requires []<natt... Is>(sequence<Is...>) { return (convertible_to<typename VSCBuffers::template at<Is>, ID3D11Buffer*> && ...); }(make_indices_for<VSCBuffers>{});
  requires []<natt... Is>(sequence<Is...>) { return (convertible_to<typename GSCBuffers::template at<Is>, ID3D11Buffer*> && ...); }(make_indices_for<GSCBuffers>{});
  requires []<natt... Is>(sequence<Is...>) { return (convertible_to<typename PSCBuffers::template at<Is>, ID3D11Buffer*> && ...); }(make_indices_for<PSCBuffers>{});
} class renderer {
protected:
  comptr<ID3D11VertexShader> d3d_vs{};
  comptr<ID3D11GeometryShader> d3d_gs{};
  comptr<ID3D11PixelShader> d3d_ps{};
public:
  /// list of shader resources and textures for the vertex shader
  using vs_resource_list = type_switch<VSResources::size == 0, none, array<ID3D11ShaderResourceView*, VSResources::size>>;

  /// list of shader resources and textures for the geometry shader
  using gs_resource_list = type_switch<GSResources::size == 0, none, array<ID3D11ShaderResourceView*, GSResources::size>>;

  /// list of shader resources and textures for the pixel shader
  using ps_resource_list = type_switch<PSResources::size == 0, none, array<ID3D11ShaderResourceView*, PSResources::size>>;

  /// list of constant buffers for the vertex shader
  using vs_cbuffer_list = type_switch<VSCBuffers::size == 0, none, array<ID3D11Buffer*, VSCBuffers::size>>;

  /// list of constant buffers for the geometry shader
  using gs_cbuffer_list = type_switch<GSCBuffers::size == 0, none, array<ID3D11Buffer*, GSCBuffers::size>>;

  /// list of constant buffers for the pixel shader
  using ps_cbuffer_list = type_switch<PSCBuffers::size == 0, none, array<ID3D11Buffer*, PSCBuffers::size>>;

  /// default constructor
  renderer() noexcept = default;

  /// constructor
  /// @note `HLSL` is a `vs_5_0` shader code which includes `vsmain` and `psmain` (optional `gsmain`) as the entry points.
  renderer(stv1 HLSL) {
    comptr<ID3DBlob> b, e;
    if (0 > D3DCompile(HLSL.data(), HLSL.size(), 0, 0, 0, "vsmain", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS,
                       0, b.addressof(), e.addressof())) throw except((cat1*)e->GetBufferPointer());
    tiff(main::d3d_device->CreateVertexShader(b->GetBufferPointer(), b->GetBufferSize(), 0, d3d_vs.addressof()));
    b.reset(), e.reset();
    if (0 > D3DCompile(HLSL.data(), HLSL.size(), 0, 0, 0, "psmain", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS,
                       0, b.addressof(), e.addressof())) throw except((cat1*)e->GetBufferPointer());
    tiff(main::d3d_device->CreatePixelShader(b->GetBufferPointer(), b->GetBufferSize(), 0, d3d_ps.addressof()));
    b.reset(), e.reset();
    if (HLSL.find("gsmain") == npos) return;
    if (0 > D3DCompile(HLSL.data(), HLSL.size(), 0, 0, 0, "gsmain", "gs_5_0", D3D10_SHADER_ENABLE_STRICTNESS,
                       0, b.addressof(), e.addressof())) throw except((cat1*)e->GetBufferPointer());
    tiff(main::d3d_device->CreateGeometryShader(b->GetBufferPointer(), b->GetBufferSize(), 0, d3d_gs.addressof()));
  }

  /// checks if this is valid
  explicit operator bool() const noexcept { return bool(d3d_vs) && bool(d3d_ps); }

  /// performs rendering
  void operator()(natt VertexCounts,
                  vs_resource_list VSResources, vs_cbuffer_list VSCBuffers,
                  gs_resource_list GSResources = {}, gs_cbuffer_list GSCBuffers = {},
                  ps_resource_list PSResources = {}, ps_cbuffer_list PSCbuffers = {}) const {
    if constexpr (vs_resource_list.size > 0) main::d3d_context->VSSetShaderResources(0, nat4(VSResources.count), &VSResources.first);
    if constexpr (gs_resource_list.size > 0) main::d3d_context->GSSetShaderResources(0, nat4(VSResources.count), &VSResources.first);
    if constexpr (ps_resource_list.size > 0) main::d3d_context->PSSetShaderResources(0, nat4(VSResources.count), &VSResources.first);
    if constexpr (vs_cbuffer_list != none) main::d3d_context->VSSetConstantBuffers(0, VSCBuffers.size, nat4(VSCBuffers.count));
    if constexpr (gs_cbuffer_list != none) main::d3d_context->GSSetConstantBuffers(0, GSCBuffers.size, nat4(GSCBuffers.count));
    if constexpr (ps_cbuffer_list != none) main::d3d_context->PSSetConstantBuffers(0, PSCbuffers.size, nat4(PSCbuffers.count));
    main::d3d_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    main::d3d_context->IASetInputLayout(nullptr);
    main::d3d_context->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
    main::d3d_context->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
    main::d3d_context->VSSetShader(d3d_vs, 0, 0);
    main::d3d_context->GSSetShader(d3d_gs, 0, 0);
    main::d3d_context->PSSetShader(d3d_ps, 0, 0);
    main::d3d_context->Draw(nat4(VertexCounts), 0);
  }
};

// clang-format on

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// class for creating 2D bitmaps
class bitmap {
  bitmap(bitmap&) = delete;
  bitmap& operator=(bitmap&) = delete;
  friend void main::resize(natt, natt);
protected:
  comptr<ID2D1Bitmap1> d2d_bitmap;
public:
  /// width
  const nat4 width{};

  /// height
  const nat4 height{};

  /// default constructor
  bitmap() noexcept = default;

  /// move constructor
  bitmap(bitmap&& Bitmap) noexcept : d2d_bitmap{mv(Bitmap.d2d_bitmap)}, width{Bitmap.width}, height{Bitmap.height} {}

  /// creates a bitmap with `Width` and `Height`
  bitmap(natt Width, natt Height) : width(nat4(Width)), height(nat4(Height)) {
    auto p = D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET, D2D1::PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));
    tiff(main::d2d_context->CreateBitmap(D2D1::SizeU(width, height), nullptr, 0, p, d2d_bitmap.addressof()));
  }

  /// creates a bitmap from an image file
  explicit bitmap(const path& Image) {
    comptr<IWICBitmapDecoder> decoder{};
    tiff(main::wic_factory->CreateDecoderFromFilename(
      Image.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, decoder.addressof()));
    comptr<IWICBitmapFrameDecode> frame;
    tiff(decoder->GetFrame(0, frame.addressof()));
    tiff(frame->GetSize(const_cast<nat4*>(&width), const_cast<nat4*>(&height)));
    WICPixelFormatGUID pixel_format, guid = GUID_WICPixelFormat32bppPRGBA;
    comptr<IWICFormatConverter> fc;
    tiff(frame->GetPixelFormat(&pixel_format));
    tiff(main::wic_factory->CreateFormatConverter(fc.addressof()));
    [&](BOOL b) { tiff(fc->CanConvert(pixel_format, guid, &b)), tiff(b); }({});
    tiff(fc->Initialize(frame, guid, WICBitmapDitherTypeErrorDiffusion, nullptr, 0, WICBitmapPaletteTypeMedianCut));
    tiff(main::d2d_context->CreateBitmapFromWicBitmap(fc, d2d_bitmap.addressof()));
  }

  /// move assignment
  bitmap& operator=(bitmap&& Bitmap) noexcept {
    d2d_bitmap = mv(Bitmap.d2d_bitmap);
    const_cast<nat4&>(width) = Bitmap.width;
    const_cast<nat4&>(height) = Bitmap.height;
    return *this;
  }

  /// checks if this is valid
  explicit operator bool() const noexcept { return bool(d2d_bitmap); }

  /// conversion to a restricted pointer to `ID2D1Bitmap1`
  operator comptr<ID2D1Bitmap1>::reference*() const noexcept {
    ywlib_assert(d2d_bitmap, "this bitmap is not valid");
    return d2d_bitmap;
  }

  /// saves this bitmap as a PNG file
  void to_png(const path& Path) const {
    if (Path.exists()) Path.remove();
    comptr<IWICStream> stream{};
    tiff(main::wic_factory->CreateStream(stream.addressof()));
    tiff(stream->InitializeFromFilename(Path.c_str(), GENERIC_WRITE));
    comptr<IWICBitmapEncoder> encoder{};
    tiff(main::wic_factory->CreateEncoder(GUID_ContainerFormatPng, nullptr, encoder.addressof()));
    tiff(encoder->Initialize(stream, WICBitmapEncoderNoCache));
    comptr<IWICBitmapFrameEncode> frame{};
    tiff(encoder->CreateNewFrame(frame.addressof(), nullptr));
    tiff(frame->Initialize(nullptr));
    comptr<IWICImageEncoder> image_encoder{};
    tiff(main::wic_factory->CreateImageEncoder(main::d2d_device, image_encoder.addressof()));
    tiff(image_encoder->WriteFrame(d2d_bitmap, frame, nullptr));
    frame->Commit(), encoder->Commit(), stream->Commit(STGC_DEFAULT);
  }

  /// begins drawing to this bitmap
  void begin_draw() const { main::d2d_context->SetTarget(d2d_bitmap), main::d2d_context->BeginDraw(); }

  /// begins drawing to this bitmap after filling it with `Color`
  void begin_draw(const color& Color) const { begin_draw(), main::d2d_context->Clear(bitcast<D2D1_COLOR_F>(Color)); }

  /// ends drawing to this bitmap
  void end_draw() const { main::d2d_context->EndDraw(); }

  /// draws this bitmap to another bitmap
  void draw(rect_like auto&& Rect, const fat4 Opacity = 1.0f) const {
    if constexpr (!same_as<remove_cvref<decltype(RECT)>, vector>) {
      auto&& r = D2D1_RECT_F(fat4(get<0>(Rect)), fat4(get<1>(Rect)), fat4(get<2>(Rect)), fat4(get<3>(Rect)));
      main::d2d_context->DrawBitmap(d2d_bitmap, mv(r), Opacity);
    } else main::d2d_context->DrawBitmap(d2d_bitmap, bitcast<D2D1_RECT_F>(Rect), Opacity);
  }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// class for creating textures
class texture : public bitmap {
  void initialize() {
    comptr<IDXGISurface> surface;
    tiff(operator comptr<ID2D1Bitmap1>::reference*()->GetSurface(surface.addressof()));
    tiff(surface->QueryInterface(d3d_texture.addressof()));
    D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc{
      .Format = DXGI_FORMAT_R8G8B8A8_UNORM, .ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D, .Texture2D = {0, 1}};
    tiff(main::d3d_device->CreateShaderResourceView(*this, &srv_desc, d3d_srv.addressof()));
  }
protected:
  comptr<ID3D11Texture2D> d3d_texture;
  comptr<ID3D11ShaderResourceView> d3d_srv;
public:
  /// default constructor
  texture() noexcept = default;

  /// creates a texture with `Width` and `Height`
  texture(natt Width, natt Height) : bitmap(Width, Height) { initialize(); }

  /// creates a texture from an image file
  explicit texture(const path& Image) : bitmap(Image) { initialize(); }

  /// creates a texture from a bitmap
  explicit texture(bitmap&& Bitmap) : bitmap(mv(Bitmap)) { initialize(); }

  /// checks if this is valid
  explicit operator bool() const noexcept { return bool(d3d_texture); }

  /// conversion to a restricted pointer to `ID3D11Texture2D`
  operator comptr<ID3D11Texture2D>::reference*() const noexcept {
    ywlib_assert(d3d_texture, "this texture is not valid");
    return d3d_texture;
  }

  /// conversion to a restricted pointer to `ID3D11ShaderResourceView`
  operator comptr<ID3D11ShaderResourceView>::reference*() const noexcept {
    ywlib_assert(d3d_srv, "this texture is not valid");
    return d3d_srv;
  }

  /// sets this texture to the vertex shader
  void to_vs(natt Slot = 0) const {
    ywlib_assert(*this, "this texture is not valid");
    main::d3d_context->VSSetShaderResources(nat4(Slot), 1, &d3d_srv);
  }

  /// sets this texture to the geometry shader
  void to_gs(natt Slot = 0) const {
    ywlib_assert(*this, "this texture is not valid");
    main::d3d_context->GSSetShaderResources(nat4(Slot), 1, &d3d_srv);
  }

  /// sets this texture to the pixel shader
  void to_ps(natt Slot = 0) const {
    ywlib_assert(*this, "this texture is not valid");
    main::d3d_context->PSSetShaderResources(nat4(Slot), 1, &d3d_srv);
  }

  /// sets this texture to the compute shader
  void to_cs(natt Slot = 0) const {
    ywlib_assert(*this, "this texture is not valid");
    main::d3d_context->CSSetShaderResources(nat4(Slot), 1, &d3d_srv);
  }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// class for creating 3D render targets
class canvas : public texture {
private:
  void initialize() {
    comptr<ID3D11Texture2D> temp;
    if (const_cast<nat4&>(msaa) = msaa < 2 ? 0 : (msaa < 4 ? 2 : (msaa < 8 ? 4 : 8)); msaa) {
      D3D11_TEXTURE2D_DESC desc{
        width, height, 1, 1, DXGI_FORMAT_D24_UNORM_S8_UINT, {msaa, 0}, D3D11_USAGE_DEFAULT, D3D11_BIND_DEPTH_STENCIL};
      tiff(main::d3d_device->CreateTexture2D(&desc, nullptr, temp.addressof()));
      tiff(main::d3d_device->CreateDepthStencilView(temp, nullptr, d3d_dsv.addressof()));
      temp.reset(), desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM, desc.BindFlags = D3D11_BIND_RENDER_TARGET;
      tiff(main::d3d_device->CreateTexture2D(&desc, nullptr, temp.addressof()));
      D3D11_RENDER_TARGET_VIEW_DESC rtv_desc{DXGI_FORMAT_R8G8B8A8_UNORM, D3D11_RTV_DIMENSION_TEXTURE2DMS};
      tiff(main::d3d_device->CreateRenderTargetView(temp, &rtv_desc, d3d_rtv.addressof()));
    } else {
      D3D11_TEXTURE2D_DESC desc{
        width, height, 1, 1, DXGI_FORMAT_D24_UNORM_S8_UINT, {1, 0}, D3D11_USAGE_DEFAULT, D3D11_BIND_DEPTH_STENCIL};
      tiff(main::d3d_device->CreateTexture2D(&desc, nullptr, temp.addressof()));
      tiff(main::d3d_device->CreateDepthStencilView(temp, nullptr, d3d_dsv.addressof()));
      D3D11_RENDER_TARGET_VIEW_DESC rtv_desc{DXGI_FORMAT_R8G8B8A8_UNORM, D3D11_RTV_DIMENSION_TEXTURE2D};
      tiff(main::d3d_device->CreateRenderTargetView(*this, &rtv_desc, d3d_rtv.addressof()));
    }
  }
protected:
  comptr<ID3D11RenderTargetView> d3d_rtv;
  comptr<ID3D11DepthStencilView> d3d_dsv;
public:
  /// sampling count for MSAA
  const nat4 msaa{};

  /// default constructor
  canvas() noexcept = default;

  /// move constructor
  canvas(canvas&& Canvas) noexcept : texture(mv(Canvas)), msaa(Canvas.msaa),
                                     d3d_rtv{mv(Canvas.d3d_rtv)}, d3d_dsv{mv(Canvas.d3d_dsv)} {}

  /// creates a canvas with `Width` and `Height`
  canvas(natt Width, natt Height, natt MSAA = 0) : texture(Width, Height), msaa(nat4(MSAA)) { initialize(); }

  /// creates a canvas from a bitmap
  explicit canvas(texture&& Texture, natt MSAA = 0) : texture(mv(Texture)), msaa(nat4(MSAA)) { initialize(); }

  /// checks if this is valid
  explicit operator bool() const noexcept { return bool(d3d_rtv) && bool(d3d_dsv); }

  /// conversion to a restricted pointer to `ID3D11RenderTargetView`
  operator comptr<ID3D11RenderTargetView>::reference*() const noexcept {
    ywlib_assert(d3d_rtv, "this canvas is not valid");
    return d3d_rtv;
  }

  /// conversion to a restricted pointer to `ID3D11DepthStencilView`
  operator comptr<ID3D11DepthStencilView>::reference*() const noexcept {
    ywlib_assert(d3d_dsv, "this canvas is not valid");
    return d3d_dsv;
  }

  /// begins 3d rendering to this canvas
  template<typename... RTV_OR_UAV> void begin_render(RTV_OR_UAV&&... Views) const
    requires(bool(convertible_to<RTV_OR_UAV, ID3D11RenderTargetView*> ^ convertible_to<RTV_OR_UAV, ID3D11UnorderedAccessView*>) && ...) {
    ywlib_assert(*this, "this canvas is not valid");
    main::d3d_context->ClearDepthStencilView(d3d_dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    [](D3D11_VIEWPORT vp) { main::d3d_context->RSSetViewports(1, &vp); }({0, 0, (fat4)width, (fat4)height, 0, 1});
    static constexpr nat4 m = nat4(counts<convertible_to<RTV_OR_UAV, ID3D11RenderTargetView*>...>), n = nat4(sizeof...(RTV_OR_UAV) - m);
    if constexpr (n == 0) [&]<natt... Is>(array<ID3D11RenderTargetView*, m + 1> RTVs) {
      main::d3d_context->OMSetRenderTargets(m + 1, RTVs.data(), d3d_dsv);
    }({d3d_rtv, Views...});
    else [&](auto&& RTVs, auto&& UAVs) {
      main::d3d_context->OMSetRenderTargetsAndUnorderedAccessViews(m + 1, RTVs.data(), d3d_dsv, m + 1, n, UAVs.data(), nullptr);
    }([&]<natt... Is>(sequence<Is...>) { return array<ID3D11RenderTargetView*, m + 1>{d3d_rtv, parameter_switch<Is>(Views...)...}; }(
        cond_indices<sequence<convertible_to<RTV_OR_UAV, ID3D11RenderTargetView*>...>>{}),
      [&]<natt... Is>(sequence<Is...>) { return array<ID3D11UnorderedAccessView*, n>{parameter_switch<Is>(Views...)...}; }(
        cond_indices<sequence<convertible_to<RTV_OR_UAV, ID3D11UnorderedAccessView*>...>>{}));
  };

  /// begins 3d rendering to this canvas after clearing it with `Color`
  template<typename... RTV_OR_UAV> void begin_render(const color& Color, RTV_OR_UAV&&... Views) const
    requires(bool(convertible_to<RTV_OR_UAV, ID3D11RenderTargetView*> ^ convertible_to<RTV_OR_UAV, ID3D11UnorderedAccessView*>) && ...) {
    ywlib_assert(*this, "this canvas is not valid");
    main::d3d_context->ClearRenderTargetView(d3d_rtv, &Color.r);
    begin_render(fwd<RTV_OR_UAV>(Views)...);
  }

  /// ends 3d rendering to this canvas
  void end_render() const {
    if (msaa) [&](comptr<ID3D11Resource> a) {
      d3d_rtv->GetResource(a.addressof());
      main::d3d_context->ResolveSubresource(*this, 0, a, 0, DXGI_FORMAT_R8G8B8A8_UNORM); }({});
    main::d3d_context->OMSetRenderTargets(0, nullptr, nullptr);
  }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// class for creating cameras for 3D rendering
class camera : public canvas {
public:
  /// view matrix
  xmatrix view;

  /// view * projection matrix
  xmatrix view_proj;

  /// offsets of camera lens form the center of body
  vector offset{};

  /// radians to rotate the camera around the x, y, z-axis
  vector rotation{};

  /// position of the camera
  vector position{};

  /// magnification factor
  /// @note If orthographic projection is used, this is the magnification factor of the lens.
  /// @note If perspective projection is used, this is the reciprocal tangent of the half of the field of view.
  fat4 factor = 1.0f;

  /// enables or disables orthographic projection
  bool orthographic = false;

  /// default constructor
  camera() noexcept = default;

  /// creates a camera with `Width` and `Height`
  camera(natt Width, natt Height, natt MSAA = 0) : canvas(Width, Height, MSAA) {}

  /// creates a camera from a canvas
  explicit camera(canvas&& Canvas) : canvas(mv(Canvas)) {}

  /// updates the matrices
  void update() {
    constexpr fat4 f = 1048576.0f, n = 0.25f;
    xvview(position, rotation, offset, view);
    if (orthographic) {
      auto a = xv(-2.0f * factor / width, 2.0f * factor / height, 1.0f / (f - n), n / (f - n));
      view_proj[0] = xvmul(view[0], xvpermute<0, 0, 0, 0>(a));
      view_proj[1] = xvmul(view[1], xvpermute<1, 1, 1, 1>(a));
      view_proj[2] = xvsub(xvmul(view[2], xvpermute<2, 2, 2, 2>(a)), xvsetzero<1, 1, 1, 0>(a));
      view_proj[3] = view[3];
    } else {
      auto a = xv((-factor * height) / width, factor, f / (f - n), -f * n / (f - n));
      view_proj[0] = xvmul(view[0], xvpermute<0, 0, 0, 0>(a));
      view_proj[1] = xvmul(view[1], xvpermute<1, 1, 1, 1>(a));
      view_proj[2] = xvadd(xvmul(view[2], xvpermute<2, 2, 2, 2>(a)), xvsetzero<1, 1, 1, 0>(a));
      view_proj[3] = view[2];
    }
  }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace key {

/// class for handling key states
class key {
public:
  /// application time when the key was pressed
  /// @note if the ky is not pressed, this is `0`.
  const fat8 pushed_time{};

  /// callback function for key press
  void (*down)() = nullptr;

  /// callback function for key release
  void (*up)(fat8 hold_time) = nullptr;

  /// checks if the key is pressed
  explicit operator bool() const noexcept { return pushed_time != 0; }
};

/// array of keys
inline array<key, 256> keys{};

inline key &lbutton = keys[VK_LBUTTON], &rbutton = keys[VK_RBUTTON], &mbutton = keys[VK_MBUTTON];
inline key &xbutton1 = keys[VK_XBUTTON1], &xbutton2 = keys[VK_XBUTTON2], &win = keys[VK_LWIN], &app = keys[VK_APPS];
inline key &back = keys[VK_BACK], &tab = keys[VK_TAB], &enter = keys[VK_RETURN], &escape = keys[VK_ESCAPE];
inline key &shift = keys[VK_SHIFT], &control = keys[VK_CONTROL], &alt = keys[VK_MENU], &space = keys[VK_SPACE];
inline key &pageup = keys[VK_PRIOR], &pagedown = keys[VK_NEXT], &end = keys[VK_END], &home = keys[VK_HOME];
inline key &left = keys[VK_LEFT], &up = keys[VK_UP], &right = keys[VK_RIGHT], &down = keys[VK_DOWN];
inline key &screenshot = keys[VK_SNAPSHOT], &insert = keys[VK_INSERT], &delete_ = keys[VK_DELETE];
inline key &n0 = keys['0'], &n1 = keys['1'], &n2 = keys['2'], &n3 = keys['3'], &n4 = keys['4'];
inline key &n5 = keys['5'], &n6 = keys['6'], &n7 = keys['7'], &n8 = keys['8'], &n9 = keys['9'];
inline key &a = keys['A'], &b = keys['B'], &c = keys['C'], &d = keys['D'], &e = keys['E'], &f = keys['F'], &g = keys['G'], &h = keys['H'], &i = keys['I'];
inline key &j = keys['J'], &k = keys['K'], &l = keys['L'], &m = keys['M'], &n = keys['N'], &o = keys['O'], &p = keys['P'], &q = keys['Q'], &r = keys['R'];
inline key &s = keys['S'], &t = keys['T'], &u = keys['U'], &v = keys['V'], &w = keys['W'], &x = keys['X'], &y = keys['Y'], &z = keys['Z'];
inline key &np_0 = keys[VK_NUMPAD0], &np_1 = keys[VK_NUMPAD1], &np_2 = keys[VK_NUMPAD2], &np_3 = keys[VK_NUMPAD3], &np_4 = keys[VK_NUMPAD4];
inline key &np_5 = keys[VK_NUMPAD5], &np_6 = keys[VK_NUMPAD6], &np_7 = keys[VK_NUMPAD7], &np_8 = keys[VK_NUMPAD8], &np_9 = keys[VK_NUMPAD9];
inline key &f1 = keys[VK_F1], &f2 = keys[VK_F2], &f3 = keys[VK_F3], &f4 = keys[VK_F4], &f5 = keys[VK_F5], &f6 = keys[VK_F6];
inline key &f7 = keys[VK_F7], &f8 = keys[VK_F8], &f9 = keys[VK_F9], &f10 = keys[VK_F10], &f11 = keys[VK_F11], &f12 = keys[VK_F12];
inline key &minus = keys[VK_OEM_MINUS], &plus = keys[VK_OEM_PLUS], &comma = keys[VK_OEM_COMMA], &period = keys[VK_OEM_PERIOD];
inline key &oem00 = keys[VK_OEM_7], &oem01 = keys[VK_OEM_5], &oem10 = keys[VK_OEM_3], &oem11 = keys[VK_OEM_4];
inline key &oem20 = keys[VK_OEM_1], &oem21 = keys[VK_OEM_6], &oem30 = keys[VK_OEM_2], &oem31 = keys[VK_OEM_102];
}

/// empty class for operating the mouse
class mouse {
public:
  /// sets the position of the mouse cursor
  mouse(int4 X, int4 Y) {
    [&](RECT r) { GetClientRect(main::hwnd, &r), SetCursorPos(r.left + X, r.top + Y); }({});
    const_cast<int4&>(main::mouse.left) = X, const_cast<int4&>(main::mouse.top) = Y;
  }

  /// x-position of the mouse cursor
  inline static const auto& x = main::mouse.left;

  /// y-position of the mouse cursor
  inline static const auto& y = main::mouse.top;

  /// checks if the mouse cursor is hovering over the window
  inline static const bool& hover = main::hover;

  /// callback function for mouse wheel
  inline static void (*wheeled)(fat4 delta) = nullptr;

  /// callback function for mouse movement
  inline static void (*moved)(int4 dx, int4 dy) = nullptr;

  /// callback function for mouse left button press
  inline static auto& left = key::lbutton;

  /// callback function for mouse right button press
  inline static auto& right = key::rbutton;

  /// callback function for mouse middle button press
  inline static auto& middle = key::mbutton;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// draws a rectangle
/// BAD CODE
void draw_rectangle(const rect& Rect, convertible_to<ID2D1SolidColorBrush*> auto&& Brush, fat4 Opacity = 1.0f) {
  main::d2d_context->FillRectangle(bitcast<D2D1_RECT_F>(Rect), Brush);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace ui {

/// creates a message box with an OK button and returns `true`
inline constexpr overload ok{
  [](const stv1& Text, const stv1& Caption = "OK?") {
    if (Text.null_terminated()) {
      if (Caption.null_terminated()) MessageBoxA(main::hwnd, Text.data(), Caption.data(), MB_OK | MB_ICONEXCLAMATION);
      else [&](str1 c) { MessageBoxA(main::hwnd, Text.data(), c.data(), MB_OK | MB_ICONEXCLAMATION); }(Caption);
    } else {
      if (str1 t{Text}; Caption.null_terminated()) MessageBoxA(main::hwnd, t.data(), Caption.data(), MB_OK | MB_ICONEXCLAMATION);
      else [&](str1 c) { MessageBoxA(main::hwnd, t.data(), c.data(), MB_OK | MB_ICONEXCLAMATION); }(Caption);
    }
    return true;
  },
  [](const stv2& Text, const stv2& Caption = L"OK?") {
    if (Text.null_terminated()) {
      if (Caption.null_terminated()) MessageBoxW(main::hwnd, Text.data(), Caption.data(), MB_OK | MB_ICONEXCLAMATION);
      else [&](str2 c) { MessageBoxW(main::hwnd, Text.data(), c.data(), MB_OK | MB_ICONEXCLAMATION); }(Caption);
    } else {
      if (str2 t{Text}; Caption.null_terminated()) MessageBoxW(main::hwnd, t.data(), Caption.data(), MB_OK | MB_ICONEXCLAMATION);
      else [&](str2 c) { MessageBoxW(main::hwnd, t.data(), c.data(), MB_OK | MB_ICONEXCLAMATION); }(Caption);
    }
    return true;
  }};

/// creates a message box with YES/NO buttons and returns `true` if YES is pressed
inline constexpr overload yes{
  [](const stv1& Text, const stv1& Caption = "Yes?") {
    if (Text.null_terminated()) {
      if (Caption.null_terminated()) return MessageBoxA(main::hwnd, Text.data(), Caption.data(), MB_YESNO | MB_ICONQUESTION) == IDYES;
      else return [&](str1 c) { return MessageBoxA(main::hwnd, Text.data(), c.data(), MB_YESNO | MB_ICONQUESTION) == IDYES; }(Caption);
    } else {
      if (str1 t{Text}; Caption.null_terminated()) return MessageBoxA(main::hwnd, t.data(), Caption.data(), MB_YESNO | MB_ICONQUESTION) == IDYES;
      else return [&](str1 c) { return MessageBoxA(main::hwnd, t.data(), c.data(), MB_YESNO | MB_ICONQUESTION) == IDYES; }(Caption);
    }
  },
  [](const stv2& Text, const stv2& Caption = L"Yes?") {
    if (Text.null_terminated()) {
      if (Caption.null_terminated()) return MessageBoxW(main::hwnd, Text.data(), Caption.data(), MB_YESNO | MB_ICONQUESTION) == IDYES;
      else return [&](str2 c) { return MessageBoxW(main::hwnd, Text.data(), c.data(), MB_YESNO | MB_ICONQUESTION) == IDYES; }(Caption);
    } else {
      if (str2 t{Text}; Caption.null_terminated()) return MessageBoxW(main::hwnd, t.data(), Caption.data(), MB_YESNO | MB_ICONQUESTION) == IDYES;
      else return [&](str2 c) { return MessageBoxW(main::hwnd, t.data(), c.data(), MB_YESNO | MB_ICONQUESTION) == IDYES; }(Caption);
    }
  }};

/// creates a file open dialog and returns the selected file
inline path open_file(const path& InitialDir = {}) {
  cat2 buf[260]{0};
  OPENFILENAMEW ofn{DWORD(sizeof(ofn)), main::hwnd, 0, L"All Files (*.*)\0*.*\0", 0, 0, 1, buf, nat4(extent<decltype(buf)>)};
  ofn.lpstrInitialDir = InitialDir.empty() ? nullptr : InitialDir.c_str(), ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
  return GetOpenFileNameW(&ofn) == TRUE ? path(ofn.lpstrFile) : path{};
}

/// creates a file save dialog and returns the selected file
inline path save_file(const path& InitialDir = {}, const path& InitialFileName = {}) {
  cat2 buf[260]{0};
  if (!InitialFileName.empty()) std::ranges::copy(InitialFileName.filename().wstring(), buf);
  OPENFILENAMEW ofn{DWORD(sizeof(ofn)), main::hwnd, 0, L"All Files (*.*)\0*.*\0", 0, 0, 1, buf, nat4(extent<decltype(buf)>)};
  ofn.lpstrInitialDir = InitialDir.empty() ? nullptr : InitialDir.c_str(), ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
  return GetSaveFileNameW(&ofn) == TRUE ? path(ofn.lpstrFile) : path{};
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// base class for creating controls
class control {
  inline static HFONT hfont{};
public:
  /// maximum number of control groups
  inline static constexpr natt max_groups = 16;

  /// hides all controls in the group
  static void hide(natt GroupNo, bool Hide = true) {
    ywlib_assert(GroupNo < max_groups, "Control group index is out of range");
    for (auto& g : groups[GroupNo]) g.second->hide(Hide);
  }

  /// handle of the control
  const HWND hwnd{};

  /// group number of the control
  const natt group{npos};

  /// destructor
  ~control() noexcept {
    if (!hwnd) return;
    DestroyWindow(hwnd), whole_controls.erase(hwnd);
    groups[group + 1].erase(groups[group + 1].begin() + get_index(groups[group + 1], hwnd));
  }

  /// default constructor
  control() noexcept = default;

  /// move constructor
  control(control&& A) : hwnd(exchange(const_cast<HWND&>(A.hwnd), nullptr)), group(A.group) {
    whole_controls[hwnd].second = this;
    groups[group + 1][get_index(groups[group + 1], hwnd)].second = this;
  }

  /// move assignment
  control& operator=(control&& A) {
    if (auto& g = groups[group + 1]; hwnd)
      DestroyWindow(hwnd), g.erase(g.begin() + get_index(g, hwnd)), whole_controls.erase(hwnd);
    const_cast<HWND&>(hwnd) = exchange(const_cast<HWND&>(A.hwnd), nullptr), const_cast<natt&>(group) = A.group;
    if (hwnd) whole_controls[hwnd].second = this, groups[group + 1][get_index(groups[group + 1], hwnd)].second = this;
    return *this;
  }

  /// checks if this is valid
  explicit operator bool() const noexcept { return hwnd; }

  /// sets the focus to this
  virtual void setfocus() const { ywlib_assert(hwnd, "This control is not valid"), SetFocus(hwnd); }

  /// hids or shows this
  virtual void hide(bool Hide = true) const {
    ywlib_assert(hwnd, "This control is not valid");
    ShowWindow(hwnd, Hide ? SW_HIDE : SW_SHOW);
  }

  /// sets the text to this
  virtual void text(const str2& Text) const {
    ywlib_assert(hwnd, "This control is not valid");
    SetWindowTextW(hwnd, Text.data());
  }

  /// gets the text of this
  virtual str2 text() const {
    ywlib_assert(hwnd, "This control is not valid");
    str2 out(GetWindowTextLengthW(hwnd), {});
    return GetWindowTextW(hwnd, out.data(), int4(out.size() + 1)), mv(out);
  }
protected:
  /// control groups
  inline static array<array<list<HWND, control*, bool>>, max_groups + 1> groups;

  /// whole controls
  inline static std::map<HWND, list<natt, control*>> whole_controls;

  /// creates a control and adds it to group[`-1 + 1`]
  control(const cat2* Class, const str2& Text, nat4 Style, const rect& Rect, bool Inputable)
    : hwnd(CreateWindowExW(0, Class, Text.data(), WS_VISIBLE | WS_CHILD | WS_GROUP | Style, Rect.left, Rect.top,
                           Rect.width(), Rect.height(), main::hwnd, 0, main::hinstance, this)) {
    if (!hfont) hfont = CreateFontW(16, 0, 0, 0, FW_NORMAL, 0, 0, 0, SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS,
                                    CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, VARIABLE_PITCH | FF_ROMAN, L"Yu Gothic UI");
    SendMessageW(hwnd, WM_SETFONT, (WPARAM)hfont, true), SetWindowLongPtrW(hwnd, GWLP_ID, (LONG_PTR)hwnd);
    if (groups[0].empty()) groups[0].reserve(1024);
    groups[0].push_back(list{hwnd, this, Inputable}), whole_controls[hwnd] = list{npos, this};
  }

  /// creates a control and adds it to groups[`GroupNo + 1`]
  /// @note `GroupNo` must be less than `max_groups`
  control(const cat2* Class, const str2& Text, nat4 Style, const rect& Rect, const natt GroupNo, bool Inputable)
    : hwnd(CreateWindowExW(0, Class, Text.data(), WS_VISIBLE | WS_CHILD | WS_GROUP | Style, Rect.left, Rect.top,
                           Rect.width(), Rect.height(), main::hwnd, 0, main::hinstance, this)) {
    ywlib_assert(GroupNo < max_groups, "Control group index is out of range");
    if (!hfont) hfont = CreateFontW(16, 0, 0, 0, FW_NORMAL, 0, 0, 0, SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS,
                                    CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, VARIABLE_PITCH | FF_ROMAN, L"Yu Gothic UI");
    SendMessageW(hwnd, WM_SETFONT, (WPARAM)hfont, true), SetWindowLongPtrW(hwnd, GWLP_ID, (LONG_PTR)hwnd);
    if (groups[GroupNo + 1].empty()) groups[GroupNo + 1].reserve(1024);
    groups[GroupNo + 1].push_back(list{hwnd, this, Inputable}), whole_controls[hwnd] = list{GroupNo, this};
  }

  /// gets a control group by `GroupNo`
  static array<list<HWND, control*, bool>>& get_group(const natt GroupNo) {
    ywlib_assert(GroupNo < max_groups, "Control group index is out of range");
    return groups[GroupNo + 1];
  }

  /// gets the index of a control
  static natt get_index(const array<list<HWND, control*, bool>>& Group, HWND Handle) {
    for (natt i{}; i < Group.size(); ++i)
      if (Group[i].first == Handle) return i;
    return npos;
  }

  /// changes the focus to the next/prev control
  static void focus_on_next(const array<list<HWND, control*, bool>>& Group, natt Now, bool Prev) {
    if (Prev) {
      for (natt j = Now - 1; j < Now; --j)
        if (Group[j].third) return Group[j].second->setfocus(), std::cout << j << std::endl, void();
      for (natt j = Group.size() - 1; Now < j; --j)
        if (Group[j].third) return Group[j].second->setfocus(), std::cout << j << std::endl, void();
    } else {
      for (natt j = Now + 1; j < Group.size(); ++j)
        if (Group[j].third) return Group[j].second->setfocus(), std::cout << j << std::endl, void();
      for (natt j = 0; j < Now; ++j)
        if (Group[j].third) return Group[j].second->setfocus(), std::cout << j << std::endl, void();
    }
  }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// class for creating textbox controls
class textbox : public control {
protected:
  inline static WNDPROC defproc = nullptr;
  static LRESULT CALLBACK proc(HWND hw, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_CHAR) {
      if (wp == VK_RETURN) {
        if (auto& t = *static_cast<textbox*>(whole_controls[hw].second); t.enter) return t.enter(t), 0;
      } else if (wp == VK_TAB) {
        auto& g = get_group(whole_controls[hw].first);
        auto i = get_index(g, hw);
        auto& t = *static_cast<textbox*>(g[i].second);
        if (t.tab) t.tab(t);
        return focus_on_next(g, i, GetKeyState(VK_SHIFT) < 0), 0;
      } else if (wp == VK_ESCAPE) return SetFocus(main::hwnd), 0;
    } else if (msg == WM_SETFOCUS) {
      if (auto& t = *static_cast<textbox*>(whole_controls[hw].second); t.intofocus) return t.intofocus(t), 0;
    } else if (msg == WM_KILLFOCUS) {
      if (auto& t = *static_cast<textbox*>(whole_controls[hw].second); t.killfocus) return t.killfocus(t), 0;
    }
    return CallWindowProcW(defproc, hw, msg, wp, lp);
  }
public:
  void (*enter)(const textbox& This) = nullptr;
  void (*tab)(const textbox& This) = nullptr;
  void (*intofocus)(const textbox& This) = nullptr;
  void (*killfocus)(const textbox& This) = nullptr;
  textbox() noexcept = default;
  textbox(textbox&&) = default;
  textbox& operator=(textbox&&) = default;
  textbox(const rect& Rect, const str2& Init = L"", nat4 EditStyle = {})
    : control(L"EDIT", Init, EditStyle, Rect, true) {
    if (!defproc) defproc = (WNDPROC)SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)proc);
    SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)proc);
  }
  textbox(const natt GroupNo, const rect& Rect, const str2& Init, nat4 EditStyle = {})
    : control(L"EDIT", Init, EditStyle, Rect, GroupNo, true) {
    if (!defproc) defproc = (WNDPROC)SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)proc);
    SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)proc);
  }
};

/// class for creating a valuebox control
class valuebox : public control {
protected:
  inline static WNDPROC defproc = nullptr;
  static LRESULT CALLBACK proc(HWND hw, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_CHAR) {
      if (wp == VK_RETURN) {
        if (auto& t = *static_cast<valuebox*>(whole_controls[hw].second); t.enter) return t.enter(t), 0;
      } else if (wp == VK_TAB) {
        auto& g = get_group(whole_controls[hw].first);
        auto i = get_index(g, hw);
        auto& t = *static_cast<valuebox*>(g[i].second);
        if (t.tab) t.tab(t);
        return focus_on_next(g, i, GetKeyState(VK_SHIFT) < 0), 0;
      } else if (wp == VK_ESCAPE) return SetFocus(main::hwnd), 0;
      else if (wp == VK_BACK || wp == VK_DELETE || wp == VK_LEFT || wp == VK_RIGHT) void(0);
      else if (!(wp >= '0' && wp <= '9' || wp == '-' || wp == '.')) return 0;
    } else if (msg == WM_SETFOCUS) {
      if (auto& t = *static_cast<valuebox*>(whole_controls[hw].second); t.intofocus) return t.intofocus(t), 0;
    } else if (msg == WM_KILLFOCUS) {
      if (auto& t = *static_cast<valuebox*>(whole_controls[hw].second); t.killfocus) return t.killfocus(t), 0;
    }
    return CallWindowProcW(defproc, hw, msg, wp, lp);
  }
public:
  void (*enter)(const valuebox& This) = nullptr;
  void (*tab)(const valuebox& This) = nullptr;
  void (*intofocus)(const valuebox& This) = nullptr;
  void (*killfocus)(const valuebox& This) = nullptr;
  valuebox() noexcept = default;
  valuebox(valuebox&&) = default;
  valuebox& operator=(valuebox&&) = default;
  valuebox(const rect& Rect, const arithmetic auto Init, nat4 EditStyle = {})
    : control(L"EDIT", vtos<cat2>(Init), EditStyle, Rect, true) {
    if (!defproc) defproc = (WNDPROC)SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)proc);
    SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)proc);
  }
  valuebox(const natt GroupNo, const rect& Rect, const arithmetic auto Init, nat4 EditStyle = {})
    : control(L"EDIT", vtos<cat2>(Init), EditStyle, Rect, GroupNo, true) {
    if (!defproc) defproc = (WNDPROC)SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)proc);
    SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)proc);
  }
  void value(const arithmetic auto Value) const { SetWindowTextW(hwnd, vtos<cat2>(Value).data()); }
  template<arithmetic T> T value() const { return stov<T>(text()); }
};

/// class for creating a label control
class label : public control {
public:
  label() noexcept = default;
  label(label&&) = default;
  label& operator=(label&&) = default;
  label(const rect& Rect, const str2& Text, nat4 StaticStyle = {})
    : control(L"STATIC", Text, StaticStyle, Rect, false) {}
  label(const natt GroupNo, const rect& Rect, const str2& Text, nat4 StaticStyle = {})
    : control(L"STATIC", Text, StaticStyle, Rect, GroupNo, false) {}
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// class for obtaining the button state
template<typename Button, invocable<const Button&> Converter> class button_state {
  const Button* ptr;
  Converter func;
public:
  template<typename F> button_state(const Button* B, F&& Func) noexcept : ptr(B), func(fwd<F>(Func)) {}
  operator invoke_result_t<Converter, const Button&>() const { return func(*ptr); }
};
template<typename B, typename F> button_state(const B*, F&&) -> button_state<B, F>;

/// class for creating button controls
class button : public control {
  static bool get_state(const button& This) { return bool(defproc(This.hwnd, BM_GETSTATE, 0, 0) & BST_PUSHED); }
protected:
  inline static WNDPROC defproc = nullptr;
  static LRESULT CALLBACK proc(HWND hw, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_LBUTTONDOWN) {
      if (auto& t = *static_cast<button*>(whole_controls[hw].second); t.enter)
        return [&](auto r) { return t.enter(t), r; }(defproc(hw, msg, wp, lp));
    } else if (msg == WM_KEYDOWN) {
      if (wp == VK_RETURN || wp == VK_SPACE) {
        if (auto& t = *static_cast<button*>(whole_controls[hw].second); t.enter)
          return [&](auto r) { return t.enter(t), r; }(defproc(hw, msg, wp, lp));
      } else if (wp == VK_TAB) {
        auto& g = get_group(whole_controls[hw].first);
        auto i = get_index(g, hw);
        return focus_on_next(g, i, GetKeyState(VK_SHIFT) < 0), 0;
      } else if (wp == VK_ESCAPE) return SetFocus(main::hwnd), 0;
    }
    return CallWindowProcW(defproc, hw, msg, wp, lp);
  }
public:
  const button_state<button, decltype(&get_state)> state;
  void (*enter)(const button& This) = nullptr;
  button() noexcept : control(), state(this, get_state) {}
  button(button&& A) : control(mv(dynamic_cast<control&>(A))), state(this, get_state), enter(A.enter) {}
  button& operator=(button&& A) {
    return dynamic_cast<control&>(*this) = mv(dynamic_cast<control&>(A)), enter = A.enter, *this;
  }
  button(const rect& Rect, const str2& Text, nat4 ButtonStyle = {})
    : control(L"BUTTON", Text.data(), ButtonStyle, Rect, true), state(this, get_state) {
    if (!defproc) defproc = (WNDPROC)GetWindowLongPtrW(hwnd, GWLP_WNDPROC);
    SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)proc);
  }
  button(natt GroupNo, const rect& Rect, const str2& Text, nat4 ButtonStyle = {})
    : control(L"BUTTON", Text.data(), ButtonStyle, Rect, GroupNo, true), state(this, get_state) {
    if (!defproc) defproc = (WNDPROC)GetWindowLongPtrW(hwnd, GWLP_WNDPROC);
    SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)proc);
  }
};

/// class for creating checkbox controls
class checkbox : public control {
  static bool get_state(const checkbox& This) { return bool(defproc(This.hwnd, BM_GETCHECK, 0, 0) & BST_CHECKED); }
protected:
  inline static WNDPROC defproc = nullptr;
  static LRESULT CALLBACK proc(HWND hw, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_LBUTTONUP) {
      if (auto& t = *static_cast<checkbox*>(whole_controls[hw].second); t.enter)
        return [&](auto r) { return t.enter(t), r; }(defproc(hw, msg, wp, lp));
    } else if (msg == WM_KEYUP) {
      if (wp == VK_RETURN || wp == VK_SPACE)
        if (auto& t = *static_cast<checkbox*>(whole_controls[hw].second); t.enter)
          return [&](auto r) { return t.enter(t), r; }(defproc(hw, msg, VK_SPACE, lp));
    } else if (msg == WM_KEYDOWN) {
      if (wp == VK_RETURN) wp = VK_SPACE;
      else if (wp == VK_TAB) {
        auto& g = get_group(whole_controls[hw].first);
        auto i = get_index(g, hw);
        return focus_on_next(g, i, GetKeyState(VK_SHIFT) < 0), 0;
      } else if (wp == VK_ESCAPE) return SetFocus(main::hwnd), 0;
    }
    return CallWindowProcW(defproc, hw, msg, wp, lp);
  }
public:
  const button_state<checkbox, decltype(&get_state)> state;
  void (*enter)(const checkbox& This) = nullptr;
  checkbox() noexcept : control(), state(this, get_state) {}
  checkbox(checkbox&& A) : control(mv(dynamic_cast<control&>(A))), state(this, get_state), enter(A.enter) {}
  checkbox& operator=(checkbox&& A) {
    return dynamic_cast<control&>(*this) = mv(dynamic_cast<control&>(A)), enter = A.enter, *this;
  }
  checkbox(const rect& Rect, const str2& Text, nat4 ButtonStyle = {})
    : control(L"BUTTON", Text.data(), BS_AUTOCHECKBOX | ButtonStyle, Rect, true), state(this, get_state) {
    if (!defproc) defproc = (WNDPROC)GetWindowLongPtrW(hwnd, GWLP_WNDPROC);
    SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)proc);
  }
  checkbox(natt GroupNo, const rect& Rect, const str2& Text, nat4 ButtonStyle = {})
    : control(L"BUTTON", Text.data(), BS_AUTOCHECKBOX | ButtonStyle, Rect, GroupNo, true), state(this, get_state) {
    if (!defproc) defproc = (WNDPROC)GetWindowLongPtrW(hwnd, GWLP_WNDPROC);
    SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)proc);
  }
};

/// class for creating radiobutton controls
class radiobutton : public control {
  nat4 get_index(HWND hw) {
    for (nat4 i{}; i < buttons.size(); ++i)
      if (buttons[i] == hw) return i;
    return nat4(-1);
  }
  void initialize(const rect& r, const str2& t, const auto& ts) {
    if (!defproc) defproc = (WNDPROC)GetWindowLongPtrW(hwnd, GWLP_WNDPROC);
    SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)proc);
    int4 height = r.height() / (1 + extent<decltype(ts)>), width = r.width();
    auto hw = CreateWindowExW(WS_EX_TOPMOST, L"BUTTON", t.data(),
                              WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON | WS_GROUP,
                              0, 0, width, height, hwnd, 0, main::hinstance, 0);
    if (!defproc2) defproc2 = (WNDPROC)GetWindowLongPtrW(hw, GWLP_WNDPROC);
    SendMessageW(hw, WM_SETFONT, (WPARAM)hfont, true);
    SetWindowLongPtrW(hw, GWLP_WNDPROC, (LONG_PTR)proc2);
    SetWindowLongPtrW(hw, GWLP_ID, (LONG_PTR)hw);
    buttons.emplace_back(hw);
    cfor<0, [](natt i) { return i < extent<decltype(ts)>; }>(
      [&](const str2& Text, natt i) {
        auto hw = CreateWindowExW(WS_EX_TOPMOST, L"BUTTON", Text.data(),
                                  WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,
                                  0, int4(i) * height, width,  height,
                                  hwnd, 0, main::hinstance, 0);
        SendMessageW(hw, WM_SETFONT, (WPARAM)hfont, true);
        SetWindowLongPtrW(hw, GWLP_WNDPROC, (LONG_PTR)proc2);
        SetWindowLongPtrW(hw, GWLP_ID, (LONG_PTR)hw);
        buttons.emplace_back(hw); }, ts, make_sequence<1 + extent<decltype(ts)>, 1>{});
    SendMessageW(buttons[0], BM_SETCHECK, BST_CHECKED, 0);
  }
protected:
  array<HWND> buttons{};
  inline static WNDPROC defproc = nullptr, defproc2 = nullptr;
  static LRESULT CALLBACK proc(HWND hw, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_COMMAND && HIWORD(wp) == BN_CLICKED) {
      auto& t = *static_cast<radiobutton*>(whole_controls[hw].second);
      const_cast<nat4&>(t.state) = t.get_index((HWND)lp);
      std::cout << t.state << std::endl;
      if (t.enter) t.enter(t);
    }
    return CallWindowProcW(defproc, hw, msg, wp, lp);
  }
  static LRESULT CALLBACK proc2(HWND hw, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_LBUTTONUP) {
      if (auto& t = *static_cast<radiobutton*>(whole_controls[GetParent(hw)].second); t.enter)
        return [&](auto r) { return t.enter(t), r; }(defproc(hw, msg, wp, lp));
    } else if (msg == WM_KEYUP) {
      if (wp == VK_RETURN || wp == VK_SPACE)
        if (auto& t = *static_cast<radiobutton*>(whole_controls[GetParent(hw)].second); t.enter)
          return [&](auto r) { return t.enter(t), r; }(defproc(hw, msg, wp, lp));
    } else if (msg == WM_KEYDOWN) {
      if (wp == VK_RETURN) wp = VK_SPACE;
      else if (wp == VK_TAB) {
        auto p = GetParent(hw);
        auto& g = get_group(whole_controls[p].first);
        auto i = control::get_index(g, p);
        return focus_on_next(g, i, GetKeyState(VK_SHIFT) < 0), 0;
      } else if (wp == VK_UP || wp == VK_LEFT) {
        auto& t = *static_cast<radiobutton*>(whole_controls[GetParent(hw)].second);
        // const_cast<nat4&>(t.state) = static_cast<nat4>((t.get_index(hw) + t.buttons.size() - 1) % t.buttons.size());
        SetFocus(t.buttons[(t.get_index(hw) + t.buttons.size() - 1) % t.buttons.size()]);
      } else if (wp == VK_DOWN || wp == VK_RIGHT) {
        auto& t = *static_cast<radiobutton*>(whole_controls[GetParent(hw)].second);
        // const_cast<nat4&>(t.state) = (t.get_index(hw) + 1) % t.buttons.size();
        SetFocus(t.buttons[(t.get_index(hw) + 1) % t.buttons.size()]);
      } else if (wp == VK_ESCAPE) return SetFocus(main::hwnd), 0;
    }
    return CallWindowProcW(defproc2, hw, msg, wp, lp);
  }
public:
  const nat4 state{};
  const nat4 count{};
  void (*enter)(const radiobutton& This) = nullptr;
  radiobutton() noexcept = default;
  radiobutton(radiobutton&& A) : control(mv(dynamic_cast<control&>(A))), buttons(mv(A.buttons)), count(A.count), enter(A.enter) {}
  radiobutton& operator=(radiobutton&& A) {
    dynamic_cast<control&>(*this) = mv(dynamic_cast<control&>(A));
    return buttons = mv(A.buttons), const_cast<nat4&>(count) = A.count, enter = A.enter, *this;
  }
  radiobutton(const rect& Rect, const str2& Title,
              const str2& FirstButtonText, convertible_to<str2> auto&&... ButtonTexts)
    : control(L"BUTTON", Title.data(), WS_BORDER | BS_GROUPBOX, Rect, true), count(1 + sizeof...(ButtonTexts)) {
    initialize(Rect, FirstButtonText, list<>::asref(ButtonTexts...));
  }
  radiobutton(natt GroupNo, const rect& Rect, const str2& Title,
              const str2& FirstButtonText, convertible_to<str2> auto&&... ButtonTexts)
    : control(L"BUTTON", Title.data(), WS_BORDER | BS_GROUPBOX, Rect, GroupNo, true), count(1 + sizeof...(ButtonTexts)) {
    initialize(Rect, FirstButtonText, list<>::asref(ButtonTexts...));
  }
  virtual void setfocus() const override { SetFocus(buttons[state]); }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// class for displaying a bitmap on main-window
class bitmap : public control {
protected:
  yw::rect yw_rect{};
  yw::bitmap yw_bitmap{};
  bitmap(const bitmap&) = delete;
  bitmap& operator=(const bitmap&) = delete;
  bitmap(yw::bitmap&& Bitmap) : yw_bitmap(mv(Bitmap)) {}
public:
  bitmap() noexcept = default;
  bitmap(bitmap&&) noexcept = default;
  bitmap& operator=(bitmap&&) noexcept = default;
  bitmap(const rect& Rect) : control(L"STATIC", L"", 0, {}, false), yw_rect(Rect), yw_bitmap(Rect.width(), Rect.height()) {}
  bitmap(natt GroupNo, const rect& Rect) : control(L"STATIC", L"", 0, {}, GroupNo, false), yw_rect(Rect), yw_bitmap(Rect.width(), Rect.height()) {}
};

class coordinator {
protected:
  texture tex;
  camera cmr;
  coordinator(const coordinator&) = delete;
  coordinator& operator=(const coordinator&) = delete;
  void initialize() {
    static constexpr auto ff = font<200, L"Yu Gothic UI", 0>{};
    static constexpr auto bb = brush<color::white>{};
    cmr.orthographic = true;
    cmr.offset.z = -10;
    cmr.factor = 0.3125f * min(cmr.width, cmr.height);
    tex.begin_draw(color::black);
    rect{0, 0, 1024, 512}.draw_rectangle(brush<color::red>{});
    rect{1024, 0, 2048, 512}.draw_rectangle(brush<color::blue>{});
    rect{2048, 0, 3072, 512}.draw_rectangle(brush<color::green>{});
    rect{0, 100, 512, 512}.draw_text(L"+X", ff, bb);
    rect{512, 100, 1024, 512}.draw_text(L"-X", ff, bb);
    rect{1024, 100, 1512, 512}.draw_text(L"+Y", ff, bb);
    rect{1512, 100, 2048, 512}.draw_text(L"-Y", ff, bb);
    rect{2048, 100, 2512, 512}.draw_text(L"+Z", ff, bb);
    rect{2512, 100, 3072, 512}.draw_text(L"-Z", ff, bb);
    tex.end_draw();
    operator()({});
  }
public:
  coordinator() noexcept = default;
  coordinator(coordinator&&) noexcept = default;
  coordinator& operator=(coordinator&&) noexcept = default;
  coordinator(natt Width, natt Height, natt Msaa = {}) : tex(4096, 512), cmr(Width, Height, Msaa) { initialize(); }
  void operator()(const vector& Radians, const color& Background = color::black) {
    static constexpr stv1 hlsl = R"(
#pragma pack_matrix(row_major)
struct SB { float4 p, t; };
struct PSIN {
  float4 p : SV_Position;
  float2 t : TEXCOORD;
};
cbuffer CB : register(b0) { matrix v; };
StructuredBuffer<SB> sb : register(t0);
void vsmain(uint In : SV_VertexID, out PSIN Out) {
  Out.p = mul(v, float4(sb[In].p.xyz, 1));
  Out.t = sb[In].t.xy;
}
Texture2D tx : register(t0);
SamplerState ss : register(s0);
void psmain(PSIN In, out float4 Out : SV_Target) {
  Out = tx.Sample(ss, In.t);
})";
    static constexpr array<list<vector, vector>, 36> vv{
      // clang-format off
      list{vector{1, 1, 1}, vector{0, 0}}, list{vector{1, -1, 1}, vector{0, 1}}, list{vector{1, 1, -1}, vector{0.125f, 0}},              // +X
      list{vector{1, -1, -1}, vector{0.125f, 1}}, list{vector{1, 1, -1}, vector{0.125f, 0}}, list{vector{1, -1, 1}, vector{0, 1}},
      list{vector{-1, 1, -1}, vector{0.125f, 0}}, list{vector{-1, -1, -1}, vector{0.125f, 1}}, list{vector{-1, 1, 1}, vector{0.25f, 0}}, // -X
      list{vector{-1, -1, 1}, vector{0.25f, 1}}, list{vector{-1, 1, 1}, vector{0.25f, 0}}, list{vector{-1, -1, -1}, vector{0.125f, 1}},
      list{vector{1, 1, 1}, vector{0.25f, 0}}, list{vector{1, 1, -1}, vector{0.25f, 1}}, list{vector{-1, 1, 1}, vector{0.375f, 0}},      // +Y
      list{vector{-1, 1, -1}, vector{0.375f, 1}}, list{vector{-1, 1, 1}, vector{0.375f, 0}}, list{vector{1, 1, -1}, vector{0.25f, 1}},
      list{vector{-1, -1, 1}, vector{0.375f, 0}}, list{vector{-1, -1, -1}, vector{0.375f, 1}}, list{vector{1, -1, 1}, vector{0.5f, 0}},  // -Y
      list{vector{1, -1, -1}, vector{0.5f, 1}}, list{vector{1, -1, 1}, vector{0.5f, 0}}, list{vector{-1, -1, -1}, vector{0.375f, 1}},
      list{vector{-1, 1, 1}, vector{0.5f, 0}}, list{vector{-1, -1, 1}, vector{0.5f, 1}}, list{vector{1, 1, 1}, vector{0.625f, 0}},       // +Z
      list{vector{1, -1, 1}, vector{0.625f, 1}}, list{vector{1, 1, 1}, vector{0.625f, 0}}, list{vector{-1, -1, 1}, vector{0.5f, 1}},
      list{vector{1, 1, -1}, vector{0.625f, 0}}, list{vector{1, -1, -1}, vector{0.625f, 1}}, list{vector{-1, 1, -1}, vector{0.75f, 0}},  // -Z
      list{vector{-1, -1, -1}, vector{0.75f, 1}}, list{vector{-1, 1, -1}, vector{0.75f, 0}}, list{vector{1, -1, -1}, vector{0.625f, 1}},
    }; // clang-format on
    static auto rd = renderer<typepack<structured_buffer<list<vector, vector>>>, typepack<xmatrix>, typepack<>, typepack<>, typepack<texture>>(hlsl);
    static auto sb = structured_buffer<list<vector, vector>>(vv.data(), 36);
    static constant_buffer<xmatrix> cb;
    try {
      tiff(tex);
      cmr.rotation = Radians, cmr.update();
      cb.load(cmr.view_projection_matrix);
      cmr.begin_render(Background);
      rd(36, {sb}, {cb}, {}, {}, {tex});
      cmr.end_render();
    } catch (const std::exception& E) { throw except(E); }
  }
  void operator()(const rect& Rect) const { rect{Rect}.draw_bitmap(cmr); }
  // void operator()(const array<const yw::bitmap&, 6>& PlusXYZ_MinusXYZ) const {
  //   tex.begin_draw(color::black);
  //   if (PlusXYZ_MinusXYZ[0]) rect{0, 0, 512, 512}.draw_bitmap(PlusXYZ_MinusXYZ[0]);
  //   if (PlusXYZ_MinusXYZ[1]) rect{1024, 0, 1536, 512}.draw_bitmap(PlusXYZ_MinusXYZ[1]);
  //   if (PlusXYZ_MinusXYZ[2]) rect{2048, 0, 2560, 512}.draw_bitmap(PlusXYZ_MinusXYZ[2]);
  //   if (PlusXYZ_MinusXYZ[3]) rect{512, 0, 1024, 512}.draw_bitmap(PlusXYZ_MinusXYZ[3]);
  //   if (PlusXYZ_MinusXYZ[4]) rect{1536, 0, 2048, 512}.draw_bitmap(PlusXYZ_MinusXYZ[4]);
  //   if (PlusXYZ_MinusXYZ[5]) rect{2560, 0, 3072, 512}.draw_bitmap(PlusXYZ_MinusXYZ[5]);
  //   tex.end_draw();
  // }
};

/// Class for creating a progress bar.
class progressbar {
protected:
  yw::bitmap bm;
  yw::brush<> brush;
  progressbar(const progressbar&) = delete;
  progressbar& operator=(const progressbar&) = delete;
public:
  color background{color::white};
  progressbar() noexcept = default;
  progressbar(progressbar&&) = default;
  progressbar& operator=(progressbar&&) = default;
  explicit operator bool() const noexcept { return bool(bm); }
  operator comptr<ID2D1Bitmap1>::reference*() const noexcept { return bm; }
  progressbar(natt Width, natt Height, const color& Color) : bm(Width, Height), brush(Color) { bm.begin_draw(background), bm.end_draw(); }
  void operator()(fat4 Progress) { bm.begin_draw(background), rect{0, 0, bm.width * Progress, bm.height}.draw_rectangle(brush), bm.end_draw(); }
  void operator()(const color& Color) { brush = yw::brush(Color); }
  void operator()(const rect& Rect) const { Rect.draw_bitmap(bm); }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

namespace main {

/// render target of the main window
inline const bitmap render_target;

/// width of the main window
inline const nat4& width = render_target.width;

/// height of the main window
inline const nat4& height = render_target.height;

/// resizes the main window
void resize(natt Width, natt Height) {
  static constexpr auto getaddressof = []<typename Com>(const comptr<Com>& a) { return const_cast<comptr<Com>&>(a).addressof(); };
  static constexpr cat2 class_name[] = L"ywlib";
  static int4 width_pad{}, height_pad{};
  if (auto& name = const_cast<str2&>(username); name.empty()) // gets the user name
    [&](DWORD s) { ::GetUserNameW(nullptr, &s), name.resize(s), ::GetUserNameW(name.data(), &s); }(0);
  if (!main::hinstance) { // initializes hinstance and window class
    const_cast<HINSTANCE&>(main::hinstance) = GetModuleHandleW(nullptr);
    WNDCLASSEXW wc{sizeof(WNDCLASSEXW), CS_OWNDC, main::wndproc, 0, 0, main::hinstance};
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW), wc.lpszClassName = class_name;
    tiff(RegisterClassExW(&wc));
  }
  if (main::args.empty()) { // gets the command line arguments
    int c;
    auto args = CommandLineToArgvW(GetCommandLineW(), &c);
    auto& a = const_cast<array<str2>&>(main::args);
    for (a.resize(c); 0 <= --c;) a[c] = args[c];
  }
  if (!main::hwnd) { // creates the main window
    const_cast<HWND&>(main::hwnd) = CreateWindowExW(
      WS_EX_ACCEPTFILES, class_name, class_name, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_CLIPCHILDREN,
      0, 0, int4(Width), int4(Height), 0, 0, main::hinstance, 0);
    [&](RECT r = {}) { GetClientRect(main::hwnd, &r), width_pad = width - r.right, height_pad = height - r.bottom; }();
    tiff(SetWindowPos(main::hwnd, 0, 0, 0, width + width_pad, height + height_pad, SWP_NOZORDER));
  }
  if (!main::d3d_device) { // initializes DirectX
    comptr<IDXGIFactory2> factory{};
    tiff(CreateDXGIFactory1(IID_PPV_ARGS(factory.addressof())));
    tiff(factory->MakeWindowAssociation(main::hwnd, DXGI_MWA_NO_ALT_ENTER));
    D3D_FEATURE_LEVEL featurelevels[] = {D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0};
    tiff(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT,
                           featurelevels, (UINT)extent<decltype(featurelevels)>, D3D11_SDK_VERSION,
                           (ID3D11Device**)getaddressof(main::d3d_device), nullptr, (ID3D11DeviceContext**)getaddressof(main::d3d_context)));
    DXGI_SWAP_CHAIN_DESC1 sc_desc{width, height, DXGI_FORMAT_R8G8B8A8_UNORM, false, {1, 0}, DXGI_USAGE_RENDER_TARGET_OUTPUT, 2};
    sc_desc.Scaling = DXGI_SCALING_STRETCH, sc_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    tiff(factory->CreateSwapChainForHwnd(main::d3d_device, main::hwnd, &sc_desc, nullptr, nullptr, getaddressof(main::swap_chain)));
    tiff(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, getaddressof(main::d2d_factory)));
    [&](comptr<IDXGIDevice2> dxgi_device) {
      tiff(main::d3d_device.as(dxgi_device));
      tiff(main::d2d_factory->CreateDevice(dxgi_device, getaddressof(main::d2d_device))); }({});
    tiff(main::d2d_device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, getaddressof(main::d2d_context)));
    tiff(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)getaddressof(main::dw_factory)));
    tiff(CoCreateInstance(CLSID_WICImagingFactory2, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(getaddressof(main::wic_factory))));
    [&](comptr<ID3D11BlendState> state, D3D11_BLEND_DESC desc) {
      desc.RenderTarget[0] = {true, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_OP_ADD,
                                D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_OP_ADD, 0x0f};
      tiff(main::d3d_device->CreateBlendState(&desc, state.addressof()));
      main::d3d_context->OMSetBlendState(state, nullptr, 0xffffffff); }({}, {});
    [&](comptr<ID3D11SamplerState> state, D3D11_SAMPLER_DESC desc) {
      desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP, desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
      desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP, desc.MaxAnisotropy = 1;
      desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS, desc.MaxLOD = D3D11_FLOAT32_MAX;
      tiff(main::d3d_device->CreateSamplerState(&desc, state.addressof()));
      main::d3d_context->PSSetSamplers(0, 1, &state); }({}, {D3D11_FILTER_MIN_MAG_MIP_LINEAR});
    [&](comptr<ID3D11RasterizerState> state, D3D11_RASTERIZER_DESC desc) {
      desc.DepthClipEnable = true, desc.MultisampleEnable = true, desc.AntialiasedLineEnable = true;
      tiff(main::d3d_device->CreateRasterizerState(&desc, state.addressof()));
      main::d3d_context->RSSetState(state); }({}, {D3D11_FILL_SOLID, D3D11_CULL_NONE, true});
  }
  auto& rt = const_cast<bitmap&>(main::render_target);
  rt = bitmap{}, main::d2d_context->SetTarget(nullptr);
  const_cast<nat4&>(rt.width) = nat4(Width), const_cast<nat4&>(rt.height) = nat4(Height);
  tiff(SetWindowPos(main::hwnd, 0, 0, 0, width + width_pad, height + height_pad, SWP_NOMOVE | SWP_NOZORDER));
  tiff(main::swap_chain->ResizeBuffers(2, width, height, DXGI_FORMAT_UNKNOWN, 0));
  [&](comptr<ID3D11Texture2D> tex, comptr<IDXGISurface> surface) {
    tiff(main::swap_chain->GetBuffer(0, IID_PPV_ARGS(tex.addressof())));
    tiff(tex->QueryInterface(IID_PPV_ARGS(surface.addressof())));
    auto p = D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
                                         D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED));
    tiff(main::d2d_context->CreateBitmapFromDxgiSurface(surface, p, rt.d2d_bitmap.addressof())); }({}, {});
}

/// renames the main window
inline void rename(const str2& Name) { SetWindowTextW(main::hwnd, Name.data()); }

/// terminates the main window
inline void terminate() noexcept {
  if (main::hwnd) DestroyWindow(exchange(const_cast<HWND&>(main::hwnd), nullptr));
}

/// sets the main window to topmost
inline void topmost(bool Flag = true) {
  if (auto style = GetWindowLongW(main::hwnd, GWL_EXSTYLE); Flag) {
    SetWindowPos(main::hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    SetWindowLongW(main::hwnd, GWL_EXSTYLE, style | WS_EX_TOPMOST);
  } else {
    SetWindowPos(main::hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    if (style & WS_EX_TOPMOST) SetWindowLongW(main::hwnd, GWL_EXSTYLE, style & ~WS_EX_TOPMOST);
  }
}

/// takes a screenshot of the main window
inline void screenshot(const path& Path) { main::render_target.to_png(Path); }

/// updates the main window
inline constexpr caster update{
  []() {
    static MSG msg{};
    static rect rc{}, pt{};
    static stopwatch sw{};
    if (!mouse::hover) {
      rect& r = const_cast<rect&>(main::mouse);
      GetClientRect(main::hwnd, (RECT*)&rc), GetCursorPos((POINT*)&pt);
      r = [&](const xrect& a) { return xvpermute<0, 1, 4, 5>(a, xvsub(a, r)); }(xvsub(pt, rc));
    }
    const_cast<fat8&>(main::spf) = sw.push(), const_cast<fat8&>(main::fps) = 1.0f / main::spf;
    main::swap_chain->Present(1, 0);
    while (main::hwnd) {
      if (PeekMessageW(&msg, 0, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) return false;
        TranslateMessage(&msg), DispatchMessageW(&msg);
      } else return true;
    }
    return false;
  }};

/// begins drawing to main-window.
void begin_draw() { main::render_target.begin_draw(); }

/// begins drawing to main-window after filling with the specified color.
void begin_draw(const color& Fill) { main::render_target.begin_draw(Fill); }

/// ends drawing to main-window.
void end_draw() { main::render_target.end_draw(); }

/// window procedure for the main window
inline LRESULT WINAPI wndproc(HWND hw, UINT msg, WPARAM wp, LPARAM lp) {
  static constexpr auto key_down = [](key::key& k) { if (const_cast<fat8&>(k.pushed_time) = main::timer.read(), k.down) k.down(); };
  static constexpr auto key_up = [](key::key& k) { fat8& t = const_cast<fat8&>(k.pushed_time); t == 0 ? void() : (k.up ? k.up(main::timer.read() - exchange(t, 0.0)) : void(t = 0.0)); };
  static TRACKMOUSEEVENT tme{.cbSize = sizeof(TRACKMOUSEEVENT), .dwFlags = TME_LEAVE};
  if (userproc && userproc(hw, msg, wp, lp)) return 0;
  switch (msg) {
  case WM_MOUSEMOVE: {
    if (!main::hover) TrackMouseEvent(&tme), const_cast<bool&>(main::hover) = true;
    const_cast<rect&>(main::mouse) = [&](const xrect& a) { return xvpermute<0, 1, 4, 5>(a, xvsub(a, main::mouse)); }(_mm_cvtepi16_epi32(_mm_loadu_si128((xrect*)&lp)));
    return (yw::mouse::moved ? yw::mouse::moved(mouse.right, mouse.bottom) : void(0)), 0;
  }
  case WM_MOUSEWHEEL: return (mouse::wheeled ? mouse::wheeled(int2((wp & 0xffff0000) >> 16)) : void(0)), 0;
  case WM_MOUSELEAVE:
    for (auto& k : key::keys) const_cast<fat8&>(k.pushed_time) = 0.0;
    return (const_cast<bool&>(main::hover) = false), 0;
  case WM_LBUTTONDOWN: return SetFocus(hw), key_down(key::keys[VK_LBUTTON]), 0;
  case WM_RBUTTONDOWN: return SetFocus(hw), key_down(key::keys[VK_RBUTTON]), 0;
  case WM_MBUTTONDOWN: return SetFocus(hw), key_down(key::keys[VK_MBUTTON]), 0;
  case WM_KEYDOWN: return SetFocus(hw), key_down(key::keys[wp]), 0;
  case WM_LBUTTONUP: return key_up(key::keys[VK_LBUTTON]), 0;
  case WM_RBUTTONUP: return key_up(key::keys[VK_RBUTTON]), 0;
  case WM_MBUTTONUP: return key_up(key::keys[VK_MBUTTON]), 0;
  case WM_KEYUP: return key_up(key::keys[wp]), 0;
  case WM_DROPFILES: {
    if (!main::dropped) return 0;
    auto hdrop = (HDROP)wp;
    const natt count = DragQueryFileW(hdrop, 0xFFFFFFFF, nullptr, 0);
    array<path> out(count);
    for (nat4 i{}; i < count; ++i) {
      str2 buffer(DragQueryFileW(hdrop, i, nullptr, 0), L'\0');
      DragQueryFileW(hdrop, i, buffer.data(), (nat4)buffer.size() + 1), out[i] = path(buffer);
    }
    return main::dropped(mv(out)), 0;
  }
  case WM_CLOSE: return DestroyWindow(hw), 0;
  case WM_DESTROY: return PostQuitMessage(0), 0;
  case WM_CREATE: tme.hwndTrack = hw, TrackMouseEvent(&tme);
  }
  return DefWindowProcW(hw, msg, wp, lp);
}
}
}

#endif
