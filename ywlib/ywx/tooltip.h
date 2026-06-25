// #pragma once
// #include "ywx/window.h"
// #include "ywx/ui_label.h"

// namespace yw::internal {
// /// shows/hides tooltip window
// /// \param Pos relative position of control to window's client area
// /// \param Size size of control
// std::expected<void, error> show_tooltip(float2 Pos, float2 Size, std::wstring Text) {
//   constexpr DWORD style = WS_POPUP;
//   constexpr DWORD exstyle = WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE;
//   constexpr float margin = ui::arbitrary_value; // avoid tooltip being too close to the control
//   static auto tooltip_window = [&] {
//     auto res = window::open<window::type::custom>({.style = style, .exstyle = exstyle, .is_primal_window = false});
//     if (!res) fatal_error(res.error());
//     res->background().color(colors::ivory);
//     return std::move(*res);
//   }();
//   static auto tooltip_label = [&] {
//     auto res = ui::label::add(tooltip_window);
//     if (!res) fatal_error(res.error());
//     res->core().margin({});
//     res->text().font_size(12.0f);
//     return std::move(*res);
//   }();
//   if (Text.empty()) {
//     if (auto res = tooltip_window.hide(); !res) return unexpected_error(res.error());
//     return {};
//   }
//   tooltip_label.text().string(std::move(Text));
//   if (auto res = tooltip_window.redraw(); !res) return unexpected_error(res.error());
//   const auto tooltip_size = tooltip_window.size();
//   const auto left_right = Pos + Size;
//   const auto desktop_size = desktop_client_size();
//   float2 origin{Pos.x, 0.0f};
//   if (Pos.y < desktop_size.y - left_right.y) origin.y = left_right.y + margin;
//   else origin.y = Pos.y - tooltip_size.y - margin;
//   if (const auto wsp = get_slot_pointer<window::handle<window::type::unknown>>(tooltip_window.id())) {
//     ::SetWindowPos(wsp->hwnd, nullptr, int(origin.x), int(origin.y), 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
//     ::ShowWindow(wsp->hwnd, SW_SHOW);
//     wsp->swapchain->Present(0, 0);
//   }
//   return {};
// }

// }
