#pragma once

#include "win.hpp"

#include <d2d1_3.h>
#pragma comment(lib, "d2d1.lib")

#include <d3d11_3.h>
#pragma comment(lib, "d3d11.lib")

#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

#include <dwrite_3.h>
#pragma comment(lib, "dwrite.lib")

#include <dxgi1_6.h>
#pragma comment(lib, "dxgi.lib")

#include <wincodec.h>
#pragma comment(lib, "windowscodecs.lib")

export namespace yw {

using id3d11buffer = ::ID3D11Buffer;
using id3d11shaderresourceview = ::ID3D11ShaderResourceView;
using id3d11unorderedaccessview = ::ID3D11UnorderedAccessView;

/// deleter for COM objects
template<typename Com> struct com_deleter {
  constexpr com_deleter() noexcept = default;
  template<typename U> constexpr com_deleter(const com_deleter<U>&) noexcept {}
  constexpr void operator()(Com* p) const noexcept { p ? void(p->Release()) : void(); }
};

/// COM smart pointer
template<typename Com> class comptr : public std::unique_ptr<Com, com_deleter<Com>> {
public:
  using base = std::unique_ptr<Com, com_deleter<Com>>;
  using base::base;
  using base::operator=;
  using base::operator->;
  using base::operator bool;
  constexpr operator Com*() const noexcept { return base::get(); }
  Com** init(const source& _ = {}) & {
    if (bool(*this)) main::log.format(logger::critical, "COM object already initialized: {}", _);
    return reinterpret_cast<Com**>(this);
  }
  template<typename OtherCom> HRESULT as(comptr<OtherCom>& other) const noexcept { //
    return base::get()->QueryInterface(IID_PPV_ARGS(other.init()));
  }
};
static_assert(sizeof(comptr<IUnknown>) == sizeof(IUnknown*));

namespace main::system {

class com_init {
  bool _initialized{};
public:
  explicit operator bool() const noexcept { return _initialized; }
  ~com_init() noexcept { _initialized ? ::CoUninitialize() : void(); }
  com_init(COINIT coinit) noexcept {
    if (auto r = ::CoInitializeEx(0, coinit); r == S_OK) _initialized = true;
    else if (r != S_FALSE) main::log(logger::warning, "COM library is already initialized");
    else main::log.format(logger::critical, "failed to initialize COM: {}", format_error(r));
  }
};
inline const com_init com_initialized{COINIT_APARTMENTTHREADED};

comptr<ID3D11Device> d3d_device{};
comptr<ID3D11DeviceContext> d3d_context{};
comptr<IDXGIFactory2> dxgi_factory{};
comptr<IDXGIDevice2> dxgi_device{};
comptr<ID2D1Factory1> d2d_factory{};
comptr<ID2D1Device> d2d_device{};
comptr<ID2D1DeviceContext> d2d_context{};
comptr<IDWriteFactory1> dwrite_factory{};
comptr<IWICImagingFactory> wic_factory{};
comptr<ID3D11BlendState> d3d_blend_state{};
comptr<ID3D11SamplerState> d3d_sampler_state{};
comptr<ID3D11RasterizerState> d3d_rasterizer_state{};

inline const bool directx_initialized = [] {
  HRESULT hr{};
  try {
    // create D3D11 device and context
    const D3D_FEATURE_LEVEL feature_levels[] = {D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0};
    hr = ::D3D11CreateDevice(                                                                       //
      nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT, feature_levels, //
      arraysize(feature_levels), D3D11_SDK_VERSION, d3d_device.init(), nullptr, d3d_context.init());
    if (hr != S_OK) throw std::runtime_error("failed to create D3D11 device");
    // create default D3D11 blend state and set it to the context
    const D3D11_RENDER_TARGET_BLEND_DESC d3d_render_target_blend_desc = //
      {true, D3D11_BLEND(5), D3D11_BLEND(6), D3D11_BLEND_OP(1), D3D11_BLEND(5), D3D11_BLEND(6), D3D11_BLEND_OP(1), 0xf};
    const D3D11_BLEND_DESC blend_desc{false, false, {d3d_render_target_blend_desc}};
    hr = d3d_device->CreateBlendState(&blend_desc, d3d_blend_state.init());
    if (hr != S_OK) throw std::runtime_error("failed to create D3D11 blend state");
    else d3d_context->OMSetBlendState(d3d_blend_state, 0, 0xf);
    // create default D3D11 sampler state and set it to the context
    const D3D11_TEXTURE_ADDRESS_MODE wrap = D3D11_TEXTURE_ADDRESS_WRAP;
    const D3D11_SAMPLER_DESC sampler_desc = {D3D11_FILTER(1), wrap, wrap, wrap, 0, 1, D3D11_COMPARISON_FUNC(8)};
    hr = d3d_device->CreateSamplerState(&sampler_desc, d3d_sampler_state.init());
    if (hr != S_OK) throw std::runtime_error("failed to create D3D11 sampler state");
    else [&](auto p) { d3d_context->PSSetSamplers(0, 1, &p); }(d3d_sampler_state.get());
    // create default D3D11 rasterizer state and set it to the context
    const D3D11_RASTERIZER_DESC d3d_rasterizer_desc{D3D11_FILL_SOLID, D3D11_CULL_NONE, 1, 0, 0, 0, 1, 0, 1, 1};
    hr = d3d_device->CreateRasterizerState(&d3d_rasterizer_desc, d3d_rasterizer_state.init());
    if (hr != S_OK) throw std::runtime_error("failed to create D3D11 rasterizer state");
    else d3d_context->RSSetState(d3d_rasterizer_state);
    // create dxgi-factory
    hr = ::CreateDXGIFactory2(0, IID_PPV_ARGS(dxgi_factory.init()));
    if (hr != S_OK) throw std::runtime_error("failed to create DXGI factory");
    // create dxgi-device
    hr = d3d_device->QueryInterface(IID_PPV_ARGS(dxgi_device.init()));
    if (hr != S_OK) throw std::runtime_error("failed to create DXGI device");
    // create d2d-factory
    hr = ::D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, IID_PPV_ARGS(d2d_factory.init()));
    if (hr != S_OK) throw std::runtime_error("failed to create D2D1 factory");
    // create d2d-device
    hr = d2d_factory->CreateDevice(dxgi_device, d2d_device.init());
    if (hr != S_OK) throw std::runtime_error("failed to create D2D1 device");
    // create d2d-context
    hr = d2d_device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, d2d_context.init());
    if (hr != S_OK) throw std::runtime_error("failed to create D2D1 device context");
    // create dwrite-factory
    hr = ::DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory1), (IUnknown**)(dwrite_factory.init()));
    if (hr != S_OK) throw std::runtime_error("failed to create DWrite factory");
    // create wic-factory
    hr = ::CoCreateInstance(CLSID_WICImagingFactory, 0, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(wic_factory.init()));
    if (hr != S_OK) throw std::runtime_error("failed to create WIC imaging factory");
    return true;
  } catch (const std::exception& e) {
    main::log(logger::critical, e.what());
    if (hr != S_OK) main::log(logger::critical, format_error(hr));
    main::log(logger::critical, "failed to initialize DirectX system");
    return false;
  }
}();
}

