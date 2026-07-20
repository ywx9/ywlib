#pragma once
#include <ywx/bitmap.h>
#include <ywx/svgpath.h>
#include <ywx/text.h>
#include <ywx/ui_frame.h>

namespace yw::ui {

class radiobutton : public frame {
public:
  enum class content_kind : unsigned char { none, image, geometry };

  struct item {
    yw::text caption{};
    float2 pos{};
    float2 size{};
    float2 icon_pos{};
    float2 text_pos{};
  };

  struct slot : frame::slot {
    std::vector<item> items{};
    bitmap circle_bitmap{};
    bitmap dot_bitmap{};
    svgpath circle_svg{};
    svgpath dot_svg{};
    color circle_fill_color = colors::transparent;
    color circle_stroke_color = colors::black;
    float circle_stroke_width = 1.0f;
    color dot_fill_color = colors::black;
    color dot_stroke_color = colors::black;
    color text_color = colors::black;
    color pressed_overlay_color = color(0.0f, 0.0f, 0.0f, 0.16f);
    float dot_stroke_width = 1.0f;
    content_kind circle_kind = content_kind::geometry;
    content_kind dot_kind = content_kind::geometry;
    float2 icon_size_value{16.0f, 16.0f};
    float icon_gap = arbitrary_value;
    float item_gap = arbitrary_value;
    size_t selected = npos;
    size_t pressed = npos;
    size_t hovered_item = npos;

    function<void, size_t> on_change{};

    std::expected<void, error> draw_icon_part(
      float2 Pos, float2 Size, content_kind Kind, const bitmap& Image, const svgpath& Geometry,
      const color& FillColor, const color& StrokeColor, float StrokeWidth) const {
      if (Kind == content_kind::image) {
        if (auto res = draw_bitmap(Pos, Size, Image); !res) return res.error().relay();
      } else if (Kind == content_kind::geometry) {
        if (FillColor.a > 0.0f) {
          brush::color(FillColor);
          if (auto res = fill_svgpath(Pos, Size, Geometry); !res) return res.error().relay();
        }
        if (StrokeColor.a > 0.0f && StrokeWidth > 0.0f) {
          brush::color(StrokeColor);
          if (auto res = stroke_svgpath(Pos, Size, Geometry, StrokeWidth); !res) return res.error().relay();
        }
      }
      return {};
    }

    std::expected<void, error> draw_icons() const {
      for (size_t i = 0; i < items.size(); ++i) {
        const auto pos = items[i].icon_pos;
        const auto size = icon_size_value;
        if (auto res = draw_icon_part(
              pos, size, circle_kind, circle_bitmap, circle_svg, circle_fill_color, circle_stroke_color,
              circle_stroke_width);
            !res)
          return res.error().relay();
        if (i != selected) continue;
        if (auto res = draw_icon_part(pos, size, dot_kind, dot_bitmap, dot_svg, dot_fill_color, dot_stroke_color,
              dot_stroke_width);
            !res)
          return res.error().relay();
      }
      return {};
    }

    std::expected<void, error> draw_pressed_overlay() const {
      if (pressed == npos || pressed_overlay_color.a <= 0.0f) return {};
      if (pressed >= items.size()) return {};
      brush::color(pressed_overlay_color);
      if (auto res = fill_rectangle(items[pressed].pos, items[pressed].size); !res) return res.error().relay();
      return {};
    }

    virtual std::expected<void, error> draw_hovered_overlay() override {
      if (hovered_item == npos || hovered_overlay_color.a <= 0.0f) return {};
      if (hovered_item >= items.size()) return {};
      brush::color(hovered_overlay_color);
      if (auto res = fill_rectangle(items[hovered_item].pos, items[hovered_item].size); !res)
        return res.error().relay();
      return {};
    }

