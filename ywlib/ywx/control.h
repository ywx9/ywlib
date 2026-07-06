#pragma once
#include <ywx/core.h>
#include <ywx/keys.h>

namespace yw {

#define ywlib_control_get(mop)                    \
  const auto s = get_slot(this);                  \
  if (!s) error(errors::invalid_slotid).go_off(); \
  return s->mop

#define ywlib_control_set(mop, val, dirty)                      \
  const auto s = get_slot(&self);                               \
  if (!s) error(errors::invalid_slotid).go_off();               \
  s->mop = val;                                                 \
  if (auto res = s->make_##dirty(); !res) res.error().go_off(); \
  return self;

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
    string<wchar_t> tooltip{};
    comptr<ID2D1Geometry> geometry{};
    alignment align = alignment::center;
    vector2<size_policy> policy = {}; // free
    bool geometry_dirty = false;
    bool visible = true;
    bool enabled = true;

    static float _necessary_size(size_policy p, float min, float req, float inner) noexcept {
      const bool fixed = p == size_policy::fixed;
      return yw::max(min, req * fixed, inner * !fixed);
    }

    float2 _offset(float2 MaxSize) const noexcept {
      constexpr float c[]{0.5f, 0.0f, 1.0f};
      const float2 cc{c[unsigned(align) % 3], c[unsigned(align) / 3 % 3]};
      return margin.xy() + (MaxSize - size) * cc;
    }

    //-- override interface::slot --//

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

    //-- virtual methods for control --//

    virtual float2 bounds() const { return size + margin.xy() + margin.zw(); }

    /// {x, y, height} of caret position in window coordinates
    virtual std::optional<float3> caret_pos() const { return {}; }

    virtual slotid find_next_tabstop(slotid Focused, bool Backward, bool& Found) const {
      if (!focusable()) return {};
      if (Focused == id) Found = true;
      else if (Found) return id;
      return {};
    }

    virtual bool focusable() const { return false; }

    virtual std::expected<float2, error> get_necessary_size() const {
      return vapply_r<float2>(_necessary_size, policy, minimum_size, required_size, float2{});
    }

    virtual slotid hittest(float2 Pt) const {
      const auto r = float4(pos, pos + size);
      return visible && Pt.x >= r.x && Pt.y >= r.y && Pt.x <= r.z && Pt.y <= r.w ? id : slotid{};
    }

    virtual std::expected<void, error> make_geometry_dirty() {
      geometry_dirty = true;
      if (auto res = make_dirty(); !res) return res.error().relay();
      return {};
    }

    virtual std::expected<void, error> redraw() { return {}; }

    virtual std::expected<void, error> relocate() {
      const auto max_size = provided_area - margin.xy() - margin.zw();
      if (auto res = set_size_to_necessary(); !res) return res.error().relay();
      if (policy.x == size_policy::free) size.x = max_size.x;
      if (policy.y == size_policy::free) size.y = max_size.y;
      pos = provided_pos + _offset(max_size);
      ID2D1RoundedRectangleGeometry* geom = nullptr;
      D2D1_ROUNDED_RECT rr{D2D1::RectF(pos.x, pos.y, pos.x + size.x, pos.y + size.y), radius.x, radius.y};
      hresult_test(d2d::factory()->CreateRoundedRectangleGeometry, &rr, &geom);
      geometry.reset(geom);
      return {};
    }

    virtual std::expected<void, error> relocate(float2 Pos, float2 Area) {
      provided_pos = Pos;
      provided_area = Area;
      if (auto res = relocate(); !res) return res.error().relay();
      return {};
    }

    virtual std::expected<void, error> reset_state() { return {}; }

    virtual std::expected<void, error> set_size_to_necessary() {
      if (auto res = get_necessary_size()) size = *res;
      else return res.error().relay();
      return {};
    }

    //-- events --//

    virtual bool button_event(yw::button_event e) { return false; }
    virtual bool char_event(wchar_t c) { return false; }
    virtual bool click_event(yw::button_event e) { return false; }
    virtual bool double_click_event(yw::button_event e) { return false; }
    virtual bool drag_event(yw::drag_event e) { return false; }
    virtual void focus_event(bool Focused) {}
    virtual bool hover_event(yw::hover_event e) { return false; }
    virtual bool key_event(yw::key_event e) { return false; }
    virtual bool move_event(yw::move_event e) { return false; }
    virtual bool wheel_event(yw::wheel_event e) { return false; }
  };

  //-- getter --//

  const auto& margin() const noexcept { ywlib_control_get(margin); }
  const auto& minimum_size() const noexcept { ywlib_control_get(minimum_size); }
  const auto& required_size() const noexcept { ywlib_control_get(required_size); }
  const auto& pos() const noexcept { ywlib_control_get(pos); }
  const auto& size() const noexcept { ywlib_control_get(size); }
  const auto& width() const noexcept { ywlib_control_get(size).x; }
  const auto& height() const noexcept { ywlib_control_get(size).y; }
  const auto& radius() const noexcept { ywlib_control_get(radius); }
  const auto& align() const noexcept { ywlib_control_get(align); }
  const auto& policy() const noexcept { ywlib_control_get(policy); }

  //-- setter --//

  auto& margin(this auto& self, float4 v) noexcept { ywlib_control_set(margin, v, messy); }
  auto& minimum_size(this auto& self, float2 v) noexcept { ywlib_control_set(minimum_size, v, messy); }

  auto& size(this auto& self, float2 Size) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->required_size = Size;
    sp->policy = vector2<size_policy>::fill(size_policy::fixed);
    if (auto res = sp->make_messy(); !res) res.error().go_off();
    return self;
  }
  auto& width(this auto& self, float1 Width) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->required_size.x = Width.x;
    sp->policy.x = size_policy::fixed;
    if (auto res = sp->make_messy(); !res) res.error().go_off();
    return self;
  }
  auto& height(this auto& self, float1 Height) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->required_size.y = Height.x;
    sp->policy.y = size_policy::fixed;
    if (auto res = sp->make_messy(); !res) res.error().go_off();
    return self;
  }

  auto& radius(this auto& self, float2 v) noexcept { ywlib_control_set(radius, v, geometry_dirty); }
  auto& align(this auto& self, alignment v) noexcept { ywlib_control_set(align, v, geometry_dirty); }
  auto& policy(this auto& self, vector2<size_policy> v) noexcept { ywlib_control_set(policy, v, messy); }
  auto& tooltip(this auto& self, string<wchar_t> v) noexcept { ywlib_control_set(tooltip, std::move(v), none); }
};
} // namespace yw
