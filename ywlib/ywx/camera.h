#pragma once
#include "ywx/buffer.h"
#include "ywx/rendering.h"

namespace yw {

/// \note right-handed coordinate system; X is left, Y is up, looking to Z.
/// \note reverse-z, row-major

class camera : public handle_base {
  static constexpr float4x4 identity = {float4(1, 0, 0, 0), float4(0, 1, 0, 0), float4(0, 0, 1, 0), float4(0, 0, 0, 1)};
  static constexpr float min_perspective_deg = 1.0f;
  static constexpr float max_perspective_deg = 179.0f;

  static bool _finite(float value) noexcept { return value == value && value != inf && value != -inf; }

  static bool _finite(const float4& value) noexcept {
    return _finite(value.x) && _finite(value.y) && _finite(value.z) && _finite(value.w);
  }

public:
  static constexpr float far_by_near = 1024.0f * 1024.0f;

  struct constants {
    float4x4 view;
    float4x4 projection;
    float4x4 view_projection;
    float4x4 inverse_view;
    float4x4 inverse_projection;
    float4x4 inverse_view_projection;
    float4 camera_pos;
    float4 camera_dir;
    float4 viewport_size;
  };

  struct slot : handle_base::slot {
    uint2 size;
    constants cb_value;
    yw::constant_buffer<constants> cb;
    float far_plane{1e6f};
    float factor{1.0f};
    bool orthographic{};
    bool dirty{true};
    float4 offset;   // offset of lens from camera body
    float4 rotation; // degree
    float4 orientation{0.0f, 0.0f, 0.0f, 1.0f};
    float4 position; // position of camera body
  };

private:
  static float4 _euler_deg_from_orientation(float4 Orientation) noexcept {
    const auto rad = yw::euler_from_quaternion(Orientation);
    return float4(rad.x * 180.0f / float(pi), rad.y * 180.0f / float(pi), rad.z * 180.0f / float(pi), 0.0f);
  }

  static float4 _orientation_from_euler_deg(float4 Rotation) noexcept {
    return yw::quaternion_from_euler(
      float4(
        Rotation.x * float(pi) / 180.0f, Rotation.y * float(pi) / 180.0f, Rotation.z * float(pi) / 180.0f, 0.0f));
  }

  static void _load_matrix(const float4x4& f, matrix& m) noexcept {
    m.x = _mm_loadu_ps(f.x.data());
    m.y = _mm_loadu_ps(f.y.data());
    m.z = _mm_loadu_ps(f.z.data());
    m.w = _mm_loadu_ps(f.w.data());
  }

  static void _rotation_matrix(slot* sp, matrix& m) {
    float4x4 f;
    yw::rotation_matrix_from_quaternion(sp->orientation, f);
    _load_matrix(f, m);
  }

  static void _inverse_rotation_matrix(slot* sp, matrix& m) {
    float4x4 f;
    yw::inverse_rotation_matrix_from_quaternion(sp->orientation, f);
    _load_matrix(f, m);
  }

  static void _view_matrix(slot* sp, matrix& m) {
    _inverse_rotation_matrix(sp, m);
    const auto pos = mm_neg(_mm_loadu_ps(sp->position.data()));
    m.x = mm_insert<0, 3>(mm_dot<3>(pos, m.x), m.x);
    m.y = mm_insert<0, 3>(mm_dot<3>(pos, m.y), m.y);
    m.z = mm_insert<0, 3>(mm_dot<3>(pos, m.z), m.z);
    const auto off = _mm_loadu_ps(sp->offset.data());
    m.x = _mm_sub_ps(m.x, mm_insert<0, 3, 0b0111>(off, off));
    m.y = _mm_sub_ps(m.y, mm_insert<1, 3, 0b0111>(off, off));
    m.z = _mm_sub_ps(m.z, mm_insert<2, 3, 0b0111>(off, off));
  }

