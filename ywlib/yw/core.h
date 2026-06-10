#pragma once
#include <algorithm>
#include <charconv>
#include <chrono>
#include <cmath>
#include <compare>
#include <concepts>
#include <cstring>
#include <expected>
#include <filesystem>
#include <functional>
#include <iterator>
#include <memory>
#include <numeric>
#include <ranges>
#include <source_location>
#include <string_view>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

inline constexpr char yw_core[] = "yw/core.h";

namespace yw {

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

template<typename C> inline const std::basic_string<C> empty_string{};

template<typename... Ts> inline constexpr bool always_false = false;
template<bool... Bs> inline constexpr size_t counts = (Bs + ...);
template<bool... Bs> inline constexpr size_t inspects = false;
template<bool B, bool... Bs> inline constexpr size_t inspects<B, Bs...> = B ? 0 : 1 + inspects<Bs...>;
template<typename T, size_t N> constexpr size_t arraysize(const T (&)[N]) noexcept { return N; }

using pass = std::identity;
using equal = std::ranges::equal_to;
using not_equal = std::ranges::not_equal_to;
using less = std::ranges::less;
using greater = std::ranges::greater;
using less_equal = std::ranges::less_equal;
using greater_equal = std::ranges::greater_equal;

inline constexpr equal eq{};
inline constexpr not_equal ne{};
inline constexpr less lt{};
inline constexpr greater gt{};
inline constexpr less_equal le{};
inline constexpr greater_equal ge{};

template<typename T> using remove_const = std::remove_const_t<T>;
template<typename T> using remove_volatile = std::remove_volatile_t<T>;
template<typename T> using remove_cv = std::remove_cv_t<T>;
template<typename T> using remove_ref = std::remove_reference_t<T>;
template<typename T> using remove_cvref = remove_cv<remove_ref<T>>;
template<typename T> using remove_pointer = std::remove_pointer_t<T>;
template<typename T> using remove_extent = std::remove_extent_t<T>;

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
template<typename T> concept is_function = !is_const<const T> && !is_reference<T>;

template<typename T, typename... Ts> concept same_as = (std::same_as<T, Ts> && ...);
template<typename T, typename... Ts> concept different_from = ((!std::same_as<T, Ts>) && ...);
template<typename T, typename... Ts> concept included_in = (std::same_as<T, Ts> || ...);
template<typename T, typename... Ts> concept castable_to = ((requires { static_cast<Ts>(std::declval<T>()); }) && ...);
template<typename T, typename... Ts> concept convertible_to = (std::convertible_to<T, Ts> && ...);
template<typename T, typename... Ts> concept nt_castable_to =
  castable_to<T, Ts...> && noexcept((static_cast<Ts>(std::declval<T>()), ...));
template<typename T, typename... Ts> concept nt_convertible_to = convertible_to<T, Ts...> && nt_castable_to<T, Ts...>;
template<typename T, typename... Ts> concept derived_from = (std::derived_from<T, Ts> && ...);

template<typename T> concept is_void = same_as<remove_cv<T>, void>;
template<typename T> concept is_bool = same_as<remove_cv<T>, bool>;
template<typename T> concept is_nullptr = same_as<remove_cv<T>, decltype(nullptr)>;
template<typename T> concept char_type = included_in<remove_cv<T>, char, wchar_t, char8_t, char16_t, char32_t>;
template<typename T> concept int_type = included_in<remove_cv<T>, signed char, short, int, long, long long>;
template<typename T> concept uint_type =
  included_in<remove_cv<T>, uint8_t, uint16_t, uint32_t, unsigned long, uint64_t>;
template<typename T> concept float_type = included_in<remove_cv<T>, float, double, long double>;
template<typename T> concept integral = is_bool<T> || char_type<T> || int_type<T> || uint_type<T>;
template<typename T> concept signed_integral = integral<T> && requires { requires T(-1) < T(0); };
template<typename T> concept unsigned_integral = integral<T> && !signed_integral<T>;
template<typename T> concept floating = float_type<T>;
template<typename T> concept arithmetic = integral<T> || float_type<T>;
template<typename T> concept trivial = __is_trivially_copyable(T);
template<typename T> concept is_enum = std::is_enum_v<T>;
template<typename T> concept is_class = std::is_class_v<T>;
template<typename T> concept is_union = std::is_union_v<T>;
template<typename T> concept is_object = std::is_object_v<T>;

namespace _ { // clang-format off
template<typename T> struct _member_traits { using class_type = void; using member_type = void; };
template<typename C, typename M> struct _member_traits<M C::*> { using class_type = C; using member_type = M; };
} // clang-format on

template<typename T> using member_type = _::_member_traits<remove_cvref<T>>::member_type;
template<typename T> using class_type = _::_member_traits<remove_cvref<T>>::class_type;
template<typename T> concept is_member_object_pointer = std::is_member_object_pointer_v<T>;
template<typename T> concept is_member_function_pointer = std::is_member_function_pointer_v<T>;
template<typename T> concept is_member_pointer = is_member_object_pointer<T> || is_member_function_pointer<T>;

//////////////////////////////////////// MARK: narrow_cast

inline constexpr auto uint_cast = []<typename T>(T value) noexcept requires arithmetic<T> || is_enum<T> {
  if constexpr (sizeof(T) == 1) return std::bit_cast<uint8_t>(value);
  else if constexpr (sizeof(T) == 2) return std::bit_cast<uint16_t>(value);
  else if constexpr (sizeof(T) == 4) return std::bit_cast<uint32_t>(value);
  else if constexpr (sizeof(T) == 8) return std::bit_cast<uint64_t>(value);
  else static_assert(always_false<T>, "unsupported type for uint_cast");
};

//////////////////////////////////////// MARK: construct

template<typename T, typename... As> concept constructible = std::is_constructible_v<T, As...>;
template<typename T, typename... As> concept nt_constructible =
  constructible<T, As...> && std::is_nothrow_constructible_v<T, As...>;
template<typename T>
inline constexpr auto construct = []<typename... As>(As&&... as) noexcept(nt_constructible<T, As...>) -> T
  requires constructible<T, As...> { return T{static_cast<As&&>(as)...}; };

//////////////////////////////////////// MARK: assign

template<typename T, typename U> concept assignable = std::is_assignable_v<T, U>;
template<typename T, typename U> concept nt_assignable = assignable<T, U> && std::is_nothrow_assignable_v<T, U>;
inline constexpr auto assign = []<typename T, typename U>(T&& t, U&& u) noexcept(nt_assignable<T, U>) -> void
  requires assignable<T, U> { static_cast<T&&>(t) = static_cast<U&&>(u); };

//////////////////////////////////////// MARK: specialization and variation
namespace _ {
template<typename T, template<typename...> typename Tm> inline constexpr bool _specialization_of{0};
template<template<typename...> typename Tm, typename... Ts> inline constexpr bool _specialization_of<Tm<Ts...>, Tm>{1};
template<typename T, typename U> inline constexpr bool _variation_of{0};
template<template<typename...> typename Tm, typename... Ts, typename... Us>
inline constexpr bool _variation_of<Tm<Ts...>, Tm<Us...>>{1};
template<template<auto...> typename Tm, auto... Ts, auto... Us>
inline constexpr bool _variation_of<Tm<Ts...>, Tm<Us...>>{1};
template<template<typename, auto...> typename Tm, typename T, auto... Ts, typename U, auto... Us>
inline constexpr bool _variation_of<Tm<T, Ts...>, Tm<U, Us...>>{1};
} // namespace _

template<typename T, template<typename...> typename Tm> concept specialization_of =
  _::_specialization_of<remove_cvref<T>, Tm>;
template<typename T, typename U> concept variation_of = _::_variation_of<remove_cvref<T>, remove_cvref<U>>;

template<auto V, typename T = decltype(V)> requires convertible_to<decltype(V), T> struct constant {
  using type = T;
  static constexpr type value{V};
  consteval operator type() const noexcept { return value; }
  consteval type operator()() const noexcept { return value; }
};

//////////////////////////////////////// MARK: select

namespace sys {
template<size_t I, typename T, typename... Ts> constexpr decltype(auto) _select(T&& a, Ts&&... as) noexcept {
  if constexpr (I == 0) return static_cast<T&&>(a);
  else return _select<I - 1>(static_cast<Ts&&>(as)...);
}
} // namespace sys

/// selects I-th argument from given arguments.
/// \note If I is a bool value, the first argument is selected when I is true.
template<std::convertible_to<size_t> auto I, typename... Ts>
requires((is_bool<decltype(I)> && sizeof...(Ts) == 2) || (!is_bool<decltype(I)> && I < sizeof...(Ts)))
constexpr decltype(auto) select(Ts&&... as) noexcept {
  if constexpr (is_bool<decltype(I)>) return sys::_select<size_t(!I)>(static_cast<Ts&&>(as)...);
  else return sys::_select<size_t(I)>(static_cast<Ts&&>(as)...);
}

/// selects I-th type.
/// \note If I is a bool value, the first type is selected when I is true.
template<std::convertible_to<size_t> auto I, typename... Ts> using select_type =
  remove_ref<decltype(select<I>(std::type_identity<Ts>{}...))>::type;

/// selects I-th value.
/// \note If I is a bool value, the first argument is selected when I is true.
template<std::convertible_to<size_t> auto I, auto... Vs> constexpr auto select_value = select<I>(Vs...);

//////////////////////////////////////// MARK: add/copy_cvref

template<typename T> using add_lvref = std::add_lvalue_reference_t<T>;
template<typename T> using add_rvref = std::add_rvalue_reference_t<remove_ref<T>>;
template<typename T> using add_fwref = std::add_rvalue_reference_t<T>;

template<typename T, typename To> using copy_const = select_type<is_const<T>, const To, remove_const<To>>;
template<typename T, typename To> using copy_volatile = select_type<is_volatile<T>, volatile To, remove_volatile<To>>;
template<typename T, typename To> using copy_cv = copy_const<T, copy_volatile<T, To>>;
template<typename T, typename To> using copy_ref =
  select_type<inspects<is_lvref<T>, is_rvref<T>>, add_lvref<To>, add_rvref<To>, remove_ref<To>>;
template<typename T, typename To> using copy_cvref = copy_ref<T, copy_cv<remove_ref<T>, remove_ref<To>>>;

template<typename T> using add_const = copy_ref<T, const remove_ref<T>>;
template<typename T> using add_volatile = copy_ref<T, volatile remove_ref<T>>;
template<typename T> using add_cv = copy_ref<T, const volatile remove_ref<T>>;

//////////////////////////////////////// MARK: none

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
  template<char_type C> constexpr std::basic_string_view<C> to_string() const noexcept {
    constexpr C _str[] = {'n', 'o', 'n', 'e'};
    return std::basic_string_view<C>(_str, 4);
  }
  constexpr std::basic_string_view<char> to_string() const noexcept { return to_string<char>(); }
};

