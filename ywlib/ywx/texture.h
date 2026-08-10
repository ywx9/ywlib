#pragma once
#include <ywx/bitmap.h>

namespace yw {

class texture_base : public handle_base {
public:
  struct slot : handle_base::slot {
    uint2 size{};
    comptr<ID3D11Texture2D> texture;
    DXGI_FORMAT dxgiformat = DXGI_FORMAT_UNKNOWN;
  };

protected:
  static constexpr size_t dxgi_format_bytes(DXGI_FORMAT Format) noexcept {
    switch (Format) {
    case DXGI_FORMAT_R8_UNORM:
    case DXGI_FORMAT_R8_UINT:
    case DXGI_FORMAT_R8_SINT:
      return 1;
    case DXGI_FORMAT_R16_FLOAT:
    case DXGI_FORMAT_R16_UNORM:
    case DXGI_FORMAT_R16_UINT:
    case DXGI_FORMAT_R16_SINT:
      return 2;
    case DXGI_FORMAT_B8G8R8A8_UNORM:
    case DXGI_FORMAT_R8G8B8A8_UNORM:
    case DXGI_FORMAT_R8G8B8A8_UINT:
    case DXGI_FORMAT_R8G8B8A8_SINT:
    case DXGI_FORMAT_R16G16_FLOAT:
    case DXGI_FORMAT_R16G16_UNORM:
    case DXGI_FORMAT_R16G16_UINT:
    case DXGI_FORMAT_R16G16_SINT:
    case DXGI_FORMAT_R32_FLOAT:
    case DXGI_FORMAT_R32_UINT:
    case DXGI_FORMAT_R32_SINT:
    case DXGI_FORMAT_D32_FLOAT:
    case DXGI_FORMAT_R32_TYPELESS:
      return 4;
    case DXGI_FORMAT_R16G16B16A16_FLOAT:
    case DXGI_FORMAT_R16G16B16A16_UNORM:
    case DXGI_FORMAT_R16G16B16A16_UINT:
    case DXGI_FORMAT_R16G16B16A16_SINT:
    case DXGI_FORMAT_R32G32_FLOAT:
    case DXGI_FORMAT_R32G32_UINT:
    case DXGI_FORMAT_R32G32_SINT:
      return 8;
    case DXGI_FORMAT_R32G32B32A32_FLOAT:
    case DXGI_FORMAT_R32G32B32A32_UINT:
    case DXGI_FORMAT_R32G32B32A32_SINT:
      return 16;
    default:
      return 0;
    }
  }

  std::expected<void, error> copy_to_cpu_impl(void* Out) const {
    if (!Out) return std::unexpected(error(errors::invalid_argument, "null output buffer"));
    const auto sp = get_slot(this);
    if (!sp || !sp->texture) return std::unexpected(error(errors::not_initialized, "texture not initialized"));
    const auto pixel_size = dxgi_format_bytes(sp->dxgiformat);
    if (pixel_size == 0) return std::unexpected(error(errors::invalid_argument, "unsupported texture format"));
    if (sp->size.x == 0 || sp->size.y == 0)
      return std::unexpected(error(errors::invalid_argument, "invalid texture size"));

    D3D11_TEXTURE2D_DESC desc{
      sp->size.x, sp->size.y, 1, 1, sp->dxgiformat, DXGI_SAMPLE_DESC{1, 0}, D3D11_USAGE_STAGING, 0,
      D3D11_CPU_ACCESS_READ};
    comptr<ID3D11Texture2D> staging;
    hresult_test(d3d::device()->CreateTexture2D, &desc, nullptr, &staging.get());
    d3d::context()->CopyResource(staging.get(), sp->texture.get());

    D3D11_MAPPED_SUBRESOURCE mapped{};
    hresult_test(d3d::context()->Map, staging.get(), 0, D3D11_MAP_READ, 0, &mapped);
    const auto dst_stride = size_t(sp->size.x) * pixel_size;
    for (uint32_t y = 0; y < sp->size.y; ++y)
      std::memcpy(static_cast<std::byte*>(Out) + dst_stride * y,
        static_cast<const std::byte*>(mapped.pData) + size_t(mapped.RowPitch) * y, dst_stride);
    d3d::context()->Unmap(staging.get(), 0);
    return {};
  }

public:
  explicit operator bool() const noexcept {
    const auto sp = get_slot(this);
    return sp && bool(sp->texture);
  }

