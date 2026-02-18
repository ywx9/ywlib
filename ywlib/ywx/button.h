#pragma once
#include "ywx/label.h"

namespace yw::control {

class button : public label {
public:
  class slot : public label::slot {
  public:
    function<void> on_left_down;
    function<void> on_left_up;
    function<void> on_middle_down;
    function<void> on_middle_up;
    function<void> on_right_down;
    function<void> on_right_up;
    function<void> on_drag;
    function<void> on_wheel;
  };

  class agent {
    function<void>* _dest = nullptr;
  public:
    agent() noexcept = default;
    agent(function<void>& Dest) noexcept : _dest(&Dest) {}
    operator function<void>&() const noexcept { return *_dest; }
    template<typename F> requires std::is_assignable_v<function<void>&, F&&>
    void operator=(F&& func) {
      if (_dest) *_dest = static_cast<F&&>(func);
    }
  };

private:
  slot* _button(window::slot* window_slot) const noexcept {
    return dynamic_cast<slot*>(window_slot->controls.get(_id.control));
  }
  slot* _button() const noexcept {
    const auto window_slot = _window();
    return window_slot ? _button(window_slot) : nullptr;
  }

public:
  using base::base;
  using base::operator bool;

  agent on_left_down() const noexcept {
    if (const auto button_slot = _button()) return agent(button_slot->on_left_down);
    return {};
  }

  agent on_left_up() const noexcept {
    if (const auto button_slot = _button()) return agent(button_slot->on_left_up);
    return {};
  }

  agent on_middle_down() const noexcept {
    if (const auto button_slot = _button()) return agent(button_slot->on_middle_down);
    return {};
  }

  agent on_middle_up() const noexcept {
    if (const auto button_slot = _button()) return agent(button_slot->on_middle_up);
    return {};
  }

  agent on_right_down() const noexcept {
    if (const auto button_slot = _button()) return agent(button_slot->on_right_down);
    return {};
  }

  agent on_right_up() const noexcept {
    if (const auto button_slot = _button()) return agent(button_slot->on_right_up);
    return {};
  }

  agent on_drag() const noexcept {
    if (const auto button_slot = _button()) return agent(button_slot->on_drag);
    return {};
  }

  agent on_wheel() const noexcept {
    if (const auto button_slot = _button()) return agent(button_slot->on_wheel);
    return {};
  }

  static std::expected<button, error_trace> add(window::slave& window, float2 pos, float2 size) {
    if (auto res = )
  }
};
}
