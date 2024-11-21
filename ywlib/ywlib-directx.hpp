#pragma once

//
#ifndef ywlib_com_init
#define ywlib_com_init COINIT_APARTMENTTHREADED
#endif

#include "ywlib-std.hpp"
#include "ywlib-windows.hpp"
#include "ywlib-xv.hpp"

#include <d2d1_3.h>
#include <d3d11_3.h>
#include <d3dcompiler.h>
#include <dwrite_3.h>
#include <dxgi1_6.h>
#include <wincodec.h>
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "windowscodecs.lib")

export namespace yw {

using ID3D11Buffer = ::ID3D11Buffer;
using ID3D11ShaderResourceView = ::ID3D11ShaderResourceView;
using ID3D11UnorderedAccessView = ::ID3D11UnorderedAccessView;

/// \brief deleter for COM objects
template<typename Com> struct com_deleter {
  constexpr com_deleter() noexcept = default;
  template<typename U> constexpr com_deleter(const com_deleter<U>&) noexcept {}
  constexpr void operator()(Com* p) const noexcept { p ? void(p->Release()) : void(); }
};

/// \brief COM smart pointer
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

// `comptr<Com>` must be the same size as `Com*`
static_assert(sizeof(comptr<IUnknown>) == sizeof(IUnknown*));

namespace main::system {

/// \brief class for initializing and unintializing COM
class com_init {
  bool _initialized{};
public:
  explicit operator bool() const noexcept { return _initialized; }
  ~com_init() noexcept { _initialized ? ::CoUninitialize() : void(); }
  com_init(COINIT coinit) noexcept {
    if (auto r = ::CoInitializeEx(0, coinit); r == S_OK) _initialized = true;
    else if (r != S_FALSE) void(); // already initialized
    else main::log.format(logger::critical, "failed to initialize COM: {}", format_error(r));
  }
};

/// \brief checks if COM is initialized
inline const com_init com_initialized{ywlib_com_init};

comptr<ID3D11Device> d3d_device{};         // D3D11 device; Do not access directly
comptr<ID3D11DeviceContext> d3d_context{}; // D3D11 device context; Do not access directly
comptr<IDXGIFactory2> dxgi_factory{};      // DXGI factory; Do not access directly
comptr<IDXGIDevice2> dxgi_device{};        // DXGI device; Do not access directly
comptr<ID2D1Factory1> d2d_factory{};       // D2D1 factory; Do not access directly
comptr<ID2D1Device> d2d_device{};          // D2D1 device; Do not access directly
comptr<ID2D1DeviceContext> d2d_context{};  // D2D1 device context; Do not access directly
comptr<IDWriteFactory1> dwrite_factory{};  // DWrite factory; Do not access directly
comptr<IWICImagingFactory> wic_factory{};  // WIC imaging factory; Do not access directly

comptr<ID3D11BlendState> d3d_blend_state{};           // default D3D11 blend state; Do not access directly
comptr<ID3D11SamplerState> d3d_sampler_state[1]{};    // default D3D11 sampler state; Do not access directly
comptr<ID3D11RasterizerState> d3d_rasterizer_state{}; // default D3D11 rasterizer state; Do not access directly

inline const bool directx_initialized = [](comptr<ID3D11Device> a) {
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
    hr = d3d_device->CreateSamplerState(&sampler_desc, d3d_sampler_state[0].init());
    if (hr != S_OK) throw std::runtime_error("failed to create D3D11 sampler state");
    else d3d_context->PSSetSamplers(0, 1, reinterpret_cast<ID3D11SamplerState**>(d3d_sampler_state));
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
    if (hr != S_OK) main::log(logger::error, format_error(hr));
    main::log(logger::error, e.what());
    main::log(logger::critical, "failed to initialize DirectX system");
    return false;
  }
}({});
}

namespace main {

/// \brief D3D11 device
inline const comptr<ID3D11Device>& d3d_device = system::d3d_device;
/// \brief D3D11 device context
inline const comptr<ID3D11DeviceContext>& d3d_context = system::d3d_context;
/// \brief DXGI factory
inline const comptr<IDXGIFactory2>& dxgi_factory = system::dxgi_factory;
/// \brief DXGI device
inline const comptr<IDXGIDevice2>& dxgi_device = system::dxgi_device;
/// \brief D2D1 factory
inline const comptr<ID2D1Factory1>& d2d_factory = system::d2d_factory;
/// \brief D2D1 device
inline const comptr<ID2D1Device>& d2d_device = system::d2d_device;
/// \brief D2D1 device context
inline const comptr<ID2D1DeviceContext>& d2d_context = system::d2d_context;
/// \brief DWrite factory
inline const comptr<IDWriteFactory1>& dwrite_factory = system::dwrite_factory;
/// \brief WIC imaging factory
inline const comptr<IWICImagingFactory>& wic_factory = system::wic_factory;
}

/// \brief primitive topologies
enum class topology : int {
  point_list = int(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST),
  line_list = int(D3D11_PRIMITIVE_TOPOLOGY_LINELIST),
  line_strip = int(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP),
  triangle_list = int(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST),
  triangle_strip = int(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP),
};

template<typename T> class staging_buffer;

