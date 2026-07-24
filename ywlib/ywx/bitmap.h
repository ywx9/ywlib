#pragma once
#include "ywx/drawing.h"

namespace yw {

class bitmap : public handle_base {
public:
  static constexpr auto dxgiformat = DXGI_FORMAT_B8G8R8A8_UNORM;
  static constexpr auto pixelformat = D2D1_PIXEL_FORMAT(dxgiformat, D2D1_ALPHA_MODE_PREMULTIPLIED);
  static constexpr auto properties = D2D1_BITMAP_PROPERTIES1(pixelformat, 96.0f, 96.0f, D2D1_BITMAP_OPTIONS_TARGET);

  struct slot : handle_base::slot {
    comptr<ID2D1Bitmap1> bitmap;
    uint2 size{};
  };

  bitmap() noexcept = default;

  explicit operator bool() const noexcept {
    const auto sp = slot::get_as<bitmap>(id());
    return sp && static_cast<bool>(sp->bitmap);
  }

  auto d2d_bitmap(this auto&& Self) noexcept -> copy_cv<remove_ref<decltype(Self)>, ID2D1Bitmap1>* {
    if (const auto sp = slot::get_as<bitmap>(Self.id())) return sp->bitmap.get();
    else return nullptr;
  }

  static std::expected<bitmap, error> create(const bitmap& Other) {
    if (auto res = create(Other.size())) {
      // copy bitmap content
      if (auto hr = d2d::context()->CopyFromBitmap(res.value().d2d_bitmap(), Other.d2d_bitmap()); FAILED(hr))
        return std::unexpected(error(errors::operation_failed, "failed to copy bitmap", int32_t(hr)));
    }
  }

  static std::expected<bitmap, error> create(uint2 Size) {
    const auto sp = make_slot<bitmap>();
    if (!sp) return std::unexpected(error(errors::invalid_operation, "failed to create bitmap slot"));
    const auto hr = d2d::context()->CreateBitmap({Size.x, Size.y}, nullptr, 0, &properties, &sp->bitmap.get());
    if (FAILED(hr)) {
      erase_slot(sp->id);
      return std::unexpected(error(errors::operation_failed, "failed to create bitmap", int32_t(hr)));
    }
    sp->size = Size;
    return make_handle<bitmap>(sp->id);
  }

  bitmap(uint2 Size, const source_line& sl = here()) {
    if (auto res = create(Size); !res) res.error().go_off(sl);
    else *this = std::move(res.value());
  }

  static std::expected<bitmap, error> create(const std::filesystem::path& Path) {
    const auto sp = make_slot<bitmap>();
    if (!sp) return std::unexpected(error(errors::invalid_operation, "failed to create bitmap slot"));
    comptr<IWICBitmapDecoder> decoder;
    const auto option = WICDecodeMetadataCacheOnLoad;
    const auto wicf = wic::factory();
    hresult_test(wicf->CreateDecoderFromFilename, Path.c_str(), nullptr, GENERIC_READ, option, &decoder.get());
    comptr<IWICBitmapFrameDecode> frame;
    hresult_test(decoder->GetFrame, 0, &frame.get());
    comptr<IWICFormatConverter> converter;
    hresult_test(wicf->CreateFormatConverter, &converter.get());
    hresult_test(
      converter->Initialize, frame.get(), GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, nullptr, 0.0,
      WICBitmapPaletteTypeMedianCut);
    hresult_test(converter->GetSize, &sp->size.x, &sp->size.y);
    hresult_test(d2d::context()->CreateBitmapFromWicBitmap, converter.get(), &properties, &sp->bitmap.get());
    return make_handle<bitmap>(sp->id);
  }

  bitmap(const std::filesystem::path& Path, const source_line& sl = here()) {
    if (auto res = create(Path); !res) res.error().go_off(sl);
    else *this = std::move(res.value());
  }

  static std::expected<bitmap, error> create(IDXGISwapChain1* SwapChain) {
    if (!SwapChain) return std::unexpected(error(errors::invalid_argument, "null swapchain"));
    const auto sp = make_slot<bitmap>();
    if (!sp) return std::unexpected(error(errors::invalid_operation, "failed to create bitmap slot"));
    DXGI_SWAP_CHAIN_DESC1 scdesc{};
    hresult_test(SwapChain->GetDesc1, &scdesc);
    sp->size = uint2{scdesc.Width, scdesc.Height};
    comptr<IDXGISurface> surface;
    hresult_test(SwapChain->GetBuffer, 0, __uuidof(IDXGISurface), reinterpret_cast<void**>(&surface.get()));
    D2D1_BITMAP_PROPERTIES1 bp{pixelformat, 96.0f, 96.0f, D2D1_BITMAP_OPTIONS(3), nullptr};
    hresult_test(d2d::context()->CreateBitmapFromDxgiSurface, surface.get(), &bp, &sp->bitmap.get());
    return make_handle<bitmap>(sp->id);
  }

