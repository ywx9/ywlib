#pragma once
#include "ywx/ui_base.h"
#include "ywx/ui_part_label.h"

namespace yw::ui {

class edit : public base {
public:
	class slot : public base::slot {
	public:
		part::label text;
		size_t caret_pos{};

		slot() {
			focusable = true;
			enabled = true;
		}

		virtual std::expected<void, error_trace> draw() const override {
			if (auto res = base::slot::draw(); !res) return unexpected_error(res.error());
      if (auto res = text.draw(pos); !res) return unexpected_error(res.error());
      const auto w_slot_p = system::windows.get(window_id);
      if (w_slot_p && w_slot_p->focused_ui == id) {
        if (auto res = text.layout().hit_test(caret_pos)) {
          const float caret_x = pos.x + text.padding().x + res->x;
          const float caret_y = pos.y + text.padding().y + res->y;
          brush.color(text.color);
          if (auto res = draw_line({caret_x, caret_y}, {caret_x, caret_y + res->w}, 1.0f); !res)
            return unexpected_error(res.error());
        }
      }
		}

		virtual void key_event(event::key e) override {
			if (!focusable || !e.down) return;
      auto& value = text.text();
			if (e.code == key::backspace) {
				if (caret_pos > 0 && caret_pos <= value.size()) {
					value.erase(value.begin() + (caret_pos - 1));
					--caret_pos;
					if (const auto w = system::windows.get(window_id)) w->dirty = true;
				}
			} else if (e.code == key::left) {
				if (caret_pos > 0) {
					if (e.ctrl) {
						size_t pos = caret_pos - 1;
						const bool cur = is_alnum(value[pos]);
						while (pos > 0 && is_alnum(value[pos - 1]) == cur) --pos;
						caret_pos = pos;
					} else --caret_pos;
					if (const auto w = system::windows.get(window_id)) w->dirty = true;
				}
			} else if (e.code == key::right) {
				if (caret_pos < value.size()) {
					if (e.ctrl) {
						size_t pos = caret_pos;
						const bool cur = is_alnum(value[pos]);
						while (++pos < value.size() && is_alnum(value[pos]) == cur) continue;
						caret_pos = pos;
					} else ++caret_pos;
					if (const auto w = system::windows.get(window_id)) w->dirty = true;
				}
			} else if (e.code == key::home) {
				caret_pos = 0;
				if (const auto w = system::windows.get(window_id)) w->dirty = true;
			} else if (e.code == key::end) {
				caret_pos = value.size();
				if (const auto w = system::windows.get(window_id)) w->dirty = true;
			}
		}

		virtual void char_event(wchar_t c) override {
			if (!is_focusable()) return;
			if (is_ascii(c)) {
        const auto text_slot_p = text.slot_address(&text);
        if (!text_slot_p) return;
        auto& value = text_slot_p->text;
				if (caret_pos > value.size()) caret_pos = value.size();
				value.insert(value.begin() + caret_pos, c);
        text_slot_p->update_layout();
				++caret_pos;
				if (const auto w = system::windows.get(window_id)) w->dirty = true;
			}
		}
	};

public:
	using base::operator bool;
	slot* slot_address() const noexcept { return dynamic_cast<slot*>(system::uis.get(_id)); }

	const auto& text() const { return unsafe_get(&slot::text); }
  const auto& caret_pos() const { return unsafe_get(&slot::caret_pos); }

  void caret_pos(size_t value) {
    if (const auto s = slot_address()) {
      s->caret_pos = value;
      if (const auto w = system::windows.get(s->window_id)) w->dirty = true;
    }
  }

	template<included_in<window&, none> Window, stringable S>
  static std::expected<edit, error_trace> add(Window&& w, float2 Pos, float2 Size) {
		if (auto res = base::add<edit>(w, Pos, Size)) {
			const auto slot_p = res->second;
			if (auto text_res = ui::text::add(none{}, Pos, Size)) {
				slot_p->text = std::move(*text_res);
				return edit{std::move(res->first)};
			} else return unexpected_error(text_res.error());
		} else return unexpected_error(res.error());
	}
};
} // namespace yw::ui
