#pragma once
#include "ywx/window/slave.h"

namespace yw::control {

class base {
public:
  using slot = window::control_slot;
  static constexpr auto kind = source().unique_id();

protected:
  window::control_slotid _id;
  uint64_t _kind{};

  base(window::control_slotid i, uint64_t k) noexcept : _id(i), _kind(k) {}

  window::slot* _window() const noexcept { return _id.get_window(); }
  window::control_slot* _control() const noexcept { return _id.get_control(); }

  template<typename Ctrl> static std::expected<Ctrl, error_trace> add(window::slave& w, float2 position, float2 size) {
    const auto window_slot = w._window();
    if (!window_slot) return unexpected_error(errors::invalid_argument, "invalid window");
    auto control_slot = std::make_unique<typename Ctrl::slot>(window_slot->id, position, size);
    const auto cid = window_slot->controls.push(std::move(control_slot));
    const auto control_slot_ptr = window_slot->controls.get(cid);
    if (!control_slot_ptr) return unexpected_error(errors::operation_failed, "failed to create control slot");
    control_slot_ptr->id.control = cid;
    window_slot->dirty = true;
    return Ctrl({window_slot->id.master, window_slot->id.slave, cid}, Ctrl::kind);
  }

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

  auto on_hover() noexcept {
    using func_slot = typename function<void, control::base&, hover_event>::slot;
    if (const auto control_slot = _control()) return func_slot(control_slot->on_hover);
    return func_slot();
  }

  template<derived_from<base> Ctrl> Ctrl* as() noexcept {
    if (_kind != Ctrl::kind) return nullptr;
    return dynamic_cast<Ctrl*>(this);
  }

  static std::expected<base, error_trace> add(window::slave& w, float2 Pos, float2 Size) {
    return add<base>(w, Pos, Size);
  }
};

template<derived_from<base> T> inline constexpr auto kind = T::kind;
} // namespace yw::control
