#pragma once
#include "ywx/ui_parts.h"
#include "ywx/event.h"

namespace yw::ui {

class control : public unknown {
protected:
  template<derived_from<control> Ctrl>
  static std::expected<slotid, error_trace> create_control(derived_from<unknown> auto& Layout) {
    const auto lid = Layout.id();
    const auto lsp = system::slot_address<unknown::slot>(lid);
    if (!lsp) return unexpected_error(errors::operation_failed, "Failed to access layout slot");
    const auto cid = system::uis.add(std::make_unique<typename Ctrl::slot>());
    const auto csp = system::slot_address<Ctrl>(cid);
    if (!csp) return unexpected_error(errors::operation_failed, "Failed to create control slot");
    if (!lsp->attach(cid)) {
      system::uis.erase(cid);
      return unexpected_error(errors::operation_failed, "Failed to attach control slot to layout");
    } else return cid;
  }

  control() noexcept = default;
public:
  struct slot : public unknown::slot {
    part::core core;
    bool visible = true;
    bool enabled = true;
    bool dying = false;

    std::wstring tooltip{};
    function<void, event::hover> on_hover;

    float2 area() const noexcept { return core.area(); }
    void make_messy() const noexcept;
    void make_dirty() const noexcept;

    virtual bool focusable() { return false; }

    /// messy 時の前処理として各コントロールの size を必要最小サイズに設定する関数
    virtual void ensure_minimum_size() { core.size = core.minimum_size(); }

    /// messy 時の最終処理として各コントロールの size と pos を確定する関数
    /// \note 引数はレイアウトが提供する描画領域。この中に余白込みで配置する
    virtual void update_layout(float2 Pos, float2 Size) { core.update_layout(Pos, Size); }

    /// 設定済みの pos, size に従ってコントロールを描画する
    virtual void draw() {}

    /// TABキーによるフォーカス移動を処理する関数
    virtual slotid next_tab_stop(slotid Focused, bool Forward, bool& Found) {
      if (Focused == id) Found = true;
      else if (Found && visible && focusable()) return id;
      return {};
    }

    virtual float2 ime_position() const { return {}; };
    virtual void ime_insert_text(std::wstring_view) {}

    virtual void char_event(wchar_t c) {}
    virtual void click_event(event::button e) {}
    virtual void button_event(event::button e) {}
    virtual void drag_event(event::drag e) {}
    virtual bool focus_event(bool) { return false; }
    virtual void hover_event(event::hover Event);
    virtual bool key_event(event::key e) { return false; }
    virtual void move_event(event::move e) {}
    virtual void wheel_event(event::wheel e) {}
  };

  auto core() {
    const auto csp = system::slot_address<control>(_id);
    if (!csp) fatal_error(errors::invalid_operation, "Invalid slot address");
    return csp->core.handle();
  }

  const auto core() const {
    const auto csp = system::slot_address<control>(_id);
    if (!csp) fatal_error(errors::invalid_operation, "Invalid slot address");
    return csp->core.handle();
  }
};
}
