/// \file core/core.hpp

#pragma once

#ifndef YWLIB
#include <compare>
#else
import std;
#endif


#include "constant.hpp"
#include "none.hpp"


#define ywlib_wrapper(...) noexcept(noexcept(__VA_ARGS__)) \
  requires requires { __VA_ARGS__; } { return __VA_ARGS__; }


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

using nat = nat8;
inline constexpr nat npos = nat(-1);
inline constexpr po_t unordered = std::partial_ordering::unordered;

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


/// checks if the type is the same as all of `Ts`
template<typename T, typename... Ts> concept same_as =
  requires { requires (std::same_as<T, Ts> && ...); };

/// checks if the type is the same as any of `Ts`
template<typename T, typename... Ts> concept included_in =
  requires { requires (std::same_as<T, Ts> || ...); };

/// checks if the type is derived from all of `Ts`
template<typename T, typename... Ts> concept derived_from =
  requires { requires (std::derived_from<T, Ts> && ...); };

/// checks if the type can be explicitly converted to all of `Ts`
template<typename T, typename... Ts> concept castable_to =
  requires(T (&f)()) { ((static_cast<Ts>(f())), ...); };

/// checks if the type can be explicitly converted to all of `Ts` without throwing exceptions
template<typename T, typename... Ts> concept nt_castable_to =
  castable_to<T, Ts...> &&
  requires(T (&f)() noexcept) { { ((static_cast<Ts>(f())), ...) } noexcept; };

/// checks if the type can be implicitly converted to all of `Ts`
template<typename T, typename... Ts> concept convertible_to =
  requires { requires (std::convertible_to<T, Ts> && ...); };

/// checks if the type can be implicitly converted to all of `Ts` without throwing exceptions
template<typename T, typename... Ts> concept nt_convertible_to =
  convertible_to<T, Ts...> &&
  requires { requires (std::is_nothrow_convertible_v<T, Ts> && ...); };

/// checks if the type is a numeric type
template<typename T> concept numeric =
  nt_castable_to<T, int4, int8, nat4, nat8, fat4, fat8>;


/// checks if the type has `const` qualifier
template<typename T> concept is_const = std::is_const_v<T>;

/// checks if the type has `volatile` qualifier
template<typename T> concept is_volatile = std::is_volatile_v<T>;

/// checks if the type has `const volatile` qualifier
template<typename T> concept is_cv = is_const<T> && is_volatile<T>;

/// checks if the type is a pointer type
template<typename T> concept is_pointer = std::is_pointer_v<T>;

/// checks if the type is a lvalue reference type
template<typename T> concept is_lvref = std::is_lvalue_reference_v<T>;

/// checks if the type is a rvalue reference type
template<typename T> concept is_rvref = std::is_rvalue_reference_v<T>;

/// checks if the type is a reference type
template<typename T> concept is_reference = is_lvref<T> || is_rvref<T>;

/// checks if the type is a bounded array type
template<typename T> concept is_bounded_array = std::is_bounded_array_v<T>;

/// checks if the type is an unbounded array type
template<typename T> concept is_unbounded_array = std::is_unbounded_array_v<T>;

/// checks if the type is an array type
template<typename T> concept is_array =
  is_bounded_array<T> || is_unbounded_array<T>;

/// checks if the type is a function type
template<typename T> concept is_function = std::is_function_v<T>;


/// checks if the type is valid
template<typename T> concept valid =
  requires { typename std::type_identity<T>::type; };


} ////////////////////////////////////////////////////////////////////////////// namespace yw


