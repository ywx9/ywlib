#pragma once
#include "ywx/ui_unknown.h"

namespace yw::ui {
/// 特に意味を持たない初期値
inline constexpr float arbitrary_value = 4.0f;

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

using slotid = slotset<ui::unknown::slot>::slotid;
}

namespace yw::ui::part {

void make_dirty(slotid Window) noexcept;
void make_messy(slotid Window) noexcept;

struct core {
  slotid window_id{};
  float4 margin = float4::fill(arbitrary_value);
  float2 min_size = float2::fill(arbitrary_value * 2);
  float2 pos{};
  float2 size{};
  float2 provided_pos{};
  float2 provided_size{};
  float2 radius = float2::fill(arbitrary_value);
  comptr<ID2D1Geometry> geometry{};
  ui::alignment alignment = ui::alignment::center;
  vector<bool, 2> constrained{false, false};
  bool view_changed = false;
  bool geometry_changed = false;
  bool layout_changed = false;

  void reset_change_flags() { view_changed = geometry_changed = layout_changed = false; }

  std::expected<void, error_trace> update_geometry() {
    ID2D1RoundedRectangleGeometry* rrgp = nullptr;
    D2D1_ROUNDED_RECT rr{D2D1_RECT_F(pos.x, pos.y, pos.x + size.x, pos.y + size.y), radius.x, radius.y};
    if (const auto hr = d2d.factory()->CreateRoundedRectangleGeometry(&rr, &rrgp); FAILED(hr))
      return unexpected_error(errors::operation_failed, "Failed to create geometry", int32_t(hr));
    geometry.reset(rrgp);
    return {};
  }

  class handle {
    friend struct core;
    core* _p = nullptr;
    handle(core& Ref) : _p(&Ref) {}

  public:
    ~handle() {
      if (!_p) return;
      if (_p->layout_changed) make_messy(_p->window_id);
      else if (_p->geometry_changed) {
        _p->update_geometry();
        make_dirty(_p->window_id);
      } else if (_p->view_changed) make_dirty(_p->window_id);
      _p->reset_change_flags();
      _p = nullptr;
    }

    handle(handle&& Other) noexcept : _p(std::exchange(Other._p, nullptr)) {}
    handle& operator=(handle&& Other) noexcept {
      if (this != &Other) _p = std::exchange(Other._p, nullptr);
      return *this;
    }

    const auto& margin() const { return _p->margin; }
    auto& margin(float4 Margin) { _p->margin = Margin, _p->layout_changed = true; return *this; }

    const auto& min_size() const { return _p->min_size; }
    auto& min_size(float2 MinSize) { _p->min_size = MinSize; _p->layout_changed = true; return *this; }

    const auto& pos() const { return _p->pos; }

    const auto& size() const { return _p->size; }
    auto& size(std::optional<float> width, std::optional<float> height) {
      if (_p->constrained.x = width.has_value()) _p->size.x = *width;
      if (_p->constrained.y = height.has_value()) _p->size.y = *height;
      _p->layout_changed = true;
      return *this;
    }

    const auto& width() const { return _p->size.x; }
    auto& width(std::optional<float> Width) {
      if (_p->constrained.x = Width.has_value()) _p->size.x = *Width;
      _p->layout_changed = true;
      return *this;
    }

    const auto& height() const { return _p->size.y; }
    auto& height(std::optional<float> Height) {
      if (_p->constrained.y = Height.has_value()) _p->size.y = *Height;
      _p->layout_changed = true;
      return *this;
    }

    const auto& radius() const { return _p->radius; }
    auto& radius(float2 Radius) { _p->radius = Radius; _p->geometry_changed = true; return *this; }

    const auto& alignment() const { return _p->alignment; }
    auto& alignment(ui::alignment Alignment) { _p->alignment = Alignment; _p->geometry_changed = true; return *this; }
  };

