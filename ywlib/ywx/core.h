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

#define win32_bool_test(func, ...) \
  if (!(func)(__VA_ARGS__))        \
  return std::unexpected(error(errors::operation_failed, #func " failed", int32_t(::GetLastError())))

namespace yw {

inline constexpr float arbitrary_value = 4.0f;

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
  std::vector<function<void>> _functions;

public:
  ~com_list_to_release() { release(); }
  void release() {
    for (const auto& func : _functions | std::views::reverse) func();
  }
  void clear() { _functions.clear(); }
  void push(function<void> func) { _functions.push_back(func); }
} com_list_to_release;
} // namespace system

/// MARK: interface

/// base handle class for window and controls
class interface {
public:
  struct slot {
    inline static slotset<slot> slots{};
    template<typename H> static slotset<slot>::slotid add() { return slots.add(std::make_unique<typename H::slot>()); }
    template<typename H> static typename H::slot* get(slotset<slot>::slotid Id) noexcept {
      return static_cast<typename H::slot*>(slots.get(Id));
    }
    slotset<slot>::slotid id;
    yw::source_line source = source_line::null();
    virtual bool attachable() const { return false; }
    virtual std::expected<void, error> attach(slotset<slot>::slotid Child) { return {}; }
    virtual std::expected<void, error> detach(slotset<slot>::slotid Child) { return {}; }
    virtual void make_none() { return; }
    virtual void make_dirty() { return; }
    virtual void make_messy() { return; }
  };

protected:
  slotset<slot>::slotid _id{};
  explicit interface(slotset<slot>::slotid Id) : _id(Id) {}
  template<typename H> static typename H::slot* get_slot(const H* h) { return slot::get<H>(h->_id); }

public:
  using slotid = slotset<slot>::slotid;

  virtual ~interface() noexcept { slot::slots.erase(_id); }
  interface() noexcept = default;
  interface(const interface&) = delete;
  interface& operator=(const interface&) = delete;
  interface(interface&& Other) noexcept : _id(std::exchange(Other._id, {})) {}
  interface& operator=(interface&& Other) noexcept {
    if (this != &Other) {
      slot::slots.erase(_id);
      _id = std::exchange(Other._id, {});
    }
    return *this;
  }
  slotset<slot>::slotid id() const noexcept { return _id; }
  explicit operator bool() const noexcept;
};

/// MARK: wclass

class wclass {
  inline static void* ptr = nullptr;
  static LRESULT __stdcall wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

  struct slot {
    WNDCLASSW wc{
      .style = CS_DBLCLKS,
      .lpfnWndProc = wndproc,
      .hInstance = ::GetModuleHandleW(nullptr),
      .hCursor = ::LoadCursorW(nullptr, IDC_ARROW),
      .lpszClassName = L"ywlib_window_class"};

    slot() {
      if (!::RegisterClassW(&wc) && ::GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
        error(errors::operation_failed, "RegisterClassW failed").go_off();
    }

    static slot* get() noexcept {
      if (!ptr) ptr = new slot();
      return static_cast<slot*>(ptr);
    }
  };

public:
  static HINSTANCE hinstance() noexcept { return slot::get()->wc.hInstance; }
  static const wchar_t* name() noexcept { return slot::get()->wc.lpszClassName; }
};

/// MARK: d3d

class d3d {
  inline static void* ptr = nullptr;

  struct slot {
    ID3D11Device* device{};
    ID3D11DeviceContext* context{};
    ID3D11BlendState* blend_state{};
    ID3D11SamplerState* sampler_state{};
    ID3D11RasterizerState* rasterizer_state{};

    slot() {
      if (auto res = _init_device(); !res) res.error().go_off();
      if (auto res = _init_blend_state(); !res) res.error().go_off();
      if (auto res = _init_sampler_state(); !res) res.error().go_off();
      if (auto res = _init_rasterizer_state(); !res) res.error().go_off();
    }

    static slot* get() noexcept {
      if (!ptr) ptr = new slot();
      return static_cast<slot*>(ptr);
    }

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
  };

public:
  static ID3D11Device* device() noexcept { return slot::get()->device; }
  static ID3D11DeviceContext* context() noexcept { return slot::get()->context; }
  static ID3D11BlendState* blend_state() noexcept { return slot::get()->blend_state; }
  static ID3D11RasterizerState* rasterizer_state() noexcept { return slot::get()->rasterizer_state; }
  static ID3D11SamplerState* sampler_state() noexcept { return slot::get()->sampler_state; }

  static void pointlist() noexcept { context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST); }
  static void linelist() noexcept { context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST); }
  static void linestrip() noexcept { context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP); }
  static void trianglelist() noexcept { context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); }
  static void trianglestrip() noexcept { context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP); }
}; // namespace d3d

