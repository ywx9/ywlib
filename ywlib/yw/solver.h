#pragma once
#include <yw/error.h>
#include <yw/vector.h>

namespace yw {

/// \note The diagonal elements of L are assumed to be 1.
template<floating T, size_t N> requires(N > 0) struct lu_decompose {
  matrix<T, N, N> lu;
  vector<size_t, N> pivots;
  /// \note `pivots[0] == npos` means that the matrix is singular.

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
        pivots[0] = npos;
        return;
      }
      if (pivot_row != k) std::swap(lu[k], lu[pivot_row]), std::swap(pivots[k], pivots[pivot_row]);
      for (size_t i = k + 1; i < N; ++i) {
        lu[i][k] /= lu[k][k];
        for (size_t j = k + 1; j < N; ++j) lu[i][j] -= lu[i][k] * lu[k][j];
      }
    }
  }
};

template<arithmetic T, size_t N> lu_decompose(const matrix<T, N, N>&) -> lu_decompose<math_type<T>, N>;

template<typename T, typename U, size_t N>
constexpr vector<math_type<T, U>, N> solve(const lu_decompose<T, N>& lu, const vector<U, N>& b) {
  if (lu.pivots[0] == npos) return vector<math_type<T, U>, N>{};
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
