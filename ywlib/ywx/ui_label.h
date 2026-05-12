#pragma once
#include "ywx/ui_control.h"
#include "ywx/uip_text.h"

namespace yw::ui {

class label : public control {
public:
  struct slot : public control::slot {
    part::background background;
    part::border border;
    part::text text;

    std::expected<void, error_trace> initialize() {
      background.owner_window_id = core.owner_window_id;
      border.owner_window_id = core.owner_window_id;
      text.owner_window_id = core.owner_window_id;
      if (auto res = text.initialize(); !res) return unexpected_error(res.error());
      return {};
    }

    virtual void draw() override {
      if (!visible) return;
      brush.color(background.color);
      fill_geometry(core.geometry.get());
      d2d.push_layer(core.geometry.get());
      if (background.image) draw_bitmap(core.pos, core.size, background.image, background.image_opacity);
      text.draw(core.pos, core.size);
      d2d.pop_layer();
      brush.color(border.color);
      draw_geometry(core.geometry.get());
    }
  };

  using control::operator bool;
  label() noexcept = default;

  static std::expected<label, error_trace> add(derived_from<unknown> auto& Layout) {
    if (auto res = create_control<label>(Layout)) {
      label lbl;
      lbl._id = *res;
      if (const auto csp = system::slot_address<label>(lbl._id)) {
        if (auto res = csp->initialize(); !res) return unexpected_error(res.error());
        make_dirty();
      } else return unexpected_error(errors::operation_failed, "missing slot");
      return lbl;
    } else return unexpected_error(res.error());
  }

  auto background() {
    const auto csp = system::slot_address<label>(_id);
    if (!csp) fatal_error(errors::invalid_operation, "Invalid slot address");
    return csp->background.handle();
  }

  const auto background() const {
    const auto csp = system::slot_address<label>(_id);
    if (!csp) fatal_error(errors::invalid_operation, "Invalid slot address");
    return csp->background.handle();
  }

  auto border() {
    const auto csp = system::slot_address<label>(_id);
    if (!csp) fatal_error(errors::invalid_operation, "Invalid slot address");
    return csp->border.handle();
  }

  const auto border() const {
    const auto csp = system::slot_address<label>(_id);
    if (!csp) fatal_error(errors::invalid_operation, "Invalid slot address");
    return csp->border.handle();
  }

  auto text() {
    const auto csp = system::slot_address<label>(_id);
    if (!csp) fatal_error(errors::invalid_operation, "Invalid slot address");
    return csp->text.handle();
  }

  const auto text() const {
    const auto csp = system::slot_address<label>(_id);
    if (!csp) fatal_error(errors::invalid_operation, "Invalid slot address");
    return csp->text.handle();
  }
};
} // namespace yw::ui

// namespace yw::ui {

// class label : public control {
// public:
//   class slot : public control::slot {
//   public:
//     float4 padding = float4::fill(4.0f);

//     yw::text text = assume(yw::text::create(L""));
//     color text_color = colors::black;
//     ui::alignment text_alignment = ui::alignment::center;

//     function<void, event::button> on_button;
//     function<void, event::wheel> on_wheel;

//     virtual float2 calculate_size() const noexcept override {
//       const auto inner = text.size() + padding.xy() + padding.zw();
//       return vapply_r<float2>(yw::max, float2(), min_size, inner, size * constrained);
//     }

//     virtual void update_size() noexcept override {
//       min_size = vapply_r<float2>(yw::max, min_size, float2());
//       const auto inner = text.size() + padding.xy() + padding.zw();
//       size = vapply_r<float2>(yw::max, min_size, inner, size * constrained);
//       update_geometry();
//     }

//     virtual void draw() const override {
//       if (!visible) return;
//       draw_background();
//       brush.color(text_color);
//       const auto tsz = text.size() + padding.xy() + padding.zw();
//       auto extra = size - tsz;
//       switch (text_alignment) {
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
//       draw_text(pos + padding.xy() + extra, text);
//     }

//     virtual void button_event(event::button Event) override {
//       if (enabled && on_button) on_button(Event);
//     }

