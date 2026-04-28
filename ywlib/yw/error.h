#pragma once
#include "yw/core.h"

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

  int32_t system_code;
  uint64_t position;
  null_terminated<char> message;
  mutable bool handled{true};

  constexpr ~error() noexcept;
  constexpr error() noexcept;
  constexpr error(const error& e);
  constexpr error(error&& e) noexcept;
  constexpr error& operator=(const error& e);
  constexpr error& operator=(error&& e) noexcept;

  explicit constexpr error(
    decltype(error::type) e, null_terminated<char> msg = {}, int32_t sys_code = 0,
    uint64_t pos = uint64_t(-1)) noexcept;

  explicit constexpr operator bool() const noexcept;

  constexpr std::string to_string() const {
    handled = true;
    if (!std::is_constant_evaluated()) {
      if (position != uint64_t(-1)) {
        if (!message.empty()) {
          if (system_code == 0) return std::format("{}: {} (offset={})", type.name, message, position);
          else return std::format("{}: {} (code={}, offset={})", type.name, message, system_code, position);
        } else if (system_code != 0) return std::format("{} (code={}, offset={})", type.name, system_code, position);
        else return std::format("{} (offset={})", type.name, position);
      } else {
        if (!message.empty()) {
          if (system_code == 0) return std::format("{}: {}", type.name, message);
          else return std::format("{}: {} (code={})", type.name, message, system_code);
        } else if (system_code != 0) return std::format("{} (code={})", type.name, system_code);
        else return type.name;
      }
    } else return type.name; // C++26以降でないとconstexpr formatが使えないため簡易に
  }
};

//////////////////////////////////////// MARK: define_error macro

