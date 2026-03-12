#pragma once
#include "ywx/text_format.h"

namespace yw {

template<typename T> concept text_layout_like = castable_to<T, IDWriteTextLayout*>;

class text_layout {
  comptr<IDWriteTextLayout> _p;

  text_layout(const text_layout&) = delete;
  text_layout& operator=(const text_layout&) = delete;

public:
  explicit operator bool() const noexcept { return _p.operator bool(); }
  explicit operator IDWriteTextLayout*&() & noexcept { return _p.get(); }
  explicit operator IDWriteTextLayout*() const& noexcept { return _p.get(); }
  explicit operator IDWriteTextFormat*() const& noexcept { return static_cast<IDWriteTextFormat*>(_p.get()); }

  text_layout() noexcept = default;
  text_layout(text_layout&&) = default;
  text_layout& operator=(text_layout&&) = default;

  /// creates text layout
  static std::expected<text_layout, error_trace> create(
    stringable<wchar_t> auto&& text, text_format_like auto&& text_format, float2 size) {
    if (auto res = dwrite.initialize(); !res) return unexpected_error(res.error());
    auto sv = std::wstring_view(text);
    auto tf = static_cast<IDWriteTextFormat*>(text_format);
    if (!tf) return unexpected_error(errors::not_initialized, "text_format is not initialized");
    text_layout tl;
    auto hr = dwrite.factory()->CreateTextLayout(sv.data(), UINT(sv.size()), tf, size.x, size.y, &tl._p.get());
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateTextLayout failed", int32_t(hr));
    return std::move(tl);
  }

  /// creates text layout from another text layout
  static std::expected<text_layout, error_trace> create(
    stringable<wchar_t> auto&& text, text_layout_like auto&& source) {
    if (auto res = dwrite.initialize(); !res) return unexpected_error(res.error());
    auto sv = std::wstring_view(text);
    auto tf = static_cast<IDWriteTextFormat*>(static_cast<IDWriteTextLayout*>(source));
    if (!tf) return unexpected_error(errors::not_initialized, "source text_layout is not initialized");
    text_layout tl;
    auto hr = dwrite.factory()->CreateTextLayout(
      sv.data(), UINT(sv.size()), tf, source._p->GetMaxWidth(), source._p->GetMaxHeight(), &tl._p.get());
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateTextLayout failed", int32_t(hr));
    return std::move(tl);
  }

  /// creates text layout easily
  static std::expected<text_layout, error_trace> create(stringable<wchar_t> auto&& text) {
    if (auto res = dwrite.initialize(); !res) return unexpected_error(res.error());
    auto sv = std::wstring_view(text);
    comptr<IDWriteTextLayout> tl;
    const float lv = 1e6f;
    auto hr = dwrite.factory()->CreateTextLayout(sv.data(), UINT(sv.size()), dwrite.text_format(), lv, lv, &tl.get());
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateTextLayout failed", int32_t(hr));
    DWRITE_TEXT_METRICS metrics{};
    if (auto hr = tl->GetMetrics(&metrics); FAILED(hr))
      return unexpected_error(errors::operation_failed, "GetMetrics failed", int32_t(hr));
    return create(text, tl.get(), float2(metrics.width, metrics.height));
  }

