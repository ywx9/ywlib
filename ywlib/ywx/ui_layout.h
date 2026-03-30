#pragma once
#include "ywx/ui_control.h"

namespace yw::ui {

/// vertical layout
class layout : public control {
public:
  class slot : public control::slot {
  protected:
    template<bool V> void update_size() noexcept {
      min_size = vapply_r<float2>(yw::max, min_size, float2(0.0f, 0.0f));
      float2 inner{};
      for (const auto& cid : controls)
        if (const auto csp = system::slot_address<control>(cid)) {
          csp->update_size();
          const auto area = csp->size + csp->margin.xy() + csp->margin.zw();
          get<!V>(inner) = yw::max(yw::get<!V>(inner), yw::get<!V>(area));
          get<V>(inner) += get<V>(area);
        }
      size = vapply_r<float2>(yw::max, min_size, inner, size * constrained);
    }

    template<bool V> void draw(float2 Pos, float2 Area, bool Visible) {
      pos = Pos + margin.xy();
      const auto size_ = Area - margin.xy() - margin.zw();
      size = (float2(1.0f, 1.0f) - constrained) * size_ + constrained * size;
      align(size_ - size);
      unsigned uc_count{};
      float min_primary{};
      for (const auto& cid : controls)
        if (const auto csp = system::slot_address<control>(cid)) {
          csp->update_size();
          uc_count += !get<V>(csp->constrained);
          min_primary += get<V>(csp->size) + get<V>(csp->margin) + get<2 + V>(csp->margin);
        }
      if (Visible && visible) {
        draw_background(pos, size, background);
        brush.color(border_color);
        draw_round_rectangle(pos, size, radius, border_width);
      }
      float2 offset{};
      float2 extra_per_uc{};
      if (uc_count) get<V>(extra_per_uc) = (get<V>(size) - min_primary) / uc_count;
      for (const auto& cid : controls)
        if (const auto csp = system::slot_address<control>(cid)) {
          auto sz =
            csp->size + csp->margin.xy() + csp->margin.zw() + extra_per_uc * (float2(1.0f, 1.0f) - csp->constrained);
          get<!V>(sz) = get<!V>(size);
          csp->draw(pos + offset, sz, Visible && visible);
          get<V>(offset) += get<V>(sz);
        }
    }

  public:
    yw::background background = colors::transparent;
    color border_color = colors::transparent;
    float border_width = 1.0f;
    float4 padding = float4::fill(5.0f);

    std::vector<slotid> controls{};

    virtual ~slot() noexcept override {
      try {
        make_messy();
        dying = true;
        for (auto cid : controls) system::uis.erase(cid);
      } catch (...) {}
    }

    virtual bool attach(ui::slotid cid) override {
      if (const auto csp = system::slot_address<control>(cid)) {
        controls.push_back(cid);
        csp->id = cid;
        csp->layout_id = id;
        csp->window_id = window_id;
        make_messy();
      }
      return true;
    }

    virtual void detach(ui::slotid cid) override {
      std::erase(controls, cid);
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

    virtual void update_size() noexcept override { update_size<true>(); }
    virtual void draw(float2 Pos, float2 Area, bool Visible) override { draw<true>(Pos, Area, Visible); }

    virtual void draw() const override {
      if (!visible) return;
      draw_background(pos, size, background);
      brush.color(border_color);
      draw_round_rectangle(pos, size, radius, border_width);
      for (const auto& cid : controls)
        if (const auto csp = system::slot_address<control>(cid)) csp->draw();
    }
  };

  using control::operator bool;
  layout() noexcept = default;
  layout(derived_from<unknown> auto& Layout) {
    if (auto res = create_control<layout>(Layout)) _id = *res;
  }

  const auto& background() const { return unsafe_get(&slot::background); }
  void background(yw::background bg) { safe_set(&slot::background, std::move(bg)); }

  const auto& border_color() const { return unsafe_get(&slot::border_color); }
  void border_color(const color& c) { safe_set(&slot::border_color, c); }

