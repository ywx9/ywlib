#pragma once
#include "ywx/core.h"

namespace yw {

class text_layout;

template<typename T> concept text_format_like = castable_to<T, IDWriteTextFormat*>;

struct text_hit_test_result {
  float2 pos;
  float2 size;
  float2 caret_pos;
};

enum class text_alignment : uint8_t {
  left = DWRITE_TEXT_ALIGNMENT_LEADING,
  right = DWRITE_TEXT_ALIGNMENT_TRAILING,
  center = DWRITE_TEXT_ALIGNMENT_CENTER,
  justified = DWRITE_TEXT_ALIGNMENT_JUSTIFIED
};

enum class paragraph_alignment : uint8_t {
  top = DWRITE_PARAGRAPH_ALIGNMENT_NEAR,
  bottom = DWRITE_PARAGRAPH_ALIGNMENT_FAR,
  center = DWRITE_PARAGRAPH_ALIGNMENT_CENTER
};

enum class font_stretch : uint8_t {
  undefined = DWRITE_FONT_STRETCH_UNDEFINED,
  ultra_condensed = DWRITE_FONT_STRETCH_ULTRA_CONDENSED,
  extra_condensed = DWRITE_FONT_STRETCH_EXTRA_CONDENSED,
  condensed = DWRITE_FONT_STRETCH_CONDENSED,
  semi_condensed = DWRITE_FONT_STRETCH_SEMI_CONDENSED,
  normal = DWRITE_FONT_STRETCH_NORMAL,
  medium = DWRITE_FONT_STRETCH_MEDIUM,
  semi_expanded = DWRITE_FONT_STRETCH_SEMI_EXPANDED,
  expanded = DWRITE_FONT_STRETCH_EXPANDED,
  extra_expanded = DWRITE_FONT_STRETCH_EXTRA_EXPANDED,
  ultra_expanded = DWRITE_FONT_STRETCH_ULTRA_EXPANDED
};

enum class font_style : uint8_t {
  normal = DWRITE_FONT_STYLE_NORMAL,
  italic = DWRITE_FONT_STYLE_ITALIC,
  oblique = DWRITE_FONT_STYLE_OBLIQUE
};

enum class font_weight : uint16_t {
  thin = DWRITE_FONT_WEIGHT_THIN,
  extra_light = DWRITE_FONT_WEIGHT_EXTRA_LIGHT,
  ultra_light = DWRITE_FONT_WEIGHT_ULTRA_LIGHT,
  light = DWRITE_FONT_WEIGHT_LIGHT,
  semi_light = DWRITE_FONT_WEIGHT_SEMI_LIGHT,
  normal = DWRITE_FONT_WEIGHT_NORMAL,
  regular = DWRITE_FONT_WEIGHT_REGULAR,
  medium = DWRITE_FONT_WEIGHT_MEDIUM,
  demi_bold = DWRITE_FONT_WEIGHT_DEMI_BOLD,
  semi_bold = DWRITE_FONT_WEIGHT_SEMI_BOLD,
  bold = DWRITE_FONT_WEIGHT_BOLD,
  extra_bold = DWRITE_FONT_WEIGHT_EXTRA_BOLD,
  ultra_bold = DWRITE_FONT_WEIGHT_ULTRA_BOLD,
  black = DWRITE_FONT_WEIGHT_BLACK,
  heavy = DWRITE_FONT_WEIGHT_HEAVY,
  extra_black = DWRITE_FONT_WEIGHT_EXTRA_BLACK,
  ultra_black = DWRITE_FONT_WEIGHT_ULTRA_BLACK
};

enum class text_draw_option : uint8_t { no = 0b00, wrap = 0b01, clip = 0b10, wrap_clip = 0b11 };
using text_draw_options = flags<text_draw_option>;

namespace internal {

/// \note requires text_format is initialized
inline std::expected<std::wstring, error_trace> get_font_name(IDWriteTextFormat* tf) {
  auto n = tf->GetFontFamilyNameLength();
  std::wstring name(n, L'\0');
  if (auto hr = tf->GetFontFamilyName(name.data(), n + 1); SUCCEEDED(hr)) return name;
  else return unexpected_error(errors::operation_failed, "GetFontFamilyName failed", int32_t(hr));
}
} // namespace internal

//////////////////////////////////////// MARK: text_format

class text_format {
  comptr<IDWriteTextFormat> _p{};
  text_draw_options _draw_options{text_draw_option::clip};

