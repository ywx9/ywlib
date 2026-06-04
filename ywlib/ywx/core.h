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

namespace system {
inline LRESULT __stdcall wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
}

/// MARK: ok/yes

inline bool ok(
  null_terminated<wchar_t> Text, null_terminated<wchar_t> Title = L"Confirmation", bool topmost = false,
  bool modal = false) {
  UINT flags = MB_OK | (topmost ? MB_TOPMOST : 0) | (modal ? MB_TASKMODAL : 0);
  return ::MessageBoxW(nullptr, Text.data(), Title.data(), flags) == IDOK;
}

inline bool yes(
  null_terminated<wchar_t> Text, null_terminated<wchar_t> Title = L"Confirmation", bool topmost = false,
  bool modal = false) {
  UINT flags = MB_YESNO | (topmost ? MB_TOPMOST : 0) | (modal ? MB_TASKMODAL : 0);
  return ::MessageBoxW(nullptr, Text.data(), Title.data(), flags) == IDYES;
}

/// MARK: unexpected_win32_error

inline std::unexpected<error_trace> unexpected_win32_error(const char* msg, const source& src = {}) {
  return unexpected_error(errors::operation_failed, msg, int32_t(::GetLastError()), uint64_t(-1), src);
}

/// MARK: desktop_client_size

inline uint2 desktop_client_size() {
  if (RECT r; !::GetClientRect(::GetDesktopWindow(), &r)) return uint2();
  else return uint2(r.right, r.bottom);
}

/// MARK: unknown

class unknown {
public:
  struct slot {
    slotset<slot>::slotid id{};
    virtual ~slot() noexcept {}
  };

protected:
  slotset<slot>::slotid _id{};
  unknown() noexcept = default;
  unknown(slotset<slot>::slotid Id) : _id(Id) {}

public:
  unknown(unknown&& Other) noexcept : _id(std::move(Other._id)) {}
  unknown& operator=(unknown&& Other) noexcept {
    if (this == &Other) _id = std::exchange(Other._id, {});
    return *this;
  }
  explicit operator bool() const noexcept;
  auto id() const noexcept { return _id; }
};

using unknown_slotid = slotset<unknown::slot>::slotid;

namespace system {
inline slotset<unknown::slot> unknowns;
template<derived_from<unknown> T> typename T::slot* get_slot_pointer(unknown_slotid Id) noexcept {
  return static_cast<T::slot*>(unknowns.get(Id));
}
} // namespace system

inline unknown::operator bool() const noexcept { return system::unknowns.contains(_id); }

/// MARK: wclass

class wclass final : public unknown {
public:
  struct slot : unknown::slot {
    HINSTANCE hinstance{};
    const wchar_t* name = L"ywlib_window_class";

    std::expected<void, error_trace> initialize(unknown_slotid Id) {
      if (system::unknowns.contains(id)) return {};
      hinstance = ::GetModuleHandleW(nullptr);
      WNDCLASSW wc{};
      wc.style = CS_DBLCLKS;
      wc.lpfnWndProc = system::wndproc;
      wc.hInstance = hinstance;
      wc.hCursor = ::LoadCursorW(nullptr, IDC_ARROW);
      wc.lpszClassName = name;
      if (!::RegisterClassW(&wc) || ::GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
        return unexpected_error(errors::operation_failed, "RegisterClassW failed");
      id = Id;
      return {};
    }

    std::expected<void, error_trace> release() {
      id = {};
      ::UnregisterClassW(name, hinstance);
      hinstance = {};
      return {};
    }
  };

  using unknown::operator bool;

  explicit wclass() {
    static unknown_slotid id = {};
    if (!system::unknowns.contains(id)) {
      const auto temp_id = system::unknowns.add(std::make_unique<slot>());
      const auto temp_sp = system::get_slot_pointer<wclass>(temp_id);
      if (!temp_sp) fatal_error(errors::invalid_slotid);
      if (auto res = temp_sp->initialize(temp_id); !res) fatal_error(res.error());
      id = temp_id;
    }
    this->_id = id;
  }

