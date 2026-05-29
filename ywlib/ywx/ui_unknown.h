#pragma once
#include "ywx/core.h"

namespace yw::errors {
define_error(ui_invalid_slotid);
define_error(ui_not_attachable);
}

namespace yw::ui {

class unknown {
public:
  struct slot {
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

inline constexpr float arbitrary_value = 4.0f;
using slotid = slotset<unknown::slot>::slotid;

enum class alignment : unsigned char {
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

template<typename Ui> class accessor {
protected:
  typename Ui::slot& slot;
  bool dirty{}, moved{}, messy{};

public:
  accessor(typename Ui::slot& Slot) noexcept : slot(Slot) {}
  accessor(accessor&&) noexcept = default;
  accessor& operator=(accessor&&) noexcept = default;
  ~accessor() noexcept {
    if (messy) slot.make_messy();
    else if (moved) slot.make_moved();
    else if (dirty) slot.make_dirty();
  }
};
} // namespace yw::ui

namespace yw::system {
bool recording_undo = true;
short2 cursor_pos{};
short2 cursor_delta{};
inline slotset<ui::unknown::slot> uis{};
inline std::vector<slotset<ui::unknown::slot>::slotid> primal_windows{};
template<derived_from<ui::unknown> Ui> typename Ui::slot* slot_address(slotset<ui::unknown::slot>::slotid id) noexcept {
  return dynamic_cast<typename Ui::slot*>(uis.get(id));
}
} // namespace yw::system

namespace yw {

inline ui::unknown::operator bool() const noexcept { return system::uis.contains(_id); }

constexpr auto to_dwrite_value(ui::alignment Align) noexcept {
  if (const auto a = uint8_t(Align) & 0b11; a == 0b01) return DWRITE_TEXT_ALIGNMENT_LEADING;
  else if (a == 0b10) return DWRITE_TEXT_ALIGNMENT_TRAILING;
  else return DWRITE_TEXT_ALIGNMENT_CENTER;
}
}
