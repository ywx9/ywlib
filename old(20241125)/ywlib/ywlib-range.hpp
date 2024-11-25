#pragma once

#include "ywlib-std.hpp"

////////////////////////////////////////////////////////////////////////////////
// STATIC STRING
export namespace yw {
/// \brief constant evaluated static string
template<character Ct, nat N> class static_string {
  static_assert(same_as<Ct, remove_cv<Ct>>);
  array<Ct, N + 1> _str{};
public:
  using value_type = Ct;
  consteval static_string(const uct4 (&Str)[N + 1]) noexcept { cvt<Ct>(Str, _str, N); }
  constexpr operator string_view<Ct>() const noexcept { return _str; }
  constexpr const Ct* begin() const noexcept { return _str.begin(); }
  constexpr const Ct* end() const noexcept { return _str.end(); }
  constexpr const Ct* data() const noexcept { return _str.data(); }
  constexpr nat size() const noexcept { return _str.size(); }
  constexpr bool empty() const noexcept { return _str.empty(); }
  constexpr const Ct& front() const noexcept { return _str.front(); }
  constexpr const Ct& back() const noexcept { return _str.back(); }
  constexpr const Ct& operator[](nat I) const noexcept { return _str[I]; }
  template<nat I> requires (I < N) constexpr const Ct& get() const noexcept { return _str[I]; }
};

/// \brief creates a static string from a string literal
template<character Ct> inline constexpr auto make_static_string = //
  []<nat N>(const uct4 (&Str)[N]) { return static_string<Ct, N - 1>(Str); };
}

////////////////////////////////////////////////////////////////////////////////
// STRING UTILITY
export namespace yw {
namespace _ {
inline constexpr cat1 _to_hex_table[] = "0123456789abcdef";
constexpr fat8 _ftos_log10(fat8 const v) noexcept {
  int8 t = bitcast<int8>(v), exp = ((t & 0x7ff0000000000000) >> 52) - 1023;
  return exp * log10_2 - 0.652998017485824 + [&](const fat8 x) noexcept { //
    return x * (0.924793176223418 - 0.319987904734089 * x + 0.0480424904500012 * x * x);
  }(bitcast<fat8>((t & 0x000fffffffffffff) | 0x3ff0000000000000));
}
template<typename Ct> constexpr string<Ct> _ftos(fat8 v) {
  const auto bitn = bitcast<nat8>(v);
  if ((bitn & 0x7fffffffffffffff) == 0) { // plus/minus zero
    if (bitn == 0) return make_static_string<Ct>(U"0.");
    else return make_static_string<Ct>(U"-0.");
  } else if ((bitn & 0x7ff0000000000000) == 0x7ff0000000000000) { // inf or nan
    if (bitn == 0x7ff0000000000000) return make_static_string<Ct>(U"inf");
    else if (bitn == 0xfff0000000000000) return make_static_string<Ct>(U"-inf");
    else return make_static_string<Ct>(U"nan");
  } else { // normal number
    const bool neg = v < 0 ? (v = -v, true) : false;
    if (v > 1e+16) { // too large
      if (neg) return make_static_string<Ct>(U">1e+16");
      else return make_static_string<Ct>(U"<-1e+16");
    } else {
      string<Ct> s(20, Ct{});
      if (auto exp = int8(_ftos_log10(v)); exp < 0) {
        nat i{};
        if (neg) s[i++] = Ct('-');
        s[i++] = Ct('0'), s[i++] = Ct('.');
        for (nat j{i}; j < 20; ++j) v *= 10;
        nat n = static_cast<nat>(v);
        for (nat j{19}; i <= j; n /= 10) s[j--] = Ct(n % 10 + '0');
      } else {
        nat i{};
        if (neg) s[i++] = Ct('-');
        nat j = 20 - (exp + i + 2); // frac part length; integral part(exp + 1) + dot(1)
        for (nat j{i}; j < 20; ++j) v *= 10;
        for (nat j = 20 - (exp + 2 + i); j < 20;) nat n = static_cast<nat>(v);
      }
    }

    // length of result is less than 20
    string<Ct> s(20, Ct{});
    nat i{}, j{}, n = static_cast<nat>(v);
    // appends `-` if negative
    if (neg) s[i++] = Ct('-');
    // appends integer part
    for (nat w = n; w > 0; w /= 10) s[20 - ++j] = Ct(w % 10 + '0');
    if (j == 0) s[20 - ++j] = Ct('0');
    for (; j > 0;) s[i++] = s[20 - j--];
    // appends decimal point
    s[i++] = Ct('.');
    v = (v - static_cast<fat8>(n));
    for (nat k{i}; k < 20; ++k) v *= 10;
    n = static_cast<nat>(v);
    for (nat k{19}; i <= k; n /= 10) s[k--] = Ct(n % 10 + '0');
    auto dot = s.find(Ct('.'));
  }
}
}

/// \brief converts a scalar value to a string in hexadecimal format
template<character Ct> inline constexpr auto to_hex = //
  []<scalar T>(const T Scalar) noexcept -> string<Ct> {
  if constexpr (is_pointer<T>) {
    string<Ct> s(16, Ct('0'));
    if (!is_cev) {
      auto n = reinterpret_cast<nat>(Scalar);
      for (nat i = 0; i < 16; ++i, n >>= 4) s[15 - i] = _::to_hex_table[n & 0xf];
    }
    return s;
  } else {
    string<Ct> s(sizeof(T) * 2, Ct('0'));
    auto n = natcast(Scalar);
    for (nat i = 0, m = s.size() - 1; n != 0; ++i, n >>= 4) s[m - i] = _::to_hex_table[n & 0xf];
    return s;
  }
};

/// \brief converts a scalar value to a string
template<character Ct> constexpr auto to_string(const scalar auto Scalar) noexcept -> string<Ct> {
  if constexpr (is_none<decltype(Scalar)>) return string<Ct>(make_static_string<Ct>(U"none"));
  else if constexpr (is_bool<decltype(Scalar)>) {
    if (Scalar) return string<Ct>(make_static_string<Ct>(U"true"));
    else return string<Ct>(make_static_string<Ct>(U"false"));
  } else if constexpr (is_nullptr<decltype(Scalar)>) return string<Ct>(make_static_string<Ct>("nullptr"));
  else if constexpr (is_pointer<decltype(Scalar)>) return to_hex<Ct>(Scalar);
  else if constexpr (unsigned_integral<decltype(Scalar)>) {
    if (Scalar == 0) return string<Ct>(1, Ct('0'));
    string<Ct> s(20, Ct{});
    nat i = 0;
    for (; Scalar != 0; ++i, Scalar /= 10) s[i] = Ct(Scalar % 10 + '0');
    std::reverse(s.begin(), s.begin() + i), s.resize(i);
    return s;
  } else if constexpr (signed_integral<decltype(Scalar)>) {
    if (Scalar == 0) return string<Ct>(1, Ct('0'));
    string<Ct> s(20, Ct{});
    nat i = 0;
    bool neg = Scalar < 0;
    for (; Scalar != 0; ++i, Scalar /= 10) s[i] = Ct(std::abs(Scalar) % 10 + '0');
    if (neg) s[i++] = Ct('-');
    std::reverse(s.begin(), s.begin() + i), s.resize(i);
    return s;
  } else return _::_ftos<Ct>(Scalar);
};
}

