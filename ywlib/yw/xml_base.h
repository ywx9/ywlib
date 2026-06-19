#pragma once
#include "yw/error.h"

namespace yw::errors {
inline constexpr error::kind invalid_xml_document{"invalid XML document"};
}

namespace yw::xml {

template<bool View> using string_type = select_type<View, string_view<char>, string<char>>;

enum class child_type {
  unknown,
  comment,
  pi,
  text,
  element,
};

inline constexpr auto _is_s = [](char8_t c) noexcept { return c == 0x20 || (0x9 <= c && c <= 0xD); };
inline constexpr auto _is_char = [](char8_t c) noexcept { return _is_s(c) || 0x21 <= c; };
inline constexpr auto _is_name_char = [](char8_t c) noexcept //
{ return is_alnum(c) || c == ':' || c == '_' || c == '-' || c == '.' || 0x80 <= c; };

constexpr string_view<char> _extract_whitespace(string_view<char>& rest) {
  if (rest.empty()) return {};
  auto it = std::ranges::find_if_not(rest, _is_s);
  string_view<char> result{rest.begin(), it};
  rest = string_view<char>(it, rest.end());
  return result;
}

constexpr string_view<char> _extract_equal(string_view<char>& rest) {
  auto cur = rest;
  const auto start = cur.begin();
  _extract_whitespace(cur);
  if (cur.empty() || cur.front() != '=') return {};
  cur.remove_prefix(1);
  _extract_whitespace(cur);
  const auto out = string_view<char>(start, cur.begin());
  rest = cur;
  return out;
}

constexpr string_view<char> _extract_name(string_view<char>& rest) {
  if (rest.empty() || ![](auto c) { return is_alpha(c) || c == ':' || c == '_' || 0x80 <= c; }(rest.front())) return {};
  const auto it = std::ranges::find_if_not(rest.begin() + 1, rest.end(), _is_name_char);
  string_view<char> result(rest.begin(), it);
  rest = string_view<char>(it, rest.end());
  return result;
}

constexpr string_view<char> _extract_reference(string_view<char>& rest) {
  auto cur = rest;
  const char* start = cur.data();
  if (cur.size() < 3 || cur.front() != '&') return {};
  if (cur.size() >= 3 && cur[1] == '#') {
    size_t i = 2;
    if (i < cur.size() && (cur[i] == 'x' || cur[i] == 'X')) {
      const size_t digits = ++i;
      while (i < cur.size() && is_xdigit(cur[i])) ++i;
      if (i == digits) return {};
    } else {
      const size_t digits = i;
      while (i < cur.size() && is_digit(cur[i])) ++i;
      if (i == digits) return {};
    }
    if (i >= cur.size() || cur[i] != ';') return {};
    rest = cur.substr(++i);
    return string_view<char>(start, i);
  }
  if (cur.size() < 4) return {};
  cur.remove_prefix(1);
  if (auto name = _extract_name(cur); name.empty()) return {};
  if (cur.empty() || cur.front() != ';') return {};
  cur.remove_prefix(1);
  const size_t consumed = static_cast<size_t>(cur.data() - start);
  rest = cur;
  return string_view<char>(start, consumed);
}

constexpr std::unexpected<error> _unexpected_error(string_view<char> msg, const char* pos, string_view<char> doc, const source_line& sl = source_line::here()) {
  const auto offset = uint64_t(pos - doc.data());
  size_t line_count = 0;
  size_t line_start_index = 0;
  for (size_t i = 0; i < offset; ++i) {
    if (doc[i] == '\n') {
      ++line_count;
      line_start_index = i + 1;
    }
  }
  return std::unexpected(error(
    errors::invalid_xml_document,
    format(msg, " (line ", line_count + 1, ", column ", offset - line_start_index + 1, ")"), 0, offset, sl));
}
} // namespace yw::xml
