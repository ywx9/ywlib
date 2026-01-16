#pragma once
#include "yw/core.h"

namespace yw {
namespace internal {
static constexpr char _b64_encode_table[] = {
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', //
  'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', //
  'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', //
  'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'};
static constexpr unsigned char _b64_decode_table[] = {            //
  99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, //
  99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, //
  99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 62, 99, 99, 99, 63, //
  52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 99, 99, 99, 98, 99, 99, //
  99, +0, +1, +2, +3, +4, +5, +6, +7, +8, +9, 10, 11, 12, 13, 14, //
  15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 99, 99, 99, 99, 99, //
  99, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 99, 99,
  99, 99, 99};
inline constexpr std::string _b64_encode(const void* data, size_t size) {
  const auto input_bits = size * 8;
  const auto blocks = (input_bits + 5) / 6;
  const auto output_bytes = (blocks + 3) / 4 * 4;
  std::string result(output_bytes, '=');
  auto it = reinterpret_cast<const unsigned char*>(data);
  const auto end = it + size;
  auto out = result.data();
  for (; it + 3 <= end; it += 3) {
    *out++ = _b64_encode_table[*it >> 2];
    *out++ = _b64_encode_table[((*it & 3) << 4) | (it[1] >> 4)];
    *out++ = _b64_encode_table[((it[1] & 15) << 2) | (it[2] >> 6)];
    *out++ = _b64_encode_table[it[2] & 63];
  }
  if (it + 1 == end) {
    *out++ = _b64_encode_table[*it >> 2];
    *out++ = _b64_encode_table[(*it & 3) << 4];
  } else if (it + 2 == end) {
    *out++ = _b64_encode_table[*it >> 2];
    *out++ = _b64_encode_table[(*it & 3) << 4 | (it[1] >> 4)];
    *out++ = _b64_encode_table[(it[1] & 15) << 2];
  }
  return result;
}
template<contiguous_range Out> std::expected<Out, error_trace> _b64_decode(stringable<char> auto&& encoded) {
  std::string_view sv(encoded);
  using C = iter_value_t<Out>;
  const size_t m = sv.size();
  if (m % 4 != 0) return unexpected_error(errors::invalid_argument, "base64::decode: invalid length");
  if (m == 0) return {};
  Out result;
  result.reserve((m * 3) / 4);
  const char* it = sv.data();
  for (const char* const end = it + m; it < end; it += 4) {
    const auto a = internal::_b64_decode_table[*it];
    if (a >= 64) return unexpected_error(errors::invalid_argument, "base64::decode: invalid character");
    const auto b = internal::_b64_decode_table[*(it + 1)];
    if (b >= 64) return unexpected_error(errors::invalid_argument, "base64::decode: invalid character");
    result.emplace_back(C((a << 2) | (b >> 4)));
    const auto c = internal::_b64_decode_table[*(it + 2)];
    const auto d = internal::_b64_decode_table[*(it + 3)];
    if (c < 64) {
      result.emplace_back(C((b << 4) | (c >> 2)));
      if (d >= 64) {
        if (d == 98 && it + 4 == end) break;
        if (d != 98) return unexpected_error(errors::invalid_argument, "base64::decode: invalid padding");
        else return unexpected_error(errors::invalid_argument, "base64::decode: invalid padding");
      } else result.emplace_back(C((c << 6) | d));
    } else if (c == 98 && d == 98 && it + 4 == end) break;
    else if (c == 98) return unexpected_error(errors::invalid_argument, "base64::decode: invalid padding");
    else return unexpected_error(errors::invalid_argument, "base64::decode: invalid character");
  }
  return result;
}
} // namespace internal

struct _base64 {
  static constexpr std::string encode(const void* Data, size_t Size) { return internal::_b64_encode(Data, Size); }
  template<contiguous_range R> static constexpr std::string encode(const R& Data) {
    return internal::_b64_encode(std::ranges::data(Data), std::ranges::size(Data));
  }
  static constexpr std::expected<std::vector<std::byte>, error_trace> decode(stringable<char> auto&& encoded) {
    return internal::_b64_decode<std::vector<std::byte>>(encoded);
  }
  static constexpr std::expected<std::string, error_trace> decode_as_string(stringable<char> auto&& encoded) {
    return internal::_b64_decode<std::string>(encoded);
  }
};

inline constexpr _base64 base64{};
} // namespace yw
