#pragma once
#include <ywx/ui_frame.h>

namespace yw::ui {

template<bool Vertical> class scrollbar : public frame {
public:
  enum class scrollbar_part : unsigned char {
    none,
    decrement_button,
    decrement_track,
    thumb,
    increment_track,
    increment_button,
  };

  struct slot : frame::slot {
    float scroll_offset = 0.0f;
    float scroll_content_size = 0.0f;
    float scroll_view_size = 0.0f;
    float bar_width = 14.0f;
    float wheel_scroll_amount = 40.0f;
    color track_color = color(colors::black, 0.08f);
    color button_color = color(colors::black, 0.10f);
    color button_hover_color = color(colors::black, 0.16f);
    color button_pressed_color = color(colors::black, 0.24f);
    color thumb_color = color(colors::black, 0.28f);
    color thumb_hover_color = color(colors::black, 0.38f);
    color thumb_pressed_color = color(colors::black, 0.48f);
    color arrow_color = color(colors::black, 0.65f);
    scrollbar_part hovered_part = scrollbar_part::none;
    scrollbar_part pressed_part = scrollbar_part::none;

    static bool rect_contains(float4 Rect, float2 Pt) noexcept {
      return Pt.x >= Rect.x && Pt.y >= Rect.y && Pt.x <= Rect.z && Pt.y <= Rect.w;
    }

    static float2 rect_pos(float4 Rect) noexcept { return {Rect.x, Rect.y}; }
    static float2 rect_size(float4 Rect) noexcept { return {Rect.z - Rect.x, Rect.w - Rect.y}; }

    float max_scroll_offset() const noexcept { return yw::max(0.0f, scroll_content_size - scroll_view_size); }

    float clamp_scroll_offset(float Offset) const noexcept {
      return yw::clamp(Offset, 0.0f, max_scroll_offset());
    }

    bool set_scroll_offset(float Offset) noexcept {
      const auto next = clamp_scroll_offset(Offset);
      if (scroll_offset == next) return false;
      scroll_offset = next;
      make_dirty();
      return true;
    }

    bool scroll_by(float Delta) noexcept { return set_scroll_offset(scroll_offset + Delta); }

    void update_scroll_content_size(float ContentSize) noexcept {
      scroll_content_size = yw::max(0.0f, ContentSize);
      set_scroll_offset(scroll_offset);
    }

    float4 content_rect() const noexcept {
      if constexpr (Vertical) {
        const auto w = yw::max(0.0f, size.x - bar_width);
        const float4 outer{pos.x, pos.y, pos.x + w, pos.y + size.y};
        return {
          outer.x + padding.x,
          outer.y + padding.y,
          yw::max(outer.x + padding.x, outer.z - padding.z),
          yw::max(outer.y + padding.y, outer.w - padding.w)};
      } else {
        const auto h = yw::max(0.0f, size.y - bar_width);
        const float4 outer{pos.x, pos.y, pos.x + size.x, pos.y + h};
        return {
          outer.x + padding.x,
          outer.y + padding.y,
          yw::max(outer.x + padding.x, outer.z - padding.z),
          yw::max(outer.y + padding.y, outer.w - padding.w)};
      }
    }

    float4 bar_rect() const noexcept {
      if constexpr (Vertical) return {pos.x + size.x - bar_width, pos.y, pos.x + size.x, pos.y + size.y};
      else return {pos.x, pos.y + size.y - bar_width, pos.x + size.x, pos.y + size.y};
    }

    float4 decrement_button_rect() const noexcept {
      const auto r = bar_rect();
      if constexpr (Vertical) return {r.x, r.y, r.z, r.y + bar_width};
      else return {r.x, r.y, r.x + bar_width, r.w};
    }

    float4 increment_button_rect() const noexcept {
      const auto r = bar_rect();
      if constexpr (Vertical) return {r.x, r.w - bar_width, r.z, r.w};
      else return {r.z - bar_width, r.y, r.z, r.w};
    }

    float4 track_rect() const noexcept {
      const auto r = bar_rect();
      if constexpr (Vertical) return {r.x, r.y + bar_width, r.z, r.w - bar_width};
      else return {r.x + bar_width, r.y, r.z - bar_width, r.w};
    }

    float track_length() const noexcept {
      const auto r = track_rect();
      if constexpr (Vertical) return yw::max(0.0f, r.w - r.y);
      else return yw::max(0.0f, r.z - r.x);
    }

    float thumb_length() const noexcept {
      const auto tl = track_length();
      if (tl <= 0.0f) return 0.0f;
      if (scroll_content_size <= 0.0f || scroll_content_size <= scroll_view_size) return tl;
      const auto raw = tl * scroll_view_size / scroll_content_size;
      return yw::min(tl, yw::max(bar_width * 3.0f, raw));
    }

    float thumb_offset_in_track() const noexcept {
      const auto movable = track_length() - thumb_length();
      const auto max_offset = max_scroll_offset();
      if (movable <= 0.0f || max_offset <= 0.0f) return 0.0f;
      return movable * scroll_offset / max_offset;
    }

    float4 thumb_rect() const noexcept {
      const auto tr = track_rect();
      const auto off = thumb_offset_in_track();
      const auto len = thumb_length();
      if constexpr (Vertical) return {tr.x, tr.y + off, tr.z, tr.y + off + len};
      else return {tr.x + off, tr.y, tr.x + off + len, tr.w};
    }

    scrollbar_part part_at(float2 Pt) const noexcept {
      if (!rect_contains(bar_rect(), Pt)) return scrollbar_part::none;
      if (rect_contains(decrement_button_rect(), Pt)) return scrollbar_part::decrement_button;
      if (rect_contains(increment_button_rect(), Pt)) return scrollbar_part::increment_button;
      const auto th = thumb_rect();
      if (rect_contains(th, Pt)) return scrollbar_part::thumb;
      const auto tr = track_rect();
      if (!rect_contains(tr, Pt)) return scrollbar_part::none;
      if constexpr (Vertical) return Pt.y < th.y ? scrollbar_part::decrement_track : scrollbar_part::increment_track;
      else return Pt.x < th.x ? scrollbar_part::decrement_track : scrollbar_part::increment_track;
    }

    void update_scroll_view_size() noexcept {
      const auto r = content_rect();
      if constexpr (Vertical) scroll_view_size = yw::max(0.0f, r.w - r.y);
      else scroll_view_size = yw::max(0.0f, r.z - r.x);
      set_scroll_offset(scroll_offset);
    }

    bool scroll_part(scrollbar_part Part) noexcept {
      switch (Part) {
      case scrollbar_part::decrement_button: return scroll_by(-wheel_scroll_amount);
      case scrollbar_part::increment_button: return scroll_by(+wheel_scroll_amount);
      case scrollbar_part::decrement_track: return scroll_by(-scroll_view_size);
      case scrollbar_part::increment_track: return scroll_by(+scroll_view_size);
      default: return false;
      }
    }

    std::expected<void, error> draw_button_arrow(scrollbar_part Part) const {
      const auto r = Part == scrollbar_part::decrement_button ? decrement_button_rect() : increment_button_rect();
      const auto c = (rect_pos(r) + rect_size(r) * 0.5f);
      const auto aw = bar_width * 0.22f;
      const auto ah = bar_width * 0.16f;
      brush::color(arrow_color);
      if constexpr (Vertical) {
        if (Part == scrollbar_part::decrement_button) {
          if (auto res = draw_line({c.x - aw, c.y + ah}, {c.x, c.y - ah}, 1.0f); !res) return res.error().relay();
          if (auto res = draw_line({c.x, c.y - ah}, {c.x + aw, c.y + ah}, 1.0f); !res) return res.error().relay();
        } else {
          if (auto res = draw_line({c.x - aw, c.y - ah}, {c.x, c.y + ah}, 1.0f); !res) return res.error().relay();
          if (auto res = draw_line({c.x, c.y + ah}, {c.x + aw, c.y - ah}, 1.0f); !res) return res.error().relay();
        }
      } else {
        if (Part == scrollbar_part::decrement_button) {
          if (auto res = draw_line({c.x + ah, c.y - aw}, {c.x - ah, c.y}, 1.0f); !res) return res.error().relay();
          if (auto res = draw_line({c.x - ah, c.y}, {c.x + ah, c.y + aw}, 1.0f); !res) return res.error().relay();
        } else {
          if (auto res = draw_line({c.x - ah, c.y - aw}, {c.x + ah, c.y}, 1.0f); !res) return res.error().relay();
          if (auto res = draw_line({c.x + ah, c.y}, {c.x - ah, c.y + aw}, 1.0f); !res) return res.error().relay();
        }
      }
      return {};
    }

    color color_for_button(scrollbar_part Part) const noexcept {
      if (pressed_part == Part) return button_pressed_color;
      if (hovered_part == Part) return button_hover_color;
      return button_color;
    }

    color color_for_thumb() const noexcept {
      if (pressed_part == scrollbar_part::thumb) return thumb_pressed_color;
      if (hovered_part == scrollbar_part::thumb) return thumb_hover_color;
      return thumb_color;
    }

    std::expected<void, error> draw_scrollbar() const {
      if (!visible || bar_width <= 0.0f) return {};
      brush::color(track_color);
      if (auto res = fill_rectangle(rect_pos(bar_rect()), rect_size(bar_rect())); !res) return res.error().relay();

      brush::color(color_for_button(scrollbar_part::decrement_button));
      if (auto res = fill_rectangle(rect_pos(decrement_button_rect()), rect_size(decrement_button_rect())); !res)
        return res.error().relay();
      brush::color(color_for_button(scrollbar_part::increment_button));
      if (auto res = fill_rectangle(rect_pos(increment_button_rect()), rect_size(increment_button_rect())); !res)
        return res.error().relay();

      const auto th = thumb_rect();
      if (rect_size(th).x > 0.0f && rect_size(th).y > 0.0f) {
        brush::color(color_for_thumb());
        if (auto res = fill_rectangle(rect_pos(th), rect_size(th)); !res) return res.error().relay();
      }
      if (auto res = draw_button_arrow(scrollbar_part::decrement_button); !res) return res.error().relay();
      if (auto res = draw_button_arrow(scrollbar_part::increment_button); !res) return res.error().relay();
      return {};
    }

    virtual bool focusable() const noexcept override { return enabled && visible; }

    virtual std::expected<void, error> relocate() override {
      if (auto res = frame::slot::relocate(); !res) return res.error().relay();
      update_scroll_view_size();
      return {};
    }

    virtual bool button_event(yw::button_event e) override {
      if (!enabled || !visible || e.key != keys::lbutton) return frame::slot::button_event(e);
      const auto pt = float2(float(e.pos.x), float(e.pos.y));
      if (e.down) {
        pressed_part = part_at(pt);
        if (pressed_part == scrollbar_part::none) return false;
        scroll_part(pressed_part);
        make_dirty();
        return true;
      }
      const bool had_pressed = pressed_part != scrollbar_part::none;
      pressed_part = scrollbar_part::none;
      if (had_pressed) make_dirty();
      return had_pressed;
    }

    virtual bool click_event(yw::button_event e) override {
      return enabled && visible && !e.down && e.key == keys::lbutton && rect_contains(bar_rect(), float2(e.pos));
    }

    virtual bool drag_event(yw::drag_event e) override {
      if (!enabled || !visible || e.key != keys::lbutton || pressed_part != scrollbar_part::thumb) return false;
      const auto movable = track_length() - thumb_length();
      const auto max_offset = max_scroll_offset();
      if (movable <= 0.0f || max_offset <= 0.0f) return true;
      const auto delta = float(Vertical ? e.delta.y : e.delta.x);
      scroll_by(delta * max_offset / movable);
      return true;
    }

    virtual bool hover_event(yw::hover_event e) override {
      const auto next = e.leave() ? scrollbar_part::none : part_at(float2(e.pos));
      if (hovered_part != next) {
        hovered_part = next;
        make_dirty();
      }
      return frame::slot::hover_event(e);
    }

    virtual std::expected<void, error> reset_state() override {
      if (pressed_part == scrollbar_part::none && hovered_part == scrollbar_part::none) return {};
      pressed_part = scrollbar_part::none;
      hovered_part = scrollbar_part::none;
      make_dirty();
      return {};
    }

    virtual bool wheel_event(yw::wheel_event e) override {
      if (!enabled || !visible || e.horizontal == Vertical || e.delta == 0) return false;
      const auto steps = float(e.delta) / 120.0f;
      return scroll_by(-steps * wheel_scroll_amount);
    }
  };

  using frame::operator bool;

  float scroll_offset() const noexcept { ywlib_control_get(scroll_offset); }
  float scroll_content_size() const noexcept { ywlib_control_get(scroll_content_size); }
  float scroll_view_size() const noexcept { ywlib_control_get(scroll_view_size); }
  float bar_width() const noexcept { ywlib_control_get(bar_width); }
  float wheel_scroll_amount() const noexcept { ywlib_control_get(wheel_scroll_amount); }
  const auto& track_color() const noexcept { ywlib_control_get(track_color); }
  const auto& button_color() const noexcept { ywlib_control_get(button_color); }
  const auto& button_hover_color() const noexcept { ywlib_control_get(button_hover_color); }
  const auto& button_pressed_color() const noexcept { ywlib_control_get(button_pressed_color); }
  const auto& thumb_color() const noexcept { ywlib_control_get(thumb_color); }
  const auto& thumb_hover_color() const noexcept { ywlib_control_get(thumb_hover_color); }
  const auto& thumb_pressed_color() const noexcept { ywlib_control_get(thumb_pressed_color); }
  const auto& arrow_color() const noexcept { ywlib_control_get(arrow_color); }

  float4 content_rect() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->content_rect();
  }

