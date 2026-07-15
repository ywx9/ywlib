#pragma once
#include <ywx/command_manager.h>
#include <ywx/core.h>
#include <ywx/keys.h>

namespace yw {

namespace ui {
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

enum class orientation : unsigned char {
  horizontal,
  vertical,
};
}

#define ywlib_control_get(mop)                    \
  const auto s = get_slot(this);                  \
  if (!s) error(errors::invalid_slotid).go_off(); \
  return s->mop

#define ywlib_control_set(mop, val, dirty)        \
  const auto s = get_slot(&self);                 \
  if (!s) error(errors::invalid_slotid).go_off(); \
  s->mop = val;                                   \
  s->make_##dirty();                              \
  return self;

class control : public interface {
public:
  struct slot : interface::slot {
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
    function<bool, button_event> on_button{};
    function<bool, key_event> on_key{};
    function<void, hover_event> on_hover{};
    function<void, bool> on_focus{};
    ui::alignment align{}; // center
    vector2<ui::size_policy> policy{}; // free
    bool geometry_dirty = false;
    bool visible = true;
    bool enabled = true;

    float2 get_bounds() const { return size + margin.xy() + margin.zw(); }

    float2 calc_necessary_size_by_policy(float2 Inner) const noexcept {
      const vector2<bool> fixed{policy.x == ui::size_policy::fixed, policy.y == ui::size_policy::fixed};
      return vapply_r<float2>(yw::max, minimum_size, required_size * fixed, Inner - Inner * fixed);
    }

    float2 calc_offset_by_align(float2 MaxSize) const noexcept {
      constexpr float c[]{0.5f, 0.0f, 1.0f};
      const float2 cc{c[unsigned(align) % 3], c[unsigned(align) / 3 % 3]};
      return margin.xy() + (MaxSize - size) * cc;
    }

    void clear_window_state() noexcept;
    command_manager* commands() const noexcept;
    bool focused() const noexcept;
    bool hovered() const noexcept;
    /// \note These functions are implemented in "window.h" because they require window::slot::focused_control_id

    //-- override interface::slot --//

    virtual slotid get_window_id() const noexcept override { return window_id; }

    virtual void make_dirty() noexcept override {
      if (const auto wsp = get_slot<interface>(window_id)) wsp->make_dirty();
    }

    virtual void make_messy() noexcept override {
      if (const auto wsp = get_slot<interface>(window_id)) wsp->make_messy();
    }

    //-- virtual methods for control --//

    virtual std::expected<void, error> draw_focusring(const color& Color, float Thickness, float2 Offset) {
      const auto p = pos - Offset;
      const auto r = radius + Offset;
      const auto s = size + Offset * 2.0f;
      brush::color(Color);
      if (auto res = draw_round_rectangle(p, s, r, Thickness); !res) return res.error().relay();
      return {};
    }

    virtual slotid find_next_tabstop(slotid Focused, bool Backward, bool& Found) const {
      if (!focusable()) return {};
      if (Focused == id) Found = true;
      else if (Found) return id;
      return {};
    }

    virtual bool focusable() const noexcept { return false; }

    virtual std::expected<float2, error> get_necessary_size() const {
      return calc_necessary_size_by_policy(float2{});
    }

    virtual std::expected<std::optional<float3>, error> get_caret_pos() { return std::nullopt; }

    virtual slotid hittest(float2 Pt) const {
      const auto r = float4(pos, pos + size);
      return visible && Pt.x >= r.x && Pt.y >= r.y && Pt.x <= r.z && Pt.y <= r.w ? id : slotid{};
    }

    virtual void make_geometry_dirty() {
      geometry_dirty = true;
      make_dirty();
    }

    virtual std::expected<void, error> redraw() { return {}; }

