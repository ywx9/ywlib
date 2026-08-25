#pragma once
#include <ywx/bitmap.h>
#include <ywx/svgpath.h>

namespace yw {

class icon {
public:
  using bitmap = yw::bitmap;
  struct vector {
    svgpath path;
    color fill_color = colors::black;
    color stroke_color = colors::black;
    float stroke_width = 1.0f;
  };
private:
  variant<bitmap, vector> _data;

public:
  icon() = default;
  icon(yw::bitmap Bitmap) : _data(bitmap{std::move(Bitmap)}) {}
  icon(yw::svgpath Vector) : _data(vector{.path = std::move(Vector)}) {}
  icon(vector Vector) : _data(std::move(Vector)) {}

  bool empty() const noexcept { return _data.empty(); }
  bool is_bitmap() const noexcept { return _data.index() == 0; }
  bool is_vector() const noexcept { return _data.index() == 1; }

  float2 size() const {
    if (is_bitmap()) return _data.template get<0>().size();
    else if (is_vector()) return _data.template get<1>().path.size();
    else return float2();
  }

  auto get_if_bitmap(this auto&& self) noexcept { return self._data.template get_if<bitmap>(); }
  auto get_if_vector(this auto&& self) noexcept { return self._data.template get_if<vector>(); }

  auto& get_bitmap(this auto&& self) noexcept {
    if (!self.is_bitmap()) error(errors::invalid_operation, "icon is not bitmap").go_off();
    return self._data.template get<0>();
  }
  auto& get_vector(this auto&& self) noexcept {
    if (!self.is_vector()) error(errors::invalid_operation, "icon is not vector").go_off();
    return self._data.template get<1>();
  }

  std::expected<void, error> set(yw::bitmap Bitmap) noexcept {
    if (_data.emplace<0>(std::move(Bitmap))) return {};
    else return std::unexpected(error(errors::operation_failed, "failed to set bitmap icon"));
  }
  std::expected<void, error> set(yw::svgpath Vector) noexcept {
    if (_data.emplace<1>(vector{.path = std::move(Vector)})) return {};
    else return std::unexpected(error(errors::operation_failed, "failed to set vector icon"));
  }
  std::expected<void, error> set(vector Vector) noexcept {
    if (_data.emplace<1>(std::move(Vector))) return {};
    else return std::unexpected(error(errors::operation_failed, "failed to set vector icon"));
  }
};

/// MARK: draw_icon

inline std::expected<void, error> draw_icon(float2 Pos, const icon& i) {
  if (i.is_bitmap()) {
    if (auto res = draw_bitmap(Pos, i.get_bitmap())) return {};
    else return res.error().relay();
  } else if (i.is_vector()) {
    const auto& vector = i.get_vector();
    if (vector.fill_color.a > 0.0f)
      if (auto res = fill_svgpath(Pos, vector.path, vector.fill_color); !res) return res.error().relay();
    if (vector.stroke_color.a > 0.0f && vector.stroke_width > 0.0f)
      if (auto res = stroke_svgpath(Pos, vector.path, vector.stroke_color, vector.stroke_width); !res)
        return res.error().relay();
    return {};
  } else return std::unexpected(error(errors::invalid_operation, "empty icon"));
}

inline std::expected<void, error> draw_icon(float2 Pos, float2 Size, const icon& i) {
  if (i.is_bitmap()) {
    if (auto res = draw_bitmap(Pos, Size, i.get_bitmap())) return {};
    else return res.error().relay();
  } else if (i.is_vector()) {
    const auto& vector = i.get_vector();
    if (vector.fill_color.a > 0.0f)
      if (auto res = fill_svgpath(Pos, Size, vector.path, vector.fill_color); !res) return res.error().relay();
    if (vector.stroke_color.a > 0.0f && vector.stroke_width > 0.0f)
      if (auto res = stroke_svgpath(Pos, Size, vector.path, vector.stroke_color, vector.stroke_width); !res)
        return res.error().relay();
    return {};
  } else return std::unexpected(error(errors::invalid_operation, "empty icon"));
}
} // namespace yw
