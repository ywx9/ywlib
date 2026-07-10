#pragma once
#include <ywlib>

namespace yw {

struct modifiers {
  bool ctrl : 1;
  bool shift : 1;
  bool alt : 1;
  string<char> to_string() const {
    string<char> s("(ctrl:0, shift:0, alt:0)");
    s[6] = '0' + ctrl, s[15] = '0' + shift, s[22] = '0' + alt;
    return s;
  }
};
static_assert(sizeof(modifiers) == 1);

struct key {
  uint8_t code;

  friend constexpr bool operator==(key x, key y) noexcept = default;
  friend constexpr key operator&(key x, key y) noexcept { return key(x.code & y.code); }
  friend constexpr key operator|(key x, key y) noexcept { return key(x.code | y.code); }
  friend constexpr key operator^(key x, key y) noexcept { return key(x.code ^ y.code); }
  friend constexpr key& operator&=(key& x, key y) noexcept { return x.code &= y.code, x; }
  friend constexpr key& operator|=(key& x, key y) noexcept { return x.code |= y.code, x; }
  friend constexpr key& operator^=(key& x, key y) noexcept { return x.code ^= y.code, x; }

  bool pressed() const noexcept { return (::GetKeyState(int(code)) & 0x8000) != 0; }
  constexpr string_view<char> to_string() const noexcept {
    static constexpr const char* names[] = {
      // clang-format off
      "__0", "lbutton", "rbutton", "cancel", "mbutton", "xbutton1", "xbutton2", "__7", "back", "tab", "__10", "__11",
      "clear", "return", "__14", "__15", "shift", "control", "menu", "pause", "capital", "kana", "ime_on", "junja",
      "final", "hanja", "ime_off", "escape", "convert", "nonconvert", "accept", "modechange", "space", "prior", "next",
      "end", "home", "left", "up", "right", "down", "select", "print", "execute", "snapshot", "insert", "delete",
      "help", "__48", "__49", "__50", "__51", "__52", "__53", "__54", "__55", "__56", "__57", "__58", "__59", "__60",
      "__61", "__62", "__63", "__64", "__65", "__66", "__67", "__68", "__69", "__70", "__71", "__72", "__73", "__74",
      "__75", "__76", "__77", "__78", "__79", "__80", "__81", "__82", "__83", "__84", "__85", "__86", "__87", "__88",
      "__89", "__90", "lwin", "rwin", "apps", "__94", "sleep", "numpad0", "numpad1", "numpad2", "numpad3", "numpad4",
      "numpad5", "numpad6", "numpad7", "numpad8", "numpad9", "multiply", "add", "separator", "subtract", "decimal",
      "divide", "f1", "f2", "f3", "f4", "f5", "f6", "f7", "f8", "f9", "f10", "f11", "f12", "f13",  "f14", "f15", "f16",
      "f17", "f18", "f19", "f20", "f21", "f22", "f23", "f24", "navigation_view", "navigation_menu", "navigation_up",
      "navigation_down", "navigation_left", "navigation_right", "navigation_accept", "navigation_cancel", "numlock",
      "scroll", "oem_nec_equal", "oem_fj_masshou", "oem_fj_touroku", "oem_fj_loya", "oem_fj_roya", "__151", "__152",
      "__153", "__154", "__155", "__156", "__157", "__158", "__159", "lshift", "rshift", "lcontrol", "rcontrol",
      "lmenu", "rmenu", "browser_back", "browser_forward", "browser_refresh", "browser_stop", "browser_search",
      "browser_favorites", "browser_home", "volume_mute", "volume_down", "volume_up", "media_next_track",
      "media_prev_track", "media_stop", "media_play_pause", "launch_mail", "launch_media_select", "launch_app1",
      "launch_app2", "__184", "__185", "oem_1", "oem_plus", "oem_comma", "oem_minus", "oem_period", "oem_2", "oem_3",
      "__193", "__194", "gamepad_a", "gamepad_b", "gamepad_x", "gamepad_y", "gamepad_right_shoulder",
      "gamepad_left_shoulder", "gamepad_left_trigger", "gamepad_right_trigger", "gamepad_dpad_up", "gamepad_dpad_down",
      "gamepad_dpad_left", "gamepad_dpad_right", "gamepad_menu", "gamepad_view", "gamepad_left_thumbstick_button",
      "gamepad_right_thumbstick_button", "gamepad_left_thumbstick_up", "gamepad_left_thumbstick_down",
      "gamepad_left_thumbstick_right", "gamepad_left_thumbstick_left", "gamepad_right_thumbstick_up",
      "gamepad_right_thumbstick_down", "gamepad_right_thumbstick_right", "gamepad_right_thumbstick_left",
      "oem_4", "oem_5", "oem_6", "oem_7", "oem_8", "__224", "oem_ax", "oem_102", "ico_help", "ico_00", "processkey",
      "ico_clear", "packet", "__232", "oem_reset", "oem_jump", "oem_pa1", "oem_pa2", "oem_pa3", "oem_wsctrl",
      "oem_cusel", "oem_attn", "oem_finish", "oem_copy", "oem_auto", "oem_enlw", "oem_backtab", "attn", "crsel",
      "exsel", "ereof", "play", "zoom", "noname", "pa1", "oem_clear", "__255"}; // clang-format on
    return names[static_cast<uint8_t>(code)];
  }
};
static_assert(sizeof(key) == 1);

namespace keys {
inline constexpr key unknown{0};
inline constexpr key lbutton{VK_LBUTTON};
inline constexpr key rbutton{VK_RBUTTON};
inline constexpr key mbutton{VK_MBUTTON};
inline constexpr key xbutton1{VK_XBUTTON1};
inline constexpr key xbutton2{VK_XBUTTON2};

inline constexpr key backspace{VK_BACK};
inline constexpr key tab{VK_TAB};
inline constexpr key enter{VK_RETURN};
inline constexpr key shift{VK_SHIFT};
inline constexpr key ctrl{VK_CONTROL};
inline constexpr key alt{VK_MENU};
inline constexpr key caps_lock{VK_CAPITAL};
inline constexpr key escape{VK_ESCAPE};
inline constexpr key space{VK_SPACE};
inline constexpr key page_up{VK_PRIOR};
inline constexpr key page_down{VK_NEXT};
inline constexpr key end{VK_END};
inline constexpr key home{VK_HOME};
inline constexpr key left{VK_LEFT};
inline constexpr key up{VK_UP};
inline constexpr key right{VK_RIGHT};
inline constexpr key down{VK_DOWN};
inline constexpr key print_screen{VK_SNAPSHOT};
inline constexpr key insert{VK_INSERT};
inline constexpr key delete_{VK_DELETE};
inline constexpr key win{VK_LWIN};
inline constexpr key menu{VK_APPS};
inline constexpr key num_lock{VK_NUMLOCK};
inline constexpr key scroll_lock{VK_SCROLL};

inline constexpr key n0{'0'};
inline constexpr key n1{'1'};
inline constexpr key n2{'2'};
inline constexpr key n3{'3'};
inline constexpr key n4{'4'};
inline constexpr key n5{'5'};
inline constexpr key n6{'6'};
inline constexpr key n7{'7'};
inline constexpr key n8{'8'};
inline constexpr key n9{'9'};

inline constexpr key a{'A'};
inline constexpr key b{'B'};
inline constexpr key c{'C'};
inline constexpr key d{'D'};
inline constexpr key e{'E'};
inline constexpr key f{'F'};
inline constexpr key g{'G'};
inline constexpr key h{'H'};
inline constexpr key i{'I'};
inline constexpr key j{'J'};
inline constexpr key k{'K'};
inline constexpr key l{'L'};
inline constexpr key m{'M'};
inline constexpr key n{'N'};
inline constexpr key o{'O'};
inline constexpr key p{'P'};
inline constexpr key q{'Q'};
inline constexpr key r{'R'};
inline constexpr key s{'S'};
inline constexpr key t{'T'};
inline constexpr key u{'U'};
inline constexpr key v{'V'};
inline constexpr key w{'W'};
inline constexpr key x{'X'};
inline constexpr key y{'Y'};
inline constexpr key z{'Z'};

inline constexpr key f1{VK_F1};
inline constexpr key f2{VK_F2};
inline constexpr key f3{VK_F3};
inline constexpr key f4{VK_F4};
inline constexpr key f5{VK_F5};
inline constexpr key f6{VK_F6};
inline constexpr key f7{VK_F7};
inline constexpr key f8{VK_F8};
inline constexpr key f9{VK_F9};
inline constexpr key f10{VK_F10};
inline constexpr key f11{VK_F11};
inline constexpr key f12{VK_F12};

inline constexpr key hiphen{VK_OEM_MINUS};
inline constexpr key semicolon{VK_OEM_1};
inline constexpr key comma{VK_OEM_COMMA};
inline constexpr key period{VK_OEM_PERIOD};
inline constexpr key slash{VK_OEM_2};
} // namespace keys

#pragma pack(push, 1)

struct button_event {
  short2 pos;
  yw::key key;
  yw::modifiers mods;
  bool down;
  string<char> to_string() const {
    if (down) return format("button_event(pos:", pos, ", key:", key, ", mods:", mods, ", down)");
    else return format("button_event(pos:", pos, ", key:", key, ", mods:", mods, ", up)");
  }
};
static_assert(sizeof(button_event) <= 8);

struct drag_event {
  short2 delta;
  yw::key key;
  yw::modifiers mods;
  string<char> to_string() const { return format("drag_event(delta:", delta, ", key:", key, ", mods:", mods, ")"); }
};
static_assert(sizeof(drag_event) <= 8);

struct hover_event {
  short2 pos;
  enum class type : uint8_t { enter = 0x1, leave = 0x2, move = 0x3 } type;
  bool enter() const { return type == type::enter; }
  bool leave() const { return type == type::leave; }
  bool move() const { return type == type::move; }
  string<char> to_string() const {
    if (enter()) return format("hover_event(pos:", pos, ", type:enter)");
    else if (leave()) return format("hover_event(pos:", pos, ", type:leave)");
    else if (move()) return format("hover_event(pos:", pos, ", type:move)");
    else return format("hover_event(pos:", pos, ", type:unknown)");
  }
};
static_assert(sizeof(hover_event) <= 8);

struct key_event {
  yw::key key;
  yw::modifiers mods;
  bool down;
  string<char> to_string() const {
    if (down) return format("key_event(key:", key, ", mods:", mods, ", down)");
    else return format("key_event(key:", key, ", mods:", mods, ", up)");
  }
};
static_assert(sizeof(key_event) <= 8);

struct move_event {
  short2 pos;
  short2 delta;
  string<char> to_string() const { return format("move_event(pos:", pos, ", delta:", delta, ")"); }
};
static_assert(sizeof(move_event) <= 8);

struct wheel_event {
  short2 pos;
  short delta;
  yw::modifiers mods;
  bool horizontal;
  string<char> to_string() const {
    if (horizontal) return format("wheel_event(pos:", pos, ", delta:", delta, ", mods:", mods, ", horizontal)");
    else return format("wheel_event(pos:", pos, ", delta:", delta, ", mods:", mods, ", vertical)");
  }
};
static_assert(sizeof(wheel_event) <= 8);

#pragma pack(pop)
} // namespace yw

