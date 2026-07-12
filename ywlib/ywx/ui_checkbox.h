#pragma once
#include <ywx/bitmap.h>
#include <ywx/drawing.h>
#include <ywx/ui_frame.h>
#include <ywx/ui_geometry.h>
#include <ywx/ui_image.h>
#include <ywx/ui_label.h>
#include <ywx/ui_layer.h>
#include <ywx/ui_layout.h>

namespace yw::ui {

class checkbox : public frame {
public:
  enum class content_kind : unsigned char { none, image, geometry };

  struct slot : frame::slot {
    horizontal_layout content{};
    layer box_layer{};
    image box_image{};
    yw::ui::geometry box_geometry{};
    image check_image{};
    yw::ui::geometry check_geometry{};
    label caption{};

    function<void, bool> on_change{};
    slotid content_id{};
    content_kind box_kind = content_kind::geometry;
    content_kind check_kind = content_kind::geometry;
    bool attach_lock = true;
    bool checked = false;
    bool pressed = false;

    virtual bool attachable() const override { return !attach_lock && !content_id; }

    virtual std::expected<void, error> attach(slotid Child) override {
      if (attach_lock || content_id) return std::unexpected(error(errors::invalid_operation, "not attachable"));
      const auto csp = interface::slot::get<control>(Child);
      if (!csp) return std::unexpected(error(errors::invalid_slotid));
      csp->layout_id = id;
      csp->window_id = window_id;
      content_id = Child;
      make_messy();
      return {};
    }

    virtual std::expected<void, error> detach(slotid Child) override {
      if (content_id != Child) return std::unexpected(error(errors::invalid_operation, "not attached to this control"));
      if (auto res = interface::slot::slots.erase(Child); !res) return res.error().relay();
      content_id = {};
      make_messy();
      return {};
    }

    void update_part_visibility() {
      box_image.visible(box_kind == content_kind::image);
      box_geometry.visible(box_kind == content_kind::geometry);
      check_image.visible(checked && check_kind == content_kind::image);
      check_geometry.visible(checked && check_kind == content_kind::geometry);
    }

    std::expected<void, error> initialize(checkbox& Self) {
      attach_lock = false;
      content = horizontal_layout(Self);
      attach_lock = true;

      box_layer = layer(content);
      box_image = image(box_layer);
      box_geometry = yw::ui::geometry(box_layer);
      check_image = image(box_layer);
      check_geometry = yw::ui::geometry(box_layer);
      caption = label(content, false);

      const auto border = colors.border;
      box_layer.size(float2::fill(16.0f));
      box_geometry.content_mode(yw::ui::geometry::geometry_size_mode::stretch)
        .content(svgpath(float2::fill(16.0f), "M1 1 L15 1 L15 15 L1 15 Z"))
        .fill_color(colors::transparent)
        .stroke_color(border)
        .stroke_width(1.5f);
      check_geometry.content_mode(yw::ui::geometry::geometry_size_mode::stretch)
        .content(svgpath(float2::fill(16.0f), "M3 8 L7 12 L13 4"))
        .fill_color(border)
        .stroke_color(border)
        .stroke_width(2.0f);
      caption.text_color(border).background_color(colors::transparent);
      caption.text_align(alignment::left);
      colors.border = colors::transparent;
      update_part_visibility();
      return {};
    }

    virtual bool focusable() const override { return enabled && visible; }

    virtual std::expected<float2, error> get_necessary_size() const override {
      const auto csp = interface::slot::get<control>(content_id);
      if (!csp) return vapply_r<float2>(_necessary_size, policy, minimum_size, required_size, float2{});
      if (auto res = csp->get_necessary_size())
        return vapply_r<float2>(_necessary_size, policy, minimum_size, required_size, *res);
      else return res.error().relay();
    }

    virtual slotid hittest(float2 Pt) const override { return control::slot::hittest(Pt) ? id : slotid{}; }

    virtual std::expected<void, error> redraw() override {
      if (geometry_dirty) {
        geometry_dirty = false;
        if (auto res = relocate(); !res) return res.error().relay();
      }
      if (!visible) return {};
      if (auto res = _draw_background(); !res) return res.error().relay();
      const auto csp = interface::slot::get<control>(content_id);
      if (csp)
        if (auto res = csp->redraw(); !res) return res.error().relay();
      if (auto res = _draw_foreground(); !res) return res.error().relay();
      return {};
    }

    virtual std::expected<void, error> relocate() override {
      const auto max_size = provided_area - margin.xy() - margin.zw();
      if (auto res = set_size_to_necessary(); !res) return res.error().relay();
      if (policy.x == size_policy::free) size.x = max_size.x;
      if (policy.y == size_policy::free) size.y = max_size.y;
      pos = provided_pos + _offset(max_size);
      ID2D1RoundedRectangleGeometry* geom = nullptr;
      D2D1_ROUNDED_RECT rr{D2D1::RectF(pos.x, pos.y, pos.x + size.x, pos.y + size.y), radius.x, radius.y};
      hresult_test(d2d::factory()->CreateRoundedRectangleGeometry, &rr, &geom);
      geometry.reset(geom);

      const auto csp = interface::slot::get<control>(content_id);
      if (!csp) return {};
      if (auto res = csp->relocate(pos, size); !res) return res.error().relay();
      return {};
    }

