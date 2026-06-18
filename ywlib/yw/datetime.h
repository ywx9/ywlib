#pragma once
#include "yw/string.h"

namespace yw {

namespace internal {
inline auto current_datetime() noexcept {
  return std::chrono::zoned_time(std::chrono::current_zone(), std::chrono::system_clock::now()).get_local_time();
}
} // namespace internal

//////////////////////////////////////// MARK: date

struct date {
  int year{};
  unsigned month{}, day{};

  date() noexcept : date(internal::current_datetime()) {}

  constexpr date(arithmetic auto y, arithmetic auto m, arithmetic auto d) noexcept
    : year(int(y)), month(unsigned(m)), day(unsigned(d)) {}

  constexpr date(const std::chrono::year_month_day& ymd)
    : year(int(ymd.year())), month(unsigned(ymd.month())), day(unsigned(ymd.day())) {}

  template<typename Clock, typename Dur> constexpr date(const std::chrono::time_point<Clock, Dur>& tp) noexcept
    : date(std::chrono::year_month_day(std::chrono::floor<std::chrono::days>(tp))) {}

  constexpr string<char> to_string() const { return to_string<char>(); }

  template<char_type C> constexpr string<C> to_string() const {
    const bool b = year < 0;
    string<C> r(10 + b, '0');
    auto p = r.data();
    int y = b ? -year : year;
    if (b) *p++ = C('-');
    *p++ = C('0' + y / 1000), y %= 1000;
    *p++ = C('0' + y / 100), y %= 100;
    *p++ = C('0' + y / 10), y %= 10;
    *p++ = C('0' + y);
    *p++ = C('-');
    int m = month % 100;
    *p++ = C('0' + m / 10), *p++ = C('0' + m % 10), *p++ = C('-');
    int d = day % 100;
    *p++ = C('0' + d / 10), *p++ = C('0' + d % 10);
    return r;
  }
};

//////////////////////////////////////// MARK: time

struct time {
  unsigned hour{}, minute{}, second{};

  time() : time(internal::current_datetime()) {}

  constexpr time(arithmetic auto h, arithmetic auto m, arithmetic auto s) noexcept
    : hour(unsigned(h)), minute(unsigned(m)), second(unsigned(s)) {}

  constexpr time(const std::chrono::hh_mm_ss<std::chrono::seconds>& hms) noexcept
    : time(hms.hours().count(), hms.minutes().count(), hms.seconds().count()) {}

  template<typename Clock, typename Duration> time(const std::chrono::time_point<Clock, Duration>& tp)
    : time(
        std::chrono::hh_mm_ss(
          std::chrono::floor<std::chrono::seconds>(tp - std::chrono::floor<std::chrono::days>(tp)))) {}

  constexpr string<char> to_string() const { return to_string<char>(); }

  template<char_type C> constexpr string<C> to_string() const {
    string<C> r(8);
    auto p = r.data();
    auto h = hour % 100;
    *p++ = C('0' + h / 10), *p++ = C('0' + h % 10), *p++ = C(':');
    int m = minute % 100;
    *p++ = C('0' + m / 10), *p++ = C('0' + m % 10), *p++ = C(':');
    int s = second % 100;
    *p++ = C('0' + s / 10), *p++ = C('0' + s % 10);
    return r;
  }
};

//////////////////////////////////////// MARK: DATETIME

struct datetime {
  yw::date date;
  yw::time time;
  explicit constexpr operator yw::date() const { return date; }
  explicit constexpr operator yw::time() const { return time; }

  datetime() : datetime(internal::current_datetime()) {}
  constexpr datetime(const yw::date& Date, const yw::time& Time) noexcept : date(Date), time(Time) {}
  template<typename Clock, typename Dur> datetime(const std::chrono::time_point<Clock, Dur>& tp) : date(tp), time(tp) {}

  constexpr string<char> to_string() const { return to_string<char>(); }

