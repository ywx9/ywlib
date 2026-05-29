// #pragma once
// #include "ywx/ui_control.h"
// #include "ywx/uip_text.h"
// #include <algorithm>
// #include <cmath>
// #include <iomanip>
// #include <sstream>

// namespace yw::ui {

// namespace parts {

// struct progressbar : public base {
//   color track_color = colors::darkgray;
//   color thumb_color = colors::gray;
//   color border_color = colors::black;
//   float4 padding = float4::fill(arbitrary_value);
//   float radius = arbitrary_value;
//   float border_width = arbitrary_value;
//   double max_value = 1.0;                          // always positive value
//   double value = 0.0;                              // 0.0 - max_value
//   ui::alignment orientation = ui::alignment::left; // left, right, top, bottom is valid. left means right-to-left

//   /// \note parts::progressbar does not have size

//   void normalize_value() noexcept { value = std::clamp(value, 0.0, max_value); }
//   double ratio() const noexcept { return value / max_value; }
//   float2 calculate_minimum_size() const noexcept { return float2::fill(arbitrary_value) + padding.xy() + padding.zw(); }

//   std::expected<void, error_trace> draw(float2 Pos, float2 Size) {
//     normalize_value();
//     float2 thumb_origin = Pos, thumb_size = Size;
//     switch (orientation) {
//     case ui::alignment::left: thumb_origin.x = Pos.x + Size.x * float(1.0 - ratio()); // fall-through
//     case ui::alignment::right: thumb_size.x = Size.x * float(ratio()); break;
//     case ui::alignment::top: thumb_origin.y = Pos.y + Size.y * float(1.0 - ratio()); // fall-through
//     case ui::alignment::bottom: thumb_size.y = Size.y * float(ratio()); break;
//     default: return unexpected_error(errors::unreachable);
//     }
//     brush.color(track_color);
//     fill_round_rectangle(Pos, Size, {radius, radius});
//     brush.color(thumb_color);
//     fill_round_rectangle(thumb_origin, thumb_size, {radius, radius});
//     brush.color(border_color);
//     draw_round_rectangle(Pos, Size, {radius, radius}, border_width);
//     return {};
//   }

//   class accessor : public base::accessor<progressbar> {
//     using base::accessor<progressbar>::part;

//   public:
//     const auto& track_color() const noexcept { return part.track_color; }
//     auto& track_color(yw::color Color) noexcept {
//       part.track_color = Color;
//       part.view_changed = true;
//       return *this;
//     }
//     const auto& thumb_color() const noexcept { return part.thumb_color; }
//     auto& thumb_color(yw::color Color) noexcept {
//       part.thumb_color = Color;
//       part.view_changed = true;
//       return *this;
//     }
//     const auto& border_color() const noexcept { return part.border_color; }
//     auto& border_color(yw::color Color) noexcept {
//       part.border_color = Color;
//       part.view_changed = true;
//       return *this;
//     }
//     const auto& padding() const noexcept { return part.padding; }
//     auto& padding(float4 Padding) noexcept {
//       part.padding = Padding;
//       part.layout_changed = true;
//       return *this;
//     }
//     const auto& radius() const noexcept { return part.radius; }
//     auto& radius(float Radius) noexcept {
//       part.radius = Radius;
//       part.view_changed = true;
//       return *this;
//     }
//     const auto& border_width() const noexcept { return part.border_width; }
//     auto& border_width(float BorderWidth) noexcept {
//       part.border_width = BorderWidth;
//       part.view_changed = true;
//       return *this;
//     }
//     const auto& orientation() const noexcept { return part.orientation; }
//     auto& orientation(ui::alignment Orientation) noexcept {
//       switch (Orientation) {
//       case ui::alignment::left:
//       case ui::alignment::right:
//       case ui::alignment::top:
//       case ui::alignment::bottom:
//         part.orientation = Orientation;
//         part.view_changed = true;
//         break;
//       default: print.err(errors::invalid_argument);
//       }
//       return *this;
//     }
//     const auto& max_value() const noexcept { return part.max_value; }
//     auto& max_value(double MaxValue) noexcept {
//       part.max_value = MaxValue;
//       part.value = std::clamp(part.value, 0.0, MaxValue);
//       part.view_changed = true;
//       return *this;
//     }
//     const auto& value() const noexcept { return part.value; }
//     auto& value(double Value) noexcept {
//       part.value = std::clamp(Value, 0.0, part.max_value);
//       part.view_changed = true;
//       return *this;
//     }
//     auto ratio() const noexcept { return part.ratio(); }
//   };

