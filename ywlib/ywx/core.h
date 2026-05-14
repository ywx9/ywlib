#pragma once

#include "ywlib"

#include <d2d1_1.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <dwrite_1.h>
#include <dxgi1_3.h>
#include <imm.h>
#include <wincodec.h>
#include <xaudio2.h>

namespace yw {

//////////////////////////////////////// MARK: ok/yes

inline bool ok(
  null_terminated<wchar_t> Text, null_terminated<wchar_t> Title = L"Confirmation", bool topmost = false,
  bool modal = false) {
  UINT flags = MB_OK;
  if (topmost) flags |= MB_TOPMOST;
  if (modal) flags |= MB_TASKMODAL;
  return ::MessageBoxW(nullptr, Text.data(), Title.data(), flags) == IDOK;
}

inline bool yes(
  null_terminated<wchar_t> Text, null_terminated<wchar_t> Title = L"Confirmation", bool topmost = false,
  bool modal = false) {
  UINT flags = MB_YESNO;
  if (topmost) flags |= MB_TOPMOST;
  if (modal) flags |= MB_TASKMODAL;
  return ::MessageBoxW(nullptr, Text.data(), Title.data(), flags) == IDYES;
}

//////////////////////////////////////// MARK: unexpected_win32_error

inline std::unexpected<error_trace> unexpected_win32_error(const char* msg, const source& src = {}) {
  return unexpected_error(errors::operation_failed, msg, int32_t(::GetLastError()), uint64_t(-1), src);
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

/// \note Writes `if (auto res = wclass.initialize(); !res) fatal_error(res.error());` before using.
inline class {
  struct contents {
    HINSTANCE hinstance{};
    const wchar_t* name = L"ywlib_window_class";
    bool initialized = false;
  } c;

public:
  static LRESULT __stdcall proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

  std::expected<void, error_trace> initialize() {
    if (c.initialized) return {};
    c.hinstance = ::GetModuleHandleW(nullptr);
    WNDCLASSW wc{};
    wc.style = CS_DBLCLKS;
    wc.lpfnWndProc = proc;
    wc.hInstance = c.hinstance;
    wc.hCursor = ::LoadCursorW(nullptr, IDC_ARROW);
    wc.lpszClassName = c.name;
    if (!::RegisterClassW(&wc)) return unexpected_win32_error("RegisterClassW failed");
    c.initialized = true;
    return {};
  }

  HINSTANCE hinstance() const noexcept { return c.hinstance; }
  const wchar_t* name() const noexcept { return c.name; }
} wclass;

//////////////////////////////////////// MARK: comptr

template<typename Com> class comptr {
  comptr(const comptr&) = delete;
  comptr& operator=(const comptr&) = delete;
  Com* p{nullptr};

public:
  explicit operator bool() const { return p != nullptr; }
  Com* operator->() const { return p; }
  bool operator==(Com* other) const { return p == other; }
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

  /// releases the current COM pointer and takes ownership of the new pointer
  void reset(Com* ptr) noexcept {
    if (p) p->Release();
    p = ptr;
  }

  explicit operator Com*&() & { return p; }
  explicit operator Com*() const& { return p; }
};

//////////////////////////////////////// MARK: d3d

/// \note Writes `if (auto res = d3d.initialize(); !res) fatal_error(res.error());` before using.
inline class {
  struct contents {
    ID3D11Device* device{};
    ID3D11DeviceContext* context{};
    ID3D11BlendState* blend_state{};
    ID3D11RasterizerState* rasterizer_state{};
    ID3D11SamplerState* sampler_state{};
    ID3D11DepthStencilState* depth_stencil_state{};
    bool initialized{};

    void release() noexcept {
      if (blend_state) blend_state->Release(), blend_state = nullptr;
      if (rasterizer_state) rasterizer_state->Release(), rasterizer_state = nullptr;
      if (sampler_state) sampler_state->Release(), sampler_state = nullptr;
      if (depth_stencil_state) depth_stencil_state->Release(), depth_stencil_state = nullptr;
      if (context) context->Release(), context = nullptr;
      if (device) device->Release(), device = nullptr;
      initialized = false;
    }

    ~contents() noexcept { release(); }
  } c{};

public:
  std::expected<void, error_trace> initialize() {
    if (c.initialized) return {};
    const D3D_FEATURE_LEVEL _levels[] = {D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0};
    auto hr = D3D11CreateDevice(
      nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT, _levels, _countof(_levels),
      D3D11_SDK_VERSION, &c.device, nullptr, &c.context);
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
    hr = c.device->CreateBlendState(&blend_desc, &c.blend_state);
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateBlendState failed", int32_t(hr));
    c.context->OMSetBlendState(c.blend_state, nullptr, 0xffffffff);
    D3D11_RASTERIZER_DESC rasterizer_desc{};
    rasterizer_desc.FillMode = D3D11_FILL_SOLID;
    rasterizer_desc.CullMode = D3D11_CULL_BACK;
    rasterizer_desc.FrontCounterClockwise = TRUE;
    rasterizer_desc.DepthClipEnable = TRUE;
    hr = c.device->CreateRasterizerState(&rasterizer_desc, &c.rasterizer_state);
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateRasterizerState failed", int32_t(hr));
    c.context->RSSetState(c.rasterizer_state);
    D3D11_SAMPLER_DESC sampler_desc{};
    sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    hr = c.device->CreateSamplerState(&sampler_desc, &c.sampler_state);
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateSamplerState failed", int32_t(hr));
    c.context->PSSetSamplers(0, 1, &c.sampler_state);
    D3D11_DEPTH_STENCIL_DESC depth_stencil_desc{};
    depth_stencil_desc.DepthEnable = TRUE;
    depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depth_stencil_desc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL; // Reverse-Z
    hr = c.device->CreateDepthStencilState(&depth_stencil_desc, &c.depth_stencil_state);
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateDepthStencilState failed", int32_t(hr));
    c.context->OMSetDepthStencilState(c.depth_stencil_state, 0);
    c.initialized = true;
    return {};
  }

  void release() noexcept { c.release(); }

  ID3D11Device* device() noexcept { return c.device; }
  ID3D11DeviceContext* context() noexcept { return c.context; }
  ID3D11BlendState* blend_state() noexcept { return c.blend_state; }
  ID3D11RasterizerState* rasterizer_state() noexcept { return c.rasterizer_state; }
  ID3D11SamplerState* sampler_state() noexcept { return c.sampler_state; }
  ID3D11DepthStencilState* depth_stencil_state() noexcept { return c.depth_stencil_state; }
} d3d;

//////////////////////////////////////// MARK: dxgi

/// \note Writes `if (auto res = dxgi.initialize(); !res) fatal_error(res.error());` before using.
inline class {
  struct contents {
    IDXGIFactory2* factory{nullptr};
    IDXGIDevice2* device{nullptr};
    bool initialized{false};

    void release() noexcept {
      if (device) device->Release(), device = nullptr;
      if (factory) factory->Release(), factory = nullptr;
      initialized = false;
    }

    ~contents() noexcept { release(); }
  } c{};

public:
  std::expected<void, error_trace> initialize() {
    if (c.initialized) return {};
    if (auto res = d3d.initialize(); !res) return unexpected_error(res.error());
    auto hr = ::CreateDXGIFactory2(0, __uuidof(IDXGIFactory2), reinterpret_cast<void**>(&c.factory));
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateDXGIFactory2 failed", int32_t(hr));
    hr = d3d.device()->QueryInterface(__uuidof(IDXGIDevice2), reinterpret_cast<void**>(&c.device));
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateDevice failed", int32_t(hr));
    c.initialized = true;
    return {};
  }

  void release() noexcept { c.release(); }

  IDXGIFactory2* factory() noexcept { return c.factory; }
  IDXGIDevice2* device() noexcept { return c.device; }
} dxgi;

//////////////////////////////////////// MARK: coinit

inline class {
  struct contents {
    bool initialized{};

    void release() noexcept {
      if (initialized) ::CoUninitialize();
      initialized = false;
    }

    ~contents() noexcept { release(); }
  } c{};

public:
  std::expected<void, error_trace> initialize() {
    if (c.initialized) return {};
    if (auto hr = ::CoInitializeEx(nullptr, COINIT_MULTITHREADED); FAILED(hr))
      return unexpected_error(errors::operation_failed, "CoInitializeEx failed", int32_t(hr));
    c.initialized = true;
    return {};
  }

  void release() noexcept { c.release(); }
} coinit;

//////////////////////////////////////// MARK: d2d

/// \note Writes `if (auto res = d2d.initialize(); !res) fatal_error(res.error());` before using.
inline class {
  struct contents {
    ID2D1Factory1* factory{};
    ID2D1Device* device{};
    ID2D1DeviceContext* context{};
    bool initialized{};

    void release() noexcept {
      if (context) context->Release(), context = nullptr;
      if (device) device->Release(), device = nullptr;
      if (factory) factory->Release(), factory = nullptr;
      initialized = false;
    }

    ~contents() noexcept { release(); }
  } c{};

public:
  std::expected<void, error_trace> initialize() {
    if (c.initialized) return {};
    if (auto res = dxgi.initialize(); !res) return unexpected_error(res.error());
    if (auto res = coinit.initialize(); !res) return unexpected_error(res.error());
    auto hr = ::D2D1CreateFactory(
      D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory1), reinterpret_cast<void**>(&c.factory));
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "D2D1CreateFactory failed", int32_t(hr));
    hr = c.factory->CreateDevice(dxgi.device(), &c.device);
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateDevice failed", int32_t(hr));
    hr = c.device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &c.context);
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateDeviceContext failed", int32_t(hr));
    c.initialized = true;
    return {};
  }

  void release() noexcept { c.release(); }

  ID2D1Factory1* factory() noexcept { return c.factory; }
  ID2D1Device* device() noexcept { return c.device; }
  ID2D1DeviceContext* context() noexcept { return c.context; }

  std::expected<void, error_trace> push_layer(ID2D1Geometry* Geometry) {
    if (auto res = initialize(); !res) return unexpected_error(res.error());
    context()->PushLayer(D2D1::LayerParameters1(D2D1::InfiniteRect(), Geometry), nullptr);
    return {};
  }

  std::expected<void, error_trace> pop_layer() {
    if (auto res = initialize(); !res) return unexpected_error(res.error());
    context()->PopLayer();
    return {};
  }
} d2d;

