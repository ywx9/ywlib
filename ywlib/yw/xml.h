#pragma once
#include "yw/file.h"

/// \note 簡易なXMLパーサーを提供する。特に仕様と異なる点を以下に明記する。
/// - char8_tで簡易にトークン解析を行うため、以下のように定義する。
///   - 空白(S)は、`0x20 | HT | LF | VT | FF | CR`を指す。
///   - 文字(Char)は、`S | 0x21 <= c`を指す。
///   - NameStartCharは、`[A-Za-z:_] | 0x80 <= c`を指す。
///   - NameCharは、`NameStartChar | [0-9] | '-' | '.'`を指す。

namespace yw::xml {

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

constexpr std::unexpected<error_trace> unexpected_error(
  null_terminated<char> msg, const char* pos, std::string_view doc, const source& src = {}) {
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
  return yw::unexpected_error(errors::invalid_argument, null_terminated<char>(std::move(s)), 0, offset, src);
}

//////////////////////////////////////// MARK: comment

template<bool View> class comment {
  constexpr comment(std::string_view Data) noexcept : data(Data) {}

public:
  select_type<View, std::string_view, std::string> data{};
  explicit constexpr operator bool() const { return !data.empty(); }
  constexpr operator std::string_view() const { return data; }
  constexpr comment() noexcept = default;

  static constexpr std::expected<comment<View>, error_trace> parse(std::string_view& rest, std::string_view doc) {
    if (!rest.starts_with("<!--"sv)) return unexpected_error("xml: expected comment '<!--'", rest.data(), doc);
    rest.remove_prefix(4);
    if (rest.empty()) return unexpected_error("xml: unterminated comment (missing '-->')", rest.data(), doc);
    if (rest.front() == '-') return unexpected_error("xml: comment text must not start with '-'", rest.data(), doc);
    if (const size_t end = rest.find("-->"sv); end == std::string_view::npos)
      return unexpected_error("xml: unterminated comment (missing '-->')", rest.data() + rest.size(), doc);
    else if (auto text = rest.substr(0, end); text.find("--"sv) != std::string_view::npos)
      return unexpected_error("xml: comment text must not contain '--'", rest.data() + text.find("--"sv), doc);
    else return comment<View>((rest.remove_prefix(end + 3), text));
  }
};

//////////////////////////////////////// MARK: text

template<bool View> class text {
  constexpr text(std::string_view Data) noexcept : data(Data) {}

public:
  select_type<View, std::string_view, std::string> data{};
  explicit constexpr operator bool() const { return !data.empty(); }
  constexpr operator std::string_view() const { return data; }
  constexpr text() noexcept = default;

  static constexpr std::expected<text<View>, error_trace> parse(std::string_view& rest, std::string_view doc) {
    const char* start = rest.data();
    while (!rest.empty()) {
      if (rest.starts_with("<![CDATA["sv)) {
        rest.remove_prefix(9);
        if (const auto sr = std::ranges::search(rest, "]]>"sv); sr.begin() == rest.end())
          return unexpected_error("xml: unterminated CDATA section (missing ']]>')", rest.data(), doc);
        else if (const auto fr = std::ranges::find_if_not(rest.begin(), sr.begin(), _is_char); fr != sr.begin())
          return unexpected_error("xml: invalid character in CDATA section", std::to_address(fr), doc);
        else rest = std::string_view(std::to_address(sr.end()), rest.data() + rest.size() - std::to_address(sr.end()));
      } else if (rest.front() == '&') {
        const auto before = rest.data();
        if (const auto r = _extract_reference(rest); r.empty())
          return unexpected_error("xml: invalid character reference", before, doc);
      } else if (rest.front() == '<') break;
      else if (rest.starts_with("]]>"sv)) return unexpected_error("xml: ']]>' is not allowed in character data", rest.data(), doc);
      else if (!_is_char(rest.front())) return unexpected_error("xml: invalid character in text", rest.data(), doc);
      else rest.remove_prefix(1);
    }
    return text<View>(std::string_view(start, static_cast<size_t>(rest.data(); - start)));
  }
};

////////////////////////////////////////// MARK: pi

template<bool View> class pi {
  constexpr pi(std::string_view Target, std::string_view Data) noexcept : target(Target), data(Data) {}
public:
  select_type<View, std::string_view, std::string> target{};
  select_type<View, std::string_view, std::string> data{};
  explicit operator bool() const { return !target.empty(); }
  operator std::string_view() const { return target; }
  constexpr pi() noexcept = default;

  static constexpr std::expected<pi<View>, error_trace> parse(std::string_view& rest, std::string_view doc) {
    if (!rest.starts_with("<?"sv)) return unexpected_error("xml: expected processing instruction '<?'", rest.data(), doc);
    rest.remove_prefix(2);
    const char* target_pos = rest.data();
    auto target = _extract_name(rest);
    if (target.empty()) return unexpected_error("xml: invalid processing instruction target", target_pos, doc);
    if (target.size() == 3 && (target[0] | 0x20) == 'x' && (target[1] | 0x20) == 'm' && (target[2] | 0x20) == 'l')
      return unexpected_error("xml: processing instruction target must not be 'xml'", target.data(), doc);
    _extract_whitespace(rest);
    const char* data_start = rest.data();
    const auto sr = std::ranges::search(rest, "?>"sv);
    if (sr.begin() == rest.end())
      return unexpected_error("xml: unterminated processing instruction (missing '?>')", data_start, doc);
    auto data = std::string_view(data_start,static_cast<size_t>(std::to_address(sr.begin()) - data_start));
    rest = std::string_view(std::to_address(sr.end()),rest.data() + rest.size() - std::to_address(sr.end()));
    return pi<View>(target, data);
  }
};

/////////////////////////////////////// MARK: misc

template<bool View> class misc {
  using value_type = std::variant<std::monostate, comment<View>, pi<View>>;
  std::variant<std::monostate, comment<View>, pi<View>> _value{};
public:
  constexpr misc() = default;
  constexpr misc(comment<View> c) : _value(std::move(c)) {}
  constexpr misc(pi<View> p) : _value(std::move(p)) {}
  constexpr explicit operator bool() const { return _value.index() != 0; }

  constexpr const value_type& get() const { return _value; }
  constexpr value_type& get() { return _value; }

  constexpr child_type type() const {
    if (std::holds_alternative<comment<View>>(_value)) return child_type::comment;
    else if (std::holds_alternative<pi<View>>(_value)) return child_type::pi;
    else return child_type::unknown;
  }

  constexpr std::expected<comment<View>*, error_trace> as_comment() {
    if (auto p = std::get_if<comment<View>>(&_value); p) return p;
    else return yw::unexpected_error(errors::invalid_operation, "xml: misc is not a comment");
  }
  constexpr std::expected<const comment<View>*, error_trace> as_comment() const {
    if (auto p = std::get_if<comment<View>>(&_value); p) return p;
    else return yw::unexpected_error(errors::invalid_operation, "xml: misc is not a comment");
  }

  constexpr std::expected<pi<View>*, error_trace> as_pi() {
    if (auto p = std::get_if<pi<View>>(&_value); p) return p;
    else return yw::unexpected_error(errors::invalid_operation, "xml: misc is not a processing instruction");
  }
  constexpr std::expected<const pi<View>*, error_trace> as_pi() const {
    if (auto p = std::get_if<pi<View>>(&_value); p) return p;
    else return yw::unexpected_error(errors::invalid_operation, "xml: misc is not a processing instruction");
  }

  static constexpr std::expected<misc<View>, error_trace> parse(std::string_view& rest, std::string_view doc) {
    if (rest.starts_with("<!--"sv)) {
      if (auto res = comment<View>::parse(rest, doc); res) return misc<View>(std::move(*res));
      else return yw::unexpected_error(res.error());
    } else if (rest.starts_with("<?"sv)) {
      if (auto res = pi<View>::parse(rest, doc); res) return misc<View>(std::move(*res));
      else return yw::unexpected_error(res.error());
    } else return unexpected_error("xml: expected comment or processing instruction", rest.data(), doc);
  }
};

template<bool View> class attribute {
  constexpr attribute(std::string_view Name, std::string_view Value) noexcept : name(Name), value(Value) {}
public:
  select_type<View, std::string_view, std::string> name{};
  select_type<View, std::string_view, std::string> value{};
  explicit constexpr operator bool() const { return !name.empty(); }
  constexpr operator std::string_view() const { return name; }
  constexpr attribute() noexcept = default;

  static constexpr std::expected<attribute<View>, error_trace> parse(std::string_view& rest, std::string_view doc) {
    const char* name_pos = rest.data();
    auto name = _extract_name(rest);
    if (name.empty()) return unexpected_error("xml: invalid attribute name", name_pos, doc);
    if (const char* equal_pos = rest.data(); _extract_equal(rest).empty())
      return unexpected_error("xml: expected '=' after attribute name", equal_pos, doc);
    if (rest.empty()) return unexpected_error("xml: expected attribute value", rest.data(), doc);
    const char quote = rest.front();
    if (quote != '"' && quote != '\'')
      return unexpected_error("xml: expected quote (\" or ') at start of attribute value", rest.data(), doc);
    rest.remove_prefix(1);
    const char* value_start = rest.data();
    const char* p = value_start;
    const char* end = rest.data() + rest.size();
    while (true) {
      if (p == end) return unexpected_error("xml: unterminated attribute value", value_start, doc);
      else if (*p == quote) break;
      else if (*p == '&') {
        std::string_view tmp(p, static_cast<size_t>(end - p));
        if (auto r = _extract_reference(tmp); r.empty())
          return unexpected_error("xml: invalid character reference in attribute value", p, doc);
        p += static_cast<size_t>(tmp.data() - p);
      } else if (*p == '<') return unexpected_error("xml: '<' is not allowed in attribute value", p, doc);
      else if (!_is_char(*p)) return unexpected_error("xml: invalid character in attribute value", p, doc);
      else ++p;
    }
    auto value = std::string_view(value_start, static_cast<size_t>(p - value_start));
    const char* after_quote = p + 1;
    rest = std::string_view(after_quote, static_cast<size_t>(end - after_quote));
    return attribute<View>(name, value);
  }
};

//////////////////////////////////////// MARK: element

template<bool View> class child;

template<bool View> class element {
  constexpr element(std::string_view Name, std::vector<attribute<View>> Attributes, std::vector<child<View>> Children) noexcept;
public:
  select_type<View, std::string_view, std::string> name{};
  std::vector<attribute<View>> attributes{};
  std::vector<child<View>> children{};
  constexpr element() noexcept = default;
  constexpr ~element();
  constexpr element(const element&);
  constexpr element(element&&) noexcept;
  constexpr element& operator=(const element&);
  constexpr element& operator=(element&&) noexcept;

  static constexpr std::expected<element, error_trace> parse(std::string_view& rest, std::string_view doc);
};

template<bool View> class child {
  std::variant<std::monostate, comment<View>, pi<View>, text<View>, element<View>> _value{};
public:
  constexpr child() noexcept = default;
  constexpr child(comment<View> c) : _value(std::move(c)) {}
  constexpr child(pi<View> p) : _value(std::move(p)) {}
  constexpr child(text<View> t) : _value(std::move(t)) {}
  constexpr child(element<View> e) : _value(std::move(e)) {}
  constexpr explicit operator bool() const { return _value.index() != 0; }

  constexpr const auto& get() const { return _value; }
  constexpr auto& get() { return _value; }

  constexpr child_type type() const {
    if (std::holds_alternative<comment<View>>(_value)) return child_type::comment;
    else if (std::holds_alternative<pi<View>>(_value)) return child_type::pi;
    else if (std::holds_alternative<text<View>>(_value)) return child_type::text;
    else if (std::holds_alternative<element<View>>(_value)) return child_type::element;
    else return child_type::unknown;
  }

  constexpr std::expected<comment<View>*, error_trace> as_comment() {
    if (auto p = std::get_if<comment<View>>(&_value); p) return p;
    else return yw::unexpected_error(errors::invalid_operation, "xml: child is not a comment");
  }
  constexpr std::expected<const comment<View>*, error_trace> as_comment() const {
    if (auto p = std::get_if<comment<View>>(&_value); p) return p;
    else return yw::unexpected_error(errors::invalid_operation, "xml: child is not a comment");
  }

  constexpr std::expected<pi<View>*, error_trace> as_pi() {
    if (auto p = std::get_if<pi<View>>(&_value); p) return p;
    else return yw::unexpected_error(errors::invalid_operation, "xml: child is not a processing instruction");
  }
  constexpr std::expected<const pi<View>*, error_trace> as_pi() const {
    if (auto p = std::get_if<pi<View>>(&_value); p) return p;
    else return yw::unexpected_error(errors::invalid_operation, "xml: child is not a processing instruction");
  }

  constexpr std::expected<text<View>*, error_trace> as_text() {
    if (auto p = std::get_if<text<View>>(&_value); p) return p;
    else return yw::unexpected_error(errors::invalid_operation, "xml: child is not a text node");
  }
  constexpr std::expected<const text<View>*, error_trace> as_text() const {
    if (auto p = std::get_if<text<View>>(&_value); p) return p;
    else return yw::unexpected_error(errors::invalid_operation, "xml: child is not a text node");
  }

  constexpr std::expected<element<View>*, error_trace> as_element() {
    if (auto p = std::get_if<element<View>>(&_value); p) return p;
    else return yw::unexpected_error(errors::invalid_operation, "xml: child is not an element");
  }
  constexpr std::expected<const element<View>*, error_trace> as_element() const {
    if (auto p = std::get_if<element<View>>(&_value); p) return p;
    else return yw::unexpected_error(errors::invalid_operation, "xml: child is not an element");
  }

  static constexpr std::expected<child<View>, error_trace> parse(std::string_view& rest, std::string_view doc) {
    else if (rest.front() == '<') {
      if (rest[1] == '!') {
        if (rest[2] == '-') {
          if (auto res = comment<View>::parse(rest, doc); res) return child<View>(std::move(*res));
          else return yw::unexpected_error(res.error());
        } else if (rest[2] == '[') {
          if (auto res = text<View>::parse(rest, doc); res) return child<View>(std::move(*res));
          else return yw::unexpected_error(res.error());
        } else return unexpected_error("xml: invalid child", rest.data(), doc);
      } else if (rest[1] == '?') {
        if (auto res = pi<View>::parse(rest, doc); res) return child<View>(std::move(*res));
        else return yw::unexpected_error(res.error());
      } else if (rest[1] == '/') return unexpected_error("xml: unexpected end tag", rest.data(), doc);
      else {
        if (auto res = element<View>::parse(rest, doc); res) return child<View>(std::move(*res));
        else return yw::unexpected_error(res.error());
      }
    } else {
      if (auto res = text<View>::parse(rest, doc); res) return child<View>(std::move(*res));
      else return yw::unexpected_error(res.error());
    }
  }
};

template<bool View> inline element<View>::~element() = default;
template<bool View> inline element<View>::element(const element&) = default;
template<bool View> inline element<View>::element(element&&) noexcept = default;
template<bool View> inline element<View>& element<View>::operator=(const element&) = default;
template<bool View> inline element<View>& element<View>::operator=(element&&) noexcept = default;

template<bool View> inline constexpr element<View>::element(
  std::string_view Name, std::vector<attribute<View>> Attributes, std::vector<child<View>> Children) noexcept
  : name(Name), attributes(std::move(Attributes)), children(std::move(Children)) {}

template<bool View> inline std::expected<element<View>, error_trace> element<View>::parse(std::string_view& rest, std::string_view doc) {
  const char* doc_end = doc.data() + doc.size();
  if (rest.empty()) return unexpected_error("xml: unexpected end of input (expected start tag)", doc_end, doc);
  if (rest.front() != '<') return unexpected_error("xml: expected start tag '<'", rest.data(), doc);
  rest.remove_prefix(1);
  auto name = _extract_name(rest);
  if (name.empty()) return unexpected_error("xml: invalid element name", rest.data(), doc);
  std::vector<attribute<View>> attributes;
  while (true) {
    _extract_whitespace(rest);
    if (rest.empty()) return unexpected_error("xml: unterminated start tag", doc_end, doc);
    if (rest.front() == '/' || rest.front() == '>') break;
    const char* attr_pos = rest.data();
    if (auto res = attribute<View>::parse(rest, doc); res) {
      attribute<View>& attr = *res;
      if (std::ranges::find(attributes, attr.name, {}, &attribute<View>::name) != attributes.end())
        return unexpected_error("xml: duplicate attribute name", attr_pos, doc);
      attributes.push_back(std::move(attr));
    } return yw::unexpected_error(res.error());
  }
  bool is_self_closing = false;
  if (rest.starts_with("/>"sv)) is_self_closing = true, rest.remove_prefix(2);
  else if (!rest.empty() && rest.front() == '>') rest.remove_prefix(1);
  else return unexpected_error("xml: expected '>' or '/>' at end of start tag", rest.data(), doc);
  std::vector<child<View>> children;
  if (!is_self_closing) {
    while (true) {
      if (rest.empty()) return unexpected_error("xml: unexpected end of input (missing end tag)", doc_end, doc);
      if (rest.starts_with("</"sv)) break;
      if (auto res = child<View>::parse(rest, doc); res) children.push_back(std::move(*res));
      else return yw::unexpected_error(res.error());
    }
    rest.remove_prefix(2);
    if (auto end_name = _extract_name(rest); end_name != name)
      return unexpected_error("xml: end tag name does not match start tag name", rest.data(), doc);
    _extract_whitespace(rest);
    if (rest.empty() || rest.front() != '>')
      return unexpected_error("xml: expected '>' at end of end tag", rest.data(), doc);
    rest.remove_prefix(1);
  }
  return element<View>(name, std::move(attributes), std::move(children));
}
} // namespace yw::xml


