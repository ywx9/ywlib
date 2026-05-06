#pragma once
#include "ywx/icon.h"
#include "ywx/text.h"
#include "ywx/ui_control.h"

namespace yw::ui {

class radiobutton : public control {
  static constexpr float2 default_icon_size = {20.0f, 20.0f};
  static constexpr std::string_view box_path = "M 10 0 A 10 10 0 1 1 9.999 0 Z";
  static constexpr std::string_view mark_path = "M 10 6 A 4 4 0 1 1 9.999 6 Z";

public:
  class slot : public control::slot {
  public:
    float4 padding = float4::fill(4.0f);

    yw::icon box = assume(svgpath::create(default_icon_size, box_path));
    color box_fill_color = colors::transparent;
    color box_stroke_color = colors::black;
    float box_stroke_width = 1.0f;

    yw::icon mark = assume(svgpath::create(default_icon_size, mark_path));
    color mark_fill_color = colors::black;
    color mark_stroke_color = colors::black;
    float mark_stroke_width = 1.0f;

    float2 icon_size = default_icon_size;
    float icon_offset = 4.0f;
    float item_gap = 4.0f;
    unsigned checked{};
    unsigned focused_item = ~0u;

    struct item {
      yw::text text = assume(yw::text::create(L""));
      color text_color = colors::black;
    };

    std::vector<item> items{};

    function<void, event::button> on_button;
    function<void, unsigned> on_change;
    function<void, key> on_click;
    function<void, bool> on_focus;
    function<void, event::key> on_keydown;
    function<void, event::key> on_keyup;
    function<void, event::move> on_move;
    function<void, event::wheel> on_wheel;

    key captured_key{};

    float row_height(const item& value) const {
      return yw::max(icon_size.y, value.text.size().y);
    }

    unsigned normalize_focus() const {
      if (items.empty()) return ~0u;
      if (focused_item < items.size()) return focused_item;
      if (checked < items.size()) return checked;
      return 0;
    }

    float row_top(unsigned index) const {
      float y = pos.y + padding.y;
      for (unsigned i = 0; i < index && i < items.size(); ++i) y += row_height(items[i]) + item_gap;
      return y;
    }

    void move_focus(int delta) {
      if (items.empty()) {
        focused_item = ~0u;
        return;
      }
      const auto base = int(normalize_focus());
      const auto n = int(items.size());
      focused_item = unsigned((base + delta + n) % n);
      make_dirty();
    }

    unsigned hit_item(float2 pt) const {
      if (items.empty()) return ~0u;
      float y = pos.y + padding.y;
      for (unsigned index = 0; index < items.size(); ++index) {
        const auto height = row_height(items[index]);
        if (pt.y >= y && pt.y <= y + height) return index;
        y += height + item_gap;
      }
      return ~0u;
    }

    void select(unsigned index) {
      if (index >= items.size() || checked == index) return;
      checked = index;
      make_dirty();
      if (on_change) on_change(index);
    }

    virtual float2 calculate_size() const noexcept override {
      float2 inner = padding.xy() + padding.zw();
      if (items.empty()) inner.y += icon_size.y;
      for (const auto& value : items) {
        const auto tsz = value.text.size();
        inner.x = yw::max(inner.x, icon_size.x + icon_offset + tsz.x + padding.x + padding.z);
        inner.y += yw::max(icon_size.y, tsz.y);
      }
      if (items.size() > 1) inner.y += item_gap * float(items.size() - 1);
      return vapply_r<float2>(yw::max, float2(), min_size, inner, size * constrained);
    }

    virtual void update_size() noexcept override {
      min_size = vapply_r<float2>(yw::max, min_size, float2());
      float2 inner = padding.xy() + padding.zw();
      if (items.empty()) inner.y += icon_size.y;
      for (const auto& value : items) {
        const auto tsz = value.text.size();
        inner.x = yw::max(inner.x, icon_size.x + icon_offset + tsz.x + padding.x + padding.z);
        inner.y += yw::max(icon_size.y, tsz.y);
      }
      if (items.size() > 1) inner.y += item_gap * float(items.size() - 1);
      size = vapply_r<float2>(yw::max, min_size, inner, size * constrained);
      update_geometry();
    }

