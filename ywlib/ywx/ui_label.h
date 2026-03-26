#pragma once
#include "ywx/text.h"
#include "ywx/ui_plain.h"

namespace yw::ui {

class label : public plain {
public:
  class slot : public plain::slot {
  public:
    yw::text text;
    color text_color = colors::black;
    float4 padding = float4::fill(5.0f);

    virtual float2 demand_survey() const noexcept {
      const auto tsz = text.size() + padding.xy() + padding.zw();
      auto result = float2(ucc.x ? 0.0f : size.x, ucc.y ? 0.0f : size.y);
      result.x = yw::max(result.x, minimum_size.x, tsz.x);
      result.y = yw::max(result.y, minimum_size.y, tsz.y);
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
      brush.color(text_color);
      const auto tsz = text.size() + padding.xy() + padding.zw();
      draw_text(pos + padding.xy() + (size - tsz) * 0.5f, text);
    }
  };

  using plain::operator bool;
  label() noexcept = default;
  label(derived_from<unknown> auto& Layout) {
    if (auto res = create_control<label>(Layout)) _id = *res;
    if (const auto csp = system::slot_address<slot>(_id))
      if (auto t = yw::text::create(L"")) csp->text = std::move(*t);
  }

  const auto& text() const { return unsafe_get(&slot::text); }
  const auto& text_color() const { return unsafe_get(&slot::text_color); }
  const auto& padding() const { return unsafe_get(&slot::padding); }

  auto& text() {
    if (const auto csp = system::slot_address<slot>(_id)) {
      csp->make_messy();
      return csp->text;
    } else throw std::logic_error("Invalid member access");
  }

  template<stringable S> void text(S&& Text) {
    if (const auto csp = system::slot_address<slot>(_id)) {
      csp->make_messy();
      csp->text(unicode<wchar_t>(static_cast<S&&>(Text)));
    }
  }
  void text_color(const color& c) { safe_set(&slot::text_color, c); }
  void padding(const float4& p) { safe_set(&slot::padding, p); }
};
} // namespace yw::ui
