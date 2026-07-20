// #pragma once
// #include <ywx/ui_frame.h>

// namespace yw::ui {

// class scrollbar : public frame {
// public:
//   struct slot : frame::slot {
//     enum class part : unsigned char {
//       none,
//       horizontal_decrease = 0x00,
//       horizontal_increase = 0x10,
//       horizontal_track_decrease = 0x20,
//       horizontal_track_increase = 0x30,
//       horizontal_thumb = 0x40,
//       vertical_decrease = 0x01,
//       vertical_increase = 0x11,
//       vertical_track_decrease = 0x21,
//       vertical_track_increase = 0x31,
//       vertical_thumb = 0x41,
//     };

//     color track_color{};
//     color thumb_color{};
//     color button_color{};
//     color pressed_color{};
//     float2 viewport_size{};
//     float2 scroll_offset{};
//     float bar_width = 16.0f;
//     float line_scroll = 16.0f;
//     float page_scroll = 0.0f;
//     float wheel_scroll = 48.0f;
//     vector2<bool> bar_visible{};
//     part pressed_part = part::none;

//     //-- helper functions --//

//     static bool _is_thumb_part(part Part) noexcept { return (static_cast<unsigned char>(Part) & 0x40) != 0; }

//     static vector2<bool> _horizontal_or_vertical(part Part) noexcept {
//       return {(static_cast<unsigned char>(Part) & 0x01) == 0, (static_cast<unsigned char>(Part) & 0x01) == 1};
//     }

//     float2 _max_scroll() const noexcept {
//       return vapply_r<float2>(yw::max, float2{}, get_content_size() - viewport_size);
//     }

//     void _clamp_scroll() noexcept {
//       scroll_offset = vapply_r<float2>(yw::clamp, scroll_offset, float2{}, _max_scroll());
//     }

//     part _part_hittest(float2 Pt) const noexcept {
//       const auto pt = Pt - pos;
//       const auto inner = size - bar_visible * bar_width;
//       bool v{};
//       if (inner.x <= pt.x && pt.x <= size.x && bar_visible.y) v = true;
//       else if (inner.y <= pt.y && pt.y <= size.y && bar_visible.x) v = false;
//       else return part::none;
//       const auto p = pt[v];
//       if (p <= bar_width) return static_cast<part>(0x00 + v); // decrease button
//       const auto track_length = inner[v] - bar_width * (2.0f + bar_visible[!v]);
//       const auto track_end = bar_width + track_length;
//       if (p <= track_end) {
//         const auto content_size = get_content_size()[v];
//         const auto viewport_size = this->viewport_size[v];
//         const auto thumb_length = viewport_size * track_length / content_size;
//         const auto thumb_start = scroll_offset[v] * (track_length - thumb_length) / (content_size - viewport_size);
//         const auto thumb_end = thumb_start + thumb_length;
//         if (p <= bar_width + thumb_start) return static_cast<part>(0x20 + v);    // track decrease
//         if (p <= bar_width + thumb_end) return static_cast<part>(0x40 + v);      // thumb
//         return static_cast<part>(0x30 + v);                                      // track increase
//       } else if (p <= track_end + bar_width) return static_cast<part>(0x10 + v); // increase button
//       return part::none;
//     }

//     //-- override functions --//

//     // static bool contains(float4 Rect, float2 Pt) noexcept {
//     //   return Pt.x >= Rect.x && Pt.y >= Rect.y && Pt.x <= Rect.z && Pt.y <= Rect.w;
//     // }

//     // static float2 rect_pos(float4 Rect) noexcept { return {Rect.x, Rect.y}; }
//     // static float2 rect_size(float4 Rect) noexcept { return {Rect.z - Rect.x, Rect.w - Rect.y}; }

//     // static bool vertical_part(part Part) noexcept {
//     //   return Part == part::vertical_decrease || Part == part::vertical_increase ||
//     //          Part == part::vertical_track_decrease || Part == part::vertical_track_increase ||
//     //          Part == part::vertical_thumb;
//     // }

