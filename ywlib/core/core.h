#pragma once
#include <algorithm>
#include <compare>
#include <concepts>
#include <contracts>
#include <cstdint>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <memory>
#include <meta>
#include <new>
#include <ranges>
#include <simd>
#include <source_location>
#include <text_encoding>
#include <type_traits>
#include <utility>

namespace yw {

static_assert(std::endian::native == std::endian::little, "YWLIB requires a little-endian architecture.");

#ifdef __cpp_lib_text_encoding
static_assert(
  std::text_encoding::literal() == std::text_encoding::id::UTF8, "YWLIB requires a UTF-8 execution character set.");
#endif

using int8_t = std::int8_t;
using int16_t = std::int16_t;
using int32_t = std::int32_t;
using int64_t = std::int64_t;

using uint8_t = std::uint8_t;
using uint16_t = std::uint16_t;
using uint32_t = std::uint32_t;
using uint64_t = std::uint64_t;

using size_t = std::size_t;
using ssize_t = std::make_signed_t<size_t>;

inline constexpr size_t npos = static_cast<size_t>(-1);
inline constexpr auto unordered = std::partial_ordering::unordered;
inline constexpr double nan = std::numeric_limits<double>::quiet_NaN();
inline constexpr double inf = std::numeric_limits<double>::infinity();
inline constexpr double pi = 3.14159265358979323846;
inline constexpr double pi2 = 6.28318530717958647692;
inline constexpr double euler = 2.71828182845904523536;
inline constexpr double sqrt2 = 1.41421356237309504880;
inline constexpr double sqrt3 = 1.73205080756887729352;
inline constexpr double sqrt5 = 2.23606797749978969640;
inline constexpr double ln2 = 0.69314718055994530942;
inline constexpr double ln3 = 1.09861228866810969139;
inline constexpr double ln5 = 1.60943791243410037460;

template<typename... Ts> inline constexpr bool always_false = false;

namespace internal { // clang-format off
template<typename... Ts> using _void = void;
template<typename T> struct _type { using type = T; };
template<typename T> struct _remove_const : _type<T> {};
template<typename T> struct _remove_const<const T> : _type<T> {};
template<typename T> struct _remove_volatile : _type<T> {};
template<typename T> struct _remove_volatile<volatile T> : _type<T> {};
template<typename T> struct _remove_ref : _type<T> {};
template<typename T> struct _remove_ref<T&> : _type<T> {};
template<typename T> struct _remove_ref<T&&> : _type<T> {};
template<typename T> struct _remove_pointer : _type<T> {};
template<typename T> struct _remove_pointer<T*> : _type<T> {};
template<typename T> struct _remove_pointer<T* const> : _type<T> {};
template<typename T> struct _remove_pointer<T* volatile> : _type<T> {};
template<typename T> struct _remove_pointer<T* const volatile> : _type<T> {};
template<typename T> struct _remove_extent : _type<T> {};
template<typename T> struct _remove_extent<T[]> : _type<T> {};
template<typename T, size_t N> struct _remove_extent<T[N]> : _type<T> {};
} // clang-format on

template<typename T> using remove_const = internal::_remove_const<T>::type;
template<typename T> using remove_volatile = internal::_remove_volatile<T>::type;
template<typename T> using remove_cv = remove_const<remove_volatile<T>>;
template<typename T> using remove_ref = internal::_remove_ref<T>::type;
template<typename T> using remove_cvref = remove_cv<remove_ref<T>>;
template<typename T> using remove_pointer = internal::_remove_pointer<T>::type;
template<typename T> using remove_extent = internal::_remove_extent<T>::type;

namespace internal { // clang-format off
template<typename T> struct _add_const : _type<const T> {};
template<typename T> struct _add_volatile : _type<volatile T> {};
template<typename T> struct _add_lvref : _type<T> {};
template<typename T> struct _add_rvref : _type<T> {};
template<typename T> struct _add_pointer : _type<T> {};
template<typename T, size_t N> struct _add_extent : _type<T> {};
template<typename T> requires requires { typename _void<T&>; } struct _add_lvref<T> : _type<T&> {};
template<typename T> requires requires { typename _void<T&&>; } struct _add_rvref<T> : _type<T&&> {};
template<typename T> requires requires { typename _void<T*>; } struct _add_pointer<T> : _type<T*> {};
template<typename T> requires requires { typename _void<T[]>; } struct _add_extent<T, 0> : _type<T[]> {};
template<typename T, size_t N> requires requires { typename _void<T[N]>; } struct _add_extent<T, N> : _type<T[N]> {};
} // clang-format on

template<typename T> using add_const = internal::_add_const<T>::type;
template<typename T> using add_volatile = internal::_add_volatile<T>::type;
template<typename T> using add_cv = add_const<add_volatile<T>>;
template<typename T> using add_lvref = internal::_add_lvref<T>::type;
template<typename T> using add_fwref = internal::_add_rvref<T>::type;
template<typename T> using add_rvref = internal::_add_rvref<remove_ref<T>>::type;
template<typename T> using add_pointer = internal::_add_pointer<T>::type;
template<typename T, size_t N = 0> using add_extent = internal::_add_extent<T, N>::type;

namespace internal { // clang-format off
template<typename T> struct _declval { static add_fwref<T> operator()() noexcept; };
} // clang-format on

template<typename T> inline constexpr internal::_declval<T> declval;

namespace internal { // clang-format off
template<typename T, typename U> inline constexpr bool __same_as = false;
template<typename T> inline constexpr bool __same_as<T, T> = true;
template<typename T, typename U> concept _same_as = __same_as<T, U>;
template<typename T, typename U> concept _castable_to = requires(T&& t) { static_cast<U>(static_cast<T&&>(t)); };
template<typename T, typename U> concept _convertible_to = requires(T&& t, int(&u)(U)) { u(static_cast<T&&>(t)); };
template<typename T, typename U> concept _base_of = std::is_base_of_v<T, U>;
template<typename T, typename U> concept _derived_from = _base_of<U, T> && _convertible_to<add_cv<T>*, add_cv<U>*>;
template<typename T, typename U> concept _nt_castable_to = _castable_to<T, U> && noexcept(static_cast<U>(declval<T>()));
template<typename T, typename U> concept _nt_convertible_to = _convertible_to<T, U> && _nt_castable_to<T, U>;
} // clang-format on

template<typename T, typename... Ts> concept any_of = (internal::_same_as<T, Ts> || ...);
template<typename T, typename... Ts> concept same_as = (internal::_same_as<T, Ts> && ...);
template<typename T, typename... Ts> concept castable_to = (internal::_castable_to<T, Ts> && ...);
template<typename T, typename... Ts> concept convertible_to = (internal::_convertible_to<T, Ts> && ...);
template<typename T, typename... Ts> concept base_of = (internal::_base_of<T, Ts> && ...);
template<typename T, typename... Ts> concept derived_from = (internal::_derived_from<T, Ts> && ...);
template<typename T, typename... Ts> concept nt_castable_to = (internal::_nt_castable_to<T, Ts> && ...);
template<typename T, typename... Ts> concept nt_convertible_to = (internal::_nt_convertible_to<T, Ts> && ...);

namespace internal { // clang-format off
template<typename T> inline constexpr bool _is_bounded_array = false;
template<typename T, size_t N> inline constexpr bool _is_bounded_array<T[N]> = true;
} // clang-format on

template<typename T> concept is_const = !same_as<T, remove_const<T>>;
template<typename T> concept is_volatile = !same_as<T, remove_volatile<T>>;
template<typename T> concept is_reference = !same_as<T, remove_ref<T>>;
template<typename T> concept is_lvref = is_reference<T> && same_as<T, add_lvref<T>>;
template<typename T> concept is_rvref = is_reference<T> && same_as<T, add_rvref<T>>;
template<typename T> concept is_pointer = !same_as<T, remove_pointer<T>>;
template<typename T> concept is_array = !same_as<T, remove_extent<T>>;
template<typename T> concept is_bounded_array = is_array<T> && internal::_is_bounded_array<T>;
template<typename T> concept is_unbounded_array = is_array<T> && !is_bounded_array<T>;
template<typename T> concept is_function = !is_const<const T> && !is_reference<T>;

struct none;

template<typename T> concept is_void = same_as<remove_cv<T>, void>;
template<typename T> concept is_none = same_as<remove_cv<T>, none>;
template<typename T> concept is_bool = same_as<remove_cv<T>, bool>;
template<typename T> concept is_nullptr = same_as<remove_cv<T>, decltype(nullptr)>;
template<typename T> concept char_type = any_of<remove_cv<T>, char, wchar_t, char8_t, char16_t, char32_t>;
template<typename T> concept int_type = any_of<remove_cv<T>, int8_t, int16_t, int32_t, int64_t, long>;
template<typename T> concept uint_type = any_of<remove_cv<T>, uint8_t, uint16_t, uint32_t, uint64_t, unsigned long>;
template<typename T> concept float_type = any_of<remove_cv<T>, float, double, long double>;
template<typename T> concept integral = is_bool<T> || char_type<T> || int_type<T> || uint_type<T>;
template<typename T> concept signed_integral = integral<T> && requires { requires T(-1) < T(0); };
template<typename T> concept unsigned_integral = integral<T> && !signed_integral<T>;
template<typename T> concept arithmetic = integral<T> || float_type<T>;
template<typename T> concept trivial = std::is_trivially_copyable_v<T>;
template<typename T> concept is_enum = std::is_enum_v<T>;
template<typename T> concept is_class = std::is_class_v<T>;
template<typename T> concept is_union = std::is_union_v<T>;
template<typename T> concept is_object = std::is_object_v<T>;
template<typename T> concept movable = std::movable<T>;
template<typename T> concept copyable = std::copyable<T>;
template<typename T> concept semiregular = std::semiregular<T>;
template<typename T> concept regular = std::regular<T>;

struct none {
  constexpr none(auto&&...) noexcept {}
  constexpr none& operator=(auto&&) noexcept { return *this; }
  explicit constexpr operator bool() const noexcept { return false; }
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
  template<char_type C> consteval const C* to_string() const noexcept {
    if constexpr (same_as<C, char>) return "none";
    else if constexpr (same_as<C, wchar_t>) return L"none";
    else if constexpr (same_as<C, char8_t>) return u8"none";
    else if constexpr (same_as<C, char16_t>) return u"none";
    else if constexpr (same_as<C, char32_t>) return U"none";
    else static_assert(always_false<C>, "unsupported character type");
  }
  consteval const char* to_string() const noexcept { return to_string<char>(); }
};

template<typename T, typename U = T> concept equality_comparable = std::equality_comparable_with<T, U>;
template<typename T, typename U = T> concept totally_ordered = std::totally_ordered_with<T, U>;
template<typename T, typename U = T> concept three_way_comparable = std::three_way_comparable_with<T, U>;

inline constexpr auto move = [](auto&& t) noexcept -> auto&& { return static_cast<remove_ref<decltype(t)>&&>(t); };
template<typename T> inline constexpr auto forward = [](auto&& t) noexcept -> T&& { return static_cast<T&&>(t); };
template<typename T> inline constexpr auto bitcast = [](const auto& u) noexcept { return std::bit_cast<T>(u); };

inline constexpr auto array_size = []<typename T, size_t N>(const T (&)[N]) noexcept { return N; };
inline constexpr auto count = [](is_bool auto... Bs) noexcept -> size_t { return (Bs + ...); };
inline constexpr auto inspect = [](is_bool auto... Bs) noexcept -> size_t {
  size_t counting = true, count = 0;
  ((count += counting * !Bs, counting *= !Bs), ...);
  return count;
};

// clang-format off

struct noop { template<typename... As> constexpr void operator()(As&&...) const noexcept {} };
struct pass { template<typename T> constexpr T&& operator()(T&& t) const noexcept { return static_cast<T&&>(t); } };

inline constexpr struct equal {
  template<typename T, equality_comparable<T> U> constexpr bool operator()(T&& t, U&& u) const
    noexcept(noexcept(declval<T>() == declval<U>())) { return static_cast<T&&>(t) == static_cast<U&&>(u); }
} eq;

inline constexpr struct not_equal {
  template<typename T, equality_comparable<T> U> constexpr bool operator()(T&& t, U&& u) const
    noexcept(noexcept(declval<T>() != declval<U>())) { return static_cast<T&&>(t) != static_cast<U&&>(u); }
} ne;

inline constexpr struct less {
  template<typename T, totally_ordered<T> U> constexpr bool operator()(T&& t, U&& u) const
    noexcept(noexcept(declval<T>() < declval<U>())) { return static_cast<T&&>(t) < static_cast<U&&>(u); }
} lt;

inline constexpr struct greater {
  template<typename T, totally_ordered<T> U> constexpr bool operator()(T&& t, U&& u) const
    noexcept(noexcept(declval<T>() > declval<U>())) { return static_cast<T&&>(t) > static_cast<U&&>(u); }
} gt;

inline constexpr struct less_equal {
  template<typename T, totally_ordered<T> U> constexpr bool operator()(T&& t, U&& u) const
    noexcept(noexcept(declval<T>() <= declval<U>())) { return static_cast<T&&>(t) <= static_cast<U&&>(u); }
} le;

inline constexpr struct greater_equal {
  template<typename T, totally_ordered<T> U> constexpr bool operator()(T&& t, U&& u) const
    noexcept(noexcept(declval<T>() >= declval<U>())) { return static_cast<T&&>(t) >= static_cast<U&&>(u); }
} ge;

inline constexpr struct three_way {
  template<typename T, three_way_comparable<T> U> constexpr auto operator()(T&& t, U&& u) const
    noexcept(noexcept(declval<T>() <=> declval<U>())) { return static_cast<T&&>(t) <=> static_cast<U&&>(u); }
} tw;

// clang-format on

inline constexpr auto int_cast = []<trivial T>(const T& value) noexcept {
  if constexpr (sizeof(T) == 1) return bitcast<int8_t>(value);
  else if constexpr (sizeof(T) == 2) return bitcast<int16_t>(value);
  else if constexpr (sizeof(T) == 4) return bitcast<int32_t>(value);
  else if constexpr (sizeof(T) == 8) return bitcast<int64_t>(value);
  else static_assert(always_false<T>, "unsupported type for int_cast");
};

inline constexpr auto uint_cast = []<trivial T>(const T& value) noexcept {
  if constexpr (sizeof(T) == 1) return bitcast<uint8_t>(value);
  else if constexpr (sizeof(T) == 2) return bitcast<uint16_t>(value);
  else if constexpr (sizeof(T) == 4) return bitcast<uint32_t>(value);
  else if constexpr (sizeof(T) == 8) return bitcast<uint64_t>(value);
  else static_assert(always_false<T>, "unsupported type for uint_cast");
};

namespace internal { // clang-format off
template<typename T> struct _member_traits { using class_type = void; using member_type = void; };
template<typename C, typename M> struct _member_traits<M C::*> { using class_type = C; using member_type = M; };
} // clang-format on

template<typename T> using member_type = internal::_member_traits<remove_cvref<T>>::member_type;
template<typename T> using class_type = internal::_member_traits<remove_cvref<T>>::class_type;
template<typename T> concept is_member_object_pointer = std::is_member_object_pointer_v<T>;
template<typename T> concept is_member_function_pointer = std::is_member_function_pointer_v<T>;
template<typename T> concept is_member_pointer = is_member_object_pointer<T> || is_member_function_pointer<T>;

template<typename T, typename... As> concept constructible = std::is_constructible_v<T, As...>;
template<typename T, typename... As> concept nt_constructible = std::is_nothrow_constructible_v<T, As...>;
template<typename T> inline constexpr auto construct = []<typename... As>(As&&... as) //
  noexcept(nt_constructible<T, As...>) requires constructible<T, As...> { return T{static_cast<As&&>(as)...}; };

template<typename T, typename U> concept assignable = std::is_assignable_v<T, U>;
template<typename T, typename U> concept nt_assignable = std::is_nothrow_assignable_v<T, U>;
inline constexpr auto assign = []<typename T, typename U>(T&& t, U&& u) //
  noexcept(nt_assignable<T, U>) requires assignable<T, U> { static_cast<T&&>(t) = static_cast<U&&>(u); };

// clang-format off

template<typename T, typename U = T> concept exchangeable =
  constructible<remove_ref<T>, add_rvref<T>> && assignable<T&, add_rvref<U>>;
template<typename T, typename U = T> concept nt_exchangeable =
  nt_constructible<remove_ref<T>, add_rvref<T>> && nt_assignable<T&, add_rvref<U>>;
inline constexpr auto exchange = []<typename T, typename U = T>(T&& t, U&& u) //
  noexcept(nt_exchangeable<T, U>) requires exchangeable<T, U> { auto tmp = move(t); t = move(u); return tmp; };

// clang-format on

template<typename T, typename U = T> concept swappable = std::swappable_with<T, U>;
template<typename T, typename U = T> concept nt_swappable = std::is_nothrow_swappable_with_v<T, U>;
inline constexpr auto swap = []<typename T, typename U = T>(T&& t, U&& u) noexcept(nt_swappable<T, U>) //
  requires swappable<T, U> { std::ranges::swap(static_cast<T&&>(t), static_cast<U&&>(u)); };

namespace internal { // clang-format off
template<typename T, template<typename...> typename Tm> inline constexpr bool _specialization_of = false;
template<template<typename...> typename Tm, typename... Ts>
inline constexpr bool _specialization_of<Tm<Ts...>, Tm> = true;
template<typename T, typename U> inline constexpr bool _variation_of = false;
template<template<typename...> typename Tm, typename... Ts, typename... Us>
inline constexpr bool _variation_of<Tm<Ts...>, Tm<Us...>> = true;
template<template<auto...> typename Tm, auto... Ts, auto... Us>
inline constexpr bool _variation_of<Tm<Ts...>, Tm<Us...>> = true;
template<template<typename, auto...> typename Tm, typename T, auto... Ts, typename U, auto... Us>
inline constexpr bool _variation_of<Tm<T, Ts...>, Tm<U, Us...>> = true;
} // clang-format on

template<typename T, template<typename...> typename Tm> //
concept specialization_of = internal::_specialization_of<remove_cvref<T>, Tm>;
template<typename T, typename U> concept variation_of = internal::_variation_of<remove_cvref<T>, remove_cvref<U>>;

template<auto V, typename T = decltype(V)> requires convertible_to<decltype(V), T> struct constant {
  using type = T;
  static constexpr type value{V};
  consteval operator type() const noexcept { return value; }
  consteval type operator()() const noexcept { return value; }
};

template<size_t I> using index = constant<I, size_t>;

/// selects I-th argument from given arguments.
/// \note If I is a bool value, the first argument is selected when I is true.
template<convertible_to<size_t> auto I, typename... Ts>
requires((is_bool<decltype(I)> && sizeof...(Ts) == 2) || (!is_bool<decltype(I)> && I < sizeof...(Ts)))
constexpr decltype(auto) select(Ts&&... as) noexcept {
  if constexpr (is_bool<decltype(I)>) return static_cast<Ts...[!I]&&>(as...[!I]);
  else return static_cast<Ts...[I]&&>(as...[I]);
}

/// selects I-th type.
/// \note If I is a bool value, the first type is selected when I is true.
template<convertible_to<size_t> auto I, typename... Ts>
requires((is_bool<decltype(I)> && sizeof...(Ts) == 2) || (!is_bool<decltype(I)> && I < sizeof...(Ts)))
using select_type = Ts...[is_bool<decltype(I)> ? !I : I];

namespace internal {
template<bool Max, arithmetic T, arithmetic U> constexpr auto _max(T a, U b) noexcept {
  if constexpr (same_as<T, U>) {
    if constexpr (Max) return a < b ? b : a;
    else return a < b ? a : b;
  } else if constexpr (using V = decltype(a + b); float_type<V> || signed_integral<V>) return _max<Max>(V(a), V(b));
  else if constexpr (unsigned_integral<T> && unsigned_integral<U>) return _max<Max>(V(a), V(b));
  else return _max<Max>(int_cast(a), int_cast(b));
}
} // namespace internal

inline constexpr struct {
  static constexpr none operator()() noexcept { return {}; }
  static constexpr auto operator()(arithmetic auto a) noexcept { return a; }
  static constexpr auto operator()(arithmetic auto a, arithmetic auto b, arithmetic auto... cs) noexcept {
    if constexpr (sizeof...(cs) > 0) return operator()(operator()(a, b), cs...);
    else return internal::_max<1>(a, b);
  }
} max;

inline constexpr struct {
  static constexpr none operator()() noexcept { return {}; }
  static constexpr auto operator()(arithmetic auto a) noexcept { return a; }
  static constexpr auto operator()(arithmetic auto a, arithmetic auto b, arithmetic auto... cs) noexcept {
    if constexpr (sizeof...(cs) > 0) return operator()(operator()(a, b), cs...);
    else return internal::_max<0>(a, b);
  }
} min;

inline constexpr auto clamp = []<arithmetic T>(T v, arithmetic auto lo, arithmetic auto hi) noexcept {
  return static_cast<T>(max(lo, min(v, hi)));
};

template<typename T, typename To> using copy_const = select_type<is_const<T>, add_const<To>, remove_const<To>>;
template<typename T, typename To> using copy_volatile =
  select_type<is_volatile<T>, add_volatile<To>, remove_volatile<To>>;
template<typename T, typename To> using copy_cv = copy_const<T, copy_volatile<T, To>>;
template<typename T, typename To> using copy_ref =
  select_type<inspect(is_lvref<T>, is_rvref<T>), add_lvref<To>, add_rvref<To>, remove_ref<To>>;
template<typename T, typename To> using copy_cvref = copy_ref<T, copy_cv<remove_ref<T>, remove_ref<To>>>;

template<typename... Ts> using common_type = select_type<requires {
  typename std::common_reference<Ts...>::type;
}, std::common_reference<Ts...>, internal::_type<none>>::type;

template<typename... Ts> concept common_with = !is_none<common_type<Ts...>>;

template<typename F, typename... As> concept invocable = std::invocable<F, As...>;
template<typename F, typename... As> concept nt_invocable = std::is_nothrow_invocable_v<F, As...>;

inline constexpr auto invoke = []<typename F, typename... As>(F&& f, As&&... as) //
  noexcept(nt_invocable<F, As...>) -> decltype(auto) requires invocable<F, As...> {
  if constexpr (!is_void<std::invoke_result_t<F, As...>>)
    return std::invoke(static_cast<F&&>(f), static_cast<As&&>(as)...);
  else return std::invoke(static_cast<F&&>(f), static_cast<As&&>(as)...), none{};
};

template<typename F, typename... As> requires invocable<F, As...>
using invoke_result = decltype(invoke(std::declval<F>(), std::declval<As>()...));

template<typename F, typename R, typename... As> concept invocable_r =
  invocable<F, As...> && convertible_to<std::invoke_result_t<F, As...>, R>;
template<typename F, typename R, typename... As> concept nt_invocable_r =
  nt_invocable<F, As...> && nt_convertible_to<std::invoke_result_t<F, As...>, R>;

template<typename R> inline constexpr auto invoke_r = []<typename F, typename... As>(F&& f, As&&... as) //
  noexcept(nt_invocable_r<F, R, As...>) requires invocable_r<F, R, As...>
{ return std::invoke_r<R>(static_cast<F&&>(f), static_cast<As&&>(as)...); };

template<typename T> inline constexpr size_t extent = select_type<requires {
  std::tuple_size<remove_cvref<T>>::value;
}, std::tuple_size<remove_cvref<T>>, std::extent<remove_cvref<T>>>::value;

template<typename T, size_t N = extent<T>> concept tuple_like = extent<T> == N && N != 0;

namespace internal { // clang-format off
template<size_t I, typename T> inline constexpr int get_strategy = []() -> int {
  using std::get;
  if constexpr (is_bounded_array<remove_cvref<T>> && I < extent<remove_cvref<T>>) return 1 | 4;
  else if constexpr (requires { get<I>(declval<T&&>()); }) return 2 | noexcept(get<I>(declval<T&&>())) * 4;
  else if constexpr (requires { declval<T&&>().template get<I>(); })
    return 3 | noexcept(declval<T&&>().template get<I>()) * 4;
  return 0;
}();
template<size_t I, typename T> constexpr decltype(auto) _get(T&& t) //
  noexcept(bool(get_strategy<I, T> & 4)) requires(get_strategy<I, T> != 0) {
  using std::get;
  if constexpr ((get_strategy<I, T> & 3) == 1) return t[I];
  else if constexpr ((get_strategy<I, T> & 3) == 2) return get<I>(static_cast<T&&>(t));
  else if constexpr ((get_strategy<I, T> & 3) == 3) return static_cast<T&&>(t).template get<I>();
}
} // clang-format on

template<size_t I> inline constexpr auto get = []<typename T>(T&& a)           //
  noexcept(noexcept(internal::_get<I>(static_cast<T&&>(a)))) -> decltype(auto) //
  requires requires { internal::_get<I>(static_cast<T&&>(a)); } { return internal::_get<I>(static_cast<T&&>(a)); };

template<typename T, size_t I> concept gettable = requires { yw::get<I>(std::declval<T>()); };
template<typename T, size_t I> concept nt_gettable = gettable<T, I> && noexcept(yw::get<I>(std::declval<T>()));
template<typename T, size_t I> requires gettable<T, I> using element_t = decltype(yw::get<I>(std::declval<T>()));
} // namespace yw

namespace std {
template<typename T> struct common_type<T, yw::none> : type_identity<yw::none> {};
template<typename T> struct common_type<yw::none, T> : type_identity<yw::none> {};
template<typename C> struct formatter<yw::none, C> {
  std::formatter<const C*, C> fmt;
  constexpr auto parse(auto& ctx) { return fmt.parse(ctx); }
  auto format(const yw::none n, auto& ctx) const { fmt.format(n.to_string<C>(), ctx); }
};
}
