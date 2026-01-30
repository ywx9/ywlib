#pragma once
#include "ywx/core.h"

namespace yw {

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
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateTextFormat failed", hr);
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
    auto n = _text_format->GetFontFamilyNameLength();
    if (n == 0) return unexpected_error(errors::invalid_operation, "GetFontFamilyNameLength returned 0");
    std::wstring name(n, L'\0');
    if (auto hr = _text_format->GetFontFamilyName(name.data(), n + 1); FAILED(hr))
      return unexpected_error(errors::operation_failed, "GetFontFamilyName failed", hr);
    return name;
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
    switch (align) {
    case DWRITE_TEXT_ALIGNMENT_LEADING:
    case DWRITE_TEXT_ALIGNMENT_CENTER:
    case DWRITE_TEXT_ALIGNMENT_TRAILING:
    case DWRITE_TEXT_ALIGNMENT_JUSTIFIED: _text_format->SetTextAlignment(align); return {};
    default: return unexpected_error(errors::invalid_argument, "invalid text alignment value");
    }
  }

  std::expected<DWRITE_PARAGRAPH_ALIGNMENT, error_trace> paragraph_alignment() const {
    if (!_text_format) return unexpected_error(errors::not_initialized, "text_format is not initialized");
    return _text_format->GetParagraphAlignment();
  }
  std::expected<void, error_trace> paragraph_alignment(DWRITE_PARAGRAPH_ALIGNMENT align) {
    if (!_text_format) return unexpected_error(errors::not_initialized, "text_format is not initialized");
    switch (align) {
    case DWRITE_PARAGRAPH_ALIGNMENT_NEAR:
    case DWRITE_PARAGRAPH_ALIGNMENT_CENTER:
    case DWRITE_PARAGRAPH_ALIGNMENT_FAR: _text_format->SetParagraphAlignment(align); return {};
    default: return unexpected_error(errors::invalid_argument, "invalid paragraph alignment value");
    }
  }

  std::expected<DWRITE_WORD_WRAPPING, error_trace> word_wrapping() const {
    if (!_text_format) return unexpected_error(errors::not_initialized, "text_format is not initialized");
    return _text_format->GetWordWrapping();
  }
  std::expected<void, error_trace> word_wrapping(DWRITE_WORD_WRAPPING wrap) {
    if (!_text_format) return unexpected_error(errors::not_initialized, "text_format is not initialized");
    switch (wrap) {
    case DWRITE_WORD_WRAPPING_WRAP:
    case DWRITE_WORD_WRAPPING_NO_WRAP:
    case DWRITE_WORD_WRAPPING_EMERGENCY_BREAK:
    case DWRITE_WORD_WRAPPING_WHOLE_WORD:
    case DWRITE_WORD_WRAPPING_CHARACTER: _text_format->SetWordWrapping(wrap); return {};
    default: return unexpected_error(errors::invalid_argument, "invalid word wrapping value");
    }
  }
};

inline std::expected<void, error_trace> draw_text(
  float2 pos, float2 size, stringable<wchar_t> auto&& text, const text_format& tf, const color& c = colors::black) {
  if (auto res = dwrite.initialize(); !res) return unexpected_error(res.error());
  if (sys::rendertarget.index() != 1) return unexpected_error(errors::invalid_operation, "render target not set");
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
  static std::expected<text_layout, error_trace> create(
    stringable<wchar_t> auto&& text, const text_format& tf, float2 size) {
    if (auto res = dwrite.initialize(); !res) return unexpected_error(res.error());
    auto sv = std::wstring_view(text);
    text_layout tl;
    auto hr = dwrite.factory()->CreateTextLayout(
      sv.data(), UINT(sv.size()), (IDWriteTextFormat*)tf, size.x, size.y, &tl._text_layout.get());
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateTextLayout failed", hr);
    return std::move(tl);
  }

  /// returns position and size `{left, top, width, height}` of the character at the specified text position
  std::expected<float4, error_trace> hit_test(uint1 text_position, bool is_trailing = false) const {
    if (!_text_layout) return unexpected_error(errors::not_initialized, "text_layout is not initialized");
    DWRITE_HIT_TEST_METRICS metrics{};
    float x = 0.0f, y = 0.0f;
    auto hr = _text_layout->HitTestTextPosition(text_position.x, is_trailing, &x, &y, &metrics);
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "HitTestTextPosition failed", hr);
    return float4(metrics.left, metrics.top, metrics.width, metrics.height);
  }

  /// returns text position at the specified point
  std::expected<uint32_t, error_trace> hit_test(float2 point) const {
    if (!_text_layout) return unexpected_error(errors::not_initialized, "text_layout is not initialized");
    DWRITE_HIT_TEST_METRICS metrics{};
    BOOL is_inside = FALSE, is_trailing = FALSE;
    auto hr = _text_layout->HitTestPoint(point.x, point.y, &is_inside, &is_trailing, &metrics);
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "HitTestPoint failed", hr);
    return metrics.textPosition + uint32_t(is_trailing);
  }
};

inline std::expected<void, error_trace> draw_text_layout(
  float2 pos, const text_layout& tl, const color& c = colors::black) {
  if (auto res = dwrite.initialize(); !res) return unexpected_error(res.error());
  if (sys::rendertarget.index() != 1) return unexpected_error(errors::invalid_operation, "render target not set");
  d2d.solid_brush()->SetColor((const D2D1_COLOR_F*)&c);
  d2d.context()->DrawTextLayout(D2D1_POINT_2F{pos.x, pos.y}, static_cast<IDWriteTextLayout*>(tl), d2d.solid_brush());
  return {};
}
} // namespace yw