//////////////////////////////////////// MARK: brush

/// \note Writes `if (auto res = brush.initialize(); !res) fatal_error(res.error());` before using.
inline class {
  struct contents {
    ID2D1SolidColorBrush* solid_brush{};
    ID2D1StrokeStyle* stroke_style{};
    ID2D1StrokeStyle* dashed_stroke_style{};
    bool dashed = false;
    bool initialized = false;

    void release() noexcept {
      if (dashed_stroke_style) dashed_stroke_style->Release(), dashed_stroke_style = nullptr;
      if (stroke_style) stroke_style->Release(), stroke_style = nullptr;
      if (solid_brush) solid_brush->Release(), solid_brush = nullptr;
      initialized = false;
    }

    ~contents() noexcept { release(); }
  } c{};

public:
  /// initializes brush if it hasn't been initialized yet.
  std::expected<void, error_trace> initialize() {
    if (c.initialized) return {};
    if (auto res = d2d.initialize(); !res) return unexpected_error(res.error());
    if (auto hr = d2d.context()->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &c.solid_brush); FAILED(hr))
      return unexpected_error(errors::operation_failed, "CreateSolidColorBrush failed", int32_t(hr));
    D2D1_STROKE_STYLE_PROPERTIES stroke_style_props{};
    stroke_style_props.startCap = D2D1_CAP_STYLE_ROUND;
    stroke_style_props.endCap = D2D1_CAP_STYLE_ROUND;
    stroke_style_props.dashCap = D2D1_CAP_STYLE_ROUND;
    stroke_style_props.lineJoin = D2D1_LINE_JOIN_ROUND;
    stroke_style_props.miterLimit = 10.0f;
    if (auto hr = d2d.factory()->CreateStrokeStyle(&stroke_style_props, nullptr, 0, &c.stroke_style); FAILED(hr))
      return unexpected_error(errors::operation_failed, "CreateStrokeStyle failed", int32_t(hr));
    stroke_style_props.dashStyle = D2D1_DASH_STYLE_DASH;
    if (auto hr = d2d.factory()->CreateStrokeStyle(&stroke_style_props, nullptr, 0, &c.dashed_stroke_style); FAILED(hr))
      return unexpected_error(errors::operation_failed, "CreateStrokeStyle (dashed) failed", int32_t(hr));
    c.initialized = true;
    return {};
  }

  void release() noexcept { c.release(); }

  yw::color color() {
    if (auto res = initialize(); !res) fatal_error(res.error());
    return std::bit_cast<yw::color>(c.solid_brush->GetColor());
  }
  void color(const yw::color& Color) {
    if (auto res = initialize(); !res) fatal_error(res.error());
    c.solid_brush->SetColor(reinterpret_cast<const D2D1_COLOR_F*>(&Color));
  }
  bool dashed() { return c.dashed; }

  /// \note Note that the effect of this style change will continue until explicitly switched.
  void dashed(bool Dashed = true) { c.dashed = Dashed; }

  ID2D1SolidColorBrush* d2d_brush() {
    if (auto res = initialize(); !res) fatal_error(res.error());
    return c.solid_brush;
  }
  ID2D1StrokeStyle* d2d_stroke() {
    if (auto res = initialize(); !res) fatal_error(res.error());
    return c.dashed ? c.dashed_stroke_style : c.stroke_style;
  }
} brush;

