#pragma once
#include <algorithm>
#include <charconv>
#include <compare>
#include <concepts>
#include <expected>
#include <filesystem>
#include <format>
#include <functional>
#include <iterator>
#include <memory>
#include <print>
#include <ranges>
#include <source_location>
#include <string_view>

#ifdef _WIN32
#include <windows.h>
#endif

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
template<typename T> concept is_class = std::is_class_v<T>;
template<typename T> concept is_union = std::is_union_v<T>;
template<typename T> concept is_object = std::is_object_v<T>;

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

namespace sys {
template<size_t I, typename T, typename... Ts> constexpr decltype(auto) _select(T&& a, Ts&&... as) noexcept {
  if constexpr (I == 0) return static_cast<T&&>(a);
  else return _select<I - 1>(static_cast<Ts&&>(as)...);
}
} // namespace sys

/// selects the I-th argument from the given arguments.
/// \note If I is a bool value, selects the first argument if I is true.
template<std::convertible_to<size_t> auto I, typename... Ts>
requires((is_bool<decltype(I)> && sizeof...(Ts) == 2) || (!is_bool<decltype(I)> && I < sizeof...(Ts)))
constexpr decltype(auto) select(Ts&&... as) noexcept {
  if constexpr (is_bool<decltype(I)>) return sys::_select<size_t(!I)>(static_cast<Ts&&>(as)...);
  else return sys::_select<size_t(I)>(static_cast<Ts&&>(as)...);
}

/// selects the type of the I-th argument from the given types.
/// \note If I is a bool value, selects the first type if I is true.
template<std::convertible_to<size_t> auto I, typename... Ts> using select_type =
  remove_ref<decltype(select<I>(std::type_identity<Ts>{}...))>::type;

/// selects the value of the I-th argument from the given arguments.
/// \note If I is a bool value, selects the first argument if I is true.
template<std::convertible_to<size_t> auto I, auto... Vs> constexpr auto select_value = select<I>(Vs...);

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
};

template<typename T> concept is_none = same_as<remove_cv<T>, none>;

//////////////////////////////////////// MARK: common_type

template<typename... Ts> using common_type = select_type<requires { typename std::common_reference<Ts...>::type; },
  std::common_reference<Ts...>, std::type_identity<none>>::type;

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

inline constexpr auto begin = []<std::ranges::range R>(R&& r) noexcept(noexcept(std::ranges::begin(std::declval<R>())))
                                requires requires { std::ranges::begin(std::declval<R>()); }
{ return std::ranges::begin(fwd<R>(r)); };
inline constexpr auto end = []<std::ranges::range R>(R&& r) noexcept(noexcept(std::ranges::end(std::declval<R>())))
                              requires requires { std::ranges::end(std::declval<R>()); }
{ return std::ranges::end(fwd<R>(r)); };
inline constexpr auto size = []<std::ranges::range R>(R&& r) noexcept(noexcept(std::ranges::size(std::declval<R>())))
                               requires requires { std::ranges::size(std::declval<R>()); }
{ return std::ranges::size(fwd<R>(r)); };
inline constexpr auto data = []<std::ranges::range R>(R&& r) noexcept(noexcept(std::ranges::data(std::declval<R>())))
                               requires requires { std::ranges::data(std::declval<R>()); }
{ return std::ranges::data(fwd<R>(r)); };

namespace sys {
template<typename T, template<typename> typename> struct iter_type : std::type_identity<void> {};
template<std::input_iterator I, template<typename> typename Tm> struct iter_type<I, Tm> : std::type_identity<Tm<I>> {};
template<std::ranges::input_range R, template<typename> typename Tm> struct iter_type<R, Tm>
  : std::type_identity<Tm<iterator_t<R>>> {};
} // namespace sys

template<typename T> using iter_value_t = sys::iter_type<remove_cvref<T>, std::iter_value_t>::type;
template<typename T> using iter_difference_t = sys::iter_type<remove_cvref<T>, std::iter_difference_t>::type;
template<typename T> using iter_reference_t = sys::iter_type<remove_cvref<T>, std::iter_reference_t>::type;

