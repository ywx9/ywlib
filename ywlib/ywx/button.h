// #pragma once
// #include "ywx/label.h"

// namespace yw::controls {

// //////////////////////////////////////// MARK: button

// class button : public label {
// public:
//   class slot : public label::slot {
//   protected:
//     uint64_t _focused_frame = 0;
//     uint64_t _last_down_frame = 0;
//     uint64_t _last_up_frame = 0;

//   public:
//     bool is_pressed() const noexcept {
//       const auto current_frame = window_class.frame_count;
//       return _last_up_frame < _last_down_frame && _last_down_frame == current_frame;
//     }

//     bool is_down() const noexcept {
//       const auto current_frame = window_class.frame_count;
//       return _last_up_frame < _last_down_frame && _last_down_frame <= current_frame;
//     }

//     bool is_released() const noexcept {
//       const auto current_frame = window_class.frame_count;
//       return _last_down_frame < _last_up_frame && _last_up_frame == current_frame;
//     }

//     bool is_hover() const noexcept {
//       const auto w = _window();
//       return w ? hit_test(w->cursor_pos) : false;
//     }

//     virtual std::expected<bool, error_trace> proc(const MSG& msg) override;
//   };

// protected:
//   slot* _button() const noexcept { return dynamic_cast<slot*>(_control()); }

// public:
//   using slot_type = slot;
//   using control::control;
//   using control::operator bool;

//   slot* operator->() const noexcept { return _button(); }

//   static std::expected<button, error_trace> add(is_window auto& window) {
//     if (auto result = _add<button, slot>(window); result) {
//       auto& btn = result.value();
//       text_layout& tl = btn->text_layout();
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
//       _last_down_frame = window_class.frame_count;
//       return true;
//     }
//     break;
//   case WM_LBUTTONUP:
//     if (hit_test(w->cursor_pos)) {
//       _last_up_frame = window_class.frame_count;
//       return true;
//     }
//     break;
//   }
//   return label::slot::proc(msg);
// }
// }
