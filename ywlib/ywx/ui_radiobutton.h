// #pragma once
// #include "ywx/ui_checkbox.h"

// namespace yw::ui {

// class radiobutton : public vertical_layout {
//   static constexpr float2 default_icon_size{16.0f, 16.0f};
//   static constexpr std::string_view box_path = "M 8 0 A 8 8 0 1 0 8 16 A 8 8 0 1 0 8 0";
//   static constexpr std::string_view check_path = "M 8 4 A 4 4 0 1 0 8 12 A 4 4 0 1 0 8 4";

// public:
//   /// MARK: slot

//   struct slot : public vertical_layout::slot {
//     std::vector<checkbox> items;
//     color_pair default_color{colors::transparent, colors::black};
//     size_t selected_index = npos;
//     size_t focused_index = npos;
//     bool locked = true;
//     bool updating = false;

//     function<void, size_t> on_change;
//     function<void, bool> on_focus;
//     function<void, key> on_click;

//     virtual bool attachable() const override { return !locked; }
//     virtual std::expected<void, error_trace> attach(slotid Child) override {
//       if (locked) return unexpected_error(errors::ui_not_attachable);
//       if (auto res = vertical_layout::slot::attach(Child)) return {};
//       else return unexpected_error(res.error());
//     }
//     virtual std::expected<void, error_trace> detach(slotid Child) override {
//       if (auto res = vertical_layout::slot::detach(Child)) return {};
//       else return unexpected_error(res.error());
//     }

//     std::expected<void, error_trace> select_index(size_t index, bool notify = true) {
//       if (index >= items.size()) return unexpected_error(errors::invalid_argument);
//       if (updating) return {};
//       updating = true;
//       const auto prev = selected_index;
//       if (prev < items.size() && prev != index) items[prev].checked(false);
//       selected_index = index;
//       focused_index = index;
//       items[index].checked(true);
//       updating = false;
//       if (auto res = make_dirty(); !res) return unexpected_error(res.error());
//       if (notify && prev != index && on_change) on_change(index);
//       return {};
//     }

//     std::expected<void, error_trace> item_change(size_t index, bool checked) {
//       if (index >= items.size()) return unexpected_error(errors::invalid_argument);
//       if (updating) return {};
//       if (checked) return select_index(index);
//       if (selected_index == index) {
//         updating = true;
//         items[index].checked(true);
//         updating = false;
//       }
//       return {};
//     }

//     std::expected<void, error_trace> item_focus(size_t index, bool focused) {
//       if (index >= items.size()) return unexpected_error(errors::invalid_argument);
//       if (focused) focused_index = index;
//       else if (focused_index == index) focused_index = npos;
//       if (auto res = make_dirty(); !res) return unexpected_error(res.error());
//       if (enabled && on_focus) on_focus(focused);
//       return {};
//     }
//   };

//   class icon_accessor : public accessor<radiobutton> {
//     using accessor<radiobutton>::slot;

//     icon_style& style;
//     bool check_icon = false;

//     template<typename Fn> void apply(Fn&& fn) {
//       for (auto& item : slot.items) fn(radiobutton::select_icon(item, check_icon));
//       this->dirty = true;
//     }

//   public:
//     icon_accessor(radiobutton::slot& Slot, icon_style& Style, bool CheckIcon) noexcept :
//       accessor<radiobutton>(Slot), style(Style), check_icon(CheckIcon) {}

//     const auto& icon() const { return style.icon; }
//     auto& icon(svgpath Icon) {
//       style.icon = std::move(Icon);
//       apply([&](auto&& accessor) { accessor.icon(assume(svgpath::create(style.icon))); });
//       return *this;
//     }
//     const auto& stroke_color() const { return style.stroke_color; }
//     auto& stroke_color(color StrokeColor) {
//       style.stroke_color = StrokeColor;
//       apply([&](auto&& accessor) { accessor.stroke_color(style.stroke_color); });
//       return *this;
//     }
//     const auto& fill_color() const { return style.fill_color; }
//     auto& fill_color(color FillColor) {
//       style.fill_color = FillColor;
//       apply([&](auto&& accessor) { accessor.fill_color(style.fill_color); });
//       return *this;
//     }
//     const auto& stroke_width() const { return style.stroke_width; }
//     auto& stroke_width(float StrokeWidth) {
//       style.stroke_width = StrokeWidth;
//       apply([&](auto&& accessor) { accessor.stroke_width(style.stroke_width); });
//       return *this;
//     }
//   };

// private:
//   static std::expected<svgpath, error_trace> create_box_icon() { return svgpath::create(default_icon_size, box_path); }
//   static std::expected<svgpath, error_trace> create_check_icon() { return svgpath::create(default_icon_size, check_path); }
//   static ui::icon::icon_accessor select_icon(checkbox& item, bool check) {
//     if (check) return item.check_icon().icon_();
//     else return item.box_icon().icon_();
//   }

//   static void apply_icon_style(checkbox& item, const icon_style& style, bool check) {
//     select_icon(item, check)
//       .icon(assume(svgpath::create(style.icon)))
//       .fill_color(style.fill_color)
//       .stroke_color(style.stroke_color)
//       .stroke_width(style.stroke_width);
//   }

// public:
//   using control::operator bool;
//   radiobutton() noexcept = default;

