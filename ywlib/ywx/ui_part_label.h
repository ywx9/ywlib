#pragma once
#include "ywx/text_layout.h"
#include "ywx/window.h"

namespace yw::ui::part {

class label {
  float2 _size{};
  float2 _padding{4.0f, 2.0f};
  std::wstring _text{};
  yw::color _color = colors::black;
  mutable text_layout _layout{};
  mutable bool _need_update_layout = true;

  std::expected<void, error_trace> _update_layout() const {
    IDWriteTextFormat* tf = _layout ? static_cast<IDWriteTextFormat*>(_layout) : dwrite.text_format();
    if (auto tl = text_layout::create(_text, tf, _size - _padding * 2.0f)) _layout = std::move(*tl);
    else return unexpected_error(tl.error());
    _need_update_layout = false;
    return {};
  }

public:
  const auto& size() const { return _size; }
  void size(float2 value) { _size = value, _need_update_layout = true; }

  const auto& padding() const { return _padding; }
  void padding(float2 value) { _padding = value, _need_update_layout = true; }

  const auto& text() const { return _text; }
  auto& text() { return _need_update_layout = true, _text; }
  void text(std::wstring value) { _text = std::move(value), _need_update_layout = true; }

  const auto& color() const { return _color; }
  void color(yw::color value) { _color = value; }

  const auto& layout() const { return _layout; }
  auto& layout() {
    if (!_layout) _update_layout();
    _need_update_layout = true;
    return _layout;
  }

  std::expected<void, error_trace> format(text_format_like auto&& text_format) {
    if (auto tl = text_layout::create(_text, text_format, _size - _padding * 2.0f)) {
      _layout = std::move(*tl);
      _need_update_layout = false;
      return {};
    } else return unexpected_error(tl.error());
  }

  std::expected<void, error_trace> draw(float2 Pos) const {
    if (_need_update_layout)
      if (auto res = _update_layout(); !res) return unexpected_error(res.error());
    brush.color(_color);
    if (auto res = draw_text(Pos + _padding, _layout); !res) return unexpected_error(res.error());
    return {};
  }
};
} // namespace yw::ui::part
