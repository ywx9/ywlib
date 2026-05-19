// #pragma once
// #include "ywx/core.h"

// namespace yw::ui::part {

// struct text {
//   std::wstring string = L"";
//   font_config font = yw::font_config::default_;
//   text_alignment alignment = yw::text_alignment::left;
//   color color = colors::black;
//   float2 layout_size{};
//   comptr<IDWriteTextLayout> text_layout;

//   std::expected<void, error_trace> update_font_name() {
//     if (auto res = initialize(); !res) return unexpected_error(res.error());
//     IDWriteTextFormat* tfp = static_cast<IDWriteTextFormat*>(text_layout.get());
//     const auto n = tfp->GetFontFamilyNameLength();
//     std::wstring font_name(n, L'\0');
//     if (const auto hr = tfp->GetFontFamilyName(font_name.data(), n + 1); FAILED(hr))
//       return unexpected_error(errors::operation_failed, "GetFontFamilyName failed", int(hr));
//     font.name = std::move(font_name);
//     return {};
//   }

//   std::expected<void, error_trace> update_layout_size() {
//     if (auto res = initialize(); !res) return unexpected_error(res.error());
//     DWRITE_TEXT_METRICS metrics;
//     if (const auto hr = text_layout->GetMetrics(&metrics); FAILED(hr))
//       return unexpected_error(errors::operation_failed, "GetMetrics failed", int(hr));
//     layout_size = float2(metrics.width, metrics.height);
//     return {};
//   }

//   std::expected<void, error_trace> set_font_config(const yw::font_config& fc) {
//     if (auto res = initialize(); !res) return unexpected_error(res.error());
//     {
//       IDWriteTextFormat* tfp;
//       const auto hr = dwrite.factory()->CreateTextFormat( //
//         fc.name.value_or(*font.name).c_str(), nullptr, //
//         static_cast<DWRITE_FONT_WEIGHT>(fc.weight.value_or(*font.weight)), //
//         static_cast<DWRITE_FONT_STYLE>(fc.style.value_or(*font.style)), //
//         static_cast<DWRITE_FONT_STRETCH>(fc.stretch.value_or(*font.stretch)), //
//         fc.size.value_or(*font.size), L"", &tfp);
//       if (FAILED(hr) || !tfp) return unexpected_error(errors::operation_failed, "CreateTextFormat failed", int(hr));
//       tfp->SetTextAlignment(static_cast<DWRITE_TEXT_ALIGNMENT>(alignment));
//       {
//         IDWriteTextLayout* tlp = nullptr;
//         const auto hr = dwrite.factory()->CreateTextLayout(string.c_str(), UINT(string.size()), tfp, 1e6, 1e6, &tlp);
//         if (FAILED(hr) || !tlp) return unexpected_error(errors::operation_failed, "CreateTextLayout failed", int(hr));
//         text_layout.reset(tlp);
//       }
//     }
//     if (fc.weight.has_value()) font.weight = *fc.weight;
//     if (fc.style.has_value()) font.style = *fc.style;
//     if (fc.stretch.has_value()) font.stretch = *fc.stretch;
//     if (fc.size.has_value()) font.size = *fc.size;
//     if (fc.name.has_value())
//       if (auto res = update_font_name(); !res) fatal_error(res.error());
//     if (auto res = update_layout_size(); !res) fatal_error(res.error());
//   }

//   std::expected<void, error_trace> initialize() {
//     if (text_layout) return {};
//     if (auto res = dwrite.initialize(); !res) return unexpected_error(res.error());
//     font.size = font.size.value_or(16.0f);
//     font.weight = font.weight.value_or(font_weight::normal);
//     font.style = font.style.value_or(font_style::normal);
//     font.stretch = font.stretch.value_or(font_stretch::normal);
//     {
//       IDWriteTextFormat* tfp = nullptr;
//       const auto hr = dwrite.factory()->CreateTextFormat( //
//         font.name.value_or(L"").c_str(), nullptr, //
//         static_cast<DWRITE_FONT_WEIGHT>(*font.weight), //
//         static_cast<DWRITE_FONT_STYLE>(*font.style), //
//         static_cast<DWRITE_FONT_STRETCH>(*font.stretch), //
//         *font.size, L"", &tfp);
//       if (FAILED(hr) || !tfp) return unexpected_error(errors::operation_failed, "CreateTextFormat failed", int(hr));
//       tfp->SetTextAlignment(static_cast<DWRITE_TEXT_ALIGNMENT>(alignment));
//       {
//         IDWriteTextLayout* tlp = nullptr;
//         const auto hr = dwrite.factory()->CreateTextLayout(string.c_str(), UINT(string.size()), tfp, 1e6, 1e6, &tlp);
//         if (FAILED(hr) || !tlp) return unexpected_error(errors::operation_failed, "CreateTextLayout failed", int(hr));
//         text_layout.reset(tlp);
//       }
//     }
//     if (auto res = update_font_name(); !res) return unexpected_error(res.error());
//     if (auto res = update_layout_size(); !res) return unexpected_error(res.error());
//   }