  HINSTANCE hinstance() const noexcept {
    if (auto sp = system::get_slot_pointer<wclass>(_id)) return sp->hinstance;
    return nullptr;
  }
  const wchar_t* name() const noexcept {
    if (auto sp = system::get_slot_pointer<wclass>(_id)) return sp->name;
    return nullptr;
  }

  std::expected<void, error_trace> release() const {
    const auto sp = system::get_slot_pointer<wclass>(_id);
    if (!sp) return unexpected_error(errors::invalid_slotid);
    if (auto res = sp->release(); !res) return unexpected_error(res.error());
    return {};
  }
};

/// MARK: d3d

class d3d final : public unknown {
public:
  struct slot : unknown::slot {
    ID3D11Device* device{};
    ID3D11DeviceContext* context{};
    ID3D11BlendState* blend_state{};
    ID3D11SamplerState* sampler_state{};
    ID3D11RasterizerState* rasterizer_state{};
    ID3D11DepthStencilState* depth_stencil_state{};

    ~slot() noexcept {
      if (depth_stencil_state) depth_stencil_state->Release();
      if (rasterizer_state) rasterizer_state->Release();
      if (sampler_state) sampler_state->Release();
      if (blend_state) blend_state->Release();
      if (context) context->Release();
      if (device) device->Release();
    }

    std::expected<void, error_trace> _init_device() {
      const D3D_FEATURE_LEVEL _levels[] = {D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0};
      const auto hr = ::D3D11CreateDevice(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT, _levels, _countof(_levels),
        D3D11_SDK_VERSION, &device, nullptr, &context);
      if (FAILED(hr)) return unexpected_error(errors::operation_failed, "D3D11CreateDevice failed");
      return {};
    }

    std::expected<void, error_trace> _init_blend_state() {
      D3D11_BLEND_DESC blend_desc{};
      blend_desc.RenderTarget[0].BlendEnable = TRUE;
      blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
      blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
      blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
      blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
      blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
      blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
      blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
      const auto hr = device->CreateBlendState(&blend_desc, &blend_state);
      if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateBlendState failed");
      context->OMSetBlendState(blend_state, nullptr, 0xffffffff);
      return {};
    }

    std::expected<void, error_trace> _init_sampler_state() {
      D3D11_SAMPLER_DESC sampler_desc{};
      sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
      sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
      sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
      sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
      sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
      const auto hr = device->CreateSamplerState(&sampler_desc, &sampler_state);
      if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateSamplerState failed");
      context->PSSetSamplers(0, 1, &sampler_state);
      return {};
    }

    std::expected<void, error_trace> _init_rasterizer_state() {
      D3D11_RASTERIZER_DESC rasterizer_desc{};
      rasterizer_desc.FillMode = D3D11_FILL_SOLID;
      rasterizer_desc.CullMode = D3D11_CULL_NONE;
      rasterizer_desc.FrontCounterClockwise = TRUE;
      rasterizer_desc.DepthClipEnable = TRUE;
      const auto hr = device->CreateRasterizerState(&rasterizer_desc, &rasterizer_state);
      if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateRasterizerState failed");
      context->RSSetState(rasterizer_state);
      return {};
    }

    std::expected<void, error_trace> _init_depth_stencil_state() {
      D3D11_DEPTH_STENCIL_DESC depth_stencil_desc{};
      depth_stencil_desc.DepthEnable = TRUE;
      depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
      depth_stencil_desc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL; // Reverse-Z
      const auto hr = device->CreateDepthStencilState(&depth_stencil_desc, &depth_stencil_state);
      if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateDepthStencilState failed");
      context->OMSetDepthStencilState(depth_stencil_state, 0);
      return {};
    }

    std::expected<void, error_trace> initialize(unknown_slotid Id) {
      if (system::unknowns.contains(id)) return {};
      if (auto res = _init_device(); !res) return res;
      if (auto res = _init_blend_state(); !res) return res;
      if (auto res = _init_sampler_state(); !res) return res;
      if (auto res = _init_rasterizer_state(); !res) return res;
      if (auto res = _init_depth_stencil_state(); !res) return res;
      id = Id;
      return {};
    }

