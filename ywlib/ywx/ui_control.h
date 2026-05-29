#pragma once
#include "ywx/keys.h"
#include "ywx/ui_unknown.h"
// #include "ywx/ui_parts.h"

namespace yw::ui {

class control : public unknown {
protected:
  template<derived_from<control> Ctrl, derived_from<unknown> Layout>
  static std::expected<slotid, error_trace> create_control(Layout& layout) {
    const auto lid = layout._slotid();
    const auto lsp = system::slot_address<unknown>(lid);
    if (!lsp) return unexpected_error(errors::ui_invalid_slotid);
    if (auto res = lsp->attachable(); !res) return unexpected_error(res.error());
    const auto cid = system::uis.add(std::make_unique<typename Ctrl::slot>());
    const auto csp = system::slot_address<Ctrl>(cid);
    if (!csp) return unexpected_error(errors::ui_invalid_slotid);
    lsp->attach(cid);
    csp->id = cid;
    csp->layout_id = lid;
    csp->window_id = lsp->get_window_id();
    return cid;
  }

  template<typename Accessor, typename Self> static Accessor create_accessor(Self& self) {
    const auto csp = system::slot_address<remove_const<Self>>(self._slotid());
    if (!csp) fatal_error(errors::ui_invalid_slotid);
    if constexpr (!is_const<Self>) return Accessor(*csp);
    else return const_cast<const Accessor>(Accessor(*csp));
  }

  /// returns background and border colors.
  static tuple<color, color> get_auto_color() noexcept {
    constexpr float hues[] = {240.0f, 120.0f, 0.0f, 270.0f, 180.0f, 30.0f, 210.0f, 60.0f, 300.0f};
    static size_t color_index = 0;
    const float h = hues[color_index++ % arraysize(hues)] / 180.0f * yw::pi;
    return {hsl(h, 0.5f, 0.9f).to_srgb(), hsl(h, 0.5f, 0.2f).to_srgb()};
  }

  control() noexcept = default;

  template<typename Mp> const auto& unsafe_get(Mp mp) const {
    const auto csp = dynamic_cast<class_type<Mp>*>(system::uis.get(_id));
    if (!csp) fatal_error(errors::ui_invalid_slotid);
    return csp->*mp;
  }
  template<typename Mp, typename T> void unsafe_set(Mp mp, T&& value) {
    const auto csp = dynamic_cast<class_type<Mp>*>(system::uis.get(_id));
    if (!csp) fatal_error(errors::ui_invalid_slotid);
    csp->*mp = static_cast<T&&>(value);
  }
  template<typename Mp, typename T> std::expected<void, error_trace> safe_set(Mp mp, T&& value) {
    const auto csp = dynamic_cast<class_type<Mp>*>(system::uis.get(_id));
    if (!csp) return unexpected_error(errors::ui_invalid_slotid);
    csp->*mp = static_cast<T&&>(value);
    return {};
  }

public:
  struct slot : public unknown::slot {
    slotid layout_id{};
    slotid window_id{};
    float4 margin = float4::fill(arbitrary_value);
    float4 padding = float4::fill(arbitrary_value);
    float2 min_size = float2::fill(arbitrary_value * 2);
    float2 required_size{};
    float2 provided_pos{};
    float2 provided_area{};
    float2 pos{};
    float2 size{};
    float2 radius = float2::fill(arbitrary_value);
    comptr<ID2D1Geometry> geometry{};
    ui::alignment alignment = ui::alignment::center;
    vector<bool, 2> constrained{};
    bool visible = true;
    bool enabled = true;
    bool dying = false;

    std::wstring tooltip{};
    function<void, yw::hover_event> on_hover;

    float2 calculate_content_origin(float2 Size, float4 Padding, ui::alignment Alignment) const {
      const auto area = size - Padding.xy() - Padding.zw();
      constexpr float c[]{0.5f, 0.0f, 1.0f};
      const float2 cc{c[unsigned(Alignment) % 3], c[(unsigned(Alignment) / 3) % 3]};
      return pos + Padding.xy() + (area - Size) * cc;
    }

    //-- overrides --//

    virtual slotid get_window_id() const override { return window_id; }

