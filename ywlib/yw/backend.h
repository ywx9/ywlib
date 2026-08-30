#pragma once
#include <yw/error.h>

#include <new>

#ifndef YW_DEFAULT_BACKEND
#define YW_DEFAULT_BACKEND cpu
#endif

namespace yw {

template<size_t N> requires(N > 0) struct literal_string {
  char str[N];
  consteval literal_string(const char (&Str)[N]) {
    for (size_t i = 0; i < N; ++i) str[i] = Str[i];
  }
  consteval bool null_terminated() const noexcept { return str[N - 1] == '\0'; }
  consteval bool empty() const noexcept { return N == 1 && str[0] == '\0'; }
  consteval size_t size() const noexcept { return N - null_terminated(); }
  consteval const char* c_str() const noexcept { return str; }
  consteval const char* data() const noexcept { return str; }
  consteval char operator[](size_t index) const noexcept { return str[index]; }
  consteval operator const char*() const noexcept { return str; }
  consteval string_view<char> view() const noexcept { return {str, size()}; }
  consteval const char* begin() const noexcept { return str; }
  consteval const char* end() const noexcept { return str + size(); }
  template<size_t M> consteval bool operator==(const literal_string<M>& Other) const noexcept {
    if (size() != Other.size()) return false;
    for (size_t i = 0; i < size(); ++i)
      if (str[i] != Other[i]) return false;
    return true;
  }
};

template<literal_string Str> struct backend {
  static constexpr literal_string value = Str;
  static consteval string_view<char> to_string() noexcept { return value.view(); }
  template<literal_string OtherStr> consteval bool operator==(const backend<OtherStr>&) const noexcept {
    return backend::value == backend<OtherStr>::value;
  }
};

inline constexpr backend<"cpu"> cpu;
inline constexpr backend<"gpu"> gpu;

inline constexpr auto default_backend = YW_DEFAULT_BACKEND;
} // namespace yw
