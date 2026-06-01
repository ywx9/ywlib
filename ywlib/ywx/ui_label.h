#pragma once
#include "ywx/text.h"
#include "ywx/ui_frame.h"

namespace yw::ui {

class label : public frame {
public:
  struct slot : public frame::slot {
    yw::text text;
    ui::alignment text_alignment = ui::alignment::center;

    /// MARK: overrides

    virtual std::expected<void, error_trace> draw() const override {
      if (!visible) return {};
      if (auto res = draw_background(); !res) return unexpected_error(res.error());
      const auto text_origin = calculate_content_origin(text.bounds(), padding, text_alignment);
      if (auto res = text.draw(text_origin); !res) return unexpected_error(res.error());
      if (auto res = draw_foreground(); !res) return unexpected_error(res.error());
      return {};
    }

    virtual std::expected<float2, error_trace> calculate_necessary_size() const override {
      const float2 inner = text.bounds() + padding.xy() + padding.zw();
      return vapply_r<float2>(yw::max, minimum_size, required_size * constrained, inner);
    }

    virtual std::expected<void, error_trace> ensure_necessary_size() override {
      if (auto res = text.update(); !res) return unexpected_error(res.error());
      if (auto res = calculate_necessary_size()) size = *res;
      else return unexpected_error(res.error());
      return {};
    }
  };

  /// MARK: text accessor

  class text_accessor : public accessor<label> {
    using accessor<label>::slot;

  public:
    ~text_accessor() noexcept {
      try {
        if (slot.text.messy())
          if (auto res = slot.text.update(); !res) fatal_error(res.error());
      } catch (...) {
        fatal_error(errors::unreachable, "Unhandled exception in ui::label::text_accessor destructor");
      }
    }

    const auto& string() const { return slot.text.string(); }
    auto& string(std::wstring String) {
      slot.text.string(std::move(String));
      this->messy = true;
      return *this;
    }
    const auto& color() const { return slot.text.color(); }
    auto& color(const yw::color& Color) {
      slot.text.color(Color);
      this->dirty = true;
      return *this;
    }
    const auto& font() const { return slot.text.font(); }
    auto& font(font_config Font) {
      slot.text.font(std::move(Font));
      this->messy = true;
      return *this;
    }
    /// gets text alignment
    const auto& text_alignment() const { return slot.text.alignment(); }
    /// sets text alignment
    auto& text_alignment(ui::alignment Alignment) {
      slot.text.alignment(Alignment);
      this->dirty = true;
      return *this;
    }
    const auto& font_name() const { return slot.text.font_name(); }
    auto& font_name(std::wstring FontName) { return font({.name = std::move(FontName)}); }
    auto font_size() const { return slot.text.font_size(); }
    auto& font_size(float FontSize) { return font({.size = FontSize}); }
    auto font_weight() const { return slot.text.font_weight(); }
    auto& font_weight(yw::font_weight FontWeight) { return font({.weight = FontWeight}); }
    auto font_style() const { return slot.text.font_style(); }
    auto& font_style(yw::font_style FontStyle) { return font({.style = FontStyle}); }
    auto font_stretch() const { return slot.text.font_stretch(); }
    auto& font_stretch(yw::font_stretch FontStretch) { return font({.stretch = FontStretch}); }
    /// gets alignment of text block
    const auto& alignment() const { return slot.text_alignment; }
    /// sets alignment of text block
    auto& alignment(ui::alignment Alignment) {
      slot.text_alignment = Alignment;
      this->dirty = true;
      return *this;
    }
  };

  /// MARK: handle functions

  using control::operator bool;
  label() noexcept = default;

  static std::expected<label, error_trace> add(derived_from<unknown> auto& Layout, bool auto_color = true) {
    label lbl;
    if (auto res = create_control<label>(Layout)) lbl._id = *res;
    else return unexpected_error(res.error());
    if (const auto csp = system::slot_address<label>(lbl._id)) {
      color text_color = colors::black;
      if (auto_color) text_color = control::get_auto_color().second;
      csp->text.color(text_color);
    } else return unexpected_error(errors::ui_invalid_slotid);
    return std::move(lbl);
  }

  template<typename Self> decltype(auto) text(this Self& self) { return create_accessor<text_accessor>(self); }

  std::expected<void, error_trace> fit_to_text() {
    const auto csp = system::slot_address<label>(_id);
    if (!csp) return unexpected_error(errors::ui_invalid_slotid);
    if (auto res = csp->text.update(); !res) return unexpected_error(res.error());
    const auto inner = csp->text.bounds() + csp->padding.xy() + csp->padding.zw();
    csp->constrained = {true, true};
    csp->required_size = vapply_r<float2>(yw::max, csp->minimum_size, inner);
    if (auto res = csp->make_messy(); !res) return unexpected_error(res.error());
    return {};
  }
};
} // namespace yw::ui
