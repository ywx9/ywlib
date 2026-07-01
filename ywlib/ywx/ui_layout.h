#pragma once
#include <ywx/ui_frame.h>

namespace yw::ui {

template<bool Vertical> class layout : public frame {
public:
  struct slot : public frame::slot {
    std::vector<slotid> controls{};

    virtual bool attachable() const override { return true; }

    virtual std::expected<void, error> attach(slotid Child) override {
      controls.push_back(Child);
      if (auto res = make_messy(); !res) return res.error().relay();
      return {};
    }

    virtual std::expected<void, error> detach(slotid Child) override {
      controls.erase(std::remove(controls.begin(), controls.end(), Child), controls.end());
      interface::slot::slots.erase(Child);
      if (auto res = make_messy(); !res) return res.error().relay();
      return {};
    }

    virtual std::expected<float2, error> calculate_necessary_size() const override {
      float2 inner{};
      for (const auto& cid : controls) {
        const auto csp = interface::slot::get<control>(cid);
        if (!csp) return std::unexpected(error(errors::invalid_slotid));
        auto bounds = csp->margin.xy() + csp->margin.zw();
        if (auto res = csp->calculate_necessary_size()) bounds += *res;
        else return res.error().relay();
        get<!Vertical>(inner) = yw::max(get<!Vertical>(inner), get<!Vertical>(bounds));
        get<Vertical>(inner) += get<Vertical>(bounds);
      }
      return vapply_r<float2>(calculate_necessary_width, size_policy, minimum_size, required_size, inner);
    }

    virtual std::expected<void, error> ensure_necessary_size() override {
      float2 inner{};
      for (const auto& cid : controls) {
        const auto csp = interface::slot::get<control>(cid);
        if (!csp) return std::unexpected(error(errors::invalid_slotid));
        if (auto res = csp->ensure_necessary_size(); !res) return res.error().relay();
        const auto bounds = csp->bounds();
        get<!Vertical>(inner) = yw::max(get<!Vertical>(inner), get<!Vertical>(bounds));
        get<Vertical>(inner) += get<Vertical>(bounds);
      }
      size = vapply_r<float2>(calculate_necessary_width, size_policy, minimum_size, required_size, inner);
      return {};
    }

    virtual std::expected<void, error> update_geometry(float2 Pos, float2 Area) override {
      const auto necessary_size = size;
      if (auto res = frame::slot::update_geometry(Pos, Area); !res) return res.error().relay();
      if (controls.empty()) return {};

      const auto extra = size - necessary_size;
      unsigned free_count = 0;
      for (const auto& cid : controls) {
        const auto csp = interface::slot::get<control>(cid);
        if (!csp) return std::unexpected(error(errors::invalid_slotid));
        free_count += !bool(get<Vertical>(csp->size_policy));
      }

      float2 extra_per_uc{};
      float2 off = pos;
      const float cross = get<!Vertical>(size);

      if (free_count) {
        get<Vertical>(extra_per_uc) = get<Vertical>(extra) / free_count;
        for (const auto& cid : controls) {
          const auto csp = interface::slot::get<control>(cid);
          if (!csp) return std::unexpected(error(errors::invalid_slotid));
          float2 area = csp->bounds();
          if (get<Vertical>(csp->size_policy) == yw::size_policy::free)
            get<Vertical>(area) += get<Vertical>(extra_per_uc);
          get<!Vertical>(area) = cross;
          if (auto res = csp->update_geometry(off, area); !res) return res.error().relay();
          get<Vertical>(off) += get<Vertical>(area);
        }
      } else {
        get<Vertical>(extra_per_uc) = controls.empty() ? 0.0f : get<Vertical>(extra) / controls.size();
        for (const auto& cid : controls) {
          const auto csp = interface::slot::get<control>(cid);
          if (!csp) return std::unexpected(error(errors::invalid_slotid));
          float2 area = csp->bounds() + extra_per_uc;
          get<!Vertical>(area) = cross;
          if (auto res = csp->update_geometry(off, area); !res) return res.error().relay();
          get<Vertical>(off) += get<Vertical>(area);
        }
      }
      return {};
    }

    virtual std::expected<void, error> draw() const override {
      if (!visible) return {};
      if (auto res = draw_background(); !res) return res.error().relay();
      for (const auto& cid : controls) {
        const auto csp = interface::slot::get<control>(cid);
        if (!csp) return std::unexpected(error(errors::invalid_slotid));
        if (auto res = csp->draw(); !res) return res.error().relay();
      }
      if (auto res = draw_foreground(); !res) return res.error().relay();
      return {};
    }

    virtual slotid hittest(float2 Pt) const override {
      if (!visible) return {};
      for (const auto& cid : controls) {
        if (const auto csp = interface::slot::get<control>(cid))
          if (const auto hit = csp->hittest(Pt)) return hit;
      }
      return frame::slot::hittest(Pt);
    }

    virtual slotid find_next_tabstop(slotid Focused, bool Forward, bool& Found) const override {
      if (!Forward) {
        for (auto it = controls.rbegin(); it != controls.rend(); ++it) {
          if (const auto csp = interface::slot::get<control>(*it))
            if (const auto next = csp->find_next_tabstop(Focused, Forward, Found)) return next;
        }
        return {};
      }
      for (const auto& cid : controls) {
        if (const auto csp = interface::slot::get<control>(cid))
          if (const auto next = csp->find_next_tabstop(Focused, Forward, Found)) return next;
      }
      return {};
    }
  };

  using control::operator bool;
  layout() noexcept = default;

  template<typename Layout> static std::expected<layout, error> add(Layout& Layout_) {
    const auto lsp = interface::slot::slots.get(Layout_.id());
    if (!lsp) return std::unexpected(error(errors::invalid_slotid));
    if (!lsp->attachable()) return std::unexpected(error(errors::invalid_operation, "Layout is not attachable"));
    const auto id = interface::slot::add<layout>();
    const auto csp = interface::slot::get<layout>(id);
    if (!csp) return std::unexpected(error(errors::invalid_slotid));
    csp->id = id;
    csp->layout_id = Layout_.id();
    if (const auto parent_control = dynamic_cast<control::slot*>(lsp)) csp->window_id = parent_control->window_id;
    else if (const auto parent_window = dynamic_cast<window::handle<window::type::unknown>::slot*>(lsp))
      csp->window_id = parent_window->id;
    else {
      interface::slot::slots.erase(id);
      return std::unexpected(error(errors::invalid_operation, "Unsupported layout parent"));
    }
    if (auto res = lsp->attach(id); !res) {
      interface::slot::slots.erase(id);
      return res.error().relay();
    }
    layout lyt;
    lyt._id = id;
    return lyt;
  }

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
