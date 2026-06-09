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
    if (position != uint64_t(-1)) {
      if (!message.empty()) {
        if (system_code == 0) return yw::format(type.name, ": ", message, " (offset=", position, ")");
        else return yw::format(type.name, ": ", message, " (code=", system_code, ", offset=", position, ")");
      } else if (system_code != 0) return yw::format(type.name, " (code=", system_code, ", offset=", position, ")");
      else return yw::format(type.name, " (offset=", position, ")");
    } else {
      if (!message.empty()) {
        if (system_code == 0) return yw::format(type.name, ": ", message);
        else return yw::format(type.name, ": ", message, " (code=", system_code, ")");
      } else if (system_code != 0) return yw::format(type.name, " (code=", system_code, ")");
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
    yw::print.err("unhandled error destroyed", "\n", to_string(), "\nFootprints:\n", footprint::dump());
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

/// MARK: unexpected_error

inline std::unexpected<error> unexpected_error(error& e) { return std::unexpected(std::move(e)); }
inline std::unexpected<error> unexpected_error(
  decltype(error::type) e, string<char> msg = {}, int32_t sys_code = 0, uint64_t pos = uint64_t(-1)) {
  return std::unexpected(error(e, std::move(msg), sys_code, pos));
}

/// MARK: fatal_error

namespace internal {
[[noreturn]] inline void _fatal_error(const error& e, const char* source_info) {
  print_fallback.err(format("Fatal error: {} at {}\nFootprints:\n{}", e, source_info, footprint::dump()));
  std::exit(e.system_code);
}
} // namespace internal

#define fatal_error(e) ::yw::internal::_fatal_error(e, ywlib_make_source_info)

/// MARK: assume

namespace internal {
template<typename T> T _assume(std::expected<T, yw::error>&& res, const char* source_info) {
  if (!res) {
    print_fallback.err(format("Assumption failed: {} at {}", res.error(), source_info));
    std::exit(res.error().system_code);
  }
  if constexpr (is_void<T>) return;
  else return std::move(*res);
}
} // namespace internal

#define assume(res) ::yw::internal::_assume(std::move(res), ywlib_make_source_info)

} // namespace yw

//////////////////////////////////////// MARK: std::formatter

namespace std {

template<typename C> struct formatter<yw::error, C> {
  formatter<basic_string<C>, C> fmt;
  constexpr auto parse(auto& ctx) { return fmt.parse(ctx); }
  auto format(const yw::error& err, auto& ctx) const { return fmt.format(yw::unicode<C>(err.to_string()), ctx); }
};
} // namespace std