    std::expected<void, error_trace> release() {
      id = {};
      if (depth_stencil_state) depth_stencil_state->Release(), depth_stencil_state = nullptr;
      if (rasterizer_state) rasterizer_state->Release(), rasterizer_state = nullptr;
      if (blend_state) blend_state->Release(), blend_state = nullptr;
      if (sampler_state) sampler_state->Release(), sampler_state = nullptr;
      if (context) context->Release(), context = nullptr;
      if (device) device->Release(), device = nullptr;
      return {};
    }
  };

  using unknown::operator bool;

  explicit d3d() {
    static unknown_slotid id = {};
    if (!system::unknowns.contains(id)) {
      const auto temp_id = system::unknowns.add(std::make_unique<slot>());
      const auto temp_sp = system::get_slot_pointer<d3d>(temp_id);
      if (!temp_sp) fatal_error(errors::invalid_slotid);
      if (auto res = temp_sp->initialize(temp_id); !res) fatal_error(res.error());
      id = temp_id;
    }
    this->_id = id;
  }

  ID3D11Device* device() const noexcept {
    if (const auto sp = system::get_slot_pointer<d3d>(_id)) return sp->device;
    return nullptr;
  }
  ID3D11DeviceContext* context() const noexcept {
    if (const auto sp = system::get_slot_pointer<d3d>(_id)) return sp->context;
    return nullptr;
  }
  ID3D11BlendState* blend_state() const noexcept {
    if (const auto sp = system::get_slot_pointer<d3d>(_id)) return sp->blend_state;
    return nullptr;
  }
  ID3D11RasterizerState* rasterizer_state() const noexcept {
    if (const auto sp = system::get_slot_pointer<d3d>(_id)) return sp->rasterizer_state;
    return nullptr;
  }
  ID3D11SamplerState* sampler_state() const noexcept {
    if (const auto sp = system::get_slot_pointer<d3d>(_id)) return sp->sampler_state;
    return nullptr;
  }
  ID3D11DepthStencilState* depth_stencil_state() const noexcept {
    if (const auto sp = system::get_slot_pointer<d3d>(_id)) return sp->depth_stencil_state;
    return nullptr;
  }

  std::expected<void, error_trace> release() const {
    const auto sp = system::get_slot_pointer<d3d>(_id);
    if (!sp) return unexpected_error(errors::invalid_slotid);
    if (auto res = sp->release(); !res) return unexpected_error(res.error());
    return {};
  }
};

/// MARK: dxgi

class dxgi final : public unknown {
public:
  struct slot : unknown::slot {
    IDXGIFactory2* factory{};
    IDXGIDevice2* device{};

    ~slot() noexcept {
      if (device) device->Release();
      if (factory) factory->Release();
    }

    std::expected<void, error_trace> initialize(unknown_slotid Id) {
      if (system::unknowns.contains(id)) return {};
      const auto& d3d = yw::d3d();
      auto hr = ::CreateDXGIFactory2(0, __uuidof(IDXGIFactory2), reinterpret_cast<void**>(&factory));
      if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateDXGIFactory2 failed", int32_t(hr));
      hr = d3d.device()->QueryInterface(__uuidof(IDXGIDevice2), reinterpret_cast<void**>(&device));
      if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateDevice failed", int32_t(hr));
      id = Id;
      return {};
    }

    std::expected<void, error_trace> release() {
      id = {};
      if (device) device->Release(), device = nullptr;
      if (factory) factory->Release(), factory = nullptr;
      return {};
    }
  };

  using unknown::operator bool;

  explicit dxgi() {
    static unknown_slotid id = {};
    if (!system::unknowns.contains(id)) {
      const auto temp_id = system::unknowns.add(std::make_unique<slot>());
      const auto temp_sp = system::get_slot_pointer<dxgi>(temp_id);
      if (!temp_sp) fatal_error(errors::invalid_slotid);
      if (auto res = temp_sp->initialize(temp_id); !res) fatal_error(res.error());
      id = temp_id;
    }
    this->_id = id;
  }
  std::expected<void, error_trace> release() const {
    const auto sp = system::get_slot_pointer<dxgi>(_id);
    if (!sp) return unexpected_error(errors::invalid_slotid);
    if (auto res = sp->release(); !res) return unexpected_error(res.error());
    return {};
  }

