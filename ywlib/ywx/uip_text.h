#pragma once
#include "ywx/ui_parts.h"

namespace yw::ui::part {

//////////////////////////////////////// MARK: text

struct text {
  slotid control_id{};
  std::wstring string = L"";
  font_config font = yw::font_config::default_;
  text_alignment alignment = yw::text_alignment::left;
  ui::alignment block_alignment = ui::alignment::center;
  yw::color color = colors::black;
  float2 layout_size{};
  float2 offset{};
  comptr<IDWriteTextLayout> text_layout;
  bool view_changed = false;

  std::expected<void, error_trace> update_font_name() {
    if (auto res = initialize(); !res) return unexpected_error(res.error());
    IDWriteTextFormat* tfp = static_cast<IDWriteTextFormat*>(text_layout.get());
    const auto n = tfp->GetFontFamilyNameLength();
    std::wstring font_name(n, L'\0');
    if (const auto hr = tfp->GetFontFamilyName(font_name.data(), n + 1); FAILED(hr))
      return unexpected_error(errors::operation_failed, "GetFontFamilyName failed", int(hr));
    font.name = std::move(font_name);
    return {};
  }

  std::expected<void, error_trace> update_layout_size() {
    if (auto res = initialize(); !res) return unexpected_error(res.error());
    DWRITE_TEXT_METRICS metrics;
    if (const auto hr = text_layout->GetMetrics(&metrics); FAILED(hr))
      return unexpected_error(errors::operation_failed, "GetMetrics failed", int(hr));
    layout_size = float2(metrics.width, metrics.height);
    return {};
  }

  std::expected<void, error_trace> set_font_config(const yw::font_config& fc) {
    if (auto res = initialize(); !res) return unexpected_error(res.error());
    {
      IDWriteTextFormat* tfp;
      const auto hr = dwrite.factory()->CreateTextFormat( //
        fc.name.value_or(*font.name).c_str(), nullptr, //
        static_cast<DWRITE_FONT_WEIGHT>(fc.weight.value_or(*font.weight)), //
        static_cast<DWRITE_FONT_STYLE>(fc.style.value_or(*font.style)), //
        static_cast<DWRITE_FONT_STRETCH>(fc.stretch.value_or(*font.stretch)), //
        fc.size.value_or(*font.size), L"", &tfp);
      if (FAILED(hr) || !tfp) return unexpected_error(errors::operation_failed, "CreateTextFormat failed", int(hr));
      tfp->SetTextAlignment(static_cast<DWRITE_TEXT_ALIGNMENT>(alignment));
      {
        IDWriteTextLayout* tlp = nullptr;
        const auto hr = dwrite.factory()->CreateTextLayout(string.c_str(), UINT(string.size()), tfp, 1e6, 1e6, &tlp);
        if (FAILED(hr) || !tlp) return unexpected_error(errors::operation_failed, "CreateTextLayout failed", int(hr));
        text_layout.reset(tlp);
      }
    }
    if (fc.weight.has_value()) font.weight = *fc.weight;
    if (fc.style.has_value()) font.style = *fc.style;
    if (fc.stretch.has_value()) font.stretch = *fc.stretch;
    if (fc.size.has_value()) font.size = *fc.size;
    if (fc.name.has_value())
      if (auto res = update_font_name(); !res) fatal_error(res.error());
    if (auto res = update_layout_size(); !res) fatal_error(res.error());
  }

  std::expected<void, error_trace> initialize() {
    if (text_layout) return {};
    if (auto res = dwrite.initialize(); !res) return unexpected_error(res.error());
    font.size = font.size.value_or(16.0f);
    font.weight = font.weight.value_or(font_weight::normal);
    font.style = font.style.value_or(font_style::normal);
    font.stretch = font.stretch.value_or(font_stretch::normal);
    {
      IDWriteTextFormat* tfp = nullptr;
      const auto hr = dwrite.factory()->CreateTextFormat( //
        font.name.value_or(L"").c_str(), nullptr, //
        static_cast<DWRITE_FONT_WEIGHT>(*font.weight), //
        static_cast<DWRITE_FONT_STYLE>(*font.style), //
        static_cast<DWRITE_FONT_STRETCH>(*font.stretch), //
        *font.size, L"", &tfp);
      if (FAILED(hr) || !tfp) return unexpected_error(errors::operation_failed, "CreateTextFormat failed", int(hr));
      tfp->SetTextAlignment(static_cast<DWRITE_TEXT_ALIGNMENT>(alignment));
      {
        IDWriteTextLayout* tlp = nullptr;
        const auto hr = dwrite.factory()->CreateTextLayout(string.c_str(), UINT(string.size()), tfp, 1e6, 1e6, &tlp);
        if (FAILED(hr) || !tlp) return unexpected_error(errors::operation_failed, "CreateTextLayout failed", int(hr));
        text_layout.reset(tlp);
      }
    }
    if (auto res = update_font_name(); !res) return unexpected_error(res.error());
    if (auto res = update_layout_size(); !res) return unexpected_error(res.error());
  }