namespace std {

template<> struct formatter<yw::modifiers> {
  formatter<yw::string<char>> fmt;
  constexpr auto parse(auto& ctx) { return fmt.parse(ctx); }
  auto format(const yw::modifiers& m, auto& ctx) const { return fmt.format(m.to_string(), ctx); }
};

template<> struct formatter<yw::key> {
  formatter<yw::string<char>> fmt;
  constexpr auto parse(auto& ctx) { return fmt.parse(ctx); }
  auto format(const yw::key& k, auto& ctx) const { return fmt.format(k.to_string(), ctx); }
};

template<> struct formatter<yw::button_event> {
  formatter<yw::string<char>> fmt;
  constexpr auto parse(auto& ctx) { return fmt.parse(ctx); }
  auto format(const yw::button_event& e, auto& ctx) const { return fmt.format(e.to_string(), ctx); }
};

template<> struct formatter<yw::drag_event> {
  formatter<yw::string<char>> fmt;
  constexpr auto parse(auto& ctx) { return fmt.parse(ctx); }
  auto format(const yw::drag_event& e, auto& ctx) const { return fmt.format(e.to_string(), ctx); }
};

template<> struct formatter<yw::hover_event> {
  formatter<yw::string<char>> fmt;
  constexpr auto parse(auto& ctx) { return fmt.parse(ctx); }
  auto format(const yw::hover_event& e, auto& ctx) const { return fmt.format(e.to_string(), ctx); }
};

template<> struct formatter<yw::key_event> {
  formatter<yw::string<char>> fmt;
  constexpr auto parse(auto& ctx) { return fmt.parse(ctx); }
  auto format(const yw::key_event& e, auto& ctx) const { return fmt.format(e.to_string(), ctx); }
};

template<> struct formatter<yw::move_event> {
  formatter<yw::string<char>> fmt;
  constexpr auto parse(auto& ctx) { return fmt.parse(ctx); }
  auto format(const yw::move_event& e, auto& ctx) const { return fmt.format(e.to_string(), ctx); }
};

template<> struct formatter<yw::wheel_event> {
  formatter<yw::string<char>> fmt;
  constexpr auto parse(auto& ctx) { return fmt.parse(ctx); }
  auto format(const yw::wheel_event& e, auto& ctx) const { return fmt.format(e.to_string(), ctx); }
};
} // namespace std