  IDXGIFactory2* factory() const noexcept {
    if (const auto sp = system::get_slot_pointer<dxgi>(_id)) return sp->factory;
    return nullptr;
  }
  IDXGIDevice2* device() const noexcept {
    if (const auto sp = system::get_slot_pointer<dxgi>(_id)) return sp->device;
    return nullptr;
  }
};

/// MARK: coinit

class coinit final : public unknown {
public:
  struct slot : unknown::slot {
    ~slot() noexcept {
      if (system::unknowns.contains(id)) ::CoUninitialize();
    }

    std::expected<void, error_trace> initialize(unknown_slotid Id) {
      if (system::unknowns.contains(id)) return {};
      if (auto hr = ::CoInitializeEx(nullptr, COINIT_MULTITHREADED); FAILED(hr))
        return unexpected_error(errors::operation_failed, "CoInitializeEx failed", int32_t(hr));
      id = Id;
      return {};
    }

    std::expected<void, error_trace> release() {
      if (system::unknowns.contains(id)) ::CoUninitialize();
      id = {};
      return {};
    }
  };

  using unknown::operator bool;

  explicit coinit() {
    static unknown_slotid id = {};
    if (!system::unknowns.contains(id)) {
      const auto temp_id = system::unknowns.add(std::make_unique<slot>());
      const auto temp_sp = system::get_slot_pointer<coinit>(temp_id);
      if (!temp_sp) fatal_error(errors::invalid_slotid);
      if (auto res = temp_sp->initialize(temp_id); !res) fatal_error(res.error());
      id = temp_id;
    }
    this->_id = id;
  }
  std::expected<void, error_trace> release() const {
    const auto sp = system::get_slot_pointer<coinit>(_id);
    if (!sp) return unexpected_error(errors::invalid_slotid);
    if (auto res = sp->release(); !res) return unexpected_error(res.error());
    return {};
  }
};

/// MARK: d2d

class d2d final : public unknown {
public:
  struct slot : unknown::slot {
    ID2D1Factory1* factory{};
    ID2D1Device* device{};
    ID2D1DeviceContext* context{};

    ~slot() noexcept {
      if (context) context->Release();
      if (device) device->Release();
      if (factory) factory->Release();
    }

    std::expected<void, error_trace> initialize(unknown_slotid Id) {
      if (system::unknowns.contains(id)) return {};
      const auto dxgi = yw::dxgi();
      const auto coinit = yw::coinit();
      auto hr = ::D2D1CreateFactory(
        D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory1), reinterpret_cast<void**>(&factory));
      if (FAILED(hr)) return unexpected_error(errors::operation_failed, "D2D1CreateFactory failed", int32_t(hr));
      hr = factory->CreateDevice(dxgi.device(), &device);
      if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateDevice failed", int32_t(hr));
      hr = device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &context);
      if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateDeviceContext failed", int32_t(hr));
      id = Id;
      return {};
    }

    std::expected<void, error_trace> release() {
      id = {};
      if (context) context->Release(), context = nullptr;
      if (device) device->Release(), device = nullptr;
      if (factory) factory->Release(), factory = nullptr;
      return {};
    }
  };

  using unknown::operator bool;

  explicit d2d() {
    static unknown_slotid id = {};
    if (!system::unknowns.contains(id)) {
      const auto temp_id = system::unknowns.add(std::make_unique<slot>());
      const auto temp_sp = system::get_slot_pointer<d2d>(temp_id);
      if (!temp_sp) fatal_error(errors::invalid_slotid);
      if (auto res = temp_sp->initialize(temp_id); !res) fatal_error(res.error());
      id = temp_id;
    }
    this->_id = id;
  }
  std::expected<void, error_trace> release() const {
    const auto sp = system::get_slot_pointer<d2d>(_id);
    if (!sp) return unexpected_error(errors::invalid_slotid);
    if (auto res = sp->release(); !res) return unexpected_error(res.error());
    return {};
  }

  ID2D1Factory1* factory() const noexcept {
    if (const auto sp = system::get_slot_pointer<d2d>(_id)) return sp->factory;
    return nullptr;
  }
  ID2D1Device* device() const noexcept {
    if (const auto sp = system::get_slot_pointer<d2d>(_id)) return sp->device;
    return nullptr;
  }
  ID2D1DeviceContext* context() const noexcept {
    if (const auto sp = system::get_slot_pointer<d2d>(_id)) return sp->context;
    return nullptr;
  }

  std::expected<void, error_trace> push_layer(ID2D1Geometry* Geometry) const {
    context()->PushLayer(D2D1::LayerParameters1(D2D1::InfiniteRect(), Geometry), nullptr);
    return {};
  }

  std::expected<void, error_trace> pop_layer() const {
    context()->PopLayer();
    return {};
  }
};

