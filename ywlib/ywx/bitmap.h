#pragma once

#include "ywx/core.h"

namespace yw {

//////////////////////////////////////// MARK: bitmap

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

  static std::expected<bitmap, error_trace> create(uint2 size) {
    if (auto res = d2d.initialize(); !res) return unexpected_error(res.error());
    comptr<::ID2D1Bitmap1> bmp;
    auto hr = d2d.context()->CreateBitmap(D2D1_SIZE_U{size.x, size.y}, nullptr, 0, &properties, &bmp.get());
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateBitmap failed", int32_t(hr));
    return bitmap(std::move(bmp), size);
  }

  static std::expected<bitmap, error_trace> create(const std::filesystem::path& p) {
    if (auto res = d2d.initialize(); !res) return unexpected_error(res.error());
    if (auto res = wic.initialize(); !res) return unexpected_error(res.error());
    comptr<IWICBitmapDecoder> decoder;
    auto hr = wic.factory()->CreateDecoderFromFilename(
      p.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &decoder.get());
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateDecoderFromFilename failed", int32_t(hr));
    comptr<IWICBitmapFrameDecode> frame;
    hr = decoder->GetFrame(0, &frame.get());
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "GetFrame failed", int32_t(hr));
    comptr<IWICFormatConverter> converter;
    hr = wic.factory()->CreateFormatConverter(&converter.get());
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateFormatConverter failed", int32_t(hr));
    hr = converter->Initialize(
      frame.get(), GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeMedianCut);
    if (FAILED(hr))
      return unexpected_error(errors::operation_failed, "FormatConverter::Initialize failed", int32_t(hr));
    uint2 size;
    hr = converter->GetSize(&size.x, &size.y);
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "FormatConverter::GetSize failed", int32_t(hr));
    comptr<::ID2D1Bitmap1> bmp;
    hr = d2d.context()->CreateBitmapFromWicBitmap(converter.get(), &properties, &bmp.get());
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateBitmapFromWicBitmap failed", int32_t(hr));
    return bitmap(std::move(bmp), size);
  }

  static std::expected<bitmap, error_trace> create(IDXGISwapChain1* swapchain) {
    if (auto res = d2d.initialize(); !res) return unexpected_error(res.error());
    DXGI_SWAP_CHAIN_DESC1 scdesc{};
    if (auto hr = swapchain->GetDesc1(&scdesc); FAILED(hr))
      return unexpected_error(errors::operation_failed, "GetDesc1 failed", int32_t(hr));
    uint2 size{scdesc.Width, scdesc.Height};
    comptr<::IDXGISurface> surface;
    auto hr = swapchain->GetBuffer(0, __uuidof(IDXGISurface), reinterpret_cast<void**>(&surface.get()));
    if (FAILED(hr)) return unexpected_error(errors::operation_failed, "GetBuffer failed", int32_t(hr));
    comptr<::ID2D1Bitmap1> bmp;
    D2D1_BITMAP_PROPERTIES1 bp{pixelformat, 96.0f, 96.0f, D2D1_BITMAP_OPTIONS(3), nullptr};
    hr = d2d.context()->CreateBitmapFromDxgiSurface(surface.get(), &bp, &bmp.get());
    if (FAILED(hr))
      return unexpected_error(errors::operation_failed, "CreateBitmapFromDxgiSurface failed", int32_t(hr));
    return bitmap(std::move(bmp), size);
  }

  uint2 size() const noexcept { return _size; }

  std::expected<drawing, error_trace> begin_draw(const source& src = {}) {
    if (!*this) return unexpected_error(errors::not_initialized, "bitmap not initialized");
    return drawing::create(_bitmap.get(), src);
  }

  std::expected<drawing, error_trace> begin_draw(const color& clear_color, const source& src = {}) {
    if (auto d = begin_draw(src); d) {
      d2d.context()->Clear(reinterpret_cast<const D2D1::ColorF*>(&clear_color));
      return std::move(d);
    } else return unexpected_error(d.error());
  }

  std::expected<void, error_trace> save_as(const std::filesystem::path& p, const GUID& FileFormat) const {
    if (!*this) return unexpected_error(errors::not_initialized, "bitmap not initialized");
    if (auto res = wic.initialize(); !res) return unexpected_error(errors::not_initialized, "wic not initialized");
    comptr<IWICStream> stream;
    if (auto hr = wic.factory()->CreateStream(&stream.get()); FAILED(hr))
      return unexpected_error(errors::operation_failed, "CreateStream failed", int32_t(hr));
    if (auto hr = stream->InitializeFromFilename(p.c_str(), GENERIC_WRITE); FAILED(hr))
      return unexpected_error(errors::operation_failed, "Stream::InitializeFromFilename failed", int32_t(hr));
    comptr<IWICBitmapEncoder> encoder;
    if (auto hr = wic.factory()->CreateEncoder(FileFormat, nullptr, &encoder.get()); FAILED(hr))
      return unexpected_error(errors::operation_failed, "CreateEncoder failed", int32_t(hr));
    if (auto hr = encoder->Initialize(stream.get(), WICBitmapEncoderNoCache); FAILED(hr))
      return unexpected_error(errors::operation_failed, "Encoder::Initialize failed", int32_t(hr));
    comptr<IWICBitmapFrameEncode> frame;
    if (auto hr = encoder->CreateNewFrame(&frame.get(), nullptr); FAILED(hr))
      return unexpected_error(errors::operation_failed, "CreateNewFrame failed", int32_t(hr));
    if (auto hr = frame->Initialize(nullptr); FAILED(hr))
      return unexpected_error(errors::operation_failed, "Frame::Initialize failed", int32_t(hr));
    comptr<IWICImageEncoder> image_encoder;
    if (auto hr = wic.factory()->CreateImageEncoder(d2d.device(), &image_encoder.get()); FAILED(hr))
      return unexpected_error(errors::operation_failed, "CreateImageEncoder failed", int32_t(hr));
    if (auto hr = image_encoder->WriteFrame(_bitmap.get(), frame.get(), nullptr); FAILED(hr))
      return unexpected_error(errors::operation_failed, "ImageEncoder::WriteFrame failed", int32_t(hr));
    if (auto hr = frame->Commit(); FAILED(hr))
      return unexpected_error(errors::operation_failed, "Frame::Commit failed", int32_t(hr));
    if (auto hr = encoder->Commit(); FAILED(hr))
      return unexpected_error(errors::operation_failed, "Encoder::Commit failed", int32_t(hr));
    if (auto hr = stream->Commit(STGC_DEFAULT); FAILED(hr))
      return unexpected_error(errors::operation_failed, "Stream::Commit failed", int32_t(hr));
    return {};
  }

  std::expected<void, error_trace> save_as_png(const std::filesystem::path& p) const {
    return save_as(p, GUID_ContainerFormatPng);
  }
  std::expected<void, error_trace> save_as_jpeg(const std::filesystem::path& p) const {
    return save_as(p, GUID_ContainerFormatJpeg);
  }
};

