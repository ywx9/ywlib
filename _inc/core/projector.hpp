/// \file core/projector.hpp

#pragma once

#include "sequence.hpp"

export namespace yw {


/// struct to project and arrange a tuple-like object
template<typename T, typename F = pass, sequence_of<nat> S = make_indices_for<T>>
struct projector {

  static_assert(0 < to_sequence<S, nat>::count);
  static_assert(!tuple<T> || indices_for<S, T>);

  /// indices type
  using indices = to_sequence<S, nat>;

  /// number of elements
  static constexpr nat count = indices::count;

  /// reference to the tuple-like object
  add_fwref<T> ref;

  /// projection function
  remove_ref<F> func{};

  /// constructor to specify the projection function and the arrangement
  template<typename U = T> constexpr projector(U&& Ref, F f, S)
    noexcept(nt_constructible<F, F>) : ref(fwd<U>(Ref)), func(mv(f)) {}

  /// constructor to specify the projection function
  template<typename U = T> constexpr projector(U&& Ref, F f)
    noexcept(nt_constructible<F, F>) : ref(fwd<U>(Ref)), func(mv(f)) {}

  /// constructor to specify the arrangement
  template<typename U = T> constexpr projector(U&& Ref, S)
    noexcept : ref(fwd<U>(Ref)) {}

  /// constructor
  template<typename U = T> constexpr projector(U&& Ref)
    noexcept : ref(fwd<U>(Ref)) {}

  /// `get` function
  template<nat I> requires (I < indices::count && tuple<T>)
  constexpr auto get() const
    ywlib_wrapper(invoke(func, yw::get<indices::template at<I>>(ref)));

  /// `get` function
  template<nat I> requires (I < indices::count && !tuple<T>)
  constexpr auto get() const ywlib_wrapper(invoke(func, ref));
};


template<typename T, typename F, sequence_of<nat> S>
projector(T&&, F, S) -> projector<T, F, S>;

template<typename T, typename F> requires(!sequence_of<F, nat>)
projector(T&&, F) -> projector<T, F>;

template<typename T, sequence_of<nat> S>
projector(T&&, S) -> projector<T, pass, S>;

template<typename T> projector(T&&) -> projector<T>;


} ////////////////////////////////////////////////////////////////////////////// namespace yw


namespace std {

template<typename T, typename F, yw::sequence_of<size_t> S>
struct tuple_size<yw::projector<T, F, S>>
  : integral_constant<size_t, yw::projector<T, F, S>::count> {};

template<size_t I, typename T, typename F, yw::sequence_of<size_t> S>
struct tuple_element<I, yw::projector<T, F, S>> {
  using type = decltype(declval<yw::projector<T, F, S>>().template get<I>());
};

} ////////////////////////////////////////////////////////////////////////////// namespace std
