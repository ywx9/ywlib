/// \file get.hpp
/// \brief defines `function get` and ...

#pragma once

#ifndef YWLIB
#include <type_traits>
#else
import std;
#endif


#include "core.hpp"


namespace yw::_ {

namespace _get {

template<nat I> void get() = delete;

template<nat I, typename T> inline constexpr auto pattern = []() -> int {
  if constexpr (is_bounded_array<remove_ref<T>>)
    return I < std::extent_v<remove_ref<T>> ? 16 : 8;
  else if constexpr (requires { get<I>(declval<T>()); })
    return 1 | noexcept(get<I>(declval<T>())) * 16;
  else if constexpr (requires { declval<T>().template get<I>(); })
    return 2 | noexcept(declval<T>().template get<I>()) * 16;
  else return I == 0 ? 20 : 8;
}();

template<nat I, typename T, nat P = pattern<I, T>> requires (P != 8)
constexpr decltype(auto) get(T&& t) noexcept(bool(P & 16)) {
  if constexpr ((P & 7) == 0) return static_cast<T&&>(t)[I];
  else if constexpr ((P & 7) == 1) return get<I>(static_cast<T&&>(t));
  else if constexpr ((P & 7) == 2) return static_cast<T&&>(t).template get<I>();
  else if constexpr ((P & 7) == 4) return static_cast<T&&>(t);
}

}

// extent

template<typename T, nat I = (_get::pattern<0, T> & 15)>
struct extent : std::tuple_size<T> {};

template<typename T> struct extent<T, 0> : std::extent<T> {};

template<typename T> struct extent<T, 4> { static constexpr nat value = 1; };

} ////////////////////////////////////////////////////////////////////////////// namespace yw::_


export namespace yw {


/// number of elements in the tuple-like type
template<typename T> inline constexpr nat extent =
  _::extent<remove_ref<T>>::value;


/// checks if the extent of `T` and `Ts...` are equal
template<typename T, typename... Ts> concept same_extent =
  requires { requires ((extent<T> == extent<Ts>) && ...); };


/// checks if the type is tuple-like
template<typename T> concept tuple =
  requires { requires (_::_get::pattern<0, T> & 15) < 3; };


/// checks if `get<I>(T)` is well-formed
template<typename T, nat I> concept gettable =
  requires { _::_get::get<I>(declval<T>()); };


/// checks if `get<I>(T)` is well-formed and noexcept
template<typename T, nat I> concept nt_gettable = requires {
  requires gettable<T, I>;
  { _::_get::get<I>(declval<T>()) } noexcept;
};


/// result type of `get<I>(T)`
template<typename T, nat I> requires gettable<T, I>
using element_t = decltype(_::_get::get<I>(declval<T>()));


/// obtains the element at index `I` from `T`
template<nat I> inline constexpr auto get =
[]<typename T>(T&& t) noexcept(nt_gettable<T, I>)
  -> element_t<T, I> { return _::_get::get<I>(static_cast<T&&>(t)); };


} ////////////////////////////////////////////////////////////////////////////// namespace yw
