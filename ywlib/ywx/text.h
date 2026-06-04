#pragma once
#include "ywx/core.h"
#include "ywx/ui_unknown.h"

namespace yw {

class text {
  comptr<IDWriteTextLayout> _layout;
  std::wstring _string;
  yw::color _color = colors::black;
  font_config _font = yw::font_config::default_;
  ui::alignment _alignment = ui::alignment::left;
  float2 _bounds;
  mutable bool _dirty = false; // alignment and/or color changed
  mutable bool _messy = false; // layout is changed

  std::expected<void, error_trace> _update_font_name() {
    if (!_layout) return unexpected_error(errors::not_initialized, "Not initialized");
    IDWriteTextFormat* tfp = static_cast<IDWriteTextFormat*>(_layout.get());
    const auto n = tfp->GetFontFamilyNameLength();
    std::wstring font_name(n, L'\0');
    if (const auto hr = tfp->GetFontFamilyName(font_name.data(), n + 1); FAILED(hr))
      return unexpected_error(errors::operation_failed, "GetFontFamilyName failed", int(hr));
    _font.name = std::move(font_name);
    return {};
  }

  std::expected<void, error_trace> _calculate_bounds() {
    if (!_layout) return unexpected_error(errors::not_initialized, "Not initialized");
    DWRITE_TEXT_METRICS metrics;
    if (const auto hr = _layout->GetMetrics(&metrics); FAILED(hr))
      return unexpected_error(errors::operation_failed, "GetMetrics failed", int(hr));
    _bounds = float2(metrics.width, metrics.height);
    return {};
  }

  std::expected<void, error_trace> _reset_align() {
    if (!_layout) return {};
    const auto hr = _layout->SetTextAlignment(to_dwrite_value(_alignment));
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "SetTextAlignment failed", int(hr));
    return {};
  }

  std::expected<void, error_trace> _update_text_layout() {
    comptr<IDWriteTextFormat> tfp{};
    const auto& dwrite = yw::dwrite();
    auto hr = dwrite.factory()->CreateTextFormat(
      _font.name->c_str(), nullptr, DWRITE_FONT_WEIGHT(*_font.weight), DWRITE_FONT_STYLE(*_font.style),
      DWRITE_FONT_STRETCH(*_font.stretch), *_font.size, L"", &tfp.get());
    if (FAILED(hr) || !tfp) return unexpected_error(errors::operation_failed, "CreateTextFormat failed", int(hr));
    IDWriteTextLayout* tlp = nullptr;
    hr = dwrite.factory()->CreateTextLayout(_string.c_str(), UINT(_string.size()), tfp.get(), 1e6, 1e6, &tlp);
    if (FAILED(hr) || !tlp) return unexpected_error(errors::operation_failed, "CreateTextLayout failed", int(hr));
    _layout.reset(tlp);
    if (auto res = _reset_align(); !res) return unexpected_error(res.error());
    if (auto res = _update_font_name(); !res) return unexpected_error(res.error());
    if (auto res = _calculate_bounds(); !res) return unexpected_error(res.error());
    return {};
  }

public:
  IDWriteTextLayout* dwrite_text_layout() { return _layout.get(); }

  const auto& string() const noexcept { return _string; }
  auto& string(std::wstring String) noexcept {
    _string = std::move(String);
    _messy = true;
    return *this;
  }

  const color& color() const noexcept { return _color; }
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
  auto& font_name(std::wstring FontName) { return font({.name = std::move(FontName)}); }

  auto font_weight() const noexcept { return _font.weight.value_or(yw::font_weight::normal); }
  auto& font_weight(yw::font_weight FontWeight) { return font({.weight = FontWeight}); }

  auto font_style() const noexcept { return _font.style.value_or(yw::font_style::normal); }
  auto& font_style(yw::font_style FontStyle) { return font({.style = FontStyle}); }

  auto font_stretch() const noexcept { return _font.stretch.value_or(yw::font_stretch::normal); }
  auto& font_stretch(yw::font_stretch FontStretch) { return font({.stretch = FontStretch}); }

  auto font_size() const noexcept { return _font.size.value_or(0.0f); }
  auto& font_size(float FontSize) { return font({.size = FontSize}); }

  const auto& alignment() const noexcept { return _alignment; }
  auto& alignment(ui::alignment Alignment) noexcept {
    _alignment = Alignment;
    _reset_align(); // message will be printed if failed
    _dirty = true;
    return *this;
  }

  float2 bounds() const noexcept { return _bounds; }
  bool dirty() const noexcept { return _dirty; }
  bool messy() const noexcept { return _messy; }

  /// updates text layout
  std::expected<void, error_trace> update() {
    if (!_messy && _layout) return {};
    if (auto res = _update_text_layout(); !res) return unexpected_error(res.error());
    _dirty = true, _messy = false;
    return {};
  }

  /// draws text at specified position
  /// \note `block_alignment` is ignored
  std::expected<void, error_trace> draw(float2 Pos) const {
    if (_string.empty() || !_layout) return {};
    brush().color(_color);
    if (auto res = draw_text(Pos, _layout.get()); !res) return unexpected_error(res.error());
    _dirty = false;
    return {};
  }

  /// returns bounding box (xywh) of character at given index
  std::expected<float4, error_trace> hittest(uint1 Index, bool Trailing = false) const {
    if (!_layout) return unexpected_error(errors::not_initialized, "Text layout is not initialized");
    DWRITE_HIT_TEST_METRICS htm{};
    float2 pt{};
    const auto hr = _layout->HitTestTextPosition(Index.x, Trailing, &pt.x, &pt.y, &htm);
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "HitTestTextPosition failed", int(hr));
    return float4(htm.left, htm.top, htm.width, htm.height);
  }

  /// returns index and size of character at given point
  std::expected<yw::tuple<uint32_t, float2>, error_trace> hittest(float2 Pt) const {
    if (!_layout) return unexpected_error(errors::not_initialized, "Text layout is not initialized");
    DWRITE_HIT_TEST_METRICS htm{};
    BOOL inside{}, trailing{};
    const auto hr = _layout->HitTestPoint(Pt.x, Pt.y, &inside, &trailing, &htm);
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "HitTestPoint failed", int(hr));
    return yw::tuple<uint32_t, float2>{htm.textPosition + bool(trailing), float2(htm.width, htm.height)};
  }

  /// returns areas (multiple xywh) of string in given range
  std::expected<std::vector<float4>, error_trace> hittest_range(uint2 Range, float2 Origin = {}) const {
    if (!_layout) return unexpected_error(errors::not_initialized, "Text layout is not initialized");
    if (Range.x >= Range.y) return unexpected_error(errors::invalid_argument, "Invalid range");
    const auto length = Range.y - Range.x;
    uint32_t count = 0;
    auto hr = _layout->HitTestTextRange(Range.x, length, Origin.x, Origin.y, nullptr, 0, &count);
    if (hr != E_NOT_SUFFICIENT_BUFFER)
      return unexpected_error(errors::operation_failed, "HitTestTextRange failed", int32_t(hr));
    std::vector<DWRITE_HIT_TEST_METRICS> metrics(count);
    hr = _layout->HitTestTextRange(Range.x, length, Origin.x, Origin.y, metrics.data(), count, &count);
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "HitTestTextRange failed", int32_t(hr));
    std::vector<float4> rects;
    rects.reserve(count);
    for (const auto& m : metrics) rects.emplace_back(m.left, m.top, m.width, m.height);
    return rects;
  }
};
} // namespace yw
