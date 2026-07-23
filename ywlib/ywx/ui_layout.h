#pragma once
#include <ywx/control.h>

namespace yw::ui {

template<orientation Orientation> class layout : public control {
  static constexpr bool Vert = Orientation == orientation::vertical;
public:
  struct slot : public control::slot {
    std::vector<slotid> controls{};

    //-- override functions --//

    virtual std::expected<void, error> apply_color_theme(const yw::ui::color_theme& Theme, bool Recursive) override {
      background_color = colors::transparent;
      border_color = colors::transparent;
      if (Recursive)
        for (const auto& cid : controls)
          if (const auto csp = get_slot<control>(cid)) {
            if (auto res = csp->apply_color_theme(Theme, true); !res) return res.error().relay();
          } else return std::unexpected(error(errors::invalid_slotid));
      make_dirty();
      return {};
    }

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
      if (const auto csp = get_slot<control>(Child)) csp->clear_window_state();
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
        if (auto res = csp->get_necessary_size()) {
          const auto bounds = *res + csp->margin.xy() + csp->margin.zw();
          yw::get<!Vert>(inner) = yw::max(yw::get<!Vert>(inner), yw::get<!Vert>(bounds));
          yw::get<Vert>(inner) += yw::get<Vert>(bounds);
        } else return res.error().relay();
      }
      inner += padding.xy() + padding.zw();
      return calc_necessary_size_by_policy(inner);
    }

    virtual slotid hittest(float2 Pt) const override {
      if (!visible) return {};
      if (const auto hit = control::slot::hittest(Pt); !hit) return {};
      for (const auto& cid : controls | std::views::reverse)
        if (const auto csp = get_slot<control>(cid))
          if (const auto hit = csp->hittest(Pt)) return hit;
      return id;
    }

    virtual std::expected<void, error> draw_content() override {
      for (const auto& cid : controls)
        if (const auto csp = get_slot<control>(cid)) {
          if (auto res = csp->redraw(); !res) return res.error().relay();
        } else return std::unexpected(error(errors::invalid_slotid));
      return {};
    }

    virtual std::expected<void, error> relocate() override {
      float2 extra;
      if (auto res = update_geometry()) extra = *res;
      else return res.error().relay();
      if (extra[Vert] > 0) {
        unsigned visible_count = 0;
        unsigned free_count = 0;
        for (const auto& cid : controls) {
          const auto csp = get_slot<control>(cid);
          if (!csp) return std::unexpected(error(errors::invalid_slotid));
          if (!csp->visible) continue;
          ++visible_count;
          free_count += (csp->policy[Vert] == size_policy::free);
        }
        if (visible_count == 0) return {};
        const auto cross = size[!Vert] - padding[!Vert] - padding[2 + !Vert];
        float2 offset = padding.xy();
        if (free_count > 0) {
          const auto extra_per_free = extra[Vert] / float(free_count);
          for (const auto& cid : controls) {
            const auto csp = get_slot<control>(cid);
            if (!csp) return std::unexpected(error(errors::invalid_slotid));
            if (!csp->visible) continue;
            float2 area = csp->get_bounds();
            area[Vert] += extra_per_free * (csp->policy[Vert] == size_policy::free);
            area[!Vert] = cross;
            if (auto res = csp->relocate(pos + offset, area); !res) return res.error().relay();
            offset[Vert] += area[Vert];
          }
        } else {
          for (const auto& cid : controls) {
            const auto csp = get_slot<control>(cid);
            if (!csp) return std::unexpected(error(errors::invalid_slotid));
            if (!csp->visible) continue;
            float2 area = csp->get_bounds();
            area[!Vert] = cross;
            if (auto res = csp->relocate(pos + offset, area); !res) return res.error().relay();
            offset[Vert] += area[Vert];
          }
        }
      } else {
        float2 offset = padding.xy();
        const auto cross = size[!Vert] - padding[!Vert] - padding[2 + !Vert];
        for (const auto& cid : controls) {
          const auto csp = get_slot<control>(cid);
          if (!csp) return std::unexpected(error(errors::invalid_slotid));
          if (!csp->visible) continue;
          float2 area = csp->get_bounds();
          area[!Vert] = cross;
          if (auto res = csp->relocate(pos + offset, area); !res) return res.error().relay();
          offset[Vert] += area[Vert];
        }
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
        const auto bounds = csp->get_bounds();
        inner[!Vert] = yw::max(inner[!Vert], bounds[!Vert]);
        inner[Vert] += bounds[Vert];
      }
      inner += padding.xy() + padding.zw();
      size = calc_necessary_size_by_policy(inner);
      return {};
    }
  };

  layout() noexcept = default;

  layout(derived_from<interface> auto& Parent, const source_line& sl = here()) {
    if (auto res = create(Parent)) *this = std::move(*res);
    else res.error().add_footprint().go_off(sl);
  }

  static std::expected<layout, error> create(derived_from<interface> auto& Parent) {
    layout l;
    const auto temp_id = make_slot<layout>();
    const auto sp = get_slot<layout>(temp_id);
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
    if (auto theme = sp->get_color_theme(); !theme) return theme.error().relay();
    else if (auto res = sp->apply_color_theme(*(*theme), false); !res) return res.error().relay();
    return l;
  }
};

using hlayout = layout<orientation::horizontal>;
using vlayout = layout<orientation::vertical>;

using horizontal_layout = layout<orientation::horizontal>;
using vertical_layout = layout<orientation::vertical>;
} // namespace yw::ui
