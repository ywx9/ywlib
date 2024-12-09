#pragma once

#include "std.hpp"

#define nat size_t

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

template<typename T, typename... Ts> concept same_as = (std::same_as<T, Ts> && ...);
template<typename T, typename... Ts> concept included_in = (std::same_as<T, Ts> || ...);
template<typename T, typename... Ts> concept different_from = !included_in<T, Ts...>;
template<typename T, typename... Ts> concept derived_from = (std::derived_from<T, Ts> && ...);
template<typename T, typename... Ts> concept convertible_to = (std::convertible_to<T, Ts> && ...);
template<typename T, typename... Ts> concept nt_convertible_to = convertible_to<T, Ts...> && (std::is_nothrow_convertible_v<T, Ts> && ...);
template<typename T, typename... Ts> concept castable_to = requires(T (&f)()) { ((static_cast<Ts>(f())) && ...); };
template<typename T, typename... Ts> concept nt_castable_to = castable_to<T, Ts...> && requires(T (&f)() noexcept) { requires (noexcept(static_cast<Ts>(f())) && ...); };

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
inline constexpr auto mv = []<typename T>(T&& Ref) noexcept -> decltype(auto) { return static_cast<add_rvref<T>>(Ref); };
template<typename T> inline constexpr auto fwd = [](nt_convertible_to<T&&> auto&& Ref) noexcept { return static_cast<T&&>(Ref); };
template<typename T> inline constexpr auto declval = []() noexcept -> add_fwref<T> {};

// clang-format off

template<typename T> struct t_is_member_pointer : constant<false> {};
template<typename M, typename C> struct t_is_member_pointer<M C::*> : constant<true> { using class_type = C; using member_type = M; };
template<typename M, typename C> struct t_is_member_pointer<M C::* const> : constant<true> { using class_type = C; using member_type = M; };
template<typename M, typename C> struct t_is_member_pointer<M C::* volatile> : constant<true> { using class_type = C; using member_type = M; };
template<typename M, typename C> struct t_is_member_pointer<M C::* const volatile> : constant<true> { using class_type = C; using member_type = M; };
template<typename T> concept is_member_pointer = t_is_member_pointer<T>::value && is_pointer<T>;
template<is_member_pointer T> using class_type = typename t_is_member_pointer<T>::class_type;
template<is_member_pointer T> using member_type = typename t_is_member_pointer<T>::member_type;
template<typename T> concept is_member_function_pointer = is_member_pointer<T> && is_function<member_type<T>>;
template<typename T> concept is_member_object_pointer = is_member_pointer<T> && !is_member_function_pointer<T>;

// clang-format on

template<bool... Bs> inline constexpr nat counts = (Bs + ...);
template<bool... Bs> inline constexpr nat inspects = 0;
template<bool B, bool... Bs> inline constexpr nat inspects<B, Bs...> = B ? 0 : 1 + inspects<Bs...>;
template<convertible_to<nat> auto I, nat N> inline constexpr bool selectable = (is_bool<decltype(I)> && N == 2) || (!is_bool<decltype(I)> && static_cast<nat>(I) < N);

namespace _ {
template<typename T, T I, typename... Ts> struct _select_type : _select_type<nat, I, Ts...> {};
template<bool B, typename Tt, typename Tf> struct _select_type<bool, B, Tt, Tf> : std::conditional<B, Tt, Tf> {};
template<nat I, typename T, typename... Ts> struct _select_type<nat, I, T, Ts...> : _select_type<nat, I - 1, Ts...> {};
template<typename T, typename... Ts> struct _select_type<nat, 0, T, Ts...> : t_type<T> {};
}

template<convertible_to<nat> auto I, typename... Ts> requires selectable<I, sizeof...(Ts)> using select_type = typename _::_select_type<decltype(I), I, Ts...>::type;
template<convertible_to<nat> auto I, auto... Vs> requires selectable<I, sizeof...(Vs)> inline constexpr auto select_value = select_type<I, constant<Vs>...>::value;

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
template<typename T, typename A> concept nt_assignable = assignable<T, A> && std::is_nothrow_assignable_v<T, A>;
inline constexpr auto assign = []<typename T, typename A>(T&& Ref, A&& Arg) noexcept(nt_assignable<T, A>) requires assignable<T, A> { fwd<T>(Ref) = fwd<A>(Arg); };

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

template<trivially_copyable T, trivially_copyable U> requires (sizeof(T) == sizeof(U)) constexpr T bitcast(const U& Val) noexcept { return __builtin_bit_cast(T, Val); };
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
  template<typename T> requires (j<T> < sizeof...(Fs)) //
  constexpr operator T() const ywlib_wrap_auto(select_type<j<T>, Fs...>::operator()());
  template<typename... As> requires (k<As...> < sizeof...(Fs)) //
  constexpr decltype(auto) operator()(As&&... Args) const ywlib_wrap_auto(select_type<k<As...>, Fs...>::operator()(fwd<As>(Args)...));
};

