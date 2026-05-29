#pragma once
#include "ywx/ui_control.h"

namespace yw::ui {

template<bool Vertical> class layout : public frame {
public:
  struct slot : public frame::slot {
    std::vector<slotid> controls{};
    float4 padding = float4::fill(arbitrary_value);

    //-- overrides --//

    virtual std::expected<void, error_trace> attachable() const override { return {}; }

    virtual std::expected<void, error_trace> attach(slotid Child) override {
      controls.push_back(Child);
      make_messy();
      return {};
    }

    virtual std::expected<void, error_trace> detach(slotid Child) override {
      controls.erase(std::remove(controls.begin(), controls.end(), Child), controls.end());
      system::uis.erase(Child);
      make_messy();
      return {};
    }

    virtual std::expected<float2, error_trace> calculate_minimum_size() const override {
      float2 internal;
      for (const auto& cid : controls)
        if (const auto csp = system::slot_address<control>(cid)) {
          auto bounds = csp->margin.xy() + csp->margin.zw();
          if (auto res = csp->calculate_minimum_size()) bounds += *res;
          else return unexpected_error(res.error());
          get<!Vertical>(internal) = yw::max(get<!Vertical>(internal), get<!Vertical>(bounds));
          get<Vertical>(internal) += get<Vertical>(bounds);
        } else return unexpected_error(errors::ui_invalid_slotid);
      return vapply_r<float2>(yw::max, min_size, internal, required_size * constrained) + padding.xy() + padding.zw();
    }

    virtual std::expected<void, error_trace> ensure_minimum_size() override {
      float2 internal;
      for (const auto& cid : controls)
        if (const auto csp = system::slot_address<control>(cid)) {
          if (auto res = csp->ensure_minimum_size(); !res) return unexpected_error(res.error());
          const auto bounds = csp->bounds();
          get<!Vertical>(internal) = yw::max(get<!Vertical>(internal), get<!Vertical>(bounds));
          get<Vertical>(internal) += get<Vertical>(bounds);
        } else return unexpected_error(errors::ui_invalid_slotid);
      size = vapply_r<float2>(yw::max, min_size, internal, required_size * constrained) + padding.xy() + padding.zw();
      return {};
    }

    virtual std::expected<void, error_trace> update_geometry() override {


      // const auto minimum_sz = size;
      // update_geometry(Pos, Size);
      // const auto extra = size - minimum_sz;
      // unsigned uc_count = 0;
      // for (const auto& cid : controls)
      //   if (const auto csp = system::slot_address<control>(cid)) uc_count += !get<Vertical>(csp->constrained);
      // float2 extra_per_uc{};
      // float2 off = pos + padding.xy();
      // get<Vertical>(extra_per_uc) = uc_count ? get<Vertical>(extra) / uc_count : 0.0f;
      // float width = get<!Vertical>(size - padding.xy() - padding.zw());
      // for (const auto& cid : controls)
      //   if (const auto csp = system::slot_address<control>(cid)) {
      //     float2 area = csp->bounds() + extra_per_uc * (int2(1, 1) - csp->constrained);
      //     get<!Vertical>(area) = width;
      //     csp->update_geometry(off, area);
      //     get<Vertical>(off) += get<Vertical>(area);
      //   }
      // return {};
    }

    virtual std::expected<void, error_trace> draw() override {
      if (!visible) return {};
      if (auto res = background.draw(core); !res) return unexpected_error(res.error());
      for (const auto& cid : controls)
        if (const auto csp = system::slot_address<control>(cid))
          if (auto res = csp->draw(); !res) return unexpected_error(res.error());
      if (auto res = border.draw(core); !res) return unexpected_error(res.error());
      return {};
    }

    size_t hittest_index(float2 Pt) const {
      if (!core.hittest(Pt)) return npos;
      for (size_t i = 0; i < controls.size(); ++i) {
        if (const auto csp = system::slot_address<control>(controls[i]))
          if (csp->core.hittest(Pt)) return i;
      }
      return npos;
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
    if (!system::uis.contains(lyt._id)) return unexpected_error(errors::ui_invalid_slotid);
    return lyt;
  }

  std::expected<void, error_trace> erase(derived_from<control> auto& Control) {
    const auto csp = system::slot_address<layout>(_id);
    if (!csp) return unexpected_error(errors::ui_invalid_slotid);
    const auto cid = Control._id;
    if (auto it = std::find(csp->controls.begin(), csp->controls.end(), cid); it != csp->controls.end()) {
      csp->controls.erase(it);
      system::uis.erase(cid);
      return {};
    } else return unexpected_error(errors::invalid_argument, "Control not found in layout");
  }

  std::expected<void, error_trace> clear() {
    const auto csp = system::slot_address<layout>(_id);
    if (!csp) return unexpected_error(errors::ui_invalid_slotid);
    for (const auto& cid : csp->controls) system::uis.erase(cid);
    csp->controls.clear();
    return {};
  }
};

using horizontal_layout = layout<false>;
using vertical_layout = layout<true>;
} // namespace yw::ui
