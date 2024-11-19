#pragma once

#ifndef ywlib_com_init
#define ywlib_com_init COINIT_APARTMENTTHREADED
#endif

#include "ywlib-std.hpp"
#include "ywlib-windows.hpp"
#include "ywlib-xv.hpp"

export namespace yw {

template<typename Com> struct com_deleter;
template<typename Com> class comptr;
}

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

/// \brief deleter for COM objects
template<typename Com> struct yw::com_deleter {
  constexpr com_deleter() noexcept = default;
  template<typename U> constexpr com_deleter(const com_deleter<U>&) noexcept {}
  constexpr void operator()(Com* p) const noexcept { p ? void(p->Release()) : void(); }
};

/// \brief COM smart pointer
template<typename Com> class yw::comptr : public std::unique_ptr<Com, com_deleter<Com>> {
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

export namespace yw::main::system {
static_assert(sizeof(comptr<IUnknown>) == sizeof(IUnknown*));
class com_init_t {
  bool _initialized{};
public:
  explicit operator bool() const noexcept { return _initialized; }
  ~com_init_t() noexcept { _initialized ? ::CoUninitialize() : void(); }
  com_init_t(COINIT coinit) noexcept {
    if (auto r = ::CoInitializeEx(0, coinit); r == S_OK) _initialized = true;
    else if (r != S_FALSE) void(); // already initialized
    else main::log.format(logger::critical, "failed to initialize COM: {}", format_error(r));
  }
};
inline const com_init_t com_init{ywlib_com_init};
inline const D3D_FEATURE_LEVEL d3d_feature_levels[] = //
  {D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0};
inline const D3D11_RENDER_TARGET_BLEND_DESC d3d_render_target_blend_desc = //
  {true, D3D11_BLEND(5), D3D11_BLEND(6), D3D11_BLEND_OP(1), D3D11_BLEND(5), D3D11_BLEND(6), D3D11_BLEND_OP(1), 0xf};
inline const D3D11_BLEND_DESC d3d_blend_desc{false, false, {d3d_render_target_blend_desc}};
inline const D3D11_SAMPLER_DESC d3d_sampler_desc = //
  {D3D11_FILTER(1), D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP, 0, 1, D3D11_COMPARISON_FUNC(8)};
inline const D3D11_RASTERIZER_DESC d3d_rasterizer_desc{D3D11_FILL_SOLID, D3D11_CULL_NONE, true, 0, 0, 0, true, false, true, true};

comptr<ID3D11Device> d3d_device{};
comptr<ID3D11DeviceContext> d3d_context{};
comptr<IDXGIFactory2> dxgi_factory{};
comptr<IDXGIDevice2> dxgi_device{};
comptr<ID2D1Factory1> d2d_factory{};
comptr<ID2D1Device> d2d_device{};
comptr<ID2D1DeviceContext> d2d_context{};
comptr<IDWriteFactory1> dwrite_factory{};
comptr<IWICImagingFactory> wic_factory{};

inline const bool directx_initialized = [](comptr<ID3D11Device> a) {
  HRESULT hr{};
  try {
    // create d3d-device and d3d-context
    hr = ::D3D11CreateDevice(                                                                           //
      nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT, d3d_feature_levels, //
      arraysize(d3d_feature_levels), D3D11_SDK_VERSION, d3d_device.init(), nullptr, d3d_context.init());
    if (hr != S_OK) throw std::runtime_error("failed to create D3D11 device");
    // set d3d-blend-state
    if (comptr<ID3D11BlendState> bs{}; (hr = d3d_device->CreateBlendState(&d3d_blend_desc, bs.init())) != S_OK) //
      throw std::runtime_error("failed to create D3D11 blend state");
    else d3d_context->OMSetBlendState(bs, 0, 0xf);
    // set d3d-sampler-state
    if (comptr<ID3D11SamplerState> ss{}; (hr = d3d_device->CreateSamplerState(&d3d_sampler_desc, ss.init())) != S_OK) //
      throw std::runtime_error("failed to create D3D11 sampler state");
    else [&](auto p) { d3d_context->PSSetSamplers(0, 1, &p); }(ss.get());
    // set d3d-rasterizer-state
    if (comptr<ID3D11RasterizerState> rs{}; (hr = d3d_device->CreateRasterizerState(&d3d_rasterizer_desc, rs.init())) != S_OK) //
      throw std::runtime_error("failed to create D3D11 rasterizer state");
    else d3d_context->RSSetState(rs);
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
    hr = ::DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory1), reinterpret_cast<IUnknown**>(dwrite_factory.init()));
    if (hr != S_OK) throw std::runtime_error("failed to create DWrite factory");
    // create wic-factory
    hr = ::CoCreateInstance(CLSID_WICImagingFactory, 0, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(wic_factory.init()));
    if (hr != S_OK) throw std::runtime_error("failed to create WIC imaging factory");
    return true;
  } catch (const std::exception& e) {
    main::log(logger::error, format_error(hr));
    main::log(logger::error, e.what());
    main::log(logger::critical, "failed to initialize DirectX system");
    return false;
  }
}({});
}

