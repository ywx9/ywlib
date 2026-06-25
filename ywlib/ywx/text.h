#pragma once
#include "ywx/control.h"

namespace yw {

class text {
  comptr<IDWriteTextLayout> _layout;
  std::wstring _string;
  yw::color _color = colors::black;
  font_config _font = yw::font_config::default_;
  yw::alignment _alignment = yw::alignment::left;
  float2 _bounds{};
  mutable bool _dirty = false;
  mutable bool _messy = false;

  static DWRITE_TEXT_ALIGNMENT dwrite_alignment(yw::alignment alignment) noexcept {
    switch (alignment) {
    case yw::alignment::left:
    case yw::alignment::left_top:
    case yw::alignment::left_bottom:
      return DWRITE_TEXT_ALIGNMENT_LEADING;
    case yw::alignment::right:
    case yw::alignment::right_top:
    case yw::alignment::right_bottom:
      return DWRITE_TEXT_ALIGNMENT_TRAILING;
    default:
      return DWRITE_TEXT_ALIGNMENT_CENTER;
    }
  }

  std::expected<void, error> update_font_name(IDWriteTextFormat* format) {
    if (!format) return std::unexpected(error(errors::not_initialized, "Text format is not initialized"));
    const auto length = format->GetFontFamilyNameLength();
    std::wstring font_name(static_cast<size_t>(length) + 1, L'\0');
    hresult_test(format->GetFontFamilyName, font_name.data(), UINT(font_name.size()));
    font_name.resize(length);
    _font.name = std::move(font_name);
    return {};
  }

  std::expected<void, error> calculate_bounds() {
    if (!_layout) return std::unexpected(error(errors::not_initialized, "Text layout is not initialized"));
    DWRITE_TEXT_METRICS metrics{};
    hresult_test(_layout->GetMetrics, &metrics);
    _bounds = float2(metrics.width, metrics.height);
    return {};
  }

  std::expected<void, error> reset_align() {
    if (!_layout) return {};
    hresult_test(_layout->SetTextAlignment, dwrite_alignment(_alignment));
    return {};
  }

  std::expected<void, error> update_text_layout() {
    comptr<IDWriteTextFormat> format{};
    const auto font_name = _font.name.value_or(std::wstring());
    const auto font_weight = _font.weight.value_or(yw::font_weight::normal);
    const auto font_style = _font.style.value_or(yw::font_style::normal);
    const auto font_stretch = _font.stretch.value_or(yw::font_stretch::normal);
    const auto font_size = _font.size.value_or(font_config::default_.size.value_or(16.0f));
    hresult_test(
      dwrite::factory()->CreateTextFormat, font_name.c_str(), nullptr, DWRITE_FONT_WEIGHT(font_weight),
      DWRITE_FONT_STYLE(font_style), DWRITE_FONT_STRETCH(font_stretch), font_size, L"", &format.get());
    IDWriteTextLayout* layout = nullptr;
    hresult_test(
      dwrite::factory()->CreateTextLayout, _string.c_str(), UINT(_string.size()), format.get(), 1e6f, 1e6f, &layout);
    _layout.reset(layout);
    if (auto res = reset_align(); !res) return res.error().relay();
    if (auto res = update_font_name(format.get()); !res) return res.error().relay();
    if (auto res = calculate_bounds(); !res) return res.error().relay();
    return {};
  }

public:
  IDWriteTextLayout* dwrite_text_layout() const noexcept { return _layout.get(); }

  const auto& string() const noexcept { return _string; }
  auto& string(std::wstring String) noexcept {
    _string = std::move(String);
    _messy = true;
    return *this;
  }

  const yw::color& color() const noexcept { return _color; }
  auto& color(yw::color Color) noexcept {
    _color = Color;
    _dirty = true;
    return *this;
  }

  const auto& font() const noexcept { return _font; }
  auto& font(yw::font_config Font) noexcept {
    if (Font.name.has_value()) _font.name = std::move(*Font.name);
    if (Font.weight.has_value()) _font.weight = *Font.weight;
    if (Font.style.has_value()) _font.style = *Font.style;
    if (Font.stretch.has_value()) _font.stretch = *Font.stretch;
    if (Font.size.has_value()) _font.size = *Font.size;
    _messy = true;
    return *this;
  }

