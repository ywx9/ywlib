#pragma once
#include <core/core.h>
#include <core/property.h>
#include <core/string.h>

namespace yw {

inline constexpr auto here(std::source_location loc = std::source_location::current()) noexcept { return loc; }

inline constexpr uint64_t unique_id(std::source_location loc = here()) noexcept {
  constexpr uint64_t basis = 14695981039346656037ull;
  constexpr uint64_t prime = 1099511628211ull;
  auto hash = [](std::string_view s) {
    uint64_t h = basis;
    for (char c : s) { h ^= uint8_t(c), h *= prime; }
    return h;
  };
  uint64_t h = basis;
  auto mix = [&](uint64_t v) { h ^= v, h *= prime; };
  mix(hash(loc.file_name())), mix(loc.line()), mix(loc.column());
  return h;
}

struct error;

struct error_type {
  static const error_type ok;
  size_t value;
  constexpr error_type(std::source_location sl = here()) noexcept : value(unique_id(sl)) {}
  constexpr bool operator==(const error_type& o) const noexcept { return value == o.value; }

private:
  consteval error_type(size_t v) : value(v) {}
};

inline constexpr error_type error_type::ok(size_t(0));

namespace errors {
inline constexpr const auto& ok = error_type::ok;
inline constexpr error_type unknown{};
inline constexpr error_type invalid_argument{};
inline constexpr error_type invalid_operation{};
inline constexpr error_type operation_failed{};
inline constexpr error_type unreachable{};
} // namespace errors

struct error {
  inline static property<string<char>, error> message = {};
  inline static const_property<string<char>, error> footprint = {};
  inline static const_property<error_type, error> type = errors::ok;

  const_property<bool, error> has = false;
  explicit constexpr operator bool() const noexcept { return has; }

  constexpr ~error() noexcept {
    if (has && type != errors::ok) {
      "unhandled error: "_print_error;
      print();
      ::abort();
    }
  }

  constexpr error() noexcept = default;
  constexpr error(const error& o) = delete;
  constexpr error& operator=(const error& o) = delete;

  constexpr error(error&& o) noexcept : has(exchange(o.has.ref(), false)) {}

  constexpr error& operator=(error&& o) noexcept {
    if (this != &o) {
      if (has && type != errors::ok) {
        "unhandled error: "_print_error;
        print();
        ::abort();
      }
      has = exchange(o.has.ref(), false);
    }
    return *this;
  }

  constexpr error(error_type t, string<char> m, std::source_location sl = here()) noexcept {
    if (t == errors::ok) return;
    bool already_set = type != errors::ok;
    if (already_set) {
      "error already set: "_print_error;
      print();
      footprint.ref().clear();
    }
    type = t;
    message = move(m);
    footprint.ref().reserve(256);
    footprint.ref().append("* ");
    footprint.ref().append(sl.file_name());
    footprint.ref().append(":");
    footprint.ref().append(vtos<char>(sl.line()));
    footprint.ref().append(":");
    footprint.ref().append(vtos<char>(sl.column()));
    has = true;
    if (already_set) {
      "new error: "_print_error;
      print();
      ::abort();
    }
  }

  constexpr error(string<char> m, std::source_location sl = here()) noexcept : error(errors::unknown, move(m), sl) {}

  static constexpr void add_footprint(std::source_location sl = here()) noexcept {
    if (type == errors::ok) return;
    footprint.ref().append("\n");
    footprint.ref().append("^ ");
    footprint.ref().append(sl.file_name());
    footprint.ref().append(":");
    footprint.ref().append(vtos<char>(sl.line()));
    footprint.ref().append(":");
    footprint.ref().append(vtos<char>(sl.column()));
  }

  static constexpr void print() noexcept {
    if (!std::is_constant_evaluated()) {
      if (type != errors::ok) {
        ::fputs(message().c_str(), stderr);
        ::fputs("\n", stderr);
        ::fputs(footprint().c_str(), stderr);
        ::fputs("\n", stderr);
      } else ::fputs("no error\n", stderr);
    }
  }

  constexpr void ignore() noexcept {
    if (!has) return;
    type = errors::ok;
    message.ref().clear();
    footprint.ref().clear();
    has = false;
  }

  constexpr void consume(std::source_location sl = here()) noexcept {
    if (has && type != errors::ok) {
      add_footprint(sl);
      print();
      ignore();
    }
  }
};

///--------------------------------------------------------------------------///
/// MARK: result

template<typename T> class result {
  union {
    T _value;
    yw::error _error;
  };

public:
  const_property<bool, result> _has_value = false;
  const_property<bool, result> _has_error = false;

  constexpr ~result() noexcept {
    if (_has_value()) { _value.~T(); }
  }

  template<typename... As> requires constructible<T, As...>
  constexpr result(As&&... as) noexcept(nt_constructible<T, As...>) {
    std::start_lifetime(&_value);
    new (&_value) T(static_cast<As&&>(as)...);
    _has_value = true;
  }

  template<typename... As> requires constructible<yw::error, As...> && (!constructible<T, As...>)constexpr result(
    As&&... as) noexcept(nt_constructible<yw::error, As...>) {
    std::start_lifetime(&_error);
    new (&_error) yw::error(static_cast<As&&>(as)...);
    _has_error = true;
  }

  constexpr T* operator->() noexcept { return &_value; }
  constexpr T& operator*() noexcept { return _value; }

  explicit constexpr operator bool() const noexcept { return _has_value(); }

  constexpr T& value() & noexcept {
    if (!_has_value()) {

    }
    return _value;
  }
  constexpr const T& value() const & noexcept { return _value; }
  constexpr T&& value() && noexcept { return static_cast<T&&>(_value); }
  constexpr const T&& value() const && noexcept { return static_cast<const T&&>(_value); }

  constexpr yw::error& error() & noexcept { return _error; }
  constexpr const yw::error& error() const & noexcept { return _error; }
  constexpr yw::error&& error() && noexcept { return static_cast<yw::error&&>(_error); }
  constexpr const yw::error&& error() const && noexcept { return static_cast<const yw::error&&>(_error); }
};
} // namespace yw
