#pragma once
#include "ywx/ui_plain.h"

namespace yw::ui {

class layout : public plain {
public:
  class slot : public plain::slot {
  protected:
    template<bool V> void draw_layout() const {
      uint32_t ucc_count = 0;
      float min_primary = 0.0f;
      for (const auto& cid : controls)
        if (const auto csp = system::slot_address<control>(cid)) {
          const auto [demand, _] = csp->demand_survey();
          ucc_count += yw::get<V>(csp->ucc);
          min_primary += yw::get<V>(demand);
        }
      const auto extra_per_ucc = ucc_count ? (yw::get<V>(size) - min_primary) / float(ucc_count) : 0.0f;
      float offset = 0.0f;
      for (const auto& cid : controls)
        if (const auto csp = system::slot_address<control>(cid)) {
          const auto [demand, ucc] = csp->demand_survey();
          if constexpr (V) {
            const auto control_size = float2(size.x, demand.y + ucc.y * extra_per_ucc);
            if (csp->visible) csp->draw({pos.x, pos.y + offset}, control_size);
            offset += control_size.y;
          } else {
            const auto control_size = float2(demand.x + ucc.x * extra_per_ucc, size.y);
            if (csp->visible) csp->draw({pos.x + offset, pos.y}, control_size);
            offset += control_size.x;
          }
        }
    }

    template<bool V> tuple<float2, uint2> demand_survey() const noexcept {
      tuple<float2, uint2> result;
      yw::get<!V>(result.second) = yw::get<!V>(ucc);
      for (const auto& cid : controls)
        if (const auto csp = system::slot_address<control>(cid)) {
          const auto [ms, ucc_] = csp->demand_survey();
          yw::get<!V>(result.first) = yw::max(yw::get<!V>(result.first), yw::get<!V>(ms));
          yw::get<V>(result.first) += yw::max(yw::get<V>(ms), 0.0f);
          yw::get<!V>(result.second) &= bool(yw::get<!V>(ucc_));
          yw::get<V>(result.second) += bool(yw::get<V>(ucc_));
        }
      result.first.x = yw::max(result.first.x, size.x, 0.0f) + margin.x + margin.z;
      result.first.y = yw::max(result.first.y, size.y, 0.0f) + margin.y + margin.w;
      yw::get<V>(result.second) *= bool(yw::get<V>(ucc)); // レイアウトの縦幅が非拘束の場合のみ有効
      return result;
    }

  public:
    std::vector<slotid> controls{};

    virtual ~slot() noexcept {
      make_messy();
      dying = true;
      for (auto cid : controls) system::uis.erase(cid);
    }

    virtual tuple<float2, uint2> demand_survey() const noexcept override { return demand_survey<true>(); }

    virtual bool attach(ui::slotid ChildId) override {
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
      if (Pt.x < pos.x || Pt.y < pos.y || Pt.x > pos.x + size.x || Pt.y > pos.y + size.y) return {};
      for (const auto& cid : controls)
        if (const auto csp = system::slot_address<control>(cid))
          if (const auto hit = csp->hit_test(Pt)) return csp->visible ? hit : slotid{};
      return {};
    }

    virtual void draw(float2 Pos, float2 Size) override { //
      pos = Pos + margin.xy();
      if (ucc.x) size.x = Size.x - margin.x - margin.z;
      if (ucc.y) size.y = Size.y - margin.y - margin.w;
      draw_layout<true>();
    }

    virtual void draw() const override {
      draw_plain(pos, size);
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

  using plain::operator bool;
  layout() noexcept = default;
  layout(derived_from<unknown> auto& Layout) {
    if (auto res = create_control<layout>(Layout)) _id = *res;
  }
};

using vertical_layout = layout;

//////////////////////////////////////// MARK: horizontal_layout

class horizontal_layout : public layout {
public:
  class slot : public layout::slot {
  public:
    virtual tuple<float2, uint2> demand_survey() const noexcept override {
      return layout::slot::demand_survey<false>();
    }

    virtual void draw(float2 Pos, float2 Size) override {
      pos = Pos + margin.xy();
      if (ucc.x) size.x = Size.x - margin.x - margin.z;
      if (ucc.y) size.y = Size.y - margin.y - margin.w;
      draw_layout<false>();
    }
  };