  static void _inverse_view_matrix(slot* sp, matrix& m) {
    _rotation_matrix(sp, m);
    const auto off = _mm_loadu_ps(sp->offset.data());
    const auto pos = _mm_loadu_ps(sp->position.data());
    m.x = mm_insert<0, 3>(mm_dot<3>(off, m.x), m.x);
    m.y = mm_insert<0, 3>(mm_dot<3>(off, m.y), m.y);
    m.z = mm_insert<0, 3>(mm_dot<3>(off, m.z), m.z);
    m.x = mm_insert<0, 3>(_mm_add_ps(mm_permute<3, 3, 3, 3>(m.x), mm_permute<0, 0, 0, 0>(pos)), m.x);
    m.y = mm_insert<0, 3>(_mm_add_ps(mm_permute<3, 3, 3, 3>(m.y), mm_permute<1, 1, 1, 1>(pos)), m.y);
    m.z = mm_insert<0, 3>(_mm_add_ps(mm_permute<3, 3, 3, 3>(m.z), mm_permute<2, 2, 2, 2>(pos)), m.z);
  }

public:
  explicit operator bool() const noexcept {
    const auto sp = get_slot(this);
    return sp && sp->size.x != 0 && sp->size.y != 0 && static_cast<bool>(sp->cb);
  }

  camera() noexcept = default;
  camera(camera&&) noexcept = default;
  camera& operator=(camera&&) noexcept = default;

  /// creates a camera.
  static std::expected<camera, error> create(int2 Size) {
    if (Size.x <= 0 || Size.y <= 0)
      return std::unexpected(error(errors::invalid_argument, "camera size must be positive"));
    const auto sp = make_slot<camera>();
    if (!sp) return std::unexpected(error(errors::slot_creation_failed));
    sp->size = Size;
    if (auto res = yw::constant_buffer<constants>::create(sp->cb_value)) sp->cb = std::move(*res);
    else return res.error().relay();
    return make_handle<camera>(sp->id);
  }

  /// creates a camera.
  explicit camera(int2 Size, const source_line& sl = here()) {
    if (auto res = create(Size); !res) res.error().go_off(sl);
    else *this = std::move(*res);
  }

  uint2 size() const noexcept {
    if (const auto sp = get_slot(this)) return sp->size;
    else return {};
  }

  bool dirty() const noexcept {
    if (const auto sp = get_slot(this)) return sp->dirty;
    else return false;
  }

  std::expected<void, error> resize(int2 Size) {
    const auto sp = get_slot(this);
    if (!sp) return std::unexpected(error(errors::not_initialized, "camera not initialized"));
    if (Size.x <= 0 || Size.y <= 0)
      return std::unexpected(error(errors::invalid_argument, "camera size must be positive"));
    sp->size = Size;
    sp->dirty = true;
    return {};
  }

  std::expected<void, error> perspective(float1 fov_deg) {
    const auto sp = get_slot(this);
    if (!sp) return std::unexpected(error(errors::not_initialized, "camera not initialized"));
    if (!_finite(fov_deg.x)) return std::unexpected(error(errors::invalid_argument, "perspective fov must be finite"));
    if (fov_deg.x < min_perspective_deg || fov_deg.x > max_perspective_deg)
      return std::unexpected(error(errors::invalid_argument, "perspective fov must be in [1, 179] degrees"));
    sp->factor = fov_deg.x * float(pi) / 180.0f;
    sp->orthographic = false;
    sp->dirty = true;
    return {};
  }

  /// returns the field of view in degrees if the camera is perspective, otherwise returns 0.0f.
  float fov() const noexcept {
    if (const auto sp = get_slot(this); sp && !sp->orthographic) return 0.5f * sp->factor * 180.0f / float(pi);
    else return 0.0f;
  }

  std::expected<void, error> orthographic(float1 magnification) {
    const auto sp = get_slot(this);
    if (!sp) return std::unexpected(error(errors::not_initialized, "camera not initialized"));
    if (!_finite(magnification.x))
      return std::unexpected(error(errors::invalid_argument, "orthographic magnification must be finite"));
    if (magnification.x <= 0.0f)
      return std::unexpected(error(errors::invalid_argument, "orthographic magnification must be greater than zero"));
    sp->factor = magnification.x;
    sp->orthographic = true;
    sp->dirty = true;
    return {};
  }

  /// returns the magnification if the camera is orthographic, otherwise returns 0.0f.
  float magnification() const noexcept {
    if (const auto sp = get_slot(this); sp && sp->orthographic) return sp->factor;
    else return 0.0f;
  }

  float far_() const noexcept {
    if (const auto sp = get_slot(this)) return sp->far_plane;
    else return 0.0f;
  }

