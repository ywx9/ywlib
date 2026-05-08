#pragma once
#include "ywx/ui_control.h"

namespace yw::ui {

template<bool Vertical>
class layout : public control {
public:
  struct slot : public control::slot {
    part::background background{};
    part::background border{};
    std::vector<slotid> controls{};
    float4 padding = float4::fill(arbitrary_value);

    std::expected<void, error_trace> initialize() {
      background.window_id = core.window_id;
      border.window_id = core.window_id;
      return {};
    }

    virtual void ensure_minimum_size() override {
      float2 internal;
      for (const auto& cid : controls)
        if (const auto csp = system::slot_address<control>(cid)) {
          csp->ensure_minimum_size();
          get<!Vertical>(internal) = yw::max(get<!Vertical>(internal), get<!Vertical>(csp->core.size));
          get<Vertical>(internal) += get<Vertical>(csp->core.size + csp->core.margin.xy() + csp->core.margin.zw());
        }
      size = vapply_r<float2>(yw::max, core.min_size, internal, core.required_size * core.constrained);
    }

    virtual void update_layout(float2 Pos, float2 Size) override {
      const auto minimum_sz = core.size;
      core.update_layout(Pos, Size);
      const auto extra = core.size - minimum_sz;
      unsigned uc_count = 0;
      for (const auto& cid : controls)
        if (const auto csp = system::slot_address<control>(cid))
          uc_count += !yw::get<Vertical>(csp->core.constrained);
      const float extra_per_uc = uc_count ? get<Vertical>(extra) / uc_count : 0.0f;

    }
  };
};
}

// /// vertical layout
// class layout : public control {
// public:
//   class slot : public control::slot {
//   protected:
//     template<bool V> float2 calculate_size() const noexcept {
//       float2 inner = padding.xy() + padding.zw();
//       for (const auto& cid : controls)
//         if (const auto csp = system::slot_address<control>(cid)) {
//           const auto area = csp->calculate_size() + csp->margin.xy() + csp->margin.zw();
//           get<!V>(inner) = yw::max(yw::get<!V>(inner), yw::get<!V>(area));
//           get<V>(inner) += get<V>(area);
//         }
//       return vapply_r<float2>(yw::max, float2(), min_size, inner, size * constrained);
//     }

//     template<bool V> void update_size() noexcept {
//       min_size = vapply_r<float2>(yw::max, min_size, float2(0.0f, 0.0f));
//       float2 inner{};
//       for (const auto& cid : controls)
//         if (const auto csp = system::slot_address<control>(cid)) {
//           csp->update_size();
//           const auto area = csp->size + csp->margin.xy() + csp->margin.zw();
//           get<!V>(inner) = yw::max(yw::get<!V>(inner), yw::get<!V>(area));
//           get<V>(inner) += get<V>(area);
//         }
//       size = vapply_r<float2>(yw::max, min_size, inner, size * constrained);
//       update_geometry();
//     }

//     template<bool V> void update_layout(float2 Pos, float2 Area) {
//       pos = Pos + margin.xy();
//       const auto size_ = Area - margin.xy() - margin.zw();
//       size = (float2(1.0f, 1.0f) - constrained) * size_ + constrained * size;
//       align(size_ - size);
//       unsigned uc_count{};
//       float min_primary{};
//       for (const auto& cid : controls)
//         if (const auto csp = system::slot_address<control>(cid)) {
//           csp->update_size();
//           uc_count += !get<V>(csp->constrained);
//           min_primary += get<V>(csp->size) + get<V>(csp->margin) + get<2 + V>(csp->margin);
//         }
//       float2 offset{};
//       float2 extra_per_uc{};
//       if (uc_count) get<V>(extra_per_uc) = (get<V>(size) - min_primary) / uc_count;
//       for (const auto& cid : controls)
//         if (const auto csp = system::slot_address<control>(cid)) {
//           auto sz =
//             csp->size + csp->margin.xy() + csp->margin.zw() + extra_per_uc * (float2(1.0f, 1.0f) - csp->constrained);
//           get<!V>(sz) = get<!V>(size);
//           csp->update_layout(pos + offset, sz);
//           get<V>(offset) += get<V>(sz);
//         }
//       update_geometry();
//     }

//   public:
//     slot() {
//       background = colors::transparent;
//       border_color = colors::transparent;
//     }
//     float4 padding = float4::fill(4.0f);

//     std::vector<slotid> controls{};

//     virtual ~slot() noexcept override {
//       try {
//         make_messy();
//         dying = true;
//         for (auto cid : controls) system::uis.erase(cid);
//       } catch (...) {}
//     }