//   class handle {
//     friend class text;
//     text* _p = nullptr;
//     handle(text& Ref) : _p(&Ref) {}

//   public:
//     const std::wstring& string() const { return _p->string; }
//     handle& string(std::wstring Text) {
//       if (auto res = _p->initialize(); !res) fatal_error(res.error());
//       {
//         IDWriteTextLayout* tlp = nullptr;
//         IDWriteTextFormat* tfp = static_cast<IDWriteTextFormat*>(_p->text_layout.get());
//         const auto hr = dwrite.factory()->CreateTextLayout(Text.c_str(), UINT(Text.size()), tfp, 1e6, 1e6, &tlp);
//         if (FAILED(hr) || !tlp) fatal_error(errors::operation_failed, "CreateTextLayout failed", int(hr));
//         _p->text_layout.reset(tlp);
//       }
//       _p->string = std::move(Text);
//       if (auto res = _p->update_layout_size(); !res) fatal_error(res.error());
//       return *this;
//     }

//     const std::wstring& font_name() const { return _p->font.name.value_or(L""); }
//     handle& font_name(std::wstring Name) {
//       if (auto res = _p->initialize(); !res) fatal_error(res.error());
//       if (auto res = _p->set_font_config({std::move(Name)}); !res) fatal_error(res.error());
//       return *this;
//     }

//     float font_size() const { return _p->font.size.value_or(16.0f); }
//     handle& font_size(float1 Size) {
//       if (auto res = _p->initialize(); !res) fatal_error(res.error());
//       if (auto res = _p->set_font_config({{}, Size.x}); !res) fatal_error(res.error());
//       return *this;
//     }

//     font_weight font_weight() const { return _p->font.weight.value_or(font_weight::normal); }
//     handle& font_weight(yw::font_weight Weight) {
//       if (auto res = _p->initialize(); !res) fatal_error(res.error());
//       if (auto res = _p->set_font_config({{}, {}, Weight}); !res) fatal_error(res.error());
//       return *this;
//     }

//     font_style font_style() const { return _p->font.style.value_or(font_style::normal); }
//     handle& font_style(yw::font_style Style) {
//       if (auto res = _p->initialize(); !res) fatal_error(res.error());
//       if (auto res = _p->set_font_config({{}, {}, {}, Style}); !res) fatal_error(res.error());
//       return *this;
//     }

//     font_stretch font_stretch() const { return _p->font.stretch.value_or(font_stretch::normal); }
//     handle& font_stretch(yw::font_stretch Stretch) {
//       if (auto res = _p->initialize(); !res) fatal_error(res.error());
//       if (auto res = _p->set_font_config({{}, {}, {}, {}, Stretch}); !res) fatal_error(res.error());
//       return *this;
//     }

//     yw::font_config font() const { return _p->font; }
//     handle& font(yw::font_config Config) {
//       if (auto res = _p->initialize(); !res) fatal_error(res.error());
//       if (auto res = _p->set_font_config(std::move(Config)); !res) fatal_error(res.error());
//       return *this;
//     }

//     color font_color() const { return _p->color; }
//     handle& font_color(color Color) { return _p->color = Color, *this; }

//     yw::text_alignment alignment() const { return _p->alignment; }
//     handle& alignment(yw::text_alignment Alignment) { return _p->alignment = Alignment, *this; }

//     float2 layout_size() const { return _p->layout_size; }

//     IDWriteTextLayout* text_layout() const { return _p->text_layout.get(); }
//   };

//   handle handle() noexcept { return *this; }
// };
// } // namespace yw::ui::part
