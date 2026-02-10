#pragma once
#include "ywx/window.h"

//////////////////////////////////////// MARK: control

namespace yw::controls {

class control {
protected:
  slotlist<window_slot>::id _master_id;
  slotlist<window_slot>::id _slave_id;
  slotlist<control_slot>::id _control_id;

  window_slot* _window() const noexcept {
    const auto master = window_class.windows.get(_master_id);
    if (_slave_id.is_zero()) return master;
    else return master ? master->slaves.get(_slave_id) : nullptr;
  }

  control_slot* _control() const noexcept {
    const auto w = _window();
    return w ? w->controls.get(_control_id) : nullptr;
  }

  template<typename Ctrl, typename Slot, typename W> static std::expected<Ctrl, error_trace> _add(W& window) {
    if (!window) return unexpected_error(errors::invalid_argument, "invalid window");
    slotlist<window_slot>::id mid, sid;
    mid.index = ::GetWindowLongPtrW(window.hwnd(), 0);
    mid.generation = ::GetWindowLongPtrW(window.hwnd(), 8);
    sid.index = ::GetWindowLongPtrW(window.hwnd(), 16);
    sid.generation = ::GetWindowLongPtrW(window.hwnd(), 24);
    auto w = window_class.windows.get(mid);
    if (w->is_slave) w = w->slaves.get(sid);
    if (!w) return unexpected_error(errors::invalid_argument, "invalid window");
    std::unique_ptr<Slot> slot = std::make_unique<Slot>();
    const auto cid = w->controls.push(std::move(slot));
    return Ctrl(mid, sid, cid);
  }

public:
  using slot_type = control_slot;

  virtual ~control() noexcept { this->remove(); }
  control() noexcept : _master_id(), _slave_id(), _control_id() {}

  explicit control(
    slotlist<window_slot>::id mid, slotlist<window_slot>::id sid, slotlist<control_slot>::id cid) noexcept
    : _master_id(mid), _slave_id(sid), _control_id(cid) {}

  control(control&& other) noexcept
    : _master_id(std::exchange(other._master_id, {})), _slave_id(std::exchange(other._slave_id, {})),
      _control_id(std::exchange(other._control_id, {})) {}

  control& operator=(control&& other) noexcept {
    if (this == &other) return *this;
    this->remove();
    _master_id = std::exchange(other._master_id, {});
    _slave_id = std::exchange(other._slave_id, {});
    _control_id = std::exchange(other._control_id, {});
    return *this;
  }

  explicit operator bool() const noexcept {
    const auto w = _window();
    return w ? w->controls.contains(_control_id) : false;
  }

  void remove() noexcept {
    if (const auto w = _window(); w) w->controls.erase(_control_id);
  }

  control_slot* operator->() const noexcept { return _control(); }

  static std::expected<control, error_trace> add(auto& window) { return _add<control, control_slot>(window); }
};
}

//////////////////////////////////////// MARK: label

namespace yw::controls {

class label : public control {
public:
  class slot : public control_slot {
  protected:
    std::wstring _text;
    yw::text_layout _text_layout;
  public:
    color text_color = colors::black;

    virtual std::expected<void, error_trace> draw() const override {
      if (auto res = control_slot::draw(); !res) return unexpected_error(res.error());
      if (!_text_layout) return unexpected_error(errors::not_initialized, "text_layout is not initialized");
      if (auto res = draw_text(position + padding, _text_layout, text_color); !res)
        return unexpected_error(res.error());
      return {};
    }

    const std::wstring& text() const noexcept { return _text; }

    void text(stringable<wchar_t> auto&& Text) {
      _text.assign(std::wstring_view(Text));
      if (auto res = yw::text_layout::create(_text, _text_layout); res) _text_layout = std::move(res.value());
      else _text_layout = {};
    }

    void reset_text_layout(text_format_like auto&& text_format, float2 size) {
      if (auto res = yw::text_layout::create(_text, (IDWriteTextFormat*)text_format, size); !res) _text_layout = {};
      else _text_layout = std::move(res.value());
    }

    void reset_text_layout(const text_layout& source) {
      if (auto res = yw::text_layout::create(_text, source); !res) _text_layout = {};
      else _text_layout = std::move(res.value());
    }

    std::wstring font_name() const {
      if (auto res = _text_layout.font_name(); res) return std::move(res.value());
      else return {};
    }

    float font_size() const {
      if (auto res = _text_layout.font_size(); res) return res.value();
      else return {};
    }

    DWRITE_FONT_WEIGHT font_weight() const {
      if (auto res = _text_layout.font_weight(); res) return res.value();
      else return {};
    }

    DWRITE_FONT_STYLE font_style() const {
      if (auto res = _text_layout.font_style(); res) return res.value();
      else return {};
    }

    DWRITE_FONT_STRETCH font_stretch() const {
      if (auto res = _text_layout.font_stretch(); res) return res.value();
      else return {};
    }

    DWRITE_TEXT_ALIGNMENT text_alignment() const {
      if (auto res = _text_layout.text_alignment(); res) return res.value();
      else return {};
    }
    void text_alignment(DWRITE_TEXT_ALIGNMENT align) {
      if (auto res = _text_layout.text_alignment(align); !res) return;
    }

    DWRITE_PARAGRAPH_ALIGNMENT paragraph_alignment() const {
      if (auto res = _text_layout.paragraph_alignment(); res) return res.value();
      else return {};
    }
    void paragraph_alignment(DWRITE_PARAGRAPH_ALIGNMENT align) {
      if (auto res = _text_layout.paragraph_alignment(align); !res) return;
    }
  };

protected:
  slot* _label() const noexcept { return dynamic_cast<slot*>(_control()); }

public:
  using slot_type = slot;
  using control::control;
  using control::operator bool;

  slot* operator->() const noexcept { return _label(); }
  static std::expected<label, error_trace> add(is_window auto& window) { return _add<label, slot>(window); }
};
}

//////////////////////////////////////// MARK: button

namespace yw::controls {

class button : public control {
public:
  class slot : public control_slot {
  protected:
    std::wstring _text;
    yw::text_layout _text_layout;
  public:
    color text_color = colors::black;
    std::function<std::expected<void, error_trace>()> on_click;
  };
};
}