/// adds new error definition in namespace yw::errors
#define define_error(error_name)                                                                       \
  inline constexpr decltype(::yw::error::type) error_name = {#error_name, ::yw::source().unique_id()};

namespace errors {
define_error(success);
define_error(invalid_argument);
define_error(invalid_operation);
define_error(operation_failed);
define_error(not_initialized);
}

constexpr error::~error() noexcept {
  try {
    if (type != errors::success && !handled && !std::is_constant_evaluated()) {
      print("unhandled error destroyed");
      yw::print.err(to_string());
    }
  } catch (...) {} // noexcept destructor
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
  : type(std::exchange(e.type, errors::success)), system_code(std::exchange(e.system_code, 0)),
    position(std::exchange(e.position, uint64_t(-1))), message(std::move(e.message)),
    handled(std::exchange(e.handled, true)) {}

constexpr error& error::operator=(error&& e) noexcept {
  type = std::exchange(e.type, errors::success);
  system_code = std::exchange(e.system_code, 0);
  position = std::exchange(e.position, uint64_t(-1));
  message = std::move(e.message);
  handled = std::exchange(e.handled, true);
  return *this;
}

constexpr error::error(decltype(error::type) e, null_terminated<char> msg, int32_t sys_code, uint64_t pos) noexcept
  : type(e), system_code(sys_code), message(std::move(msg)), position(pos), handled(e == errors::success) {}

constexpr error::operator bool() const noexcept { return type != errors::success; }

////////////////////////////// MARK: error_trace

class error_trace {
public:
  yw::error error;
  std::vector<source> frames;

  constexpr ~error_trace() noexcept {
    try {
      if (error && !error.handled && !std::is_constant_evaluated()) {
        print("unhandled error_trace destroyed");
        yw::print.err(to_string());
      }
    } catch (...) {} // noexcept destructor
  }

  constexpr error_trace() = default;
  constexpr error_trace(const error_trace& e) = default;
  constexpr error_trace& operator=(const error_trace& e) = default;
  constexpr error_trace(error_trace&&) = default;
  constexpr error_trace& operator=(error_trace&&) = default;

  constexpr error_trace(yw::error err, const source& src = {}) : error(std::move(err)) {
    frames.reserve(8);
    frames.push_back(src);
  }

  explicit constexpr operator bool() const noexcept { return bool(error); }

  constexpr error_trace& push(const source& src = {}) & {
    frames.push_back(src);
    return *this;
  }

  constexpr std::string to_string() const {
    auto s = error.to_string();
    if (std::is_constant_evaluated()) {
      for (const auto& src : frames) s += "\n  at " + src.to_string();
    } else
      for (const auto& src : frames) s += std::format("\n  at {}", src);
    return s;
  }
};

//////////////////////////////////////// MARK: unexpected_error

/// creates `std::unexpected<error_trace>` with given information.
inline std::unexpected<error_trace> unexpected_error(
  decltype(error::type) e, null_terminated<char> msg, int32_t sys_code = 0, uint64_t pos = uint64_t(-1),
  const source& src = {}) {
  return std::unexpected<error_trace>(error_trace(yw::error(e, std::move(msg), sys_code, pos), src));
}

/// adds source information (default is current location) to error trace and returns `std::unexpected<error_trace>`.
inline std::unexpected<error_trace> unexpected_error(error_trace& e, const source& src = {}) {
  return std::unexpected<error_trace>(std::move(e.push(src)));
}

/// adds source information (default is current location) to error trace and returns `std::unexpected<error_trace>`.
inline std::unexpected<error_trace> unexpected_error(std::unexpected<error_trace>& e, const source& src = {}) {
  e.error().push(src);
  return std::move(e);
}
inline std::unexpected<error_trace> unexpected_error(std::unexpected<error_trace>&& e, const source& src = {}) {
  e.error().push(src);
  return std::move(e);
}

//////////////////////////////////////// MARK: check_error

inline std::expected<void, error_trace> check_error(std::expected<void, error_trace>&& res, const source& src = {}) {
  if (!res) return unexpected_error(res.error(), src);
  return {};
}

//////////////////////////////////////// MARK: fatal_error

[[noreturn]] inline void fatal_error(const error_trace& e, const source& src = {}) {
  print_fallback.err(format("Fatal error at {}\n{}", src, e));
  std::exit(e.error.system_code);
}

[[noreturn]] inline void fatal_error(const error& e, const source& src = {}) {
  print_fallback.err(format("Fatal error at {}\n{}", src, e));
  std::exit(e.system_code);
}

[[noreturn]] inline void fatal_error(decltype(errors::success) e, null_terminated<char> msg, const source& src = {}) {
  print_fallback.err(format("Fatal error at {}\n{}", src, error(e, std::move(msg))));
  std::exit(1);
}

//////////////////////////////////////// MARK: assume

/// returns value if `res` is valid; otherwise prints error and exits.
template<typename T> requires(!is_void<T>)
[[nodiscard]] T assume(std::expected<T, yw::error_trace>&& res, const source& src = {}) {
  if (!res) {
    print.err(format("Assumption failed\n{}\n  at {}", res.error(), src));
    std::exit(res.error().error.system_code);
  }
  return std::move(*res);
}

/// checks if `res` is valid; if not, prints error and exits.
inline void assume(std::expected<void, yw::error_trace>&& res, const source& src = {}) {
  if (!res) {
    print.err(format("Assumption failed\n{}\n  at {}", res.error(), src));
    std::exit(res.error().error.system_code);
  }
}

template<typename T> requires(!is_void<T> && !specialization_of<T, std::expected>) [[nodiscard]] T&& assume(T&& value) {
  return static_cast<T&&>(value);
}

} // namespace yw

//////////////////////////////////////// MARK: std::formatter

namespace std {

template<typename C> struct formatter<yw::error, C> {
  formatter<basic_string<C>, C> fmt;
  constexpr auto parse(auto& ctx) { return fmt.parse(ctx); }
  auto format(const yw::error& err, auto& ctx) const { return fmt.format(yw::unicode<C>(err.to_string()), ctx); }
};

template<typename C> struct formatter<yw::error_trace, C> {
  formatter<basic_string<C>, C> fmt;
  constexpr auto parse(auto& ctx) { return fmt.parse(ctx); }
  auto format(const yw::error_trace& err, auto& ctx) const { return fmt.format(yw::unicode<C>(err.to_string()), ctx); }
};
} // namespace std
