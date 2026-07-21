#pragma once
#include <ywx/ui_frame.h>

namespace yw::ui {

class scrollbar : public frame {
public:
  struct slot : frame::slot {
    enum class part : unsigned char {
      none = 0xff,
      horizontal_decrease = 0x00,
      horizontal_increase = 0x10,
      horizontal_track_decrease = 0x20,
      horizontal_track_increase = 0x30,
      horizontal_thumb = 0x40,
      vertical_decrease = 0x01,
      vertical_increase = 0x11,
      vertical_track_decrease = 0x21,
      vertical_track_increase = 0x31,
      vertical_thumb = 0x41,
    };

    color track_color{};
    color thumb_color{};
    color button_color{};
    color arrow_color{};
    color pressed_overlay_color{};
    float2 content_area{};  // == viewport_size - padding
    float2 viewport_size{}; // == size - scrollbar_area == content_area + padding
    float2 track_offset{};  // {x position of horizontal track, y position of vertical track}
    float2 track_length{};  // {length of horizontal track, length of vertical track}
    float2 thumb_offset{};  // {x position of horizontal thumb, y position of vertical thumb}
    float2 thumb_length{};  // {length of horizontal thumb, length of vertical thumb
    float2 scroll_offset{};
    float bar_width = 16.0f;
    float line_scroll = 16.0f;
    float page_scroll = 0.0f;
    float wheel_scroll = 48.0f;
    vector2<bool> bar_visible{};
    part pressed_part = part::none;
    part hovered_part = part::none;

    //-- helper functions --//

    static bool _is_thumb_part(part Part) noexcept { return (static_cast<unsigned char>(Part) & 0x40) != 0; }

    static vector2<bool> _horizontal_or_vertical(part Part) noexcept {
      return {(static_cast<unsigned char>(Part) & 0x01) == 0, (static_cast<unsigned char>(Part) & 0x01) == 1};
    }

    float2 _max_scroll() const noexcept {
      return vapply_r<float2>(yw::max, float2{}, get_content_size() - content_area);
    }

    void _clamp_scroll() noexcept {
      scroll_offset = vapply_r<float2>(yw::clamp, scroll_offset, float2{}, _max_scroll());
    }

    template<bool Vertical> part _part_hittest(float p) const noexcept {
      if (p <= pos[Vertical] + bar_width) return static_cast<part>(0x00 + Vertical); // decrease button
      if (p < thumb_offset[Vertical]) return static_cast<part>(0x20 + Vertical);     // track decrease
      if (p <= thumb_offset[Vertical] + thumb_length[Vertical]) return static_cast<part>(0x40 + Vertical); // thumb
      const auto track_end = track_offset[Vertical] + track_length[Vertical];
      if (p < track_end) return static_cast<part>(0x30 + Vertical);              // track increase
      if (p <= track_end + bar_width) return static_cast<part>(0x10 + Vertical); // increase button
      return part::none;
    }

    part _part_hittest(float2 Pt) const noexcept {
      const auto pt = Pt - pos;
      if (viewport_size.x <= pt.x && pt.x <= size.x && bar_visible.y) return _part_hittest<true>(Pt.y);
      else if (viewport_size.y <= pt.y && pt.y <= size.y && bar_visible.x) return _part_hittest<false>(Pt.x);
      return part::none;
    }

    void update_scrollbar_metrics() {
      const auto content = get_content_size();
      content_area = size - padding.xy() - padding.zw();
      bar_visible = {content.x > content_area.x, content.y > content_area.y};
      content_area -= bar_width * bar_visible.yx();
      bar_visible = {content.x > content_area.x, content.y > content_area.y};
      viewport_size = size - bar_width * bar_visible.yx();
      content_area = viewport_size - padding.xy() - padding.zw();
      _clamp_scroll();
      const auto bar_size = float2::fill(bar_width);
      track_offset = pos + bar_size;
      track_length = viewport_size - 2 * bar_size;
      if (bar_visible.x) {
        thumb_length.x = yw::clamp(content_area.x * track_length.x / content.x, 0.0f, track_length.x);
        thumb_offset.x = track_offset.x + scroll_offset.x * (track_length.x - thumb_length.x) / (content.x - content_area.x);
      } else {
        thumb_length.x = 0.0f;
        thumb_offset.x = track_offset.x;
      }
      if (bar_visible.y) {
        thumb_length.y = yw::clamp(content_area.y * track_length.y / content.y, 0.0f, track_length.y);
        thumb_offset.y = track_offset.y + scroll_offset.y * (track_length.y - thumb_length.y) / (content.y - content_area.y);
      } else {
        thumb_length.y = 0.0f;
        thumb_offset.y = track_offset.y;
      }
    }

