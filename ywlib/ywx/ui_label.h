// #pragma once
// #include "ywx/text_layout.h"
// #include "ywx/ui_plain.h"

// namespace yw::ui {

// class label : public plain {
// public:
//   class part {
//     mutable float2 _size{};
//     std::wstring _text{};
//     yw::color _color = colors::black;
//     mutable text_layout _layout{};
//     mutable bool _need_update_layout = true;

//     std::expected<void, error_trace> _update_layout() const {
//       IDWriteTextFormat* tfp = _layout ? static_cast<IDWriteTextFormat*>(_layout) : dwrite.text_format();
//       if (auto tl = text_layout::create(_text, tfp, float2(1e6f, 1e6f))) {
//         if (auto ms = tl->metrics_size()) _size = *ms;
//         const auto tlp = static_cast<IDWriteTextLayout*>(*tl);
//         if (!tlp) return unexpected_error(errors::operation_failed, "Failed to create text layout");
//         tlp->SetMaxWidth(_size.x);
//         tlp->SetMaxHeight(_size.y);
//         _layout = std::move(*tl);
//       } else return unexpected_error(tl.error());
//       _need_update_layout = false;
//       return {};
//     }

//   public:
//     const auto& size() const {
//       if (_need_update_layout) _update_layout();
//       return _size;
//     }
//     const auto& text() const { return _text; }
//     const auto& color() const { return _color; }
//     const auto& layout() const {
//       if (_need_update_layout) _update_layout();
//       return _layout;
//     }

//     template<typename T> void size(T&&) { always_false<T>; }
//     template<stringable S> void text(S&& Value) {
//       _need_update_layout = true, _text = unicode<wchar_t>(static_cast<S&&>(Value));
//     }
//     void color(const yw::color& Value) { _color = Value; }

//     auto& text() { return _need_update_layout = true, _text; }
//     auto& layout() { return _need_update_layout = true, _layout; }

//     std::expected<void, error_trace> layout(text_format_like auto&& text_format) {
//       if (auto tl = text_layout::create(_text, text_format, float2(1e6f, 1e6f))) {
//         if (auto res = tl->shrink_to_fit(); !res) return unexpected_error(res.error());
//         _layout = std::move(*tl);
//         _need_update_layout = false;
//         return {};
//       } else return unexpected_error(tl.error());
//     }

//     std::expected<void, error_trace> draw(float2 Pos) const {
//       if (_need_update_layout)
//         if (auto res = _update_layout(); !res) return unexpected_error(res.error());
//       brush.color(_color);
//       if (auto res = draw_text(Pos, _layout); !res) return unexpected_error(res.error());
//       return {};
//     }
//   };

//   class slot : public plain::slot {
//   public:
//     mutable part text;

//     /// left, top, right, bottom paddings. `unconstrained` maximizes the available space
//     float4 padding{4.0f, 4.0f, 4.0f, 4.0f};

//     virtual tuple<float2, uint2> require_size() const noexcept {
//       const auto ts = text.size();
//       tuple<float2, uint2> result;
//       result.first.x = yw::max(size.x, ts.x + yw::max(padding.x, 0.0f) + yw::max(padding.z, 0.0f));
//       result.first.y = yw::max(size.y, ts.y + yw::max(padding.y, 0.0f) + yw::max(padding.w, 0.0f));
//       result.second.x = size.x < 0.0f && (padding.x < 0.0f || padding.z < 0.0f);
//       result.second.y = size.y < 0.0f && (padding.y < 0.0f || padding.w < 0.0f);
//       return result;
//     }

//     virtual void draw(float2 Pos, float2 Size) const override {
//       update_last_rect(Pos, Size);
//       draw();
//     }

//     virtual void draw() const override {
//       plain::slot::draw();
//       const auto lrsz = last_rect.zw() - last_rect.xy();
//       const auto tsz = text.size();
//       float2 offset = lrsz - tsz;
//       if (padding.x < 0.0f) {
//         if (padding.z < 0.0f) offset.x *= 0.5f; // 中央寄せ
//         else offset.x -= padding.z;             // 右寄せ
//       } else {
//         if (padding.z < 0.0f) offset.x = padding.x;                // 左寄せ
//         else offset.x = (offset.x - padding.x - padding.z) * 0.5f; // 中央寄せ
//       }
//       if (padding.y < 0.0f) {
//         if (padding.w < 0.0f) offset.y *= 0.5f;
//         else offset.y -= padding.w;
//       } else {
//         if (padding.w < 0.0f) offset.y = padding.y;
//         else offset.y = (offset.y - padding.y - padding.w) * 0.5f;
//       }
//       text.draw(last_rect.xy() + offset);
//     }
//   };

//   using plain::operator bool;
//   label() noexcept = default;

//   label(derived_from<unknown> auto& Layout) {
//     if (auto res = create_control<label>(Layout)) _id = *res;
//   }

//   void size(float2) {}

//   auto& text() { return unsafe_get(&slot::text); }
//   const auto& text() const { return unsafe_get(&slot::text); }

//   auto& padding() { return unsafe_get(&slot::padding); }
//   const auto& padding() const { return unsafe_get(&slot::padding); }
// };
// } // namespace yw::ui
