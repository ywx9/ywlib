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
  fit,   // minimum size to show whole content
  free,  // any size so that at least whole content is visible
  fixed, // specified size as is
};

enum class orientation : unsigned char {
  horizontal,
  vertical,
};

enum class part : unsigned char {
  none,
  decrease_button,
  decrease_track,
  thumb,
  increase_track,
  increase_button,
  horizontal_decrease_button,
  horizontal_decrease_track,
  horizontal_thumb,
  horizontal_increase_track,
  horizontal_increase_button,
  vertical_decrease_button,
  vertical_decrease_track,
  vertical_thumb,
  vertical_increase_track,
  vertical_increase_button,
};

using enum alignment;
using enum size_policy;
using enum orientation;

struct color_theme {
  color canvas;        // ex) background of window
  color surface;       // ex) background of control
  color surface_popup; // ex) background of popup
  color outline;       // ex) border of control
  color part;          // ex) button of checkbox, thumb of scrollbar
  color text;          // ex) text, icon
  color text_muted;    // ex) placeholder text
  color accent;        // ex) focus, selection
  color warning;
  color error;
  color success;
};

struct overlay_opacity {
  float hover = 0.10f;
  float press = 0.20f;
  float selection = 0.30f;
  float muted_text = 0.75f;
};

inline constexpr overlay_opacity default_overlay_opacity{};

inline constexpr color_theme light_color_theme{
  .canvas = color(0.96f, 0.97f, 0.98f),
  .surface = colors::white,
  .surface_popup = color(0.99f, 0.99f, 1.00f),
  .outline = color(0.45f, 0.47f, 0.50f),
  .part = color(0.75f, 0.75f, 0.75f),
  .text = color(0.10f, 0.11f, 0.13f),
  .text_muted = color(0.38f, 0.40f, 0.44f),
  .accent = color(0.05f, 0.38f, 0.78f),
  .warning = color(0.78f, 0.43f, 0.02f),
  .error = color(0.78f, 0.12f, 0.15f),
  .success = color(0.08f, 0.52f, 0.25f)};

inline constexpr color_theme dark_color_theme{
  .canvas = color(0.08f, 0.09f, 0.11f),
  .surface = color(0.13f, 0.14f, 0.17f),
  .surface_popup = color(0.18f, 0.19f, 0.23f),
  .outline = color(0.47f, 0.49f, 0.54f),
  .text = color(0.92f, 0.93f, 0.95f),
  .text_muted = color(0.65f, 0.67f, 0.72f),
  .accent = color(0.30f, 0.63f, 1.00f),
  .warning = color(1.00f, 0.67f, 0.20f),
  .error = color(1.00f, 0.38f, 0.40f),
  .success = color(0.30f, 0.78f, 0.46f)};
} // namespace ui

class control : public interface {
public:
  struct slot : interface::slot {
    function<bool, yw::button_event> button_event;
    function<bool, yw::drag_event> drag_event;
    function<bool, yw::focus_event> focus_event;
    function<bool, yw::hover_event> hover_event;
    function<bool, yw::key_event> key_event;
    function<bool, yw::pointer_event> pointer_event;
    function<bool, yw::wheel_event> wheel_event;
    string<wchar_t> tooltip;
    color background_color;
    color border_color;
    float4 margin = float4::fill(arbitrary_value);
    float4 padding = float4::fill(arbitrary_value);
    slotid window_id;
    comptr<ID2D1Geometry> geometry;
    float2 required_size;
    float2 provided_pos;
    float2 provided_area;
    float2 pos;
    float2 size;
    float2 radius = float2::fill(arbitrary_value);
    float2 minimum_size = float2::fill(arbitrary_value);
    float border_thickness = 1.0f;
    ui::alignment align = ui::center;
    vector2<ui::size_policy> policy{ui::free, ui::free};
    bool geometry_dirty = false;
    bool visible = true;
    bool enabled = true;

    //-- override functions --//

    virtual slotid get_window_id() const noexcept override { return window_id; }

    //-- vertual functions --//

    virtual std::expected<void, error> apply_color_theme(const ui::color_theme& Theme, bool Recursive) { return {}; }
    virtual void close_child_controls() {}

    virtual std::expected<void, error> draw_background() {
      if (background_color.a > 0.0f) {
        brush::color(background_color);
        if (auto res = fill_geometry(geometry.get()); !res) return res.error().relay();
      }
      return {};
    }

    virtual std::expected<void, error> draw_backcontent() { return {}; }
    virtual std::expected<void, error> draw_forecontent() { return {}; }
    virtual std::expected<void, error> draw_foreground(); // defined in `window.h`
    virtual std::expected<void, error> draw_overlay();    // defined in `window.h`

    virtual slotid find_next_tabstop(slotid Focused, bool Backward, bool& Found) const {
      if (!is_focusable()) return {};
      if (Focused == id) Found = true;
      else if (Found) return id;
      return {};
    }