inline constexpr caster is_cev{[]() noexcept { return std::is_constant_evaluated(); }};

enum class get_strategy {
  ungettable = 0,
  itself = 0x1,
  array = 0x2,
  tuple = 0x4,
  member = 0x8,
  nothrow = 0x10,
};
constexpr get_strategy operator&(get_strategy a, get_strategy b) noexcept { return static_cast<get_strategy>(static_cast<int>(a) & static_cast<int>(b)); }
constexpr get_strategy operator|(get_strategy a, get_strategy b) noexcept { return static_cast<get_strategy>(static_cast<int>(a) | static_cast<int>(b)); }
constexpr get_strategy operator^(get_strategy a, get_strategy b) noexcept { return static_cast<get_strategy>(static_cast<int>(a) ^ static_cast<int>(b)); }
constexpr get_strategy operator~(get_strategy a) noexcept { return static_cast<get_strategy>(~static_cast<int>(a)); }

namespace _::_get {
template<nat I> void get() = delete;
template<nat I, typename T> inline constexpr get_strategy strategy = []() -> get_strategy {
  using t = remove_ref<T>;
  using enum get_strategy;
  if constexpr (is_bounded_array<t>) {
    if constexpr (I < std::extent_v<t>) return array | nothrow;
    else return ungettable;
  } else if constexpr (is_unbounded_array<t>) return ungettable;
  else if constexpr (is_class<t> || is_union<t>) {
    if constexpr (requires { get<I>(declval<T>()); }) return noexcept(get<I>(declval<T>())) ? tuple | nothrow : tuple;
    else if constexpr (requires { declval<T>().template get<I>(); }) return noexcept(declval<T>().template get<I>()) ? tuple | member | nothrow : tuple | member;
    else return I == 0 ? itself | nothrow : ungettable;
  } else return I == 0 ? itself | nothrow : ungettable;
}();
}

template<typename T> concept tuple = _::_get::strategy<0, T> & get_strategy::tuple;
template<typename T, nat I> concept gettable = _::_get::strategy<I, T> != get_strategy::ungettable;
template<typename T, nat I> concept nt_gettable = gettable<T, I> && bool(_::_get::strategy<I, T> & get_strategy::nothrow);
template<nat I> inline constexpr auto get = []<typename T>(T&& Ref) noexcept(nt_gettable<T, I>) -> decltype(auto) requires gettable<T, I> {
  using enum get_strategy;
  constexpr auto s = _::_get::strategy<I, T>;
  if constexpr (s & itself) return fwd<T>(Ref);
  else if constexpr (s & array) return fwd<T>(Ref)[I];
  else if constexpr (s & member)
};
template<typename T, nat I> requires gettable<T, I> using element_t = decltype(get<I>(declval<T>()));

template<typename T> struct t_extent : constant<0_n> {};
template<typename T> requires (bool(_::_get::strategy<0, T>& get_strategy::itself)) struct t_extent<T> : constant<1_n> {};
template<typename T> requires (bool(_::_get::strategy<0, T>& get_strategy::array)) struct t_extent<T> : std::extent<remove_ref<T>> {};
template<typename T> requires (bool(_::_get::strategy<0, T>& get_strategy::tuple)) struct t_extent<T> : std::tuple_size<remove_ref<T>> {};
template<typename T> inline constexpr nat extent = t_extent<T>::value;
template<typename T, typename... Ts> concept same_extent = ((extent<T> == extent<Ts>) && ...);

template<auto... Vs> struct sequence;
template<typename... Ts> struct typepack;
template<typename... Ts> struct list;

template<typename S, typename T> struct t_to_sequence;
template<template<auto...> typename Tm, typename T, auto... Vs> struct t_to_sequence<Tm<Vs...>, T> : t_type<sequence<T(Vs)...>> {};
template<template<auto...> typename Tm, auto... Vs> struct t_to_sequence<Tm<Vs...>, none> : t_type<sequence<Vs...>> {};
template<template<typename, auto...> typename Tm, typename T, typename U, auto... Vs> struct t_to_sequence<Tm<U, Vs...>, T> : t_type<sequence<T(Vs)...>> {};
template<template<typename, auto...> typename Tm, typename U, auto... Vs> struct t_to_sequence<Tm<U, Vs...>, none> : t_type<sequence<Vs...>> {};
template<typename S, typename T = none> using to_sequence = typename t_to_sequence<S, T>::type;
template<typename S, typename T = none> concept sequence_of = variation_of<to_sequence<S, T>, sequence<>>;

template<typename S, typename T> struct t_indices_for : t_indices_for<to_sequence<S>, T> {};
template<nat... Vs, typename T> struct t_indices_for<sequence<Vs...>, T> : constant<(lt(Vs, extent<T>) && ...)> {};
template<typename S, typename T> concept indices_for = t_indices_for<S, T>::value;