    size_t item_at(float2 Pt) const noexcept {
      for (size_t i = 0; i < items.size(); ++i) {
        const auto r = float4(items[i].pos, items[i].pos + items[i].size);
        if (Pt.x >= r.x && Pt.y >= r.y && Pt.x <= r.z && Pt.y <= r.w) return i;
      }
      return npos;
    }

    void select(size_t Index) {
      if (Index >= items.size() || selected == Index) return;
      selected = Index;
      make_dirty();
      if (on_change) on_change(selected);
    }

    virtual bool focusable() const noexcept override { return enabled && visible; }

    virtual std::expected<float2, error> get_necessary_size() const override {
      float2 inner{};
      for (size_t i = 0; i < items.size(); ++i) {
        const auto text_size = items[i].caption.size();
        const auto gap = text_size.x > 0.0f ? icon_gap : 0.0f;
        const float2 row_size{icon_size_value.x + gap + text_size.x, yw::max(icon_size_value.y, text_size.y)};
        inner.x = yw::max(inner.x, row_size.x);
        inner.y += row_size.y;
        if (i + 1 < items.size()) inner.y += item_gap;
      }
      inner += padding.xy() + padding.zw();
      return calc_necessary_size_by_policy(inner);
    }

    virtual slotid hittest(float2 Pt) const override { return control::slot::hittest(Pt) ? id : slotid{}; }

    virtual std::expected<void, error> redraw() override {
      if (geometry_dirty) {
        geometry_dirty = false;
        if (auto res = relocate(); !res) return res.error().relay();
      }
      if (!visible) return {};
      if (auto res = draw_frame_background(); !res) return res.error().relay();
      if (auto res = draw_pressed_overlay(); !res) return res.error().relay();
      if (auto res = draw_icons(); !res) return res.error().relay();
      brush::color(text_color);
      for (const auto& it : items)
        if (auto res = it.caption.draw(it.text_pos); !res) return res.error().relay();
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

      const auto content_pos = pos + padding.xy();
      const auto content_size = size - padding.xy() - padding.zw();
      float y = content_pos.y;
      for (size_t i = 0; i < items.size(); ++i) {
        const auto text_size = items[i].caption.size();
        const auto gap = text_size.x > 0.0f ? icon_gap : 0.0f;
        const float row_height = yw::max(icon_size_value.y, text_size.y);
        auto& it = items[i];
        it.pos = {content_pos.x, y};
        it.size = {content_size.x, row_height};
        it.icon_pos = {content_pos.x, y + (row_height - icon_size_value.y) * 0.5f};
        it.text_pos = {content_pos.x + icon_size_value.x + gap, y + (row_height - text_size.y) * 0.5f};
        y += row_height + (i + 1 < items.size() ? item_gap : 0.0f);
      }
      return {};
    }

    virtual std::expected<void, error> set_size_to_necessary() override {
      if (auto res = get_necessary_size()) size = *res;
      else return res.error().relay();
      return {};
    }

    virtual std::expected<void, error> reset_state() override {
      if (pressed == npos) return {};
      pressed = npos;
      make_dirty();
      return {};
    }

    virtual bool button_event(yw::button_event e) override {
      if (!enabled || !visible || e.key != keys::lbutton) return false;
      const auto next_pressed = e.down ? item_at(float2(float(e.pos.x), float(e.pos.y))) : npos;
      if (pressed == next_pressed) return true;
      pressed = next_pressed;
      make_dirty();
      return true;
    }

    virtual bool click_event(yw::button_event e) override {
      if (!enabled || !visible || e.down || e.key != keys::lbutton) return false;
      const auto hit = item_at(float2(float(e.pos.x), float(e.pos.y)));
      if (hit == npos) return false;
      select(hit);
      pressed = npos;
      return true;
    }

    virtual void focus_event(bool Focused) override {
      if (!Focused && pressed != npos) {
        pressed = npos;
        make_dirty();
      }
    }