// //////////////////////////////////////// MARK: element

// class child;

// class element {
//   int start_tag(std::string_view& rest) {
//     if (rest.empty() || rest.front() != '<') throw std::runtime_error("xml::element: expected <");
//     rest.remove_prefix(1);
//     if (name = extract_name(rest); name.empty()) throw std::runtime_error("xml::element: invalid element name");
//     while (true) {
//       if (const auto ws = extract_whitespace(rest); !ws.empty()) {
//         if (rest.front() == '>') return rest.remove_prefix(1), 1;
//         else if (rest.starts_with("/>"sv)) return rest.remove_prefix(2), 2;
//         if (auto attr = attribute(rest); attr.name.empty()) throw std::runtime_error("xml::element: invalid
//         attribute"); else if (std::ranges::find_if(attributes, [&](const attribute& a) noexcept { return a.name ==
//         attr.name; }) !=
//                  attributes.end())
//           throw std::runtime_error("xml::element: duplicate attribute name");
//         else attributes.push_back(std::move(attr));
//       } else {
//         if (rest.front() == '>') return rest.remove_prefix(1), 1;
//         else if (rest.starts_with("/>"sv)) return rest.remove_prefix(2), 2;
//         else throw std::runtime_error("xml::element: expected whitespace or > or />");
//       }
//     }
//     return 0;
//   }

