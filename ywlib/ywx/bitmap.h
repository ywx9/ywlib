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

  bool _error(const char* msg) const {
    std::print("bitmap: {}\n", msg);
    return false;
  }

  bool _fatal(const char* msg) {
    std::print("bitmap: {}\n", msg);
    if (_bitmap) _bitmap.release();
    return false;
  }

  bool _create_empty_bitmap(uint2 size) {
    if (!d2d.initialize()) return _fatal("d2d not initialized");
    auto hr = d2d.context()->CreateBitmap(D2D1_SIZE_U{size.x, size.y}, nullptr, 0, &properties, &_bitmap.get());
    if (FAILED(hr)) return _fatal("CreateBitmap failed");
  }

  bool _create_from_file(const std::filesystem::path& p) {
    if (!d2d.initialize()) return _fatal("d2d not initialized");
    if (!wic.initialize()) return _fatal("wic not initialized");
    yw::comptr<IWICBitmapDecoder> decoder;
    auto hr = wic.factory()->CreateDecoderFromFilename(p.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad,
                                                       &decoder.get());
    if (FAILED(hr)) return _fatal("CreateDecoderFromFilename failed");
    yw::comptr<IWICBitmapFrameDecode> frame;
    hr = decoder->GetFrame(0, &frame.get());
    if (FAILED(hr)) return _fatal("GetFrame failed");
    yw::comptr<IWICFormatConverter> converter;
    hr = wic.factory()->CreateFormatConverter(&converter.get());
    if (FAILED(hr)) return _fatal("CreateFormatConverter failed");
    hr = converter->Initialize(frame.get(), GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, nullptr, 0.0,
                               WICBitmapPaletteTypeMedianCut);
    if (FAILED(hr)) return _fatal("FormatConverter::Initialize failed");
    hr = converter->GetSize(&_size.x, &_size.y);
    if (FAILED(hr)) return _fatal("FormatConverter::GetSize failed");
    hr = d2d.context()->CreateBitmapFromWicBitmap(converter.get(), &properties, &_bitmap.get());
    if (FAILED(hr)) return _fatal("CreateBitmapFromWicBitmap failed");
    return true;
  }

  bool _create_from_swapchain(::IDXGISwapChain1* swapchain) {
    if (!d2d.initialize()) return _fatal("d2d not initialized");
    DXGI_SWAP_CHAIN_DESC1 scdesc{};
    if (auto hr = swapchain->GetDesc1(&scdesc); FAILED(hr)) return _fatal("GetDesc1 failed");
    _size = uint2{scdesc.Width, scdesc.Height};
    yw::comptr<::IDXGISurface> surface;
    auto hr = swapchain->GetBuffer(0, __uuidof(IDXGISurface), reinterpret_cast<void**>(&surface.get()));
    if (FAILED(hr)) return _fatal("GetBuffer failed");
    hr = d2d.context()->CreateBitmapFromDxgiSurface(surface.get(), &properties, &_bitmap.get());
    if (FAILED(hr)) return _fatal("CreateBitmapFromDxgiSurface failed");
    return true;
  }

