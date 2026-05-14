// #pragma once
// #include "ywx/bitmap.h"
// #include "ywx/uip_text.h"
// #include "ywx/uip_window.h"

// namespace yw::system {

// struct ime {
//   ui::part::window core;
//   ui::part::text composition;
//   uint32_t cursor_pos = 0;

//   struct ime_attr_run {
//     uint32_t begin, end;
//     uint8_t type;
//   };

//   std::vector<ime_attr_run> attrs;

//   std::expected<void, error_trace> initialize() {
//     core.style = WS_POPUP;
//     core.ex_style = WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE;
//     if (auto res = core.initialize(); !res) return unexpected_error(res.error());
//     if (auto res = composition.initialize(); !res) return unexpected_error(res.error());
//     return {};
//   }

//   std::expected<void, error_trace> update_attrs(const std::vector<uint8_t>& Attrs) {
//     attrs.clear();
//     if (Attrs.empty()) {
//       attrs.clear();
//       return {};
//     }
//     attrs.reserve(Attrs.size());
//     uint8_t a = Attrs[0];
//     for (uint32_t i = 0, j = 1; true; ++j) {
//       if (j == Attrs.size()) {
//         attrs.push_back({i, j, a});
//         break;
//       }
//       if (a != Attrs[j]) {
//         attrs.push_back({i, j, a});
//         a = Attrs[j];
//         i = j;
//       }
//     }
//     return {};
//   }

//   std::expected<void, error_trace> update_window_size() {
//     constexpr int2 margin{5, 0}; // キャレットを右端に描画用するための余裕
//     if (auto res = initialize(); !res) return unexpected_error(res.error());
//     const auto window_size = int2(composition.layout_size()) + margin;
//     if (window_size == core.size || window_size == margin) return {};
//     if (auto res = core.resize(window_size); !res) return unexpected_error(res.error());
//     return {};
//   }

//   std::expected<void, error_trace> draw() {
//     if (auto res = initialize(); !res) return unexpected_error(res.error());
//     if (auto d = core.rendertarget.begin_draw(colors::whitesmoke)) {
//       brush.color(colors::black);
//       for (const auto& attr : attrs) {
//         auto res = composition.hittest_range({attr.begin, attr.end});
//         if (!res) return unexpected_error(res.error());
//         if (res->empty()) continue;
//         const auto pt = res->operator[](0).xy();
//         const auto sz = res->operator[](0).zw();
//         switch (attr.type) {
//         case ATTR_TARGET_CONVERTED:
//         case ATTR_TARGET_NOTCONVERTED:
//           brush.color(color(0.5f, 0.5f, 1.0f, 0.5f));
//           fill_rectangle(pt, sz);
//           brush.color(colors::black);
//           draw_line({pt.x, sz.y}, {pt.x + sz.x, sz.y}, 4.0f);
//           break;
//         case ATTR_CONVERTED: draw_line({pt.x, sz.y}, {pt.x + sz.x, sz.y}, 2.0f); break;
//         default:
//           brush.dashed(true);
//           draw_line({pt.x, sz.y}, {pt.x + sz.x, sz.y}, 2.0f);
//           brush.dashed(false);
//         }
//       }
//       brush.color(colors::black);
//       if (auto res = composition.draw(); !res) return unexpected_error(res.error());
//       if (auto ht = composition.hittest(cursor_pos); !ht) return unexpected_error(ht.error());
//       else if (auto res = draw_line(ht->xy(), ht->xw()); !res) return unexpected_error(res.error());
//     }
//     core.swapchain->Present(0, 0);
//     return {};
//   }

//   std::expected<void, error_trace> show(int2 GlobalPos) {
//     if (auto res = initialize(); !res) return unexpected_error(res.error());
//     if (auto res = core.locate(GlobalPos); !res) return unexpected_error(res.error());
//     core.show();
//     return {};
//   }
// } ime;
// } // namespace yw::system
