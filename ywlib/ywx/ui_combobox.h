// #pragma once
// #include <ywx/ui_listbox.h>
// #include <ywx/window.h>

// namespace yw::ui {

// class combobox : public label {
// public:
//   struct slot : label::slot {
//     std::vector<yw::text> items{};
//     size_t selected = npos;
//     slotid dropdown_window_id{};
//     slotid dropdown_listbox_id{};
//     color selection_color = color(colors::dodgerblue, 0.35f);
//     color pressed_overlay_color = color(0.0f, 0.0f, 0.0f, 0.10f);
//     float4 item_padding = float4::fill(arbitrary_value);
//     float button_width = 18.0f;
//     float dropdown_height = 160.0f;
//     bool pressed = false;
//     bool close_dropdown_requested = false;

//     function<void, size_t> on_change{};

//     virtual bool focusable() const noexcept override { return enabled && visible; }

//     void close_dropdown() {
//       close_dropdown_requested = false;
//       const auto wid = std::exchange(dropdown_window_id, {});
//       dropdown_listbox_id = {};
//       if (const auto wsp = get_slot<window>(wid)) {
//         wsp->on_focus = {};
//         if (auto res = wsp->close(); !res) res.error().go_off();
//       }
//     }

//     void request_close_dropdown() {
//       close_dropdown_requested = true;
//       make_dirty();
//     }

//     void select(size_t Index, bool Notify = true) {
//       if (Index >= items.size()) return;
//       const bool changed = selected != Index;
//       selected = Index;
//       if (auto res = text.string(items[selected].string()); !res) res.error().go_off();
//       make_messy();
//       if (changed && Notify && on_change) on_change(selected);
//     }

//     std::expected<void, error> open_dropdown() {
//       if (dropdown_window_id || items.empty()) return {};
//       const auto parent_wsp = get_slot<window>(window_id);
//       if (!parent_wsp) return std::unexpected(error(errors::invalid_slotid));

//       const int2 dropdown_pos = parent_wsp->pos + parent_wsp->frame_thickness.xy() +
//                                 int2(int(std::round(pos.x)), int(std::round(pos.y + size.y)));
//       const int2 dropdown_size{
//         yw::max(int(std::ceil(size.x)), int(arbitrary_value)),
//         yw::max(int(std::ceil(dropdown_height)), int(arbitrary_value))};
//       window::options op{
//         .pos = dropdown_pos,
//         .size = dropdown_size,
//         .has_border = true,
//         .has_caption = false,
//         .resizable = false,
//         .visible = true};

//       auto wres = window::slot::create(std::move(op), window_id);
//       if (!wres) return wres.error().relay();
//       const auto dropdown_wsp = *wres;
//       dropdown_window_id = dropdown_wsp->id;

//       const auto lb_id = make_slot<listbox>();
//       const auto lbsp = get_slot<listbox>(lb_id);
//       if (!lbsp) {
//         close_dropdown();
//         return std::unexpected(error(errors::slot_creation_failed));
//       }
//       if (auto res = dropdown_wsp->attach(lb_id); !res) {
//         slot::slots.erase(lb_id);
//         close_dropdown();
//         return res.error().relay();
//       }
//       dropdown_listbox_id = lb_id;
//       lbsp->id = lb_id;
//       lbsp->window_id = dropdown_wsp->id;
//       lbsp->items.reserve(items.size());
//       for (const auto& item : items) {
//         auto tres = yw::text::create(item.string(), item.font());
//         if (!tres) {
//           close_dropdown();
//           return tres.error().relay();
//         }
//         lbsp->items.push_back(std::move(*tres));
//       }
//       lbsp->selected = selected < items.size() ? selected : npos;
//       lbsp->colors = colors;
//       lbsp->text_color = text_color;
//       lbsp->selection_color = selection_color;
//       lbsp->item_padding = item_padding;
//       lbsp->policy = vector2<size_policy>::fill(size_policy::free);
//       lbsp->ensure_visible(lbsp->selected);

//       const auto combobox_id = id;
//       dropdown_wsp->on_focus = [combobox_id](bool Focused) {
//         if (Focused) return;
//         if (const auto sp = get_slot<combobox>(combobox_id)) sp->close_dropdown();
//       };
//       lbsp->on_change = [combobox_id](size_t Index) {
//         if (const auto sp = get_slot<combobox>(combobox_id)) {
//           sp->select(Index);
//           sp->request_close_dropdown();
//         }
//       };
//       dropdown_wsp->dirty = dropdown_wsp->messy = true;
//       return {};
//     }

