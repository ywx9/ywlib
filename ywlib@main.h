/**
 * @file  ywlib@main.h
 * @brief WIN32やDirectXに関係する定義をまとめたヘッダーファイル
 ******************************************************************************/

#pragma once

#include "ywlib@xv.h"
#define WIN32_LEAN_AND_MEAN
#include <Shlwapi.h>
#include <Windows.h>
#include <commdlg.h>
#include <d2d1_3.h>
#include <d3d11_3.h>
#include <d3dcompiler.h>
#include <dwrite_3.h>
#include <dxgi1_6.h>
#include <shellapi.h>
#include <wincodec.h>

#pragma comment(lib, "Comdlg32.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "uuid.lib")

#undef max
#undef min

#ifdef ywlib_debug
#define _DEBUG
#define ywlib_enable_console
#define ywee(Str) std::cerr << Str << std::endl
#else
#define ywee(Str) void()
#endif

namespace yw {

/// Rect Like
template<typename T> concept rect_like = requires {
  requires extent<T> == 4;
  requires arithmetic<remove_ref<element_t<T, 0>>>;
  requires arithmetic<remove_ref<element_t<T, 1>>>;
  requires arithmetic<remove_ref<element_t<T, 2>>>;
  requires arithmetic<remove_ref<element_t<T, 3>>>;
};

/// Throw IF Failed
inline constexpr auto tiff = overload{
  [](HRESULT B, source S = {}) {
    if (B < 0) {
      cat1 buf[128];
      FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, B,
                     MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buf, 128, nullptr);
      throw except(buf, mv(S));
    }
  },
  [](const auto& B, source S = {}) requires requires { { !B } -> convertible_to<bool>; } {
    if (!B) throw except("FAILED", mv(S));
  }};

/// COM Pointer
template<typename Com> class comptr {
  static_assert(requires(Com& a, IUnknown* b) {
    { a.QueryInterface(__uuidof(IUnknown), (void**)&b) } -> std::same_as<HRESULT>;
    { a.QueryInterface(&b) } -> same_as<HRESULT>;
    { a.Release() } -> same_as<ULONG>;
    { a.AddRef() } -> same_as<ULONG>;
  });
  template<typename Uy> friend class comptr;
  Com* _ptr = nullptr;
  void _addref() const noexcept { _ptr ? void(_ptr->AddRef()) : void(); }
  void _release() noexcept { _ptr ? void(exchange(_ptr, nullptr)->Release()) : void(); }
  void _copy(Com* a) noexcept { _ptr == a ? void() : (_release(), _ptr = a, _addref()); }
  void _move(auto& a) noexcept { _ptr == a._ptr ? void() : void((_release(), _ptr = a._ptr, a._ptr = nullptr)); }
public:
  class reference : public Com {
    void operator&() const = delete;
    ULONG __stdcall AddRef() { return Com::AddRef(); }
    ULONG __stdcall Release() { return Com::Release(); }
  };
  ~comptr() noexcept { _release(); }
  comptr() noexcept = default;
  comptr(comptr&& a) noexcept : _ptr{a._ptr} { a._ptr = nullptr; }
  comptr(const comptr& a) noexcept : _ptr{a._ptr} { _addref(); }
  comptr(Com*&& a) noexcept : _ptr(a) {}
  template<typename Uy> comptr(comptr<Uy>&& a) noexcept : _ptr{a._ptr} { a._ptr = nullptr; }
  template<typename Uy> comptr(const comptr<Uy>& a) noexcept : _ptr{a._ptr} { _addref(); }
  comptr& operator=(const comptr& a) noexcept { return _copy(a._ptr), *this; }
  template<typename Uy> comptr& operator=(const comptr<Uy>& a) noexcept { return _copy(a._ptr), *this; }
  template<typename Uy> comptr& operator=(comptr<Uy>&& a) noexcept { return _move(a), *this; }
  template<typename Uy> bool operator==(const comptr<Uy>& a) noexcept { return _ptr == a._ptr; }
  explicit operator bool() const noexcept { return _ptr; }
  operator reference*() const noexcept { return static_cast<reference*>(_ptr); }
  reference* operator->() const noexcept { return static_cast<reference*>(_ptr); }
  void reset() noexcept { _release(); }
  Com* get() const noexcept { return _ptr; }
  Com** operator&() {
    if (_ptr != nullptr) throw std::exception("You must reset comptr or use &asconst(comptr)");
    return &_ptr;
  }
  Com* const* operator&() const noexcept { return &_ptr; }
  template<typename Uy> HRESULT as(comptr<Uy>& a) const noexcept { return _ptr->QueryInterface(IID_PPV_ARGS(&a)); }
};

/// ストップウォッチ
class stopwatch {
  mutable LARGE_INTEGER _li;
  fat8 _freq;
  int8 _last;
public:
  stopwatch() noexcept : _li{}, _freq{}, _last{} { QueryPerformanceFrequency(&_li), _freq = static_cast<fat8>(_li.QuadPart), QueryPerformanceCounter(&_li), _last = _li.QuadPart; }
  fat8 read() const noexcept { return QueryPerformanceCounter(&_li), (_li.QuadPart - _last) / _freq; }
  fat8 push() noexcept { return QueryPerformanceCounter(&_li), (_li.QuadPart - exchange(_last, _li.QuadPart)) / _freq; }
  void start() noexcept { QueryPerformanceCounter(&_li), _last = _li.QuadPart; }
};

/// メインシステム
namespace main::system {
inline HWND hwnd{};
inline HFONT hfont{};
inline HINSTANCE hinstance{};
inline comptr<ID3D11Device1> d3d_device{};
inline comptr<ID3D11DeviceContext1> d3d_context{};
inline comptr<IDXGISwapChain1> swap_chain{};
inline comptr<ID2D1Device5> d2d_device{};
inline comptr<ID2D1DeviceContext5> d2d_context{};
inline comptr<ID2D1Factory6> d2d_factory{};
inline comptr<IDWriteFactory> dw_factory{};
inline comptr<IWICImagingFactory2> wic_factory{};
inline array<str2> args{};
inline stopwatch timer{};
inline fat8 spf{}, fps{};
inline list<RECT, bool> mouse{};
inline comptr<ID2D1Bitmap1> bitmap{};
inline nat4 width{}, height{};
LRESULT WINAPI wndproc(HWND, UINT, WPARAM, LPARAM);
}

/// Brush class
/// @note `brush(color) -> dynamic-brush`
/// @note `brush<color>{} -> static-brush`
template<color Color = color::undefined> class brush;

/// Dynamic brush class
template<> class brush<color::undefined> {
protected:
  comptr<ID2D1SolidColorBrush> d2d_brush;
public:
  explicit operator bool() const noexcept { return bool(d2d_brush); }
  operator comptr<ID2D1SolidColorBrush>::reference*() const noexcept { return d2d_brush; }
  brush() noexcept = default;
  brush(color Color) { tiff(main::system::d2d_context->CreateSolidColorBrush(bitcast<D2D1_COLOR_F>(Color), &d2d_brush)); }
};

/// Static brush class
template<color Color> requires(Color != color::undefined) class brush<Color> {
  inline static yw::brush<color::undefined> _{};
public:
  explicit operator bool() const noexcept { return bool(_) ? bool(_) : bool(_ = yw::brush<color::undefined>(Color)); }
  operator const yw::brush<color::undefined>&() const noexcept { return bool(_) ? _ : (_ = yw::brush<color::undefined>(Color)); }
  operator comptr<ID2D1SolidColorBrush>::reference*() const noexcept { return bool(_) ? _ : (_ = yw::brush<color::undefined>(Color)); }
};

template<typename... Ts> brush(Ts&&...) -> brush<color::undefined>;

/// Class for managing static fonts.
template<natt Size, string Name = L"Yu Gothic UI",
         intt Alignment = -1, bool Bold = false, bool Oblique = false>
requires(0 < Size && same_as<cat2, typename decltype(Name)::value_type>) class font {
protected:
  inline static constexpr auto dw_alignment =
    Alignment < 0 ? DWRITE_TEXT_ALIGNMENT_LEADING
                  : (Alignment > 0 ? DWRITE_TEXT_ALIGNMENT_TRAILING : DWRITE_TEXT_ALIGNMENT_CENTER);
  inline static constexpr auto dw_weight = Bold ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_NORMAL;
  inline static constexpr auto dw_style = Oblique ? DWRITE_FONT_STYLE_OBLIQUE : DWRITE_FONT_STYLE_NORMAL;
  inline static comptr<IDWriteTextFormat> dw_format{};
  static void initialize() {
    try {
      tiff(main::system::dw_factory->CreateTextFormat(
        name.data(), 0, dw_weight, dw_style, DWRITE_FONT_STRETCH_NORMAL, size, L"", &dw_format));
      tiff(dw_format->SetTextAlignment(dw_alignment));
    } catch (const std::exception& E) { ywee(E.what()), throw except(E); }
  }
public:
  inline static constexpr auto size = static_cast<fat4>(Size);
  inline static constexpr auto name = Name;
  inline static constexpr auto alignment = Alignment;
  inline static constexpr auto bold = Bold, oblique = Oblique;
  constexpr font() noexcept = default;
  /// Checks if this object is valid.
  explicit operator bool() const noexcept { return (dw_format ? void() : initialize()), bool(dw_format); }
  /// Obtains the pointer to the font object.
  operator comptr<IDWriteTextFormat>::reference*() const noexcept { return (dw_format ? void() : initialize()), dw_format; }
};

template<typename T> class staging_buffer;

/// Base class for creating a buffer.
template<typename T> class buffer {
protected:
  comptr<ID3D11Buffer> d3d_buffer;
  buffer(const buffer&) = delete;
  buffer& operator=(const buffer&) = delete;
  buffer(natt Count) noexcept : count(static_cast<nat4>(Count)) {}
public:
  using value_type = T;
  const nat4 count{};
  buffer() noexcept = default;
  buffer(buffer&& B) noexcept : d3d_buffer{mv(B.d3d_buffer)}, count{B.count} {}
  buffer& operator=(buffer&& B) noexcept {
    d3d_buffer = mv(B.d3d_buffer);
    const_cast<nat4&>(count) = B.count;
    return *this;
  }
  /// Checks if this object is valid.
  explicit operator bool() const noexcept { return bool(d3d_buffer); }
  /// Obtains the pointer to the buffer object.
  operator comptr<ID3D11Buffer>::reference*() const noexcept { return d3d_buffer; }
  /// Copies the contents of `Src` to this buffer.
  void from(const buffer& Src) {
    tiff(count == Src.count);
    main::system::d3d_context->CopyResource(*this, Src);
  }
  /// Copies this contents to CPU.
  array<T> to_cpu() const;
  /// Copies this contents to CPU.
  array<T> to_cpu(staging_buffer<T>& Staging) const;
};

/// Class for creating a staging buffer.
template<typename T> class staging_buffer : public buffer<T> {
public:
  staging_buffer() noexcept = default;
  /// Creates a staging buffer and copies the contents of `Src` to it.
  explicit staging_buffer(const buffer<T>& Src) : staging_buffer(Src.count) { buffer<T>::from(Src); }
  /// Creates a empty staging buffer.
  staging_buffer(natt Count) : buffer<T>(Count) {
    D3D11_BUFFER_DESC desc{(nat4)(sizeof(T) * Count), D3D11_USAGE_STAGING, 0, D3D11_CPU_ACCESS_READ};
    tiff(main::system::d3d_device->CreateBuffer(&desc, nullptr, &(buffer<T>::d3d_buffer)));
  }
  /// Copies the contents of `Src` to this.
  void from(const buffer<T>& Src) { buffer<T>::from(Src); }
  /// Copies this contents to CPU.
  array<T> to_cpu() const {
    array<T> a(buffer<T>::count);
    D3D11_MAPPED_SUBRESOURCE mapped;
    tiff(main::system::d3d_context->Map(*this, 0, D3D11_MAP_READ, 0, &mapped));
    memcpy(a.data(), mapped.pData, (nat4)sizeof(T) * buffer<T>::count);
    main::system::d3d_context->Unmap(*this, 0);
    return a;
  }
};

template<typename T> array<T> buffer<T>::to_cpu() const {
  return staging_buffer<T>(*this).to_cpu();
}
template<typename T> array<T> buffer<T>::to_cpu(staging_buffer<T>& S) const {
  return S.from(*this), S.to_cpu();
}

/// Class for creating a vertex buffer.
template<typename T> class vertex_buffer : public buffer<T> {
public:
  vertex_buffer() noexcept = default;
  template<cnt_range_of<T> R> vertex_buffer(R&& Range) : vertex_buffer(yw::data(Range), yw::size(Range)) {}
  vertex_buffer(const T* Data, natt Count) : buffer<T>(Count) {
    D3D11_BUFFER_DESC desc{nat4(sizeof(T) * Count), D3D11_USAGE_DEFAULT, D3D11_BIND_VERTEX_BUFFER};
    if (Data) {
      D3D11_SUBRESOURCE_DATA data{.pSysMem = Data};
      tiff(main::system::d3d_device->CreateBuffer(&desc, &data, &(buffer<T>::d3d_buffer)));
    } else tiff(main::system::d3d_device->CreateBuffer(&desc, 0, &(buffer<T>::d3d_buffer)));
  }
  /// `Range`で内容を上書きする
  template<cnt_range_of<T> R> void load(R&& Range) const { load(yw::data(Range), yw::size(Range)); }
  /// 内容を上書きする
  void load(const T* Data, natt Count) const {
    tiff(Count == buffer<T>::count);
    main::system::d3d_context->UpdateSubresource(*this, 0, 0, Data, (nat4)sizeof(T), nat4(sizeof(T) * Count));
  }
  /// Vertex Shaderにセットする
  void to_vs(nat4 Slot = 0) const {
    nat4 a((nat4)sizeof(T)), b{};
    main::system::d3d_context->IASetVertexBuffers(Slot, 1, &(buffer<T>::d3d_buffer), &a, &b);
  }
};

template<> class vertex_buffer<none> : public buffer<none> {
public:
  explicit operator bool() const noexcept { return buffer<none>::count != 0; }
  operator comptr<ID3D11Buffer>::reference*() const noexcept { return nullptr; }
  vertex_buffer() noexcept = default;
  vertex_buffer(natt Count) noexcept : buffer<none>(Count) {}
  /// Vertex Shaderにセットする
  void to_vs(nat4 Slot = 0) const { main::system::d3d_context->IASetVertexBuffers(Slot, 1, 0, 0, 0); }
};

// index_buffer

class index_buffer : public buffer<nat4> {
public:
  explicit operator bool() const noexcept { return buffer<nat4>::operator bool(); }
  operator comptr<ID3D11Buffer>::reference*() const noexcept { return buffer<nat4>::d3d_buffer; }
  index_buffer() noexcept = default;
  template<cnt_range_of<nat4> R> index_buffer(R&& Range) : index_buffer(yw::data(Range), yw::size(Range)) {}
  index_buffer(const nat4* Data, natt Count) : buffer<nat4>(Count) {
    D3D11_BUFFER_DESC desc{nat4(sizeof(nat4) * Count), D3D11_USAGE_DEFAULT, D3D11_BIND_INDEX_BUFFER};
    if (Data) {
      D3D11_SUBRESOURCE_DATA data{.pSysMem = Data};
      tiff(main::system::d3d_device->CreateBuffer(&desc, &data, &(buffer<nat4>::d3d_buffer)));
    } else tiff(main::system::d3d_device->CreateBuffer(&desc, nullptr, &(buffer<nat4>::d3d_buffer)));
  }
  template<cnt_range_of<nat4> R> void load(R&& Range) const { load(yw::data(Range), yw::size(Range)); }
  void load(const nat4* Data, natt Count) const {
    tiff(Count == buffer<nat4>::count);
    main::system::d3d_context->UpdateSubresource(*this, 0, 0, Data, (nat4)sizeof(nat4), nat4(sizeof(nat4) * Count));
  }
  void to_vs() const { main::system::d3d_context->IASetIndexBuffer(*this, DXGI_FORMAT_R32_UINT, 0); }
};

/// Class for creating a constant buffer.
template<typename T> requires((sizeof(T) & 0xf) == 0)
class constant_buffer : public buffer<T> {
public:
  explicit operator bool() const noexcept { return buffer<T>::operator bool(); }
  operator comptr<ID3D11Buffer>::reference*() const noexcept { return buffer<T>::d3d_buffer; }
  constant_buffer() noexcept : buffer<T>(1) {}
  constant_buffer(const T& Value) : constant_buffer() {
    static constexpr D3D11_BUFFER_DESC desc{(nat4)sizeof(T), D3D11_USAGE_DYNAMIC, D3D11_BIND_CONSTANT_BUFFER, D3D11_CPU_ACCESS_WRITE};
    [&](D3D11_SUBRESOURCE_DATA d) { tiff(main::system::d3d_device->CreateBuffer(&desc, &d, &(buffer<T>::d3d_buffer))); }({&Value});
  }
  void load(const T& Value) {
    if (*this) {
      D3D11_MAPPED_SUBRESOURCE mapped;
      tiff(main::system::d3d_context->Map(*this, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped));
      *reinterpret_cast<T*>(mapped.pData) = Value, main::system::d3d_context->Unmap(*this, 0);
    } else *this = constant_buffer(Value);
  }
  template<typename U> requires assignable<T&, U> || vassignable<T&, U> void load(U&& Value) {
    if (*this) {
      D3D11_MAPPED_SUBRESOURCE mapped;
      tiff(main::system::d3d_context->Map(*this, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped));
      if constexpr (assignable<T&, U>) *reinterpret_cast<T*>(mapped.pData) = Value;
      else vassign(*reinterpret_cast<T*>(mapped.pData), Value);
      main::system::d3d_context->Unmap(*this, 0);
    } else {
      T temp;
      if constexpr (assignable<T&, U>) temp = Value;
      else vassign(temp, Value);
      *this = constant_buffer(temp);
    }
  }
  void load(invocable<T&> auto&& Func) {
    if (*this) {
      D3D11_MAPPED_SUBRESOURCE mapped;
      tiff(main::system::d3d_context->Map(*this, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped));
      Func(*reinterpret_cast<T*>(mapped.pData)), main::system::d3d_context->Unmap(*this, 0);
    } else [&](T temp) { Func(temp), *this = constant_buffer(temp); }({});
  }
  void to_vs(natt Slot = 0) const { main::system::d3d_context->VSSetConstantBuffers(nat4(Slot), 1, &(buffer<T>::d3d_buffer)); }
  void to_gs(natt Slot = 0) const { main::system::d3d_context->GSSetConstantBuffers(nat4(Slot), 1, &(buffer<T>::d3d_buffer)); }
  void to_ps(natt Slot = 0) const { main::system::d3d_context->PSSetConstantBuffers(nat4(Slot), 1, &(buffer<T>::d3d_buffer)); }
  void to_cs(natt Slot = 0) const { main::system::d3d_context->CSSetConstantBuffers(nat4(Slot), 1, &(buffer<T>::d3d_buffer)); }
};

