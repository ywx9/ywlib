#pragma once
#include "ywx/ui_control.h"
#include "ywx/uip_checkbox.h"
#include "ywx/uip_text.h"

namespace yw::ui {

class checkbox : public control {
  static constexpr std::string_view box_path = "M 2 2 L 18 2 L 18 18 L 2 18 Z";
  static constexpr std::string_view mark_path = "M 4 10 L 8 14 L 16 6";

public:
  struct slot : public control::slot {
    parts::background background{.color = colors::transparent};
    parts::border border{.color = colors::transparent};
    parts::checkbox checkbox{};
    parts::text text{.block_alignment = alignment::left};

    float4 padding = float4::fill(arbitrary_value);
    float icon_offset = arbitrary_value;

    function<void, bool> on_change;
    function<void, bool> on_focus;
    function<void, key> on_click;

    key captured_key{};

    //-- overrides --//

    virtual bool focusable() const override { return enabled; }

    virtual std::expected<void, error_trace> draw() override {
      if (!visible) return {};
      if (auto res = background.draw(core); !res) return unexpected_error(res.error());
      const auto cb_bounds = checkbox.bounds();
      const auto tx_bounds = text.bounds();
      const float gap = text.string.empty() ? 0.0f : icon_offset;
      const float2 inner = core.size - padding.xy() - padding.zw();
      const float2 icon_origin = core.pos + padding.xy();
      const float2 icon_area(cb_bounds.x, inner.y);
      if (auto res = checkbox.draw(icon_origin, icon_area); !res) return unexpected_error(res.error());
      const float2 text_origin = icon_origin + float2(cb_bounds.x + gap, 0.0f);
      const float2 text_area(inner.x - cb_bounds.x - gap, inner.y);
      if (auto res = text.draw(text_origin, text_area); !res) return unexpected_error(res.error());
      if (auto res = border.draw(core); !res) return unexpected_error(res.error());
      return {};
    }

    virtual float2 calculate_minimum_size() const override {
      const auto cb_bounds = checkbox.bounds();
      const auto tx_bounds = text.bounds();
      const float gap = text.string.empty() ? 0.0f : icon_offset;
      const float2 inner{cb_bounds.x + gap + tx_bounds.x, yw::max(cb_bounds.y, tx_bounds.y)};
      return vapply_r<float2>(
        yw::max, core.min_size, core.required_size * core.constrained, inner + padding.xy() + padding.zw());
    }

    virtual void ensure_minimum_size() override {
      core.size = calculate_minimum_size();
    }

    virtual slotid next_tab_stop(slotid Focused, bool Forward, bool& Found) const override {
      if (Focused == id) Found = true;
      else if (Found && visible) return id;
      return {};
    }

    virtual void click_event(events::button e) override {
      if (enabled && e.code == captured_key) click_action();
      captured_key = {};
    }

    virtual void button_event(events::button e) override {
      if (enabled && e.down) captured_key = e.code;
      else captured_key = {};
    }

    virtual void focus_event(bool focused) override {
      if (!focused) captured_key = {};
      if (enabled && on_focus) on_focus(focused);
    }

    virtual bool key_event(events::key e) override {
      if (!enabled) return false;
      if (!e.down) {
        if (captured_key == e.code) click_action();
        captured_key = {};
      } else captured_key = e.code;
      return e.code == keys::space || e.code == keys::enter;
    }

    void click_action() {
      switch (captured_key.code) {
      case keys::lbutton.code:
      case keys::enter.code:
      case keys::space.code:
        checkbox.checked = !checkbox.checked;
        assume(make_dirty());
        if (on_click) on_click(captured_key);
        if (on_change) on_change(checkbox.checked);
      }
    }
  };

  using control::operator bool;
  checkbox() noexcept = default;

