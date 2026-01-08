#pragma once
#include <chrono>
#include <cstdint>
#include <type_traits>
#include <utility>

namespace yw {

class stopwatch {
public:
  using clock = std::chrono::steady_clock;
  using time_point = clock::time_point;
  using duration = clock::duration;

  constexpr stopwatch() = default;

  void start() noexcept {
    if (running_) return;
    running_ = true;
    start_ = clock::now();
    last_lap_ = start_;
  }

  void stop() noexcept {
    if (!running_) return;
    const auto now = clock::now();
    accumulated_ += (now - start_);
    running_ = false;
  }

  void reset() noexcept {
    running_ = false;
    accumulated_ = duration::zero();
    start_ = time_point{};
    last_lap_ = time_point{};
  }

  void restart() noexcept {
    accumulated_ = duration::zero();
    running_ = true;
    start_ = clock::now();
    last_lap_ = start_;
  }

  bool running() const noexcept { return running_; }

  duration elapsed() const noexcept {
    if (!running_) return accumulated_;
    return accumulated_ + (clock::now() - start_);
  }

  duration lap() noexcept {
    if (!running_) return duration::zero();
    const auto now = clock::now();
    const auto d = now - last_lap_;
    last_lap_ = now;
    return d;
  }

  template<typename Dur> Dur elapsed_as() const noexcept {
    return std::chrono::duration_cast<Dur>(elapsed());
  }

  double seconds() const noexcept {
    return std::chrono::duration<double>(elapsed()).count();
  }
  std::int64_t milliseconds() const noexcept {
    return elapsed_as<std::chrono::milliseconds>().count();
  }
  std::int64_t microseconds() const noexcept {
    return elapsed_as<std::chrono::microseconds>().count();
  }
  std::int64_t nanoseconds() const noexcept {
    return elapsed_as<std::chrono::nanoseconds>().count();
  }

private:
  bool running_ = false;
  duration accumulated_ = duration::zero();
  time_point start_{};
  time_point last_lap_{};
};
} // namespace yw
