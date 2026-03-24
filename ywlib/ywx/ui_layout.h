#pragma once
#include "ywx/ui_plain.h"

namespace yw::ui {

class layout : public plain {
public:
  class slot : public plain::slot {
  protected:
    template<bool V> void draw_layout(float2 Pos, float2 Size) const {
      float min_primary = 0.0f;
      uint32_t ucc = 0;
      for (const auto& cid : controls)
        if (const auto csp = system::slot_address<control>(cid)) {
          const auto [ms, uc] = csp->require_size();
          min_primary += yw::max(yw::get<V>(ms), 0.0f);
          ucc += yw::get<V>(uc);
        }
      float offset = 0.0f;
      if (ucc == 0) {
        for (const auto& cid : controls)
          if (const auto csp = system::slot_address<control>(cid)) {
            const auto [ms, _] = csp->require_size();
            if constexpr (V) {
              const auto control_size = float2(Size.x, ms.y);
              if (csp->visible) csp->draw({Pos.x, Pos.y + offset}, control_size);
              offset += yw::max(control_size.y, 0.0f);
            } else {
              const auto control_size = float2(ms.x, Size.y);
              if (csp->visible) csp->draw({Pos.x + offset, Pos.y}, control_size);
              offset += yw::max(control_size.x, 0.0f);
            }
          }
      } else {
        const auto extra_per_ucc = yw::max((yw::get<V>(Size) - min_primary) / float(ucc), 0.0f);
        for (const auto& cid : controls)
          if (const auto csp = system::slot_address<control>(cid)) {
            const auto [ms, ucc] = csp->require_size();
            if constexpr (V) {
              const auto control_size = float2(Size.x, ms.y + ucc.y * extra_per_ucc);
              if (csp->visible) csp->draw({Pos.x, Pos.y + offset}, control_size);
              offset += control_size.y;
            } else {
              const auto control_size = float2(ms.x + ucc.x * extra_per_ucc, Size.y);
              if (csp->visible) csp->draw({Pos.x + offset, Pos.y}, control_size);
              offset += control_size.x;
            }
          }
      }
    }

    template<bool V> tuple<float2, uint2> require_size() const noexcept {
      tuple<float2, uint2> result{};
      for (const auto& cid : controls)
        if (const auto csp = system::slot_address<control>(cid)) {
          const auto [ms, uc] = csp->require_size();
          yw::get<!V>(result.first) = yw::max(yw::get<!V>(result.first), yw::get<!V>(ms));
          yw::get<V>(result.first) += yw::max(yw::get<V>(ms), 0.0f);
          yw::get<!V>(result.second) |= bool(yw::get<!V>(uc));
          yw::get<V>(result.second) += yw::get<V>(uc);
        }
      result.first.x = yw::max(result.first.x, size.x, 0.0f) + margin.x + margin.z;
      result.first.y = yw::max(result.first.y, size.y, 0.0f) + margin.y + margin.w;
      yw::get<!V>(result.second) &= yw::get<!V>(size) < 0.0f;
      if (yw::get<V>(size) >= 0.0f) yw::get<V>(result.second) = 0;
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

    virtual bool attach(ui::slotid ChildId) override {
      // 無効なIDを渡すことでアタッチ可能か確認できる
      if (const auto csp = system::slot_address<control>(ChildId)) {
        controls.push_back(ChildId);
        csp->id = ChildId;
        csp->layout_id = id;
        csp->window_id = window_id;
        make_messy();
      }
      return true;
    }

    virtual void detach(ui::slotid ChildId) override {
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

    virtual slotid next_tab_stop(slotid Current, bool Forward, bool& Found) override {
      if (Forward) {
        for (const auto cid : controls)
          if (const auto csp = system::slot_address<control>(cid))
            if (const auto hit = csp->next_tab_stop(Current, Forward, Found)) return hit;
      } else {
        for (const auto cid : controls | std::views::reverse)
          if (const auto csp = system::slot_address<control>(cid))
            if (const auto hit = csp->next_tab_stop(Current, Forward, Found)) return hit;
      }
      return {};
    }
  };

  layout() noexcept = default;

  layout(derived_from<unknown> auto& Layout) {
    if (auto res = create_control<layout>(Layout)) _id = *res;
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

  horizontal_layout(derived_from<unknown> auto& Layout) {
    if (auto res = create_control<horizontal_layout>(Layout)) _id = *res;
  }

  using layout::operator bool;
};

// inline plain::plain(layout& Layout) : plain(static_cast<unknown&>(Layout)) {}
} // namespace yw::ui
