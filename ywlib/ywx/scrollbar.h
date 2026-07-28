// #pragma once
// #include <ywx/control.h>
// #include <ywx/drawing.h>

// namespace yw {

// template<ui::orientation Orientation> class scrollbar {
//   static constexpr bool vertical = Orientation == ui::vertical;

// public:
//   enum class part : unsigned char {
//     none,
//     decrease_button,
//     decrease_track,
//     thumb,
//     increase_track,
//     increase_button,
//   };

//   color arrow_color = colors::black;
//   color button_color = colors::transparent;
//   color thumb_color = colors::black;
//   color track_color = colors::transparent;
//   float2 pos{};
//   float2 size{};
//   float content_length = 0.0f;
//   float value = 0.0f;
//   float line_step = common_size_value;
//   float wheel_step = common_size_value * 3.0f;
//   float arrow_thickness = 1.0f;

//   float page_size() const noexcept { return yw::max(0.0f, size[vertical]); }
//   float bar_width() const noexcept { return yw::max(0.0f, size[!vertical]); }
//   float maximum() const noexcept { return yw::max(0.0f, content_length - page_size()); }
//   float clamp_value(float Value) const noexcept { return yw::clamp(Value, 0.0f, maximum()); }
//   void clamp_value() noexcept { value = clamp_value(value); }

//   bool contains(float2 Pt) const noexcept {
//     return pos.x <= Pt.x && Pt.x <= pos.x + size.x && pos.y <= Pt.y && Pt.y <= pos.y + size.y;
//   }

//   std::expected<void, error> draw_decrease_button(const color* Color = nullptr) const {
//     return draw_button(false, Color);
//   }

//   std::expected<void, error> draw_increase_button(const color* Color = nullptr) const {
//     return draw_button(true, Color);
//   }

//   std::expected<void, error> draw_decrease_track(const color* Color = nullptr) const {
//     brush::color(Color ? *Color : track_color);
//     if (auto res = fill_rectangle(decrease_track_rect()); !res) return res.error().relay();
//     return {};
//   }

//   std::expected<void, error> draw_increase_track(const color* Color = nullptr) const {
//     brush::color(Color ? *Color : track_color);
//     if (auto res = fill_rectangle(increase_track_rect()); !res) return res.error().relay();
//     return {};
//   }

//   std::expected<void, error> draw_thumb(const color* Color = nullptr) const {
//     brush::color(Color ? *Color : thumb_color);
//     if (auto res = fill_rectangle(thumb_rect()); !res) return res.error().relay();
//     return {};
//   }

//   part hittest(float2 Pt) const noexcept {
//     if (!contains(Pt)) return part::none;
//     if (contains(decrease_button_rect(), Pt)) return part::decrease_button;
//     if (contains(increase_button_rect(), Pt)) return part::increase_button;
//     const auto thumb = thumb_rect();
//     if (contains(thumb, Pt)) return part::thumb;
//     return Pt[vertical] < thumb[vertical] ? part::decrease_track : part::increase_track;
//   }

//   bool scroll_by(float Delta) noexcept { return scroll_to(value + Delta); }

//   bool scroll_to(float Value) noexcept {
//     const auto next = clamp_value(Value);
//     if (value == next) return false;
//     value = next;
//     return true;
//   }

//   bool scroll_part(part Part) noexcept {
//     if (Part == part::decrease_button) return scroll_by(-line_step);
//     if (Part == part::increase_button) return scroll_by(line_step);
//     if (Part == part::decrease_track) return scroll_by(-page_size());
//     if (Part == part::increase_track) return scroll_by(page_size());
//     return false;
//   }

//   float4 decrease_button_rect() const noexcept { return button_rect(false); }
//   float4 increase_button_rect() const noexcept { return button_rect(true); }

//   float4 decrease_track_rect() const noexcept {
//     const auto track = track_rect(), thumb = thumb_rect();
//     if constexpr (vertical) return float4(track.x, track.y, track.z, thumb.y);
//     else return float4(track.x, track.y, thumb.x, track.w);
//   }

