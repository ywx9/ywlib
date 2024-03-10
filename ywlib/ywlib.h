/// @file ywlib/ywlib.h
#pragma once
#ifndef __ywlib__
#define __ywlib__
#define __ywlib_codepage__ 932


#include <Windows.h>
#include <array>
#include <compare>
#include <concepts>
#include <filesystem>
#include <iostream>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>


namespace yw {

/// concept for checking if the types are the same
template<typename T, typename... Ts> concept same_as = (std::same_as<T, Ts> && ...);

/// concept for checking if the type is included in the list
template<typename T, typename... Ts> concept included_in = (std::same_as<T, Ts> || ...);


/// integer type
struct it {
  long long cpp{0};

  /// default constructor
  constexpr it() noexcept = default;

  /// constructors
  constexpr it(const signed char Integer) noexcept : cpp(Integer) {}
  constexpr it(const short Integer) noexcept : cpp(Integer) {}
  constexpr it(const int Integer) noexcept : cpp(Integer) {}
  constexpr it(const long Integer) noexcept : cpp(Integer) {}
  constexpr it(const long long Integer) noexcept : cpp(Integer) {}
  constexpr it(const unsigned char Integer) noexcept : cpp(Integer) {}
  constexpr it(const unsigned short Integer) noexcept : cpp(Integer) {}
  constexpr it(const unsigned int Integer) noexcept : cpp(Integer) {}
  constexpr it(const unsigned long Integer) noexcept : cpp(Integer) {}
  constexpr it(const unsigned long long Integer) noexcept : cpp(__builtin_bit_cast(long long, Integer)) {}
  explicit constexpr it(const float Value) noexcept : cpp(static_cast<long long>(Value)) {}
  explicit constexpr it(const double Value) noexcept : cpp(static_cast<long long>(Value)) {}
  explicit constexpr it(const long double Value) noexcept : cpp(static_cast<long long>(Value)) {}

  /// conversion operators
  explicit constexpr operator signed char() const noexcept { return static_cast<char>(cpp); }
  explicit constexpr operator short() const noexcept { return static_cast<short>(cpp); }
  explicit constexpr operator int() const noexcept { return static_cast<int>(cpp); }
  explicit constexpr operator long() const noexcept { return static_cast<long>(cpp); }
  constexpr operator long long() const noexcept { return cpp; }
  explicit constexpr operator unsigned char() const noexcept { return static_cast<unsigned char>(cpp); }
  explicit constexpr operator unsigned short() const noexcept { return static_cast<unsigned short>(cpp); }
  explicit constexpr operator unsigned int() const noexcept { return static_cast<unsigned int>(cpp); }
  explicit constexpr operator unsigned long() const noexcept { return static_cast<unsigned long>(cpp); }
  explicit constexpr operator unsigned long long() const noexcept { return __builtin_bit_cast(unsigned long long, cpp); }
  explicit constexpr operator float() const noexcept { return static_cast<float>(cpp); }
  explicit constexpr operator double() const noexcept { return static_cast<double>(cpp); }
  explicit constexpr operator long double() const noexcept { return static_cast<long double>(cpp); }

  /// comparison operators
  friend constexpr bool operator==(const it Left, const it Right) noexcept { return Left.cpp == Right.cpp; }
  friend constexpr auto operator<=>(const it Left, const it Right) noexcept { return Left.cpp <=> Right.cpp; }
  friend constexpr bool operator==(const it Left, const signed char Right) noexcept { return Left.cpp == Right; }
  friend constexpr bool operator==(const it Left, const short Right) noexcept { return Left.cpp == Right; }
  friend constexpr bool operator==(const it Left, const int Right) noexcept { return Left.cpp == Right; }
  friend constexpr bool operator==(const it Left, const long Right) noexcept { return Left.cpp == Right; }
  friend constexpr bool operator==(const it Left, const long long Right) noexcept { return Left.cpp == Right; }
  friend constexpr bool operator==(const it Left, const unsigned char Right) noexcept { return Left.cpp == Right; }
  friend constexpr bool operator==(const it Left, const unsigned short Right) noexcept { return Left.cpp == Right; }
  friend constexpr bool operator==(const it Left, const unsigned int Right) noexcept { return Left.cpp == Right; }
  friend constexpr bool operator==(const it Left, const unsigned long Right) noexcept { return Left.cpp == Right; }
  friend constexpr bool operator==(const it Left, const unsigned long long Right) noexcept { return Left.cpp == Right; }
  friend constexpr auto operator<=>(const it Left, const signed char Right) noexcept { return Left.cpp <=> Right; }
  friend constexpr auto operator<=>(const it Left, const short Right) noexcept { return Left.cpp <=> Right; }
  friend constexpr auto operator<=>(const it Left, const int Right) noexcept { return Left.cpp <=> Right; }
  friend constexpr auto operator<=>(const it Left, const long Right) noexcept { return Left.cpp <=> Right; }
  friend constexpr auto operator<=>(const it Left, const long long Right) noexcept { return Left.cpp <=> Right; }
  friend constexpr auto operator<=>(const it Left, const unsigned char Right) noexcept { return Left.cpp <=> Right; }
  friend constexpr auto operator<=>(const it Left, const unsigned short Right) noexcept { return Left.cpp <=> Right; }
  friend constexpr auto operator<=>(const it Left, const unsigned int Right) noexcept { return Left.cpp <=> Right; }
  friend constexpr auto operator<=>(const it Left, const unsigned long Right) noexcept { return Left.cpp <=> Right; }
  friend constexpr auto operator<=>(const it Left, const unsigned long long Right) noexcept { return Left.cpp < 0 ? std::strong_ordering::less : static_cast<unsigned long long>(Left.cpp) <=> Right; }
  friend constexpr bool operator==(const signed char Left, const it Right) noexcept { return Left == Right.cpp; }
  friend constexpr bool operator==(const short Left, const it Right) noexcept { return Left == Right.cpp; }
  friend constexpr bool operator==(const int Left, const it Right) noexcept { return Left == Right.cpp; }
  friend constexpr bool operator==(const long Left, const it Right) noexcept { return Left == Right.cpp; }
  friend constexpr bool operator==(const long long Left, const it Right) noexcept { return Left == Right.cpp; }
  friend constexpr bool operator==(const unsigned char Left, const it Right) noexcept { return Left == Right.cpp; }
  friend constexpr bool operator==(const unsigned short Left, const it Right) noexcept { return Left == Right.cpp; }
  friend constexpr bool operator==(const unsigned int Left, const it Right) noexcept { return Left == Right.cpp; }
  friend constexpr bool operator==(const unsigned long Left, const it Right) noexcept { return Left == Right.cpp; }
  friend constexpr bool operator==(const unsigned long long Left, const it Right) noexcept { return Left == Right.cpp; }
  friend constexpr auto operator<=>(const signed char Left, const it Right) noexcept { return Left <=> Right.cpp; }
  friend constexpr auto operator<=>(const short Left, const it Right) noexcept { return Left <=> Right.cpp; }
  friend constexpr auto operator<=>(const int Left, const it Right) noexcept { return Left <=> Right.cpp; }
  friend constexpr auto operator<=>(const long Left, const it Right) noexcept { return Left <=> Right.cpp; }
  friend constexpr auto operator<=>(const long long Left, const it Right) noexcept { return Left <=> Right.cpp; }
  friend constexpr auto operator<=>(const unsigned char Left, const it Right) noexcept { return Left <=> Right.cpp; }
  friend constexpr auto operator<=>(const unsigned short Left, const it Right) noexcept { return Left <=> Right.cpp; }
  friend constexpr auto operator<=>(const unsigned int Left, const it Right) noexcept { return Left <=> Right.cpp; }
  friend constexpr auto operator<=>(const unsigned long Left, const it Right) noexcept { return Left <=> Right.cpp; }
  friend constexpr auto operator<=>(const unsigned long long Left, const it Right) noexcept { return Right.cpp < 0 ? std::strong_ordering::greater : Left <=> static_cast<unsigned long long>(Right.cpp); }

