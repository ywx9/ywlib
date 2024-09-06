/// \file ywlib-core.hpp

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
#define ywlib_dummy {
#define ywlib_namespace_std_begin } namespace std {
#define ywlib_namespace_std_end } export namespace yw {
#define ywlib_namespace_internal_begin } namespace yw::_ {
#define ywlib_namespace_internal_end } export namespace yw {
#undef  ywlib_dummy
#define ywlib_dummy }

static_assert(sizeof(int*) == 8, "ywlib requires 64-bit environment.");

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

using byte = std::byte;

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

inline constexpr auto npos = nat(-1);
inline constexpr auto unordered = std::partial_ordering::unordered;

/// checks if the type is valid
template<typename T> concept valid = requires { typename std::type_identity<T>::type; };

/// checks if the type is the same as all of `Ts`
template<typename T, typename... Ts> concept same_as = requires { requires (std::same_as<T, Ts> && ...); };

/// checks if the type is the same as any of `Ts`
template<typename T, typename... Ts> concept included_in = requires { requires (std::same_as<T, Ts> || ...); };

/// checks if the type is derived from all of `Ts`
template<typename T, typename... Ts> concept derived_from = requires { requires (std::derived_from<T, Ts> && ...); };

/// checks if the type can be explicitly converted to all of `Ts`
template<typename T, typename... Ts> concept castable_to = requires(T (&f)()) { ((static_cast<Ts>(f())), ...); };

/// checks if the type can be explicitly converted to all of `Ts` without throwing exceptions
template<typename T, typename... Ts> concept nt_castable_to =
  castable_to<T, Ts...> && requires(T (&f)() noexcept) { { ((static_cast<Ts>(f())), ...) } noexcept; };

/// checks if the type can be implicitly converted to all of `Ts`
template<typename T, typename... Ts> concept convertible_to =
  requires { requires (std::convertible_to<T, Ts> && ...); };

/// checks if the type can be implicitly converted to all of `Ts` without throwing exceptions
template<typename T, typename... Ts> concept nt_convertible_to =
  convertible_to<T, Ts...> && requires { requires (std::is_nothrow_convertible_v<T, Ts> && ...); };

/// checks if `T` can be explicitly converted to a value
template<typename T> concept numeric = nt_castable_to<T, int4, int8, nat4, nat8, fat4, fat8>;

/// struct to represent a constant value
template<auto V, typename T = decltype(V)>
requires convertible_to<decltype(V), T>
struct constant {
  using type = T;
  static constexpr type value = V;
  consteval operator type() const noexcept { return value; }
  consteval type operator()() const noexcept { return value; }
};

/// struct to represent a null value
struct none {
  constexpr none() noexcept = default;
  constexpr none(auto&&...) noexcept {}
  constexpr none& operator=(auto&&) noexcept { return *this; }
  explicit constexpr operator bool() const noexcept { return false; }
  constexpr none operator()(auto&&...) const noexcept { return {}; }
  constexpr friend bool operator==(none, none) noexcept { return false; }
  constexpr friend auto operator<=>(none, none) noexcept { return unordered; }
  constexpr friend none operator+(none) noexcept { return {}; }
  constexpr friend none operator-(none) noexcept { return {}; }
  constexpr friend none operator+(none, none) noexcept { return {}; }
  constexpr friend none operator-(none, none) noexcept { return {}; }
  constexpr friend none operator*(none, none) noexcept { return {}; }
  constexpr friend none operator/(none, none) noexcept { return {}; }
  constexpr none& operator+=(none) noexcept { return *this; }
  constexpr none& operator-=(none) noexcept { return *this; }
  constexpr none& operator*=(none) noexcept { return *this; }
  constexpr none& operator/=(none) noexcept { return *this; }
};

ywlib_namespace_std_begin
template<typename T> struct common_type<T, yw::none> : type_identity<yw::none> {};
template<typename T> struct common_type<yw::none, T> : type_identity<yw::none> {};
template<typename Ct> struct formatter<yw::none, Ct> : formatter<const Ct*, Ct> {
  static constexpr const Ct text[] = {'n', 'o', 'n', 'e', '\0'};
  auto format(yw::none, auto& ctx) const { return formatter<const Ct*, Ct>::format(text, ctx); }
};
ywlib_namespace_std_end

/// struct to represent a generic value
struct value {

  /// internal value
  fat8 _{};

  /// default constructor; `0.0`
  constexpr value() noexcept = default;

  /// constructor from `none`; `nan`
  constexpr value(none) noexcept : _(std::numeric_limits<fat8>::quiet_NaN()) {}

  /// constructor from `numeric` value; `fat8(Value)`
  template<numeric T> requires(!same_as<T, value>)
  constexpr value(T&& Value) noexcept : _(fat8(static_cast<T&&>(Value))) {}

  constexpr operator fat8&() & noexcept { return _; }
  constexpr operator const fat8&() const& noexcept { return _; }
  constexpr operator fat8&&() && noexcept { return static_cast<fat8&&>(_); }
  constexpr operator const fat8&&() const&& noexcept { return static_cast<const fat8&&>(_); }

  /// conversion to a type which can be casted from `fat8`
  template<typename T> requires castable_to<fat8, T>
  explicit constexpr operator T() const
    noexcept(nt_castable_to<fat8, T>) { return T(_); }

  /// unary plus operator
  constexpr value operator+() const noexcept { return *this; }

  /// unary minus operator
  constexpr value operator-() const noexcept { return -_; }

  /// addition assignment operator
  template<numeric T> constexpr value& operator+=(T&& Value)
    noexcept { return _ += double(static_cast<T&&>(Value)), *this; }

  /// subtraction assignment operator
  template<numeric T> constexpr value& operator-=(T&& Value)
    noexcept { return _ -= double(static_cast<T&&>(Value)), *this; }

  /// multiplication assignment operator
  template<numeric T> constexpr value& operator*=(T&& Value)
    noexcept { return _ *= double(static_cast<T&&>(Value)), *this; }

  /// division assignment operator
  template<numeric T> constexpr value& operator/=(T&& Value)
    noexcept { return _ /= double(static_cast<T&&>(Value)), *this; }
};

/// euler's number
inline constexpr value e = std::numbers::e_v<fat8>;

/// golden ratio
inline constexpr value phi = std::numbers::phi_v<fat8>;

/// pi
inline constexpr value pi = std::numbers::pi_v<fat8>;

/// gamma constant
inline constexpr value gamma = std::numbers::egamma_v<fat8>;

/// ln(2)
inline constexpr value ln2 = std::numbers::ln2_v<fat8>;

/// ln(10)
inline constexpr value ln10 = std::numbers::ln10_v<fat8>;

/// log2(10)
inline constexpr value log2_10 = 3.3219280948873623478703194294894;

/// log10(2)
inline constexpr value log10_2 = 0.30102999566398119521373889472449;

/// sqrt(2)
inline constexpr value sqrt2 = std::numbers::sqrt2_v<fat8>;

/// sqrt(3)
inline constexpr value sqrt3 = std::numbers::sqrt3_v<fat8>;

/// infinity
inline constexpr value inf = std::numeric_limits<fat8>::infinity();

/// quiet NaN
inline constexpr value nan = std::numeric_limits<fat8>::quiet_NaN();

ywlib_namespace_std_begin
template<typename T> struct common_type<T, yw::value> : common_type<T, fat8> {};
template<typename T> struct common_type<yw::value, T> : common_type<fat8, T> {};
template<typename Ct> struct formatter<yw::value, Ct> : formatter<fat8, Ct> {};
ywlib_namespace_std_end

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
template<typename T> concept is_array = is_bounded_array<T> || is_unbounded_array<T>;

/// checks if the type is a function type
template<typename T> concept is_function = std::is_function_v<T>;

ywlib_namespace_internal_begin
template<typename T> struct _type { using type = T; };
template<typename T> struct _remove_all_pointers : _type<T> {};
template<is_pointer T> struct _remove_all_pointers<T> : _remove_all_pointers<remove_pointer<T>> {};
ywlib_namespace_internal_end

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
template<typename T> using remove_all_pointers = typename _::_remove_all_pointers<T>::type;

