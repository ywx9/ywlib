/// @file ywlib.h

#pragma once
#ifndef __ywlib__
#define __ywlib__

#include <array>
#include <compare>
#include <concepts>
#include <filesystem>
#include <iostream>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>

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
inline constexpr auto mbs_to_wcs(const std::string_view s, const int CodePage = 932) noexcept -> std::wstring {
  if (s.empty()) return {};
  std::wstring result(MultiByteToWideChar(CodePage, 0, s.data(), s.size(), nullptr, 0), L'\0');
  MultiByteToWideChar(CodePage, 0, s.data(), s.size(), result.data(), result.size());
  return result;
};

/// converts the wide character string to a multi-byte string
inline constexpr auto wcs_to_mbs(const std::wstring_view s, const int CodePage = 932) noexcept -> std::string {
  if (s.empty()) return {};
  std::string result(WideCharToMultiByte(CodePage, 0, s.data(), s.size(), nullptr, 0, nullptr, nullptr), '\0');
  WideCharToMultiByte(CodePage, 0, s.data(), s.size(), result.data(), result.size(), nullptr, nullptr);
  return result;
};

/// reinterprets the wide character string view as a utf-16 string view
constexpr auto wcs_to_utf16(const std::wstring_view s) noexcept
  -> std::u16string_view { return {reinterpret_cast<const char16_t*>(s.data()), s.size()}; }

/// reinterprets the utf-32 string view as a wide character string view
constexpr auto utf16_to_wcs(const std::u16string_view s) noexcept
  -> std::wstring_view { return {reinterpret_cast<const wchar_t*>(s.data()), s.size()}; }

/// reinterprets the wide character string as a utf-16 string
constexpr auto wcs_to_utf16(std::wstring&& s) noexcept
  -> std::u16string { return {reinterpret_cast<std::u16string&&>(std::move(s))}; }

/// reinterprets the utf-16 string as a wide character string
constexpr auto utf16_to_wcs(std::u16string&& s) noexcept
  -> std::wstring { return {reinterpret_cast<std::wstring&&>(std::move(s))}; }

/// converts the utf-8 string to a utf-16 string
inline constexpr auto utf8_to_utf16(const std::u8string_view s) noexcept -> std::u16string {
  if (s.empty()) return {};
  std::u16string o(s.size(), u'\0');
  char32_t c;
  for (size_t i{}, j{};;) {
    if ((s[i] & 0x80) == 0) c = s[i++];
    else if ((s[i] & 0xE0) == 0xC0) c = ((s[i++] & 0x1F) << 6) | (s[i++] & 0x3F);
    else if ((s[i] & 0xF0) == 0xE0) c = ((s[i++] & 0x0F) << 12) | ((s[i++] & 0x3F) << 6) | (s[i++] & 0x3F);
    else if ((s[i] & 0xF8) == 0xF0) c = ((s[i++] & 0x07) << 18) | ((s[i++] & 0x3F) << 12) | ((s[i++] & 0x3F) << 6) | (s[i++] & 0x3F);
    else return o.resize(j), std::move(o);
    if (c < 0x10000) o[j++] = c;
    else if (c < 0x110000) o[j++] = 0xD800 | ((c - 0x10000) >> 10), o[j++] = 0xDC00 | ((c - 0x10000) & 0x3FF);
    else return o.resize(j), std::move(o);
    if (i >= s.size()) return o.resize(j), std::move(o);
  }
  return {};
}