  /// arithmetic operators
  friend constexpr it operator-(const it Right) noexcept { return it(-Right.cpp); }
  friend constexpr it operator++(it& Right) noexcept { return ++Right.cpp, Right; }
  friend constexpr it operator--(it& Right) noexcept { return --Right.cpp, Right; }
  friend constexpr it operator++(it& Left, int) noexcept { return it(Left.cpp++); }
  friend constexpr it operator--(it& Left, int) noexcept { return it(Left.cpp--); }
  friend constexpr it operator+(const it Left, const it Right) noexcept { return it(Left.cpp + Right.cpp); }
  friend constexpr it operator-(const it Left, const it Right) noexcept { return it(Left.cpp - Right.cpp); }
  friend constexpr it operator*(const it Left, const it Right) noexcept { return it(Left.cpp * Right.cpp); }
  friend constexpr it operator/(const it Left, const it Right) noexcept { return it(Left.cpp / Right.cpp); }
  friend constexpr it operator+(const it Left, const signed char Right) noexcept { return it(Left.cpp + Right); }
  friend constexpr it operator+(const it Left, const short Right) noexcept { return it(Left.cpp + Right); }
  friend constexpr it operator+(const it Left, const int Right) noexcept { return it(Left.cpp + Right); }
  friend constexpr it operator+(const it Left, const long Right) noexcept { return it(Left.cpp + Right); }
  friend constexpr it operator+(const it Left, const long long Right) noexcept { return it(Left.cpp + Right); }
  friend constexpr it operator+(const it Left, const unsigned char Right) noexcept { return it(Left.cpp + Right); }
  friend constexpr it operator+(const it Left, const unsigned short Right) noexcept { return it(Left.cpp + Right); }
  friend constexpr it operator+(const it Left, const unsigned int Right) noexcept { return it(Left.cpp + Right); }
  friend constexpr it operator+(const it Left, const unsigned long Right) noexcept { return it(Left.cpp + Right); }
  friend constexpr it operator+(const it Left, const unsigned long long Right) noexcept { return it(Left.cpp + Right); }
  friend constexpr it operator-(const it Left, const signed char Right) noexcept { return it(Left.cpp - Right); }
  friend constexpr it operator-(const it Left, const short Right) noexcept { return it(Left.cpp - Right); }
  friend constexpr it operator-(const it Left, const int Right) noexcept { return it(Left.cpp - Right); }
  friend constexpr it operator-(const it Left, const long Right) noexcept { return it(Left.cpp - Right); }
  friend constexpr it operator-(const it Left, const long long Right) noexcept { return it(Left.cpp - Right); }
  friend constexpr it operator-(const it Left, const unsigned char Right) noexcept { return it(Left.cpp - Right); }
  friend constexpr it operator-(const it Left, const unsigned short Right) noexcept { return it(Left.cpp - Right); }
  friend constexpr it operator-(const it Left, const unsigned int Right) noexcept { return it(Left.cpp - Right); }
  friend constexpr it operator-(const it Left, const unsigned long Right) noexcept { return it(Left.cpp - Right); }
  friend constexpr it operator-(const it Left, const unsigned long long Right) noexcept { return it(Left.cpp - Right); }
  friend constexpr it operator*(const it Left, const signed char Right) noexcept { return it(Left.cpp * Right); }
  friend constexpr it operator*(const it Left, const short Right) noexcept { return it(Left.cpp * Right); }
  friend constexpr it operator*(const it Left, const int Right) noexcept { return it(Left.cpp * Right); }
  friend constexpr it operator*(const it Left, const long Right) noexcept { return it(Left.cpp * Right); }
  friend constexpr it operator*(const it Left, const long long Right) noexcept { return it(Left.cpp * Right); }
  friend constexpr it operator*(const it Left, const unsigned char Right) noexcept { return it(Left.cpp * Right); }
  friend constexpr it operator*(const it Left, const unsigned short Right) noexcept { return it(Left.cpp * Right); }
  friend constexpr it operator*(const it Left, const unsigned int Right) noexcept { return it(Left.cpp * Right); }
  friend constexpr it operator*(const it Left, const unsigned long Right) noexcept { return it(Left.cpp * Right); }
  friend constexpr it operator*(const it Left, const unsigned long long Right) noexcept { return it(Left.cpp * Right); }
  friend constexpr it operator/(const it Left, const signed char Right) noexcept { return it(Left.cpp / Right); }
  friend constexpr it operator/(const it Left, const short Right) noexcept { return it(Left.cpp / Right); }
  friend constexpr it operator/(const it Left, const int Right) noexcept { return it(Left.cpp / Right); }
  friend constexpr it operator/(const it Left, const long Right) noexcept { return it(Left.cpp / Right); }
  friend constexpr it operator/(const it Left, const long long Right) noexcept { return it(Left.cpp / Right); }
  friend constexpr it operator/(const it Left, const unsigned char Right) noexcept { return it(Left.cpp / Right); }
  friend constexpr it operator/(const it Left, const unsigned short Right) noexcept { return it(Left.cpp / Right); }
  friend constexpr it operator/(const it Left, const unsigned int Right) noexcept { return it(Left.cpp / Right); }
  friend constexpr it operator/(const it Left, const unsigned long Right) noexcept { return it(Left.cpp / Right); }
  friend constexpr it operator/(const it Left, const unsigned long long Right) noexcept { return it(Left.cpp / Right); }
  friend constexpr it operator+(const signed char Left, const it Right) noexcept { return it(Left + Right.cpp); }
  friend constexpr it operator+(const short Left, const it Right) noexcept { return it(Left + Right.cpp); }
  friend constexpr it operator+(const int Left, const it Right) noexcept { return it(Left + Right.cpp); }
  friend constexpr it operator+(const long Left, const it Right) noexcept { return it(Left + Right.cpp); }
  friend constexpr it operator+(const long long Left, const it Right) noexcept { return it(Left + Right.cpp); }
  friend constexpr it operator+(const unsigned char Left, const it Right) noexcept { return it(Left + Right.cpp); }
  friend constexpr it operator+(const unsigned short Left, const it Right) noexcept { return it(Left + Right.cpp); }
  friend constexpr it operator+(const unsigned int Left, const it Right) noexcept { return it(Left + Right.cpp); }
  friend constexpr it operator+(const unsigned long Left, const it Right) noexcept { return it(Left + Right.cpp); }
  friend constexpr it operator+(const unsigned long long Left, const it Right) noexcept { return it(Left + Right.cpp); }
  friend constexpr it operator-(const signed char Left, const it Right) noexcept { return it(Left - Right.cpp); }
  friend constexpr it operator-(const short Left, const it Right) noexcept { return it(Left - Right.cpp); }
  friend constexpr it operator-(const int Left, const it Right) noexcept { return it(Left - Right.cpp); }
  friend constexpr it operator-(const long Left, const it Right) noexcept { return it(Left - Right.cpp); }
  friend constexpr it operator-(const long long Left, const it Right) noexcept { return it(Left - Right.cpp); }
  friend constexpr it operator-(const unsigned char Left, const it Right) noexcept { return it(Left - Right.cpp); }
  friend constexpr it operator-(const unsigned short Left, const it Right) noexcept { return it(Left - Right.cpp); }
  friend constexpr it operator-(const unsigned int Left, const it Right) noexcept { return it(Left - Right.cpp); }
  friend constexpr it operator-(const unsigned long Left, const it Right) noexcept { return it(Left - Right.cpp); }
  friend constexpr it operator-(const unsigned long long Left, const it Right) noexcept { return it(Left - Right.cpp); }
  friend constexpr it operator*(const signed char Left, const it Right) noexcept { return it(Left * Right.cpp); }
  friend constexpr it operator*(const short Left, const it Right) noexcept { return it(Left * Right.cpp); }
  friend constexpr it operator*(const int Left, const it Right) noexcept { return it(Left * Right.cpp); }
  friend constexpr it operator*(const long Left, const it Right) noexcept { return it(Left * Right.cpp); }
  friend constexpr it operator*(const long long Left, const it Right) noexcept { return it(Left * Right.cpp); }
  friend constexpr it operator*(const unsigned char Left, const it Right) noexcept { return it(Left * Right.cpp); }
  friend constexpr it operator*(const unsigned short Left, const it Right) noexcept { return it(Left * Right.cpp); }
  friend constexpr it operator*(const unsigned int Left, const it Right) noexcept { return it(Left * Right.cpp); }
  friend constexpr it operator*(const unsigned long Left, const it Right) noexcept { return it(Left * Right.cpp); }
  friend constexpr it operator*(const unsigned long long Left, const it Right) noexcept { return it(Left * Right.cpp); }
  friend constexpr it operator/(const signed char Left, const it Right) noexcept { return it(Left / Right.cpp); }
  friend constexpr it operator/(const short Left, const it Right) noexcept { return it(Left / Right.cpp); }
  friend constexpr it operator/(const int Left, const it Right) noexcept { return it(Left / Right.cpp); }
  friend constexpr it operator/(const long Left, const it Right) noexcept { return it(Left / Right.cpp); }
  friend constexpr it operator/(const long long Left, const it Right) noexcept { return it(Left / Right.cpp); }
  friend constexpr it operator/(const unsigned char Left, const it Right) noexcept { return it(Left / Right.cpp); }
  friend constexpr it operator/(const unsigned short Left, const it Right) noexcept { return it(Left / Right.cpp); }
  friend constexpr it operator/(const unsigned int Left, const it Right) noexcept { return it(Left / Right.cpp); }
  friend constexpr it operator/(const unsigned long Left, const it Right) noexcept { return it(Left / Right.cpp); }
  friend constexpr it operator/(const unsigned long long Left, const it Right) noexcept { return it(Left / Right.cpp); }

