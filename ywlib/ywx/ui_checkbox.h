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

  static std::expected<checkbox, error_trace> add(
    derived_from<unknown> auto& Layout, const color_pair& Colors = color_pair::auto_color(), bool default_icon = true) {
    checkbox ckb;
    if (auto res = create_control<checkbox>(Layout)) ckb._id = *res;
    else return unexpected_error(res.error());
    svgpath box, check;
    if (default_icon) {
      if (auto res = svgpath::create(default_icon_size, box_path)) box = std::move(*res);
      else return unexpected_error(res.error());
      if (auto res = svgpath::create(default_icon_size, mark_path)) check = std::move(*res);
      else return unexpected_error(res.error());
    }
    if (const auto csp = system::slot_address<checkbox>(ckb._id)) {
      csp->locked = false;
      csp->background_color = Colors.background;
      if (auto res = ui::layer::add(ckb)) csp->icon_layer = std::move(*res);
      else return unexpected_error(res.error());
      csp->icon_layer.core().padding({}).fit_content(true).crop_content(false);
      if (auto res = ui::icon::add(csp->icon_layer, Colors)) csp->box_icon = std::move(*res);
      else return unexpected_error(res.error());
      csp->box_icon.core().fit_content(true);
      csp->box_icon.icon_().icon(std::move(box)).stroke_width(2.0f);
      if (auto res = ui::icon::add(csp->icon_layer, Colors)) csp->check_icon = std::move(*res);
      else return unexpected_error(res.error());
      csp->check_icon.core().fit_content(true);
      csp->check_icon.icon_().icon(std::move(check)).stroke_width(2.0f);
      csp->check_icon.visible(false);
      if (auto res = ui::label::add(ckb)) csp->text = std::move(*res);
      else return unexpected_error(res.error());
      csp->text.text().color(Colors.foreground);
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
