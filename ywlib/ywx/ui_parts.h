// #pragma once
// #include "ywx/unknown.h"

// namespace yw::ui::parts {

// /// MARK: base

// struct base {
//   slotid control_id{};
//   bool view_changed = false;
//   bool geometry_changed = false;
//   bool layout_changed = false;
//   void make_dirty() const {
//     if (const auto csp = system::slot_address<unknown>(control_id)) csp->make_dirty();
//   }
//   void make_moved() const {
//     if (const auto csp = system::slot_address<unknown>(control_id)) csp->make_moved();
//   }
//   void make_messy() const {
//     if (const auto csp = system::slot_address<unknown>(control_id)) csp->make_messy();
//   }

//   template<typename Part> class accessor {
//   protected:
//     friend Part;
//     Part& part;
//     accessor(Part& PartRef) : part(PartRef) {}

//   public:
//     ~accessor() noexcept {
//       if (const auto csp = system::slot_address<unknown>(part.control_id)) {
//         if (part.layout_changed) csp->make_messy();
//         else if (part.geometry_changed) csp->make_moved();
//         else if (part.view_changed) csp->make_dirty();
//       }
//     }

//     accessor(accessor&& Other) noexcept = default;
//     accessor(const accessor& Other) noexcept = default;
//   };
// }; // base

// /// MARK: core

// struct core : public base {
//   float4 margin = float4::fill(arbitrary_value);
//   float2 min_size = float2::fill(arbitrary_value * 2);
//   float2 required_size{};
//   float2 provided_pos{};
//   float2 provided_area{};
//   float2 pos{};
//   float2 size{};
//   float2 radius = float2::fill(arbitrary_value);
//   comptr<ID2D1Geometry> geometry{};
//   ui::alignment alignment = ui::alignment::center;
//   vector<bool, 2> constrained{false, false};

//   class accessor : public base::accessor<core> {
//   public:
//     const auto& margin() const { return part.margin; }
//     auto& margin(float4 Margin) {
//       part.margin = vapply_r<float4>(yw::max, float4(), Margin);
//       part.layout_changed = true;
//       return *this;
//     }
//     const auto& min_size() const { return part.min_size; }
//     auto& min_size(float2 MinSize) {
//       part.min_size = vapply_r<float2>(yw::max, float2(), MinSize);
//       part.layout_changed = true;
//       return *this;
//     }
//     const auto& pos() const { return part.pos; }

//     const auto& size() const { return part.size; }
//     auto& size(std::optional<float2> Size) {
//       if (Size) {
//         part.constrained = {true, true};
//         part.required_size = *Size;
//       } else part.constrained = {false, false};
//       part.layout_changed = true;
//       return *this;
//     }

//     auto bounds() const { return part.bounds(); }

//     const auto& radius() const { return part.radius; }
//     auto& radius(float2 Radius) {
//       part.radius = Radius;
//       part.geometry_changed = true;
//       return *this;
//     }
//     const auto& alignment() const { return part.alignment; }
//     auto& alignment(ui::alignment Alignment) {
//       part.alignment = Alignment;
//       part.geometry_changed = true;
//       return *this;
//     }
//   };

//   accessor access() & noexcept { return accessor(*this); }

//   float2 bounds() const { return size + margin.xy() + margin.zw(); }
//   float2 minimum_size() const { return vapply_r<float2>(yw::max, min_size, required_size * constrained); }
//   bool hittest(float2 Pt) const noexcept {
//     BOOL b{};
//     return geometry && SUCCEEDED(geometry->FillContainsPoint(std::bit_cast<D2D1_POINT_2F>(Pt), nullptr, &b)) && b;
//   }

//   std::expected<void, error_trace> update_geometry() {
//     constexpr float c[]{0.5f, 0.0f, 1.0f};
//     const float2 cc{c[unsigned(alignment) % 3], c[(unsigned(alignment) / 3) % 3]};
//     const auto max_size = provided_area - margin.xy() - margin.zw();
//     size = size * constrained + max_size * (int2(1, 1) - constrained);
//     pos = provided_pos + margin.xy() + (max_size - size) * cc;
//     if (auto res = d2d.initialize(); !res) return unexpected_error(res.error());
//     ID2D1RoundedRectangleGeometry* geom;
//     D2D1_ROUNDED_RECT rr{D2D1_RECT_F(pos.x, pos.y, pos.x + size.x, pos.y + size.y), radius.x, radius.y};
//     if (const auto hr = d2d.factory()->CreateRoundedRectangleGeometry(&rr, &geom); FAILED(hr))
//       return unexpected_error(errors::operation_failed, "CreateRoundedRectangleGeometry failed", int32_t(hr));
//     geometry.reset(geom);
//     return {};
//   }
//   std::expected<void, error_trace> update_geometry(float2 Pos, float2 Area) {
//     provided_pos = Pos, provided_area = Area;
//     if (auto res = update_geometry(); !res) return unexpected_error(res.error());
//     return {};
//   }
// };