//////////////////////////////////////// MARK: draw_bitmap

inline std::expected<void, error_trace> draw_bitmap(float2 pos, float2 size, const bitmap& b, float1 opacity = 1.0f) {
  if (!drawing::d2d_drawing()) return unexpected_error(errors::invalid_operation, "drawing not begun");
  if (!b) return {};
  D2D1_RECT_F rect = D2D1::RectF(pos.x, pos.y, pos.x + size.x, pos.y + size.y);
  d2d.context()->DrawBitmap((ID2D1Bitmap1*)b, &rect, opacity.x);
  return {};
}

inline std::expected<void, error_trace> draw_bitmap(float2 pos, const bitmap& b, float1 opacity = 1.0f) {
  if (!drawing::d2d_drawing()) return unexpected_error(errors::invalid_operation, "drawing not begun");
  if (!b) return {};
  D2D1_RECT_F rect = D2D1::RectF(pos.x, pos.y, pos.x + b.size().x, pos.y + b.size().y);
  d2d.context()->DrawBitmap((ID2D1Bitmap1*)b, &rect, opacity.x);
  return {};
}

//////////////////////////////////////// MARK: draw line

inline std::expected<void, error_trace> draw_line(float2 p0, float2 p1, const color& c, float1 width = 1.0f) {
  if (!drawing::d2d_drawing()) return unexpected_error(errors::invalid_operation, "drawing not begun");
  d2d.solid_brush()->SetColor((const D2D1_COLOR_F*)&c);
  d2d.context()->DrawLine({p0.x, p0.y}, {p1.x, p1.y}, d2d.solid_brush(), width.x, d2d.stroke_style());
  return {};
}

inline std::expected<void, error_trace> draw_line(float2 p0, float2 p1, float1 width = 1.0f) {
  return draw_line(p0, p1, colors::black, width);
}