    virtual void draw() const override {
      if (!visible) return;
      draw_background();

      float y = pos.y + padding.y;
      for (unsigned index = 0; index < items.size(); ++index) {
        const auto& value = items[index];
        const auto tsz = value.text.size();
        const auto height = yw::max(icon_size.y, tsz.y);
        const auto icon_pos = float2(pos.x + padding.x, y + (height - icon_size.y) * 0.5f);
        const auto text_pos = float2(icon_pos.x + icon_size.x + icon_offset, y + (height - tsz.y) * 0.5f);

        brush.color(box_fill_color);
        draw_icon(icon_pos, icon_size, box);
        brush.color(box_stroke_color);
        stroke_icon(icon_pos, icon_size, box, box_stroke_width);
        if (checked == index) {
          brush.color(mark_fill_color);
          draw_icon(icon_pos, icon_size, mark);
          brush.color(mark_stroke_color);
          stroke_icon(icon_pos, icon_size, mark, mark_stroke_width);
        }
        brush.color(value.text_color);
        draw_text(text_pos, value.text);
        y += height + item_gap;
      }
    }

    virtual void draw_focus_ring(float offset, float width) const override {
      if (!visible || items.empty()) return;
      const auto index = normalize_focus();
      if (index == ~0u || index >= items.size()) return;
      const auto y = row_top(index);
      const auto h = row_height(items[index]);
      const auto off = float2::fill(offset);
      const auto ring_pos = float2(pos.x + padding.x, y) - off;
      const auto ring_size = float2(size.x - padding.x - padding.z, h) + off * 2.0f;
      draw_round_rectangle(ring_pos, ring_size, float2::fill(4.0f) + off, width);
    }

    virtual void click_event(event::button e) override {
      if (enabled && e.code == captured_key && e.code == key::lbutton) {
        if (const auto index = hit_item(float2(e.pos)); index != ~0u) {
          focused_item = index;
          select(index);
        }
        if (on_click) on_click(captured_key);
      }
      captured_key = {};
    }

    virtual void button_event(event::button e) override {
      if (!enabled) return;
      if (e.down) captured_key = e.code;
      else captured_key = {};
      if (on_button) on_button(e);
    }

    virtual bool focus_event(bool focused) override {
      if (!focused) {
        captured_key = {};
        focused_item = ~0u;
      } else {
        focused_item = normalize_focus();
        make_dirty();
      }
      if (enabled && on_focus) on_focus(focused);
      return enabled;
    }

    virtual bool key_event(event::key e) override {
      if (!enabled) return false;
      if (e.down) {
        switch (e.code) {
        case key::up:
          captured_key = {};
          move_focus(-1);
          break;
        case key::down:
          captured_key = {};
          move_focus(1);
          break;
        case key::enter:
        case key::space:
          captured_key = e.code;
          break;
        default:
          captured_key = {};
          break;
        }
        if (e.first && on_keydown) on_keydown(e);
        return true;
      } else {
        if (e.code == captured_key && (e.code == key::enter || e.code == key::space)) {
          if (const auto index = normalize_focus(); index != ~0u) select(index);
          if (on_click) on_click(captured_key);
        }
        captured_key = {};
        if (on_keyup) on_keyup(e);
        return true;
      }
    }

    virtual void wheel_event(event::wheel Event) override {
      if (enabled && on_wheel) on_wheel(Event);
    }

  };

  using control::operator bool;
  radiobutton() noexcept = default;
  radiobutton(derived_from<unknown> auto& Layout) {
    if (auto res = create_control<radiobutton>(Layout)) _id = *res;
  }

  const auto& border_color() const { return unsafe_get(&slot::border_color); }
  void border_color(const color& c) { safe_set(&slot::border_color, c); }

  const auto& border_width() const { return unsafe_get(&slot::border_width); }
  void border_width(float w) { safe_set(&slot::border_width, w); }

  const auto& padding() const { return unsafe_get(&slot::padding); }
  void padding(const float4& p) { safe_set_size(&slot::padding, p); }

  const auto& box() const { return unsafe_get(&slot::box); }
  auto& box() { return unsafe_get_size(&slot::box); }
  void box(icon v) { safe_set(&slot::box, std::move(v)); }