    virtual std::expected<void, error> set_size_to_necessary() override {
      if (auto res = get_necessary_size()) size = *res;
      else return res.error().relay();
      return {};
    }

    virtual std::expected<void, error> reset_state() override {
      if (!pressed) return {};
      pressed = false;
      make_dirty();
      return {};
    }

    virtual bool button_event(yw::button_event e) override {
      if (!enabled || !visible || e.key != keys::lbutton) return false;
      const bool next_pressed = e.down;
      if (pressed == next_pressed) return true;
      pressed = next_pressed;
      make_dirty();
      return true;
    }

    virtual bool click_event(yw::button_event e) override {
      if (!enabled || !visible || e.down || e.key != keys::lbutton) return false;
      checked = !checked;
      update_part_visibility();
      make_messy();
      if (on_change) on_change(checked);
      return true;
    }

    virtual void focus_event(bool Focused) override {
      if (!Focused && pressed) {
        pressed = false;
        make_dirty();
      }
    }

    virtual bool key_event(yw::key_event e) override {
      if (!enabled || !visible) return false;
      if (e.key != keys::space && e.key != keys::enter) return false;
      if (e.down) {
        if (!pressed) {
          pressed = true;
          make_dirty();
        }
        return true;
      }
      const bool was_pressed = pressed;
      pressed = false;
      if (was_pressed) {
        checked = !checked;
        update_part_visibility();
        make_messy();
        if (on_change) on_change(checked);
      }
      return true;
    }

    template<derived_from<checkbox> H, derived_from<interface> L>
    static std::expected<typename H::slot*, error> create(L& Parent, bool AutoColor, const source_line& sl) {
      return frame::slot::create<H>(Parent, AutoColor, sl);
    }
  };

  using frame::operator bool;
  checkbox() noexcept = default;

  checkbox(derived_from<interface> auto& Parent, bool AutoColor = true, const source_line& sl = here()) {
    if (auto res = create(Parent, AutoColor, sl)) *this = std::move(*res);
    else res.error().add_footprint().go_off(sl);
  }

  static std::expected<checkbox, error> create(
    derived_from<interface> auto& Parent, bool AutoColor = true, const source_line& sl = here()) {
    checkbox c;
    if (auto res = slot::create<checkbox>(Parent, AutoColor, sl)) {
      const auto sp = *res;
      c._id = sp->id;
      if (auto ires = sp->initialize(c); !ires) return ires.error().relay();
      return c;
    } else return res.error().relay();
  }

  bool checked() const noexcept { ywlib_control_get(checked); }
  bool pressed() const noexcept { ywlib_control_get(pressed); }
  const auto& on_change() const noexcept { ywlib_control_get(on_change); }
  const auto& string() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->caption.string();
  }

  auto& checked(this auto& self, bool b) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    if (sp->checked == b) return self;
    sp->checked = b;
    sp->update_part_visibility();
    sp->make_messy();
    if (sp->on_change) sp->on_change(sp->checked);
    return self;
  }

  auto& on_change(this auto& self, function<void, bool> f) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->on_change = std::move(f);
    return self;
  }

  auto& string(this auto& self, yw::string<wchar_t> s) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->caption.string(std::move(s));
    return self;
  }

  auto& font(this auto& self, font_config f) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->caption.font(std::move(f));
    return self;
  }

  auto& text_color(this auto& self, const color& c) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->caption.text_color(c);
    return self;
  }

  auto& box_size(this auto& self, float2 v) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->box_layer.size(v);
    sp->box_image.content_size(v);
    sp->box_geometry.size(v);
    sp->check_image.content_size(v);
    sp->check_geometry.size(v);
    return self;
  }

  auto& box(this auto& self, bitmap b) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->box_image.content(std::move(b));
    sp->box_kind = content_kind::image;
    sp->update_part_visibility();
    return self;
  }

  auto& box(this auto& self, svgpath p) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->box_geometry.content(std::move(p));
    sp->box_kind = content_kind::geometry;
    sp->update_part_visibility();
    return self;
  }

  auto& check(this auto& self, bitmap b) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->check_image.content(std::move(b));
    sp->check_kind = content_kind::image;
    sp->update_part_visibility();
    return self;
  }

  auto& check(this auto& self, svgpath p) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->check_geometry.content(std::move(p));
    sp->check_kind = content_kind::geometry;
    sp->update_part_visibility();
    return self;
  }

  auto& box_fill_color(this auto& self, const color& c) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->box_geometry.fill_color(c);
    return self;
  }

  auto& box_stroke_color(this auto& self, const color& c) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->box_geometry.stroke_color(c);
    return self;
  }

  auto& check_fill_color(this auto& self, const color& c) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->check_geometry.fill_color(c);
    return self;
  }

  auto& check_stroke_color(this auto& self, const color& c) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->check_geometry.stroke_color(c);
    return self;
  }
};
} // namespace yw::ui
