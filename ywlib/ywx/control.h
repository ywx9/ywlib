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
    optional<color> background_color;
    optional<color> border_color;
    float4 margin = float4::fill(arbitrary_value);
    float4 padding = float4::fill(arbitrary_value);
    slotid parent_id;
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
    virtual void detach_from_parent() noexcept override;
    virtual void prepare_destroy() noexcept override;

    //-- vertual functions --//

    virtual void close_child_controls() {}
    virtual void set_window_id(slotid Window) noexcept { window_id = Window; }

    virtual color get_background_color(const interface::slot* Window) const noexcept;
    virtual color get_border_color(const interface::slot* Window) const noexcept;

    virtual std::expected<void, error> draw_background(interface::slot* Window) {
      const auto color = get_background_color(Window);
      if (color.a > 0.0f) {
        brush::color(color);
        if (auto res = fill_geometry(geometry.get()); !res) return res.error().relay();
      }
      return {};
    }

    virtual std::expected<void, error> draw_backcontent(interface::slot*) { return {}; }
    virtual std::expected<void, error> draw_forecontent(interface::slot*) { return {}; }
    virtual std::expected<void, error> draw_foreground(interface::slot*); // defined in `window.h`
    virtual std::expected<void, error> draw_overlay(interface::slot*);    // defined in `window.h`

    virtual slotid find_next_tabstop(slotid Focused, bool Backward, bool& Found) const {
      if (!is_focusable()) return {};
      if (Focused == id) Found = true;
      else if (Found) return id;
      return {};
    }
    virtual slotid find_next_tabstop(interface::slot*, slotid Focused, bool Backward, bool& Found) const {
      return find_next_tabstop(Focused, Backward, Found);
    }

    virtual std::optional<float3> get_caret_pos() const { return std::nullopt; }
    virtual std::optional<float3> get_caret_pos(interface::slot*) const { return get_caret_pos(); }
    virtual float2 get_minimum_size() const { return minimum_size; }
    virtual std::expected<float2, error> get_necessary_size() const { return calc_necessary_size_by_policy({}); }

    template<typename... As>
    static bool call_event(const function<bool, As...>& Event, std::type_identity_t<As>... args) {
      if (!Event) return false;
      if (auto res = Event(static_cast<As&&>(args)...)) return *res;
      else res.error().fizzle_out();
      return false;
    }

    virtual bool handle_button_event(yw::button_event e) { return call_event(button_event, e); }
    virtual bool handle_button_event(interface::slot*, yw::button_event e) { return handle_button_event(e); }

    virtual bool handle_char_event(wchar_t c) { return false; }
    virtual bool handle_click_event(yw::button_event e) { return false; }
    virtual bool handle_double_click_event(yw::button_event e) { return false; }
    virtual bool handle_drag_event(yw::drag_event e) { return call_event(drag_event, e); }
    virtual bool handle_focus_event(yw::focus_event e) {
      make_dirty();
      return call_event(focus_event, e);
    }
    virtual bool handle_hover_event(yw::hover_event e) { return call_event(hover_event, e); }
    virtual bool handle_key_event(yw::key_event e) { return call_event(key_event, e); }
    virtual bool handle_pointer_event(yw::pointer_event e) { return call_event(pointer_event, e); }
    virtual bool handle_wheel_event(yw::wheel_event e) { return call_event(wheel_event, e); }
    virtual bool handle_char_event(interface::slot*, wchar_t c) { return handle_char_event(c); }
    virtual bool handle_click_event(interface::slot*, yw::button_event e) { return handle_click_event(e); }
    virtual bool handle_double_click_event(interface::slot*, yw::button_event e) {
      return handle_double_click_event(e);
    }
    virtual bool handle_drag_event(interface::slot*, yw::drag_event e) { return handle_drag_event(e); }
    virtual bool handle_focus_event(interface::slot*, yw::focus_event e) { return handle_focus_event(e); }
    virtual bool handle_hover_event(interface::slot*, yw::hover_event e) { return handle_hover_event(e); }
    virtual bool handle_key_event(interface::slot*, yw::key_event e) { return handle_key_event(e); }
    virtual bool handle_pointer_event(interface::slot*, yw::pointer_event e) { return handle_pointer_event(e); }
    virtual bool handle_wheel_event(interface::slot*, yw::wheel_event e) { return handle_wheel_event(e); }

    virtual bool2 has_free_size_policy() const { return {policy.x == ui::free, policy.y == ui::free}; }

    virtual slotid hittest(float2 Pt) const {
      if (!visible || !enabled || !geometry) return {};
      BOOL contains = FALSE;
      if (const auto hr = geometry->FillContainsPoint({Pt.x, Pt.y}, nullptr, &contains); FAILED(hr))
        error(errors::operation_failed, "ID2D1Geometry::FillContainsPoint failed", int32_t(hr)).fizzle_out();
      return contains ? id : slotid{};
    }
    virtual slotid hittest(interface::slot*, float2 Pt) const { return hittest(Pt); }

    virtual bool is_focusable() const { return enabled && bool(focus_event); }

    virtual bool is_interactive() const {
      return enabled && visible && (button_event || drag_event || hover_event || pointer_event || wheel_event);
    }

    virtual std::expected<void, error> redraw(interface::slot* Window) {
      if (geometry_dirty) {
        geometry_dirty = false;
        if (auto res = relocate(); !res) return res.error().relay();
      }
      if (!visible) return {};
      d2d::push_layer(geometry.get());
      if (auto res = draw_background(Window); !res) {
        d2d::pop_layer();
        return res.error().relay();
      }
      if (auto res = draw_backcontent(Window); !res) {
        d2d::pop_layer();
        return res.error().relay();
      }
      if (auto res = draw_overlay(Window); !res) {
        d2d::pop_layer();
        return res.error().relay();
      }
      if (auto res = draw_forecontent(Window); !res) {
        d2d::pop_layer();
        return res.error().relay();
      }
      d2d::pop_layer();
      if (auto res = draw_foreground(Window); !res) return res.error().relay();
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
    void clear_attachment() noexcept;

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
    std::expected<const ui::color_theme*, error> get_color_theme(const interface::slot* Window) const noexcept;
    std::expected<command_manager*, error> get_command_manager() const noexcept; // defined in `window.h`
    bool is_captured() const noexcept;                                           // defined in `window.h`
    bool is_focused() const noexcept;                                            // defined in `window.h`
    bool is_hovered() const noexcept;                                            // defined in `window.h`
    void make_dirty() const noexcept;                                            // defined in `window.h`
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

  class proxy {
  protected:
    friend class control;
    slot* _slot{};
    bool _dirty = false;
    bool _messy = false;
    bool _geometry_dirty = false;
    proxy(derived_from<slot> auto* Sp) noexcept : _slot(Sp) {
      if (!_slot) error(errors::invalid_argument, "null slot has been passed").go_off();
    }

  public:
    ~proxy() noexcept {
      if (!_slot) return;
      if (_messy) _slot->make_messy();
      else if (_geometry_dirty) _slot->make_geometry_dirty();
      else if (_dirty) _slot->make_dirty();
    }

    proxy(proxy&& o) noexcept : _slot(o._slot), _dirty(o._dirty), _messy(o._messy), _geometry_dirty(o._geometry_dirty) {
      o._slot = {};
    }

    //-- getter --//

    bool enabled() const&& noexcept { return _slot->enabled; }
    bool visible() const&& noexcept { return _slot->visible; }
    bool focused() const&& noexcept { return _slot->is_focused(); }
    bool hovered() const&& noexcept { return _slot->is_hovered(); }
    color background_color() const&& noexcept {
      return _slot->get_background_color(interface::slot::slots.get(_slot->window_id));
    }
    color border_color() const&& noexcept {
      return _slot->get_border_color(interface::slot::slots.get(_slot->window_id));
    }
    float4 margin() const&& noexcept { return _slot->margin; }
    float4 padding() const&& noexcept { return _slot->padding; }
    float2 pos() const&& noexcept { return _slot->pos; }
    float2 size() const&& noexcept { return _slot->size; }
    float width() const&& noexcept { return _slot->size.x; }
    float height() const&& noexcept { return _slot->size.y; }
    float2 radius() const&& noexcept { return _slot->radius; }
    float2 minimum_size() const&& noexcept { return _slot->minimum_size; }
    const auto& button_event() const&& noexcept { return _slot->button_event; }
    const auto& drag_event() const&& noexcept { return _slot->drag_event; }
    const auto& focus_event() const&& noexcept { return _slot->focus_event; }
    const auto& hover_event() const&& noexcept { return _slot->hover_event; }
    const auto& key_event() const&& noexcept { return _slot->key_event; }
    const auto& pointer_event() const&& noexcept { return _slot->pointer_event; }
    const auto& wheel_event() const&& noexcept { return _slot->wheel_event; }
    const auto& tooltip() const&& noexcept { return _slot->tooltip; }
    ui::alignment align() const&& noexcept { return _slot->align; }
    vector2<ui::size_policy> policy() const&& noexcept { return _slot->policy; }
    const auto& geometry() const&& noexcept { return _slot->geometry; }

    //-- setter --//

    auto enabled(this auto&& Self, bool Enabled) noexcept {
      if (Self._slot->enabled != Enabled) {
        Self._slot->enabled = Enabled;
        if (!Enabled) Self._slot->clear_window_state();
        Self._dirty = true;
      }
      return std::move(Self);
    }

    auto visible(this auto&& Self, bool Visible) noexcept {
      if (Self._slot->visible != Visible) {
        Self._slot->visible = Visible;
        if (!Visible) Self._slot->clear_window_state();
        Self._dirty = true;
      }
      return std::move(Self);
    }

    auto background_color(this auto&& Self, color Color) noexcept {
      Self._slot->background_color = Color;
      Self._dirty = true;
      return std::move(Self);
    }

    auto background_color(this auto&& Self, none) noexcept {
      Self._slot->background_color = none();
      Self._dirty = true;
      return std::move(Self);
    }

    auto border_color(this auto&& Self, color Color) noexcept {
      Self._slot->border_color = Color;
      Self._dirty = true;
      return std::move(Self);
    }

    auto border_color(this auto&& Self, none) noexcept {
      Self._slot->border_color = none();
      Self._dirty = true;
      return std::move(Self);
    }

    auto margin(this auto&& Self, float4 Margin) noexcept {
      Self._slot->margin = Margin;
      Self._messy = true;
      return std::move(Self);
    }

    auto padding(this auto&& Self, float4 Padding) noexcept {
      Self._slot->padding = Padding;
      Self._messy = true;
      return std::move(Self);
    }

    auto size(this auto&& Self, float2 Size) noexcept {
      Self._slot->required_size = Size;
      Self._slot->policy = vector2<ui::size_policy>(ui::fixed, ui::fixed);
      Self._messy = true;
      return std::move(Self);
    }

    auto width(this auto&& Self, float Width) noexcept {
      Self._slot->required_size.x = Width;
      Self._slot->policy.x = ui::fixed;
      Self._messy = true;
      return std::move(Self);
    }

    auto height(this auto&& Self, float Height) noexcept {
      Self._slot->required_size.y = Height;
      Self._slot->policy.y = ui::fixed;
      Self._messy = true;
      return std::move(Self);
    }

    auto radius(this auto&& Self, float2 Radius) noexcept {
      Self._slot->radius = Radius;
      Self._geometry_dirty = true;
      return std::move(Self);
    }

    auto minimum_size(this auto&& Self, float2 MinimumSize) noexcept {
      Self._slot->minimum_size = MinimumSize;
      if (Self._slot->size.x < MinimumSize.x || Self._slot->size.y < MinimumSize.y) Self._messy = true;
      return std::move(Self);
    }

    auto button_event(this auto&& Self, function<bool, yw::button_event> Event) noexcept {
      Self._slot->button_event = std::move(Event);
      return std::move(Self);
    }

    auto drag_event(this auto&& Self, function<bool, yw::drag_event> Event) noexcept {
      Self._slot->drag_event = std::move(Event);
      return std::move(Self);
    }

    auto focus_event(this auto&& Self, function<bool, yw::focus_event> Event) noexcept {
      Self._slot->focus_event = std::move(Event);
      return std::move(Self);
    }

    auto hover_event(this auto&& Self, function<bool, yw::hover_event> Event) noexcept {
      Self._slot->hover_event = std::move(Event);
      return std::move(Self);
    }

    auto key_event(this auto&& Self, function<bool, yw::key_event> Event) noexcept {
      Self._slot->key_event = std::move(Event);
      return std::move(Self);
    }

    auto pointer_event(this auto&& Self, function<bool, yw::pointer_event> Event) noexcept {
      Self._slot->pointer_event = std::move(Event);
      return std::move(Self);
    }

    auto wheel_event(this auto&& Self, function<bool, yw::wheel_event> Event) noexcept {
      Self._slot->wheel_event = std::move(Event);
      return std::move(Self);
    }

    auto tooltip(this auto&& Self, string<wchar_t> Tooltip) noexcept {
      Self._slot->tooltip = std::move(Tooltip);
      return std::move(Self);
    }

    auto align(this auto&& Self, ui::alignment Align) noexcept {
      Self._slot->align = Align;
      Self._geometry_dirty = true;
      return std::move(Self);
    }

    auto policy(this auto&& Self, vector2<ui::size_policy> Policy) noexcept {
      Self._slot->policy = Policy;
      Self._messy = true;
      return std::move(Self);
    }
  };

protected:
  template<derived_from<control> Handle> static std::expected<typename Handle::slot*, error> create_control() {
    const auto temp_id = make_slot<Handle>();
    const auto sp = get_slot<Handle>(temp_id);
    if (!sp) return std::unexpected(error(errors::slot_creation_failed));
    sp->id = temp_id;
    return sp;
  }

  template<derived_from<control> Handle>
  static std::expected<typename Handle::slot*, error> create_control(derived_from<interface> auto& Parent) {
    auto res_sp = create_control<Handle>();
    if (!res_sp) return res_sp.error().relay();
    const auto sp = *res_sp;
    const auto psp = get_slot<interface>(Parent.id());
    if (!psp) {
      destroy_slot(sp->id);
      return std::unexpected(error(errors::invalid_slotid));
    }
    if (auto res = psp->attach(sp->id); !res) {
      destroy_slot(sp->id);
      return res.error().relay();
    }
    return sp;
  }

public:
  bool attached() const noexcept {
    const auto sp = get_slot(this);
    return sp && bool(sp->parent_id);
  }

  std::expected<void, error> attach(derived_from<interface> auto& Parent) noexcept {
    const auto sp = get_slot(this);
    if (!sp) return std::unexpected(error(errors::not_initialized));
    if (sp->parent_id) return std::unexpected(error(errors::invalid_operation, "control already attached"));
    const auto psp = get_slot<interface>(Parent.id());
    if (!psp) return std::unexpected(error(errors::invalid_slotid));
    if (auto res = psp->attach(sp->id); !res) return res.error().relay();
    return {};
  }

  std::expected<void, error> detach() noexcept {
    const auto sp = get_slot(this);
    if (!sp) return std::unexpected(error(errors::not_initialized));
    if (!sp->parent_id) return {};
    sp->detach_from_parent();
    return {};
  }

  std::expected<void, error> destroy() noexcept {
    destroy_slot(_id);
    _id = {};
    return {};
  }

  /// synchronizes layout of this control with its parent layout.
  void sync_layout(this auto& self) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return;
    }
    sp->sync_layout();
  }

