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
template<typename T, is_pointer U> struct _copy_pointer<T, U> : trait<copy_pointer<add_pointer<T>, U>> {};
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
template<typename F, typename... As> using result_t =
  type_switch<is_void<std::invoke_result_t<F, As...>>, none, std::invoke_result_t<F, As...>>;

/// checks if `F` is invocable with `As` and the result is convertible to `R`
template<typename F, typename R, typename... As> concept invocable_r =
  invocable<F, As...> && convertible_to<result_t<F, As...>, R>;

/// checks if `F` is nothrow invocable with `As` and the result is nothrow convertible to `R`
template<typename F, typename R, typename... As> concept nt_invocable_r =
  nt_invocable<F, As...> && nt_convertible_to<result_t<F, As...>, R>;

/// checks if `F` is a predicate; i.e. the result of invoking `F` with `As` is convertible to `bool`
template<typename F, typename... As> concept predicate = invocable_r<F, bool, As...>;

// clang-format off

/// invokes `Func` with `Args`
inline constexpr auto invoke = []<typename F, typename... As>(F&& Func, As&&... Args)
  noexcept(nt_invocable<F, As...>) -> result_t<F, As...> requires invocable<F, As...> {
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// clang-format off

/// generic value type that can be used for non-type template parameters
struct value {
  fat8 _cpp_double{};

  /// default constructor
  constexpr value() noexcept = default;

  /// constructs from `none`; sets the value to NaN
  explicit constexpr value(none) noexcept : _cpp_double(bitcast<fat8>(0x7ff8000000000001)) {}

  /// constructs from a value that can be casted to `fat8`
  template<castable_to<fat8> T> constexpr value(T&& V) noexcept(nt_castable_to<T, fat8>)
    : _cpp_double(static_cast<fat8>(fwd<T>(V))) {}

  /// assigns `none` to the value; sets the value to NaN
  constexpr value& operator=(none) noexcept { return _cpp_double = bitcast<fat8>(0x7ff8000000000001), *this; }

  /// assigns a value that can be casted to `fat8`
  template<castable_to<fat8> T> constexpr value& operator=(T&& V)
    noexcept { return _cpp_double = static_cast<fat8>(fwd<T>(V)), *this; }

  /// `value` can be casted to any type that can be casted from `fat8`
  template<typename T> requires castable_to<fat8, T>
  constexpr operator T() const noexcept { return static_cast<T>(_cpp_double); }

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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

/// checks if `iter_value_t<It>` is convertible to `Uy`
template<typename It, typename Uy> concept iterator_for = iterator<It> && convertible_to<iter_reference_t<It>, Uy>;

/// checks if `iter_value_t<It>` is the same as `Uy`
template<typename It, typename Uy> concept iterator_of = iterator_for<It, Uy> && same_as<iter_value_t<It>, Uy>;

/// checks if `iter_value_t<Rg>` is convertible to `Uy`
template<typename Rg, typename Uy> concept range_for = iterator_for<iterator_t<Rg>, Uy>;

/// checks if `iter_value_t<Rg>` is the same as `Uy`
template<typename Rg, typename Uy> concept range_of = iterator_of<iterator_t<Rg>, Uy>;

/// checks if `cnt_range<Rg> && range_of<Rg, Uy>`
template<typename Rg, typename Uy> concept cnt_range_of = cnt_range<Rg> && range_of<Rg, Uy>;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// clang-format off

/// static array class if `N > 0`; otherwise, dynamic array class
template<typename T, natt N = 0> class array {
public:
  using value_type = T;
  T _cpp_array[N]{};

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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
  ywlib_wrapper(_::_get::call<I>(static_cast<T&&>(Tuple)));

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
template<natt I, natt N, typename S, typename... T> struct _comb_indices;
template<natt I, natt N, bool... Bs, natt... Is> struct _comb_indices<I, N, sequence<Bs...>, sequence<Is...>>
  : _comb_indices<I + 1, N, sequence<Bs...>, type_switch<value_switch<I, Bs...>, sequence<Is..., I>, sequence<Is...>>> {};
template<natt N, bool... Bs, natt... Is> struct _comb_indices<N, N, sequence<Bs...>, sequence<Is...>> : trait<sequence<Is...>> {};
template<typename T, typename S> struct _to_typepack;
template<typename T, natt... Is> struct _to_typepack<T, sequence<Is...>> : trait<typepack<element_t<T, Is>...>> {};
} // clang-format on

/// gets the extent of `Tp`
template<typename Tp> inline constexpr natt extent = _::_extent<Tp>;

/// checks if `Tp` is a tuple; i.e. has a non-zero extent
template<typename Tp> concept tuple = extent<Tp> != 0;

/// checks if `Tps` are tuples with the same extent
template<typename... Tps> concept same_size_tuples = requires { requires(tuple<Tps> && ...); requires((extent<Tps> == extent<type_switch<0, Tps...>>) && ...); };
template<typename Tp, typename U> concept tuple_for = requires { requires tuple<Tp>; requires _::_tuple_for<Tp, U, make_indices_for<Tp>>::value; };
template<typename Tp, typename U> concept nt_tuple_for = requires { requires tuple_for<Tp, U>; requires _::_nt_tuple_for<Tp, U, make_indices_for<Tp>>::value; };

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
  template<natt I, sequence_of S> requires(I < size) using insert = typename fore<I>::template append<S>::template append<back<size - I>>;
  template<template<auto...> typename Tm> using expand = Tm<Vs...>;
  template<natt I> requires(I < size) constexpr const auto&& get() const noexcept { return mv(at<I>); }
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

// clang-format off

template<typename... Ts> struct list : typepack<Ts...>::template fore<sizeof...(Ts) - 1>::template expand<list> {
  using base = typepack<Ts...>::template fore<sizeof...(Ts) - 1>::template expand<list>;
  static constexpr natt size = sizeof...(Ts);
  using last_type = type_switch<size - 1, Ts...>;
  last_type last;
  template<natt I> constexpr auto get() & noexcept -> type_switch<I, Ts...>& { if constexpr (I == size - 1) return last; else return base::template get<I>(); }
  template<natt I> constexpr auto get() const& noexcept -> add_const<type_switch<I, Ts...>&> { if constexpr (I == size - 1) return last; else return base::template get<I>(); }
  template<natt I> constexpr auto get() && noexcept -> type_switch<I, Ts...>&& { if constexpr (I == size - 1) return fwd<type_switch<I, Ts...>>(last); else return mv(*this).base::template get<I>(); }
  template<natt I> constexpr auto get() const&& noexcept -> add_const<type_switch<I, Ts...>&&> { if constexpr (I == size - 1) return fwd<add_const<type_switch<I, Ts...>>>(last); else return mv(*this).base::template get<I>(); }
};

template<typename T1, typename T2, typename T3> struct list<T1, T2, T3> : list<T1, T2> {
  using base = list<T1, T2>;
  static constexpr natt size = 3;
  using third_type = T3;
  third_type third;
  template<natt I> constexpr auto get() & noexcept -> type_switch<I, T1, T2, T3>& { if constexpr (I == 2) return third; else return base::template get<I>(); }
  template<natt I> constexpr auto get() const& noexcept -> add_const<type_switch<I, T1, T2, T3>&> { if constexpr (I == 2) return third; else return base::template get<I>(); }
  template<natt I> constexpr auto get() && noexcept -> type_switch<I, T1, T2, T3>&& { if constexpr (I == 2) return fwd<type_switch<I, T1, T2, T3>>(third); else return mv(*this).base::template get<I>(); }
  template<natt I> constexpr auto get() const&& noexcept -> add_const<type_switch<I, T1, T2, T3>&&> { if constexpr (I == 2) return fwd<add_const<type_switch<I, T1, T2, T3>>>(third); else return mv(*this).base::template get<I>(); }
};

template<typename T1, typename T2> struct list<T1, T2> : public list<T1> {
  using base = list<T1>;
  static constexpr natt size = 2;
  using second_type = T2;
  second_type second;
  template<natt I> constexpr auto get() & noexcept -> type_switch<I, T1, T2>& { if constexpr (I == 1) return second; else return base::template get<I>(); }
  template<natt I> constexpr auto get() const& noexcept -> add_const<type_switch<I, T1, T2>&> { if constexpr (I == 1) return second; else return base::template get<I>(); }
  template<natt I> constexpr auto get() && noexcept -> type_switch<I, T1, T2>&& { if constexpr (I == 1) return fwd<type_switch<I, T1, T2>>(second); else return mv(*this).base::template get<I>(); }
  template<natt I> constexpr auto get() const&& noexcept -> add_const<type_switch<I, T1, T2>&&> { if constexpr (I == 1) return fwd<add_const<type_switch<I, T1, T2>>>(second); else return mv(*this).base::template get<I>(); }
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

template<> struct list<> {
private:
  template<typename T, typename U, natt... Is, natt... Js> static constexpr auto _concat(T&& t, U&& u, sequence<Is...>, sequence<Js...>) ywlib_wrapper((yw::list{get<Is>(fwd<T>(t))..., get<Js>(fwd<U>(u))...}));
  template<typename T, typename U, typename V> struct _from_typepack {};
  template<typename... Ts, typename U, template<typename...> typename Tm, typename... Vs> struct _from_typepack<typepack<Ts...>, U, Tm<Vs...>> : trait<list<copy_cvref<Tm<Ts>, U>...>> {};
  template<typename... Ts, typename U, typename V> struct _from_typepack<typepack<Ts...>, U, V> : trait<list<copy_cvref<Ts, U>...>> {};
public:
  static constexpr natt size = 0;
  template<tuple T, tuple U> static constexpr auto concat(T&& Fore, U&& Back) ywlib_wrapper(_concat(fwd<T>(Fore), fwd<U>(Back), make_indices_for<T>{}, make_indices_for<U>{}));
  template<typename... Ts> static constexpr auto asref(Ts&&... Args) noexcept { return list<Ts&&...>{fwd<Ts>(Args)...}; }
  template<specialization_of<typepack> T, typename Template = none> using from_typepack = _from_typepack<T, Template, remove_cvref<Template>>::type;
};

template<typename... Ts> list(Ts...) -> list<Ts...>;

struct t_apply {
  template<natt... Is, typename F, typename T> constexpr auto operator()(sequence<Is...>, F&& f, T&& t) const ywlib_wrapper(invoke(fwd<F>(f), get<Is>(fwd<T>(t))...));
  template<typename S, typename F, typename T> constexpr auto operator()(S, F&& f, T&& t) const ywlib_wrapper((*this)(to_sequence<T, natt>{}, fwd<F>(f), fwd<T>(t)));
  template<typename F, typename T> constexpr auto operator()(F&& f, T&& t) const ywlib_wrapper((*this)(make_indices_for<T>{}, fwd<F>(f), fwd<T>(t)));
};

inline constexpr t_apply apply;
template<typename Fn, typename Tp, typename S = make_indices_for<Tp>> concept applyable = requires { requires tuple<Tp>; requires indices_for<S, Tp>; apply(S{}, declval<Fn>(), declval<Tp>()); };
template<typename Fn, typename Tp, typename S = make_indices_for<Tp>> concept nt_applyable = requires { requires applyable<Fn, Tp, S>; { apply(S{}, declval<Fn>(), declval<Tp>()) } noexcept; };
template<typename Fn, typename Tp, typename S = make_indices_for<Tp>> requires applyable<Fn, Tp, S> using apply_result = decltype(apply(S{}, declval<Fn>(), declval<Tp>()));

struct t_vapply {
private:
  template<natt I, typename Fn, typename... Ts> static constexpr auto call(constant<I>, Fn& f, Ts&&... ts)
    noexcept((nt_gettable<Ts, I> && ...) && nt_invocable<Fn&, element_t<Ts, I>...>) -> decltype(auto) { return invoke(f, get<I>(fwd<Ts>(ts)...)); }
public:
  template<natt... Is, typename Fn, tuple... Ts> constexpr auto operator()(sequence<Is...>, Fn&& f, Ts&&... ts) const noexcept((noexcept(call(constant<Is>{}, f, fwd<Ts>(ts)...)) && ...))
    requires requires { ((call(constant<Is>{}, f, fwd<Ts>(ts)...)), ...); } { return list<>::asref(call(constant<Is>{}, f, fwd<Ts>(ts)...)...); }
  template<sequence_of<natt> S, typename Fn, tuple... Ts> constexpr auto operator()(S, Fn&& f, Ts&&... ts) const ywlib_wrapper((*this)(to_sequence<S, natt>{}, f, fwd<Ts>(ts)...));
  template<typename Fn, tuple T, tuple... Ts> constexpr auto operator()(Fn&& f, T&& t, Ts&&... ts) const ywlib_wrapper((*this)(make_indices_for<T>{}, f, fwd<T>(t), fwd<Ts>(ts)...));
};

inline constexpr t_vapply vapply;
template<typename Fn, typename... Ts> concept vapplyable = requires { requires(tuple<Ts> && ...); vapply(declval<Fn>(), declval<Ts>()...); };
template<typename Fn, typename... Ts> concept nt_vapplyable = requires { requires vapplyable<Fn, Ts...>; { vapply(declval<Fn>(), declval<Ts>()...) } noexcept; };
template<typename Fn, typename... Ts> requires vapplyable<Fn, Ts...> using vapply_reresult = decltype(vapply(declval<Fn>(), declval<Ts>()...));

struct t_vassign {
  template<natt... Is, typename T, typename U> constexpr T&& operator()(sequence<Is...>, T&& t, U&& u) const noexcept(((nt_gettable<T, Is> && nt_gettable<U, Is...>) && ...) && (nt_assignable<element_t<T, Is>, element_t<U, Is>> && ...))
    requires (gettable<T, Is> && ...) && (gettable<U, Is> && ...) && (assignable<element_t<T, Is>, element_t<U, Is>> && ...) { return ((get<Is>(fwd<T>(t)) = get<Is>(fwd<U>(u))), ...), fwd<T>(t); }
  template<typename S, typename T, typename U> constexpr auto operator()(S, T&& t, U&& u) const ywlib_wrapper((*this)(to_sequence<S, natt>{}, fwd<T>(t), fwd<U>(u)));
  template<typename T, typename U> constexpr auto operator()(T&& t, U&& u) const ywlib_wrapper((*this)(make_indices_for<T>{}, fwd<T>(t), fwd<U>(u)));
};

inline constexpr t_vassign vassign;
template<typename T, typename U, typename S = make_indices_for<T>> concept vassignable = requires { requires tuple<T>; requires tuple<U>; requires indices_for<S, T>; requires indices_for<S, U>; vassign(S{}, declval<T>(), declval<U>()); };
template<typename T, typename U, typename S = make_indices_for<T>> concept nt_vassignable = requires { requires vassignable<T, U, S>; { vassign(S{}, declval<T>(), declval<U>()) } noexcept; };

template<typename T> struct t_build {
  template<natt... Is, typename U> constexpr T operator()(sequence<Is...>, U&& t) const noexcept((nt_gettable<U, Is> && ...) && nt_constructible<T, element_t<U, Is>...>)
    requires requires { requires(gettable<U, Is> && ...); requires constructible<T, element_t<U, Is>...>; } { return T{get<Is>(fwd<U>(t))...}; }
  template<typename S, typename U> constexpr auto operator()(S, U&& t) const ywlib_wrapper((*this)(to_sequence<S, natt>{}, fwd<U>(t)));
  template<tuple U> constexpr auto operator()(U&& t) const ywlib_wrapper((*this)(make_indices_for<U>{}, fwd<U>(t)));
};

template<typename T> inline constexpr t_build<T> build;
template<typename T, typename Tp, typename S = make_indices_for<Tp>> concept buildable = requires { requires tuple<Tp>; requires indices_for<S, Tp>; build<T>(S{}, declval<Tp>()); };
template<typename T, typename Tp, typename S = make_indices_for<Tp>> concept nt_buildable = requires { requires buildable<T, Tp, S>; { build<T>(S{}, declval<Tp>()) } noexcept; };

// clang-format on

template<typename T, sequence_of<natt> Sq = make_indices_for<T>, typename Fn = none> struct projector;

template<typename T, sequence_of<natt> Sq> requires(!tuple<T>) struct projector<T, Sq, none> {
  using indices_type = to_sequence<Sq, natt>;
  static constexpr natt size = indices_type::size;
  static_assert(size != 0);
  add_fwref<T> ref;
  constexpr projector(add_fwref<T> Ref, Sq) noexcept : ref(Ref) {}
  template<natt I> requires(I < size) constexpr auto get() const ywlib_wrapper(ref);
};

template<typename T, sequence_of<natt> Sq, invocable<T> Fn> requires(!tuple<T>) struct projector<T, Sq, Fn> {
  using indices_type = to_sequence<Sq, natt>;
  static constexpr natt size = indices_type::size;
  static_assert(size != 0);
  add_fwref<T> ref;
  Fn proj;
  constexpr projector(add_fwref<T> Ref, Sq, Fn Proj) noexcept : ref(Ref), proj(mv(Proj)) {}
  template<natt I> requires(I < size) constexpr auto get() const ywlib_wrapper(invoke(proj, ref));
};

template<tuple Tp, sequence_of<natt> Sq> struct projector<Tp, Sq, none> {
  using indices_type = to_sequence<Sq, natt>;
  static constexpr natt size = indices_type::size;
  static_assert(size != 0);
  add_fwref<Tp> ref;
  constexpr projector(add_fwref<Tp> Ref) noexcept : ref(Ref) {}
  constexpr projector(add_fwref<Tp> Ref, Sq) noexcept : ref(Ref) {}
  template<natt I> requires(I < size) constexpr auto get() const ywlib_wrapper(yw::get<indices_type::template at<I>>(ref));
};

template<tuple Tp, sequence_of<natt> Sq, vapplyable<Tp, Sq> Fn> struct projector<Tp, Sq, Fn> {
  using indices_type = to_sequence<Sq, natt>;
  static constexpr natt size = indices_type::size;
  static_assert(size != 0);
  add_fwref<Tp> ref;
  Fn proj;
  constexpr projector(add_fwref<Tp> Ref, Fn Proj) noexcept : ref(Ref), proj(mv(Proj)) {}
  constexpr projector(add_fwref<Tp> Ref, Sq, Fn Proj) noexcept : ref(Ref), proj(mv(Proj)) {}
  template<natt I> requires(I < size) constexpr auto get() const ywlib_wrapper(invoke(proj, yw::get<indices_type::template at<I>>(ref)));
};

template<typename T, sequence_of<natt> Sq> requires(!tuple<T>) projector(T&&, Sq) -> projector<T, Sq, none>;
template<typename T, sequence_of<natt> Sq, invocable<T> Fn> requires(!tuple<T>) projector(T&&, Sq, Fn) -> projector<T, Sq, Fn>;
template<tuple Tp> projector(Tp&&) -> projector<Tp, make_indices_for<Tp>, none>;
template<tuple Tp, sequence_of<natt> Sq> projector(Tp&&, Sq) -> projector<Tp, Sq, none>;
template<tuple Tp, vapplyable<Tp, make_indices_for<Tp>> Fn> projector(Tp&&, Fn) -> projector<Tp, make_indices_for<Tp>, Fn>;
template<tuple Tp, sequence_of<natt> Sq, vapplyable<Tp, Sq> Fn> projector(Tp&&, Sq, Fn) -> projector<Tp, Sq, Fn>;

template<character Ct> using string_view = std::basic_string_view<Ct>;
using stv1 = string_view<cat1>;
using stv2 = string_view<cat2>;

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
  constexpr bool operator==(const string_view<Ct>& s) const noexcept { return std::ranges::equal(begin(), end(), s.begin(), s.end()); }
  constexpr auto operator<=>(const string_view<Ct>& s) const noexcept { return std::lexicographical_compare_three_way(begin(), end(), s.begin(), s.end()); }
  template<typename Tr> friend std::basic_ostream<Ct, Tr>& operator<<(std::basic_ostream<Ct, Tr>& os, const string& s) { return os << s.cpp_array; }
};

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
  template<iterator_of<Ct> It, sentinel_for<It> Se> requires(!same_as<It, Se>) constexpr string(It i, Se s)
    : std::basic_string<Ct>(std::common_iterator<It, Se>(mv(i)), std::common_iterator<It, Se>(mv(s))) {}
  template<range_of<Ct> Rg> constexpr string(Rg&& r) : string(yw::begin(r), yw::end(r)) {}
};
using str1 = string<cat1>;
using str2 = string<cat2>;

template<character Ct, natt N> string(const Ct (&)[N]) -> string<Ct, N>;
template<character Ct, natt N> string(const Ct (&&)[N]) -> string<Ct, N>;
template<character Ct> requires(!is_const<Ct>) string(Ct*) -> string<Ct, 0>;
template<character Ct> string(natt, Ct) -> string<Ct, 0>;
template<range Rg> string(Rg&&) -> string<iter_value_t<Rg>, 0>;
template<iterator It, sentinel_for<It> Se> string(It, Se) -> string<iter_value_t<It>, 0>;

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

template<included_in<cat1, cat2> Ct> inline constexpr auto vtos = []<arithmetic T>(const T v) noexcept {
  if constexpr (same_as<Ct, cat1>) return std::to_string(v);
  else return std::to_wstring(v);
};

struct source {
  nat4 line, column;
  stv1 file, func;
  constexpr source(nat4 Line = __builtin_LINE(), nat4 Column = __builtin_COLUMN(), stv1 File = __builtin_FILE(), stv1 Func = __builtin_FUNCTION()) noexcept : line(Line), column(Column), file(mv(File)), func(mv(Func)) {}
  template<typename Tr> friend std::basic_ostream<cat1, Tr>& operator<<(std::basic_ostream<cat1, Tr>& OS, const source& S) { return OS << std::format("file={},func={},line={},column={}", S.file, S.func, S.line, S.column); }
};

class except : public std::exception {
public:
  explicit except(const std::string& s, source _ = {}) noexcept : except(s.data(), mv(_)){};
  explicit except(const cat1* s, source _ = {}) noexcept : std::exception(std::format("{}->{}\n", s, _).data()) {}
  explicit except(const std::exception& Base, source _ = {}) noexcept : except(Base.what(), mv(_)){};
};

class path : public std::filesystem::path {
public:
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
  template<range Rg> requires character<iter_value_t<Rg>> path(Rg&& r) : std::filesystem::path(yw::begin(r), yw::end(r)) {}
  template<character Ct> path(const Ct* s) : std::filesystem::path(s) {}
  template<range Rg> requires character<iter_value_t<Rg>> path& operator=(Rg&& r) { return *this = path(fwd<Rg>(r)); }
  template<character Ct> path& operator=(const Ct* s) { return *this = path(s); }
};

template<invocable... Fs> struct caster : public Fs... {
  template<typename T> static constexpr natt i = []<typename... Ts>(typepack<Ts...>) { return inspects<same_as<Ts, T>...>; }(typepack<result_t<Fs>...>{});
  template<typename T> static constexpr natt j = i<T> < sizeof...(Fs) ? i<T> : []<typename... Ts>(typepack<Ts...>) { return inspects<convertible_to<Ts, T>...>; }(typepack<result_t<Fs>...>{});
public:
  template<typename T> requires(j<T> < sizeof...(Fs)) constexpr operator T() const noexcept(nt_convertible_to<result_t<type_switch<j<T>, Fs...>>, T>) { return type_switch<j<T>, Fs...>::operator()(); }
};

inline constexpr caster is_cev{[]() noexcept { return std::is_constant_evaluated(); }};

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
template<typename C, size_t N> struct formatter<yw::string<C, N>, C> : formatter<basic_string_view<C>, C> {
  auto format(const yw::string<C, N>& s, yw::type_switch<same_as<C, wchar_t>, wformat_context, format_context>& ctx) const {
    return formatter<basic_string_view<C>, C>::format(basic_string_view<C>{s.cpp_array}, ctx); } };
template<> struct formatter<yw::source> : formatter<string> {
  auto format(const yw::source& s, format_context& ctx) const {
    return formatter<string>::format(std::format("source(file={},func={},line={},column={})", s.file, s.func, s.line, s.column), ctx); } };
template<yw::iterator It, yw::iter_unary_invocable<It> Pj> struct incrementable_traits<yw::projector<It, yw::sequence<>, Pj>> { using difference_type = yw::iter_difference_t<It>; };
} // clang-format on

