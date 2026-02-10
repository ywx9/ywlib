#pragma once
#include "ywx/core.h"

namespace yw {

namespace internal {

/// \note requires text_format is initialized
inline std::expected<std::wstring, error_trace> get_font_name(IDWriteTextFormat* tf) {
  auto n = tf->GetFontFamilyNameLength();
  std::wstring name(n, L'\0');
  if (auto hr = tf->GetFontFamilyName(name.data(), n + 1); SUCCEEDED(hr)) return name;
  else return unexpected_error(errors::operation_failed, "GetFontFamilyName failed", int32_t(hr));
}

/// \note requires text_format is initialized
inline bool set_text_alignment(IDWriteTextFormat* tf, DWRITE_TEXT_ALIGNMENT align) {
  switch (align) {
    case DWRITE_TEXT_ALIGNMENT_LEADING:
    case DWRITE_TEXT_ALIGNMENT_CENTER:
    case DWRITE_TEXT_ALIGNMENT_TRAILING:
    case DWRITE_TEXT_ALIGNMENT_JUSTIFIED: tf->SetTextAlignment(align); return {};
    default: return false;
  }
}

/// \note requires text_format is initialized
inline bool set_paragraph_alignment(IDWriteTextFormat* tf, DWRITE_PARAGRAPH_ALIGNMENT align) {
  switch (align) {
    case DWRITE_PARAGRAPH_ALIGNMENT_NEAR:
    case DWRITE_PARAGRAPH_ALIGNMENT_CENTER:
    case DWRITE_PARAGRAPH_ALIGNMENT_FAR: tf->SetParagraphAlignment(align); return {};
    default: return false;
  }
}
}

template<typename T> concept text_format_like = convertible_to<const remove_ref<T>&, IDWriteTextFormat*>;

//////////////////////////////////////// MARK: text_format

class text_format {
  comptr<IDWriteTextFormat> _text_format;

  text_format(const text_format&) = delete;
  text_format& operator=(const text_format&) = delete;

public:
  explicit operator bool() const noexcept { return _text_format.operator bool(); }
  explicit operator IDWriteTextFormat*&() & noexcept { return _text_format.get(); }
  explicit operator IDWriteTextFormat*() const& noexcept { return _text_format.get(); }

  text_format() noexcept = default;
  text_format(text_format&&) = default;
  text_format& operator=(text_format&&) = default;

  static std::expected<text_format, error_trace> create(null_terminated<wchar_t> name, float1 size,
    DWRITE_FONT_WEIGHT weight = DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE style = DWRITE_FONT_STYLE_NORMAL,
    DWRITE_FONT_STRETCH stretch = DWRITE_FONT_STRETCH_NORMAL) {
    text_format tf;
    if (auto res = dwrite.initialize(); !res) return unexpected_error(res.error());
    auto hr = dwrite.factory()->CreateTextFormat(
      name.data(), nullptr, weight, style, stretch, size.x, L"", &tf._text_format.get());
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateTextFormat failed", int32_t(hr));
    return std::move(tf);
  }

  static std::expected<text_format, error_trace> create(const text_format& source) {
    auto name = source.font_name();
    if (!name) return unexpected_error(name.error());
    const auto size = source._text_format->GetFontSize();
    const auto weight = source._text_format->GetFontWeight();
    const auto style = source._text_format->GetFontStyle();
    const auto stretch = source._text_format->GetFontStretch();
    return create(*name, size, weight, style, stretch);
  }

  std::expected<std::wstring, error_trace> font_name() const {
    if (!_text_format) return unexpected_error(errors::not_initialized, "text_format is not initialized");
    if (auto res = internal::get_font_name(_text_format.get()); res) return std::move(*res);
    else return unexpected_error(res.error());
  }

  std::expected<float, error_trace> font_size() const {
    if (!_text_format) return unexpected_error(errors::not_initialized, "text_format is not initialized");
    return _text_format->GetFontSize();
  }

  std::expected<DWRITE_FONT_WEIGHT, error_trace> font_weight() const {
    if (!_text_format) return unexpected_error(errors::not_initialized, "text_format is not initialized");
    return _text_format->GetFontWeight();
  }

  std::expected<DWRITE_FONT_STYLE, error_trace> font_style() const {
    if (!_text_format) return unexpected_error(errors::not_initialized, "text_format is not initialized");
    return _text_format->GetFontStyle();
  }

  std::expected<DWRITE_FONT_STRETCH, error_trace> font_stretch() const {
    if (!_text_format) return unexpected_error(errors::not_initialized, "text_format is not initialized");
    return _text_format->GetFontStretch();
  }

