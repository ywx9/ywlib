#pragma once
#include <ywx/core.h>

namespace yw {

template<typename T> class staging_buffer;

/// MARK: buffer

template<typename T> class buffer : public handle_base {
public:
  struct slot : public handle_base::slot {
    comptr<ID3D11Buffer> buffer{};
    uint32_t size{};
  };

  using value_type = T;
  buffer() noexcept = default;
  buffer(buffer&&) noexcept = default;
  buffer& operator=(buffer&&) noexcept = default;

  explicit operator bool() const noexcept {
    const auto sp = slot::template get_as<buffer>(id());
    return sp && sp->buffer;
  }

  explicit operator ID3D11Buffer*() const noexcept { return buffer<T>::d3d_buffer(); }

  ID3D11Buffer* d3d_buffer() const noexcept {
    if (const auto sp = slot::template get_as<buffer>(id())) return sp->buffer.get();
    else return nullptr;
  }

  uint32_t size() const noexcept {
    if (const auto sp = slot::template get_as<buffer>(id())) return sp->size;
    else return 0;
  }

  std::expected<void, error> copy_from(const buffer& Other) {
    if (!Other) return std::unexpected(error(errors::invalid_argument, "uninitialized source buffer"));
    if (!*this) return std::unexpected(error(errors::invalid_argument, "uninitialized destination buffer"));
    const auto sp = slot::template get_as<buffer>(id());
    const auto osp = slot::template get_as<buffer>(Other.id());
    if (sp->size != osp->size) return std::unexpected(error(errors::invalid_operation, "unmatched buffer sizes"));
    d3d::context()->CopyResource(sp->buffer.get(), osp->buffer.get());
    return {};
  }

  std::expected<void, error> copy_to_cpu(void* o) const;
  std::vector<T> copy_to_cpu(const source_line& sl = here()) const;
};

/// MARK: staging_buffer

template<typename T> class staging_buffer : public buffer<T> {
public:
  struct slot : public buffer<T>::slot {
    using yw::buffer<T>::slot::buffer;
    using yw::buffer<T>::slot::size;
    std::expected<void, error> copy_to_cpu(void* o) const {
      if (size == 0) return std::unexpected(error(errors::invalid_operation, "empty buffer"));
      D3D11_MAPPED_SUBRESOURCE mapped;
      hresult_test(d3d::context()->Map, buffer.get(), 0, D3D11_MAP_READ, 0, &mapped);
      std::memcpy(o, mapped.pData, size * sizeof(T));
      d3d::context()->Unmap(buffer.get(), 0);
      return {};
    }
  };

  using buffer<T>::operator bool;
  using buffer<T>::operator ID3D11Buffer*;
  staging_buffer() noexcept = default;

  static std::expected<staging_buffer, error> create(uint1 Size) {
    const auto sp = handle_base::make_slot<staging_buffer>();
    if (!sp) return std::unexpected(error(errors::slot_creation_failed));
    sp->size = Size.x;
    D3D11_BUFFER_DESC desc{UINT(sizeof(T)) * Size.x, D3D11_USAGE_STAGING, {}, D3D11_CPU_ACCESS_READ, {}, 0};
    hresult_test(d3d::device()->CreateBuffer, &desc, nullptr, &sp->buffer.get());
    return handle_base::make_handle<staging_buffer>(sp->id);
  }

  staging_buffer(uint1 Size, const source_line& sl = here()) {
    if (auto res = create(Size); !res) res.error().go_off(sl);
    else *this = std::move(res.value());
  }

  static std::expected<staging_buffer, error> create(const buffer<T>& Other) {
    if (auto b = create(uint1{Other.size()}); !b) return b.error().relay();
    else if (auto res = b->copy_from(Other); !res) return res.error().relay();
    else return std::move(*b);
  }

  staging_buffer(const buffer<T>& Other, const source_line& sl = here()) {
    if (auto res = create(Other); !res) res.error().go_off(sl);
    else *this = std::move(res.value());
  }

  std::expected<void, error> copy_to_cpu(void* o) const {
    if (const auto sp = slot::template get_as<staging_buffer>(handle_base::id()); !sp) return std::unexpected(error(errors::invalid_slotid));
    else if (auto res = sp->copy_to_cpu(o); !res) return res.error().relay();
    else return {};
  }

  std::vector<T> copy_to_cpu(const source_line& sl = here()) const {
    std::vector<T> Data(buffer<T>::size());
    if (const auto sp = slot::template get_as<staging_buffer>(handle_base::id()); !sp) error(errors::invalid_slotid).go_off(sl);
    else if (auto res = sp->copy_to_cpu(Data.data()); !res) {
      res.error().add_footprint().fizzle_out(sl);
      return {};
    }
    return Data;
  }
};

template<typename T> staging_buffer(const buffer<T>&) -> staging_buffer<T>;

template<typename T> inline std::expected<void, error> buffer<T>::copy_to_cpu(void* o) const {
  if (auto stb = staging_buffer<T>::create(size()); !stb) return stb.error().relay();
  else if (auto res = stb->copy_from(*this); !res) return res.error().relay();
  else if (auto res = stb->copy_to_cpu(o); !res) return res.error().relay();
  else return {};
}

template<typename T> std::vector<T> buffer<T>::copy_to_cpu(const source_line& sl) const {
  std::vector<T> Data(buffer<T>::size());
  if (auto res = copy_to_cpu(Data.data()); !res) res.error().add_footprint().fizzle_out(sl);
  return Data;
}

/// MARK: constant_buffer

template<typename T> requires(sizeof(T) % 16 == 0) class constant_buffer : public buffer<T> {
  static constexpr D3D11_BUFFER_DESC desc{
    sizeof(T), D3D11_USAGE_DYNAMIC, D3D11_BIND_CONSTANT_BUFFER, D3D11_CPU_ACCESS_WRITE};

public:
  struct slot : buffer<T>::slot {};

  using buffer<T>::operator bool;
  using buffer<T>::operator ID3D11Buffer*;
  constant_buffer() noexcept = default;

  static std::expected<constant_buffer, error> create(const T& Val) {
    const auto sp = handle_base::make_slot<constant_buffer>();
    if (!sp) return std::unexpected(error(errors::slot_creation_failed));
    sp->size = 1;
    D3D11_SUBRESOURCE_DATA srd(&Val, 0, 0);
    hresult_test(d3d::device()->CreateBuffer, &desc, &srd, &sp->buffer.get());
    return handle_base::make_handle<constant_buffer>(sp->id);
  }

  constant_buffer(const T& Val, const source_line& sl = here()) {
    if (auto res = create(Val); !res) res.error().go_off(sl);
    else *this = std::move(res.value());
  }

  std::expected<void, error> copy_from(const T& Val) {
    const auto sp = slot::template get_as<constant_buffer>(handle_base::id());
    if (!sp) return std::unexpected(error(errors::invalid_slotid));
    D3D11_MAPPED_SUBRESOURCE mapped;
    hresult_test(d3d::context()->Map, sp->buffer.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    std::memcpy(mapped.pData, &Val, sizeof(T));
    d3d::context()->Unmap(sp->buffer.get(), 0);
    return {};
  }
};

template<typename T> constant_buffer(const T&) -> constant_buffer<T>;

/// MARK: structured_buffer

template<typename T, bool RW = false> class structured_buffer : public buffer<T> {
  using resource_view_type = select_type<RW, ::ID3D11UnorderedAccessView, ::ID3D11ShaderResourceView>;

public:
  struct slot : buffer<T>::slot {
    comptr<resource_view_type> resource_view{};

    std::expected<void, error> initialize(const T* Data, uint32_t Size) {
      this->size = Size;
      D3D11_BUFFER_DESC bd{UINT(sizeof(T)) * Size};
      bd.BindFlags = RW ? D3D11_BIND_UNORDERED_ACCESS : D3D11_BIND_SHADER_RESOURCE;
      bd.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
      bd.StructureByteStride = sizeof(T);
      if (Data) {
        D3D11_SUBRESOURCE_DATA srd{Data, int(sizeof(T))};
        hresult_test(d3d::device()->CreateBuffer, &bd, &srd, &this->buffer.get());
      } else hresult_test(d3d::device()->CreateBuffer, &bd, nullptr, &this->buffer.get());
      if constexpr (!RW) {
        D3D11_BUFFER_SRV buffer_srv{.FirstElement = 0, .NumElements = Size};
        D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc{{}, D3D11_SRV_DIMENSION_BUFFER, buffer_srv};
        hresult_test(d3d::device()->CreateShaderResourceView, this->buffer.get(), &srv_desc, &resource_view.get());
      } else hresult_test(d3d::device()->CreateUnorderedAccessView, this->buffer.get(), nullptr, &resource_view.get());
      return {};
    }
  };

  explicit operator bool() const noexcept {
    const auto sp = slot::template get_as<structured_buffer>(handle_base::id());
    return sp && static_cast<bool>(sp->resource_view);
  }

  explicit operator resource_view_type*() const noexcept { return structured_buffer<T, RW>::d3d_resource_view(); }

  resource_view_type* d3d_resource_view() const noexcept {
    if (const auto sp = slot::template get_as<structured_buffer>(handle_base::id())) return sp->resource_view.get();
    else return nullptr;
  }

  structured_buffer() noexcept = default;

  static std::expected<structured_buffer, error> create(uint1 Size) {
    const auto sp = handle_base::make_slot<structured_buffer>();
    if (!sp) return std::unexpected(error(errors::slot_creation_failed));
    if (auto res = sp->initialize(nullptr, Size.x); !res) return res.error().relay();
    return handle_base::make_handle<structured_buffer>(sp->id);
  }

  structured_buffer(uint1 Size, const source_line& sl = here()) {
    if (auto res = create(Size); !res) res.error().go_off(sl);
    else *this = std::move(res.value());
  }

  static std::expected<structured_buffer, error> create(const T* Data, uint1 Size) {
    const auto sp = handle_base::make_slot<structured_buffer>();
    if (!sp) return std::unexpected(error(errors::slot_creation_failed));
    if (auto res = sp->initialize(Data, Size.x); !res) return res.error().relay();
    return handle_base::make_handle<structured_buffer>(sp->id);
  }

  structured_buffer(const T* Data, uint1 Size, const source_line& sl = here()) {
    if (auto res = create(Data, Size); !res) res.error().go_off(sl);
    else *this = std::move(res.value());
  }

  template<contiguous_range<T> Rg> static std::expected<structured_buffer, error> create(Rg&& rg) {
    const auto sp = handle_base::make_slot<structured_buffer>();
    if (!sp) return std::unexpected(error(errors::slot_creation_failed));
    if (auto res = sp->initialize(yw::data(rg), yw::size(rg)); !res) return res.error().relay();
    return handle_base::make_handle<structured_buffer>(sp->id);
  }

  template<contiguous_range<T> Rg> structured_buffer(Rg&& rg, const source_line& sl = here()) {
    if (auto res = create(yw::data(rg), yw::size(rg)); !res) res.error().go_off(sl);
    else *this = std::move(res.value());
  }

  std::expected<void, error> copy_from(const T* Data, uint1 Size) {
    const auto sp = slot::template get_as<structured_buffer>(handle_base::id());
    if (!sp) return std::unexpected(error(errors::invalid_slotid));
    if (Size.x != sp->size) return std::unexpected(error(errors::invalid_operation, "unmatched buffer sizes"));
    d3d::context()->UpdateSubresource((::ID3D11Buffer*)*this, 0, nullptr, Data, 0, 0);
    return {};
  }

  template<contiguous_range<T> Rg> std::expected<void, error> copy_from(Rg&& rg) {
    if (auto res = copy_from(yw::data(rg), yw::size(rg)); !res) return res.error().relay();
    return {};
  }
};

template<typename T> using rw_structured_buffer = structured_buffer<T, true>;

template<typename T> structured_buffer(const buffer<T>&) -> structured_buffer<T>;
template<typename T> structured_buffer(const T*, uint1) -> structured_buffer<T>;
template<contiguous_range Rg> structured_buffer(Rg&&) -> structured_buffer<iter_value_t<Rg>>;
} // namespace yw
