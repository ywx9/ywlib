#pragma once
#include "ywx/event.h"
#include "ywx/ui_unknown.h"

namespace yw::ui {

class control : public unknown {
protected:
  control() noexcept = default;

  template<typename Mp, typename T> void safe_set(Mp Member, T&& Value) const noexcept {
    if (auto csp = system::slot_address<class_type<Mp>>(_id)) {
      csp->*Member = static_cast<T&&>(Value);
      csp->make_dirty();
    }
  }

  template<typename Mp, typename T> void safe_set_size(Mp Member, T&& Value) const noexcept {
    if (auto csp = system::slot_address<class_type<Mp>>(_id)) {
      csp->*Member = static_cast<T&&>(Value);
      csp->make_messy();
    }
  }

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

public:
  class slot : public unknown::slot {
  public:
    slotid layout_id{}, window_id{};
    float4 margin{5.0f, 5.0f, 5.0f, 5.0f};
    float2 pos{};
    union {
      float2 size{};
      struct {
        float width, height;
      };
    };
    float2 radius{5.0f, 5.0f};
    float2 minimum_size{10.0f, 10.0f};
    uint2 ucc{1, 1};
    bool visible = true, enabled = true, dying = false;

    std::wstring tooltip{};
    function<void, event::hover> on_hover;

    virtual ~slot() noexcept {
      if (const auto lsp = system::slot_address<slot>(layout_id); lsp && !lsp->dying) lsp->detach(id);
    }

    /// コントロールの最小占有サイズと非拘束係数を取得する
    virtual tuple<float2, uint2> demand_survey() const noexcept {
      tuple<float2, uint2> result{};
      result.first.x = yw::max(size.x, minimum_size.x, 0.0f) + margin.x + margin.z;
      result.first.y = yw::max(size.y, minimum_size.y, 0.0f) + margin.y + margin.w;
      result.second.x = size.x < 0.0f;
      result.second.y = size.y < 0.0f;
      return result;
    }

    virtual void make_dirty() noexcept;
    virtual void make_messy() noexcept;

    virtual slotid hit_test(float2 Pt) const noexcept {
      return Pt.x < pos.x || Pt.y < pos.y || Pt.x > pos.x + size.x || Pt.y > pos.y + size.y ? slotid{} : id;
    }

    /// コントロールの位置とサイズを更新して描画する
    virtual void draw(float2 Pos, float2 Size) {
      pos = Pos + margin.xy();
      if (ucc.x) size.x = Size.x - margin.x - margin.z;
      if (ucc.y) size.y = Size.y - margin.y - margin.w;
      draw();
    };

    /// 前回の描画位置に再描画する
    virtual void draw() const {}

    virtual void char_event(wchar_t c) {}
    virtual void click_event(event::button e) {}
    virtual void button_event(event::button e) {}
    virtual void drag_event(event::drag e) {}
    virtual bool focus_event(bool) { return false; }
    virtual void key_event(event::key e) {}
    virtual void move_event(event::move e) {}
    virtual void wheel_event(event::wheel e) {}

    virtual void hover_event(event::hover Event) {
      if (enabled && on_hover) on_hover(Event);
      if (tooltip.empty()) return;
      if (Event.enter()) {
        if (const auto w = system::slot_address<ui::window>(window_id))
          system::tooltip.show(pos + w->pos() + w->margin.xy(), size, tooltip);
      } else if (Event.leave()) system::tooltip.hide();
    }

    virtual slotid next_tab_stop(slotid Current, bool Forward, bool& Found) {
      if (Found && visible && enabled && focus_event(true)) return id;
      if (Current == id) Found = true;
      return {};
    }
  };

  using unknown::operator bool;

  const auto& margin() const { return unsafe_get(&slot::margin); }
  const auto& pos() const { return unsafe_get(&slot::pos); }
  const auto& size() const { return unsafe_get(&slot::size); }
  const auto& width() const { return unsafe_get(&slot::width); }
  const auto& height() const { return unsafe_get(&slot::height); }
  const auto& radius() const { return unsafe_get(&slot::radius); }
  const auto& minimum_size() const { return unsafe_get(&slot::minimum_size); }
  const auto& visible() const { return unsafe_get(&slot::visible); }
  const auto& enabled() const { return unsafe_get(&slot::enabled); }
  const auto& tooltip() const { return unsafe_get(&slot::tooltip); }
  const auto& on_hover() const { return unsafe_get(&slot::on_hover); }

  void margin(const float4& margin) { safe_set_size(&slot::margin, margin); }
  void size(float2 size) {
    if (const auto csp = system::slot_address<slot>(_id)) {
      csp->ucc.x = size.x < 0.0f;
      csp->ucc.y = size.y < 0.0f;
      csp->size = size;
      csp->make_messy();
    }
  }
  void width(float1 width) {
    if (const auto csp = system::slot_address<slot>(_id)) {
      csp->ucc.x = width.x < 0.0f;
      csp->size.x = width.x;
      csp->make_messy();
    }
  }
  void height(float1 height) {
    if (const auto csp = system::slot_address<slot>(_id)) {
      csp->ucc.y = height.x < 0.0f;
      csp->size.y = height.x;
      csp->make_messy();
    }
  }
  void radius(float2 radius) { safe_set(&slot::radius, radius); }
  void minimum_size(float2 size) { safe_set_size(&slot::minimum_size, size); }
  void visible(bool b) { safe_set(&slot::visible, b); }
  void enabled(bool b) { safe_set(&slot::enabled, b); }
  template<stringable S> void tooltip(S&& s) { safe_set(&slot::tooltip, unicode<wchar_t>(static_cast<S&&>(s))); }
  void on_hover(function<void, event::hover> value) { safe_set(&slot::on_hover, std::move(value)); }
};
} // namespace yw::ui
