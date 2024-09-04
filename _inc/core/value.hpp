/// \file core/value.hpp

#ifndef YWLIB
#include <limits>
#include <numbers>
#else
import std;
#endif


#include "core.hpp"
#include "none.hpp"


export namespace yw {


/// struct to represent a value
struct value {

  /// internal representation of the value
  fat8 _value{};

  /// default constructor
  constexpr value() noexcept = default;

  /// constructor from `none`
  constexpr value(none) noexcept : _value{std::numeric_limits<fat8>::quiet_NaN()} {}

  /// constructor from `numeric` value
  template<numeric T> requires (!same_as<T, value>)
  constexpr value(T&& v) noexcept : _value{fat8(fwd<T>(v))} {}

  /// conversion to `fat8&`
  constexpr operator fat8&() & noexcept { return _value; }

  /// conversion to `const fat8&`
  constexpr operator const fat8&() const& noexcept { return _value; }

  /// conversion to `fat8&&`
  constexpr operator fat8&&() && noexcept { return mv(_value); }

  /// conversion to `const fat8&&`
  constexpr operator const fat8&&() const&& noexcept { return mv(_value); }

  /// conversion to `arithmetic` value
  template<arithmetic T> explicit constexpr operator T() const
    noexcept { return static_cast<T>(_value); }

  /// addition assignment operator
  template<numeric T> constexpr value& operator+=(T&& v) noexcept {
    _value += fat8(fwd<T>(v));
    return *this;
  }

  /// subtraction assignment operator
  template<numeric T> constexpr value& operator-=(T&& v) noexcept {
    _value -= fat8(fwd<T>(v));
    return *this;
  }

  /// multiplication assignment operator
  template<numeric T> constexpr value& operator*=(T&& v) noexcept {
    _value *= fat8(fwd<T>(v));
    return *this;
  }

  /// division assignment operator
  template<numeric T> constexpr value& operator/=(T&& v) noexcept {
    _value /= fat8(fwd<T>(v));
    return *this;
  }
};


/// euler's number
inline constexpr value e = std::numbers::e_v<fat8>;

/// golden ratio
inline constexpr value phi = std::numbers::phi_v<fat8>;

/// pi
inline constexpr value pi = std::numbers::pi_v<fat8>;

/// gamma constant
inline constexpr value gamma = std::numbers::egamma_v<fat8>;

/// ln(2)
inline constexpr value ln2 = std::numbers::ln2_v<fat8>;

/// ln(10)
inline constexpr value ln10 = std::numbers::ln10_v<fat8>;

/// log2(10)
inline constexpr value log2_10 = 3.3219280948873623478703194294894;

/// log10(2)
inline constexpr value log10_2 = 0.30102999566398119521373889472449;

/// sqrt(2)
inline constexpr value sqrt2 = std::numbers::sqrt2_v<fat8>;

/// sqrt(3)
inline constexpr value sqrt3 = std::numbers::sqrt3_v<fat8>;

/// infinity
inline constexpr value inf = std::numeric_limits<fat8>::infinity();

/// quiet NaN
inline constexpr value nan = std::numeric_limits<fat8>::quiet_NaN();


} ////////////////////////////////////////////////////////////////////////////// yw
