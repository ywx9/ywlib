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
    color arrow_color;
    color button_color;
    float arrow_thickness = 1.0f;
    float button_width = common_size_value;
    int pressed_button = 0;
    int hovered_button = 0;
    bool syncing_string = false;

    function<void, T> change_event{};

    //-- override functions --//

    virtual std::expected<void, error> apply_color_theme(const yw::ui::color_theme& Theme, bool Recursive) override {
      if (auto res = edit::slot::apply_color_theme(Theme, Recursive); !res) return res.error().relay();
      arrow_color = Theme.text;
      button_color = Theme.part;
      make_dirty();
      return {};
    }

    virtual std::expected<void, error> draw_backcontent() override {
      if (auto res = edit::slot::draw_backcontent(); !res) return res.error().relay();
      const auto origin = pos + float2(size.x - button_width, 0.0f);
      brush::color(button_color);
      if (auto res = fill_rectangle(origin, float2(button_width, size.y)); !res) return res.error().relay();
      return {};
    }

    virtual std::expected<void, error> draw_overlay() override {
      std::optional<int> pressed_button_;
      const auto wsp = get_slot<window>(window_id);
      if (!wsp) return std::unexpected(error(errors::invalid_slotid));
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

    virtual std::expected<void, error> draw_forecontent() override {
      const auto origin = pos + float2(size.x - button_width, 0.0f);
      const auto half_y = size.y * 0.5f;
      const auto arrow_size = float2(button_width, half_y);
      const auto origin2 = origin.add<1>(half_y);
      if (auto res = edit::slot::draw_forecontent(); !res) return res.error().relay();
      brush::color(border_color);
      if (auto res = stroke_line(origin, origin.add<1>(size.y), border_thickness); !res) return res.error().relay();
      if (auto res = stroke_line(origin2, origin2.add<0>(button_width), border_thickness); !res)
        return res.error().relay();
      brush::color(arrow_color);
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
        step_by(hit, e.mods.shift ? 10 : 1);
        make_dirty();
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
      if (e.key == keys::up) return step_by(+1, e.mods.shift ? 10 : 1), true;
      if (e.key == keys::down) return step_by(-1, e.mods.shift ? 10 : 1), true;
      if (e.key == keys::page_up) return step_by(+1, 10), true;
      if (e.key == keys::page_down) return step_by(-1, 10), true;
      if (e.key == keys::home) return set_value(minimum), true;
      if (e.key == keys::end) return set_value(maximum), true;
      return edit::slot::handle_key_event(e);
    }

    virtual bool handle_wheel_event(yw::wheel_event e) override {
      if (!enabled || !visible || e.horizontal || e.delta == 0) return false;
      step_by(e.delta > 0 ? +1 : -1, e.mods.shift ? 10 : 1);
      return true;
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

    void set_value(T v, bool Notify = true) {
      const auto next = normalize_value(v);
      if (value == next) {
        sync_string();
        return;
      }
      value = next;
      sync_string();
      if (Notify && change_event) change_event(value);
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

    void step_by(int Direction, int Multiplier = 1) {
      if (Direction == 0) return;
      commit_string();
      auto next = value;
      for (int i = 0; i < Multiplier; ++i) next = stepped_value(next, Direction);
      set_value(next);
    }
  };

  using edit::operator bool;

  spinbox() noexcept = default;

  spinbox(derived_from<interface> auto& Parent, const source_line& sl = here()) {
    if (auto res = create(Parent)) *this = std::move(*res);
    else res.error().add_footprint().go_off(sl);
  }

  static std::expected<spinbox, error> create(derived_from<interface> auto& Parent) {
    spinbox s;
    const auto temp_id = make_slot<spinbox>();
    const auto sp = get_slot<spinbox>(temp_id);
    if (!sp) return std::unexpected(error(errors::slot_creation_failed));
    const auto psp = get_slot<control>(Parent.id());
    if (!psp) return std::unexpected(error(errors::invalid_slotid));
    if (auto res = psp->attach(temp_id); !res) {
      slot::slots.erase(temp_id);
      return res.error().relay();
    }
    s._id = temp_id;
    sp->id = temp_id;
    sp->window_id = psp->get_window_id();
    sp->policy = {ui::free, ui::fit};
    sp->text_align = alignment::right;
    sp->filter = [](wchar_t c) { return slot::is_number_char(c); };
    if (auto theme = sp->get_color_theme(); !theme) return theme.error().relay();
    else if (auto res = sp->apply_color_theme(*(*theme), false); !res) return res.error().relay();
    sp->padding.z += sp->button_width;
    const auto spinbox_id = temp_id;
    sp->enter_event = [spinbox_id](yw::key_event) {
      if (const auto sp = get_slot<spinbox>(spinbox_id)) sp->commit_string();
    };
    sp->sync_string();
    return s;
  }

  //-- getter --//

  float4 padding() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return {};
    }
    return sp->padding.template add<2>(-sp->button_width);
  }

  T value() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return {};
    }
    return sp->value;
  }

  T minimum() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return {};
    }
    return sp->minimum;
  }

  T maximum() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return {};
    }
    return sp->maximum;
  }

  T step() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return {};
    }
    return sp->step;
  }

  float button_width() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return {};
    }
    return sp->button_width;
  }

  const auto& arrow_color() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->arrow_color;
  }

  const auto& button_color() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->button_color;
  }

  const auto& change_event() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->change_event;
  }

  //-- setter --//

  auto& padding(this auto& self, float4 v) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    sp->padding = v.add<2>(sp->button_width);
    sp->make_messy();
    return self;
  }

  auto& value(this auto& self, T v) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    sp->set_value(v);
    return self;
  }

  auto& range(this auto& self, T Min, T Max) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    if (Max < Min) {
      error(errors::invalid_argument, "spinbox range maximum must be greater than or equal to minimum").fizzle_out();
      return self;
    }
    sp->minimum = Min;
    sp->maximum = Max;
    sp->set_value(sp->value);
    return self;
  }

  auto& minimum(this auto& self, T v) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    if (sp->maximum < v) {
      error(errors::invalid_argument, "spinbox minimum must be less than or equal to maximum").fizzle_out();
      return self;
    }
    sp->minimum = v;
    sp->set_value(sp->value);
    return self;
  }

  auto& maximum(this auto& self, T v) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    if (v < sp->minimum) {
      error(errors::invalid_argument, "spinbox maximum must be greater than or equal to minimum").fizzle_out();
      return self;
    }
    sp->maximum = v;
    sp->set_value(sp->value);
    return self;
  }

  auto& step(this auto& self, T v) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).fizzle_out();
    else if (v <= T{}) error(errors::invalid_argument, "spinbox step must be positive").fizzle_out();
    else sp->step = v, sp->set_value(sp->value, false);
    return self;
  }

  auto& button_width(this auto& self, float1 v) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    if (v.x <= 0.0f) {
      error(errors::invalid_argument, "spinbox button_width must be positive").fizzle_out();
      return self;
    }
    sp->padding.z -= sp->button_width - v.x;
    sp->button_width = v.x;
    sp->make_messy();
    return self;
  }

  auto& arrow_color(this auto& self, const color& c) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    sp->arrow_color = c;
    sp->make_dirty();
    return self;
  }

  auto& button_color(this auto& self, const color& c) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    sp->button_color = c;
    sp->make_dirty();
    return self;
  }

  auto& change_event(this auto& self, function<void, T> f) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return self;
    }
    sp->change_event = std::move(f);
    return self;
  }

private:
  using control::padding;
};
} // namespace yw::ui
