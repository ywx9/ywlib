/// \file core/range.hpp

#pragma once

#ifndef YWLIB
#include <iterator>
#include <ranges>
#else
import std;
#endif


#include "core.hpp"


namespace yw::_ {

/// iter type

template<typename T> struct _iter_t {};

template<std::ranges::range Rg>
struct _iter_t<Rg> : _iter_t<std::ranges::iterator_t<Rg>> {};

template<std::input_or_output_iterator It> struct _iter_t<It> {
  using v = std::iter_value_t<It>;
  using d = std::iter_difference_t<It>;
  using r = std::iter_reference_t<It>;
  using rr = std::iter_rvalue_reference_t<It>;
};

} ////////////////////////////////////////////////////////////////////////////// namespace yw::_


export namespace yw {


template<typename It> concept iterator = std::input_or_output_iterator<It>;

template<typename Se, typename It> concept sentinel_for = std::sentinel_for<Se, It>;

template<typename Se, typename It> concept sized_sentinel_for = std::sized_sentinel_for<Se, It>;

template<typename Rg> concept range = std::ranges::range<Rg>;

template<typename Rg> concept borrowed_range = std::ranges::borrowed_range<Rg>;

template<typename Rg> concept sized_range = std::ranges::sized_range<Rg>;

template<range Rg> using iterator_t = std::ranges::iterator_t<Rg>;

template<range Rg> using borrowed_iterator_t = std::ranges::borrowed_iterator_t<Rg>;

template<range Rg> using sentinel_t = std::ranges::sentinel_t<Rg>;


/// value type of iterator/range
template<typename T> requires iterator<remove_ref<T>> || range<remove_ref<T>>
using iter_value = typename _::_iter_t<remove_ref<T>>::v;

/// difference type of iterator/range
template<typename T> requires iterator<remove_ref<T>> || range<remove_ref<T>>
using iter_difference = typename _::_iter_t<remove_ref<T>>::d;

/// reference type of iterator/range
template<typename T> requires iterator<remove_ref<T>> || range<remove_ref<T>>
using iter_reference = typename _::_iter_t<remove_ref<T>>::r;

/// rvalue reference type of iterator/range
template<typename T> requires iterator<remove_ref<T>> || range<remove_ref<T>>
using iter_rvref = typename _::_iter_t<remove_ref<T>>::rr;

/// common type of `iter_reference` and `iter_value`
template<typename T> requires iterator<remove_ref<T>> || range<remove_ref<T>>
using iter_common = common_type<iter_reference<T>, iter_value<T>>;


template<typename It, typename T> concept output_iterator = std::output_iterator<It, T>;

template<typename It> concept input_iterator = std::input_iterator<It>;

template<typename It> concept fwd_iterator = std::forward_iterator<It>;

template<typename It> concept bid_iterator = std::bidirectional_iterator<It>;

template<typename It> concept rnd_iterator = std::random_access_iterator<It>;

template<typename It> concept cnt_iterator = std::contiguous_iterator<It>;

template<typename It, typename U> concept iterator_for = iterator<It> && convertible_to<iter_reference<It>, U>;

template<typename It, typename U> concept cnt_iterator_of = cnt_iterator<It> && same_as<remove_ref<iter_reference<It>>, U>;

template<typename Rg, typename T> concept output_range = std::ranges::output_range<Rg, T>;

template<typename Rg> concept input_range = std::ranges::input_range<Rg>;

template<typename Rg> concept fwd_range = std::ranges::forward_range<Rg>;

template<typename Rg> concept bid_range = std::ranges::bidirectional_range<Rg>;

template<typename Rg> concept rnd_range = std::ranges::random_access_range<Rg>;

template<typename Rg> concept cnt_range = std::ranges::contiguous_range<Rg>;

template<typename Rg, typename U> concept range_for = iterator_for<iterator_t<Rg>, U>;

template<typename Rg, typename U> concept cnt_range_of = cnt_range<Rg> && same_as<iter_value<iterator_t<Rg>>, U>;

template<typename Rg> concept cnt_sized_range = cnt_range<Rg> && sized_range<Rg>;

template<typename Rg, typename U> concept cnt_sized_range_of = cnt_sized_range<Rg> && cnt_range_of<Rg, U>;

template<typename It, typename In> concept iter_copyable = iterator<It> && iterator<In> && std::indirectly_copyable<In, It>;

template<typename It, typename In> concept iter_movable = iterator<It> && iterator<In> && std::indirectly_movable<In, It>;

template<typename Fn, typename It> concept iter_unary_invocable = iterator<It> && std::indirectly_unary_invocable<Fn, It>;

template<typename Fn, typename It> concept iter_unary_predicate = iterator<It> && std::indirect_unary_predicate<Fn, It>;


inline constexpr auto begin = []<range Rg>(Rg&& r)
  noexcept(noexcept(std::ranges::begin(fwd<Rg>(r))))
  -> decltype(std::ranges::begin(fwd<Rg>(r)))
  requires requires { std::ranges::begin(fwd<Rg>(r)); }
{ return std::ranges::begin(fwd<Rg>(r)); };

inline constexpr auto end = []<range Rg>(Rg&& r)
  noexcept(noexcept(std::ranges::end(fwd<Rg>(r))))
  -> decltype(std::ranges::end(fwd<Rg>(r)))
  requires requires { std::ranges::end(fwd<Rg>(r)); }
{ return std::ranges::end(fwd<Rg>(r)); };

inline constexpr auto rbegin = []<range Rg>(Rg&& r)
  noexcept(noexcept(std::ranges::rbegin(fwd<Rg>(r))))
  -> decltype(std::ranges::rbegin(fwd<Rg>(r)))
  requires requires { std::ranges::rbegin(fwd<Rg>(r)); }
{ return std::ranges::rbegin(fwd<Rg>(r)); };

inline constexpr auto rend = []<range Rg>(Rg&& r)
  noexcept(noexcept(std::ranges::rend(fwd<Rg>(r))))
  -> decltype(std::ranges::rend(fwd<Rg>(r)))
  requires requires { std::ranges::rend(fwd<Rg>(r)); }
{ return std::ranges::rend(fwd<Rg>(r)); };

inline constexpr auto size = []<range Rg>(Rg&& r)
  noexcept(noexcept(std::ranges::size(fwd<Rg>(r))))
  -> decltype(std::ranges::size(fwd<Rg>(r)))
  requires requires { std::ranges::size(fwd<Rg>(r)); }
{ return std::ranges::size(fwd<Rg>(r)); };

inline constexpr auto empty = []<range Rg>(Rg&& r)
  noexcept(noexcept(std::ranges::empty(fwd<Rg>(r))))
  -> decltype(std::ranges::empty(fwd<Rg>(r)))
  requires requires { std::ranges::empty(fwd<Rg>(r)); }
{ return std::ranges::empty(fwd<Rg>(r)); };

inline constexpr auto data = []<range Rg>(Rg&& r)
  noexcept(noexcept(std::ranges::data(fwd<Rg>(r))))
  -> decltype(std::ranges::data(fwd<Rg>(r)))
  requires requires { std::ranges::data(fwd<Rg>(r)); }
{ return std::ranges::data(fwd<Rg>(r)); };

inline constexpr auto iter_move = []<iterator It>(It&& i)
  noexcept(noexcept(std::ranges::iter_move(fwd<It>(i))))
  -> decltype(std::ranges::iter_move(fwd<It>(i)))
  requires requires { std::ranges::iter_move(fwd<It>(i)); }
{ return std::ranges::iter_move(fwd<It>(i)); };

inline constexpr auto iter_swap = []<iterator It, iterator Jt>(It&& i, Jt&& j)
  noexcept(noexcept(std::ranges::iter_swap(fwd<It>(i), fwd<Jt>(j))))
  requires requires { std::ranges::iter_swap(fwd<It>(i), fwd<Jt>(j)); }
{ return std::ranges::iter_swap(fwd<It>(i), fwd<Jt>(j)); };


} ////////////////////////////////////////////////////////////////////////////// namespace yw
