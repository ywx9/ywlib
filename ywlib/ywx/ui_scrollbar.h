#pragma once
// #include <ywx/scrollbar.h>
#include <ywx/control.h>
#include <ywx/window.h>

namespace yw::ui {

template<orientation Orientation> class scrollbar : public control {
  static constexpr bool vertical = Orientation == orientation::vertical;

public:
  struct slot : control::slot {
    optional<color> arrow_color;
    optional<color> button_color;
    optional<color> thumb_color;
    optional<color> track_color;
    float bar_width = common_size_value;
    float line_step = common_size_value;
    float scroll_offset = 0.0f;
    part pressed_part = part::none;
    part hovered_part = part::none;

    //-- override functions --//

    virtual bool is_interactive() const override { return enabled && visible; }

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

    virtual color get_thumb_color(const interface::slot* Window) const noexcept {
      if (thumb_color) return *thumb_color;
      if (auto theme = get_color_theme(Window)) return (*theme)->part;
      return colors::transparent;
    }

    virtual color get_track_color(const interface::slot* Window) const noexcept {
      if (track_color) return *track_color;
      if (auto theme = get_color_theme(Window)) return (*theme)->surface_popup;
      return colors::transparent;
    }

    virtual color get_border_color(const interface::slot*) const noexcept override {
      return border_color ? *border_color : colors::transparent;
    }

    virtual std::expected<void, error> draw_backcontent(interface::slot* Window) override {
      brush::color(get_track_color(Window));
      if (auto res = draw_scrollbar_part(part::decrease_track); !res) return res.error().relay();
      if (auto res = draw_scrollbar_part(part::increase_track); !res) return res.error().relay();
      brush::color(get_button_color(Window));
      if (auto res = draw_scrollbar_part(part::decrease_button); !res) return res.error().relay();
      if (auto res = draw_scrollbar_part(part::increase_button); !res) return res.error().relay();
      brush::color(get_thumb_color(Window));
      if (auto res = draw_scrollbar_part(part::thumb); !res) return res.error().relay();
      return {};
    }

    virtual std::expected<void, error> draw_forecontent(interface::slot* Window) override {
      brush::color(get_arrow_color(Window));
      const auto arrow_size = float2(bar_width, bar_width);
      const auto origin2 = pos + size - arrow_size;
      if constexpr (vertical) {
        const auto origin = pos + float2(size.x - bar_width, 0.0f);
        if (auto res = draw_arrow<ui::top>(origin, arrow_size, 1.0f); !res) return res.error().relay();
        if (auto res = draw_arrow<ui::bottom>(origin2, arrow_size, 1.0f); !res) return res.error().relay();
      } else {
        const auto origin = pos + float2(0.0f, size.y - bar_width);
        if (auto res = draw_arrow<ui::right>(origin, arrow_size, 1.0f); !res) return res.error().relay();
        if (auto res = draw_arrow<ui::left>(origin2, arrow_size, 1.0f); !res) return res.error().relay();
      }
      return {};
    }

    virtual std::expected<void, error> draw_overlay(interface::slot* Window) override {
      const auto wsp = static_cast<window::slot*>(Window);
      if (!wsp) return {};
      if (pressed_part != part::none && wsp->press_overlay_color.a > 0.0f) {
        brush::color(wsp->press_overlay_color);
        if (auto res = draw_scrollbar_part(pressed_part); !res) return res.error().relay();
      }
      if (hovered_part != part::none && hovered_part != pressed_part && wsp->hover_overlay_color.a > 0.0f) {
        brush::color(wsp->hover_overlay_color);
        if (auto res = draw_scrollbar_part(hovered_part); !res) return res.error().relay();
      }
      return {};
    }

    virtual float2 get_minimum_size() const override {
      if constexpr (vertical) return float2(bar_width * 2.0f, bar_width * 3.0f);
      else return float2(bar_width * 3.0f, bar_width * 2.0f);
    }

    virtual std::expected<float2, error> get_necessary_size() const override {
      const auto inner = get_content_size() + padding.xy() + padding.zw();
      if constexpr (vertical) return float2(inner.x + bar_width, bar_width * 3.0f);
      else return float2(bar_width * 3.0f, inner.y + bar_width);
    }

    virtual void reset_state() override {
      if (pressed_part == part::none) return;
      pressed_part = part::none;
      make_dirty();
    }

    virtual bool handle_button_event(yw::button_event e) override {
      if (!enabled || !visible || e.key != keys::lbutton) return false;
      if (e.down) {
        if (const auto hit = hittest_bar(e.pos); hit != part::none) {
          pressed_part = hit;
          if (hit != part::thumb) scroll_part(hit);
          make_dirty();
          return true;
        } else return false;
      }
      if (pressed_part == part::none) return false;
      pressed_part = part::none;
      make_dirty();
      return true;
    }

    virtual bool handle_click_event(yw::button_event e) override {
      if (!enabled || !visible || e.down || e.key != keys::lbutton) return false;
      return hittest_bar(e.pos) != part::none;
    }

    virtual bool handle_drag_event(yw::drag_event e) override {
      if (!enabled || !visible || pressed_part != part::thumb) return false;
      if (const auto scroll = convert_pixel_to_scroll(e.delta[vertical]); scroll == 0.0f) return false;
      else scroll_by(scroll);
      return true;
    }

    virtual bool handle_focus_event(yw::focus_event e) override {
      if (!e.focused && pressed_part != part::none) {
        pressed_part = part::none;
        make_dirty();
      }
      return control::slot::handle_focus_event(e);
    }

    virtual bool handle_hover_event(yw::hover_event e) override {
      if (!enabled || !visible) return false;
      if (const auto next = e.leave() ? part::none : hittest_bar(e.pos); hovered_part != next) {
        hovered_part = next;
        make_dirty();
      }
      return true;
    }

    virtual bool handle_wheel_event(yw::wheel_event e) override {
      if (!enabled || !visible || e.delta == 0 || (!vertical != e.horizontal)) return false;
      const auto delta = line_step * (e.delta > 0 ? -1.0f : 1.0f) * (e.mods.shift ? 10.0f : 1.0f);
      return scroll_by(delta);
    }

    //-- virtual functions --//

    virtual float2 get_content_size() const { return {}; }

    //-- shared functions --//

    /// returns scroll amount when moving thumb by Pixels.
    float convert_pixel_to_scroll(float Pixel) const noexcept {
      return Pixel * get_maximum_scroll_offset() / get_movable_thumb_length();
    }

    std::expected<void, error> draw_scrollbar_part(part Part) const noexcept {
      const auto origin = pos.add<!vertical>(size[!vertical] - bar_width);
      auto begin = origin[vertical];
      auto length = 0.0f;
      if (Part == part::decrease_button) length = bar_width;
      else if (Part == part::decrease_track) begin += bar_width, length = get_decrease_track_length();
      else if (Part == part::thumb) begin += bar_width + get_decrease_track_length(), length = get_thumb_length();
      else if (Part == part::increase_track) {
        begin += bar_width + get_decrease_track_length() + get_thumb_length();
        length = get_increase_track_length();
      } else if (Part == part::increase_button) begin += size[vertical] - bar_width, length = bar_width;
      else return {};
      if (length <= 0.0f || bar_width <= 0.0f) return {};
      if constexpr (vertical) return fill_rectangle(float4(origin.x, begin, origin.x + bar_width, begin + length));
      else return fill_rectangle(float4(begin, origin.y, begin + length, origin.y + bar_width));
    }

    float get_decrease_track_length() const noexcept { return get_thumb_pos(); }

    float get_increase_track_length() const noexcept {
      return yw::max(0.0f, get_track_length() - get_decrease_track_length() - get_thumb_length());
    }

    float get_thumb_length() const noexcept {
      const auto track = get_track_length();
      if (track <= 0.0f) return 0.0f;
      const auto viewport = get_viewport_size()[vertical];
      const auto content = get_content_size()[vertical];
      if (content <= viewport) return track;
      return yw::clamp(viewport * track / content, yw::min(bar_width, track), track);
    }

    float get_thumb_pos() const noexcept {
      const auto movable = get_movable_thumb_length();
      if (movable <= 0.0f) return 0.0f;
      const auto maximum = get_maximum_scroll_offset();
      if (maximum <= 0.0f) return 0.0f;
      return movable * yw::clamp(scroll_offset / maximum, 0.0f, 1.0f);
    }

    float2 get_viewport_size() const noexcept {
      return vapply_r<float2>(yw::max, float2(), (size - padding.xy() - padding.zw()).add<!vertical>(-bar_width));
    }

    float get_track_length() const noexcept { return yw::max(0.0f, size[vertical] - bar_width * 2.0f); }

    float get_movable_thumb_length() const noexcept { return yw::max(0.0f, get_track_length() - get_thumb_length()); }

    float get_maximum_scroll_offset() const noexcept {
      return yw::max(0.0f, get_content_size()[vertical] - get_viewport_size()[vertical]);
    }

    /// \note `Pt` is assumed to be inside [`pos`, `pos + size`]
    part hittest_bar(float2 Pt) const noexcept {
      const auto origin = pos.add<!vertical>(size[!vertical] - bar_width);
      if (Pt[!vertical] < origin[!vertical] || Pt[!vertical] > origin[!vertical] + bar_width) return part::none;
      const auto p = Pt[vertical];
      auto o = origin[vertical] + bar_width;
      if (p <= o) return part::decrease_button;
      o += get_decrease_track_length();
      if (p < o) return part::decrease_track;
      o += get_thumb_length();
      if (p <= o) return part::thumb;
      o += get_increase_track_length();
      if (p < o) return part::increase_track;
      return part::increase_button;
    }

    bool scroll_by(float Delta) noexcept { return scroll_to(scroll_offset + Delta); }

    bool scroll_to(float Value) noexcept {
      const auto next = yw::clamp(Value, 0.0f, get_maximum_scroll_offset());
      if (scroll_offset == next) return false;
      scroll_offset = next;
      make_dirty();
      return true;
    }

    bool scroll_part(part Part) noexcept {
      if (Part == part::decrease_button) return scroll_by(-line_step);
      if (Part == part::increase_button) return scroll_by(line_step);
      if (Part == part::decrease_track) return scroll_by(-get_viewport_size()[vertical]);
      if (Part == part::increase_track) return scroll_by(get_viewport_size()[vertical]);
      return false;
    }
  };

  scrollbar() noexcept = default;
};
} // namespace yw::ui