  handle handle() noexcept { return *this; }
};

//////////////////////////////////////// MARK: background

struct background {
  slotid window_id{};
  color color = colors::white;
  yw::bitmap image{}; // optional
  float image_opacity = 1.0f;
  bool view_changed = false;

  class handle {
    friend struct background;
    background* _p = nullptr;
    handle(background& Ref) : _p(&Ref) {}

  public:
    ~handle() {
      if (!_p) return;
      if (_p->view_changed) make_dirty(_p->window_id);
      _p->view_changed = false;
      _p = nullptr;
    }

    handle(handle&& Other) noexcept : _p(std::exchange(Other._p, nullptr)) {}
    handle& operator=(handle&& Other) noexcept {
      if (this != &Other) _p = std::exchange(Other._p, nullptr);
      return *this;
    }

    const auto& color() const { return _p->color; }
    auto& color(yw::color Color) { return _p->color = Color, _p->view_changed = true, *this; }

    const auto& image() const { return _p->image; }
    auto& image(yw::bitmap Image) { return _p->image = std::move(Image), _p->view_changed = true, *this; }

    float image_opacity() const { return _p->image_opacity; }
    auto& image_opacity(float Opacity) { return _p->image_opacity = Opacity, _p->view_changed = true, *this; }
  };

  handle handle() noexcept { return *this; }
};

//////////////////////////////////////// MARK: border

struct border {
  slotid window_id{};
  color color = colors::black;
  float width = 1.0f;
  bool view_changed = false;

  class handle {
    friend struct border;
    border* _p = nullptr;
    handle(border& Ref) : _p(&Ref) {}

  public:
    ~handle() {
      if (!_p) return;
      if (_p->view_changed) make_dirty(_p->window_id);
      _p->view_changed = false;
      _p = nullptr;
    }

    handle(handle&& Other) noexcept : _p(std::exchange(Other._p, nullptr)) {}
    handle& operator=(handle&& Other) noexcept {
      if (this != &Other) _p = std::exchange(Other._p, nullptr);
      return *this;
    }

    const auto& color() const { return _p->color; }
    auto& color(yw::color Color) { return _p->color = Color, _p->view_changed = true, *this; }

    const auto& width() const { return _p->width; }
    auto& width(float Width) { return _p->width = Width, _p->view_changed = true, *this; }
  };

  handle handle() noexcept { return *this; }
};

//////////////////////////////////////// MARK: text

struct text {
  slotid window_id{};
  std::wstring string = L"";
  font_config font_config = yw::font_config::default_;
  text_alignment alignment = yw::text_alignment::left;
  color font_color = colors::black;
  float2 layout_size{};
  comptr<IDWriteTextLayout> text_layout;
  bool view_changed = false;

  std::expected<void, error_trace> update_font_name() {
    if (auto res = initialize(); !res) return unexpected_error(res.error());
    IDWriteTextFormat* tfp = static_cast<IDWriteTextFormat*>(text_layout.get());
    const auto n = tfp->GetFontFamilyNameLength();
    std::wstring font_name(n, L'\0');
    if (const auto hr = tfp->GetFontFamilyName(font_name.data(), n + 1); FAILED(hr))
      return unexpected_error(errors::operation_failed, "GetFontFamilyName failed", int(hr));
    font_config.name = std::move(font_name);
    return {};
  }

  std::expected<void, error_trace> update_layout_size() {
    if (auto res = initialize(); !res) return unexpected_error(res.error());
    DWRITE_TEXT_METRICS metrics;
    if (const auto hr = text_layout->GetMetrics(&metrics); FAILED(hr))
      return unexpected_error(errors::operation_failed, "GetMetrics failed", int(hr));
    layout_size = float2(metrics.width, metrics.height);
    return {};
  }

