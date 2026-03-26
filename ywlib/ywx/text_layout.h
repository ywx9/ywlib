#pragma once
#include "ywx/text_format.h"

namespace yw {

template<typename T> concept text_layout_like = castable_to<T, IDWriteTextLayout*>;

class text_layout {
  comptr<IDWriteTextLayout> _p;
  text_draw_options _draw_options{text_draw_option::clip};

  text_layout(const text_layout&) = delete;
  text_layout& operator=(const text_layout&) = delete;

public:
  explicit operator bool() const noexcept { return _p.operator bool(); }
  explicit operator IDWriteTextLayout*&() & noexcept { return _p.get(); }
  explicit operator IDWriteTextLayout*() const& noexcept { return _p.get(); }
  explicit operator IDWriteTextFormat*() const& noexcept { return static_cast<IDWriteTextFormat*>(_p.get()); }

  text_layout() noexcept = default;
  text_layout(text_layout&&) noexcept = default;
  text_layout& operator=(text_layout&&) noexcept = default;

  template<text_format_like TF, stringable<wchar_t> S> static std::expected<text_layout, error_trace> create(
    S&& text, TF&& tf, float2 size, std::optional<yw::text_draw_options> draw_options = {}) {
    if (auto res = dwrite.initialize(); !res) return unexpected_error(res.error());
    const auto sv = std::wstring_view(text);
    const auto tfp = static_cast<IDWriteTextFormat*>(tf);
    if (!tfp) return unexpected_error(errors::not_initialized, "text_format is not initialized");
    text_layout result;
    auto hr = dwrite.factory()->CreateTextLayout(sv.data(), UINT(sv.size()), tfp, size.x, size.y, &result._p.get());
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateTextLayout failed", int32_t(hr));
    if (!draw_options) {
      if constexpr (derived_from<remove_cvref<TF>, text_format>) result.text_draw_options(tf.text_draw_options());
      else if constexpr (derived_from<remove_cvref<TF>, text_layout>) result.text_draw_options(tf.text_draw_options());
      else result._draw_options = text_draw_option::clip;
    } else result._draw_options = *draw_options;
    return std::move(result);
  }

  template<text_layout_like TL, stringable<wchar_t> S> static std::expected<text_layout, error_trace> create(
    S&& text, TL&& tl, std::optional<yw::text_draw_options> draw_options = {}) {
    if (auto res = dwrite.initialize(); !res) return unexpected_error(res.error());
    const auto sv = std::wstring_view(text);
    const auto tlp = static_cast<IDWriteTextLayout*>(tl);
    if (!tlp) return unexpected_error(errors::not_initialized, "text_layout is not initialized");
    const auto size = float2(tlp->GetMaxWidth(), tlp->GetMaxHeight());
    text_layout result;
    auto hr = dwrite.factory()->CreateTextLayout(sv.data(), UINT(sv.size()), tlp, size.x, size.y, &result._p.get());
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateTextLayout failed", int32_t(hr));
    if (!draw_options) {
      if constexpr (derived_from<remove_cvref<TL>, text_layout>) result._draw_options = tl.text_draw_options();
      else result._draw_options = text_draw_option::clip;
    } else result._draw_options = *draw_options;
    return std::move(result);
  }

  template<stringable S> static std::expected<text_layout, error_trace> create(
    S&& text, float2 size, std::optional<yw::text_draw_options> draw_options = {}) {
    if (auto res = dwrite.initialize(); !res) return unexpected_error(res.error());
    if (auto res = create(text, dwrite.text_format(), size, draw_options); !res) return unexpected_error(res.error());
    else return std::move(*res);
  }

  /// returns position and size `{left, top, width, height}` of the character at the specified text position
  std::expected<float4, error_trace> hit_test(uint1 text_position, bool is_trailing = false) const {
    if (!_p) return unexpected_error(errors::not_initialized, "text_layout is not initialized");
    DWRITE_HIT_TEST_METRICS metrics{};
    float x = 0.0f, y = 0.0f;
    auto hr = _p->HitTestTextPosition(text_position.x, is_trailing, &x, &y, &metrics);
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "HitTestTextPosition failed", int32_t(hr));
    return float4(metrics.left, metrics.top, metrics.width, metrics.height);
  }

