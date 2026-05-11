#pragma once
#include "ywx/command_manager.h"
#include "ywx/ui_control.h"
#include "ywx/tooltip.h"

namespace yw {

class window : public ui::unknown {
public:
  enum class style : uint32_t {
    unknown,
    regular = WS_OVERLAPPEDWINDOW,
    fixed = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
    borderless = WS_POPUP
  };

  struct slot : public ui::unknown::slot {
    ui::part::window core;

    ui::part::background background;
    ui::part::focus_ring focus_ring;

    bitmap ui_bitmap{};
    ui::slotid layout_id{};
    bool resizing = false;

    ui::slotid focused_control{};
    ui::slotid hovered_control{};
    ui::slotid captured_control{};
    bool tracking = false;

    function<bool> on_close;
    function<void, event::key> on_keydown;
    function<void, event::key> on_keyup;

    command_manager commands;

    std::expected<void, error_trace> initialize(
      ui::slotid Id, std::optional<int2> Pos, std::optional<int2> Size, //
      std::optional<std::wstring> Title, window::style Style) {
      { //-- initialize core --//
        switch (Style) {
        case style::regular:
        case style::fixed:
        case style::borderless: break;
        default: return unexpected_error(errors::invalid_argument, "Invalid window style");
        }
        core.window_id = Id;
        core.style = DWORD(Style);
        core.ex_style = WS_EX_ACCEPTFILES;
        if (auto res = core.initialize(); !res) return unexpected_error(res.error());
        auto&& h = core.handle();
        if (Size.has_value()) {
          h.size(*Size);
          if (Pos.has_value()) h.pos(*Pos);
          else h.centering();
        } else if (Pos.has_value()) h.pos(*Pos);
        if (!Title) {
          std::wstring t(MAX_PATH, L'\0');
          if (const auto n = ::GetModuleFileNameW(nullptr, t.data(), MAX_PATH); n > 0) t.resize(n);
          else return unexpected_error(errors::operation_failed, "GetModuleFileNameW failed");
          h.title(std::move(t));
        } else h.title(std::move(*Title));
      }
      background.window_id = Id;
      focus_ring.window_id = Id;
      return {};
    }

    std::expected<void, error_trace> update_ui() {
      const auto lsp = system::slot_address<ui::control>(layout_id);
      if (!lsp) return unexpected_error(errors::operation_failed, "Missing layout");
      if (core.messy) { // messyだけの処理
        lsp->ensure_minimum_size();
        lsp->update_layout({}, lsp->core.size);
        if (core.size.x < lsp->core.size.x || core.size.y < lsp->core.size.y) {
          core.size = vapply_r<int2>(yw::max, core.size, lsp->core.size);
          ::SetWindowPos(core.hwnd, nullptr, 0, 0, core.size.x, core.size.y, SWP_NOZORDER | SWP_NOMOVE);
        }
      } else if (!core.dirty) return {};
      // 以降 messy と dirty の共通処理
      if (!ui_bitmap || ui_bitmap.size() != core.size) {
        if (auto res = bitmap::create(core.size)) ui_bitmap = std::move(*res);
        else return unexpected_error(res.error());
      }
      if (auto d = ui_bitmap.begin_draw(colors::transparent)) lsp->draw();
      else return unexpected_error(d.error());
      core.messy = core.dirty = false;
      return {};
    }

    std::expected<void, error_trace> draw() {
      core.resize(core.size);
      auto d = core.rendertarget.begin_draw();
      if (!d) return unexpected_error(d.error());
      if (!core.manually_drawn) {
        brush.color(background.color);
        fill_rectangle({}, core.size);
        if (background.image) draw_bitmap({}, core.size, background.image, background.image_opacity);
      }
      draw_bitmap({}, ui_bitmap);
      core.manually_drawn = false;
      return {};
    }

    virtual bool attach_child(ui::slotid Child) override {
      if (const auto lsp = system::slot_address<ui::control>(layout_id)) return lsp->attach_child(Child);
      return true;
    }

