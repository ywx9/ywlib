#pragma once

#include "ywx/core.h"

namespace yw {

//////////////////////////////////////// MARK: svgpath

class svgpath {
private:
  comptr<::ID2D1PathGeometry> _geometry;
  D2D1_RECT_F _bounds{};
  float2 _size{};

  svgpath(comptr<::ID2D1PathGeometry>&& geom) : _geometry(std::move(geom)) {}

  static std::expected<comptr<::ID2D1PathGeometry>, error_trace> parse_svg_path(const std::string_view svg_path_str) {
    if (auto res = d2d.initialize(); !res) return unexpected_error(res.error());

    comptr<::ID2D1PathGeometry> geometry;
    auto hr = d2d.factory()->CreatePathGeometry(&geometry.get());
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreatePathGeometry failed", int32_t(hr));

    comptr<::ID2D1GeometrySink> sink;
    hr = geometry->Open(&sink.get());
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "Open sink failed", int32_t(hr));

    float current_x = 0.0f, current_y = 0.0f;
    float last_control_x = 0.0f, last_control_y = 0.0f;
    size_t pos = 0;
    char current_cmd = '\0';
    char last_cmd = '\0';
    bool use_relative = false;
    bool figure_open = false;

    auto skip_whitespace = [&]() {
      while (pos < svg_path_str.length() && std::isspace(svg_path_str[pos])) ++pos;
    };

    auto parse_number = [&]() -> std::expected<float, error_trace> {
      skip_whitespace();
      if (pos >= svg_path_str.length()) return unexpected_error(errors::invalid_argument, "unexpected end of path");

      size_t start = pos;
      if (svg_path_str[pos] == '-' || svg_path_str[pos] == '+') ++pos;

      bool has_digit = false;
      while (pos < svg_path_str.length() && std::isdigit(svg_path_str[pos])) {
        has_digit = true;
        ++pos;
      }

      if (pos < svg_path_str.length() && svg_path_str[pos] == '.') {
        ++pos;
        while (pos < svg_path_str.length() && std::isdigit(svg_path_str[pos])) {
          has_digit = true;
          ++pos;
        }
      }

      if (!has_digit) return unexpected_error(errors::invalid_argument, "invalid number in path");

      try {
        return std::stof(std::string(svg_path_str.substr(start, pos - start)));
      } catch (...) {
        return unexpected_error(errors::invalid_argument, "number parsing error");
      }
    };

    auto parse_command = [&]() -> char {
      skip_whitespace();
      if (pos >= svg_path_str.length()) return '\0';
      char cmd = svg_path_str[pos];
      if (std::isalpha(cmd)) {
        ++pos;
        return cmd;
      }
      return '\0';
    };

