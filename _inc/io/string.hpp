/// \file io/string.hpp

#pragma once

#ifndef YWLIB
#include <string>
#include <string_view>
#else
import std;
#endif

#include "../core.hpp"

export namespace yw {


/// class to represent a string
template<character Ct> class string {

  /// pointer to the string
  Ct* _ptr{};

  /// size of the string
  nat _size{};

public:

  /// default constructor to initialize the string
  constexpr string() noexcept = default;

  /// constructor with a string view
  template<typename Ct2, typename Tr>
  constexpr string(const std::basic_string_view<Ct2, Tr> sv)
    : _ptr(new Ct[sv.size()]), _size(std::bit_ceil(sv.size() + 1))
  { std::ranges::copy(sv, _ptr); }


};

}