/// MARK: dxgi

class dxgi {
  inline static void* ptr = nullptr;

  struct slot {
    IDXGIFactory2* factory{};
    IDXGIDevice2* device{};

    slot() {
      if (auto res = _init_factory(); !res) res.error().go_off();
      if (auto res = _init_device(); !res) res.error().go_off();
    }

    static slot* get() noexcept {
      if (!ptr) ptr = new slot();
      return static_cast<slot*>(ptr);
    }

    std::expected<void, error> _init_factory() {
      hresult_test(::CreateDXGIFactory2, 0, __uuidof(IDXGIFactory2), reinterpret_cast<void**>(&factory));
      return {};
    }

    std::expected<void, error> _init_device() {
      hresult_test(d3d().device()->QueryInterface, __uuidof(IDXGIDevice2), reinterpret_cast<void**>(&device));
      return {};
    }
  };

public:
  static IDXGIFactory2* factory() noexcept { return slot::get()->factory; }
  static IDXGIDevice2* device() noexcept { return slot::get()->device; }
};

/// MARK: d2d

class d2d {
  inline static void* ptr = nullptr;

  struct slot {
    ID2D1Factory1* factory{};
    ID2D1Device* device{};
    ID2D1DeviceContext* context{};

    slot() {
      if (auto res = _init_factory(); !res) res.error().go_off();
      if (auto res = _init_device(); !res) res.error().go_off();
      if (auto res = _init_context(); !res) res.error().go_off();
    }

    static slot* get() noexcept {
      if (!ptr) ptr = new slot();
      return static_cast<slot*>(ptr);
    }

    std::expected<void, error> _init_factory() {
      const auto factory_type = D2D1_FACTORY_TYPE_SINGLE_THREADED;
      hresult_test(::D2D1CreateFactory, factory_type, __uuidof(ID2D1Factory1), reinterpret_cast<void**>(&factory));
      return {};
    }

    std::expected<void, error> _init_device() {
      hresult_test(factory->CreateDevice, dxgi().device(), &device);
      return {};
    }

    std::expected<void, error> _init_context() {
      hresult_test(device->CreateDeviceContext, D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &context);
      return {};
    }
  };

public:
  static ID2D1Factory1* factory() noexcept { return slot::get()->factory; }
  static ID2D1Device* device() noexcept { return slot::get()->device; }
  static ID2D1DeviceContext* context() noexcept { return slot::get()->context; }

  static void push_layer(ID2D1Geometry* Geom) noexcept {
    context()->PushLayer(D2D1::LayerParameters1(D2D1::InfiniteRect(), Geom), nullptr);
  }
  static void pop_layer() noexcept { context()->PopLayer(); }
};

/// MARK: brush

class brush {
  inline static void* ptr = nullptr;

  struct slot {
    ID2D1SolidColorBrush* solid_brush{};
    ID2D1StrokeStyle* stroke_style{};
    ID2D1StrokeStyle* dashed_stroke_style{};
    yw::color color = colors::black;
    bool dashed = false;

    slot() {
      if (auto res = _init_brush(); !res) res.error().go_off();
      if (auto res = _init_styles(); !res) res.error().go_off();
    }

    static slot* get() noexcept {
      if (!ptr) ptr = new slot();
      return static_cast<slot*>(ptr);
    }

    std::expected<void, error> _init_brush() {
      hresult_test(d2d().context()->CreateSolidColorBrush, D2D1::ColorF(D2D1::ColorF::Black), &solid_brush);
      return {};
    }

    std::expected<void, error> _init_styles() {
      D2D1_STROKE_STYLE_PROPERTIES stroke_style_props{
        .startCap = D2D1_CAP_STYLE_ROUND,
        .endCap = D2D1_CAP_STYLE_ROUND,
        .dashCap = D2D1_CAP_STYLE_ROUND,
        .lineJoin = D2D1_LINE_JOIN_ROUND,
        .miterLimit = 10.0f};
      hresult_test(d2d().factory()->CreateStrokeStyle, &stroke_style_props, nullptr, 0, &stroke_style);
      stroke_style_props.dashStyle = D2D1_DASH_STYLE_DASH;
      hresult_test(d2d().factory()->CreateStrokeStyle, &stroke_style_props, nullptr, 0, &dashed_stroke_style);
      return {};
    }
  };

public:
  static ID2D1SolidColorBrush* d2d_brush() noexcept { return slot::get()->solid_brush; }
  static const yw::color& color() noexcept { return slot::get()->color; }
  static bool dashed() noexcept { return slot::get()->dashed; }

