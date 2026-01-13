#pragma once
#include "yw/core.h"
#include "yw/unicode.h"

#include <stacktrace>
#include <stdexcept>

namespace yw {

class error : public std::exception {
  std::string _what;

public:
  std::stacktrace stack;

  template<stringable<char> S> error(S&& msg, std::stacktrace st = std::stacktrace::current()) : stack(std::move(st)) {
    std::ostringstream oss;
    oss << std::string_view(msg) << "\nStack trace:\n" << stack;
    _what = oss.str();
  }

  const char* what() const noexcept override {
    try {
      return _what.c_str();
    } catch (...) { return "yw::error: failed to retrieve what() message"; }
  }
};
} // namespace yw