template<typename T> constant_buffer(const T&) -> constant_buffer<T>;

/// Class for creating a structured buffer.
template<typename T> class structured_buffer : public buffer<T> {
public:
  comptr<ID3D11ShaderResourceView> d3d_srv;
  explicit operator bool() const noexcept { return buffer<T>::operator bool() && bool(d3d_srv); }
  operator comptr<ID3D11ShaderResourceView>::reference*() const noexcept { return d3d_srv; }
  structured_buffer() noexcept = default;
  explicit structured_buffer(const buffer<T>& Buffer) : structured_buffer(nullptr, Buffer.count) { buffer<T>::from(Buffer); }
  template<cnt_range_of<T> R> explicit structured_buffer(R&& Range) : structured_buffer(yw::data(Range), yw::size(Range)) {}
  structured_buffer(const T* Data, natt Count) : buffer<T>(Count) {
    D3D11_BUFFER_DESC desc{nat4(sizeof(T) * Count), D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE,
                           0, D3D11_RESOURCE_MISC_BUFFER_STRUCTURED, (nat4)sizeof(T)};
    if (Data) {
      D3D11_SUBRESOURCE_DATA data{.pSysMem = Data, .SysMemPitch = (nat4)sizeof(T)};
      tiff(main::system::d3d_device->CreateBuffer(&desc, &data, &(buffer<T>::d3d_buffer)));
    } else tiff(main::system::d3d_device->CreateBuffer(&desc, nullptr, &(buffer<T>::d3d_buffer)));
    D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc{
      DXGI_FORMAT_UNKNOWN, D3D11_SRV_DIMENSION_BUFFER, D3D11_BUFFER_SRV{0, nat4(Count)}};
    tiff(main::system::d3d_device->CreateShaderResourceView(*this, &srv_desc, &d3d_srv));
  }
  template<cnt_range_of<T> R> void load(R&& Range) { load(yw::data(Range), yw::size(Range)); }
  void load(const T* Data, natt Count) {
    tiff(buffer<T>::cout == Count);
    main::system::d3d_context->UpdateSubresource(*this, 0, 0, Data, (nat4)sizeof(T), nat4(sizeof(T) * Count));
  }
  void to_vs(nat4 Slot = 0) const { main::system::d3d_context->VSSetShaderResources(Slot, 1, &d3d_srv); }
  void to_gs(nat4 Slot = 0) const { main::system::d3d_context->GSSetShaderResources(Slot, 1, &d3d_srv); }
  void to_ps(nat4 Slot = 0) const { main::system::d3d_context->PSSetShaderResources(Slot, 1, &d3d_srv); }
  void to_cs(nat4 Slot = 0) const { main::system::d3d_context->CSSetShaderResources(Slot, 1, &d3d_srv); }
};

template<typename T> structured_buffer(const buffer<T>&) -> structured_buffer<T>;
template<typename T> structured_buffer(const T*, natt) -> structured_buffer<T>;
template<cnt_range R> structured_buffer(R&&) -> structured_buffer<iter_value_t<R>>;

/// Class for creating an unordered buffer.
template<typename T> class unordered_buffer : public buffer<T> {
public:
  comptr<ID3D11UnorderedAccessView> d3d_uav{};
  explicit operator bool() const noexcept { return buffer<T>::operator bool() && bool(d3d_uav); }
  operator comptr<ID3D11UnorderedAccessView>::reference*() const noexcept { return d3d_uav; }
  unordered_buffer() noexcept = default;
  explicit unordered_buffer(const structured_buffer<T>& Sb) : unordered_buffer(nullptr, Sb.size()) { buffer<T>::from(Sb); }
  template<cnt_range_of<T> R> explicit unordered_buffer(R&& Range) : unordered_buffer(yw::data(Range), yw::size(Range)) {}
  unordered_buffer(const T* Data, natt Count) : buffer<T>(Count) {
    D3D11_BUFFER_DESC bdesc{nat4(sizeof(T) * Count), D3D11_USAGE_DEFAULT, D3D11_BIND_UNORDERED_ACCESS,
                            0, D3D11_RESOURCE_MISC_BUFFER_STRUCTURED, (nat4)sizeof(T)};
    if (Data) {
      D3D11_SUBRESOURCE_DATA data{.pSysMem = Data, .SysMemPitch = (nat4)sizeof(T)};
      tiff(main::system::d3d_device->CreateBuffer(&bdesc, &data, &(buffer<T>::d3d_buffer)));
    } else tiff(main::system::d3d_device->CreateBuffer(&bdesc, nullptr, &(buffer<T>::d3d_buffer)));
    tiff(main::system::d3d_device->CreateUnorderedAccessView(*this, nullptr, &d3d_uav));
  }
  void to_cs(nat4 Slot = 0) const { main::system::d3d_context->CSSetUnorderedAccessViews(Slot, 1, &d3d_uav, nullptr); }
};

/// Class for creating 3D renderer.
template<specialization_of<typepack> VSResources, specialization_of<typepack> VSConstant,
         specialization_of<typepack> GSResources = typepack<>, specialization_of<typepack> GSConstant = typepack<>,
         specialization_of<typepack> PSResources = typepack<>, specialization_of<typepack> PSConstant = typepack<>>
class renderer {
public:
  comptr<ID3D11VertexShader> d3d_vs;
  comptr<ID3D11GeometryShader> d3d_gs;
  comptr<ID3D11PixelShader> d3d_ps;
  using vs_sr_list = list<>::from_typepack<VSResources, const int&>;
  using vs_cb_list = list<>::from_typepack<VSConstant, const constant_buffer<fat4[4]>&>;
  using gs_sr_list = list<>::from_typepack<GSResources, const int&>;
  using gs_cb_list = list<>::from_typepack<GSConstant, const constant_buffer<fat4[4]>&>;
  using ps_sr_list = list<>::from_typepack<PSResources, const int&>;
  using ps_cb_list = list<>::from_typepack<PSConstant, const constant_buffer<fat4[4]>&>;
  renderer(stv1 HLSL) {
    comptr<ID3DBlob> b, e;
    if (0 > D3DCompile(HLSL.data(), HLSL.size(), 0, 0, 0, "vsmain", "vs_5_0",
                       D3D10_SHADER_ENABLE_STRICTNESS, 0, &b, &e)) throw except((cat1*)e->GetBufferPointer());
    tiff(main::system::d3d_device->CreateVertexShader(b->GetBufferPointer(), b->GetBufferSize(), 0, &d3d_vs));
    b.reset(), e.reset();
    if (0 > D3DCompile(HLSL.data(), HLSL.size(), 0, 0, 0, "psmain", "ps_5_0",
                       D3D10_SHADER_ENABLE_STRICTNESS, 0, &b, &e)) throw except((cat1*)e->GetBufferPointer());
    tiff(main::system::d3d_device->CreatePixelShader(b->GetBufferPointer(), b->GetBufferSize(), 0, &d3d_ps));
    b.reset(), e.reset();
    if (HLSL.find("gsmain") == HLSL.npos) return;
    if (0 > D3DCompile(HLSL.data(), HLSL.size(), 0, 0, 0, "gsmain", "gs_5_0",
                       D3D10_SHADER_ENABLE_STRICTNESS, 0, &b, &e)) throw except((cat1*)e->GetBufferPointer());
    tiff(main::system::d3d_device->CreateGeometryShader(b->GetBufferPointer(), b->GetBufferSize(), 0, &d3d_gs));
  }
  explicit operator bool() const noexcept { return bool(d3d_vs) && bool(d3d_ps); }
  void operator()(
    natt VertexCount, vs_sr_list vs_resources, vs_cb_list vs_constant_buffers,
    gs_sr_list gs_resources = {}, gs_cb_list gs_constant_buffers = {},
    ps_sr_list ps_resources = {}, ps_cb_list ps_constant_buffers = {}) const {
    [&]<natt... Is>(sequence<Is...>) { ((yw::get<Is>(vs_resources).to_vs(Is)), ...); }(make_indices_for<vs_sr_list>{});
    [&]<natt... Is>(sequence<Is...>) { ((yw::get<Is>(gs_resources).to_gs(Is)), ...); }(make_indices_for<gs_sr_list>{});
    [&]<natt... Is>(sequence<Is...>) { ((yw::get<Is>(ps_resources).to_ps(Is)), ...); }(make_indices_for<ps_sr_list>{});
    [&]<natt... Is>(sequence<Is...>) { ((yw::get<Is>(vs_constant_buffers).to_vs(Is)), ...); }(make_indices_for<vs_cb_list>{});
    [&]<natt... Is>(sequence<Is...>) { ((yw::get<Is>(gs_constant_buffers).to_gs(Is)), ...); }(make_indices_for<gs_cb_list>{});
    [&]<natt... Is>(sequence<Is...>) { ((yw::get<Is>(ps_constant_buffers).to_ps(Is)), ...); }(make_indices_for<ps_cb_list>{});
    main::system::d3d_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    main::system::d3d_context->IASetInputLayout(nullptr);
    main::system::d3d_context->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
    main::system::d3d_context->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
    main::system::d3d_context->VSSetShader(d3d_vs, nullptr, 0);
    main::system::d3d_context->GSSetShader(d3d_gs, nullptr, 0);
    main::system::d3d_context->PSSetShader(d3d_ps, nullptr, 0);
    main::system::d3d_context->Draw(nat4(VertexCount), 0);
  }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  BITMAP, TEXTURE, CANVAS, CAMERA

/// BITMAP
class bitmap {
  bitmap(bitmap&) = delete;
  bitmap& operator=(bitmap&) = delete;
protected:
  /// ID2D1Bitmap1
  comptr<ID2D1Bitmap1> d2d_bitmap{};
public:
  /// Width of bitmap
  const nat4 width{};
  /// Height of bitmap
  const nat4 height{};
  /// Checks if bitmap has been initialized.
  explicit operator bool() const noexcept { return bool(d2d_bitmap); }
  /// Converts to restricted pointer of ID2D1Bitmap1.
  operator comptr<ID2D1Bitmap1>::reference*() const noexcept { return d2d_bitmap; }
  /// Accesses to ID2D1Bitmap1 methods.
  auto* operator->() const noexcept { return d2d_bitmap.operator->(); }
  /// Default constructor
  bitmap() noexcept = default;
  /// Move constructor
  bitmap(bitmap&& Bitmap) noexcept : d2d_bitmap{mv(Bitmap.d2d_bitmap)}, width{Bitmap.width}, height{Bitmap.height} {}
  /// Move assignment
  bitmap& operator=(bitmap&& Bitmap) noexcept {
    d2d_bitmap = mv(Bitmap.d2d_bitmap);
    const_cast<nat4&>(width) = Bitmap.width;
    const_cast<nat4&>(height) = Bitmap.height;
    return *this;
  }
  /// Creates an empty bitmap with the specified size.
  bitmap(natt Width, natt Height) : width(int4(Width)), height(int4(Height)) {
    auto p = D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET, D2D1::PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));
    tiff(main::system::d2d_context->CreateBitmap(D2D1::SizeU(width, height), nullptr, 0, p, &d2d_bitmap));
  }
  /// Creates a bitmap from an image file.
  explicit bitmap(const path& Image) {
    comptr<IWICBitmapDecoder> decoder{};
    tiff(main::system::wic_factory->CreateDecoderFromFilename(Image.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &decoder));
    comptr<IWICBitmapFrameDecode> frame;
    tiff(decoder->GetFrame(0, &frame));
    tiff(frame->GetSize(const_cast<nat4*>(&width), const_cast<nat4*>(&height)));
    WICPixelFormatGUID pixel_format, guid = GUID_WICPixelFormat32bppPRGBA;
    comptr<IWICFormatConverter> fc;
    tiff(frame->GetPixelFormat(&pixel_format));
    tiff(main::system::wic_factory->CreateFormatConverter(&fc));
    [&](BOOL b) { tiff(fc->CanConvert(pixel_format, guid, &b)), tiff(b); }({});
    tiff(fc->Initialize(frame, guid, WICBitmapDitherTypeErrorDiffusion, nullptr, 0, WICBitmapPaletteTypeMedianCut));
    tiff(main::system::d2d_context->CreateBitmapFromWicBitmap(fc, &d2d_bitmap));
  }
  /// Begins drawing to bitmap.
  void begin_draw() const {
    main::system::d2d_context->SetTarget(*this), main::system::d2d_context->BeginDraw();
  }
  /// Begins drawing to bitmap after filling with the specified color.
  void begin_draw(const color& Fill) const {
    begin_draw(), main::system::d2d_context->Clear(bitcast<D2D1_COLOR_F>(Fill));
  }
  /// Ends drawing to bitmap.
  void end_draw() const {
    tiff(main::system::d2d_context->EndDraw());
  }
  /// Save bitmap as PNG file.
  void to_png(const path& Path) const {
    try {
      if (file::exists(Path)) file::remove(Path);
      auto& factory = main::system::wic_factory;
      comptr<IWICStream> stream{};
      tiff(factory->CreateStream(&stream));
      tiff(stream->InitializeFromFilename(Path.c_str(), GENERIC_WRITE));
      comptr<IWICBitmapEncoder> encoder{};
      tiff(factory->CreateEncoder(GUID_ContainerFormatPng, nullptr, &encoder));
      tiff(encoder->Initialize(stream, WICBitmapEncoderNoCache));
      comptr<IWICBitmapFrameEncode> frame{};
      tiff(encoder->CreateNewFrame(&frame, nullptr));
      tiff(frame->Initialize(nullptr));
      comptr<IWICImageEncoder> image_encoder{};
      tiff(factory->CreateImageEncoder(main::system::d2d_device, &image_encoder));
      tiff(image_encoder->WriteFrame(d2d_bitmap, frame, nullptr));
      frame->Commit(), encoder->Commit(), stream->Commit(STGC_DEFAULT);
    } catch (const std::exception& E) { throw except(E); }
  }
  /// Draws this to another
  void draw(const rect_like auto& Rect, fat4 Opacity = 1.0f) const {
    main::system::d2d_context->DrawBitmap(d2d_bitmap, D2D1_RECT_F{fat4(get<0>(Rect)), fat4(get<1>(Rect)), fat4(get<2>(Rect)), fat4(get<3>(Rect))}, Opacity);
  }
};

/// TEXTURE
class texture : public bitmap {
  void initialize() {
    comptr<IDXGISurface> surface;
    tiff(operator comptr<ID2D1Bitmap1>::reference*()->GetSurface(&surface));
    tiff(surface->QueryInterface(&d3d_texture));
    D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc{
      .Format = DXGI_FORMAT_R8G8B8A8_UNORM, .ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D, .Texture2D = {0, 1}};
    tiff(main::system::d3d_device->CreateShaderResourceView(*this, &srv_desc, &d3d_srv));
  }
protected:
  /// ID3D11Texture2D
  comptr<ID3D11Texture2D> d3d_texture{};
  /// ID3D11ShaderResourceView
  comptr<ID3D11ShaderResourceView> d3d_srv{};
public:
  /// Checks if this has been initialized.
  explicit operator bool() const noexcept { return bitmap::operator bool() && bool(d3d_texture) && bool(d3d_srv); }
  /// Converts to restricted pointer to ID3D11Texture2D.
  operator comptr<ID3D11Texture2D>::reference*() const noexcept { return d3d_texture; }
  /// Converts to restricted pointer to ID3D11ShaderResourceView.
  operator comptr<ID3D11ShaderResourceView>::reference*() const noexcept { return d3d_srv; }
  /// Accesses to ID3D11ShaderResouceView methods.
  auto* operator->() const noexcept { return d3d_srv.operator->(); }
  /// Default constructor
  texture() noexcept = default;
  /// Creates texture from an image file.
  explicit texture(const path& File) : bitmap(File) { initialize(); }
  /// Creates texture from an bitmap.
  texture(bitmap&& Bitmap) : bitmap(mv(Bitmap)) { initialize(); }
  /// Creates an empty texture with the specified size.
  texture(natt Width, natt Height) : bitmap(Width, Height) { initialize(); }
  /// Sets SRV to vertex shader.
  void to_vs(nat4 Slot) const { main::system::d3d_context->VSSetShaderResources(Slot, 1, &d3d_srv); }
  /// Sets SRV to pixel shader.
  void to_ps(nat4 Slot) const { main::system::d3d_context->PSSetShaderResources(Slot, 1, &d3d_srv); }
  /// Sets SRV to geometry shader.
  void to_gs(nat4 Slot) const { main::system::d3d_context->GSSetShaderResources(Slot, 1, &d3d_srv); }
  /// Sets SRV to compute shader.
  void to_cs(nat4 Slot) const { main::system::d3d_context->CSSetShaderResources(Slot, 1, &d3d_srv); }
};

