#pragma once
#include <ywx/ui_frame.h>

namespace yw::ui {

class label : public frame {
public:
  struct slot : frame::slot {
    string<wchar_t> text{};
    color text_color = colors::black;
    font_config font = font_config::default_;
    float2 text_layout_size{};
    alignment text_align = alignment::center;
    comptr<IDWriteTextLayout> text_layout{};

    std::expected<void, error> update_text_layout() {
      text_layout.release();
      text_layout_size = {};
      if (text.empty()) return {};

      IDWriteTextFormat* format = nullptr;
      hresult_test(
        dwrite::factory()->CreateTextFormat,
        font.name.value_or(string<wchar_t>()).c_str(),
        nullptr,
        DWRITE_FONT_WEIGHT(font.weight.value_or(font_weight::normal)),
        DWRITE_FONT_STYLE(font.style.value_or(font_style::normal)),
        DWRITE_FONT_STRETCH(font.stretch.value_or(font_stretch::normal)),
        font.size.value_or(16.0f),
        L"",
        &format);
      comptr<IDWriteTextFormat> text_format;
      text_format.reset(format);
      hresult_test(format->SetTextAlignment, DWRITE_TEXT_ALIGNMENT_LEADING);
      hresult_test(format->SetParagraphAlignment, DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
      hresult_test(format->SetWordWrapping, DWRITE_WORD_WRAPPING_NO_WRAP);

      IDWriteTextLayout* layout = nullptr;
      hresult_test(
        dwrite::factory()->CreateTextLayout,
        text.c_str(),
        UINT32(text.size()),
        format,
        16384.0f,
        16384.0f,
        &layout);
      text_layout.reset(layout);

      DWRITE_TEXT_METRICS metrics{};
      hresult_test(layout->GetMetrics, &metrics);
      text_layout_size = float2(metrics.widthIncludingTrailingWhitespace, metrics.height);
      hresult_test(layout->SetMaxWidth, yw::max(text_layout_size.x, 1.0f));
      hresult_test(layout->SetMaxHeight, yw::max(text_layout_size.y, 1.0f));
      return {};
    }

    std::expected<void, error> ensure_text_layout() {
      if (text_layout || text.empty()) return {};
      return update_text_layout();
    }

    float2 _text_offset() const noexcept {
      constexpr float c[]{0.5f, 0.0f, 1.0f};
      const float2 cc{c[unsigned(text_align) % 3], c[unsigned(text_align) / 3 % 3]};
      return (size - text_layout_size) * cc;
    }

    virtual std::expected<float2, error> get_necessary_size() const override {
      const auto sp = const_cast<slot*>(this);
      if (auto res = sp->ensure_text_layout(); !res) return res.error().relay();
      const auto inner = margin.xy() + margin.zw();
      return vapply_r<float2>(_necessary_size, policy, minimum_size, required_size, text_layout_size + inner);
    }

    virtual std::expected<void, error> redraw() override {
      if (!visible) return {};
      if (auto res = _draw_background(); !res) return res.error().relay();
      if (!text.empty()) {
        if (auto res = ensure_text_layout(); !res) return res.error().relay();
        brush::color(text_color);
        if (auto res = draw_text(pos + _text_offset(), text_layout.get()); !res) return res.error().relay();
      }
      if (auto res = _draw_foreground(); !res) return res.error().relay();
      return {};
    }
  };

  label() noexcept = default;

  label(derived_from<interface> auto& Parent, bool AutoColor = true, const source_line& sl = here()) {
    if (auto res = slot::create<label>(Parent, AutoColor, sl)) {
      const auto sp = *res;
      _id = sp->id;
      sp->text_color = std::exchange(sp->colors.border, colors::transparent);
    } else res.error().add_footprint().go_off(sl);
  }

  static std::expected<label, error> create(
    derived_from<interface> auto& Parent, bool AutoColor = true, const source_line& sl = here()) {
    label l;
    if (auto res = slot::create<label>(Parent, AutoColor, sl)) {
      const auto sp = *res;
      l._id = sp->id;
      sp->text_color = std::exchange(sp->colors.border, colors::transparent);
      return l;
    } else return res.error().relay();
  }

  const auto& text() const noexcept { ywlib_control_get(this, text); }
  const auto& text_color() const noexcept { ywlib_control_get(this, text_color); }
  const auto& font() const noexcept { ywlib_control_get(this, font); }
  const auto& text_layout_size() const noexcept { ywlib_control_get(this, text_layout_size); }
  const auto& text_align() const noexcept { ywlib_control_get(this, text_align); }

  auto& text(string<wchar_t> v) noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->text = std::move(v);
    sp->text_layout.release();
    sp->text_layout_size = {};
    if (auto res = sp->make_layout_dirty(); !res) res.error().go_off();
    return *this;
  }
  auto& text_color(const color& c) noexcept { ywlib_control_set(this, text_color, c, paint_dirty); }
  auto& font(font_config v) noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->font = std::move(v);
    sp->text_layout.release();
    sp->text_layout_size = {};
    if (auto res = sp->make_layout_dirty(); !res) res.error().go_off();
    return *this;
  }

  auto& text_align(alignment v) noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->text_align = v;
    if (auto res = sp->make_paint_dirty(); !res) res.error().go_off();
    return *this;
  }
};
} // namespace yw::ui
