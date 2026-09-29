#pragma once
#include <apps/directx.h>
#include <core/string.h>

namespace yw {

///--------------------------------------------------------------------------///
/// MARK: font_config

struct font_config {
  string<wchar_t> name = L"";
  float size = 16.0f;
  DWRITE_FONT_WEIGHT weight = DWRITE_FONT_WEIGHT_NORMAL;
  DWRITE_FONT_STYLE style = DWRITE_FONT_STYLE_NORMAL;
  DWRITE_FONT_STRETCH stretch = DWRITE_FONT_STRETCH_NORMAL;
};

comptr<IDWriteTextFormat> create_text_format(const font_config& Config) {
  comptr<IDWriteTextFormat> result;
  const auto hr = dwrite::factory()->CreateTextFormat(
    Config.name.c_str(), nullptr, Config.weight, Config.style, Config.stretch, Config.size, L"", &result.get());
  if (FAILED(hr)) {
    error(errors::operation_failed, "CreateTextFormat failed").consume();
    return {};
  } else return result;
}

///--------------------------------------------------------------------------///
/// MARK: text_layout_like

template<typename T> concept text_layout_like = castable_to<T&, IDWriteTextLayout*>;
template<typename T> concept text_format_like = castable_to<T&, IDWriteTextFormat*> || text_layout_like<T>;

inline constexpr auto get_text_layout = []<text_layout_like T>(T&& TextLayout) //
  noexcept(nt_castable_to<T&, IDWriteTextLayout*>) { return static_cast<IDWriteTextLayout*>(TextLayout); };
inline constexpr auto get_text_format = []<text_format_like T>(T&& TextFormat) //
  noexcept((text_layout_like<T>&& nt_castable_to<T&, IDWriteTextLayout*>) || nt_castable_to<T&, IDWriteTextFormat*>) {
    if constexpr (text_layout_like<T>) return static_cast<IDWriteTextFormat*>(get_text_layout(TextFormat));
    else return static_cast<IDWriteTextFormat*>(TextFormat);
  };

///--------------------------------------------------------------------------///
/// MARK: text

class text {
  font_config _font;
  yw::string<wchar_t> _string;
  comptr<IDWriteTextLayout> _dwrite_text_layout;
  float2 _size;

public:
  const font_config& font() const noexcept { return _font; }
  result<void> font(const font_config& Font) {
    if (auto res = create(Font, _string)) *this = move(*res);
    else return res.relay();
    return {};
  }

  const yw::string<wchar_t>& string() const noexcept { return _string; }
  result<void> string(const yw::string<wchar_t>& String) {
    if (auto res = create(_font, String)) *this = move(*res);
    else return res.relay();
    return {};
  }

  auto dwrite_text_layout() const noexcept { return _dwrite_text_layout.get(); }
  explicit operator IDWriteTextLayout*() const noexcept { return _dwrite_text_layout.get(); }
  explicit operator bool() const noexcept { return bool(_dwrite_text_layout); }
  float2 size() const noexcept { return _size; }

  text() = default;

  template<stringable S> static result<text> create(font_config Font, S&& String) {
    text result;
    result._font = move(Font);
    auto tf = create_text_format(result._font);
    if (!tf) return error(errors::operation_failed, "CreateTextFormat failed");
    result._string = unicode<wchar_t>(static_cast<S&&>(String));
    const auto hr = dwrite::factory()->CreateTextLayout(
      result._string.c_str(), UINT32(result._string.size()), tf.get(), 1e6, 1e6, &result._dwrite_text_layout.get());
    if (FAILED(hr)) return error(errors::operation_failed, "CreateTextLayout failed");
    DWRITE_TEXT_METRICS metrics;
    if (const auto hr = result._dwrite_text_layout->GetMetrics(&metrics); FAILED(hr))
      return error(errors::operation_failed, "GetMetrics failed");
    result._size = float2(metrics.widthIncludingTrailingWhitespace, metrics.height);
    if (const auto hr = result._dwrite_text_layout->SetMaxWidth(result._size.x()); FAILED(hr))
      return error(errors::operation_failed, "SetMaxWidth failed");
    if (const auto hr = result._dwrite_text_layout->SetMaxHeight(result._size.y()); FAILED(hr))
      return error(errors::operation_failed, "SetMaxHeight failed");
    return result;
  }

