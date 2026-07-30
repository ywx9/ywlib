#pragma once
#include "ywx/bitmap.h"
#include "ywx/buffer.h"
#include "ywx/rendering.h"

namespace yw {

/// \note right-handed coordinate system; X is left, Y is up, looking to Z.
/// \note reverse-z, row-major

class viewport {
  static constexpr float4x4 identity = {float4(1, 0, 0, 0), float4(0, 1, 0, 0), float4(0, 0, 1, 0), float4(0, 0, 0, 1)};
  static constexpr float min_perspective_deg = 1.0f;
  static constexpr float max_perspective_deg = 179.0f;
  inline static comptr<ID3D11DepthStencilState> _dss; /// for Reverse-Z

  static std::expected<void, error> _init_dss() {
    if (_dss) return {};
    D3D11_DEPTH_STENCIL_DESC depth_stencil_desc{};
    depth_stencil_desc.DepthEnable = TRUE;
    depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depth_stencil_desc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
    hresult_test(d3d::device()->CreateDepthStencilState, &depth_stencil_desc, &_dss.get());
    return {};
  }

  static bool _finite(float value) noexcept { return value == value && value != inf && value != -inf; }

  static bool _finite(const float4& value) noexcept {
    return _finite(value.x) && _finite(value.y) && _finite(value.z) && _finite(value.w);
  }

  static std::expected<void, error> _validate_size(uint2 Size) {
    if (Size.x == 0 || Size.y == 0)
      return std::unexpected(error(errors::invalid_argument, "viewport size must be non-zero"));
    return {};
  }

  static std::expected<void, error> _validate_perspective(float fov_deg) {
    if (!_finite(fov_deg)) return std::unexpected(error(errors::invalid_argument, "perspective fov must be finite"));
    if (fov_deg < min_perspective_deg || fov_deg > max_perspective_deg)
      return std::unexpected(error(errors::invalid_argument, "perspective fov must be in [1, 179] degrees"));
    return {};
  }

  static std::expected<void, error> _validate_orthographic(float magnification) {
    if (!_finite(magnification))
      return std::unexpected(error(errors::invalid_argument, "orthographic magnification must be finite"));
    if (magnification <= 0.0f)
      return std::unexpected(error(errors::invalid_argument, "orthographic magnification must be greater than zero"));
    return {};
  }

  static std::expected<void, error> _validate_far(float Far) {
    if (!_finite(Far)) return std::unexpected(error(errors::invalid_argument, "far plane must be finite"));
    if (Far <= 0.0f) return std::unexpected(error(errors::invalid_argument, "far plane must be greater than zero"));
    return {};
  }

  static std::expected<void, error> _validate_vector(const float4& value, const char* name) {
    if (!_finite(value))
      return std::unexpected(error(errors::invalid_argument, std::format("{} must be finite", name)));
    return {};
  }

