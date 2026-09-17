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
  constexpr string_view(It i, S s) noexcept : data(std::to_address(i)), size(s - i) {}

  template<contiguous_range<C> Rg> //
  constexpr string_view(Rg&& r) noexcept : string_view(yw::data(r), yw::size(r)) {}

  constexpr bool empty() const noexcept { return size == 0; }
  constexpr const C* c_str() const noexcept { return data; }
  constexpr const C* begin() const noexcept { return data; }
  constexpr const C* end() const noexcept { return data + size; }
  constexpr C operator[](size_t i) const noexcept pre(i < size) { return data[i]; }
  constexpr C front() const noexcept pre(size > 0) { return data[0]; }
  constexpr C back() const noexcept pre(size > 0) { return data[size - 1]; }

  constexpr void remove_prefix(size_t n) noexcept pre(n <= size) { data += n, size -= n; }
  constexpr void remove_suffix(size_t n) noexcept pre(n <= size) { size -= n; }
  constexpr void swap(string_view& o) noexcept {
    auto data_temp = data();
    data = o.data();
    o.data = data_temp;
    auto size_temp = size();
    size = o.size();
    o.size = size_temp;
  }

  constexpr string_view substr(size_t pos, size_t n = npos) const noexcept pre(pos <= size()) {
    return string_view(data() + pos, yw::min(n, size() - pos));
  }
  constexpr string_view subview(size_t pos, size_t n = npos) const noexcept pre(pos <= size()) {
    return string_view(data() + pos, yw::min(n, size() - pos));
  }
};

template<char_type C> string_view(const C*) -> string_view<C>;
template<char_type C> string_view(const C*, size_t) -> string_view<C>;
template<contiguous_iterator It, sized_sentinel_for<It> S> requires char_type<iter_value_t<It>>
string_view(It, S) -> string_view<iter_value_t<It>>;
template<contiguous_range Rg> requires char_type<iter_value_t<Rg>> string_view(Rg&&) -> string_view<iter_value_t<Rg>>;

template<typename S, typename C = iter_value_t<S>> concept stringable = requires {
  requires char_type<C>;
  requires convertible_to<S, string_view<C>>;
  requires constructible<string_view<C>, S>;
};

template<char_type C> constexpr bool operator==(const string_view<C>& a, const string_view<C>& b) noexcept {
  return a.size() == b.size() && std::char_traits<C>::compare(a.data(), b.data(), a.size()) == 0;
}

template<char_type C, stringable<C> S> requires(!same_as<remove_cvref<S>, string_view<C>>)
constexpr bool operator==(const string_view<C>& a, S&& b) noexcept {
  return a == string_view<C>(static_cast<S&&>(b));
}

template<char_type C, stringable<C> S> requires(!same_as<remove_cvref<S>, string_view<C>>)
constexpr bool operator==(S&& a, const string_view<C>& b) noexcept {
  return b == static_cast<S&&>(a);
}

template<char_type C> constexpr auto operator<=>(const string_view<C>& a, const string_view<C>& b) noexcept {
  const auto cmp = std::char_traits<C>::compare(a.data(), b.data(), yw::min(a.size(), b.size()));
  return cmp != 0 ? cmp <=> 0 : a.size() <=> b.size();
}

template<char_type C, stringable<C> S> requires(!same_as<remove_cvref<S>, string_view<C>>)
constexpr auto operator<=>(const string_view<C>& a, S&& b) noexcept {
  return a <=> string_view<C>(static_cast<S&&>(b));
}

template<char_type C, stringable<C> S> requires(!same_as<remove_cvref<S>, string_view<C>>)
constexpr auto operator<=>(S&& a, const string_view<C>& b) noexcept {
  return string_view<C>(static_cast<S&&>(a)) <=> b;
}

template<char_type C> inline constexpr const C empty_string[]{0};

namespace internal {
inline constexpr size_t _string_preferred_capacity(size_t Size) noexcept {
  return yw::max(Size + 1, 2 * std::bit_ceil(Size), size_t(256));
}
} // namespace internal

