#pragma once
#include <yw/array1.h>

namespace yw {

template<trivial T, backend Backend = cpu> class array2;

template<trivial T> class array2<T, cpu> {
  array1<T, cpu> _data;
  size_t _width = 0;
  size_t _height = 0;

  static constexpr std::expected<size_t, error> _count(size_t Width, size_t Height) {
    if (Height != 0 && Width > std::numeric_limits<size_t>::max() / Height)
      return std::unexpected(error(errors::invalid_argument, "array size is too large"));
    return Width * Height;
  }

public:
  static constexpr auto backend = cpu;
  using value_type = T;

  constexpr array2() noexcept = default;
  array2(array2&&) noexcept = default;
  array2& operator=(array2&&) noexcept = default;
  array2(const array2&) = default;
  array2& operator=(const array2&) = default;

  constexpr array2(size_t Width, size_t Height, const source_line& sl = here()) requires std::default_initializable<T> {
    if (auto res = resize(Width, Height); !res) res.error().go_off(sl);
  }

  constexpr array2(size_t Width, size_t Height, const T& Value, const source_line& sl = here()) {
    if (auto res = resize(Width, Height, Value); !res) res.error().go_off(sl);
  }

  static constexpr std::expected<array2, error> create(size_t Width, size_t Height)
    requires std::default_initializable<T> {
    array2 a;
    if (auto res = a.resize(Width, Height); !res) return res.error().relay();
    return a;
  }

  static constexpr std::expected<array2, error> create(size_t Width, size_t Height, const T& Value) {
    array2 a;
    if (auto res = a.resize(Width, Height, Value); !res) return res.error().relay();
    return a;
  }

  constexpr bool empty() const noexcept { return _data.empty(); }
  constexpr size_t size() const noexcept { return _data.size(); }
  constexpr size_t width() const noexcept { return _width; }
  constexpr size_t height() const noexcept { return _height; }
  constexpr T* data() noexcept { return _data.data(); }
  constexpr const T* data() const noexcept { return _data.data(); }
  constexpr T* begin() noexcept { return _data.begin(); }
  constexpr const T* begin() const noexcept { return _data.begin(); }
  constexpr T* end() noexcept { return _data.end(); }
  constexpr const T* end() const noexcept { return _data.end(); }
  constexpr T& operator[](size_t Index) noexcept { return _data[Index]; }
  constexpr const T& operator[](size_t Index) const noexcept { return _data[Index]; }
  constexpr T& operator()(size_t X, size_t Y) noexcept { return _data[Y * _width + X]; }
  constexpr const T& operator()(size_t X, size_t Y) const noexcept { return _data[Y * _width + X]; }

  constexpr void clear() noexcept {
    _data.clear();
    _width = 0;
    _height = 0;
  }

  constexpr void reset() noexcept {
    _data.reset();
    _width = 0;
    _height = 0;
  }

  constexpr std::expected<void, error> resize(size_t Width, size_t Height, const T& Value) {
    auto count = _count(Width, Height);
    if (!count) return count.error().relay();
    if (auto res = _data.resize(*count, Value); !res) return res.error().relay();
    _width = Width;
    _height = Height;
    return {};
  }

  constexpr std::expected<void, error> resize(size_t Width, size_t Height) requires std::default_initializable<T> {
    return resize(Width, Height, T{});
  }
};
} // namespace yw