  std::expected<void, error> _recreate_targets(uint2 Size) {
    if (auto res = _validate_size(Size); !res) return res.error().relay();
    if (auto res = bitmap::create(Size)) _bitmap = std::move(*res);
    else return res.error().relay();
    comptr<IDXGISurface> surface;
    hresult_test(_bitmap.d2d_bitmap()->GetSurface, &surface.get());
    hresult_test(surface->QueryInterface, &_texture.get());
    {
      D3D11_TEXTURE2D_DESC tex_desc{};
      _texture->GetDesc(&tex_desc);
      yw::print("Width: ", tex_desc.Width);
      yw::print("Height: ", tex_desc.Height);
      yw::print("MipLevels: ", tex_desc.MipLevels);
      yw::print("ArraySize: ", tex_desc.ArraySize);
      yw::print("Format: ", static_cast<int>(tex_desc.Format));
      yw::print("SampleDesc.Count: ", tex_desc.SampleDesc.Count);
      yw::print("SampleDesc.Quality: ", tex_desc.SampleDesc.Quality);
      yw::print("Usage: ", static_cast<int>(tex_desc.Usage));
      yw::print("BindFlags: ", static_cast<int>(tex_desc.BindFlags));
      yw::print("CPUAccessFlags: ", static_cast<int>(tex_desc.CPUAccessFlags));
      yw::print("MiscFlags: ", static_cast<int>(tex_desc.MiscFlags));
    }
    D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc{bitmap::dxgiformat, D3D11_SRV_DIMENSION_TEXTURE2D};
    srv_desc.Texture2D.MipLevels = 1;
    hresult_test(d3d::device()->CreateShaderResourceView, _texture.get(), &srv_desc, &_srv.get());
    D3D11_TEXTURE2D_DESC tex_desc{
      Size.x, Size.y, 1, 1, DXGI_FORMAT_D32_FLOAT, DXGI_SAMPLE_DESC(1, 0), {}, D3D11_BIND_DEPTH_STENCIL};
    hresult_test(d3d::device()->CreateTexture2D, &tex_desc, nullptr, &_depth.get());
    hresult_test(d3d::device()->CreateDepthStencilView, _depth.get(), nullptr, &_dsv.get());
    D3D11_RENDER_TARGET_VIEW_DESC rtv_desc{bitmap::dxgiformat, D3D11_RTV_DIMENSION_TEXTURE2D};
    hresult_test(d3d::device()->CreateRenderTargetView, _texture.get(), &rtv_desc, &_rtv.get());
    return {};
  }

  std::expected<void, error> _validate_state() const {
    if (!*this) return std::unexpected(error(errors::not_initialized, "Viewport not initialized"));
    if (auto res = _validate_size(_bitmap.size()); !res) return res.error().relay();
    if (auto res = _validate_far(_far); !res) return res.error().relay();
    if (auto res = _validate_vector(position, "position"); !res) return res.error().relay();
    if (auto res = _validate_vector(rotation, "rotation"); !res) return res.error().relay();
    if (auto res = _validate_vector(offset, "offset"); !res) return res.error().relay();
    if (_orthographic) {
      if (auto res = _validate_orthographic(_factor); !res) return res.error().relay();
    } else {
      if (auto res = _validate_perspective(_factor * 180.0f / float(pi)); !res)
        return res.error().relay();
    }
    return {};
  }

  std::expected<void, error> _prepare_render(const color* clear_color) {
    if (!*this) return std::unexpected(error(errors::not_initialized, "Viewport not initialized"));
    if (clear_color) {
      d3d::context()->ClearDepthStencilView(_dsv.get(), D3D11_CLEAR_DEPTH, 0.0f, 0);
      d3d::context()->ClearRenderTargetView(_rtv.get(), &clear_color->r);
    }
    D3D11_VIEWPORT vp{0.0f, 0.0f, float(_bitmap.size().x), float(_bitmap.size().y), 0.0f, 1.0f};
    d3d::context()->RSSetViewports(1, &vp);
    d3d::context()->OMSetDepthStencilState(_dss.get(), 0);
    return {};
  }

  void _view_matrix(matrix& m) {
    const auto radians = vapply_r<float4>([](float deg) { return deg * float(pi) / 180.0f; }, rotation);
    const auto cos = vapply_r<float4>(yw::cos, radians);
    const auto sin = vapply_r<float4>(yw::sin, radians);
    yw::inverse_rotation_matrix(_mm_loadu_ps(cos.data()), _mm_loadu_ps(sin.data()), m);
    const auto pos = mm_neg(_mm_loadu_ps(position.data()));
    m.x = mm_insert<0, 3>(mm_dot<3>(pos, m.x), m.x);
    m.y = mm_insert<0, 3>(mm_dot<3>(pos, m.y), m.y);
    m.z = mm_insert<0, 3>(mm_dot<3>(pos, m.z), m.z);
    const auto off = _mm_loadu_ps(offset.data());
    m.x = _mm_sub_ps(m.x, mm_insert<0, 3, 0b0111>(off, off));
    m.y = _mm_sub_ps(m.y, mm_insert<1, 3, 0b0111>(off, off));
    m.z = _mm_sub_ps(m.z, mm_insert<2, 3, 0b0111>(off, off));
  }

public:
  static constexpr float far_by_near = 1024.0f * 1024.0f;

