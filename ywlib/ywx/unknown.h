#pragma once
#include "ywx/core.h"

namespace yw {

class unknown {
  /// \note window と control で共有する基底クラス
public:
  class slot {
  public:
    slotset<slot>::slotid id{};
    slotset<slot>::slotid layout_id{};
    slotset<slot>::slotid window_id{};
    bool setter_fence = false;

    class setter {
      slot* _p;
    };


    virtual ~slot() noexcept {}
    virtual const char* attachable() const { return "Non-attachable control"; }
    virtual void attach(slotset<slot>::slotid Child) {}
    virtual void detach(slotset<slot>::slotid Child) {}
    virtual void make_dirty() {} /// UIレイアウトの再描画フラグを立てる (ジオメトリ、レイアウトは再利用)
    virtual void make_moved() {} /// UIコントロールのジオメトリ更新フラグを立てる (レイアウトは再利用)
    virtual void make_messy() {} /// UIレイアウトの再計算フラグを立てる
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
