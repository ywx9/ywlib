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
    float opacity = 1.0f;
  };

private:
  template<typename T> static constexpr bool _memory_byte = sizeof(remove_cv<T>) == 1 && !char_type<T>;

  static std::expected<bitmap, error> _create_from_decoder(IWICBitmapDecoder* decoder) {
    if (!decoder) return std::unexpected(error(errors::invalid_argument, "null decoder"));
    const auto sp = make_slot<bitmap>();
    if (!sp) return std::unexpected(error(errors::slot_creation_failed, "failed to create bitmap slot"));
    const auto wicf = wic::factory();
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

  static std::expected<bitmap, error> _create_from_memory(const void* Data, size_t Bytes) {
    if (!Data || Bytes == 0) return std::unexpected(error(errors::invalid_argument, "empty bitmap data"));
    if (Bytes > DWORD(-1)) return std::unexpected(error(errors::invalid_argument, "bitmap data is too large"));
    const auto wicf = wic::factory();
    comptr<IWICStream> stream;
    hresult_test(wicf->CreateStream, &stream.get());
    hresult_test(stream->InitializeFromMemory, static_cast<BYTE*>(const_cast<void*>(Data)), DWORD(Bytes));
    comptr<IWICBitmapDecoder> decoder;
    hresult_test(wicf->CreateDecoderFromStream, stream.get(), nullptr, WICDecodeMetadataCacheOnLoad, &decoder.get());
    return _create_from_decoder(decoder.get());
  }

  static std::expected<bitmap, error> _create_from_pixels(uint2 Size, const bgra* Pixels, size_t Count) {
    if (Size.x == 0 || Size.y == 0) return std::unexpected(error(errors::invalid_argument, "invalid bitmap size"));
    const auto expected = size_t(Size.x) * size_t(Size.y);
    if (Count != expected)
      return std::unexpected(error(errors::invalid_argument, "pixel count does not match bitmap size"));
    if (!Pixels) return std::unexpected(error(errors::invalid_argument, "null pixel data"));
    if (Size.x > UINT32_MAX / sizeof(bgra))
      return std::unexpected(error(errors::invalid_argument, "bitmap row pitch is too large"));
    const auto sp = make_slot<bitmap>();
    if (!sp) return std::unexpected(error(errors::slot_creation_failed, "failed to create bitmap slot"));
    const auto pitch = UINT32(Size.x * sizeof(bgra));
    hresult_test(d2d::context()->CreateBitmap, D2D1_SIZE_U{Size.x, Size.y}, Pixels, pitch, &props, &sp->bitmap.get());
    sp->size = Size;
    return make_handle<bitmap>(sp->id);
  }

public:
  bitmap() noexcept = default;
  bitmap(bitmap&&) noexcept = default;
  bitmap& operator=(bitmap&&) noexcept = default;

  explicit operator bool() const noexcept {
    const auto sp = slot::get_as<bitmap>(id());
    return sp && static_cast<bool>(sp->bitmap);
  }

  auto d2d_bitmap(this auto&& self) noexcept -> ID2D1Bitmap1* {
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
  explicit bitmap(uint2 Size, const source_line& sl = here()) {
    if (auto res = create(Size); !res) res.error().go_off(sl);
    else *this = std::move(*res);
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
  explicit bitmap(const bitmap& Other, const source_line& sl = here()) {
    if (auto res = create(Other); !res) res.error().go_off(sl);
    else *this = std::move(*res);
  }

  /// creates bitmap from file.
  static std::expected<bitmap, error> create_from_file(stringable auto&& Path) {
    comptr<IWICBitmapDecoder> decoder;
    const auto option = WICDecodeMetadataCacheOnLoad;
    const auto wicf = wic::factory();
    const auto path = null_terminated<wchar_t>(static_cast<decltype(Path)&&>(Path));
    hresult_test(wicf->CreateDecoderFromFilename, path.c_str(), nullptr, GENERIC_READ, option, &decoder.get());
    return _create_from_decoder(decoder.get());
  }

  /// creates bitmap from file.
  explicit bitmap(stringable auto&& Path, const source_line& sl = here()) {
    if (auto res = create_from_file(static_cast<decltype(Path)&&>(Path)); !res) res.error().go_off(sl);
    else *this = std::move(*res);
  }

  /// creates bitmap from encoded image data in memory.
  static std::expected<bitmap, error> create_from_file_data(
    contiguous_iterator<std::byte> auto First, sized_sentinel_for<decltype(First)> auto Last)
    requires _memory_byte<iter_value_t<decltype(First)>> {
    const auto count = Last - First;
    if (count < 0) return std::unexpected(error(errors::invalid_argument, "invalid bitmap data range"));
    return _create_from_memory(std::to_address(First), size_t(count));
  }

  /// creates bitmap from encoded image data in memory.
  static std::expected<bitmap, error> create_from_file_data(contiguous_range<std::byte> auto&& Data)
    requires _memory_byte<iter_value_t<decltype(Data)>> {
    return _create_from_memory(yw::data(Data), yw::size(Data));
  }

  /// creates bitmap from encoded image data in memory.
  explicit bitmap(contiguous_range<std::byte> auto&& Data, const source_line& sl = here())
    requires _memory_byte<iter_value_t<decltype(Data)>> {
    if (auto res = create_from_file_data(static_cast<decltype(Data)&&>(Data)); !res) res.error().go_off(sl);
    else *this = std::move(*res);
  }

  /// creates bitmap from premultiplied BGRA pixels.
  static std::expected<bitmap, error> create_from_pixels(
    uint2 Size, contiguous_iterator<bgra> auto First, sized_sentinel_for<decltype(First)> auto Last) {
    const auto count = Last - First;
    if (count < 0) return std::unexpected(error(errors::invalid_argument, "invalid pixel data range"));
    return _create_from_pixels(Size, std::to_address(First), size_t(count));
  }

  /// creates bitmap from premultiplied BGRA pixels.
  static std::expected<bitmap, error> create_from_pixels(uint2 Size, contiguous_range<bgra> auto&& Pixels) {
    return _create_from_pixels(Size, yw::data(Pixels), yw::size(Pixels));
  }

  /// creates bitmap from premultiplied BGRA pixels.
  explicit bitmap(
    uint2 Size, contiguous_iterator<bgra> auto First, sized_sentinel_for<decltype(First)> auto Last,
    const source_line& sl = here()) {
    if (auto res = create_from_pixels(Size, First, Last); !res) res.error().go_off(sl);
    else *this = std::move(*res);
  }

  /// creates bitmap from premultiplied BGRA pixels.
  explicit bitmap(uint2 Size, contiguous_range<bgra> auto&& Pixels, const source_line& sl = here()) {
    if (auto res = create_from_pixels(Size, static_cast<decltype(Pixels)&&>(Pixels)); !res) res.error().go_off(sl);
    else *this = std::move(*res);
  }

  /// creates bitmap from swapchain.
  static std::expected<bitmap, error> create_from_swapchain(IDXGISwapChain1* SwapChain) {
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

  // bitmap(IDXGISwapChain1* SwapChain) = delete; // force to use create_from_swapchain()

  /// creates bitmap from d2d bitmap.
  static std::expected<bitmap, error> create_from_d2d_bitmap(ID2D1Bitmap* Bitmap) {
    if (!Bitmap) return std::unexpected(error(errors::invalid_argument, "null bitmap"));
    const auto sp = make_slot<bitmap>();
    if (!sp) return std::unexpected(error(errors::invalid_operation, "failed to create bitmap slot"));
    const auto size = Bitmap->GetPixelSize();
    sp->size = {size.width, size.height};
    hresult_test(d2d::context()->CreateBitmap, size, nullptr, 0, &props, &sp->bitmap.get());
    hresult_test(sp->bitmap->CopyFromBitmap, nullptr, Bitmap, nullptr);
    return make_handle<bitmap>(sp->id);
  }

  // bitmap(ID2D1Bitmap* Bitmap) = delete; // force to use create_from_d2d_bitmap()

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

  std::expected<void, error> copy_to_cpu(bgra* o) const {
    if (!o) return std::unexpected(error(errors::invalid_argument, "null output buffer"));
    const auto sp = slot::get_as<bitmap>(id());
    if (!sp || !sp->bitmap) return std::unexpected(error(errors::not_initialized, "bitmap not initialized"));
    const auto width = sp->size.x;
    const auto height = sp->size.y;
    if (width == 0 || height == 0) return std::unexpected(error(errors::invalid_argument, "invalid bitmap size"));

    comptr<ID2D1Bitmap1> cpu_bmp;
    D2D1_BITMAP_PROPERTIES1 props{
      pixelformat, 96.0f, 96.0f,
      D2D1_BITMAP_OPTIONS(D2D1_BITMAP_OPTIONS_CPU_READ | D2D1_BITMAP_OPTIONS_CANNOT_DRAW), nullptr};
    hresult_test(d2d::context()->CreateBitmap, D2D1_SIZE_U{width, height}, nullptr, 0, &props, &cpu_bmp.get());
    hresult_test(cpu_bmp->CopyFromBitmap, nullptr, sp->bitmap.get(), nullptr);

    D2D1_MAPPED_RECT mapped{};
    hresult_test(cpu_bmp->Map, D2D1_MAP_OPTIONS_READ, &mapped);
    const auto src_stride = mapped.pitch;
    const auto dst_stride = size_t(width) * sizeof(bgra);
    for (uint32_t y = 0; y < height; ++y)
      std::memcpy(reinterpret_cast<std::byte*>(o) + dst_stride * y, mapped.bits + src_stride * y, dst_stride);
    hresult_test(cpu_bmp->Unmap);
    return {};
  }

  std::vector<bgra> copy_to_cpu(const source_line& sl = here()) const {
    std::vector<bgra> pixels(size_t(size().x) * size_t(size().y));
    if (auto res = copy_to_cpu(pixels.data()); !res) {
      res.error().add_footprint().fizzle_out(sl);
      return {};
    }
    return pixels;
  }

  std::expected<void, error> save_as(null_terminated<wchar_t> Path, const GUID& Format) const {
    const auto sp = slot::get_as<bitmap>(id());
    if (!sp) return std::unexpected(error(errors::invalid_slotid));
    comptr<IWICStream> stream;
    hresult_test(wic::factory()->CreateStream, &stream.get());
    hresult_test(stream->InitializeFromFilename, Path.data(), GENERIC_WRITE);
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

  std::expected<void, error> save_as_png(null_terminated<wchar_t> Path) const {
    if (auto res = save_as(std::move(Path), GUID_ContainerFormatPng)) return {};
    else return res.error().relay();
  }

  std::expected<void, error> save_as_jpeg(null_terminated<wchar_t> Path) const {
    if (auto res = save_as(std::move(Path), GUID_ContainerFormatJpeg)) return {};
    else return res.error().relay();
  }

  //-- getter --//

  float opacity() const noexcept {
    if (const auto sp = slot::get_as<bitmap>(id()); !sp) {
      error(errors::invalid_slotid, "invalid bitmap").fizzle_out();
      return 0.0f;
    } else return sp->opacity;
  }

  //-- setter --//

  auto& opacity(this auto& self, float1 o) noexcept {
    if (const auto sp = slot::get_as<bitmap>(self.id())) sp->opacity = o.x;
    else error(errors::invalid_slotid, "invalid bitmap").fizzle_out();
    return self;
  }
};

/// MARK: draw_bitmap

inline std::expected<void, error> draw_bitmap(
  float2 Pos, float2 Size, castable_to<ID2D1Bitmap*> auto&& b, float1 Opacity = 1.0f) {
  if (!drawing::d2d_drawing()) return std::unexpected(error(errors::invalid_operation, "drawing not begun"));
  D2D1_RECT_F rect(Pos.x, Pos.y, Pos.x + Size.x, Pos.y + Size.y);
  d2d::context()->DrawBitmap(static_cast<ID2D1Bitmap*>(b), &rect, Opacity.x);
  return {};
}

inline std::expected<void, error> draw_bitmap(float2 Pos, castable_to<ID2D1Bitmap*> auto&& b, float1 Opacity = 1.0f) {
  if (!drawing::d2d_drawing()) return std::unexpected(error(errors::invalid_operation, "drawing not begun"));
  const auto size = static_cast<ID2D1Bitmap*>(b)->GetPixelSize();
  D2D1_RECT_F rect = D2D1::RectF(Pos.x, Pos.y, Pos.x + size.width, Pos.y + size.height);
  d2d::context()->DrawBitmap(static_cast<ID2D1Bitmap*>(b), &rect, Opacity.x);
  return {};
}

inline std::expected<void, error> draw_bitmap(float2 Pos, float2 Size, const bitmap& b) {
  if (const auto sp = bitmap::slot::get_as<bitmap>(b.id())) {
    if (auto res = draw_bitmap(Pos, Size, sp->bitmap.get(), sp->opacity); !res) res.error().go_off();
  } else return std::unexpected(error(errors::invalid_argument, "invalid bitmap"));
  return {};
}

inline std::expected<void, error> draw_bitmap(float2 Pos, const bitmap& b) {
  if (!drawing::d2d_drawing()) return std::unexpected(error(errors::invalid_operation, "drawing not begun"));
  if (const auto sp = bitmap::slot::get_as<bitmap>(b.id())) {
    if (auto res = draw_bitmap(Pos, sp->size, sp->bitmap.get(), sp->opacity); !res) res.error().go_off();
  } else return std::unexpected(error(errors::invalid_argument, "invalid bitmap"));
  return {};
}
} // namespace yw