  std::expected<DWRITE_TEXT_ALIGNMENT, error_trace> text_alignment() const {
    if (!_text_format) return unexpected_error(errors::not_initialized, "text_format is not initialized");
    return _text_format->GetTextAlignment();
  }
  std::expected<void, error_trace> text_alignment(DWRITE_TEXT_ALIGNMENT align) {
    if (!_text_format) return unexpected_error(errors::not_initialized, "text_format is not initialized");
    if (internal::set_text_alignment(_text_format.get(), align)) return {};
    else return unexpected_error(errors::invalid_argument, "invalid text alignment value");
  }

  std::expected<DWRITE_PARAGRAPH_ALIGNMENT, error_trace> paragraph_alignment() const {
    if (!_text_format) return unexpected_error(errors::not_initialized, "text_format is not initialized");
    return _text_format->GetParagraphAlignment();
  }
  std::expected<void, error_trace> paragraph_alignment(DWRITE_PARAGRAPH_ALIGNMENT align) {
    if (!_text_format) return unexpected_error(errors::not_initialized, "text_format is not initialized");
    if (internal::set_paragraph_alignment(_text_format.get(), align)) return {};
    else return unexpected_error(errors::invalid_argument, "invalid paragraph alignment value");
  }
};

inline std::expected<void, error_trace> draw_text(
  float2 pos, float2 size, stringable<wchar_t> auto&& text, const text_format& tf, const color& c = colors::black) {
  if (auto res = dwrite.initialize(); !res) return unexpected_error(res.error());
  if (!drawing::d2d_drawing()) return unexpected_error(errors::invalid_operation, "drawing not begun");
  d2d.solid_brush()->SetColor((const D2D1_COLOR_F*)&c);
  auto sv = std::wstring_view(text);
  d2d.context()->DrawTextW(sv.data(), static_cast<UINT>(sv.size()), static_cast<IDWriteTextFormat*>(tf),
    D2D1_RECT_F(pos.x, pos.y, pos.x + size.x, pos.y + size.y), d2d.solid_brush());
  return {};
}

///////////////////////////////////////// MARK: text_layout

class text_layout {
  comptr<IDWriteTextLayout> _text_layout;

  text_layout(const text_layout&) = delete;
  text_layout& operator=(const text_layout&) = delete;

public:
  explicit operator bool() const noexcept { return _text_layout.operator bool(); }
  explicit operator IDWriteTextLayout*&() & noexcept { return _text_layout.get(); }
  explicit operator IDWriteTextLayout*() const& noexcept { return _text_layout.get(); }
  explicit operator IDWriteTextFormat*() const& noexcept { return static_cast<IDWriteTextFormat*>(_text_layout.get()); }

  text_layout() noexcept = default;
  text_layout(text_layout&&) = default;
  text_layout& operator=(text_layout&&) = default;

  /// creates text layout
  template<castable_to<IDWriteTextFormat*> T>
  static std::expected<text_layout, error_trace> create(stringable<wchar_t> auto&& text, T&& text_format, float2 size) {
    if (auto res = dwrite.initialize(); !res) return unexpected_error(res.error());
    auto sv = std::wstring_view(text);
    auto tf = static_cast<IDWriteTextFormat*>(text_format);
    if (!tf) return unexpected_error(errors::not_initialized, "text_format is not initialized");
    text_layout tl;
    auto hr =
      dwrite.factory()->CreateTextLayout(sv.data(), UINT(sv.size()), tf, size.x, size.y, &tl._text_layout.get());
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateTextLayout failed", int32_t(hr));
    return std::move(tl);
  }

  /// creates text layout from another text layout
  template<castable_to<IDWriteTextLayout*> T>
  static std::expected<text_layout, error_trace> create(stringable<wchar_t> auto&& text, T&& source) {
    if (auto res = dwrite.initialize(); !res) return unexpected_error(res.error());
    auto sv = std::wstring_view(text);
    auto tf = static_cast<IDWriteTextFormat*>(static_cast<IDWriteTextLayout*>(source));
    if (!tf) return unexpected_error(errors::not_initialized, "source text_layout is not initialized");
    text_layout tl;
    auto hr = dwrite.factory()->CreateTextLayout(sv.data(), UINT(sv.size()), tf, source._text_layout->GetMaxWidth(),
      source._text_layout->GetMaxHeight(), &tl._text_layout.get());
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateTextLayout failed", int32_t(hr));
    return std::move(tl);
  }