  /// returns text position at the specified point
  std::expected<uint32_t, error_trace> hit_test(float2 point) const {
    if (!_p) return unexpected_error(errors::not_initialized, "text_layout is not initialized");
    DWRITE_HIT_TEST_METRICS metrics{};
    BOOL is_inside = FALSE, is_trailing = FALSE;
    auto hr = _p->HitTestPoint(point.x, point.y, &is_inside, &is_trailing, &metrics);
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "HitTestPoint failed", int32_t(hr));
    return metrics.textPosition + uint32_t(is_trailing);
  }

  /// returns the size of the text layout
  std::expected<float2, error_trace> layout_size() const {
    if (!_p) return unexpected_error(errors::not_initialized, "text_layout is not initialized");
    return float2(_p->GetMaxWidth(), _p->GetMaxHeight());
  }

  std::expected<std::wstring, error_trace> font_name() const {
    if (!_p) return unexpected_error(errors::not_initialized, "text_layout is not initialized");
    if (auto res = internal::get_font_name(_p.get()); res) return std::move(*res);
    else return unexpected_error(res.error());
  }

  std::expected<float, error_trace> font_size() const {
    if (!_p) return unexpected_error(errors::not_initialized, "text_layout is not initialized");
    return static_cast<IDWriteTextFormat*>(_p.get())->GetFontSize();
  }

  std::expected<font_weight, error_trace> font_weight() const {
    const auto tf = static_cast<IDWriteTextFormat*>(_p.get());
    if (!tf) return unexpected_error(errors::not_initialized, "text_layout is not initialized");
    return yw::font_weight(tf->GetFontWeight());
  }

  std::expected<font_style, error_trace> font_style() const {
    const auto tf = static_cast<IDWriteTextFormat*>(_p.get());
    if (!tf) return unexpected_error(errors::not_initialized, "text_layout is not initialized");
    return yw::font_style(tf->GetFontStyle());
  }

  std::expected<font_stretch, error_trace> font_stretch() const {
    const auto tf = static_cast<IDWriteTextFormat*>(_p.get());
    if (!tf) return unexpected_error(errors::not_initialized, "text_layout is not initialized");
    return yw::font_stretch(tf->GetFontStretch());
  }

  std::expected<text_alignment, error_trace> text_alignment() const {
    const auto tf = static_cast<IDWriteTextFormat*>(_p.get());
    if (!tf) return unexpected_error(errors::not_initialized, "text_layout is not initialized");
    return yw::text_alignment(tf->GetTextAlignment());
  }

  std::expected<void, error_trace> text_alignment(yw::text_alignment align) {
    const auto tf = static_cast<IDWriteTextFormat*>(_p.get());
    if (!tf) return unexpected_error(errors::not_initialized, "text_layout is not initialized");
    if (auto hr = tf->SetTextAlignment(DWRITE_TEXT_ALIGNMENT(align)); FAILED(hr))
      return unexpected_error(errors::operation_failed, "SetTextAlignment failed", int32_t(hr));
    else return {};
  }

  std::expected<paragraph_alignment, error_trace> paragraph_alignment() const {
    const auto tf = static_cast<IDWriteTextFormat*>(_p.get());
    if (!tf) return unexpected_error(errors::not_initialized, "text_layout is not initialized");
    return yw::paragraph_alignment(tf->GetParagraphAlignment());
  }

  std::expected<void, error_trace> paragraph_alignment(yw::paragraph_alignment align) {
    const auto tf = static_cast<IDWriteTextFormat*>(_p.get());
    if (!tf) return unexpected_error(errors::not_initialized, "text_layout is not initialized");
    if (auto hr = tf->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT(align)); FAILED(hr))
      return unexpected_error(errors::operation_failed, "SetParagraphAlignment failed", int32_t(hr));
    else return {};
  }

  yw::text_draw_options text_draw_options() const noexcept { return _draw_options; }

  std::expected<void, error_trace> text_draw_options(yw::text_draw_options value) {
    const auto tfp = static_cast<IDWriteTextFormat*>(_p.get());
    if (!tfp) return unexpected_error(errors::not_initialized, "text_layout is not initialized");
    const auto wrap = value.contains(text_draw_option::wrap) ? DWRITE_WORD_WRAPPING_WRAP : DWRITE_WORD_WRAPPING_NO_WRAP;
    if (auto hr = tfp->SetWordWrapping(wrap); FAILED(hr))
      return unexpected_error(errors::operation_failed, "SetWordWrapping failed", int32_t(hr));
    _draw_options = value;
    return {};
  }

  std::expected<float2, error_trace> metrics_size() const {
    if (!_p) return unexpected_error(errors::not_initialized, "text_layout is not initialized");
    DWRITE_TEXT_METRICS metrics{};
    if (auto hr = _p->GetMetrics(&metrics); FAILED(hr))
      return unexpected_error(errors::operation_failed, "GetMetrics failed", int32_t(hr));
    return float2(metrics.width, metrics.height);
  }

  std::expected<void, error_trace> shrink_to_fit() {
    if (!_p) return unexpected_error(errors::not_initialized, "text_layout is not initialized");
    if (auto hr = _p->SetMaxWidth(1e6f); FAILED(hr))
      return unexpected_error(errors::operation_failed, "SetMaxWidth failed", int32_t(hr));
    if (auto hr = _p->SetMaxHeight(1e6f); FAILED(hr))
      return unexpected_error(errors::operation_failed, "SetMaxHeight failed", int32_t(hr));
    if (auto size = metrics_size()) {
      if (auto hr = _p->SetMaxWidth(size->x); FAILED(hr))
        return unexpected_error(errors::operation_failed, "SetMaxWidth failed", int32_t(hr));
      if (auto hr = _p->SetMaxHeight(size->y); FAILED(hr))
        return unexpected_error(errors::operation_failed, "SetMaxHeight failed", int32_t(hr));
    } else return unexpected_error(size.error());
    return {};
  }

  std::expected<std::vector<float4>, error_trace> hit_test_range(uint2 Range, float2 Origin = {}) const {
    /// \note This function ignores maxBidiReorderingDepth.
    if (!_p) return unexpected_error(errors::not_initialized, "text_layout is not initialized");
    if (Range.y <= Range.x) return std::vector<float4>{}; // empty range
    const auto length = Range.y - Range.x;
    uint32_t actual_count = 0;
    auto hr = _p->HitTestTextRange(Range.x, length, Origin.x, Origin.y, nullptr, 0, &actual_count);
    if (hr != E_NOT_SUFFICIENT_BUFFER)
      return unexpected_error(errors::operation_failed, "HitTestTextRange failed", int32_t(hr));
    std::vector<DWRITE_HIT_TEST_METRICS> metrics(actual_count);
    hr = _p->HitTestTextRange(Range.x, length, Origin.x, Origin.y, metrics.data(), actual_count, &actual_count);
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "HitTestTextRange failed", int32_t(hr));
    std::vector<float4> rects;
    rects.reserve(actual_count);
    for (const auto& m : metrics) rects.emplace_back(m.left, m.top, m.width, m.height);
    return rects;
  }
};