////////////////////////////////////////////////////////////////////////////////
// SOURCE
export namespace yw {
/// \brief source location
struct source {
  explicit source(const cat1*) = delete;

  /// \brief file name
  const cat1* const file;

  /// \brief function name
  const cat1* const func;

  /// \brief line number
  const nat4 line;

  /// \brief column number
  const nat4 column;

  /// \brief default constructor
  source(const cat1* File = __builtin_FILE(), const cat1* Func = __builtin_FUNCTION(), //
         nat4 Line = __builtin_LINE(), nat4 Column = __builtin_COLUMN()) noexcept
    : file(File), func(Func), line(Line), column(Column) {}
};
}
namespace std {
template<typename Ct> struct formatter<yw::source, Ct> : formatter<basic_string<Ct>, Ct> {
  static constexpr auto _fmt = yw::make_static_string<Ct>(U"{}({},{})");
  auto format(const yw::source& s, auto& Ctx) const { //
    return formatter<basic_string<Ct>, Ct>::format(std::format(_fmt, s.file, s.line, s.column), Ctx);
  }
};
}

////////////////////////////////////////////////////////////////////////////////
// TIME
export namespace yw {
namespace _ {
auto get_zoned_time() { return std::chrono::zoned_time(std::chrono::current_zone(), std::chrono::system_clock::now()); }
}

/// \brief structure to represent a date
struct date {
  /// \brief year
  int4 year{};
  /// \brief month
  int4 month{};
  /// \brief day
  int4 day{};

  /// \brief default constructor
  date() : date(_::get_zoned_time().get_local_time()) {}