//     virtual bool attach(ui::slotid cid) override {
//       if (const auto csp = system::slot_address<control>(cid)) {
//         controls.push_back(cid);
//         csp->id = cid;
//         csp->layout_id = id;
//         csp->window_id = window_id;
//         make_messy();
//       }
//       return true;
//     }

//     virtual void detach(ui::slotid cid) override {
//       std::erase(controls, cid);
//       make_messy();
//     }

//     virtual slotid hit_test(float2 Pt) const noexcept override {
//       if (!visible) return {};
//       if (Pt.x < pos.x || Pt.y < pos.y || Pt.x > pos.x + size.x || Pt.y > pos.y + size.y) return {};
//       for (const auto& cid : controls)
//         if (const auto csp = system::slot_address<control>(cid))
//           if (const auto hit = csp->hit_test(Pt)) return csp->visible ? hit : slotid{};
//       return {};
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

//     virtual float2 calculate_size() const noexcept override { return calculate_size<true>(); }
//     virtual void update_size() noexcept override { update_size<true>(); }
//     virtual void update_layout(float2 Pos, float2 Area) override { update_layout<true>(Pos, Area); }

//     virtual void draw() const override {
//       if (!visible) return;
//       draw_background();
//       for (const auto& cid : controls)
//         if (const auto csp = system::slot_address<control>(cid)) csp->draw();
//     }
//   };

//   using control::operator bool;
//   layout() noexcept = default;
//   layout(derived_from<unknown> auto& Layout) {
//     if (auto res = create_control<layout>(Layout)) _id = *res;
//   }

//   const auto& border_color() const { return unsafe_get(&slot::border_color); }
//   void border_color(const color& c) { safe_set(&slot::border_color, c); }

//   const auto& border_width() const { return unsafe_get(&slot::border_width); }
//   void border_width(float1 w) { safe_set(&slot::border_width, w.x); }

//   const auto& padding() const { return unsafe_get(&slot::padding); }
//   void padding(const float4& p) { safe_set_size(&slot::padding, p); }
// };

// using vertical_layout = layout;

// class horizontal_layout : public layout {
// public:
//   class slot : public layout::slot {
//   public:
//     virtual float2 calculate_size() const noexcept override { return layout::slot::calculate_size<false>(); }
//     virtual void update_size() noexcept override { layout::slot::update_size<false>(); }
//     virtual void update_layout(float2 Pos, float2 Area) override { layout::slot::update_layout<false>(Pos, Area); }
//   };

//   using layout::operator bool;
//   horizontal_layout() noexcept = default;
//   horizontal_layout(derived_from<unknown> auto& Layout) {
//     if (auto res = create_control<horizontal_layout>(Layout)) _id = *res;
//   }
// };

// //////////////////////////////////////// MARK: grid_layout

// template<size_t Columns> class grid_layout : public control {
// public:
//   class slot : public control::slot {
//   public:
//     slot() {
//       background = colors::transparent;
//       border_color = colors::transparent;
//     }
//     float4 padding = float4::fill(4.0f);

//     std::vector<vector<slotid, Columns>> rows{};
//     size_t attach_count{};

//     virtual ~slot() noexcept override {
//       try {
//         make_messy();
//         dying = true;
//         for (const auto& row : rows)
//           for (const auto& cid : row) system::uis.erase(cid);
//       } catch (...) {}
//     }

//     virtual bool attach(ui::slotid cid) override {
//       const auto row = attach_count / Columns;
//       const auto col = attach_count % Columns;
//       if (row >= rows.size()) rows.resize(row + 1);
//       if (const auto csp = system::slot_address<control>(cid)) {
//         ++attach_count;
//         rows[row][col] = cid;
//         csp->id = cid;
//         csp->layout_id = id;
//         csp->window_id = window_id;
//         make_messy();
//       }
//       return true;
//     }

//     virtual void detach(ui::slotid cid) override {
//       for (auto& row : rows)
//         for (auto& slot : row)
//           if (slot == cid) slot = {};
//       if (attach_count > 0) --attach_count;
//       make_messy();
//     }

//     virtual slotid hit_test(float2 Pt) const noexcept override {
//       if (!visible) return {};
//       if (Pt.x < pos.x || Pt.y < pos.y || Pt.x > pos.x + size.x || Pt.y > pos.y + size.y) return {};
//       for (const auto& row : rows)
//         for (const auto& cid : row)
//           if (const auto csp = system::slot_address<control>(cid))
//             if (const auto hit = csp->hit_test(Pt)) return csp->visible ? hit : slotid{};
//       return {};
//     }

