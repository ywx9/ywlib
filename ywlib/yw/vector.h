#pragma once
#include <yw/math.h>
#include <yw/string.h>
#include <yw/tuple.h>

/// \note

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

  template<tuple_like Tp> requires(lt(extent<Tp>, N) && !castable_to<Tp, T> && !variation_of<Tp, vector<T, N>>)
  explicit constexpr vector(Tp&& tp) {
    constexpr auto seq = make_sequence<0, extent<Tp>>{};
    [&]<size_t... Is>(sequence<Is...>) { ((_vals[Is] = T(yw::get<Is>(static_cast<Tp&&>(tp)))), ...); }(seq);
  }

  template<castable_to<T> U> constexpr vector(const vector<U, N>& v) noexcept(nt_castable_to<U, T>) {
    for (size_t i = 0; i < N; ++i) _vals[i] = static_cast<T>(v[i]);
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
  constexpr const auto& operator[](integral auto i) const noexcept { return _vals[size_t((i % N) + N) % N]; }

  template<size_t I> requires(I < N) constexpr auto& get() & { return _vals[I]; }
  template<size_t I> requires(I < N) constexpr const auto& get() const& { return _vals[I]; }
  template<size_t I> requires(I < N) constexpr auto&& get() && { return std::move(_vals[I]); }
  template<size_t I> requires(I < N) constexpr const auto&& get() const&& { return std::move(_vals[I]); }

  constexpr vector operator()() const noexcept { return *this; }

  template<char_type C> constexpr string<C> to_string() const {
    if (N == 0) return string<C>("()");
    string<C> s;
    s.reserve((8 * sizeof(T) + 2) * N);
    s.push_back('(');
    s.append(yw::format(operator[](0)));
    for (size_t i = 1; i < N; ++i) {
      s.push_back(',').push_back(' ');
      s.append(yw::format(operator[](i)));
    }
    s.push_back(')');
    return s;
  }

  constexpr string<char> to_string() const { return to_string<char>(); }

  template<size_t I, castable_to<T> U> requires(I < N) constexpr auto insert(U&& V) const
    noexcept(noexcept(operator[](I) = static_cast<U&&>(V))) requires requires { operator[](I) = static_cast<U&&>(V); } {
    vector res = *this;
    res._vals[I] = static_cast<U&&>(V);
    return res;
  }

  template<size_t I> requires(I < N) constexpr auto add(const std::regular auto& V) const
    noexcept(noexcept(operator[](I) + V)) requires requires { operator[](I) + V; } {
    vector<decltype(operator[](I) + V), N> res = *this;
    res._vals[I] += V;
    return res;
  }

  constexpr auto squared_length() const requires requires { T{} * T{}; } {
    using RT = decltype(T{} * T{});
    RT result{};
    for (size_t i = 0; i < N; ++i) result += operator[](i) * operator[](i);
    return result;
  }

  constexpr auto length() const requires requires(const vector& v) { yw::sqrt(v.squared_length()); } {
    return yw::sqrt(squared_length());
  }

  constexpr auto normalized() const requires requires(const vector& v) { v / v.length(); } {
    const auto len = length();
    if (len <= 0) return decltype(*this / len){};
    return *this / len;
  }
};

template<typename T, typename... Ts> vector(T&&, Ts&&...) -> vector<T, sizeof...(Ts) + 1>;
template<tuple_like Tp> vector(Tp&&) -> vector<remove_cvref<element_t<Tp, 0>>, extent<Tp>>;

template<std::regular T> using vector1 = vector<T, 1>;
template<std::regular T> using vector2 = vector<T, 2>;
template<std::regular T> using vector3 = vector<T, 3>;
template<std::regular T> using vector4 = vector<T, 4>;

using bool2 = vector2<bool>;
using bool3 = vector3<bool>;
using bool4 = vector4<bool>;
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
  return [&]<size_t... Is>(sequence<Is...>) -> bool { return ((get<Is>(a) == get<Is>(b)) && ...); }(seq);
}

template<typename T, std::three_way_comparable_with<T> U, size_t N>
constexpr std::partial_ordering operator<=>(const vector<T, N>& a, const vector<U, N>& b) {
  for (size_t i = 0; i < N; ++i)
    if (auto cmp = a[i] <=> b[i]; cmp != 0) return cmp;
  return std::partial_ordering::equivalent;
}

