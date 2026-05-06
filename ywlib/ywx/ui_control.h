#pragma once
#include "ywx/bitmap.h"
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
    static constexpr float default_value = 4.0f;

    slotid layout_id{}, window_id{};
    float4 margin{5.0f, 5.0f, 5.0f, 5.0f};
    float2 pos{};
    union { // clang-format off
      float2 size{};
      struct { float width, height; };
    }; // clang-format on
    float2 radius{5.0f, 5.0f};
    color border_color = colors::black;
    float border_width = 1.0f;
    std::variant<color, yw::bitmap> background = colors::white;
    /// index=0: auto geometry (owned), index=1: manual geometry (non-owned)
    std::variant<comptr<ID2D1Geometry>, ID2D1Geometry*> geometry{};
      mutable comptr<ID2D1Layer> layer{};
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
      update_geometry();
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
      update_geometry();
    };

    ID2D1Geometry* geometry_ptr() const {
      if (geometry.index() == 0) return std::get<0>(geometry).get();
      return std::get<1>(geometry);
    }

    void set_manual_geometry(ID2D1Geometry* geo) {
      geometry = geo;
      make_dirty();
    }

    void set_auto_geometry() {
      if (geometry.index() == 0) return;
      geometry = comptr<ID2D1Geometry>{};
    }

    /// ジオメトリを更新する
    std::expected<void, error_trace> update_geometry() {
      if (geometry.index() == 1) return {};
      if (auto res = d2d.initialize(); !res) return unexpected_error(res.error());
      auto& geo = std::get<0>(geometry);
      geo.release();
      D2D1_ROUNDED_RECT rrect{D2D1::RectF(pos.x, pos.y, pos.x + size.x, pos.y + size.y), radius.x, radius.y};
      ID2D1RoundedRectangleGeometry* rounded_geo = nullptr;
      const auto hr = d2d.factory()->CreateRoundedRectangleGeometry(rrect, &rounded_geo);
      if (FAILED(hr)) return unexpected_error(errors::operation_failed, "Failed to create geometry", int32_t(hr));
      geo.get() = rounded_geo;
      return {};
    }

    /// 背景を描画する
    std::expected<void, error_trace> draw_background() const {
      if (!drawing::d2d_drawing()) return unexpected_error(errors::invalid_operation, "drawing not begun");
      ID2D1Geometry* geo = geometry_ptr();
      if (!geo) return unexpected_error(errors::operation_failed, "Geometry is not set");
      if (std::holds_alternative<color>(background)) {
        brush.color(std::get<color>(background));
        d2d.context()->FillGeometry(geo, brush.d2d_brush(), nullptr);
        if (border_width > 0.0f) {
          brush.color(border_color);
          d2d.context()->DrawGeometry(geo, brush.d2d_brush(), border_width, brush.d2d_stroke());
        }
        return {};
      } else {
        const auto& bmp = std::get<yw::bitmap>(background);
        if (bmp) {
            if (!layer) {
              const auto hr = d2d.context()->CreateLayer(nullptr, &layer.get());
              if (FAILED(hr)) return unexpected_error(errors::operation_failed, "CreateLayer failed", int32_t(hr));
            }
          const D2D1_LAYER_PARAMETERS params =
            D2D1::LayerParameters(D2D1::InfiniteRect(), geo, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
          d2d.context()->PushLayer(params, layer.get());
          D2D1_RECT_F rect = D2D1::RectF(pos.x, pos.y, pos.x + size.x, pos.y + size.y);
          yw::draw_bitmap(pos, size, bmp);
          d2d.context()->PopLayer();
        }
        if (border_width > 0.0f) {
          brush.color(border_color);
          d2d.context()->DrawGeometry(geo, brush.d2d_brush(), border_width, brush.d2d_stroke());
        }
        return {};
      }
    }

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
  const auto& border_color() const { return unsafe_get(&slot::border_color); }
  const auto& border_width() const { return unsafe_get(&slot::border_width); }
  const auto& background() const { return unsafe_get(&slot::background); }

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
      csp->set_auto_geometry();
      csp->update_geometry();
      csp->make_messy();
    }
  }
  void width(float1 width) {
    if (const auto csp = system::slot_address<slot>(_id)) {
      csp->constrained.x = width.x >= 0.0f;
      csp->size.x = width.x;
      csp->set_auto_geometry();
      csp->update_geometry();
      csp->make_messy();
    }
  }
  void height(float1 height) {
    if (const auto csp = system::slot_address<slot>(_id)) {
      csp->constrained.y = height.x >= 0.0f;
      csp->size.y = height.x;
      csp->set_auto_geometry();
      csp->update_geometry();
      csp->make_messy();
    }
  }
  void radius(float2 radius) {
    if (auto csp = system::slot_address<slot>(_id)) {
      csp->radius = radius;
      csp->set_auto_geometry();
      csp->update_geometry();
      csp->make_dirty();
    }
  }
  void border_color(const color& c) { safe_set(&slot::border_color, c); }
  void border_width(float w) { safe_set(&slot::border_width, w); }
  void background(const color& c) { safe_set(&slot::background, std::variant<color, yw::bitmap>(c)); }
  void background(yw::bitmap b) { safe_set(&slot::background, std::variant<color, yw::bitmap>(std::move(b))); }
  void background(std::variant<color, yw::bitmap> bg) { safe_set(&slot::background, std::move(bg)); }
  void geometry(ID2D1Geometry* geo) {
    if (auto csp = system::slot_address<slot>(_id)) csp->set_manual_geometry(geo);
  }
  void min_size(float2 size) { safe_set_size(&slot::min_size, size); }
  void alignment(ui::alignment alignment) { safe_set_size(&slot::alignment, alignment); }
  void visible(bool b) { safe_set(&slot::visible, b); }
  void enabled(bool b) { safe_set(&slot::enabled, b); }
  template<stringable S> void tooltip(S&& s) { safe_set(&slot::tooltip, unicode<wchar_t>(static_cast<S&&>(s))); }
  void on_hover(function<void, event::hover> value) { safe_set(&slot::on_hover, std::move(value)); }
};