  static ID2D1StrokeStyle* d2d_stroke() noexcept {
    const auto sp = slot::get();
    return sp->dashed ? sp->dashed_stroke_style : sp->stroke_style;
  }

  static void color(const yw::color& Color) noexcept {
    const auto sp = slot::get();
    sp->color = Color;
    sp->solid_brush->SetColor(reinterpret_cast<const D2D1_COLOR_F*>(&Color));
  }

  static void dashed(bool Dashed = true) noexcept { slot::get()->dashed = Dashed; }
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
  const wchar_t* get_name() const noexcept { return name ? name->c_str() : L""; }
  float get_size() const noexcept { return size.value_or(16.0f); }
  font_weight get_weight() const noexcept { return weight.value_or(font_weight::normal); }
  font_style get_style() const noexcept { return style.value_or(font_style::normal); }
  font_stretch get_stretch() const noexcept { return stretch.value_or(font_stretch::normal); }
};

inline const font_config font_config::default_{
  string<wchar_t>(), 16.0f, font_weight::normal, font_style::normal, font_stretch::normal};

/// MARK: dwrite

class dwrite {
  inline static void* ptr = nullptr;

  struct slot {
    IDWriteFactory1* factory{};
    IDWriteTextFormat* text_format{};

    slot() {
      if (auto res = _init_factory(); !res) res.error().go_off();
      if (auto res = _init_text_format(); !res) res.error().go_off();
    }

    static slot* get() noexcept {
      if (!ptr) ptr = new slot();
      return static_cast<slot*>(ptr);
    }

    std::expected<void, error> _init_factory() {
      hresult_test(
        ::DWriteCreateFactory, DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory1),
        reinterpret_cast<IUnknown**>(&factory));
      return {};
    }

    std::expected<void, error> _init_text_format() {
      hresult_test(
        factory->CreateTextFormat, font_config::default_.name->c_str(), nullptr,
        DWRITE_FONT_WEIGHT(*font_config::default_.weight), DWRITE_FONT_STYLE(*font_config::default_.style),
        DWRITE_FONT_STRETCH(*font_config::default_.stretch), font_config::default_.size.value_or(16.0f), L"",
        &text_format);
      hresult_test(text_format->SetTextAlignment, DWRITE_TEXT_ALIGNMENT_CENTER);
      return {};
    }
  };

public:
  static IDWriteFactory1* factory() noexcept { return slot::get()->factory; }
  static IDWriteTextFormat* text_format() noexcept { return slot::get()->text_format; }
};

/// MARK: coinit

class coinit {
  inline static void* ptr = nullptr;

  struct slot {
    slot() {
      if (auto res = initialize(); !res) res.error().go_off();
      system::com_list_to_release.push([]() { ::CoUninitialize(); });
    }

    std::expected<void, error> initialize() {
      hresult_test(::CoInitializeEx, nullptr, COINIT_MULTITHREADED);
      return {};
    }
  };

public:
  coinit() {
    if (!ptr) ptr = new slot();
  }
};

//// MARK: wic

class wic {
  inline static void* ptr = nullptr;

public:
  struct slot : general_slot {
    ::IWICImagingFactory2* factory{};

    slot() {
      const coinit coinit;
      if (auto res = _init_factory(); !res) res.error().go_off();
      system::com_list_to_release.push([&]() {
        if (factory) factory->Release();
      });
    }

    static slot* get() noexcept {
      if (!ptr) ptr = new slot();
      return static_cast<slot*>(ptr);
    }

    std::expected<void, error> _init_factory() {
      hresult_test(::CoCreateInstance, CLSID_WICImagingFactory2, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory));
      return {};
    }
  };

public:
  static ::IWICImagingFactory2* factory() noexcept { return slot::get()->factory; }
};

/// MARK: xaudio2

class xaudio2 {
  inline static void* ptr = nullptr;

public:
  struct slot {
    IXAudio2* device{};
    IXAudio2MasteringVoice* mastering_voice{};

    slot() {
      const coinit coinit;
      if (auto res = _init(); !res) res.error().go_off();
      system::com_list_to_release.push([&]() {
        if (mastering_voice) mastering_voice->DestroyVoice();
        if (device) device->Release();
      });
    }

    static slot* get() noexcept {
      if (!ptr) ptr = new slot();
      return static_cast<slot*>(ptr);
    }

    std::expected<void, error> _init() {
      hresult_test(::XAudio2Create, &device, 0, XAUDIO2_DEFAULT_PROCESSOR);
      hresult_test(device->CreateMasteringVoice, &mastering_voice);
      return {};
    }
  };

  static IXAudio2* device() noexcept { return slot::get()->device; }
  static IXAudio2MasteringVoice* mastering_voice() noexcept { return slot::get()->mastering_voice; }
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
