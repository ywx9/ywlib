/// \file sequence.hpp
/// \brief defines `struct sequence`

#pragma once

#ifndef YWLIB
#include <type_traits>
#else
import std;
#endif


#include "get.hpp"


export namespace yw {


using nat = decltype(sizeof(int));
template<auto... Vs> struct sequence;


} ////////////////////////////////////////////////////////////////////////////// namespace yw


namespace yw::_ {

// to_sequence

template<typename S, typename T> struct _to_sequence;

template<template<auto...> typename Tm, typename T, auto... Vs>
requires (std::convertible_to<decltype(Vs), T> && ...)
struct _to_sequence<Tm<Vs...>, T> { using type = sequence<T(Vs)...>; };

template<template<typename, auto...> typename Tm, typename T, typename U, auto... Vs>
requires (std::convertible_to<decltype(Vs), T> && ...)
struct _to_sequence<Tm<U, Vs...>, T> { using type = sequence<T(Vs)...>; };

template<template<auto...> typename Tm, auto... Vs>
struct _to_sequence<Tm<Vs...>, none> { using type = sequence<Vs...>; };

template<template<typename, auto...> typename Tm, typename U, auto... Vs>
struct _to_sequence<Tm<U, Vs...>, none> { using type = sequence<Vs...>; };

// indices_for

template<typename S, nat N> struct _indices_for : constant<false> {};

template<nat... Is, nat N> struct _indices_for<sequence<Is...>, N> { static constexpr bool value = ((Is < N) && ...); };

// make_sequence

template<nat I, nat N, auto F, nat...Vs> struct _make_sequence : _make_sequence<I + 1, N, F, Vs..., F(I)> {};

template<nat N, auto F, nat...Vs> struct _make_sequence<N, N, F, Vs...> { using type = sequence<Vs...>; };

// extracting_indices

template<nat I, nat N, typename S, nat... Is> struct _extracting_indices;

template<nat I, nat N, bool... Bs, nat... Is> struct _extracting_indices<I, N, sequence<Bs...>, Is...>
  : select_type<select_value<I, Bs...>, _extracting_indices<I + 1, N, sequence<Bs...>, Is..., I>,
                                        _extracting_indices<I + 1, N, sequence<Bs...>, Is...>> {};

template<nat N, bool... Bs, nat... Is> struct _extracting_indices<N, N, sequence<Bs...>, Is...> { using type = sequence<Is...>; };

// common_element

template<typename T, typename S> struct _common_element;

template<typename T, nat... Is> struct _common_element<T, sequence<Is...>> : std::common_reference<element_t<T, Is>...> {};

// tuple_for

template<typename T, typename U, typename S> struct _tuple_for : constant<false> {};

template<typename T, typename U, nat... Is> struct _tuple_for<T, U, sequence<Is...>>
  : constant<(std::convertible_to<element_t<T, Is>, U> && ...) +
             (std::is_nothrow_constructible_v<element_t<T, Is>, U> && ...) * 2> {};

} ////////////////////////////////////////////////////////////////////////////// namespace yw::_


export namespace yw {


/// converts a sequence-like type to `sequence`
template<typename S, typename T = none>
using to_sequence = typename _::_to_sequence<S, T>::type;


/// checks if the type is a sequence-like type whose elements are convertible to `T`
template<typename S, typename T = none>
concept sequence_of = requires { typename to_sequence<S, T>; };


/// checks if the type is a indices-like type whose elements are less than the extent of `T`
template<typename S, typename T> concept indices_for =
  requires { requires _::_indices_for<S, extent<T>>::value; };


/// generates a sequence; `{F(Begin), F(Begin + 1), ..., F(End - 1)}`
template<nat Begin, nat End, std::invocable<nat> auto F = pass{}>
requires (Begin < End && !std::is_void_v<decltype(F(Begin))>)
using make_sequence = typename _::_make_sequence<Begin, End, F>::type;


/// generates a indices for the tuple-like type `T`
template<typename T> using make_indices_for = make_sequence<0, extent<T>>;


/// generates a indices which contains the indices of `true` in the boolean sequence
template<sequence_of<bool> S> using extracting_indices =
  typename _::_extracting_indices<0, extent<S>, S>::type;


/// common type of the elements of the tuple-like type
template<typename T> using common_element =
  typename _::_common_element<T, make_indices_for<T>>::type;


/// checks if the all elements of the tuple-like type are convertible to `U`
template<typename T, typename U> concept tuple_for = requires {
  requires bool(_::_tuple_for<T, U, make_indices_for<T>>::value & 1);
};


/// checks if the all elements of the tuple-like type are convertible to `U` without throwing exceptions
template<typename T, typename U> concept nt_tuple_for = requires {
  requires tuple_for<T, U>;
  requires bool(_::_tuple_for<T, U, make_indices_for<T>>::value & 2);
};


} ////////////////////////////////////////////////////////////////////////////// namespace yw


namespace yw::_ {

// sequence

template<typename S, typename T> struct _sequence_append : _sequence_append<to_sequence<S>, to_sequence<T>> {};

template<auto... Vs, auto... Ws>
struct _sequence_append<sequence<Vs...>, sequence<Ws...>> { using type = sequence<Vs..., Ws...>; };

template<typename S, typename T> struct _sequence_extract : _sequence_extract<S, to_sequence<T, nat>> {};

template<auto... Vs, nat... Is> struct _sequence_extract<sequence<Vs...>, sequence<Is...>>
{ using type = sequence<select_value<Is, Vs...>...>; };

} ////////////////////////////////////////////////////////////////////////////// namespace yw::_


export namespace yw {


/// struct to represent a sequence of values
template<auto... Vs> struct sequence {

  /// number of values
  static constexpr nat count = sizeof...(Vs);

  /// value at the index
  template<nat I> static constexpr auto at = select_value<I, Vs...>;

  /// type of the value at the index
  template<nat I> using type_at = decltype(at<I>);

  /// appends the sequence to this
  template<sequence_of S> using append =
    typename _::_sequence_append<sequence, S>::type;

  /// extracts a subsequence
  template<indices_for<sequence> S> using extract =
    typename _::_sequence_extract<sequence, S>::type;

  /// extracts the first `N` values
  template<nat N> requires (N <= sizeof...(Vs))
  using fore = extract<make_sequence<0, N>>;

  /// extracts the last `N` values
  template<nat N> requires (N <= sizeof...(Vs))
  using back = extract<make_sequence<sizeof...(Vs) - N, sizeof...(Vs)>>;

  /// inserts the sequence to this
  template<nat I, sequence_of S> requires (I <= sizeof...(Vs))
  using insert = typename fore<I>::template append<S>::template append<back<sizeof...(Vs) - I>>;

  /// expands this into the template
  template<template<auto...> typename T> using expand = T<Vs...>;

  /// `get` function
  template<nat I> requires (I < sizeof...(Vs))
  constexpr const auto&& get() const noexcept { return static_cast<const type_at<I>&&>(at<I>); }

}; ///////////////////////////////////////////////////////////////////////////// struct sequence


} ////////////////////////////////////////////////////////////////////////////// namespace yw


namespace std {

template<auto... Vs> struct tuple_size<yw::sequence<Vs...>> : std::integral_constant<yw::nat, sizeof...(Vs)> {};

template<yw::nat I, auto... Vs> struct tuple_element<I, yw::sequence<Vs...>> { using type = yw::sequence<Vs...>::template type_at<I>; };

} ////////////////////////////////////////////////////////////////////////////// namespace std