  /// bitwise operators
  friend constexpr it operator~(const it Right) noexcept { return it(~Right.cpp); }
  friend constexpr it operator&(const it Left, const it Right) noexcept { return it(Left.cpp & Right.cpp); }
  friend constexpr it operator|(const it Left, const it Right) noexcept { return it(Left.cpp | Right.cpp); }
  friend constexpr it operator^(const it Left, const it Right) noexcept { return it(Left.cpp ^ Right.cpp); }
  friend constexpr it operator<<(const it Left, const it Right) noexcept { return it(Left.cpp << Right.cpp); }
  friend constexpr it operator>>(const it Left, const it Right) noexcept { return it(Left.cpp >> Right.cpp); }
  friend constexpr it operator&(const it Left, const signed char Right) noexcept { return it(Left.cpp & Right); }
  friend constexpr it operator&(const it Left, const short Right) noexcept { return it(Left.cpp & Right); }
  friend constexpr it operator&(const it Left, const int Right) noexcept { return it(Left.cpp & Right); }
  friend constexpr it operator&(const it Left, const long Right) noexcept { return it(Left.cpp & Right); }
  friend constexpr it operator&(const it Left, const long long Right) noexcept { return it(Left.cpp & Right); }
  friend constexpr it operator&(const it Left, const unsigned char Right) noexcept { return it(Left.cpp & Right); }
  friend constexpr it operator&(const it Left, const unsigned short Right) noexcept { return it(Left.cpp & Right); }
  friend constexpr it operator&(const it Left, const unsigned int Right) noexcept { return it(Left.cpp & Right); }
  friend constexpr it operator&(const it Left, const unsigned long Right) noexcept { return it(Left.cpp & Right); }
  friend constexpr it operator&(const it Left, const unsigned long long Right) noexcept { return it(Left.cpp & Right); }
  friend constexpr it operator|(const it Left, const signed char Right) noexcept { return it(Left.cpp | Right); }
  friend constexpr it operator|(const it Left, const short Right) noexcept { return it(Left.cpp | Right); }
  friend constexpr it operator|(const it Left, const int Right) noexcept { return it(Left.cpp | Right); }
  friend constexpr it operator|(const it Left, const long Right) noexcept { return it(Left.cpp | Right); }
  friend constexpr it operator|(const it Left, const long long Right) noexcept { return it(Left.cpp | Right); }
  friend constexpr it operator|(const it Left, const unsigned char Right) noexcept { return it(Left.cpp | Right); }
  friend constexpr it operator|(const it Left, const unsigned short Right) noexcept { return it(Left.cpp | Right); }
  friend constexpr it operator|(const it Left, const unsigned int Right) noexcept { return it(Left.cpp | Right); }
  friend constexpr it operator|(const it Left, const unsigned long Right) noexcept { return it(Left.cpp | Right); }
  friend constexpr it operator|(const it Left, const unsigned long long Right) noexcept { return it(Left.cpp | Right); }
  friend constexpr it operator^(const it Left, const signed char Right) noexcept { return it(Left.cpp ^ Right); }
  friend constexpr it operator^(const it Left, const short Right) noexcept { return it(Left.cpp ^ Right); }
  friend constexpr it operator^(const it Left, const int Right) noexcept { return it(Left.cpp ^ Right); }
  friend constexpr it operator^(const it Left, const long Right) noexcept { return it(Left.cpp ^ Right); }
  friend constexpr it operator^(const it Left, const long long Right) noexcept { return it(Left.cpp ^ Right); }
  friend constexpr it operator^(const it Left, const unsigned char Right) noexcept { return it(Left.cpp ^ Right); }
  friend constexpr it operator^(const it Left, const unsigned short Right) noexcept { return it(Left.cpp ^ Right); }
  friend constexpr it operator^(const it Left, const unsigned int Right) noexcept { return it(Left.cpp ^ Right); }
  friend constexpr it operator^(const it Left, const unsigned long Right) noexcept { return it(Left.cpp ^ Right); }
  friend constexpr it operator^(const it Left, const unsigned long long Right) noexcept { return it(Left.cpp ^ Right); }
  friend constexpr it operator<<(const it Left, const signed char Right) noexcept { return it(Left.cpp << Right); }
  friend constexpr it operator<<(const it Left, const short Right) noexcept { return it(Left.cpp << Right); }
  friend constexpr it operator<<(const it Left, const int Right) noexcept { return it(Left.cpp << Right); }
  friend constexpr it operator<<(const it Left, const long Right) noexcept { return it(Left.cpp << Right); }
  friend constexpr it operator<<(const it Left, const long long Right) noexcept { return it(Left.cpp << Right); }
  friend constexpr it operator<<(const it Left, const unsigned char Right) noexcept { return it(Left.cpp << Right); }
  friend constexpr it operator<<(const it Left, const unsigned short Right) noexcept { return it(Left.cpp << Right); }
  friend constexpr it operator<<(const it Left, const unsigned int Right) noexcept { return it(Left.cpp << Right); }
  friend constexpr it operator<<(const it Left, const unsigned long Right) noexcept { return it(Left.cpp << Right); }
  friend constexpr it operator<<(const it Left, const unsigned long long Right) noexcept { return it(Left.cpp << Right); }
  friend constexpr it operator>>(const it Left, const signed char Right) noexcept { return it(Left.cpp >> Right); }
  friend constexpr it operator>>(const it Left, const short Right) noexcept { return it(Left.cpp >> Right); }
  friend constexpr it operator>>(const it Left, const int Right) noexcept { return it(Left.cpp >> Right); }
  friend constexpr it operator>>(const it Left, const long Right) noexcept { return it(Left.cpp >> Right); }
  friend constexpr it operator>>(const it Left, const long long Right) noexcept { return it(Left.cpp >> Right); }
  friend constexpr it operator>>(const it Left, const unsigned char Right) noexcept { return it(Left.cpp >> Right); }
  friend constexpr it operator>>(const it Left, const unsigned short Right) noexcept { return it(Left.cpp >> Right); }
  friend constexpr it operator>>(const it Left, const unsigned int Right) noexcept { return it(Left.cpp >> Right); }
  friend constexpr it operator>>(const it Left, const unsigned long Right) noexcept { return it(Left.cpp >> Right); }
  friend constexpr it operator>>(const it Left, const unsigned long long Right) noexcept { return it(Left.cpp >> Right); }
  friend constexpr it operator&(const signed char Left, const it Right) noexcept { return it(Left & Right.cpp); }
  friend constexpr it operator&(const short Left, const it Right) noexcept { return it(Left & Right.cpp); }
  friend constexpr it operator&(const int Left, const it Right) noexcept { return it(Left & Right.cpp); }
  friend constexpr it operator&(const long Left, const it Right) noexcept { return it(Left & Right.cpp); }
  friend constexpr it operator&(const long long Left, const it Right) noexcept { return it(Left & Right.cpp); }
  friend constexpr it operator&(const unsigned char Left, const it Right) noexcept { return it(Left & Right.cpp); }
  friend constexpr it operator&(const unsigned short Left, const it Right) noexcept { return it(Left & Right.cpp); }
  friend constexpr it operator&(const unsigned int Left, const it Right) noexcept { return it(Left & Right.cpp); }
  friend constexpr it operator&(const unsigned long Left, const it Right) noexcept { return it(Left & Right.cpp); }
  friend constexpr it operator&(const unsigned long long Left, const it Right) noexcept { return it(Left & Right.cpp); }
  friend constexpr it operator|(const signed char Left, const it Right) noexcept { return it(Left | Right.cpp); }
  friend constexpr it operator|(const short Left, const it Right) noexcept { return it(Left | Right.cpp); }
  friend constexpr it operator|(const int Left, const it Right) noexcept { return it(Left | Right.cpp); }
  friend constexpr it operator|(const long Left, const it Right) noexcept { return it(Left | Right.cpp); }
  friend constexpr it operator|(const long long Left, const it Right) noexcept { return it(Left | Right.cpp); }
  friend constexpr it operator|(const unsigned char Left, const it Right) noexcept { return it(Left | Right.cpp); }
  friend constexpr it operator|(const unsigned short Left, const it Right) noexcept { return it(Left | Right.cpp); }
  friend constexpr it operator|(const unsigned int Left, const it Right) noexcept { return it(Left | Right.cpp); }
  friend constexpr it operator|(const unsigned long Left, const it Right) noexcept { return it(Left | Right.cpp); }
  friend constexpr it operator|(const unsigned long long Left, const it Right) noexcept { return it(Left | Right.cpp); }
  friend constexpr it operator^(const signed char Left, const it Right) noexcept { return it(Left ^ Right.cpp); }
  friend constexpr it operator^(const short Left, const it Right) noexcept { return it(Left ^ Right.cpp); }
  friend constexpr it operator^(const int Left, const it Right) noexcept { return it(Left ^ Right.cpp); }
  friend constexpr it operator^(const long Left, const it Right) noexcept { return it(Left ^ Right.cpp); }
  friend constexpr it operator^(const long long Left, const it Right) noexcept { return it(Left ^ Right.cpp); }
  friend constexpr it operator^(const unsigned char Left, const it Right) noexcept { return it(Left ^ Right.cpp); }
  friend constexpr it operator^(const unsigned short Left, const it Right) noexcept { return it(Left ^ Right.cpp); }
  friend constexpr it operator^(const unsigned int Left, const it Right) noexcept { return it(Left ^ Right.cpp); }
  friend constexpr it operator^(const unsigned long Left, const it Right) noexcept { return it(Left ^ Right.cpp); }
  friend constexpr it operator^(const unsigned long long Left, const it Right) noexcept { return it(Left ^ Right.cpp); }
  friend constexpr it operator<<(const signed char Left, const it Right) noexcept { return it(Left << Right.cpp); }
  friend constexpr it operator<<(const short Left, const it Right) noexcept { return it(Left << Right.cpp); }
  friend constexpr it operator<<(const int Left, const it Right) noexcept { return it(Left << Right.cpp); }
  friend constexpr it operator<<(const long Left, const it Right) noexcept { return it(Left << Right.cpp); }
  friend constexpr it operator<<(const long long Left, const it Right) noexcept { return it(Left << Right.cpp); }
  friend constexpr it operator<<(const unsigned char Left, const it Right) noexcept { return it(Left << Right.cpp); }
  friend constexpr it operator<<(const unsigned short Left, const it Right) noexcept { return it(Left << Right.cpp); }
  friend constexpr it operator<<(const unsigned int Left, const it Right) noexcept { return it(Left << Right.cpp); }
  friend constexpr it operator<<(const unsigned long Left, const it Right) noexcept { return it(Left << Right.cpp); }
  friend constexpr it operator<<(const unsigned long long Left, const it Right) noexcept { return it(Left << Right.cpp); }
  friend constexpr it operator>>(const signed char Left, const it Right) noexcept { return it(Left >> Right.cpp); }
  friend constexpr it operator>>(const short Left, const it Right) noexcept { return it(Left >> Right.cpp); }
  friend constexpr it operator>>(const int Left, const it Right) noexcept { return it(Left >> Right.cpp); }
  friend constexpr it operator>>(const long Left, const it Right) noexcept { return it(Left >> Right.cpp); }
  friend constexpr it operator>>(const long long Left, const it Right) noexcept { return it(Left >> Right.cpp); }
  friend constexpr it operator>>(const unsigned char Left, const it Right) noexcept { return it(Left >> Right.cpp); }
  friend constexpr it operator>>(const unsigned short Left, const it Right) noexcept { return it(Left >> Right.cpp); }
  friend constexpr it operator>>(const unsigned int Left, const it Right) noexcept { return it(Left >> Right.cpp); }
  friend constexpr it operator>>(const unsigned long Left, const it Right) noexcept { return it(Left >> Right.cpp); }
  friend constexpr it operator>>(const unsigned long long Left, const it Right) noexcept { return it(Left >> Right.cpp); }

