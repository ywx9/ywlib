#pragma once
#include "ywx/ui_control.h"
#include "ywx/uip_checkbox.h"
#include "ywx/uip_text.h"

namespace yw::ui {

class radiobutton : public control {
  static constexpr float2 default_icon_size = {20.0f, 20.0f};
  static constexpr std::string_view box_path = "M 10 2 A 8 8 0 1 0 10 18 A 8 8 0 1 0 10 2";
  static constexpr std::string_view mark_path = "M 10 6 A 4 4 0 1 0 10 14 A 4 4 0 1 0 10 6";
public:
  struct slot : public control::slot {
    static constexpr size_t invalid_index = size_t(-1);

    parts::background background;
    parts::border border;
    parts::checkbox checkbox;
    std::vector<parts::text> items;

    float4 padding = float4::fill(arbitrary_value);
    float icon_offset = arbitrary_value;
    float item_offset = arbitrary_value;
    size_t selected_index = invalid_index;
    size_t focused_index = invalid_index;

    function<void, size_t> on_change;
    function<void, bool> on_focus;
    function<void, key> on_click;

    key captured_key{};

    size_t normalize_focus() const noexcept {
      if (items.empty()) return invalid_index;
      if (focused_index < items.size()) return focused_index;
      if (selected_index < items.size()) return selected_index;
      return 0;
    }

    float row_height(size_t Index) const noexcept {
      if (Index >= items.size()) return 0.0f;
      return yw::max(checkbox.bounds().y, items[Index].bounds().y);
    }

    size_t hit_item(float2 Point) const noexcept {
      if (!core.hittest(Point)) return invalid_index;
      float y = core.pos.y + padding.y;
      for (size_t i = 0; i < items.size(); ++i) {
        const auto h = row_height(i);
        if (Point.y >= y && Point.y <= y + h) return i;
        y += h + item_offset;
      }
      return invalid_index;
    }

    void move_focus(int Delta) {
      const auto base = normalize_focus();
      if (base == invalid_index) return;
      const auto n = int(items.size());
      const auto index = int(base);
      focused_index = size_t((index + Delta + n) % n);
      assume(make_dirty());
    }

    void select_index(size_t Index) {
      if (Index >= items.size()) return;
      const bool changed = selected_index != Index;
      selected_index = Index;
      focused_index = Index;
      assume(make_dirty());
      if (on_click) on_click(captured_key);
      if (changed && on_change) on_change(Index);
    }

    //-- overrides --//

    virtual bool focusable() const override { return enabled; }

    virtual std::expected<void, error_trace> draw() override {
      if (!visible) return {};
      if (auto res = background.draw(core); !res) return unexpected_error(res.error());
      const auto cb_bounds = checkbox.bounds();
      const float text_width = core.size.x - padding.x - padding.z - cb_bounds.x - icon_offset;
      float2 offset = core.pos + padding.xy();
      for (size_t i = 0; i < items.size(); ++i) {
        auto& item = items[i];
        const auto height = yw::max(cb_bounds.y, item.bounds().y);
        const auto text_pos = offset + float2(cb_bounds.x + icon_offset, 0);
        checkbox.checked = i == selected_index;
        if (auto res = checkbox.draw(offset, float2(cb_bounds.x, height)); !res) return unexpected_error(res.error());
        if (auto res = item.draw(text_pos, float2(text_width, height)); !res) return unexpected_error(res.error());
        offset.y += height + item_offset;
      }
      if (auto res = border.draw(core); !res) return unexpected_error(res.error());
      return {};
    }

    virtual float2 calculate_minimum_size() const override {
      const auto cb_bounds = checkbox.bounds();
      float2 inner = padding.xy() + padding.zw();
      for (size_t i = 0; i < items.size(); ++i) {
        const auto tx_bounds = items[i].bounds();
        inner.y += yw::max(cb_bounds.y, tx_bounds.y);
        inner.x = yw::max(inner.x, padding.x + cb_bounds.x + icon_offset + tx_bounds.x + padding.z);
      }
      inner.y += (items.size() - 1) * item_offset;
      return vapply_r<float2>(yw::max, core.min_size, core.required_size * core.constrained, inner);
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
      if (enabled && e.code == captured_key && e.code == keys::lbutton) click_action(hit_item(float2(e.pos)));
      captured_key = {};
    }

    virtual void button_event(events::button e) override {
      if (enabled && e.down) captured_key = e.code;
      else captured_key = {};
    }

    virtual void focus_event(bool focused) override {
      if (focused) focused_index = normalize_focus();
      else
        captured_key = {}, focused_index = invalid_index;
      assume(make_dirty());
      if (enabled && on_focus) on_focus(focused);
    }

    virtual bool key_event(events::key e) override {
      if (!enabled) return false;
      if (e.down) {
        switch (e.code.code) {
        case keys::up.code:
          captured_key = {};
          move_focus(-1);
          return true;
        case keys::down.code:
          captured_key = {};
          move_focus(1);
          return true;
        case keys::space.code:
        case keys::enter.code:
          captured_key = e.code;
          return true;
        default:
          captured_key = {};
          return false;
        }
      }
      if (captured_key == e.code) click_action(normalize_focus());
      captured_key = {};
      return e.code == keys::space || e.code == keys::enter;
    }

    void click_action(size_t NewIndex) {
      switch (captured_key.code) {
      case keys::lbutton.code:
      case keys::enter.code:
      case keys::space.code:
        select_index(NewIndex);
      }
    }
  };

  using control::operator bool;
  radiobutton() noexcept = default;

  static std::expected<radiobutton, error_trace> add(derived_from<unknown> auto& Layout) {
    radiobutton rb;
    if (auto res = create_control<radiobutton>(Layout, rb)) rb._id = *res;
    else return unexpected_error(res.error());
    if (const auto csp = system::slot_address<radiobutton>(rb._id)) {
      csp->background.control_id = rb._id;
      csp->border.control_id = rb._id;
      csp->checkbox.control_id = rb._id;
      csp->checkbox.box.data = assume(svgpath::create(parts::icon::default_size, box_path));
      csp->checkbox.check.data = assume(svgpath::create(parts::icon::default_size, mark_path));
    } else return unexpected_error(errors::ui_invalid_slotid);
    return rb;
  }

  template<typename Self> decltype(auto) background(this Self& self) {
    const auto csp = system::slot_address<radiobutton>(self._id);
    if (!csp) fatal_error(errors::ui_invalid_slotid);
    if constexpr (!is_const<Self>) return csp->background.access();
    else return std::as_const(csp->background).access();
  }
  template<typename Self> decltype(auto) border(this Self& self) {
    const auto csp = system::slot_address<radiobutton>(self._id);
    if (!csp) fatal_error(errors::ui_invalid_slotid);
    if constexpr (!is_const<Self>) return csp->border.access();
    else return std::as_const(csp->border).access();
  }
  template<typename Self> decltype(auto) icon(this Self& self) {
    const auto csp = system::slot_address<radiobutton>(self._id);
    if (!csp) fatal_error(errors::ui_invalid_slotid);
    if constexpr (!is_const<Self>) return csp->checkbox.access();
    else return std::as_const(csp->checkbox).access();
  }
};
}
