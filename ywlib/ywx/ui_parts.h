#pragma once
#include "ywx/ui_unknown.h"

namespace yw::ui {
/// 特に意味を持たない初期値
inline constexpr float arbitrary_value = 4.0f;

/// コントロールの配置を指定する列挙型
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

using slotid = slotset<ui::unknown::slot>::slotid;
}

namespace yw::ui::part {

void make_dirty(slotid Window) noexcept;
void make_messy(slotid Window) noexcept;

struct core {
  slotid window_id{};
  float4 margin = float4::fill(arbitrary_value);
  float2 min_size = float2::fill(arbitrary_value * 2);
  float2 pos{};
  float2 size{};
  float2 provided_pos{};
  float2 provided_size{};
  float2 radius = float2::fill(arbitrary_value);
  comptr<ID2D1Geometry> geometry{};
  ui::alignment alignment = ui::alignment::center;
  vector<bool, 2> constrained{false, false};
  bool view_changed = false;
  bool geometry_changed = false;
  bool layout_changed = false;

  void reset_change_flags() { view_changed = geometry_changed = layout_changed = false; }

  std::expected<void, error_trace> update_geometry() {
    ID2D1RoundedRectangleGeometry* rrgp = nullptr;
    D2D1_ROUNDED_RECT rr{D2D1_RECT_F(pos.x, pos.y, pos.x + size.x, pos.y + size.y), radius.x, radius.y};
    if (const auto hr = d2d.factory()->CreateRoundedRectangleGeometry(&rr, &rrgp); FAILED(hr))
      return unexpected_error(errors::operation_failed, "Failed to create geometry", int32_t(hr));
    geometry.reset(rrgp);
    return {};
  }

  class handle {
    friend struct core;
    core* _p = nullptr;
    handle(core& Ref) : _p(&Ref) {}

  public:
    ~handle() {
      if (!_p) return;
      if (_p->layout_changed) make_messy(_p->window_id);
      else if (_p->geometry_changed) {
        _p->update_geometry();
        make_dirty(_p->window_id);
      } else if (_p->view_changed) make_dirty(_p->window_id);
      _p->reset_change_flags();
      _p = nullptr;
    }

    handle(handle&& Other) noexcept : _p(std::exchange(Other._p, nullptr)) {}
    handle& operator=(handle&& Other) noexcept {
      if (this != &Other) _p = std::exchange(Other._p, nullptr);
      return *this;
    }

    const auto& margin() const { return _p->margin; }
    auto& margin(float4 Margin) { _p->margin = Margin, _p->layout_changed = true; return *this; }

    const auto& min_size() const { return _p->min_size; }
    auto& min_size(float2 MinSize) { _p->min_size = MinSize; _p->layout_changed = true; return *this; }

    const auto& pos() const { return _p->pos; }
    /// \note position of control is automatically determined by layout

    const auto& size() const { return _p->size; }
    auto& size(std::optional<float> width, std::optional<float> height) {
      if (_p->constrained.x = width.has_value()) _p->size.x = *width;
      if (_p->constrained.y = height.has_value()) _p->size.y = *height;
      _p->layout_changed = true;
      return *this;
    }

    const auto& width() const { return _p->size.x; }
    auto& width(std::optional<float> Width) {
      if (_p->constrained.x = Width.has_value()) _p->size.x = *Width;
      _p->layout_changed = true;
      return *this;
    }

    const auto& height() const { return _p->size.y; }
    auto& height(std::optional<float> Height) {
      if (_p->constrained.y = Height.has_value()) _p->size.y = *Height;
      _p->layout_changed = true;
      return *this;
    }

    const auto& radius() const { return _p->radius; }
    auto& radius(float2 Radius) { _p->radius = Radius; _p->geometry_changed = true; return *this; }

    const auto& alignment() const { return _p->alignment; }
    auto& alignment(ui::alignment Alignment) { _p->alignment = Alignment; _p->geometry_changed = true; return *this; }
  };

  handle handle(window_id Window) noexcept { return *this; }
};


//////////////////////////////////////// MARK: background

struct background {
  slotid window_id{};
  color color = colors::white;
  yw::bitmap image{}; // optional
  float image_opacity = 1.0f;

  class handle {
    friend struct background;
    background* _p = nullptr;
    handle(background& Ref) : _p(&Ref) {}

  public:
    const auto& color() const { return _p->color; }
    auto& color(yw::color Color) { return _p->color = Color, *this; }

    const auto& image() const { return _p->image; }
    auto& image(yw::bitmap Image) { return _p->image = std::move(Image), *this; }

    float image_opacity() const { return _p->image_opacity; }
    auto& image_opacity(float Opacity) { return _p->image_opacity = Opacity, *this; }
  };

  handle handle() noexcept { return *this; }
};

struct border {
  color color = colors::black;
  float width = 1.0f;

  class handle {
    friend struct border;
    border* _p =
    border* _p = nullptr;
    handle(border& Ref) : _p(&Ref) {}

  public:
    const auto& color() const { return _p->color; }
    auto& color(yw::color Color) { return _p->color = Color, *this; }

    const auto& width() const { return _p->width; }
    auto& width(float Width) { return _p->width = Width, *this; }
  };

  handle handle() noexcept { return *this; }
};

}
