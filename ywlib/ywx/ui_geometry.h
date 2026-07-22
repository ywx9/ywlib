#pragma once
#include <variant>

#include <ywx/control.h>
#include <ywx/svgpath.h>

namespace yw::ui {

class geometry : public control {
public:
  struct scaled_content {
    float2 value = float2::fill(1.0f);
  };

  struct sized_content {
    float2 value{};
  };

  struct slot : control::slot {
    svgpath content{};
    std::variant<scaled_content, sized_content> content_transform = scaled_content{};
    color fill_color = colors::black;
    color stroke_color = colors::transparent;
    float stroke_width = 1.0f;

    float2 _original_content_size() const noexcept {
      if (!content) return {};
      return content.size();
    }

    float2 _content_scale() const noexcept {
      if (!content) return float2::fill(1.0f);
      if (const auto* scaled = std::get_if<scaled_content>(&content_transform)) return scaled->value;
      const auto original = _original_content_size();
      if (original.x <= 0.0f || original.y <= 0.0f) return float2::fill(1.0f);
      return std::get<sized_content>(content_transform).value / original;
    }

    float2 _content_size() const noexcept {
      if (!content) return {};
      if (const auto* sized = std::get_if<sized_content>(&content_transform)) return sized->value;
      return _original_content_size() * std::get<scaled_content>(content_transform).value;
    }

    float2 _content_offset(float2 DrawSize) const noexcept {
      constexpr float c[]{0.5f, 0.0f, 1.0f};
      const float2 cc{c[unsigned(align) % 3], c[unsigned(align) / 3 % 3]};
      return (size - DrawSize) * cc;
    }

    virtual std::expected<float2, error> get_necessary_size() const override {
      return calc_necessary_size_by_policy(_content_size());
    }

    virtual std::expected<void, error> redraw() override {
      if (!visible || !content) return {};
      const auto draw_size = _content_size();
      const auto draw_pos = pos + _content_offset(draw_size);
      d2d::push_layer(geometry.get());

      if (fill_color.a > 0.0f) {
        brush::color(fill_color);
        if (auto res = fill_svgpath(draw_pos, draw_size, content); !res) {
          d2d::pop_layer();
          return res.error().relay();
        }
      }

      if (stroke_color.a > 0.0f && stroke_width > 0.0f) {
        brush::color(stroke_color);
        if (auto res = stroke_svgpath(draw_pos, draw_size, content, stroke_width); !res) {
          d2d::pop_layer();
          return res.error().relay();
        }
      }

      d2d::pop_layer();
      return {};
    }

    virtual std::expected<void, error> apply_color_theme(const yw::ui::color_theme& Theme, bool Recursive) override {
      fill_color = Theme.text;
      stroke_color = colors::transparent;
      make_dirty();
      return {};
    }
  };

  using control::operator bool;
  geometry() noexcept = default;

  geometry(derived_from<interface> auto& Parent, const source_line& sl = here()) {
    if (auto res = create(Parent, sl)) *this = std::move(*res);
    else res.error().add_footprint().go_off(sl);
  }

  static std::expected<geometry, error> create(derived_from<interface> auto& Parent, const source_line& sl = here()) {
    geometry g;
    const auto temp_id = make_slot<geometry>();
    const auto sp = get_slot<geometry>(temp_id);
    if (!sp) return std::unexpected(error(errors::slot_creation_failed));
    const auto psp = get_slot<control>(Parent.id());
    if (!psp) return std::unexpected(error(errors::invalid_slotid));
    if (auto res = psp->attach(temp_id); !res) {
      slot::slots.erase(temp_id);
      return res.error().relay();
    }
    g._id = temp_id;
    sp->id = temp_id;
    sp->window_id = psp->get_window_id();
    sp->radius = {};
    if (auto res = sp->apply_current_color_theme(false); !res) return res.error().relay();
    return g;
  }

  const auto& content() const noexcept { ywlib_control_get(content); }
  float2 content_size() const noexcept {
    if (const auto sp = get_slot(this); !sp) {
      error(errors::invalid_slotid).fizzle_out();
      return float2{};
    } else return sp->_content_size();
  }
  float2 content_scale() const noexcept {
    if (const auto sp = get_slot(this); !sp) {
      error(errors::invalid_slotid).fizzle_out();
      return float2::fill(1.0f);
    } else return sp->_content_scale();
  }
  const auto& fill_color() const noexcept { ywlib_control_get(fill_color); }
  const auto& stroke_color() const noexcept { ywlib_control_get(stroke_color); }
  const auto& stroke_width() const noexcept { ywlib_control_get(stroke_width); }

  auto& content(this auto& self, svgpath p) noexcept { ywlib_control_set(content, std::move(p), messy); }
  auto& content_size(this auto& self, float2 v) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    if (v.x <= 0.0f || v.y <= 0.0f) {
      error(errors::invalid_argument, format("content_size must be positive: ", v)).go_off();
      return self;
    }
    sp->content_transform = sized_content{v};
    sp->make_messy();
    return self;
  }
  auto& content_scale(this auto& self, float2 v) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    if (v.x <= 0.0f || v.y <= 0.0f) {
      error(errors::invalid_argument, format("content_scale must be positive: ", v)).go_off();
      return self;
    }
    sp->content_transform = scaled_content{v};
    sp->make_messy();
    return self;
  }
  auto& content_scale(this auto& self, arithmetic auto v) noexcept { return self.content_scale(float2::fill(float(v))); }
  auto& fill_color(this auto& self, const color& c) noexcept { ywlib_control_set(fill_color, c, dirty); }
  auto& stroke_color(this auto& self, const color& c) noexcept { ywlib_control_set(stroke_color, c, dirty); }
  auto& stroke_width(this auto& self, float1 f) noexcept { ywlib_control_set(stroke_width, f.x, dirty); }
};
} // namespace yw::ui