    virtual void detach_child(ui::slotid Child) override {
      if (const auto lsp = system::slot_address<ui::control>(layout_id)) lsp->detach_child(Child);
    }

    void next_tab_stop(bool Forward) {
      if (const auto lsp = system::slot_address<ui::control>(layout_id)) {
        bool found = !focused_control;
        focused_control = lsp->next_tab_stop(focused_control, Forward, found);
      }
    }
  };

  virtual ~window() noexcept override { destroy(); }
  explicit window() noexcept = default;
  window(window&& other) noexcept {
    if (this == &other) return;
    destroy();
    ui::unknown::_id = std::exchange(other._id, {});
  }
  window& operator=(window&& other) noexcept {
    if (this == &other) return *this;
    destroy();
    ui::unknown::_id = std::exchange(other._id, {});
    return *this;
  }

private:
  static std::expected<window, error_trace> _open(
    std::optional<int2> Pos, std::optional<int2> Size, std::optional<std::wstring> Title, style Style, bool Show) {
    window w{};
    const auto id = system::uis.add(std::make_unique<slot>());
    const auto wsp = system::slot_address<window>(id);
    if (!wsp) return unexpected_error(errors::operation_failed, "Failed to create window slot");
    if (auto res = wsp->initialize(id, Pos, Size, std::move(Title), Style)) return unexpected_error(res.error());
    w._id = id;
    return w;
  }

public:
  static std::expected<window, error_trace> open(
    int2 Pos, uint2 Size, std::wstring Title, style Style = style::regular, bool Show = true) {
    if (auto res = _open(Pos, int2(Size), std::move(Title), Style, Show)) return std::move(*res);
    else return unexpected_error(res.error());
  }

  static std::expected<window, error_trace> open(int2 Pos, uint2 Size, style Style = style::regular, bool Show = true) {
    if (auto res = _open(Pos, int2(Size), std::nullopt, Style, Show)) return std::move(*res);
    else return unexpected_error(res.error());
  }

  /// \note 位置は指定するのにサイズを指定しないケースは想定しない

  static std::expected<window, error_trace> open(
    uint2 Size, std::wstring Title, style Style = style::regular, bool Show = true) {
    if (auto res = _open(std::nullopt, int2(Size), std::move(Title), Style, Show)) return std::move(*res);
    else return unexpected_error(res.error());
  }

  static std::expected<window, error_trace> open(uint2 Size, style Style = style::regular, bool Show = true) {
    if (auto res = _open(std::nullopt, int2(Size), std::nullopt, Style, Show)) return std::move(*res);
    else return unexpected_error(res.error());
  }

  static std::expected<window, error_trace> open(std::wstring Title, style Style = style::regular, bool Show = true) {
    if (auto res = _open(std::nullopt, std::nullopt, std::move(Title), Style, Show)) return std::move(*res);
    else return unexpected_error(res.error());
  }

  static std::expected<window, error_trace> open(style Style = style::regular, bool Show = true) {
    if (auto res = _open(std::nullopt, std::nullopt, std::nullopt, Style, Show)) return std::move(*res);
    else return unexpected_error(res.error());
  }

  explicit operator bool() const noexcept;
  const ui::slotid& id() const noexcept { return _id; }

  auto core() {
    if (auto wsp = system::slot_address<window>(_id); !wsp)
      fatal_error(errors::operation_failed, "Failed to access window slot.");
    else return wsp->core.handle();
  }
  const auto core() const {
    if (auto wsp = system::slot_address<window>(_id); !wsp)
      fatal_error(errors::operation_failed, "Failed to access window slot.");
    else return wsp->core.handle();
  }

  auto background() {
    const auto wsp = system::slot_address<window>(_id);
    if (!wsp) fatal_error(errors::operation_failed, "Failed to access window slot.");
    return wsp->background.handle();
  }
  const auto background() const {
    const auto wsp = system::slot_address<window>(_id);
    if (!wsp) fatal_error(errors::operation_failed, "Failed to access window slot.");
    return wsp->background.handle();
  }