template<nat Begin, nat End, auto Proj, nat... Vs> struct _make_sequence : _make_sequence<Begin + 1, End, Proj, Vs..., Proj(I)> {};
template<nat End, auto Proj, nat... Vs> struct _make_sequence<End, End, Proj, Vs...> : _type<sequence<Vs...>> {};
template<nat Begin, nat End, invocable<nat> auto Proj = pass{}> requires (Begin <= End) using make_sequence = typename t_make_sequence<Begin, End, Proj>::type;
template<typename Tp> using make_indices_for = make_sequence<0, extent<Tp>>;

template<typename Sq, nat... Is> struct t_extracting_indices : t_extracting_indices<to_sequence<Sq, bool>> {};
template<bool... Bs> struct t_extracting_indices<sequence<Bs...>> : t_extracting_indices<sequence<Bs...>, 0, sizeof...(Bs)> {};
template<bool... Bs, nat I, nat N, nat... Is> struct t_extracting_indices<sequence<Bs...>, I, N, Is...> //
  : select_type<select_value<I, Bs...>, t_extracting_indices<sequence<Bs...>, I + 1, N, Is..., I>, t_extracting_indices<sequence<Bs...>, I + 1, N, Is...>> {};
template<bool... Bs, nat N, nat... Is> struct t_extracting_indices<sequence<Bs...>, N, N, Is...> : t_type<sequence<Is...>> {};
template<sequence_of<bool> Sq> using extracting_indices = typename t_extracting_indices<Sq>::type;

template<typename S1, typename S2> struct t_append_sequence : t_append_sequence<to_sequence<S1>, to_sequence<S2>> {};
template<auto... Vs, auto... Ws> struct t_append_sequence<sequence<Vs...>, sequence<Ws...>> : t_type<sequence<Vs..., Ws...>> {};
template<typename S1, typename S2> using append_sequence = typename t_append_sequence<S1, S2>::type;

template<typename S, typename Is> struct t_extract_sequence : t_extract_sequence<to_sequence<S>, to_sequence<Is, nat>> {};
template<auto... Vs, nat... Is> struct t_extract_sequence<sequence<Vs...>, sequence<Is...>> : t_type<sequence<select_value<Is, Vs...>...>> {};
template<typename S, typename Is> using extract_sequence = typename t_extract_sequence<S, Is>::type;

template<auto... Vs> struct sequence {
  static constexpr nat count = sizeof...(Vs);
  template<nat I> requires (I < sizeof...(Vs)) static constexpr auto at = select_value<I, Vs...>;
  template<nat I> requires (I < sizeof...(Vs)) using type_at = select_type<I, decltype(Vs)...>;
  template<sequence_of Sq> using append = typename t_sequence_append<sequence, Sq>::type;
  template<indices_for<sequence> Ix> using extract = typename t_extract_sequence<sequence, Ix>::type;
  template<nat N> requires (N <= sizeof...(Vs)) using fore = extract<make_sequence<0, N>>;
  template<nat N> requires (N <= sizeof...(Vs)) using back = extract<make_sequence<sizeof...(Vs) - N, sizeof...(Vs)>>;
  template<nat I> requires (I < sizeof...(Vs)) using remove = typename fore<I>::template append<back<sizeof...(Vs) - I - 1>>;
  template<nat I, sequence_of Sq> requires (I <= sizeof...(Vs)) using insert = typename fore<I>::template append<Sq>::template append<back<sizeof...(Vs) - I>>;
  template<template<auto...> typename Tm> using expand = Tm<Vs...>;
  template<nat I> requires (I < sizeof...(Vs)) constexpr const auto&& get() const noexcept { return mv(at<I>); }
};

template<typename T, typename Is = make_indices_for<T>> struct t_to_typepack : t_to_typepack<T, to_sequence<Is, nat>> {};
template<typename T, nat... Is> struct t_to_typepack<T, sequence<Is...>> : t_type<typepack<element_t<T, Is>...>> {};
template<typename T> using to_typepack = typename t_to_typepack<T>::type;

template<typename T> struct t_common_element : t_common_element<to_typepack<T>> {};
template<typename... Ts> struct t_common_element<typepack<Ts...>> : t_type<common_type<Ts...>> {};
template<typename T> using common_element = typename t_common_element<T>::type;

template<typename T, typename U> struct t_tuple_for : t_tuple_for<to_typepack<T>, U> {};
template<typename... Ts, typename U> struct t_tuple_for<typepack<Ts...>, U> : constant<(convertible_to<Ts, U> && ...)> {};
template<typename T, typename U> concept tuple_for = to_typepack<T>::template expand<t_tuple_for>::value;

template<typename T, typename U> struct append_typepack : append_typepack<to_typepack<T>, to_typepack<U>> {};
template<typename... Ts, typename... Us> struct append_typepack<typepack<Ts...>, typepack<Us...>> : t_type<typepack<Ts..., Us...>> {};
template<typename T, typename U> using append_typepack_t = typename append_typepack<T, U>::type;