    virtual std::expected<void, error> relocate() {
      const auto max_size = provided_area - margin.xy() - margin.zw();
      if (auto res = set_size_to_necessary(); !res) return res.error().relay();
      if (policy.x == ui::size_policy::free) size.x = max_size.x;
      if (policy.y == ui::size_policy::free) size.y = max_size.y;
      pos = provided_pos + calc_offset_by_align(max_size);
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

    virtual bool button_event(yw::button_event e) {
      if (on_button) return on_button(e);
      return false;
    }

    virtual bool char_event(wchar_t c) { return false; }
    virtual bool click_event(yw::button_event e) { return false; }
    virtual bool double_click_event(yw::button_event e) { return false; }
    virtual bool drag_event(yw::drag_event e) { return false; }

    virtual void focus_event(bool Focused) {
      if (on_focus) on_focus(Focused);
    }

    virtual bool hover_event(yw::hover_event e) {
      if (on_hover) on_hover(e);
      return false;
    }

    virtual bool key_event(yw::key_event e) {
      if (on_key) return on_key(e);
      return false;
    }

    virtual bool move_event(yw::move_event e) { return false; }
    virtual bool wheel_event(yw::wheel_event e) { return false; }
  };

  //-- getter --//

  bool focused() const noexcept {
    if (const auto sp = get_slot(this)) return sp->focused();
    return false;
  }

  bool hovered() const noexcept {
    if (const auto sp = get_slot(this)) return sp->hovered();
    return false;
  }

  const auto& margin() const noexcept { ywlib_control_get(margin); }
  const auto& minimum_size() const noexcept { ywlib_control_get(minimum_size); }
  const auto& required_size() const noexcept { ywlib_control_get(required_size); }
  const auto& pos() const noexcept { ywlib_control_get(pos); }
  const auto& size() const noexcept { ywlib_control_get(size); }
  const auto& width() const noexcept { ywlib_control_get(size).x; }
  const auto& height() const noexcept { ywlib_control_get(size).y; }
  const auto& radius() const noexcept { ywlib_control_get(radius); }
  const auto& on_button() const noexcept { ywlib_control_get(on_button); }
  const auto& on_key() const noexcept { ywlib_control_get(on_key); }
  const auto& on_hover() const noexcept { ywlib_control_get(on_hover); }
  const auto& on_focus() const noexcept { ywlib_control_get(on_focus); }
  const auto& align() const noexcept { ywlib_control_get(align); }
  const auto& policy() const noexcept { ywlib_control_get(policy); }
  bool visible() const noexcept { ywlib_control_get(visible); }
  bool enabled() const noexcept { ywlib_control_get(enabled); }

  //-- setter --//

  auto& margin(this auto& self, float4 v) noexcept { ywlib_control_set(margin, v, messy); }
  auto& minimum_size(this auto& self, float2 v) noexcept { ywlib_control_set(minimum_size, v, messy); }

  auto& size(this auto& self, float2 Size) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->required_size = Size;
    sp->policy = vector2<ui::size_policy>::fill(ui::size_policy::fixed);
    sp->make_messy();
    return self;
  }

  auto& width(this auto& self, float1 Width) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->required_size.x = Width.x;
    sp->policy.x = ui::size_policy::fixed;
    sp->make_messy();
    return self;
  }

  auto& height(this auto& self, float1 Height) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->required_size.y = Height.x;
    sp->policy.y = ui::size_policy::fixed;
    sp->make_messy();
    return self;
  }

  auto& radius(this auto& self, float2 v) noexcept { ywlib_control_set(radius, v, geometry_dirty); }

  auto& on_button(this auto& self, function<bool, button_event> f) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->on_button = std::move(f);
    return self;
  }

  auto& on_key(this auto& self, function<bool, key_event> f) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->on_key = std::move(f);
    return self;
  }

  auto& on_hover(this auto& self, function<void, hover_event> f) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->on_hover = std::move(f);
    return self;
  }

  auto& on_focus(this auto& self, function<void, bool> f) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->on_focus = std::move(f);
    return self;
  }

  auto& align(this auto& self, ui::alignment v) noexcept { ywlib_control_set(align, v, geometry_dirty); }
  auto& policy(this auto& self, vector2<ui::size_policy> v) noexcept { ywlib_control_set(policy, v, messy); }
  auto& tooltip(this auto& self, string<wchar_t> v) noexcept { ywlib_control_set(tooltip, std::move(v), none); }

  auto& visible(this auto& self, bool b) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    if (sp->visible == b) return self;
    sp->visible = b;
    if (!b) sp->clear_window_state();
    sp->make_messy();
    return self;
  }

  auto& enabled(this auto& self, bool b) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    if (sp->enabled == b) return self;
    sp->enabled = b;
    if (!b) sp->clear_window_state();
    sp->make_dirty();
    return self;
  }
};
} // namespace yw
