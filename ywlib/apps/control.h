#pragma once
#include <apps/bitmap.h>
#include <apps/key.h>
#include <apps/text.h>
#include <core/array.h>
#include <core/core.h>
#include <core/format.h>
#include <core/function.h>
#include <core/heap.h>
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
/// MARK: color_theme

struct color_theme {
  color canvas = color(0xf0f0f0);        // ex) background of window
  color surface = color(0xf8f8f8);       // ex) background of control
  color surface_popup = color(0xffffff); // ex) background of tooltip
  color outline = colors::black;         // ex) border of control
  color part = colors::gray;             // ex) button of checkbox, thumb of scrollbar
  color text = colors::black;            // ex) text, icon
  color text_muted = colors::gray;       // ex) placeholder text
  color accent = colors::dodgerblue;     // ex) focus, selection
  color warning = colors::orange;
  color error = colors::red;
  color success = colors::green;
};

///--------------------------------------------------------------------------///
/// MARK: events

struct button_event {
  short2 pos;
  key key;
  key_state state;
  constexpr string<char> to_string() const {
    return format("button_event(pos:", pos, ", key:", key.to_string(), ", state:", state, ")");
  }
};

struct drag_event {
  short2 delta;
  key key;
  key_state state;
  constexpr string<char> to_string() const {
    return format("drag_event(delta:", delta, ", key:", key.to_string(), ", state:", state, ")");
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
  enum class state : uint8_t {
    leave = 0,
    enter = 1,
    hover = 3,
  };
  using enum state;
  short2 pos;
  state state;
  constexpr string_view<char> to_string() const {
    constexpr auto n = 5;
    auto s = format("hover_event(pos:", pos, ", state:leave)");
    if (state == enter) std::ranges::copy_n("enter", n, s.end() - n - 1);
    if (state == hover) std::ranges::copy_n("hover", n, s.end() - n - 1);
    return s;
  }
};

struct key_event {
  key key;
  key_state state;
  constexpr string<char> to_string() const { return format("key_event(key:", key.to_string(), ", state:", state, ")"); }
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
/// MARK: window system

class window;
class control;

namespace window_system {
inline slotset<control*> controls{};
/// slotidからcontrol*を取得する
inline control* get_control(slotset<control*>::slotid id) {
  if (const auto cpp = controls.get(id)) return *cpp;
  return nullptr;
}
/// control_layerの再描画が必要な状態にする
inline void make_dirty(HWND hwnd);
/// control_layerのレイアウト再計算が必要な状態にする
inline void make_messy(HWND hwnd);
/// ウィンドウのcolor_themeを取得する
inline const color_theme* get_color_theme(HWND hwnd);
/// ウィンドウのルートコントロールとしてコントロールを設定する
inline result<void> attach_control(control& c, window& w);
} // namespace window_system

///--------------------------------------------------------------------------///
/// MARK: control

class control {
  friend class window;
  using slotid = slotset<control*>::slotid;

public:
  static constexpr float arbitrary_value = 4.0f;
  const_property<slotid, control> id = slotid{};

protected:
  HWND _window = nullptr;
  comptr<ID2D1Geometry> _geometry;
  float4 _margin = float4::fill(arbitrary_value);
  float2 _served_origin;
  float2 _served_area;
  float2 _current_pos;
  float2 _current_size;
  float2 _desired_size;
  /// サイズが指定されているか
  bool2 _desired;
  /// 空き領域がある場合に拡張するか
  bool2 _grow = false;
  /// 与えられた領域内での配置方法
  yw::alignment _alignment = yw::alignment::center;
  /// ジオメトリの再設定が必要か
  bool _geometry_dirty = false;
  bool _visible = true;
  bool _enabled = true;
  /// 必要な最小サイズを計算する
  virtual result<float2> _calculate_minimum_size() { return _desired_size * _desired; }
  /// 必要な最小領域サイズを計算する
  virtual result<float2> _calculate_minimum_area() {
    if (!_visible) return float2{};
    if (auto res = _calculate_minimum_size()) return *res + _margin.xy() + _margin.zw();
    else return res.relay();
  }
  /// 与えられた領域に基づいて配置を更新する
  virtual result<void> _update_layout() {
    const auto pos = _served_origin + _margin.xy();
    const auto maximum_size = _served_area - _margin.xy() - _margin.zw();
    float2 minimum_size = maximum_size * _grow;
    if (auto res = _calculate_minimum_size()) minimum_size = vapply_r<float2>(yw::max, *res, minimum_size);
    else return res.relay();
    const auto difference = maximum_size - minimum_size;
    constexpr float c[]{0.5f, 0.0f, 1.0f};
    const auto a = uint8_t(_alignment);
    const float2 cc{c[a % 3], c[a / 4 % 3]};
    _current_pos = pos + difference * cc;
    _current_size = minimum_size;
    return {};
  }
  /// 新たに領域を設定して配置を更新する
  virtual result<void> _update_layout(float2 Origin, float2 Area) {
    _served_origin = Origin;
    _served_area = Area;
    if (auto res = _update_layout()) return {};
    else return res.relay();
  }
  /// 現在の配置でジオメトリを更新する
  virtual result<void> _update_geometry() {
    ID2D1RectangleGeometry* geometry = nullptr;
    const auto left_top = _current_pos;
    const auto right_bottom = _current_pos + _current_size;
    D2D1_RECT_F rect{left_top.x(), left_top.y(), right_bottom.x(), right_bottom.y()};
    if (const auto hr = d2d::factory()->CreateRectangleGeometry(&rect, &geometry); FAILED(hr))
      return error(errors::operation_failed, "CreateRectangleGeometry failed");
    _geometry.reset(geometry);
    _geometry_dirty = false;
    return {};
  }
  /// 描画する
  virtual result<void> _draw(window&) {
    if (_geometry_dirty) {
      if (auto res = _update_layout(); !res) return res.relay();
      if (auto res = _update_geometry(); !res) return res.relay();
      _geometry_dirty = false;
    }
    return {};
  }
  /// イベント処理用の仮想関数
  virtual result<bool> _handle_button_event(window&, button_event) { return false; }
  virtual result<bool> _handle_char_event(window&, wchar_t) { return false; }
  virtual result<bool> _handle_click_event(window&, button_event) { return false; }
  virtual result<bool> _handle_double_click_event(window&, button_event) { return false; }
  virtual result<bool> _handle_drag_event(window&, drag_event) { return false; }
  virtual result<bool> _handle_focus_event(window&, focus_event) { return false; }
  virtual result<bool> _handle_hover_event(window&, hover_event) { return false; }
  virtual result<bool> _handle_key_event(window&, key_event) { return false; }
  virtual result<bool> _handle_wheel_event(window&, wheel_event) { return false; }

