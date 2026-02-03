// #pragma once
// #include "ywx/window.h"

// namespace yw {

// //////////////////////////////////////// MARK: label

// class label : public control {
//   std::wstring _text;
//   text_layout _text_layout;

//   label(window& owner) noexcept : control(owner) {}

//   template<stringable S> std::expected<void, error_trace> _set_text(S&& text) {
//     if constexpr (same_as<iter_value_t<S>, wchar_t>) _text = std::wstring(std::wstring_view(text));
//     else _text = unicode<wchar_t>(static_cast<S&&>(text));
//     if (!_text_layout) return unexpected_error(errors::not_initialized, "text_layout is not initialized");
//     if (auto tl = text_layout::create(_text, _text_layout); !tl) return unexpected_error(tl.error());
//     else _text_layout = std::move(*tl);
//     return {};
//   }
//   template<castable_to<IDWriteTextFormat*> T> std::expected<void, error_trace> _set_text_format(T&& text_format) {
//     IDWriteTextFormat* tf = static_cast<IDWriteTextFormat*>(text_format);
//     if (!tf) return unexpected_error(errors::not_initialized, "text_format is not initialized");
//     float2 size = {this->size.x - padding.x * 2.0f, this->size.y - padding.y * 2.0f};
//     if (auto tl_new = text_layout::create(_text, tf, size); !tl_new) return unexpected_error(tl_new.error());
//     else _text_layout = std::move(*tl_new);
//     return {};
//   }
// public:
//   color text_color = colors::black;

//   explicit operator bool() const noexcept { return static_cast<bool>(_text_layout); }

//   /// \note this function is called within mainloop
//   /// \note d2d.transform may be modified after this call
//   std::expected<void, error_trace> draw() override {
//     if (auto res = dwrite.initialize(); !res) return unexpected_error(res.error());
//     if (!drawing::d2d_drawing()) return unexpected_error(errors::invalid_operation, "drawing not begun");
//     if (!_text_layout) return {}; // nothing to draw
//     const float2 layout_size{size.x - padding.x * 2.0f, size.y - padding.y * 2.0f};
//     if (auto size = _text_layout.layout_size(); !size) return unexpected_error(size.error());
//     else if (size != layout_size) _set_text_format(_text_layout); // update layout size
//     d2d.context()->SetTransform(D2D1::Matrix3x2F::Translation(position.x, position.y));
//     fill_round_rectangle({}, size, rounded_radius, background_color);
//     draw_round_rectangle({}, size, rounded_radius, border_color, border_width);
//     draw_text_layout(padding, _text_layout, text_color);
//     return {};
//   }



//   template<castable_to<IDWriteTextFormat*> T>
//   [[nodiscard]] static std::expected<label, error_trace> create(
//     window& owner, float2 position, float2 size, stringable<wchar_t> auto&& text, T&& text_format) {
//     label lbl(owner);
//     lbl.position = position;
//     lbl.size = size;
//     if constexpr (same_as<decltype(text)&&, std::wstring&&>) lbl._text = std::move(text);
//     else lbl._text = unicode<wchar_t>(std::basic_string_view<iter_value_t<decltype(text)>>(text));
//     if (auto res = lbl._set_text_format(static_cast<T&&>(text_format)); !res) return unexpected_error(res.error());
//     return std::move(lbl);
//   }
// };
// }
