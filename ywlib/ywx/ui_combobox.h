#pragma once
#include <ywx/ui_icon.h>
#include <ywx/ui_layout.h>
#include <ywx/ui_listbox.h>
#include <ywx/window.h>

namespace yw::ui {

class combobox : public control {
public:
  struct slot : control::slot {
    yw::text text = yw::text(L"");
    window dropdown_window{};
    vlayout dropdown_layout{};
    listbox dropdown_listbox{};
    ui::icon dropdown_icon{};
    float4 item_padding = float4::fill(arbitrary_value);
    float button_width = common_size_value;
    bool pressed = false;

    function<void, size_t> change_event{};

    //-- override functions --//

    virtual bool is_focusable() const override { return enabled && visible; }
    virtual bool is_interactive() const override { return true; }

    virtual std::expected<void, error> apply_color_theme(const color_theme& Theme, bool Recursive) override {
      background_color = Theme.surface;
      border_color = Theme.outline;
      text.color(Theme.text);
      if (auto res = apply_dropdown_color_theme(Theme); !res) return res.error().relay();
      make_dirty();
      return {};
    }

    virtual std::expected<void, error> draw_overlay() override {
      const auto wsp = get_slot<window>(window_id);
      if (!wsp) return std::unexpected(error(errors::invalid_slotid));
      if (pressed && wsp->press_overlay_color.a > 0.0f) {
        brush::color(wsp->press_overlay_color);
        if (auto res = fill_geometry(geometry.get()); !res) return res.error().relay();
      } else if (id == wsp->hovered_control_id && wsp->hover_overlay_color.a > 0.0f) {
        brush::color(wsp->hover_overlay_color);
        if (auto res = fill_geometry(geometry.get()); !res) return res.error().relay();
      }
      return {};
    }

    virtual std::expected<void, error> draw_forecontent() override {
      const auto text_origin = pos + padding.xy();
      const auto text_area = size - padding.xy() - padding.zw() - float2(button_width, 0.0f);
      const auto text_pos = align_position(text_origin, text_area, text.size(), left);
      if (auto res = draw_text(text_pos, text); !res) return res.error().relay();

      const auto button_origin = pos + float2(size.x - button_width, 0.0f);
      brush::color(border_color);
      if (auto res = stroke_line(button_origin, button_origin.add<1>(size.y), border_thickness); !res)
        return res.error().relay();
      const auto arrow_pos = button_origin.add<1>((size.y - button_width) * 0.5f);
      const auto arrow_size = float2::fill(button_width);
      if (auto res = draw_arrow<bottom>(arrow_pos, arrow_size, 1.0f); !res) return res.error().relay();
      return {};
    }

    virtual std::expected<float2, error> get_necessary_size() const override {
      float2 item_size = text.size();
      if (const auto lbsp = get_slot<listbox>(dropdown_listbox.id())) {
        for (const auto& item : lbsp->items) item_size = vapply_r<float2>(yw::max, item_size, item.size());
      }
      item_size.x += button_width;
      return calc_necessary_size_by_policy(item_size + padding.xy() + padding.zw());
    }

    virtual bool handle_button_event(yw::button_event e) override {
      if (!enabled || !visible || e.key != keys::lbutton) return false;
      const bool next_pressed = e.down;
      if (pressed != next_pressed) {
        pressed = next_pressed;
        make_dirty();
      }
      return true;
    }

    virtual bool handle_click_event(yw::button_event e) override {
      if (!enabled || !visible || e.down || e.key != keys::lbutton) return false;
      if (auto res = toggle_dropdown(); !res) res.error().go_off();
      return true;
    }

    virtual bool handle_focus_event(yw::focus_event e) override {
      if (!e.focused && pressed) {
        pressed = false;
        make_dirty();
      }
      return control::slot::handle_focus_event(e);
    }