/// CANVAS
class canvas : public texture {
  void initialize() {
    comptr<ID3D11Texture2D> temp;
    if (const_cast<nat4&>(msaa) = msaa < 2 ? 0 : (msaa < 4 ? 2 : (msaa < 8 ? 4 : 8)); msaa) {
      D3D11_TEXTURE2D_DESC desc{
        width, height, 1, 1, DXGI_FORMAT_D24_UNORM_S8_UINT, {msaa, 0}, D3D11_USAGE_DEFAULT, D3D11_BIND_DEPTH_STENCIL};
      tiff(main::system::d3d_device->CreateTexture2D(&desc, nullptr, &temp));
      tiff(main::system::d3d_device->CreateDepthStencilView(temp, nullptr, &d3d_dsv));
      temp.reset(), desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM, desc.BindFlags = D3D11_BIND_RENDER_TARGET;
      tiff(main::system::d3d_device->CreateTexture2D(&desc, nullptr, &temp));
      D3D11_RENDER_TARGET_VIEW_DESC rtv_desc{DXGI_FORMAT_R8G8B8A8_UNORM, D3D11_RTV_DIMENSION_TEXTURE2DMS};
      tiff(main::system::d3d_device->CreateRenderTargetView(temp, &rtv_desc, &d3d_rtv));
    } else {
      D3D11_TEXTURE2D_DESC desc{
        width, height, 1, 1, DXGI_FORMAT_D24_UNORM_S8_UINT, {1, 0}, D3D11_USAGE_DEFAULT, D3D11_BIND_DEPTH_STENCIL};
      tiff(main::system::d3d_device->CreateTexture2D(&desc, nullptr, &temp));
      tiff(main::system::d3d_device->CreateDepthStencilView(temp, nullptr, &d3d_dsv));
      D3D11_RENDER_TARGET_VIEW_DESC rtv_desc{DXGI_FORMAT_R8G8B8A8_UNORM, D3D11_RTV_DIMENSION_TEXTURE2D};
      tiff(main::system::d3d_device->CreateRenderTargetView(*this, &rtv_desc, &d3d_rtv));
    }
  }
protected:
  /// ID3D11RenderTargetView
  comptr<ID3D11RenderTargetView> d3d_rtv{};
  /// ID3D11DepthStencilView
  comptr<ID3D11DepthStencilView> d3d_dsv{};
public:
  /// Sampling number for MSAA
  const nat4 msaa{};
  /// Checks if this has been initialized.
  explicit operator bool() const noexcept { return texture::operator bool() && bool(d3d_rtv) && bool(d3d_dsv); }
  /// Converts to a restricted pointer to ID3D11RenderTargetView.
  operator comptr<ID3D11RenderTargetView>::reference*() const noexcept { return d3d_rtv; }
  /// Accesses to ID3D11RenderTargetView methods.
  comptr<ID3D11RenderTargetView>::reference* operator->() const noexcept { return d3d_rtv; }
  /// Default constructor
  canvas() noexcept = default;
  /// Move constructor
  canvas(canvas&& Canvas) noexcept : texture(mv(Canvas)), d3d_rtv{mv(Canvas.d3d_rtv)}, d3d_dsv{mv(Canvas.d3d_dsv)}, msaa{Canvas.msaa} {}
  /// Move assignment
  canvas& operator=(canvas&& Canvas) noexcept {
    texture::operator=(mv(Canvas));
    d3d_rtv = mv(Canvas.d3d_rtv);
    d3d_dsv = mv(Canvas.d3d_dsv);
    const_cast<nat4&>(msaa) = Canvas.msaa;
    return *this;
  }
  /// Creates a canvas from a texture.
  canvas(yw::texture&& Texture, natt Msaa = {}) : yw::texture(mv(Texture)), msaa(nat4(Msaa)) { initialize(); }
  /// Creates an empty canvas with the specified size.
  canvas(natt Width, natt Height, natt Msaa = {}) : yw::texture(Width, Height), msaa(nat4(Msaa)) { initialize(); }
  /// Begins rendering.
  template<typename... RTV_OR_UAV> void begin_render(RTV_OR_UAV&&... As) const
    requires(bool(convertible_to<RTV_OR_UAV, ID3D11RenderTargetView*> ^ convertible_to<RTV_OR_UAV, ID3D11UnorderedAccessView*>) && ...) {
    main::system::d3d_context->ClearDepthStencilView(d3d_dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    [](D3D11_VIEWPORT vp) { main::system::d3d_context->RSSetViewports(1, &vp); }({0, 0, (fat4)width, (fat4)height, 0, 1});
    static constexpr nat4 m = nat4(counts<convertible_to<RTV_OR_UAV, ID3D11RenderTargetView*>...>), n = nat4(sizeof...(RTV_OR_UAV) - m);
    if constexpr (n == 0) [&]<natt... Is>(array<ID3D11RenderTargetView*, m + 1> RTVs) {
      main::system::d3d_context->OMSetRenderTargets(m + 1, RTVs.data(), d3d_dsv);
    }({d3d_rtv, As...});
    else [&](auto&& RTVs, auto&& UAVs) {
      main::system::d3d_context->OMSetRenderTargetsAndUnorderedAccessViews(m + 1, RTVs.data(), d3d_dsv, m + 1, n, UAVs.data(), nullptr);
    }([&]<natt... Is>(sequence<Is...>) { return array<ID3D11RenderTargetView*, m + 1>{d3d_rtv, parameter_switch<Is>(As...)...}; }(
        comb_indices<sequence<convertible_to<RTV_OR_UAV, ID3D11RenderTargetView*>...>>{}),
      [&]<natt... Is>(sequence<Is...>) { return array<ID3D11UnorderedAccessView*, n>{parameter_switch<Is>(As...)...}; }(
        comb_indices<sequence<convertible_to<RTV_OR_UAV, ID3D11UnorderedAccessView*>...>>{}));
  }
  /// Begins rendering after filling with the specified color.
  template<typename... RTV_OR_UAV> void begin_render(const color& Fill, RTV_OR_UAV&&... As) const
    requires((convertible_to<RTV_OR_UAV, ID3D11RenderTargetView*> ^ convertible_to<RTV_OR_UAV, ID3D11UnorderedAccessView*>) && ...) {
    main::system::d3d_context->ClearRenderTargetView(d3d_rtv, &(Fill.r));
    begin_render(fwd<RTV_OR_UAV>(As)...);
  }
  /// Ends rendering.
  void end_render() const {
    if (msaa) [&](comptr<ID3D11Resource> a) { d3d_rtv->GetResource(&a), main::system::d3d_context->ResolveSubresource(*this, 0, a, 0, DXGI_FORMAT_R8G8B8A8_UNORM); }({});
    main::system::d3d_context->OMSetRenderTargets(0, nullptr, nullptr);
  }
};

/// CAMERA
class camera : public canvas {
public:
  /// View matrix
  xmatrix view_matrix{xv_x, xv_y, xv_z, xv_w};
  /// View projection matrix
  xmatrix view_projection_matrix{xv_x, xv_y, xv_z, xv_w};
  /// Offsets of camera lens from the center of camera body
  vector offset{};
  /// Radians to rotate camera body
  vector rotation{};
  /// Position of camera body
  vector position{};
  /// Magnification
  /// @note モードによらず大きいほど拡大して描画される
  fat4 magnification = 1.0f;
  /// Enables orthographic mode.
  bool orthographic = false;
  /// Checks if this has been initialized.
  explicit operator bool() const noexcept { return canvas::operator bool(); }
  /// Default constructor
  camera() noexcept = default;
  /// Creates a camera from a canvas.
  camera(yw::canvas&& Canvas) : yw::canvas(mv(Canvas)) {}
  /// Creates a camera with a empty canvas.
  camera(natt Width, natt Height, natt Msaa = {}) : yw::canvas(Width, Height, Msaa) {}
  /// Updates camera matrices.
  void update() {
    constexpr fat4 f = 1048576.0f, n = 0.25f;
    xvview(position, rotation, offset, view_matrix);
    if (orthographic) {
      auto a = xv(-2.0f * magnification / width, 2.0f * magnification / height, 1.0f / (f - n), n / (f - n));
      view_projection_matrix[0] = xvmul(view_matrix[0], xvpermute<0, 0, 0, 0>(a));
      view_projection_matrix[1] = xvmul(view_matrix[1], xvpermute<1, 1, 1, 1>(a));
      view_projection_matrix[2] = xvsub(xvmul(view_matrix[2], xvpermute<2, 2, 2, 2>(a)), xvsetzero<1, 1, 1, 0>(a));
      view_projection_matrix[3] = view_matrix[3];
    } else {
      auto a = xv((-magnification * height) / width, magnification, f / (f - n), -f * n / (f - n));
      view_projection_matrix[0] = xvmul(view_matrix[0], xvpermute<0, 0, 0, 0>(a));
      view_projection_matrix[1] = xvmul(view_matrix[1], xvpermute<1, 1, 1, 1>(a));
      view_projection_matrix[2] = xvadd(xvmul(view_matrix[2], xvpermute<2, 2, 2, 2>(a)), xvsetzero<1, 1, 1, 0>(a));
      view_projection_matrix[3] = view_matrix[2];
    }
  }
};
}


namespace yw::key {

/// キーの状態を保持するクラス
class key {
public:
  /// キーが押し込まれたアプリ時間
  const fat8 pushed_time{};
  /// キーが押されたときにコールバックされる関数
  void (*down)() = nullptr;
  /// キーが解放されたときにコールバックされる関数
  void (*up)(fat8 hold_time) = nullptr;
  /// キーが現在押されているかどうかを確認する
  explicit operator bool() const noexcept { return pushed_time; }
};
inline key keys[256]{};
inline key &lbutton = keys[VK_LBUTTON], &rbutton = keys[VK_RBUTTON], &mbutton = keys[VK_MBUTTON];
inline key &xbutton1 = keys[VK_XBUTTON1], &xbutton2 = keys[VK_XBUTTON2], &win = keys[VK_LWIN], &app = keys[VK_APPS];
inline key &back = keys[VK_BACK], &tab = keys[VK_TAB], &enter = keys[VK_RETURN], &escape = keys[VK_ESCAPE];
inline key &shift = keys[VK_SHIFT], &control = keys[VK_CONTROL], &alt = keys[VK_MENU], &space = keys[VK_SPACE];
inline key &pageup = keys[VK_PRIOR], &pagedown = keys[VK_NEXT], &end = keys[VK_END], &home = keys[VK_HOME];
inline key &left = keys[VK_LEFT], &up = keys[VK_UP], &right = keys[VK_RIGHT], &down = keys[VK_DOWN];
inline key &screenshot = keys[VK_SNAPSHOT], &insert = keys[VK_INSERT], &delete_ = keys[VK_DELETE];
inline key &n0 = keys['0'], &n1 = keys['1'], &n2 = keys['2'], &n3 = keys['3'], &n4 = keys['4'];
inline key &n5 = keys['5'], &n6 = keys['6'], &n7 = keys['7'], &n8 = keys['8'], &n9 = keys['9'];
inline key &a = keys['A'], &b = keys['B'], &c = keys['C'], &d = keys['D'], &e = keys['E'], &f = keys['F'], &g = keys['G'], &h = keys['H'], &i = keys['I'];
inline key &j = keys['J'], &k = keys['K'], &l = keys['L'], &m = keys['M'], &n = keys['N'], &o = keys['O'], &p = keys['P'], &q = keys['Q'], &r = keys['R'];
inline key &s = keys['S'], &t = keys['T'], &u = keys['U'], &v = keys['V'], &w = keys['W'], &x = keys['X'], &y = keys['Y'], &z = keys['Z'];
inline key &np_0 = keys[VK_NUMPAD0], &np_1 = keys[VK_NUMPAD1], &np_2 = keys[VK_NUMPAD2], &np_3 = keys[VK_NUMPAD3], &np_4 = keys[VK_NUMPAD4];
inline key &np_5 = keys[VK_NUMPAD5], &np_6 = keys[VK_NUMPAD6], &np_7 = keys[VK_NUMPAD7], &np_8 = keys[VK_NUMPAD8], &np_9 = keys[VK_NUMPAD9];
inline key &np_mul = keys[VK_MULTIPLY], &np_add = keys[VK_ADD], &np_sub = keys[VK_SUBTRACT], &np_div = keys[VK_DIVIDE], &np_decimal = keys[VK_DECIMAL];
inline key &f1 = keys[VK_F1], &f2 = keys[VK_F2], &f3 = keys[VK_F3], &f4 = keys[VK_F4], &f5 = keys[VK_F5], &f6 = keys[VK_F6];
inline key &f7 = keys[VK_F7], &f8 = keys[VK_F8], &f9 = keys[VK_F9], &f10 = keys[VK_F10], &f11 = keys[VK_F11], &f12 = keys[VK_F12];
inline key &minus = keys[VK_OEM_MINUS], &plus = keys[VK_OEM_PLUS], &comma = keys[VK_OEM_COMMA], &period = keys[VK_OEM_PERIOD];
inline key &oem00 = keys[VK_OEM_7], &oem01 = keys[VK_OEM_5], &oem10 = keys[VK_OEM_3], &oem11 = keys[VK_OEM_4];
inline key &oem20 = keys[VK_OEM_1], &oem21 = keys[VK_OEM_6], &oem30 = keys[VK_OEM_2], &oem31 = keys[VK_OEM_102];
}


namespace yw {

/// マウス入力
class mouse {
public:
  /// マウスカーソルの位置を設定する
  mouse(int4 X, int4 Y) {
    [&](RECT r) { GetClientRect(main::system::hwnd, &r), SetCursorPos(r.left + X, r.top + Y); }({});
    main::system::mouse.first.left = X, main::system::mouse.first.top = Y;
  }

  /// マウスカーソルの横方向の位置を取得する
  inline static const auto& x = main::system::mouse.first.left;

  /// マウスカーソルの縦方向の位置を取得する
  inline static const auto& y = main::system::mouse.first.top;

  /// マウスカーソルがウィンドウ内にあるかどうかを取得する
  inline static const bool& absent = main::system::mouse.second;

  /// マウスホイールが回転したときにコールバックされる関数
  inline static void (*wheeled)(fat4 delta) = nullptr;

  /// マウスカーソルが移動したときにコールバックされる関数
  inline static void (*moved)(int4 dx, int4 dy) = nullptr;

  /// マウスの左クリックに相当するキークラス
  inline static auto& left = key::lbutton;

  /// マウスの右クリックに相当するキークラス
  inline static auto& right = key::rbutton;

  /// マウスの中央クリックに相当するキークラス
  inline static auto& middle = key::mbutton;
};

namespace main::system {
inline yw::bitmap logo;
}

/// メイン機能
namespace main {

inline const str2 username{};

/// 実行ファイル呼出し時の引数リスト
/// @note `args[0]`は実行ファイルのパス
inline const array<str2>& args = system::args;

/// System timer
inline const stopwatch& timer = system::timer;

/// Seconds Per Frame
inline const fat8& spf = system::spf;

/// Frames Per Second
inline const fat8& fps = system::fps;

/// Width of Main Window
inline const nat4& width = system::width;

/// Height of Main Window
inline const nat4& height = system::height;

/// Logo bitmap for ywlib
inline const bitmap& logo = system::logo;

/// メインウィンドウにファイルがドラッグアンドドロップされたときにコールバックされる関数
inline void (*dropped)(array<path> Files) = nullptr;

/// デフォルトのプロシージャに先行してコールバックされる関数
/// @note `true`を返す場合はデフォルトのプロシージャを呼び出さない
inline bool (*userproc)(HWND, nat4, natt, intt) = nullptr;

/// メインウィンドウのサイズを変更する
void resize(natt Width, natt Height) {
  static int4 width_pad{}, height_pad{};
  static constexpr cat2 class_name[] = L"ywlib";
  system::width = nat4(Width), system::height = nat4(Height);
  if (username.empty()) {
    DWORD size{};
    ::GetUserNameW(nullptr, &size), system::username.resize(size - 1);
    ::GetUserNameW(system::username.data(), &size);
  }
  if (!system::hinstance) {
    system::hinstance = GetModuleHandleW(0);
    WNDCLASSEXW wc{sizeof(WNDCLASSEXW), CS_OWNDC, system::wndproc, 0, 0, system::hinstance};
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW), wc.lpszClassName = class_name;
    tiff(RegisterClassExW(&wc));
  }
  if (!system::hwnd) {
    tiff(system::hwnd = CreateWindowExW(WS_EX_ACCEPTFILES, class_name, class_name, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_CLIPCHILDREN,
                                        0, 0, int4(width), int4(height), 0, 0, system::hinstance, 0));
    tiff(system::hfont = CreateFontW(16, 0, 0, 0, FW_NORMAL, 0, 0, 0, SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS,
                                     CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, VARIABLE_PITCH | FF_ROMAN, L"Yu Gothic UI"));
    [&](int c) { auto args = CommandLineToArgvW(GetCommandLineW(), &c); for (system::args.resize(c); 0 <= --c;) system::args[c] = args[c]; }(0);
    [&](RECT r = {}) { GetClientRect(system::hwnd, &r), width_pad = width - r.right, height_pad = height - r.bottom; }();
    tiff(SetWindowPos(system::hwnd, 0, 0, 0, width + width_pad, height + height_pad, SWP_NOZORDER));
    comptr<IDXGIFactory2> factory{};
    tiff(CreateDXGIFactory1(IID_PPV_ARGS(&factory)));
    tiff(factory->MakeWindowAssociation(system::hwnd, DXGI_MWA_NO_ALT_ENTER));
    D3D_FEATURE_LEVEL featurelevels[] = {D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0};
    tiff(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT,
                           featurelevels, (UINT)extent<decltype(featurelevels)>, D3D11_SDK_VERSION,
                           (ID3D11Device**)&system::d3d_device, nullptr, (ID3D11DeviceContext**)&system::d3d_context));
    DXGI_SWAP_CHAIN_DESC1 sc_desc{width, height, DXGI_FORMAT_R8G8B8A8_UNORM, false, {1, 0}, DXGI_USAGE_RENDER_TARGET_OUTPUT, 2};
    sc_desc.Scaling = DXGI_SCALING_STRETCH, sc_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    tiff(factory->CreateSwapChainForHwnd(system::d3d_device, system::hwnd, &sc_desc, nullptr, nullptr, &system::swap_chain));
    tiff(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &system::d2d_factory));
    [&](comptr<IDXGIDevice2> dxgi_device) {
      tiff(system::d3d_device.as(dxgi_device));
      tiff(system::d2d_factory->CreateDevice(dxgi_device, &system::d2d_device)); }({});
    tiff(system::d2d_device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &system::d2d_context));
    tiff(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)&system::dw_factory));
    tiff(CoCreateInstance(CLSID_WICImagingFactory2, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&system::wic_factory)));
    [&](comptr<ID3D11BlendState> state, D3D11_BLEND_DESC desc) {
      desc.RenderTarget[0] = {true, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_OP_ADD,
                                D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_OP_ADD, 0x0f};
      tiff(system::d3d_device->CreateBlendState(&desc, &state));
      system::d3d_context->OMSetBlendState(state, nullptr, 0xffffffff); }({}, {});
    [&](comptr<ID3D11SamplerState> state, D3D11_SAMPLER_DESC desc) {
      desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP, desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
      desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP, desc.MaxAnisotropy = 1;
      desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS, desc.MaxLOD = D3D11_FLOAT32_MAX;
      tiff(system::d3d_device->CreateSamplerState(&desc, &state));
      system::d3d_context->PSSetSamplers(0, 1, &asconst(state)); }({}, {D3D11_FILTER_MIN_MAG_MIP_LINEAR});
    [&](comptr<ID3D11RasterizerState> state, D3D11_RASTERIZER_DESC desc) {
      desc.DepthClipEnable = true, desc.MultisampleEnable = true, desc.AntialiasedLineEnable = true;
      tiff(system::d3d_device->CreateRasterizerState(&desc, &state));
      system::d3d_context->RSSetState(state); }({}, {D3D11_FILL_SOLID, D3D11_CULL_NONE, true});
  }
  tiff(SetWindowPos(system::hwnd, 0, 0, 0, width + width_pad, height + height_pad, SWP_NOMOVE | SWP_NOZORDER));
  system::bitmap.reset();
  system::d2d_context->SetTarget(0);
  tiff(system::swap_chain->ResizeBuffers(2, width, height, DXGI_FORMAT_UNKNOWN, 0));
  [&](comptr<ID3D11Texture2D> tex, comptr<IDXGISurface> surface) {
    tiff(system::swap_chain->GetBuffer(0, IID_PPV_ARGS(&tex)));
    tiff(tex->QueryInterface(IID_PPV_ARGS(&surface)));
    auto props = D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
                                         D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED));
    tiff(system::d2d_context->CreateBitmapFromDxgiSurface(surface, props, &system::bitmap)); }({}, {});
}

