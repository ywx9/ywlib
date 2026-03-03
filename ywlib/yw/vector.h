#pragma once
#include "yw/tuple.h"

namespace yw {

template<std::regular T, size_t N> struct vector {
  using value_type = T;
  static constexpr auto count{N};
  T _vals[N];

  static constexpr vector fill(const T& value) {
    vector v;
    std::ranges::fill(v._vals, value);
    return v;
  }

  constexpr vector() noexcept(std::is_nothrow_default_constructible_v<T>) = default;

  template<typename... Us> requires(sizeof...(Us) == N && (castable_to<Us, T> && ...))
  explicit(!(convertible_to<Us, T> && ...)) constexpr vector(Us&&... as) noexcept(
    (noexcept(static_cast<T>(std::declval<Us>())) && ...))
    : _vals{static_cast<T>(static_cast<Us&&>(as))...} {}

  template<tuple_like Tp>
  requires(lt(extent<Tp>, N) && !castable_to<Tp, T> && !same_as<remove_cvref<Tp>, vector>)
  constexpr vector(Tp&& tp) {
    constexpr auto seq = make_sequence<0, extent<Tp>>{};
    [&]<size_t... Is>(sequence<Is...>) {
      ((_vals[Is] = T(yw::get<Is>(static_cast<Tp&&>(tp)))), ...);
    }(seq);
  }

  static constexpr bool empty() noexcept { return N == 0; }
  static constexpr size_t size() noexcept { return N; }

  /// \note following functions (begin, end, data) are not constexpr, because of consistency with
  /// the specializations.

  auto* begin() noexcept { return _vals; }
  const auto* begin() const noexcept { return _vals; }
  auto* end() noexcept { return _vals + N; }
  const auto* end() const noexcept { return _vals + N; }
  auto* data() noexcept { return _vals; }
  const auto* data() const noexcept { return _vals; }

  constexpr auto& front() noexcept { return _vals[0]; }
  constexpr const auto& front() const noexcept { return _vals[0]; }
  constexpr auto& back() noexcept { return _vals[N - 1]; }
  constexpr const auto& back() const noexcept { return _vals[N - 1]; }
  constexpr auto& operator[](integral auto i) noexcept { return _vals[size_t((i % N) + N) % N]; }
  constexpr const auto& operator[](integral auto i) const noexcept {
    return _vals[size_t((i % N) + N) % N];
  }

  template<size_t I> requires(I < N)
  constexpr auto& get() & {
    return _vals[I];
  }
  template<size_t I> requires(I < N)
  constexpr const auto& get() const& {
    return _vals[I];
  }
  template<size_t I> requires(I < N)
  constexpr auto&& get() && {
    return std::move(_vals[I]);
  }
  template<size_t I> requires(I < N)
  constexpr const auto&& get() const&& {
    return std::move(_vals[I]);
  }

  constexpr vector operator()() const noexcept { return *this; }
};

template<typename T, typename... Ts> vector(T&&, Ts&&...) -> vector<T, sizeof...(Ts) + 1>;
template<tuple_like Tp> vector(Tp&&) -> vector<element_t<Tp, 0>, extent<Tp>>;

template<std::regular T> using vector1 = vector<T, 1>;
template<std::regular T> using vector2 = vector<T, 2>;
template<std::regular T> using vector3 = vector<T, 3>;
template<std::regular T> using vector4 = vector<T, 4>;

using short1 = vector1<short>;
using short2 = vector2<short>;
using short3 = vector3<short>;
using short4 = vector4<short>;
using int1 = vector1<int>;
using int2 = vector2<int>;
using int3 = vector3<int>;
using int4 = vector4<int>;
using uint1 = vector1<uint32_t>;
using uint2 = vector2<uint32_t>;
using uint3 = vector3<uint32_t>;
using uint4 = vector4<uint32_t>;
using float1 = vector1<float>;
using float2 = vector2<float>;
using float3 = vector3<float>;
using float4 = vector4<float>;
using double1 = vector1<double>;
using double2 = vector2<double>;
using double3 = vector3<double>;
using double4 = vector4<double>;

template<typename T, std::equality_comparable_with<T> U, size_t N>
constexpr bool operator==(const vector<T, N>& a, const vector<U, N>& b) {
  constexpr auto seq = make_sequence<0, N>{};
  return
    [&]<size_t... Is>(sequence<Is...>) -> bool { return ((get<Is>(a) == get<Is>(b)) && ...); }(seq);
}

template<typename T, std::three_way_comparable_with<T> U, size_t N>
constexpr std::partial_ordering operator<=>(const vector<T, N>& a, const vector<U, N>& b) {
  for (size_t i = 0; i < N; ++i)
    if (auto cmp = a[i] <=> b[i]; cmp != 0) return cmp;
  return std::partial_ordering::equivalent;
}

template<typename T, size_t N> constexpr vector<T, N> operator+(const vector<T, N>& a) {
  return a;
}
template<typename T, size_t N> constexpr vector<T, N> operator-(const vector<T, N>& a) {
  return [&]<size_t... Is>(sequence<Is...>) -> vector<T, N> {
    return {(-get<Is>(a))...};
  }(make_sequence<0, N>{});
}

template<typename T, typename U, size_t N>
constexpr vector<decltype(T{} + U{}), N> operator+(const vector<T, N>& a, const vector<U, N>& b) {
  return [&]<size_t... Is>(sequence<Is...>) -> vector<decltype(T{} + U{}), N> {
    return {(get<Is>(a) + get<Is>(b))...};
  }(make_sequence<0, N>{});
}
template<typename T, typename U, size_t N>
constexpr vector<decltype(T{} - U{}), N> operator-(const vector<T, N>& a, const vector<U, N>& b) {
  return [&]<size_t... Is>(sequence<Is...>) -> vector<decltype(T{} - U{}), N> {
    return {(get<Is>(a) - get<Is>(b))...};
  }(make_sequence<0, N>{});
}
template<typename T, typename U, size_t N>
constexpr vector<decltype(T{} * U{}), N> operator*(const vector<T, N>& a, const vector<U, N>& b) {
  return [&]<size_t... Is>(sequence<Is...>) -> vector<decltype(T{} * U{}), N> {
    return {(get<Is>(a) * get<Is>(b))...};
  }(make_sequence<0, N>{});
}
template<typename T, typename U, size_t N>
constexpr vector<decltype(T{} / U{}), N> operator/(const vector<T, N>& a, const vector<U, N>& b) {
  return [&]<size_t... Is>(sequence<Is...>) -> vector<decltype(T{} / U{}), N> {
    return {(get<Is>(a) / get<Is>(b))...};
  }(make_sequence<0, N>{});
}

template<typename T, typename U, size_t N>
constexpr vector<decltype(T{} * U{}), N> operator*(const vector<T, N>& a, const U & b) {
  using RT = decltype(T{} * U{});
  const auto bb = static_cast<RT>(b);
  return [&]<size_t... Is>(sequence<Is...>) -> vector<RT, N> {
    return {(get<Is>(a) * bb)...};
  }(make_sequence<0, N>{});
}
template<typename T, typename U, size_t N>
constexpr vector<decltype(T{} / U{}), N> operator/(const vector<T, N>& a, const U & b) {
  using RT = decltype(T{} / U{});
  const auto bb = static_cast<RT>(b);
  return [&]<size_t... Is>(sequence<Is...>) -> vector<RT, N> {
    return {(get<Is>(a) / bb)...};
  }(make_sequence<0, N>{});
}
template<typename T, typename U, size_t N>
constexpr vector<decltype(U{} * T{}), N> operator*(const U & a, const vector<T, N>& b) {
  using RT = decltype(U{} * T{});
  const auto aa = static_cast<RT>(a);
  return [&]<size_t... Is>(sequence<Is...>) -> vector<RT, N> {
    return {(aa * get<Is>(b))...};
  }(make_sequence<0, N>{});
}

template<typename T, typename U, size_t N> constexpr auto mul(const vector<T, N>& a, const U& b) {
  return a * b;
}
template<typename T, typename U, size_t N> constexpr auto mul(const U& a, const vector<T, N>& b) {
  return a * b;
}
template<typename T, typename U, size_t N>
constexpr auto mul(const vector<T, N>& a, const vector<U, N>& b) {
  return [&]<size_t... Is>(sequence<Is...>) {
    return ((get<Is>(a) * get<Is>(b)) + ...);
  }(make_sequence<0, N>{});
}

template<typename T, size_t M, typename U, size_t N>
constexpr vector<decltype(T{} + U{}), 3> cross(const vector<T, M>& a, const vector<U, N>& b) {
  vector<decltype(T{} + U{}), 3> result;
  if constexpr (M > 1 && N > 2) result[0] = a[1] * b[2];
  if constexpr (M > 2 && N > 1) result[0] -= a[2] * b[1];
  if constexpr (M > 2 && N > 0) result[1] = a[2] * b[0];
  if constexpr (M > 0 && N > 2) result[1] -= a[0] * b[2];
  if constexpr (M > 0 && N > 1) result[2] = a[0] * b[1];
  if constexpr (M > 1 && N > 0) result[2] -= a[1] * b[0];
  return result;
}

//////////////////////////////////////// MARK: VECTOR1

template<std::regular T> struct vector<T, 1> {
  using value_type = T;
  static constexpr size_t count{1};
  T x{};

  static constexpr vector fill(const T& value) { return {value}; }

  constexpr vector() noexcept = default;

  template<castable_to<T> Xt> constexpr vector(Xt&& X) noexcept(nt_castable_to<Xt, T>)
    : x(T(static_cast<Xt&&>(X))) {}

  template<tuple_like<1> Tp> requires(!castable_to<Tp, T> && !same_as<remove_cvref<Tp>, vector>)
  constexpr vector(Tp&& tp) noexcept : x(T(yw::get<0>(static_cast<Tp&&>(tp)))) {}

  static constexpr bool empty() noexcept { return false; }
  static constexpr size_t size() noexcept { return 1; }

  T* begin() noexcept { return &x; }
  const T* begin() const noexcept { return &x; }
  T* end() noexcept { return &x + 1; }
  const T* end() const noexcept { return &x + 1; }
  T* data() noexcept { return &x; }
  const T* data() const noexcept { return &x; }

  constexpr T& front() noexcept { return x; }
  constexpr const T& front() const noexcept { return x; }
  constexpr T& back() noexcept { return x; }
  constexpr const T& back() const noexcept { return x; }
  constexpr T& operator[](integral auto) noexcept { return x; }
  constexpr const T& operator[](integral auto) const noexcept { return x; }

  template<size_t I> requires(I == 0)
  constexpr T& get() & noexcept {
    return x;
  }
  template<size_t I> requires(I == 0)
  constexpr T&& get() && noexcept {
    return std::move(x);
  }
  template<size_t I> requires(I == 0)
  constexpr const T& get() const& noexcept {
    return x;
  }
  template<size_t I> requires(I == 0)
  constexpr const T&& get() const&& noexcept {
    return std::move(x);
  }

  constexpr vector operator()() const noexcept { return *this; }
};

//////////////////////////////////////// MARK: VECTOR2

template<std::regular T> struct vector<T, 2> {
  using value_type = T;
  static constexpr size_t count{2};
  T x{}, y{};

  static constexpr vector fill(const T& value) { return {value, value}; }

  constexpr vector() noexcept = default;

  template<castable_to<T> Xt, castable_to<T> Yt>
  constexpr vector(Xt&& X, Yt&& Y) noexcept(nt_castable_to<Xt, T> && nt_castable_to<Yt, T>)
    : x(T(static_cast<Xt&&>(X))), y(T(static_cast<Yt&&>(Y))) {}

  template<tuple_like<1> Tp> requires(!castable_to<Tp, T>)
  constexpr vector(Tp&& tp) : x(T(yw::get<0>(static_cast<Tp&&>(tp)))), y() {}

  template<tuple_like<2> Tp> requires(!castable_to<Tp, T> && !same_as<remove_cvref<Tp>, vector>)
  constexpr vector(Tp&& tp)
    : x(T(yw::get<0>(static_cast<Tp&&>(tp)))), y(T(yw::get<1>(static_cast<Tp&&>(tp)))) {}

  constexpr bool empty() const noexcept { return false; }
  constexpr size_t size() const noexcept { return 2; }

  T* begin() noexcept { return &x; }
  const T* begin() const noexcept { return &x; }
  T* end() noexcept { return &x + 2; }
  const T* end() const noexcept { return &x + 2; }
  T* data() noexcept { return &x; }
  const T* data() const noexcept { return &x; }

  constexpr T& front() noexcept { return x; }
  constexpr const T& front() const noexcept { return x; }
  constexpr T& back() noexcept { return y; }
  constexpr const T& back() const noexcept { return y; }

  constexpr T& operator[](integral auto i) noexcept {
    const auto ii = ((i % 2) + 2) % 2;
    if (!std::is_constant_evaluated()) return *(&x + ii);
    return ii == 0 ? x : y;
  }
  const T& operator[](integral auto i) const noexcept {
    const auto ii = ((i % 2) + 2) % 2;
    if (!std::is_constant_evaluated()) return *(&x + ii);
    return ii == 0 ? x : y;
  }

  template<size_t I> requires(I < 2) constexpr T& get() & noexcept { return select<I>(x, y); }
  template<size_t I> requires(I < 2) constexpr T&& get() && noexcept { return std::move(select<I>(x, y)); }
  template<size_t I> requires(I < 2) constexpr const T& get() const& noexcept { return select<I>(x, y); }
  template<size_t I> requires(I < 2) constexpr const T&& get() const&& noexcept { return std::move(select<I>(x, y)); }

  constexpr vector operator()() const noexcept { return *this; }
};

//////////////////////////////////////// MARK: VECTOR3

template<std::regular T> struct vector<T, 3> {
  using value_type = T;
  static constexpr size_t count{3};
  T x{}, y{}, z{};

  static constexpr vector fill(const T& value) { return {value, value, value}; }

  constexpr vector() noexcept = default;

  template<castable_to<T> Xt, castable_to<T> Yt, castable_to<T> Zt>
  constexpr vector(Xt&& X, Yt&& Y, Zt&& Z) noexcept(nt_castable_to<Xt, T> &&
                                                    nt_castable_to<Yt, T> && nt_castable_to<Zt, T>)
    : x(T(static_cast<Xt&&>(X))), y(T(static_cast<Yt&&>(Y))), z(T(static_cast<Zt&&>(Z))) {}

  template<tuple_like<1> Tp> requires(!castable_to<Tp, T>)
  constexpr vector(Tp&& tp) : x(T(yw::get<0>(static_cast<Tp&&>(tp)))), y(), z() {}

  template<tuple_like<2> Tp> requires(!castable_to<Tp, T>)
  constexpr vector(Tp&& tp)
    : x(T(yw::get<0>(static_cast<Tp&&>(tp)))), y(T(yw::get<1>(static_cast<Tp&&>(tp)))), z() {}

  template<tuple_like<3> Tp> requires(!castable_to<Tp, T> && !same_as<remove_cvref<Tp>, vector>)
  constexpr vector(Tp&& tp)
    : x(T(yw::get<0>(static_cast<Tp&&>(tp)))), y(T(yw::get<1>(static_cast<Tp&&>(tp)))),
      z(T(yw::get<2>(static_cast<Tp&&>(tp)))) {}

  constexpr bool empty() const noexcept { return false; }
  constexpr size_t size() const noexcept { return count; }

  T* data() noexcept { return &x; }
  const T* data() const noexcept { return &x; }
  T* begin() noexcept { return &x; }
  const T* begin() const noexcept { return &x; }
  T* end() noexcept { return &x + count; }
  const T* end() const noexcept { return &x + count; }

  constexpr T& front() noexcept { return x; }
  constexpr const T& front() const noexcept { return x; }
  constexpr T& back() noexcept { return z; }
  constexpr const T& back() const noexcept { return z; }

  constexpr T& operator[](integral auto i) noexcept {
    const auto ii = size_t((i % 3) + 3) % 3;
    if (!std::is_constant_evaluated()) return *(&x + ii);
    return ii == 0 ? x : (ii == 1 ? y : z);
  }
  constexpr const T& operator[](integral auto i) const noexcept {
    const auto ii = ((i % 3) + 3) % 3;
    if (!std::is_constant_evaluated()) return *(&x + ii);
    return ii == 0 ? x : (ii == 1 ? y : z);
  }

  template<size_t I> requires(I < 3)
  constexpr T& get() & noexcept {
    return select<I>(x, y, z);
  }
  template<size_t I> requires(I < 3)
  constexpr T&& get() && noexcept {
    return std::move(select<I>(x, y, z));
  }
  template<size_t I> requires(I < 3)
  constexpr const T& get() const& noexcept {
    return select<I>(x, y, z);
  }
  template<size_t I> requires(I < 3)
  constexpr const T&& get() const&& noexcept {
    return std::move(select<I>(x, y, z));
  }

  constexpr vector operator()() const noexcept { return *this; }
};

//////////////////////////////////////// MARK: VECTOR4

template<std::regular T> struct vector<T, 4> {
  using value_type = T;
  static constexpr size_t count{4};
  T x{}, y{}, z{}, w{};

  static constexpr vector fill(const T& value) { return {value, value, value, value}; }

  constexpr vector() noexcept = default;

  template<castable_to<T> Xt> constexpr vector(Xt&& X) noexcept(nt_castable_to<Xt, T>)
    : x(T(static_cast<Xt&&>(X))), y(), z(), w() {}

  template<castable_to<T> Xt, castable_to<T> Yt, castable_to<T> Zt, castable_to<T> Wt>
  constexpr vector(Xt&& X, Yt&& Y, Zt&& Z,
                   Wt&& W) noexcept(nt_castable_to<Xt, T> && nt_castable_to<Yt, T> &&
                                    nt_castable_to<Zt, T> && nt_castable_to<Wt, T>)
    : x(T(static_cast<Xt&&>(X))), y(T(static_cast<Yt&&>(Y))), z(T(static_cast<Zt&&>(Z))),
      w(T(static_cast<Wt&&>(W))) {}

  template<tuple_like<1> Tp> requires(!castable_to<Tp, T>)
  constexpr vector(Tp&& tp) : x(T(yw::get<0>(static_cast<Tp&&>(tp)))), y(), z(), w() {}

  template<tuple_like<2> Tp> requires(!castable_to<Tp, T>)
  constexpr vector(Tp&& tp)
    : x(T(yw::get<0>(static_cast<Tp&&>(tp)))), y(T(yw::get<1>(static_cast<Tp&&>(tp)))), z(), w() {}

  template<tuple_like<3> Tp> requires(!castable_to<Tp, T>)
  constexpr vector(Tp&& tp)
    : x(T(yw::get<0>(static_cast<Tp&&>(tp)))), y(T(yw::get<1>(static_cast<Tp&&>(tp)))),
      z(T(yw::get<2>(static_cast<Tp&&>(tp)))), w() {}

  template<tuple_like<4> Tp> requires(!castable_to<Tp, T> && !same_as<remove_cvref<Tp>, vector>)
  constexpr vector(Tp&& tp)
    : x(T(yw::get<0>(static_cast<Tp&&>(tp)))), y(T(yw::get<1>(static_cast<Tp&&>(tp)))),
      z(T(yw::get<2>(static_cast<Tp&&>(tp)))), w(T(yw::get<3>(static_cast<Tp&&>(tp)))) {}

  constexpr bool empty() const noexcept { return false; }
  constexpr size_t size() const noexcept { return count; }

  T* data() noexcept { return &x; }
  const T* data() const noexcept { return &x; }
  T* begin() noexcept { return &x; }
  const T* begin() const noexcept { return &x; }
  T* end() noexcept { return &x + count; }
  const T* end() const noexcept { return &x + count; }

  constexpr T& front() noexcept { return x; }
  constexpr const T& front() const noexcept { return x; }
  constexpr T& back() noexcept { return w; }
  constexpr const T& back() const noexcept { return w; }

  constexpr T& operator[](integral auto i) {
    const auto ii = size_t((i % 4) + 4) % 4;
    if (!std::is_constant_evaluated()) return *(&x + ii);
    return ii == 0 ? x : (ii == 1 ? y : (ii == 2 ? z : w));
  }
  constexpr const T& operator[](integral auto i) const {
    const auto ii = size_t((i % 4) + 4) % 4;
    if (!std::is_constant_evaluated()) return *(&x + ii);
    return ii == 0 ? x : (ii == 1 ? y : (ii == 2 ? z : w));
  }

  template<size_t I> requires(I < 4)
  constexpr T& get() & noexcept {
    return select<I>(x, y, z, w);
  }
  template<size_t I> requires(I < 4)
  constexpr T&& get() && noexcept {
    return std::move(select<I>(x, y, z, w));
  }
  template<size_t I> requires(I < 4)
  constexpr const T& get() const& noexcept {
    return select<I>(x, y, z, w);
  }
  template<size_t I> requires(I < 4)
  constexpr const T&& get() const&& noexcept {
    return std::move(select<I>(x, y, z, w));
  }

  constexpr vector operator()() const noexcept { return *this; }
};
} // namespace yw

//////////////////////////////////////// MARK: std

namespace std {

template<size_t I, typename T, size_t N> //
struct tuple_element<I, yw::vector<T, N>> : type_identity<T> {};

template<typename T, size_t N> //
struct tuple_size<yw::vector<T, N>> : integral_constant<size_t, N> {};

template<typename C, typename T, size_t N> struct formatter<yw::vector<T, N>, C> {
  std::formatter<T, C> fmt;
  constexpr auto parse(auto& ctx) { return fmt.parse(ctx); }
  auto format(const yw::vector<T, N>& vec, auto& ctx) const {
    auto out = ctx.out();
    *out++ = '(', out = fmt.format(vec[0], ctx);
    for (size_t i = 1; i < N; ++i) *out++ = ',', out = fmt.format(vec[i], ctx);
    *out++ = ')';
    return out;
  }
};
} // namespace std
