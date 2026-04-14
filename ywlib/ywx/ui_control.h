#pragma once
#include "ywx/event.h"
#include "ywx/tooltip.h"
#include "ywx/ui_unknown.h"

/*
描画の仕組みを整理する。
- `get_min_size()`:
  そのコントロールの最小描画サイズを取得する。
  サイズが拘束されている場合、そのサイズと`min_size`の最大値を返す。

- `update_layout(float2 Pos, float2 Area)`:
  コントロールを指定の位置とエリアでレイアウト確定する。
  ここで指定されるエリアは、サイズ+マージンを考慮した領域であり、必ずこれよりも大きい。

- `draw()`:
  前回描画した位置とエリア(サイズ)で描画する。

レイアウト
*/

namespace yw::ui {

/// コントロールの配置を指定する列挙型
enum class alignment {
  center = 0b0000,
  left = 0b0001,
  right = 0b0010,
  top = 0b0100,
  bottom = 0b1000,
  left_top = 0b0101,
  left_bottom = 0b1001,
  right_top = 0b0110,
  right_bottom = 0b1010,
};

/// すべてのコントロールの基底クラス
class control : public unknown {
protected:
  control() noexcept = default;

  template<typename Mp> member_type<Mp>& unsafe_get_size(Mp Member) const {
    if (const auto csp = system::slot_address<class_type<Mp>>(_id)) {
      csp->make_messy();
      return csp->*Member;
    } else throw std::logic_error("Invalid member access");
  }

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
    float2 min_size{10.0f, 10.0f};
    ui::alignment alignment = ui::alignment::center;
    vector<bool, 2> constrained{false, false};
    bool visible = true, enabled = true, dying = false;

    std::wstring tooltip{};
    function<void, event::hover> on_hover;

    virtual ~slot() noexcept {
      if (const auto lsp = system::slot_address<slot>(layout_id); lsp && !lsp->dying) lsp->detach(id);
      system::uis.erase(id);
    }

    /// 所属ウィンドウのdirtyフラグを立てる
    virtual void make_dirty() noexcept;

    /// 所属ウィンドウのmessyフラグを立てる
    virtual void make_messy() noexcept;

    /// 矩形ヒットテストの結果によって自身のIDか無効IDを返す
    virtual slotid hit_test(float2 Pt) const noexcept {
      return Pt.x < pos.x || Pt.y < pos.y || Pt.x > pos.x + size.x || Pt.y > pos.y + size.y ? slotid{} : id;
    }

    /// 最小サイズを計算する
    virtual float2 calculate_size() const noexcept {
      return vapply_r<float2>(yw::max, float2(), min_size, size * constrained);
    }

    /// 描画の前準備として`size`を更新する
    virtual void update_size() noexcept {
      min_size = vapply_r<float2>(yw::max, min_size, float2(0.0f, 0.0f));
      size = vapply_r<float2>(yw::max, min_size, size * constrained);
    }

    /// 描画サイズに余剰がある場合に配置を調整する
    virtual void align(float2 Extra) {
      if (Extra == float2()) return;
      switch (alignment) {
      case ui::alignment::center: pos += Extra * 0.5f; break;
      case ui::alignment::left: break;
      case ui::alignment::right: pos.x += Extra.x; break;
      case ui::alignment::top: break;
      case ui::alignment::bottom: pos.y += Extra.y; break;
      case ui::alignment::left_top: break;
      case ui::alignment::left_bottom: pos.y += Extra.y; break;
      case ui::alignment::right_top: pos.x += Extra.x; break;
      case ui::alignment::right_bottom: pos += Extra; break;
      }
    }

    /// 描画レイアウトを更新する
    virtual void update_layout(float2 Pos, float2 Area) {
      pos = Pos + margin.xy();
      const auto size_ = Area - margin.xy() - margin.zw();
      size = (float2(1.0f, 1.0f) - constrained) * size_ + constrained * size;
      align(size_ - size);
    };

    /// 前回の描画位置に再描画する
    virtual void draw() const {}

    /// フォーカス・リングを描画する
    /// \note ラジオボタンなどでoverrideが必要
    virtual void draw_focus_ring(float offset, float width) const {
      const auto off = float2::fill(offset);
      draw_round_rectangle(pos - off, size + off * 2, radius + off);
    }

    virtual slotid next_tab_stop(slotid Current, bool Forward, bool& Found) {
      if (Found && visible && enabled && focus_event(true)) return id;
      if (Current == id) Found = true;
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

  using unknown::operator bool;

  const auto& margin() const { return unsafe_get(&slot::margin); }
  const auto& pos() const { return unsafe_get(&slot::pos); }
  const auto& size() const { return unsafe_get(&slot::size); }
  const auto& width() const { return unsafe_get(&slot::width); }
  const auto& height() const { return unsafe_get(&slot::height); }
  const auto& radius() const { return unsafe_get(&slot::radius); }
  const auto& min_size() const { return unsafe_get(&slot::min_size); }
  const auto& alignment() const { return unsafe_get(&slot::alignment); }
  const auto& visible() const { return unsafe_get(&slot::visible); }
  const auto& enabled() const { return unsafe_get(&slot::enabled); }
  const auto& tooltip() const { return unsafe_get(&slot::tooltip); }
  const auto& on_hover() const { return unsafe_get(&slot::on_hover); }

  void margin(const float4& margin) { safe_set_size(&slot::margin, margin); }
  void size(float2 size) {
    if (const auto csp = system::slot_address<slot>(_id)) {
      csp->constrained.x = size.x >= 0.0f;
      csp->constrained.y = size.y >= 0.0f;
      csp->size = size;
      csp->make_messy();
    }
  }
  void width(float1 width) {
    if (const auto csp = system::slot_address<slot>(_id)) {
      csp->constrained.x = width.x >= 0.0f;
      csp->size.x = width.x;
      csp->make_messy();
    }
  }
  void height(float1 height) {
    if (const auto csp = system::slot_address<slot>(_id)) {
      csp->constrained.y = height.x >= 0.0f;
      csp->size.y = height.x;
      csp->make_messy();
    }
  }
  void radius(float2 radius) { safe_set(&slot::radius, radius); }
  void min_size(float2 size) { safe_set_size(&slot::min_size, size); }
  void alignment(ui::alignment alignment) { safe_set_size(&slot::alignment, alignment); }
  void visible(bool b) { safe_set(&slot::visible, b); }
  void enabled(bool b) { safe_set(&slot::enabled, b); }
  template<stringable S> void tooltip(S&& s) { safe_set(&slot::tooltip, unicode<wchar_t>(static_cast<S&&>(s))); }
  void on_hover(function<void, event::hover> value) { safe_set(&slot::on_hover, std::move(value)); }
};
} // namespace yw::ui