// public:
//   std::string name;
//   std::vector<attribute> attributes;
//   std::vector<child> children;
//   element() = default;
//   element(std::string_view& rest);
//   const element* operator[](std::string_view ElementName) const;
// };

// //////////////////////////////////////// MARK: child

// class child {
//   std::variant<bool, xml::comment, xml::pi, xml::text, xml::element> _val;

// public:
//   child_type type{};
//   child() noexcept = default;
//   child(std::string_view& rest) : _val{false} {
//     if (rest.size() < 3) throw std::runtime_error("xml::child: invalid child");
//     else if (rest[0] == '<') {
//       if (rest[1] == '!') {
//         if (rest[2] == '-') {
//           if (auto temp = xml::comment(rest); temp.data.empty())
//             throw std::runtime_error("xml::child: invalid comment");
//           else _val = std::move(temp), type = child_type::comment;
//         } else if (rest[2] == '[') {
//           if (auto temp = xml::text(rest); temp.data.empty()) throw std::runtime_error("xml::child: invalid text");
//           else _val = std::move(temp), type = child_type::text;
//         }
//       } else if (rest[1] == '?') {
//         if (auto temp = xml::pi(rest); temp.data.empty())
//           throw std::runtime_error("xml::child: invalid processing instruction");
//         else _val = std::move(temp), type = child_type::pi;
//       } else if (rest[1] == '/') _val = true, type = child_type::unknown;
//       else {
//         if (auto temp = xml::element(rest); temp.name.empty()) throw std::runtime_error("xml::child: invalid
//         element"); else _val = std::move(temp), type = child_type::element;
//       }
//     } else {
//       if (auto temp = xml::text(rest); temp.data.empty()) throw std::runtime_error("xml::child: invalid text");
//       else _val = std::move(temp), type = child_type::text;
//     }
//   }
//   child(comment a) : _val(std::move(a)), type(child_type::comment) {}
//   child(text a) : _val(std::move(a)), type(child_type::text) {}
//   child(pi a) : _val(std::move(a)), type(child_type::pi) {}
//   child(element a) : _val(std::move(a)), type(child_type::element) {}
//   bool error() const { return _val.index() == 0 && !std::get<bool>(_val); }
//   bool empty() const { return _val.index() == 0 && std::get<bool>(_val); }
//   const xml::comment* comment() const {
//     if (!std::holds_alternative<xml::comment>(_val)) return nullptr;
//     return std::addressof(std::get<xml::comment>(_val));
//   }
//   const xml::text* text() const {
//     if (!std::holds_alternative<xml::text>(_val)) return nullptr;
//     return std::addressof(std::get<xml::text>(_val));
//   }
//   const xml::pi* pi() const {
//     if (!std::holds_alternative<xml::pi>(_val)) return nullptr;
//     return std::addressof(std::get<xml::pi>(_val));
//   }
//   const xml::element* element() const {
//     if (!std::holds_alternative<xml::element>(_val)) return nullptr;
//     return std::addressof(std::get<xml::element>(_val));
//   }
// };