  bitmap(IDXGISwapChain1* SwapChain, const source_line& sl = here()) {
    if (auto res = create(SwapChain); !res) res.error().go_off(sl);
    else *this = std::move(res.value());
  }
};

class bitmap {
public:
  static constexpr auto dxgiformat = DXGI_FORMAT_B8G8R8A8_UNORM;
  static constexpr auto pixelformat = D2D1_PIXEL_FORMAT(dxgiformat, D2D1_ALPHA_MODE_PREMULTIPLIED);
  static constexpr auto properties = D2D1_BITMAP_PROPERTIES1(pixelformat, 96.0f, 96.0f, D2D1_BITMAP_OPTIONS_TARGET);

private:
  comptr<ID2D1Bitmap1> _bitmap;
  uint2 _size{};

  std::expected<void, error> initialize(uint2 sz) {
    hresult_test(d2d::context()->CreateBitmap, D2D1_SIZE_U{sz.x, sz.y}, nullptr, 0, &properties, &_bitmap.get());
    _size = sz;
    return {};
  }

  std::expected<void, error> initialize(const std::filesystem::path& p) {
    comptr<IWICBitmapDecoder> decoder;
    const auto option = WICDecodeMetadataCacheOnLoad;
    hresult_test(wic::factory()->CreateDecoderFromFilename, p.c_str(), nullptr, GENERIC_READ, option, &decoder.get());
    comptr<IWICBitmapFrameDecode> frame;
    hresult_test(decoder->GetFrame, 0, &frame.get());
    comptr<IWICFormatConverter> converter;
    hresult_test(wic::factory()->CreateFormatConverter, &converter.get());
    hresult_test(
      converter->Initialize, frame.get(), GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, nullptr, 0.0,
      WICBitmapPaletteTypeMedianCut);
    hresult_test(converter->GetSize, &_size.x, &_size.y);
    hresult_test(d2d::context()->CreateBitmapFromWicBitmap, converter.get(), &properties, &_bitmap.get());
    return {};
  }

  std::expected<void, error> initialize(IDXGISwapChain1* sc) {
    if (!sc) return std::unexpected(error(errors::invalid_argument, "null swapchain"));
    DXGI_SWAP_CHAIN_DESC1 scdesc{};
    hresult_test(sc->GetDesc1, &scdesc);
    _size = uint2{scdesc.Width, scdesc.Height};
    comptr<IDXGISurface> surface;
    hresult_test(sc->GetBuffer, 0, __uuidof(IDXGISurface), reinterpret_cast<void**>(&surface.get()));
    D2D1_BITMAP_PROPERTIES1 bp{pixelformat, 96.0f, 96.0f, D2D1_BITMAP_OPTIONS(3), nullptr};
    hresult_test(d2d::context()->CreateBitmapFromDxgiSurface, surface.get(), &bp, &_bitmap.get());
    return {};
  }

  std::expected<void, error> initialize(ID2D1Bitmap1* bmp) {
    if (!bmp) return std::unexpected(error(errors::invalid_argument, "null bitmap"));
    const auto [width, height] = bmp->GetPixelSize();
    _size = uint2{width, height};
    hresult_test(d2d::context()->CreateBitmap, D2D1_SIZE_U{width, height}, nullptr, 0, &properties, &_bitmap.get());
    D2D1_RECT_U rect{0, 0, width, height};
    D2D1_POINT_2U pt{0, 0};
    hresult_test(_bitmap->CopyFromBitmap, &pt, bmp, &rect);
    return {};
  }

public:
  bitmap() = default;
  explicit operator bool() const noexcept { return static_cast<bool>(_bitmap); }
  explicit operator ID2D1Bitmap1*&() & noexcept { return _bitmap.get(); }
  explicit operator ID2D1Bitmap1*() const& noexcept { return _bitmap.get(); }
  ID2D1Bitmap1* d2d_bitmap() const noexcept { return _bitmap.get(); }

  /// creates empty bitmap with specified size
  bitmap(uint2 Size, const source_line& sl = here()) {
    if (auto res = initialize(Size); !res) res.error().go_off(sl);
  }

  /// creates bitmap from image file
  bitmap(const std::filesystem::path& p, const source_line& sl = here()) {
    if (auto res = initialize(p); !res) res.error().go_off(sl);
  }

  /// creates bitmap for rendertarget from swapchain
  bitmap(IDXGISwapChain1* swapchain, const source_line& sl = here()) {
    if (auto res = initialize(swapchain); !res) res.error().go_off(sl);
  }