  /// returns position and size `{left, top, width, height}` of the character at the specified text position
  std::expected<float4, error_trace> hit_test(uint1 text_position, bool is_trailing = false) const {
    if (!_text_layout) return unexpected_error(errors::not_initialized, "text_layout is not initialized");
    DWRITE_HIT_TEST_METRICS metrics{};
    float x = 0.0f, y = 0.0f;
    auto hr = _text_layout->HitTestTextPosition(text_position.x, is_trailing, &x, &y, &metrics);
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "HitTestTextPosition failed", int32_t(hr));
    return float4(metrics.left, metrics.top, metrics.width, metrics.height);
  }

  /// returns text position at the specified point
  std::expected<uint32_t, error_trace> hit_test(float2 point) const {
    if (!_text_layout) return unexpected_error(errors::not_initialized, "text_layout is not initialized");
    DWRITE_HIT_TEST_METRICS metrics{};
    BOOL is_inside = FALSE, is_trailing = FALSE;
    auto hr = _text_layout->HitTestPoint(point.x, point.y, &is_inside, &is_trailing, &metrics);
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "HitTestPoint failed", int32_t(hr));
    return metrics.textPosition + uint32_t(is_trailing);
  }

  /// returns the size of the text layout
  std::expected<float2, error_trace> layout_size() const {
    if (!_text_layout) return unexpected_error(errors::not_initialized, "text_layout is not initialized");
    return float2(_text_layout->GetMaxWidth(), _text_layout->GetMaxHeight());
  }

  std::expected<std::wstring, error_trace> font_name() const {
    if (!_text_layout) return unexpected_error(errors::not_initialized, "text_layout is not initialized");
    if (auto res = internal::get_font_name(_text_layout.get()); res) return std::move(*res);
    else return unexpected_error(res.error());
  }

  std::expected<float, error_trace> font_size() const {
    if (!_text_layout) return unexpected_error(errors::not_initialized, "text_layout is not initialized");
    return static_cast<IDWriteTextFormat*>(_text_layout.get())->GetFontSize();
  }

  std::expected<DWRITE_FONT_WEIGHT, error_trace> font_weight() const {
    if (!_text_layout) return unexpected_error(errors::not_initialized, "text_layout is not initialized");
    return static_cast<IDWriteTextFormat*>(_text_layout.get())->GetFontWeight();
  }

  std::expected<DWRITE_FONT_STYLE, error_trace> font_style() const {
    if (!_text_layout) return unexpected_error(errors::not_initialized, "text_layout is not initialized");
    return static_cast<IDWriteTextFormat*>(_text_layout.get())->GetFontStyle();
  }

  std::expected<DWRITE_FONT_STRETCH, error_trace> font_stretch() const {
    if (!_text_layout) return unexpected_error(errors::not_initialized, "text_layout is not initialized");
    return static_cast<IDWriteTextFormat*>(_text_layout.get())->GetFontStretch();
  }

  std::expected<DWRITE_TEXT_ALIGNMENT, error_trace> text_alignment() const {
    if (!_text_layout) return unexpected_error(errors::not_initialized, "text_layout is not initialized");
    return _text_layout->GetTextAlignment();
  }
  std::expected<void, error_trace> text_alignment(DWRITE_TEXT_ALIGNMENT align) {
    if (!_text_layout) return unexpected_error(errors::not_initialized, "text_layout is not initialized");
    if (internal::set_text_alignment(_text_layout.get(), align)) return {};
    else return unexpected_error(errors::invalid_argument, "invalid text alignment value");
  }

  std::expected<DWRITE_PARAGRAPH_ALIGNMENT, error_trace> paragraph_alignment() const {
    if (!_text_layout) return unexpected_error(errors::not_initialized, "text_layout is not initialized");
    return _text_layout->GetParagraphAlignment();
  }
  std::expected<void, error_trace> paragraph_alignment(DWRITE_PARAGRAPH_ALIGNMENT align) {
    if (!_text_layout) return unexpected_error(errors::not_initialized, "text_layout is not initialized");
    if (internal::set_paragraph_alignment(_text_layout.get(), align)) return {};
    else return unexpected_error(errors::invalid_argument, "invalid paragraph alignment value");
  }
};

inline std::expected<void, error_trace> draw_text(float2 pos, const text_layout& tl, const color& c = colors::black) {
  if (auto res = dwrite.initialize(); !res) return unexpected_error(res.error());
  if (!drawing::d2d_drawing()) return unexpected_error(errors::invalid_operation, "drawing not begun");
  d2d.solid_brush()->SetColor((const D2D1_COLOR_F*)&c);
  d2d.context()->DrawTextLayout(D2D1_POINT_2F{pos.x, pos.y}, static_cast<IDWriteTextLayout*>(tl), d2d.solid_brush());
  return {};
}
} // namespace yw
