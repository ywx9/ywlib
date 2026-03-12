#pragma once
#include "ywx/core.h"

namespace yw {

template<typename T> concept text_format_like = castable_to<T, IDWriteTextFormat*>;

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
  case DWRITE_TEXT_ALIGNMENT_JUSTIFIED: tf->SetTextAlignment(align); return true;
  default: return false;
  }
}

/// \note requires text_format is initialized
inline bool set_paragraph_alignment(IDWriteTextFormat* tf, DWRITE_PARAGRAPH_ALIGNMENT align) {
  switch (align) {
  case DWRITE_PARAGRAPH_ALIGNMENT_NEAR:
  case DWRITE_PARAGRAPH_ALIGNMENT_CENTER:
  case DWRITE_PARAGRAPH_ALIGNMENT_FAR: tf->SetParagraphAlignment(align); return true;
  default: return false;
  }
}
} // namespace internal

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

//////////////////////////////////////// MARK: draw_text

template<text_format_like TF>
std::expected<void, error_trace> draw_text(float2 pos, float2 size, stringable<wchar_t> auto&& text, TF&& text_format) {
  if (auto res = dwrite.initialize(); !res) return unexpected_error(res.error());
  if (!drawing::d2d_drawing()) return unexpected_error(errors::invalid_operation, "drawing not begun");
  const auto sv = std::wstring_view(text);
  const auto tf = static_cast<IDWriteTextFormat*>(text_format);
  d2d.context()->DrawTextW(sv.data(), static_cast<UINT>(sv.size()), tf,
    D2D1_RECT_F(pos.x, pos.y, pos.x + size.x, pos.y + size.y), brush.brush());
  return {};
}
} // namespace yw