export namespace yw::main {
inline const comptr<ID3D11Device>& d3d_device = system::d3d_device;
inline const comptr<ID3D11DeviceContext>& d3d_context = system::d3d_context;
inline const comptr<IDXGIFactory2>& dxgi_factory = system::dxgi_factory;
inline const comptr<IDXGIDevice2>& dxgi_device = system::dxgi_device;
inline const comptr<ID2D1Factory1>& d2d_factory = system::d2d_factory;
inline const comptr<ID2D1Device>& d2d_device = system::d2d_device;
inline const comptr<ID2D1DeviceContext>& d2d_context = system::d2d_context;
inline const comptr<IDWriteFactory1>& dwrite_factory = system::dwrite_factory;
inline const comptr<IWICImagingFactory>& wic_factory = system::wic_factory;
}

export namespace yw {
/// \brief primitive topologies
enum class topology;

/// \brief rendering shader class
template<specialization_of<typepack> VSResources, specialization_of<typepack> VSConstants,                           //
         specialization_of<typepack> GSResources = typepack<>, specialization_of<typepack> GSConstants = typepack<>, //
         specialization_of<typepack> PSResources = typepack<>, specialization_of<typepack> PSConstants = typepack<>>
class rendering_shader;

/// \brief computing shader class
template<specialization_of<typepack> Unordered,         //
         specialization_of<typepack> StructuredBuffers, //
         specialization_of<typepack> Constants>
class computing_shader;

/// \brief staging buffer class for copying data to CPU
template<typename T> class staging_buffer;

/// @brief base class for GPU buffers
template<typename T> class buffer;

/// \brief constant buffer class
template<typename T> requires ((sizeof(T) % 16) == 0) class constant_buffer;

/// \brief structured buffer class
template<typename T> class structured_buffer;

/// \brief unordered access buffer class
template<typename T> class unordered_buffer;
}

enum class yw::topology : int {
  point_list = int(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST),
  line_list = int(D3D11_PRIMITIVE_TOPOLOGY_LINELIST),
  line_strip = int(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP),
  triangle_list = int(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST),
  triangle_strip = int(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP),
};

template<yw::specialization_of<yw::typepack> VSResources, yw::specialization_of<yw::typepack> VSConstants, //
         yw::specialization_of<yw::typepack> GSResources, yw::specialization_of<yw::typepack> GSConstants, //
         yw::specialization_of<yw::typepack> PSResources, yw::specialization_of<yw::typepack> PSConstants>