template<typename T, typename Is> struct extract_typepack : extract_typepack<to_typepack<T>, to_sequence<Is, nat>> {};
template<typename... Ts, nat... Is> struct extract_typepack<typepack<Ts...>, sequence<Is...>> : t_type<typepack<select_type<Is, Ts...>...>> {};
template<typename T, typename Is> using extract_typepack_t = typename extract_typepack<T, Is>::type;

template<typename... Ts> struct typepack {
  static constexpr nat count = sizeof...(Ts);
  using common = common_type<Ts...>;
  template<nat I> requires (I < sizeof...(Ts)) using at = select_type<I, Ts...>;
  template<tuple Tp> using append = typename t_append_typepack<typepack, Tp>::type;
  template<indices_for<typepack> Ix> using extract = typename t_extract_typepack<typepack, Ix>::type;
  template<nat N> requires (N <= sizeof...(Ts)) using fore = extract<make_sequence<0, N>>;
  template<nat N> requires (N <= sizeof...(Ts)) using back = extract<make_sequence<sizeof...(Ts) - N, sizeof...(Ts)>>;
  template<nat I> requires (I < sizeof...(Ts)) using remove = typename fore<I>::template append<back<sizeof...(Ts) - I - 1>>;
  template<nat I, typename T> requires (I <= sizeof...(Ts)) using insert = typename fore<I>::template append<T>::template append<back<sizeof...(Ts) - I>>;
  template<template<typename...> typename Tm> using expand = Tm<Ts...>;
  template<nat I> requires (I < sizeof...(Ts)) constexpr const at<I> get() const noexcept;
};

template<typename T, typename Pj, typename Sq> class tuple_view {
  using sequence = to_sequence<Sq, nat>;
  static_assert(sequence::count > 0);
  static_assert(invocable<Pj&, element_t<T, sequence::template at<0>>>);
  T&& ref;
  Pj proj{};
public:
  tuple_view(T&& Ref) noexcept : ref(fwd<T>(Ref)) {}
  tuple_view(T&& Ref, Pj p) noexcept : ref(fwd<T>(Ref)), proj(mv(p)) {}
  tuple_view(T&& Ref, Sq) noexcept : ref(fwd<T>(Ref)) {}
  tuple_view(T&& Ref, Pj p, Sq) noexcept : ref(fwd<T>(Ref)), proj(mv(p)) {}
  template<nat I> using type_at = invoke_result<Pj&, element_t<T, sequence::template at<I>>>;
  template<nat I> requires (I < sequence::count) constexpr decltype(auto) get() const { return invoke(proj, yw::get<sequence::template at<I>>(ref)); }
};

template<typename T> tuple_view(T&&) -> tuple_view<T&&, pass, make_indices_for<T>>;
template<typename T, typename Pj> tuple_view(T&&, Pj) -> tuple_view<T&&, Pj, make_indices_for<T>>;
template<typename T, typename Sq> tuple_view(T&&, Sq) -> tuple_view<T&&, pass, Sq>;
template<typename T, typename Pj, typename Sq> tuple_view(T&&, Pj, Sq) -> tuple_view<T&&, Pj, Sq>;

/// \brief function to check if all elements of a tuple satisfy a concept
/// \return true if all elements of the tuple satisfy the concept
/// \note This returns `(Concept<element_t<Tuple, Is>, Others...> && ...)` where `Is` are the indices of the tuple.
#define all_elements_are(Tuple, Concept, ...)                              \
  []<nat... Is>(yw::sequence<Is...>) {                                     \
    if constexpr (sizeof...(Is) == 0) return true;                         \
    else return ((Concept<yw::element_t<Tuple, Is>, __VA_ARGS__>) && ...); \
  }(yw::make_indices_for<Tuple>{})

