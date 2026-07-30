#pragma once
#include <ywx/shader.h>

namespace yw {

class rendering {
  ID3D11VertexShader* _vs{};
  ID3D11GeometryShader* _gs{};
  ID3D11PixelShader* _ps{};
  uint32_t _num_rtv = 0;
  uint32_t _num_dsv = 0;
  uint32_t _num_uav = 0;
  uint32_t _num_vs_cbuffers = 0;
  uint32_t _num_gs_cbuffers = 0;
  uint32_t _num_ps_cbuffers = 0;
  uint32_t _num_vs_resources = 0;
  uint32_t _num_gs_resources = 0;
  uint32_t _num_ps_resources = 0;

  std::expected<void, error> _render(uint32_t Count) {
    if (!(_vs && _ps))
      return std::unexpected(
        error(errors::invalid_operation, "Vertex shader and pixel shader must be set before rendering"));
    if (!_gs) d3d::context()->GSSetShader(nullptr, nullptr, 0);
    d3d::context()->Draw(Count, 0);
    return {};
  }

public:
  rendering() noexcept = default;
  rendering(rendering&&) noexcept = default;
  rendering& operator=(rendering&&) noexcept = default;

  rendering(const rendering&) = delete;
  rendering& operator=(const rendering&) = delete;

  template<typename... Ts> static std::expected<rendering, error> create(const Ts&... rtvs_dsv_uavs) {
    constexpr size_t i_dsv = inspect<castable_to<const Ts&, ID3D11DepthStencilView*>...>;
    constexpr size_t num_dsv = i_dsv < sizeof...(Ts);
    constexpr size_t num_rtv = num_dsv ? i_dsv : sizeof...(Ts);
    constexpr size_t num_uav = sizeof...(Ts) - (num_dsv ? i_dsv + 1 : 0);
    static_assert([]<size_t... Is>(sequence<Is...>) {
      return (castable_to<select_type<Is, Ts...>, ID3D11RenderTargetView*> && ...);
    }(make_sequence<0, num_rtv>()));
    static_assert([]<size_t... Is>(sequence<Is...>) {
      return (castable_to<select_type<Is, Ts...>, ID3D11UnorderedAccessView*> && ...);
    }(make_sequence<i_dsv + 1, sizeof...(Ts)>()));
    std::array<ID3D11RenderTargetView*, num_rtv> rtv{};
    [&]<size_t... Is>(sequence<Is...>) {
      ((rtv[Is] = static_cast<ID3D11RenderTargetView*>(select<Is>(rtvs_dsv_uavs...))), ...);
    }(make_sequence<0, num_rtv>());
    ID3D11DepthStencilView* dsv = nullptr;
    if (num_dsv) dsv = static_cast<ID3D11DepthStencilView*>(select<i_dsv>(rtvs_dsv_uavs...));
    if constexpr (num_uav > 0) {
      std::array<ID3D11UnorderedAccessView*, num_uav> buf{};
      [&]<size_t... Is>(sequence<Is...>) {
        ((buf[Is] = static_cast<ID3D11UnorderedAccessView*>(select<i_dsv + 1 + Is>(rtvs_dsv_uavs...))), ...);
      }(make_sequence<0, num_uav>());
      d3d::context()->OMSetRenderTargetsAndUnorderedAccessViews(
        uint32_t(num_rtv), rtv.data(), dsv, uint32_t(num_rtv), uint32_t(num_uav), buf.data(), nullptr);
    } else d3d::context()->OMSetRenderTargets(uint32_t(num_rtv), rtv.data(), dsv);
    rendering r{};
    r._num_rtv = uint32_t(num_rtv);
    r._num_dsv = uint32_t(num_dsv);
    r._num_uav = uint32_t(num_uav);
    return std::move(r);
  }

  template<castable_to<ID3D11VertexShader*> T, typename... Us>
  std::expected<void, error> set_vs(const T& vs, const Us&... cbuffers_resources) {
    constexpr size_t i_resources = inspect<castable_to<const Us&, ID3D11ShaderResourceView*>...>;
    constexpr size_t num_cbuffers = i_resources;
    constexpr size_t num_resources = sizeof...(Us) - i_resources;
    static_assert([]<size_t... Is>(sequence<Is...>) {
      return (castable_to<select_type<Is, Us...>, ID3D11Buffer*> && ...);
    }(make_sequence<0, i_resources>()));
    static_assert([]<size_t... Is>(sequence<Is...>) {
      return (castable_to<select_type<Is, Us>, ID3D11ShaderResourceView*> && ...);
    }(make_sequence<i_resources, sizeof...(Us)>()));
    _vs = static_cast<ID3D11VertexShader*>(vs);
    _num_vs_cbuffers = uint32_t(num_cbuffers);
    _num_vs_resources = uint32_t(num_resources);
    d3d::context()->VSSetShader(_vs, nullptr, 0);
    if constexpr (num_cbuffers > 0) {
      std::array<ID3D11Buffer*, num_cbuffers> buf{};
      [&]<size_t... Is>(sequence<Is...>) {
        ((buf[Is] = static_cast<ID3D11Buffer*>(select<Is>(cbuffers_resources...))), ...);
      }(make_sequence<0, num_cbuffers>());
      d3d::context()->VSSetConstantBuffers(0, uint32_t(num_cbuffers), buf.data());
    }
    if constexpr (num_resources > 0) {
      std::array<ID3D11ShaderResourceView*, num_resources> buf{};
      [&]<size_t... Is>(sequence<Is...>) {
        ((buf[Is] = static_cast<ID3D11ShaderResourceView*>(select<i_resources + Is>(cbuffers_resources...))), ...);
      }(make_sequence<0, num_resources>());
      d3d::context()->VSSetShaderResources(0, uint32_t(num_resources), buf.data());
    }
    return {};
  }

