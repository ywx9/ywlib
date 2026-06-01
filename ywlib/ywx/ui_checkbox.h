#pragma once
#include "ywx/ui_icon.h"
#include "ywx/ui_label.h"
#include "ywx/ui_layer.h"
#include "ywx/ui_layout.h"

namespace yw::ui {

class checkbox : public horizontal_layout {
  static constexpr float2 default_icon_size{16.0f, 16.0f};
  static constexpr std::string_view box_path = "M 0 0 L 16 0 L 16 16 L 0 16 Z";
  static constexpr std::string_view mark_path = "M 2 8 L 6 12 L 14 4 L 6 12";

public:
  /// MARK: slot

  struct slot : public horizontal_layout::slot {
    ui::layer icon_layer;
    ui::icon box_icon;
    ui::icon check_icon;
    ui::label text;
    bool checked = false;
    bool locked = false;
    key captured_key{};

    function<void, bool> on_change;
    function<void, bool> on_focus;
    function<void, key> on_click;

    virtual bool attachable() const override { return !locked; }
    virtual std::expected<void, error_trace> attach(slotid Child) override {
      if (locked) return unexpected_error(errors::ui_not_attachable);
      if (auto res = horizontal_layout::slot::attach(Child)) return {};
      else return unexpected_error(res.error());
    }
    virtual std::expected<void, error_trace> detach(slotid Child) override {
      if (auto res = horizontal_layout::slot::detach(Child)) return {};
      else return unexpected_error(res.error());
    }

    virtual slotid hittest(float2 Pt) const override { return hittest_geometry(Pt) ? id : slotid(); }
    virtual slotid next_tab_stop(slotid Focused, bool Forward, bool& Found) const override {
      if (Focused == id) Found = true;
      else if (Found && enabled) return id;
      return {};
    }
    virtual bool focusable() const override { return enabled; }

    virtual void click_event(yw::button_event e) override {
      if (!enabled) return;
      if (e.key == captured_key && e.key == keys::lbutton) click_action();
      captured_key = {};
    }

    virtual void button_event(yw::button_event e) override {
      if (!enabled) return;
      captured_key = e.down ? e.key : key{};
    }

    virtual void focus_event(bool focused) override {
      if (!focused) captured_key = {};
      if (auto res = make_dirty(); !res) fatal_error(res.error());
      if (enabled && on_focus) on_focus(focused);
    }

    virtual bool key_event(yw::key_event e) override {
      if (!enabled) return false;
      if (e.down) captured_key = e.key;
      else if (captured_key == e.key) click_action(), captured_key = {};
      else captured_key = {};
      return e.key == keys::space || e.key == keys::enter;
    }

    void click_action() {
      switch (captured_key.code) {
      case keys::lbutton.code:
      case keys::enter.code:
      case keys::space.code:
        checked = !checked;
        if (const auto sp = system::slot_address<ui::control>(check_icon._slotid())) {
          sp->visible = checked;
          if (auto res = make_dirty(); !res) fatal_error(res.error());
        } else fatal_error(errors::ui_invalid_slotid);
        if (on_click) on_click(captured_key);
        if (on_change) on_change(checked);
        break;
      }
    }
  };

  using control::operator bool;
  checkbox() noexcept = default;