template<typename I> concept input_iterator = std::input_iterator<I>;
template<typename I> concept forward_iterator = std::forward_iterator<I>;
template<typename I> concept bidirectional_iterator = std::bidirectional_iterator<I>;
template<typename I> concept random_access_iterator = std::random_access_iterator<I>;

template<typename R> concept sized_range = std::ranges::sized_range<R>;
template<typename R> concept input_range = std::ranges::input_range<R>;
template<typename R> concept forward_range = std::ranges::forward_range<R>;
template<typename R> concept bidirectional_range = std::ranges::bidirectional_range<R>;
template<typename R> concept random_access_range = std::ranges::random_access_range<R>;

template<typename I, typename T = iter_value_t<I>> concept contiguous_iterator =
  std::contiguous_iterator<I> && same_as<T, iter_value_t<I>>;
template<typename R, typename T = iter_value_t<R>> concept contiguous_range =
  std::ranges::contiguous_range<R> && sized_range<R> && same_as<T, iter_value_t<R>>;

template<typename I, typename T = iter_value_t<I>> concept output_iterator = std::output_iterator<I, T>;
template<typename R, typename T = iter_value_t<R>> concept output_range = std::ranges::output_range<R, T>;

template<typename S, typename I> concept sentinel_for = std::sentinel_for<S, I>;
template<typename S, typename I> concept sized_sentinel_for = std::sized_sentinel_for<S, I>;

//////////////////////////////////////// MARK: char / string

using namespace std::string_view_literals;
using namespace std::string_literals;

inline constexpr auto is_ascii = []<char_type C>(C c) noexcept { return 0x20 <= c && c < 0x7F; };
inline constexpr auto is_digit = []<char_type C>(C c) noexcept { return '0' <= c && c <= '9'; };
inline constexpr auto is_lower = []<char_type C>(C c) noexcept { return 'a' <= c && c <= 'z'; };
inline constexpr auto is_upper = []<char_type C>(C c) noexcept { return 'A' <= c && c <= 'Z'; };
inline constexpr auto is_alpha = []<char_type C>(C c) noexcept { return is_lower(c) || is_upper(c); };
inline constexpr auto is_alnum = []<char_type C>(C c) noexcept { return is_alpha(c) || is_digit(c); };
inline constexpr auto is_xdigit = []<char_type C>(C c) noexcept {
  return is_digit(c) || (('a' <= c && c <= 'f') || ('A' <= c && c <= 'F'));
};

template<typename S, typename C = iter_value_t<S>> concept stringable =
  std::convertible_to<S, std::basic_string_view<C>> && std::constructible_from<std::basic_string_view<C>, S>;

template<arithmetic T> constexpr auto stov = [](stringable<char> auto&& str) -> T {
  const auto sv = std::string_view(str);
  T result{};
  std::from_chars(sv.data(), sv.data() + sv.size(), result);
  return result;
};

constexpr auto vtos = [](arithmetic auto value) -> std::string {
  auto temp = std::string(32, '\0');
  const auto [ptr, ec] = std::to_chars(temp.data(), temp.data() + temp.size(), value);
  if (ec == std::errc()) temp.resize(ptr - temp.data());
  else temp.clear();
  return temp;
};

//////////////////////////////////////// MARK: GET

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

//////////////////////////////////////// MARK: std

namespace std {

template<typename T> struct common_type<T, yw::none> : type_identity<yw::none> {};
template<typename T> struct common_type<yw::none, T> : type_identity<yw::none> {};
template<typename C> struct formatter<yw::none, C> {
  std::formatter<const C*, C> fmt;
  constexpr auto parse(auto& ctx) { return fmt.parse(ctx); }
  auto format(const yw::none, auto& ctx) const {
    if constexpr (same_as<C, char>) return fmt.format("none", ctx);
    else if constexpr (same_as<C, wchar_t>) return fmt.format(L"none", ctx);
    else if constexpr (same_as<C, char8_t>) return fmt.format(u8"none", ctx);
    else if constexpr (same_as<C, char16_t>) return fmt.format(u"none", ctx);
    else if constexpr (same_as<C, char32_t>) return fmt.format(U"none", ctx);
  }
};
} // namespace std

