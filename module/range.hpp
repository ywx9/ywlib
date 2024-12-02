#pragma once

#include "std.hpp"

namespace yw::_ {

template<typename T> struct _iter_t {
  using v = std::iter_value_t<T>;
  using d = std::iter_difference_t<T>;
  using r = std::iter_reference_t<T>;
  using rr = std::iter_rvalue_reference_t<T>;
};
template<std::ranges::range Rg> struct _iter_t<Rg> : _iter_t<std::ranges::iterator_t<Rg>> {};

auto get_zoned_time() { return std::chrono::zoned_time(std::chrono::current_zone(), std::chrono::system_clock::now()); }
}

export namespace yw {

template<typename It> concept iterator = std::input_or_output_iterator<remove_ref<It>>;
template<typename Se, typename It> concept sentinel_for = std::sentinel_for<remove_ref<Se>, remove_ref<It>>;
template<typename Se, typename It> concept sized_sentinel_for = std::sized_sentinel_for<remove_ref<Se>, remove_ref<It>>;
template<typename Rg> concept range = std::ranges::range<Rg>;
template<typename Rg> concept borrowed_range = std::ranges::borrowed_range<Rg>;
template<typename Rg> concept sized_range = std::ranges::sized_range<Rg>;
template<range Rg> using iterator_t = std::ranges::iterator_t<Rg>;
template<range Rg> using borrowed_iterator_t = std::ranges::borrowed_iterator_t<Rg>;
template<range Rg> using sentinel_t = std::ranges::sentinel_t<Rg>;

template<typename T> requires iterator<T> || range<T> using iter_value = typename _::_iter_t<T>::v;
template<typename T> requires iterator<T> || range<T> using iter_difference = typename _::_iter_t<T>::d;
template<typename T> requires iterator<T> || range<T> using iter_reference = typename _::_iter_t<T>::r;
template<typename T> requires iterator<T> || range<T> using iter_rvref = typename _::_iter_t<T>::rr;
template<typename T> requires iterator<T> || range<T> using iter_common = common_type<iter_reference<T>, iter_value<T>>;

template<typename It, typename T> concept output_iterator = std::output_iterator<remove_ref<It>, T>;
template<typename It> concept input_iterator = std::input_iterator<remove_ref<It>>;
template<typename It> concept forward_iterator = std::forward_iterator<remove_ref<It>>;
template<typename It> concept bidirectional_iterator = std::bidirectional_iterator<remove_ref<It>>;
template<typename It> concept random_access_iterator = std::random_access_iterator<remove_ref<It>>;
template<typename It> concept contiguous_iterator = std::contiguous_iterator<remove_ref<It>>;
template<typename It, typename U> concept iterator_for = iterator<remove_ref<It>> && convertible_to<iter_reference<It>, U>;
template<typename Rg, typename T> concept output_range = std::ranges::output_range<Rg, T>;
template<typename Rg> concept input_range = std::ranges::input_range<Rg>;
template<typename Rg> concept forward_range = std::ranges::forward_range<Rg>;
template<typename Rg> concept bidirectional_range = std::ranges::bidirectional_range<Rg>;
template<typename Rg> concept random_access_range = std::ranges::random_access_range<Rg>;
template<typename Rg> concept contiguous_range = std::ranges::contiguous_range<Rg>;
template<typename Rg, typename U> concept range_for = iterator_for<iterator_t<Rg>, U>;
template<typename It, typename In> concept iter_copyable = iterator<It> && iterator<In> && std::indirectly_copyable<In, It>;
template<typename It, typename In> concept iter_movable = iterator<It> && iterator<In> && std::indirectly_movable<In, It>;
template<typename Fn, typename It> concept iter_unary_invocable = iterator<It> && std::indirectly_unary_invocable<Fn, It>;
template<typename Fn, typename It> concept iter_unary_predicate = iterator<It> && std::indirect_unary_predicate<Fn, It>;

inline constexpr auto begin = []<range Rg>(Rg&& r) ywlib_wrap_auto(std::ranges::begin(fwd<Rg>(r)));
inline constexpr auto end = []<range Rg>(Rg&& r) ywlib_wrap_auto(std::ranges::end(fwd<Rg>(r)));
inline constexpr auto rbegin = []<range Rg>(Rg&& r) ywlib_wrap_auto(std::ranges::rbegin(fwd<Rg>(r)));
inline constexpr auto rend = []<range Rg>(Rg&& r) ywlib_wrap_auto(std::ranges::rend(fwd<Rg>(r)));
inline constexpr auto size = []<range Rg>(Rg&& r) ywlib_wrap_auto(std::ranges::size(fwd<Rg>(r)));
inline constexpr auto empty = []<range Rg>(Rg&& r) ywlib_wrap_auto(std::ranges::empty(fwd<Rg>(r)));
inline constexpr auto data = []<range Rg>(Rg&& r) ywlib_wrap_auto(std::ranges::data(fwd<Rg>(r)));
inline constexpr auto iter_move = []<iterator It>(It&& i) ywlib_wrap_ref(std::ranges::iter_move(fwd<It>(i)));
inline constexpr auto iter_swap = []<iterator It, iterator Jt>(It&& i, Jt&& j) //
  ywlib_wrap_void(std::ranges::iter_swap(fwd<It>(i), fwd<Jt>(j)));

template<character Ct> using string = std::basic_string<Ct>;
template<character Ct> using string_view = std::basic_string_view<Ct>;
template<character Ct, typename... Args> using format_string = std::basic_format_string<Ct, Args...>;
template<typename T, typename Ct = iter_value<T>> concept stringable = nt_convertible_to<T&, string_view<Ct>>;

template<typename T, nat N = npos> class array {
public:
  T _[N]{};
  static constexpr nat count = N;
  using value_type = T;
  template<typename U> requires assignable<T&, const U&> //
  constexpr array& operator=(const U (&Other)[N]) {
    return std::ranges::copy(Other, _), *this;
  }
  constexpr operator add_lvref<T[N]>() noexcept { return _; }
  constexpr operator add_lvref<const T[N]>() const noexcept { return _; }
  constexpr T& operator[](nat I) { return _[I]; }
  constexpr const T& operator[](nat I) const { return _[I]; }
  constexpr nat size() const noexcept { return N; }
  constexpr bool empty() const noexcept { return false; }
  constexpr T* data() noexcept { return _; }
  constexpr const T* data() const noexcept { return _; }
  constexpr T* begin() noexcept { return _; }
  constexpr const T* begin() const noexcept { return _; }
  constexpr T* end() noexcept { return _ + N; }
  constexpr const T* end() const noexcept { return _ + N; }
  constexpr T& front() noexcept { return *_; }
  constexpr const T& front() const noexcept { return *_; }
  constexpr T& back() noexcept { return _[N - 1]; }
  constexpr const T& back() const noexcept { return _[N - 1]; }
  template<nat I> requires (I < N) constexpr T& get() & noexcept { return _[I]; }
  template<nat I> requires (I < N) constexpr T&& get() && noexcept { return mv(_[I]); }
  template<nat I> requires (I < N) constexpr const T& get() const& noexcept { return _[I]; }
  template<nat I> requires (I < N) constexpr const T&& get() const&& noexcept { return mv(_[I]); }
  constexpr string<T> to_string() const requires character<T> { return string<T>(_, N); }
  constexpr string_view<T> to_string_view() const requires character<T> { return string_view<T>(_, N); }
  static constexpr array fill(const T& v) {
    array a;
    std::ranges::fill(a._, v);
    return a;
  }
};

template<typename T> class array<T, npos> : public std::vector<T> {
public:
  constexpr array() noexcept = default;
  constexpr array(std::vector<T>&& v) : std::vector<T>(mv(v)) {}
  constexpr explicit array(nat n) : std::vector<T>(n) {}
  constexpr array(nat n, const T& v) : std::vector<T>(n, v) {}
  template<iterator_for<T> It> constexpr array(It i, It s) : std::vector<T>(i, s) {}
  template<iterator_for<T> It, sentinel_for<It> Se> requires (!same_as<It, Se>) //
  constexpr array(It i, Se s) : std::vector<T>(std::common_iterator<It, Se>(i), std::common_iterator<It, Se>(s)) {}
  template<range_for<T> Rg> constexpr array(Rg&& r) : std::vector<T>(yw::begin(r), yw::end(r)) {}
};
static_assert(sizeof(array<int>) == sizeof(std::vector<int>));

template<typename T> class array<T, 0> {
public:
  static constexpr nat count = 0;
  using value_type = T;
  constexpr nat size() const noexcept { return 0; }
  constexpr bool empty() const noexcept { return true; }
  constexpr T* data() noexcept { return nullptr; }
  constexpr const T* data() const noexcept { return nullptr; }
  constexpr T* begin() noexcept { return nullptr; }
  constexpr const T* begin() const noexcept { return nullptr; }
  constexpr T* end() noexcept { return nullptr; }
  constexpr const T* end() const noexcept { return nullptr; }
  template<nat I> constexpr void get() const = delete;
  explicit constexpr operator string<T>() const requires character<T> { return string<T>{}; }
  explicit constexpr operator string_view<T>() const requires character<T> { return string_view<T>{}; }
};

template<typename T, convertible_to<T>... Ts> array(T, Ts...) -> array<T, 1 + sizeof...(Ts)>;
template<typename T> array(nat, const T&) -> array<T, npos>;
template<iterator It, sentinel_for<It> Se> array(It, Se) -> array<iter_value<It>, npos>;
template<range Rg> array(Rg&&) -> array<iter_value<Rg>, npos>;

inline constexpr auto strlen = []<stringable St>(St&& Str) -> nat {
  if constexpr (is_array<remove_ref<St>>) return extent<St> - 1;
  else if constexpr (is_pointer<remove_ref<St>>) return std::char_traits<iter_value<St>>::length(Str);
  else if constexpr (specialization_of<remove_cvref<St>, std::basic_string>) return Str.size();
  else if constexpr (specialization_of<remove_cvref<St>, std::basic_string_view>) return Str.size();
  else return string_view<iter_value<St>>(Str).size();
};

inline constexpr auto is_digit = []<character Ct>(const Ct c) noexcept { return (natcast(c) ^ 0x30) < 10; };

inline constexpr auto to_nat = []<character Ct>(string_view<Ct> s) noexcept -> nat8 {
  nat8 v{};
  while (1) {
    if (s.empty()) return v;
    else if (is_digit(s.front())) {
      v = s.front() - '0';
      break;
    } else s.remove_prefix(1);
  }
  s.remove_prefix(1);
  while (1) {
    if (s.empty()) return v;
    else if (is_digit(s.front())) v = v * 10 + s.front() - '0', s.remove_prefix(1);
    else return v;
  }
};

inline constexpr auto to_int = []<character Ct>(string_view<Ct> s) noexcept -> int8 {
  int8 v{};
  bool neg = false;
  while (1) {
    if (s.empty()) return {};
    else if (is_digit(s.front())) {
      neg = true, v = s.front() - '0';
      break;
    } else if (s.front() == Ct('-')) {
      neg = true;
      break;
    } else s.remove_prefix(1);
  }
  s.remove_prefix(1);
  while (1) {
    if (s.empty()) return neg ? -v : v;
    else if (is_digit(s.front())) v = v * 10 + s.front() - '0', s.remove_prefix(1);
    else return neg ? -v : v;
  }
};

inline constexpr auto to_fat = []<character Ct>(string_view<Ct> s) noexcept -> fat8 {
  fat8 v{};
  bool neg = false, dot = false;
  while (1) {
    if (s.empty()) return {};
    else if (is_digit(s.front())) {
      v = s.front() - '0';
      break;
    } else if (s.front() == Ct('-')) {
      neg = true;
      break;
    } else if (s.front() == Ct('.')) {
      dot = true;
      break;
    } else s.remove_prefix(1);
  }
  s.remove_prefix(1);
  while (!dot) {
    if (s.empty()) return neg ? -v : v;
    else if (is_digit(s.front())) v = v * 10 + s.front() - '0', s.remove_prefix(1);
    else if (s.front() == Ct('.')) {
      dot = true;
      break;
    } else return neg ? -v : v;
  }
  s.remove_prefix(1);
  fat8 p = 0.1;
  while (1) {
    if (s.empty()) return neg ? -v : v;
    else if (is_digit(s.front())) {
      v += (s.front() - '0') * p, p *= 0.1, s.remove_prefix(1);
    } else return neg ? -v : v;
  }
};

template<character Ct> inline constexpr auto to_hex = []<scalar T>(const T v) noexcept -> string<Ct> {
  if constexpr (is_pointer<T>) {
    string<Ct> r(16, '0');
    if (!is_cev)
      for (nat i = 16, n = natcast(v); 0 < i; n >>= 4) r[--i] = _::vtos_codes[n & 0xf];
    return r;
  } else {
    string<Ct> r(sizeof(T) * 2, '0');
    for (nat j = r.size(), n = natcast(v); 0 < j; n >>= 4) r[--j] = _::vtos_codes[n & 0xf];
    return r;
  }
};

template<typename Ct> constexpr string<Ct> to_string(is_none auto) noexcept { //
  return array{Ct('n'), Ct('o'), Ct('n'), Ct('e')}.to_string();
}

template<typename Ct> constexpr string<Ct> to_string(is_nullptr auto) noexcept { return string<Ct>(16, '0'); }

template<typename Ct> constexpr string<Ct> to_string(is_pointer auto v) noexcept {
  if (is_cev) return string<Ct>(16, '0');
  else return to_hex<Ct>(reinterpret_cast<nat>(v));
}

template<typename Ct> constexpr string<Ct> to_string(integral auto v) noexcept {
  if constexpr (is_bool<decltype(v)>) {
    if (v) return array{Ct('t'), Ct('r'), Ct('u'), Ct('e')}.to_string();
    else return array{Ct('f'), Ct('a'), Ct('l'), Ct('s'), Ct('e')}.to_string();
  } else if constexpr (unsigned_integral<decltype(v)>) {
    if (v == 0) return string<Ct>(1, Ct('0'));
    Ct temp[20];
    nat it = 20;
    for (; v != 0; v /= 10) temp[--it] = Ct(v % 10 + '0');
    return string<Ct>(temp + it, 20 - it);
  } else {
    if (v == 0) return string<Ct>(1, Ct('0'));
    const bool neg = v < 0 ? v = -v, true : false;
    Ct temp[20];
    nat it = 20;
    for (; v != 0; v /= 10) temp[--it] = Ct(v % 10 + '0');
    if (neg) temp[--it] = Ct('-');
    return string<Ct>(temp + it, 20 - it);
  }
}

template<typename Ct> constexpr string<Ct> to_string(fat8 v) noexcept {
  const nat bitn = bitcast<nat8>(v);
  if ((bitn & 0x7fffffffffffffff) == 0) {
    if (bitn == 0) return array{Ct('0')}.to_string();
    else return array{Ct('-'), Ct('0')}.to_string();
  } else if ((bitn & 0x7ff0000000000000) == 0x7ff0000000000000) {
    if (bitn == 0x7ff0000000000000) return array{Ct('i'), Ct('n'), Ct('f')}.to_string();
    else if (bitn == 0xfff0000000000000) return array{Ct('-'), Ct('i'), Ct('n'), Ct('f')}.to_string();
    else return array{Ct('n'), Ct('a'), Ct('n')}.to_string();
  } else {
    if (v > 1e+16) return array{Ct('>'), Ct('1'), Ct('e'), Ct('+'), Ct('1'), Ct('6')}.to_string();
    else if (v < -1e+16) return array{Ct('<'), Ct('-'), Ct('1'), Ct('e'), Ct('+'), Ct('1'), Ct('6')}.to_string();
    const auto ii = static_cast<int8>(v);
    auto s0 = to_string<Ct>(ii);
    if (ii < 0) v = static_cast<fat8>(ii) - v;
    else v -= static_cast<fat8>(ii);
    nat m = 20 - (s0.size() + 1);
    for (auto n(m); n > 0; --n) v *= 10;
    auto s1 = string<Ct>(m, '0');
    auto jj = std::abs(static_cast<int8>(v));
    for (; m > 0; jj /= 10) s1[--m] = Ct('0' + jj % 10);
    if (auto sr = std::ranges::search(s1, array<Ct, 5>::fill(Ct('9'))); !sr.empty()) {
      if (auto it = sr.begin(); it == s1.begin()) return ii < 0 ? to_string<Ct>(ii - 1) : to_string<Ct>(ii + 1);
      else s1.resize(it - s1.begin()), ++*--it;
    }
    if (auto sr = std::ranges::search(s1, array<Ct, 5>::fill(Ct('0'))); !sr.empty()) s1.resize(sr.begin() - s1.begin());
    if (s1.empty()) return s0;
    else return s0.push_back(Ct('.')), s0.append(s1), mv(s0);
  }
}

template<character Out> constexpr string<Out> cvt(stringable auto&& s) noexcept {
  using In = iter_value<decltype(s)>;
  string_view<In> v(s);
  if constexpr (sizeof(In) == sizeof(Out)) return string<Out>(bitcast<string_view<Out>>(v));
  else if constexpr (included_in<In, cat1, uct1>) {
    if constexpr (same_as<Out, uct4>) {
      string<uct4> r(v.size(), {});
      auto p = r.data();
      for (auto i = v.data(), last = i + v.size(); i < last;) {
        uct1 c = *i++;
        if (c < 0x80) *p++ = c;
        else if (c < 0xc0) *p++ = 0xfffe;
        else if (c < 0xe0) *p++ = (c & 0x1f) << 6 | (*i++ & 0x3f);
        else if (c < 0xf0) *p++ = (c & 0x0f) << 12 | (*i++ & 0x3f) << 6 | (*i++ & 0x3f);
        else if (c < 0xf8) *p++ = (c & 0x07) << 18 | (*i++ & 0x3f) << 12 | (*i++ & 0x3f) << 6 | (*i++ & 0x3f);
        else *p++ = 0xfffe;
      }
      r.resize(p - r.data());
      return r;
    } else if constexpr (included_in<Out, cat2, uct2>) return cvt<Out>(cvt<uct4>(s));
  } else if constexpr (included_in<In, cat2, uct2>) {
    if constexpr (same_as<Out, uct4>) {
      string<uct4> r(v.size(), {});
      auto p = r.data();
      for (auto i = v.data(), last = i + v.size(); i < last;) {
        if (*i < 0xd800 || *i >= 0xe000) *p++ = *i++;
        else *p++ = *i++ & 0x03ff_u2 | (*i++ & 0x03ff_u2 << 10);
      }
      r.resize(p - r.data());
      return r;
    } else if constexpr (included_in<Out, cat1, uct1>) return cvt<Out>(cvt<uct4>(s));
  } else if constexpr (same_as<In, uct4>) {
    if constexpr (included_in<Out, cat1, uct1>) {
      string<Out> r(v.size() * 4, {});
      auto p = r.data();
      for (auto i = v.data(), last = i + v.size(); i < last;) {
        uct4 c = *i++;
        if (c < 0x80) *p++ = Out(c);
        else if (c < 0x800) *p++ = Out(0xc0 | (c >> 6)), *p++ = Out(0x80 | (c & 0x3f));
        else if (c < 0x10000) {
          *p++ = Out(0xe0 | (c >> 12));         //
          *p++ = Out(0x80 | ((c >> 6) & 0x3f)); //
          *p++ = Out(0x80 | (c & 0x3f));
        } else {
          *p++ = Out(0xf0 | (c >> 18));          //
          *p++ = Out(0x80 | ((c >> 12) & 0x3f)); //
          *p++ = Out(0x80 | ((c >> 6) & 0x3f));  //
          *p++ = Out(0x80 | (c & 0x3f));
        }
      }
      r.resize(p - r.data());
      return r;
    } else if constexpr (included_in<Out, cat2, uct2>) {
      string<Out> r(v.size() * 2, {});
      auto p = r.data();
      for (auto i = v.data(), last = i + v.size(); i < last;) {
        auto c = *i++;
        if (c < 0x10000) *p++ = Out(c);
        else *p++ = Out(0xd800 | ((c - 0x10000) >> 10)), *p++ = Out(0xdc00_u2 | ((c - 0x10000) & 0x3ff));
      }
      r.resize(p - r.data());
      return r;
    }
  }
}

struct source {
  explicit source(const cat1*) = delete;
  const string_view<cat1> file;
  const string_view<cat1> func;
  const nat4 line, column;
  consteval source(const cat1* File = __builtin_FILE(), const cat1* Func = __builtin_FUNCTION(), //
                   nat4 Line = __builtin_LINE(), nat4 Column = __builtin_COLUMN()) noexcept
    : file(File), func(Func), line(Line), column(Column) {}
  template<character Ct> string<Ct> to_string() const {
    string<Ct> s0{}, s1 = yw::to_string<Ct>(line), s2 = yw::to_string<Ct>(column);
    s0.reserve(file.size() + s1.size() + s2.size() + 3);
    s0.append(file), s0.push_back(Ct('(')), s0.append(s1), s0.push_back(Ct(',')), s0.append(s2), s0.push_back(Ct(')'));
    return s0;
  }
  string<cat1> to_string() const { return to_string<cat1>(); }
};

struct date {
  int4 year{}, month{}, day{};
  date() : date(_::get_zoned_time().get_local_time()) {}
  date(numeric auto&& Year, numeric auto&& Month, numeric auto&& Day) noexcept //
    : year(int4(Year)), month(int4(Month)), day(int4(Day)) {}
  template<typename Clock, typename Duration> date(const std::chrono::time_point<Clock, Duration>& tp) {
    const auto ymd = std::chrono::year_month_day(std::chrono::floor<std::chrono::days>(tp));
    year = int4(ymd.year()), month = int4(nat4(ymd.month())), day = int4(nat4(ymd.day()));
  }
  template<character Ct> string<Ct> to_string() const {
    string<Ct> s(10, {});
    if constexpr (sizeof(Ct) == 4) {
      auto t = std::format("{:04d}-{:02d}-{:02d}", year, month, day);
      std::ranges::copy(t, s.data());
    } else if constexpr (sizeof(Ct) == 1) std::format_to(s.data(), "{:04d}-{:02d}-{:02d}", year, month, day);
    else if constexpr (sizeof(Ct) == 2) std::format_to(s.data(), L"{:04d}-{:02d}-{:02d}", year, month, day);
    return s;
  }
  string<cat1> to_string() const { return to_string<cat1>(); }
};

struct clock {
  int4 hour{}, minute{}, second{};
  clock() : clock(_::get_zoned_time().get_local_time()) {}
  clock(numeric auto&& Hour, numeric auto&& Minute, numeric auto&& Second) noexcept //
    : hour(int4(Hour)), minute(int4(Minute)), second(int4(Second)) {}
  template<typename Clock, typename Duration> clock(const std::chrono::time_point<Clock, Duration>& tp) {
    const std::chrono::hh_mm_ss hms(std::chrono::floor<std::chrono::seconds>(tp - std::chrono::floor<std::chrono::days>(tp)));
    hour = int4(hms.hours().count()), minute = int4(hms.minutes().count()), second = int4(hms.seconds().count());
  }
  template<character Ct> string<Ct> to_string() const {
    string<Ct> s(8, {});
    if constexpr (sizeof(Ct) == 4) {
      auto t = std::format("{:02d}:{:02d}:{:02d}", hour, minute, second);
      std::ranges::copy(t, s.data());
    } else if constexpr (sizeof(Ct) == 1) std::format_to(s.data(), "{:02d}:{:02d}:{:02d}", hour, minute, second);
    else if constexpr (sizeof(Ct) == 2) std::format_to(s.data(), L"{:02d}:{:02d}:{:02d}", hour, minute, second);
    return s;
  }
  string<cat1> to_string() const { return to_string<cat1>(); }
};

struct time {
  yw::date date;
  yw::clock clock;
  time() : time(_::get_zoned_time().get_local_time()) {}
  time(const yw::date& Date, const yw::clock& Clock) noexcept : date(Date), clock(Clock) {}
  template<typename Clock, typename Duration> time(const std::chrono::time_point<Clock, Duration>& tp) : date(tp), clock(tp) {}
  operator yw::date() const { return date; }
  operator yw::clock() const { return clock; }
  template<character Ct> string<Ct> to_string() const {
    string<Ct> s(19, {});
    if constexpr (sizeof(Ct) == 4) {
      auto t = std::format("{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}", //
                           date.year, date.month, date.day, clock.hour, clock.minute, clock.second);
      std::ranges::copy(t, s.data());
    } else if constexpr (sizeof(Ct) == 1)                                   //
      std::format_to(s.data(), "{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}", //
                     date.year, date.month, date.day, clock.hour, clock.minute, clock.second);
    else if constexpr (sizeof(Ct) == 2)                                      //
      std::format_to(s.data(), L"{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}", //
                     date.year, date.month, date.day, clock.hour, clock.minute, clock.second);
    return s;
  }
  string<cat1> to_string() const { return to_string<cat1>(); }
};

inline constexpr caster now{[] { return time{}; }};

using path = std::filesystem::path;

class file {
public:
  yw::path path{};
  file() = default;
  file(const yw::path& Path) : path(Path) {}
  file(yw::path&& Path) : path(mv(Path)) {}
  file& operator=(const yw::path& Path) { return path = Path, *this; }
  file& operator=(yw::path&& Path) { return path = mv(Path), *this; }
  bool exists() const { return std::filesystem::exists(path); }
  bool is_file() const { return std::filesystem::is_regular_file(path); }
  bool is_directory() const { return std::filesystem::is_directory(path); }
  nat size() const { return std::filesystem::file_size(path); }
  bool read(void* Data, nat Size) const {
    if (std::ifstream ifs(path, std::ios::binary); !ifs) return false;
    else return ifs.read(static_cast<char*>(Data), Size), nat(ifs.gcount()) == Size;
  }
  bool read(contiguous_range auto&& Range) const { return read(yw::data(Range), yw::size(Range)); }
  template<character Ct> string<Ct> read() const {
    const nat Size = size();
    string<Ct> s(Size / sizeof(Ct), {});
    if (std::ifstream ifs(path, std::ios::binary); ifs) ifs.read(s.data(), Size);
    return s;
  }
  string<cat1> read() const { return read<cat1>(); }
  bool write(const void* Data, nat Size) const {
    if (std::ofstream ofs(path, std::ios::binary); !ofs) return false;
    else return ofs.write(static_cast<const char*>(Data), Size), true;
  }
  bool write(contiguous_range auto&& Range) const { return write(yw::data(Range), yw::size(Range)); }
};

class logger {
protected:
  string<cat1> text{};
public:
  struct level_t {
    string_view<cat1> name;
    int value;
    friend bool operator==(const level_t& l, const level_t& r) noexcept { return l.value == r.value; }
    friend auto operator<=>(const level_t& l, const level_t& r) noexcept { return l.value <=> r.value; }
  };
  static constexpr level_t all{"all", 0};
  static constexpr level_t debug{"debug", 10};
  static constexpr level_t info{"info", 20};
  static constexpr level_t warning{"warn", 30};
  static constexpr level_t error{"error", 40};
  static constexpr level_t critical{"critical", 50};
  path path;
  level_t level = info;
  bool console = true;
  logger(const yw::path& File) noexcept : path(File) {}
  ~logger() noexcept {
    try {
      if (std::basic_ofstream<cat1> ofs(path, std::ios::app); ofs) ofs << text, text.clear();
      else std::cerr << "failed to open log file: " << path << std::endl;
    } catch (...) { std::cerr << "failed to write log file: " << path << std::endl; }
  }
  void operator()(const level_t& Level, stringable auto&& Text) noexcept {
    if (Level < level) return;
    try {
      string<cat1> s;
      if constexpr (same_as<iter_value<decltype(Text)>, cat1>) {
        s = std::format("{} [{}] {}\n", now(), Level.name, string_view<cat1>(Text));
      } else s = std::format("{} [{}] {}\n", now(), Level.name, cvt<cat1>(Text));
      if (console) std::cout << s;
      text += s;
    } catch (...) { std::cerr << "failed to write log" << std::endl; }
  }
  void operator()(const level_t& Level, stringable auto&& Text, const source& Source) noexcept {
    if (Level < level) return;
    if (level > debug) return operator()(Level, fwd<decltype(Text)>(Text));
    try {
      string<cat1> s = std::format("{} [{}] {}: {}\n", now(), Level.name, Source, cvt<cat1>(Text));
      if (console) std::cout << s;
      text += s;
    } catch (...) { std::cerr << "failed to write log" << std::endl; }
  }
  template<typename... Ts> void format( //
    const level_t& Level, const std::format_string<Ts...> Format, Ts&&... Args) noexcept {
    if (Level < level) return;
    try {
      string<cat1> s;
      if constexpr (sizeof...(Ts) == 0) s = std::format("{} [{}] {}\n", now(), Level.name, Format);
      else s = std::format("{} [{}] {}\n", now(), Level.name, std::format(Format, fwd<Ts>(Args)...));
      if (console) std::cout << s;
      text += s;
    } catch (...) { std::cerr << "failed to write log" << std::endl; }
  }
  template<typename... Ts> void format( //
    const level_t& Level, const std::wformat_string<Ts...> Format, Ts&&... Args) noexcept {
    if (Level < level) return;
    try {
      string<cat2> s;
      if constexpr (sizeof...(Ts) == 0) s = std::format(L"{} [{}] {}\n", now(), Level.name, Format);
      else s = std::format(L"{} [{}] {}\n", now(), cvt<cat2>(Level.name), std::format(Format, fwd<Ts>(Args)...));
      if (console) std::wcout << s;
      text += cvt<cat1>(s);
    } catch (...) { std::wcerr << "failed to write log" << std::endl; }
  }
  void flush() noexcept {
    try {
      if (std::basic_ofstream<cat1> ofs(path, std::ios::app); ofs) ofs << text, text.clear();
      else std::cerr << "failed to open log file: " << path << std::endl;
    } catch (...) { std::cerr << "failed to write log file: " << path << std::endl; }
  }
};
}

