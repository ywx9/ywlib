#pragma once
#include "yw/core.h"

#ifdef ywlib_header_name
#error "ywlib_header_name already defined unexpectedly"
#endif
#define ywlib_header_name "yw/xml_base.h"

namespace yw::xml {

template<bool View> using string_type = select_type<View, std::string_view, std::string>;

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

constexpr std::string_view _extract_whitespace(std::string_view& rest) {
  if (rest.empty()) return {};
  auto it = std::ranges::find_if_not(rest, _is_s);
  std::string_view result{rest.begin(), it};
  rest = std::string_view(it, rest.end());
  return result;
}

constexpr std::string_view _extract_equal(std::string_view& rest) {
  auto cur = rest;
  const auto start = cur.begin();
  _extract_whitespace(cur);
  if (cur.empty() || cur.front() != '=') return {};
  cur.remove_prefix(1);
  _extract_whitespace(cur);
  const auto out = std::string_view(start, cur.begin());
  rest = cur;
  return out;
}

constexpr std::string_view _extract_name(std::string_view& rest) {
  if (rest.empty() || ![](auto c) { return is_alpha(c) || c == ':' || c == '_' || 0x80 <= c; }(rest.front())) return {};
  const auto it = std::ranges::find_if_not(rest.begin() + 1, rest.end(), _is_name_char);
  std::string_view result(rest.begin(), it);
  rest = std::string_view(it, rest.end());
  return result;
}

constexpr std::string_view _extract_reference(std::string_view& rest) {
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
    return std::string_view(start, i);
  }
  if (cur.size() < 4) return {};
  cur.remove_prefix(1);
  if (auto name = _extract_name(cur); name.empty()) return {};
  if (cur.empty() || cur.front() != ';') return {};
  cur.remove_prefix(1);
  const size_t consumed = static_cast<size_t>(cur.data() - start);
  rest = cur;
  return std::string_view(start, consumed);
}

constexpr std::unexpected<error> unexpected_error(
  null_terminated<char> msg, const char* pos, std::string_view doc) {
  if (!std::is_constant_evaluated()) make_footprint;
  const auto offset = uint64_t(pos - doc.data());
  const auto line = uint64_t(std::count(doc.data(), pos, '\n') + 1);
  size_t line_start_index = 0;
  if (offset > 0) {
    const auto head = doc.substr(0, offset);
    const size_t p = head.find_last_of('\n');
    if (p != std::string_view::npos) line_start_index = p + 1;
  }
  const auto line_start = doc.data() + line_start_index;
  const auto column = uint64_t(pos - line_start + 1);
  auto s = std::format("{} (line {}, column {})", msg, line, column);
  return unexpected_error(errors::invalid_argument, std::move(s), 0, offset);
}
}

#undef ywlib_header_name
