#pragma once
#include "yw/string.h"

namespace yw {

//////////////////////////////////////// MARK: error

struct error {
  /// error type defined by `define_error`
  struct {
    const char* name;
    uint64_t id;
    constexpr bool operator==(const auto& other) const noexcept
      requires same_as<remove_cvref<decltype(*this)>, remove_cvref<decltype(other)>> {
      return id == other.id;
    }
    int32_t exit_code() const noexcept { return int32_t(uint32_t(id & 0xFFFFFFFF)); }
  } type;
  ministr<char> message;
  uint64_t position;
  int32_t system_code;
  mutable bool handled{true};

  constexpr ~error() noexcept;
  constexpr error() noexcept;
  constexpr error(const error& e);
  constexpr error(error&& e) noexcept;
  constexpr error& operator=(const error& e);
  constexpr error& operator=(error&& e) noexcept;

  explicit constexpr error(
    decltype(error::type) e, ministr<char> msg = {}, int32_t sys_code = 0, uint64_t pos = uint64_t(-1)) noexcept;

  explicit constexpr operator bool() const noexcept;

  constexpr string<char> to_string() const {
    handled = true;
    auto code = system_code ? system_code : type.exit_code();
    if (position != uint64_t(-1)) {
      if (!message.empty()) {
        if (code == 0) return yw::format(type.name, ": ", message, " (offset=", position, ")");
        else return yw::format(type.name, ": ", message, " (code=", code, ", offset=", position, ")");
      } else if (code != 0) return yw::format(type.name, " (code=", code, ", offset=", position, ")");
      else return yw::format(type.name, " (offset=", position, ")");
    } else {
      if (!message.empty()) {
        if (code == 0) return yw::format(type.name, ": ", message);
        else return yw::format(type.name, ": ", message, " (code=", code, ")");
      } else if (code != 0) return yw::format(type.name, " (code=", code, ")");
      else return type.name;
    }
  }
};

//////////////////////////////////////// MARK: define_error macro

/// adds new error definition in namespace yw::errors
#define define_error(error_name) \
  inline constexpr decltype(::yw::error::type) error_name = {#error_name, ::yw::unique_id()};

namespace errors {
define_error(success);
define_error(invalid_argument);
define_error(invalid_operation);
define_error(operation_failed);
define_error(not_initialized);
define_error(unreachable);
} // namespace errors

constexpr error::~error() {
  if (type != errors::success && !handled && !std::is_constant_evaluated()) {
    yw::print.err("Unhandled error goes off: ", to_string());
    footprint::print(true);
    std::exit(system_code ? system_code : type.exit_code());
  }
}

constexpr error::error() noexcept : type(errors::success), system_code(0), position(uint64_t(-1)), message() {}

constexpr error::error(const error& e)
  : type(e.type), system_code(e.system_code), position(e.position), message(e.message),
    handled(type == errors::success) {}

constexpr error& error::operator=(const error& e) {
  type = e.type;
  system_code = e.system_code;
  position = e.position;
  message = e.message;
  handled = (type == errors::success);
  return *this;
}

constexpr error::error(error&& e) noexcept
  : type(std::exchange(e.type, errors::success)), message(std::move(e.message)),
    position(std::exchange(e.position, uint64_t(-1))), system_code(std::exchange(e.system_code, 0)),
    handled(std::exchange(e.handled, true)) {}

constexpr error& error::operator=(error&& e) noexcept {
  type = std::exchange(e.type, errors::success);
  system_code = std::exchange(e.system_code, 0);
  position = std::exchange(e.position, uint64_t(-1));
  message = std::move(e.message);
  handled = std::exchange(e.handled, true);
  return *this;
}

constexpr error::error(decltype(error::type) e, ministr<char> msg, int32_t sys_code, uint64_t pos) noexcept
  : type(e), message(std::move(msg)), position(pos), system_code(sys_code), handled(e == errors::success) {}

constexpr error::operator bool() const noexcept { return type != errors::success; }

/// MARK: fatal_error

[[noreturn]] inline void fatal_error(const error& e) {
  print_fallback.err(format("Fatal error: ", e));
  footprint::print(true);
  std::exit(e.system_code ? e.system_code : e.type.exit_code());
}
} // namespace yw

//////////////////////////////////////// MARK: std::formatter

namespace std {

template<typename C> struct formatter<yw::error, C> {
  formatter<basic_string<C>, C> fmt;
  constexpr auto parse(auto& ctx) { return fmt.parse(ctx); }
  auto format(const yw::error& err, auto& ctx) const { return fmt.format(yw::unicode<C>(err.to_string()), ctx); }
};
} // namespace std
