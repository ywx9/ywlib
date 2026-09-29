// #pragma once
// #include <apps/bitmap.h>
// #include <apps/control.h>
// #include <apps/text.h>

// #ifdef _WIN32

// namespace yw::ui {

// ///--------------------------------------------------------------------------///
// /// MARK: label

// class label : public control {
// public:
//   struct slot : control::slot {
//     yw::text text;
//     float4 padding = float4::fill(arbitrary_value);
//     optional<color> background_color;
//     optional<color> border_color;
//     optional<color> text_color;
//     float border_thickness = 1.0f;
//     alignment text_alignment = alignment::center;

//     virtual result<float2> get_content_size() const override { return text.size() + padding.xy() + padding.zw(); }

//     virtual result<void> draw_background(interface::slot* wsp) const override {
//       if (!wsp) return error(errors::invalid_operation, "wsp is null");

//       if (auto res = fill_geometry(geometry, background_color.value_or()); !res) return res.relay();
//     }
//   };
// };
// } // namespace yw

// #endif