// element::element(std::string_view& rest) {
//   if (const int result = start_tag(rest); result == 2) return;
//   else if (result == 0) throw std::runtime_error("xml::element: invalid start tag");
//   while (true) {
//     if (auto temp = child(rest); temp.error()) throw std::runtime_error("xml::element: invalid child");
//     else if (temp.empty()) break;
//     else children.push_back(std::move(temp));
//   }
//   if (rest.starts_with("</"sv)) {
//     if (rest.remove_prefix(2); rest.starts_with(name)) {
//       rest.remove_prefix(name.size());
//       extract_whitespace(rest);
//       if (rest.empty() || rest.front() != '>') throw std::runtime_error("xml::element: expected > in end tag");
//       else rest.remove_prefix(1);
//     }
//   } else throw std::runtime_error("xml::element: invalid end tag");
// }

// const element* element::operator[](std::string_view ElementName) const {
//   for (const auto& child : children) {
//     if (child.type == child_type::element)
//       if (const auto& e = child.element(); e->name == ElementName) return e;
//   }
//   return nullptr;
// }

// //////////////////////////////////////////////////////////////////////////////// MARK: DOCUMENT

// class document {
//   static constexpr auto is_enc_start = [](char c) noexcept { return is_alpha(c); };
//   static constexpr auto is_enc{[](char c) noexcept { return is_alnum(c) || c == '-' || c == '_' || c == '.'; }};
//   double extract_version(std::string_view& rest) {
//     try {
//       // if (!rest.starts_with("version"sv)) return 0.0;
//       rest.remove_prefix(7);
//       if (const auto eq = extract_equal(rest); eq.empty() || rest.size() < 4) return 0.0;
//       const auto quote = rest.front();
//       if (quote != '"' && quote != '\'') throw "expected quote(\" or ')";
//       if (rest[1] != '1' || rest[2] != '.' || !is_digit(rest[3])) throw "expected version number (1.xxx)";
//       for (auto it = rest.begin() + 4;; ++it) {
//         if (it == rest.end()) throw "invalid version number";
//         else if (is_digit(*it)) continue;
//         else if (*it != quote) throw "expected closing quote";
//         auto result = yw::stov<double>(std::string_view(rest.begin() + 1, it));
//         rest = {it + 1, rest.end()};
//         return result;
//       }
//     } catch (std::string_view e) { print("[ywlib] xml::document::version: {}", e); }
//     return 0.0;
//   }
//   std::string_view extract_encoding(std::string_view& rest) {
//     try {
//       // if (!rest.starts_with("encoding"sv)) return {};
//       rest.remove_prefix(8);
//       if (const auto eq = extract_equal(rest); eq.empty() || rest.size() < 3) return {};
//       const auto quote = rest.front();
//       if (quote != '"' && quote != '\'') return {};
//       if (!is_enc_start(rest[1])) return {};
//       const auto it = std::ranges::find_if_not(rest.begin() + 1, rest.end(), is_enc);
//       if (it == rest.end() || *it != quote) return {};
//       auto result = std::string_view(rest.begin() + 1, it);
//       rest = {it + 1, rest.end()};
//       return result;
//     } catch (std::string_view e) { print("[ywlib] xml::document::encoding: {}", e); }
//     return {};
//   }
//   int extract_xml_decl(std::string_view& rest) {
//     // if (!rest.starts_with("<?xml"sv)) return 2; // no xml decl
//     rest.remove_prefix(5);
//     extract_whitespace(rest);
//     if (rest.starts_with("version")) version = extract_version(rest);
//     extract_whitespace(rest);
//     if (rest.starts_with("encoding")) encoding = extract_encoding(rest);
//     const auto sr = std::ranges::search(rest, "?>"sv);
//     if (sr.empty()) return 0; // error
//     rest = {sr.end(), rest.end()};
//     return 1; // xml decl is valid
//   }