//////////////////////////////////////// MARK: draw_text

template<text_layout_like TL> std::expected<void, error_trace> draw_text_layout(float2 pos, TL&& tl, std::optional<bool> clip = {}) {
  if (auto res = dwrite.initialize(); !res) return unexpected_error(res.error());
  if (!drawing::d2d_drawing()) return unexpected_error(errors::invalid_operation, "drawing not begun");
  const auto tlp = static_cast<IDWriteTextLayout*>(tl);
  D2D1_DRAW_TEXT_OPTIONS draw_options = D2D1_DRAW_TEXT_OPTIONS_NONE;
  if (!clip) {
    if constexpr (derived_from<remove_cvref<TL>, text_layout>) {
      if (tl.text_draw_options().contains(text_draw_option::clip)) draw_options |= D2D1_DRAW_TEXT_OPTIONS_CLIP;
    } else draw_options |= D2D1_DRAW_TEXT_OPTIONS_CLIP;
  } else if (*clip) draw_options |= D2D1_DRAW_TEXT_OPTIONS_CLIP;
  d2d.context()->DrawTextLayout(D2D1_POINT_2F{pos.x, pos.y}, tlp, brush.d2d_brush(), draw_options);
  return {};
}
} // namespace yw

// #pragma once
// #include "ywx/text_format.h"

