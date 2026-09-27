#pragma once
#include <base/color.h>
#include <base/comptr.h>
#include <base/slotset.h>
#include <base/optional.h>
#include <core/core.h>
#include <core/property.h>

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <d2d1_3.h>

#ifdef interface
#undef interface
#endif

namespace yw {

// clang-format off

enum class key : uint8_t {
  unknown = 0,

  lbutton = VK_LBUTTON, rbutton = VK_RBUTTON, mbutton = VK_MBUTTON, xbutton1 = VK_XBUTTON1, xbutton2 = VK_XBUTTON2,

  backspace = VK_BACK,
  tab = VK_TAB,
  enter = VK_RETURN,
  shift = VK_SHIFT,
  ctrl = VK_CONTROL,
  alt = VK_MENU,
  caps_lock = VK_CAPITAL,
  escape = VK_ESCAPE,
  space = VK_SPACE,
  page_up = VK_PRIOR,
  page_down = VK_NEXT,
  end = VK_END,
  home = VK_HOME,
  left = VK_LEFT,
  up = VK_UP,
  right = VK_RIGHT,
  down = VK_DOWN,
  print_screen = VK_SNAPSHOT,
  insert = VK_INSERT,
  delete_ = VK_DELETE,
  win = VK_LWIN,
  menu = VK_APPS,
  num_lock = VK_NUMLOCK,
  scroll_lock = VK_SCROLL,

  n0 = '0', n1 = '1', n2 = '2', n3 = '3', n4 = '4', n5 = '5', n6 = '6', n7 = '7', n8 = '8', n9 = '9',

  a = 'A', b = 'B', c = 'C', d = 'D', e = 'E', f = 'F', g = 'G', h = 'H', i = 'I', j = 'J', k = 'K', l = 'L', m = 'M',
  n = 'N', o = 'O', p = 'P', q = 'Q', r = 'R', s = 'S', t = 'T', u = 'U', v = 'V', w = 'W', x = 'X', y = 'Y', z = 'Z',

  f1 = VK_F1, f2 = VK_F2, f3 = VK_F3, f4 = VK_F4, f5 = VK_F5, f6 = VK_F6,
  f7 = VK_F7, f8 = VK_F8, f9 = VK_F9, f10 = VK_F10, f11 = VK_F11, f12 = VK_F12,

  hiphen = VK_OEM_MINUS, semicolon = VK_OEM_1, comma = VK_OEM_COMMA, period = VK_OEM_PERIOD, slash = VK_OEM_2,
};

// clang-format on

struct key_state {
  bool down : 1;
  bool shift : 1;
  bool ctrl : 1;
  bool alt : 1;
  constexpr string<char> to_string() const {
    auto s = string<char>("(down:0, shift:0, ctrl:0, alt:0)");
    s[6] = char('0' + down);
    s[15] = char('0' + shift);
    s[23] = char('0' + ctrl);
    s[30] = char('0' + alt);
    return s;
  }
};

namespace internal {
constexpr string_view<char> _get_key_name(const key k) {
#ifdef __cpp_lib_meta
  template for (constexpr auto e : std::define_static_array(std::meta::numerators_of(^^key)))
    if (k == [:e:]) return std::meta::identifier_of(e);
#endif
  return "unknown";
}
}

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

enum class alignment : unsigned char {
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
    float2 required_size;
    float2 provided_pos;
    float2 provided_area;
    float2 pos;
    float2 size;
    float2 radius = float2::fill(arbitrary_value);
    float2 minimum_size = float2::fill(arbitrary_value);
    ui::alignment align = ui::center;
    vector2<ui::size_policy> policy{ui::free, ui::free};
    comptr<ID2D1Geometry> geometry;

    string<preferred_char> tooltip;
    optional<color> background_color;
    optional<color> border_color;
    float border_thickness = 1.0f;

    function<bool, yw::button_event> button_event;
    function<bool, yw::cursor_event> cursor_event;
    function<bool, yw::drag_event> drag_event;
    function<bool, yw::focus_event> focus_event;
    function<bool, yw::hover_event> hover_event;
    function<bool, yw::key_event> key_event;
    function<bool, yw::wheel_event> wheel_event;
    bool geometry_dirty = false;
    bool visible = true;
    bool enabled = true;

  };
};

} // namespace yw

#endif
