#pragma once
#include <ywx/bitmap.h>

namespace yw {

/// \note 'd2d_bitmap' and 'unordered_access' are mutually exclusive

class texture : public handle_base {
public:
  enum class flag : uint8_t {
    none = 0,
    d2d_bitmap = 0x1,
    shader_resource = 0x2,
    render_target = 0x4,
    unordered_access = 0x8,
  };
  friend constexpr flag operator~(flag f) noexcept { return flag(~uint8_t(f)); }
  friend constexpr flag operator|(flag lhs, flag rhs) noexcept { return flag(uint8_t(lhs) | uint8_t(rhs)); }
  friend constexpr flag operator&(flag lhs, flag rhs) noexcept { return flag(uint8_t(lhs) & uint8_t(rhs)); }
  friend constexpr flag operator^(flag lhs, flag rhs) noexcept { return flag(uint8_t(lhs) ^ uint8_t(rhs)); }

  struct slot : handle_base::slot {
    uint2 size{};
    comptr<ID2D1Bitmap1> bitmap;
    comptr<ID3D11Texture2D> texture;
    comptr<ID3D11ShaderResourceView> srv;
    comptr<ID3D11RenderTargetView> rtv;
    comptr<ID3D11UnorderedAccessView> uav;
    DXGI_FORMAT dxgiformat = DXGI_FORMAT_UNKNOWN;
    yw::flags<flag> flags;
  };

private:
  static std::expected<void, error> _create_views(slot* sp) {
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
  explicit operator bool() const noexcept {
    const auto sp = get_slot(this);
    return sp && bool(sp->texture);
  }

  explicit operator ID3D11Texture2D*() const noexcept { return d3d_texture(); }
  explicit operator ID2D1Bitmap*() const noexcept { return d2d_bitmap(); }
  explicit operator ID2D1Bitmap1*() const noexcept { return d2d_bitmap(); }
  explicit operator ID3D11ShaderResourceView*() const noexcept { return d3d_srv(); }
  explicit operator ID3D11RenderTargetView*() const noexcept { return d3d_rtv(); }
  explicit operator ID3D11UnorderedAccessView*() const noexcept { return d3d_uav(); }

  ID3D11Texture2D* d3d_texture() const noexcept {
    const auto sp = get_slot(this);
    return sp ? sp->texture.get() : nullptr;
  }

  ID2D1Bitmap1* d2d_bitmap() const noexcept {
    const auto sp = get_slot(this);
    return sp ? sp->bitmap.get() : nullptr;
  }

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

  uint2 size() const noexcept {
    const auto sp = get_slot(this);
    return sp ? sp->size : uint2{};
  }

  DXGI_FORMAT format() const noexcept {
    const auto sp = get_slot(this);
    return sp ? sp->dxgiformat : DXGI_FORMAT_UNKNOWN;
  }

  yw::flags<flag> flags() const noexcept {
    const auto sp = get_slot(this);
    return sp ? sp->flags : yw::flags<flag>{};
  }

  texture() noexcept = default;

  /// creates a texture with `bitmap::dxgiformat`.
  /// \note `d2d_bitmap` and `unordered_access` are mutually exclusive.
  static std::expected<texture, error> create(
    int2 Size, yw::flags<flag> Flags = flag::d2d_bitmap | flag::shader_resource | flag::render_target) {
    if (Size.x <= 0 || Size.y <= 0) return std::unexpected(error(errors::invalid_argument, "invalid texture size"));
    if (Flags.contains(flag::d2d_bitmap | flag::unordered_access))
      return std::unexpected(error(errors::invalid_argument, "d2d_bitmap and unordered_access are mutually exclusive"));
    const uint2 sz{Size};
    const auto sp = make_slot<texture>();
    if (!sp) return std::unexpected(error(errors::slot_creation_failed));
    sp->size = sz;
    sp->flags = Flags;
    sp->dxgiformat = bitmap::dxgiformat;
    if (bool(Flags & flag::d2d_bitmap)) {
      hresult_test(d2d::context()->CreateBitmap, {sz.x, sz.y}, nullptr, 0, &bitmap::props, &sp->bitmap.get());
      comptr<IDXGISurface> surface;
      hresult_test(sp->bitmap->GetSurface, &surface.get());
      hresult_test(surface->QueryInterface, &sp->texture.get());
    } else {
      D3D11_TEXTURE2D_DESC tex_desc{sz.x, sz.y, 1, 1, bitmap::dxgiformat, DXGI_SAMPLE_DESC{1, 0}};
      if (Flags.contains(flag::shader_resource)) tex_desc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
      if (Flags.contains(flag::render_target)) tex_desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
      if (Flags.contains(flag::unordered_access)) tex_desc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
      hresult_test(d3d::device()->CreateTexture2D, &tex_desc, nullptr, &sp->texture.get());
    }
    if (auto res = _create_views(sp); !res) return res.error().relay();
    return make_handle<texture>(sp->id);
  }

  /// creates a texture with an explicit DXGI format.
  /// \note `d2d_bitmap` is not allowed.
  static std::expected<texture, error> create(
    int2 Size, DXGI_FORMAT Format, yw::flags<flag> Flags = flag::shader_resource) {
    if (Size.x <= 0 || Size.y <= 0) return std::unexpected(error(errors::invalid_argument, "invalid texture size"));
    if (Flags.contains(flag::d2d_bitmap))
      return std::unexpected(error(errors::invalid_argument, "d2d_bitmap is not allowed with explicit DXGI_FORMAT"));
    const uint2 sz{Size};
    const auto sp = make_slot<texture>();
    if (!sp) return std::unexpected(error(errors::slot_creation_failed));
    sp->size = sz;
    sp->flags = Flags;
    sp->dxgiformat = Format;
    D3D11_TEXTURE2D_DESC tex_desc{sz.x, sz.y, 1, 1, Format, DXGI_SAMPLE_DESC{1, 0}};
    if (Flags.contains(flag::shader_resource)) tex_desc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
    if (Flags.contains(flag::render_target)) tex_desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
    if (Flags.contains(flag::unordered_access)) tex_desc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
    hresult_test(d3d::device()->CreateTexture2D, &tex_desc, nullptr, &sp->texture.get());
    if (auto res = _create_views(sp); !res) return res.error().relay();
    return make_handle<texture>(sp->id);
  }

  /// creates a texture by moving a bitmap resource.
  /// \note `unordered_access` is not allowed.
  static std::expected<texture, error> create(
    bitmap&& Bitmap, yw::flags<flag> Flags = flag::d2d_bitmap | flag::shader_resource) {
    if (Flags.contains(flag::unordered_access))
      return std::unexpected(error(errors::invalid_argument, "unordered_access is not allowed with bitmap source"));
    const auto bsp = bitmap::slot::get_as<bitmap>(Bitmap.id());
    if (!bsp || !bsp->bitmap) return std::unexpected(error(errors::invalid_argument, "invalid bitmap"));
    const auto sp = make_slot<texture>();
    if (!sp) return std::unexpected(error(errors::slot_creation_failed));
    sp->size = bsp->size;
    sp->flags = Flags;
    sp->dxgiformat = bitmap::dxgiformat;
    if (Flags.contains(flag::d2d_bitmap)) {
      sp->bitmap = std::move(bsp->bitmap);
      comptr<IDXGISurface> surface;
      hresult_test(sp->bitmap->GetSurface, &surface.get());
      hresult_test(surface->QueryInterface, &sp->texture.get());
    } else {
      comptr<ID2D1Bitmap1> bitmap = std::move(bsp->bitmap);
      comptr<IDXGISurface> surface;
      hresult_test(bitmap->GetSurface, &surface.get());
      hresult_test(surface->QueryInterface, &sp->texture.get());
    }
    if (auto res = _create_views(sp); !res) return res.error().relay();
    return make_handle<texture>(sp->id);
  }

  /// creates a texture by copying a bitmap resource.
  /// \note `unordered_access` is not allowed.
  static std::expected<texture, error> create(
    const bitmap& Bitmap, yw::flags<flag> Flags = flag::d2d_bitmap | flag::shader_resource) {
    if (Flags.contains(flag::unordered_access))
      return std::unexpected(error(errors::invalid_argument, "unordered_access is not allowed with bitmap source"));
    if (auto b = bitmap::create(Bitmap); !b) return b.error().relay();
    else if (auto res = create(std::move(*b), Flags); !res) return res.error().relay();
    else return std::move(*res);
  }

  /// creates a texture from a file through `yw::bitmap`.
  /// \note `unordered_access` is not allowed.
  static std::expected<texture, error> create(
    const path& Path, yw::flags<flag> Flags = flag::d2d_bitmap | flag::shader_resource) {
    if (auto b = bitmap::create(Path); !b) return b.error().relay();
    else if (auto res = create(std::move(*b), Flags); !res) return res.error().relay();
    else return std::move(*res);
  }

  /// creates a texture by copying a bitmap resource.
  /// \note `unordered_access` is not allowed.
  explicit texture(const bitmap& Bitmap, yw::flags<flag> Flags = flag::d2d_bitmap | flag::shader_resource,
    const source_line& sl = here()) {
    if (auto res = create(Bitmap, Flags); !res) res.error().go_off(sl);
    else *this = std::move(*res);
  }

  /// creates a texture by moving a bitmap resource.
  /// \note `unordered_access` is not allowed.
  explicit texture(bitmap&& Bitmap, yw::flags<flag> Flags = flag::d2d_bitmap | flag::shader_resource,
    const source_line& sl = here()) {
    if (auto res = create(std::move(Bitmap), Flags); !res) res.error().go_off(sl);
    else *this = std::move(*res);
  }

  /// creates a texture from a file through `yw::bitmap`.
  /// \note `unordered_access` is not allowed.
  explicit texture(const path& Path, yw::flags<flag> Flags = flag::d2d_bitmap | flag::shader_resource,
    const source_line& sl = here()) {
    if (auto res = create(Path, Flags); !res) res.error().go_off(sl);
    else *this = std::move(*res);
  }

  /// creates a texture with `bitmap::dxgiformat`.
  /// \note `d2d_bitmap` and `unordered_access` are mutually exclusive.
  explicit texture(
    int2 Size, yw::flags<flag> Flags = flag::d2d_bitmap | flag::shader_resource | flag::render_target,
    const source_line& sl = here()) {
    if (auto res = create(Size, Flags); !res) res.error().go_off(sl);
    else *this = std::move(*res);
  }

  /// creates a texture with an explicit DXGI format.
  /// \note `d2d_bitmap` is not allowed.
  explicit texture(int2 Size, DXGI_FORMAT Format, yw::flags<flag> Flags = flag::shader_resource,
    const source_line& sl = here()) {
    if (auto res = create(Size, Format, Flags); !res) res.error().go_off(sl);
    else *this = std::move(*res);
  }

  /// clears the texture with a specified color.
  /// \note `render_target` flag must be set.
  std::expected<void, error> clear(const color& ClearColor = colors::transparent) {
    if (!d3d_rtv()) return std::unexpected(error(errors::invalid_operation, "texture is not a render target"));
    d3d::context()->ClearRenderTargetView(d3d_rtv(), &ClearColor.r);
    return {};
  }
};

class depth_texture : public handle_base {
public:
  enum class flag : uint8_t {
    none = 0,
    shader_resource = 0x1,
  };
  friend constexpr flag operator~(flag f) noexcept { return flag(~uint8_t(f)); }
  friend constexpr flag operator|(flag lhs, flag rhs) noexcept { return flag(uint8_t(lhs) | uint8_t(rhs)); }
  friend constexpr flag operator&(flag lhs, flag rhs) noexcept { return flag(uint8_t(lhs) & uint8_t(rhs)); }
  friend constexpr flag operator^(flag lhs, flag rhs) noexcept { return flag(uint8_t(lhs) ^ uint8_t(rhs)); }

