#pragma once
#include "yw/file.h"

/// \note 簡易なXMLパーサーを提供する。特に仕様と異なる点を以下に明記する。
/// - char8_tで簡易にトークン解析を行うため、以下のように定義する。
///   - 空白(S)は、`0x20 | HT | LF | VT | FF | CR`を指す。
///   - 文字(Char)は、`S | 0x21 <= c`を指す。
///   - NameStartCharは、`[A-Za-z:_] | 0x80 <= c`を指す。
///   - NameCharは、`NameStartChar | [0-9] | '-' | '.'`を指す。

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
  return yw::unexpected_error(errors::invalid_argument, std::move(s), 0, offset, src);
}

//////////////////////////////////////// MARK: comment

template<bool View> class comment {
  constexpr comment(std::string_view Content) noexcept : content(Content) {}

public:
  select_type<View, const std::string_view, std::string> content;

  /// checks if content can be written. (valid or empty)
  constexpr operator bool() const noexcept {
    return content.empty() || (content.front() != '-' && content.find("--"sv) == std::string_view::npos);
  }

  /// checks if content is empty. In writing, empty comment is ignored.
  static constexpr bool is_empty(std::string_view content) noexcept { return content.empty(); }

  /// checks if content is valid for comment or empty
  constexpr bool is_valid() const noexcept { return operator bool(); }

  /// checks if given content is valid for comment
  static constexpr bool is_valid(std::string_view Content) noexcept {
    return Content.empty() || (Content.front() != '-' && Content.find("--"sv) == std::string_view::npos);
  }

  /// returns string size returned by `to_string`
  constexpr size_t to_string_size() const noexcept { return content.empty() ? 0 : content.size() + 7; }

  /// writes comment into the provided buffer
  constexpr char* to_string_into(char* out) const noexcept {
    if (content.empty()) return out;
    auto it = std::ranges::copy("<!--"sv, out).out;
    it = std::ranges::copy(content, it).out;
    it = std::ranges::copy("-->"sv, it).out;
    return it;
  }

  /// returns comment as string
  constexpr std::string to_string() const {
    if (content.empty()) return {};
    std::string result(to_string_size(), '\0');
    to_string_into(result.data());
    return result;
  }

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
  string_type<View> data{};
  explicit constexpr operator bool() const { return !data.empty(); }
  constexpr operator std::string_view() const { return data; }
  constexpr text() noexcept = default;

  constexpr size_t to_string_size() const { return data.size(); }
  constexpr char* to_string_into(char* out) const { return std::ranges::copy(data, out).out; }
  constexpr std::string to_string() const {
    std::string result(to_string_size(), '\0');
    to_string_into(result.data());
    return result;
  }

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
      else if (rest.starts_with("]]>"sv))
        return unexpected_error("xml: ']]>' is not allowed in character data", rest.data(), doc);
      else if (!_is_char(rest.front())) return unexpected_error("xml: invalid character in text", rest.data(), doc);
      else rest.remove_prefix(1);
    }
    return text<View>(std::string_view(start, rest.data()));
  }
};

////////////////////////////////////////// MARK: pi

