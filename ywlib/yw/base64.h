#pragma once
#include <array>
#include <ranges>
#include <stdexcept>
#include <string>
#include <string_view>

namespace yw {

namespace internal {
static constexpr char encode_table[] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', //
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', //
    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', //
    'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'};
static constexpr unsigned char decode_table[] = {                   //
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, //
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, //
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 62, 99, 99, 99, 63, //
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 99, 99, 99, 98, 99, 99, //
    99, +0, +1, +2, +3, +4, +5, +6, +7, +8, +9, 10, 11, 12, 13, 14, //
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 99, 99, 99, 99, 99, //
    99, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 99, 99, 99, 99, 99};
} // namespace internal

inline const struct {
  static constexpr std::string encode(const void* Data, size_t Size) {
    const auto size = Size;
    const auto input_bits = size * 8;
    const auto blocks = (input_bits + 5) / 6;
    const auto output_bytes = (blocks + 3) / 4 * 4;
    std::string result;
    result.resize(output_bytes, '=');
    const char* const input = reinterpret_cast<const char*>(Data);
    size_t i{};
    auto out = result.data();
    for (; i + 3 <= size; i += 3) {
      *out++ = internal::encode_table[input[i] >> 2];
      *out++ = internal::encode_table[((input[i] & 3) << 4) | (input[i + 1] >> 4)];
      *out++ = internal::encode_table[((input[i + 1] & 15) << 2) | (input[i + 2] >> 6)];
      *out++ = internal::encode_table[input[i + 2] & 63];
    }
    if (i + 1 == size) {
      *out++ = internal::encode_table[input[i] >> 2];
      *out++ = internal::encode_table[(input[i] & 3) << 4];
    } else if (i + 2 == size) {
      *out++ = internal::encode_table[input[i] >> 2];
      *out++ = internal::encode_table[(input[i] & 3) << 4 | (input[i + 1] >> 4)];
      *out++ = internal::encode_table[(input[i + 1] & 15) << 2];
    }
    return result;
  }

  template<std::ranges::contiguous_range R> static constexpr std::string encode(const R& Data) {
    return encode(std::data(Data), std::size(Data));
  }

  static constexpr std::string decode(stringable<char> auto&& Base64) {
    std::string_view sv(Base64);
    const size_t m = sv.size(), n = (m * 3) / 4;
    if (m % 4 != 0) throw std::runtime_error("invalid base64 string length");
    if (m == 0) return {};
    std::string result;
    result.reserve(n);
    const char* in = sv.data();
    for (const char* const end = in + m; in < end; in += 4) {
      const auto a = internal::decode_table[*in];
      if (a >= 64) throw std::runtime_error("invalid base64 character");
      const auto b = internal::decode_table[*(in + 1)];
      if (b >= 64) throw std::runtime_error("invalid base64 character");
      result.push_back((a << 2) | (b >> 4));
      const auto c = internal::decode_table[*(in + 2)];
      const auto d = internal::decode_table[*(in + 3)];
      if (c < 64) {
        result.push_back((b << 4) | (c >> 2));
        if (d >= 64) {
          if (d == 98 && in + 4 == end) break;
          if (d != 98) throw std::runtime_error("invalid base64 character");
          else throw std::runtime_error("invalid base64 padding");
        } else result.push_back((c << 6) | d);
      } else if (c == 98 && d == 98 && in + 4 == end) break;
      else if (c == 98) throw std::runtime_error("invalid base64 padding");
      else throw std::runtime_error("invalid base64 character");
    }
    return result;
  }
} base64{};

} // namespace yw
