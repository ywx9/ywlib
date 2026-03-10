#pragma once

#include "ywlib"

#include <d2d1_1.h>
#include <d3d11.h>
#include <dwrite_1.h>
#include <dxgi1_3.h>
#include <wincodec.h>
#include <xaudio2.h>

namespace yw {

//////////////////////////////////////// MARK: ok/yes

inline bool ok(null_terminated<wchar_t> Text, null_terminated<wchar_t> Title = L"Confirmation") {
  return ::MessageBoxW(nullptr, Text.data(), Title.data(), MB_OK) == IDOK;
}

inline bool yes(null_terminated<wchar_t> Text, null_terminated<wchar_t> Title = L"Confirmation") {
  return ::MessageBoxW(nullptr, Text.data(), Title.data(), MB_YESNO) == IDYES;
}

//////////////////////////////////////// MARK: print_fallback

/// prints formatted string and shows message box as fallback
inline constexpr auto print_fallback = []<typename S, typename... Ts>(S&& fmt, Ts&&... as) {
  const auto s = format(static_cast<S&&>(fmt), static_cast<Ts&&>(as)...);
  print(s);
  ok(s, L"Message");
};

/// prints error message and shows message box as fallback
/// \return system error code
inline constexpr auto print_error_fallback = //
  []<stringable S>(S&& message, const error_trace& err, const source& src = {}) -> int {
  const auto s = format("{}\n  at {}\n{}", err, src, message);
  print.err(s);
  ok(s, L"Error");
  return err.error.system_code;
};

//////////////////////////////////////// MARK: unexpected_win32_error

inline std::unexpected<error_trace> unexpected_win32_error(const char* msg, const source& src = {}) {
  return unexpected_error(errors::operation_failed, msg, int32_t(::GetLastError()), {}, src);
}

//////////////////////////////////////// MARK: desktop_client_size

inline uint2 desktop_client_size() {
  static const uint2 _size = [] {
    if (RECT r; !::GetClientRect(::GetDesktopWindow(), &r)) return uint2();
    else return uint2(r.right, r.bottom);
  }();
  return _size;
}

//////////////////////////////////////// MARK: wclass

inline class {
  bool _initialized{};
  HINSTANCE _hinstance{};
  std::wstring _name{};

public:
  static LRESULT __stdcall proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

  std::expected<void, error_trace> initialize() {
    if (_initialized) return {};
    _hinstance = ::GetModuleHandleW(nullptr);
    _name = L"ywlib_window_class";
    WNDCLASSW wc{};
    wc.lpfnWndProc = proc;
    wc.hInstance = _hinstance;
    wc.hCursor = ::LoadCursorW(nullptr, IDC_ARROW);
    wc.lpszClassName = _name.data();
    if (!::RegisterClassW(&wc)) return unexpected_win32_error("RegisterClassW failed");
    _initialized = true;
    return {};
  }

  HINSTANCE hinstance() const noexcept { return _hinstance; }
  const std::wstring& name() const noexcept { return _name; }
} wclass;

//////////////////////////////////////// MARK: comptr

template<typename Com> class comptr {
  comptr(const comptr&) = delete;
  comptr& operator=(const comptr&) = delete;
  Com* p{nullptr};

public:
  explicit operator bool() const { return p != nullptr; }
  Com* operator->() const { return p; }
  bool operator==(Com* other) { return p == other; }
  ~comptr() {
    if (p) p->Release();
    p = nullptr;
  }
  comptr() = default;
  comptr(comptr&& other) : p(std::exchange(other.p, nullptr)) {}
  comptr& operator=(comptr&& other) {
    if (this == &other) return *this;
    if (p) p->Release();
    p = std::exchange(other.p, nullptr);
    return *this;
  }
  Com*& get() & { return p; }
  Com* get() const& { return p; }
  void release() {
    if (p) p->Release();
    p = nullptr;
  }
};

//////////////////////////////////////// MARK: d3d

inline class {
  struct pointers {
    ID3D11Device* device{};
    ID3D11DeviceContext* context{};
    ID3D11BlendState* blend_state{};
    ID3D11RasterizerState* rasterizer_state{};
    ID3D11SamplerState* sampler_state{};
    ID3D11DepthStencilState* depth_stencil_state{};
    bool initialized{};

    ~pointers() {
      if (blend_state) blend_state->Release();
      if (rasterizer_state) rasterizer_state->Release();
      if (sampler_state) sampler_state->Release();
      if (depth_stencil_state) depth_stencil_state->Release();
      if (context) context->Release();
      if (device) device->Release();
      initialized = false;
    }
  } p{};

public:
  std::expected<void, error_trace> initialize() {
    if (p.initialized) return {};
    const D3D_FEATURE_LEVEL _levels[] = {D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0};
    auto hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT, _levels,
      _countof(_levels), D3D11_SDK_VERSION, &p.device, nullptr, &p.context);
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "D3D11CreateDevice failed", int32_t(hr));
    D3D11_BLEND_DESC blend_desc{};
    blend_desc.RenderTarget[0].BlendEnable = TRUE;
    blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    hr = p.device->CreateBlendState(&blend_desc, &p.blend_state);
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateBlendState failed", int32_t(hr));
    p.context->OMSetBlendState(p.blend_state, nullptr, 0xffffffff);
    D3D11_RASTERIZER_DESC rasterizer_desc{};
    rasterizer_desc.FillMode = D3D11_FILL_SOLID;
    rasterizer_desc.CullMode = D3D11_CULL_BACK;
    rasterizer_desc.FrontCounterClockwise = TRUE;
    rasterizer_desc.DepthClipEnable = TRUE;
    hr = p.device->CreateRasterizerState(&rasterizer_desc, &p.rasterizer_state);
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateRasterizerState failed", int32_t(hr));
    p.context->RSSetState(p.rasterizer_state);
    D3D11_SAMPLER_DESC sampler_desc{};
    sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    hr = p.device->CreateSamplerState(&sampler_desc, &p.sampler_state);
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateSamplerState failed", int32_t(hr));
    p.context->PSSetSamplers(0, 1, &p.sampler_state);
    D3D11_DEPTH_STENCIL_DESC depth_stencil_desc{};
    depth_stencil_desc.DepthEnable = TRUE;
    depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depth_stencil_desc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL; // Reverse-Z
    hr = p.device->CreateDepthStencilState(&depth_stencil_desc, &p.depth_stencil_state);
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateDepthStencilState failed", int32_t(hr));
    p.context->OMSetDepthStencilState(p.depth_stencil_state, 0);
    p.initialized = true;
    return {};
  }

  void release() {
    if (p.blend_state) std::exchange(p.blend_state, nullptr)->Release();
    if (p.rasterizer_state) std::exchange(p.rasterizer_state, nullptr)->Release();
    if (p.sampler_state) std::exchange(p.sampler_state, nullptr)->Release();
    if (p.depth_stencil_state) std::exchange(p.depth_stencil_state, nullptr)->Release();
    if (p.context) std::exchange(p.context, nullptr)->Release();
    if (p.device) std::exchange(p.device, nullptr)->Release();
    p.initialized = false;
  }

  ID3D11Device* device() { return p.device; }
  ID3D11DeviceContext* context() { return p.context; }
  ID3D11BlendState* blend_state() { return p.blend_state; }
  ID3D11RasterizerState* rasterizer_state() { return p.rasterizer_state; }
  ID3D11SamplerState* sampler_state() { return p.sampler_state; }
  ID3D11DepthStencilState* depth_stencil_state() { return p.depth_stencil_state; }
} d3d;

