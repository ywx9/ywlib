#pragma once
#include "ywx/window.h"

namespace yw::ui {

//////////////////////////////////////// MARK: ui::focusable_base

class focusable_base : public base {
public:
  class slot : public base::slot {
  public:
    function<void, bool> on_focus;

    virtual bool focusable() const noexcept override { return true; }

    virtual void focus_changed(bool focused) override {
      if (on_focus) on_focus(focused);
    }
  };

protected:
  slot* _focusable_base_slot() const noexcept { return dynamic_cast<slot*>(_ui_slot()); }

  template<typename Mp, typename T> void _focusable_base_set(Mp mp, T&& value) {
    if (const auto s = _focusable_base_slot()) {
      s->*mp = static_cast<T&&>(value);
      if (const auto w = system::windows.get(s->window_id)) w->dirty = true;
    }
  }

  template<typename Mp> const auto* _focusable_base_get(Mp mp) const {
    if (const auto s = _focusable_base_slot()) return &(s->*mp);
    else return static_cast<const remove_cvref<decltype(s->*mp)>*>(nullptr);
  }

public:
  using base::base;

  const auto& on_focus() const { return *_focusable_base_get(&slot::on_focus); }
  void on_focus(function<void, bool> f) { _focusable_base_set(&slot::on_focus, std::move(f)); }

  static std::expected<focusable_base, error_trace> add(window& w, float2 Pos, float2 Size) {
    return _add<focusable_base>(w, Pos, Size);
  }
};

} // namespace yw::ui