  explicit operator ID3D11Texture2D*() const noexcept { return d3d_texture(); }

  ID3D11Texture2D* d3d_texture() const noexcept {
    const auto sp = get_slot(this);
    return sp ? sp->texture.get() : nullptr;
  }

  uint2 size() const noexcept {
    const auto sp = get_slot(this);
    return sp ? sp->size : uint2{};
  }

  DXGI_FORMAT format() const noexcept {
    const auto sp = get_slot(this);
    return sp ? sp->dxgiformat : DXGI_FORMAT_UNKNOWN;
  }

  std::expected<void, error> copy_to_cpu(void* Out) const { return copy_to_cpu_impl(Out); }
};

class texture : public texture_base {
public:
  enum class flag : uint8_t {
    none = 0,
    shader_resource = 0x1,
    render_target = 0x2,
    unordered_access = 0x4,
  };
  friend constexpr flag operator~(flag f) noexcept { return flag(~uint8_t(f)); }
  friend constexpr flag operator|(flag lhs, flag rhs) noexcept { return flag(uint8_t(lhs) | uint8_t(rhs)); }
  friend constexpr flag operator&(flag lhs, flag rhs) noexcept { return flag(uint8_t(lhs) & uint8_t(rhs)); }
  friend constexpr flag operator^(flag lhs, flag rhs) noexcept { return flag(uint8_t(lhs) ^ uint8_t(rhs)); }

  struct slot : texture_base::slot {
    comptr<ID3D11ShaderResourceView> srv;
    comptr<ID3D11RenderTargetView> rtv;
    comptr<ID3D11UnorderedAccessView> uav;
    yw::flags<flag> flags;
  };

private:
  static std::expected<void, error> create_views(slot* sp) {
    if (sp->flags.contains(flag::shader_resource)) {
      D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc{sp->dxgiformat, D3D11_SRV_DIMENSION_TEXTURE2D};
      srv_desc.Texture2D.MipLevels = 1;
      hresult_test(d3d::device()->CreateShaderResourceView, sp->texture.get(), &srv_desc, &sp->srv.get());
    }
    if (sp->flags.contains(flag::render_target)) {
      D3D11_RENDER_TARGET_VIEW_DESC rtv_desc{sp->dxgiformat, D3D11_RTV_DIMENSION_TEXTURE2D};
      hresult_test(d3d::device()->CreateRenderTargetView, sp->texture.get(), &rtv_desc, &sp->rtv.get());
    }
    if (sp->flags.contains(flag::unordered_access)) {
      D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc{sp->dxgiformat, D3D11_UAV_DIMENSION_TEXTURE2D};
      hresult_test(d3d::device()->CreateUnorderedAccessView, sp->texture.get(), &uav_desc, &sp->uav.get());
    }
    return {};
  }

public:
  explicit operator ID3D11ShaderResourceView*() const noexcept { return d3d_srv(); }
  explicit operator ID3D11RenderTargetView*() const noexcept { return d3d_rtv(); }
  explicit operator ID3D11UnorderedAccessView*() const noexcept { return d3d_uav(); }

  ID3D11ShaderResourceView* d3d_srv() const noexcept {
    const auto sp = get_slot(this);
    return sp ? sp->srv.get() : nullptr;
  }

  ID3D11RenderTargetView* d3d_rtv() const noexcept {
    const auto sp = get_slot(this);
    return sp ? sp->rtv.get() : nullptr;
  }

