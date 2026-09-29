#pragma once
#include <core/color.h>
#include <core/core.h>
#include <core/property.h>
#include <core/result.h>

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>

#include <d3d11_4.h>

#include <d2d1_3.h>

#include <dwrite_3.h>

#include <wincodec.h>

#include <xaudio2.h>

namespace yw {

template<typename Com> class comptr {
  comptr(const comptr&) = delete;
  comptr& operator=(const comptr&) = delete;
  Com* _ptr{nullptr};

public:
  explicit operator bool() const noexcept { return _ptr != nullptr; }
  explicit operator Com*&() & noexcept { return _ptr; }
  explicit operator Com*() const& noexcept { return _ptr; }
  Com* operator->() const noexcept { return _ptr; }
  bool operator==(Com* Other) const noexcept { return _ptr == Other; }
  ~comptr() {
    if (_ptr) _ptr->Release();
    _ptr = nullptr;
  }
  comptr() noexcept = default;
  comptr(comptr&& Other) noexcept : _ptr(std::exchange(Other._ptr, nullptr)) {}

  comptr& operator=(comptr&& Other) {
    if (this == &Other) return *this;
    if (_ptr) _ptr->Release();
    _ptr = std::exchange(Other._ptr, nullptr);
    return *this;
  }

  Com*& get() & noexcept { return _ptr; }
  Com* get() const& noexcept { return _ptr; }

  void release() {
    if (_ptr) _ptr->Release();
    _ptr = nullptr;
  }

  void reset(Com* New) noexcept {
    if (_ptr) _ptr->Release();
    _ptr = New;
  }
};

///--------------------------------------------------------------------------///
/// MARK: d3d

class d3d {
  inline static ID3D11Device* _device = nullptr;
  inline static ID3D11DeviceContext* _context = nullptr;
  inline static ID3D11RasterizerState* _rasterizer_state = nullptr;
  inline static bool _initialized = false;

public:
  static ID3D11Device* device() {
    if (auto res = initialize(); !res) res.error().print_and_abort();
    return _device;
  }
  static ID3D11DeviceContext* context() {
    if (auto res = initialize(); !res) res.error().print_and_abort();
    return _context;
  }
  static ID3D11RasterizerState* rasterizer_state() {
    if (auto res = initialize(); !res) res.error().print_and_abort();
    return _rasterizer_state;
  }
  static result<void> initialize() {
    if (_initialized) return {}; // Already initialized
    {
      if (_device) return error(errors::invalid_operation, "D3DDevice is already initialized");
      if (_context) return error(errors::invalid_operation, "D3DContext is already initialized");
      const D3D_FEATURE_LEVEL _levels[] = {D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0};
      const auto hr = ::D3D11CreateDevice(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT, _levels, _countof(_levels),
        D3D11_SDK_VERSION, &_device, nullptr, &_context);
      if (FAILED(hr)) return error(errors::operation_failed, "D3D11CreateDevice failed");
    }
    {
      if (_rasterizer_state) return error(errors::invalid_operation, "RasterizerState is already initialized");
      D3D11_RASTERIZER_DESC rasterizer_desc{};
      rasterizer_desc.FillMode = D3D11_FILL_SOLID;
      rasterizer_desc.CullMode = D3D11_CULL_NONE;
      rasterizer_desc.FrontCounterClockwise = TRUE;
      rasterizer_desc.DepthClipEnable = TRUE;
      const auto hr = _device->CreateRasterizerState(&rasterizer_desc, &_rasterizer_state);
      if (FAILED(hr)) return error(errors::operation_failed, "CreateRasterizerState failed");
      _context->RSSetState(_rasterizer_state);
    }
    _initialized = true;
    return {};
  }
  static void release() {
    if (_rasterizer_state) {
      _rasterizer_state->Release();
      _rasterizer_state = nullptr;
    }
    if (_context) {
      _context->Release();
      _context = nullptr;
    }
    if (_device) {
      _device->Release();
      _device = nullptr;
    }
    _initialized = false;
  }
};

///--------------------------------------------------------------------------///
/// MARK: dxgi

class dxgi {
  inline static IDXGIFactory2* _factory = nullptr;
  inline static IDXGIDevice2* _device = nullptr;
  inline static bool _initialized = false;

public:
  static IDXGIFactory2* factory() {
    if (auto res = initialize(); !res) res.error().print_and_abort();
    return _factory;
  }
  static IDXGIDevice2* device() {
    if (auto res = initialize(); !res) res.error().print_and_abort();
    return _device;
  }
  static result<void> initialize() {
    if (_initialized) return {};
    {
      if (_factory) return error(errors::invalid_operation, "DXGI Factory is already initialized");
      const auto hr = ::CreateDXGIFactory2(0, __uuidof(IDXGIFactory2), reinterpret_cast<void**>(&_factory));
      if (FAILED(hr)) return error(errors::operation_failed, "CreateDXGIFactory2 failed");
    }
    {
      if (_device) return error(errors::invalid_operation, "DXGI Device is already initialized");
      const auto hr = d3d::device()->QueryInterface(__uuidof(IDXGIDevice2), reinterpret_cast<void**>(&_device));
      if (FAILED(hr)) return error(errors::operation_failed, "QueryInterface for IDXGIDevice2 failed");
    }
    _initialized = true;
    return {};
  }
  static void release() {
    if (_device) {
      _device->Release();
      _device = nullptr;
    }
    if (_factory) {
      _factory->Release();
      _factory = nullptr;
    }
    _initialized = false;
  }
};

///--------------------------------------------------------------------------///
/// MARK: d2d

class d2d {
  inline static ID2D1Factory1* _factory = nullptr;
  inline static ID2D1Device* _device = nullptr;
  inline static ID2D1DeviceContext* _context = nullptr;
  inline static ID2D1SolidColorBrush* _solid_color_brush = nullptr;
  inline static ID2D1StrokeStyle* _stroke_style = nullptr;
  inline static bool _initialized = false;

public:
  static ID2D1Factory1* factory() {
    if (auto res = initialize(); !res) res.error().print_and_abort();
    return _factory;
  }
  static ID2D1Device* device() {
    if (auto res = initialize(); !res) res.error().print_and_abort();
    return _device;
  }
  static ID2D1DeviceContext* context() {
    if (auto res = initialize(); !res) res.error().print_and_abort();
    return _context;
  }
  static ID2D1SolidColorBrush* solid_color_brush() {
    if (auto res = initialize(); !res) res.error().print_and_abort();
    return _solid_color_brush;
  }
  static ID2D1StrokeStyle* stroke_style() {
    if (auto res = initialize(); !res) res.error().print_and_abort();
    return _stroke_style;
  }
  static void push_layer(ID2D1Geometry* g) {
    context()->PushLayer(D2D1::LayerParameters1(D2D1::InfiniteRect(), g), nullptr);
  }
  static void pop_layer() { context()->PopLayer(); }
  static void set_solid_color(const yw::color& c) {
    if (auto res = initialize(); !res) res.error().print_and_abort();
    _solid_color_brush->SetColor(reinterpret_cast<const D2D1_COLOR_F*>(&c));
  }
  static result<void> initialize() {
    if (_initialized) return {};
    {
      if (_factory) return error(errors::invalid_operation, "D2D Factory is already initialized");
      const auto hr = ::D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &_factory);
      if (FAILED(hr)) return error(errors::operation_failed, "D2D1CreateFactory failed");
    }
    {
      if (_device) return error(errors::invalid_operation, "D2D Device is already initialized");
      const auto hr = dxgi::device()->QueryInterface(__uuidof(ID2D1Device), reinterpret_cast<void**>(&_device));
      if (FAILED(hr)) return error(errors::operation_failed, "QueryInterface for ID2D1Device failed");
    }
    {
      if (_context) return error(errors::invalid_operation, "D2D Device Context is already initialized");
      _device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &_context);
      if (!_context) return error(errors::operation_failed, "CreateDeviceContext failed");
    }
    {
      if (_solid_color_brush) return error(errors::invalid_operation, "D2D Solid Color Brush is already initialized");
      const auto hr = _context->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &_solid_color_brush);
      if (FAILED(hr)) return error(errors::operation_failed, "CreateSolidColorBrush failed");
    }
    {
      if (_stroke_style) return error(errors::invalid_operation, "D2D Stroke Style is already initialized");
      D2D1_STROKE_STYLE_PROPERTIES props{
        .startCap = D2D1_CAP_STYLE_ROUND,
        .endCap = D2D1_CAP_STYLE_ROUND,
        .dashCap = D2D1_CAP_STYLE_ROUND,
        .lineJoin = D2D1_LINE_JOIN_ROUND,
        .miterLimit = 10.0f};
      const auto hr = _factory->CreateStrokeStyle(&props, nullptr, 0, &_stroke_style);
      if (FAILED(hr)) return error(errors::operation_failed, "CreateStrokeStyle failed");
    }
    _initialized = true;
    return {};
  }
  static void release() {
    if (_stroke_style) {
      _stroke_style->Release();
      _stroke_style = nullptr;
    }
    if (_solid_color_brush) {
      _solid_color_brush->Release();
      _solid_color_brush = nullptr;
    }
    if (_context) {
      _context->Release();
      _context = nullptr;
    }
    if (_device) {
      _device->Release();
      _device = nullptr;
    }
    if (_factory) {
      _factory->Release();
      _factory = nullptr;
    }
    _initialized = false;
  }
};