  template<char_type C> constexpr string<C> to_string() const {
    const bool b = date.year < 0;
    string<C> r(19 + b, '0');
    auto p = r.data();
    int y = b ? -date.year : date.year;
    if (b) *p++ = C('-');
    *p++ = C('0' + y / 1000), y %= 1000;
    *p++ = C('0' + y / 100), y %= 100;
    *p++ = C('0' + y / 10), y %= 10;
    *p++ = C('0' + y);
    *p++ = C('-');
    int m = date.month % 100;
    *p++ = C('0' + m / 10), *p++ = C('0' + m % 10), *p++ = C('-');
    int d = date.day % 100;
    *p++ = C('0' + d / 10), *p++ = C('0' + d % 10), *p++ = C(' ');
    auto h = time.hour % 100;
    *p++ = C('0' + h / 10), *p++ = C('0' + h % 10), *p++ = C(':');
    int min = time.minute % 100;
    *p++ = C('0' + min / 10), *p++ = C('0' + min % 10), *p++ = C(':');
    int s = time.second % 100;
    *p++ = C('0' + s / 10), *p++ = C('0' + s % 10);
    return r;
  }
};

//////////////////////////////////////// MARK: DATETIMEMS

struct datetimems : datetime {
  unsigned ms{};
  explicit constexpr operator yw::date() const { return date; }
  explicit constexpr operator yw::time() const { return time; }

  datetimems() : datetimems(internal::current_datetime()) {}

  constexpr datetimems(const datetime& dt, arithmetic auto ms) noexcept : datetime(dt), ms(unsigned(ms)) {}

  constexpr datetimems(const yw::date& d, const yw::time& t, arithmetic auto ms) noexcept
    : datetime(d, t), ms(unsigned(ms)) {}

  template<typename Clock, typename Dur> datetimems(const std::chrono::time_point<Clock, Dur>& tp)
    : datetime(tp),
      ms(unsigned(std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()).count() % 1000)) {}

  constexpr string<char> to_string() const { return to_string<char>(); }

  template<char_type C> constexpr string<C> to_string() const {
    const bool b = date.year < 0;
    string<C> r(23 + b, '0');
    auto p = r.data();
    int y = b ? -date.year : date.year;
    if (b) *p++ = C('-');
    *p++ = C('0' + y / 1000), y %= 1000;
    *p++ = C('0' + y / 100), y %= 100;
    *p++ = C('0' + y / 10), y %= 10;
    *p++ = C('0' + y);
    *p++ = C('-');
    int m = date.month % 100;
    *p++ = C('0' + m / 10), *p++ = C('0' + m % 10), *p++ = C('-');
    int d = date.day % 100;
    *p++ = C('0' + d / 10), *p++ = C('0' + d % 10), *p++ = C(' ');
    auto h = time.hour % 100;
    *p++ = C('0' + h / 10), *p++ = C('0' + h % 10), *p++ = C(':');
    int min = time.minute % 100;
    *p++ = C('0' + min / 10), *p++ = C('0' + min % 10), *p++ = C(':');
    int s = time.second % 100;
    *p++ = C('0' + s / 10), *p++ = C('0' + s % 10);
    int ms_ = ms % 1000;
    *p++ = C('.'), *p++ = C('0' + ms_ / 100), ms_ %= 100;
    *p++ = C('0' + ms_ / 10), ms_ %= 10;
    *p++ = C('0' + ms_);
    return r;
  }
};
} // namespace yw

namespace std {

template<typename C> struct formatter<yw::date, C> {
  formatter<basic_string<C>, C> fmt;
  constexpr auto parse(auto& ctx) { return fmt.parse(ctx); }
  auto format(const yw::date& d, auto& ctx) const { return fmt.format(d.to_string<C>(), ctx); }
};

template<typename C> struct formatter<yw::time, C> {
  formatter<basic_string<C>, C> fmt;
  constexpr auto parse(auto& ctx) { return fmt.parse(ctx); }
  auto format(const yw::time& t, auto& ctx) const { return fmt.format(t.to_string<C>(), ctx); }
};

template<typename C> struct formatter<yw::datetime, C> {
  formatter<basic_string<C>, C> fmt;
  constexpr auto parse(auto& ctx) { return fmt.parse(ctx); }
  auto format(const yw::datetime& dt, auto& ctx) const { return fmt.format(dt.to_string<C>(), ctx); }
};

template<typename C> struct formatter<yw::datetimems, C> {
  formatter<basic_string<C>, C> fmt;
  constexpr auto parse(auto& ctx) { return fmt.parse(ctx); }
  auto format(const yw::datetimems& dtms, auto& ctx) const { return fmt.format(dtms.to_string<C>(), ctx); }
};
} // namespace std