template<typename T> concept is_none = same_as<remove_cv<T>, none>;

//////////////////////////////////////// MARK: common_type

template<typename... Ts> using common_type = select_type<requires {
  typename std::common_reference<Ts...>::type;
}, std::common_reference<Ts...>, std::type_identity<none>>::type;

template<typename... Ts> concept common_with = !is_none<common_type<Ts...>>;

//////////////////////////////////////// MARK: invoke

template<typename F, typename... As> concept invocable = std::invocable<F, As...>;
template<typename F, typename... As> concept nt_invocable =
  invocable<F, As...> && std::is_nothrow_invocable_v<F, As...>;

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

template<typename R>
inline constexpr auto invoke_r = []<typename F, typename... As>(F&& f, As&&... as) noexcept(nt_invocable_r<F, R, As...>)
                                   requires invocable_r<F, R, As...>
{ return std::invoke_r<R>(static_cast<F&&>(f), static_cast<As&&>(as)...); };

//////////////////////////////////////// MARK: max

inline constexpr struct {
  static constexpr none operator()() noexcept { return {}; }
  template<typename T> static constexpr decltype(auto) operator()(T&& a) noexcept { return static_cast<T&&>(a); }
  template<typename T, std::common_with<T> U> static constexpr auto operator()(T&& a, U&& b) noexcept(noexcept(a < b)) {
    return a < b ? std::common_type_t<T, U>(static_cast<U&&>(b)) : std::common_type_t<T, U>(static_cast<T&&>(a));
  }
  template<typename T, typename U, typename... Ts> //
  static constexpr std::common_type_t<T, U, Ts...> operator()(T&& a, U&& b, Ts&&... cs) {
    return operator()(operator()(static_cast<T&&>(a), static_cast<U&&>(b)), static_cast<Ts&&>(cs)...);
  }
} max;