template<bool View> class pi {
  constexpr pi(std::string_view Target, std::string_view Data) noexcept : target(Target), data(Data) {}

public:
  string_type<View> target{};
  string_type<View> data{};
  explicit constexpr operator bool() const { return !target.empty(); }
  constexpr operator std::string_view() const { return target; }
  constexpr pi() noexcept = default;

  constexpr size_t to_string_size() const { return target.size() + data.size() + 4 + !data.empty(); }
  constexpr char* to_string_into(char* out) const {
    auto it = std::ranges::copy("<?"sv, out).out;
    it = std::ranges::copy(target, it).out;
    if (!data.empty()) *it = ' ', it = std::ranges::copy(data, it + 1).out;
    it = std::ranges::copy("?>"sv, it).out;
    return it;
  }
  constexpr std::string to_string() const {
    std::string result(to_string_size(), '\0');
    to_string_into(result.data());
    return result;
  }

  static constexpr std::expected<pi<View>, error_trace> parse(std::string_view& rest, std::string_view doc) {
    if (!rest.starts_with("<?"sv))
      return unexpected_error("xml: expected processing instruction '<?'", rest.data(), doc);
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
    auto data = std::string_view(data_start, static_cast<size_t>(std::to_address(sr.begin()) - data_start));
    rest = std::string_view(std::to_address(sr.end()), rest.data() + rest.size() - std::to_address(sr.end()));
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

  constexpr size_t to_string_size() const {
    if (auto p = std::get_if<comment<View>>(&_value); p) return p->to_string_size();
    else if (auto p = std::get_if<pi<View>>(&_value); p) return p->to_string_size();
    else return 0;
  }
  constexpr char* to_string_into(char* out) const {
    if (auto p = std::get_if<comment<View>>(&_value); p) return p->to_string_into(out);
    else if (auto p = std::get_if<pi<View>>(&_value); p) return p->to_string_into(out);
    else return out;
  }
  constexpr std::string to_string() const {
    if (auto p = std::get_if<comment<View>>(&_value); p) return p->to_string();
    else if (auto p = std::get_if<pi<View>>(&_value); p) return p->to_string();
    else return {};
  }

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
  string_type<View> name{};
  string_type<View> value{};
  explicit constexpr operator bool() const { return !name.empty(); }
  constexpr operator std::string_view() const { return name; }
  constexpr attribute() noexcept = default;

  constexpr size_t to_string_size() const { return name.size() + value.size() + 3; }
  constexpr char* to_string_into(char* out) const {
    auto it = std::ranges::copy(name, out).out;
    it = std::ranges::copy("=\""sv, it).out;
    it = std::ranges::copy(value, it).out;
    *it = '"';
    return it + 1;
  }
  constexpr std::string to_string() const {
    std::string result(to_string_size(), '\0');
    to_string_into(result.data());
    return result;
  }

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
  constexpr element(
    std::string_view Name, std::vector<attribute<View>> Attributes, std::vector<child<View>> Children) noexcept;

public:
  string_type<View> name{};
  std::vector<attribute<View>> attributes{};
  std::vector<child<View>> children{};
  constexpr element() noexcept = default;
  constexpr ~element();
  constexpr element(const element&);
  constexpr element(element&&) noexcept;
  constexpr element& operator=(const element&);
  constexpr element& operator=(element&&) noexcept;
  constexpr explicit operator bool() const { return !name.empty(); }

  constexpr size_t to_string_size() const;
  constexpr char* to_string_into(char* out) const;
  constexpr std::string to_string() const;

  static constexpr std::expected<element, error_trace> parse(std::string_view& rest, std::string_view doc);

  constexpr bool has_attribute(std::string_view attr_name) const noexcept {
    return std::ranges::any_of(attributes, [attr_name](const auto& attr) { return attr.name == attr_name; });
  }

  constexpr element* first_element() noexcept;
  constexpr const element* first_element() const noexcept;
};

/////////////////////////////////////// MARK: child

template<bool View> class child {
  std::variant<std::monostate, comment<View>, pi<View>, text<View>, element<View>> _value{};

public:
  constexpr child() noexcept = default;
  constexpr child(comment<View> c) : _value(std::move(c)) {}
  constexpr child(pi<View> p) : _value(std::move(p)) {}
  constexpr child(text<View> t) : _value(std::move(t)) {}
  constexpr child(element<View> e) : _value(std::move(e)) {}
  constexpr explicit operator bool() const { return _value.index() != 0; }

  constexpr size_t to_string_size() const {
    if (auto p = std::get_if<comment<View>>(&_value); p) return p->to_string_size();
    else if (auto p = std::get_if<pi<View>>(&_value); p) return p->to_string_size();
    else if (auto p = std::get_if<text<View>>(&_value); p) return p->to_string_size();
    else if (auto p = std::get_if<element<View>>(&_value); p) return p->to_string_size();
    else return 0;
  }
  constexpr char* to_string_into(char* out) const {
    if (auto p = std::get_if<comment<View>>(&_value); p) return p->to_string_into(out);
    else if (auto p = std::get_if<pi<View>>(&_value); p) return p->to_string_into(out);
    else if (auto p = std::get_if<text<View>>(&_value); p) return p->to_string_into(out);
    else if (auto p = std::get_if<element<View>>(&_value); p) return p->to_string_into(out);
    else return out;
  }
  constexpr std::string to_string() const {
    if (auto p = std::get_if<comment<View>>(&_value); p) return p->to_string();
    else if (auto p = std::get_if<pi<View>>(&_value); p) return p->to_string();
    else if (auto p = std::get_if<text<View>>(&_value); p) return p->to_string();
    else if (auto p = std::get_if<element<View>>(&_value); p) return p->to_string();
    else return {};
  }

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
    if (rest.front() == '<') {
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
        if (auto res = element<View>::parse(rest, doc); res) {
          return child<View>(std::move(*res));
        } else {
          return yw::unexpected_error(res.error());
        }
      }
    } else {
      if (auto res = text<View>::parse(rest, doc); res) return child<View>(std::move(*res));
      else return yw::unexpected_error(res.error());
    }
  }
};

