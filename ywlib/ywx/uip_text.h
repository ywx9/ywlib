#pragma once
#include "ywx/ui_parts.h"

namespace yw::ui::parts {

//////////////////////////////////////// MARK: text

struct text : public base {
  std::wstring string = L"";
  font_config font = yw::font_config::default_;
  text_alignment alignment = yw::text_alignment::left;
  ui::alignment block_alignment = ui::alignment::center;
  yw::color color = colors::black;
  float4 padding = float4::fill(arbitrary_value);
  float2 layout_size{};
  comptr<IDWriteTextLayout> text_layout;
  bool text_layout_changed = false;
  bool text_alignment_changed = false;

  std::expected<void, error_trace> set_font_config(const yw::font_config& fc) {
    if (fc.name.has_value()) font.name = *fc.name;
    if (fc.weight.has_value()) font.weight = *fc.weight;
    if (fc.style.has_value()) font.style = *fc.style;
    if (fc.stretch.has_value()) font.stretch = *fc.stretch;
    if (fc.size.has_value()) font.size = *fc.size;
    return {};
  }

  class accessor : public base::accessor<text> {
  public:
    ~accessor() {
      if (part.text_layout_changed) {
        if (auto res = part.rebuild_text_layout(); !res) fatal_error(res.error());
        part.layout_changed = true;
      } else if (part.text_alignment_changed) {
        if (auto res = part.apply_text_alignment(); !res) fatal_error(res.error());
        part.view_changed = true;
      }
      part.text_layout_changed = part.text_alignment_changed = false;
    }

    auto layout_size() const { return part.layout_size; }

    const auto& string() const { return part.string; }
    auto& string(std::wstring String) {
      part.string = std::move(String);
      part.text_layout_changed = true;
      return *this;
    }
    const auto& font() const { return part.font; }
    auto& font(const font_config& Font) {
      part.set_font_config(Font);
      part.text_layout_changed = true;
      return *this;
    }
    const auto& alignment() const { return part.alignment; }
    auto& alignment(text_alignment Alignment) {
      part.alignment = Alignment;
      part.text_alignment_changed = true;
      return *this;
    }
    const auto& block_alignment() const { return part.block_alignment; }
    auto& block_alignment(ui::alignment BlockAlignment) {
      part.block_alignment = BlockAlignment;
      part.text_layout_changed = true;
      return *this;
    }
    const auto& color() const { return part.color; }
    auto& color(yw::color Color) {
      part.color = Color;
      part.text_layout_changed = true;
      return *this;
    }
    const auto& padding() const { return part.padding; }
    auto& padding(float4 Padding) {
      part.padding = Padding;
      part.text_layout_changed = true;
      return *this;
    }

    const auto& font_name() const { return part.font.name; }
    auto& font_name(std::wstring FontName) { return font({.name = std::move(FontName)}); }

    const auto& font_size() const { return part.font.size; }
    auto& font_size(float FontSize) { return font({.size = FontSize}); }

    const auto& font_weight() const { return part.font.weight; }
    auto& font_weight(yw::font_weight FontWeight) { return font({.weight = FontWeight}); }

    const auto& font_style() const { return part.font.style; }
    auto& font_style(yw::font_style FontStyle) { return font({.style = FontStyle}); }

    const auto& font_stretch() const { return part.font.stretch; }
    auto& font_stretch(yw::font_stretch FontStretch) { return font({.stretch = FontStretch}); }
  };

  accessor access() & noexcept { return {*this}; }

  float2 bounds() const noexcept { return layout_size + padding.xy() + padding.zw(); }

  void apply_font_defaults() {
    font.name = font.name.value_or(L"");
    font.size = font.size.value_or(16.0f);
    font.weight = font.weight.value_or(font_weight::normal);
    font.style = font.style.value_or(font_style::normal);
    font.stretch = font.stretch.value_or(font_stretch::normal);
  }

  std::expected<void, error_trace> update_font_name() {
    if (!text_layout) return unexpected_error(errors::not_initialized, "Not initialized");
    IDWriteTextFormat* tfp = static_cast<IDWriteTextFormat*>(text_layout.get());
    const auto n = tfp->GetFontFamilyNameLength();
    std::wstring font_name(n, L'\0');
    if (const auto hr = tfp->GetFontFamilyName(font_name.data(), n + 1); FAILED(hr))
      return unexpected_error(errors::operation_failed, "GetFontFamilyName failed", int(hr));
    font.name = std::move(font_name);
    return {};
  }

  std::expected<void, error_trace> update_layout_size() {
    if (!text_layout) return unexpected_error(errors::not_initialized, "Not initialized");
    DWRITE_TEXT_METRICS metrics;
    if (const auto hr = text_layout->GetMetrics(&metrics); FAILED(hr))
      return unexpected_error(errors::operation_failed, "GetMetrics failed", int(hr));
    layout_size = float2(metrics.width, metrics.height);
    return {};
  }