    virtual bool hover_event(yw::hover_event e) override {
      const auto next_hovered = e.leave() ? npos : item_at(float2(float(e.pos.x), float(e.pos.y)));
      if (hovered_item != next_hovered) {
        hovered_item = next_hovered;
        make_dirty();
      }
      return frame::slot::hover_event(e);
    }

    virtual bool key_event(yw::key_event e) override {
      if (!enabled || !visible || items.empty()) return false;
      if (e.key == keys::up || e.key == keys::left || e.key == keys::down || e.key == keys::right) {
        if (!e.down) return true;
        const bool backward = e.key == keys::up || e.key == keys::left;
        const auto base = selected == npos ? 0 : selected;
        const auto next = backward ? (base == 0 ? items.size() - 1 : base - 1) : (base + 1) % items.size();
        select(next);
        return true;
      }
      if (e.key != keys::space && e.key != keys::enter) return false;
      if (e.down) return true;
      select(selected == npos ? 0 : selected);
      return true;
    }

    virtual std::expected<void, error> apply_color_theme(const yw::ui::color_theme& Theme, bool Recursive) override {
      background_color = Theme.surface;
      border_color = colors::transparent;
      hovered_overlay_color = color(Theme.accent, default_overlay_opacity.hover);
      circle_fill_color = colors::transparent;
      circle_stroke_color = Theme.outline;
      dot_fill_color = Theme.accent;
      dot_stroke_color = Theme.accent;
      text_color = Theme.text;
      pressed_overlay_color = color(Theme.accent, default_overlay_opacity.pressed);
      make_dirty();
      return {};
    }
  };

  using frame::operator bool;
  radiobutton() noexcept = default;

  radiobutton(derived_from<interface> auto& Parent, const source_line& sl = here()) {
    if (auto res = create(Parent)) *this = std::move(*res);
    else res.error().add_footprint().go_off(sl);
  }

  static std::expected<radiobutton, error> create(derived_from<interface> auto& Parent) {
    radiobutton r;
    const auto temp_id = make_slot<radiobutton>();
    const auto sp = get_slot<radiobutton>(temp_id);
    if (!sp) return std::unexpected(error(errors::slot_creation_failed));
    const auto psp = get_slot<control>(Parent.id());
    if (!psp) return std::unexpected(error(errors::invalid_slotid));
    if (auto res = psp->attach(temp_id); !res) {
      slot::slots.erase(temp_id);
      return res.error().relay();
    }
    r._id = temp_id;
    sp->id = temp_id;
    sp->window_id = psp->get_window_id();
    sp->policy = {ui::size_policy::free, ui::size_policy::fit};
    constexpr float2 init_icon_size{16.0f, 16.0f};
    if (auto res = svgpath::create(init_icon_size,
          "M8 1 C4.134 1 1 4.134 1 8 C1 11.866 4.134 15 8 15 C11.866 15 15 11.866 15 8 C15 4.134 11.866 1 8 1 Z"))
      sp->circle_svg = std::move(*res);
    else return res.error().relay();
    if (auto res = svgpath::create(init_icon_size,
          "M8 5 C6.343 5 5 6.343 5 8 C5 9.657 6.343 11 8 11 C9.657 11 11 9.657 11 8 C11 6.343 9.657 5 8 5 Z"))
      sp->dot_svg = std::move(*res);
    else return res.error().relay();
    if (auto res = sp->apply_current_color_theme(false); !res) return res.error().relay();
    return r;
  }

  //-- getter --//

  size_t selected_index() const noexcept { ywlib_control_get(selected); }
  size_t item_count() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->items.size();
  }
  const auto& on_change() const noexcept { ywlib_control_get(on_change); }
  const auto& circle_fill_color() const noexcept { ywlib_control_get(circle_fill_color); }
  const auto& circle_stroke_color() const noexcept { ywlib_control_get(circle_stroke_color); }
  const auto& circle_stroke_width() const noexcept { ywlib_control_get(circle_stroke_width); }
  const auto& dot_fill_color() const noexcept { ywlib_control_get(dot_fill_color); }
  const auto& dot_stroke_color() const noexcept { ywlib_control_get(dot_stroke_color); }
  const auto& dot_stroke_width() const noexcept { ywlib_control_get(dot_stroke_width); }
  const auto& text_color() const noexcept { ywlib_control_get(text_color); }
  const auto& pressed_overlay_color() const noexcept { ywlib_control_get(pressed_overlay_color); }
  const auto& icon_gap() const noexcept { ywlib_control_get(icon_gap); }
  const auto& item_gap() const noexcept { ywlib_control_get(item_gap); }

