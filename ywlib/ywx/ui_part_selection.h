#pragma once
#include "ywx/ui_part_label.h"

namespace yw::ui::part {

class selection {
  size_t _anchor{};
  size_t _start{};
  size_t _end{};
  yw::color _color = yw::color(0x3399FF, 0.35f);

public:
  const auto& anchor() const noexcept { return _anchor; }
  void anchor(size_t value) noexcept { _anchor = value; }

  const auto& start() const noexcept { return _start; }
  void start(size_t value) noexcept { _start = value; }

  const auto& end() const noexcept { return _end; }
  void end(size_t value) noexcept { _end = value; }

  const auto& color() const noexcept { return _color; }
  void color(yw::color value) noexcept { _color = value; }

  bool empty() const noexcept { return _start == _end; }
  size_t length() const noexcept { return _end - _start; }

  void clear() noexcept { _start = _end = _anchor; }

  void move_backward() noexcept {
    if (_anchor < _end) --_end;
    else if (_start > 0) --_start;
  }
  void move_backward(size_t n) noexcept {
    while (n-- > 0) move_backward();
  }

  void move_forward(size_t End) noexcept {
    if (_start < _anchor) ++_start;
    else if (_end < End) ++_end;
  }
  void move_forward(size_t n, size_t End) noexcept {
    while (n-- > 0) move_forward(End);
  }

  std::expected<void, error_trace> draw(float2 Pos, const label& Label) const {
    const auto& layout = Label.layout();
    const auto& text = Label.text();
    const auto pad = Label.padding();
    brush.color(_color);
    if (auto ht = layout.hit_test_range({_start, _end}, Pos + pad)) {
      for (const auto& h : *ht)
        if (auto res = fill_rectangle({h.x, h.y}, {h.z, h.w}); !res)
          return unexpected_error(res.error());
    } else return unexpected_error(ht.error());
    return {};
  }
};
}
