#pragma once
#include <apps/directx.h>
#include <apps/window_base.h>
#include <base/color.h>
#include <base/function.h>
#include <base/optional.h>
#include <base/slotset.h>
#include <base/vector.h>
#include <core/core.h>
#include <core/property.h>

#ifdef _WIN32

#ifdef interface
#undef interface
#endif

namespace yw {

///--------------------------------------------------------------------------///
/// MARK: interface

class interface {
public:
  struct slot {
    inline static slotset<slot> slots{};
    slotset<slot>::slotid id;
    virtual ~slot() noexcept = default;
  };

  using slotid = slotset<slot>::slotid;
  const_property<slotid, interface> id;
  explicit operator bool() const noexcept { return slot::slots.exists(id); }

  virtual ~interface() noexcept { _destroy_slot(); }
  interface() noexcept = default;
  interface(const interface&) = delete;
  interface& operator=(const interface&) = delete;
  interface(interface&& o) noexcept : id(exchange(o.id.ref(), {})) {}
  interface& operator=(interface&& o) noexcept {
    if (this == &o) return *this;
    _destroy_slot();
    id = exchange(o.id.ref(), {});
    return *this;
  }

protected:
  explicit interface(slotid Id) : id(Id) {}
  void _destroy_slot() noexcept {
    if (const auto sp = slot::slots.get(id))
      if (auto res = slot::slots.erase(id); !res) // 設計上、エラーを想定していない
        res.error().print_and_abort("Failed to erase slot for interface");
  }
};

namespace ui {

enum class size_policy : unsigned char {
  fit,   // minimum size to show whole content
  free,  // any size so that at least whole content is visible
  fixed, // specified size as is
};
}

///--------------------------------------------------------------------------///
/// MARK: control

class control : public interface {
public:
  struct slot : interface::slot {
    static constexpr float arbitrary_value = 4.0f;
    static constexpr float arbitrary_size_value = 16.0f;

    slotid parent_id;
    slotid window_id;

    float4 margin = float4::fill(arbitrary_value);
    float4 padding = float4::fill(arbitrary_value);
    optional<float> required_width;
    optional<float> required_height;
    float2 provided_origin;
    float2 provided_area;
    float2 pos;
    float2 size;
    float2 radius = float2::fill(arbitrary_value);
    float2 minimum_size = float2::fill(arbitrary_value);
    alignment align = alignment::center;
    bool2 grow = false;
    comptr<ID2D1Geometry> geometry;

    string<preferred_char> tooltip;

    function<bool, yw::button_event> button_event;
    function<bool, yw::cursor_event> cursor_event;
    function<bool, yw::drag_event> drag_event;
    function<bool, yw::focus_event> focus_event;
    function<bool, yw::hover_event> hover_event;
    function<bool, yw::key_event> key_event;
    function<bool, yw::wheel_event> wheel_event;

    bool geometry_dirty = false;
    bool visible = false;
    bool enabled = false;

    virtual bool focusable() const { return enabled && bool(focus_event); }
    virtual bool interactive() const {
      return enabled && visible && (button_event || cursor_event || drag_event || hover_event || wheel_event);
    }

    virtual float2 get_minimum_size() const { return minimum_size; }
    virtual float2 get_content_size() const { return {}; }

    virtual result<float2> get_necessary_size() const {
      const auto inner = get_content_size() + padding.xy() + padding.zw();
      const auto required_size = float2(required_width.value_or(inner.x()), required_height.value_or(inner.y()));
      return vapply_r<float2>(yw::max, get_minimum_size(), required_size);
    }

    virtual result<void> draw_background(interface::slot*) { return {}; }
    virtual result<void> draw_overlay(interface::slot*) { return {}; }
    virtual result<void> draw_foreground(interface::slot*) { return {}; }

    virtual slotid get_tabstop(slotid Current, bool Backward, bool& CurrentIsFound) const {
      if (!focusable()) return {};
      if (Current == id) CurrentIsFound = true;
      else if (CurrentIsFound) return id;
      return {};
    }

    virtual slotid hittest(float2 Pt) const {
      if (!visible || !enabled || !geometry) return {};
      BOOL contains = FALSE;
      if (const auto hr = geometry->FillContainsPoint({Pt.x(), Pt.y()}, nullptr, &contains); FAILED(hr))
        error(errors::operation_failed, "ID2D1Geometry::FillContainsPoint failed").consume();
      return contains ? id : slotid{};
    }