  /// copies bitmap from another
  bitmap(ID2D1Bitmap1* Bitmap, const source_line& sl = here()) {
    if (auto res = initialize(Bitmap); !res) res.error().go_off(sl);
  }

  template<typename... As> requires constructible<bitmap, As...>
  static std::expected<bitmap, error> create(As&&... Args) {
    bitmap b;
    if (auto res = b.initialize(static_cast<As&&>(Args)...)) return b;
    else return res.error().relay();
  }

  uint2 size() const noexcept { return _size; }

  drawing begin_draw(const source_line& sl) {
    if (!*this) error(errors::invalid_operation, "drawing on uninitialized bitmap").go_off(sl);
    return drawing(_bitmap.get(), sl);
  }

  drawing begin_draw(const color& clear_color, const source_line& sl) {
    if (!*this) error(errors::invalid_operation, "drawing on uninitialized bitmap").go_off(sl);
    auto d = drawing(_bitmap.get(), sl);
    d2d::context()->Clear(reinterpret_cast<const D2D1::ColorF*>(&clear_color));
    return d;
  }

  std::expected<drawing, error> begin_draw() {
    if (!*this) return std::unexpected(error(errors::invalid_operation, "drawing on uninitialized bitmap"));
    if (auto res = drawing::create(_bitmap.get()); !res) return res.error().relay();
    else return std::move(*res);
  }

  std::expected<drawing, error> begin_draw(const color& clear_color) {
    if (!*this) return std::unexpected(error(errors::invalid_operation, "drawing on uninitialized bitmap"));
    if (auto res = drawing::create(_bitmap.get())) {
      d2d::context()->Clear(reinterpret_cast<const D2D1::ColorF*>(&clear_color));
      return std::move(*res);
    } else return res.error().relay();
  }

  std::expected<void, error> save_as(const std::filesystem::path& p, const GUID& FileFormat) const {
    if (!*this) return std::unexpected(error(errors::not_initialized));
    comptr<IWICStream> stream;
    hresult_test(wic::factory()->CreateStream, &stream.get());
    hresult_test(stream->InitializeFromFilename, p.c_str(), GENERIC_WRITE);
    comptr<IWICBitmapEncoder> encoder;
    hresult_test(wic::factory()->CreateEncoder, FileFormat, nullptr, &encoder.get());
    hresult_test(encoder->Initialize, stream.get(), WICBitmapEncoderNoCache);
    comptr<IWICBitmapFrameEncode> frame;
    hresult_test(encoder->CreateNewFrame, &frame.get(), nullptr);
    hresult_test(frame->Initialize, nullptr);
    comptr<IWICImageEncoder> image_encoder;
    hresult_test(wic::factory()->CreateImageEncoder, d2d::device(), &image_encoder.get());
    hresult_test(image_encoder->WriteFrame, _bitmap.get(), frame.get(), nullptr);
    hresult_test(frame->Commit);
    hresult_test(encoder->Commit);
    hresult_test(stream->Commit, STGC_DEFAULT);
    return {};
  }

  std::expected<void, error> save_as_png(const std::filesystem::path& p) const {
    if (auto res = save_as(p, GUID_ContainerFormatPng)) return {};
    else return res.error().relay();
  }
  std::expected<void, error> save_as_jpeg(const std::filesystem::path& p) const {
    if (auto res = save_as(p, GUID_ContainerFormatJpeg)) return {};
    else return res.error().relay();
  }
};

//////////////////////////////////////// MARK: draw_bitmap

inline std::expected<void, error> draw_bitmap(float2 pos, float2 size, const bitmap& b, float1 opacity = 1.0f) {
  const auto d2d = yw::d2d();
  if (!drawing::d2d_drawing()) return std::unexpected(error(errors::invalid_operation, "drawing not begun"));
  if (!b) return {};
  D2D1_RECT_F rect = D2D1::RectF(pos.x, pos.y, pos.x + size.x, pos.y + size.y);
  d2d.context()->DrawBitmap((ID2D1Bitmap1*)b, &rect, opacity.x);
  return {};
}

inline std::expected<void, error> draw_bitmap(float2 pos, const bitmap& b, float1 opacity = 1.0f) {
  const auto d2d = yw::d2d();
  if (!drawing::d2d_drawing()) return std::unexpected(error(errors::invalid_operation, "drawing not begun"));
  if (!b) return {};
  D2D1_RECT_F rect = D2D1::RectF(pos.x, pos.y, pos.x + b.size().x, pos.y + b.size().y);
  d2d.context()->DrawBitmap((ID2D1Bitmap1*)b, &rect, opacity.x);
  return {};
}
} // namespace yw
