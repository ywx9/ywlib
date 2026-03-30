#pragma once
#include "ywx/icon.h"
#include "ywx/text.h"
#include "ywx/ui_layout.h"

namespace yw::ui {

class checkbox : public control {
  static constexpr float2 default_icon_size = {20.0f, 20.0f};
  static constexpr std::string_view box_path = "M 10 0 A 10 10 0 1 1 9.999 0 Z";
  static constexpr std::string_view mark_path = "M 10 5 A 5 5 0 1 1 9.999 0 Z";

public:
  class slot : public control::slot {
  public:
    yw::background background = colors::white;
    color border_color = colors::black;
    float border_width = 1.0f;
    float4 padding = float4::fill(10.0f);

    yw::icon box = assume(svgpath::create(default_icon_size, box_path));
    color box_fill_color = colors::transparent;
    color box_stroke_color = colors::black;
    float box_stroke_width = 1.0f;

    yw::icon mark = assume(svgpath::create(default_icon_size, mark_path));
    color mark_fill_color = colors::transparent;
    color mark_stroke_color = colors::black;
    float mark_stroke_width = 1.0f;

    float2 icon_size = default_icon_size;
    float icon_offset = 5.0f;
    bool checked{};

    yw::text text = assume(yw::text::create(L""));
    color text_color = colors::black;

    function<void, event::button> on_button;
    function<void, bool> on_change;
    function<void, key> on_click;
    function<void, bool> on_focus;
    function<void, event::key> on_key;
    function<void, event::move> on_move;
    function<void, event::wheel> on_wheel;

    key captured_key{};

    virtual void update_size() noexcept override {
      min_size = vapply_r<float2>(yw::max, min_size, float2());
      const auto tsz = text.size();
      const auto inner_x = icon_size.x + icon_offset + tsz.x + padding.x + padding.z;
      const auto inner = float2(inner_x, yw::max(icon_size.y, tsz.y) + padding.y + padding.w);
      size = vapply_r<float2>(yw::max, min_size, inner, size * constrained);
    }

    virtual void draw() const override {
      if (!visible) return;
      draw_background(pos, size, background);
      brush.color(border_color);
      draw_round_rectangle(pos, size, radius, border_width);
      brush.color(box_fill_color);
      draw_icon(pos, icon_size, box);
      brush.color(box_stroke_color);
      stroke_icon(pos, icon_size, box);
      if (checked) {
        brush.color(mark_fill_color);
        draw_icon(pos, icon_size, mark);
        brush.color(mark_stroke_color);
        stroke_icon(pos, icon_size, mark);
      }
      brush.color(text_color);
      const auto tsz = text.size() + padding.xy() + padding.zw();
      draw_text(pos + padding.xy() + (size - tsz) * 0.5f, text);
    }

    virtual void click_event(event::button e) override {
      if (enabled && e.code == captured_key) click_action();
      captured_key = {};
    }

    virtual void button_event(event::button e) override {
      if (!enabled) return;
      if (e.down) captured_key = e.code;
      else captured_key = {};
      if (on_button) on_button(e);
    }

    virtual bool focus_event(bool focused) override {
      captured_key = {};
      if (enabled && on_focus) on_focus(focused);
      return enabled;
    }

    virtual void key_event(event::key e) override {
      if (!enabled) return;
      if (e.first) {
        if (e.down) captured_key = e.code;
        else if (e.code == captured_key) click_action();
        else captured_key = {};
      }
      if (on_key) on_key(e);
    }

    virtual void wheel_event(event::wheel Event) override {
      if (enabled && on_wheel) on_wheel(Event);
    }

    virtual void click_action() {
      switch (captured_key) {
      case key::lbutton:
      case key::enter:
      case key::space:
        checked = !checked;
        if (on_click) on_click(captured_key);
        if (on_change) on_change(checked);
      }
    }
  };

  using control::operator bool;
  checkbox() noexcept = default;
  checkbox(derived_from<unknown> auto& Layout) {
    if (auto res = create_control<checkbox>(Layout)) _id = *res;
  }

  const auto& background() const { return unsafe_get(&slot::background); }
  void background(yw::background bg) { safe_set(&slot::background, std::move(bg)); }

