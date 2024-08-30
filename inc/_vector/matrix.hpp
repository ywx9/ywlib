/// \file vector/matrix.hpp

#pragma once

#include "vector.hpp"

export namespace yw {


/// structure for 4x4 matrix
struct matrix {

  /// first row
  vector x{};

  /// second row
  vector y{};

  /// third row
  vector z{};

  /// fourth row
  vector w{};

  /// number of rows
  static constexpr nat count = 4;

  /// row type
  using value_type = vector;

  /// default constructor
  constexpr matrix() noexcept = default;

  /// constructor with a value to fill
  explicit constexpr matrix(const fat4 Fill) noexcept
    : x(Fill), y(Fill), z{Fill}, w{Fill} {}

  /// constructor with a value to fill
  explicit constexpr matrix(numeric auto&& Fill) noexcept
    requires (!same_as<remove_cvref<decltype(Fill)>, fat4>)
    : matrix(fat4(Fill)) {}

  /// constructor with four vectors
  constexpr matrix(
    const vector& X, const vector& Y, const vector& Z, const vector& W)
    noexcept : x(X), y(Y), z(Z), w(W) {}

  /// performs `get` operation
  template<nat I> requires (I < 4)
  constexpr vector& get() &
  noexcept { return select<I>(x, y, z, w); }

  /// performs `get` operation
  template<nat I> requires (I < 4)
  constexpr const vector& get() const &
  noexcept { return select<I>(x, y, z, w); }

  /// performs `get` operation
  template<nat I> requires (I < 4)
  constexpr vector&& get() &&
  noexcept { return select<I>(mv(x), mv(y), mv(z), mv(w)); }

  /// performs `get` operation
  template<nat I> requires (I < 4)
  constexpr const vector&& get() const &&
  noexcept { return select<I>(mv(x), mv(y), mv(z), mv(w)); }

  /// obtains the reference to the specified row and column
  template<nat Row, nat Column> requires ((Row | Column) < 4)
  constexpr fat4& get() &
  noexcept { return get<Row>().get<Column>(); }

  /// obtains the reference to the specified row and column
  template<nat Row, nat Column> requires ((Row | Column) < 4)
  constexpr const fat4& get() const &
  noexcept { return get<Row>().get<Column>(); }

  /// obtains the reference to the specified row and column
  template<nat Row, nat Column> requires ((Row | Column) < 4)
  constexpr fat4&& get() &&
  noexcept { return mv(get<Row>().get<Column>()); }

  /// obtains the reference to the specified row and column
  template<nat Row, nat Column> requires ((Row | Column) < 4)
  constexpr const fat4&& get() const &&
  noexcept { return mv(get<Row>().get<Column>()); }

  /// number of elements
  constexpr nat size() const noexcept { return 4; }

  /// obtains the pointer to the first element
  constexpr vector* data() noexcept { return &x; }

  /// obtains the pointer to the first element
  constexpr const vector* data() const noexcept { return &x; }

  /// obtains the reference to the specified row
  constexpr vector& operator[](nat Row) noexcept {
    if (is_cev) {
      if (Row == 0) return x;
      else if (Row == 1) return y;
      else if (Row == 2) return z;
      else if (Row == 3) return w;
      else cannot_be_constant_evaluated();
    } else return (&x)[Row];
  }

  /// obtains the reference to the specified row
  constexpr const vector& operator[](nat Row) const noexcept {
    if (is_cev) {
      if (Row == 0) return x;
      else if (Row == 1) return y;
      else if (Row == 2) return z;
      else if (Row == 3) return w;
      else cannot_be_constant_evaluated();
    } else return (&x)[Row];
  }

  /// obtains the reference to the specified element
  constexpr fat4& operator()(nat Row, nat Column)
    noexcept { return operator[](Row)[Column]; }

  /// obtains the reference to the specified element
  constexpr const fat4& operator()(nat Row, nat Column) const
    noexcept { return operator[](Row)[Column]; }

  /// obtains the iterator to the first row
  constexpr vector* begin() noexcept { return data(); }

  /// obtains the iterator to the first row
  constexpr const vector* begin() const noexcept { return data(); }

  /// obtains the iterator to the end
  constexpr vector* end() noexcept { return data() + count; }

  /// obtains the iterator to the end
  constexpr const vector* end() const noexcept { return data() + count; }

  /// equality comparison
  friend constexpr bool operator==(const matrix& a, const matrix& b) noexcept
  { return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w; }

  /// three-way comparison
  friend constexpr auto operator<=>(const matrix& a, const matrix& b) noexcept {
    if (auto c = a.x <=> b.x; c != 0) return c;
    else if (c = a.y <=> b.y; c != 0) return c;
    else if (c = a.z <=> b.z; c != 0) return c;
    else return a.w <=> b.w;
  }

  /// unary plus
  friend constexpr matrix operator+(const matrix& a) noexcept { return a; }

  /// unary minus
  friend constexpr matrix operator-(const matrix& a) noexcept {
    return {-a.x, -a.y, -a.z, -a.w};
  }

  /// addition
  friend constexpr matrix operator+(const matrix& a, const matrix& b) noexcept {
    return {a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w};
  }

  /// subtraction
  friend constexpr matrix operator-(const matrix& a, const matrix& b) noexcept {
    return {a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w};
  }

  /// multiplication
  friend constexpr matrix operator*(const matrix& a, const matrix& b) noexcept {
    return {a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w};
  }

  /// multiplication
  friend constexpr matrix operator*(const matrix& a, const fat4& b) noexcept {
    return {a.x * b, a.y * b, a.z * b, a.w * b};
  }

  /// multiplication
  friend constexpr matrix operator*(const fat4& a, const matrix& b) noexcept {
    return b * a;
  }

  /// division
  friend constexpr matrix operator/(const matrix& a, const matrix& b) noexcept {
    return {a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w};
  }

  /// division
  friend constexpr matrix operator/(const matrix& a, const fat4& b) noexcept {
    return a * (1.f / b);
  }

  /// addition assignment
  constexpr matrix& operator+=(const matrix& a) noexcept {
    return x += a.x, y += a.y, z += a.z, w += a.w, *this;
  }

  /// subtraction assignment
  constexpr matrix& operator-=(const matrix& a) noexcept {
    return x -= a.x, y -= a.y, z -= a.z, w -= a.w, *this;
  }

  /// multiplication assignment
  constexpr matrix& operator*=(const matrix& a) noexcept {
    return x *= a.x, y *= a.y, z *= a.z, w *= a.w, *this;
  }

  /// multiplication assignment
  constexpr matrix& operator*=(const fat4& a) noexcept {
    return x *= a, y *= a, z *= a, w *= a, *this;
  }

  /// division assignment
  constexpr matrix& operator/=(const matrix& a) noexcept {
    return x /= a.x, y /= a.y, z /= a.z, w /= a.w, *this;
  }

  /// division assignment
  constexpr matrix& operator/=(const fat4& a) noexcept {
    return operator*=(1.f / a);
  }

  /// returns the transposed matrix
  constexpr matrix t() const noexcept {
    auto r0 = _mm_loadu_ps(x.data()), r1 = _mm_loadu_ps(y.data()),
         r2 = _mm_loadu_ps(z.data()), r3 = _mm_loadu_ps(w.data());

  }


};

} ////////////////////////////////////////////////////////////////////////////// namespace yw
