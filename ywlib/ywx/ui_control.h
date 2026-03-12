#pragma once
#include "ywx/core.h"

namespace yw::ui {

enum class alignment { center, leading, trailing };

inline constexpr float unconstrained = std::numeric_limits<float>::quiet_NaN();

class control {
public:
  class slot {
  public:
    slotset<slot>::slotid id{};
    slotset<slot>::slotid parent{};
    slotset<slot>::slotid window{};
    float width{unconstrained};
    float height{unconstrained};
    float4 margin{5.0f, 5.0f, 5.0f, 5.0f};
    ui::alignment alignment{alignment::center};
    bool visible = true;
    bool enabled = true;

    /// returns the minimum size and the number of flexible controls.
    virtual tuple<float2, uint2> minimum_size() const noexcept {
      float2 size = {std::isnan(width) ? 0.0f : width, std::isnan(height) ? 0.0f : height};
      return {size + margin.xy() + margin.zw(), {std::isnan(width), std::isnan(height)}};
    }

    /// erases the child control with the given slotid if it has.
    virtual void erase_child(const slotset<slot>::slotid& child_id) noexcept {}

    /// adds a child control with the given slotid if it can.
    virtual bool add_child(const slotset<slot>::slotid& child_id) noexcept { return false; }

    /// sets the dirty flag of the window containing this control.
    virtual void make_dirty() noexcept;

    virtual void draw(float2 Pos, float2 Size) const {};
  };

protected:
  template<typename CtrlSlot> static slotset<slot>::slotid create(control& Layout) noexcept;

  slotset<slot>::slotid _id{};

  control(slotset<slot>::slotid Id) : _id(Id) {}

  control(const control&) = delete;
  control& operator=(const control&) = delete;

  control(control&& other) noexcept : _id(std::exchange(other._id, {})) {}

  control& operator=(control&& other) noexcept {
    if (this == &other) return *this;
    destroy();
    _id = std::exchange(other._id, {});
    return *this;
  }

  /// \note `c` must not be null.
  template<typename Ctrl> static Ctrl::slot* slot_address(const Ctrl* c) noexcept;

  template<typename Mop> member_type<Mop>* safe_get(Mop Member) const noexcept {
    const auto s = dynamic_cast<class_type<Mop>*>(slot_address(this));
    return s ? &s->*Member : nullptr;
  }

  template<typename Mop> member_type<Mop>& unsafe_get(Mop Member) const noexcept {
    const auto s = dynamic_cast<class_type<Mop>*>(slot_address(this));
    if (s) return s->*Member;
    else throw std::logic_error("Invalid member access");
  }

  template<typename Mop, typename T> void safe_set(Mop Member, T&& Value) const noexcept {
    if (auto s = dynamic_cast<class_type<Mop>*>(slot_address(this))) {
      s->*Member = std::forward<T>(Value);
      s->make_dirty();
    }
  }

public:
  virtual ~control() noexcept;
  control() noexcept = default;

  explicit operator bool() const noexcept;
  const slotset<slot>::slotid& id() const noexcept { return _id; }

  const slotset<slot>::slotid& parent() const { return unsafe_get(&slot::parent); }
  const slotset<slot>::slotid& window() const { return unsafe_get(&slot::window); }

  const float& width() const { return unsafe_get(&slot::width); }
  void width(float value) { safe_set(&slot::width, value); }

  const float& height() const { return unsafe_get(&slot::height); }
  void height(float value) { safe_set(&slot::height, value); }

  const float4& margin() const { return unsafe_get(&slot::margin); }
  void margin(std::optional<float4> value) { safe_set(&slot::margin, value); }

  const bool& visible() const { return unsafe_get(&slot::visible); }
  void visible(bool value) { safe_set(&slot::visible, value); }

  const bool& enabled() const { return unsafe_get(&slot::enabled); }
  void enabled(bool value) { safe_set(&slot::enabled, value); }

  void destroy() noexcept;
};

//////////////////////////////////////// MARK: system::controls

namespace system {
inline slotset<control::slot> controls;
}

//////////////////////////////////////// MARK: implementation

void control::slot::make_dirty() noexcept {
  if (auto c = system::controls.get(parent)) c->make_dirty();
}

template<typename Ctrl> slotset<control::slot>::slotid
control::create(control& Layout) noexcept {
  if (const auto parent = system::controls.get(Layout.id())) {
    const auto id = system::controls.add(std::make_unique<Ctrl::slot>());
    if (Layout.add_child(id)) {
      if (auto child = system::controls.get(id)) {
        child->parent = Layout.id();
        child->window = parent->window;
        return id;
      }
    }
    system::controls.erase(id);
  }
  return {};
}

template<typename Ctrl> Ctrl::slot* control::slot_address(const Ctrl* c) noexcept {
  return dynamic_cast<Ctrl::slot*>(system::controls.get(c->_id));
}

inline control::~control() noexcept { destroy(); }

inline control::operator bool() const noexcept { return system::controls.contains(_id); }

void control::destroy() noexcept {
  if (const auto s = system::controls.get(_id)) {
    system::controls.erase(_id);
    if (const auto ps = system::controls.get(s->parent)) ps->erase_child(_id);
  }
}
} // namespace yw::ui