  text_format(const text_format&) = delete;
  text_format& operator=(const text_format&) = delete;

public:
  explicit operator bool() const noexcept { return bool(_p); }
  explicit operator IDWriteTextFormat*&() & noexcept { return _p.get(); }
  explicit operator IDWriteTextFormat*() const& noexcept { return _p.get(); }

  text_format() noexcept = default;

  text_format(text_format&& other) noexcept
    : _p(std::exchange(other._p, {})), _draw_options(other._draw_options) {}

  text_format& operator=(text_format&& other) noexcept {
    if (this == &other) return *this;
    if (_p) _p->Release();
    _p = std::exchange(other._p, {});
    _draw_options = other._draw_options;
    return *this;
  }

  virtual ~text_format() noexcept {
    try {
      if (_p) std::exchange(_p, {})->Release();
    } catch (...) {}
  }

  static std::expected<text_format, error_trace> create(
    null_terminated<wchar_t> name, float1 size, font_weight weight = font_weight::regular,
    font_style style = font_style::normal, font_stretch stretch = font_stretch::normal,
    text_draw_options draw_options = text_draw_option::clip) {
    text_format tf;
    if (auto res = dwrite.initialize(); !res) return unexpected_error(res.error());
    auto hr = dwrite.factory()->CreateTextFormat(
      name.data(), nullptr, DWRITE_FONT_WEIGHT(weight), DWRITE_FONT_STYLE(style), DWRITE_FONT_STRETCH(stretch), size.x,
      L"", &tf._p.get());
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateTextFormat failed", int32_t(hr));
    return std::move(tf);
  }

  template<text_format_like TF>
  static std::expected<text_format, error_trace> create(TF&& tf, std::optional<text_draw_options> draw_options = {}) {
    if (auto res = dwrite.initialize(); !res) return unexpected_error(res.error());
    const auto tfp = static_cast<IDWriteTextFormat*>(tf);
    if (!tfp) return unexpected_error(errors::not_initialized, "source text_format is not initialized");
    auto name = internal::get_font_name(tfp);
    if (!name) return unexpected_error(name.error());
    text_format result;
    auto hr = dwrite.factory()->CreateTextFormat(
      name->data(), nullptr, tfp->GetFontWeight(), tfp->GetFontStyle(), tfp->GetFontStretch(), tfp->GetFontSize(), L"",
      &result._p.get());
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateTextFormat failed", int32_t(hr));
    if (!draw_options) {
      if constexpr (derived_from<remove_cvref<TF>, text_format>) result._draw_options = tf.text_draw_options();
      else result._draw_options = text_draw_option::clip;
    } else result._draw_options = *draw_options;
    return std::move(tf);
  }

  std::expected<std::wstring, error_trace> font_name() const {
    if (!_p) return unexpected_error(errors::not_initialized, "text_format is not initialized");
    if (auto res = internal::get_font_name(_p.get()); res) return std::move(*res);
    else return unexpected_error(res.error());
  }

  std::expected<float, error_trace> font_size() const {
    if (!_p) return unexpected_error(errors::not_initialized, "text_format is not initialized");
    return _p->GetFontSize();
  }

  std::expected<yw::font_weight, error_trace> font_weight() const {
    if (!_p) return unexpected_error(errors::not_initialized, "text_format is not initialized");
    return yw::font_weight(_p->GetFontWeight());
  }