    while (pos < svg_path_str.length()) {
      skip_whitespace();
      if (pos >= svg_path_str.length()) break;

      char cmd = parse_command();
      if (cmd != '\0') {
        last_cmd = current_cmd;
        current_cmd = cmd;
        use_relative = std::islower(cmd);
        cmd = std::toupper(cmd);
      } else {
        cmd = std::toupper(current_cmd);
      }

      switch (cmd) {
      case 'M': { // moveto
        auto res_x = parse_number();
        if (!res_x) return unexpected_error(res_x.error());
        auto res_y = parse_number();
        if (!res_y) return unexpected_error(res_y.error());

        float x = res_x.value();
        float y = res_y.value();
        if (use_relative) {
          x += current_x;
          y += current_y;
        }

        // End previous figure if one is open
        if (figure_open) {
          sink->EndFigure(D2D1_FIGURE_END_OPEN);
        }

        sink->BeginFigure({x, y}, D2D1_FIGURE_BEGIN_FILLED);
        figure_open = true;
        current_x = x;
        current_y = y;
        last_control_x = x;
        last_control_y = y;
        break;
      }

      case 'L': { // lineto
        auto res_x = parse_number();
        if (!res_x) return unexpected_error(res_x.error());
        auto res_y = parse_number();
        if (!res_y) return unexpected_error(res_y.error());

        float x = res_x.value();
        float y = res_y.value();
        if (use_relative) {
          x += current_x;
          y += current_y;
        }

        sink->AddLine({x, y});
        current_x = x;
        current_y = y;
        last_control_x = x;
        last_control_y = y;
        break;
      }

      case 'H': { // horizontal lineto
        auto res_x = parse_number();
        if (!res_x) return unexpected_error(res_x.error());

        float x = res_x.value();
        if (use_relative) {
          x += current_x;
        }

        sink->AddLine({x, current_y});
        current_x = x;
        last_control_x = x;
        last_control_y = current_y;
        break;
      }

      case 'V': { // vertical lineto
        auto res_y = parse_number();
        if (!res_y) return unexpected_error(res_y.error());

        float y = res_y.value();
        if (use_relative) {
          y += current_y;
        }

        sink->AddLine({current_x, y});
        current_y = y;
        last_control_x = current_x;
        last_control_y = y;
        break;
      }

      case 'C': { // cubic bezier
        auto res_x1 = parse_number();
        if (!res_x1) return unexpected_error(res_x1.error());
        auto res_y1 = parse_number();
        if (!res_y1) return unexpected_error(res_y1.error());
        auto res_x2 = parse_number();
        if (!res_x2) return unexpected_error(res_x2.error());
        auto res_y2 = parse_number();
        if (!res_y2) return unexpected_error(res_y2.error());
        auto res_x = parse_number();
        if (!res_x) return unexpected_error(res_x.error());
        auto res_y = parse_number();
        if (!res_y) return unexpected_error(res_y.error());

        float x1 = res_x1.value(), y1 = res_y1.value();
        float x2 = res_x2.value(), y2 = res_y2.value();
        float x = res_x.value(), y = res_y.value();

        if (use_relative) {
          x1 += current_x; y1 += current_y;
          x2 += current_x; y2 += current_y;
          x += current_x; y += current_y;
        }

        D2D1_BEZIER_SEGMENT segments[1];
        segments[0].point1 = D2D1::Point2F(x1, y1);
        segments[0].point2 = D2D1::Point2F(x2, y2);
        segments[0].point3 = D2D1::Point2F(x, y);
        sink->AddBezier(segments);

        last_control_x = x2;
        last_control_y = y2;
        current_x = x;
        current_y = y;
        break;
      }

      case 'S': { // smooth cubic bezier
        auto res_x2 = parse_number();
        if (!res_x2) return unexpected_error(res_x2.error());
        auto res_y2 = parse_number();
        if (!res_y2) return unexpected_error(res_y2.error());
        auto res_x = parse_number();
        if (!res_x) return unexpected_error(res_x.error());
        auto res_y = parse_number();
        if (!res_y) return unexpected_error(res_y.error());

        float x2 = res_x2.value(), y2 = res_y2.value();
        float x = res_x.value(), y = res_y.value();

        if (use_relative) {
          x2 += current_x; y2 += current_y;
          x += current_x; y += current_y;
        }

        // Calculate reflected control point
        float x1 = current_x;
        float y1 = current_y;
        if (std::toupper(last_cmd) == 'C' || std::toupper(last_cmd) == 'S') {
          x1 = 2.0f * current_x - last_control_x;
          y1 = 2.0f * current_y - last_control_y;
        }

        D2D1_BEZIER_SEGMENT segments[1];
        segments[0].point1 = D2D1::Point2F(x1, y1);
        segments[0].point2 = D2D1::Point2F(x2, y2);
        segments[0].point3 = D2D1::Point2F(x, y);
        sink->AddBezier(segments);

        last_control_x = x2;
        last_control_y = y2;
        current_x = x;
        current_y = y;
        break;
      }

      case 'Q': { // quadratic bezier
        auto res_x1 = parse_number();
        if (!res_x1) return unexpected_error(res_x1.error());
        auto res_y1 = parse_number();
        if (!res_y1) return unexpected_error(res_y1.error());
        auto res_x = parse_number();
        if (!res_x) return unexpected_error(res_x.error());
        auto res_y = parse_number();
        if (!res_y) return unexpected_error(res_y.error());

        float x1 = res_x1.value(), y1 = res_y1.value();
        float x = res_x.value(), y = res_y.value();

        if (use_relative) {
          x1 += current_x; y1 += current_y;
          x += current_x; y += current_y;
        }

        D2D1_QUADRATIC_BEZIER_SEGMENT segment;
        segment.point1 = D2D1::Point2F(x1, y1);
        segment.point2 = D2D1::Point2F(x, y);
        sink->AddQuadraticBezier(segment);

        last_control_x = x1;
        last_control_y = y1;
        current_x = x;
        current_y = y;
        break;
      }

      case 'T': { // smooth quadratic bezier
        auto res_x = parse_number();
        if (!res_x) return unexpected_error(res_x.error());
        auto res_y = parse_number();
        if (!res_y) return unexpected_error(res_y.error());

        float x = res_x.value(), y = res_y.value();

        if (use_relative) {
          x += current_x; y += current_y;
        }

        // Calculate reflected control point
        float x1 = current_x;
        float y1 = current_y;
        if (std::toupper(last_cmd) == 'Q' || std::toupper(last_cmd) == 'T') {
          x1 = 2.0f * current_x - last_control_x;
          y1 = 2.0f * current_y - last_control_y;
        }

        D2D1_QUADRATIC_BEZIER_SEGMENT segment;
        segment.point1 = D2D1::Point2F(x1, y1);
        segment.point2 = D2D1::Point2F(x, y);
        sink->AddQuadraticBezier(segment);

        last_control_x = x1;
        last_control_y = y1;
        current_x = x;
        current_y = y;
        break;
      }

      case 'A': { // elliptical arc
        auto res_rx = parse_number();
        if (!res_rx) return unexpected_error(res_rx.error());
        auto res_ry = parse_number();
        if (!res_ry) return unexpected_error(res_ry.error());
        auto res_rot = parse_number();
        if (!res_rot) return unexpected_error(res_rot.error());
        auto res_large = parse_number();
        if (!res_large) return unexpected_error(res_large.error());
        auto res_sweep = parse_number();
        if (!res_sweep) return unexpected_error(res_sweep.error());
        auto res_x = parse_number();
        if (!res_x) return unexpected_error(res_x.error());
        auto res_y = parse_number();
        if (!res_y) return unexpected_error(res_y.error());

        float rx = std::abs(res_rx.value());
        float ry = std::abs(res_ry.value());
        float rot = res_rot.value();
        bool large_arc = res_large.value() != 0.0f;
        bool sweep = res_sweep.value() != 0.0f;

        float x = res_x.value();
        float y = res_y.value();
        if (use_relative) {
          x += current_x;
          y += current_y;
        }

        if (rx <= 0.0f || ry <= 0.0f) {
          sink->AddLine({x, y});
        } else {
          D2D1_ARC_SEGMENT arc{};
          arc.point = D2D1::Point2F(x, y);
          arc.size = D2D1::SizeF(rx, ry);
          arc.rotationAngle = rot;
          arc.sweepDirection = sweep ? D2D1_SWEEP_DIRECTION_CLOCKWISE : D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE;
          arc.arcSize = large_arc ? D2D1_ARC_SIZE_LARGE : D2D1_ARC_SIZE_SMALL;
          sink->AddArc(arc);
        }

        current_x = x;
        current_y = y;
        last_control_x = x;
        last_control_y = y;
        break;
      }

      case 'Z': { // closepath
        if (figure_open) {
          sink->EndFigure(D2D1_FIGURE_END_CLOSED);
          figure_open = false;
        }
        break;
      }

      default:
        sink->Release();
        return unexpected_error(errors::invalid_argument, "unsupported SVG path command");
      }
    }

