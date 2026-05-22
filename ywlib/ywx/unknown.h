#pragma once
#include "ywx/core.h"

namespace yw {

namespace errors{
define_error(ui_not_attachable);
}

class unknown {
  /// \note window と control で共有する基底クラス
public:
  class slot {
  public:
    slotset<slot>::slotid id{};

    virtual ~slot() noexcept {}
    virtual slotset<slot>::slotid get_window_id() const = 0;
    virtual std::expected<void, error_trace> attachable() const { return unexpected_error(errors::ui_not_attachable); }
    virtual std::expected<void, error_trace> attach(slotset<slot>::slotid Child) { return {}; }
    virtual std::expected<void, error_trace> detach(slotset<slot>::slotid Child) { return {}; }
    virtual std::expected<void, error_trace> make_dirty() = 0;
    virtual std::expected<void, error_trace> make_moved() = 0;
    virtual std::expected<void, error_trace> make_messy() = 0;
  };

  template<typename Handle> class setter {
  protected:
    friend Handle;
    using slot_type = typename Handle::slot;
    slot_type* _p = nullptr;
    bool _dirty = false, _moved = false, _messy = false;
    setter(slot_type* p) : _p(p) {}
  };

  template<typename Handle> class getter {
  protected:
    friend Handle;
    using slot_type = typename Handle::slot;
    const slot_type* _p = nullptr;
    getter(const slot_type* p) : _p(p) {}

  public:
    const auto& id() const { return _p->id; }
  };

protected:
  slotset<slot>::slotid _id{};
  unknown() noexcept = default;
  unknown(slotset<slot>::slotid Id) : _id(Id) {}

public:
  virtual ~unknown() noexcept {}

  unknown(unknown&& Other) noexcept : _id(std::exchange(Other._id, {})) {}
  unknown& operator=(unknown&& Other) noexcept {
    if (this != &Other) _id = std::exchange(Other._id, {});
    return *this;
  }

  explicit operator bool() const noexcept;
  auto _slotid() const noexcept { return _id; }
};

namespace ui {

inline constexpr float arbitrary_value = 4.0f;
using slotid = slotset<unknown::slot>::slotid;

enum class alignment {
  center = 0b0000,
  left = 0b0001,
  right = 0b0010,
  top = 0b0100,
  bottom = 0b1000,
  left_top = 0b0101,
  left_bottom = 0b1001,
  right_top = 0b0110,
  right_bottom = 0b1010,
};
}

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
