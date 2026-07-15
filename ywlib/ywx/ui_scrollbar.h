#pragma once
#include <ywx/ui_frame.h>

namespace yw::ui {

class scrollbar : public frame {
public:
  struct slot : frame::slot {
    enum class part : unsigned char {
      none,
      vertical_decrease,
      vertical_increase,
      vertical_track_decrease,
      vertical_track_increase,
      vertical_thumb,
      horizontal_decrease,
      horizontal_increase,
      horizontal_track_decrease,
      horizontal_track_increase,
      horizontal_thumb,
    };

    struct metrics {
      float2 content_size{};
      float2 inner_pos{};
      float2 inner_size{};
      float2 viewport_pos{};
      float2 viewport_size{};
      bool vertical = false;
      bool horizontal = false;
    };

    float2 scroll_offset{};
    float bar_width = 16.0f;
    float line_scroll = 16.0f;
    float page_scroll = 0.0f;
    float wheel_scroll = 48.0f;
    color track_color = color(0.0f, 0.0f, 0.0f, 0.06f);
    color thumb_color = color(0.0f, 0.0f, 0.0f, 0.28f);
    color button_color = color(0.0f, 0.0f, 0.0f, 0.10f);
    color pressed_color = color(0.0f, 0.0f, 0.0f, 0.12f);
    part pressed_part = part::none;

    static bool contains(float4 Rect, float2 Pt) noexcept {
      return Pt.x >= Rect.x && Pt.y >= Rect.y && Pt.x <= Rect.z && Pt.y <= Rect.w;
    }

    static float2 rect_pos(float4 Rect) noexcept { return {Rect.x, Rect.y}; }
    static float2 rect_size(float4 Rect) noexcept { return {Rect.z - Rect.x, Rect.w - Rect.y}; }

    static bool vertical_part(part Part) noexcept {
      return Part == part::vertical_decrease || Part == part::vertical_increase ||
             Part == part::vertical_track_decrease || Part == part::vertical_track_increase ||
             Part == part::vertical_thumb;
    }

    static bool horizontal_part(part Part) noexcept {
      return Part == part::horizontal_decrease || Part == part::horizontal_increase ||
             Part == part::horizontal_track_decrease || Part == part::horizontal_track_increase ||
             Part == part::horizontal_thumb;
    }

    static bool thumb_part(part Part) noexcept {
      return Part == part::vertical_thumb || Part == part::horizontal_thumb;
    }

    virtual std::expected<float2, error> get_content_size() const { return float2{}; }

    float2 max_scroll(const metrics& m) const noexcept {
      return {yw::max(0.0f, m.content_size.x - m.viewport_size.x), yw::max(0.0f, m.content_size.y - m.viewport_size.y)};
    }

    void clamp_scroll(const metrics& m) noexcept {
      const auto max = max_scroll(m);
      scroll_offset.x = yw::clamp(scroll_offset.x, 0.0f, max.x);
      scroll_offset.y = yw::clamp(scroll_offset.y, 0.0f, max.y);
    }

    std::expected<metrics, error> get_metrics() const {
      metrics m;
      if (auto res = get_content_size()) m.content_size = *res;
      else return res.error().relay();
      m.inner_pos = pos + padding.xy();
      m.inner_size = size - padding.xy() - padding.zw();
      m.inner_size.x = yw::max(0.0f, m.inner_size.x);
      m.inner_size.y = yw::max(0.0f, m.inner_size.y);

      for (int i = 0; i < 2; ++i) {
        m.viewport_size = m.inner_size - float2(m.vertical ? bar_width : 0.0f, m.horizontal ? bar_width : 0.0f);
        m.viewport_size.x = yw::max(0.0f, m.viewport_size.x);
        m.viewport_size.y = yw::max(0.0f, m.viewport_size.y);
        m.horizontal = m.content_size.x > m.viewport_size.x;
        m.vertical = m.content_size.y > m.viewport_size.y;
      }
      m.viewport_size = m.inner_size - float2(m.vertical ? bar_width : 0.0f, m.horizontal ? bar_width : 0.0f);
      m.viewport_size.x = yw::max(0.0f, m.viewport_size.x);
      m.viewport_size.y = yw::max(0.0f, m.viewport_size.y);
      m.viewport_pos = m.inner_pos;
      return m;
    }

    float4 vertical_rect(const metrics& m) const noexcept {
      const float left = m.inner_pos.x + m.inner_size.x - bar_width;
      const float bottom = m.inner_pos.y + m.inner_size.y - (m.horizontal ? bar_width : 0.0f);
      return {left, m.inner_pos.y, left + bar_width, bottom};
    }

