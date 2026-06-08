#pragma once
#include "yw/xml_base.h"

#ifdef ywlib_header_name
#error "ywlib_header_name already defined unexpectedly"
#endif
#define ywlib_header_name "yw/xml_node.h"

namespace yw::xml {

//////////////////////////////////////// MARK: comment

template<bool View> class comment {
  constexpr comment(std::string_view Content) noexcept : content(Content) {}

public:
  select_type<View, const std::string_view, std::string> content;

  /// checks if content is empty or valid for comment
  explicit constexpr operator bool() const noexcept { return is_empty() || is_valid(); }

  /// checks if content is empty. In writing, empty comment is ignored.
  constexpr bool is_empty() const noexcept { return content.empty(); }

  /// checks if content is valid for comment. If empty, this returns false.
  constexpr bool is_valid() const noexcept { return is_valid(content); }

  /// checks if given content is valid for comment
  static constexpr bool is_valid(std::string_view Content) noexcept {
    return !Content.empty() && Content.front() != '-' && Content.find("--"sv) == std::string_view::npos;
  }

  /// returns string size returned by `to_string`
  constexpr size_t to_string_size() const noexcept { return content.empty() ? 0 : content.size() + 7; }

  /// writes this comment as `<!--content-->`.
  /// If content is empty, nothing is written.
  /// Otherwise, content is written without validation.
  constexpr char* to_string_into(char* out) const noexcept {
    if (content.empty()) return out;
    auto it = std::ranges::copy("<!--"sv, out).out;
    it = std::ranges::copy(content, it).out;
    it = std::ranges::copy("-->"sv, it).out;
    return it;
  }

  /// writes this comment as `<!--content-->`.
  /// If content is empty, nothing is written.
  /// Otherwise, content is written without validation.
  constexpr std::string to_string() const {
    if (content.empty()) return {};
    std::string result(to_string_size(), '\0');
    to_string_into(result.data());
    return result;
  }

  static constexpr std::expected<comment<View>, error> parse(std::string_view& rest, std::string_view doc) {
    if (!std::is_constant_evaluated()) make_footprint;
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
  constexpr text(std::string_view Content) noexcept : content(Content) {}

public:
  select_type<View, const std::string_view, std::string> content;

  /// checks if content is empty or valid for text
  explicit constexpr operator bool() const noexcept { return is_empty() || is_valid(); }

  /// checks if content is empty. In writing, empty text is ignored.
  constexpr bool is_empty() const noexcept { return content.empty(); }

  /// checks if content is valid for text. If empty, this returns false.
  constexpr bool is_valid() const noexcept { return is_valid(content); }

  /// checks if given content is valid for text
  static constexpr bool is_valid(std::string_view Content) noexcept {
    std::string_view c = Content;
    while (!c.empty()) {
      if (c.starts_with("<![CDATA["sv)) {
        c.remove_prefix(9);
        const auto sr = std::ranges::search(c, "]]>"sv);
        if (sr.begin() == c.end()) return false;
        if (const auto fr = std::ranges::find_if_not(c.begin(), sr.begin(), _is_char); fr != sr.begin()) return false;
        c = std::string_view(std::to_address(sr.end()), c.data() + c.size() - std::to_address(sr.end()));
      } else if (c.front() == '&') {
        if (_extract_reference(c).empty()) return false;
      } else if (c.front() == '<') return false;
      else if (c.starts_with("]]>"sv)) return false;
      else if (!_is_char(c.front())) return false;
      else c.remove_prefix(1);
    }
    return true;
  }

  /// returns string size returned by `to_string`
  constexpr size_t to_string_size() const noexcept { return content.size(); }

  /// writes this text as-is.
  /// If content is empty, nothing is written.
  /// Otherwise, content is emitted as-is without validation.
  constexpr char* to_string_into(char* out) const noexcept { return std::ranges::copy(content, out).out; }

  /// returns this text as-is.
  /// If content is empty, returns an empty string.
  /// Otherwise, content is emitted as-is without validation.
  constexpr std::string to_string() const { return std::string(content); }

  static constexpr std::expected<text<View>, error> parse(std::string_view& rest, std::string_view doc) {
    if (!std::is_constant_evaluated()) make_footprint;
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
  select_type<View, const std::string_view, std::string> target;
  select_type<View, const std::string_view, std::string> data;

  /// checks if this processing instruction is empty or valid
  explicit constexpr operator bool() const noexcept { return is_empty() || is_valid(); }

  /// checks if given target and data are empty.
  /// In writing, empty processing instruction is ignored.
  constexpr bool is_empty() const noexcept { return target.empty() && data.empty(); }

  /// checks if this processing instruction is valid.
  /// If empty, this returns false.
  constexpr bool is_valid() const noexcept { return is_valid(target, data); }

  /// checks if given target and data are valid for processing instruction
  static constexpr bool is_valid(std::string_view target, std::string_view data) noexcept {
    if (target.empty()) return false;
    {
      auto rest = target;
      if (const auto name = _extract_name(rest); name.empty() || !rest.empty()) return false;
    }
    if (target.size() == 3 && (target[0] | 0x20) == 'x' && (target[1] | 0x20) == 'm' && (target[2] | 0x20) == 'l')
      return false;
    return std::ranges::search(data, "?>"sv).begin() == data.end();
  }

  /// returns string size returned by `to_string`
  constexpr size_t to_string_size() const noexcept {
    if (is_empty(target, data)) return 0;
    return target.size() + data.size() + 4 + !data.empty();
  }

  /// writes this processing instruction in XML PI form (`<?target data?>`).
  /// If both target and data are empty, nothing is written.
  /// Otherwise, target and data are emitted as-is without validation.
  constexpr char* to_string_into(char* out) const noexcept {
    if (is_empty(target, data)) return out;
    auto it = std::ranges::copy("<?"sv, out).out;
    it = std::ranges::copy(target, it).out;
    if (!data.empty()) {
      *it++ = ' ';
      it = std::ranges::copy(data, it).out;
    }
    it = std::ranges::copy("?>"sv, it).out;
    return it;
  }

  /// returns this processing instruction in XML PI form (`<?target data?>`).
  /// If both target and data are empty, returns an empty string.
  /// Otherwise, target and data are emitted as-is without validation.
  constexpr std::string to_string() const {
    if (is_empty(target, data)) return {};
    std::string result(to_string_size(), '\0');
    to_string_into(result.data());
    return result;
  }

  static constexpr std::expected<pi<View>, error> parse(std::string_view& rest, std::string_view doc) {
    if (!std::is_constant_evaluated()) make_footprint;
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

//////////////////////////////////////// MARK: attribute

template<bool View> class attribute {
  constexpr attribute(std::string_view Name, std::string_view Value) noexcept : name(Name), value(Value) {}

public:
  select_type<View, const std::string_view, std::string> name;
  select_type<View, const std::string_view, std::string> value;

  /// checks if this attribute is empty or valid
  explicit constexpr operator bool() const noexcept { return is_empty() || is_valid(); }

  /// checks if this attribute is empty. In writing, empty attribute is ignored.
  constexpr bool is_empty() const noexcept { return is_empty(name, value); }

  /// checks if given name and value are empty. In writing, empty attribute is ignored.
  static constexpr bool is_empty(std::string_view Name, std::string_view Value) noexcept {
    return Name.empty() && Value.empty();
  }

  /// checks if this attribute is valid. If empty, this returns false.
  constexpr bool is_valid() const noexcept { return is_valid(name, value); }

  /// checks if given name and value are valid for attribute
  static constexpr bool is_valid(std::string_view Name, std::string_view Value) noexcept {
    if (is_empty(Name, Value)) return false;
    {
      auto rest = Name;
      const auto parsed = _extract_name(rest);
      if (parsed.empty() || !rest.empty()) return false;
    }
    {
      auto rest = Value;
      while (!rest.empty()) {
        if (rest.front() == '&') {
          if (_extract_reference(rest).empty()) return false;
        } else if (rest.front() == '<') return false;
        else if (!_is_char(rest.front())) return false;
        else rest.remove_prefix(1);
      }
    }
    return true;
  }

  /// returns string size returned by `to_string`
  constexpr size_t to_string_size() const noexcept { return is_empty() ? 0 : name.size() + value.size() + 3; }

  /// writes this attribute in XML attribute form (`name="value"`).
  /// If this attribute is empty, nothing is written.
  /// Otherwise, name and value are emitted as-is without validation.
  constexpr char* to_string_into(char* out) const noexcept {
    if (is_empty()) return out;
    auto it = std::ranges::copy(name, out).out;
    it = std::ranges::copy("=\""sv, it).out;
    it = std::ranges::copy(value, it).out;
    *it++ = '"';
    return it;
  }

  /// returns this attribute in XML attribute form (`name="value"`).
  /// If this attribute is empty, returns an empty string.
  /// Otherwise, name and value are emitted as-is without validation.
  constexpr std::string to_string() const {
    if (is_empty()) return {};
    std::string result(to_string_size(), '\0');
    to_string_into(result.data());
    return result;
  }

  static constexpr std::expected<attribute<View>, error> parse(std::string_view& rest, std::string_view doc) {
    if (!std::is_constant_evaluated()) make_footprint;
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
}

#undef ywlib_header_name
