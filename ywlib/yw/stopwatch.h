#pragma once
#include <chrono>
#include <cstdint>
#include <type_traits>
#include <utility>

namespace yw {

using hours = std::chrono::hours;
using minutes = std::chrono::minutes;
using seconds = std::chrono::seconds;
using milliseconds = std::chrono::milliseconds;
using microseconds = std::chrono::microseconds;
using nanoseconds = std::chrono::nanoseconds;

class stopwatch {
  bool _running = false;
  std::chrono::steady_clock::duration _accum = std::chrono::steady_clock::duration::zero();
  std::chrono::steady_clock::time_point _start{}, _last{};

public:
  using clock = std::chrono::steady_clock;
  using time_point = clock::time_point;
  using duration = clock::duration;

  constexpr stopwatch() = default;

  void start() noexcept {
    if (_running) return;
    _running = true;
    _last = _start = clock::now();
  }

  void stop() noexcept {
    if (!_running) return;
    _running = false;
    const auto now = clock::now();
    _accum += (now - _start);
  }

  void reset() noexcept {
    _running = false;
    _accum = duration::zero();
    _start = time_point{};
    _last = time_point{};
  }

  void restart() noexcept {
    _running = true;
    _accum = duration::zero();
    _last = _start = clock::now();
  }

  bool running() const noexcept { return _running; }

  duration elapsed() const noexcept {
    return _accum + (_running ? (clock::now() - _start) : duration::zero());
  }

  duration lap() noexcept {
    if (!_running) return duration::zero();
    const auto now = clock::now();
    const auto d = now - _last;
    _last = now;
    return d;
  }

  template<typename Dur> Dur elapsed_as() const noexcept {
    return std::chrono::duration_cast<Dur>(elapsed());
  }

  double seconds() const noexcept { return std::chrono::duration<double>(elapsed()).count(); }
  std::int64_t milliseconds() const noexcept { return elapsed_as<yw::milliseconds>().count(); }
  std::int64_t microseconds() const noexcept { return elapsed_as<yw::microseconds>().count(); }
  std::int64_t nanoseconds() const noexcept { return elapsed_as<yw::nanoseconds>().count(); }
};
} // namespace yw
