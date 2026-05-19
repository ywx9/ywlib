// #pragma once
// #include "ywx/bitmap.h"
// #include "ywx/ui_control.h"

// namespace yw::ui {

// template<bitmap_like T> class bitmap : public control {
// public:
//   class slot : public control::slot {
//   public:
//     float4 padding = float4::fill(4.0f);

//     T image{};
//     ui::alignment image_alignment = ui::alignment::center;

//     function<void, event::button> on_button;
//     function<void, event::wheel> on_wheel;

//     float2 image_size() const noexcept {
//       const auto p = static_cast<ID2D1Bitmap1*>(image);
//       if (!p) return {};
//       const auto s = p->GetSize();
//       return {s.width, s.height};
//     }

//     virtual float2 calculate_size() const noexcept override {
//       const auto inner = image_size() + padding.xy() + padding.zw();
//       return vapply_r<float2>(yw::max, float2(), min_size, inner, size * constrained);
//     }

//     virtual void update_size() noexcept override {
//       min_size = vapply_r<float2>(yw::max, min_size, float2());
//       const auto inner = image_size() + padding.xy() + padding.zw();
//       size = vapply_r<float2>(yw::max, min_size, inner, size * constrained);
//       update_geometry();
//     }

//     virtual void draw() const override {
//       if (!visible) return;
//       draw_background();

//       const auto p = static_cast<ID2D1Bitmap1*>(image);
//       if (!p) return;
//       const auto isz = image_size();
//       const auto bsz = isz + padding.xy() + padding.zw();
//       auto extra = size - bsz;
//       switch (image_alignment) {
//       case ui::alignment::center: extra *= 0.5f; break;
//       case ui::alignment::left: extra *= float2(0.0f, 0.5f); break;
//       case ui::alignment::right: extra *= float2(1.0f, 0.5f); break;
//       case ui::alignment::top: extra *= float2(0.5f, 0.0f); break;
//       case ui::alignment::bottom: extra *= float2(0.5f, 1.0f); break;
//       case ui::alignment::left_top: extra = {}; break;
//       case ui::alignment::left_bottom: extra *= float2(0.0f, 1.0f); break;
//       case ui::alignment::right_top: extra *= float2(1.0f, 0.0f); break;
//       // case ui::alignment::right_bottom: break;
//       }

//       const auto draw_pos = pos + padding.xy() + extra;
//       D2D1_RECT_F rect = D2D1::RectF(draw_pos.x, draw_pos.y, draw_pos.x + isz.x, draw_pos.y + isz.y);
//       d2d.context()->DrawBitmap(p, &rect, 1.0f);
//     }

//     virtual void button_event(event::button Event) override {
//       if (enabled && on_button) on_button(Event);
//     }

//     virtual void wheel_event(event::wheel Event) override {
//       if (enabled && on_wheel) on_wheel(Event);
//     }
//   };

//   using control::operator bool;
//   bitmap() noexcept = default;
//   bitmap(derived_from<unknown> auto& Layout) {
//     if (auto res = create_control<bitmap<T>>(Layout)) _id = *res;
//   }
//   bitmap(derived_from<unknown> auto& Layout, T&& Image) {
//     if (auto res = create_control<bitmap<T>>(Layout)) {
//       _id = *res;
//       safe_set_size(&slot::image, static_cast<T&&>(Image));
//     }
//   }

//   const auto& border_color() const { return unsafe_get(&slot::border_color); }
//   void border_color(const color& c) { safe_set(&slot::border_color, c); }

//   const auto& border_width() const { return unsafe_get(&slot::border_width); }
//   void border_width(float1 w) { safe_set(&slot::border_width, w.x); }

//   const auto& padding() const { return unsafe_get(&slot::padding); }
//   void padding(const float4& p) { safe_set_size(&slot::padding, p); }

//   const auto& image() const { return unsafe_get(&slot::image); }
//   auto& image() { return unsafe_get_size(&slot::image); }
//   void image(T&& Image) { safe_set_size(&slot::image, static_cast<T&&>(Image)); }

//   const auto& image_alignment() const { return unsafe_get(&slot::image_alignment); }
//   void image_alignment(ui::alignment align) { safe_set(&slot::image_alignment, align); }

//   const auto& on_button() const { return unsafe_get(&slot::on_button); }
//   void on_button(function<void, event::button> cb) { safe_set(&slot::on_button, std::move(cb)); }

//   const auto& on_wheel() const { return unsafe_get(&slot::on_wheel); }
//   void on_wheel(function<void, event::wheel> cb) { safe_set(&slot::on_wheel, std::move(cb)); }
// };

// template<derived_from<unknown> Ctrl, bitmap_like T> bitmap(Ctrl& layout, T&& image) -> bitmap<T>;
// } // namespace yw::ui
