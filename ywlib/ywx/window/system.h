#pragma once
#include "ywx/window/core.h"

namespace yw::window {

inline struct {
  friend class slot;

  static LRESULT __stdcall proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
  bool _initialized{};

public:
  HINSTANCE hinstance{};
  std::wstring_view name{};

  slotlist<slot> windows{};
  error_trace last_error{};
  uint64_t frame_count{};
  uint32_t max_frames_per_second = 1000;  // FPS上限。それ以上の描画はスキップされる。
  uint32_t max_messages_per_frame = 1000; // 1フレームあたりの処理メッセージ数上限。それ以上は次フレームに持ち越し。

  slotid focused_window{};

  std::expected<void, error_trace> initialize() {
    if (_initialized) return {};
    hinstance = ::GetModuleHandleW(nullptr);
    name = L"ywlib_window_class";
    WNDCLASSW wc{};
    wc.lpfnWndProc = proc;
    wc.hInstance = hinstance;
    wc.hCursor = ::LoadCursorW(nullptr, IDC_ARROW);
    wc.lpszClassName = name.data();
    if (!::RegisterClassW(&wc)) return unexpected_win32_error("RegisterClassW failed");
    _initialized = true;
    return {};
  }

  slot* get_window(const slave& w) const noexcept;
} system{};
} // namespace yw::window
