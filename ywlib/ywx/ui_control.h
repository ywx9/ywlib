#pragma once
#include "ywx/event.h"
#include "ywx/unknown.h"

namespace yw::ui {

inline constexpr float arbitrary_value = 4.0f;
using slotid = slotset<unknown::slot>::slotid;

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
    }
    csp->owner_window_id = lsp->owner_window_id;
    csp->owner_layout_id = lid;
    return cid;
  }

  control() noexcept = default;

public:
  struct slot : public unknown::slot {
    slotid owner_window_id{};
    slotid owner_layout_id{};

    struct core {
      slotid control_id{};
      float4 margin = float4::fill(arbitrary_value);
      float2 min_size = float2::fill(arbitrary_value * 2);
      float2 required_size{};
      float2 provided_pos{};
      float2 provided_size{};
      float2 pos{};
      float2 size{};
      float2 radius = float2::fill(arbitrary_value);
      comptr<ID2D1Geometry> geometry{};
      ui::alignment alignment = ui::alignment::center;
      vector<bool, 2> constrained{false, false};
      bool view_changed = false;
      bool geometry_changed = false;
      bool layout_changed = false;

      void reset_change_flags() { view_changed = geometry_changed = layout_changed = false; }

      std::expected<void, error_trace> update_geometry() {
        if (auto res = d2d.initialize(); !res) return unexpected_error(res.error());
        ID2D1RoundedRectangleGeometry* rrgp = nullptr;
        D2D1_ROUNDED_RECT rr{D2D1_RECT_F(pos.x, pos.y, pos.x + size.x, pos.y + size.y), radius.x, radius.y};
        if (const auto hr = d2d.factory()->CreateRoundedRectangleGeometry(&rr, &rrgp); FAILED(hr))
          return unexpected_error(errors::operation_failed, "Failed to create geometry", int32_t(hr));
        geometry.reset(rrgp);
        return {};
      }

      std::expected<void, error_trace> update_layout(float2 Pos, float2 Size) {
        provided_pos = Pos, provided_size = Size;
        static const float c[] = {0.5f, 0.0f, 1.0f};
        const float2 cc = {c[unsigned(alignment) % 3], c[(unsigned(alignment) / 3) % 3]};
        pos = Pos + margin.xy();
        auto sz = Size - (margin.xy() + margin.zw());
        size = size * constrained + sz * (int2(1, 1) - constrained);
        pos += (sz - size) * cc;
        if (auto res = update_geometry(); !res) return unexpected_error(res.error());
        return {};
      }

      float2 minimum_size() const noexcept { return vapply_r<float2>(yw::max, min_size, required_size * constrained); }
      float2 area() const noexcept { return size + margin.xy() + margin.zw(); }

      bool hittest(float2 Pt) const noexcept {
        return Pt.x >= pos.x && Pt.x <= pos.x + size.x && Pt.y >= pos.y && Pt.y <= pos.y + size.y;
      }

      class handle {
        friend struct core;
        core* _p = nullptr;
        handle(core& Ref) : _p(&Ref) {}

      public:
        ~handle() {
          if (!_p) return;
          if (_p->layout_changed) {
            if (const auto csp = system::slot_address<control>(_p->control_id)) csp->make_messy();
          } else if (_p->geometry_changed) {
            _p->update_geometry();
            if (const auto csp = system::slot_address<control>(_p->control_id)) csp->make_dirty();
          } else if (_p->view_changed)
            if (const auto csp = system::slot_address<control>(_p->control_id)) csp->make_dirty();
          _p->view_changed = _p->geometry_changed = _p->layout_changed = false;
        }

        handle(handle&& Other) noexcept : _p(std::exchange(Other._p, nullptr)) {}
        handle& operator=(handle&& Other) noexcept {
          if (this != &Other) _p = std::exchange(Other._p, nullptr);
          return *this;
        }

        const auto& margin() const { return _p->margin; }
        auto& margin(float4 Margin) {
          _p->margin = Margin, _p->layout_changed = true;
          return *this;
        }

        const auto& min_size() const { return _p->min_size; }
        auto& min_size(float2 MinSize) {
          _p->min_size = vapply_r<float2>(yw::max, float2(), MinSize);
          _p->layout_changed = true;
          return *this;
        }

        const auto& pos() const { return _p->pos; }

        const auto& size() const { return _p->size; }
        auto& size(std::optional<float> width, std::optional<float> height) {
          if (_p->constrained.x = width.has_value()) _p->required_size.x = *width;
          if (_p->constrained.y = height.has_value()) _p->required_size.y = *height;
          _p->layout_changed = true;
          return *this;
        }

        const auto& width() const { return _p->size.x; }
        auto& width(std::optional<float> Width) {
          if (_p->constrained.x = Width.has_value()) _p->required_size.x = *Width;
          _p->layout_changed = true;
          return *this;
        }

        const auto& height() const { return _p->size.y; }
        auto& height(std::optional<float> Height) {
          if (_p->constrained.y = Height.has_value()) _p->required_size.y = *Height;
          _p->layout_changed = true;
          return *this;
        }

        const auto& radius() const { return _p->radius; }
        auto& radius(float2 Radius) {
          _p->radius = Radius;
          _p->geometry_changed = true;
          return *this;
        }

        const auto& alignment() const { return _p->alignment; }
        auto& alignment(ui::alignment Alignment) {
          _p->alignment = Alignment;
          _p->geometry_changed = true;
          return *this;
        }
      };

      handle handle() noexcept { return *this; }
    } core;

    bool visible = true;
    bool enabled = true;
    bool dying = false;

    std::wstring tooltip{};
    function<void, event::hover> on_hover;

    float2 area() const noexcept { return core.area(); }

    virtual void make_dirty() override {
      if (const auto wsp = system::slot_address<unknown>(owner_window_id)) wsp->make_dirty();
    }

    virtual void make_messy() override {
      if (const auto wsp = system::slot_address<unknown>(owner_window_id)) wsp->make_messy();
    }

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

    virtual slotid hittest(float2 Point) {
      if (core.hittest(Point)) return id;
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
} // namespace yw::ui
