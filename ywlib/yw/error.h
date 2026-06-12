#pragma once
#include "yw/string.h"

namespace yw {

class error {
public:
  struct kind {
  public:
    const char* name;
    template<size_t N> constexpr kind(const char (&Name)[N]) : name(Name) {}
    constexpr bool operator==(const kind& other) const noexcept { return name == other.name; }
    constexpr string_view<char> to_string() const noexcept { return string_view<char>(name); }
  };

  struct slot {
    error::kind kind;
    string<char> message;
    int32_t system_code;
    uint64_t position;
    std::vector<source_line> locations;

    constexpr string<char> to_string() const {
      string<char> result;
      if (system_code != 0) {
        if (position == npos) result = yw::format(kind, ": ", message, " (code=", system_code, ")");
        else result = yw::format(kind, ": ", message, " (code=", system_code, ", offset=", position, ")");
      } else if (position != npos) result = yw::format(kind, ": ", message, " (offset=", position, ")");
      else result = yw::format(kind, ": ", message);
      if (!locations.empty()) {
        result += yw::format("\n* ", locations.front());
        for (size_t i = 1; i < locations.size(); ++i) result += yw::format("\n^ ", locations[i]);
      }
      return result;
    }

    void print() const {
      if (system_code != 0) {
        if (position == npos) yw::print.err(kind.to_string(), ": ", message, " (code=", system_code, ")");
        else yw::print.err(kind.to_string(), ": ", message, " (code=", system_code, ", offset=", position, ")");
      } else if (position != npos) yw::print.err(kind.to_string(), ": ", message, " (offset=", position, ")");
      else yw::print.err(kind.to_string(), ": ", message);
      if (locations.empty()) return;
      yw::print.err("* ", locations.front());
      for (size_t i = 1; i < locations.size(); ++i) yw::print.err("^ ", locations[i]);
    }
  };

private:
  inline static std::unique_ptr<slot> _current;
  bool _ticking = false;

public:
  constexpr error() noexcept = default;

  constexpr error(
    error::kind Kind, string<char> Message = {}, int32_t Code = 0, uint64_t Position = npos,
    const source_line& Source = {}) {
    if (_current) {
      print.err("Error occurred while handling another error");
      print.err("New Error: ");
      slot new_error{Kind, std::move(Message), Code, Position};
      new_error.locations.push_back(Source);
      new_error.print();
      print.err("Previous Error: ");
      _current->print();
      std::exit(_current->system_code ? _current->system_code : EXIT_FAILURE);
    }
    _current = std::make_unique<slot>(slot{Kind, std::move(Message), Code, Position});
    _current->locations.push_back(Source);
    _ticking = true;
  }

  constexpr ~error() {
    if (!_current || !_ticking) return;
    print.err("Unhandled error goes off:");
    _current->print();
    std::exit(_current->system_code ? _current->system_code : EXIT_FAILURE);
  }

  constexpr error(error&& e) noexcept : _ticking(std::exchange(e._ticking, false)) {}

  constexpr error& operator=(error&& e) noexcept {
    if (this != &e) _ticking = std::exchange(e._ticking, false);
    return *this;
  }

  explicit constexpr operator bool() const noexcept { return _current != nullptr; }

  constexpr void sleep() noexcept {
    _ticking = false;
    _current.reset();
  }

  void print_as_warning() const {
    print_inline.err("Warning: ");
    _current->print();
  }
  void print_as_warning(bool Sleep) {
    print_as_warning();
    if (Sleep) sleep();
  }

  void print_as_fatal() const {
    print_inline.err("Fatal Error: ");
    if (!_current) std::exit(EXIT_FAILURE);
    _current->print();
    std::exit(_current->system_code ? _current->system_code : EXIT_FAILURE);
  }

  constexpr string<char> to_string() const {
    if (!_current) return {};
    return _current->to_string();
  }

  constexpr std::unexpected<error> relay(const source_line& Source = {}) & {
    add_footprint(Source);
    return std::unexpected(std::move(*this));
  }

  constexpr error& add_footprint(const source_line& Source = {}) {
    if (_current) _current->locations.push_back(Source);
    return *this;
  }
};

namespace errors {
inline constexpr error::kind success{"success"};
inline constexpr error::kind unreachable{"unreachable"};
inline constexpr error::kind invalid_argument{"invalid argument"};
inline constexpr error::kind invalid_operation{"invalid operation"};
inline constexpr error::kind operation_failed{"operation failed"};
inline constexpr error::kind not_initialized{"not initialized"};
} // namespace errors
} // namespace yw

namespace yw {} // namespace yw

//////////////////////////////////////// MARK: std::formatter

namespace std {

template<typename C> struct formatter<yw::error, C> {
  formatter<basic_string<C>, C> fmt;
  constexpr auto parse(auto& ctx) { return fmt.parse(ctx); }
  auto format(const yw::error& err, auto& ctx) const { return fmt.format(yw::unicode<C>(err.to_string()), ctx); }
};
} // namespace std
