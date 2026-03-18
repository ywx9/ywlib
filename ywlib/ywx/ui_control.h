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

public:
  class slot : public unknown::slot {
  protected:
    void update_last_rect(float2 Pos, float2 Size) const {
      Pos += margin.xy();
      Size -= margin.xy() + margin.zw();
      if (Size.x <= 0.0f || Size.y <= 0.0f) {
        last_rect = float4(Pos.x, Pos.y, Pos.x + yw::max(Size.x, 0.0f), Pos.y + yw::max(Size.y, 0.0f));
        return;
      }
      if (size.x >= 0.0f) Size.x = yw::min(Size.x, size.x);
      if (size.y >= 0.0f) Size.y = yw::min(Size.y, size.y);
      last_rect = float4(Pos, Pos + Size);
    }
  public:
    slotid layout_id{}, window_id{};
    union {
      float2 size{ui::unconstrained, ui::unconstrained};
      struct {
        float width, height;
      };
    };
    float2 minimum_size{10.0f, 10.0f};
    mutable float4 last_rect{}; // excluding margins
    float4 margin{5.0f, 5.0f, 5.0f, 5.0f};
    bool visible = true, enabled = true, dying = false;

    virtual ~slot() noexcept {
      if (const auto lsp = system::slot_address<slot>(layout_id); lsp && !lsp->dying) lsp->detach(id);
    }

    /// returns the minimum size and the number of unconstrained dimensions.
    virtual tuple<float2, uint2> require_size() const noexcept {
      tuple<float2, uint2> result{};
      result.first.x = yw::max(size.x, minimum_size.x) + margin.x + margin.z;
      result.first.y = yw::max(size.y, minimum_size.y) + margin.y + margin.w;
      result.second.x = size.x < 0.0f;
      result.second.y = size.y < 0.0f;
      return result;
    }

    /// need to be called when the control needs to be redrawn.
    virtual void make_dirty() noexcept;

    /// need to be called when the layout needs to be updated.
    virtual void make_messy() noexcept;

    /// checks if the given position is within the control's bounds.
    virtual slotid hit_test(float2 Pt) const noexcept { return {}; }

    /// draws the control at the given position with the given size.
    virtual void draw(float2 Pos, float2) const { last_rect = float4(Pos, Pos); };
    virtual void draw() const {}
    virtual void draw_focus() const {}

    virtual void button_event(event::button e) {}
    virtual void hover_event(event::hover e) {}
    virtual void move_event(event::move e) {}
    virtual void drag_event(event::drag e) {}
    virtual void key_event(event::key e) {}
    virtual void char_event(wchar_t c) {}
    virtual bool focus_event(bool) { return false; }
    virtual void click_event(event::button e) {}
  };

  ~control() noexcept { destroy(); }

  using unknown::operator bool;

  const float2& size() const { return unsafe_get(&slot::size); }
  void size(float2 value) { safe_set_size(&slot::size, value); }

  const float& width() const { return unsafe_get(&slot::width); }
  void width(float value) { safe_set_size(&slot::width, value); }

  const float& height() const { return unsafe_get(&slot::height); }
  void height(float value) { safe_set_size(&slot::height, value); }

  const float2& minimum_size() const { return unsafe_get(&slot::minimum_size); }
  void minimum_size(float2 value) { safe_set_size(&slot::minimum_size, value); }

  const float4& margin() const { return unsafe_get(&slot::margin); }
  void margin(const float4 value) { safe_set_size(&slot::margin, value); }

  const bool& visible() const { return unsafe_get(&slot::visible); }
  void visible(bool value) { safe_set(&slot::visible, value); }

  const bool& enabled() const { return unsafe_get(&slot::enabled); }
  void enabled(bool value) { safe_set(&slot::enabled, value); }

  void destroy() noexcept { system::uis.erase(_id); }
};
} // namespace yw