  /// assignment operators
  friend constexpr it& operator+=(it& Left, const it Right) noexcept { return Left.cpp += Right.cpp, Left; }
  friend constexpr it& operator-=(it& Left, const it Right) noexcept { return Left.cpp -= Right.cpp, Left; }
  friend constexpr it& operator*=(it& Left, const it Right) noexcept { return Left.cpp *= Right.cpp, Left; }
  friend constexpr it& operator/=(it& Left, const it Right) noexcept { return Left.cpp /= Right.cpp, Left; }
  friend constexpr it& operator&=(it& Left, const it Right) noexcept { return Left.cpp &= Right.cpp, Left; }
  friend constexpr it& operator|=(it& Left, const it Right) noexcept { return Left.cpp |= Right.cpp, Left; }
  friend constexpr it& operator^=(it& Left, const it Right) noexcept { return Left.cpp ^= Right.cpp, Left; }
  friend constexpr it& operator<<=(it& Left, const it Right) noexcept { return Left.cpp <<= Right.cpp, Left; }
  friend constexpr it& operator>>=(it& Left, const it Right) noexcept { return Left.cpp >>= Right.cpp, Left; }

  /// stream operators
  template<typename Ct, typename Tr> friend std::basic_istream<Ct, Tr>& operator>>(
    std::basic_istream<Ct, Tr>& Stream, it& Integer) { return Stream >> Integer.cpp; }
};


/// floating point type
struct ft {
  double cpp{0};

  /// default constructor
  constexpr ft() noexcept = default;

  /// constructors
  constexpr ft(const signed char Integer) noexcept : cpp(Integer) {}
  constexpr ft(const short Integer) noexcept : cpp(Integer) {}
  constexpr ft(const int Integer) noexcept : cpp(Integer) {}
  constexpr ft(const long Integer) noexcept : cpp(Integer) {}
  constexpr ft(const long long Integer) noexcept : cpp(static_cast<double>(Integer)) {}
  constexpr ft(const unsigned char Integer) noexcept : cpp(Integer) {}
  constexpr ft(const unsigned short Integer) noexcept : cpp(Integer) {}
  constexpr ft(const unsigned int Integer) noexcept : cpp(Integer) {}
  constexpr ft(const unsigned long Integer) noexcept : cpp(Integer) {}
  constexpr ft(const unsigned long long Integer) noexcept : cpp(static_cast<double>(Integer)) {}
  constexpr ft(const float Value) noexcept : cpp(Value) {}
  constexpr ft(const double Value) noexcept : cpp(Value) {}
  constexpr ft(const long double Value) noexcept : cpp(Value) {}
  constexpr ft(const it Value) noexcept : cpp(static_cast<double>(Value.cpp)) {}

