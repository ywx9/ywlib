#pragma once
#include <yw/file_handle.h>
#include <yw/variant.h>

namespace yw::errors {
inline constexpr error::kind invalid_json_document{"invalid JSON document"};
}

namespace yw::json {

enum class type {
  null,
  boolean,
  number,
  string,
  array,
  object,
};

constexpr bool _is_ws(char c) noexcept { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; }

constexpr void _extract_whitespace(string_view<char>& rest) noexcept {
  while (!rest.empty() && _is_ws(rest.front())) rest.remove_prefix(1);
}

constexpr uint32_t _hex_value(char c) noexcept {
  if ('0' <= c && c <= '9') return uint32_t(c - '0');
  if ('a' <= c && c <= 'f') return uint32_t(c - 'a' + 10);
  return uint32_t(c - 'A' + 10);
}

constexpr std::unexpected<error> _unexpected_error(
  string_view<char> msg, const char* pos, string_view<char> doc, const source_line& sl = here()) {
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
    errors::invalid_json_document,
    format(msg, " (line ", line_count + 1, ", column ", offset - line_start_index + 1, ")"), 0, offset, sl));
}

class data {
public:
  using string_type = string<char>;
  using array_type = std::vector<data>;
  using member_type = tuple<string_type, data>;
  using object_type = std::vector<member_type>;
  using value_type = variant<bool, double, string_type, array_type, object_type>;

private:
  value_type _value;

  static constexpr std::expected<string_type, error> _parse_string(string_view<char>& rest, string_view<char> doc) {
    if (rest.empty() || rest.front() != '"') return _unexpected_error("json: expected string", rest.data(), doc);
    rest.remove_prefix(1);
    string_type out;
    while (!rest.empty()) {
      const char c = rest.front();
      rest.remove_prefix(1);
      if (c == '"') return out;
      if (static_cast<unsigned char>(c) < 0x20) return _unexpected_error("json: control character in string", rest.data() - 1, doc);
      if (c != '\\') {
        out.push_back(c);
        continue;
      }
      if (rest.empty()) return _unexpected_error("json: unterminated escape sequence", rest.data(), doc);
      const char esc = rest.front();
      rest.remove_prefix(1);
      switch (esc) {
      case '"': out.push_back('"'); break;
      case '\\': out.push_back('\\'); break;
      case '/': out.push_back('/'); break;
      case 'b': out.push_back('\b'); break;
      case 'f': out.push_back('\f'); break;
      case 'n': out.push_back('\n'); break;
      case 'r': out.push_back('\r'); break;
      case 't': out.push_back('\t'); break;
      case 'u': {
        const char* escape_pos = rest.data() - 2;
        if (rest.size() < 4) return _unexpected_error("json: incomplete unicode escape", escape_pos, doc);
        uint32_t cp = 0;
        for (size_t i = 0; i < 4; ++i) {
          if (!is_xdigit(rest[i])) return _unexpected_error("json: invalid unicode escape", rest.data() + i, doc);
          cp = (cp << 4) | _hex_value(rest[i]);
        }
        rest.remove_prefix(4);
        if (0xD800 <= cp && cp <= 0xDBFF) {
          if (rest.size() < 6 || rest[0] != '\\' || rest[1] != 'u')
            return _unexpected_error("json: missing low surrogate", escape_pos, doc);
          uint32_t low = 0;
          for (size_t i = 2; i < 6; ++i) {
            if (!is_xdigit(rest[i])) return _unexpected_error("json: invalid low surrogate", rest.data() + i, doc);
            low = (low << 4) | _hex_value(rest[i]);
          }
          if (low < 0xDC00 || 0xDFFF < low) return _unexpected_error("json: invalid low surrogate", rest.data(), doc);
          rest.remove_prefix(6);
          cp = 0x10000 + ((cp - 0xD800) << 10) + (low - 0xDC00);
        } else if (0xDC00 <= cp && cp <= 0xDFFF) return _unexpected_error("json: unexpected low surrogate", escape_pos, doc);
        const char32_t cp32 = static_cast<char32_t>(cp);
        out.append(unicode<char>(string_view<char32_t>(&cp32, 1)));
        break;
      }
      default: return _unexpected_error("json: invalid escape sequence", rest.data() - 1, doc);
      }
    }
    return _unexpected_error("json: unterminated string", doc.data() + doc.size(), doc);
  }

