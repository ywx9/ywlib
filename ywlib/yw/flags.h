#pragma once
#include "yw/core.h"

namespace yw {

template<is_enum E> struct alignas(E) flags {
  using enum_type = E;
  using underlying_type = std::underlying_type_t<E>;

  E value{};

  constexpr flags() noexcept = default;
  constexpr flags(const flags&) = default;
  constexpr flags& operator=(const flags&) = default;
  constexpr flags(flags&&) noexcept = default;
  constexpr flags& operator=(flags&&) noexcept = default;

  constexpr explicit flags(E v) noexcept : value(v) {}
  explicit constexpr operator E() const noexcept { return value; }
  constexpr auto to_underlying() const noexcept { return std::to_underlying(value); }

  friend constexpr auto operator|(flags a, E b) noexcept { return flags(E(a.to_underlying() | std::to_underlying(b))); }
  friend constexpr auto operator&(flags a, E b) noexcept { return flags(E(a.to_underlying() & std::to_underlying(b))); }
  friend constexpr auto operator^(flags a, E b) noexcept { return flags(E(a.to_underlying() ^ std::to_underlying(b))); }

  friend constexpr auto operator|(E a, flags b) noexcept { return b | a; }
  friend constexpr auto operator&(E a, flags b) noexcept { return b & a; }
  friend constexpr auto operator^(E a, flags b) noexcept { return b ^ a; }

  friend constexpr flags operator|(flags a, flags b) noexcept { return a | b.value; }
  friend constexpr flags operator&(flags a, flags b) noexcept { return a & b.value; }
  friend constexpr flags operator^(flags a, flags b) noexcept { return a ^ b.value; }
  friend constexpr flags operator~(flags a) noexcept { return flags(E(~a.to_underlying())); }

  constexpr flags& operator|=(E b) noexcept { return (*this = *this | b); }
  constexpr flags& operator&=(E b) noexcept { return (*this = *this & b); }
  constexpr flags& operator^=(E b) noexcept { return (*this = *this ^ b); }

  constexpr flags& operator|=(flags other) noexcept { return *this |= other.value; }
  constexpr flags& operator&=(flags other) noexcept { return *this &= other.value; }
  constexpr flags& operator^=(flags other) noexcept { return *this ^= other.value; }

  constexpr explicit operator bool() const noexcept { return value != E(); }
  friend constexpr bool operator==(flags a, flags b) noexcept = default;
  friend constexpr bool operator==(flags a, E b) noexcept { return a == flags(b); }
  friend constexpr bool operator==(E a, flags b) noexcept { return flags(a) == b; }

  constexpr bool contains(flags other) const noexcept { return (std::to_underlying(value) & std::to_underlying(other.value)) == std::to_underlying(other.value); }
  constexpr bool none() const noexcept { return to_underlying() == 0; }
  constexpr bool any()  const noexcept { return to_underlying() != 0; }
  constexpr bool all(flags other) const noexcept { return contains(other); }

  constexpr flags& set(flags f) noexcept   { return (*this |= f); }
  constexpr flags& reset(flags f) noexcept { return (*this &= ~f); }
  constexpr flags& flip(flags f) noexcept  { return (*this ^= f); }

  constexpr bool contains(E e) const noexcept { return contains(flags{e}); }
  constexpr bool test(E e) const noexcept { return contains(e); }

  constexpr flags& set(E e) noexcept   { return (*this |= e); }
  constexpr flags& reset(E e) noexcept { return (*this &= ~flags{e}); }
  constexpr flags& flip(E e) noexcept  { return (*this ^= e); }

  constexpr void clear() noexcept { value = E(); }
};
}
