#pragma once
#ifdef VSCODE
#include <type_traits>
#include <utility>
#include <compare>
#else
import <type_traits>;
import <utility>;
import <compare>;
#endif

namespace yw {

  using std::nullptr_t;
  using cat1 = char;
  using cat2 = wchar_t;
  using uct1 = char8_t;
  using uct2 = char16_t;
  using uct4 = char32_t;
  using int1 = signed char;
  using int2 = signed short;
  using int4 = signed int;
  using int8 = signed long long;
  using nat1 = unsigned char;
  using nat2 = unsigned short;
  using nat4 = unsigned int;
  using nat8 = unsigned long long;
  using fat4 = float;
  using fat8 = double;

  template<typename...> using void_t = void;
  template<typename T> using type_identity = std::type_identity_t<T>;
  template<bool B, typename T = void> using type_breaker = std::enable_if_t<B, T>;
  template<bool B, typename T1, typename T2> using type_switch = std::conditional_t<B, T1, T2>;

  using catt = cat2;
  using intt = type_switch<sizeof nullptr == 4, int4, int8>;
  using natt = type_switch<sizeof nullptr == 4, nat4, nat8>;
  using fatt = type_switch<sizeof nullptr == 4, fat4, fat8>;

  enum class byte : nat1 {};

  template<typename T, T Val> struct constant_agent {
    static constexpr T value = Val;
    constexpr operator T(void)const noexcept { return value; }
    [[nodiscard]] constexpr T operator()(void)const noexcept { return value; }
  };
  
  namespace _zyx {
    template<natt N> struct _xxx_type {};
    template<> struct _xxx_type<1> { using tc = cat1; using tu = uct1; using ti = int1; using tn = nat1; };
    template<> struct _xxx_type<2> { using tc = cat2; using tu = uct2; using ti = int2; using tn = nat2; };
    template<> struct _xxx_type<4> { using tu = uct4; using ti = int4; using tn = nat4; using tf = fat4; };
    template<> struct _xxx_type<8> { using ti = int8; using tn = nat8; using tf = fat8; };
  }
  template<natt N> requires requires { typename _zyx::_xxx_type<N>::tc; } using cat_type = _zyx::_xxx_type<N>::tc;
  template<natt N> requires requires { typename _zyx::_xxx_type<N>::tu; } using uct_type = _zyx::_xxx_type<N>::tu;
  template<natt N> requires requires { typename _zyx::_xxx_type<N>::ti; } using int_type = _zyx::_xxx_type<N>::ti;
  template<natt N> requires requires { typename _zyx::_xxx_type<N>::tn; } using nat_type = _zyx::_xxx_type<N>::tn;
  template<natt N> requires requires { typename _zyx::_xxx_type<N>::tf; } using fat_type = _zyx::_xxx_type<N>::tf;
  
  template<typename T, typename U> concept same_as = std::same_as<T, U>;
  template<typename T, typename...Us> concept included_in = (same_as<T, Us> || ...);
  template<typename T> concept is_nullptr = same_as<std::remove_cv_t<T>, nullptr_t>;
  template<typename T> concept is_void = same_as<std::remove_cv_t<T>, void>;
  template<typename T> concept is_bool = same_as<std::remove_cv_t<T>, bool>;
  template<typename T> concept is_byte = same_as<std::remove_cv_t<T>, byte>;
  template<typename T> concept is_cat = included_in<std::remove_cv_t<T>, cat1, cat2>;
  template<typename T> concept is_uct = included_in<std::remove_cv_t<T>, uct1, uct2, uct4>;
  template<typename T> concept is_int = included_in<std::remove_cv_t<T>, int1, int2, int4, int8>;
  template<typename T> concept is_nat = included_in<std::remove_cv_t<T>, nat1, nat2, nat4, nat8>;
  template<typename T> concept is_fat = included_in<std::remove_cv_t<T>, fat4, fat8>;
  template<typename T> concept character = is_cat<T> || is_uct<T>;
  template<typename T> concept countable = is_int<T> || is_nat<T>;
  template<typename T> concept quantable = is_fat<T> || countable<T>;
  
  constexpr byte operator"" ywb(nat8 val)noexcept { return static_cast<byte>(val & 0xFF); }
  constexpr byte operator<<(byte a, countable auto b)noexcept { return static_cast<byte>((static_cast<natt>(a) << b) & 0xFF); }
  constexpr byte operator>>(byte a, countable auto b)noexcept { return static_cast<byte>((static_cast<natt>(a) >> b) & 0xFF); }
  constexpr byte& operator<<=(byte& a, countable auto b)noexcept { return a = (a << b); }
  constexpr byte& operator>>=(byte& a, countable auto b)noexcept { return a = (a >> b); }
  constexpr byte operator&(byte a, byte b)noexcept { return static_cast<byte>(static_cast<nat1>(a) & static_cast<nat1>(b)); }
  constexpr byte operator|(byte a, byte b)noexcept { return static_cast<byte>(static_cast<nat1>(a) | static_cast<nat1>(b)); }
  constexpr byte operator^(byte a, byte b)noexcept { return static_cast<byte>(static_cast<nat1>(a) ^ static_cast<nat1>(b)); }
  constexpr byte& operator&=(byte& a, byte b)noexcept { return a = (a & b); }
  constexpr byte& operator|=(byte& a, byte b)noexcept { return a = (a | b); }
  constexpr byte& operator^=(byte& a, byte b)noexcept { return a = (a ^ b); }
  constexpr byte operator~(byte a)noexcept { return static_cast<byte>(~static_cast<nat1>(a)); }
  constexpr byte operator+(byte a, byte b)noexcept { return static_cast<byte>((static_cast<natt>(a) + static_cast<natt>(b)) & 0xFF); }
  constexpr byte operator-(byte a, byte b)noexcept { return static_cast<byte>((static_cast<natt>(a) - static_cast<natt>(b)) & 0xFF); }
  constexpr byte operator*(byte a, quantable auto b)noexcept { return static_cast<byte>(static_cast<natt>(static_cast<intt>(static_cast<natt>(a) * b)) & 0xFF); }
  constexpr byte operator/(byte a, quantable auto b)noexcept { return static_cast<byte>(static_cast<natt>(static_cast<intt>(static_cast<fat4>(static_cast<natt>(a)) / b)) & 0xFF); }
  constexpr byte operator+=(byte& a, byte b)noexcept { return a = (a + b); }
  constexpr byte operator-=(byte& a, byte b)noexcept { return a = (a - b); }
  constexpr byte operator*=(byte& a, quantable auto b)noexcept { return a = (a * b); }
  constexpr byte operator/=(byte& a, quantable auto b)noexcept { return a = (a / b); }

  using std::is_constant_evaluated;
  using std::declval;
  using std::addressof;
  using std::move;
  using std::forward;

}