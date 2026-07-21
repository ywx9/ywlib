#pragma once
#include <ywx/ui_frame.h>

namespace yw::ui {

class layer : public frame {
public:
  struct slot : public frame::slot {
    std::vector<slotid> controls{};

    virtual bool attachable() const override { return true; }

    virtual std::expected<void, error> attach(slotid Child) override {
      const auto csp = get_slot<control>(Child);
      if (!csp) return std::unexpected(error(errors::invalid_slotid));
      csp->window_id = window_id;
      controls.push_back(Child);
      make_messy();
      return {};
    }

    virtual void close_child_controls() override {
      for (const auto& cid : controls)
        if (const auto csp = get_slot<control>(cid)) {
          csp->close_child_controls();
          interface::slot::slots.erase(cid);
        }
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
          if (const auto csp = get_slot<control>(cid))
            if (const auto next = csp->find_next_tabstop(Focused, Backward, Found)) return next;
      } else {
        for (const auto& cid : controls)
          if (const auto csp = get_slot<control>(cid))
            if (const auto next = csp->find_next_tabstop(Focused, Backward, Found)) return next;
      }
      return {};
    }

    virtual std::expected<float2, error> get_necessary_size() const override {
      float2 inner{};
      for (const auto& cid : controls) {
        const auto csp = get_slot<control>(cid);
        if (!csp) return std::unexpected(error(errors::invalid_slotid));
        if (!csp->visible) continue;
        if (auto res = csp->get_necessary_size())
          inner = vapply_r<float2>(yw::max, inner, *res + csp->margin.xy() + csp->margin.zw());
        else return res.error().relay();
      }
      inner += padding.xy() + padding.zw();
      return calc_necessary_size_by_policy(inner);
    }

    virtual slotid hittest(float2 Pt) const override {
      if (!visible) return {};
      const auto hit = frame::slot::hittest(Pt);
      if (!hit) return {};
      for (const auto& cid : controls | std::views::reverse) {
        if (const auto csp = get_slot<control>(cid))
          if (const auto hit = csp->hittest(Pt)) return hit;
      }
      return hit;
    }

    virtual std::expected<void, error> redraw() override {
      if (geometry_dirty) {
        geometry_dirty = false;
        if (auto res = relocate(); !res) return res.error().relay();
      }
      if (!visible) return {};
      if (auto res = draw_frame_background(); !res) return res.error().relay();
      for (const auto& cid : controls) {
        const auto csp = get_slot<control>(cid);
        if (!csp) return std::unexpected(error(errors::invalid_slotid));
        if (auto res = csp->redraw(); !res) return res.error().relay();
      }
      if (auto res = draw_frame_foreground(); !res) return res.error().relay();
      return {};
    }

    virtual std::expected<void, error> relocate() override {
      const auto max_size = provided_area - margin.xy() - margin.zw();
      if (auto res = set_size_to_necessary(); !res) return res.error().relay();
      if (policy.x == size_policy::free) size.x = max_size.x;
      if (policy.y == size_policy::free) size.y = max_size.y;
      pos = provided_pos + calc_offset_by_align(max_size);
      ID2D1RoundedRectangleGeometry* geom = nullptr;
      D2D1_ROUNDED_RECT rr{D2D1::RectF(pos.x, pos.y, pos.x + size.x, pos.y + size.y), radius.x, radius.y};
      hresult_test(d2d::factory()->CreateRoundedRectangleGeometry, &rr, &geom);
      geometry.reset(geom);
      const float2 area = size - padding.xy() - padding.zw();
      for (const auto& cid : controls) {
        const auto csp = get_slot<control>(cid);
        if (!csp) return std::unexpected(error(errors::invalid_slotid));
        if (!csp->visible) continue;
        if (auto res = csp->relocate(pos + padding.xy(), area); !res) return res.error().relay();
      }
      return {};
    }

    virtual std::expected<void, error> set_size_to_necessary() override {
      float2 inner{};
      for (const auto& cid : controls) {
        const auto csp = get_slot<control>(cid);
        if (!csp) return std::unexpected(error(errors::invalid_slotid));
        if (!csp->visible) continue;
        if (auto res = csp->set_size_to_necessary(); !res) return res.error().relay();
        inner = vapply_r<float2>(yw::max, inner, csp->get_bounds());
      }
      inner += padding.xy() + padding.zw();
      size = calc_necessary_size_by_policy(inner);
      return {};
    }

    virtual std::expected<void, error> apply_color_theme(const yw::ui::color_theme& Theme, bool Recursive) override {
      background_color = colors::transparent;
      border_color = colors::transparent;
      hovered_overlay_color = colors::transparent;
      if (Recursive) {
        for (const auto& cid : controls) {
          const auto csp = get_slot<control>(cid);
          if (!csp) return std::unexpected(error(errors::invalid_slotid));
          if (auto res = csp->apply_color_theme(Theme, true); !res) return res.error().relay();
        }
      }
      make_dirty();
      return {};
    }
  };

  layer() noexcept = default;

  layer(derived_from<interface> auto& Parent, const source_line& sl = here()) {
    if (auto res = create(Parent)) *this = std::move(*res);
    else res.error().add_footprint().go_off(sl);
  }

  static std::expected<layer, error> create(derived_from<interface> auto& Parent) {
    layer l;
    const auto temp_id = make_slot<layer>();
    const auto sp = get_slot<layer>(temp_id);
    if (!sp) return std::unexpected(error(errors::slot_creation_failed));
    const auto psp = get_slot<control>(Parent.id());
    if (!psp) return std::unexpected(error(errors::invalid_slotid));
    if (auto res = psp->attach(temp_id); !res) {
      slot::slots.erase(temp_id);
      return res.error().relay();
    }
    l._id = temp_id;
    sp->id = temp_id;
    sp->window_id = psp->get_window_id();
    sp->margin = {};
    sp->padding = {};
    sp->radius = {};
    if (auto res = sp->apply_current_color_theme(false); !res) return res.error().relay();
    return l;
  }
};
} // namespace yw::ui
