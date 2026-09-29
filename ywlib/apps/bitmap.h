#pragma once
#include <apps/directx.h>
#include <core/array.h>
#include <core/property.h>
#include <core/result.h>
#include <core/vector.h>

#ifdef _WIN32

namespace yw {

///--------------------------------------------------------------------------///
/// MARK: bgra

/// BGRA pixel structure (premultiplied alpha)
struct bgra {
  uint8_t b;
  uint8_t g;
  uint8_t r;
  uint8_t a;

  constexpr color to_straight_color() const noexcept {
    if (a == 0) return color(0, 0, 0, 0);
    const float inv_alpha = 1.0f / a;
    return color(r * inv_alpha, g * inv_alpha, b * inv_alpha, a / 255.0f);
  }

  static constexpr bgra from_straight_color(const color& c) noexcept {
    const auto alpha_255 = c.a * 255.0f;
    return bgra(
      static_cast<uint8_t>(c.b * alpha_255), static_cast<uint8_t>(c.g * alpha_255),
      static_cast<uint8_t>(c.r * alpha_255), static_cast<uint8_t>(alpha_255));
  }
};

///--------------------------------------------------------------------------///
/// MARK: bitmap_like

template<typename T> concept bitmap_like = convertible_to<T&, ID2D1Bitmap*> || convertible_to<T&, ID2D1Bitmap1*>;

inline constexpr auto get_d2d_bitmap = []<bitmap_like T>(T& Bitmap) {
  if constexpr (castable_to<T&, ID2D1Bitmap1*>) return static_cast<ID2D1Bitmap1*>(Bitmap);
  else if constexpr (castable_to<T&, ID2D1Bitmap*>) return static_cast<ID2D1Bitmap*>(Bitmap);
  else static_assert(always_false<T>, "unreachable");
};

///--------------------------------------------------------------------------///
/// MARK: drawing

class drawing {
  inline static void* _target = nullptr;

public:
  const_property<bool, drawing> active = false;
  explicit operator bool() const noexcept { return active() && _target; }
  static bool target_exists() noexcept { return _target != nullptr; }

  ~drawing() { close(); }
  drawing() = default;
  drawing(const drawing&) = delete;
  drawing& operator=(const drawing&) = delete;

  drawing(drawing&& o) noexcept : active(exchange(o.active.ref(), false)) {}

  drawing& operator=(drawing&& o) noexcept {
    if (this == &o) return *this;
    close();
    active.ref() = exchange(o.active.ref(), false);
    return *this;
  }

  static result<drawing> create(bitmap_like auto&& Target) {
    if (_target) return error(errors::operation_failed, "drawing target already in use");
    if (!Target) return error(errors::invalid_argument, "null target");
    d2d::context()->SetTarget(Target);
    d2d::context()->BeginDraw();
    _target = get_d2d_bitmap(Target);
    drawing result;
    result.active.ref() = true;
    return result;
  }

  drawing(bitmap_like auto&& Target, const std::source_location& sl = here()) {
    if (auto res = create(Target)) *this = move(*res);
    else res.error().add_footprint().print_and_abort(sl);
  }

  result<void> close() {
    if (!active()) return {};
    if (const auto res = d2d::context()->EndDraw(); FAILED(res))
      return error(errors::operation_failed, "EndDraw failed");
    d2d::context()->SetTarget(nullptr);
    _target = nullptr;
    active.ref() = false;
    return {};
  }
};

///--------------------------------------------------------------------------///
/// MARK: bitmap

class bitmap {
public:
  static constexpr auto dxgiformat = DXGI_FORMAT_B8G8R8A8_UNORM;
  static constexpr auto pixelformat = D2D1_PIXEL_FORMAT(dxgiformat, D2D1_ALPHA_MODE_PREMULTIPLIED);
  static constexpr auto props = D2D1_BITMAP_PROPERTIES1(pixelformat, 96.0f, 96.0f, D2D1_BITMAP_OPTIONS_TARGET);

  const_property<comptr<ID2D1Bitmap1>, bitmap> d2d_bitmap;
  const_property<uint2, bitmap> size;

