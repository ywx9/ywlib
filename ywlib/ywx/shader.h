#pragma once
#include "ywx/buffer.h"

namespace yw {

//////////////////////////////////////// MARK: VERTEX SHADER

class vertex_shader {
  comptr<ID3D11VertexShader> _shader{};

public:
  explicit operator bool() const noexcept { return bool(_shader); }
  explicit operator ID3D11VertexShader*&() noexcept { return _shader.get(); }
  explicit operator ID3D11VertexShader*() const noexcept { return _shader.get(); }

  vertex_shader() noexcept = default;

  template<stringable<char> S>
  static std::expected<vertex_shader, error_trace> create(S&& Hlsl, const char* Entry = "vsmain") {
    const auto hlsl = std::string_view(Hlsl);
    vertex_shader shader;
    comptr<ID3D10Blob> b, e;
    auto hr = ::D3DCompile(hlsl.data(), hlsl.size(), 0, 0, 0, Entry, "vs_5_0", 1 << 11, 0, &b.get(), &e.get());
    if (FAILED(hr)) {
      auto mes = e ? std::string_view((char*)e->GetBufferPointer(), e->GetBufferSize()) : std::string_view{};
      auto s = mes.empty() ? std::string("D3DCompile failed") : format("D3DCompile failed\n{}", mes);
      return unexpected_error(errors::operation_failed, s, int32_t(hr));
    }
    hr = d3d().device()->CreateVertexShader(b->GetBufferPointer(), b->GetBufferSize(), nullptr, &shader._shader.get());
    if (FAILED(hr)) {
      return unexpected_error(errors::operation_failed, "Failed to create vertex shader", int32_t(hr));
    } else return shader;
  }

  void set_shader() const noexcept { d3d().context()->VSSetShader(_shader.get(), nullptr, 0); }
  static void set_shader(const vertex_shader& shader) noexcept { shader.set_shader(); }

  template<specialization_of<constant_buffer>... Ts> static void set_cbuffer(const Ts&... as) {
    std::array<ID3D11Buffer*, sizeof...(Ts)> a{(ID3D11Buffer*)as...};
    d3d().context()->VSSetConstantBuffers(0, UINT(a.size()), a.data());
  }

  template<castable_to<ID3D11ShaderResourceView*>... Ts> static void set_resource(const Ts&... as) {
    std::array<ID3D11ShaderResourceView*, sizeof...(Ts)> a{(ID3D11ShaderResourceView*)as...};
    d3d().context()->VSSetShaderResources(0, UINT(a.size()), a.data());
  }
};

//////////////////////////////////////// MARK: PIXEL SHADER

class pixel_shader {
  comptr<ID3D11PixelShader> _shader{};

public:
  explicit operator bool() const noexcept { return bool(_shader); }
  explicit operator ID3D11PixelShader*&() noexcept { return _shader.get(); }
  explicit operator ID3D11PixelShader*() const noexcept { return _shader.get(); }

  pixel_shader() noexcept = default;

  template<stringable<char> S>
  static std::expected<pixel_shader, error_trace> create(S&& Hlsl, const char* Entry = "psmain") {
    const auto hlsl = std::string_view(Hlsl);
    pixel_shader shader;
    comptr<ID3D10Blob> b, e;
    auto hr = ::D3DCompile(hlsl.data(), hlsl.size(), 0, 0, 0, Entry, "ps_5_0", 1 << 11, 0, &b.get(), &e.get());
    if (FAILED(hr)) {
      auto mes = e ? std::string_view((char*)e->GetBufferPointer(), e->GetBufferSize()) : std::string_view{};
      auto s = mes.empty() ? std::string("D3DCompile failed") : format("D3DCompile failed\n{}", mes);
      return unexpected_error(errors::operation_failed, s, int32_t(hr));
    }
    hr = d3d().device()->CreatePixelShader(b->GetBufferPointer(), b->GetBufferSize(), nullptr, &shader._shader.get());
    if (FAILED(hr)) {
      return unexpected_error(errors::operation_failed, "Failed to create pixel shader", int32_t(hr));
    } else return shader;
  }

  void set_shader() const noexcept { d3d().context()->PSSetShader(_shader.get(), nullptr, 0); }
  static void set_shader(const pixel_shader& shader) noexcept { shader.set_shader(); }

  template<specialization_of<constant_buffer>... Ts> static void set_cbuffer(const Ts&... as) {
    std::array<ID3D11Buffer*, sizeof...(Ts)> a{(ID3D11Buffer*)as...};
    d3d().context()->PSSetConstantBuffers(0, UINT(a.size()), a.data());
  }

  template<castable_to<ID3D11ShaderResourceView*>... Ts> static void set_resource(const Ts&... as) {
    std::array<ID3D11ShaderResourceView*, sizeof...(Ts)> a{(ID3D11ShaderResourceView*)as...};
    d3d().context()->PSSetShaderResources(0, UINT(a.size()), a.data());
  }
};

//////////////////////////////////////// MARK: GEOMETRY SHADER

class geometry_shader {
  comptr<ID3D11GeometryShader> _shader{};

public:
  explicit operator bool() const noexcept { return bool(_shader); }
  explicit operator ID3D11GeometryShader*&() noexcept { return _shader.get(); }
  explicit operator ID3D11GeometryShader*() const noexcept { return _shader.get(); }

