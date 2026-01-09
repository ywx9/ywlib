#pragma once
#include "yw/reader.h"

#include <algorithm>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

namespace yw {

namespace xml {

enum class child_type {
  unknown = 0,
  comment = 1,
  pi = 2,
  text = 3,
  element = 4,
};

inline constexpr auto is_whitespace = [](char8_t c) noexcept {
  return c == 0x20 || c == 0x0A || c == 0x0D || c == 0x09;
};
inline constexpr auto is_char = [](char8_t c) noexcept { return 0x20 <= c || c == 0x9 || c == 0xA || c == 0xD; };
inline constexpr auto is_name_start_char = [](char8_t c) noexcept -> bool {
  return is_alpha(c) || c == ':' || c == '_' || 0x80 <= c;
};
inline constexpr auto is_name_char = [](char8_t c) noexcept -> bool {
  return is_alnum(c) || c == '-' || c == '.' || c == ':' || c == '_' || 0x80 <= c;
};

constexpr std::string_view extract_whitespace(std::string_view& rest) {
  if (rest.empty()) return {};
  auto it = std::ranges::find_if_not(rest, is_whitespace);
  std::string_view result{rest.begin(), it};
  rest = std::string_view(it, rest.end());
  return result;
}

constexpr std::string_view extract_equal(std::string_view& rest) {
  const auto start = rest.begin();
  extract_whitespace(rest);
  if (rest.empty() || rest.front() != '=') return {};
  rest.remove_prefix(1);
  extract_whitespace(rest);
  return {start, rest.begin()};
}

constexpr std::string_view extract_name(std::string_view& rest) {
  if (rest.empty() || !is_name_start_char(rest.front())) return {};
  const auto it = std::ranges::find_if_not(rest.begin() + 1, rest.end(), is_name_char);
  std::string_view result{rest.begin(), it};
  rest = std::string_view(it, rest.end());
  return result;
}

constexpr std::string_view extract_reference(std::string_view& rest) {
  auto it = rest.begin();
  if (rest.size() < 3 || rest[0] != '&') return {};
  if (rest[1] == '#') {
    if (rest[2] == 'x') it = std::ranges::find_if_not(it + 3, rest.end(), is_xdigit);
    else it = std::ranges::find_if_not(it + 2, rest.end(), is_digit);
    if (it == rest.end() || *it != ';') return rest = std::string_view(it, rest.end()), std::string_view();
    std::string_view result{rest.begin(), ++it};
    rest = std::string_view(it, rest.end());
    return result;
  } else {
    rest.remove_prefix(1);
    if (const auto temp = extract_name(rest); temp.empty() || rest.empty() || rest.front() != ';') return {};
    rest.remove_prefix(1);
    return {it, rest.begin()};
  }
}

//////////////////////////////////////// MARK: comment

class comment {
public:
  std::string data;
  explicit operator bool() const { return !data.empty(); }
  operator std::string_view() const { return data; }
  constexpr comment() noexcept = default;
  constexpr comment(std::string_view& rest) {
    if (!rest.starts_with("<!--"sv) || rest.size() < 8) throw std::runtime_error("xml::comment: invalid comment");
    auto it = rest.begin() + 4;
    if (*it == '-') throw std::runtime_error("xml::comment: invalid comment (<!---)");
    auto it2 = std::ranges::search(std::string_view(it + 1, rest.end()), "--"sv).end();
    if (it2 == rest.end() || *it2 != '>') {
      rest = {it2, rest.end()};
      throw std::runtime_error("xml::comment: invalid comment ('--' found without closing '>')");
    }
    data = std::string_view(it, it2 - 2);
    rest = std::string_view(it2 + 1, rest.end());
  }
};

//////////////////////////////////////// MARK: text

class text {
public:
  std::string data;
  explicit operator bool() const { return !data.empty(); }
  constexpr operator std::string_view() const { return data; }
  constexpr text() noexcept = default;
  constexpr text(std::string_view& rest) {
    const auto start = rest.begin();
    while (!rest.empty()) {
      if (rest.starts_with("<![CDATA["sv)) {
        rest.remove_prefix(9);
        if (const auto sr = std::ranges::search(rest, "]]>"sv); sr.empty())
          throw std::runtime_error("xml::text: not found ]]>");
        else if (const auto fr = std::ranges::find_if_not(rest.begin(), sr.begin(), is_char); fr != sr.begin()) {
          rest = {fr, rest.end()};
          throw std::runtime_error("xml::text: invalid char in CDATA");
        } else rest = {sr.end(), rest.end()};
      } else if (rest.front() == '&') {
        if (const auto r = extract_reference(rest); r.empty()) throw std::runtime_error("xml::text: invalid reference");
      } else if (rest.front() == '<') break;
      else rest.remove_prefix(1);
    }
    data = std::string_view(start, rest.begin());
  }
};

////////////////////////////////////////// MARK: pi

class pi {
public:
  std::string target, data;
  constexpr pi() noexcept = default;
  constexpr pi(std::string_view& rest) {
    if (!rest.starts_with("<?"sv)) throw std::runtime_error("xml::pi: expected <?");
    rest.remove_prefix(2);
    auto _target = extract_name(rest);
    if (rest.empty() || _target.empty()) throw std::runtime_error("xml::pi: invalid target");
    else if (_target.size() == 3 && (_target[0] & ~0x20) == 'X' && (_target[1] & ~0x20) == 'M' &&
             (_target[2] & ~0x20) == 'L')
      throw std::runtime_error("xml::pi: target cannot be XML (case-insensitive)");
    if (const auto sr = std::ranges::search(rest, "?>"sv); sr.empty())
      throw std::runtime_error("xml::pi: not found ?>");
    else if (rest.begin() == sr.begin()) rest = {sr.end(), rest.end()};
    else if (const auto ws = extract_whitespace(rest); ws.empty())
      throw std::runtime_error("xml::pi: expected whitespace before data");
    else if (const auto fr = std::ranges::find_if_not(rest.begin(), sr.begin(), is_char); fr != sr.begin()) {
      rest = {fr, rest.end()};
      throw std::runtime_error("xml::pi: invalid char in PI data");
    } else {
      data = std::string_view(rest.begin(), sr.begin());
      rest = {sr.end(), rest.end()};
    }
    target = std::move(_target);
  }
};

//////////////////////////////////////// MARK: misc

class misc {
  std::variant<bool, comment, pi> _val;

public:
  child_type type{};
  constexpr misc() noexcept = default;
  constexpr misc(std::string_view& rest) {
    extract_whitespace(rest);
    if (rest.size() < 2 || rest.front() != '<') _val = true;
    else if (rest[1] == '!') {
      if (auto temp = xml::comment(rest); temp.data.empty()) _val = false;
      else _val = std::move(temp), type = child_type::comment;
    } else if (rest[1] == '?') {
      if (auto temp = xml::pi(rest); temp.data.empty()) _val = false;
      else _val = std::move(temp), type = child_type::pi;
    } else _val = true;
  }
  constexpr misc(comment c) : _val(std::move(c)) {}
  constexpr misc(pi p) : _val(std::move(p)) {}
  constexpr bool error() const { return _val.index() == 0 && !std::get<bool>(_val); }
  constexpr bool empty() const { return _val.index() == 0 && std::get<bool>(_val); }
  constexpr const xml::comment* comment() const {
    if (!std::holds_alternative<xml::comment>(_val)) return nullptr;
    return &std::get<xml::comment>(_val);
  }
  constexpr const xml::pi* pi() const {
    if (!std::holds_alternative<xml::pi>(_val)) return nullptr;
    return &std::get<xml::pi>(_val);
  }
};

//////////////////////////////////////// MARK: attribute

class attribute {
public:
  std::string name;
  std::string value;
  constexpr attribute() noexcept = default;
  constexpr attribute(std::string_view& rest) {
    auto name_ = extract_name(rest);
    if (name_.empty()) throw std::runtime_error("xml::attribute: invalid attribute name");
    else if (const auto _ = extract_equal(rest); _.empty() || rest.empty())
      throw std::runtime_error("xml::attribute: expected =");
    const char quote = rest.front();
    if (quote != '"' && quote != '\'') throw std::runtime_error("xml::attribute: expected quote(\" or ')");
    rest.remove_prefix(1);
    auto it = rest.begin();
    while (true) {
      if (it == rest.end() || *it == '<') {
        rest = {it, rest.end()};
        throw std::runtime_error("xml::attribute: invalid attribute value");
      } else if (*it == '&') {
        if (const auto _ = extract_reference(rest); _.empty()) {
          rest = {it, rest.end()};
          throw std::runtime_error("xml::attribute: invalid reference");
        } else it = _.end();
      } else if (*it == quote) break;
      else ++it;
    }
    auto value = std::string_view(rest.begin(), it);
    name = std::string(name_);
    rest = std::string_view(it + 1, rest.end());
  }
};

//////////////////////////////////////// MARK: element

class child;

class element {
  int start_tag(std::string_view& rest) {
    if (rest.empty() || rest.front() != '<') throw std::runtime_error("xml::element: expected <");
    rest.remove_prefix(1);
    if (name = extract_name(rest); name.empty()) throw std::runtime_error("xml::element: invalid element name");
    while (true) {
      if (const auto ws = extract_whitespace(rest); !ws.empty()) {
        if (rest.front() == '>') return rest.remove_prefix(1), 1;
        else if (rest.starts_with("/>"sv)) return rest.remove_prefix(2), 2;
        if (auto attr = attribute(rest); attr.name.empty()) throw std::runtime_error("xml::element: invalid attribute");
        else if (std::ranges::find_if(attributes, [&](const attribute& a) noexcept { return a.name == attr.name; }) !=
                 attributes.end())
          throw std::runtime_error("xml::element: duplicate attribute name");
        else attributes.push_back(std::move(attr));
      } else {
        if (rest.front() == '>') return rest.remove_prefix(1), 1;
        else if (rest.starts_with("/>"sv)) return rest.remove_prefix(2), 2;
        else throw std::runtime_error("xml::element: expected whitespace or > or />");
      }
    }
    return 0;
  }

public:
  std::string name;
  std::vector<attribute> attributes;
  std::vector<child> children;
  element() = default;
  element(std::string_view& rest);
  const element& operator[](std::string_view ElementName) const;
};

//////////////////////////////////////// MARK: child

class child {
  std::variant<bool, xml::comment, xml::pi, xml::text, xml::element> _val;

public:
  child_type type{};
  child() noexcept = default;
  child(std::string_view& rest) : _val{false} {
    if (rest.size() < 3) throw std::runtime_error("xml::child: invalid child");
    else if (rest[0] == '<') {
      if (rest[1] == '!') {
        if (rest[2] == '-') {
          if (auto temp = xml::comment(rest); temp.data.empty())
            throw std::runtime_error("xml::child: invalid comment");
          else _val = std::move(temp), type = child_type::comment;
        } else if (rest[2] == '[') {
          if (auto temp = xml::text(rest); temp.data.empty()) throw std::runtime_error("xml::child: invalid text");
          else _val = std::move(temp), type = child_type::text;
        }
      } else if (rest[1] == '?') {
        if (auto temp = xml::pi(rest); temp.data.empty())
          throw std::runtime_error("xml::child: invalid processing instruction");
        else _val = std::move(temp), type = child_type::pi;
      } else if (rest[1] == '/') _val = true, type = child_type::unknown;
      else {
        if (auto temp = xml::element(rest); temp.name.empty()) throw std::runtime_error("xml::child: invalid element");
        else _val = std::move(temp), type = child_type::element;
      }
    } else {
      if (auto temp = xml::text(rest); temp.data.empty()) throw std::runtime_error("xml::child: invalid text");
      else _val = std::move(temp), type = child_type::text;
    }
  }
  child(comment a) : _val(std::move(a)), type(child_type::comment) {}
  child(text a) : _val(std::move(a)), type(child_type::text) {}
  child(pi a) : _val(std::move(a)), type(child_type::pi) {}
  child(element a) : _val(std::move(a)), type(child_type::element) {}
  bool error() const { return _val.index() == 0 && !std::get<bool>(_val); }
  bool empty() const { return _val.index() == 0 && std::get<bool>(_val); }
  const xml::comment& comment() const {
    if (!std::holds_alternative<xml::comment>(_val)) return {};
    return std::get<xml::comment>(_val);
  }
  const xml::text& text() const {
    if (!std::holds_alternative<xml::text>(_val)) return {};
    return std::get<xml::text>(_val);
  }
  const xml::pi& pi() const {
    if (!std::holds_alternative<xml::pi>(_val)) return {};
    return std::get<xml::pi>(_val);
  }
  const xml::element& element() const {
    if (!std::holds_alternative<xml::element>(_val)) return {};
    return std::get<xml::element>(_val);
  }
};

element::element(std::string_view& rest) {
  if (const int result = start_tag(rest); result == 2) return;
  else if (result == 0) throw std::runtime_error("xml::element: invalid start tag");
  while (true) {
    if (auto temp = child(rest); temp.error()) throw std::runtime_error("xml::element: invalid child");
    else if (temp.empty()) break;
    else children.push_back(std::move(temp));
  }
  if (rest.starts_with("</"sv)) {
    if (rest.remove_prefix(2); rest.starts_with(name)) {
      rest.remove_prefix(name.size());
      extract_whitespace(rest);
      if (rest.empty() || rest.front() != '>') throw std::runtime_error("xml::element: expected > in end tag");
      else rest.remove_prefix(1);
    }
  } else throw std::runtime_error("xml::element: invalid end tag");
}

const element& element::operator[](std::string_view ElementName) const {
  for (const auto& child : children) {
    if (child.type == child_type::element)
      if (const auto& e = child.element(); e.name == ElementName) return e;
  }
  return empty_element;
}

//////////////////////////////////////////////////////////////////////////////// MARK: DOCUMENT

class document {
  static constexpr auto is_enc_start = [](char c) noexcept { return is_alpha(c); };
  static constexpr auto is_enc{[](char c) noexcept { return is_alnum(c) || c == '-' || c == '_' || c == '.'; }};
  double extract_version(std::string_view& rest) {
    try {
      // if (!rest.starts_with("version"sv)) return 0.0;
      rest.remove_prefix(7);
      if (const auto eq = extract_equal(rest); eq.empty() || rest.size() < 4) return 0.0;
      const auto quote = rest.front();
      if (quote != '"' && quote != '\'') throw "expected quote(\" or ')";
      if (rest[1] != '1' || rest[2] != '.' || !is_digit(rest[3])) throw "expected version number (1.xxx)";
      for (auto it = rest.begin() + 4;; ++it) {
        if (it == rest.end()) throw "invalid version number";
        else if (is_digit(*it)) continue;
        else if (*it != quote) throw "expected closing quote";
        auto result = yw::from_string<double>(std::string_view(rest.begin() + 1, it));
        rest = {it + 1, rest.end()};
        return result;
      }
    } catch (std::string_view e) { print("[ywlib] xml::document::version: {}", e); }
    return 0.0;
  }
  std::string_view extract_encoding(std::string_view& rest) {
    try {
      // if (!rest.starts_with("encoding"sv)) return {};
      rest.remove_prefix(8);
      if (const auto eq = extract_equal(rest); eq.empty() || rest.size() < 3) return {};
      const auto quote = rest.front();
      if (quote != '"' && quote != '\'') return {};
      if (!is_enc_start(rest[1])) return {};
      const auto it = std::ranges::find_if_not(rest.begin() + 1, rest.end(), is_enc);
      if (it == rest.end() || *it != quote) return {};
      auto result = std::string_view(rest.begin() + 1, it);
      rest = {it + 1, rest.end()};
      return result;
    } catch (std::string_view e) { print("[ywlib] xml::document::encoding: {}", e); }
    return {};
  }
  int extract_xml_decl(std::string_view& rest) {
    // if (!rest.starts_with("<?xml"sv)) return 2; // no xml decl
    rest.remove_prefix(5);
    extract_whitespace(rest);
    if (rest.starts_with("version")) version = extract_version(rest);
    extract_whitespace(rest);
    if (rest.starts_with("encoding")) encoding = extract_encoding(rest);
    const auto sr = std::ranges::search(rest, "?>"sv);
    if (sr.empty()) return 0; // error
    rest = {sr.end(), rest.end()};
    return 1; // xml decl is valid
  }

public:
  double version{1.0};
  str encoding;
  array<misc> miscs;
  element root;
  array<misc> end_miscs;
  document() : version{} {}
  document(std::string_view& XMLDocument);
};

document::document(std::string_view& XMLDocument) {
  if (XMLDocument.starts_with("<?xml"sv)) { // XML宣言があるなら
    if (const bool res = extract_xml_decl(XMLDocument); res == 0) {
      version = 0.0;
      encoding.clear();
    }
  }
  // analyzes miscs
  while (true) {
    auto temp = misc(XMLDocument);
    if (temp.error()) {
      version = 0.0;
      encoding.clear();
      miscs.clear();
      return;
    } else if (temp.empty()) break;
    else miscs.push_back(std::move(temp));
  }
  // analyzes root-element
  root = element(XMLDocument);
  if (root.name.empty()) {
    version = 0.0;
    encoding.clear();
    miscs.clear();
    return;
  }
  // analyzes miscs
  while (true) {
    auto temp = misc(XMLDocument);
    if (temp.error()) {
      version = 0.0;
      encoding.clear();
      miscs.clear();
      root = {};
      end_miscs.clear();
      return;
    } else if (temp.empty()) break;
    else end_miscs.push_back(std::move(temp));
  }
}

template<stringable<char> S> inline std::expected<document, str> from_string(S&& xml_doc) {
  const auto original = std::string_view(xml_doc);
  auto rest = original;
  auto doc = document(rest);
  if (doc.version < 1.0) {
    auto fore_part = std::string_view(original.begin(), rest.begin());
    const auto line = std::ranges::count(original.begin(), rest.begin(), '\n') + 1;
    fore_part = fore_part.substr([&](size_t s) noexcept { return s == npos ? 0 : s; }(fore_part.find_last_of('\n')));
    fore_part = fore_part.substr(fore_part.size() < 20 ? 0 : fore_part.size() - 20);
    auto back_part = rest.substr(0, [&](size_t s) noexcept { return s == npos ? 0 : s; }(rest.find_first_of('\n')));
    back_part = back_part.substr(0, back_part.size() < 20 ? back_part.size() : 20);
    auto here = str(fore_part.size(), ' ') + '^';
    return std::unexpected(
      format("[ywlib] yw::xml::from_string: error at line {}\n{}{}\n{}", line, fore_part, back_part, here));
  } else return doc;
}

template<stringable S> requires different_from<iter_value_t<S>, char>
std::expected<document, str> from_string(S&& xml_doc) {
  str temp = unicode<char>(static_cast<S&&>(xml_doc));
  return from_string(temp);
}

template<path_like T> std::expected<document, str> from_file(T&& xml_file) {
  decltype(auto) p = to_path(static_cast<T&&>(xml_file));
  size_t file_size{};
  if (auto size_ = file::size(p); size_.has_value()) file_size = *size_;
  else return std::unexpected(format("[ywlib] xml::from_file: failed to get file size: {}", unicode<char>(p.native())));
  auto reader = file::open_to_read(p);
  auto doc = reader();
  reader.close();
  if (doc.size() != file_size)
    return std::unexpected(format("[ywlib] yw::xml::from_file: failed to read file: {}", unicode<char>(p.native())));
  return from_string(std::move(doc));
}
} // namespace xml
} // namespace yw
