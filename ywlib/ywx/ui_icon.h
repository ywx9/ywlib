// #pragma once
// #include "ywx/icon.h"
// #include "ywx/ui_control.h"

// namespace yw::ui {

// class icon : public plain {
// public:
//   class slot : public plain::slot {
//   public:
//     float4 padding = float4::fill(5.0f);
//     yw::icon icon;
//     color fill_color = colors::transparent;
//     color stroke_color = colors::black;
//     float stroke_width = 1.0f;

//     virtual void update_size() noexcept override {
//       min_size = vapply_r<float2>(yw::max, min_size, float2());
//       const auto inner = icon.size() + padding.xy() + padding.zw();
//       size = vapply_r<float2>(yw::max, min_size, inner, size * constrained);
//     }

//     virtual void draw() const override {
//       if (!visible) return;
//       draw_background(pos, size, background);
//       const auto isz = icon.size() + padding.xy() + padding.zw();
//       brush.color(fill_color);
//       draw_icon(pos + padding.xy() + (size - isz) * 0.5f, icon);
//       brush.color(stroke_color);
//       stroke_icon(pos + padding.xy() + (size - isz) * 0.5f, icon, stroke_width);
//     }
//   };

//   using control::operator bool;
//   icon() noexcept = default;
//   icon(derived_from<unknown> auto& Layout) {
//     if (auto res = create_control<icon>(Layout)) _id = *res;
//   }

//   const auto& padding() const { return unsafe_get(&slot::padding); }
//   const auto& object() const { return unsafe_get(&slot::icon); }
//   const auto& fill_color() const { return unsafe_get(&slot::fill_color); }
//   const auto& stroke_color() const { return unsafe_get(&slot::stroke_color); }
//   const auto& stroke_width() const { return unsafe_get(&slot::stroke_width); }

//   auto& object() {
//     if (const auto csp = system::slot_address<slot>(_id)) {
//       csp->make_messy(); // アイコンのサイズ等が変更される可能性があるため
//       return *csp;
//     } else throw std::logic_error("Invalid member access");
//   }

//   void padding(const float4& p) { safe_set_size(&slot::padding, p); }
//   void object(icon Icon) { safe_set(&slot::icon, std::move(Icon)); }
//   void fill_color(const color& c) { safe_set(&slot::fill_color, c); }
//   void stroke_color(const color& c) { safe_set(&slot::stroke_color, c); }
//   void stroke_width(float w) { safe_set(&slot::stroke_width, w); }
// };
// }