    virtual result<void> redraw(interface::slot* Window) {
      if (geometry_dirty) {
        geometry_dirty = false;
        if (auto res = relocate(); !res) return res.relay();
      }
      if (!visible) return {};
      d2d::push_layer(geometry.get());
      if (auto res = draw_background(Window); !res) return d2d::pop_layer(), res.relay();
      d2d::pop_layer();
      if (auto res = draw_overlay(Window); !res) return res.relay();
      if (auto res = draw_foreground(Window); !res) return res.relay();
      return {};
    }

    virtual result<void> relocate() {
      if (auto res = update_geometry()) return {};
      else res.relay();
    }

    virtual result<void> relocate(float2 Origin, float2 Area) {
      provided_origin = Origin;
      provided_area = Area;
      if (auto res = update_geometry()) return {};
      else res.relay();
    }

    virtual result<void> set_size_to_necessary() {
      if (auto res = get_necessary_size()) size = *res;
      else return res.relay();
      return {};
    }

    result<float2> update_geometry() {
      const auto max_size = provided_area - margin.xy() - margin.zw();
      if (auto res = set_size_to_necessary(); !res) return res.relay();
      const auto necessary_size = size;
      if (grow.x()) size.x() = max_size.x();
      if (grow.y()) size.y() = max_size.y();
      constexpr float c[]{0.5f, 0.0f, 1.0f};
      const float2 cc{c[unsigned(align) % 3], c[unsigned(align) / 4 % 3]};
      pos = provided_origin + margin.xy() + (max_size - size) * cc;
      ID2D1RoundedRectangleGeometry* geom = nullptr;
      D2D1_ROUNDED_RECT rr{
        D2D1::RectF(pos.x(), pos.y(), pos.x() + size.x(), pos.y() + size.y()), radius.x(), radius.y()};
      const auto hr = d2d::factory()->CreateRoundedRectangleGeometry(&rr, &geom);
      if (FAILED(hr)) return error(errors::operation_failed, "CreateRoundedRectangleGeometry failed");
      geometry.reset(geom);
      return size - necessary_size; // used by layout-like controls
    }
  };
};

///--------------------------------------------------------------------------///
/// MARK: window

class window : public interface {
public:
  struct custom_options {
    string<preferred_char> title;
    optional<int2> pos;
    optional<int2> size;
    DWORD style = WS_OVERLAPPEDWINDOW;
    DWORD ex_style = WS_EX_ACCEPTFILES;
    const string<preferred_char>& get_title() const noexcept { return title; }
    DWORD get_style() const noexcept { return style; }
    DWORD get_ex_style() const noexcept { return ex_style; }
  };

  struct options {
    string<preferred_char> title;
    optional<int2> pos;
    optional<int2> size;
    bool has_border = true;
    bool has_caption = true;
    bool resizable = true;
    bool visible = true;
    bool enabled = true;
    bool topmost = false;
    const string<preferred_char>& get_title() const noexcept { return title; }
    DWORD get_style() const noexcept {
      DWORD s = has_caption ? WS_CAPTION | WS_SYSMENU : WS_POPUP;
      s |= WS_BORDER * has_border;
      s |= WS_THICKFRAME * resizable;
      s |= WS_VISIBLE * visible;
      s |= WS_DISABLED * !enabled;
      return s;
    }
    DWORD get_ex_style() const noexcept {
      DWORD s = WS_EX_ACCEPTFILES;
      s |= topmost ? WS_EX_TOPMOST : 0;
      return s;
    }
  };

  struct slot : interface::slot {
    inline static std::vector<slotid> main_windows{};

    slotid parent_id{};
    HWND hwnd{};

    int4 frame_thickness{};
    DWORD style{}, exstyle{};

    bitmap control_layer;
    bitmap render_target;
    comptr<IDXGISwapChain1> swap_chain;

    yw::color_theme color_theme;

    struct tooltip {
      optional<yw::color> background_color;
      optional<yw::color> text_color;
      float4 padding{};
      float2 offset{};
    } tooltip;


  };
};

} // namespace yw

#endif
