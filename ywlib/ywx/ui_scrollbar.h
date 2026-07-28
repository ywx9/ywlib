#pragma once
// #include <ywx/scrollbar.h>
#include <ywx/control.h>
#include <ywx/window.h>

namespace yw::ui {

template<orientation Orientation> class scrollbar : public control {
  static constexpr bool vertical = Orientation == orientation::vertical;

public:
  struct slot : control::slot {
    color arrow_color;
    color button_color;
    color thumb_color;
    color track_color;
    float bar_width = common_size_value;
    float line_step = common_size_value;
    float scroll_offset = 0.0f;
    part pressed_part = part::none;
    part hovered_part = part::none;

    //-- override functions --//

    virtual bool is_interactive() const override { return enabled && visible; }

    virtual std::expected<void, error> apply_color_theme(const color_theme& Theme, bool) override {
      background_color = Theme.surface;
      border_color = colors::transparent;
      arrow_color = Theme.text;
      button_color = Theme.part;
      thumb_color = Theme.part;
      track_color = Theme.surface_popup;
      make_dirty();
      return {};
    }

    virtual std::expected<void, error> draw_backcontent() override {
      brush::color(track_color);
      if (auto res = draw_scrollbar_part(part::decrease_track); !res) return res.error().relay();
      if (auto res = draw_scrollbar_part(part::increase_track); !res) return res.error().relay();
      brush::color(button_color);
      if (auto res = draw_scrollbar_part(part::decrease_button); !res) return res.error().relay();
      if (auto res = draw_scrollbar_part(part::increase_button); !res) return res.error().relay();
      brush::color(thumb_color);
      if (auto res = draw_scrollbar_part(part::thumb); !res) return res.error().relay();
      return {};
    }

    virtual std::expected<void, error> draw_forecontent() override {
      brush::color(arrow_color);
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

    virtual std::expected<void, error> draw_overlay() override {
      const auto wsp = get_slot<window>(window_id);
      if (!wsp) return std::unexpected(error(errors::invalid_slotid));
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

// class scrollbars : public control {
// public:
//   struct slot : control::slot {
//     enum class part : unsigned char {
//       none,
//       horizontal_decrease_button,
//       horizontal_decrease_track,
//       horizontal_thumb,
//       horizontal_increase_track,
//       horizontal_increase_button,
//       vertical_decrease_button,
//       vertical_decrease_track,
//       vertical_thumb,
//       vertical_increase_track,
//       vertical_increase_button,
//     };

//     ::yw::scrollbar<horizontal> horizontal_bar;
//     ::yw::scrollbar<vertical> vertical_bar;
//     float2 content_size{};
//     float bar_width = common_size_value;
//     part pressed_part = part::none;
//     part hovered_part = part::none;

//     function<void, float2> change_event{};

//     virtual bool is_interactive() const override { return enabled && visible; }

//     virtual std::expected<void, error> apply_color_theme(const color_theme& Theme, bool) override {
//       background_color = Theme.surface;
//       border_color = colors::transparent;
//       horizontal_bar.track_color = vertical_bar.track_color = Theme.surface;
//       horizontal_bar.thumb_color = vertical_bar.thumb_color = Theme.part;
//       horizontal_bar.button_color = vertical_bar.button_color = Theme.part;
//       horizontal_bar.arrow_color = vertical_bar.arrow_color = Theme.text;
//       make_dirty();
//       return {};
//     }

//     virtual void reset_state() override {
//       if (pressed_part == part::none) return;
//       pressed_part = part::none;
//       make_dirty();
//     }

//     virtual bool handle_button_event(yw::button_event e) override {
//       if (!enabled || !visible || e.key != keys::lbutton) return false;
//       update_scrollbars();
//       const auto hit = hittest_scrollbar(float2(float(e.pos.x), float(e.pos.y)));
//       if (e.down) {
//         if (hit == part::none) return false;
//         pressed_part = hit;
//         if (hit != part::horizontal_thumb && hit != part::vertical_thumb) scroll_part(hit);
//         make_dirty();
//         return true;
//       }
//       if (pressed_part == part::none) return false;
//       pressed_part = part::none;
//       make_dirty();
//       return true;
//     }

//     virtual bool handle_click_event(yw::button_event e) override {
//       if (!enabled || !visible || e.down || e.key != keys::lbutton) return false;
//       update_scrollbars();
//       return hittest_scrollbar(float2(float(e.pos.x), float(e.pos.y))) != part::none;
//     }

//     virtual bool handle_drag_event(yw::drag_event e) override {
//       if (!enabled || !visible) return false;
//       update_scrollbars();
//       if (pressed_part == part::horizontal_thumb) {
//         const auto movable = horizontal_bar.movable_thumb_length();
//         if (movable > 0.0f)
//           scroll_to(float2(horizontal_bar.value + e.delta.x * horizontal_bar.maximum() / movable,
//           vertical_bar.value));
//         return true;
//       }
//       if (pressed_part == part::vertical_thumb) {
//         const auto movable = vertical_bar.movable_thumb_length();
//         if (movable > 0.0f)
//           scroll_to(float2(horizontal_bar.value, vertical_bar.value + e.delta.y * vertical_bar.maximum() / movable));
//         return true;
//       }
//       return false;
//     }

//     virtual bool handle_focus_event(yw::focus_event e) override {
//       if (!e.focused && pressed_part != part::none) {
//         pressed_part = part::none;
//         make_dirty();
//       }
//       return control::slot::handle_focus_event(e);
//     }

//     virtual bool handle_hover_event(yw::hover_event e) override {
//       if (!enabled || !visible) return false;
//       update_scrollbars();
//       const auto next = e.leave() ? part::none : hittest_scrollbar(float2(float(e.pos.x), float(e.pos.y)));
//       if (hovered_part != next) {
//         hovered_part = next;
//         make_dirty();
//       }
//       return true;
//     }

//     virtual bool handle_wheel_event(yw::wheel_event e) override {
//       if (!enabled || !visible || e.delta == 0) return false;
//       update_scrollbars();
//       const auto amount = (e.delta > 0 ? -1.0f : 1.0f) * (e.mods.shift ? 10.0f : 1.0f);
//       if (e.horizontal)
//         return scroll_by(float2(horizontal_bar.wheel_step * amount, 0.0f)) || horizontal_bar.maximum() > 0.0f;
//       return scroll_by(float2(0.0f, vertical_bar.wheel_step * amount)) || vertical_bar.maximum() > 0.0f;
//     }

//     std::expected<void, error> draw_scrollbars() {
//       update_scrollbars();
//       if (auto res = draw_bar(horizontal_bar); !res) return res.error().relay();
//       if (auto res = draw_bar(vertical_bar); !res) return res.error().relay();
//       brush::color(background_color);
//       if (auto res = fill_rectangle(corner_rect()); !res) return res.error().relay();
//       if (auto res = draw_part_overlay(hovered_part, false); !res) return res.error().relay();
//       if (auto res = draw_part_overlay(pressed_part, true); !res) return res.error().relay();
//       return {};
//     }

//     float2 content_area() const noexcept {
//       return vapply_r<float2>(yw::max, size - padding.xy() - padding.zw() - float2::fill(bar_width), float2{});
//     }

//     float2 scroll_offset() const noexcept { return float2(horizontal_bar.value, vertical_bar.value); }

//     void update_scrollbars() noexcept {
//       const auto area = content_area();
//       horizontal_bar.pos = pos + padding.xy() + float2(0.0f, area.y);
//       horizontal_bar.size = float2(area.x, bar_width);
//       horizontal_bar.content_length = content_size.x;
//       horizontal_bar.clamp_value();
//       vertical_bar.pos = pos + padding.xy() + float2(area.x, 0.0f);
//       vertical_bar.size = float2(bar_width, area.y);
//       vertical_bar.content_length = content_size.y;
//       vertical_bar.clamp_value();
//     }

//     bool scroll_by(float2 Delta) noexcept { return scroll_to(scroll_offset() + Delta); }

//     bool scroll_part(part Part) noexcept {
//       if (Part == part::horizontal_decrease_button) return scroll_by(float2(-horizontal_bar.line_step, 0.0f));
//       if (Part == part::horizontal_decrease_track) return scroll_by(float2(-horizontal_bar.page_size(), 0.0f));
//       if (Part == part::horizontal_increase_track) return scroll_by(float2(horizontal_bar.page_size(), 0.0f));
//       if (Part == part::horizontal_increase_button) return scroll_by(float2(horizontal_bar.line_step, 0.0f));
//       if (Part == part::vertical_decrease_button) return scroll_by(float2(0.0f, -vertical_bar.line_step));
//       if (Part == part::vertical_decrease_track) return scroll_by(float2(0.0f, -vertical_bar.page_size()));
//       if (Part == part::vertical_increase_track) return scroll_by(float2(0.0f, vertical_bar.page_size()));
//       if (Part == part::vertical_increase_button) return scroll_by(float2(0.0f, vertical_bar.line_step));
//       return false;
//     }

//     bool scroll_to(float2 Offset) noexcept {
//       const auto old = scroll_offset();
//       horizontal_bar.scroll_to(Offset.x);
//       vertical_bar.scroll_to(Offset.y);
//       if (old == scroll_offset()) return false;
//       make_dirty();
//       if (change_event) change_event(scroll_offset());
//       return true;
//     }

//     auto& track_color(const color& c) noexcept {
//       return horizontal_bar.track_color = vertical_bar.track_color = c, *this;
//     }
//     auto& thumb_color(const color& c) noexcept {
//       return horizontal_bar.thumb_color = vertical_bar.thumb_color = c, *this;
//     }
//     auto& button_color(const color& c) noexcept {
//       return horizontal_bar.button_color = vertical_bar.button_color = c, *this;
//     }
//     auto& arrow_color(const color& c) noexcept {
//       return horizontal_bar.arrow_color = vertical_bar.arrow_color = c, *this;
//     }

//   private:
//     template<orientation O> static std::expected<void, error> draw_bar(const ::yw::scrollbar<O>& Bar) {
//       if (auto res = Bar.draw_decrease_button(); !res) return res.error().relay();
//       if (auto res = Bar.draw_decrease_track(); !res) return res.error().relay();
//       if (auto res = Bar.draw_increase_track(); !res) return res.error().relay();
//       if (auto res = Bar.draw_thumb(); !res) return res.error().relay();
//       if (auto res = Bar.draw_increase_button(); !res) return res.error().relay();
//       return {};
//     }

//     float4 corner_rect() const noexcept {
//       return float4(
//         vertical_bar.pos.x, horizontal_bar.pos.y, vertical_bar.pos.x + vertical_bar.size.x,
//         horizontal_bar.pos.y + horizontal_bar.size.y);
//     }

//     std::expected<void, error> draw_part_overlay(part Part, bool Pressed) const {
//       if (Part == part::none) return {};
//       const auto wsp = get_slot<window>(window_id);
//       if (!wsp) return std::unexpected(error(errors::invalid_slotid));
//       const auto& overlay = Pressed ? wsp->press_overlay_color : wsp->hover_overlay_color;
//       if (overlay.a <= 0.0f) return {};
//       switch (Part) {
//       case part::horizontal_decrease_button: return horizontal_bar.draw_decrease_button(&overlay);
//       case part::horizontal_decrease_track: return horizontal_bar.draw_decrease_track(&overlay);
//       case part::horizontal_thumb: return horizontal_bar.draw_thumb(&overlay);
//       case part::horizontal_increase_track: return horizontal_bar.draw_increase_track(&overlay);
//       case part::horizontal_increase_button: return horizontal_bar.draw_increase_button(&overlay);
//       case part::vertical_decrease_button: return vertical_bar.draw_decrease_button(&overlay);
//       case part::vertical_decrease_track: return vertical_bar.draw_decrease_track(&overlay);
//       case part::vertical_thumb: return vertical_bar.draw_thumb(&overlay);
//       case part::vertical_increase_track: return vertical_bar.draw_increase_track(&overlay);
//       case part::vertical_increase_button: return vertical_bar.draw_increase_button(&overlay);
//       default: return {};
//       }
//     }

//     part hittest_scrollbar(float2 Pt) const noexcept {
//       const auto h = horizontal_bar.hittest(Pt);
//       if (h == decltype(horizontal_bar)::part::decrease_button) return part::horizontal_decrease_button;
//       if (h == decltype(horizontal_bar)::part::decrease_track) return part::horizontal_decrease_track;
//       if (h == decltype(horizontal_bar)::part::thumb) return part::horizontal_thumb;
//       if (h == decltype(horizontal_bar)::part::increase_track) return part::horizontal_increase_track;
//       if (h == decltype(horizontal_bar)::part::increase_button) return part::horizontal_increase_button;
//       const auto v = vertical_bar.hittest(Pt);
//       if (v == decltype(vertical_bar)::part::decrease_button) return part::vertical_decrease_button;
//       if (v == decltype(vertical_bar)::part::decrease_track) return part::vertical_decrease_track;
//       if (v == decltype(vertical_bar)::part::thumb) return part::vertical_thumb;
//       if (v == decltype(vertical_bar)::part::increase_track) return part::vertical_increase_track;
//       if (v == decltype(vertical_bar)::part::increase_button) return part::vertical_increase_button;
//       return part::none;
//     }
//   };

//   scrollbars() noexcept = default;
// };
} // namespace yw::ui