/// converts the utf-8 string to a utf-32 string
inline constexpr auto utf8_to_utf32(const std::u8string_view s) noexcept -> std::u32string {
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
inline constexpr auto utf16_to_utf8(const std::u16string_view s) noexcept -> std::u8string {
  if (s.empty()) return {};
  std::u8string o(s.size() * 4, u8'\0');
  char32_t c;
  for (size_t i{}, j{};;) {
    if (s[i] < 0xD800 || s[i] > 0xDFFF) c = s[i++];
    else if (s[i] < 0xDC00) c = (s[i++] & 0x3FF) << 10 | (s[i++] & 0x3FF) | 0x10000;
    else return o.resize(j), std::move(o);
    if (c < 0x80) o[j++] = c;
    else if (c < 0x800) o[j++] = 0xC0 | (c >> 6), o[j++] = 0x80 | (c & 0x3F);
    else if (c < 0x10000) o[j++] = 0xE0 | (c >> 12), o[j++] = 0x80 | ((c >> 6) & 0x3F), o[j++] = 0x80 | (c & 0x3F);
    else if (c < 0x110000) o[j++] = 0xF0 | (c >> 18), o[j++] = 0x80 | ((c >> 12) & 0x3F), o[j++] = 0x80 | ((c >> 6) & 0x3F), o[j++] = 0x80 | (c & 0x3F);
    else return o.resize(j), std::move(o);
    if (i >= s.size()) return o.resize(j), std::move(o);
  }
  return {};
}

/// converts the utf-16 string to a utf-32 string
inline constexpr auto utf16_to_utf32(const std::u16string_view s) noexcept -> std::u32string {
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
inline constexpr auto utf32_to_utf8(const std::u32string_view s) noexcept -> std::u8string {
  if (s.empty()) return {};
  std::u8string o(s.size() * 4, u8'\0');
  for (size_t i{}, j{};;) {
    if (s[i] < 0x80) o[j++] = s[i++];
    else if (s[i] < 0x800) o[j++] = 0xC0 | (s[i] >> 6), o[j++] = 0x80 | (s[i] & 0x3F), i++;
    else if (s[i] < 0x10000) o[j++] = 0xE0 | (s[i] >> 12), o[j++] = 0x80 | ((s[i] >> 6) & 0x3F), o[j++] = 0x80 | (s[i] & 0x3F), i++;
    else if (s[i] < 0x110000) o[j++] = 0xF0 | (s[i] >> 18), o[j++] = 0x80 | ((s[i] >> 12) & 0x3F), o[j++] = 0x80 | ((s[i] >> 6) & 0x3F), o[j++] = 0x80 | (s[i] & 0x3F), i++;
    else return o.resize(j), std::move(o);
    if (i >= s.size()) return o.resize(j), std::move(o);
  }
  return {};
}

/// converts the utf-32 string to a utf-16 string
inline constexpr auto utf32_to_utf16(const std::u32string_view s) noexcept -> std::u16string {
  if (s.empty()) return {};
  std::u16string o(s.size(), u'\0');
  for (size_t i{}, j{};;) {
    if (s[i] < 0x10000) o[j++] = s[i++];
    else if (s[i] < 0x110000) o[j++] = 0xD800 | ((s[i] - 0x10000) >> 10), o[j++] = 0xDC00 | ((s[i] - 0x10000) & 0x3FF), i++;
    else return o.resize(j), std::move(o);
    if (i >= s.size()) return o.resize(j), std::move(o);
  }
  return {};
}


template<typename T> using remove_cv = std::remove_cv_t<T>;

template<typename T, typename... Ts> concept same_as = (std::same_as<T, Ts> && ...);
template<typename T, typename... Ts> concept included_in = (std::same_as<T, Ts> || ...);

template<typename T> concept character = included_in<remove_cv<T>, char, wchar_t, char8_t, char16_t, char32_t>;


template<character Ct, size_t N> requires(0 < N && N < 256) struct phrase {
  using value_type = Ct;
  static constexpr size_t count = N - 1;
  consteval phrase(const Ct (&s)[N]) { std::ranges::copy_n(s, N, _); }
  constexpr operator std::basic_string_view<Ct>() const noexcept { return {_, count}; }
  const Ct _[N];
};
template<character Ct, size_t N> phrase(const Ct (&)[N]) -> phrase<Ct, N>;


/// deleter for COM objects
template<typename Com> struct com_deleter {
  constexpr com_deleter() noexcept = default;
  template<typename U> constexpr com_deleter(const com_deleter<U>& Other) noexcept {}
  void operator()(Com* Ptr) const { Ptr ? Ptr->Release() : none{}; }
};


/// com pointer
template<typename Com> using comptr = std::unique_ptr<Com, com_deleter<Com>>;


namespace main {
namespace system {
inline comptr<IDWriteFactory7> factory{};
inline void initialize(value Width, value Height);
}

inline const auto& factory = system::factory;
}


/// font object
class Font {
  comptr<IDWriteTextFormat> text_format{};
public:
  Font() noexcept = default;
  Font(const value Size, const string& Name, bool Bold, bool Italic) {
    auto hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory7), reinterpret_cast<IUnknown**>(&factory));
    if (SUCCEEDED(hr)) {
      hr = factory->CreateTextFormat(Name.c_str(), nullptr, Bold ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_NORMAL, Italic ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, Size, L"", &text_format);
    }
  }
};

namespace _ {
inline Font font{};
}
inline const Font& font = _::font;
}

namespace std { // clang-format off
template<typename Ct, size_t N> struct formatter<yw::phrase<Ct, N>, Ct> : formatter<basic_string_view<Ct>, Ct> {
  template<typename Context> auto format(const yw::phrase<Ct, N>& s, Context& ctx) {
    return formatter<basic_string_view<Ct>, Ct>::format(s, ctx); } };
} // clang-format on


#endif // !__ywlib__