  std::expected<void, error> far_(float1 Far) {
    const auto sp = get_slot(this);
    if (!sp) return std::unexpected(error(errors::not_initialized, "camera not initialized"));
    if (!_finite(Far.x)) return std::unexpected(error(errors::invalid_argument, "far plane must be finite"));
    if (Far.x <= 0.0f) return std::unexpected(error(errors::invalid_argument, "far plane must be greater than zero"));
    sp->far_plane = Far.x;
    sp->dirty = true;
    return {};
  }

  float4 position() const noexcept {
    if (const auto sp = get_slot(this)) return sp->position;
    else return {};
  }

  std::expected<void, error> position(float4 Position) {
    const auto sp = get_slot(this);
    if (!sp) return std::unexpected(error(errors::not_initialized, "camera not initialized"));
    if (!_finite(Position)) return std::unexpected(error(errors::invalid_argument, "position must be finite"));
    sp->position = Position;
    sp->dirty = true;
    return {};
  }

  float4 rotation() const noexcept {
    if (const auto sp = get_slot(this)) return _euler_deg_from_orientation(sp->orientation);
    else return {};
  }

  std::expected<void, error> rotation(float4 Rotation) {
    const auto sp = get_slot(this);
    if (!sp) return std::unexpected(error(errors::not_initialized, "camera not initialized"));
    if (!_finite(Rotation)) return std::unexpected(error(errors::invalid_argument, "rotation must be finite"));
    sp->rotation = Rotation;
    sp->orientation = _orientation_from_euler_deg(Rotation);
    sp->dirty = true;
    return {};
  }

  float4 offset() const noexcept {
    if (const auto sp = get_slot(this)) return sp->offset;
    else return {};
  }

  std::expected<void, error> offset(float4 Offset) {
    const auto sp = get_slot(this);
    if (!sp) return std::unexpected(error(errors::not_initialized, "camera not initialized"));
    if (!_finite(Offset)) return std::unexpected(error(errors::invalid_argument, "offset must be finite"));
    sp->offset = Offset;
    sp->dirty = true;
    return {};
  }

  std::expected<void, error> set_pose(float4 Position, float4 Rotation, float4 Offset = {}) {
    const auto sp = get_slot(this);
    if (!sp) return std::unexpected(error(errors::not_initialized, "camera not initialized"));
    if (!_finite(Position)) return std::unexpected(error(errors::invalid_argument, "position must be finite"));
    if (!_finite(Rotation)) return std::unexpected(error(errors::invalid_argument, "rotation must be finite"));
    if (!_finite(Offset)) return std::unexpected(error(errors::invalid_argument, "offset must be finite"));
    sp->position = Position;
    sp->rotation = Rotation;
    sp->orientation = _orientation_from_euler_deg(Rotation);
    sp->offset = Offset;
    sp->dirty = true;
    return {};
  }

  std::expected<void, error> rotate_axis(float3 Axis, float1 AngleDeg) {
    const auto sp = get_slot(this);
    if (!sp) return std::unexpected(error(errors::not_initialized, "camera not initialized"));
    if (!_finite(float4(Axis.x, Axis.y, Axis.z, AngleDeg.x)))
      return std::unexpected(error(errors::invalid_argument, "axis rotation must be finite"));
    const auto delta = yw::quaternion_from_axis_angle(Axis, AngleDeg.x * float(pi) / 180.0f);
    sp->orientation = yw::quaternion_multiply(delta, sp->orientation);
    sp->rotation = _euler_deg_from_orientation(sp->orientation);
    sp->dirty = true;
    return {};
  }

  std::expected<void, error> rotate_local_axis(float3 Axis, float1 AngleDeg) {
    const auto sp = get_slot(this);
    if (!sp) return std::unexpected(error(errors::not_initialized, "camera not initialized"));
    if (!_finite(float4(Axis.x, Axis.y, Axis.z, AngleDeg.x)))
      return std::unexpected(error(errors::invalid_argument, "axis rotation must be finite"));
    const auto delta = yw::quaternion_from_axis_angle(Axis, AngleDeg.x * float(pi) / 180.0f);
    sp->orientation = yw::quaternion_multiply(sp->orientation, delta);
    sp->rotation = _euler_deg_from_orientation(sp->orientation);
    sp->dirty = true;
    return {};
  }

