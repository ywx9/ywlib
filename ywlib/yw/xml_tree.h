#pragma once
#include "yw/xml_node.h"

namespace yw::xml {

/////////////////////////////////////// MARK: misc

template<bool View> class misc {
public:
  using value_type = std::variant<std::monostate, comment<View>, pi<View>>;

private:
  value_type _value{};

public:
  constexpr misc() noexcept = default;
  constexpr misc(comment<View> c) noexcept : _value(std::move(c)) {}
  constexpr misc(pi<View> p) noexcept : _value(std::move(p)) {}

  /// checks if this misc is empty or valid
  explicit constexpr operator bool() const noexcept { return is_empty() || is_valid(); }

  /// checks if this misc is empty
  constexpr bool is_empty() const noexcept {
    if (std::holds_alternative<std::monostate>(_value)) return true;
    else if (auto p = std::get_if<comment<View>>(&_value); p) return p->is_empty(p->content);
    else if (auto p = std::get_if<pi<View>>(&_value); p) return p->is_empty(p->target, p->data);
    else return false;
  }

  /// checks if this misc is valid. If empty, this returns false.
  constexpr bool is_valid() const noexcept {
    if (auto p = std::get_if<comment<View>>(&_value); p) return p->is_valid();
    else if (auto p = std::get_if<pi<View>>(&_value); p) return p->is_valid();
    else return false;
  }

  constexpr const value_type& value() const noexcept { return _value; }
  constexpr value_type& value() noexcept { return _value; }

  constexpr child_type type() const noexcept {
    if (std::holds_alternative<comment<View>>(_value)) return child_type::comment;
    else if (std::holds_alternative<pi<View>>(_value)) return child_type::pi;
    else return child_type::unknown;
  }

  template<typename T> constexpr bool holds() const noexcept { return std::holds_alternative<T>(_value); }

  template<typename T> constexpr T& get() { return std::get<T>(_value); }
  template<typename T> constexpr const T& get() const { return std::get<T>(_value); }

  template<typename T> constexpr T* get_if() noexcept { return std::get_if<T>(&_value); }
  template<typename T> constexpr const T* get_if() const noexcept { return std::get_if<T>(&_value); }

  constexpr comment<View>& get_comment() { return get<comment<View>>(); }
  constexpr const comment<View>& get_comment() const { return get<comment<View>>(); }
  constexpr comment<View>* get_if_comment() noexcept { return get_if<comment<View>>(); }
  constexpr const comment<View>* get_if_comment() const noexcept { return get_if<comment<View>>(); }

  constexpr pi<View>& get_pi() { return get<pi<View>>(); }
  constexpr const pi<View>& get_pi() const { return get<pi<View>>(); }
  constexpr pi<View>* get_if_pi() noexcept { return get_if<pi<View>>(); }
  constexpr const pi<View>* get_if_pi() const noexcept { return get_if<pi<View>>(); }

  constexpr size_t to_string_size() const noexcept {
    if (auto p = std::get_if<comment<View>>(&_value); p) return p->to_string_size();
    else if (auto p = std::get_if<pi<View>>(&_value); p) return p->to_string_size();
    else return 0;
  }

  constexpr char* to_string_into(char* out) const noexcept {
    if (auto p = std::get_if<comment<View>>(&_value); p) return p->to_string_into(out);
    else if (auto p = std::get_if<pi<View>>(&_value); p) return p->to_string_into(out);
    else return out;
  }

  constexpr std::string to_string() const {
    if (auto p = std::get_if<comment<View>>(&_value); p) return p->to_string();
    else if (auto p = std::get_if<pi<View>>(&_value); p) return p->to_string();
    else return {};
  }

  static constexpr std::expected<misc<View>, error_trace> parse(std::string_view& rest, std::string_view doc) {
    if (rest.starts_with("<!--"sv)) {
      if (auto res = comment<View>::parse(rest, doc); res) return misc<View>(std::move(*res));
      else return yw::unexpected_error(res.error());
    } else if (rest.starts_with("<?"sv)) {
      if (auto res = pi<View>::parse(rest, doc); res) return misc<View>(std::move(*res));
      else return yw::unexpected_error(res.error());
    } else {
      return unexpected_error("xml: expected comment or processing instruction", rest.data(), doc);
    }
  }
};

//////////////////////////////////////// MARK: element

template<bool View> class child;

template<bool View> class element {
  constexpr element(std::string_view n, std::vector<attribute<View>> a, std::vector<child<View>> c) noexcept
    : name(n), attributes(std::move(a)), children(std::move(c)) {}

public:
  select_type<View, const std::string_view, std::string> name;
  std::vector<attribute<View>> attributes;
  std::vector<child<View>> children;

  constexpr element() noexcept = default;
  constexpr ~element() = default;
  constexpr element(const element&) = default;
  constexpr element(element&&) noexcept = default;
  constexpr element& operator=(const element&) = default;
  constexpr element& operator=(element&&) noexcept = default;

  /// checks if this element is empty or valid
  explicit constexpr operator bool() const noexcept { return is_empty() || is_valid(); }

  /// checks if this element is empty. In writing, empty element is ignored.
  constexpr bool is_empty() const noexcept { return name.empty() && attributes.empty() && children.empty(); }

  /// checks if this element has no non-empty child elements or text/comments/etc.
  /// In writing, such an element is emitted in empty-element form if not empty.
  constexpr bool is_empty_element() const noexcept;

  /// checks if this element is valid. If empty, this returns false.
  constexpr bool is_valid() const noexcept;

  /// returns string size returned by `to_string`
  constexpr size_t to_string_size() const noexcept;

  /// writes this element in XML element form.
  /// If this element is empty, nothing is written.
  /// Otherwise, this element is emitted as-is without validation.
  constexpr char* to_string_into(char* out) const noexcept;

  /// returns this element in XML element form.
  /// If this element is empty, returns an empty string.
  /// Otherwise, this element is emitted as-is without validation.
  constexpr std::string to_string() const {
    std::string result(to_string_size(), '\0');
    to_string_into(result.data());
    return result;
  }

  static constexpr std::expected<element, error_trace> parse(std::string_view& rest, std::string_view doc);

  constexpr bool has_attribute(std::string_view attr_name) const noexcept {
    return get_if_attribute(attr_name) != nullptr;
  }

  constexpr attribute<View>* get_if_attribute(std::string_view attr_name) noexcept {
    const auto it = std::ranges::find(attributes, attr_name, &attribute<View>::name);
    return it != attributes.end() ? std::addressof(*it) : nullptr;
  }
  constexpr const attribute<View>* get_if_attribute(std::string_view attr_name) const noexcept {
    const auto it = std::ranges::find(attributes, attr_name, &attribute<View>::name);
    return it != attributes.end() ? std::addressof(*it) : nullptr;
  }

  constexpr attribute<View>& get_attribute(std::string_view attr_name) {
    if (auto* p = get_if_attribute(attr_name); p) return *p;
    throw std::out_of_range("xml: attribute not found");
  }
  constexpr const attribute<View>& get_attribute(std::string_view attr_name) const {
    if (auto* p = get_if_attribute(attr_name); p) return *p;
    throw std::out_of_range("xml: attribute not found");
  }

  constexpr bool has_element(std::string_view element_name) const noexcept {
    return get_if_first_element(element_name) != nullptr;
  }

  constexpr element<View>* get_if_first_element(std::string_view element_name) noexcept;
  constexpr const element<View>* get_if_first_element(std::string_view element_name) const noexcept;

  constexpr element<View>& get_first_element(std::string_view element_name) {
    if (auto* p = get_if_first_element(element_name); p) return *p;
    throw std::out_of_range("xml: element not found");
  }
  constexpr const element<View>& get_first_element(std::string_view element_name) const {
    if (auto* p = get_if_first_element(element_name); p) return *p;
    throw std::out_of_range("xml: element not found");
  }

  constexpr size_t count_elements(std::string_view name) const noexcept;
};

/////////////////////////////////////// MARK: child

template<bool View> class child {
public:
  using value_type = std::variant<std::monostate, comment<View>, pi<View>, text<View>, element<View>>;

private:
  value_type _value{};

public:
  constexpr child() noexcept = default;
  constexpr child(comment<View> c) noexcept : _value(std::move(c)) {}
  constexpr child(pi<View> p) noexcept : _value(std::move(p)) {}
  constexpr child(text<View> t) noexcept : _value(std::move(t)) {}
  constexpr child(element<View> e) noexcept : _value(std::move(e)) {}

  /// checks if this child is empty or valid
  explicit constexpr operator bool() const noexcept { return is_empty() || is_valid(); }

  /// checks if this child is empty
  constexpr bool is_empty() const noexcept {
    if (std::holds_alternative<std::monostate>(_value)) return true;
    else if (auto p = std::get_if<comment<View>>(&_value); p) return p->is_empty();
    else if (auto p = std::get_if<pi<View>>(&_value); p) return p->is_empty();
    else if (auto p = std::get_if<text<View>>(&_value); p) return p->is_empty();
    else if (auto p = std::get_if<element<View>>(&_value); p) return p->is_empty();
    else return false;
  }

  /// checks if this child is valid. If empty, this returns false.
  constexpr bool is_valid() const noexcept {
    if (auto p = std::get_if<comment<View>>(&_value); p) return p->is_valid();
    else if (auto p = std::get_if<pi<View>>(&_value); p) return p->is_valid();
    else if (auto p = std::get_if<text<View>>(&_value); p) return p->is_valid();
    else if (auto p = std::get_if<element<View>>(&_value); p) return p->is_valid();
    else return false;
  }

  constexpr const value_type& value() const noexcept { return _value; }
  constexpr value_type& value() noexcept { return _value; }

  constexpr child_type type() const noexcept {
    if (std::holds_alternative<comment<View>>(_value)) return child_type::comment;
    else if (std::holds_alternative<pi<View>>(_value)) return child_type::pi;
    else if (std::holds_alternative<text<View>>(_value)) return child_type::text;
    else if (std::holds_alternative<element<View>>(_value)) return child_type::element;
    else return child_type::unknown;
  }

  template<typename T> constexpr bool holds() const noexcept { return std::holds_alternative<T>(_value); }

  template<typename T> constexpr T& get() { return std::get<T>(_value); }

  template<typename T> constexpr const T& get() const { return std::get<T>(_value); }

  template<typename T> constexpr T* get_if() noexcept { return std::get_if<T>(&_value); }

  template<typename T> constexpr const T* get_if() const noexcept { return std::get_if<T>(&_value); }

  constexpr comment<View>& get_comment() { return get<comment<View>>(); }
  constexpr const comment<View>& get_comment() const { return get<comment<View>>(); }
  constexpr comment<View>* get_if_comment() noexcept { return get_if<comment<View>>(); }
  constexpr const comment<View>* get_if_comment() const noexcept { return get_if<comment<View>>(); }

  constexpr pi<View>& get_pi() { return get<pi<View>>(); }
  constexpr const pi<View>& get_pi() const { return get<pi<View>>(); }
  constexpr pi<View>* get_if_pi() noexcept { return get_if<pi<View>>(); }
  constexpr const pi<View>* get_if_pi() const noexcept { return get_if<pi<View>>(); }

  constexpr text<View>& get_text() { return get<text<View>>(); }
  constexpr const text<View>& get_text() const { return get<text<View>>(); }
  constexpr text<View>* get_if_text() noexcept { return get_if<text<View>>(); }
  constexpr const text<View>* get_if_text() const noexcept { return get_if<text<View>>(); }

  constexpr element<View>& get_element() { return get<element<View>>(); }
  constexpr const element<View>& get_element() const { return get<element<View>>(); }
  constexpr element<View>* get_if_element() noexcept { return get_if<element<View>>(); }
  constexpr const element<View>* get_if_element() const noexcept { return get_if<element<View>>(); }

  /// returns string size returned by `to_string`
  constexpr size_t to_string_size() const noexcept {
    if (auto p = std::get_if<comment<View>>(&_value); p) return p->to_string_size();
    else if (auto p = std::get_if<pi<View>>(&_value); p) return p->to_string_size();
    else if (auto p = std::get_if<text<View>>(&_value); p) return p->to_string_size();
    else if (auto p = std::get_if<element<View>>(&_value); p) return p->to_string_size();
    else return 0;
  }

  /// writes this child as-is.
  /// If empty, nothing is written.
  /// Otherwise, the stored value is emitted as-is without validation.
  constexpr char* to_string_into(char* out) const noexcept {
    if (auto p = std::get_if<comment<View>>(&_value); p) return p->to_string_into(out);
    else if (auto p = std::get_if<pi<View>>(&_value); p) return p->to_string_into(out);
    else if (auto p = std::get_if<text<View>>(&_value); p) return p->to_string_into(out);
    else if (auto p = std::get_if<element<View>>(&_value); p) return p->to_string_into(out);
    else return out;
  }

  /// returns this child as string.
  /// If empty, returns an empty string.
  /// Otherwise, the stored value is emitted as-is without validation.
  constexpr std::string to_string() const {
    if (auto p = std::get_if<comment<View>>(&_value); p) return p->to_string();
    else if (auto p = std::get_if<pi<View>>(&_value); p) return p->to_string();
    else if (auto p = std::get_if<text<View>>(&_value); p) return p->to_string();
    else if (auto p = std::get_if<element<View>>(&_value); p) return p->to_string();
    else return {};
  }

  static constexpr std::expected<child<View>, error_trace> parse(std::string_view& rest, std::string_view doc) {
    if (rest.front() == '<') {
      if (rest[1] == '!') {
        if (rest[2] == '-') {
          if (auto res = comment<View>::parse(rest, doc)) return child<View>(std::move(*res));
          else return yw::unexpected_error(res.error());
        } else if (rest[2] == '[') {
          if (auto res = text<View>::parse(rest, doc)) return child<View>(std::move(*res));
          else return yw::unexpected_error(res.error());
        } else return unexpected_error("xml: invalid child", rest.data(), doc);
      } else if (rest[1] == '?') {
        if (auto res = pi<View>::parse(rest, doc)) return child<View>(std::move(*res));
        else return yw::unexpected_error(res.error());
      } else if (rest[1] == '/') return unexpected_error("xml: unexpected end tag", rest.data(), doc);
      else if (auto res = element<View>::parse(rest, doc)) return child<View>(std::move(*res));
      else return yw::unexpected_error(res.error());
    } else if (auto res = text<View>::parse(rest, doc)) return child<View>(std::move(*res));
    else return yw::unexpected_error(res.error());
  }
};

/////////////////////////////////////// MARK: element definitions

template<bool View> constexpr bool element<View>::is_empty_element() const noexcept {
  return std::ranges::all_of(children, [](const auto& child) { return child.is_empty(); });
}

template<bool View> constexpr bool element<View>::is_valid() const noexcept {
  if (is_empty()) return false;
  {
    auto rest = std::string_view(name);
    const auto parsed = _extract_name(rest);
    if (parsed.empty() || !rest.empty()) return false;
  }
  if (!std::ranges::all_of(attributes, [](const auto& attr) { return attr.is_empty() || attr.is_valid(); }))
    return false;

  for (auto it = attributes.begin(); it != attributes.end(); ++it) {
    if (it->is_empty()) continue;
    if (std::ranges::find_if(std::next(it), attributes.end(), [&](const auto& other) {
          return !other.is_empty() && other.name == it->name;
        }) != attributes.end())
      return false;
  }

  return std::ranges::all_of(children, [](const auto& child) { return child.is_empty() || child.is_valid(); });
}

template<bool View> constexpr size_t element<View>::to_string_size() const noexcept {
  if (is_empty()) return 0;
  size_t size = 1 + name.size(); // <name
  for (const auto& attr : attributes)
    if (!attr.is_empty()) size += 1 + attr.to_string_size(); // space + attr

  if (is_empty_element()) size += 2; // />
  else {
    size += 1; // >
    for (const auto& child : children)
      if (!child.is_empty()) size += child.to_string_size();
    size += 3 + name.size(); // </name>
  }
  return size;
}

template<bool View> constexpr char* element<View>::to_string_into(char* out) const noexcept {
  if (is_empty()) return out;
  auto it = std::ranges::copy("<"sv, out).out;
  it = std::ranges::copy(name, it).out;
  for (const auto& attr : attributes) {
    if (!attr.is_empty()) {
      *it++ = ' ';
      it = attr.to_string_into(it);
    }
  }
  if (is_empty_element()) it = std::ranges::copy("/>"sv, it).out;
  else {
    *it++ = '>';
    for (const auto& child : children)
      if (!child.is_empty()) it = child.to_string_into(it);
    it = std::ranges::copy("</"sv, it).out;
    it = std::ranges::copy(name, it).out;
    *it++ = '>';
  }
  return it;
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

template<bool View>
constexpr element<View>* element<View>::get_if_first_element(std::string_view element_name) noexcept {
  const auto it = std::ranges::find_if(children, [&](auto& child) {
    if (child.is_empty()) return false;
    if (auto* p = child.template get_if<element<View>>(); p) return p->name == element_name;
    return false;
  });
  if (it == children.end()) return nullptr;
  return it->template get_if<element<View>>();
}

template<bool View>
constexpr const element<View>* element<View>::get_if_first_element(std::string_view element_name) const noexcept {
  const auto it = std::ranges::find_if(children, [&](const auto& child) {
    if (child.is_empty()) return false;
    if (auto* p = child.template get_if<element<View>>(); p) return p->name == element_name;
    return false;
  });
  if (it == children.end()) return nullptr;
  return it->template get_if<element<View>>();
}

template<bool View> constexpr size_t element<View>::count_elements(std::string_view element_name) const noexcept {
  return std::ranges::count_if(children, [&](const auto& child) {
    if (child.is_empty()) return false;
    if (auto* p = child.template get_if<element<View>>(); p) return p->name == element_name;
    return false;
  });
}
}
