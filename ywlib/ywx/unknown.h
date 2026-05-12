#pragma once
#include "ywx/core.h"

namespace yw {

class unknown {
  /// \note window と control で共有する基底クラス
public:
  class slot {
  public:
    slotset<slot>::slotid id{};
    virtual ~slot() noexcept {}
    virtual bool attach_child(slotset<slot>::slotid Child) { return false; }
    virtual void detach_child(slotset<slot>::slotid Child) {}
    virtual void make_dirty() {}
    virtual void make_messy() {}
  };

protected:
  slotset<slot>::slotid _id{};
  unknown() noexcept = default;

public:
  virtual ~unknown() noexcept {}

  unknown(unknown&& Other) noexcept : _id(std::exchange(Other._id, {})) {}
  unknown& operator=(unknown&& Other) noexcept {
    if (this != &Other) _id = std::exchange(Other._id, {});
    return *this;
  }

  explicit operator bool() const noexcept;
  const slotset<slot>::slotid& id() const noexcept { return _id; }
};

namespace system {
short2 cursor_pos{};
short2 cursor_delta{};
inline slotset<unknown::slot> uis{};
inline std::vector<slotset<unknown::slot>::slotid> primal_windows{};
template<derived_from<unknown> Ui> typename Ui::slot* slot_address(slotset<unknown::slot>::slotid id) noexcept {
  return dynamic_cast<typename Ui::slot*>(uis.get(id));
}
} // namespace system

inline unknown::operator bool() const noexcept { return system::uis.contains(_id); }
} // namespace yw