class yw::rendering_shader {
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
  rendering_shader(const string_view<cat1> HLSL,          //
                   format_string<cat1> VSMAIN = "vsmain", //
                   format_string<cat1> GSMAIN = "gsmain", //
                   format_string<cat1> PSMAIN = "psmain", const source& _ = {}) noexcept {
    HRESULT hr{};
    comptr<ID3DBlob> b{}, e{};
    try {
      // compile vertex shader
      hr = ::D3DCompile(HLSL.data(), HLSL.size(), 0, 0, 0, VSMAIN.get().c_str(), "vs_5_0", (1 << 11), 0, b.init(), e.init());
      if (hr != S_OK) throw std::runtime_error("failed to compile vertex shader");
      // compile pixel shader
      b.reset(), e.reset();
      hr = ::D3DCompile(HLSL.data(), HLSL.size(), 0, 0, 0, PSMAIN.get().c_str(), "ps_5_0", (1 << 11), 0, b.init(), e.init());
      if (hr != S_OK) throw std::runtime_error("failed to compile pixel shader");
      // compile geometry shader if `GSMAIN` is found in HLSL
      if (HLSL.find(GSMAIN.get()) == npos) return;
      b.reset(), e.reset();
      hr = ::D3DCompile(HLSL.data(), HLSL.size(), 0, 0, 0, GSMAIN.get().c_str(), "gs_5_0", (1 << 11), 0, b.init(), e.init());
      if (hr != S_OK) throw std::runtime_error("failed to compile geometry shader");
    } catch (const std::exception& e) {
      if (hr != S_OK) main::log(logger::error, format_error(hr));
      if (e->GetBufferSize() != 0) main::log(logger::error, reinterpret_cast<const char*>(e->GetBufferPointer()));
      main::log(logger::error, e.what());
      main::log(logger::error, "failed to create rendering shader", _);
    }
  }

