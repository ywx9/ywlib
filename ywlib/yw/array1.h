#pragma once
#include <yw/backend.h>
#include <yw/property.h>

namespace yw {

template<trivial T, backend Backend = cpu> class array1;

template<trivial T> class array1<T, cpu> {
public:
  const_property<T*, array1> data = nullptr;
  const_property<size_t, array1> size = 0;
  const_property<size_t, array1> capacity = 0;

protected:
  std::allocator<T> _alloc{};
  using alloc_traits = std::allocator_traits<std::allocator<T>>;

  static constexpr size_t _max_capacity() noexcept { return std::numeric_limits<size_t>::max() / sizeof(T); }

  constexpr std::expected<T*, error> _allocate(size_t Capacity) {
    if (Capacity == 0) return nullptr;
    if (Capacity > _max_capacity())
      return std::unexpected(error(errors::invalid_argument, "array capacity is too large"));
    try {
      return alloc_traits::allocate(_alloc, Capacity);
    } catch (...) { return std::unexpected(error(errors::allocation_failed, "array allocation failed")); }
  }

  constexpr void _deallocate(T* Data, size_t Capacity) noexcept {
    if (Data) alloc_traits::deallocate(_alloc, Data, Capacity);
  }

  constexpr size_t _preferred_capacity(size_t Required) const noexcept {
    size_t preferred = capacity() == 0 ? size_t(8) : capacity();
    while (preferred < Required && preferred <= _max_capacity() / 2) preferred *= 2;
    return yw::max(preferred, Required);
  }

public:
  static constexpr auto backend = cpu;
  using value_type = T;

  constexpr array1() noexcept = default;

  constexpr ~array1() noexcept {
    clear();
    _deallocate(data(), capacity());
  }

  constexpr array1(array1&& Other) noexcept : data(Other.data()), size(Other.size()), capacity(Other.capacity()) {
    Other.data = nullptr;
    Other.size = 0;
    Other.capacity = 0;
  }

  constexpr array1& operator=(array1&& Other) noexcept {
    if (this == &Other) return *this;
    clear();
    _deallocate(data(), capacity());
    data = Other.data();
    size = Other.size();
    capacity = Other.capacity();
    Other.data = nullptr;
    Other.size = 0;
    Other.capacity = 0;
    return *this;
  }

  constexpr array1(const array1& Other) {
    if (auto res = assign(Other); !res) res.error().go_off();
  }

  constexpr array1& operator=(const array1& Other) {
    if (this == &Other) return *this;
    if (auto res = assign(Other); !res) res.error().go_off();
    return *this;
  }

  explicit constexpr array1(size_t Size, const source_line& sl = here()) requires std::default_initializable<T> {
    if (auto res = resize(Size); !res) res.error().go_off(sl);
  }

  constexpr array1(const T* Data, size_t Size, const source_line& sl = here()) {
    if (auto res = assign(Data, Size); !res) res.error().go_off(sl);
  }

  template<contiguous_range<T> Rg> explicit constexpr array1(Rg&& rg, const source_line& sl = here()) {
    if (auto res = assign(rg); !res) res.error().go_off(sl);
  }

  constexpr bool empty() const noexcept { return size() == 0; }
  constexpr T* begin() noexcept { return data(); }
  constexpr const T* begin() const noexcept { return data(); }
  constexpr T* end() noexcept { return data() + size(); }
  constexpr const T* end() const noexcept { return data() + size(); }
  constexpr T& front() noexcept { return data()[0]; }
  constexpr const T& front() const noexcept { return data()[0]; }
  constexpr T& back() noexcept { return data()[size() - 1]; }
  constexpr const T& back() const noexcept { return data()[size() - 1]; }
  constexpr T& operator[](size_t Index) noexcept { return data()[Index]; }
  constexpr const T& operator[](size_t Index) const noexcept { return data()[Index]; }

  constexpr void clear() noexcept {
    if consteval {
      for (size_t i = 0; i < size(); ++i) alloc_traits::destroy(_alloc, data() + i);
    }
    size = 0;
  }

  constexpr void reset() noexcept {
    clear();
    _deallocate(data(), capacity());
    data = nullptr;
    size = 0;
    capacity = 0;
  }

  constexpr std::expected<void, error> reserve(size_t Capacity) {
    if (Capacity <= capacity()) return {};
    auto new_data = _allocate(Capacity);
    if (!new_data) return new_data.error().relay();
    if consteval {
      for (size_t i = 0; i < size(); ++i) alloc_traits::construct(_alloc, *new_data + i, data()[i]);
      for (size_t i = 0; i < size(); ++i) alloc_traits::destroy(_alloc, data() + i);
    } else {
      if (size() != 0) std::memcpy(*new_data, data(), size() * sizeof(T));
    }
    _deallocate(data(), capacity());
    data = *new_data;
    capacity = Capacity;
    return {};
  }

  constexpr std::expected<void, error> resize(size_t Size, const T& Value) {
    if (Size > capacity())
      if (auto res = reserve(_preferred_capacity(Size)); !res) return res.error().relay();
    if consteval {
      for (size_t i = Size; i < size(); ++i) alloc_traits::destroy(_alloc, data() + i);
    }
    for (size_t i = size(); i < Size; ++i) alloc_traits::construct(_alloc, data() + i, Value);
    size = Size;
    return {};
  }

  constexpr std::expected<void, error> resize(size_t Size) requires std::default_initializable<T> { return resize(Size, T{}); }

  constexpr std::expected<void, error> push_back(const T& Value) {
    if (size() == capacity())
      if (auto res = reserve(_preferred_capacity(size() + 1)); !res) return res.error().relay();
    alloc_traits::construct(_alloc, data() + size(), Value);
    size = size() + 1;
    return {};
  }

  constexpr std::expected<void, error> append(const T* Data, size_t Count) {
    if (!Data && Count != 0) return std::unexpected(error(errors::invalid_argument, "null array data"));
    if (Count == 0) return {};
    if (Count > _max_capacity() - size())
      return std::unexpected(error(errors::invalid_argument, "array size is too large"));
    const auto old_size = size();
    const auto new_size = size() + Count;
    const auto data_offset = Data == data() ? size_t(0) : npos;
    if (new_size > capacity())
      if (auto res = reserve(_preferred_capacity(new_size)); !res) return res.error().relay();
    if (data_offset != npos) Data = data() + data_offset;
    if consteval {
      for (size_t i = 0; i < Count; ++i) alloc_traits::construct(_alloc, data() + old_size + i, Data[i]);
    } else {
      std::memcpy(data() + old_size, Data, Count * sizeof(T));
    }
    size = new_size;
    return {};
  }

  template<contiguous_range<T> Rg> constexpr std::expected<void, error> append(Rg&& rg) {
    return append(yw::data(rg), yw::size(rg));
  }

  constexpr std::expected<void, error> assign(const T* Data, size_t Count) {
    if (!Data && Count != 0) return std::unexpected(error(errors::invalid_argument, "null array data"));
    if (auto res = reserve(Count); !res) return res.error().relay();
    if consteval {
      for (size_t i = 0; i < Count; ++i) {
        if (i < size()) data()[i] = Data[i];
        else alloc_traits::construct(_alloc, data() + i, Data[i]);
      }
      for (size_t i = Count; i < size(); ++i) alloc_traits::destroy(_alloc, data() + i);
    } else {
      if (Count != 0) std::memcpy(data(), Data, Count * sizeof(T));
    }
    size = Count;
    return {};
  }

  template<contiguous_range<T> Rg> constexpr std::expected<void, error> assign(Rg&& rg) {
    return assign(yw::data(rg), yw::size(rg));
  }
};
} // namespace yw
