#pragma once
#include "ywx/bitmap.h"
#include "ywx/core.h"

namespace yw {

inline class {
  struct guard {
    bool opened = false;
    ~guard() {
      if (opened) ::CloseClipboard();
    }
  };

  static std::expected<bitmap, error_trace> bitmap_from_pixels(uint2 size, const uint8_t* pixels, uint32_t stride) {
    if (!pixels) return unexpected_error(errors::invalid_argument, "null pixel buffer");
    if (auto res = d2d.initialize(); !res) return unexpected_error(res.error());
    auto bmp = bitmap::create(size);
    if (!bmp) return unexpected_error(bmp.error());
    auto* p = static_cast<ID2D1Bitmap1*>(*bmp);
    if (!p) return unexpected_error(errors::not_initialized, "bitmap not initialized");
    auto hr = p->CopyFromMemory(nullptr, pixels, stride);
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CopyFromMemory failed", int32_t(hr));
    return std::move(*bmp);
  }

  static std::expected<std::vector<uint8_t>, error_trace> pixels_from_bitmap(
    const bitmap& b, uint2& size, uint32_t& stride) {
    if (!b) return unexpected_error(errors::not_initialized, "bitmap not initialized");
    if (auto res = d2d.initialize(); !res) return unexpected_error(res.error());

    size = b.size();
    const uint32_t width = size.x;
    const uint32_t height = size.y;
    if (width == 0 || height == 0) return unexpected_error(errors::invalid_argument, "invalid bitmap size");

    comptr<::ID2D1Bitmap1> cpu_bmp;
    D2D1_BITMAP_PROPERTIES1 props{
      bitmap::pixelformat,
      96.0f,
      96.0f,
      D2D1_BITMAP_OPTIONS(D2D1_BITMAP_OPTIONS_CPU_READ | D2D1_BITMAP_OPTIONS_CANNOT_DRAW),
      nullptr};
    auto hr = d2d.context()->CreateBitmap(D2D1_SIZE_U{width, height}, nullptr, 0, &props, &cpu_bmp.get());
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateBitmap (CPU_READ) failed", int32_t(hr));

    auto* src = static_cast<ID2D1Bitmap1*>(b);
    if (!src) return unexpected_error(errors::not_initialized, "bitmap not initialized");
    hr = cpu_bmp->CopyFromBitmap(nullptr, src, nullptr);
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CopyFromBitmap failed", int32_t(hr));

    D2D1_MAPPED_RECT mapped{};
    hr = cpu_bmp->Map(D2D1_MAP_OPTIONS_READ, &mapped);
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "Map failed", int32_t(hr));

    stride = width * 4;
    std::vector<uint8_t> pixels(stride * height);
    const uint8_t* src_row = mapped.bits;
    for (uint32_t y = 0; y < height; ++y) {
      std::memcpy(pixels.data() + y * stride, src_row, stride);
      src_row += mapped.pitch;
    }
    cpu_bmp->Unmap();
    return pixels;
  }

  std::expected<guard, error_trace> open() const {
    if (!::OpenClipboard(nullptr)) return unexpected_win32_error("OpenClipboard failed");
    return guard{true};
  }

  std::expected<void, error_trace> empty_clipboard() const {
    if (::EmptyClipboard()) return {};
    const auto err = ::GetLastError();
    if (err == ERROR_CLIPBOARD_NOT_OPEN) {
      if (!::OpenClipboard(::GetActiveWindow())) return unexpected_win32_error("OpenClipboard failed");
      if (!::EmptyClipboard()) return unexpected_win32_error("EmptyClipboard failed");
      return {};
    }
    return unexpected_error(errors::operation_failed, "EmptyClipboard failed", int32_t(err));
  }

  std::expected<HANDLE, error_trace> get_clipboard_data(UINT format) const {
    if (auto h = ::GetClipboardData(format)) return h;
    const auto err = ::GetLastError();
    if (err == ERROR_CLIPBOARD_NOT_OPEN) {
      if (!::OpenClipboard(::GetActiveWindow())) return unexpected_win32_error("OpenClipboard failed");
      if (auto h2 = ::GetClipboardData(format)) return h2;
      return unexpected_win32_error("GetClipboardData failed");
    }
    return unexpected_error(errors::operation_failed, "GetClipboardData failed", int32_t(err));
  }

public:
  std::expected<void, error_trace> clear() const {
    if (auto g = open()) {
      if (auto res = empty_clipboard(); !res) return unexpected_error(res.error());
      return {};
    } else return unexpected_error(g.error());
  }

  std::expected<bool, error_trace> has_text() const {
    if (auto g = open()) {
      return ::IsClipboardFormatAvailable(CF_UNICODETEXT) != 0;
    } else return unexpected_error(g.error());
  }

  std::expected<std::wstring, error_trace> text() const {
    if (auto g = open()) {
      if (!::IsClipboardFormatAvailable(CF_UNICODETEXT))
        return unexpected_error(errors::invalid_operation, "clipboard does not contain text");
      if (auto h = get_clipboard_data(CF_UNICODETEXT)) {
        if (auto p = static_cast<const wchar_t*>(::GlobalLock(*h))) {
          std::wstring s = p;
          ::GlobalUnlock(*h);
          return s;
        } else return unexpected_win32_error("GlobalLock failed");
      } else return unexpected_error(h.error());
    } else return unexpected_error(g.error());
  }

