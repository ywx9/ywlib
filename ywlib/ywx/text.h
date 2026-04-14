#pragma once
#include "ywx/text_format.h"

namespace yw {

class text {
  comptr<IDWriteTextLayout> _p{};
  DWRITE_TEXT_METRICS _metrics{};
  std::wstring _text{};

  IDWriteTextFormat* _text_format() const { return static_cast<IDWriteTextFormat*>(_p.get()); }

public:
  text() noexcept = default;
  explicit operator bool() const noexcept { return static_cast<bool>(_p); }
  explicit operator IDWriteTextFormat*() const& noexcept { return _text_format(); }
  explicit operator IDWriteTextLayout*() const& noexcept { return _p.get(); }

  template<stringable S> text& operator=(S&& Text) {
    if (!*this) *this = assume(create(static_cast<S&&>(Text)));
    else this->operator()(static_cast<S&&>(Text));
    return *this;
  }

  template<stringable S> static std::expected<text, error_trace> create(S&& Text) {
    if (auto res = dwrite.initialize(); !res) return unexpected_error(res.error());
    text t{};
    t._text = unicode<wchar_t>(static_cast<S&&>(Text));
    auto hr =
      dwrite.factory()->CreateTextLayout(t._text.data(), UINT(t._text.size()), dwrite.text_format(), 0, 0, &t._p.get());
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateTextLayout failed", int32_t(hr));
    if (auto hr = t._p->GetMetrics(&t._metrics); FAILED(hr))
      return unexpected_error(errors::operation_failed, "GetMetrics failed", int32_t(hr));
    t._p->SetMaxWidth(t._metrics.width);
    return std::move(t);
  }

  template<stringable S> static std::expected<text, error_trace> create(S&& Text, const text_format_like auto& Format) {
    if (auto res = dwrite.initialize(); !res) return unexpected_error(res.error());
    text t{};
    t._text = unicode<wchar_t>(static_cast<S&&>(Text));
    auto hr = dwrite.factory()->CreateTextLayout(
      t._text.data(), UINT(t._text.size()), static_cast<IDWriteTextFormat*>(Format), 0, 0, &t._p.get());
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateTextLayout failed", int32_t(hr));
    if (auto hr = t._p->GetMetrics(&t._metrics); FAILED(hr))
      return unexpected_error(errors::operation_failed, "GetMetrics failed", int32_t(hr));
    t._p->SetMaxWidth(t._metrics.width);
    return std::move(t);
  }

  template<stringable S> static std::expected<text, error_trace> create(
    S&& Text, null_terminated<wchar_t> FontName, std::optional<float> FontSize = {},
    std::optional<yw::font_weight> FontWeight = {}, std::optional<yw::font_style> FontStyle = {},
    std::optional<yw::font_stretch> FontStretch = {}) {
    comptr<IDWriteTextFormat> tf{};
    if (auto res = dwrite.initialize(); !res) return unexpected_error(res.error());
    if (FontName.empty()) FontName = L"";
    if (!FontSize) FontSize = 16.0f;
    if (!FontWeight) FontWeight = yw::font_weight::regular;
    if (!FontStyle) FontStyle = yw::font_style::normal;
    if (!FontStretch) FontStretch = yw::font_stretch::normal;
    auto hr = dwrite.factory()->CreateTextFormat(
      FontName.data(), nullptr, DWRITE_FONT_WEIGHT(*FontWeight), DWRITE_FONT_STYLE(*FontStyle),
      DWRITE_FONT_STRETCH(*FontStretch), *FontSize, L"", &tf.get());
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateTextFormat failed", int32_t(hr));
    tf->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
    text t{};
    t._text = unicode<wchar_t>(static_cast<S&&>(Text));
    hr = dwrite.factory()->CreateTextLayout(t._text.data(), UINT(t._text.size()), tf.get(), 0, 0, &t._p.get());
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateTextLayout failed", int32_t(hr));
    if (auto hr = t._p->GetMetrics(&t._metrics); FAILED(hr))
      return unexpected_error(errors::operation_failed, "GetMetrics failed", int32_t(hr));
    t._p->SetMaxWidth(t._metrics.width);
    return std::move(t);
  }

  bool empty() const { return _text.empty(); }
  float2 size() const { return {_metrics.width, _metrics.height}; }

  std::wstring font_name() const {
    if (!_p) return {};
    if (auto res = internal::get_font_name(_text_format()); !res) return {};
    else return std::move(*res);
  }
  float font_size() const { return _p ? _text_format()->GetFontSize() : 0.0f; }
  yw::font_weight font_weight() const {
    return _p ? yw::font_weight(_text_format()->GetFontWeight()) : yw::font_weight{};
  }
  yw::font_style font_style() const { return _p ? yw::font_style(_text_format()->GetFontStyle()) : yw::font_style{}; }
  yw::font_stretch font_stretch() const {
    return _p ? yw::font_stretch(_text_format()->GetFontStretch()) : yw::font_stretch{};
  }

  yw::text_alignment text_alignment() const {
    return _p ? yw::text_alignment(_text_format()->GetTextAlignment()) : yw::text_alignment{};
  }
  void text_alignment(yw::text_alignment Align) {
    if (_p) _text_format()->SetTextAlignment(static_cast<DWRITE_TEXT_ALIGNMENT>(Align));
  }

