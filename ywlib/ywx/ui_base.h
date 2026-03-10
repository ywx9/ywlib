#pragma once
#include "ywx/core.h"

namespace yw {
class window;
class window_slot;
}

namespace yw::ui {

class base {
public:
  class slot {
  public:
    slotset<slot>::slotid id;
    slotset<window_slot>::slotid window_id;

    float2 pos{};
    float2 size{};
    float2 radius{};

    color bg_color = colors::white;
    color border_color = colors::black;
    float border_width = 1.0f;

    std::wstring tooltip{};

    bool visible = true;
    bool enabled = true;
    bool focusable = true;

    function<void, event::button> on_button;
    function<void, event::hover> on_hover;
    function<void, event::move> on_move;
    function<void, event::key> on_key;
    function<void, bool> on_focus;
    function<void, event::button> on_click;

    virtual bool hit_test(float2 pt) const noexcept {
      return pt.x >= pos.x && pt.x < pos.x + size.x && pt.y >= pos.y && pt.y < pos.y + size.y;
    }

    virtual std::expected<void, error_trace> draw() const {
      brush.color(bg_color);
      if (auto res = fill_round_rectangle(pos, size, radius); !res) return unexpected_error(res.error());
      brush.color(border_color);
      if (auto res = draw_round_rectangle(pos, size, radius, border_width); !res) return unexpected_error(res.error());
      return {};
    }

    virtual void draw_focus() const {
      constexpr float2 margin = {2.5f, 2.5f};
      const auto focus_pos = pos - margin;
      const auto focus_size = size + margin * 2.0f;
      const auto focus_radius = radius + margin;
      brush.color(colors::black);
      draw_dashed_round_rectangle(focus_pos, focus_size, focus_radius, 1.0f);
    }

    virtual void button_event(event::button e) {
      if (enabled && on_button) on_button(e);
    }

    virtual void hover_event(event::hover e);

    virtual void move_event(event::move e) {
      if (focusable && on_move) on_move(e);
    }

    virtual void key_event(event::key e) {
      if (focusable && on_key) on_key(e);
    }

    virtual bool focus_event(bool focused) {
      if (focusable && on_focus) on_focus(focused);
      return focusable;
    }

    virtual void click_event(event::button e) {
      if (enabled && on_click) on_click(e);
    }

    virtual void char_event(wchar_t ch) {}
  };

protected:
  slotset<slot>::slotid _id;
  base(slotset<slot>::slotid id) : _id(id) {}

  slot* _base_slot_address() const noexcept;
  window_slot* _window_slot_address() const noexcept;

  void _clear();

  template<typename Mp> auto& unsafe_get(Mp mp) const {
    if (const auto s = dynamic_cast<class_type<Mp>*>(_base_slot_address())) return s->*mp;
    else throw std::runtime_error("invalid member access");
  }

  template<typename Ui, included_in<window, none> Window>
  static std::expected<tuple<Ui, typename Ui::slot*>, error_trace> add(Window& w, float2 Pos, float2 Size);

public:
  ~base() noexcept { _clear(); }
  base() noexcept = default;
  base(const base&) = delete;
  base& operator=(const base&) = delete;
  base(base&& other) noexcept : _id(std::exchange(other._id, {})) {}

  base& operator=(base&& other) noexcept {
    if (this == &other) return *this;
    _clear();
    _id = std::exchange(other._id, {});
    return *this;
  }

  explicit operator bool() const noexcept;
  const auto& id() const noexcept { return _id; }

  template<typename Ui> typename Ui::slot* slot_address(const Ui* self) const noexcept {
    return self ? dynamic_cast<typename Ui::slot*>(_base_slot_address()) : nullptr;
  }

  const auto& pos() const { return unsafe_get(&slot::pos); }
  const auto& size() const { return unsafe_get(&slot::size); }
  const auto& radius() const { return unsafe_get(&slot::radius); }
  const auto& bg_color() const { return unsafe_get(&slot::bg_color); }
  const auto& border_color() const { return unsafe_get(&slot::border_color); }
  const auto& border_width() const { return unsafe_get(&slot::border_width); }
  const auto& tooltip() const { return unsafe_get(&slot::tooltip); }
  const auto& on_button() const { return unsafe_get(&slot::on_button); }
  const auto& on_hover() const { return unsafe_get(&slot::on_hover); }
  const auto& on_move() const { return unsafe_get(&slot::on_move); }
  const auto& on_key() const { return unsafe_get(&slot::on_key); }
  const auto& on_focus() const { return unsafe_get(&slot::on_focus); }
  const auto& on_click() const { return unsafe_get(&slot::on_click); }

  void pos(float2 value) { unsafe_get(&slot::pos) = value; }
  void size(float2 value) { unsafe_get(&slot::size) = value; }
  void radius(float2 value) { unsafe_get(&slot::radius) = value; }
  void bg_color(const color& value) { unsafe_get(&slot::bg_color) = value; }
  void border_color(const color& value) { unsafe_get(&slot::border_color) = value; }
  void border_width(float value) { unsafe_get(&slot::border_width) = value; }
  template<stringable S> void tooltip(S&& s) { unsafe_get(&slot::tooltip) = unicode<wchar_t>(static_cast<S&&>(s)); }
  void on_button(function<void, event::button> f) { unsafe_get(&slot::on_button) = std::move(f); }
  void on_hover(function<void, event::hover> f) { unsafe_get(&slot::on_hover) = std::move(f); }
  void on_move(function<void, event::move> f) { unsafe_get(&slot::on_move) = std::move(f); }
  void on_key(function<void, event::key> f) { unsafe_get(&slot::on_key) = std::move(f); }
  void on_focus(function<void, bool> f) { unsafe_get(&slot::on_focus) = std::move(f); }
  void on_click(function<void, event::button> f) { unsafe_get(&slot::on_click) = std::move(f); }

  template<included_in<window&, none> Window>
  static std::expected<base, error_trace> add(Window&& w, float2 Pos, float2 Size) {
    if (auto res = base::add<base>(w, Pos, Size)) return std::move(yw::get<0>(*res));
    else return unexpected_error(res.error());
  }
};
} // namespace yw::ui
