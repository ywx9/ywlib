#pragma once
#include "ywx/window.h"
#include "ywx/text_format.h"

namespace yw::control {

class base {
protected:
  slotlist<window::slot>::id _master_id{};
  slotlist<window::slot>::id _slave_id{};
  slotlist<control_slot>::id _control_id{};

  base(slotlist<window::slot>::id master_id, slotlist<window::slot>::id slave_id,
    slotlist<control_slot>::id control_id) noexcept
    : _master_id(master_id), _slave_id(slave_id), _control_id(control_id) {
    if (const auto ws = _window())
      if (const auto cs = ws->controls.get(_control_id)) cs->control_id = _control_id;
  }

  window::slot* _window() const noexcept {
    const auto ms = window::system.windows.get(_master_id);
    if (_slave_id.is_zero()) return ms;
    else return ms ? ms->slaves.get(_slave_id) : nullptr;
  }

  control_slot* _control() const noexcept {
    if (const auto w = _window()) return w->controls.get(_control_id);
    else return nullptr;
  }

  static window::slot* _window(const window& w) noexcept {
    const auto ms = window::system.windows.get(w._master_id);
    if (w._slave_id.is_zero()) return ms;
    else return ms ? ms->slaves.get(w._slave_id) : nullptr;
  }

  template<typename Ctrl, derived_from<control_slot> Slot>
  static std::expected<Ctrl, error_trace> _add(window& w, float2 Pos, float2 Size) {
    const auto ws = _window(w);
    if (!ws) return unexpected_error(errors::invalid_argument, "invalid window");
    auto cs = std::make_unique<Slot>();
    cs->master_id = w._master_id;
    cs->slave_id = w._slave_id;
    cs->position = Pos;
    cs->size = Size;
    const auto cid = ws->controls.push(std::move(cs));
    return Ctrl(w._master_id, w._slave_id, cid);
  }

public:
  using slot = control_slot;

  base() noexcept = default;

  base(base&& c) noexcept
    : _master_id(std::exchange(c._master_id, {})), _slave_id(std::exchange(c._slave_id, {})),
      _control_id(std::exchange(c._control_id, {})) {}

  base& operator=(base&& c) noexcept {
    if (this == &c) return *this;
    _master_id = std::exchange(c._master_id, {});
    _slave_id = std::exchange(c._slave_id, {});
    _control_id = std::exchange(c._control_id, {});
    return *this;
  }

  explicit operator bool() const noexcept {
    if (const auto w = _window()) return w->controls.contains(_control_id);
    else return false;
  }

  float2 position() const noexcept {
    if (const auto c = _control()) return c->position;
    else return float2{};
  }

  float2 size() const noexcept {
    if (const auto c = _control()) return c->size;
    else return float2{};
  }

  float2 radius() const noexcept {
    if (const auto c = _control()) return c->radius;
    else return float2{};
  }

  color background_color() const noexcept {
    if (const auto c = _control()) return c->background_color;
    else return colors::white;
  }

  color border_color() const noexcept {
    if (const auto c = _control()) return c->border_color;
    else return colors::black;
  }

  float border_width() const noexcept {
    if (const auto c = _control()) return c->border_width;
    else return 1.0f;
  }

  bool visible() const noexcept {
    if (const auto c = _control()) return c->visible;
    else return true;
  }

  bool enabled() const noexcept {
    if (const auto c = _control()) return c->enabled;
    else return true;
  }

  void position(float2 p) noexcept {
    if (const auto c = _control()) c->position = p;
  }

  void size(float2 s) noexcept {
    if (const auto c = _control()) c->size = s;
  }

  void radius(float2 r) noexcept {
    if (const auto c = _control()) c->radius = r;
  }

  void background_color(const color& c) noexcept {
    if (const auto csl = _control()) csl->background_color = c;
  }

  void border_color(const color& c) noexcept {
    if (const auto csl = _control()) csl->border_color = c;
  }

  void border_width(float w) noexcept {
    if (const auto c = _control()) c->border_width = w;
  }

  void visible(bool v) noexcept {
    if (const auto c = _control()) c->visible = v;
  }

  void enabled(bool e) noexcept {
    if (const auto c = _control()) c->enabled = e;
  }

  static std::expected<base, error_trace> add(window& w, float2 position, float2 size) {
    if (auto res = _add<base, control_slot>(w, position, size); res) return std::move(*res);
    else return unexpected_error(res.error());
  }
};

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
      if (auto res = draw_text(position + padding, text_layout, text_color); !res)
        return unexpected_error(res.error());
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

  static std::expected<label, error_trace> add(window& w, float2 position, float2 size) {
    if (auto res = _add<label, slot>(w, position, size); res) {
      auto &lbl = *res;
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
