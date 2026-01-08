#pragma once
#include "yw/core.h"

namespace yw {

template<auto... Vs> struct sequence;

namespace _ {
template<typename T, typename U> struct _to_sequence : std::type_identity<void> {};
template<template<auto...> typename Tm, auto... Vs, typename U> struct _to_sequence<Tm<Vs...>, U>
  : std::type_identity<sequence<static_cast<U>(Vs)...>> {};
template<template<typename T, T...> typename Tm, typename T, T... Vs, typename U>
struct _to_sequence<Tm<T, Vs...>, U> : std::type_identity<sequence<static_cast<U>(Vs)...>> {};
template<template<typename T, T...> typename Tm, typename T, T... Vs>
struct _to_sequence<Tm<T, Vs...>, none> : std::type_identity<sequence<Vs...>> {};
template<template<auto...> typename Tm, auto... Vs> struct _to_sequence<Tm<Vs...>, none>
  : std::type_identity<sequence<Vs...>> {};
} // namespace _

template<typename T, typename U = none> using to_sequence = _::_to_sequence<T, U>::type;
template<typename T, typename U = none> concept is_sequence = !is_void<to_sequence<T, U>>;

namespace _ {
template<typename Sq, size_t N> inline constexpr bool _indices_for{false};
template<size_t... Is, size_t N>
inline constexpr bool _indices_for<sequence<Is...>, N>{(sizeof...(Is) < N)};
} // namespace _

template<typename Sq, typename Tp> concept indices_for =
  _::_indices_for<to_sequence<Sq, size_t>, extent<Tp>>;

namespace _ {
template<size_t I, size_t N, auto P, auto... Vs> struct _make_sequence
  : _make_sequence<I + 1, N, P, Vs..., P(I)> {};
template<size_t N, auto P, auto... Vs> struct _make_sequence<N, N, P, Vs...>
  : std::type_identity<sequence<Vs...>> {};
} // namespace _

template<size_t Begin, size_t End, auto Proj = pass{}>
requires(Begin <= End) && invocable<decltype(Proj), size_t>
using make_sequence = _::_make_sequence<Begin, End, Proj>::type;
template<typename T> using make_indices_for = make_sequence<0, extent<T>>;

namespace _ {
template<typename S, size_t... Is> struct _extracting_indices : std::type_identity<void> {};
template<bool... Bs> struct _extracting_indices<sequence<Bs...>>
  : _extracting_indices<sequence<Bs...>, 0, sizeof...(Bs)> {};
template<bool... Bs, size_t I, size_t N, size_t... Is>
struct _extracting_indices<sequence<Bs...>, I, N, Is...>
  : select_type<select_value<I, Bs...>, _extracting_indices<sequence<Bs...>, I + 1, N, Is..., I>,
                _extracting_indices<sequence<Bs...>, I + 1, N, Is...>> {};
template<bool... Bs, size_t N, size_t... Is>
struct _extracting_indices<sequence<Bs...>, N, N, Is...> : std::type_identity<sequence<Is...>> {};
} // namespace _

template<is_sequence<bool> S> using extracting_indices =
  _::_extracting_indices<to_sequence<S, bool>>::type;

namespace _ {
template<typename S, typename T> struct _sequence_extract : std::type_identity<void> {};
template<auto... Vs, size_t... Is> struct _sequence_extract<sequence<Vs...>, sequence<Is...>>
  : std::type_identity<sequence<select_value<Is, Vs...>...>> {};
template<typename S, typename T> struct _sequence_append : std::type_identity<void> {};
template<auto... Vs, auto... Ws> struct _sequence_append<sequence<Vs...>, sequence<Ws...>>
  : std::type_identity<sequence<Vs..., Ws...>> {};
} // namespace _

template<auto... Vs> struct sequence {
  static constexpr size_t count{sizeof...(Vs)};
  template<size_t I> requires(I < sizeof...(Vs))
  static constexpr auto at = select_value<I, Vs...>;
  template<size_t I> requires(I < sizeof...(Vs))
  using type_at = select_type<I, decltype(Vs)...>;
  template<indices_for<sequence> Ix> using extract = _::_sequence_extract<sequence, Ix>;
  template<size_t N> requires(N <= sizeof...(Vs))
  using fore = extract<make_sequence<0, N>>;
  template<size_t N> requires(N <= sizeof...(Vs))
  using back = extract<make_sequence<sizeof...(Vs) - N, sizeof...(Vs)>>;
  template<is_sequence Sq> using append = _::_sequence_append<sequence, to_sequence<Sq>>::type;
  template<size_t I> requires(I < sizeof...(Vs))
  using remove = fore<I>::template append<back<sizeof...(Vs) - I - 1>>;
  template<size_t I, is_sequence Sq> requires(I <= sizeof...(Vs))
  using insert = typename fore<I>::template append<Sq>::template append<back<sizeof...(Vs) - I>>;
  template<template<auto...> typename Tm> using expand = Tm<Vs...>;
  template<size_t I> requires(I < sizeof...(Vs))
  constexpr const auto&& get() const noexcept {
    return mv(at<I>);
  }
};
} // namespace yw

//////////////////////////////////////// MARK: std

namespace std {

template<auto... Ts> struct tuple_size<yw::sequence<Ts...>>
  : integral_constant<size_t, sizeof...(Ts)> {};

template<size_t I, auto... Ts> struct tuple_element<I, yw::sequence<Ts...>>
  : type_identity<yw::select_type<I, decltype(Ts)...>> {};
} // namespace std
