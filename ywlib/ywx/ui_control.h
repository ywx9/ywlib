#pragma once
#include "ywx/core.h"

namespace yw::ui {

inline constexpr float unconstrained = 0.0f;

class control {
public:
  class slot;
  using slotid = slotset<slot>::slotid;

  template<typename Slot> static Slot* _slot_address(const slotid& id) noexcept;
  template<typename Slot> static slotid _create_slot();

  class slot {
  public:
    slotid id{}, layout_id{};
    union {
      float2 size{unconstrained, unconstrained};
      struct {
        float width, height;
      };
    };
    mutable float4 last_rect{};
    float4 margin{5.0f, 5.0f, 5.0f, 5.0f};
    bool visible = true, enabled = true;
    mutable bool dirty = true;

    virtual ~slot() noexcept {
      if (const auto lsp = _slot_address<slot>(layout_id)) lsp->detach(id);
    }

    /// returns the minimum size and the number of unconstrained dimensions.
    virtual tuple<float2, uint2> minimum_size() const noexcept {
      return {size + margin.xy() + margin.zw(), uint2(size.x == 0.0f, size.y == 0.0f)};
    }

    /// attaches a child control with the given slotid. Returns false if the control cannot be attached.
    virtual bool attach(const slotid& child_id) noexcept { return false; }

    /// detaches the child control with the given slotid.
    virtual void detach(const slotid& child_id) noexcept {}

    /// need to be called when the control needs to be redrawn.
    virtual void make_dirty() noexcept {
      dirty = true;
      if (const auto lsp = _slot_address<slot>(layout_id)) lsp->make_dirty();
    }

    /// need to be called when the layout needs to be updated.
    virtual void make_mess() noexcept {
      if (const auto lsp = _slot_address<slot>(layout_id)) lsp->make_mess();
    }

    /// checks if the given position is within the control's bounds.
    virtual slotid hit_test(float2 Pt) const noexcept { return {}; }

    /// draws the control at the given position with the given size.
    virtual void draw(float2 Pos, float2) const { last_rect = float4(Pos, Pos); };
    virtual void draw_focus() const {}

    virtual void button_event(event::button e) {}
    virtual void hover_event(event::hover e) {}
    virtual void move_event(event::move e) {}
    virtual void key_event(event::key e) {}
    virtual void char_event(wchar_t c) {}
    virtual bool focus_event(bool) { return false; }
    virtual void click_event(event::button e) {}
  };

protected:
  template<typename Slot> static slotid create(control& Layout) {
    if (const auto l_slot_p = _slot_address<slot>(Layout._id)) {
      const auto id = _create_slot<Slot>();
      if (l_slot_p->attach(id)) {
        if (auto slot_p = _slot_address<Slot>(id)) {
          slot_p->id = id;
          slot_p->layout_id = Layout._id;
          return id;
        } else unexpected_error(errors::operation_failed, "Failed to access the created slot.");
      } else unexpected_error(errors::operation_failed, "Failed to attach the slot to the layout.");
    } else unexpected_error(errors::operation_failed, "Failed to access the layout's slot.");
    unexpected_error(errors::operation_failed, "Failed to create slot.");
    return {};
  }

  slotid _id{};
  control(slotid Id) : _id(Id) {}

  template<typename Mp> member_type<Mp>* safe_get(Mp Member) const noexcept {
    const auto s = _slot_address<class_type<Mp>>(this);
    return s ? &s->*Member : nullptr;
  }

  template<typename Mp> member_type<Mp>& unsafe_get(Mp Member) const {
    const auto s = _slot_address<class_type<Mp>>(this);
    if (s) return s->*Member;
    else throw std::logic_error("Invalid member access");
  }

  template<typename Mp, typename T> void safe_set(Mp Member, T&& Value) const noexcept {
    if (auto s = _slot_address<class_type<Mp>>(this)) {
      s->*Member = std::forward<T>(Value);
      s->make_dirty();
    }
  }

  template<typename Mp, typename T> void safe_set_size(Mp Member, T&& Value) const noexcept {
    if (auto s = _slot_address<class_type<Mp>>(this)) {
      s->*Member = std::forward<T>(Value);
      s->make_dirty();
      s->make_mess();
    }
  }

public:
  virtual ~control() noexcept { destroy(); }
  control() noexcept = default;

  control(const control&) = delete;
  control& operator=(const control&) = delete;

  control(control&& other) noexcept : _id(std::exchange(other._id, {})) {}

  control& operator=(control&& other) noexcept {
    if (this == &other) return *this;
    destroy();
    _id = std::exchange(other._id, {});
    return *this;
  }

  explicit operator bool() const noexcept;
  const slotid& id() const noexcept { return _id; }

  const float2& size() const { return unsafe_get(&slot::size); }
  void size(float2 value) { safe_set_size(&slot::size, value); }

  const float& width() const { return unsafe_get(&slot::width); }
  void width(float value) { safe_set_size(&slot::width, value); }

  const float& height() const { return unsafe_get(&slot::height); }
  void height(float value) { safe_set_size(&slot::height, value); }

  const float4& margin() const { return unsafe_get(&slot::margin); }
  void margin(const float4 value) { safe_set(&slot::margin, value); }

  const bool& visible() const { return unsafe_get(&slot::visible); }
  void visible(bool value) { safe_set(&slot::visible, value); }

  const bool& enabled() const { return unsafe_get(&slot::enabled); }
  void enabled(bool value) { safe_set(&slot::enabled, value); }

  void destroy() noexcept;
};
} // namespace yw::ui

namespace yw {

namespace system {
inline slotset<ui::control::slot> controls;
}

template<typename Slot> Slot* ui::control::_slot_address(const slotid& id) noexcept {
  return dynamic_cast<Slot*>(system::controls.get(id));
}

template<typename Slot> ui::control::slotid ui::control::_create_slot() {
  return system::controls.add(std::make_unique<Slot>());
}

inline explicit ui::control::operator bool() const noexcept { return system::controls.contains(_id); }

inline void ui::control::destroy() noexcept { system::controls.erase(std::exchange(_id, {})); }
} // namespace yw
