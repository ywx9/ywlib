#pragma once
#include <core/core.h>
#include <core/property.h>

namespace yw {

inline constexpr auto is_ascii = []<char_type C>(C c) noexcept { return 0x20 <= c && c < 0x7F; };
inline constexpr auto is_digit = []<char_type C>(C c) noexcept { return '0' <= c && c <= '9'; };
inline constexpr auto is_lower = []<char_type C>(C c) noexcept { return 'a' <= c && c <= 'z'; };
inline constexpr auto is_upper = []<char_type C>(C c) noexcept { return 'A' <= c && c <= 'Z'; };
inline constexpr auto is_alpha = []<char_type C>(C c) noexcept { return is_lower(c) || is_upper(c); };
inline constexpr auto is_alnum = []<char_type C>(C c) noexcept { return is_alpha(c) || is_digit(c); };
inline constexpr auto is_xdigit = []<char_type C>(C c) noexcept {
  return is_digit(c) || (('a' <= c && c <= 'f') || ('A' <= c && c <= 'F'));
};

template<char_type C> struct string_view {
  static_assert(same_as<C, remove_cv<C>>);
  using value_type = C;

  const_property<const C*, string_view> data = nullptr;
  const_property<size_t, string_view> size = 0;

  constexpr string_view() noexcept = default;
  constexpr string_view(const string_view& o) noexcept = default;
  constexpr string_view& operator=(const string_view& o) noexcept = default;
  constexpr string_view(string_view&& o) noexcept = default;
  constexpr string_view& operator=(string_view&& o) noexcept = default;

  string_view(decltype(nullptr)) = delete;

  constexpr string_view(const C* s) noexcept pre(s != nullptr) : data(s), size(std::char_traits<C>::length(s)) {}

  constexpr string_view(const C* s, size_t n) noexcept pre(s != nullptr) : data(s), size(n) {}

  template<contiguous_iterator<C> It, sized_sentinel_for<It> S> //
  constexpr string_view(It i, S s) noexcept : data(yw::data(i)), size(s - i) {}

  template<contiguous_range<C> Rg> //
  constexpr string_view(Rg&& r) noexcept : string_view(yw::begin(r), yw::end(r)) {}

  constexpr bool empty() const noexcept { return size == 0; }
  constexpr const C* c_str() const noexcept { return data; }
  constexpr const C* begin() const noexcept { return data; }
  constexpr const C* end() const noexcept { return data + size; }
  constexpr C operator[](size_t i) const noexcept pre(i < size) { return data[i]; }
  constexpr C front() const noexcept pre(size > 0) { return data[0]; }
  constexpr C back() const noexcept pre(size > 0) { return data[size - 1]; }

  constexpr void remove_prefix(size_t n) noexcept pre(n <= size) { data += n, size -= n; }
  constexpr void remove_suffix(size_t n) noexcept pre(n <= size) { size -= n; }
  constexpr void swap(string_view& other) noexcept {
    yw::swap(data, other.data);
    yw::swap(size, other.size);
  }

  constexpr string_view substr(size_t pos, size_t n = npos) const noexcept pre(pos <= size) {
    return string_view(data + pos, yw::min(n, size - pos));
  }
  constexpr string_view subview(size_t pos, size_t n = npos) const noexcept pre(pos <= size) {
    return string_view(data + pos, yw::min(n, size - pos));
  }
};

template<char_type C> string_view(const C*) -> string_view<C>;
template<char_type C> string_view(const C*, size_t) -> string_view<C>;
template<contiguous_iterator It, sized_sentinel_for<It> S> requires char_type<iter_value_t<It>>
string_view(It, S) -> string_view<iter_value_t<It>>;
template<contiguous_range Rg> requires char_type<iter_value_t<Rg>> string_view(Rg&&) -> string_view<iter_value_t<Rg>>;

template<typename S, typename C = iter_value_t<S>> concept stringable = requires {
  requires char_type<C>;
  requires convertible_to<string_view<C>>;
  requires constructible<string_view<C>, S>;
};

template<char_type C> inline constexpr const C empty_string[]{0};

namespace internal {
inline constexpr size_t _string_preferred_capacity(size_t Size) noexcept {
  return yw::max(Size + 1, 2 * std::bit_ceil(Size), size_t(256));
}
} // namespace internal

template<char_type C> class string {
  static_assert(same_as<C, remove_cv<C>>);

  constexpr string(is_none auto, size_t Size) noexcept

public:
  using value_type = C;

  const_property<size_t, string> size = 0;
  const_property<size_t, string> capacity = 0;
  const_property<const C*, string> data = nullptr;
};

} // namespace yw