//////////////////////////////////////// MARK: unary plus

template<typename T, size_t N> constexpr vector<T, N> operator+(const vector<T, N>& a) { return a; }

//////////////////////////////////////// MARK: unary minus

template<typename T, size_t N> constexpr vector<T, N> operator-(const vector<T, N>& a) {
  return [&]<size_t... Is>(sequence<Is...>) -> vector<T, N> { return {(-get<Is>(a))...}; }(make_sequence<0, N>{});
}

//////////////////////////////////////// MARK: logical not

template<typename T, size_t N> requires requires(const T& a) { !a; }
constexpr vector<decltype(!std::declval<const T&>()), N> operator!(const vector<T, N>& a) {
  return [&]<size_t... Is>(sequence<Is...>) -> vector<decltype(!std::declval<const T&>()), N> {
    return {(!get<Is>(a))...};
  }(make_sequence<0, N>{});
}

//////////////////////////////////////// MARK: binary plus

template<typename T, typename U, size_t N>
constexpr vector<decltype(T{} + U{}), N> operator+(const vector<T, N>& a, const vector<U, N>& b) {
  return [&]<size_t... Is>(sequence<Is...>) -> vector<decltype(T{} + U{}), N> {
    return {(get<Is>(a) + get<Is>(b))...};
  }(make_sequence<0, N>{});
}

//////////////////////////////////////// MARK: binary minus

template<typename T, typename U, size_t N>
constexpr vector<decltype(T{} - U{}), N> operator-(const vector<T, N>& a, const vector<U, N>& b) {
  return [&]<size_t... Is>(sequence<Is...>) -> vector<decltype(T{} - U{}), N> {
    return {(get<Is>(a) - get<Is>(b))...};
  }(make_sequence<0, N>{});
}

//////////////////////////////////////// MARK: multiplication

template<typename T, typename U, size_t N>
constexpr vector<decltype(T{} * U{}), N> operator*(const vector<T, N>& a, const vector<U, N>& b) {
  return [&]<size_t... Is>(sequence<Is...>) -> vector<decltype(T{} * U{}), N> {
    return {(get<Is>(a) * get<Is>(b))...};
  }(make_sequence<0, N>{});
}

template<typename T, typename U, size_t N>
constexpr vector<decltype(T{} * U{}), N> operator*(const vector<T, N>& a, const U & b) {
  using RT = decltype(T{} * U{});
  const auto bb = static_cast<RT>(b);
  return [&]<size_t... Is>(sequence<Is...>) -> vector<RT, N> { return {(get<Is>(a) * bb)...}; }(make_sequence<0, N>{});
}

template<typename T, typename U, size_t N>
constexpr vector<decltype(U{} * T{}), N> operator*(const U & a, const vector<T, N>& b) {
  using RT = decltype(U{} * T{});
  const auto aa = static_cast<RT>(a);
  return [&]<size_t... Is>(sequence<Is...>) -> vector<RT, N> { return {(aa * get<Is>(b))...}; }(make_sequence<0, N>{});
}

//////////////////////////////////////// MARK: division

template<typename T, typename U, size_t N>
constexpr vector<decltype(T{} / U{}), N> operator/(const vector<T, N>& a, const vector<U, N>& b) {
  return [&]<size_t... Is>(sequence<Is...>) -> vector<decltype(T{} / U{}), N> {
    return {(get<Is>(a) / get<Is>(b))...};
  }(make_sequence<0, N>{});
}

template<typename T, typename U, size_t N>
constexpr vector<decltype(T{} / U{}), N> operator/(const vector<T, N>& a, const U & b) {
  using RT = decltype(T{} / U{});
  const auto bb = static_cast<RT>(b);
  return [&]<size_t... Is>(sequence<Is...>) -> vector<RT, N> { return {(get<Is>(a) / bb)...}; }(make_sequence<0, N>{});
}

//////////////////////////////////////// MARK: plus assign