  static std::expected<checkbox, error_trace> add(derived_from<unknown> auto& Layout) {
    checkbox cb;
    if (auto res = create_control<checkbox>(Layout)) cb._id = *res;
    else return unexpected_error(res.error());
    if (const auto csp = system::slot_address<checkbox>(cb._id)) {
      csp->background.control_id = cb._id;
      csp->border.control_id = cb._id;
      csp->checkbox.control_id = cb._id;
      csp->checkbox.box.data = assume(svgpath::create(parts::icon::default_size, box_path));
      csp->checkbox.check.data = assume(svgpath::create(parts::icon::default_size, mark_path));
      csp->text.control_id = cb._id;
    } else return unexpected_error(errors::ui_invalid_slotid);
    return cb;
  }

  template<typename Self> decltype(auto) background(this Self& self) {
    const auto csp = system::slot_address<checkbox>(self._id);
    if (!csp) fatal_error(errors::ui_invalid_slotid);
    if constexpr (!is_const<Self>) return csp->background.access();
    else return std::as_const(csp->background).access();
  }
  template<typename Self> decltype(auto) border(this Self& self) {
    const auto csp = system::slot_address<checkbox>(self._id);
    if (!csp) fatal_error(errors::ui_invalid_slotid);
    if constexpr (!is_const<Self>) return csp->border.access();
    else return std::as_const(csp->border).access();
  }
  template<typename Self> decltype(auto) icon(this Self& self) {
    const auto csp = system::slot_address<checkbox>(self._id);
    if (!csp) fatal_error(errors::ui_invalid_slotid);
    if constexpr (!is_const<Self>) return csp->checkbox.access();
    else return std::as_const(csp->checkbox).access();
  }
  template<typename Self> decltype(auto) text(this Self& self) {
    const auto csp = system::slot_address<checkbox>(self._id);
    if (!csp) fatal_error(errors::ui_invalid_slotid);
    if constexpr (!is_const<Self>) return csp->text.access();
    else return std::as_const(csp->text).access();
  }

  const auto& checked() const {
    const auto csp = system::slot_address<checkbox>(_id);
    if (!csp) fatal_error(errors::ui_invalid_slotid);
    return csp->checkbox.checked;
  }
  auto& checked(bool Checked) {
    const auto csp = system::slot_address<checkbox>(_id);
    if (!csp) fatal_error(errors::ui_invalid_slotid);
    if (csp->checkbox.checked != Checked) {
      csp->checkbox.checked = Checked;
      assume(csp->make_dirty());
    }
    return *this;
  }

  float icon_offset() const {
    const auto csp = system::slot_address<checkbox>(_id);
    if (!csp) fatal_error(errors::ui_invalid_slotid);
    return csp->icon_offset;
  }

  auto& icon_offset(float1 Offset) {
    const auto csp = system::slot_address<checkbox>(_id);
    if (!csp) fatal_error(errors::ui_invalid_slotid);
    csp->icon_offset = yw::max(0.0f, Offset.x);
    assume(csp->make_messy());
    return *this;
  }

  const auto& padding() const {
    const auto csp = system::slot_address<checkbox>(_id);
    if (!csp) fatal_error(errors::ui_invalid_slotid);
    return csp->padding;
  }

  auto& padding(float4 Padding) {
    const auto csp = system::slot_address<checkbox>(_id);
    if (!csp) fatal_error(errors::ui_invalid_slotid);
    csp->padding = Padding;
    assume(csp->make_messy());
    return *this;
  }

  const auto& on_change() const { return unsafe_get(&checkbox::slot::on_change); }
  void on_change(function<void, bool> f) { unsafe_set(&checkbox::slot::on_change, std::move(f)); }

  const auto& on_click() const { return unsafe_get(&checkbox::slot::on_click); }
  void on_click(function<void, key> f) { unsafe_set(&checkbox::slot::on_click, std::move(f)); }

  const auto& on_focus() const { return unsafe_get(&checkbox::slot::on_focus); }
  void on_focus(function<void, bool> f) { unsafe_set(&checkbox::slot::on_focus, std::move(f)); }
};
} // namespace yw::ui
