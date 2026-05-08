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
} // namespace yw::ui

namespace yw::ui::part {

void make_dirty(slotid Window) noexcept;
void make_messy(slotid Window) noexcept;

struct core {
  slotid window_id{};
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

  float2 minimum_size() const {
    return vapply_r<float2>(yw::max, min_size, required_size * constrained);
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
    ~handle() noexcept {
      if (_p && _p->view_changed) make_dirty(_p->window_id), _p->view_changed = false;
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
    ~handle() noexcept {
      if (_p && _p->view_changed) make_dirty(_p->window_id), _p->view_changed = false;
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
  font_config font = yw::font_config::default_;
  text_alignment alignment = yw::text_alignment::left;
  ui::alignment block_alignment = ui::alignment::center;
  yw::color color = colors::black;
  float2 layout_size{};
  float2 offset{};
  comptr<IDWriteTextLayout> text_layout;
  bool view_changed = false;

  std::expected<void, error_trace> update_font_name() {
    if (auto res = initialize(); !res) return unexpected_error(res.error());
    IDWriteTextFormat* tfp = static_cast<IDWriteTextFormat*>(text_layout.get());
    const auto n = tfp->GetFontFamilyNameLength();
    std::wstring font_name(n, L'\0');
    if (const auto hr = tfp->GetFontFamilyName(font_name.data(), n + 1); FAILED(hr))
      return unexpected_error(errors::operation_failed, "GetFontFamilyName failed", int(hr));
    font.name = std::move(font_name);
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
        fc.name.value_or(*font.name).c_str(), nullptr, //
        static_cast<DWRITE_FONT_WEIGHT>(fc.weight.value_or(*font.weight)), //
        static_cast<DWRITE_FONT_STYLE>(fc.style.value_or(*font.style)), //
        static_cast<DWRITE_FONT_STRETCH>(fc.stretch.value_or(*font.stretch)), //
        fc.size.value_or(*font.size), L"", &tfp);
      if (FAILED(hr) || !tfp) return unexpected_error(errors::operation_failed, "CreateTextFormat failed", int(hr));
      tfp->SetTextAlignment(static_cast<DWRITE_TEXT_ALIGNMENT>(alignment));
      {
        IDWriteTextLayout* tlp = nullptr;
        const auto hr = dwrite.factory()->CreateTextLayout(string.c_str(), UINT(string.size()), tfp, 1e6, 1e6, &tlp);
        if (FAILED(hr) || !tlp) return unexpected_error(errors::operation_failed, "CreateTextLayout failed", int(hr));
        text_layout.reset(tlp);
      }
    }
    if (fc.weight.has_value()) font.weight = *fc.weight;
    if (fc.style.has_value()) font.style = *fc.style;
    if (fc.stretch.has_value()) font.stretch = *fc.stretch;
    if (fc.size.has_value()) font.size = *fc.size;
    if (fc.name.has_value())
      if (auto res = update_font_name(); !res) fatal_error(res.error());
    if (auto res = update_layout_size(); !res) fatal_error(res.error());
  }

  std::expected<void, error_trace> initialize() {
    if (text_layout) return {};
    if (auto res = dwrite.initialize(); !res) return unexpected_error(res.error());
    font.size = font.size.value_or(16.0f);
    font.weight = font.weight.value_or(font_weight::normal);
    font.style = font.style.value_or(font_style::normal);
    font.stretch = font.stretch.value_or(font_stretch::normal);
    {
      IDWriteTextFormat* tfp = nullptr;
      const auto hr = dwrite.factory()->CreateTextFormat( //
        font.name.value_or(L"").c_str(), nullptr, //
        static_cast<DWRITE_FONT_WEIGHT>(*font.weight), //
        static_cast<DWRITE_FONT_STYLE>(*font.style), //
        static_cast<DWRITE_FONT_STRETCH>(*font.stretch), //
        *font.size, L"", &tfp);
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

  std::expected<void, error_trace> draw(float2 Pos, float2 Size) {
    if (!text_layout) return unexpected_error(errors::not_initialized, "Not initialized");
    static const float c[] = {0.5f, 0.0f, 1.0f};
    const auto cc = float2(c[unsigned(block_alignment) % 3], c[(unsigned(block_alignment) / 3) % 3]);
    const auto text_origin = Pos + (Size - layout_size) * cc + offset;
    brush.color(color);
    draw_text(text_origin, text_layout.get());
  }

  class handle {
    friend class text;
    text* _p = nullptr;
    handle(text& Ref) : _p(&Ref) {}

  public:
    ~handle() noexcept {
      if (_p && _p->view_changed) make_dirty(_p->window_id), _p->view_changed = false;
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

    yw::font_config font() const { return _p->font; }
    handle& font(yw::font_config Config) {
      if (auto res = _p->initialize(); !res) fatal_error(res.error());
      if (auto res = _p->set_font_config(std::move(Config)); !res) fatal_error(res.error());
      _p->view_changed = true;
      return *this;
    }

    const std::wstring& font_name() const { return _p->font.name.value_or(L""); }
    handle& font_name(std::wstring Name) { return font({Name}); }

    float font_size() const { return _p->font.size.value_or(16.0f); }
    handle& font_size(float1 Size) { return font({{}, Size.x}); }

    font_weight font_weight() const { return _p->font.weight.value_or(font_weight::normal); }
    handle& font_weight(yw::font_weight Weight) { return font({{}, {}, Weight}); }

    font_style font_style() const { return _p->font.style.value_or(font_style::normal); }
    handle& font_style(yw::font_style Style) { return font({{}, {}, {}, Style}); }

    font_stretch font_stretch() const { return _p->font.stretch.value_or(font_stretch::normal); }
    handle& font_stretch(yw::font_stretch Stretch) { return font({{}, {}, {}, {}, Stretch}); }

    const auto& color() const { return _p->color; }
    auto& color(yw::color Color) { return _p->color = Color, _p->view_changed = true, *this; }

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
    ~handle() noexcept {
      if (_p && _p->view_changed) make_dirty(_p->window_id), _p->view_changed = false;
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

    const auto& offset() const { return _p->offset; }
    auto& offset(float1 Offset) { return _p->offset = Offset.x, _p->view_changed = true, *this; }
  };

  handle handle() noexcept { return *this; }
};

//////////////////////////////////////// MARK: scrollbar

template<bool Vertical> struct scrollbar {
  slotid window_id{};
  color track_color = colors::darkgray;
  color thumb_color = colors::gray;
  color border_color = colors::black;
  color icon_color = colors::black;
  float2 range{};
  float max_value{};
  float radius = arbitrary_value;
  float width = arbitrary_value;
  float border_width = 1.0f;
  bool view_changed = false;
  bool layout_changed = false;
  bool dragging = false;

  std::expected<void, error_trace> draw(float2 Pos, float2 Size) {
    constexpr bool v = Vertical;
    const float2 box_sz = {width, width};
    const float2 track_sz = v ? float2(width, Size.y - width * 2.0f) : float2(Size.x - width * 2.0f, width);
    const float left = get<!v>(Pos) + get<!v>(Size) - width;
    const float2 top_pos = v ? float2(left, 0) : float2(0, left);
    const float2 track_pos = v ? float2(left, width) : float2(width, left);
    const float2 bot_pos = v ? float2(left, Size.y - width) : float2(Size.x - width, left);
    float2 thumb_pos = track_pos;
    if (max_value == 0.0f) get<v>(thumb_pos) += get<v>(track_sz);
    else get<v>(thumb_pos) += get<v>(track_sz) * (range.x / max_value);
    float2 thumb_sz = track_sz;
    if (max_value != 0.0f) get<v>(thumb_sz) *= (range.y - range.x) / max_value;
    const auto r = yw::min(radius, width / 2);
    brush.color(track_color);
    fill_rectangle(track_pos, track_sz);
    brush.color(thumb_color);
    fill_round_rectangle(thumb_pos, thumb_sz, float2(r, r));
    fill_rectangle(top_pos, box_sz);
    fill_rectangle(bot_pos, box_sz);
    brush.color(border_color);
    draw_round_rectangle(thumb_pos, thumb_sz, float2(r, r));
    draw_rectangle(top_pos, box_sz);
    draw_rectangle(bot_pos, box_sz);
    const float o = width * 0.2f;
    const float w_o = width - o;
    if constexpr (Vertical) {
      const auto top = top_pos + float2(width * 0.5f, o);
      draw_line(top, top_pos + float2(o, w_o));
      draw_line(top, top_pos + float2(w_o, w_o));
      const auto bot = bot_pos + float2(width * 0.5f, w_o);
      draw_line(bot, bot_pos + float2(o, o));
      draw_line(bot, bot_pos + float2(w_o, o));
    } else {
      const auto top = top_pos + float2(o, width * 0.5f);
      draw_line(top, top_pos + float2(w_o, o));
      draw_line(top, top_pos + float2(w_o, w_o));
      const auto bot = bot_pos + float2(w_o, width * 0.5f);
      draw_line(bot, bot_pos + float2(o, o));
      draw_line(bot, bot_pos + float2(o, w_o));
    }
  }

  class handle {
    template<bool Vertical> friend struct scrollbar;
    scrollbar* _p = nullptr;
    handle(scrollbar& Ref) : _p(&Ref) {}

  public:
    ~handle() noexcept {
      if (!_p) return;
      if (_p->layout_changed) make_messy(_p->window_id);
      else if (_p->view_changed) make_dirty(_p->window_id);
      _p->layout_changed = _p->view_changed = false;
    }

    handle(handle&& Other) noexcept : _p(std::exchange(Other._p, nullptr)) {}
    handle& operator=(handle&& Other) noexcept {
      if (this != &Other) _p = std::exchange(Other._p, nullptr);
      return *this;
    }

    const auto& track_color() const { return _p->track_color; }
    auto& track_color(yw::color Color) { return _p->track_color = Color, _p->view_changed = true, *this; }

    const auto& thumb_color() const { return _p->thumb_color; }
    auto& thumb_color(yw::color Color) { return _p->thumb_color = Color, _p->view_changed = true, *this; }

    const auto& border_color() const { return _p->border_color; }
    auto& border_color(yw::color Color) { return _p->border_color = Color, _p->view_changed = true, *this; }

    const auto& icon_color() const { return _p->icon_color; }
    auto& icon_color(yw::color Color) { return _p->icon_color = Color, _p->view_changed = true, *this; }

    const auto& radius() const { return _p->radius; }
    auto& radius(float1 Radius) { return _p->radius = Radius.x, _p->view_changed = true, *this; }

    const auto& width() const { return _p->width; }
    auto& width(float1 Width) {
      _p->width = yw::max(0.0f, Width.x);
      _p->layout_changed = true;
      return *this;
    }

    const auto& max_value() const { return _p->max_value; }
    auto& max_value(float1 Value) {
      _p->max_value = yw::max(0.0f, Value.x);
      _p->view_changed = true;
      return *this;
    }

    const auto& border_width() const { return _p->border_width; }
    auto& border_width(float BorderWidth) { return _p->border_width = BorderWidth, _p->view_changed = true, *this; }

    const auto& range() const { return _p->range; }
  };

  handle handle() noexcept { return *this; }
};
} // namespace yw::ui::part