  geometry_shader() noexcept = default;

  template<stringable<char> S>
  static std::expected<geometry_shader, error_trace> create(S&& Hlsl, const char* Entry = "gsmain") {
    const auto hlsl = std::string_view(Hlsl);
    geometry_shader shader;
    comptr<ID3D10Blob> b, e;
    auto hr = ::D3DCompile(hlsl.data(), hlsl.size(), 0, 0, 0, Entry, "gs_5_0", 1 << 11, 0, &b.get(), &e.get());
    if (FAILED(hr)) {
      auto mes = e ? std::string_view((char*)e->GetBufferPointer(), e->GetBufferSize()) : std::string_view{};
      auto s = mes.empty() ? std::string("D3DCompile failed") : format("D3DCompile failed\n{}", mes);
      return unexpected_error(errors::operation_failed, s, int32_t(hr));
    }
    hr = d3d().device()->CreateGeometryShader(b->GetBufferPointer(), b->GetBufferSize(), nullptr, &shader._shader.get());
    if (FAILED(hr)) {
      return unexpected_error(errors::operation_failed, "Failed to create geometry shader", int32_t(hr));
    } else return shader;
  }

  void set_shader() const noexcept { d3d().context()->GSSetShader(_shader.get(), nullptr, 0); }
  static void set_shader(const geometry_shader& shader) noexcept { shader.set_shader(); }

  template<specialization_of<constant_buffer>... Ts> static void set_cbuffer(const Ts&... as) {
    std::array<ID3D11Buffer*, sizeof...(Ts)> a{(ID3D11Buffer*)as...};
    d3d().context()->GSSetConstantBuffers(0, UINT(a.size()), a.data());
  }

  template<castable_to<ID3D11ShaderResourceView*>... Ts> static void set_resource(const Ts&... as) {
    std::array<ID3D11ShaderResourceView*, sizeof...(Ts)> a{(ID3D11ShaderResourceView*)as...};
    d3d().context()->GSSetShaderResources(0, UINT(a.size()), a.data());
  }
};

//////////////////////////////////////// MARK: COMPUTE SHADER

class compute_shader {
  comptr<ID3D11ComputeShader> _shader{};

public:
  explicit operator bool() const noexcept { return bool(_shader); }
  explicit operator ID3D11ComputeShader*&() noexcept { return _shader.get(); }
  explicit operator ID3D11ComputeShader*() const noexcept { return _shader.get(); }

  compute_shader() noexcept = default;

  template<stringable<char> S>
  static std::expected<compute_shader, error_trace> create(S&& Hlsl, const char* Entry = "csmain") {
    const auto hlsl = std::string_view(Hlsl);
    compute_shader shader;
    comptr<ID3D10Blob> b, e;
    auto hr = ::D3DCompile(hlsl.data(), hlsl.size(), 0, 0, 0, Entry, "cs_5_0", 1 << 11, 0, &b.get(), &e.get());
    if (FAILED(hr)) {
      auto mes = e ? std::string_view((char*)e->GetBufferPointer(), e->GetBufferSize()) : std::string_view{};
      auto s = mes.empty() ? std::string("D3DCompile failed") : format("D3DCompile failed\n{}", mes);
      return unexpected_error(errors::operation_failed, s, int32_t(hr));
    }
    hr = d3d().device()->CreateComputeShader(b->GetBufferPointer(), b->GetBufferSize(), nullptr, &shader._shader.get());
    if (FAILED(hr)) {
      return unexpected_error(errors::operation_failed, "Failed to create compute shader", int32_t(hr));
    } else return shader;
  }

  void set_shader() const noexcept { d3d().context()->CSSetShader(_shader.get(), nullptr, 0); }
  static void set_shader(const compute_shader& shader) noexcept { shader.set_shader(); }

  template<specialization_of<constant_buffer>... Ts> static void set_cbuffer(const Ts&... as) {
    std::array<ID3D11Buffer*, sizeof...(Ts)> a{(ID3D11Buffer*)as...};
    d3d().context()->CSSetConstantBuffers(0, UINT(a.size()), a.data());
  }

  template<castable_to<ID3D11ShaderResourceView*>... Ts> static void set_resource(const Ts&... as) {
    std::array<ID3D11ShaderResourceView*, sizeof...(Ts)> a{(ID3D11ShaderResourceView*)as...};
    d3d().context()->CSSetShaderResources(0, UINT(a.size()), a.data());
  }

  template<castable_to<ID3D11UnorderedAccessView*>... Ts> static void set_rwbuffer(const Ts&... as) {
    std::array<ID3D11UnorderedAccessView*, sizeof...(Ts)> a{(ID3D11UnorderedAccessView*)as...};
    d3d().context()->CSSetUnorderedAccessViews(0, UINT(a.size()), a.data(), nullptr);
  }

  static void dispatch(const arithmetic auto Parallel, const uint32_t NumThreads = 1024) {
    d3d().context()->Dispatch((uint32_t(Parallel) + NumThreads - 1) / NumThreads, 1, 1);
  }
};
} // namespace yw