#define yw_control_getter(Prop)                                                  \
  decltype(auto) Prop(this const auto& Self) noexcept {                          \
    return typename remove_cvref<decltype(Self)>::proxy(get_slot(&Self)).Prop(); \
  }

#define yw_control_getter_setter(Prop, ...)                                                                       \
  decltype(auto) Prop(this const auto& Self) noexcept {                                                           \
    return typename remove_cvref<decltype(Self)>::proxy(get_slot(&Self)).Prop();                                  \
  }                                                                                                               \
  auto Prop(this auto& Self, __VA_ARGS__ Value) noexcept {                                                        \
    return typename remove_cvref<decltype(Self)>::proxy(get_slot(&Self)).Prop(static_cast<__VA_ARGS__&&>(Value)); \
  }

  yw_control_getter_setter(enabled, bool);
  yw_control_getter_setter(visible, bool);
  yw_control_getter(focused);
  yw_control_getter(hovered);
  yw_control_getter_setter(background_color, color);
  auto background_color(this auto& Self, none None) noexcept {
    return typename remove_cvref<decltype(Self)>::proxy(get_slot(&Self)).background_color(None);
  }
  yw_control_getter_setter(border_color, color);
  auto border_color(this auto& Self, none None) noexcept {
    return typename remove_cvref<decltype(Self)>::proxy(get_slot(&Self)).border_color(None);
  }
  yw_control_getter_setter(margin, float4);
  yw_control_getter_setter(padding, float4);
  yw_control_getter(pos);
  yw_control_getter_setter(size, float2);
  yw_control_getter_setter(width, float);
  yw_control_getter_setter(height, float);
  yw_control_getter_setter(radius, float2);
  yw_control_getter_setter(minimum_size, float2);
  yw_control_getter_setter(button_event, function<bool, yw::button_event>);
  yw_control_getter_setter(drag_event, function<bool, yw::drag_event>);
  yw_control_getter_setter(focus_event, function<bool, yw::focus_event>);
  yw_control_getter_setter(hover_event, function<bool, yw::hover_event>);
  yw_control_getter_setter(key_event, function<bool, yw::key_event>);
  yw_control_getter_setter(pointer_event, function<bool, yw::pointer_event>);
  yw_control_getter_setter(wheel_event, function<bool, yw::wheel_event>);
  yw_control_getter_setter(tooltip, string<wchar_t>);
  yw_control_getter_setter(align, ui::alignment);
  yw_control_getter_setter(policy, vector2<ui::size_policy>);
  yw_control_getter(geometry);
};
} // namespace yw