//////////////////////////////////////// MARK: dwrite

enum class text_alignment : uint8_t {
  left = DWRITE_TEXT_ALIGNMENT_LEADING,
  right = DWRITE_TEXT_ALIGNMENT_TRAILING,
  center = DWRITE_TEXT_ALIGNMENT_CENTER,
  justified = DWRITE_TEXT_ALIGNMENT_JUSTIFIED
};

enum class font_weight : uint16_t {
  thin = DWRITE_FONT_WEIGHT_THIN,
  extra_light = DWRITE_FONT_WEIGHT_EXTRA_LIGHT,
  ultra_light = DWRITE_FONT_WEIGHT_ULTRA_LIGHT,
  light = DWRITE_FONT_WEIGHT_LIGHT,
  semi_light = DWRITE_FONT_WEIGHT_SEMI_LIGHT,
  normal = DWRITE_FONT_WEIGHT_NORMAL,
  regular = DWRITE_FONT_WEIGHT_REGULAR,
  medium = DWRITE_FONT_WEIGHT_MEDIUM,
  demi_bold = DWRITE_FONT_WEIGHT_DEMI_BOLD,
  semi_bold = DWRITE_FONT_WEIGHT_SEMI_BOLD,
  bold = DWRITE_FONT_WEIGHT_BOLD,
  extra_bold = DWRITE_FONT_WEIGHT_EXTRA_BOLD,
  ultra_bold = DWRITE_FONT_WEIGHT_ULTRA_BOLD,
  black = DWRITE_FONT_WEIGHT_BLACK,
  heavy = DWRITE_FONT_WEIGHT_HEAVY,
  extra_black = DWRITE_FONT_WEIGHT_EXTRA_BLACK,
  ultra_black = DWRITE_FONT_WEIGHT_ULTRA_BLACK
};