//     virtual slotid next_tab_stop(slotid Current, bool Forward, bool& Found) override {
//       if (Forward) {
//         for (const auto& row : rows)
//           for (const auto& cid : row)
//             if (const auto csp = system::slot_address<control>(cid))
//               if (const auto hit = csp->next_tab_stop(Current, Forward, Found)) return hit;
//       } else {
//         for (const auto& row : rows | std::views::reverse)
//           for (const auto& cid : row | std::views::reverse)
//             if (const auto csp = system::slot_address<control>(cid))
//               if (const auto hit = csp->next_tab_stop(Current, Forward, Found)) return hit;
//       }
//       return {};
//     }

//     virtual float2 calculate_size() const noexcept override {
//       vector<float, Columns> col_width{};
//       float2 inner{};
//       for (const auto& row : rows) {
//         float temp_height{};
//         for (size_t c = 0; c < Columns; ++c)
//           if (const auto csp = system::slot_address<control>(row[c])) {
//             csp->update_size();
//             const auto area = csp->size + csp->margin.xy() + csp->margin.zw();
//             temp_height = yw::max(temp_height, area.y);
//             col_width[c] = yw::max(col_width[c], area.x);
//           }
//         inner.y += temp_height;
//       }
//       inner.x = std::accumulate(col_width.begin(), col_width.end(), 0.0f);
//       return vapply_r<float2>(yw::max, float2(), min_size, inner, size * constrained);
//     }

//     virtual void update_size() noexcept override {
//       min_size = yw::min(min_size, float2::fill(0.0f));
//       size = calculate_size();
//       update_geometry();
//     }

//     virtual void update_layout(float2 Pos, float2 Area) override {
//       pos = Pos + margin.xy();
//       const auto size_ = Area - margin.xy() - margin.zw();
//       size = (float2(1.0f, 1.0f) - constrained) * size_ + constrained * size;
//       align(size_ - size);
//       auto uc_columns = vector<unsigned, Columns>::fill(true);
//       vector<float, Columns> col_width{};
//       std::vector<unsigned> uc_rows(rows.size(), true);
//       std::vector<float> row_width(rows.size());
//       for (size_t r = 0; r < rows.size(); ++r) {
//         for (size_t c = 0; c < Columns; ++c)
//           if (const auto csp = system::slot_address<control>(rows[r][c])) {
//             const auto area = csp->size + csp->margin.xy() + csp->margin.zw();
//             uc_columns[c] &= !csp->constrained.x;
//             col_width[c] = yw::max(col_width[c], area.x);
//             uc_rows[r] &= !csp->constrained.y;
//             row_width[r] = yw::max(row_width[r], area.y);
//           }
//       }
//       const uint2 uc_count{
//         std::count(uc_columns.begin(), uc_columns.end(), true), std::count(uc_rows.begin(), uc_rows.end(), true)};
//       const float2 inner{
//         std::accumulate(col_width.begin(), col_width.end(), 0.0f),
//         std::accumulate(row_width.begin(), row_width.end(), 0.0f)};
//       float2 extra_per_uc{};
//       if (uc_count.x) extra_per_uc.x = (size.x - inner.x) / uc_count.x;
//       if (uc_count.y) extra_per_uc.y = (size.y - inner.y) / uc_count.y;
//       float yoff{};
//       for (size_t r = 0; r < rows.size(); ++r) {
//         const auto height = row_width[r] + extra_per_uc.y * !uc_rows[r];
//         float xoff = 0.0f;
//         for (size_t c = 0; c < Columns; ++c)
//           if (const auto csp = system::slot_address<control>(rows[r][c])) {
//             const auto width = col_width[c] + extra_per_uc.x * !uc_columns[c];
//             csp->update_layout(pos + float2(xoff, yoff), float2(width, height));
//             xoff += width;
//           }
//         yoff += height;
//       }
//       update_geometry();
//     }

//     virtual void draw() const override {
//       if (!visible) return;
//       draw_background();
//       for (const auto& row : rows)
//         for (const auto& cid : row)
//           if (const auto csp = system::slot_address<control>(cid)) csp->draw();
//     }
//   };

//   using control::operator bool;
//   grid_layout() noexcept = default;
//   grid_layout(derived_from<unknown> auto& Layout) {
//     if (auto res = create_control<grid_layout<Columns>>(Layout)) _id = *res;
//   }

//   const auto& border_color() const { return unsafe_get(&slot::border_color); }
//   void border_color(const color& c) { safe_set(&slot::border_color, c); }

//   const auto& border_width() const { return unsafe_get(&slot::border_width); }
//   void border_width(float1 w) { safe_set(&slot::border_width, w.x); }

//   const auto& padding() const { return unsafe_get(&slot::padding); }
//   void padding(const float4& p) { safe_set_size(&slot::padding, p); }
// };
// } // namespace yw::ui