  float4 bar_rect() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->bar_rect();
  }

  auto& scroll_offset(this auto& self, float1 v) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->set_scroll_offset(v.x);
    return self;
  }

  auto& scroll_content_size(this auto& self, float1 v) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->update_scroll_content_size(v.x);
    sp->make_dirty();
    return self;
  }

  auto& bar_width(this auto& self, float1 v) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    if (v.x <= 0.0f) {
      error(errors::invalid_argument, "scrollbar bar_width must be positive").go_off();
      return self;
    }
    sp->bar_width = v.x;
    sp->make_messy();
    return self;
  }

  auto& wheel_scroll_amount(this auto& self, float1 v) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    if (v.x <= 0.0f) {
      error(errors::invalid_argument, "scrollbar wheel_scroll_amount must be positive").go_off();
      return self;
    }
    sp->wheel_scroll_amount = v.x;
    return self;
  }

  auto& track_color(this auto& self, const color& c) noexcept { ywlib_control_set(track_color, c, dirty); }
  auto& button_color(this auto& self, const color& c) noexcept { ywlib_control_set(button_color, c, dirty); }
  auto& button_hover_color(this auto& self, const color& c) noexcept { ywlib_control_set(button_hover_color, c, dirty); }
  auto& button_pressed_color(this auto& self, const color& c) noexcept {
    ywlib_control_set(button_pressed_color, c, dirty);
  }
  auto& thumb_color(this auto& self, const color& c) noexcept { ywlib_control_set(thumb_color, c, dirty); }
  auto& thumb_hover_color(this auto& self, const color& c) noexcept { ywlib_control_set(thumb_hover_color, c, dirty); }
  auto& thumb_pressed_color(this auto& self, const color& c) noexcept {
    ywlib_control_set(thumb_pressed_color, c, dirty);
  }
  auto& arrow_color(this auto& self, const color& c) noexcept { ywlib_control_set(arrow_color, c, dirty); }

protected:
  scrollbar() noexcept = default;
};

using vscrollbar = scrollbar<true>;
using hscrollbar = scrollbar<false>;
} // namespace yw::ui