namespace yw::_ {

template<typename T> struct _type { using type = T; };

// remove_all_pointers

template<typename T> struct _remove_all_pointers : _type<T> {};
template<is_pointer T> struct _remove_all_pointers<T> : _remove_all_pointers<std::remove_pointer<T>> {};

// member_type

template<typename T> struct _member_type;
template<typename M, typename C> struct _member_type<M C::*> { using m = M; using c = C; };

// remove_cv

template<typename T> struct _add_cv { using cv = const volatile T; using c = const T; using v = volatile T; };
template<typename T> struct _add_cv<T&> { using cv = const volatile T&; using c = const T&; using v = volatile T&; };
template<typename T> struct _add_cv<T&&> { using cv = const volatile T&&; using c = const T&&; using v = volatile T&&; };

// add_pointer

template<typename T> struct _add_pointer : _type<T> {};
template<typename T> requires valid<T*> struct _add_pointer<T> : _type<T*> {};
template<typename T> requires valid<T*> struct _add_pointer<T&> : _type<T*&> {};
template<typename T> requires valid<T*> struct _add_pointer<T&&> : _type<T*&&> {};

// add_extent

template<typename T, nat N> struct _add_extent : _type<T> {};
template<typename T, nat N> requires valid<T[N]> struct _add_extent<T, N> : _type<T[N]> {};
template<typename T, nat N> requires valid<T[N]> struct _add_extent<T&, N> : _type<T(&)[N]> {};
template<typename T, nat N> requires valid<T[N]> struct _add_extent<T&&, N> : _type<T(&&)[N]> {};
template<typename T> requires valid<T[]> struct _add_extent<T, 0> : _type<T[]> {};
template<typename T> requires valid<T[]> struct _add_extent<T&, 0> : _type<T(&)[]> {};
template<typename T> requires valid<T[]> struct _add_extent<T&&, 0> : _type<T(&&)[]> {};

// select_type

template<typename T, T I, typename... Ts> struct _select_type : _select_type<nat, I, Ts...> {};

template<bool B, typename Tt, typename Tf> struct _select_type<bool, B, Tt, Tf> : std::conditional<B, Tt, Tf> {};

template<nat I, typename T, typename... Ts> struct _select_type<nat, I, T, Ts...> : _select_type<nat, I - 1, Ts...> {};

template<typename T, typename... Ts> struct _select_type<nat, 0, T, Ts...> { using type = T; };

// common_type

template<typename... Ts> struct _common_type : _type<none> {};
template<typename... Ts> requires valid<std::common_reference_t<Ts...>>
struct _common_type<Ts...> : std::common_reference<Ts...> {};

// specialization_of

template<typename T, template<typename...> typename Tm>
struct _specialization_of : constant<false> {};

template<template<typename...> typename Tm, typename... Ts>
struct _specialization_of<Tm<Ts...>, Tm> : constant<true> {};

// variation_of

template<typename T, typename U> struct _variation_of : constant<false> {};

template<template<typename...> typename Tm, typename... Ts, typename... Us>
struct _variation_of<Tm<Ts...>, Tm<Us...>> : constant<true> {};

template<template<auto...> typename Tm, auto... Vs, auto... Ws>
struct _variation_of<Tm<Vs...>, Tm<Ws...>> : constant<true> {};

template<template<typename, auto...> typename Tm, typename T, typename U, auto... Vs, auto... Ws>
struct _variation_of<Tm<T, Vs...>, Tm<U, Ws...>> : constant<true> {};

template<template<auto, typename...> typename Tm, auto V, auto W, typename... Ts, typename... Us>
struct _variation_of<Tm<V, Ts...>, Tm<W, Us...>> : constant<true> {};

} ////////////////////////////////////////////////////////////////////////////// namespace yw::_