  const auto& border_color() const { return unsafe_get(&slot::border_color); }
  void border_color(const color& c) { safe_set(&slot::border_color, c); }

  const auto& border_width() const { return unsafe_get(&slot::border_width); }
  void border_width(float w) { safe_set(&slot::border_width, w); }

  const auto& padding() const { return unsafe_get(&slot::padding); }
  void padding(const float4& p) { safe_set_size(&slot::padding, p); }

  const auto& box() const { return unsafe_get(&slot::box); }
  auto& box() { return unsafe_get_size(&slot::box); }
  void box(icon b) { safe_set(&slot::box, std::move(b)); }

  const auto& box_fill_color() const { return unsafe_get(&slot::box_fill_color); }
  void box_fill_color(const color& c) { safe_set(&slot::box_fill_color, c); }

  const auto& box_stroke_color() const { return unsafe_get(&slot::box_stroke_color); }
  void box_stroke_color(const color& c) { safe_set(&slot::box_stroke_color, c); }

  const auto& box_stroke_width() const { return unsafe_get(&slot::box_stroke_width); }
  void box_stroke_width(float w) { safe_set(&slot::box_stroke_width, w); }

  const auto& mark() const { return unsafe_get(&slot::mark); }
  auto& mark() { return unsafe_get_size(&slot::mark); }
  void mark(icon b) { safe_set(&slot::mark, std::move(b)); }

  const auto& mark_fill_color() const { return unsafe_get(&slot::mark_fill_color); }
  void mark_fill_color(const color& c) { safe_set(&slot::mark_fill_color, c); }

  const auto& mark_stroke_color() const { return unsafe_get(&slot::mark_stroke_color); }
  void mark_stroke_color(const color& c) { safe_set(&slot::mark_stroke_color, c); }

  const auto& mark_stroke_width() const { return unsafe_get(&slot::mark_stroke_width); }
  void mark_stroke_width(float1 w) { safe_set(&slot::mark_stroke_width, w.x); }

  const auto& icon_size() const { return unsafe_get(&slot::icon_size); }
  void icon_size(float2 size) { safe_set_size(&slot::icon_size, size); }

  const auto& icon_offset() const { return unsafe_get(&slot::icon_offset); }
  void icon_offset(float1 offset) { safe_set_size(&slot::icon_offset, offset.x); }

  const auto& checked() const { return unsafe_get(&slot::checked); }
  void checked(bool c) { safe_set(&slot::checked, c); }

  const auto& text() const { return unsafe_get(&slot::text); }
  auto& text() { return unsafe_get_size(&slot::text); }
  void text(yw::text t) { safe_set(&slot::text, std::move(t)); }

  const auto& text_color() const { return unsafe_get(&slot::text_color); }
  void text_color(const color& c) { safe_set(&slot::text_color, c); }

  const auto& on_button() const { return unsafe_get(&slot::on_button); }
  void on_button(function<void, event::button> f) { safe_set(&slot::on_button, std::move(f)); }

  const auto& on_change() const { return unsafe_get(&slot::on_change); }
  void on_change(function<void, bool> f) { safe_set(&slot::on_change, std::move(f)); }

  const auto& on_click() const { return unsafe_get(&slot::on_click); }
  void on_click(function<void, key> f) { safe_set(&slot::on_click, std::move(f)); }

  const auto& on_focus() const { return unsafe_get(&slot::on_focus); }
  void on_focus(function<void, bool> f) { safe_set(&slot::on_focus, std::move(f)); }

  const auto& on_key() const { return unsafe_get(&slot::on_key); }
  void on_key(function<void, event::key> f) { safe_set(&slot::on_key, std::move(f)); }

  const auto& on_move() const { return unsafe_get(&slot::on_move); }
  void on_move(function<void, event::move> f) { safe_set(&slot::on_move, std::move(f)); }

  const auto& on_wheel() const { return unsafe_get(&slot::on_wheel); }
  void on_wheel(function<void, event::wheel> f) { safe_set(&slot::on_wheel, std::move(f)); }
};
} // namespace yw::ui