  static std::expected<checkbox, error_trace> add(derived_from<unknown> auto& Layout, bool auto_color = true) {
    checkbox ckb;
    if (auto res = create_control<checkbox>(Layout)) ckb._id = *res;
    else return unexpected_error(res.error());
    svgpath box, check;
    if (auto res = svgpath::create(default_icon_size, box_path)) box = std::move(*res);
    else return unexpected_error(res.error());
    if (auto res = svgpath::create(default_icon_size, mark_path)) check = std::move(*res);
    else return unexpected_error(res.error());
    if (const auto csp = system::slot_address<checkbox>(ckb._id)) {
      csp->locked = false;
      color bg_color = colors::white, border_color = colors::black;
      if (auto_color) tuple<>::asref(bg_color, border_color) = control::get_auto_color();
      csp->background_color = bg_color;
      if (auto res = ui::layer::add(ckb)) csp->icon_layer = std::move(*res);
      else return unexpected_error(res.error());
      csp->icon_layer.core().size(default_icon_size);
      csp->icon_layer.background().color(colors::blue);
      if (auto res = ui::icon::add(csp->icon_layer)) csp->box_icon = std::move(*res);
      else return unexpected_error(res.error());
      csp->box_icon.icon_()
        .icon(std::move(box))
        .icon_size(default_icon_size)
        .fill_color(colors::transparent)
        .stroke_color(border_color)
        .stroke_width(2.0f);
      if (auto res = ui::icon::add(csp->icon_layer)) csp->check_icon = std::move(*res);
      else return unexpected_error(res.error());
      csp->check_icon.icon_()
        .icon(std::move(check))
        .icon_size(default_icon_size)
        .fill_color(border_color)
        .stroke_color(border_color)
        .stroke_width(2.0f);
      if (const auto sp = system::slot_address<ui::control>(csp->check_icon._slotid())) sp->visible = false;
      else return unexpected_error(errors::ui_invalid_slotid);
      if (auto res = ui::label::add(ckb, false)) csp->text = std::move(*res);
      else return unexpected_error(res.error());
      csp->text.text().color(border_color);
      csp->locked = true;
    } else return unexpected_error(errors::ui_invalid_slotid);
    return ckb;
  }

  template<typename Self> auto& box_icon(this Self&& self) {
    const auto csp = system::slot_address<checkbox>(self._slotid());
    if (!csp) fatal_error(errors::ui_invalid_slotid);
    return csp->box_icon;
  }

  template<typename Self> auto& check_icon(this Self&& self) {
    const auto csp = system::slot_address<checkbox>(self._slotid());
    if (!csp) fatal_error(errors::ui_invalid_slotid);
    return csp->check_icon;
  }

  template<typename Self> auto& text(this Self&& self) {
    const auto csp = system::slot_address<checkbox>(self._slotid());
    if (!csp) fatal_error(errors::ui_invalid_slotid);
    return csp->text;
  }

  bool checked() const {
    const auto csp = system::slot_address<checkbox>(_slotid());
    if (!csp) fatal_error(errors::ui_invalid_slotid);
    return csp->checked;
  }

  auto& checked(bool Checked) {
    const auto csp = system::slot_address<checkbox>(_slotid());
    if (!csp) fatal_error(errors::ui_invalid_slotid);
    if (csp->checked != Checked) {
      csp->checked = Checked;
      if (const auto sp = system::slot_address<ui::control>(csp->check_icon._slotid())) {
        sp->visible = Checked;
        assume(csp->make_dirty());
      } else fatal_error(errors::ui_invalid_slotid);
    }
    return *this;
  }

  const auto& on_change() const { return unsafe_get(&checkbox::slot::on_change); }
  std::expected<void, error_trace> on_change(function<void, bool> f) {
    if (auto res = safe_set(&checkbox::slot::on_change, std::move(f))) return {};
    else return unexpected_error(res.error());
  }

  const auto& on_click() const { return unsafe_get(&checkbox::slot::on_click); }
  std::expected<void, error_trace> on_click(function<void, key> f) {
    if (auto res = safe_set(&checkbox::slot::on_click, std::move(f))) return {};
    else return unexpected_error(res.error());
  }

  const auto& on_focus() const { return unsafe_get(&checkbox::slot::on_focus); }
  std::expected<void, error_trace> on_focus(function<void, bool> f) {
    if (auto res = safe_set(&checkbox::slot::on_focus, std::move(f))) return {};
    else return unexpected_error(res.error());
  }
};
} // namespace yw::ui

// #include "ywx/ui_control.h"
// #include "ywx/uip_checkbox.h"
// #include "ywx/uip_text.h"

// namespace yw::ui {

