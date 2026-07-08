#pragma once
#include "yw/string.h"

/*
# エラーハンドリング設計

## 関数の戻り値型の決め方

```
if (not_fallible) return T;
else if (is_internal_function) return std::expected<T, error>;
else if (is_constructor) {
  prepare("static std::expected<T, error> create(As&&...)");
  return T(As...);
} else if (is_setter_like) {
  if (failed) error::fizzle_out(); // warning
  else T::set(As...);
} else if (return_value_is_essential) { // (e.g. file_handle::file_size())
  if (failed) {
    if (T::has_invalid_value) return T::invalid_value;
    else error::go_off(); // fatal -> exit
  }
  return T;
} else return std::expected<T, error>;
*/

namespace yw {

class error {
public:
  struct kind {
    const char* name;
    constexpr kind() noexcept : name("success") {}
    template<size_t N> constexpr kind(const char (&Name)[N]) : name(Name) {}
    constexpr string_view<char> to_string() const noexcept { return string_view<char>(name); }
    constexpr bool operator==(const kind& other) const noexcept { return name == other.name; }
  };

private:
  inline static error::kind _kind{};
  inline static string<char> _message{};
  inline static int32_t _system_code = 0;
  inline static uint64_t _position = npos;
  inline static std::vector<source_line> _footprints;

  static void _print_error(error::kind Kind, string<char> Message, int32_t SystemCode, uint64_t Position) {
    if (SystemCode != 0) {
      if (Position == npos) print.err(Kind.to_string(), ": ", Message, " (code=", SystemCode, ")");
      else print.err(Kind.to_string(), ": ", Message, " (code=", SystemCode, ", offset=", Position, ")");
    } else if (Position != npos) print.err(Kind.to_string(), ": ", Message, " (offset=", Position, ")");
    else if (Message.empty()) print.err(Kind.to_string());
    else print.err(Kind.to_string(), ": ", Message);
  }

  static void _print_error() { _print_error(_kind, _message, _system_code, _position); }

  static void _print_footprints() {
    if (_footprints.empty()) return;
    print.err("* ", _footprints.front());
    for (size_t i = 1; i < _footprints.size(); ++i) print.err("^ ", _footprints[i]);
  }

  bool _ticking = false;

public:
  constexpr error() noexcept = default;

  constexpr error(
    kind Kind, string<char> Message = {}, int32_t Code = 0, uint64_t Position = npos, const source_line& sl = here()) {
    if (std::is_constant_evaluated()) throw "An error occurred in a constant expression";
    if (!_footprints.empty()) {
      print.err("Error occurred while handling another error");
      print_inline.err("New Error: ");
      _print_error(Kind, Message, Code, Position);
      print.err("* ", sl);
      print_inline.err("Previous Error: ");
      _print_error();
      _print_footprints();
      std::exit(_system_code ? _system_code : EXIT_FAILURE);
    }
    _kind = Kind;
    _message = std::move(Message);
    _system_code = Code;
    _position = Position;
    _footprints.push_back(sl);
    _ticking = true;
  }

  constexpr ~error() {
    if (!_ticking) return;
    // cannnot print in constexpr context, so compile error expected in _ticking state
    print.err("Unhandled error goes off:");
    _print_error();
    _print_footprints();
    std::exit(_system_code ? _system_code : EXIT_FAILURE);
  }

  constexpr error(error&& e) noexcept : _ticking(std::exchange(e._ticking, false)) {}

  constexpr error& operator=(error&& e) noexcept {
    if (this == &e) return *this;
    if (_ticking) {
      print.err("Error object is overwritten while ticking");
      _print_error();
      _print_footprints();
      std::exit(_system_code ? _system_code : EXIT_FAILURE);
    } else _ticking = std::exchange(e._ticking, false);
    return *this;
  }

  explicit constexpr operator bool() const noexcept { return _ticking; }
  static constexpr bool ticking() noexcept { return !_footprints.empty(); }

  /// prints error message and exits program
  void go_off() {
    if (_footprints.empty()) return;
    print_inline.err("Error: ");
    _print_error();
    _print_footprints();
    std::exit(_system_code ? _system_code : EXIT_FAILURE);
    _ticking = false;
  }

  /// prints error message and exits program
  void go_off(const source_line& Add) {
    if (_footprints.empty()) return;
    print_inline.err("Error: ");
    _print_error();
    _footprints.push_back(Add);
    _print_footprints();
    std::exit(_system_code ? _system_code : EXIT_FAILURE);
    _ticking = false;
  }

  /// prints warning message and clears error state
  void fizzle_out() {
    if (_footprints.empty()) return;
    print_inline.err("Warning: ");
    _print_error();
    _print_footprints();
    _ticking = false;
  }

  /// prints warning message and clears error state
  void fizzle_out(const source_line& Add) {
    if (_footprints.empty()) return;
    print_inline.err("Warning: ");
    _print_error();
    _footprints.push_back(Add);
    _print_footprints();
    _ticking = false;
  }

  /// returns std::unexpected<error> while adding footprint to error stack
  constexpr std::unexpected<error> relay(const source_line& Source = here()) & {
    add_footprint(Source);
    return std::unexpected(std::move(*this));
  }

  /// adds footprint to error stack
  constexpr error& add_footprint(const source_line& Source = here()) & {
    if (_ticking) _footprints.push_back(Source);
    return *this;
  }
};

namespace errors {
inline constexpr error::kind unreachable{"unreachable"};
inline constexpr error::kind invalid_argument{"invalid argument"};
inline constexpr error::kind invalid_operation{"invalid operation"};
inline constexpr error::kind operation_failed{"operation failed"};
inline constexpr error::kind not_initialized{"not initialized"};
inline constexpr error::kind already_initialized{"already initialized"};
inline constexpr error::kind warning{"warning"};
} // namespace errors
} // namespace yw

/// MARK: std::formatter

namespace std {

template<> struct formatter<yw::error::kind, char> {
  formatter<basic_string_view<char>, char> fmt;
  constexpr auto parse(auto& ctx) { return fmt.parse(ctx); }
  auto format(const yw::error::kind& k, auto& ctx) const { return fmt.format(k.to_string(), ctx); }
};
} // namespace std
