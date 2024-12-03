#pragma once
#include "module/.hpp"

#ifndef nat
#define nat size_t
#endif

#define ywlib_wrap_auto(...)                                                                 \
  noexcept(noexcept(__VA_ARGS__)) requires requires { __VA_ARGS__; } { return __VA_ARGS__; }
#define ywlib_wrap_void(...)                                                          \
  noexcept(noexcept(__VA_ARGS__)) requires requires { __VA_ARGS__; } { __VA_ARGS__; }
#define ywlib_wrap_ref(...)                                                                                  \
  noexcept(noexcept(__VA_ARGS__))->decltype(auto) requires requires { __VA_ARGS__; } { return __VA_ARGS__; }

namespace yw {

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
template<typename T, typename... Ts> concept nt_convertible_to = //
  convertible_to<T, Ts...> && (std::is_nothrow_convertible_v<T, Ts> && ...);
template<typename T, typename... Ts> concept castable_to = requires(T (&f)()) { ((static_cast<Ts>(f())) && ...); };
template<typename T, typename... Ts> concept nt_castable_to = //
  castable_to<T, Ts...> && requires(T (&f)() noexcept) { requires (noexcept(static_cast<Ts>(f())) && ...); };

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

template<typename... Fs> struct overload : public Fs... {
  using Fs::operator()...;
};
struct equal {
  template<typename T, typename U> constexpr bool operator()(T&& a, U&& b) const ywlib_wrap_auto(a == b)
};
struct not_equal {
  template<typename T, typename U> constexpr bool operator()(T&& a, U&& b) const ywlib_wrap_auto(a != b)
};
struct less {
  template<typename T, typename U> constexpr bool operator()(T&& a, U&& b) const ywlib_wrap_auto(a < b)
};
struct less_equal {
  template<typename T, typename U> constexpr bool operator()(T&& a, U&& b) const ywlib_wrap_auto(a <= b)
};
struct greater {
  template<typename T, typename U> constexpr bool operator()(T&& a, U&& b) const ywlib_wrap_auto(a > b)
};
struct greater_equal {
  template<typename T, typename U> constexpr bool operator()(T&& a, U&& b) const ywlib_wrap_auto(a >= b)
};

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
template<typename T> constexpr decltype(auto) mv(T&& Ref) noexcept { return static_cast<add_rvref<T>>(Ref); };
template<typename T> inline constexpr auto fwd = [](auto&& Ref) noexcept -> T&& { return static_cast<T&&>(Ref); };
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
  template<typename T> static constexpr nat j = //
    i<T> < sizeof...(Fs) ? i<T> : inspects<convertible_to<T, invoke_result<Fs>>...>;
  template<typename... As> static constexpr nat k = inspects<invocable<Fs, As...>...>;
public:
  using Fs::operator()...;
  template<typename T> requires (j<T> < sizeof...(Fs)) constexpr operator T() const //
    noexcept(noexcept(select_type<j<T>, Fs...>::operator()())) {
    return select_type<j<T>, Fs...>::operator()();
  }
  template<typename... As> requires (k<As...> < sizeof...(Fs)) //
  constexpr decltype(auto) operator()(As&&... Args) const      //
    noexcept(noexcept(select_type<k<As...>, Fs...>::operator()(fwd<As>(Args)...))) {
    return select_type<k<As...>, Fs...>::operator()(fwd<As>(Args)...);
  }
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
template<nat I, typename T> requires (strategy<I, T> != get_strategy::ungettable) //
decltype(auto) call(T&& Ref) noexcept(bool(strategy<I, T> & get_strategy::nothrow)) {
  constexpr auto s = strategy<I, T>;
  if constexpr (bool(s & get_strategy::itself)) return fwd<T>(Ref);
  else if constexpr (bool(s & get_strategy::array)) return fwd<T>(Ref)[I];
  else if constexpr (bool(s & get_strategy::member)) return fwd<T>(Ref).template get<I>();
  else return get<I>(fwd<T>(Ref));
}
}

template<typename T> concept tuple = bool(_::_get::strategy<0, T> & get_strategy::tuple);
template<typename T, nat I> concept gettable = _::_get::strategy<I, T> != get_strategy::ungettable;
template<typename T, nat I> concept nt_gettable = gettable<T, I> && bool(_::_get::strategy<I, T> & get_strategy::nothrow);
template<nat I> inline constexpr auto get = []<typename T>(T&& Ref) ywlib_wrap_ref(_::_get::call<I>(fwd<T>(Ref)));
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
template<typename T, nat N> class array;

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

template<nat Begin, nat End, auto Proj, nat... Vs> struct t_make_sequence : t_make_sequence<Begin + 1, End, Proj, Vs..., Proj(Begin)> {};
template<nat End, auto Proj, nat... Vs> struct t_make_sequence<End, End, Proj, Vs...> : t_type<sequence<Vs...>> {};
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
  template<sequence_of Sq> using append = append_sequence<sequence, Sq>;
  template<indices_for<sequence> Ix> using extract = extract_sequence<sequence, Ix>;
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
  template<tuple Tp> using append = append_typepack<typepack, Tp>;
  template<indices_for<typepack> Ix> using extract = extract_typepack<typepack, Ix>;
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

template<typename... Ts> struct tuple_size<yw::list<Ts...>> : integral_constant<nat, sizeof...(Ts)> {};
template<nat I, typename... Ts> struct tuple_element<I, yw::list<Ts...>> : type_identity<yw::select_type<I, Ts...>> {};

template<typename T, nat N> requires (N != yw::npos) struct tuple_size<yw::array<T, N>> : integral_constant<size_t, N> {};
template<size_t I, typename T, nat N> requires (N != yw::npos) struct tuple_element<I, yw::array<T, N>> : type_identity<T> {};

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

template<nat I, typename F, typename... Ts> constexpr void _vapply_i(F&& f, Ts&&... ts) ywlib_wrap_void(invoke(fwd<F>(f), get<I>(fwd<Ts>(ts))...));
template<nat... Is, typename F, typename... Ts> constexpr void _vapply_is(sequence<Is...>, F&& f, Ts&&... ts) //
  ywlib_wrap_void(((_vapply_i<Is>(fwd<F>(f), fwd<Ts>(ts)...)), ...));
template<typename F, typename T, typename... Ts> requires same_extent<T, Ts...> constexpr void _vapply(F&& f, T&& t, Ts&&... ts) //
  ywlib_wrap_void(_vapply_is(make_indices_for<T>{}, fwd<F>(f), fwd<T>(t), fwd<Ts>(ts)...));
template<typename F, typename T, typename... Ts> requires (!same_extent<T, Ts...> && tuple<T> && (tuple<Ts> && ...)) void _vapply(F&& f, T&& t, Ts&&... ts) = delete;
template<typename F, typename... Ts> requires (!(tuple<Ts> || ...)) void _vapply(F&& f, Ts&&... ts) = delete;
template<nat I, nat N, nat... Is, nat... Js, typename F, typename... Ts> constexpr void _vapply_b(sequence<Is...>, sequence<Js...>, F&& f, Ts&&... ts) //
  ywlib_wrap_void(_vapply(fwd<F>(f), select<Is>(fwd<Ts>(ts)...)..., tuple_view(select<I>(fwd<Ts>(ts)...), make_sequence<0, N>{}), select<Js>(fwd<Ts>(ts)...)...));
template<nat I, nat J, typename F, typename... Ts> constexpr void _vapply_a(F&& f, Ts&&... ts) //
  ywlib_wrap_void(_vapply_b<I, extent<select_type<J, Ts...>>>(make_sequence<0, I>{}, make_sequence<I + 1, sizeof...(Ts)>{}, fwd<F>(f), fwd<Ts>(ts)...));
template<typename F, typename... Ts> requires (!(tuple<Ts> && ...) && (tuple<Ts> || ...)) constexpr void _vapply(F&& f, Ts&&... ts) //
  ywlib_wrap_void(_vapply_a<inspects<!tuple<Ts>...>, inspects<tuple<Ts>...>>(fwd<F>(f), fwd<Ts>(ts)...));

template<nat I, typename L, typename R> constexpr void _vassign_i(L&& lhs, R&& rhs) ywlib_wrap_void(assign(get<I>(fwd<L>(lhs)), get<I>(fwd<R>(rhs))));
template<nat... Is, typename L, typename R> constexpr void _vassign(sequence<Is...>, L&& lhs, R&& rhs) //
  ywlib_wrap_void(((_vassign_i<Is>(fwd<L>(lhs), fwd<R>(rhs))), ...));

template<typename T, typename U, typename V> struct list_from_typepack;
template<typename U, typename V> struct list_from_typepack<typepack<>, U, V> : t_type<list<>> {};
template<typename... Ts, typename U, template<typename...> typename Tm, typename... Vs> //
struct list_from_typepack<typepack<Ts...>, U, Tm<Vs...>> : t_type<list<copy_cvref<U, Tm<Ts>>...>> {};
template<typename... Ts, typename U, template<typename, auto...> typename Tm, typename V, auto... Vs> //
struct list_from_typepack<typepack<Ts...>, U, Tm<V, Vs...>> : t_type<list<copy_cvref<U, Tm<Ts, Vs...>>...>> {};
template<typename... Ts, typename U, typename V> struct list_from_typepack<typepack<Ts...>, U, V> : t_type<list<copy_cvref<U, Ts>...>> {};

