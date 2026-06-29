#pragma once
#include <ywx/core.h>

namespace yw {

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

    static float calculate_necessary_size(size_policy p, float min, float req, float inner) noexcept {
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

    virtual std::expected<float2, error>
  };
};
} // namespace yw