    virtual float2 get_content_size() const { return float2{}; }

    template<bool Vertical> float page_amount() const noexcept {
      if (page_scroll > 0.0f) return page_scroll;
      return yw::max(line_scroll, content_area[Vertical]);
    }

    bool scroll_by(float2 Delta) noexcept {
      const auto old = scroll_offset;
      scroll_offset = vapply_r<float2>(yw::clamp, scroll_offset + Delta, float2(), _max_scroll());
      if (scroll_offset == old) return false;
      update_scrollbar_metrics();
      make_dirty();
      return true;
    }

    bool scroll_to(float2 Offset) noexcept {
      const auto old = scroll_offset;
      scroll_offset = vapply_r<float2>(yw::clamp, Offset, float2(), _max_scroll());
      if (scroll_offset == old) return false;
      update_scrollbar_metrics();
      make_dirty();
      return true;
    }

    bool scroll_part(part Part) noexcept {
      switch (Part) {
      case part::vertical_decrease: return scroll_by({0.0f, -line_scroll});
      case part::vertical_increase: return scroll_by({0.0f, line_scroll});
      case part::vertical_track_decrease: return scroll_by({0.0f, -page_amount<true>()});
      case part::vertical_track_increase: return scroll_by({0.0f, page_amount<true>()});
      case part::horizontal_decrease: return scroll_by({-line_scroll, 0.0f});
      case part::horizontal_increase: return scroll_by({line_scroll, 0.0f});
      case part::horizontal_track_decrease: return scroll_by({-page_amount<false>(), 0.0f});
      case part::horizontal_track_increase: return scroll_by({page_amount<false>(), 0.0f});
      default: return false;
      }
    }

    template<bool Vertical, bool Increase> std::expected<void, error> _draw_scrollbar_arrow(float2 Pos) const {
      const auto a = bar_width * 0.15f;
      const auto b = bar_width * 0.26f;
      const auto c = Pos + float2::fill(bar_width * 0.5f);
      brush::color(arrow_color);
      if constexpr (Vertical && Increase) {
        const auto tip = c + float2(0.0f, bar_width * 0.3f);
        if (auto res = draw_line(tip, c + float2(-b, -a), 1.0f); !res) return res.error().relay();
        if (auto res = draw_line(tip, c + float2(b, -a), 1.0f); !res) return res.error().relay();
      } else if constexpr (Vertical) {
        const auto tip = c + float2(0.0f, -bar_width * 0.3f);
        if (auto res = draw_line(tip, c + float2(-b, a), 1.0f); !res) return res.error().relay();
        if (auto res = draw_line(tip, c + float2(b, a), 1.0f); !res) return res.error().relay();
      } else if constexpr (Increase) {
        const auto tip = c + float2(bar_width * 0.3f, 0.0f);
        if (auto res = draw_line(tip, c + float2(-a, -b), 1.0f); !res) return res.error().relay();
        if (auto res = draw_line(tip, c + float2(-a, b), 1.0f); !res) return res.error().relay();
      } else {
        const auto tip = c + float2(-bar_width * 0.3f, 0.0f);
        if (auto res = draw_line(tip, c + float2(a, -b), 1.0f); !res) return res.error().relay();
        if (auto res = draw_line(tip, c + float2(a, b), 1.0f); !res) return res.error().relay();
      }
      return {};
    }

    template<bool Vertical, bool Increase> std::expected<void, error> _draw_scrollbar_button(float2 Pos) const {
      brush::color(button_color);
      const auto button_size = float2::fill(bar_width);
      if (auto res = fill_rectangle(Pos, button_size); !res) return res.error().relay();
      if ((Vertical && Increase && pressed_part == part::vertical_increase) ||
          (Vertical && !Increase && pressed_part == part::vertical_decrease) ||
          (!Vertical && Increase && pressed_part == part::horizontal_increase) ||
          (!Vertical && !Increase && pressed_part == part::horizontal_decrease)) {
        brush::color(pressed_overlay_color);
        if (auto res = fill_rectangle(Pos, button_size); !res) return res.error().relay();
      } else if ((Vertical && Increase && hovered_part == part::vertical_increase) ||
                 (Vertical && !Increase && hovered_part == part::vertical_decrease) ||
                 (!Vertical && Increase && hovered_part == part::horizontal_increase) ||
                 (!Vertical && !Increase && hovered_part == part::horizontal_decrease)) {
        brush::color(hovered_overlay_color);
        if (auto res = fill_rectangle(Pos, button_size); !res) return res.error().relay();
      }
      if (auto res = _draw_scrollbar_arrow<Vertical, Increase>(Pos); !res) return res.error().relay();
      return {};
    }

