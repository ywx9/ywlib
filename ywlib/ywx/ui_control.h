#pragma once
#include "ywx/event.h"

namespace yw::errors {
define_error(ui_invalid_slotid);
}

namespace yw::ui {

class control : public unknown {
protected:
  template<derived_from<control> Ctrl, derived_from<unknown> Layout>
  static std::expected<slotid, error_trace> create_control(Layout& layout) {
    const auto lid = layout.id();
    const auto lsp = system::slot_address<unknown>(lid);
    if (!lsp) return unexpected_error(errors::operation_failed, "Failed to access layout slot");
    if (auto res = lsp->attachable(); !res) return unexpected_error(res.error());
    const auto cid = system::uis.add(std::make_unique<typename Ctrl::slot>());
    const auto csp = system::slot_address<Ctrl>(cid);
    if (!csp) return unexpected_error(errors::operation_failed, "Failed to create control slot");
    lsp->attach(cid);
    csp->id = cid;
    csp->layout_id = lid;
    csp->window_id = lsp->get_window_id();
    return cid;
  }

  /// returns default background and border colors.
  static tuple<color, color> get_auto_color() noexcept {
    constexpr float hues[] = {240.0f, 120.0f, 0.0f, 270.0f, 180.0f, 30.0f, 210.0f, 60.0f, 300.0f};
    static size_t color_index = 0;
    const float h = hues[color_index++ % arraysize(hues)] / 180.0f * yw::pi;
    return {hsl(h, 0.5f, 0.9f).to_srgb(), hsl(h, 0.5f, 0.2f).to_srgb()};
  }

  control() noexcept = default;

  template<typename Mp> const auto& unsafe_get(Mp mp) const {
    const auto csp = dynamic_cast<class_type<Mp>*>(system::uis.get(_id));
    if (!csp) fatal_error(errors::ui_invalid_slotid);
    return csp->*mp;
  }
  template<typename Mp, typename T> void unsafe_set(Mp mp, T&& value) {
    const auto csp = dynamic_cast<class_type<Mp>*>(system::uis.get(_id));
    if (!csp) fatal_error(errors::ui_invalid_slotid);
    csp->*mp = static_cast<T&&>(value);
  }

public:
  struct slot : public unknown::slot {
    slotid layout_id{};
    slotid window_id{};

    parts::core core{};

    bool visible = true;
    bool enabled = true;
    bool dying = false;

    std::wstring tooltip{};
    function<void, events::hover> on_hover;

    //-- overrides --//

    virtual slotid get_window_id() const override { return window_id; }

    virtual std::expected<void, error_trace> make_dirty() override {
      const auto wsp = system::slot_address<unknown>(window_id);
      if (!wsp) return unexpected_error(errors::ui_invalid_slotid);
      wsp->make_dirty();
      return {};
    }

    virtual std::expected<void, error_trace> make_moved() override {
      if (auto res = core.update_geometry(); !res) return unexpected_error(res.error());
      const auto wsp = system::slot_address<unknown>(window_id);
      if (!wsp) return unexpected_error(errors::ui_invalid_slotid);
      wsp->make_dirty();
      return {};
    }

    virtual std::expected<void, error_trace> make_messy() override {
      const auto wsp = system::slot_address<unknown>(window_id);
      if (!wsp) return unexpected_error(errors::ui_invalid_slotid);
      wsp->make_messy();
      return {};
    }

    //-- virtual functions --//

    virtual bool focusable() const { return false; }
    virtual slotid hittest(float2 Point) const { return core.hittest(Point) ? id : slotid{}; }
    virtual slotid next_tab_stop(slotid Focused, bool Forward, bool& Found) const { return {}; }

    virtual std::expected<void, error_trace> draw_focus_ring(const parts::focus_ring& fr) {
      const auto origin = core.pos - float2::fill(fr.offset);
      const auto size = core.size + float2::fill(fr.offset * 2.0f);
      const auto r = core.radius + float2::fill(fr.offset);
      brush.color(fr.color);
      if (auto res = draw_round_rectangle(origin, size, r, fr.width); !res) return unexpected_error(res.error());
      return {};
    }

    virtual void ensure_minimum_size() { core.size = core.minimum_size(); }
    virtual void update_layout(float2 Pos, float2 Size) { core.update_geometry(Pos, Size); }
    virtual std::expected<void, error_trace> draw() = 0;

    virtual float2 ime_position() const { return {}; };
    virtual void ime_insert_text(std::wstring_view) {}

    virtual void char_event(wchar_t c) {}
    virtual void click_event(events::button e) {}
    virtual void button_event(events::button e) {}
    virtual void drag_event(events::drag e) {}
    virtual void focus_event(bool) {}
    virtual void hover_event(events::hover Event);
    virtual bool key_event(events::key e) { return false; }
    virtual void move_event(events::move e) {}
    virtual void wheel_event(events::wheel e) {}
  };

  auto core() {
    const auto csp = system::slot_address<control>(_id);
    if (!csp) fatal_error(errors::ui_invalid_slotid);
    return csp->core.set();
  }

  const auto core() const {
    const auto csp = system::slot_address<control>(_id);
    if (!csp) fatal_error(errors::ui_invalid_slotid);
    return csp->core.get();
  }

  const auto& tooltip() const {

    const auto csp = system::slot_address<control>(_id);
    if (!csp) fatal_error(errors::ui_invalid_slotid);
    return csp->tooltip;
  }

  auto& tooltip(std::wstring Text) {
    const auto csp = system::slot_address<control>(_id);
    if (!csp) fatal_error(errors::ui_invalid_slotid);
    csp->tooltip = std::move(Text);
    return *this;
  }

  const auto& on_hover() const {
    const auto csp = system::slot_address<control>(_id);
    if (!csp) fatal_error(errors::ui_invalid_slotid);
    return csp->on_hover;
  }

  auto& on_hover(function<void, events::hover> Handler) {
    const auto csp = system::slot_address<control>(_id);
    if (!csp) fatal_error(errors::ui_invalid_slotid);
    csp->on_hover = std::move(Handler);
    return *this;
  }
};
} // namespace yw::ui