//////////////////////////////////////// MARK: unicode

namespace yw {
template<char_type C> inline constexpr auto unicode = []<stringable S>(S&& s) -> std::basic_string<C> {
  using From = iter_value_t<S>;
  if constexpr (same_as<S&&, std::basic_string<C>&&>) return std::move(s);
  if constexpr (same_as<From, C>) return std::basic_string<C>(std::basic_string_view<C>(s));
  const auto sv_original = std::basic_string_view<From>(s);
  if constexpr (sizeof(From) == sizeof(C))
    return std::basic_string<C>(std::bit_cast<std::basic_string_view<C>>(sv_original));
  using T = select_type<sizeof(From) / 2, char8_t, char16_t, char32_t>;
  const auto sv = std::bit_cast<std::basic_string_view<T>>(sv_original);
  constexpr auto scale = select_value<yw::max(int(sizeof(T)) - int(sizeof(C)), 0), 1, 3, 2, 4>;
  auto r = std::basic_string<C>(sv.size() * scale, C{});
  auto out = r.data();
  for (auto s = sv.data(), end = s + sv.size(); s < end;) {
    char32_t uc;
    if constexpr (same_as<T, char8_t>) {
      const auto c = char32_t(*s);
      const auto i = unsigned(c >= 0xc0) + unsigned(c >= 0xe0) + unsigned(c >= 0xf0);
      const auto j = i + 1 + unsigned(i != 0);
      uc = char32_t(-int(i == 3) & s[i < 3 ? i : 3] & 0x3f);
      uc |= char32_t((-int(i > 1) & s[i < 2 ? i : 2] & 0x3f)) << (6 * (i > 1 ? i - 2 : 0));
      uc |= char32_t((-int(i > 0) & s[i < 1 ? 1 : 1] & 0x3f)) << (6 * (i > 0 ? i - 1 : 0));
      uc |= char32_t(char8_t(c << j) >> j) << (6 * i);
      s += i + 1;
    } else if constexpr (same_as<T, char16_t>) {
      const auto c = char32_t(*s);
      const bool b = (c & 0xff00) == 0xd800;
      uc = c ^ (-int(b) & (c ^ (0x10000 | ((c - 0xd800) << 10 | char32_t(s[b] - 0xdc00)))));
      s += 1 + b;
    } else uc = char32_t(*s++);
    if constexpr (sizeof(C) == 1) {
      const auto i = unsigned(uc >= 0x80) + unsigned(uc >= 0x800) + unsigned(uc >= 0x10000);
      out[i < 3 ? i : 3] = C(0x80 | (uc & 0x3f));
      out[i < 2 ? i : 2] = C(0x80 | ((uc >> (6 * (i > 1 ? i - 2 : 0))) & 0x3f));
      out[i < 1 ? i : 1] = C(0x80 | ((uc >> (6 * (i > 0 ? i - 1 : 0))) & 0x3f));
      *out = C(uint32_t(((i + (i >> 1)) << 4) + (-i & 0xb0)) | ((uc >> (6 * i)) & (0x3f >> i | -int(i == 0))));
      out += i + 1;
    } else if constexpr (sizeof(C) == 2) {
      const bool b = uc >= 0x10000;
      out[b] = C(0xdc00 | (uc & 0x3ff));
      *out = C(uc ^ ((uc ^ (0xd800 | (uc >> 10))) & -int(b)));
      out += 1 + b;
    } else *out++ = C(uc);
  }
  r.resize(out - r.data());
  return r;
};
} // namespace yw

//////////////////////////////////////// MARK: source

