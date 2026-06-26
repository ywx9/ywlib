#pragma once
#include "ywx/buffer.h"

namespace yw {

/// MARK: vertex_shader

class vertex_shader {
  comptr<ID3D11VertexShader> _shader{};

  std::expected<void, error> initialize(string_view<char> Hlsl, const char* Entry) {
    comptr<ID3D10Blob> b, e;
    auto hr = ::D3DCompile(Hlsl.data(), Hlsl.size(), 0, 0, 0, Entry, "vs_5_0", 1 << 11, 0, &b.get(), &e.get());
    if (FAILED(hr)) {
      auto mes = string_view<char>((char*)e->GetBufferPointer(), e->GetBufferSize());
      auto s = mes.empty() ? string<char>("D3DCompile failed") : format("D3DCompile failed\n{}", mes);
      return std::unexpected(error(errors::operation_failed, std::move(s), int32_t(hr)));
    }
    hresult_test(d3d::device()->CreateVertexShader, b->GetBufferPointer(), b->GetBufferSize(), nullptr, &_shader.get());
    return {};
  }

public:
  explicit operator bool() const noexcept { return bool(_shader); }
  explicit operator ID3D11VertexShader*&() noexcept { return _shader.get(); }
  explicit operator ID3D11VertexShader*() const noexcept { return _shader.get(); }

  vertex_shader() noexcept = default;

  template<stringable<char> S>
  vertex_shader(S&& Hlsl, const char* Entry = "vsmain", const source_line& sl = here()) {
    if (auto res = initialize(string_view<char>(Hlsl), Entry); !res) res.error().add_footprint().go_off(sl);
  }

  template<typename... As> static std::expected<vertex_shader, error> create(As&&... Args) {
    vertex_shader vs{};
    if (auto res = vs.initialize(static_cast<As&&>(Args)...); !res) return res.error().relay();
    return vs;
  }

  void set_shader() const noexcept { d3d::context()->VSSetShader(_shader.get(), nullptr, 0); }
  static void set_shader(const vertex_shader& shader) noexcept { shader.set_shader(); }

  template<specialization_of<constant_buffer>... Ts> static void set_cbuffer(const Ts&... as) {
    std::array<ID3D11Buffer*, sizeof...(Ts)> a{(ID3D11Buffer*)as...};
    d3d::context()->VSSetConstantBuffers(0, UINT(a.size()), a.data());
  }

  template<castable_to<ID3D11ShaderResourceView*>... Ts> static void set_resource(const Ts&... as) {
    std::array<ID3D11ShaderResourceView*, sizeof...(Ts)> a{(ID3D11ShaderResourceView*)as...};
    d3d::context()->VSSetShaderResources(0, UINT(a.size()), a.data());
  }
};

/// MARK: pixel_shader

class pixel_shader {
  comptr<ID3D11PixelShader> _shader{};

  std::expected<void, error> initialize(string_view<char> Hlsl, const char* Entry) {
    comptr<ID3D10Blob> b, e;
    auto hr = ::D3DCompile(Hlsl.data(), Hlsl.size(), 0, 0, 0, Entry, "ps_5_0", 1 << 11, 0, &b.get(), &e.get());
    if (FAILED(hr)) {
      auto mes = string_view<char>((char*)e->GetBufferPointer(), e->GetBufferSize());
      auto s = mes.empty() ? string<char>("D3DCompile failed") : format("D3DCompile failed\n{}", mes);
      return std::unexpected(error(errors::operation_failed, std::move(s), int32_t(hr)));
    }
    hresult_test(d3d::device()->CreatePixelShader, b->GetBufferPointer(), b->GetBufferSize(), nullptr, &_shader.get());
    return {};
  }

public:
  explicit operator bool() const noexcept { return bool(_shader); }
  explicit operator ID3D11PixelShader*&() noexcept { return _shader.get(); }
  explicit operator ID3D11PixelShader*() const noexcept { return _shader.get(); }

  pixel_shader() noexcept = default;

  template<stringable<char> S>
  pixel_shader(S&& Hlsl, const char* Entry = "psmain", const source_line& sl = here()) {
    if (auto res = initialize(string_view<char>(Hlsl), Entry); !res) res.error().add_footprint().go_off(sl);
  }

  template<typename... As> requires constructible<pixel_shader, As...>
  static std::expected<pixel_shader, error> create(As&&... Args) {
    pixel_shader ps{};
    if (auto res = ps.initialize(static_cast<As&&>(Args)...); !res) return res.error().relay();
    return ps;
  }

  void set_shader() const noexcept { d3d::context()->PSSetShader(_shader.get(), nullptr, 0); }
  static void set_shader(const pixel_shader& shader) noexcept { shader.set_shader(); }

  template<specialization_of<constant_buffer>... Ts> static void set_cbuffer(const Ts&... as) {
    std::array<ID3D11Buffer*, sizeof...(Ts)> a{(ID3D11Buffer*)as...};
    d3d::context()->PSSetConstantBuffers(0, UINT(a.size()), a.data());
  }

  template<castable_to<ID3D11ShaderResourceView*>... Ts> static void set_resource(const Ts&... as) {
    std::array<ID3D11ShaderResourceView*, sizeof...(Ts)> a{(ID3D11ShaderResourceView*)as...};
    d3d::context()->PSSetShaderResources(0, UINT(a.size()), a.data());
  }
};

/// MARK: geometry_shader

class geometry_shader {
  comptr<ID3D11GeometryShader> _shader{};