inline constexpr cat1 vtos_codes[36]{
  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', //
  'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',                     //
};
} // namespace _

inline constexpr auto apply = []<typename F, typename... Ts>(F f, Ts&&... ts) ywlib_wrap_ref(_::_apply<0>(fwd<F>(f), fwd<Ts>(ts)...));
template<typename F, typename... Ts> concept applyable = requires(F f, Ts&&... ts) { apply(f, fwd<Ts>(ts)...); };
template<typename F, typename... Ts> using apply_result = decltype(apply(declval<F&>(), declval<Ts>()...));

template<typename T> inline constexpr auto build = []<typename Tp>(Tp&& Tuple) ywlib_wrap_auto(apply(construct<T>, fwd<Tp>(Tuple)));
template<typename T, typename Tp> concept buildable = requires { build<T>(declval<Tp>()); };

inline constexpr auto vapply = []<typename F, typename... Ts>(F&& f, Ts&&... ts) ywlib_wrap_void(_::_vapply(fwd<F>(f), fwd<Ts>(ts)...));
template<typename T, typename... Ts> concept vapplyable = requires { vapply(declval<T>(), declval<Ts>()...); };

inline constexpr auto vassign = []<typename Lt, same_extent<Lt> Rt>(Lt&& Lhs, Rt&& Rhs) { _::_vassign(make_indices_for<Lt>{}, fwd<Lt>(Lhs), fwd<Rt>(Rhs)); };
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
    if constexpr (I == 2) return third;
    else return list<T1, T2>::template get<I>();
  }
  template<nat I> requires (I < 3) constexpr auto get() const& noexcept -> add_const<select_type<I, T1, T2, T3>&> {
    if constexpr (I == 2) return third;
    else return list<T1, T2>::template get<I>();
  }
  template<nat I> requires (I < 3) constexpr auto get() && noexcept -> select_type<I, T1, T2, T3>&& {
    if constexpr (I == 2) return mv(third);
    else return static_cast<list<T1, T2>&&>(*this).template get<I>();
  }
  template<nat I> requires (I < 3) constexpr auto get() const&& noexcept -> add_const<select_type<I, T1, T2, T3>&&> {
    if constexpr (I == 2) return mv(third);
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
    if constexpr (I == 1) return second;
    else return list<T1>::template get<I>();
  }
  template<nat I> requires (I < 2) constexpr auto get() const& noexcept -> add_const<select_type<I, T1, T2>&> {
    if constexpr (I == 1) return second;
    else return list<T1>::template get<I>();
  }
  template<nat I> requires (I < 2) constexpr auto get() && noexcept -> select_type<I, T1, T2>&& {
    if constexpr (I == 1) return mv(second);
    else return static_cast<list<T1>&&>(*this).template get<I>();
  }
  template<nat I> requires (I < 2) constexpr auto get() const&& noexcept -> add_const<select_type<I, T1, T2>&&> {
    if constexpr (I == 1) return mv(second);
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
}

namespace yw::_ {

template<typename T> struct _iter_t {
  using v = std::iter_value_t<T>;
  using d = std::iter_difference_t<T>;
  using r = std::iter_reference_t<T>;
  using rr = std::iter_rvalue_reference_t<T>;
};
template<std::ranges::range Rg> struct _iter_t<Rg> : _iter_t<std::ranges::iterator_t<Rg>> {};

auto get_zoned_time() { return std::chrono::zoned_time(std::chrono::current_zone(), std::chrono::system_clock::now()); }
}

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
inline constexpr auto iter_swap = []<iterator It, iterator Jt>(It&& i, Jt&& j) //
  ywlib_wrap_void(std::ranges::iter_swap(fwd<It>(i), fwd<Jt>(j)));

template<character Ct> using string = std::basic_string<Ct>;
template<character Ct> using string_view = std::basic_string_view<Ct>;
template<character Ct, typename... Args> using format_string = std::basic_format_string<Ct, Args...>;
template<typename T, typename Ct = iter_value<T>> concept stringable = nt_convertible_to<T&, string_view<Ct>>;

template<typename T, nat N = npos> class array {
public:
  T _[N]{};
  static constexpr nat count = N;
  using value_type = T;
  template<typename U> requires assignable<T&, const U&> //
  constexpr array& operator=(const U (&Other)[N]) {
    return std::ranges::copy(Other, _), *this;
  }
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
  static constexpr array fill(const T& v) {
    array a;
    std::ranges::fill(a._, v);
    return a;
  }
};

template<character Ct, nat N> constexpr string<Ct> to_string(const array<Ct, N>& a) { return string<Ct>(a.begin(), a.end()); }

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
static_assert(sizeof(array<int>) == sizeof(std::vector<int>));

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

inline constexpr auto strlen = []<stringable St>(St&& Str) -> nat {
  if constexpr (is_array<remove_ref<St>>) return extent<St> - 1;
  else if constexpr (is_pointer<remove_ref<St>>) return std::char_traits<iter_value<St>>::length(Str);
  else if constexpr (specialization_of<remove_cvref<St>, std::basic_string>) return Str.size();
  else if constexpr (specialization_of<remove_cvref<St>, std::basic_string_view>) return Str.size();
  else return string_view<iter_value<St>>(Str).size();
};

inline constexpr auto is_digit = []<character Ct>(const Ct c) noexcept { return (natcast(c) ^ 0x30) < 10; };

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

template<character Ct> inline constexpr auto to_hex = []<scalar T>(const T v) noexcept -> string<Ct> {
  if constexpr (is_pointer<T>) {
    string<Ct> r(16, '0');
    if (!is_cev)
      for (nat i = 16, n = natcast(v); 0 < i; n >>= 4) r[--i] = _::vtos_codes[n & 0xf];
    return r;
  } else {
    string<Ct> r(sizeof(T) * 2, '0');
    for (nat j = r.size(), n = natcast(v); 0 < j; n >>= 4) r[--j] = _::vtos_codes[n & 0xf];
    return r;
  }
};

template<typename Ct> constexpr string<Ct> to_string(is_none auto) noexcept { //
  return to_string(array{Ct('n'), Ct('o'), Ct('n'), Ct('e')});
}

template<typename Ct> constexpr string<Ct> to_string(is_nullptr auto) noexcept { return string<Ct>(16, '0'); }

template<typename Ct> constexpr string<Ct> to_string(is_pointer auto v) noexcept {
  if (is_cev) return string<Ct>(16, '0');
  else return to_hex<Ct>(reinterpret_cast<nat>(v));
}

template<typename Ct> constexpr string<Ct> to_string(integral auto v) noexcept {
  if constexpr (is_bool<decltype(v)>) {
    if (v) return to_string(array{Ct('t'), Ct('r'), Ct('u'), Ct('e')});
    else return to_string(array{Ct('f'), Ct('a'), Ct('l'), Ct('s'), Ct('e')});
  } else if constexpr (unsigned_integral<decltype(v)>) {
    if (v == 0) return string<Ct>(1, Ct('0'));
    Ct temp[20];
    nat it = 20;
    for (; v != 0; v /= 10) temp[--it] = Ct(v % 10 + '0');
    return string<Ct>(temp + it, 20 - it);
  } else {
    if (v == 0) return string<Ct>(1, Ct('0'));
    const bool neg = v < 0 ? v = -v, true : false;
    Ct temp[20];
    nat it = 20;
    for (; v != 0; v /= 10) temp[--it] = Ct(v % 10 + '0');
    if (neg) temp[--it] = Ct('-');
    return string<Ct>(temp + it, 20 - it);
  }
}

template<typename Ct> constexpr string<Ct> to_string(fat8 v) noexcept {
  const nat bitn = bitcast<nat8>(v);
  if ((bitn & 0x7fffffffffffffff) == 0) {
    if (bitn == 0) return to_string(array{Ct('0')});
    else return to_string(array{Ct('-'), Ct('0')});
  } else if ((bitn & 0x7ff0000000000000) == 0x7ff0000000000000) {
    if (bitn == 0x7ff0000000000000) return to_string(array{Ct('i'), Ct('n'), Ct('f')});
    else if (bitn == 0xfff0000000000000) return to_string(array{Ct('-'), Ct('i'), Ct('n'), Ct('f')});
    else return to_string(array{Ct('n'), Ct('a'), Ct('n')});
  } else {
    if (v > 1e+16) return to_string(array{Ct('>'), Ct('1'), Ct('e'), Ct('+'), Ct('1'), Ct('6')});
    else if (v < -1e+16) return to_string(array{Ct('<'), Ct('-'), Ct('1'), Ct('e'), Ct('+'), Ct('1'), Ct('6')});
    const auto ii = static_cast<int8>(v);
    auto s0 = to_string<Ct>(ii);
    if (ii < 0) v = static_cast<fat8>(ii) - v;
    else v -= static_cast<fat8>(ii);
    nat m = 20 - (s0.size() + 1);
    for (auto n(m); n > 0; --n) v *= 10;
    auto s1 = string<Ct>(m, '0');
    auto jj = std::abs(static_cast<int8>(v));
    for (; m > 0; jj /= 10) s1[--m] = Ct('0' + jj % 10);
    if (auto sr = std::ranges::search(s1, array<Ct, 5>::fill(Ct('9'))); !sr.empty()) {
      if (auto it = sr.begin(); it == s1.begin()) return ii < 0 ? to_string<Ct>(ii - 1) : to_string<Ct>(ii + 1);
      else s1.resize(it - s1.begin()), ++*--it;
    }
    if (auto sr = std::ranges::search(s1, array<Ct, 5>::fill(Ct('0'))); !sr.empty()) s1.resize(sr.begin() - s1.begin());
    if (s1.empty()) return s0;
    else return s0.push_back(Ct('.')), s0.append(s1), mv(s0);
  }
}

