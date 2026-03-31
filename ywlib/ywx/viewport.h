#pragma once
#include "ywx/bitmap.h"
#include "ywx/buffer.h"

namespace yw {

using float4x4 = vector4<vector4<float>>;

inline void rotation_matrix(float4 rad, float4x4& out) {
  const auto c_ = vapply_r<float4>(yw::cos, rad);
  const auto s_ = vapply_r<float4>(yw::sin, rad);
  const auto c = _mm_load_ps(c_.data());
  const auto s = _mm_load_ps(s_.data());
  // const auto t0 =
}

/// \note right-handed coordinate system; X is right, Y is up, Z is outward
/// \note reverse-z

class viewport {
  static constexpr float4x4 identity = {float4(1, 0, 0, 0), float4(0, 1, 0, 0), float4(0, 0, 1, 0), float4(0, 0, 0, 1)};

public:
  struct cb {
    float4x4 view = identity;
    float4x4 projection = identity;
    float4x4 view_projection = identity;
    float4 camera_pos = {};
  };

private:
  cb _matrices{};
  bitmap _bitmap{};
  comptr<ID3D11Texture2D> _texture{};
  comptr<ID3D11Texture2D> _depth{};
  comptr<ID3D11ShaderResourceView> _srv{};
  comptr<ID3D11RenderTargetView> _rtv{};
  comptr<ID3D11DepthStencilView> _dsv{};
  constant_buffer<cb> _cb{};

  float _factor{pi / 2};
  bool _orthographic{};

public:
  float4 offset{};
  float4 rotation{};
  float4 position{};

  viewport() noexcept = default;

  static std::expected<viewport, error_trace> create(uint2 Size) {
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
    if (auto res = constant_buffer<cb>::create(vp._matrices)) vp._cb = std::move(*res);
    else return unexpected_error(res.error());
    return vp;
  }

  void perspective(float1 fov_deg) noexcept {
    _factor = 1.0f / std::tan(fov_deg.x * float(pi) / 360.0f);
    _orthographic = false;
  }

  void orthographic(float1 magnification) noexcept {
    _factor = magnification.x;
    _orthographic = true;
  }

  const auto& view_matrix() const noexcept { return _matrices.view; }
  const auto& projection_matrix() const noexcept { return _matrices.projection; }
  const auto& view_projection_matrix() const noexcept { return _matrices.view_projection; }

  std::expected<void, error_trace> update() {
    // updates constant buffer
  }
};
} // namespace yw
