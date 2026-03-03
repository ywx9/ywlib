// #pragma once
// #include "ywx/focusable_base.h"
// #include "ywx/icon.h"
// #include "ywx/label.h"
// #include "ywx/svgpath.h"

// namespace yw::ui {

// //////////////////////////////////////// MARK: ui::checkbox

// class checkbox : public focusable_base {
// public:
//   class slot : public focusable_base::slot {
//   public:
//     bool checked = false;
//     icon box_icon{};
//     icon mark_icon{};
//     label label_slot{};
//     float2 margin{4.0f, 4.0f};
//     float2 icon_size{16.0f, 16.0f};
//     color icon_color = colors::black;
//     function<void, bool> on_toggle;

//     virtual std::expected<void, error_trace> proc(UINT msg, WPARAM wp, LPARAM lp) override {
//       switch (msg) {
//       case WM_LBUTTONDOWN:
//         return {};
//       case WM_LBUTTONUP: {
//         const auto pt = float2(std::bit_cast<short2>(static_cast<uint32_t>(lp & 0xFFFFFFFF)));
//         if (hit_test(pt)) {
//           checked = !checked;
//           if (on_toggle) on_toggle(checked);
//           if (const auto w = system::windows.get(window_id)) w->dirty = true;
//         }
//         return {};
//       }
//       case WM_KEYDOWN:
//         if ((wp == VK_SPACE || wp == VK_RETURN)) return {};
//         break;
//       case WM_KEYUP:
//         if ((wp == VK_SPACE || wp == VK_RETURN)) {
//           checked = !checked;
//           if (on_toggle) on_toggle(checked);
//           if (const auto w = system::windows.get(window_id)) w->dirty = true;
//         }
//         break;
//       }
//       return {};
//     }

//     virtual void draw() const override {
//       // Draw background and border
//       base::slot::draw();

//       // Draw box icon (centered vertically)
//       {
//         const auto icon_pos = pos + float2{4.0f, (size.y - icon_size.y) / 2.0f};
//         auto* box_icon_slot = const_cast<icon::slot*>(
//           dynamic_cast<const icon::slot*>(box_icon._ui_slot())
//         );
//         if (box_icon_slot) {
//           auto saved_pos = box_icon_slot->pos;
//           auto saved_size = box_icon_slot->size;
//           box_icon_slot->pos = icon_pos;
//           box_icon_slot->size = icon_size;
//           box_icon_slot->draw();
//           box_icon_slot->pos = saved_pos;
//           box_icon_slot->size = saved_size;
//         }
//       }

//       // Draw mark icon if checked
//       if (checked) {
//         const auto icon_pos = pos + float2{4.0f, (size.y - icon_size.y) / 2.0f};
//         auto* mark_icon_slot = const_cast<icon::slot*>(
//           dynamic_cast<const icon::slot*>(mark_icon._ui_slot())
//         );
//         if (mark_icon_slot) {
//           auto saved_pos = mark_icon_slot->pos;
//           auto saved_size = mark_icon_slot->size;
//           mark_icon_slot->pos = icon_pos;
//           mark_icon_slot->size = icon_size;
//           mark_icon_slot->draw();
//           mark_icon_slot->pos = saved_pos;
//           mark_icon_slot->size = saved_size;
//         }
//       }

//       // Draw label text
//       {
//         const auto text_pos = pos + float2{4.0f + icon_size.x + 4.0f, 0};
//         const auto text_size = size - float2{4.0f + icon_size.x + 4.0f + 4.0f, 0};
//         auto* label_slot = const_cast<label::slot*>(
//           dynamic_cast<const label::slot*>(label_text._ui_slot())
//         );
//         if (label_slot) {
//           auto saved_pos = label_slot->pos;
//           auto saved_size = label_slot->size;
//           label_slot->pos = text_pos;
//           label_slot->size = text_size;
//           label_slot->draw();
//           label_slot->pos = saved_pos;
//           label_slot->size = saved_size;
//         }
//       }
//     }
//   };

// private:
//   static inline svgpath default_box_icon{};
//   static inline svgpath default_mark_icon{};
//   static inline bool icons_initialized = false;

//   static std::expected<void, error_trace> initialize_default_icons() {
//     if (icons_initialized) return {};

//     // Box icon (empty box) - 12x12 size
//     if (auto res = svgpath::create({12.0f, 12.0f}, "M2 2 L14 2 L14 14 L2 14 Z")) {
//       default_box_icon = std::move(*res);
//     } else {
//       return unexpected_error(res.error());
//     }

//     // Mark icon (checkmark) - 12x12 size to match box
//     if (auto res = svgpath::create({12.0f, 12.0f}, "M3 8 L6 11 L13 4")) {
//       default_mark_icon = std::move(*res);
//     } else {
//       return unexpected_error(res.error());
//     }

//     icons_initialized = true;
//     return {};
//   }

// protected:
//   slot* _checkbox_slot() const noexcept { return dynamic_cast<slot*>(_ui_slot()); }

//   template<typename Mp, typename T> void _checkbox_set(Mp mp, T&& value) {
//     if (const auto s = _checkbox_slot()) {
//       s->*mp = static_cast<T&&>(value);
//       if (const auto w = system::windows.get(s->window_id)) w->dirty = true;
//     }
//   }

//   template<typename Mp> const auto* _checkbox_get(Mp mp) const {
//     if (const auto s = _checkbox_slot()) return &(s->*mp);
//     else return static_cast<const remove_cvref<decltype(s->*mp)>*>(nullptr);
//   }

// public:
//   using focusable_base::focusable_base;

//   const auto& checked() const { return *_checkbox_get(&slot::checked); }
//   const auto& icon_size() const { return *_checkbox_get(&slot::icon_size); }
//   const auto& icon_color() const { return *_checkbox_get(&slot::icon_color); }
//   const auto& on_toggle() const { return *_checkbox_get(&slot::on_toggle); }

//   void checked(bool value) { _checkbox_set(&slot::checked, value); }
//   void icon_size(float2 value) { _checkbox_set(&slot::icon_size, value); }
//   void icon_color(const color& value) { _checkbox_set(&slot::icon_color, value); }
//   void on_toggle(function<void, bool> f) { _checkbox_set(&slot::on_toggle, std::move(f)); }

//   static std::expected<checkbox, error_trace> add(window& w, float2 Pos, float2 Size, stringable<wchar_t> auto&& label_text = L"") {
//     if (auto res = dwrite.initialize(); !res) return unexpected_error(res.error());
//     if (auto res = initialize_default_icons(); !res) return unexpected_error(res.error());

//     if (auto result = _add<checkbox>(w, Pos, Size); result) {
//       if (const auto slot = result->_checkbox_slot()) {
//         // Set up box icon
//         slot->box_icon.image(default_box_icon);
//         slot->box_icon.image_color(colors::black);

//         // Set up mark icon
//         slot->mark_icon.image(default_mark_icon);
//         slot->mark_icon.image_color(colors::black);

//         // Set up label
//         slot->label_text.text(std::wstring_view(label_text));
//       }
//       return result;
//     } else {
//       return unexpected_error(result.error());
//     }
//   }
// };

// } // namespace yw::ui