  std::expected<void, error_trace> set_font_config(const yw::font_config& fc) {
    if (auto res = initialize(); !res) return unexpected_error(res.error());
    {
      IDWriteTextFormat* tfp;
      const auto hr = dwrite.factory()->CreateTextFormat( //
        fc.name.value_or(*font_config.name).c_str(), nullptr, //
        static_cast<DWRITE_FONT_WEIGHT>(fc.weight.value_or(*font_config.weight)), //
        static_cast<DWRITE_FONT_STYLE>(fc.style.value_or(*font_config.style)), //
        static_cast<DWRITE_FONT_STRETCH>(fc.stretch.value_or(*font_config.stretch)), //
        fc.size.value_or(*font_config.size), L"", &tfp);
      if (FAILED(hr) || !tfp) return unexpected_error(errors::operation_failed, "CreateTextFormat failed", int(hr));
      tfp->SetTextAlignment(static_cast<DWRITE_TEXT_ALIGNMENT>(alignment));
      {
        IDWriteTextLayout* tlp = nullptr;
        const auto hr = dwrite.factory()->CreateTextLayout(string.c_str(), UINT(string.size()), tfp, 1e6, 1e6, &tlp);
        if (FAILED(hr) || !tlp) return unexpected_error(errors::operation_failed, "CreateTextLayout failed", int(hr));
        text_layout.reset(tlp);
      }
    }
    if (fc.weight.has_value()) font_config.weight = *fc.weight;
    if (fc.style.has_value()) font_config.style = *fc.style;
    if (fc.stretch.has_value()) font_config.stretch = *fc.stretch;
    if (fc.size.has_value()) font_config.size = *fc.size;
    if (fc.name.has_value())
      if (auto res = update_font_name(); !res) fatal_error(res.error());
    if (auto res = update_layout_size(); !res) fatal_error(res.error());
  }

  std::expected<void, error_trace> initialize() {
    if (text_layout) return {};
    if (auto res = dwrite.initialize(); !res) return unexpected_error(res.error());
    font_config.size = font_config.size.value_or(16.0f);
    font_config.weight = font_config.weight.value_or(font_weight::normal);
    font_config.style = font_config.style.value_or(font_style::normal);
    font_config.stretch = font_config.stretch.value_or(font_stretch::normal);
    {
      IDWriteTextFormat* tfp = nullptr;
      const auto hr = dwrite.factory()->CreateTextFormat( //
        font_config.name.value_or(L"").c_str(), nullptr, //
        static_cast<DWRITE_FONT_WEIGHT>(*font_config.weight), //
        static_cast<DWRITE_FONT_STYLE>(*font_config.style), //
        static_cast<DWRITE_FONT_STRETCH>(*font_config.stretch), //
        *font_config.size, L"", &tfp);
      if (FAILED(hr) || !tfp) return unexpected_error(errors::operation_failed, "CreateTextFormat failed", int(hr));
      tfp->SetTextAlignment(static_cast<DWRITE_TEXT_ALIGNMENT>(alignment));
      {
        IDWriteTextLayout* tlp = nullptr;
        const auto hr = dwrite.factory()->CreateTextLayout(string.c_str(), UINT(string.size()), tfp, 1e6, 1e6, &tlp);
        if (FAILED(hr) || !tlp) return unexpected_error(errors::operation_failed, "CreateTextLayout failed", int(hr));
        text_layout.reset(tlp);
      }
    }
    if (auto res = update_font_name(); !res) return unexpected_error(res.error());
    if (auto res = update_layout_size(); !res) return unexpected_error(res.error());
  }

  class handle {
    friend class text;
    text* _p = nullptr;
    handle(text& Ref) : _p(&Ref) {}

  public:
    ~handle() {
      if (!_p) return;
      if (_p->view_changed) make_dirty(_p->window_id);
      _p->view_changed = false;
      _p = nullptr;
    }

    handle(handle&& Other) noexcept : _p(std::exchange(Other._p, nullptr)) {}
    handle& operator=(handle&& Other) noexcept {
      if (this != &Other) _p = std::exchange(Other._p, nullptr);
      return *this;
    }

