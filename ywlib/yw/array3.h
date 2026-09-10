#pragma once
#include <yw/array1.h>

namespace yw {

template<trivial T, backend Backend = cpu> class array3;

template<trivial T> class array3<T, cpu> {
  array1<T, cpu> _data;
  size_t _width = 0;
  size_t _height = 0;
  size_t _depth = 0;

  static constexpr std::expected<size_t, error> _count(size_t Width, size_t Height, size_t Depth) {
    if (Height != 0 && Width > std::numeric_limits<size_t>::max() / Height)
      return std::unexpected(error(errors::invalid_argument, "array size is too large"));
    const auto plane = Width * Height;
    if (Depth != 0 && plane > std::numeric_limits<size_t>::max() / Depth)
      return std::unexpected(error(errors::invalid_argument, "array size is too large"));
    return plane * Depth;
  }

public:
  static constexpr auto backend = cpu;
  using value_type = T;

  constexpr array3() noexcept = default;
  array3(array3&&) noexcept = default;
  array3& operator=(array3&&) noexcept = default;
  array3(const array3&) = default;
  array3& operator=(const array3&) = default;

  constexpr array3(size_t Width, size_t Height, size_t Depth, const source_line& sl = here())
    requires std::default_initializable<T> {
    if (auto res = resize(Width, Height, Depth); !res) res.error().go_off(sl);
  }

  constexpr array3(size_t Width, size_t Height, size_t Depth, const T& Value, const source_line& sl = here()) {
    if (auto res = resize(Width, Height, Depth, Value); !res) res.error().go_off(sl);
  }

  static constexpr std::expected<array3, error> create(size_t Width, size_t Height, size_t Depth)
    requires std::default_initializable<T> {
    array3 a;
    if (auto res = a.resize(Width, Height, Depth); !res) return res.error().relay();
    return a;
  }

  static constexpr std::expected<array3, error> create(size_t Width, size_t Height, size_t Depth, const T& Value) {
    array3 a;
    if (auto res = a.resize(Width, Height, Depth, Value); !res) return res.error().relay();
    return a;
  }

  constexpr bool empty() const noexcept { return _data.empty(); }
  constexpr size_t size() const noexcept { return _data.size(); }
  constexpr size_t width() const noexcept { return _width; }
  constexpr size_t height() const noexcept { return _height; }
  constexpr size_t depth() const noexcept { return _depth; }
  constexpr T* data() noexcept { return _data.data(); }
  constexpr const T* data() const noexcept { return _data.data(); }
  constexpr T* begin() noexcept { return _data.begin(); }
  constexpr const T* begin() const noexcept { return _data.begin(); }
  constexpr T* end() noexcept { return _data.end(); }
  constexpr const T* end() const noexcept { return _data.end(); }
  constexpr T& operator[](size_t Index) noexcept { return _data[Index]; }
  constexpr const T& operator[](size_t Index) const noexcept { return _data[Index]; }
  constexpr T& operator()(size_t X, size_t Y, size_t Z) noexcept { return _data[(Z * _height + Y) * _width + X]; }
  constexpr const T& operator()(size_t X, size_t Y, size_t Z) const noexcept {
    return _data[(Z * _height + Y) * _width + X];
  }

  constexpr void clear() noexcept {
    _data.clear();
    _width = 0;
    _height = 0;
    _depth = 0;
  }

  constexpr void reset() noexcept {
    _data.reset();
    _width = 0;
    _height = 0;
    _depth = 0;
  }

  constexpr std::expected<void, error> resize(size_t Width, size_t Height, size_t Depth, const T& Value) {
    auto count = _count(Width, Height, Depth);
    if (!count) return count.error().relay();
    if (auto res = _data.resize(*count, Value); !res) return res.error().relay();
    _width = Width;
    _height = Height;
    _depth = Depth;
    return {};
  }

  constexpr std::expected<void, error> resize(size_t Width, size_t Height, size_t Depth)
    requires std::default_initializable<T> {
    return resize(Width, Height, Depth, T{});
  }
};
} // namespace yw