// namespace yw {

// template<typename T> concept text_layout_like = castable_to<T, IDWriteTextLayout*>;

// class text_layout {
//   comptr<IDWriteTextLayout> _p{};

//   text_layout(const text_layout&) = delete;
//   text_layout& operator=(const text_layout&) = delete;

// public:
//   explicit operator bool() const noexcept { return bool(_p); }
//   explicit operator IDWriteTextLayout*&() & noexcept { return _p.get(); }
//   explicit operator IDWriteTextLayout*() const& noexcept { return _p.get(); }
//   explicit operator IDWriteTextFormat*() const& noexcept { return static_cast<IDWriteTextFormat*>(_p.get()); }

//   text_layout() noexcept = default;
//   text_layout(text_layout&&) = default;
//   text_layout& operator=(text_layout&&) = default;

//   template<text_format_like TF, stringable S> static std::expected<text_layout, error_trace> create(
//     TF&& tf, S&& text, float2 size, std::optional<yw::text_draw_options> draw_options = {}) {
//     if (auto res = dwrite.initialize(); !res) return unexpected_error(res.error());
//     auto s = unicode<wchar_t>(static_cast<S&&>(text));
//     const auto tfp = static_cast<IDWriteTextFormat*>(tf);
//     if (!tfp) return unexpected_error(errors::not_initialized, "text_format is not initialized");
//     text_layout result;
//     auto hr = dwrite.factory()->CreateTextLayout(s.data(), UINT(s.size()), tfp, size.x, size.y, &result._p.get());
//     if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateTextLayout failed", int32_t(hr));
//     result._text = std::move(s);
//     result._size = size;
//     if (!draw_options) {
//       if constexpr (derived_from<remove_cvref<TF>, text_format>) result.text_draw_options(tf.text_draw_options());
//       else if constexpr (derived_from<remove_cvref<TF>, text_layout>)
//       result.text_draw_options(tf.text_draw_options()); else result._draw_options = text_draw_option::clip;
//     } else result._draw_options = *draw_options;
//     return std::move(result);
//   }

//   template<text_layout_like TL, stringable S> static std::expected<text_layout, error_trace> create(
//     TL&& tl, S&& text, std::optional<yw::text_draw_options> draw_options = {}) {
//     if (auto res = dwrite.initialize(); !res) return unexpected_error(res.error());
//     auto s = unicode<wchar_t>(static_cast<S&&>(text));
//     const auto tlp = static_cast<IDWriteTextLayout*>(tl);
//     if (!tlp) return unexpected_error(errors::not_initialized, "text_layout is not initialized");
//     const auto size = float2(tlp->GetMaxWidth(), tlp->GetMaxHeight());
//     text_layout result;
//     auto hr = dwrite.factory()->CreateTextLayout(s.data(), UINT(s.size()), tlp, size.x, size.y, &result._p.get());
//     if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateTextLayout failed", int32_t(hr));
//     result._text = std::move(s);
//     result._size = size;
//     if (!draw_options) {
//       if constexpr (derived_from<remove_cvref<TL>, text_layout>) result._draw_options = tl.text_draw_options();
//       else result._draw_options = text_draw_option::clip;
//     } else result._draw_options = *draw_options;
//     return std::move(result);
//   }

