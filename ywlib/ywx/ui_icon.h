#pragma once
#include "ywx/icon.h"
#include "ywx/ui_control.h"

namespace yw::ui {

class icon : public plain {
public:
  class slot : public plain::slot {
  public:
    float4 padding = float4::fill(5.0f);
    yw::icon icon;
    color fill_color = colors::transparent;
    color stroke_color = colors::black;
    float stroke_width = 1.0f;

    virtual float2 demand_survey() const noexcept {
      const auto isz = icon.size() + padding.xy() + padding.zw();
      auto result = float2(ucc.x ? 0.0f : size.x, ucc.y ? 0.0f : size.y);
      result.x = yw::max(result.x, minimum_size.x, isz.x);
      result.y = yw::max(result.y, minimum_size.y, isz.y);
      return result + margin.xy() + margin.zw();
    }

    virtual void draw(float2 Pos, float2 Size) override {
      Pos += margin.xy();
      Size -= margin.xy() + margin.zw();
      const auto min_size = demand_survey() - margin.xy() - margin.zw();
      size.x = ucc.x ? Size.x : yw::max(size.x, min_size.x);
      size.y = ucc.y ? Size.y : yw::max(size.y, min_size.y);
      const auto extra = Size - size;
      pos = Pos;
      switch (alignment) {
      case ui::alignment::center: pos += extra * 0.5f; break;
      case ui::alignment::left: break;
      case ui::alignment::right: pos.x += extra.x; break;
      case ui::alignment::top: break;
      case ui::alignment::bottom: pos.y += extra.y; break;
      case ui::alignment::left_top: break;
      case ui::alignment::left_bottom: pos.y += extra.y; break;
      case ui::alignment::right_top: pos.x += extra.x; break;
      case ui::alignment::right_bottom: pos += extra; break;
      }
      draw();
    }

    virtual void draw() const override {
      plain::slot::draw();
      const auto isz = icon.size() + padding.xy() + padding.zw();
      brush.color(fill_color);
      draw_icon(pos + padding.xy() + (size - isz) * 0.5f, icon);
      brush.color(stroke_color);
      stroke_icon(pos + padding.xy() + (size - isz) * 0.5f, icon, stroke_width);
    }
  };

  using control::operator bool;
  icon() noexcept = default;
  icon(derived_from<unknown> auto& Layout) {
    if (auto res = create_control<icon>(Layout)) _id = *res;
  }

  const auto& padding() const { return unsafe_get(&slot::padding); }
  const auto& object() const { return unsafe_get(&slot::icon); }
  const auto& fill_color() const { return unsafe_get(&slot::fill_color); }
  const auto& stroke_color() const { return unsafe_get(&slot::stroke_color); }
  const auto& stroke_width() const { return unsafe_get(&slot::stroke_width); }

  auto& object() {
    if (const auto csp = system::slot_address<slot>(_id)) {
      csp->make_messy(); // アイコンのサイズ等が変更される可能性があるため
      return *csp;
    } else throw std::logic_error("Invalid member access");
  }

  void padding(const float4& p) { safe_set_size(&slot::padding, p); }
  void object(icon Icon) { safe_set(&slot::icon, std::move(Icon)); }
  void fill_color(const color& c) { safe_set(&slot::fill_color, c); }
  void stroke_color(const color& c) { safe_set(&slot::stroke_color, c); }
  void stroke_width(float w) { safe_set(&slot::stroke_width, w); }
};
}