//     // static bool horizontal_part(part Part) noexcept {
//     //   return Part == part::horizontal_decrease || Part == part::horizontal_increase ||
//     //          Part == part::horizontal_track_decrease || Part == part::horizontal_track_increase ||
//     //          Part == part::horizontal_thumb;
//     // }

//     virtual float2 get_content_size() const { return float2{}; }

//     // vector2<bool> _bar_visible() const {
//     //   const auto content = get_content_size();
//     //   const auto viewport = size - padding.xy() - padding.zw();
//     //   return {content.x > viewport.x, content.y > viewport.y};
//     // }

//     // float2 _viewport_size() const noexcept {
//     //   const auto content = get_content_size();
//     //   const auto viewport = size - padding.xy() - padding.zw();
//     //   const auto bar = bar_width * float2{content.x > viewport.x, content.y > viewport.y};
//     //   return viewport - bar;
//     // }

//     // std::expected<metrics, error> get_metrics() const {
//     //   metrics m;
//     //   if (auto res = get_content_size()) m.content_size = *res;
//     //   else return res.error().relay();
//     //   m.inner_pos = pos;
//     //   m.inner_size = size;
//     //   m.inner_size.x = yw::max(0.0f, m.inner_size.x);
//     //   m.inner_size.y = yw::max(0.0f, m.inner_size.y);
//     //   m.viewport_pos = pos + padding.xy();

//     //   for (int i = 0; i < 2; ++i) {
//     //     const auto viewport_end = pos + size - padding.zw() -
//     //                               float2(m.vertical ? bar_width : 0.0f, m.horizontal ? bar_width : 0.0f);
//     //     m.viewport_size = viewport_end - m.viewport_pos;
//     //     m.viewport_size.x = yw::max(0.0f, m.viewport_size.x);
//     //     m.viewport_size.y = yw::max(0.0f, m.viewport_size.y);
//     //     m.horizontal = m.content_size.x > m.viewport_size.x;
//     //     m.vertical = m.content_size.y > m.viewport_size.y;
//     //   }
//     //   const auto viewport_end = pos + size - padding.zw() -
//     //                             float2(m.vertical ? bar_width : 0.0f, m.horizontal ? bar_width : 0.0f);
//     //   m.viewport_size = viewport_end - m.viewport_pos;
//     //   m.viewport_size.x = yw::max(0.0f, m.viewport_size.x);
//     //   m.viewport_size.y = yw::max(0.0f, m.viewport_size.y);
//     //   return m;
//     // }

//     // float4 vertical_rect(const metrics& m) const noexcept {
//     //   const float left = pos.x + size.x - bar_width;
//     //   const float bottom = pos.y + size.y - (m.horizontal ? bar_width : 0.0f);
//     //   return {left, pos.y, pos.x + size.x, bottom};
//     // }

//     // float4 horizontal_rect(const metrics& m) const noexcept {
//     //   const float top = pos.y + size.y - bar_width;
//     //   const float right = pos.x + size.x - (m.vertical ? bar_width : 0.0f);
//     //   return {pos.x, top, right, pos.y + size.y};
//     // }

//     // float4 corner_rect(const metrics& m) const noexcept {
//     //   return {pos.x + size.x - bar_width, pos.y + size.y - bar_width, pos.x + size.x, pos.y + size.y};
//     // }

//     // float4 decrease_button_rect(float4 Bar) const noexcept {
//     //   return {Bar.x, Bar.y, Bar.x + bar_width, Bar.y + bar_width};
//     // }

//     // float4 increase_button_rect(float4 Bar, bool Vertical) const noexcept {
//     //   if (Vertical) return {Bar.x, Bar.w - bar_width, Bar.z, Bar.w};
//     //   return {Bar.z - bar_width, Bar.y, Bar.z, Bar.w};
//     // }

//     // float track_length(float4 Bar, bool Vertical) const noexcept {
//     //   return yw::max(0.0f, (Vertical ? Bar.w - Bar.y : Bar.z - Bar.x) - bar_width * 2.0f);
//     // }

