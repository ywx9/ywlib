#pragma once
#include <ywx/bitmap.h>
#include <ywx/svgpath.h>
#include <ywx/ui_blank.h>
#include <ywx/ui_label.h>
#include <ywx/ui_layout.h>

namespace yw::ui {

class checkbox : public frame {
public:
  enum class content_kind : unsigned char { none, image, geometry };

  struct slot : frame::slot {
    horizontal_layout content{};
    blank icon_spacer{};
    label caption{};
    bitmap box_bitmap{};
    bitmap check_bitmap{};
    svgpath box_svg{};
    svgpath check_svg{};
    color box_fill_color = colors::transparent;
    color box_stroke_color = colors::black;
    float box_stroke_width = 1.0f;
    color check_fill_color = colors::black;
    color check_stroke_color = colors::black;
    float check_stroke_width = 1.0f;
    content_kind box_kind = content_kind::geometry;
    content_kind check_kind = content_kind::geometry;
    float icon_gap = arbitrary_value;
    bool attach_lock = true;
    bool checked = false;
    bool pressed = false;

    function<void, bool> on_change{};

    void update_icon_margin() {
      if (icon_spacer) icon_spacer.margin(float4(arbitrary_value, arbitrary_value, arbitrary_value + icon_gap, arbitrary_value));
    }

    std::expected<void, error> draw_icon() const {
      const auto isp = get_slot<control>(icon_spacer.id());
      if (!isp || !isp->visible) return {};
      const auto pos = isp->pos();
      const auto size = isp->size();
      if (box_kind == content_kind::image) {
        if (auto res = draw_bitmap(pos, size, box_bitmap); !res) return res.error().relay();
      } else if (box_kind == content_kind::geometry) {
        if (box_fill_color.a > 0.0f) {
          brush::color(box_fill_color);
          if (auto res = fill_svgpath(pos, size, box_svg); !res) return res.error().relay();
        }
        if (box_stroke_color.a > 0.0f && box_stroke_width > 0.0f) {
          brush::color(box_stroke_color);
          if (auto res = stroke_svgpath(pos, size, box_svg, box_stroke_width); !res) return res.error().relay();
        }
      }
      if (!checked) return {};
      if (check_kind == content_kind::image) {
        if (auto res = draw_bitmap(pos, size, check_bitmap); !res) return res.error().relay();
      } else if (check_kind == content_kind::geometry) {
        if (check_fill_color.a > 0.0f) {
          brush::color(check_fill_color);
          if (auto res = fill_svgpath(pos, size, check_svg); !res) return res.error().relay();
        }
        if (check_stroke_color.a > 0.0f && check_stroke_width > 0.0f) {
          brush::color(check_stroke_color);
          if (auto res = stroke_svgpath(pos, size, check_svg, check_stroke_width); !res) return res.error().relay();
        }
      }
      return {};
    }

    virtual bool attachable() const override { return !attach_lock; }

    virtual std::expected<void, error> attach(slotid Child) override {
      if (attach_lock) return std::unexpected(error(errors::invalid_operation, "not attachable"));
      const auto csp = get_slot<control>(Child);
      if (!csp) return std::unexpected(error(errors::invalid_slotid));
      csp->window_id = window_id;
      make_messy();
      return {};
    }

    virtual std::expected<void, error> detach(slotid Child) override {
      return std::unexpected(error(errors::unreachable));
    }

    virtual bool focusable() const noexcept override { return enabled && visible; }

    virtual std::expected<float2, error> get_necessary_size() const override {
      const auto csp = get_slot<control>(content.id());
      if (!csp) return calc_necessary_size_by_policy(float2{});
      if (auto res = csp->get_necessary_size()) return calc_necessary_size_by_policy(*res);
      else return res.error().relay();
    }

    virtual slotid hittest(float2 Pt) const override { return control::slot::hittest(Pt) ? id : slotid{}; }

    virtual std::expected<void, error> redraw() override {
      if (geometry_dirty) {
        geometry_dirty = false;
        if (auto res = relocate(); !res) return res.error().relay();
      }
      if (!visible) return {};
      if (auto res = draw_frame_background(); !res) return res.error().relay();
      if (auto res = draw_icon(); !res) return res.error().relay();
      const auto csp = get_slot<control>(content.id());
      if (csp)
        if (auto res = csp->redraw(); !res) return res.error().relay();
      if (auto res = draw_frame_foreground(); !res) return res.error().relay();
      return {};
    }

    virtual std::expected<void, error> relocate() override {
      const auto max_size = provided_area - margin.xy() - margin.zw();
      if (auto res = set_size_to_necessary(); !res) return res.error().relay();
      if (policy.x == size_policy::free) size.x = max_size.x;
      if (policy.y == size_policy::free) size.y = max_size.y;
      pos = provided_pos + calc_offset_by_align(max_size);
      ID2D1RoundedRectangleGeometry* geom = nullptr;
      D2D1_ROUNDED_RECT rr{D2D1::RectF(pos.x, pos.y, pos.x + size.x, pos.y + size.y), radius.x, radius.y};
      hresult_test(d2d::factory()->CreateRoundedRectangleGeometry, &rr, &geom);
      geometry.reset(geom);

      const auto csp = get_slot<control>(content.id());
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
      make_dirty();
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
        make_dirty();
        if (on_change) on_change(checked);
      }
      return true;
    }
  };

  using frame::operator bool;
  checkbox() noexcept = default;

  checkbox(derived_from<interface> auto& Parent, strict<bool> AutoColor = true, const source_line& sl = here()) {
    if (auto res = create(Parent, AutoColor)) *this = std::move(*res);
    else res.error().add_footprint().go_off(sl);
  }