template<character Out> constexpr string<Out> cvt(stringable auto&& s) noexcept {
  using In = iter_value<decltype(s)>;
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
        else if (c < 0x10000) {
          *p++ = Out(0xe0 | (c >> 12));         //
          *p++ = Out(0x80 | ((c >> 6) & 0x3f)); //
          *p++ = Out(0x80 | (c & 0x3f));
        } else {
          *p++ = Out(0xf0 | (c >> 18));          //
          *p++ = Out(0x80 | ((c >> 12) & 0x3f)); //
          *p++ = Out(0x80 | ((c >> 6) & 0x3f));  //
          *p++ = Out(0x80 | (c & 0x3f));
        }
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
}

struct source {
  explicit source(const cat1*) = delete;
  const cat1* const file;
  const cat1* const func;
  const nat4 line, column;
  consteval source(const cat1* File = __builtin_FILE(), const cat1* Func = __builtin_FUNCTION(), //
                   nat4 Line = __builtin_LINE(), nat4 Column = __builtin_COLUMN()) noexcept
    : file(File), func(Func), line(Line), column(Column) {}
  template<character Ct> string<Ct> to_string() const {
    string_view<Ct> f(file);
    string<Ct> s0{}, s1 = yw::to_string<Ct>(line), s2 = yw::to_string<Ct>(column);
    s0.reserve(f.size() + s1.size() + s2.size() + 3);
    s0.append(f), s0.push_back(Ct('(')), s0.append(s1), s0.push_back(Ct(',')), s0.append(s2), s0.push_back(Ct(')'));
    return s0;
  }
  string<cat1> to_string() const { return to_string<cat1>(); }
};

struct date {
  int4 year{}, month{}, day{};
  date() : date(_::get_zoned_time().get_local_time()) {}
  date(numeric auto&& Year, numeric auto&& Month, numeric auto&& Day) noexcept //
    : year(int4(Year)), month(int4(Month)), day(int4(Day)) {}
  template<typename Clock, typename Duration> date(const std::chrono::time_point<Clock, Duration>& tp) {
    const auto ymd = std::chrono::year_month_day(std::chrono::floor<std::chrono::days>(tp));
    year = int4(ymd.year()), month = int4(nat4(ymd.month())), day = int4(nat4(ymd.day()));
  }
  template<character Ct> string<Ct> to_string() const {
    string<Ct> s(10, {});
    if constexpr (sizeof(Ct) == 4) {
      auto t = std::format("{:04d}-{:02d}-{:02d}", year, month, day);
      std::ranges::copy(t, s.data());
    } else if constexpr (sizeof(Ct) == 1) std::format_to(s.data(), "{:04d}-{:02d}-{:02d}", year, month, day);
    else if constexpr (sizeof(Ct) == 2) std::format_to(s.data(), L"{:04d}-{:02d}-{:02d}", year, month, day);
    return s;
  }
  string<cat1> to_string() const { return to_string<cat1>(); }
};

struct clock {
  int4 hour{}, minute{}, second{};
  clock() : clock(_::get_zoned_time().get_local_time()) {}
  clock(numeric auto&& Hour, numeric auto&& Minute, numeric auto&& Second) noexcept //
    : hour(int4(Hour)), minute(int4(Minute)), second(int4(Second)) {}
  template<typename Clock, typename Duration> clock(const std::chrono::time_point<Clock, Duration>& tp) {
    const std::chrono::hh_mm_ss hms(std::chrono::floor<std::chrono::seconds>(tp - std::chrono::floor<std::chrono::days>(tp)));
    hour = int4(hms.hours().count()), minute = int4(hms.minutes().count()), second = int4(hms.seconds().count());
  }
  template<character Ct> string<Ct> to_string() const {
    string<Ct> s(8, {});
    if constexpr (sizeof(Ct) == 4) {
      auto t = std::format("{:02d}:{:02d}:{:02d}", hour, minute, second);
      std::ranges::copy(t, s.data());
    } else if constexpr (sizeof(Ct) == 1) std::format_to(s.data(), "{:02d}:{:02d}:{:02d}", hour, minute, second);
    else if constexpr (sizeof(Ct) == 2) std::format_to(s.data(), L"{:02d}:{:02d}:{:02d}", hour, minute, second);
    return s;
  }
  string<cat1> to_string() const { return to_string<cat1>(); }
};

struct time {
  yw::date date;
  yw::clock clock;
  time() : time(_::get_zoned_time().get_local_time()) {}
  time(const yw::date& Date, const yw::clock& Clock) noexcept : date(Date), clock(Clock) {}
  template<typename Clock, typename Duration> time(const std::chrono::time_point<Clock, Duration>& tp) : date(tp), clock(tp) {}
  operator yw::date() const { return date; }
  operator yw::clock() const { return clock; }
  template<character Ct> string<Ct> to_string() const {
    string<Ct> s(19, {});
    if constexpr (sizeof(Ct) == 4) {
      auto t = std::format("{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}", //
                           date.year, date.month, date.day, clock.hour, clock.minute, clock.second);
      std::ranges::copy(t, s.data());
    } else if constexpr (sizeof(Ct) == 1)                                   //
      std::format_to(s.data(), "{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}", //
                     date.year, date.month, date.day, clock.hour, clock.minute, clock.second);
    else if constexpr (sizeof(Ct) == 2)                                      //
      std::format_to(s.data(), L"{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}", //
                     date.year, date.month, date.day, clock.hour, clock.minute, clock.second);
    return s;
  }
  string<cat1> to_string() const { return to_string<cat1>(); }
};

inline constexpr caster now{[] { return time{}; }};

using path = std::filesystem::path;

class file {
public:
  yw::path path{};
  file() = default;
  file(const yw::path& Path) : path(Path) {}
  file(yw::path&& Path) : path(mv(Path)) {}
  file& operator=(const yw::path& Path) { return path = Path, *this; }
  file& operator=(yw::path&& Path) { return path = mv(Path), *this; }
  bool exists() const { return std::filesystem::exists(path); }
  bool is_file() const { return std::filesystem::is_regular_file(path); }
  bool is_directory() const { return std::filesystem::is_directory(path); }
  nat size() const { return std::filesystem::file_size(path); }
  bool read(void* Data, nat Size) const {
    if (std::ifstream ifs(path, std::ios::binary); !ifs) return false;
    else return ifs.read(static_cast<char*>(Data), Size), nat(ifs.gcount()) == Size;
  }
  bool read(contiguous_range auto&& Range) const { return read(yw::data(Range), yw::size(Range)); }
  template<character Ct> string<Ct> read() const {
    const nat Size = size();
    string<Ct> s(Size / sizeof(Ct), {});
    if (std::ifstream ifs(path, std::ios::binary); ifs) ifs.read(s.data(), Size);
    return s;
  }
  string<cat1> read() const { return read<cat1>(); }
  bool write(const void* Data, nat Size) const {
    if (std::ofstream ofs(path, std::ios::binary); !ofs) return false;
    else return ofs.write(static_cast<const char*>(Data), Size), true;
  }
  bool write(contiguous_range auto&& Range) const { return write(yw::data(Range), yw::size(Range)); }
};