template<typename T> class staging_buffer;

/// base class for GPU buffers
template<typename T> class buffer {
protected:
  comptr<ID3D11Buffer> d3d_buffer;
  buffer(const buffer&) = delete;
  buffer& operator=(const buffer&) = delete;
  buffer(numeric auto&& Count) noexcept : count(nat4(Count)) {}
public:
  using value_type = T;
  const nat4 count{};
  buffer() noexcept = default;
  buffer(buffer&& B) noexcept : d3d_buffer{mv(B.d3d_buffer)}, count{B.count} {}
  explicit operator bool() const noexcept { return bool(d3d_buffer); }
  operator id3d11buffer*() const noexcept { return d3d_buffer; }
  buffer& operator=(buffer&& B) noexcept {
    d3d_buffer = mv(B.d3d_buffer);
    const_cast<nat4&>(count) = B.count;
    return *this;
  }
  bool from(const buffer& Src, const source& _ = {}) noexcept {
    try {
      if (!Src) throw std::runtime_error("source buffer is not valid");
      if (count != Src.count) throw std::runtime_error("size of the source buffer must be the same as this buffer");
      main::system::d3d_context->CopyResource(*this, Src);
      return true;
    } catch (const std::exception& e) {
      main::log(logger::error, e.what());
      main::log(logger::error, "failed to copy buffer", _);
      return false;
    }
  }
  std::vector<T> to_cpu(const source& _ = {}) const noexcept;
  std::vector<T> to_cpu(staging_buffer<T>& Staging, const source& _ = {}) const noexcept;
};