    virtual std::expected<void, error_trace> make_dirty() override {
      if (const auto wsp = system::slot_address<unknown>(window_id))
        if (auto res = wsp->make_dirty(); !res) return unexpected_error(res.error());
      return {};
    }

    virtual std::expected<void, error_trace> make_moved() override {
      if (auto res = update_geometry(); !res) return unexpected_error(res.error());
      if (const auto wsp = system::slot_address<unknown>(window_id))
        if (auto res = wsp->make_dirty(); !res) return unexpected_error(res.error());
      return {};
    }

    virtual std::expected<void, error_trace> make_messy() override {
      if (const auto wsp = system::slot_address<unknown>(window_id))
        if (auto res = wsp->make_messy(); !res) return unexpected_error(res.error());
      return {};
    }

    //-- virtual functions --//

    virtual float2 bounds() const { return size + margin.xy() + margin.zw(); }

    virtual bool focusable() const { return false; }

    virtual slotid hittest(float2 Pt) const {
      const bool b = visible && Pt.x >= pos.x && Pt.y >= pos.y && Pt.x < pos.x + size.x && Pt.y < pos.y + size.y;
      return b ? id : slotid{};
    }

    virtual slotid next_tab_stop(slotid Focused, bool Forward, bool& Found) const { return {}; }

    virtual std::expected<void, error_trace> draw_focusring(
      const yw::color& Color, float Offset, float Width, bool Dashed) {
      brush.color(Color).dashed(Dashed);
      auto res = draw_round_rectangle(
        pos - float2::fill(Offset), size + float2::fill(Offset * 2.0f), radius + float2::fill(Offset), Width);
      if (!res) return unexpected_error(res.error());
      brush.dashed(false);
      return {};
    }

    virtual std::expected<float2, error_trace> calculate_minimum_size() {
      return vapply_r<float2>(yw::max, min_size, padding.xy() + padding.zw(), required_size * constrained);
    }
    virtual std::expected<void, error_trace> ensure_minimum_size() {
      if (auto res = calculate_minimum_size()) return {};
      else return unexpected_error(res.error());
    }

    virtual std::expected<void, error_trace> update_geometry() {
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
    virtual std::expected<void, error_trace> update_geometry(float2 Pos, float2 Area) {
      provided_pos = Pos, provided_area = Area;
      if (auto res = update_geometry()) return {};
      else return unexpected_error(res.error());
    }

    virtual std::expected<void, error_trace> draw() = 0;

    virtual float2 ime_position() const { return {}; };
    virtual void ime_insert_text(std::wstring_view) {}

    virtual void button_event(yw::button_event e) {}
    virtual void char_event(wchar_t c) {}
    virtual void click_event(yw::button_event e) {}
    virtual void double_click_event(yw::button_event e) {}
    virtual void drag_event(yw::drag_event e) {}
    virtual void focus_event(bool) {}
    virtual void hover_event(yw::hover_event e);
    virtual bool key_event(yw::key_event e) { return false; }
    virtual void move_event(yw::move_event e) {}
    virtual void wheel_event(yw::wheel_event e) {}
  };

  class core_accessor : public accessor<control> {
    using accessor<control>::slot;

  public:
    const auto& margin() const { return slot.margin; }
    auto& margin(float4 Margin) {
      slot.margin = vapply_r<float4>(yw::max, float4(), Margin);
      messy = true;
      return *this;
    }
    const auto& padding() const { return slot.padding; }
    auto& padding(float4 Padding) {
      slot.padding = vapply_r<float4>(yw::max, float4(), Padding);
      messy = true;
      return *this;
    }

    const auto& min_size() const { return slot.min_size; }
    auto& min_size(float2 MinSize) {
      slot.min_size = vapply_r<float2>(yw::max, float2(), MinSize);
      messy = true;
      return *this;
    }
    const auto& pos() const { return slot.pos; }

    const auto& size() const { return slot.size; }
    auto& size(std::optional<float2> Size) {
      if (Size) {
        slot.constrained = {true, true};
        slot.required_size = *Size;
      } else slot.constrained = {false, false};
      messy = true;
      return *this;
    }

    auto bounds() const { return slot.bounds(); }