/////////////////////////////////////// MARK: element definitions

template<bool View> constexpr element<View>::~element() = default;
template<bool View> constexpr element<View>::element(const element&) = default;
template<bool View> constexpr element<View>::element(element&&) noexcept = default;
template<bool View> constexpr element<View>& element<View>::operator=(const element&) = default;
template<bool View> constexpr element<View>& element<View>::operator=(element&&) noexcept = default;

template<bool View> constexpr element<View>::element(
  std::string_view Name, std::vector<attribute<View>> Attributes, std::vector<child<View>> Children) noexcept
  : name(Name), attributes(std::move(Attributes)), children(std::move(Children)) {}

template<bool View> constexpr size_t element<View>::to_string_size() const {
  size_t size = 2 + name.size() + 1;                                     // <name>
  for (const auto& attr : attributes) size += attr.to_string_size() + 1; // space + attr
  if (!children.empty()) {
    size += 1;                                                         // >
    for (const auto& child : children) size += child.to_string_size(); // child
    size += 3 + name.size();                                           // </name>
  } else size += 2;                                                    // />
  return size;
}
template<bool View> constexpr char* element<View>::to_string_into(char* out) const {
  auto it = std::ranges::copy("<"sv, out).out;
  it = std::ranges::copy(name, it).out;
  for (const auto& attr : attributes) *it = ' ', it = attr.to_string_into(it + 1);
  if (!children.empty()) {
    *it++ = '>';
    for (const auto& child : children) it = child.to_string_into(it);
    it = std::ranges::copy("</"sv, it).out;
    it = std::ranges::copy(name, it).out;
    *it++ = '>';
  } else it = std::ranges::copy("/>"sv, it).out;
  return it;
}
template<bool View> constexpr std::string element<View>::to_string() const {
  std::string result(to_string_size(), '\0');
  to_string_into(result.data());
  return result;
}

template<bool View> inline constexpr std::expected<element<View>, error_trace> element<View>::parse(
  std::string_view& rest, std::string_view doc) {
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
      if (std::ranges::find(attributes, attr.name, &attribute<View>::name) != attributes.end())
        return unexpected_error("xml: duplicate attribute name", attr_pos, doc);
      attributes.push_back(std::move(attr));
    } else return yw::unexpected_error(res.error());
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
    auto end_name = _extract_name(rest);
    if (end_name != name) return unexpected_error("xml: end tag name does not match start tag name", rest.data(), doc);
    _extract_whitespace(rest);
    if (rest.empty() || rest.front() != '>')
      return unexpected_error("xml: expected '>' at end of end tag", rest.data(), doc);
    rest.remove_prefix(1);
  }
  return element<View>(name, std::move(attributes), std::move(children));
}

/////////////////////////////////// MARK: document

