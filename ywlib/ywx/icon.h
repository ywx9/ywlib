#pragma once
#include <ywx/bitmap.h>
#include <ywx/svgpath.h>

namespace yw {

class icon {
public:
  struct bitmap {
    yw::bitmap object;
    float opacity = 1.0f;
    float2 size() const { return object ? float2(object.size()) : float2(); }
    std::expected<void, error> draw(float2 Pos) const {
      if (object && opacity > 0.0f)
        if (auto res = draw_bitmap(Pos, object, opacity); !res) return res.error().relay();
      return {};
    }
    std::expected<void, error> draw(float2 Pos, float2 Size) const {
      if (object && opacity > 0.0f)
        if (auto res = draw_bitmap(Pos, Size, object, opacity); !res) return res.error().relay();
      return {};
    }
  };
  struct vector {
    yw::svgpath object;
    color fill_color = colors::black;
    color stroke_color = colors::black;
    float stroke_width = 1.0f;
    float2 size() const { return object ? object.size() : float2(); }
    std::expected<void, error> draw(float2 Pos) const {
      if (!object) return {};
      if (fill_color.a > 0.0f) {
        brush::color(fill_color);
        if (auto res = fill_svgpath(Pos, object); !res) return res.error().relay();
      }
      if (stroke_color.a > 0.0f && stroke_width > 0.0f) {
        brush::color(stroke_color);
        if (auto res = stroke_svgpath(Pos, object, stroke_width); !res) return res.error().relay();
      }
      return {};
    }
    std::expected<void, error> draw(float2 Pos, float2 Size) const {
      if (!object) return {};
      if (fill_color.a > 0.0f) {
        brush::color(fill_color);
        if (auto res = fill_svgpath(Pos, Size, object); !res) return res.error().relay();
      }
      if (stroke_color.a > 0.0f && stroke_width > 0.0f) {
        brush::color(stroke_color);
        if (auto res = stroke_svgpath(Pos, Size, object, stroke_width); !res) return res.error().relay();
      }
      return {};
    }
  };

private:
  variant<bitmap, vector> _data;

public:
  icon() = default;
  icon(yw::bitmap Bitmap) : _data(bitmap{std::move(Bitmap)}) {}
  icon(yw::svgpath Vector) : _data(vector{std::move(Vector)}) {}

  bool empty() const noexcept { return _data.empty(); }
  bool is_bitmap() const noexcept { return _data.index() == 0; }
  bool is_vector() const noexcept { return _data.index() == 1; }

  float2 size() const {
    if (is_bitmap()) return _data.template get<0>().size();
    else if (is_vector()) return _data.template get<1>().size();
    else return float2();
  }

  auto get_bitmap(this auto&& self) noexcept { return self._data.template get_if<bitmap>(); }
  auto get_vector(this auto&& self) noexcept { return self._data.template get_if<vector>(); }

  std::expected<void, error> set(yw::bitmap Bitmap) noexcept {
    if (_data.emplace<0>(std::move(Bitmap))) return {};
    else return std::unexpected(error(errors::operation_failed, "failed to set bitmap icon"));
  }
  std::expected<void, error> set(yw::svgpath Vector) noexcept {
    if (_data.emplace<1>(std::move(Vector))) return {};
    else return std::unexpected(error(errors::operation_failed, "failed to set vector icon"));
  }

  std::expected<void, error> set_bitmap_opacity(float1 Opacity) noexcept {
    if (const auto bmp = _data.get_if<bitmap>()) bmp->opacity = Opacity.x;
    else return std::unexpected(error(errors::invalid_operation, "icon is not a bitmap"));
    return {};
  }

  std::expected<void, error> set_vector_fill_color(color FillColor) noexcept {
    if (const auto vec = _data.get_if<vector>()) vec->fill_color = FillColor;
    else return std::unexpected(error(errors::invalid_operation, "icon is not a vector"));
    return {};
  }
  std::expected<void, error> set_vector_stroke_color(color StrokeColor) noexcept {
    if (const auto vec = _data.get_if<vector>()) vec->stroke_color = StrokeColor;
    else return std::unexpected(error(errors::invalid_operation, "icon is not a vector"));
    return {};
  }
  std::expected<void, error> set_vector_stroke_width(float1 StrokeWidth) noexcept {
    if (const auto vec = _data.get_if<vector>()) vec->stroke_width = StrokeWidth.x;
    else return std::unexpected(error(errors::invalid_operation, "icon is not a vector"));
    return {};
  }

  std::expected<void, error> draw(float2 Pos) const {
    return visit([Pos](const auto& data) { return data.draw(Pos); }, _data);
  }

  std::expected<void, error> draw(float2 Pos, float2 Size) const {
    return visit([Pos, Size](const auto& data) { return data.draw(Pos, Size); }, _data);
  }
};
} // namespace yw