namespace _ {
template<nat I, nat... Is, nat... Js, nat... Ks, typename F, typename... Ts> //
constexpr decltype(auto) _apply_i(sequence<Is...>, sequence<Js...>, sequence<Ks...>, F&& f, Ts&&... ts) {
  return _apply<I>(f, select<Is>(fwd<Ts>(ts)...)..., get<Js>(fwd<select_type<I, Ts...>>(select<I>(fwd<Ts>(ts)...)))..., select<Ks>(fwd<Ts>(ts)...)...);
}
template<nat I, typename F, typename... Ts> constexpr decltype(auto) _apply(F&& f, Ts&&... ts) {
  if constexpr (sizeof...(Ts) == 0) return invoke(f);
  if constexpr (I == sizeof...(Ts)) return invoke(f, fwd<Ts>(ts)...);
  else if constexpr (!tuple<select_type<I, Ts...>>) return _apply<I + 1>(f, fwd<Ts>(ts)...);
  else return _apply_i<I>(make_sequence<0, I>{}, make_indices_for<select_type<I, Ts...>>{}, make_sequence<I + 1, sizeof...(Ts)>{}, f, fwd<Ts>(ts)...);
}

template<nat I, typename F, typename... Ts> constexpr decltype(auto) _vapply_i(F&& f, Ts&&... ts) ywlib_wrap_auto(invoke(fwd<F>(f), get<I>(fwd<Ts>(ts))...));
template<nat... Is, typename F, typename... Ts> constexpr decltype(auto) _vapply_is(sequence<Is...>, F&& f, Ts&&... ts) //
  ywlib_wrap_auto(((_vapply_i<Is>(fwd<F>(f), fwd<Ts>(ts)...)), ...));
template<typename F, typename T, typename... Ts> requires same_extent<T, Ts...> constexpr decltype(auto) _vapply(F&& f, T&& t, Ts&&... ts) //
  ywlib_wrap_auto(_vapply_is(make_indices_for<T>(), fwd<F>(f), fwd<T>(t), fwd<Ts>(ts)...));
template<typename F, typename T, typename... Ts> requires (!same_extent<T, Ts...> && tuple<T> && (tuple<Ts> && ...)) void _vapply(F&& f, T&& t, Ts&&... ts) = delete;
template<typename F, typename... Ts> requires (!(tuple<Ts> || ...)) void _vapply(F&& f, Ts&&... ts) = delete;
template<nat I, nat N, nat... Is, nat... Js, typename F, typename... Ts> constexpr decltype(auto) _vapply_b(sequence<Is...>, sequence<Js...>, F&& f, Ts&&... ts) //
  ywlib_wrap_auto(_vapply(fwd<F>(f), select<Is>(fwd<Ts>(ts)...)..., tuple_view(select<I>(fwd<Ts>(ts)...), make_sequence<0, N>{}), select<Js>(fwd<Ts>(ts)...)...));
template<nat I, nat J, typename F, typename... Ts> constexpr decltype(auto) _vapply_a(F&& f, Ts&&... ts) //
  ywlib_wrap_auto(_vapply_b<I, extent<select_type<J, Ts...>>>(make_sequence<0, I>{}, make_sequence<I + 1, sizeof...(Ts)>{}, fwd<F>(f), fwd<Ts>(ts)...));
template<typename F, typename... Ts> requires (!(tuple<Ts> && ...) && (tuple<Ts> || ...)) constexpr decltype(auto) _vapply(F&& f, Ts&&... ts) //
  ywlib_wrap_auto(_vapply_a<inspects<!tuple<Ts>...>, inspects<tuple<Ts>...>>(fwd<F>(f), fwd<Ts>(ts)...));

template<typename T, typename U, typename V> struct list_from_typepack;
template<typename U, typename V> struct list_from_typepack<typepack<>, U, V> : _type<list<>> {};
template<typename... Ts, typename U, template<typename...> typename Tm, typename... Vs> //
struct list_from_typepack<typepack<Ts...>, U, Tm<Vs...>> : _type<list<copy_cvref<U, Tm<Ts>>...>> {};
template<typename... Ts, typename U, template<typename, auto...> typename Tm, typename V, auto... Vs> //
struct list_from_typepack<typepack<Ts...>, U, Tm<V, Vs...>> : _type<list<copy_cvref<U, Tm<Ts, Vs...>>...>> {};
template<typename... Ts, typename U, typename V> struct list_from_typepack<typepack<Ts...>, U, V> : _type<list<copy_cvref<U, Ts>...>> {};

template<typename T> struct _iter_t {
  using v = std::iter_value_t<T>;
  using d = std::iter_difference_t<T>;
  using r = std::iter_reference_t<T>;
  using rr = std::iter_rvalue_reference_t<T>;
};
template<std::ranges::range Rg> struct _iter_t<Rg> : _iter_t<std::ranges::iterator_t<Rg>> {};

inline constexpr cat1 vtos_codes[36]{
  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', //
  'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',                     //
};

constexpr fat8 _ftos_log10(fat8 v) noexcept {
  int8 t = bitcast<int8>(v);
  int8 exp = ((t & 0x7ff0000000000000) >> 52) - 1023;
  t = (t & 0x000fffffffffffff) | 0x3ff0000000000000;
  return exp * log10_2 - 0.652998017485824 + [&](const fat8 x) noexcept { //
    return x * (0.924793176223418 - 0.319987904734089 * x + 0.0480424904500012 * x * x);
  }(bitcast<fat8>(t));
}
}

inline constexpr auto apply = []<typename F, typename... Ts>(F f, Ts&&... ts) ywlib_wrap_ref(_::_apply<0>(fwd<F>(f), fwd<Ts>(ts)...));
template<typename F, typename... Ts> concept applyable = requires(F f, Ts&&... ts) { apply(f, fwd<Ts>(ts)...); };
template<typename F, typename... Ts> using apply_result = decltype(apply(declval<F&>(), declval<Ts>()...));

template<typename T> inline constexpr auto build = []<typename Tp>(Tp&& Tuple) ywlib_wrap_auto(apply(construct<T>, fwd<Tp>(Tuple)));
template<typename T, typename Tp> concept buildable = requires { build<T>(declval<Tp>()); };