  /// conversion operators
  explicit constexpr operator signed char() const noexcept { return static_cast<char>(cpp); }
  explicit constexpr operator short() const noexcept { return static_cast<short>(cpp); }
  explicit constexpr operator int() const noexcept { return static_cast<int>(cpp); }
  explicit constexpr operator long() const noexcept { return static_cast<long>(cpp); }
  explicit constexpr operator long long() const noexcept { return static_cast<long long>(cpp); }
  explicit constexpr operator unsigned char() const noexcept { return static_cast<unsigned char>(cpp); }
  explicit constexpr operator unsigned short() const noexcept { return static_cast<unsigned short>(cpp); }
  explicit constexpr operator unsigned int() const noexcept { return static_cast<unsigned int>(cpp); }
  explicit constexpr operator unsigned long() const noexcept { return static_cast<unsigned long>(cpp); }
  explicit constexpr operator unsigned long long() const noexcept { return static_cast<unsigned long long>(cpp); }
  explicit constexpr operator float() const noexcept { return static_cast<float>(cpp); }
  constexpr operator double() const noexcept { return cpp; }
  explicit constexpr operator long double() const noexcept { return static_cast<long double>(cpp); }
  explicit constexpr operator it() const noexcept { return it(static_cast<long long>(cpp)); }

  /// comparison operators
  friend constexpr bool operator==(const ft Left, const ft Right) noexcept { return Left.cpp == Right.cpp; }
  friend constexpr auto operator<=>(const ft Left, const ft Right) noexcept { return Left.cpp <=> Right.cpp; }
  friend constexpr bool operator==(const ft Left, const signed char Right) noexcept { return Left.cpp == Right; }
  friend constexpr bool operator==(const ft Left, const short Right) noexcept { return Left.cpp == Right; }
  friend constexpr bool operator==(const ft Left, const int Right) noexcept { return Left.cpp == Right; }
  friend constexpr bool operator==(const ft Left, const long Right) noexcept { return Left.cpp == Right; }
  friend constexpr bool operator==(const ft Left, const long long Right) noexcept { return Left.cpp == Right; }
  friend constexpr bool operator==(const ft Left, const unsigned char Right) noexcept { return Left.cpp == Right; }
  friend constexpr bool operator==(const ft Left, const unsigned short Right) noexcept { return Left.cpp == Right; }
  friend constexpr bool operator==(const ft Left, const unsigned int Right) noexcept { return Left.cpp == Right; }
  friend constexpr bool operator==(const ft Left, const unsigned long Right) noexcept { return Left.cpp == Right; }
  friend constexpr bool operator==(const ft Left, const unsigned long long Right) noexcept { return Left.cpp == Right; }
  friend constexpr bool operator==(const ft Left, const float Right) noexcept { return Left.cpp == Right; }
  friend constexpr bool operator==(const ft Left, const double Right) noexcept { return Left.cpp == Right; }
  friend constexpr bool operator==(const ft Left, const long double Right) noexcept { return Left.cpp == Right; }
  friend constexpr auto operator<=>(const ft Left, const signed char Right) noexcept { return Left.cpp <=> Right; }
  friend constexpr auto operator<=>(const ft Left, const short Right) noexcept { return Left.cpp <=> Right; }
  friend constexpr auto operator<=>(const ft Left, const int Right) noexcept { return Left.cpp <=> Right; }
  friend constexpr auto operator<=>(const ft Left, const long Right) noexcept { return Left.cpp <=> Right; }
  friend constexpr auto operator<=>(const ft Left, const long long Right) noexcept { return Left.cpp <=> Right; }
  friend constexpr auto operator<=>(const ft Left, const unsigned char Right) noexcept { return Left.cpp <=> Right; }
  friend constexpr auto operator<=>(const ft Left, const unsigned short Right) noexcept { return Left.cpp <=> Right; }
  friend constexpr auto operator<=>(const ft Left, const unsigned int Right) noexcept { return Left.cpp <=> Right; }
  friend constexpr auto operator<=>(const ft Left, const unsigned long Right) noexcept { return Left.cpp <=> Right; }
  friend constexpr auto operator<=>(const ft Left, const unsigned long long Right) noexcept { return Left.cpp <=> Right; }
  friend constexpr auto operator<=>(const ft Left, const float Right) noexcept { return Left.cpp <=> Right; }
  friend constexpr auto operator<=>(const ft Left, const double Right) noexcept { return Left.cpp <=> Right; }
  friend constexpr auto operator<=>(const ft Left, const long double Right) noexcept { return Left.cpp <=> Right; }
  friend constexpr bool operator==(const signed char Left, const ft Right) noexcept { return Left == Right.cpp; }
  friend constexpr bool operator==(const short Left, const ft Right) noexcept { return Left == Right.cpp; }
  friend constexpr bool operator==(const int Left, const ft Right) noexcept { return Left == Right.cpp; }
  friend constexpr bool operator==(const long Left, const ft Right) noexcept { return Left == Right.cpp; }
  friend constexpr bool operator==(const long long Left, const ft Right) noexcept { return Left == Right.cpp; }
  friend constexpr bool operator==(const unsigned char Left, const ft Right) noexcept { return Left == Right.cpp; }
  friend constexpr bool operator==(const unsigned short Left, const ft Right) noexcept { return Left == Right.cpp; }
  friend constexpr bool operator==(const unsigned int Left, const ft Right) noexcept { return Left == Right.cpp; }
  friend constexpr bool operator==(const unsigned long Left, const ft Right) noexcept { return Left == Right.cpp; }
  friend constexpr bool operator==(const unsigned long long Left, const ft Right) noexcept { return Left == Right.cpp; }
  friend constexpr bool operator==(const float Left, const ft Right) noexcept { return Left == Right.cpp; }
  friend constexpr bool operator==(const double Left, const ft Right) noexcept { return Left == Right.cpp; }
  friend constexpr bool operator==(const long double Left, const ft Right) noexcept { return Left == Right.cpp; }
  friend constexpr auto operator<=>(const signed char Left, const ft Right) noexcept { return Left <=> Right.cpp; }
  friend constexpr auto operator<=>(const short Left, const ft Right) noexcept { return Left <=> Right.cpp; }
  friend constexpr auto operator<=>(const int Left, const ft Right) noexcept { return Left <=> Right.cpp; }
  friend constexpr auto operator<=>(const long Left, const ft Right) noexcept { return Left <=> Right.cpp; }
  friend constexpr auto operator<=>(const long long Left, const ft Right) noexcept { return Left <=> Right.cpp; }
  friend constexpr auto operator<=>(const unsigned char Left, const ft Right) noexcept { return Left <=> Right.cpp; }
  friend constexpr auto operator<=>(const unsigned short Left, const ft Right) noexcept { return Left <=> Right.cpp; }
  friend constexpr auto operator<=>(const unsigned int Left, const ft Right) noexcept { return Left <=> Right.cpp; }
  friend constexpr auto operator<=>(const unsigned long Left, const ft Right) noexcept { return Left <=> Right.cpp; }
  friend constexpr auto operator<=>(const unsigned long long Left, const ft Right) noexcept { return Left <=> Right.cpp; }
  friend constexpr auto operator<=>(const float Left, const ft Right) noexcept { return Left <=> Right.cpp; }
  friend constexpr auto operator<=>(const double Left, const ft Right) noexcept { return Left <=> Right.cpp; }
  friend constexpr auto operator<=>(const long double Left, const ft Right) noexcept { return Left <=> Right.cpp; }

