#pragma once
#include <core/core.h>

namespace yw {

// clang-format off

///--------------------------------------------------------------------------///
/// MARK: property with callback

/// public property which triggers a callback on modification
template<typename T, typename Class, invocable Callback = noop> class property {
  friend Class;
  T _;
  Callback _cb;

  constexpr T& ref() noexcept { return _; }
  constexpr const T& cref() const noexcept { return _; }
  constexpr Callback& callback() noexcept { return _cb; }
  constexpr const Callback& callback() const noexcept { return _cb; }

public:
  constexpr property() noexcept = default;
  constexpr property(const T& v) noexcept : _(v) {}
  constexpr property(T&& v) noexcept : _(move(v)) {}
  constexpr property& operator=(const T& v) noexcept(nt_assignable<T, const T&> && nt_invocable<Callback>) { return _ = v, _cb(), *this; }
  constexpr property& operator=(T&& v) noexcept(nt_assignable<T, T> && nt_invocable<Callback>) { return _ = move(v), _cb(), *this; }
  constexpr operator T&() noexcept { return _; }
  constexpr operator const T&() const noexcept { return _; }
  constexpr T* operator->() noexcept { return &_; }
  constexpr const T* operator->() const noexcept { return &_; }
  constexpr T& operator*() noexcept { return _; }
  constexpr const T& operator*() const noexcept { return _; }
  constexpr T& operator()() noexcept { return _; }
  constexpr const T& operator()() const noexcept { return _; }
  template<convertible_to<T> U> constexpr void operator()(U&& v) noexcept(nt_convertible_to<U, T> && nt_invocable<Callback>) { _ = T(v), _cb(); }
  constexpr property& operator++() noexcept(noexcept(++_, _cb())) requires requires { ++_; } { return ++_, _cb(), *this; }
  constexpr property& operator--() noexcept(noexcept(--_, _cb())) requires requires { --_; } { return --_, _cb(), *this; }
  constexpr property& operator+=(const T& v) noexcept(noexcept(_ += v, _cb())) requires requires { _ += v; } { return _ += v, _cb(), *this; }
  constexpr property& operator-=(const T& v) noexcept(noexcept(_ -= v, _cb())) requires requires { _ -= v; } { return _ -= v, _cb(), *this; }
  constexpr property& operator*=(const T& v) noexcept(noexcept(_ *= v, _cb())) requires requires { _ *= v; } { return _ *= v, _cb(), *this; }
  constexpr property& operator/=(const T& v) noexcept(noexcept(_ /= v, _cb())) requires requires { _ /= v; } { return _ /= v, _cb(), *this; }
};

///--------------------------------------------------------------------------///
/// MARK: property without callback

/// public property which is structural type if `T` is a structural type
template<typename T, typename Class> class property<T, Class, noop> {
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
  T _;
  constexpr const_property(const T& v) noexcept(nt_constructible<T, const T&>) requires constructible<T, const T&> : _(v) {}
  constexpr const_property(T&& v) noexcept(nt_constructible<T, T>) requires constructible<T, T> : _(move(v)) {}
  constexpr operator const T&() const noexcept { return _; }
  constexpr const T* operator->() const noexcept { return &_; }
  constexpr const T& operator*() const noexcept { return _; }
  constexpr const T& operator()() const noexcept { return _; }
};
} // namespace yw
