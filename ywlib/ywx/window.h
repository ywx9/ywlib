#pragma once

/// \note window関係の定義は依存関係が複雑なため、特別に別ヘッダに分けた

#include "ywx/window/control_base.h"
#include "ywx/window/core.h"
#include "ywx/window/master.h"
#include "ywx/window/proc.h"
#include "ywx/window/slave.h"
#include "ywx/window/slot.h"
#include "ywx/window/system.h"

namespace yw {

inline window::slot* window::slotid::get_window() const noexcept {
  if (const auto ms = system.windows.get(master); !ms) return nullptr;
  else return slave.is_zero() ? ms : ms->slaves.get(slave);
}

inline window::control_slot* window::control_slotid::get_control() const noexcept {
  const auto window_slot = get_window();
  return window_slot ? window_slot->controls.get(control) : nullptr;
}

//////////////////////////////////////// MARK: window/open.h

inline std::expected<window::master, error_trace> decltype(window::open)::operator()(
  int2 Pos, int2 Size, const null_terminated<wchar_t>& Title, window::style Style = style::regular, bool Show = true) {
  if (auto res = window::system.initialize(); !res) return unexpected_error(res.error());
  switch (Style) {
  case window::style::regular:
  case window::style::fixed:
  case window::style::borderless: break;
  default: return unexpected_error(errors::invalid_argument, "invalid window style");
  }
  auto master_slot = std::make_unique<window::slot>();
  if (auto res = master_slot->_create_window(Title.data(), Style); !res) return unexpected_error(res.error());
  if (auto res = master_slot->_set_possize(Pos, Size); !res) return unexpected_error(res.error());
  const auto master_id = window::system.windows.push(std::move(master_slot));
  return window::master({master_id, {}}, Show);
}

inline std::expected<window::master, error_trace> decltype(window::open)::operator()(
  int2 Size, const null_terminated<wchar_t>& Title, window::style Style = style::regular, bool Show = true) {
  if (auto res = window::system.initialize(); !res) return unexpected_error(res.error());
  switch (Style) {
  case window::style::regular:
  case window::style::fixed:
  case window::style::borderless: break;
  default: return unexpected_error(errors::invalid_argument, "invalid window style");
  }
  auto master_slot = std::make_unique<window::slot>();
  if (auto res = master_slot->_create_window(Title.data(), Style); !res) return unexpected_error(res.error());
  if (auto res = master_slot->_set_possize(Size); !res) return unexpected_error(res.error());
  const auto master_id = window::system.windows.push(std::move(master_slot));
  return window::master({master_id, {}}, Show);
}

inline std::expected<window::slave, error_trace> decltype(window::open)::subwindow(window::master& mw, int2 Offset,
  int2 Size, const null_terminated<wchar_t>& Title, window::style Style = style::unknown, bool Show = true) {
  const auto master_slot = window::system.windows.get(mw._id.master);
  if (!master_slot) return unexpected_error(errors::invalid_argument, "invalid master window");
  switch (Style) {
  case window::style::unknown: Style = master_slot->style; break;
  case window::style::regular:
  case window::style::fixed:
  case window::style::borderless: break;
  default: return unexpected_error(errors::invalid_argument, "invalid window style");
  }
  auto slave_slot = std::make_unique<window::slot>();
  if (auto res = slave_slot->_create_window(Title.data(), Style); !res) return unexpected_error(res.error());
  if (auto res = slave_slot->_set_possize(mw.position() + Offset, Size); !res) return unexpected_error(res.error());
  const auto slave_id = master_slot->slaves.push(std::move(slave_slot));
  return window::slave({mw._id.master, slave_id}, Show);
}

//////////////////////////////////////// MARK: window/system.h
inline window::slot* decltype(window::system)::get_window(const window::slave& w) const noexcept {
  if (const auto master_slot = windows.get(w._id.master); !master_slot) return nullptr;
  else return w._id.slave.is_zero() ? master_slot : master_slot->slaves.get(w._id.slave);
}

//////////////////////////////////////// MARK: mainloop

inline bool mainloop() {
  static stopwatch frame_timer = [] {
    stopwatch t;
    t.start();
    return t;
  }();
  ++window::system.frame_count;
  uint32_t message_count = 0;
  for (MSG msg; ::PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE);) {
    if (msg.message == WM_QUIT) return false;
    ::TranslateMessage(&msg), ::DispatchMessageW(&msg);
    if (window::system.last_error) return false;
    if (++message_count > window::system.max_messages_per_frame) break;
  }
  // 負荷軽減のため、前回の描画から十分な時間が経過していない場合は描画をスキップする。
  if (frame_timer.elapsed() < 1.0 / window::system.max_frames_per_second) return true;
  frame_timer.restart();
  // 各ウィンドウについてコントロールを描画して更新する。
  // 描画の失敗は扱わないが、error_traceのデストラクタによって自動でエラー出力される。
  for (auto& master_slot : window::system.windows) {
    if (auto d = master_slot.rendertarget.begin_draw())
      for (auto& control : master_slot.controls)
        if (control.visible) control.draw();
    if (master_slot.swapchain) master_slot.swapchain->Present(0, 0);
    for (auto& slave_slot : master_slot.slaves) {
      if (window::system.last_error) return false;
      if (auto d = slave_slot.rendertarget.begin_draw())
        for (auto& control : slave_slot.controls)
          if (control.visible) control.draw();
      if (slave_slot.swapchain) slave_slot.swapchain->Present(0, 0);
    }
  }
  return !window::system.last_error;
}
} // namespace yw