//     // float4 thumb_rect(const metrics& m, bool Vertical) const noexcept {
//     //   const auto bar = Vertical ? vertical_rect(m) : horizontal_rect(m);
//     //   const float track = track_length(bar, Vertical);
//     //   if (track <= 0.0f) return {};
//     //   const float viewport = Vertical ? m.viewport_size.y : m.viewport_size.x;
//     //   const float content = Vertical ? m.content_size.y : m.content_size.x;
//     //   const float max = yw::max(0.0f, content - viewport);
//     //   const float thumb = content <= 0.0f ? track : track * yw::clamp(viewport / content, 0.0f, 1.0f);
//     //   const float movable = yw::max(0.0f, track - thumb);
//     //   const float value = Vertical ? scroll_offset.y : scroll_offset.x;
//     //   const float offset = max <= 0.0f ? 0.0f : movable * value / max;
//     //   if (Vertical) {
//     //     const float y = bar.y + bar_width + offset;
//     //     return {bar.x, y, bar.z, y + thumb};
//     //   }
//     //   const float x = bar.x + bar_width + offset;
//     //   return {x, bar.y, x + thumb, bar.w};
//     // }

//     // part hit_test(float2 Pt, const metrics& m) const noexcept {
//     //   if (m.vertical) {
//     //     const auto bar = vertical_rect(m);
//     //     if (contains(bar, Pt)) {
//     //       if (contains(decrease_button_rect(bar), Pt)) return part::vertical_decrease;
//     //       if (contains(increase_button_rect(bar, true), Pt)) return part::vertical_increase;
//     //       const auto thumb = thumb_rect(m, true);
//     //       if (contains(thumb, Pt)) return part::vertical_thumb;
//     //       return Pt.y < thumb.y ? part::vertical_track_decrease : part::vertical_track_increase;
//     //     }
//     //   }
//     //   if (m.horizontal) {
//     //     const auto bar = horizontal_rect(m);
//     //     if (contains(bar, Pt)) {
//     //       if (contains(decrease_button_rect(bar), Pt)) return part::horizontal_decrease;
//     //       if (contains(increase_button_rect(bar, false), Pt)) return part::horizontal_increase;
//     //       const auto thumb = thumb_rect(m, false);
//     //       if (contains(thumb, Pt)) return part::horizontal_thumb;
//     //       return Pt.x < thumb.x ? part::horizontal_track_decrease : part::horizontal_track_increase;
//     //     }
//     //   }
//     //   return part::none;
//     // }

//     float page_amount(bool Vertical) const noexcept {
//       if (page_scroll > 0.0f) return page_scroll;
//       return yw::max(line_scroll, viewport_size[Vertical] - bar_width);
//     }

//     bool scroll_by(float2 Delta) noexcept {
//       const auto old = scroll_offset;
//       scroll_offset = vapply_r<float2>(yw::clamp, scroll_offset + Delta, float2(), _max_scroll());
//       if (scroll_offset == old) return false;
//       make_dirty();
//       return true;
//     }

//     bool scroll_part(part Part) noexcept {
//       switch (Part) {
//       case part::vertical_decrease: return scroll_by({0.0f, -line_scroll});
//       case part::vertical_increase: return scroll_by({0.0f, line_scroll});
//       case part::vertical_track_decrease: return scroll_by({0.0f, -page_amount(true)});
//       case part::vertical_track_increase: return scroll_by({0.0f, page_amount(true)});
//       case part::horizontal_decrease: return scroll_by({-line_scroll, 0.0f});
//       case part::horizontal_increase: return scroll_by({line_scroll, 0.0f});
//       case part::horizontal_track_decrease: return scroll_by({-page_amount(false), 0.0f});
//       case part::horizontal_track_increase: return scroll_by({page_amount(false), 0.0f});
//       default: return false;
//       }
//     }

//     // std::expected<void, error> fill_rect(float4 Rect, const color& Color) const {
//     //   if (Color.a <= 0.0f || Rect.z <= Rect.x || Rect.w <= Rect.y) return {};
//     //   brush::color(Color);
//     //   if (auto res = fill_rectangle(rect_pos(Rect), rect_size(Rect)); !res) return res.error().relay();
//     //   return {};
//     // }

