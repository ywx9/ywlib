#pragma once
#include <optional>
#include <ywx/ui_edit.h>

namespace yw::ui {

template<typename T>
concept spinbox_value = arithmetic<T> && !is_bool<T> && !char_type<T>;

template<spinbox_value T> class spinbox : public edit {
public:
  struct slot : edit::slot {
    T value{};
    T minimum = std::numeric_limits<T>::lowest();
    T maximum = std::numeric_limits<T>::max();
    T step = T(1);
    float button_width = 18.0f;
    float button_gap = 2.0f;
    float4 user_padding = float4::fill(arbitrary_value);
    int pressed_button = 0;
    bool syncing_text = false;

    function<void, T> on_value_change{};

    static bool ascii_number_char(wchar_t c) noexcept {
      if (L'0' <= c && c <= L'9') return true;
      if constexpr (signed_integral<T> || floating<T>)
        if (c == L'+' || c == L'-') return true;
      if constexpr (floating<T>)
        if (c == L'.' || c == L'e' || c == L'E') return true;
      return false;
    }

    static yw::string<char> narrow_ascii_number(string_view<wchar_t> s) {
      yw::string<char> result;
      result.reserve(s.size());
      for (const auto c : s) {
        if (c < 0x20 || c >= 0x7f) break;
        result.push_back(static_cast<char>(c));
      }
      return result;
    }

    static std::optional<T> parse_value(string_view<wchar_t> s) {
      const auto narrow = narrow_ascii_number(s);
      if (narrow.empty()) return std::nullopt;
      T result{};
      const auto begin = narrow.data();
      const auto end = begin + narrow.size();
      const auto [ptr, ec] = std::from_chars(begin, end, result);
      if (ec != std::errc() || ptr == begin) return std::nullopt;
      return result;
    }

    T clamp_value(T v) const noexcept { return yw::clamp(v, minimum, maximum); }

    float reserved_button_width() const noexcept { return button_width + button_gap; }

    void apply_padding() noexcept {
      padding = user_padding;
      padding.z += reserved_button_width();
    }

    float4 button_rect(int Direction) const noexcept {
      const float left = pos.x + size.x - button_width;
      const float top = pos.y;
      const float h = size.y * 0.5f;
      if (Direction > 0) return {left, top, pos.x + size.x, top + h};
      return {left, top + h, pos.x + size.x, pos.y + size.y};
    }

    int button_at(float2 Pt) const noexcept {
      const float left = pos.x + size.x - button_width;
      if (Pt.x < left || Pt.x > pos.x + size.x || Pt.y < pos.y || Pt.y > pos.y + size.y) return 0;
      return Pt.y < pos.y + size.y * 0.5f ? +1 : -1;
    }

    void sync_text() {
      syncing_text = true;
      if (auto res = text.string(yw::format<wchar_t>(value)); !res) res.error().go_off();
      caret = yw::clamp(caret, 0, text.string().size());
      anchor = yw::clamp(anchor, 0, text.string().size());
      make_messy();
      syncing_text = false;
    }

    void set_value(T v, bool Notify = true) {
      v = clamp_value(v);
      if (value == v) {
        sync_text();
        return;
      }
      value = v;
      sync_text();
      if (Notify && on_value_change) on_value_change(value);
    }

    void commit_text() {
      const auto parsed = parse_value(text.string());
      if (!parsed) {
        sync_text();
        return;
      }
      set_value(*parsed);
    }

    T stepped_value(T Base, int Direction) const {
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
        return static_cast<T>(Base + step * Direction);
      }
    }

    void step_by(int Direction, int Multiplier = 1) {
      if (Direction == 0) return;
      T next = value;
      for (int i = 0; i < Multiplier; ++i) next = stepped_value(next, Direction);
      set_value(next);
    }

    std::expected<void, error> draw_button(int Direction) {
      const auto r = button_rect(Direction);
      const float2 p{r.x, r.y};
      const float2 s{r.z - r.x, r.w - r.y};
      if (pressed_button == Direction) {
        brush::color(color(0.0f, 0.0f, 0.0f, 0.15f));
        if (auto res = fill_rectangle(p, s); !res) return res.error().relay();
      }

      brush::color(colors.border);
      const float cx = (r.x + r.z) * 0.5f;
      const float cy = (r.y + r.w) * 0.5f;
      const float aw = yw::clamp(button_width * 0.22f, 3.0f, 5.0f);
      const float ah = yw::clamp(button_width * 0.16f, 2.0f, 4.0f);
      // const float aw = yw::max(3.0f, button_width * 0.22f);
      // const float ah = yw::max(2.0f, s.y * 0.18f);
      if (Direction > 0) {
        if (auto res = draw_line({cx - aw, cy + ah}, {cx, cy - ah}, 1.0f); !res) return res.error().relay();
        if (auto res = draw_line({cx, cy - ah}, {cx + aw, cy + ah}, 1.0f); !res) return res.error().relay();
      } else {
        if (auto res = draw_line({cx - aw, cy - ah}, {cx, cy + ah}, 1.0f); !res) return res.error().relay();
        if (auto res = draw_line({cx, cy + ah}, {cx + aw, cy - ah}, 1.0f); !res) return res.error().relay();
      }
      return {};
    }

    std::expected<void, error> draw_buttons() {
      if (button_width <= 0.0f) return {};
      brush::color(colors.border);
      const float left = pos.x + size.x - button_width;
      if (auto res = draw_line({left, pos.y}, {left, pos.y + size.y}, border_thickness); !res)
        return res.error().relay();
      if (auto res = draw_line({left, pos.y + size.y * 0.5f}, {pos.x + size.x, pos.y + size.y * 0.5f}, border_thickness); !res)
        return res.error().relay();
      if (auto res = draw_button(+1); !res) return res.error().relay();
      if (auto res = draw_button(-1); !res) return res.error().relay();
      return {};
    }

    virtual std::expected<void, error> redraw() override {
      if (geometry_dirty) {
        geometry_dirty = false;
        if (auto res = relocate(); !res) return res.error().relay();
      }
      if (!visible) return {};
      const bool focused = control::slot::focused();
      if (auto res = draw_frame_background(); !res) return res.error().relay();
      if (auto res = _update_scroll_offset(); !res) return res.error().relay();
      if (auto res = _draw_selection(); !res) return res.error().relay();
      if (!focused && text.string().empty()) {
        brush::color(placeholder_color);
        if (auto res = placeholder_string.draw(pos + _placeholder_offset()); !res) return res.error().relay();
      } else {
        brush::color(text_color);
        if (auto res = text.draw(pos + calc_text_offset() - scroll_offset); !res) return res.error().relay();
      }
      if (focused)
        if (auto res = _draw_caret(); !res) return res.error().relay();
      if (auto res = draw_buttons(); !res) return res.error().relay();
      if (auto res = draw_frame_foreground(); !res) return res.error().relay();
      return {};
    }

    virtual bool button_event(yw::button_event e) override {
      if (!enabled || !visible || e.key != keys::lbutton) return edit::slot::button_event(e);
      const auto pt = float2(float(e.pos.x), float(e.pos.y));
      const int hit = button_at(pt);
      if (hit == 0) {
        if (!e.down && pressed_button != 0) {
          pressed_button = 0;
          make_dirty();
          return true;
        }
        return edit::slot::button_event(e);
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

    virtual bool click_event(yw::button_event e) override {
      if (!enabled || !visible || e.down || e.key != keys::lbutton) return edit::slot::click_event(e);
      const auto pt = float2(float(e.pos.x), float(e.pos.y));
      const int hit = button_at(pt);
      if (hit == 0) return edit::slot::click_event(e);
      return true;
    }

    virtual bool double_click_event(yw::button_event e) override {
      const auto pt = float2(float(e.pos.x), float(e.pos.y));
      if (button_at(pt) != 0) return true;
      return edit::slot::double_click_event(e);
    }

    virtual bool drag_event(yw::drag_event e) override {
      if (pressed_button != 0) return true;
      return edit::slot::drag_event(e);
    }

    virtual std::expected<void, error> reset_state() override {
      if (pressed_button == 0) return edit::slot::reset_state();
      pressed_button = 0;
      make_dirty();
      return {};
    }

    virtual void focus_event(bool Focused) override {
      if (!Focused) {
        if (pressed_button != 0) {
          pressed_button = 0;
          make_dirty();
        }
        commit_text();
      }
      edit::slot::focus_event(Focused);
    }

    virtual bool key_event(yw::key_event e) override {
      if (!enabled || !visible) return false;
      if (!e.down) {
        if (e.key == keys::up || e.key == keys::down) return true;
        return edit::slot::key_event(e);
      }
      if (e.key == keys::up) {
        step_by(+1, e.mods.shift ? 10 : 1);
        return true;
      }
      if (e.key == keys::down) {
        step_by(-1, e.mods.shift ? 10 : 1);
        return true;
      }
      if (e.key == keys::page_up) {
        step_by(+1, 10);
        return true;
      }
      if (e.key == keys::page_down) {
        step_by(-1, 10);
        return true;
      }
      return edit::slot::key_event(e);
    }

    virtual bool wheel_event(yw::wheel_event e) override {
      if (!enabled || !visible || e.horizontal || e.delta == 0) return false;
      step_by(e.delta > 0 ? +1 : -1, e.mods.shift ? 10 : 1);
      return true;
    }
  };

  using edit::operator bool;
  spinbox() noexcept = default;

  spinbox(derived_from<interface> auto& Parent, strict<bool> AutoColor = true, const source_line& sl = here()) {
    if (auto res = create(Parent, AutoColor)) *this = std::move(*res);
    else res.error().add_footprint().go_off(sl);
  }

  static std::expected<spinbox, error> create(derived_from<interface> auto& Parent, strict<bool> AutoColor = true) {
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
    sp->policy = {ui::size_policy::fit, ui::size_policy::fit};
    sp->text_align = alignment::right;
    sp->filter = [](wchar_t c) { return slot::ascii_number_char(c); };
    if (AutoColor) {
      sp->colors = color_pair(none{});
      sp->text_color = sp->colors.border;
    }
    sp->apply_padding();

    const auto spinbox_id = temp_id;
    sp->on_change = [spinbox_id](string_view<wchar_t> s) {
      const auto sp = get_slot<spinbox>(spinbox_id);
      if (!sp || sp->syncing_text) return;
      const auto parsed = slot::parse_value(s);
      if (!parsed) return;
      const auto next = sp->clamp_value(*parsed);
      if (sp->value == next) return;
      sp->value = next;
      sp->make_dirty();
      if (sp->on_value_change) sp->on_value_change(sp->value);
    };
    sp->on_enter = [spinbox_id](yw::key_event) {
      if (const auto sp = get_slot<spinbox>(spinbox_id)) sp->commit_text();
    };
    sp->sync_text();
    return s;
  }

  //-- getter --//

  T value() const noexcept { ywlib_control_get(value); }
  T minimum() const noexcept { ywlib_control_get(minimum); }
  T maximum() const noexcept { ywlib_control_get(maximum); }
  T step() const noexcept { ywlib_control_get(step); }
  float button_width() const noexcept { ywlib_control_get(button_width); }
  float button_gap() const noexcept { ywlib_control_get(button_gap); }
  const auto& on_change() const noexcept { ywlib_control_get(on_value_change); }
  const auto& padding() const noexcept { ywlib_control_get(user_padding); }

  //-- setter --//

  auto& value(this auto& self, T v) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->set_value(v);
    return self;
  }

  auto& range(this auto& self, T Min, T Max) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    if (Max < Min) {
      error(errors::invalid_argument, "spinbox range maximum must be greater than or equal to minimum").go_off();
      return self;
    }
    sp->minimum = Min;
    sp->maximum = Max;
    sp->set_value(sp->value);
    return self;
  }