// class checkbox : public control {
//   static constexpr std::string_view box_path = "M 2 2 L 18 2 L 18 18 L 2 18 Z";
//   static constexpr std::string_view mark_path = "M 4 10 L 8 14 L 16 6";

// public:
//   struct slot : public control::slot {
//     parts::background background{.color = colors::transparent};
//     parts::border border{.color = colors::transparent};
//     parts::checkbox checkbox{};
//     parts::text text{.block_alignment = alignment::left};

//     float4 padding = float4::fill(arbitrary_value);
//     float icon_offset = arbitrary_value;

//     function<void, bool> on_change;
//     function<void, bool> on_focus;
//     function<void, key> on_click;

//     key captured_key{};

//     //-- overrides --//

//     virtual bool focusable() const override { return enabled; }

//     virtual std::expected<void, error_trace> draw() override {
//       if (!visible) return {};
//       if (auto res = background.draw(core); !res) return unexpected_error(res.error());
//       const auto cb_bounds = checkbox.bounds();
//       const auto tx_bounds = text.bounds();
//       const float gap = text.string.empty() ? 0.0f : icon_offset;
//       const float2 inner = core.size - padding.xy() - padding.zw();
//       const float2 icon_origin = core.pos + padding.xy();
//       const float2 icon_area(cb_bounds.x, inner.y);
//       if (auto res = checkbox.draw(icon_origin, icon_area); !res) return unexpected_error(res.error());
//       const float2 text_origin = icon_origin + float2(cb_bounds.x + gap, 0.0f);
//       const float2 text_area(inner.x - cb_bounds.x - gap, inner.y);
//       if (auto res = text.draw(text_origin, text_area); !res) return unexpected_error(res.error());
//       if (auto res = border.draw(core); !res) return unexpected_error(res.error());
//       return {};
//     }

//     virtual float2 calculate_minimum_size() const override {
//       const auto cb_bounds = checkbox.bounds();
//       const auto tx_bounds = text.bounds();
//       const float gap = text.string.empty() ? 0.0f : icon_offset;
//       const float2 inner{cb_bounds.x + gap + tx_bounds.x, yw::max(cb_bounds.y, tx_bounds.y)};
//       return vapply_r<float2>(
//         yw::max, core.min_size, core.required_size * core.constrained, inner + padding.xy() + padding.zw());
//     }

//     virtual void ensure_minimum_size() override {
//       core.size = calculate_minimum_size();
//     }

//     virtual slotid next_tab_stop(slotid Focused, bool Forward, bool& Found) const override {
//       if (Focused == id) Found = true;
//       else if (Found && visible) return id;
//       return {};
//     }

//     virtual void click_event(events::button e) override {
//       if (enabled && e.code == captured_key) click_action();
//       captured_key = {};
//     }

//     virtual void button_event(events::button e) override {
//       if (enabled && e.down) captured_key = e.code;
//       else captured_key = {};
//     }

//     virtual void focus_event(bool focused) override {
//       if (!focused) captured_key = {};
//       if (enabled && on_focus) on_focus(focused);
//     }

//     virtual bool key_event(events::key e) override {
//       if (!enabled) return false;
//       if (!e.down) {
//         if (captured_key == e.code) click_action();
//         captured_key = {};
//       } else captured_key = e.code;
//       return e.code == keys::space || e.code == keys::enter;
//     }

//     void click_action() {
//       switch (captured_key.code) {
//       case keys::lbutton.code:
//       case keys::enter.code:
//       case keys::space.code:
//         checkbox.checked = !checkbox.checked;
//         assume(make_dirty());
//         if (on_click) on_click(captured_key);
//         if (on_change) on_change(checkbox.checked);
//       }
//     }
//   };

//   using control::operator bool;
//   checkbox() noexcept = default;