  /// \brief constructor from components
  date(numeric auto&& Year, numeric auto&& Month, numeric auto&& Day) noexcept //
    : year(int4(Year)), month(int4(Month)), day(int4(Day)) {}

  /// \brief constructor from `std::chrono::time_point`
  template<typename Clock, typename Duration> date(const std::chrono::time_point<Clock, Duration>& tp) {
    const auto ymd = std::chrono::year_month_day(std::chrono::floor<std::chrono::days>(tp));
    year = int4(ymd.year()), month = int4(nat4(ymd.month())), day = int4(nat4(ymd.day()));
  }

  /// \brief converts to `string<cat1>`
  operator string<cat1>() const { return std::format("{:04d}-{:02d}-{:02d}", year, month, day); }

  /// \brief converts to `string<Ct>`
  template<character Ct> explicit operator string<Ct>() const {
    string<Ct> s(10, {});
    if constexpr (sizeof(Ct) == 4) {
      auto t = std::format("{:04d}-{:02d}-{:02d}", year, month, day);
      std::ranges::copy(t, s.data());
    } else if constexpr (sizeof(Ct) == 1) std::format_to(s.data(), "{:04d}-{:02d}-{:02d}", year, month, day);
    else if constexpr (sizeof(Ct) == 2) std::format_to(s.data(), L"{:04d}-{:02d}-{:02d}", year, month, day);
    return s;
  }
};

////////////////////////////////////////////////////////////////////////////////
/// \brief clock structure
struct clock {
  /// \brief hour
  int4 hour{};
  /// \brief minute
  int4 minute{};
  /// \brief second
  int4 second{};

  /// \brief default constructor
  clock() : clock(_::get_zoned_time().get_local_time()) {}

  /// \brief constructor from components
  clock(numeric auto&& Hour, numeric auto&& Minute, numeric auto&& Second) noexcept //
    : hour(int4(Hour)), minute(int4(Minute)), second(int4(Second)) {}

  /// \brief constructor from `std::chrono::time_point`
  template<typename Clock, typename Duration> clock(const std::chrono::time_point<Clock, Duration>& tp) {
    const std::chrono::hh_mm_ss hms(std::chrono::floor<std::chrono::seconds>(tp - std::chrono::floor<std::chrono::days>(tp)));
    hour = int4(hms.hours().count()), minute = int4(hms.minutes().count()), second = int4(hms.seconds().count());
  }

  /// \brief converts to `string<cat1>`
  operator string<cat1>() const { return std::format("{:02d}:{:02d}:{:02d}", hour, minute, second); }

  /// \brief converts to `string<Ct>`
  template<character Ct> explicit operator string<Ct>() const {
    string<Ct> s(8, {});
    if constexpr (sizeof(Ct) == 4) {
      auto t = std::format("{:02d}:{:02d}:{:02d}", hour, minute, second);
      std::ranges::copy(t, s.data());
    } else if constexpr (sizeof(Ct) == 1) std::format_to(s.data(), "{:02d}:{:02d}:{:02d}", hour, minute, second);
    else if constexpr (sizeof(Ct) == 2) std::format_to(s.data(), L"{:02d}:{:02d}:{:02d}", hour, minute, second);
    return s;
  }
};

////////////////////////////////////////////////////////////////////////////////
/// \brief date and clock structure
struct time {
  /// \brief date
  yw::date date;
  /// \brief clock
  yw::clock clock;

  /// \brief default constructor
  time() : time(_::get_zoned_time().get_local_time()) {}

  /// \brief constructor from components
  time(const yw::date& Date, const yw::clock& Clock) noexcept : date(Date), clock(Clock) {}

  /// \brief constructor from `std::chrono::time_point`
  template<typename Clock, typename Duration> //
  time(const std::chrono::time_point<Clock, Duration>& tp) : date(tp), clock(tp) {}

  /// \brief converts to date
  operator yw::date() const { return date; }

  /// \brief converts to clock
  operator yw::clock() const { return clock; }

  /// \brief converts to `string<cat1>`
  operator string<cat1>() const {                                   //
    return std::format("{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}", //
                       date.year, date.month, date.day, clock.hour, clock.minute, clock.second);
  }

