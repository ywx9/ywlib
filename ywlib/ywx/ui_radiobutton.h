// #pragma once
// #include "ywx/ui_checkbox.h"
// #include "ywx/ui_layout.h"

// namespace yw::ui {

// class radiobutton : public vertical_layout {
//   static constexpr std::string_view box_path = "M 10 2 A 8 8 0 1 0 10 18 A 8 8 0 1 0 10 2";
//   static constexpr std::string_view mark_path = "M 10 6 A 4 4 0 1 0 10 14 A 4 4 0 1 0 10 6";

// public:
//   struct slot : public vertical_layout::slot {
//     std::vector<checkbox> items;
//     size_t selected_index = npos, focused_index = npos;
//     function<void, size_t> on_change;
//     function<void, bool> on_focus;
//     function<void, key> on_click;
//     key captured_key{};
//     bool locked = true;

//     //-- overrides --//

//     virtual std::expected<void, error_trace> attachable() const override {
//       if (locked) return unexpected_error(errors::ui_not_attachable);
//       return {};
//     }
//     virtual std::expected<void, error_trace> attach(slotid Child) override {
//       if (locked) return unexpected_error(errors::ui_not_attachable);
//       if (auto res = vertical_layout::slot::attach(Child)) return {};
//       else return unexpected_error(res.error());
//     }
//     virtual std::expected<void, error_trace> detach(slotid Child) override {
//       if (locked) return {};
//       if (auto res = vertical_layout::slot::detach(Child)) return {};
//       else return unexpected_error(res.error());
//     }

//     virtual bool focusable() const override { return enabled && !items.empty(); }
//     virtual slotid hittest(float2 Pt) const override { return core.hittest(Pt) ? id : slotid(); }
//     virtual slotid next_tab_stop(slotid Focused, bool Forward, bool& Found) const override {
//       if (Focused == id) Found = true;
//       else if (Found && visible) return id;
//       return {};
//     }

//     size_t normalize_focus() const {
//       if (items.empty()) return npos;
//       if (focused_index < items.size()) return focused_index;
//       if (selected_index < items.size()) return selected_index;
//       return 0;
//     }

//     size_t hit_item(float2 Pt) const {
//       for (size_t i = {}; i < items.size(); ++i) {
//         const auto item_sp = system::slot_address<checkbox>(items[i]._slotid());
//         if (item_sp && item_sp->core.hittest(Pt)) return i;
//       }
//       return npos;
//     }

//     std::expected<void, error_trace> select_index(size_t index) {
//       if (index >= items.size()) return unexpected_error(errors::invalid_argument);
//       const bool changed = selected_index != index;
//       if (selected_index < items.size()) {
//         if (const auto sp = system::slot_address<checkbox>(items[selected_index]._slotid())) sp->checkbox.checked = false;
//         else return unexpected_error(errors::ui_invalid_slotid);
//       }
//       selected_index = index;
//       focused_index = index;
//       if (const auto sp = system::slot_address<checkbox>(items[selected_index]._slotid())) sp->checkbox.checked = true;
//       else return unexpected_error(errors::ui_invalid_slotid);
//       if (auto res = make_dirty(); !res) return unexpected_error(res.error());
//       if (changed && on_change) on_change(index);
//       return {};
//     }

//     virtual std::expected<void, error_trace> draw_focus_ring(const parts::focus_ring& fr) override {
//       /// \note draw forcus ring around focused item
//       const auto fsp = system::slot_address<checkbox>(items[focused_index]._slotid());
//       if (!fsp) return unexpected_error(errors::ui_invalid_slotid);
//       if (auto res = fsp->draw_focus_ring(fr)) return {};
//       else return unexpected_error(res.error());
//     }

//     virtual void click_event(events::button e) override {
//       if (enabled && e.code == captured_key)
//         if (const auto index = hit_item(float2(e.pos)); index != npos) {
//           select_index(index);
//           if (on_click) on_click(captured_key);
//         }
//       captured_key = {};
//     }

//     virtual void button_event(events::button e) override {
//       if (!enabled) {
//         captured_key = {};
//         return;
//       }
//       if (e.down) {
//         captured_key = e.code;
//         if (e.code == keys::lbutton)
//           if (const auto index = hit_item(float2(e.pos)); index != npos && focused_index != index) {
//             focused_index = index;
//             make_dirty();
//           }
//       } else captured_key = {};
//     }

//     virtual void focus_event(bool focused) override {
//       if (focused) focused_index = normalize_focus();
//       else captured_key = {}, focused_index = npos;
//       make_dirty();
//       if (enabled && on_focus) on_focus(focused);
//     }

