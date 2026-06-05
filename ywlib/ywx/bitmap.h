#pragma once
#include "ywx/drawing.h"

namespace yw {

template<typename Bitmap> concept bitmap_like = castable_to<Bitmap, ID2D1Bitmap1*>;

class bitmap {
public:
  static constexpr auto dxgiformat = DXGI_FORMAT_B8G8R8A8_UNORM;
  static constexpr auto pixelformat = D2D1_PIXEL_FORMAT(dxgiformat, D2D1_ALPHA_MODE_PREMULTIPLIED);
  static constexpr auto properties = D2D1_BITMAP_PROPERTIES1(pixelformat, 96.0f, 96.0f, D2D1_BITMAP_OPTIONS_TARGET);

private:
  comptr<::ID2D1Bitmap1> _bitmap;
  uint2 _size;

  bitmap(comptr<::ID2D1Bitmap1>&& bmp, uint2 size) : _bitmap(std::move(bmp)), _size(size) {}

public:
  bitmap() = default;
  explicit operator bool() const noexcept { return static_cast<bool>(_bitmap); }
  explicit operator ::ID2D1Bitmap1*&() & noexcept { return _bitmap.get(); }
  explicit operator ::ID2D1Bitmap1*() const& noexcept { return _bitmap.get(); }
  ::ID2D1Bitmap1* d2d_bitmap() const noexcept { return _bitmap.get(); }

  /// creates empty bitmap with specified size
  static std::expected<bitmap, error_trace> create(uint2 size) {
    comptr<::ID2D1Bitmap1> bmp;
    hresult_test(d2d().context()->CreateBitmap, D2D1_SIZE_U{size.x, size.y}, nullptr, 0, &properties, &bmp.get());
    return bitmap(std::move(bmp), size);
  }

  /// creates bitmap from image file
  static std::expected<bitmap, error_trace> create(const std::filesystem::path& p) {
    comptr<IWICBitmapDecoder> decoder;
    const auto& wic = yw::wic();
    const auto option = WICDecodeMetadataCacheOnLoad;
    hresult_test(wic.factory()->CreateDecoderFromFilename, p.c_str(), nullptr, GENERIC_READ, option, &decoder.get());
    comptr<IWICBitmapFrameDecode> frame;
    hresult_test(decoder->GetFrame, 0, &frame.get());
    comptr<IWICFormatConverter> converter;
    hresult_test(wic.factory()->CreateFormatConverter, &converter.get());
    hresult_test(
      converter->Initialize, frame.get(), GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, nullptr, 0.0,
      WICBitmapPaletteTypeMedianCut);
    uint2 size;
    hresult_test(converter->GetSize, &size.x, &size.y);
    comptr<::ID2D1Bitmap1> bmp;
    hresult_test(d2d().context()->CreateBitmapFromWicBitmap, converter.get(), &properties, &bmp.get());
    return bitmap(std::move(bmp), size);
  }

  /// creates bitmap for rendertarget from swapchain
  static std::expected<bitmap, error_trace> create(IDXGISwapChain1* swapchain) {
    if (!swapchain) return unexpected_error(errors::invalid_argument, "null swapchain");
    DXGI_SWAP_CHAIN_DESC1 scdesc{};
    hresult_test(swapchain->GetDesc1, &scdesc);
    uint2 size{scdesc.Width, scdesc.Height};
    comptr<::IDXGISurface> surface;
    hresult_test(swapchain->GetBuffer, 0, __uuidof(IDXGISurface), reinterpret_cast<void**>(&surface.get()));
    comptr<::ID2D1Bitmap1> bmp;
    D2D1_BITMAP_PROPERTIES1 bp{pixelformat, 96.0f, 96.0f, D2D1_BITMAP_OPTIONS(3), nullptr};
    hresult_test(d2d().context()->CreateBitmapFromDxgiSurface, surface.get(), &bp, &bmp.get());
    return bitmap(std::move(bmp), size);
  }

