#pragma once
#include "ywx/text_layout.h"
#include "ywx/ui_plain.h"

namespace yw::ui {

class label : public plain {
public:
  class part {
    float2 _size{};
    std::wstring _text{};
    yw::color _color = colors::black;
    mutable text_layout _layout{};
    mutable bool _need_update_layout = true;

    std::expected<void, error_trace> _update_layout() const {
      IDWriteTextFormat* tf = _layout ? static_cast<IDWriteTextFormat*>(_layout) : dwrite.text_format();
      if (auto tl = text_layout::create(_text, tf, _size)) _layout = std::move(*tl);
      else return unexpected_error(tl.error());
      _need_update_layout = false;
      return {};
    }

  public:
    const auto& size() const { return _size; }
    const auto& text() const { return _text; }
    const auto& color() const { return _color; }
    const auto& layout() const { return _layout; }

    auto& text() {
      _need_update_layout = true;
      return _text;
    }
    auto& layout() {
      _need_update_layout = true;
      return _layout;
    }

    void size(float2 Value) {
      _need_update_layout = true;
      _size = Value;
    }

    template<stringable S> void text(S&& Value) {
      _need_update_layout = true;
      _text = unicode<wchar_t>(static_cast<S&&>(Value));
    }

    void color(const yw::color& Value) { _color = Value; }

    std::expected<void, error_trace> layout(text_format_like auto&& text_format) {
      if (auto tl = text_layout::create(_text, text_format, _size)) {
        _layout = std::move(*tl);
        _need_update_layout = false;
        return {};
      } else return unexpected_error(tl.error());
    }

    std::expected<void, error_trace> draw(float2 Pos) const {
      if (_need_update_layout)
        if (auto res = _update_layout(); !res) return unexpected_error(res.error());
      brush.color(_color);
      if (auto res = draw_text(Pos, _layout); !res) return unexpected_error(res.error());
      return {};
    }
  };

  class slot : public plain::slot {
  public:
    mutable part text;

    virtual void draw(float2 Pos, float2 Size) const override {
      plain::slot::draw(Pos, Size);
      text.size(last_rect.zw() - last_rect.xy());
      text.draw(last_rect.xy());
    }

    virtual void draw() const override {
      plain::slot::draw();
      text.draw(last_rect.xy());
    }
  };

  label() noexcept = default;

  label(derived_from<unknown> auto& Layout) {
    if (auto res = create_control<label>(Layout)) _id = *res;
  }

  using plain::operator bool;

  auto& text() { return unsafe_get(&slot::text); }
  const auto& text() const { return unsafe_get(&slot::text); }
};
} // namespace yw::ui