namespace yw::file {

inline constexpr auto listup = [](const path& Directory, bool Recursive) -> array<path> {
  namespace fs = std::filesystem;
  array<path> a;
  if (Recursive)
    for (const fs::directory_entry& e : fs::recursive_directory_iterator(Directory)) a.emplace_back(e.path());
  else
    for (const fs::directory_entry& e : fs::directory_iterator(Directory)) a.emplace_back(e.path());
  return a;
};

inline constexpr auto exists = [](const path& p) -> bool { return std::filesystem::exists(p); };
inline constexpr auto is_file = [](const path& p) -> bool { return std::filesystem::is_regular_file(p); };
inline constexpr auto is_directory = [](const path& p) -> bool { return std::filesystem::is_directory(p); };
inline constexpr auto size = [](const path& p) -> natt { return std::filesystem::file_size(p); };

inline constexpr auto read =
  []<cnt_range Rg>(const path& p, Rg& Out)
    requires requires { Out.clear(), Out.resize(natt{}); } {
      if (!exists(p)) return Out.clear();
      std::ifstream ifs(p, std::ios_base::binary);
      auto n = size(p);
      Out.resize((n - 1) / sizeof(iter_value_t<Rg>) + 1);
      ifs.read(reinterpret_cast<cat1*>(yw::data(Out)), n);
    };

inline constexpr overload write{
  [](const path& p, const void* a, natt n) {
    if (std::ofstream ofs(p, std::ios_base::binary); ofs) ofs.write(reinterpret_cast<const cat1*>(a), n);
  },
  []<cnt_iterator It, sized_sentinel_for<It> Se>(const path& p, It i, Se s) { write(p, &(*i), s - i); },
  []<cnt_range Rg>(const path& p, Rg&& a) { write(p, data(a), yw::size(a)); }};

inline constexpr auto remove = [](const path& Path) { return std::filesystem::remove(Path); };
}
