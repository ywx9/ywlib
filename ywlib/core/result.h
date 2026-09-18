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
    if (has && type != errors::ok) print_and_abort("unhandled error: ");
  }

  constexpr error() noexcept = default;
  constexpr error(const error& o) = delete;
  constexpr error& operator=(const error& o) = delete;

  constexpr error(error&& o) noexcept : has(exchange(o.has.ref(), false)) {}

  constexpr error& operator=(error&& o) noexcept {
    if (this != &o) {
      if (has && type != errors::ok) print_and_abort("unhandled error: ");
      has = exchange(o.has.ref(), false);
    }
    return *this;
  }

  constexpr error(error_type t, string<char> m, std::source_location sl = here()) noexcept {
    if (t == errors::ok) return;
    bool already_set = type != errors::ok;
    if (already_set) {
      print_error("error already set: ");
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
    if (already_set) print_and_abort("new error: ");
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

  static constexpr void print(const char* msg = nullptr) noexcept {
    if (std::is_constant_evaluated()) return;
    if (type != errors::ok) {
      if (msg) ::fputs(msg, stderr);
      ::fputs(message().c_str(), stderr);
      ::fputs("\n", stderr);
      ::fputs(footprint().c_str(), stderr);
      ::fputs("\n", stderr);
    } else ::fputs("no error\n", stderr);
  }

  static constexpr void print_and_abort(const char* msg = "fatal error: ") noexcept {
    print(msg);
    ::abort();
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
  union _union {
    none _none;
    T _value;
    error _error;
    constexpr ~_union() noexcept {}
    constexpr _union() noexcept : _none() {}
  } _union;

public:
  const_property<bool, result> has_value = false;
  const_property<bool, result> has_error = false;

  constexpr ~result() noexcept {
    if (has_value()) _union._value.~T();
    if (has_error()) _union._error.~error();
  }

  constexpr result(result&& o) noexcept(nt_constructible<T, T> && nt_assignable<T, T>) {
    if (o.has_value()) {
      new (&_union._value) T(move(o._union._value));
      has_value = true;
      o.has_value = false;
    } else if (o.has_error()) {
      new (&_union._error) yw::error(move(o._union._error));
      has_error = true;
      o.has_error = false;
    }
  }

  template<typename... As> requires constructible<T, As...>
  constexpr result(As&&... as) noexcept(nt_constructible<T, As...>) {
    new (&_union._value) T(static_cast<As&&>(as)...);
    has_value = true;
  }

  template<typename... As> requires constructible<yw::error, As...> && (!constructible<T, As...>)constexpr result(
    As&&... as) noexcept(nt_constructible<yw::error, As...>) {
    new (&_union._error) yw::error(static_cast<As&&>(as)...);
    has_error = true;
  }

  constexpr T* operator->() & noexcept { return std::addressof(value()); }
  constexpr const T* operator->() const& noexcept { return std::addressof(value()); }

  constexpr T& operator*() & noexcept { return value(); }
  constexpr const T& operator*() const& noexcept { return value(); }
  constexpr T&& operator*() && noexcept { return value(); }
  constexpr const T&& operator*() const&& noexcept { return value(); }

  explicit constexpr operator bool() const noexcept { return has_value(); }

  template<typename Self> constexpr auto&& value(this Self&& self) noexcept {
    if (!self.has_value()) {
      if (self.has_error()) {
        print_error("attempted to access value of result when it has error");
        self._union._error.print_and_abort();
      } else yw::error("attempted to access value of result when it has neither value nor error").print_and_abort();
    }
    return static_cast<copy_cvref<Self&&, T>>(self._union._value);
  }

  template<typename Self> constexpr auto&& error(this Self&& self) noexcept {
    if (!self.has_error()) yw::error("attempted to access error of result when it has no error").print_and_abort();
    return static_cast<copy_cvref<Self&&, yw::error>>(self._union._error);
  }
};

template<> class result<void> {
  union _union {
    none _none;
    error _error;
    constexpr ~_union() noexcept {}
    constexpr _union() noexcept : _none() {}
  } _union;

public:
  const_property<bool, result> has_error = false;

  constexpr ~result() noexcept {
    if (has_error()) _union._error.~error();
  }

  constexpr result(result&& o) noexcept {
    if (o.has_error()) {
      new (&_union._error) yw::error(move(o._union._error));
      has_error = true;
      o.has_error = false;
    }
  }

  template<typename... As> requires constructible<yw::error, As...>
  constexpr result(As&&... as) noexcept(nt_constructible<yw::error, As...>) {
    new (&_union._error) yw::error(static_cast<As&&>(as)...);
    has_error = true;
  }

  explicit constexpr operator bool() const noexcept { return false; }

  template<typename Self> constexpr auto&& error(this Self&& self) noexcept {
    if (!self.has_error()) yw::error("attempted to access error of result when it has no error").print_and_abort();
    return static_cast<copy_cvref<Self&&, yw::error>>(self._union._error);
  }
};
} // namespace yw
