#pragma once
#include "ywx/window.h"
#include "ywx/text_format.h"

namespace yw::ui {

class text : public base {
public:
  class slot : public base::slot {
  public:
    float2 padding{};
    color text_color = colors::black;
    std::wstring text{};
    text_layout layout{};
    virtual bool hit_test(float2 pt) const noexcept override {
      const auto lt = base::slot::pos + padding;
      const auto rb = base::slot::pos + base::slot::size - padding;
      return pt.x >= lt.x && pt.x <= rb.x && pt.y >= lt.y && pt.y <= rb.y;
    }
    virtual void draw() const override {
      base::slot::draw();
      draw_text(pos + padding, layout, text_color);
    }
  };

public:
  using base::operator bool;

  const auto& padding() const { return unsafe_get(&slot::padding); }
  const auto& text_color() const { return unsafe_get(&slot::text_color); }
  const auto& layout() const { return unsafe_get(&slot::layout); }
  void text_color(const color& value) { _set(&slot::text_color, value); }

  /// gets text
  const auto& operator()() const { return unsafe_get(&slot::text); }

  /// sets padding and updates text layouts
  void padding(float2 value) {
    if (const auto s = slot_adress(this))
      if (auto tl = text_layout::create(s->text, s->layout, s->size - value * 2.0f)) {
        s->padding = value;
        s->layout = std::move(*tl);
        if (const auto w = system::windows.get(s->window_id)) w->dirty = true;
      }
  }

  /// sets text and updates text layout
  template<stringable S> void operator()(S&& Text) {
    if (const auto s = slot_adress(this))
      if (auto tl = text_layout::create(Text, s->layout, s->size - s->padding * 2.0f)) {
        s->text = unicode<wchar_t>(static_cast<S&&>(Text));
        s->layout = std::move(*tl);
        if (const auto w = system::windows.get(s->window_id)) w->dirty = true;
      }
  }

  /// sets text alignment without rebuilding the layout
  void text_alignment(DWRITE_TEXT_ALIGNMENT align) {
    if (const auto s = slot_adress(this))
      if (s->layout.text_alignment(align))
        if (const auto w = system::windows.get(s->window_id)) w->dirty = true;
  }

  /// sets paragraph alignment without rebuilding the layout
  void paragraph_alignment(DWRITE_PARAGRAPH_ALIGNMENT align) {
    if (const auto s = slot_adress(this))
      if (s->layout.paragraph_alignment(align))
        if (const auto w = system::windows.get(s->window_id)) w->dirty = true;
  }

  /// sets text format and updates text layout
  void text_format(text_format_like auto&& tf) {
    if (const auto s = slot_adress(this))
      if (auto tl = text_layout::create(s->text, tf, s->size - s->padding * 2.0f)) {
        s->layout = std::move(*tl);
        if (const auto w = system::windows.get(s->window_id)) w->dirty = true;
      }
  }

  /// adds a text UI to the specified window and returns its handle
  template<included_in<window&, none> Window, stringable S> static std::expected<text, error_trace> add(Window&& w, float2 pos, float2 size, S&& Text, float2 Padding = {}) {
    if (auto res = dwrite.initialize(); !res) return unexpected_error(res.error());
    if (auto res = base::add<text>(w, pos, size)) {
      auto slot_p = res->second;
      slot_p->text = unicode<wchar_t>(static_cast<S&&>(Text));
      slot_p->padding = Padding;
      const auto sz = float2(yw::max(size.x - Padding.x * 2.0f, 0.0f), yw::max(size.y - Padding.y * 2.0f, 0.0f));
      if (auto tl = text_layout::create(slot_p->text, dwrite.text_format(), sz)) slot_p->layout = std::move(*tl);
      else return unexpected_error(tl.error());
      return text{std::move(res->first)};
    } else return unexpected_error(res.error());
  }
};
}