class logger {
protected:
  string<cat1> text{};
public:
  struct level_t {
    string_view<cat1> name;
    int value;
    friend bool operator==(const level_t& l, const level_t& r) noexcept { return l.value == r.value; }
    friend auto operator<=>(const level_t& l, const level_t& r) noexcept { return l.value <=> r.value; }
  };
  static constexpr level_t all{"all", 0};
  static constexpr level_t debug{"debug", 10};
  static constexpr level_t info{"info", 20};
  static constexpr level_t warning{"warn", 30};
  static constexpr level_t error{"error", 40};
  static constexpr level_t critical{"critical", 50};
  path path;
  level_t level = info;
  bool console = true;
  logger(const yw::path& File) noexcept : path(File) {}
  ~logger() noexcept {
    try {
      if (std::basic_ofstream<cat1> ofs(path, std::ios::app); ofs) ofs << text, text.clear();
      else std::cerr << "failed to open log file: " << path << std::endl;
    } catch (...) { std::cerr << "failed to write log file: " << path << std::endl; }
  }
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
  void operator()(const level_t& Level, stringable auto&& Text, const source& Source) noexcept {
    if (Level < level) return;
    if (level > debug) return operator()(Level, fwd<decltype(Text)>(Text));
    try {
      string<cat1> s = std::format("{} [{}] {}: {}\n", now(), Level.name, Source, cvt<cat1>(Text));
      if (console) std::cout << s;
      text += s;
    } catch (...) { std::cerr << "failed to write log" << std::endl; }
  }
  template<typename... Ts> void format( //
    const level_t& Level, const std::format_string<Ts...> Format, Ts&&... Args) noexcept {
    if (Level < level) return;
    try {
      string<cat1> s;
      if constexpr (sizeof...(Ts) == 0) s = std::format("{} [{}] {}\n", now(), Level.name, Format);
      else s = std::format("{} [{}] {}\n", now(), Level.name, std::format(Format, fwd<Ts>(Args)...));
      if (console) std::cout << s;
      text += s;
    } catch (...) { std::cerr << "failed to write log" << std::endl; }
  }
  template<typename... Ts> void format( //
    const level_t& Level, const std::wformat_string<Ts...> Format, Ts&&... Args) noexcept {
    if (Level < level) return;
    try {
      string<cat2> s;
      if constexpr (sizeof...(Ts) == 0) s = std::format(L"{} [{}] {}\n", now(), Level.name, Format);
      else s = std::format(L"{} [{}] {}\n", now(), cvt<cat2>(Level.name), std::format(Format, fwd<Ts>(Args)...));
      if (console) std::wcout << s;
      text += cvt<cat1>(s);
    } catch (...) { std::wcerr << "failed to write log" << std::endl; }
  }
  void flush() noexcept {
    try {
      if (std::basic_ofstream<cat1> ofs(path, std::ios::app); ofs) ofs << text, text.clear();
      else std::cerr << "failed to open log file: " << path << std::endl;
    } catch (...) { std::cerr << "failed to write log file: " << path << std::endl; }
  }
};
}

namespace std {

template<typename Ct> struct formatter<yw::source, Ct> : formatter<basic_string<Ct>, Ct> {
  auto format(const yw::source& s, auto& Ctx) const { return formatter<basic_string<Ct>, Ct>::format(s.to_string(), Ctx); }
};

template<typename Ct> struct formatter<yw::date, Ct> : formatter<basic_string<Ct>, Ct> {
  auto format(const yw::date& d, auto& Ctx) const { return formatter<basic_string<Ct>, Ct>::format(d.to_string(), Ctx); }
};

template<typename Ct> struct formatter<yw::clock, Ct> : formatter<basic_string<Ct>, Ct> {
  auto format(const yw::clock& c, auto& Ctx) const { return formatter<basic_string<Ct>, Ct>::format(c.to_string(), Ctx); }
};

template<typename Ct> struct formatter<yw::time, Ct> : formatter<basic_string<Ct>, Ct> {
  auto format(const yw::time& t, auto& Ctx) const { return formatter<basic_string<Ct>, Ct>::format(t.to_string(), Ctx); }
};

template<typename Ct> struct formatter<yw::file, Ct> : formatter<basic_string<Ct>, Ct> {
  auto format(const yw::file& f, auto& Ctx) const { return formatter<basic_string<Ct>, Ct>::format(f.path.string<Ct>(), Ctx); }
};
}

export namespace yw {

using xvector = __m128;
using xmatrix = array<xvector, 4>;

namespace _ {
template<value X, value Y, value Z, value W> struct _xv_constant {
  inline static const xvector f = _mm_set_ps(fat4(W), fat4(Z), fat4(Y), fat4(X));
  inline static const __m256d d = _mm256_set_pd(fat8(W), fat8(Z), fat8(Y), fat8(X));
  inline static const __m128i i = _mm_set_epi32(int4(static_cast<int8>(W) & 0xffffffff), int4(static_cast<int8>(Z) & 0xffffffff), //
                                                int4(static_cast<int8>(Y) & 0xffffffff), int4(static_cast<int8>(X) & 0xffffffff));
  operator add_const<xvector&>() const noexcept { return f; }
  operator add_const<__m128i&>() const noexcept { return i; }
  operator add_const<__m256d&>() const noexcept { return d; }
};
}

template<value X, value Y = X, value Z = Y, value W = Z> //
inline constexpr _::_xv_constant<X, Y, Z, W> xv_constant{};

inline constexpr auto xv_zero = xv_constant<0>;
inline constexpr auto xv_negzero = xv_constant<-0.0>;
inline constexpr auto xv_one = xv_constant<1>;
inline constexpr auto xv_negone = xv_constant<-1>;
inline constexpr auto xv_x = xv_constant<1, 0, 0, 0>;
inline constexpr auto xv_y = xv_constant<0, 1, 0, 0>;
inline constexpr auto xv_z = xv_constant<0, 0, 1, 0>;
inline constexpr auto xv_w = xv_constant<0, 0, 0, 1>;
inline constexpr auto xv_negx = xv_constant<-1, 0, 0, 0>;
inline constexpr auto xv_negy = xv_constant<0, -1, 0, 0>;
inline constexpr auto xv_negz = xv_constant<0, 0, -1, 0>;
inline constexpr auto xv_negw = xv_constant<0, 0, 0, -1>;

inline xvector xvload(const fat4* p) noexcept { return _mm_load_ps(p); }
inline xvector xvfill(const fat4 v) noexcept { return _mm_set1_ps(v); }
inline xvector xvset(const fat4 x, const fat4 y, const fat4 z, const fat4 w) noexcept { return _mm_set_ps(w, z, y, x); }
inline void xvstore(const xvector& a, fat4* p) noexcept { _mm_store_ps(p, a); }

template<nat To> requires (To < 4) xvector xvinsert(const xvector& v, const fat4 x) noexcept { //
  return _mm_castsi128_ps(_mm_insert_epi32<To>(_mm_castps_si128(v), bitcast<int4>(x)));
}

template<nat To, nat From, nat Zero = 0> requires (To < 4 && From < 4 && Zero < 16) //
xvector xvinsert(const xvector& a, const xvector& b) noexcept {
  return _mm_insert_ps(a, b, Zero | To << 4 | From << 6);
}

template<nat I> requires (I < 4) //
fat4 xvextract(const xvector& v) noexcept {
  if constexpr (I == 0) return _mm_cvtss_f32(v);
  else return bitcast<fat4>(_mm_extract_ps(v, I));
}

template<nat I> requires (I < 4) int4 xvextract(const __m128i& m) noexcept { return _mm_extract_epi32(m, I); }

template<nat Mask> requires (Mask < 16) //
xvector xvblend(const xvector& a, const xvector& b) noexcept {
  return _mm_blend_ps(a, b, Mask);
}

template<bool X, bool Y, bool Z, bool W> //
xvector xvblend(const xvector& a, const xvector& b) noexcept {
  return _mm_blend_ps(a, b, X | Y << 1 | Z << 2 | W << 3);
}
}

