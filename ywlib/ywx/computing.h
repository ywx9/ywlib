#pragma once
#include <ywx/shader.h>

namespace yw {

class computing {
  bool _active = false;
  ID3D11ComputeShader* _cs = nullptr;
  uint32_t _num_cbuffers = 0;
  uint32_t _num_resources = 0;
  uint32_t _num_rw_resources = 0;

  std::expected<void, error> _ready() const {
    if (!_cs) return std::unexpected(error(errors::invalid_operation, "Compute shader must be set before computing"));
    return {};
  }

public:
  computing() noexcept = default;
  ~computing() { close(); }

  computing(computing&& Other) noexcept { *this = std::move(Other); }

  computing& operator=(computing&& Other) noexcept {
    if (this == &Other) return *this;
    close();
    _active = std::exchange(Other._active, false);
    _cs = std::exchange(Other._cs, nullptr);
    _num_cbuffers = std::exchange(Other._num_cbuffers, 0);
    _num_resources = std::exchange(Other._num_resources, 0);
    _num_rw_resources = std::exchange(Other._num_rw_resources, 0);
    return *this;
  }

  computing(const computing&) = delete;
  computing& operator=(const computing&) = delete;

  void close() noexcept {
    if (!_active) return;
    d3d::context()->CSSetShader(nullptr, nullptr, 0);
    if (_num_cbuffers > 0) {
      std::array<ID3D11Buffer*, 16> null_cbuffers{};
      d3d::context()->CSSetConstantBuffers(0, _num_cbuffers, null_cbuffers.data());
    }
    if (_num_resources > 0) {
      std::array<ID3D11ShaderResourceView*, 128> null_resources{};
      d3d::context()->CSSetShaderResources(0, _num_resources, null_resources.data());
    }
    if (_num_rw_resources > 0) {
      std::array<ID3D11UnorderedAccessView*, 64> null_rw_resources{};
      d3d::context()->CSSetUnorderedAccessViews(0, _num_rw_resources, null_rw_resources.data(), nullptr);
    }
    _active = false;
    _cs = nullptr;
    _num_cbuffers = 0;
    _num_resources = 0;
    _num_rw_resources = 0;
  }

  template<castable_to<ID3D11UnorderedAccessView*>... Ts> static std::expected<computing, error> create(const Ts&... rw_resources) {
    computing c{};
    c._active = true;
    c._num_rw_resources = uint32_t(sizeof...(Ts));
    if constexpr (sizeof...(Ts) > 0) {
      std::array<ID3D11UnorderedAccessView*, sizeof...(Ts)> uavs{static_cast<ID3D11UnorderedAccessView*>(rw_resources)...};
      d3d::context()->CSSetUnorderedAccessViews(0, uint32_t(uavs.size()), uavs.data(), nullptr);
    }
    return std::move(c);
  }

  explicit computing(castable_to<ID3D11UnorderedAccessView*> auto const&... rw_resources, const source_line& sl = here()) {
    if (auto res = create(rw_resources...); !res) res.error().go_off(sl);
    else *this = std::move(*res);
  }

  template<typename T, typename... Us> requires castable_to<const T&, ID3D11ComputeShader*>
  std::expected<void, error> set_cs(const T& cs, const Us&... cbuffers_resources) {
    constexpr size_t i_resources = inspect<castable_to<const Us&, ID3D11ShaderResourceView*>...>;
    constexpr size_t num_cbuffers = i_resources;
    constexpr size_t num_resources = sizeof...(Us) - i_resources;
    static_assert([]<size_t... Is>(sequence<Is...>) {
      return (castable_to<select_type<Is, Us...>, ID3D11Buffer*> && ...);
    }(make_sequence<0, i_resources>()));
    static_assert([]<size_t... Is>(sequence<Is...>) {
      return (castable_to<select_type<Is, Us...>, ID3D11ShaderResourceView*> && ...);
    }(make_sequence<i_resources, sizeof...(Us)>()));
    _cs = static_cast<ID3D11ComputeShader*>(cs);
    _num_cbuffers = uint32_t(num_cbuffers);
    _num_resources = uint32_t(num_resources);
    d3d::context()->CSSetShader(_cs, nullptr, 0);
    if constexpr (num_cbuffers > 0) {
      std::array<ID3D11Buffer*, num_cbuffers> buf{};
      [&]<size_t... Is>(sequence<Is...>) {
        ((buf[Is] = static_cast<ID3D11Buffer*>(select<Is>(cbuffers_resources...))), ...);
      }(make_sequence<0, num_cbuffers>());
      d3d::context()->CSSetConstantBuffers(0, uint32_t(num_cbuffers), buf.data());
    }
    if constexpr (num_resources > 0) {
      std::array<ID3D11ShaderResourceView*, num_resources> buf{};
      [&]<size_t... Is>(sequence<Is...>) {
        ((buf[Is] = static_cast<ID3D11ShaderResourceView*>(select<i_resources + Is>(cbuffers_resources...))), ...);
      }(make_sequence<0, num_resources>());
      d3d::context()->CSSetShaderResources(0, uint32_t(num_resources), buf.data());
    }
    return {};
  }

  std::expected<void, error> compute(uint32_t ElementCount, uint32_t ThreadsPerGroup = 1024) {
    if (auto res = _ready(); !res) return res.error().relay();
    if (ThreadsPerGroup == 0) return std::unexpected(error(errors::invalid_argument, "threads per group must be positive"));
    if (ElementCount == 0) return {};
    d3d::context()->Dispatch((ElementCount + ThreadsPerGroup - 1) / ThreadsPerGroup, 1, 1);
    return {};
  }

  std::expected<void, error> dispatch(uint32_t X, uint32_t Y = 1, uint32_t Z = 1) {
    if (auto res = _ready(); !res) return res.error().relay();
    d3d::context()->Dispatch(X, Y, Z);
    return {};
  }
};
} // namespace yw