/// @brief base class for GPU buffers
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
  operator add_pointer<ID3D11Buffer>() const noexcept { return d3d_buffer; }
  explicit operator bool() const noexcept { return bool(d3d_buffer); }

  buffer& operator=(buffer&& B) noexcept {
    d3d_buffer = mv(B.d3d_buffer);
    const_cast<nat4&>(count) = B.count;
    return *this;
  }

  bool from(const buffer& Src, const source& _ = {}) noexcept {
    try {
      if (!Src) throw std::runtime_error("the source buffer is not valid");
      if (count != Src.count) throw std::runtime_error("the size of the source buffer must be the same as this buffer");
      main::d3d_context->CopyResource(*this, Src);
      return true;
    } catch (const std::exception& e) {
      main::log(logger::error, e.what());
      main::log(logger::error, "failed to copy buffer", _);
      return false;
    }
  }

  std::vector<T> to_cpu() const noexcept;
  std::vector<T> to_cpu(staging_buffer<T>& Staging) const noexcept;
};

/// \brief staging buffer class for copying data to CPU
template<typename T> class staging_buffer : public buffer<T> {
public:
  staging_buffer() noexcept = default;
  explicit staging_buffer(const buffer<T>& Src) : staging_buffer(Src.count) { buffer<T>::from(Src); }
  operator ::ID3D11Buffer*() const noexcept { return buffer<T>::d3d_buffer; }

  /// \brief creates a staging buffer with the specified size
  explicit staging_buffer(numeric auto&& Count) noexcept : buffer<T>(Count) {
    try {
      D3D11_BUFFER_DESC bd{nat4(sizeof(T) * buffer<T>::count), D3D11_USAGE_STAGING, 0, D3D11_CPU_ACCESS_READ};
      if (auto hr = main::d3d_device->CreateBuffer(&bd, nullptr, buffer<T>::d3d_buffer.init()); hr != S_OK) {
        if (hr != S_OK) main::log(logger::error, format_error(hr));
        main::log(logger::error, "failed to create staging buffer");
      }
    } catch (const std::exception& e) {
      main::log(logger::error, e.what());
      main::log(logger::error, "failed to create staging buffer");
    }
  }

  /// \brief copies the data to CPU
  array<T> to_cpu(const source& _ = {}) const noexcept {
    try {
      if (!(*this)) throw std::runtime_error("the staging buffer is not valid");
      array<T> data(buffer<T>::count);
      D3D11_MAPPED_SUBRESOURCE msr{};
      if (auto hr = main::d3d_context->Map(*this, 0, D3D11_MAP_READ, 0, &msr); hr != S_OK) {
        if (hr != S_OK) main::log(logger::error, format_error(hr));
        main::log(logger::error, "failed to map staging buffer", _);
        return {};
      }
      memcpy(data.data(), msr.pData, sizeof(T) * buffer<T>::count);
      main::d3d_context->Unmap(*this, 0);
      return data;
    } catch (const std::exception& e) {
      main::log(logger::error, e.what());
      main::log(logger::error, "failed to copy buffer to CPU", _);
      return {};
    }
  }
};

template<typename T> staging_buffer(const buffer<T>&) -> staging_buffer<T>;

template<typename T> std::vector<T> buffer<T>::to_cpu() const noexcept {
  staging_buffer<T> staging(*this);
  return staging.to_cpu();
}

template<typename T> std::vector<T> buffer<T>::to_cpu(staging_buffer<T>& Staging) const noexcept {
  Staging.from(*this);
  return Staging.to_cpu();
}

