#pragma once
#include "ywx/text_format.h"
#include "ywx/window.h"

namespace yw::ui {

//////////////////////////////////////// MARK: ui::label

class label : public base {
public:
  class slot : public base::slot {
  public:
    float2 padding{};
    color text_color = colors::black;
    std::wstring text{};
    text_layout layout{};

    virtual void draw() const override {
      base::slot::draw();
      draw_text(pos + padding, layout, text_color);
    }

    virtual std::expected<void, error_trace> proc(UINT, WPARAM, LPARAM) override { return {}; }
  };

protected:
  slot* _label_slot() const noexcept { return dynamic_cast<slot*>(_ui_slot()); }

public:
  using base::base;

  const auto& padding() const { return *_label_get(&slot::padding); }
  const auto& text_color() const { return *_label_get(&slot::text_color); }
  const auto& text() const { return *_label_get(&slot::text); }

  DWRITE_TEXT_ALIGNMENT text_alignment() const noexcept {
    if (const auto s = _label_slot())
      if (auto r = s->layout.text_alignment()) return *r;
    return DWRITE_TEXT_ALIGNMENT_LEADING;
  }

  DWRITE_PARAGRAPH_ALIGNMENT paragraph_alignment() const noexcept {
    if (const auto s = _label_slot())
      if (auto r = s->layout.paragraph_alignment()) return *r;
    return DWRITE_PARAGRAPH_ALIGNMENT_NEAR;
  }

  void padding(float2 value) {
    if (const auto s = _label_slot()) {
      if (auto tl = text_layout::create(s->text, s->layout, s->size - value * 2.0f)) {
        s->padding = value;
        s->layout = std::move(*tl);
        if (const auto w = system::windows.get(s->window_id)) w->dirty = true;
      }
    }
  }

  void text_color(const color& value) { _label_set(&slot::text_color, value); }

  template<stringable S> void text(S&& Text) {
    auto s = unicode<wchar_t>(static_cast<S&&>(Text));
    if (const auto s = _label_slot()) {
      if (auto tl = text_layout::create(s->text, s->layout, s->size - s->padding * 2.0f)) {
        s->text = std::move(s);
        s->layout = std::move(*tl);
        if (const auto w = system::windows.get(s->window_id)) w->dirty = true;
      }
    }
  }

  void text_alignment(DWRITE_TEXT_ALIGNMENT align) {
    if (const auto s = _label_slot()) {
      s->layout.text_alignment(align);
      if (const auto w = system::windows.get(s->window_id)) w->dirty = true;
    }
  }

  void paragraph_alignment(DWRITE_PARAGRAPH_ALIGNMENT align) {
    if (const auto s = _label_slot()) {
      s->layout.paragraph_alignment(align);
      if (const auto w = system::windows.get(s->window_id)) w->dirty = true;
    }
  }

  template<text_format_like Tf> void text_format(Tf&& tf) {
    if (const auto s = _label_slot()) {
      if (auto tl = text_layout::create(s->text, static_cast<Tf&&>(tf), s->size - s->padding * 2.0f)) {
        s->layout = std::move(*tl);
        if (const auto w = system::windows.get(s->window_id)) w->dirty = true;
      }
    }
  }

  template<stringable S> static std::expected<label, error_trace> add(
    window& w, float2 Pos, float2 Size, S&& Text) {
    if (auto res = dwrite.initialize(); !res) return unexpected_error(res.error());
    if (auto res = _add<label>(w, Pos, Size)) {
      auto s = unicode<wchar_t>(static_cast<S&&>(Text));
      if (auto tl = text_layout::create(s.text, dwrite.text_format(), Size)) {

      }
    }
    return _add<label>(w, Pos, Size);
  }
};
} // namespace yw::ui