// /// MARK: background

// struct background : public base {
//   yw::color color = colors::white;
//   yw::bitmap image{}; // optional
//   float image_opacity = 1.0f;

//   class accessor : public base::accessor<background> {
//     using base::accessor<background>::part;

//   public:
//     const auto& color() const { return part.color; }
//     auto& color(yw::color Color) {
//       part.color = Color;
//       part.view_changed = true;
//       return *this;
//     }
//     const auto& image() const { return part.image; }
//     auto& image(yw::bitmap Image) {
//       part.image = std::move(Image);
//       part.view_changed = true;
//       return *this;
//     }
//     float image_opacity() const { return part.image_opacity; }
//     auto& image_opacity(float Opacity) {
//       part.image_opacity = Opacity;
//       part.view_changed = true;
//       return *this;
//     }
//   };

//   accessor access() & noexcept { return accessor(*this); }

//   /// \note need to call border::draw function after this
//   std::expected<void, error_trace> draw(const core& Core) {
//     brush.color(color);
//     if (auto res = fill_geometry(Core.geometry.get()); !res) return unexpected_error(res.error());
//     d2d.push_layer(Core.geometry.get());
//     if (image && image_opacity > 0.0f)
//     if (auto res = draw_bitmap(Core.pos, Core.size, image, image_opacity); !res) return unexpected_error(res.error());
//     return {};
//   }
// }; // background

// /// MARK: border

// struct border : public base {
//   yw::color color = colors::black;
//   float width = 1.0f;
//   bool dashed = false;

//   class accessor : public base::accessor<border> {
//     using base::accessor<border>::part;

//   public:
//     const auto& color() const { return part.color; }
//     auto& color(yw::color Color) {
//       part.color = Color;
//       part.view_changed = true;
//       return *this;
//     }
//     float width() const { return part.width; }
//     auto& width(float Width) {
//       part.width = Width;
//       part.view_changed = true;
//       return *this;
//     }
//     bool dashed() const { return part.dashed; }
//     auto& dashed(bool Dashed) {
//       part.dashed = Dashed;
//       part.view_changed = true;
//       return *this;
//     }
//   };

//   accessor access() noexcept { return accessor(*this); }

//   std::expected<void, error_trace> draw(const core& Core) {
//     d2d.pop_layer();
//     if (color.a > 0.0f) {
//       brush.color(color);
//       if (auto res = draw_geometry(Core.geometry.get()); !res) return unexpected_error(res.error());
//     }
//     return {};
//   }
// }; // border

// /// MARK: focus_ring

// struct focus_ring : public base {
//   color color = {0.0f, 0.0f, 1.0f, 0.5f};
//   float offset = arbitrary_value;
//   float width = arbitrary_value * 0.5f;
//   bool dashed = false;

//   class accessor : public base::accessor<focus_ring> {
//     using base::accessor<focus_ring>::part;

//   public:
//     const auto& color() const { return part.color; }
//     auto& color(yw::color Color) {
//       part.color = Color;
//       part.view_changed = true;
//       return *this;
//     }
//     float offset() const { return part.offset; }
//     auto& offset(float Offset) {
//       part.offset = Offset;
//       part.view_changed = true;
//       return *this;
//     }
//     float width() const { return part.width; }
//     auto& width(float Width) {
//       part.width = Width;
//       part.view_changed = true;
//       return *this;
//     }
//   };

//   accessor access() & noexcept { return accessor(*this); }

//   std::expected<void, error_trace> draw(const core& Core) {
//     if (!Core.geometry) return unexpected_error(errors::invalid_argument, "Invalid geometry");
//     if (brush.color().a <= 0.0f) return {};
//     const float2 center = Core.pos + Core.size * 0.5f;
//     const float2 scale = (Core.size + float2::fill(offset * 2.0f)) / Core.size;
//     auto m = D2D1::Matrix3x2F::Translation(-center.x, -center.y) *
//              D2D1::Matrix3x2F::Scale(scale.x, scale.y) *
//              D2D1::Matrix3x2F::Translation(center.x, center.y);
//     d2d.context()->SetTransform(m);
//     brush.color(color);
//     brush.dashed(dashed);
//     d2d.context()->DrawGeometry(Core.geometry.get(), brush.d2d_brush(), width, brush.d2d_stroke());
//     brush.dashed(false);
//     return {};
//   }
// }; // focus_ring
// } // namespace yw::ui::parts
