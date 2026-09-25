#pragma once
#include <core/core.h>
#include <core/math.h>
#include <core/property.h>
#include <core/string.h>
#include <core/tuple.h>

namespace yw {

template<typename T, size_t N> requires arithmetic<T> || is_enum<T> struct vector {
  using value_type = T;
  static constexpr size_t count = N;

  property<T[N], vector> data{};

  static constexpr vector fill(const T& v) {
    vector result{};
    std::ranges::fill_n(result.data(), N, v);
    return result;
  }

  constexpr vector() = default;

  template<typename... Us> requires(sizeof...(Us) <= N && (castable_to<Us, T> && ...))
  explicit(is_enum<T>) constexpr vector(Us&&... us) noexcept((nt_castable_to<Us, T> && ...)) {
    size_t i = 0;
    ((data[i++] = static_cast<T>(us)), ...);
  }

  template<tuple_like Tp> requires(extent<Tp> <= N && vassignable<T (&)[N], Tp>) explicit(
    is_enum<T> || !variation_of<Tp, vector<int, 1>>) constexpr vector(Tp&& tp) noexcept(nt_vassignable<T (&)[N], Tp>) {
    [&]<size_t... Is>(sequence<Is...>) { ((data()[Is] = yw::get<Is>(static_cast<Tp&&>(tp))), ...); }(
      make_indices_for<Tp>());
  }

  static constexpr bool empty() noexcept { return N == 0; }
  static constexpr size_t size() noexcept { return N; }
  constexpr T* begin() noexcept { return data(); }
  constexpr const T* begin() const noexcept { return data(); }
  constexpr T* end() noexcept { return data() + N; }
  constexpr const T* end() const noexcept { return data() + N; }
  constexpr T& operator[](size_t i) noexcept pre(i < N) { return data()[i]; }
  constexpr const T& operator[](size_t i) const noexcept pre(i < N) { return data()[i]; }
  constexpr T& front() noexcept requires(N > 0) { return data()[0]; }
  constexpr const T& front() const noexcept requires(N > 0) { return data()[0]; }
  constexpr T& back() noexcept requires(N > 0) { return data()[N - 1]; }
  constexpr const T& back() const noexcept requires(N > 0) { return data()[N - 1]; }

  constexpr vector operator()() const noexcept { return *this; }

  template<size_t I, typename Self> requires(I < N) constexpr auto&& get(this Self&& self) noexcept {
    return static_cast<copy_cvref_weak<Self&&, T>>(self.data()[I]);
  }

  template<typename Self> requires(N > 0) constexpr auto&& x(this Self&& self) noexcept {
    return static_cast<copy_cvref_weak<Self&&, T>>(self.data()[0]);
  }
  template<typename Self> requires(N > 1) constexpr auto&& y(this Self&& self) noexcept {
    return static_cast<copy_cvref_weak<Self&&, T>>(self.data()[1]);
  }
  template<typename Self> requires(N > 2) constexpr auto&& z(this Self&& self) noexcept {
    return static_cast<copy_cvref_weak<Self&&, T>>(self.data()[2]);
  }
  template<typename Self> requires(N > 3) constexpr auto&& w(this Self&& self) noexcept {
    return static_cast<copy_cvref_weak<Self&&, T>>(self.data()[3]);
  }

  constexpr vector<T, 2> xy() const noexcept requires(N > 1) { return {x(), y()}; }
  constexpr vector<T, 3> xyz() const noexcept requires(N > 2) { return {x(), y(), z()}; }
  constexpr vector<T, 4> xyzw() const noexcept requires(N > 3) { return {x(), y(), z(), w()}; }

  constexpr string<char> to_string() const { return to_string<char>(); }
  template<char_type C> constexpr string<C> to_string() const {
    if constexpr (N == 0) return string<C>("()");
    else {
      string<C> s;
      s.reserve((8 * sizeof(T) + 2) * N);
      s.push_back('(');
      s.append(vtos<C>(data()[0]));
      for (size_t i = 1; i < N; ++i) {
        s.push_back(',');
        s.append(vtos<C>(data()[i]));
      }
      s.push_back(')');
      return s;
    }
  }

  constexpr auto squared_length() const noexcept {
    using RT = decltype(T() * T());
    RT sum{};
    for (size_t i = 0; i < N; ++i) sum += data()[i] * data()[i];
    return sum;
  }
  constexpr auto length() const noexcept { return yw::sqrt(squared_length()); }
  constexpr vector<math_type<T>, N> normalized() const noexcept {
    const auto len = math_type<T>(length());
    if (len == 0) return *this;
    vector<math_type<T>, N> result;
    for (size_t i = 0; i < N; ++i) result[i] = data()[i] / len;
    return result;
  }
};

template<typename... Us> requires(sizeof...(Us) > 0)
vector(Us&&... us) -> vector<remove_cvref<Us...[0]>, sizeof...(Us)>;
template<tuple_like Tp> vector(Tp&&) -> vector<remove_cvref<element_t<Tp, 0>>, extent<Tp>>;

template<typename T, typename U, size_t N> requires requires(T a, U b) { a == b; }
constexpr bool operator==(const vector<T, N>& a, const vector<U, N>& b) noexcept {
  for (size_t i = 0; i < N; ++i)
    if (!(a[i] == b[i])) return false;
  return true;
}
template<typename T, typename U, size_t N> requires requires(T a, U b) { a <=> b; }
constexpr auto operator<=>(const vector<T, N>& a, const vector<U, N>& b) noexcept {
  for (size_t i = 0; i < N; ++i)
    if (const auto c = a[i] <=> b[i]; c != 0) return c;
  return std::compare_three_way_result_t<T, U>::equivalent;
}

template<typename T, size_t N> constexpr auto operator+(const vector<T, N>& a) noexcept { return a; }
template<typename T, size_t N> requires requires(T a) {
  { -a } -> convertible_to<T>;
} constexpr auto operator-(const vector<T, N>& a) noexcept {
  vector<T, N> result;
  for (size_t i = 0; i < N; ++i) result[i] = -a[i];
  return result;
}

template<typename T, size_t N> requires requires(T a) { ~a; } constexpr auto operator~(const vector<T, N>& a) noexcept {
  vector<decltype(~T()), N> result;
  for (size_t i = 0; i < N; ++i) result[i] = ~a[i];
  return result;
}

template<typename T, size_t N> requires requires(T a) { ++a; } constexpr auto operator++(vector<T, N>& a) noexcept {
  for (size_t i = 0; i < N; ++i) ++a[i];
  return a;
}

template<typename T, size_t N> requires requires(T a) { --a; } constexpr auto operator--(vector<T, N>& a) noexcept {
  for (size_t i = 0; i < N; ++i) --a[i];
  return a;
}

template<typename T, size_t N> requires requires(T a) { a++; }
constexpr auto operator++(vector<T, N>& a, int) noexcept {
  vector<T, N> result = a;
  for (size_t i = 0; i < N; ++i) a[i]++;
  return result;
}

template<typename T, size_t N> requires requires(T a) { a--; }
constexpr auto operator--(vector<T, N>& a, int) noexcept {
  vector<T, N> result = a;
  for (size_t i = 0; i < N; ++i) a[i]--;
  return result;
}

template<typename T, typename U, size_t N> requires requires(T a, U b) { a + b; }
constexpr auto operator+(const vector<T, N>& a, const vector<U, N>& b) noexcept {
  vector<decltype(T() + U()), N> result;
  for (size_t i = 0; i < N; ++i) result[i] = a[i] + b[i];
  return result;
}

template<typename T, typename U, size_t N> requires requires(T a, U b) { a - b; }
constexpr auto operator-(const vector<T, N>& a, const vector<U, N>& b) noexcept {
  vector<decltype(T() - U()), N> result;
  for (size_t i = 0; i < N; ++i) result[i] = a[i] - b[i];
  return result;
}

template<typename T, typename U, size_t N> requires requires(T a, U b) { a * b; }
constexpr auto operator*(const vector<T, N>& a, const vector<U, N>& b) noexcept {
  vector<decltype(T() * U()), N> result;
  for (size_t i = 0; i < N; ++i) result[i] = a[i] * b[i];
  return result;
}

template<typename T, typename U, size_t N> requires((arithmetic<U> || is_enum<U>) && requires(T a, U b) { a * b; })
constexpr auto operator*(const vector<T, N>& a, U b) noexcept {
  vector<decltype(T() * U()), N> result;
  for (size_t i = 0; i < N; ++i) result[i] = a[i] * b;
  return result;
}

template<typename T, typename U, size_t N> requires((arithmetic<T> || is_enum<T>) && requires(T a, U b) { a * b; })
constexpr auto operator*(T a, const vector<U, N>& b) noexcept {
  vector<decltype(T() * U()), N> result;
  for (size_t i = 0; i < N; ++i) result[i] = a * b[i];
  return result;
}

template<typename T, typename U, size_t N> requires requires(T a, U b) { a / b; }
constexpr auto operator/(const vector<T, N>& a, const vector<U, N>& b) noexcept {
  vector<decltype(T() / U()), N> result;
  for (size_t i = 0; i < N; ++i) result[i] = a[i] / b[i];
  return result;
}

template<typename T, typename U, size_t N> requires((arithmetic<U> || is_enum<U>) && requires(T a, U b) { a / b; })
constexpr auto operator/(const vector<T, N>& a, U b) noexcept {
  vector<decltype(T() / U()), N> result;
  for (size_t i = 0; i < N; ++i) result[i] = a[i] / b;
  return result;
}

template<typename T, typename U, size_t N> requires((arithmetic<T> || is_enum<T>) && requires(T a, U b) { a / b; })
constexpr auto operator/(T a, const vector<U, N>& b) noexcept {
  vector<decltype(T() / U()), N> result;
  for (size_t i = 0; i < N; ++i) result[i] = a / b[i];
  return result;
}

template<typename T, typename U, size_t N> requires requires(T a, U b) { a % b; }
constexpr auto operator%(const vector<T, N>& a, const vector<U, N>& b) noexcept {
  vector<decltype(T() % U()), N> result;
  for (size_t i = 0; i < N; ++i) result[i] = a[i] % b[i];
  return result;
}

template<typename T, typename U, size_t N> requires requires(T a, U b) { a & b; }
constexpr auto operator&(const vector<T, N>& a, const vector<U, N>& b) noexcept {
  vector<decltype(T() & U()), N> result;
  for (size_t i = 0; i < N; ++i) result[i] = a[i] & b[i];
  return result;
}

template<typename T, typename U, size_t N> requires requires(T a, U b) { a | b; }
constexpr auto operator|(const vector<T, N>& a, const vector<U, N>& b) noexcept {
  vector<decltype(T() | U()), N> result;
  for (size_t i = 0; i < N; ++i) result[i] = a[i] | b[i];
  return result;
}

template<typename T, typename U, size_t N> requires requires(T a, U b) { a ^ b; }
constexpr auto operator^(const vector<T, N>& a, const vector<U, N>& b) noexcept {
  vector<decltype(T() ^ U()), N> result;
  for (size_t i = 0; i < N; ++i) result[i] = a[i] ^ b[i];
  return result;
}

template<typename T, typename U, size_t N> requires requires(T a, U b) { a << b; }
constexpr auto operator<<(const vector<T, N>& a, const vector<U, N>& b) noexcept {
  vector<decltype(T() << U()), N> result;
  for (size_t i = 0; i < N; ++i) result[i] = a[i] << b[i];
  return result;
}

template<typename T, typename U, size_t N> requires((arithmetic<U> || is_enum<U>) && requires(T a, U b) { a << b; })
constexpr auto operator<<(const vector<T, N>& a, U b) noexcept {
  vector<decltype(T() << U()), N> result;
  for (size_t i = 0; i < N; ++i) result[i] = a[i] << b;
  return result;
}

template<typename T, typename U, size_t N> requires requires(T a, U b) { a >> b; }
constexpr auto operator>>(const vector<T, N>& a, const vector<U, N>& b) noexcept {
  vector<decltype(T() >> U()), N> result;
  for (size_t i = 0; i < N; ++i) result[i] = a[i] >> b[i];
  return result;
}

template<typename T, typename U, size_t N> requires((arithmetic<U> || is_enum<U>) && requires(T a, U b) { a >> b; })
constexpr auto operator>>(const vector<T, N>& a, U b) noexcept {
  vector<decltype(T() >> U()), N> result;
  for (size_t i = 0; i < N; ++i) result[i] = a[i] >> b;
  return result;
}

template<typename T, typename U, size_t N> requires requires(T& a, U b) { a += b; }
constexpr vector<T, N>& operator+=(vector<T, N>& a, const vector<U, N>& b) noexcept {
  for (size_t i = 0; i < N; ++i) a[i] += b[i];
  return a;
}

template<typename T, typename U, size_t N> requires requires(T& a, U b) { a -= b; }
constexpr vector<T, N>& operator-=(vector<T, N>& a, const vector<U, N>& b) noexcept {
  for (size_t i = 0; i < N; ++i) a[i] -= b[i];
  return a;
}

template<typename T, typename U, size_t N> requires requires(T& a, U b) { a *= b; }
constexpr vector<T, N>& operator*=(vector<T, N>& a, const vector<U, N>& b) noexcept {
  for (size_t i = 0; i < N; ++i) a[i] *= b[i];
  return a;
}

template<typename T, typename U, size_t N> requires((arithmetic<U> || is_enum<U>) && requires(T& a, U b) { a *= b; })
constexpr vector<T, N>& operator*=(vector<T, N>& a, U b) noexcept {
  for (size_t i = 0; i < N; ++i) a[i] *= b;
  return a;
}

template<typename T, typename U, size_t N> requires requires(T& a, U b) { a /= b; }
constexpr vector<T, N>& operator/=(vector<T, N>& a, const vector<U, N>& b) noexcept {
  for (size_t i = 0; i < N; ++i) a[i] /= b[i];
  return a;
}

template<typename T, typename U, size_t N> requires((arithmetic<U> || is_enum<U>) && requires(T& a, U b) { a /= b; })
constexpr vector<T, N>& operator/=(vector<T, N>& a, U b) noexcept {
  for (size_t i = 0; i < N; ++i) a[i] /= b;
  return a;
}

template<typename T, typename U, size_t N> requires requires(T& a, U b) { a %= b; }
constexpr vector<T, N>& operator%=(vector<T, N>& a, const vector<U, N>& b) noexcept {
  for (size_t i = 0; i < N; ++i) a[i] %= b[i];
  return a;
}

template<typename T, typename U, size_t N> requires requires(T& a, U b) { a &= b; }
constexpr vector<T, N>& operator&=(vector<T, N>& a, const vector<U, N>& b) noexcept {
  for (size_t i = 0; i < N; ++i) a[i] &= b[i];
  return a;
}

template<typename T, typename U, size_t N> requires requires(T& a, U b) { a |= b; }
constexpr vector<T, N>& operator|=(vector<T, N>& a, const vector<U, N>& b) noexcept {
  for (size_t i = 0; i < N; ++i) a[i] |= b[i];
  return a;
}

template<typename T, typename U, size_t N> requires requires(T& a, U b) { a ^= b; }
constexpr vector<T, N>& operator^=(vector<T, N>& a, const vector<U, N>& b) noexcept {
  for (size_t i = 0; i < N; ++i) a[i] ^= b[i];
  return a;
}

template<typename T, typename U, size_t N> requires requires(T& a, U b) { a <<= b; }
constexpr vector<T, N>& operator<<=(vector<T, N>& a, const vector<U, N>& b) noexcept {
  for (size_t i = 0; i < N; ++i) a[i] <<= b[i];
  return a;
}

template<typename T, typename U, size_t N> requires((arithmetic<U> || is_enum<U>) && requires(T& a, U b) { a <<= b; })
constexpr vector<T, N>& operator<<=(vector<T, N>& a, U b) noexcept {
  for (size_t i = 0; i < N; ++i) a[i] <<= b;
  return a;
}

template<typename T, typename U, size_t N> requires requires(T& a, U b) { a >>= b; }
constexpr vector<T, N>& operator>>=(vector<T, N>& a, const vector<U, N>& b) noexcept {
  for (size_t i = 0; i < N; ++i) a[i] >>= b[i];
  return a;
}

template<typename T, typename U, size_t N> requires((arithmetic<U> || is_enum<U>) && requires(T& a, U b) { a >>= b; })
constexpr vector<T, N>& operator>>=(vector<T, N>& a, U b) noexcept {
  for (size_t i = 0; i < N; ++i) a[i] >>= b;
  return a;
}

template<typename T, typename U, size_t N> requires requires(T a, U b) { a * b; }
constexpr auto dot(const vector<T, N>& a, const vector<U, N>& b) noexcept {
  using R = decltype(T() * U());
  R result{};
  for (size_t i = 0; i < N; ++i) result += a[i] * b[i];
  return result;
}

template<typename T, typename U, size_t M, size_t N> requires requires(T a, U b) { a * b; } && (M <= 4 && N <= 4)
constexpr auto cross(const vector<T, M>& a, const vector<U, N>& b) noexcept {
  using R = decltype(yw::min(T(), U(), int8_t(-1)));
  constexpr size_t L = yw::min(yw::max(M, N, 3u), 4u);
  vector<R, L> result{};
  if constexpr (M == 0 || N == 0 || (M == 1 && N == 1)) return result;
  if constexpr (M == 2 && N >= 3) result[0] = R(a[1] * b[2]);
  else if constexpr (M >= 3 && N == 2) result[0] = -R(a[2] * b[1]);
  else if constexpr (M >= 3 && N >= 3) result[0] = R(a[1] * b[2]) - R(a[2] * b[1]);
  if constexpr (M >= 3 && N <= 2) result[1] = R(a[2] * b[0]);
  else if constexpr (M <= 2 && N >= 3) result[1] = -R(a[0] * b[2]);
  else if constexpr (M >= 3 && N >= 3) result[1] = R(a[2] * b[0]) - R(a[0] * b[2]);
  if constexpr (M == 1 && N >= 2) result[2] = R(a[0] * b[1]);
  else if constexpr (M >= 2 && N == 1) result[2] = -R(a[1] * b[0]);
  else if constexpr (M >= 2 && N >= 2) result[2] = R(a[0] * b[1]) - R(a[1] * b[0]);
  return result;
}
} // namespace yw
