#pragma once
#include "yw/unicode.h"

#include <filesystem>
#include <format>
#include <optional>
#include <print>
#include <string>
#include <string_view>

#ifdef _WIN32
#include <windows.h>
#endif

namespace yw {

//////////////////////////////////////// MARK: format

inline constexpr struct {
  template<typename T> requires(!stringable<T>)
  static std::basic_string<char> operator()(const T& a) {
    if constexpr (same_as<T, std::filesystem::path>) return unicode<char>(a.native());
    return std::format("{}", a);
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
      auto s = std::vformat(bitcast<std::string_view>(std::basic_string_view<C>(fmt)), std::make_format_args(as...));
      return string<C>(reinterpret_cast<std::basic_string<C>&&>(mv(s)));
    } else if constexpr (same_as<C, char16_t>) {
      auto s = std::vformat(bitcast<std::wstring_view>(std::basic_string_view<C>(fmt)), std::make_wformat_args(as...));
      return string<C>(reinterpret_cast<std::basic_string<C>&&>(mv(s)));
    } else if constexpr (same_as<C, char32_t>) {
      const auto f{unicode<char>(fmt)};
      return unicode<C>(std::vformat(f, std::make_format_args(as...)));
    } else throw "yw::format: unsupported character type";
  }
} format;

//////////////////////////////////////// MARK: print_inline

inline constexpr struct {
  template<typename S, typename... Ts> static void operator()(S&& fmt, Ts&&... as) {
#ifdef _WIN32
    const auto s = unicode<wchar_t>(format(static_cast<S&&>(fmt), static_cast<Ts&&>(as)...));
    ::WriteConsoleW(::GetStdHandle(STD_OUTPUT_HANDLE), s.data(), unsigned(s.size()), nullptr, nullptr);
#else
    const auto s = unicode<char>(format(static_cast<S&&>(fmt), static_cast<Ts&&>(as)...));
    std::fputs((const char*)s.data(), stdout);
#endif
  }
} print_inline;

//////////////////////////////////////// MARK: print

inline constexpr struct {
  static void operator()() {
#ifdef _WIN32
    static constexpr wchar_t nl[] = L"\n";
    ::WriteConsoleW(::GetStdHandle(STD_OUTPUT_HANDLE), nl, 1, nullptr, nullptr);
#else
    std::fputc('\n', stdout);
#endif
  }
  template<typename S, typename... Ts> static void operator()(S&& fmt, Ts&&... as) {
    print_inline(static_cast<S&&>(fmt), static_cast<Ts&&>(as)...);
    operator()();
  }
} print;

//////////////////////////////////////// MARK: format_with_location

class format_with_location {
  std::source_location _fail_site;
  std::optional<std::source_location> _call_site{};

  static bool _same_location(const std::source_location& a, const std::source_location& b) noexcept {
    return a.file_name() == b.file_name() && a.line() == b.line() && a.column() == b.column();
  }

public:
  format_with_location(const std::source_location& call_site = std::source_location::current(),
                       const std::source_location& fail_site = std::source_location::current())
    : _fail_site(fail_site) {
    if (!_same_location(call_site, fail_site)) _call_site = call_site;
  }

  template<stringable<char> S, typename... Ts> std::string operator()(S&& fmt, Ts&&... as) const {
    if (_call_site)
      return format("{}({}): {}\n <- {}({})\n", _fail_site.file_name(), _fail_site.line(), format(fmt, as...),
                    _call_site->file_name(), _call_site->line());
    else return format("{}({}): {}\n", _fail_site.file_name(), _fail_site.line(), format(fmt, as...));
  }
};

//////////////////////////////////////// MARK: print_with_location

class print_with_location {
  std::source_location _fail_site;
  std::optional<std::source_location> _call_site{};
  bool _handled{false};

  static bool _same_location(const std::source_location& a, const std::source_location& b) noexcept {
    return a.file_name() == b.file_name() && a.line() == b.line() && a.column() == b.column();
  }

public:
  print_with_location(const std::source_location& call_site = std::source_location::current(),
                      const std::source_location& fail_site = std::source_location::current())
    : _fail_site(fail_site) {
    if (!_same_location(call_site, fail_site)) _call_site = call_site;
  }

  ~print_with_location() noexcept {
    try {
      if (_handled) return;
      if (_call_site)
        print_fallback("{}({}): no message attached\n <- {}({})\n", _fail_site.file_name(), _fail_site.line(),
                       _call_site->file_name(), _call_site->line());
      else print_fallback("{}({}): no message attached\n", _fail_site.file_name(), _fail_site.line());
    } catch (...) { print("print_with_location: exception thrown during destruction\n"); }
  }

  template<stringable<char> S, typename... Ts> void operator()(S&& fmt, Ts&&... as) && noexcept {
    _handled = true;
    try {
      if (_call_site)
        print_fallback("{}({}): {}\n <- {}({})\n", _fail_site.file_name(), _fail_site.line(), format(fmt, as...),
                       _call_site->file_name(), _call_site->line());
      else print_fallback("{}({}): {}\n", _fail_site.file_name(), _fail_site.line(), format(fmt, as...));
    } catch (...) { print("print_with_location: exception thrown during operator()\n"); }
  }
};
} // namespace yw