  std::expected<void, error_trace> font_conf(
    null_terminated<wchar_t> name = {}, std::optional<float> size = {}, std::optional<yw::font_weight> weight = {},
    std::optional<yw::font_style> style = {}, std::optional<yw::font_stretch> stretch = {}) {
    if (!_p) return unexpected_error(errors::operation_failed, "Text layout not initialized");
    if (name.empty()) name = font_name();
    if (!size) size = font_size();
    if (!weight) weight = font_weight();
    if (!style) style = font_style();
    if (!stretch) stretch = font_stretch();
    if (auto res = create(_text, std::move(name), *size, *weight, *style, *stretch)) *this = std::move(*res);
    else return unexpected_error(res.error());
    return {};
  }

  std::expected<float4, error_trace> hit_test(uint1 text_position, bool is_trailing = false) const {
    if (!_p) return unexpected_error(errors::not_initialized, "text is not initialized");
    DWRITE_HIT_TEST_METRICS metrics{};
    float2 pt{};
    auto hr = _p->HitTestTextPosition(text_position.x, is_trailing, &pt.x, &pt.y, &metrics);
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "HitTestTextPosition failed", int32_t(hr));
    return float4(metrics.left, metrics.top, metrics.width, metrics.height);
  }

  /// returns text position at the specified point
  std::expected<uint32_t, error_trace> hit_test(float2 point) const {
    if (!_p) return unexpected_error(errors::not_initialized, "text is not initialized");
    DWRITE_HIT_TEST_METRICS metrics{};
    BOOL is_inside = FALSE, is_trailing = FALSE;
    auto hr = _p->HitTestPoint(point.x, point.y, &is_inside, &is_trailing, &metrics);
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "HitTestPoint failed", int32_t(hr));
    return metrics.textPosition + uint32_t(is_trailing);
  }

  /// returns rectangles `{left, top, width, height}` for each run in the specified text range
  std::expected<std::vector<float4>, error_trace> hit_test_range(uint2 Range, float2 Origin = {}) const {
    if (!_p) return unexpected_error(errors::not_initialized, "text is not initialized");
    if (Range.y <= Range.x) return std::vector<float4>{};
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

  const std::wstring& operator()() const { return _text; }

  template<stringable S> std::expected<void, error_trace> operator()(S&& NewText) {
    _text = unicode<wchar_t>(static_cast<S&&>(NewText));
    if (auto res = create(_text, font_name(), font_size(), font_weight(), font_style(), font_stretch()))
      *this = std::move(*res);
    else return unexpected_error(res.error());
    return {};
  }
};

inline std::expected<void, error_trace> draw_text(float2 Pos, const text& Text) {
  if (auto res = dwrite.initialize(); !res) return unexpected_error(res.error());
  if (!Text) return unexpected_error(errors::operation_failed, "Text not initialized");
  d2d.context()->DrawTextLayout(
    std::bit_cast<D2D1_POINT_2F>(Pos), static_cast<IDWriteTextLayout*>(Text), brush.d2d_brush());
  return {};
}

/// draws `Text` at `Pos`, clipping to the rectangle `[Pos, Pos + Size)`
inline std::expected<void, error_trace> draw_text(float2 Pos, float2 Size, const text& Text) {
  if (auto res = dwrite.initialize(); !res) return unexpected_error(res.error());
  if (!drawing::d2d_drawing()) return unexpected_error(errors::invalid_operation, "drawing not begun");
  if (!Text) return unexpected_error(errors::operation_failed, "Text not initialized");
  const D2D1_RECT_F rect{Pos.x, Pos.y, Pos.x + Size.x, Pos.y + Size.y};
  d2d.context()->PushAxisAlignedClip(&rect, D2D1_ANTIALIAS_MODE_ALIASED);
  d2d.context()->DrawTextLayout(
    std::bit_cast<D2D1_POINT_2F>(Pos), static_cast<IDWriteTextLayout*>(Text), brush.d2d_brush());
  d2d.context()->PopAxisAlignedClip();
  return {};
}

/// draws `Text` at `Pos`, clipping to the rectangle `[ClipPos, ClipPos + ClipSize)`
inline std::expected<void, error_trace> draw_text(float2 Pos, float2 ClipPos, float2 ClipSize, const text& Text) {
  if (auto res = dwrite.initialize(); !res) return unexpected_error(res.error());
  if (!drawing::d2d_drawing()) return unexpected_error(errors::invalid_operation, "drawing not begun");
  if (!Text) return unexpected_error(errors::operation_failed, "Text not initialized");
  const D2D1_RECT_F rect{ClipPos.x, ClipPos.y, ClipPos.x + ClipSize.x, ClipPos.y + ClipSize.y};
  d2d.context()->PushAxisAlignedClip(&rect, D2D1_ANTIALIAS_MODE_ALIASED);
  d2d.context()->DrawTextLayout(
    std::bit_cast<D2D1_POINT_2F>(Pos), static_cast<IDWriteTextLayout*>(Text), brush.d2d_brush());
  d2d.context()->PopAxisAlignedClip();
  return {};
}
} // namespace yw