  const auto& box_fill_color() const { return unsafe_get(&slot::box_fill_color); }
  void box_fill_color(const color& c) { safe_set(&slot::box_fill_color, c); }

  const auto& box_stroke_color() const { return unsafe_get(&slot::box_stroke_color); }
  void box_stroke_color(const color& c) { safe_set(&slot::box_stroke_color, c); }

  const auto& box_stroke_width() const { return unsafe_get(&slot::box_stroke_width); }
  void box_stroke_width(float w) { safe_set(&slot::box_stroke_width, w); }

  const auto& mark() const { return unsafe_get(&slot::mark); }
  auto& mark() { return unsafe_get_size(&slot::mark); }
  void mark(icon v) { safe_set(&slot::mark, std::move(v)); }

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

  const auto& item_gap() const { return unsafe_get(&slot::item_gap); }
  void item_gap(float1 value) { safe_set_size(&slot::item_gap, value.x); }

  const auto& checked() const { return unsafe_get(&slot::checked); }
  void checked(unsigned index) { safe_set(&slot::checked, index); }

  size_t item_count() const { return unsafe_get(&slot::items).size(); }

  void clear_items() {
    if (auto csp = system::slot_address<slot>(_id)) {
      csp->items.clear();
      csp->checked = 0;
      csp->make_messy();
    }
  }

  template<stringable S> void append_item(S&& s, const color& c = colors::black) {
    if (auto csp = system::slot_address<slot>(_id)) {
      typename slot::item value;
      value.text = static_cast<S&&>(s);
      value.text_color = c;
      csp->items.push_back(std::move(value));
      csp->make_messy();
    }
  }

  const auto& text(size_t index) const { return unsafe_get(&slot::items).at(index).text; }
  auto& text(size_t index) { return unsafe_get_size(&slot::items).at(index).text; }
  void text(size_t index, yw::text value) {
    if (auto csp = system::slot_address<slot>(_id)) {
      if (index >= csp->items.size()) csp->items.resize(index + 1);
      csp->items[index].text = std::move(value);
      csp->make_messy();
    }
  }
  template<stringable S> void text(size_t index, S&& value) {
    if (auto csp = system::slot_address<slot>(_id)) {
      if (index >= csp->items.size()) csp->items.resize(index + 1);
      csp->items[index].text = static_cast<S&&>(value);
      csp->make_messy();
    }
  }

  const auto& text_color(size_t index) const { return unsafe_get(&slot::items).at(index).text_color; }
  void text_color(size_t index, const color& value) {
    if (auto csp = system::slot_address<slot>(_id)) {
      if (index >= csp->items.size()) csp->items.resize(index + 1);
      csp->items[index].text_color = value;
      csp->make_dirty();
    }
  }

  const auto& on_button() const { return unsafe_get(&slot::on_button); }
  void on_button(function<void, event::button> f) { safe_set(&slot::on_button, std::move(f)); }

  const auto& on_change() const { return unsafe_get(&slot::on_change); }
  void on_change(function<void, unsigned> f) { safe_set(&slot::on_change, std::move(f)); }

  const auto& on_click() const { return unsafe_get(&slot::on_click); }
  void on_click(function<void, key> f) { safe_set(&slot::on_click, std::move(f)); }

  const auto& on_focus() const { return unsafe_get(&slot::on_focus); }
  void on_focus(function<void, bool> f) { safe_set(&slot::on_focus, std::move(f)); }

  const auto& on_keydown() const { return unsafe_get(&slot::on_keydown); }
  void on_keydown(function<void, event::key> f) { safe_set(&slot::on_keydown, std::move(f)); }

  const auto& on_keyup() const { return unsafe_get(&slot::on_keyup); }
  void on_keyup(function<void, event::key> f) { safe_set(&slot::on_keyup, std::move(f)); }

  const auto& on_move() const { return unsafe_get(&slot::on_move); }
  void on_move(function<void, event::move> f) { safe_set(&slot::on_move, std::move(f)); }

  const auto& on_wheel() const { return unsafe_get(&slot::on_wheel); }
  void on_wheel(function<void, event::wheel> f) { safe_set(&slot::on_wheel, std::move(f)); }
};
} // namespace yw::ui