  explicit operator bool() const noexcept { return bool(d2d_bitmap.cref()); }
  explicit operator ID2D1Bitmap1*() const noexcept { return d2d_bitmap.cref().get(); }

private:
  template<typename T> static constexpr bool _memory_byte = sizeof(remove_cv<T>) == 1 && !char_type<T>;

  static result<bitmap> _create_from_decoder(IWICBitmapDecoder* Dec) {
    if (!Dec) return error(errors::invalid_argument, "null decoder");
    comptr<IWICBitmapFrameDecode> frame;
    if (const auto hr = Dec->GetFrame(0, &frame.get()); FAILED(hr))
      return error(errors::operation_failed, "GetFrame failed");
    comptr<IWICFormatConverter> converter;
    if (const auto hr = com::wic()->CreateFormatConverter(&converter.get()); FAILED(hr))
      return error(errors::operation_failed, "CreateFormatConverter failed");
    const auto hr = converter->Initialize(
      frame.get(), GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, nullptr, 0, WICBitmapPaletteTypeMedianCut);
    if (FAILED(hr)) return error(errors::operation_failed, "FormatConverter initialization failed");
    bitmap result;
    if (const auto hr = converter->GetSize(&result.size.ref().x(), &result.size.ref().y()); FAILED(hr))
      return error(errors::operation_failed, "GetSize failed");
    auto& p = result.d2d_bitmap.ref().get();
    if (const auto hr = d2d::context()->CreateBitmapFromWicBitmap(converter.get(), &props, &p); FAILED(hr))
      return error(errors::operation_failed, "CreateBitmapFromWicBitmap failed");
    return result;
  }

public:
  bitmap() = default;
  bitmap(bitmap&&) = default;
  bitmap& operator=(bitmap&&) = default;

  static result<bitmap> create(uint2 Size) {
    bitmap result;
    auto& p = result.d2d_bitmap.ref().get();
    if (const auto hr = d2d::context()->CreateBitmap({Size.x(), Size.y()}, nullptr, 0, &props, &p); FAILED(hr))
      return error(errors::operation_failed, "CreateBitmap failed");
    result.size.ref() = Size;
    return result;
  }

  explicit bitmap(uint2 Size, const std::source_location& sl = here()) {
    if (auto res = create(Size)) *this = move(*res);
    else res.error().add_footprint().print_and_abort(sl);
  }

  static result<bitmap> create(uint2 Size, const bgra* Pixels, size_t Count) {
    const auto width = Size.x(), height = Size.y();
    if (width == 0 || height == 0) return error(errors::invalid_argument, "invalid bitmap size");
    const auto expected = size_t(width) * size_t(height);
    if (Count != expected) return error(errors::invalid_argument, "pixel count does not match bitmap size");
    if (!Pixels) return error(errors::invalid_argument, "null pixel data");
    if (width > UINT32_MAX / 4) return error(errors::invalid_argument, "bitmap row pitch is too large");
    bitmap result;
    auto& p = result.d2d_bitmap.ref().get();
    const auto pitch = UINT32(width * 4);
    if (const auto hr = d2d::context()->CreateBitmap(D2D1_SIZE_U{width, height}, Pixels, pitch, &props, &p); FAILED(hr))
      return error(errors::operation_failed, "CreateBitmap failed");
    result.size.ref() = uint2(width, height);
    return result;
  }

  template<bitmap_like T> static result<bitmap> create(T&& Bitmap) {
    auto b = get_d2d_bitmap(Bitmap);
    if (!b) return error(errors::invalid_argument, "null bitmap");
    const auto size = b->GetPixelSize();
    bitmap result;
    result.size.ref() = uint2(size.width, size.height);
    auto& p = result.d2d_bitmap.ref().get();
    if (const auto hr = d2d::context()->CreateBitmap(size, nullptr, 0, &props, &p); FAILED(hr))
      return error(errors::operation_failed, "CreateBitmap failed");
    if (const auto hr = p->CopyFromBitmap(nullptr, b, nullptr); FAILED(hr))
      return error(errors::operation_failed, "CopyFromBitmap failed");
    return result;
  }

  template<bitmap_like T> explicit bitmap(T&& Bitmap, const std::source_location& sl = here()) {
    if (auto res = create(Bitmap)) *this = move(*res);
    else res.error().add_footprint().print_and_abort(sl);
  }