  auto& minimum(this auto& self, T v) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    if (sp->maximum < v) {
      error(errors::invalid_argument, "spinbox minimum must be less than or equal to maximum").go_off();
      return self;
    }
    sp->minimum = v;
    sp->set_value(sp->value);
    return self;
  }

  auto& maximum(this auto& self, T v) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    if (v < sp->minimum) {
      error(errors::invalid_argument, "spinbox maximum must be greater than or equal to minimum").go_off();
      return self;
    }
    sp->maximum = v;
    sp->set_value(sp->value);
    return self;
  }

  auto& step(this auto& self, T v) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    if (v <= T{}) {
      error(errors::invalid_argument, "spinbox step must be positive").go_off();
      return self;
    }
    sp->step = v;
    return self;
  }

  auto& button_width(this auto& self, float1 v) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    if (v.x <= 0.0f) {
      error(errors::invalid_argument, "spinbox button_width must be positive").go_off();
      return self;
    }
    sp->button_width = v.x;
    sp->apply_padding();
    sp->make_messy();
    return self;
  }

  auto& button_gap(this auto& self, float1 v) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    if (v.x < 0.0f) {
      error(errors::invalid_argument, "spinbox button_gap must be non-negative").go_off();
      return self;
    }
    sp->button_gap = v.x;
    sp->apply_padding();
    sp->make_messy();
    return self;
  }

  auto& padding(this auto& self, float4 f) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->user_padding = f;
    sp->apply_padding();
    sp->make_messy();
    return self;
  }

  auto& on_change(this auto& self, function<void, T> f) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->on_value_change = std::move(f);
    return self;
  }
};
} // namespace yw::ui