//   template<stringable S> static std::expected<text_layout, error_trace> create(
//     S&& text, std::optional<yw::text_draw_options> draw_options = {}) {
//     if (auto res = dwrite.initialize(); !res) return unexpected_error(res.error());
//     if (auto res = create(text, dwrite.text_format(), draw_options); !res) return unexpected_error(res.error());
//     else return std::move(*res);
//   }

//   std::expected<float4, error_trace> hit_test(uint1 text_position, bool is_trailing = false) const {
//     if (!_p) return unexpected_error(errors::not_initialized, "text_layout is not initialized");
//     DWRITE_HIT_TEST_METRICS metrics{};
//     float x = 0.0f, y = 0.0f;
//     auto hr = _p->HitTestTextPosition(text_position.x, is_trailing, &x, &y, &metrics);
//     if (FAILED(hr)) return unexpected_error(errors::operation_failed, "HitTestTextPosition failed", int32_t(hr));
//     return float4(metrics.left, metrics.top, metrics.width, metrics.height);
//   }

//   std::expected<uint32_t, error_trace> hit_test(float2 point) const {
//     if (!_p) return unexpected_error(errors::not_initialized, "text_layout is not initialized");
//     DWRITE_HIT_TEST_METRICS metrics{};
//     BOOL is_inside = FALSE, is_trailing = FALSE;
//     auto hr = _p->HitTestPoint(point.x, point.y, &is_inside, &is_trailing, &metrics);
//     if (FAILED(hr)) return unexpected_error(errors::operation_failed, "HitTestPoint failed", int32_t(hr));
//     return metrics.textPosition + (is_trailing ? 1 : 0);
//   }

//   float2 size() const noexcept { return _size; }

//   std::expected<void, error_trace> size(float2 NewSize) {
//     if (!_p) return unexpected_error(errors::not_initialized, "text_layout is not initialized");
//     if (auto res = create(_p.get(), _text, NewSize, _draw_options)) *this = std::move(*res);
//     else return unexpected_error(res.error());
//     return {};
//   }

//   const std::wstring& text() const noexcept { return _text; }

//   template<stringable S> std::expected<void, error_trace> text(S&& NewText) {
//     if (!_p) return unexpected_error(errors::not_initialized, "text_layout is not initialized");
//     if (auto res = create(_p.get(), static_cast<S&&>(NewText), _size, _draw_options)) *this = std::move(*res);
//     else return unexpected_error(res.error());
//     return {};
//   }

//   std::expected<std::wstring, error_trace> font_name() const {
//     const auto tf = static_cast<IDWriteTextFormat*>(_p.get());
//     if (!tf) return unexpected_error(errors::not_initialized, "text_layout is not initialized");
//     if (auto res = internal::get_font_name(tf)) return std::move(*res);
//     else return unexpected_error(res.error());
//   }

//   std::expected<float, error_trace> font_size() const {
//     const auto tf = static_cast<IDWriteTextFormat*>(_p.get());
//     if (!tf) return unexpected_error(errors::not_initialized, "text_layout is not initialized");
//     return tf->GetFontSize();
//   }

//   std::expected<font_weight, error_trace> font_weight() const {
//     const auto tf = static_cast<IDWriteTextFormat*>(_p.get());
//     if (!tf) return unexpected_error(errors::not_initialized, "text_layout is not initialized");
//     return yw::font_weight(tf->GetFontWeight());
//   }

//   std::expected<font_style, error_trace> font_style() const {
//     const auto tf = static_cast<IDWriteTextFormat*>(_p.get());
//     if (!tf) return unexpected_error(errors::not_initialized, "text_layout is not initialized");
//     return yw::font_style(tf->GetFontStyle());
//   }