    // Close any remaining open figure before closing the sink
    if (figure_open) sink->EndFigure(D2D1_FIGURE_END_OPEN);

    hr = sink->Close();
    if (FAILED(hr)) {
      sink->Release();
      return unexpected_error(errors::operation_failed, "Close sink failed", int32_t(hr));
    }
    sink.release();

    return geometry;
  }

public:
  svgpath() = default;

  // デフォルト以外のコンストラクタは隠す
  svgpath(const svgpath&) = delete;
  svgpath& operator=(const svgpath&) = delete;
  svgpath(svgpath&& other) noexcept : _geometry(std::move(other._geometry)), _bounds(other._bounds), _size(other._size) {}
  svgpath& operator=(svgpath&& other) noexcept {
    if (this == &other) return *this;
    _geometry = std::move(other._geometry);
    _bounds = other._bounds;
    _size = other._size;
    return *this;
  }

  explicit operator bool() const noexcept { return static_cast<bool>(_geometry); }
  explicit operator ::ID2D1PathGeometry*&() & noexcept { return _geometry.get(); }
  explicit operator ::ID2D1PathGeometry*() const& noexcept { return _geometry.get(); }

  ::ID2D1PathGeometry* get() const noexcept { return _geometry.get(); }
  const D2D1_RECT_F& bounds() const noexcept { return _bounds; }
  float2 size() const noexcept { return _size; }
  void size(float2 value) noexcept { _size = value; }
  float width() const noexcept { return _size.x; }
  void width(float value) noexcept { _size.x = value; }
  float height() const noexcept { return _size.y; }
  void height(float value) noexcept { _size.y = value; }