    template<bool Vertical> std::expected<void, error> _draw_scrollbar_track(float2 Pos) const {
      brush::color(track_color);
      const auto track_end = Pos + track_length.insert<!Vertical>(bar_width);
      if (auto res = fill_rectangle(float4(Pos, track_end)); !res) return res.error().relay();
      if ((Vertical && pressed_part == part::vertical_track_decrease) ||
          (!Vertical && pressed_part == part::horizontal_track_decrease)) {
        brush::color(pressed_overlay_color);
        const auto rect = float4(Pos, thumb_offset.insert<!Vertical>(track_end[!Vertical]));
        if (auto res = fill_rectangle(rect); !res) return res.error().relay();
      } else if ((Vertical && pressed_part == part::vertical_track_increase) ||
                 (!Vertical && pressed_part == part::horizontal_track_increase)) {
        brush::color(pressed_overlay_color);
        const auto thumb_end = Pos.insert<Vertical>(thumb_offset[Vertical] + thumb_length[Vertical]);
        if (auto res = fill_rectangle(float4(thumb_end, track_end)); !res) return res.error().relay();
      } else if ((Vertical && hovered_part == part::vertical_track_decrease) ||
                 (!Vertical && hovered_part == part::horizontal_track_decrease)) {
        brush::color(hovered_overlay_color);
        const auto rect = float4(Pos, thumb_offset.insert<!Vertical>(track_end[!Vertical]));
        if (auto res = fill_rectangle(rect); !res) return res.error().relay();
      } else if ((Vertical && hovered_part == part::vertical_track_increase) ||
                 (!Vertical && hovered_part == part::horizontal_track_increase)) {
        brush::color(hovered_overlay_color);
        const auto thumb_end = Pos.insert<Vertical>(thumb_offset[Vertical] + thumb_length[Vertical]);
        if (auto res = fill_rectangle(float4(thumb_end, track_end)); !res) return res.error().relay();
      }
      return {};
    }

    template<bool Vertical> std::expected<void, error> _draw_scrollbar_thumb(float2 Pos) const {
      brush::color(thumb_color);
      const auto thumb_size = thumb_length.insert<!Vertical>(float(bar_width));
      if (auto res = fill_rectangle(Pos, thumb_size); !res) return res.error().relay();
      if ((Vertical && pressed_part == part::vertical_thumb) ||
          (!Vertical && pressed_part == part::horizontal_thumb)) {
        brush::color(pressed_overlay_color);
        if (auto res = fill_rectangle(Pos, thumb_size); !res) return res.error().relay();
      } else if ((Vertical && hovered_part == part::vertical_thumb) ||
                 (!Vertical && hovered_part == part::horizontal_thumb)) {
        brush::color(hovered_overlay_color);
        if (auto res = fill_rectangle(Pos, thumb_size); !res) return res.error().relay();
      }
      return {};
    }

    template<bool Vertical> std::expected<void, error> draw_bar() const {
      // background
      brush::color(background_color);
      const float2 origin = pos.add<!Vertical>(size[!Vertical] - bar_width);
      const auto bar_size = Vertical ? float2{bar_width, size.y} : float2{size.x, bar_width};
      if (auto res = fill_rectangle(origin, bar_size); !res) return res.error().relay();
      // decrease button
      if (auto res = _draw_scrollbar_button<Vertical, false>(origin); !res) return res.error().relay();
      // track
      const auto track_pos = track_offset.insert<!Vertical>(origin[!Vertical]);
      if (auto res = _draw_scrollbar_track<Vertical>(track_pos); !res) return res.error().relay();
      // thumb
      const auto thumb_pos = thumb_offset.insert<!Vertical>(origin[!Vertical]);
      if (auto res = _draw_scrollbar_thumb<Vertical>(thumb_pos); !res) return res.error().relay();
      // increase button
      const auto inc_pos = origin.insert<Vertical>(track_offset[Vertical] + track_length[Vertical]);
      if (auto res = _draw_scrollbar_button<Vertical, true>(inc_pos); !res) return res.error().relay();
      return {};
    }