template<char_type C> class string {
  static_assert(same_as<C, remove_cv<C>>);
  constexpr string(is_none auto, size_t Size) noexcept
    : size(Size), capacity(internal::_string_preferred_capacity(Size)), data(yw::allocate<C>(capacity)) {}

public:
  using value_type = C;

  const_property<size_t, string> size = 0;
  const_property<size_t, string> capacity = 0;
  const_property<C*, string> data = nullptr;

  constexpr ~string() noexcept { yw::deallocate(data()); }
  constexpr string() noexcept = default;

  constexpr string(const string& s) noexcept : string(s.data(), s.size()) {}

  constexpr string& operator=(const string& s) noexcept {
    if (this != &s) {
      const auto new_capacity = internal::_string_preferred_capacity(s.size());
      auto new_data = yw::allocate<C>(new_capacity);
      std::ranges::copy_n(s.data(), s.size(), new_data);
      new_data[s.size()] = C();

      yw::deallocate(data());
      size = s.size();
      capacity = new_capacity;
      data = new_data;
    }
    return *this;
  }

  constexpr string(string&& o) noexcept : data(o.data()), size(o.size()), capacity(o.capacity()) {
    o.data = nullptr, o.size = 0, o.capacity = 0;
  }

  constexpr string& operator=(string&& o) noexcept {
    if (this != &o) {
      yw::deallocate(data());
      data = o.data();
      size = o.size();
      capacity = o.capacity();
      o.data = nullptr;
      o.size = 0;
      o.capacity = 0;
    }
    return *this;
  }

  constexpr string(size_t n) noexcept : string(n, C()) {}

  constexpr string(size_t n, C c) noexcept : string(none(), n) {
    std::ranges::fill_n(data(), size(), c);
    data.ref()[size()] = C();
  }

  template<stringable<C> S> requires same_as<remove_cvref<S>, string_view<C>>
  constexpr string(S&& s) noexcept : string(none(), s.size()) {
    std::ranges::copy_n(s.data(), s.size(), data());
    data.ref()[size()] = C();
  }

  template<stringable<C> S> requires(!same_as<remove_cvref<S>, string_view<C>>)
  constexpr string(S&& s) noexcept : string(string_view<C>(s)) {}

  template<stringable<C> S> constexpr string& operator=(S&& s) noexcept {
    const auto sv = string_view<C>(s);
    reserve(sv.size());
    std::ranges::copy_n(sv.data(), sv.size(), data());
    size = sv.size();
    data.ref()[size()] = C();
    return *this;
  }

  constexpr operator string_view<C>() const noexcept { return {data(), size()}; }
  constexpr auto view() const noexcept { return string_view<C>{data(), size()}; }

  constexpr bool empty() const noexcept { return size() == 0; }
  constexpr const C* c_str() const noexcept { return data(); }
  constexpr C* begin() noexcept { return data(); }
  constexpr const C* begin() const noexcept { return data(); }
  constexpr C* end() noexcept { return data() + size(); }
  constexpr const C* end() const noexcept { return data() + size(); }
  constexpr C& operator[](size_t i) noexcept { return data()[i]; }
  constexpr const C& operator[](size_t i) const noexcept { return data()[i]; }
  constexpr C& front() noexcept { return data()[0]; }
  constexpr const C& front() const noexcept { return data()[0]; }
  constexpr C& back() noexcept { return data()[size() - 1]; }
  constexpr const C& back() const noexcept { return data()[size() - 1]; }

  constexpr void clear() noexcept {
    size = 0;
    if (data()) data.ref()[0] = C();
  }

  constexpr void reserve(size_t Capacity) noexcept {
    if (Capacity + 1 <= capacity()) return;
    const auto new_capacity = internal::_string_preferred_capacity(Capacity);
    C* Data = yw::allocate<C>(new_capacity);
    std::ranges::copy_n(data(), size(), Data);
    Data[size()] = C();
    yw::deallocate(data());
    data = Data;
    capacity = new_capacity;
  }

  constexpr void resize(size_t n, C c = C()) noexcept {
    reserve(n);
    if (size() < n) std::ranges::fill_n(data() + size(), n - size(), c);
    size = n;
    data.ref()[size()] = C();
  }

  constexpr void pop_back() noexcept {
    if (size() > 0) size = size() - 1;
    if (data()) data.ref()[size()] = C();
  }

  constexpr void push_back(C c) noexcept {
    reserve(size() + 1);
    data()[size()] = c;
    size = size() + 1;
    data.ref()[size()] = C();
  }

  constexpr string& append(stringable<C> auto&& s) noexcept {
    auto sv = string_view<C>(s);
    if (sv.empty()) return *this;
    const bool overlaps = data() && sv.data() < data() + size() && data() < sv.data() + sv.size();
    const auto new_size = size() + sv.size();
    const bool need_realloc = overlaps || new_size + 1 > capacity();
    if (need_realloc) {
      const auto new_capacity = internal::_string_preferred_capacity(new_size);
      const auto new_data = yw::allocate<C>(new_capacity);
      std::ranges::copy_n(data(), size(), new_data);
      std::ranges::copy_n(sv.data(), sv.size(), new_data + size());
      yw::deallocate(data());
      data = new_data;
      size = new_size;
      capacity = new_capacity;
    } else {
      std::ranges::copy_n(sv.data(), sv.size(), data() + size());
      size = new_size;
    }
    data.ref()[size()] = C();
    return *this;
  }

  template<stringable<C> S> constexpr string& operator+=(S&& s) noexcept { return append(static_cast<S&&>(s)); }

  constexpr string substr(size_t Pos = 0, size_t Count = npos) const noexcept {
    if (Pos >= size()) return string();
    return string(data() + Pos, yw::min(Count, size() - Pos));
  }

  constexpr string_view<C> subview(size_t Pos = 0, size_t Count = npos) const noexcept {
    if (Pos >= size()) return string_view<C>();
    return string_view<C>(data() + Pos, yw::min(Count, size() - Pos));
  }

  constexpr void swap(string& other) noexcept {
    auto tmp_data = data();
    auto tmp_size = size();
    auto tmp_capacity = capacity();
    data = other.data;
    size = other.size;
    capacity = other.capacity;
    other.data = tmp_data;
    other.size = tmp_size;
    other.capacity = tmp_capacity;
  }
};

