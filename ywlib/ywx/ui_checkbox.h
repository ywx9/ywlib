#pragma once
#include "ywx/ui_frame.h"
#include "ywx/ui_label.h"

namespace yw::ui {

using icon_variant = std::variant<std::monostate, bitmap, svgpath>;

class checkbox : public frame {
public:
  class slot : public frame::slot {

    void _draw_icon(float2 Pos, float2 Size,
      const icon_variant& iv, color fc, color sc, float sw) const {
      if (std::holds_alternative<svgpath>(iv)) {
        const auto& path = std::get<svgpath>(iv);
        float2 icon_sz = path.size();
        const float scale = (icon_sz.x > 0.0f && icon_sz.y > 0.0f)
          ? yw::min(Size.x / icon_sz.x, Size.y / icon_sz.y) : 1.0f;
        icon_sz *= scale;
        const float2 offset = (Size - icon_sz) * 0.5f;
        brush.color(fc);
        fill_svgpath(Pos + offset, icon_sz, path);
        brush.color(sc);
        stroke_svgpath(Pos + offset, icon_sz, path, sw);
      } else if (std::holds_alternative<bitmap>(iv)) {
        draw_bitmap(Pos, Size, std::get<bitmap>(iv));
      }
    }

    void _draw_content() const {
      const auto sz = last_rect.zw() - last_rect.xy();
      const float bs = yw::min(box_size, sz.y);
      const float2 box_pos = {last_rect.x, last_rect.y + (sz.y - bs) * 0.5f};
      const float2 box_sz  = {bs, bs};

      // off アイコン（ボックス本体）を常に描画
      _draw_icon(box_pos, box_sz, icon_off, fill_color_off, stroke_color_off, stroke_width_off);
      // on アイコン（チェックマーク等）を checked 時に重ねて描画
      if (checked)
        _draw_icon(box_pos, box_sz, icon_on, fill_color_on, stroke_color_on, stroke_width_on);

      const float text_x = last_rect.x + bs + gap;
      const float text_w  = last_rect.z - text_x;
      if (text_w > 0.0f) {
        text.size({text_w, sz.y});
        text.draw({text_x, last_rect.y});
      }
    }

  public:
    icon_variant icon_off;
    color        fill_color_off   = colors::transparent;
    color        stroke_color_off = colors::black;
    float        stroke_width_off = 1.0f;

    icon_variant icon_on;
    color        fill_color_on   = colors::transparent;
    color        stroke_color_on = colors::black;
    float        stroke_width_on = 1.0f;

    bool  checked  = false;
    float box_size = 20.0f;
    float gap      = 6.0f;
    mutable label::part text;
    function<void, bool> on_change;

    virtual void draw(float2 Pos, float2 Size) const override {
      frame::slot::draw(Pos, Size);
      _draw_content();
    }

    virtual void draw() const override {
      frame::slot::draw();
      _draw_content();
    }

    virtual void click_event(event::button Event) override {
      if (enabled && Event.code == key::lbutton && !Event.down) {
        checked = !checked;
        make_dirty();
        if (on_change) on_change(checked);
      }
      frame::slot::click_event(Event);
    }
  };

  checkbox() noexcept = default;

  checkbox(derived_from<unknown> auto& Layout) {
    if (auto res = create_control<checkbox>(Layout)) _id = *res;
  }

  using frame::operator bool;

  // --- icon_off ---
  const auto& icon_off() const { return unsafe_get(&slot::icon_off); }
  void        icon_off(icon_variant v) { safe_set(&slot::icon_off, std::move(v)); }

  const color& fill_color_off() const { return unsafe_get(&slot::fill_color_off); }
  void         fill_color_off(const color& c) { safe_set(&slot::fill_color_off, c); }

  const color& stroke_color_off() const { return unsafe_get(&slot::stroke_color_off); }
  void         stroke_color_off(const color& c) { safe_set(&slot::stroke_color_off, c); }

  float stroke_width_off() const { return unsafe_get(&slot::stroke_width_off); }
  void  stroke_width_off(float w) { safe_set(&slot::stroke_width_off, w); }

  // --- icon_on ---
  const auto& icon_on() const { return unsafe_get(&slot::icon_on); }
  void        icon_on(icon_variant v) { safe_set(&slot::icon_on, std::move(v)); }

  const color& fill_color_on() const { return unsafe_get(&slot::fill_color_on); }
  void         fill_color_on(const color& c) { safe_set(&slot::fill_color_on, c); }

  const color& stroke_color_on() const { return unsafe_get(&slot::stroke_color_on); }
  void         stroke_color_on(const color& c) { safe_set(&slot::stroke_color_on, c); }

  float stroke_width_on() const { return unsafe_get(&slot::stroke_width_on); }
  void  stroke_width_on(float w) { safe_set(&slot::stroke_width_on, w); }

  // --- state ---
  bool checked() const { return unsafe_get(&slot::checked); }
  void checked(bool v) { safe_set(&slot::checked, v); }

  float box_size() const { return unsafe_get(&slot::box_size); }
  void  box_size(float v) { safe_set(&slot::box_size, v); }

  float gap() const { return unsafe_get(&slot::gap); }
  void  gap(float v) { safe_set(&slot::gap, v); }

  // --- text ---
  auto&       text()       { return unsafe_get(&slot::text); }
  const auto& text() const { return unsafe_get(&slot::text); }

  // --- callback ---
  const auto& on_change() const { return unsafe_get(&slot::on_change); }
  void        on_change(function<void, bool> value) { safe_set(&slot::on_change, std::move(value)); }
};
} // namespace yw::ui
