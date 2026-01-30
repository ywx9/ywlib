#pragma once

#include "ywlib"

#include <print>
#include <variant>

#include <d2d1_1.h>
#include <d3d11.h>
#include <dwrite_1.h>
#include <dxgi1_3.h>
#include <wincodec.h>
#include <xaudio2.h>

namespace yw {

//////////////////////////////////////// MARK: ok/yes

inline bool ok(const null_terminated<wchar_t>& Text, const null_terminated<wchar_t>& Title = L"Confirmation") {
  return ::MessageBoxW(nullptr, Text.data(), Title.data(), MB_OK) == IDOK;
}

inline bool yes(const null_terminated<wchar_t>& Text, const null_terminated<wchar_t>& Title = L"Confirmation") {
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
  print(s);
  ok(s, L"Error");
  return err.error.system_code;
};

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
  ::ID3D11Device* _device{nullptr};
  ::ID3D11DeviceContext* _context{nullptr};
  ::ID3D11BlendState* _blend_state{nullptr};
  ::ID3D11RasterizerState* _rasterizer_state{nullptr};
  ::ID3D11SamplerState* _sampler_state{nullptr};
  ::ID3D11DepthStencilState* _depth_stencil_state{nullptr};
  bool _initialized{false};

public:
  std::expected<void, error_trace> initialize() {
    if (_initialized) return {};
    const D3D_FEATURE_LEVEL _levels[] = {D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0};
    auto hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT, _levels,
      _countof(_levels), D3D11_SDK_VERSION, &_device, nullptr, &_context);
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "D3D11CreateDevice failed", hr);
    D3D11_BLEND_DESC blend_desc{};
    blend_desc.RenderTarget[0].BlendEnable = TRUE;
    blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    hr = _device->CreateBlendState(&blend_desc, &_blend_state);
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateBlendState failed", hr);
    _context->OMSetBlendState(_blend_state, nullptr, 0xffffffff);
    D3D11_RASTERIZER_DESC rasterizer_desc{};
    rasterizer_desc.FillMode = D3D11_FILL_SOLID;
    rasterizer_desc.CullMode = D3D11_CULL_BACK;
    rasterizer_desc.FrontCounterClockwise = TRUE;
    rasterizer_desc.DepthClipEnable = TRUE;
    hr = _device->CreateRasterizerState(&rasterizer_desc, &_rasterizer_state);
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateRasterizerState failed", hr);
    _context->RSSetState(_rasterizer_state);
    D3D11_SAMPLER_DESC sampler_desc{};
    sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    hr = _device->CreateSamplerState(&sampler_desc, &_sampler_state);
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateSamplerState failed", hr);
    _context->PSSetSamplers(0, 1, &_sampler_state);
    D3D11_DEPTH_STENCIL_DESC depth_stencil_desc{};
    depth_stencil_desc.DepthEnable = TRUE;
    depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depth_stencil_desc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL; // Reverse-Z
    hr = _device->CreateDepthStencilState(&depth_stencil_desc, &_depth_stencil_state);
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateDepthStencilState failed", hr);
    _context->OMSetDepthStencilState(_depth_stencil_state, 0);
    _initialized = true;
    return {};
  }

  void release() {
    if (_blend_state) std::exchange(_blend_state, nullptr)->Release();
    if (_rasterizer_state) std::exchange(_rasterizer_state, nullptr)->Release();
    if (_sampler_state) std::exchange(_sampler_state, nullptr)->Release();
    if (_depth_stencil_state) std::exchange(_depth_stencil_state, nullptr)->Release();
    if (_context) std::exchange(_context, nullptr)->Release();
    if (_device) std::exchange(_device, nullptr)->Release();
    _initialized = false;
  }

  ::ID3D11Device* device() { return _device; }
  ::ID3D11DeviceContext* context() { return _context; }
  ::ID3D11BlendState* blend_state() { return _blend_state; }
  ::ID3D11RasterizerState* rasterizer_state() { return _rasterizer_state; }
  ::ID3D11SamplerState* sampler_state() { return _sampler_state; }
  ::ID3D11DepthStencilState* depth_stencil_state() { return _depth_stencil_state; }
} d3d;

//////////////////////////////////////// MARK: dxgi