//     std::expected<void, error> draw_arrow(float2 Pos, bool Vertical, bool Increase) const {
//       const auto a = bar_width * 0.15f;
//       const auto b = bar_width * 0.26f;
//       const auto c = Pos + float2::fill(bar_width * 0.5f);
//       brush::color(text_color);
//       if (Vertical && Increase) {
//         const auto tip = c + float2(0.0f, bar_width * 0.3f);
//         if (auto res = draw_line(tip, c + float2(-b, -a), 1.0f); !res) return res.error().relay();
//         if (auto res = draw_line(tip, c + float2(b, -a), 1.0f); !res) return res.error().relay();
//       } else if (Vertical) {
//         const auto tip = c + float2(0.0f, -bar_width * 0.3f);
//         if (auto res = draw_line(tip, c + float2(-b, a), 1.0f); !res) return res.error().relay();
//         if (auto res = draw_line(tip, c + float2(b, a), 1.0f); !res) return res.error().relay();
//       } else if (Increase) {
//         const auto tip = c + float2(bar_width * 0.3f, 0.0f);
//         if (auto res = draw_line(tip, c + float2(-a, -b), 1.0f); !res) return res.error().relay();
//         if (auto res = draw_line(tip, c + float2(-a, b), 1.0f); !res) return res.error().relay();
//       } else {
//         const auto tip = c + float2(-bar_width * 0.3f, 0.0f);
//         if (auto res = draw_line(tip, c + float2(a, -b), 1.0f); !res) return res.error().relay();
//         if (auto res = draw_line(tip, c + float2(a, b), 1.0f); !res) return res.error().relay();
//       }
//       return {};
//     }

//     template<bool Vertical> std::expected<void, error> draw_bar() const {
//       auto origin = pos;
//       origin[!Vertical] += size[!Vertical] - bar_width;
//       auto bar_size = size;
//       bar_size[!Vertical] = bar_width;
//       brush::color(background_color);
//     }

//     std::expected<void, error> draw_bar(bool Vertical) const {
//       auto origin = pos;
//       origin[!Vertical] += size[!Vertical] - bar_width;
//       auto bar_size = size;
//       bar_size[!Vertical] = bar_width;




//       const auto bar = Vertical ? vertical_rect(m) : horizontal_rect(m);
//       const auto dec = decrease_button_rect(bar);
//       const auto inc = increase_button_rect(bar, Vertical);
//       if (auto res = fill_rect(bar, track_color); !res) return res.error().relay();
//       if (auto res = fill_rect(dec, button_color); !res) return res.error().relay();
//       if (auto res = fill_rect(inc, button_color); !res) return res.error().relay();
//       if (auto res = fill_rect(thumb_rect(m, Vertical), thumb_color); !res) return res.error().relay();
//       if (pressed_part != part::none &&
//           ((Vertical && vertical_part(pressed_part)) || (!Vertical && horizontal_part(pressed_part)))) {
//         const auto pressed_rect =
//           thumb_part(pressed_part)                                                           ? thumb_rect(m, Vertical)
//           : pressed_part == (Vertical ? part::vertical_decrease : part::horizontal_decrease) ? dec
//           : pressed_part == (Vertical ? part::vertical_increase : part::horizontal_increase) ? inc
//                                                                                              : bar;
//         if (auto res = fill_rect(pressed_rect, pressed_color); !res) return res.error().relay();
//       }
//       if (auto res = draw_arrow(dec, Vertical, false); !res) return res.error().relay();
//       if (auto res = draw_arrow(inc, Vertical, true); !res) return res.error().relay();
//       return {};
//     }

//     std::expected<void, error> draw_scrollbars() {
//       metrics m;
//       if (auto res = get_metrics()) {
//         m = *res;
//         clamp_scroll(m);
//       } else return res.error().relay();
//       if (m.vertical && m.horizontal) {
//         if (auto res = fill_rect(corner_rect(m), colors.background); !res) return res.error().relay();
//       }
//       if (m.vertical)
//         if (auto res = draw_bar(m, true); !res) return res.error().relay();
//       if (m.horizontal)
//         if (auto res = draw_bar(m, false); !res) return res.error().relay();
//       return {};
//     }

