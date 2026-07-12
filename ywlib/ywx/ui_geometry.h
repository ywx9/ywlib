#pragma once
#include <ywx/control.h>
#include <ywx/svgpath.h>

namespace yw::ui {

class geometry : public control {
public:
  enum class geometry_size_mode : unsigned char {
    none,
    stretch
  };

  struct slot : control::slot {
    svgpath content{};
    geometry_size_mode content_mode = geometry_size_mode::none;
    color fill_color = colors::black;
    color stroke_color = colors::transparent;
    float stroke_width = 1.0f;
    bool crop_content = true;

    static std::expected<slot*, error> create(derived_from<interface> auto& Parent, const source_line& sl) {
      const auto psp = interface::slot::get<interface>(Parent.id());
      if (!psp) return std::unexpected(error(errors::invalid_slotid));
      if (!psp->attachable()) return std::unexpected(error(errors::invalid_operation, "not attachable"));
      const auto temp_id = interface::slot::add<yw::ui::geometry>();
      const auto sp = interface::slot::get<yw::ui::geometry>(temp_id);
      if (!sp) return std::unexpected(error(errors::slot_creation_failed));
      sp->id = temp_id;
      sp->source = sl;
      if (auto res = psp->attach(temp_id); !res) {
        interface::slot::slots.erase(temp_id);
        return res.error().relay();
      }
      return sp;
    }

    float2 _content_size() const noexcept { return content.size(); }

    float2 _content_offset(float2 DrawSize) const noexcept {
      constexpr float c[]{0.5f, 0.0f, 1.0f};
      const float2 cc{c[unsigned(align) % 3], c[unsigned(align) / 3 % 3]};
      return (size - DrawSize) * cc;
    }

    virtual std::expected<float2, error> get_necessary_size() const override {
      return vapply_r<float2>(_necessary_size, policy, minimum_size, required_size, _content_size());
    }

    virtual std::expected<void, error> redraw() override {
      if (!visible || !content) return {};
      const auto draw_size = content_mode == geometry_size_mode::stretch ? size : _content_size();
      const auto draw_pos = pos + _content_offset(draw_size);
      if (crop_content) d2d::push_layer(geometry.get());

      if (fill_color.a > 0.0f) {
        brush::color(fill_color);
        if (auto res = content_mode == geometry_size_mode::stretch ? fill_svgpath(draw_pos, draw_size, content)
                                                                   : fill_svgpath(draw_pos, content);
            !res) {
          if (crop_content) d2d::pop_layer();
          return res.error().relay();
        }
      }

      if (stroke_color.a > 0.0f && stroke_width > 0.0f) {
        brush::color(stroke_color);
        if (auto res = content_mode == geometry_size_mode::stretch
                         ? stroke_svgpath(draw_pos, draw_size, content, stroke_width)
                         : stroke_svgpath(draw_pos, content, stroke_width);
            !res) {
          if (crop_content) d2d::pop_layer();
          return res.error().relay();
        }
      }

      if (crop_content) d2d::pop_layer();
      return {};
    }
  };

  using control::operator bool;
  geometry() noexcept = default;

  geometry(derived_from<interface> auto& Parent, const source_line& sl = here()) {
    if (auto res = slot::create(Parent, sl)) _id = (*res)->id;
    else res.error().add_footprint().go_off(sl);
  }

  static std::expected<geometry, error> create(derived_from<interface> auto& Parent, const source_line& sl = here()) {
    geometry g;
    if (auto res = slot::create(Parent, sl)) {
      const auto sp = *res;
      g._id = sp->id;
      sp->radius = {};
      return g;
    } else return res.error().relay();
  }

  const auto& content() const noexcept { ywlib_control_get(content); }
  const auto& content_mode() const noexcept { ywlib_control_get(content_mode); }
  const auto& fill_color() const noexcept { ywlib_control_get(fill_color); }
  const auto& stroke_color() const noexcept { ywlib_control_get(stroke_color); }
  const auto& stroke_width() const noexcept { ywlib_control_get(stroke_width); }
  const auto& crop_content() const noexcept { ywlib_control_get(crop_content); }

  auto& content(this auto& self, svgpath p) noexcept { ywlib_control_set(content, std::move(p), messy); }
  auto& content_mode(this auto& self, geometry_size_mode m) noexcept { ywlib_control_set(content_mode, m, dirty); }
  auto& fill_color(this auto& self, const color& c) noexcept { ywlib_control_set(fill_color, c, dirty); }
  auto& stroke_color(this auto& self, const color& c) noexcept { ywlib_control_set(stroke_color, c, dirty); }
  auto& stroke_width(this auto& self, float1 f) noexcept { ywlib_control_set(stroke_width, f.x, dirty); }
  auto& crop_content(this auto& self, bool b) noexcept { ywlib_control_set(crop_content, b, dirty); }
};
} // namespace yw::ui