    virtual std::optional<float3> get_caret_pos() const { return std::nullopt; }
    virtual float2 get_minimum_size() const { return minimum_size; }
    virtual std::expected<float2, error> get_necessary_size() const { return calc_necessary_size_by_policy({}); }

    virtual bool handle_button_event(yw::button_event e) {
      if (button_event) return button_event(e);
      return false;
    }

    virtual bool handle_char_event(wchar_t c) { return false; }
    virtual bool handle_click_event(yw::button_event e) { return false; }
    virtual bool handle_double_click_event(yw::button_event e) { return false; }
    virtual bool handle_drag_event(yw::drag_event e) { return drag_event ? drag_event(e) : false; }
    virtual bool handle_focus_event(yw::focus_event e) {
      make_dirty();
      return focus_event ? focus_event(e) : false;
    }
    virtual bool handle_hover_event(yw::hover_event e) { return hover_event ? hover_event(e) : false; }
    virtual bool handle_key_event(yw::key_event e) { return key_event ? key_event(e) : false; }
    virtual bool handle_pointer_event(yw::pointer_event e) { return pointer_event ? pointer_event(e) : false; }
    virtual bool handle_wheel_event(yw::wheel_event e) { return wheel_event ? wheel_event(e) : false; }

    virtual bool2 has_free_size_policy() const { return {policy.x == ui::free, policy.y == ui::free}; }

    virtual slotid hittest(float2 Pt) const {
      if (!visible || !enabled || !geometry) return {};
      BOOL contains = FALSE;
      if (const auto hr = geometry->FillContainsPoint({Pt.x, Pt.y}, nullptr, &contains); FAILED(hr))
        error(errors::operation_failed, "ID2D1Geometry::FillContainsPoint failed", int32_t(hr)).fizzle_out();
      return contains ? id : slotid{};
    }

    virtual bool is_focusable() const { return enabled && bool(focus_event); }

    virtual bool is_interactive() const {
      return enabled && visible && (button_event || drag_event || hover_event || pointer_event || wheel_event);
    }

    virtual std::expected<void, error> redraw() {
      if (geometry_dirty) {
        geometry_dirty = false;
        if (auto res = relocate(); !res) return res.error().relay();
      }
      if (!visible) return {};
      d2d::push_layer(geometry.get());
      if (auto res = draw_background(); !res) {
        d2d::pop_layer();
        return res.error().relay();
      }
      if (auto res = draw_backcontent(); !res) {
        d2d::pop_layer();
        return res.error().relay();
      }
      if (auto res = draw_overlay(); !res) {
        d2d::pop_layer();
        return res.error().relay();
      }
      if (auto res = draw_forecontent(); !res) {
        d2d::pop_layer();
        return res.error().relay();
      }
      d2d::pop_layer();
      if (auto res = draw_foreground(); !res) return res.error().relay();
      return {};
    }

    virtual std::expected<void, error> relocate() {
      if (auto res = update_geometry(); !res) return res.error().relay();
      return {};
    }

    virtual std::expected<void, error> relocate(float2 Pos, float2 Area) {
      provided_pos = Pos;
      provided_area = Area;
      if (auto res = relocate(); !res) return res.error().relay();
      return {};
    }

    virtual void reset_state() {}

    virtual std::expected<void, error> set_size_to_necessary() {
      if (auto res = get_necessary_size()) size = *res;
      else return res.error().relay();
      return {};
    }

    //-- shared functions --//

    static float2 align_position(float2 Origin, float2 Area, float2 Size, ui::alignment Align) noexcept {
      constexpr float c[]{0.5f, 0.0f, 1.0f};
      const float2 cc{c[unsigned(Align) % 3], c[unsigned(Align) / 4 % 3]};
      return Origin + (Area - Size) * cc;
    }

    float2 calc_necessary_size_by_policy(float2 Inner) const noexcept {
      const auto is_fixed = bool2(policy.x == ui::fixed, policy.y == ui::fixed);
      return vapply_r<float2>(yw::max, get_minimum_size(), required_size * is_fixed, Inner * (uint2(1, 1) - is_fixed));
    }

    void clear_window_state() noexcept; // defined in `window.h`