  const auto& border_width() const { return unsafe_get(&slot::border_width); }
  void border_width(float1 w) { safe_set(&slot::border_width, w.x); }

  const auto& padding() const { return unsafe_get(&slot::padding); }
  void padding(const float4& p) { safe_set_size(&slot::padding, p); }
};

using vertical_layout = layout;

class horizontal_layout : public layout {
public:
  class slot : public layout::slot {
  public:
    virtual void update_size() noexcept override { layout::slot::update_size<false>(); }
    virtual void draw(float2 Pos, float2 Area, bool Visible) override { layout::slot::draw<false>(Pos, Area, Visible); }
  };

  using layout::operator bool;
  horizontal_layout() noexcept = default;
  horizontal_layout(derived_from<unknown> auto& Layout) {
    if (auto res = create_control<horizontal_layout>(Layout)) _id = *res;
  }
};

class grid_layout : public control {};
} // namespace yw::ui

// //////////////////////////////////////// MARK: grid_layout

// /// \note
// /// `grid_layout`は、ラジオボタンのように盾にも横にも整列させたいケースのために用意する。
// /// そのため、列数は定数であり、複数を跨ぐような配置はサポートしない。

// template<size_t Columns> class grid_layout : public plain {
// public:
//   class slot : public plain::slot {
//   protected:
//     void draw_layout() const {
//       std::array<float, Columns> col_min{};
//       std::array<bool, Columns> col_ucc{};
//       std::vector<float> row_min(rows.size());
//       std::vector<bool> row_ucc(rows.size());

//       for (size_t r = 0; r < rows.size(); ++r)
//         for (size_t c = 0; c < Columns; ++c)
//           if (const auto csp = system::slot_address<control>(rows[r][c])) {
//             const auto min_size = csp->demand_survey();
//             col_min[c] = yw::max(col_min[c], min_size.x);
//             row_min[r] = yw::max(row_min[r], min_size.y);
//             col_ucc[c] = col_ucc[c] || bool(csp->ucc.x);
//             row_ucc[r] = row_ucc[r] || bool(csp->ucc.y);
//           }

//       float min_width = 0.0f;
//       float min_height = 0.0f;
//       uint32_t ucc_cols = 0;
//       uint32_t ucc_rows = 0;
//       for (size_t c = 0; c < Columns; ++c) {
//         min_width += col_min[c];
//         ucc_cols += uint32_t(col_ucc[c]);
//       }
//       for (size_t r = 0; r < rows.size(); ++r) {
//         min_height += row_min[r];
//         ucc_rows += uint32_t(row_ucc[r]);
//       }

//       const float extra_per_col = ucc_cols ? yw::max(size.x - min_width, 0.0f) / ucc_cols : 0.0f;
//       const float extra_per_row = ucc_rows ? yw::max(size.y - min_height, 0.0f) / ucc_rows : 0.0f;

//       std::array<float, Columns> col_size{};
//       std::vector<float> row_size(rows.size());
//       for (size_t c = 0; c < Columns; ++c) col_size[c] = col_min[c] + (col_ucc[c] ? extra_per_col : 0.0f);
//       for (size_t r = 0; r < rows.size(); ++r) row_size[r] = row_min[r] + (row_ucc[r] ? extra_per_row : 0.0f);

//       float yoff = 0.0f;
//       for (size_t r = 0; r < rows.size(); ++r) {
//         float xoff = 0.0f;
//         for (size_t c = 0; c < Columns; ++c)
//           if (const auto csp = system::slot_address<control>(rows[r][c])) {
//             const auto control_size = float2(col_size[c], row_size[r]);
//             if (csp->visible) csp->draw(pos + float2(xoff, yoff), control_size);
//             xoff += col_size[c];
//           } else xoff += col_size[c];
//         yoff += row_size[r];
//       }
//     }

//   public:
//     slot() noexcept { ucc = {true, true}; }

//     std::vector<std::array<slotid, Columns>> rows{};
//     size_t attached_count = 0;

//     virtual ~slot() noexcept {
//       make_messy();
//       dying = true;
//       for (auto& row : rows)
//         for (auto cid : row) system::uis.erase(cid);
//     }

//     virtual float2 demand_survey() const noexcept override {
//       std::array<float, Columns> col_min{};
//       std::vector<float> row_min(rows.size());
//       for (size_t r = 0; r < rows.size(); ++r)
//         for (size_t c = 0; c < Columns; ++c)
//           if (const auto csp = system::slot_address<control>(rows[r][c])) {
//             const auto min_size = csp->demand_survey();
//             col_min[c] = yw::max(col_min[c], min_size.x);
//             row_min[r] = yw::max(row_min[r], min_size.y);
//           }

//       float2 result{};
//       for (size_t c = 0; c < Columns; ++c) result.x += col_min[c];
//       for (size_t r = 0; r < rows.size(); ++r) result.y += row_min[r];
//       if (!ucc.x) result.x = yw::max(result.x, size.x);
//       if (!ucc.y) result.y = yw::max(result.y, size.y);
//       return result + margin.xy() + margin.zw();
//     }

//     virtual bool attach(ui::slotid ChildId) override {
//       const auto row = attached_count / Columns;
//       const auto col = attached_count % Columns;
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
//       if (attached_count) --attached_count;
//       make_messy();
//     }

//     virtual slotid hit_test(float2 Pt) const noexcept override {
//       if (!visible) return {};
//       if (Pt.x < pos.x || Pt.y < pos.y || Pt.x > pos.x + size.x || Pt.y > pos.y + size.y) return {};
//       for (const auto& row : rows)
//         for (const auto& cid : row)
//           if (const auto csp = system::slot_address<control>(cid); csp && csp->visible)
//             if (const auto hit = csp->hit_test(Pt)) return hit;
//       return {};
//     }

//     virtual void draw(float2 Pos, float2 Size) override {
//       Pos += margin.xy();
//       Size -= margin.xy() + margin.zw();
//       if (ucc.x) size.x = Size.x;
//       if (ucc.y) size.y = Size.y;
//       auto extra = Size - size;
//       pos = Pos;
//       switch (alignment) {
//       case ui::alignment::center: pos += extra * 0.5f; break;
//       case ui::alignment::left: break;
//       case ui::alignment::right: pos.x += extra.x; break;
//       case ui::alignment::top: break;
//       case ui::alignment::bottom: pos.y += extra.y; break;
//       case ui::alignment::left_top: break;
//       case ui::alignment::left_bottom: pos.y += extra.y; break;
//       case ui::alignment::right_top: pos.x += extra.x; break;
//       case ui::alignment::right_bottom: pos += extra; break;
//       }
//       draw_layout();
//     }

//     virtual void draw() const override {
//       draw_plain(pos, size);
//       for (const auto& row : rows)
//         for (const auto cid : row)
//           if (const auto csp = system::slot_address<control>(cid); csp && csp->visible) csp->draw();
//     }

//     virtual slotid next_tab_stop(slotid Current, bool Forward, bool& Found) override {
//       if (Forward) {
//         for (const auto& row : rows)
//           for (const auto cid : row)
//             if (const auto csp = system::slot_address<control>(cid))
//               if (const auto hit = csp->next_tab_stop(Current, Forward, Found)) return hit;
//       } else {
//         for (const auto& row : rows | std::views::reverse)
//           for (const auto cid : row | std::views::reverse)
//             if (const auto csp = system::slot_address<control>(cid))
//               if (const auto hit = csp->next_tab_stop(Current, Forward, Found)) return hit;
//       }
//       return {};
//     }
//   };

//   grid_layout() noexcept = default;

//   grid_layout(derived_from<unknown> auto& Layout) {
//     if (auto res = create_control<grid_layout<Columns>>(Layout)) _id = *res;
//   }

//   using plain::operator bool;
// };
// } // namespace yw::ui