namespace yw::_ {

template<int X, int Y, int Z, int W> xvector _xvpermute(const xvector& a) noexcept {
  constexpr auto f = [](auto i, auto j) { return i < 0 || i == j || 4 <= i; };
  constexpr auto x = f(X, 0) ? 0 : X, y = f(Y, 1) ? 1 : Y, z = f(Z, 2) ? 2 : Z, w = f(W, 3) ? 3 : W;
  if constexpr (f(X, 0) && f(Y, 1) && f(Z, 2) && f(W, 3)) return a;
  else if constexpr (f(X, 0) && f(Y, 0) && f(Z, 0) && f(W, 0)) return _mm_broadcastss_ps(a);
  else if constexpr (f(x, 0) && f(y, 0) && f(z, 2) && f(w, 2)) return _mm_moveldup_ps(a);
  else if constexpr (f(x, 1) && f(y, 1) && f(z, 3) && f(w, 3)) return _mm_movehdup_ps(a);
  else return _mm_permute_ps(a, x | y << 2 | z << 4 | w << 6);
}

template<int X, int Y, int Z, int W> xvector _xvpermute(const xvector& a, const xvector& b) noexcept {
  constexpr bool bx = X < 0, by = Y < 0, bz = Z < 0, bw = W < 0;
  if constexpr (X < 4 && Y < 4 && Z < 4 && W < 4) {
    return _xvpermute<X, Y, Z, W>(a);
  } else if constexpr ((bx || X >= 4) && (by || Y >= 4) && (bz || Z >= 4) && (bw || W >= 4)) {
    return _xvpermute<X - 4, Y - 4, Z - 4, W - 4>(b);
  } else if constexpr ((bx || !(X & 3 ^ 0)) && (by || !(Y & 3 ^ 1)) && (bz || !(Z & 3 ^ 2)) && (bw || !(W & 3 ^ 3))) {
    return xvblend<ge(X, 4), ge(Y, 4), ge(Z, 4), ge(W, 4)>(a, b);
  } else if constexpr (X < 4 && Y < 4 && (bz || Z >= 4) && (bw || W >= 4)) {
    constexpr auto x = bx ? 0 : X, y = by ? 1 : Y, z = bz ? 2 : Z - 4, w = bw ? 3 : W - 4;
    return _mm_shuffle_ps(a, b, x | y << 2 | z << 4 | w << 6);
  } else if constexpr ((bx || X >= 4) && (by || Y >= 4) && Z < 4 && W < 4) {
    constexpr auto x = bx ? 0 : X - 4, y = by ? 1 : Y - 4, z = f(Z) ? 2 : Z, w = f(W) ? 3 : W;
    return _mm_shuffle_ps(b, a, x | y << 2 | z << 4 | w << 6);
  } else if constexpr ((bx || X == 0) + (by || Y == 1) + (f(Z) || Z == 2) + (f(W) || W == 3) == 3) {
    constexpr auto x = bx ? 0 : X, y = by ? 1 : Y, z = f(Z) ? 2 : Z, w = f(W) ? 3 : W;
    constexpr nat i = inspects<ge(x, 4), ge(y, 4), ge(z, 4), ge(w, 4)>;
    return xvinsert<i, int(select_value<i, x, y, z, w> - 4)>(a, b);
  } else if constexpr ((bx || X == 4) + (by || Y == 5) + (f(Z) || Z == 6) + (f(W) || W == 7) == 3) {
    constexpr auto x = bx ? 4 : X, y = by ? 5 : Y, z = f(Z) ? 6 : Z, w = f(W) ? 7 : W;
    constexpr nat i = inspects<lt(x, 4), lt(y, 4), lt(z, 4), lt(w, 4)>;
    return xvinsert<i, select_value<i, x, y, z, w>>(b, a);
  } else if constexpr ((X < 4 || X == 4) && (Y < 4 || Y == 5) && (Z < 4 || Z == 6) && (W < 4 || W == 7)) {
    return xvblend<ge(X, 4), ge(Y, 4), ge(Z, 4), ge(W, 4)>(_xvpermute<X, Y, Z, W>(a), b);
  } else if constexpr ((bx || X >= 4 || X == 0) && (by || Y >= 4 || Y == 1) && (bz || Z >= 4 || Z == 2) && (bw || W >= 4 || W == 3)) {
    return xvblend<ge(X, 4), ge(Y, 4), ge(Z, 4), ge(W, 4)>(a, _xvpermute<X - 4, Y - 4, Z - 4, W - 4>(b));
  } else if constexpr ((0 <= X && X < 4) + (0 <= Y && Y < 4) + (0 <= Z && Z < 4) + (0 <= W && W < 4) == 1) {
    constexpr nat i = inspects<(0 <= X && X < 4), (0 <= Y && Y < 4), (0 <= Z && Z < 4), (0 <= W && W < 4)>;
    return xvinsert<i, select_value<i, X, Y, Z, W>>(_xvpermute<X - 4, Y - 4, Z - 4, W - 4>(b), a);
  } else if constexpr ((4 <= X) + (4 <= Y) + (4 <= Z) + (4 <= W) == 1) {
    constexpr nat i = inspects<(4 <= X), (4 <= Y), (4 <= Z), (4 <= W)>;
    return xvinsert<i, select_value<i, X, Y, Z, W> - 4>(_xvpermute<X, Y, Z, W>(a), b);
  } else return xvblend<ge(X, 4), ge(Y, 4), ge(Z, 4), ge(W, 4)>(_xvpermute<X, Y, Z, W>(a), _xvpermute<X - 4, Y - 4, Z - 4, W - 4>(b));
}

}

