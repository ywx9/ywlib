#pragma once
#include <ywx/drawing.h>

namespace yw {

class text : public handle_base {
public:
  struct slot : public handle_base::slot {
    font_config font = font_config::default_;
    string<wchar_t> string{};
    comptr<IDWriteTextLayout> layout{};
    float2 size{};

    std::expected<void, error> apply_font(const font_config& Font) {
      if (Font.name) font = Font;
      if (Font.size) font.size = Font.size;
      if (Font.weight) font.weight = Font.weight;
      if (Font.style) font.style = Font.style;
      if (Font.stretch) font.stretch = Font.stretch;
      if (auto res = update_layout(); !res) return res.error().relay();
      return {};
    }

    std::expected<void, error> update_layout() {
      IDWriteTextFormat* tf;
      IDWriteTextLayout* tl;
      hresult_test(
        dwrite::factory()->CreateTextFormat, font.get_name(), nullptr, DWRITE_FONT_WEIGHT(font.get_weight()),
        DWRITE_FONT_STYLE(font.get_style()), DWRITE_FONT_STRETCH(font.get_stretch()), font.get_size(), L"", &tf);
      hresult_test(dwrite::factory()->CreateTextLayout, string.c_str(), UINT32(string.size()), tf, 1e6, 1e6, &tl);
      tf->Release();
      layout.reset(tl);
      if (auto res = update_size(); !res) return res.error().relay();
      return {};
    }

    std::expected<void, error> update_size() {
      if (!layout) return std::unexpected(error(errors::invalid_operation));
      DWRITE_TEXT_METRICS metrics{};
      hresult_test(layout->GetMetrics, &metrics);
      size = float2(metrics.widthIncludingTrailingWhitespace, metrics.height);
      hresult_test(layout->SetMaxWidth, yw::max(size.x, 1.0f));
      hresult_test(layout->SetMaxHeight, yw::max(size.y, 1.0f));
      return {};
    }
  };

  explicit operator bool() const noexcept {
    const auto sp = slot::get_as<text>(id());
    return sp && sp->layout;
  }

  IDWriteTextLayout* dwrite_text_layout() const noexcept {
    if (const auto sp = slot::get_as<text>(id())) return sp->layout.get();
    else return nullptr;
  }

  text() noexcept = default;

  static std::expected<text, error> create(
    yw::string<wchar_t> String, const font_config& Font = font_config::default_) {
    const auto sp = make_slot<text>();
    if (!sp) return std::unexpected(error(errors::slot_creation_failed));
    sp->string = std::move(String);
    if (auto res = sp->apply_font(Font); !res) return res.error().relay();
    return make_handle<text>(sp->id);
  }

  text(yw::string<wchar_t> String, const font_config& Font = font_config::default_, const source_line& sl = here()) {
    if (auto res = create(std::move(String), Font); !res) res.error().go_off(sl);
    else *this = std::move(*res);
  }

  //-- getter --//

  const yw::string<wchar_t>& string() const noexcept {
    const auto sp = slot::get_as<text>(id());
    if (!sp) error(errors::invalid_operation, "invalid handle").go_off();
    return sp->string;
  }

  const font_config& font() const noexcept {
    const auto sp = slot::get_as<text>(id());
    if (!sp) error(errors::invalid_operation, "invalid handle").go_off();
    return sp->font;
  }

  float2 size() const noexcept {
    if (const auto sp = slot::get_as<text>(id()); !sp) {
      error(errors::invalid_operation, "invalid handle").fizzle_out();
      return {};
    } else return sp->size;
  }

  //-- setter --//

  auto& string(this auto& self, yw::string<wchar_t> String) {
    if (const auto sp = slot::get_as<text>(self.id())) {
      sp->string = std::move(String);
      if (auto res = sp->update_layout(); !res) res.error().go_off();
    } else error(errors::invalid_operation, "invalid handle").fizzle_out();
    return self;
  }

  auto& font(this auto& self, const font_config& Font) {
    if (const auto sp = slot::get_as<text>(self.id())) {
      if (auto res = sp->apply_font(Font); !res) res.error().go_off();
    } else error(errors::invalid_operation, "invalid handle").fizzle_out();
    return self;
  }

