#pragma once
#include <yw/array.h>
#include <yw/backend.h>
#include <yw/math.h>
#include <yw/matrix.h>

namespace yw::geom {

template<template<backend> typename Geometry> struct remeshing_option {};

/// MARK: geom::vertex

template<backend Backend> struct vertex;

template<> struct vertex<cpu> {
  double4 position = {0, 0, 0, 1};
};

template<> struct vertex<gpu> {
  /// packs a vector4 into a 32-bit unsigned integer (for normals and tangents)
  static constexpr uint32_t pack(double4 v) noexcept {
    return std::bit_cast<uint32_t>(
      (static_cast<int32_t>(v.x * 1023) & 0x3FF) | ((static_cast<int32_t>(v.y * 1023) & 0x3FF) << 10) |
      ((static_cast<int32_t>(v.z * 1023) & 0x3FF) << 20) | (int32_t(v.w < 0) << 30));
  }
  /// packs a vector4 into a 32-bit unsigned integer (for normals and tangents)
  static constexpr uint32_t pack(float4 v) noexcept {
    return std::bit_cast<uint32_t>(
      (static_cast<int32_t>(v.x * 1023) & 0x3FF) | ((static_cast<int32_t>(v.y * 1023) & 0x3FF) << 10) |
      ((static_cast<int32_t>(v.z * 1023) & 0x3FF) << 20) | (int32_t(v.w < 0) << 30));
  }
  static constexpr uint32_t normal_z = pack(float4{0, 0, 1, 0});
  static constexpr uint32_t tangent_x = pack(float4{1, 0, 0, 0});
  float4 position = {0, 0, 0, 1};
  float2 uv = {0, 0};
  uint32_t normal = normal_z;   // encoded normal {x:10, y:10, z:10, w:2 (unused)}
  uint32_t tangent = tangent_x; // encoded tangent {x:10, y:10, z:10, w:2 (bitangent sign)}
};

#define yw_hlsl_vertex \
  struct Vertex {      \
    float4 position;   \
    float2 uv;         \
    uint normal;       \
    uint tangent;      \
  };

#define yw_hlsl_tangent_basis \
  struct TangentBasis {       \
    float3 normal;            \
    float3 tangent;           \
    float3 bitangent;         \
  };

#define yw_hlsl_make_tangent_basis_from_vertex                \
  float _yw_unpack_snorm10(uint bits) {                       \
    int v = bits & 0x3FF;                                     \
    if (v & 0x200) v |= ~0x3FF;                               \
    return float(v) / 1023.0;                                 \
  }                                                           \
  TangentBasis make_tangent_basis_from_vertex(Vertex v) {     \
    TangentBasis tb;                                          \
    tb.normal.x = _yw_unpack_snorm10(v.normal & 0x3FF);       \
    tb.normal.y = _yw_unpack_snorm10(v.normal >> 10 & 0x3FF); \
    tb.normal.z = _yw_unpack_snorm10(v.normal >> 20 & 0x3FF); \
    float3 tangent;                                           \
    tangent.x = _yw_unpack_snorm10(v.tangent);                \
    tangent.y = _yw_unpack_snorm10(v.tangent >> 10);          \
    tangent.z = _yw_unpack_snorm10(v.tangent >> 20);          \
    tangent = tangent - tb.normal * dot(tb.normal, tangent);  \
    tb.tangent = normalize(tangent);                          \
    float sign = ((v.tangent >> 30) & 1) != 0 ? -1.0 : 1.0;   \
    tb.bitangent = cross(tb.normal, tb.tangent) * sign;       \
    return tb;                                                \
  }

/// MARK: geom::geometry_base

template<template<backend> typename Geometry, backend Backend> class geometry_base;

template<template<backend> typename Geometry> class geometry_base<Geometry, cpu> {
protected:
  template<template<backend> typename G, backend B> friend class geometry_base;
  matrix<double, 3, 4> _rigid = {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}};
  double4 _scale = {1, 1, 1, 1};
  bool _dirty = false; // need to update world matrix
  bool _messy = false; // need to update mesh
  geom::remeshing_option<Geometry> _remeshing_option{};