  static std::expected<double, error> _parse_number(string_view<char>& rest, string_view<char> doc) {
    const char* start = rest.data();
    if (!rest.empty() && rest.front() == '-') rest.remove_prefix(1);
    if (rest.empty()) return _unexpected_error("json: invalid number", start, doc);
    if (rest.front() == '0') rest.remove_prefix(1);
    else if ('1' <= rest.front() && rest.front() <= '9') {
      do rest.remove_prefix(1);
      while (!rest.empty() && is_digit(rest.front()));
    } else return _unexpected_error("json: invalid number", rest.data(), doc);
    if (!rest.empty() && rest.front() == '.') {
      rest.remove_prefix(1);
      if (rest.empty() || !is_digit(rest.front())) return _unexpected_error("json: expected digit after decimal point", rest.data(), doc);
      do rest.remove_prefix(1);
      while (!rest.empty() && is_digit(rest.front()));
    }
    if (!rest.empty() && (rest.front() == 'e' || rest.front() == 'E')) {
      rest.remove_prefix(1);
      if (!rest.empty() && (rest.front() == '+' || rest.front() == '-')) rest.remove_prefix(1);
      if (rest.empty() || !is_digit(rest.front())) return _unexpected_error("json: expected digit in exponent", rest.data(), doc);
      do rest.remove_prefix(1);
      while (!rest.empty() && is_digit(rest.front()));
    }
    double value = 0;
    const char* end = rest.data();
    const auto res = std::from_chars(start, end, value);
    if (res.ec != std::errc() || res.ptr != end) return _unexpected_error("json: number is out of range", start, doc);
    return value;
  }

  static std::expected<data, error> _parse_value(string_view<char>& rest, string_view<char> doc) {
    _extract_whitespace(rest);
    if (rest.empty()) return _unexpected_error("json: expected value", doc.data() + doc.size(), doc);
    if (rest.starts_with("null"sv)) {
      rest.remove_prefix(4);
      return data{};
    }
    if (rest.starts_with("true"sv)) {
      rest.remove_prefix(4);
      return data(true);
    }
    if (rest.starts_with("false"sv)) {
      rest.remove_prefix(5);
      return data(false);
    }
    if (rest.front() == '"') {
      if (auto s = _parse_string(rest, doc)) return data(std::move(*s));
      else return s.error().relay();
    }
    if (rest.front() == '[') {
      rest.remove_prefix(1);
      array_type values;
      _extract_whitespace(rest);
      if (!rest.empty() && rest.front() == ']') {
        rest.remove_prefix(1);
        return data(std::move(values));
      }
      while (true) {
        if (auto value = _parse_value(rest, doc)) values.push_back(std::move(*value));
        else return value.error().relay();
        _extract_whitespace(rest);
        if (rest.empty()) return _unexpected_error("json: unterminated array", doc.data() + doc.size(), doc);
        if (rest.front() == ']') {
          rest.remove_prefix(1);
          return data(std::move(values));
        }
        if (rest.front() != ',') return _unexpected_error("json: expected ',' or ']'", rest.data(), doc);
        rest.remove_prefix(1);
      }
    }
    if (rest.front() == '{') {
      rest.remove_prefix(1);
      object_type members;
      _extract_whitespace(rest);
      if (!rest.empty() && rest.front() == '}') {
        rest.remove_prefix(1);
        return data(std::move(members));
      }
      while (true) {
        _extract_whitespace(rest);
        auto key = _parse_string(rest, doc);
        if (!key) return key.error().relay();
        _extract_whitespace(rest);
        if (rest.empty() || rest.front() != ':') return _unexpected_error("json: expected ':' after object key", rest.data(), doc);
        rest.remove_prefix(1);
        auto value = _parse_value(rest, doc);
        if (!value) return value.error().relay();
        members.push_back(member_type{std::move(*key), std::move(*value)});
        _extract_whitespace(rest);
        if (rest.empty()) return _unexpected_error("json: unterminated object", doc.data() + doc.size(), doc);
        if (rest.front() == '}') {
          rest.remove_prefix(1);
          return data(std::move(members));
        }
        if (rest.front() != ',') return _unexpected_error("json: expected ',' or '}'", rest.data(), doc);
        rest.remove_prefix(1);
      }
    }
    if (rest.front() == '-' || is_digit(rest.front())) {
      if (auto number = _parse_number(rest, doc)) return data(*number);
      else return number.error().relay();
    }
    return _unexpected_error("json: expected value", rest.data(), doc);
  }

