#pragma once
#include "ywx/core.h"

namespace yw {

class part {
  part(const part&) = delete;
  part& operator=(const part&) = delete;
public:
  class slot {
  public:
    slotid id{}, owner{};
    float2 pos{}, size{};
    color background_color = colors::white;
    bool visible = true;

    virtual bool focusable() const noexcept { return false; }
    virtual bool hit_test(float2 local_pt) const noexcept { return local_pt.x >= 0 && local_pt.y >= 0 && local_pt.x < size.x && local_pt.y < size.y; }
    virtual std::expected<void, error_trace> draw() const { return {}; }
  };

  virtual ~part() noexcept = default;
  part() noexcept = default;

  part(part&& p) noexcept : id(std::exchange(p.id, {})) {}
  part& operator=(part&& p) noexcept { return id = std::exchange(p.id, {}), *this; }


protected:
  slotid id{};
  part(slotid id) noexcept : id(id) {}
  slot* _slot() const noexcept;

  template<typename Mp> const auto& _get(Mp mp) const {
    if (const auto s = dynamic_cast<class_type<Mp>*>(_slot())) return s->*mp;
    else throw std::runtime_error("unaccessible part");
  }

  template<typename Mp, typename T> void _set(Mp mp, T&& value) const noexcept {
    if (auto s = dynamic_cast<class_type<Mp>*>(_slot())) s->*mp = std::forward<T>(value);
  }

  template<typename P> static std::expected<P, error_trace> add(const slotid& Owner, float2 Pos, float2 Size);
};

namespace system {
slotlist<part::slot> parts;
}

inline part::slot* part::_slot() const noexcept {
  if (const auto s = system::parts.get(id)) return s;
  else return nullptr;
}

template<typename P> std::expected<P, error_trace> part::add(const slotid& Owner, float2 Pos, float2 Size) {
  auto s = std::make_unique<typename P::slot>(Owner, Pos, Size);
  const auto id = system::parts.push(std::move(s));
  if (auto ps = system::parts.get(id)) {
    return P(ps->id = id);
  } else return unexpected_error(errors::operation_failed, "failed to add part");
}

class window : public part {
public:
  enum class style : uint32_t {
    unknown,
    regular = WS_OVERLAPPEDWINDOW,
    fixed = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
    borderless = WS_POPUP
  };

  class slot : public part::slot {
  public:
    HWND hwnd{};
    int4 margin{};
    window::style style{};
    bitmap rendertarget{};
    comptr<IDXGISwapChain1> swapchain{};
    stopwatch timer{};
    slotlist<part::slot> parts;
    slotid hovered_part{}, focused_part{};
  };
};
}
