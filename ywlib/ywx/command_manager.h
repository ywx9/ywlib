#pragma once
#include "ywx/ui_control.h"

namespace yw {

class command_manager {
public:
  struct command {
    function<void> redo;
    function<void> undo;
    std::wstring label{};
  };

  struct group {
    std::wstring label{};
    std::vector<command> commands{};
  };

private:
  std::vector<group> _undo_stack{};
  std::vector<group> _redo_stack{};
  std::optional<group> _building{};
  bool _replaying = false;
  size_t _limit = 256;

  void trim_undo_stack() {
    if (_limit == 0) {
      _undo_stack.clear();
      return;
    }
    while (_undo_stack.size() > _limit) _undo_stack.erase(_undo_stack.begin());
  }

public:
  bool is_replaying() const noexcept { return _replaying; }
  bool has_open_group() const noexcept { return _building.has_value(); }
  bool can_undo() const noexcept { return !_undo_stack.empty(); }
  bool can_redo() const noexcept { return !_redo_stack.empty(); }

  size_t limit() const noexcept { return _limit; }
  void limit(size_t value) {
    _limit = value;
    trim_undo_stack();
  }

  void clear() {
    _undo_stack.clear();
    _redo_stack.clear();
    _building.reset();
  }

  void begin_group(std::wstring label = {}) {
    if (_replaying) return;
    end_group();
    _building.emplace();
    _building->label = std::move(label);
  }

  void end_group() {
    if (!_building) return;
    if (!_building->commands.empty()) {
      _undo_stack.push_back(std::move(*_building));
      _redo_stack.clear();
      trim_undo_stack();
    }
    _building.reset();
  }

  void push(command cmd) {
    if (_replaying) return;
    if (!cmd.undo || !cmd.redo) return;
    if (_building) _building->commands.push_back(std::move(cmd));
    else {
      group g{};
      g.commands.push_back(std::move(cmd));
      _undo_stack.push_back(std::move(g));
      _redo_stack.clear();
      trim_undo_stack();
    }
  }

  template<stringable S> void push(function<void> redo_fn, function<void> undo_fn, S&& label) {
    command cmd{};
    cmd.redo = std::move(redo_fn);
    cmd.undo = std::move(undo_fn);
    cmd.label = unicode<wchar_t>(static_cast<S&&>(label));
    push(std::move(cmd));
  }

  void push(function<void> redo_fn, function<void> undo_fn) {
    command cmd{};
    cmd.redo = std::move(redo_fn);
    cmd.undo = std::move(undo_fn);
    push(std::move(cmd));
  }

  bool undo() {
    end_group();
    if (_undo_stack.empty()) return false;
    auto g = std::move(_undo_stack.back());
    _undo_stack.pop_back();
    _replaying = true;
    for (auto it = g.commands.rbegin(); it != g.commands.rend(); ++it)
      if (it->undo) it->undo();
    _replaying = false;
    _redo_stack.push_back(std::move(g));
    return true;
  }

  bool redo() {
    end_group();
    if (_redo_stack.empty()) return false;
    auto g = std::move(_redo_stack.back());
    _redo_stack.pop_back();
    _replaying = true;
    for (auto& cmd : g.commands)
      if (cmd.redo) cmd.redo();
    _replaying = false;
    _undo_stack.push_back(std::move(g));
    trim_undo_stack();
    return true;
  }
};
} // namespace yw
