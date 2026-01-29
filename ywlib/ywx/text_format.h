#pragma once
#include "ywx/core.h"

namespace yw {

class text_format {
  comptr<::IDWriteTextFormat> _text_format;

public:
  explicit operator bool() const noexcept { return _text_format.operator bool(); }
  explicit operator ::IDWriteTextFormat*&() & noexcept { return _text_format.get(); }
  explicit operator ::IDWriteTextFormat*() const& noexcept { return _text_format.get(); }

  text_format() noexcept = default;
  text_format(const text_format&) = delete;
  text_format& operator=(const text_format&) = delete;
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
    if (!source) return unexpected_error(errors::not_initialized, "source text_format is not initialized");
    auto n = source._text_format->GetFontFamilyNameLength();
    if (n == 0) return unexpected_error(errors::invalid_operation, "GetFontFamilyNameLength returned 0");
    std::wstring name(n, L'\0');
    if (auto hr = source._text_format->GetFontFamilyName(name.data(), n + 1); FAILED(hr))
      return unexpected_error(errors::operation_failed, "GetFontFamilyName failed", hr);
    const float size = source._text_format->GetFontSize();
    const auto weight = source._text_format->GetFontWeight();
    const auto style = source._text_format->GetFontStyle();
    const auto stretch = source._text_format->GetFontStretch();
    return create(name, size, weight, style, stretch);
  }

  void align_leading() {
    if (_text_format) _text_format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
  }
  void align_center() {
    if (_text_format) _text_format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
  }
  void align_trailing() {
    if (_text_format) _text_format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
  }

  void align_top() {
    if (_text_format) _text_format->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
  }
  void align_middle() {
    if (_text_format) _text_format->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
  }
  void align_bottom() {
    if (_text_format) _text_format->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_FAR);
  }
};

template<stringable<wchar_t> S> std::expected<void, error_trace> draw_text(
  float2 pos, float2 size, S&& text, const text_format& tf, const color& c = colors::black) {
  if (auto res = dwrite.initialize(); !res) return unexpected_error(res.error());
  if (sys::rendertarget.index() != 1) return unexpected_error(errors::invalid_operation, "render target not set");
  d2d.solid_brush()->SetColor((const D2D1_COLOR_F*)&c);
  auto sv = std::wstring_view(text);
  d2d.context()->DrawTextW(sv.data(), static_cast<UINT>(sv.size()), static_cast<::IDWriteTextFormat*>(tf),
    D2D1_RECT_F(pos.x, pos.y, pos.x + size.x, pos.y + size.y), d2d.solid_brush());
  return {};
}
} // namespace yw
