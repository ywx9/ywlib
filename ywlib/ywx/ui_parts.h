#pragma once
#include "ywx/ui_control.h"
#include "ywx/unknown.h"

namespace yw::ui::parts {

//////////////////////////////////////// MARK: part_base

struct part_base {
  slotid control_id{};
  bool view_changed = false;
  bool geometry_changed = false;
  bool layout_changed = false;

  void make_dirty() const {
    if (const auto csp = system::slot_address<unknown>(control_id)) csp->make_dirty();
  }
  void make_moved() const {
    if (const auto csp = system::slot_address<unknown>(control_id)) csp->make_moved();
  }
  void make_messy() const {
    if (const auto csp = system::slot_address<unknown>(control_id)) csp->make_messy();
  }

  template<typename Part> class handle {
  protected:
    Part* _p = nullptr;
    handle(Part& Ref) : _p(&Ref) {}

  public:
    ~handle() {
      if (!_p) return;
      if (_p->layout_changed) _p->make_messy();
      else if (_p->geometry_changed) _p->make_moved();
      else if (_p->view_changed) _p->make_dirty();
      _p->view_changed = _p->geometry_changed = _p->layout_changed = false;
    }
    handle(handle&& Other) noexcept : _p(std::exchange(Other._p, nullptr)) {}
    handle& operator=(handle&& Other) noexcept {
      if (this != &Other) _p = std::exchange(Other._p, nullptr);
      return *this;
    }
  };
};

//////////////////////////////////////// MARK: background

struct background : public part_base {
  color color = colors::white;
  yw::bitmap image{}; // optional
  float image_opacity = 1.0f;

  class handle : public part_base::handle<background> {
    friend struct background;
    using part_base::handle<background>::handle;

  public:
    const auto& color() const { return _p->color; }
    auto& color(yw::color Color) {
      _p->color = Color;
      _p->view_changed = true;
      return *this;
    }

    const auto& image() const { return _p->image; }
    auto& image(yw::bitmap Image) {
      _p->image = std::move(Image);
      _p->view_changed = true;
      return *this;
    }

    float image_opacity() const { return _p->image_opacity; }
    auto& image_opacity(float Opacity) {
      _p->image_opacity = Opacity;
      _p->view_changed = true;
      return *this;
    }
  };

  handle handle() noexcept { return *this; }
};

//////////////////////////////////////// MARK: border

struct border : public part_base {
  color color = colors::black;
  float width = 1.0f;
  bool dashed = false;

  class handle : public part_base::handle<border> {
    friend struct border;
    using part_base::handle<border>::handle;

  public:
    const auto& color() const { return _p->color; }
    auto& color(yw::color Color) {
      _p->color = Color;
      _p->view_changed = true;
      return *this;
    }

    const auto& width() const { return _p->width; }
    auto& width(float Width) {
      _p->width = Width;
      _p->view_changed = true;
      return *this;
    }

    const auto& dashed() const { return _p->dashed; }
    auto& dashed(bool Dashed) {
      _p->dashed = Dashed;
      _p->view_changed = true;
      return *this;
    }
  };

  handle handle() noexcept { return *this; }

  void draw(ID2D1Geometry* Geometry) const {
    brush.color(color);
    brush.dashed(dashed);
    draw_geometry(Geometry, width);
    brush.dashed(false);
  }
};

//////////////////////////////////////// MARK: focus_ring

struct focus_ring : public part_base {
  color color = {0.0f, 0.0f, 1.0f, 0.5f};
  float offset = arbitrary_value;
  float width = arbitrary_value * 0.5f;

  class handle : public part_base::handle<focus_ring> {
    friend struct focus_ring;
    using part_base::handle<focus_ring>::handle;

  public:
    const auto& color() const { return _p->color; }
    auto& color(yw::color Color) {
      _p->color = Color;
      _p->view_changed = true;
      return *this;
    }

    const auto& offset() const { return _p->offset; }
    auto& offset(float1 Offset) {
      _p->offset = Offset.x;
      _p->view_changed = true;
      return *this;
    }

    const auto& width() const { return _p->width; }
    auto& width(float1 Width) {
      _p->width = Width.x;
      _p->view_changed = true;
      return *this;
    }
  };

  handle handle() noexcept { return *this; }
};
} // namespace yw::ui::parts