/// メインウィンドウのタイトルを変更する
inline void rename(const str2& Name) { SetWindowTextW(main::system::hwnd, Name.data()); }

/// メインウィンドウを閉じる
inline void terminate() noexcept { main::system::hwnd ? DestroyWindow(exchange(main::system::hwnd, nullptr)) : none{}; }

/// メインウィンドウの常時最前面表示を切り替える
inline void topmost(bool Flag = true) {
  if (auto style = GetWindowLongW(main::system::hwnd, GWL_EXSTYLE); Flag) {
    SetWindowPos(main::system::hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    SetWindowLongW(main::system::hwnd, GWL_EXSTYLE, style | WS_EX_TOPMOST);
  } else {
    SetWindowPos(main::system::hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    if (style & WS_EX_TOPMOST) SetWindowLongW(main::system::hwnd, GWL_EXSTYLE, style & ~WS_EX_TOPMOST);
  }
}

/// メインウィンドウのスクリーンショットを撮影する
inline void screenshot(const path& Path) {
  try {
    if (file::exists(Path)) file::remove(Path);
    auto& factory = main::system::wic_factory;
    comptr<IWICStream> stream{};
    tiff(factory->CreateStream(&stream));
    tiff(stream->InitializeFromFilename(Path.c_str(), GENERIC_WRITE));
    comptr<IWICBitmapEncoder> encoder{};
    tiff(factory->CreateEncoder(GUID_ContainerFormatPng, nullptr, &encoder));
    tiff(encoder->Initialize(stream, WICBitmapEncoderNoCache));
    comptr<IWICBitmapFrameEncode> frame{};
    tiff(encoder->CreateNewFrame(&frame, nullptr));
    tiff(frame->Initialize(nullptr));
    comptr<IWICImageEncoder> image_encoder{};
    tiff(factory->CreateImageEncoder(main::system::d2d_device, &image_encoder));
    tiff(image_encoder->WriteFrame(system::bitmap, frame, nullptr));
    frame->Commit(), encoder->Commit(), stream->Commit(STGC_DEFAULT);
  } catch (const std::exception& E) { throw except(E); }
}

/// メインウィンドウの描画を更新する
inline constexpr caster update{
  []() {
    static MSG msg{};
    static RECT rc{};
    static POINT pt{};
    static stopwatch sw{};
    if (mouse::absent) {
      GetClientRect(system::hwnd, &rc), GetCursorPos(&pt);
      _mm_storeu_si128((xrect*)&system::mouse, [](const xrect& a) {
        return xvpermute<0, 1, 4, 5>(a, xvsub(a, _mm_loadu_si128((xrect*)&system::mouse)));
      }(xvsub(_mm_loadu_si128((xrect*)&pt), _mm_loadu_si128((xrect*)&rc))));
    }
    system::spf = sw.push(), system::fps = 1.0f / system::spf;
    system::swap_chain->Present(1, 0);
    while (system::hwnd) {
      if (PeekMessageW(&msg, 0, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) return false;
        TranslateMessage(&msg), DispatchMessageW(&msg);
      } else return true;
    }
    return false;
  }};

/// Begins drawing to main-window.
void begin_draw() {
  system::d2d_context->SetTarget(system::bitmap), system::d2d_context->BeginDraw();
}

/// Begins drawing to main-window after filling with the specified color.
void begin_draw(const color& Fill) {
  begin_draw(), system::d2d_context->Clear(bitcast<D2D1_COLOR_F>(Fill));
}

/// Ends drawing to main-window.
void end_draw() { tiff(system::d2d_context->EndDraw()); }
}

/// メインウィンドウのプロシージャ
inline LRESULT WINAPI main::system::wndproc(HWND hw, UINT msg, WPARAM wp, LPARAM lp) {
  static constexpr auto key_down = [](key::key& k) { if (const_cast<fat8&>(k.pushed_time) = main::system::timer.read(), k.down) k.down(); };
  static constexpr auto key_up = [](key::key& k) { fat8& t = const_cast<fat8&>(k.pushed_time); t == 0 ? void() : (k.up ? k.up(main::system::timer.read() - exchange(t, 0.0)) : void(t = 0.0)); };
  static TRACKMOUSEEVENT tme{.cbSize = sizeof(TRACKMOUSEEVENT), .dwFlags = TME_LEAVE};
  if (userproc && userproc(hw, msg, wp, lp)) return 0;
  switch (msg) {
  case WM_MOUSEMOVE: {
    if (mouse.second) TrackMouseEvent(&tme), mouse.second = false;
    _mm_storeu_epi32(&mouse, [](auto a) {
      return xvpermute<0, 1, 4, 5>(a, xvsub(a, _mm_loadu_si128((xrect*)&mouse)));
    }(_mm_cvtepi16_epi32(_mm_loadu_si128((xrect*)&lp))));
    return (yw::mouse::moved ? yw::mouse::moved(mouse.first.right, mouse.first.bottom) : void(0)), 0;
  }
  case WM_MOUSEWHEEL: return (mouse::wheeled ? mouse::wheeled(int2((wp & 0xffff0000) >> 16)) : void(0)), 0;
  case WM_MOUSELEAVE:
    for (auto& k : key::keys) const_cast<fat8&>(k.pushed_time) = 0.0;
    return (mouse.second = true), 0;
  case WM_LBUTTONDOWN: return SetFocus(hw), key_down(key::keys[VK_LBUTTON]), 0;
  case WM_RBUTTONDOWN: return SetFocus(hw), key_down(key::keys[VK_RBUTTON]), 0;
  case WM_MBUTTONDOWN: return SetFocus(hw), key_down(key::keys[VK_MBUTTON]), 0;
  case WM_KEYDOWN: return SetFocus(hw), key_down(key::keys[wp]), 0;
  case WM_LBUTTONUP: return key_up(key::keys[VK_LBUTTON]), 0;
  case WM_RBUTTONUP: return key_up(key::keys[VK_RBUTTON]), 0;
  case WM_MBUTTONUP: return key_up(key::keys[VK_MBUTTON]), 0;
  case WM_KEYUP: return key_up(key::keys[wp]), 0;
  case WM_DROPFILES: {
    if (!main::dropped) return 0;
    auto hdrop = (HDROP)wp;
    const natt count = DragQueryFileW(hdrop, 0xFFFFFFFF, nullptr, 0);
    array<path> out(count);
    for (nat4 i{}; i < count; ++i) {
      str2 buffer(DragQueryFileW(hdrop, i, nullptr, 0), L'\0');
      DragQueryFileW(hdrop, i, buffer.data(), (nat4)buffer.size() + 1), out[i] = path(buffer);
    }
    return main::dropped(mv(out)), 0;
  }
  case WM_CLOSE: return DestroyWindow(hw), 0;
  case WM_DESTROY: return PostQuitMessage(0), 0;
  case WM_CREATE: tme.hwndTrack = hw, TrackMouseEvent(&tme), mouse.second = false;
  }
  return DefWindowProcW(hw, msg, wp, lp);
}

struct rect {
  int4 left{}, top{}, right{}, bottom{};
  constexpr int4 width() const noexcept { return right - left; }
  constexpr int4 height() const noexcept { return bottom - top; }
  constexpr rect() noexcept = default;
  explicit constexpr rect(const int4 Fill) noexcept : left(Fill), top(Fill), right(Fill), bottom(Fill) {}
  template<castable_to<int4> T> explicit constexpr rect(T&& Fill) noexcept(nt_castable_to<T, int4>)
    requires(!same_as<remove_cvref<T>, int4>) : rect(static_cast<int4>(fwd<T>(Fill))) {}
  template<castable_to<int4> T0, castable_to<int4> T1, castable_to<int4> T2, castable_to<int4> T3>
  constexpr rect(T0&& L, T1&& T, T2&& R, T3&& B) noexcept(
    nt_castable_to<T0, int4> && nt_castable_to<T1, int4> && nt_castable_to<T2, int4> && nt_castable_to<T3, int4>)
    : left(static_cast<int4>(fwd<T0>(L))), top(static_cast<int4>(fwd<T1>(T))),
      right(static_cast<int4>(fwd<T2>(R))), bottom(static_cast<int4>(fwd<T3>(B))) {}
  rect(const xrect& A) noexcept { _mm_storeu_epi32(&left, A); }
  rect& operator=(const xrect& A) noexcept { return _mm_storeu_epi32(&left, A), *this; }
  operator xrect() const noexcept { return xv(&left); }
  constexpr natt size() const noexcept { return 4; }
  int4* data() noexcept { return &left; }
  const int4* data() const noexcept { return &left; }
  int4* begin() noexcept { return &left; }
  const int4* begin() const noexcept { return &left; }
  int4* end() noexcept { return &left + 4; }
  const int4* end() const noexcept { return &left + 4; }
  template<natt Ix> requires(Ix < 4) constexpr int4& get() noexcept {
    if constexpr (Ix == 0) return left;
    else if constexpr (Ix == 1) return top;
    else if constexpr (Ix == 2) return right;
    else return bottom;
  }
  template<natt Ix> requires(Ix < 4) constexpr int4 get() const noexcept {
    if constexpr (Ix == 0) return left;
    else if constexpr (Ix == 1) return top;
    else if constexpr (Ix == 2) return right;
    else return bottom;
  }
  friend constexpr bool operator==(const rect& A, const rect& B) noexcept {
    if (!is_cev) return xveq(A, B);
    return A.left == B.left && A.top == B.top && A.right == B.right && A.bottom == B.bottom;
  }
  friend constexpr auto operator<=>(const rect& A, const rect& B) noexcept {
    if (!is_cev) return xvtw(A, B);
    if (auto a = A.left <=> B.left; a != 0) return a;
    else if (a = A.top <=> B.top; a != 0) return a;
    else if (a = A.right <=> B.right; a != 0) return a;
    else return A.bottom <=> B.bottom;
  }
  friend constexpr rect operator+(const rect& A) noexcept { return A; }
  friend constexpr rect operator-(const rect& A) noexcept { return {-A.left, -A.top, -A.right, -A.bottom}; }
  friend constexpr rect operator+(const rect& A, const rect& B) noexcept { return {A.left + B.left, A.top + B.top, A.right + B.right, A.bottom + B.bottom}; }
  friend constexpr rect operator-(const rect& A, const rect& B) noexcept { return {A.left - B.left, A.top - B.top, A.right - B.right, A.bottom - B.bottom}; }
  friend constexpr rect operator*(const rect& A, const rect& B) noexcept { return {A.left * B.left, A.top * B.top, A.right * B.right, A.bottom * B.bottom}; }
  friend constexpr rect operator/(const rect& A, const rect& B) noexcept { return {A.left / B.left, A.top / B.top, A.right / B.right, A.bottom / B.bottom}; }
  friend constexpr rect operator+(const rect& A, const int4& B) noexcept { return {A.left + B, A.top + B, A.right + B, A.bottom + B}; }
  friend constexpr rect operator-(const rect& A, const int4& B) noexcept { return {A.left - B, A.top - B, A.right - B, A.bottom - B}; }
  friend constexpr rect operator*(const rect& A, const int4& B) noexcept { return {A.left * B, A.top * B, A.right * B, A.bottom * B}; }
  friend constexpr rect operator/(const rect& A, const int4& B) noexcept { return {A.left / B, A.top / B, A.right / B, A.bottom / B}; }
  friend constexpr rect operator+(const int4& A, const rect& B) noexcept { return {A + B.left, A + B.top, A + B.right, A + B.bottom}; }
  friend constexpr rect operator-(const int4& A, const rect& B) noexcept { return {A - B.left, A - B.top, A - B.right, A - B.bottom}; }
  friend constexpr rect operator*(const int4& A, const rect& B) noexcept { return {A * B.left, A * B.top, A * B.right, A * B.bottom}; }
  friend constexpr rect operator/(const int4& A, const rect& B) noexcept { return {A / B.left, A / B.top, A / B.right, A / B.bottom}; }
  friend constexpr rect& operator+=(rect& A, const rect& B) noexcept { return A = A + B; }
  friend constexpr rect& operator-=(rect& A, const rect& B) noexcept { return A = A - B; }
  friend constexpr rect& operator*=(rect& A, const rect& B) noexcept { return A = A * B; }
  friend constexpr rect& operator/=(rect& A, const rect& B) noexcept { return A = A / B; }
  friend constexpr rect& operator+=(rect& A, const int4& B) noexcept { return A = A + B; }
  friend constexpr rect& operator-=(rect& A, const int4& B) noexcept { return A = A - B; }
  friend constexpr rect& operator*=(rect& A, const int4& B) noexcept { return A = A * B; }
  friend constexpr rect& operator/=(rect& A, const int4& B) noexcept { return A = A / B; }

  operator D2D1_RECT_U() const noexcept {
    return bitcast<D2D1_RECT_U>(*this);
  }
  operator D2D1_RECT_F() const noexcept {
    return [&](D2D1_RECT_F A) noexcept { return _mm_storeu_ps(&A.left, xvcast<xvector>(xv(&left))), mv(A); }({});
  }
  explicit operator bool() const noexcept {
    return left <= main::system::mouse.first.left &&
           top <= main::system::mouse.first.top &&
           main::system::mouse.first.left < right &&
           main::system::mouse.first.top < bottom;
  }
  void draw_rectangle(const convertible_to<ID2D1SolidColorBrush*> auto& Brush) const { main::system::d2d_context->FillRectangle(*this, Brush); }
  void draw_rectangle(const convertible_to<ID2D1SolidColorBrush*> auto& Brush, const fat4 Thickness) const { main::system::d2d_context->DrawRectangle(*this, Brush, Thickness); }
  void draw_rounded_rectangle(const convertible_to<ID2D1SolidColorBrush*> auto& Brush, fat4 Rx, fat4 Ry) const { main::system::d2d_context->FillRoundedRectangle(D2D1_ROUNDED_RECT{*this, Rx, Ry}, Brush); }
  void draw_rounded_rectangle(const convertible_to<ID2D1SolidColorBrush*> auto& Brush, fat4 Thickness, fat4 Rx, fat4 Ry) const { main::system::d2d_context->DrawRoundedRectangle(D2D1_ROUNDED_RECT{*this, Rx, Ry}, Brush, Thickness); }
  void draw_bitmap(const convertible_to<ID2D1Bitmap*> auto& Bitmap, fat4 Opacity = 1.0f) const { main::system::d2d_context->DrawBitmap(Bitmap, *this, Opacity); }
  void draw_text(stv2 Text, const convertible_to<IDWriteTextFormat*> auto& Font, const convertible_to<ID2D1SolidColorBrush*> auto& Brush) const { main::system::d2d_context->DrawTextW(Text.data(), nat4(Text.size()), Font, *this, Brush, D2D1_DRAW_TEXT_OPTIONS_CLIP); }
  void draw_line(const convertible_to<ID2D1SolidColorBrush*> auto& Brush, fat4 Thickness = 1.0f) const {
    main::system::d2d_context->DrawLine({(fat4)left, (fat4)top}, {(fat4)right, (fat4)bottom}, Brush, Thickness);
  }
};
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  UI

#include <map>

namespace yw::ui {

/// OKボタンを持つダイアログ
/// @note 常に`true`を返す
inline bool ok(const str1& Text, const str1& Caption = "OK?") { return MessageBoxA(main::system::hwnd, Text.data(), Caption.data(), MB_OK | MB_ICONEXCLAMATION), true; }

/// OKボタンを持つダイアログ
/// @note 常に`true`を返す
inline bool ok(const str2& Text, const str2& Caption = L"OK?") { return MessageBoxW(main::system::hwnd, Text.data(), Caption.data(), MB_OK | MB_ICONEXCLAMATION), true; }

/// YES/NOボタンを持つダイアログ
/// @note YESボタンが押された場合は`true`を返す
inline bool yes(const str1& Text, const str1& Caption = "YES?") { return MessageBoxA(main::system::hwnd, Text.data(), Caption.data(), MB_YESNO | MB_ICONQUESTION) == IDYES; }

/// YES/NOボタンを持つダイアログ
/// @note YESボタンが押された場合は`true`を返す
inline bool yes(const str2& Text, const str2& Caption = L"YES?") { return MessageBoxW(main::system::hwnd, Text.data(), Caption.data(), MB_YESNO | MB_ICONQUESTION) == IDYES; }

/// ファイルを開くダイアログ
inline path open_file(const path& InitialDir = {}) {
  cat2 buf[260]{0};
  OPENFILENAMEW ofn{DWORD(sizeof(ofn)), main::system::hwnd, 0, L"All Files (*.*)\0*.*\0", 0, 0, 1, buf, nat4(extent<decltype(buf)>)};
  ofn.lpstrInitialDir = InitialDir.empty() ? nullptr : InitialDir.c_str(), ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
  return GetOpenFileNameW(&ofn) == TRUE ? path(ofn.lpstrFile) : path{};
}

/// ファイルを保存するダイアログ
inline path save_file(const path& InitialDir = {}, const path& InitialFileName = {}) {
  cat2 buf[260]{0};
  if (!InitialFileName.empty()) std::ranges::copy(InitialFileName.filename().wstring(), buf);
  OPENFILENAMEW ofn{DWORD(sizeof(ofn)), main::system::hwnd, 0, L"All Files (*.*)\0*.*\0", 0, 0, 1, buf, nat4(extent<decltype(buf)>)};
  ofn.lpstrInitialDir = InitialDir.empty() ? nullptr : InitialDir.c_str(), ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
  return GetSaveFileNameW(&ofn) == TRUE ? path(ofn.lpstrFile) : path{};
}

/**
 * @brief virtual class for creating a control
 **********************************************************************************************************************/
class control {
protected:
  inline static constexpr natt max_groups = 16;
  inline static array<array<list<HWND, control*, bool>>, max_groups> groups;
  inline static array<list<HWND, control*, bool>> others;
  inline static std::map<HWND, list<natt, control*>> whole_group;
  control(const cat2* Class, const str2& Text, nat4 Style, const rect& Rect, bool Inputable)
    : hwnd(CreateWindowExW(0, Class, Text.data(), WS_VISIBLE | WS_CHILD | WS_GROUP | Style, Rect.left, Rect.top,
                           Rect.width(), Rect.height(), main::system::hwnd, 0, main::system::hinstance, this)) {
    SendMessageW(hwnd, WM_SETFONT, (WPARAM)main::system::hfont, true), SetWindowLongPtrW(hwnd, GWLP_ID, (LONG_PTR)hwnd);
    (others.empty() ? others.reserve(1024) : void()), others.push_back(list{hwnd, this, Inputable}), whole_group[hwnd] = list{npos, this};
  }
  control(const cat2* Class, const str2& Text, nat4 Style, const rect& Rect, const natt GroupNo, bool Inputable)
    : hwnd(CreateWindowExW(0, Class, Text.data(), WS_VISIBLE | WS_CHILD | WS_GROUP | Style, Rect.left, Rect.top,
                           Rect.width(), Rect.height(), main::system::hwnd, 0, main::system::hinstance, this)) {
    SendMessageW(hwnd, WM_SETFONT, (WPARAM)main::system::hfont, true), SetWindowLongPtrW(hwnd, GWLP_ID, (LONG_PTR)hwnd);
    if (GroupNo < max_groups) {
      if (groups[GroupNo].empty()) groups[GroupNo].reserve(1024);
      groups[GroupNo].push_back(list{hwnd, this, Inputable}), whole_group[hwnd] = list{GroupNo, this};
    } else {
      ywee("Control group index is out of range.");
      (others.empty() ? others.reserve(1024) : void()), others.push_back(list{hwnd, this, Inputable}), whole_group[hwnd] = list{npos, this};
    }
  }
  static array<list<HWND, control*, bool>>& get_group(const natt GroupNo) { return GroupNo == npos ? others : groups[GroupNo]; }
  static natt get_index_in_group(const array<list<HWND, control*, bool>>& g, HWND hw) {
    for (natt i{}; i < g.size(); ++i)
      if (g[i].first == hw) return i;
    return npos;
  }
  static void focus_on_next(const array<list<HWND, control*, bool>>& g, natt i, bool Prev) {
    std::cout << std::format("{} / {} - ", i, g.size());
    if (Prev) {
      for (natt j = i - 1; j < i; --j)
        if (g[j].third) return g[j].second->setfocus(), std::cout << j << std::endl, void();
      for (natt j = g.size() - 1; i < j; --j)
        if (g[j].third) return g[j].second->setfocus(), std::cout << j << std::endl, void();
    } else {
      for (natt j = i + 1; j < g.size(); ++j)
        if (g[j].third) return g[j].second->setfocus(), std::cout << j << std::endl, void();
      for (natt j = 0; j < i; ++j)
        if (g[j].third) return g[j].second->setfocus(), std::cout << j << std::endl, void();
    }
  }
public:
  static void hide(natt GroupNo, bool Hide = true) {
    if (GroupNo >= max_groups) return ywee("Control group index is out of range."), void();
    std::cout << groups[GroupNo].size() << std::endl;
    for (auto& g : groups[GroupNo]) g.second->hide(Hide), std::cout << std::format("{} - {}\n", natt(g.first), Hide);
  }
  const HWND hwnd{};
  const natt group{};
  virtual void setfocus() const { hwnd ? void(SetFocus(hwnd)) : void(ywee("Control is not created yet.")); }
  virtual void hide(bool Hide = true) const { ShowWindow(hwnd, Hide ? SW_HIDE : SW_SHOW); }
  virtual void text(const str2& Text) const { SetWindowTextW(hwnd, Text.data()); }
  virtual str2 text() const {
    str2 out(GetWindowTextLengthW(hwnd), {});
    return GetWindowTextW(hwnd, out.data(), int4(out.size() + 1)), mv(out);
  }
  ~control() noexcept {
    if (!hwnd) return;
    DestroyWindow(hwnd), whole_group.erase(hwnd);
    if (group == npos) others.erase(others.begin() + get_index_in_group(others, hwnd));
    else groups[group].erase(groups[group].begin() + get_index_in_group(groups[group], hwnd));
  }
  control() noexcept = default;
  control(control&& A) : hwnd(exchange(const_cast<HWND&>(A.hwnd), nullptr)), group(A.group) {
    whole_group[hwnd].second = this;
    if (group == npos) others[get_index_in_group(others, hwnd)].second = this;
    else groups[group][get_index_in_group(groups[group], hwnd)].second = this;
  }
  control& operator=(control&& A) {
    if (hwnd) {
      DestroyWindow(hwnd);
      if (group == npos) others.erase(others.begin() + get_index_in_group(others, hwnd));
      else groups[group].erase(groups[group].begin() + get_index_in_group(groups[group], hwnd));
    }
    const_cast<HWND&>(hwnd) = exchange(const_cast<HWND&>(A.hwnd), nullptr), const_cast<natt&>(group) = A.group;
    whole_group[hwnd].second = this;
    if (group == npos) others[get_index_in_group(others, hwnd)].second = this;
    else groups[group][get_index_in_group(groups[group], hwnd)].second = this;
    return *this;
  }
  explicit operator bool() const noexcept { return hwnd; }
};


/**
 * @brief class for creating a textbox control
 **********************************************************************************************************************/
class textbox : public control {
protected:
  inline static WNDPROC defproc = nullptr;
  static LRESULT CALLBACK proc(HWND hw, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_CHAR) {
      if (wp == VK_RETURN) {
        if (auto& t = *static_cast<textbox*>(whole_group[hw].second); t.enter) return t.enter(t), 0;
      } else if (wp == VK_TAB) {
        auto& g = get_group(whole_group[hw].first);
        auto i = get_index_in_group(g, hw);
        auto& t = *static_cast<textbox*>(g[i].second);
        if (t.tab) t.tab(t);
        return focus_on_next(g, i, GetKeyState(VK_SHIFT) < 0), 0;
      } else if (wp == VK_ESCAPE) return SetFocus(main::system::hwnd), 0;
    } else if (msg == WM_SETFOCUS) {
      if (auto& t = *static_cast<textbox*>(whole_group[hw].second); t.intofocus) return t.intofocus(t), 0;
    } else if (msg == WM_KILLFOCUS) {
      if (auto& t = *static_cast<textbox*>(whole_group[hw].second); t.killfocus) return t.killfocus(t), 0;
    }
    return CallWindowProcW(defproc, hw, msg, wp, lp);
  }
public:
  void (*enter)(const textbox& This) = nullptr;
  void (*tab)(const textbox& This) = nullptr;
  void (*intofocus)(const textbox& This) = nullptr;
  void (*killfocus)(const textbox& This) = nullptr;
  textbox() noexcept = default;
  textbox(textbox&&) = default;
  textbox& operator=(textbox&&) = default;
  textbox(const rect& Rect, const str2& Init = L"", nat4 EditStyle = {})
    : control(L"EDIT", Init, EditStyle, Rect, true) {
    if (!defproc) defproc = (WNDPROC)SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)proc);
    SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)proc);
  }
  textbox(const natt GroupNo, const rect& Rect, const str2& Init, nat4 EditStyle = {})
    : control(L"EDIT", Init, EditStyle, Rect, GroupNo, true) {
    if (!defproc) defproc = (WNDPROC)SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)proc);
    SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)proc);
  }
};

