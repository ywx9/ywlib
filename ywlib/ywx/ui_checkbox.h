#pragma once
#include <ywx/bitmap.h>
#include <ywx/svgpath.h>
#include <ywx/ui_button.h>

namespace yw::ui {

class checkbox : public button {
public:
  enum class content_kind : unsigned char { none, image, geometry };

  struct slot : button::slot {
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
    float2 icon_size_value{16.0f, 16.0f};
    float icon_gap = arbitrary_value;
    bool checked = false;

    function<void, bool> on_change{};

    float2 icon_pos() const noexcept {
      const auto inner_pos = pos + padding.xy();
      const auto inner_size = size - padding.xy() - padding.zw();
      return {inner_pos.x, inner_pos.y + (inner_size.y - icon_size_value.y) * 0.5f};
    }

    float2 checkbox_text_pos() const noexcept {
      const auto p = pos + padding.xy();
      const auto inner_size = size - padding.xy() - padding.zw();
      const auto text_y = p.y + (inner_size.y - text.size().y) * 0.5f;
      return {p.x + icon_size_value.x + icon_gap, text_y};
    }

    std::expected<void, error> draw_icon() const {
      const auto pos = icon_pos();
      const auto size = icon_size_value;
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

    virtual std::expected<float2, error> get_necessary_size() const override {
      const auto text_size = text.size();
      const auto gap = text_size.x > 0.0f ? icon_gap : 0.0f;
      const auto inner = float2{icon_size_value.x + gap + text_size.x, yw::max(icon_size_value.y, text_size.y)} +
                         padding.xy() + padding.zw();
      return calc_necessary_size_by_policy(inner);
    }

    virtual std::expected<void, error> redraw() override {
      if (geometry_dirty) {
        geometry_dirty = false;
        if (auto res = relocate(); !res) return res.error().relay();
      }
      if (!visible) return {};
      if (auto res = draw_frame_background(); !res) return res.error().relay();
      if (pressed && pressed_overlay_color.a > 0.0f) {
        brush::color(pressed_overlay_color);
        if (auto res = fill_geometry(geometry.get()); !res) return res.error().relay();
      }
      if (auto res = draw_icon(); !res) return res.error().relay();
      brush::color(text_color);
      if (auto res = text.draw(checkbox_text_pos()); !res) return res.error().relay();
      if (auto res = draw_frame_foreground(); !res) return res.error().relay();
      return {};
    }

    virtual void invoke(yw::button_event e) override {
      checked = !checked;
      make_dirty();
      if (on_change) on_change(checked);
      button::slot::invoke(e);
    }

    virtual std::expected<void, error> apply_color_theme(const yw::ui::color_theme& Theme, bool Recursive) override {
      background_color = Theme.surface;
      border_color = colors::transparent;
      hovered_overlay_color = color(Theme.accent, default_overlay_opacity.hover);
      text_color = Theme.text;
      box_fill_color = colors::transparent;
      box_stroke_color = Theme.outline;
      check_fill_color = Theme.accent;
      check_stroke_color = Theme.accent;
      pressed_overlay_color = color(Theme.accent, default_overlay_opacity.pressed);
      make_dirty();
      return {};
    }
  };

  using button::operator bool;
  checkbox() noexcept = default;

  checkbox(derived_from<interface> auto& Parent, const source_line& sl = here()) {
    if (auto res = create(Parent)) *this = std::move(*res);
    else res.error().add_footprint().go_off(sl);
  }

  static std::expected<checkbox, error> create(derived_from<interface> auto& Parent) {
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
    sp->policy = {ui::size_policy::free, ui::size_policy::fit};
    sp->text_align = alignment::left;
    constexpr float2 init_icon_size{16.0f, 16.0f};
    if (auto res = svgpath::create(init_icon_size, "M1 1 L15 1 L15 15 L1 15 Z")) sp->box_svg = std::move(*res);
    else return res.error().relay();
    if (auto res = svgpath::create(init_icon_size, "M3 8 L7 12 L13 4 L7 12 Z")) sp->check_svg = std::move(*res);
    else return res.error().relay();
    if (auto res = sp->apply_current_color_theme(false); !res) return res.error().relay();
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
  const auto& pressed_overlay_color() const noexcept { ywlib_control_get(pressed_overlay_color); }
  const auto& icon_gap() const noexcept { ywlib_control_get(icon_gap); }
  const auto& string() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->text.string();
  }

  float2 icon_size() const noexcept {
    if (const auto sp = get_slot(this); !sp) {
      error(errors::invalid_slotid).fizzle_out();
      return float2{};
    } else return sp->icon_size_value;
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
    if (auto res = sp->text.string(std::move(s)); !res) res.error().go_off();
    sp->make_messy();
    return self;
  }

  auto& font(this auto& self, font_config f) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    if (auto res = sp->text.font(std::move(f)); !res) res.error().go_off();
    sp->make_messy();
    return self;
  }

  auto& text_color(this auto& self, const color& c) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->text_color = c;
    sp->make_dirty();
    return self;
  }

  auto& icon_size(this auto& self, float2 v) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    if (v.x <= 0.0f || v.y <= 0.0f) {
      error(errors::invalid_argument, format("icon_size must be positive: ", v)).go_off();
      return self;
    }
    sp->icon_size_value = v;
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
    const auto icon_size = sp->icon_size_value;
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
    const auto icon_size = sp->icon_size_value;
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

  auto& pressed_overlay_color(this auto& self, const color& c) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->pressed_overlay_color = c;
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
    sp->make_messy();
    return self;
  }
};
} // namespace yw::ui