  const auto& string(size_t Index) const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    if (Index >= sp->items.size()) error(errors::invalid_argument, format("invalid item index: ", Index)).go_off();
    return sp->items[Index].caption.string();
  }

  float2 icon_size() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return float2{};
    }
    return sp->icon_size_value;
  }

  //-- setter --//

  auto& add(this auto& self, yw::string<wchar_t> s) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    item it;
    if (auto res = yw::text::create(std::move(s))) it.caption = std::move(*res);
    else {
      res.error().go_off();
      return self;
    }
    sp->items.push_back(std::move(it));
    if (sp->selected == npos) sp->selected = 0;
    sp->make_messy();
    return self;
  }

  auto& string(this auto& self, size_t Index, yw::string<wchar_t> s) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    if (Index >= sp->items.size()) {
      error(errors::invalid_argument, format("invalid item index: ", Index)).go_off();
      return self;
    }
    sp->items[Index].caption.string(std::move(s));
    return self;
  }

  auto& selected_index(this auto& self, size_t Index) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    if (Index >= sp->items.size()) {
      error(errors::invalid_argument, format("invalid item index: ", Index)).go_off();
      return self;
    }
    sp->select(Index);
    return self;
  }

  auto& on_change(this auto& self, function<void, size_t> f) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->on_change = std::move(f);
    return self;
  }

  auto& font(this auto& self, font_config f) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    for (auto& it : sp->items)
      if (auto res = it.caption.font(f); !res) res.error().go_off();
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
    sp->circle_svg.size(v);
    sp->dot_svg.size(v);
    sp->make_messy();
    return self;
  }

  auto& circle(this auto& self, bitmap b) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->circle_bitmap = std::move(b);
    sp->circle_kind = content_kind::image;
    sp->make_dirty();
    return self;
  }

  auto& circle(this auto& self, svgpath p) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    p.size(sp->icon_size_value);
    sp->circle_svg = std::move(p);
    sp->circle_kind = content_kind::geometry;
    sp->make_dirty();
    return self;
  }

  auto& dot(this auto& self, bitmap b) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->dot_bitmap = std::move(b);
    sp->dot_kind = content_kind::image;
    sp->make_dirty();
    return self;
  }

  auto& dot(this auto& self, svgpath p) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    p.size(sp->icon_size_value);
    sp->dot_svg = std::move(p);
    sp->dot_kind = content_kind::geometry;
    sp->make_dirty();
    return self;
  }

  auto& circle_fill_color(this auto& self, const color& c) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->circle_fill_color = c;
    sp->make_dirty();
    return self;
  }

  auto& circle_stroke_color(this auto& self, const color& c) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->circle_stroke_color = c;
    sp->make_dirty();
    return self;
  }

  auto& circle_stroke_width(this auto& self, float1 f) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->circle_stroke_width = f.x;
    sp->make_dirty();
    return self;
  }

  auto& dot_fill_color(this auto& self, const color& c) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->dot_fill_color = c;
    sp->make_dirty();
    return self;
  }

  auto& dot_stroke_color(this auto& self, const color& c) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->dot_stroke_color = c;
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

  auto& dot_stroke_width(this auto& self, float1 f) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->dot_stroke_width = f.x;
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

  auto& item_gap(this auto& self, float1 f) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->item_gap = f.x;
    sp->make_messy();
    return self;
  }
};
} // namespace yw::ui