template<char_type C> string(size_t, C) -> string<C>;
template<char_type C> string(string_view<C>) -> string<C>;
template<stringable S> string(S&&) -> string<iter_value_t<S>>;

template<char_type C> constexpr bool operator==(const string<C>& a, const string<C>& b) noexcept {
  return string_view<C>(a) == string_view<C>(b);
}

template<char_type C, stringable<C> S>
requires(!same_as<remove_cvref<S>, string<C>> && !same_as<remove_cvref<S>, string_view<C>>)
constexpr bool operator==(const string<C>& a, S&& b) noexcept {
  return string_view<C>(a) == string_view<C>(static_cast<S&&>(b));
}

template<char_type C, stringable<C> S>
requires(!same_as<remove_cvref<S>, string<C>> && !same_as<remove_cvref<S>, string_view<C>>)
constexpr bool operator==(S&& a, const string<C>& b) noexcept {
  return b == static_cast<S&&>(a);
}

template<char_type C> constexpr auto operator<=>(const string<C>& a, const string<C>& b) noexcept {
  return string_view<C>(a) <=> string_view<C>(b);
}

template<char_type C, stringable<C> S>
requires(!same_as<remove_cvref<S>, string<C>> && !same_as<remove_cvref<S>, string_view<C>>)
constexpr auto operator<=>(const string<C>& a, S&& b) noexcept {
  return string_view<C>(a) <=> string_view<C>(static_cast<S&&>(b));
}

template<char_type C, stringable<C> S>
requires(!same_as<remove_cvref<S>, string<C>> && !same_as<remove_cvref<S>, string_view<C>>)
constexpr auto operator<=>(S&& a, const string<C>& b) noexcept {
  return string_view<C>(static_cast<S&&>(a)) <=> string_view<C>(b);
}