inline constexpr struct {
  static constexpr none operator()() noexcept { return {}; }
  template<typename T> static constexpr decltype(auto) operator()(T&& a) noexcept { return static_cast<T&&>(a); }
  template<typename T, std::common_with<T> U> static constexpr auto operator()(T&& a, U&& b) noexcept(noexcept(b < a)) {
    return b < a ? std::common_type_t<T, U>(static_cast<U&&>(b)) : std::common_type_t<T, U>(static_cast<T&&>(a));
  }
  template<typename T, typename U, typename... Ts> //
  static constexpr std::common_type_t<T, U, Ts...> operator()(T&& a, U&& b, Ts&&... cs) {
    return operator()(operator()(static_cast<T&&>(a), static_cast<U&&>(b)), static_cast<Ts&&>(cs)...);
  }
} min;

//////////////////////////////////////// MARK: iterator / range

template<std::ranges::range R> using iterator_t = std::ranges::iterator_t<R>;
template<std::ranges::range R> using sentinel_t = std::ranges::sentinel_t<R>;

inline constexpr auto begin = []<std::ranges::range R>(R&& r) noexcept(noexcept(std::ranges::begin(r)))
                                requires requires { std::ranges::begin(r); } { return std::ranges::begin(r); };
inline constexpr auto end = []<std::ranges::range R>(R&& r) noexcept(noexcept(std::ranges::end(r)))
                              requires requires { std::ranges::end(r); } { return std::ranges::end(r); };