  ID3D11UnorderedAccessView* d3d_uav() const noexcept {
    const auto sp = get_slot(this);
    return sp ? sp->uav.get() : nullptr;
  }

  yw::flags<flag> flags() const noexcept {
    const auto sp = get_slot(this);
    return sp ? sp->flags : yw::flags<flag>{};
  }

  texture() noexcept = default;

  static std::expected<texture, error> create(
    int2 Size, DXGI_FORMAT Format = bitmap::dxgiformat,
    yw::flags<flag> Flags = flag::shader_resource | flag::render_target) {
    if (Size.x <= 0 || Size.y <= 0) return std::unexpected(error(errors::invalid_argument, "invalid texture size"));
    const uint2 sz{Size};
    const auto sp = make_slot<texture>();
    if (!sp) return std::unexpected(error(errors::slot_creation_failed));
    sp->size = sz;
    sp->dxgiformat = Format;
    sp->flags = Flags;
    D3D11_TEXTURE2D_DESC tex_desc{sz.x, sz.y, 1, 1, Format, DXGI_SAMPLE_DESC{1, 0}};
    if (Flags.contains(flag::shader_resource)) tex_desc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
    if (Flags.contains(flag::render_target)) tex_desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
    if (Flags.contains(flag::unordered_access)) tex_desc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
    hresult_test(d3d::device()->CreateTexture2D, &tex_desc, nullptr, &sp->texture.get());
    if (auto res = create_views(sp); !res) return res.error().relay();
    return make_handle<texture>(sp->id);
  }

  explicit texture(
    int2 Size, DXGI_FORMAT Format = bitmap::dxgiformat,
    yw::flags<flag> Flags = flag::shader_resource | flag::render_target, const source_line& sl = here()) {
    if (auto res = create(Size, Format, Flags); !res) res.error().go_off(sl);
    else *this = std::move(*res);
  }

  std::expected<void, error> clear(const color& ClearColor = colors::transparent) {
    if (!d3d_rtv()) return std::unexpected(error(errors::invalid_operation, "texture is not a render target"));
    d3d::context()->ClearRenderTargetView(d3d_rtv(), &ClearColor.r);
    return {};
  }
};

class bitmap_texture : public texture_base {
public:
  struct slot : texture_base::slot {
    comptr<ID2D1Bitmap1> bitmap;
  };

  explicit operator ID2D1Bitmap*() const noexcept { return d2d_bitmap(); }
  explicit operator ID2D1Bitmap1*() const noexcept { return d2d_bitmap(); }

  ID2D1Bitmap1* d2d_bitmap() const noexcept {
    const auto sp = get_slot(this);
    return sp ? sp->bitmap.get() : nullptr;
  }

  bitmap_texture() noexcept = default;

  static std::expected<bitmap_texture, error> create(int2 Size) {
    if (Size.x <= 0 || Size.y <= 0)
      return std::unexpected(error(errors::invalid_argument, "invalid bitmap texture size"));
    const uint2 sz{Size};
    const auto sp = make_slot<bitmap_texture>();
    if (!sp) return std::unexpected(error(errors::slot_creation_failed));
    sp->size = sz;
    sp->dxgiformat = bitmap::dxgiformat;
    hresult_test(d2d::context()->CreateBitmap, {sz.x, sz.y}, nullptr, 0, &bitmap::props, &sp->bitmap.get());
    comptr<IDXGISurface> surface;
    hresult_test(sp->bitmap->GetSurface, &surface.get());
    hresult_test(surface->QueryInterface, &sp->texture.get());
    return make_handle<bitmap_texture>(sp->id);
  }