/// \brief constant buffer class
template<typename T> requires ((sizeof(T) % 16) == 0) class constant_buffer : public buffer<T> {
  static constexpr D3D11_BUFFER_DESC bd = {sizeof(T), D3D11_USAGE(2), D3D11_BIND_FLAG(4), D3D11_CPU_ACCESS_WRITE};
public:
  using buffer<T>::operator ID3D11Buffer*;
  constant_buffer() noexcept = default;

  /// \brief creates a constant buffer and loads the specified value
  template<convertible_to<T> U = T> //
  explicit constant_buffer(U&& Val, const source& _ = {}) noexcept : buffer<T>(1) {
    HRESULT hr{};
    try {
      if constexpr (included_in<U, T&, const T&>) {
        D3D11_SUBRESOURCE_DATA srd{&Val};
        auto hr = main::d3d_device->CreateBuffer(&bd, &srd, buffer<T>::d3d_buffer.init());
        if (hr != S_OK) throw std::runtime_error("failed to create buffer");
      } else {
        T tmp(fwd<U>(Val));
        D3D11_SUBRESOURCE_DATA srd{&tmp};
        auto hr = main::d3d_device->CreateBuffer(&bd, &srd, buffer<T>::d3d_buffer.init());
        if (hr != S_OK) throw std::runtime_error("failed to create buffer");
      }
    } catch (const std::exception& e) {
      main::log(logger::error, e.what());
      if (hr != S_OK) main::log(logger::error, format_error(hr));
      main::log(logger::error, "failed to create constant buffer", _);
    }
  }

  /// \brief updates the buffer with the specified value
  template<convertible_to<T> U = T> //
  bool from(U&& Val, const source& _ = {}) noexcept {
    HRESULT hr{};
    try {
      if (!(*this)) { // need to initialize the buffer
        if constexpr (included_in<U, T&, const T&>) {
          D3D11_SUBRESOURCE_DATA srd{&Val};
          hr = main::d3d_device->CreateBuffer(&bd, &srd, buffer<T>::d3d_buffer.init());
          if (hr != S_OK) throw std::runtime_error("failed to create buffer");
        } else {
          T tmp(fwd<U>(Val));
          D3D11_SUBRESOURCE_DATA srd{&tmp};
          hr = main::d3d_device->CreateBuffer(&bd, &srd, buffer<T>::d3d_buffer.init());
          if (hr != S_OK) throw std::runtime_error("failed to create buffer");
        }
      } else { // buffer already initialized
        D3D11_MAPPED_SUBRESOURCE msr{};
        hr = main::d3d_context->Map(*this, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
        if (hr != S_OK) throw std::runtime_error("failed to map buffer");
        if constexpr (different_from<U, T&, const T&>) {
          T tmp(fwd<U>(Val));
          memcpy(msr.pData, &tmp, sizeof(T));
        } else memcpy(msr.pData, &Val, sizeof(T));
        main::d3d_context->Unmap(*this, 0);
      }
    } catch (const std::exception& e) {
      main::log(logger::error, e.what());
      if (hr != S_OK) main::log(logger::error, format_error(hr));
      main::log(logger::error, "failed to update constant buffer", _);
      return false;
    }
  }

  /// \brief updates the buffer through the specified function
  template<invocable<T&> F> bool from(F&& Func) noexcept {
    HRESULT hr{};
    try {
      D3D11_MAPPED_SUBRESOURCE msr{};
      hr = main::d3d_context->Map(*this, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
      if (hr != S_OK) throw std::runtime_error("failed to map buffer");
      Func(*reinterpret_cast<T*>(msr.pData));
      main::d3d_context->Unmap(*this, 0);
      return true;
    } catch (const std::exception& e) {
      main::log(logger::error, e.what());
      if (hr != S_OK) main::log(logger::error, format_error(hr));
      main::log(logger::error, "failed to update constant buffer");
      return false;
    }
  }
};

template<typename T> constant_buffer(const T&) -> constant_buffer<T>;

/// \brief structured buffer class
template<typename T> class structured_buffer : public buffer<T> {
protected:
  comptr<ID3D11ShaderResourceView> d3d_srv{};
public:
  using buffer<T>::operator ID3D11Buffer*;
  structured_buffer() noexcept = default;
  operator ::ID3D11ShaderResourceView*() const noexcept { return d3d_srv; }
  explicit operator bool() const noexcept { return buffer<T>::operator bool() && d3d_srv; }
  explicit structured_buffer(const buffer<T>& Src) noexcept : structured_buffer(Src.count) { buffer<T>::from(Src); }

  /// \brief creates a structured buffer with the specified size
  explicit structured_buffer(numeric auto&& Count, const source& _ = {}) noexcept : buffer<T>(Count) {
    HRESULT hr{};
    try {
      D3D11_BUFFER_DESC bd{nat4(sizeof(T) * buffer<T>::count), D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE};
      bd.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED, bd.StructureByteStride = sizeof(T);
      hr = main::d3d_device->CreateBuffer(&bd, nullptr, buffer<T>::d3d_buffer.init());
      if (hr != S_OK) throw std::runtime_error("failed to create buffer");
      D3D11_SHADER_RESOURCE_VIEW_DESC srvd = //
        {DXGI_FORMAT_UNKNOWN, D3D11_SRV_DIMENSION_BUFFER, D3D11_BUFFER_SRV{0, buffer<T>::count}};
      hr = main::d3d_device->CreateShaderResourceView(*this, &srvd, d3d_srv.init());
      if (hr != S_OK) throw std::runtime_error("failed to create shader resource view");
    } catch (const std::exception& e) {
      main::log(logger::error, e.what());
      if (hr != S_OK) main::log(logger::error, format_error(hr));
      main::log(logger::error, "failed to create structured buffer", _);
    }
  }

  /// \brief creates a structured buffer from CPU data
  structured_buffer(convertible_to<const T*> auto&& Data, numeric auto&& Count, const source& _ = {}) noexcept : buffer<T>(Count) {
    HRESULT hr{};
    try {
      D3D11_BUFFER_DESC bd{nat4(sizeof(T) * buffer<T>::count), D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE};
      bd.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED, bd.StructureByteStride = sizeof(T);
      const T* data = Data;
      D3D11_SUBRESOURCE_DATA srd{data, sizeof(T)};
      hr = main::d3d_device->CreateBuffer(&bd, &srd, buffer<T>::d3d_buffer.init());
      if (hr != S_OK) throw std::runtime_error("failed to create buffer");
      D3D11_SHADER_RESOURCE_VIEW_DESC srvd = //
        {DXGI_FORMAT_UNKNOWN, D3D11_SRV_DIMENSION_BUFFER, D3D11_BUFFER_SRV{0, buffer<T>::count}};
      hr = main::d3d_device->CreateShaderResourceView(*this, &srvd, d3d_srv.init());
      if (hr != S_OK) throw std::runtime_error("failed to create shader resource view");
    } catch (const std::exception& e) {
      main::log(logger::error, e.what());
      if (hr != S_OK) main::log(logger::error, format_error(hr));
      main::log(logger::error, "failed to create structured buffer", _);
    }
  }

  /// \brief creates a structured buffer from CPU data
  template<contiguous_range Rg> requires same_as<iter_value<Rg>, T> //
  structured_buffer(Rg&& Range, const source& _ = {}) noexcept      //
    : structured_buffer(Range.data(), Range.size(), _) {}

  /// \brief updates the buffer with the specified data
  void from(convertible_to<const T*> auto&& Data, numeric auto&& Count, const source& _ = {}) noexcept {
    try {
      if (!(*this)) *this = structured_buffer(fwd<Data>, Count, _);
      else if (buffer<T>::count != nat4(Count)) //
        throw std::runtime_error("the size of the source data must be the same as this buffer");
      else main::d3d_context->UpdateSubresource(*this, 0, nullptr, Data, 0, 0);
    } catch (const std::exception& e) {
      main::log(logger::error, e.what());
      main::log(logger::error, "failed to update structured buffer", _);
    }
  }

  /// \brief updates the buffer with the specified data
  template<contiguous_range Rg> requires same_as<iter_value<Rg>, T> //
  void from(Rg&& Range, const source& _ = {}) noexcept {
    from(Range.data(), Range.size(), _);
  }
};

template<typename T> structured_buffer(const buffer<T>&) -> structured_buffer<T>;
template<typename T> structured_buffer(const T*, numeric auto&&) -> structured_buffer<T>;
template<contiguous_range Rg> structured_buffer(Rg&&) -> structured_buffer<iter_value<Rg>>;

/// \brief unordered access buffer class
template<typename T> class unordered_buffer : public buffer<T> {
protected:
  comptr<ID3D11UnorderedAccessView> d3d_uav{};
public:
  unordered_buffer() noexcept = default;
  operator ID3D11Buffer*() const noexcept { return buffer<T>::d3d_buffer; }
  operator ID3D11UnorderedAccessView*() const noexcept { return d3d_uav; }
  explicit operator bool() const noexcept { return buffer<T>::operator bool() && d3d_uav; }
  explicit unordered_buffer(const buffer<T>& Src) noexcept : unordered_buffer(Src.count) { buffer<T>::from(Src); }

  /// \brief creates an unordered access buffer with the specified size
  explicit unordered_buffer(numeric auto&& Count, const source& _ = {}) noexcept : buffer<T>(Count) {
    HRESULT hr{};
    try {
      D3D11_BUFFER_DESC bd{nat4(sizeof(T) * buffer<T>::count), D3D11_USAGE_DEFAULT, D3D11_BIND_UNORDERED_ACCESS};
      bd.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED, bd.StructureByteStride = sizeof(T);
      hr = main::d3d_device->CreateBuffer(&bd, nullptr, buffer<T>::d3d_buffer.init());
      if (hr != S_OK) throw std::runtime_error("failed to create buffer");
      D3D11_UNORDERED_ACCESS_VIEW_DESC uavd = //
        {DXGI_FORMAT_UNKNOWN, D3D11_UAV_DIMENSION_BUFFER, D3D11_BUFFER_UAV{0, buffer<T>::count}};
      hr = main::d3d_device->CreateUnorderedAccessView(*this, &uavd, d3d_uav.init());
      if (hr != S_OK) throw std::runtime_error("failed to create unordered access view");
    } catch (const std::exception& e) {
      main::log(logger::error, e.what());
      if (hr != S_OK) main::log(logger::error, format_error(hr));
      main::log(logger::error, "failed to create unordered access buffer", _);
    }
  }

  /// \brief creates an unordered access buffer from CPU data
  unordered_buffer(convertible_to<const T*> auto&& Data, numeric auto&& Count, const source& _ = {}) noexcept : buffer<T>(Count) {
    HRESULT hr{};
    try {
      D3D11_BUFFER_DESC bd{nat4(sizeof(T) * buffer<T>::count), D3D11_USAGE_DEFAULT, D3D11_BIND_UNORDERED_ACCESS};
      bd.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED, bd.StructureByteStride = sizeof(T);
      D3D11_SUBRESOURCE_DATA srd{static_cast<const T*>(Data), sizeof(T)};
      hr = main::d3d_device->CreateBuffer(&bd, &srd, buffer<T>::d3d_buffer.init());
      if (hr != S_OK) std::runtime_error("failed to create buffer");
      D3D11_UNORDERED_ACCESS_VIEW_DESC uavd = //
        {DXGI_FORMAT_UNKNOWN, D3D11_UAV_DIMENSION_BUFFER, D3D11_BUFFER_UAV{0, buffer<T>::count}};
      hr = main::d3d_device->CreateUnorderedAccessView(*this, &uavd, d3d_uav.init());
      if (hr != S_OK) throw std::runtime_error("failed to create unordered access view");
    } catch (const std::exception& e) {
      main::log(logger::error, e.what());
      if (hr != S_OK) main::log(logger::error, format_error(hr));
      main::log(logger::error, "failed to create unordered access buffer", _);
    }
  }

  /// \brief creates an unordered access buffer from CPU data
  template<contiguous_range Rg> requires same_as<iter_value<Rg>, T> //
  unordered_buffer(Rg&& Range, const source& _ = {}) noexcept : unordered_buffer(Range.data(), Range.size(), _) {}

  /// \brief updates the buffer with the specified data
  void from(convertible_to<const T*> auto&& Data, numeric auto&& Count, const source& _ = {}) noexcept {
    try {
      if (!(*this)) *this = unordered_buffer(fwd<Data>, Count, _);
      else if (buffer<T>::count != nat4(Count)) //
        throw std::runtime_error("the size of the source data must be the same as this buffer");
      else main::d3d_context->UpdateSubresource(*this, 0, nullptr, Data, 0, 0);
    } catch (const std::exception& e) {
      main::log(logger::error, e.what());
      main::log(logger::error, "failed to update unordered access buffer", _);
    }
  }
};

template<typename T> unordered_buffer(const buffer<T>&) -> unordered_buffer<T>;
template<typename T> unordered_buffer(const T*, numeric auto&&) -> unordered_buffer<T>;
template<contiguous_range Rg> unordered_buffer(Rg&&) -> unordered_buffer<iter_value<Rg>>;

/// \brief rendering shader class
template<specialization_of<typepack> VSResources, specialization_of<typepack> VSConstants,                           //
         specialization_of<typepack> GSResources = typepack<>, specialization_of<typepack> GSConstants = typepack<>, //
         specialization_of<typepack> PSResources = typepack<>, specialization_of<typepack> PSConstants = typepack<>>
class rendering_shader {
protected:
  comptr<ID3D11VertexShader> d3d_vs{};
  comptr<ID3D11GeometryShader> d3d_gs{};
  comptr<ID3D11PixelShader> d3d_ps{};
public:
  using vs_resource_list = array<ID3D11ShaderResourceView*, VSResources::size>;
  using gs_resource_list = array<ID3D11ShaderResourceView*, GSResources::size>;
  using ps_resource_list = array<ID3D11ShaderResourceView*, PSResources::size>;
  using vs_constant_list = array<ID3D11Buffer*, VSConstants::size>;
  using gs_constant_list = array<ID3D11Buffer*, GSConstants::size>;
  using ps_constant_list = array<ID3D11Buffer*, PSConstants::size>;
  yw::topology topology = yw::topology::triangle_list;
  rendering_shader() noexcept = default;
  explicit operator bool() const noexcept { return bool(d3d_vs) && bool(d3d_ps); }

  /// \brief compiles shaders
  rendering_shader(                                                      //
    const string_view<cat1> HLSL, format_string<cat1> VSMAIN = "vsmain", //
    format_string<cat1> GSMAIN = "gsmain", format_string<cat1> PSMAIN = "psmain", const source& _ = {}) noexcept {
    HRESULT hr{};
    comptr<ID3DBlob> b{}, c{};
    try {
      // compile vertex shader
      hr = ::D3DCompile(HLSL.data(), HLSL.size(), 0, 0, 0, VSMAIN.get().c_str(), //
                        "vs_5_0", (1 << 11), 0, b.init(), c.init());
      if (hr != S_OK) throw std::runtime_error("failed to compile vertex shader");
      // compile pixel shader
      b.reset(), c.reset();
      hr = ::D3DCompile(HLSL.data(), HLSL.size(), 0, 0, 0, PSMAIN.get().c_str(), //
                        "ps_5_0", (1 << 11), 0, b.init(), c.init());
      if (hr != S_OK) throw std::runtime_error("failed to compile pixel shader");
      // compile geometry shader if `GSMAIN` is found in HLSL
      if (HLSL.find(GSMAIN.get()) == npos) return;
      b.reset(), c.reset();
      hr = ::D3DCompile(HLSL.data(), HLSL.size(), 0, 0, 0, GSMAIN.get().c_str(), //
                        "gs_5_0", (1 << 11), 0, b.init(), c.init());
      if (hr != S_OK) throw std::runtime_error("failed to compile geometry shader");
    } catch (const std::exception& e) {
      main::log(logger::error, e.what());
      if (hr != S_OK) main::log(logger::error, format_error(hr));
      if (c->GetBufferSize() != 0) main::log(logger::error, reinterpret_cast<const char*>(c->GetBufferPointer()));
      main::log(logger::error, "failed to create rendering shader", _);
    }
  }

  /// \brief performs 3D rendering
  bool operator()(                                                                                      //
    numeric auto&& VertexCounts, vs_resource_list vsr, vs_constant_list vsc, gs_resource_list gsr = {}, //
    gs_constant_list gsc = {}, ps_resource_list psr = {}, ps_constant_list psc = {}, const source& _ = {}) noexcept {
    try {
      if (!(*this)) throw std::runtime_error("rendering shader is not valid");
      if constexpr (vs_resource_list::count != 0) main::d3d_context->VSSetShaderResources(0, vsr.count, vsr.data());
      if constexpr (gs_resource_list::count != 0) main::d3d_context->GSSetShaderResources(0, gsr.count, gsr.data());
      if constexpr (ps_resource_list::count != 0) main::d3d_context->PSSetShaderResources(0, psr.count, psr.data());
      if constexpr (vs_constant_list::count != 0) main::d3d_context->VSSetConstantBuffers(0, vsc.count, vsc.data());
      if constexpr (gs_constant_list::count != 0) main::d3d_context->GSSetConstantBuffers(0, gsc.count, gsc.data());
      if constexpr (ps_constant_list::count != 0) main::d3d_context->PSSetConstantBuffers(0, psc.count, psc.data());
      main::d3d_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY(topology));
      main::d3d_context->IASetInputLayout(nullptr);
      main::d3d_context->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
      main::d3d_context->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
      main::d3d_context->VSSetShader(d3d_vs, 0, 0);
      main::d3d_context->GSSetShader(d3d_gs, 0, 0);
      main::d3d_context->PSSetShader(d3d_ps, 0, 0);
      main::d3d_context->Draw(nat4(VertexCounts), 0);
      return true;
    } catch (const std::exception& e) {
      main::log(logger::error, e.what());
      main::log(logger::error, "failed to render", _);
      return false;
    }
  }
};

template<specialization_of<typepack> Unordered,               //
         specialization_of<typepack> Structured = typepack<>, //
         specialization_of<typepack> Constant = typepack<>>
class computing_shader {
  static_assert(Unordered::count > 0, "unordered access views must be specified");
  static_assert([]<typename... Ts>(typepack<Ts...>) { return (convertible_to<Ts, ID3D11UnorderedAccessView*> && ...); }(Unordered{}));
  static_assert([]<typename... Ts>(typepack<Ts...>) { return (convertible_to<Ts, ID3D11ShaderResourceView*> && ...); }(Structured{}));
  static_assert([]<typename... Ts>(typepack<Ts...>) { return (convertible_to<Ts, ID3D11Buffer*> && ...); }(Constant{}));
protected:
  comptr<ID3D11ComputeShader> d3d_cs{};
  bool call(array<ID3D11UnorderedAccessView*, extent<Unordered>> u, //
            array<ID3D11ShaderResourceView*, extent<Structured>> s, //
            array<ID3D11Buffer*, extent<Constant>> c,               //
            nat4 x, nat4 y, nat4 z) const {
    if (!(x && y && z)) return false;
    main::d3d_context->CSSetUnorderedAccessViews(0, u.count, u.data(), nullptr);
    main::d3d_context->CSSetShaderResources(0, s.count, s.data());
    main::d3d_context->CSSetConstantBuffers(0, c.count, c.data());
    main::d3d_context->CSSetShader(d3d_cs, 0, 0);
    main::d3d_context->Dispatch(x, y, z);
    return true;
  }
public:
  using ub_list = list<>::from_typepack<Unordered, const int&>;
  using sb_list = list<>::from_typepack<Structured, const int&>;
  using cb_list = list<>::from_typepack<Constant, const int&>;
  computing_shader() noexcept = default;
  explicit operator bool() const noexcept { return bool(d3d_cs); }

  /// \brief compiles compute shader
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

  /// \brief performs GPGPU calculation
  /// \param x, y, z: number of threads in each dimension; must be `(1, 1, 1)` or greater
  bool operator()(ub_list ub, sb_list sb, cb_list cb, numeric auto&& x, //
                  numeric auto&& y, numeric auto&& z, const source& _ = {}) const noexcept {
    try {
      if (!(*this)) throw std::runtime_error("the computing shader is not valid");
      using ub_array = array<ID3D11UnorderedAccessView*, extent<Unordered>>;
      using sb_array = array<ID3D11ShaderResourceView*, extent<Structured>>;
      using cb_array = array<ID3D11Buffer*, extent<Constant>>;
      return call([&]<nat... Is>(sequence<Is...>) { return ub_array{static_cast<ID3D11UnorderedAccessView*>(get<Is>(ub))...}; }(make_indices_for<ub_list>{}), //
                  [&]<nat... Is>(sequence<Is...>) { return sb_array{static_cast<ID3D11ShaderResourceView*>(get<Is>(sb))...}; }(make_indices_for<sb_list>{}),  //
                  [&]<nat... Is>(sequence<Is...>) { return cb_array{static_cast<ID3D11Buffer*>(get<Is>(cb))...}; }(make_indices_for<cb_list>{}), nat4(x), nat4(y), nat4(z));
    } catch (const std::exception& e) {
      main::log(logger::error, e.what());
      main::log(logger::error, "failed to perform GPGPU calculation", _);
      return false;
    }
  }

  /// \brief performs GPGPU calculation; `numthreads` must be `(1024, 1, 1)`
  bool operator()(numeric auto&& Parallels, ub_list ub, sb_list sb, cb_list cb, const source& _ = {}) const noexcept { //
    return operator()(mv(ub), mv(sb), mv(cb), (nat4(Parallels) - 1_n4) / 1024_n4, 1, 1, _);
  }
};
}

