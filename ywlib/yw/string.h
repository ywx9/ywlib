#pragma once
#include "yw/core.h"

#ifdef ywlib_header_name
#error "ywlib_header_name already defined unexpectedly"
#endif
#define ywlib_header_name "yw/string.h"

namespace yw {

/// MARK: string

template<char_type C> class string {
  static_assert(same_as<C, remove_cv<C>>);
  static constexpr C _empty[1] = {C()};

  C* _ptr = nullptr;
  size_t _size = 0;
  size_t _capacity = 0;

  constexpr size_t _get_preferred_capacity(size_t Size) const noexcept {
    return yw::max(Size, 2 * std::bit_ceil(Size), size_t(256));
  }

  constexpr bool _has_capacity_for(size_t Size) const noexcept { return _capacity > Size; }

  constexpr void _set_terminator() noexcept {
    if (_ptr) _ptr[_size] = C();
  }

  constexpr string(is_none auto, size_t Size) // constructing in an uninitialized state
    : _size(Size), _capacity(_get_preferred_capacity(Size + 1)), _ptr(new C[_capacity]) {}

public:
  constexpr ~string() noexcept { delete[] _ptr; }
  constexpr string() noexcept = default;
  constexpr string(string&& Other) noexcept
    : _ptr(std::exchange(Other._ptr, nullptr)), _size(std::exchange(Other._size, 0)),
      _capacity(std::exchange(Other._capacity, 0)) {}
  constexpr string& operator=(string&& Other) noexcept {
    if (this == &Other) return *this;
    delete[] _ptr;
    _ptr = std::exchange(Other._ptr, nullptr);
    _size = std::exchange(Other._size, 0);
    _capacity = std::exchange(Other._capacity, 0);
    return *this;
  }
  constexpr string(const string& Other) : string(std::basic_string_view<C>(Other)) {}
  constexpr string& operator=(const string& Other) {
    if (this == &Other) return *this;
    delete[] _ptr;
    _size = Other._size;
    _capacity = _get_preferred_capacity(_size + 1);
    _ptr = new C[_capacity];
    for (size_t i = 0; i < _size; ++i) _ptr[i] = Other._ptr[i];
    _ptr[_size] = C();
    return *this;
  }
  constexpr string(size_t Size, C FillChar = C()) : string(none{}, Size) {
    for (size_t i = 0; i < _size; ++i) _ptr[i] = FillChar;
    _ptr[_size] = C();
  }
  template<stringable<C> S> requires same_as<remove_cvref<S>, std::basic_string_view<C>>
  constexpr string(S&& View) : string(none{}, View.size()) {
    for (size_t i = 0; i < _size; ++i) _ptr[i] = View[i];
    _ptr[_size] = C();
  }
  template<stringable<C> S>
  requires different_from<remove_cvref<S>, string> && different_from<remove_cvref<S>, std::basic_string_view<C>>
  constexpr string(S&& Other) : string(std::basic_string_view<C>(Other)) {}

  constexpr operator std::basic_string_view<C>() const noexcept { return {_ptr, _size}; }
  constexpr auto view() const noexcept { return std::basic_string_view<C>(_ptr, _size); }

  constexpr bool empty() const noexcept { return _size == 0; }
  constexpr size_t size() const noexcept { return _size; }
  constexpr size_t capacity() const noexcept { return _capacity; }

  constexpr auto data() noexcept { return _ptr; }
  constexpr auto data() const noexcept { return _ptr; }
  constexpr auto c_str() const noexcept { return _ptr ? _ptr : _empty; }
  constexpr auto begin() noexcept { return _ptr; }
  constexpr auto begin() const noexcept { return _ptr; }
  constexpr auto end() noexcept { return _ptr + _size; }
  constexpr auto end() const noexcept { return _ptr + _size; }

  constexpr auto& front() { return _ptr[0]; }
  constexpr auto& front() const { return _ptr[0]; }
  constexpr auto& back() { return _ptr[_size - 1]; }
  constexpr auto& back() const { return _ptr[_size - 1]; }

  constexpr auto& operator[](size_t Index) { return _ptr[Index]; }
  constexpr auto& operator[](size_t Index) const { return _ptr[Index]; }

  constexpr void clear() noexcept {
    _size = 0;
    _set_terminator();
  }

  constexpr void reserve(size_t Size) {
    if (_has_capacity_for(Size)) return;
    const auto new_capacity = _get_preferred_capacity(Size + 1);
    auto* new_ptr = new C[new_capacity];
    for (size_t i = 0; i < _size; ++i) new_ptr[i] = _ptr[i];
    new_ptr[_size] = C();
    delete[] _ptr;
    _ptr = new_ptr;
    _capacity = new_capacity;
  }

  constexpr void resize(size_t Size, C FillChar = C()) {
    reserve(Size);
    for (size_t i = _size; i < Size; ++i) _ptr[i] = FillChar;
    _size = Size;
    _ptr[_size] = C();
  }

  constexpr string& append(std::basic_string_view<C> View) {
    if (View.empty()) return *this;
    const auto append_size = View.size();
    const auto old_size = _size;
    const auto* src = View.data();
    const bool overlaps = _ptr && src < _ptr + _size && _ptr < src + append_size;
    const auto offset = overlaps ? size_t(src - _ptr) : size_t(0);
    reserve(_size + append_size);
    if (overlaps) src = _ptr + offset;
    for (size_t i = 0; i < append_size; ++i) _ptr[old_size + i] = src[i];
    _size += append_size;
    _ptr[_size] = C();
    return *this;
  }

  template<stringable<C> S> constexpr string& append(S&& Other) {
    return append(std::basic_string_view<C>(static_cast<S&&>(Other)));
  }

  template<stringable<C> S> constexpr string& operator+=(S&& Other) { return append(static_cast<S&&>(Other)); }
  constexpr string& operator+=(C Char) {
    reserve(_size + 1);
    _ptr[_size++] = Char;
    _ptr[_size] = C();
    return *this;
  }

  constexpr string substr(size_t Pos = 0, size_t Count = npos) const {
    if (Pos >= _size) return {};
    return string(std::basic_string_view<C>(_ptr + Pos, yw::min(Count, _size - Pos)));
  }

  constexpr void swap(string& Other) noexcept {
    std::swap(_ptr, Other._ptr);
    std::swap(_size, Other._size);
    std::swap(_capacity, Other._capacity);
  }

  friend constexpr bool operator==(const string& Lhs, const string& Rhs) noexcept { return Lhs.view() == Rhs.view(); }
  friend constexpr auto operator<=>(const string& Lhs, const string& Rhs) noexcept { return Lhs.view() <=> Rhs.view(); }
};

/// MARK: bool_to_string

template<char_type C> constexpr string<C> bool_to_string(bool Value) {
  constexpr C _true[] = {'t', 'r', 'u', 'e'};
  constexpr C _false[] = {'f', 'a', 'l', 's', 'e'};
  return Value ? string<C>(std::basic_string_view<C>(_true, 4)) : string<C>(std::basic_string_view<C>(_false, 5));
}
constexpr string<char> bool_to_string(bool Value) { return bool_to_string<char>(Value); }

/// MARK: uint_to_string

template<char_type C> constexpr string<C> uint_to_string(unsigned_integral auto Value) {
  if (Value == 0) return string<C>(1, C('0'));
  auto u = static_cast<uint64_t>(Value);
  unsigned keta = 0;
  for (auto u_ = u; u_ > 0; u_ /= 10) ++keta;
  string<C> result(keta);
  for (auto p = result.data() + keta; u > 0; u /= 10) *--p = static_cast<C>('0' + (u % 10));
  return result;
}
constexpr string<char> uint_to_string(unsigned_integral auto Value) { return uint_to_string<char>(Value); }

} // namespace yw

#undef ywlib_header_name