export namespace yw {

template<int X, int Y, int Z, int W> requires (X < 4 && Y < 4 && Z < 4 && W < 4) //
xvector xvpermute(const xvector& v) noexcept {
  return _::_xvpermute<X, Y, Z, W>(v);
}

template<int X, int Y, int Z, int W> requires (X < 8 && Y < 8 && Z < 8 && W < 8) //
xvector xvpermute(const xvector& a, const xvector& b) noexcept {
  return _::_xvpermute<X, Y, Z, W>(a, b);
}

inline xvector xvabs(const xvector& v) noexcept { return _mm_andnot_ps(xv_negzero, v); }
inline xvector xvneg(const xvector& a) noexcept { return _mm_xor_ps(a, xv_negzero); }

inline xvector xvadd(const xvector& a, const xvector& b) noexcept { return _mm_add_ps(a, b); }
inline xvector xvsub(const xvector& a, const xvector& b) noexcept { return _mm_sub_ps(a, b); }
inline xvector xvmul(const xvector& a, const xvector& b) noexcept { return _mm_mul_ps(a, b); }
inline xvector xvdiv(const xvector& a, const xvector& b) noexcept { return _mm_div_ps(a, b); }

inline xvector xvand(const xvector& a, const xvector& b) noexcept { return _mm_and_ps(a, b); }
inline xvector xvor(const xvector& a, const xvector& b) noexcept { return _mm_or_ps(a, b); }
inline xvector xvxor(const xvector& a, const xvector& b) noexcept { return _mm_xor_ps(a, b); }

inline xvector operator+(const xvector& a) noexcept { return a; }
inline xvector operator-(const xvector& a) noexcept { return xvneg(a); }
inline xvector operator+(const xvector& a, const xvector& b) noexcept { return xvadd(a, b); }
inline xvector operator-(const xvector& a, const xvector& b) noexcept { return xvsub(a, b); }
inline xvector operator*(const xvector& a, const xvector& b) noexcept { return xvmul(a, b); }
inline xvector operator/(const xvector& a, const xvector& b) noexcept { return xvdiv(a, b); }

inline bool operator==(const xvector& a, const xvector& b) noexcept { //
  return _mm_test_all_ones(_mm_castps_si128(_mm_cmpeq_ps(a, b)));
}
inline auto operator<=>(const xvector& a, const xvector& b) noexcept {      //
  return _mm_movemask_ps(_::_xvpermute<3, 2, 1, 0>(_mm_cmpgt_ps(a, b))) <=> //
         _mm_movemask_ps(_::_xvpermute<3, 2, 1, 0>(_mm_cmplt_ps(a, b)));
}

inline xvector xvceil(const xvector& a) noexcept { return _mm_ceil_ps(a); }
inline xvector xvfloor(const xvector& a) noexcept { return _mm_floor_ps(a); }
inline xvector xvround(const xvector& a) noexcept { return _mm_round_ps(a, 8); }
inline xvector xvtrunc(const xvector& a) noexcept { return _mm_trunc_ps(a); }

inline xvector xvmax(const xvector& a, const xvector& b) noexcept { return _mm_max_ps(a, b); }
inline xvector xvmin(const xvector& a, const xvector& b) noexcept { return _mm_min_ps(a, b); }

inline xvector xvfma(const xvector& a, const xvector& b, const xvector& c) noexcept { return _mm_fmadd_ps(a, b, c); }
inline xvector xvfms(const xvector& a, const xvector& b, const xvector& c) noexcept { return _mm_fmsub_ps(a, b, c); }
inline xvector xvfnma(const xvector& a, const xvector& b, const xvector& c) noexcept { return _mm_fnmadd_ps(a, b, c); }
inline xvector xvfnms(const xvector& a, const xvector& b, const xvector& c) noexcept { return _mm_fnmsub_ps(a, b, c); }

inline xvector xvsin(const xvector& a) noexcept { return _mm_sin_ps(a); }
inline xvector xvcos(const xvector& a) noexcept { return _mm_cos_ps(a); }
inline xvector xvtan(const xvector& a) noexcept { return _mm_tan_ps(a); }
inline xvector xvasin(const xvector& a) noexcept { return _mm_asin_ps(a); }
inline xvector xvacos(const xvector& a) noexcept { return _mm_acos_ps(a); }
inline xvector xvatan(const xvector& a) noexcept { return _mm_atan_ps(a); }
inline xvector xvatan(const xvector& y, const xvector& x) noexcept { return _mm_atan2_ps(y, x); }
inline xvector xvsincos(const xvector& a, xvector& Cos) noexcept { return _mm_sincos_ps(&Cos, a); }

inline xvector xvsinh(const xvector& a) noexcept { return _mm_sinh_ps(a); }
inline xvector xvcosh(const xvector& a) noexcept { return _mm_cosh_ps(a); }
inline xvector xvtanh(const xvector& a) noexcept { return _mm_tanh_ps(a); }
inline xvector xvasinh(const xvector& a) noexcept { return _mm_asinh_ps(a); }
inline xvector xvacosh(const xvector& a) noexcept { return _mm_acosh_ps(a); }
inline xvector xvatanh(const xvector& a) noexcept { return _mm_atanh_ps(a); }

inline xvector xvpow(const xvector& Base, const xvector& Exp) noexcept { return _mm_pow_ps(Base, Exp); }
inline xvector xvexp(const xvector& a) noexcept { return _mm_exp_ps(a); }
inline xvector xvexp2(const xvector& a) noexcept { return _mm_exp2_ps(a); }
inline xvector xvexp10(const xvector& a) noexcept { return _mm_exp10_ps(a); }
inline xvector xvexpm1(const xvector& a) noexcept { return _mm_expm1_ps(a); }
inline xvector xvsqrt(const xvector& a) noexcept { return _mm_sqrt_ps(a); }
inline xvector xvcbrt(const xvector& a) noexcept { return _mm_cbrt_ps(a); }
inline xvector xvsqrt_r(const xvector& a) noexcept { return _mm_invsqrt_ps(a); }
inline xvector xvcbrt_r(const xvector& a) noexcept { return _mm_invcbrt_ps(a); }
inline xvector xvhypot(const xvector& a, const xvector& b) noexcept { return _mm_hypot_ps(a, b); }

inline xvector xvln(const xvector& a) noexcept { return _mm_log_ps(a); }
inline xvector xvlog(const xvector& a, const xvector& Base) noexcept { return xvln(a) / xvln(Base); }
inline xvector xvlog2(const xvector& a) noexcept { return _mm_log2_ps(a); }
inline xvector xvlog10(const xvector& a) noexcept { return _mm_log10_ps(a); }
inline xvector xvlog1p(const xvector& a) noexcept { return _mm_log1p_ps(a); }
inline xvector xvlogb(const xvector& a) noexcept { return _mm_logb_ps(a); }

inline xvector xverf(const xvector& a) noexcept { return _mm_erf_ps(a); }
inline xvector xverfc(const xvector& a) noexcept { return _mm_erfc_ps(a); }
inline xvector xverf_r(const xvector& a) noexcept { return _mm_erfinv_ps(a); }
inline xvector xverfc_r(const xvector& a) noexcept { return _mm_erfcinv_ps(a); }

inline xvector xvcross(const xvector& a, const xvector& b) noexcept { //
  return xvfms(xvpermute<1, 2, 0, 3>(a), xvpermute<2, 0, 1, 3>(b),    //
               xvpermute<2, 0, 1, 3>(a) * xvpermute<1, 2, 0, 3>(b));
}

/// \brief dot product of two vectors
/// \tparam N dimension of the vectors
/// \tparam Zero zero mask of the result
/// \example `xvdot<2>(a, b)` returns `{c, c, c, c}` where `c = a.x * b.x + a.y * b.y`
/// \example `xvdot<3, 0b1010>(a, b)` returns `{c, 0, c, 0}` where `c = a.x * b.x + a.y * b.y + a.z * b.z`
template<nat N, nat Zero = 0> requires (1 <= N && N <= 4 && Zero < 16) //
xvector xvdot(const xvector& a, const xvector& b) noexcept {
  if constexpr (Zero == 15) return xv_zero;
  else return _mm_dp_ps(a, b, (Zero ^ 15) | ((1 << N) - 1) << 4);
}

template<nat N, tuple_for<xvector> Matrix, nat M = extent<Matrix>>                  //
requires (1 <= N && N <= 4 && !convertible_to<Matrix, xvector> && 1 <= M && M <= 4) //
xvector xvdot(Matrix&& m, const xvector& v) {
  auto r = xvdot<N, 0b1110>(get<0>(m), v);
  if constexpr (2 <= M) r = xvor(r, xvdot<N, 0b1101>(get<1>(m), v));
  if constexpr (3 <= M) r = xvor(r, xvdot<N, 0b1011>(get<2>(m), v));
  if constexpr (4 == M) r = xvor(r, xvdot<N, 0b0111>(get<3>(m), v));
  return r;
}

template<nat N, tuple_for<xvector> Matrix, nat M = extent<Matrix>>                  //
requires (1 <= N && N <= 4 && !convertible_to<Matrix, xvector> && 1 <= M && M <= 4) //
xvector xvdot(const xvector& v, Matrix&& m) {
  auto r = xvmul(xvpermute<0, 0, 0, 0>(v), get<0>(m));
  if constexpr (2 <= M) r = xvfma(xvpermute<1, 1, 1, 1>(v), get<1>(m), r);
  if constexpr (3 <= M) r = xvfma(xvpermute<2, 2, 2, 2>(v), get<2>(m), r);
  if constexpr (4 == M) r = xvfma(xvpermute<3, 3, 3, 3>(v), get<3>(m), r);
  return xvblend<(1 << N) - 1>(xv_zero, r);
}

template<nat N, tuple_for<xvector> Mt1, tuple_for<xvector> Mt2, nat L = extent<Mt1>, nat M = extent<Mt2>>         //
requires (1 <= N && N <= 4 && !convertible_to<Mt1, xvector> && !convertible_to<Mt2, xvector> && 1 <= M && M <= 4) //
void xvdot(const Mt1& m1, const Mt2& m2, Mt1& out) requires vassignable<Mt1&, const array<xvector, L>&> {
  [&]<nat... Is>(sequence<Is...>) { ((get<Is>(out) = xvdot<N>(get<Is>(m1), m2)), ...); }(make_sequence<0, L>{});
}

/// \brief converts a hex rgb color to a vector
/// \param Color hex rgb color: `0xAARRGGBB`
/// \return vector with components: `R / 255, G / 255, B / 255, A / 255`
inline xvector xvhex2rgb(const nat4 Color) noexcept { //
  auto a = _mm_cvtepi32_ps(_mm_cvtepu8_epi32(_mm_castps_si128(_mm_broadcast_ss((const fat4*)&Color))));
  return xvpermute<2, 1, 0, 3>(a * xv_constant<1 / 255.0>);
}

/// \brief converts a vector to a hex rgb color
/// \param v vector: `{R, G, B, A}`
/// \return hex rgb color: `0xAARRGGBB`
inline nat4 xvrgb2hex(const xvector& v) noexcept { //
  // multiply by 255 (value rage: 0-255)
  auto a = xvmul(v, xv_constant<255>);
  // fat4 to int4 (0x000000FF 0x000000FF 0x000000FF 0x000000FF)
  auto b = _mm_cvtps_epi32(a);
  // collect low 8 bits of each component
  auto c = _mm_shuffle_epi8(b, xv_constant<0x0c000408, 0>);
  return bitcast<nat4>(_mm_cvtsi128_si32(c));
}

template<typename T> struct vector {
  static constexpr nat count = 4;
  using value_type = T;
  T x{}, y{}, z{}, w{};
  constexpr vector() noexcept = default;
  explicit constexpr vector(const T& v) noexcept : x(v), y(v), z(v), w(v) {}
  constexpr vector(const T& X, const T& Y) noexcept : x(X), y(Y) {}
  constexpr vector(const T& X, const T& Y, const T& Z) noexcept : x(X), y(Y), z(Z) {}
  constexpr vector(const T& X, const T& Y, const T& Z, const T& W) noexcept : x(X), y(Y), z(Z), w(W) {}
  template<nat I> requires (I < count) constexpr T& get() & noexcept { return select<I>(x, y, z, w); }
  template<nat I> requires (I < count) constexpr const T& get() const& noexcept { return select<I>(x, y, z, w); }
  template<nat I> requires (I < count) constexpr T&& get() && noexcept { return mv(select<I>(x, y, z, w)); }
  template<nat I> requires (I < count) constexpr const T&& get() const&& noexcept { return mv(select<I>(x, y, z, w)); }
  constexpr nat size() const noexcept { return count; }
  T* data() noexcept { return &x; }
  const T* data() const noexcept { return &x; }
  T* begin() noexcept { return &x; }
  const T* begin() const noexcept { return &x; }
  T* end() noexcept { return &w + 1; }
  const T* end() const noexcept { return &w + 1; }
  constexpr T& operator[](nat i) {
    if (is_cev) {
      if (4 <= i) throw "index out of range";
      return i == 0 ? x : i == 1 ? y : i == 2 ? z : w;
    } else return data()[i];
  }
  constexpr const T& operator[](nat i) const {
    if (is_cev) {
      if (4 <= i) throw "index out of range";
      return i == 0 ? x : i == 1 ? y : i == 2 ? z : w;
    } else return data()[i];
  }
};

template<> struct vector<fat4> {
  static constexpr nat count = 4;
  using value_type = fat4;
  fat4 x{}, y{}, z{}, w{};
  constexpr vector() noexcept = default;
  explicit constexpr vector(const fat4 Fill) noexcept : x(Fill), y(Fill), z(Fill), w(Fill) {}
  explicit constexpr vector(numeric auto&& Fill) noexcept : vector(fat4(Fill)) {}
  constexpr vector(numeric auto&& X, numeric auto&& Y) noexcept : x(fat4(X)), y(fat4(Y)) {}
  constexpr vector(numeric auto&& X, numeric auto&& Y, numeric auto&& Z) noexcept : x(fat4(X)), y(fat4(Y)), z(fat4(Z)) {}
  constexpr vector(numeric auto&& X, numeric auto&& Y, numeric auto&& Z, numeric auto&& W) noexcept : x(fat4(X)), y(fat4(Y)), z(fat4(Z)), w(fat4(W)) {}
  vector(const xvector& Xv) noexcept { xvstore(Xv, &x); }
  operator xvector() const noexcept { return xvload(&x); }
  template<nat I> requires (I < count) constexpr fat4& get() & noexcept { return select<I>(x, y, z, w); }
  template<nat I> requires (I < count) constexpr const fat4& get() const& noexcept { return select<I>(x, y, z, w); }
  template<nat I> requires (I < count) constexpr fat4&& get() && noexcept { return mv(select<I>(x, y, z, w)); }
  template<nat I> requires (I < count) constexpr const fat4&& get() const&& noexcept { return mv(select<I>(x, y, z, w)); }
  constexpr nat size() const noexcept { return count; }
  fat4* data() noexcept { return &x; }
  const fat4* data() const noexcept { return &x; }
  fat4* begin() noexcept { return &x; }
  const fat4* begin() const noexcept { return &x; }
  fat4* end() noexcept { return &w + 1; }
  const fat4* end() const noexcept { return &w + 1; }
  constexpr fat4& operator[](nat i) {
    if (is_cev) {
      if (4 <= i) throw "index out of range";
      return i == 0 ? x : i == 1 ? y : i == 2 ? z : w;
    } else return data()[i];
  }
  constexpr const fat4& operator[](nat i) const {
    if (is_cev) {
      if (4 <= i) throw "index out of range";
      return i == 0 ? x : i == 1 ? y : i == 2 ? z : w;
    } else return data()[i];
  }
  friend constexpr bool operator==(const vector& a, const vector& b) noexcept {
    if (is_cev) return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
    else return xvector(a) == xvector(b);
  }
  friend constexpr auto operator<=>(const vector& a, const vector& b) noexcept {
    if (is_cev) {
      if (auto c = a.x <=> b.x; c != 0) return c;
      else if (c = a.y <=> b.y; c != 0) return c;
      else if (c = a.z <=> b.z; c != 0) return c;
      else return a.w <=> b.w;
    } else return std::partial_ordering(xvector(a) <=> xvector(b));
  }
  friend constexpr vector operator+(const vector& a) noexcept { return a; }
  friend constexpr vector operator-(const vector& a) noexcept {
    if (is_cev) return {-a.x, -a.y, -a.z, -a.w};
    else return xvneg(a);
  }
  friend constexpr vector operator+(const vector& a, const vector& b) noexcept {
    if (is_cev) return {a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w};
    else return xvadd(a, b);
  }
  friend constexpr vector operator-(const vector& a, const vector& b) noexcept {
    if (is_cev) return {a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w};
    else return xvsub(a, b);
  }
  friend constexpr vector operator*(const vector& a, const vector& b) noexcept {
    if (is_cev) return {a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w};
    else return xvmul(a, b);
  }
  friend constexpr vector operator/(const vector& a, const vector& b) noexcept {
    if (is_cev) return {a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w};
    else return xvdiv(a, b);
  }
  friend constexpr vector operator*(const vector& a, const fat4 b) noexcept {
    if (is_cev) return {a.x * b, a.y * b, a.z * b, a.w * b};
    else return xvmul(a, xvfill(b));
  }
  friend constexpr vector operator/(const vector& a, const fat4 b) noexcept {
    if (is_cev) return {a.x / b, a.y / b, a.z / b, a.w / b};
    else return xvmul(a, xvfill(1.0f / b));
  }
  friend constexpr vector operator*(numeric auto&& a, const vector& b) noexcept { return b * fat4(a); }
  friend constexpr vector operator*(const vector& a, numeric auto&& b) noexcept { return a * fat4(b); }
  friend constexpr vector operator/(const vector& a, numeric auto&& b) noexcept { return a / fat4(b); }
  constexpr vector abs() const noexcept {
    if (is_cev) return {std::fabs(x), std::fabs(y), std::fabs(z), std::fabs(w)};
    else return xvabs(*this);
  }
  fat4 length() const noexcept {
    if (is_cev) return std::sqrt(x * x + y * y + z * z + w * w);
    else return [](const xvector& v) { return xvextract<0>(xvsqrt(xvdot<4>(v, v))); }(*this);
  }
  fat4 length3() const noexcept {
    if (is_cev) return std::sqrt(x * x + y * y + z * z);
    else return [](const xvector& v) { return xvextract<0>(xvsqrt(xvdot<3>(v, v))); }(*this);
  }
  vector normalize() const noexcept {
    if (is_cev) return *this / length();
    else return [](const xvector& v) { return v * xvsqrt_r(xvdot<4>(v, v)); }(*this);
  }
  vector normalize3() const noexcept {
    if (is_cev) return *this / length3();
    else return [](const xvector& v) { return v * xvsqrt_r(xvdot<3>(v, v)); }(*this);
  }
};

template<typename... Ts> vector(Ts...) -> vector<fat4>;

////////////////////////////////////////////////////////////////////////////////
/// \brief RGBA color
struct color {