  /// creates text layout from another text format and adjusts the layout size to fit the text
  static std::expected<text_layout, error_trace> create(
    stringable<wchar_t> auto&& text, text_format_like auto&& source, is_none auto) {
    if (auto tl = create(text, source, float2(1e6f, 1e6f)); tl) {
      if (auto res = tl->metrics_size()) {
        return create(text, source, *res);
      } else return unexpected_error(res.error());
    } else return unexpected_error(tl.error());
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

  std::expected<DWRITE_FONT_WEIGHT, error_trace> font_weight() const {
    if (!_p) return unexpected_error(errors::not_initialized, "text_layout is not initialized");
    return static_cast<IDWriteTextFormat*>(_p.get())->GetFontWeight();
  }

  std::expected<DWRITE_FONT_STYLE, error_trace> font_style() const {
    if (!_p) return unexpected_error(errors::not_initialized, "text_layout is not initialized");
    return static_cast<IDWriteTextFormat*>(_p.get())->GetFontStyle();
  }

  std::expected<DWRITE_FONT_STRETCH, error_trace> font_stretch() const {
    if (!_p) return unexpected_error(errors::not_initialized, "text_layout is not initialized");
    return static_cast<IDWriteTextFormat*>(_p.get())->GetFontStretch();
  }

  std::expected<DWRITE_TEXT_ALIGNMENT, error_trace> text_alignment() const {
    if (!_p) return unexpected_error(errors::not_initialized, "text_layout is not initialized");
    return _p->GetTextAlignment();
  }
  std::expected<void, error_trace> text_alignment(DWRITE_TEXT_ALIGNMENT align) {
    if (!_p) return unexpected_error(errors::not_initialized, "text_layout is not initialized");
    if (internal::set_text_alignment(_p.get(), align)) return {};
    else return unexpected_error(errors::invalid_argument, "invalid text alignment value");
  }

  std::expected<DWRITE_PARAGRAPH_ALIGNMENT, error_trace> paragraph_alignment() const {
    if (!_p) return unexpected_error(errors::not_initialized, "text_layout is not initialized");
    return _p->GetParagraphAlignment();
  }
  std::expected<void, error_trace> paragraph_alignment(DWRITE_PARAGRAPH_ALIGNMENT align) {
    if (!_p) return unexpected_error(errors::not_initialized, "text_layout is not initialized");
    if (internal::set_paragraph_alignment(_p.get(), align)) return {};
    else return unexpected_error(errors::invalid_argument, "invalid paragraph alignment value");
  }

  std::expected<float2, error_trace> metrics_size() const {
    if (!_p) return unexpected_error(errors::not_initialized, "text_layout is not initialized");
    DWRITE_TEXT_METRICS metrics{};
    if (auto hr = _p->GetMetrics(&metrics); FAILED(hr))
      return unexpected_error(errors::operation_failed, "GetMetrics failed", int32_t(hr));
    return float2(metrics.width, metrics.height);
  }

  std::expected<void, error_trace> adjust_size() {
    if (!_p) return unexpected_error(errors::not_initialized, "text_layout is not initialized");
    _p->SetMaxWidth(1e6f);
    _p->SetMaxHeight(1e6f);
    if (auto size = metrics_size()) {
      _p->SetMaxWidth(size->x);
      _p->SetMaxHeight(size->y);
      return {};
    } else return unexpected_error(size.error());
  }

  std::expected<std::vector<float4>, error_trace> hit_test_range(uint2 Range, float2 Origin = {}) const {
    /// \note This function ignores maxBidiReorderingDepth.
    if (!_p) return unexpected_error(errors::not_initialized, "text_layout is not initialized");
    if (Range.y <= Range.x) return std::vector<float4>{}; // empty range
    const auto length = Range.y - Range.x;
    uint32_t actual_count = 0;
    auto hr = _p->HitTestTextRange(Range.x, length, Origin.x, Origin.y, nullptr, 0, &actual_count);
    if (hr != E_NOT_SUFFICIENT_BUFFER) return unexpected_error(errors::operation_failed, "HitTestTextRange failed", int32_t(hr));
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

template<text_layout_like TL> std::expected<void, error_trace> draw_text(float2 pos, TL&& text_layout) {
  if (auto res = dwrite.initialize(); !res) return unexpected_error(res.error());
  if (!drawing::d2d_drawing()) return unexpected_error(errors::invalid_operation, "drawing not begun");
  const auto tl = static_cast<IDWriteTextLayout*>(text_layout);
  d2d.context()->DrawTextLayout(D2D1_POINT_2F{pos.x, pos.y}, tl, brush.brush());
  return {};
}
} // namespace yw
