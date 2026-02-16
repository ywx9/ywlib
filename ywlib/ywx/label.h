#pragma once
#include "ywx/text_format.h"
#include "ywx/window.h"

namespace yw::control {

//////////////////////////////////////// MARK: control::label

class label : public base {
public:
  class slot : public base::slot {
  public:
    float2 padding{};
    color text_color = colors::black;
    std::wstring text{};
    yw::text_layout text_layout{};

    virtual std::expected<void, error_trace> draw() const {
      if (auto res = base::slot::draw(); !res) return unexpected_error(res.error());
      if (auto res = draw_text(position + padding, text_layout, text_color); !res) return unexpected_error(res.error());
      return {};
    }

    virtual std::expected<bool, error_trace> proc(const MSG& msg) { return false; }
  };

protected:
  slot* _label() const noexcept {
    if (const auto w = _window()) return dynamic_cast<slot*>(w->controls.get(_control_id));
    else return nullptr;
  }

public:
  using base::base;
  using base::operator bool;

  float2 padding() const noexcept {
    if (const auto l = _label()) return l->padding;
    else return float2{};
  }

  color text_color() const noexcept {
    if (const auto l = _label()) return l->text_color;
    else return color();
  }

  std::wstring text() const noexcept {
    if (const auto l = _label()) return l->text;
    else return std::wstring{};
  }

  void padding(float2 p) noexcept {
    if (const auto l = _label()) {
      if (auto res = yw::text_layout::create(l->text, l->text_layout, l->size - p * 2)) {
        l->text_layout = std::move(*res);
        l->padding = p;
      }
    }
  }

  void text_color(const color& c) noexcept {
    if (const auto l = _label()) l->text_color = c;
  }

  void text(stringable<wchar_t> auto&& t) {
    auto sv = static_cast<std::wstring_view>(t);
    if (const auto l = _label()) {
      if (auto res = yw::text_layout::create(sv, l->text_layout, l->size - l->padding * 2)) {
        l->text_layout = std::move(*res);
        l->text.assign(sv);
      }
    }
  }

  void text_format(text_format_like auto&& tf) {
    if (const auto l = _label()) {
      if (auto res = yw::text_layout::create(l->text, tf, l->size - l->padding * 2)) {
        l->text_layout = std::move(*res);
      }
    }
  }

  static std::expected<label, error_trace> add(window::slave& w, float2 position, float2 size) {
    if (auto res = dwrite.initialize(); !res) return unexpected_error(res.error());
    if (auto res = _add<label, slot>(w, position, size); res) {
      auto& lbl = *res;
      if (const auto ls = lbl._label()) {
        if (auto res = yw::text_layout::create(L"", dwrite.text_format(), size); !res)
          return unexpected_error(res.error());
        else ls->text_layout = std::move(*res);
        ls->text_layout.text_alignment(DWRITE_TEXT_ALIGNMENT_CENTER);
        ls->text_layout.paragraph_alignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
        return std::move(lbl);
      } else return unexpected_error(errors::operation_failed, "failed to get label slot");
    } else return unexpected_error(res.error());
  }
};
} // namespace yw::control