// public:
//   double version{1.0};
//   std::string encoding;
//   std::vector<misc> miscs;
//   element root;
//   std::vector<misc> end_miscs;
//   document() : version{} {}
//   document(std::string_view& XMLDocument);
// };

// document::document(std::string_view& XMLDocument) {
//   if (XMLDocument.starts_with("<?xml"sv)) { // XML宣言があるなら
//     if (const bool res = extract_xml_decl(XMLDocument); res == 0) {
//       version = 0.0;
//       encoding.clear();
//     }
//   }
//   // analyzes miscs
//   while (true) {
//     auto temp = misc(XMLDocument);
//     if (temp.error()) {
//       version = 0.0;
//       encoding.clear();
//       miscs.clear();
//       return;
//     } else if (temp.empty()) break;
//     else miscs.push_back(std::move(temp));
//   }
//   // analyzes root-element
//   root = element(XMLDocument);
//   if (root.name.empty()) {
//     version = 0.0;
//     encoding.clear();
//     miscs.clear();
//     return;
//   }
//   // analyzes miscs
//   while (true) {
//     auto temp = misc(XMLDocument);
//     if (temp.error()) {
//       version = 0.0;
//       encoding.clear();
//       miscs.clear();
//       root = {};
//       end_miscs.clear();
//       return;
//     } else if (temp.empty()) break;
//     else end_miscs.push_back(std::move(temp));
//   }
// }

