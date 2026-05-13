#pragma once
#include "ywx/ui_control.h"

namespace yw::ui {

template<bool Vertical> class layout : public control {
public:
  struct slot : public control::slot {
    parts::background background{.color = colors::transparent};
    parts::border border{.color = colors::red};

    std::vector<slotid> controls{};
    float4 padding = float4::fill(arbitrary_value);

    //-- override functions --//

    virtual const char* attachable() const { return nullptr; }

    virtual void attach(slotid Child) override {
      controls.push_back(Child);
      make_messy();
    }

    virtual void detach(slotid Child) override {
      controls.erase(std::remove(controls.begin(), controls.end(), Child), controls.end());
      system::uis.erase(Child);
      make_messy();
    }

    virtual void ensure_minimum_size() override {
      float2 internal;
      for (const auto& cid : controls)
        if (const auto csp = system::slot_address<control>(cid)) {
          csp->ensure_minimum_size();
          const auto area = csp->core.area();
          get<!Vertical>(internal) = yw::max(get<!Vertical>(internal), get<!Vertical>(area));
          get<Vertical>(internal) += get<Vertical>(area);
        }
      core.size = vapply_r<float2>(yw::max, core.min_size, internal, core.required_size * core.constrained) +
                  padding.xy() + padding.zw();
    }

    virtual void update_layout(float2 Pos, float2 Size) override {
      const auto minimum_sz = core.size; // ensure_minimum_size で決まった最小サイズを保存
      core.update_layout(Pos, Size);     // 実際に渡された描画領域を元に pos, size を再計算
      const auto extra = core.size - minimum_sz; // 余ったスペース

      unsigned uc_count = 0; // 非拘束な子コントロールの数
      for (const auto& cid : controls)
        if (const auto csp = system::slot_address<control>(cid)) uc_count += !get<Vertical>(csp->core.constrained);
      float2 extra_per_uc{};
      float2 off = core.pos + padding.xy();
      get<Vertical>(extra_per_uc) = uc_count ? get<Vertical>(extra) / uc_count : 0.0f;
      float width = get<!Vertical>(core.size - padding.xy() - padding.zw());
      for (const auto& cid : controls)
        if (const auto csp = system::slot_address<control>(cid)) {
          float2 area = csp->core.area() + extra_per_uc * (int2(1, 1) - csp->core.constrained);
          get<!Vertical>(area) = width;
          csp->update_layout(off, area);
          get<Vertical>(off) += get<Vertical>(area);
        }
    }

    virtual void draw() override {
      if (!visible) return;
      brush.color(background.color);
      fill_geometry(core.geometry.get());
      d2d.push_layer(core.geometry.get());
      if (background.image) draw_bitmap(core.pos, core.size, background.image, background.image_opacity);
      for (const auto& cid : controls)
        if (const auto csp = system::slot_address<control>(cid)) csp->draw();
      d2d.pop_layer();
      border.draw(core.geometry.get());
    }

    virtual slotid hittest(float2 Pt) const override {
      if (!core.hittest(Pt)) return {};
      for (const auto& cid : controls)
        if (const auto csp = system::slot_address<control>(cid))
          if (const auto hit = csp->hittest(Pt)) return hit;
      return id;
    }

    virtual slotid next_tab_stop(slotid Focused, bool Forward, bool& Found) const override {
      for (const auto& cid : controls)
        if (const auto csp = system::slot_address<control>(cid))
          if (const auto next = csp->next_tab_stop(Focused, Forward, Found)) return next;
      return {};
    }
  };

  using control::operator bool;
  layout() noexcept = default;

  static std::expected<layout, error_trace> add(derived_from<unknown> auto& Layout) {
    layout lyt;
    if (auto res = create_control<layout>(Layout)) lyt._id = *res;
    else return unexpected_error(res.error());
    if (const auto csp = system::slot_address<layout>(lyt._id)) {
      csp->background.control_id = lyt._id;
      csp->border.control_id = lyt._id;
    } else return unexpected_error(errors::ui_invalid_slotid, "missing slot");
    return lyt;
  }
};

using horizontal_layout = layout<false>;
using vertical_layout = layout<true>;
} // namespace yw::ui
