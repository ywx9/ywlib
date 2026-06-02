#pragma once
#include "ywx/keys.h"
#include "ywx/ui_unknown.h"
// #include "ywx/ui_parts.h"

namespace yw::errors {
define_error(ui_invalid_size_policy);
}

namespace yw::ui {

enum class size_policy { free, fixed, fit_content };

struct color_pair {
  color background;
  color foreground;
  static color_pair auto_color() noexcept {
    constexpr float hues[] = {240.0f, 120.0f, 0.0f, 270.0f, 180.0f, 30.0f, 210.0f, 60.0f, 300.0f};
    static size_t color_index = 0;
    const float h = hues[color_index++ % arraysize(hues)] / 180.0f * yw::pi;
    return {hsl(h, 0.5f, 0.9f).to_srgb(), hsl(h, 0.5f, 0.2f).to_srgb()};
  }
  template<size_t I, typename Self> requires(I < 2) constexpr color& get(this Self&& self) noexcept {
    return select<I>(self.background, self.foreground);
  }
};

class control : public unknown {
protected:
  template<derived_from<control> Ctrl, derived_from<unknown> Layout>
  static std::expected<slotid, error_trace> create_control(Layout& layout) {
    const auto lid = layout._slotid();
    const auto lsp = system::slot_address<unknown>(lid);
    if (!lsp) return unexpected_error(errors::ui_invalid_slotid);
    if (!lsp->attachable()) return unexpected_error(errors::ui_not_attachable);
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
  /// MARK: slot

  struct slot : public unknown::slot {
    slotid layout_id{};
    slotid window_id{};
    float4 margin = float4::fill(arbitrary_value);
    float4 padding = float4::fill(arbitrary_value);
    float2 minimum_size = float2::fill(arbitrary_value * 2);
    float2 required_size{};
    float2 provided_pos{};
    float2 provided_area{};
    float2 pos{};
    float2 size{};
    float2 radius = float2::fill(arbitrary_value);
    comptr<ID2D1Geometry> geometry{};
    ui::alignment alignment = ui::alignment::center;
    // vector<bool, 2> constrained{};
    vector<ui::size_policy, 2> size_policy{};
    bool crop_content = true;
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

    static float _calc_nec_size(ui::size_policy Policy, float Min, float Required, float Inner) {
      const float a[]{Inner, Required, Inner};
      return yw::max(Min, a[yw::min(unsigned(Policy), unsigned(ui::size_policy::fit_content))]);
    }

    bool hittest_geometry(float2 Pt) const {
      if (!geometry) return false;
      BOOL contains = FALSE;
      const auto hr = geometry->FillContainsPoint(D2D1_POINT_2F{Pt.x, Pt.y}, nullptr, &contains);
      return SUCCEEDED(hr) && contains;
    }

    virtual slotid get_window_id() const override { return window_id; }

    virtual std::expected<void, error_trace> make_dirty() override {
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
    virtual slotid hittest(float2 Pt) const { return hittest_geometry(Pt) ? id : slotid{}; }
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

    virtual std::expected<float2, error_trace> calculate_necessary_size() const {
      const auto inner = padding.xy() + padding.zw();
      return vapply_r<float2>(_calc_nec_size, size_policy, minimum_size, required_size, inner);
    }

    virtual std::expected<void, error_trace> ensure_necessary_size() {
      if (auto res = calculate_necessary_size()) return size = *res, std::expected<void, error_trace>{};
      else return unexpected_error(res.error());
    }

    virtual std::expected<void, error_trace> update_geometry(float2 Pos, float2 Area) {
      constexpr float c[]{0.5f, 0.0f, 1.0f};
      const float2 cc{c[unsigned(alignment) % 3], c[(unsigned(alignment) / 3) % 3]};
      provided_pos = Pos;
      provided_area = Area;
      const auto max_size = provided_area - margin.xy() - margin.zw();
      if (size_policy.x == ui::size_policy::free) size.x = max_size.x;
      if (size_policy.y == ui::size_policy::free) size.y = max_size.y;
      pos = provided_pos + margin.xy() + (max_size - size) * cc;
      if (auto res = d2d.initialize(); !res) return unexpected_error(res.error());
      ID2D1RoundedRectangleGeometry* geom;
      D2D1_ROUNDED_RECT rr{D2D1_RECT_F(pos.x, pos.y, pos.x + size.x, pos.y + size.y), radius.x, radius.y};
      if (const auto hr = d2d.factory()->CreateRoundedRectangleGeometry(&rr, &geom); FAILED(hr))
        return unexpected_error(errors::operation_failed, "CreateRoundedRectangleGeometry failed", int32_t(hr));
      geometry.reset(geom);
      return {};
    }

    virtual std::expected<void, error_trace> draw() const = 0;

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

  /// MARK: core accessor

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
    const auto& minimum_size() const { return slot.minimum_size; }
    auto& minimum_size(float2 Size) {
      slot.minimum_size = vapply_r<float2>(yw::max, float2(), Size);
      messy = true;
      return *this;
    }
    const auto& pos() const { return slot.pos; }
    const auto& size() const { return slot.size; }
    auto& size(float2 Size) {
      slot.size_policy = {ui::size_policy::fixed, ui::size_policy::fixed};
      slot.required_size = Size;
      messy = true;
      return *this;
    }
    auto& size(std::nullopt_t) {
      slot.size_policy = {ui::size_policy::free, ui::size_policy::free};
      messy = true;
      return *this;
    }
    const auto& width() const { return slot.size.x; }
    auto& width(float1 Width) {
      slot.size_policy.x = ui::size_policy::fixed;
      slot.required_size.x = Width.x;
      messy = true;
      return *this;
    }
    auto& width(std::nullopt_t) {
      slot.size_policy.x = ui::size_policy::free;
      messy = true;
      return *this;
    }
    const auto& height() const { return slot.size.y; }
    auto& height(float1 Height) {
      slot.size_policy.y = ui::size_policy::fixed;
      slot.required_size.y = Height.x;
      messy = true;
      return *this;
    }
    auto& height(std::nullopt_t) {
      slot.size_policy.y = ui::size_policy::free;
      messy = true;
      return *this;
    }
    auto bounds() const { return slot.bounds(); }
    const auto& radius() const { return slot.radius; }
    auto& radius(float1 Radius) {
      slot.radius = float2(Radius.x, Radius.x);
      messy = true;
      return *this;
    }
    auto& radius(float2 Radius) {
      slot.radius = Radius;
      messy = true;
      return *this;
    }
    const auto& alignment() const { return slot.alignment; }
    auto& alignment(ui::alignment Alignment) {
      slot.alignment = Alignment;
      messy = true;
      return *this;
    }
    const auto& size_policy() const { return slot.size_policy; }
    auto& fit_content(bool b) {
      if (b) slot.size_policy = {ui::size_policy::fit_content, ui::size_policy::fit_content};
      else slot.size_policy = {ui::size_policy::free, ui::size_policy::free};
      messy = true;
      return *this;
    }
    auto& fit_content(vector2<bool> b) {
      slot.size_policy.x = b.x ? ui::size_policy::fit_content : ui::size_policy::free;
      slot.size_policy.y = b.y ? ui::size_policy::fit_content : ui::size_policy::free;
      messy = true;
      return *this;
    }
    const auto& crop_content() const { return slot.crop_content; }
    auto& crop_content(bool b) {
      dirty = slot.crop_content != b;
      slot.crop_content = b;
      return *this;
    }
  };

  /// MARK: handle functions

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

  bool visible() const { return unsafe_get(&slot::visible); }
  auto& visible(bool b) {
    const auto csp = system::slot_address<control>(_slotid());
    if (!csp) fatal_error(errors::ui_invalid_slotid);
    if (csp->visible != b) {
      csp->visible = b;
      assume(csp->make_dirty());
    }
    return *this;
  }

  bool enabled() const { return unsafe_get(&slot::enabled); }
  auto& enabled(bool b) {
    const auto csp = system::slot_address<control>(_slotid());
    if (!csp) fatal_error(errors::ui_invalid_slotid);
    if (csp->enabled != b) {
      csp->enabled = b;
      assume(csp->make_dirty());
    }
    return *this;
  }

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
} // namespace yw::ui
