// #pragma once
// #include "ywx/window.h"

// namespace yw::control {

// //////////////////////////////////////// MARK: label

// class label : public base {
// public:
//   class slot : public control::slot {
//   protected:
//     std::wstring _text;
//     yw::text_layout _text_layout;
//   public:
//     color text_color = colors::black;

//     virtual std::expected<void, error_trace> draw() const override {
//       if (auto res = control::slot::draw(); !res) return unexpected_error(res.error());
//       if (!_text_layout) return unexpected_error(errors::not_initialized, "text_layout is not initialized");
//       if (auto res = draw_text(position + padding, _text_layout, text_color); !res)
//         return unexpected_error(res.error());
//       return {};
//     }

//     const std::wstring& text() const noexcept { return _text; }
//     yw::text_layout& text_layout() noexcept { return _text_layout; }
//     const yw::text_layout& text_layout() const noexcept { return _text_layout; }

//     void text(stringable<wchar_t> auto&& Text) {
//       _text.assign(std::wstring_view(Text));
//       if (auto res = yw::text_layout::create(_text, _text_layout); res) _text_layout = std::move(res.value());
//       else _text_layout = {};
//     }

//     void text_layout(text_format_like auto&& text_format, float2 size) {
//       if (auto res = yw::text_layout::create(_text, (IDWriteTextFormat*)text_format, size); !res) _text_layout = {};
//       else _text_layout = std::move(res.value());
//     }

//     void text_layout(const yw::text_layout& tl) {
//       if (auto res = yw::text_layout::create(_text, tl); !res) _text_layout = {};
//       else _text_layout = std::move(res.value());
//     }
//   };

// protected:
//   slot* _label() const noexcept { return dynamic_cast<slot*>(_control()); }

// public:
//   using slot_type = slot;
//   using control::control;
//   using control::operator bool;

//   slot* operator->() const noexcept { return _label(); }

//   static std::expected<label, error_trace> add(is_window auto& window) { return _add<label, slot>(window); }
// };
// }

// //////////////////////////////////////// MARK: button

// namespace yw::controls {


// }
