#pragma once

#include "2.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "user32.lib")

#include <shellapi.h>
#pragma comment(lib, "shell32.lib")

export namespace yw {

/// formats a Windows error message
inline string<cat1> format_windows_error(same_as<long> auto hr) {
  std::string buf(256, '\0');
  buf.resize(::FormatMessageA(0x1200, 0, hr, 0, buf.data(), buf.size(), 0));
  return buf;
}

/// null-terminated string
template<character Ct> class null_terminated {
  const char* _p{};
  nat _n{};
public:
  template<stringable<Ct> Str> null_terminated(Str&& s) {}
};

/// displays a message box with an OK button; always returns `true`
inline bool ok(format_string<cat1> Text, format_string<cat1> Caption = "OK?") { //
  return ::MessageBoxA(nullptr, Text.get().data(), Caption.get().data(), MB_OK) == IDOK;
}

/// displays a message box with an OK button; always returns `true`
inline bool ok(format_string<cat2> Text, format_string<cat2> Caption = L"OK?") { //
  return ::MessageBoxW(nullptr, Text.get().data(), Caption.get().data(), MB_OK) == IDOK;
}

/// displays a message box with an OK button; always returns `true`
inline bool ok(stringable auto&& Text, stringable auto&& Caption)    //
  requires (!(convertible_to<decltype(Text), format_string<cat1>> && //
              convertible_to<decltype(Caption), format_string<cat1>>) &&
            !(convertible_to<decltype(Text), format_string<cat2>> && //
              convertible_to<decltype(Caption), format_string<cat2>>)) {
  if constexpr (same_as<cat1, iter_value<decltype(Text)>, iter_value<decltype(Caption)>>) {
    string<cat1> s1(string_view<cat1>(Text)), s2(string_view<cat1>(Caption));
    return ::MessageBoxA(nullptr, s1.data(), s2.data(), MB_OK) == IDOK;
  } else {
    string<cat2> s1(cvt<cat2>(Text)), s2(cvt<cat2>(Caption));
    return ::MessageBoxW(nullptr, s1.data(), s2.data(), MB_OK) == IDOK;
  }
}

}
