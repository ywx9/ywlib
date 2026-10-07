#pragma once
#include <core/core.h>

namespace yw {

// clang-format off

///--------------------------------------------------------------------------///
/// MARK: property

/// public property which is structural type if `T` is a structural type
template<typename T, typename Class> class property {
  friend Class;
  constexpr T& ref() noexcept { return _; }
  constexpr const T& cref() const noexcept { return _; }

public:
  T _{};

  constexpr property() = default;
  constexpr property(const T& v) noexcept(nt_constructible<T, const T&>) requires constructible<T, const T&> : _(v) {}
  constexpr property(T&& v) noexcept(nt_constructible<T, T>) requires constructible<T, T> : _(move(v)) {}
  constexpr property& operator=(const T& v) noexcept(nt_assignable<T, const T&>) requires assignable<T, const T&> { return _ = v, *this; }
  constexpr property& operator=(T&& v) noexcept(nt_assignable<T, T>) requires assignable<T, T> { return _ = move(v), *this; }
  constexpr operator T&() noexcept { return _; }
  constexpr operator const T&() const noexcept { return _; }
  constexpr T* operator->() noexcept { return &_; }
  constexpr const T* operator->() const noexcept { return &_; }
  constexpr T& operator*() noexcept { return _; }
  constexpr const T& operator*() const noexcept { return _; }
  constexpr T& operator()() noexcept { return _; }
  constexpr const T& operator()() const noexcept { return _; }
  template<convertible_to<T> U> constexpr void operator()(U&& v) noexcept(nt_convertible_to<U, T>) { _ = static_cast<T>(v); }
  constexpr property& operator++() noexcept requires requires { ++_; } { return ++_, *this; }
  constexpr property& operator--() noexcept requires requires { --_; } { return --_, *this; }
  constexpr property& operator+=(const T& v) noexcept requires requires { _ += v; } { return _ += v, *this; }
  constexpr property& operator-=(const T& v) noexcept requires requires { _ -= v; } { return _ -= v, *this; }
  constexpr property& operator*=(const T& v) noexcept requires requires { _ *= v; } { return _ *= v, *this; }
  constexpr property& operator/=(const T& v) noexcept requires requires { _ /= v; } { return _ /= v, *this; }
};

///--------------------------------------------------------------------------///
/// MARK: const_property

/// public const property
template<typename T, typename Class> class const_property {
  friend Class;
  constexpr const_property() requires constructible<T> = default;
  constexpr const_property& operator=(const T& v) noexcept(nt_assignable<T, const T&>) requires assignable<T, const T&> { return _ = v, *this; }
  constexpr const_property& operator=(T&& v) noexcept(nt_assignable<T, T>) requires assignable<T, T> { return _ = move(v), *this; }
  constexpr T& ref() noexcept { return _; }
  constexpr const T& cref() const noexcept { return _; }

public:
  T _{};
  constexpr const_property(const T& v) noexcept(nt_constructible<T, const T&>) requires constructible<T, const T&> : _(v) {}
  constexpr const_property(T&& v) noexcept(nt_constructible<T, T>) requires constructible<T, T> : _(move(v)) {}
  constexpr operator const T&() const noexcept { return _; }
  constexpr const T* operator->() const noexcept { return &_; }
  constexpr const T& operator*() const noexcept { return _; }
  constexpr const T& operator()() const noexcept { return _; }
};
} // namespace yw