/// MARK: brush

class brush final : public unknown {
public:
  struct slot : unknown::slot {
    ID2D1SolidColorBrush* solid_brush{};
    ID2D1StrokeStyle* stroke_style{};
    ID2D1StrokeStyle* dashed_stroke_style{};
    bool dashed = false;

    ~slot() noexcept {
      if (dashed_stroke_style) dashed_stroke_style->Release();
      if (stroke_style) stroke_style->Release();
      if (solid_brush) solid_brush->Release();
    }

    std::expected<void, error_trace> initialize(unknown_slotid Id) {
      if (system::unknowns.contains(id)) return {};
      const auto& d2d = yw::d2d();
      if (auto hr = d2d.context()->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &solid_brush); FAILED(hr))
        return unexpected_error(errors::operation_failed, "CreateSolidColorBrush failed", int32_t(hr));
      D2D1_STROKE_STYLE_PROPERTIES stroke_style_props{};
      stroke_style_props.startCap = D2D1_CAP_STYLE_ROUND;
      stroke_style_props.endCap = D2D1_CAP_STYLE_ROUND;
      stroke_style_props.dashCap = D2D1_CAP_STYLE_ROUND;
      stroke_style_props.lineJoin = D2D1_LINE_JOIN_ROUND;
      stroke_style_props.miterLimit = 10.0f;
      if (auto hr = d2d.factory()->CreateStrokeStyle(&stroke_style_props, nullptr, 0, &stroke_style); FAILED(hr))
        return unexpected_error(errors::operation_failed, "CreateStrokeStyle failed", int32_t(hr));
      stroke_style_props.dashStyle = D2D1_DASH_STYLE_DASH;
      if (auto hr = d2d.factory()->CreateStrokeStyle(&stroke_style_props, nullptr, 0, &dashed_stroke_style); FAILED(hr))
        return unexpected_error(errors::operation_failed, "CreateStrokeStyle (dashed) failed", int32_t(hr));
      id = Id;
      return {};
    }

    std::expected<void, error_trace> release() {
      id = {};
      if (dashed_stroke_style) dashed_stroke_style->Release(), dashed_stroke_style = nullptr;
      if (stroke_style) stroke_style->Release(), stroke_style = nullptr;
      if (solid_brush) solid_brush->Release(), solid_brush = nullptr;
      dashed = false;
      return {};
    }
  };

  using unknown::operator bool;

  explicit brush() {
    static unknown_slotid id = {};
    if (!system::unknowns.contains(id)) {
      const auto temp_id = system::unknowns.add(std::make_unique<slot>());
      const auto temp_sp = system::get_slot_pointer<brush>(temp_id);
      if (!temp_sp) fatal_error(errors::invalid_slotid);
      if (auto res = temp_sp->initialize(temp_id); !res) fatal_error(res.error());
      id = temp_id;
    }
    this->_id = id;
  }
  std::expected<void, error_trace> release() const {
    const auto sp = system::get_slot_pointer<brush>(_id);
    if (!sp) return unexpected_error(errors::invalid_slotid);
    if (auto res = sp->release(); !res) return unexpected_error(res.error());
    return {};
  }

  yw::color color() const {
    return std::bit_cast<yw::color>(system::get_slot_pointer<brush>(_id)->solid_brush->GetColor());
  }
  auto& color(const yw::color& Color) const {
    system::get_slot_pointer<brush>(_id)->solid_brush->SetColor(reinterpret_cast<const D2D1_COLOR_F*>(&Color));
    return *this;
  }
  bool dashed() const {
    if (const auto sp = system::get_slot_pointer<brush>(_id)) return sp->dashed;
    return false;
  }
  auto& dashed(bool Dashed = true) const {
    if (const auto sp = system::get_slot_pointer<brush>(_id)) sp->dashed = Dashed;
    return *this;
  }
  ID2D1SolidColorBrush* d2d_brush() const {
    if (const auto sp = system::get_slot_pointer<brush>(_id)) return sp->solid_brush;
    return nullptr;
  }
  ID2D1StrokeStyle* d2d_stroke() const {
    if (const auto sp = system::get_slot_pointer<brush>(_id))
      return sp->dashed ? sp->dashed_stroke_style : sp->stroke_style;
    return nullptr;
  }
};

