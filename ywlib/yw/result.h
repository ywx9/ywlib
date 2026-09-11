#pragma once
#include <yw/error.h>
#include <yw/optional.h>
#include <yw/tuple.h>

namespace yw {

template<typename... Ts> union variant_base {
  select_type<0, Ts...> _m;
  using _rest_type = typepack<Ts...>::template back<sizeof...(Ts) - 1>::template expand<variant_base>;
  _rest_type _rest;

  constexpr variant_base() noexcept {}
  constexpr ~variant_base() {}

  template<size_t I, typename... As> requires(I == 0)
  constexpr variant_base(index<I>, As&&... Args) : _m(static_cast<As&&>(Args)...) {}

  template<size_t I, typename... As> requires(I != 0)
  constexpr variant_base(index<I>, As&&... Args) : _rest(index<I - 1>(), static_cast<As&&>(Args)...) {}

  template<size_t I, typename Self> requires(I < sizeof...(Ts))
  constexpr copy_cvref<Self&&, select_type<I, Ts...>> get(this Self&& self) noexcept {
    if constexpr (I == 0) return static_cast<copy_cvref<Self&&, select_type<I, Ts...>>>(self._m);
    else return static_cast<copy_cvref<Self&&, select_type<I, Ts...>>>(self._rest.template get<I - 1>());
  }

  template<size_t I> constexpr void destroy(index<I>) noexcept {
    if constexpr (I == 0) std::destroy_at(std::addressof(_m));
    else _rest.destroy(index<I - 1>());
  }

  template<size_t I, typename... As> constexpr bool construct(index<I>, As&&... Args) noexcept {
    try {
      if constexpr (I == 0) std::construct_at(std::addressof(_m), static_cast<As&&>(Args)...);
      else std::construct_at(std::addressof(_rest), index<I - 1>(), static_cast<As&&>(Args)...);
      return true;
    } catch (...) { return false; }
  }
};

template<typename T> union variant_base<T> {
  T _m;

  constexpr variant_base() noexcept {}
  constexpr ~variant_base() {}

  template<size_t I, typename... As> requires(I == 0)
  constexpr variant_base(index<I>, As&&... Args) : _m(static_cast<As&&>(Args)...) {}

  template<size_t I, typename Self> requires(I == 0) constexpr copy_cvref<Self&&, T> get(this Self&& self) noexcept {
    return static_cast<copy_cvref<Self&&, T>>(self._m);
  }

  template<size_t I> constexpr void destroy(index<I>) noexcept {
    static_assert(I == 0);
    std::destroy_at(std::addressof(_m));
  }

  template<size_t I, typename... As> constexpr bool construct(index<I>, As&&... Args) noexcept {
    static_assert(I == 0);
    try {
      std::construct_at(std::addressof(_m), static_cast<As&&>(Args)...);
      return true;
    } catch (...) { return false; }
  }
};

namespace errors {
inline constexpr error::kind invalid_error_access{"invalid error access"};
inline constexpr error::kind invalid_value_access{"invalid value access"};
} // namespace errors

template<typename T> class result;

template<> class result<void> {
  variant_base<yw::none, yw::error> _var;
  bool _has_error = false;

  constexpr void _destroy() noexcept {
    if (_has_error) _var.destroy(index<1>());
    _has_error = false;
  }

  template<typename Self> constexpr void _move_construct(this Self& self, Self&& other) noexcept {
    if (other._has_error) {
      if (!self._var.construct(index<1>(), move(other._var).template get<1>()))
        yw::error(errors::operation_failed, "failed to move result<void> error").go_off();
      self._has_error = true;
    } else self._var.construct(index<0>());
  }

public:
  using value_type = void;
  using error_type = yw::error;

  constexpr result() noexcept : _var(index<0>()) {}

  template<typename... Ts> requires(sizeof...(Ts) > 0 && constructible<yw::error, Ts...>)
  constexpr result(Ts&&... ts) noexcept(nt_constructible<yw::error, Ts...>)
    : _var(index<1>(), static_cast<Ts&&>(ts)...), _has_error(true) {}

  constexpr ~result() noexcept { _destroy(); }

  result(const result&) = delete;
  result& operator=(const result&) = delete;

  constexpr result(result&& other) noexcept { _move_construct(static_cast<result&&>(other)); }

  constexpr result& operator=(result&& other) noexcept {
    if (this == &other) return *this;
    _destroy();
    _move_construct(static_cast<result&&>(other));
    return *this;
  }

  explicit constexpr operator bool() const noexcept { return has_value(); }
  constexpr bool operator!() const noexcept { return !has_value(); }
  constexpr bool has_value() const noexcept { return !_has_error; }
  constexpr bool has_error() const noexcept { return _has_error; }

  template<typename Self> constexpr copy_ref<Self&&, yw::error> error(this Self&& self) noexcept {
    if (!self.has_error()) yw::error(errors::invalid_error_access, "No error present in result<void>").go_off();
    return static_cast<Self&&>(self)._var.template get<1>();
  }

  template<typename Self> constexpr remove_ref<Self>&& relay(this Self&& self) noexcept {
    if (self.has_error()) self._var.template get<1>().add_footprint();
    return move(self);
  }

  template<typename To, typename Self> constexpr result<To> relay(this Self& self) noexcept {
    if (!self.has_error()) yw::error(errors::invalid_error_access).go_off();
    return result<To>::fail(move(self.error()));
  }

  static constexpr result success() noexcept { return {}; }

  template<typename... Ts> requires constructible<yw::error, Ts...>
  static constexpr result fail(Ts&&... ts) noexcept(nt_constructible<yw::error, Ts...>) {
    return result(static_cast<Ts&&>(ts)...);
  }
};

template<typename T> requires(!is_void<T>) class result<T> {
  variant_base<yw::none, T, yw::error> _var;
  bool _has_value = false;
  bool _has_error = false;

  constexpr void _destroy() noexcept {
    if (_has_value) _var.destroy(index<1>());
    else if (_has_error) _var.destroy(index<2>());
    _has_value = false;
    _has_error = false;
  }

  template<typename... Ts> requires constructible<T, Ts...> constexpr result<void> _emplace_value(Ts&&... ts) noexcept {
    if (!_var.construct(index<1>(), static_cast<Ts&&>(ts)...))
      return yw::error(errors::operation_failed, "failed to construct result value");
    _has_value = true;
    return result<void>{};
  }

  template<typename... Ts> requires constructible<yw::error, Ts...>
  constexpr result<void> _emplace_error(Ts&&... ts) noexcept {
    if (!_var.construct(index<2>(), static_cast<Ts&&>(ts)...))
      return yw::error(errors::operation_failed, "failed to construct result error");
    _has_error = true;
    return result<void>{};
  }

public:
  using value_type = T;
  using error_type = yw::error;

  constexpr result() noexcept : _var(index<0>()) {}

  template<typename... Ts> requires constructible<T, Ts...> && (!constructible<yw::error, Ts...>)constexpr result(
    Ts&&... ts) noexcept(nt_constructible<T, Ts...>)
    : _var(index<1>(), static_cast<Ts&&>(ts)...), _has_value(true) {}

  template<typename... Ts> requires constructible<yw::error, Ts...> && (!constructible<T, Ts...>)constexpr result(
    Ts&&... ts) noexcept(nt_constructible<yw::error, Ts...>)
    : _var(index<2>(), static_cast<Ts&&>(ts)...), _has_error(true) {}

  constexpr ~result() noexcept { _destroy(); }

  result(const result&) = delete;
  result& operator=(const result&) = delete;

  constexpr result(result&& other) noexcept {
    if (other._has_value) {
      if (auto res = _emplace_value(static_cast<result&&>(other)._var.template get<1>()); !res)
        res.error().go_off(here());
    } else if (other._has_error) {
      if (auto res = _emplace_error(static_cast<result&&>(other)._var.template get<2>()); !res)
        res.error().go_off(here());
    } else _var.construct(index<0>());
  }

  constexpr result& operator=(result&& other) noexcept {
    if (this == &other) return *this;
    _destroy();
    if (other._has_value) {
      if (auto res = _emplace_value(static_cast<result&&>(other)._var.template get<1>()); !res)
        res.error().go_off(here());
    } else if (other._has_error) {
      if (auto res = _emplace_error(static_cast<result&&>(other)._var.template get<2>()); !res)
        res.error().go_off(here());
    } else _var.construct(index<0>());
    return *this;
  }

  explicit constexpr operator bool() const noexcept { return has_value(); }
  constexpr bool operator!() const noexcept { return !has_value(); }
  constexpr bool has_value() const noexcept { return _has_value; }
  constexpr bool has_error() const noexcept { return _has_error; }

  template<typename Self> constexpr copy_ref<Self&&, T> value(this Self&& self) noexcept {
    if (!self.has_value()) yw::error(errors::invalid_value_access, "No value present in result<T>").go_off();
    return static_cast<Self&&>(self)._var.template get<1>();
  }

  template<typename Self> constexpr copy_ref<Self&&, yw::error> error(this Self&& self) noexcept {
    if (!self.has_error()) yw::error(errors::invalid_error_access, "No error present in result<T>").go_off();
    return static_cast<Self&&>(self)._var.template get<2>();
  }

  template<typename Self> constexpr copy_ref<Self&&, T> operator*(this Self&& self) noexcept {
    return static_cast<Self&&>(self).value();
  }

  constexpr T* operator->() noexcept { return std::addressof(value()); }
  constexpr const T* operator->() const noexcept { return std::addressof(value()); }

  template<typename Self> constexpr result&& relay(this Self&& self) noexcept {
    if (self.has_error()) self._var.template get<2>().add_footprint();
    return move(self);
  }

  template<typename To, typename Self> constexpr result<To> relay(this Self& self) noexcept {
    if (!self.has_error()) yw::error(errors::invalid_error_access).go_off();
    return result<To>::fail(move(self.error()));
  }

  template<typename... Ts> requires constructible<T, Ts...>
  static constexpr result success(Ts&&... ts) noexcept(nt_constructible<T, Ts...>) {
    result res;
    if (auto r = res._emplace_value(static_cast<Ts&&>(ts)...); !r) r.error().go_off(here());
    return res;
  }

  template<typename... Ts> requires constructible<yw::error, Ts...>
  static constexpr result fail(Ts&&... ts) noexcept(nt_constructible<yw::error, Ts...>) {
    result res;
    if (auto r = res._emplace_error(static_cast<Ts&&>(ts)...); !r) r.error().go_off(here());
    return res;
  }
};

template<typename T> inline constexpr auto success = []<typename... Ts>(Ts&&... ts) //
  noexcept(is_void<T> || nt_constructible<T, Ts...>) requires(is_void<T> || constructible<T, Ts...>)
{ return result<T>::success(static_cast<Ts&&>(ts)...); };

template<typename T> inline constexpr auto fail = []<typename... Ts>(Ts&&... ts) //
  noexcept(nt_constructible<yw::error, Ts...>) requires constructible<yw::error, Ts...>
{ return result<T>::fail(static_cast<Ts&&>(ts)...); };

/// Unwraps a value whose success is assumed.
/// @return The unwrapped value.
/// @warning This function terminates the program when the assumption is violated.
template<typename T> requires is_rvref<T&&> constexpr auto assume(T&& t, const source_line& sl = here()) {
  using U = remove_cvref<T>;
  if constexpr (specialization_of<U, result>) {
    if (!t) t.error().go_off(sl);
    if constexpr (is_void<typename U::value_type>) return none();
    else return std::move(*t);
  } else if constexpr (specialization_of<U, optional>) {
    if (!t) error(errors::invalid_operation, "assume() called on empty optional").go_off(sl);
    return std::move(*t);
  } else if constexpr (is_pointer<U>) {
    if (!t) error(errors::invalid_operation, "assume() called on null pointer").go_off(sl);
    return std::move(*t);
  } else return std::move(t);
}
} // namespace yw