    float4 horizontal_rect(const metrics& m) const noexcept {
      const float top = m.inner_pos.y + m.inner_size.y - bar_width;
      const float right = m.inner_pos.x + m.inner_size.x - (m.vertical ? bar_width : 0.0f);
      return {m.inner_pos.x, top, right, top + bar_width};
    }

    float4 corner_rect(const metrics& m) const noexcept {
      return {
        m.inner_pos.x + m.inner_size.x - bar_width, m.inner_pos.y + m.inner_size.y - bar_width,
        m.inner_pos.x + m.inner_size.x, m.inner_pos.y + m.inner_size.y};
    }

    float4 decrease_button_rect(float4 Bar) const noexcept {
      return {Bar.x, Bar.y, Bar.x + bar_width, Bar.y + bar_width};
    }

    float4 increase_button_rect(float4 Bar, bool Vertical) const noexcept {
      if (Vertical) return {Bar.x, Bar.w - bar_width, Bar.z, Bar.w};
      return {Bar.z - bar_width, Bar.y, Bar.z, Bar.w};
    }

    float track_length(float4 Bar, bool Vertical) const noexcept {
      return yw::max(0.0f, (Vertical ? Bar.w - Bar.y : Bar.z - Bar.x) - bar_width * 2.0f);
    }

    float4 thumb_rect(const metrics& m, bool Vertical) const noexcept {
      const auto bar = Vertical ? vertical_rect(m) : horizontal_rect(m);
      const float track = track_length(bar, Vertical);
      if (track <= 0.0f) return {};
      const float viewport = Vertical ? m.viewport_size.y : m.viewport_size.x;
      const float content = Vertical ? m.content_size.y : m.content_size.x;
      const float max = yw::max(0.0f, content - viewport);
      const float thumb = content <= 0.0f ? track : track * yw::clamp(viewport / content, 0.0f, 1.0f);
      const float movable = yw::max(0.0f, track - thumb);
      const float value = Vertical ? scroll_offset.y : scroll_offset.x;
      const float offset = max <= 0.0f ? 0.0f : movable * value / max;
      if (Vertical) {
        const float y = bar.y + bar_width + offset;
        return {bar.x, y, bar.z, y + thumb};
      }
      const float x = bar.x + bar_width + offset;
      return {x, bar.y, x + thumb, bar.w};
    }

    part hit_test(float2 Pt, const metrics& m) const noexcept {
      if (m.vertical) {
        const auto bar = vertical_rect(m);
        if (contains(bar, Pt)) {
          if (contains(decrease_button_rect(bar), Pt)) return part::vertical_decrease;
          if (contains(increase_button_rect(bar, true), Pt)) return part::vertical_increase;
          const auto thumb = thumb_rect(m, true);
          if (contains(thumb, Pt)) return part::vertical_thumb;
          return Pt.y < thumb.y ? part::vertical_track_decrease : part::vertical_track_increase;
        }
      }
      if (m.horizontal) {
        const auto bar = horizontal_rect(m);
        if (contains(bar, Pt)) {
          if (contains(decrease_button_rect(bar), Pt)) return part::horizontal_decrease;
          if (contains(increase_button_rect(bar, false), Pt)) return part::horizontal_increase;
          const auto thumb = thumb_rect(m, false);
          if (contains(thumb, Pt)) return part::horizontal_thumb;
          return Pt.x < thumb.x ? part::horizontal_track_decrease : part::horizontal_track_increase;
        }
      }
      return part::none;
    }

    float page_amount(const metrics& m, bool Vertical) const noexcept {
      if (page_scroll > 0.0f) return page_scroll;
      const float viewport = Vertical ? m.viewport_size.y : m.viewport_size.x;
      return yw::max(line_scroll, viewport - bar_width);
    }

    bool scroll_by(float2 Delta, const metrics& m) noexcept {
      const auto old = scroll_offset;
      const auto max = max_scroll(m);
      scroll_offset.x = yw::clamp(scroll_offset.x + Delta.x, 0.0f, max.x);
      scroll_offset.y = yw::clamp(scroll_offset.y + Delta.y, 0.0f, max.y);
      if (scroll_offset == old) return false;
      make_dirty();
      return true;
    }

    bool scroll_part(part Part, const metrics& m) noexcept {
      switch (Part) {
      case part::vertical_decrease: return scroll_by({0.0f, -line_scroll}, m);
      case part::vertical_increase: return scroll_by({0.0f, line_scroll}, m);
      case part::vertical_track_decrease: return scroll_by({0.0f, -page_amount(m, true)}, m);
      case part::vertical_track_increase: return scroll_by({0.0f, page_amount(m, true)}, m);
      case part::horizontal_decrease: return scroll_by({-line_scroll, 0.0f}, m);
      case part::horizontal_increase: return scroll_by({line_scroll, 0.0f}, m);
      case part::horizontal_track_decrease: return scroll_by({-page_amount(m, false), 0.0f}, m);
      case part::horizontal_track_increase: return scroll_by({page_amount(m, false), 0.0f}, m);
      default: return false;
      }
    }