/**
 * @brief class for creating a valuebox control
 **********************************************************************************************************************/
class valuebox : public control {
protected:
  inline static WNDPROC defproc = nullptr;
  static LRESULT CALLBACK proc(HWND hw, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_CHAR) {
      if (wp == VK_RETURN) {
        if (auto& t = *static_cast<valuebox*>(whole_group[hw].second); t.enter) return t.enter(t), 0;
      } else if (wp == VK_TAB) {
        auto& g = get_group(whole_group[hw].first);
        auto i = get_index_in_group(g, hw);
        auto& t = *static_cast<valuebox*>(g[i].second);
        if (t.tab) t.tab(t);
        return focus_on_next(g, i, GetKeyState(VK_SHIFT) < 0), 0;
      } else if (wp == VK_ESCAPE) return SetFocus(main::system::hwnd), 0;
      else if (wp == VK_BACK || wp == VK_DELETE || wp == VK_LEFT || wp == VK_RIGHT) void(0);
      else if (!(wp >= '0' && wp <= '9' || wp == '-' || wp == '.')) return 0;
    } else if (msg == WM_SETFOCUS) {
      if (auto& t = *static_cast<valuebox*>(whole_group[hw].second); t.intofocus) return t.intofocus(t), 0;
    } else if (msg == WM_KILLFOCUS) {
      if (auto& t = *static_cast<valuebox*>(whole_group[hw].second); t.killfocus) return t.killfocus(t), 0;
    }
    return CallWindowProcW(defproc, hw, msg, wp, lp);
  }
public:
  void (*enter)(const valuebox& This) = nullptr;
  void (*tab)(const valuebox& This) = nullptr;
  void (*intofocus)(const valuebox& This) = nullptr;
  void (*killfocus)(const valuebox& This) = nullptr;
  valuebox() noexcept = default;
  valuebox(valuebox&&) = default;
  valuebox& operator=(valuebox&&) = default;
  valuebox(const rect& Rect, const arithmetic auto Init, nat4 EditStyle = {})
    : control(L"EDIT", vtos<cat2>(Init), EditStyle, Rect, true) {
    if (!defproc) defproc = (WNDPROC)SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)proc);
    SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)proc);
  }
  valuebox(const natt GroupNo, const rect& Rect, const arithmetic auto Init, nat4 EditStyle = {})
    : control(L"EDIT", vtos<cat2>(Init), EditStyle, Rect, GroupNo, true) {
    if (!defproc) defproc = (WNDPROC)SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)proc);
    SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)proc);
  }
  void value(const arithmetic auto Value) const { SetWindowTextW(hwnd, vtos<cat2>(Value).data()); }
  template<arithmetic T> T value() const { return stov<T>(text()); }
};

/**
 * @brief class for creating a label control
 **********************************************************************************************************************/
class label : public control {
public:
  label() noexcept = default;
  label(label&&) = default;
  label& operator=(label&&) = default;
  label(const rect& Rect, const str2& Text, nat4 StaticStyle = {})
    : control(L"STATIC", Text, StaticStyle, Rect, false) {}
  label(const natt GroupNo, const rect& Rect, const str2& Text, nat4 StaticStyle = {})
    : control(L"STATIC", Text, StaticStyle, Rect, GroupNo, false) {}
};

/**
 * @brief class for obtaining the button state
 **********************************************************************************************************************/
template<typename Button, invocable<const Button&> Converter> class button_state {
  const Button* ptr;
  Converter func;
public:
  template<typename F> button_state(const Button* B, F&& Func) noexcept : ptr(B), func(fwd<F>(Func)) {}
  operator invoke_result<Converter, const Button&>() const { return func(*ptr); }
};
template<typename B, typename F> button_state(const B*, F&&) -> button_state<B, F>;

/**
 * @brief class for creating a button control
 **********************************************************************************************************************/
class button : public control {
  static bool get_state(const button& This) { return bool(defproc(This.hwnd, BM_GETSTATE, 0, 0) & BST_PUSHED); }
protected:
  inline static WNDPROC defproc = nullptr;
  static LRESULT CALLBACK proc(HWND hw, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_LBUTTONDOWN) {
      if (auto& t = *static_cast<button*>(whole_group[hw].second); t.enter)
        return [&](auto r) { return t.enter(t), r; }(defproc(hw, msg, wp, lp));
    } else if (msg == WM_KEYDOWN) {
      if (wp == VK_RETURN || wp == VK_SPACE) {
        if (auto& t = *static_cast<button*>(whole_group[hw].second); t.enter)
          return [&](auto r) { return t.enter(t), r; }(defproc(hw, msg, wp, lp));
      } else if (wp == VK_TAB) {
        auto& g = get_group(whole_group[hw].first);
        auto i = get_index_in_group(g, hw);
        return focus_on_next(g, i, GetKeyState(VK_SHIFT) < 0), 0;
      } else if (wp == VK_ESCAPE) return SetFocus(main::system::hwnd), 0;
    }
    return CallWindowProcW(defproc, hw, msg, wp, lp);
  }
public:
  const button_state<button, decltype(&get_state)> state;
  void (*enter)(const button& This) = nullptr;
  button() noexcept : control(), state(this, get_state) {}
  button(button&& A) : control(mv(dynamic_cast<control&>(A))), state(this, get_state), enter(A.enter) {}
  button& operator=(button&& A) {
    return dynamic_cast<control&>(*this) = mv(dynamic_cast<control&>(A)), enter = A.enter, *this;
  }
  button(const rect& Rect, const str2& Text, nat4 ButtonStyle = {})
    : control(L"BUTTON", Text.data(), ButtonStyle, Rect, true), state(this, get_state) {
    if (!defproc) defproc = (WNDPROC)GetWindowLongPtrW(hwnd, GWLP_WNDPROC);
    SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)proc);
  }
  button(natt GroupNo, const rect& Rect, const str2& Text, nat4 ButtonStyle = {})
    : control(L"BUTTON", Text.data(), ButtonStyle, Rect, GroupNo, true), state(this, get_state) {
    if (!defproc) defproc = (WNDPROC)GetWindowLongPtrW(hwnd, GWLP_WNDPROC);
    SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)proc);
  }
};

/**
 * @brief class for creating a checkbox control
 **********************************************************************************************************************/
class checkbox : public control {
  static bool get_state(const checkbox& This) { return bool(defproc(This.hwnd, BM_GETCHECK, 0, 0) & BST_CHECKED); }
protected:
  inline static WNDPROC defproc = nullptr;
  static LRESULT CALLBACK proc(HWND hw, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_LBUTTONUP) {
      if (auto& t = *static_cast<checkbox*>(whole_group[hw].second); t.enter)
        return [&](auto r) { return t.enter(t), r; }(defproc(hw, msg, wp, lp));
    } else if (msg == WM_KEYUP) {
      if (wp == VK_RETURN || wp == VK_SPACE)
        if (auto& t = *static_cast<checkbox*>(whole_group[hw].second); t.enter)
          return [&](auto r) { return t.enter(t), r; }(defproc(hw, msg, VK_SPACE, lp));
    } else if (msg == WM_KEYDOWN) {
      if (wp == VK_RETURN) wp = VK_SPACE;
      else if (wp == VK_TAB) {
        auto& g = get_group(whole_group[hw].first);
        auto i = get_index_in_group(g, hw);
        return focus_on_next(g, i, GetKeyState(VK_SHIFT) < 0), 0;
      } else if (wp == VK_ESCAPE) return SetFocus(main::system::hwnd), 0;
    }
    return CallWindowProcW(defproc, hw, msg, wp, lp);
  }
public:
  const button_state<checkbox, decltype(&get_state)> state;
  void (*enter)(const checkbox& This) = nullptr;
  checkbox() noexcept : control(), state(this, get_state) {}
  checkbox(checkbox&& A) : control(mv(dynamic_cast<control&>(A))), state(this, get_state), enter(A.enter) {}
  checkbox& operator=(checkbox&& A) {
    return dynamic_cast<control&>(*this) = mv(dynamic_cast<control&>(A)), enter = A.enter, *this;
  }
  checkbox(const rect& Rect, const str2& Text, nat4 ButtonStyle = {})
    : control(L"BUTTON", Text.data(), BS_AUTOCHECKBOX | ButtonStyle, Rect, true), state(this, get_state) {
    if (!defproc) defproc = (WNDPROC)GetWindowLongPtrW(hwnd, GWLP_WNDPROC);
    SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)proc);
  }
  checkbox(natt GroupNo, const rect& Rect, const str2& Text, nat4 ButtonStyle = {})
    : control(L"BUTTON", Text.data(), BS_AUTOCHECKBOX | ButtonStyle, Rect, GroupNo, true), state(this, get_state) {
    if (!defproc) defproc = (WNDPROC)GetWindowLongPtrW(hwnd, GWLP_WNDPROC);
    SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)proc);
  }
};

/**
 * @brief class for creating a radiobutton control
 **********************************************************************************************************************/
class radiobutton : public control {
  nat4 get_index(HWND hw) {
    for (nat4 i{}; i < buttons.size(); ++i)
      if (buttons[i] == hw) return i;
    return nat4(-1);
  }
  void initialize(const rect& r, const str2& t, const auto& ts) {
    if (!defproc) defproc = (WNDPROC)GetWindowLongPtrW(hwnd, GWLP_WNDPROC);
    SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)proc);
    int4 height = r.height() / (1 + extent<decltype(ts)>), width = r.width();
    auto hw = CreateWindowExW(WS_EX_TOPMOST, L"BUTTON", t.data(),
                              WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON | WS_GROUP,
                              0, 0, width, height, hwnd, 0, main::system::hinstance, 0);
    if (!defproc2) defproc2 = (WNDPROC)GetWindowLongPtrW(hw, GWLP_WNDPROC);
    SendMessageW(hw, WM_SETFONT, (WPARAM)main::system::hfont, true);
    SetWindowLongPtrW(hw, GWLP_WNDPROC, (LONG_PTR)proc2);
    SetWindowLongPtrW(hw, GWLP_ID, (LONG_PTR)hw);
    buttons.emplace_back(hw);
    cfor<0, [](natt i) { return i < extent<decltype(ts)>; }>(
      [&](const str2& Text, natt i) {
        auto hw = CreateWindowExW(WS_EX_TOPMOST, L"BUTTON", Text.data(),
                                  WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,
                                  0, int4(i) * height, width,  height,
                                  hwnd, 0, main::system::hinstance, 0);
        SendMessageW(hw, WM_SETFONT, (WPARAM)main::system::hfont, true);
        SetWindowLongPtrW(hw, GWLP_WNDPROC, (LONG_PTR)proc2);
        SetWindowLongPtrW(hw, GWLP_ID, (LONG_PTR)hw);
        buttons.emplace_back(hw); }, ts, make_sequence<1 + extent<decltype(ts)>, 1>{});
    SendMessageW(buttons[0], BM_SETCHECK, BST_CHECKED, 0);
  }
