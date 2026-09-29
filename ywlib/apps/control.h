#pragma once
#include <apps/directx.h>
#include <apps/key.h>
#include <core/core.h>
#include <core/format.h>
#include <core/function.h>
#include <core/optional.h>
#include <core/property.h>
#include <core/slotset.h>
#include <core/vector.h>

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

///--------------------------------------------------------------------------///
/// MARK: alignment

enum class alignment : uint8_t {
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

///--------------------------------------------------------------------------///
/// MARK: events

struct button_event {
  short2 pos;
  key key;
  key_state state;
  constexpr string<char> to_string() const {
    return format("button_event(pos:", pos, ", key:", internal::_get_key_name(key), ", state:", state, ")");
  }
};

struct cursor_event {
  short2 pos;
  short2 delta;
  constexpr string<char> to_string() const { return format("cursor_event(pos:", pos, ", delta:", delta, ")"); }
};

struct drag_event {
  short2 delta;
  key key;
  key_state state;
  constexpr string<char> to_string() const {
    return format("drag_event(delta:", delta, ", key:", internal::_get_key_name(key), ", state:", state, ")");
  }
};

struct focus_event {
  bool focused;
  constexpr string_view<char> to_string() const {
    if (focused) return "focus_event(focused:1)";
    else return "focus_event(focused:0)";
  }
};

struct hover_event {
  bool hovered;
  constexpr string_view<char> to_string() const {
    if (hovered) return "hover_event(hovered:1)";
    else return "hover_event(hovered:0)";
  }
};

struct key_event {
  key key;
  key_state state;
  constexpr string<char> to_string() const {
    return format("key_event(key:", internal::_get_key_name(key), ", state:", state, ")");
  }
};

struct wheel_event {
  short2 pos;
  short2 delta;
  key_state state;
  constexpr string<char> to_string() const {
    return format("wheel_event(pos:", pos, ", delta:", delta, ", state:", state, ")");
  }
};

///--------------------------------------------------------------------------///
/// MARK: control

class control : public interface {
public:
  struct slot : interface::slot {
    static constexpr float arbitrary_value = 4.0f;

    slotid parent_id;
    slotid window_id;

    float4 margin = float4::fill(arbitrary_value);
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

    string<wchar_t> tooltip;

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
    virtual result<float2> get_content_size() const { return {}; }

    virtual result<float2> get_necessary_size() const {
      if (auto cs = get_content_size()) {
        const auto required_size = float2(required_width.value_or(cs->x()), required_height.value_or(cs->y()));
        return vapply_r<float2>(yw::max, get_minimum_size(), required_size);
      } else return cs.relay();
    }

    virtual result<void> draw_background(interface::slot*) const { return {}; }
    virtual result<void> draw_overlay(interface::slot*) const { return {}; }
    virtual result<void> draw_foreground(interface::slot*) const { return {}; }

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
      return size - necessary_size; // layout-like controls may use
    }
  };
};
} // namespace yw

#endif
