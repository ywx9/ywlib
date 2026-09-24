#pragma once
#include <core/core.h>
#include <core/math.h>
#include <core/property.h>
#include <core/tuple.h>

namespace yw {

template<typename T, size_t N> requires arithmetic<T> || is_enum<T> struct vector {
  using value_type = T;
  static constexpr size_t count = N;

  property<T[N], vector> data{};

  static constexpr vector fill(T v) noexcept {
    vector result{};
    std::ranges::fill_n(result.data(), N, v);
    return result;
  }

  constexpr vector() noexcept = default;

  template<typename... Us> requires(lt(sizeof...(Us), N) && (castable_to<Us, T> && ...))
  constexpr vector(Us&&... us) noexcept((nt_castable_to<Us, T> && ...)) : data{static_cast<T>(us)...} {}

  template<tuple_like Tp> requires(lt(extent<Tp>, N) && !castable_to<Tp, T>)
  explicit constexpr vector(Tp&& tp) noexcept(
    nt_vassignable<
  )
};
} // namespace yw