  template<stringable S> text(font_config Font, S&& String, const std::source_location& sl = here()) {
    if (auto res = create(move(Font), static_cast<S&&>(String))) *this = move(*res);
    else res.error().add_footprint().print_and_abort(sl);
  }

  struct hit_test_result {
    float2 pos{};     // position(x, y) of character/text that is hit
    float2 size{};    // size(w, h) of character/text that is hit
    uint32_t index{}; // index of character that is hit
    bool trailing{};  // whether hit is on trailing side of character
    bool inside{};    // whether hit is inside text
  };

  result<hit_test_result> hittest(uint1 Index) const {
    if (!*this) return error(errors::not_initialized);
    DWRITE_HIT_TEST_METRICS metrics{};
    float x, y;
    if (const auto hr = _dwrite_text_layout->HitTestTextPosition(Index[0], false, &x, &y, &metrics); FAILED(hr))
      return error(errors::operation_failed, "HitTestTextPosition failed");
    return hit_test_result{
      .pos = {metrics.left, metrics.top},
      .size = {metrics.width, metrics.height},
      .index = Index[0],
      .trailing = {},
      .inside = Index[0] < _string.size()};
  }

  result<hit_test_result> hittest(float2 Pt) const {
    if (!*this) return error(errors::not_initialized);
    DWRITE_HIT_TEST_METRICS metrics{};
    BOOL trailing, inside;
    if (const auto hr = _dwrite_text_layout->HitTestPoint(Pt[0], Pt[1], &trailing, &inside, &metrics); FAILED(hr))
      return error(errors::operation_failed, "HitTestPoint failed");
    return hit_test_result{
      .pos = {metrics.left, metrics.top},
      .size = {metrics.width, metrics.height},
      .index = metrics.textPosition,
      .trailing = bool(trailing),
      .inside = bool(inside)};
  }

  result<array<hit_test_result>> hittest_range(uint2 Range, float2 Origin = {}) const {
    if (!*this) return error(errors::not_initialized);
    if (Range[0] >= _string.size()) return error(errors::invalid_argument, "Range start is out of bounds");
    const auto length = Range[1] - Range[0];
    uint32_t count = 0;
    auto hr = _dwrite_text_layout->HitTestTextRange(Range[0], length, Origin[0], Origin[1], nullptr, 0, &count);
    if (hr != E_NOT_SUFFICIENT_BUFFER) return error(errors::operation_failed, "HitTestTextRange failed");
    array<DWRITE_HIT_TEST_METRICS> metrics(count);
    hr = _dwrite_text_layout->HitTestTextRange(Range[0], length, Origin[0], Origin[1], metrics.data(), count, &count);
    if (FAILED(hr)) return error(errors::operation_failed, "HitTestTextRange failed");
    array<hit_test_result> result;
    result.reserve(count);
    for (size_t i = 0; i < count; ++i)
      result.emplace_back(
        hit_test_result{
          .pos = {metrics[i].left, metrics[i].top},
          .size = {metrics[i].width, metrics[i].height},
          .index = metrics[i].textPosition});
    return result;
  }
};

///--------------------------------------------------------------------------///
/// MARK: draw text

inline result<void> draw_text(float2 Pos, text_layout_like auto&& TextLayout) {
  if (!drawing::target_exists()) return error(errors::invalid_operation, "drawing target not available");
  if (auto tl = get_text_layout(TextLayout); !tl) return error(errors::not_initialized);
  else d2d::context()->DrawTextLayout(D2D1::Point2F(Pos.x(), Pos.y()), tl, d2d::solid_color_brush());
  return {};
}

inline result<void> draw_text(float2 Pos, text_layout_like auto&& TextLayout, const color& Color) {
  d2d::set_solid_color(Color);
  if (auto res = draw_text(Pos, TextLayout)) return {};
  else return res.relay();
}
} // namespace yw
