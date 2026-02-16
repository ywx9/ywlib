// #pragma once
// #include "ywx/label.h"

// namespace yw::control {

// //////////////////////////////////////// MARK: button

// class button : public label {
// public:
//   class slot : public label::slot {
//   public:
//     uint64_t focused_frame = 0;
//     uint64_t down_frame = 0;
//     uint64_t up_frame = 0;

//     virtual std::expected<void, error_trace> draw() const override {
//       if (auto res = label::slot::draw(); !res) return unexpected_error(res.error());
//       return {};
//     }

//     virtual std::expected<bool, error_trace> proc(const MSG& msg) override;
//   };

// protected:
//   slot* _button() const noexcept { return dynamic_cast<slot*>(_control()); }

// public:
//   using slot_type = slot;
//   using base::base;
//   using base::operator bool;

//   slot* operator->() const noexcept { return _button(); }

//   bool is_pressed() const noexcept {
//     if (const auto b = _button()) {
//       const auto current_frame = window::system.frame_count;
//       return b->up_frame < b->down_frame && b->down_frame == current_frame;
//     } else return false;
//   }

//   bool is_down() const noexcept {
//     if (const auto b = _button()) {
//       const auto current_frame = window::system.frame_count;
//       return b->up_frame < b->down_frame && b->down_frame <= current_frame;
//     } else return false;
//   }

//   bool is_released() const noexcept {
//     if (const auto b = _button()) {
//       const auto current_frame = window::system.frame_count;
//       return b->up_frame < b->down_frame && b->up_frame == current_frame;
//     } else return false;
//   }

//   bool is_hover() const noexcept { return control::system.hovered_control == _counter; }

//   static std::expected<button, error_trace> add(window::slave& window, float2 pos, float2 size) {
//     if (auto result = _add<button, slot>(window, pos, size); result) {
//       auto& btn = result.value();
//       text_layout& tl = btn->text_layout;
//       tl.text_alignment(DWRITE_TEXT_ALIGNMENT_CENTER);
//       tl.paragraph_alignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
//       return result;
//     } else return unexpected_error(result.error());
//   }
// };

// //////////////////////////////////////// MARK: button_slot::proc

// inline std::expected<bool, error_trace> button::slot::proc(const MSG& msg) {
//   const auto w = _window();
//   if (!w) return unexpected_error(errors::invalid_operation, "window slot is not available");
//   switch (msg.message) {
//   case WM_LBUTTONDOWN:
//     if (hit_test(w->cursor_pos)) {
//       _last_down_frame = window::system.frame_count;
//       return true;
//     }
//     break;
//   case WM_LBUTTONUP:
//     if (hit_test(w->cursor_pos)) {
//       _last_up_frame = window::system.frame_count;
//       return true;
//     }
//     break;
//   }
//   return label::slot::proc(msg);
// }
// }
