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
  std::expected<void, error> validate_initialized(const char* message = "Uninitialized buffer") const {
    if (_buffer) return {};
    return std::unexpected(error(errors::not_initialized, message));
  }

  std::expected<void, error> validate_same_size(uint32_t size, const char* message = "Buffer size mismatch") const {
    if (_size == size) return {};
    return std::unexpected(error(errors::invalid_argument, message));
  }

public:
  using value_type = T;

  explicit operator bool() const noexcept { return bool(_buffer); }
  explicit operator ::ID3D11Buffer*&() & { return _buffer.get(); }
  explicit operator ::ID3D11Buffer*() const& { return _buffer.get(); }

  buffer() noexcept = default;

  std::expected<void, error> copy_from(const buffer& src) {
    if (auto res = src.validate_initialized("Uninitialized source buffer"); !res) return res.error().relay();
    if (auto res = validate_initialized("Uninitialized destination buffer"); !res) return res.error().relay();
    if (auto res = validate_same_size(src._size); !res) return res.error().relay();
    d3d().context()->CopyResource(_buffer.get(), src._buffer.get());
    return {};
  }

  uint32_t size() const noexcept { return _size; }
  ID3D11Buffer* d3d_buffer() const { return _buffer.get(); }

  std::expected<std::vector<T>, error> copy_to_cpu() const;
  std::expected<std::vector<T>, error> copy_to_cpu(staging_buffer<T>&) const;
};

//////////////////////////////////////// MARK: STAGING BUFFER

template<typename T> class staging_buffer : public buffer<T> {
  std::expected<void, error> initialize(uint1 Size) {
    this->_size = Size.x;
    D3D11_BUFFER_DESC desc{UINT(sizeof(T)) * Size.x, D3D11_USAGE_STAGING, {}, D3D11_CPU_ACCESS_READ, {}, 0};
    hresult_test(d3d().device()->CreateBuffer, &desc, nullptr, &this->_buffer.get());
    return {};
  }

public:
  using buffer<T>::operator bool;
  using buffer<T>::operator ::ID3D11Buffer*&;
  using buffer<T>::operator ::ID3D11Buffer*;

  staging_buffer() noexcept = default;
  staging_buffer(uint1 Size, const source_line& sl = source_line::here()) {
    if (auto res = initialize(Size); !res) res.error().print_as_fatal(sl);
  }

  staging_buffer(const buffer<T>& b, const source_line& sl = source_line::here()) : staging_buffer(b.size(), sl) {
    if (auto res = this->copy_from(b); !res) res.error().print_as_fatal(sl);
  }

  std::expected<std::vector<T>, error> copy_to_cpu() const {
    if (auto res = this->validate_initialized("Uninitialized staging buffer"); !res) return res.error().relay();
    std::vector<T> Data(buffer<T>::size());
    D3D11_MAPPED_SUBRESOURCE mapped;
    hresult_test(d3d().context()->Map, buffer<T>::d3d_buffer(), 0, D3D11_MAP_READ, 0, &mapped);
    std::memcpy(Data.data(), mapped.pData, buffer<T>::size() * sizeof(T));
    d3d().context()->Unmap(buffer<T>::d3d_buffer(), 0);
    return Data;
  }
};

template<typename T> staging_buffer(const buffer<T>&) -> staging_buffer<T>;

template<typename T> std::expected<std::vector<T>, error> buffer<T>::copy_to_cpu() const {
  if (auto res = validate_initialized("Uninitialized buffer"); !res) return res.error().relay();
  staging_buffer<T> staging(*this);
  if (auto res = staging.copy_to_cpu()) return std::move(*res);
  else return res.error().relay();
}

template<typename T>
std::expected<std::vector<T>, error> buffer<T>::copy_to_cpu(staging_buffer<T>& Staging) const {
  if (auto res = Staging.copy_from(*this)) return Staging.copy_to_cpu();
  else return res.error().relay();
}

//////////////////////////////////////// MARK: CONSTANT BUFFER

