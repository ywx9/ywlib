#pragma once
#include "yw/core.h"

namespace yw {

//////////////////////////////////////// MARK: errors

enum class errors : uint32_t {
  success = 0,
  invalid_argument,  // part of `invalid_operation`; ex) argument out of range
  invalid_file,      // unexpected file format
  invalid_operation, // wrong use of API; ex) file_handle::write called on read-only file
  operation_failed,  // unexpected error from system or library
  not_initialized,   // part of `invalid_operation`; system/object is not initialized
  rendering_during_drawing,
  drawing_during_rendering,
};

//////////////////////////////////////// MARK: error_text

inline constexpr auto error_text = [](errors e) -> std::string_view {
  if (e == errors::success) return "success";
  else if (e == errors::invalid_argument) return "invalid argument";
  else if (e == errors::invalid_file) return "invalid file format";
  else if (e == errors::invalid_operation) return "invalid operation";
  else if (e == errors::operation_failed) return "operation failed";
  else if (e == errors::not_initialized) return "not-initialized object or system is accessed";
  else if (e == errors::rendering_during_drawing) return "3D rendering function called during 2D drawing";
  else if (e == errors::drawing_during_rendering) return "2D drawing function called during 3D rendering";
  else return "unknown error";
};

//////////////////////////////////////// MARK: error

struct error {
  errors code;
  int32_t system_code;
  uint64_t position;
  null_terminated<char> message;
  mutable bool handled{true};

  ~error() noexcept {
    try {
      if (code != errors::success && !handled) {
        print("unhandled error destroyed");
        yw::print.err(to_string());
      }
    } catch (...) {} // noexcept destructor
  }

  error() noexcept : code(errors::success), system_code(0), position(uint64_t(-1)), message() {}

  error(const error& e)
    : code(e.code), system_code(e.system_code), position(e.position), message(e.message),
      handled(code == errors::success) {}

  error& operator=(const error& e) {
    code = e.code;
    system_code = e.system_code;
    position = e.position;
    message = e.message;
    handled = (code == errors::success);
    return *this;
  }

  error(error&& e) noexcept
    : code(std::exchange(e.code, errors::success)), system_code(std::exchange(e.system_code, 0)),
      position(std::exchange(e.position, uint64_t(-1))), message(std::move(e.message)),
      handled(std::exchange(e.handled, true)) {}

  error& operator=(error&& e) noexcept {
    code = std::exchange(e.code, errors::success);
    system_code = std::exchange(e.system_code, 0);
    position = std::exchange(e.position, uint64_t(-1));
    message = std::move(e.message);
    handled = std::exchange(e.handled, true);
    return *this;
  }

  explicit error(errors e, null_terminated<char> msg = {}, int32_t sys_code = 0, uint64_t pos = uint64_t(-1)) noexcept
    : code(e), system_code(sys_code), message(std::move(msg)), position(pos), handled(e == errors::success) {}

  explicit operator bool() const noexcept { return code != errors::success; }

  std::string to_string() const {
    std::string s;
    if (!message.empty()) {
      if (system_code == 0) s = std::format("{}", message);
      else s = std::format("{} (code={})", message, system_code);
    } else if (system_code != 0) s = std::format("error code={}", system_code);
    if (position != uint64_t(-1)) s += std::format("\n  input offset={}", position);
    handled = true;
    return s;
  }
};

////////////////////////////// MARK: error_trace

class error_trace {
public:
  yw::error error;
  std::vector<source> frames;

  ~error_trace() noexcept {
    try {
      if (error && !error.handled) {
        print("unhandled error_trace destroyed");
        yw::print.err(to_string());
      }
    } catch (...) {} // noexcept destructor
  }

  error_trace() = default;
  error_trace(const error_trace& e) = default;
  error_trace& operator=(const error_trace& e) = default;
  error_trace(error_trace&&) = default;
  error_trace& operator=(error_trace&&) = default;

  error_trace(yw::error err, const source& src = {}) : error(std::move(err)) {
    frames.reserve(8);
    frames.push_back(src);
  }

  explicit operator bool() const noexcept { return bool(error); }

  error_trace& push(const source& src = {}) & {
    frames.push_back(src);
    return *this;
  }

  std::string to_string() const {
    auto s = error.to_string();
    for (const auto& src : frames) s += std::format("\n  at {}", src);
    return s;
  }
};

//////////////////////////////////////// MARK: unexpected_error

/// creates `std::unexpected<error_trace>` with given information.
inline std::unexpected<error_trace> unexpected_error(
  errors e, null_terminated<char> msg, int32_t sys_code = 0, uint64_t pos = uint64_t(-1), const source& src = {}) {
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

//////////////////////////////////////// MARK: print_error

inline constexpr struct {
  /// prints error message and error trace to standard error output.
  template<stringable S> static int operator()(S&& message, const error_trace& err, const source& src = {}) {
    print.err(format("{}\n at {}\n{}", err.to_string(), src, message));
    return err.error.code == errors::success ? 0 : err.error.system_code;
  }

  /// prints error message and error trace to standard error output.
  template<stringable S> static int operator()(const error_trace& err, const source& src = {}) {
    return operator()(error_text(err.error.code), err, src);
  }
} print_error;

//////////////////////////////////////// MARK: assume

/// returns value if `res` is valid; otherwise prints error and exits.
template<typename T> requires(!is_void<T>)
[[nodiscard]] T assume(std::expected<T, yw::error_trace>&& res, const source& src = {}) {
  if (!res) std::exit(print_error("Assumption failed", res.error(), src));
  return std::move(*res);
}

/// checks if `res` is valid; if not, prints error and exits.
inline void assume(std::expected<void, yw::error_trace>&& res, const source& src = {}) {
  if (!res) std::exit(print_error("Assumption failed", res.error(), src));
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