  std::expected<void, error> initialize(string_view<char> Hlsl, const char* Entry) {
    comptr<ID3D10Blob> b, e;
    auto hr = ::D3DCompile(Hlsl.data(), Hlsl.size(), 0, 0, 0, Entry, "gs_5_0", 1 << 11, 0, &b.get(), &e.get());
    if (FAILED(hr)) {
      auto mes = string_view<char>((char*)e->GetBufferPointer(), e->GetBufferSize());
      auto s = mes.empty() ? string<char>("D3DCompile failed") : format("D3DCompile failed\n{}", mes);
      return std::unexpected(error(errors::operation_failed, std::move(s), int32_t(hr)));
    }
    hresult_test(
      d3d::device()->CreateGeometryShader, b->GetBufferPointer(), b->GetBufferSize(), nullptr, &_shader.get());
    return {};
  }

public:
  explicit operator bool() const noexcept { return bool(_shader); }
  explicit operator ID3D11GeometryShader*&() noexcept { return _shader.get(); }
  explicit operator ID3D11GeometryShader*() const noexcept { return _shader.get(); }

  geometry_shader() noexcept = default;

  template<stringable<char> S>
  geometry_shader(S&& Hlsl, const char* Entry = "gsmain", const source_line& sl = here()) {
    if (auto res = initialize(string_view<char>(Hlsl), Entry); !res) res.error().add_footprint().go_off(sl);
  }

  template<typename... As> requires constructible<geometry_shader, As...>
  static std::expected<geometry_shader, error> create(As&&... Args) {
    geometry_shader gs{};
    if (auto res = gs.initialize(static_cast<As&&>(Args)...); !res) return res.error().relay();
    return gs;
  }

  void set_shader() const noexcept { d3d::context()->GSSetShader(_shader.get(), nullptr, 0); }
  static void set_shader(const geometry_shader& shader) noexcept { shader.set_shader(); }

  template<specialization_of<constant_buffer>... Ts> static void set_cbuffer(const Ts&... as) {
    std::array<ID3D11Buffer*, sizeof...(Ts)> a{(ID3D11Buffer*)as...};
    d3d::context()->GSSetConstantBuffers(0, UINT(a.size()), a.data());
  }

  template<castable_to<ID3D11ShaderResourceView*>... Ts> static void set_resource(const Ts&... as) {
    std::array<ID3D11ShaderResourceView*, sizeof...(Ts)> a{(ID3D11ShaderResourceView*)as...};
    d3d::context()->GSSetShaderResources(0, UINT(a.size()), a.data());
  }
};

/// MARK: compute_shader

class compute_shader {
  comptr<ID3D11ComputeShader> _shader{};

  std::expected<void, error> initialize(string_view<char> Hlsl, const char* Entry) {
    comptr<ID3D10Blob> b, e;
    auto hr = ::D3DCompile(Hlsl.data(), Hlsl.size(), 0, 0, 0, Entry, "cs_5_0", 1 << 11, 0, &b.get(), &e.get());
    if (FAILED(hr)) {
      auto mes = string_view<char>((char*)e->GetBufferPointer(), e->GetBufferSize());
      auto s = mes.empty() ? string<char>("D3DCompile failed") : format("D3DCompile failed\n{}", mes);
      return std::unexpected(error(errors::operation_failed, std::move(s), int32_t(hr)));
    }
    hresult_test(
      d3d::device()->CreateComputeShader, b->GetBufferPointer(), b->GetBufferSize(), nullptr, &_shader.get());
    return {};
  }

public:
  explicit operator bool() const noexcept { return bool(_shader); }
  explicit operator ID3D11ComputeShader*&() noexcept { return _shader.get(); }
  explicit operator ID3D11ComputeShader*() const noexcept { return _shader.get(); }

  compute_shader() noexcept = default;

  template<stringable<char> S>
  compute_shader(S&& Hlsl, const char* Entry = "csmain", const source_line& sl = here()) {
    if (auto res = initialize(string_view<char>(Hlsl), Entry); !res) res.error().add_footprint().go_off(sl);
  }

  template<typename... As> requires constructible<compute_shader, As...>
  static std::expected<compute_shader, error> create(As&&... Args) {
    compute_shader cs{};
    if (auto res = cs.initialize(static_cast<As&&>(Args)...); !res) return res.error().relay();
    return cs;
  }

  void set_shader() const noexcept { d3d::context()->CSSetShader(_shader.get(), nullptr, 0); }
  static void set_shader(const compute_shader& shader) noexcept { shader.set_shader(); }

  template<specialization_of<constant_buffer>... Ts> static void set_cbuffer(const Ts&... as) {
    std::array<ID3D11Buffer*, sizeof...(Ts)> a{(ID3D11Buffer*)as...};
    d3d::context()->CSSetConstantBuffers(0, UINT(a.size()), a.data());
  }

  template<castable_to<ID3D11ShaderResourceView*>... Ts> static void set_resource(const Ts&... as) {
    std::array<ID3D11ShaderResourceView*, sizeof...(Ts)> a{(ID3D11ShaderResourceView*)as...};
    d3d::context()->CSSetShaderResources(0, UINT(a.size()), a.data());
  }

  template<castable_to<ID3D11UnorderedAccessView*>... Ts> static void set_rwbuffer(const Ts&... as) {
    std::array<ID3D11UnorderedAccessView*, sizeof...(Ts)> a{(ID3D11UnorderedAccessView*)as...};
    d3d::context()->CSSetUnorderedAccessViews(0, UINT(a.size()), a.data(), nullptr);
  }

  static void dispatch(const arithmetic auto Parallel, const uint32_t NumThreads = 1024) {
    d3d::context()->Dispatch((uint32_t(Parallel) + NumThreads - 1) / NumThreads, 1, 1);
  }
};
} // namespace yw