//     //-- override functions --//

//     virtual std::expected<float2, error> get_necessary_size() const override {
//       const auto inner = padding.xy() + padding.zw() + float2::fill(bar_width * 3.0f + arbitrary_value);
//       return calc_necessary_size_by_policy(inner);
//     }

//     virtual std::expected<void, error> redraw() override {
//       if (geometry_dirty) {
//         geometry_dirty = false;
//         if (auto res = relocate(); !res) return res.error().relay();
//       }
//       if (!visible) return {};
//       if (auto res = draw_frame_background(); !res) return res.error().relay();
//       if (auto res = draw_scrollbars(); !res) return res.error().relay();
//       if (auto res = draw_frame_foreground(); !res) return res.error().relay();
//       return {};
//     }

//     virtual std::expected<void, error> relocate() override {
//       if (auto res = frame::slot::relocate(); !res) return res.error().relay();
//       _clamp_scroll();
//       return {};
//     }

//     virtual bool button_event(yw::button_event e) override {
//       if (!enabled || !visible || e.key != keys::lbutton) return frame::slot::button_event(e);
//       _clamp_scroll();
//       const auto hit = _part_hittest(e.pos);
//       if (e.down) {
//         if (hit == part::none) return frame::slot::button_event(e);
//         pressed_part = hit;
//         scroll_part(hit);
//         make_dirty();
//         return true;
//       } else if (pressed_part == part::none) return frame::slot::button_event(e);
//       pressed_part = part::none;
//       make_dirty();
//       return true;
//     }

//     virtual bool click_event(yw::button_event e) override {
//       if (!enabled || !visible || e.down || e.key != keys::lbutton) return frame::slot::click_event(e);
//       return _part_hittest(e.pos) != part::none;
//     }

//     virtual bool drag_event(yw::drag_event e) override {
//       if (!enabled || !visible || !_is_thumb_part(pressed_part)) return frame::slot::drag_event(e);
//       const auto track_length = viewport_size - bar_width * 2.0f * float2(bar_visible.y, bar_visible.x);
//       const auto thumb_length = get_content_size() * track_length / viewport_size;
//       const auto movable_length = track_length - thumb_length;
//       const auto max_scroll = _max_scroll();
//       const auto scroll = (e.delta * max_scroll / movable_length) * _horizontal_or_vertica=(pressed_part);
//       scroll_by(scroll);
//       01t1rn=tr1e;
//     }

//     virtual std::expected<void, error> reset_state() override {
//       if (pressed_part == part::none) return {};
//       pressed_part = part::none;
//       make_dirty();
//       return {};
//     }

//     virtual void focus_event(bool Focused) override {
//       if (!Focused && pressed_part != part::none) {
//         pressed_part = part::none;
//         make_dirty();
//       }
//       frame::slot::focus_event(Focused);
//     }

//     virtual bool key_event(yw::key_event e) override {
//       if (!enabled || !visible) return false;
//       if (!e.down) {
//         if (e.key == keys::up || e.key == keys::down || e.key == keys::left || e.key == keys::right ||
//             e.key == keys::page_up || e.key == keys::page_down || e.key == keys::home || e.key == keys::end)
//           return true;
//         return frame::slot::key_event(e);
//       }
//       if (e.key == keys::up) return scroll_by({0.0f, -line_scroll}), true;
//       if (e.key == keys::down) return scroll_by({0.0f, line_scroll}), true;
//       if (e.key == keys::left) return scroll_by({-line_scroll, 0.0f}), true;
//       if (e.key == keys::right) return scroll_by({line_scroll, 0.0f}), true;
//       if (e.key == keys::page_up) return scroll_by({0.0f, -page_amount(true)}), true;
//       if (e.key == keys::page_down) return scroll_by({0.0f, page_amount(true)}), true;
//       if (e.key == keys::home) return scroll_offset = {}, make_dirty(), true;
//       if (e.key == keys::end) return scroll_offset = _max_scroll(), make_dirty(), true;
//       return frame::slot::key_event(e);
//     }

