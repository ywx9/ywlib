#pragma once

#include <initializer_list>
#include <stdexcept>

namespace yw {

/// @brief generic data type.
struct lxiv {
  union {
    char c8[64];
    wchar_t c16[32];
    char8_t u8[64];
    char16_t u16[32];
    char32_t u32[16];
    signed char i8[64];
    short i16[32];
    int i32[16];
    long long i64[8];
    unsigned char n8[64];
    unsigned short n16[32];
    unsigned int n32[16];
    unsigned long long n64[8];
    float f32[16];
    double f64[8];
  };
  /// @brief type of the lxiv object.
  enum class type : unsigned { // clang-format off
    c8, c16,
    u8, u16, u32,
    i8, i16, i32, i64,
    n8, n16, n32, n64,
    f32, f64} t; // clang-format on
  /// @brief number of elements in the lxiv object.
  unsigned count;
  /// @brief constructs a lxiv object from a charater.
  constexpr lxiv(char c) : c8{c}, t(type::c8), count(1) {}
  /// @brief constructs a lxiv object from a charater.
  constexpr lxiv(wchar_t c) : c16{c}, t(type::c16), count(1) {}
  /// @brief constructs a lxiv object from a charater.
  constexpr lxiv(char8_t c) : u8{c}, t(type::u8), count(1) {}
  /// @brief constructs a lxiv object from a charater.
  constexpr lxiv(char16_t c) : u16{c}, t(type::u16), count(1) {}
  /// @brief constructs a lxiv object from a charater.
  constexpr lxiv(char32_t c) : u32{c}, t(type::u32), count(1) {}
  /// @brief constructs a lxiv object from a signed integer.
  constexpr lxiv(signed char i) : i8{i}, t(type::i8), count(1) {}
  /// @brief constructs a lxiv object from a signed integer.
  constexpr lxiv(short i) : i16{i}, t(type::i16), count(1) {}
  /// @brief constructs a lxiv object from a signed integer.
  constexpr lxiv(int i) : i32{i}, t(type::i32), count(1) {}
  /// @brief constructs a lxiv object from a signed integer.
  constexpr lxiv(long long i) : i64{i}, t(type::i64), count(1) {}
  /// @brief constructs a lxiv object from a unsigned integer.
  constexpr lxiv(unsigned char n) : n8{n}, t(type::n8), count(1) {}
  /// @brief constructs a lxiv object from a unsigned integer.
  constexpr lxiv(unsigned short n) : n16{n}, t(type::n16), count(1) {}
  /// @brief constructs a lxiv object from a unsigned integer.
  constexpr lxiv(unsigned int n) : n32{n}, t(type::n32), count(1) {}
  /// @brief constructs a lxiv object from a unsigned integer.
  constexpr lxiv(unsigned long long n) : n64{n}, t(type::n64), count(1) {}
  /// @brief constructs a lxiv object from a floating point number.
  constexpr lxiv(float f) : f32{f}, t(type::f32), count(1) {}
  /// @brief constructs a lxiv object from a floating point number.
  constexpr lxiv(double f) : f64{f}, t(type::f64), count(1) {}
  /// @brief constructs a lxiv object from a initializer list.
  constexpr lxiv(const std::initializer_list<char>& il) : c8(), t(type::c8), count(il.size() < 64 ? il.size() : 64) {
    for (int j{}; j < count; ++j) c8[j] = il.begin()[j];
  }
  /// @brief constructs a lxiv object from a initializer list.
  constexpr lxiv(const std::initializer_list<wchar_t>& il) : c16(), t(type::c16), count(il.size() < 32 ? il.size() : 32) {
    for (int j{}; j < count; ++j) c16[j] = il.begin()[j];
  }
  /// @brief constructs a lxiv object from a initializer list.
  constexpr lxiv(const std::initializer_list<char8_t>& il) : u8(), t(type::u8), count(il.size() < 64 ? il.size() : 64) {
    for (int j{}; j < count; ++j) u8[j] = il.begin()[j];
  }
  /// @brief constructs a lxiv object from a initializer list.
  constexpr lxiv(const std::initializer_list<char16_t>& il) : u16(), t(type::u16), count(il.size() < 32 ? il.size() : 32) {
    for (int j{}; j < count; ++j) u16[j] = il.begin()[j];
  }
  /// @brief constructs a lxiv object from a initializer list.
  constexpr lxiv(const std::initializer_list<char32_t>& il) : u32(), t(type::u32), count(il.size() < 16 ? il.size() : 16) {
    for (int j{}; j < count; ++j) u32[j] = il.begin()[j];
  }
  /// @brief constructs a lxiv object from a initializer list.
  constexpr lxiv(const std::initializer_list<signed char>& il) : i8(), t(type::i8), count(il.size() < 64 ? il.size() : 64) {
    for (int j{}; j < count; ++j) i8[j] = il.begin()[j];
  }
  /// @brief constructs a lxiv object from a initializer list.
  constexpr lxiv(const std::initializer_list<short>& il) : i16(), t(type::i16), count(il.size() < 32 ? il.size() : 32) {
    for (int j{}; j < count; ++j) i16[j] = il.begin()[j];
  }
  /// @brief constructs a lxiv object from a initializer list.
  constexpr lxiv(const std::initializer_list<int>& il) : i32(), t(type::i32), count(il.size() < 16 ? il.size() : 16) {
    for (int j{}; j < count; ++j) i32[j] = il.begin()[j];
  }
  /// @brief constructs a lxiv object from a initializer list.
  constexpr lxiv(const std::initializer_list<long long>& il) : i64(), t(type::i64), count(il.size() < 8 ? il.size() : 8) {
    for (int j{}; j < count; ++j) i64[j] = il.begin()[j];
  }
  /// @brief constructs a lxiv object from a initializer list.
  constexpr lxiv(const std::initializer_list<unsigned char>& il) : n8(), t(type::n8), count(il.size() < 64 ? il.size() : 64) {
    for (int j{}; j < count; ++j) n8[j] = il.begin()[j];
  }
  /// @brief constructs a lxiv object from a initializer list.
  constexpr lxiv(const std::initializer_list<unsigned short>& il) : n16(), t(type::n16), count(il.size() < 32 ? il.size() : 32) {
    for (int j{}; j < count; ++j) n16[j] = il.begin()[j];
  }
  /// @brief constructs a lxiv object from a initializer list.
  constexpr lxiv(const std::initializer_list<unsigned int>& il) : n32(), t(type::n32), count(il.size() < 16 ? il.size() : 16) {
    for (int j{}; j < count; ++j) n32[j] = il.begin()[j];
  }
  /// @brief constructs a lxiv object from a initializer list.
  constexpr lxiv(const std::initializer_list<unsigned long long>& il) : n64(), t(type::n64), count(il.size() < 8 ? il.size() : 8) {
    for (int j{}; j < count; ++j) n64[j] = il.begin()[j];
  }
  /// @brief constructs a lxiv object from a initializer list.
  constexpr lxiv(const std::initializer_list<float>& il) : f32(), t(type::f32), count(il.size() < 16 ? il.size() : 16) {
    for (int j{}; j < count; ++j) f32[j] = il.begin()[j];
  }
  /// @brief constructs a lxiv object from a initializer list.
  constexpr lxiv(const std::initializer_list<double>& il) : f64(), t(type::f64), count(il.size() < 8 ? il.size() : 8) {
    for (int j{}; j < count; ++j) f64[j] = il.begin()[j];
  }
  /// @brief constructs a lxiv object from a string literal.
  template<int n> requires(n < 64) constexpr lxiv(const char (&s)[n]) : c8(), t(type::c), count(n - 1) {
    for (int i{}; i < n - 1; ++i) c8[i] = s[i];
  }
  /// @brief constructs a lxiv object from a string literal.
  template<int n> requires(n < 32) constexpr lxiv(const wchar_t (&s)[n]) : c16(), t(type::w), count(n - 1) {
    for (int i{}; i < n - 1; ++i) c16[i] = s[i];
  }
  /// @brief constructs a lxiv object from a string literal.
  template<int n> requires(n < 64) constexpr lxiv(const char8_t (&s)[n]) : u8(), t(type::u8), count(n - 1) {
    for (int i{}; i < n - 1; ++i) u8[i] = s[i];
  }
  /// @brief constructs a lxiv object from a string literal.
  template<int n> requires(n < 32) constexpr lxiv(const char16_t (&s)[n]) : u16(), t(type::u16), count(n - 1) {
    for (int i{}; i < n - 1; ++i) u16[i] = s[i];
  }
  /// @brief constructs a lxiv object from a string literal.
  template<int n> requires(n < 16) constexpr lxiv(const char32_t (&s)[n]) : u32(), t(type::u32), count(n - 1) {
    for (int i{}; i < n - 1; ++i) u32[i] = s[i];
  }
  /// @brief converts the lxiv object to a character.
  constexpr operator char() const {
    if (t != type::c8) throw std::logic_error("invalid type");
    return c8[0];
  }
  /// @brief converts the lxiv object to a character.
  constexpr operator wchar_t() const {
    if (t != type::c16) throw std::logic_error("invalid type");
    return c16[0];
  }
  /// @brief converts the lxiv object to a character.
  constexpr operator char8_t() const {
    if (t != type::u8) throw std::logic_error("invalid type");
    return u8[0];
  }
  /// @brief converts the lxiv object to a character.
  constexpr operator char16_t() const {
    if (t != type::u16) throw std::logic_error("invalid type");
    return u16[0];
  }
  /// @brief converts the lxiv object to a character.
  constexpr operator char32_t() const {
    if (t != type::u32) throw std::logic_error("invalid type");
    return u32[0];
  }
  /// @brief converts the lxiv object to a signed integer.
  constexpr operator signed char() const {
    if (t != type::i8) throw std::logic_error("invalid type");
    return i8[0];
  }
  /// @brief converts the lxiv object to a signed integer.
  constexpr operator short() const {
    if (t != type::i16) throw std::logic_error("invalid type");
    return i16[0];
  }
  /// @brief converts the lxiv object to a signed integer.
  constexpr operator int() const {
    if (t != type::i32) throw std::logic_error("invalid type");
    return i32[0];
  }
  /// @brief converts the lxiv object to a signed integer.
  constexpr operator long long() const {
    if (t != type::i64) throw std::logic_error("invalid type");
    return i64[0];
  }
  /// @brief converts the lxiv object to a unsigned integer.
  constexpr operator unsigned char() const {
    if (t != type::n8) throw std::logic_error("invalid type");
    return n8[0];
  }
  /// @brief converts the lxiv object to a unsigned integer.
  constexpr operator unsigned short() const {
    if (t != type::n16) throw std::logic_error("invalid type");
    return n16[0];
  }
  /// @brief converts the lxiv object to a unsigned integer.
  constexpr operator unsigned int() const {
    if (t != type::n32) throw std::logic_error("invalid type");
    return n32[0];
  }
  /// @brief converts the lxiv object to a unsigned integer.
  constexpr operator unsigned long long() const {
    if (t != type::n64) throw std::logic_error("invalid type");
    return n64[0];
  }
  /// @brief converts the lxiv object to a floating point number.
  constexpr operator float() const {
    if (t != type::f32) throw std::logic_error("invalid type");
    return f32[0];
  }
  /// @brief converts the lxiv object to a floating point number.
  constexpr operator double() const {
    if (t != type::f64) throw std::logic_error("invalid type");
    return f64[0];
  }
  /// @brief converts the lxiv object to a string.
  constexpr operator const char*() const {
    if (t != type::c8) throw std::logic_error("invalid type");
    return c8;
  }
  /// @brief converts the lxiv object to a string.
  constexpr operator const wchar_t*() const {
    if (t != type::c16) throw std::logic_error("invalid type");
    return c16;
  }
  /// @brief converts the lxiv object to a string.
  constexpr operator const char8_t*() const {
    if (t != type::u8) throw std::logic_error("invalid type");
    return u8;
  }
  /// @brief converts the lxiv object to a string.
  constexpr operator const char16_t*() const {
    if (t != type::u16) throw std::logic_error("invalid type");
    return u16;
  }
  /// @brief converts the lxiv object to a string.
  constexpr operator const char32_t*() const {
    if (t != type::u32) throw std::logic_error("invalid type");
    return u32;
  }
  /// @brief converts the lxiv object to an array.
  constexpr operator const signed char*() const {
    if (t != type::i8) throw std::logic_error("invalid type");
    return i8;
  }
  /// @brief converts the lxiv object to an array.
  constexpr operator const short*() const {
    if (t != type::i16) throw std::logic_error("invalid type");
    return i16;
  }
  /// @brief converts the lxiv object to an array.
  constexpr operator const int*() const {
    if (t != type::i32) throw std::logic_error("invalid type");
    return i32;
  }
  /// @brief converts the lxiv object to an array.
  constexpr operator const long long*() const {
    if (t != type::i64) throw std::logic_error("invalid type");
    return i64;
  }
  /// @brief converts the lxiv object to an array.
  constexpr operator const unsigned char*() const {
    if (t != type::n8) throw std::logic_error("invalid type");
    return n8;
  }
  /// @brief converts the lxiv object to an array.
  constexpr operator const unsigned short*() const {
    if (t != type::n16) throw std::logic_error("invalid type");
    return n16;
  }
  /// @brief converts the lxiv object to an array.
  constexpr operator const unsigned int*() const {
    if (t != type::n32) throw std::logic_error("invalid type");
    return n32;
  }
  /// @brief converts the lxiv object to an array.
  constexpr operator const unsigned long long*() const {
    if (t != type::n64) throw std::logic_error("invalid type");
    return n64;
  }
  /// @brief converts the lxiv object to an array.
  constexpr operator const float*() const {
    if (t != type::f32) throw std::logic_error("invalid type");
    return f32;
  }
  /// @brief converts the lxiv object to an array.
  constexpr operator const double*() const {
    if (t != type::f64) throw std::logic_error("invalid type");
    return f64;
  }
};
}