export namespace yw {


/// removes `const` and `volatile` qualifiers
template<typename T> using remove_cv = std::remove_cv_t<T>;

/// removes `const` qualifier
template<typename T> using remove_const = std::remove_const_t<T>;

/// removes `volatile` qualifier
template<typename T> using remove_volatile = std::remove_volatile_t<T>;

/// removes reference
template<typename T> using remove_ref = std::remove_reference_t<T>;

/// removes `const` and `volatile` qualifiers and reference
template<typename T> using remove_cvref = std::remove_cvref_t<T>;

/// removes the topmost pointer
template<typename T> using remove_pointer = std::remove_pointer_t<T>;

/// removes all pointers
template<typename T> using remove_all_pointers =
  typename _::_remove_all_pointers<T>::type;

/// removes the topmost extent
template<typename T> using remove_extent = std::remove_extent_t<T>;

/// removes all extents
template<typename T> using remove_all_extents = std::remove_all_extents_t<T>;


/// checks if the type is a pointer to member
template<typename T> concept is_member_pointer =
  is_pointer<T> && std::is_member_pointer_v<T>;

/// member type of the pointer to member
template<is_member_pointer Mp> using member_type =
  typename _::_member_type<remove_cv<Mp>>::m;

/// class type of the pointer to member
template<is_member_pointer Mp> using class_type =
  typename _::_member_type<remove_cv<Mp>>::c;

/// checks if the type is a member function pointer
template<typename T> concept is_member_function_pointer =
  is_member_pointer<T> && is_function<member_type<T>>;

/// checks if the type is a member object pointer
template<typename T> concept is_member_object_pointer =
  is_member_pointer<T> && !is_member_function_pointer<T>;


/// checks if the type is a enum
template<typename T> concept is_enum = std::is_enum_v<T>;

/// checks if the type is a class
template<typename T> concept is_class = std::is_class_v<T>;

/// checks if the type is a union
template<typename T> concept is_union = std::is_union_v<T>;

/// checks if the type is an abstract class
template<typename T> concept is_abstract = is_class<T> && std::is_abstract_v<T>;

/// checks if the type is an aggregate
template<typename T> concept is_aggregate =
  is_class<T> && std::is_aggregate_v<T>;

/// checks if the type is an empty class
template<typename T> concept is_empty = is_class<T> && std::is_empty_v<T>;

/// checks if the type is a final class
template<typename T> concept is_final = is_class<T> && std::is_final_v<T>;

/// checks if the type is a polymorphic class
template<typename T> concept is_polymorphic =
  is_class<T> && std::is_polymorphic_v<T>;

/// checks if the type has a virtual destructor
template<typename T> concept has_virtual_destructor =
  is_class<T> && std::has_virtual_destructor_v<T>;

/// checks if the type is a standard layout
template<typename T> concept is_standard_layout = std::is_standard_layout_v<T>;

/// checks if the type is a trivially copyable
template<typename T> concept trivial = std::is_trivially_copyable_v<T>;


/// checks if the type is `void`
template<typename T> concept is_void = same_as<remove_cv<T>, void>;

/// checks if the type is `bool`
template<typename T> concept is_bool = same_as<remove_cv<T>, bool>;

/// checks if the type is a character type
template<typename T> concept character =
  included_in<remove_cv<T>, cat1, cat2, uct1, uct2, uct4>;

/// checks if the type is a integer type
template<typename T> concept integral = std::integral<T>;

/// checks if the type is a floating-point type
template<typename T> concept floating_point = std::floating_point<T>;

/// checks if the type is a arithmetic type
template<typename T> concept arithmetic = integral<T> || floating_point<T>;


/// adds `const` and `volatile` qualifiers through reference
template<typename T> using add_cv = typename _::_add_cv<T>::cv;

/// adds `const` qualifier through reference
template<typename T> using add_const = typename _::_add_cv<T>::c;

/// adds `volatile` qualifier through reference
template<typename T> using add_volatile = typename _::_add_cv<T>::v;

/// makes the type lvalue reference
template<typename T> using add_lvref = std::add_lvalue_reference_t<T>;

/// makes the type rvalue reference
template<typename T> using add_rvref =
  std::add_rvalue_reference_t<remove_ref<T>>;

/// makes the type foward reference
template<typename T> using add_fwref = std::add_rvalue_reference_t<T>;

/// adds a pointer to the type
template<typename T> using add_pointer = typename _::_add_pointer<T>::type;

/// adds an extent to the type
template<typename T, nat N> using add_extent =
  typename _::_add_extent<T, N>::type;


/// checks if the value can be used for selection
template<convertible_to<nat> auto I, nat N>
inline constexpr bool selectable =
  (same_as<decltype(I), bool> && N == 2) ||
  (!same_as<decltype(I), bool> && nat(I) < N);

/// selects a type from the list
template<convertible_to<nat> auto I, typename... Ts>
requires selectable<I, sizeof...(Ts)>
using select_type = typename _::_select_type<decltype(I), I, Ts...>::type;

/// selects a constant from the list
template<convertible_to<nat> auto I, auto... Vs>
requires selectable<I, sizeof...(Vs)>
inline constexpr auto select_value = select_type<I, constant<Vs>...>::value;

/// selects a parameter from the arguments
template<convertible_to<nat> auto I> inline constexpr auto select =
[]<typename T, typename... Ts>(T&& Arg, Ts&&... Args)
  noexcept -> decltype(auto) requires selectable<I, sizeof...(Ts) + 1> {
  if constexpr (same_as<decltype(I), nat>) {
    if constexpr (I == 0) return fwd<T>(Arg);
    else return select<I - 1>(fwd<Ts>(Args)...);
  } else if constexpr (same_as<decltype(I), bool>) {
    return select<nat(!I)>(fwd<T>(Arg), fwd<Ts>(Args)...);
  } else return select<nat(I)>(fwd<T>(Arg), fwd<Ts>(Args)...);
};


/// common type of the types
template<typename... Ts> using common_type =
  typename _::_common_type<Ts...>::type;


/// performs compile-time evaluation
inline consteval auto cev(auto a) noexcept { return a; }

/// returns the size of the array
inline consteval auto arraysize(const is_bounded_array auto& Array)
  noexcept { return std::extent_v<remove_ref<decltype(Array)>>; }

/// `declval` function
template<typename T>
inline constexpr auto declval = []() noexcept -> add_fwref<T> {};


/// obtains the address of the reference
inline constexpr auto addressof =
[]<is_lvref T>(T&& Ref) noexcept { return __builtin_addressof(Ref); };

/// returns the constant reference
inline constexpr auto asconst =
[]<typename T>(T&& Ref) noexcept -> add_const<T&&> { return Ref; };

/// returns the rvalue reference
inline constexpr auto mv = []<typename T>(T&& Ref) noexcept
  -> decltype(auto) { return static_cast<add_rvref<T>>(Ref); };

/// fowards the reference
template<typename T> inline constexpr auto fwd =
[](auto&& Ref) noexcept -> decltype(auto) { return static_cast<T&&>(Ref); };


/// checks if the type is destructible
template<typename T> concept destructible =
  requires { requires std::is_destructible_v<T>; };

/// chekcs if the type is nothrow destructible
template<typename T> concept nt_destructible =
  destructible<T> && std::destructible<T>;

/// checks if the type can be assigned from the type `A`
template<typename T, typename A> concept assignable =
  requires { requires std::is_assignable_v<T, A>; };

/// checks if the type can be assigned from the type `A` without throwing exceptions
template<typename T, typename A> concept nt_assignable =
  assignable<T, A> && requires { requires std::is_nothrow_assignable_v<T, A>; };

/// checks if the type can be constructed from the arguments `As...`
template<typename T, typename... As> concept constructible =
  requires { T{declval<As>()...}; };

/// checks if the type can be constructed from the arguments `As...` without throwing exceptions
template<typename T, typename... As> concept nt_constructible =
  constructible<T, As...> && requires { { T{declval<As>()...} } noexcept; };

/// checks if the type can be exchanged with the type `U`
template<typename T, typename U = T> concept exchangeable =
  constructible<remove_cvref<T>, add_rvref<T>> && assignable<T, U>;

/// checks if the type can be exchanged with the type `U` without throwing exceptions
template<typename T, typename U = T> concept nt_exchangeable =
  nt_constructible<remove_cvref<T>, add_rvref<T>> && nt_assignable<T, U>;


/// constructs the type from the arguments
template<typename T> inline constexpr auto construct =
[]<typename... Ts>(Ts&&... Args) noexcept(nt_constructible<T, Ts...>)
  -> T requires constructible<T, Ts...> { return T{fwd<Ts>(Args)...}; };

/// performs the assignment
inline constexpr auto assign =
[]<typename L, typename R>(L&& Lhs, R&& Rhs)
  noexcept(nt_assignable<L, R>) -> decltype(auto)
  requires assignable<L, R> { return fwd<L>(Lhs) = fwd<R>(Rhs); };

/// performs the exchange
inline constexpr auto exchange =
[]<typename T, typename U = T>(T& Ref, U&& Value)
  noexcept(nt_exchangeable<T&, U>) -> T requires exchangeable<T&, U>
{ auto a = mv(Ref); Ref = fwd<U>(Value); return a; };


/// decays the type
template<typename T> using decay = std::decay_t<T>;

/// performs `decay-copy` operation
inline constexpr auto decay_copy = []<typename T>(T&& Ref)
   noexcept(nt_convertible_to<T, decay<T>>) -> decay<T> { return fwd<T>(Ref); };


/// function object which can be overloaded
template<typename... Fs>
struct overload : public Fs... { using Fs::operator()...; };

/// function object to pass the argument as it is
struct pass {
  constexpr auto operator()(auto&& Arg) const noexcept
    -> decltype(auto) { return static_cast<decltype(Arg)>(Arg); }
};


/// returns the number of `true`
template<bool... Bs> inline constexpr nat counts = (nat(0) + ... + Bs);

/// returns the index of the first `true`
template<bool... Bs> inline constexpr nat inspects = 0;
template<bool B, bool... Bs> inline constexpr nat inspects<B, Bs...> = B ? 0 : 1 + inspects<Bs...>;


/// checks if `invoke(F, As...)` is well-formed
template<typename F, typename... As>
concept invocable = std::invocable<F, As...>;

/// chekcs if `invoke(F, As...)` is well-formed and noexcept
template<typename F, typename... As> concept nt_invocable =
  invocable<F, As...> &&
  requires { requires std::is_nothrow_invocable_v<F, As...>; };

/// result type of `invoke(F, As...)`
template<typename F, typename... As> using invoke_result =
  select_type<is_void<std::invoke_result_t<F, As...>>,
              none, std::invoke_result_t<F, As...>>;

/// checks if `invoke_r<R>(F, As...)` is well-formed
template<typename F, typename R, typename... As> concept invocable_r =
  invocable<F, As...> && convertible_to<invoke_result<F, As...>, R>;

/// checks if `invoke_r<R>(F, As...)` is well-formed and noexcept
template<typename F, typename R, typename... As> concept nt_invocable_r =
  nt_invocable<F, As...> && nt_convertible_to<invoke_result<F, As...>, R>;

/// checks if `F` is a predicate
template<typename F, typename... As>
concept predicate = invocable_r<F, bool, As...>;

/// performs `invoke(F, As...)`
/// \param Func the first parameter
/// \param Args the rest parameters
/// \return `std::invoke(Func, Args...)` if the return type is not `void`, otherwise `none{}`
inline constexpr auto invoke = []<typename F, typename... As>(
  F&& Func, As&&... Args) noexcept(nt_invocable<F, As...>)
  -> invoke_result<F, As...> requires invocable<F, As...> {
  if constexpr (is_void<std::invoke_result_t<F, As...>>)
    return std::invoke(fwd<F>(Func), fwd<As>(Args)...), none{};
  else return std::invoke(fwd<F>(Func), fwd<As>(Args)...);
};

/// performs `invoke_r<R>(F, As...)`
/// \tparam R the return type
/// \param Func the first parameter
/// \param Args the rest parameters
/// \return `static_cast<R>(invoke(Func, Args...))`
template<typename R> inline constexpr auto invoke_r =
[]<typename F, typename... As>(F&& f, As&&... as)
  noexcept(nt_invocable_r<F, R, As...>) -> R requires invocable_r<F, R, As...> {
  return static_cast<R>(invoke(fwd<F>(f), fwd<As>(as)...));
};


/// checks if the type is a specialization of the template
template<typename T, template<typename...> typename Tm>
concept specialization_of = _::_specialization_of<T, Tm>::value;

/// checks if the type is a variation of `U`
template<typename T, typename U>
concept variation_of = _::_variation_of<T, U>::value;


/// function object to compare the values
struct equal {
  template<typename T, typename U>
  constexpr bool operator()(T&& Lhs, U&& Rhs) const
    noexcept(noexcept(bool(Lhs == Rhs)))
    requires requires { { Lhs == Rhs } -> convertible_to<bool>; }
  { return Lhs == Rhs; }
};

/// function object to compare the values
struct not_equal {
  template<typename T, typename U>
  constexpr bool operator()(T&& Lhs, U&& Rhs) const
    noexcept(noexcept(bool(Lhs != Rhs)))
    requires requires { { Lhs != Rhs } -> convertible_to<bool>; }
  { return Lhs != Rhs; }
};

/// function object to compare the values
struct less {
  template<typename T, typename U>
  constexpr bool operator()(T&& Lhs, U&& Rhs) const
    noexcept(noexcept(bool(Lhs < Rhs)))
    requires requires { { Lhs < Rhs } -> convertible_to<bool>; }
  { return Lhs < Rhs; }
};

/// function object to compare the values
struct less_equal {
  template<typename T, typename U>
  constexpr bool operator()(T&& Lhs, U&& Rhs) const
    noexcept(noexcept(bool(Lhs <= Rhs)))
    requires requires { { Lhs <= Rhs } -> convertible_to<bool>; }
  { return Lhs <= Rhs; }
};

/// function object to compare the values
struct greater {
  template<typename T, typename U>
  constexpr bool operator()(T&& Lhs, U&& Rhs) const
    noexcept(noexcept(bool(Lhs > Rhs)))
    requires requires { { Lhs > Rhs } -> convertible_to<bool>; }
  { return Lhs > Rhs; }
};

/// function object to compare the values
struct greater_equal {
  template<typename T, typename U>
  constexpr bool operator()(T&& Lhs, U&& Rhs) const
    noexcept(noexcept(bool(Lhs >= Rhs)))
    requires requires { { Lhs >= Rhs } -> convertible_to<bool>; }
  { return Lhs >= Rhs; }
};


inline constexpr equal eq;
inline constexpr not_equal ne;
inline constexpr less lt;
inline constexpr less_equal le;
inline constexpr greater gt;
inline constexpr greater_equal ge;

inline constexpr auto tw = []<typename T, typename U>(T&& Lhs, U&& Rhs)
  noexcept(noexcept(Lhs <=> Rhs)) requires std::three_way_comparable_with<T, U>
{ return Lhs <=> Rhs; };


template<trivial T> inline constexpr auto bitcast =
[](const trivial auto& Ref) noexcept
  requires (sizeof(T) == sizeof(Ref)) { return __builtin_bit_cast(T, Ref); };


template<typename... Fs> struct caster : public Fs... {
private:
  template<typename T> static constexpr nat i =
    inspects<same_as<T, invoke_result<Fs>>...>;
  template<typename T> static constexpr nat j =
    i<T> < sizeof...(Fs) ? i<T> : inspects<invocable_r<T, Fs>...>;
  template<typename... As> static constexpr nat k =
    inspects<invocable<Fs, As...>...>;
public:

  using Fs::operator()...;

  template<typename T> requires (j<T> < sizeof...(Fs))
  constexpr operator T() const
    noexcept(nt_convertible_to<invoke_result<select_type<j<T>, Fs...>>, T>)
    requires convertible_to<invoke_result<select_type<j<T>, Fs...>>, T>
  { return select_type<j<T>, Fs...>::operator()(); }

  template<typename... As> requires (k<As...> < sizeof...(Fs))
  constexpr auto operator()(As&&... Args) const
    noexcept(nt_invocable<select_type<k<As...>, Fs...>, As...>)
    requires invocable<select_type<k<As...>, Fs...>, As...>
  { return select_type<k<As...>, Fs...>::operator()(fwd<As>(Args)...); }
};


/// checks if the code is in the constant evaluation
/// \note Used as the condition of `if`.
inline constexpr caster is_cev{
[]() noexcept { return std::is_constant_evaluated(); }
};

/// checks if the code is not in the constant evaluation
/// \note Put this anywhere in the code, and if constant evaluation is detected, the program will not compile.
constexpr void cannot_be_constant_evaluated(auto&&...) {
  if (is_cev) throw "This code cannot be constant evaluated.";
}


} ////////////////////////////////////////////////////////////////////////////// namespace yw


