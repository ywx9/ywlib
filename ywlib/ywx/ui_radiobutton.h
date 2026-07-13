#pragma once
#include <ywx/bitmap.h>
#include <ywx/svgpath.h>
#include <ywx/ui_blank.h>
#include <ywx/ui_label.h>
#include <ywx/ui_layout.h>

namespace yw::ui {

class radiobutton : public frame {
public:
  enum class content_kind : unsigned char { none, image, geometry };

  struct item {
    horizontal_layout row{};
    blank icon_spacer{};
    label caption{};
  };

  struct slot : frame::slot {
    vertical_layout content{};
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
    float dot_stroke_width = 1.0f;
    content_kind circle_kind = content_kind::geometry;
    content_kind dot_kind = content_kind::geometry;
    float2 icon_size_value{16.0f, 16.0f};
    float icon_gap = arbitrary_value;
    bool attach_lock = true;
    size_t selected = npos;
    size_t pressed = npos;

    function<void, size_t> on_change{};

    void update_icon_margins() {
      for (auto& it : items)
        if (it.icon_spacer)
          it.icon_spacer.margin(float4(arbitrary_value, arbitrary_value, arbitrary_value + icon_gap, arbitrary_value));
    }

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
        const auto isp = get_slot<control>(items[i].icon_spacer.id());
        if (!isp || !isp->visible) continue;
        const auto pos = isp->pos();
        const auto size = isp->size();
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

    size_t item_at(float2 Pt) const noexcept {
      for (size_t i = 0; i < items.size(); ++i) {
        const auto rsp = get_slot<control>(items[i].row.id());
        if (!rsp || !rsp->visible) continue;
        const auto r = float4(rsp->pos, rsp->pos + rsp->size);
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
      if (auto res = draw_icons(); !res) return res.error().relay();
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
  };

  using frame::operator bool;
  radiobutton() noexcept = default;

  radiobutton(derived_from<interface> auto& Parent, strict<bool> AutoColor = true, const source_line& sl = here()) {
    if (auto res = create(Parent, AutoColor)) *this = std::move(*res);
    else res.error().add_footprint().go_off(sl);
  }

  static std::expected<radiobutton, error> create(derived_from<interface> auto& Parent, strict<bool> AutoColor = true) {
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
    if (AutoColor) {
      sp->colors = color_pair(none{});
      sp->circle_stroke_color = sp->colors.border;
      sp->dot_fill_color = sp->colors.border;
      sp->dot_stroke_color = sp->colors.border;
      sp->text_color = sp->colors.border;
    }
    sp->attach_lock = false;
    if (auto res = ui::vertical_layout::create(r)) sp->content = std::move(*res);
    else return res.error().relay();
    sp->attach_lock = true;
    constexpr float2 init_icon_size{16.0f, 16.0f};
    if (auto res = svgpath::create(init_icon_size,
          "M8 1 C4.134 1 1 4.134 1 8 C1 11.866 4.134 15 8 15 C11.866 15 15 11.866 15 8 C15 4.134 11.866 1 8 1 Z"))
      sp->circle_svg = std::move(*res);
    else return res.error().relay();
    if (auto res = svgpath::create(init_icon_size,
          "M8 5 C6.343 5 5 6.343 5 8 C5 9.657 6.343 11 8 11 C9.657 11 11 9.657 11 8 C11 6.343 9.657 5 8 5 Z"))
      sp->dot_svg = std::move(*res);
    else return res.error().relay();
    sp->colors.border = colors::transparent;
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
  const auto& icon_gap() const noexcept { ywlib_control_get(icon_gap); }

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
    if (auto res = ui::horizontal_layout::create(sp->content)) it.row = std::move(*res);
    else {
      res.error().go_off();
      return self;
    }
    if (auto res = ui::blank::create(it.row)) it.icon_spacer = std::move(*res);
    else {
      res.error().go_off();
      return self;
    }
    if (auto res = ui::label::create(it.row, false)) it.caption = std::move(*res);
    else {
      res.error().go_off();
      return self;
    }
    it.icon_spacer.size(sp->icon_size_value);
    it.icon_spacer.margin(float4(arbitrary_value, arbitrary_value, arbitrary_value + sp->icon_gap, arbitrary_value));
    it.caption.text_align(alignment::left).text_color(sp->text_color).margin({}).string(std::move(s));
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
    for (auto& it : sp->items) it.caption.font(f);
    return self;
  }

  auto& text_color(this auto& self, const color& c) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->text_color = c;
    for (auto& it : sp->items) it.caption.text_color(c);
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
    for (auto& it : sp->items) it.icon_spacer.size(sp->icon_size_value);
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
    sp->update_icon_margins();
    sp->make_messy();
    return self;
  }
};
} // namespace yw::ui