  constexpr void _translation(double x, double y, double z) noexcept {
    _rigid[0][3] = x, _rigid[1][3] = y, _rigid[2][3] = z, _dirty = true;
  }
  constexpr void _translate(double x, double y, double z) noexcept {
    _rigid[0][3] += x, _rigid[1][3] += y, _rigid[2][3] += z, _dirty = true;
  }
  template<typename T> constexpr void _rotation(const T& radians) noexcept {
    const auto c = vapply_r<T>(yw::cos, radians);
    const auto s = vapply_r<T>(yw::sin, radians);
    _rigid[0][0] = c.y * c.z;
    _rigid[0][1] = s.x * s.y * c.z - c.x * s.z;
    _rigid[0][2] = c.x * s.y * c.z + s.x * s.z;
    _rigid[1][0] = c.y * s.z;
    _rigid[1][1] = s.x * s.y * s.z + c.x * c.z;
    _rigid[1][2] = c.x * s.y * s.z - s.x * c.z;
    _rigid[2][0] = -s.y;
    _rigid[2][1] = s.x * c.y;
    _rigid[2][2] = c.x * c.y;
    _dirty = true;
  }
  constexpr void _rotate_x(double c, double s) noexcept {
    const auto r10 = _rigid[1], r20 = _rigid[2];
    for (size_t i = 0; i < 3; ++i) {
      _rigid[1][i] = c * r10[i] - s * r20[i];
      _rigid[2][i] = s * r10[i] + c * r20[i];
    }
    _dirty = true;
  }
  constexpr void _rotate_y(double c, double s) noexcept {
    const auto r00 = _rigid[0], r20 = _rigid[2];
    for (size_t i = 0; i < 3; ++i) {
      _rigid[0][i] = c * r00[i] + s * r20[i];
      _rigid[2][i] = -s * r00[i] + c * r20[i];
    }
    _dirty = true;
  }
  constexpr void _rotate_z(double c, double s) noexcept {
    const auto r00 = _rigid[0], r10 = _rigid[1];
    for (size_t i = 0; i < 3; ++i) {
      _rigid[0][i] = c * r00[i] - s * r10[i];
      _rigid[1][i] = s * r00[i] + c * r10[i];
    }
    _dirty = true;
  }
  constexpr void _rotate(double x, double y, double z) noexcept {
    if (x != 0) _rotate_x(yw::cos(x), yw::sin(x));
    if (y != 0) _rotate_y(yw::cos(y), yw::sin(y));
    if (z != 0) _rotate_z(yw::cos(z), yw::sin(z));
  }
  constexpr void _revolve_x(double c, double s) noexcept {
    const auto y = _rigid[1][3], z = _rigid[2][3];
    _rigid[1][3] = c * y - s * z;
    _rigid[2][3] = s * y + c * z;
    _rotate_x(c, s);
  }
  constexpr void _revolve_y(double c, double s) noexcept {
    const auto x = _rigid[0][3], z = _rigid[2][3];
    _rigid[0][3] = c * x + s * z;
    _rigid[2][3] = -s * x + c * z;
    _rotate_y(c, s);
  }
  constexpr void _revolve_z(double c, double s) noexcept {
    const auto x = _rigid[0][3], y = _rigid[1][3];
    _rigid[0][3] = c * x - s * y;
    _rigid[1][3] = s * x + c * y;
    _rotate_z(c, s);
  }
  constexpr void _revolve(double x, double y, double z) noexcept {
    if (x != 0) _revolve_x(yw::cos(x), yw::sin(x));
    if (y != 0) _revolve_y(yw::cos(y), yw::sin(y));
    if (z != 0) _revolve_z(yw::cos(z), yw::sin(z));
  }
  constexpr void _set_scale(double x, double y, double z) noexcept {
    _scale = {x, y, z, 1};
    _dirty = true;
  }
  constexpr void _rescale(double x, double y, double z) noexcept {
    _scale.x *= x;
    _scale.y *= y;
    _scale.z *= z;
    _dirty = true;
  }

public:
  static constexpr bool is_bounded_surface = false;
  const geom::remeshing_option<Geometry>& remeshing_option() const noexcept { return _remeshing_option; }
  geom::remeshing_option<Geometry>& remeshing_option() noexcept { return _remeshing_option; }
  void remeshing_option(const geom::remeshing_option<Geometry>& option) noexcept { _remeshing_option = option; }
  template<is_base_of<geom::remeshing_option<Geometry>> T> void remeshing_option(const T& option) noexcept {
    static_cast<T&>(_remeshing_option) = option;
  }