  struct cb {
    float4x4 view = identity;
    float4x4 projection = identity;
    float4x4 view_projection = identity;
    float4 camera_pos = {};
  };

private:
  cb _cb_value{};
  bitmap _bitmap{};
  comptr<ID3D11Texture2D> _texture{};
  comptr<ID3D11Texture2D> _depth{};
  comptr<ID3D11ShaderResourceView> _srv{};
  comptr<ID3D11RenderTargetView> _rtv{};
  comptr<ID3D11DepthStencilView> _dsv{};
  yw::constant_buffer<cb> _cb{};

  float _far{1e6f};
  float _factor{1.0f};
  bool _orthographic{};

public:
  float4 offset{};   // offset of lens from camera body
  float4 rotation{}; // degree
  float4 position{}; // position of camera body

  explicit operator bool() const noexcept { return static_cast<bool>(_bitmap); }

  viewport() noexcept = default;

  static std::expected<viewport, error> create(uint2 Size) {
    if (auto res = _init_dss(); !res) return res.error().relay();
    viewport vp;
    if (auto res = vp._recreate_targets(Size); !res) return res.error().relay();
    if (auto res = yw::constant_buffer<cb>::create(vp._cb_value)) vp._cb = std::move(*res);
    else return res.error().relay();
    return vp;
  }

  std::expected<void, error> resize(uint2 Size) {
    if (auto res = _init_dss(); !res) return res.error().relay();
    if (auto res = _recreate_targets(Size); !res) return res.error().relay();
    return {};
  }

  std::expected<void, error> perspective(float1 fov_deg) {
    if (auto res = _validate_perspective(fov_deg.x); !res) return res.error().relay();
    _factor = fov_deg.x * float(pi) / 180.0f;
    _orthographic = false;
    return {};
  }

  std::expected<void, error> orthographic(float1 magnification) {
    if (auto res = _validate_orthographic(magnification.x); !res) return res.error().relay();
    _factor = magnification.x;
    _orthographic = true;
    return {};
  }

  float far_() const noexcept { return _far; }
  std::expected<void, error> far_(float1 Far) {
    if (auto res = _validate_far(Far.x); !res) return res.error().relay();
    _far = Far.x;
    return {};
  }

  std::expected<void, error> set_pose(float4 Position, float4 Rotation, float4 Offset = {}) {
    if (auto res = _validate_vector(Position, "position"); !res) return res.error().relay();
    if (auto res = _validate_vector(Rotation, "rotation"); !res) return res.error().relay();
    if (auto res = _validate_vector(Offset, "offset"); !res) return res.error().relay();
    position = Position;
    rotation = Rotation;
    offset = Offset;
    return {};
  }

  /// sets `rotation.x` (pitch) and `rotation.y` (yaw) so that camera faces `Target`.
  /// this function ignores `offset` and uses only `position`.
  std::expected<void, error> look_at(float4 Target) {
    if (auto res = _validate_vector(Target, "target"); !res) return res.error().relay();
    constexpr float eps = 1e-6f;
    const float dx = Target.x - position.x;
    const float dy = Target.y - position.y;
    const float dz = Target.z - position.z;
    const bool x_zero = yw::abs(dx) <= eps;
    const bool y_zero = yw::abs(dy) <= eps;
    const bool z_zero = yw::abs(dz) <= eps;

    // target == position: keep current rotation.
    if (x_zero && y_zero && z_zero) return {};

    // straight up/down: update pitch only and keep current yaw.
    if (x_zero && z_zero) {
      rotation.x = dy > 0.0f ? -90.0f : 90.0f;
      rotation.z = 0.0f;
      rotation.w = 0.0f;
      return {};
    }

    const float yaw = yw::atan2(dx, dz);
    const float horizontal = yw::sqrt(dx * dx + dz * dz);
    const float pitch = -yw::atan2(dy, horizontal);
    rotation.x = pitch * 180.0f / float(pi);
    rotation.y = yaw * 180.0f / float(pi);
    rotation.z = 0.0f;
    rotation.w = 0.0f;
    return {};
  }

