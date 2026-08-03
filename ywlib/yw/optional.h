#pragma once
#include <yw/core.h>

namespace yw {

template<std::destructible T> requires same_as<T, remove_cvref<T>> && (!is_none<T>)class optional {
  union storage {
    none _none;
    T _value;
    constexpr ~storage() {}
    constexpr storage() noexcept : _none{} {}
    constexpr storage(none) noexcept : _none{} {}
    template<typename... As> requires constructible<T, As...>
    constexpr storage(As&&... Args) noexcept(nt_constructible<T, As...>) : _value(static_cast<As&&>(Args)...) {}
  } _data{};

  bool _has_value = false;

  template<typename... As> constexpr void _construct(As&&... Args) noexcept(nt_constructible<T, As...>)
    requires constructible<T, As...> {
    std::construct_at(std::addressof(_data._value), static_cast<As&&>(Args)...);
    _has_value = true;
  }

public:
  constexpr optional() noexcept = default;
  constexpr optional(none) noexcept {}

  constexpr optional(const optional& Other) requires constructible<T, const T&> {
    if (Other) _construct(Other._data._value);
  }
  constexpr optional(const optional&) requires(!constructible<T, const T&>) = delete;

  constexpr optional(optional&& Other) noexcept(nt_constructible<T, T&&>) requires constructible<T, T&&> {
    if (Other) _construct(static_cast<T&&>(Other._data._value));
  }
  constexpr optional(optional&&) requires(!constructible<T, T &&>) = delete;

  template<typename U> requires different_from<remove_cvref<U>, optional, none> && constructible<T, U>
  constexpr optional(U&& Value) noexcept(nt_constructible<T, U>) : _data(static_cast<U&&>(Value)), _has_value(true) {}

  template<typename... As> requires(sizeof...(As) != 1 && constructible<T, As...>)
  constexpr optional(As&&... Args) noexcept(nt_constructible<T, As...>)
    : _data(static_cast<As&&>(Args)...), _has_value(true) {}

  constexpr ~optional() noexcept { reset(); }

  constexpr optional& operator=(none) noexcept {
    reset();
    return *this;
  }

  constexpr optional& operator=(const optional& Other) requires constructible<T, const T&> && assignable<T&, const T&> {
    if (this == &Other) return *this;
    if (_has_value && Other._has_value) _data._value = Other._data._value;
    else if (_has_value) reset();
    else if (Other._has_value) _construct(Other._data._value);
    return *this;
  }
  constexpr optional& operator=(const optional& Other)
    requires constructible<T, const T&> && (!assignable<T&, const T&>) {
    if (this == &Other) return *this;
    reset();
    if (Other._has_value) _construct(Other._data._value);
    return *this;
  }
  constexpr optional& operator=(const optional&) requires(!constructible<T, const T&>) = delete;

  constexpr optional& operator=(optional&& Other) requires constructible<T, T&&> && assignable<T&, T&&> {
    if (this == &Other) return *this;
    if (_has_value && Other._has_value) _data._value = static_cast<T&&>(Other._data._value);
    else if (_has_value) reset();
    else if (Other._has_value) _construct(static_cast<T&&>(Other._data._value));
    return *this;
  }
  constexpr optional& operator=(optional&& Other) requires constructible<T, T&&> && (!assignable<T&, T &&>) {
    if (this == &Other) return *this;
    reset();
    if (Other._has_value) _construct(static_cast<T&&>(Other._data._value));
    return *this;
  }
  constexpr optional& operator=(optional&&) requires(!constructible<T, T &&>) = delete;

  template<typename U> requires different_from<remove_cvref<U>, optional, none> && constructible<T, U>
  constexpr optional& operator=(U&& Value) {
    if constexpr (assignable<T&, U>) {
      if (_has_value) _data._value = static_cast<U&&>(Value);
      else _construct(static_cast<U&&>(Value));
    } else {
      reset();
      _construct(static_cast<U&&>(Value));
    }
    return *this;
  }

  template<typename... As> requires constructible<T, As...> constexpr T& emplace(As&&... Args) {
    reset();
    _construct(static_cast<As&&>(Args)...);
    return _data._value;
  }

  constexpr void reset() noexcept {
    if (!_has_value) return;
    std::destroy_at(std::addressof(_data._value));
    std::construct_at(std::addressof(_data._none));
    _has_value = false;
  }

  constexpr bool has_value() const noexcept { return _has_value; }
  constexpr bool empty() const noexcept { return !_has_value; }
  explicit constexpr operator bool() const noexcept { return _has_value; }

  constexpr auto get_if(this auto&& self) noexcept {
    if (!self._has_value) return static_cast<copy_cv<remove_ref<decltype(self)>, T>*>(nullptr);
    return std::addressof(static_cast<decltype(self)&&>(self)._data._value);
  }

  constexpr decltype(auto) value(this auto&& self) noexcept {
    return static_cast<copy_cvref<decltype(self), T>>(static_cast<decltype(self)&&>(self)._data._value);
  }

  constexpr auto value_or(this auto&& self, T Default) noexcept {
    if (!self._has_value) return Default;
    else return T(static_cast<copy_cvref<decltype(self), T>>(static_cast<decltype(self)&&>(self)._data._value));
  }

  constexpr auto operator*(this auto&& self) noexcept {
    return static_cast<decltype(self)&&>(self).value();
  }

  constexpr auto operator->() noexcept { return std::addressof(_data._value); }
  constexpr auto operator->() const noexcept { return std::addressof(_data._value); }
};

template<typename T> optional(T) -> optional<T>;

} // namespace yw