//   static std::expected<radiobutton, error_trace> add(
//     derived_from<unknown> auto& Layout, const color_pair& Colors = color_pair::auto_color()) {
//     radiobutton rb;
//     if (auto res = create_control<radiobutton>(Layout)) rb._id = *res;
//     else return unexpected_error(res.error());
//     if (const auto csp = system::slot_address<radiobutton>(rb._id)) {
//       if (auto box = create_box_icon()) csp->box_icon_style.icon = std::move(*box);
//       else return unexpected_error(box.error());
//       if (auto check = create_check_icon()) csp->check_icon_style.icon = std::move(*check);
//       else return unexpected_error(check.error());
//       csp->default_color = Colors;
//       csp->background_color = Colors.background;
//     } else return unexpected_error(errors::ui_invalid_slotid);
//     return rb;
//   }

//   size_t item_count() const {
//     const auto csp = system::slot_address<radiobutton>(_slotid());
//     if (!csp) fatal_error(errors::ui_invalid_slotid);
//     return csp->items.size();
//   }

//   std::expected<void, error_trace> append_item(std::wstring s) {
//     const auto csp = system::slot_address<radiobutton>(_slotid());
//     if (!csp) return unexpected_error(errors::ui_invalid_slotid);
//     csp->locked = false;
//     auto res = checkbox::add(*this, csp->default_color, false);
//     csp->locked = true;
//     if (!res) return unexpected_error(res.error());

//     const auto index = csp->items.size();
//     apply_icon_style(*res, csp->box_icon_style, false);
//     apply_icon_style(*res, csp->check_icon_style, true);
//     res->text().text().string(std::move(s));
//     if (auto r = res->on_change([csp, index](bool checked) {
//           if (auto res = csp->item_change(index, checked); !res) fatal_error(res.error());
//         });
//       !r)
//       return unexpected_error(r.error());
//     if (auto r = res->on_focus([csp, index](bool focused) {
//           if (auto res = csp->item_focus(index, focused); !res) fatal_error(res.error());
//         });
//       !r)
//       return unexpected_error(r.error());
//     if (auto r = res->on_click([csp](key k) {
//           if (csp->on_click) csp->on_click(k);
//         });
//       !r)
//       return unexpected_error(r.error());

//     csp->items.push_back(std::move(*res));
//     if (csp->selected_index == npos) {
//       csp->selected_index = index;
//       csp->focused_index = index;
//       csp->updating = true;
//       csp->items.back().checked(true);
//       csp->updating = false;
//     }
//     if (auto r = csp->make_messy(); !r) return unexpected_error(r.error());
//     return {};
//   }

//   std::expected<void, error_trace> clear_items() {
//     const auto csp = system::slot_address<radiobutton>(_slotid());
//     if (!csp) return unexpected_error(errors::ui_invalid_slotid);
//     csp->locked = false;
//     if (auto res = clear(); !res) {
//       csp->locked = true;
//       return unexpected_error(res.error());
//     }
//     csp->locked = true;
//     csp->items.clear();
//     csp->selected_index = npos;
//     csp->focused_index = npos;
//     csp->updating = false;
//     if (auto res = csp->make_dirty(); !res) return unexpected_error(res.error());
//     return {};
//   }

//   size_t checked() const { return unsafe_get(&radiobutton::slot::selected_index); }

//   auto& checked(size_t index) {
//     const auto csp = system::slot_address<radiobutton>(_slotid());
//     if (!csp) fatal_error(errors::ui_invalid_slotid);
//     assume(csp->select_index(index));
//     return *this;
//   }

//   template<typename Self> decltype(auto) text(this Self&& self, size_t index) {
//     const auto csp = system::slot_address<radiobutton>(self._slotid());
//     if (!csp) fatal_error(errors::ui_invalid_slotid);
//     if (index >= csp->items.size()) fatal_error(errors::invalid_argument);
//     return csp->items[index].text().text();
//   }

//   decltype(auto) box_icon() {
//     const auto csp = system::slot_address<radiobutton>(_slotid());
//     if (!csp) fatal_error(errors::ui_invalid_slotid);
//     return icon_accessor(*csp, csp->box_icon_style, false);
//   }

//   decltype(auto) check_icon() {
//     const auto csp = system::slot_address<radiobutton>(_slotid());
//     if (!csp) fatal_error(errors::ui_invalid_slotid);
//     return icon_accessor(*csp, csp->check_icon_style, true);
//   }

//   const auto& on_change() const { return unsafe_get(&radiobutton::slot::on_change); }
//   std::expected<void, error_trace> on_change(function<void, size_t> f) {
//     if (auto res = safe_set(&radiobutton::slot::on_change, std::move(f))) return {};
//     else return unexpected_error(res.error());
//   }

//   const auto& on_click() const { return unsafe_get(&radiobutton::slot::on_click); }
//   std::expected<void, error_trace> on_click(function<void, key> f) {
//     if (auto res = safe_set(&radiobutton::slot::on_click, std::move(f))) return {};
//     else return unexpected_error(res.error());
//   }

//   const auto& on_focus() const { return unsafe_get(&radiobutton::slot::on_focus); }
//   std::expected<void, error_trace> on_focus(function<void, bool> f) {
//     if (auto res = safe_set(&radiobutton::slot::on_focus, std::move(f))) return {};
//     else return unexpected_error(res.error());
//   }
// };
// } // namespace yw::ui