inline constexpr auto size = []<std::ranges::range R>(R&& r) noexcept(noexcept(std::ranges::size(r)))
                               requires requires { std::ranges::size(r); } { return std::ranges::size(r); };
inline constexpr auto data = []<std::ranges::range R>(R&& r) noexcept(noexcept(std::ranges::data(r)))
                               requires requires { std::ranges::data(r); } { return std::ranges::data(r); };

namespace sys {
template<typename T, template<typename> typename> struct iter_type : std::type_identity<void> {};
template<std::input_iterator I, template<typename> typename Tm> struct iter_type<I, Tm> : std::type_identity<Tm<I>> {};
template<std::ranges::input_range R, template<typename> typename Tm> struct iter_type<R, Tm>
  : std::type_identity<Tm<iterator_t<R>>> {};
} // namespace sys

template<typename T> using iter_value_t = sys::iter_type<remove_cvref<T>, std::iter_value_t>::type;
template<typename T> using iter_difference_t = sys::iter_type<remove_cvref<T>, std::iter_difference_t>::type;
template<typename T> using iter_reference_t = sys::iter_type<remove_cvref<T>, std::iter_reference_t>::type;

template<typename I, typename T = iter_value_t<I>> concept input_iterator =
  std::input_iterator<I> && same_as<T, iter_value_t<I>>;
template<typename I, typename T = iter_value_t<I>> concept forward_iterator =
  std::forward_iterator<I> && same_as<T, iter_value_t<I>>;
template<typename I, typename T = iter_value_t<I>> concept bidirectional_iterator =
  std::bidirectional_iterator<I> && same_as<T, iter_value_t<I>>;
template<typename I, typename T = iter_value_t<I>> concept random_access_iterator =
  std::random_access_iterator<I> && same_as<T, iter_value_t<I>>;

template<typename R, typename T = iter_value_t<R>> concept sized_range =
  std::ranges::sized_range<R> && same_as<T, iter_value_t<R>>;
template<typename R, typename T = iter_value_t<R>> concept input_range =
  std::ranges::input_range<R> && same_as<T, iter_value_t<R>>;
template<typename R, typename T = iter_value_t<R>> concept forward_range =
  std::ranges::forward_range<R> && same_as<T, iter_value_t<R>>;
template<typename R, typename T = iter_value_t<R>> concept bidirectional_range =
  std::ranges::bidirectional_range<R> && same_as<T, iter_value_t<R>>;
