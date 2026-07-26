#pragma once
#include <ywx/drawing.h>

namespace yw {

class bitmap : public handle_base {
public:
  static constexpr auto dxgiformat = DXGI_FORMAT_B8G8R8A8_UNORM;
  static constexpr auto pixelformat = D2D1_PIXEL_FORMAT(dxgiformat, D2D1_ALPHA_MODE_PREMULTIPLIED);
  static constexpr auto props = D2D1_BITMAP_PROPERTIES1(pixelformat, 96.0f, 96.0f, D2D1_BITMAP_OPTIONS_TARGET);

  struct slot : handle_base::slot {
    comptr<ID2D1Bitmap1> bitmap;
    uint2 size{};
  };

  bitmap() noexcept = default;
  bitmap(bitmap&&) noexcept = default;
  bitmap& operator=(bitmap&&) noexcept = default;

  explicit operator bool() const noexcept {
    const auto sp = slot::get_as<bitmap>(id());
    return sp && static_cast<bool>(sp->bitmap);
  }

  auto d2d_bitmap(this auto&& self) noexcept -> copy_cv<remove_ref<decltype(self)>, ID2D1Bitmap1>* {
    if (const auto sp = slot::get_as<bitmap>(self.id())) return sp->bitmap.get();
    else return nullptr;
  }

  /// creates empty bitmap.
  static std::expected<bitmap, error> create(uint2 Size) {
    const auto sp = make_slot<bitmap>();
    if (!sp) return std::unexpected(error(errors::slot_creation_failed, "failed to create bitmap slot"));
    hresult_test(d2d::context()->CreateBitmap, {Size.x, Size.y}, nullptr, 0, &props, &sp->bitmap.get());
    sp->size = Size;
    return make_handle<bitmap>(sp->id);
  }

  /// creates empty bitmap.
  bitmap(uint2 Size, const source_line& sl = here()) {
    if (auto res = create(Size); !res) res.error().go_off(sl);
    else *this = std::move(res.value());
  }

  /// creates bitmap by copying from another.
  static std::expected<bitmap, error> create(const bitmap& Other) {
    const auto sp = make_slot<bitmap>();
    if (!sp) return std::unexpected(error(errors::slot_creation_failed, "failed to create bitmap slot"));
    const auto osp = slot::get_as<bitmap>(Other.id());
    if (!osp) return std::unexpected(error(errors::invalid_argument, "invalid bitmap"));
    hresult_test(d2d::context()->CreateBitmap, {osp->size.x, osp->size.y}, nullptr, 0, &props, &sp->bitmap.get());
    sp->size = osp->size;
    hresult_test(sp->bitmap->CopyFromBitmap, nullptr, osp->bitmap.get(), nullptr);
    return make_handle<bitmap>(sp->id);
  }

  /// creates bitmap by copying from another.
  bitmap(const bitmap& Other, const source_line& sl = here()) {
    if (auto res = create(Other); !res) res.error().go_off(sl);
    else *this = std::move(res.value());
  }

  /// creates bitmap from file.
  static std::expected<bitmap, error> create(const std::filesystem::path& Path) {
    const auto sp = make_slot<bitmap>();
    if (!sp) return std::unexpected(error(errors::slot_creation_failed, "failed to create bitmap slot"));
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
    hresult_test(d2d::context()->CreateBitmapFromWicBitmap, converter.get(), &props, &sp->bitmap.get());
    return make_handle<bitmap>(sp->id);
  }

  /// creates bitmap from file.
  bitmap(const std::filesystem::path& Path, const source_line& sl = here()) {
    if (auto res = create(Path); !res) res.error().go_off(sl);
    else *this = std::move(res.value());
  }

  /// creates bitmap from swapchain.
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

  // bitmap(IDXGISwapChain1* SwapChain) = delete; // force to use create()

  /// creates bitmap from d2d bitmap.
  static std::expected<bitmap, error> create(ID2D1Bitmap* Bitmap) {
    if (!Bitmap) return std::unexpected(error(errors::invalid_argument, "null bitmap"));
    const auto sp = make_slot<bitmap>();
    if (!sp) return std::unexpected(error(errors::invalid_operation, "failed to create bitmap slot"));
    const auto size = Bitmap->GetPixelSize();
    sp->size = {size.width, size.height};
    hresult_test(d2d::context()->CreateBitmap, size, nullptr, 0, &props, &sp->bitmap.get());
    hresult_test(sp->bitmap->CopyFromBitmap, nullptr, Bitmap, nullptr);
    return make_handle<bitmap>(sp->id);
  }