  static constexpr size_t _escaped_string_size(string_view<char> str) noexcept {
    size_t size = 2;
    for (const char c : str) {
      switch (c) {
      case '"':
      case '\\':
      case '\b':
      case '\f':
      case '\n':
      case '\r':
      case '\t': size += 2; break;
      default: size += static_cast<unsigned char>(c) < 0x20 ? 6 : 1; break;
      }
    }
    return size;
  }

  static constexpr char* _write_hex4(char* out, unsigned char c) noexcept {
    constexpr char hex[] = "0123456789abcdef";
    *out++ = '\\';
    *out++ = 'u';
    *out++ = '0';
    *out++ = '0';
    *out++ = hex[(c >> 4) & 0xF];
    *out++ = hex[c & 0xF];
    return out;
  }

  static constexpr char* _write_escaped_string(string_view<char> str, char* out) noexcept {
    *out++ = '"';
    for (const char c : str) {
      switch (c) {
      case '"': *out++ = '\\', *out++ = '"'; break;
      case '\\': *out++ = '\\', *out++ = '\\'; break;
      case '\b': *out++ = '\\', *out++ = 'b'; break;
      case '\f': *out++ = '\\', *out++ = 'f'; break;
      case '\n': *out++ = '\\', *out++ = 'n'; break;
      case '\r': *out++ = '\\', *out++ = 'r'; break;
      case '\t': *out++ = '\\', *out++ = 't'; break;
      default:
        if (static_cast<unsigned char>(c) < 0x20) out = _write_hex4(out, static_cast<unsigned char>(c));
        else *out++ = c;
        break;
      }
    }
    *out++ = '"';
    return out;
  }

public:
  constexpr data() noexcept = default;
  constexpr data(none) noexcept {}
  constexpr data(std::nullptr_t) noexcept {}
  constexpr data(bool value) noexcept : _value(value) {}
  constexpr data(floating auto value) noexcept : _value(static_cast<double>(value)) {}
  constexpr data(integral auto value) noexcept requires(!is_bool<decltype(value)> && !char_type<decltype(value)>)
    : _value(static_cast<double>(value)) {}
  template<stringable<char> S> constexpr data(S&& value) : _value(string_type(static_cast<S&&>(value))) {}
  constexpr data(string_type value) noexcept : _value(std::move(value)) {}
  constexpr data(array_type value) noexcept : _value(std::move(value)) {}
  constexpr data(object_type value) noexcept : _value(std::move(value)) {}

  constexpr const value_type& value() const noexcept { return _value; }
  constexpr value_type& value() noexcept { return _value; }

  constexpr json::type type() const noexcept {
    switch (_value.index()) {
    case 0: return json::type::boolean;
    case 1: return json::type::number;
    case 2: return json::type::string;
    case 3: return json::type::array;
    case 4: return json::type::object;
    default: return json::type::null;
    }
  }

  constexpr bool is_null() const noexcept { return _value.empty(); }
  constexpr bool is_bool() const noexcept { return get_if<bool>() != nullptr; }
  constexpr bool is_number() const noexcept { return get_if<double>() != nullptr; }
  constexpr bool is_string() const noexcept { return get_if<string_type>() != nullptr; }
  constexpr bool is_array() const noexcept { return get_if<array_type>() != nullptr; }
  constexpr bool is_object() const noexcept { return get_if<object_type>() != nullptr; }

  template<typename T> constexpr T* get_if() noexcept { return _value.template get_if<T>(); }
  template<typename T> constexpr const T* get_if() const noexcept { return _value.template get_if<T>(); }

  constexpr bool* get_if_bool() noexcept { return get_if<bool>(); }
  constexpr const bool* get_if_bool() const noexcept { return get_if<bool>(); }
  constexpr double* get_if_number() noexcept { return get_if<double>(); }
  constexpr const double* get_if_number() const noexcept { return get_if<double>(); }
  constexpr string_type* get_if_string() noexcept { return get_if<string_type>(); }
  constexpr const string_type* get_if_string() const noexcept { return get_if<string_type>(); }
  constexpr array_type* get_if_array() noexcept { return get_if<array_type>(); }
  constexpr const array_type* get_if_array() const noexcept { return get_if<array_type>(); }
  constexpr object_type* get_if_object() noexcept { return get_if<object_type>(); }
  constexpr const object_type* get_if_object() const noexcept { return get_if<object_type>(); }

  constexpr const data* get_if(string_view<char> key) const noexcept {
    const auto* obj = get_if_object();
    if (!obj) return nullptr;
    const auto it = std::ranges::find(*obj, key, &member_type::first);
    return it == obj->end() ? nullptr : std::addressof(it->second);
  }

