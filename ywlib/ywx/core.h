#pragma once

#include <ywlib>

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
  return std::unexpected(error(errors::operation_failed, #func " failed", int32_t(hr)))

namespace yw {

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

/// MARK: desktop_client_size

inline uint2 desktop_client_size() {
  if (RECT r; !::GetClientRect(::GetDesktopWindow(), &r)) return uint2();
  else return uint2(r.right, r.bottom);
}

/// MARK: com_list_to_release

namespace system {
inline class com_list_to_release {
  std::vector<general_slotid> _ids;

public:
  ~com_list_to_release() { release(); }
  void release() {
    for (const auto id : _ids | std::views::reverse) internal::general_slotset.erase(id);
  }
  void clear() { _ids.clear(); }
  void push(general_slotid id) { _ids.push_back(id); }
} com_list_to_release;
} // namespace system

/// MARK: instance

/// base handle class for window and controls
class interface : public general_handle {
public:
  struct slot : general_slot {
    virtual bool attachable() const { return false; }
    virtual std::expected<void, error> attach(general_slotid Child) { return {}; }
    virtual std::expected<void, error> detach(general_slotid Child) { return {}; }
    virtual std::expected<void, error> make_dirty() = 0;
    virtual std::expected<void, error> make_messy() = 0;
    virtual general_slotid get_window_id() const = 0;
  };
};

/// MARK: wclass

class wclass final : public general_handle {
  static constexpr const source_line _source_line = source_line::here();
  inline static general_slot* _sp = nullptr;

public:
  struct slot : general_slot {
    static slot* get() noexcept { return static_cast<slot*>(_sp); }
    static LRESULT __stdcall wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

    WNDCLASSW wc{
      .style = CS_DBLCLKS,
      .lpfnWndProc = wndproc,
      .hInstance = ::GetModuleHandleW(nullptr),
      .hCursor = ::LoadCursorW(nullptr, IDC_ARROW),
      .lpszClassName = L"ywlib_window_class"};

    std::expected<void, error> initialize() {
      if (::RegisterClassW(&wc)) return {};
      else if (::GetLastError() == ERROR_CLASS_ALREADY_EXISTS) return {};
      else return std::unexpected(error(errors::operation_failed, "RegisterClassW failed"));
    }
  };

  explicit wclass() {
    if (_sp) return;
    const auto sp = general_handle::create_slot<wclass>(_source_line);
    if (!sp) error(errors::slot_creation_failed).print_as_fatal(_source_line);
    if (auto res = sp->initialize(); !res) res.error().print_as_fatal(_source_line);
    _sp = sp;
    _id = sp->id;
  }

  HINSTANCE hinstance() const noexcept { return slot::get()->wc.hInstance; }
  null_terminated<wchar_t> name() const noexcept { return slot::get()->wc.lpszClassName; }
};

/// MARK: d3d

class d3d final : public general_handle {
  static constexpr const source_line _source_line = source_line::here();
  inline static general_slot* _sp = nullptr;

public:
  struct slot : general_slot {
    static slot* get() noexcept { return static_cast<slot*>(_sp); }

    ID3D11Device* device{};
    ID3D11DeviceContext* context{};
    ID3D11BlendState* blend_state{};
    ID3D11SamplerState* sampler_state{};
    ID3D11RasterizerState* rasterizer_state{};

    std::expected<void, error> _init_device() {
      const D3D_FEATURE_LEVEL _levels[] = {D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0};
      const auto hr = ::D3D11CreateDevice(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT, _levels, _countof(_levels),
        D3D11_SDK_VERSION, &device, nullptr, &context);
      if (FAILED(hr)) return std::unexpected(error(errors::operation_failed, "D3D11CreateDevice failed"));
      return {};
    }

    std::expected<void, error> _init_blend_state() {
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

    std::expected<void, error> _init_sampler_state() {
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

    std::expected<void, error> _init_rasterizer_state() {
      D3D11_RASTERIZER_DESC rasterizer_desc{};
      rasterizer_desc.FillMode = D3D11_FILL_SOLID;
      rasterizer_desc.CullMode = D3D11_CULL_NONE;
      rasterizer_desc.FrontCounterClockwise = TRUE;
      rasterizer_desc.DepthClipEnable = TRUE;
      hresult_test(device->CreateRasterizerState, &rasterizer_desc, &rasterizer_state);
      context->RSSetState(rasterizer_state);
      return {};
    }

    std::expected<void, error> initialize() {
      if (auto res = _init_device(); !res) return res;
      if (auto res = _init_blend_state(); !res) return res;
      if (auto res = _init_sampler_state(); !res) return res;
      if (auto res = _init_rasterizer_state(); !res) return res;
      return {};
    }
  };

  explicit d3d() {
    if (_sp) return;
    const auto sp = general_handle::create_slot<d3d>(_source_line);
    if (!sp) error(errors::slot_creation_failed).print_as_fatal(_source_line);
    if (auto res = sp->initialize(); !res) res.error().print_as_fatal(_source_line);
    _sp = sp;
    _id = sp->id;
  }

  ID3D11Device* device() const noexcept { return slot::get()->device; }
  ID3D11DeviceContext* context() const noexcept { return slot::get()->context; }
  ID3D11BlendState* blend_state() const noexcept { return slot::get()->blend_state; }
  ID3D11RasterizerState* rasterizer_state() const noexcept { return slot::get()->rasterizer_state; }
  ID3D11SamplerState* sampler_state() const noexcept { return slot::get()->sampler_state; }
};

/// MARK: dxgi

class dxgi final : public general_handle {
  static constexpr const source_line _source_line = source_line::here();
  inline static general_slot* _sp = nullptr;

public:
  struct slot : general_slot {
    static slot* get() noexcept { return static_cast<slot*>(_sp); }

    IDXGIFactory2* factory{};
    IDXGIDevice2* device{};

    std::expected<void, error> initialize() {
      const auto d3d = yw::d3d();
      hresult_test(::CreateDXGIFactory2, 0, __uuidof(IDXGIFactory2), reinterpret_cast<void**>(&factory));
      hresult_test(d3d.device()->QueryInterface, __uuidof(IDXGIDevice2), reinterpret_cast<void**>(&device));
      return {};
    }
  };

  explicit dxgi() {
    if (_sp) return;
    const auto sp = general_handle::create_slot<dxgi>(_source_line);
    if (!sp) error(errors::slot_creation_failed).print_as_fatal(_source_line);
    if (auto res = sp->initialize(); !res) res.error().print_as_fatal(_source_line);
    _sp = sp;
    _id = sp->id;
  }

  IDXGIFactory2* factory() const noexcept { return slot::get()->factory; }
  IDXGIDevice2* device() const noexcept { return slot::get()->device; }
};

/// MARK: d2d

class d2d final : public general_handle {
  static constexpr const source_line _source_line = source_line::here();
  inline static general_slot* _sp = nullptr;

public:
  struct slot : general_slot {
    static slot* get() noexcept { return static_cast<slot*>(_sp); }

    ID2D1Factory1* factory{};
    ID2D1Device* device{};
    ID2D1DeviceContext* context{};

    std::expected<void, error> initialize() {
      const auto dxgi = yw::dxgi();
      const auto factory_type = D2D1_FACTORY_TYPE_SINGLE_THREADED;
      hresult_test(::D2D1CreateFactory, factory_type, __uuidof(ID2D1Factory1), reinterpret_cast<void**>(&factory));
      hresult_test(factory->CreateDevice, dxgi.device(), &device);
      hresult_test(device->CreateDeviceContext, D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &context);
      return {};
    }
  };

  explicit d2d() {
    if (_sp) return;
    const auto sp = general_handle::create_slot<d2d>(_source_line);
    if (!sp) error(errors::slot_creation_failed).print_as_fatal(_source_line);
    if (auto res = sp->initialize(); !res) res.error().print_as_fatal(_source_line);
    _sp = sp;
    _id = sp->id;
  }

  ID2D1Factory1* factory() const noexcept { return slot::get()->factory; }
  ID2D1Device* device() const noexcept { return slot::get()->device; }
  ID2D1DeviceContext* context() const noexcept { return slot::get()->context; }

  void push_layer(ID2D1Geometry* Geom) const {
    context()->PushLayer(D2D1::LayerParameters1(D2D1::InfiniteRect(), Geom), nullptr);
  }
  void pop_layer() const { context()->PopLayer(); }
};

/// MARK: brush

class brush final : public general_handle {
  static constexpr const source_line _source_line = source_line::here();
  inline static general_slot* _sp = nullptr;

public:
  struct slot : general_slot {
    static slot* get() noexcept { return static_cast<slot*>(_sp); }

    ID2D1SolidColorBrush* solid_brush{};
    ID2D1StrokeStyle* stroke_style{};
    ID2D1StrokeStyle* dashed_stroke_style{};
    yw::color color = colors::black;
    bool dashed = false;

    std::expected<void, error> initialize() {
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

  explicit brush() {
    if (_sp) return;
    const auto sp = general_handle::create_slot<brush>(_source_line);
    if (!sp) error(errors::slot_creation_failed).print_as_fatal(_source_line);
    if (auto res = sp->initialize(); !res) res.error().print_as_fatal(_source_line);
    _sp = sp;
    _id = sp->id;
  }

  ID2D1SolidColorBrush* d2d_brush() const noexcept { return slot::get()->solid_brush; }
  const yw::color& color() const noexcept { return slot::get()->color; }
  bool dashed() const noexcept { return slot::get()->dashed; }

  ID2D1StrokeStyle* d2d_stroke() const noexcept {
    const auto sp = slot::get();
    return sp->dashed ? sp->dashed_stroke_style : sp->stroke_style;
  }

  auto& color(const yw::color& Color) const noexcept {
    const auto sp = slot::get();
    sp->color = Color;
    sp->solid_brush->SetColor(reinterpret_cast<const D2D1_COLOR_F*>(&Color));
    return *this;
  }

  auto& dashed(bool Dashed = true) const noexcept {
    slot::get()->dashed = Dashed;
    return *this;
  }
};

/// MARK: font_config

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
  std::optional<string<wchar_t>> name = std::nullopt;
  std::optional<float> size = std::nullopt;
  std::optional<font_weight> weight = std::nullopt;
  std::optional<font_style> style = std::nullopt;
  std::optional<font_stretch> stretch = std::nullopt;
  static const font_config default_;
};

inline const font_config font_config::default_{
  string<wchar_t>(), 16.0f, font_weight::normal, font_style::normal, font_stretch::normal};

/// MARK: dwrite

class dwrite final : public general_handle {
  static constexpr const source_line _source_line = source_line::here();
  inline static general_slot* _sp = nullptr;

public:
  struct slot : general_slot {
    static slot* get() noexcept { return reinterpret_cast<slot*>(_sp); }

    IDWriteFactory1* factory{};
    IDWriteTextFormat* text_format{};

    std::expected<void, error> initialize() {
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

  explicit dwrite() {
    if (_sp) return;
    const auto sp = general_handle::create_slot<dwrite>(_source_line);
    if (!sp) error(errors::slot_creation_failed).print_as_fatal(_source_line);
    if (auto res = sp->initialize(); !res) res.error().print_as_fatal(_source_line);
    _sp = sp;
    _id = sp->id;
  }

  IDWriteFactory1* factory() const noexcept { return slot::get()->factory; }
  IDWriteTextFormat* text_format() const noexcept { return slot::get()->text_format; }
};

/// MARK: coinit

class coinit final : public general_handle {
  static constexpr const source_line _source_line = source_line::here();
  inline static general_slot* _sp = nullptr;

public:
  struct slot : general_slot {
    static slot* get() noexcept { return static_cast<slot*>(_sp); }

    std::expected<void, error> initialize() {
      hresult_test(::CoInitializeEx, nullptr, COINIT_MULTITHREADED);
      system::com_list_to_release.push(this->id);
      return {};
    }
  };

  explicit coinit() {
    if (_sp) return;
    const auto sp = general_handle::create_slot<coinit>(_source_line);
    if (!sp) error(errors::slot_creation_failed).print_as_fatal(_source_line);
    if (auto res = sp->initialize(); !res) res.error().print_as_fatal(_source_line);
    _sp = sp;
    _id = sp->id;
  }
};

//// MARK: wic

class wic final : public general_handle {
  static constexpr const source_line _source_line = source_line::here();
  inline static general_slot* _sp = nullptr;

public:
  struct slot : general_slot {
    static slot* get() noexcept { return static_cast<slot*>(_sp); }

    ::IWICImagingFactory2* factory{};

    std::expected<void, error> initialize() {
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

    std::expected<void, error> initialize() {
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
