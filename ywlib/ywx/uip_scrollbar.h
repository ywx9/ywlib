#pragma once
#include "ywx/ui_parts.h"

namespace yw::ui::parts {

template<bool Vertical> struct scrollbar : public part_base {
  color track_color = colors::darkgray;
  color thumb_color = colors::gray;
  color border_color = colors::black;
  color icon_color = colors::black;
  float2 range{};
  float max_value{};
  float radius = arbitrary_value;
  float width = arbitrary_value;
  float border_width = 1.0f;
  bool dragging = false;

  std::expected<void, error_trace> draw(float2 Pos, float2 Size) {
    constexpr bool v = Vertical;
    const float2 box_sz = {width, width};
    const float2 track_sz = v ? float2(width, Size.y - width * 2.0f) : float2(Size.x - width * 2.0f, width);
    const float left = get<!v>(Pos) + get<!v>(Size) - width;
    const float2 top_pos = v ? float2(left, 0) : float2(0, left);
    const float2 track_pos = v ? float2(left, width) : float2(width, left);
    const float2 bot_pos = v ? float2(left, Size.y - width) : float2(Size.x - width, left);
    float2 thumb_pos = track_pos;
    if (max_value == 0.0f) get<v>(thumb_pos) += get<v>(track_sz);
    else get<v>(thumb_pos) += get<v>(track_sz) * (range.x / max_value);
    float2 thumb_sz = track_sz;
    if (max_value != 0.0f) get<v>(thumb_sz) *= (range.y - range.x) / max_value;
    const auto r = yw::min(radius, width / 2);
    brush.color(track_color);
    fill_rectangle(track_pos, track_sz);
    brush.color(thumb_color);
    fill_round_rectangle(thumb_pos, thumb_sz, float2(r, r));
    fill_rectangle(top_pos, box_sz);
    fill_rectangle(bot_pos, box_sz);
    brush.color(border_color);
    draw_round_rectangle(thumb_pos, thumb_sz, float2(r, r));
    draw_rectangle(top_pos, box_sz);
    draw_rectangle(bot_pos, box_sz);
    const float o = width * 0.2f;
    const float w_o = width - o;
    if constexpr (Vertical) {
      const auto top = top_pos + float2(width * 0.5f, o);
      draw_line(top, top_pos + float2(o, w_o));
      draw_line(top, top_pos + float2(w_o, w_o));
      const auto bot = bot_pos + float2(width * 0.5f, w_o);
      draw_line(bot, bot_pos + float2(o, o));
      draw_line(bot, bot_pos + float2(w_o, o));
    } else {
      const auto top = top_pos + float2(o, width * 0.5f);
      draw_line(top, top_pos + float2(w_o, o));
      draw_line(top, top_pos + float2(w_o, w_o));
      const auto bot = bot_pos + float2(w_o, width * 0.5f);
      draw_line(bot, bot_pos + float2(o, o));
      draw_line(bot, bot_pos + float2(o, w_o));
    }
  }

  class handle : public part_base::handle<scrollbar<Vertical>> {
    template<bool V> friend struct scrollbar;
    using part_base::handle<scrollbar<Vertical>>::handle;

  public:
    const auto& track_color() const { return _p->track_color; }
    auto& track_color(yw::color Color) {
      _p->track_color = Color;
      _p->view_changed = true;
      return *this;
    }

    const auto& thumb_color() const { return _p->thumb_color; }
    auto& thumb_color(yw::color Color) {
      _p->thumb_color = Color;
      _p->view_changed = true;
      return *this;
    }

    const auto& border_color() const { return _p->border_color; }
    auto& border_color(yw::color Color) {
      _p->border_color = Color;
      _p->view_changed = true;
      return *this;
    }

    const auto& icon_color() const { return _p->icon_color; }
    auto& icon_color(yw::color Color) {
      _p->icon_color = Color;
      _p->view_changed = true;
      return *this;
    }

    const auto& radius() const { return _p->radius; }
    auto& radius(float1 Radius) {
      _p->radius = Radius.x;
      _p->view_changed = true;
      return *this;
    }

    const auto& width() const { return _p->width; }
    auto& width(float1 Width) {
      _p->width = yw::max(0.0f, Width.x);
      _p->layout_changed = true;
      return *this;
    }

    const auto& max_value() const { return _p->max_value; }
    auto& max_value(float1 Value) {
      _p->max_value = yw::max(0.0f, Value.x);
      _p->view_changed = true;
      return *this;
    }

    const auto& border_width() const { return _p->border_width; }
    auto& border_width(float BorderWidth) {
      _p->border_width = BorderWidth;
      _p->view_changed = true;
      return *this;
    }

    const auto& range() const { return _p->range; }
  };

  handle handle() noexcept { return *this; }
};

using vscrollbar = scrollbar<true>;
using hscrollbar = scrollbar<false>;
}
