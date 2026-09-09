#pragma once
#include <yw/core.h>

namespace yw {

/// type for public property
template<typename T, typename Class> class property {
  friend Class;
  T value;

public:
  constexpr property() noexcept = default;
  constexpr property(const T& Value) noexcept : value(Value) {}
  constexpr property(T&& Value) noexcept : value(std::move(Value)) {}

  constexpr property& operator=(const T& Value) noexcept {
    value = Value;
    return *this;
  }
  constexpr property& operator=(T&& Value) noexcept {
    value = std::move(Value);
    return *this;
  }

  constexpr operator T&() noexcept { return value; }
  constexpr operator const T&() const noexcept { return value; }

  constexpr T* operator->() noexcept { return &value; }
  constexpr const T* operator->() const noexcept { return &value; }

  constexpr T& operator*() noexcept { return value; }
  constexpr const T& operator*() const noexcept { return value; }
};

/// type for const property
template<typename T, typename Class> class const_property {
  friend Class;
  T value;

  constexpr const_property() noexcept = default;

  constexpr const_property& operator=(const T& Value) noexcept {
    value = Value;
    return *this;
  }
  constexpr const_property& operator=(T&& Value) noexcept {
    value = std::move(Value);
    return *this;
  }
public:

  constexpr const_property(const T& Value) noexcept : value(Value) {}
  constexpr const_property(T&& Value) noexcept : value(std::move(Value)) {}

  constexpr operator const T&() const noexcept { return value; }

  constexpr const T* operator->() const noexcept { return &value; }

  constexpr const T& operator*() const noexcept { return value; }
};
} // namespace yw