inline constexpr auto vapply = []<typename F, typename... Ts>(F&& f, Ts&&... ts) ywlib_wrap_void(_::_vapply(fwd<F>(f), fwd<Ts>(ts)...));
template<typename T, typename... Ts> concept vapplyable = requires { vapply(declval<T>(), declval<Ts>()...); };

inline constexpr auto vassign = []<typename Lt, typename Rt>(Lt&& Lhs, Rt&& Rhs) ywlib_wrap_void(vapply(assign, fwd<Lt>(Lhs), fwd<Rt>(Rhs)));
template<typename Lt, typename Rt> concept vassignable = vapplyable<decltype(vassign), Lt, Rt>;

template<typename... Ts> using list_base = typepack<Ts...>::template fore<sizeof...(Ts) - 1>::template expand<list>;

template<typename... Ts> struct list : list_base<Ts...> {
  static constexpr nat count = sizeof...(Ts);
  using last_type = select_type<sizeof...(Ts) - 1, Ts...>;
  select_type<sizeof...(Ts) - 1, Ts...> last;
  template<nat I> requires (I < sizeof...(Ts)) constexpr auto get() & noexcept -> select_type<I, Ts...>& {
    if constexpr (I == sizeof...(Ts) - 1) return last;
    else return list_base<Ts...>::template get<I>();
  }
  template<nat I> requires (I < sizeof...(Ts)) constexpr auto get() const& noexcept -> add_const<select_type<I, Ts...>&> {
    if constexpr (I == sizeof...(Ts) - 1) return last;
    else return list_base<Ts...>::template get<I>();
  }
  template<nat I> requires (I < sizeof...(Ts)) constexpr auto get() && noexcept -> select_type<I, Ts...>&& {
    if constexpr (I == sizeof...(Ts) - 1) return mv(last);
    else return static_cast<list_base<Ts...>&&>(*this).template get<I>();
  }
  template<nat I> requires (I < sizeof...(Ts)) constexpr auto get() const&& noexcept -> add_const<select_type<I, Ts...>&&> {
    if constexpr (I == sizeof...(Ts) - 1) return mv(last);
    else return static_cast<list_base<Ts...>&&>(*this).template get<I>();
  }
  template<typename A> constexpr list& operator=(A&& Arg) requires vassignable<list&, A> { return vassign(*this, fwd<A>(Arg)), *this; }
  template<typename A> constexpr const list& operator=(A&& Arg) const requires vassignable<const list&, A> { return vassign(*this, fwd<A>(Arg)), *this; }
};

template<typename T1, typename T2, typename T3> struct list<T1, T2, T3> : list<T1, T2> {
  static constexpr nat count = 3;
  using third_type = T3;
  third_type third;
  template<nat I> requires (I < 3) constexpr auto get() & noexcept -> select_type<I, T1, T2, T3>& {
    if constexpr (I == 3) return third;
    else return list<T1, T2>::template get<I>();
  }
  template<nat I> requires (I < 3) constexpr auto get() const& noexcept -> add_const<select_type<I, T1, T2, T3>&> {
    if constexpr (I == 3) return third;
    else return list<T1, T2>::template get<I>();
  }
  template<nat I> requires (I < 3) constexpr auto get() && noexcept -> select_type<I, T1, T2, T3>&& {
    if constexpr (I == 3) return mv(third);
    else return static_cast<list<T1, T2>&&>(*this).template get<I>();
  }
  template<nat I> requires (I < 3) constexpr auto get() const&& noexcept -> add_const<select_type<I, T1, T2, T3>&&> {
    if constexpr (I == 3) return mv(third);
    else return static_cast<list<T1, T2>&&>(*this).template get<I>();
  }
  template<typename A> constexpr list& operator=(A&& Arg) requires vassignable<list&, A> { return vassign(*this, fwd<A>(Arg)), *this; }
  template<typename A> constexpr const list& operator=(A&& Arg) const requires vassignable<const list&, A> { return vassign(*this, fwd<A>(Arg)), *this; }
};

template<typename T1, typename T2> struct list<T1, T2> : list<T1> {
  static constexpr nat count = 2;
  using second_type = T2;
  second_type second;
  template<nat I> requires (I < 2) constexpr auto get() & noexcept -> select_type<I, T1, T2>& {
    if constexpr (I == 2) return second;
    else return list<T1>::template get<I>();
  }
  template<nat I> requires (I < 2) constexpr auto get() const& noexcept -> add_const<select_type<I, T1, T2>&> {
    if constexpr (I == 2) return second;
    else return list<T1>::template get<I>();
  }
  template<nat I> requires (I < 2) constexpr auto get() && noexcept -> select_type<I, T1, T2>&& {
    if constexpr (I == 2) return mv(second);
    else return static_cast<list<T1>&&>(*this).template get<I>();
  }
  template<nat I> requires (I < 2) constexpr auto get() const&& noexcept -> add_const<select_type<I, T1, T2>&&> {
    if constexpr (I == 2) return mv(second);
    else return static_cast<list<T1>&&>(*this).template get<I>();
  }
  template<typename A> constexpr list& operator=(A&& Arg) requires vassignable<list&, A> { return vassign(*this, fwd<A>(Arg)), *this; }
  template<typename A> constexpr const list& operator=(A&& Arg) const requires vassignable<const list&, A> { return vassign(*this, fwd<A>(Arg)), *this; }
};

