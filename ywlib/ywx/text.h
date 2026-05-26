#pragma once
#include "ywx/core.h"

namespace yw {

class text {
  comptr<IDWriteTextLayout> _text_layout;
  float2 _layout_size;
  bool text_layout_changed = false;
  bool text_alignment_changed = false;

  std::expected<void, error_trace> update_font_name() {
    if (!_text_layout) return unexpected_error(errors::not_initialized, "Not initialized");
    if (auto res = dwrite.initialize(); !res) return unexpected_error(res.error());
    IDWriteTextFormat* tfp = static_cast<IDWriteTextFormat*>(_text_layout.get());
    const auto n = tfp->GetFontFamilyNameLength();
    std::wstring font_name(n, L'\0');
    if (const auto hr = tfp->GetFontFamilyName(font_name.data(), n + 1); FAILED(hr))
      return unexpected_error(errors::operation_failed, "GetFontFamilyName failed", int(hr));
    font.name = std::move(font_name);
    return {};
  }

  std::expected<void, error_trace> update_layout_size() {
    if (!_text_layout) return unexpected_error(errors::not_initialized, "Not initialized");
    DWRITE_TEXT_METRICS metrics;
    if (const auto hr = _text_layout->GetMetrics(&metrics); FAILED(hr))
      return unexpected_error(errors::operation_failed, "GetMetrics failed", int(hr));
    _layout_size = float2(metrics.width, metrics.height);
    return {};
  }

  std::expected<void, error_trace> update_layout_alignment() {
    if (!_text_layout) return {};
    const auto hr = _text_layout->SetTextAlignment(static_cast<DWRITE_TEXT_ALIGNMENT>(text_alignment));
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "SetTextAlignment failed", int(hr));
    return {};
  }

  std::expected<void, error_trace> update_text_layout() {
    if (auto res = dwrite.initialize(); !res) return unexpected_error(res.error());
    comptr<IDWriteTextFormat> tfp{};
    auto hr = dwrite.factory()->CreateTextFormat(
      font.name->c_str(), nullptr, DWRITE_FONT_WEIGHT(*font.weight), DWRITE_FONT_STYLE(*font.style),
      DWRITE_FONT_STRETCH(*font.stretch), *font.size, L"", &tfp.get());
    if (FAILED(hr) || !tfp) return unexpected_error(errors::operation_failed, "CreateTextFormat failed", int(hr));
    IDWriteTextLayout* tlp = nullptr;
    auto hr = dwrite.factory()->CreateTextLayout(string.c_str(), UINT(string.size()), tfp.get(), 1e6, 1e6, &tlp);
    if (FAILED(hr) || !tlp) return unexpected_error(errors::operation_failed, "CreateTextLayout failed", int(hr));
    _text_layout.reset(tlp);
    if (auto res = update_layout_alignment(); !res) return unexpected_error(res.error());
    if (auto res = update_font_name(); !res) return unexpected_error(res.error());
    if (auto res = update_layout_size(); !res) return unexpected_error(res.error());
    return {};
  }

public:
  std::wstring string;
  font_config font = yw::font_config::default_;
  yw::color color;
  yw::text_alignment text_alignment = yw::text_alignment::left;

  float2 layout_size() const noexcept { return _layout_size; }

  void set_font(const font_config& new_font) {
    if (new_font.name.has_value()) font.name = *new_font.name, text_layout_changed = true;
    if (new_font.weight.has_value()) font.weight = *new_font.weight, text_layout_changed = true;
    if (new_font.style.has_value()) font.style = *new_font.style, text_layout_changed = true;
    if (new_font.stretch.has_value()) font.stretch = *new_font.stretch, text_layout_changed = true;
    if (new_font.size.has_value()) font.size = *new_font.size, text_layout_changed = true;
  }

  std::expected<void, error_trace> update() {
    if (text_layout_changed || !_text_layout) {
      if (auto res = update_text_layout(); !res) return unexpected_error(res.error());
    } else if (text_alignment_changed) {
      if (auto res = update_layout_alignment(); !res) return unexpected_error(res.error());
    }
    text_layout_changed = text_alignment_changed = false;
    return {};
  }

  std::expected<void, error_trace> draw(float2 Pos) {
    if (string.empty() || !_text_layout) return {};
    if (auto res = draw_text(Pos, _text_layout.get()); !res) return unexpected_error(res.error());
    return {};
  }

  /// returns bounding box (xywh) of character at given index
  std::expected<float4, error_trace> hittest(uint1 Index, bool Trailing = false) const {
    if (!_text_layout) return unexpected_error(errors::not_initialized, "Text layout is not initialized");
    DWRITE_HIT_TEST_METRICS htm{};
    float2 pt{};
    const auto hr = _text_layout->HitTestTextPosition(Index.x, Trailing, &pt.x, &pt.y, &htm);
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "HitTestTextPosition failed", int(hr));
    return float4(htm.left, htm.top, htm.width, htm.height);
  }

  /// returns index and size of character at given point
  std::expected<yw::tuple<uint32_t, float2>, error_trace> hittest(float2 Pt) const {
    if (!_text_layout) return unexpected_error(errors::not_initialized, "Text layout is not initialized");
    DWRITE_HIT_TEST_METRICS htm{};
    BOOL inside{}, trailing{};
    const auto hr = _text_layout->HitTestPoint(Pt.x, Pt.y, &inside, &trailing, &htm);
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "HitTestPoint failed", int(hr));
    return yw::tuple<uint32_t, float2>(htm.textPosition + trailing, float2(htm.width, htm.height));
  }

  /// returns areas (multiple xywh) of string in given range
  std::expected<std::vector<float4>, error_trace> hittest_range(uint2 Range, float2 Origin = {}) const {
    if (!_text_layout) return unexpected_error(errors::not_initialized, "Text layout is not initialized");
    if (Range.x >= Range.y) return unexpected_error(errors::invalid_argument, "Invalid range");
    const auto length = Range.y - Range.x;
    uint32_t count = 0;
    auto hr = _text_layout->HitTestTextRange(Range.x, length, Origin.x, Origin.y, nullptr, 0, &count);
    if (hr != E_NOT_SUFFICIENT_BUFFER)
      return unexpected_error(errors::operation_failed, "HitTestTextRange failed", int32_t(hr));
    std::vector<DWRITE_HIT_TEST_METRICS> metrics(count);
    hr = _text_layout->HitTestTextRange(Range.x, length, Origin.x, Origin.y, metrics.data(), count, &count);
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "HitTestTextRange failed", int32_t(hr));
    std::vector<float4> rects;
    rects.reserve(count);
    for (const auto& m : metrics) rects.emplace_back(m.left, m.top, m.width, m.height);
    return rects;
  }
};
} // namespace yw