template<typename T, typename U, size_t N> requires requires { T() + U(); }
constexpr vector<T, N>& operator+=(vector<T, N>& a, const vector<U, N>& b) {
  [&]<size_t... Is>(sequence<Is...>) { ((get<Is>(a) += get<Is>(b)), ...); }(make_sequence<0, N>{});
  return a;
}

//////////////////////////////////////// MARK: minus assign

template<typename T, typename U, size_t N> requires requires { T() - U(); }
constexpr vector<T, N>& operator-=(vector<T, N>& a, const vector<U, N>& b) {
  [&]<size_t... Is>(sequence<Is...>) { ((get<Is>(a) -= get<Is>(b)), ...); }(make_sequence<0, N>{});
  return a;
}

//////////////////////////////////////// MARK: multiplication assign

template<typename T, typename U, size_t N> requires requires { T() * U(); }
constexpr vector<T, N>& operator*=(vector<T, N>& a, const vector<U, N>& b) {
  [&]<size_t... Is>(sequence<Is...>) { ((get<Is>(a) *= get<Is>(b)), ...); }(make_sequence<0, N>{});
  return a;
}

template<typename T, typename U, size_t N> requires requires { T() * U(); }
constexpr vector<T, N>& operator*=(vector<T, N>& a, const U& b) {
  [&]<size_t... Is>(sequence<Is...>) { ((get<Is>(a) *= b), ...); }(make_sequence<0, N>{});
  return a;
}

//////////////////////////////////////// MARK: division assign

template<typename T, typename U, size_t N> requires requires { T() / U(); }
constexpr vector<T, N>& operator/=(vector<T, N>& a, const vector<U, N>& b) {
  [&]<size_t... Is>(sequence<Is...>) { ((get<Is>(a) /= get<Is>(b)), ...); }(make_sequence<0, N>{});
  return a;
}

template<typename T, typename U, size_t N> requires requires { T() / U(); }
constexpr vector<T, N>& operator/=(vector<T, N>& a, const U& b) {
  [&]<size_t... Is>(sequence<Is...>) { ((get<Is>(a) /= b), ...); }(make_sequence<0, N>{});
  return a;
}

//////////////////////////////////////// MARK: vector operations

// template<typename T, typename U, size_t N> constexpr auto mul(const vector<T, N>& a, const U& b) { return a * b; }
// template<typename T, typename U, size_t N> constexpr auto mul(const U& a, const vector<T, N>& b) { return a * b; }
// template<typename T, typename U, size_t N> constexpr auto mul(const vector<T, N>& a, const vector<U, N>& b) {
//   return [&]<size_t... Is>(sequence<Is...>) { return ((get<Is>(a) * get<Is>(b)) + ...); }(make_sequence<0, N>{});
// }