  const auto& font_name() const noexcept {
    if (_font.name.has_value()) return *_font.name;
    return yw::empty_string<wchar_t>;
  }
  auto& font_name(std::wstring FontName) noexcept { return font({.name = std::move(FontName)}); }

  auto font_weight() const noexcept { return _font.weight.value_or(yw::font_weight::normal); }
  auto& font_weight(yw::font_weight FontWeight) noexcept { return font({.weight = FontWeight}); }

  auto font_style() const noexcept { return _font.style.value_or(yw::font_style::normal); }
  auto& font_style(yw::font_style FontStyle) noexcept { return font({.style = FontStyle}); }

  auto font_stretch() const noexcept { return _font.stretch.value_or(yw::font_stretch::normal); }
  auto& font_stretch(yw::font_stretch FontStretch) noexcept { return font({.stretch = FontStretch}); }

  auto font_size() const noexcept { return _font.size.value_or(0.0f); }
  auto& font_size(float FontSize) noexcept { return font({.size = FontSize}); }

  const auto& alignment() const noexcept { return _alignment; }
  auto& alignment(yw::alignment Alignment) noexcept {
    _alignment = Alignment;
    _dirty = true;
    return *this;
  }

  float2 bounds() const noexcept { return _bounds; }
  bool dirty() const noexcept { return _dirty; }
  bool messy() const noexcept { return _messy; }

  std::expected<void, error> update() {
    if (_messy || !_layout) {
      if (auto res = update_text_layout(); !res) return res.error().relay();
      _dirty = true;
      _messy = false;
      return {};
    }
    if (_dirty) {
      if (auto res = reset_align(); !res) return res.error().relay();
    }
    return {};
  }

  std::expected<void, error> draw(float2 Pos) const {
    if (_string.empty() || !_layout) return {};
    brush::color(_color);
    if (auto res = draw_text(Pos, _layout.get()); !res) return res.error().relay();
    _dirty = false;
    return {};
  }

  std::expected<float4, error> hittest(uint1 Index, bool Trailing = false) const {
    if (!_layout) return std::unexpected(error(errors::not_initialized, "Text layout is not initialized"));
    DWRITE_HIT_TEST_METRICS metrics{};
    float2 pt{};
    hresult_test(_layout->HitTestTextPosition, Index.x, Trailing, &pt.x, &pt.y, &metrics);
    return float4(metrics.left, metrics.top, metrics.width, metrics.height);
  }

  std::expected<yw::tuple<uint32_t, float2>, error> hittest(float2 Pt) const {
    if (!_layout) return std::unexpected(error(errors::not_initialized, "Text layout is not initialized"));
    DWRITE_HIT_TEST_METRICS metrics{};
    BOOL inside = FALSE;
    BOOL trailing = FALSE;
    hresult_test(_layout->HitTestPoint, Pt.x, Pt.y, &inside, &trailing, &metrics);
    return yw::tuple<uint32_t, float2>{metrics.textPosition + bool(trailing), float2(metrics.width, metrics.height)};
  }

  std::expected<std::vector<float4>, error> hittest_range(uint2 Range, float2 Origin = {}) const {
    if (!_layout) return std::unexpected(error(errors::not_initialized, "Text layout is not initialized"));
    if (Range.x >= Range.y) return std::unexpected(error(errors::invalid_argument, "Invalid range"));
    const auto length = Range.y - Range.x;
    uint32_t count = 0;
    auto hr = _layout->HitTestTextRange(Range.x, length, Origin.x, Origin.y, nullptr, 0, &count);
    if (hr != E_NOT_SUFFICIENT_BUFFER)
      return std::unexpected(error(errors::operation_failed, "HitTestTextRange failed", int32_t(hr)));
    std::vector<DWRITE_HIT_TEST_METRICS> metrics(count);
    hresult_test(_layout->HitTestTextRange, Range.x, length, Origin.x, Origin.y, metrics.data(), count, &count);
    std::vector<float4> rects;
    rects.reserve(count);
    for (size_t i = 0; i < count; ++i)
      rects.emplace_back(metrics[i].left, metrics[i].top, metrics[i].width, metrics[i].height);
    return rects;
  }
};
} // namespace yw
