#pragma once
#include <ywx/ui_frame.h>

namespace yw::ui {

template<bool Vertical> class layout : public frame {
public:
  struct slot : public frame::slot {
    std::vector<slotid> controls{};

    //-- override functions --//

    virtual bool attachable() const override { return true; }

    virtual std::expected<void, error> attach(slotid Child) override {
      const auto csp = interface::slot::get<control>(Child);
      if (!csp) return std::unexpected(error(errors::invalid_slotid));
      csp->layout_id = id;
      csp->window_id = window_id;
      controls.push_back(Child);
      make_messy();
      return {};
    }

    virtual std::expected<void, error> detach(slotid Child) override {
      controls.erase(std::remove(controls.begin(), controls.end(), Child), controls.end());
      interface::slot::slots.erase(Child);
      make_messy();
      return {};
    }

    virtual slotid find_next_tabstop(slotid Focused, bool Backward, bool& Found) const override {
      if (Backward) {
        for (const auto& cid : controls | std::views::reverse)
          if (const auto csp = interface::slot::get<control>(cid))
            if (const auto next = csp->find_next_tabstop(Focused, Backward, Found)) return next;
      } else {
        for (const auto& cid : controls)
          if (const auto csp = interface::slot::get<control>(cid))
            if (const auto next = csp->find_next_tabstop(Focused, Backward, Found)) return next;
      }
      return {};
    }

    virtual std::expected<float2, error> get_necessary_size() const override {
      float2 inner{};
      for (const auto& cid : controls) {
        const auto csp = interface::slot::get<control>(cid);
        if (!csp) return std::unexpected(error(errors::invalid_slotid));
        if (!csp->visible) continue;
        if (auto res = csp->get_necessary_size()) {
          const auto bounds = *res + csp->margin.xy() + csp->margin.zw();
          yw::get<!Vertical>(inner) = yw::max(yw::get<!Vertical>(inner), yw::get<!Vertical>(bounds));
          yw::get<Vertical>(inner) += yw::get<Vertical>(bounds);
        } else return res.error().relay();
      }
      inner += padding.xy() + padding.zw();
      return vapply_r<float2>(_necessary_size, policy, minimum_size, required_size, inner);
    }

    virtual slotid hittest(float2 Pt) const override {
      if (!visible) return {};
      for (const auto& cid : controls | std::views::reverse) {
        if (const auto csp = interface::slot::get<control>(cid))
          if (const auto hit = csp->hittest(Pt)) return hit;
      }
      return frame::slot::hittest(Pt);
    }

    virtual std::expected<void, error> redraw() override {
      if (geometry_dirty) {
        geometry_dirty = false;
        if (auto res = relocate(); !res) return res.error().relay();
      }
      if (!visible) return {};
      if (auto res = _draw_background(); !res) return res.error().relay();
      for (const auto& cid : controls) {
        const auto csp = interface::slot::get<control>(cid);
        if (!csp) return std::unexpected(error(errors::invalid_slotid));
        if (auto res = csp->redraw(); !res) return res.error().relay();
      }
      if (auto res = _draw_foreground(); !res) return res.error().relay();
      return {};
    }

    virtual std::expected<void, error> relocate() override {
      const auto max_size = provided_area - margin.xy() - margin.zw();
      if (auto res = set_size_to_necessary(); !res) return res.error().relay();
      const auto extra = max_size - size;
      if (policy.x == size_policy::free) size.x = max_size.x;
      if (policy.y == size_policy::free) size.y = max_size.y;
      pos = provided_pos + _offset(max_size);
      ID2D1RoundedRectangleGeometry* geom = nullptr;
      D2D1_ROUNDED_RECT rr{D2D1::RectF(pos.x, pos.y, pos.x + size.x, pos.y + size.y), radius.x, radius.y};
      hresult_test(d2d::factory()->CreateRoundedRectangleGeometry, &rr, &geom);
      geometry.reset(geom);
      if (extra[Vertical] > 0) {
        unsigned visible_count = 0;
        unsigned free_count = 0;
        for (const auto& cid : controls) {
          const auto csp = interface::slot::get<control>(cid);
          if (!csp) return std::unexpected(error(errors::invalid_slotid));
          if (!csp->visible) continue;
          ++visible_count;
          free_count += (csp->policy[Vertical] == size_policy::free);
        }
        if (visible_count == 0) return {};
        const auto cross = size[!Vertical] - padding[!Vertical] - padding[2 + !Vertical];
        float2 offset = padding.xy();
        if (free_count > 0) {
          const auto extra_per_free = extra[Vertical] / float(free_count);
          for (const auto& cid : controls) {
            const auto csp = interface::slot::get<control>(cid);
            if (!csp) return std::unexpected(error(errors::invalid_slotid));
            if (!csp->visible) continue;
            float2 area = csp->bounds();
            area[Vertical] += extra_per_free * (csp->policy[Vertical] == size_policy::free);
            area[!Vertical] = cross;
            if (auto res = csp->relocate(pos + offset, area); !res) return res.error().relay();
            offset[Vertical] += area[Vertical];
          }
        } else {
          const auto extra_per_control = extra[Vertical] / float(visible_count);
          for (const auto& cid : controls) {
            const auto csp = interface::slot::get<control>(cid);
            if (!csp) return std::unexpected(error(errors::invalid_slotid));
            if (!csp->visible) continue;
            float2 area = csp->bounds();
            area[Vertical] += extra_per_control;
            area[!Vertical] = cross;
            if (auto res = csp->relocate(pos + offset, area); !res) return res.error().relay();
            offset[Vertical] += area[Vertical];
          }
        }
      } else {
        float2 offset = padding.xy();
        for (const auto& cid : controls) {
          const auto csp = interface::slot::get<control>(cid);
          if (!csp) return std::unexpected(error(errors::invalid_slotid));
          if (!csp->visible) continue;
          float2 area = csp->bounds();
          if (auto res = csp->relocate(pos + offset, area); !res) return res.error().relay();
          offset[Vertical] += area[Vertical];
        }
      }
      return {};
    }

    virtual std::expected<void, error> set_size_to_necessary() override {
      float2 inner{};
      for (const auto& cid : controls) {
        const auto csp = interface::slot::get<control>(cid);
        if (!csp) return std::unexpected(error(errors::invalid_slotid));
        if (!csp->visible) continue;
        if (auto res = csp->set_size_to_necessary(); !res) return res.error().relay();
        const auto bounds = csp->bounds();
        inner[!Vertical] = yw::max(inner[!Vertical], bounds[!Vertical]);
        inner[Vertical] += bounds[Vertical];
      }
      inner += padding.xy() + padding.zw();
      size = vapply_r<float2>(_necessary_size, policy, minimum_size, required_size, inner);
      return {};
    }
  };

  using control::operator bool;
  layout() noexcept = default;

  layout(derived_from<interface> auto& Parent, const source_line& sl = here()) {
    if (auto res = frame::slot::create<layout>(Parent, false, sl)) {
      const auto sp = *res;
      _id = sp->id;
      sp->margin = {};
      sp->padding = {};
      sp->colors.background = colors::transparent;
      sp->colors.border = colors::transparent;
    } else res.error().add_footprint().go_off(sl);
  }

  template<typename... Ts> requires constructible<layout, Ts...>
  static std::expected<layout, error> create(Ts&&... Args) {
    if (auto res = frame::slot::create<layout>(static_cast<Ts&&>(Args)...)) {
      const auto sp = *res;
      layout l;
      l._id = sp->id;
      sp->margin = {};
      sp->padding = {};
      sp->colors.background = colors::transparent;
      sp->colors.border = colors::transparent;
      return l;
    } else return res.error().relay();
  }

  template<typename Layout> static std::expected<layout, error> add(Layout& Layout_) { return create(Layout_); }

  template<typename Ctrl> std::expected<void, error> erase(Ctrl& Control) {
    const auto csp = interface::slot::get<layout>(_id);
    if (!csp) return std::unexpected(error(errors::invalid_slotid));
    const auto cid = Control.id();
    if (auto it = std::find(csp->controls.begin(), csp->controls.end(), cid); it != csp->controls.end()) {
      csp->controls.erase(it);
      interface::slot::slots.erase(cid);
      if (auto res = csp->make_messy(); !res) return res.error().relay();
      return {};
    }
    return std::unexpected(error(errors::invalid_argument, "Control not found in layout"));
  }

  std::expected<void, error> clear() {
    const auto csp = interface::slot::get<layout>(_id);
    if (!csp) return std::unexpected(error(errors::invalid_slotid));
    for (const auto& cid : csp->controls) interface::slot::slots.erase(cid);
    csp->controls.clear();
    if (auto res = csp->make_messy(); !res) return res.error().relay();
    return {};
  }
};

using horizontal_layout = layout<false>;
using vertical_layout = layout<true>;
} // namespace yw::ui