    std::expected<void, error> fill_rect(float4 Rect, const color& Color) const {
      if (Color.a <= 0.0f || Rect.z <= Rect.x || Rect.w <= Rect.y) return {};
      brush::color(Color);
      if (auto res = fill_rectangle(rect_pos(Rect), rect_size(Rect)); !res) return res.error().relay();
      return {};
    }

    std::expected<void, error> draw_arrow(float4 Rect, bool Vertical, bool Increase) const {
      brush::color(colors.border);
      const float cx = (Rect.x + Rect.z) * 0.5f;
      const float cy = (Rect.y + Rect.w) * 0.5f;
      const float aw = yw::clamp(bar_width * 0.22f, 3.0f, 6.0f);
      const float ah = yw::clamp(bar_width * 0.18f, 2.0f, 5.0f);
      if (Vertical && !Increase) {
        if (auto res = draw_line({cx - aw, cy + ah}, {cx, cy - ah}, 1.0f); !res) return res.error().relay();
        if (auto res = draw_line({cx, cy - ah}, {cx + aw, cy + ah}, 1.0f); !res) return res.error().relay();
      } else if (Vertical) {
        if (auto res = draw_line({cx - aw, cy - ah}, {cx, cy + ah}, 1.0f); !res) return res.error().relay();
        if (auto res = draw_line({cx, cy + ah}, {cx + aw, cy - ah}, 1.0f); !res) return res.error().relay();
      } else if (!Increase) {
        if (auto res = draw_line({cx + ah, cy - aw}, {cx - ah, cy}, 1.0f); !res) return res.error().relay();
        if (auto res = draw_line({cx - ah, cy}, {cx + ah, cy + aw}, 1.0f); !res) return res.error().relay();
      } else {
        if (auto res = draw_line({cx - ah, cy - aw}, {cx + ah, cy}, 1.0f); !res) return res.error().relay();
        if (auto res = draw_line({cx + ah, cy}, {cx - ah, cy + aw}, 1.0f); !res) return res.error().relay();
      }
      return {};
    }

    std::expected<void, error> draw_bar(const metrics& m, bool Vertical) const {
      const auto bar = Vertical ? vertical_rect(m) : horizontal_rect(m);
      const auto dec = decrease_button_rect(bar);
      const auto inc = increase_button_rect(bar, Vertical);
      if (auto res = fill_rect(bar, track_color); !res) return res.error().relay();
      if (auto res = fill_rect(dec, button_color); !res) return res.error().relay();
      if (auto res = fill_rect(inc, button_color); !res) return res.error().relay();
      if (auto res = fill_rect(thumb_rect(m, Vertical), thumb_color); !res) return res.error().relay();
      if (pressed_part != part::none &&
          ((Vertical && vertical_part(pressed_part)) || (!Vertical && horizontal_part(pressed_part)))) {
        const auto pressed_rect =
          thumb_part(pressed_part)                                                           ? thumb_rect(m, Vertical)
          : pressed_part == (Vertical ? part::vertical_decrease : part::horizontal_decrease) ? dec
          : pressed_part == (Vertical ? part::vertical_increase : part::horizontal_increase) ? inc
                                                                                             : bar;
        if (auto res = fill_rect(pressed_rect, pressed_color); !res) return res.error().relay();
      }
      if (auto res = draw_arrow(dec, Vertical, false); !res) return res.error().relay();
      if (auto res = draw_arrow(inc, Vertical, true); !res) return res.error().relay();
      return {};
    }

    std::expected<void, error> draw_scrollbars() {
      metrics m;
      if (auto res = get_metrics()) {
        m = *res;
        clamp_scroll(m);
      } else return res.error().relay();
      if (m.vertical && m.horizontal) {
        if (auto res = fill_rect(corner_rect(m), colors.background); !res) return res.error().relay();
      }
      if (m.vertical)
        if (auto res = draw_bar(m, true); !res) return res.error().relay();
      if (m.horizontal)
        if (auto res = draw_bar(m, false); !res) return res.error().relay();
      return {};
    }

    //-- override functions --//

    virtual std::expected<float2, error> get_necessary_size() const override {
      const auto inner = padding.xy() + padding.zw() + float2::fill(bar_width * 3.0f + arbitrary_value);
      return calc_necessary_size_by_policy(inner);
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
      if (auto res = get_metrics()) clamp_scroll(*res);
      else return res.error().relay();
      return {};
    }