template<typename T> requires(sizeof(T) % 16 == 0) class constant_buffer : public buffer<T> {
  static constexpr D3D11_BUFFER_DESC desc{
    sizeof(T), D3D11_USAGE_DYNAMIC, D3D11_BIND_CONSTANT_BUFFER, D3D11_CPU_ACCESS_WRITE};

  std::expected<void, error> initialize(const T& Val) {
    this->_size = 1;
    D3D11_SUBRESOURCE_DATA srd(&Val, 0, 0);
    hresult_test(d3d().device()->CreateBuffer, &desc, &srd, &this->_buffer.get());
    return {};
  }

public:
  using buffer<T>::operator bool;
  using buffer<T>::operator ::ID3D11Buffer*&;
  using buffer<T>::operator ::ID3D11Buffer*;

  constant_buffer() noexcept = default;
  constant_buffer(const T& Val, const source_line& sl = source_line::here()) {
    if (auto res = initialize(Val); !res) res.error().print_as_fatal(sl);
  }

  std::expected<void, error> set(const T& Val) {
    if (auto res = this->validate_initialized("Uninitialized constant buffer"); !res) return res.error().relay();
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

  std::expected<void, error> initialize(const T* Data, uint1 Size) {
    this->_size = Size.x;
    const auto count = this->size();
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
  structured_buffer(uint1 Size, const source_line& sl = source_line::here()) {
    if (auto res = initialize(nullptr, Size); !res) res.error().print_as_fatal(sl);
  }

  structured_buffer(const T* Data, uint1 Size, const source_line& sl = source_line::here()) {
    if (auto res = initialize(Data, Size); !res) res.error().print_as_fatal(sl);
  }

  template<contiguous_range<T> Rg>
  structured_buffer(Rg&& rg, const source_line& sl = source_line::here()) {
    if (auto res = initialize(yw::data(rg), yw::size(rg)); !res) res.error().print_as_fatal(sl);
  }

  std::expected<void, error> copy_from(const T* Data, uint1 Size) {
    if (auto res = this->validate_initialized("Uninitialized structured buffer"); !res) return res.error().relay();
    if (Size.x != 0 && !Data) return std::unexpected(error(errors::invalid_argument, "null source data"));
    if (auto res = this->validate_same_size(Size.x); !res) return res.error().relay();
    d3d().context()->UpdateSubresource((::ID3D11Buffer*)*this, 0, nullptr, Data, 0, 0);
    return {};
  }
};

template<typename T> structured_buffer(const buffer<T>&) -> structured_buffer<T>;
template<typename T> structured_buffer(const T*, uint1) -> structured_buffer<T>;

//////////////////////////////////////// MARK: RW STRUCTURED BUFFER

template<typename T> class rw_structured_buffer : public buffer<T> {
  comptr<::ID3D11UnorderedAccessView> _uav;

  std::expected<void, error> initialize(const T* Data, uint1 Size) {
    this->_size = Size.x;
    D3D11_BUFFER_DESC desc{UINT(sizeof(T)) * this->size(), {}, 0x80, {}, 0x40, sizeof(T)};
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
  rw_structured_buffer(uint1 Size, const source_line& sl = source_line::here()) {
    if (auto res = initialize(nullptr, Size); !res) res.error().print_as_fatal(sl);
  }

  rw_structured_buffer(const T* Data, uint1 Size, const source_line& sl = source_line::here()) {
    if (auto res = initialize(Data, Size); !res) res.error().print_as_fatal(sl);
  }

  std::expected<void, error> copy_from(const T* Data, uint1 Size) {
    if (auto res = this->validate_initialized("Uninitialized rw_structured_buffer"); !res) return res.error().relay();
    if (Size.x != 0 && !Data) return std::unexpected(error(errors::invalid_argument, "null source data"));
    if (auto res = this->validate_same_size(Size.x); !res) return res.error().relay();
    d3d().context()->UpdateSubresource(buffer<T>::d3d_buffer(), 0, nullptr, Data, 0, 0);
    return {};
  }
};

template<typename T> rw_structured_buffer(const buffer<T>&) -> rw_structured_buffer<T>;
template<typename T> rw_structured_buffer(const T*, uint1) -> rw_structured_buffer<T>;
} // namespace yw