  /// sets `rotation.x` (pitch) and `rotation.y` (yaw) so that camera faces `Target`.
  /// \note this function ignores `offset` and uses only `position`.
  std::expected<void, error> look_at(float4 Target) {
    const auto sp = get_slot(this);
    if (!sp) return std::unexpected(error(errors::not_initialized, "camera not initialized"));
    if (!_finite(Target)) return std::unexpected(error(errors::invalid_argument, "target must be finite"));
    constexpr float eps = 1e-6f;
    const float dx = Target.x - sp->position.x;
    const float dy = Target.y - sp->position.y;
    const float dz = Target.z - sp->position.z;
    const bool x_zero = yw::abs(dx) <= eps;
    const bool y_zero = yw::abs(dy) <= eps;
    const bool z_zero = yw::abs(dz) <= eps;

    if (x_zero && y_zero && z_zero) return {};

    if (x_zero && z_zero) {
      sp->rotation.x = dy > 0.0f ? -90.0f : 90.0f;
      sp->rotation.y = 0.0f;
      sp->rotation.z = 0.0f;
      sp->rotation.w = 0.0f;
      sp->orientation = _orientation_from_euler_deg(sp->rotation);
      sp->dirty = true;
      return {};
    }

    const float yaw = yw::atan2(dx, dz);
    const float horizontal = yw::sqrt(dx * dx + dz * dz);
    const float pitch = -yw::atan2(dy, horizontal);
    sp->rotation.x = pitch * 180.0f / float(pi);
    sp->rotation.y = yaw * 180.0f / float(pi);
    sp->rotation.z = 0.0f;
    sp->rotation.w = 0.0f;
    sp->orientation = _orientation_from_euler_deg(sp->rotation);
    sp->dirty = true;
    return {};
  }

  const auto& view_matrix() const noexcept {
    if (const auto sp = get_slot(this)) return sp->cb_value.view;
    else return identity;
  }

  const auto& projection_matrix() const noexcept {
    if (const auto sp = get_slot(this)) return sp->cb_value.projection;
    else return identity;
  }

  const auto& view_projection_matrix() const noexcept {
    if (const auto sp = get_slot(this)) return sp->cb_value.view_projection;
    else return identity;
  }

  const auto& inverse_view_matrix() const noexcept {
    if (const auto sp = get_slot(this)) return sp->cb_value.inverse_view;
    else return identity;
  }

  const auto& inverse_projection_matrix() const noexcept {
    if (const auto sp = get_slot(this)) return sp->cb_value.inverse_projection;
    else return identity;
  }

  const auto& inverse_view_projection_matrix() const noexcept {
    if (const auto sp = get_slot(this)) return sp->cb_value.inverse_view_projection;
    else return identity;
  }

  float4 camera_direction() const noexcept {
    if (const auto sp = get_slot(this)) return sp->cb_value.camera_dir;
    else return float4(0, 0, 1, 0);
  }

  float4 right_direction() const noexcept {
    if (const auto sp = get_slot(this)) {
      float4x4 m;
      yw::rotation_matrix_from_quaternion(sp->orientation, m);
      return float4(-m.x.x, -m.y.x, -m.z.x, 0.0f);
    } else return float4(-1, 0, 0, 0);
  }

  float4 up_direction() const noexcept {
    if (const auto sp = get_slot(this)) {
      float4x4 m;
      yw::rotation_matrix_from_quaternion(sp->orientation, m);
      return float4(m.x.y, m.y.y, m.z.y, 0.0f);
    } else return float4(0, 1, 0, 0);
  }

  float4 forward_direction() const noexcept {
    if (const auto sp = get_slot(this)) {
      float4x4 m;
      yw::rotation_matrix_from_quaternion(sp->orientation, m);
      return float4(m.x.z, m.y.z, m.z.z, 0.0f);
    } else return float4(0, 0, 1, 0);
  }

  const auto& constant_buffer() const noexcept {
    static yw::constant_buffer<constants> empty;
    if (const auto sp = get_slot(this)) return sp->cb;
    else return empty;
  }