public:
  bitmap() = default;
  explicit bitmap(uint2 size) : _size(size) { _create_empty_bitmap(size); }
  explicit bitmap(const std::filesystem::path& p) { _create_from_file(p); }
  explicit operator bool() const noexcept { return static_cast<bool>(_bitmap); }
  explicit operator ::ID2D1Bitmap1*&() & noexcept { return _bitmap.get(); }
  explicit operator ::ID2D1Bitmap1*() const& noexcept { return _bitmap.get(); }

  uint2 size() const noexcept { return _size; }

  bool begin_draw() {
    if (!*this) return _error("bitmap not initialized");
    internal::render_target = _bitmap.get();
    d2d.context()->SetTarget(_bitmap.get());
    d2d.context()->BeginDraw();
    return true;
  }

  bool begin_draw(const color& clear_color) {
    if (!begin_draw()) return false;
    d2d.context()->Clear(D2D1::ColorF(clear_color.r, clear_color.g, clear_color.b, clear_color.a));
    return true;
  }

  bool end_draw() {
    if (!*this) return _error("bitmap not initialized");
    if (internal::render_target.index() != 1 || std::get<1>(internal::render_target) != _bitmap.get())
      return _error("bitmap not in drawing state");
    if (auto hr = d2d.context()->EndDraw(); FAILED(hr)) return _error("EndDraw failed");
    d2d.context()->SetTarget(nullptr);
    internal::render_target = std::monostate{};
    return true;
  }

  bool save_as(const std::filesystem::path& p, const GUID& FileFormat) const {
    if (!*this) return _error("bitmap not initialized");
    if (!wic.initialize()) return _error("wic not initialized");
    comptr<IWICStream> stream;
    if (auto hr = wic.factory()->CreateStream(&stream.get()); FAILED(hr)) return _error("CreateStream failed");
    if (auto hr = stream->InitializeFromFilename(p.c_str(), GENERIC_WRITE); FAILED(hr))
      return _error("InitializeFromFilename failed");
    comptr<IWICBitmapEncoder> encoder;
    if (auto hr = wic.factory()->CreateEncoder(FileFormat, nullptr, &encoder.get()); FAILED(hr))
      return _error("CreateEncoder failed");
    if (auto hr = encoder->Initialize(stream.get(), WICBitmapEncoderNoCache); FAILED(hr))
      return _error("Encoder::Initialize failed");
    comptr<IWICBitmapFrameEncode> frame;
    if (auto hr = encoder->CreateNewFrame(&frame.get(), nullptr); FAILED(hr)) return _error("CreateNewFrame failed");
    if (auto hr = frame->Initialize(nullptr); FAILED(hr)) return _error("Frame::Initialize failed");
    comptr<IWICImageEncoder> image_encoder;
    if (auto hr = wic.factory()->CreateImageEncoder(d2d.device(), &image_encoder.get()); FAILED(hr))
      return _error("CreateImageEncoder failed");
    if (auto hr = image_encoder->WriteFrame(_bitmap.get(), frame.get(), nullptr); FAILED(hr))
      return _error("ImageEncoder::WriteFrame failed");
    if (auto hr = frame->Commit(); FAILED(hr)) return _error("Frame::Commit failed");
    if (auto hr = encoder->Commit(); FAILED(hr)) return _error("Encoder::Commit failed");
    if (auto hr = stream->Commit(STGC_DEFAULT); FAILED(hr)) return _error("Stream::Commit failed");
    return true;
  }

  bool save_as_png(const std::filesystem::path& p) const { return save_as(p, GUID_ContainerFormatPng); }
  bool save_as_jpeg(const std::filesystem::path& p) const { return save_as(p, GUID_ContainerFormatJpeg); }
};

//////////////////////////////////////// MARK: draw_bitmap

inline void draw_bitmap(float2 pos, float2 size, const bitmap& b, float1 opacity = 1.0f) {
  if (!d2d.initialize()) throw std::runtime_error("draw_bitmap: d2d not initialized");
  if (internal::render_target.index() != 1) throw std::runtime_error("draw_bitmap: render target not set");
  if (std::get<1>(internal::render_target) == static_cast<::ID2D1Bitmap1*>(b))
    throw std::runtime_error("draw_bitmap: cannot draw bitmap onto itself");
  if (!b) return;
  D2D1_RECT_F rect = D2D1::RectF(pos.x, pos.y, pos.x + size.x, pos.y + size.y);
  d2d.context()->DrawBitmap((ID2D1Bitmap1*)b, &rect, opacity.x);
}

inline void draw_bitmap(float2 pos, const bitmap& b, float1 opacity = 1.0f) {
  if (!d2d.initialize()) throw std::runtime_error("draw_bitmap: d2d not initialized");
  if (internal::render_target.index() != 1) throw std::runtime_error("draw_bitmap: render target not set");
  if (std::get<1>(internal::render_target) == static_cast<::ID2D1Bitmap1*>(b))
    throw std::runtime_error("draw_bitmap: cannot draw bitmap onto itself");
  if (!b) return;
  D2D1_RECT_F rect = D2D1::RectF(pos.x, pos.y, pos.x + b.size().x, pos.y + b.size().y);
  d2d.context()->DrawBitmap((ID2D1Bitmap1*)b, &rect, opacity.x);
}

//////////////////////////////////////// MARK: draw line

inline void draw_line(float2 p0, float2 p1, const color& c, float1 width = 1.0f) {
  if (!d2d.initialize()) throw std::runtime_error("draw_line: d2d not initialized");
  if (internal::render_target.index() != 1) throw std::runtime_error("draw_line: render target not set");
  d2d.solid_brush()->SetColor((const D2D1_COLOR_F*)&c);
  d2d.context()->DrawLine(D2D1::Point2F(p0.x, p0.y), D2D1::Point2F(p1.x, p1.y), d2d.solid_brush(), width.x,
                          d2d.stroke_style());
}

inline void draw_line(float2 p0, float2 p1, float1 width = 1.0f) {
  draw_line(p0, p1, colors::black, width);
}

/////////////////////////////////////// MARK: draw/fill_rectangle