namespace internal {
inline constexpr size_t _bool_to_str_sufficient_buffer_size = 5;
template<char_type C> constexpr C* _bool_to_str(bool b, C* o) noexcept {
  if (b) *o++ = 't', *o++ = 'r', *o++ = 'u', *o++ = 'e';
  else *o++ = 'f', *o++ = 'a', *o++ = 'l', *o++ = 's', *o++ = 'e';
  return o;
}
inline constexpr size_t _uint_to_str_sufficient_buffer_size = 20;
template<char_type C> constexpr C* _uint_to_str(uint64_t u, C* o) noexcept {
  if (u == 0) {
    *o++ = '0';
    return o;
  }
  unsigned digits = 0;
  for (auto t = u; t > 0; t /= 10) ++digits;
  for (auto i = digits; i > 0; --i, u /= 10) o[i - 1] = static_cast<C>('0' + (u % 10));
  return o + digits;
}
inline constexpr size_t _int_to_str_sufficient_buffer_size = 20;
template<char_type C> constexpr C* _int_to_str(int64_t i, C* o) noexcept {
  if (i < 0) *o++ = '-', i = -i;
  return _uint_to_str(static_cast<uint64_t>(i), o);
}
inline constexpr size_t _float_to_str_sufficient_buffer_size = 32;
template<char_type C> constexpr C* _float_to_str(long double f, C* o) noexcept {
  if (std::isnan(f)) {
    *o++ = 'n', *o++ = 'a', *o++ = 'n';
    return o;
  }
  const bool negative = std::signbit(f);
  if (negative) f = -f;
  if (std::isinf(f)) {
    if (negative) *o++ = '-';
    *o++ = 'i', *o++ = 'n', *o++ = 'f';
    return o;
  } else if (f == 0) {
    *o++ = '0';
    return o;
  }
  constexpr uint64_t scale = 1'000'000;
  constexpr long double threshold = static_cast<long double>(uint64_t(1) << 63);
  uint64_t exponent = 0;
  if (f >= threshold) // scientific notation
    while (f >= 10) f /= 10, ++exponent;
  uint64_t int_part = static_cast<uint64_t>(f);
  uint64_t frac_part = static_cast<uint64_t>((f - int_part) * scale + 0.5L);
  if (frac_part >= scale) {
    ++int_part, frac_part = 0;
    if (exponent && int_part >= 10) int_part = 1, ++exponent;
  }
  if (int_part == 0 && frac_part == 0) {
    *o++ = '0';
    return o;
  }
  if (negative) *o++ = '-';
  o = _uint_to_str(int_part, o);
  if (frac_part != 0) {
    uint64_t divisor = 100'000;
    while (frac_part % 10 == 0) frac_part /= 10, divisor /= 10;
    *o++ = '.';
    for (; divisor > 0; frac_part %= divisor, divisor /= 10) *o++ = static_cast<C>('0' + (frac_part / divisor));
  }
  if (exponent) {
    *o++ = 'e';
    o = _uint_to_str(exponent, o);
  }
  return o;
}
} // namespace internal

template<char_type C> constexpr string<C> vtos(arithmetic auto v) noexcept {
  using T = decltype(v);
  string<C> result;
  if constexpr (is_bool<T>) {
    result.resize(internal::_bool_to_str_sufficient_buffer_size);
    internal::_bool_to_str(v, result.data());
  } else if constexpr (float_type<T>) {
    result.resize(internal::_float_to_str_sufficient_buffer_size);
    internal::_float_to_str(v, result.data());
  } else if constexpr (unsigned_integral<T>) {
    result.resize(internal::_uint_to_str_sufficient_buffer_size);
    internal::_uint_to_str(v, result.data());
  } else if constexpr (signed_integral<T>) {
    result.resize(internal::_int_to_str_sufficient_buffer_size);
    internal::_int_to_str(v, result.data());
  } else static_assert(always_false<T>, "Unsupported arithmetic type for vtos.");
  return result;
}
} // namespace yw