    virtual bool button_event(yw::button_event e) override {
      if (!enabled || !visible || e.key != keys::lbutton) return frame::slot::button_event(e);
      const auto pt = float2(float(e.pos.x), float(e.pos.y));
      auto mres = get_metrics();
      if (!mres) {
        mres.error().go_off();
        return false;
      }
      auto m = *mres;
      clamp_scroll(m);
      const auto hit = hit_test(pt, m);
      if (e.down) {
        if (hit == part::none) return frame::slot::button_event(e);
        pressed_part = hit;
        scroll_part(hit, m);
        make_dirty();
        return true;
      }
      if (pressed_part == part::none) return frame::slot::button_event(e);
      pressed_part = part::none;
      make_dirty();
      return true;
    }

    virtual bool click_event(yw::button_event e) override {
      if (!enabled || !visible || e.down || e.key != keys::lbutton) return frame::slot::click_event(e);
      auto mres = get_metrics();
      if (!mres) {
        mres.error().go_off();
        return false;
      }
      return hit_test(float2(float(e.pos.x), float(e.pos.y)), *mres) != part::none;
    }

    virtual bool drag_event(yw::drag_event e) override {
      if (!enabled || !visible || !thumb_part(pressed_part)) return frame::slot::drag_event(e);
      auto mres = get_metrics();
      if (!mres) {
        mres.error().go_off();
        return false;
      }
      const auto m = *mres;
      const bool vertical = pressed_part == part::vertical_thumb;
      const auto bar = vertical ? vertical_rect(m) : horizontal_rect(m);
      const float track = track_length(bar, vertical);
      const auto thumb = thumb_rect(m, vertical);
      const float thumb_len = vertical ? thumb.w - thumb.y : thumb.z - thumb.x;
      const float movable = yw::max(0.0f, track - thumb_len);
      if (movable <= 0.0f) return true;
      const auto max = max_scroll(m);
      const float delta = vertical ? float(e.delta.y) : float(e.delta.x);
      const float scroll = delta * (vertical ? max.y : max.x) / movable;
      if (vertical) scroll_by({0.0f, scroll}, m);
      else scroll_by({scroll, 0.0f}, m);
      return true;
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

    virtual bool key_event(yw::key_event e) override {
      if (!enabled || !visible) return false;
      if (!e.down) {
        if (e.key == keys::up || e.key == keys::down || e.key == keys::left || e.key == keys::right ||
            e.key == keys::page_up || e.key == keys::page_down || e.key == keys::home || e.key == keys::end)
          return true;
        return frame::slot::key_event(e);
      }
      auto mres = get_metrics();
      if (!mres) {
        mres.error().go_off();
        return false;
      }
      const auto m = *mres;
      if (e.key == keys::up) return scroll_by({0.0f, -line_scroll}, m), true;
      if (e.key == keys::down) return scroll_by({0.0f, line_scroll}, m), true;
      if (e.key == keys::left) return scroll_by({-line_scroll, 0.0f}, m), true;
      if (e.key == keys::right) return scroll_by({line_scroll, 0.0f}, m), true;
      if (e.key == keys::page_up) return scroll_by({0.0f, -page_amount(m, true)}, m), true;
      if (e.key == keys::page_down) return scroll_by({0.0f, page_amount(m, true)}, m), true;
      if (e.key == keys::home) return scroll_offset = {}, clamp_scroll(m), make_dirty(), true;
      if (e.key == keys::end) return scroll_offset = max_scroll(m), make_dirty(), true;
      return frame::slot::key_event(e);
    }

    virtual bool wheel_event(yw::wheel_event e) override {
      if (!enabled || !visible || e.delta == 0) return false;
      auto mres = get_metrics();
      if (!mres) {
        mres.error().go_off();
        return false;
      }
      const auto m = *mres;
      const float amount = wheel_scroll * (e.mods.shift ? 10.0f : 1.0f) * (e.delta > 0 ? -1.0f : 1.0f);
      if (e.horizontal) return scroll_by({amount, 0.0f}, m) || m.horizontal;
      return scroll_by({0.0f, amount}, m) || m.vertical;
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
  const auto& button_color() const noexcept { ywlib_control_get(button_color); }
  const auto& pressed_color() const noexcept { ywlib_control_get(pressed_color); }

  //-- setter --//

  auto& scroll_offset(this auto& self, float2 v) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->scroll_offset = v;
    if (auto res = sp->get_metrics()) sp->clamp_scroll(*res);
    else res.error().go_off();
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
  auto& button_color(this auto& self, const color& c) noexcept { ywlib_control_set(button_color, c, dirty); }
  auto& pressed_color(this auto& self, const color& c) noexcept { ywlib_control_set(pressed_color, c, dirty); }
};
} // namespace yw::ui
