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

#ifdef interface
#undef interface
#endif

#define hresult_test(func, ...)                        \
  if (const auto hr = (func)(__VA_ARGS__); FAILED(hr)) \
  return unexpected_error(errors::operation_failed, #func " failed", int32_t(hr))

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

// inline std::unexpected<error_trace> unexpected_win32_error(const char* msg, const source& src = {}) {
//   return unexpected_error(errors::operation_failed, msg, int32_t(::GetLastError()), uint64_t(-1), src);
// }

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
    virtual ~slot() {}
  };

protected:
  slotset<slot>::slotid _id{};
  unknown() noexcept = default;
  unknown(slotset<slot>::slotid Id) : _id(Id) {}

public:
  unknown(unknown&& Other) noexcept : _id(std::move(Other._id)) {}
  unknown& operator=(unknown&& Other) noexcept {
    if (this != &Other) _id = std::exchange(Other._id, {});
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

/// MARK: com_list_to_release

namespace system {
inline class com_list_to_release {
  std::vector<unknown_slotid> _ids;

public:
  ~com_list_to_release() {
    for (const auto id : _ids | std::views::reverse) unknowns.erase(id);
  }
  void clear() { _ids.clear(); }
  void push(unknown_slotid id) { _ids.push_back(id); }
} com_list_to_release;
} // namespace system

/// MARK: instance

class interface : public unknown {
protected:
  using unknown::unknown;

public:
  struct slot : unknown::slot {
    virtual bool attachable() const { return false; }
    virtual std::expected<void, error_trace> attach(unknown_slotid Child) { return {}; }
    virtual std::expected<void, error_trace> detach(unknown_slotid Child) { return {}; }
    virtual unknown_slotid get_window_id() const = 0;
    virtual std::expected<void, error_trace> make_dirty() = 0;
    virtual std::expected<void, error_trace> make_messy() = 0;
  };

  using unknown::operator bool;
};

/// MARK: singleton

template<typename T> class singleton : public unknown {
protected:
  inline static unknown_slotid singleton_id = {};
  using unknown::unknown;

  static std::expected<void, error_trace> initialize_singleton() {
    if (system::unknowns.contains(singleton_id)) return {};
    const auto temp_id = system::unknowns.add(std::make_unique<typename T::slot>());
    if (const auto sp = system::get_slot_pointer<T>(temp_id); !sp) {
      system::unknowns.erase(temp_id);
      return unexpected_error(errors::invalid_slotid);
    } else if (singleton_id = sp->id = temp_id; false) (void)0;
    else if (auto res = sp->initialize(); !res) {
      sp->id = singleton_id = {};
      system::unknowns.erase(temp_id);
      return unexpected_error(res.error());
    } else return {};
  }

public:
  struct slot : unknown::slot {};
  using unknown::operator bool;
};

/// MARK: wclass

class wclass final : public singleton<wclass> {
public:
  struct slot : singleton<wclass>::slot {
    WNDCLASSW wc{
      .style = CS_DBLCLKS,
      .lpfnWndProc = system::wndproc,
      .hInstance = ::GetModuleHandleW(nullptr),
      .hCursor = ::LoadCursorW(nullptr, IDC_ARROW),
      .lpszClassName = L"ywlib_window_class"};

    std::expected<void, error_trace> initialize() {
      if (::RegisterClassW(&wc)) return {};
      else if(::GetLastError() == ERROR_CLASS_ALREADY_EXISTS) return {};
      else return unexpected_error(errors::operation_failed, "RegisterClassW failed");
    }
  };

  using unknown::operator bool;

  explicit wclass() {
    if (auto res = initialize_singleton(); !res) fatal_error(res.error());
    this->_id = singleton_id;
  }

  HINSTANCE hinstance() const noexcept {
    if (auto sp = system::get_slot_pointer<wclass>(_id)) return sp->wc.hInstance;
    return nullptr;
  }

  null_terminated<wchar_t> name() const noexcept {
    if (auto sp = system::get_slot_pointer<wclass>(_id)) return sp->wc.lpszClassName;
    return {};
  }
};

/// MARK: d3d

class d3d final : public singleton<d3d> {
public:
  struct slot : singleton<d3d>::slot {
    ID3D11Device* device{};
    ID3D11DeviceContext* context{};
    ID3D11BlendState* blend_state{};
    ID3D11SamplerState* sampler_state{};
    ID3D11RasterizerState* rasterizer_state{};

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
      hresult_test(device->CreateBlendState, &blend_desc, &blend_state);
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
      hresult_test(device->CreateSamplerState, &sampler_desc, &sampler_state);
      context->PSSetSamplers(0, 1, &sampler_state);
      return {};
    }

    std::expected<void, error_trace> _init_rasterizer_state() {
      D3D11_RASTERIZER_DESC rasterizer_desc{};
      rasterizer_desc.FillMode = D3D11_FILL_SOLID;
      rasterizer_desc.CullMode = D3D11_CULL_NONE;
      rasterizer_desc.FrontCounterClockwise = TRUE;
      rasterizer_desc.DepthClipEnable = TRUE;
      hresult_test(device->CreateRasterizerState, &rasterizer_desc, &rasterizer_state);
      context->RSSetState(rasterizer_state);
      return {};
    }

    std::expected<void, error_trace> initialize() {
      if (auto res = _init_device(); !res) return res;
      if (auto res = _init_blend_state(); !res) return res;
      if (auto res = _init_sampler_state(); !res) return res;
      if (auto res = _init_rasterizer_state(); !res) return res;
      return {};
    }
  };

  using unknown::operator bool;

  explicit d3d() {
    if (auto res = initialize_singleton(); !res) fatal_error(res.error());
    this->_id = singleton_id;
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
};

/// MARK: dxgi

class dxgi final : public singleton<dxgi> {
public:
  struct slot : singleton<dxgi>::slot {
    IDXGIFactory2* factory{};
    IDXGIDevice2* device{};

    std::expected<void, error_trace> initialize() {
      hresult_test(::CreateDXGIFactory2, 0, __uuidof(IDXGIFactory2), reinterpret_cast<void**>(&factory));
      hresult_test(d3d().device()->QueryInterface, __uuidof(IDXGIDevice2), reinterpret_cast<void**>(&device));
      return {};
    }
  };

  using unknown::operator bool;

  explicit dxgi() {
    if (auto res = initialize_singleton(); !res) fatal_error(res.error());
    this->_id = singleton_id;
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

/// MARK: d2d

class d2d final : public singleton<d2d> {
public:
  struct slot : singleton<d2d>::slot {
    ID2D1Factory1* factory{};
    ID2D1Device* device{};
    ID2D1DeviceContext* context{};

    std::expected<void, error_trace> initialize() {
      const auto factory_type = D2D1_FACTORY_TYPE_SINGLE_THREADED;
      hresult_test(::D2D1CreateFactory, factory_type, __uuidof(ID2D1Factory1), reinterpret_cast<void**>(&factory));
      hresult_test(factory->CreateDevice, dxgi().device(), &device);
      hresult_test(device->CreateDeviceContext, D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &context);
      return {};
    }
  };

  using unknown::operator bool;

  explicit d2d() {
    if (auto res = initialize_singleton(); !res) fatal_error(res.error());
    this->_id = singleton_id;
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

class brush final : public singleton<brush> {
public:
  struct slot : singleton<brush>::slot {
    ID2D1SolidColorBrush* solid_brush{};
    ID2D1StrokeStyle* stroke_style{};
    ID2D1StrokeStyle* dashed_stroke_style{};
    bool dashed = false;

    std::expected<void, error_trace> initialize() {
      const auto& d2d = yw::d2d();
      hresult_test(d2d.context()->CreateSolidColorBrush, D2D1::ColorF(D2D1::ColorF::Black), &solid_brush);
      D2D1_STROKE_STYLE_PROPERTIES stroke_style_props{
        .startCap = D2D1_CAP_STYLE_ROUND,
        .endCap = D2D1_CAP_STYLE_ROUND,
        .dashCap = D2D1_CAP_STYLE_ROUND,
        .lineJoin = D2D1_LINE_JOIN_ROUND,
        .miterLimit = 10.0f};
      hresult_test(d2d.factory()->CreateStrokeStyle, &stroke_style_props, nullptr, 0, &stroke_style);
      stroke_style_props.dashStyle = D2D1_DASH_STYLE_DASH;
      hresult_test(d2d.factory()->CreateStrokeStyle, &stroke_style_props, nullptr, 0, &dashed_stroke_style);
      return {};
    }
  };

  using unknown::operator bool;

  explicit brush() {
    if (auto res = initialize_singleton(); !res) fatal_error(res.error());
    this->_id = singleton_id;
  }

  ID2D1SolidColorBrush* d2d_brush() const noexcept {
    if (const auto sp = system::get_slot_pointer<brush>(_id)) return sp->solid_brush;
    return nullptr;
  }

  ID2D1StrokeStyle* d2d_stroke() const noexcept {
    if (const auto sp = system::get_slot_pointer<brush>(_id))
      return sp->dashed ? sp->dashed_stroke_style : sp->stroke_style;
    return nullptr;
  }

  yw::color color() const noexcept {
    if (const auto sp = system::get_slot_pointer<brush>(_id))
      return std::bit_cast<yw::color>(sp->solid_brush->GetColor());
    return {};
  }

  auto& color(const yw::color& Color) const noexcept {
    if (const auto sp = system::get_slot_pointer<brush>(_id))
      sp->solid_brush->SetColor(reinterpret_cast<const D2D1_COLOR_F*>(&Color));
    return *this;
  }

  bool dashed() const noexcept {
    if (const auto sp = system::get_slot_pointer<brush>(_id)) return sp->dashed;
    return false;
  }

  auto& dashed(bool Dashed = true) const noexcept {
    if (const auto sp = system::get_slot_pointer<brush>(_id)) sp->dashed = Dashed;
    return *this;
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

class dwrite final : public singleton<dwrite> {
public:
  struct slot : singleton<dwrite>::slot {
    IDWriteFactory1* factory{};
    IDWriteTextFormat* text_format{};

    std::expected<void, error_trace> initialize() {
      hresult_test(
        ::DWriteCreateFactory, DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory1),
        reinterpret_cast<IUnknown**>(&factory));
      hresult_test(
        factory->CreateTextFormat, font_config::default_.name->c_str(), nullptr,
        DWRITE_FONT_WEIGHT(*font_config::default_.weight), DWRITE_FONT_STYLE(*font_config::default_.style),
        DWRITE_FONT_STRETCH(*font_config::default_.stretch), font_config::default_.size.value_or(16.0f), L"",
        &text_format);
      hresult_test(text_format->SetTextAlignment, DWRITE_TEXT_ALIGNMENT_CENTER);
      return {};
    }
  };

  using unknown::operator bool;

  explicit dwrite() {
    if (auto res = initialize_singleton(); !res) fatal_error(res.error());
    this->_id = singleton_id;
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

/// MARK: coinit

class coinit final : public singleton<coinit> {
public:
  struct slot : singleton<coinit>::slot {
    std::expected<void, error_trace> initialize() {
      hresult_test(::CoInitializeEx, nullptr, COINIT_MULTITHREADED);
      system::com_list_to_release.push(this->id);
      return {};
    }
  };

  using unknown::operator bool;

  explicit coinit() {
    if (auto res = initialize_singleton(); !res) fatal_error(res.error());
    this->_id = singleton_id;
  }
};

//// MARK: wic

class wic final : public singleton<wic> {
public:
  struct slot : singleton<wic>::slot {
    ::IWICImagingFactory2* factory{};

    std::expected<void, error_trace> initialize() {
      const auto& coinit = yw::coinit();
      hresult_test(::CoCreateInstance, CLSID_WICImagingFactory2, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory));
      system::com_list_to_release.push(this->id);
      return {};
    }
  };

  using unknown::operator bool;

  explicit wic() {
    if (auto res = initialize_singleton(); !res) fatal_error(res.error());
    this->_id = singleton_id;
  }

  ::IWICImagingFactory2* factory() const noexcept {
    if (const auto sp = system::get_slot_pointer<wic>(_id)) return sp->factory;
    return nullptr;
  }
};

/// MARK: xaudio2

class xaudio2 final : public singleton<xaudio2> {
public:
  struct slot : singleton<xaudio2>::slot {
    ::IXAudio2* device{};
    ::IXAudio2MasteringVoice* mastering_voice{};

    std::expected<void, error_trace> initialize() {
      const auto& coinit = yw::coinit();
      hresult_test(::XAudio2Create, &device, 0, XAUDIO2_DEFAULT_PROCESSOR);
      hresult_test(device->CreateMasteringVoice, &mastering_voice);
      system::com_list_to_release.push(this->id);
      return {};
    }
  };

  using unknown::operator bool;

  explicit xaudio2() {
    if (auto res = initialize_singleton(); !res) fatal_error(res.error());
    this->_id = singleton_id;
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