  static std::expected<bitmap_texture, error> create(bitmap&& Bitmap) {
    const auto bsp = bitmap::slot::get_as<bitmap>(Bitmap.id());
    if (!bsp || !bsp->bitmap) return std::unexpected(error(errors::invalid_argument, "invalid bitmap"));
    const auto sp = make_slot<bitmap_texture>();
    if (!sp) return std::unexpected(error(errors::slot_creation_failed));
    sp->size = bsp->size;
    sp->dxgiformat = bitmap::dxgiformat;
    sp->bitmap = std::move(bsp->bitmap);
    comptr<IDXGISurface> surface;
    hresult_test(sp->bitmap->GetSurface, &surface.get());
    hresult_test(surface->QueryInterface, &sp->texture.get());
    return make_handle<bitmap_texture>(sp->id);
  }

  static std::expected<bitmap_texture, error> create(const bitmap& Bitmap) {
    if (auto b = bitmap::create(Bitmap); !b) return b.error().relay();
    else return create(std::move(*b));
  }

  static std::expected<bitmap_texture, error> create(stringable auto&& Path) {
    if (auto b = bitmap::create_from_file(static_cast<decltype(Path)&&>(Path)); !b) return b.error().relay();
    else return create(std::move(*b));
  }

  static std::expected<bitmap_texture, error> create_from_file_data(
    contiguous_iterator<std::byte> auto First, sized_sentinel_for<decltype(First)> auto Last) {
    if (auto b = bitmap::create_from_file_data(First, Last); !b) return b.error().relay();
    else return create(std::move(*b));
  }

  static std::expected<bitmap_texture, error> create(contiguous_range<std::byte> auto&& Data) {
    if (auto b = bitmap::create_from_file_data(static_cast<decltype(Data)&&>(Data)); !b) return b.error().relay();
    else return create(std::move(*b));
  }

  explicit bitmap_texture(int2 Size, const source_line& sl = here()) {
    if (auto res = create(Size); !res) res.error().go_off(sl);
    else *this = std::move(*res);
  }

  explicit bitmap_texture(bitmap&& Bitmap, const source_line& sl = here()) {
    if (auto res = create(std::move(Bitmap)); !res) res.error().go_off(sl);
    else *this = std::move(*res);
  }

  explicit bitmap_texture(const bitmap& Bitmap, const source_line& sl = here()) {
    if (auto res = create(Bitmap); !res) res.error().go_off(sl);
    else *this = std::move(*res);
  }

  explicit bitmap_texture(stringable auto&& Path, const source_line& sl = here()) {
    if (auto res = create(static_cast<decltype(Path)&&>(Path)); !res) res.error().go_off(sl);
    else *this = std::move(*res);
  }

  explicit bitmap_texture(contiguous_range<std::byte> auto&& Data, const source_line& sl = here()) {
    if (auto res = create(static_cast<decltype(Data)&&>(Data)); !res) res.error().go_off(sl);
    else *this = std::move(*res);
  }

  std::expected<drawing, error> begin_draw() {
    if (!d2d_bitmap()) return std::unexpected(error(errors::invalid_operation, "bitmap texture is not initialized"));
    if (auto res = drawing::create(d2d_bitmap()); !res) return res.error().relay();
    else return std::move(*res);
  }

