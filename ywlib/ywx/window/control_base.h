#pragma once
#include "ywx/window/slave.h"

namespace yw::control {

class base {
  static uint64_t _get_counter() noexcept {
    static uint64_t counter = 1;
    return counter++;
  }

public:
  using slot = window::control_slot;

protected:
  window::control_slotid _id;
  uint64_t _counter{};

  base(window::control_slotid id) noexcept : _id(id), _counter(_get_counter()) {}

  window::slot* _window() const noexcept { return _id.get_window(); }
  window::control_slot* _control() const noexcept { return _id.get_control(); }

  // template<typename Ctrl, derived_from<slot> Slot>
  // static std::expected<Ctrl, error_trace> _add(window::slave& w, float2 Pos, float2 Size) {
  //   if (const auto window_slot = w._window()) {
  //     window_slot->dirty = true;
  //     auto control_slot = std::make_unique<Slot>();
  //     control_slot->id.master = window_slot->id.master;
  //     control_slot->id.slave = window_slot->id.slave;
  //     control_slot->position = Pos;
  //     control_slot->size = Size;
  //     const auto cid = window_slot->controls.push(std::move(control_slot));
  //     return Ctrl({window_slot->id.master, window_slot->id.slave, cid});
  //   } else return unexpected_error(errors::invalid_argument, "invalid window");
  // }

public:
  base() noexcept = default;
  base(base&&) noexcept = default;
  base& operator=(base&&) noexcept = default;

  base(const base&) = delete;
  base& operator=(const base&) = delete;

  explicit operator bool() const noexcept {
    const auto w = _window();
    return w && w->controls.contains(_id.control);
  }

  uint64_t counter() const noexcept { return _counter; }

  float2 position() const noexcept {
    const auto control_slot = _control();
    return control_slot ? control_slot->position : float2{};
  }

  float2 size() const noexcept {
    const auto control_slot = _control();
    return control_slot ? control_slot->size : float2{};
  }

  float2 radius() const noexcept {
    const auto control_slot = _control();
    return control_slot ? control_slot->radius : float2{};
  }

  color background_color() const noexcept {
    const auto control_slot = _control();
    return control_slot ? control_slot->background_color : color{};
  }

  color border_color() const noexcept {
    const auto control_slot = _control();
    return control_slot ? control_slot->border_color : color{};
  }

  float border_width() const noexcept {
    const auto control_slot = _control();
    return control_slot ? control_slot->border_width : 0.0f;
  }

  bool visible() const noexcept {
    const auto control_slot = _control();
    return control_slot ? control_slot->visible : false;
  }

  bool enabled() const noexcept {
    const auto control_slot = _control();
    return control_slot ? control_slot->enabled : false;
  }

  void position(float2 p) noexcept {
    if (const auto window_slot = _window())
      if (const auto control_slot = _control()) {
        window_slot->dirty = true;
        control_slot->position = p;
      }
  }

  void size(float2 s) noexcept {
    if (const auto window_slot = _window())
      if (const auto control_slot = _control()) {
        window_slot->dirty = true;
        control_slot->size = s;
      }
  }

  void radius(float2 r) noexcept {
    if (const auto window_slot = _window())
      if (const auto control_slot = _control()) {
        window_slot->dirty = true;
        control_slot->radius = r;
      }
  }

  void background_color(const color& c) noexcept {
    if (const auto window_slot = _window())
      if (const auto control_slot = _control()) {
        window_slot->dirty = true;
        control_slot->background_color = c;
      }
  }

  void border_color(const color& c) noexcept {
    if (const auto window_slot = _window())
      if (const auto control_slot = _control()) {
        window_slot->dirty = true;
        control_slot->border_color = c;
      }
  }

  void border_width(float w) noexcept {
    if (const auto window_slot = _window())
      if (const auto control_slot = _control()) {
        window_slot->dirty = true;
        control_slot->border_width = w;
      }
  }

  void visible(bool b) noexcept {
    if (const auto window_slot = _window())
      if (const auto control_slot = _control()) {
        window_slot->dirty = true;
        control_slot->visible = b;
      }
  }

  void enabled(bool e) noexcept {
    if (const auto window_slot = _window())
      if (const auto control_slot = _control()) {
        window_slot->dirty = true;
        control_slot->enabled = e;
      }
  }

  static std::expected<base, error_trace> add(window::slave& w, float2 Pos, float2 Size) {
    const auto window_slot = window::system.get_window(w);
    if (!window_slot) return unexpected_error(errors::invalid_argument, "invalid window");
    auto base_slot = std::make_unique<window::control_slot>(window_slot->id, Pos, Size);
    const auto cid = window_slot->controls.push(std::move(base_slot));
    const auto control_slot = window_slot->controls.get(cid);
    if (!control_slot) return unexpected_error(errors::operation_failed, "failed to create control slot");
    control_slot->id.control = cid;
    return base({window_slot->id.master, window_slot->id.slave, cid});
  }
};
} // namespace yw::control