// namespace yw::ui {

// class checkbox_labelless final : public control {
// public:
//   class slot : public control::slot {
//     void _click_action() {
//       switch (captured_key) {
//       case key::lbutton:
//       case key::enter:
//       case key::space:
//         checked = !checked;
//         if (on_click) on_click();
//         if (on_change) on_change(checked);
//         break;
//       }
//       captured_key = {};
//     }

//   public:
//     icon box;
//     icon mark;
//     function<void, bool> on_change;
//     function<void> on_click;
//     function<void, bool> on_focus;
//     function<void, event::hover> on_hover;
//     key captured_key{};
//     bool checked{};

//     virtual void draw(float2 Pos, float2 Size) const override {
//       update_last_rect(Pos, Size);
//       draw();
//     }

//     virtual void draw() const override {
//       const auto lr_off = last_rect.xy();
//       const auto lr_size = last_rect.zw() - lr_off;
//       auto _draw = [&lr_off, &lr_size](const icon& Icon) {
//         const auto i_size = Icon.size();
//         const float i_ratio = yw::min(lr_size.x / i_size.x, lr_size.y / i_size.y);
//         const auto i_draw_size = i_size * i_ratio;
//         const auto i_offset = (lr_size - i_draw_size) * 0.5f;
//         draw_icon(lr_off + i_offset, i_draw_size, Icon);
//       };
//       _draw(box);
//       if (checked) _draw(mark);
//     }

//     virtual void button_event(event::button e) override {
//       if (enabled && e.down) captured_key = e.code;
//       else captured_key = {};
//     }

//     virtual void click_event(event::button e) override {
//       if (enabled && e.code == captured_key) _click_action();
//       captured_key = {};
//     }

//     virtual bool focus_event(bool focused) override {
//       captured_key = {};
//       if (on_focus) on_focus(focused);
//       return enabled;
//     }

//     virtual void hover_event(event::hover Event) override {
//       if (enabled && on_hover) on_hover(Event);
//       if (tooltip.empty()) return;
//       if (Event.move()) {
//         if (const auto w = system::slot_address<ui::window>(window_id))
//           system::tooltip.show(last_rect.xy() + w->pos + w->margin.xy(), last_rect.zw() - last_rect.xy());
//       } else if (Event.enter()) {
//         if (const auto w = system::slot_address<ui::window>(window_id))
//           system::tooltip.show(last_rect.xy() + w->pos() + w->margin.xy(), last_rect.zw() - last_rect.xy(), tooltip);
//       } else if (Event.leave()) system::tooltip.hide();
//     }

//     virtual void key_event(event::key e) override {
//       if (enabled && e.first) {
//         if (e.down) captured_key = e.code;
//         else if (e.code == captured_key) _click_action();
//         else captured_key = {};
//       }
//     }
//   };

//   using control::operator bool;
//   checkbox_labelless() noexcept = default;
//   checkbox_labelless(derived_from<unknown> auto& Layout) {
//     if (auto res = create_control<checkbox_labelless>(Layout); !res) return;
//     else _id = *res;
//     constexpr float2 vb = {24.0f, 24.0f};
//     constexpr std::string_view box_path = "M 12 2 "
//                                           "C 17.5228 2 22 6.4772 22 12 "
//                                           "C 22 17.5228 17.5228 22 12 22 "
//                                           "C 6.4772 22 2 17.5228 2 12 "
//                                           "C 2 6.4772 6.4772 2 12 2 Z";
//     constexpr std::string_view mark_path = "M 12 6 "
//                                            "C 15.3137 6 18 8.6863 18 12 "
//                                            "C 18 15.3137 15.3137 18 12 18 "
//                                            "C 8.6863 18 6 15.3137 6 12 "
//                                            "C 6 8.6863 8.6863 6 12 6 Z";
//     const auto csp = system::slot_address<slot>(_id);
//     if (auto p = svgpath::create(vb, box_path)) csp->box = icon_variant(std::move(*p));
//     if (auto p = svgpath::create(vb, mark_path)) csp->mark = icon_variant(std::move(*p));
//   }

