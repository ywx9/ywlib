#pragma once
#include <apps/bitmap.h>
#include <apps/control.h>
#include <core/optional.h>

namespace yw::ui {

class frame : public control {
public:
protected:
  optional<color> _background_color;
  optional<color> _border_color;
  float4 _padding = float4::fill(arbitrary_value);
  float2 _radius = float2::fill(arbitrary_value);
  float _border_thickness = 1.0f;
  /// 内容の描画に必要な最小サイズを計算する (paddingを除く)
  virtual result<float2> _calculate_content_size() { return float2(); }
  /// 必要な最小サイズを計算する
  virtual result<float2> _calculate_minimum_size() override {
    float2 content_size = _padding.xy() + _padding.zw();
    if (auto res = _calculate_content_size()) content_size += *res;
    else return res.relay();
    return vapply_r<float2>(yw::max, content_size, _desired_size * _desired);
  }
  /// 現在の配置でジオメトリを更新する
  virtual result<void> _update_geometry() override {
    ID2D1RoundedRectangleGeometry* geometry = nullptr;
    const auto left_top = _current_pos;
    const auto right_bottom = _current_pos + _current_size;
    const D2D1_ROUNDED_RECT rounded_rect{
      D2D1::RectF(left_top[0], left_top[1], right_bottom[0], right_bottom[1]), _radius[0], _radius[1]};
    if (const auto hr = d2d::factory()->CreateRoundedRectangleGeometry(&rounded_rect, &geometry); FAILED(hr))
      return error(errors::operation_failed, "CreateRoundedRectangleGeometry failed");
    _geometry.reset(geometry);
    return {};
  }
  /// 背景を描画する
  virtual result<void> _draw_background() {
    if (const auto& bgc = background_color(); bgc.a <= 0.0f) return {};
    else if (auto res = fill_geometry(_geometry, bgc); !res) return res.relay();
    return {};
  }
  /// 内容を描画する
  virtual result<void> _draw_content() { return {}; }
  /// 前景を描画する
  virtual result<void> _draw_foreground() {
    if (const auto& bc = border_color(); bc.a <= 0.0f || _border_thickness <= 0.0f) return {};
    else if (auto res = stroke_geometry(_geometry, bc, _border_thickness); !res) return res.relay();
    return {};
  }
  /// 描画する
  virtual result<void> _draw() override {
    if (auto res = control::_draw(); !res) return res.relay();
    if (auto res = _draw_background(); !res) return res.relay();
    if (auto res = _draw_content(); !res) return res.relay();
    if (auto res = _draw_foreground(); !res) return res.relay();
    return {};
  }

public:
  ~frame() noexcept = default;
  frame() noexcept = default;
  frame(frame&& f) noexcept = default;
  frame& operator=(frame&& f) noexcept = default;

  const color& background_color() const {
    if (_background_color) return *_background_color;
    else if (const auto ct = window_system::get_color_theme(_window)) return ct->surface;
    else return colors::transparent;
  }
  result<void> background_color(color c) {
    _background_color = c;
    window_system::make_dirty(_window);
    return {};
  }
  result<void> background_color(is_none auto) {
    _background_color.reset();
    window_system::make_dirty(_window);
    return {};
  }

  const color& border_color() const {
    if (_border_color) return *_border_color;
    else if (const auto ct = window_system::get_color_theme(_window)) return ct->outline;
    else return colors::transparent;
  }
  result<void> border_color(color c) {
    _border_color = c;
    window_system::make_dirty(_window);
    return {};
  }
  result<void> border_color(is_none auto) {
    _border_color.reset();
    window_system::make_dirty(_window);
    return {};
  }

  float4 padding() const { return _padding; }
  result<void> padding(float4 p) {
    _padding = p;
    window_system::make_messy(_window);
    return {};
  }

  float2 radius() const { return _radius; }
  result<void> radius(float2 r) {
    _radius = r;
    _geometry_dirty = true;
    window_system::make_dirty(_window);
    return {};
  }

  float border_thickness() const { return _border_thickness; }
  result<void> border_thickness(float t) {
    _border_thickness = t;
    window_system::make_dirty(_window);
    return {};
  }
};
} // namespace yw::ui
