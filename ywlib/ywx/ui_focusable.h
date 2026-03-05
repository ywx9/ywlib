#pragma once
#include "ywx/window.h"

namespace yw::ui {

//////////////////////////////////////// MARK: ui::focusable

class focusable : public frame {
public:
  class slot : public frame::slot {
  public:
    function<void, bool> on_focus;

    virtual bool focus_event(bool focused) override {
      if (on_focus) on_focus(focused);
      return true;
    }
  };

public:
  using frame::operator bool;

  const auto& on_focus() const { return unsafe_get(&slot::on_focus); }
  void on_focus(function<void, bool> f) { _set(&slot::on_focus, std::move(f)); }

  template<included_in<window&, none> Window>
  static std::expected<focusable, error_trace> add(Window&& w, float2 Pos, float2 Size) {
    if (auto res = base::add<focusable>(w, Pos, Size)) return std::move(yw::get<0>(*res));
    else return unexpected_error(res.error());
  }
};

} // namespace yw::ui
