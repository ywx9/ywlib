/// @file ywlib.h

#pragma once
#ifndef ywlib
#define ywlib

#include <array>
#include <compare>
#include <concepts>
#include <filesystem>
#include <iostream>
#include <source_location>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

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


namespace yw {


/// @brief structural type for null value
struct none {
  constexpr none() noexcept = default;
  constexpr none(auto&&...) noexcept {}
  constexpr none& operator=(auto&&) noexcept { return *this; }
  explicit constexpr operator bool() const noexcept { return false; }
  constexpr none operator()(auto&&...) const noexcept { return {}; }
  friend constexpr bool operator==(none, none) noexcept { return false; }
  friend constexpr auto operator<=>(none, none) noexcept { return std::partial_ordering::unordered; }
  friend constexpr none operator+(none) noexcept { return {}; }
  friend constexpr none operator-(none) noexcept { return {}; }
  friend constexpr none operator+(none, none) noexcept { return {}; }
  friend constexpr none operator-(none, none) noexcept { return {}; }
  friend constexpr none operator*(none, none) noexcept { return {}; }
  friend constexpr none operator/(none, none) noexcept { return {}; }
  friend constexpr none& operator+=(none& a, none) noexcept { return a; }
  friend constexpr none& operator-=(none& a, none) noexcept { return a; }
  friend constexpr none& operator*=(none& a, none) noexcept { return a; }
  friend constexpr none& operator/=(none& a, none) noexcept { return a; }
};


/// @brief structural type for value
struct value {
  double _{};
  constexpr value() noexcept = default;
  constexpr value(signed char v) noexcept : _(v) {}
  constexpr value(unsigned char v) noexcept : _(v) {}
  constexpr value(signed short v) noexcept : _(v) {}
  constexpr value(unsigned short v) noexcept : _(v) {}
  constexpr value(signed int v) noexcept : _(v) {}
  constexpr value(unsigned int v) noexcept : _(v) {}
  constexpr value(signed long v) noexcept : _(v) {}
  constexpr value(unsigned long v) noexcept : _(v) {}
  constexpr value(signed long long v) noexcept : _(double(v)) {}
  constexpr value(unsigned long long v) noexcept : _(double(v)) {}
  constexpr value(float v) noexcept : _(v) {}
  constexpr value(double v) noexcept : _(v) {}
  constexpr value(long double v) noexcept : _(double(v)) {}
  constexpr operator double() const noexcept { return _; }
  template<typename T> explicit constexpr operator T() const noexcept(noexcept(static_cast<T>(_))) requires requires { static_cast<T>(_); } { return static_cast<T>(_); }
  template<typename Ct, typename Tr> friend auto& operator<<(std::basic_ostream<Ct, Tr>& os, const value v) { return os << v._; }
  template<typename Ct, typename Tr> friend auto& operator>>(std::basic_istream<Ct, Tr>& is, value& v) { return is >> v._; }
};


/// helper structure for `max` and `min`
template<bool Max> struct t_max {
  constexpr none operator()() const noexcept { return {}; }
  template<typename T> constexpr T operator()(T&& t) const noexcept { return t; }
  template<typename T0, typename T1, typename... Ts> constexpr auto operator()(T0&& t0, T1&& t1, Ts&&... ts) const
    -> std::common_type_t<T0, T1, Ts...> requires std::totally_ordered<std::common_type_t<T0, T1, Ts...>> {
    if constexpr (Max) return (*this)(t0 < t1 ? t1 : t0, static_cast<Ts&&>(ts)...);
    else return (*this)(t1 < t0 ? t1 : t0, static_cast<Ts&&>(ts)...);
  }
  template<typename Pd, typename T0, typename T1, typename... Ts>
  constexpr auto operator()(Pd Less, T0&& t0, T1&& t1, Ts&&... ts) const -> std::common_type_t<T0, T1, Ts...>
    requires std::predicate<Pd, std::common_type_t<T0, T1, Ts...>, std::common_type_t<T0, T1, Ts...>> {
    if constexpr (Max) return (*this)(Less(t0, t1) ? t1 : t0, static_cast<Ts&&>(ts)...);
    else return (*this)(Less(t1, t0) ? t1 : t0, static_cast<Ts&&>(ts)...);
  }
};

/// returns the maximum object
/// @note `max()` returns `none`
/// @note `max(A)` returns `A`
/// @note `max(A, B, C...)` returns the maximum of `A`, `B`, `C...`
/// @note `max(Less, A, B, C)` returns the maximum of `A`, `B`, `C` using `Less` as the comparison function
inline constexpr t_max<1> max;

/// returns the minimum object
/// @note `min()` returns `none`
/// @note `min(A)` returns `A`
/// @note `min(A, B, C...)` returns the minimum of `A`, `B`, `C...`
/// @note `min(Less, A, B, C)` returns the minimum of `A`, `B`, `C` using `Less` as the comparison function
inline constexpr t_max<0> min;


/// converts the multi-byte string to a wide character string
inline constexpr auto to_wcs(const std::string_view s, const int CodePage = 932) noexcept -> std::wstring {
  if (s.empty()) return {};
  std::wstring result(MultiByteToWideChar(CodePage, 0, s.data(), int(s.size()), nullptr, 0), L'\0');
  MultiByteToWideChar(CodePage, 0, s.data(), int(s.size()), result.data(), int(result.size()));
  return result;
};

/// converts the wide character string to a multi-byte string
inline constexpr auto to_mbs(const std::wstring_view s, const int CodePage = 932) noexcept -> std::string {
  if (s.empty()) return {};
  std::string result(WideCharToMultiByte(CodePage, 0, s.data(), int(s.size()), nullptr, 0, nullptr, nullptr), '\0');
  WideCharToMultiByte(CodePage, 0, s.data(), int(s.size()), result.data(), int(result.size()), nullptr, nullptr);
  return result;
};

/// reinterprets the wide character string view as a utf-16 string view
constexpr auto to_utf16(const std::wstring_view s) noexcept
  -> std::u16string_view { return {reinterpret_cast<const char16_t*>(s.data()), s.size()}; }

/// reinterprets the utf-32 string view as a wide character string view
constexpr auto to_wcs(const std::u16string_view s) noexcept
  -> std::wstring_view { return {reinterpret_cast<const wchar_t*>(s.data()), s.size()}; }

/// reinterprets the wide character string as a utf-16 string
constexpr auto to_utf16(std::wstring&& s) noexcept
  -> std::u16string { return {reinterpret_cast<std::u16string&&>(std::move(s))}; }

/// reinterprets the utf-16 string as a wide character string
constexpr auto to_wcs(std::u16string&& s) noexcept
  -> std::wstring { return {reinterpret_cast<std::wstring&&>(std::move(s))}; }

/// converts the utf-8 string to a utf-16 string
inline constexpr auto to_utf16(const std::u8string_view s) noexcept -> std::u16string {
  if (s.empty()) return {};
  std::u16string o(s.size(), u'\0');
  char32_t c;
  for (size_t i{}, j{};;) {
    if ((s[i] & 0x80) == 0) c = s[i++];
    else if ((s[i] & 0xE0) == 0xC0) c = ((s[i++] & 0x1F) << 6) | (s[i++] & 0x3F);
    else if ((s[i] & 0xF0) == 0xE0) c = ((s[i++] & 0x0F) << 12) | ((s[i++] & 0x3F) << 6) | (s[i++] & 0x3F);
    else if ((s[i] & 0xF8) == 0xF0) c = ((s[i++] & 0x07) << 18) | ((s[i++] & 0x3F) << 12) | ((s[i++] & 0x3F) << 6) | (s[i++] & 0x3F);
    else return o.resize(j), std::move(o);
    if (c < 0x10000) o[j++] = char16_t(c);
    else if (c < 0x110000) o[j++] = char16_t(0xD800 | ((c - 0x10000) >> 10)), o[j++] = char16_t(0xDC00 | ((c - 0x10000) & 0x3FF));
    else return o.resize(j), std::move(o);
    if (i >= s.size()) return o.resize(j), std::move(o);
  }
  return {};
}

/// converts the utf-8 string to a utf-32 string
inline constexpr auto to_utf32(const std::u8string_view s) noexcept -> std::u32string {
  if (s.empty()) return {};
  std::u32string o(s.size(), U'\0');
  for (size_t i{}, j{};;) {
    if ((s[i] & 0x80) == 0) o[j++] = s[i++];
    else if ((s[i] & 0xE0) == 0xC0) o[j++] = ((s[i++] & 0x1F) << 6) | (s[i++] & 0x3F);
    else if ((s[i] & 0xF0) == 0xE0) o[j++] = ((s[i++] & 0x0F) << 12) | ((s[i++] & 0x3F) << 6) | (s[i++] & 0x3F);
    else if ((s[i] & 0xF8) == 0xF0) o[j++] = ((s[i++] & 0x07) << 18) | ((s[i++] & 0x3F) << 12) | ((s[i++] & 0x3F) << 6) | (s[i++] & 0x3F);
    else return o.resize(j), std::move(o);
    if (i >= s.size()) return o.resize(j), std::move(o);
  }
  return {};
}

/// converts the utf-16 string to a utf-8 string
inline constexpr auto to_utf8(const std::u16string_view s) noexcept -> std::u8string {
  if (s.empty()) return {};
  std::u8string o(s.size() * 4, u8'\0');
  char32_t c;
  for (size_t i{}, j{};;) {
    if (s[i] < 0xD800 || s[i] > 0xDFFF) c = s[i++];
    else if (s[i] < 0xDC00) c = (s[i++] & 0x3FF) << 10 | (s[i++] & 0x3FF) | 0x10000;
    else return o.resize(j), std::move(o);
    if (c < 0x80) o[j++] = char8_t(c);
    else if (c < 0x800) o[j++] = char8_t(0xC0 | (c >> 6)), char8_t(o[j++] = 0x80 | (c & 0x3F));
    else if (c < 0x10000) o[j++] = char8_t(0xE0 | (c >> 12)), o[j++] = char8_t(0x80 | ((c >> 6) & 0x3F)),
                          o[j++] = char8_t(0x80 | (c & 0x3F));
    else if (c < 0x110000) o[j++] = char8_t(0xF0 | (c >> 18)), o[j++] = char8_t(0x80 | ((c >> 12) & 0x3F)),
                           o[j++] = char8_t(0x80 | ((c >> 6) & 0x3F)), o[j++] = char8_t(0x80 | (c & 0x3F));
    else return o.resize(j), std::move(o);
    if (i >= s.size()) return o.resize(j), std::move(o);
  }
  return {};
}

/// converts the utf-16 string to a utf-32 string
inline constexpr auto to_utf32(const std::u16string_view s) noexcept -> std::u32string {
  if (s.empty()) return {};
  std::u32string o(s.size(), U'\0');
  for (size_t i{}, j{};;) {
    if (s[i] < 0xD800 || s[i] > 0xDFFF) o[j++] = s[i++];
    else if (s[i] < 0xDC00) o[j++] = (s[i++] & 0x3FF) << 10 | (s[i++] & 0x3FF) | 0x10000;
    else return o.resize(j), std::move(o);
    if (i >= s.size()) return o.resize(j), std::move(o);
  }
  return {};
}

/// converts the utf-32 string to a utf-8 string
inline constexpr auto to_utf8(const std::u32string_view s) noexcept -> std::u8string {
  if (s.empty()) return {};
  std::u8string o(s.size() * 4, u8'\0');
  for (size_t i{}, j{};;) {
    if (s[i] < 0x80) o[j++] = char8_t(s[i++]);
    else if (s[i] < 0x800) o[j++] = char8_t(0xC0 | (s[i] >> 6)), o[j++] = char8_t(0x80 | (s[i] & 0x3F)), i++;
    else if (s[i] < 0x10000) o[j++] = char8_t(0xE0 | (s[i] >> 12)), o[j++] = char8_t(0x80 | ((s[i] >> 6) & 0x3F)),
                             o[j++] = char8_t(0x80 | (s[i] & 0x3F)), i++;
    else if (s[i] < 0x110000) o[j++] = char8_t(0xF0 | (s[i] >> 18)), o[j++] = char8_t(0x80 | ((s[i] >> 12) & 0x3F)),
                              o[j++] = char8_t(0x80 | ((s[i] >> 6) & 0x3F)), o[j++] = char8_t(0x80 | (s[i] & 0x3F)), i++;
    else return o.resize(j), std::move(o);
    if (i >= s.size()) return o.resize(j), std::move(o);
  }
  return {};
}

/// converts the utf-32 string to a utf-16 string
inline constexpr auto to_utf16(const std::u32string_view s) noexcept -> std::u16string {
  if (s.empty()) return {};
  std::u16string o(s.size(), u'\0');
  for (size_t i{}, j{};;) {
    if (s[i] < 0x10000) o[j++] = char16_t(s[i++]);
    else if (s[i] < 0x110000) o[j++] = char16_t(0xD800 | ((s[i] - 0x10000) >> 10)),
                              o[j++] = char16_t(0xDC00 | ((s[i] - 0x10000) & 0x3FF)), i++;
    else return o.resize(j), std::move(o);
    if (i >= s.size()) return o.resize(j), std::move(o);
  }
  return {};
}


template<typename T> using remove_cv = std::remove_cv_t<T>;

template<typename T, typename... Ts> concept same_as = (std::same_as<T, Ts> && ...);
template<typename T, typename... Ts> concept included_in = (std::same_as<T, Ts> || ...);

template<typename T> concept character = included_in<remove_cv<T>, char, wchar_t, char8_t, char16_t, char32_t>;


/// static string
template<character Ct, size_t N> struct phrase {
  Ct _[N + 1]{};
  using value_type = Ct;
  static constexpr unsigned count = unsigned(N);
  constexpr phrase(const phrase& Other) { std::ranges::copy_n(Other._, count, _); }
  constexpr phrase(const Ct (&s)[N + 1]) { std::ranges::copy_n(s, count, _); }
  constexpr operator std::basic_string_view<Ct>() const noexcept { return {_, count}; }
  constexpr const Ct* data() const noexcept { return _; }
  template<typename Tr> friend std::basic_ostream<Ct, Tr>& operator<<(std::basic_ostream<Ct, Tr>& os, const phrase& Phrase) { return os << Phrase._; }
};

/// short string
template<character Ct> struct phrase<Ct, 0> {
  Ct _[320]{}; // 320 is the maximum length of a short string
  const unsigned count{};
  using value_type = Ct;
  constexpr phrase(const phrase& Other) : count(Other.count) { std::ranges::copy_n(Other._, count, _); }
  template<unsigned N> constexpr phrase(const Ct (&Str)[N]) : count(min(N - 1, 255)) { std::ranges::copy_n(Str, count, _); }
  constexpr phrase(const Ct* Str, size_t N) : count(unsigned(min(N, 255))) { std::ranges::copy_n(Str, count, _); }
  constexpr phrase(const Ct* Str) : count(unsigned(std::char_traits<Ct>::length(Str))) { std::ranges::copy_n(Str, count, _); }
  constexpr operator std::basic_string_view<Ct>() const noexcept { return {_, count}; }
  constexpr const Ct* data() const noexcept { return _; }
  template<typename Tr> friend std::basic_ostream<Ct, Tr>& operator<<(std::basic_ostream<Ct, Tr>& os, const phrase& Phrase) { return os << Phrase._; }
};

template<character Ct, size_t N> phrase(const Ct (&)[N]) -> phrase<Ct, N - 1>;
template<character Ct> phrase(const Ct*) -> phrase<Ct, 0>;
template<character Ct> phrase(const Ct*, size_t) -> phrase<Ct, 0>;


/// source location
/// @note `std::cout << source()` prints `file(line,column): function: `
class source {
  std::string text;
public:
  static constexpr const char* file(const char* const _ = __builtin_FILE()) noexcept { return _; }
  static constexpr const char* function(const char* const _ = __builtin_FUNCTION()) noexcept { return _; }
  static constexpr int line(const int _ = __builtin_LINE()) noexcept { return _; }
  static constexpr int column(const int _ = __builtin_COLUMN()) noexcept { return _; }
  source(const char* const File = file(), const char* const Function = function(),
         const int Line = line(), const int Column = column()) noexcept {
    auto sline = std::to_string(Line), scolumn = std::to_string(Column);
    auto nfile = std::char_traits<char>::length(File), nfunc = std::char_traits<char>::length(Function);
    text.resize(nfile + 1 + sline.size() + 1 + scolumn.size() + 3 + nfunc + 2);
    auto p = text.data();
    std::ranges::copy_n(File, nfile, p), p += nfile;
    *p++ = '(', std::ranges::copy_n(sline.data(), sline.size(), p), p += sline.size();
    *p++ = ',', std::ranges::copy_n(scolumn.data(), scolumn.size(), p), p += scolumn.size();
    *p++ = ')', *p++ = ':', *p++ = ' ', std::ranges::copy_n(Function, nfunc, p), p += nfunc;
    *p++ = ':', *p++ = ' ', *p = '\0';
  }
  constexpr operator std::string_view() const noexcept { return text; }
  template<typename Ct, typename Tr> friend auto& operator<<(
    std::basic_ostream<Ct, Tr>& os, const source& Source) { return os << Source.text; }
};


/// exception with source location
class except : public std::exception {
public:
  except(const std::exception& Std, const source& Source = {})
    : std::exception(std::format("{}\n -> {}", Std.what(), std::string_view(Source)).data()) {}
  except(const std::string_view Message, const source& Source = {})
    : std::exception(std::format("{}\n -> {}", Message, std::string_view(Source)).data()) {}
};


/// deleter for COM objects
template<typename Com> struct com_deleter {
  constexpr com_deleter() noexcept = default;
  template<typename U> constexpr com_deleter(const com_deleter<U>& Other) noexcept {}
  void operator()(Com* Ptr) const { Ptr ? Ptr->Release() : none{}; }
};


#ifdef ywlib_debug
/// checks if the application is in debug mode
inline constexpr bool debug = true;
/// call `system::fatalerror` if `Check` is `false` (only in debug mode)
#define ywlib_assert(Check, Message) yw::system::fatalerror(Check, Message)
#else
/// checks if the application is in debug mode
inline constexpr bool debug = false;
/// call `system::fatalerror` if `Check` is `false` (only in debug mode)
#define ywlib_assert(Check, Message) void()
#endif


/// com pointer
template<typename Com> using comptr = std::unique_ptr<Com, com_deleter<Com>>;


/// obtains the address of com pointer
template<typename Com> constexpr Com** com_init(comptr<Com>& Ptr) {
  ywlib_assert(!Ptr, L"com_init: com pointer is already initialized");
  return reinterpret_cast<Com**>(&Ptr);
}


namespace main::system {
inline double stopwatch_freq{}; // for `stopwatch`
}


/// stopwatch
class stopwatch {
  inline static const double& freq = main::system::stopwatch_freq;
  mutable LARGE_INTEGER _li;
  long long _last;
public:
  stopwatch() noexcept : _li{}, _last{} { QueryPerformanceCounter(&_li), _last = _li.QuadPart; }
  void reset() noexcept { QueryPerformanceCounter(&_li), _last = _li.QuadPart; }
  double read() const noexcept { return QueryPerformanceCounter(&_li), (_li.QuadPart - _last) / freq; }
  double read_reset() noexcept { return QueryPerformanceCounter(&_li), (_li.QuadPart - std::exchange(_last, _li.QuadPart)) / freq; }
};


namespace main {
namespace system {
inline stopwatch timer{};
inline std::wstring username{};
inline std::vector<std::wstring> args{};
inline HINSTANCE hinstance{};
inline HFONT hfont{};
inline HWND hwnd{};
inline int padding_w{}, padding_h{};
inline comptr<ID3D11Device1> d3d_device{};
inline comptr<ID3D11DeviceContext1> d3d_context{};
inline comptr<IDXGISwapChain1> swap_chain{};
inline comptr<ID2D1Device5> d2d_device{};
inline comptr<ID2D1DeviceContext5> d2d_context{};
inline comptr<ID2D1Factory6> d2d_factory{};
inline comptr<IDWriteFactory7> dw_factory{};
inline comptr<IWICImagingFactory2> wic_factory{};
inline void initialize(value Width, value Height);
LRESULT CALLBACK wndproc(HWND hw, UINT msg, WPARAM wp, LPARAM lp);
template<character Ct, size_t N> inline void fatalerror(bool Check, const phrase<Ct, N> Message);
}

inline const auto& timer = system::timer;             // system timer
inline const auto& username = system::username;       // user name
inline const auto& args = system::args;               // arguments
inline const auto& hinstance = system::hinstance;     // instance handle
inline const auto& hwnd = system::hwnd;               // window handle
inline const auto& d3d_device = system::d3d_device;   // Direct3D device
inline const auto& d3d_context = system::d3d_context; // Direct3D context
inline const auto& swap_chain = system::swap_chain;   // DXGI swap chain
inline const auto& d2d_device = system::d2d_device;   // Direct2D device
inline const auto& d2d_context = system::d2d_context; // Direct2D context
inline const auto& dw_factory = system::dw_factory;   // DirectWrite factory
inline const auto& wic_factory = system::wic_factory; // WIC factory

inline void terminate();
inline void resize(value Width, value Height);
}


/// bitmap object
class bitmap {
  bitmap(ID2D1Bitmap1* Ptr, const value Width, const value Height) noexcept
    : d2d_bitmap(Ptr), width(unsigned(Width)), height(unsigned(Height)) {}
  friend void ::yw::main::resize(value Width, value Height);
protected:
  comptr<ID2D1Bitmap1> d2d_bitmap{};
public:
  const unsigned width{};  // width of the bitmap
  const unsigned height{}; // height of the bitmap
  bitmap() noexcept = default;
  bitmap(bitmap&& Other) noexcept : d2d_bitmap(std::move(Other.d2d_bitmap)), width(Other.width), height(Other.height) {}
  bitmap(const value Width, const value Height) : width(unsigned(Width)), height(unsigned(Height)) {
    auto p = D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET,
                                     D2D1::PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));
    auto hr = main::d2d_context->CreateBitmap({width, height}, nullptr, 0, p, com_init(d2d_bitmap));
    ywlib_assert(SUCCEEDED(hr), L"bitmap::bitmap: CreateBitmap failed");
  }
  bitmap(const std::filesystem::path& Path) {
    comptr<IWICBitmapDecoder> decoder;
    auto hr = main::wic_factory->CreateDecoderFromFilename(
      Path.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, com_init(decoder));
    ywlib_assert(SUCCEEDED(hr), L"bitmap::bitmap: CreateDecoderFromFilename failed");
    comptr<IWICBitmapFrameDecode> frame;
    hr = decoder->GetFrame(0, com_init(frame));
    ywlib_assert(SUCCEEDED(hr), L"bitmap::bitmap: GetFrame failed");
    hr = frame->GetSize(const_cast<unsigned*>(&width), const_cast<unsigned*>(&height));
    ywlib_assert(SUCCEEDED(hr), L"bitmap::bitmap: GetSize failed");
    WICPixelFormatGUID format, target = GUID_WICPixelFormat32bppPRGBA;
    hr = frame->GetPixelFormat(&format);
    ywlib_assert(SUCCEEDED(hr), L"bitmap::bitmap: GetPixelFormat failed");
    comptr<IWICFormatConverter> converter;
    hr = main::wic_factory->CreateFormatConverter(com_init(converter));
    ywlib_assert(SUCCEEDED(hr), L"bitmap::bitmap: CreateFormatConverter failed");
    BOOL b = [&](BOOL b) { hr = converter->CanConvert(format, target, &b); return b; }({});
    ywlib_assert(b, L"bitmap::bitmap: CanConvert failed");
    ywlib_assert(SUCCEEDED(hr), L"bitmap::bitmap: CanConvert failed");
    hr = converter->Initialize(frame.get(), target, WICBitmapDitherTypeNone, nullptr, 0, WICBitmapPaletteTypeCustom);
    ywlib_assert(SUCCEEDED(hr), L"bitmap::bitmap: Initialize failed");
    hr = main::d2d_context->CreateBitmapFromWicBitmap(converter.get(), com_init(d2d_bitmap));
    ywlib_assert(SUCCEEDED(hr), L"bitmap::bitmap: CreateBitmapFromWicBitmap failed");
  }
  bitmap& operator=(bitmap&& Other) noexcept {
    d2d_bitmap = std::move(Other.d2d_bitmap);
    const_cast<unsigned&>(width) = Other.width;
    const_cast<unsigned&>(height) = Other.height;
    return *this;
  }
  explicit operator bool() const noexcept { return bool(d2d_bitmap); }
  operator ID2D1Bitmap1*() const noexcept { return d2d_bitmap.get(); }
};


