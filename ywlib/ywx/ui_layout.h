#pragma once
#include "ywx/ui_frame.h"

namespace yw::ui {

template<bool Vertical> class layout : public frame {
public:
  struct slot : public frame::slot {
    std::vector<slotid> controls{};

    /// MARK: overrides

    virtual bool attachable() const override { return true; }

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

    virtual std::expected<float2, error_trace> calculate_necessary_size() const override {
      float2 inner;
      for (const auto& cid : controls)
        if (const auto csp = system::slot_address<control>(cid)) {
          auto bounds = csp->margin.xy() + csp->margin.zw();
          if (auto res = csp->calculate_necessary_size()) bounds += *res;
          else return unexpected_error(res.error());
          get<!Vertical>(inner) = yw::max(get<!Vertical>(inner), get<!Vertical>(bounds));
          get<Vertical>(inner) += get<Vertical>(bounds);
        } else return unexpected_error(errors::ui_invalid_slotid);
      inner += padding.xy() + padding.zw();
      return vapply_r<float2>(_calc_nec_size, size_policy, minimum_size, required_size, inner);
    }

    virtual std::expected<void, error_trace> ensure_necessary_size() override {
      float2 inner;
      for (const auto& cid : controls)
        if (const auto csp = system::slot_address<control>(cid)) {
          if (auto res = csp->ensure_necessary_size(); !res) return unexpected_error(res.error());
          const auto bounds = csp->bounds();
          get<!Vertical>(inner) = yw::max(get<!Vertical>(inner), get<!Vertical>(bounds));
          get<Vertical>(inner) += get<Vertical>(bounds);
        } else return unexpected_error(errors::ui_invalid_slotid);
      inner += padding.xy() + padding.zw();
      size = vapply_r<float2>(_calc_nec_size, size_policy, minimum_size, required_size, inner);
      return {};
    }

    virtual std::expected<void, error_trace> update_geometry(float2 Pos, float2 Area) override {
      const auto necessary_size = size;
      frame::slot::update_geometry(Pos, Area);
      if (controls.empty()) return {};
      const auto extra = size - necessary_size;
      unsigned free_count = 0;
      for (const auto& cid : controls)
        if (const auto csp = system::slot_address<control>(cid)) free_count += !bool(get<Vertical>(csp->size_policy));
        else return unexpected_error(errors::ui_invalid_slotid);
      float2 extra_per_uc{};
      float2 off = pos + padding.xy();
      const float width = get<!Vertical>(size - padding.xy() - padding.zw());
      if (free_count) {
        get<Vertical>(extra_per_uc) = get<Vertical>(extra) / free_count;
        for (const auto& cid : controls)
          if (const auto csp = system::slot_address<control>(cid)) {
            float2 area = csp->bounds() + extra_per_uc * (int2(1, 1) - vector2<bool>(csp->size_policy));
            get<!Vertical>(area) = width;
            if (auto rse = csp->update_geometry(off, area); !rse) return unexpected_error(rse.error());
            get<Vertical>(off) += get<Vertical>(area);
          } else return unexpected_error(errors::ui_invalid_slotid);
      } else {
        get<Vertical>(extra_per_uc) = get<Vertical>(extra) / controls.size();
        for (const auto& cid : controls)
          if (const auto csp = system::slot_address<control>(cid)) {
            float2 area = csp->bounds() + extra_per_uc;
            get<!Vertical>(area) = width;
            if (auto res = csp->update_geometry(off, area); !res) return unexpected_error(res.error());
            get<Vertical>(off) += get<Vertical>(area);
          } else return unexpected_error(errors::ui_invalid_slotid);
      }
      return {};
    }

    virtual std::expected<void, error_trace> draw() const override{
      if (!visible) return {};
      if (auto res = draw_background(); !res) return unexpected_error(res.error());
      for (const auto& cid : controls)
        if (const auto csp = system::slot_address<control>(cid)) {
          if (auto res = csp->draw(); !res) return unexpected_error(res.error());
        } else return unexpected_error(errors::ui_invalid_slotid);
      if (auto res = draw_foreground(); !res) return unexpected_error(res.error());
      return {};
    }

    size_t hittest_index(float2 Pt) const {
      if (!hittest_geometry(Pt)) return npos;
      for (size_t i = 0; i < controls.size(); ++i)
        if (const auto csp = system::slot_address<control>(controls[i]))
          if (csp->hittest(Pt)) return i;
      return npos;
    }

    virtual slotid hittest(float2 Pt) const override {
      if (!hittest_geometry(Pt)) return {};
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

  /// MARK: handle functions

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
      csp->make_messy();
      return {};
    } else return unexpected_error(errors::invalid_argument, "Control not found in layout");
  }

  std::expected<void, error_trace> clear() {
    const auto csp = system::slot_address<layout>(_id);
    if (!csp) return unexpected_error(errors::ui_invalid_slotid);
    for (const auto& cid : csp->controls) system::uis.erase(cid);
    csp->controls.clear();
    csp->make_messy();
    return {};
  }
};

using horizontal_layout = layout<false>;
using vertical_layout = layout<true>;
} // namespace yw::ui