//     std::expected<void, error> toggle_dropdown() {
//       if (dropdown_window_id) close_dropdown();
//       else if (auto res = open_dropdown(); !res) return res.error().relay();
//       return {};
//     }

//     float4 button_rect() const noexcept {
//       return {pos.x + size.x - button_width, pos.y, pos.x + size.x, pos.y + size.y};
//     }

//     std::expected<void, error> draw_button() const {
//       const auto r = button_rect();
//       const float2 p{r.x, r.y};
//       const float2 s{r.z - r.x, r.w - r.y};
//       brush::color(colors.border);
//       if (auto res = draw_line({r.x, r.y}, {r.x, r.w}, border_thickness); !res) return res.error().relay();
//       if (pressed && pressed_overlay_color.a > 0.0f) {
//         brush::color(pressed_overlay_color);
//         if (auto res = fill_rectangle(p, s); !res) return res.error().relay();
//       }
//       brush::color(colors.border);
//       const float cx = (r.x + r.z) * 0.5f;
//       const float cy = (r.y + r.w) * 0.5f;
//       const float aw = yw::clamp(button_width * 0.22f, 3.0f, 6.0f);
//       const float ah = yw::clamp(button_width * 0.16f, 2.0f, 5.0f);
//       if (auto res = draw_line({cx - aw, cy - ah}, {cx, cy + ah}, 1.0f); !res) return res.error().relay();
//       if (auto res = draw_line({cx, cy + ah}, {cx + aw, cy - ah}, 1.0f); !res) return res.error().relay();
//       return {};
//     }

//     virtual std::expected<float2, error> get_necessary_size() const override {
//       float2 item_size{};
//       for (const auto& item : items) item_size = vapply_r<float2>(yw::max, item_size, item.size());
//       item_size.x += button_width;
//       const auto inner = item_size + padding.xy() + padding.zw();
//       return calc_necessary_size_by_policy(inner);
//     }

//     virtual std::expected<void, error> redraw() override {
//       if (close_dropdown_requested) close_dropdown();
//       if (geometry_dirty) {
//         geometry_dirty = false;
//         if (auto res = relocate(); !res) return res.error().relay();
//       }
//       if (!visible) return {};
//       if (auto res = draw_frame_background(); !res) return res.error().relay();
//       brush::color(text_color);
//       if (auto res = text.draw(pos + calc_text_offset()); !res) return res.error().relay();
//       if (auto res = draw_button(); !res) return res.error().relay();
//       if (auto res = draw_frame_foreground(); !res) return res.error().relay();
//       return {};
//     }

//     virtual bool button_event(yw::button_event e) override {
//       if (!enabled || !visible || e.key != keys::lbutton) return false;
//       pressed = e.down;
//       make_dirty();
//       return true;
//     }

//     virtual bool click_event(yw::button_event e) override {
//       if (!enabled || !visible || e.down || e.key != keys::lbutton) return false;
//       if (auto res = toggle_dropdown(); !res) res.error().go_off();
//       return true;
//     }

//     virtual std::expected<void, error> reset_state() override {
//       if (!pressed) return {};
//       pressed = false;
//       make_dirty();
//       return {};
//     }

//     virtual void focus_event(bool Focused) override {
//       if (!Focused) {
//         pressed = false;
//         make_dirty();
//       }
//       label::slot::focus_event(Focused);
//     }

//     virtual bool key_event(yw::key_event e) override {
//       if (!enabled || !visible) return false;
//       if (!e.down) {
//         if (e.key == keys::space || e.key == keys::enter || e.key == keys::down || e.key == keys::up) return true;
//         return label::slot::key_event(e);
//       }
//       if (e.key == keys::space || e.key == keys::enter || e.key == keys::down) {
//         if (auto res = open_dropdown(); !res) res.error().go_off();
//         return true;
//       }
//       if (e.key == keys::up) {
//         if (selected != npos && selected > 0) select(selected - 1);
//         return true;
//       }
//       return label::slot::key_event(e);
//     }
//   };

//   using label::operator bool;
//   combobox() noexcept = default;

//   combobox(derived_from<interface> auto& Parent, strict<bool> AutoColor = true, const source_line& sl = here()) {
//     if (auto res = create(Parent, AutoColor)) *this = std::move(*res);
//     else res.error().add_footprint().go_off(sl);
//   }