namespace yw::_ {

// copy_reference

template<typename T, typename U>
struct _copy_reference : _type<remove_ref<T>> {};

template<typename T, typename U>
struct _copy_reference<T, U&> : _type<add_lvref<T>> {};

template<typename T, typename U>
struct _copy_reference<T, U&&> : _type<add_rvref<T>> {};

// copy_extent

template<typename T, typename Src> struct _copy_extent : _type<T> {};

template<typename T, is_array U>
struct _copy_extent<T, U> : _type<add_extent<T, std::extent_v<U>>> {};

} ////////////////////////////////////////////////////////////////////////////// namespace yw::_


export namespace yw {


///
template<typename T, typename Src> using copy_const =
  select_type<is_const<Src>, add_const<T>, T>;

///
template<typename T, typename Src> using copy_volatile =
  select_type<is_volatile<Src>, add_volatile<T>, T>;

///
template<typename T, typename Src> using copy_cv =
  copy_const<copy_volatile<T, Src>, Src>;

///
template<typename T, typename Src> using copy_reference =
  typename _::_copy_reference<T, Src>::type;

///
template<typename T, typename Src> using copy_cvref =
  copy_reference<copy_cv<T, Src>, Src>;

///
template<typename T, typename Src> using copy_extent =
  typename _::_copy_extent<T, Src>::type;

///
template<typename T, typename Src> using copy_pointer =
  select_type<is_pointer<Src>, copy_cv<add_pointer<T>, Src>, T>;


} ////////////////////////////////////////////////////////////////////////////// namespace yw
