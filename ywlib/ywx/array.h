#pragma once
#include <ywx/buffer.h>

namespace yw {

namespace internal {
inline std::expected<uint1, error> gpu_array_size(size_t Size) {
  if (Size > std::numeric_limits<uint32_t>::max())
    return std::unexpected(error(errors::invalid_argument, "gpu array size is too large"));
  return uint1{uint32_t(Size)};
}

inline std::expected<size_t, error> gpu_array_count(size_t Width, size_t Height) {
  if (Height != 0 && Width > std::numeric_limits<size_t>::max() / Height)
    return std::unexpected(error(errors::invalid_argument, "gpu array size is too large"));
  return Width * Height;
}

inline std::expected<size_t, error> gpu_array_count(size_t Width, size_t Height, size_t Depth) {
  auto plane = gpu_array_count(Width, Height);
  if (!plane) return plane.error().relay();
  if (Depth != 0 && *plane > std::numeric_limits<size_t>::max() / Depth)
    return std::unexpected(error(errors::invalid_argument, "gpu array size is too large"));
  return *plane * Depth;
}
} // namespace internal

template<trivial T> class array1<T, gpu> {
  rw_structured_buffer<T> _buffer;

public:
  static constexpr auto backend = gpu;
  using value_type = T;

  array1() noexcept = default;
  array1(array1&&) noexcept = default;
  array1& operator=(array1&&) noexcept = default;
  array1(const array1&) = delete;
  array1& operator=(const array1&) = delete;

  explicit array1(size_t Size, const source_line& sl = here()) {
    if (auto res = create(Size); !res) res.error().go_off(sl);
    else *this = std::move(*res);
  }

  array1(const T* Data, size_t Size, const source_line& sl = here()) {
    if (auto res = create(Data, Size); !res) res.error().go_off(sl);
    else *this = std::move(*res);
  }

  template<contiguous_range<T> Rg> explicit array1(Rg&& rg, const source_line& sl = here()) {
    if (auto res = create(rg); !res) res.error().go_off(sl);
    else *this = std::move(*res);
  }

  static std::expected<array1, error> create(size_t Size) {
    auto size = internal::gpu_array_size(Size);
    if (!size) return size.error().relay();
    array1 a;
    if (Size == 0) return a;
    if (auto res = rw_structured_buffer<T>::create(*size); !res) return res.error().relay();
    else a._buffer = std::move(*res);
    return a;
  }

  static std::expected<array1, error> create(const T* Data, size_t Size) {
    if (!Data && Size != 0) return std::unexpected(error(errors::invalid_argument, "null gpu array data"));
    auto size = internal::gpu_array_size(Size);
    if (!size) return size.error().relay();
    array1 a;
    if (Size == 0) return a;
    if (auto res = rw_structured_buffer<T>::create(Data, *size); !res) return res.error().relay();
    else a._buffer = std::move(*res);
    return a;
  }

  template<contiguous_range<T> Rg> static std::expected<array1, error> create(Rg&& rg) {
    return create(yw::data(rg), yw::size(rg));
  }

  explicit operator bool() const noexcept { return static_cast<bool>(_buffer); }
  explicit operator ::ID3D11Buffer*() const noexcept { return d3d_buffer(); }
  explicit operator ::ID3D11ShaderResourceView*() const noexcept { return d3d_shader_resource_view(); }
  explicit operator ::ID3D11UnorderedAccessView*() const noexcept { return d3d_unordered_access_view(); }

  bool empty() const noexcept { return size() == 0; }
  size_t size() const noexcept { return _buffer.size(); }
  ::ID3D11Buffer* d3d_buffer() const noexcept { return _buffer.d3d_buffer(); }
  ::ID3D11ShaderResourceView* d3d_shader_resource_view() const noexcept { return _buffer.d3d_shader_resource_view(); }
  ::ID3D11UnorderedAccessView* d3d_unordered_access_view() const noexcept {
    return _buffer.d3d_unordered_access_view();
  }

  std::expected<void, error> copy_from(const T* Data, size_t Size) {
    if (!Data && Size != 0) return std::unexpected(error(errors::invalid_argument, "null gpu array data"));
    auto size = internal::gpu_array_size(Size);
    if (!size) return size.error().relay();
    if (Size == 0 && this->size() == 0) return {};
    if (auto res = _buffer.copy_from(Data, *size); !res) return res.error().relay();
    return {};
  }

  template<contiguous_range<T> Rg> std::expected<void, error> copy_from(Rg&& rg) {
    return copy_from(yw::data(rg), yw::size(rg));
  }

  std::expected<void, error> copy_from(const array1<T, cpu>& Array) { return copy_from(Array.data(), Array.size()); }

  std::expected<void, error> copy_to_cpu(T* Out) const {
    if (!Out && size() != 0) return std::unexpected(error(errors::invalid_argument, "null output buffer"));
    if (size() == 0) return {};
    if (auto res = _buffer.copy_to_cpu(Out); !res) return res.error().relay();
    return {};
  }

  std::expected<array1<T, cpu>, error> copy_to_cpu() const requires std::default_initializable<T> {
    auto out = array1<T, cpu>::create(size());
    if (!out) return out.error().relay();
    if (auto res = copy_to_cpu(out->data()); !res) return res.error().relay();
    return std::move(*out);
  }
};

template<trivial T> class array2<T, gpu> {
  array1<T, gpu> _data;
  size_t _width = 0;
  size_t _height = 0;

public:
  static constexpr auto backend = gpu;
  using value_type = T;

  array2() noexcept = default;
  array2(array2&&) noexcept = default;
  array2& operator=(array2&&) noexcept = default;
  array2(const array2&) = delete;
  array2& operator=(const array2&) = delete;

  array2(size_t Width, size_t Height, const source_line& sl = here()) {
    if (auto res = create(Width, Height); !res) res.error().go_off(sl);
    else *this = std::move(*res);
  }

  static std::expected<array2, error> create(size_t Width, size_t Height) {
    auto count = internal::gpu_array_count(Width, Height);
    if (!count) return count.error().relay();
    array2 a;
    if (auto res = array1<T, gpu>::create(*count); !res) return res.error().relay();
    else a._data = std::move(*res);
    a._width = Width;
    a._height = Height;
    return a;
  }

  explicit operator bool() const noexcept { return static_cast<bool>(_data); }
  explicit operator ::ID3D11Buffer*() const noexcept { return d3d_buffer(); }
  explicit operator ::ID3D11ShaderResourceView*() const noexcept { return d3d_shader_resource_view(); }
  explicit operator ::ID3D11UnorderedAccessView*() const noexcept { return d3d_unordered_access_view(); }

  bool empty() const noexcept { return _data.empty(); }
  size_t size() const noexcept { return _data.size(); }
  size_t width() const noexcept { return _width; }
  size_t height() const noexcept { return _height; }
  ::ID3D11Buffer* d3d_buffer() const noexcept { return _data.d3d_buffer(); }
  ::ID3D11ShaderResourceView* d3d_shader_resource_view() const noexcept { return _data.d3d_shader_resource_view(); }
  ::ID3D11UnorderedAccessView* d3d_unordered_access_view() const noexcept {
    return _data.d3d_unordered_access_view();
  }

  std::expected<void, error> copy_from(const T* Data, size_t Count) { return _data.copy_from(Data, Count); }

  template<contiguous_range<T> Rg> std::expected<void, error> copy_from(Rg&& rg) {
    return _data.copy_from(rg);
  }

  std::expected<void, error> copy_from(const array2<T, cpu>& Array) {
    if (Array.width() != _width || Array.height() != _height)
      return std::unexpected(error(errors::invalid_operation, "unmatched array sizes"));
    return _data.copy_from(Array.data(), Array.size());
  }

  std::expected<void, error> copy_to_cpu(T* Out) const { return _data.copy_to_cpu(Out); }

  std::expected<array2<T, cpu>, error> copy_to_cpu() const requires std::default_initializable<T> {
    auto out = array2<T, cpu>::create(_width, _height);
    if (!out) return out.error().relay();
    if (auto res = copy_to_cpu(out->data()); !res) return res.error().relay();
    return std::move(*out);
  }
};

template<trivial T> class array3<T, gpu> {
  array1<T, gpu> _data;
  size_t _width = 0;
  size_t _height = 0;
  size_t _depth = 0;

public:
  static constexpr auto backend = gpu;
  using value_type = T;

  array3() noexcept = default;
  array3(array3&&) noexcept = default;
  array3& operator=(array3&&) noexcept = default;
  array3(const array3&) = delete;
  array3& operator=(const array3&) = delete;

  array3(size_t Width, size_t Height, size_t Depth, const source_line& sl = here()) {
    if (auto res = create(Width, Height, Depth); !res) res.error().go_off(sl);
    else *this = std::move(*res);
  }

  static std::expected<array3, error> create(size_t Width, size_t Height, size_t Depth) {
    auto count = internal::gpu_array_count(Width, Height, Depth);
    if (!count) return count.error().relay();
    array3 a;
    if (auto res = array1<T, gpu>::create(*count); !res) return res.error().relay();
    else a._data = std::move(*res);
    a._width = Width;
    a._height = Height;
    a._depth = Depth;
    return a;
  }

  explicit operator bool() const noexcept { return static_cast<bool>(_data); }
  explicit operator ::ID3D11Buffer*() const noexcept { return d3d_buffer(); }
  explicit operator ::ID3D11ShaderResourceView*() const noexcept { return d3d_shader_resource_view(); }
  explicit operator ::ID3D11UnorderedAccessView*() const noexcept { return d3d_unordered_access_view(); }

  bool empty() const noexcept { return _data.empty(); }
  size_t size() const noexcept { return _data.size(); }
  size_t width() const noexcept { return _width; }
  size_t height() const noexcept { return _height; }
  size_t depth() const noexcept { return _depth; }
  ::ID3D11Buffer* d3d_buffer() const noexcept { return _data.d3d_buffer(); }
  ::ID3D11ShaderResourceView* d3d_shader_resource_view() const noexcept { return _data.d3d_shader_resource_view(); }
  ::ID3D11UnorderedAccessView* d3d_unordered_access_view() const noexcept {
    return _data.d3d_unordered_access_view();
  }

  std::expected<void, error> copy_from(const T* Data, size_t Count) { return _data.copy_from(Data, Count); }

  template<contiguous_range<T> Rg> std::expected<void, error> copy_from(Rg&& rg) {
    return _data.copy_from(rg);
  }

  std::expected<void, error> copy_from(const array3<T, cpu>& Array) {
    if (Array.width() != _width || Array.height() != _height || Array.depth() != _depth)
      return std::unexpected(error(errors::invalid_operation, "unmatched array sizes"));
    return _data.copy_from(Array.data(), Array.size());
  }

  std::expected<void, error> copy_to_cpu(T* Out) const { return _data.copy_to_cpu(Out); }

  std::expected<array3<T, cpu>, error> copy_to_cpu() const requires std::default_initializable<T> {
    auto out = array3<T, cpu>::create(_width, _height, _depth);
    if (!out) return out.error().relay();
    if (auto res = copy_to_cpu(out->data()); !res) return res.error().relay();
    return std::move(*out);
  }
};
} // namespace yw