//   static std::expected<combobox, error> create(derived_from<interface> auto& Parent, strict<bool> AutoColor = true) {
//     combobox c;
//     const auto temp_id = make_slot<combobox>();
//     const auto sp = get_slot<combobox>(temp_id);
//     if (!sp) return std::unexpected(error(errors::slot_creation_failed));
//     const auto psp = get_slot<control>(Parent.id());
//     if (!psp) return std::unexpected(error(errors::invalid_slotid));
//     if (auto res = psp->attach(temp_id); !res) {
//       slot::slots.erase(temp_id);
//       return res.error().relay();
//     }
//     c._id = temp_id;
//     sp->id = temp_id;
//     sp->window_id = psp->get_window_id();
//     sp->policy = {ui::size_policy::fit, ui::size_policy::fit};
//     sp->text_align = alignment::left;
//     if (AutoColor) {
//       sp->colors = color_pair(none{});
//       sp->text_color = sp->colors.border;
//     }
//     return c;
//   }

//   //-- getter --//

//   size_t item_count() const noexcept {
//     const auto sp = get_slot(this);
//     if (!sp) error(errors::invalid_slotid).go_off();
//     return sp->items.size();
//   }

//   size_t selected_index() const noexcept { ywlib_control_get(selected); }
//   bool dropdown_open() const noexcept { return get_slot(this)->dropdown_window_id; }
//   float button_width() const noexcept { ywlib_control_get(button_width); }
//   float dropdown_height() const noexcept { ywlib_control_get(dropdown_height); }
//   const auto& on_change() const noexcept { ywlib_control_get(on_change); }
//   const auto& selection_color() const noexcept { ywlib_control_get(selection_color); }
//   const auto& item_padding() const noexcept { ywlib_control_get(item_padding); }

//   const auto& text(size_t Index) const noexcept {
//     const auto sp = get_slot(this);
//     if (!sp) error(errors::invalid_slotid).go_off();
//     if (Index >= sp->items.size()) error(errors::invalid_argument, format("invalid item index: ", Index)).go_off();
//     return sp->items[Index];
//   }

//   const auto& string(size_t Index) const noexcept { return text(Index).string(); }

//   //-- setter --//

//   auto& add(this auto& self, yw::string<wchar_t> s) noexcept {
//     const auto sp = get_slot(&self);
//     if (!sp) error(errors::invalid_slotid).go_off();
//     sp->close_dropdown();
//     if (auto res = yw::text::create(std::move(s))) sp->items.push_back(std::move(*res));
//     else {
//       res.error().go_off();
//       return self;
//     }
//     if (sp->selected == npos) sp->select(0, false);
//     sp->make_messy();
//     return self;
//   }

//   auto& add(this auto& self, yw::text t) noexcept {
//     const auto sp = get_slot(&self);
//     if (!sp) error(errors::invalid_slotid).go_off();
//     sp->close_dropdown();
//     sp->items.push_back(std::move(t));
//     if (sp->selected == npos) sp->select(0, false);
//     sp->make_messy();
//     return self;
//   }

//   auto& insert(this auto& self, size_t Index, yw::string<wchar_t> s) noexcept {
//     const auto sp = get_slot(&self);
//     if (!sp) error(errors::invalid_slotid).go_off();
//     if (Index > sp->items.size()) {
//       error(errors::invalid_argument, format("invalid item index: ", Index)).go_off();
//       return self;
//     }
//     sp->close_dropdown();
//     if (auto res = yw::text::create(std::move(s))) sp->items.insert(sp->items.begin() + Index, std::move(*res));
//     else {
//       res.error().go_off();
//       return self;
//     }
//     if (sp->selected == npos) sp->select(0, false);
//     else if (Index <= sp->selected) ++sp->selected;
//     sp->make_messy();
//     return self;
//   }

//   auto& insert(this auto& self, size_t Index, yw::text t) noexcept {
//     const auto sp = get_slot(&self);
//     if (!sp) error(errors::invalid_slotid).go_off();
//     if (Index > sp->items.size()) {
//       error(errors::invalid_argument, format("invalid item index: ", Index)).go_off();
//       return self;
//     }
//     sp->close_dropdown();
//     sp->items.insert(sp->items.begin() + Index, std::move(t));
//     if (sp->selected == npos) sp->select(0, false);
//     else if (Index <= sp->selected) ++sp->selected;
//     sp->make_messy();
//     return self;
//   }

