#pragma once
#include "ywx/label.h"

namespace yw::control {

/// simple push button control
class button : public label {
public:
  class slot : public label::slot {
  public:
    function<void, button&, float2> on_down;
    function<void, button&, float2> on_up;
    function<void, button&, float2> on_click; // called after on_up if the cursor is still inside the button
    function<void, button&, bool> on_focus;   // called when the button is focused or unfocused
    function<void, button&> on_cancel;        // called when the button loses focus during pressing

    bool pressed = false;

    slot(const window::slotid& window_id, float2 Pos, float2 Size) : label::slot(window_id, Pos, Size) {}

    virtual std::expected<bool, error_trace> proc(UINT msg, WPARAM wparam, LPARAM lparam) override {
      return true;
    }
  };

  static constexpr auto kind = source().unique_id();

protected:
  slot* _button(window::slot* window_slot) const noexcept {
    return dynamic_cast<slot*>(window_slot->controls.get(_id.control));
  }

  slot* _button() const noexcept {
    const auto window_slot = _window();
    return window_slot ? _button(window_slot) : nullptr;
  }

public:
  using label::label;
  using label::operator bool;

  auto on_down() noexcept {
    using func_slot = typename function<void, button&, float2>::slot;
    if (const auto button_slot = _button()) return func_slot(button_slot->on_down);
    return func_slot();
  }

  auto on_up() noexcept {
    using func_slot = typename function<void, button&, float2>::slot;
    if (const auto button_slot = _button()) return func_slot(button_slot->on_up);
    return func_slot();
  }

  auto on_click() noexcept {
    using func_slot = typename function<void, button&, float2>::slot;
    if (const auto button_slot = _button()) return func_slot(button_slot->on_click);
    return func_slot();
  }

  auto on_focus() noexcept {
    using func_slot = typename function<void, button&, bool>::slot;
    if (const auto button_slot = _button()) return func_slot(button_slot->on_focus);
    return func_slot();
  }

  auto on_cancel() noexcept {
    using func_slot = typename function<void, button&>::slot;
    if (const auto button_slot = _button()) return func_slot(button_slot->on_cancel);
    return func_slot();
  }

  static std::expected<button, error_trace> add(window::slave& window, float2 pos, float2 size) {
    return base::add<button>(window, pos, size);
  }
};
}
