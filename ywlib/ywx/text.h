#pragma once
#include "ywx/core.h"
#include "ywx/unknown.h"

namespace yw {

class text {
  comptr<IDWriteTextLayout> _text_layout;
  std::wstring _string;
  yw::color _color;
  font_config _font = yw::font_config::default_;
  yw::text_alignment _text_alignment = yw::text_alignment::left;
  ui::alignment _block_alignment = ui::alignment::center;
  float2 _layout_size;
  bool _dirty = false; // alignment and/or color changed
  bool _messy = false; // layout is changed

  std::expected<void, error_trace> _update_font_name() {
    if (!_text_layout) return unexpected_error(errors::not_initialized, "Not initialized");
    if (auto res = dwrite.initialize(); !res) return unexpected_error(res.error());
    IDWriteTextFormat* tfp = static_cast<IDWriteTextFormat*>(_text_layout.get());
    const auto n = tfp->GetFontFamilyNameLength();
    std::wstring font_name(n, L'\0');
    if (const auto hr = tfp->GetFontFamilyName(font_name.data(), n + 1); FAILED(hr))
      return unexpected_error(errors::operation_failed, "GetFontFamilyName failed", int(hr));
    _font.name = std::move(font_name);
    return {};
  }

  std::expected<void, error_trace> _update_layout_size() {
    if (!_text_layout) return unexpected_error(errors::not_initialized, "Not initialized");
    DWRITE_TEXT_METRICS metrics;
    if (const auto hr = _text_layout->GetMetrics(&metrics); FAILED(hr))
      return unexpected_error(errors::operation_failed, "GetMetrics failed", int(hr));
    _layout_size = float2(metrics.width, metrics.height);
    return {};
  }

  std::expected<void, error_trace> _update_text_alignment() {
    if (!_text_layout) return {};
    const auto hr = _text_layout->SetTextAlignment(static_cast<DWRITE_TEXT_ALIGNMENT>(_text_alignment));
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "SetTextAlignment failed", int(hr));
    return {};
  }

  std::expected<void, error_trace> _update_text_layout() {
    if (auto res = dwrite.initialize(); !res) return unexpected_error(res.error());
    comptr<IDWriteTextFormat> tfp{};
    auto hr = dwrite.factory()->CreateTextFormat(
      _font.name->c_str(), nullptr, DWRITE_FONT_WEIGHT(*_font.weight), DWRITE_FONT_STYLE(*_font.style),
      DWRITE_FONT_STRETCH(*_font.stretch), *_font.size, L"", &tfp.get());
    if (FAILED(hr) || !tfp) return unexpected_error(errors::operation_failed, "CreateTextFormat failed", int(hr));
    IDWriteTextLayout* tlp = nullptr;
    auto hr = dwrite.factory()->CreateTextLayout(_string.c_str(), UINT(_string.size()), tfp.get(), 1e6, 1e6, &tlp);
    if (FAILED(hr) || !tlp) return unexpected_error(errors::operation_failed, "CreateTextLayout failed", int(hr));
    _text_layout.reset(tlp);
    if (auto res = _update_text_alignment(); !res) return unexpected_error(res.error());
    if (auto res = _update_font_name(); !res) return unexpected_error(res.error());
    if (auto res = _update_layout_size(); !res) return unexpected_error(res.error());
    return {};
  }

public:
  IDWriteTextLayout* dwrite_text_layout() { return _text_layout.get(); }

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

  const auto& text_alignment() const noexcept { return _text_alignment; }
  auto& text_alignment(yw::text_alignment Alignment) noexcept {
    _text_alignment = Alignment;
    _update_text_alignment(); // message will be printed if failed
    _dirty = true;
    return *this;
  }

  const auto& block_alignment() const noexcept { return _block_alignment; }
  auto& block_alignment(yw::ui::alignment Alignment) noexcept {
    _block_alignment = Alignment;
    _dirty = true;
    return *this;
  }

  float2 layout_size() const noexcept { return _layout_size; }
  bool dirty() const noexcept { return _dirty; }
  bool messy() const noexcept { return _messy; }
  /// \note layout_size is determined by the current text layout

  /// explicitly updates text layout
  std::expected<void, error_trace> update() {
    if (!_messy && _text_layout) return {};
    if (auto res = _update_text_layout(); !res) return unexpected_error(res.error());
    _dirty = true, _messy = false;
    return {};
  }

  /// draws text at specified position. layout will be updated if necessary
  /// \note `block_alignment` is ignored
  std::expected<void, error_trace> draw(float2 Pos) {
    if (_string.empty() || !_text_layout) return {};
    if (auto res = update(); !res) return unexpected_error(res.error());
    brush.color(_color);
    if (auto res = draw_text(Pos, _text_layout.get()); !res) return unexpected_error(res.error());
    return {};
  }

  /// draws text at specfied area. layout will be updated if necessary
  std::expected<void, error_trace> draw(float2 Pos, float2 Area) {
    if (_string.empty() || !_text_layout) return {};
    if (auto res = update(); !res) return unexpected_error(res.error());
    constexpr float c[]{0.5f, 0.0f, 1.0f};
    const float2 cc{c[unsigned(_block_alignment) % 3], c[unsigned(_block_alignment) / 3 % 3]};
    const auto origin = Pos + cc * (Area - _layout_size);
    brush.color(_color);
    if (auto res = draw_text(origin, _text_layout.get()); !res) return unexpected_error(res.error());
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