enum class font_style : uint8_t {
  normal = DWRITE_FONT_STYLE_NORMAL,
  italic = DWRITE_FONT_STYLE_ITALIC,
  oblique = DWRITE_FONT_STYLE_OBLIQUE
};

enum class font_stretch : uint8_t {
  undefined = DWRITE_FONT_STRETCH_UNDEFINED,
  ultra_condensed = DWRITE_FONT_STRETCH_ULTRA_CONDENSED,
  extra_condensed = DWRITE_FONT_STRETCH_EXTRA_CONDENSED,
  condensed = DWRITE_FONT_STRETCH_CONDENSED,
  semi_condensed = DWRITE_FONT_STRETCH_SEMI_CONDENSED,
  normal = DWRITE_FONT_STRETCH_NORMAL,
  medium = DWRITE_FONT_STRETCH_MEDIUM,
  semi_expanded = DWRITE_FONT_STRETCH_SEMI_EXPANDED,
  expanded = DWRITE_FONT_STRETCH_EXPANDED,
  extra_expanded = DWRITE_FONT_STRETCH_EXTRA_EXPANDED,
  ultra_expanded = DWRITE_FONT_STRETCH_ULTRA_EXPANDED
};

struct font_config {
  std::optional<std::wstring> name = std::nullopt;
  std::optional<float> size = std::nullopt;
  std::optional<font_weight> weight = std::nullopt;
  std::optional<font_style> style = std::nullopt;
  std::optional<font_stretch> stretch = std::nullopt;