    template<ui::alignment Al> static std::expected<void, error> draw_arrow(float2 Pos, float2 Size, float Thickness) {
      const auto c = Pos + Size * 0.5f;
      if constexpr (Al == ui::left || Al == ui::right) {
        const auto dx = Size.x * 0.15f;
        const auto dy = Size.y * 0.26f;
        if constexpr (Al == ui::left) {
          const auto tip = c.add<0>(-Size.x * 0.3f);
          if (auto res = stroke_line(tip, c + float2(dx, dy), Thickness); !res) return res.error().relay();
          if (auto res = stroke_line(tip, c + float2(dx, -dy), Thickness); !res) return res.error().relay();
        } else {
          const auto tip = c.add<0>(Size.x * 0.3f);
          if (auto res = stroke_line(tip, c - float2(dx, dy), Thickness); !res) return res.error().relay();
          if (auto res = stroke_line(tip, c - float2(dx, -dy), Thickness); !res) return res.error().relay();
        }
      } else if constexpr (Al == ui::top || Al == ui::bottom) {
        const auto dx = Size.x * 0.26f;
        const auto dy = Size.y * 0.15f;
        if constexpr (Al == ui::top) {
          const auto tip = c.add<1>(-Size.y * 0.3f);
          if (auto res = stroke_line(tip, c + float2(dx, dy), Thickness); !res) return res.error().relay();
          if (auto res = stroke_line(tip, c + float2(-dx, dy), Thickness); !res) return res.error().relay();
        } else {
          const auto tip = c.add<1>(Size.y * 0.3f);
          if (auto res = stroke_line(tip, c - float2(dx, dy), Thickness); !res) return res.error().relay();
          if (auto res = stroke_line(tip, c - float2(-dx, dy), Thickness); !res) return res.error().relay();
        }
      }
      return {};
    }

    float2 get_bounds() const { return size + margin.xy() + margin.zw(); }
    std::expected<const ui::color_theme*, error> get_color_theme() const noexcept; // defined in `window.h`
    std::expected<command_manager*, error> get_command_manager() const noexcept;   // defined in `window.h`
    bool is_captured() const noexcept;                                             // defined in `window.h`
    bool is_focused() const noexcept;                                              // defined in `window.h`
    bool is_hovered() const noexcept;                                              // defined in `window.h`
    void make_dirty() const noexcept;                                              // defined in `window.h`
    void make_geometry_dirty() { geometry_dirty = true, make_dirty(); }
    void make_messy() const noexcept; // defined in `window.h`
    void sync_layout() noexcept;      // defined in `window.h`

    void swap_dimensions() {
      std::swap(minimum_size.x, minimum_size.y);
      std::swap(required_size.x, required_size.y);
      std::swap(radius.x, radius.y);
      std::swap(policy.x, policy.y);
      make_messy();
    }

    /// \return `(provided_area - margin) - necessary_size`
    std::expected<float2, error> update_geometry() noexcept {
      const auto max_size = provided_area - margin.xy() - margin.zw();
      if (auto res = set_size_to_necessary(); !res) return res.error().relay();
      const auto necessary_size = size;
      if (policy.x == ui::size_policy::free) size.x = max_size.x;
      if (policy.y == ui::size_policy::free) size.y = max_size.y;
      constexpr float c[]{0.5f, 0.0f, 1.0f};
      const float2 cc{c[unsigned(align) % 3], c[unsigned(align) / 4 % 3]};
      pos = provided_pos + margin.xy() + (max_size - size) * cc;
      ID2D1RoundedRectangleGeometry* geom = nullptr;
      D2D1_ROUNDED_RECT rr{D2D1::RectF(pos.x, pos.y, pos.x + size.x, pos.y + size.y), radius.x, radius.y};
      hresult_test(d2d::factory()->CreateRoundedRectangleGeometry, &rr, &geom);
      geometry.reset(geom);
      return size - necessary_size;
    }

    //-- internal functions --//
  };

  //-- getter --//

  bool enabled() const noexcept {
    if (const auto sp = get_slot(this)) return sp->enabled;
    error(errors::not_initialized).fizzle_out();
    return false;
  }

  bool focused() const noexcept {
    if (const auto sp = get_slot(this)) return sp->is_focused();
    error(errors::not_initialized).fizzle_out();
    return false;
  }

  bool hovered() const noexcept {
    if (const auto sp = get_slot(this)) return sp->is_hovered();
    error(errors::not_initialized).fizzle_out();
    return false;
  }

  bool visible() const noexcept {
    if (const auto sp = get_slot(this)) return sp->visible;
    error(errors::not_initialized).fizzle_out();
    return false;
  }