  /// gets the translation component.
  constexpr double4 translation() const noexcept { return {_rigid[0][3], _rigid[1][3], _rigid[2][3], 0}; }
  /// sets the translation component.
  constexpr void translation(const double3& t) noexcept { _translation(t.x, t.y, t.z); }
  constexpr void translation(const double4& t) noexcept { _translation(t.x, t.y, t.z); }
  /// adds to the translation component.
  constexpr void translate(const double3& t) noexcept { _translate(t.x, t.y, t.z); }
  constexpr void translate(const double4& t) noexcept { _translate(t.x, t.y, t.z); }

  /// gets the rotation component as XYZ Euler angles.
  constexpr double4 rotation() const noexcept {
    double4 result;
    result.y = yw::asin(yw::clamp(-_rigid[2][0], -1.0, 1.0));
    if (yw::abs(_rigid[2][0]) < 1.0 - 1e-10) {
      result.x = yw::atan2(_rigid[2][1], _rigid[2][2]);
      result.z = yw::atan2(_rigid[1][0], _rigid[0][0]);
    } else result.x = yw::atan2(-_rigid[1][2], _rigid[1][1]);
    return result;
  }
  /// sets the rotation component as XYZ Euler angles.
  constexpr void rotation(const double3& radians) noexcept { _rotation(radians); }
  constexpr void rotation(const double4& radians) noexcept { _rotation(radians); }
  /// adds an XYZ Euler rotation.
  constexpr void rotate(const double3& radians) noexcept { _rotate(radians.x, radians.y, radians.z); }
  constexpr void rotate(const double4& radians) noexcept { _rotate(radians.x, radians.y, radians.z); }
  /// rotates the entire transformation about the origin.
  constexpr void revolve(const double3& radians) noexcept { _revolve(radians.x, radians.y, radians.z); }
  constexpr void revolve(const double4& radians) noexcept { _revolve(radians.x, radians.y, radians.z); }

  /// gets the scale component.
  constexpr double4 scale() const noexcept { return _scale; }
  /// sets the scale component.
  constexpr void scale(const double3& s) noexcept { _set_scale(s.x, s.y, s.z); }
  constexpr void scale(const double4& s) noexcept { _set_scale(s.x, s.y, s.z); }
  /// multiplies the scale component.
  constexpr void rescale(const double3& s) noexcept { _rescale(s.x, s.y, s.z); }
  constexpr void rescale(const double4& s) noexcept { _rescale(s.x, s.y, s.z); }

  /// gets the 3D transformation matrix.
  constexpr matrix<double, 3, 4> transformation3() const noexcept {
    return {
      matrix_row(double4(_rigid[0]) * _scale), matrix_row(double4(_rigid[1]) * _scale),
      matrix_row(double4(_rigid[2]) * _scale)};
  }
  /// gets the 3D transformation matrix.
  constexpr matrix<double, 4, 4> transformation4() const noexcept {
    return {
      matrix_row(double4(_rigid[0]) * _scale), matrix_row(double4(_rigid[1]) * _scale),
      matrix_row(double4(_rigid[2]) * _scale), matrix_row{0, 0, 0, 1}};
  }
  /// copies the transformation from another geometry object.
  template<template<backend B> typename G, backend B>
  constexpr void copy_transformation_from(const geometry_base<G, B>& From) noexcept {
    _rigid = From._rigid, _scale = From._scale, _dirty = From._dirty;
  }
};
} // namespace yw::geom