//   float4 increase_track_rect() const noexcept {
//     const auto track = track_rect(), thumb = thumb_rect();
//     if constexpr (vertical) return float4(track.x, thumb.w, track.z, track.w);
//     else return float4(thumb.z, track.y, track.z, track.w);
//   }

//   float4 thumb_rect() const noexcept {
//     const auto track = track_rect();
//     auto begin = track[vertical];
//     if (const auto movable = movable_thumb_length(); movable > 0.0f) begin += movable * ratio();
//     if constexpr (vertical) return float4(track.x, begin, track.z, begin + thumb_length());
//     else return float4(begin, track.y, begin + thumb_length(), track.w);
//   }

//   float4 track_rect() const noexcept {
//     if constexpr (vertical) return float4(pos.x, pos.y + bar_width(), pos.x + size.x, pos.y + size.y - bar_width());
//     else return float4(pos.x + bar_width(), pos.y, pos.x + size.x - bar_width(), pos.y + size.y);
//   }

//   float thumb_length() const noexcept {
//     const auto track = track_length();
//     if (track <= 0.0f) return 0.0f;
//     if (content_length <= page_size()) return track;
//     return yw::clamp(page_size() * track / content_length, yw::min(bar_width(), track), track);
//   }

//   float movable_thumb_length() const noexcept { return yw::max(0.0f, track_length() - thumb_length()); }

//   float ratio() const noexcept {
//     if (const auto max = maximum(); max > 0.0f) return yw::clamp(value / max, 0.0f, 1.0f);
//     return 0.0f;
//   }

//   float track_length() const noexcept { return yw::max(0.0f, page_size() - bar_width() * 2.0f); }

// private:
//   static bool contains(float4 Rect, float2 Pt) noexcept {
//     return Rect.x <= Pt.x && Pt.x <= Rect.z && Rect.y <= Pt.y && Pt.y <= Rect.w;
//   }

//   float4 button_rect(bool Increase) const noexcept {
//     if constexpr (vertical) {
//       const auto y = Increase ? pos.y + size.y - bar_width() : pos.y;
//       return float4(pos.x, y, pos.x + size.x, y + bar_width());
//     } else {
//       const auto x = Increase ? pos.x + size.x - bar_width() : pos.x;
//       return float4(x, pos.y, x + bar_width(), pos.y + size.y);
//     }
//   }

//   std::expected<void, error> draw_arrow(bool Increase) const {
//     const auto rect = button_rect(Increase);
//     const auto c = rect.xy() + (rect.zw() - rect.xy()) * 0.5f;
//     brush::color(arrow_color);
//     if constexpr (vertical) {
//       const auto dx = size.x * 0.26f;
//       const auto dy = bar_width() * 0.15f;
//       const auto tip = c.template add<1>(bar_width() * (Increase ? 0.3f : -0.3f));
//       const auto tail_y = Increase ? c.y - dy : c.y + dy;
//       if (auto res = stroke_line(tip, float2(c.x - dx, tail_y), arrow_thickness); !res) return res.error().relay();
//       if (auto res = stroke_line(tip, float2(c.x + dx, tail_y), arrow_thickness); !res) return res.error().relay();
//     } else {
//       const auto dx = bar_width() * 0.15f;
//       const auto dy = size.y * 0.26f;
//       const auto tip = c.template add<0>(bar_width() * (Increase ? 0.3f : -0.3f));
//       const auto tail_x = Increase ? c.x - dx : c.x + dx;
//       if (auto res = stroke_line(tip, float2(tail_x, c.y - dy), arrow_thickness); !res) return res.error().relay();
//       if (auto res = stroke_line(tip, float2(tail_x, c.y + dy), arrow_thickness); !res) return res.error().relay();
//     }
//     return {};
//   }

//   std::expected<void, error> draw_button(bool Increase, const color* Color) const {
//     brush::color(Color ? *Color : button_color);
//     if (auto res = fill_rectangle(button_rect(Increase)); !res) return res.error().relay();
//     if (!Color)
//       if (auto res = draw_arrow(Increase); !res) return res.error().relay();
//     return {};
//   }
// };
// } // namespace yw