//   std::expected<font_stretch, error_trace> font_stretch() const {
//     const auto tf = static_cast<IDWriteTextFormat*>(_p.get());
//     if (!tf) return unexpected_error(errors::not_initialized, "text_layout is not initialized");
//     return yw::font_stretch(tf->GetFontStretch());
//   }

//   std::expected<text_alignment, error_trace> text_alignment() const {
//     const auto tf = static_cast<IDWriteTextFormat*>(_p.get());
//     if (!tf) return unexpected_error(errors::not_initialized, "text_layout is not initialized");
//     return yw::text_alignment(tf->GetTextAlignment());
//   }

//   std::expected<void, error_trace> text_alignment(yw::text_alignment align) {
//     const auto tf = static_cast<IDWriteTextFormat*>(_p.get());
//     if (!tf) return unexpected_error(errors::not_initialized, "text_layout is not initialized");
//     if (auto hr = tf->SetTextAlignment(DWRITE_TEXT_ALIGNMENT(align)); FAILED(hr))
//       return unexpected_error(errors::operation_failed, "SetTextAlignment failed", int32_t(hr));
//     else return {};
//   }

//   std::expected<paragraph_alignment, error_trace> paragraph_alignment() const {
//     const auto tf = static_cast<IDWriteTextFormat*>(_p.get());
//     if (!tf) return unexpected_error(errors::not_initialized, "text_layout is not initialized");
//     return yw::paragraph_alignment(tf->GetParagraphAlignment());
//   }

//   std::expected<void, error_trace> paragraph_alignment(yw::paragraph_alignment align) {
//     const auto tf = static_cast<IDWriteTextFormat*>(_p.get());
//     if (!tf) return unexpected_error(errors::not_initialized, "text_layout is not initialized");
//     if (auto hr = tf->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT(align)); FAILED(hr))
//       return unexpected_error(errors::operation_failed, "SetParagraphAlignment failed", int32_t(hr));
//     else return {};
//   }

//   yw::text_draw_options text_draw_options() const noexcept { return _draw_options; }

//   std::expected<void, error_trace> text_draw_options(yw::text_draw_options value) {
//     const auto tfp = static_cast<IDWriteTextFormat*>(_p.get());
//     if (!tfp) return unexpected_error(errors::not_initialized, "text_layout is not initialized");
//     const auto wrap = value.contains(text_draw_option::wrap) ? DWRITE_WORD_WRAPPING_WRAP :
//     DWRITE_WORD_WRAPPING_NO_WRAP; if (auto hr = tfp->SetWordWrapping(wrap); FAILED(hr))
//       return unexpected_error(errors::operation_failed, "SetWordWrapping failed", int32_t(hr));
//     _draw_options = value;
//     return {};
//   }

//   std::expected<void, error_trace> shrink_to_fit() {
//     if (!_p) return unexpected_error(errors::not_initialized, "text_layout is not initialized");
//     if (auto res = create(_p.get(), _text, float2{1e6f, 1e6f}, _draw_options)) *this = std::move(*res);
//     else return unexpected_error(res.error());
//     return {};
//   }
// };

// //////////////////////////////////////// MARK: draw_text

// template<text_layout_like TL> std::expected<void, error_trace> draw_text(float2 pos, TL&& tl, std::optional<bool>
// clip = {}) {
//   if (auto res = dwrite.initialize(); !res) return unexpected_error(res.error());
//   if (!drawing::d2d_drawing()) return unexpected_error(errors::invalid_operation, "drawing not begun");
//   const auto tlp = static_cast<IDWriteTextLayout*>(tl);
//   D2D1_DRAW_TEXT_OPTIONS options = D2D1_DRAW_TEXT_OPTIONS_NONE;
//   if (!clip) {
//     if constexpr (derived_from<remove_cvref<TL>, text_layout>) {
//       if (tl.text_draw_options().contains(text_draw_option::clip)) options |= D2D1_DRAW_TEXT_OPTIONS_CLIP;
//     } else options |= D2D1_DRAW_TEXT_OPTIONS_CLIP;
//   } else if (*clip) options |= D2D1_DRAW_TEXT_OPTIONS_CLIP;
//   d2d.context()->DrawTextLayout(D2D1_POINT_2F{pos.x, pos.y}, tlp, brush.d2d_brush(), options);
//   return {};
// }

