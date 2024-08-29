/// \file vector/matrix.hpp

#pragma once

#include "vector.hpp"

export namespace yw {


/// structure for 4x4 matrix
struct matrix {

  /// first row
  vector x{};

  /// second row
  vector y{};

  /// third row
  vector z{};

  /// fourth row
  vector w{};

  /// default constructor
  constexpr matrix() noexcept = default;

  /// constructor with a value to fill
  explicit constexpr matrix(const fat4 Fill) noexcept
    : x(Fill), y(Fill), z{Fill}, w{Fill} {}

  /// constructor with a value to fill
  explicit constexpr matrix(numeric auto&& Fill) noexcept
    requires (!same_as<remove_cvref<decltype(Fill)>, fat4>)
    : matrix(fat4(Fill)) {}

};

} ////////////////////////////////////////////////////////////////////////////// namespace yw
