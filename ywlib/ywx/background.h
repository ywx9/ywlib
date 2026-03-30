#pragma once
#include "ywx/bitmap.h"

namespace yw {

class background {
  std::variant<color, bitmap> _bg;

public:
  background() noexcept = default;
  background(const color& c) : _bg(c) {}
  background(bitmap b) : _bg(std::move(b)) {}

  explicit operator bool() const noexcept {
    return std::holds_alternative<color>(_bg) || (std::holds_alternative<bitmap>(_bg) && bool(std::get<bitmap>(_bg)));
  }

  bool is_color() const noexcept { return std::holds_alternative<color>(_bg); }
  bool is_bitmap() const noexcept { return std::holds_alternative<bitmap>(_bg); }

  const color& get_color() const { return std::get<color>(_bg); }
  const bitmap& get_bitmap() const { return std::get<bitmap>(_bg); }
};

inline std::expected<void, error_trace> draw_background(float2 Pos, float2 Size, const background& Bg) {
  if (Bg.is_color()) {
    brush.color(Bg.get_color());
    fill_rectangle(Pos, Size);
  } else if (Bg.is_bitmap()) draw_bitmap(Pos, Size, Bg.get_bitmap());
  return {};
}
} // namespace yw
