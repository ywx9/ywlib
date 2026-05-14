#pragma once
#include "ywx/event.h"
#include "ywx/unknown.h"

namespace yw::errors {
define_error(ui_invalid_slotid);
}

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

struct part_base {
  slotid control_id{};
  bool view_changed = false;
  bool geometry_changed = false;
  bool layout_changed = false;

  void make_dirty() const {
    if (const auto csp = system::slot_address<unknown>(control_id)) csp->make_dirty();
  }
  void make_moved() const {
    if (const auto csp = system::slot_address<unknown>(control_id)) csp->make_moved();
  }
  void make_messy() const {
    if (const auto csp = system::slot_address<unknown>(control_id)) csp->make_messy();
  }

  template<typename Part> class handle {
  protected:
    Part* _p = nullptr;
    handle(Part& Ref) : _p(&Ref) {}

  public:
    ~handle() {
      if (!_p) return;
      if (_p->layout_changed) _p->make_messy();
      else if (_p->geometry_changed) _p->make_moved();
      else if (_p->view_changed) _p->make_dirty();
      _p->view_changed = _p->geometry_changed = _p->layout_changed = false;
    }
    handle(handle&& Other) noexcept : _p(std::exchange(Other._p, nullptr)) {}
    handle& operator=(handle&& Other) noexcept {
      if (this != &Other) _p = std::exchange(Other._p, nullptr);
      return *this;
    }
  };
};

class control : public unknown {
protected:
  template<derived_from<control> Ctrl>
  static std::expected<slotid, error_trace> create_control(derived_from<unknown> auto& Layout) {
    const auto lid = Layout.id();
    const auto lsp = system::slot_address<unknown>(lid);
    if (!lsp) return unexpected_error(errors::operation_failed, "Failed to access layout slot");
    if (const auto msg = lsp->attachable(); msg) return unexpected_error(errors::operation_failed, msg);
    const auto cid = system::uis.add(std::make_unique<typename Ctrl::slot>());
    const auto csp = system::slot_address<Ctrl>(cid);
    if (!csp) return unexpected_error(errors::operation_failed, "Failed to create control slot");
    lsp->attach(cid);
    csp->id = cid;
    csp->layout_id = lid;
    csp->window_id = lsp->window_id;
    return cid;
  }

  control() noexcept = default;

public:
  struct slot : public unknown::slot {
    struct core : public part_base {
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

      class handle : public part_base::handle<core> {
        friend struct core;
        using part_base::handle<core>::handle;

      public:
        const auto& margin() const { return _p->margin; }
        auto& margin(float4 Margin) {
          _p->margin = Margin;
          _p->layout_changed = true;
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
        auto& size(std::optional<float2> Size) {
          if (Size) {
            _p->constrained = {true, true};
            _p->required_size = *Size;
          } else _p->constrained = {false, false};
          _p->layout_changed = true;
          return *this;
        }

        const auto& width() const { return _p->size.x; }
        auto& width(std::optional<float> Width) {
          if (Width) {
            _p->constrained.x = true;
            _p->required_size.x = *Width;
          } else _p->constrained.x = false;
          _p->layout_changed = true;
          return *this;
        }

        const auto& height() const { return _p->size.y; }
        auto& height(std::optional<float> Height) {
          if (Height) {
            _p->constrained.y = true;
            _p->required_size.y = *Height;
          } else _p->constrained.y = false;
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
    } core;

    bool visible = true;
    bool enabled = true;
    bool dying = false;

    std::wstring tooltip{};
    function<void, events::hover> on_hover;

    //-- override functions --//

    virtual void make_dirty() override {
      if (const auto wsp = system::slot_address<unknown>(window_id)) wsp->make_dirty();
    }

    virtual void make_moved() override {
      if (auto res = core.update_geometry(); !res) {
        print_fallback.err("Failed to update geometry:\n{}", res.error());
        return;
      }
      if (const auto wsp = system::slot_address<unknown>(window_id)) wsp->make_dirty();
    }

    virtual void make_messy() override {
      if (const auto wsp = system::slot_address<unknown>(window_id)) wsp->make_messy();
    }

    //-- virtual functions --//

    virtual bool focusable() const { return false; }
    virtual void ensure_minimum_size() { core.size = core.minimum_size(); }
    virtual void update_layout(float2 Pos, float2 Size) { core.update_layout(Pos, Size); }
    virtual void draw() {}
    virtual slotid hittest(float2 Point) const { return core.hittest(Point) ? id : slotid{}; }
    virtual slotid next_tab_stop(slotid Focused, bool Forward, bool& Found) const {
      /// \note 以下はレイアウト以外のコントロールで共通化可能な実装。
      /// \note フォーカスを受けないコントロールなら即`return {}`するように`override`した方が良い。
      if (Focused == id) Found = true;
      else if (Found && visible && focusable()) return id;
      return {};
    }

    virtual float2 ime_position() const { return {}; };
    virtual void ime_insert_text(std::wstring_view) {}

    virtual void char_event(wchar_t c) {}
    virtual void click_event(events::button e) {}
    virtual void button_event(events::button e) {}
    virtual void drag_event(events::drag e) {}
    virtual bool focus_event(bool) { return false; }
    virtual void hover_event(events::hover Event);
    virtual bool key_event(events::key e) { return false; }
    virtual void move_event(events::move e) {}
    virtual void wheel_event(events::wheel e) {}
  };

  auto core() {
    const auto csp = system::slot_address<control>(_id);
    if (!csp) fatal_error(errors::ui_invalid_slotid);
    return csp->core.handle();
  }

  const auto core() const {
    const auto csp = system::slot_address<control>(_id);
    if (!csp) fatal_error(errors::ui_invalid_slotid);
    return csp->core.handle();
  }

  const auto& tooltip() const {
    const auto csp = system::slot_address<control>(_id);
    if (!csp) fatal_error(errors::ui_invalid_slotid);
    return csp->tooltip;
  }

  auto& tooltip(std::wstring Text) {
    const auto csp = system::slot_address<control>(_id);
    if (!csp) fatal_error(errors::ui_invalid_slotid);
    csp->tooltip = std::move(Text);
    return *this;
  }

  const auto& on_hover() const {
    const auto csp = system::slot_address<control>(_id);
    if (!csp) fatal_error(errors::ui_invalid_slotid);
    return csp->on_hover;
  }

  auto& on_hover(function<void, events::hover> Handler) {
    const auto csp = system::slot_address<control>(_id);
    if (!csp) fatal_error(errors::ui_invalid_slotid);
    csp->on_hover = std::move(Handler);
    return *this;
  }
};
} // namespace yw::ui