protected:
  array<HWND> buttons{};
  inline static WNDPROC defproc = nullptr, defproc2 = nullptr;
  static LRESULT CALLBACK proc(HWND hw, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_COMMAND && HIWORD(wp) == BN_CLICKED) {
      auto& t = *static_cast<radiobutton*>(whole_group[hw].second);
      const_cast<nat4&>(t.state) = t.get_index((HWND)lp);
      std::cout << t.state << std::endl;
      if (t.enter) t.enter(t);
    }
    return CallWindowProcW(defproc, hw, msg, wp, lp);
  }
  static LRESULT CALLBACK proc2(HWND hw, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_LBUTTONUP) {
      if (auto& t = *static_cast<radiobutton*>(whole_group[GetParent(hw)].second); t.enter)
        return [&](auto r) { return t.enter(t), r; }(defproc(hw, msg, wp, lp));
    } else if (msg == WM_KEYUP) {
      if (wp == VK_RETURN || wp == VK_SPACE)
        if (auto& t = *static_cast<radiobutton*>(whole_group[GetParent(hw)].second); t.enter)
          return [&](auto r) { return t.enter(t), r; }(defproc(hw, msg, wp, lp));
    } else if (msg == WM_KEYDOWN) {
      if (wp == VK_RETURN) wp = VK_SPACE;
      else if (wp == VK_TAB) {
        auto p = GetParent(hw);
        auto& g = get_group(whole_group[p].first);
        auto i = get_index_in_group(g, p);
        return focus_on_next(g, i, GetKeyState(VK_SHIFT) < 0), 0;
      } else if (wp == VK_UP || wp == VK_LEFT) {
        auto& t = *static_cast<radiobutton*>(whole_group[GetParent(hw)].second);
        // const_cast<nat4&>(t.state) = static_cast<nat4>((t.get_index(hw) + t.buttons.size() - 1) % t.buttons.size());
        SetFocus(t.buttons[(t.get_index(hw) + t.buttons.size() - 1) % t.buttons.size()]);
      } else if (wp == VK_DOWN || wp == VK_RIGHT) {
        auto& t = *static_cast<radiobutton*>(whole_group[GetParent(hw)].second);
        // const_cast<nat4&>(t.state) = (t.get_index(hw) + 1) % t.buttons.size();
        SetFocus(t.buttons[(t.get_index(hw) + 1) % t.buttons.size()]);
      } else if (wp == VK_ESCAPE) return SetFocus(main::system::hwnd), 0;
    }
    return CallWindowProcW(defproc2, hw, msg, wp, lp);
  }
public:
  const nat4 state{};
  const nat4 count{};
  void (*enter)(const radiobutton& This) = nullptr;
  radiobutton() noexcept = default;
  radiobutton(radiobutton&& A) : control(mv(dynamic_cast<control&>(A))), buttons(mv(A.buttons)), count(A.count), enter(A.enter) {}
  radiobutton& operator=(radiobutton&& A) {
    dynamic_cast<control&>(*this) = mv(dynamic_cast<control&>(A));
    return buttons = mv(A.buttons), const_cast<nat4&>(count) = A.count, enter = A.enter, *this;
  }
  radiobutton(const rect& Rect, const str2& Title,
              const str2& FirstButtonText, convertible_to<str2> auto&&... ButtonTexts)
    : control(L"BUTTON", Title.data(), WS_BORDER | BS_GROUPBOX, Rect, true), count(1 + sizeof...(ButtonTexts)) {
    initialize(Rect, FirstButtonText, list<>::asref(ButtonTexts...));
  }
  radiobutton(natt GroupNo, const rect& Rect, const str2& Title,
              const str2& FirstButtonText, convertible_to<str2> auto&&... ButtonTexts)
    : control(L"BUTTON", Title.data(), WS_BORDER | BS_GROUPBOX, Rect, GroupNo, true), count(1 + sizeof...(ButtonTexts)) {
    initialize(Rect, FirstButtonText, list<>::asref(ButtonTexts...));
  }
  virtual void setfocus() const override { SetFocus(buttons[state]); }
};


// /// Base class for all controls.
// class control {
//   friend class bitmap;
// protected:
//   inline static constexpr natt max_groups = 16;
//   inline static array<array<list<HWND, control*>>, max_groups> groups{};
//   inline static array<list<HWND, control*>> others{};
//   inline static array<array<bitmap*>> bitmaps{};
//   inline static std::map<HWND, natt> whole_group{};
//   control(const cat2* Class, const cat2* Text, nat4 ExtraStyle, const rect& Rect)
//     : group(npos), hwnd(CreateWindowExW(0, Class, Text, WS_VISIBLE | WS_CHILD | WS_GROUP | ExtraStyle,
//                                         Rect.left, Rect.top, Rect.width(), Rect.height(),
//                                         main::system::hwnd, 0, main::system::hinstance, this)) {
//     SendMessageW(hwnd, WM_SETFONT, (WPARAM)main::system::hfont, true);
//     SetWindowLongPtrW(hwnd, GWLP_ID, (LONG_PTR)hwnd);
//     if (others.empty()) others.reserve(1024);
//     others.push_back(list{hwnd, this});
//     whole_group[hwnd] = npos;
//   }
//   control(const cat2* Class, const cat2* Text, nat4 ExtraStyle, const rect& Rect, const natt GroupNo)
//     : group(GroupNo), hwnd(CreateWindowExW(0, Class, Text, WS_VISIBLE | WS_CHILD | WS_GROUP | ExtraStyle,
//                                            Rect.left, Rect.top, Rect.width(), Rect.height(),
//                                            main::system::hwnd, 0, main::system::hinstance, this)) {
//     SendMessageW(hwnd, WM_SETFONT, (WPARAM)main::system::hfont, true);
//     SetWindowLongPtrW(hwnd, GWLP_ID, (LONG_PTR)hwnd);
//     if (GroupNo < max_groups) {
//       if (groups[GroupNo].empty()) groups[GroupNo].reserve(1024);
//       groups[GroupNo].push_back(list{hwnd, this});
//       whole_group[hwnd] = GroupNo;
//     } else {
//       ywee("Control group index is out of range.");
//       if (others.empty()) others.reserve(1024);
//       others.push_back(list{hwnd, this});
//       whole_group[hwnd] = npos;
//     }
//   }
//   static natt get_this(const array<list<HWND, control*>>& g, HWND hw) {
//     for (natt i{}; i < g.size(); ++i)
//       if (g[i].first == hw) return i;
//     return npos;
//   }
//   static control* get_prev(const array<list<HWND, control*>>& g, natt i) {
//     return g[(i + g.size() - 1) % g.size()].second;
//   }
//   static control* get_next(const array<list<HWND, control*>>& g, natt i) {
//     return g[(i + 1) % g.size()].second;
//   }
// public:
//   const HWND hwnd{};
//   const natt group{};
//   ~control() noexcept {
//     if (hwnd) {
//       DestroyWindow(hwnd);
//       if (group == npos) others.erase(others.begin() + get_this(others, hwnd));
//       else groups[group].erase(groups[group].begin() + get_this(groups[group], hwnd));
//       whole_group.erase(hwnd);
//     }
//   }
//   control() noexcept = default;
//   control(const control&) = delete;
//   control(control&& A) noexcept {
//     if (hwnd) {
//       DestroyWindow(hwnd);
//       if (group == npos) others.erase(others.begin() + get_this(others, hwnd));
//       else groups[group].erase(groups[group].begin() + get_this(groups[group], hwnd));
//     }
//     const_cast<HWND&>(hwnd) = exchange(const_cast<HWND&>(A.hwnd), nullptr);
//     const_cast<natt&>(group) = A.group;
//     if (group == npos) others[get_this(others, hwnd)].second = this;
//     else groups[group][get_this(groups[group], hwnd)].second = this;
//   }
//   control& operator=(control&& A) {
//     if (hwnd) {
//       DestroyWindow(hwnd);
//       if (group == npos) others.erase(others.begin() + get_this(others, hwnd));
//       else groups[group].erase(groups[group].begin() + get_this(groups[group], hwnd));
//     }
//     const_cast<HWND&>(hwnd) = exchange(const_cast<HWND&>(A.hwnd), nullptr);
//     const_cast<natt&>(group) = A.group;
//     if (group == npos) others[get_this(others, hwnd)].second = this;
//     else groups[group][get_this(groups[group], hwnd)].second = this;
//     return *this;
//   }
//   explicit operator bool() const noexcept { return hwnd; }
//   operator HWND() const noexcept { return hwnd; }
//   virtual void setfocus(bool = {}) const = 0;
//   virtual void hide(bool Hide = true) const { ShowWindow(hwnd, Hide ? SW_HIDE : SW_SHOW); }
//   static void hide_group(natt GroupNo, bool Hide = true) {
//     if (GroupNo >= max_groups) return ywee("Control group index is out of range."), void();
//     for (auto& c : groups[GroupNo]) c.second->hide(Hide);
//   }
//   virtual void text(const str2& Text) const { SetWindowTextW(hwnd, Text.data()); }
//   virtual str2 text() const {
//     str2 o(GetWindowTextLengthW(hwnd), cat2{});
//     return GetWindowTextW(hwnd, o.data(), int4(o.size() + 1)), mv(o);
//   }
// };


/// Class for creating a button.
// class button : public control {
// protected:
//   inline static WNDPROC defproc = nullptr;
//   static LRESULT CALLBACK proc(HWND hw, UINT msg, WPARAM wp, LPARAM lp) {
//     if (msg == WM_LBUTTONDOWN) PostMessageW(hw, WM_APP, 0, 0);
//     else if (msg == WM_KEYDOWN) {
//       if (wp == VK_RETURN) return PostMessageW(hw, WM_KEYDOWN, VK_SPACE, 0), 0;
//       else if (wp == VK_SPACE) PostMessageW(hw, WM_APP, 0, 0);
//       else if (wp == VK_TAB) {
//         natt i = whole_group[hw];
//         array<list<HWND, control*>>& g = (i == npos ? others : groups[i]);
//         if (GetKeyState(VK_SHIFT) < 0) get_prev(g, get_this(g, hw))->setfocus(true);
//         else get_next(g, get_this(g, hw))->setfocus(false);
//       } else if (wp == VK_ESCAPE) return SetFocus(main::system::hwnd), 0;
//     } else if (msg == WM_APP) {
//       natt i = whole_group[hw];
//       array<list<HWND, control*>>& g = (i == npos ? others : groups[i]);
//       button* t = static_cast<button*>(g[get_this(g, hw)].second);
//       return t->enter ? t->enter(), 0 : 0;
//     }
//     return CallWindowProcW(defproc, hw, msg, wp, lp);
//   }
// public:
//   button() noexcept = default;
//   button(button&&) noexcept = default;
//   button& operator=(button&&) noexcept = default;
//   /// Called when the button is pressed.
//   void (*enter)(void) = nullptr;
//   /// Creates a button.
//   button(const rect& Rect, const str2& Text, nat4 ButtonStyle = {})
//     : control(L"BUTTON", Text.data(), WS_BORDER | ButtonStyle, Rect) {
//     if (!defproc) defproc = (WNDPROC)GetWindowLongPtrW(hwnd, GWLP_WNDPROC);
//     SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)proc);
//   }
//   /// Creates and attaches a button to the specified group.
//   button(natt GroupNo, const rect& Rect, const str2& Text, nat4 ButtonStyle = {})
//     : control(L"BUTTON", Text.data(), WS_BORDER | ButtonStyle, Rect, GroupNo) {
//     if (!defproc) defproc = (WNDPROC)GetWindowLongPtrW(hwnd, GWLP_WNDPROC);
//     SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)proc);
//   }
//   /// Converts to the state of this control.
//   explicit operator bool() const { return this->operator()(); }
//   /// Obtains the state of this control.
//   bool operator()() const { return hwnd && SendMessageW(hwnd, BM_GETSTATE, 0, 0) == 108; }
//   /// Sets the text in this control.
//   void operator()(const str2& Text) { SetWindowTextW(hwnd, Text.data()); }
//   /// Sets the focus to this control.
//   virtual void setfocus(bool = {}) const override { hwnd ? SetFocus(hwnd) : none{}; }
// };

/// Class for creating a check box.
// class checkbox : public control {
// protected:
//   inline static WNDPROC defproc = nullptr;
//   static LRESULT CALLBACK proc(HWND hw, UINT msg, WPARAM wp, LPARAM lp) {
//     if (msg == WM_LBUTTONDOWN) PostMessageW(hw, WM_APP, 0, 0);
//     else if (msg == WM_KEYDOWN) {
//       if (wp == VK_RETURN) return PostMessageW(hw, WM_KEYDOWN, VK_SPACE, 0), 0;
//       else if (wp == VK_SPACE) PostMessageW(hw, WM_APP, 0, 0);
//       else if (wp == VK_TAB) {
//         natt i = whole_group[hw];
//         array<list<HWND, control*>>& g = (i == npos ? others : groups[i]);
//         if (GetKeyState(VK_SHIFT) < 0) get_prev(g, get_this(g, hw))->setfocus(true);
//         else get_next(g, get_this(g, hw))->setfocus(false);
//       } else if (wp == VK_ESCAPE) return SetFocus(main::system::hwnd), 0;
//     } else if (msg == WM_APP) {
//       natt i = whole_group[hw];
//       array<list<HWND, control*>>& g = (i == npos ? others : groups[i]);
//       checkbox* t = static_cast<checkbox*>(g[get_this(g, hw)].second);
//       return t->enter ? t->enter(!t->operator()()), 0 : 0;
//     }
//     return CallWindowProcW(defproc, hw, msg, wp, lp);
//   }
// public:
//   checkbox() noexcept = default;
//   checkbox(checkbox&&) noexcept = default;
//   checkbox& operator=(checkbox&&) noexcept = default;
//   /// Called when the checkbox is changed.
//   void (*enter)(bool Checked) = nullptr;
//   /// Creates a checkbox.
//   checkbox(const rect& Rect, const str2& Text, nat4 ButtonStyle = {})
//     : control(L"BUTTON", Text.data(), WS_BORDER | BS_AUTOCHECKBOX | ButtonStyle, Rect) {
//     if (!defproc) defproc = (WNDPROC)GetWindowLongPtrW(hwnd, GWLP_WNDPROC);
//     SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)proc);
//   }
//   /// Creates and attaches a checkbox to the specified group.
//   checkbox(natt GroupNo, const rect& Rect, const str2& Text, nat4 ButtonStyle = {})
//     : control(L"BUTTON", Text.data(), WS_BORDER | BS_AUTOCHECKBOX | ButtonStyle, Rect, GroupNo) {
//     if (!defproc) defproc = (WNDPROC)GetWindowLongPtrW(hwnd, GWLP_WNDPROC);
//     SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)proc);
//   }
//   /// Converts to the state of this control.
//   explicit operator bool() const { return this->operator()(); }
//   /// Obtains the state of this control.
//   bool operator()() const { return SendMessageW(hwnd, BM_GETCHECK, 0, 0); }
//   /// Sets the state of this control.
//   void operator()(bool Check) const { hwnd ? void() : void(SendMessageW(hwnd, BM_SETSTATE, Check, 0)); }
//   /// Sets the text in this control.
//   void operator()(const str2& Text) { SetWindowTextW(hwnd, Text.data()); }
//   /// Sets the focus to this control.
//   virtual void setfocus(bool = {}) const override { hwnd ? SetFocus(hwnd) : none{}; }
// };