///--------------------------------------------------------------------------///
/// MARK: dwrite

class dwrite {
  inline static IDWriteFactory1* _factory = nullptr;
  inline static IDWriteTextFormat* _text_format = nullptr;
  inline static bool _initialized = false;

public:
  static IDWriteFactory1* factory() {
    if (auto res = initialize(); !res) res.error().print_and_abort();
    return _factory;
  }
  static IDWriteTextFormat* text_format() {
    if (auto res = initialize(); !res) res.error().print_and_abort();
    return _text_format;
  }
  static result<void> initialize() {
    if (_initialized) return {};
    {
      if (_factory) return error(errors::invalid_operation, "D2D Factory is already initialized");
      const auto hr = ::DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory1), reinterpret_cast<IUnknown**>(&_factory));
      if (FAILED(hr)) return error(errors::operation_failed, "DWriteCreateFactory failed");
    }
    {
      if (_text_format) return error(errors::invalid_operation, "D2D Text Format is already initialized");
      const auto hr = _factory->CreateTextFormat(
        L"", nullptr, DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 16.0f, L"",
        &_text_format);
      if (FAILED(hr)) return error(errors::operation_failed, "CreateTextFormat failed");
    }
    _initialized = true;
    return {};
  }
  static void release() {
    if (_text_format) {
      _text_format->Release();
      _text_format = nullptr;
    }
    if (_factory) {
      _factory->Release();
      _factory = nullptr;
    }
    _initialized = false;
  }
};