  /// \brief converts to `string<Ct>`
  template<character Ct> explicit operator string<cat2>() const {
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
};

/// \brief caster to obtain the current time
inline constexpr caster now{[] { return time{}; }};
}
namespace std {
template<typename Ct> struct formatter<yw::date, Ct> : formatter<basic_string<Ct>, Ct> {
  auto format(const yw::date& d, auto& Ctx) const { //
    return formatter<basic_string<Ct>, Ct>::format(basic_string<Ct>(d), Ctx);
  }
};
template<typename Ct> struct formatter<yw::clock, Ct> : formatter<basic_string<Ct>, Ct> {
  auto format(const yw::clock& c, auto& Ctx) const { //
    return formatter<basic_string<Ct>, Ct>::format(basic_string<Ct>(c), Ctx);
  }
};

template<typename Ct> struct formatter<yw::time, Ct> : formatter<basic_string<Ct>, Ct> {
  auto format(const yw::time& t, auto& Ctx) const { //
    return formatter<basic_string<Ct>, Ct>::format(basic_string<Ct>(t), Ctx);
  }
};
}

////////////////////////////////////////////////////////////////////////////////
// FILE
export namespace yw {
/// \brief file path
using path = std::filesystem::path;

/// \brief class for operating a file
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
    if (std::ifstream ifs(path, std::ios::binary); ifs) {
      ifs.read(static_cast<char*>(Data), Size);
      return ifs.gcount() == Size;
    } else return false;
  }
  bool read(contiguous_range auto&& Range) const { //
    return read(yw::data(Range), yw::size(Range));
  }
  template<character Ct> string<Ct> read() const {
    const nat Size = size();
    string<Ct> s(Size / sizeof(Ct), {});
    if (std::ifstream ifs(path, std::ios::binary); ifs) {
      ifs.read(s.data(), Size);
      return s;
    } else return {};
  }
  string<cat1> read() const { return read<cat1>(); }

  bool write(const void* Data, nat Size) const {
    if (std::ofstream ofs(path, std::ios::binary); ofs) {
      ofs.write(static_cast<const char*>(Data), Size);
      return true;
    } else return false;
  }
  bool write(contiguous_range auto&& Range) const { //
    return write(yw::data(Range), yw::size(Range));
  }
};

}

////////////////////////////////////////////////////////////////////////////////
// LOGGER
export namespace yw {

/// \brief class for logging
class logger {
protected:
  string<cat1> text{};
public:
  /// \brief logging level type
  struct level_t {
    string_view<cat1> name;
    int value;
    friend bool operator==(const level_t& l, const level_t& r) noexcept { return l.value == r.value; }
    friend auto operator<=>(const level_t& l, const level_t& r) noexcept { return l.value <=> r.value; }
  };

  /// \brief level to output all logs
  static constexpr level_t all{"all", 0};

  /// \brief level to output debug logs
  static constexpr level_t debug{"debug", 10};

  /// \brief level to output info logs
  static constexpr level_t info{"info", 20};

  /// \brief level to output warning logs
  static constexpr level_t warning{"warn", 30};

  /// \brief level to output error logs
  static constexpr level_t error{"error", 40};

  /// \brief level to output critical logs
  static constexpr level_t critical{"critical", 50};

  /// \brief file path
  path path;

  /// \brief level
  level_t level = info;

  /// \brief flag to output to console
  bool console = true;

  /// \brief constructor
  logger(const yw::path& File) noexcept : path(File) {}

  /// \brief destructor; writes to log
  ~logger() noexcept {
    try {
      if (std::basic_ofstream<cat1> ofs(path, std::ios::app); ofs) ofs << text, text.clear();
      else std::cerr << "failed to open log file: " << path << std::endl;
    } catch (...) { std::cerr << "failed to write log file: " << path << std::endl; }
  }

  /// \brief appends text to log
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

  /// \brief appends text to log
  void operator()(const level_t& Level, stringable auto&& Text, const source& Source) noexcept {
    if (Level < level) return;
    if (level > debug) return operator()(Level, fwd<decltype(Text)>(Text)); // avoid leaking source code information
    try {
      string<cat1> s = std::format("{} [{}] {}: {}\n", now(), Level.name, Source, cvt<cat1>(Text));
      if (console) std::cout << s;
      text += s;
    } catch (...) { std::cerr << "failed to write log" << std::endl; }
  }

  /// \brief checks level and appends formatted text to the log
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

  /// \brief checks level and appends formatted text to the log
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

  /// \brief flushes the log
  void flush() noexcept {
    try {
      if (std::basic_ofstream<cat1> ofs(path, std::ios::app); ofs) ofs << text, text.clear();
      else std::cerr << "failed to open log file: " << path << std::endl;
    } catch (...) { std::cerr << "failed to write log file: " << path << std::endl; }
  }
};
}
