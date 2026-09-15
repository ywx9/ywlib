#pragma once
#include <core/core.h>

namespace yw {

template<std::ranges::range R> using iterator_t = std::ranges::iterator_t<R>;
template<std::ranges::range R> using sentinel_t = std::ranges::sentinel_t<R>;

inline constexpr auto begin = []<std::ranges::range R>(R&& r) noexcept(noexcept(std::ranges::begin(r)))
                                requires requires { std::ranges::begin(r); } { return std::ranges::begin(r); };
inline constexpr auto end = []<std::ranges::range R>(R&& r) noexcept(noexcept(std::ranges::end(r)))
                              requires requires { std::ranges::end(r); } { return std::ranges::end(r); };
inline constexpr auto size = []<std::ranges::range R>(R&& r) noexcept(noexcept(std::ranges::size(r)))
                               requires requires { std::ranges::size(r); } { return std::ranges::size(r); };
inline constexpr auto data = []<std::ranges::range R>(R&& r) noexcept(noexcept(std::ranges::data(r)))
                               requires requires { std::ranges::data(r); } { return std::ranges::data(r); };

namespace internal {
template<typename T, template<typename> typename> struct iter_type : std::type_identity<void> {};
template<std::input_iterator I, template<typename> typename Tm> struct iter_type<I, Tm> : std::type_identity<Tm<I>> {};
template<std::ranges::input_range R, template<typename> typename Tm> struct iter_type<R, Tm>
  : std::type_identity<Tm<iterator_t<R>>> {};
} // namespace internal

template<typename T> using iter_value_t = internal::iter_type<remove_cvref<T>, std::iter_value_t>::type;
template<typename T> using iter_difference_t = internal::iter_type<remove_cvref<T>, std::iter_difference_t>::type;
template<typename T> using iter_reference_t = internal::iter_type<remove_cvref<T>, std::iter_reference_t>::type;

template<typename I, typename T = iter_value_t<I>> concept input_iterator =
  std::input_iterator<I> && same_as<T, iter_value_t<I>>;
template<typename I, typename T = iter_value_t<I>> concept forward_iterator =
  std::forward_iterator<I> && same_as<T, iter_value_t<I>>;
template<typename I, typename T = iter_value_t<I>> concept bidirectional_iterator =
  std::bidirectional_iterator<I> && same_as<T, iter_value_t<I>>;
template<typename I, typename T = iter_value_t<I>> concept random_access_iterator =
  std::random_access_iterator<I> && same_as<T, iter_value_t<I>>;

template<typename R, typename T = iter_value_t<R>> concept sized_range =
  std::ranges::sized_range<R> && same_as<T, iter_value_t<R>>;
template<typename R, typename T = iter_value_t<R>> concept input_range =
  std::ranges::input_range<R> && same_as<T, iter_value_t<R>>;
template<typename R, typename T = iter_value_t<R>> concept forward_range =
  std::ranges::forward_range<R> && same_as<T, iter_value_t<R>>;
template<typename R, typename T = iter_value_t<R>> concept bidirectional_range =
  std::ranges::bidirectional_range<R> && same_as<T, iter_value_t<R>>;
template<typename R, typename T = iter_value_t<R>> concept random_access_range =
  std::ranges::random_access_range<R> && same_as<T, iter_value_t<R>>;

template<typename I, typename T = iter_value_t<I>> concept contiguous_iterator =
  std::contiguous_iterator<I> && same_as<T, iter_value_t<I>>;
template<typename R, typename T = iter_value_t<R>> concept contiguous_range =
  std::ranges::contiguous_range<R> && sized_range<R> && same_as<T, iter_value_t<R>>;

template<typename I, typename T = iter_value_t<I>> concept output_iterator = std::output_iterator<I, T>;
template<typename R, typename T = iter_value_t<R>> concept output_range = std::ranges::output_range<R, T>;
/// mutable contiguous range.
template<typename R, typename T = iter_value_t<R>> concept contiguous_output_range =
  contiguous_range<R, T> && output_range<R, T>;

template<typename S, typename I> concept sentinel_for = std::sentinel_for<S, I>;
template<typename S, typename I> concept sized_sentinel_for = std::sized_sentinel_for<S, I>;
} // namespace yw