namespace std {

template<typename Ct> struct formatter<yw::source, Ct> : formatter<basic_string<Ct>, Ct> {
  auto format(const yw::source& s, auto& Ctx) const { return formatter<basic_string<Ct>, Ct>::format(s.to_string(), Ctx); }
};

template<typename Ct> struct formatter<yw::date, Ct> : formatter<basic_string<Ct>, Ct> {
  auto format(const yw::date& d, auto& Ctx) const { return formatter<basic_string<Ct>, Ct>::format(d.to_string(), Ctx); }
};

template<typename Ct> struct formatter<yw::clock, Ct> : formatter<basic_string<Ct>, Ct> {
  auto format(const yw::clock& c, auto& Ctx) const { return formatter<basic_string<Ct>, Ct>::format(c.to_string(), Ctx); }
};

template<typename Ct> struct formatter<yw::time, Ct> : formatter<basic_string<Ct>, Ct> {
  auto format(const yw::time& t, auto& Ctx) const { return formatter<basic_string<Ct>, Ct>::format(t.to_string(), Ctx); }
};

template<typename Ct> struct formatter<yw::file, Ct> : formatter<basic_string<Ct>, Ct> {
  auto format(const yw::file& f, auto& Ctx) const { return formatter<basic_string<Ct>, Ct>::format(f.path.string<Ct>(), Ctx); }
};
}