//////////////////////////////////////// MARK: dxgi

inline class {
  struct pointers {
    IDXGIFactory2* factory{nullptr};
    IDXGIDevice2* device{nullptr};
    bool initialized{false};

    ~pointers() {
      if (device) device->Release();
      if (factory) factory->Release();
      initialized = false;
    }
  } p{};

public:
  std::expected<void, error_trace> initialize() {
    if (p.initialized) return {};
    if (auto res = d3d.initialize(); !res) return unexpected_error(res.error());
    auto hr = ::CreateDXGIFactory2(0, __uuidof(IDXGIFactory2), reinterpret_cast<void**>(&p.factory));
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateDXGIFactory2 failed", int32_t(hr));
    hr = d3d.device()->QueryInterface(__uuidof(IDXGIDevice2), reinterpret_cast<void**>(&p.device));
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateDevice failed", int32_t(hr));
    p.initialized = true;
    return {};
  }

  void release() {
    if (p.device) std::exchange(p.device, nullptr)->Release();
    if (p.factory) std::exchange(p.factory, nullptr)->Release();
    p.initialized = false;
  }

  IDXGIFactory2* factory() { return p.factory; }
  IDXGIDevice2* device() { return p.device; }
} dxgi;

//////////////////////////////////////// MARK: coinit

inline class {
  struct pointers {
    bool initialized{};

    ~pointers() {
      if (initialized) ::CoUninitialize();
    }
  } p{};

public:
  std::expected<void, error_trace> initialize() {
    if (p.initialized) return {};
    if (auto hr = ::CoInitializeEx(nullptr, COINIT_MULTITHREADED); FAILED(hr))
      return unexpected_error(errors::operation_failed, "CoInitializeEx failed", int32_t(hr));
    p.initialized = true;
    return {};
  }

  void release() {
    if (std::exchange(p.initialized, false)) ::CoUninitialize();
  }
} coinit;

//////////////////////////////////////// MARK: d2d

