#pragma once
#include <yw/error.h>
#include <yw/vector.h>

namespace yw {

/// MARK: sparse matrix

/// square sparse matrix in compressed row storage (CRS) format.
template<floating T, uint_type SizeType = size_t> class sparse_matrix {
  std::vector<T> _values;
  std::vector<SizeType> _column_indices;
  std::vector<SizeType> _row_offsets;
  SizeType _node_count{};
  SizeType _dof_per_node{};
  bool _node_major{};

  constexpr void _increment_row_offsets(size_t NextRow) {
    for (size_t i = NextRow; i < _row_offsets.size(); ++i) ++_row_offsets[i];
  }

  constexpr void _register_pattern(size_t Row, size_t Col) {
    const auto row_start = _row_offsets[Row];
    const auto row_end = _row_offsets[Row + 1];
    for (size_t i = row_start; i < row_end; ++i) {
      if (const auto ci = _column_indices[i]; ci > Col) {
        _column_indices.insert(_column_indices.begin() + i, Col);
        _increment_row_offsets(Row + 1);
        return;
      } else if (ci == Col) return; // already registered
    }
    _column_indices.insert(_column_indices.begin() + row_end, Col);
    _increment_row_offsets(Row + 1);
  }

  template<size_t N, contiguous_range<vector<uint32_t, N>> ElementsType> void _sparse_pattern(const ElementsType& Elements) {
    if (yw::size(Elements) == 0 || _node_count == 0 || _dof_per_node == 0) return;
    const auto n = _node_count * _dof_per_node;
    _column_indices.reserve(n);
    _row_offsets.resize(n + 1);
    if (_node_major)
      for (const auto& e : Elements)
        for (size_t ni = 0; ni < N; ++ni)
          for (size_t di = 0; di < _dof_per_node; ++di) {
            const auto row = e[ni] * _dof_per_node + di;
            for (size_t nj = 0; nj < N; ++nj)
              for (size_t dj = 0; dj < _dof_per_node; ++dj) {
                const auto col = e[nj] * _dof_per_node + dj;
                _register_pattern(row, col);
              }
          }
    else
      for (const auto& e : Elements)
        for (size_t ni = 0; ni < N; ++ni)
          for (size_t di = 0; di < _dof_per_node; ++di) {
            const auto row = di * _node_count + e[ni];
            for (size_t nj = 0; nj < N; ++nj)
              for (size_t dj = 0; dj < _dof_per_node; ++dj) {
                const auto col = dj * _node_count + e[nj];
                _register_pattern(row, col);
              }
          }
    _values.resize(_column_indices.size());
  }

public:
  size_t node_count() const noexcept { return _node_count; }
  size_t nonzero_count() const noexcept { return _values.size(); }
  uint32_t dof_per_node() const noexcept { return _dof_per_node; }
  /// returns `node_count() * dof_per_node()`
  size_t size() const noexcept { return _node_count * _dof_per_node; }
  bool node_major() const noexcept { return _node_major; }
  bool component_major() const noexcept { return !_node_major; }
  size_t dof_index(size_t Node, size_t Dof) const noexcept {
    return _node_major ? Node * _dof_per_node + Dof : Dof * _node_count + Node;
  }

  std::span<T> values() noexcept { return _values; }
  std::span<const T> values() const noexcept { return _values; }

  std::span<const SizeType> row_offsets() const noexcept { return _row_offsets; }
  std::span<const SizeType> column_indices() const noexcept { return _column_indices; }

  T operator()(size_t Row, size_t Col) const {
    const auto row_end = _row_offsets[Row + 1];
    for (size_t i = _row_offsets[Row]; i < row_end; ++i)
      if (_column_indices[i] == Col) return _values[i];
    return T(0);
  }

  T& at(size_t Row, size_t Col) {
    const auto row_end = _row_offsets[Row + 1];
    for (size_t i = _row_offsets[Row]; i < row_end; ++i)
      if (_column_indices[i] == Col) return _values[i];
    error(errors::invalid_argument, "sparse_matrix::at: element not found").go_off();
  }

  void add(size_t Row, size_t Col, const T& Value) {
    const auto row_end = _row_offsets[Row + 1];
    for (size_t i = _row_offsets[Row]; i < row_end; ++i)
      if (_column_indices[i] == Col) {
        _values[i] += Value;
        return;
      }
    error(errors::invalid_argument, "sparse_matrix::add: element not found").go_off();
  }

  void clear_values() noexcept { std::memset(_values.data(), 0, _values.size() * sizeof(T)); }

  sparse_matrix() = default;

  template<contiguous_range<uint2> LineSegmentsType>
  sparse_matrix(size_t NodeCount, uint1 DofPerNode, const LineSegmentsType& LineSegments, bool NodeMajor = true)
    : _node_count(NodeCount), _dof_per_node(DofPerNode.x), _node_major(NodeMajor) {
    _sparse_pattern<2>(LineSegments);
  }

  template<contiguous_range<uint3> TrianglesType>
  sparse_matrix(size_t NodeCount, uint1 DofPerNode, const TrianglesType& Triangles, bool NodeMajor = true)
    : _node_count(NodeCount), _dof_per_node(DofPerNode.x), _node_major(NodeMajor) {
    _sparse_pattern<3>(Triangles);
  }

  template<contiguous_range<uint4> TetrahedraType>
  sparse_matrix(size_t NodeCount, uint1 DofPerNode, const TetrahedraType& Tetrahedra, bool NodeMajor = true)
    : _node_count(NodeCount), _dof_per_node(DofPerNode.x), _node_major(NodeMajor) {
    _sparse_pattern<4>(Tetrahedra);
  }
};

template<floating T, uint_type SizeType, contiguous_range<T> In>
constexpr std::vector<T> transform(const sparse_matrix<T, SizeType>& m, const In& x) {
  const auto n = m.size();
  if (yw::size(x) != n) error(errors::invalid_argument, "input vector size mismatch").go_off();
  std::vector<T> y(n);
  const auto* in = yw::data(x);
  for (size_t row = 0; row < n; ++row) {
    const auto row_end = m.row_offsets()[row + 1];
    for (size_t i = m.row_offsets()[row]; i < row_end; ++i) {
      const auto col = m.column_indices()[i];
      y[row] += m.values()[i] * in[col];
    }
  }
  return y;
}

template<floating T, uint_type SizeType, contiguous_range<T> In, contiguous_output_range<T> Out>
constexpr void transform(const sparse_matrix<T, SizeType>& m, const In& x, Out& y) {
  const auto n = m.size();
  if (yw::size(x) != n) error(errors::invalid_argument, "input vector size mismatch").go_off();
  if (yw::size(y) != n) error(errors::invalid_argument, "output vector size mismatch").go_off();
  const auto* in = yw::data(x);
  auto* out = yw::data(y);
  for (size_t row = 0; row < n; ++row) {
    T sum = 0;
    const auto row_end = m.row_offsets()[row + 1];
    for (size_t i = m.row_offsets()[row]; i < row_end; ++i) {
      const auto col = m.column_indices()[i];
      sum += m.values()[i] * in[col];
    }
    out[row] = sum;
  }
}

template<floating T, contiguous_range<T> A, contiguous_range<T> B> constexpr T dot(const A& a, const B& b) {
  if (yw::size(a) != yw::size(b)) error(errors::invalid_argument, "vector size mismatch").go_off();
  T result = 0;
  const auto* pa = yw::data(a);
  const auto* pb = yw::data(b);
  for (size_t i = 0; i < yw::size(a); ++i) result += pa[i] * pb[i];
  return result;
}

template<floating T> struct dof_constraint {
  size_t dof{};
  T value{};
};

template<floating T> struct node_dof_constraint {
  size_t node{};
  size_t dof{};
  T value{};
};

template<floating T, uint_type SizeType, contiguous_range<T> B>
std::expected<std::vector<T>, error> solve_cg(
  const sparse_matrix<T, SizeType>& a,
  const B& b,
  T tolerance = T(1e-10),
  size_t max_iterations = 1000) {
  const auto n = a.size();
  if (yw::size(b) != n) return std::unexpected(error(errors::invalid_argument, "right hand side size mismatch"));
  if (tolerance <= T(0)) return std::unexpected(error(errors::invalid_argument, "tolerance must be positive"));

  std::vector<T> x(n);
  std::vector<T> r(yw::begin(b), yw::end(b));
  std::vector<T> p(r);
  std::vector<T> ap(n);

  auto rr = dot<T>(r, r);
  const auto bb = yw::max(dot<T>(b, b), T(1));
  const auto limit = tolerance * tolerance * bb;
  if (rr <= limit) return x;

  for (size_t iteration = 0; iteration < max_iterations; ++iteration) {
    transform(a, p, ap);
    const auto denom = dot<T>(p, ap);
    if (yw::abs(denom) <= std::numeric_limits<T>::epsilon())
      return std::unexpected(error(errors::operation_failed, "conjugate gradient breakdown"));

    const auto alpha = rr / denom;
    for (size_t i = 0; i < n; ++i) {
      x[i] += alpha * p[i];
      r[i] -= alpha * ap[i];
    }

    const auto rr_next = dot<T>(r, r);
    if (rr_next <= limit) return x;

    const auto beta = rr_next / rr;
    for (size_t i = 0; i < n; ++i) p[i] = r[i] + beta * p[i];
    rr = rr_next;
  }

  return std::unexpected(error(errors::operation_failed, "conjugate gradient did not converge"));
}

template<floating T, uint_type SizeType, contiguous_output_range<T> B>
std::expected<void, error> constrain(sparse_matrix<T, SizeType>& a, B& b, size_t Dof, T Value = T(0)) {
  const auto n = a.size();
  if (yw::size(b) != n) return std::unexpected(error(errors::invalid_argument, "right hand side size mismatch"));
  if (Dof >= n) return std::unexpected(error(errors::invalid_argument, "degree of freedom out of range"));
  auto* rhs = yw::data(b);

  bool has_diagonal = false;
  for (size_t i = a.row_offsets()[Dof]; i < a.row_offsets()[Dof + 1]; ++i)
    if (a.column_indices()[i] == Dof) {
      has_diagonal = true;
      break;
    }
  if (!has_diagonal) return std::unexpected(error(errors::invalid_argument, "constrained diagonal element not found"));

  for (size_t row = 0; row < n; ++row) {
    const auto row_end = a.row_offsets()[row + 1];
    for (size_t i = a.row_offsets()[row]; i < row_end; ++i) {
      const auto col = a.column_indices()[i];
      if (row == Dof) a.values()[i] = col == Dof ? T(1) : T(0);
      else if (col == Dof) {
        rhs[row] -= a.values()[i] * Value;
        a.values()[i] = T(0);
      }
    }
  }
  rhs[Dof] = Value;
  return {};
}

template<floating T, uint_type SizeType, contiguous_output_range<T> B, contiguous_range<dof_constraint<T>> Constraints>
std::expected<void, error> constrain(sparse_matrix<T, SizeType>& a, B& b, const Constraints& constraints) {
  for (const auto& c : constraints)
    if (auto res = constrain(a, b, c.dof, c.value); !res) return res.error().relay();
  return {};
}

template<floating T, uint_type SizeType, contiguous_output_range<T> B>
std::expected<void, error> constrain(
  sparse_matrix<T, SizeType>& a,
  B& b,
  size_t Node,
  size_t Dof,
  T Value = T(0)) {
  if (Node >= a.node_count()) return std::unexpected(error(errors::invalid_argument, "node index out of range"));
  if (Dof >= a.dof_per_node()) return std::unexpected(error(errors::invalid_argument, "degree of freedom out of range"));
  return constrain(a, b, a.dof_index(Node, Dof), Value);
}

template<floating T, uint_type SizeType, contiguous_output_range<T> B, contiguous_range<node_dof_constraint<T>> Constraints>
std::expected<void, error> constrain(sparse_matrix<T, SizeType>& a, B& b, const Constraints& constraints) {
  for (const auto& c : constraints)
    if (auto res = constrain(a, b, c.node, c.dof, c.value); !res) return res.error().relay();
  return {};
}

template<size_t NodeCount, size_t DofPerNode, floating T, uint_type SizeType>
std::expected<void, error> add_element_matrix(
  sparse_matrix<T, SizeType>& a,
  const vector<uint32_t, NodeCount>& nodes,
  const matrix<T, NodeCount * DofPerNode, NodeCount * DofPerNode>& element_matrix) {
  if (DofPerNode != a.dof_per_node())
    return std::unexpected(error(errors::invalid_argument, "element degree of freedom count mismatch"));

  for (size_t ni = 0; ni < NodeCount; ++ni)
    if (nodes[ni] >= a.node_count()) return std::unexpected(error(errors::invalid_argument, "node index out of range"));

  for (size_t ni = 0; ni < NodeCount; ++ni)
    for (size_t di = 0; di < DofPerNode; ++di) {
      const auto row = a.dof_index(nodes[ni], di);
      const auto local_row = ni * DofPerNode + di;
      for (size_t nj = 0; nj < NodeCount; ++nj)
        for (size_t dj = 0; dj < DofPerNode; ++dj) {
          const auto col = a.dof_index(nodes[nj], dj);
          const auto local_col = nj * DofPerNode + dj;
          a.add(row, col, element_matrix[local_row][local_col]);
        }
    }

  return {};
}

/// \note The diagonal elements of L are assumed to be 1.
template<floating T, size_t N> requires(N > 0) struct lu_decompose {
  matrix<T, N, N> lu;
  vector<size_t, N> pivots;
  bool success = false;

  template<typename U = T> constexpr lu_decompose(const matrix<U, N, N>& m) : lu(m) {
    for (size_t i = 0; i < N; ++i) pivots[i] = i;
    for (size_t k = 0; k < N; ++k) {
      size_t pivot_row = k;
      T pivot_value = yw::abs(lu[k][k]);
      for (size_t i = k + 1; i < N; ++i) {
        const auto value = yw::abs(lu[i][k]);
        if (value > pivot_value) pivot_row = i, pivot_value = value;
      }
      if (pivot_value <= std::numeric_limits<T>::epsilon()) {
        success = false;
        return;
      }
      if (pivot_row != k) std::swap(lu[k], lu[pivot_row]), std::swap(pivots[k], pivots[pivot_row]);
      for (size_t i = k + 1; i < N; ++i) {
        lu[i][k] /= lu[k][k];
        for (size_t j = k + 1; j < N; ++j) lu[i][j] -= lu[i][k] * lu[k][j];
      }
    }
    success = true;
  }
};

template<arithmetic T, size_t N> lu_decompose(const matrix<T, N, N>&) -> lu_decompose<math_type<T>, N>;

template<typename T, typename U, size_t N>
constexpr vector<math_type<T, U>, N> solve(const lu_decompose<T, N>& lu, const vector<U, N>& b) {
  if (!lu.success) return vector<math_type<T, U>, N>{};
  vector<math_type<T, U>, N> x;
  for (size_t i = 0; i < N; ++i) x[i] = b[lu.pivots[i]];
  for (size_t i = 0; i < N; ++i)
    for (size_t j = 0; j < i; ++j) x[i] -= lu.lu[i][j] * x[j];
  for (size_t i = N; i-- > 0;) {
    for (size_t j = i + 1; j < N; ++j) x[i] -= lu.lu[i][j] * x[j];
    x[i] /= lu.lu[i][i];
  }
  return x;
}

template<typename T, typename U, size_t N>
constexpr vector<math_type<T, U>, N> solve(const matrix<T, N, N>& m, const vector<U, N>& b) {
  return solve(lu_decompose(m), b);
}

template<typename Derivative, typename State, typename Time> concept integrable =
  requires(Derivative derivative, State& state, const State& cstate, Time t) {
    state += t * derivative(cstate, t);
    requires same_as<decltype(derivative(cstate, t)), decltype(derivative(cstate + t* derivative(cstate, t), t))>;
    { derivative(cstate, t) + derivative(cstate, t) } -> same_as<decltype(derivative(cstate, t))>;
    { 2 * derivative(cstate, t) } -> same_as<decltype(derivative(cstate, t))>;
    { t + t } -> convertible_to<Time>;
    { t * double() } -> convertible_to<Time>;
  };

inline constexpr auto integrate_euler = //
  []<typename State, typename Time, integrable<State, Time> F>(
    State& state, Time t, Time dt, F&& derivative) { state += dt * derivative(state, t); };

inline constexpr auto integrate_rk2 = //
  []<typename State, typename Time, integrable<State, Time> F>(State& state, Time t, Time dt, F&& derivative) {
    const auto k1 = derivative(state, t);
    const auto k2 = derivative(state + (dt * 0.5) * k1, t + dt * 0.5);
    state += dt * k2;
  };

inline constexpr auto integrate_rk4 = //
  []<typename State, typename Time, integrable<State, Time> F>(State& state, Time t, Time dt, F&& derivative) {
    const auto k1 = derivative(state, t);
    const auto k2 = derivative(state + (dt * 0.5) * k1, t + dt * 0.5);
    const auto k3 = derivative(state + (dt * 0.5) * k2, t + dt * 0.5);
    const auto k4 = derivative(state + dt * k3, t + dt);
    state += (dt * double(1.0 / 6.0)) * (k1 + 2 * k2 + 2 * k3 + k4);
  };

template<typename Derivative, typename State, typename Time> concept integrable_inplace =
  std::ranges::contiguous_range<State> && requires(Derivative derivative, State& state, Time t) {
    { state.buffer(size_t{}) } -> same_as<std::span<iter_value_t<State>, std::dynamic_extent>>;
    derivative(state, state.buffer(size_t{}), t);
    derivative(state.buffer(size_t{}), state.buffer(size_t{}), t);
    std::declval<iter_reference_t<State>>() += t * std::declval<iter_reference_t<decltype(state.buffer(size_t{}))>>();
  };

inline constexpr auto integrate_euler_inplace = //
  []<typename State, typename Time, integrable_inplace<State&, Time> F>(
    State& state, Time t, Time dt, F&& derivative) -> std::expected<void, error> {
  const auto n = std::ranges::size(state);
  auto buffer = state.buffer(0);
  if (n == 0 || n != std::ranges::size(buffer))
    return std::unexpected(error(errors::invalid_argument, "State and buffer must have the same size"));
  derivative(state, buffer, t);
  auto si = std::ranges::begin(state);
  auto bi = std::ranges::begin(buffer);
  for (size_t i = 0; i < n; ++i, ++si, ++bi) *si += dt * *bi;
  return {};
};

inline constexpr auto integrate_rk2_inplace = //
  []<typename State, typename Time, integrable_inplace<State&, Time> F>(
    State& state, Time t, Time dt, F&& derivative) -> std::expected<void, error> {
  const auto n = std::ranges::size(state);
  auto buffer0 = state.buffer(0);
  auto buffer1 = state.buffer(1);
  if (n == 0 || n != std::ranges::size(buffer0) || n != std::ranges::size(buffer1))
    return std::unexpected(error(errors::invalid_argument, "State and buffer must have the same size"));
  derivative(state, buffer1, t);
  auto si = std::ranges::begin(state);
  auto b0i = std::ranges::begin(buffer0);
  auto b1i = std::ranges::begin(buffer1);
  for (size_t i = 0; i < n; ++i, ++si, ++b0i, ++b1i) *b0i = *si + (dt * 0.5) * *b1i;
  derivative(buffer0, buffer1, t + dt * 0.5);
  si = std::ranges::begin(state);
  b1i = std::ranges::begin(buffer1);
  for (size_t i = 0; i < n; ++i, ++si, ++b1i) *si += dt * *b1i;
  return {};
};

/// \note requires three buffers in the state.
inline constexpr auto integrate_rk4_inplace = //
  []<typename State, typename Time, integrable_inplace<State&, Time> F>(
    State& state, Time t, Time dt, F&& derivative) -> std::expected<void, error> {
  const auto n = std::ranges::size(state);
  auto buffer0 = state.buffer(0);
  auto buffer1 = state.buffer(1);
  auto buffer2 = state.buffer(2);
  if (n == 0 || n != std::ranges::size(buffer0) || n != std::ranges::size(buffer1) || n != std::ranges::size(buffer2))
    return std::unexpected(error(errors::invalid_argument, "State and buffer must have the same size"));
  derivative(state, buffer1, t);
  auto si = std::ranges::begin(state);
  auto b0i = std::ranges::begin(buffer0);
  auto b1i = std::ranges::begin(buffer1);
  for (size_t i = 0; i < n; ++i, ++si, ++b0i, ++b1i) *b0i = *si + (dt * 0.5) * *b1i;
  derivative(buffer0, buffer2, t + dt * 0.5);
  si = std::ranges::begin(state);
  b0i = std::ranges::begin(buffer0);
  b1i = std::ranges::begin(buffer1);
  auto b2i = std::ranges::begin(buffer2);
  for (size_t i = 0; i < n; ++i, ++si, ++b0i, ++b1i, ++b2i) *b0i = *si + (dt * 0.5) * *b2i, *b1i += 2 * *b2i;
  derivative(buffer0, buffer2, t + dt * 0.5);
  si = std::ranges::begin(state);
  b0i = std::ranges::begin(buffer0);
  b1i = std::ranges::begin(buffer1);
  b2i = std::ranges::begin(buffer2);
  for (size_t i = 0; i < n; ++i, ++si, ++b0i, ++b1i, ++b2i) *b0i = *si + dt * *b2i, *b1i += 2 * *b2i;
  derivative(buffer0, buffer2, t + dt);
  si = std::ranges::begin(state);
  b1i = std::ranges::begin(buffer1);
  b2i = std::ranges::begin(buffer2);
  for (size_t i = 0; i < n; ++i, ++si, ++b1i, ++b2i) *si += (dt * double(1.0 / 6.0)) * (*b1i + *b2i);
  return {};
};

template<size_t Dim> requires(0 < Dim && Dim < 4) inline constexpr auto explicit_euler_stable_dt =
  [](double1 dx, floating auto alpha, double1 safety = 0.9) { return safety.x * dx.x * dx.x / (2 * Dim * alpha); };
} // namespace yw