inline class {
  ::IDXGIFactory2* _factory{nullptr};
  ::IDXGIDevice2* _device{nullptr};
  bool _initialized{false};

public:
  std::expected<void, error_trace> initialize() {
    if (_initialized) return {};
    if (auto res = d3d.initialize(); !res) return unexpected_error(res.error());
    auto hr = ::CreateDXGIFactory2(0, __uuidof(IDXGIFactory2), reinterpret_cast<void**>(&_factory));
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateDXGIFactory2 failed", hr);
    hr = d3d.device()->QueryInterface(__uuidof(IDXGIDevice2), reinterpret_cast<void**>(&_device));
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateDevice failed", hr);
    _initialized = true;
    return {};
  }

  void release() {
    if (_device) std::exchange(_device, nullptr)->Release();
    if (_factory) std::exchange(_factory, nullptr)->Release();
    _initialized = false;
  }

  ::IDXGIFactory2* factory() { return _factory; }
  ::IDXGIDevice2* device() { return _device; }
} dxgi;

//////////////////////////////////////// MARK: coinit

inline class {
  bool _initialized{false};

public:
  std::expected<void, error_trace> initialize() {
    if (_initialized) return {};
    auto hr = ::CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CoInitializeEx failed", hr);
    _initialized = true;
    return {};
  }

  void release() {
    if (!_initialized) return;
    ::CoUninitialize();
    _initialized = false;
  }
} coinit;

//////////////////////////////////////// MARK: d2d

inline class {
  ::ID2D1Factory1* _factory{nullptr};
  ::ID2D1Device* _device{nullptr};
  ::ID2D1DeviceContext* _context{nullptr};
  ::ID2D1SolidColorBrush* _solid_brush{nullptr};
  ::ID2D1StrokeStyle* _stroke_style{nullptr};
  bool _initialized{false};

public:
  std::expected<void, error_trace> initialize() {
    if (_initialized) return {};
    if (auto res = dxgi.initialize(); !res) return unexpected_error(res.error());
    if (auto res = coinit.initialize(); !res) return unexpected_error(res.error());
    auto hr = ::D2D1CreateFactory(
      D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory1), reinterpret_cast<void**>(&_factory));
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "D2D1CreateFactory failed", hr);
    hr = _factory->CreateDevice(dxgi.device(), &_device);
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateDevice failed", hr);
    hr = _device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &_context);
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateDeviceContext failed", hr);
    hr = _context->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &_solid_brush);
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateSolidColorBrush failed", hr);
    D2D1_STROKE_STYLE_PROPERTIES stroke_style_props{};
    stroke_style_props.startCap = D2D1_CAP_STYLE_ROUND;
    stroke_style_props.endCap = D2D1_CAP_STYLE_ROUND;
    stroke_style_props.dashCap = D2D1_CAP_STYLE_ROUND;
    stroke_style_props.lineJoin = D2D1_LINE_JOIN_ROUND;
    stroke_style_props.miterLimit = 10.0f;
    hr = _factory->CreateStrokeStyle(&stroke_style_props, nullptr, 0, &_stroke_style);
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateStrokeStyle failed", hr);
    _initialized = true;
    return {};
  }

  void release() {
    if (_stroke_style) std::exchange(_stroke_style, nullptr)->Release();
    if (_solid_brush) std::exchange(_solid_brush, nullptr)->Release();
    if (_context) std::exchange(_context, nullptr)->Release();
    if (_device) std::exchange(_device, nullptr)->Release();
    if (_factory) std::exchange(_factory, nullptr)->Release();
    _initialized = false;
  }

  ::ID2D1Factory1* factory() { return _factory; }
  ::ID2D1Device* device() { return _device; }
  ::ID2D1DeviceContext* context() { return _context; }
  ::ID2D1SolidColorBrush* solid_brush() { return _solid_brush; }
  ::ID2D1StrokeStyle* stroke_style() { return _stroke_style; }
} d2d;

//////////////////////////////////////// MARK: dwrite

