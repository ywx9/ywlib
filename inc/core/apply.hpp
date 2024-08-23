/// \file apply.hpp

#pragma once

#include "sequence.hpp"

namespace yw::_ {

// apply

template<typename F, typename... Ts> requires (!(tuple<Ts> || ...))
constexpr decltype(auto) _apply(F&& f, Ts&&... ts)
  ywlib_wrapper(invoke(fwd<F>(f), fwd<Ts>(ts)...));

template<nat I, nat... Is, nat... Js, nat... Ks, typename F, typename... Ts>
constexpr decltype(auto) _apply_b(
  sequence<Is...>, sequence<Js...>, sequence<Ks...>, F&& f, Ts&&... ts)
  ywlib_wrapper(_apply(fwd<F>(f), select_parameter<Is>(fwd<Ts>(ts)...)...,
    get<Js>(fwd<select_type<I, Ts...>>(select_parameter<I>(fwd<Ts>(ts)...)))...,
    select_parameter<Ks>(fwd<Ts>(ts)...)...));

template<nat I, typename F, typename... Ts>
constexpr decltype(auto) _apply_a(F&& f, Ts&&... ts)
  ywlib_wrapper(_apply_b<I>(make_sequence<0, I>{},
    make_indices_for<select_type<I, Ts...>>{},
    make_sequence<I + 1, sizeof...(Ts)>{}, fwd<F>(f), fwd<Ts>(ts)...));

template<typename F, typename... Ts> requires (tuple<Ts> || ...)
constexpr decltype(auto) _apply(F&& f, Ts&&... ts)
  ywlib_wrapper(_apply_a<inspects<tuple<Ts>...>>(fwd<F>(f), fwd<Ts>(ts)...));

// vassign

template<typename Sq, typename Lt, typename Rt>
struct _vassignable : sequence<false, false> {};

template<nat... Is, typename Lt, typename Rt>
struct _vassignable<sequence<Is...>, Lt, Rt>
  : sequence<(assignable<element_t<Lt, Is>, element_t<Rt, Is>> && ...),
             (nt_assignable<element_t<Lt, Is>, element_t<Rt, Is>> && ...)> {};

} ////////////////////////////////////////////////////////////////////////////// namespace yw::_


export namespace yw {


/// checks if `apply(F, Ts...)` is well-formed
template<typename F, typename... Ts>
concept applyable = requires(F&& f, Ts&&... ts) {
  { _::_apply(fwd<F>(f), fwd<Ts>(ts)...) };
};


/// checks if `apply(F, Ts...)` is well-formed and noexcept
template<typename F, typename... Ts>
concept nt_applyable = requires(F&& f, Ts&&... ts) {
  applyable<F, Ts...>;
  { _::_apply(fwd<F>(f), fwd<Ts>(ts)...) } noexcept;
};


/// result type of `apply(F, Ts...)`
template<typename F, typename... Ts>
using apply_result = decltype(_::_apply(declval<F>(), declval<Ts>()...));


/// applies a function to arguments with tuples expanded
inline constexpr auto apply =
[]<typename F, typename... Ts>(F&& Func, Ts&&... Args)
  noexcept(nt_applyable<F, Ts...>) requires applyable<F, Ts...> {
  return _::_apply(fwd<F>(Func), fwd<Ts>(Args)...);
};


/// checks if `build<T>(Tp)` is well-formed
template<typename T, typename Tp>
concept buildable = applyable<decltype(construct<T>), Tp>;


/// checks if `build<T>(Tp)` is well-formed and noexcept
template<typename T, typename Tp>
concept nt_buildable = nt_applyable<decltype(construct<T>), Tp>;


/// constructs an object of type `T` from elements of `Tp`
template<typename T> inline constexpr auto build =
[]<typename Tp>(Tp&& Tuple)
  noexcept(nt_buildable<T, Tp>) requires buildable<T, Tp> {
  return apply(construct<T>, fwd<Tp>(Tuple));
};


/// checks if each element of `Rt` can be assigned to the corresponding element of `Lt`
template<typename Lt, typename Rt> concept vassignable = requires {
  requires same_extent<Lt, Rt>;
  requires _::_vassignable<make_indices_for<Lt>, Lt, Rt>::template at<0>;
};


/// checks if each element of `Rt` can be assigned to the corresponding element of `Lt` without throwing exceptions
template<typename Lt, typename Rt> concept nt_vassignable = requires {
  requires vassignable<Lt, Rt>;
  requires _::_vassignable<make_indices_for<Lt>, Lt, Rt>::template at<1>;
};


/// function object to assign each element of `Rt` to the corresponding element of `Lt`
/// \note `void vassign(Lt&& l, Rt&& r) noexcept(nt_vassignable<Lt, Rt>) requires vassignable<Lt, Rt>`
inline constexpr auto vassign = []<typename Lt, typename Rt>(Lt&& l, Rt&& r)
  noexcept(nt_vassignable<Lt, Rt>) requires vassignable<Lt, Rt> {
  [&]<nat... Is>(sequence<Is...>) {
    ((get<Is>(fwd<Lt>(l)) = get<Is>(fwd<Rt>(r))), ...);
  }(make_indices_for<Lt>{});
};


} ////////////////////////////////////////////////////////////////////////////// namespace yw