  const color& background_color() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::not_initialized).go_off();
    else return sp->background_color;
  }

  const color& border_color() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::not_initialized).go_off();
    else return sp->border_color;
  }

  const float4& margin() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::not_initialized).go_off();
    else return sp->margin;
  }

  const float4& padding() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::not_initialized).go_off();
    else return sp->padding;
  }

  float2 pos() const noexcept {
    if (const auto sp = get_slot(this)) return sp->pos;
    error(errors::not_initialized).fizzle_out();
    return {};
  }

  float2 size() const noexcept {
    if (const auto sp = get_slot(this)) return sp->size;
    error(errors::not_initialized).fizzle_out();
    return {};
  }

  float width() const noexcept {
    if (const auto sp = get_slot(this)) return sp->size.x;
    error(errors::not_initialized).fizzle_out();
    return {};
  }

  float height() const noexcept {
    if (const auto sp = get_slot(this)) return sp->size.y;
    error(errors::not_initialized).fizzle_out();
    return {};
  }

  float2 radius() const noexcept {
    if (const auto sp = get_slot(this)) return sp->radius;
    error(errors::not_initialized).fizzle_out();
    return {};
  }

  float2 minimum_size() const noexcept {
    if (const auto sp = get_slot(this)) return sp->minimum_size;
    error(errors::not_initialized).fizzle_out();
    return {};
  }

  const auto& button_event() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::not_initialized).go_off();
    else return sp->button_event;
  }

  const auto& drag_event() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::not_initialized).go_off();
    else return sp->drag_event;
  }

  const auto& focus_event() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::not_initialized).go_off();
    else return sp->focus_event;
  }

  const auto& hover_event() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::not_initialized).go_off();
    else return sp->hover_event;
  }

  const auto& key_event() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::not_initialized).go_off();
    else return sp->key_event;
  }

  const auto& pointer_event() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::not_initialized).go_off();
    else return sp->pointer_event;
  }

  const auto& wheel_event() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::not_initialized).go_off();
    else return sp->wheel_event;
  }

  ui::alignment align() const noexcept {
    if (const auto sp = get_slot(this)) return sp->align;
    error(errors::not_initialized).fizzle_out();
    return {};
  }

  vector2<ui::size_policy> policy() const noexcept {
    if (const auto sp = get_slot(this)) return sp->policy;
    error(errors::not_initialized).fizzle_out();
    return {};
  }

  //-- setter --//

  auto& enabled(this auto& self, bool b) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    if (sp->enabled == b) return self;
    sp->enabled = b;
    if (!b) sp->clear_window_state();
    sp->make_dirty();
    return self;
  }

  auto& visible(this auto& self, bool b) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    if (sp->visible == b) return self;
    sp->visible = b;
    if (!b) sp->clear_window_state();
    sp->make_messy();
    return self;
  }

  auto& background_color(this auto& self, const color& c) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    sp->background_color = c;
    sp->make_dirty();
    return self;
  }

  auto& border_color(this auto& self, const color& c) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    sp->border_color = c;
    sp->make_dirty();
    return self;
  }

  auto& margin(this auto& self, const float4& v) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    sp->margin = v;
    sp->make_messy();
    return self;
  }

  auto& padding(this auto& self, const float4& v) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    sp->padding = v;
    sp->make_messy();
    return self;
  }

  auto& size(this auto& self, float2 v) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    sp->required_size = v;
    sp->policy = vector2<ui::size_policy>::fill(ui::size_policy::fixed);
    sp->make_messy();
    return self;
  }

  auto& width(this auto& self, float1 v) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    sp->required_size.x = v.x;
    sp->policy.x = ui::size_policy::fixed;
    sp->make_messy();
    return self;
  }

  auto& height(this auto& self, float1 v) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    sp->required_size.y = v.x;
    sp->policy.y = ui::size_policy::fixed;
    sp->make_messy();
    return self;
  }

  auto& radius(this auto& self, float2 v) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    sp->radius = v;
    sp->make_geometry_dirty();
    return self;
  }

  auto& minimum_size(this auto& self, float2 v) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    sp->minimum_size = v;
    sp->make_messy();
    return self;
  }

  auto& button_event(this auto& self, function<bool, yw::button_event> f) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    sp->button_event = std::move(f);
    return self;
  }

  auto& drag_event(this auto& self, function<bool, yw::drag_event> f) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    sp->drag_event = std::move(f);
    return self;
  }

  auto& focus_event(this auto& self, function<bool, yw::focus_event> f) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    sp->focus_event = std::move(f);
    return self;
  }

  auto& hover_event(this auto& self, function<bool, yw::hover_event> f) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    sp->hover_event = std::move(f);
    return self;
  }

  auto& key_event(this auto& self, function<bool, yw::key_event> f) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    sp->key_event = std::move(f);
    return self;
  }

  auto& pointer_event(this auto& self, function<bool, yw::pointer_event> f) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    sp->pointer_event = std::move(f);
    return self;
  }

  auto& wheel_event(this auto& self, function<bool, yw::wheel_event> f) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    sp->wheel_event = std::move(f);
    return self;
  }

  auto& align(this auto& self, ui::alignment v) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    sp->align = v;
    sp->make_geometry_dirty();
    return self;
  }

  auto& policy(this auto& self, vector2<ui::size_policy> v) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    sp->policy = v;
    sp->make_messy();
    return self;
  }

  //-- other functions --//

  void sync_layout(this auto& self) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return;
    }
    sp->sync_layout();
  }
};
} // namespace yw