/// MARK: dwrite

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

class dwrite final : public unknown {
public:
  struct slot : unknown::slot {
    IDWriteFactory1* factory{};
    IDWriteTextFormat* text_format{};

    ~slot() noexcept {
      if (text_format) text_format->Release();
      if (factory) factory->Release();
    }

    std::expected<void, error_trace> initialize(unknown_slotid Id) {
      if (system::unknowns.contains(id)) return {};
      const auto& d2d = yw::d2d();
      auto hr = ::DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory1), reinterpret_cast<IUnknown**>(&factory));
      if (FAILED(hr)) return unexpected_error(errors::operation_failed, "DWriteCreateFactory failed", int32_t(hr));
      hr = factory->CreateTextFormat(
        font_config::default_.name->c_str(), nullptr, DWRITE_FONT_WEIGHT(*font_config::default_.weight),
        DWRITE_FONT_STYLE(*font_config::default_.style), DWRITE_FONT_STRETCH(*font_config::default_.stretch),
        font_config::default_.size.value_or(16.0f), L"", &text_format);
      if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateTextFormat failed", int32_t(hr));
      text_format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
      id = Id;
      return {};
    }

    std::expected<void, error_trace> release() {
      id = {};
      if (text_format) text_format->Release(), text_format = nullptr;
      if (factory) factory->Release(), factory = nullptr;
      return {};
    }
  };

  using unknown::operator bool;

  explicit dwrite() {
    static unknown_slotid id = {};
    if (!system::unknowns.contains(id)) {
      const auto temp_id = system::unknowns.add(std::make_unique<slot>());
      const auto temp_sp = system::get_slot_pointer<dwrite>(temp_id);
      if (!temp_sp) fatal_error(errors::invalid_slotid);
      if (auto res = temp_sp->initialize(temp_id); !res) fatal_error(res.error());
      id = temp_id;
    }
    this->_id = id;
  }
  std::expected<void, error_trace> release() const {
    const auto sp = system::get_slot_pointer<dwrite>(_id);
    if (!sp) return unexpected_error(errors::invalid_slotid);
    if (auto res = sp->release(); !res) return unexpected_error(res.error());
    return {};
  }

  IDWriteFactory1* factory() const noexcept {
    if (const auto sp = system::get_slot_pointer<dwrite>(_id)) return sp->factory;
    return nullptr;
  }
  IDWriteTextFormat* text_format() const noexcept {
    if (const auto sp = system::get_slot_pointer<dwrite>(_id)) return sp->text_format;
    return nullptr;
  }
};

//// MARK: wic

class wic final : public unknown {
public:
  struct slot : unknown::slot {
    ::IWICImagingFactory2* factory{};

    ~slot() noexcept {
      if (factory) factory->Release();
    }

    std::expected<void, error_trace> initialize(unknown_slotid Id) {
      if (system::unknowns.contains(id)) return {};
      const auto& d2d = yw::d2d();
      auto hr = ::CoCreateInstance(CLSID_WICImagingFactory2, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory));
      if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CoCreateInstance failed", int32_t(hr));
      id = Id;
      return {};
    }

    std::expected<void, error_trace> release() {
      id = {};
      if (factory) factory->Release(), factory = nullptr;
      return {};
    }
  };

  using unknown::operator bool;

  explicit wic() {
    static unknown_slotid id = {};
    if (!system::unknowns.contains(id)) {
      const auto temp_id = system::unknowns.add(std::make_unique<slot>());
      const auto temp_sp = system::get_slot_pointer<wic>(temp_id);
      if (!temp_sp) fatal_error(errors::invalid_slotid);
      if (auto res = temp_sp->initialize(temp_id); !res) fatal_error(res.error());
      id = temp_id;
    }
    this->_id = id;
  }
  std::expected<void, error_trace> release() const {
    const auto sp = system::get_slot_pointer<wic>(_id);
    if (!sp) return unexpected_error(errors::invalid_slotid);
    if (auto res = sp->release(); !res) return unexpected_error(res.error());
    return {};
  }

  ::IWICImagingFactory2* factory() const noexcept {
    if (const auto sp = system::get_slot_pointer<wic>(_id)) return sp->factory;
    return nullptr;
  }
};