  constexpr data* get_if(string_view<char> key) noexcept {
    auto* obj = get_if_object();
    if (!obj) return nullptr;
    const auto it = std::ranges::find(*obj, key, &member_type::first);
    return it == obj->end() ? nullptr : std::addressof(it->second);
  }

  constexpr size_t to_string_size() const noexcept {
    if (const auto* p = get_if_bool()) return *p ? 4 : 5;
    if (const auto* p = get_if_number()) return float_to_string(*p).size();
    if (const auto* p = get_if_string()) return _escaped_string_size(*p);
    if (const auto* p = get_if_array()) {
      size_t size = 2;
      for (size_t i = 0; i < p->size(); ++i) size += (*p)[i].to_string_size() + (i != 0);
      return size;
    }
    if (const auto* p = get_if_object()) {
      size_t size = 2;
      for (size_t i = 0; i < p->size(); ++i)
        size += _escaped_string_size((*p)[i].first) + 1 + (*p)[i].second.to_string_size() + (i != 0);
      return size;
    }
    return 4;
  }

  constexpr char* to_string_into(char* out) const noexcept {
    if (const auto* p = get_if_bool()) return std::ranges::copy(*p ? "true"sv : "false"sv, out).out;
    if (const auto* p = get_if_number()) return std::ranges::copy(float_to_string(*p), out).out;
    if (const auto* p = get_if_string()) return _write_escaped_string(*p, out);
    if (const auto* p = get_if_array()) {
      *out++ = '[';
      for (size_t i = 0; i < p->size(); ++i) {
        if (i != 0) *out++ = ',';
        out = (*p)[i].to_string_into(out);
      }
      *out++ = ']';
      return out;
    }
    if (const auto* p = get_if_object()) {
      *out++ = '{';
      for (size_t i = 0; i < p->size(); ++i) {
        if (i != 0) *out++ = ',';
        out = _write_escaped_string((*p)[i].first, out);
        *out++ = ':';
        out = (*p)[i].second.to_string_into(out);
      }
      *out++ = '}';
      return out;
    }
    return std::ranges::copy("null"sv, out).out;
  }

  constexpr string_type to_string() const {
    string_type result(to_string_size(), '\0');
    to_string_into(result.data());
    return result;
  }

  static std::expected<data, error> parse(string_view<char> doc) {
    string_view<char> rest = doc;
    if (rest.size() >= 3 && char8_t(rest[0]) == 0xEF && char8_t(rest[1]) == 0xBB && char8_t(rest[2]) == 0xBF)
      rest.remove_prefix(3);
    auto value = _parse_value(rest, doc);
    if (!value) return value.error().relay();
    _extract_whitespace(rest);
    if (!rest.empty()) return _unexpected_error("json: unexpected content after value", rest.data(), doc);
    return std::move(value);
  }

  std::expected<void, error> save(stringable auto&& path, bool allow_overwrite = false) const {
    const auto mode = allow_overwrite ? file::open_mode::create_always : file::open_mode::create_new;
    auto text = to_string();
    if (auto f = file::handle::create(static_cast<decltype(path)&&>(path), mode); !f) return f.error().relay();
    else if (auto res = f->write_exact(text.data(), text.size()); !res) return res.error().relay();
    else if (auto res = f->close(); !res) return res.error().relay();
    else return {};
  }
};

inline std::expected<data, error> from_string(string_view<char> text) {
  return data::parse(text);
}

inline std::expected<data, error> parse(string_view<char> text) {
  return from_string(text);
}

inline string<char> to_string(const data& value) {
  return value.to_string();
}

inline std::expected<data, error> from_file(stringable auto&& path) {
  if (auto f = file::handle::create(static_cast<decltype(path)&&>(path), file::open_mode::read_existing); !f)
    return f.error().relay();
  else return data::parse(f->read_as_string());
}

inline std::expected<data, error> open(stringable auto&& path) {
  return from_file(static_cast<decltype(path)&&>(path));
}

inline std::expected<void, error> to_file(const data& value, stringable auto&& path, bool allow_overwrite = false) {
  return value.save(static_cast<decltype(path)&&>(path), allow_overwrite);
}

inline std::expected<void, error> save(const data& value, stringable auto&& path, bool allow_overwrite = false) {
  return to_file(value, static_cast<decltype(path)&&>(path), allow_overwrite);
}
} // namespace yw::json