class frame;

class control_new : public unknown {
protected:
  control_new() noexcept = default;

  template<typename Mp, typename T> void safe_set(Mp Member, T&& Value) const {
    if (auto csp = system::slot_address<class_type<Mp>>(_id)) {
      csp->*Member = static_cast<T&&>(Value);
      csp->make_dirty();
    }
  }

  template<typename Mp, typename T> void safe_set_size(Mp Member, T&& Value) const {
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
  struct slot : public unknown::slot {
    static constexpr float default_value = 4.0f;

    slotid layout_id{}, window_id{};
    float4 margin = float4::fill(default_value);
    float2 pos{};
    float2 size{};
    comptr<ID2D1Geometry> geometry{};
    float2 min_size = float2::fill(default_value * 2);
    ui::alignment alignment = ui::alignment::center;
    vector<bool, 2> constrained{false, false};
    bool visible = true, enabled = true, dying = false;
    std::wstring tooltip{};
    function<void, event::hover> on_hover;

    void make_dirty() const noexcept;
    void make_messy() const noexcept;

    void align(float2 Extra) {
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

    virtual ~slot() noexcept {
      if (const auto lsp = system::slot_address<slot>(layout_id); lsp && !lsp->dying) lsp->detach(id);
      system::uis.erase(id);
    }

    virtual slotid hittest(float2 Pt) const {
      if (!visible || !geometry) return {};
      BOOL hit = false;
      const auto hr = geometry->FillContainsPoint(D2D1_POINT_2F{Pt.x, Pt.y}, D2D1::Matrix3x2F::Identity(), &hit);
      return SUCCEEDED(hr) && hit ? id : slotid{};
    }

    virtual float2 calculate_min_size() const {
      return vapply_r<float2>(yw::max, float2(), min_size, size * constrained);
    }

    virtual void update_geometry() {
      if (geometry) geometry->Release();
      D2D1_RECT_F rect = D2D1::RectF(pos.x, pos.y, pos.x + size.x, pos.y + size.y);
      ID2D1RectangleGeometry* rect_geo = nullptr;
      if (const auto hr = d2d.factory()->CreateRectangleGeometry(rect, &rect_geo); FAILED(hr))
        fatal_error(error(errors::operation_failed, "Failed to create geometry", int32_t(hr)));
      geometry.get() = rect_geo;
    }

    virtual void update_layout(float2 Pos, float2 Area) {
      pos = Pos + margin.xy();
      const auto size_ = Area - margin.xy() - margin.zw();
      size = (float2(1.0f, 1.0f) - constrained) * size_ + constrained * size;
      align(size_ - size);
      update_geometry();
    }

    virtual slotid next_tab_stop(slotid Current, bool Forward, bool& Found) {
      if (Found && visible && enabled && focusable()) return id;
      if (Current == id) Found = true;
      return {};
    }

    virtual bool accept_child(slotid Id) { return false; }

    virtual void draw() const {}
    virtual void draw_focus_ring() const {}

    virtual bool focusable() const noexcept { return false; }

    virtual float2 ime_position() const { return {}; };
    virtual void ime_insert_text(std::wstring_view) {}

    virtual void char_event(wchar_t Char) {}
    virtual void click_event(event::button Event) {}
    virtual void button_event(event::button Event) {}
    virtual void drag_event(event::drag Event) {}
    virtual void focus_event(bool Focused) {}
    virtual void hover_event(event::hover Event);
    virtual bool key_event(event::key Event) { return false; }
    virtual void move_event(event::move Event) {}
    virtual void wheel_event(event::wheel Event) {}
  };
};
} // namespace yw::ui