/// staging buffer class for copying data to CPU
template<typename T> class staging_buffer : public buffer<T> {
public:
  staging_buffer() noexcept = default;
  explicit staging_buffer(const buffer<T>& Src, const source& _ = {}) : staging_buffer(Src.count, _) { buffer<T>::from(Src); }
  explicit staging_buffer(numeric auto&& Count, const source& _ = {}) noexcept : buffer<T>(Count) {
    HRESULT hr{};
    try {
      D3D11_BUFFER_DESC bd{nat4(sizeof(T) * buffer<T>::count), D3D11_USAGE_STAGING, 0, D3D11_CPU_ACCESS_READ};
      hr = main::system::d3d_device->CreateBuffer(&bd, nullptr, buffer<T>::d3d_buffer.init());
      if (hr != S_OK) throw std::runtime_error("failed to create buffer");
    } catch (const std::exception& e) {
      main::log(logger::error, e.what());
      if (hr != S_OK) main::log(logger::error, format_error(hr));
      main::log(logger::error, "failed to create staging buffer", _);
    }
  }
  array<T> to_cpu(const source& _ = {}) const noexcept {
    HRESULT hr{};
    try {
      if (!(*this)) throw std::runtime_error("the staging buffer is not valid");
      array<T> data(buffer<T>::count);
      D3D11_MAPPED_SUBRESOURCE msr{};
      if (hr = main::system::d3d_context->Map(*this, 0, D3D11_MAP_READ, 0, &msr); hr != S_OK) //
        throw std::runtime_error("failed to map staging buffer");
      memcpy(data.data(), msr.pData, sizeof(T) * buffer<T>::count);
      main::system::d3d_context->Unmap(*this, 0);
      return data;
    } catch (const std::exception& e) {
      main::log(logger::error, e.what());
      if (hr != S_OK) main::log(logger::error, format_error(hr));
      main::log(logger::error, "failed to copy buffer to CPU", _);
      return {};
    }
  }
};
template<typename T> staging_buffer(const buffer<T>&) -> staging_buffer<T>;

template<typename T> std::vector<T> buffer<T>::to_cpu(const source& _) const noexcept {
  staging_buffer<T> staging(*this, _);
  return staging.to_cpu(_);
}
template<typename T> std::vector<T> buffer<T>::to_cpu(staging_buffer<T>& Staging, const source& _) const noexcept {
  Staging.from(*this, _);
  return Staging.to_cpu(_);
}

