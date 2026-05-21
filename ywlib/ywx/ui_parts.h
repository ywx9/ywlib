#pragma once
#include "ywx/unknown.h"

namespace yw::ui::parts {

/// MARK: base

struct base {
  slotid control_id{};
  bool view_changed = false;
  bool geometry_changed = false;
  bool layout_changed = false;
  void make_dirty() const {
    if (const auto csp = system::slot_address<unknown>(control_id)) csp->make_dirty();
  }
  void make_moved() const {
    if (const auto csp = system::slot_address<unknown>(control_id)) csp->make_moved();
  }
  void make_messy() const {
    if (const auto csp = system::slot_address<unknown>(control_id)) csp->make_messy();
  }
  template<derived_from<base> Part> class setter {
  protected:
    friend Part;
    Part& part;
    setter(Part& PartRef) : part(PartRef) {}

  public:
    ~setter() {
      if (part.layout_changed) part.make_messy();
      else if (part.geometry_changed) part.make_moved();
      else if (part.view_changed) part.make_dirty();
      part.view_changed = part.geometry_changed = part.layout_changed = false;
    }
    setter(setter&& Other) noexcept = default;
    setter(const setter& Other) noexcept = default;
  };

  template<derived_from<base> Part> class getter {
  protected:
    friend Part;
    const Part& part;
    getter(const Part& PartRef) : part(PartRef) {}

  public:
    getter(getter&& Other) noexcept = default;
    getter(const getter& Other) noexcept = default;
  };
}; // base

/// MARK: core

struct core : public base {
  float4 margin = float4::fill(arbitrary_value);
  float2 min_size = float2::fill(arbitrary_value * 2);
  float2 required_size{};
  float2 provided_pos{};
  float2 provided_area{};
  float2 pos{};
  float2 size{};
  float2 radius = float2::fill(arbitrary_value);
  comptr<ID2D1Geometry> geometry{};
  ui::alignment alignment = ui::alignment::center;
  vector<bool, 2> constrained{false, false};

  class setter : public base::setter<core> {
  public:
    auto& margin(float4 Margin) {
      part.margin = vapply_r<float4>(yw::max, float4(), Margin);
      part.layout_changed = true;
      return *this;
    }
    auto& min_size(float2 MinSize) {
      part.min_size = vapply_r<float2>(yw::max, float2(), MinSize);
      part.layout_changed = true;
      return *this;
    }
    auto& size(std::optional<float2> Size) {
      if (Size) {
        part.constrained = {true, true};
        part.required_size = *Size;
      } else part.constrained = {false, false};
      part.layout_changed = true;
      return *this;
    }
    auto& radius(float2 Radius) {
      part.radius = Radius;
      part.geometry_changed = true;
      return *this;
    }
    auto& alignment(ui::alignment Alignment) {
      part.alignment = Alignment;
      part.geometry_changed = true;
      return *this;
    }
  };

  class getter : public base::getter<core> {
  public:
    const auto& margin() const { return part.margin; }
    const auto& min_size() const { return part.min_size; }
    const auto& pos() const { return part.pos; }
    const auto& size() const { return part.size; }
    const auto& radius() const { return part.radius; }
    const auto& alignment() const { return part.alignment; }
  };

  setter set() { return setter(*this); }
  getter get() const { return getter(*this); }

  float2 bounds() const { return size + margin.xy() + margin.zw(); }
  float2 minimum_size() const { return vapply_r<float2>(yw::max, min_size, required_size * constrained); }
  bool hittest(float2 Pt) const noexcept {
    BOOL b{};
    return geometry && SUCCEEDED(geometry->FillContainsPoint(std::bit_cast<D2D1_POINT_2F>(Pt), nullptr, &b)) && b;
  }

  std::expected<void, error_trace> update_geometry() {
    constexpr float c[]{0.5f, 0.0f, 1.0f};
    const float2 cc{c[unsigned(alignment) % 3], c[(unsigned(alignment) / 3) % 3]};
    const auto max_size = provided_area - margin.xy() - margin.zw();
    size = size * constrained + max_size * (int2(1, 1) - constrained);
    pos = provided_pos + margin.xy() + (max_size - size) * cc;
    if (auto res = d2d.initialize(); !res) return unexpected_error(res.error());
    ID2D1RoundedRectangleGeometry* geom;
    D2D1_ROUNDED_RECT rr{D2D1_RECT_F(pos.x, pos.y, pos.x + size.x, pos.y + size.y), radius.x, radius.y};
    if (const auto hr = d2d.factory()->CreateRoundedRectangleGeometry(&rr, &geom); FAILED(hr))
      return unexpected_error(errors::operation_failed, "CreateRoundedRectangleGeometry failed", int32_t(hr));
    geometry.reset(geom);
    return {};
  }
  std::expected<void, error_trace> update_geometry(float2 Pos, float2 Area) {
    provided_pos = Pos, provided_area = Area;
    if (auto res = update_geometry(); !res) return unexpected_error(res.error());
    return {};
  }
};

/// MARK: background

struct background : public base {
  yw::color color = colors::white;
  yw::bitmap image{}; // optional
  float image_opacity = 1.0f;

  class setter : public base::setter<background> {
  public:
    auto& color(yw::color Color) {
      part.color = Color;
      part.view_changed = true;
      return *this;
    }
    auto& image(yw::bitmap Image) {
      part.image = std::move(Image);
      part.view_changed = true;
      return *this;
    }
    auto& image_opacity(float Opacity) {
      part.image_opacity = Opacity;
      part.view_changed = true;
      return *this;
    }
  };

  class getter : public base::getter<background> {
  public:
    const auto& color() const { return part.color; }
    const auto& image() const { return part.image; }
    float image_opacity() const { return part.image_opacity; }
  };

  setter set() { return setter(*this); }
  getter get() const { return getter(*this); }
}; // background

/// MARK: border

struct border : public base {
  yw::color color = colors::black;
  float width = 1.0f;
  bool dashed = false;

  class setter : public base::setter<border> {
  public:
    auto& color(yw::color Color) {
      part.color = Color;
      part.view_changed = true;
      return *this;
    }
    auto& width(float Width) {
      part.width = Width;
      part.view_changed = true;
      return *this;
    }
    auto& dashed(bool Dashed) {
      part.dashed = Dashed;
      part.view_changed = true;
      return *this;
    }
  };

  class getter : public base::getter<border> {
  public:
    const auto& color() const { return part.color; }
    float width() const { return part.width; }
    bool dashed() const { return part.dashed; }
  };

  setter set() { return setter(*this); }
  getter get() const { return getter(*this); }
}; // border

/// MARK: focus_ring

struct focus_ring : public base {
  color color = {0.0f, 0.0f, 1.0f, 0.5f};
  float offset = arbitrary_value;
  float width = arbitrary_value * 0.5f;

  class setter : public base::setter<focus_ring> {
  public:
    auto& color(yw::color Color) {
      part.color = Color;
      part.view_changed = true;
      return *this;
    }
    auto& offset(float Offset) {
      part.offset = Offset;
      part.view_changed = true;
      return *this;
    }
    auto& width(float Width) {
      part.width = Width;
      part.view_changed = true;
      return *this;
    }
  };

  class getter : public base::getter<focus_ring> {
  public:
    const auto& color() const { return part.color; }
    float offset() const { return part.offset; }
    float width() const { return part.width; }
  };

  setter set() { return setter(*this); }
  getter get() const { return getter(*this); }
}; // focus_ring
} // namespace yw::ui::parts