  /// copies bitmap from another
  static std::expected<bitmap, error_trace> create(const bitmap& source) {
    if (!source) return unexpected_error(errors::invalid_argument, "source bitmap not initialized");
    const auto size = source.size();
    comptr<::ID2D1Bitmap1> bmp;
    hresult_test(d2d().context()->CreateBitmap, D2D1_SIZE_U{size.x, size.y}, nullptr, 0, &properties, &bmp.get());
    D2D1_RECT_U rect{0, 0, size.x, size.y};
    D2D1_POINT_2U pt{0, 0};
    hresult_test(bmp->CopyFromBitmap, &pt, source._bitmap.get(), &rect);
    return bitmap(std::move(bmp), size);
  }

  uint2 size() const noexcept { return _size; }

  std::expected<drawing, error_trace> begin_draw(const source& src = {}) {
    if (!*this) return unexpected_error(errors::not_initialized, "bitmap not initialized");
    if (auto res = drawing::create(_bitmap.get(), src)) return std::move(*res);
    else return unexpected_error(res.error());
  }

  std::expected<drawing, error_trace> begin_draw(const color& clear_color, const source& src = {}) {
    if (auto d = begin_draw(src); d) {
      d2d().context()->Clear(reinterpret_cast<const D2D1::ColorF*>(&clear_color));
      return std::move(d);
    } else return unexpected_error(d.error());
  }

  std::expected<void, error_trace> save_as(const std::filesystem::path& p, const GUID& FileFormat) const {
    if (!*this) return unexpected_error(errors::not_initialized, "bitmap not initialized");
    const auto& wic = yw::wic();
    comptr<IWICStream> stream;
    hresult_test(wic.factory()->CreateStream, &stream.get());
    hresult_test(stream->InitializeFromFilename, p.c_str(), GENERIC_WRITE);
    comptr<IWICBitmapEncoder> encoder;
    hresult_test(wic.factory()->CreateEncoder, FileFormat, nullptr, &encoder.get());
    hresult_test(encoder->Initialize, stream.get(), WICBitmapEncoderNoCache);
    comptr<IWICBitmapFrameEncode> frame;
    hresult_test(encoder->CreateNewFrame, &frame.get(), nullptr);
    hresult_test(frame->Initialize, nullptr);
    comptr<IWICImageEncoder> image_encoder;
    hresult_test(wic.factory()->CreateImageEncoder, d2d().device(), &image_encoder.get());
    hresult_test(image_encoder->WriteFrame, _bitmap.get(), frame.get(), nullptr);
    hresult_test(frame->Commit);
    hresult_test(encoder->Commit);
    hresult_test(stream->Commit, STGC_DEFAULT);
    return {};
  }

  std::expected<void, error_trace> save_as_png(const std::filesystem::path& p) const {
    if (auto res = save_as(p, GUID_ContainerFormatPng)) return {};
    else return unexpected_error(res.error());
  }
  std::expected<void, error_trace> save_as_jpeg(const std::filesystem::path& p) const {
    if (auto res = save_as(p, GUID_ContainerFormatJpeg)) return {};
    else return unexpected_error(res.error());
  }
};

//////////////////////////////////////// MARK: draw_bitmap

inline std::expected<void, error_trace> draw_bitmap(float2 pos, float2 size, const bitmap& b, float1 opacity = 1.0f) {
  if (!drawing::d2d_drawing()) return unexpected_error(errors::invalid_operation, "drawing not begun");
  if (!b) return {};
  D2D1_RECT_F rect = D2D1::RectF(pos.x, pos.y, pos.x + size.x, pos.y + size.y);
  d2d().context()->DrawBitmap((ID2D1Bitmap1*)b, &rect, opacity.x);
  return {};
}

inline std::expected<void, error_trace> draw_bitmap(float2 pos, const bitmap& b, float1 opacity = 1.0f) {
  if (!drawing::d2d_drawing()) return unexpected_error(errors::invalid_operation, "drawing not begun");
  if (!b) return {};
  D2D1_RECT_F rect = D2D1::RectF(pos.x, pos.y, pos.x + b.size().x, pos.y + b.size().y);
  d2d().context()->DrawBitmap((ID2D1Bitmap1*)b, &rect, opacity.x);
  return {};
}
} // namespace yw
