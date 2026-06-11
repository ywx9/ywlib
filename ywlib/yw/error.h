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
    std::vector<string<char>> locations;

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
  constexpr error(
    error::kind Kind, string<char> Message = {}, int32_t SystemCode = 0, uint64_t Position = npos,
    const source_line& Source = {}) {
    if (_current) {
      print.err("Error occurred while handling another error");
      print.err("New Error: ");
      slot new_error{Kind, std::move(Message), SystemCode, Position};
      new_error.locations.push_back(Source.to_string());
      new_error.print();
      print.err("Previous Error: ");
      _current->print();
      std::exit(_current->system_code ? _current->system_code : EXIT_FAILURE);
    }
    _current = std::make_unique<slot>(slot{Kind, std::move(Message), SystemCode, Position});
    _current->locations.push_back(Source.to_string());
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

  void relay(string_view<char> Source = source_name(), uint32_t Line = source_line()) {
    auto location = source_line(Source, Line);
    if (_current) _current->locations.push_back(std::move(location));
  }

  string<char> to_string() const {
    if (!_current) return {};
    const auto& c = *_current;
    if (c.system_code != 0) {
      if (c.position == npos) return yw::format(c.kind, ": ", c.message, " (code=", c.system_code, ")");
      else return yw::format(c.kind, ": ", c.message, " (code=", c.system_code, ", offset=", c.position, ")");
    } else if (c.position != npos) return yw::format(c.kind, ": ", c.message, " (offset=", c.position, ")");
    else return yw::format(c.kind, ": ", c.message);
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
