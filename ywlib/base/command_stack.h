#pragma once
#include <core/function.h>
#include <core/rolling_stack.h>

namespace yw {

class command_stack {
public:
  struct grouped_redo {
    function<result<void>> old_redo;
    function<result<void>> new_redo;
    constexpr result<void> operator()() {
      if (old_redo)
        if (auto res = old_redo(); !res) return res.relay();
      if (new_redo)
        if (auto res = new_redo(); !res) return res.relay();
      return {};
    }
  };

  struct grouped_undo {
    function<result<void>> old_undo;
    function<result<void>> new_undo;
    constexpr result<void> operator()() {
      if (new_undo)
        if (auto res = new_undo(); !res) return res.relay();
      if (old_undo)
        if (auto res = old_undo(); !res) return res.relay();
      return {};
    }
  };

  struct command {
    std::chrono::steady_clock::time_point timestamp;
    function<result<void>> redo;
    function<result<void>> undo;
  };

protected:
  rolling_stack<command> _commands{};
  size_t _current_index = 0;
  std::chrono::duration<double> _grouping_interval{0.5};

public:
  constexpr command_stack() = default;
  constexpr command_stack(size_t undo_limit) : _commands(undo_limit) {}

  constexpr command_stack(const command_stack&) = default;
  constexpr command_stack& operator=(const command_stack&) = default;

  constexpr command_stack(command_stack&& o) noexcept
    : _commands(move(o._commands)), _current_index(exchange(o._current_index, 0)),
      _grouping_interval(o._grouping_interval) {}

  constexpr command_stack& operator=(command_stack&& o) noexcept {
    if (this == &o) return *this;
    _commands = move(o._commands);
    _current_index = exchange(o._current_index, 0);
    _grouping_interval = o._grouping_interval;
    return *this;
  }

  constexpr size_t undo_limit() const noexcept { return _commands.capacity(); }
  constexpr void undo_limit(size_t limit) {
    const auto old_size = _commands.size();
    _commands.set_capacity(limit);
    const auto skipped = old_size - _commands.size();
    _current_index -= yw::min(_current_index, skipped);
  }

  constexpr size_t command_count() const noexcept { return _commands.size(); }
  // Grouping interval in seconds.
  constexpr double grouping_interval() const noexcept { return _grouping_interval.count(); }
  constexpr void grouping_interval(double interval) noexcept {
    _grouping_interval = std::chrono::duration<double>(interval);
  }
  constexpr bool can_redo() const noexcept { return _current_index < _commands.size(); }
  constexpr bool can_undo() const noexcept { return _current_index > 0; }
  constexpr size_t redo_count() const noexcept { return _commands.size() - _current_index; }
  constexpr size_t undo_count() const noexcept { return _current_index; }

  constexpr void clear() noexcept {
    _commands.clear();
    _current_index = 0;
  }

  // Register an already executed command, discarding any redo history.
  command_stack& push(function<result<void>> Redo, function<result<void>> Undo, bool TryGrouping = false) {
    if (undo_limit() == 0) return *this;
    const auto timestamp = std::chrono::steady_clock::now();
    const bool had_redo = can_redo();
    while (_commands.size() > _current_index) _commands.pop();
    if (TryGrouping && !had_redo && !_commands.empty() &&
        timestamp - _commands[_current_index - 1].timestamp <= _grouping_interval) {
      auto& previous = _commands[_current_index - 1];
      previous.redo = grouped_redo{move(previous.redo), move(Redo)};
      previous.undo = grouped_undo{move(previous.undo), move(Undo)};
      previous.timestamp = timestamp;
    } else {
      _commands.emplace(timestamp, move(Redo), move(Undo));
    }
    _current_index = _commands.size();
    return *this;
  }

  constexpr result<void> redo() {
    if (!can_redo()) return {};
    auto result = _commands[_current_index].redo();
    if (!result) return result.relay();
    ++_current_index;
    return {};
  }
  constexpr result<void> undo() {
    if (!can_undo()) return {};
    auto result = _commands[_current_index - 1].undo();
    if (!result) return result.relay();
    --_current_index;
    return {};
  }
};
} // namespace yw
