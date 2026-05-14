// #pragma once
// #include "ywx/ui_parts.h"

// namespace yw::ui::part {

// struct window {
//   HWND hwnd{};
//   int4 frame_thickness{};
//   int2 pos{};
//   int2 size{};
//   DWORD style{};
//   DWORD exstyle{};
//   std::wstring title{};
//   bitmap rendertarget{};
//   comptr<IDXGISwapChain1> swapchain{};
//   bool dirty = true, messy = true, manually_drawn = false;
//   bool visible = false, active = false;

//   /// closes window
//   std::expected<void, error_trace> close() {
//     if (!hwnd) return {};
//     ::DestroyWindow(hwnd);
//     hwnd = nullptr;
//     return {};
//   }

//   /// shows window with specified size at specified position
//   std::expected<void, error_trace> show(int2 Pos, uint2 Size) {
//     if (size == Size) goto skip_resize;
//     pos = Pos, size = Size;

//   skip_resize:
//   }

//   /// hides window
//   std::expected<void, error_trace> hide();
// };
// } // namespace yw::ui::part