template<typename T, typename U, size_t N>
requires(!variation_of<T, vector<int, 1>> && !variation_of<U, vector<int, 1>>)
constexpr auto dot(const vector<T, N>& a, const vector<U, N>& b) {
  return [&]<size_t... Is>(sequence<Is...>) { return ((get<Is>(a) * get<Is>(b)) + ...); }(make_sequence<0, N>{});
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

  template<castable_to<T> Xt> constexpr vector(Xt&& X) noexcept(nt_castable_to<Xt, T>) : x(T(static_cast<Xt&&>(X))) {}

  template<tuple_like<1> Tp> requires(!castable_to<Tp, T> && !variation_of<Tp, vector<T, 1>>)
  explicit constexpr vector(Tp&& tp) noexcept : x(T(yw::get<0>(static_cast<Tp&&>(tp)))) {}

  template<castable_to<T> U> constexpr vector(const vector<U, 1>& v) noexcept(nt_castable_to<U, T>) {
    x = static_cast<T>(v[0]);
  }

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

  template<size_t I> requires(I == 0) constexpr T& get() & noexcept { return x; }
  template<size_t I> requires(I == 0) constexpr T&& get() && noexcept { return std::move(x); }
  template<size_t I> requires(I == 0) constexpr const T& get() const& noexcept { return x; }
  template<size_t I> requires(I == 0) constexpr const T&& get() const&& noexcept { return std::move(x); }

  constexpr vector operator()() const noexcept { return *this; }

  template<char_type C> constexpr string<C> to_string() const {
    string<C> s;
    s.reserve((8 * sizeof(T) + 2) * 1);
    s.push_back('(');
    s.append(yw::format(x));
    s.push_back(')');
    return s;
  }

  constexpr string<char> to_string() const { return to_string<char>(); }

  template<size_t I> requires(I < 1) constexpr auto insert(castable_to<T> auto&& V) const {
    return vector<T, 1>(static_cast<T&&>(V));
  }

  template<size_t I> requires(I < 1) constexpr auto add(const std::regular auto& V) const
    noexcept(noexcept(operator[](I) + V)) requires requires { operator[](I) + V; } {
    return vector<decltype(operator[](I) + V), 1>(x + V);
  }

  constexpr auto squared_length() const requires requires { x * x; } { return x * x; }

  constexpr auto length() const requires requires(const vector& v) { yw::sqrt(v.squared_length()); } {
    return yw::sqrt(squared_length());
  }

  constexpr auto normalized() const requires requires(const vector& v) { v / v.length(); } {
    const auto len = length();
    if (len <= 0) return decltype(*this / len){};
    return *this / len;
  }
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

  template<tuple_like<1> Tp> requires(!castable_to<Tp, T> && !variation_of<Tp, vector<T, 2>>)
  explicit constexpr vector(Tp&& tp) : x(T(yw::get<0>(static_cast<Tp&&>(tp)))), y() {}

  template<tuple_like<2> Tp> requires(!castable_to<Tp, T> && !variation_of<Tp, vector<T, 2>>)
  explicit constexpr vector(Tp&& tp)
    : x(T(yw::get<0>(static_cast<Tp&&>(tp)))), y(T(yw::get<1>(static_cast<Tp&&>(tp)))) {}

  template<castable_to<T> U> constexpr vector(const vector<U, 2>& v) noexcept(nt_castable_to<U, T>) {
    x = static_cast<T>(v[0]);
    y = static_cast<T>(v[1]);
  }

  static constexpr bool empty() noexcept { return false; }
  static constexpr size_t size() noexcept { return 2; }

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
    else return ii == 0 ? x : y;
  }
  constexpr const T& operator[](integral auto i) const noexcept {
    const auto ii = ((i % 2) + 2) % 2;
    if (!std::is_constant_evaluated()) return *(&x + ii);
    else return ii == 0 ? x : y;
  }

  template<size_t I> requires(I < 2) constexpr T& get() & noexcept { return select<I>(x, y); }
  template<size_t I> requires(I < 2) constexpr T&& get() && noexcept { return std::move(select<I>(x, y)); }
  template<size_t I> requires(I < 2) constexpr const T& get() const& noexcept { return select<I>(x, y); }
  template<size_t I> requires(I < 2) constexpr const T&& get() const&& noexcept { return std::move(select<I>(x, y)); }

  constexpr vector operator()() const noexcept { return *this; }

  constexpr vector yx() const noexcept { return {y, x}; }

  template<char_type C> constexpr string<C> to_string() const {
    string<C> s;
    s.reserve((8 * sizeof(T) + 2) * 2);
    s.push_back(C('(')).append(yw::format<C>(x)).push_back(C(','));
    s.push_back(C(' ')).append(yw::format<C>(y)).push_back(C(')'));
    return s;
  }

  constexpr string<char> to_string() const { return to_string<char>(); }

  template<size_t I, castable_to<T> U> requires(I < 2) constexpr auto insert(U&& V) const {
    if constexpr (I == 0) return vector<T, 2>(static_cast<U&&>(V), y);
    else return vector<T, 2>(x, static_cast<U&&>(V));
  }

  template<size_t I> requires(I < 2) constexpr auto add(const std::regular auto& V) const
    noexcept(noexcept(operator[](I) + V)) requires requires { operator[](I) + V; } {
    if constexpr (I == 0) return vector<decltype(operator[](I) + V), 2>(x + V, y);
    else return vector<decltype(operator[](I) + V), 2>(x, y + V);
  }

  constexpr auto squared_length() const requires requires { x * x + y * y; } { return x * x + y * y; }

  constexpr auto length() const requires requires(const vector& v) { yw::sqrt(v.squared_length()); } {
    return yw::sqrt(squared_length());
  }

  constexpr auto normalized() const requires requires(const vector& v) { v / v.length(); } {
    const auto len = length();
    if (len <= 0) return decltype(*this / len){};
    return *this / len;
  }
};