  std::expected<drawing, error> begin_draw(const color& ClearColor) {
    if (!d2d_bitmap()) return std::unexpected(error(errors::invalid_operation, "bitmap texture is not initialized"));
    if (auto res = drawing::create(d2d_bitmap())) {
      d2d::context()->Clear(reinterpret_cast<const D2D1_COLOR_F*>(&ClearColor));
      return std::move(*res);
    } else return res.error().relay();
  }
};

template<DXGI_FORMAT Format> class staging_texture : public texture_base {
public:
  struct slot : texture_base::slot {};

  staging_texture() noexcept = default;

  static std::expected<staging_texture, error> create(int2 Size) {
    if (Size.x <= 0 || Size.y <= 0)
      return std::unexpected(error(errors::invalid_argument, "invalid staging texture size"));
    if (dxgi_format_bytes(Format) == 0)
      return std::unexpected(error(errors::invalid_argument, "unsupported staging texture format"));
    const uint2 sz{Size};
    const auto sp = make_slot<staging_texture>();
    if (!sp) return std::unexpected(error(errors::slot_creation_failed));
    sp->size = sz;
    sp->dxgiformat = Format;
    D3D11_TEXTURE2D_DESC desc{sz.x, sz.y, 1, 1, Format, DXGI_SAMPLE_DESC{1, 0}, D3D11_USAGE_STAGING, 0,
      D3D11_CPU_ACCESS_READ};
    hresult_test(d3d::device()->CreateTexture2D, &desc, nullptr, &sp->texture.get());
    return make_handle<staging_texture>(sp->id);
  }

  explicit staging_texture(int2 Size, const source_line& sl = here()) {
    if (auto res = create(Size); !res) res.error().go_off(sl);
    else *this = std::move(*res);
  }

  std::expected<void, error> copy_from(const texture_base& Other) {
    if (!*this) return std::unexpected(error(errors::not_initialized, "staging texture not initialized"));
    if (!Other) return std::unexpected(error(errors::not_initialized, "source texture not initialized"));
    if (Other.format() != Format) return std::unexpected(error(errors::invalid_argument, "texture formats do not match"));
    if (Other.size() != size()) return std::unexpected(error(errors::invalid_argument, "texture sizes do not match"));
    d3d::context()->CopyResource(d3d_texture(), Other.d3d_texture());
    return {};
  }

  std::expected<void, error> copy_to_cpu(void* Out) const {
    if (!Out) return std::unexpected(error(errors::invalid_argument, "null output buffer"));
    const auto sp = get_slot(this);
    if (!sp || !sp->texture)
      return std::unexpected(error(errors::not_initialized, "staging texture not initialized"));
    const auto pixel_size = dxgi_format_bytes(Format);
    D3D11_MAPPED_SUBRESOURCE mapped{};
    hresult_test(d3d::context()->Map, sp->texture.get(), 0, D3D11_MAP_READ, 0, &mapped);
    const auto dst_stride = size_t(sp->size.x) * pixel_size;
    for (uint32_t y = 0; y < sp->size.y; ++y)
      std::memcpy(static_cast<std::byte*>(Out) + dst_stride * y,
        static_cast<const std::byte*>(mapped.pData) + size_t(mapped.RowPitch) * y, dst_stride);
    d3d::context()->Unmap(sp->texture.get(), 0);
    return {};
  }
};

/// MARK: depth_texture

class depth_texture : public texture_base {
public:
  enum class flag : uint8_t {
    none = 0,
    shader_resource = 0x1,
  };
  friend constexpr flag operator~(flag f) noexcept { return flag(~uint8_t(f)); }
  friend constexpr flag operator|(flag lhs, flag rhs) noexcept { return flag(uint8_t(lhs) | uint8_t(rhs)); }
  friend constexpr flag operator&(flag lhs, flag rhs) noexcept { return flag(uint8_t(lhs) & uint8_t(rhs)); }
  friend constexpr flag operator^(flag lhs, flag rhs) noexcept { return flag(uint8_t(lhs) ^ uint8_t(rhs)); }

  struct slot : texture_base::slot {
    comptr<ID3D11DepthStencilView> dsv;
    comptr<ID3D11ShaderResourceView> srv;
    yw::flags<flag> flags;
  };

  explicit operator ID3D11DepthStencilView*() const noexcept { return d3d_dsv(); }
  explicit operator ID3D11ShaderResourceView*() const noexcept { return d3d_srv(); }

  ID3D11DepthStencilView* d3d_dsv() const noexcept {
    const auto sp = get_slot(this);
    return sp ? sp->dsv.get() : nullptr;
  }

