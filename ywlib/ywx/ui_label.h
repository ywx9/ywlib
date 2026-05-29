#pragma once
#include "ywx/ui_control.h"
#include "ywx/text.h"

namespace yw::ui {

class label : public frame {
public:
  struct slot : public frame::slot {
    yw::text text;
    float4 text_padding = float4::fill(arbitrary_value);
    ui::alignment text_alignment = ui::alignment::center;

//-- overrides --//

    virtual std::expected<void, error_trace> draw() override {
      if (!visible) return {};
      if (auto res = draw_background(); !res) return unexpected_error(res.error());
      const auto tx_origin = calculate_content_origin(text.bounds(), text_padding, text_alignment);
      if (auto res = text.draw(tx_origin); !res) return unexpected_error(res.error());
      if (auto res = draw_foreground(); !res) return unexpected_error(res.error());
      return {};
    }

    virtual std::expected<float2, error_trace> calculate_minimum_size() override {
      if (auto res = text.update(); !res) return unexpected_error(res.error());
      const float2 inner = text.bounds() + text_padding.xy() + text_padding.zw();
      return vapply_r<float2>(yw::max, min_size, required_size * constrained, inner);
    }

    virtual std::expected<void, error_trace> ensure_minimum_size() override {
      if (auto res = text.update(); !res) return unexpected_error(res.error());
      if (auto res = calculate_minimum_size()) size = *res;
      else return unexpected_error(res.error());
      return {};
    }
  };

  class text_accessor : public accessor<label> {
    using accessor<label>::slot;

  public:
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
    const auto& alignment() const { return slot.text.alignment(); }
    auto& alignment(ui::alignment Alignment) {
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

    const auto& block_alignment() const { return slot.text_alignment; }
    auto& block_alignment(ui::alignment Alignment) {
      slot.text_alignment = Alignment;
      this->dirty = true;
      return *this;
    }

    const auto& block_padding() const { return slot.text_padding; }
    auto& block_padding(float4 Padding) {
      slot.text_padding = Padding;
      this->messy = true;
      return *this;
    }
  };

  using control::operator bool;
  label() noexcept = default;

  static std::expected<label, error_trace> add(derived_from<unknown> auto& Layout) {
    label lbl;
    if (auto res = create_control<label>(Layout)) lbl._id = *res;
    else return unexpected_error(res.error());
    if (const auto csp = system::slot_address<label>(lbl._id)) {
      csp->background_color = control::get_auto_color().first;
    } else return unexpected_error(errors::ui_invalid_slotid);
    return std::move(lbl);
  }

  template<typename Self> decltype(auto) text(this Self& self) { return create_accessor<text_accessor>(self); }
};
} // namespace yw::ui