template<bool View> class document {
  constexpr document(std::vector<misc<View>> Prolog, xml::element<View> Root, std::vector<misc<View>> Trailing) noexcept
    : prolog(std::move(Prolog)), element(std::move(Root)), trailing_misc(std::move(Trailing)) {}

public:
  string_type<View> xml_declaration;
  std::vector<misc<View>> prolog;
  xml::element<View> element;
  std::vector<misc<View>> trailing_misc;

  constexpr document() = default;

  constexpr bool has_xml_declaration() const noexcept { return !xml_declaration.empty(); }

  constexpr size_t to_string_size() const {
    size_t size = xml_declaration.size() + (has_xml_declaration() ? 1 : 0); // xml declaration + newline
    for (const auto& m : prolog) size += m.to_string_size() + 1;            // misc + newline
    size += element.to_string_size();
    for (const auto& m : trailing_misc) size += m.to_string_size() + 1; // newline + misc
    return size;
  }
  constexpr char* to_string_into(char* out) const {
    auto it = out;
    if (has_xml_declaration()) {
      it = std::ranges::copy(xml_declaration, it).out;
      *it++ = '\n';
    }
    for (const auto& m : prolog) {
      it = std::ranges::copy(m.to_string(), it).out;
      *it++ = '\n';
    }
    it = std::ranges::copy(element.to_string(), it).out;
    for (const auto& m : trailing_misc) {
      *it++ = '\n';
      it = std::ranges::copy(m.to_string(), it).out;
    }
    return it;
  }
  constexpr std::string to_string() const {
    std::string result(to_string_size(), '\0');
    to_string_into(result.data());
    return result;
  }

  static constexpr std::expected<document<View>, error_trace> parse(std::string_view doc) {
    std::string_view rest = doc;
    const char* doc_end = doc.data() + doc.size();
    if (rest.size() >= 3 && char8_t(rest[0]) == 0xEF && char8_t(rest[1]) == 0xBB && char8_t(rest[2]) == 0xBF)
      rest.remove_prefix(3);
    _extract_whitespace(rest);
    string_type<View> xml_declaration;
    if (rest.starts_with("<?xml"sv)) {
      const char* decl_begin = rest.data();
      if (rest.size() < 6 || !_is_s(rest[5])) return unexpected_error("xml: invalid xml declaration", rest.data(), doc);
      const auto sr = std::ranges::search(rest, "?>"sv);
      if (sr.begin() == rest.end())
        return unexpected_error("xml: unterminated xml declaration (missing '?>')", rest.data(), doc);
      xml_declaration = string_type<View>(decl_begin, sr.end());
      rest.remove_prefix(static_cast<size_t>(sr.end() - decl_begin));
      _extract_whitespace(rest);
    }
    std::vector<misc<View>> prolog;
    while (true) {
      if (rest.starts_with("<!--"sv) || rest.starts_with("<?"sv)) {
        if (auto m = misc<View>::parse(rest, doc); !m) return yw::unexpected_error(m.error());
        else prolog.push_back(std::move(*m));
        _extract_whitespace(rest);
      } else if (!rest.starts_with("<!DOCTYPE"sv)) break;
      else return unexpected_error("xml: DOCTYPE is not supported", rest.data(), doc);
    }
    if (rest.empty()) return unexpected_error("xml: missing root element", doc_end, doc);
    xml::element<View> element;
    if (rest.starts_with("<"sv)) {
      if (auto r = xml::element<View>::parse(rest, doc); !r) return yw::unexpected_error(r.error());
      else element = std::move(*r);
    } else return unexpected_error("xml: expected root element", rest.data(), doc);
    std::vector<misc<View>> trailing_misc;
    while (true) {
      _extract_whitespace(rest);
      if (rest.starts_with("<!--"sv) || rest.starts_with("<?"sv)) {
        if (auto m = misc<View>::parse(rest, doc); !m) return yw::unexpected_error(m.error());
        else trailing_misc.push_back(std::move(*m));
      } else break;
    }
    _extract_whitespace(rest);
    if (!rest.empty()) return unexpected_error("xml: unexpected content after root element", rest.data(), doc);
    return document<View>(std::move(prolog), std::move(element), std::move(trailing_misc));
  }
};

/// opens and parses an XML document from a file
inline std::expected<document<false>, error_trace> open(const std::filesystem::path& path) {
  if (auto fh = yw::open(path, open_mode::read_existing); !fh) return yw::unexpected_error(fh.error());
  else if (auto size = fh->file_size(); !size) return yw::unexpected_error(size.error());
  else if (std::string content(static_cast<size_t>(*size), '\0'); false) return {};
  else if (auto read = fh->read_exact(content.data(), content.size()); !read) return yw::unexpected_error(read.error());
  else return document<false>::parse(content);
}
} // namespace yw::xml
