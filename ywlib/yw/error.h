#pragma once
#include "yw/core.h"

#include <source_location>

namespace yw {

/// error codes used in ywlib
enum class errors : uint32_t {
  success = 0,
  invalid_argument,
  operation_failed,
  not_initialized,
};

/// represents an error condition in ywlib
struct error {
  std::string_view message;
  std::string_view source_file;
  uint32_t source_line;
  errors code;
  error(errors c, std::string_view msg, const std::source_location& loc = std::source_location::current())
    : message(msg), source_file(loc.file_name()), source_line(static_cast<int>(loc.line())), code(c) {}
};

/// makes an error object from an error code
#define yw_make_error(code) yw::error(code, #code)

} // namespace yw

namespace std {

template<typename C> struct formatter<yw::error, C> {
  formatter<basic_string<C>, C> fmt;
  constexpr auto parse(auto& ctx) { return fmt.parse(ctx); }
  auto format(const yw::error& err, auto& ctx) const {
    return fmt.format(std::format("{}({}): {}", err.source_file, err.source_line, err.message), ctx);
  }
};
} // namespace std