    virtual bool handle_key_event(yw::key_event e) override {
      if (!enabled || !visible) return false;
      const bool open_key = e.key == keys::space || e.key == keys::enter || e.key == keys::down;
      if (!e.down) return open_key || e.key == keys::up;
      if (open_key) {
        if (auto res = open_dropdown(); !res) res.error().go_off();
        return true;
      }
      if (e.key == keys::up) {
        if (auto lbsp = get_slot<listbox>(dropdown_listbox.id())) {
          if (lbsp->selected_index != npos && lbsp->selected_index > 0) select(lbsp->selected_index - 1);
        }
        return true;
      }
      return control::slot::handle_key_event(e);
    }

    virtual void reset_state() override {
      if (!pressed) return;
      pressed = false;
      make_dirty();
    }

    //-- shared functions --//

    std::expected<void, error> apply_dropdown_color_theme(const color_theme& Theme) {
      if (const auto wsp = get_slot<window>(dropdown_window.id())) {
        if (auto res = wsp->apply_color_theme(Theme, false); !res) return res.error().relay();
      } else if (dropdown_window) return std::unexpected(error(errors::invalid_slotid));

      if (const auto lbsp = get_slot<listbox>(dropdown_listbox.id())) {
        if (auto res = lbsp->apply_color_theme(Theme, false); !res) return res.error().relay();
        lbsp->background_color = Theme.surface_popup;
        lbsp->item_padding = item_padding;
        lbsp->make_messy();
      } else if (dropdown_listbox) return std::unexpected(error(errors::invalid_slotid));

      if (const auto isp = get_slot<ui::icon>(dropdown_icon.id())) {
        if (auto res = isp->apply_color_theme(Theme, false); !res) return res.error().relay();
        isp->background_color = Theme.surface_popup;
        if (auto vector = isp->content.get_if_vector()) {
          vector->fill_color(Theme.text_muted);
          vector->stroke_color(Theme.text_muted);
        }
        isp->make_dirty();
      } else if (dropdown_icon) return std::unexpected(error(errors::invalid_slotid));
      return {};
    }

    void close_dropdown() {
      if (const auto wsp = get_slot<window>(dropdown_window.id()); wsp && wsp->visible)
        if (auto res = dropdown_window.hide(); !res) res.error().go_off();
    }

    std::expected<void, error> open_dropdown() {
      if (!dropdown_window || !dropdown_listbox) return std::unexpected(error(errors::invalid_slotid));
      if (item_count() == 0 || dropdown_window.visible()) return {};
      const auto parent_wsp = get_slot<window>(window_id);
      if (!parent_wsp) return std::unexpected(error(errors::invalid_slotid));
      if (auto res = apply_dropdown_color_theme(parent_wsp->color_theme); !res) return res.error().relay();
      const auto lbsp = get_slot<listbox>(dropdown_listbox.id());
      if (!lbsp) return std::unexpected(error(errors::invalid_slotid));

      const int2 dropdown_pos = parent_wsp->pos + parent_wsp->frame_thickness.xy() +
                                int2(int(std::round(pos.x)), int(std::round(pos.y + size.y)));
      const auto dropdown_wsp = get_slot<window>(dropdown_window.id());
      if (!dropdown_wsp) return std::unexpected(error(errors::invalid_slotid));
      if (auto res = dropdown_wsp->get_necessary_size(); !res) return res.error().relay();
      else {
        dropdown_wsp->fit_to_necessary_size = false;
        dropdown_wsp->pos = dropdown_pos;
        dropdown_wsp->size = uint2(yw::max(uint32_t(std::ceil(size.x)), res->x), res->y);
      }
      dropdown_window.sync_layout();
      lbsp->ensure_visible(lbsp->selected_index);
      dropdown_window.sync_redraw();
      dropdown_window.window_pos(dropdown_pos);
      if (auto res = dropdown_window.show(); !res) return res.error().relay();
      return {};
    }