/////////////////////////////////////// MARK: draw/fill_rectangle

inline std::expected<void, error_trace> draw_rectangle(
  float2 pos, float2 size, const color& c, float1 border_width = 1.0f) {
  if (!drawing::d2d_drawing()) return unexpected_error(errors::invalid_operation, "drawing not begun");
  d2d.solid_brush()->SetColor((const D2D1_COLOR_F*)&c);
  D2D1_RECT_F rect = D2D1::RectF(pos.x, pos.y, pos.x + size.x, pos.y + size.y);
  d2d.context()->DrawRectangle(&rect, d2d.solid_brush(), border_width.x, d2d.stroke_style());
  return {};
}

inline std::expected<void, error_trace> draw_rectangle(float2 pos, float2 size, float1 border_width = 1.0f) {
  return draw_rectangle(pos, size, colors::black, border_width);
}

inline std::expected<void, error_trace> fill_rectangle(float2 pos, float2 size, const color& c = colors::black) {
  if (!drawing::d2d_drawing()) {
    if (drawing::d3d_drawing()) return unexpected_error(errors::invalid_operation, "in d3d rendering");
    else return unexpected_error(errors::invalid_operation, "drawing not begun");
  }
  d2d.solid_brush()->SetColor((const D2D1_COLOR_F*)&c);
  d2d.context()->FillRectangle({pos.x, pos.y, pos.x + size.x, pos.y + size.y}, d2d.solid_brush());
  return {};
}

//////////////////////////////////// MARK: draw/fill_round_rectangle

inline std::expected<void, error_trace> draw_round_rectangle(
  float2 pos, float2 size, float2 radius, const color& c, float1 border_width = 1.0f) {
  if (!drawing::d2d_drawing()) return unexpected_error(errors::invalid_operation, "drawing not begun");
  d2d.solid_brush()->SetColor((const D2D1_COLOR_F*)&c);
  D2D1_ROUNDED_RECT r{D2D1::RectF(pos.x, pos.y, pos.x + size.x, pos.y + size.y), radius.x, radius.y};
  d2d.context()->DrawRoundedRectangle(&r, d2d.solid_brush(), border_width.x, d2d.stroke_style());
  return {};
}

inline std::expected<void, error_trace> draw_round_rectangle(
  float2 pos, float2 size, float2 radius, float1 border_width = 1.0f) {
  return draw_round_rectangle(pos, size, radius, colors::black, border_width);
}

inline std::expected<void, error_trace> fill_round_rectangle(
  float2 pos, float2 size, float2 radius, const color& c = colors::black) {
  if (!drawing::d2d_drawing()) return unexpected_error(errors::invalid_operation, "drawing not begun");
  d2d.solid_brush()->SetColor((const D2D1_COLOR_F*)&c);
  D2D1_ROUNDED_RECT r{D2D1::RectF(pos.x, pos.y, pos.x + size.x, pos.y + size.y), radius.x, radius.y};
  d2d.context()->FillRoundedRectangle(&r, d2d.solid_brush());
  return {};
}

//////////////////////////////////////// MARK: draw/fill_ellipse

inline std::expected<void, error_trace> draw_ellipse(
  float2 center, float2 radius, const color& c, float1 border_width = 1.0f) {
  if (!drawing::d2d_drawing()) return unexpected_error(errors::invalid_operation, "drawing not begun");
  d2d.solid_brush()->SetColor((const D2D1_COLOR_F*)&c);
  D2D1_ELLIPSE ellipse = D2D1::Ellipse({center.x, center.y}, radius.x, radius.y);
  d2d.context()->DrawEllipse(&ellipse, d2d.solid_brush(), border_width.x, d2d.stroke_style());
  return {};
}

inline std::expected<void, error_trace> draw_ellipse(float2 center, float2 radius, float1 border_width = 1.0f) {
  return draw_ellipse(center, radius, colors::black, border_width);
}

inline std::expected<void, error_trace> fill_ellipse(float2 center, float2 radius, const color& c = colors::black) {
  if (!drawing::d2d_drawing()) return unexpected_error(errors::invalid_operation, "drawing not begun");
  d2d.solid_brush()->SetColor((const D2D1_COLOR_F*)&c);
  D2D1_ELLIPSE ellipse = D2D1::Ellipse(D2D1::Point2F(center.x, center.y), radius.x, radius.y);
  d2d.context()->FillEllipse(&ellipse, d2d.solid_brush());
  return {};
}
} // namespace yw