/// Class for creating a group of radio buttons.
// class radiobutton : public control {
// protected:
//   array<HWND> buttons;
//   inline static WNDPROC defproc = nullptr, defproc2 = nullptr;
//   static LRESULT CALLBACK proc(HWND hw, UINT msg, WPARAM wp, LPARAM lp) {
//     if (msg == WM_COMMAND && HIWORD(wp) == BN_CLICKED) {
//       natt i = whole_group[hw];
//       array<list<HWND, control*>>& g = (i == npos ? others : groups[i]);
//       radiobutton* t = static_cast<radiobutton*>(g[get_this(g, hw)].second);
//       if (t->enter) t->enter(t->get_number((HWND)lp));
//     }
//     return CallWindowProcW(defproc, hw, msg, wp, lp);
//   }
//   static LRESULT CALLBACK proc2(HWND hw, UINT msg, WPARAM wp, LPARAM lp) {
//     switch (msg) {
//     case WM_KEYDOWN:
//       if (wp == VK_RETURN) wp = VK_SPACE;
//       else if (wp == VK_ESCAPE) return SetFocus(main::system::hwnd), 0;
//       else if (wp == VK_TAB) {
//         HWND parent = GetParent(hw);
//         natt i = whole_group[parent];
//         array<list<HWND, control*>>& g = (i == npos ? others : groups[i]);
//         if (GetKeyState(VK_SHIFT) < 0) get_prev(g, get_this(g, parent))->setfocus(true);
//         else get_next(g, get_this(g, parent))->setfocus(false);
//       } else if (wp == VK_UP || wp == VK_LEFT) {
//         HWND parent = GetParent(hw);
//         natt i = whole_group[parent];
//         array<list<HWND, control*>>& g = (i == npos ? others : groups[i]);
//         radiobutton* t = static_cast<radiobutton*>(g[get_this(g, parent)].second);
//         SetFocus(t->buttons[(t->get_number(hw) + t->buttons.size() - 1) % t->buttons.size()]);
//       } else if (wp == VK_DOWN || wp == VK_RIGHT) {
//         HWND parent = GetParent(hw);
//         natt i = whole_group[parent];
//         array<list<HWND, control*>>& g = (i == npos ? others : groups[i]);
//         radiobutton* t = static_cast<radiobutton*>(g[get_this(g, parent)].second);
//         SetFocus(t->buttons[(t->get_number(hw) + 1) % t->buttons.size()]);
//       }
//     case WM_KEYUP:
//       if (wp == VK_RETURN) wp = VK_SPACE;
//     }
//     return CallWindowProcW(defproc2, hw, msg, wp, lp);
//   }
//   nat4 get_number(HWND hw) {
//     for (nat4 i{}; i < buttons.size(); ++i)
//       if (buttons[i] == hw) return i;
//     return nat4(-1);
//   }
//   void initialize(const rect& r, const str2& t, const auto& ts) {
//     if (!defproc) defproc = (WNDPROC)GetWindowLongPtrW(hwnd, GWLP_WNDPROC);
//     SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)proc);
//     int4 height = r.height() / (1 + extent<decltype(ts)>), width = r.width();
//     auto hw = CreateWindowExW(WS_EX_TOPMOST, L"BUTTON", t.data(),
//                               WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON | WS_GROUP,
//                               0, 0, width, height, hwnd, 0, main::system::hinstance, 0);
//     if (!defproc2) defproc2 = (WNDPROC)GetWindowLongPtrW(hw, GWLP_WNDPROC);
//     SendMessageW(hw, WM_SETFONT, (WPARAM)main::system::hfont, true);
//     SetWindowLongPtrW(hw, GWLP_WNDPROC, (LONG_PTR)proc2);
//     SetWindowLongPtrW(hw, GWLP_ID, (LONG_PTR)hw);
//     buttons.emplace_back(hw);
//     cfor<0, [](natt i) { return i < extent<decltype(ts)>; }>(
//       [&](const str2& Text, natt i) {
//         auto hw = CreateWindowExW(WS_EX_TOPMOST, L"BUTTON", Text.data(),
//                                   WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,
//                                   0, int4(i) * height, width,  height,
//                                   hwnd, 0, main::system::hinstance, 0);
//         SendMessageW(hw, WM_SETFONT, (WPARAM)main::system::hfont, true);
//         SetWindowLongPtrW(hw, GWLP_WNDPROC, (LONG_PTR)proc2);
//         SetWindowLongPtrW(hw, GWLP_ID, (LONG_PTR)hw);
//         buttons.emplace_back(hw); }, ts, make_sequence<1 + extent<decltype(ts)>, 1>{});
//     SendMessageW(buttons[0], BM_SETCHECK, BST_CHECKED, 0);
//   }
// public:
//   radiobutton() noexcept = default;
//   radiobutton(radiobutton&&) noexcept = default;
//   radiobutton& operator=(radiobutton&&) noexcept = default;
//   /// Called when a radiobutton is pressed.
//   void (*enter)(nat4 Index) = nullptr;
//   /// Creates a group of radio buttons.
//   radiobutton(const rect& Rect, const str2& Title, const str2& FirstButtonText,
//               convertible_to<str2> auto&&... ButtonTexts)
//     : control(L"BUTTON", Title.data(), WS_BORDER | BS_GROUPBOX, Rect) {
//     initialize(Rect, FirstButtonText, list<>::asref(ButtonTexts...));
//   }
//   /// Creates and attaches a group of radio buttons to the specified group.
//   radiobutton(natt GroupNo, const rect& Rect, const str2& Title, const str2& FirstButtonText,
//               convertible_to<str2> auto&&... ButtonTexts)
//     : control(L"BUTTON", Title.data(), WS_BORDER | BS_GROUPBOX, Rect, GroupNo) {
//     initialize(Rect, FirstButtonText, list<>::asref(ButtonTexts...));
//   }
//   /// Obtains the count of radio buttons in this group.
//   nat4 size() const { return static_cast<nat4>(buttons.size()); }
//   /// Converts to the index of the selected button.
//   operator nat4() const { return this->operator()(); }
//   /// Obtains the index of the selected button.
//   nat4 operator()() const {
//     for (nat4 i{}; i < size(); ++i)
//       if (SendMessageW(buttons[i], BM_GETCHECK, 0, 0) == BST_CHECKED) return i;
//     return nat4(-1);
//   }
//   /// Sets the index of the selected button.
//   void operator()(natt Index) const {
//     if (Index < size()) SendMessageW(buttons[Index], BM_SETCHECK, BST_CHECKED, 0);
//     else ywee("Index is out of range.");
//   }
//   /// Sets the focus to this control.
//   void setfocus(bool = {}) const override { hwnd ? SetFocus(buttons[0]) : none{}; }
// };

/// class for displaying a bitmap on main-window
class bitmap : public control {
protected:
  yw::rect yw_rect{};
  yw::bitmap yw_bitmap{};
  bitmap(const bitmap&) = delete;
  bitmap& operator=(const bitmap&) = delete;
  bitmap(yw::bitmap&& Bitmap) : yw_bitmap(mv(Bitmap)) {}
public:
  bitmap() noexcept = default;
  bitmap(bitmap&&) noexcept = default;
  bitmap& operator=(bitmap&&) noexcept = default;
  bitmap(const rect& Rect) : control(L"STATIC", L"", 0, {}, false), yw_rect(Rect), yw_bitmap(Rect.width(), Rect.height()) {}
  bitmap(natt GroupNo, const rect& Rect) : control(L"STATIC", L"", 0, {}, GroupNo, false), yw_rect(Rect), yw_bitmap(Rect.width(), Rect.height()) {}
};

class coordinator {
protected:
  texture tex;
  camera cmr;
  coordinator(const coordinator&) = delete;
  coordinator& operator=(const coordinator&) = delete;
  void initialize() {
    static constexpr auto ff = font<200, L"Yu Gothic UI", 0>{};
    static constexpr auto bb = brush<color::white>{};
    cmr.orthographic = true;
    cmr.offset.z = -10;
    cmr.magnification = 0.3125f * min(cmr.width, cmr.height);
    tex.begin_draw(color::black);
    rect{0, 0, 1024, 512}.draw_rectangle(brush<color::red>{});
    rect{1024, 0, 2048, 512}.draw_rectangle(brush<color::blue>{});
    rect{2048, 0, 3072, 512}.draw_rectangle(brush<color::green>{});
    rect{0, 100, 512, 512}.draw_text(L"+X", ff, bb);
    rect{512, 100, 1024, 512}.draw_text(L"-X", ff, bb);
    rect{1024, 100, 1512, 512}.draw_text(L"+Y", ff, bb);
    rect{1512, 100, 2048, 512}.draw_text(L"-Y", ff, bb);
    rect{2048, 100, 2512, 512}.draw_text(L"+Z", ff, bb);
    rect{2512, 100, 3072, 512}.draw_text(L"-Z", ff, bb);
    tex.end_draw();
    operator()({});
  }
public:
  coordinator() noexcept = default;
  coordinator(coordinator&&) noexcept = default;
  coordinator& operator=(coordinator&&) noexcept = default;
  coordinator(natt Width, natt Height, natt Msaa = {}) : tex(4096, 512), cmr(Width, Height, Msaa) { initialize(); }
  void operator()(const vector& Radians, const color& Background = color::black) {
    static constexpr stv1 hlsl = R"(
#pragma pack_matrix(row_major)
struct SB { float4 p, t; };
struct PSIN {
  float4 p : SV_Position;
  float2 t : TEXCOORD;
};
cbuffer CB : register(b0) { matrix v; };
StructuredBuffer<SB> sb : register(t0);
void vsmain(uint In : SV_VertexID, out PSIN Out) {
  Out.p = mul(v, float4(sb[In].p.xyz, 1));
  Out.t = sb[In].t.xy;
}
Texture2D tx : register(t0);
SamplerState ss : register(s0);
void psmain(PSIN In, out float4 Out : SV_Target) {
  Out = tx.Sample(ss, In.t);
})";
    static constexpr array<list<vector, vector>, 36> vv{
      // clang-format off
      list{vector{1, 1, 1}, vector{0, 0}}, list{vector{1, -1, 1}, vector{0, 1}}, list{vector{1, 1, -1}, vector{0.125f, 0}},              // +X
      list{vector{1, -1, -1}, vector{0.125f, 1}}, list{vector{1, 1, -1}, vector{0.125f, 0}}, list{vector{1, -1, 1}, vector{0, 1}},
      list{vector{-1, 1, -1}, vector{0.125f, 0}}, list{vector{-1, -1, -1}, vector{0.125f, 1}}, list{vector{-1, 1, 1}, vector{0.25f, 0}}, // -X
      list{vector{-1, -1, 1}, vector{0.25f, 1}}, list{vector{-1, 1, 1}, vector{0.25f, 0}}, list{vector{-1, -1, -1}, vector{0.125f, 1}},
      list{vector{1, 1, 1}, vector{0.25f, 0}}, list{vector{1, 1, -1}, vector{0.25f, 1}}, list{vector{-1, 1, 1}, vector{0.375f, 0}},      // +Y
      list{vector{-1, 1, -1}, vector{0.375f, 1}}, list{vector{-1, 1, 1}, vector{0.375f, 0}}, list{vector{1, 1, -1}, vector{0.25f, 1}},
      list{vector{-1, -1, 1}, vector{0.375f, 0}}, list{vector{-1, -1, -1}, vector{0.375f, 1}}, list{vector{1, -1, 1}, vector{0.5f, 0}},  // -Y
      list{vector{1, -1, -1}, vector{0.5f, 1}}, list{vector{1, -1, 1}, vector{0.5f, 0}}, list{vector{-1, -1, -1}, vector{0.375f, 1}},
      list{vector{-1, 1, 1}, vector{0.5f, 0}}, list{vector{-1, -1, 1}, vector{0.5f, 1}}, list{vector{1, 1, 1}, vector{0.625f, 0}},       // +Z
      list{vector{1, -1, 1}, vector{0.625f, 1}}, list{vector{1, 1, 1}, vector{0.625f, 0}}, list{vector{-1, -1, 1}, vector{0.5f, 1}},
      list{vector{1, 1, -1}, vector{0.625f, 0}}, list{vector{1, -1, -1}, vector{0.625f, 1}}, list{vector{-1, 1, -1}, vector{0.75f, 0}},  // -Z
      list{vector{-1, -1, -1}, vector{0.75f, 1}}, list{vector{-1, 1, -1}, vector{0.75f, 0}}, list{vector{1, -1, -1}, vector{0.625f, 1}},
    }; // clang-format on
    static auto rd = renderer<typepack<structured_buffer<list<vector, vector>>>, typepack<xmatrix>, typepack<>, typepack<>, typepack<texture>>(hlsl);
    static auto sb = structured_buffer<list<vector, vector>>(vv.data(), 36);
    static constant_buffer<xmatrix> cb;
    try {
      tiff(tex);
      cmr.rotation = Radians, cmr.update();
      cb.load(cmr.view_projection_matrix);
      cmr.begin_render(Background);
      rd(36, {sb}, {cb}, {}, {}, {tex});
      cmr.end_render();
    } catch (const std::exception& E) { throw except(E); }
  }
  void operator()(const rect& Rect) const { rect{Rect}.draw_bitmap(cmr); }
  // void operator()(const array<const yw::bitmap&, 6>& PlusXYZ_MinusXYZ) const {
  //   tex.begin_draw(color::black);
  //   if (PlusXYZ_MinusXYZ[0]) rect{0, 0, 512, 512}.draw_bitmap(PlusXYZ_MinusXYZ[0]);
  //   if (PlusXYZ_MinusXYZ[1]) rect{1024, 0, 1536, 512}.draw_bitmap(PlusXYZ_MinusXYZ[1]);
  //   if (PlusXYZ_MinusXYZ[2]) rect{2048, 0, 2560, 512}.draw_bitmap(PlusXYZ_MinusXYZ[2]);
  //   if (PlusXYZ_MinusXYZ[3]) rect{512, 0, 1024, 512}.draw_bitmap(PlusXYZ_MinusXYZ[3]);
  //   if (PlusXYZ_MinusXYZ[4]) rect{1536, 0, 2048, 512}.draw_bitmap(PlusXYZ_MinusXYZ[4]);
  //   if (PlusXYZ_MinusXYZ[5]) rect{2560, 0, 3072, 512}.draw_bitmap(PlusXYZ_MinusXYZ[5]);
  //   tex.end_draw();
  // }
};

/// Class for creating a progress bar.
class progressbar {
protected:
  yw::bitmap bm;
  yw::brush<> brush;
  progressbar(const progressbar&) = delete;
  progressbar& operator=(const progressbar&) = delete;
public:
  color background{color::white};
  progressbar() noexcept = default;
  progressbar(progressbar&&) = default;
  progressbar& operator=(progressbar&&) = default;
  explicit operator bool() const noexcept { return bool(bm); }
  operator comptr<ID2D1Bitmap1>::reference*() const noexcept { return bm; }
  progressbar(natt Width, natt Height, const color& Color) : bm(Width, Height), brush(Color) { bm.begin_draw(background), bm.end_draw(); }
  void operator()(fat4 Progress) { bm.begin_draw(background), rect{0, 0, bm.width * Progress, bm.height}.draw_rectangle(brush), bm.end_draw(); }
  void operator()(const color& Color) { brush = yw::brush(Color); }
  void operator()(const rect& Rect) const { Rect.draw_bitmap(bm); }
};
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  MAIN FUNCTION

/// Main Function for YWLIB
extern void
ywmain();

#ifndef ywlib_disable_main
#ifdef ywlib_enable_console
int wmain() {
  using namespace yw;
  try {
    std::wcout.imbue(std::locale("Japanese"));
    tiff(CoInitialize(nullptr));
    main::resize(400, 400);
    ShowWindow(main::system::hwnd, SW_SHOW);
    SetForegroundWindow(main::system::hwnd);
    SetFocus(main::system::hwnd);
    main::system::timer.start();
    ywmain();
    return 0;
  } catch (const std::exception& e) { std::cout << e.what(); }
  CoUninitialize();
  return -1;
}
#else
int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int) {
  using namespace yw;
  try {
    std::wcout.imbue(std::locale("Japanese"));
    tiff(CoInitialize(nullptr));
    main::resize(400, 400);
    ShowWindow(main::system::hwnd, SW_SHOW);
    SetForegroundWindow(main::system::hwnd);
    SetFocus(main::system::hwnd);
    main::system::timer.start();
    ywmain();
    return 0;
  } catch (const std::exception& e) { ui::ok(e.what(), "FATAL ERROR"); }
  CoUninitialize();
  return -1;
}
#endif
#endif

namespace yw {

// gpgpu

template<specialization_of<typepack> Unordered,
         specialization_of<typepack> Structured = typepack<>,
         specialization_of<typepack> Constant = typepack<>>
class gpgpu {
  static_assert(Unordered::size != 0);
public:
  using ub_list = list<>::from_typepack<Unordered, const unordered_buffer<int4>&>;
  using sb_list = list<>::from_typepack<Structured, const structured_buffer<int4>&>;
  using cb_list = list<>::from_typepack<Constant, const constant_buffer<vector>&>;
  comptr<ID3D11ComputeShader> cs{};
  gpgpu() noexcept = default;
  gpgpu(gpgpu&&) noexcept = default;
  gpgpu& operator=(gpgpu&&) noexcept = default;
  gpgpu(stv1 Hlsl, str1 Entry = "csmain", str1 Target = "cs_5_0") {
    comptr<ID3DBlob> b, r;
    if (0 > D3DCompile(Hlsl.data(), Hlsl.size(), 0, 0, 0, Entry.data(), Target.data(),
                       D3D10_SHADER_ENABLE_STRICTNESS, 0, &b, &r)) throw except((cat1*)r->GetBufferPointer());
    tiff(main::system::d3d_device->CreateComputeShader(b->GetBufferPointer(), b->GetBufferSize(), nullptr, &cs));
  }
  void operator()(ub_list UBuffers, sb_list SBuffers, cb_list CBuffers,
                  nat4 ThreadGroupX, nat4 ThreadGroupY = 1, nat4 ThreadGroupZ = 1) const {
    using ubseq = make_indices_for<ub_list>;
    tiff([&]<natt... Is>(sequence<Is...>) { return (bool(get<Is>(UBuffers)) && ...); }(ubseq{}));
    [&]<natt... Is>(sequence<Is...>) { (get<Is>(UBuffers).to_cs(Is), ...); }(ubseq{});
    using sbseq = make_indices_for<sb_list>;
    if constexpr (extent<sb_list> != 0) {
      tiff([&]<natt... Is>(sequence<Is...>) { return (bool(get<Is>(SBuffers)) && ...); }(sbseq{}));
      [&]<natt... Is>(sequence<Is...>) { (get<Is>(SBuffers).to_cs(Is), ...); }(sbseq{});
    }
    using cbseq = make_indices_for<cb_list>;
    if constexpr (extent<cb_list> != 0) {
      tiff([&]<natt... Is>(sequence<Is...>) { return (bool(get<Is>(CBuffers)) && ...); }(cbseq{}));
      [&]<natt... Is>(sequence<Is...>) { (get<Is>(CBuffers).to_cs(Is), ...); }(cbseq{});
    }
    main::system::d3d_context->CSSetShader(cs, nullptr, 0);
    main::system::d3d_context->Dispatch(ThreadGroupX, ThreadGroupY, ThreadGroupZ);
    [&]<natt... Is>(ID3D11UnorderedAccessView* t, sequence<Is...>) { (main::system::d3d_context->CSSetUnorderedAccessViews(Is, 1, &t, 0), ...); }({}, ubseq{});
    [&]<natt... Is>(ID3D11ShaderResourceView* t, sequence<Is...>) { (main::system::d3d_context->CSSetShaderResources(Is, 1, &t), ...); }({}, sbseq{});
    [&]<natt... Is>(ID3D11Buffer* t, sequence<Is...>) { (main::system::d3d_context->CSSetConstantBuffers(Is, 1, &t), ...); }({}, cbseq{});
  }
  /// @brief 一次元に並列化したGPGPUを実行する。
  /// @param Parallels 並列数
  /// @note  動作条件：Shaderにおいて`numthreads(1024, 1, 1)`が指定されていること。
  void operator()(natt Parallels, ub_list UBuffers, sb_list SBuffers, cb_list CBuffers) const {
    this->operator()(mv(UBuffers), mv(SBuffers), mv(CBuffers), nat4(Parallels - 1) / 1024u + 1, 1, 1);
  }
};

// vertex_shader

enum class topology {
  undefined = static_cast<int4>(D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED),
  pointlist = static_cast<int4>(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST),
  linelist = static_cast<int4>(D3D11_PRIMITIVE_TOPOLOGY_LINELIST),
  linestrip = static_cast<int4>(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP),
  trianglelist = static_cast<int4>(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST),
  trianglestrip = static_cast<int4>(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP),
};

template<yw::topology Topology, typename In, typename Out,
         specialization_of<typepack> Constant = typepack<>,
         convertible_to<comptr<ID3D11ShaderResourceView>::reference*>... Resource>
class vertex_shader {
public:
  using input = In;
  using output = Out;
  using cb_list = list<>::from_typepack<Constant, const constant_buffer<fat4[4]>&>;
  using sr_list = list<const Resource&...>;
  inline static constexpr yw::topology topology = Topology;
  comptr<ID3D11InputLayout> d3d_il;
  comptr<ID3D11VertexShader> d3d_vs;
  bool wireframe = false;
  explicit operator bool() const noexcept { return bool(d3d_vs) && bool(d3d_il); }
  operator comptr<ID3D11VertexShader>::reference*() const noexcept { return d3d_vs; }
  vertex_shader() noexcept = default;
  template<natt Nx> vertex_shader(const D3D11_INPUT_ELEMENT_DESC (&Inputs)[Nx], stv1 Hlsl,
                                  const str1& Entry = "vsmain", const str1& Target = "vs_5_0") {
    comptr<ID3DBlob> b, e;
    if (0 > D3DCompile(Hlsl.data(), Hlsl.size(), 0, 0, 0, Entry.data(), Target.data(),
                       D3D10_SHADER_ENABLE_STRICTNESS, 0, &b, &e)) throw except((cat1*)e->GetBufferPointer());
    tiff(main::system::d3d_device->CreateVertexShader(b->GetBufferPointer(), b->GetBufferSize(), 0, &d3d_vs));
    tiff(main::system::d3d_device->CreateInputLayout(Inputs, (nat4)Nx, b->GetBufferPointer(), b->GetBufferSize(), &d3d_il));
  }
  void operator()(const vertex_buffer<input>& vb, const index_buffer& ib,
                  cb_list CBuffers, sr_list Resources) const {
    const nat4 stride = (nat4)sizeof(input), offset = 0;
    main::system::d3d_context->IASetPrimitiveTopology((D3D11_PRIMITIVE_TOPOLOGY)Topology);
    main::system::d3d_context->IASetInputLayout(d3d_il);
    main::system::d3d_context->IASetVertexBuffers(0, 1, vb.operator&(), &stride, &offset);
    main::system::d3d_context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);
    main::system::d3d_context->VSSetShader(d3d_vs, nullptr, 0);
    [&]<natt... Is>(sequence<Is...>) { ((yw::get<Is>(CBuffers).to_vs(Is)), ...); }(make_indices_for<cb_list>{});
    [&]<natt... Is>(sequence<Is...>) { ((yw::get<Is>(Resources).to_vs(Is)), ...); }(make_indices_for<sr_list>{});
  }
};