//     virtual void wheel_event(event::wheel Event) override {
//       if (enabled && on_wheel) on_wheel(Event);
//     }
//   };

//   using control::operator bool;
//   label() noexcept = default;
//   label(derived_from<unknown> auto& Layout) {
//     if (auto res = create_control<label>(Layout)) _id = *res;
//   }

//   const auto& border_color() const { return unsafe_get(&slot::border_color); }
//   void border_color(const color& c) { safe_set(&slot::border_color, c); }

//   const auto& border_width() const { return unsafe_get(&slot::border_width); }
//   void border_width(float1 w) { safe_set(&slot::border_width, w.x); }

//   const auto& padding() const { return unsafe_get(&slot::padding); }
//   void padding(const float4& p) { safe_set_size(&slot::padding, p); }

//   const auto& text() const { return unsafe_get(&slot::text); }
//   auto& text() { return unsafe_get_size(&slot::text); }
//   template<stringable S> void text(S&& Text) { safe_set_size(&slot::text, static_cast<S&&>(Text)); }

//   const auto& text_color() const { return unsafe_get(&slot::text_color); }
//   void text_color(const color& c) { safe_set(&slot::text_color, c); }

//   const auto& text_alignment() const { return unsafe_get(&slot::text_alignment); }
//   void text_alignment(ui::alignment align) {
//     if (auto csp = system::slot_address<label>(_id)) {
//       csp->text_alignment = align;
//       switch (uint_cast(align) & 0b11) {
//       case 0b00: csp->text.text_alignment(yw::text_alignment::center); break;
//       case 0b01: csp->text.text_alignment(yw::text_alignment::left); break;
//       case 0b10: csp->text.text_alignment(yw::text_alignment::right); break;
//       }
//       csp->make_dirty();
//     }
//   }

//   const auto& on_button() const { return unsafe_get(&slot::on_button); }
//   void on_button(function<void, event::button> cb) { safe_set(&slot::on_button, std::move(cb)); }

//   const auto& on_wheel() const { return unsafe_get(&slot::on_wheel); }
//   void on_wheel(function<void, event::wheel> cb) { safe_set(&slot::on_wheel, std::move(cb)); }
// };
// } // namespace yw::ui

// #include "ywx/ui_control.h"
// #include "ywx/uip_base.h"
// #include "ywx/uip_text.h"

// namespace yw::ui {

// class label_new : public control_new {
// public:
//   struct slot : public control_new::slot {
//     part::base base;
//     part::text text;
//     float2 offset{};
//     ui::alignment alignment = ui::alignment::center; //

//     virtual std::expected<void, error_trace> draw() const {
//       /// \note base.background -> text -> base.border
//       if (!visible) return {};
//       if (!drawing::d2d_drawing()) return unexpected_error(errors::invalid_operation, "drawing not begun");
//       ID2D1Geometry* geo = geometry.get();
//       if (!geo) return unexpected_error(errors::invalid_operation, "geometry not set");
//       brush.color(base.background_color);
//       d2d.context()->FillGeometry(geo, brush.d2d_brush(), nullptr);
//       const D2D1_LAYER_PARAMETERS params =
//             D2D1::LayerParameters(D2D1::InfiniteRect(), geo, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
//       d2d.context()->PushLayer(params, nullptr);
//       if (base.background_image) draw_bitmap(pos, size, base.background_image, base.background_image_opacity);
//       float2 text_origin = pos;
//       const float coeff[] = {0.5f, 1.0f, 0.0f};
//       text_origin.x += (size.x - text.layout_size().x) * coeff[int(alignment) & 0b11] + offset.x;
//       text_origin.y += (size.y - text.layout_size().y) * coeff[(int(alignment) >> 2) & 0b11] + offset.y;
//       brush.color(text.font_color);
//       draw_text(text_origin, text.text_layout.get());
//       d2d.context()->PopLayer();
//       brush.color(base.border_color);
//       draw_geometry(geo, base.border_width);
//     }
//   };
// };
// }