  ID3D11ShaderResourceView* d3d_srv() const noexcept {
    const auto sp = get_slot(this);
    return sp ? sp->srv.get() : nullptr;
  }

  yw::flags<flag> flags() const noexcept {
    const auto sp = get_slot(this);
    return sp ? sp->flags : yw::flags<flag>{};
  }

  depth_texture() noexcept = default;

  static std::expected<depth_texture, error> create(int2 Size, yw::flags<flag> Flags = flag::shader_resource) {
    if (Size.x <= 0 || Size.y <= 0)
      return std::unexpected(error(errors::invalid_argument, "invalid depth texture size"));
    const uint2 sz{Size};
    const auto sp = make_slot<depth_texture>();
    if (!sp) return std::unexpected(error(errors::slot_creation_failed));
    sp->size = sz;
    sp->flags = Flags;
    sp->dxgiformat = Flags.contains(flag::shader_resource) ? DXGI_FORMAT_R32_TYPELESS : DXGI_FORMAT_D32_FLOAT;
    D3D11_TEXTURE2D_DESC tex_desc{
      sz.x, sz.y, 1, 1, sp->dxgiformat, DXGI_SAMPLE_DESC{1, 0}, {}, D3D11_BIND_DEPTH_STENCIL};
    if (Flags.contains(flag::shader_resource)) tex_desc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
    hresult_test(d3d::device()->CreateTexture2D, &tex_desc, nullptr, &sp->texture.get());
    D3D11_DEPTH_STENCIL_VIEW_DESC dsv_desc{DXGI_FORMAT_D32_FLOAT, D3D11_DSV_DIMENSION_TEXTURE2D};
    hresult_test(d3d::device()->CreateDepthStencilView, sp->texture.get(), &dsv_desc, &sp->dsv.get());
    if (Flags.contains(flag::shader_resource)) {
      D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc{DXGI_FORMAT_R32_FLOAT, D3D11_SRV_DIMENSION_TEXTURE2D};
      srv_desc.Texture2D.MipLevels = 1;
      hresult_test(d3d::device()->CreateShaderResourceView, sp->texture.get(), &srv_desc, &sp->srv.get());
    }
    return make_handle<depth_texture>(sp->id);
  }

  depth_texture(int2 Size, yw::flags<flag> Flags = flag::shader_resource, const source_line& sl = here()) {
    if (auto res = create(Size, Flags); !res) res.error().go_off(sl);
    else *this = std::move(*res);
  }

  std::expected<void, error> clear(float Depth = 1.0f) {
    if (!d3d_dsv()) return std::unexpected(error(errors::invalid_operation, "depth texture is not a depth stencil view"));
    d3d::context()->ClearDepthStencilView(d3d_dsv(), D3D11_CLEAR_DEPTH, Depth, 0);
    return {};
  }
};

inline std::expected<void, error> draw_bitmap(
  float2 Pos, float2 Size, const bitmap_texture& Texture, float1 Opacity = 1.0f) {
  if (!Texture) return std::unexpected(error(errors::invalid_argument, "invalid bitmap texture"));
  if (!Texture.d2d_bitmap()) return std::unexpected(error(errors::invalid_argument, "bitmap texture is not initialized"));
  if (auto res = yw::draw_bitmap(Pos, Size, Texture.d2d_bitmap(), Opacity); !res) return res.error().relay();
  return {};
}

inline std::expected<void, error> draw_bitmap(float2 Pos, const bitmap_texture& Texture, float1 Opacity = 1.0f) {
  if (!Texture) return std::unexpected(error(errors::invalid_argument, "invalid bitmap texture"));
  if (!Texture.d2d_bitmap()) return std::unexpected(error(errors::invalid_argument, "bitmap texture is not initialized"));
  if (auto res = yw::draw_bitmap(Pos, Texture.size(), Texture.d2d_bitmap(), Opacity); !res) return res.error().relay();
  return {};
}
} // namespace yw