/// constant buffer class
template<typename T> requires ((sizeof(T) % 16) == 0) class constant_buffer : public buffer<T> {
  static constexpr D3D11_BUFFER_DESC bd = {sizeof(T), D3D11_USAGE(2), D3D11_BIND_FLAG(4), D3D11_CPU_ACCESS_WRITE};
public:
  constant_buffer() noexcept = default;
  template<convertible_to<T> U = T> explicit constant_buffer(U&& Val, const source& _ = {}) noexcept : buffer<T>(1) {
    HRESULT hr{};
    try {
      if constexpr (included_in<U, T&, const T&>) {
        D3D11_SUBRESOURCE_DATA srd{&Val};
        auto hr = main::system::d3d_device->CreateBuffer(&bd, &srd, buffer<T>::d3d_buffer.init());
        if (hr != S_OK) throw std::runtime_error("failed to create buffer");
      } else {
        T tmp(fwd<U>(Val));
        D3D11_SUBRESOURCE_DATA srd{&tmp};
        auto hr = main::system::d3d_device->CreateBuffer(&bd, &srd, buffer<T>::d3d_buffer.init());
        if (hr != S_OK) throw std::runtime_error("failed to create buffer");
      }
    } catch (const std::exception& e) {
      main::log(logger::error, e.what());
      if (hr != S_OK) main::log(logger::error, format_error(hr));
      main::log(logger::error, "failed to create constant buffer", _);
    }
  }
  template<convertible_to<T> U = T> bool from(U&& Val, const source& _ = {}) noexcept {
    HRESULT hr{};
    try {
      if (!(*this)) {
        if constexpr (included_in<U, T&, const T&>) {
          D3D11_SUBRESOURCE_DATA srd{&Val};
          hr = main::system::d3d_device->CreateBuffer(&bd, &srd, buffer<T>::d3d_buffer.init());
          if (hr != S_OK) throw std::runtime_error("failed to create buffer");
        } else {
          T tmp(fwd<U>(Val));
          D3D11_SUBRESOURCE_DATA srd{&tmp};
          hr = main::system::d3d_device->CreateBuffer(&bd, &srd, buffer<T>::d3d_buffer.init());
          if (hr != S_OK) throw std::runtime_error("failed to create buffer");
        }
      } else {
        D3D11_MAPPED_SUBRESOURCE msr{};
        hr = main::system::d3d_context->Map(*this, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
        if (hr != S_OK) throw std::runtime_error("failed to map buffer");
        if constexpr (different_from<U, T&, const T&>) {
          T tmp(fwd<U>(Val));
          memcpy(msr.pData, &tmp, sizeof(T));
        } else memcpy(msr.pData, &Val, sizeof(T));
        main::system::d3d_context->Unmap(*this, 0);
      }
    } catch (const std::exception& e) {
      main::log(logger::error, e.what());
      if (hr != S_OK) main::log(logger::error, format_error(hr));
      main::log(logger::error, "failed to update constant buffer", _);
      return false;
    }
  }
  template<invocable<T&> F> bool from(F&& Func, const source& _ = {}) noexcept {
    HRESULT hr{};
    try {
      D3D11_MAPPED_SUBRESOURCE msr{};
      hr = main::system::d3d_context->Map(*this, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
      if (hr != S_OK) throw std::runtime_error("failed to map buffer");
      Func(*reinterpret_cast<T*>(msr.pData));
      main::system::d3d_context->Unmap(*this, 0);
      return true;
    } catch (const std::exception& e) {
      main::log(logger::error, e.what());
      if (hr != S_OK) main::log(logger::error, format_error(hr));
      main::log(logger::error, "failed to update constant buffer", _);
      return false;
    }
  }
};
template<typename T> constant_buffer(const T&) -> constant_buffer<T>;

/// structured buffer class
template<typename T> class structured_buffer : public buffer<T> {
protected:
  comptr<ID3D11ShaderResourceView> d3d_srv{};
public:
  structured_buffer() noexcept = default;
  explicit operator bool() const noexcept { return buffer<T>::operator bool() && d3d_srv; }
  operator id3d11shaderresourceview*() const noexcept { return d3d_srv; }
  explicit structured_buffer(const buffer<T>& Src, const source& _ = {}) noexcept : structured_buffer(Src.count, _) { buffer<T>::from(Src); }
  explicit structured_buffer(numeric auto&& Count, const source& _ = {}) noexcept : buffer<T>(Count) {
    HRESULT hr{};
    try {
      D3D11_BUFFER_DESC bd{nat4(sizeof(T) * buffer<T>::count), D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE};
      bd.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED, bd.StructureByteStride = sizeof(T);
      hr = main::system::d3d_device->CreateBuffer(&bd, nullptr, buffer<T>::d3d_buffer.init());
      if (hr != S_OK) throw std::runtime_error("failed to create buffer");
      D3D11_SHADER_RESOURCE_VIEW_DESC srvd = {DXGI_FORMAT_UNKNOWN, D3D11_SRV_DIMENSION_BUFFER, D3D11_BUFFER_SRV{0, buffer<T>::count}};
      hr = main::system::d3d_device->CreateShaderResourceView(*this, &srvd, d3d_srv.init());
      if (hr != S_OK) throw std::runtime_error("failed to create shader resource view");
    } catch (const std::exception& e) {
      main::log(logger::error, e.what());
      if (hr != S_OK) main::log(logger::error, format_error(hr));
      main::log(logger::error, "failed to create structured buffer", _);
    }
  }
  structured_buffer(convertible_to<const T*> auto&& Data, numeric auto&& Count, const source& _ = {}) noexcept : buffer<T>(Count) {
    HRESULT hr{};
    try {
      D3D11_BUFFER_DESC bd{nat4(sizeof(T) * buffer<T>::count), D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE};
      bd.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED, bd.StructureByteStride = sizeof(T);
      const T* data = Data;
      D3D11_SUBRESOURCE_DATA srd{data, sizeof(T)};
      hr = main::system::d3d_device->CreateBuffer(&bd, &srd, buffer<T>::d3d_buffer.init());
      if (hr != S_OK) throw std::runtime_error("failed to create buffer");
      D3D11_SHADER_RESOURCE_VIEW_DESC srvd = {DXGI_FORMAT_UNKNOWN, D3D11_SRV_DIMENSION_BUFFER, D3D11_BUFFER_SRV{0, buffer<T>::count}};
      hr = main::system::d3d_device->CreateShaderResourceView(*this, &srvd, d3d_srv.init());
      if (hr != S_OK) throw std::runtime_error("failed to create shader resource view");
    } catch (const std::exception& e) {
      main::log(logger::error, e.what());
      if (hr != S_OK) main::log(logger::error, format_error(hr));
      main::log(logger::error, "failed to create structured buffer", _);
    }
  }
  template<contiguous_range Rg> requires same_as<iter_value<Rg>, T> structured_buffer(Rg&& Range, const source& _ = {}) noexcept //
    : structured_buffer(Range.data(), Range.size(), _) {}
  void from(convertible_to<const T*> auto&& Data, numeric auto&& Count, const source& _ = {}) noexcept {
    try {
      if (!(*this)) *this = structured_buffer(fwd<Data>, Count, _);
      else if (buffer<T>::count != nat4(Count)) throw std::runtime_error("the size of the source data must be the same as this buffer");
      else main::system::d3d_context->UpdateSubresource(*this, 0, nullptr, Data, 0, 0);
    } catch (const std::exception& e) {
      main::log(logger::error, e.what());
      main::log(logger::error, "failed to update structured buffer", _);
    }
  }
  template<contiguous_range Rg> requires same_as<iter_value<Rg>, T> //
  void from(Rg&& Range, const source& _ = {}) noexcept {
    from(Range.data(), Range.size(), _);
  }
};
template<typename T> structured_buffer(const buffer<T>&) -> structured_buffer<T>;
template<typename T> structured_buffer(const T*, numeric auto&&) -> structured_buffer<T>;
template<contiguous_range Rg> structured_buffer(Rg&&) -> structured_buffer<iter_value<Rg>>;

/// unordered access buffer class
template<typename T> class unordered_buffer : public buffer<T> {
protected:
  comptr<ID3D11UnorderedAccessView> d3d_uav{};
public:
  unordered_buffer() noexcept = default;
  explicit operator bool() const noexcept { return buffer<T>::operator bool() && d3d_uav; }
  operator id3d11unorderedaccessview*() const noexcept { return d3d_uav; }
  explicit unordered_buffer(const buffer<T>& Src, const source& _ = {}) noexcept : unordered_buffer(Src.count, _) { buffer<T>::from(Src); }
  explicit unordered_buffer(numeric auto&& Count, const source& _ = {}) noexcept : buffer<T>(Count) {
    HRESULT hr{};
    try {
      D3D11_BUFFER_DESC bd{nat4(sizeof(T) * buffer<T>::count), D3D11_USAGE_DEFAULT, D3D11_BIND_UNORDERED_ACCESS};
      bd.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED, bd.StructureByteStride = sizeof(T);
      hr = main::system::d3d_device->CreateBuffer(&bd, nullptr, buffer<T>::d3d_buffer.init());
      if (hr != S_OK) throw std::runtime_error("failed to create buffer");
      D3D11_UNORDERED_ACCESS_VIEW_DESC uavd = {DXGI_FORMAT_UNKNOWN, D3D11_UAV_DIMENSION_BUFFER, D3D11_BUFFER_UAV{0, buffer<T>::count}};
      hr = main::system::d3d_device->CreateUnorderedAccessView(*this, &uavd, d3d_uav.init());
      if (hr != S_OK) throw std::runtime_error("failed to create unordered access view");
    } catch (const std::exception& e) {
      main::log(logger::error, e.what());
      if (hr != S_OK) main::log(logger::error, format_error(hr));
      main::log(logger::error, "failed to create unordered access buffer", _);
    }
  }
  unordered_buffer(convertible_to<const T*> auto&& Data, numeric auto&& Count, const source& _ = {}) noexcept : buffer<T>(Count) {
    HRESULT hr{};
    try {
      D3D11_BUFFER_DESC bd{nat4(sizeof(T) * buffer<T>::count), D3D11_USAGE_DEFAULT, D3D11_BIND_UNORDERED_ACCESS};
      bd.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED, bd.StructureByteStride = sizeof(T);
      D3D11_SUBRESOURCE_DATA srd{static_cast<const T*>(Data), sizeof(T)};
      hr = main::system::d3d_device->CreateBuffer(&bd, &srd, buffer<T>::d3d_buffer.init());
      if (hr != S_OK) std::runtime_error("failed to create buffer");
      D3D11_UNORDERED_ACCESS_VIEW_DESC uavd = {DXGI_FORMAT_UNKNOWN, D3D11_UAV_DIMENSION_BUFFER, D3D11_BUFFER_UAV{0, buffer<T>::count}};
      hr = main::system::d3d_device->CreateUnorderedAccessView(*this, &uavd, d3d_uav.init());
      if (hr != S_OK) throw std::runtime_error("failed to create unordered access view");
    } catch (const std::exception& e) {
      main::log(logger::error, e.what());
      if (hr != S_OK) main::log(logger::error, format_error(hr));
      main::log(logger::error, "failed to create unordered access buffer", _);
    }
  }
  template<contiguous_range Rg> requires same_as<iter_value<Rg>, T> unordered_buffer(Rg&& Range, const source& _ = {}) noexcept //
    : unordered_buffer(Range.data(), Range.size(), _) {}
  void from(convertible_to<const T*> auto&& Data, numeric auto&& Count, const source& _ = {}) noexcept {
    try {
      if (!(*this)) *this = unordered_buffer(fwd<Data>, Count, _);
      else if (buffer<T>::count != nat4(Count)) throw std::runtime_error("the size of the source data must be the same as this buffer");
      else main::system::d3d_context->UpdateSubresource(*this, 0, nullptr, Data, 0, 0);
    } catch (const std::exception& e) {
      main::log(logger::error, e.what());
      main::log(logger::error, "failed to update unordered access buffer", _);
    }
  }
};
template<typename T> unordered_buffer(const buffer<T>&) -> unordered_buffer<T>;
template<typename T> unordered_buffer(const T*, numeric auto&&) -> unordered_buffer<T>;
template<contiguous_range Rg> unordered_buffer(Rg&&) -> unordered_buffer<iter_value<Rg>>;

/// primitive topologies
enum class topology : int {
  point_list = int(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST),
  line_list = int(D3D11_PRIMITIVE_TOPOLOGY_LINELIST),
  line_strip = int(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP),
  triangle_list = int(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST),
  triangle_strip = int(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP),
};

/// rendering shader
template<specialization_of<typepack> VSResources, specialization_of<typepack> VSConstants,                           //
         specialization_of<typepack> GSResources = typepack<>, specialization_of<typepack> GSConstants = typepack<>, //
         specialization_of<typepack> PSResources = typepack<>, specialization_of<typepack> PSConstants = typepack<>>
class rendering_shader {
  bool render(const array<ID3D11ShaderResourceView*, VSResources::count>& vsr, //
              const array<ID3D11Buffer*, VSConstants::count>& vsc,             //
              const array<ID3D11ShaderResourceView*, GSResources::count>& gsr, //
              const array<ID3D11Buffer*, GSConstants::count>& gsc,             //
              const array<ID3D11ShaderResourceView*, PSResources::count>& psr, //
              const array<ID3D11Buffer*, PSConstants::count>& psc,             //
              nat4 VertexCounts, D3D11_PRIMITIVE_TOPOLOGY Topology, const source& _ = {}) noexcept {
    try {
      if (!(*this)) throw std::runtime_error("rendering shader is not valid");
      main::system::d3d_context->VSSetShaderResources(0, vsr.size(), vsr.data());
      main::system::d3d_context->GSSetShaderResources(0, gsr.size(), gsr.data());
      main::system::d3d_context->PSSetShaderResources(0, psr.size(), psr.data());
      main::system::d3d_context->VSSetConstantBuffers(0, vsc.size(), vsc.data());
      main::system::d3d_context->GSSetConstantBuffers(0, gsc.size(), gsc.data());
      main::system::d3d_context->PSSetConstantBuffers(0, psc.size(), psc.data());
      main::system::d3d_context->IASetPrimitiveTopology(Topology);
      main::system::d3d_context->IASetInputLayout(nullptr);
      main::system::d3d_context->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
      main::system::d3d_context->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
      main::system::d3d_context->VSSetShader(d3d_vs, 0, 0);
      main::system::d3d_context->GSSetShader(d3d_gs, 0, 0);
      main::system::d3d_context->PSSetShader(d3d_ps, 0, 0);
      main::system::d3d_context->Draw(VertexCounts, 0);
      return true;
    } catch (const std::exception& e) {
      main::log(logger::error, e.what());
      main::log(logger::error, "failed to render", _);
      return false;
    }
  }
protected:
  comptr<ID3D11VertexShader> d3d_vs{};
  comptr<ID3D11GeometryShader> d3d_gs{};
  comptr<ID3D11PixelShader> d3d_ps{};
public:
  using vs_resource_list = list<>::from_typepack<VSResources, const int&>;
  using gs_resource_list = list<>::from_typepack<GSResources, const int&>;
  using ps_resource_list = list<>::from_typepack<PSResources, const int&>;
  using vs_constant_list = list<>::from_typepack<VSConstants, const int&>;
  using gs_constant_list = list<>::from_typepack<GSConstants, const int&>;
  using ps_constant_list = list<>::from_typepack<PSConstants, const int&>;
  yw::topology topology = yw::topology::triangle_list;
  rendering_shader() noexcept = default;
  explicit operator bool() const noexcept { return bool(d3d_vs) && bool(d3d_ps); }
  rendering_shader(                                                      //
    const string_view<cat1> HLSL, format_string<cat1> VSMAIN = "vsmain", //
    format_string<cat1> GSMAIN = "gsmain", format_string<cat1> PSMAIN = "psmain", const source& _ = {}) noexcept {
    HRESULT hr{};
    comptr<ID3DBlob> b{}, c{};
    try {
      hr = ::D3DCompile(HLSL.data(), HLSL.size(), 0, 0, 0, VSMAIN.get().c_str(), "vs_5_0", (1 << 11), 0, b.init(), c.init());
      if (hr != S_OK) throw std::runtime_error("failed to compile vertex shader");
      b.reset(), c.reset();
      hr = ::D3DCompile(HLSL.data(), HLSL.size(), 0, 0, 0, PSMAIN.get().c_str(), "ps_5_0", (1 << 11), 0, b.init(), c.init());
      if (hr != S_OK) throw std::runtime_error("failed to compile pixel shader");
      if (HLSL.find(GSMAIN.get()) == npos) return;
      b.reset(), c.reset();
      hr = ::D3DCompile(HLSL.data(), HLSL.size(), 0, 0, 0, GSMAIN.get().c_str(), "gs_5_0", (1 << 11), 0, b.init(), c.init());
      if (hr != S_OK) throw std::runtime_error("failed to compile geometry shader");
    } catch (const std::exception& e) {
      main::log(logger::error, e.what());
      if (hr != S_OK) main::log(logger::error, format_error(hr));
      if (c->GetBufferSize() != 0) main::log(logger::error, reinterpret_cast<const char*>(c->GetBufferPointer()));
      main::log(logger::error, "failed to create rendering shader", _);
    }
  }
  bool operator()(                                                                                      //
    numeric auto&& VertexCounts, vs_resource_list vsr, vs_constant_list vsc, gs_resource_list gsr = {}, //
    gs_constant_list gsc = {}, ps_resource_list psr = {}, ps_constant_list psc = {}, const source& _ = {}) noexcept {
    array<ID3D11ShaderResourceView*, VSResources::count> vsr_;
    array<ID3D11Buffer*, VSConstants::count> vsc_;
    array<ID3D11ShaderResourceView*, GSResources::count> gsr_;
    array<ID3D11Buffer*, GSConstants::count> gsc_;
    array<ID3D11ShaderResourceView*, PSResources::count> psr_;
    array<ID3D11Buffer*, PSConstants::count> psc_;
    if constexpr (vsr_.count != 0) vassign(vsr_, vsr);
    if constexpr (vsc_.count != 0) vassign(vsc_, vsc);
    if constexpr (gsr_.count != 0) vassign(gsr_, gsr);
    if constexpr (gsc_.count != 0) vassign(gsc_, gsc);
    if constexpr (psr_.count != 0) vassign(psr_, psr);
    if constexpr (psc_.count != 0) vassign(psc_, psc);
    return render(vsr_, vsc_, gsr_, gsc_, psr_, psc_, _, nat4(VertexCounts), bitcast<D3D11_PRIMITIVE_TOPOLOGY>(topology), _);
  }
};

/// performs GPGPU calculation
template<specialization_of<typepack> Unordered,               //
         specialization_of<typepack> Structured = typepack<>, //
         specialization_of<typepack> Constant = typepack<>>
class computing_shader {
  bool call(const array<id3d11unorderedaccessview*, Unordered::count>& u, //
            const array<id3d11shaderresourceview*, Structured::count>& s, //
            const array<id3d11buffer*, Constant::count>& c,               //
            nat4 x, nat4 y, nat4 z) const {
    if (!(x && y && z)) return false;
    try {
      main::system::d3d_context->CSSetUnorderedAccessViews(0, u.count, u.data(), nullptr);
      main::system::d3d_context->CSSetShaderResources(0, s.count, s.data());
      main::system::d3d_context->CSSetConstantBuffers(0, c.count, c.data());
      main::system::d3d_context->CSSetShader(d3d_cs, 0, 0);
      main::system::d3d_context->Dispatch(x, y, z);
      return true;
    } catch (const std::exception& e) {
      main::log(logger::error, e.what());
      main::log(logger::error, "failed to call computing shader");
      return false;
    }
  }
protected:
  comptr<ID3D11ComputeShader> d3d_cs{};
public:
  using ub_list = typename list<>::template from_typepack<Unordered, const int&>;
  using sb_list = typename list<>::template from_typepack<Structured, const int&>;
  using cb_list = typename list<>::template from_typepack<Constant, const int&>;
  computing_shader() noexcept = default;
  explicit operator bool() const noexcept { return bool(d3d_cs); }
  computing_shader(const string_view<cat1> HLSL, format_string<cat1> CSMain = "csmain", const source& _ = {}) noexcept {
    HRESULT hr{};
    comptr<ID3DBlob> b{}, c{};
    try {
      hr = ::D3DCompile(HLSL.data(), HLSL.size(), 0, 0, 0, CSMain.get().data(), "cs_5_0", (1 << 11), 0, b.init(), c.init());
      if (hr != S_OK) throw std::runtime_error("failed to compile compute shader");
    } catch (const std::exception& e) {
      main::log(logger::error, e.what());
      if (hr != S_OK) main::log(logger::error, format_error(hr));
      if (c->GetBufferSize() != 0) main::log(logger::error, reinterpret_cast<const char*>(c->GetBufferPointer()));
      main::log(logger::error, "failed to create computing shader", _);
    }
  }
  /// performs GPGPU calculation
  /// \param x, y, z: number of threads in each dimension; must be `(1, 1, 1)` or greater
  bool operator()(ub_list ub, sb_list sb, cb_list cb, numeric auto&& x, //
                  numeric auto&& y, numeric auto&& z, const source& _ = {}) const noexcept {
    try {
      if (!(*this)) throw std::runtime_error("the computing shader is not valid");
      array<id3d11unorderedaccessview*, Unordered::count> ub_{};
      array<id3d11shaderresourceview*, Structured::count> sb_{};
      array<id3d11buffer*, Constant::count> cb_{};
      // if constexpr (ub_.count != 0) vassign(ub_, ub);
      get<0>(ub_) = ub[0];
      if constexpr (sb_.count != 0) vassign(sb_, sb);
      if constexpr (cb_.count != 0) vassign(cb_, cb);
      return call(ub_, sb_, cb_, nat4(x), nat4(y), nat4(z));
    } catch (const std::exception& e) {
      main::log(logger::error, e.what());
      main::log(logger::error, "failed to perform GPGPU calculation", _);
      return false;
    }
  }
  /// performs GPGPU calculation; `numthreads` must be `(1024, 1, 1)`
  bool operator()(numeric auto&& Parallels, ub_list ub, sb_list sb, cb_list cb, const source& _ = {}) const noexcept { //
    return operator()(mv(ub), mv(sb), mv(cb), (nat4(Parallels) - 1_n4) / 1024_n4, 1, 1, _);
  }
};

} // namespace yw
