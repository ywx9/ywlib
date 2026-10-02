#pragma once
#include <apps/text.h>
#include <apps/ui_frame.h>

namespace yw::ui {

class label : public frame {
public:
protected:
  yw::text _text;
  optional<color> _text_color;
  yw::alignment _text_alignment;
  /// 内容の描画に必要な最小サイズを計算する
  virtual result<float2> _calculate_content_size() override { return _text.size(); }
  /// 内容を描画する
  virtual result<void> _draw_content() override {
    if (const auto& tc = text_color(); tc.a <= 0.0f) return {};
    else {
      constexpr float c[]{0.5f, 0.0f, 1.0f};
      const auto a = uint8_t(_text_alignment);
      const float2 cc{c[a % 3], c[a / 4 % 3]};
      const auto difference = _current_size - _text.size();
      if (auto res = draw_text(_current_pos + cc * difference, _text, text_color()); !res) return res.relay();
    }
    return {};
  }

public:
  ~label() noexcept = default;
  label() noexcept = default;
  label(label&& l) noexcept = default;
  label& operator=(label&& l) noexcept = default;

  const yw::text& text() const { return _text; }
  result<void> text(yw::text t) {
    _text = move(t);
    window_system::make_messy(_window);
    return {};
  }

  const yw::string<wchar_t>& string() const { return _text.string(); }
  result<void> string(stringable auto&& s) {
    if (auto res = yw::text::create(unicode<char>(s))) _text = move(*res);
    else return res.relay();
    window_system::make_messy(_window);
    return {};
  }

  const color& text_color() const {
    if (_text_color) return *_text_color;
    else if (const auto ct = window_system::get_color_theme(_window)) return ct->text;
    else return colors::transparent;
  }
  result<void> text_color(color c) {
    _text_color = c;
    window_system::make_dirty(_window);
    return {};
  }
  result<void> text_color(is_none auto) {
    _text_color.reset();
    window_system::make_dirty(_window);
    return {};
  }

  yw::alignment text_alignment() const { return _text_alignment; }
  result<void> text_alignment(yw::alignment a) {
    _text_alignment = a;
    window_system::make_dirty(_window);
    return {};
  }
};
} // namespace yw::ui