  /// \brief red component [0..1]
  fat4 r{};

  /// \brief green component [0..1]
  fat4 g{};

  /// \brief blue component [0..1]
  fat4 b{};

  /// \brief alpha component [0..1]; `0` is transparent
  fat4 a = 1.f;

  /// \brief default constructor
  constexpr color() noexcept = default;

  /// \brief constructs an untransparent color
  constexpr color(numeric auto&& R, numeric auto&& G, numeric auto&& B) noexcept : r(fat4(R)), g(fat4(G)), b(fat4(B)) {}

  /// \brief constructs a color with alpha
  constexpr color(numeric auto&& R, numeric auto&& G, numeric auto&& B, numeric auto&& A) noexcept //
    : r(fat4(R)), g(fat4(G)), b(fat4(B)), a(fat4(A)) {}

  /// \brief constructs an untransparent color from `0xRRGGBB`
  explicit constexpr color(nat4 xRRGGBB) noexcept { from_code(*this, xRRGGBB), a = 1.f; }

  /// \brief constructs a color from `0xRRGGBB` and alpha
  constexpr color(nat4 xRRGGBB, numeric auto&& Alpha) noexcept { from_code(*this, xRRGGBB), a = fat4(Alpha); }

  /// \brief converts the color to `0xAARRGGBB`
  explicit constexpr operator nat4() const noexcept {
    if (is_cev) return (nat4)(r * 255.f) << 16 | (nat4)(g * 255.f) << 8 | (nat4)(b * 255.f) | (nat4)(a * 255.f) << 24;
    else return xvrgb2hex(*this);
  }

  /// \brief constructs a color from `xvector`
  explicit color(const xvector& Xv) noexcept { _mm_storeu_ps(&r, Xv); }

  /// \brief converts the color to `xvector`
  operator xvector() const noexcept { return xvload(&r); }

  /// \brief checks if two colors are equal
  friend constexpr bool operator==(const color& A, const color& B) noexcept {
    if (!is_cev) return xvector(A) == xvector(B);
    else return A.r == B.r && A.g == B.g && A.b == B.b && A.a == B.a;
  }

  /// \brief `get` function
  template<nat I> requires (I < 4) constexpr fat4& get() noexcept { return select<I>(r, g, b, a); }

  /// \brief `get` function
  template<nat I> requires (I < 4) constexpr fat4 get() const noexcept { return select<I>(r, g, b, a); }