    std::expected<void, error> draw_scrollbars() {
      if (bar_visible.x) {
        if (bar_visible.y) {
          brush::color(background_color);
          const auto bar_size = float2::fill(bar_width);
          if (auto res = fill_rectangle(pos + size - bar_size, bar_size); !res) return res.error().relay();
          if (auto res = draw_bar<true>(); !res) return res.error().relay();
        }
        if (auto res = draw_bar<false>(); !res) return res.error().relay();
      } else if (bar_visible.y)
        if (auto res = draw_bar<true>(); !res) return res.error().relay();
      return {};
    }

    //-- override functions --//

    virtual std::expected<void, error> draw_hovered_overlay() override { return {}; }

    virtual float2 get_minimum_size() const override {
      const auto content = padding.xy() + padding.zw() + float2::fill(arbitrary_value);
      return vapply_r<float2>(yw::max, content + float2::fill(bar_width), float2::fill(bar_width * 4.0f));
    }

    virtual std::expected<float2, error> get_necessary_size() const override {
      const auto inner = get_content_size() + padding.xy() + padding.zw() + float2::fill(bar_width);
      const auto fit = bool2(policy.x == size_policy::fit, policy.y == size_policy::fit);
      return calc_necessary_size_by_policy(inner * fit);
    }

    virtual std::expected<void, error> redraw() override {
      if (geometry_dirty) {
        geometry_dirty = false;
        if (auto res = relocate(); !res) return res.error().relay();
      }
      if (!visible) return {};
      if (auto res = draw_frame_background(); !res) return res.error().relay();
      if (auto res = draw_scrollbars(); !res) return res.error().relay();
      if (auto res = draw_frame_foreground(); !res) return res.error().relay();
      return {};
    }

    virtual std::expected<void, error> relocate() override {
      if (auto res = frame::slot::relocate(); !res) return res.error().relay();
      update_scrollbar_metrics();
      return {};
    }

    virtual std::expected<void, error> apply_color_theme(const yw::ui::color_theme& Theme, bool Recursive) override {
      background_color = Theme.surface;
      border_color = Theme.outline;
      hovered_overlay_color = color(Theme.accent, default_overlay_opacity.hover);
      track_color = Theme.surface;
      thumb_color = color(Theme.outline, 0.5f);
      button_color = color(Theme.outline, 0.5f);
      arrow_color = Theme.text;
      pressed_overlay_color = color(Theme.accent, default_overlay_opacity.pressed);
      make_dirty();
      return {};
    }

    virtual bool button_event(yw::button_event e) override {
      if (!enabled || !visible || e.key != keys::lbutton) return frame::slot::button_event(e);
      const auto hit = _part_hittest(e.pos);
      if (e.down) {
        if (hit == part::none) return frame::slot::button_event(e);
        pressed_part = hit;
        scroll_part(hit);
        make_dirty();
        return true;
      } else if (pressed_part == part::none) return frame::slot::button_event(e);
      pressed_part = part::none;
      make_dirty();
      return true;
    }

    virtual bool click_event(yw::button_event e) override {
      if (!enabled || !visible || e.down || e.key != keys::lbutton) return frame::slot::click_event(e);
      return _part_hittest(e.pos) != part::none;
    }

    virtual bool drag_event(yw::drag_event e) override {
      if (!enabled || !visible || !_is_thumb_part(pressed_part)) return frame::slot::drag_event(e);
      const auto axis = _horizontal_or_vertical(pressed_part);
      const auto movable_length = track_length - thumb_length;
      return scroll_by(axis * e.delta * _max_scroll() / movable_length) || true;
    }

    virtual std::expected<void, error> reset_state() override {
      if (pressed_part == part::none) return {};
      pressed_part = part::none;
      make_dirty();
      return {};
    }

    virtual void focus_event(bool Focused) override {
      if (!Focused && pressed_part != part::none) {
        pressed_part = part::none;
        make_dirty();
      }
      frame::slot::focus_event(Focused);
    }