/// initializes the system
inline void main::system::initialize(value Width, value Height) {
  try {
    // Starts timer
    system::stopwatch_freq = [](LARGE_INTEGER li) { return ::QueryPerformanceFrequency(&li), double(li.QuadPart); }({});
    main::system::timer.reset();
    // Sets locale
    std::wcout.imbue(std::locale("Japanese"));
    // Initializes COM
    auto hr = ::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    ywlib_assert(SUCCEEDED(hr), L"main::system::initialize: CoInitializeEx failed");
    // Obtains user name
    [](DWORD d) { ::GetUserNameW(nullptr, &d), username.resize(d - 1), ::GetUserNameW(username.data(), &d); }({});
    // Obtains the arguments
    [](int c) { auto a = ::CommandLineToArgvW(::GetCommandLineW(), &c);
                for (args.resize(c); c--;) args[c] = a[c]; LocalFree(a); }({});
    // Obtains instance handle
    hinstance = ::GetModuleHandleW(nullptr);
    // Creates font for WIN32 controls
    hfont = ::CreateFontW(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS,
                          CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"Yu Gothic UI");
    ywlib_assert(hfont, L"main::system::initialize: CreateFontW failed");
    // Registers window class
    static constexpr wchar_t classname[] = L"ywlib";
    WNDCLASSEXW wc{sizeof(WNDCLASSEXW), CS_OWNDC, wndproc, 0, 0, hinstance};
    wc.hCursor = ::LoadCursorW(nullptr, IDC_ARROW), wc.lpszClassName = classname;
    ywlib_assert(::RegisterClassExW(&wc), L"main::system::initialize: RegisterClassExW failed");
    // Creates window
    hwnd = ::CreateWindowExW(WS_EX_ACCEPTFILES, classname, classname, WS_OVERLAPPED | WS_SYSMENU | WS_CLIPCHILDREN,
                             CW_USEDEFAULT, CW_USEDEFAULT, 100, 100, nullptr, nullptr, hinstance, nullptr);
    ywlib_assert(hwnd, L"main::system::initialize: CreateWindowExW failed");
    // Obtains the padding size of the window and sets the window at left-top corner
    [](RECT r) { ::GetClientRect(hwnd, &r), padding_w = 100 - r.right, padding_h = 100 - r.bottom; }({});
    ::SetWindowPos(hwnd, 0, -padding_w / 2, 0, int(Width + padding_w), int(Height + padding_h), 0);
    // Initializes Direct3D
    constexpr D3D_FEATURE_LEVEL levels[] = {D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0};
    hr = ::D3D11CreateDevice(0, D3D_DRIVER_TYPE_HARDWARE, 0, D3D11_CREATE_DEVICE_BGRA_SUPPORT,
                             levels, 2, D3D11_SDK_VERSION,
                             (ID3D11Device**)com_init(d3d_device), 0, (ID3D11DeviceContext**)com_init(d3d_context));
    ywlib_assert(SUCCEEDED(hr), L"main::system::initialize: D3D11CreateDevice failed");
    // Initializes DXGI
    DXGI_SWAP_CHAIN_DESC1 scd{};
    scd.Width = UINT(Width), scd.Height = UINT(Height);
    scd.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
    scd.SampleDesc.Count = 1;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.BufferCount = 2;
    scd.Scaling = DXGI_SCALING_STRETCH;
    scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    comptr<IDXGIFactory2> dxgi_factory;
    hr = ::CreateDXGIFactory1(IID_PPV_ARGS(com_init(dxgi_factory)));
    ywlib_assert(SUCCEEDED(hr), L"main::system::initialize: CreateDXGIFactory1 failed");
    hr = dxgi_factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);
    ywlib_assert(SUCCEEDED(hr), L"main::system::initialize: MakeWindowAssociation failed");
    hr = dxgi_factory->CreateSwapChainForHwnd(d3d_device.get(), hwnd, &scd, nullptr, nullptr, com_init(swap_chain));
    ywlib_assert(SUCCEEDED(hr), L"main::system::initialize: CreateSwapChainForHwnd failed");
    // Initializes Direct2D
    hr = ::D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, IID_PPV_ARGS(com_init(d2d_factory)));
    ywlib_assert(SUCCEEDED(hr), L"main::system::initialize: D2D1CreateFactory failed");
    comptr<IDXGIDevice3> dxgi_device;
    hr = d3d_device->QueryInterface(IID_PPV_ARGS(com_init(dxgi_device)));
    ywlib_assert(SUCCEEDED(hr), L"main::system::initialize: QueryInterface failed");
    hr = d2d_factory->CreateDevice(dxgi_device.get(), com_init(d2d_device));
    ywlib_assert(SUCCEEDED(hr), L"main::system::initialize: CreateDevice failed");
    hr = d2d_device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, com_init(d2d_context));
    ywlib_assert(SUCCEEDED(hr), L"main::system::initialize: CreateDeviceContext failed");
    // Initializes DirectWrite
    hr = ::DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory7), (IUnknown**)com_init(dw_factory));
    ywlib_assert(SUCCEEDED(hr), L"main::system::initialize: DWriteCreateFactory failed");
    // Initializes WIC
    hr = CoCreateInstance(CLSID_WICImagingFactory2, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(com_init(wic_factory)));
    ywlib_assert(SUCCEEDED(hr), L"main::system::initialize: CoCreateInstance failed");
    // Sets D3D blend state
    D3D11_BLEND_DESC1 bd{};
    bd.RenderTarget[0].BlendEnable = TRUE;
    bd.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    bd.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    bd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    bd.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    bd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    comptr<ID3D11BlendState1> blend_state;
    hr = d3d_device->CreateBlendState1(&bd, com_init(blend_state));
    ywlib_assert(SUCCEEDED(hr), L"main::system::initialize: CreateBlendState1 failed");
    float blend_factor[] = {0.0f, 0.0f, 0.0f, 0.0f};
    d3d_context->OMSetBlendState(blend_state.get(), blend_factor, 0xFFFFFFFF);
    // Sets D3D rasterizer state
    D3D11_RASTERIZER_DESC1 rd{};
    rd.FillMode = D3D11_FILL_SOLID;
    rd.CullMode = D3D11_CULL_NONE;
    comptr<ID3D11RasterizerState1> rasterizer_state;
    hr = d3d_device->CreateRasterizerState1(&rd, com_init(rasterizer_state));
    ywlib_assert(SUCCEEDED(hr), L"main::system::initialize: CreateRasterizerState1 failed");
    d3d_context->RSSetState(rasterizer_state.get());
    // Sets D3D Sampler state
    D3D11_SAMPLER_DESC sd{};
    sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sd.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sd.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sd.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sd.MinLOD = 0;
    sd.MaxLOD = D3D11_FLOAT32_MAX;
    comptr<ID3D11SamplerState> sampler_state;
    hr = d3d_device->CreateSamplerState(&sd, com_init(sampler_state));
    ywlib_assert(SUCCEEDED(hr), L"main::system::initialize: CreateSamplerState failed");
    [](auto p) { d3d_context->PSSetSamplers(0, 1, &p); }(sampler_state.get());
    // Shows the window
    resize(Width, Height), ::SetForegroundWindow(hwnd), ::SetFocus(hwnd);
  } catch (const std::exception& E) { throw except(E); }
}


