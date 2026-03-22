#pragma once
#include "ywx/core.h"

namespace yw {

namespace ui {

class unknown {
public:
  class slot {
  public:
    slotset<slot>::slotid id{};
    virtual ~slot() noexcept {}
    virtual void draw(float2, float2) const {}
    virtual void draw() const {}
    virtual bool attach(slotset<slot>::slotid child_id) { return false; }
    virtual void detach(slotset<slot>::slotid child_id) {}
  };

protected:
  slotset<slot>::slotid _id{};
  unknown() noexcept = default;

  template<typename Mp> member_type<Mp>* safe_get(Mp Member) const noexcept;
  template<typename Mp> member_type<Mp>& unsafe_get(Mp Member) const;

public:
  virtual ~unknown() noexcept { destroy(); }
  unknown(unknown&& other) noexcept : _id(std::exchange(other._id, {})) {}
  unknown& operator=(unknown&& other) noexcept {
    if (this != &other) _id = std::exchange(other._id, {});
    return *this;
  }

  explicit operator bool() const noexcept;
  const slotset<slot>::slotid& id() const noexcept { return _id; }
  virtual void destroy() noexcept;
};

using slotid = slotset<unknown::slot>::slotid;
inline constexpr float unconstrained = -1.0f;
} // namespace ui

namespace system {
short2 cursor_pos{};
short2 cursor_delta{};
inline slotset<ui::unknown::slot> uis{};
inline std::vector<ui::slotid> primal_windows{};
template<typename Ui> typename Ui::slot* slot_address(ui::slotid id) noexcept {
  return dynamic_cast<typename Ui::slot*>(uis.get(id));
}
} // namespace system

template<typename Mp> member_type<Mp>* ui::unknown::safe_get(Mp Member) const noexcept {
  const auto sp = dynamic_cast<typename class_type<Mp>::slot*>(system::uis.get(_id));
  return sp ? &sp->*Member : nullptr;
}

template<typename Mp> member_type<Mp>& ui::unknown::unsafe_get(Mp Member) const {
  const auto sp = dynamic_cast<typename class_type<Mp>::slot*>(system::uis.get(_id));
  if (sp) return sp->*Member;
  else throw std::logic_error("Invalid member access");
}

inline ui::unknown::operator bool() const noexcept { return system::uis.contains(_id); }

inline void ui::unknown::destroy() noexcept {
  try {
    system::uis.erase(std::exchange(_id, {}));
  } catch (...) {}
}
} // namespace yw
