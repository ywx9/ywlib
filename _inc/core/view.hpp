/// \file core/view.hpp

#pragma once

#include "ranges.hpp"

export namespace yw {


/// struct to represent a view
template<yw::iterator It, sentinel_for<It> Se>
struct view {

  /// iterator type
  using iterator = It;

  /// sentinel type
  using sentinel = Se;

  /// begin iterator
  It first{};

  /// end sentinel
  Se last{};

  /// default constructor to initialize the view
  constexpr view() noexcept = default;

  /// constructor with a range
  constexpr view(It i, Se s) noexcept
    : first(mv(i)), last(mv(s)) {}

  /// constructor with a range
  constexpr view(It i, iter_difference<It> n) noexcept
    requires std::sized_sentinel_for<Se, It>
    : first(mv(i)), last(i + n) {}

  /// constructor with a range
  constexpr view(range auto&& r) noexcept
    : first(yw::begin(r)), last(yw::end(r)) {}

  /// checks if the view is empty
  constexpr bool empty() const noexcept { return first == last; }

  /// obtains the size of the view
  constexpr nat size() const noexcept { return nat(last - first); }

  /// obtains the begin iterator
  constexpr It begin() const noexcept { return first; }

  /// obtains the end sentinel
  constexpr Se end() const noexcept { return last; }

  /// access the element at the given index
  constexpr auto operator[](nat i) const noexcept
    requires rnd_iterator<It> { return first[i]; }

  /// removes the given prefix
  constexpr void remove_prefix(nat n) noexcept {
    if constexpr (rnd_iterator<It>) first += n;
    else while (n--) ++first;
  }

  /// removes the given suffix
  constexpr void remove_suffix(nat n) noexcept {
    if constexpr (rnd_iterator<Se>) last -= n;
    else while (n--) --last;
  }










};

}