  using layout::operator bool;
  horizontal_layout() noexcept = default;
  horizontal_layout(derived_from<unknown> auto& Layout) {
    if (auto res = create_control<horizontal_layout>(Layout)) _id = *res;
  }
};

//////////////////////////////////////// MARK: grid_layout

/// \note
/// `grid_layout`は、ラジオボタンのように盾にも横にも整列させたいケースのために用意する。
/// そのため、列数は定数であり、複数を跨ぐような配置はサポートしない。

// template<size_t Columns> class grid_layout : public plain {
// public:
//   class slot : public plain::slot {
//     void draw_layout(float2 Pos, float2 Size) const;

//   protected:
//     template<bool V> void draw_layout(float2 Pos, float2 Size) const {
//       float min_primary = 0.0f;
//       uint32_t ucc = 0;
//       for (const auto& cid : controls)
//         if (const auto csp = system::slot_address<control>(cid)) {
//           const auto [ms, uc] = csp->require_size();
//           min_primary += yw::max(yw::get<V>(ms), 0.0f);
//           ucc += yw::get<V>(uc);
//         }
//       float offset = 0.0f;
//       if (ucc == 0) {
//         for (const auto& cid : controls)
//           if (const auto csp = system::slot_address<control>(cid)) {
//             const auto [ms, _] = csp->require_size();
//             if constexpr (V) {
//               const auto control_size = float2(Size.x, ms.y);
//               if (csp->visible) csp->draw({Pos.x, Pos.y + offset}, control_size);
//               offset += yw::max(control_size.y, 0.0f);
//             } else {
//               const auto control_size = float2(ms.x, Size.y);
//               if (csp->visible) csp->draw({Pos.x + offset, Pos.y}, control_size);
//               offset += yw::max(control_size.x, 0.0f);
//             }
//           }
//       } else {
//         const auto extra_per_ucc = yw::max((yw::get<V>(Size) - min_primary) / float(ucc), 0.0f);
//         for (const auto& cid : controls)
//           if (const auto csp = system::slot_address<control>(cid)) {
//             const auto [ms, ucc] = csp->require_size();
//             if constexpr (V) {
//               const auto control_size = float2(Size.x, ms.y + ucc.y * extra_per_ucc);
//               if (csp->visible) csp->draw({Pos.x, Pos.y + offset}, control_size);
//               offset += control_size.y;
//             } else {
//               const auto control_size = float2(ms.x + ucc.x * extra_per_ucc, Size.y);
//               if (csp->visible) csp->draw({Pos.x + offset, Pos.y}, control_size);
//               offset += control_size.x;
//             }
//           }
//       }
//     }

//   public:
//     std::vector<std::array<slotid, Columns>> rows{};
//     size_t attached_count = 0;

//     virtual ~slot() noexcept {
//       make_messy();
//       dying = true;
//       for (auto& row : controls)
//         for (auto cid : row) system::uis.erase(cid);
//     }

//     virtual tuple<float2, uint2> require_size() const noexcept override {
//       tuple<float2, uint2> result{};
//       std::array<tuple<float, bool>, Columns> r{};
//       for (const auto& row : rows) {
//         float h = 0.0f;
//         bool uc = false;
//         for (size_t i = 0; i < Columns; ++i)
//           if (const auto csp = system::slot_address<control>(row[i])) {
//             const auto [ms, uc] = csp->require_size();
//             r[i].first = yw::max(r[i].first, ms.x);
//             r[i].second &= bool(uc.x);
//             h = yw::max(h, ms.y);
//             uc &= bool(uc.y);
//           }
//         result.first.y += h;
//         result.second.y += uc; // ucな行の数
//       }
//       for (size_t i = 0; i < Columns; ++i) {
//         result.first.x += r[i].first;
//         result.second.x += r[i].second; // ucな列の数
//       }
//       return result;
//     }

//     virtual bool attach(ui::slotid ChildId) override {
//       const auto row = count / Columns;
//       const auto col = count % Columns;
//       if (row >= rows.size()) rows.resize(row + 1);
//       if (const auto csp = system::slot_address<control>(ChildId)) {
//         ++attached_count;
//         rows[row][col] = ChildId;
//         csp->id = ChildId;
//         csp->layout_id = id;
//         csp->window_id = window_id;
//         make_messy();
//       }
//       return true;
//     }

//     virtual void detach(ui::slotid ChildId) override {
//       for (auto& row : rows)
//         for (auto& cid : row)
//           if (cid == ChildId) cid = {};
//       --attached_count;
//       make_messy();
//     }

//     virtual slotid hit_test(float2 Pt) const noexcept override {
//       if (!visible) return {};
//       if (Pt.x < last_rect.x || Pt.y < last_rect.y || Pt.x > last_rect.z || Pt.y > last_rect.w) return {};
//       for (const auto& row : rows)
//         for (const auto& cid : row)
//           if (const auto csp = system::slot_address<control>(cid); csp && csp->visible)
//             if (const auto hit = csp->hit_test(Pt)) return hit;
//       return {};
//     }

//     virtual void draw() const override {
//       const auto lr_pos = last_rect.xy();
//       const auto lr_size = last_rect.zw() - lr_pos;
//       draw_plain(lr_pos, lr_size);
//       for ()
//       // for (const auto& cid : controls)
//       //   if (const auto csp = system::slot_address<control>(cid); csp && csp->visible) csp->draw();
//     }

//     virtual slotid next_tab_stop(slotid Current, bool Forward, bool& Found) override {
//       if (Forward) {
//         for (const auto cid : controls)
//           if (const auto csp = system::slot_address<control>(cid))
//             if (const auto hit = csp->next_tab_stop(Current, Forward, Found)) return hit;
//       } else {
//         for (const auto cid : controls | std::views::reverse)
//           if (const auto csp = system::slot_address<control>(cid))
//             if (const auto hit = csp->next_tab_stop(Current, Forward, Found)) return hit;
//       }
//       return {};
//     }
//   };

//   grid_layout() noexcept = default;

//   grid_layout(derived_from<unknown> auto& Layout) {
//     if (auto res = create_control<grid_layout>(Layout)) _id = *res;
//   }

//   using plain::operator bool;
// };
} // namespace yw::ui
