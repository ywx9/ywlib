#pragma once
#include "ywx/ui_plain.h"

namespace yw::ui {

class layout : public plain {
public:
  class slot : public plain::slot {
  protected:
    template<bool Vertical> void draw_layout(float2 Pos, float2 Size) const {
      float min_primary = 0.0f;
      uint32_t ucc = 0;
      for (const auto& cid : controls)
        if (const auto csp = system::slot_address<control>(cid); csp && csp->visible) {
          const auto [ms, uc] = csp->require_size();
          min_primary += yw::max(yw::get<Vertical>(ms), 0.0f);
          ucc += yw::get<Vertical>(uc);
        }
      float offset = 0.0f;
      if (ucc == 0) {
        for (const auto& cid : controls)
          if (const auto csp = system::slot_address<control>(cid); csp && csp->visible) {
            const auto [ms, _] = csp->require_size();
            if constexpr (Vertical) {
              const auto control_size = float2(Size.x, ms.y);
              csp->draw({Pos.x, Pos.y + offset}, control_size);
              offset += yw::max(control_size.y, 0.0f);
            } else {
              const auto control_size = float2(ms.x, Size.y);
              csp->draw({Pos.x + offset, Pos.y}, control_size);
              offset += yw::max(control_size.x, 0.0f);
            }
          }
      } else {
        const auto extra_per_ucc = yw::max((yw::get<Vertical>(Size) - min_primary) / float(ucc), 0.0f);
        for (const auto& cid : controls)
          if (const auto csp = system::slot_address<control>(cid); csp && csp->visible) {
            const auto [ms, ucc] = csp->require_size();
            if constexpr (Vertical) {
              const auto control_size = float2(Size.x, ms.y + ucc.y * extra_per_ucc);
              csp->draw({Pos.x, Pos.y + offset}, control_size);
              offset += control_size.y;
            } else {
              const auto control_size = float2(ms.x + ucc.x * extra_per_ucc, Size.y);
              csp->draw({Pos.x + offset, Pos.y}, control_size);
              offset += control_size.x;
            }
          }
      }
    }

    template<bool Vertical> tuple<float2, uint2> require_size() const noexcept {
      tuple<float2, uint2> result{};
      for (const auto& cid : controls)
        if (const auto csp = system::slot_address<control>(cid)) {
          const auto [ms, uc] = csp->require_size();
          yw::get<!Vertical>(result.first) = yw::max(yw::get<!Vertical>(result.first), yw::get<!Vertical>(ms));
          yw::get<Vertical>(result.first) += yw::max(yw::get<Vertical>(ms), 0.0f);
          yw::get<!Vertical>(result.second) |= bool(yw::get<!Vertical>(uc));
          yw::get<Vertical>(result.second) += yw::get<Vertical>(uc);
        }
      result.first.x = yw::max(result.first.x, size.x) + margin.x + margin.z;
      result.first.y = yw::max(result.first.y, size.y) + margin.y + margin.w;
      if (size.x >= 0.0f) result.second.x = 0;
      if (size.y >= 0.0f) result.second.y = 0;
      return result;
    }

  public:
    std::vector<slotid> controls{};

    virtual ~slot() noexcept {
      make_messy();
      dying = true;
      for (auto cid : controls) system::uis.erase(cid);
    }

    virtual tuple<float2, uint2> require_size() const noexcept override { return require_size<true>(); }

    virtual bool attach(const ui::slotid& ChildId) override {
      controls.push_back(ChildId);
      make_messy();
      return true;
    }

    virtual void detach(const ui::slotid& ChildId) override {
      std::erase(controls, ChildId);
      make_messy();
    }

    virtual slotid hit_test(float2 Pt) const noexcept override {
      if (!visible) return {};
      if (Pt.x < last_rect.x || Pt.y < last_rect.y || Pt.x > last_rect.z || Pt.y > last_rect.w) return {};
      for (const auto& cid : controls | std::views::reverse)
        if (const auto csp = system::slot_address<control>(cid); csp && csp->visible)
          if (const auto hit = csp->hit_test(Pt)) return hit;
      return {};
    }

    virtual void draw(float2 Pos, float2 Size) const override {
      update_last_rect(Pos, Size);
      const auto sz = last_rect.zw() - last_rect.xy();
      draw_plain(last_rect.xy(), sz);
      draw_layout<true>(last_rect.xy(), sz);
    }

    virtual void draw() const override {
      draw_plain(last_rect.xy(), last_rect.zw() - last_rect.xy());
      for (const auto& cid : controls)
        if (const auto csp = system::slot_address<control>(cid); csp && csp->visible) csp->draw();
    }
  };

  layout() noexcept = default;

  layout(unknown& Layout) {
    const auto cid = system::uis.add(std::make_unique<slot>());
    const auto csp = system::slot_address<layout>(cid);
    if (!csp) throw unexpected_error(errors::operation_failed, "Failed to create layout slot");
    const auto lid = Layout.id();
    const auto lsp = system::uis.get(lid);
    if (!lsp) throw unexpected_error(errors::operation_failed, "Failed to get parent layout slot");
    if (!lsp->attach(cid)) {
      system::uis.erase(cid);
      throw unexpected_error(errors::operation_failed, "Failed to attach layout slot to parent");
    } else _id = cid;
  }

  using plain::operator bool;
};

using vertical_layout = layout;

class horizontal_layout : public layout {
public:
  class slot : public layout::slot {
  public:
    virtual tuple<float2, uint2> require_size() const noexcept override { return layout::slot::require_size<false>(); }

    virtual void draw(float2 Pos, float2 Size) const override {
      update_last_rect(Pos, Size);
      const auto sz = last_rect.zw() - last_rect.xy();
      draw_plain(last_rect.xy(), sz);
      draw_layout<false>(last_rect.xy(), sz);
    }
  };

  horizontal_layout() noexcept = default;

  horizontal_layout(unknown& Layout) {
    const auto cid = system::uis.add(std::make_unique<slot>());
    const auto csp = system::slot_address<horizontal_layout>(cid);
    if (!csp) throw unexpected_error(errors::operation_failed, "Failed to create horizontal layout slot");
    const auto lid = Layout.id();
    const auto lsp = system::uis.get(lid);
    if (!lsp) throw unexpected_error(errors::operation_failed, "Failed to get parent layout slot");
    if (!lsp->attach(cid)) {
      system::uis.erase(cid);
      throw unexpected_error(errors::operation_failed, "Failed to attach horizontal layout slot to parent");
    } else _id = cid;
  }

  using layout::operator bool;
};

// inline plain::plain(layout& Layout) : plain(static_cast<unknown&>(Layout)) {}
} // namespace yw::ui