    virtual bool hover_event(yw::hover_event e) override {
      if (const auto next_hovered = e.leave() ? part::none : _part_hittest(e.pos); hovered_part != next_hovered) {
        hovered_part = next_hovered;
        make_dirty();
      }
      return frame::slot::hover_event(e);
    }

    virtual bool key_event(yw::key_event e) override {
      if (!enabled || !visible) return false;
      if (!e.down) {
        if (e.key == keys::up || e.key == keys::down || e.key == keys::left || e.key == keys::right ||
            e.key == keys::page_up || e.key == keys::page_down || e.key == keys::home || e.key == keys::end)
          return true;
        return frame::slot::key_event(e);
      }
      if (e.key == keys::up) return scroll_by({0.0f, -line_scroll}), true;
      if (e.key == keys::down) return scroll_by({0.0f, line_scroll}), true;
      if (e.key == keys::left) return scroll_by({-line_scroll, 0.0f}), true;
      if (e.key == keys::right) return scroll_by({line_scroll, 0.0f}), true;
      if (e.key == keys::page_up) return scroll_by({0.0f, -page_amount<true>()}), true;
      if (e.key == keys::page_down) return scroll_by({0.0f, page_amount<true>()}), true;
      if (e.key == keys::home) return scroll_to(float2{}), true;
      if (e.key == keys::end) return scroll_to(_max_scroll()), true;
      return frame::slot::key_event(e);
    }

    virtual bool wheel_event(yw::wheel_event e) override {
      if (!enabled || !visible || e.delta == 0) return false;
      const float amount = wheel_scroll * (e.mods.shift ? 10.0f : 1.0f) * (e.delta > 0 ? -1.0f : 1.0f);
      if (e.horizontal) return scroll_by({amount, 0.0f}) || bar_visible.x;
      return scroll_by({0.0f, amount}) || bar_visible.y;
    }
  };

  //-- getter --//

  float2 scroll_offset() const noexcept { ywlib_control_get(scroll_offset); }
  float bar_width() const noexcept { ywlib_control_get(bar_width); }
  float line_scroll() const noexcept { ywlib_control_get(line_scroll); }
  float page_scroll() const noexcept { ywlib_control_get(page_scroll); }
  float wheel_scroll() const noexcept { ywlib_control_get(wheel_scroll); }
  const auto& track_color() const noexcept { ywlib_control_get(track_color); }
  const auto& thumb_color() const noexcept { ywlib_control_get(thumb_color); }
  const auto& arrow_color() const noexcept { ywlib_control_get(arrow_color); }
  const auto& button_color() const noexcept { ywlib_control_get(button_color); }
  const auto& pressed_overlay_color() const noexcept { ywlib_control_get(pressed_overlay_color); }

  //-- setter --//

  auto& scroll_offset(this auto& self, float2 v) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->scroll_offset = v;
    sp->update_scrollbar_metrics();
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
    sp->update_scrollbar_metrics();
    sp->make_messy();
    return self;
  }

  auto& line_scroll(this auto& self, float1 v) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    if (v.x <= 0.0f) {
      error(errors::invalid_argument, "scrollbar line_scroll must be positive").go_off();
      return self;
    }
    sp->line_scroll = v.x;
    return self;
  }

  auto& page_scroll(this auto& self, float1 v) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    if (v.x < 0.0f) {
      error(errors::invalid_argument, "scrollbar page_scroll must be non-negative").go_off();
      return self;
    }
    sp->page_scroll = v.x;
    return self;
  }

  auto& wheel_scroll(this auto& self, float1 v) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    if (v.x <= 0.0f) {
      error(errors::invalid_argument, "scrollbar wheel_scroll must be positive").go_off();
      return self;
    }
    sp->wheel_scroll = v.x;
    return self;
  }

  auto& track_color(this auto& self, const color& c) noexcept { ywlib_control_set(track_color, c, dirty); }
  auto& thumb_color(this auto& self, const color& c) noexcept { ywlib_control_set(thumb_color, c, dirty); }
  auto& arrow_color(this auto& self, const color& c) noexcept { ywlib_control_set(arrow_color, c, dirty); }
  auto& button_color(this auto& self, const color& c) noexcept { ywlib_control_set(button_color, c, dirty); }
  auto& pressed_overlay_color(this auto& self, const color& c) noexcept {
    ywlib_control_set(pressed_overlay_color, c, dirty);
  }
};
} // namespace yw::ui
