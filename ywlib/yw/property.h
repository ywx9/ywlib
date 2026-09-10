#pragma once
#include <yw/core.h>

namespace yw {

/// type for public property
template<typename T, typename Class, invocable Callback = noop> class property {
  friend Class;
  T value;
  Callback callback;

public:
  constexpr property() noexcept = default;
  constexpr property(const T& v) noexcept : value(v) {}
  constexpr property(T&& v) noexcept : value(move(v)) {}
  constexpr property& operator=(const T& v) noexcept { return value = v, callback(), *this; }
  constexpr property& operator=(T&& v) noexcept { return value = move(v), callback(), *this; }
  constexpr operator T&() noexcept { return value; }
  constexpr operator const T&() const noexcept { return value; }
  constexpr T* operator->() noexcept { return &value; }
  constexpr const T* operator->() const noexcept { return &value; }
  constexpr T& operator*() noexcept { return value; }
  constexpr const T& operator*() const noexcept { return value; }
  constexpr T& operator()() noexcept { return value; }
  constexpr const T& operator()() const noexcept { return value; }
  template<convertible_to<T> U> constexpr void operator()(U&& v) noexcept(nt_convertible_to<U, T>) {
    value = static_cast<T>(v);
    callback();
  }

  constexpr property& operator++() noexcept requires requires { ++value; } { return ++value, callback(), *this; }
  constexpr property& operator--() noexcept requires requires { --value; } { return --value, callback(), *this; }

  constexpr property& operator+=(const T& v) noexcept requires requires { value += v; } {
    return value += v, callback(), *this;
  }
  constexpr property& operator-=(const T& v) noexcept requires requires { value -= v; } {
    return value -= v, callback(), *this;
  }
  constexpr property& operator*=(const T& v) noexcept requires requires { value *= v; } {
    return value *= v, callback(), *this;
  }
  constexpr property& operator/=(const T& v) noexcept requires requires { value /= v; } {
    return value /= v, callback(), *this;
  }
};

/// type for public const property
template<typename T, typename Class, invocable Callback = noop> class const_property {
  friend Class;
  T value;
  Callback callback;

  constexpr const_property() noexcept = default;
  constexpr const_property& operator=(const T& v) noexcept { return value = v, callback(), *this; }
  constexpr const_property& operator=(T&& v) noexcept { return value = move(v), callback(), *this; }

public:
  constexpr const_property(const T& v) noexcept : value(v) {}
  constexpr const_property(T&& v) noexcept : value(move(v)) {}
  constexpr operator const T&() const noexcept { return value; }
  constexpr const T* operator->() const noexcept { return &value; }
  constexpr const T& operator*() const noexcept { return value; }
  constexpr const T& operator()() const noexcept { return value; }
};
} // namespace yw