template<typename R, typename T = iter_value_t<R>> concept random_access_range =
  std::ranges::random_access_range<R> && same_as<T, iter_value_t<R>>;

template<typename I, typename T = iter_value_t<I>> concept contiguous_iterator =
  std::contiguous_iterator<I> && same_as<T, iter_value_t<I>>;
template<typename R, typename T = iter_value_t<R>> concept contiguous_range =
  std::ranges::contiguous_range<R> && sized_range<R> && same_as<T, iter_value_t<R>>;

template<typename I, typename T = iter_value_t<I>> concept output_iterator = std::output_iterator<I, T>;
template<typename R, typename T = iter_value_t<R>> concept output_range = std::ranges::output_range<R, T>;

template<typename S, typename I> concept sentinel_for = std::sentinel_for<S, I>;
template<typename S, typename I> concept sized_sentinel_for = std::sized_sentinel_for<S, I>;
} // namespace yw

/// MARK: GET

namespace yw {
template<typename T> inline constexpr size_t extent = select_type<requires {
  std::tuple_size<std::remove_reference_t<T>>::value;
}, std::tuple_size<std::remove_reference_t<T>>, std::extent<std::remove_reference_t<T>>>::value;

template<typename T, size_t N = extent<T>> concept tuple_like = extent<T> == N && N != 0;

namespace sys {
template<size_t I, typename T> inline constexpr int get_strategy = []() -> int {
  using std::get;
  if constexpr (std::is_bounded_array_v<std::remove_cvref_t<T>> && I < std::extent_v<std::remove_cvref_t<T>>)
    return 1 | 4;
  else if constexpr (requires { get<I>(std::declval<T&&>()); }) return 2 | noexcept(get<I>(std::declval<T&&>())) * 4;
  else if constexpr (requires { std::declval<T&&>().template get<I>(); })
    return 3 | noexcept(std::declval<T&&>().template get<I>()) * 4;
  return 0;
}();
} // namespace sys

template<size_t I> inline constexpr auto get =                      //
  []<typename T>(T&& a) noexcept(bool(sys::get_strategy<I, T> & 4)) //
  -> decltype(auto) requires(sys::get_strategy<I, T> != 0) {
  using std::get;
  if constexpr ((sys::get_strategy<I, T> & 3) == 1) return a[I];
  else if constexpr ((sys::get_strategy<I, T> & 3) == 2) return get<I>(static_cast<T&&>(a));
  else if constexpr ((sys::get_strategy<I, T> & 3) == 3) return static_cast<T&&>(a).template get<I>();
};

template<typename T, size_t I> concept gettable = requires { yw::get<I>(std::declval<T>()); };
template<typename T, size_t I> concept nt_gettable = gettable<T, I> && noexcept(yw::get<I>(std::declval<T>()));
template<typename T, size_t I> requires gettable<T, I> using element_t = decltype(get<I>(std::declval<T>()));
} // namespace yw

/// MARK: std

namespace std {
template<typename T> struct common_type<T, yw::none> : type_identity<yw::none> {};
template<typename T> struct common_type<yw::none, T> : type_identity<yw::none> {};
template<typename C> struct formatter<yw::none, C> {
  std::formatter<const C*, C> fmt;
  constexpr auto parse(auto& ctx) { return fmt.parse(ctx); }
  auto format(const yw::none n, auto& ctx) const { fmt.format(n.to_string<C>(), ctx); }
};
} // namespace std

namespace yw {

/// MARK: unique_id

inline constexpr uint64_t unique_id(std::source_location loc = std::source_location::current()) noexcept {
  constexpr uint64_t basis = 14695981039346656037ull;
  constexpr uint64_t prime = 1099511628211ull;
  auto hash = [](std::string_view s) {
    uint64_t h = basis;
    for (char c : s) { h ^= uint8_t(c), h *= prime; }
    return h;
  };
  uint64_t h = basis;
  auto mix = [&](uint64_t v) { h ^= v, h *= prime; };
  mix(hash(loc.file_name())), mix(hash(loc.function_name())), mix(loc.line()), mix(loc.column());
  return h;
}
} // namespace yw