/// terminates the system
inline void main::terminate() {
  try {
    system::wic_factory.reset();
    system::dw_factory.reset();
    system::d2d_context.reset();
    system::d2d_device.reset();
    system::d2d_factory.reset();
    system::swap_chain.reset();
    system::d3d_context.reset();
    system::d3d_device.reset();
    ::DestroyWindow(system::hwnd);
    ::UnregisterClassW(L"ywlib", system::hinstance);
    ::DeleteObject(system::hfont);
    ::CoUninitialize();
  } catch (const std::exception& E) { throw except(E); }
}


/// font object
template<value Size, phrase Name = L"Yu Gothic UI", bool Bold = false, bool Italic = false, int Align = 0> class font {
  inline static comptr<IDWriteTextFormat> text_format{};
public:
  explicit operator bool() const noexcept { return true; }
  operator const comptr<IDWriteTextFormat>&() const noexcept {
    if (!text_format) {
      ywlib_assert(main::dw_factory, L"Font::token: main::factory is not initialized");
      auto hr = main::dw_factory->CreateTextFormat(
        Name.data(), nullptr, Bold ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_NORMAL,
        Italic ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
        float(Size), L"", com_init(text_format));
      ywlib_assert(SUCCEEDED(hr), L"Font::token: CreateTextFormat failed");
      if (Align < 0) text_format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
      else if (Align > 0) text_format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
      else text_format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    }
    return text_format;
  }
};

}

namespace std { // clang-format off
template<typename Ct, size_t N> struct formatter<yw::phrase<Ct, N>, Ct> : formatter<basic_string_view<Ct>, Ct> {
  template<typename Context> auto format(const yw::phrase<Ct, N>& s, Context& ctx) {
    return formatter<basic_string_view<Ct>, Ct>::format(s, ctx); } };
} // clang-format on


#endif // !ywlib
