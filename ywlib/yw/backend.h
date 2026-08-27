#pragma once
#include <yw/error.h>

#include <new>

#ifndef YW_DEFAULT_BACKEND
#define YW_DEFAULT_BACKEND cpu
#endif

namespace yw {

template<size_t N> requires(N > 0) struct literal_string {
  char str[N];
  consteval literal_string(const char (&Str)[N]) {
    for (size_t i = 0; i < N; ++i) str[i] = Str[i];
  }
  consteval bool null_terminated() const noexcept { return str[N - 1] == '\0'; }
  consteval bool empty() const noexcept { return N == 1 && str[0] == '\0'; }
  consteval size_t size() const noexcept { return N - null_terminated(); }
  consteval const char* c_str() const noexcept { return str; }
  consteval const char* data() const noexcept { return str; }
  consteval char operator[](size_t index) const noexcept { return str[index]; }
  consteval operator const char*() const noexcept { return str; }
  consteval string_view<char> view() const noexcept { return {str, size()}; }
  consteval const char* begin() const noexcept { return str; }
  consteval const char* end() const noexcept { return str + size(); }
  template<size_t M> consteval bool operator==(const literal_string<M>& Other) const noexcept {
    if (size() != Other.size()) return false;
    for (size_t i = 0; i < size(); ++i)
      if (str[i] != Other[i]) return false;
    return true;
  }
};

template<literal_string Str> struct backend {
  static constexpr literal_string value = Str;
  static consteval string_view<char> to_string() noexcept { return value.view(); }
  template<literal_string OtherStr> consteval bool operator==(const backend<OtherStr>&) const noexcept {
    return backend::value == backend<OtherStr>::value;
  }
};

inline constexpr backend<"cpu"> cpu;
inline constexpr backend<"gpu"> gpu;

inline constexpr auto default_backend = YW_DEFAULT_BACKEND;

template<trivial T, backend Backend = default_backend> class array1;
template<trivial T, backend Backend = default_backend> class array2;
template<trivial T, backend Backend = default_backend> class array3;

template<trivial T> class array1<T, cpu> {
  T* _data = nullptr;
  size_t _size = 0;
  size_t _capacity = 0;

  static constexpr size_t _max_capacity() noexcept { return std::numeric_limits<size_t>::max() / sizeof(T); }

  static std::expected<T*, error> _allocate(size_t Capacity) {
    if (Capacity == 0) return nullptr;
    if (Capacity > _max_capacity())
      return std::unexpected(error(errors::invalid_argument, "array capacity is too large"));
    auto* p = static_cast<T*>(::operator new[](Capacity * sizeof(T), std::nothrow));
    if (!p) return std::unexpected(error(errors::allocation_failed));
    return p;
  }

  static void _deallocate(T* Data) noexcept { ::operator delete[](Data); }

  constexpr size_t _preferred_capacity(size_t Required) const noexcept {
    size_t preferred = _capacity == 0 ? size_t(8) : _capacity;
    while (preferred < Required && preferred <= _max_capacity() / 2) preferred *= 2;
    return yw::max(preferred, Required);
  }

public:
  static constexpr auto backend = cpu;
  using value_type = T;

  constexpr array1() noexcept = default;

  ~array1() noexcept { _deallocate(_data); }

  array1(array1&& Other) noexcept
    : _data(std::exchange(Other._data, nullptr)), _size(std::exchange(Other._size, 0)),
      _capacity(std::exchange(Other._capacity, 0)) {}

  array1& operator=(array1&& Other) noexcept {
    if (this == &Other) return *this;
    _deallocate(_data);
    _data = std::exchange(Other._data, nullptr);
    _size = std::exchange(Other._size, 0);
    _capacity = std::exchange(Other._capacity, 0);
    return *this;
  }

  array1(const array1& Other) {
    if (auto res = assign(Other); !res) res.error().go_off();
  }

  array1& operator=(const array1& Other) {
    if (this == &Other) return *this;
    if (auto res = assign(Other); !res) res.error().go_off();
    return *this;
  }

  explicit array1(size_t Size, const source_line& sl = here()) requires std::default_initializable<T> {
    if (auto res = resize(Size); !res) res.error().go_off(sl);
  }

  array1(const T* Data, size_t Size, const source_line& sl = here()) {
    if (auto res = assign(Data, Size); !res) res.error().go_off(sl);
  }

  template<contiguous_range<T> Rg> explicit array1(Rg&& rg, const source_line& sl = here()) {
    if (auto res = assign(rg); !res) res.error().go_off(sl);
  }

  static std::expected<array1, error> create(size_t Size) requires std::default_initializable<T> {
    array1 a;
    if (auto res = a.resize(Size); !res) return res.error().relay();
    return a;
  }

  static std::expected<array1, error> create(const T* Data, size_t Size) {
    array1 a;
    if (auto res = a.assign(Data, Size); !res) return res.error().relay();
    return a;
  }

  template<contiguous_range<T> Rg> static std::expected<array1, error> create(Rg&& rg) {
    return create(yw::data(rg), yw::size(rg));
  }

  constexpr bool empty() const noexcept { return _size == 0; }
  constexpr size_t size() const noexcept { return _size; }
  constexpr size_t capacity() const noexcept { return _capacity; }
  constexpr T* data() noexcept { return _data; }
  constexpr const T* data() const noexcept { return _data; }
  constexpr T* begin() noexcept { return _data; }
  constexpr const T* begin() const noexcept { return _data; }
  constexpr T* end() noexcept { return _data + _size; }
  constexpr const T* end() const noexcept { return _data + _size; }
  constexpr T& front() noexcept { return _data[0]; }
  constexpr const T& front() const noexcept { return _data[0]; }
  constexpr T& back() noexcept { return _data[_size - 1]; }
  constexpr const T& back() const noexcept { return _data[_size - 1]; }
  constexpr T& operator[](size_t Index) noexcept { return _data[Index]; }
  constexpr const T& operator[](size_t Index) const noexcept { return _data[Index]; }

  void clear() noexcept { _size = 0; }

  void reset() noexcept {
    _deallocate(_data);
    _data = nullptr;
    _size = 0;
    _capacity = 0;
  }

  std::expected<void, error> reserve(size_t Capacity) {
    if (Capacity <= _capacity) return {};
    auto data = _allocate(Capacity);
    if (!data) return data.error().relay();
    if (_size != 0) std::memcpy(*data, _data, _size * sizeof(T));
    _deallocate(_data);
    _data = *data;
    _capacity = Capacity;
    return {};
  }

  std::expected<void, error> resize(size_t Size, const T& Value) {
    if (Size > _capacity)
      if (auto res = reserve(_preferred_capacity(Size)); !res) return res.error().relay();
    for (size_t i = _size; i < Size; ++i) _data[i] = Value;
    _size = Size;
    return {};
  }

  std::expected<void, error> resize(size_t Size) requires std::default_initializable<T> { return resize(Size, T{}); }

  std::expected<void, error> push_back(const T& Value) {
    if (_size == _capacity)
      if (auto res = reserve(_preferred_capacity(_size + 1)); !res) return res.error().relay();
    _data[_size++] = Value;
    return {};
  }

  std::expected<void, error> append(const T* Data, size_t Count) {
    if (!Data && Count != 0) return std::unexpected(error(errors::invalid_argument, "null array data"));
    if (Count > _max_capacity() - _size)
      return std::unexpected(error(errors::invalid_argument, "array size is too large"));
    const auto new_size = _size + Count;
    if (new_size > _capacity)
      if (auto res = reserve(_preferred_capacity(new_size)); !res) return res.error().relay();
    if (Count != 0) std::memcpy(_data + _size, Data, Count * sizeof(T));
    _size = new_size;
    return {};
  }

  template<contiguous_range<T> Rg> std::expected<void, error> append(Rg&& rg) {
    return append(yw::data(rg), yw::size(rg));
  }

  std::expected<void, error> assign(const T* Data, size_t Count) {
    if (!Data && Count != 0) return std::unexpected(error(errors::invalid_argument, "null array data"));
    if (auto res = reserve(Count); !res) return res.error().relay();
    if (Count != 0) std::memcpy(_data, Data, Count * sizeof(T));
    _size = Count;
    return {};
  }

  template<contiguous_range<T> Rg> std::expected<void, error> assign(Rg&& rg) {
    return assign(yw::data(rg), yw::size(rg));
  }
};

template<trivial T> class array2<T, cpu> {
  array1<T, cpu> _data;
  size_t _width = 0;
  size_t _height = 0;

  static std::expected<size_t, error> _count(size_t Width, size_t Height) {
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

  array2(size_t Width, size_t Height, const source_line& sl = here()) requires std::default_initializable<T> {
    if (auto res = resize(Width, Height); !res) res.error().go_off(sl);
  }

  array2(size_t Width, size_t Height, const T& Value, const source_line& sl = here()) {
    if (auto res = resize(Width, Height, Value); !res) res.error().go_off(sl);
  }

  static std::expected<array2, error> create(size_t Width, size_t Height) requires std::default_initializable<T> {
    array2 a;
    if (auto res = a.resize(Width, Height); !res) return res.error().relay();
    return a;
  }

  static std::expected<array2, error> create(size_t Width, size_t Height, const T& Value) {
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

  void clear() noexcept {
    _data.clear();
    _width = 0;
    _height = 0;
  }

  void reset() noexcept {
    _data.reset();
    _width = 0;
    _height = 0;
  }

  std::expected<void, error> resize(size_t Width, size_t Height, const T& Value) {
    auto count = _count(Width, Height);
    if (!count) return count.error().relay();
    if (auto res = _data.resize(*count, Value); !res) return res.error().relay();
    _width = Width;
    _height = Height;
    return {};
  }

  std::expected<void, error> resize(size_t Width, size_t Height) requires std::default_initializable<T> {
    return resize(Width, Height, T{});
  }
};

template<trivial T> class array3<T, cpu> {
  array1<T, cpu> _data;
  size_t _width = 0;
  size_t _height = 0;
  size_t _depth = 0;

  static std::expected<size_t, error> _count(size_t Width, size_t Height, size_t Depth) {
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

  array3(size_t Width, size_t Height, size_t Depth, const source_line& sl = here())
    requires std::default_initializable<T> {
    if (auto res = resize(Width, Height, Depth); !res) res.error().go_off(sl);
  }

  array3(size_t Width, size_t Height, size_t Depth, const T& Value, const source_line& sl = here()) {
    if (auto res = resize(Width, Height, Depth, Value); !res) res.error().go_off(sl);
  }

  static std::expected<array3, error> create(size_t Width, size_t Height, size_t Depth)
    requires std::default_initializable<T> {
    array3 a;
    if (auto res = a.resize(Width, Height, Depth); !res) return res.error().relay();
    return a;
  }

  static std::expected<array3, error> create(size_t Width, size_t Height, size_t Depth, const T& Value) {
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

  void clear() noexcept {
    _data.clear();
    _width = 0;
    _height = 0;
    _depth = 0;
  }

  void reset() noexcept {
    _data.reset();
    _width = 0;
    _height = 0;
    _depth = 0;
  }

  std::expected<void, error> resize(size_t Width, size_t Height, size_t Depth, const T& Value) {
    auto count = _count(Width, Height, Depth);
    if (!count) return count.error().relay();
    if (auto res = _data.resize(*count, Value); !res) return res.error().relay();
    _width = Width;
    _height = Height;
    _depth = Depth;
    return {};
  }

  std::expected<void, error> resize(size_t Width, size_t Height, size_t Depth) requires std::default_initializable<T> {
    return resize(Width, Height, Depth, T{});
  }
};
} // namespace yw