  std::expected<void, error> update() {
    const auto sp = get_slot(this);
    if (!sp || !sp->cb) return std::unexpected(error(errors::not_initialized, "camera not initialized"));
    if (!sp->dirty) return {};
    if (!_finite(sp->far_plane)) return std::unexpected(error(errors::invalid_argument, "far plane must be finite"));
    if (sp->far_plane <= 0.0f)
      return std::unexpected(error(errors::invalid_argument, "far plane must be greater than zero"));
    if (!_finite(sp->position)) return std::unexpected(error(errors::invalid_argument, "position must be finite"));
    if (!_finite(sp->rotation)) return std::unexpected(error(errors::invalid_argument, "rotation must be finite"));
    if (!_finite(sp->orientation)) return std::unexpected(error(errors::invalid_argument, "orientation must be finite"));
    if (!_finite(sp->offset)) return std::unexpected(error(errors::invalid_argument, "offset must be finite"));
    if (sp->orthographic) {
      if (!_finite(sp->factor))
        return std::unexpected(error(errors::invalid_argument, "orthographic magnification must be finite"));
      if (sp->factor <= 0.0f)
        return std::unexpected(error(errors::invalid_argument, "orthographic magnification must be greater than zero"));
    } else {
      const auto fov_deg = sp->factor * 180.0f / float(pi);
      if (!_finite(fov_deg)) return std::unexpected(error(errors::invalid_argument, "perspective fov must be finite"));
      if (fov_deg < min_perspective_deg || fov_deg > max_perspective_deg)
        return std::unexpected(error(errors::invalid_argument, "perspective fov must be in [1, 179] degrees"));
    }

    auto& vm = sp->cb_value.view;
    auto& pm = sp->cb_value.projection;
    auto& vpm = sp->cb_value.view_projection;
    auto& ivm = sp->cb_value.inverse_view;
    auto& ipm = sp->cb_value.inverse_projection;
    auto& ivpm = sp->cb_value.inverse_view_projection;
    matrix m;
    _view_matrix(sp, m);
    m.store(vm);
    _inverse_view_matrix(sp, m);
    m.store(ivm);
    if (sp->orthographic) {
      const float sx = 2.0f * sp->factor / sp->size.x;
      const float sy = 2.0f * sp->factor / sp->size.y;
      const float sz = 1.0f / sp->far_plane;
      pm.x = float4(sx, 0, 0, 0);
      pm.y = float4(0, sy, 0, 0);
      pm.z = float4(0, 0, sz, 0);
      pm.w = float4(0, 0, 0, 1);
      ipm.x = float4(1.0f / sx, 0, 0, 0);
      ipm.y = float4(0, 1.0f / sy, 0, 0);
      ipm.z = float4(0, 0, 1.0f / sz, 0);
      ipm.w = float4(0, 0, 0, 1);
    } else {
      const float f = 1.0f / yw::tan(sp->factor * 0.5f);
      const float a = f * sp->size.y / sp->size.x;
      constexpr float b = -1.0f / (far_by_near - 1.0f);
      const float c = -sp->far_plane * b;
      pm.x = float4(a, 0, 0, 0);
      pm.y = float4(0, f, 0, 0);
      pm.z = float4(0, 0, b, c);
      pm.w = float4(0, 0, 1, 0);
      ipm.x = float4(-1.0f / a, 0, 0, 0);
      ipm.y = float4(0, 1.0f / f, 0, 0);
      ipm.z = float4(0, 0, 0, 1);
      ipm.w = float4(0, 0, 1.0f / c, -b / c);
    }
    // matrix * vector convention: clip = projection * view * world.
    matrix_transform(pm, vm, vpm);
    matrix_transform(ivm, ipm, ivpm);
    sp->cb_value.camera_pos = sp->position;
    sp->cb_value.camera_dir = float4(ivm.x.z, ivm.y.z, ivm.z.z, 0);
    sp->cb_value.viewport_size = float4(float(sp->size.x), float(sp->size.y), 1.0f / sp->size.x, 1.0f / sp->size.y);
    if (auto res = sp->cb.copy_from(sp->cb_value); !res) return res.error().relay();
    sp->dirty = false;
    return {};
  }

  template<typename... Ts> std::expected<rendering, error> begin_render(const Ts&... rtvs_dsv_uavs) {
    const auto sp = get_slot(this);
    if (!sp || !sp->cb) return std::unexpected(error(errors::not_initialized, "camera not initialized"));
    if (auto res = update(); !res) return res.error().relay();
    D3D11_VIEWPORT vp{0.0f, 0.0f, float(sp->size.x), float(sp->size.y), 0.0f, 1.0f};
    d3d::context()->RSSetViewports(1, &vp);
    d3d::context()->OMSetDepthStencilState(d3d::dss_reverse_z(), 0);
    if (auto res = rendering::create(rtvs_dsv_uavs...); !res) return res.error().relay();
    else return std::move(*res);
  }
};
} // namespace yw
