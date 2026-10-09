// #pragma once
// #include <core/rolling_stack.h>
// #include <core/function.h>

// namespace yw {

// class command_stack {
// public:
//   struct command {
//     function<result<void>> redo;
//     function<result<void>> undo;
//   };

// protected:
//   rolling_stack<command> _commands{};

// public:
//   constexpr command_stack() = default;
//   constexpr command_stack(size_t undo_limit) : _commands(undo_limit) {}

//   constexpr size_t undo_limit() const noexcept { return _commands.capacity(); }
//   constexpr void undo_limit(size_t limit) { _commands.set_capacity(limit); }

//   constexpr size_t command_count() const noexcept { return _commands.size(); }
//   constexpr bool can_redo() const noexcept { return _current_index < _commands.size(); }
//   constexpr bool can_undo() const noexcept { return !_commands.empty(); }
//   constexpr size_t redo_count() const noexcept { return _commands.size() - _current_index; }
//   constexpr size_t undo_count() const noexcept { return _current_index; }

//   constexpr void redo() {
//     if (!can_redo()) return;
//     _commands[_current_index].redo();
//     ++_current_index;
//   }
//   constexpr void undo() {
//     if (!can_undo()) return;
//     --_current_index;
//     _commands[_current_index].undo();
//   }

// };
// } // namespace yw