  static result<bitmap> create_from_file(stringable auto&& Path) {
    comptr<IWICBitmapDecoder> decoder;
    const auto op = WICDecodeMetadataCacheOnLoad;
    const auto p = unicode<wchar_t>(string_view(Path));
    if (const auto hr = com::wic()->CreateDecoderFromFilename(p.c_str(), nullptr, GENERIC_READ, op, &decoder.get()))
      return error(errors::operation_failed, "CreateDecoderFromFilename failed");
    return _create_from_decoder(decoder.get());
  }

  static result<bitmap> create_from_file_data(const void* Data, size_t Bytes) {
    if (!Data || Bytes == 0) return error(errors::invalid_argument, "empty bitmap data");
    if (Bytes > DWORD(-1)) return error(errors::invalid_argument, "bitmap data is too large");
    comptr<IWICStream> stream;
    if (const auto hr = com::wic()->CreateStream(&stream.get()); FAILED(hr))
      return error(errors::operation_failed, "CreateStream failed");
    if (const auto hr = stream->InitializeFromMemory((BYTE*)const_cast<void*>(Data), DWORD(Bytes)); FAILED(hr))
      return error(errors::operation_failed, "InitializeFromMemory failed");
    comptr<IWICBitmapDecoder> decoder;
    const auto hr = com::wic()->CreateDecoderFromStream(stream.get(), 0, WICDecodeMetadataCacheOnLoad, &decoder.get());
    if (FAILED(hr)) return error(errors::operation_failed, "CreateDecoderFromStream failed");
    if (auto res = _create_from_decoder(decoder.get())) return move(*res);
    else return res.relay();
  }

  static result<bitmap> create_from_swapchain(IDXGISwapChain1* SwapChain) {
    if (!SwapChain) return error(errors::invalid_argument, "null swapchain");
    DXGI_SWAP_CHAIN_DESC1 scdesc{};
    if (const auto res = SwapChain->GetDesc1(&scdesc); FAILED(res))
      return error(errors::operation_failed, "GetDesc1 failed");
    bitmap result;
    result.size.ref() = uint2(scdesc.Width, scdesc.Height);
    comptr<IDXGISurface> surface;
    if (const auto res = SwapChain->GetBuffer(0, __uuidof(IDXGISurface), (void**)&surface.get()); FAILED(res))
      return error(errors::operation_failed, "GetBuffer failed");
    D2D1_BITMAP_PROPERTIES1 bp{pixelformat, 96.0f, 96.0f, D2D1_BITMAP_OPTIONS(3), nullptr};
    auto& p = result.d2d_bitmap.ref().get();
    if (const auto res = d2d::context()->CreateBitmapFromDxgiSurface(surface.get(), &bp, &p); FAILED(res))
      return error(errors::operation_failed, "CreateBitmapFromDxgiSurface failed");
    return result;
  }

  result<drawing> begin_draw() {
    if (auto res = drawing::create(d2d_bitmap.ref().get())) return move(*res);
    else res.relay();
  }

  result<drawing> begin_draw(const color& ClearColor) {
    if (auto res = drawing::create(d2d_bitmap.ref().get())) {
      d2d::context()->Clear(reinterpret_cast<const D2D1_COLOR_F*>(&ClearColor));
      return move(*res);
    } else res.relay();
  }

  result<void> copy_to_cpu(bgra* Out) const {
    if (!Out) return error(errors::invalid_argument, "null output buffer");
    if (!*this) return error(errors::invalid_argument, "invalid bitmap");
    const auto w = size().x(), h = size().y();
    comptr<ID2D1Bitmap1> staging;
    D2D1_BITMAP_PROPERTIES1 props{pixelformat, 96.0f, 96.0f, D2D1_BITMAP_OPTIONS(6), nullptr};
    if (const auto res = d2d::context()->CreateBitmap({w, h}, nullptr, 0, &props, &staging.get()); FAILED(res))
      return error(errors::operation_failed, "CreateBitmap failed");
    if (const auto res = staging->CopyFromBitmap(nullptr, d2d_bitmap.cref().get(), nullptr); FAILED(res))
      return error(errors::operation_failed, "CopyFromBitmap failed");
    D2D1_MAPPED_RECT mapped{};
    if (const auto res = staging->Map(D2D1_MAP_OPTIONS_READ, &mapped); FAILED(res))
      return error(errors::operation_failed, "Map failed");
    const auto src_stride = mapped.pitch;
    const auto dst_stride = size_t(w) * sizeof(bgra);
    for (uint32_t y = 0; y < h; ++y)
      ::memcpy(reinterpret_cast<uint8_t*>(Out) + y * dst_stride, mapped.bits + y * src_stride, dst_stride);
    if (const auto res = staging->Unmap(); FAILED(res)) return error(errors::operation_failed, "Unmap failed");
    return {};
  }