  static std::expected<checkbox, error> create(derived_from<interface> auto& Parent, strict<bool> AutoColor = true) {
    checkbox c;
    const auto temp_id = make_slot<checkbox>();
    const auto sp = get_slot<checkbox>(temp_id);
    if (!sp) return std::unexpected(error(errors::slot_creation_failed));
    const auto psp = get_slot<control>(Parent.id());
    if (!psp) return std::unexpected(error(errors::invalid_slotid));
    if (auto res = psp->attach(temp_id); !res) {
      slot::slots.erase(temp_id);
      return res.error().relay();
    }
    c._id = temp_id;
    sp->id = temp_id;
    sp->window_id = psp->get_window_id();
    if (AutoColor) {
      sp->colors = color_pair(none{});
      sp->box_stroke_color = sp->colors.border;
      sp->check_fill_color = sp->colors.border;
      sp->check_stroke_color = sp->colors.border;
    }
    sp->attach_lock = false;
    if (auto res = ui::horizontal_layout::create(c)) sp->content = std::move(*res);
    else return res.error().relay();
    sp->attach_lock = true;
    if (auto res = ui::blank::create(sp->content)) sp->icon_spacer = std::move(*res);
    else return res.error().relay();
    if (auto res = ui::label::create(sp->content, false)) sp->caption = std::move(*res);
    else return res.error().relay();
    sp->caption.text_align(alignment::left).text_color(sp->colors.border).margin({});
    constexpr float2 init_icon_size{16.0f, 16.0f};
    sp->icon_spacer.size(init_icon_size);
    sp->update_icon_margin();
    if (auto res = svgpath::create(init_icon_size, "M1 1 L15 1 L15 15 L1 15 Z")) sp->box_svg = std::move(*res);
    else return res.error().relay();
    if (auto res = svgpath::create(init_icon_size, "M3 8 L7 12 L13 4 L7 12 Z")) sp->check_svg = std::move(*res);
    else return res.error().relay();
    sp->colors.border = colors::transparent;
    return c;
  }

  // -- getter --//

  bool checked() const noexcept { ywlib_control_get(checked); }
  bool pressed() const noexcept { ywlib_control_get(pressed); }
  const auto& on_change() const noexcept { ywlib_control_get(on_change); }
  const auto& box_fill_color() const noexcept { ywlib_control_get(box_fill_color); }
  const auto& box_stroke_color() const noexcept { ywlib_control_get(box_stroke_color); }
  const auto& box_stroke_width() const noexcept { ywlib_control_get(box_stroke_width); }
  const auto& check_fill_color() const noexcept { ywlib_control_get(check_fill_color); }
  const auto& check_stroke_color() const noexcept { ywlib_control_get(check_stroke_color); }
  const auto& check_stroke_width() const noexcept { ywlib_control_get(check_stroke_width); }
  const auto& icon_gap() const noexcept { ywlib_control_get(icon_gap); }
  const auto& string() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->caption.string();
  }

  float2 icon_size() const noexcept {
    if (const auto sp = get_slot(this); !sp) {
      error(errors::invalid_slotid).fizzle_out();
      return float2{};
    } else return sp->icon_spacer.size();
  }

  //-- setter --//

  auto& checked(this auto& self, bool b) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    if (sp->checked == b) return self;
    sp->checked = b;
    sp->make_dirty();
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

  auto& icon_size(this auto& self, float2 v) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    if (v.x <= 0.0f || v.y <= 0.0f) {
      error(errors::invalid_argument, format("icon_size must be positive: ", v)).go_off();
      return self;
    }
    sp->icon_spacer.size(v);
    sp->box_svg.size(v);
    sp->check_svg.size(v);
    sp->make_messy();
    return self;
  }

  auto& box(this auto& self, bitmap b) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->box_bitmap = std::move(b);
    sp->box_kind = content_kind::image;
    sp->make_dirty();
    return self;
  }

  auto& box(this auto& self, svgpath p) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    const auto icon_size = sp->icon_spacer.size();
    p.size(icon_size);
    sp->box_svg = std::move(p);
    sp->box_kind = content_kind::geometry;
    sp->make_dirty();
    return self;
  }

  auto& check(this auto& self, bitmap b) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->check_bitmap = std::move(b);
    sp->check_kind = content_kind::image;
    sp->make_dirty();
    return self;
  }

  auto& check(this auto& self, svgpath p) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    const auto icon_size = sp->icon_spacer.size();
    p.size(icon_size);
    sp->check_svg = std::move(p);
    sp->check_kind = content_kind::geometry;
    sp->make_dirty();
    return self;
  }

  auto& box_fill_color(this auto& self, const color& c) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->box_fill_color = c;
    sp->make_dirty();
    return self;
  }

  auto& box_stroke_color(this auto& self, const color& c) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->box_stroke_color = c;
    sp->make_dirty();
    return self;
  }

  auto& box_stroke_width(this auto& self, float1 f) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->box_stroke_width = f.x;
    sp->make_dirty();
    return self;
  }

  auto& check_fill_color(this auto& self, const color& c) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->check_fill_color = c;
    sp->make_dirty();
    return self;
  }

  auto& check_stroke_color(this auto& self, const color& c) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->check_stroke_color = c;
    sp->make_dirty();
    return self;
  }

  auto& check_stroke_width(this auto& self, float1 f) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->check_stroke_width = f.x;
    sp->make_dirty();
    return self;
  }

  auto& icon_gap(this auto& self, float1 f) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->icon_gap = f.x;
    sp->update_icon_margin();
    sp->make_messy();
    return self;
  }
};
} // namespace yw::ui
