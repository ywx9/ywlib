#pragma once

namespace yw {

template<typename T> struct wrapper {
  const T value;
  template<typename U = T> constexpr wrapper(U&& v) : value(static_cast<U&&>(v)) {}
  template<typename U, int N> constexpr wrapper(const U (&v)[N]) : value() {
    for (int i = 0; i < N; ++i) value[i] = v[i];
  }
};

template<typename T> wrapper(T&&) -> wrapper<T>;
template<typename T, int N> wrapper(const T (&)[N]) -> wrapper<T[N]>;

}
