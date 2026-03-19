#pragma once
#include "ywx/ui_plain.h"

namespace yw::ui {

class frame : public plain {
public:
  class slot : public plain::slot {
  public:
    function<void, bool> on_focus;
    function<void, event::move> on_move;

    virtual bool focus_event(bool focused) override {
      if (enabled && on_focus) on_focus(focused);
      return true;
    }

    virtual void move_event(event::move e) override {
      if (enabled && on_move) on_move(e);
    }
  };

  const auto& on_focus() const { return unsafe_get(&slot::on_focus); }
  void on_focus(function<void, bool> value) { safe_set(&slot::on_focus, std::move(value)); }

  const auto& on_move() const { return unsafe_get(&slot::on_move); }
  void on_move(function<void, event::move> value) { safe_set(&slot::on_move, std::move(value)); }
};
}
