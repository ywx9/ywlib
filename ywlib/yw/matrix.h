#pragma once
#include <initializer_list>
#include <yw/math.h>
#include <yw/tuple.h>
#include <yw/vector.h>

namespace yw {

template<std::regular T, size_t Cols> struct matrix_row {
  using value_type = T;
  static constexpr size_t count{Cols};

  T _vals[Cols]{};

  static constexpr matrix_row fill(const T& value) {
    matrix_row row;
    std::ranges::fill(row._vals, value);
    return row;
  }

  constexpr matrix_row() noexcept(std::is_nothrow_default_constructible_v<T>) = default;

  template<typename... Us> requires(sizeof...(Us) == Cols && (castable_to<Us, T> && ...))
  explicit(!(convertible_to<Us, T> && ...)) constexpr matrix_row(Us&&... as) noexcept(
    (noexcept(static_cast<T>(std::declval<Us>())) && ...))
    : _vals{static_cast<T>(static_cast<Us&&>(as))...} {}

  template<tuple_like Tp> requires(extent<Tp> == Cols && !castable_to<Tp, T> && !variation_of<Tp, matrix_row<T, Cols>>)
  explicit constexpr matrix_row(Tp&& tp) {
    constexpr auto seq = make_sequence<0, Cols>{};
    [&]<size_t... Is>(sequence<Is...>) { ((_vals[Is] = T(yw::get<Is>(static_cast<Tp&&>(tp)))), ...); }(seq);
  }

  template<castable_to<T> U> constexpr matrix_row(const matrix_row<U, Cols>& row) noexcept(nt_castable_to<U, T>) {
    for (size_t i = 0; i < Cols; ++i) _vals[i] = static_cast<T>(row[i]);
  }

  static constexpr bool empty() noexcept { return Cols == 0; }
  static constexpr size_t size() noexcept { return Cols; }

  constexpr T* begin() noexcept { return _vals; }
  constexpr const T* begin() const noexcept { return _vals; }
  constexpr T* end() noexcept { return _vals + Cols; }
  constexpr const T* end() const noexcept { return _vals + Cols; }
  constexpr T* data() noexcept { return _vals; }
  constexpr const T* data() const noexcept { return _vals; }

  constexpr T& front() noexcept { return _vals[0]; }
  constexpr const T& front() const noexcept { return _vals[0]; }
  constexpr T& back() noexcept { return _vals[Cols - 1]; }
  constexpr const T& back() const noexcept { return _vals[Cols - 1]; }
  constexpr T& operator[](integral auto i) noexcept { return _vals[size_t((i % Cols) + Cols) % Cols]; }
  constexpr const T& operator[](integral auto i) const noexcept { return _vals[size_t((i % Cols) + Cols) % Cols]; }

  template<size_t I> requires(I < Cols) constexpr T& get() & noexcept { return _vals[I]; }
  template<size_t I> requires(I < Cols) constexpr const T& get() const& noexcept { return _vals[I]; }
  template<size_t I> requires(I < Cols) constexpr T&& get() && noexcept { return std::move(_vals[I]); }
  template<size_t I> requires(I < Cols) constexpr const T&& get() const&& noexcept { return std::move(_vals[I]); }
};

template<typename T, typename... Ts> matrix_row(T&&, Ts&&...) -> matrix_row<remove_cvref<T>, sizeof...(Ts) + 1>;
template<tuple_like Tp> matrix_row(Tp&&) -> matrix_row<remove_cvref<element_t<Tp, 0>>, extent<Tp>>;

template<std::regular T, size_t Rows, size_t Cols> struct matrix {
  using value_type = T;
  using row_type = matrix_row<T, Cols>;
  static constexpr size_t rows{Rows};
  static constexpr size_t cols{Cols};

  row_type _rows[Rows]{};

  constexpr matrix() noexcept(std::is_nothrow_default_constructible_v<row_type>) = default;

  constexpr matrix(std::initializer_list<row_type> rows) noexcept {
    size_t r = 0;
    for (const auto& row : rows) {
      if (r == Rows) break;
      _rows[r++] = row;
    }
  }

  template<castable_to<T> U> explicit constexpr matrix(const matrix<U, Rows, Cols>& m) noexcept(nt_castable_to<U, T>) {
    for (size_t r = 0; r < Rows; ++r)
      for (size_t c = 0; c < Cols; ++c) _rows[r][c] = static_cast<T>(m[r][c]);
  }

  static constexpr matrix fill(const T& value) {
    matrix m;
    for (auto& row : m._rows) row = row_type::fill(value);
    return m;
  }

  constexpr row_type* begin() noexcept { return _rows; }
  constexpr const row_type* begin() const noexcept { return _rows; }
  constexpr row_type* end() noexcept { return _rows + Rows; }
  constexpr const row_type* end() const noexcept { return _rows + Rows; }
  constexpr T* data() noexcept { return _rows[0].data(); }
  constexpr const T* data() const noexcept { return _rows[0].data(); }

  constexpr row_type& operator[](integral auto r) noexcept { return _rows[size_t((r % Rows) + Rows) % Rows]; }
  constexpr const row_type& operator[](integral auto r) const noexcept {
    return _rows[size_t((r % Rows) + Rows) % Rows];
  }

  template<size_t I> requires(I < Rows) constexpr row_type& get() & noexcept { return _rows[I]; }
  template<size_t I> requires(I < Rows) constexpr const row_type& get() const& noexcept { return _rows[I]; }
  template<size_t I> requires(I < Rows) constexpr row_type&& get() && noexcept { return std::move(_rows[I]); }
  template<size_t I> requires(I < Rows) constexpr const row_type&& get() const&& noexcept {
    return std::move(_rows[I]);
  }

  template<arithmetic U> constexpr vector<math_type<T, U>, Rows> transform(const vector<U, Cols>& v) const noexcept {
    vector<math_type<T, U>, Rows> out;
    for (size_t r = 0; r < Rows; ++r) {
      out[r] = {};
      for (size_t c = 0; c < Cols; ++c) out[r] += _rows[r][c] * v[c];
    }
    return out;
  }
};

template<typename T, typename U, size_t Rows, size_t Cols>
requires(!variation_of<T, vector<int, 1>> && !variation_of<U, vector<int, 1>>)
constexpr auto outer(const vector<T, Rows>& a, const vector<U, Cols>& b) {
  matrix<decltype(T{} * U{}), Rows, Cols> result;
  for (size_t i = 0; i < Rows; ++i)
    for (size_t j = 0; j < Cols; ++j) result[i][j] = a[i] * b[j];
  return result;
}

/// MARK: transpose

template<typename T, size_t Rows, size_t Cols>
constexpr matrix<T, Cols, Rows> transpose(const matrix<T, Rows, Cols>& m) noexcept {
  matrix<T, Cols, Rows> out;
  for (size_t r = 0; r < Rows; ++r)
    for (size_t c = 0; c < Cols; ++c) out[c][r] = m[r][c];
  return out;
}

template<typename T, size_t Row, size_t Col>
constexpr void transpose(const matrix<T, Row, Col>& m, matrix<T, Col, Row>& out) noexcept {
  for (size_t r = 0; r < Row; ++r)
    for (size_t c = 0; c < Col; ++c) out[c][r] = m[r][c];
}

template<typename T, size_t Rows, size_t Cols> requires(Rows == Cols)
constexpr void transpose_inplace(matrix<T, Rows, Cols>& m) noexcept {
  for (size_t r = 0; r < Rows; ++r)
    for (size_t c = r + 1; c < Cols; ++c) std::swap(m[r][c], m[c][r]);
}

/// MARK: transform

template<arithmetic T, arithmetic U, size_t Rows, size_t Cols>
constexpr vector<math_type<T, U>, Rows> transform(const matrix<T, Rows, Cols>& m, const vector<U, Cols>& v) noexcept {
  return m.transform(v);
}

/// MARK: dot

template<arithmetic T, arithmetic U, size_t Rows, size_t Cols, size_t Cols2>
constexpr matrix<math_type<T, U>, Rows, Cols2> dot(
  const matrix<T, Rows, Cols>& a, const matrix<U, Cols, Cols2>& b) noexcept {
  matrix<math_type<T, U>, Rows, Cols2> out;
  for (size_t r = 0; r < Rows; ++r)
    for (size_t c = 0; c < Cols2; ++c) {
      out[r][c] = {};
      for (size_t k = 0; k < Cols; ++k) out[r][c] += a[r][k] * b[k][c];
    }
  return out;
}

template<arithmetic T, arithmetic U, arithmetic V, size_t Rows, size_t Cols, size_t Cols2>
requires(convertible_to<decltype(T{} * U{}), V>) constexpr void dot(
  const matrix<T, Rows, Cols>& a, const matrix<U, Cols, Cols2>& b, matrix<V, Rows, Cols2>& out) noexcept {
  for (size_t r = 0; r < Rows; ++r)
    for (size_t c = 0; c < Cols2; ++c) {
      out[r][c] = {};
      for (size_t k = 0; k < Cols; ++k) out[r][c] += a[r][k] * b[k][c];
    }
}
} // namespace yw

namespace std {
template<size_t I, typename T, size_t N> //
struct tuple_element<I, yw::matrix_row<T, N>> : type_identity<T> {};

template<typename T, size_t N> //
struct tuple_size<yw::matrix_row<T, N>> : integral_constant<size_t, N> {};
} // namespace std