  const auto& view_matrix() const noexcept { return _cb_value.view; }
  const auto& projection_matrix() const noexcept { return _cb_value.projection; }
  const auto& view_projection_matrix() const noexcept { return _cb_value.view_projection; }
  const auto& bitmap() const noexcept { return _bitmap; }
  const auto& constant_buffer() const noexcept { return _cb; }

  std::expected<void, error> update() {
    if (auto res = _validate_state(); !res) return res.error().relay();
    auto& vm = _cb_value.view;
    auto& pm = _cb_value.projection;
    auto& vpm = _cb_value.view_projection;
    matrix m;
    _view_matrix(m);
    _mm_storeu_ps(vm.x.data(), m.x);
    _mm_storeu_ps(vm.y.data(), m.y);
    _mm_storeu_ps(vm.z.data(), m.z);
    _mm_storeu_ps(vm.w.data(), m.w);
    if (_orthographic) {
      const auto b = 1.0f / _far;
      pm.x.x = -2.0f * _factor / _bitmap.size().x;
      pm.y.y = 2.0f * _factor / _bitmap.size().y;
      pm.z = float4(0, 0, b, 0);
      pm.w = float4(0, 0, 0, 1);
      _mm_storeu_ps(vpm.z.data(), _mm_mul_ps(m.z, mm_set1(b)));
    } else {
      const auto f = 1.0f / yw::tan(_factor * 0.5f);
      const auto a = f * _bitmap.size().y / _bitmap.size().x;
      constexpr float b = -1.0f / (far_by_near - 1.0f);
      const float c = -_far * b;
      pm.x.x = -a;
      pm.y.y = f;
      pm.z = float4(0, 0, b, c);
      pm.w = float4(0, 0, 1, 0);
      _mm_storeu_ps(vpm.z.data(), _mm_add_ps(_mm_mul_ps(m.z, mm_set1(b)), mm_set<3>(c)));
      _mm_storeu_ps(vpm.w.data(), _mm_mul_ps(m.z, mm_set1(pm.w.z)));
    }
    _mm_storeu_ps(vpm.x.data(), _mm_mul_ps(m.x, mm_set1(pm.x.x)));
    _mm_storeu_ps(vpm.y.data(), _mm_mul_ps(m.y, mm_set1(pm.y.y)));
    _cb_value.camera_pos = position;
    if (auto res = _cb.copy_from(_cb_value); !res) return res.error().relay();
    return {};
  }

  std::expected<void, error> clear_depth() {
    d3d::context()->ClearDepthStencilView(_dsv.get(), D3D11_CLEAR_DEPTH, 0.0f, 0);
    return {};
  }

  std::expected<void, error> clear(const color& Clear = colors::transparent) {
    d3d::context()->ClearDepthStencilView(_dsv.get(), D3D11_CLEAR_DEPTH, 0.0f, 0);
    d3d::context()->ClearRenderTargetView(_rtv.get(), &Clear.r);
    return {};
  }

  std::expected<rendering, error> begin_render() {
    if (auto res = _prepare_render(nullptr); !res) return res.error().relay();
    if (auto res = rendering::create(_rtv.get(), _dsv.get())) return std::move(*res);
    else return res.error().relay();
  }

  std::expected<rendering, error> begin_render(const color& Clear) {
    if (auto res = _prepare_render(&Clear); !res) return res.error().relay();
    if (auto res = rendering::create(_rtv.get(), _dsv.get())) return std::move(*res);
    else return res.error().relay();
  }
};
} // namespace yw