inline void draw_rectangle(float2 pos, float2 size, const color& c, float1 border_width = 1.0f) {
  if (!d2d.initialize()) throw std::runtime_error("draw_rectangle: d2d not initialized");
  if (internal::render_target.index() != 1) throw std::runtime_error("draw_rectangle: render target not set");
  d2d.solid_brush()->SetColor((const D2D1_COLOR_F*)&c);
  D2D1_RECT_F rect = D2D1::RectF(pos.x, pos.y, pos.x + size.x, pos.y + size.y);
  d2d.context()->DrawRectangle(&rect, d2d.solid_brush(), border_width.x, d2d.stroke_style());
}

inline void draw_rectangle(float2 pos, float2 size, float1 border_width = 1.0f) {
  draw_rectangle(pos, size, colors::black, border_width);
}

inline void fill_rectangle(float2 pos, float2 size, const color& c = colors::black) {
  if (!d2d.initialize()) throw std::runtime_error("fill_rectangle: d2d not initialized");
  if (internal::render_target.index() != 1) throw std::runtime_error("fill_rectangle: render target not set");
  d2d.solid_brush()->SetColor((const D2D1_COLOR_F*)&c);
  D2D1_RECT_F rect = D2D1::RectF(pos.x, pos.y, pos.x + size.x, pos.y + size.y);
  d2d.context()->FillRectangle(&rect, d2d.solid_brush());
}

//////////////////////////////////// MARK: draw/fill_round_rectangle

inline void draw_round_rectangle(float2 pos, float2 size, float2 radius, const color& c, float1 border_width = 1.0f) {
  if (!d2d.initialize()) throw std::runtime_error("draw_round_rectangle: d2d not initialized");
  if (internal::render_target.index() != 1) throw std::runtime_error("draw_round_rectangle: render target not set");
  d2d.solid_brush()->SetColor((const D2D1_COLOR_F*)&c);
  D2D1_ROUNDED_RECT r{D2D1::RectF(pos.x, pos.y, pos.x + size.x, pos.y + size.y), radius.x, radius.y};
  d2d.context()->DrawRoundedRectangle(&r, d2d.solid_brush(), border_width.x, d2d.stroke_style());
}

inline void draw_round_rectangle(float2 pos, float2 size, float2 radius, float1 border_width = 1.0f) {
  draw_round_rectangle(pos, size, radius, colors::black, border_width);
}

inline void fill_round_rectangle(float2 pos, float2 size, float2 radius, const color& c = colors::black) {
  if (!d2d.initialize()) throw std::runtime_error("fill_round_rectangle: d2d not initialized");
  if (internal::render_target.index() != 1) throw std::runtime_error("fill_round_rectangle: render target not set");
  d2d.solid_brush()->SetColor((const D2D1_COLOR_F*)&c);
  D2D1_ROUNDED_RECT r{D2D1::RectF(pos.x, pos.y, pos.x + size.x, pos.y + size.y), radius.x, radius.y};
  d2d.context()->FillRoundedRectangle(&r, d2d.solid_brush());
}

//////////////////////////////////////// MARK: draw/fill_ellipse

inline void draw_ellipse(float2 center, float2 radius, const color& c, float1 border_width = 1.0f) {
  if (!d2d.initialize()) throw std::runtime_error("draw_ellipse: d2d not initialized");
  if (internal::render_target.index() != 1) throw std::runtime_error("draw_ellipse: render target not set");
  d2d.solid_brush()->SetColor((const D2D1_COLOR_F*)&c);
  D2D1_ELLIPSE ellipse = D2D1::Ellipse(D2D1::Point2F(center.x, center.y), radius.x, radius.y);
  d2d.context()->DrawEllipse(&ellipse, d2d.solid_brush(), border_width.x, d2d.stroke_style());
}

inline void draw_ellipse(float2 center, float2 radius, float1 border_width = 1.0f) {
  draw_ellipse(center, radius, colors::black, border_width);
}

inline void fill_ellipse(float2 center, float2 radius, const color& c = colors::black) {
  if (!d2d.initialize()) throw std::runtime_error("fill_ellipse: d2d not initialized");
  if (internal::render_target.index() != 1) throw std::runtime_error("fill_ellipse: render target not set");
  d2d.solid_brush()->SetColor((const D2D1_COLOR_F*)&c);
  D2D1_ELLIPSE ellipse = D2D1::Ellipse(D2D1::Point2F(center.x, center.y), radius.x, radius.y);
  d2d.context()->FillEllipse(&ellipse, d2d.solid_brush());
}

} // namespace yw