namespace yw {
struct source {
  std::string_view file, func;
  uint32_t line, column;
  source(const std::source_location& loc = std::source_location::current()) noexcept
    : file(loc.file_name()), func(loc.function_name()), line(loc.line()), column(loc.column()) {}
  friend constexpr bool operator==(const source& a, const source& b) noexcept {
    return a.file == b.file && a.func == b.func && a.line == b.line && a.column == b.column;
  }
};
} // namespace yw
namespace std {
template<typename C> struct formatter<yw::source, C> {
  formatter<basic_string<C>, C> fmt;
  constexpr auto parse(auto& ctx) { return fmt.parse(ctx); }
  auto format(const yw::source& src, auto& ctx) const {
    auto s = std::format("{}({},{}) in {}", src.file, src.line, src.column, src.func);
    return fmt.format(yw::unicode<C>(move(s)), ctx);
  }
};
} // namespace std

//////////////////////////////////////// MARK: null_terminated

namespace yw {
template<char_type C> class null_terminated {
  std::variant<std::basic_string<C>, std::basic_string_view<C>> _data;
  template<typename S> static constexpr bool _is_array = is_bounded_array<remove_ref<S>> && same_as<iter_value_t<S>, C>;

public:
  constexpr null_terminated() : _data(std::basic_string_view<C>{}) {}
  constexpr null_terminated(std::basic_string<C>& str) : _data(std::in_place_index_t<1>{}, str) {}
  constexpr null_terminated(const std::basic_string<C>& str) : _data(std::in_place_index_t<1>{}, str) {}
  constexpr null_terminated(std::basic_string<C>&& str) : _data(std::in_place_index_t<0>{}, std::move(str)) {}
  constexpr null_terminated(const std::basic_string<C>&& str) : _data(std::in_place_index_t<0>{}, std::move(str)) {}
  template<typename S> requires _is_array<S> constexpr null_terminated(const S& a)
    : _data(std::in_place_index_t<1>{}, std::basic_string_view<C>(a, std::char_traits<C>::length(a))) {}
  template<stringable S> requires(!_is_array<S>) constexpr null_terminated(S&& s) : _data() {
    if constexpr (different_from<iter_value_t<S>, C>) _data.template emplace<0>(unicode<C>(static_cast<S&&>(s)));
    else _data.template emplace<0>(std::basic_string<C>(std::basic_string_view<C>(static_cast<S&&>(s))));
  }

  constexpr operator std::basic_string_view<C>() const {
    return std::visit([](const auto& v) { return std::basic_string_view<C>(v); }, _data);
  }

  bool empty() const noexcept {
    return std::visit([](const auto& v) { return v.empty(); }, _data);
  }
  size_t size() const noexcept {
    return std::visit([](const auto& v) { return v.size(); }, _data);
  }
  const C* data() const noexcept {
    return std::visit([](const auto& v) { return v.data(); }, _data);
  }
  const C* begin() const noexcept {
    return std::visit([](const auto& v) { return v.data(); }, _data);
  }
  const C* end() const noexcept {
    return std::visit([](const auto& v) { return v.data() + v.size(); }, _data);
  }
};
} // namespace yw
namespace std {
template<typename C> struct formatter<yw::null_terminated<C>, C> {
  formatter<basic_string_view<C>, C> fmt;
  constexpr auto parse(auto& ctx) { return fmt.parse(ctx); }
  auto format(const yw::null_terminated<C>& str, auto& ctx) const {
    return fmt.format(static_cast<basic_string_view<C>>(str), ctx);
  }
};
} // namespace std

//////////////////////////////////////// MARK: error