//     virtual bool wheel_event(yw::wheel_event e) override {
//       if (!enabled || !visible || e.delta == 0) return false;
//       const float amount = wheel_scroll * (e.mods.shift ? 10.0f : 1.0f) * (e.delta > 0 ? -1.0f : 1.0f);
//       if (e.horizontal) return scroll_by({amount, 0.0f}) || bar_visible.x;
//       return scroll_by({0.0f, amount}) || bar_visible.y;
//     }
//   };

//   //-- getter --//

//   float2 scroll_offset() const noexcept { ywlib_control_get(scroll_offset); }
//   float bar_width() const noexcept { ywlib_control_get(bar_width); }
//   float line_scroll() const noexcept { ywlib_control_get(line_scroll); }
//   float page_scroll() const noexcept { ywlib_control_get(page_scroll); }
//   float wheel_scroll() const noexcept { ywlib_control_get(wheel_scroll); }
//   const auto& track_color() const noexcept { ywlib_control_get(track_color); }
//   const auto& thumb_color() const noexcept { ywlib_control_get(thumb_color); }
//   const auto& button_color() const noexcept { ywlib_control_get(button_color); }
//   const auto& pressed_color() const noexcept { ywlib_control_get(pressed_color); }

//   //-- setter --//

//   auto& scroll_offset(this auto& self, float2 v) noexcept {
//     const auto sp = get_slot(&self);
//     if (!sp) error(errors::invalid_slotid).go_off();
//     sp->scroll_offset = v;
//     if (auto res = sp->get_metrics()) sp->clamp_scroll(*res);
//     else res.error().go_off();
//     sp->make_dirty();
//     return self;
//   }

//   auto& bar_width(this auto& self, float1 v) noexcept {
//     const auto sp = get_slot(&self);
//     if (!sp) error(errors::invalid_slotid).go_off();
//     if (v.x <= 0.0f) {
//       error(errors::invalid_argument, "scrollbar bar_width must be positive").go_off();
//       return self;
//     }
//     sp->bar_width = v.x;
//     sp->make_messy();
//     return self;
//   }

//   auto& line_scroll(this auto& self, float1 v) noexcept {
//     const auto sp = get_slot(&self);
//     if (!sp) error(errors::invalid_slotid).go_off();
//     if (v.x <= 0.0f) {
//       error(errors::invalid_argument, "scrollbar line_scroll must be positive").go_off();
//       return self;
//     }
//     sp->line_scroll = v.x;
//     return self;
//   }

//   auto& page_scroll(this auto& self, float1 v) noexcept {
//     const auto sp = get_slot(&self);
//     if (!sp) error(errors::invalid_slotid).go_off();
//     if (v.x < 0.0f) {
//       error(errors::invalid_argument, "scrollbar page_scroll must be non-negative").go_off();
//       return self;
//     }
//     sp->page_scroll = v.x;
//     return self;
//   }

//   auto& wheel_scroll(this auto& self, float1 v) noexcept {
//     const auto sp = get_slot(&self);
//     if (!sp) error(errors::invalid_slotid).go_off();
//     if (v.x <= 0.0f) {
//       error(errors::invalid_argument, "scrollbar wheel_scroll must be positive").go_off();
//       return self;
//     }
//     sp->wheel_scroll = v.x;
//     return self;
//   }

//   auto& track_color(this auto& self, const color& c) noexcept { ywlib_control_set(track_color, c, dirty); }
//   auto& thumb_color(this auto& self, const color& c) noexcept { ywlib_control_set(thumb_color, c, dirty); }
//   auto& button_color(this auto& self, const color& c) noexcept { ywlib_control_set(button_color, c, dirty); }
//   auto& pressed_color(this auto& self, const color& c) noexcept { ywlib_control_set(pressed_color, c, dirty); }
// };
// } // namespace yw::ui
