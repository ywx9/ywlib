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
    optional<color> text_color;
    window dropdown_window{};
    vlayout dropdown_layout{};
    listbox dropdown_listbox{};
    ui::icon dropdown_icon{};
    float4 item_padding = float4::fill(arbitrary_value);
    float button_width = common_size_value;
    bool pressed = false;

    function<bool, size_t> change_event{};

    //-- override functions --//

    virtual bool is_focusable() const override { return enabled && visible; }
    virtual bool is_interactive() const override { return true; }

    virtual color get_text_color(const interface::slot* Window) const noexcept {
      if (text_color) return *text_color;
      if (auto theme = get_color_theme(Window)) return (*theme)->text;
      return colors::transparent;
    }

    virtual std::expected<void, error> draw_overlay(interface::slot* Window) override {
      const auto wsp = static_cast<window::slot*>(Window);
      if (!wsp) return {};
      if (pressed && wsp->press_overlay_color.a > 0.0f) {
        brush::color(wsp->press_overlay_color);
        if (auto res = fill_geometry(geometry.get()); !res) return res.error().relay();
      } else if (id == wsp->hovered_control_id && wsp->hover_overlay_color.a > 0.0f) {
        brush::color(wsp->hover_overlay_color);
        if (auto res = fill_geometry(geometry.get()); !res) return res.error().relay();
      }
      return {};
    }

    virtual std::expected<void, error> draw_forecontent(interface::slot* Window) override {
      const auto text_origin = pos + padding.xy();
      const auto text_area = size - padding.xy() - padding.zw() - float2(button_width, 0.0f);
      const auto text_pos = align_position(text_origin, text_area, text.size(), left);
      if (auto res = draw_text(text_pos, text, get_text_color(Window)); !res) return res.error().relay();

      const auto button_origin = pos + float2(size.x - button_width, 0.0f);
      brush::color(get_border_color(Window));
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
        if (auto res = wsp->set_color_theme(Theme, false); !res) return res.error().relay();
      } else if (dropdown_window) return std::unexpected(error(errors::invalid_slotid));

      if (const auto lbsp = get_slot<listbox>(dropdown_listbox.id())) {
        lbsp->background_color = Theme.surface_popup;
        lbsp->item_padding = item_padding;
        lbsp->make_messy();
      } else if (dropdown_listbox) return std::unexpected(error(errors::invalid_slotid));

      if (const auto isp = get_slot<ui::icon>(dropdown_icon.id())) {
        isp->background_color = Theme.surface_popup;
        isp->fill_color = Theme.text_muted;
        isp->stroke_color = Theme.text_muted;
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
      // dropdown_window.window_pos(dropdown_pos);
      ::SetWindowPos(
        dropdown_wsp->hwnd, HWND_TOPMOST, dropdown_pos.x, dropdown_pos.y, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
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
  class proxy : public control::proxy {
    friend class combobox;
    using control::proxy::proxy;
    combobox::slot* _get_slot() const noexcept { return static_cast<combobox::slot*>(_slot); }

  public:
    //-- getter --//

    size_t item_count() const&& noexcept { return _get_slot()->item_count(); }
    size_t selected_index() const&& noexcept {
      const auto lbsp = get_slot<listbox>(_get_slot()->dropdown_listbox.id());
      return lbsp ? lbsp->selected_index : npos;
    }
    bool dropdown_open() const&& noexcept { return _get_slot()->dropdown_window.visible(); }
    float button_width() const&& noexcept { return _get_slot()->button_width; }
    const auto& change_event() const&& noexcept { return _get_slot()->change_event; }
    const auto& item_padding() const&& noexcept { return _get_slot()->item_padding; }
    color text_color() const&& noexcept {
      return _get_slot()->get_text_color(interface::slot::slots.get(_get_slot()->window_id));
    }
    const auto& text(size_t Index) const&& noexcept {
      const auto lbsp = get_slot<listbox>(_get_slot()->dropdown_listbox.id());
      if (!lbsp) error(errors::invalid_slotid).go_off();
      if (Index >= lbsp->items.size()) error(errors::invalid_argument, format("invalid item index: ", Index)).go_off();
      return lbsp->items[Index];
    }
    const auto& selected_text() const&& noexcept {
      const auto lbsp = get_slot<listbox>(_get_slot()->dropdown_listbox.id());
      if (!lbsp) error(errors::invalid_slotid).go_off();
      if (lbsp->selected_index == npos) error(errors::invalid_operation, "no item selected").go_off();
      return lbsp->items[lbsp->selected_index];
    }
    const auto& string(size_t Index) const&& noexcept {
      const auto lbsp = get_slot<listbox>(_get_slot()->dropdown_listbox.id());
      if (!lbsp) error(errors::invalid_slotid).go_off();
      if (Index >= lbsp->items.size()) error(errors::invalid_argument, format("invalid item index: ", Index)).go_off();
      return lbsp->items[Index].string();
    }
    const auto& selected_string() const&& noexcept {
      const auto lbsp = get_slot<listbox>(_get_slot()->dropdown_listbox.id());
      if (!lbsp) error(errors::invalid_slotid).go_off();
      if (lbsp->selected_index == npos) error(errors::invalid_operation, "no item selected").go_off();
      return lbsp->items[lbsp->selected_index].string();
    }

    //-- setter --//

    auto selected_index(this auto&& Self, size_t Index) noexcept {
      if (auto res = Self._get_slot()->select(Index); !res) res.error().fizzle_out();
      return std::move(Self);
    }

    auto button_width(this auto&& Self, float1 Width) noexcept {
      if (Width.x <= 0.0f) error(errors::invalid_argument, "combobox button_width must be positive").fizzle_out();
      else Self._get_slot()->button_width = Width.x, Self._messy = true;
      return std::move(Self);
    }

    auto change_event(this auto&& Self, function<bool, size_t> Event) noexcept {
      Self._get_slot()->change_event = std::move(Event);
      return std::move(Self);
    }

    auto item_padding(this auto&& Self, float4 Padding) noexcept {
      Self._get_slot()->item_padding = Padding;
      if (const auto lbsp = get_slot<listbox>(Self._get_slot()->dropdown_listbox.id())) {
        lbsp->item_padding = Padding;
        lbsp->update_content_size();
      }
      Self._messy = true;
      return std::move(Self);
    }

    auto text_color(this auto&& Self, const color& Color) noexcept {
      Self._get_slot()->text_color = Color;
      if (const auto lbsp = get_slot<listbox>(Self._get_slot()->dropdown_listbox.id())) lbsp->text_color = Color;
      Self._dirty = true;
      return std::move(Self);
    }

    auto text_color(this auto&& Self, none) noexcept {
      Self._get_slot()->text_color = none();
      if (const auto lbsp = get_slot<listbox>(Self._get_slot()->dropdown_listbox.id())) lbsp->text_color = none();
      Self._dirty = true;
      return std::move(Self);
    }

    auto text(this auto&& Self, size_t Index, yw::text Text) noexcept {
      if (const auto lbsp = get_slot<listbox>(Self._get_slot()->dropdown_listbox.id())) {
        if (Index >= lbsp->items.size()) error(errors::invalid_argument, format("invalid item index: ", Index)).fizzle_out();
        else {
          lbsp->items[Index] = std::move(Text);
          if (Index == lbsp->selected_index)
            if (auto res = Self._get_slot()->select(Index, false); !res) res.error().fizzle_out();
          lbsp->update_content_size();
          Self._messy = true;
        }
      } else error(errors::invalid_slotid).fizzle_out();
      return std::move(Self);
    }

    auto string(this auto&& Self, size_t Index, yw::string<wchar_t> String) noexcept {
      if (const auto lbsp = get_slot<listbox>(Self._get_slot()->dropdown_listbox.id())) {
        if (Index >= lbsp->items.size()) error(errors::invalid_argument, format("invalid item index: ", Index)).fizzle_out();
        else {
          if (auto res = lbsp->items[Index].string(std::move(String)); !res) res.error().fizzle_out();
          if (Index == lbsp->selected_index)
            if (auto res = Self._get_slot()->select(Index, false); !res) res.error().fizzle_out();
          lbsp->update_content_size();
          Self._messy = true;
        }
      } else error(errors::invalid_slotid).fizzle_out();
      return std::move(Self);
    }

    auto font(this auto&& Self, font_config Font) noexcept {
      if (const auto lbsp = get_slot<listbox>(Self._get_slot()->dropdown_listbox.id())) {
        for (auto& item : lbsp->items) item.font(Font);
        if (lbsp->selected_index < lbsp->items.size())
          if (auto res = Self._get_slot()->select(lbsp->selected_index, false); !res) res.error().fizzle_out();
        lbsp->update_content_size();
        Self._messy = true;
      } else error(errors::invalid_slotid).fizzle_out();
      return std::move(Self);
    }
  };

  combobox() noexcept = default;

  combobox(derived_from<interface> auto& Parent, const source_line& sl = here()) {
    if (auto res = create(Parent)) *this = std::move(*res);
    else res.error().add_footprint().go_off(sl);
  }

  static std::expected<combobox, error> create(derived_from<interface> auto& Parent) {
    combobox c;
    combobox::slot* sp;
    if (auto res = create_control<combobox>(Parent)) sp = *res;
    else return res.error().relay();
    c._id = sp->id;
    sp->policy = {size_policy::fit, size_policy::fit};
    sp->padding = float4::fill(arbitrary_value);

    if (auto res = window::create(
          {.has_border = true, .has_caption = false, .resizable = false, .visible = false, .topmost = true}))
      sp->dropdown_window = std::move(*res);
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
    icon_sp->content = yw::icon(yw::svgpath({32.0f, 16.0f}, "M 3 5 L 16 11 L 29 5 Z"));

    const auto combobox_id = sp->id;
    dropdown_wsp->focus_event = [combobox_id](yw::focus_event e) {
      if (e.focused) return false;
      if (const auto sp = get_slot<combobox>(combobox_id)) sp->close_dropdown();
      return false;
    };
    list_sp->change_event = [combobox_id](size_t Index) {
      if (const auto sp = get_slot<combobox>(combobox_id)) {
        if (Index < sp->item_count()) {
          const auto lbsp = get_slot<listbox>(sp->dropdown_listbox.id());
          if (!lbsp) error(errors::invalid_slotid).go_off();
          sp->text.string(lbsp->items[Index].string());
          sp->make_messy();
          if (sp->change_event) sp->change_event(Index);
        }
        sp->close_dropdown();
      }
      return true;
    };
    icon_sp->drag_event = [combobox_id](yw::drag_event e) {
      if (const auto sp = get_slot<combobox>(combobox_id))
        if (auto res = sp->resize_dropdown_by(float(e.delta.y)); !res) res.error().go_off();
      return true;
    };

    return c;
  }

  yw_control_getter(item_count);
  yw_control_getter_setter(selected_index, size_t);
  yw_control_getter(dropdown_open);

  decltype(auto) dropdown(this auto& self) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).go_off();
    return static_cast<copy_cvref<decltype(self), listbox>>(sp->dropdown_listbox);
  }

  yw_control_getter_setter(button_width, float1);
  yw_control_getter_setter(change_event, function<bool, size_t>);
  yw_control_getter_setter(item_padding, float4);
  yw_control_getter_setter(text_color, color);
  auto text_color(this auto& Self, none None) noexcept {
    return typename remove_cvref<decltype(Self)>::proxy(get_slot(&Self)).text_color(None);
  }
  const auto& text(size_t Index) const noexcept { return proxy(get_slot(this)).text(Index); }
  const auto& selected_text() const noexcept { return proxy(get_slot(this)).selected_text(); }
  const auto& string(size_t Index) const noexcept { return proxy(get_slot(this)).string(Index); }
  const auto& selected_string() const noexcept { return proxy(get_slot(this)).selected_string(); }

  auto& add(this auto& self, yw::string<wchar_t> String) noexcept {
    const auto sp = get_slot(&self);
    if (!sp) error(errors::invalid_slotid).fizzle_out();
    else if (const auto lbsp = get_slot<listbox>(sp->dropdown_listbox.id())) {
      lbsp->items.emplace_back(std::move(String));
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
      if (Index >= lbsp->items.size())
        error(errors::invalid_argument, format("invalid item index: ", Index)).fizzle_out();
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

  auto text(this auto& Self, size_t Index, yw::text Text) noexcept {
    return typename remove_cvref<decltype(Self)>::proxy(get_slot(&Self)).text(Index, std::move(Text));
  }

  auto string(this auto& Self, size_t Index, yw::string<wchar_t> String) noexcept {
    return typename remove_cvref<decltype(Self)>::proxy(get_slot(&Self)).string(Index, std::move(String));
  }

  auto font(this auto& Self, font_config Font) noexcept {
    return typename remove_cvref<decltype(Self)>::proxy(get_slot(&Self)).font(std::move(Font));
  }
};
} // namespace yw::ui