//     virtual bool key_event(events::key e) override {
//       if (!enabled || items.empty()) return false;
//       if (e.down) {
//         switch (e.code.code) {
//         case keys::up.code:
//         case keys::left.code: {
//           captured_key = {};
//           const auto current = normalize_focus();
//           const auto next = current ? current - 1 : items.size() - 1;
//           if (current != next) {
//             focused_index = next;
//             assume(make_dirty());
//           }
//           return true;
//         }
//         case keys::down.code:
//         case keys::right.code: {
//           captured_key = {};
//           const auto current = normalize_focus();
//           const auto next = (current + 1) % items.size();
//           if (current != next) {
//             focused_index = next;
//             assume(make_dirty());
//           }
//           return true;
//         }
//         case keys::space.code:
//         case keys::enter.code:
//           captured_key = e.code;
//           return true;
//         default:
//           captured_key = {};
//           return false;
//         }
//       }
//       if (captured_key == e.code) {
//         if (const auto index = normalize_focus(); index != npos) {
//           assume(select_index(index));
//           if (on_click) on_click(captured_key);
//         }
//       }
//       const bool handled = e.code == keys::space || e.code == keys::enter;
//       captured_key = {};
//       return handled;
//     }
//   };

//   using control::operator bool;
//   radiobutton() noexcept = default;

//   static std::expected<radiobutton, error_trace> add(derived_from<unknown> auto& Layout) {
//     radiobutton rb;
//     if (auto res = create_control<radiobutton>(Layout)) rb._id = *res;
//     else return unexpected_error(res.error());
//     if (const auto csp = system::slot_address<radiobutton>(rb._id)) {
//       const auto [bg_color, _] = control::get_auto_color();
//       csp->background.control_id = rb._id;
//       csp->background.color = bg_color;
//       csp->border.control_id = rb._id;
//     } else return unexpected_error(errors::ui_invalid_slotid);
//     return rb;
//   }

//   size_t item_count() const {
//     const auto csp = system::slot_address<radiobutton>(_id);
//     if (!csp) fatal_error(errors::ui_invalid_slotid);
//     return csp->items.size();
//   }

//   std::expected<void, error_trace> append_item(std::wstring s) {
//     const auto csp = system::slot_address<radiobutton>(_id);
//     if (!csp) return unexpected_error(errors::ui_invalid_slotid);
//     csp->locked = false;
//     if (auto res = checkbox::add(*this)) {
//       res->icon().box_icon(assume(svgpath::create(parts::icon::default_size, box_path)));
//       res->icon().box_fill_color(colors::white);
//       res->icon().box_stroke_color(colors::black);
//       res->icon().check_icon(assume(svgpath::create(parts::icon::default_size, mark_path)));
//       res->icon().check_fill_color(colors::black);
//       res->icon().check_stroke_width(0.0f);
//       res->text().string(std::move(s));
//       csp->items.push_back(std::move(*res));
//       if (csp->selected_index == npos) {
//         csp->selected_index = 0;
//         csp->focused_index = 0;
//         csp->items.back().checked(true);
//       }
//     } else return unexpected_error(res.error());
//     csp->locked = true;
//     return {};
//   }

//   void clear_items() {
//     const auto csp = system::slot_address<radiobutton>(_id);
//     if (!csp) fatal_error(errors::ui_invalid_slotid);
//     csp->locked = false;
//     assume(clear());
//     csp->locked = true;
//     csp->items.clear();
//     csp->selected_index = npos;
//     csp->focused_index = npos;
//     assume(csp->make_dirty());
//   }

//   size_t checked() const { return unsafe_get(&radiobutton::slot::selected_index); }

//   auto& checked(size_t index) {
//     const auto csp = system::slot_address<radiobutton>(_id);
//     if (!csp) fatal_error(errors::ui_invalid_slotid);
//     assume(csp->select_index(index));
//     return *this;
//   }

//   template<typename Self> auto&& item(this Self&& self, size_t index) {
//     const auto csp = system::slot_address<radiobutton>(self._id);
//     if (!csp) fatal_error(errors::ui_invalid_slotid);
//     if (index >= csp->items.size()) fatal_error(errors::invalid_argument);
//     if constexpr (!is_const<remove_ref<Self>>) return csp->items[index];
//     else return std::as_const(csp->items[index]);
//   }

//   const auto& on_change() const { return unsafe_get(&radiobutton::slot::on_change); }
//   void on_change(function<void, size_t> f) { unsafe_set(&radiobutton::slot::on_change, std::move(f)); }

//   const auto& on_click() const { return unsafe_get(&radiobutton::slot::on_click); }
//   void on_click(function<void, key> f) { unsafe_set(&radiobutton::slot::on_click, std::move(f)); }

//   const auto& on_focus() const { return unsafe_get(&radiobutton::slot::on_focus); }
//   void on_focus(function<void, bool> f) { unsafe_set(&radiobutton::slot::on_focus, std::move(f)); }
// };
// } // namespace yw::ui