///--------------------------------------------------------------------------///
/// MARK: com

class com {
  inline static IWICImagingFactory2* _wic = nullptr;
  inline static IXAudio2* _xaudio2 = nullptr;
  inline static bool _initialized = false;

public:
  static IWICImagingFactory2* wic() {
    if (auto res = initialize(); !res) res.error().print_and_abort();
    return _wic;
  }
  static IXAudio2* xaudio2() {
    if (auto res = initialize(); !res) res.error().print_and_abort();
    return _xaudio2;
  }
  static result<void> initialize() {
    if (_initialized) return {};
    {
      const auto hr = ::CoInitializeEx(nullptr, COINIT_MULTITHREADED);
      if (FAILED(hr)) return error(errors::operation_failed, "CoInitializeEx failed");
    }
    {
      if (_wic) return error(errors::invalid_operation, "WIC Imaging Factory is already initialized");
      const auto hr = ::CoCreateInstance(CLSID_WICImagingFactory2, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&_wic));
      if (FAILED(hr)) return error(errors::operation_failed, "CoCreateInstance for WIC Imaging Factory failed");
    }
    {
      if (_xaudio2) return error(errors::invalid_operation, "XAudio2 is already initialized");
      const auto hr = ::XAudio2Create(&_xaudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
      if (FAILED(hr)) return error(errors::operation_failed, "XAudio2Create failed");
    }
    _initialized = true;
    return {};
  }
  static void release() {
    if (_xaudio2) {
      _xaudio2->Release();
      _xaudio2 = nullptr;
    }
    if (_wic) {
      _wic->Release();
      _wic = nullptr;
    }
    if (_initialized) ::CoUninitialize();
    _initialized = false;
  }
};

} // namespace yw

#endif
