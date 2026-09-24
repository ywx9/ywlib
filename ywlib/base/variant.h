#pragma once
#include <core/core.h>
#include <core/result.h>

namespace yw {

namespace internal {
template<typename... Ts> union _variant_union;
template<> union _variant_union<> {
  none _none;
  constexpr ~_variant_union() noexcept {}
  constexpr _variant_union() noexcept : _none() {}
};
template<typename T, typename... Ts> union _variant_union<T, Ts...> {
  T _value;
  _variant_union<Ts...> _rest;
  constexpr ~_variant_union() noexcept {}
  constexpr _variant_union() noexcept : _rest() {}
  template<size_t I, typename Self> constexpr auto&& get(this Self&& self) noexcept {
    if constexpr (I == 0) return static_cast<copy_cvref<Self, T>>(self._value);
    else return self._rest.template get<I - 1>();
  }
};
} // namespace internal

template<is_object... Ts> requires((same_as<Ts, remove_cvref<Ts>> && ...)) class variant {
  internal::_variant_union<Ts...> _union;
  size_t _index = npos;

  static constexpr bool _copyable = (constructible<Ts, const Ts&> && ...);
  static constexpr bool _movable = (constructible<Ts, Ts> && ...);

  static constexpr bool _nt_copyable = (nt_constructible<Ts, const Ts&> && ...);
  static constexpr bool _nt_movable = (nt_constructible<Ts, Ts> && ...);

  template<typename T> static consteval bool _unique_type() noexcept {
    return count(same_as<remove_cvref<T>, Ts>...) == 1;
  }

  template<typename T> static consteval size_t _get_index() noexcept {
    return inspect(same_as<remove_cvref<T>, Ts>...);
  }

  template<size_t I> using _get_type = select_type<I, Ts...>;

  template<size_t I, typename... As> constexpr bool _try_construct(As&&... as) noexcept {
    ywlib_try {
      new (&_union) _get_type<I>(static_cast<As&&>(as)...);
      _index = I;
      return true;
    } ywlib_catch(...) {
      _index = npos;
      return false;
    }
  }

  template<size_t I = 0> constexpr void _destroy() noexcept {
    if constexpr (I < sizeof...(Ts)) {
      if (_index == I) _union.template get<I>().~_get_type<I>(), _index = npos;
      else _destroy<I + 1>();
    } else _index = npos;
  }

  template<size_t I = 0> constexpr void _copy_from(const variant& o) noexcept(_nt_copyable) {
    if constexpr (I < sizeof...(Ts)) {
      if (o._index == I) {
        new (&_union) _get_type<I>(o._union.template get<I>());
        _index = I;
      } else _copy_from<I + 1>(o);

    } else _index = npos;
  }

  template<size_t I = 0> constexpr void _move_from(variant&& o) noexcept(_nt_movable) {
    if constexpr (I < sizeof...(Ts)) {
      if (o._index == I) {
        new (&_union) _get_type<I>(move(o._union.template get<I>()));
        _index = I;
      } else _move_from<I + 1>(move(o));
    } else _index = npos;
  }

  template<size_t I = 0, typename Self, typename F> constexpr decltype(auto) _visit(this Self&& self, F&& f) {
    if constexpr (I < sizeof...(Ts)) {
      if constexpr (I == 0)
        return invoke(static_cast<F&&>(f), static_cast<copy_cvref<Self, _get_type<I>>>(self._union.template get<I>()));
      else return self.template _visit<I + 1>(static_cast<F&&>(f));
    } else error(errors::invalid_operation, "variant index mismatch").print_and_abort();
  }

public:
  constexpr variant() noexcept = default;
  constexpr ~variant() noexcept { _destroy<>(); }

  variant(const variant&) requires(!_copyable) = delete;
  variant(variant&&) requires(!_movable) = delete;
  variant& operator=(const variant&) requires(!_copyable) = delete;
  variant& operator=(variant&&) requires(!_movable) = delete;

  constexpr variant(const variant& o) noexcept(_nt_copyable) requires _copyable { _copy_from<>(o); }
  constexpr variant(variant&& o) noexcept(_nt_movable) requires _movable { _move_from<>(move(o)); }

  template<typename T> requires(_unique_type<T>() && constructible<remove_cvref<T>, T>)
  constexpr variant(T&& v) noexcept(nt_constructible<remove_cvref<T>, T>) {
    _try_construct<_get_index<T>()>(static_cast<T&&>(v));
  }

  constexpr variant& operator=(const variant& o) noexcept(_nt_copyable) requires _copyable {
    if (this == &o) return *this;
    if (_index != npos) _destroy<>();
    _copy_from<>(o);
    return *this;
  }

  constexpr variant& operator=(variant&& o) noexcept(_nt_movable) requires _movable {
    if (this == &o) return *this;
    if (_index != npos) _destroy<>();
    _move_from<>(move(o));
    return *this;
  }

  template<typename T> requires(_unique_type<T>() && constructible<remove_cvref<T>, T>)
  constexpr variant& operator=(T&& v) noexcept(nt_constructible<remove_cvref<T>, T>) {
    if (_index != npos) _destroy<>();
    _try_construct<_get_index<T>()>(static_cast<T&&>(v));
    return *this;
  }

  template<size_t I, typename... As> requires(I < sizeof...(Ts) && constructible<_get_type<I>, As...>)
  constexpr bool emplace(As&&... as) noexcept(nt_constructible<_get_type<I>, As...>) {
    if (_index != npos) _destroy<>();
    return _try_construct<I>(static_cast<As&&>(as)...);
  }

  constexpr void reset() noexcept {
    if (_index != npos) _destroy<>();
  }

  constexpr size_t index() const noexcept { return _index; }
  constexpr bool has_value() const noexcept { return _index != npos; }
  constexpr bool empty() const noexcept { return _index == npos; }

  template<size_t I, typename Self> requires(I < sizeof...(Ts)) constexpr auto* get_if(this Self& self) noexcept {
    if (self._index == I) return &self._union.template get<I>();
    else return nullptr;
  }

  template<typename T, typename Self> requires(_unique_type<T>()) constexpr auto* get_if(this Self& self) noexcept {
    return self.template get_if<_get_index<T>()>();
  }

  template<size_t I, typename Self> requires(I < sizeof...(Ts)) constexpr auto&& get(this Self&& self) noexcept {
    if (self._index != I) error(errors::invalid_operation, "variant index mismatch").print_and_abort();
    return static_cast<copy_cvref<Self, _get_type<I>>>(self._union.template get<I>());
  }

  template<typename T, typename Self> requires(_unique_type<T>()) constexpr auto&& get(this Self&& self) noexcept {
    return static_cast<Self&&>(self).template get<_get_index<T>()>();
  }

  template<typename F, typename Self> requires requires {
    requires sizeof...(Ts) > 0;
    requires(invocable<F, copy_cvref<Self, Ts>> && ...);
    requires same_as<invoke_result<F, copy_cvref<Self, Ts...[0]>>, invoke_result<F, copy_cvref<Self, Ts>>...>;
  } constexpr decltype(auto) visit(this Self&& self, F&& f) {
    return static_cast<Self&&>(self).template _visit<0>(static_cast<F&&>(f));
  }
};

template<typename F, typename V> concept visitable = requires { declval<V>().visit(declval<F>()); };

inline constexpr auto visit = []<typename V, visitable<V> F>(F&& f, V&& v) //
{ return static_cast<V&&>(v).template visit(static_cast<F&&>(f)); };
} // namespace yw