// template<stringable<char> S> document from_string(S&& xml_doc) {
//   const auto original = std::string_view(xml_doc);
//   auto rest = original;
//   auto doc = document(rest);
//   if (doc.version < 1.0) {
//     auto fore_part = std::string_view(original.begin(), rest.begin());
//     const auto line = std::ranges::count(original.begin(), rest.begin(), '\n') + 1;
//     fore_part = fore_part.substr([&](size_t s) noexcept { return s == npos ? 0 : s; }(fore_part.find_last_of('\n')));
//     fore_part = fore_part.substr(fore_part.size() < 20 ? 0 : fore_part.size() - 20);
//     auto back_part = rest.substr(0, [&](size_t s) noexcept { return s == npos ? 0 : s; }(rest.find_first_of('\n')));
//     back_part = back_part.substr(0, back_part.size() < 20 ? back_part.size() : 20);
//     auto here = std::string(fore_part.size(), ' ') + '^';
//     // return std::unexpected(
//     //   format("[ywlib] yw::xml::from_string: error at line {}\n{}{}\n{}", line, fore_part, back_part, here));
//     return {};
//   } else return doc;
// }

// inline std::expected<document, error_trace> from_file(const std::filesystem::path& p) {
//   file_handle fh;
//   if (auto res = yw::open(p, open_mode::read_existing); !res) return unexpected_error(res.error());
//   else fh = std::move(res.value());

//   auto doc = reader.read_as_string();
//   reader.close();
//   return from_string(std::move(doc));
// }
// } // namespace yw::xml
