#pragma once
#include "ywx/bitmap.h"
#include "ywx/ui_control.h"
#include "ywx/window.h"
#include "ywx/uip_base.h"

namespace yw::ui {

namespace part {
}

class frame : public control_new {
  frame() noexcept = default;
public:
  struct slot : public control_new::slot {
    part::base base{};
    slotid child_id{};

    virtual bool attach(slotid ChildId) override {
      if (child_id) return false;
      child_id = ChildId;
      return true;
    }

    virtual void detach(slotid ChildId) override {
      if (child_id == ChildId) child_id = {};
    }

    virtual slotid hittest(float2 Pt) const noexcept override {
      if (const auto hit = control_new::slot::hittest(Pt); !hit) return {};
      else if (const auto csp = system::slot_address<control_new>(child_id)) {
        const auto child_hit = csp->hittest(Pt);
        return child_hit ? child_hit : hit;
      } else return hit;
    }

    virtual float2 calculate_min_size() const override {
      float2 child_min_size;
      if (const auto csp = system::slot_address<control_new>(child_id)) child_min_size = csp->calculate_min_size();
      return vapply_r<float2>(yw::max, child_min_size, min_size, size * constrained);
    }

    virtual void update_geometry() override {
      if (geometry) geometry->Release();
      ID2D1RoundedRectangleGeometry* rect_geo = nullptr;
      D2D1_ROUNDED_RECT rect{D2D1_RECT_F(pos.x, pos.y, pos.x + size.x, pos.y + size.y), base.radius.x, base.radius.y};
      if (const auto hr = d2d.factory()->CreateRoundedRectangleGeometry(rect, &rect_geo); FAILED(hr))
        fatal_error(error(errors::operation_failed, "Failed to create geometry", int32_t(hr)));
      geometry.get() = rect_geo;
    }

    virtual void update_layout(float2 Pos, float2 Area) override {
      pos = Pos + margin.xy();
      const auto size_ = Area - margin.xy() - margin.zw();
      size = (float2(1.0f, 1.0f) - constrained) * size_ + constrained * size;
      align(size_ - size);
      update_geometry();
      if (const auto csp = system::slot_address<control_new>(child_id)) csp->update_layout(pos, size);
    }

    virtual void draw() const override {
      /// \note マスク適用、背景塗潰し、背景画像(オプション)、子コントロール、枠線の順で描画する
      if (!drawing::d2d_drawing()) fatal_error(errors::invalid_operation, "invalid drawing state");
      if (!visible) return;
      const D2D1_LAYER_PARAMETERS layer_params = D2D1::LayerParameters(
        D2D1::InfiniteRect(), geometry.get(), D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
      d2d.context()->PushLayer(&layer_params, nullptr);
      brush.color(base.border_color);
      fill_geometry(geometry.get());
      if (base.background_image) draw_bitmap(pos, size, base.background_image);
      if (const auto csp = system::slot_address<control_new>(child_id)) csp->draw();
      d2d.context()->PopLayer();
      brush.color(base.border_color);
      draw_geometry(geometry.get(), base.border_width);
    }

    virtual void draw_focus_ring() const override {
      /// \note ui::focus自体はunfocusableなので不要だが、派生先で使用できるようにしておく
      if (!drawing::d2d_drawing()) fatal_error(errors::invalid_operation, "invalid drawing state");
      if (!visible) return;
      if (const auto wsp = system::slot_address<window>(window_id)) {
        brush.color(wsp->focus_ring.color);
        draw_geometry(geometry.get(), wsp->focus_ring.width);
      } else fatal_error(errors::unreachable, "frame::draw_focus_ring: window slot not found");
    }

    virtual void char_event(wchar_t Char) override {
      if (const auto csp = system::slot_address<control_new>(child_id)) csp->char_event(Char);
    }

    virtual void click_event(event::button Event) override {
      if (const auto csp = system::slot_address<control_new>(child_id)) csp->click_event(Event);
    }

    virtual void button_event(event::button Event) override {
      if (const auto csp = system::slot_address<control_new>(child_id)) csp->button_event(Event);
    }

    virtual void drag_event(event::drag Event) override {
      if (const auto csp = system::slot_address<control_new>(child_id)) csp->drag_event(Event);
    }

    virtual void focus_event(bool Focused) override {
      if (const auto csp = system::slot_address<control_new>(child_id)) csp->focus_event(Focused);
    }

    virtual void hover_event(event::hover Event) override {
      if (const auto csp = system::slot_address<control_new>(child_id)) csp->hover_event(Event);
    }

    virtual bool key_event(event::key Event) override {
      if (const auto csp = system::slot_address<control_new>(child_id)) return csp->key_event(Event);
      else return false;
    }

    virtual void move_event(event::move Event) override {
      if (const auto csp = system::slot_address<control_new>(child_id)) csp->move_event(Event);
    }

    virtual void wheel_event(event::wheel Event) override {
      if (const auto csp = system::slot_address<control_new>(child_id)) csp->wheel_event(Event);
    }
  };

  using control_new::operator bool;

  const auto& base() const noexcept { return unsafe_get(&slot::base).handle(); }
  auto& base() noexcept { return unsafe_get(&slot::base).handle(); }
};
} // namespace yw::ui