  // bitmap(ID2D1Bitmap* Bitmap) = delete; // force to use create()

  uint2 size() const noexcept {
    if (const auto sp = slot::get_as<bitmap>(id())) return sp->size;
    else return {};
  }

  std::expected<drawing, error> begin_draw() {
    if (const auto sp = slot::get_as<bitmap>(id()); !sp) return std::unexpected(error(errors::invalid_slotid));
    else if (auto res = drawing::create(sp->bitmap.get())) return std::move(*res);
    else return res.error().relay();
  }

  std::expected<drawing, error> begin_draw(const color& ClearColor) {
    if (const auto sp = slot::get_as<bitmap>(id()); !sp) return std::unexpected(error(errors::invalid_slotid));
    else if (auto res = drawing::create(sp->bitmap.get())) {
      static_assert(sizeof(D2D1_COLOR_F) == sizeof(color));
      d2d::context()->Clear(reinterpret_cast<const D2D1_COLOR_F*>(&ClearColor));
      return std::move(*res);
    } else return res.error().relay();
  }

  std::expected<void, error> save_as(const path& Path, const GUID& Format) const {
    const auto sp = slot::get_as<bitmap>(id());
    if (!sp) return std::unexpected(error(errors::invalid_slotid));
    comptr<IWICStream> stream;
    hresult_test(wic::factory()->CreateStream, &stream.get());
    hresult_test(stream->InitializeFromFilename, Path.c_str(), GENERIC_WRITE);
    comptr<IWICBitmapEncoder> encoder;
    hresult_test(wic::factory()->CreateEncoder, Format, nullptr, &encoder.get());
    hresult_test(encoder->Initialize, stream.get(), WICBitmapEncoderNoCache);
    comptr<IWICBitmapFrameEncode> frame;
    hresult_test(encoder->CreateNewFrame, &frame.get(), nullptr);
    hresult_test(frame->Initialize, nullptr);
    comptr<IWICImageEncoder> image_encoder;
    hresult_test(wic::factory()->CreateImageEncoder, d2d::device(), &image_encoder.get());
    hresult_test(image_encoder->WriteFrame, sp->bitmap.get(), frame.get(), nullptr);
    hresult_test(frame->Commit);
    hresult_test(encoder->Commit);
    hresult_test(stream->Commit, STGC_DEFAULT);
    return {};
  }

  std::expected<void, error> save_as_png(const path& Path) const {
    if (auto res = save_as(Path, GUID_ContainerFormatPng)) return {};
    else return res.error().relay();
  }

  std::expected<void, error> save_as_jpeg(const path& Path) const {
    if (auto res = save_as(Path, GUID_ContainerFormatJpeg)) return {};
    else return res.error().relay();
  }
};

/// MARK: draw_bitmap

inline std::expected<void, error> draw_bitmap(
  float2 Pos, float2 Size, const bitmap& b, float1 Opacity = 1.0f, const source_line& sl = here()) {
  if (!drawing::d2d_drawing()) return std::unexpected(error(errors::invalid_operation, "drawing not begun"));
  if (const auto sp = bitmap::slot::get_as<bitmap>(b.id())) {
    D2D1_RECT_F rect(Pos.x, Pos.y, Pos.x + Size.x, Pos.y + Size.y);
    d2d::context()->DrawBitmap(sp->bitmap.get(), &rect, Opacity.x);
  } else error(errors::invalid_argument, "invalid bitmap").fizzle_out(sl);
  return {};
}

inline std::expected<void, error> draw_bitmap(float2 Pos, const bitmap& b, float1 Opacity = 1.0f, const source_line& sl = here()) {
  if (!drawing::d2d_drawing()) return std::unexpected(error(errors::invalid_operation, "drawing not begun"));
  if (const auto sp = bitmap::slot::get_as<bitmap>(b.id())) {
    D2D1_RECT_F rect = D2D1::RectF(Pos.x, Pos.y, Pos.x + b.size().x, Pos.y + b.size().y);
    d2d::context()->DrawBitmap(sp->bitmap.get(), &rect, Opacity.x);
  } else error(errors::invalid_argument, "invalid bitmap").fizzle_out(sl);
  return {};
}
} // namespace yw
