#pragma once
#include <ywx/control.h>

namespace yw::ui {

class layer : public control {
public:
  struct slot : public control::slot {
    std::vector<slotid> controls{};

    //-- override functions --//

    virtual bool attachable() const override { return true; }

    virtual std::expected<void, error> attach(slotid Child) override {
      const auto csp = get_slot<control>(Child);
      if (!csp) return std::unexpected(error(errors::invalid_slotid));
      if (csp->parent_id) return std::unexpected(error(errors::invalid_operation, "control already attached"));
      csp->parent_id = id;
      csp->set_window_id(window_id);
      controls.push_back(Child);
      make_messy();
      return {};
    }

    virtual void close_child_controls() override {
      for (const auto& cid : controls)
        if (const auto csp = get_slot<control>(cid)) csp->clear_attachment();
      controls.clear();
    }

    virtual void set_window_id(slotid Window) noexcept override {
      window_id = Window;
      for (const auto& cid : controls)
        if (const auto csp = get_slot<control>(cid)) csp->set_window_id(Window);
    }

    virtual std::expected<void, error> detach(slotid Child) override {
      const auto old_size = controls.size();
      controls.erase(std::remove(controls.begin(), controls.end(), Child), controls.end());
      if (old_size == controls.size())
        return std::unexpected(error(errors::invalid_operation, "not attached to this control"));
      if (const auto csp = get_slot<control>(Child)) csp->clear_attachment();
      else return std::unexpected(error(errors::invalid_slotid));
      make_messy();
      return {};
    }

    std::expected<void, error> replace(slotid From, slotid To) {
      if (From == To) return {};
      const auto it = std::ranges::find(controls, From);
      if (it == controls.end()) return std::unexpected(error(errors::invalid_operation, "not attached to this control"));
      const auto tsp = get_slot<control>(To);
      if (!tsp) return std::unexpected(error(errors::invalid_slotid));
      if (tsp->parent_id) return std::unexpected(error(errors::invalid_operation, "control already attached"));
      if (const auto fsp = get_slot<control>(From)) fsp->clear_attachment();
      else return std::unexpected(error(errors::invalid_slotid));
      tsp->parent_id = id;
      tsp->set_window_id(window_id);
      *it = To;
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

    virtual bool2 has_free_size_policy() const override {
      uint2 free{};
      for (const auto& cid : controls)
        if (const auto csp = get_slot<control>(cid)) free += csp->has_free_size_policy();
      return free * control::slot::has_free_size_policy();
    }

    virtual slotid hittest(float2 Pt) const override {
      if (!visible) return {};
      const auto hit = control::slot::hittest(Pt);
      if (!hit) return {};
      for (const auto& cid : controls | std::views::reverse) {
        if (const auto csp = get_slot<control>(cid))
          if (const auto hit = csp->hittest(Pt)) return hit;
      }
      return hit;
    }

    virtual std::expected<void, error> draw_backcontent(interface::slot* Window) override {
      for (const auto& cid : controls) {
        const auto csp = get_slot<control>(cid);
        if (!csp) return std::unexpected(error(errors::invalid_slotid));
        if (auto res = csp->redraw(Window); !res) return res.error().relay();
      }
      return {};
    }

    virtual std::expected<void, error> relocate() override {
      if (auto res = update_geometry(); !res) return res.error().relay();
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

  };

  layer() noexcept = default;

  layer(derived_from<interface> auto& Parent, const source_line& sl = here()) {
    if (auto res = create(Parent)) *this = std::move(*res);
    else res.error().add_footprint().go_off(sl);
  }

  static std::expected<layer, error> create() {
    layer l;
    layer::slot* sp;
    if (auto res = create_control<layer>()) sp = *res;
    else return res.error().relay();
    l._id = sp->id;
    sp->margin = {};
    sp->padding = {};
    sp->radius = {};
    sp->background_color = colors::transparent;
    sp->border_color = colors::transparent;
    return l;
  }

  static std::expected<layer, error> create(derived_from<interface> auto& Parent) {
    auto res = create();
    if (!res) return res.error().relay();
    if (auto attached = res->attach(Parent); !attached) return attached.error().relay();
    return res;
  }

  std::expected<void, error> replace(derived_from<control> auto& From, derived_from<control> auto& To) {
    const auto sp = get_slot(this);
    if (!sp) return std::unexpected(error(errors::not_initialized));
    if (auto res = sp->replace(From.id(), To.id()); !res) return res.error().relay();
    return {};
  }
};
} // namespace yw::ui