  std::expected<void, error_trace> apply_text_alignment() {
    if (!text_layout) return {};
    if (const auto hr = text_layout->SetTextAlignment(static_cast<DWRITE_TEXT_ALIGNMENT>(alignment)); FAILED(hr))
      return unexpected_error(errors::operation_failed, "SetTextAlignment failed", int(hr));
    return {};
  }

  std::expected<void, error_trace> rebuild_text_layout() {
    if (auto res = dwrite.initialize(); !res) return unexpected_error(res.error());
    apply_font_defaults();
    {
      comptr<IDWriteTextFormat> tfp{};
      const auto hr = dwrite.factory()->CreateTextFormat(
        font.name->c_str(), nullptr, static_cast<DWRITE_FONT_WEIGHT>(*font.weight),
        static_cast<DWRITE_FONT_STYLE>(*font.style), static_cast<DWRITE_FONT_STRETCH>(*font.stretch), *font.size, L"",
        &tfp.get());
      if (FAILED(hr) || !tfp) return unexpected_error(errors::operation_failed, "CreateTextFormat failed", int(hr));
      tfp->SetTextAlignment(static_cast<DWRITE_TEXT_ALIGNMENT>(alignment));
      {
        IDWriteTextLayout* tlp = nullptr;
        const auto hr =
          dwrite.factory()->CreateTextLayout(string.c_str(), UINT(string.size()), tfp.get(), 1e6, 1e6, &tlp);
        if (FAILED(hr) || !tlp) return unexpected_error(errors::operation_failed, "CreateTextLayout failed", int(hr));
        text_layout.reset(tlp);
      }
    }
    if (auto res = update_font_name(); !res) return unexpected_error(res.error());
    if (auto res = update_layout_size(); !res) return unexpected_error(res.error());
    return {};
  }

  std::expected<void, error_trace> initialize() {
    if (text_layout_changed || !text_layout) {
      if (auto res = rebuild_text_layout(); !res) return unexpected_error(res.error());
    } else if (text_alignment_changed) {
      if (auto res = apply_text_alignment(); !res) return unexpected_error(res.error());
    } else return {};
    text_layout_changed = text_alignment_changed = false;
    return {};
  }

  std::expected<void, error_trace> draw(float2 Pos, float2 Area) {
    if (string.empty()) return {}; // nothing to draw
    if (!text_layout) return unexpected_error(errors::not_initialized, "Not initialized");
    static const float c[] = {0.5f, 0.0f, 1.0f};
    if (color.a > 0.0f) {
      brush.color(color);
      const auto cc = float2(c[unsigned(block_alignment) % 3], c[(unsigned(block_alignment) / 3) % 3]);
      const auto origin = Pos + padding.xy() + (Area - bounds()) * cc;
      if (auto res = draw_text(origin, text_layout.get()); !res) return unexpected_error(res.error());
    }
    return {};
  }

  std::expected<float4, error_trace> hittest(uint1 Position, bool Trailing = false) const {
    if (!text_layout) return unexpected_error(errors::not_initialized, "Not initialized");
    DWRITE_HIT_TEST_METRICS metrics{};
    float2 pt{};
    const auto hr = text_layout->HitTestTextPosition(Position.x, Trailing, &pt.x, &pt.y, &metrics);
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "HitTestTextPosition failed", int(hr));
    return float4(metrics.left, metrics.top, metrics.width, metrics.height);
  }

  std::expected<yw::tuple<uint32_t, float2>, error_trace> hittest(float2 Point) const {
    if (!text_layout) return unexpected_error(errors::not_initialized, "Not initialized");
    DWRITE_HIT_TEST_METRICS metrics{};
    BOOL inside = FALSE, trailing = FALSE;
    const auto hr = text_layout->HitTestPoint(Point.x, Point.y, &inside, &trailing, &metrics);
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "HitTestPoint failed", int(hr));
    return yw::tuple<uint32_t, float2>{metrics.textPosition + trailing, float2(metrics.width, metrics.height)};
  }

  std::expected<std::vector<float4>, error_trace> hittest_range(uint2 Range, float2 Origin = {}) const {
    if (!text_layout) return unexpected_error(errors::not_initialized, "Not initialized");
    if (Range.x >= Range.y) return unexpected_error(errors::invalid_argument, "Invalid range");
    const auto length = Range.y - Range.x;
    uint32_t count = 0;
    auto hr = text_layout->HitTestTextRange(Range.x, length, Origin.x, Origin.y, nullptr, 0, &count);
    if (hr != E_NOT_SUFFICIENT_BUFFER)
      return unexpected_error(errors::operation_failed, "HitTestTextRange failed", int32_t(hr));
    std::vector<DWRITE_HIT_TEST_METRICS> metrics(count);
    hr = text_layout->HitTestTextRange(Range.x, length, Origin.x, Origin.y, metrics.data(), count, &count);
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "HitTestTextRange failed", int32_t(hr));
    std::vector<float4> rects;
    rects.reserve(count);
    for (const auto& m : metrics) rects.emplace_back(m.left, m.top, m.width, m.height);
    return rects;
  }
};
} // namespace yw::ui::parts