inline class {
  ::IDWriteFactory1* _factory = nullptr;
  ::IDWriteTextFormat* _text_format = nullptr;
  bool _initialized{false};

public:
  std::expected<void, error_trace> initialize() {
    if (_initialized) return {};
    if (auto res = d2d.initialize(); !res) return unexpected_error(res.error());
    auto hr = ::DWriteCreateFactory(
      DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory1), reinterpret_cast<IUnknown**>(&_factory));
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "DWriteCreateFactory failed", hr);
    hr = _factory->CreateTextFormat(L"", nullptr, DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE_NORMAL,
      DWRITE_FONT_STRETCH_NORMAL, 16.0f, L"", &_text_format);
    _initialized = true;
    return {};
  }

  void release() {
    if (_text_format) std::exchange(_text_format, nullptr)->Release();
    if (_factory) std::exchange(_factory, nullptr)->Release();
    _initialized = false;
  }

  ::IDWriteFactory1* factory() { return _factory; }
  ::IDWriteTextFormat* text_format() { return _text_format; }
} dwrite;

///////////////////////////////////////// MARK: wic

inline class {
  ::IWICImagingFactory2* _factory{nullptr};
  bool _initialized{false};

public:
  std::expected<void, error_trace> initialize() {
    if (_initialized) return {};
    if (auto res = d2d.initialize(); !res) return unexpected_error(res.error());
    auto hr = ::CoCreateInstance(CLSID_WICImagingFactory2, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&_factory));
    if (FAILED(hr))
      return unexpected_error(errors::operation_failed, "CoCreateInstance for WICImagingFactory2 failed", hr);
    _initialized = true;
    return {};
  }

  void release() {
    if (_factory) std::exchange(_factory, nullptr)->Release();
    _initialized = false;
  }

  ::IWICImagingFactory2* factory() { return _factory; }
} wic;

//////////////////////////////////////// MARK: xaudio2

inline class {
  ::IXAudio2* _xaudio2{nullptr};
  ::IXAudio2MasteringVoice* _mastering_voice{nullptr};
  bool _initialized{false};

  bool _initialize_error(const char* msg) {
    std::print("xaudio2::initialize: {}\n", msg);
    return false;
  }

public:
  std::expected<void, error_trace> initialize() {
    if (_initialized) return {};
    if (auto res = coinit.initialize(); !res) return unexpected_error(res.error());
    auto hr = ::XAudio2Create(&_xaudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "XAudio2Create failed", hr);
    hr = _xaudio2->CreateMasteringVoice(&_mastering_voice);
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateMasteringVoice failed", hr);
    _initialized = true;
    return {};
  }

  void release() {
    if (_mastering_voice) { std::exchange(_mastering_voice, nullptr)->DestroyVoice(); }
    if (_xaudio2) std::exchange(_xaudio2, nullptr)->Release();
    _initialized = false;
  }

  ::IXAudio2* device() { return _xaudio2; }
  ::IXAudio2MasteringVoice* mastering_voice() { return _mastering_voice; }
} xaudio2;

//////////////////////////////////////// MARK: drawing

class drawing {
  inline static std::variant<std::monostate, ID2D1Image*, ID3D11RenderTargetView*> _rendertarget{};
  source _source;
  drawing(const source& src) : _source(src) {}
public:
  ~drawing() {
    if (dimension() == 2) {
      if (auto hr = d2d.context()->EndDraw(); FAILED(hr))
        print_fallback("drawing failed (code={}) that starts at {}", hr, _source);
      d2d.context()->SetTarget(nullptr);
    } else if (dimension() == 3) {
      // nothing to do yet
    }
    _rendertarget = std::monostate{};
  }

  static std::expected<drawing, error_trace> create(ID2D1Image* rendertarget, const source& src) {
    if (_rendertarget.index() != 0) return unexpected_error(errors::invalid_operation, "rendertarget already set");
    if (rendertarget == nullptr) return unexpected_error(errors::invalid_argument, "null rendertarget");
    if (auto res = d2d.initialize(); !res) return unexpected_error(res.error());
    _rendertarget = rendertarget;
    d2d.context()->SetTarget(rendertarget);
    d2d.context()->BeginDraw();
    return drawing{src};
  }
  static std::expected<drawing, error_trace> create(ID3D11RenderTargetView* rendertarget, const source& src) {
    if (_rendertarget.index() != 0) return unexpected_error(errors::invalid_operation, "rendertarget already set");
    if (rendertarget == nullptr) return unexpected_error(errors::invalid_argument, "null rendertarget");
    if (auto res = d3d.initialize(); !res) return unexpected_error(res.error());
    // nothing to do yet
    return drawing{src};
  }

  static uint32_t dimension() { return uint32_t(_rendertarget.index() ? _rendertarget.index() + 1 : 0); }
};

} // namespace yw
