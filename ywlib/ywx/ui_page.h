#pragma once
#include <ywx/ui_layer.h>

namespace yw::ui {

class page : public layer {
public:
  struct slot : layer::slot {
    size_t active_index = 0;

    slotid active_control_id() const noexcept {
      if (active_index >= controls.size()) return {};
      return controls[active_index];
    }

    control::slot* active_control() const noexcept {
      if (const auto cid = active_control_id()) return get_slot<control>(cid);
      return nullptr;
    }

    //-- override functions --//

    virtual std::expected<void, error> detach(slotid Child) override {
      const auto it = std::ranges::find(controls, Child);
      const auto erased_index = it == controls.end() ? npos : size_t(it - controls.begin());
      if (erased_index == active_index)
        if (const auto csp = get_slot<control>(Child)) csp->clear_window_state();

      if (auto res = layer::slot::detach(Child); !res) return res.error().relay();

      if (controls.empty()) active_index = 0;
      else if (active_index >= controls.size()) active_index = controls.size() - 1;
      else if (erased_index < active_index) --active_index;
      return {};
    }

    virtual slotid find_next_tabstop(slotid Focused, bool Backward, bool& Found) const override {
      if (const auto csp = active_control())
        if (const auto next = csp->find_next_tabstop(Focused, Backward, Found)) return next;
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
      if (const auto csp = active_control())
        if (const auto child_hit = csp->hittest(Pt)) return child_hit;
      return hit;
    }

    virtual std::expected<void, error> draw_backcontent(interface::slot* Window) override {
      if (const auto csp = active_control()) {
        if (auto res = csp->redraw(Window); !res) return res.error().relay();
      } else if (active_index < controls.size()) return std::unexpected(error(errors::invalid_slotid));
      return {};
    }

    virtual std::expected<void, error> relocate() override {
      if (auto res = update_geometry(); !res) return res.error().relay();
      const float2 area = size - padding.xy() - padding.zw();
      if (const auto csp = active_control()) {
        if (!csp->visible) return {};
        if (auto res = csp->relocate(pos + padding.xy(), area); !res) return res.error().relay();
      } else if (active_index < controls.size()) return std::unexpected(error(errors::invalid_slotid));
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

    //-- shared functions --//

    std::expected<void, error> set_active(size_t Index) {
      if (Index >= controls.size()) return std::unexpected(error(errors::invalid_argument, "invalid page index"));
      if (active_index == Index) return {};
      if (const auto csp = active_control()) csp->clear_window_state();
      active_index = Index;
      make_messy();
      return {};
    }
  };

  class proxy : public control::proxy {
    friend class page;
    using control::proxy::proxy;
    page::slot* _get_slot() const noexcept { return static_cast<page::slot*>(_slot); }

  public:
    //-- getter --//

    size_t active() const&& noexcept { return _get_slot()->active_index; }
    slotid active_control_id() const&& noexcept { return _get_slot()->active_control_id(); }
    size_t count() const&& noexcept { return _get_slot()->controls.size(); }

    //-- setter --//

    auto active(this auto&& Self, size_t Index) noexcept {
      if (auto res = Self._get_slot()->set_active(Index); !res) res.error().fizzle_out();
      return std::move(Self);
    }
  };

  page() noexcept = default;

  page(derived_from<interface> auto& Parent, const source_line& sl = here()) {
    if (auto res = create(Parent)) *this = std::move(*res);
    else res.error().add_footprint().go_off(sl);
  }

  static std::expected<page, error> create() {
    page p;
    page::slot* sp;
    if (auto res = create_control<page>()) sp = *res;
    else return res.error().relay();
    p._id = sp->id;
    sp->margin = {};
    sp->padding = {};
    sp->radius = {};
    sp->background_color = colors::transparent;
    sp->border_color = colors::transparent;
    return p;
  }

  static std::expected<page, error> create(derived_from<interface> auto& Parent) {
    auto res = create();
    if (!res) return res.error().relay();
    if (auto attached = res->attach(Parent); !attached) return attached.error().relay();
    return res;
  }

  yw_control_getter_setter(active, size_t);
  yw_control_getter(active_control_id);
  yw_control_getter(count);
};
} // namespace yw::ui
