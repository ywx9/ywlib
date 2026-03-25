// #pragma once
// #include "ywx/ui_plain.h"
// #include "ywx/ui_label.h"

// namespace yw::ui {

// class button : public clickable_plain {
// public:
//   class slot : public clickable_plain::slot {
//   public:
//     mutable label::part text;

//     virtual void draw(float2 Pos, float2 Size) const override {
//       clickable_plain::slot::draw(Pos, Size);
//       text.size(last_rect.zw() - last_rect.xy());
//       text.draw(last_rect.xy());
//     }

//     virtual void draw() const override {
//       clickable_plain::slot::draw();
//       text.draw(last_rect.xy());
//     }
//   };

//   auto& text() { return unsafe_get(&slot::text); }
//   const auto& text() const { return unsafe_get(&slot::text); }

//   bool pressed() const {
//     if (auto csp = system::slot_address<slot>(_id)) {
//       const auto ck = csp->captured_key;
//       return ck == key::lbutton || ck == key::enter || ck == key::space;
//     } else return false;
//   }

//   using clickable_plain::operator bool;
//   button() noexcept = default;
//   button(derived_from<unknown> auto& Layout) {
//     if (auto res = create_control<button>(Layout)) _id = *res;
//   }
// };
// } // namespace yw::ui
