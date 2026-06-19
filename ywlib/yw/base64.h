#pragma once
#include "yw/core.h"

namespace yw::base64 {

template<typename T> concept is_valid_value_type = std::is_trivially_copyable_v<T> && sizeof(T) == 1;

inline constexpr char _encode_table[] =
  {                                                                                 //
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', //
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', //
    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', //
    'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'};

static constexpr uint8_t _decode_table[] =
  {                                                                 //
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, //
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, //
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 62, 99, 99, 99, 63, //
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 99, 99, 99, 98, 99, 99, //
    99, +0, +1, +2, +3, +4, +5, +6, +7, +8, +9, 10, 11, 12, 13, 14, //
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 99, 99, 99, 99, 99, //
    99, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, //
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 99, 99, 99, 99, 99};

/// encodes data
inline constexpr struct {
  /// encodes data
  template<input_iterator It, sentinel_for<It> Se, output_iterator<char> Out> //
  requires is_valid_value_type<iter_value_t<It>> && is_valid_value_type<iter_value_t<Out>>
  static constexpr std::ranges::in_out_result<It, Out> operator()(It it, Se se, Out out) {
    while (it != se) {
      const auto b0 = uint_cast(*it);
      if (++it; it == se) {
        *out++ = _encode_table[b0 >> 2];
        *out++ = _encode_table[(b0 & 3) << 4];
        *out++ = '=';
        *out++ = '=';
        return {it, out};
      }
      const auto b1 = uint_cast(*it);
      if (++it; it == se) {
        *out++ = _encode_table[b0 >> 2];
        *out++ = _encode_table[((b0 & 3) << 4) | (b1 >> 4)];
        *out++ = _encode_table[(b1 & 15) << 2];
        *out++ = '=';
        return {it, out};
      }
      const auto b2 = uint_cast(*it);
      ++it;
      *out++ = _encode_table[b0 >> 2];
      *out++ = _encode_table[((b0 & 3) << 4) | (b1 >> 4)];
      *out++ = _encode_table[((b1 & 15) << 2) | (b2 >> 6)];
      *out++ = _encode_table[b2 & 63];
    }
    return {it, out};
  }

  /// encodes data
  template<input_range Rg, output_iterator<char> Out> //
  requires is_valid_value_type<iter_value_t<Rg>> && is_valid_value_type<iter_value_t<Out>>
  static constexpr std::ranges::in_out_result<iterator_t<Rg>, Out> operator()(Rg&& r, Out out) {
    return operator()(std::ranges::begin(r), std::ranges::end(r), out);
  }

  /// returns required buffer size
  static constexpr size_t size(size_t input_size) { return ((input_size + 2) / 3) * 4; }
} encode;

/// decodes base64 data
inline constexpr struct {
  /// decodes base64 data
  template<input_iterator It, sentinel_for<It> Se, output_iterator<uint8_t> Out> //
  requires is_valid_value_type<iter_value_t<It>> && is_valid_value_type<iter_value_t<Out>>
  static constexpr std::ranges::in_out_result<It, Out> operator()(It it, Se se, Out out) {
    while (it != se) {
      const auto v0 = _decode_table[uint_cast(*it++)];
      const auto v1 = _decode_table[uint_cast(*it++)];
      *out++ = std::bit_cast<iter_value_t<Out>>(uint8_t((v0 << 2) | (v1 >> 4)));
      const auto v2 = _decode_table[uint_cast(*it++)];
      const auto v3 = _decode_table[uint_cast(*it++)];
      if (v2 < 64) {
        *out++ = std::bit_cast<iter_value_t<Out>>(uint8_t((v1 << 4) | (v2 >> 2)));
        if (v3 < 64) *out++ = std::bit_cast<iter_value_t<Out>>(uint8_t((v2 << 6) | v3));
        else break;
      } else break;
    }
    return {it, out};
  }

  /// decodes base64 data
  template<input_range Rg, output_iterator<uint8_t> Out> //
  requires is_valid_value_type<iter_value_t<Rg>> && is_valid_value_type<iter_value_t<Out>>
  static constexpr std::ranges::in_out_result<iterator_t<Rg>, Out> operator()(Rg&& r, Out out) {
    return operator()(std::ranges::begin(r), std::ranges::end(r), out);
  }

  /// decodes base64 data in-place
  template<contiguous_iterator It, sentinel_for<It> Se> //
  requires is_valid_value_type<iter_value_t<It>> && output_iterator<It, iter_value_t<It>>
  static constexpr std::ranges::in_out_result<It, It> inplace(It it, Se se) {
    auto out = it;
    while (it != se) {
      const auto v0 = _decode_table[uint_cast(*it++)];
      const auto v1 = _decode_table[uint_cast(*it++)];
      *out++ = std::bit_cast<iter_value_t<It>>(uint8_t((v0 << 2) | (v1 >> 4)));
      const auto v2 = _decode_table[uint_cast(*it++)];
      const auto v3 = _decode_table[uint_cast(*it++)];
      if (v2 < 64) {
        *out++ = std::bit_cast<iter_value_t<It>>(uint8_t((v1 << 4) | (v2 >> 2)));
        if (v3 < 64) *out++ = std::bit_cast<iter_value_t<It>>(uint8_t((v2 << 6) | v3));
        else break;
      } else break;
    }
    return {it, out};
  }

  /// decodes base64 data in-place
  template<contiguous_range Rg> //
  requires is_valid_value_type<iter_value_t<Rg>> && output_iterator<iterator_t<Rg>, iter_value_t<Rg>>
  static constexpr std::ranges::in_out_result<iterator_t<Rg>, iterator_t<Rg>> inplace(Rg&& r) {
    return inplace(std::ranges::begin(r), std::ranges::end(r));
  }

  /// returns size of decoded data
  static constexpr size_t size(size_t input_size) { return (input_size / 4) * 3; }
} decode;
} // namespace yw::base64
