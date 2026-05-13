#pragma once
#include "ywx/ui_control.h"
#include "ywx/uip_text.h"

namespace yw::ui {

/// \note ui::labelのminimum_sizeについて、非拘束である場合、テキストがpaddingも含めて表示されるように決定される。

class label : public control {
public:
  struct slot : public control::slot {
    parts::background background;
    parts::border border;
    parts::text text;

    //-- overrides --//

    virtual void draw() override {
      if (!visible) return;
      brush.color(background.color);
      fill_geometry(core.geometry.get());
      d2d.push_layer(core.geometry.get());
      if (background.image) draw_bitmap(core.pos, core.size, background.image, background.image_opacity);
      text.draw(core.pos, core.size);
      d2d.pop_layer();
      border.draw(core.geometry.get());
    }

    virtual void ensure_minimum_size() {
      const float2 inner = text.area() * (int2(1, 1) - core.constrained);
      core.size = vapply_r<float2>(yw::max, core.min_size, core.required_size * core.constrained, inner);
    }

    virtual slotid next_tab_stop(slotid, bool, bool&) const override { return {}; }
  };

  using control::operator bool;
  label() noexcept = default;

  static std::expected<label, error_trace> add(derived_from<unknown> auto& Layout) {
    label lbl;
    if (auto res = create_control<label>(Layout)) lbl._id = *res;
    else return unexpected_error(res.error());
    if (const auto csp = system::slot_address<label>(lbl._id)) {
      csp->background.control_id = lbl._id;
      csp->border.control_id = lbl._id;
      csp->text.control_id = lbl._id;
    } else return unexpected_error(errors::ui_invalid_slotid, "missing slot ");
    return lbl;
  }

  auto background() {
    const auto csp = system::slot_address<label>(_id);
    if (!csp) fatal_error(errors::ui_invalid_slotid);
    return csp->background.handle();
  }

  const auto background() const {
    const auto csp = system::slot_address<label>(_id);
    if (!csp) fatal_error(errors::ui_invalid_slotid);
    return csp->background.handle();
  }

  auto border() {
    const auto csp = system::slot_address<label>(_id);
    if (!csp) fatal_error(errors::ui_invalid_slotid);
    return csp->border.handle();
  }

  const auto border() const {
    const auto csp = system::slot_address<label>(_id);
    if (!csp) fatal_error(errors::ui_invalid_slotid);
    return csp->border.handle();
  }

  auto text() {
    const auto csp = system::slot_address<label>(_id);
    if (!csp) fatal_error(errors::ui_invalid_slotid);
    return csp->text.handle();
  }

  const auto text() const {
    const auto csp = system::slot_address<label>(_id);
    if (!csp) fatal_error(errors::ui_invalid_slotid);
    return csp->text.handle();
  }

  /// textの表示サイズにコントロールのサイズを合わせる
  auto& fit_to_text() {
    const auto csp = system::slot_address<label>(_id);
    if (!csp) fatal_error(errors::ui_invalid_slotid);
    this->core().size(csp->text.area());
    return *this;
  }
};
} // namespace yw::ui