    const std::wstring& string() const { return _p->string; }
    handle& string(std::wstring Text) {
      if (auto res = _p->initialize(); !res) fatal_error(res.error());
      {
        IDWriteTextLayout* tlp = nullptr;
        IDWriteTextFormat* tfp = static_cast<IDWriteTextFormat*>(_p->text_layout.get());
        const auto hr = dwrite.factory()->CreateTextLayout(Text.c_str(), UINT(Text.size()), tfp, 1e6, 1e6, &tlp);
        if (FAILED(hr) || !tlp) fatal_error(errors::operation_failed, "CreateTextLayout failed", int(hr));
        _p->text_layout.reset(tlp);
      }
      _p->string = std::move(Text);
      if (auto res = _p->update_layout_size(); !res) fatal_error(res.error());
      _p->view_changed = true;
      return *this;
    }

    yw::font_config font() const { return _p->font_config; }
    handle& font(yw::font_config Config) {
      if (auto res = _p->initialize(); !res) fatal_error(res.error());
      if (auto res = _p->set_font_config(std::move(Config)); !res) fatal_error(res.error());
      _p->view_changed = true;
      return *this;
    }

    const std::wstring& font_name() const { return _p->font_config.name.value_or(L""); }
    handle& font_name(std::wstring Name) { return font({Name}); }

    float font_size() const { return _p->font_config.size.value_or(16.0f); }
    handle& font_size(float1 Size) { return font({{}, Size.x}); }

    font_weight font_weight() const { return _p->font_config.weight.value_or(font_weight::normal); }
    handle& font_weight(yw::font_weight Weight) { return font({{}, {}, Weight}); }

    font_style font_style() const { return _p->font_config.style.value_or(font_style::normal); }
    handle& font_style(yw::font_style Style) { return font({{}, {}, {}, Style}); }

    font_stretch font_stretch() const { return _p->font_config.stretch.value_or(font_stretch::normal); }
    handle& font_stretch(yw::font_stretch Stretch) { return font({{}, {}, {}, {}, Stretch}); }

    const auto& font_color() const { return _p->font_color; }
    auto& font_color(color Color) { return _p->font_color = Color, _p->view_changed = true, *this; }

    const auto& alignment() const { return _p->alignment; }
    auto& alignment(yw::text_alignment Alignment) { return _p->alignment = Alignment, _p->view_changed = true, *this; }

    const auto& layout_size() const { return _p->layout_size; }

    IDWriteTextLayout* text_layout() const { return _p->text_layout.get(); }
  };

  handle handle() noexcept { return *this; }
};

//////////////////////////////////////// MARK: focus_ring

struct focus_ring {
  slotid window_id{};
  yw::color color = yw::color(0.0f, 0.0f, 1.0f, 0.5f);
  float offset = 3.0f;
  float width = 1.0f;
  bool view_changed = false;

  class handle {
    friend struct focus_ring;
    focus_ring* _p = nullptr;
    handle(focus_ring& Ref) : _p(&Ref) {}
  public:
    ~handle() {
      if (!_p) return;
      make_dirty(_p->window_id);
      _p = nullptr;
    }

    handle(handle&& Other) noexcept : _p(std::exchange(Other._p, nullptr)) {}
    handle& operator=(handle&& Other) noexcept {
      if (this != &Other) _p = std::exchange(Other._p, nullptr);
      return *this;
    }

    const auto& color() const { return _p->color; }
    auto& color(yw::color Color) { return _p->color = Color, _p->view_changed = true, *this; }

    const auto& offset() const { return _p->offset; }
    auto& offset(float1 Offset) { return _p->offset = Offset.x, _p->view_changed = true, *this; }

    const auto& width() const { return _p->width; }
    auto& width(float1 Width) { return _p->width = Width.x, _p->view_changed = true, *this; }
  };

  handle handle() noexcept { return *this; }
};

}