  //-- other functions --//

  struct hittest_result {
    float2 pos{};     // position of character/text that is hit
    float2 size{};    // size of character/text that is hit
    uint32_t index{}; // index of character that is hit
    bool trailing{};  // whether hit is on trailing side of character
    bool inside{};    // whether hit is inside text
  };

  /// returns `{left, top, width, height}` of character at `Index`
  std::expected<hittest_result, error> hittest(uint1 Index) const {
    const auto sp = slot::get_as<text>(id());
    if (!sp) return std::unexpected(error(errors::invalid_operation, "invalid handle"));
    if (!sp->layout) return std::unexpected(error(errors::not_initialized));
    DWRITE_HIT_TEST_METRICS metrics{};
    float2 pt{};
    hresult_test(sp->layout->HitTestTextPosition, Index.x, false, &pt.x, &pt.y, &metrics);
    return hittest_result{
      .pos = float2(metrics.left, metrics.top),
      .size = float2(metrics.width, metrics.height),
      .index = Index.x,
      .trailing = {},
      .inside = Index.x < sp->string.size()};
  }

  /// returns `{textPosition, {x, y, width, height}}` of character at `Pt`
  std::expected<hittest_result, error> hittest(float2 Pt) const {
    const auto sp = slot::get_as<text>(id());
    if (!sp) return std::unexpected(error(errors::invalid_operation, "invalid handle"));
    if (!sp->layout) return std::unexpected(error(errors::not_initialized));
    DWRITE_HIT_TEST_METRICS metrics{};
    BOOL trailing, inside;
    hresult_test(sp->layout->HitTestPoint, Pt.x, Pt.y, &trailing, &inside, &metrics);
    return hittest_result{
      .pos = float2(metrics.left, metrics.top),
      .size = float2(metrics.width, metrics.height),
      .index = metrics.textPosition,
      .trailing = bool(trailing),
      .inside = bool(inside)};
  }

  std::expected<std::vector<hittest_result>, error> hittest_range(uint2 Range, float2 Origin = {}) const {
    const auto sp = slot::get_as<text>(id());
    if (!sp) return std::unexpected(error(errors::invalid_operation, "invalid handle"));
    if (!sp->layout) return std::unexpected(error(errors::not_initialized));
    if (Range.x >= Range.y) return std::unexpected(error(errors::invalid_argument));
    const auto length = Range.y - Range.x;
    uint32_t count = 0;
    auto hr = sp->layout->HitTestTextRange(Range.x, length, Origin.x, Origin.y, nullptr, 0, &count);
    if (hr != E_NOT_SUFFICIENT_BUFFER)
      return std::unexpected(error(errors::operation_failed, "HitTestTextRange failed", int32_t(hr)));
    std::vector<DWRITE_HIT_TEST_METRICS> metrics(count);
    hresult_test(sp->layout->HitTestTextRange, Range.x, length, Origin.x, Origin.y, metrics.data(), count, &count);
    std::vector<hittest_result> rects;
    rects.reserve(count);
    for (size_t i = 0; i < count; ++i)
      rects.emplace_back(
        hittest_result{
          .pos = float2(metrics[i].left, metrics[i].top),
          .size = float2(metrics[i].width, metrics[i].height),
          .index = metrics[i].textPosition});
    return rects;
  }
};

/// MARK: draw_text

inline std::expected<void, error> draw_text(float2 Pos, const text& Text) {
  if (!drawing::d2d_drawing()) return std::unexpected(error(errors::invalid_operation, "drawing not begun"));
  const auto sp = text::slot::get_as<text>(Text.id());
  if (!sp) return std::unexpected(error(errors::invalid_slotid));
  if (!sp->layout) return std::unexpected(error(errors::not_initialized));
  d2d::context()->DrawTextLayout({Pos.x, Pos.y}, sp->layout.get(), brush::d2d_brush());
  return {};
}

inline std::expected<void, error> draw_text(float2 Pos, const text& Text, const color& Color) {
  brush::color(Color);
  return draw_text(Pos, Text);
}

} // namespace yw
