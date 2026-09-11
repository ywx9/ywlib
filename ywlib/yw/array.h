#pragma once
#include <yw/backend.h>
#include <yw/property.h>
#include <yw/result.h>

namespace yw {

template<typename T> class array {
public:
  const_property<T*, array> data = nullptr;
  const_property<size_t, array> size = 0;
  const_property<size_t, array> capacity = 0;

protected:
  std::allocator<T> _alloc{};
  using alloc_traits = std::allocator_traits<std::allocator<T>>;
  static constexpr size_t _max_capacity = std::numeric_limits<size_t>::max() / sizeof(T);

  static constexpr size_t _preferred_capacity(size_t Required) noexcept {
    if (Required == 0) return 0;
    if (Required > _max_capacity / 2) return _max_capacity;
    size_t preferred = std::bit_floor(Required) << 1;
    return yw::min(yw::max(preferred, Required), _max_capacity);
  }

  constexpr result<T*> _allocate(size_t Capacity) {
    if (Capacity == 0) return nullptr;
    if (Capacity > _max_capacity) return fail<T*>(errors::invalid_argument, "array capacity is too large");
    try {
      return alloc_traits::allocate(_alloc, Capacity);
    } catch (...) { return fail<T*>(errors::allocation_failed, "array allocation failed"); }
  }

public:
  using value_type = T;

  constexpr ~array() noexcept {
    clear();
    if (data()) alloc_traits::deallocate(_alloc, data(), capacity());
  }
  constexpr array() noexcept = default;

  constexpr array(array&& Other) noexcept : data(Other.data()), size(Other.size()), capacity(Other.capacity()) {
    Other.data = nullptr;
    Other.size = 0;
    Other.capacity = 0;
  }

  constexpr array(const array& Other) {
    if (auto res = assign(Other); !res) res.error().go_off();
  }

  explicit constexpr array(size_t Size) requires std::default_initializable<T> {
    if (auto res = resize(Size); !res) res.error().go_off();
  }

  constexpr array(const T* Data, size_t Size) {
    if (auto res = assign(Data, Size); !res) res.error().go_off();
  }

  template<std::ranges::input_range Rg> requires std::convertible_to<std::ranges::range_reference_t<Rg>, T>
  explicit constexpr array(Rg&& r) {
    if (auto res = assign(static_cast<Rg&&>(r)); !res) res.error().go_off();
  }

  constexpr array& operator=(array&& Other) noexcept {
    if (this == &Other) return *this;
    clear();
    if (data()) alloc_traits::deallocate(_alloc, data(), capacity());
    data = Other.data();
    size = Other.size();
    capacity = Other.capacity();
    Other.data = nullptr;
    Other.size = 0;
    Other.capacity = 0;
    return *this;
  }

  constexpr array& operator=(const array& Other) {
    if (this == &Other) return *this;
    if (auto res = assign(Other); !res) res.error().go_off();
    return *this;
  }

  constexpr bool empty() const noexcept { return size() == 0; }
  constexpr T* begin() noexcept { return data(); }
  constexpr const T* begin() const noexcept { return data(); }
  constexpr T* end() noexcept { return data() + size(); }
  constexpr const T* end() const noexcept { return data() + size(); }
  constexpr T& front() noexcept { return *data(); }
  constexpr const T& front() const noexcept { return *data(); }
  constexpr T& back() noexcept { return *(data() + size() - 1); }
  constexpr const T& back() const noexcept { return *(data() + size() - 1); }
  constexpr T& operator[](size_t Index) noexcept { return *(data() + Index); }
  constexpr const T& operator[](size_t Index) const noexcept { return *(data() + Index); }

  constexpr void clear() noexcept {
    for (size_t i = 0; i < size(); ++i) alloc_traits::destroy(_alloc, data() + i);
    size = 0;
  }

  constexpr result<void> reserve(size_t Capacity) {
    if (Capacity <= capacity()) return {};
    auto new_data = _allocate(Capacity);
    if (!new_data) return new_data.relay();
    if (data()) {
      for (size_t i = 0; i < size(); ++i) alloc_traits::construct(_alloc, *new_data + i, move(data()[i]));
      for (size_t i = 0; i < size(); ++i) alloc_traits::destroy(_alloc, data() + i);
      alloc_traits::deallocate(_alloc, data(), capacity());
    }
    data = *new_data;
    capacity = Capacity;
    return {};
  }

  constexpr result<void> resize(size_t Count) requires std::default_initializable<T> {
    if (Count <= capacity()) {
      if (Count > size())
        for (size_t i = size(); i < Count; ++i) alloc_traits::construct(_alloc, data() + i);
      else
        for (size_t i = Count; i < size(); ++i) alloc_traits::destroy(_alloc, data() + i);
      size = Count;
      return {};
    }
    if (auto res = reserve(_preferred_capacity(Count)); !res) return res.relay();
    for (size_t i = size(); i < Count; ++i) alloc_traits::construct(_alloc, data() + i);
    size = Count;
    return {};
  }

  constexpr result<void> resize(size_t Count, const T& Value) {
    if (Count <= capacity()) {
      if (Count > size())
        for (size_t i = size(); i < Count; ++i) alloc_traits::construct(_alloc, data() + i, Value);
      else
        for (size_t i = Count; i < size(); ++i) alloc_traits::destroy(_alloc, data() + i);
      size = Count;
      return {};
    }
    if (auto res = reserve(_preferred_capacity(Count)); !res) return res.relay<void>();
    for (size_t i = size(); i < Count; ++i) alloc_traits::construct(_alloc, data() + i, Value);
    size = Count;
    return {};
  }

  constexpr result<void> push_back(const T& Value) {
    if (size() == capacity())
      if (auto res = reserve(_preferred_capacity(size() + 1)); !res) return res.relay<void>();
    alloc_traits::construct(_alloc, data() + size(), Value);
    size = size() + 1;
    return {};
  }

  template<typename... As> requires constructible<T, As...> constexpr result<void> emplace_back(As&&... args) {
    if (size() == capacity())
      if (auto res = reserve(_preferred_capacity(size() + 1)); !res) return res.relay<void>();
    alloc_traits::construct(_alloc, data() + size(), static_cast<As&&>(args)...);
    size = size() + 1;
    return {};
  }

  constexpr result<void> append(const T* Data, size_t Count) {
    if (!Data && Count > 0) return std::unexpected(error(errors::invalid_argument, "null array data"));
    if (Count == 0) return {};
    if (Count > _max_capacity - size()) return fail<void>(errors::invalid_argument, "array size is too large");
    const auto old_count = size();
    const auto new_size = size() + Count;
    const auto data_offset = Data == data() ? size_t(0) : npos;
    if (new_size > capacity())
      if (auto res = reserve(_preferred_capacity(new_size)); !res) return res.relay<void>();
    if (data_offset != npos) Data = data() + data_offset;
    for (size_t i = old_count; i < new_size; ++i) alloc_traits::construct(_alloc, data() + i, Data[i - old_count]);
    size = new_size;
    return {};
  }

  template<std::ranges::input_range Rg> requires std::convertible_to<std::ranges::range_reference_t<Rg>, T>
  constexpr result<void> append(Rg&& r) {
    if constexpr (std::ranges::contiguous_range<Rg> && std::ranges::sized_range<Rg>) {
      return append(std::ranges::data(r), std::ranges::size(r));
    } else {
      if constexpr (std::ranges::sized_range<Rg>) {
        const auto Count = std::ranges::size(r);
        if (Count == 0) return {};
        if (Count > _max_capacity - size()) return fail<void>(errors::invalid_argument, "array size is too large");
        if (auto res = reserve(_preferred_capacity(size() + Count)); !res) return res.relay<void>();
      }
      for (auto&& value : r)
        if (auto res = push_back(static_cast<T>(value)); !res) return res.relay<void>();
      return {};
    }
  }

  constexpr result<void> assign(const T* Data, size_t Count) {
    if (!Data && Count > 0) return fail<void>(errors::invalid_argument, "null array data");
    if (Count > _max_capacity) return fail<void>(errors::invalid_argument, "array size is too large");
    const auto old_count = size();
    if (Count > capacity())
      if (auto res = reserve(_preferred_capacity(Count)); !res) return res.relay<void>();
    const auto assigned = yw::min(old_count, Count);
    for (size_t i = 0; i < assigned; ++i) data()[i] = Data[i];
    for (size_t i = assigned; i < Count; ++i) alloc_traits::construct(_alloc, data() + i, Data[i]);
    for (size_t i = Count; i < old_count; ++i) alloc_traits::destroy(_alloc, data() + i);
    size = Count;
    return {};
  }

  template<std::ranges::input_range Rg> requires std::convertible_to<std::ranges::range_reference_t<Rg>, T>
  constexpr result<void> assign(Rg&& r) {
    if constexpr (std::ranges::contiguous_range<Rg> && std::ranges::sized_range<Rg>) {
      return assign(std::ranges::data(r), std::ranges::size(r));
    } else {
      clear();
      return append(static_cast<Rg&&>(r));
    }
  }
};
} // namespace yw
