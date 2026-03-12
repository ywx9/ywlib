#pragma once
#include "ywx/ui_control.h"

namespace yw::ui {

class horizontal : public control {
public:
  class slot : public control::slot {
  public:
    std::vector<slotset<control::slot>::slotid> children;

    virtual tuple<float2, uint2> minimum_size() const noexcept override {
      tuple<float2, uint2> result{};
      for (const auto& child_id : children)
        if (const auto child = system::controls.get(child_id)) {
          const auto [child_min_size, child_fc_count] = child->minimum_size();
          result.first.x += child_min_size.x;
          result.first.y = std::max(result.first.y, child_min_size.y);
          result.second.x += child_fc_count.x;
          result.second.y = std::max(result.second.y, child_fc_count.y);
        }
      return result;
    }

    virtual void erase_child(const slotset<control::slot>::slotid& child_id) noexcept override {
      if (auto it = std::find(children.begin(), children.end(), child_id); it != children.end()) {
        children.erase(it);
        make_dirty();
      }
    }

    virtual bool add_child(const slotset<control::slot>::slotid& child_id) noexcept override {
      children.push_back(child_id);
      make_dirty();
      return true;
    }

    virtual void draw(float2 Pos, float2 Size) const override {
      if (!visible || Size.x <= 0.0f || Size.y <= 0.0f || children.empty()) return;
      const auto [min_size, fc_count] = minimum_size();
      const auto position = Pos + margin.xy();
      const auto available_size = Size - margin.xy() - margin.zw();
      const auto extra_size = available_size - min_size;
      float offset = 0.0f;
      if (fc_count.x == 0) {
        for (const auto& child_id : children)
          if (const auto child = system::controls.get(child_id)) {
            const auto [child_min_size, _] = child->minimum_size();
            const auto child_size = float2{child_min_size.x, available_size.y};
            child->draw(position + float2{offset, 0.0f}, child_size);
            offset += child_min_size.x;
          }
      } else {
        const auto extra_per_fc = extra_size.x / fc_count.x;
        for (const auto& child_id : children)
          if (const auto child = system::controls.get(child_id)) {
            const auto [child_min_size, child_fc_count] = child->minimum_size();
            const auto child_extra_size = extra_per_fc * child_fc_count.x;
            const auto child_size = float2{child_min_size.x + child_extra_size, available_size.y};
            child->draw(position + float2{offset, 0.0f}, child_size);
            offset += child_size.x;
          }
      }
    }
  };

  using control::control;
  horizontal() noexcept = default;
  horizontal(control& Layout) noexcept : control(create<horizontal>(Layout)) {}
};
}