  /// creates svgpath from SVG path string
  /// \param svg_path_str SVG path string (e.g., "M10 10 L90 90 Z")
  /// \return svgpath on success, error_trace on failure
  static std::expected<svgpath, error_trace> create(float2 Size, const std::string_view svg_path_str) {
    if (auto res = parse_svg_path(svg_path_str)) {
      auto path = svgpath(std::move(res.value()));
      path._size = Size;
      return path;
    } else return unexpected_error(res.error());
  }

  /// creates a copy of an existing svgpath with a new geometry resource
  /// \param source source svgpath to copy
  /// \return copied svgpath on success, error_trace on failure
  static std::expected<svgpath, error_trace> create(const svgpath& source) {
    if (!source) return unexpected_error(errors::invalid_argument, "source svgpath not initialized");
    if (auto res = d2d.initialize(); !res) return unexpected_error(res.error());

    comptr<::ID2D1PathGeometry> new_geometry;
    auto hr = d2d.factory()->CreatePathGeometry(&new_geometry.get());
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreatePathGeometry failed", int32_t(hr));

    comptr<::ID2D1GeometrySink> sink;
    hr = new_geometry->Open(&sink.get());
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "Open sink failed", int32_t(hr));

    // Copy geometry from source to new geometry via sink
    hr = source._geometry->Simplify(D2D1_GEOMETRY_SIMPLIFICATION_OPTION_CUBICS_AND_LINES, nullptr, sink.get());
    if (FAILED(hr)) {
      sink.release();
      return unexpected_error(errors::operation_failed, "Simplify failed", int32_t(hr));
    }

    hr = sink->Close();
    sink.release();
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "Close sink failed", int32_t(hr));

    auto copied = svgpath(std::move(new_geometry));
    copied._size = source._size;
    copied._bounds = source._bounds;
    return copied;
  }
};

//////////////////////////////////////// MARK: draw_svgpath

inline std::expected<void, error_trace> draw_svgpath(
  float2 pos, float2 size, const svgpath& path, const color& c = colors::black, float1 border_width = 1.0f) {
  if (!drawing::d2d_drawing()) return unexpected_error(errors::invalid_operation, "drawing not begun");
  if (!path) return {};
  const float2 scale = size / path.size();
  comptr<ID2D1TransformedGeometry> transformed;
  D2D1_MATRIX_3X2_F matrix = D2D1::Matrix3x2F::Scale(scale.x, scale.y) * D2D1::Matrix3x2F::Translation(pos.x, pos.y);
  if (FAILED(d2d.factory()->CreateTransformedGeometry(path.get(), &matrix, &transformed.get())))
    return unexpected_error(errors::invalid_operation, "failed to create transformed geometry");
  d2d.solid_brush()->SetColor((const D2D1_COLOR_F*)&c);
  d2d.context()->DrawGeometry(transformed.get(), d2d.solid_brush(), border_width.x, d2d.stroke_style());
  return {};
}

inline std::expected<void, error_trace> draw_svgpath(
  float2 pos, float2 size, const svgpath& path, float1 border_width) {
  return draw_svgpath(pos, size, path, colors::black, border_width);
}

//////////////////////////////////////// MARK: fill_svgpath

inline std::expected<void, error_trace> fill_svgpath(
  float2 pos, float2 size, const svgpath& path, const color& c = colors::black) {
  if (!drawing::d2d_drawing()) return unexpected_error(errors::invalid_operation, "drawing not begun");
  if (!path) return {};
  const float2 scale = size / path.size();
  comptr<ID2D1TransformedGeometry> transformed;
  D2D1_MATRIX_3X2_F matrix = D2D1::Matrix3x2F::Scale(scale.x, scale.y) * D2D1::Matrix3x2F::Translation(pos.x, pos.y);
  if (FAILED(d2d.factory()->CreateTransformedGeometry(path.get(), &matrix, &transformed.get())))
    return unexpected_error(errors::invalid_operation, "failed to create transformed geometry");
  d2d.solid_brush()->SetColor((const D2D1_COLOR_F*)&c);
  d2d.context()->FillGeometry(transformed.get(), d2d.solid_brush());
  return {};
}
} // namespace yw
