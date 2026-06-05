#pragma once
#include "ywx/ui_frame.h"

namespace yw::ui {

class layer : public frame {
public:
  /// MARK: slot

  struct slot : public frame::slot {
    std::vector<unknown_slotid> controls;

    virtual bool attachable() const override { return true; }

    virtual std::expected<void, error_trace> attach(unknown_slotid Child) override {
      controls.push_back(Child);
      make_messy();
      return {};
    }

    virtual std::expected<void, error_trace> detach(unknown_slotid Child) override {
      controls.erase(std::remove(controls.begin(), controls.end(), Child), controls.end());
      system::unknowns.erase(Child);
      make_messy();
      return {};
    }

    virtual std::expected<float2, error_trace> calculate_necessary_size() const override {
      float2 inner;
      for (const auto& cid : controls)
        if (const auto csp = system::get_slot_pointer<control>(cid)) {
          if (auto res = csp->calculate_necessary_size()) inner = vapply_r<float2>(yw::max, inner, *res);
          else return unexpected_error(res.error());
        } else return unexpected_error(errors::invalid_slotid);
      inner += padding.xy() + padding.zw();
      return vapply_r<float2>(_calc_nec_size, size_policy, minimum_size, required_size, inner);
    }

    virtual std::expected<void, error_trace> ensure_necessary_size() override {
      float2 inner;
      for (const auto& cid : controls)
        if (const auto csp = system::get_slot_pointer<control>(cid)) {
          if (auto res = csp->ensure_necessary_size(); !res) return unexpected_error(res.error());
          inner = vapply_r<float2>(yw::max, inner, csp->size);
        } else return unexpected_error(errors::invalid_slotid);
      inner += padding.xy() + padding.zw();
      size = vapply_r<float2>(_calc_nec_size, size_policy, minimum_size, required_size, inner);
      return {};
    }

    virtual std::expected<void, error_trace> update_geometry(float2 Pos, float2 Area) override {
      frame::slot::update_geometry(Pos, Area);
      if (controls.empty()) return {};
      const auto _pos = pos + padding.xy();
      const auto _area = size - padding.xy() - padding.zw();
      for (const auto& cid : controls)
        if (const auto csp = system::get_slot_pointer<control>(cid)) {
          if (auto res = csp->update_geometry(_pos, _area); !res) return unexpected_error(res.error());
        } else return unexpected_error(errors::invalid_slotid);
      return {};
    }

    virtual std::expected<void, error_trace> draw() const override {
      if (!visible) return {};
      if (auto res = draw_background(); !res) return unexpected_error(res.error());
      for (const auto& cid : controls)
        if (const auto csp = system::get_slot_pointer<control>(cid)) {
          if (auto res = csp->draw(); !res) return unexpected_error(res.error());
        } else return unexpected_error(errors::invalid_slotid);
      if (auto res = draw_foreground(); !res) return unexpected_error(res.error());
      return {};
    }

    virtual unknown_slotid hittest(float2 Pt) const override {
      if (!hittest_geometry(Pt)) return {};
      if (!controls.empty())
        if (const auto csp = system::get_slot_pointer<control>(controls.back()))
          if (const auto hit = csp->hittest(Pt)) return hit;
      return id;
    }

    virtual unknown_slotid next_tab_stop(unknown_slotid Focused, bool Forward, bool& Found) const override {
      if (const auto csp = system::get_slot_pointer<control>(controls.back()))
        if (const auto next = csp->next_tab_stop(Focused, Forward, Found)) return next;
      return {};
    }
  };

  /// MARK: handle functions

  using control::operator bool;
  layer() noexcept = default;

  static std::expected<layer, error_trace> add(derived_from<unknown> auto& Layout) {
    layer lyt;
    if (auto res = create_control<layer>(Layout)) lyt._id = *res;
    else return unexpected_error(res.error());
    if (!system::unknowns.contains(lyt._id)) return unexpected_error(errors::invalid_slotid);
    return lyt;
  }

  std::expected<void, error_trace> erase(derived_from<control> auto& Control) {
    const auto csp = system::get_slot_pointer<layer>(_id);
    if (!csp) return unexpected_error(errors::invalid_slotid);
    const auto cid = Control._id;
    if (auto it = std::find(csp->controls.begin(), csp->controls.end(), cid); it != csp->controls.end()) {
      csp->controls.erase(it);
      system::unknowns.erase(cid);
      csp->make_messy();
      return {};
    } else return unexpected_error(errors::invalid_argument, "Control not found in layer");
  }

  std::expected<void, error_trace> clear() {
    const auto csp = system::get_slot_pointer<layer>(_id);
    if (!csp) return unexpected_error(errors::invalid_slotid);
    for (const auto& cid : csp->controls) system::unknowns.erase(cid);
    csp->controls.clear();
    csp->make_messy();
    return {};
  }
};
} // namespace yw::ui