//   const auto& box() const { return unsafe_get(&slot::box); }
//   auto& box() { return unsafe_get(&slot::box); }
//   void box(icon New) { unsafe_get(&slot::box) = std::move(New); }

//   const auto& mark() const { return unsafe_get(&slot::mark); }
//   auto& mark() { return unsafe_get(&slot::mark); }
//   void mark(icon New) { unsafe_get(&slot::mark) = std::move(New); }

//   const auto& on_change() const { return unsafe_get(&slot::on_change); }
//   void on_change(function<void, bool> New) { unsafe_get(&slot::on_change) = std::move(New); }

//   const auto& on_click() const { return unsafe_get(&slot::on_click); }
//   void on_click(function<void> New) { unsafe_get(&slot::on_click) = std::move(New); }

//   const auto& on_focus() const { return unsafe_get(&slot::on_focus); }
//   void on_focus(function<void, bool> New) { unsafe_get(&slot::on_focus) = std::move(New); }

//   bool checked() const { return unsafe_get(&slot::checked); }
//   void checked(bool value) { unsafe_get(&slot::checked) = value; }
// };

// //////////////////////////////////////// MARK: checkbox

// class checkbox : public horizontal_layout {
// public:
//   class slot : public horizontal_layout::slot {
//   public:
//     checkbox_labelless checkbox;
//     label text;

//     function<void, bool> on_focus;
//     function<void, event::key> on_key;
//     function<void, event::move> on_move;

//   };
// };
// } // namespace yw::ui

// #pragma once
// #include "ywx/ui_label.h"
// #include "ywx/ui_plain.h"

// namespace yw::ui {

// using icon_variant = std::variant<std::monostate, bitmap, svgpath>;

// class checkbox : public clickable_plain {
// public:
//   class part {
//     void _draw(float2 Pos, float2 Size, const icon_variant& iv, const color& fc, const color& sc, float w) const {
//       if (std::holds_alternative<svgpath>(iv)) {
//         const auto& path = std::get<svgpath>(iv);
//         float2 icon_sz = path.size();
//         icon_sz *= (icon_sz.x > 0.0f && icon_sz.y > 0.0f) ? yw::min(Size.x / icon_sz.x, Size.y / icon_sz.y) : 1.0f;
//         const float2 offset = (Size - icon_sz) * 0.5f;
//         brush.color(fc);
//         fill_svgpath(Pos + offset, icon_sz, path);
//         brush.color(sc);
//         stroke_svgpath(Pos + offset, icon_sz, path, w);
//       } else if (std::holds_alternative<bitmap>(iv)) draw_bitmap(Pos, Size, std::get<bitmap>(iv));
//     }

//   public:
//     icon_variant box_icon = _default_off_circle_icon();
//     color box_fill_color = colors::white;
//     color box_stroke_color = colors::black;
//     float box_stroke_width = 1.0f;

//     icon_variant mark_icon = _default_on_circle_icon();
//     color mark_fill_color = colors::black;
//     color mark_stroke_color = colors::transparent;
//     float mark_stroke_width = 1.0f;

//     void draw(float2 Pos, float2 Size, bool Checked) {
//       _draw(Pos, Size, box_icon, box_fill_color, box_stroke_color, box_stroke_width);
//       if (Checked) _draw(Pos, Size, mark_icon, mark_fill_color, mark_stroke_color, mark_stroke_width);
//     }
//   };

//   class slot : public clickable_plain::slot {

//     void _draw_content() const {
//       const auto sz = last_rect.zw() - last_rect.xy();
//       const float bs = yw::min(icon_size, sz.y);
//       const float2 box_pos = {last_rect.x, last_rect.y + (sz.y - bs) * 0.5f};
//       const float2 box_sz = {bs, bs};
//       _draw_icon(box_pos, box_sz, box_icon, box_fill_color, box_stroke_color, box_stroke_width);
//       if (checked) _draw_icon(box_pos, box_sz, mark_icon, mark_fill_color, mark_stroke_color, mark_stroke_width);
//       const float text_x = last_rect.x + bs + gap;
//       const float text_w = last_rect.z - text_x;
//       if (text_w > 0.0f) {
//         text.size({text_w, sz.y});
//         text.draw({text_x, last_rect.y});
//       }
//     }