/// MARK: xaudio2

class xaudio2 final : public unknown {
public:
  struct slot : unknown::slot {
    ::IXAudio2* device{};
    ::IXAudio2MasteringVoice* mastering_voice{};

    ~slot() noexcept {
      if (mastering_voice) mastering_voice->DestroyVoice();
      if (device) device->Release();
    }

    std::expected<void, error_trace> initialize(unknown_slotid Id) {
      if (system::unknowns.contains(id)) return {};
      const auto& coinit = yw::coinit();
      if (const auto hr = ::XAudio2Create(&device, 0, XAUDIO2_DEFAULT_PROCESSOR); FAILED(hr))
        return unexpected_error(errors::operation_failed, "XAudio2Create failed", int32_t(hr));
      if (const auto hr = device->CreateMasteringVoice(&mastering_voice); FAILED(hr))
        return unexpected_error(errors::operation_failed, "CreateMasteringVoice failed", int32_t(hr));
      id = Id;
      return {};
    }

    std::expected<void, error_trace> release() {
      id = {};
      if (mastering_voice) mastering_voice->DestroyVoice(), mastering_voice = nullptr;
      if (device) device->Release(), device = nullptr;
      return {};
    }
  };

  using unknown::operator bool;

  explicit xaudio2() {
    static unknown_slotid id = {};
    if (!system::unknowns.contains(id)) {
      const auto temp_id = system::unknowns.add(std::make_unique<slot>());
      const auto temp_sp = system::get_slot_pointer<xaudio2>(temp_id);
      if (!temp_sp) fatal_error(errors::invalid_slotid);
      if (auto res = temp_sp->initialize(temp_id); !res) fatal_error(res.error());
      id = temp_id;
    }
    this->_id = id;
  }
  std::expected<void, error_trace> release() const {
    const auto sp = system::get_slot_pointer<xaudio2>(_id);
    if (!sp) return unexpected_error(errors::invalid_slotid);
    if (auto res = sp->release(); !res) return unexpected_error(res.error());
    return {};
  }

  ::IXAudio2* device() const noexcept {
    if (const auto sp = system::get_slot_pointer<xaudio2>(_id)) return sp->device;
    return nullptr;
  }
  ::IXAudio2MasteringVoice* mastering_voice() const noexcept {
    if (const auto sp = system::get_slot_pointer<xaudio2>(_id)) return sp->mastering_voice;
    return nullptr;
  }
};

/// MARK: comptr

template<typename Com> class comptr {
  comptr(const comptr&) = delete;
  comptr& operator=(const comptr&) = delete;
  Com* _ptr{nullptr};

public:
  explicit operator bool() const { return _ptr != nullptr; }
  Com* operator->() const { return _ptr; }
  bool operator==(Com* Other) const { return _ptr == Other; }
  ~comptr() {
    if (_ptr) _ptr->Release();
    _ptr = nullptr;
  }
  comptr() = default;
  comptr(comptr&& Other) : _ptr(std::exchange(Other._ptr, nullptr)) {}
  comptr& operator=(comptr&& Other) {
    if (this == &Other) return *this;
    if (_ptr) _ptr->Release();
    _ptr = std::exchange(Other._ptr, nullptr);
    return *this;
  }
  Com*& get() & { return _ptr; }
  Com* get() const& { return _ptr; }
  void release() {
    if (_ptr) _ptr->Release();
    _ptr = nullptr;
  }

  /// releases current pointer and takes ownership of new
  void reset(Com* New) noexcept {
    if (_ptr) _ptr->Release();
    _ptr = New;
  }

  explicit operator Com*&() & { return _ptr; }
  explicit operator Com*() const& { return _ptr; }
};
} // namespace yw