  template<castable_to<ID3D11GeometryShader*> T, typename... Us>
  std::expected<void, error> set_gs(const T& gs, const Us&... cbuffers_resources) {
    constexpr size_t i_resources = inspect<castable_to<const Us&, ID3D11ShaderResourceView*>...>;
    constexpr size_t num_cbuffers = i_resources;
    constexpr size_t num_resources = sizeof...(Us) - i_resources;
    static_assert([]<size_t... Is>(sequence<Is...>) {
      return (castable_to<select_type<Is, Us...>, ID3D11Buffer*> && ...);
    }(make_sequence<0, i_resources>()));
    static_assert([]<size_t... Is>(sequence<Is...>) {
      return (castable_to<select_type<Is, Us...>, ID3D11ShaderResourceView*> && ...);
    }(make_sequence<i_resources, sizeof...(Us)>()));
    _gs = static_cast<ID3D11GeometryShader*>(gs);
    _num_gs_cbuffers = uint32_t(num_cbuffers);
    _num_gs_resources = uint32_t(num_resources);
    d3d::context()->GSSetShader(_gs, nullptr, 0);
    if constexpr (num_cbuffers > 0) {
      std::array<ID3D11Buffer*, num_cbuffers> buf{};
      [&]<size_t... Is>(sequence<Is...>) {
        ((buf[Is] = static_cast<ID3D11Buffer*>(select<Is>(cbuffers_resources...))), ...);
      }(make_sequence<0, num_cbuffers>());
      d3d::context()->GSSetConstantBuffers(0, uint32_t(num_cbuffers), buf.data());
    }
    if constexpr (num_resources > 0) {
      std::array<ID3D11ShaderResourceView*, num_resources> buf{};
      [&]<size_t... Is>(sequence<Is...>) {
        ((buf[Is] = static_cast<ID3D11ShaderResourceView*>(select<i_resources + Is>(cbuffers_resources...))), ...);
      }(make_sequence<0, num_resources>());
      d3d::context()->GSSetShaderResources(0, uint32_t(num_resources), buf.data());
    }
    return {};
  }

  template<castable_to<ID3D11PixelShader*> T, typename... Us>
  std::expected<void, error> set_ps(const T& ps, const Us&... cbuffers_resources) {
    constexpr size_t i_resources = inspect<castable_to<const Us&, ID3D11ShaderResourceView*>...>;
    constexpr size_t num_cbuffers = i_resources;
    constexpr size_t num_resources = sizeof...(Us) - i_resources;
    static_assert([]<size_t... Is>(sequence<Is...>) {
      return (castable_to<select_type<Is, Us...>, ID3D11Buffer*> && ...);
    }(make_sequence<0, i_resources>()));
    static_assert([]<size_t... Is>(sequence<Is...>) {
      return (castable_to<select_type<Is, Us...>, ID3D11ShaderResourceView*> && ...);
    }(make_sequence<i_resources, sizeof...(Us)>()));
    _ps = static_cast<ID3D11PixelShader*>(ps);
    _num_ps_cbuffers = uint32_t(num_cbuffers);
    _num_ps_resources = uint32_t(num_resources);
    d3d::context()->PSSetShader(_ps, nullptr, 0);
    if constexpr (num_cbuffers > 0) {
      std::array<ID3D11Buffer*, num_cbuffers> buf{};
      [&]<size_t... Is>(sequence<Is...>) {
        ((buf[Is] = static_cast<ID3D11Buffer*>(select<Is>(cbuffers_resources...))), ...);
      }(make_sequence<0, num_cbuffers>());
      d3d::context()->PSSetConstantBuffers(0, uint32_t(num_cbuffers), buf.data());
    }
    if constexpr (num_resources > 0) {
      std::array<ID3D11ShaderResourceView*, num_resources> buf{};
      [&]<size_t... Is>(sequence<Is...>) {
        ((buf[Is] = static_cast<ID3D11ShaderResourceView*>(select<i_resources + Is>(cbuffers_resources...))), ...);
      }(make_sequence<0, num_resources>());
      d3d::context()->PSSetShaderResources(0, uint32_t(num_resources), buf.data());
    }
    return {};
  }

  std::expected<void, error> render_points(uint32_t NumPoints) {
    d3d::context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
    if (auto res = _render(NumPoints); !res) return res.error().relay();
    return {};
  }

  std::expected<void, error> render_lines(uint32_t NumLines) {
    d3d::context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
    if (auto res = _render(NumLines * 2); !res) return res.error().relay();
    return {};
  }

  std::expected<void, error> render_triangles(uint32_t NumTriangles) {
    d3d::context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    if (auto res = _render(NumTriangles * 3); !res) return res.error().relay();
    return {};
  }

  std::expected<void, error> render_any(
    uint32_t NumIndices, std::optional<D3D11_PRIMITIVE_TOPOLOGY> Topology = std::nullopt) {
    if (Topology) d3d::context()->IASetPrimitiveTopology(*Topology);
    if (auto res = _render(NumIndices); !res) return res.error().relay();
    return {};
  }
};
} // namespace yw