inline class {
  struct pointers {
    ID2D1Factory1* factory{};
    ID2D1Device* device{};
    ID2D1DeviceContext* context{};
    bool initialized{};

    ~pointers() {
      if (context) context->Release();
      if (device) device->Release();
      if (factory) factory->Release();
      initialized = false;
    }
  } p{};

public:
  std::expected<void, error_trace> initialize() {
    if (p.initialized) return {};
    if (auto res = dxgi.initialize(); !res) return unexpected_error(res.error());
    if (auto res = coinit.initialize(); !res) return unexpected_error(res.error());
    auto hr = ::D2D1CreateFactory(
      D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory1), reinterpret_cast<void**>(&p.factory));
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "D2D1CreateFactory failed", int32_t(hr));
    hr = p.factory->CreateDevice(dxgi.device(), &p.device);
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateDevice failed", int32_t(hr));
    hr = p.device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &p.context);
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateDeviceContext failed", int32_t(hr));
    p.initialized = true;
    return {};
  }

  void release() {
    if (p.context) std::exchange(p.context, nullptr)->Release();
    if (p.device) std::exchange(p.device, nullptr)->Release();
    if (p.factory) std::exchange(p.factory, nullptr)->Release();
    p.initialized = false;
  }

  ID2D1Factory1* factory() { return p.factory; }
  ID2D1Device* device() { return p.device; }
  ID2D1DeviceContext* context() { return p.context; }
} d2d;

//////////////////////////////////////// MARK: dwrite

inline class {
  struct pointers {
    IDWriteFactory1* factory{};
    IDWriteTextFormat* text_format{};
    bool initialized = false;

    ~pointers() {
      if (text_format) std::exchange(text_format, nullptr)->Release();
      if (factory) std::exchange(factory, nullptr)->Release();
      initialized = false;
    }
  } p{};

public:
  std::expected<void, error_trace> initialize() {
    if (p.initialized) return {};
    if (auto res = d2d.initialize(); !res) return unexpected_error(res.error());
    // Creates DirectWrite factory.
    auto hr = ::DWriteCreateFactory(
      DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory1), reinterpret_cast<IUnknown**>(&p.factory));
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "DWriteCreateFactory failed", int32_t(hr));
    // Creates default text format.
    hr = p.factory->CreateTextFormat(L"", nullptr, DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE_NORMAL,
      DWRITE_FONT_STRETCH_NORMAL, 16.0f, L"", &p.text_format);
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateTextFormat failed", int32_t(hr));
    if (!p.text_format) return unexpected_error(errors::operation_failed, "CreateTextFormat returned null");
    p.initialized = true;
    return {};
  }

  void release() {
    if (p.text_format) std::exchange(p.text_format, nullptr)->Release();
    if (p.factory) std::exchange(p.factory, nullptr)->Release();
    p.initialized = false;
  }

  IDWriteFactory1* factory() { return p.factory; }

  IDWriteTextFormat* text_format() {
    if (!initialize()) return nullptr;
    return p.text_format;
  }
} dwrite;

///////////////////////////////////////// MARK: wic

inline class {
  struct pointers {
    ::IWICImagingFactory2* factory{};
    bool initialized{};

    ~pointers() {
      if (factory) std::exchange(factory, nullptr)->Release();
      initialized = false;
    }
  } p{};

public:
  std::expected<void, error_trace> initialize() {
    if (p.initialized) return {};
    if (auto res = d2d.initialize(); !res) return unexpected_error(res.error());
    auto hr = ::CoCreateInstance(CLSID_WICImagingFactory2, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&p.factory));
    if (FAILED(hr))
      return unexpected_error(errors::operation_failed, "CoCreateInstance for WICImagingFactory2 failed", int32_t(hr));
    p.initialized = true;
    return {};
  }

  void release() {
    if (p.factory) std::exchange(p.factory, nullptr)->Release();
    p.initialized = false;
  }

  ::IWICImagingFactory2* factory() { return p.factory; }
} wic;

//////////////////////////////////////// MARK: xaudio2

inline class {
  struct pointers {
    ::IXAudio2* xaudio2{};
    ::IXAudio2MasteringVoice* mastering_voice{};
    bool initialized{};

    ~pointers() {
      if (mastering_voice) std::exchange(mastering_voice, nullptr)->DestroyVoice();
      if (xaudio2) std::exchange(xaudio2, nullptr)->Release();
      initialized = false;
    }
  } p{};

public:
  std::expected<void, error_trace> initialize() {
    if (p.initialized) return {};
    if (auto res = coinit.initialize(); !res) return unexpected_error(res.error());
    auto hr = ::XAudio2Create(&p.xaudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "XAudio2Create failed", int32_t(hr));
    hr = p.xaudio2->CreateMasteringVoice(&p.mastering_voice);
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateMasteringVoice failed", int32_t(hr));
    p.initialized = true;
    return {};
  }

  void release() {
    if (p.mastering_voice) std::exchange(p.mastering_voice, nullptr)->DestroyVoice();
    if (p.xaudio2) std::exchange(p.xaudio2, nullptr)->Release();
    p.initialized = false;
  }

  ::IXAudio2* device() { return p.xaudio2; }
  ::IXAudio2MasteringVoice* mastering_voice() { return p.mastering_voice; }
} xaudio2;
} // namespace yw