  /// arithmetic operators
  friend constexpr ft operator-(const ft Right) noexcept { return ft(-Right.cpp); }
  friend constexpr ft operator+(const ft Left, const ft Right) noexcept { return ft(Left.cpp + Right.cpp); }
  friend constexpr ft operator-(const ft Left, const ft Right) noexcept { return ft(Left.cpp - Right.cpp); }
  friend constexpr ft operator*(const ft Left, const ft Right) noexcept { return ft(Left.cpp * Right.cpp); }
  friend constexpr ft operator/(const ft Left, const ft Right) noexcept { return ft(Left.cpp / Right.cpp); }
  friend constexpr ft operator+(const ft Left, const signed char Right) noexcept { return ft(Left.cpp + Right); }
  friend constexpr ft operator+(const ft Left, const short Right) noexcept { return ft(Left.cpp + Right); }
  friend constexpr ft operator+(const ft Left, const int Right) noexcept { return ft(Left.cpp + Right); }
  friend constexpr ft operator+(const ft Left, const long Right) noexcept { return ft(Left.cpp + Right); }
  friend constexpr ft operator+(const ft Left, const long long Right) noexcept { return ft(Left.cpp + Right); }
  friend constexpr ft operator+(const ft Left, const unsigned char Right) noexcept { return ft(Left.cpp + Right); }
  friend constexpr ft operator+(const ft Left, const unsigned short Right) noexcept { return ft(Left.cpp + Right); }
  friend constexpr ft operator+(const ft Left, const unsigned int Right) noexcept { return ft(Left.cpp + Right); }
  friend constexpr ft operator+(const ft Left, const unsigned long Right) noexcept { return ft(Left.cpp + Right); }
  friend constexpr ft operator+(const ft Left, const unsigned long long Right) noexcept { return ft(Left.cpp + Right); }
  friend constexpr ft operator+(const ft Left, const float Right) noexcept { return ft(Left.cpp + Right); }
  friend constexpr ft operator+(const ft Left, const double Right) noexcept { return ft(Left.cpp + Right); }
  friend constexpr ft operator+(const ft Left, const long double Right) noexcept { return ft(Left.cpp + Right); }
  friend constexpr ft operator-(const ft Left, const signed char Right) noexcept { return ft(Left.cpp - Right); }
  friend constexpr ft operator-(const ft Left, const short Right) noexcept { return ft(Left.cpp - Right); }
  friend constexpr ft operator-(const ft Left, const int Right) noexcept { return ft(Left.cpp - Right); }
  friend constexpr ft operator-(const ft Left, const long Right) noexcept { return ft(Left.cpp - Right); }
  friend constexpr ft operator-(const ft Left, const long long Right) noexcept { return ft(Left.cpp - Right); }
  friend constexpr ft operator-(const ft Left, const unsigned char Right) noexcept { return ft(Left.cpp - Right); }
  friend constexpr ft operator-(const ft Left, const unsigned short Right) noexcept { return ft(Left.cpp - Right); }
  friend constexpr ft operator-(const ft Left, const unsigned int Right) noexcept { return ft(Left.cpp - Right); }
  friend constexpr ft operator-(const ft Left, const unsigned long Right) noexcept { return ft(Left.cpp - Right); }
  friend constexpr ft operator-(const ft Left, const unsigned long long Right) noexcept { return ft(Left.cpp - Right); }
  friend constexpr ft operator-(const ft Left, const float Right) noexcept { return ft(Left.cpp - Right); }
  friend constexpr ft operator-(const ft Left, const double Right) noexcept { return ft(Left.cpp - Right); }
  friend constexpr ft operator-(const ft Left, const long double Right) noexcept { return ft(Left.cpp - Right); }
  friend constexpr ft operator*(const ft Left, const signed char Right) noexcept { return ft(Left.cpp * Right); }
  friend constexpr ft operator*(const ft Left, const short Right) noexcept { return ft(Left.cpp * Right); }
  friend constexpr ft operator*(const ft Left, const int Right) noexcept { return ft(Left.cpp * Right); }
  friend constexpr ft operator*(const ft Left, const long Right) noexcept { return ft(Left.cpp * Right); }
  friend constexpr ft operator*(const ft Left, const long long Right) noexcept { return ft(Left.cpp * Right); }
  friend constexpr ft operator*(const ft Left, const unsigned char Right) noexcept { return ft(Left.cpp * Right); }
  friend constexpr ft operator*(const ft Left, const unsigned short Right) noexcept { return ft(Left.cpp * Right); }
  friend constexpr ft operator*(const ft Left, const unsigned int Right) noexcept { return ft(Left.cpp * Right); }
  friend constexpr ft operator*(const ft Left, const unsigned long Right) noexcept { return ft(Left.cpp * Right); }
  friend constexpr ft operator*(const ft Left, const unsigned long long Right) noexcept { return ft(Left.cpp * Right); }
  friend constexpr ft operator*(const ft Left, const float Right) noexcept { return ft(Left.cpp * Right); }
  friend constexpr ft operator*(const ft Left, const double Right) noexcept { return ft(Left.cpp * Right); }
  friend constexpr ft operator*(const ft Left, const long double Right) noexcept { return ft(Left.cpp * Right); }
  friend constexpr ft operator/(const ft Left, const signed char Right) noexcept { return ft(Left.cpp / Right); }
  friend constexpr ft operator/(const ft Left, const short Right) noexcept { return ft(Left.cpp / Right); }
  friend constexpr ft operator/(const ft Left, const int Right) noexcept { return ft(Left.cpp / Right); }
  friend constexpr ft operator/(const ft Left, const long Right) noexcept { return ft(Left.cpp / Right); }
  friend constexpr ft operator/(const ft Left, const long long Right) noexcept { return ft(Left.cpp / Right); }
  friend constexpr ft operator/(const ft Left, const unsigned char Right) noexcept { return ft(Left.cpp / Right); }
  friend constexpr ft operator/(const ft Left, const unsigned short Right) noexcept { return ft(Left.cpp / Right); }
  friend constexpr ft operator/(const ft Left, const unsigned int Right) noexcept { return ft(Left.cpp / Right); }
  friend constexpr ft operator/(const ft Left, const unsigned long Right) noexcept { return ft(Left.cpp / Right); }
  friend constexpr ft operator/(const ft Left, const unsigned long long Right) noexcept { return ft(Left.cpp / Right); }
  friend constexpr ft operator/(const ft Left, const float Right) noexcept { return ft(Left.cpp / Right); }
  friend constexpr ft operator/(const ft Left, const double Right) noexcept { return ft(Left.cpp / Right); }
  friend constexpr ft operator/(const ft Left, const long double Right) noexcept { return ft(Left.cpp / Right); }
  friend constexpr ft operator+(const signed char Left, const ft Right) noexcept { return ft(Left + Right.cpp); }
  friend constexpr ft operator+(const short Left, const ft Right) noexcept { return ft(Left + Right.cpp); }
  friend constexpr ft operator+(const int Left, const ft Right) noexcept { return ft(Left + Right.cpp); }
  friend constexpr ft operator+(const long Left, const ft Right) noexcept { return ft(Left + Right.cpp); }
  friend constexpr ft operator+(const long long Left, const ft Right) noexcept { return ft(Left + Right.cpp); }
  friend constexpr ft operator+(const unsigned char Left, const ft Right) noexcept { return ft(Left + Right.cpp); }
  friend constexpr ft operator+(const unsigned short Left, const ft Right) noexcept { return ft(Left + Right.cpp); }
  friend constexpr ft operator+(const unsigned int Left, const ft Right) noexcept { return ft(Left + Right.cpp); }
  friend constexpr ft operator+(const unsigned long Left, const ft Right) noexcept { return ft(Left + Right.cpp); }
  friend constexpr ft operator+(const unsigned long long Left, const ft Right) noexcept { return ft(Left + Right.cpp); }
  friend constexpr ft operator+(const float Left, const ft Right) noexcept { return ft(Left + Right.cpp); }
  friend constexpr ft operator+(const double Left, const ft Right) noexcept { return ft(Left + Right.cpp); }
  friend constexpr ft operator+(const long double Left, const ft Right) noexcept { return ft(Left + Right.cpp); }
  friend constexpr ft operator-(const signed char Left, const ft Right) noexcept { return ft(Left - Right.cpp); }
  friend constexpr ft operator-(const short Left, const ft Right) noexcept { return ft(Left - Right.cpp); }
  friend constexpr ft operator-(const int Left, const ft Right) noexcept { return ft(Left - Right.cpp); }
  friend constexpr ft operator-(const long Left, const ft Right) noexcept { return ft(Left - Right.cpp); }
  friend constexpr ft operator-(const long long Left, const ft Right) noexcept { return ft(Left - Right.cpp); }
  friend constexpr ft operator-(const unsigned char Left, const ft Right) noexcept { return ft(Left - Right.cpp); }
  friend constexpr ft operator-(const unsigned short Left, const ft Right) noexcept { return ft(Left - Right.cpp); }
  friend constexpr ft operator-(const unsigned int Left, const ft Right) noexcept { return ft(Left - Right.cpp); }
  friend constexpr ft operator-(const unsigned long Left, const ft Right) noexcept { return ft(Left - Right.cpp); }
  friend constexpr ft operator-(const unsigned long long Left, const ft Right) noexcept { return ft(Left - Right.cpp); }
  friend constexpr ft operator-(const float Left, const ft Right) noexcept { return ft(Left - Right.cpp); }
  friend constexpr ft operator-(const double Left, const ft Right) noexcept { return ft(Left - Right.cpp); }
  friend constexpr ft operator-(const long double Left, const ft Right) noexcept { return ft(Left - Right.cpp); }
  friend constexpr ft operator*(const signed char Left, const ft Right) noexcept { return ft(Left * Right.cpp); }
  friend constexpr ft operator*(const short Left, const ft Right) noexcept { return ft(Left * Right.cpp); }
  friend constexpr ft operator*(const int Left, const ft Right) noexcept { return ft(Left * Right.cpp); }
  friend constexpr ft operator*(const long Left, const ft Right) noexcept { return ft(Left * Right.cpp); }
  friend constexpr ft operator*(const long long Left, const ft Right) noexcept { return ft(Left * Right.cpp); }
  friend constexpr ft operator*(const unsigned char Left, const ft Right) noexcept { return ft(Left * Right.cpp); }
  friend constexpr ft operator*(const unsigned short Left, const ft Right) noexcept { return ft(Left * Right.cpp); }
  friend constexpr ft operator*(const unsigned int Left, const ft Right) noexcept { return ft(Left * Right.cpp); }
  friend constexpr ft operator*(const unsigned long Left, const ft Right) noexcept { return ft(Left * Right.cpp); }
  friend constexpr ft operator*(const unsigned long long Left, const ft Right) noexcept { return ft(Left * Right.cpp); }
  friend constexpr ft operator*(const float Left, const ft Right) noexcept { return ft(Left * Right.cpp); }
  friend constexpr ft operator*(const double Left, const ft Right) noexcept { return ft(Left * Right.cpp); }
  friend constexpr ft operator*(const long double Left, const ft Right) noexcept { return ft(Left * Right.cpp); }
  friend constexpr ft operator/(const signed char Left, const ft Right) noexcept { return ft(Left / Right.cpp); }
  friend constexpr ft operator/(const short Left, const ft Right) noexcept { return ft(Left / Right.cpp); }
  friend constexpr ft operator/(const int Left, const ft Right) noexcept { return ft(Left / Right.cpp); }
  friend constexpr ft operator/(const long Left, const ft Right) noexcept { return ft(Left / Right.cpp); }
  friend constexpr ft operator/(const long long Left, const ft Right) noexcept { return ft(Left / Right.cpp); }
  friend constexpr ft operator/(const unsigned char Left, const ft Right) noexcept { return ft(Left / Right.cpp); }
  friend constexpr ft operator/(const unsigned short Left, const ft Right) noexcept { return ft(Left / Right.cpp); }
  friend constexpr ft operator/(const unsigned int Left, const ft Right) noexcept { return ft(Left / Right.cpp); }
  friend constexpr ft operator/(const unsigned long Left, const ft Right) noexcept { return ft(Left / Right.cpp); }
  friend constexpr ft operator/(const unsigned long long Left, const ft Right) noexcept { return ft(Left / Right.cpp); }
  friend constexpr ft operator/(const float Left, const ft Right) noexcept { return ft(Left / Right.cpp); }
  friend constexpr ft operator/(const double Left, const ft Right) noexcept { return ft(Left / Right.cpp); }
  friend constexpr ft operator/(const long double Left, const ft Right) noexcept { return ft(Left / Right.cpp); }

