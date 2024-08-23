/// \file typepack.hpp
/// \brief defines `struct typepack`

#pragma once

#include "sequence.hpp"


export namespace yw {


template<typename... Ts> struct typepack;


} ////////////////////////////////////////////////////////////////////////////// namespace yw


namespace yw::_ {

// to_typepack

template<typename T, typename S> struct _to_typepack;

template<typename T, nat... Is> struct _to_typepack<T, sequence<Is...>> : _type<typepack<element_t<T, Is>...>> {};

// typepack::append

template<typename T, typename U> struct _typepack_append;

template<typename... Ts, typename... Us>
struct _typepack_append<typepack<Ts...>, typepack<Us...>>
  : _type<typepack<Ts..., Us...>> {};

// typepack::extract

template<typename T, typename S> struct _typepack_extract;

template<typename... Ts, nat... Is>
struct _typepack_extract<typepack<Ts...>, sequence<Is...>>
  : _type<typepack<select_type<Is, Ts...>...>> {};

} ////////////////////////////////////////////////////////////////////////////// namespace yw::_


export namespace yw {


/// converts the tuple-like type to `typepack`
template<typename T> using to_typepack =
  typename _::_to_typepack<T, make_indices_for<T>>::type;


/// struct to represent a pack of types
template<typename... Ts> struct typepack {

  /// number of types
  static constexpr nat count = sizeof...(Ts);

  /// common type of the types
  using common = common_type<Ts...>;

  template<nat I> requires(I < (sizeof...(Ts)))
  using at = select_type<I, Ts...>;

  template<tuple Tp> using append =
    typename _::_typepack_append<typepack, to_typepack<Tp>>::type;

  template<indices_for<typepack> Sq> using extract =
    typename _::_typepack_extract<typepack, to_sequence<Sq, nat>>::type;

  template<nat N> requires(N <= (sizeof...(Ts)))
  using fore = extract<make_sequence<0, N>>;

  template<nat N> requires(N <= (sizeof...(Ts)))
  using back = extract<make_sequence<(sizeof...(Ts)), (sizeof...(Ts)) - N>>;

  template<nat I, tuple T> requires(I < (sizeof...(Ts))) using insert =
    typename fore<I>::template append<T>::template append<back<(sizeof...(Ts)) - I>>;

  template<template<typename...> typename Tm> using expand = Tm<Ts...>;

  template<nat I> requires(I < (sizeof...(Ts)))
  constexpr at<I> get() const noexcept;
};


} ////////////////////////////////////////////////////////////////////////////// namespace yw


namespace std {

// tuple_size, tuple_element

template<typename... Ts> struct tuple_size<yw::typepack<Ts...>> : integral_constant<size_t, sizeof...(Ts)> {};
template<size_t I, typename... Ts> struct tuple_element<I, yw::typepack<Ts...>> { using type = typename yw::typepack<Ts...>::template at<I>; };

} ////////////////////////////////////////////////////////////////////////////// namespace std
