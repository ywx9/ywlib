#pragma once
#include "ywx/ui_plain.h"
#include "ywx/ui_label.h"

namespace yw::ui {

class button : public control {
public:
  class slot : public control::slot {
  public:
    yw::background background = colors::white;
    color border_color = colors::black;
    float border_width = 1.0f;
    float4 padding = float4::fill(5.0f);

    yw::text text = assume(yw::text::create(L""));
    color text_color = colors::black;

    function<void, event::button> on_button;
    function<void, key> on_click;
    function<void, bool> on_focus;
    function<void, event::key> on_key;
    function<void, event::move> on_move;
    function<void, event::wheel> on_wheel;

    key captured_key{};

    virtual float2 get_minimum_draw_size() const noexcept {
      const auto tsz = text.size() + padding.xy() + padding.zw();
      auto result = float2(ucc.x ? 0.0f : size.x, ucc.y ? 0.0f : size.y);
      result.x = yw::max(result.x, minimum_size.x, tsz.x);
      result.y = yw::max(result.y, minimum_size.y, tsz.y);
      return result;
    }

    virtual void draw() const override {
      if (!visible) return;
      draw_background(pos, size, background);
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
        if (on_click) on_click(captured_key);
      }
    }
  };

  using control::operator bool;
  button() noexcept = default;
  button(derived_from<unknown> auto& Layout) {
    if (auto res = create_control<button>(Layout)) _id = *res;
  }

  const auto& background() const { return unsafe_get(&slot::background); }
  void background(const yw::background& bg) { safe_set(&slot::background, bg); }

  const auto& border_color() const { return unsafe_get(&slot::border_color); }
  void border_color(const color& c) { safe_set(&slot::border_color, c); }

  const auto& border_width() const { return unsafe_get(&slot::border_width); }
  void border_width(float w) { safe_set(&slot::border_width, w); }

  const auto& padding() const { return unsafe_get(&slot::padding); }
  void padding(const float4& p) { safe_set_size(&slot::padding, p); }

  const auto& text() const { return unsafe_get(&slot::text); }
  auto& text() { return unsafe_get_size(&slot::text); }
  void text(const yw::text& t) { safe_set(&slot::text, t); }

  const auto& text_color() const { return unsafe_get(&slot::text_color); }
  void text_color(const color& c) { safe_set(&slot::text_color, c); }

  const auto& on_button() const { return unsafe_get(&slot::on_button); }
  void on_button(const function<void, event::button>& f) { safe_set(&slot::on_button, f); }

  const auto& on_click() const { return unsafe_get(&slot::on_click); }
  void on_click(const function<void, key>& f) { safe_set(&slot::on_click, f); }

  const auto& on_focus() const { return unsafe_get(&slot::on_focus); }
  void on_focus(const function<void, bool>& f) { safe_set(&slot::on_focus, f); }

  const auto& on_key() const { return unsafe_get(&slot::on_key); }
  void on_key(const function<void, event::key>& f) { safe_set(&slot::on_key, f); }

  const auto& on_move() const { return unsafe_get(&slot::on_move); }
  void on_move(const function<void, event::move>& f) { safe_set(&slot::on_move, f); }

  const auto& on_wheel() const { return unsafe_get(&slot::on_wheel); }
  void on_wheel(const function<void, event::wheel>& f) { safe_set(&slot::on_wheel, f); }
};

// class button : public clickable_plain {
// public:
//   class slot : public clickable_plain::slot {
//   public:
//     yw::text text;
//     color text_color = colors::black;
//     float4 padding = float4::fill(5.0f);

//     virtual float2 demand_survey() const noexcept {
//       const auto tsz = text.size() + padding.xy() + padding.zw();
//       auto result = float2(ucc.x ? 0.0f : size.x, ucc.y ? 0.0f : size.y);
//       result.x = yw::max(result.x, minimum_size.x, tsz.x);
//       result.y = yw::max(result.y, minimum_size.y, tsz.y);
//       return result + margin.xy() + margin.zw();
//     }

//     virtual void draw(float2 Pos, float2 Size) override {
//       Pos += margin.xy();
//       Size -= margin.xy() + margin.zw();
//       const auto min_size = demand_survey() - margin.xy() - margin.zw();
//       size.x = ucc.x ? Size.x : yw::max(size.x, min_size.x);
//       size.y = ucc.y ? Size.y : yw::max(size.y, min_size.y);
//       const auto extra = Size - size;
//       pos = Pos;
//       switch (alignment) {
//       case ui::alignment::center: pos += extra * 0.5f; break;
//       case ui::alignment::left: break;
//       case ui::alignment::right: pos.x += extra.x; break;
//       case ui::alignment::top: break;
//       case ui::alignment::bottom: pos.y += extra.y; break;
//       case ui::alignment::left_top: break;
//       case ui::alignment::left_bottom: pos.y += extra.y; break;
//       case ui::alignment::right_top: pos.x += extra.x; break;
//       case ui::alignment::right_bottom: pos += extra; break;
//       }
//       draw();
//     }

//     virtual void draw() const override {
//       plain::slot::draw();
//       brush.color(text_color);
//       const auto tsz = text.size() + padding.xy() + padding.zw();
//       draw_text(pos + padding.xy() + (size - tsz) * 0.5f, text);
//     }
//   };

//   using clickable_plain::operator bool;
//   button() noexcept = default;
//   button(derived_from<unknown> auto& Layout) {
//     if (auto res = create_control<button>(Layout)) _id = *res;
//     if (const auto csp = system::slot_address<slot>(_id))
//       if (auto t = yw::text::create(L"")) csp->text = std::move(*t);
//   }

//   const auto& text() const { return unsafe_get(&slot::text); }
//   const auto& text_color() const { return unsafe_get(&slot::text_color); }
//   const auto& padding() const { return unsafe_get(&slot::padding); }

//   auto& text() {
//     if (const auto csp = system::slot_address<slot>(_id)) {
//       csp->make_messy();
//       return csp->text;
//     } else throw std::logic_error("Invalid member access");
//   }

//   template<stringable S> void text(S&& Text) {
//     if (const auto csp = system::slot_address<slot>(_id)) {
//       csp->make_messy();
//       csp->text(unicode<wchar_t>(static_cast<S&&>(Text)));
//     }
//   }
//   void text_color(const color& c) { safe_set(&slot::text_color, c); }
//   void padding(const float4& p) { safe_set(&slot::padding, p); }
// };
} // namespace yw::ui