//   accessor access() & noexcept { return accessor{*this}; }
// };
// } // namespace parts

// class progressbar : public label {
// public:
//   enum class text_style {
//     none,
//     value,
//     value_over_max,
//     ratio,
//     percentage,
//   };
//   struct slot : public label::slot {
//     // static constexpr double epsilon = 1e-9;
//     parts::progressbar progressbar;
//     text_style style = text_style::none;

//     function<void, double> on_change;

//     //-- overrides --//

//     virtual std::expected<void, error_trace> draw() override {
//       if (!visible) return {};
//       if (auto res = background.draw(core); !res) return unexpected_error(res.error());
//       const auto pb_origin = core.pos + progressbar.padding.xy();
//       const auto pb_area = core.size - progressbar.padding.xy() - progressbar.padding.zw();
//       if (auto res = progressbar.draw(pb_origin, pb_area); !res) return unexpected_error(res.error());
//       if (style != text_style::none) {
//         const auto tx_origin = core.pos + text.padding.xy();
//         const auto tx_area = core.size - text.padding.xy() - text.padding.zw();
//         if (auto res = text.draw(tx_origin, tx_area); !res) return unexpected_error(res.error());
//       }
//       if (auto res = border.draw(core); !res) return unexpected_error(res.error());
//       return {};
//     }

//     virtual float2 calculate_minimum_size() const override {
//       const float2 pb_inner = progressbar.calculate_minimum_size();
//       const float2 tx_inner = style != text_style::none ? text.calculate_minimum_size() : float2();
//       return vapply_r<float2>(yw::max, core.min_size, core.required_size * core.constrained, pb_inner, tx_inner);
//     }
//   };

//   using control::operator bool;
//   progressbar() noexcept = default;

//   static std::expected<progressbar, error_trace> add(derived_from<unknown> auto& Layout) {
//     progressbar pb;
//     if (auto res = create_control<progressbar>(Layout)) pb._id = *res;
//     else return unexpected_error(res.error());
//     if (const auto csp = system::slot_address<progressbar>(pb._id)) {
//       csp->background.control_id = pb._id;
//       csp->border.control_id = pb._id;
//       csp->text.control_id = pb._id;
//       csp->text.alignment = text_alignment::center;
//       csp->text.block_alignment = alignment::center;
//       csp->progressbar.control_id = pb._id;
//       if (auto res = csp->update_text_label(); !res) return unexpected_error(res.error());
//     } else return unexpected_error(errors::ui_invalid_slotid);
//     return pb;
//   }

//   template<typename Self> decltype(auto) bar(this Self& self) {
//     const auto csp = system::slot_address<progressbar>(self._id);
//     if (!csp) fatal_error(errors::ui_invalid_slotid);
//     if constexpr (!is_const<Self>) return csp->progressbar.access();
//     else return std::as_const(csp->progressbar.access());
//   }

//   const auto& style() const { return unsafe_get(&progressbar::slot::style); }
//   void style(text_style Style) {
//     unsafe_set(&progressbar::slot::style, Style);
//   }

//   const auto& on_change() const { return unsafe_get(&progressbar::slot::on_change); }
//   void on_change(function<void, double> f) { unsafe_set(&progressbar::slot::on_change, std::move(f)); }
// };
// } // namespace yw::ui