  /// assignment operators
  friend constexpr ft& operator+=(ft& Left, const ft Right) noexcept { return Left.cpp += Right.cpp, Left; }
  friend constexpr ft& operator-=(ft& Left, const ft Right) noexcept { return Left.cpp -= Right.cpp, Left; }
  friend constexpr ft& operator*=(ft& Left, const ft Right) noexcept { return Left.cpp *= Right.cpp, Left; }
  friend constexpr ft& operator/=(ft& Left, const ft Right) noexcept { return Left.cpp /= Right.cpp, Left; }

  /// stream operators
  template<typename Ct, typename Tr> friend std::basic_istream<Ct, Tr>& operator>>(
    std::basic_istream<Ct, Tr>& Stream, ft& Float) { return Stream >> Float.cpp; }
};


/// character type
struct ct {

  /// converts utf-8 to utf-32
  static constexpr it utf8_to_utf32(const char8_t* Chars, char32_t& Out) noexcept {
    if ((Chars[0] & 0x80) == 0) return Out = Chars[0], 1;
    if ((Chars[0] & 0xE0) == 0xC0) return Out = ((Chars[0] & 0x1F) << 6) | (Chars[1] & 0x3F), 2;
    if ((Chars[0] & 0xF0) == 0xE0) return Out = ((Chars[0] & 0x0F) << 12) | ((Chars[1] & 0x3F) << 6) | (Chars[2] & 0x3F), 3;
    if ((Chars[0] & 0xF8) == 0xF0) return Out = ((Chars[0] & 0x07) << 18) | ((Chars[1] & 0x3F) << 12) | ((Chars[2] & 0x3F) << 6) | (Chars[3] & 0x3F), 4;
    return 0;
  }

  /// converts utf-16 to utf-32
  static constexpr it utf16_to_utf32(const char16_t* Chars, char32_t& Out) noexcept {
    if (Chars[0] < 0xD800 || Chars[0] > 0xDFFF) return Out = Chars[0], 1;
    if (Chars[0] < 0xDC00) return Out = 0x10000 + ((Chars[0] & 0x3FF) << 10) + (Chars[1] & 0x3FF), 2;
    return 0;
  }

  /// converts utf-32 to utf-8
  static constexpr it utf32_to_utf8(const char32_t Char, char8_t* Out) noexcept {
    if (Char < 0x80) return Out[0] = char8_t(Char), 1;
    if (Char < 0x800) return Out[0] = char8_t(0xC0 | (Char >> 6)), Out[1] = char8_t(0x80 | (Char & 0x3F)), 2;
    if (Char < 0x10000) return Out[0] = char8_t(0xE0 | (Char >> 12)), Out[1] = char8_t(0x80 | ((Char >> 6) & 0x3F)), Out[2] = char8_t(0x80 | (Char & 0x3F)), 3;
    if (Char < 0x110000) return Out[0] = char8_t(0xF0 | (Char >> 18)), Out[1] = char8_t(0x80 | ((Char >> 12) & 0x3F)),
                                Out[2] = char8_t(0x80 | ((Char >> 6) & 0x3F)), Out[3] = char8_t(0x80 | (Char & 0x3F)), 4;
    return 0;
  }

  /// converts utf-32 to utf-16
  static constexpr it utf32_to_utf16(const char32_t Char, char16_t* Out) noexcept {
    if (Char < 0x10000) return Out[0] = char16_t(Char), 1;
    if (Char < 0x110000) return Out[0] = char16_t(0xD800 | ((Char - 0x10000) >> 10)), Out[1] = char16_t(0xDC00 | ((Char - 0x10000) & 0x3FF)), 2;
    return 0;
  }

  char32_t cpp{0};

  /// default constructor
  constexpr ct() noexcept = default;

  /// constructor from a single character
  explicit constexpr ct(const char Char) noexcept : cpp(Char < 0x80 ? Char : 0) {}
  explicit constexpr ct(const wchar_t Char) noexcept : cpp(Char < 0xD800 ? Char : 0) {}
  explicit constexpr ct(const char8_t Char) noexcept : cpp(Char < 0x80 ? Char : 0) {}
  explicit constexpr ct(const char16_t Char) noexcept : cpp(Char < 0xD800 ? Char : 0) {}
  constexpr ct(const char32_t Char) noexcept : cpp(Char) {}

