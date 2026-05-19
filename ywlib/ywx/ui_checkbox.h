#pragma once
#include "ywx/ui_control.h"
#include "ywx/uip_icon.h"
#include "ywx/uip_text.h"

namespace yw::ui {

class checkbox : public control {
  static constexpr float2 default_icon_size = {20.0f, 20.0f};
  static constexpr std::string_view box_path = "M 2 2 L 18 2 L 18 18 L 2 18 Z";
  static constexpr std::string_view mark_path = "M 4 10 L 8 14 L 16 6";

public:
  struct slot : public control::slot {
    parts::background background{.color = colors::transparent};
    parts::border border{.color = colors::transparent};
    parts::icon box{
      .data = assume(svgpath::create(default_icon_size, box_path)),
      .fill_color = colors::white,
      .size = default_icon_size};
    parts::icon mark{
      .data = assume(svgpath::create(default_icon_size, mark_path)), .stroke_width = 2.0f, .size = default_icon_size};
    parts::text text{.block_alignment = alignment::left};

    float4 padding = float4::fill(arbitrary_value);
    float icon_offset = arbitrary_value;
    bool checked = false;

    function<void, bool> on_change;
    function<void, bool> on_focus;
    function<void, key> on_click;

    key captured_key{};

    float2 icon_bounds() const noexcept { return vapply_r<float2>(yw::max, box.bounds(), mark.bounds()); }

    //-- overrides --//

    virtual bool focusable() const override { return enabled; }

    virtual std::expected<void, error_trace> draw() override {
      if (!visible) return {};
      brush.color(background.color);
      fill_geometry(core.geometry.get());
      d2d.push_layer(core.geometry.get());
      if (background.image) draw_bitmap(core.pos, core.size, background.image, background.image_opacity);
      const auto isz = icon_bounds();
      const auto tsz = text.bounds();
      const float gap = text.string.empty() ? 0.0f : icon_offset;
      const float2 inner = core.size - padding.xy() - padding.zw();
      const float2 icon_origin = core.pos + padding.xy();
      const float2 icon_area(isz.x, inner.y);
      if (auto res = box.draw(icon_origin, icon_area); !res) return unexpected_error(res.error());
      if (checked)
        if (auto res = mark.draw(icon_origin, icon_area); !res) return unexpected_error(res.error());
      const float2 text_origin = icon_origin + float2(isz.x + gap, 0.0f);
      const float2 text_area(inner.x - isz.x - gap, inner.y);
      if (auto res = text.draw(text_origin, text_area); !res) return unexpected_error(res.error());
      d2d.pop_layer();
      border.draw(core.geometry.get());
      return {};
    }

    virtual void ensure_minimum_size() override {
      const auto isz = icon_bounds();
      const auto tsz = text.bounds();
      const float gap = text.string.empty() ? 0.0f : icon_offset;
      const float2 inner = {isz.x + gap + tsz.x, yw::max(isz.y, tsz.y)};
      core.size = vapply_r<float2>(
        yw::max, core.min_size, core.required_size * core.constrained, inner + padding.xy() + padding.zw());
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
        checked = !checked;
        assume(make_dirty());
        if (on_click) on_click(captured_key);
        if (on_change) on_change(checked);
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
      csp->box.control_id = cb._id;
      csp->mark.control_id = cb._id;
      csp->text.control_id = cb._id;
    } else return unexpected_error(errors::ui_invalid_slotid);
    return cb;
  }

  auto background() {
    const auto csp = system::slot_address<checkbox>(_id);
    if (!csp) fatal_error(errors::ui_invalid_slotid);
    return csp->background.handle();
  }

  const auto background() const {
    const auto csp = system::slot_address<checkbox>(_id);
    if (!csp) fatal_error(errors::ui_invalid_slotid);
    return csp->background.handle();
  }

  auto border() {
    const auto csp = system::slot_address<checkbox>(_id);
    if (!csp) fatal_error(errors::ui_invalid_slotid);
    return csp->border.handle();
  }

  const auto border() const {
    const auto csp = system::slot_address<checkbox>(_id);
    if (!csp) fatal_error(errors::ui_invalid_slotid);
    return csp->border.handle();
  }

  auto box() {
    const auto csp = system::slot_address<checkbox>(_id);
    if (!csp) fatal_error(errors::ui_invalid_slotid);
    return csp->box.handle();
  }

  const auto box() const {
    const auto csp = system::slot_address<checkbox>(_id);
    if (!csp) fatal_error(errors::ui_invalid_slotid);
    return csp->box.handle();
  }

  auto mark() {
    const auto csp = system::slot_address<checkbox>(_id);
    if (!csp) fatal_error(errors::ui_invalid_slotid);
    return csp->mark.handle();
  }

  const auto mark() const {
    const auto csp = system::slot_address<checkbox>(_id);
    if (!csp) fatal_error(errors::ui_invalid_slotid);
    return csp->mark.handle();
  }

  auto text() {
    const auto csp = system::slot_address<checkbox>(_id);
    if (!csp) fatal_error(errors::ui_invalid_slotid);
    return csp->text.handle();
  }

  const auto text() const {
    const auto csp = system::slot_address<checkbox>(_id);
    if (!csp) fatal_error(errors::ui_invalid_slotid);
    return csp->text.handle();
  }

  const auto& checked() const {
    const auto csp = system::slot_address<checkbox>(_id);
    if (!csp) fatal_error(errors::ui_invalid_slotid);
    return csp->checked;
  }

  auto& checked(bool Checked) {
    const auto csp = system::slot_address<checkbox>(_id);
    if (!csp) fatal_error(errors::ui_invalid_slotid);
    if (csp->checked != Checked) {
      csp->checked = Checked;
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
