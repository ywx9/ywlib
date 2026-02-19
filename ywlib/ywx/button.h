#pragma once
#include "ywx/label.h"

namespace yw::control {

/// simple push button control
class button : public label {
public:
  class slot : public label::slot {
  public:
    function<void, const slot&, float2> on_down;
    function<void, const slot&, float2> on_up;
    function<void, const slot&, float2> on_click; // called after on_up if the cursor is still inside the button
    function<void, const slot&, bool> on_focus;   // called when the button is focused or unfocused
    function<void, const slot&> on_cancel;        // called when the button loses focus during pressing

    bool pressed = false;

    slot(const window::slotid& window_id, float2 Pos, float2 Size) : label::slot(window_id, Pos, Size) {}

    virtual std::expected<bool, error_trace> proc(UINT msg, WPARAM wparam, LPARAM lparam) override {
      const auto window_slot = _window();
      if (!window_slot) return unexpected_error(errors::operation_failed, "window slot not found");
      const auto pt = float2(std::bit_cast<short2>(static_cast<uint32_t>(std::bit_cast<size_t>(lparam))));
      switch (msg) {
      case WM_LBUTTONDOWN:
        if (hit_test(pt)) {
          if (on_left_down) on_left_down(*this, pt);
          return true;
        } else {
          for (auto& control_slot : window_slot->controls) {
            if (control_slot.hit_test(pt)) {
              window_slot->focused_control = control_slot.id.control;
              return control_slot.proc(msg);
            }
          }
          window_slot->focused_control = {};
          return false;
        }
      case WM_LBUTTONUP:
        // ...
      }
      return false;
    }
  };

protected:
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

  auto on_left_down() noexcept {
    using func_slot = typename function<void, const slot&, float2>::slot;
    if (const auto button_slot = _button()) return func_slot(button_slot->on_left_down);
    return func_slot();
  }

  auto on_left_up() const noexcept {
    using func_slot = typename function<void, const slot&, float2>::slot;
    if (const auto button_slot = _button()) return func_slot(button_slot->on_left_up);
    return func_slot();
  }

  auto on_middle_down() const noexcept {
    using func_slot = typename function<void, const slot&, float2>::slot;
    if (const auto button_slot = _button()) return func_slot(button_slot->on_middle_down);
    return func_slot();
  }

  auto on_middle_up() const noexcept {
    using func_slot = typename function<void, const slot&, float2>::slot;
    if (const auto button_slot = _button()) return func_slot(button_slot->on_middle_up);
    return func_slot();
  }

  auto on_right_down() const noexcept {
    using func_slot = typename function<void, const slot&, float2>::slot;
    if (const auto button_slot = _button()) return func_slot(button_slot->on_right_down);
    return func_slot();
  }

  auto on_right_up() const noexcept {
    using func_slot = typename function<void, const slot&, float2>::slot;
    if (const auto button_slot = _button()) return func_slot(button_slot->on_right_up);
    return func_slot();
  }

  auto on_drag() const noexcept {
    using func_slot = typename function<void, const slot&, float2>::slot;
    if (const auto button_slot = _button()) return func_slot(button_slot->on_drag);
    return func_slot();
  }

  auto on_wheel() const noexcept {
    using func_slot = typename function<void, const slot&, float2, int>::slot;
    if (const auto button_slot = _button()) return func_slot(button_slot->on_wheel);
    return func_slot();
  }

  static std::expected<button, error_trace> add(window::slave& window, float2 pos, float2 size) {
    const auto window_slot = yw::window::system.get_window(window);
    if (!window_slot) return unexpected_error(errors::operation_failed, "window slot not found");
    auto button_slot = std::make_unique<slot>(window_slot->id, pos, size);
    const auto button_id = window_slot->controls.push(std::move(button_slot));
    const auto control_slot = window_slot->controls.get(button_id);
    if (!control_slot) return unexpected_error(errors::operation_failed, "failed to get control slot");
    control_slot->id.control = button_id;
    return button({window_slot->id.master, button_id, button_id});
  }
};
}