template<typename T> struct list<T> {
  static constexpr nat count = 1;
  using first_type = T;
  first_type first;
  template<nat I> requires (I < 1) constexpr auto get() & noexcept -> select_type<I, T>& {
    if constexpr (I == 0) return first;
    else return first;
  }
  template<nat I> requires (I < 1) constexpr auto get() const& noexcept -> add_const<select_type<I, T>&> {
    if constexpr (I == 0) return first;
    else return first;
  }
  template<nat I> requires (I < 1) constexpr auto get() && noexcept -> select_type<I, T>&& {
    if constexpr (I == 0) return mv(first);
    else return mv(first);
  }
  template<nat I> requires (I < 1) constexpr auto get() const&& noexcept -> add_const<select_type<I, T>&&> {
    if constexpr (I == 0) return mv(first);
    else return mv(first);
  }
  template<typename A> constexpr list& operator=(A&& Arg) requires vassignable<list&, A> { return vassign(*this, fwd<A>(Arg)), *this; }
  template<typename A> constexpr const list& operator=(A&& Arg) const requires vassignable<const list&, A> { return vassign(*this, fwd<A>(Arg)), *this; }
};

template<> struct list<> {
  static constexpr nat count = 0;
  template<typename... Ts> static constexpr auto asref(Ts&&... Args) noexcept { return list<Ts&&...>{fwd<Ts>(Args)...}; }
  template<specialization_of<typepack> Tp, typename Qualifier = none> using from_typepack = _::list_from_typepack<Tp, Qualifier, remove_cvref<Qualifier>>::type;
};

template<typename... Ts> list(Ts...) -> list<Ts...>;

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

template<character Ct> using string = std::basic_string<Ct>;
template<character Ct> using string_view = std::basic_string_view<Ct>;
template<character Ct, typename... Args> using format_string = std::basic_format_string<Ct, Args...>;
template<typename T, typename Ct = iter_value<T>> concept stringable = nt_convertible_to<T&, string_view<Ct>>;

inline constexpr auto to_nat = []<character Ct>(string_view<Ct> s) noexcept -> nat8 {
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
};

inline constexpr auto to_int = []<character Ct>(string_view<Ct> s) noexcept -> int8 {
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
};

inline constexpr auto to_fat = []<character Ct>(string_view<Ct> s) noexcept -> fat8 {
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
    } else return neg ? -v : v;
  }
};

template<character Ct> inline constexpr auto to_hex = []<integral T>(const T Val) noexcept -> string<Ct> {
  string<Ct> r(sizeof(T) * 2, '0');
  for (nat j = r.size(), n = natcast(Val); i < r.size(); ++i, n >>= 4) r[--j] = _::vtos_codes[n & 0xf];
  return r;
};

