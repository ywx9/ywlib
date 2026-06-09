#pragma once
#include "yw/core.h"

#ifdef ywlib_header_name
#error "ywlib_header_name already defined unexpectedly"
#endif
#define ywlib_header_name "yw/string.h"

namespace yw {

using path = std::filesystem::path;

inline constexpr auto is_ascii = []<char_type C>(C c) noexcept { return 0x20 <= c && c < 0x7F; };
inline constexpr auto is_digit = []<char_type C>(C c) noexcept { return '0' <= c && c <= '9'; };
inline constexpr auto is_lower = []<char_type C>(C c) noexcept { return 'a' <= c && c <= 'z'; };
inline constexpr auto is_upper = []<char_type C>(C c) noexcept { return 'A' <= c && c <= 'Z'; };
inline constexpr auto is_alpha = []<char_type C>(C c) noexcept { return is_lower(c) || is_upper(c); };
inline constexpr auto is_alnum = []<char_type C>(C c) noexcept { return is_alpha(c) || is_digit(c); };
inline constexpr auto is_xdigit = []<char_type C>(C c) noexcept {
  return is_digit(c) || (('a' <= c && c <= 'f') || ('A' <= c && c <= 'F'));
};

template<typename S, typename C = iter_value_t<S>> concept stringable = requires {
  requires char_type<C>;
  requires std::convertible_to<S, std::basic_string_view<C>>;
  requires std::constructible_from<std::basic_string_view<C>, S>;
};

template<arithmetic T> constexpr auto stov = [](stringable<char> auto&& str) -> T {
  const auto sv = std::string_view(str);
  T result{};
  std::from_chars(sv.data(), sv.data() + sv.size(), result);
  return result;
};

template<char_type C> using string_view = std::basic_string_view<C>;

/// MARK: string

template<char_type C> class string {
  static_assert(same_as<C, remove_cv<C>>);
  static constexpr C _empty[1] = {C()};

  size_t _size = 0;
  size_t _capacity = 0;
  C* _ptr = nullptr;

  constexpr size_t _get_preferred_capacity(size_t Size) const noexcept {
    return yw::max(Size, 2 * std::bit_ceil(Size), size_t(256));
  }

  constexpr string(is_none auto, size_t Size) // constructing in uninitialized state
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

  constexpr string(const string& Other) : string(string_view<C>(Other)) {}

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

  template<stringable<C> S> requires same_as<remove_cvref<S>, string_view<C>>
  constexpr string(S&& View) : string(none{}, View.size()) {
    for (size_t i = 0; i < _size; ++i) _ptr[i] = View[i];
    _ptr[_size] = C();
  }

  template<stringable<C> S>
  requires different_from<remove_cvref<S>, string> && different_from<remove_cvref<S>, string_view<C>>
  constexpr string(S&& Other) : string(string_view<C>(Other)) {}

  constexpr operator string_view<C>() const noexcept { return {_ptr, _size}; }
  constexpr auto view() const noexcept { return string_view<C>(_ptr, _size); }

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
    if (_ptr) _ptr[0] = C();
  }