  struct slot : handle_base::slot {
    uint2 size{};
    comptr<ID3D11Texture2D> texture;
    comptr<ID3D11DepthStencilView> dsv;
    comptr<ID3D11ShaderResourceView> srv;
    yw::flags<flag> flags;
  };

  explicit operator bool() const noexcept {
    const auto sp = get_slot(this);
    return sp && bool(sp->texture) && bool(sp->dsv);
  }

  explicit operator ID3D11Texture2D*() const noexcept { return d3d_texture(); }
  explicit operator ID3D11DepthStencilView*() const noexcept { return d3d_dsv(); }
  explicit operator ID3D11ShaderResourceView*() const noexcept { return d3d_srv(); }

  ID3D11Texture2D* d3d_texture() const noexcept {
    const auto sp = get_slot(this);
    return sp ? sp->texture.get() : nullptr;
  }

  ID3D11DepthStencilView* d3d_dsv() const noexcept {
    const auto sp = get_slot(this);
    return sp ? sp->dsv.get() : nullptr;
  }

  ID3D11ShaderResourceView* d3d_srv() const noexcept {
    const auto sp = get_slot(this);
    return sp ? sp->srv.get() : nullptr;
  }

  uint2 size() const noexcept {
    const auto sp = get_slot(this);
    return sp ? sp->size : uint2{};
  }

