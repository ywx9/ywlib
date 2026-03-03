// #pragma once
// #include "ywx/focusable_base.h"
// #include "ywx/label.h"

// namespace yw::ui {

// //////////////////////////////////////// MARK: ui::button

// class button : public focusable_base {
// public:
//   class slot : public focusable_base::slot {
//   public:
//     ui::label label;
//     bool pressed = false;
//     function<void> on_click;
//     function<void, bool> on_press;

//     void _set_pressed(bool value) {
//       pressed = value;
//       if (on_press) on_press(value);
//       if (const auto w = system::windows.get(window_id)) w->dirty = true;
//     }

//     virtual void focus_changed(bool focused) override {
//       focusable_label::slot::focus_changed(focused);
//       if (!focused && pressed) _set_pressed(false);
//     }

//     virtual std::expected<void, error_trace> proc(UINT msg, WPARAM wp, LPARAM lp) override {
//       switch (msg) {
//       case WM_LBUTTONDOWN:
//         if (!pressed) _set_pressed(true);
//         return {};
//       case WM_LBUTTONUP:
//         if (pressed) {
//           _set_pressed(false);
//           const auto pt = float2(std::bit_cast<short2>(static_cast<uint32_t>(lp & 0xFFFFFFFF)));
//           if (hit_test(pt) && on_click) on_click();
//         }
//         return {};
//       case WM_KEYDOWN:
//         if ((wp == VK_SPACE || wp == VK_RETURN) && !pressed) _set_pressed(true);
//         return {};
//       case WM_KEYUP:
//         if ((wp == VK_SPACE || wp == VK_RETURN) && pressed) {
//           _set_pressed(false);
//           if (on_click) on_click();
//         }
//         return {};
//       }
//       return {};
//     }
//   };

// protected:
//   slot* _button_slot() const noexcept { return dynamic_cast<slot*>(_ui_slot()); }

//   template<typename Mp, typename T> void _button_set(Mp mp, T&& value) {
//     if (const auto s = _button_slot()) {
//       s->*mp = static_cast<T&&>(value);
//       if (const auto w = system::windows.get(s->window_id)) w->dirty = true;
//     }
//   }

//   template<typename Mp> const auto* _button_get(Mp mp) const {
//     if (const auto s = _button_slot()) return &(s->*mp);
//     else return static_cast<const remove_cvref<decltype(s->*mp)>*>(nullptr);
//   }

// public:
//   using focusable_label::focusable_label;

//   const auto& pressed() const { return *_button_get(&slot::pressed); }
//   const auto& on_click() const { return *_button_get(&slot::on_click); }
//   const auto& on_press() const { return *_button_get(&slot::on_press); }

//   void on_click(function<void> f) { _button_set(&slot::on_click, std::move(f)); }
//   void on_press(function<void, bool> f) { _button_set(&slot::on_press, std::move(f)); }

//   static std::expected<button, error_trace> add(window& w, float2 Pos, float2 Size) {
//     if (auto res = dwrite.initialize(); !res) return unexpected_error(res.error());
//     return _add<button>(w, Pos, Size);
//   }
// };

// } // namespace yw::ui
