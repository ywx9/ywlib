#pragma once
#include <yw/core.h>

namespace yw {

template<std::destructible T> class optional {
  union {
    none _none;
    T _data;
  };
  bool _has_value = false;

public:
  optional() noexcept = default;
  optional(const optional&) = delete;
  optional& operator=(const optional&) = delete;

  constexpr ~optional() noexcept {
    reset();
  }

  template<typename... Ts> requires(std::constructible_from<T, Ts...>) explicit optional(Ts&&... Args) {
    _data = T(std::forward<Ts>(Args)...);
    _has_value = true;
  }


  constexpr void reset() noexcept {
    if (!_has_value) return;
    _data.~T();
    _has_value = false;
  }
};
}