/// removes the topmost extent
template<typename T> using remove_extent = std::remove_extent_t<T>;

/// removes all extents
template<typename T> using remove_all_extents = std::remove_all_extents_t<T>;

ywlib_namespace_internal_begin
template<typename T> struct _member_type;
template<typename M, typename C> struct _member_type<M C::*> { using m = M; using c = C; };
ywlib_namespace_internal_end

/// checks if the type is a pointer to member
template<typename T> concept is_member_pointer = is_pointer<T> && std::is_member_pointer_v<T>;

/// member type of the pointer to member
template<is_member_pointer Mp> using member_type = typename _::_member_type<remove_cv<Mp>>::m;

/// class type of the pointer to member
template<is_member_pointer Mp> using class_type = typename _::_member_type<remove_cv<Mp>>::c;

/// checks if the type is a member function pointer
template<typename T> concept is_member_function_pointer = is_member_pointer<T> && is_function<member_type<T>>;

/// checks if the type is a member object pointer
template<typename T> concept is_member_object_pointer = is_member_pointer<T> && !is_member_function_pointer<T>;

/// checks if the type is a enum
template<typename T> concept is_enum = std::is_enum_v<T>;

/// checks if the type is a class
template<typename T> concept is_class = std::is_class_v<T>;

/// checks if the type is a union
template<typename T> concept is_union = std::is_union_v<T>;

/// checks if the type is an abstract class
template<typename T> concept is_abstract = is_class<T> && std::is_abstract_v<T>;

/// checks if the type is an aggregate
template<typename T> concept is_aggregate = is_class<T> && std::is_aggregate_v<T>;

/// checks if the type is an empty class
template<typename T> concept is_empty = is_class<T> && std::is_empty_v<T>;

/// checks if the type is a final class
template<typename T> concept is_final = is_class<T> && std::is_final_v<T>;

/// checks if the type is a polymorphic class
template<typename T> concept is_polymorphic = is_class<T> && std::is_polymorphic_v<T>;

/// checks if the type has a virtual destructor
template<typename T> concept has_virtual_destructor = is_class<T> && std::has_virtual_destructor_v<T>;

/// checks if the type is a standard layout
template<typename T> concept is_standard_layout = std::is_standard_layout_v<T>;

/// checks if the type is a trivially copyable
template<typename T> concept trivial = std::is_trivially_copyable_v<T>;

/// checks if the type is `void`
template<typename T> concept is_void = same_as<remove_cv<T>, void>;

/// checks if the type is `bool`
template<typename T> concept is_bool = same_as<remove_cv<T>, bool>;

/// checks if the type is a character type
template<typename T> concept character = included_in<remove_cv<T>, cat1, cat2, uct1, uct2, uct4>;

/// checks if the type is a integer type
template<typename T> concept integral = std::integral<T>;

/// checks if the type is a floating-point type
template<typename T> concept floating_point = std::floating_point<T>;

/// checks if the type is a arithmetic type
template<typename T> concept arithmetic = integral<T> || floating_point<T>;

ywlib_namespace_internal_begin
template<typename T> struct _add_cv { using cv = const volatile T; using c = const T; using v = volatile T; };
template<typename T> struct _add_cv<T&> { using cv = const volatile T&; using c = const T&; using v = volatile T&; };
template<typename T> struct _add_cv<T&&> { using cv = const volatile T&&; using c = const T&&; using v = volatile T&&; };
template<typename T> struct _add_pointer : _type<T> {};
template<typename T> requires valid<T*> struct _add_pointer<T> : _type<T*> {};
template<typename T> requires valid<T*> struct _add_pointer<T&> : _type<T*&> {};
template<typename T> requires valid<T*> struct _add_pointer<T&&> : _type<T*&&> {};
template<typename T, nat N> struct _add_extent : _type<T> {};
template<typename T, nat N> requires valid<T[N]> struct _add_extent<T, N> : _type<T[N]> {};
template<typename T, nat N> requires valid<T[N]> struct _add_extent<T&, N> : _type<T(&)[N]> {};
template<typename T, nat N> requires valid<T[N]> struct _add_extent<T&&, N> : _type<T(&&)[N]> {};
template<typename T> requires valid<T[]> struct _add_extent<T, 0> : _type<T[]> {};
template<typename T> requires valid<T[]> struct _add_extent<T&, 0> : _type<T(&)[]> {};
template<typename T> requires valid<T[]> struct _add_extent<T&&, 0> : _type<T(&&)[]> {};
ywlib_namespace_internal_end

/// adds `const` and `volatile` qualifiers through reference
template<typename T> using add_cv = typename _::_add_cv<T>::cv;

/// adds `const` qualifier through reference
template<typename T> using add_const = typename _::_add_cv<T>::c;

/// adds `volatile` qualifier through reference
template<typename T> using add_volatile = typename _::_add_cv<T>::v;

/// makes the type lvalue reference
template<typename T> using add_lvref = std::add_lvalue_reference_t<T>;

/// makes the type rvalue reference
template<typename T> using add_rvref = std::add_rvalue_reference_t<remove_ref<T>>;

/// makes the type foward reference
template<typename T> using add_fwref = std::add_rvalue_reference_t<T>;

/// adds a pointer to the type
template<typename T> using add_pointer = typename _::_add_pointer<T>::type;

/// adds an extent to the type
template<typename T, nat N> using add_extent = typename _::_add_extent<T, N>::type;

ywlib_namespace_internal_begin
template<typename T, T I, typename... Ts> struct _select_type : _select_type<nat, I, Ts...> {};
template<bool B, typename Tt, typename Tf> struct _select_type<bool, B, Tt, Tf> : std::conditional<B, Tt, Tf> {};
template<nat I, typename T, typename... Ts> struct _select_type<nat, I, T, Ts...> : _select_type<nat, I - 1, Ts...> {};
template<typename T, typename... Ts> struct _select_type<nat, 0, T, Ts...> : _type<T> {};
ywlib_namespace_internal_end

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