namespace yw {
enum class errors : uint32_t {
  success = 0,
  invalid_argument,
  invalid_file,
  invalid_operation,
  operation_failed,
  not_initialized,
};
struct error {
  errors code;
  int32_t system_code;
  uint64_t position;
  null_terminated<char> message;
  error() noexcept : code(errors::success), system_code(0), position(uint64_t(-1)), message() {}
  explicit error(errors e, null_terminated<char> msg, int32_t sys_code = 0, uint64_t pos = uint64_t(-1)) noexcept
    : code(e), system_code(sys_code), message(std::move(msg)), position(pos) {}
};
struct error_trace {
  yw::error error;
  std::vector<source> frames;
  error_trace() = default;
  error_trace(yw::error err, const source& src = {}) : error(std::move(err)) {
    frames.reserve(8);
    frames.push_back(src);
  }
  error_trace& push(const source& src = {}) & {
    frames.push_back(src);
    return *this;
  }
};
inline std::unexpected<error_trace> unexpected_error(
  errors e, null_terminated<char> msg, int32_t sys_code = 0, uint64_t pos = uint64_t(-1), const source& src = {}) {
  return std::unexpected<error_trace>(error_trace(yw::error(e, std::move(msg), sys_code, pos), src));
}
inline std::unexpected<error_trace> unexpected_error(error_trace& e, const source& src = {}) {
  return std::unexpected<error_trace>(std::move(e.push(src)));
}
inline std::unexpected<error_trace> unexpected_error(std::unexpected<error_trace>& e, const source& src = {}) {
  e.error().push(src);
  return std::move(e);
}
} // namespace yw
namespace std {
template<typename C> struct formatter<yw::error, C> {
  formatter<basic_string<C>, C> fmt;
  constexpr auto parse(auto& ctx) { return fmt.parse(ctx); }
  auto format(const yw::error& err, auto& ctx) const {
    std::string s;
    if (err.system_code == 0) s = std::format("{}", err.message);
    else s = std::format("{} (code={})", err.message, err.system_code);
    if (err.position != uint64_t(-1)) s += std::format("\n  input offset={}", err.position);
    return fmt.format(yw::unicode<C>(std::move(s)), ctx);
  }
};
template<typename C> struct formatter<yw::error_trace, C> {
  formatter<basic_string<C>, C> fmt;
  constexpr auto parse(auto& ctx) { return fmt.parse(ctx); }
  auto format(const yw::error_trace& err, auto& ctx) const {
    std::string s = std::format("{}", err.error);
    for (const auto& src : err.frames) s += std::format("\n  at {}", src);
    return fmt.format(yw::unicode<C>(std::move(s)), ctx);
  }
};
} // namespace std

//////////////////////////////////////// MARK: format

namespace yw {
inline constexpr struct {
  template<typename C, typename T> static std::basic_string<C> as(const T& a) {
    if constexpr (same_as<T, std::filesystem::path>) return unicode<C>(a.native());
    else if constexpr (same_as<T, char>) return std::format("{}", a);
    else if constexpr (same_as<T, wchar_t>) return std::format(L"{}", a);
    else return unicode<C>(std::format("{}", a));
  }
  template<stringable S, typename... Ts> static std::basic_string<iter_value_t<S>> operator()(S&& fmt, Ts&&... as) {
    using C = iter_value_t<S>;
    if constexpr (included_in<std::filesystem::path, remove_cvref<Ts>...>) {
      constexpr auto _to_string = []<typename T>(T&& a) -> decltype(auto) {
        if constexpr (same_as<T, std::filesystem::path>) return unicode<C>(a.native());
        else return static_cast<T&&>(a);
      };
      return operator()(static_cast<S&&>(fmt), _to_string(static_cast<Ts&&>(as))...);
    } else if constexpr (same_as<C, char>) return std::vformat(std::string_view(fmt), std::make_format_args(as...));
    else if constexpr (same_as<C, wchar_t>) return std::vformat(std::wstring_view(fmt), std::make_wformat_args(as...));
    else if constexpr (same_as<C, char8_t>) {
      auto s =
        std::vformat(std::bit_cast<std::string_view>(std::basic_string_view<C>(fmt)), std::make_format_args(as...));
      return std::basic_string<C>(reinterpret_cast<std::basic_string<C>&&>(std::move(s)));
    } else if constexpr (same_as<C, char16_t>) {
      auto s =
        std::vformat(std::bit_cast<std::wstring_view>(std::basic_string_view<C>(fmt)), std::make_wformat_args(as...));
      return std::basic_string<C>(reinterpret_cast<std::basic_string<C>&&>(std::move(s)));
    } else if constexpr (same_as<C, char32_t>) {
      const auto f{unicode<char>(fmt)};
      return unicode<C>(std::vformat(f, std::make_format_args(as...)));
    } else throw "yw::format: unsupported character type";
  }
} format;
} // namespace yw