// export namespace yw {

// ////////////////////////////////////////////////////////////////////////////////
// /// \brief DirectX window class
// class dxwindow {
//   dxwindow(const dxwindow& Window) = delete;
//   dxwindow& operator=(const dxwindow& Window) = delete;
//   inline static const auto d2d_bitmap_properties = //
//     D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS(1) | D2D1_BITMAP_OPTIONS(2), {DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE(1)});
// protected:
//   yw::window _window{};
//   comptr<IDXGISwapChain1> _swapchain{};
//   comptr<ID2D1Bitmap1> _rendertarget{};
//   bool _resize_rendertarget(numeric auto&& Width, numeric auto&& Height) noexcept {
//     try {
//       if (!(*this)) {
//         main::log(logger::error, "the DirectX window is not valid");
//         return false;
//       }
//       _rendertarget.reset();
//       main::d2d_context->SetTarget(nullptr);
//       if (_swapchain->ResizeBuffers(0, nat4(Width), nat4(Height), DXGI_FORMAT_UNKNOWN, 0) != S_OK) {
//         main::log(logger::error, "failed to resize DXGI swap chain buffers");
//         return false;
//       } else if (comptr<ID3D11Texture2D> t{}; _swapchain->GetBuffer(0, IID_PPV_ARGS(t.init())) != S_OK) {
//         main::log(logger::error, "failed to get render target buffer");
//         return false;
//       } else if (comptr<IDXGISurface> s{}; t->QueryInterface(IID_PPV_ARGS(s.init())) != S_OK) {
//         main::log(logger::error, "failed to get DXGI surface");
//         return false;
//       } else if (main::d2d_context->CreateBitmapFromDxgiSurface(s, d2d_bitmap_properties, _rendertarget.init()) != S_OK) {
//         main::log(logger::error, "failed to create D2D1 bitmap from DXGI surface");
//         return false;
//       } else return main::d2d_context->SetTarget(_rendertarget), true;
//     } catch (const std::exception& e) {
//       main::log(logger::error, e.what());
//       return false;
//     }
//   }
// public:
//   const comptr<IDXGISwapChain1>& swapchain = _swapchain;
//   const comptr<ID2D1Bitmap1>& rendertarget = _rendertarget;
//   window::message_t& message = _window.message;
//   ~dxwindow() noexcept = default;
//   dxwindow() noexcept = default;
//   dxwindow(dxwindow&&) noexcept = default;
//   dxwindow& operator=(dxwindow&&) noexcept = default;
//   /// \brief creates a DirectX window
//   dxwindow(yw::window&& Window) noexcept : _window(mv(Window)) {
//     try {
//       if (!main::system::directx_initialized) {
//         main::log(logger::error, "DirectX system is not initialized");
//         main::log(logger::error, "failed to create DirectX window");
//         return;
//       }
//       if (!_window) {
//         main::log(logger::error, "the window is not valid");
//         main::log(logger::error, "failed to create DirectX window");
//         return;
//       }
//       // create swapchain
//       auto size = _window.size();
//       DXGI_SWAP_CHAIN_DESC1 scd{nat4(size.x), nat4(size.y), DXGI_FORMAT_B8G8R8A8_UNORM, false, {1, 0}};
//       scd.SampleDesc = {1, 0}, scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
//       scd.BufferCount = 2, scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
//       HRESULT hr = main::dxgi_factory->CreateSwapChainForHwnd( //
//         main::d3d_device, HWND(_window), &scd, nullptr, nullptr, _swapchain.init());
//       if (hr != S_OK) {
// if (hr != S_OK) //         main::log(logger::error, format_error(hr));
//         main::log(logger::error, "failed to create DXGI swap chain");
//         main::log(logger::error, "failed to create DirectX window");
//         return;
//       }
//       // create render-target
//       if (comptr<ID3D11Texture2D> t{}; (hr = _swapchain->GetBuffer(0, IID_PPV_ARGS(t.init())) != S_OK)) {
// if (hr != S_OK) //         main::log(logger::error, format_error(hr));
//         main::log(logger::error, "failed to get render target buffer");
//         main::log(logger::error, "failed to create DirectX window");
//         return;
//       } else if (comptr<IDXGISurface> s{}; (hr = t->QueryInterface(IID_PPV_ARGS(s.init())) != S_OK)) {
// if (hr != S_OK) //         main::log(logger::error, format_error(hr));
//         main::log(logger::error, "failed to get DXGI surface");
//         main::log(logger::error, "failed to create DirectX window");
//         return;
//       } else if (hr = main::d2d_context->CreateBitmapFromDxgiSurface(s, d2d_bitmap_properties, _rendertarget.init()); hr != S_OK) {
// if (hr != S_OK) //         main::log(logger::error, format_error(hr));
//         main::log(logger::error, "failed to create D2D1 bitmap from DXGI surface");
//         main::log(logger::error, "failed to create DirectX window");
//         return;
//       }
//     } catch (const std::exception& e) {
//       main::log(logger::error, e.what());
//       main::log(logger::error, "failed to create DirectX window");
//     }
//   }
//   /// \brief conversion to window handle
//   explicit operator HWND() const noexcept { return HWND(_window); }
//   /// \brief checks if the window is valid
//   [[nodiscard]] explicit operator bool() const noexcept { return _window && _swapchain && _rendertarget; }
//   /// \brief gets window text
//   [[nodiscard]] string<cat1> text() const noexcept { return _window.text(); }
//   /// \brief sets window text
//   void text(stringable auto&& Text) noexcept { _window.text(fwd<decltype(Text)>(Text)); }
//   /// \brief gets window position; `{x = left, y = top, z = right, w = bottom}`
//   [[nodiscard]] vector<int> position() const noexcept { return _window.position(); }
//   /// \brief sets window position
//   void position(numeric auto&& X, numeric auto&& Y) noexcept { return _window.position(X, Y); }
//   /// \brief sets window position and size
//   bool position(numeric auto&& X, numeric auto&& Y, numeric auto&& Width, numeric auto&& Height) noexcept {
//     if (!_resize_rendertarget(Width, Height) && _window.position(X, Y, Width, Height)) {
//       main::log(logger::error, "failed to resize the directx window");
//       return false;
//     } else return true;
//   }
//   /// \brief gets client area size; `{x = width, y = height}`
//   [[nodiscard]] vector2<int> size() const noexcept { return _window.size(); }
//   /// \brief sets client area size
//   bool size(numeric auto&& Width, numeric auto&& Height) noexcept {
//     if (!_resize_rendertarget(Width, Height) && _window.size(Width, Height)) {
//       main::log(logger::error, "failed to resize the directx window");
//       return false;
//     } else return true;
//   }
//   /// \brief displays a message box with an OK button
//   bool ok(const string<cat1>& Text, const string<cat1>& Caption = "OK?") const noexcept { return _window.ok(Text, Caption); }
//   /// \brief displays a message box with an OK button
//   bool ok(const string<cat2>& Text, const string<cat2>& Caption = L"OK?") const noexcept { return _window.ok(Text, Caption); }
//   /// \brief displays a message box with an OK button
//   bool ok(stringable auto&& Text, stringable auto&& Caption) const noexcept   //
//     requires different_from<remove_cvref<decltype(Text)>, string<cat1>> &&    //
//              different_from<remove_cvref<decltype(Caption)>, string<cat1>> && //
//              different_from<remove_cvref<decltype(Text)>, string<cat2>> &&    //
//              different_from<remove_cvref<decltype(Caption)>, string<cat2>> {
//     return _window.ok(fwd<decltype(Text)>(Text), fwd<decltype(Caption)>(Caption));
//   }
//   /// \brief displays a message box with an OK button
//   bool ok(stringable auto&& Text) const noexcept                           //
//     requires different_from<remove_cvref<decltype(Text)>, string<cat1>> && //
//              different_from<remove_cvref<decltype(Text)>, string<cat2>> {
//     return _window.ok(fwd<decltype(Text)>(Text));
//   }
//   /// \brief displays a message box with Yes and No buttons
//   [[nodiscard]] bool yes(const string<cat1>& Text, const string<cat1>& Caption = "Yes?") const noexcept { return _window.yes(Text, Caption); }
//   /// \brief displays a message box with Yes and No buttons
//   [[nodiscard]] bool yes(const string<cat2>& Text, const string<cat2>& Caption = L"Yes?") const noexcept { return _window.yes(Text, Caption); }
//   /// \brief displays a message box with Yes and No buttons
//   [[nodiscard]] bool yes(stringable auto&& Text, stringable auto&& Caption) const noexcept //
//     requires different_from<remove_cvref<decltype(Text)>, string<cat1>> &&                 //
//              different_from<remove_cvref<decltype(Caption)>, string<cat1>> &&              //
//              different_from<remove_cvref<decltype(Text)>, string<cat2>> &&                 //
//              different_from<remove_cvref<decltype(Caption)>, string<cat2>> {
//     return _window.yes(fwd<decltype(Text)>(Text), fwd<decltype(Caption)>(Caption));
//   }
//   /// \brief displays a message box with Yes and No buttons
//   [[nodiscard]] bool yes(stringable auto&& Text) const noexcept            //
//     requires different_from<remove_cvref<decltype(Text)>, string<cat1>> && //
//              different_from<remove_cvref<decltype(Text)>, string<cat2>> {
//     return _window.yes(fwd<decltype(Text)>(Text));
//   }
//   /// \brief waits and gets window message
//   bool wait_message() noexcept { return _window.wait_message(); }
//   /// \brief waits and gets window message in range
//   bool wait_message(nat4 Min, nat4 Max) noexcept { return _window.wait_message(Min, Max); }
//   /// \brief peeks window message
//   bool peek_message(nat4 Remove = 1) noexcept { return _window.peek_message(Remove); }
//   /// \brief peeks window message in range
//   bool peek_message(nat4 Min, nat4 Max, nat4 Remove = 1) noexcept { return _window.peek_message(Min, Max, Remove); }
//   /// \brief sends message
//   void send_message(wmessage Message, nat8 WParam = 0, int8 LParam = 0) noexcept { _window.send_message(Message, WParam, LParam); }
//   /// \brief posts message
//   void post_message(wmessage Message, nat8 WParam = 0, int8 LParam = 0) noexcept { _window.post_message(Message, WParam, LParam); }
//   /// \brief posts quit message
//   void post_quit_message(nat4 ExitCode = 0) noexcept { _window.post_quit_message(ExitCode); }
// };

// }

// export namespace yw {

// /// \brief checks if the object can be rendered
// /// \details The object must fulfill the following requirements:
// /// - `convertible_to<T, ID3D11RenderTargetView*>`
// /// -
// template<typename T> concept renderable =

// }