  constexpr void reserve(size_t Size) {
    if (_capacity > Size) return;
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

  constexpr string& pop_back() {
    if (_size > 0) --_size;
    if (_ptr) _ptr[_size] = C();
    return *this;
  }

  constexpr string& append(stringable<C> auto&& View) {
    auto sv = static_cast<string_view<C>>(View);
    if (sv.empty()) return *this;
    const auto append_size = sv.size();
    const auto old_size = _size;
    const auto* src = sv.data();
    const bool overlaps = _ptr && src < _ptr + _size && _ptr < src + append_size;
    const auto offset = overlaps ? size_t(src - _ptr) : size_t(0);
    reserve(_size + append_size);
    if (overlaps) src = _ptr + offset;
    for (size_t i = 0; i < append_size; ++i) _ptr[old_size + i] = src[i];
    _size += append_size;
    _ptr[_size] = C();
    return *this;
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
    return string(string_view<C>(_ptr + Pos, yw::min(Count, _size - Pos)));
  }

  constexpr void swap(string& Other) noexcept {
    std::swap(_ptr, Other._ptr);
    std::swap(_size, Other._size);
    std::swap(_capacity, Other._capacity);
  }

  friend constexpr bool operator==(const string& Lhs, const string& Rhs) noexcept { return Lhs.view() == Rhs.view(); }
  friend constexpr auto operator<=>(const string& Lhs, const string& Rhs) noexcept { return Lhs.view() <=> Rhs.view(); }
};

template<stringable S> string(S&&) -> string<iter_value_t<S>>;

/// MARK: bool_to_string

template<char_type C> constexpr string<C> bool_to_string(bool Value) {
  constexpr C _true[] = {'t', 'r', 'u', 'e'};
  constexpr C _false[] = {'f', 'a', 'l', 's', 'e'};
  return Value ? string<C>(string_view<C>(_true, 4)) : string<C>(string_view<C>(_false, 5));
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

/// MARK: int_to_string

template<char_type C> constexpr string<C> int_to_string(integral auto Value) {
  if constexpr (uint_type<decltype(Value)>) return uint_to_string<C>(Value);
  const bool minus = Value < 0;
  auto u = static_cast<uint64_t>(minus ? -Value : Value);
  unsigned keta = 0;
  for (auto u_ = u; u_ != 0; u_ /= 10) ++keta;
  string<C> result(keta + minus, '\0');
  if (minus) result[0] = '-';
  for (auto p = result.data() + result.size(); u != 0; u /= 10) *(--p) = static_cast<C>('0' + (u % 10));
  return result;
}
constexpr string<char> int_to_string(integral auto Value) { return int_to_string<char>(Value); }

/// MARK: float_to_string

template<char_type C> constexpr string<C> float_to_string(floating auto Value) {
  using T = decltype(Value);
  using limits = std::numeric_limits<uint64_t>;
  constexpr C _nan[] = {'n', 'a', 'n'};
  constexpr C _inf[] = {'i', 'n', 'f'};
  constexpr C _neg_inf[] = {'-', 'i', 'n', 'f'};
  constexpr C _zero[] = {'0'};
  constexpr C _neg_zero[] = {'-', '0'};
  if (std::isnan(Value)) return string(string_view<C>(_nan, 3));
  if (std::isinf(Value)) return Value < 0 ? string(string_view<C>(_neg_inf, 4)) : string(string_view<C>(_inf, 3));
  const bool minus = std::signbit(Value);
  long double abs_value = static_cast<long double>(minus ? -Value : Value);
  if (abs_value == 0) return minus ? string(string_view<C>(_neg_zero, 2)) : string(string_view<C>(_zero, 1));
  constexpr uint64_t scale = 1000000;
  constexpr long double max_uint64 = static_cast<long double>(limits::max());
  auto append_fixed6 = [](string<C>& s, uint64_t frac) {
    const size_t offset = s.size();
    s.resize(offset + 6, C('0'));
    for (size_t i = 0; i < 6; ++i) {
      s[offset + 5 - i] = C('0' + (frac % 10));
      frac /= 10;
    }
  };
  auto append_sign = [&](string<C>& s) {
    if (minus) s += C('-');
  };
  auto to_exponential = [&]() {
    int exponent = 0;
    long double normalized = abs_value;
    while (normalized >= 10) normalized /= 10, ++exponent;
    uint64_t int_part = static_cast<uint64_t>(normalized);
    uint64_t frac_part = static_cast<uint64_t>((normalized - int_part) * scale + 0.5L);
    if (frac_part >= scale) {
      ++int_part, frac_part -= scale;
      if (int_part >= 10) int_part = 1, ++exponent;
    }
    string<C> s;
    append_sign(s);
    s += uint_to_string<C>(int_part), s += C('.');
    append_fixed6(s, frac_part);
    s += C('e'), s += C('+');
    s += uint_to_string<C>(static_cast<uint64_t>(exponent));
    return s;
  };
  if (abs_value >= max_uint64) return to_exponential();
  uint64_t int_part = static_cast<uint64_t>(abs_value);
  uint64_t frac_part = static_cast<uint64_t>((abs_value - int_part) * scale + 0.5L);
  if (frac_part >= scale) ++int_part, frac_part -= scale;
  if (int_part >= limits::max()) return to_exponential();
  string<C> s;
  append_sign(s);
  s += uint_to_string<C>(int_part);
  s += C('.');
  append_fixed6(s, frac_part);
  if (int_part != 0) {
    while (!s.empty() && s.back() == C('0')) s.pop_back();
    if (!s.empty() && s.back() == C('.')) s.pop_back();
  }
  return s;
}
constexpr string<char> float_to_string(floating auto value) { return float_to_string<char>(value); }

/// MARK: vtos

template<char_type C> constexpr string<C> vtos(arithmetic auto Value) {
  if constexpr (is_bool<decltype(Value)>) return bool_to_string<C>(Value);
  else if constexpr (integral<decltype(Value)>) return int_to_string<C>(Value);
  else return float_to_string<C>(Value);
}

constexpr string<char> vtos(arithmetic auto Value) { return vtos<char>(Value); }

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

template<char_type C> inline constexpr auto unicode = []<stringable S>(S&& s) -> string<C> {
  using From = iter_value_t<S>;
  if constexpr (same_as<S&&, string<C>&&>) return std::move(s);
  if constexpr (same_as<From, C>) return string(string_view<C>(s));
  const auto sv_original = string_view<From>(s);
  if constexpr (sizeof(From) == sizeof(C)) return string(std::bit_cast<string_view<C>>(sv_original));
  using T = select_type<sizeof(From) / 2, char8_t, char16_t, char32_t>;
  const auto sv = std::bit_cast<std::basic_string_view<T>>(sv_original);
  constexpr auto scale = select_value<yw::max(int(sizeof(T)) - int(sizeof(C)), 0), 1, 3, 2, 4>;
  auto r = string<C>(sv.size() * scale, C{});
  auto out = internal::_unicode(sv.data(), sv.size(), r.data());
  r.resize(out - r.data());
  return r;
};

/// MARK: format

namespace internal {
constexpr auto hex_table = "0123456789abcdef";
template<typename T, typename C> concept has_to_string_c = requires(T&& a) {
  { a.template to_string<C>() } -> convertible_to<string<C>>;
};
template<typename T> concept has_to_string = requires(T&& a) {
  { a.to_string() } -> stringable;
};
template<char_type C, typename T> constexpr string<C> _format(T&& Arg) {
  using t = remove_cvref<T>;
  if constexpr (stringable<t>) return unicode<C>(static_cast<T&&>(Arg));
  else if constexpr (char_type<t>) return unicode<C>(string<t>(1, Arg));
  else if constexpr (arithmetic<t>) return vtos<C>(Arg);
  else if constexpr (is_pointer<t>) {
    string<C> s(sizeof(void*) * 2 + 2, C('0'));
    s[1] = C('x');
    if (!std::is_constant_evaluated()) {
      auto u = reinterpret_cast<size_t>(Arg);
      for (auto p = s.data() + s.size(); u != 0; u /= 16) *--p = C(internal::hex_table[u % 16]);
      return s;
    } else return s; // always return 0x0...0
  } else if constexpr (same_as<T, path>) return unicode<C>(Arg.native());
  else if constexpr (internal::has_to_string_c<T, C>) return Arg.template to_string<C>();
  else if constexpr (internal::has_to_string<T>) return unicode<C>(Arg.to_string());
  else static_assert(always_false<T>, "Type does not have to_string<C> or to_string method");
}
} // namespace internal

template<char_type C, typename... Ts> constexpr string<C> format(Ts&&... Args) {
  string<C> s;
  ((s += internal::_format<C>(static_cast<Ts&&>(Args))), ...);
  return s;
}
template<typename... Ts> constexpr string<char> format(Ts&&... Args) {
  return format<char>(static_cast<Ts&&>(Args)...);
}

/// MARK: print

namespace internal {
template<bool Error, bool NewLine, typename... Ts> void _print(Ts&&... Args) {
#ifdef _WIN32
  string<wchar_t> s;
  if constexpr (NewLine) ((s += internal::_format<wchar_t>(static_cast<Ts&&>(Args))), ..., (s += L'\n'));
  else ((s += internal::_format<wchar_t>(static_cast<Ts&&>(Args))), ...);
  if constexpr (Error) ::WriteConsoleW(::GetStdHandle(STD_ERROR_HANDLE), s.data(), unsigned(s.size()), 0, 0);
  else ::WriteConsoleW(::GetStdHandle(STD_OUTPUT_HANDLE), s.data(), unsigned(s.size()), 0, 0);
#else
  string<char> s;
  if constexpr (NewLine) ((s += internal::_format<char>(static_cast<Ts&&>(Args))), ..., (s += '\n'));
  else ((s += internal::_format<char>(static_cast<Ts&&>(Args))), ...);
  if constexpr (Error) std::fputs(s.data(), stderr);
  else std::fputs(s.data(), stdout);
#endif
}
} // namespace internal

inline constexpr struct {
  template<typename... Ts> static void operator()(Ts&&... Args) { internal::_print<false>(static_cast<Ts&&>(Args)...); }
  template<typename... Ts> static void err(Ts&&... Args) { internal::_print<true>(static_cast<Ts&&>(Args)...); }
} print_inline;

inline constexpr struct {
  static void operator()() { internal::_print<false, true>(); }
  template<typename... Ts> static void operator()(Ts&&... Args) { internal::_print<false, true>(static_cast<Ts&&>(Args)...); }
  static void err() { internal::_print<true, true>(); }
  template<typename... Ts> static void err(Ts&&... Args) { internal::_print<true, true>(static_cast<Ts&&>(Args)...); }
} print;

/// MARK: print_fallback

struct {
  template<typename... Ts> static void operator()(Ts&&... Args) {
#ifdef _WIN32
    const auto s = format<wchar_t>(static_cast<Ts&&>(Args)...);
    ::WriteConsoleW(::GetStdHandle(STD_OUTPUT_HANDLE), s.data(), unsigned(s.size()), 0, 0);
    ::MessageBoxW(nullptr, s.data(), L"Message", MB_OK | MB_TOPMOST | MB_TASKMODAL);
#else
    const auto s = format<char>(static_cast<Ts&&>(Args)...);
    std::fputs(s.data(), stdout);
#endif
  }
  template<typename... Ts> static void err(Ts&&... Args) {
#ifdef _WIN32
    const auto s = format<wchar_t>(static_cast<Ts&&>(Args)...);
    ::WriteConsoleW(::GetStdHandle(STD_ERROR_HANDLE), s.data(), unsigned(s.size()), 0, 0);
    ::MessageBoxW(nullptr, s.data(), L"Error", MB_OK | MB_TOPMOST | MB_ICONERROR | MB_TASKMODAL);
#else
    const auto s = format<char>(static_cast<Ts&&>(Args)...);
    std::fputs(s.data(), stderr);
#endif
  }
} print_fallback;

/// MARK: null_terminated

template<char_type C> class null_terminated {
  static_assert(same_as<C, remove_cv<C>>);
  template<typename S> static constexpr bool _is_array = is_bounded_array<remove_ref<S>> && same_as<iter_value_t<S>, C>;
  std::variant<string<C>, string_view<C>> _data;

public:
  constexpr null_terminated() : _data(string_view<C>{}) {}
  constexpr null_terminated(string<C>& str) : _data(std::in_place_index_t<1>{}, str) {}
  constexpr null_terminated(const string<C>& str) : _data(std::in_place_index_t<1>{}, str) {}
  constexpr null_terminated(string<C>&& str) : _data(std::in_place_index_t<0>{}, std::move(str)) {}
  constexpr null_terminated(const string<C>&& str) : _data(std::in_place_index_t<0>{}, std::move(str)) {}

  template<typename S> requires _is_array<S> constexpr null_terminated(const S& a)
    : _data(std::in_place_index_t<1>{}, string_view<C>(a, std::char_traits<C>::length(a))) {}

  template<stringable S> requires(!_is_array<S>) constexpr null_terminated(S&& s) : _data() {
    if constexpr (different_from<iter_value_t<S>, C>) _data.template emplace<0>(unicode<C>(static_cast<S&&>(s)));
    else _data.template emplace<0>(string<C>(string_view<C>(static_cast<S&&>(s))));
  }

  constexpr operator string_view<C>() const {
    return std::visit([](const auto& v) { return string_view<C>(v); }, _data);
  }

  bool empty() const noexcept {
    return std::visit([](const auto& v) { return v.empty(); }, _data);
  }
  size_t size() const noexcept {
    return std::visit([](const auto& v) { return v.size(); }, _data);
  }
  const C* data() const noexcept {
    return std::visit([](const auto& v) { return v.data(); }, _data);
  }
  const C* begin() const noexcept {
    return std::visit([](const auto& v) { return v.data(); }, _data);
  }
  const C* end() const noexcept {
    return std::visit([](const auto& v) { return v.data() + v.size(); }, _data);
  }
};

/// MARK: ministr

/// 8-bytes minimalist string class.
template<char_type C> class ministr {
  static_assert(same_as<C, remove_cv<C>>);

  C* _p = nullptr;

  static C* allocate(size_t Size) {
    constexpr size_t header_size = 8 / sizeof(C);
    const auto p = new C[Size + header_size + 1];
    *(reinterpret_cast<size_t*>(p)) = Size;
    p[header_size + Size] = C('\0');
    return p;
  }

  size_t begin_index() const noexcept { return _p ? 8 / sizeof(C) : 0; }
  size_t end_index() const noexcept { return _p ? 8 / sizeof(C) + *(reinterpret_cast<size_t*>(_p)) : 0; }

public:
  ~ministr() noexcept { delete[] _p; }
  ministr() noexcept = default;
  operator string_view<C>() const { return {data(), size()}; }

  bool empty() const noexcept { return size() == 0; }
  size_t size() const { return _p ? *(reinterpret_cast<size_t*>(_p)) : 0; }
  C* begin() { return _p + begin_index(); }
  const C* begin() const { return _p + begin_index(); }
  C* end() { return _p + end_index(); }
  const C* end() const { return _p + end_index(); }
  C* data() { return _p + begin_index(); }
  const C* data() const { return _p + begin_index(); }
  C& operator[](size_t Index) { return _p[begin_index() + Index]; }
  const C& operator[](size_t Index) const { return _p[begin_index() + Index]; }

  ministr(size_t Size) : _p(allocate(static_cast<wchar_t>(Size))) {}
  ministr(ministr&& Other) noexcept : _p(std::exchange(Other._p, nullptr)) {}

  ministr(const ministr& Other) {
    if (!Other._p) return;
    const auto size = Other.size();
    _p = allocate(size);
    std::memcpy(_p + begin_index(), Other._p + Other.begin_index(), size * sizeof(C));
  }

  ministr& operator=(const ministr& Other) {
    if (this == &Other) return *this;
    delete[] _p;
    if (Other._p) {
      const auto size = Other.size();
      _p = allocate(size);
      std::memcpy(_p + begin_index(), Other._p + Other.begin_index(), size * sizeof(C));
    } else _p = nullptr;
    return *this;
  }

  ministr& operator=(ministr&& Other) noexcept {
    if (this == &Other) return *this;
    delete[] _p;
    _p = std::exchange(Other._p, nullptr);
    return *this;
  }

  template<stringable<C> S> requires different_from<remove_cvref<S>, ministr> ministr(S&& Str) {
    const auto sv = string_view<C>(Str);
    const auto size = sv.size();
    _p = allocate(size);
    if (_p) std::memcpy(_p + begin_index(), sv.data(), size * sizeof(C));
  }

  template<input_range<C> R> requires (!stringable<R, C>) ministr(R&& Range) {
    const auto n = static_cast<size_t>(std::ranges::distance(Range));
    if (n == 0) return;
    _p = allocate(static_cast<wchar_t>(n));
    if (_p) std::ranges::copy(Range, _p + begin_index());
  }

  void resize(size_t Size) {
    if (const auto n = size(); Size < n) {
      *(reinterpret_cast<size_t*>(_p)) = Size;
      _p[begin_index() + Size] = C('\0');
    } else if (Size > n) {
      auto p = allocate(static_cast<wchar_t>(Size));
      std::memcpy(p + begin_index(), _p + begin_index(), n * sizeof(C));
      std::memset(p + begin_index() + n, 0, (Size - n) * sizeof(C));
      delete[] _p;
      _p = p;
    }
  }
};

/// MARK: footprint

namespace footprint {
inline constexpr size_t buffer_size = 32;
inline std::array<const char*, buffer_size> buffer;
inline size_t _next = 0;
inline void push(const char* Str) noexcept {
  if (!Str) return;
  buffer[_next] = Str;
  _next = (_next + 1) % buffer_size;
}
inline void clear() noexcept {
  for (size_t i = 0; i < buffer_size; ++i) buffer[i] = nullptr;
  _next = 0;
}
inline string<char> dump() noexcept {
  size_t len = 0;
  for (size_t i = 0; i < buffer_size; ++i)
    if (buffer[i]) len += std::char_traits<char>::length(buffer[i]) + 1;
  string<char> out(len);
  for (size_t i = _next; i < buffer_size; ++i)
    if (buffer[i]) out += buffer[i], out += '\n';
  for (size_t i = 0; i < _next; ++i)
    if (buffer[i]) out += buffer[i], out += '\n';
  return out;
}
} // namespace footprint

#define make_footprint ::yw::footprint::push(ywlib_make_source_info)
} // namespace yw

namespace std {
template<typename C> struct formatter<yw::string<C>, C> {
  formatter<basic_string_view<C>, C> fmt;
  constexpr auto parse(auto& ctx) { return fmt.parse(ctx); }
  auto format(const yw::string<C>& str, auto& ctx) const {
    return fmt.format(static_cast<basic_string_view<C>>(str), ctx);
  }
};
template<typename C> struct formatter<yw::null_terminated<C>, C> {
  formatter<basic_string_view<C>, C> fmt;
  constexpr auto parse(auto& ctx) { return fmt.parse(ctx); }
  auto format(const yw::null_terminated<C>& str, auto& ctx) const {
    return fmt.format(static_cast<basic_string_view<C>>(str), ctx);
  }
};
} // namespace std

#undef ywlib_header_name
