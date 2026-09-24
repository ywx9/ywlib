#pragma once
#include <core/core.h>
#include <core/result.h>
#include <core/property.h>

namespace yw {

template<std::destructible T> requires same_as<T, remove_cvref<T>> && (!is_none<T>)class optional {
  union _union {
    none _none;
    T _value;
    constexpr ~_union() {}
    constexpr _union() noexcept : _none() {}
  } _union{};

public:
  const_property<bool, optional> has_value = false;

  constexpr ~optional() noexcept {
    if (has_value()) _union._value.~T();
  }

  constexpr optional() noexcept = default;
  constexpr optional(is_none auto) noexcept {}

  constexpr optional(const optional& o) requires constructible<T, const T&> {
    if (o) new (&_union._value) T(o._union._value);
  }

  constexpr optional(optional&& o) noexcept(nt_constructible<T, T&&>) requires constructible<T, T&&> {
    if (o) new (&_union._value) T(static_cast<T&&>(o._union._value));
  }

  template<typename... As> requires constructible<T, As...>
  constexpr optional(As&&... as) noexcept(nt_constructible<T, As...>) {
    new (&_union._value) T(static_cast<As&&>(as)...);
    has_value = true;
  }

  constexpr optional& operator=(is_none auto) noexcept {
    if (has_value()) _union._value.~T();
    has_value = false;
    return *this;
  }

  constexpr optional& operator=(const optional& o) noexcept(
    nt_constructible<T, const T&> && nt_assignable<T&, const T&>)
    requires constructible<T, const T&> && assignable<T&, const T&> {
    if (this == &o) return *this;
    if (has_value() && o.has_value()) _union._value = o._union._value;
    else if (has_value()) _union._value.~T();
    else if (o.has_value()) new (&_union._value) T(o._union._value);
    return *this;
  }

  constexpr optional& operator=(optional&& Other) noexcept(nt_constructible<T, T> && nt_assignable<T&, T>)
    requires constructible<T, T> && assignable<T&, T> {
    if (this == &Other) return *this;
    if (has_value() && Other.has_value()) _union._value = static_cast<T&&>(Other._union._value);
    else if (has_value()) _union._value.~T();
    else if (Other.has_value()) new (&_union._value) T(static_cast<T&&>(Other._union._value));
    return *this;
  }

  template<typename U> requires(!same_as<remove_cvref<U>, none>) && constructible<T, U> && assignable<T&, U>
  constexpr optional& operator=(U&& Value) noexcept(nt_constructible<T, U> && nt_assignable<T&, U>) {
    if constexpr (assignable<T&, U>) {
      if (has_value()) _union._value = static_cast<U&&>(Value);
      else new (&_union._value) T(static_cast<U&&>(Value));
    } else {
      if (has_value()) _union._value.~T();
      new (&_union._value) T(static_cast<U&&>(Value));
    }
    return *this;
  }

  template<typename... As> requires constructible<T, As...>
  constexpr T& emplace(As&&... as) noexcept(nt_constructible<T, As...>) {
    if (has_value()) _union._value.~T();
    new (&_union._value) T(static_cast<As&&>(as)...);
    return _union._value;
  }

  constexpr void reset() noexcept {
    if (!has_value()) return;
    _union._value.~T();
    has_value = false;
  }

  explicit constexpr operator bool() const noexcept { return has_value(); }

  template<typename Self> constexpr auto get_if(this Self&& self) noexcept {
    if (!self.has_value()) return static_cast<copy_cv<remove_ref<Self>, T>*>(nullptr);
    return std::addressof(static_cast<Self&&>(self)._union._value);
  }

  template<typename Self> constexpr auto&& value(this Self&& self) noexcept {
    if (!self.has_value()) yw::error("attempted to access value of empty optional").print_and_abort();
    return static_cast<copy_cvref<Self, T>>(static_cast<Self&&>(self)._union._value);
  }

  template<typename Self> constexpr auto value_or(this Self&& self, T Default) noexcept {
    if (!self.has_value()) return Default;
    else return T(static_cast<copy_cvref<Self, T>>(static_cast<Self&&>(self)._union._value));
  }

  constexpr T& operator*() & noexcept { return value(); }
  constexpr const T& operator*() const & noexcept { return value(); }
  constexpr T&& operator*() && noexcept { return value(); }
  constexpr const T&& operator*() const && noexcept { return value(); }

  constexpr T* operator->() noexcept { return std::addressof(value()); }
  constexpr const T* operator->() const noexcept { return std::addressof(value()); }
};

template<typename T> optional(T) -> optional<T>;

} // namespace yw
