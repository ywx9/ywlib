#pragma once
#include <ywx/ui_edit.h>
#include <ywx/window.h>

namespace yw::ui {

template<typename T> concept spinbox_value = arithmetic<T> && !is_bool<T> && !char_type<T>;

template<spinbox_value T> class spinbox : public edit {
public:
  struct slot : edit::slot {
    T value{};
    T minimum = std::numeric_limits<T>::lowest();
    T maximum = std::numeric_limits<T>::max();
    T step = T(1);
    optional<color> arrow_color;
    optional<color> button_color;
    float arrow_thickness = 1.0f;
    float button_width = common_size_value;
    int pressed_button = 0;
    int hovered_button = 0;
    bool syncing_string = false;

    function<bool, T> change_event{};

    //-- override functions --//

    virtual color get_arrow_color(const interface::slot* Window) const noexcept {
      if (arrow_color) return *arrow_color;
      if (auto theme = get_color_theme(Window)) return (*theme)->text;
      return colors::transparent;
    }

    virtual color get_button_color(const interface::slot* Window) const noexcept {
      if (button_color) return *button_color;
      if (auto theme = get_color_theme(Window)) return (*theme)->part;
      return colors::transparent;
    }

    virtual std::expected<void, error> draw_backcontent(interface::slot* Window) override {
      if (auto res = edit::slot::draw_backcontent(Window); !res) return res.error().relay();
      const auto origin = pos + float2(size.x - button_width, 0.0f);
      brush::color(get_button_color(Window));
      if (auto res = fill_rectangle(origin, float2(button_width, size.y)); !res) return res.error().relay();
      return {};
    }

    virtual std::expected<void, error> draw_overlay(interface::slot* Window) override {
      std::optional<int> pressed_button_;
      const auto wsp = static_cast<window::slot*>(Window);
      if (!wsp) return {};
      if (id == wsp->mouse_capture_control_id && wsp->press_overlay_color.a > 0.0f) {
        pressed_button_ = pressed_button;
        brush::color(wsp->press_overlay_color);
        if (auto res = fill_part(pressed_button); !res) return res.error().relay();
      }
      if (id == wsp->hovered_control_id && wsp->hover_overlay_color.a > 0.0f) {
        if (!pressed_button_ || *pressed_button_ != hovered_button) {
          brush::color(wsp->hover_overlay_color);
          if (auto res = fill_part(hovered_button); !res) return res.error().relay();
        }
      }
      return {};
    }

    virtual std::expected<void, error> draw_forecontent(interface::slot* Window) override {
      const auto origin = pos + float2(size.x - button_width, 0.0f);
      const auto half_y = size.y * 0.5f;
      const auto arrow_size = float2(button_width, half_y);
      const auto origin2 = origin.add<1>(half_y);
      if (auto res = edit::slot::draw_forecontent(Window); !res) return res.error().relay();
      brush::color(get_border_color(Window));
      if (auto res = stroke_line(origin, origin.add<1>(size.y), border_thickness); !res) return res.error().relay();
      if (auto res = stroke_line(origin2, origin2.add<0>(button_width), border_thickness); !res)
        return res.error().relay();
      brush::color(get_arrow_color(Window));
      if (auto res = draw_arrow<ui::top>(origin, arrow_size, arrow_thickness); !res) return res.error().relay();
      if (auto res = draw_arrow<ui::bottom>(origin2, arrow_size, arrow_thickness); !res) return res.error().relay();
      return {};
    }

    virtual std::expected<float2, error> get_necessary_size() const override {
      return float2(button_width * 4.0f, button_width * 2.0f);
    }

    virtual void reset_state() override {
      if (pressed_button != 0) {
        pressed_button = 0;
        make_dirty();
      }
      edit::slot::reset_state();
    }

    virtual bool handle_button_event(yw::button_event e) override {
      if (!enabled || !visible || e.key != keys::lbutton) return edit::slot::handle_button_event(e);
      const auto hit = button_at(float2(float(e.pos.x), float(e.pos.y)));
      if (hit == 0) {
        if (!e.down && pressed_button != 0) {
          pressed_button = 0;
          make_dirty();
          return true;
        }
        return edit::slot::handle_button_event(e);
      }
      if (e.down) {
        pressed_button = hit;
        const bool handled = step_by(hit, e.mods.shift ? 10 : 1);
        make_dirty();
        return handled;
      } else if (pressed_button != 0) {
        pressed_button = 0;
        make_dirty();
      }
      return true;
    }

    virtual bool handle_click_event(yw::button_event e) override {
      if (!enabled || !visible || e.down || e.key != keys::lbutton) return edit::slot::handle_click_event(e);
      return button_at(float2(float(e.pos.x), float(e.pos.y))) != 0 || edit::slot::handle_click_event(e);
    }

    virtual bool handle_double_click_event(yw::button_event e) override {
      if (button_at(float2(float(e.pos.x), float(e.pos.y))) != 0) return true;
      return edit::slot::handle_double_click_event(e);
    }

    virtual bool handle_drag_event(yw::drag_event e) override {
      if (pressed_button != 0) return true;
      return edit::slot::handle_drag_event(e);
    }

    virtual bool handle_focus_event(yw::focus_event e) override {
      if (!e.focused) {
        if (pressed_button != 0) {
          pressed_button = 0;
          make_dirty();
        }
        commit_string();
      }
      return edit::slot::handle_focus_event(e);
    }

    virtual bool handle_hover_event(yw::hover_event e) override {
      if (!enabled || !visible) return false;
      const auto hit = button_at(float2(float(e.pos.x), float(e.pos.y)));
      if (hit != hovered_button) {
        hovered_button = hit;
        make_dirty();
      }
      return true;
    }

    virtual bool handle_key_event(yw::key_event e) override {
      if (!enabled || !visible) return false;
      const bool spin_key = e.key == keys::up || e.key == keys::down || e.key == keys::page_up ||
                            e.key == keys::page_down || e.key == keys::home || e.key == keys::end;
      if (!e.down) {
        if (spin_key) return true;
        return edit::slot::handle_key_event(e);
      }
      if (e.key == keys::up) return step_by(+1, e.mods.shift ? 10 : 1);
      if (e.key == keys::down) return step_by(-1, e.mods.shift ? 10 : 1);
      if (e.key == keys::page_up) return step_by(+1, 10);
      if (e.key == keys::page_down) return step_by(-1, 10);
      if (e.key == keys::home) return set_value(minimum);
      if (e.key == keys::end) return set_value(maximum);
      return edit::slot::handle_key_event(e);
    }

    virtual bool handle_wheel_event(yw::wheel_event e) override {
      if (!enabled || !visible || e.horizontal || e.delta == 0) return false;
      return step_by(e.delta > 0 ? +1 : -1, e.mods.shift ? 10 : 1);
    }

    //-- shared functions --//

    int button_at(float2 Pt) const noexcept {
      const float left = pos.x + size.x - button_width;
      if (button_width <= 0.0f || Pt.x < left || Pt.x > pos.x + size.x || Pt.y < pos.y || Pt.y > pos.y + size.y)
        return 0;
      return Pt.y < pos.y + size.y * 0.5f ? +1 : -1;
    }

    void commit_string() {
      if (syncing_string) return;
      const auto& input = text.string();
      yw::string<char> narrow;
      narrow.reserve(input.size());
      for (const auto c : input) {
        if (c < 0x20 || c >= 0x7f) {
          narrow.clear();
          break;
        }
        narrow.push_back(static_cast<char>(c));
      }
      if (!narrow.empty()) {
        T result{};
        const auto begin = narrow.data(), end = begin + narrow.size();
        const auto [ptr, ec] = std::from_chars(begin, end, result);
        if (ec == std::errc() && ptr == end) set_value(result, true);
        else sync_string();
      } else sync_string();
    }

    std::expected<void, error> fill_part(int Part) {
      if (Part != 0) {
        const auto origin = pos.add<0>(size.x - button_width);
        const auto half_y = size.y * 0.5f;
        if (Part == 1) {
          if (auto res = fill_rectangle(origin, float2(button_width, half_y)); !res) return res.error().relay();
        } else if (Part == -1)
          if (auto res = fill_rectangle(origin.add<1>(half_y), float2(button_width, half_y)); !res)
            return res.error().relay();
      } else if (auto res = fill_rectangle(pos, size.add<0>(-button_width)); !res) return res.error().relay();
      return {};
    }

    static bool is_number_char(wchar_t c) noexcept {
      if (L'0' <= c && c <= L'9') return true;
      if constexpr (signed_integral<T> || floating<T>)
        if (c == L'+' || c == L'-') return true;
      if constexpr (floating<T>)
        if (c == L'.' || c == L'e' || c == L'E') return true;
      return false;
    }

    T normalize_value(T v) const noexcept {
      if (v <= minimum) return minimum;
      if (v >= maximum) return maximum;
      if constexpr (floating<T>) {
        if (step > T{} && maximum > minimum) {
          const auto n = std::round(static_cast<long double>(v) / static_cast<long double>(step));
          const auto w = yw::clamp(static_cast<T>(n * static_cast<long double>(step)), minimum, maximum);
          if (w == T{}) return T{};
          return w;
        } else return T{};
      } else return yw::clamp(v, minimum, maximum);
    }

    void sync_string() {
      syncing_string = true;
      text.string(yw::format<wchar_t>(value));
      caret = yw::clamp(caret, 0, text.string().size());
      anchor = yw::clamp(anchor, 0, text.string().size());
      make_messy();
      syncing_string = false;
    }

    bool set_value(T v, bool Notify = true) {
      const auto next = normalize_value(v);
      if (value == next) {
        sync_string();
        return true;
      }
      value = next;
      sync_string();
      return Notify && change_event ? change_event(value) : true;
    }

    T stepped_value(T Base, int Direction) const noexcept {
      if (Direction == 0) return Base;
      if constexpr (integral<T>) {
        if (Direction > 0) {
          if (step > 0 && Base > maximum - step) return maximum;
          return static_cast<T>(Base + step);
        } else {
          if (step > 0 && Base < minimum + step) return minimum;
          return static_cast<T>(Base - step);
        }
      } else {
        const auto next =
          static_cast<long double>(Base) + static_cast<long double>(step) * static_cast<long double>(Direction);
        return normalize_value(static_cast<T>(next));
      }
    }

    bool step_by(int Direction, int Multiplier = 1) {
      if (Direction == 0) return true;
      commit_string();
      auto next = value;
      for (int i = 0; i < Multiplier; ++i) next = stepped_value(next, Direction);
      return set_value(next);
    }
  };

  using edit::attached;
  using edit::initialized;
  class proxy : public edit::proxy {
    friend class spinbox;
    using edit::proxy::proxy;
    spinbox::slot* _get_slot() const noexcept { return static_cast<spinbox::slot*>(_slot); }

  public:
    //-- getter --//

    float4 padding() const&& noexcept { return _get_slot()->padding.template add<2>(-_get_slot()->button_width); }
    T value() const&& noexcept { return _get_slot()->value; }
    T minimum() const&& noexcept { return _get_slot()->minimum; }
    T maximum() const&& noexcept { return _get_slot()->maximum; }
    T step() const&& noexcept { return _get_slot()->step; }
    float button_width() const&& noexcept { return _get_slot()->button_width; }
    color arrow_color() const&& noexcept {
      return _get_slot()->get_arrow_color(interface::slot::slots.get(_get_slot()->window_id));
    }
    color button_color() const&& noexcept {
      return _get_slot()->get_button_color(interface::slot::slots.get(_get_slot()->window_id));
    }
    const auto& change_event() const&& noexcept { return _get_slot()->change_event; }

    //-- setter --//

    auto padding(this auto&& Self, float4 Padding) noexcept {
      Self._get_slot()->padding = Padding.add<2>(Self._get_slot()->button_width);
      Self._messy = true;
      return std::move(Self);
    }

    auto value(this auto&& Self, T Value) noexcept {
      Self._get_slot()->set_value(Value);
      return std::move(Self);
    }

    auto range(this auto&& Self, T Min, T Max) noexcept {
      if (Max < Min) {
        error(errors::invalid_argument, "spinbox range maximum must be greater than or equal to minimum").fizzle_out();
        return std::move(Self);
      }
      Self._get_slot()->minimum = Min;
      Self._get_slot()->maximum = Max;
      Self._get_slot()->set_value(Self._get_slot()->value);
      return std::move(Self);
    }

    auto minimum(this auto&& Self, T Value) noexcept {
      if (Self._get_slot()->maximum < Value) {
        error(errors::invalid_argument, "spinbox minimum must be less than or equal to maximum").fizzle_out();
        return std::move(Self);
      }
      Self._get_slot()->minimum = Value;
      Self._get_slot()->set_value(Self._get_slot()->value);
      return std::move(Self);
    }

    auto maximum(this auto&& Self, T Value) noexcept {
      if (Value < Self._get_slot()->minimum) {
        error(errors::invalid_argument, "spinbox maximum must be greater than or equal to minimum").fizzle_out();
        return std::move(Self);
      }
      Self._get_slot()->maximum = Value;
      Self._get_slot()->set_value(Self._get_slot()->value);
      return std::move(Self);
    }

    auto step(this auto&& Self, T Step) noexcept {
      if (Step <= T{}) error(errors::invalid_argument, "spinbox step must be positive").fizzle_out();
      else Self._get_slot()->step = Step, Self._get_slot()->set_value(Self._get_slot()->value, false);
      return std::move(Self);
    }

    auto button_width(this auto&& Self, float1 Width) noexcept {
      if (Width.x <= 0.0f) {
        error(errors::invalid_argument, "spinbox button_width must be positive").fizzle_out();
        return std::move(Self);
      }
      Self._get_slot()->padding.z -= Self._get_slot()->button_width - Width.x;
      Self._get_slot()->button_width = Width.x;
      Self._messy = true;
      return std::move(Self);
    }

    auto arrow_color(this auto&& Self, const color& Color) noexcept {
      Self._get_slot()->arrow_color = Color;
      Self._dirty = true;
      return std::move(Self);
    }

    auto arrow_color(this auto&& Self, none) noexcept {
      Self._get_slot()->arrow_color = none();
      Self._dirty = true;
      return std::move(Self);
    }

    auto button_color(this auto&& Self, const color& Color) noexcept {
      Self._get_slot()->button_color = Color;
      Self._dirty = true;
      return std::move(Self);
    }

    auto button_color(this auto&& Self, none) noexcept {
      Self._get_slot()->button_color = none();
      Self._dirty = true;
      return std::move(Self);
    }

    auto change_event(this auto&& Self, function<bool, T> Event) noexcept {
      Self._get_slot()->change_event = std::move(Event);
      return std::move(Self);
    }
  };

  spinbox() noexcept = default;

  spinbox(derived_from<interface> auto& Parent, const source_line& sl = here()) {
    if (auto res = create(Parent)) *this = std::move(*res);
    else res.error().add_footprint().go_off(sl);
  }

  static std::expected<spinbox, error> create() {
    spinbox s;
    spinbox::slot* sp;
    if (auto res = create_control<spinbox>()) sp = *res;
    else return res.error().relay();
    s._id = sp->id;
    sp->policy = {ui::free, ui::fit};
    sp->text_align = alignment::right;
    sp->filter = [](wchar_t c) { return slot::is_number_char(c); };
    sp->padding.z += sp->button_width;
    const auto spinbox_id = sp->id;
    sp->enter_event = [spinbox_id](yw::key_event) {
      if (const auto sp = get_slot<spinbox>(spinbox_id)) sp->commit_string();
      return true;
    };
    sp->sync_string();
    return s;
  }

  static std::expected<spinbox, error> create(derived_from<interface> auto& Parent) {
    auto res = create();
    if (!res) return res.error().relay();
    if (auto attached = res->attach(Parent); !attached) return attached.error().relay();
    return res;
  }

  yw_control_getter_setter(padding, float4);
  yw_control_getter_setter(value, T);
  auto range(this auto& Self, T Min, T Max) noexcept {
    return typename remove_cvref<decltype(Self)>::proxy(get_slot(&Self)).range(Min, Max);
  }
  yw_control_getter_setter(minimum, T);
  yw_control_getter_setter(maximum, T);
  yw_control_getter_setter(step, T);
  yw_control_getter_setter(button_width, float1);
  yw_control_getter_setter(arrow_color, color);
  auto arrow_color(this auto& Self, none None) noexcept {
    return typename remove_cvref<decltype(Self)>::proxy(get_slot(&Self)).arrow_color(None);
  }
  yw_control_getter_setter(button_color, color);
  auto button_color(this auto& Self, none None) noexcept {
    return typename remove_cvref<decltype(Self)>::proxy(get_slot(&Self)).button_color(None);
  }
  yw_control_getter_setter(change_event, function<bool, T>);

private:
  using control::padding;
};
} // namespace yw::ui