  control() noexcept : id(window_system::controls.emplace(this)) {}

  void _clear() noexcept;
  virtual void _clear_state(window& win) noexcept;

  void _move_from(control&& o) noexcept {
    _window = exchange(o._window, {});
    id = exchange(o.id.ref(), {});
    if (const auto sp = window_system::controls.get(id)) *sp = this;
    _geometry = move(o._geometry);
    _served_origin = o._served_origin;
    _served_area = o._served_area;
    _margin = o._margin;
    _current_pos = o._current_pos;
    _current_size = o._current_size;
    _desired_size = o._desired_size;
    _desired = o._desired;
    _grow = o._grow;
    _alignment = o._alignment;
    _geometry_dirty = o._geometry_dirty;
    _visible = o._visible;
    _enabled = o._enabled;
  }

  result<void> _attach(control&) {
    return error(errors::invalid_operation, "This control cannot accept other controls");
  }

  /// ウィンドウ座標がこのコントロールの領域内に含まれているかを判定する
  virtual slotid _hit_test(float2 Pt) const {
    if (!_visible || !_geometry) return {};
    BOOL contains = FALSE;
    if (const auto hr = _geometry->FillContainsPoint({Pt.x(), Pt.y()}, nullptr, &contains); FAILED(hr)) return {};
    return contains ? id() : slotid{};
  }
  /// タブストップを探索する
  virtual slotid _find_tab_stop(slotid Current, bool Backward, bool& Found) const {
    if (!_visible || !_enabled || !focusable()) return {};
    if (Current == id()) Found = true;
    else if (Found) return id();
    return {};
  }

  /// ウィンドウに対する状態を取得する。実装はwindow.hに。
  bool _focused(window& win) const;
  bool _hovered(window& win) const;
  bool _pressed(window& win) const;
  static color _get_disabled_color(window& win, const color& c);

public:
  virtual ~control() { _clear(); }
  control(const control&) = delete;
  control& operator=(const control&) = delete;
  control(control&& o) noexcept { _move_from(move(o)); }
  control& operator=(control&& o) noexcept {
    if (this == &o) return *this;
    _clear();
    _move_from(move(o));
    return *this;
  }

  virtual result<void> attach_to(window& w);
  virtual result<void> attach_to(control& c);

  result<bool> focus();

  bool visible() const noexcept { return _visible; }
  result<void> visible(bool value);
  bool enabled() const noexcept { return _enabled; }
  result<void> enabled(bool value);

  float4 margin() const noexcept { return _margin; }
  result<void> margin(float4 Margin) {
    _margin = Margin;
    window_system::make_messy(_window);
    return {};
  }

  float2 pos() const noexcept { return _current_pos; }

  float2 size() const noexcept { return _current_size; }
  result<void> size(float2 Size) {
    _desired_size = Size;
    _desired = bool2(true, true);
    window_system::make_messy(_window);
    return {};
  }
  result<void> size(is_none auto) {
    _desired = bool2(false, false);
    window_system::make_messy(_window);
    return {};
  }

  float width() const noexcept { return _current_size.x(); }
  result<void> width(float1 Width) {
    _desired_size.x() = Width.x();
    _desired.x() = true;
    window_system::make_messy(_window);
    return {};
  }
  result<void> width(is_none auto) {
    _desired.x() = false;
    window_system::make_messy(_window);
    return {};
  }

  float height() const noexcept { return _current_size.y(); }
  result<void> height(float1 Height) {
    _desired_size.y() = Height.x();
    _desired.y() = true;
    window_system::make_messy(_window);
    return {};
  }
  result<void> height(is_none auto) {
    _desired.y() = false;
    window_system::make_messy(_window);
    return {};
  }

  bool2 grow() const noexcept { return _grow; }
  result<void> grow(bool2 Grow) {
    _grow = Grow;
    window_system::make_messy(_window);
    return {};
  }

  yw::alignment alignment() const noexcept { return _alignment; }
  result<void> alignment(yw::alignment Alignment) {
    _alignment = Alignment;
    _geometry_dirty = true;
    window_system::make_dirty(_window);
    return {};
  }

  virtual bool focusable() const { return false; }
  virtual bool interactive() const { return false; }
};
} // namespace yw

#endif
