#pragma once
#include <yw/error.h>
#include <yw/tuple.h>

namespace yw {

template<typename... Ts> union variant_base {
  select_type<0, Ts...> _member;
  using _rest_type = typepack<Ts...>::template back<sizeof...(Ts) - 1>::template expand<variant_base>;
  _rest_type _rest_member;

  constexpr ~variant_base() {}

  template<size_t I, typename... As> requires(I == 0)
  constexpr variant_base(index<I>, As&&... Args) : _member(static_cast<As&&>(Args)...) {}

  template<size_t I, typename... As> requires(I != 0)
  constexpr variant_base(index<I>, As&&... Args) : _rest_member(index<I - 1>(), static_cast<As&&>(Args)...) {}

  template<size_t I, typename Self> constexpr copy_cvref<Self&&, select_type<I, Ts...>> get(this Self&& self) noexcept {
    if constexpr (I == 0) return static_cast<Self&&>(self)._member;
    else return static_cast<Self&&>(self)._rest_member.template get<I - 1>();
  }
};

template<typename T> union variant_base<T> {
  T _member;

  constexpr ~variant_base() {}

  template<size_t I, typename... As> constexpr variant_base(index<I>, As&&... Args)
    : _member(static_cast<As&&>(Args)...) {}

  template<size_t I, typename Self> constexpr copy_cvref<Self&&, T> get(this Self&& self) noexcept {
    return static_cast<Self&&>(self)._member;
  }
};

template<is_object... Ts> requires((same_as<Ts, remove_cvref<Ts>> && ...)) class variant {
  variant_base<none, Ts...> _data = variant_base<none, Ts...>(yw::index<0>());
  size_t _index = 0;

  template<typename T> static constexpr bool _unique_type = [] {
    if constexpr (sizeof...(Ts) == 0) return false;
    else return count<same_as<remove_cvref<T>, Ts>...> == 1;
  }();

  template<typename T> static constexpr size_t _type_index = inspect<same_as<remove_cvref<T>, Ts>...>;

  template<size_t I> using _type = select_type<I, Ts...>;

  template<size_t I, typename... As> constexpr variant(yw::index<I>, As&&... Args)
    : _data(yw::index<I>(), static_cast<As&&>(Args)...), _index(I) {}

  template<size_t I, typename... As> constexpr bool _try_construct(As&&... Args) noexcept {
    _destroy();
    std::destroy_at(&_data);
    try {
      std::construct_at(&_data, yw::index<I>(), static_cast<As&&>(Args)...);
      _index = I;
      return I != 0;
    } catch (...) {
      std::construct_at(&_data, yw::index<0>());
      _index = 0;
      return false;
    }
  }

  template<size_t I = 0> constexpr void _destroy_value() noexcept {
    if constexpr (I < sizeof...(Ts)) {
      if (_index == I + 1) std::destroy_at(std::addressof(_data.template get<I + 1>()));
      else _destroy_value<I + 1>();
    }
  }

  constexpr void _destroy() noexcept {
    if (_index == 0) return;
    _destroy_value();
    _index = 0;
  }

  template<size_t I = 0> constexpr void _copy_from(const variant& Other) noexcept {
    if constexpr (I < sizeof...(Ts)) {
      if (Other._index == I + 1) _try_construct<I + 1>(Other._data.template get<I + 1>());
      else _copy_from<I + 1>(Other);
    }
  }

  template<size_t I = 0> constexpr void _move_from(variant&& Other) noexcept {
    if constexpr (I < sizeof...(Ts)) {
      if (Other._index == I + 1) _try_construct<I + 1>(static_cast<variant&&>(Other)._data.template get<I + 1>());
      else _move_from<I + 1>(static_cast<variant&&>(Other));
    }
  }

  template<size_t I, typename Self> static constexpr decltype(auto) _get(Self&& self) {
    return static_cast<Self&&>(self)._data.template get<I>();
  }

  template<size_t I, typename F, typename Self> static constexpr decltype(auto) _visit(Self&& self, F&& f) {
    if constexpr (I + 1 == sizeof...(Ts)) return invoke(static_cast<F&&>(f), _get<I + 1>(static_cast<Self&&>(self)));
    else if (self._index == I + 1) return invoke(static_cast<F&&>(f), _get<I + 1>(static_cast<Self&&>(self)));
    else return _visit<I + 1>(static_cast<Self&&>(self), static_cast<F&&>(f));
  }

public:
  constexpr variant() noexcept = default;

  constexpr ~variant() noexcept { _destroy(); }

  constexpr variant(const variant& Other) noexcept requires((constructible<Ts, const Ts&> && ...)) {
    _copy_from(Other);
  }
  constexpr variant(const variant&) requires(!((constructible<Ts, const Ts&> && ...))) = delete;

  constexpr variant(variant&& Other) noexcept requires((constructible<Ts, Ts &&> && ...)) {
    _move_from(static_cast<variant&&>(Other));
  }
  constexpr variant(variant&&) requires(!((constructible<Ts, Ts &&> && ...))) = delete;

  template<typename T> requires(_unique_type<T> && constructible<_type<_type_index<T>>, T>)
  constexpr variant(T&& Value) noexcept {
    _try_construct<_type_index<T> + 1>(static_cast<T&&>(Value));
  }

  constexpr variant& operator=(const variant& Other) noexcept requires((constructible<Ts, const Ts&> && ...)) {
    if (this != &Other) {
      _destroy();
      _copy_from(Other);
    }
    return *this;
  }
  constexpr variant& operator=(const variant&) requires(!((constructible<Ts, const Ts&> && ...))) = delete;

  constexpr variant& operator=(variant&& Other) noexcept requires((constructible<Ts, Ts &&> && ...)) {
    if (this != &Other) {
      _destroy();
      _move_from(static_cast<variant&&>(Other));
    }
    return *this;
  }
  constexpr variant& operator=(variant&&) requires(!((constructible<Ts, Ts &&> && ...))) = delete;

  template<typename T> requires(_unique_type<T> && constructible<_type<_type_index<T>>, T>)
  constexpr variant& operator=(T&& Value) noexcept {
    constexpr size_t I = _type_index<T> + 1;
    if constexpr (assignable<_type<_type_index<T>>&, T>) {
      if (_index == I) {
        try {
          _data.template get<I>() = static_cast<T&&>(Value);
          return *this;
        } catch (...) {
          _destroy();
          std::destroy_at(&_data);
          std::construct_at(&_data, yw::index<0>());
          _index = 0;
          return *this;
        }
      }
    }
    _try_construct<I>(static_cast<T&&>(Value));
    return *this;
  }

  template<size_t I, typename... As> requires(I < sizeof...(Ts) && constructible<_type<I>, As...>)
  static constexpr variant create(As&&... Args) noexcept {
    try {
      return variant(yw::index<I + 1>(), static_cast<As&&>(Args)...);
    } catch (...) { return variant(); }
  }

  template<size_t I, typename... As> requires(I < sizeof...(Ts) && constructible<_type<I>, As...>)
  constexpr bool emplace(As&&... Args) noexcept {
    return _try_construct<I + 1>(static_cast<As&&>(Args)...);
  }

  constexpr void reset() noexcept {
    _destroy();
    std::destroy_at(&_data);
    std::construct_at(&_data, yw::index<0>());
  }

  constexpr size_t index() const noexcept { return _index == 0 ? npos : _index - 1; }
  constexpr bool has_value() const noexcept { return _index != 0; }
  constexpr bool empty() const noexcept { return _index == 0; }

  template<size_t I> requires(I < sizeof...(Ts))
  constexpr auto get_if(this auto&& self) noexcept -> copy_cv<remove_ref<decltype(self)>, select_type<I, Ts...>>* {
    if (self._index != I + 1) return nullptr;
    return std::addressof(static_cast<decltype(self)&&>(self)._data.template get<I + 1>());
  }

  template<typename T, typename Self> requires(count<same_as<T, Ts>...> == 1)
  constexpr copy_cv<remove_ref<Self>, select_type<inspect<same_as<T, Ts>...>, Ts...>>* get_if(
    this Self&& self) noexcept {
    return static_cast<Self&&>(self).template get_if<inspect<same_as<T, Ts>...>>();
  }

  template<size_t I, typename Self> requires(I < sizeof...(Ts))
  constexpr copy_cvref<Self, select_type<I, Ts...>> get(this Self&& self) {
    if (auto p = static_cast<Self&&>(self).template get_if<I>())
      return static_cast<copy_cvref<Self, select_type<I, Ts...>>>(*p);
    error(errors::invalid_operation, "variant index mismatch").go_off();
    return static_cast<Self&&>(self)._data.template get<I + 1>();
  }

  template<typename T, typename Self> requires(_unique_type<T>)
  constexpr copy_cvref<Self&&, _type<_type_index<T>>> get(this Self&& self) {
    return static_cast<Self&&>(self).template get<_type_index<T>>();
  }

  template<typename F, typename Self> requires requires {
    requires(std::invocable<F, Ts> && ...);
    requires same_as<std::invoke_result_t<F, select_type<0, Ts...>>, std::invoke_result_t<F, Ts>...>;
  } constexpr decltype(auto) visit(this Self&& self, F&& f) {
    using R = std::invoke_result_t<F, select_type<0, Ts...>>;
    if constexpr (is_void<R>) {
      if (self._index == 0) return none();
      else return _visit<0>(static_cast<Self&&>(self), static_cast<F&&>(f));
    } else if constexpr (variation_of<R, std::expected<void, error>>) {
      if constexpr (is_void<typename R::value_type> || same_as<typename R::value_type, none>) {
        if (self._index == 0) return R{};
        else return _visit<0>(static_cast<Self&&>(self), static_cast<F&&>(f));
      } else if constexpr (same_as<typename R::error_type, error>) {
        if (self._index == 0) return std::unexpected(error(errors::invalid_operation, "empty variant"));
        else return _visit<0>(static_cast<Self&&>(self), static_cast<F&&>(f));
      } else {
        if (self._index == 0) {
          error(errors::invalid_operation, "empty variant").go_off();
          std::exit(EXIT_FAILURE);
        } else return _visit<0>(static_cast<Self&&>(self), static_cast<F&&>(f));
      }
    } else {
      if (self._index == 0) {
        error(errors::invalid_operation, "empty variant").go_off();
        std::exit(EXIT_FAILURE);
      }
      return _visit<0>(static_cast<Self&&>(self), static_cast<F&&>(f));
    }
  }
};

template<typename F, specialization_of<variant> V> constexpr decltype(auto) visit(F&& f, V&& v) {
  return static_cast<V&&>(v).template visit(static_cast<F&&>(f));
}
} // namespace yw
