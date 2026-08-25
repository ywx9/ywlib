#pragma once
#include "ywx/core.h"

namespace yw {

class command_manager {
public:
  struct command {
    function<void> redo;
    function<void> undo;
  };

  struct group {
    std::vector<command> commands{};
  };

private:
  std::vector<group> _undo_stack{};
  std::vector<group> _redo_stack{};
  bool _replaying = false;
  size_t _limit = 256;
  stopwatch _grouping_timer;
  double _grouping_gap = 0.8;
  bool _last_command_is_groupable = false;

  void trim_undo_stack() {
    if (_limit != 0)
      while (_undo_stack.size() > _limit) _undo_stack.erase(_undo_stack.begin());
    else _undo_stack.clear();
  }

  void push_new(command cmd) {
    group g{};
    g.commands.push_back(std::move(cmd));
    _undo_stack.push_back(std::move(g));
    _redo_stack.clear();
    trim_undo_stack();
  }

public:
  bool is_replaying() const noexcept { return _replaying; }
  bool can_undo() const noexcept { return !_undo_stack.empty(); }
  bool can_redo() const noexcept { return !_redo_stack.empty(); }

  size_t limit() const noexcept { return _limit; }
  void limit(size_t value) { _limit = value, trim_undo_stack(); }

  double grouping_gap() const noexcept { return _grouping_gap; }
  void grouping_gap(double value) noexcept { _grouping_gap = value; }

  void end_grouping() noexcept { _grouping_timer.reset(); _last_command_is_groupable = false; }

  void clear() {
    _undo_stack.clear();
    _redo_stack.clear();
    _grouping_timer.reset();
    _last_command_is_groupable = false;
  }

  void push(command cmd, bool Groupable = false) {
    if (_replaying) return;
    if (!cmd.undo || !cmd.redo) return;
    const bool append = Groupable && _last_command_is_groupable && _grouping_timer.running() &&
                        _grouping_timer.elapsed() <= _grouping_gap;
    if (append) {
      _undo_stack.back().commands.push_back(std::move(cmd));
      _redo_stack.clear();
    } else push_new(std::move(cmd));
    if (Groupable) _grouping_timer.restart();
    else _grouping_timer.reset();
    _last_command_is_groupable = Groupable;
  }

  void push(function<void> redo_fn, function<void> undo_fn, bool Groupable = false) {
    command cmd{};
    cmd.redo = std::move(redo_fn);
    cmd.undo = std::move(undo_fn);
    push(std::move(cmd), Groupable);
  }

  void push(group g) {
    if (_replaying) return;
    if (g.commands.empty()) return;
    _undo_stack.push_back(std::move(g));
    _redo_stack.clear();
    trim_undo_stack();
    _grouping_timer.reset();
    _last_command_is_groupable = false;
  }

  bool undo() {
    _grouping_timer.reset();
    if (_undo_stack.empty()) return false;
    auto g = std::move(_undo_stack.back());
    _undo_stack.pop_back();
    _replaying = true;
    for (auto it = g.commands.rbegin(); it != g.commands.rend(); ++it)
      if (it->undo)
        if (auto res = it->undo(); !res) res.error().fizzle_out();
    _replaying = false;
    _redo_stack.push_back(std::move(g));
    return true;
  }

  bool redo() {
    _grouping_timer.reset();
    if (_redo_stack.empty()) return false;
    auto g = std::move(_redo_stack.back());
    _redo_stack.pop_back();
    _replaying = true;
    for (auto& cmd : g.commands)
      if (cmd.redo)
        if (auto res = cmd.redo(); !res) res.error().fizzle_out();
    _replaying = false;
    _undo_stack.push_back(std::move(g));
    trim_undo_stack();
    return true;
  }
};
} // namespace yw