//   public:
//     part icon;
//     bool checked = false;
//     float icon_size = 20.0f;
//     mutable label::part text;
//     float4 padding{6.0f, unconstrained, unconstrained, unconstrained};

//     function<void, bool> on_change;

//     virtual tuple<float2, uint2> require_size() const noexcept override {
//       tuple<float2, uint2> result{};

//       const auto label_sz = text.size();
//       const auto content_w = yw::max(icon_size + gap + yw::max(label_sz.x, 0.0f), minimum_size.x);
//       const auto content_h = yw::max(yw::max(icon_size, yw::max(label_sz.y, 0.0f)), minimum_size.y);

//       result.first.x = yw::max(size.x, content_w) + margin.x + margin.z;
//       result.first.y = yw::max(size.y, content_h) + margin.y + margin.w;
//       result.second.x = size.x < 0.0f;
//       result.second.y = size.y < 0.0f;
//       return result;
//     }

//     virtual void draw(float2 Pos, float2 Size) const override {
//       clickable_plain::slot::draw(Pos, Size);
//       _draw_content();
//     }

//     virtual void draw() const override {
//       clickable_plain::slot::draw();
//       _draw_content();
//     }

//     virtual void click_action() override {
//       switch (captured_key) {
//       case key::lbutton:
//       case key::enter:
//       case key::space:
//         checked = !checked;
//         if (on_click) on_click();
//         if (on_change) on_change(checked);
//       }
//     }
//   };

//   using clickable_plain::operator bool;
//   checkbox() noexcept = default;
//   checkbox(derived_from<unknown> auto& Layout) {
//     if (auto res = create_control<checkbox>(Layout)) _id = *res;
//   }

//   // --- box ---
//   const auto& box() const { return unsafe_get(&slot::box_icon); }
//   void box(icon_variant v) { safe_set(&slot::box_icon, std::move(v)); }

//   const color& box_fill_color() const { return unsafe_get(&slot::box_fill_color); }
//   void box_fill_color(const color& c) { safe_set(&slot::box_fill_color, c); }

//   const color& box_stroke_color() const { return unsafe_get(&slot::box_stroke_color); }
//   void box_stroke_color(const color& c) { safe_set(&slot::box_stroke_color, c); }

//   float box_stroke_width() const { return unsafe_get(&slot::box_stroke_width); }
//   void box_stroke_width(float w) { safe_set(&slot::box_stroke_width, w); }

//   // --- mark_icon ---
//   const auto& mark_icon() const { return unsafe_get(&slot::mark_icon); }
//   void mark_icon(icon_variant v) { safe_set(&slot::mark_icon, std::move(v)); }

//   const color& mark_fill_color() const { return unsafe_get(&slot::mark_fill_color); }
//   void mark_fill_color(const color& c) { safe_set(&slot::mark_fill_color, c); }

//   const color& mark_stroke_color() const { return unsafe_get(&slot::mark_stroke_color); }
//   void mark_stroke_color(const color& c) { safe_set(&slot::mark_stroke_color, c); }

//   float mark_stroke_width() const { return unsafe_get(&slot::mark_stroke_width); }
//   void mark_stroke_width(float w) { safe_set(&slot::mark_stroke_width, w); }

//   // --- state ---
//   bool checked() const { return unsafe_get(&slot::checked); }
//   void checked(bool v) { safe_set(&slot::checked, v); }

//   float icon_size() const { return unsafe_get(&slot::icon_size); }
//   void icon_size(float v) { safe_set(&slot::icon_size, v); }

//   float gap() const { return unsafe_get(&slot::gap); }
//   void gap(float v) { safe_set(&slot::gap, v); }

//   // --- text ---
//   auto& text() { return unsafe_get(&slot::text); }
//   const auto& text() const { return unsafe_get(&slot::text); }

//   const auto& on_change() const { return unsafe_get(&slot::on_change); }
//   void on_change(function<void, bool> value) { safe_set(&slot::on_change, std::move(value)); }
// };
// } // namespace yw::ui