  array<bgra> copy_to_cpu() const {
    const auto w = size().x(), h = size().y();
    array<bgra> result(w * h);
    if (auto res = copy_to_cpu(result.data())) return result;
    else res.relay();
  }
};

///--------------------------------------------------------------------------///
/// MARK: save_bitmap

inline result<void> save_bitmap(bitmap_like auto&& Bitmap, stringable auto&& Path, const GUID& Format) {
  const auto b = get_d2d_bitmap(Bitmap);
  if (!b) return error(errors::invalid_argument, "null bitmap");
  const auto s = unicode<preferred_char>(Path);
  comptr<IWICStream> stream;
  if (const auto hr = com::wic()->CreateStream(&stream.get()); FAILED(hr))
    return error(errors::operation_failed, "CreateStream failed");
  if (const auto hr = stream->InitializeFromFilename(s.c_str(), GENERIC_WRITE); FAILED(hr))
    return error(errors::operation_failed, "InitializeFromFilename failed");
  comptr<IWICBitmapEncoder> encoder;
  if (const auto hr = com::wic()->CreateEncoder(Format, nullptr, &encoder.get()); FAILED(hr))
    return error(errors::operation_failed, "CreateEncoder failed");
  if (const auto hr = encoder->Initialize(stream.get(), WICBitmapEncoderNoCache); FAILED(hr))
    return error(errors::operation_failed, "Encoder Initialize failed");
  comptr<IWICBitmapFrameEncode> frame;
  if (const auto hr = encoder->CreateNewFrame(&frame.get(), nullptr); FAILED(hr))
    return error(errors::operation_failed, "CreateNewFrame failed");
  if (const auto hr = frame->Initialize(nullptr); FAILED(hr))
    return error(errors::operation_failed, "Frame Initialize failed");
  comptr<IWICImageEncoder> image_encoder;
  if (const auto hr = com::wic()->CreateImageEncoder(d2d::device(), &image_encoder.get()); FAILED(hr))
    return error(errors::operation_failed, "CreateImageEncoder failed");
  if (const auto hr = image_encoder->WriteFrame(b, frame.get(), nullptr); FAILED(hr))
    return error(errors::operation_failed, "WriteFrame failed");
  if (const auto hr = frame->Commit(); FAILED(hr)) return error(errors::operation_failed, "Frame Commit failed");
  if (const auto hr = encoder->Commit(); FAILED(hr)) return error(errors::operation_failed, "Encoder Commit failed");
  if (const auto hr = stream->Commit(STGC_DEFAULT); FAILED(hr))
    return error(errors::operation_failed, "Stream Commit failed");
  return {};
}

inline result<void> save_bitmap_png(bitmap_like auto&& bitmap, stringable auto&& Path) {
  if (auto res = save_bitmap(bitmap, Path, GUID_ContainerFormatPng)) return {};
  else return res.error().relay();
}

inline result<void> save_bitmap_jpeg(bitmap_like auto&& bitmap, stringable auto&& Path) {
  if (auto res = save_bitmap(bitmap, Path, GUID_ContainerFormatJpeg)) return {};
  else return res.error().relay();
}

///--------------------------------------------------------------------------///
/// MARK: draw_bitmap

template<typename T>
inline result<void> draw_bitmap(float2 Pos, float2 Size, bitmap_like auto&& Bitmap, float1 Opacity = 1.0f) {
  if (!drawing::target_exists()) return error(errors::invalid_operation, "drawing target not available");
  D2D1_RECT_F rect(Pos.x(), Pos.y(), Pos.x() + Size.x(), Pos.y() + Size.y());
  d2d::context()->DrawBitmap(get_d2d_bitmap(Bitmap), &rect, Opacity.x());
  return {};
}

inline result<void> draw_bitmap(float2 Pos, bitmap_like auto&& Bitmap, float1 Opacity = 1.0f) {
  if (!drawing::target_exists()) return error(errors::invalid_operation, "drawing target not available");
  auto b = get_d2d_bitmap(Bitmap);
  const auto size = b->GetPixelSize();
  D2D1_RECT_F rect = D2D1::RectF(Pos.x(), Pos.y(), Pos.x() + size.width, Pos.y() + size.height);
  d2d::context()->DrawBitmap(b, &rect, Opacity.x());
  return {};
}

///--------------------------------------------------------------------------///
/// MARK: draw line

inline result<void> stroke_line(float2 p0, float2 p1, float1 width = 1.0f) {
  if (!drawing::target_exists()) return error(errors::invalid_operation, "drawing target not available");
  d2d::context()->DrawLine({p0[0], p0[1]}, {p1[0], p1[1]}, d2d::solid_color_brush(), width.x(), d2d::stroke_style());
  return {};
}

inline result<void> stroke_line(float2 p0, float2 p1, const color& Color, float1 width = 1.0f) {
  d2d::set_solid_color(Color);
  if (auto res = stroke_line(p0, p1, width)) return {};
  else return res.relay();
}

///--------------------------------------------------------------------------///
/// MARK: draw rectangle

inline result<void> stroke_rectangle(float2 pos, float2 size, float1 border_width = 1.0f) {
  if (!drawing::target_exists()) return error(errors::invalid_operation, "drawing target not available");
  D2D1_RECT_F rect = D2D1::RectF(pos.x(), pos.y(), pos.x() + size.x(), pos.y() + size.y());
  d2d::context()->DrawRectangle(&rect, d2d::solid_color_brush(), border_width.x(), d2d::stroke_style());
  return {};
}

inline result<void> stroke_rectangle(float2 pos, float2 size, const color& Color, float1 border_width = 1.0f) {
  d2d::set_solid_color(Color);
  if (auto res = stroke_rectangle(pos, size, border_width)) return {};
  else return res.relay();
}

/// \param Rect `{left, top, right, bottom}`
inline result<void> stroke_rectangle(const float4& Rect, float1 Width = 1.0f) {
  if (!drawing::target_exists()) return error(errors::invalid_operation, "drawing target not available");
  d2d::context()->DrawRectangle((const D2D1_RECT_F*)&Rect, d2d::solid_color_brush(), Width.x(), d2d::stroke_style());
  return {};
}

inline result<void> stroke_rectangle(const float4& Rect, const color& Color, float1 Width = 1.0f) {
  d2d::set_solid_color(Color);
  if (auto res = stroke_rectangle(Rect, Width)) return {};
  else return res.relay();
}

inline result<void> fill_rectangle(float2 pos, float2 size) {
  if (!drawing::target_exists()) return error(errors::invalid_operation, "drawing target not available");
  const auto r = D2D1_RECT_F(pos.x(), pos.y(), pos.x() + size.x(), pos.y() + size.y());
  d2d::context()->FillRectangle(r, d2d::solid_color_brush());
  return {};
}

inline result<void> fill_rectangle(float2 pos, float2 size, const color& Color) {
  d2d::set_solid_color(Color);
  if (auto res = fill_rectangle(pos, size)) return {};
  else return res.relay();
}

/// \param Rect `{left, top, right, bottom}`
inline result<void> fill_rectangle(const float4& Rect) {
  if (!drawing::target_exists()) return error(errors::invalid_operation, "drawing target not available");
  d2d::context()->FillRectangle((const D2D1_RECT_F*)&Rect, d2d::solid_color_brush());
  return {};
}

inline result<void> fill_rectangle(const float4& Rect, const color& Color) {
  d2d::set_solid_color(Color);
  if (auto res = fill_rectangle(Rect)) return {};
  else return res.relay();
}

///--------------------------------------------------------------------------///
/// MARK: draw round_rectangle

inline result<void> stroke_round_rectangle(float2 pos, float2 size, float2 radius, float1 border_width = 1.0f) {
  if (!drawing::target_exists()) return error(errors::invalid_operation, "drawing target not available");
  D2D1_ROUNDED_RECT r{D2D1::RectF(pos.x(), pos.y(), pos.x() + size.x(), pos.y() + size.y()), radius.x(), radius.y()};
  d2d::context()->DrawRoundedRectangle(&r, d2d::solid_color_brush(), border_width.x(), d2d::stroke_style());
  return {};
}

inline result<void> stroke_round_rectangle(
  float2 pos, float2 size, float2 radius, const color& Color, float1 border_width = 1.0f) {
  d2d::set_solid_color(Color);
  if (auto res = stroke_round_rectangle(pos, size, radius, border_width)) return {};
  else return res.relay();
}

inline result<void> fill_round_rectangle(float2 pos, float2 size, float2 radius) {
  if (!drawing::target_exists()) return error(errors::invalid_operation, "drawing target not available");
  D2D1_ROUNDED_RECT r{D2D1::RectF(pos.x(), pos.y(), pos.x() + size.x(), pos.y() + size.y()), radius.x(), radius.y()};
  d2d::context()->FillRoundedRectangle(&r, d2d::solid_color_brush());
  return {};
}

inline result<void> fill_round_rectangle(float2 pos, float2 size, float2 radius, const color& Color) {
  d2d::set_solid_color(Color);
  if (auto res = fill_round_rectangle(pos, size, radius)) return {};
  else return res.relay();
}

///--------------------------------------------------------------------------///
/// MARK: draw ellipse

inline result<void> stroke_ellipse(float2 center, float2 radius, float1 border_width = 1.0f) {
  if (!drawing::target_exists()) return error(errors::invalid_operation, "drawing target not available");
  D2D1_ELLIPSE ellipse = D2D1::Ellipse({center.x(), center.y()}, radius.x(), radius.y());
  d2d::context()->DrawEllipse(&ellipse, d2d::solid_color_brush(), border_width.x(), d2d::stroke_style());
  return {};
}

inline result<void> stroke_ellipse(float2 center, float2 radius, const color& Color, float1 border_width = 1.0f) {
  d2d::set_solid_color(Color);
  if (auto res = stroke_ellipse(center, radius, border_width)) return {};
  else return res.relay();
}

inline result<void> fill_ellipse(float2 center, float2 radius) {
  if (!drawing::target_exists()) return error(errors::invalid_operation, "drawing target not available");
  D2D1_ELLIPSE ellipse = D2D1::Ellipse(D2D1::Point2F(center.x(), center.y()), radius.x(), radius.y());
  d2d::context()->FillEllipse(&ellipse, d2d::solid_color_brush());
  return {};
}

inline result<void> fill_ellipse(float2 center, float2 radius, const color& Color) {
  d2d::set_solid_color(Color);
  if (auto res = fill_ellipse(center, radius)) return {};
  else return res.relay();
}

///--------------------------------------------------------------------------///
/// MARK: draw geometry

template<typename T> concept geometry_like = castable_to<T&, ID2D1Geometry*>;
inline constexpr auto get_geometry = []<geometry_like T>(T&& geometry) noexcept(nt_castable_to<T&, ID2D1Geometry*>) {
  return static_cast<ID2D1Geometry*>(geometry);
};

inline result<void> stroke_geometry(geometry_like auto&& geometry, float1 Thickness = 1.0f) {
  if (!drawing::target_exists()) return error(errors::invalid_operation, "drawing target not available");
  d2d::context()->DrawGeometry(get_geometry(geometry), d2d::solid_color_brush(), Thickness[0], d2d::stroke_style());
  return {};
}

inline result<void> stroke_geometry(geometry_like auto&& geometry, const color& Color, float1 Thickness = 1.0f) {
  d2d::set_solid_color(Color);
  if (auto res = stroke_geometry(geometry, Thickness)) return {};
  else return res.relay();
}

inline result<void> fill_geometry(geometry_like auto&& geometry) {
  if (!drawing::target_exists()) return error(errors::invalid_operation, "drawing target not available");
  d2d::context()->FillGeometry(get_geometry(geometry), d2d::solid_color_brush(), nullptr);
  return {};
}

inline result<void> fill_geometry(geometry_like auto&& geometry, const color& Color) {
  d2d::set_solid_color(Color);
  if (auto res = fill_geometry(geometry)) return {};
  else return res.relay();
}
} // namespace yw

#endif