//   static std::expected<checkbox, error_trace> add(derived_from<unknown> auto& Layout) {
//     checkbox cb;
//     if (auto res = create_control<checkbox>(Layout)) cb._id = *res;
//     else return unexpected_error(res.error());
//     if (const auto csp = system::slot_address<checkbox>(cb._id)) {
//       csp->background.control_id = cb._id;
//       csp->border.control_id = cb._id;
//       csp->checkbox.control_id = cb._id;
//       csp->checkbox.box.data = assume(svgpath::create(parts::icon::default_size, box_path));
//       csp->checkbox.check.data = assume(svgpath::create(parts::icon::default_size, mark_path));
//       csp->text.control_id = cb._id;
//     } else return unexpected_error(errors::ui_invalid_slotid);
//     return cb;
//   }

//   template<typename Self> decltype(auto) background(this Self& self) {
//     const auto csp = system::slot_address<checkbox>(self._id);
//     if (!csp) fatal_error(errors::ui_invalid_slotid);
//     if constexpr (!is_const<Self>) return csp->background.access();
//     else return std::as_const(csp->background).access();
//   }
//   template<typename Self> decltype(auto) border(this Self& self) {
//     const auto csp = system::slot_address<checkbox>(self._id);
//     if (!csp) fatal_error(errors::ui_invalid_slotid);
//     if constexpr (!is_const<Self>) return csp->border.access();
//     else return std::as_const(csp->border).access();
//   }
//   template<typename Self> decltype(auto) icon(this Self& self) {
//     const auto csp = system::slot_address<checkbox>(self._id);
//     if (!csp) fatal_error(errors::ui_invalid_slotid);
//     if constexpr (!is_const<Self>) return csp->checkbox.access();
//     else return std::as_const(csp->checkbox).access();
//   }
//   template<typename Self> decltype(auto) text(this Self& self) {
//     const auto csp = system::slot_address<checkbox>(self._id);
//     if (!csp) fatal_error(errors::ui_invalid_slotid);
//     if constexpr (!is_const<Self>) return csp->text.access();
//     else return std::as_const(csp->text).access();
//   }

//   const auto& checked() const {
//     const auto csp = system::slot_address<checkbox>(_id);
//     if (!csp) fatal_error(errors::ui_invalid_slotid);
//     return csp->checkbox.checked;
//   }
//   auto& checked(bool Checked) {
//     const auto csp = system::slot_address<checkbox>(_id);
//     if (!csp) fatal_error(errors::ui_invalid_slotid);
//     if (csp->checkbox.checked != Checked) {
//       csp->checkbox.checked = Checked;
//       assume(csp->make_dirty());
//     }
//     return *this;
//   }

//   float icon_offset() const {
//     const auto csp = system::slot_address<checkbox>(_id);
//     if (!csp) fatal_error(errors::ui_invalid_slotid);
//     return csp->icon_offset;
//   }

//   auto& icon_offset(float1 Offset) {
//     const auto csp = system::slot_address<checkbox>(_id);
//     if (!csp) fatal_error(errors::ui_invalid_slotid);
//     csp->icon_offset = yw::max(0.0f, Offset.x);
//     assume(csp->make_messy());
//     return *this;
//   }

//   const auto& padding() const {
//     const auto csp = system::slot_address<checkbox>(_id);
//     if (!csp) fatal_error(errors::ui_invalid_slotid);
//     return csp->padding;
//   }

//   auto& padding(float4 Padding) {
//     const auto csp = system::slot_address<checkbox>(_id);
//     if (!csp) fatal_error(errors::ui_invalid_slotid);
//     csp->padding = Padding;
//     assume(csp->make_messy());
//     return *this;
//   }

//   const auto& on_change() const { return unsafe_get(&checkbox::slot::on_change); }
//   void on_change(function<void, bool> f) { unsafe_set(&checkbox::slot::on_change, std::move(f)); }

//   const auto& on_click() const { return unsafe_get(&checkbox::slot::on_click); }
//   void on_click(function<void, key> f) { unsafe_set(&checkbox::slot::on_click, std::move(f)); }

//   const auto& on_focus() const { return unsafe_get(&checkbox::slot::on_focus); }
//   void on_focus(function<void, bool> f) { unsafe_set(&checkbox::slot::on_focus, std::move(f)); }
// };
// } // namespace yw::ui