  /// \brief performs 3D rendering
  bool operator()(numeric auto&& VertexCounts,                          //
                  vs_resource_list vsr, vs_constant_list vsc,           //
                  gs_resource_list gsr = {}, gs_constant_list gsc = {}, //
                  ps_resource_list psr = {}, ps_constant_list psc = {}, const source& _ = {}) noexcept {
    try {
      if (!(*this)) throw std::runtime_error("the renderer is not valid");
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

template<yw::specialization_of<yw::typepack> Unordered,         //
         yw::specialization_of<yw::typepack> StructuredBuffers, //
         yw::specialization_of<yw::typepack> Constants>
class yw::computing_shader {
  static_assert(Unordered::count > 0, "unordered access views must be specified");
protected:
  comptr<ID3D11ComputeShader> d3d_cs{};
public:
  using ub_list = array<ID3D11UnorderedAccessView*, Unordered::count>;
  using sb_list = array<ID3D11ShaderResourceView*, StructuredBuffers::count>;
  using cb_list = array<ID3D11Buffer*, Constants::count>;
  computing_shader() noexcept = default;
  explicit operator bool() const noexcept { return bool(d3d_cs); }

  /// \brief compiles compute shader
  computing_shader(const string_view<cat1> HLSL, format_string<cat1> CSMain = "csmain", const source& _ = {}) noexcept {
    try {
      comptr<ID3DBlob> b{}, e{};
      auto hr = ::D3DCompile(HLSL.data(), HLSL.size(), 0, 0, 0, CSMain.get().c_str(), "cs_5_0", (1 << 11), 0, b.init(), e.init());
      if (hr != S_OK) {
        main::log(logger::error, format_error(hr));
        main::log(logger::error, reinterpret_cast<const char*>(e->GetBufferPointer()));
        main::log(logger::error, "failed to compile compute shader");
        main::log(logger::error, "failed to create computing shader", _);
        return;
      }
    } catch (const std::exception& e) {
      main::log(logger::error, e.what());
      main::log(logger::error, "failed to create computing shader", _);
    }
  }

  /// \brief performs GPGPU calculation
  bool operator()(ub_list UBuffers, sb_list SBuffers = {}, cb_list CBuffers = {},   //
                  numeric auto&& ThreadGroupX = 1, numeric auto&& ThreadGroupY = 1, //
                  numeric auto&& ThreadGroupZ = 1, const source& _ = {}) noexcept {
    try {
      if (!(*this)) {
        main::log(logger::error, "the GPGPU is not valid");
        main::log(logger::error, "failed to perform GPGPU calculation", _);
        return false;
      }
      if constexpr (ub_list::count != 0) main::d3d_context->CSSetUnorderedAccessViews(0, UBuffers.count, UBuffers.data(), nullptr);
      if constexpr (sb_list::count != 0) main::d3d_context->CSSetShaderResources(0, SBuffers.count, SBuffers.data());
      if constexpr (cb_list::count != 0) main::d3d_context->CSSetConstantBuffers(0, Constants.count, Constants.data());
      main::d3d_context->CSSetShader(d3d_cs, 0, 0);
      main::d3d_context->Dispatch(nat4(ThreadGroupX), nat4(ThreadGroupY), nat4(ThreadGroupZ));
      return true;
    } catch (const std::exception& e) {
      main::log(logger::error, e.what());
      main::log(logger::error, "failed to perform GPGPU calculation", _);
      return false;
    }
  }

  /// \brief performs GPGPU calculation; `numthreads` must be `(1024, 1, 1)`
  bool operator()(numeric auto&& Parallels, ub_list UBuffers, sb_list SBuffers = {}, //
                  cb_list CBuffers = {}, const source& _ = {}) noexcept {
    return operator()(mv(UBuffers), mv(SBuffers), mv(CBuffers), (nat4(Parallels) - 1_n4) / 1024_n4, 1, 1, _);
  }
};

template<typename T> class yw::buffer {
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

template<typename T> class yw::staging_buffer : public buffer<T> {
public:
  operator ::ID3D11Buffer*() const noexcept { return buffer<T>::d3d_buffer; }
  staging_buffer() noexcept = default;
  explicit staging_buffer(const buffer<T>& Src) : staging_buffer(Src.count) { buffer<T>::from(Src); }

  /// \brief creates a staging buffer with the specified size
  explicit staging_buffer(numeric auto&& Count) noexcept : buffer<T>(Count) {
    try {
      D3D11_BUFFER_DESC bd{nat4(sizeof(T) * buffer<T>::count), D3D11_USAGE_STAGING, 0, D3D11_CPU_ACCESS_READ};
      if (auto hr = main::d3d_device->CreateBuffer(&bd, nullptr, buffer<T>::d3d_buffer.init()); hr != S_OK) {
        main::log(logger::error, format_error(hr));
        main::log(logger::error, "failed to create staging buffer");
      }
    } catch (const std::exception& e) {
      main::log(logger::error, e.what());
      main::log(logger::error, "failed to create staging buffer");
    }
  }

  /// \brief copies the buffer to CPU
  array<T> to_cpu(const source& _ = {}) const noexcept {
    try {
      if (!(*this)) throw std::runtime_error("the staging buffer is not valid");
      array<T> data(buffer<T>::count);
      D3D11_MAPPED_SUBRESOURCE msr{};
      if (auto hr = main::d3d_context->Map(*this, 0, D3D11_MAP_READ, 0, &msr); hr != S_OK) {
        main::log(logger::error, format_error(hr));
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

template<typename T> requires ((sizeof(T) % 16) == 0) //
class yw::constant_buffer : public buffer<T> {
  inline static const D3D11_BUFFER_DESC bd = //
    {sizeof(T), D3D11_USAGE_DYNAMIC, D3D11_BIND_CONSTANT_BUFFER, D3D11_CPU_ACCESS_WRITE};
public:
  using buffer<T>::operator ID3D11Buffer*;
  constant_buffer() noexcept = default;

  /// \brief creates a constant buffer and loads the specified value
  template<convertible_to<T> U = T> explicit constant_buffer(U&& Val, const source& _ = {}) noexcept : buffer<T>(1) {
    try {
      if constexpr (included_in<U, T&, const T&>) {
        D3D11_SUBRESOURCE_DATA srd{&Val};
        if (auto hr = main::d3d_device->CreateBuffer(&bd, &srd, buffer<T>::d3d_buffer.init()); hr != S_OK) {
          main::log(logger::error, format_error(hr));
          main::log(logger::error, "failed to create constant buffer", _);
        }
      } else {
        T tmp = fwd<U>(Val);
        D3D11_SUBRESOURCE_DATA srd{&tmp};
        if (auto hr = main::d3d_device->CreateBuffer(&bd, &srd, buffer<T>::d3d_buffer.init()); hr != S_OK) {
          main::log(logger::error, format_error(hr));
          main::log(logger::error, "failed to create constant buffer", _);
        }
      }
    } catch (const std::exception& e) {
      main::log(logger::error, e.what());
      main::log(logger::error, "failed to create constant buffer", _);
    }
  }

  /// \brief updates the buffer with the specified value
  template<convertible_to<T> U = T> bool from(U&& Val, const source& _ = {}) noexcept {
    try {
      D3D11_MAPPED_SUBRESOURCE msr{};
      if constexpr (included_in<U, T&, const T&>) {
        if (auto hr = main::d3d_context->Map(*this, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr); hr != S_OK) {
          main::log(logger::error, format_error(hr));
          main::log(logger::error, "failed to map constant buffer", _);
          return false;
        }
        memcpy(msr.pData, &Val, sizeof(T));
      } else {
        T tmp = fwd<U>(Val);
        if (auto hr = main::d3d_context->Map(*this, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr); hr != S_OK) {
          main::log(logger::error, format_error(hr));
          main::log(logger::error, "failed to map constant buffer", _);
          return false;
        }
        memcpy(msr.pData, &tmp, sizeof(T));
      }
      main::d3d_context->Unmap(*this, 0);
    } catch (const std::exception& e) {
      main::log(logger::error, e.what());
      main::log(logger::error, "failed to update constant buffer", _);
      return false;
    }
  }

  /// \brief updates the buffer through the specified function
  template<invocable<T&> F> bool from(F&& Func) noexcept {
    try {
      D3D11_MAPPED_SUBRESOURCE msr{};
      if (auto hr = main::d3d_context->Map(*this, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr); hr != S_OK) {
        main::log(logger::error, format_error(hr));
        main::log(logger::error, "failed to map constant buffer");
        return false;
      }
      Func(*reinterpret_cast<T*>(msr.pData));
      main::d3d_context->Unmap(*this, 0);
      return true;
    } catch (const std::exception& e) {
      main::log(logger::error, e.what());
      main::log(logger::error, "failed to update constant buffer");
      return false;
    }
  }
};

template<typename T> class yw::structured_buffer : public buffer<T> {
protected:
  comptr<ID3D11ShaderResourceView> d3d_srv{};
public:
  structured_buffer() noexcept = default;
  operator ::ID3D11Buffer*() const noexcept { return buffer<T>::d3d_buffer; }
  operator ::ID3D11ShaderResourceView*() const noexcept { return d3d_srv; }
  explicit operator bool() const noexcept { return buffer<T>::d3d_buffer && d3d_srv; }
  explicit structured_buffer(const buffer<T>& Src) noexcept : structured_buffer(Src.count) { buffer<T>::from(Src); }

  /// \brief creates a structured buffer with the specified size
  explicit structured_buffer(numeric auto&& Count, const source& _ = {}) noexcept : buffer<T>(Count) {
    try {
      D3D11_BUFFER_DESC bd{nat4(sizeof(T) * buffer<T>::count), D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE};
      bd.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED, bd.StructureByteStride = sizeof(T);
      if (auto hr = main::d3d_device->CreateBuffer(&bd, nullptr, buffer<T>::d3d_buffer.init()); hr != S_OK) {
        main::log(logger::error, format_error(hr));
        main::log(logger::error, "failed to create buffer for structured buffer");
        main::log(logger::error, "failed to create structured buffer", _);
      }
      D3D11_SHADER_RESOURCE_VIEW_DESC srvd{DXGI_FORMAT_UNKNOWN, D3D11_SRV_DIMENSION_BUFFER, D3D11_BUFFER_SRV{0, buffer<T>::count}};
      if (auto hr = main::d3d_device->CreateShaderResourceView(*this, &srvd, d3d_srv.init()); hr != S_OK) {
        main::log(logger::error, format_error(hr));
        main::log(logger::error, "failed to create shader resource view");
        main::log(logger::error, "failed to create structured buffer", _);
      }
    } catch (const std::exception& e) {
      main::log(logger::error, e.what());
      main::log(logger::error, "failed to create structured buffer", _);
    }
  }

  /// \brief creates a structured buffer from CPU data
  structured_buffer(convertible_to<const T*> auto&& Data, numeric auto&& Count, const source& _ = {}) noexcept : buffer<T>(Count) {
    try {
      D3D11_BUFFER_DESC bd{nat4(sizeof(T) * buffer<T>::count), D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE};
      bd.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED, bd.StructureByteStride = sizeof(T);
      const T* data = Data;
      D3D11_SUBRESOURCE_DATA srd{data, sizeof(T)};
      if (auto hr = main::d3d_device->CreateBuffer(&bd, &srd, buffer<T>::d3d_buffer.init()); hr != S_OK) {
        main::log(logger::error, format_error(hr));
        main::log(logger::error, "failed to create buffer for structured buffer");
        main::log(logger::error, "failed to create structured buffer", _);
      }
      D3D11_SHADER_RESOURCE_VIEW_DESC srvd{DXGI_FORMAT_UNKNOWN, D3D11_SRV_DIMENSION_BUFFER, D3D11_BUFFER_SRV{0, buffer<T>::count}};
      if (auto hr = main::d3d_device->CreateShaderResourceView(*this, &srvd, d3d_srv.init()); hr != S_OK) {
        main::log(logger::error, format_error(hr));
        main::log(logger::error, "failed to create shader resource view");
        main::log(logger::error, "failed to create structured buffer", _);
      }
    } catch (const std::exception& e) {
      main::log(logger::error, e.what());
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
      if (!(*this)) {
        main::log(logger::error, "the structured buffer is not valid");
        main::log(logger::error, "failed to update structured buffer", _);
        return;
      }
      if (buffer<T>::count != Count) {
        main::log(logger::error, "the size of the source data must be the same as this buffer");
        main::log(logger::error, "failed to update structured buffer", _);
        return;
      }
      const T* data = Data;
      main::d3d_context->UpdateSubresource(*this, 0, nullptr, data, 0, 0);
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

template<typename T> class yw::unordered_buffer : public buffer<T> {
protected:
  comptr<ID3D11UnorderedAccessView> d3d_uav{};
public:
  using buffer<T>::operator ID3D11Buffer*;
  unordered_buffer() noexcept = default;
  operator ID3D11UnorderedAccessView*() const noexcept { return d3d_uav; }
  explicit operator bool() const noexcept { return buffer<T>::d3d_buffer && d3d_uav; }
  explicit unordered_buffer(const buffer<T>& Src) noexcept : unordered_buffer(Src.count) { buffer<T>::from(Src); }

  /// \brief creates an unordered access buffer with the specified size
  explicit unordered_buffer(numeric auto&& Count, const source& _ = {}) noexcept : buffer<T>(Count) {
    try {
      D3D11_BUFFER_DESC bd{nat4(sizeof(T) * buffer<T>::count), D3D11_USAGE_DEFAULT, D3D11_BIND_UNORDERED_ACCESS};
      bd.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED, bd.StructureByteStride = sizeof(T);
      if (auto hr = main::d3d_device->CreateBuffer(&bd, nullptr, buffer<T>::d3d_buffer.init()); hr != S_OK) {
        main::log(logger::error, format_error(hr));
        main::log(logger::error, "failed to create buffer for unordered access buffer");
        main::log(logger::error, "failed to create unordered access buffer", _);
      }
      D3D11_UNORDERED_ACCESS_VIEW_DESC uavd{DXGI_FORMAT_UNKNOWN, D3D11_UAV_DIMENSION_BUFFER, D3D11_BUFFER_UAV{0, buffer<T>::count}};
      if (auto hr = main::d3d_device->CreateUnorderedAccessView(*this, &uavd, d3d_uav.init()); hr != S_OK) {
        main::log(logger::error, format_error(hr));
        main::log(logger::error, "failed to create unordered access view");
        main::log(logger::error, "failed to create unordered access buffer", _);
      }
    } catch (const std::exception& e) {
      main::log(logger::error, e.what());
      main::log(logger::error, "failed to create unordered access buffer", _);
    }
  }

  /// \brief creates an unordered access buffer from CPU data
  unordered_buffer(convertible_to<const T*> auto&& Data, numeric auto&& Count, const source& _ = {}) noexcept : buffer<T>(Count) {
    try {
      D3D11_BUFFER_DESC bd{nat4(sizeof(T) * buffer<T>::count), D3D11_USAGE_DEFAULT, D3D11_BIND_UNORDERED_ACCESS};
      bd.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED, bd.StructureByteStride = sizeof(T);
      const T* data = Data;
      D3D11_SUBRESOURCE_DATA srd{data, sizeof(T)};
      if (auto hr = main::d3d_device->CreateBuffer(&bd, &srd, buffer<T>::d3d_buffer.init()); hr != S_OK) {
        main::log(logger::error, format_error(hr));
        main::log(logger::error, "failed to create buffer for unordered access buffer");
        main::log(logger::error, "failed to create unordered access buffer", _);
      }
      D3D11_UNORDERED_ACCESS_VIEW_DESC uavd{DXGI_FORMAT_UNKNOWN, D3D11_UAV_DIMENSION_BUFFER, D3D11_BUFFER_UAV{0, buffer<T>::count}};
      if (auto hr = main::d3d_device->CreateUnorderedAccessView(*this, &uavd, d3d_uav.init()); hr != S_OK) {
        main::log(logger::error, format_error(hr));
        main::log(logger::error, "failed to create unordered access view");
        main::log(logger::error, "failed to create unordered access buffer", _);
      }
    } catch (const std::exception& e) {
      main::log(logger::error, e.what());
      main::log(logger::error, "failed to create unordered access buffer", _);
    }
  }

  /// \brief creates an unordered access buffer from CPU data
  template<contiguous_range Rg> requires same_as<iter_value<Rg>, T> //
  unordered_buffer(Rg&& Range, const source& _ = {}) noexcept : unordered_buffer(Range.data(), Range.size(), _) {}

  /// \brief updates the buffer with the specified data
  void from(convertible_to<const T*> auto&& Data, numeric auto&& Count, const source& _ = {}) noexcept {
    try {
      if (!(*this)) {
        main::log(logger::error, "the unordered access buffer is not valid");
        main::log(logger::error, "failed to update unordered access buffer", _);
        return;
      }
      if (buffer<T>::count != Count) {
        main::log(logger::error, "the size of the source data must be the same as this buffer");
        main::log(logger::error, "failed to update unordered access buffer", _);
        return;
      }
      const T* data = Data;
      if (auto hr = main::d3d_context->UpdateSubresource(*this, 0, nullptr, data, 0, 0); hr != S_OK) {
        main::log(logger::error, format_error(hr));
        main::log(logger::error, "failed to update unordered access buffer", _);
      }
    } catch (const std::exception& e) {
      main::log(logger::error, e.what());
      main::log(logger::error, "failed to update unordered access buffer", _);
    }
  }
};

export namespace yw {

template<typename T> staging_buffer(const buffer<T>&) -> staging_buffer<T>;

template<typename T> std::vector<T> buffer<T>::to_cpu() const noexcept {
  staging_buffer<T> staging(*this);
  return staging.to_cpu();
}

template<typename T> std::vector<T> buffer<T>::to_cpu(staging_buffer<T>& Staging) const noexcept {
  Staging.from(*this);
  return Staging.to_cpu();
}

template<typename T> constant_buffer(const T&) -> constant_buffer<T>;

template<typename T> structured_buffer(const buffer<T>&) -> structured_buffer<T>;
template<typename T> structured_buffer(const T*, numeric auto&&) -> structured_buffer<T>;
template<contiguous_range Rg> structured_buffer(Rg&&) -> structured_buffer<iter_value<Rg>>;

template<typename T> unordered_buffer(const buffer<T>&) -> unordered_buffer<T>;
template<typename T> unordered_buffer(const T*, numeric auto&&) -> unordered_buffer<T>;
template<contiguous_range Rg> unordered_buffer(Rg&&) -> unordered_buffer<iter_value<Rg>>;

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
//         main::log(logger::error, format_error(hr));
//         main::log(logger::error, "failed to create DXGI swap chain");
//         main::log(logger::error, "failed to create DirectX window");
//         return;
//       }
//       // create render-target
//       if (comptr<ID3D11Texture2D> t{}; (hr = _swapchain->GetBuffer(0, IID_PPV_ARGS(t.init())) != S_OK)) {
//         main::log(logger::error, format_error(hr));
//         main::log(logger::error, "failed to get render target buffer");
//         main::log(logger::error, "failed to create DirectX window");
//         return;
//       } else if (comptr<IDXGISurface> s{}; (hr = t->QueryInterface(IID_PPV_ARGS(s.init())) != S_OK)) {
//         main::log(logger::error, format_error(hr));
//         main::log(logger::error, "failed to get DXGI surface");
//         main::log(logger::error, "failed to create DirectX window");
//         return;
//       } else if (hr = main::d2d_context->CreateBitmapFromDxgiSurface(s, d2d_bitmap_properties, _rendertarget.init()); hr != S_OK) {
//         main::log(logger::error, format_error(hr));
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
