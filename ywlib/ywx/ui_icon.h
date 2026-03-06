#pragma once
#include "ywx/bitmap.h"
#include "ywx/svgpath.h"

namespace yw::ui {

//////////////////////////////////////// MARK: ui::icon

class icon : public base {
public:
  using icon_variant = std::variant<std::monostate, yw::bitmap, svgpath>;

  class slot : public base::slot {
  public:
    icon_variant image{};
    color fill_color = colors::transparent;
    color border_color = colors::transparent;
    float2 padding{};
    bool colors_initialized = false;

    virtual void draw() const override {
      if (std::holds_alternative<std::monostate>(image)) {
        // Empty icon
      } else if (const auto* bitmap_ptr = std::get_if<yw::bitmap>(&image)) {
        if (*bitmap_ptr) {
          const auto draw_pos = pos + padding;
          const auto draw_size = size - padding * 2.0f;
          fill_rectangle(draw_pos, draw_size, fill_color);
          draw_rectangle(draw_pos, draw_size, border_color, 1.0f);
          draw_bitmap(draw_pos, draw_size, *bitmap_ptr);
        }
      } else if (const auto* svgpath_ptr = std::get_if<svgpath>(&image)) {
        if (*svgpath_ptr) {
          const auto draw_pos = pos + padding;
          const auto draw_size = size - padding * 2.0f;
          const auto path_size = svgpath_ptr->size();
          const float2 scale = draw_size / path_size;
          comptr<ID2D1TransformedGeometry> transformed;
          D2D1_MATRIX_3X2_F matrix = D2D1::Matrix3x2F::Scale(scale.x, scale.y)
            * D2D1::Matrix3x2F::Translation(draw_pos.x, draw_pos.y);
          if (SUCCEEDED(d2d.factory()->CreateTransformedGeometry(svgpath_ptr->get(), &matrix, &transformed.get()))) {
            d2d.solid_brush()->SetColor((const D2D1_COLOR_F*)&fill_color);
            d2d.context()->FillGeometry(transformed.get(), d2d.solid_brush());
            d2d.solid_brush()->SetColor((const D2D1_COLOR_F*)&border_color);
            d2d.context()->DrawGeometry(transformed.get(), d2d.solid_brush(), 1.0f, d2d.stroke_style());
          }
        }
      }
    }
  };

protected:
  slot* _icon_slot() const noexcept { return dynamic_cast<slot*>(_ui_slot()); }

public:
  using base::operator bool;

  const auto& image() const { return unsafe_get(&slot::image); }
  const auto& fill_color() const { return unsafe_get(&slot::fill_color); }
  const auto& border_color() const { return unsafe_get(&slot::border_color); }
  const auto& padding() const { return unsafe_get(&slot::padding); }

  void image(icon_variant value) {
    if (const auto s = _icon_slot(); s && !s->colors_initialized) {
      if (std::holds_alternative<yw::bitmap>(value)) {
        _set(&slot::fill_color, colors::transparent);
        _set(&slot::border_color, colors::transparent);
      } else if (std::holds_alternative<svgpath>(value)) {
        _set(&slot::fill_color, colors::white);
        _set(&slot::border_color, colors::black);
      }
      _set(&slot::colors_initialized, true);
    }
    _set(&slot::image, std::move(value));
  }
  void fill_color(const color& value) {
    _set(&slot::fill_color, value);
    _set(&slot::colors_initialized, true);
  }
  void border_color(const color& value) {
    _set(&slot::border_color, value);
    _set(&slot::colors_initialized, true);
  }
  void padding(float2 value) { _set(&slot::padding, value); }

  template<included_in<window&, none> Window>
  static std::expected<icon, error_trace> add(Window&& w, float2 Pos, float2 Size) {
    if (auto res = base::add<icon>(w, Pos, Size)) {
      return icon{std::move(res->first)};
    } else return unexpected_error(res.error());
  }
};

} // namespace yw::ui
