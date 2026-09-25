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

template<char_type C> inline constexpr const C empty_string[]{0};

///--------------------------------------------------------------------------///
/// MARK: string_view

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

  constexpr string_view(const C* s) noexcept ywlib_pre(s != nullptr) : data(s), size(std::char_traits<C>::length(s)) {}

  constexpr string_view(const C* s, size_t n) noexcept ywlib_pre(s != nullptr) : data(s), size(n) {}

  template<contiguous_iterator<C> It, sized_sentinel_for<It> S> //
  constexpr string_view(It i, S s) noexcept : data(std::to_address(i)), size(s - i) {}

  template<contiguous_range<C> Rg> //
  constexpr string_view(Rg&& r) noexcept : string_view(yw::data(r), yw::size(r)) {}

  constexpr bool empty() const noexcept { return size == 0; }
  constexpr const C* c_str() const noexcept { return data() ? data() : empty_string<C>; }
  constexpr const C* begin() const noexcept { return data; }
  constexpr const C* end() const noexcept { return data + size; }
  constexpr C operator[](size_t i) const noexcept ywlib_pre(i < size) { return data[i]; }
  constexpr C front() const noexcept ywlib_pre(size > 0) { return data[0]; }
  constexpr C back() const noexcept ywlib_pre(size > 0) { return data[size - 1]; }

  constexpr void remove_prefix(size_t n) noexcept ywlib_pre(n <= size) { data += n, size -= n; }
  constexpr void remove_suffix(size_t n) noexcept ywlib_pre(n <= size) { size -= n; }
  constexpr void swap(string_view& o) noexcept {
    std::ranges::swap(data.ref(), o.data.ref());
    std::ranges::swap(size.ref(), o.size.ref());
  }

  constexpr string_view substr(size_t pos, size_t n = npos) const noexcept ywlib_pre(pos <= size()) {
    return string_view(data() + pos, yw::min(n, size() - pos));
  }
  constexpr string_view subview(size_t pos, size_t n = npos) const noexcept ywlib_pre(pos <= size()) {
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

namespace internal {
inline constexpr size_t _string_preferred_capacity(size_t Size) noexcept {
  return yw::max(Size + 1, 2 * std::bit_ceil(Size), size_t(256));
}
} // namespace internal

///--------------------------------------------------------------------------///
/// MARK: string

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
  constexpr const C* c_str() const noexcept { return data() ? data() : empty_string<C>; }
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

  constexpr string& push_back(C c) noexcept {
    reserve(size() + 1);
    data()[size()] = c;
    size = size() + 1;
    data.ref()[size()] = C();
    return *this;
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

///--------------------------------------------------------------------------///
/// MARK: vtos

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
  if (i >= 0) return _uint_to_str(static_cast<uint64_t>(i), o);
  *o++ = '-';
  return _uint_to_str(uint64_t(-(i + 1)) + 1, o);
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

template<char_type C> inline constexpr auto vtos = []<arithmetic T>(T v) noexcept -> string<C> {
  string<C> result;
  if constexpr (is_bool<T>) {
    result.resize(internal::_bool_to_str_sufficient_buffer_size);
    result.resize(internal::_bool_to_str(v, result.data()) - result.data());
  } else if constexpr (float_type<T>) {
    result.resize(internal::_float_to_str_sufficient_buffer_size);
    result.resize(internal::_float_to_str(v, result.data()) - result.data());
  } else if constexpr (unsigned_integral<T>) {
    result.resize(internal::_uint_to_str_sufficient_buffer_size);
    result.resize(internal::_uint_to_str(v, result.data()) - result.data());
  } else if constexpr (signed_integral<T>) {
    result.resize(internal::_int_to_str_sufficient_buffer_size);
    result.resize(internal::_int_to_str(v, result.data()) - result.data());
  } else static_assert(always_false<T>, "Unsupported arithmetic type for vtos.");
  return result;
};

///--------------------------------------------------------------------------///
/// MARK: stov

template<arithmetic T> struct stov_result {
  T value;
  size_t last; // position of the last character processed in the input string.
  constexpr operator T() const noexcept { return value; }
};

namespace internal {
constexpr stov_result<uint64_t> _stov_uint(string_view<char> sv, bool& ok) noexcept;
constexpr stov_result<bool> _stov_bool(string_view<char> sv, bool& ok) noexcept {
  if (sv.size() >= 4 && sv[0] == 't' && sv[1] == 'r' && sv[2] == 'u' && sv[3] == 'e') return {true, 4};
  if (sv.size() >= 5 && sv[0] == 'f' && sv[1] == 'a' && sv[2] == 'l' && sv[3] == 's' && sv[4] == 'e') return {false, 5};
  const auto result = _stov_uint(sv, ok);
  return {result.value != 0, result.last};
}
constexpr stov_result<uint64_t> _stov_uint(string_view<char> sv, bool& ok) noexcept {
  uint64_t value = 0;
  size_t i = 0;
  for (; i < sv.size() && is_digit(sv[i]); ++i) {
    const auto digit = uint64_t(sv[i] - '0');
    constexpr auto max = std::numeric_limits<uint64_t>::max();
    if (value > max / 10 || (value == max / 10 && digit > max % 10)) value = max;
    else value = value * 10 + digit;
  }
  ok = i != 0;
  return {value, i};
}
constexpr stov_result<long double> _stov_float(string_view<char> sv, bool& ok) noexcept {
  if (sv.size() >= 3 && sv[0] == 'n' && sv[1] == 'a' && sv[2] == 'n')
    return {std::numeric_limits<long double>::quiet_NaN(), 3};
  if (sv.size() >= 3 && sv[0] == 'i' && sv[1] == 'n' && sv[2] == 'f')
    return {std::numeric_limits<long double>::infinity(), 3};

  long double value = 0;
  size_t i = 0;
  bool has_digits = false;
  for (; i < sv.size() && is_digit(sv[i]); ++i) {
    has_digits = true;
    value = value * 10 + (sv[i] - '0');
  }
  if (i < sv.size() && sv[i] == '.') {
    long double scale = 1;
    ++i;
    for (; i < sv.size() && is_digit(sv[i]); ++i) {
      has_digits = true;
      scale *= 0.1L;
      value += (sv[i] - '0') * scale;
    }
  }
  if (!has_digits) {
    ok = false;
    return {};
  }
  if (i < sv.size() && (sv[i] == 'e' || sv[i] == 'E')) {
    const auto exp_pos = i;
    ++i;
    const bool exp_negative = i < sv.size() && sv[i] == '-';
    if (i < sv.size() && (sv[i] == '-' || sv[i] == '+')) ++i;
    bool exp_ok = true;
    const auto exp = _stov_uint({sv.data() + i, sv.size() - i}, exp_ok);
    if (exp_ok) {
      i += exp.last;
      for (uint64_t n = exp.value; n > 0; --n) {
        if (exp_negative) value *= 0.1L;
        else value *= 10;
      }
    } else i = exp_pos;
  }
  return {value, i};
}
template<arithmetic T> constexpr stov_result<T> _stov(string_view<char> sv) noexcept {
  bool ok = true;
  if constexpr (is_bool<T>) {
    const auto res = internal::_stov_bool(sv, ok);
    if (!ok) return {.value = {}, .last = 0};
    return res;
  } else if constexpr (unsigned_integral<T>) {
    const auto res = internal::_stov_uint(sv, ok);
    if (!ok) return {.value = {}, .last = 0};
    return {.value = static_cast<T>(yw::min(res.value, std::numeric_limits<T>::max())), .last = res.last};
  } else {
    const bool negative = sv.size() > 0 && sv[0] == '-';
    if constexpr (signed_integral<T>) {
      const auto body = negative ? string_view<char>(sv.data() + 1, sv.size() - 1) : sv;
      const auto res = internal::_stov_uint(body, ok);
      if (!ok) return {.value = {}, .last = 0};
      if (negative) {
        if (res.value > std::numeric_limits<T>::max())
          return {.value = std::numeric_limits<T>::min(), .last = res.last + 1};
        else return {.value = static_cast<T>(-static_cast<int64_t>(res.value)), .last = res.last + 1};
      } else return {.value = static_cast<T>(yw::min(res.value, std::numeric_limits<T>::max())), .last = res.last};
    } else {
      const auto body = negative ? string_view<char>(sv.data() + 1, sv.size() - 1) : sv;
      const auto res = internal::_stov_float(body, ok);
      if (!ok) return {.value = {}, .last = 0};
      if (negative) return {.value = -static_cast<T>(res.value), .last = res.last + 1};
      else return {.value = static_cast<T>(res.value), .last = res.last};
    }
  }
}
} // namespace internal

template<arithmetic T> constexpr auto stov = []<stringable S>(S&& s) -> stov_result<T> {
  using C = iter_value_t<S>;
  const auto sv = string_view<C>(s);
  if constexpr (!same_as<C, char>) {
    string<char> tmp(sv.size());
    for (size_t i = 0; i < sv.size(); ++i) {
      if (const auto c = sv[i]; c > 0x7f) {
        tmp.resize(i);
        return internal::_stov<T>(tmp);
      } else tmp[i] = static_cast<char>(sv[i]);
    }
    return internal::_stov<T>(tmp);
  } else return internal::_stov<T>(sv);
};

static_assert(stov<int>("-123").value == -123, "stov failed");
static_assert(stov<int8_t>("-555").value == -128, "stov failed");

///--------------------------------------------------------------------------///
/// MARK: unicode

namespace internal {
inline constexpr char32_t _unicode_s8_to_c32(const auto*& s) noexcept {
  const auto c = char32_t(*s);
  const auto i = unsigned(c >= 0xc0) + unsigned(c >= 0xe0) + unsigned(c >= 0xf0);
  const auto j = i + 1 + unsigned(i != 0);
  char32_t uc = char32_t(-int(i == 3) & s[i < 3 ? i : 3] & 0x3f);
  uc |= char32_t((-int(i >= 2) & s[i < 2 ? i : 2] & 0x3f)) << (6 * (i >= 2 ? i - 2 : 0));
  uc |= char32_t((-int(i >= 1) & s[i < 1 ? i : 1] & 0x3f)) << (6 * (i >= 1 ? i - 1 : 0));
  uc |= char32_t(char8_t(c << j) >> j) << (6 * i);
  s += i + 1;
  return uc;
}
inline constexpr char32_t _unicode_s16_to_c32(const auto*& s) noexcept {
  const auto c = char32_t(*s);
  const bool b = (c & 0xff00) == 0xd800;
  const auto uc = c ^ (-int(b) & (c ^ (0x10000 | ((c - 0xd800) << 10 | char32_t(s[b] - 0xdc00)))));
  s += 1 + b;
  return uc;
}
template<char_type C> inline constexpr void _unicode_c32_to_s8(char32_t uc, C*& s) noexcept {
  const auto i = unsigned(uc >= 0x80) + unsigned(uc >= 0x800) + unsigned(uc >= 0x10000);
  s[i < 3 ? i : 3] = C(0x80 | (uc & 0x3f));
  s[i < 2 ? i : 2] = C(0x80 | ((uc >> (6 * (i > 1 ? i - 2 : 0))) & 0x3f));
  s[i < 1 ? i : 1] = C(0x80 | ((uc >> (6 * (i > 0 ? i - 1 : 0))) & 0x3f));
  *s = C(uint32_t(((i + (i >> 1)) << 4) + (-i & 0xb0)) | ((uc >> (6 * i)) & (0x3f >> i | -int(i == 0))));
  s += i + 1;
}
template<char_type C> inline constexpr void _unicode_c32_to_s16(char32_t uc, C*& s) noexcept {
  const bool b = uc >= 0x10000;
  s[b] = C(0xdc00 | (uc & 0x3ff));
  *s = C(uc ^ ((uc ^ (0xd800 | (uc >> 10))) & -int(b)));
  s += 1 + b;
}
template<char_type In, char_type Out> constexpr Out* _unicode(const In* i, size_t n, Out* o) {
  for (auto s = i, end = i + n; s < end;) {
    char32_t uc;
    if constexpr (same_as<In, char8_t>) uc = _unicode_s8_to_c32(s);
    else if constexpr (same_as<In, char16_t>) uc = _unicode_s16_to_c32(s);
    else uc = char32_t(*s++);
    if constexpr (same_as<Out, char8_t>) _unicode_c32_to_s8(uc, o);
    else if constexpr (same_as<Out, char16_t>) _unicode_c32_to_s16(uc, o);
    else *o++ = Out(uc);
  }
  return o;
}
} // namespace internal

template<typename C> requires char_type<C> inline constexpr auto unicode = []<stringable S>(S&& s) -> string<C> {
  using From = iter_value_t<S>;
  if constexpr (same_as<S&&, string<C>&&>) return move(s);
  if constexpr (same_as<From, C>) return string(string_view<C>(s));
  const auto sv_original = string_view<From>(s);
  if constexpr (sizeof(From) == sizeof(C)) return string(bitcast<string_view<C>>(sv_original));
  using T = select_type<sizeof(From) / 2, char8_t, char16_t, char32_t>;
  const auto sv = bitcast<string_view<T>>(sv_original);
  constexpr auto scale = select<yw::max(int(sizeof(T)) - int(sizeof(C)), 0)>(1, 3, 2, 4);
  auto r = string<C>(sv.size() * scale, C{});
  auto out = internal::_unicode(sv.data(), sv.size(), r.data());
  r.resize(out - r.data());
  return r;
};
} // namespace yw
