#pragma once
#include "ywx/ui_control.h"
#include "ywx/uip_text.h"

namespace yw::ui {

class label : public frame {
public:
  struct slot : public frame::slot {
    yw::text text;
    ui::alignment content_alignment = ui::alignment::center;
    float4 padding = float4::fill(arbitrary_value);

    //-- overrides --//

    virtual std::expected<void, error_trace> draw() override {
      if (!visible) return {};
      if (auto res = draw_background(); !res) return unexpected_error(res.error());
      const auto tx_origin = calculate_inner_origin(text.layout_size(), padding, content_alignment);
      if (auto res = text.draw(tx_origin); !res) return unexpected_error(res.error());
      if (auto res = draw_foreground(); !res) return unexpected_error(res.error());
      return {};
    }

    virtual std::expected<float2, error_trace> calculate_minimum_size() const override {
      const float2 inner = text.layout_size() + padding.xy() + padding.zw();
      return vapply_r<float2>(yw::max, min_size, required_size * constrained, inner);
    }

    virtual std::expected<void, error_trace> ensure_minimum_size() override {
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
      return *this;
    }
    const auto& color() const { return slot.text.color(); }
    auto& color(const yw::color& Color) {
      slot.text.color(Color);
      return *this;
    }
    const auto& font() const { return slot.text.font(); }
    auto& font(font_config Font) {
      slot.text.font(std::move(Font));
      return *this;
    }
    const auto& alignment() const { return slot.text.text_alignment(); }
    auto& alignment(text_alignment Alignment) {
      slot.text.text_alignment(Alignment);
      return *this;
    }
    const auto& block_alignment() const { return slot.text.block_alignment(); }
    auto& block_alignment(ui::alignment BlockAlignment) {
      slot.text.block_alignment(BlockAlignment);
      return *this;
    }
    const auto& padding() const { return slot.padding; }
    auto& padding(float4 Padding) {
      slot.padding = Padding;
      messy = true;
      return *this;
    }

    const auto& font_name() const { return slot.text.font().name.value_or(L""); }
    auto& font_name(std::wstring FontName) { return font({.name = std::move(FontName)}); }

    const auto& font_size() const { return slot.text.font().size.value_or(0.0f); }
    auto& font_size(float FontSize) { return font({.size = FontSize}); }

    const auto& font_weight() const { return slot.text.font().weight.value_or(yw::font_weight::normal); }
    auto& font_weight(yw::font_weight FontWeight) { return font({.weight = FontWeight}); }

    const auto& font_style() const { return slot.text.font().style.value_or(yw::font_style::normal); }
    auto& font_style(yw::font_style FontStyle) { return font({.style = FontStyle}); }

    const auto& font_stretch() const { return slot.text.font().stretch.value_or(yw::font_stretch::normal); }
    auto& font_stretch(yw::font_stretch FontStretch) { return font({.stretch = FontStretch}); }
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

  template<typename Self> decltype(auto) text(this Self& self) {
    const auto csp = system::slot_address<label>(self._id);
    if (!csp) fatal_error(errors::ui_invalid_slotid);
    if constexpr (!is_const<Self>) return csp->text.access();
    else return std::as_const(csp->text.access());
  }
  auto& fit_to_text() {
    const auto csp = system::slot_address<label>(_id);
    if (!csp) fatal_error(errors::ui_invalid_slotid);
    this->core().size(csp->text.bounds());
    return *this;
  }
};
} // namespace yw::ui
