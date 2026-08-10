#pragma once
#include <yw/core.h>

#include <array>
#include <limits>
#include <type_traits>

namespace yw::random {

template<typename T> concept value_type = int_type<T> || uint_type<T> || float_type<T>;

class generator {
public:
  using result_type = uint64_t;

private:
  static constexpr size_t state_size = 312;
  static constexpr size_t middle_word = 156;
  static constexpr uint64_t matrix_a = 0xb5026f5aa96619e9ull;
  static constexpr uint64_t upper_mask = 0xffffffff80000000ull;
  static constexpr uint64_t lower_mask = 0x7fffffffull;

  std::array<uint64_t, state_size> state{};
  size_t index = state_size;

  void twist() noexcept {
    for (size_t i{}; i < state_size; ++i) {
      const auto x = (state[i] & upper_mask) | (state[(i + 1) % state_size] & lower_mask);
      auto xa = x >> 1;
      if (x & 1) xa ^= matrix_a;
      state[i] = state[(i + middle_word) % state_size] ^ xa;
    }
    index = 0;
  }

  template<uint_type T> T uint() noexcept {
    using limits = std::numeric_limits<T>;
    if constexpr (limits::digits >= 64) return static_cast<T>((*this)());
    else return static_cast<T>((*this)() & static_cast<uint64_t>(limits::max()));
  }

  template<uint_type T> T uint(T Min, T Max) noexcept {
    if (Max < Min) {
      const auto temp = Min;
      Min = Max;
      Max = temp;
    }

    using limits = std::numeric_limits<T>;
    const auto range = static_cast<T>(Max - Min + T(1));
    if (range == T(0)) return static_cast<T>(Min + uint<T>());

    const auto limit = static_cast<T>(limits::max() - limits::max() % range);
    T value{};
    do value = uint<T>();
    while (value >= limit);
    return static_cast<T>(Min + value % range);
  }

  template<int_type T> T int_(T Min, T Max) noexcept {
    if (Max < Min) {
      const auto temp = Min;
      Min = Max;
      Max = temp;
    }

    using uint_t = std::make_unsigned_t<T>;
    using limits = std::numeric_limits<uint_t>;
    const auto range = static_cast<uint_t>(static_cast<uint_t>(Max) - static_cast<uint_t>(Min) + uint_t(1));
    if (range == uint_t(0)) return static_cast<T>(uint<uint_t>());

    const auto limit = static_cast<uint_t>(limits::max() - limits::max() % range);
    uint_t value{};
    do value = uint<uint_t>();
    while (value >= limit);
    return static_cast<T>(static_cast<uint_t>(Min) + value % range);
  }

  template<float_type T> T real() noexcept {
    if constexpr (same_as<T, float>) return static_cast<T>((*this)() >> 40) * static_cast<T>(0x1.0p-24f);
    else if constexpr (same_as<T, double>) return static_cast<T>((*this)() >> 11) * static_cast<T>(0x1.0p-53);
    else return static_cast<T>((*this)()) * static_cast<T>(1.0L / 18446744073709551616.0L);
  }

public:
  generator(uint64_t Seed = 5489ull) noexcept { seed(Seed); }

  void seed(uint64_t Seed) noexcept {
    state[0] = Seed;
    for (size_t i{1}; i < state_size; ++i)
      state[i] = 6364136223846793005ull * (state[i - 1] ^ (state[i - 1] >> 62)) + i;
    index = state_size;
  }

  uint64_t operator()() noexcept {
    if (index >= state_size) twist();

    auto y = state[index++];
    y ^= (y >> 29) & 0x5555555555555555ull;
    y ^= (y << 17) & 0x71d67fffeda60000ull;
    y ^= (y << 37) & 0xfff7eee000000000ull;
    y ^= y >> 43;
    return y;
  }

  template<value_type T> T gen() noexcept {
    if constexpr (uint_type<T>) return uint<T>();
    else if constexpr (int_type<T>) return static_cast<T>(uint<std::make_unsigned_t<T>>());
    else return real<T>();
  }

  template<value_type T> T gen(T Max) noexcept {
    if constexpr (float_type<T>) return gen<T>(T(0), Max);
    else return gen<T>(T(0), Max);
  }

  template<value_type T> T gen(T Min, T Max) noexcept {
    if constexpr (uint_type<T>) return uint(Min, Max);
    else if constexpr (int_type<T>) return int_(Min, Max);
    else {
      if (Max < Min) {
        const auto temp = Min;
        Min = Max;
        Max = temp;
      }
      return Min + real<T>() * (Max - Min);
    }
  }
};

inline generator default_generator{};

template<value_type T> T gen() noexcept { return default_generator.gen<T>(); }
template<value_type T> T gen(T Max) noexcept { return default_generator.gen<T>(Max); }
template<value_type T> T gen(T Min, T Max) noexcept { return default_generator.gen<T>(Min, Max); }

} // namespace yw::random