ywlib_namespace_internal_begin
template<typename T, typename U> struct _copy_reference : _type<remove_ref<T>> {};
template<typename T, typename U> struct _copy_reference<T, U&> : _type<add_lvref<T>> {};
template<typename T, typename U> struct _copy_reference<T, U&&> : _type<add_rvref<T>> {};
template<typename T, typename Src> struct _copy_extent : _type<T> {};
template<typename T, is_array U> struct _copy_extent<T, U> : _type<add_extent<T, std::extent_v<U>>> {};
template<typename... Ts> struct _common_type : _type<none> {};
template<typename... Ts> requires valid<std::common_reference_t<Ts...>>
struct _common_type<Ts...> : std::common_reference<Ts...> {};
template<typename T, template<typename...> typename Tm> struct _specialization_of : std::false_type {};
template<template<typename...> typename Tm, typename... Ts>
struct _specialization_of<Tm<Ts...>, Tm> : std::true_type {};
template<typename T, typename U> struct _variation_of : std::false_type` {};
template<template<typename...> typename Tm, typename... Ts, typename... Us>
struct _variation_of<Tm<Ts...>, Tm<Us...>> : std::true_type {};
template<template<auto...> typename Tm, auto... Vs, auto... Ws>
struct _variation_of<Tm<Vs...>, Tm<Ws...>> : std::true_type {};
template<template<typename, auto...> typename Tm, typename T, typename U, auto... Vs, auto... Ws>
struct _variation_of<Tm<T, Vs...>, Tm<U, Ws...>> : std::true_type {};
template<template<auto, typename...> typename Tm, auto V, auto W, typename... Ts, typename... Us>
struct _variation_of<Tm<V, Ts...>, Tm<W, Us...>> : std::true_type {};
ywlib_namespace_internal_end

/// adds `const` qualifier to `T` if `Src` has it
template<typename T, typename Src> using copy_const = select_type<is_const<Src>, add_const<T>, T>;

/// adds `volatile` qualifier to `T` if `Src` has it
template<typename T, typename Src> using copy_volatile = select_type<is_volatile<Src>, add_volatile<T>, T>;

/// adds `const` and/or `volatile` qualifiers to `T` if `Src` has them
template<typename T, typename Src> using copy_cv = copy_const<copy_volatile<T, Src>, Src>;

/// adds reference to `T` that `Src` has; otherwise, removes reference from `T`
template<typename T, typename Src> using copy_reference = typename _::_copy_reference<T, Src>::type;

/// same as `copy_reference<copy_cv<T, Src>, Src>`
template<typename T, typename Src> using copy_cvref = copy_reference<copy_cv<T, Src>, Src>;

/// adds extent to `T` that `Src` has
template<typename T, typename Src> using copy_extent = typename _::_copy_extent<T, Src>::type;

/// adds pointer to `T` that `Src` has
template<typename T, typename Src> using copy_pointer = select_type<is_pointer<Src>, copy_cv<add_pointer<T>, Src>, T>;

/// common type of the types
template<typename... Ts> using common_type = typename _::_common_type<Ts...>::type;

/// checks if the type is a specialization of the template
template<typename T, template<typename...> typename Tm>
concept specialization_of = _::_specialization_of<T, Tm>::value;

/// checks if the type is a variation of `U`
template<typename T, typename U>
concept variation_of = _::_variation_of<T, U>::value;

/// performs compile-time evaluation
inline consteval auto cev(auto a) noexcept { return a; }

/// returns the size of the array
inline consteval auto arraysize(const is_bounded_array auto& Array)
  noexcept { return std::extent_v<remove_ref<decltype(Array)>>; }

/// `declval` function
template<typename T> inline constexpr auto declval = []() noexcept -> add_fwref<T> {};

/// obtains the address of the reference
inline constexpr auto addressof = []<is_lvref T>(T&& Ref) noexcept { return __builtin_addressof(Ref); };

/// returns the constant reference
inline constexpr auto asconst = []<typename T>(T&& Ref) noexcept -> add_const<T&&> { return Ref; };

/// returns the rvalue reference
inline constexpr auto mv = []<typename T>(T&& Ref) noexcept
  -> decltype(auto) { return static_cast<add_rvref<T>>(Ref); };

/// fowards the reference
template<typename T> inline constexpr auto fwd =
[](auto&& Ref) noexcept -> decltype(auto) { return static_cast<T&&>(Ref); };

/// checks if the type is destructible
template<typename T> concept destructible = requires { requires std::is_destructible_v<T>; };

/// chekcs if the type is nothrow destructible
template<typename T> concept nt_destructible = destructible<T> && std::destructible<T>;

/// checks if the type can be assigned from the type `A`
template<typename T, typename A> concept assignable = requires { requires std::is_assignable_v<T, A>; };

/// checks if the type can be assigned from the type `A` without throwing exceptions
template<typename T, typename A> concept nt_assignable =
  assignable<T, A> && requires { requires std::is_nothrow_assignable_v<T, A>; };

/// checks if the type can be constructed from the arguments `As...`
template<typename T, typename... As> concept constructible = requires { T{declval<As>()...}; };

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
template<typename... Fs> struct overload : public Fs... { using Fs::operator()...; };

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
template<typename F, typename... As> concept invocable = std::invocable<F, As...>;

/// chekcs if `invoke(F, As...)` is well-formed and noexcept
template<typename F, typename... As> concept nt_invocable =
  invocable<F, As...> && requires { requires std::is_nothrow_invocable_v<F, As...>; };

/// result type of `invoke(F, As...)`
template<typename F, typename... As> using invoke_result =
  select_type<is_void<std::invoke_result_t<F, As...>>, none, std::invoke_result_t<F, As...>>;

/// checks if `invoke_r<R>(F, As...)` is well-formed
template<typename F, typename R, typename... As> concept invocable_r =
  invocable<F, As...> && convertible_to<invoke_result<F, As...>, R>;

/// checks if `invoke_r<R>(F, As...)` is well-formed and noexcept
template<typename F, typename R, typename... As> concept nt_invocable_r =
  nt_invocable<F, As...> && nt_convertible_to<invoke_result<F, As...>, R>;

/// checks if `F` is a predicate
template<typename F, typename... As> concept predicate = invocable_r<F, bool, As...>;

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

/// function object to compare the values
struct equal {
  template<typename T, typename U>
  constexpr bool operator()(T&& a, U&& b) const noexcept(noexcept(bool(a == b)))
    requires requires { { a == b } -> convertible_to<bool>; } { return a == b; }
};

/// function object to compare the values
struct not_equal {
  template<typename T, typename U>
  constexpr bool operator()(T&& a, U&& b) const noexcept(noexcept(bool(a != b)))
    requires requires { { a != b } -> convertible_to<bool>; } { return a != b; }
};

/// function object to compare the values
struct less {
  template<typename T, typename U>
  constexpr bool operator()(T&& a, U&& b) const noexcept(noexcept(bool(a < b)))
    requires requires { { a < b } -> convertible_to<bool>; } { return a < b; }
};

/// function object to compare the values
struct less_equal {
  template<typename T, typename U>
  constexpr bool operator()(T&& a, U&& b) const noexcept(noexcept(bool(a <= b)))
    requires requires { { a <= b } -> convertible_to<bool>; } { return a <= b; }
};

/// function object to compare the values
struct greater {
  template<typename T, typename U>
  constexpr bool operator()(T&& a, U&& b) const noexcept(noexcept(bool(a > b)))
    requires requires { { a > b } -> convertible_to<bool>; } { return a > b; }
};

/// function object to compare the values
struct greater_equal {
  template<typename T, typename U>
  constexpr bool operator()(T&& a, U&& b) const noexcept(noexcept(bool(a >= b)))
    requires requires { { a >= b } -> convertible_to<bool>; } { return a >= b; }
};

inline constexpr equal eq;
inline constexpr not_equal ne;
inline constexpr less lt;
inline constexpr less_equal le;
inline constexpr greater gt;
inline constexpr greater_equal ge;

inline constexpr auto tw =
[]<typename T, typename U>(T&& a, U&& b) noexcept(noexcept(a <=> b))
  requires std::three_way_comparable_with<T, U> { return a <=> b; };

/// bit-casts the value
template<trivial T> inline constexpr auto bitcast =
[](const trivial auto& Ref) noexcept
  requires (sizeof(T) == sizeof(Ref)) { return __builtin_bit_cast(T, Ref); };

/// bit-casts the value to a nat-type
inline constexpr auto natcast =
[](const trivial auto& Ref) noexcept
  requires (sizeof(Ref) <= 8 && std::popcount(sizeof(Ref)) == 1) {
  if constexpr (sizeof(Ref) == 1) return bitcast<nat1>(Ref);
  else if constexpr (sizeof(Ref) == 2) return bitcast<nat2>(Ref);
  else if constexpr (sizeof(Ref) == 4) return bitcast<nat4>(Ref);
  else if constexpr (sizeof(Ref) == 8) return bitcast<nat8>(Ref);
};

/// returns the popcount of the value
constexpr int bitcount(const arithmetic auto v) noexcept { return std::popcount(natcast(v)); }

/// performs bit-floor function
constexpr auto bitfloor(const std::unsigned_integral auto v) noexcept { return std::bit_floor(v); }

/// performs bit-ceil function
constexpr auto bitceil(const std::unsigned_integral auto v) noexcept { return std::bit_ceil(v); }

/// `caster` function object
template<typename... Fs> struct caster : public Fs... {
private:
  static constexpr nat count = sizeof...(Fs);
  template<typename T> static constexpr nat i = inspects<same_as<T, invoke_result<Fs>>...>;
  template<typename T> static constexpr nat j = i<T> < sizeof...(Fs) ? i<T> : inspects<invocable_r<T, Fs>...>;
  template<typename... As> static constexpr nat k = inspects<invocable<Fs, As...>...>;

public:
  using Fs::operator()...;

  template<typename T> constexpr operator T() const
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
inline constexpr caster is_cev{
[]() noexcept { return std::is_constant_evaluated(); }
};

/// checks if the code is not in the constant evaluation
/// \note Put this anywhere in the code, and if constant evaluation is detected, the program will not compile.
constexpr void cannot_be_constant_evaluated(auto&&...) {
  if (is_cev) throw "This code cannot be constant evaluated.";
}
ywlib_namespace_internal_begin
namespace _get {
template<nat I> void get(auto&&) = delete;
template<nat I, typename T> inline constexpr auto pattern = []() -> int {
  if constexpr (is_bounded_array<remove_ref<T>>)
    return I < std::extent_v<remove_ref<T>> ? 16 : 8;
  else if constexpr (requires { get<I>(declval<T>()); })
    return 1 | noexcept(get<I>(declval<T>())) * 16;
  else if constexpr (requires { declval<T>().template get<I>(); })
    return 2 | noexcept(declval<T>().template get<I>()) * 16;
  else return I == 0 ? 20 : 8;
}();
template<nat I, typename T, nat P = pattern<I, T>> requires(P != 8)
constexpr decltype(auto) call(T&& a) noexcept(bool(P & 16)) {
  if constexpr ((P & 7) == 0) return static_cast<T&&>(a)[I];
  else if constexpr ((P & 7) == 1) return get<I>(static_cast<T&&>(a));
  else if constexpr ((P & 7) == 2) return static_cast<T&&>(a).template get<I>();
  else if constexpr ((P & 7) == 4) return static_cast<T&&>(a);
}
template<typename T, nat I = (pattern<0, T> & 15)> struct extent : std::tuple_size<T> {};
template<typename T> struct extent<T, 0> : std::extent<remove_ref<T>> {};
template<typename T> struct extent<T, 4> : constant<1_nn> {};
}
ywlib_namespace_internal_end

/// number of elements in the tuple-like type
template<typename T> inline constexpr nat extent =
  _::_get::extent<remove_ref<T>>::value;

/// checks if the extent of `T` and `Ts...` are equal
template<typename T, typename... Ts> concept same_extent =
  requires { requires ((extent<T> == extent<Ts>) && ...); };

/// checks if the type is tuple-like
template<typename T> concept tuple =
  requires { requires (_::_get::pattern<0, T> & 15) < 3; };

/// checks if `get<I>(T)` is well-formed
template<typename T, nat I> concept gettable =
  requires { _::_get::get<I>(declval<T>()); };

/// checks if `get<I>(T)` is well-formed and noexcept
template<typename T, nat I> concept nt_gettable =
  gettable<T, I> && requires { { _::_get::get<I>(declval<T>()) } noexcept; };

/// result type of `get<I>(T)`
template<typename T, nat I> requires gettable<T, I>
using element_t = decltype(_::_get::get<I>(declval<T>()));

/// obtains the element at index `I` from `T`
template<nat I> inline constexpr auto get =
[]<typename T>(T&& t) noexcept(nt_gettable<T, I>)
  -> element_t<T, I> { return _::_get::get<I>(static_cast<T&&>(t)); };

template<auto... Vs> struct sequence;
template<typename... Ts> struct typepack;
template<typename... Ts> struct list;

ywlib_namespace_internal_begin
template<typename S, typename T> struct _to_sequence;
template<template<auto...> typename Tm, typename T, auto... Vs> requires (std::convertible_to<decltype(Vs), T> && ...) struct _to_sequence<Tm<Vs...>, T> : _type<sequence<T(Vs)...>> {};
template<template<typename, auto...> typename Tm, typename T, typename U, auto... Vs> requires (std::convertible_to<decltype(Vs), T> && ...) struct _to_sequence<Tm<U, Vs...>, T> : _type<sequence<T(Vs)...>> {};
template<template<auto...> typename Tm, auto... Vs> struct _to_sequence<Tm<Vs...>, none> : _type<sequence<Vs...>> {};
template<template<typename, auto...> typename Tm, typename U, auto... Vs> struct _to_sequence<Tm<U, Vs...>, none> : _type<sequence<Vs...>> {};
template<typename S, nat N> struct _indices_for : constant<false> {};
template<nat... Is, nat N> struct _indices_for<sequence<Is...>, N> : constant<(lt(Is, N) && ...)> {};
template<nat I, nat N, auto F, nat...Vs> struct _make_sequence : _make_sequence<I + 1, N, F, Vs..., F(I)> {};
template<nat N, auto F, nat...Vs> struct _make_sequence<N, N, F, Vs...> : _type<sequence<Vs...>> {};
template<nat I, nat N, typename S, nat... Is> struct _extracting_indices;
template<nat I, nat N, bool... Bs, nat... Is> struct _extracting_indices<I, N, sequence<Bs...>, Is...> : select_type<select_value<I, Bs...>, _extracting_indices<I + 1, N, sequence<Bs...>, Is..., I>, _extracting_indices<I + 1, N, sequence<Bs...>, Is...>> {};
template<nat N, bool... Bs, nat... Is> struct _extracting_indices<N, N, sequence<Bs...>, Is...> : _type<sequence<Is...>> {};
template<typename T, typename S> struct _common_element;
template<typename T, nat... Is> struct _common_element<T, sequence<Is...>> : std::common_reference<element_t<T, Is>...> {};
template<typename T, typename U, typename S> struct _tuple_for : constant<false> {};
template<typename T, typename U, nat... Is> struct _tuple_for<T, U, sequence<Is...>> : constant<(std::convertible_to<element_t<T, Is>, U> && ...) + (std::is_nothrow_constructible_v<element_t<T, Is>, U> && ...) * 2> {};
ywlib_namespace_internal_end

/// converts a sequence-like type to `sequence`
template<typename S, typename T = none> using to_sequence = typename _::_to_sequence<S, T>::type;

/// checks if the type is a sequence-like type whose elements are convertible to `T`
template<typename S, typename T = none> concept sequence_of = requires { typename to_sequence<S, T>; };

/// checks if the type is a indices-like type whose elements are less than the extent of `T`
template<typename S, typename T> concept indices_for = requires { requires _::_indices_for<S, extent<T>>::value; };

/// generates a sequence; `{F(Begin), F(Begin + 1), ..., F(End - 1)}`
template<nat Begin, nat End, std::invocable<nat> auto F = pass{}>
requires (Begin < End && !std::is_void_v<decltype(F(Begin))>)
using make_sequence = typename _::_make_sequence<Begin, End, F>::type;

/// generates a indices for the tuple-like type `T`
template<typename T> using make_indices_for = make_sequence<0, extent<T>>;

/// generates a indices which contains the indices of `true` in the boolean sequence
template<sequence_of<bool> S> using extracting_indices = typename _::_extracting_indices<0, extent<S>, S>::type;

/// common type of the elements of the tuple-like type
template<typename T> using common_element = typename _::_common_element<T, make_indices_for<T>>::type;

/// checks if the all elements of the tuple-like type are convertible to `U`
template<typename T, typename U> concept tuple_for =
  requires { requires bool(_::_tuple_for<T, U, make_indices_for<T>>::value & 1); };

/// checks if the all elements of the tuple-like type are convertible to `U` without throwing exceptions
template<typename T, typename U> concept nt_tuple_for =
  tuple_for<T, U> && requires { requires bool(_::_tuple_for<T, U, make_indices_for<T>>::value & 2); };

ywlib_namespace_internal_begin
template<typename S, typename T> struct _sequence_append : _sequence_append<to_sequence<S>, to_sequence<T>> {};
template<auto... Vs, auto... Ws> struct _sequence_append<sequence<Vs...>, sequence<Ws...>> : _type<sequence<Vs..., Ws...>> {};
template<typename S, typename T> struct _sequence_extract : _sequence_extract<S, to_sequence<T, nat>> {};
template<auto... Vs, nat... Is> struct _sequence_extract<sequence<Vs...>, sequence<Is...>> : _type<sequence<select_value<Is, Vs...>...>> {};
ywlib_namespace_internal_end

/// struct to represent a sequence of values
template<auto... Vs> struct sequence {

  /// number of values
  static constexpr nat count = sizeof...(Vs);

  /// value at the index
  template<nat I> static constexpr auto at = select_value<I, Vs...>;

  /// type of the value at the index
  template<nat I> using type_at = decltype(at<I>);

  /// appends the sequence to this
  template<sequence_of S> using append = typename _::_sequence_append<sequence, S>::type;

  /// extracts a subsequence
  template<indices_for<sequence> S> using extract = typename _::_sequence_extract<sequence, S>::type;

  /// extracts the first `N` values
  template<nat N> requires (N <= sizeof...(Vs)) using fore = extract<make_sequence<0, N>>;

  /// extracts the last `N` values
  template<nat N> requires (N <= sizeof...(Vs)) using back = extract<make_sequence<sizeof...(Vs) - N, sizeof...(Vs)>>;

  /// inserts the sequence to this
  template<nat I, sequence_of S> requires (I <= sizeof...(Vs))
  using insert = typename fore<I>::template append<S>::template append<back<sizeof...(Vs) - I>>;

  /// expands this into the template
  template<template<auto...> typename T> using expand = T<Vs...>;

  /// `get` function
  template<nat I> requires (I < sizeof...(Vs))constexpr const auto&& get() const
    noexcept { return static_cast<const type_at<I>&&>(at<I>); }
};

ywlib_namespace_std_begin
template<auto... Vs> struct tuple_size<yw::sequence<Vs...>> : integral_constant<size_t, sizeof...(Vs)> {};
template<size_t I, auto... Vs> struct tuple_element<I, yw::sequence<Vs...>> : type_identity<decltype(yw::select_value<I, Vs...>)> {};
ywlib_namespace_std_end

ywlib_namespace_internal_begin
template<typename T, typename S> struct _to_typepack;
template<typename T, nat... Is> struct _to_typepack<T, sequence<Is...>> : _type<typepack<element_t<T, Is>...>> {};
template<typename T, typename U> struct _typepack_append;
template<typename... Ts, typename... Us> struct _typepack_append<typepack<Ts...>, typepack<Us...>> : _type<typepack<Ts..., Us...>> {};
template<typename T, typename S> struct _typepack_extract;
template<typename... Ts, nat... Is> struct _typepack_extract<typepack<Ts...>, sequence<Is...>> : _type<typepack<select_type<Is, Ts...>...>> {};
ywlib_namespace_internal_end

/// converts a tuple-like type to `typepack`
template<typename T> using to_typepack = typename _::_to_typepack<T, make_indices_for<T>>::type;

/// struct to represent a pack of types
template<typename... Ts> struct typepack {

  /// number of types
  static constexpr nat count = sizeof...(Ts);

  /// common type of the types
  using common = common_type<Ts...>;

  /// type at the index
  template<nat I> requires(I < (sizeof...(Ts))) using at = select_type<I, Ts...>;

  /// appends the element-types of the tuple-like type to this
  template<tuple Tp> using append = typename _::_typepack_append<typepack, to_typepack<Tp>>::type;

  /// extracts a sub-typepack
  template<indices_for<typepack> Sq> using extract = typename _::_typepack_extract<typepack, to_sequence<Sq, nat>>::type;

  /// extracts the first `N` types
  template<nat N> requires(N <= (sizeof...(Ts))) using fore = extract<make_sequence<0, N>>;

  /// extracts the last `N` types
  template<nat N> requires(N <= (sizeof...(Ts))) using back = extract<make_sequence<(sizeof...(Ts)), (sizeof...(Ts)) - N>>;

  /// inserts the element-types of the tuple-like type to this
  template<nat I, tuple T> requires(I < (sizeof...(Ts))) using insert = typename fore<I>::template append<T>::template append<back<(sizeof...(Ts)) - I>>;

  /// expands types into the template
  template<template<typename...> typename Tm> using expand = Tm<Ts...>;

  /// `get` function; declaration only
  template<nat I> requires(I < (sizeof...(Ts))) constexpr at<I> get() const noexcept;
};

ywlib_namespace_std_begin
template<typename... Ts> struct tuple_size<yw::typepack<Ts...>> : integral_constant<size_t, sizeof...(Ts)> {};
template<size_t I, typename... Ts> struct tuple_element<I, yw::typepack<Ts...>> : type_identity<yw::select_type<I, Ts...>> {};
ywlib_namespace_std_end

ywlib_namespace_internal_begin
template<typename F, typename... Ts> requires (!(tuple<Ts> || ...)) constexpr decltype(auto) _apply(F&& f, Ts&&... ts) ywlib_wrapper_auto(invoke(fwd<F>(f), fwd<Ts>(ts)...));
template<nat I, nat... Is, nat... Js, nat... Ks, typename F, typename... Ts> constexpr decltype(auto) _apply_b(sequence<Is...>, sequence<Js...>, sequence<Ks...>, F&& f, Ts&&... ts) ywlib_wrapper_auto(_apply(fwd<F>(f), select_parameter<Is>(fwd<Ts>(ts)...)..., get<Js>(fwd<select_type<I, Ts...>>(select_parameter<I>(fwd<Ts>(ts)...)))..., select_parameter<Ks>(fwd<Ts>(ts)...)...));
template<nat I, typename F, typename... Ts> constexpr decltype(auto) _apply_a(F&& f, Ts&&... ts) ywlib_wrapper_auto(_apply_b<I>(make_sequence<0, I>{}, make_indices_for<select_type<I, Ts...>>{}, make_sequence<I + 1, sizeof...(Ts)>{}, fwd<F>(f), fwd<Ts>(ts)...));
template<typename F, typename... Ts> requires (tuple<Ts> || ...) constexpr decltype(auto) _apply(F&& f, Ts&&... ts) ywlib_wrapper_auto(_apply_a<inspects<tuple<Ts>...>>(fwd<F>(f), fwd<Ts>(ts)...));
template<typename Sq, typename Lt, typename Rt> struct _vassignable : sequence<false, false> {};
template<nat... Is, typename Lt, typename Rt> struct _vassignable<sequence<Is...>, Lt, Rt> : sequence<(assignable<element_t<Lt, Is>, element_t<Rt, Is>> && ...), (nt_assignable<element_t<Lt, Is>, element_t<Rt, Is>> && ...)> {};
ywlib_namespace_internal_end

/// checks if `apply(F, Ts...)` is well-formed
template<typename F, typename... Ts> concept applyable =
  requires(F&& f, Ts&&... ts) { { _::_apply(fwd<F>(f), fwd<Ts>(ts)...) }; };

/// checks if `apply(F, Ts...)` is well-formed and noexcept
template<typename F, typename... Ts> concept nt_applyable =
  applyable<F, Ts...> && requires(F&& f, Ts&&... ts) { { _::_apply(fwd<F>(f), fwd<Ts>(ts)...) } noexcept; };

/// result type of `apply(F, Ts...)`
template<typename F, typename... Ts> using apply_result = decltype(_::_apply(declval<F>(), declval<Ts>()...));

/// applies a function to arguments with tuples expanded
inline constexpr auto apply =
[]<typename F, typename... Ts>(F&& Func, Ts&&... Args) noexcept(nt_applyable<F, Ts...>)
  requires applyable<F, Ts...> { return _::_apply(fwd<F>(Func), fwd<Ts>(Args)...); };

/// checks if `build<T>(Tp)` is well-formed
template<typename T, typename Tp> concept buildable = applyable<decltype(construct<T>), Tp>;

/// checks if `build<T>(Tp)` is well-formed and noexcept
template<typename T, typename Tp> concept nt_buildable = nt_applyable<decltype(construct<T>), Tp>;

/// constructs an object of type `T` from elements of `Tp`
template<typename T> inline constexpr auto build =
[]<typename Tp>(Tp&& Tuple) noexcept(nt_buildable<T, Tp>)
  requires buildable<T, Tp> { return apply(construct<T>, fwd<Tp>(Tuple)); };

/// checks if each element of `Rt` can be assigned to the corresponding element of `Lt`
template<typename Lt, typename Rt> concept vassignable =
  same_extent<Lt, Rt> && requires { requires _::_vassignable<make_indices_for<Lt>, Lt, Rt>::template at<0>; };

/// checks if each element of `Rt` can be assigned to the corresponding element of `Lt` without throwing exceptions
template<typename Lt, typename Rt> concept nt_vassignable =
  vassignable<Lt, Rt> && requires { requires _::_vassignable<make_indices_for<Lt>, Lt, Rt>::template at<1>; };

/// function object to assign each element of `Rt` to the corresponding element of `Lt`
/// \note `void vassign(Lt&& l, Rt&& r) noexcept(nt_vassignable<Lt, Rt>) requires vassignable<Lt, Rt>`
inline constexpr auto vassign =
[]<typename Lt, typename Rt>(Lt&& l, Rt&& r) noexcept(nt_vassignable<Lt, Rt>) requires vassignable<Lt, Rt>
{ [&]<nat... Is>(sequence<Is...>) { ((get<Is>(fwd<Lt>(l)) = get<Is>(fwd<Rt>(r))), ...); }(make_indices_for<Lt>{}); };

/// struct to represent a standard tuple-like object
template<typename... Ts> struct list : typepack<Ts...>::template fore<sizeof...(Ts) - 1>::template expand<list> {

  /// base class
  using base = typepack<Ts...>::template fore<sizeof...(Ts) - 1> ::template expand<list>;

  /// number of elements
  static constexpr nat count = sizeof...(Ts);

  /// last element type
  using last_type = select_type<sizeof...(Ts) - 1, Ts...>;

  /// last element
  select_type<sizeof...(Ts) - 1, Ts...> last;

  /// returns the `I`-th element of `this`
  template<nat I> constexpr auto get() & noexcept -> select_type<I, Ts...>& {
    if constexpr (I == sizeof...(Ts) - 1) return last;
    else return base::template get<I>();
  }

  /// returns the `I`-th element of `this` (const& version)
  template<nat I> constexpr auto get() const& noexcept -> add_const<select_type<I, Ts...>&> {
    if constexpr (I == sizeof...(Ts) - 1) return last;
    else return base::template get<I>();
  }

  /// returns the `I`-th element of `this` (&& version)
  template<nat I> constexpr auto get() && noexcept -> select_type<I, Ts...>&& {
    if constexpr (I == sizeof...(Ts) - 1) return fwd<last_type>(last);
    else return static_cast<base&&>(*this).template get<I>();
  }

  /// returns the `I`-th element of `this` (const&& version)
  template<nat I> constexpr auto get() const&& noexcept -> add_const<select_type<I, Ts...>&&> {
    if constexpr (I == sizeof...(Ts) - 1) return fwd<add_const<last_type>>(last);
    else return static_cast<const base&&>(*this).template get<I>();
  }

  /// assigns corresponding elements of `Rt` to `this`
  template<typename Rt> constexpr void operator=(Rt&& R) & noexcept(nt_vassignable<list&, Rt>)
    requires vassignable<list&, Rt> { vassign(*this, fwd<Rt>(R)); }

  /// assigns corresponding elements of `Rt` to `this` (const& version)
  template<typename Rt> constexpr void operator=(Rt&& R) const& noexcept(nt_vassignable<const list&, Rt>)
    requires vassignable<const list&, Rt> { vassign(*this, fwd<Rt>(R)); }

  /// assigns corresponding elements of `Rt` to `this` (&& version)
  template<typename Rt> constexpr void operator=(Rt&& R) && noexcept(nt_vassignable<list&&, Rt>)
    requires vassignable<list&&, Rt> { vassign(mv(*this), fwd<Rt>(R)); }

  /// assigns corresponding elements of `Rt` to `this` (const&& version)
  template<typename Rt> constexpr void operator=(Rt&& R) const&& noexcept(nt_vassignable<const list&&, Rt>)
    requires vassignable<const list&&, Rt> { vassign(mv(*this), fwd<Rt>(R)); }
};

/// struct to represent a standard tuple-like object
template<typename T1, typename T2, typename T3> struct list<T1, T2, T3> : list<T1, T2> {

  /// number of elements
  static constexpr nat count = 3;

  /// third element type
  using third_type = T3;

  /// third element
  third_type third;

  /// returns the `I`-th element of `this`
  template<nat I> constexpr auto get() & noexcept -> select_type<I, T1, T2, T3>& {
    if constexpr (I == 2) return third;
    else return static_cast<list<T1, T2>&>(*this).template get<I>();
  }

  /// returns the `I`-th element of `this`
  template<nat I> constexpr auto get() const& noexcept -> add_const<select_type<I, T1, T2, T3>&> {
    if constexpr (I == 2) return third;
    else return static_cast<const list<T1, T2>&>(*this).template get<I>();
  }

  /// returns the `I`-th element of `this`
  template<nat I> constexpr auto get() && noexcept -> select_type<I, T1, T2, T3>&& {
    if constexpr (I == 2) return fwd<T3>(third);
    else return static_cast<list<T1, T2>&&>(*this).template get<I>();
  }

  /// returns the `I`-th element of `this`
  template<nat I> constexpr auto get() const&& noexcept -> add_const<select_type<I, T1, T2, T3>&&> {
    if constexpr (I == 2) return fwd<add_const<T3>>(third);
    else return static_cast<const list<T1, T2>&&>(*this).template get<I>();
  }

  /// assigns corresponding elements of `Rt` to `this`
  template<typename Rt> constexpr void operator=(Rt&& R) & noexcept(nt_vassignable<list&, Rt>)
    requires vassignable<list&, Rt> { vassign(*this, fwd<Rt>(R)); }

  /// assigns corresponding elements of `Rt` to `this` (const& version)
  template<typename Rt> constexpr void operator=(Rt&& R) const& noexcept(nt_vassignable<const list&, Rt>)
    requires vassignable<const list&, Rt> { vassign(*this, fwd<Rt>(R)); }

  /// assigns corresponding elements of `Rt` to `this` (&& version)
  template<typename Rt> constexpr void operator=(Rt&& R) && noexcept(nt_vassignable<list&&, Rt>)
    requires vassignable<list&&, Rt> { vassign(mv(*this), fwd<Rt>(R)); }

  /// assigns corresponding elements of `Rt` to `this` (const&& version)
  template<typename Rt> constexpr void operator=(Rt&& R) const&& noexcept(nt_vassignable<const list&&, Rt>)
    requires vassignable<const list&&, Rt> { vassign(mv(*this), fwd<Rt>(R)); }
};

template<typename T1, typename T2> struct list<T1, T2> : public list<T1> {
  using base = list<T1>;
  static constexpr nat count = 2;
  using second_type = T2;
  second_type second;

  template<nat I> constexpr auto get() & noexcept -> select_type<I, T1, T2>& {
    if constexpr (I == 1) return second;
    else return base::first;
  }
  template<nat I> constexpr auto get() const& noexcept -> add_const<select_type<I, T1, T2>&> {
    if constexpr (I == 1) return second;
    else return base::first;
  }
  template<nat I> constexpr auto get() && noexcept -> select_type<I, T1, T2>&& {
    if constexpr (I == 1) return fwd<T2>(second);
    else return fwd<T1>(base::first);
  }
  template<nat I> constexpr auto get() const&& noexcept -> add_const<select_type<I, T1, T2>&&> {
    if constexpr (I == 1) return fwd<add_const<T2>>(second);
    else return fwd<add_const<T1>>(base::first);
  }

  /// assigns corresponding elements of `Rt` to `this`
  template<typename Rt> constexpr void operator=(Rt&& R) & noexcept(nt_vassignable<list&, Rt>)
    requires vassignable<list&, Rt> { vassign(*this, fwd<Rt>(R)); }

  /// assigns corresponding elements of `Rt` to `this` (const& version)
  template<typename Rt> constexpr void operator=(Rt&& R) const& noexcept(nt_vassignable<const list&, Rt>)
    requires vassignable<const list&, Rt> { vassign(*this, fwd<Rt>(R)); }

  /// assigns corresponding elements of `Rt` to `this` (&& version)
  template<typename Rt> constexpr void operator=(Rt&& R) && noexcept(nt_vassignable<list&&, Rt>)
    requires vassignable<list&&, Rt> { vassign(mv(*this), fwd<Rt>(R)); }

  /// assigns corresponding elements of `Rt` to `this` (const&& version)
  template<typename Rt> constexpr void operator=(Rt&& R) const&& noexcept(nt_vassignable<const list&&, Rt>)
    requires vassignable<const list&&, Rt> { vassign(mv(*this), fwd<Rt>(R)); }
};

/// struct to represent a standard tuple-like object
template<typename T> struct list<T> {

  /// number of elements
  static constexpr nat count = 1;

  /// first element type
  using first_type = T;

  /// first element
  first_type first;

  /// returns the `I`-th element of `this`
  template<nat I> requires(I == 0) constexpr T& get() & noexcept { return first; }

  /// returns the `I`-th element of `this` (const& version)
  template<nat I> requires(I == 0) constexpr add_const<T&> get() const& noexcept { return first; }

  /// returns the `I`-th element of `this` (&& version)
  template<nat I> requires(I == 0) constexpr T&& get() && noexcept { return fwd<T>(first); }

  /// returns the `I`-th element of `this` (const&& version)
  template<nat I> requires(I == 0) constexpr add_const<T&&> get() const&& noexcept { return fwd<add_const<T>>(first); }

  /// assigns corresponding elements of `Rt` to `this`
  template<typename Rt> constexpr void operator=(Rt&& R) & noexcept(nt_vassignable<list&, Rt>)
    requires vassignable<list&, Rt> { vassign(*this, fwd<Rt>(R)); }

  /// assigns corresponding elements of `Rt` to `this` (const& version)
  template<typename Rt> constexpr void operator=(Rt&& R) const& noexcept(nt_vassignable<const list&, Rt>)
    requires vassignable<const list&, Rt> { vassign(*this, fwd<Rt>(R)); }

  /// assigns corresponding elements of `Rt` to `this` (&& version)
  template<typename Rt> constexpr void operator=(Rt&& R) && noexcept(nt_vassignable<list&&, Rt>)
    requires vassignable<list&&, Rt> { vassign(mv(*this), fwd<Rt>(R)); }

  /// assigns corresponding elements of `Rt` to `this` (const&& version)
  template<typename Rt> constexpr void operator=(Rt&& R) const&& noexcept(nt_vassignable<const list&&, Rt>)
    requires vassignable<const list&&, Rt> { vassign(mv(*this), fwd<Rt>(R)); }
};

/// struct to represent a standard tuple-like object
template<> struct list<> {
private:
  template<typename T, typename U, typename V> struct _from_typepack {};
  template<typename... Ts, typename U, template<typename...> typename Tm, typename... Vs>
  struct _from_typepack<typepack<Ts...>, U, Tm<Vs...>> : _::_type<list<copy_cvref<Tm<Ts>, U>...>> {};
  template<typename... Ts, typename U, template<typename, auto...> typename Tm, typename V, auto... Vs>
  struct _from_typepack<typepack<Ts...>, U, Tm<V, Vs...>> : _::_type<list<copy_cvref<Tm<Ts, Vs...>, U>...>> {};
  template<typename... Ts, typename U, typename V> struct _from_typepack<typepack<Ts...>, U, V> : _::_type<list<copy_cvref<Ts, U>...>> {};
public:
  static constexpr nat count = 0;
  template<typename... Ts> static constexpr auto asref(Ts&&... Args) noexcept { return list<Ts&&...>{fwd<Ts>(Args)...}; }
  template<specialization_of<typepack> Tp, typename Qualifier = none> using from_typepack = _from_typepack<Tp, Qualifier, remove_cvref<Qualifier>>::type;
};

template<typename... Ts> list(Ts...) -> list<Ts...>;

ywlib_namespace_std_begin
template<typename... Ts> struct tuple_size<yw::list<Ts...>> : integral_constant<size_t, sizeof...(Ts)> {};
template<size_t I, typename... Ts> struct tuple_element<I, yw::list<Ts...>> : type_identity<yw::select_type<I, Ts...>> {};
ywlib_namespace_std_end

ywlib_namespace_internal_begin
template<nat I, typename F, typename... Ts> constexpr decltype(auto) _vapply_i(F&& f, Ts&&... ts) noexcept(nt_invocable<F, element_t<Ts, I>...>) requires invocable<F, element_t<Ts, I>...> { return invoke(fwd<F>(f), get<I>(fwd<Ts>(ts))...); }
template<nat... Is, typename F, typename... Ts> constexpr decltype(auto) _vapply_is(sequence<Is...>, F&& f, Ts&&... ts) ywlib_wrapper_auto(list{_vapply_i<Is>(fwd<F>(f), fwd<Ts>(ts)...)...});
template<typename F, typename T, typename... Ts> requires same_extent<T, Ts...> constexpr decltype(auto) _vapply(F&& f, T&& t, Ts&&... ts) ywlib_wrapper_auto(_vapply_is(make_indices_for<T>(), fwd<F>(f), fwd<T>(t), fwd<Ts>(ts)...));
template<typename F, typename T, typename... Ts> requires (!same_extent<T, Ts...> && tuple<T> && (tuple<Ts> && ...)) void _vapply(F&& f, T&& t, Ts&&... ts) = delete;
template<typename F, typename... Ts> requires (!(tuple<Ts> || ...)) void _vapply(F&& f, Ts&&... ts) = delete;
template<nat I, nat N, nat... Is, nat... Js, typename F, typename... Ts> constexpr decltype(auto) _vapply_b(sequence<Is...>, sequence<Js...>, F&& f, Ts&&... ts) ywlib_wrapper_auto(_vapply(fwd<F>(f), select<Is>(fwd<Ts>(ts)...)..., projector(select<I>(fwd<Ts>(ts)...), make_sequence<0, N>{}), select<Js>(fwd<Ts>(ts)...)...));
template<nat I, nat J, typename F, typename... Ts> constexpr decltype(auto) _vapply_a(F&& f, Ts&&... ts) ywlib_wrapper_auto(_vapply_b<I, extent<select_type<J, Ts...>>>( make_sequence<0, I>{}, make_sequence<I + 1, sizeof...(Ts)>{}, fwd<F>(f), fwd<Ts>(ts)...));
template<typename F, typename... Ts> requires (!(tuple<Ts> && ...) && (tuple<Ts> || ...)) constexpr decltype(auto) _vapply(F&& f, Ts&&... ts) ywlib_wrapper_auto(_vapply_a<inspects<!tuple<Ts>...>, inspects<tuple<Ts>...>>(fwd<F>(f), fwd<Ts>(ts)...));
ywlib_namespace_internal_end

/// checks if `vapply(F, Ts...)` is well-formed
template<typename T, typename... Ts> concept vapplyable =
  requires(T&& t, Ts&&... ts) { { _::_vapply(fwd<T>(t), fwd<Ts>(ts)...) }; };

/// checks if `vapply(F, Ts...)` is well-formed and noexcept
template<typename T, typename... Ts> concept nt_vapplyable =
  vapplyable<T, Ts...> && requires(T&& t, Ts&&... ts) { { _::_vapply(fwd<T>(t), fwd<Ts>(ts)...) } noexcept; };

/// result type of `vapply(F, Ts...)`
template<typename T, typename... Ts> using vapply_result = decltype(_::_vapply(declval<T>(), declval<Ts>()...));

/// virtically applies a function to arguments
inline constexpr auto vapply =
[]<typename F, typename... Ts>(F&& f, Ts&&... ts) noexcept(nt_vapplyable<F, Ts...>)
  requires vapplyable<F, Ts...> { return _::_vapply(fwd<F>(f), fwd<Ts>(ts)...); };

/// struct to project and arrange a tuple-like object
template<typename T, typename F = pass, sequence_of<nat> S = make_indices_for<T>>
struct projector {

  static_assert(0 < to_sequence<S, nat>::count);
  static_assert(!tuple<T> || indices_for<S, T>);

  /// indices type
  using indices = to_sequence<S, nat>;

  /// number of elements
  static constexpr nat count = indices::count;

  /// reference to the tuple-like object
  add_fwref<T> ref;

  /// projection function
  remove_ref<F> func{};

  /// constructor to specify the projection function and the arrangement
  template<typename U = T> constexpr projector(U&& Ref, F f, S)
    noexcept(nt_constructible<F, F>) : ref(fwd<U>(Ref)), func(mv(f)) {}

  /// constructor to specify the projection function
  template<typename U = T> constexpr projector(U&& Ref, F f)
    noexcept(nt_constructible<F, F>) : ref(fwd<U>(Ref)), func(mv(f)) {}

  /// constructor to specify the arrangement
  template<typename U = T> constexpr projector(U&& Ref, S) noexcept : ref(fwd<U>(Ref)) {}

  /// constructor
  template<typename U = T> constexpr projector(U&& Ref) noexcept : ref(fwd<U>(Ref)) {}

  /// `get` function
  template<nat I> requires (I < indices::count && tuple<T>)
  constexpr decltype(auto) get() const ywlib_wrapper_auto(invoke(func, yw::get<indices::template at<I>>(ref)));

  /// `get` function
  template<nat I> requires (I < indices::count && !tuple<T>)
  constexpr decltype(auto) get() const ywlib_wrapper_auto(invoke(func, ref));
};

template<typename T, typename F, sequence_of<nat> S> projector(T&&, F, S) -> projector<T, F, S>;
template<typename T, typename F> requires(!sequence_of<F, nat>) projector(T&&, F) -> projector<T, F>;
template<typename T, sequence_of<nat> S> projector(T&&, S) -> projector<T, pass, S>;
template<typename T> projector(T&&) -> projector<T>;

ywlib_namespace_std_begin
template<typename T, typename F, typename S> struct tuple_size<yw::projector<T, F, S>> : integral_constant<size_t, yw::projector<T, F, S>::count> {};
template<size_t I, typename T, typename F, typename S> struct tuple_element<I, yw::projector<T, F, S>> : type_identity<decltype(declval<yw::projector<T, F, S>>().template get<I>())> {};
ywlib_namespace_std_end

ywlib_namespace_internal_begin
template<typename T> struct _iter_t {};
template<std::ranges::range Rg> struct _iter_t<Rg> : _iter_t<std::ranges::iterator_t<Rg>> {};
template<std::input_or_output_iterator It> struct _iter_t<It> { using v = std::iter_value_t<It>; using d = std::iter_difference_t<It>; using r = std::iter_reference_t<It>; using rr = std::iter_rvalue_reference_t<It>; };
ywlib_namespace_internal_end


template<typename It> concept iterator = std::input_or_output_iterator<It>;

template<typename Se, typename It> concept sentinel_for = std::sentinel_for<Se, It>;

template<typename Se, typename It> concept sized_sentinel_for = std::sized_sentinel_for<Se, It>;

template<typename Rg> concept range = std::ranges::range<Rg>;

template<typename Rg> concept borrowed_range = std::ranges::borrowed_range<Rg>;

template<typename Rg> concept sized_range = std::ranges::sized_range<Rg>;

template<range Rg> using iterator_t = std::ranges::iterator_t<Rg>;

template<range Rg> using borrowed_iterator_t = std::ranges::borrowed_iterator_t<Rg>;

template<range Rg> using sentinel_t = std::ranges::sentinel_t<Rg>;

/// value type of iterator/range
template<typename T> requires iterator<remove_ref<T>> || range<remove_ref<T>>
using iter_value = typename _::_iter_t<remove_ref<T>>::v;

/// difference type of iterator/range
template<typename T> requires iterator<remove_ref<T>> || range<remove_ref<T>>
using iter_difference = typename _::_iter_t<remove_ref<T>>::d;

/// reference type of iterator/range
template<typename T> requires iterator<remove_ref<T>> || range<remove_ref<T>>
using iter_reference = typename _::_iter_t<remove_ref<T>>::r;

/// rvalue reference type of iterator/range
template<typename T> requires iterator<remove_ref<T>> || range<remove_ref<T>>
using iter_rvref = typename _::_iter_t<remove_ref<T>>::rr;

/// common type of `iter_reference` and `iter_value`
template<typename T> requires iterator<remove_ref<T>> || range<remove_ref<T>>
using iter_common = common_type<iter_reference<T>, iter_value<T>>;

template<typename It, typename T> concept output_iterator = std::output_iterator<It, T>;

template<typename It> concept input_iterator = std::input_iterator<It>;

template<typename It> concept fwd_iterator = std::forward_iterator<It>;

template<typename It> concept bid_iterator = std::bidirectional_iterator<It>;

template<typename It> concept rnd_iterator = std::random_access_iterator<It>;

template<typename It> concept cnt_iterator = std::contiguous_iterator<It>;

template<typename It, typename U> concept iterator_for = iterator<It> && convertible_to<iter_reference<It>, U>;

template<typename It, typename U> concept cnt_iterator_of = cnt_iterator<It> && same_as<remove_ref<iter_reference<It>>, U>;

template<typename Rg, typename T> concept output_range = std::ranges::output_range<Rg, T>;

template<typename Rg> concept input_range = std::ranges::input_range<Rg>;

template<typename Rg> concept fwd_range = std::ranges::forward_range<Rg>;

template<typename Rg> concept bid_range = std::ranges::bidirectional_range<Rg>;

template<typename Rg> concept rnd_range = std::ranges::random_access_range<Rg>;

template<typename Rg> concept cnt_range = std::ranges::contiguous_range<Rg>;

template<typename Rg, typename U> concept range_for = iterator_for<iterator_t<Rg>, U>;

template<typename Rg, typename U> concept cnt_range_of = cnt_range<Rg> && same_as<iter_value<iterator_t<Rg>>, U>;

template<typename Rg> concept cnt_sized_range = cnt_range<Rg> && sized_range<Rg>;

template<typename Rg, typename U> concept cnt_sized_range_of = cnt_sized_range<Rg> && cnt_range_of<Rg, U>;

template<typename It, typename In> concept iter_copyable = iterator<It> && iterator<In> && std::indirectly_copyable<In, It>;

template<typename It, typename In> concept iter_movable = iterator<It> && iterator<In> && std::indirectly_movable<In, It>;

template<typename Fn, typename It> concept iter_unary_invocable = iterator<It> && std::indirectly_unary_invocable<Fn, It>;

template<typename Fn, typename It> concept iter_unary_predicate = iterator<It> && std::indirect_unary_predicate<Fn, It>;


inline constexpr auto begin = []<range Rg>(Rg&& r)
  ywlib_wrapper_auto(std::ranges::begin(fwd<Rg>(r)));

inline constexpr auto end = []<range Rg>(Rg&& r)
  ywlib_wrapper_auto(std::ranges::end(fwd<Rg>(r)));

inline constexpr auto rbegin = []<range Rg>(Rg&& r)
  ywlib_wrapper_auto(std::ranges::rbegin(fwd<Rg>(r)));

inline constexpr auto rend = []<range Rg>(Rg&& r)
  ywlib_wrapper_auto(std::ranges::rend(fwd<Rg>(r)));

inline constexpr auto size = []<range Rg>(Rg&& r)
  ywlib_wrapper_auto(std::ranges::size(fwd<Rg>(r)));

inline constexpr auto empty = []<range Rg>(Rg&& r)
  ywlib_wrapper_auto(std::ranges::empty(fwd<Rg>(r)));

inline constexpr auto data = []<range Rg>(Rg&& r)
  ywlib_wrapper_auto(std::ranges::data(fwd<Rg>(r)));

inline constexpr auto iter_move = []<iterator It>(It&& i)
  ywlib_wrapper_auto(std::ranges::iter_move(fwd<It>(i)));

inline constexpr auto iter_swap = []<iterator It, iterator Jt>(It&& i, Jt&& j)
  ywlib_wrapper_auto(std::ranges::iter_swap(fwd<It>(i), fwd<Jt>(j)));

}
