#pragma once
#include "yw/file.h"
#include "yw/xml_base.h"
#include "yw/xml_node.h"
#include "yw/xml_tree.h"

/// \note 簡易なXMLパーサーを提供する。特に仕様と異なる点を以下に明記する。
/// - char8_tで簡易にトークン解析を行うため、以下のように定義する。
///   - 空白(S)は、`0x20 | HT | LF | VT | FF | CR`を指す。
///   - 文字(Char)は、`S | 0x21 <= c`を指す。
///   - NameStartCharは、`[A-Za-z:_] | 0x80 <= c`を指す。
///   - NameCharは、`NameStartChar | [0-9] | '-' | '.'`を指す。

/** \note en)
 *
 *
 */

namespace yw::xml {

/////////////////////////////////// MARK: document

template<bool View> class document {
  constexpr document(std::vector<misc<View>> Prolog, xml::element<View> Root, std::vector<misc<View>> Trailing) noexcept
    : prolog(std::move(Prolog)), element(std::move(Root)), trailing_misc(std::move(Trailing)) {}

public:
  static constexpr bool view = View;

  string_type<View> xml_declaration;
  std::vector<misc<View>> prolog;
  xml::element<View> element;
  std::vector<misc<View>> trailing_misc;

  constexpr document() = default;
  constexpr ~document() = default;
  constexpr document(const document&) = default;
  constexpr document(document&&) noexcept = default;
  constexpr document& operator=(const document&) = default;
  constexpr document& operator=(document&&) noexcept = default;

  /// checks if this document is valid XML document
  explicit constexpr operator bool() const noexcept { return is_valid(); }

  constexpr bool has_xml_declaration() const noexcept { return !xml_declaration.empty(); }

  /// checks if this document is empty. In writing, empty document is ignored.
  constexpr bool is_empty() const noexcept {
    return xml_declaration.empty()                                                    //
           && std::ranges::all_of(prolog, [](const auto& m) { return m.is_empty(); }) //
           && element.is_empty()                                                      //
           && std::ranges::all_of(trailing_misc, [](const auto& m) { return m.is_empty(); });
  }

  /// checks if this document is valid. If empty, this returns false.
  constexpr bool is_valid() const noexcept {
    if (is_empty()) return false;
    if (!xml_declaration.empty() && !is_valid_xml_declaration(xml_declaration)) return false;
    if (!std::ranges::all_of(prolog, [](const auto& m) { return m.is_empty() || m.is_valid(); })) return false;
    if (!element.is_valid()) return false;
    if (!std::ranges::all_of(trailing_misc, [](const auto& m) { return m.is_empty() || m.is_valid(); })) return false;
    return true;
  }

  /// checks if given content is valid for xml declaration
  static constexpr bool is_valid_xml_declaration(string_view<char> decl) noexcept {
    if (decl.empty()) return false;
    if (!decl.starts_with("<?xml"sv)) return false;
    if (decl.size() < 6 || !_is_s(decl[5])) return false;
    if (!decl.ends_with("?>"sv)) return false;
    return true;
  }

  /// returns string size returned by `to_string`
  constexpr size_t to_string_size() const noexcept {
    if (is_empty()) return 0;
    size_t size = xml_declaration.size() + (has_xml_declaration() ? 1 : 0); // xml declaration + newline
    for (const auto& m : prolog)
      if (!m.is_empty()) size += m.to_string_size() + 1; // misc + newline
    size += element.to_string_size();
    size_t trailing_count = 0;
    size_t trailing_size = 0;
    for (const auto& m : trailing_misc) {
      if (!m.is_empty()) {
        ++trailing_count;
        trailing_size += m.to_string_size();
      }
    }
    size += trailing_size + trailing_count; // newline + misc for each trailing misc
    return size;
  }

  /// writes this document in XML document form.
  /// If this document is empty, nothing is written.
  /// Otherwise, this document is emitted as-is without validation.
  constexpr char* to_string_into(char* out) const noexcept {
    if (is_empty()) return out;
    auto it = out;
    if (has_xml_declaration()) {
      it = std::ranges::copy(xml_declaration, it).out;
      *it++ = '\n';
    }
    for (const auto& m : prolog)
      if (!m.is_empty()) {
        it = m.to_string_into(it);
        *it++ = '\n';
      }
    it = element.to_string_into(it);
    for (const auto& m : trailing_misc)
      if (!m.is_empty()) {
        *it++ = '\n';
        it = m.to_string_into(it);
      }
    return it;
  }

  /// returns this document in XML document form.
  /// If this document is empty, returns an empty string.
  /// Otherwise, this document is emitted as-is without validation.
  constexpr string<char> to_string() const {
    if (is_empty()) return {};
    string<char> result(to_string_size(), '\0');
    to_string_into(result.data());
    return result;
  }

  static constexpr std::expected<document<View>, error> parse(string_view<char> doc) {
    string_view<char> rest = doc;
    const char* doc_end = doc.data() + doc.size();
    if (rest.size() >= 3 && char8_t(rest[0]) == 0xEF && char8_t(rest[1]) == 0xBB && char8_t(rest[2]) == 0xBF)
      rest.remove_prefix(3);
    _extract_whitespace(rest);
    string_type<View> xml_declaration;
    if (rest.starts_with("<?xml"sv)) {
      const char* decl_begin = rest.data();
      if (rest.size() < 6 || !_is_s(rest[5])) return _unexpected_error("xml: invalid xml declaration", rest.data(), doc);
      const auto sr = std::ranges::search(rest, "?>"sv);
      if (sr.begin() == rest.end()) return _unexpected_error("xml: unterminated xml declaration (missing '?>')", rest.data(), doc);
      xml_declaration = string_type<View>(string_view<char>(decl_begin, sr.end()));
      rest.remove_prefix(static_cast<size_t>(sr.end() - decl_begin));
      _extract_whitespace(rest);
    }
    std::vector<misc<View>> prolog;
    while (true) {
      if (rest.starts_with("<!--"sv) || rest.starts_with("<?"sv)) {
        if (auto m = misc<View>::parse(rest, doc); !m) return m.error().relay();
        else prolog.push_back(std::move(*m));
        _extract_whitespace(rest);
      } else if (!rest.starts_with("<!DOCTYPE"sv)) break;
      else return _unexpected_error("xml: DOCTYPE is not supported", rest.data(), doc);
    }
    if (rest.empty()) return _unexpected_error("xml: missing root element", doc_end, doc);
    xml::element<View> element;
    if (rest.starts_with("<"sv)) {
      if (auto r = xml::element<View>::parse(rest, doc); !r) return r.error().relay();
      else element = std::move(*r);
    } else return _unexpected_error("xml: expected root element", rest.data(), doc);
    std::vector<misc<View>> trailing_misc;
    while (true) {
      _extract_whitespace(rest);
      if (rest.starts_with("<!--"sv) || rest.starts_with("<?"sv)) {
        if (auto m = misc<View>::parse(rest, doc); !m) return m.error().relay();
        else trailing_misc.push_back(std::move(*m));
      } else break;
    }
    _extract_whitespace(rest);
    if (!rest.empty()) return _unexpected_error("xml: unexpected content after root element", rest.data(), doc);
    return document<View>(std::move(prolog), std::move(element), std::move(trailing_misc));
  }
};

/// opens and parses an XML document from a file
inline document<false> open(const std::filesystem::path& path) {
  auto fh = yw::open(path, open_mode::read_existing);
  if (!fh) return {};
  auto doc = document<false>::parse(fh.read_as_string());
  if (doc) return std::move(*doc);
  doc.error().add_footprint().fizzle_out(); // warning
  return {};
}
} // namespace yw::xml