  static const color black, dimgray, gray, darkgray, silver, lightgray, gainsboro, whitesmoke, white, snow, ghostwhite,            //
    floralwhite, linen, antiquewhite, papayawhip, blanchedalmond, bisque, moccasin, navajowhite, peachpuff, mistyrose,             //
    lavenderblush, seashell, oldlace, ivory, honeydew, mintcream, azure, aliceblue, lavender, lightsteelblue, lightslategray,      //
    slategray, steelblue, royalblue, midnightblue, navy, darkblue, mediumblue, blue, dodgerblue, cornflowerblue, deepskyblue,      //
    lightskyblue, skyblue, lightblue, powderblue, paleturquoise, lightcyan, cyan, aqua, turquoise, mediumturquoise, darkturquoise, //
    lightseagreen, cadetblue, darkcyan, teal, darkslategray, darkgreen, green, forestgreen, seagreen, mediumseagreen,              //
    mediumaquamarine, darkseagreen, aquamarine, palegreen, lightgreen, springgreen, mediumspringgreen, lawngreen, chartreuse,      //
    greenyellow, lime, limegreen, yellowgreen, darkolivegreen, olivedrab, olive, darkkhaki, palegoldenrod, cornsilk, beige,        //
    lightyellow, lightgoldenrodyellow, lemonchiffon, wheat, burlywood, tan, khaki, yellow, gold, orange, sandybrown, darkorange,   //
    goldenrod, peru, darkgoldenrod, chocolate, sienna, saddlebrown, maroon, darkred, brown, firebrick, indianred, rosybrown,       //
    darksalmon, lightcoral, salmon, lightsalmon, coral, tomato, orangered, red, crimson, mediumvioletred, deeppink, hotpink,       //
    palevioletred, pink, lightpink, thistle, magenta, fuchsia, violet, plum, orchid, mediumorchid, darkorchid, darkviolet,         //
    darkmagenta, purple, indigo, darkslateblue, blueviolet, mediumpurple, slateblue, mediumslateblue, transparent, undefined, yw;

private:
  static constexpr void from_code(color& c, nat4 a) noexcept {
    if (!is_cev) xvstore(xvhex2rgb(a), &c.r);
    else c = {((0xff0000 & a) >> 16) / 255.f, ((0xff00 & a) >> 8) / 255.f, (0xff & a) / 255.f, ((0xff000000 & a) >> 24) / 255.f};
  }
};

inline constexpr color color::black{0x000000};
inline constexpr color color::dimgray{0x696969};
inline constexpr color color::gray{0x808080};
inline constexpr color color::darkgray{0xa9a9a9};
inline constexpr color color::silver{0xc0c0c0};
inline constexpr color color::lightgray{0xd3d3d3};
inline constexpr color color::gainsboro{0xdcdcdc};
inline constexpr color color::whitesmoke{0xf5f5f5};
inline constexpr color color::white{0xffffff};
inline constexpr color color::snow{0xfffafa};
inline constexpr color color::ghostwhite{0xf8f8ff};
inline constexpr color color::floralwhite{0xfffaf0};
inline constexpr color color::linen{0xfaf0e6};
inline constexpr color color::antiquewhite{0xfaebd7};
inline constexpr color color::papayawhip{0xffefd5};
inline constexpr color color::blanchedalmond{0xffebcd};
inline constexpr color color::bisque{0xffe4c4};
inline constexpr color color::moccasin{0xffe4b5};
inline constexpr color color::navajowhite{0xffdead};
inline constexpr color color::peachpuff{0xffdab9};
inline constexpr color color::mistyrose{0xffe4e1};
inline constexpr color color::lavenderblush{0xfff0f5};
inline constexpr color color::seashell{0xfff5ee};
inline constexpr color color::oldlace{0xfdf5e6};
inline constexpr color color::ivory{0xfffff0};
inline constexpr color color::honeydew{0xf0fff0};
inline constexpr color color::mintcream{0xf5fffa};
inline constexpr color color::azure{0xf0ffff};
inline constexpr color color::aliceblue{0xf0f8ff};
inline constexpr color color::lavender{0xe6e6fa};
inline constexpr color color::lightsteelblue{0xb0c4de};
inline constexpr color color::lightslategray{0x778899};
inline constexpr color color::slategray{0x708090};
inline constexpr color color::steelblue{0x4682b4};
inline constexpr color color::royalblue{0x4169e1};
inline constexpr color color::midnightblue{0x191970};
inline constexpr color color::navy{0x000080};
inline constexpr color color::darkblue{0x00008b};
inline constexpr color color::mediumblue{0x0000cd};
inline constexpr color color::blue{0x0000ff};
inline constexpr color color::dodgerblue{0x1e90ff};
inline constexpr color color::cornflowerblue{0x6495ed};
inline constexpr color color::deepskyblue{0x00bfff};
inline constexpr color color::lightskyblue{0x87cefa};
inline constexpr color color::skyblue{0x87ceeb};
inline constexpr color color::lightblue{0xadd8e6};
inline constexpr color color::powderblue{0xb0e0e6};
inline constexpr color color::paleturquoise{0xafeeee};
inline constexpr color color::lightcyan{0xe0ffff};
inline constexpr color color::cyan{0x00ffff};
inline constexpr color color::aqua{0x00ffff};
inline constexpr color color::turquoise{0x40e0d0};
inline constexpr color color::mediumturquoise{0x48d1cc};
inline constexpr color color::darkturquoise{0x00ced1};
inline constexpr color color::lightseagreen{0x20b2aa};
inline constexpr color color::cadetblue{0x5f9ea0};
inline constexpr color color::darkcyan{0x008b8b};
inline constexpr color color::teal{0x008080};
inline constexpr color color::darkslategray{0x2f4f4f};
inline constexpr color color::darkgreen{0x006400};
inline constexpr color color::green{0x008000};
inline constexpr color color::forestgreen{0x228b22};
inline constexpr color color::seagreen{0x2e8b57};
inline constexpr color color::mediumseagreen{0x3cb371};
inline constexpr color color::mediumaquamarine{0x66cdaa};
inline constexpr color color::darkseagreen{0x8fbc8f};
inline constexpr color color::aquamarine{0x7fffd4};
inline constexpr color color::palegreen{0x98fb98};
inline constexpr color color::lightgreen{0x90ee90};
inline constexpr color color::springgreen{0x00ff7f};
inline constexpr color color::mediumspringgreen{0x00fa9a};
inline constexpr color color::lawngreen{0x7cfc00};
inline constexpr color color::chartreuse{0x7fff00};
inline constexpr color color::greenyellow{0xadff2f};
inline constexpr color color::lime{0x00ff00};
inline constexpr color color::limegreen{0x32cd32};
inline constexpr color color::yellowgreen{0x9acd32};
inline constexpr color color::darkolivegreen{0x556b2f};
inline constexpr color color::olivedrab{0x6b8e23};
inline constexpr color color::olive{0x808000};
inline constexpr color color::darkkhaki{0xbdb76b};
inline constexpr color color::palegoldenrod{0xeee8aa};
inline constexpr color color::cornsilk{0xfff8dc};
inline constexpr color color::beige{0xf5f5dc};
inline constexpr color color::lightyellow{0xffffe0};
inline constexpr color color::lightgoldenrodyellow{0xfafad2};
inline constexpr color color::lemonchiffon{0xfffacd};
inline constexpr color color::wheat{0xf5deb3};
inline constexpr color color::burlywood{0xdeb887};
inline constexpr color color::tan{0xd2b48c};
inline constexpr color color::khaki{0xf0e68c};
inline constexpr color color::yellow{0xffff00};
inline constexpr color color::gold{0xffd700};
inline constexpr color color::orange{0xffa500};
inline constexpr color color::sandybrown{0xf4a460};
inline constexpr color color::darkorange{0xff8c00};
inline constexpr color color::goldenrod{0xdaa520};
inline constexpr color color::peru{0xcd853f};
inline constexpr color color::darkgoldenrod{0xb8860b};
inline constexpr color color::chocolate{0xd2691e};
inline constexpr color color::sienna{0xa0522d};
inline constexpr color color::saddlebrown{0x8b4513};
inline constexpr color color::maroon{0x800000};
inline constexpr color color::darkred{0x8b0000};
inline constexpr color color::brown{0xa52a2a};
inline constexpr color color::firebrick{0xb22222};
inline constexpr color color::indianred{0xcd5c5c};
inline constexpr color color::rosybrown{0xbc8f8f};
inline constexpr color color::darksalmon{0xe9967a};
inline constexpr color color::lightcoral{0xf08080};
inline constexpr color color::salmon{0xfa8072};
inline constexpr color color::lightsalmon{0xffa07a};
inline constexpr color color::coral{0xff7f50};
inline constexpr color color::tomato{0xff6347};
inline constexpr color color::orangered{0xff4500};
inline constexpr color color::red{0xff0000};
inline constexpr color color::crimson{0xdc143c};
inline constexpr color color::mediumvioletred{0xc71585};
inline constexpr color color::deeppink{0xff1493};
inline constexpr color color::hotpink{0xff69b4};
inline constexpr color color::palevioletred{0xdb7093};
inline constexpr color color::pink{0xffc0cb};
inline constexpr color color::lightpink{0xffb6c1};
inline constexpr color color::thistle{0xd8bfd8};
inline constexpr color color::magenta{0xff00ff};
inline constexpr color color::fuchsia{0xff00ff};
inline constexpr color color::violet{0xee82ee};
inline constexpr color color::plum{0xdda0dd};
inline constexpr color color::orchid{0xda70d6};
inline constexpr color color::mediumorchid{0xba55d3};
inline constexpr color color::darkorchid{0x9932cc};
inline constexpr color color::darkviolet{0x9400d3};
inline constexpr color color::darkmagenta{0x8b008b};
inline constexpr color color::purple{0x800080};
inline constexpr color color::indigo{0x4b0082};
inline constexpr color color::darkslateblue{0x483d8b};
inline constexpr color color::blueviolet{0x8a2be2};
inline constexpr color color::mediumpurple{0x9370db};
inline constexpr color color::slateblue{0x6a5acd};
inline constexpr color color::mediumslateblue{0x7b68ee};
inline constexpr color color::transparent = color(0x0, 0.0f);
inline constexpr color color::undefined = color(0x0, -1.f);
inline constexpr color color::yw{0x081020};

template<typename T> struct vector2 {
  T x{}, y{};
  constexpr vector2() noexcept = default;
  explicit constexpr vector2(const T& Val) noexcept : x(Val), y(Val) {}
  constexpr vector2(const T& X, const T& Y) noexcept : x(X), y(Y) {}
  template<convertible_to<T> U, convertible_to<T> V> constexpr vector2(U&& X, V&& Y) noexcept : x(fwd<U>(X)), y(fwd<V>(Y)) {}
  template<nat I> requires (I < 2) constexpr T& get() & noexcept { return select<I>(x, y); }
  template<nat I> requires (I < 2) constexpr T&& get() && noexcept { return mv(select<I>(x, y)); }
  template<nat I> requires (I < 2) constexpr const T& get() const& noexcept { return select<I>(x, y); }
  template<nat I> requires (I < 2) constexpr T get() const&& noexcept { return mv(select<I>(x, y)); }
};

template<typename T1, typename T2> vector2(T1, T2) -> vector2<fat4>;
}

namespace std {

template<typename T> struct tuple_size<yw::vector<T>> : integral_constant<nat, 4> {};
template<nat I, typename T> struct tuple_element<I, yw::vector<T>> : type_identity<T> {};

template<> struct tuple_size<yw::color> : integral_constant<nat, 4> {};
template<nat I> struct tuple_element<I, yw::color> : type_identity<float> {};

template<typename Ct> struct formatter<yw::color, Ct> : formatter<basic_string<Ct>, Ct> {
  static constexpr Ct fmt[] = {'R', 'G', 'B', 'A', '(', '{', '}', ',', ' ', '{', '}', ',', ' ', '{', '}', ',', ' ', '{', '}', ')', '\0'};
  auto format(const yw::color& c, auto& Ctx) const { return formatter<basic_string<Ct>, Ct>::format(std::format(fmt, c.r, c.g, c.b, c.a), Ctx); }
};

template<typename T> struct tuple_size<yw::vector2<T>> : integral_constant<nat, 2> {};
template<nat I, typename T> struct tuple_element<I, yw::vector2<T>> : type_identity<T> {};

}