    const auto& radius() const { return slot.radius; }
    auto& radius(float2 Radius) {
      slot.radius = Radius;
      moved = true;
      return *this;
    }
    const auto& alignment() const { return slot.alignment; }
    auto& alignment(ui::alignment Alignment) {
      slot.alignment = Alignment;
      moved = true;
      return *this;
    }
  };

  virtual ~control() {
    const auto csp = system::slot_address<control>(_id);
    if (!csp) return;
    if (auto res = csp->make_messy(); !res) fatal_error(res.error());
    const auto lsp = system::slot_address<unknown>(csp->layout_id);
    if (!lsp) return;
    if (auto res = lsp->detach(csp->id); !res) return;
  }

  control(control&&) = default;
  control& operator=(control&&) = default;

  template<typename Self> decltype(auto) core(this Self& self) { return create_accessor<core_accessor>(self); }

  const auto& tooltip() const { return unsafe_get(&slot::tooltip); }
  std::expected<void, error_trace> tooltip(std::wstring Tooltip) {
    if (auto res = safe_set(&slot::tooltip, std::move(Tooltip))) return {};
    else return unexpected_error(res.error());
  }

  const auto& on_hover() const { return unsafe_get(&slot::on_hover); }
  std::expected<void, error_trace> on_hover(function<void, yw::hover_event> f) {
    if (auto res = safe_set(&slot::on_hover, std::move(f))) return {};
    else return unexpected_error(res.error());
  }
};

/// MARK: frame

class frame : public control {
public:
  struct slot : public control::slot {
    bitmap background_image;
    color background_color = colors::transparent;
    float background_image_opacity = 1.0f;

    color border_color = colors::transparent;
    float border_thickness = 1.0f;
    bool border_dashed = false;

    //-- overrides --//

    std::expected<void, error_trace> draw() override {
      if (!visible) return {};
      if (auto res = draw_background(); !res) return unexpected_error(res.error());
      if (auto res = draw_foreground(); !res) return unexpected_error(res.error());
      return {};
    }

    //-- functions --//

    std::expected<void, error_trace> draw_background() {
      if (background_color.a > 0.0f) {
        brush.color(background_color);
        if (auto res = fill_geometry(geometry.get()); !res) return unexpected_error(res.error());
      }
      d2d.push_layer(geometry.get());
      if (background_image_opacity > 0.0f && background_image) {
        if (auto res = draw_bitmap(pos, size, background_image, background_image_opacity); !res)
          return unexpected_error(res.error());
      }
      return {};
    }

    std::expected<void, error_trace> draw_foreground() {
      d2d.pop_layer();
      if (border_color.a > 0.0f && border_thickness > 0.0f) {
        brush.color(border_color).dashed(border_dashed);
        if (auto res = draw_geometry(geometry.get(), border_thickness); !res) return unexpected_error(res.error());
        brush.dashed(false);
      }
      return {};
    }
  };

  class background_accessor : public accessor<frame> {
    using accessor<frame>::slot;

  public:
    const auto& image() const { return slot.background_image; }
    auto& image(bitmap Image) {
      slot.background_image = std::move(Image);
      dirty = true;
      return *this;
    }
    const auto& image_opacity() const { return slot.background_image_opacity; }
    auto& image_opacity(float Opacity) {
      slot.background_image_opacity = Opacity;
      dirty = true;
      return *this;
    }
    const auto& color() const { return slot.background_color; }
    auto& color(const yw::color Color) {
      slot.background_color = Color;
      dirty = true;
      return *this;
    }
  };

  class border_accessor : public accessor<frame> {
    using accessor<frame>::slot;

  public:
    const auto& color() const { return slot.border_color; }
    auto& color(const yw::color Color) {
      slot.border_color = Color;
      dirty = true;
      return *this;
    }
    const auto& thickness() const { return slot.border_thickness; }
    auto& thickness(float Thickness) {
      slot.border_thickness = Thickness;
      dirty = true;
      return *this;
    }
    const auto& dashed() const { return slot.border_dashed; }
    auto& dashed(bool Dashed) {
      slot.border_dashed = Dashed;
      dirty = true;
      return *this;
    }
  };

  template<typename Self> decltype(auto) background(this Self&& self) { create_accessor<background_accessor>(self); }
  template<typename Self> decltype(auto) border(this Self&& self) { create_accessor<border_accessor>(self); }
};
} // namespace yw::ui