    std::expected<void, error> resize_dropdown_by(float HeightDelta) {
      if (HeightDelta == 0.0f || !dropdown_window) return {};
      const auto wsp = get_slot<window>(dropdown_window.id());
      if (!wsp) return std::unexpected(error(errors::invalid_slotid));
      auto next = wsp->size;
      if (auto res = wsp->get_necessary_size(); !res) return res.error().relay();
      else next.y = uint32_t(yw::max(int(std::round(float(next.y) + HeightDelta)), int(res->y)));
      wsp->fit_to_necessary_size = false;
      wsp->size = next;
      const auto b = wsp->get_bounds();
      win32_bool_test(::SetWindowPos, wsp->hwnd, 0, wsp->pos.x, wsp->pos.y, b.x, b.y, SWP_NOZORDER | SWP_NOACTIVATE);
      wsp->messy = true;
      return {};
    }

    std::expected<void, error> select(size_t Index, bool Notify = true) {
      const auto lbsp = get_slot<listbox>(dropdown_listbox.id());
      if (!lbsp) return std::unexpected(error(errors::invalid_slotid));
      if (Index >= lbsp->items.size()) return {};
      const bool changed = lbsp->selected_index != Index;
      lbsp->selected_index = Index;
      if (dropdown_window.visible()) lbsp->ensure_visible(Index);
      text.string(lbsp->items[Index].string());
      make_messy();
      lbsp->make_dirty();
      if (changed && Notify && change_event) change_event(Index);
      return {};
    }

    std::expected<void, error> toggle_dropdown() {
      if (dropdown_window.visible()) close_dropdown();
      else if (auto res = open_dropdown(); !res) return res.error().relay();
      return {};
    }

    size_t item_count() const noexcept {
      if (const auto lbsp = get_slot<listbox>(dropdown_listbox.id())) return lbsp->items.size();
      return 0;
    }
  };

  using control::operator bool;
  combobox() noexcept = default;

  combobox(derived_from<interface> auto& Parent, const source_line& sl = here()) {
    if (auto res = create(Parent)) *this = std::move(*res);
    else res.error().add_footprint().go_off(sl);
  }

  static std::expected<combobox, error> create(derived_from<interface> auto& Parent) {
    combobox c;
    const auto temp_id = make_slot<combobox>();
    const auto sp = get_slot<combobox>(temp_id);
    if (!sp) return std::unexpected(error(errors::slot_creation_failed));
    const auto psp = get_slot<control>(Parent.id());
    if (!psp) return std::unexpected(error(errors::invalid_slotid));
    if (auto res = psp->attach(temp_id); !res) {
      slot::slots.erase(temp_id);
      return res.error().relay();
    }
    c._id = temp_id;
    sp->id = temp_id;
    sp->window_id = psp->get_window_id();
    sp->policy = {size_policy::fit, size_policy::fit};
    sp->padding = float4::fill(arbitrary_value);

    window::options op{.has_border = true, .has_caption = false, .resizable = false, .visible = false};
    if (auto res = window::create(std::move(op))) sp->dropdown_window = std::move(*res);
    else return res.error().relay();
    if (auto res = vlayout::create(sp->dropdown_window)) sp->dropdown_layout = std::move(*res);
    else return res.error().relay();
    if (auto res = listbox::create(sp->dropdown_layout)) sp->dropdown_listbox = std::move(*res);
    else return res.error().relay();
    if (auto res = ui::icon::create(sp->dropdown_layout)) sp->dropdown_icon = std::move(*res);
    else return res.error().relay();

    const auto layout_sp = get_slot<vlayout>(sp->dropdown_layout.id());
    const auto list_sp = get_slot<listbox>(sp->dropdown_listbox.id());
    const auto icon_sp = get_slot<ui::icon>(sp->dropdown_icon.id());
    const auto dropdown_wsp = get_slot<window>(sp->dropdown_window.id());
    if (!layout_sp || !list_sp || !icon_sp || !dropdown_wsp) return std::unexpected(error(errors::invalid_slotid));

    layout_sp->background_color = colors::transparent;
    list_sp->margin = {};
    list_sp->padding = {};
    list_sp->policy = {size_policy::fit, size_policy::free};

    icon_sp->margin = {};
    icon_sp->padding = {};
    icon_sp->radius = {};
    // icon_sp->required_size = {arbitrary_value, arbitrary_value};
    // icon_sp->policy = {size_policy::free, size_policy::fixed};
    icon_sp->content = yw::icon(yw::svgpath({32.0f, 16.0f}, "M 3 5 L 16 11 L 29 5 Z"));

    const auto combobox_id = temp_id;
    dropdown_wsp->focus_event = [combobox_id](yw::focus_event e) {
      if (e.focused) return false;
      if (const auto sp = get_slot<combobox>(combobox_id)) sp->close_dropdown();
      return false;
    };
    list_sp->change_event = [combobox_id](size_t Index) {
      if (const auto sp = get_slot<combobox>(combobox_id)) {
        if (auto res = sp->select(Index); !res) res.error().go_off();
        sp->close_dropdown();
      }
    };
    icon_sp->drag_event = [combobox_id](yw::drag_event e) {
      if (const auto sp = get_slot<combobox>(combobox_id))
        if (auto res = sp->resize_dropdown_by(float(e.delta.y)); !res) res.error().go_off();
      return true;
    };

    if (auto theme = sp->get_color_theme(); !theme) return theme.error().relay();
    else if (auto res = sp->apply_color_theme(*(*theme), false); !res) return res.error().relay();
    return c;
  }