  yw::flags<flag> flags() const noexcept {
    const auto sp = get_slot(this);
    return sp ? sp->flags : yw::flags<flag>{};
  }

  depth_texture() noexcept = default;

  /// creates a depth texture.
  static std::expected<depth_texture, error> create(int2 Size, yw::flags<flag> Flags = flag::shader_resource) {
    if (Size.x <= 0 || Size.y <= 0)
      return std::unexpected(error(errors::invalid_argument, "invalid depth texture size"));
    const uint2 sz{Size};
    const auto sp = make_slot<depth_texture>();
    if (!sp) return std::unexpected(error(errors::slot_creation_failed));
    sp->size = sz;
    sp->flags = Flags;
    const auto tex_format = Flags.contains(flag::shader_resource) ? DXGI_FORMAT_R32_TYPELESS : DXGI_FORMAT_D32_FLOAT;
    D3D11_TEXTURE2D_DESC tex_desc{
      sz.x, sz.y, 1, 1, tex_format, DXGI_SAMPLE_DESC{1, 0}, {}, D3D11_BIND_DEPTH_STENCIL};
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

  /// creates a depth texture.
  depth_texture(int2 Size, yw::flags<flag> Flags = flag::shader_resource, const source_line& sl = here()) {
    if (auto res = create(Size, Flags); !res) res.error().go_off(sl);
    else *this = std::move(*res);
  }

  /// clears the depth texture with a specified depth value.
  /// \param Depth set to `0.0f` if reverse-z is enabled, otherwise set to `1.0f`.
  std::expected<void, error> clear(float Depth = 1.0f) {
    if (!d3d_dsv()) return std::unexpected(error(errors::invalid_operation, "depth texture is not a depth stencil view"));
    d3d::context()->ClearDepthStencilView(d3d_dsv(), D3D11_CLEAR_DEPTH, Depth, 0);
    return {};
  }
};

} // namespace yw