  std::expected<void, error_trace> draw() {
    if (!text_layout) return unexpected_error(errors::not_initialized, "Not initialized");
    if (auto res = draw_text({}, text_layout.get()); !res) return unexpected_error(res.error());
    return {};
  }

  std::expected<void, error_trace> draw(float2 Pos, float2 Size) {
    if (!text_layout) return unexpected_error(errors::not_initialized, "Not initialized");
    static const float c[] = {0.5f, 0.0f, 1.0f};
    const auto cc = float2(c[unsigned(block_alignment) % 3], c[(unsigned(block_alignment) / 3) % 3]);
    const auto origin = Pos + (Size - layout_size) * cc + offset;
    brush.color(color);
    if (auto res = draw_text(origin, text_layout.get()); !res) return unexpected_error(res.error());
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

  class handle {
    friend struct text;
    text* _p = nullptr;
    handle(text& Ref) : _p(&Ref) {}

  public:
    ~handle() noexcept {
      if (_p && _p->view_changed) {
        if (const auto csp = system::slot_address<control>(_p->control_id)) csp->make_dirty();
      }
    }

    handle(handle&& Other) noexcept : _p(std::exchange(Other._p, nullptr)) {}
    handle& operator=(handle&& Other) noexcept {
      if (this != &Other) _p = std::exchange(Other._p, nullptr);
      return *this;
    }

    const std::wstring& string() const { return _p->string; }
    handle& string(std::wstring Text) {
      if (auto res = _p->initialize(); !res) fatal_error(res.error());
      {
        IDWriteTextLayout* tlp = nullptr;
        IDWriteTextFormat* tfp = static_cast<IDWriteTextFormat*>(_p->text_layout.get());
        const auto hr = dwrite.factory()->CreateTextLayout(Text.c_str(), UINT(Text.size()), tfp, 1e6, 1e6, &tlp);
        if (FAILED(hr) || !tlp) fatal_error(errors::operation_failed, "CreateTextLayout failed", int(hr));
        _p->text_layout.reset(tlp);
      }
      _p->string = std::move(Text);
      if (auto res = _p->update_layout_size(); !res) fatal_error(res.error());
      _p->view_changed = true;
      return *this;
    }

    yw::font_config font() const { return _p->font; }
    handle& font(yw::font_config Config) {
      if (auto res = _p->initialize(); !res) fatal_error(res.error());
      if (auto res = _p->set_font_config(std::move(Config)); !res) fatal_error(res.error());
      _p->view_changed = true;
      return *this;
    }

    const std::wstring& font_name() const { return *_p->font.name; }
    handle& font_name(std::wstring Name) { return font({Name}); }

    float font_size() const { return *_p->font.size; }
    handle& font_size(float1 Size) { return font({{}, Size.x}); }

    font_weight font_weight() const { return *_p->font.weight; }
    handle& font_weight(yw::font_weight Weight) { return font({{}, {}, Weight}); }

    font_style font_style() const { return *_p->font.style; }
    handle& font_style(yw::font_style Style) { return font({{}, {}, {}, Style}); }

    font_stretch font_stretch() const { return *_p->font.stretch; }
    handle& font_stretch(yw::font_stretch Stretch) { return font({{}, {}, {}, {}, Stretch}); }

    const auto& color() const { return _p->color; }
    auto& color(yw::color Color) { return _p->color = Color, _p->view_changed = true, *this; }

    const auto& alignment() const { return _p->alignment; }
    auto& alignment(yw::text_alignment Alignment) { return _p->alignment = Alignment, _p->view_changed = true, *this; }

    const auto& layout_size() const { return _p->layout_size; }

    IDWriteTextLayout* text_layout() const { return _p->text_layout.get(); }
  };

  handle handle() noexcept { return *this; }
};
}