// // class text_layout {
// //   comptr<IDWriteTextLayout> _p;

// //   text_layout(const text_layout&) = delete;
// //   text_layout& operator=(const text_layout&) = delete;

// // public:
// //   explicit operator bool() const noexcept { return _p.operator bool(); }
// //   explicit operator IDWriteTextLayout*&() & noexcept { return _p.get(); }
// //   explicit operator IDWriteTextLayout*() const& noexcept { return _p.get(); }
// //   explicit operator IDWriteTextFormat*() const& noexcept { return static_cast<IDWriteTextFormat*>(_p.get()); }

// //   text_layout() noexcept = default;
// //   text_layout(text_layout&&) = default;
// //   text_layout& operator=(text_layout&&) = default;

// //   std::expected<float2, error_trace> metrics_size() const {
// //     if (!_p) return unexpected_error(errors::not_initialized, "text_layout is not initialized");
// //     DWRITE_TEXT_METRICS metrics{};
// //     if (auto hr = _p->GetMetrics(&metrics); FAILED(hr))
// //       return unexpected_error(errors::operation_failed, "GetMetrics failed", int32_t(hr));
// //     return float2(metrics.width, metrics.height);
// //   }

// //   std::expected<void, error_trace> adjust_size() {
// //     if (!_p) return unexpected_error(errors::not_initialized, "text_layout is not initialized");
// //     _p->SetMaxWidth(1e6f);
// //     _p->SetMaxHeight(1e6f);
// //     if (auto size = metrics_size()) {
// //       _p->SetMaxWidth(size->x);
// //       _p->SetMaxHeight(size->y);
// //       return {};
// //     } else return unexpected_error(size.error());
// //   }

// //   std::expected<std::vector<float4>, error_trace> hit_test_range(uint2 Range, float2 Origin = {}) const {
// //     /// \note This function ignores maxBidiReorderingDepth.
// //     if (!_p) return unexpected_error(errors::not_initialized, "text_layout is not initialized");
// //     if (Range.y <= Range.x) return std::vector<float4>{}; // empty range
// //     const auto length = Range.y - Range.x;
// //     uint32_t actual_count = 0;
// //     auto hr = _p->HitTestTextRange(Range.x, length, Origin.x, Origin.y, nullptr, 0, &actual_count);
// //     if (hr != E_NOT_SUFFICIENT_BUFFER) return unexpected_error(errors::operation_failed, "HitTestTextRange
// failed",
// //     int32_t(hr)); std::vector<DWRITE_HIT_TEST_METRICS> metrics(actual_count); hr = _p->HitTestTextRange(Range.x,
// //     length, Origin.x, Origin.y, metrics.data(), actual_count, &actual_count); if (FAILED(hr)) return
// //     unexpected_error(errors::operation_failed, "HitTestTextRange failed", int32_t(hr)); std::vector<float4> rects;
// //     rects.reserve(actual_count);
// //     for (const auto& m : metrics) rects.emplace_back(m.left, m.top, m.width, m.height);
// //     return rects;
// //   }
// // };

// // //////////////////////////////////////// MARK: draw_text

// // template<text_layout_like TL> std::expected<void, error_trace> draw_text(float2 pos, TL&& text_layout) {
// //   if (auto res = dwrite.initialize(); !res) return unexpected_error(res.error());
// //   if (!drawing::d2d_drawing()) return unexpected_error(errors::invalid_operation, "drawing not begun");
// //   const auto tl = static_cast<IDWriteTextLayout*>(text_layout);
// //   d2d.context()->DrawTextLayout(D2D1_POINT_2F{pos.x, pos.y}, tl, brush.d2d_brush());
// //   return {};
// // }
// } // namespace yw
