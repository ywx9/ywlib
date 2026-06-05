#pragma once
#include <ywx/core.h>

namespace yw {

template<typename T> class staging_buffer;

//////////////////////////////////////// MARK: BUFFER

template<typename T> class buffer {
protected:
  comptr<::ID3D11Buffer> _buffer{};
  uint32_t _size{};

  buffer(uint1 Size) noexcept : _size(Size.x) {}

public:
  using value_type = T;

  explicit operator bool() const noexcept { return bool(_buffer); }
  explicit operator ::ID3D11Buffer*&() & { return _buffer.get(); }
  explicit operator ::ID3D11Buffer*() const& { return _buffer.get(); }

  buffer() noexcept = default;

  std::expected<void, error_trace> copy_from(const buffer& src) {
    if (!bool(src)) return unexpected_error(errors::invalid_argument, "Uninitialized source buffer");
    if (!bool(_buffer)) return unexpected_error(errors::invalid_argument, "Uninitialized destination buffer");
    if (src._size != _size) return unexpected_error(errors::invalid_argument, "Buffer size mismatch");
    d3d().context()->CopyResource(_buffer.get(), src._buffer.get());
    return {};
  }

  uint32_t size() const noexcept { return _size; }
  ID3D11Buffer* d3d_buffer() const { return _buffer.get(); }

  std::expected<std::vector<T>, error_trace> copy_to_cpu() const;
  std::expected<std::vector<T>, error_trace> copy_to_cpu(staging_buffer<T>&) const;
};

//////////////////////////////////////// MARK: STAGING BUFFER

template<typename T> class staging_buffer : public buffer<T> {
public:
  using buffer<T>::operator bool;
  using buffer<T>::operator ::ID3D11Buffer*&;
  using buffer<T>::operator ::ID3D11Buffer*;

  staging_buffer() noexcept = default;

  static std::expected<staging_buffer, error_trace> create(uint1 Size) {
    staging_buffer stb;
    stb._size = Size.x;
    D3D11_BUFFER_DESC desc{UINT(sizeof(T)) * Size.x, D3D11_USAGE_STAGING, {}, D3D11_CPU_ACCESS_READ, {}, 0};
    hresult_test(d3d().device()->CreateBuffer, &desc, nullptr, &stb._buffer.get());
    return std::move(stb);
  }

  static std::expected<staging_buffer, error_trace> create(const buffer<T>& b) {
    staging_buffer stb;
    if (auto res = create(b.size())) stb = std::move(*res);
    else return unexpected_error(res.error());
    if (auto res = stb.copy_from(b); !res) return unexpected_error(res.error());
    return std::move(stb);
  }

  std::expected<std::vector<T>, error_trace> copy_to_cpu() const {
    if (!*this) return unexpected_error(errors::invalid_argument, "Uninitialized staging buffer");
    std::vector<T> Data(buffer<T>::size());
    D3D11_MAPPED_SUBRESOURCE mapped;
    hresult_test(d3d().context()->Map, buffer<T>::d3d_buffer(), 0, D3D11_MAP_READ, 0, &mapped);
    std::memcpy(Data.data(), mapped.pData, buffer<T>::size() * sizeof(T));
    d3d().context()->Unmap(buffer<T>::d3d_buffer(), 0);
    return std::move(Data);
  }
};

template<typename T> staging_buffer(const buffer<T>&) -> staging_buffer<T>;

template<typename T> std::expected<std::vector<T>, error_trace> buffer<T>::copy_to_cpu() const {
  staging_buffer<T> staging;
  if (auto res = staging_buffer<T>::create(*this)) staging = std::move(*res);
  else return unexpected_error(res.error());
  if (auto res = staging.copy_to_cpu()) return std::move(*res);
  else return unexpected_error(res.error());
}

template<typename T>
std::expected<std::vector<T>, error_trace> buffer<T>::copy_to_cpu(staging_buffer<T>& Staging) const {
  if (auto res = Staging.copy_from(*this)) return Staging.copy_to_cpu();
  else return unexpected_error(res.error());
}

//////////////////////////////////////// MARK: CONSTANT BUFFER

template<typename T> requires(sizeof(T) % 16 == 0) class constant_buffer : public buffer<T> {
  static constexpr D3D11_BUFFER_DESC desc{
    sizeof(T), D3D11_USAGE_DYNAMIC, D3D11_BIND_CONSTANT_BUFFER, D3D11_CPU_ACCESS_WRITE};

public:
  using buffer<T>::operator bool;
  using buffer<T>::operator ::ID3D11Buffer*&;
  using buffer<T>::operator ::ID3D11Buffer*;

  constant_buffer() noexcept = default;

  static std::expected<constant_buffer, error_trace> create(const T& Val) {
    constant_buffer cb;
    cb._size = 1;
    D3D11_SUBRESOURCE_DATA srd(&Val, 0, 0);
    hresult_test(d3d().device()->CreateBuffer, &desc, &srd, &cb._buffer.get());
    return std::move(cb);
  }

  std::expected<void, error_trace> set(const T& Val) {
    if (!bool(*this)) return unexpected_error(errors::invalid_argument, "Uninitialized constant buffer");
    D3D11_MAPPED_SUBRESOURCE mapped;
    hresult_test(d3d().context()->Map, buffer<T>::d3d_buffer(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    std::memcpy(mapped.pData, &Val, sizeof(T));
    d3d().context()->Unmap(buffer<T>::d3d_buffer(), 0);
    return {};
  }
};

template<typename T> constant_buffer(const T&) -> constant_buffer<T>;

//////////////////////////////////////// MARK: STRUCTURED BUFFER

template<typename T> class structured_buffer : public buffer<T> {
  comptr<::ID3D11ShaderResourceView> _srv{};

  std::expected<void, error_trace> _init(const T* Data) {
    const auto count = buffer<T>::size();
    D3D11_BUFFER_DESC bd{
      UINT(sizeof(T)) * count, {}, D3D11_BIND_SHADER_RESOURCE, {}, D3D11_RESOURCE_MISC_BUFFER_STRUCTURED, sizeof(T)};
    if (Data) {
      D3D11_SUBRESOURCE_DATA srd{Data, int(sizeof(T))};
      hresult_test(d3d().device()->CreateBuffer, &bd, &srd, &(::ID3D11Buffer*&)*this);
    } else hresult_test(d3d().device()->CreateBuffer, &bd, nullptr, &(::ID3D11Buffer*&)*this);
    D3D11_BUFFER_SRV buffer_srv{.FirstElement = 0, .NumElements = count};
    D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc{{}, D3D11_SRV_DIMENSION_BUFFER, buffer_srv};
    hresult_test(d3d().device()->CreateShaderResourceView, (::ID3D11Buffer*)*this, &srv_desc, &_srv.get());
    return {};
  }

public:
  using buffer<T>::operator ::ID3D11Buffer*&;
  using buffer<T>::operator ::ID3D11Buffer*;

  explicit operator bool() const noexcept { return bool(_srv); }
  explicit operator ::ID3D11ShaderResourceView*&() noexcept { return _srv.get(); }
  explicit operator ::ID3D11ShaderResourceView*() const noexcept { return _srv.get(); }

  structured_buffer() noexcept = default;

  static std::expected<structured_buffer, error_trace> create(uint1 Size) {
    if (auto res = create(nullptr, Size)) return res;
    else return unexpected_error(res.error());
  }

  static std::expected<structured_buffer, error_trace> create(const T* Data, uint1 Size) {
    structured_buffer sb;
    sb._size = Size.x;
    if (auto res = sb._init(Data)) return sb;
    else return unexpected_error(res.error());
  }

  template<contiguous_range<T> Rg> static std::expected<structured_buffer, error_trace> create(Rg&& rg) {
    if (auto res = create(yw::data(rg), yw::size(rg))) return res;
    else return unexpected_error(res.error());
  }

  std::expected<void, error_trace> copy_from(const T* Data, uint1 Size) {
    if (!bool(*this)) return unexpected_error(errors::not_initialized, "Uninitialized structured buffer");
    if (buffer<T>::size() != Size.x) return unexpected_error(errors::invalid_argument, "Buffer size mismatch");
    d3d().context()->UpdateSubresource((::ID3D11Buffer*)*this, 0, nullptr, Data, 0, 0);
    return {};
  }
};

template<typename T> structured_buffer(const buffer<T>&) -> structured_buffer<T>;
template<typename T> structured_buffer(const T*, uint1) -> structured_buffer<T>;

//////////////////////////////////////// MARK: RW STRUCTURED BUFFER

template<typename T> class rw_structured_buffer : public buffer<T> {
  comptr<::ID3D11UnorderedAccessView> _uav;

  std::expected<void, error_trace> _init(const T* Data) {
    D3D11_BUFFER_DESC desc{UINT(sizeof(T)) * buffer<T>::size(), {}, 0x80, {}, 0x40, sizeof(T)};
    if (Data) {
      D3D11_SUBRESOURCE_DATA srd{Data, int(sizeof(T))};
      hresult_test(d3d().device()->CreateBuffer, &desc, &srd, &(::ID3D11Buffer*&)*this);
    } else hresult_test(d3d().device()->CreateBuffer, &desc, nullptr, &(::ID3D11Buffer*&)*this);
    hresult_test(d3d().device()->CreateUnorderedAccessView, (::ID3D11Buffer*)*this, nullptr, &_uav.get());
    return {};
  }
public:
  using buffer<T>::operator ::ID3D11Buffer*&;
  using buffer<T>::operator ::ID3D11Buffer*;

  explicit operator bool() const noexcept { return bool(_uav); }
  explicit operator ::ID3D11UnorderedAccessView*&() noexcept { return _uav.get(); }
  explicit operator ::ID3D11UnorderedAccessView*() const noexcept { return _uav.get(); }

  rw_structured_buffer() noexcept = default;

  static std::expected<rw_structured_buffer, error_trace> create(uint1 Size) {
    if (auto res = create(nullptr, Size)) return res;
    else return unexpected_error(res.error());
  }

  static std::expected<rw_structured_buffer, error_trace> create(const T* Data, uint1 Size) {
    rw_structured_buffer sb;
    sb._size = Size.x;
    if (auto res = sb._init(Data)) return sb;
    else return unexpected_error(res.error());
  }

  std::expected<void, error_trace> copy_from(const T* Data, uint1 Size) {
    if (!bool(*this)) return unexpected_error(errors::not_initialized, "Uninitialized rw_structured_buffer");
    if (Size.x != buffer<T>::size()) return unexpected_error(errors::invalid_argument, "Buffer size mismatch");
    d3d().context()->UpdateSubresource(buffer<T>::d3d_buffer(), 0, nullptr, Data, 0, 0);
    return {};
  }
};

template<typename T> rw_structured_buffer(const buffer<T>&) -> rw_structured_buffer<T>;
template<typename T> rw_structured_buffer(const T*, uint1) -> rw_structured_buffer<T>;
} // namespace yw