template<character Ct> inline constexpr auto to_string = []<scalar T>(T v) noexcept -> string<Ct> {
  if constexpr (is_none<T>) return string<Ct>(array<Ct, 5>{Ct('n'), Ct('o'), Ct('n'), Ct('e'), Ct{}});
  else if constexpr (is_nullptr<T>) return string<Ct>(16, '0');
  else if constexpr (is_pointer<T>) {
    if (!is_cev) return to_hex<Ct>(reinterpret_cast<nat>(Val));
    else return string<Ct>(16, '0');
  } else if constexpr (is_bool<T>) {
    if (Val) return string<Ct>(array<Ct, 5>{Ct('t'), Ct('r'), Ct('u'), Ct('e'), Ct{}});
    else return string<Ct>(array<Ct, 6>{Ct('f'), Ct('a'), Ct('l'), Ct('s'), Ct('e'), Ct{}});
  } else if constexpr (unsigned_integral<T>) {
    if (v == 0) return std::basic_string<Ct>(1, Ct('0'));
    Ct temp[20];
    nat it = 20;
    for (; v != 0; v /= 10) temp[--it] = Ct(v % 10 + '0');
    return std::basic_string<Ct>(temp + it, 20 - it);
  } else if constexpr (signed_integral<T>) {
    if (v == 0) return std::basic_string<Ct>(1, Ct('0'));
    const bool neg = v < 0;
    Ct temp[20];
    nat it = 20;
    for (; v != 0; v /= 10) temp[--it] = Ct(v % 10 + '0');
    if (neg) temp[--it] = Ct('-');
    return std::basic_string<Ct>(temp + it, 20 - it);
  } else if constexpr (floating_point<T>) {
    const auto bitn = bitcast<nat8>(v);
    if ((bitn & 0x7fffffffffffffff) == 0) { // plus/minus zero
      if (bitn == 0) return make_static_string<Ct>(U"0.");
      else return make_static_string<Ct>(U"-0.");
    } else if ((bitn & 0x7ff0000000000000) == 0x7ff0000000000000) { // inf or nan
      if (bitn == 0x7ff0000000000000) return make_static_string<Ct>(U"inf");
      else if (bitn == 0xfff0000000000000) return make_static_string<Ct>(U"-inf");
      else return make_static_string<Ct>(U"nan");
    } else { // normal number
      const bool neg = v < 0 ? (v = -v, true) : false;
      if (v > 1e+16) { // too large
        if (neg) return make_static_string<Ct>(U">1e+16");
        else return make_static_string<Ct>(U"<-1e+16");
      } else {
        string<Ct> s(20, Ct{});
        if (auto exp = int8(_ftos_log10(v)); exp < 0) {
          nat i{};
          if (neg) s[i++] = Ct('-');
          s[i++] = Ct('0'), s[i++] = Ct('.');
          for (nat j{i}; j < 20; ++j) v *= 10;
          nat n = static_cast<nat>(v);
          for (nat j{19}; i <= j; n /= 10) s[j--] = Ct(n % 10 + '0');
        } else {
          nat i{};
          if (neg) s[i++] = Ct('-');
          nat j = 20 - (exp + i + 2); // frac part length; integral part(exp + 1) + dot(1)
          for (nat j{i}; j < 20; ++j) v *= 10;
          for (nat j = 20 - (exp + 2 + i); j < 20;) nat n = static_cast<nat>(v);
        }
      }

      // length of result is less than 20
      string<Ct> s(20, Ct{});
      nat i{}, j{}, n = static_cast<nat>(v);
      // appends `-` if negative
      if (neg) s[i++] = Ct('-');
      // appends integer part
      for (nat w = n; w > 0; w /= 10) s[20 - ++j] = Ct(w % 10 + '0');
      if (j == 0) s[20 - ++j] = Ct('0');
      for (; j > 0;) s[i++] = s[20 - j--];
      // appends decimal point
      s[i++] = Ct('.');
      v = (v - static_cast<fat8>(n));
      for (nat k{i}; k < 20; ++k) v *= 10;
      n = static_cast<nat>(v);
      for (nat k{19}; i <= k; n /= 10) s[k--] = Ct(n % 10 + '0');
      auto dot = s.find(Ct('.'));
    }
  }
};

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

/// \brief converts a scalar value to a string
template<character Ct> inline constexpr auto to_str = []<scalar T>(const T Val) noexcept -> string<Ct> {
  else if constexpr (floating_point<T>) return _::_ftos<Ct>(Val); else if constexpr (signed_integral<T>) return _::_itos<Ct>(Val);
  else if constexpr (unsigned_integral<T>) return _::_ntos<Ct>(Val);
};

}

namespace std {

template<typename T> struct common_type<yw::none, T> : type_identity<yw::none> {};
template<typename T> struct common_type<T, yw::none> : type_identity<yw::none> {};

template<typename T> struct common_type<yw::value, T> : std::common_type<yw::fat8, T> {};
template<typename T> struct common_type<T, yw::value> : std::common_type<T, yw::fat8> {};

template<auto... Vs> struct tuple_size<yw::sequence<Vs...>> : integral_constant<nat, sizeof...(Vs)> {};
template<nat I, auto... Vs> struct tuple_element<I, yw::sequence<Vs...>> : type_identity<decltype(yw::sequence<Vs...>::template at<I>)> {};

template<typename... Ts> struct tuple_size<yw::typepack<Ts...>> : integral_constant<nat, sizeof...(Ts)> {};
template<nat I, typename... Ts> struct tuple_element<I, yw::typepack<Ts...>> : type_identity<typename yw::typepack<Ts...>::template at<I>> {};

template<typename T, typename Pj, typename Sq> struct tuple_size<yw::tuple_view<T, Pj, Sq>> : integral_constant<size_t, yw::tuple_view<T, Pj, Sq>::sequence::count> {};
template<size_t I, typename T, typename Pj, typename Sq> struct tuple_element<I, yw::tuple_view<T, Pj, Sq>> : type_identity<typename yw::tuple_view<T, Pj, Sq>::template type_at<I>> {};

template<typename... Ts> struct tuple_size<yw::list<Ts...>> : integral_constant<size_t, yw::list<Ts...>::count> {};
template<size_t I, typename... Ts> struct tuple_element<I, yw::list<Ts...>> : type_identity<yw::select_type<I, Ts...>> {};

template<typename T, nat N> requires (N != yw::npos) struct tuple_size<yw::array<T, N>> : integral_constant<size_t, N> {};
template<size_t I, typename T, nat N> requires (N != yw::npos) struct tuple_element<I, yw::array<T, N>> : type_identity<T> {};
}