//   auto& erase(this auto& self, size_t Index) noexcept {
//     const auto sp = get_slot(&self);
//     if (!sp) error(errors::invalid_slotid).go_off();
//     if (Index >= sp->items.size()) {
//       error(errors::invalid_argument, format("invalid item index: ", Index)).go_off();
//       return self;
//     }
//     sp->close_dropdown();
//     sp->items.erase(sp->items.begin() + Index);
//     if (sp->items.empty()) {
//       sp->selected = npos;
//       if (auto res = sp->text.string(L""); !res) res.error().go_off();
//     } else if (sp->selected == Index) sp->select(yw::min(Index, sp->items.size() - 1), false);
//     else if (Index < sp->selected) --sp->selected;
//     sp->make_messy();
//     return self;
//   }

//   auto& clear(this auto& self) noexcept {
//     const auto sp = get_slot(&self);
//     if (!sp) error(errors::invalid_slotid).go_off();
//     sp->close_dropdown();
//     sp->items.clear();
//     sp->selected = npos;
//     if (auto res = sp->text.string(L""); !res) res.error().go_off();
//     sp->make_messy();
//     return self;
//   }

//   auto& selected_index(this auto& self, size_t Index) noexcept {
//     const auto sp = get_slot(&self);
//     if (!sp) error(errors::invalid_slotid).go_off();
//     if (Index >= sp->items.size()) {
//       error(errors::invalid_argument, format("invalid item index: ", Index)).go_off();
//       return self;
//     }
//     sp->select(Index);
//     return self;
//   }

//   auto& open(this auto& self) noexcept {
//     const auto sp = get_slot(&self);
//     if (!sp) error(errors::invalid_slotid).go_off();
//     if (auto res = sp->open_dropdown(); !res) res.error().go_off();
//     return self;
//   }

//   auto& close(this auto& self) noexcept {
//     const auto sp = get_slot(&self);
//     if (!sp) error(errors::invalid_slotid).go_off();
//     sp->close_dropdown();
//     return self;
//   }

//   auto& button_width(this auto& self, float1 v) noexcept {
//     const auto sp = get_slot(&self);
//     if (!sp) error(errors::invalid_slotid).go_off();
//     if (v.x <= 0.0f) {
//       error(errors::invalid_argument, "combobox button_width must be positive").go_off();
//       return self;
//     }
//     sp->button_width = v.x;
//     sp->make_messy();
//     return self;
//   }

//   auto& dropdown_height(this auto& self, float1 v) noexcept {
//     const auto sp = get_slot(&self);
//     if (!sp) error(errors::invalid_slotid).go_off();
//     if (v.x <= 0.0f) {
//       error(errors::invalid_argument, "combobox dropdown_height must be positive").go_off();
//       return self;
//     }
//     sp->dropdown_height = v.x;
//     return self;
//   }

//   auto& on_change(this auto& self, function<void, size_t> f) noexcept { ywlib_control_set(on_change, std::move(f), none); }
//   auto& selection_color(this auto& self, const color& c) noexcept { ywlib_control_set(selection_color, c, dirty); }
//   auto& item_padding(this auto& self, float4 f) noexcept { ywlib_control_set(item_padding, f, messy); }

//   auto& text(this auto& self, size_t Index, yw::text t) noexcept {
//     const auto sp = get_slot(&self);
//     if (!sp) error(errors::invalid_slotid).go_off();
//     if (Index >= sp->items.size()) {
//       error(errors::invalid_argument, format("invalid item index: ", Index)).go_off();
//       return self;
//     }
//     sp->close_dropdown();
//     sp->items[Index] = std::move(t);
//     if (sp->selected == Index) sp->select(Index, false);
//     sp->make_messy();
//     return self;
//   }

//   auto& string(this auto& self, size_t Index, yw::string<wchar_t> s) noexcept {
//     const auto sp = get_slot(&self);
//     if (!sp) error(errors::invalid_slotid).go_off();
//     if (Index >= sp->items.size()) {
//       error(errors::invalid_argument, format("invalid item index: ", Index)).go_off();
//       return self;
//     }
//     sp->close_dropdown();
//     if (auto res = sp->items[Index].string(std::move(s)); !res) res.error().go_off();
//     if (sp->selected == Index) sp->select(Index, false);
//     sp->make_messy();
//     return self;
//   }

//   auto& font(this auto& self, font_config f) noexcept {
//     const auto sp = get_slot(&self);
//     if (!sp) error(errors::invalid_slotid).go_off();
//     sp->close_dropdown();
//     for (auto& item : sp->items)
//       if (auto res = item.font(f); !res) res.error().go_off();
//     if (sp->selected < sp->items.size()) sp->select(sp->selected, false);
//     sp->make_messy();
//     return self;
//   }
// };
// } // namespace yw::ui