  std::expected<yw::font_style, error_trace> font_style() const {
    if (!_p) return unexpected_error(errors::not_initialized, "text_format is not initialized");
    return yw::font_style(_p->GetFontStyle());
  }

  std::expected<yw::font_stretch, error_trace> font_stretch() const {
    if (!_p) return unexpected_error(errors::not_initialized, "text_format is not initialized");
    return yw::font_stretch(_p->GetFontStretch());
  }

  std::expected<yw::text_alignment, error_trace> text_alignment() const {
    if (!_p) return unexpected_error(errors::not_initialized, "text_format is not initialized");
    return yw::text_alignment(_p->GetTextAlignment());
  }

  std::expected<void, error_trace> text_alignment(yw::text_alignment align) {
    if (!_p) return unexpected_error(errors::not_initialized, "text_format is not initialized");
    if (auto hr = _p->SetTextAlignment(DWRITE_TEXT_ALIGNMENT(align)); FAILED(hr))
      return unexpected_error(errors::operation_failed, "SetTextAlignment failed", int32_t(hr));
    else return {};
  }

  std::expected<yw::paragraph_alignment, error_trace> paragraph_alignment() const {
    if (!_p) return unexpected_error(errors::not_initialized, "text_format is not initialized");
    return yw::paragraph_alignment(_p->GetParagraphAlignment());
  }

  std::expected<void, error_trace> paragraph_alignment(yw::paragraph_alignment align) {
    if (!_p) return unexpected_error(errors::not_initialized, "text_format is not initialized");
    if (auto hr = _p->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT(align)); FAILED(hr))
      return unexpected_error(errors::operation_failed, "SetParagraphAlignment failed", int32_t(hr));
    else return {};
  }

  yw::text_draw_options text_draw_options() const noexcept { return _draw_options; }

  std::expected<void, error_trace> text_draw_options(yw::text_draw_options value) noexcept {
    if (!_p) return unexpected_error(errors::not_initialized, "text_format is not initialized");
    _draw_options = value;
    if (value.contains(text_draw_option::wrap)) {
      if (auto hr = _p->SetWordWrapping(DWRITE_WORD_WRAPPING_WRAP); FAILED(hr))
        return unexpected_error(errors::operation_failed, "SetWordWrapping failed", int32_t(hr));
    } else if (auto hr = _p->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP); FAILED(hr))
      return unexpected_error(errors::operation_failed, "SetWordWrapping failed", int32_t(hr));
    return {};
  }
};

template<text_format_like TF, stringable<wchar_t> S>
std::expected<void, error_trace> draw_text(float2 pos, float2 size, S&& text, TF&& tf, std::optional<bool> clip = {}) {
  if (auto res = dwrite.initialize(); !res) return unexpected_error(res.error());
  if (!drawing::d2d_drawing()) return unexpected_error(errors::invalid_operation, "drawing not begun");
  const auto sv = std::wstring_view(text);
  const auto tfp = static_cast<IDWriteTextFormat*>(tf);
  D2D1_DRAW_TEXT_OPTIONS options = D2D1_DRAW_TEXT_OPTIONS_NONE;
  if (!clip) {
    if constexpr (derived_from<remove_cvref<TF>, text_format>) {
      if (tf.text_draw_options().contains(text_draw_option::wrap)) options |= D2D1_DRAW_TEXT_OPTIONS_CLIP;
    } else options |= D2D1_DRAW_TEXT_OPTIONS_CLIP;
  } else if (*clip) options |= D2D1_DRAW_TEXT_OPTIONS_CLIP;
  const auto rect = D2D1_RECT_F(pos.x, pos.y, pos.x + size.x, pos.y + size.y);
  d2d.context()->DrawTextW(sv.data(), static_cast<UINT>(sv.size()), tfp, &rect, brush.d2d_brush(), options);
  return {};
}
} // namespace yw
