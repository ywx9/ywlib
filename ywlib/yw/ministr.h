#pragma once
#include "yw/core.h"

namespace yw {

/// mini string class
class ministr {
  static constexpr size_t max_length = std::numeric_limits<uint16_t>::max() - 2;

  wchar_t* _p = nullptr;

  static wchar_t* allocate(wchar_t Size) {
    const auto p = new wchar_t[Size + 2];
    p[0] = Size;
    p[Size + 1] = L'\0';
    return p;
  }

public:
  ~ministr() noexcept { delete[] _p; }
  ministr() noexcept = default;
  operator std::wstring_view() const { return {data(), size()}; }

  bool empty() const noexcept { return size() == 0; }
  wchar_t size() const { return _p ? _p[0] : 0; }
  wchar_t* begin() { return _p + bool(_p); }
  const wchar_t* begin() const { return _p + bool(_p); }
  wchar_t* end() { return _p ? _p + 1 + _p[0] : nullptr; }
  const wchar_t* end() const { return _p ? _p + 1 + _p[0] : nullptr; }
  wchar_t* data() { return _p + bool(_p); }
  const wchar_t* data() const { return _p + bool(_p); }
  wchar_t& operator[](size_t Index) { return _p[Index + 1]; }
  const wchar_t& operator[](size_t Index) const { return _p[Index + 1]; }

  ministr(size_t Size) {
    if (Size == 0 || Size > max_length) return;
    _p = allocate(static_cast<wchar_t>(Size));
  }
  ministr(ministr&& Other) noexcept : _p(std::exchange(Other._p, nullptr)) {}

  ministr(const ministr& Other) {
    if (!Other._p) return;
    const auto size = Other._p[0];
    _p = allocate(size);
    std::memcpy(_p + 1, Other._p + 1, size * sizeof(wchar_t));
  }

  ministr& operator=(const ministr& Other) {
    if (this == &Other) return *this;
    delete[] _p;
    if (Other._p) {
      const auto size = Other._p[0];
      _p = allocate(size);
      std::memcpy(_p + 1, Other._p + 1, size * sizeof(wchar_t));
    } else _p = nullptr;
    return *this;
  }

  ministr& operator=(ministr&& Other) noexcept {
    if (this == &Other) return *this;
    delete[] _p;
    _p = std::exchange(Other._p, nullptr);
    return *this;
  }

  template<stringable<wchar_t> S> requires different_from<remove_cvref<S>, ministr> ministr(S&& Str) {
    const auto sv = std::wstring_view(Str);
    const auto size = sv.size();
    if (size == 0 || size > max_length) return;
    _p = allocate(static_cast<wchar_t>(size));
    if (_p) std::memcpy(_p + 1, sv.data(), size * sizeof(wchar_t));
  }
};
}
