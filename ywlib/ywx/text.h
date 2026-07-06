#pragma once
#include <ywx/drawing.h>

namespace yw {

class text {
  string<wchar_t> _string{};
  font_config _font = font_config::default_;
  float2 _size{};
  comptr<IDWriteTextLayout> _layout{};

  std::expected<void, error> update_size() {
    if (!_layout) return std::unexpected(error(errors::invalid_operation));
    DWRITE_TEXT_METRICS metrics{};
    hresult_test(_layout->GetMetrics, &metrics);
    _size = float2(metrics.widthIncludingTrailingWhitespace, metrics.height);
    hresult_test(_layout->SetMaxWidth, yw::max(_size.x, 1.0f));
    hresult_test(_layout->SetMaxHeight, yw::max(_size.y, 1.0f));
    return {};
  }

  std::expected<void, error> initialize(yw::string<wchar_t> String, font_config Font) {
    _string = std::move(String);
    _font = std::move(Font);
    IDWriteTextFormat* tf;
    IDWriteTextLayout* tl;
    hresult_test(
      dwrite::factory()->CreateTextFormat, _font.get_name(), nullptr, DWRITE_FONT_WEIGHT(_font.get_weight()),
      DWRITE_FONT_STYLE(_font.get_style()), DWRITE_FONT_STRETCH(_font.get_stretch()), _font.get_size(), L"", &tf);
    hresult_test(dwrite::factory()->CreateTextLayout, _string.c_str(), UINT32(_string.size()), tf, 1e6, 1e6, &tl);
    tf->Release();
    _layout.reset(tl);
    if (auto res = update_size(); !res) return res.error().relay();
    return {};
  }

  std::expected<void, error> initialize(font_config Font) {
    if (auto res = initialize(yw::string<wchar_t>(), std::move(Font))) return {};
    else return res.error().relay();
  }

public:
  text() noexcept = default;
  explicit operator bool() const noexcept { return bool(_layout); }
  explicit operator IDWriteTextLayout*&() & noexcept { return _layout.get(); }
  explicit operator IDWriteTextLayout*() const& noexcept { return _layout.get(); }
  IDWriteTextLayout* dwrite_text_layout() const noexcept { return _layout.get(); }

  text(string<wchar_t> String, font_config Font = font_config::default_, const source_line& sl = here()) {
    if (auto res = initialize(std::move(String), std::move(Font)); !res) res.error().go_off(sl);
  }

  text(font_config Font, const source_line& sl = here()) {
    if (auto res = initialize(std::move(Font)); !res) res.error().go_off(sl);
  }

  template<typename... As> requires constructible<text, As...> static std::expected<text, error> create(As&&... as) {
    text t;
    if (auto res = t.initialize(static_cast<As&&>(as)...)) return std::move(t);
    else return res.error().relay();
  }

  //-- getter --//

  const yw::string<wchar_t>& string() const noexcept { return _string; }
  const font_config& font() const noexcept { return _font; }
  float2 size() const noexcept { return _size; }

  //-- setter --//

  std::expected<void, error> string(yw::string<wchar_t> String) & {
    _string = std::move(String);
    IDWriteTextLayout* tl;
    hresult_test(
      dwrite::factory()->CreateTextLayout, _string.c_str(), UINT32(_string.size()), _layout.get(), 1e6, 1e6, &tl);
    _layout.reset(tl);
    if (auto res = update_size(); !res) return res.error().relay();
    return {};
  }

  std::expected<void, error> font(font_config Font) & {
    if (auto res = create(_string, std::move(Font))) *this = std::move(*res);
    else return res.error().relay();
    return {};
  }

  //-- other functions --//

  std::expected<void, error> draw(float2 Pos) const {
    if (!drawing::d2d_drawing()) return std::unexpected(error(errors::invalid_operation));
    if (!_layout) return {};
    d2d::context()->DrawTextLayout(D2D1_POINT_2F{Pos.x, Pos.y}, _layout.get(), brush::d2d_brush());
    return {};
  }

  /// returns `{left, top, width, height}` of character at `Index`
  std::expected<float4, error> hittest(uint1 Index, bool Trailing = false) const {
    if (!_layout) return std::unexpected(error(errors::not_initialized));
    DWRITE_HIT_TEST_METRICS metrics{};
    float2 pt{};
    hresult_test(_layout->HitTestTextPosition, Index.x, Trailing, &pt.x, &pt.y, &metrics);
    return float4(metrics.left, metrics.top, metrics.width, metrics.height);
  }

  /// returns `{textPosition, {width, height}}` of character at `Pt`
  std::expected<yw::tuple<uint32_t, float2>, error> hittest(float2 Pt) const {
    if (!_layout) return std::unexpected(error(errors::not_initialized));
    DWRITE_HIT_TEST_METRICS metrics{};
    BOOL inside = FALSE;
    BOOL trailing = FALSE;
    hresult_test(_layout->HitTestPoint, Pt.x, Pt.y, &inside, &trailing, &metrics);
    return yw::tuple<uint32_t, float2>{metrics.textPosition + bool(trailing), float2(metrics.width, metrics.height)};
  }

  std::expected<std::vector<float4>, error> hittest_range(uint2 Range, float2 Origin = {}) const {
    if (!_layout) return std::unexpected(error(errors::not_initialized));
    if (Range.x >= Range.y) return std::unexpected(error(errors::invalid_argument));
    const auto length = Range.y - Range.x;
    uint32_t count = 0;
    auto hr = _layout->HitTestTextRange(Range.x, length, Origin.x, Origin.y, nullptr, 0, &count);
    if (hr != E_NOT_SUFFICIENT_BUFFER) return std::unexpected(error(errors::operation_failed, "HitTestTextRange failed", int32_t(hr)));
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
