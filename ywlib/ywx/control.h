#pragma once
#include <ywx/core.h>

namespace yw {

#define ywlib_control_get(h, mop)          \
  const auto s = get_slot(h);                     \
  if (!s) error(errors::invalid_slotid).go_off(); \
  return s->mop

#define ywlib_control_set(h, mop, val, make)     \
  const auto s = get_slot(h);                           \
  if (!s) error(errors::invalid_slotid).go_off();       \
  s->mop = val;                                         \
  if (auto res = s->make(); !res) res.error().go_off(); \
  return *this

class control : public interface {
public:
  enum class alignment : unsigned char {
    center = 0b0000,
    left = 0b0001,
    right = 0b0010,
    top = 0b0100,
    bottom = 0b1000,
    left_top = 0b0101,
    left_bottom = 0b1001,
    right_top = 0b0110,
    right_bottom = 0b1010,
  };

  enum class size_policy : unsigned char {
    free,  // size so that at least whole content is visible
    fixed, // specified size as is
    fit    // minimum size to show whole content
  };

  struct slot : interface::slot {
    slotid layout_id{};
    slotid window_id{};
    float4 margin = float4::fill(arbitrary_value);
    float2 minimum_size = float2::fill(arbitrary_value);
    float2 required_size{};
    float2 provided_pos{};
    float2 provided_area{};
    float2 pos{};
    float2 size{};
    float2 radius = float2::fill(arbitrary_value);
    comptr<ID2D1Geometry> geometry{};
    alignment align = alignment::center;
    vector2<size_policy> policy = {}; // free
    bool visible = true;
    bool enabled = true;

    static float _necessary_size(size_policy p, float min, float req, float inner) noexcept {
      const bool fixed = p == size_policy::fixed;
      return yw::max(min, req * fixed, inner * !fixed);
    }

    virtual std::expected<void, error> make_dirty() override {
      if (const auto wsp = slot::get<interface>(window_id)) wsp->make_dirty();
      else return std::unexpected(error(errors::invalid_slotid));
      return {};
    }

    virtual std::expected<void, error> make_messy() override {
      if (const auto wsp = slot::get<interface>(window_id)) wsp->make_messy();
      else return std::unexpected(error(errors::invalid_slotid));
      return {};
    }

    virtual float2 bounds() const { return size + margin.xy() + margin.zw(); }

    virtual std::expected<float2, error> get_necessary_size() const {
      const auto inner = margin.xy() + margin.zw();
      return vapply_r<float2>(_necessary_size, policy, minimum_size, required_size, inner);
    }

    virtual std::expected<void, error> set_size_to_necessary() {
      if (auto res = get_necessary_size()) size = *res;
      else return res.error().relay();
      return {};
    }

    virtual std::expected<void, error> redraw() { return {}; }

    float2 _offset(float2 MaxSize) const noexcept {
      constexpr float c[]{0.5f, 0.0f, 1.0f};
      const float2 cc{c[unsigned(align) % 3], c[unsigned(align) / 3 % 3]};
      return margin.xy() + (MaxSize - size) * cc;
    }

    virtual std::expected<void, error> relocate(float2 Pos, float2 Area) {
      const auto max_size = Area - margin.xy() - margin.zw();
      provided_pos = Pos;
      provided_area = Area;
      if (policy.x == size_policy::free) size.x = max_size.x;
      if (policy.y == size_policy::free) size.y = max_size.y;
      pos = Pos + _offset(max_size);
      ID2D1RoundedRectangleGeometry* geom = nullptr;
      D2D1_ROUNDED_RECT rr{D2D1::RectF(pos.x, pos.y, pos.x + size.x, pos.y + size.y), radius.x, radius.y};
      hresult_test(d2d::factory()->CreateRoundedRectangleGeometry, &rr, &geom);
      geometry.reset(geom);
      return {};
    }
  };

  //-- getter --//

  const auto& margin() const noexcept { ywlib_control_get(this, margin); }
  const auto& minimum_size() const noexcept { ywlib_control_get(this, minimum_size); }
  const auto& required_size() const noexcept { ywlib_control_get(this, required_size); }
  const auto& pos() const noexcept { ywlib_control_get(this, pos); }
  const auto& size() const noexcept { ywlib_control_get(this, size); }
  const auto& radius() const noexcept { ywlib_control_get(this, radius); }
  const auto& align() const noexcept { ywlib_control_get(this, align); }
  const auto& policy() const noexcept { ywlib_control_get(this, policy); }
};
} // namespace yw
