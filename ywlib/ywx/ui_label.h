#pragma once
#include "ywx/text.h"
#include "ywx/ui_frame.h"
#include "ywx/window.h"

namespace yw::ui {

class label : public frame {
public:
  struct slot : public frame::slot {
    yw::text text;
    yw::alignment text_alignment = yw::alignment::center;

    float2 text_origin() const noexcept {
      constexpr float c[]{0.5f, 0.0f, 1.0f};
      const float2 cc{c[unsigned(text_alignment) % 3], c[unsigned(text_alignment) / 3 % 3]};
      return pos + (size - text.bounds()) * cc;
    }

    /// MARK: overrides

    virtual std::expected<float2, error> calculate_necessary_size() const override {
      return vapply_r<float2>(calculate_necessary_width, size_policy, minimum_size, required_size, text.bounds());
    }

    virtual std::expected<void, error> ensure_necessary_size() override {
      if (auto res = text.update(); !res) return res.error().relay();
      if (auto res = calculate_necessary_size()) return size = *res, std::expected<void, error>{};
      else return res.error().relay();
    }

    virtual std::expected<void, error> draw() const override {
      if (!visible) return {};
      if (auto res = draw_background(); !res) return res.error().relay();
      if (auto res = text.draw(text_origin()); !res) return res.error().relay();
      if (auto res = draw_foreground(); !res) return res.error().relay();
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
          if (auto res = slot.text.update(); !res) res.error().print_as_fatal();
      } catch (...) { error(errors::unreachable, "Unhandled exception in ui::label::text_accessor destructor").print_as_fatal(); }
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
    auto& text_alignment(yw::alignment Alignment) {
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
    auto& alignment(yw::alignment Alignment) {
      slot.text_alignment = Alignment;
      this->dirty = true;
      return *this;
    }
  };

  /// MARK: handle functions

  using control::operator bool;
  label() noexcept = default;

  template<typename Layout> static std::expected<label, error> add(Layout& Layout_) {
    const auto lsp = interface::slot::slots.get(Layout_.id());
    if (!lsp) return std::unexpected(error(errors::invalid_slotid));
    if (!lsp->attachable()) return std::unexpected(error(errors::invalid_operation, "Layout is not attachable"));
    const auto id = interface::slot::add<label>();
    const auto csp = interface::slot::get<label>(id);
    if (!csp) return std::unexpected(error(errors::invalid_slotid));
    csp->id = id;
    csp->layout_id = Layout_.id();
    if (const auto parent_control = dynamic_cast<control::slot*>(lsp)) csp->window_id = parent_control->window_id;
    else if (const auto parent_window = dynamic_cast<window::handle<window::type::unknown>::slot*>(lsp)) csp->window_id = parent_window->id;
    else {
      interface::slot::slots.erase(id);
      return std::unexpected(error(errors::invalid_operation, "Unsupported layout parent"));
    }
    if (auto res = lsp->attach(id); !res) {
      interface::slot::slots.erase(id);
      return res.error().relay();
    }
    label lbl;
    lbl._id = id;
    return std::move(lbl);
  }

  ywlib_make_accessor(text_accessor, label);
};
} // namespace yw::ui