  const auto& on_close() const { return unsafe_get(&slot::on_close); }
  std::expected<void, error_trace> on_close(function<bool> OnClose) const {
    if (auto wsp = system::slot_address<window>(_id)) {
      wsp->on_close = std::move(OnClose);
      return {};
    } else return unexpected_error(errors::operation_failed, "Failed to access window slot.");
  }

  const auto& on_keydown() const { return unsafe_get(&slot::on_keydown); }
  std::expected<void, error_trace> on_keydown(function<void, event::key> OnKeyDown) const {
    if (auto wsp = system::slot_address<window>(_id)) {
      wsp->on_keydown = std::move(OnKeyDown);
      return {};
    } else return unexpected_error(errors::operation_failed, "Failed to access window slot.");
  }

  const auto& on_keyup() const { return unsafe_get(&slot::on_keyup); }
  std::expected<void, error_trace> on_keyup(function<void, event::key> OnKeyUp) const {
    if (auto wsp = system::slot_address<window>(_id)) {
      wsp->on_keyup = std::move(OnKeyUp);
      return {};
    } else return unexpected_error(errors::operation_failed, "Failed to access window slot.");
  }

  const auto& commands() const { return unsafe_get(&slot::commands); }
  auto& commands() { return unsafe_get(&slot::commands); }

  std::expected<drawing, error_trace> begin_draw() {
    if (const auto wsp = system::slot_address<window>(_id)) {
      wsp->core.dirty = true;
      wsp->core.manually_drawn = true;
      if (auto d = wsp->core.rendertarget.begin_draw(wsp->background.color)) {
        if (wsp->background.image)
          draw_bitmap({}, wsp->core.size, wsp->background.image, wsp->background.image_opacity);
        return std::move(d);
      } else return unexpected_error(d.error());
    } else return unexpected_error(errors::operation_failed, "Failed to access window slot.");
  }

  std::expected<void, error_trace> destroy() noexcept {
    if (const auto wsp = system::slot_address<window>(_id)) ::DestroyWindow(wsp->core.hwnd);
    return {};
  }

  std::expected<void, error_trace> screenshot(const std::filesystem::path& PngPath, bool WriteUI = false) {
    if (const auto wsp = system::slot_address<window>(_id)) {
      if (!WriteUI) {
        if (auto res = wsp->core.rendertarget.save_as_png(PngPath); !res) return unexpected_error(res.error());
        return {};
      }
      if ((wsp->core.dirty || wsp->core.messy) && wsp->layout_id)
        if (auto res = wsp->update_ui(); !res) return unexpected_error(res.error());

      auto rt_copy_res = bitmap::create(wsp->core.rendertarget);
      if (!rt_copy_res) return unexpected_error(rt_copy_res.error());
      auto rt_copy = std::move(*rt_copy_res);

      if (auto res = bitmap::create(wsp->core.size)) {
        auto composed = std::move(*res);
        if (auto d = composed.begin_draw(color(0.0f, 0.0f, 0.0f, 0.0f))) {
          draw_bitmap({}, float2(wsp->core.size), rt_copy);
          draw_bitmap({}, float2(wsp->core.size), wsp->ui_bitmap);
        } else return unexpected_error(d.error());
        if (auto res = composed.save_as_png(PngPath); !res) return unexpected_error(res.error());
      } else return unexpected_error(res.error());
      return {};
    } else return unexpected_error(errors::operation_failed, "Failed to access window slot.");
  }
};

inline void ui::control::slot::make_dirty() const noexcept {
  if (const auto wsp = system::slot_address<window>(core.window_id)) wsp->core.dirty = true;
}

inline void ui::control::slot::make_messy() const noexcept {
  if (const auto wsp = system::slot_address<window>(core.window_id)) wsp->core.messy = true;
}

inline void ui::control::slot::hover_event(event::hover Event) {
  if (enabled && on_hover) on_hover(Event);
  if (tooltip.empty()) return;
  if (Event.enter()) {
    if (const auto w = system::slot_address<window>(core.window_id))
      system::tooltip.show(core.pos + w->core.pos + w->core.margin.xy(), core.size, tooltip);
  } else if (Event.leave()) system::tooltip.hide();
}
} // namespace yw