  static const font_config default_;
};

inline const font_config font_config::default_{
  L""s, 16.0f, font_weight::normal, font_style::normal, font_stretch::normal};

/// \note Writes `if (auto res = dwrite.initialize(); !res) fatal_error(res.error())` before using.
inline class {
  struct contents {
    IDWriteFactory1* factory{};
    IDWriteTextFormat* text_format{};
    bool initialized = false;

    void release() noexcept {
      if (text_format) std::exchange(text_format, nullptr)->Release();
      if (factory) std::exchange(factory, nullptr)->Release();
      initialized = false;
    }

    ~contents() noexcept { release(); }
  } c{};

public:
  std::expected<void, error_trace> initialize() {
    if (c.initialized) return {};
    if (auto res = d2d.initialize(); !res) return unexpected_error(res.error());
    // Creates DirectWrite factory.
    auto hr = ::DWriteCreateFactory(
      DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory1), reinterpret_cast<IUnknown**>(&c.factory));
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "DWriteCreateFactory failed", int32_t(hr));
    // Creates default text format.
    hr = c.factory->CreateTextFormat( //
      font_config::default_.name->c_str(), nullptr, //
      DWRITE_FONT_WEIGHT(*font_config::default_.weight), //
      DWRITE_FONT_STYLE(*font_config::default_.style), //
      DWRITE_FONT_STRETCH(*font_config::default_.stretch), //
      font_config::default_.size.value_or(16.0f), L"", &c.text_format);
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateTextFormat failed", int32_t(hr));
    c.text_format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    c.initialized = true;
    return {};
  }

  void release() noexcept { c.release(); }

  IDWriteFactory1* factory() { return c.factory; }
  IDWriteTextFormat* text_format() { return c.text_format; }
} dwrite;

///////////////////////////////////////// MARK: wic

/// \note Writes `if (auto res = wic.initialize(); !res) fatal_error(res.error())` before using.
inline class {
  struct contents {
    ::IWICImagingFactory2* factory{};
    bool initialized{};

    void release() noexcept {
      if (factory) std::exchange(factory, nullptr)->Release();
      initialized = false;
    }

    ~contents() noexcept { release(); }
  } c{};

public:
  std::expected<void, error_trace> initialize() {
    if (c.initialized) return {};
    if (auto res = d2d.initialize(); !res) return unexpected_error(res.error());
    auto hr = ::CoCreateInstance(CLSID_WICImagingFactory2, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&c.factory));
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CoCreateInstance failed", int32_t(hr));
    c.initialized = true;
    return {};
  }

  void release() noexcept { c.release(); }

  ::IWICImagingFactory2* factory() { return c.factory; }
} wic;

//////////////////////////////////////// MARK: xaudio2

/// \note Writes `if (auto res = xaudio2.initialize(); !res) fatal_error(res.error())` before using.
inline class {
  struct contents {
    ::IXAudio2* xaudio2{};
    ::IXAudio2MasteringVoice* mastering_voice{};
    bool initialized{};

    void release() noexcept {
      if (mastering_voice) std::exchange(mastering_voice, nullptr)->DestroyVoice();
      if (xaudio2) std::exchange(xaudio2, nullptr)->Release();
      initialized = false;
    }

    ~contents() noexcept { release(); }
  } c{};

public:
  std::expected<void, error_trace> initialize() {
    if (c.initialized) return {};
    if (auto res = coinit.initialize(); !res) return unexpected_error(res.error());
    auto hr = ::XAudio2Create(&c.xaudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "XAudio2Create failed", int32_t(hr));
    hr = c.xaudio2->CreateMasteringVoice(&c.mastering_voice);
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateMasteringVoice failed", int32_t(hr));
    c.initialized = true;
    return {};
  }

  void release() noexcept { c.release(); }

  ::IXAudio2* device() { return c.xaudio2; }
  ::IXAudio2MasteringVoice* mastering_voice() { return c.mastering_voice; }
} xaudio2;
} // namespace yw