  template<stringable<wchar_t> S>
  std::expected<void, error_trace> text(S&& value) const {
    if (auto g = open()) {
      if (auto res = empty_clipboard(); !res) return unexpected_error(res.error());
      auto sv = std::wstring_view(value);
      const size_t bytes = (sv.size() + 1) * sizeof(wchar_t);
      HGLOBAL h = ::GlobalAlloc(GMEM_MOVEABLE, bytes);
      if (!h) return unexpected_win32_error("GlobalAlloc failed");
      void* p = ::GlobalLock(h);
      if (!p) {
        ::GlobalFree(h);
        return unexpected_win32_error("GlobalLock failed");
      }
      std::memcpy(p, sv.data(), sv.size() * sizeof(wchar_t));
      static_cast<wchar_t*>(p)[sv.size()] = L'\0';
      ::GlobalUnlock(h);
      if (!::SetClipboardData(CF_UNICODETEXT, h)) {
        ::GlobalFree(h);
        return unexpected_win32_error("SetClipboardData failed");
      }
      return {};
    } else return unexpected_error(g.error());
  }

  std::expected<bool, error_trace> has_image() const {
    if (auto g = open()) {
      return ::IsClipboardFormatAvailable(CF_DIBV5) != 0;
    } else return unexpected_error(g.error());
  }

  std::expected<bitmap, error_trace> image() const {
    if (auto g = open()) {
      if (!::IsClipboardFormatAvailable(CF_DIBV5))
        return unexpected_error(errors::invalid_operation, "clipboard does not contain image");
      if (auto h = get_clipboard_data(CF_DIBV5)) {
        if (auto hdr = static_cast<const BITMAPV5HEADER*>(::GlobalLock(*h))) {
          if (hdr->bV5Size < sizeof(BITMAPV5HEADER)) {
            ::GlobalUnlock(*h);
            return unexpected_error(errors::invalid_file, "invalid BITMAPV5HEADER size");
          }
          if (hdr->bV5BitCount != 32) {
            ::GlobalUnlock(*h);
            return unexpected_error(errors::invalid_file, "clipboard image is not 32bpp");
          }
          const int32_t w = hdr->bV5Width;
          const int32_t hgt = hdr->bV5Height;
          if (w <= 0 || hgt == 0) {
            ::GlobalUnlock(*h);
            return unexpected_error(errors::invalid_file, "invalid image size");
          }
          const uint32_t width = static_cast<uint32_t>(w);
          const uint32_t height = static_cast<uint32_t>(hgt > 0 ? hgt : -hgt);
          const uint32_t stride = width * 4;
          const uint8_t* src = reinterpret_cast<const uint8_t*>(hdr) + hdr->bV5Size;
          std::vector<uint8_t> pixels(stride * height);
          if (hgt > 0) {
            for (uint32_t y = 0; y < height; ++y) {
              const uint32_t src_row = height - 1 - y;
              std::memcpy(pixels.data() + y * stride, src + src_row * stride, stride);
            }
          } else {
            std::memcpy(pixels.data(), src, pixels.size());
          }
          ::GlobalUnlock(*h);
          return bitmap_from_pixels(uint2{width, height}, pixels.data(), stride);
        } else return unexpected_win32_error("GlobalLock failed");
      } else return unexpected_error(h.error());
    } else return unexpected_error(g.error());
  }

  std::expected<void, error_trace> image(const bitmap& b) const {
    if (!b) return unexpected_error(errors::not_initialized, "bitmap not initialized");
    if (auto g = open()) {
      if (auto res = empty_clipboard(); !res) return unexpected_error(res.error());
      uint2 size{};
      uint32_t stride = 0;
      auto pixels_res = pixels_from_bitmap(b, size, stride);
      if (!pixels_res) return unexpected_error(pixels_res.error());
      auto& pixels = *pixels_res;
      const uint32_t width = size.x;
      const uint32_t height = size.y;

      BITMAPV5HEADER hdr{};
      hdr.bV5Size = sizeof(BITMAPV5HEADER);
      hdr.bV5Width = static_cast<LONG>(width);
      hdr.bV5Height = -static_cast<LONG>(height); // top-down
      hdr.bV5Planes = 1;
      hdr.bV5BitCount = 32;
      hdr.bV5Compression = BI_BITFIELDS;
      hdr.bV5SizeImage = stride * height;
      hdr.bV5RedMask = 0x00FF0000;
      hdr.bV5GreenMask = 0x0000FF00;
      hdr.bV5BlueMask = 0x000000FF;
      hdr.bV5AlphaMask = 0xFF000000;
      hdr.bV5CSType = 0x73524742; // 'sRGB'

      const size_t total = sizeof(BITMAPV5HEADER) + pixels.size();
      HGLOBAL h = ::GlobalAlloc(GMEM_MOVEABLE, total);
      if (!h) return unexpected_win32_error("GlobalAlloc failed");
      void* mem = ::GlobalLock(h);
      if (!mem) {
        ::GlobalFree(h);
        return unexpected_win32_error("GlobalLock failed");
      }
      std::memcpy(mem, &hdr, sizeof(BITMAPV5HEADER));
      std::memcpy(static_cast<uint8_t*>(mem) + sizeof(BITMAPV5HEADER), pixels.data(), pixels.size());
      ::GlobalUnlock(h);

      if (!::SetClipboardData(CF_DIBV5, h)) {
        ::GlobalFree(h);
        return unexpected_win32_error("SetClipboardData failed");
      }
      return {};
    } else return unexpected_error(g.error());
  }
} clipboard;

} // namespace yw