//////////////////////////////////////// MARK: print

namespace yw {
inline constexpr struct {
  template<typename S, typename... Ts> static void operator()(S&& fmt, Ts&&... as) {
#ifdef _WIN32
    if constexpr (sizeof...(Ts) == 0) {
      const auto s = format.as<wchar_t>(fmt);
      ::WriteConsoleW(::GetStdHandle(STD_OUTPUT_HANDLE), s.data(), unsigned(s.size()), nullptr, nullptr);
    } else {
      const auto s = unicode<wchar_t>(format(static_cast<S&&>(fmt), static_cast<Ts&&>(as)...));
      ::WriteConsoleW(::GetStdHandle(STD_OUTPUT_HANDLE), s.data(), unsigned(s.size()), nullptr, nullptr);
    }
#else
    if constexpr (sizeof...(Ts) == 0) {
      const auto s = format.as<char>(fmt);
      std::fputs((const char*)s.data(), stdout);
    } else {
      const auto s = unicode<char>(format(static_cast<S&&>(fmt), static_cast<Ts&&>(as)...));
      std::fputs((const char*)s.data(), stdout);
    }
#endif
  }
} print_inline;

inline constexpr struct {
  static void operator()() {
#ifdef _WIN32
    ::WriteConsoleW(::GetStdHandle(STD_OUTPUT_HANDLE), L"\n", 1, nullptr, nullptr);
#else
    std::fputc('\n', stdout);
#endif
  }
  template<typename S, typename... Ts> static void operator()(S&& fmt, Ts&&... as) {
    print_inline(static_cast<S&&>(fmt), static_cast<Ts&&>(as)...);
    operator()();
  }
} print;

//////////////////////////////////////// MARK: print_error

inline constexpr struct {
  template<stringable S> static int operator()(S&& message, const error_trace& err, const source& src = {}) {
#ifdef _WIN32
    // auto s0 = format(L"{}", err);
    // auto s1 = format(L"\n  at {}\n", src);
    // auto s2 = format(L"{}", unicode<wchar_t>(message));
    std::wstring s;
    if constexpr (stringable<S, wchar_t>) s = format(L"{}\n  at {}\n{}", err, src, std::wstring_view(message));
    else s = format(L"{}\n  at {}\n{}", err, src, unicode<wchar_t>(message));
    ::WriteConsoleW(::GetStdHandle(STD_ERROR_HANDLE), s.data(), unsigned(s.size()), nullptr, nullptr);
#else
    if constexpr (!stringable<char, S>) {
      const auto s = format("{}\n  at {}\n{}", err, src, unicode<char>(message));
      std::fputs((const char*)s.data(), stderr);
    } else {
      const auto s = format("{}\n  at {}\n{}", err, src, std::string_view(message));
      std::fputs((const char*)s.data(), stderr);
    }
#endif
    return err.error.system_code;
  }
} print_error;

//////////////////////////////////////// MARK: assume

template<typename T> requires(!is_void<T>)
[[nodiscard]] T assume(std::expected<T, yw::error_trace>&& res, const source& src = {}) {
  if (!res) {
    print_error("Assumption failed", res.error(), src);
    std::exit(int(res.error().error.system_code));
  }
  return std::move(res.value());
}
void assume(std::expected<void, yw::error_trace>&& res, const source& src = {}) {
  if (!res) {
    print_error("Assumption failed", res.error(), src);
    std::exit(int(res.error().error.system_code));
  }
}
} // namespace yw
