#pragma once
#include "ywx/text_format.h"
#include "ywx/window.h"

namespace yw::control {

//////////////////////////////////////// MARK: control::label

class label : public base {
public:
  class slot : public base::slot {
  public:
    float2 padding{};
    color text_color = colors::black;
    std::wstring text{};
    yw::text_layout text_layout{};

    slot(const window::slotid& window_id, float2 Pos, float2 Size) : base::slot(window_id, Pos, Size) {
      if (auto res = yw::text_layout::create(L"", dwrite.text_format(), Size - padding * 2); !res) return;
      else text_layout = std::move(*res);
      text_layout.text_alignment(DWRITE_TEXT_ALIGNMENT_CENTER);
      text_layout.paragraph_alignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    }

    virtual std::expected<void, error_trace> draw() const override {
      if (auto res = base::slot::draw(); !res) return unexpected_error(res.error());
      if (auto res = draw_text(position + padding, text_layout, text_color); !res) return unexpected_error(res.error());
      return {};
    }

    virtual std::expected<bool, error_trace> proc(UINT, WPARAM, LPARAM) override { return true; }
  };

protected:
  slot* _label(window::slot* window_slot) const noexcept {
    return dynamic_cast<slot*>(window_slot->controls.get(_id.control));
  }

  slot* _label() const noexcept {
    const auto window_slot = _window();
    return window_slot ? _label(window_slot) : nullptr;
  }

public:
  using base::base;
  using base::operator bool;

  float2 padding() const noexcept {
    if (const auto label_slot = _label()) return label_slot->padding;
    else return float2{};
  }

  color text_color() const noexcept {
    if (const auto label_slot = _label()) return label_slot->text_color;
    else return color();
  }

  std::wstring text() const noexcept {
    if (const auto label_slot = _label()) return label_slot->text;
    else return std::wstring{};
  }

  std::wstring font_name() const {
    if (const auto label_slot = _label()) {
      if (auto res = label_slot->text_layout.font_name()) return std::move(*res);
      else return std::wstring{};
    } else return std::wstring{};
  }

  float font_size() const {
    if (const auto label_slot = _label()) {
      if (auto res = label_slot->text_layout.font_size()) return *res;
      else return 0.0f;
    } else return 0.0f;
  }

  DWRITE_FONT_WEIGHT font_weight() const {
    if (const auto label_slot = _label()) {
      if (auto res = label_slot->text_layout.font_weight()) return *res;
      else return DWRITE_FONT_WEIGHT_REGULAR;
    } else return DWRITE_FONT_WEIGHT_REGULAR;
  }

  DWRITE_FONT_STYLE font_style() const {
    if (const auto label_slot = _label()) {
      if (auto res = label_slot->text_layout.font_style()) return *res;
      else return DWRITE_FONT_STYLE_NORMAL;
    } else return DWRITE_FONT_STYLE_NORMAL;
  }

  DWRITE_FONT_STRETCH font_stretch() const {
    if (const auto label_slot = _label()) {
      if (auto res = label_slot->text_layout.font_stretch()) return *res;
      else return DWRITE_FONT_STRETCH_NORMAL;
    } else return DWRITE_FONT_STRETCH_NORMAL;
  }

  DWRITE_TEXT_ALIGNMENT text_alignment() const {
    if (const auto label_slot = _label()) {
      if (auto res = label_slot->text_layout.text_alignment()) return *res;
      else return DWRITE_TEXT_ALIGNMENT_LEADING;
    } else return DWRITE_TEXT_ALIGNMENT_LEADING;
  }

  DWRITE_PARAGRAPH_ALIGNMENT paragraph_alignment() const {
    if (const auto label_slot = _label()) {
      if (auto res = label_slot->text_layout.paragraph_alignment()) return *res;
      else return DWRITE_PARAGRAPH_ALIGNMENT_NEAR;
    } else return DWRITE_PARAGRAPH_ALIGNMENT_NEAR;
  }

  void padding(float2 p) noexcept {
    if (const auto window_slot = _window())
      if (const auto ls = _label(window_slot))
        if (auto res = yw::text_layout::create(ls->text, ls->text_layout, ls->size - p * 2)) {
          window_slot->dirty = true;
          ls->text_layout = std::move(*res);
          ls->padding = p;
        }
  }

  void text_color(const color& c) noexcept {
    if (const auto window_slot = _window())
      if (const auto label_slot = _label(window_slot)) {
        window_slot->dirty = true;
        label_slot->text_color = c;
      }
  }

  void text(stringable<wchar_t> auto&& t) {
    auto sv = static_cast<std::wstring_view>(t);
    if (const auto window_slot = _window())
      if (const auto ls = _label(window_slot))
        if (auto res = yw::text_layout::create(sv, ls->text_layout, ls->size - ls->padding * 2)) {
          window_slot->dirty = true;
          ls->text_layout = std::move(*res);
          ls->text.assign(sv);
        }
  }

  void text_format(text_format_like auto&& tf) {
    if (const auto window_slot = _window())
      if (const auto ls = _label(window_slot))
        if (auto res = yw::text_layout::create(ls->text, tf, ls->size - ls->padding * 2)) {
          window_slot->dirty = true;
          ls->text_layout = std::move(*res);
        }
  }

  void text_alignment(DWRITE_TEXT_ALIGNMENT a) noexcept {
    if (const auto window_slot = _window())
      if (const auto ls = _label(window_slot)) {
        window_slot->dirty = true;
        ls->text_layout.text_alignment(a);
      }
  }

  void paragraph_alignment(DWRITE_PARAGRAPH_ALIGNMENT a) noexcept {
    if (const auto window_slot = _window())
      if (const auto ls = _label(window_slot)) {
        window_slot->dirty = true;
        ls->text_layout.paragraph_alignment(a);
      }
  }

  static std::expected<label, error_trace> add(window::slave& w, float2 position, float2 size) {
    const auto window_slot = window::system.get_window(w);
    if (!window_slot) return unexpected_error(errors::invalid_argument, "invalid window");
    auto label_slot = std::make_unique<slot>(window_slot->id, position, size);
    const auto cid = window_slot->controls.push(std::move(label_slot));
    const auto control_slot = window_slot->controls.get(cid);
    if (!control_slot) return unexpected_error(errors::operation_failed, "failed to create label slot");
    control_slot->id.control = cid;
    return label({window_slot->id.master, window_slot->id.slave, cid});
  }
};
} // namespace yw::control