  //-- getter --//

  size_t item_count() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return {};
    }
    return sp->item_count();
  }

  size_t selected_index() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return npos;
    }
    const auto lbsp = get_slot<listbox>(sp->dropdown_listbox.id());
    return lbsp ? lbsp->selected_index : npos;
  }

  bool dropdown_open() const noexcept {
    const auto sp = get_slot(this);
    return sp && sp->dropdown_window.visible();
  }

  float button_width() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return {};
    }
    return sp->button_width;
  }

  const auto& change_event() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->change_event;
  }

  const auto& item_padding() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->item_padding;
  }

  const auto& text(size_t Index) const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    const auto lbsp = get_slot<listbox>(sp->dropdown_listbox.id());
    if (!lbsp) error(errors::invalid_slotid).go_off();
    if (Index >= lbsp->items.size()) error(errors::invalid_argument, format("invalid item index: ", Index)).go_off();
    return lbsp->items[Index];
  }

  const auto& string(size_t Index) const noexcept { return text(Index).string(); }

  //-- setter --//

  auto& add(this auto& self, yw::string<wchar_t> String) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).fizzle_out();
    else if (const auto lbsp = get_slot<listbox>(sp->dropdown_listbox.id())) {
      lbsp->items.emplace_back(std::move(String));
      if (auto theme = sp->get_color_theme()) lbsp->items.back().color((*theme)->text);
      if (lbsp->selected_index == npos) {
        if (auto res = sp->select(0, false); !res) res.error().fizzle_out();
      }
      lbsp->update_content_size();
      sp->make_messy();
    } else error(errors::invalid_slotid).fizzle_out();
    return self;
  }

  auto& add(this auto& self, yw::text Text) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).fizzle_out();
    else if (const auto lbsp = get_slot<listbox>(sp->dropdown_listbox.id())) {
      lbsp->items.push_back(std::move(Text));
      if (auto theme = sp->get_color_theme()) lbsp->items.back().color((*theme)->text);
      if (lbsp->selected_index == npos) {
        if (auto res = sp->select(0, false); !res) res.error().fizzle_out();
      }
      lbsp->update_content_size();
      sp->make_messy();
    } else error(errors::invalid_slotid).fizzle_out();
    return self;
  }

  auto& clear(this auto& self) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).fizzle_out();
    else if (const auto lbsp = get_slot<listbox>(sp->dropdown_listbox.id())) {
      sp->close_dropdown();
      lbsp->items.clear();
      lbsp->selected_index = lbsp->pressed_index = lbsp->hovered_index = npos;
      lbsp->scroll_to(0.0f);
      sp->text.string(L"");
      sp->make_messy();
    } else error(errors::invalid_slotid).fizzle_out();
    return self;
  }

  auto& erase(this auto& self, size_t Index) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).fizzle_out();
    else if (const auto lbsp = get_slot<listbox>(sp->dropdown_listbox.id())) {
      if (Index >= lbsp->items.size()) error(errors::invalid_argument, format("invalid item index: ", Index)).fizzle_out();
      else {
        sp->close_dropdown();
        lbsp->items.erase(lbsp->items.begin() + Index);
        if (lbsp->items.empty()) {
          lbsp->selected_index = lbsp->pressed_index = lbsp->hovered_index = npos;
          sp->text.string(L"");
        } else {
          if (lbsp->selected_index == Index) lbsp->selected_index = yw::min(Index, lbsp->items.size() - 1);
          else if (Index < lbsp->selected_index) --lbsp->selected_index;
          if (auto res = sp->select(lbsp->selected_index, false); !res) res.error().fizzle_out();
        }
        lbsp->update_content_size();
        sp->make_messy();
      }
    } else error(errors::invalid_slotid).fizzle_out();
    return self;
  }

  auto& selected_index(this auto& self, size_t Index) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).fizzle_out();
    else if (auto res = sp->select(Index); !res) res.error().fizzle_out();
    return self;
  }

  auto& open(this auto& self) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).fizzle_out();
    else if (auto res = sp->open_dropdown(); !res) res.error().fizzle_out();
    return self;
  }

  auto& close(this auto& self) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).fizzle_out();
    else sp->close_dropdown();
    return self;
  }

  auto& button_width(this auto& self, float1 Width) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).fizzle_out();
    else if (Width.x <= 0.0f) error(errors::invalid_argument, "combobox button_width must be positive").fizzle_out();
    else sp->button_width = Width.x, sp->make_messy();
    return self;
  }

  auto& change_event(this auto& self, function<void, size_t> f) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).fizzle_out();
    else sp->change_event = std::move(f);
    return self;
  }

  auto& item_padding(this auto& self, float4 Padding) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).fizzle_out();
    else {
      sp->item_padding = Padding;
      if (const auto lbsp = get_slot<listbox>(sp->dropdown_listbox.id())) {
        lbsp->item_padding = Padding;
        lbsp->update_content_size();
      }
      sp->make_messy();
    }
    return self;
  }

  auto& text(this auto& self, size_t Index, yw::text Text) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).fizzle_out();
    else if (const auto lbsp = get_slot<listbox>(sp->dropdown_listbox.id())) {
      if (Index >= lbsp->items.size()) error(errors::invalid_argument, format("invalid item index: ", Index)).fizzle_out();
      else {
        lbsp->items[Index] = std::move(Text);
        if (Index == lbsp->selected_index)
          if (auto res = sp->select(Index, false); !res) res.error().fizzle_out();
        lbsp->update_content_size();
        sp->make_messy();
      }
    } else error(errors::invalid_slotid).fizzle_out();
    return self;
  }

  auto& string(this auto& self, size_t Index, yw::string<wchar_t> String) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).fizzle_out();
    else if (const auto lbsp = get_slot<listbox>(sp->dropdown_listbox.id())) {
      if (Index >= lbsp->items.size()) error(errors::invalid_argument, format("invalid item index: ", Index)).fizzle_out();
      else {
        if (auto res = lbsp->items[Index].string(std::move(String)); !res) res.error().fizzle_out();
        if (Index == lbsp->selected_index)
          if (auto res = sp->select(Index, false); !res) res.error().fizzle_out();
        lbsp->update_content_size();
        sp->make_messy();
      }
    } else error(errors::invalid_slotid).fizzle_out();
    return self;
  }

  auto& font(this auto& self, font_config Font) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).fizzle_out();
    else if (const auto lbsp = get_slot<listbox>(sp->dropdown_listbox.id())) {
      for (auto& item : lbsp->items)
        if (auto res = item.font(Font); !res) {
          res.error().fizzle_out();
          return self;
        }
      if (lbsp->selected_index < lbsp->items.size())
        if (auto res = sp->select(lbsp->selected_index, false); !res) res.error().fizzle_out();
      lbsp->update_content_size();
      sp->make_messy();
    } else error(errors::invalid_slotid).fizzle_out();
    return self;
  }
};
} // namespace yw::ui