//////////////////////////////////////// MARK: VECTOR3

template<std::regular T> struct vector<T, 3> {
  using value_type = T;
  static constexpr size_t count{3};
  T x{}, y{}, z{};

  static constexpr vector fill(const T& value) { return {value, value, value}; }

  constexpr vector() noexcept = default;

  template<castable_to<T> Xt, castable_to<T> Yt, castable_to<T> Zt> constexpr vector(Xt&& X, Yt&& Y, Zt&& Z) noexcept(
    nt_castable_to<Xt, T> && nt_castable_to<Yt, T> && nt_castable_to<Zt, T>)
    : x(T(static_cast<Xt&&>(X))), y(T(static_cast<Yt&&>(Y))), z(T(static_cast<Zt&&>(Z))) {}

  template<tuple_like<1> Tp> requires(!castable_to<Tp, T> && !variation_of<Tp, vector<T, 3>>)
  explicit constexpr vector(Tp&& tp) : x(T(yw::get<0>(static_cast<Tp&&>(tp)))), y(), z() {}

  template<tuple_like<2> Tp> requires(!castable_to<Tp, T> && !variation_of<Tp, vector<T, 3>>)
  explicit constexpr vector(Tp&& tp)
    : x(T(yw::get<0>(static_cast<Tp&&>(tp)))), y(T(yw::get<1>(static_cast<Tp&&>(tp)))), z() {}

  template<tuple_like<3> Tp> requires(!castable_to<Tp, T> && !variation_of<Tp, vector<T, 3>>)
  explicit constexpr vector(Tp&& tp)
    : x(T(yw::get<0>(static_cast<Tp&&>(tp)))), y(T(yw::get<1>(static_cast<Tp&&>(tp)))),
      z(T(yw::get<2>(static_cast<Tp&&>(tp)))) {}

  template<castable_to<T> U> constexpr vector(const vector<U, 3>& v) noexcept(nt_castable_to<U, T>) {
    x = static_cast<T>(v[0]);
    y = static_cast<T>(v[1]);
    z = static_cast<T>(v[2]);
  }

  static constexpr bool empty() noexcept { return false; }
  static constexpr size_t size() noexcept { return count; }

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

  template<size_t I> requires(I < 3) constexpr T& get() & noexcept { return select<I>(x, y, z); }
  template<size_t I> requires(I < 3) constexpr T&& get() && noexcept { return std::move(select<I>(x, y, z)); }
  template<size_t I> requires(I < 3) constexpr const T& get() const& noexcept { return select<I>(x, y, z); }
  template<size_t I> requires(I < 3) constexpr const T&& get() const&& noexcept {
    return std::move(select<I>(x, y, z));
  }

  constexpr vector operator()() const noexcept { return *this; }

  constexpr vector<T, 2> xy() const noexcept { return {x, y}; }
  constexpr vector<T, 2> xz() const noexcept { return {x, z}; }
  constexpr vector<T, 2> yz() const noexcept { return {y, z}; }

  template<char_type C> constexpr string<C> to_string() const {
    string<C> s;
    s.reserve((8 * sizeof(T) + 2) * 3);
    s.push_back(C('(')).append(yw::format<C>(x)).push_back(C(','));
    s.push_back(C(' ')).append(yw::format<C>(y)).push_back(C(','));
    s.push_back(C(' ')).append(yw::format<C>(z)).push_back(C(')'));
    return s;
  }

  constexpr string<char> to_string() const { return to_string<char>(); }

  template<size_t I, castable_to<T> U> requires(I < 3) constexpr auto insert(U&& V) const {
    if constexpr (I == 0) return vector<T, 3>(static_cast<U&&>(V), y, z);
    else if constexpr (I == 1) return vector<T, 3>(x, static_cast<U&&>(V), z);
    else return vector<T, 3>(x, y, static_cast<U&&>(V));
  }

  template<size_t I> requires(I < 3) constexpr auto add(const std::regular auto& V) const
    noexcept(noexcept(operator[](I) + V)) requires requires { operator[](I) + V; } {
    if constexpr (I == 0) return vector<decltype(operator[](I) + V), 3>(x + V, y, z);
    else if constexpr (I == 1) return vector<decltype(operator[](I) + V), 3>(x, y + V, z);
    else return vector<decltype(operator[](I) + V), 3>(x, y, z + V);
  }

  constexpr auto squared_length() const requires requires { x * x + y * y + z * z; } { return x * x + y * y + z * z; }

  constexpr auto length() const requires requires(const vector& v) { yw::sqrt(v.squared_length()); } {
    return yw::sqrt(squared_length());
  }

  constexpr auto normalized() const requires requires(const vector& v) { v / v.length(); } {
    const auto len = length();
    if (len <= 0) return decltype(*this / len){};
    return *this / len;
  }
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
  constexpr vector(Xt&& X, Yt&& Y, Zt&& Z, Wt&& W) noexcept(
    nt_castable_to<Xt, T> && nt_castable_to<Yt, T> && nt_castable_to<Zt, T> && nt_castable_to<Wt, T>)
    : x(T(static_cast<Xt&&>(X))), y(T(static_cast<Yt&&>(Y))), z(T(static_cast<Zt&&>(Z))), w(T(static_cast<Wt&&>(W))) {}

  template<tuple_like<1> Tp> requires(!castable_to<Tp, T> && !variation_of<Tp, vector<T, 4>>)
  explicit constexpr vector(Tp&& tp) : x(T(yw::get<0>(static_cast<Tp&&>(tp)))), y(), z(), w() {}

  template<tuple_like<2> Tp> requires(!castable_to<Tp, T> && !variation_of<Tp, vector<T, 4>>)
  explicit constexpr vector(Tp&& tp)
    : x(T(yw::get<0>(static_cast<Tp&&>(tp)))), y(T(yw::get<1>(static_cast<Tp&&>(tp)))), z(), w() {}

  template<tuple_like<3> Tp> requires(!castable_to<Tp, T> && !variation_of<Tp, vector<T, 4>>)
  explicit constexpr vector(Tp&& tp)
    : x(T(yw::get<0>(static_cast<Tp&&>(tp)))), y(T(yw::get<1>(static_cast<Tp&&>(tp)))),
      z(T(yw::get<2>(static_cast<Tp&&>(tp)))), w() {}

  template<tuple_like<4> Tp> requires(!castable_to<Tp, T> && !variation_of<Tp, vector<T, 4>>)
  explicit constexpr vector(Tp&& tp)
    : x(T(yw::get<0>(static_cast<Tp&&>(tp)))), y(T(yw::get<1>(static_cast<Tp&&>(tp)))),
      z(T(yw::get<2>(static_cast<Tp&&>(tp)))), w(T(yw::get<3>(static_cast<Tp&&>(tp)))) {}

  template<tuple_like<2> Tp1, tuple_like<2> Tp2> requires(!castable_to<Tp1, T> && !castable_to<Tp2, T>)
  explicit constexpr vector(Tp1&& tp1, Tp2&& tp2)
    : x(T(yw::get<0>(static_cast<Tp1&&>(tp1)))), y(T(yw::get<1>(static_cast<Tp1&&>(tp1)))),
      z(T(yw::get<0>(static_cast<Tp2&&>(tp2)))), w(T(yw::get<1>(static_cast<Tp2&&>(tp2)))) {}

  template<castable_to<T> U> constexpr vector(const vector<U, 4>& v) noexcept(nt_castable_to<U, T>) {
    x = static_cast<T>(v[0]);
    y = static_cast<T>(v[1]);
    z = static_cast<T>(v[2]);
    w = static_cast<T>(v[3]);
  }

  template<castable_to<T> U, castable_to<T> V>
  constexpr vector(const vector<U, 3>& u, const V& v) noexcept(nt_castable_to<U, T> && nt_castable_to<V, T>) {
    x = static_cast<T>(u[0]);
    y = static_cast<T>(u[1]);
    z = static_cast<T>(u[2]);
    w = static_cast<T>(v);
  }

  static constexpr bool empty() noexcept { return false; }
  static constexpr size_t size() noexcept { return count; }

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

  constexpr T& operator[](integral auto i) noexcept {
    const auto ii = size_t((i % 4) + 4) % 4;
    if (!std::is_constant_evaluated()) return *(&x + ii);
    return ii == 0 ? x : (ii == 1 ? y : (ii == 2 ? z : w));
  }
  constexpr const T& operator[](integral auto i) const noexcept {
    const auto ii = size_t((i % 4) + 4) % 4;
    if (!std::is_constant_evaluated()) return *(&x + ii);
    return ii == 0 ? x : (ii == 1 ? y : (ii == 2 ? z : w));
  }

  template<size_t I> requires(I < 4) constexpr T& get() & noexcept { return select<I>(x, y, z, w); }
  template<size_t I> requires(I < 4) constexpr T&& get() && noexcept { return std::move(select<I>(x, y, z, w)); }
  template<size_t I> requires(I < 4) constexpr const T& get() const& noexcept { return select<I>(x, y, z, w); }
  template<size_t I> requires(I < 4) constexpr const T&& get() const&& noexcept {
    return std::move(select<I>(x, y, z, w));
  }

  constexpr vector operator()() const noexcept { return *this; }

  constexpr vector<T, 3> xyz() const noexcept { return {x, y, z}; }
  constexpr vector<T, 2> xy() const noexcept { return {x, y}; }
  constexpr vector<T, 2> xz() const noexcept { return {x, z}; }
  constexpr vector<T, 2> xw() const noexcept { return {x, w}; }
  constexpr vector<T, 2> yz() const noexcept { return {y, z}; }
  constexpr vector<T, 2> yw() const noexcept { return {y, w}; }
  constexpr vector<T, 2> zw() const noexcept { return {z, w}; }

  template<char_type C> constexpr string<C> to_string() const {
    string<C> s;
    s.reserve((8 * sizeof(T) + 2) * 4);
    s.push_back(C('(')).append(yw::format<C>(x)).push_back(C(','));
    s.push_back(C(' ')).append(yw::format<C>(y)).push_back(C(','));
    s.push_back(C(' ')).append(yw::format<C>(z)).push_back(C(','));
    s.push_back(C(' ')).append(yw::format<C>(w)).push_back(C(')'));
    return s;
  }

  constexpr string<char> to_string() const { return to_string<char>(); }

  template<size_t I, castable_to<T> U> requires(I < 4) constexpr auto insert(U&& V) const {
    if constexpr (I == 0) return vector<T, 4>(static_cast<U&&>(V), y, z, w);
    else if constexpr (I == 1) return vector<T, 4>(x, static_cast<U&&>(V), z, w);
    else if constexpr (I == 2) return vector<T, 4>(x, y, static_cast<U&&>(V), w);
    else return vector<T, 4>(x, y, z, static_cast<U&&>(V));
  }

  template<size_t I> requires(I < 4) constexpr auto add(const std::regular auto& V) const
    noexcept(noexcept(operator[](I) + V)) requires requires { operator[](I) + V; } {
    if constexpr (I == 0) return vector<decltype(operator[](I) + V), 4>(x + V, y, z, w);
    else if constexpr (I == 1) return vector<decltype(operator[](I) + V), 4>(x, y + V, z, w);
    else if constexpr (I == 2) return vector<decltype(operator[](I) + V), 4>(x, y, z + V, w);
    else return vector<decltype(operator[](I) + V), 4>(x, y, z, w + V);
  }

  constexpr auto squared_length() const requires requires { x * x + y * y + z * z + w * w; } {
    return x * x + y * y + z * z + w * w;
  }

  constexpr auto length() const requires requires(const vector& v) { yw::sqrt(v.squared_length()); } {
    return yw::sqrt(squared_length());
  }

  constexpr auto normalized() const requires requires(const vector& v) { v / v.length(); } {
    const auto len = length();
    if (len <= 0) return decltype(*this / len){};
    return *this / len;
  }
};

inline constexpr float2 rotate(float2 v, float AngleRad) noexcept {
  const auto c = yw::cos(AngleRad);
  const auto s = yw::sin(AngleRad);
  return {v.x * c - v.y * s, v.x * s + v.y * c};
}

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
