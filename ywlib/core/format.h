#pragma once
#include <core/core.h>
#include <core/string.h>

namespace yw {

namespace internal {
constexpr auto hex_table = "0123456789abcdef";
template<typename T, typename C> concept has_to_string_c = requires(T&& a) {
  { a.template to_string<C>() } -> convertible_to<string<C>>;
};
template<typename T> concept has_to_string = requires(T&& a) {
  { a.to_string() } -> stringable;
};
template<char_type C, typename T> constexpr string<C> _format(T&& Arg) {
  using t = remove_cvref<T>;
  if constexpr (stringable<t>) return unicode<C>(static_cast<T&&>(Arg));
  else if constexpr (char_type<t>) return unicode<C>(string<t>(1, Arg));
  else if constexpr (arithmetic<t>) return vtos<C>(Arg);
  else if constexpr (is_pointer<t>) {
    string<C> s(sizeof(void*) * 2 + 2, C('0'));
    s[1] = C('x');
    if consteval {
      return s; // always return 0x0...0
    } else {
      auto u = reinterpret_cast<size_t>(Arg);
      for (auto p = s.data() + s.size(); u != 0; u /= 16) *--p = C(internal::hex_table[u % 16]);
      return s;
    }
  } else if constexpr (same_as<remove_cvref<T>, std::source_location>) {
    auto s = unicode<C>(Arg.file_name());
    s += unicode<C>(":");
    s += unicode<C>(std::to_string(Arg.line()));
    s += unicode<C>(":");
    s += unicode<C>(std::to_string(Arg.column()));
    s += unicode<C>(": ");
    s += unicode<C>(Arg.function_name());
    return s;
  } else if constexpr (internal::has_to_string_c<T, C>) return Arg.template to_string<C>();
  else if constexpr (internal::has_to_string<T>) return unicode<C>(Arg.to_string());
  else static_assert(always_false<T>, "Type does not have to_string<C> or to_string method");
}
} // namespace internal

template<char_type C, typename... Ts> constexpr string<C> format(Ts&&... Args) {
  string<C> s;
  ((s += internal::_format<C>(static_cast<Ts&&>(Args))), ...);
  return s;
}

template<typename... Ts> constexpr auto format(Ts&&... Args) {
  using T = remove_cvref<select_type<0, Ts...>>;
  if constexpr (char_type<T>) return format<T>(static_cast<Ts&&>(Args)...);
  else if constexpr (stringable<T>) return format<iter_value_t<T>>(static_cast<Ts&&>(Args)...);
  else return format<char>(static_cast<Ts&&>(Args)...);
}

///--------------------------------------------------------------------------///
/// MARK: print

namespace internal {
const preferred_char _newline[] = {preferred_char('\n'), preferred_char(0)};
}

inline constexpr struct {
  template<typename... As> static void operator()(As&&... as) {
    auto s = format<preferred_char>(static_cast<As&&>(as)...);
    internal::_print<false>(s.c_str(), s.size());
  }
  template<typename... As> static void err(As&&... as) {
    auto s = format<preferred_char>(static_cast<As&&>(as)...);
    internal::_print<true>(s.c_str(), s.size());
  }
} print_inline;

inline constexpr struct {
  static void operator()() { internal::_print<false>(internal::_newline, 1); }
  template<typename... As> static void operator()(As&&... as) {
    auto s = format<preferred_char>(static_cast<As&&>(as)..., internal::_newline);
    internal::_print<false>(s.c_str(), s.size());
  }
  static void err() { internal::_print<true>(internal::_newline, 1); }
  template<typename... As> static void err(As&&... as) {
    auto s = format<preferred_char>(static_cast<As&&>(as)..., internal::_newline);
    internal::_print<true>(s.c_str(), s.size());
  }
} print;
} // namespace yw
