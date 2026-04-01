#pragma once
#include "ywx/bitmap.h"
#include "ywx/buffer.h"
#include "ywx/drawing.h"

namespace yw {

/// \note right-handed coordinate system; X is right, Y is up, Z is outward
/// \note reverse-z

class viewport {
  static constexpr float4x4 identity = {float4(1, 0, 0, 0), float4(0, 1, 0, 0), float4(0, 0, 1, 0), float4(0, 0, 0, 1)};
  inline static comptr<ID3D11DepthStencilState> _dss; /// for Reverse-Z

  static std::expected<void, error_trace> _init_dss() {
    if (_dss) return {};
    if (auto res = d3d.initialize(); !res) return unexpected_error(res.error());
    D3D11_DEPTH_STENCIL_DESC depth_stencil_desc{};
    depth_stencil_desc.DepthEnable = TRUE;
    depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depth_stencil_desc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
    if (auto hr = d3d.device()->CreateDepthStencilState(&depth_stencil_desc, &_dss.get()); FAILED(hr))
      return unexpected_error(errors::operation_failed, "CreateDepthStencilState failed", int32_t(hr));
    return {};
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
  constant_buffer<cb> _cb{};

  float _far{1e6};
  float _factor{float(pi) / 3.0f};
  bool _orthographic{};

public:
  float4 offset{};   // offset of lens from camera body
  float4 rotation{}; // degree
  float4 position{}; // position of camera body

  explicit operator bool() const noexcept { return static_cast<bool>(_bitmap); }

  viewport() noexcept = default;

  static std::expected<viewport, error_trace> create(uint2 Size) {
    if (auto res = _init_dss(); !res) return unexpected_error(res.error());
    viewport vp;
    if (auto res = bitmap::create(Size)) vp._bitmap = std::move(*res);
    else return unexpected_error(res.error());
    comptr<IDXGISurface> surface;
    if (auto hr = ((ID2D1Bitmap1*)vp._bitmap)->GetSurface(&surface.get()); FAILED(hr))
      return unexpected_error(errors::operation_failed, "Failed to get surface", int32_t(hr));
    if (auto hr = surface->QueryInterface(&vp._texture.get()); FAILED(hr))
      return unexpected_error(errors::operation_failed, "Failed to query interface", int32_t(hr));
    D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc{bitmap::dxgiformat, D3D11_SRV_DIMENSION_TEXTURE2D};
    srv_desc.Texture2D.MipLevels = 1;
    if (auto hr = d3d.device()->CreateShaderResourceView(vp._texture.get(), &srv_desc, &vp._srv.get()); FAILED(hr))
      return unexpected_error(errors::operation_failed, "Failed to create shader resource view", int32_t(hr));
    D3D11_TEXTURE2D_DESC tex_desc{
      Size.x, Size.y, 1, 1, DXGI_FORMAT_D32_FLOAT, DXGI_SAMPLE_DESC(1, 0), {}, D3D11_BIND_DEPTH_STENCIL};
    if (auto hr = d3d.device()->CreateTexture2D(&tex_desc, nullptr, &vp._depth.get()); FAILED(hr))
      return unexpected_error(errors::operation_failed, "Failed to create depth texture", int32_t(hr));
    if (auto hr = d3d.device()->CreateDepthStencilView(vp._depth.get(), nullptr, &vp._dsv.get()); FAILED(hr))
      return unexpected_error(errors::operation_failed, "Failed to create depth stencil view", int32_t(hr));
    D3D11_RENDER_TARGET_VIEW_DESC rtv_desc{bitmap::dxgiformat, D3D11_RTV_DIMENSION_TEXTURE3D};
    if (auto hr = d3d.device()->CreateRenderTargetView(vp._texture.get(), &rtv_desc, &vp._rtv.get()); FAILED(hr))
      return unexpected_error(errors::operation_failed, "Failed to create render target view", int32_t(hr));
    if (auto res = constant_buffer<cb>::create(vp._cb_value)) vp._cb = std::move(*res);
    else return unexpected_error(res.error());
    return vp;
  }

  void perspective(float1 fov_deg) noexcept {
    _factor = 1.0f / std::tan(fov_deg.x * float(pi) / 360.0f);
    _orthographic = false;
  }

  void perspective(float1 Far) noexcept {
    _far = Far.x;
    _orthographic = false;
  }

  void perspective(float1 fov_deg, float1 Far) noexcept {
    _far = Far.x;
    _factor = 1.0f / std::tan(fov_deg.x * float(pi) / 360.0f);
    _orthographic = false;
  }

  void orthographic(float1 magnification) noexcept {
    _factor = magnification.x;
    _orthographic = true;
  }

  const auto& view_matrix() const noexcept { return _cb_value.view; }
  const auto& projection_matrix() const noexcept { return _cb_value.projection; }
  const auto& view_projection_matrix() const noexcept { return _cb_value.view_projection; }
  const auto& image() const noexcept { return _bitmap; }

  std::expected<void, error_trace> update() {
    const auto radians = vapply_r<float4>([](float deg) { return deg * float(pi) / 180.0f; }, rotation);
    const auto cos = vapply_r<float4>(yw::cos, radians);
    const auto sin = vapply_r<float4>(yw::sin, radians);
    auto& vm = _cb_value.view;
    auto& pm = _cb_value.projection;
    auto& vpm = _cb_value.view_projection;
    matrix m;
    yw::view_matrix(
      _mm_loadu_ps(position.data()), _mm_loadu_ps(cos.data()), _mm_loadu_ps(sin.data()), _mm_loadu_ps(offset.data()),
      m);
    _mm_storeu_ps(vm.x.data(), m.x);
    _mm_storeu_ps(vm.y.data(), m.y);
    _mm_storeu_ps(vm.z.data(), m.z);
    _mm_storeu_ps(vm.w.data(), m.w);
    if (_orthographic) {
      const auto b = 1.0f / (_far - _far / far_by_near);
      pm.x = float4(2.0f / _bitmap.size().x, 0, 0, 0);
      pm.y = float4(0, 2.0f / _bitmap.size().y, 0, 0);
      pm.z = float4(0, 0, b, _far * b);
    } else {
      const auto f = 1.0f / yw::tan(_factor * 0.5f);
      const auto a = f * _bitmap.size().y / _bitmap.size().x;
      constexpr float b = -1.0f / (far_by_near - 1.0f);
      pm.x = float4(a, 0, 0, 0);
      pm.y = float4(0, f, 0, 0);
      pm.z = float4(0, 0, _far * b, b);
    }
    _mm_store_ps(vpm.x.data(), _mm_mul_ps(m.x, mm_set1(pm.x.x)));
    _mm_store_ps(vpm.y.data(), _mm_mul_ps(m.y, mm_set1(pm.y.y)));
    _mm_store_ps(vpm.z.data(), _mm_sub_ps(_mm_mul_ps(m.z, mm_set1(pm.z.z)), mm_set<3>(pm.z.w)));
    _cb_value.camera_pos = position;
    if (auto res = _cb.set(_cb_value); !res) return unexpected_error(res.error());
    return {};
  }

  std::expected<void, error_trace> clear_depth() {
    if (auto res = d3d.initialize(); !res) return unexpected_error(res.error());
    d3d.context()->ClearDepthStencilView(_dsv.get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
    return {};
  }

  std::expected<void, error_trace> clear(const color& Clear = colors::transparent) {
    if (auto res = d3d.initialize(); !res) return unexpected_error(res.error());
    d3d.context()->ClearDepthStencilView(_dsv.get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
    d3d.context()->ClearRenderTargetView(_rtv.get(), &Clear.r);
    return {};
  }

  std::expected<drawing, error_trace> begin_render() {
    if (!*this) return unexpected_error(errors::not_initialized, "Viewport not initialized");
    D3D11_VIEWPORT vp{0.0f, 0.0f, float(_bitmap.size().x), float(_bitmap.size().y), 0.0f, 1.0f};
    d3d.context()->RSSetViewports(1, &vp);
    if (auto res = drawing::create(_rtv.get(), _dsv.get(), source{})) return std::move(*res);
    else return unexpected_error(res.error());
  }

  std::expected<drawing, error_trace> begin_render(const color& Clear) {
    if (!*this) return unexpected_error(errors::not_initialized, "Viewport not initialized");
    if (auto res = clear(Clear); !res) return unexpected_error(res.error());
    D3D11_VIEWPORT vp{0.0f, 0.0f, float(_bitmap.size().x), float(_bitmap.size().y), 0.0f, 1.0f};
    d3d.context()->RSSetViewports(1, &vp);
    if (auto res = drawing::create(_rtv.get(), _dsv.get(), source{})) return std::move(*res);
    else return unexpected_error(res.error());
  }
};
} // namespace yw