  /// conversion to a single character
  explicit constexpr operator char() const noexcept { return cpp < 0x80 ? static_cast<char>(cpp) : 0; }
  explicit constexpr operator wchar_t() const noexcept { return cpp < 0xD800 ? static_cast<wchar_t>(cpp) : 0; }
  explicit constexpr operator char8_t() const noexcept { return cpp < 0x80 ? static_cast<char8_t>(cpp) : 0; }
  explicit constexpr operator char16_t() const noexcept { return cpp < 0xD800 ? static_cast<char16_t>(cpp) : 0; }
  constexpr operator char32_t() const noexcept { return cpp; }

  /// comparison operators
  friend constexpr bool operator==(const ct Left, const ct Right) noexcept { return Left.cpp == Right.cpp; }
  friend constexpr auto operator<=>(const ct Left, const ct Right) noexcept { return Left.cpp <=> Right.cpp; }

  /// arithmetic operators
  friend constexpr ct operator+(const ct Left, const ct Right) noexcept { return ct(char32_t(Left.cpp + Right.cpp)); }
  friend constexpr ct operator-(const ct Left, const ct Right) noexcept { return ct(char32_t(Left.cpp - Right.cpp)); }
  friend constexpr ct operator*(const ct Left, const ct Right) noexcept { return ct(char32_t(Left.cpp * Right.cpp)); }
  friend constexpr ct operator/(const ct Left, const ct Right) noexcept { return ct(char32_t(Left.cpp / Right.cpp)); }

  /// bitwise operators
  friend constexpr ct operator&(const ct Left, const ct Right) noexcept { return ct(char32_t(Left.cpp & Right.cpp)); }
  friend constexpr ct operator|(const ct Left, const ct Right) noexcept { return ct(char32_t(Left.cpp | Right.cpp)); }
  friend constexpr ct operator^(const ct Left, const ct Right) noexcept { return ct(char32_t(Left.cpp ^ Right.cpp)); }

  /// unary operators
  friend constexpr ct operator~(const ct Right) noexcept { return ct(char32_t(~Right.cpp)); }

  /// assignment operators
  friend constexpr ct& operator+=(ct& Left, const ct Right) noexcept { return Left.cpp += Right.cpp, Left; }
  friend constexpr ct& operator-=(ct& Left, const ct Right) noexcept { return Left.cpp -= Right.cpp, Left; }
  friend constexpr ct& operator*=(ct& Left, const ct Right) noexcept { return Left.cpp *= Right.cpp, Left; }
  friend constexpr ct& operator/=(ct& Left, const ct Right) noexcept { return Left.cpp /= Right.cpp, Left; }
  friend constexpr ct& operator&=(ct& Left, const ct Right) noexcept { return Left.cpp &= Right.cpp, Left; }
  friend constexpr ct& operator|=(ct& Left, const ct Right) noexcept { return Left.cpp |= Right.cpp, Left; }
  friend constexpr ct& operator^=(ct& Left, const ct Right) noexcept { return Left.cpp ^= Right.cpp, Left; }

  /// stream operators
  friend std::wostream& operator<<(std::wostream& Stream, const ct Char) {
    char16_t Chars[3];
    Chars[ct::utf32_to_utf16(Char.cpp, Chars)] = 0;
    return Stream << reinterpret_cast<wchar_t*>(Chars);
  }
  friend std::wistream& operator>>(std::wistream& Stream, ct& Char) {
    std::wstring Chars;
    Stream >> Chars;
    return ct::utf16_to_utf32(reinterpret_cast<const char16_t*>(Chars.data()), Char.cpp), Stream;
  }
};


/// string type
struct st {

  /// converts a multibyte string to a utf-16 string
  static std::u16string mbs_to_utf16(const std::string_view String, const unsigned int CodePage = __ywlib_codepage__) {
    std::u16string Out(::MultiByteToWideChar(CodePage, 0, String.data(), static_cast<int>(String.size()), nullptr, 0), 0);
    ::MultiByteToWideChar(CodePage, 0, String.data(), static_cast<int>(String.size()), reinterpret_cast<wchar_t*>(Out.data()), static_cast<int>(Out.size()));
    return Out;
  }

  /// converts a utf-16 string to a multibyte string
  static std::string utf16_to_mbs(const std::u16string_view String, const unsigned int CodePage = __ywlib_codepage__) {
    std::string Out(::WideCharToMultiByte(CodePage, 0, reinterpret_cast<const wchar_t*>(String.data()), static_cast<int>(String.size()), nullptr, 0, nullptr, nullptr), 0);
    ::WideCharToMultiByte(CodePage, 0, reinterpret_cast<const wchar_t*>(String.data()), static_cast<int>(String.size()), Out.data(), static_cast<int>(Out.size()), nullptr, nullptr);
    return Out;
  }

  /// converts a utf-8 string to a utf-16 string
  static constexpr std::u16string utf8_to_utf16(const std::u8string_view String) {
    std::u16string Out(String.size(), 0);
    char32_t Char;
    for (size_t i{}, j{};;) {
      i += ct::utf8_to_utf32(String.data() + i, Char);
      j += ct::utf32_to_utf16(Char, Out.data() + j);
      if (i >= String.size()) return Out.resize(j), std::move(Out);
    }
    return Out;
  }

  /// converts a utf-8 string to a utf-32 string
  static constexpr std::u32string utf8_to_utf32(const std::u8string_view String) {
    std::u32string Out(String.size(), 0);
    for (size_t i{}, j{};;) {
      i += ct::utf8_to_utf32(String.data() + i, Out[j++]);
      if (i >= String.size()) return Out.resize(j), std::move(Out);
    }
    return Out;
  }

  /// converts a utf-16 string to a utf-8 string
  static constexpr std::u8string utf16_to_utf8(const std::u16string_view String) {
    std::u8string Out(String.size() * 4, 0);
    char32_t Char;
    for (size_t i{}, j{};;) {
      i += ct::utf16_to_utf32(String.data() + i, Char);
      j += ct::utf32_to_utf8(Char, Out.data() + j);
      if (i >= String.size()) return Out.resize(j), std::move(Out);
    }
    return Out;
  }

  /// converts a utf-16 string to a utf-32 string
  static constexpr std::u32string utf16_to_utf32(const std::u16string_view String) {
    std::u32string Out(String.size(), 0);
    for (size_t i{}, j{};;) {
      i += ct::utf16_to_utf32(String.data() + i, Out[j++]);
      if (i >= String.size()) return Out.resize(j), std::move(Out);
    }
    return Out;
  }

  /// converts a utf-32 string to a utf-8 string
  static constexpr std::u8string utf32_to_utf8(const std::u32string_view String) {
    std::u8string Out(String.size() * 4, 0);
    for (size_t i{}, j{};;) {
      j += ct::utf32_to_utf8(String[i++], Out.data() + j);
      if (i >= String.size()) return Out.resize(j), std::move(Out);
    }
    return Out;
  }

  /// converts a utf-32 string to a utf-16 string
  static constexpr std::u16string utf32_to_utf16(const std::u32string_view String) {
    std::u16string Out(String.size() * 2, 0);
    for (size_t i{}, j{};;) {
      j += ct::utf32_to_utf16(String[i++], Out.data() + j);
      if (i >= String.size()) return Out.resize(j), std::move(Out);
    }
    return Out;
  }

  std::u32string cpp;

  /// default constructor
  constexpr st() noexcept = default;

  /// constructors
  explicit st(const std::string_view String) : cpp(utf16_to_utf32(mbs_to_utf16(String))) {}
  explicit constexpr st(const std::wstring_view String) : cpp(utf16_to_utf32(reinterpret_cast<const std::u16string_view&>(String))) {}
  explicit constexpr st(const std::u8string_view String) : cpp(utf8_to_utf32(String)) {}
  explicit constexpr st(const std::u16string_view String) : cpp(utf16_to_utf32(String)) {}
  constexpr st(const std::u32string_view String) : cpp(String) {}
  constexpr st(std::u32string&& String) : cpp(std::move(String)) {}
  explicit constexpr st(it Size, ct Fill = U' ') : cpp(Size.cpp, Fill.cpp) {}
};
}

#endif // #ifndef __ywlib__