template<topology Topology, typename Out, specialization_of<typepack> Constant,
         convertible_to<comptr<ID3D11ShaderResourceView>::reference*>... Resource>
class vertex_shader<Topology, none, Out, Constant, Resource...> {
public:
  using input = none;
  using output = Out;
  using cb_list = list<>::from_typepack<Constant, const constant_buffer<fat4[4]>&>;
  using sr_list = list<const Resource&...>;
  inline static constexpr yw::topology topology = Topology;
  inline static const comptr<ID3D11InputLayout> d3d_il{};
  comptr<ID3D11VertexShader> d3d_vs;
  explicit operator bool() const noexcept { return bool(d3d_vs); }
  operator comptr<ID3D11VertexShader>::reference*() const noexcept { return d3d_vs; }
  vertex_shader() noexcept = default;
  vertex_shader(stv1 Hlsl, const str1& Entry = "vsmain", const str1& Target = "vs_5_0") {
    comptr<ID3DBlob> b, e;
    if (0 > D3DCompile(Hlsl.data(), Hlsl.size(), 0, 0, 0, Entry.data(), Target.data(),
                       D3D10_SHADER_ENABLE_STRICTNESS, 0, &b, &e)) throw except((cat1*)e->GetBufferPointer());
    tiff(main::system::d3d_device->CreateVertexShader(b->GetBufferPointer(), b->GetBufferSize(), 0, &d3d_vs));
  }
  void operator()(none, none, cb_list CBuffers, sr_list Resources) const {
    main::system::d3d_context->IASetPrimitiveTopology((D3D11_PRIMITIVE_TOPOLOGY)topology);
    main::system::d3d_context->IASetInputLayout(d3d_il);
    main::system::d3d_context->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
    main::system::d3d_context->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
    main::system::d3d_context->VSSetShader(d3d_vs, nullptr, 0);
    [&]<natt... Is>(sequence<Is...>) { ((get<Is>(CBuffers).to_vs(Is)), ...); }(make_indices_for<cb_list>{});
    [&]<natt... Is>(sequence<Is...>) { ((get<Is>(Resources).to_vs(Is)), ...); }(make_indices_for<sr_list>{});
  }
};

// geometry_shader

template<topology Topology, typename In, typename Out,
         specialization_of<typepack> Constant = typepack<>,
         convertible_to<comptr<ID3D11ShaderResourceView>::reference*>... Resource>
class geometry_shader {
public:
  using input = In;
  using output = Out;
  using cb_list = list<>::from_typepack<Constant, const constant_buffer<fat4[4]>&>;
  using sr_list = list<const Resource&...>;
  inline static constexpr yw::topology topology = Topology;
  comptr<ID3D11GeometryShader> d3d_gs;
  explicit operator bool() const noexcept { return comptr<ID3D11GeometryShader>::operator bool(); }
  operator comptr<ID3D11GeometryShader>::reference*() const noexcept { return d3d_gs; }
  geometry_shader() noexcept = default;
  geometry_shader(stv1 Hlsl, const str1& Entry = "gsmain", const str1& Target = "gs_5_0") {
    comptr<ID3DBlob> b, e;
    if (0 > D3DCompile(Hlsl.data(), Hlsl.size(), 0, 0, 0, Entry.data(), Target.data(),
                       D3D10_SHADER_ENABLE_STRICTNESS, 0, &b, &e)) throw except((cat1*)e->GetBufferPointer());
    tiff(main::system::d3d_device->CreateGeometryShader(b->GetBufferPointer(), b->GetBufferSize(), 0, &d3d_gs));
  }
  void operator()(cb_list CBuffers, sr_list Resources) const {
    main::system::d3d_context->GSSetShader(*this, nullptr, 0);
    [&]<natt... Is>(sequence<Is...>) { ((yw::get<Is>(CBuffers).to_gs(Is)), ...); }(make_indices_for<cb_list>{});
    [&]<natt... Is>(sequence<Is...>) { ((yw::get<Is>(Resources).to_gs(Is)), ...); }(make_indices_for<sr_list>{});
  }
};

// pixel_shader

template<typename In, specialization_of<typepack> Constant = typepack<>,
         convertible_to<comptr<ID3D11ShaderResourceView>::reference*>... Resource>
class pixel_shader {
public:
  using input = In;
  using cb_list = list<>::from_typepack<Constant, const constant_buffer<float[4]>&>;
  using sr_list = list<const Resource&...>;
  comptr<ID3D11PixelShader> d3d_ps;
  explicit operator bool() const noexcept { return bool(d3d_ps); }
  operator comptr<ID3D11PixelShader>::reference*() const noexcept { return d3d_ps; }
  pixel_shader() noexcept = default;
  pixel_shader(stv1 Hlsl, const str1& Entry = "psmain", const str1& Target = "ps_5_0") {
    comptr<ID3DBlob> b, e;
    if (0 > D3DCompile(Hlsl.data(), Hlsl.size(), 0, 0, 0, Entry.data(), Target.data(),
                       D3D10_SHADER_ENABLE_STRICTNESS, 0, &b, &e)) throw except((cat1*)e->GetBufferPointer());
    tiff(main::system::d3d_device->CreatePixelShader(b->GetBufferPointer(), b->GetBufferSize(), 0, &d3d_ps));
  }
  void operator()(cb_list cb, sr_list sr) const {
    main::system::d3d_context->PSSetShader(*this, nullptr, 0);
    [&]<natt... Is>(sequence<Is...>) { ((yw::get<Is>(cb).to_ps(Is)), ...); }(make_indices_for<cb_list>{});
    [&]<natt... Is>(sequence<Is...>) { ((yw::get<Is>(sr).to_ps(Is)), ...); }(make_indices_for<sr_list>{});
  }
};

// model

template<typename VertexType> class model {
public:
  using vertex_type = VertexType;
  vector position{};
  vector rotation{};
  vector scale{1.0f, 1.0f, 1.0f, 1.0f};
  vector color{};
  yw::vertex_buffer<VertexType> vertex_buffer{};
  yw::index_buffer index_buffer{};
  explicit operator bool() const noexcept { return bool(vertex_buffer); }
  model() noexcept = default;
  model(yw::vertex_buffer<VertexType> Vb) : vertex_buffer(mv(Vb)), index_buffer() {}
  model(yw::vertex_buffer<VertexType> Vb, yw::index_buffer Ib) : vertex_buffer(mv(Vb)), index_buffer(mv(Ib)) {}
  void matrix(xmatrix& Matrix) const noexcept { xvworld(position, xvradian(rotation), scale, Matrix); }
  template<typename VShader, typename GShader, typename PShader>
  void render(const VShader& Vs, const GShader& Gs, const PShader& Ps,
              VShader::cb_list VSCBuffers, VShader::sr_list VSResources,
              GShader::cb_list GSCBuffers = {}, GShader::sr_list GSResources = {},
              PShader::cb_list PSCBuffers = {}, PShader::sr_list PSResources = {}) const
    requires requires {
      requires variation_of<VShader, vertex_shader<topology{}, void, void>>;
      requires variation_of<GShader, geometry_shader<topology{}, void, void>>;
      requires variation_of<PShader, pixel_shader<void>>;
      requires same_as<typename VShader::output, typename GShader::input>;
      requires same_as<typename GShader::output, typename PShader::input>;
      requires VShader::topology == GShader::topology;
      Vs(vertex_buffer, index_buffer, VSCBuffers, VSResources);
      Gs(GSCBuffers, GSResources), Ps(PSCBuffers, PSResources);
    } {
    Vs(vertex_buffer, index_buffer, VSCBuffers, VSResources);
    Gs(GSCBuffers, GSResources), Ps(PSCBuffers, PSResources);
    vertex_buffer.to_vs(), index_buffer.to_vs();
    if (index_buffer) main::system::d3d_context->DrawIndexed(index_buffer.count, 0, 0);
    else main::system::d3d_context->Draw(vertex_buffer.count, 0);
  }
  template<typename VShader, typename PShader>
  void render(const VShader& Vs, const PShader& Ps,
              VShader::cb_list VSCBuffers, VShader::sr_list VSResources,
              PShader::cb_list PSCBuffers = {}, PShader::sr_list PSResources = {}) const
    requires requires {
      requires variation_of<VShader, vertex_shader<topology{}, void, void>>;
      requires variation_of<PShader, pixel_shader<void>>;
      requires same_as<typename VShader::output, typename PShader::input>;
      Vs(vertex_buffer, index_buffer, VSCBuffers, VSResources), Ps(PSCBuffers, PSResources);
    } {
    Vs(vertex_buffer, index_buffer, VSCBuffers, VSResources), Ps(PSCBuffers, PSResources);
    if (index_buffer) main::system::d3d_context->DrawIndexed(index_buffer.count, 0, 0);
    else main::system::d3d_context->Draw(vertex_buffer.count, 0);
  }
};

template<> class model<none> {
public:
  using vertex = none;
  vector position{};
  vector rotation{};
  vector scale{1.0f, 1.0f, 1.0f, 1.0f};
  vector color{};
  yw::vertex_buffer<none> vertex_buffer{};
  yw::index_buffer index_buffer{};
  explicit operator bool() const noexcept { return bool(vertex_buffer); }
  model() noexcept = default;
  model(natt Size) : vertex_buffer(Size) {}
  model(yw::vertex_buffer<none> Vb) : vertex_buffer(mv(Vb)) {}
  void matrix(xmatrix& Matrix) const noexcept { xvworld(position, xvradian(rotation), scale, Matrix); }
  template<typename VShader, typename GShader, typename PShader>
  void render(const VShader& Vs, const GShader& Gs, const PShader& Ps,
              VShader::cb_list VSCBuffers, VShader::sr_list VSResources,
              GShader::cb_list GSCBuffers = {}, GShader::sr_list GSResources = {},
              PShader::cb_list PSCBuffers = {}, PShader::sr_list PSResources = {}) const
    requires requires {
      requires variation_of<VShader, vertex_shader<topology{}, void, void>>;
      requires variation_of<GShader, geometry_shader<topology{}, void, void>>;
      requires variation_of<PShader, pixel_shader<void>>;
      requires same_as<typename VShader::output, typename GShader::input>;
      requires same_as<typename GShader::output, typename PShader::input>;
      requires VShader::topology == GShader::topology;
      Vs(vertex_buffer, {}, VSCBuffers, VSResources);
      Gs(GSCBuffers, GSResources), Ps(PSCBuffers, PSResources);
    } {
    Vs(vertex_buffer, index_buffer, VSCBuffers, VSResources);
    Gs(GSCBuffers, GSResources), Ps(PSCBuffers, PSResources);
    vertex_buffer.to_vs(), main::system::d3d_context->Draw(vertex_buffer.count, 0);
  }
  template<typename VShader, typename PShader>
  void render(const VShader& Vs, const PShader& Ps,
              VShader::cb_list VSCBuffers, VShader::sr_list VSResources,
              PShader::cb_list PSCBuffers = {}, PShader::sr_list PSResources = {}) const
    requires requires {
      requires variation_of<VShader, vertex_shader<topology{}, void, void>>;
      requires variation_of<PShader, pixel_shader<void>>;
      requires same_as<typename VShader::output, typename PShader::input>;
      Vs(vertex_buffer, {}, VSCBuffers, VSResources), Ps(PSCBuffers, PSResources);
    } {
    Vs(vertex_buffer, index_buffer, VSCBuffers, VSResources), Ps(PSCBuffers, PSResources);
    vertex_buffer.to_vs(), main::system::d3d_context->Draw(vertex_buffer.count, 0);
  }
};

model(natt) -> model<none>;
template<typename Ty> model(vertex_buffer<Ty>) -> model<Ty>;
template<typename Ty> model(vertex_buffer<Ty>, index_buffer) -> model<Ty>;

// pipeline

template<typename VShader, typename PShader, typename GShader = none> class pipeline {
  static_assert(variation_of<yw::vertex_shader<topology{}, void, void>, VShader>);
  static_assert(variation_of<yw::geometry_shader<topology{}, void, void>, GShader>);
  static_assert(variation_of<yw::pixel_shader<void>, PShader>);
  static_assert(same_as<typename VShader::output, typename GShader::input>);
  static_assert(same_as<typename GShader::output, typename PShader::input>);
  static_assert(VShader::topology == GShader::topology);
public:
  using vertex_shader = VShader;
  using geometry_shader = GShader;
  using pixel_shader = PShader;
  vertex_shader vs;
  geometry_shader gs;
  pixel_shader ps;
  pipeline() noexcept = default;
  pipeline(stv1 Hlsl) : vs(Hlsl), gs(Hlsl), ps(Hlsl) {}
  pipeline(vertex_shader Vs, geometry_shader Gs, pixel_shader Ps) : vs(mv(Vs)), gs(mv(Gs)), ps(mv(Ps)) {}
  explicit operator bool() const noexcept { return bool(vs) && bool(gs) && bool(ps); }
  void operator()(const model<typename vertex_shader::input>& Model,
                  typename vertex_shader::cb_list VSCBuffers = {}, typename vertex_shader::sr_list VSResources = {},
                  typename geometry_shader::cb_list GSCBuffers = {}, typename geometry_shader::sr_list GSResources = {},
                  typename pixel_shader::cb_list PSCBuffers = {}, typename pixel_shader::sr_list PSResources = {}) const {
    vs(Model.vertex_buffer, Model.index_buffer, VSCBuffers, VSResources);
    gs(GSCBuffers, GSResources), ps(PSCBuffers, PSResources);
    if (Model.index_buffer) main::system::d3d_context->DrawIndexed(Model.index_buffer.count, 0, 0);
    else main::system::d3d_context->Draw(Model.vertex_buffer.count, 0);
  }
};

template<typename VShader, typename PShader> class pipeline<VShader, PShader, none> {
  static_assert(variation_of<yw::vertex_shader<topology{}, void, void>, VShader>);
  static_assert(variation_of<yw::pixel_shader<void>, PShader>);
  static_assert(same_as<typename VShader::output, typename PShader::input>);
public:
  using vertex_shader = VShader;
  using geometry_shader = none;
  using pixel_shader = PShader;
  vertex_shader vs;
  pixel_shader ps;
  pipeline() noexcept = default;
  pipeline(vertex_shader Vs, pixel_shader Ps) : vs(mv(Vs)), ps(mv(Ps)) {}
  pipeline(stv1 Hlsl) : vs(Hlsl), ps(Hlsl) {}
  explicit operator bool() const noexcept { return bool(vs) && bool(ps); }
  void operator()(const model<typename vertex_shader::input>& Model,
                  typename vertex_shader::cb_list VSCBuffers = {}, typename vertex_shader::sr_list VSResources = {},
                  typename pixel_shader::cb_list PSCBuffers = {}, typename pixel_shader::sr_list PSResources = {}) const {
    vs(Model.vertex_buffer, Model.index_buffer, VSCBuffers, VSResources), ps(PSCBuffers, PSResources);
    if (Model.index_buffer) main::system::d3d_context->DrawIndexed(Model.index_buffer.size(), 0, 0);
    else main::system::d3d_context->Draw(Model.vertex_buffer.size(), 0);
  }
};

template<typename VS, typename GS, typename PS> pipeline(VS, GS, PS) -> pipeline<VS, PS, GS>;
template<typename VS, typename PS> pipeline(VS, PS) -> pipeline<VS, PS, none>;

}

namespace std {
template<> struct tuple_size<yw::rect> : integral_constant<size_t, 4> {};
template<size_t I> requires(I < 4) struct tuple_element<I, yw::rect> : type_identity<int> {};
template<> struct formatter<yw::rect> : formatter<string> {
  auto format(const yw::rect& A, std::format_context& Ctx) const {
    return formatter<string>::format(std::format("[left={},top={},right={},bottom={}]", A.left, A.top, A.right, A.bottom), Ctx);
  }
};
}

namespace yw {
}
